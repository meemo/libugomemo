#ifndef UGOMEMO_CRYPTO_H_
#define UGOMEMO_CRYPTO_H_

#include <ugomemo/types.h>

// RSA key container for signature operations.
// Public keys for verification are built into the library (see keys.h).
// Private keys for signing must be provided by the caller.
typedef struct {
    u8 *modulus;
    size_t modulus_size;
    u8 *private_exponent;
    size_t private_exponent_size;
    u8 *asn_header;         // ASN.1 DigestInfo header for PKCS#1 v1.5 padding
    size_t asn_header_size;
    size_t signature_size;  // RSA signature byte length (e.g. 128 for PPM, 256 for KWZ)
    size_t hash_size;       // digest byte length (SHA1=20, SHA256=32)
    size_t padding_start;   // byte offset where 0xFF padding ends (byte before 0x00 separator)
} ugomemo_rsa_key;

// Load an RSA public key from a DER-encoded file.
// Fills key->modulus and key->modulus_size. Caller must free key->modulus.
int ugomemo_rsa_key_load_public_der(const char *path, ugomemo_rsa_key *key);

// Free dynamically allocated key data (from DER loading). Does not free the struct itself.
void ugomemo_rsa_key_free(ugomemo_rsa_key *key);

#endif
