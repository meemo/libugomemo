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

typedef unsigned int uint;

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

#define UGO_STRUCT_FOOTER  __attribute__((packed))

#elif defined(__WIN32) || defined(__MSVC__)

#pragma pack

#endif

#ifndef UGO_STRUCT_FOOTER
#define UGO_STRUCT_FOOTER
#endif

/* ============================================================
 *                       .kwz File Structs
 * ============================================================
*/


/**
 * kfh_section_header
 *
 * This header is always at the very start of a .kwz file. Only the position of this section and
 * the file sigature can have their positions assumed, all other sections interchange positions.
 *
 * Field descriptions:
 * - magic: The magic for the .kwz file.
 *   - Value = { 'K', 'F', 'H', 0x14 } or, equivalently, { 0x4B, 0x46, 0x48, 0x14 }
 *
 * - size: The size of the KFH section, NOT including itself and the magic (add 8 for entire size).
 *
 * - crc32: The CRC32 checksum of the KFH section after itself.
 *
 * - creation_timestamp: The timestamp of when the .kwz file was created.
 *   - Stored as the number of seconds since January 1st, 2000 at 00:00.
 *
 * - modified_timestamp: The timestamp of when the .kwz file was last modified.
 *
 * - app_version: This seems to be the version of FS3D that created the file.
 *   - Seen as 0, 3, and 4
 *
 * - root_author_id: The Flipnote Studio ID of the root author.
 *   - IDs displayed as lowercase hex, in the format xxxx-xxxx-xxxx-xxxxxx
 *   - The last byte of the IDs are not displayed, and always seen as null.
 *
 * - parent_author_id: The Flipnote Studio ID of the parent author.
 *
 * - current_author_id: The Flipnote Studio ID of the current author.
 *
 * - root_author_name: The username of the root author.
 *   - Names are stored as UTF-16LE encoded strings
 *   - These are stored as signed 16 bit values in the struct because multibyte char processing varies significantly
 *     per implementation.
 *
 * - parent_author_name: The username of the parent author.
 *
 * - current_author_name: The username of the current author.
 *
 * - root_file_name: The name of the root (original) kwz file.
 *   - See kfh_file_name for more information.
 *
 * - parent_file_name: The name of the parent kwz file.
 *
 * - current_file_name: The name of the current kwz file.
 *
 * - frame_count: The number of frames in the flipnote.
 *
 * - thumbnail_frame_index: The index of the frame that is used to create the thumbnail.
 *
 * - flags: Flags for the kfh section, see kfh_flags for more information.
 *
 * - frame_speed: The framerate of the flipnote, stored as an index of the KWZ_FRAMERATES table.
 *
 * - layer_flags: Layer visibility flags for the flipnote, see kfh_layer_flags for more information.
 *
 * No data or padding follows this section.
 */
typedef struct kfh_section_header {
    u8   magic[4];
    u32  size;
    u32  crc32;
    u32  creation_timestamp;
    u32  modified_timestamp;
    u32  app_version;
    u8   root_author_ID[10];
    u8   parent_author_ID[10];
    u8   current_author_ID[10];
    s16  root_author_name[11];
    s16  parent_author_name[11];
    s16  current_author_name[11];
    char root_file_name[28];
    char parent_file_name[28];
    char current_file_name[28];
    u16  frame_count;
    u16  thumbnail_frame_index;
    u16  flags;
    u8   frame_speed;
    u8   layer_flags;
} kfh_section_header;

/**
 * ktn_section_header
 *
 * The KTN section contains the thumbnail of the flipnote, which is just raw JPG data embedded in the file.
 *
 * Field descriptions:
 * - magic: The magic for the KTN section.
 *   - Value = { 'K', 'T', 'N', 0x02 } or, equivalently, { 0x4B, 0x54, 0x4E, 0x02 }
 *
 * - size: The size of the KTN section, NOT including itself and the magic (add 8 for entire size).
 *
 * - crc32: The CRC32 checksum of the thumbnail data.
 *
 * Immediately following this section is the thumbnail data, padded to the nearest multiple of 4 bytes.
 */
