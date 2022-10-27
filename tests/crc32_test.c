#include <stdio.h>

#include <libugomemo.h>

int main(void) {
    FILE *f;
    uint file_size, i;
    u32 crc32;
    u8 *buffer;

    /* Reading the license file for repeatability. */
    f = fopen("LICENSE", "r");
    if (!f) return 1;

    /* Get the file's size. */
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Create the buffer. */
    buffer = (u8 *) UGO_MALLOC(file_size * sizeof(u8));

    /* Read bytes of file into the bufer. */
    for (i = 0; i < file_size; i++) {
        buffer[i] = fgetc(f);
    }

    /* Calculate the CRC32 of the file and print the result. */
    CRC32_calculate(buffer, file_size, &crc32);

    printf("CRC32 of LICENSE: %X\n", crc32);
    printf("Expected        : 7B5D04BC\n");

    /* Returns 1 if successful, 0 if not. */
    return crc32 != (u32) 0x7B5D04BC;
}
