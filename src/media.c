#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <ugomemo.h>

// Read a 24-bit BMP into an rgb24_pixel buffer (top-down, RGB order).
// Returns pixels in top-left origin order (y=0 is top).
int read_bmp_pixels(const char *path, rgb24_pixel *pixels, int expected_w, int expected_h) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        ERROR("Failed to open BMP: %s\n", path);
        return UGOMEMO_INPUT_ERROR;
    }

    bmp_header bmp;
    if (fread(&bmp, sizeof(bmp_header), 1, f) != 1) {
        ERROR("Failed to read BMP header: %s\n", path);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    if (bmp.magic != 0x4D42) {
        ERROR("Not a BMP file: %s\n", path);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    int width = bmp.image_width;
    int height = bmp.image_height;
    bool bottom_up = (height > 0);
    if (height < 0) height = -height;

    if (width != expected_w || height != expected_h) {
        ERROR("BMP dimensions %dx%d don't match expected %dx%d: %s\n",
              width, height, expected_w, expected_h, path);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    if (bmp.bits_per_pixel != 24) {
        ERROR("BMP must be 24-bit: %s (got %d)\n", path, bmp.bits_per_pixel);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    int row_size = width * 3;
    int padding = (4 - (row_size % 4)) % 4;
    u8 *row_buf = (u8 *)malloc(row_size + padding);
    if (!row_buf) { fclose(f); return UGOMEMO_MEMORY_ERROR; }

    fseek(f, bmp.data_offset, SEEK_SET);

    for (int row = 0; row < height; row++) {
        int y = bottom_up ? (height - 1 - row) : row;

        if (fread(row_buf, 1, row_size + padding, f) != (size_t)(row_size + padding)) {
            ERROR("Failed to read BMP row %d: %s\n", row, path);
            free(row_buf);
            fclose(f);
            return UGOMEMO_INPUT_ERROR;
        }

        for (int x = 0; x < width; x++) {
            // BMP stores BGR
            pixels[y * width + x].blue  = row_buf[x * 3 + 0];
            pixels[y * width + x].green = row_buf[x * 3 + 1];
            pixels[y * width + x].red   = row_buf[x * 3 + 2];
        }
    }

    free(row_buf);
    fclose(f);
    return UGOMEMO_OK;
}

// Write an rgb24_pixel buffer as a 24-bit BMP file.
void write_bmp_file(ugomemo_file *output, rgb24_pixel *pixels, int width, int height) {
    bmp_header bmp = {0};
    int row_size = width * 3;
    int padding = (4 - (row_size % 4)) % 4;
    int image_size = (row_size + padding) * height;
    u8 pad[3] = {0};
    u8 *row_buf = (u8 *) calloc(row_size, sizeof(u8));

    bmp.magic = 0x4D42;
    bmp.file_size = sizeof(bmp_header) + image_size;
    bmp.data_offset = sizeof(bmp_header);
    bmp.header_size = 0x28;
    bmp.image_width = width;
    bmp.image_height = height;
    bmp.color_planes = 1;
    bmp.bits_per_pixel = 24;
    bmp.image_size = image_size;

    file_write(output, (u8 *)&bmp, sizeof(bmp_header));

    // BMP is bottom-up and uses BGR order
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            rgb24_pixel p = pixels[y * width + x];
            row_buf[x * 3 + 0] = p.blue;
            row_buf[x * 3 + 1] = p.green;
            row_buf[x * 3 + 2] = p.red;
        }
        file_write(output, row_buf, row_size);
        if (padding > 0) file_write(output, pad, padding);
    }

    free(row_buf);
}

// Read a mono 16-bit PCM WAV file into an i16 buffer.
// Returns the number of samples. Caller must free *samples.
int read_wav_samples(const char *path, i16 **samples, int *sample_count) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        ERROR("Failed to open WAV: %s\n", path);
        return UGOMEMO_INPUT_ERROR;
    }

    wav_header hdr;
    if (fread(&hdr, sizeof(wav_header), 1, f) != 1) {
        ERROR("Failed to read WAV header: %s\n", path);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    if (memcmp(hdr.riff_header, "RIFF", 4) != 0 ||
        memcmp(hdr.wave_header, "WAVE", 4) != 0 ||
        memcmp(hdr.fmt_header, "fmt ", 4) != 0 ||
        memcmp(hdr.data_header, "data", 4) != 0) {
        ERROR("Invalid WAV file: %s\n", path);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    if (hdr.audio_format != 1 || hdr.num_channels != 1 || hdr.bits_per_sample != 16) {
        ERROR("WAV must be mono 16-bit PCM: %s\n", path);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    int count = hdr.subchunk_2_size / sizeof(i16);
    i16 *buf = (i16 *)malloc(hdr.subchunk_2_size);
    if (!buf) { fclose(f); return UGOMEMO_MEMORY_ERROR; }

    if (fread(buf, sizeof(i16), count, f) != (size_t)count) {
        ERROR("Failed to read WAV data: %s\n", path);
        free(buf);
        fclose(f);
        return UGOMEMO_INPUT_ERROR;
    }

    fclose(f);
    *samples = buf;
    *sample_count = count;
    return UGOMEMO_OK;
}

// Count BMP files in a directory matching the pattern *.bmp
int count_bmp_files(const char *dir_path, int *count) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ERROR("Failed to open directory: %s\n", dir_path);
        return UGOMEMO_INPUT_ERROR;
    }

    *count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        size_t len = strlen(entry->d_name);
        if (len >= 4 && strcmp(entry->d_name + len - 4, ".bmp") == 0) {
            (*count)++;
        }
    }

    closedir(dir);
    return UGOMEMO_OK;
}
