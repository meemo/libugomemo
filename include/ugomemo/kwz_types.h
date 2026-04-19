#ifndef UGOMEMO_KWZ_TYPES_H_
#define UGOMEMO_KWZ_TYPES_H_

#include <stdio.h>
#include <stdbool.h>

#include <ugomemo/types.h>
#include <ugomemo/constants.h>
#include <ugomemo/errors.h>

typedef struct {
    u32 crc32;
    u64 creation_timestamp;
    u64 modified_timestamp;
    u32 app_version;

    char *root_fsid;
    char *root_fsid_ppm;
    char *root_username;
    char *root_filename;

    char *parent_fsid;
    char *parent_fsid_ppm;
    char *parent_username;
    char *parent_filename;

    char *current_fsid;
    char *current_fsid_ppm;
    char *current_username;
    char *current_filename;

    u16 frame_count;
    u16 thumbnail_frame_index;
    u16 flags;
    u8 frame_speed;
    u8 layer_vis_flags;

    size_t kfh_offset;
    size_t kfh_size;

    size_t ktn_offset;
    size_t ktn_size;

    size_t ksn_offset;
    size_t ksn_size;

    size_t kmi_offset;
    size_t kmi_size;

    size_t kmc_offset;
    size_t kmc_size;

    // CRC32 values for each section
    u32 kfh_crc32_calculated;
    u32 ktn_crc32;
    u32 ktn_crc32_calculated;
    u32 ksn_crc32;
    u32 ksn_crc32_calculated;
    u32 kmc_crc32;
    u32 kmc_crc32_calculated;

    // SHA256 values for each section
    u8 ktn_sha256[SHA256_SIZE];
    u8 kmc_sha256[SHA256_SIZE];
    u8 kmi_sha256[SHA256_SIZE];

    // Extra bytes (padding) for sections that have them
    u8 *ksn_extra_bytes;
    uint ksn_extra_bytes_len;
    u8 *kmc_extra_bytes;
    uint kmc_extra_bytes_len;
    u8 *kmi_extra_bytes;
    uint kmi_extra_bytes_len;
} kwz_meta;

typedef struct {
    u32 recorded_speed;

    u8 *bgm;
    u32 bgm_size;
    size_t bgm_offset;

    u8 *se1;
    u32 se1_size;
    size_t se1_offset;

    u8 *se2;
    u32 se2_size;
    size_t se2_offset;

    u8 *se3;
    u32 se3_size;
    size_t se3_offset;

    u8 *se4;
    u32 se4_size;
    size_t se4_offset;

    i16 *audio_buffer;
    u32 processed_audio_len;
} kwz_sound;

typedef struct {
    u32 flags;
    u16 layer_a_size;
    u16 layer_b_size;
    u16 layer_c_size;
    char *fsid;
    u8 layer_a_depth;
    u8 layer_b_depth;
    u8 layer_c_depth;
    u8 sfx_flags;
    u16 unknown;
    u16 camera_flags;
} kwz_kmi_entry;

typedef struct {
    kwz_kmi_entry *kmi;
} kwz_video;

typedef struct {
    u32 encoded_size;
    uint samples;
    uint best_step_index;
    double best_step_index_rms;
    u8 digest[SHA256_SIZE];
    bool has_data;
} kwz_track_digest;

// Per-frame color layout for encoding: maps each palette index to a (layer, value) pair.
// layer: 0=A, 1=B, 2=C.  value: 1=color1, 2=color2.
// Unassigned colors get layer=-1.
typedef struct {
    u8 paper_idx;
    u8 la_c1, la_c2;
    u8 lb_c1, lb_c2;
    u8 lc_c1, lc_c2;
    i8 color_layer[7];
    u8 color_value[7];
} kwz_frame_colors;

typedef struct {
    ugomemo_error_list errors;

    ugomemo_file *file;

    kwz_meta *meta;
    kwz_sound *sound;
    kwz_video *video;

    u8 *ktn_data;
    u8 *kmc_data;

    bool kfh_parsed;
    bool ktn_parsed;
    bool ksn_parsed;
    bool kmi_parsed;
    bool kmc_parsed;

    u8 *decrypted_signature;
    u8 *signature_sha256;

    u8 file_sha256[SHA256_SIZE];

    bool silence_warnings;
    bool silence_notices;
} kwz_ctx;

#endif