typedef struct ktn_section_header {
    u8   magic[4];
    u32  size;
    u32  crc32;
} ktn_section_header;

/**
 * ksn_section_header
 *
 * The KSN section contains the sound track data for the flipnote. Sound data is a customized version of IMA ADPCM
 * with a variable 2 or 4 bit sampling mode, and modified clamping values.
 *
 * Field descriptions:
 * - magic: The magic for the KSN section.
 *   - Value = { 'K', 'S', 'N', 0x01 } or, equivalently, { 0x4B, 0x53, 0x4E, 0x01 }
 *
 * - size: The size of the KSN section, NOT including itself and the magic (add 8 for entire size).
 *
 * - recorded_flipnote_speed: the framerate index of the flipnote when the audio was recorded
 *
 * - *_size: The length in bytes of the audio track, where * is one of the following:
 *   - BGM
 *   - SE1
 *   - SE2
 *   - SE3
 *   - SE4
 *
 * Immediately following the header is the audio track data, in the same order as the track sizes above.
 * This is most likely padded to the nearest multiple of 4 bytes, however I'm not 100% confident about this.
 */
typedef struct ksn_section_header {
    u8  magic[4];
    u32 size;
    u32 recorded_flipnote_speed;
    u32 bgm_size;
    u32 se1_size;
    u32 se2_size;
    u32 se3_size;
    u32 se4_size;
} ksn_section_header;

/* kmc_section_header
 *
 * The KMC section contains all frame data for the flipnote.
 *
 * Field descriptions:
 * - magic: The magic for the KMC section.
 *   - Value = { 'K', 'M', 'C', 0x02 } or, equivalently, { 0x4B, 0x4D, 0x43, 0x02 }
 *
 * - size: The size of the KMC section, NOT including itself and the magic (add 8 for entire size).
 *
 * - crc32: The CRC32 checksum of the entire KMC section after itself.
 *
 * Immediately following this section is the compressed frame data.
 */
typedef struct kmc_section_header {
    u8   magic[4];
    u32  size;
    u32  crc32;
} kmc_section_header;

/* kmi_section_header
 *
 * The KMI section contains metadata entries for all frames in the KMC section. Details of the entries are in kmi_entry.
 *
 * Field descriptions:
 * - magic: The magic for the KMI section.
 *   - Value = { 'K', 'M', 'I', 0x05 } or, equivalently, { 0x4B, 0x4D, 0x49, 0x05 }
 *
 * - size: The size of the KMI section, NOT including itself and the magic (add 8 for entire size).
 *
 * Immediately following this section is each of the 28 byte kmi entires.
 * Data is padded to the nearest multiple of 4 bytes.
 */
typedef struct kmi_section_header {
    u8  magic[4];
    u32 size;
} kmi_section_header;

/* kmi_entry
 *
 * Each KMI entry contains metadata for a single frame.
 *
 * Field descriptions:
 * - flags: Flags for the frame, see kmi_flags for more information.
 *
 * - layer_a_size: The size of the layer A data (in bytes? bits?).
 *
 * - layer_b_size: The size of the layer B data (in bytes? bits?).
 *
 * - layer_c_size: The size of the layer C data (in bytes? bits?).
 *
 * - frame_author_ID: The FSID of the author of the frame.
 *
 * - layer_a_depth: The depth of the 3D effect in layer A.
 *
 * - layer_b_depth: The depth of the 3D effect in layer B.
 *
 * - layer_c_depth: The depth of the 3D effect in layer C.
 *
 * - sound_effect_flags: Flags for the sound effect, see kmi_sfx_flags for more information.
 *
 * - unknown: Unknown value, maybe padding, usually seen as 0.
 *
 * - camera_flags: Flags about whether the camera was used, see kmi_camera_flags for more information.
 */
