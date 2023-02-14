/* SPDX-License-Identifier: Apache-2.0 */

#ifndef LIBUGOMEMO_TYPES_H_
#define LIBUGOMEMO_TYPES_H_

#include <stdint.h>

#include <libugomemo_config.h>

typedef uint8_t  u8;
typedef int8_t   s8;

typedef uint16_t u16;
typedef int16_t  s16;

typedef uint32_t u32;
typedef int32_t  s32;

typedef char c8;
typedef u16 c16;

typedef u32 uint;

/* ANSI C boolean */
typedef enum { false = 0, true = !false } bool;


/**
 * types.h
 *
 * This file contains structs that describes the file formats and structures within those files, for use by the library.
 *
 * This file also functions the primary source of documentation for the file formats at the moment, but that may change
 * in the future.
 *
 * Each struct has the file extension as the prefix, and is pre-typdef'd to the struct name to allow for easier usage.
 * i.e. all .kwz file structs start with `kwz_` and all .ppm file structs start with `ppm_`, etc.
 *
 * The exception to this naming scheme is for the .kwz format, where the struct prefixes are the section's magic.
 *
 * All values in these files are stored as little endian where applicable.
 */


/**
 * Compilers try to be smart and pad structs to word lengths in order to increase access speed, however we're using them
 * to extract and write bytes from buffers. As a result, we have to do some trickery to ensure this doesn't happen.
 *
 * This is confirmed to happen with at least BMP headers, but I don't feel like checking everything for word alignment so
 * this will affect everything.
 *
 * gcc and clang support the __attribute__((packed)) attribute, so we can use that. MSVC uses #pragma pack, so in that case
 * we have to use that. In any other cases we just have to pray that there's no padding going on.
 */
#if defined(__GNUC__) || defined(__clang__)

#define UGO_PACK_STRUCT  __attribute__((packed))

#elif defined(__WIN32) || defined(__MSVC__)

#pragma pack

#endif

#ifndef UGO_PACK_STRUCT
#define UGO_PACK_STRUCT
#endif

/* ============================================================
 *                       .kwz File Structs
 * ============================================================
*/

typedef struct KfhFlags {
    bool lock    : 1;
    bool loop    : 1;
    bool toolset : 1;
    u16          : 13;
} KfhFlags;

typedef struct KfhSection {
    u8  magic[4];
    u32 size;
    u32 crc32;
    u32 creation_timestamp;
    u32 modified_timestamp;
    u32 app_version;
    u8  root_author_ID[10];
    u8  parent_author_ID[10];
    u8  current_author_ID[10];
    c16 root_author_name[11];
    c16 parent_author_name[11];
    c16 current_author_name[11];
    c8  root_file_name[28];
    c8  parent_file_name[28];
    c8  current_file_name[28];
    u16 frame_count;
    u16 thumbnail_frame_index;
    KfhFlags flags;
    u8  frame_speed;
    u8  layer_flags;
} KfhSection;

typedef struct KtnSectionHeader {
    u8   magic[4];
    u32  size;
    u32  crc32;
} KtnSectionHeader;

typedef struct KsnSectionHeader {
    u8  magic[4];
    u32 size;
    u32 recorded_flipnote_speed;
    u32 bgm_size;
    u32 se1_size;
    u32 se2_size;
    u32 se3_size;
    u32 se4_size;
} KsnSectionHeader;

typedef struct KmcSectionHeader {
    u8   magic[4];
    u32  size;
    u32  crc32;
} KmcSectionHeader;

typedef struct KmiSectionHeader {
    u8  magic[4];
    u32 size;
} KmiSectionHeader;

typedef struct KmiEntry {
    u32 flags;
    u16 layer_a_size;
    u16 layer_b_size;
    u16 layer_c_size;
    u8  frame_author_ID[10];
    u8  layer_a_depth;
    u8  layer_b_depth;
    u8  layer_c_depth;
    u8  sound_effect_flags;
    u16 unknown;
    u16 camera_flags;
} KmiEntry;

