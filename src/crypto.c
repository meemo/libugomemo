#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <ugomemo.h>

#include <gmp.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <zlib.h>

// Hashing

void sha256_hash(u8 *data, size_t data_len, u8 *digest) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    unsigned int digest_len;

    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, data, data_len);
    EVP_DigestFinal_ex(mdctx, digest, &digest_len);

    EVP_MD_CTX_free(mdctx);
}

void sha1_hash(u8 *data, u32 data_len, u8 *digest) {
    EVP_MD_CTX *mdctx;
    uint digest_len;

    mdctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
    EVP_DigestUpdate(mdctx, data, data_len);
    EVP_DigestFinal_ex(mdctx, digest, &digest_len);

    EVP_MD_CTX_free(mdctx);
}

// Uses zlib's implementation
u32 get_crc32(u8 *buffer, uint length) {
    uLong crc = crc32(0, NULL, 0);
    return (u32)crc32(crc, buffer, length);
}

// RSA operations using GMP - key-agnostic

#define RSA_PUBLIC_EXPONENT 0x010001  // 65537

int rsa_decrypt(const ugomemo_rsa_key *key, const u8 *sig_data, u8 *decrypted_out) {
    mpz_t modulus, exponent, signature, decrypted;
    size_t count = 0;
    u8 temp[256];

    mpz_inits(modulus, exponent, signature, decrypted, NULL);

    mpz_import(modulus, key->modulus_size, 1, 1, 0, 0, key->modulus);
    mpz_set_ui(exponent, RSA_PUBLIC_EXPONENT);
    mpz_import(signature, key->signature_size, 1, 1, 0, 0, sig_data);

    mpz_powm(decrypted, signature, exponent, modulus);

    memset(decrypted_out, 0, key->signature_size);
    mpz_export(temp, &count, 1, 1, 0, 0, decrypted);
    memcpy(decrypted_out + (key->signature_size - count), temp, count);

    mpz_clears(modulus, exponent, signature, decrypted, NULL);
    return UGOMEMO_OK;
}

int rsa_encrypt(const ugomemo_rsa_key *key, const u8 *plaintext, u8 *signature_out) {
    mpz_t modulus, private_exponent, message, signature;
    size_t count = 0;
    u8 temp[256];

    mpz_inits(modulus, private_exponent, message, signature, NULL);

    mpz_import(modulus, key->modulus_size, 1, 1, 0, 0, key->modulus);
    mpz_import(private_exponent, key->private_exponent_size, 1, 1, 0, 0, key->private_exponent);
    mpz_import(message, key->signature_size, 1, 1, 0, 0, plaintext);

    mpz_powm(signature, message, private_exponent, modulus);

    memset(signature_out, 0, key->signature_size);
    mpz_export(temp, &count, 1, 1, 0, 0, signature);
    memcpy(signature_out + (key->signature_size - count), temp, count);

    mpz_clears(modulus, private_exponent, message, signature, NULL);
    return UGOMEMO_OK;
}

int rsa_verify_signature(const ugomemo_rsa_key *key, const u8 *hash, const u8 *decrypted_signature) {
    size_t ps = key->padding_start;
    size_t asn_offset = ps + 1;
    size_t hash_offset = asn_offset + key->asn_header_size;

    if (decrypted_signature[0] != 0x00) return UGOMEMO_SIGNATURE_ERROR_1;
    if (decrypted_signature[1] != 0x01) return UGOMEMO_SIGNATURE_ERROR_1;

    for (size_t i = 2; i < ps; i++) {
        if (decrypted_signature[i] != 0xFF) return UGOMEMO_SIGNATURE_ERROR_1;
    }

    if (decrypted_signature[ps] != 0x00) return UGOMEMO_SIGNATURE_ERROR_1;

    if (memcmp(key->asn_header, decrypted_signature + asn_offset, key->asn_header_size) != 0)
        return UGOMEMO_SIGNATURE_ERROR_1;

    if (memcmp(hash, decrypted_signature + hash_offset, key->hash_size) != 0)
        return UGOMEMO_SIGNATURE_ERROR_2;

    return UGOMEMO_OK;
}

void rsa_create_pkcs1_padding(const ugomemo_rsa_key *key, const u8 *hash, u8 *padded_out) {
    size_t ps = key->padding_start;
    size_t asn_offset = ps + 1;
    size_t hash_offset = asn_offset + key->asn_header_size;

    padded_out[0] = 0x00;
    padded_out[1] = 0x01;

    for (size_t i = 2; i < ps; i++) {
        padded_out[i] = 0xFF;
    }

    padded_out[ps] = 0x00;
    memcpy(padded_out + asn_offset, key->asn_header, key->asn_header_size);
    memcpy(padded_out + hash_offset, hash, key->hash_size);
}

// Generate an RSA signature over file_data using the given key.
// use_sha256: true for KWZ (SHA-256), false for PPM (SHA-1).
int rsa_generate_signature(const ugomemo_rsa_key *key, const u8 *file_data, size_t file_size,
                           u8 *signature_out, bool use_sha256) {
    u8 hash[SHA256_SIZE];  // big enough for both SHA-256 and SHA-1
    u8 *padded = (u8 *)calloc(key->signature_size, 1);
    if (!padded) return UGOMEMO_MEMORY_ERROR;

    if (use_sha256)
        sha256_hash((u8*)file_data, file_size, hash);
    else
        sha1_hash((u8*)file_data, file_size, hash);

    rsa_create_pkcs1_padding(key, hash, padded);
    rsa_encrypt(key, padded, signature_out);

    free(padded);
    return UGOMEMO_OK;
}

// DER file loading using OpenSSL

int ugomemo_rsa_key_load_public_der(const char *path, ugomemo_rsa_key *key) {
    FILE *f = fopen(path, "rb");
    if (!f) return UGOMEMO_INPUT_ERROR;

    EVP_PKEY *pkey = d2i_PUBKEY_fp(f, NULL);
    fclose(f);
    if (!pkey) return UGOMEMO_INPUT_ERROR;

    BIGNUM *n = NULL, *e = NULL;
    if (!EVP_PKEY_get_bn_param(pkey, "n", &n) ||
        !EVP_PKEY_get_bn_param(pkey, "e", &e)) {
        EVP_PKEY_free(pkey);
        BN_free(n);
        BN_free(e);
        return UGOMEMO_INPUT_ERROR;
    }

    int mod_len = BN_num_bytes(n);
    key->modulus = (u8 *)calloc(mod_len, 1);
    if (!key->modulus) {
        BN_free(n); BN_free(e); EVP_PKEY_free(pkey);
        return UGOMEMO_MEMORY_ERROR;
    }
    BN_bn2bin(n, key->modulus);
    key->modulus_size = mod_len;

    BN_free(n);
    BN_free(e);
    EVP_PKEY_free(pkey);
    return UGOMEMO_OK;
}

void ugomemo_rsa_key_free(ugomemo_rsa_key *key) {
    if (!key) return;
    free(key->modulus);
    free(key->private_exponent);
    free(key->asn_header);
    key->modulus = NULL;
    key->private_exponent = NULL;
    key->asn_header = NULL;
}
