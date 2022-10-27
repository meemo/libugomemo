#include <libugomemo.h>

/**
 * ppm_video.c
 *
 * This file contains functions for processing video (animation frames) data from a PPM file.
 */


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