typedef struct KmiFrameFlags {
    u16  paper_color     : 0xF;
    u8   layer_a_diff    :   1;
    u8   layer_b_diff    :   1;
    u8   layer_c_diff    :   1;
    bool is_derived      :   1;
    u16  layer_a_color_1 : 0xF;
    u16  layer_a_color_2 : 0xF;
    u16  layer_b_color_1 : 0xF;
    u16  layer_b_color_2 : 0xF;
    u16  layer_c_color_1 : 0xF;
    u16  layer_c_color_2 : 0xF;
} KmiFrameFlags;

typedef struct KmiSfxFlags {
    bool se1_used : 1;
    bool se2_used : 1;
    bool se3_used : 1;
    bool se4_used : 1;
    u8            : 4;
} KmiSfxFlags;

typedef struct KmiCameraFlags {
    bool layer_a : 1;
    bool layer_b : 1;
    bool layer_c : 1;
    u8           : 5;
} KmiCameraFlags;

typedef struct KwzFileName {
    u8 file_name[18];
} KwzFileName;

typedef struct KwzSignature {
    u8 signature[256];
} KwzSignature;

typedef struct KfhLayerFlags {
    bool layer_a_visible;
    bool layer_b_visible;
    bool layer_c_visible;
} KfhLayerFlags;


/* ============================================================
 *                      .ppm File Structs
 * ============================================================
 */

typedef struct PpmFileHeader {
    u8  magic[4];
    u32 animation_data_size;
    u32 sound_data_size;
    u16 frame_count;
    u16 format_version;
} PpmFileHeader;

typedef struct PpmMeta {
    u16 lock;
    u16 thumbnail_frame_index;
    c16 root_author_name[11];
    c16 parent_author_name[11];
    c16 current_author_name[11];
    u8  parent_author_ID[8];
    u8  current_author_ID[8];
    u8  parent_file_name[18];
    u8  current_file_name[18];
    u8  root_author_ID[8];
    u8  root_file_name_fragment[8];
    u32 timestamp;
    u16 padding;
} PpmMeta;

typedef struct PpmFrameHeader {
	u8 paper_color   : 1;
	u8 layer_1_color : 2;
	u8 layer_2_color : 2;
	u8 do_move       : 2;
	u8 is_key_frame  : 1;
} PpmFrameHeader;

typedef struct PpmAnimHeader {
    u16 offset_table_size;
    u32 unused;  /* Always seen as null */
    u16 flags;
} PpmAnimHeader;

typedef struct PpmAnimFlags {
    int unknown_1;
    int loop;
    int unknown_2;
    int unknown_3;
    int hide_layer_1;
    int hide_layer_2;
    int unknown_4;
} UGO_PACK_STRUCT PpmAnimFlags;

typedef struct PpmSoundHeader {
    u32 bgm_size;
    u32 se1_size;
    u32 se2_size;
    u32 se3_size;
    u8  frame_playback_speed;
    u8  frame_playback_speed_bgm;
    u8  padding[14];
} PpmSoundHeader;

typedef struct PpmFileFooter {
    u8 signature[144];
    u8 padding [16];
} PpmFileFooter;


/* ============================================================
 *                              Misc
 * ============================================================
 */