typedef struct kmi_entry {
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
} kmi_entry;

/* kmi_frame_flags
 *
 * The fields repesent various details about the frame.
 *
 * Field descriptions:
 * - paper_color: The kwz pallete index of the paper's color.
 *   - frame_flags & 0xF
 *
 * - layer_a_diff: The diffing flags for layer A.
 *   - (frame_flags >> 4) & 0x1
 *
 * - layer_b_diff: The diffing flags for layer B.
 *   - (frame_flags >> 5) & 0x1
 *
 * - layer_c_diff: The diffing flags for layer C.
 *  - (frame_flags >> 6) & 0x1
 *
 * - is_prev_frame_based: Whether or not the frame is based on the previous frame.
 *   - (frame_flags >> 7) & 0x1
 *
 * - layer_a_1st_color: The pallete index of the first color of layer A.
 *   - (frame_flags >> 8) & 0xF
 *
 * - layer_a_2nd_color: The pallete index of the second color of layer A.
 *   - (frame_flags >> 12) & 0xF
 *
 * - layer_b_1st_color: The pallete index of the first color of layer B.
 *   - (frame_flags >> 16) & 0xF
 *
 * - layer_b_2nd_color: The pallete index of the second color of layer B.
 *   - (frame_flags >> 20) & 0xF
 *
 * - layer_c_1st_color: The pallete index of the first color of layer C.
 *   - (frame_flags >> 24) & 0xF
 *
 * - layer_c_2nd_color: The pallete index of the second color of layer C.
 *   - (frame_flags >> 28) & 0xF
 */
typedef struct kmi_frame_flags {
    int paper_color;
    int layer_a_diff;
    int layer_b_diff;
    int layer_c_diff;
    int is_prev_frame_based;
    int layer_a_1st_color;
    int layer_a_2nd_color;
    int layer_b_1st_color;
    int layer_b_2nd_color;
    int layer_c_1st_color;
    int layer_c_2nd_color;
} kmi_frame_flags;

/* kmi_sfx_flags
 *
 * Each field indicates whether the specified sound effect is used in the frame.
 *
 * Field descriptions:
 * - se1_used: Whether sound effect 1 was used on this frame
 *   - sfx_flags & 0x1
 *
 * - se2_used: Whether sound effect 2 was used on this frame
 *   - sfx_flags & 0x2
 *
 * - se3_used: Whether sound effect 3 was used on this frame
 *   - sfx_flags & 0x4
 *
 * - se4_used: Whether sound effect 4 was used on this frame
 *   - sfx_flags & 0x8
 */
typedef struct kmi_sfx_flags {
    int se1_used;
    int se2_used;
    int se3_used;
    int se4_used;
} kmi_sfx_flags;

/* kmi_camera_flags
 *
 * Each field indicates whether a photo from the camera feature is preset in the frame on that layer or not.
 *
 * Field descriptions:
 * - layer_a: whether a photo is present in layer A
 *   - camera_flags & 0x1
 *
 * - layer_b: whether a photo is present in layer B
 *  - camera_flags & 0x2
 *
 * - layer_c: whether a photo is present in layer C
 *   - camera_flags & 0x4
 */
typedef struct kmi_camera_flags {
    int layer_a;
    int layer_b;
    int layer_c;
} kmi_camera_flags;

/* kwz_file_name
 *
 * This is the filesystem name of the .kwz file without the .kwz extension, and is stored as a 28 digit ASCII string.
 *
 * File names are encoded in base 32, with the following custom alphabet:
 * - cwmfjordvegbalksnthpyxquiz012345
 *
 * When unpacked, the decoded data contains the following values in order:
 * - Current author FSID
 * - Created timestamp
 * - Modified timestamp
 *
 * The order of created and modified is unknown, however it appears to be correct.
 *
 * Note: in DSi Library flipnotes the timestamps may overflow when converted to unix timestamps, as some users
 * appear to have created flipnotes with timestamps past the 2038 range.
 *
 * Field descriptions:
 * - file_name: The decoded data from a base32 decoded .kwz filename, described above.
 */
