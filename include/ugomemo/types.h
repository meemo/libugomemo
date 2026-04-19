#ifndef UGOMEMO_TYPES_H_
#define UGOMEMO_TYPES_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef uint8_t        u8;
typedef  int8_t        i8;
typedef uint16_t      u16;
typedef  int16_t      i16;
typedef uint32_t      u32;
typedef  int32_t      i32;
typedef uint64_t      u64;
typedef unsigned int uint;

typedef struct __attribute__((packed)) {  // Packing is required
    u8 red;
    u8 green;
    u8 blue;
} rgb24_pixel;

typedef struct __attribute__((packed)) {  // Packing is required
    /* BMP header */
    u16 magic; // 42 4D (watch out for endianness)
    u32 file_size;
    u16 reserved_1; // 0
    u16 reserved_2; // 0
    u32 data_offset;
    /* DIB header (BITMAPINFOHEADER) */
    u32 header_size; // 28
    i32 image_width;
    i32 image_height;
    u16 color_planes; // 1
    u16 bits_per_pixel; // 24
    u32 compression_type; // 0
    u32 image_size;
    i32 x_resolution;
    i32 y_resolution;
    u32 num_colors; // 0
    u32 num_important_colors; // 0
} bmp_header;

typedef struct {
    /* RIFF chunk */
    u8  riff_header[4];
    u32 chunk_size;
    u8  wave_header[4];
    /* fmt sub chunk */
    u8  fmt_header[4];
    u32 subchunk_1_size;
    u16 audio_format;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
    /* data sub chunk */
    u8  data_header[4];
    u32 subchunk_2_size;
} wav_header;

typedef struct {
    FILE *file;
    char *path;
    u8 *data;
    size_t size;
    size_t current_offset;
    bool is_writable;
    bool was_compressed;
    size_t compressed_size;
} ugomemo_file;

// Context for when a file type hasn't been determined yet
typedef struct {
    char *input_file_path;
    char *output_file_path;

    ugomemo_file *file;
    ugomemo_file *output_file;

    bool silence_warnings;
    bool silence_notices;
} ugomemo_ctx;

typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

#endif
