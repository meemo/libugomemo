/* SPDX-License-Identifier: Apache-2.0 */

#include <libugomemo.h>

/**
 * crc32.c
 *
 * An implementation of the CRC-32 hash algorithm for use in verifying KWZ file sections.
 */


void CRC32_init(crc32_state *state) {
    *state = 0xFFFFFFFF;
}

void CRC32_update(crc32_state *state, u8 input_byte) {
#ifdef UGO_CFG_LARGE_BINARY_SIZE
    /* Faster method that requires the big poly-8 table. */
    *state = CRC32_POLY8[(*state ^ input_byte) & 0xFF] ^ (*state >> 8);
#else
    /* Slower method that doesn't require any additional tables. */
    u8 i;

    *state ^= input_byte;

    /* 8 cycles */
    for (i = 0; i < 8; i++) {
        /*                                            ????????????? */
        *state = (*state >> 1) ^ (0xEDB88320 & ((u32) -(*state & 1)));
    }
#endif
}

void CRC32_finish(crc32_state *state, u32 *crc32_output) {
    *crc32_output = ~(*state);
}

/**
 * Automatically calculates the CRC32 of a given buffer.
 */
void CRC32_calculate(u8 *buffer, uint buffer_len, u32 *crc32_output) {
    crc32_state state;
    uint i;

    CRC32_init(&state);

    for (i = 0; i < buffer_len; i++) {
        CRC32_update(&state, buffer[i]);
    }

    CRC32_finish(&state, crc32_output);
}
