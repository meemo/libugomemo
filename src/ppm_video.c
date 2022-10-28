#include <libugomemo.h>

/**
 * ppm_video.c
 *
 * This file contains functions for processing video (animation frames) data from a PPM file.
 */


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
