#include <stdio.h>

#include <libugomemo.h>

#define PPM_THUMB_SIZE_BYTES  (PPM_THUMBNAIL_HEIGHT * PPM_THUMBNAIL_WIDTH * 3)

int main(void) {
    FILE *input_file, *output_file;
    uint input_file_size;
    uint i = 0;
    u8 *input_file_contents, *output_rgb;

    bmp_header *bmp;
    dib_header *dib;

    /* Open files */
    input_file = fopen("tests/samples/U19FA7_1307576D91807_000.ppm", "r");
    output_file = fopen("tests/U19FA7_1307576D91807_000.bmp", "w");
    if (!output_file || !input_file) return 2;

    /* Read input file */
    fseek(input_file, 0, SEEK_END);
    input_file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    input_file_contents = (u8 *) malloc(input_file_size * sizeof(u8));

    while (i < input_file_contents) {
        input_file_contents[i++] = fgetc(input_file);
    }

    fclose(input_file);

    /* Prep buffer for decoding */
    output_rgb = (u8 *) malloc(PPM_THUMB_SIZE_BYTES * sizeof(u8));

    /* Decode the thumbnail */
    PPMDecodeThumbnail(input_file_contents, output_rgb);

    /* Create the headers for the output BMP file */
    bmp = (bmp_header *) calloc(1, sizeof(bmp_header));
    dib = (dib_header *) calloc(1, sizeof(dib_header));

    bmp->magic[0] = 0x42;
    bmp->magic[1] = 0x4D;
    bmp->file_size = sizeof(bmp_header) + sizeof(dib_header) + PPM_THUMB_SIZE_BYTES;
    bmp->data_offset = sizeof(bmp_header) + sizeof(dib_header);

    dib->header_size = 12;
    dib->image_width = PPM_THUMBNAIL_WIDTH;
    dib->image_height = PPM_THUMBNAIL_HEIGHT;
    dib->color_planes = 1;
    dib->bits_per_pixel = 24;

    /* Write the final BMP file */
    fwrite(bmp, sizeof(bmp_header), 1, output_file);
    fwrite(dib, sizeof(dib_header), 1, output_file);
    fwrite(output_rgb, sizeof(u8), PPM_THUMB_SIZE_BYTES, output_file);

    fclose(output_file);

    return 0;
}
