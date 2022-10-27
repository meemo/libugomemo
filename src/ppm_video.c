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
    u8 tile_x, tile_y, x, y;

    for (tile_y = 0; tile_y < PPM_THUMBNAIL_HEIGHT; tile_y += 8) {
        for (tile_x = 0; tile_x < PPM_THUMBNAIL_WIDTH; tile_x += 8) {
            for (y = 0; y < 8; y++) {
                for (x = 0; x < 8; x += 2) {
                    memcpy(buffer + (((tile_y + y) * 64 ) + (tile_x + x)),     PPM_THUMBNAIL_PALETTE[*buffer >> 0x4], 3);
                    memcpy(buffer + (((tile_y + y) * 64 ) + (tile_x + x + 1)), PPM_THUMBNAIL_PALETTE[*buffer  & 0xF], 3);

                    output_buffer++;
                }
            }
        }
    }
}
