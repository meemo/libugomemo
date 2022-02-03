#ifndef LIBUGOMEMO_TYPES_H
#define LIBUGOMEMO_TYPES_H

#include <stdint.h>

typedef uint8_t  u8;

typedef uint16_t u16;
typedef int16_t  s16;

typedef uint32_t u32;

typedef struct KFH {
    /* char magic[4] = { 'K', 'F', 'H', 0x69 }; The final value's purpose is unknown */
    u32  size;   /* Does not include magic and itself, from crc32 to the end */
    u32  crc32;  /* CRC32 checksum of the rest of the section after itself (size + until size - 12) */
    u32  creation_timestamp;  /* Epoch is January 1st, 2000 */
    u32  modified_timestamp;
    u32  app_version;
    u8   root_author_ID[10];
    u8   parent_author_ID[10];
    u8   current_author_ID[10];
    s16  root_author_username[11];  /* Names are stored in little endian wchars (UTF-16 LE) */
    s16  parent_author_username[11];
    s16  current_author_username[11];
    char root_file_name[28];
    char parent_file_name[28];
    char current_file_name[28];
    u16  frame_count;
    u16  thumbnail_frame_index;
    u16  flags;
    u8   frame_speed;
    u8   layer_flags;
} KFH;

typedef struct KTN {
    /* char magic[4] = { 'K', 'F', 'H', 0x69 };  The final value seems to be an unused flag */
    u32  size;   /* Does not include magic and itself, from crc32 to the end */
    u32  crc32;  /* CRC32 checksum of the rest of the section after itself (size + until size - 12) */
    u8  *thumbnail;  /* JPG file data padding up to the nearest multiple of 4 in length. */
} KTN;

typedef struct KSN {
    /* char magic[4] = { 'K', 'S', 'N', 0x69 };  The final value seems to be an unused flag */
    u32  size;   /* Does not include magic and itself, from flipnote speed to the end */
    u32  recorded_flipnote_speed;  /* The flipnote's speed when recorded */
    u32  bgm_size;
    u32  se1_size;
    u32  se2_size;
    u32  se3_size;
    u32  se4_size;
} KSN;

typedef struct KMC {
    /* char magic[4] = { 'K', 'M', 'C', 0x00 };  The final value seems to be an unused flag */
    u32  size;   /* Does not include magic and itself, from crc32 to the end */
    u32  crc32;  /* CRC32 checksum of the rest of the section after itself (size + until size - 12) */
    u8  *frame_data;  /* Padded to the nearest multiple of 4? */
} KMC;

typedef struct KMI_ENTRY {
    u32  flags;
    u16  layer_a_size;
    u16  layer_b_size;
    u16  layer_c_size;
    u8   frame_author_ID[10];  /* ID is displayed as hex */
    u8   layer_a_depth;
    u8   layer_b_depth;
    u8   layer_c_depth;
    u8   sound_effect_flags;
    /* u16  unknown; */
    u16  camera_flags;
} KMI_ENTRY;

typedef struct KMI {
    /* char     magic[4] = { 'K', 'M', 'I', 0x00 };  The final value seems to be an unused flag */
    u32         size;   /* Does not include magic and itself, from the KMI entries to the end */
    struct KMI *KMI_entries;  /* Each frame gets an entry */
} KMI;

typedef struct PPM_HEADER {
    /* char magic[4] = { 'P', 'A', 'R', 'A' }; */
    u32  animation_data_size;
    u32  sound_data_size;
    u16  frame_count;
    /* u16  format_version = 0x24; This is always 0x24 */
} PPM_HEADER;

typedef struct PPM_META {
    u16  lock;  /* 0 if unlocked, 1 if locked */
    u16  thumbnail_frame_index;
    s16  root_author_name[11];
    s16  parent_author_name[11];
    s16  current_author_name[11];
    u8   parent_author_ID[8];
    u8   current_author_ID[8];
    u8   parent_file_name[18];
    u8   current_file_name[18];
    u8   root_author_ID[8];
    u8   root_file_name_fragment[8];
    u32  timestamp;   /* Epoch is Januray 1st, 2000 */
    /* u16  unused = 0;  Always null */
} PPM_META;

typedef struct PPM_ANIMATION_HEADER {
    u16 frame_offset_table_size;
    /* u16 unknown;  Always seen as null */
    u16 flags;
} PPM_ANIMATION_HEADER;

typedef struct PPM_SOUND_HEADER {
    u32 bgm_size;
    u32 se1_size;
    u32 se2_size;
    u32 se3_size;
    u8  frame_playback_speed;
    u8  frame_playback_speed_bgm;
    /* u8  padding[14] = { 0 };  Always null */
} PPM_SOUND_HEADER;

/* A wav header preconfirgured for mono signed 16 bit PCM audio (as produced by decoding functions)
 *
 * Only 4 values need to be set before the header is ready to be written:
 * - chunk_size: set to the audio buffer size + 36
 * - subchunk_2_size: set to the audio buffer size * 2
 * - sample_rate: set depending on audio's sample rate:
 *      - KWZ: 16364 (NOT 16384)
 *      - PPM: 8192
 * - byte_rate: set as sample rate * 2 (simplified process due to known values):
 *      - KWZ: 32728 (NOT 32768)
 *      - PPM: 16384
 *
 * Write the header then the audio buffer to write a complete wav file
 */
typedef struct wav_header {
    /* RIFF chunk */
    u8  riff_header[4]; /* = { 'R', 'I', 'F', 'F' }; */
    u32 chunk_size; /* = 0;       Data size + 36 */
    u8  wave_header[4]; /* = { 'W', 'A', 'V', 'E' }; */
    /* fmt sub chunk */
    u8  fmt_header[4]; /* = { 'f', 'm', 't', ' ' }; */
    u32 subchunk_1_size; /* = 16; */
    u16 audio_format; /* = 1;     Audio format, 1=PCM */
    u16 num_channels; /* = 1;     Number of channels, 1=Mono */
    u32 sample_rate; /* = 0;      Sampling Frequency in Hz */
    u32 byte_rate; /* = 0;        sample rate * number of channels * bits per sample / 8 */
    u16 block_align; /* = 2;      2=16 bit mono */
    u16 bits_per_sample; /* = 16; Bits per sample */
    /* data sub chunk */
    u8  data_header[4]; /* = { 'd', 'a', 't', 'a' }; */
    u32 subchunk_2_size; /* = 0;  Data size in bytes (*2 for 16 bit) */
} wav_hdr;

#endif
