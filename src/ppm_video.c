/* SPDX-License-Identifier: Apache-2.0 */

#include <libugomemo.h>

/**
 * ppm_video.c
 *
 * This file contains functions for processing video (animation frames) data from a PPM file.
 */

/* Framerate lookup table for KWZ files. 0.0f is a placeholder; framerates are indexed from 1. */
/*
static const float PPM_FRAMERATES[9] = { 0.0f, 0.5f, 1.0f, 2.0f, 4.0f, 6.0f, 12.0f, 20.0f, 30.0f };
*/

/* { R, G, B } */
static const rgb24_pixel PPM_THUMBNAIL_PALETTE[16] = {
    { 0xFF, 0xFF, 0xFF },
    { 0x52, 0x52, 0x52 },
    { 0xFF, 0xFF, 0xFF },
    { 0x9C, 0x9C, 0x9C },
    { 0xFF, 0x48, 0x44 },
    { 0xC8, 0x51, 0x4F },
    { 0xFF, 0xAD, 0xAC },
    { 0x00, 0xFF, 0x00 },
    { 0x48, 0x40, 0xFF },
    { 0x51, 0x4F, 0xB8 },
    { 0xAD, 0xAB, 0xFF },
    { 0x00, 0xFF, 0x00 },
    { 0xB6, 0x57, 0xB7 },
    { 0x00, 0xFF, 0x00 },
    { 0x00, 0xFF, 0x00 },
    { 0x00, 0xFF, 0x00 }
};

/**
 * Decodes a PPM thumbnail to a stream of RGB8 bytes in output_buffer.
 */
void PPMDecodeThumbnail(u8 *buffer, rgb24_pixel *output_buffer) {
    u8 tile_y, tile_x, line, pixel;

    buffer += 0xA0;

    for (tile_y = 0; tile_y < 48; tile_y += 8)
    for (tile_x = 0; tile_x < 64; tile_x += 8)
    for (line   = 0; line   <  8; line   += 1)
    for (pixel  = 0; pixel  <  8; pixel  += 2) {
        /* Pixel 1 */
        output_buffer[((tile_y + line) * 64) + (tile_x + pixel)] = PPM_THUMBNAIL_PALETTE[*buffer >> 0x4];

        /* Pixel 2 */
        output_buffer[((tile_y + line) * 64) + (tile_x + pixel + 1)] = PPM_THUMBNAIL_PALETTE[*buffer & 0xF];

        buffer++;
    }
}