typedef struct kwz_file_name {
    u8 *file_name[18];
} kwz_file_name;

/* kwz_signature
 *
 * The signature for a .kwz file is an RSA-2048 SHA-256 signature over the rest of the file INCLUDING PADDING.
 *
 * The DER fromat private and public keys are located in memory and in the extracted .code of the app with no
 * obfuscation of any kind; they are in plaintext.
 *
 * The private key starts with the hex bytes `30 82 04` and ends with `E4 07 50`, spanning 1192 bytes. Its CRC32
 * checksum should match `2CFD3B1C`.
 *
 * The public key starts with the hex bytes `30 82 01` and ends with `01 00 01`, spanning 294 bytes. Its CRC32
 * checksum should match `C0B3153B`.
 *
 * Field descriptions:
 * - signature: The signature of a .kwz file as described above.
 */
typedef struct kwz_signature {
    u8 *signature[256];
} kwz_signature;

/* kwz_file
 *
 * This struct details the sttheructure of a complete .kwz file.
 *
 * All data sections must be padded to the nearest multiple of 4 bytes.
 *
 * Field descriptions:
 * - thumbnail_data: The raw JPG data of the thumbnail.
 *
 * - sound_data: All sound track data in the following order. If there is no data for a certain track, write nothing:
 *  - BGM
 *  - SE1
 *  - SE2
 *  - SE3
 *  - SE4
 *
 * - kmc_data: The raw KMC (frame) data.
 *
 * - kmi_entries: All kmi entries for each frame.
 *
 * Refer to the comments for the listed structs for more information.
 */
typedef struct kwz_file {
    kfh_section_header *kfh;
    ktn_section_header *ktn;
    u8                 *thumnbail_data;
    ksn_section_header *ksn;
    u8                 *sound_data;
    kmc_section_header *kmc;
    u8                 *kmc_data;
    kmi_section_header *kmi;
    kmi_entry          *kmi_entries;
    kwz_signature      *signature;
} kwz_file;

/* kfh_flags
 *
 * These are the flags that are stored in the KFH section.
 *
 * Field descriptions:
 * - lock: if this is set, the flipnote cannot be edited.
 *   - flags & 0x01
 *
 * - loop: whether the flipnote is to loop during playback
 *   - flags & 0x02
 *
 * - toolset: ???
 *   - flags & 0x04
 *
 * - unknown: Perhaps something relating to layer depth?
 *  - flags & 0x10
 */
typedef struct kfh_flags {
    int lock;
    int loop;
    int toolset;
    int unknown;
} kfh_flags;

/* kfh_layer_flags
 *
 * These are the layer visibility flags for the flipnote. If they are set, the layer is visible.
 *
 * Field descriptions:
 * - layer_a_visible: Whether layer A is visible.
 *   - layer_flags & 0x1
 *
 * - layer_b_visible: Whether layer B is visible.
 *   - layer_flags & 0x2
 *
 * - layer_c_visible: Whether layer C is visible.
 *   - layer_flags & 0x4
 */
typedef struct kfh_layer_flags {
    int layer_a_visible;
    int layer_b_visible;
    int layer_c_visible;
} kfh_layer_flags;


/* ============================================================
 *                      .ppm File Structs
 * ============================================================
 */


/* ppm_file_header
 *
 * This is the first section of a .ppm file, spanning exactly 16 (0x10) bytes.
 *
 * Field descriptions:
 * - magic: The magic for a .ppm file.
 *   - Value = { 'P', 'A', 'R', 'A' } or, equivalently, { 0x50, 0x41, 0x52, 0x41 }
 *
 * - animation_data_size: The size of the animation data section.
 *
 * - sound_data_size: The size of the sound data section.
 *
 * - frame_count: The number of frames in the flipnote.
 *
 * - format_version: Assumed to refer to the version of the format, however it is always seen as 0x2400 (LE).
 *   - The app checks `(format_version & 0xf0) >> 4 != 0`, however this is always true.
 */
