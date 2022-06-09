#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libugomemo.h>

/**
 * sha1_test.c
 *
 * This file tests sha1.c by hashing one of the sample files. More test cases may be added in the future, however this
 * should be sufficient.
 *
 * The file path for the test assumes the executable is being run from the root of the repository.
 *
 * Along with the console output, the program will return code 0 if the test was successful and code 1 if not.
 */


/* ========================================== Constants ========================================= */
const u8 EXPECTED_HASH[20] = { 0x5F, 0xDB, 0xA6, 0x83, 0x7A, 0xF3, 0xF4, 0xA4, 0x16, 0x4D,
                               0xB8, 0xB9, 0x6D, 0x1A, 0x4C, 0xD7, 0x42, 0x8B, 0xA7, 0xFB };
/* ============================================================================================== */


int main(void) {
    FILE *file;
    long file_size;
    int i;
    u8 *file_contents;
    u8 *hash;

    /* Read file into a buffer. */
    file = fopen("tests/samples/cmtpkbxgqmxcccc53sztrd5b4aen.kwz", "rb");
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    file_contents = (u8 *) malloc(file_size + 1);
    fread(file_contents, file_size, sizeof(u8), file);
    fclose(file);

    /* Calculate the hash of the file buffer. */
    hash = (u8 *) malloc(20 * sizeof(u8));
    SHA1(hash, file_contents, file_size);

    /* Print the results. */
    printf("SHA-1 hashing test:\nCalculated hash: ");
    for (i = 0; i < 20; i++) {
        printf("%02x", hash[i]);
    }
    printf("\nExpected hash:   5fdba6837af3f4a4164db8b96d1a4cd7428ba7fb\n");

    /* Evaluate test result. */
    if (!memcmp(hash, EXPECTED_HASH, 20)) {
        printf("Test passed!\n");
        return 0;
    } else {
        printf("Test failed.\n");
        return 1;
    }
}
