#ifndef UGOMEMO_MEDIA_H_
#define UGOMEMO_MEDIA_H_

#include <ugomemo/types.h>

// BMP reading and writing utilities shared by commands and encoders

int read_bmp_pixels(const char *path, rgb24_pixel *pixels, int expected_w, int expected_h);
void write_bmp_file(ugomemo_file *output, rgb24_pixel *pixels, int width, int height);

// WAV reading
int read_wav_samples(const char *path, i16 **samples, int *sample_count);

// Directory scanning
int count_bmp_files(const char *dir_path, int *count);

#endif
