#ifndef UGOMEMO_PPM_TYPES_H_
#define UGOMEMO_PPM_TYPES_H_

#include <stdio.h>
#include <stdbool.h>

#include <ugomemo/types.h>
#include <ugomemo/errors.h>

typedef struct {
    u8 *encrypted_signature;
    u8 *decrypted_signature;
    u8 *sha1;
    u8 *sha1_computed;
    u8 *padding_bytes;
    int verify_res;
    bool valid;
} ppm_signature;

typedef struct {
    u8 *bgm;
    u8 *se1;
    u8 *se2;
    u8 *se3;

    u32 bgm_size;
    u32 se1_size;
    u32 se2_size;
    u32 se3_size;

    size_t bgm_offset;
    size_t se1_offset;
    size_t se2_offset;
    size_t se3_offset;

    u8 frame_speed;
    u8 frame_speed_when_recorded;

    // Padding bytes from the sound section header
    u8 *padding_bytes;
    bool has_padding_error;
} ppm_sound;

typedef struct {
    size_t animation_data_size;
    size_t frame_data_size;
    size_t sound_data_size;
    u16 frame_count;
    u16 format_version;

    u16 lock;
    u16 thumbnail_frame_index;
    char *root_username;
    char *parent_username;
    char *current_username;
    char *parent_fsid;
    char *current_fsid;
    char *root_fsid;
    char *parent_file_name;
    char *current_file_name;
    char *root_file_name_fragment;
    u64 timestamp;
    u16 padding;

    u16 frame_offset_table_size;
    u32 frame_offset_table_unknown;
    u16 frame_offset_table_flags;

    // Extra/padding bytes for JSON output
    u8 *animation_header_padding;
    uint animation_header_padding_len;
    u8 *animation_data_padding;
    uint animation_data_padding_len;
    u8 *sfx_flags_padding;
    uint sfx_flags_padding_len;
} ppm_meta;

typedef struct {
    ugomemo_error_list errors;

    ugomemo_file *file;

    ppm_meta *meta;

    u8 *thumbnail_data;
    u8 *frame_offset_table;
    u8 *animation_data;
    u8 *sfx_flags;

    ppm_sound *sound;

    ppm_signature *signature;

    bool silence_warnings;
    bool silence_notices;
} ppm_ctx;

#endif
