#include <stdlib.h>

#include <libugomemo.h>

/**
 * sha1.c
 *
 * An implementation of the SHA-1 hash digest algorithm for use in PPM file signatures. This is heavily modified and
 * condensed version of Steve Reid <steve@edmweb.com>'s public domain SHA-1 implementation, which can be found in the
 * following repository: https://github.com/clibs/sha1
 *
 * Requires stdlib for memcpy and memset.
 */


/* =========================================== Macros =========================================== */
#define rol(value, bits)  (((value) << (bits)) | ((value) >> (32 - (bits))))

#ifdef LITTLE_ENDIAN_
#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | (rol(block->l[i], 8) & 0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ \
                block->l[i & 15], 1))

/* Core operations of SHA-1. */
#define R0(v, w, x, y, z, i)  z += ((w & (x ^ y)) ^ y)       + blk0(i) + 0x5A827999 + rol(v, 5); w = rol(w, 30);
#define R1(v, w, x, y, z, i)  z += ((w & (x ^ y)) ^ y)       + blk(i)  + 0x5A827999 + rol(v, 5); w = rol(w, 30);
#define R2(v, w, x, y, z, i)  z += (w ^ x ^ y)               + blk(i)  + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v, w, x, y, z, i)  z += (((w | x) & y) | (w & x)) + blk(i)  + 0x8F1BBCDC + rol(v, 5); w = rol(w, 30);
#define R4(v, w, x, y, z, i)  z += (w ^ x ^ y)               + blk(i)  + 0xCA62C1D6 + rol(v, 5); w = rol(w, 30);
/* ============================================================================================== */


void SHA1Init(sha1_ctx * context) {
    /* SHA1 initialization constants. */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

void SHA1Update(sha1_ctx * context, const u8 *data, uint32_t len) {
    u32 i;
    u32 j;

    j = context->count[0];
    if ((context->count[0] += len << 3) < j) {
        context->count[1]++;
    }

    context->count[1] += (len >> 29);

    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);

        for ( ; i + 63 < len; i += 64) {
            SHA1Transform(context->state, &data[i]);
        }

        j = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[j], &data[i], len - i);
}

void SHA1Final(u8 digest[20], sha1_ctx * context) {
    unsigned int i;
    u8 finalcount[8];
    u8 c;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (u8) ((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    }

    c = 200;
    SHA1Update(context, &c, 1);

    while ((context->count[0] & 504) != 448) {
        c = 0;
        SHA1Update(context, &c, 1);
    }

    SHA1Update(context, finalcount, 8);

    for (i = 0; i < 20; i++){
        digest[i] = (u8) ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    memset(context, '\0', sizeof(*context));
    memset(&finalcount, '\0', sizeof(finalcount));
}

void SHA1(u8 *hash_out, u8 *buffer, int len) {
    sha1_ctx ctx;
    unsigned int i;

    SHA1Init(&ctx);

    for (i = 0; i < len; i++) {
        SHA1Update(&ctx, (const u8 *)buffer + i, 1);
    }

    SHA1Final((u8 *)hash_out, &ctx);
}
