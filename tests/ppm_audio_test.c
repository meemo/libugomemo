#include <stdio.h>
#include <stdlib.h>

#include <libugomemo.h>

int main(void) {
    FILE *input_file;
    long file_size;
    u8 *file_contents;
    ppm_meta meta;

    /* Read file into a buffer. */
    input_file = fopen("tests/samples/cmtpkbxgqmxcccc53sztrd5b4aen.kwz", "rb");
    fseek(input_file, 0, SEEK_END);
    file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    file_contents = (u8 *) malloc(file_size + 1);
    fread(file_contents, file_size, sizeof(u8), input_file);
    fclose(input_file);

    meta = *(ppm_meta *) file_contents;

    printf("Thumbnail frame index: %d\n", meta.thumbnail_frame_index);

    return 0;
}