typedef struct ppm_file_header {
    u8  magic[4];
    u32 animation_data_size;
    u32 sound_data_size;
    u16 frame_count;
    u16 format_version;
} ppm_file_header;

/* ppm_meta
 *
 * This section contains all the metadata for the .ppm file, starting at offset 0x10.
 *
 * Field descriptions:
 * - lock: If this is equal to 1, the flipnote cannot be edited by anyone but the author.
 *
 * - thumbnail_frame_index: The index of the frame used as the thumbnail.
 *
 * - root_author_name: The username of the original (first) author of the flipnote.
 *   - Names are stored as UTF-16LE encoded strings
 *   - These are stored as signed 16 bit values in the struct because multibyte char processing varies significantly
 *     per implementation.
 *
 * - parent_author_name: The username of the author of the parent flipnote.
 *
 * - current_author_name: The username of the current author of the flipnote.
 *
 * - parent_author_id: The FSID of the author of the parent flipnote, which is displayed as hex.
 *
 * - current_author_id: The FSID of the current author of the flipnote, which is displayed as hex.
 *
 * - parent_file_name: The file name of the parent flipnote without the .ppm extension.
 *   - File names as formatted as follows:
 *     - Last 6 digits (3 bytes) of the console's MAC address
 *     - _
 *     - 13 character string
 *     - _
 *     - edit counter, padded to 3 digits with zeros
 *   - e.g. 5DF519_0A8DBDF70A47B_000
 *
 * - current_file_name: The file name of the current flipnote without the .ppm extension.
 *
 * - root_author_id: The FSID of the original (first) author of the flipnote, which is displayed as hex.
 *
 * - root_file_name_fragment: The file name fragment of the original (first) author of the flipnote.
 *   - This contains the following data:
 *     - Last 6 digits (3 bytes) of the console's MAC address
 *     - 10 of the 13 characters in the string from the original name
 *
 * - timestamp: The last modified time of the flipnote.
 *   - Stored as the number of seconds since January 1st, 2000 at 00:00.
 *
 * - unused: Always seen as 0, most likely padding.
 *
 * Immediately following this section is the thumbnail data, spamming 1536 bytes.
 */
typedef struct ppm_meta {
    u16 lock;
    u16 thumbnail_frame_index;
    s16 root_author_name[11];
    s16 parent_author_name[11];
    s16 current_author_name[11];
    u8  parent_author_ID[8];
    u8  current_author_ID[8];
    u8  parent_file_name[18];
    u8  current_file_name[18];
    u8  root_author_ID[8];
    u8  root_file_name_fragment[8];
    u32 timestamp;
    u16 unused;
} ppm_meta;

/* ppm_anim_header
 *
 * The animation header starts at offset 0x6A0.
 *
 * Field descriptions:
 * - offset_table_size: Size of the frame offset table.
 *
 * - unknown: Unknown, always seen as 0.
 *
 * - flags: Flags for the animation header, see ppm_anim_flags for more information
 *
 * Immediately following this header is a table of u32 offsets for the frames, relative to 0x6A0. There are as many of
 * these values as are specified in the frame_count field of ppm_file_header.
 */
typedef struct ppm_anim_header {
    u16 offset_table_size;
    u16 unused;  /* Always seen as null */
    u16 flags;
} ppm_anim_header;


/* ppm_anim_flags
 *
 * These flags are stored in the ppm_anim_header to describe a couple things about the flipnote's behavior.
 *
 * Field descriptions:
 * - unknown_1: Unknown, seen as 0.
 *   - anim_flags & 0x01
 *
 * - loop: Whether the flipnote is to loop during playback.
 *   - anim_flags & 0x02
 *
 * - unknown_2: Unknown, seen as 0.
 *   - anim_flags & 0x04
 *
 * - unknown_3: Unknown, seen as 0.
 *   - anim_flags & 0x08
 *
 * - hide_layer_1: Whether to hide layer 1.
 *   - anim_flags & 0x10
 *
 * - hide_layer_2: Whether to hide layer 2.
 *  - anim_flags & 0x20
 *
 * - unknown_4: Unknown, always seen as 1.
 *   - anim_flags & 0x40
 */