/* wav_header
 *
 * Field descriptions:
 * - riff_header: The magic of a wav file
 *   - Value = { 'R', 'I', 'F', 'F' } or, equivalently, { 0x52, 0x49, 0x46, 0x46 }
 *
 * - chunk_size: The size of the wav file in bytes (size of the audio data + 36)
 *
 * - wave_header: The wave header of the wav file
 *   - Value = { 'W', 'A', 'V', 'E' } or, equivalently, { 0x57, 0x41, 0x56, 0x45 }
 *
 * - fmt_header: The fmt header of the wav file
 *   - Value = { 'f', 'm', 't', ' ' } or, equivalently, { 0x66, 0x6D, 0x74, 0x20 }
 *
 * - subchunk_2_size: The size of the fmt header in bytes
 *   - This will always be 16 for this struct
 *
 * - audio_format: The audio format of the wav file
 *   - 1 = PCM
 *
 * - num_channels: The number of audio channels in the wav file
 *   - 1 = Mono
 *   - 2 = Stereo
 *
 * - sample_rate: The sample rate of the audio data in Hz
 *
 * - byte_rate: The byte rate of the audio data
 *   - Formula: (sample_rate * num_channels * bits per sample) / 8
 *
 * - block_align: The block align of the audio data
 *   - Value = 2 for 16 bit audio
 *
 * - bits_per_sample: The number of bits per sample
 *   - Value = 2 for 16 bit audio
 *
 * - data_header: The data header of the wav file
 *   - Value = { 'd', 'a', 't', 'a' } or, equivalently, { 0x64, 0x61, 0x74, 0x61 }
 *
 * - subchunk_2_size: The size of the audio data in bytes
 *   - Value = number of samples * 2 for 16 bit audio
 *
 * Library values (what to populate this struct with for audio produced by the library):
 * - riff_header: { 'R', 'I', 'F', 'F' } or, equivalently, { 0x52, 0x49, 0x46, 0x46 }
 * - chunk_size: size of the audio data + 36
 * - wave_header: { 'W', 'A', 'V', 'E' } or, equivalently, { 0x57, 0x41, 0x56, 0x45 }
 * - fmt_header: { 'f', 'm', 't', ' ' } or, equivalently, { 0x66, 0x6D, 0x74, 0x20 }
 * - subchunk_2_size: 16
 * - audio_format: 1
 * - num_channels: 1
 * - sample_rate:
 *   - KWZ: 16364 (NOT 16384)
 *   - PPM: 8192
 *   - If upsampled, change to whatever that new value is
 * - byte_rate:
 *   - sample_rate * 2
 * - block_align: 2
 * - bits_per_sample: 16
 * - data_header: { 'd', 'a', 't', 'a' } or, equivalently, { 0x64, 0x61, 0x74, 0x61 }
 * - subchunk_2_size: size of the audio data
 *
 * Immediately after this header will be the audio data.
 */
typedef struct WavHeader {
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
} WavHeader;

/* Note: This header gets mangled by compilers padding structs. */
typedef struct BmpHeader {
    /* BMP header */
    u16 magic; /* 42 4D (watch out for endianness) */
    u32 file_size;
    u16 reserved_1; /* 0 */
    u16 reserved_2; /* 0 */
    u32 data_offset;
    /* DIB header (BITMAPINFOHEADER) */
    u32 header_size; /* 28 */
    s32 image_width;
    s32 image_height;
    u16 color_planes; /* 1 */
    u16 bits_per_pixel; /* 24 */
    u32 compression_type; /* 0 */
    u32 image_size;
    s32 x_resolution;
    s32 y_resolution;
    u32 num_colors; /* 0 */
    u32 num_important_colors; /* 0 */
} UGO_PACK_STRUCT BmpHeader;

/* kwz_audio_state
 *
 * This represents the state for an nIMA ADPCM (KWZ audio; modified IMA ADPCM) decoder.
 *
 * This is useful for decoding tracks sample by sample instead of all at once, so that
 * you do not need to pass 8 parameters to the decoder.
 */
typedef struct KwzAudio_ctx {
    /* Input and output locations */
    const u8 *file_buffer;
    uint track_len;
    uint track_offset;
    /* Decoder state */
    s16 step_index;
    s16 predictor;
    s16 step;
    s16 diff;
    u8 sample;
    u8 byte;
    /* The position in the file buffer. */
    uint file_pos;
    uint bit_pos;
    uint output_pos;
} KwzAudio_ctx;

/* RGB24Pixel
 *
 * It is what it says on the tin.
 */
typedef struct Rgb24Pixel {
    u8 red;
    u8 green;
    u8 blue;
} UGO_PACK_STRUCT Rgb24Pixel;

/* crc32_state
 *
 * An in-progress CRC32 is simply a u32, no need for fancy structs.
 */
typedef u32 CRC32_ctx;

#endif
