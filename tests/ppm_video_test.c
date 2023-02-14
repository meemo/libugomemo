#include <stdio.h>

#include <libugomemo.h>

#define PPM_THUMB_PIXEL_COUNT  (PPM_THUMBNAIL_HEIGHT * PPM_THUMBNAIL_WIDTH)
#define PPM_THUMB_SIZE_BYTES   (PPM_THUMB_PIXEL_COUNT * 3)

int main(void) {
    FILE *input_file, *output_file;
    uint input_file_size;
    uint i = 0;
    u8 *input_file_contents;
    Rgb24Pixel *output_rgb;

    BmpHeader *bmp;

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
    output_rgb = (Rgb24Pixel *) malloc(PPM_THUMB_SIZE_BYTES);

    /* Decode the thumbnail */
    PPMDecodeThumbnail(input_file_contents, output_rgb);

    /* Create the output BMP file's header. calloc is used so that the other fields are 0. */
    bmp = (BmpHeader *) UGO_CALLOC(1, sizeof(BmpHeader));

    bmp->magic = 0x4D42;
    bmp->file_size = sizeof(BmpHeader) + PPM_THUMB_SIZE_BYTES;
    bmp->data_offset = 0x36;

    bmp->header_size = 0x28;
    bmp->image_width = PPM_THUMBNAIL_WIDTH;
    bmp->image_height = PPM_THUMBNAIL_HEIGHT;
    bmp->color_planes = 1;
    bmp->bits_per_pixel = 24;
    bmp->image_size = PPM_THUMB_SIZE_BYTES;

    /* Write the BMP file */
    fwrite(bmp, sizeof(BmpHeader), 1, output_file);
    fwrite(output_rgb, sizeof(Rgb24Pixel), PPM_THUMB_PIXEL_COUNT, output_file);

    free(bmp);
    fclose(output_file);

    return 0;
}
