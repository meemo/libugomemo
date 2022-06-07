#include <stdlib.h>
#include <stdio.h>

#include <libugomemo.h>

int main(void) {
    FILE *file;
    long file_size;
    u8 *file_contents;
    u8 *hash;
    int i;

    file = fopen("tests/samples/cmtpkbxgqmxcccc53sztrd5b4aen.kwz", "rb");

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    file_contents = (u8 *) malloc(file_size + 1);

    fread(file_contents, file_size, sizeof(u8), file);

    fclose(file);

    hash = (u8 *) malloc(20 * sizeof(u8));
    SHA1(hash, file_contents, file_size);

    printf("%ld\n", file_size);

    for (i = 0; i < 20; i++) {
        printf("%02x", hash[i]);
    }

    printf("\n");

    return 0;
}