typedef struct ppm_anim_flags {
    int unknown_1;
    int loop;
    int unknown_2;
    int unknown_3;
    int hide_layer_1;
    int hide_layer_2;
    int unknown_4;
} ppm_anim_flags;

/* ppm_sound_header
 *
 * Starting at the value of `0x6A0 + ppm_file_header->animation_data_size` there are the sound effect flags.
 *
 * Starting at the value of `0x6A0 + ppm_file_header->animation_data_size + ppm_file_header->frame_count` is where
 * the sound header is located.
 *
 * Field descriptions:
 * - bgm_size: The size of the BGM audio data in bytes.
 *
 * - se1_size: The size of the sound effect 1 audio data in bytes.
 *
 * - se2_size: The size of the sound effect 2 audio data in bytes.
 *
 * - se3_size: The size of the sound effect 3 audio data in bytes.
 *
 * - frame_playback_speed: The index of the framerate table, PPM_FRAMERATE for playback of the flipnote.
 *   - Frame speed index values are reversed and must be subtracted by 8 to get the actual index.
 *
 * - frame_playback_speed_bgm: The index of the framerate table, PPM_FRAMERATE when the BGM track was recorded.
 *
 * - padding: Padding, always seen as null.
 */
typedef struct ppm_sound_header {
    u32 bgm_size;
    u32 se1_size;
    u32 se2_size;
    u32 se3_size;
    u8  frame_playback_speed;
    u8  frame_playback_speed_bgm;
    u8  padding[14];
} ppm_sound_header;

/* ppm_signature
 *
 * The signature for a .ppm file is an RSA-1024 SHA-1 signature over the rest of the file INCLUDING PADDING. It is
 * located in the very last 144 bytes of the file.
 *
 * Field descriptions:
 * - signature: The signature of a .ppm file as described above.
 */
typedef struct ppm_signature {
    u8 signature[144];
} ppm_signature;

/* ppm_file
 *
 * This struct describes the file structure for the .ppm file format.
 *
 * For more information on each section, see the comments for the structs they refer to above.
 */
typedef struct ppm_file {
    ppm_file_header  *header;
    ppm_meta         *meta;
    u8               *thumbnail_data;
    ppm_anim_header  *animation_header;
    u8               *animation_data;
    u8               *sfx_flags;
    ppm_sound_header *sound_header;
    u8               *sound_data;
    ppm_signature    *signature;
} ppm_file;


/* ============================================================
 *                       Misc Structs
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
typedef struct wav_header {
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

/* wav_file
 *
 * This struct describes a complete .wav file for audio data decoded by this library. This struct can be written
 * directly to a file.
 *
 * Field descriptions:
 * - header: A completed wav_header struct as specified by "Library values" in the comments above.
 *
 * - data: The audio data to be written to the wav file, from one of the audio decoding/processing functions.
 */
typedef struct wav_file {
    wav_header *header;
    s16         data;
} wav_file;

typedef struct bmp_header {
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
} UGO_STRUCT_FOOTER bmp_header;

/* kwz_audio_state
 *
 * This represents the state for an nIMA ADPCM (KWZ audio; modified IMA ADPCM) decoder.
 *
 * This is useful for decoding tracks sample by sample instead of all at once, so that
 * you do not need to pass 8 parameters to the decoder.
 */
typedef struct kwz_audio_state {
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
} kwz_audio_state;

/**
 * An in-progress CRC32 is simply a u32, no need for fancy structs.
 */
typedef u32 crc32_state;

typedef struct rgb24_pixel {
    u8 red;
    u8 green;
    u8 blue;
} rgb24_pixel;

#endif
