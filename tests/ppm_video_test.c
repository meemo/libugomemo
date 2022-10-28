#include <stdio.h>

#include <libugomemo.h>

#define PPM_THUMB_SIZE_BYTES  (PPM_THUMBNAIL_HEIGHT * PPM_THUMBNAIL_WIDTH * 3)

int main(void) {
    FILE *input_file, *output_file;
    uint input_file_size;
    uint i = 0;
    u8 *input_file_contents;
    rgb24_pixel *output_rgb;

    bmp_header *bmp;

    /* Open files */
    input_file = fopen("tests/samples/U19FA7_1307576D91807_000.ppm", "r");
    output_file = fopen("tests/U19FA7_1307576D91807_000.bmp", "w");
    if (!output_file || !input_file) return 2;

    /* Read input file */
    fseek(input_file, 0, SEEK_END);
    input_file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    input_file_contents = (u8 *) malloc(input_file_size * sizeof(u8));

    while (i < input_file_size) {
        input_file_contents[i++] = fgetc(input_file);
    }

    fclose(input_file);

    /* Prep buffer for decoding */
    output_rgb = (rgb24_pixel *) malloc(PPM_THUMB_SIZE_BYTES);

    /* Decode the thumbnail */
    PPMDecodeThumbnail(input_file_contents, output_rgb);

    /* Create the headers for the output BMP file */
    bmp = (bmp_header *) calloc(1, sizeof(bmp_header));

    bmp->magic = 0x4D42;
    bmp->file_size = sizeof(bmp_header) + PPM_THUMB_SIZE_BYTES;
    bmp->data_offset = 0x36;

    bmp->header_size = 0x28;
    bmp->image_width = PPM_THUMBNAIL_WIDTH;
    bmp->image_height = PPM_THUMBNAIL_HEIGHT;
    bmp->color_planes = 1;
    bmp->bits_per_pixel = 24;
    bmp->image_size = PPM_THUMB_SIZE_BYTES;

    /* Write the BMP file */
    fwrite(bmp, sizeof(bmp_header), 1, output_file);
    fwrite(output_rgb, sizeof(rgb24_pixel), PPM_THUMBNAIL_HEIGHT * PPM_THUMBNAIL_WIDTH, output_file);

    fclose(output_file);

    return 0;
}
