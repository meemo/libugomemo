#include <string.h>

#include <libugomemo.h>

/**
 * sha1.c
 *
 * An implementation of the SHA-1 hash digest algorithm for use in PPM file signatures. This is heavily modified
 * version of Steve Reid <steve@edmweb.com>'s public domain SHA-1 implementation, which can be found in the
 * following repository: https://github.com/clibs/sha1
 *
 * Requires stdlib for memcpy and memset.
 */


void SHA1Init(sha1_ctx *context) {
    /* SHA1 initialization constants. */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

void SHA1Update(sha1_ctx *context, const u8 *data, u32 len) {
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

    UGO_MEMCPY(&context->buffer[j], &data[i], len - i);
}

void SHA1Transform(u32 state[5], const u8 buffer[64]) {
    unsigned int i;
    u32 a, b, c, d, e;

    typedef union {
        u8  c[64];
        u32 l[16];
    } CHAR64LONG16;

    CHAR64LONG16 block[1];
    memcpy(block, buffer, 64);

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* R0 */
    for (i = 0; i < 15; i += 5) {
        R0(a, b, c, d, e, i);
        R0(e, a, b, c, d, (i + 1));
        R0(d, e, a, b, c, (i + 2));
        R0(c, d, e, a, b, (i + 3));
        R0(b, c, d, e, a, (i + 4));
    }

    R0(a, b, c, d, e, 15);

    /* R1 */
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);

    /* R2 */
    for (i = 20; i < 40; i += 5) {
        R2(a, b, c, d, e, i);
        R2(e, a, b, c, d, (i + 1));
        R2(d, e, a, b, c, (i + 2));
        R2(c, d, e, a, b, (i + 3));
        R2(b, c, d, e, a, (i + 4));
    }

    /* R3 */
    for (i = 40; i < 60; i += 5) {
        R3(a, b, c, d, e, i);
        R3(e, a, b, c, d, (i + 1));
        R3(d, e, a, b, c, (i + 2));
        R3(c, d, e, a, b, (i + 3));
        R3(b, c, d, e, a, (i + 4));
    }

    /* R4 */
    for (i = 60; i < 80; i += 5) {
        R4(a, b, c, d, e, i);
        R4(e, a, b, c, d, (i + 1));
        R4(d, e, a, b, c, (i + 2));
        R4(c, d, e, a, b, (i + 3));
        R4(b, c, d, e, a, (i + 4));
    }

    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    /* Wipe variables */
    a = b = c = d = e = 0;
    UGO_MEMSET(block, '\0', sizeof(block));
}

void SHA1Final(u8 digest[20], sha1_ctx *context) {
    unsigned int i;
    u8 finalcount[8];
    u8 c;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (u8) ((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    }

    c = 128;  /* Was originally octal 200, which is equivalent to base 10 128. */
    SHA1Update(context, &c, 1);

    while ((context->count[0] & 504) != 448) {
        c = 0;
        SHA1Update(context, &c, 1);
    }

    SHA1Update(context, finalcount, 8);

    for (i = 0; i < 20; i++){
        digest[i] = (u8) ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    UGO_MEMSET(context, '\0', sizeof(*context));
    UGO_MEMSET(&finalcount, '\0', sizeof(finalcount));
}

void SHA1(u8 *hash_out, const u8 *buffer, unsigned int len) {
    sha1_ctx ctx;
    unsigned int i;

    SHA1Init(&ctx);

    for (i = 0; i < len; i++) {
        SHA1Update(&ctx, (const u8 *)buffer + i, 1);
    }

    SHA1Final((u8 *)hash_out, &ctx);
}

