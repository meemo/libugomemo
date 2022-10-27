#include <libugomemo.h>

/**
 * ppm_video.c
 *
 * This file contains functions for processing video (animation frames) data from a PPM file.
 */


/* ========================================== Constants ========================================= */
/* Framerate lookup table for KWZ files. 0.0f is a placeholder; framerates are indexed from 1. */
static const float PPM_FRAMERATES[9] = { 0.0f, 0.5f, 1.0f, 2.0f, 4.0f, 6.0f, 12.0f, 20.0f, 30.0f };

/* { R, G, B } */
static const u8 THUMBNAIL_PALETTE[16][3] = {
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
/* ============================================================================================== */

/**
 * Decodes a PPM thumbnail to a stream of RGB8 bytes in output_buffer.
 */
void PPMDecodeThumbnail(const u8 *buffer, u8 *output_buffer) {
    u16 x, y;
    u8 tile_x, tile_y, pixel;

    for (tile_x = 0; tile_x < PPM_THUMBNAIL_WIDTH; tile_x++) {
        for (tile_y = 0; tile_y < PPM_THUMBNAIL_HEIGHT; tile_y++) {
            for (pixel = 0; pixel < 8; pixel += 2) {
                x = tile_x + pixel;
                y = tile_y + pixel;

                output_buffer++;
            }
        }
    }
}
