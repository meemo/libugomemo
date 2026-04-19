#ifndef UGOMEMO_FUNCTIONS_H_
#define UGOMEMO_FUNCTIONS_H_

#include <stdio.h>

#include <ugomemo/types.h>
#include <ugomemo/crypto.h>

// io.c
u8 read_le8(u8 *buf, size_t pos);
u16 read_le16(u8 *buf, size_t pos);
u32 read_le32(u8 *buf, size_t pos);
u32 read_be32(u8 *buf, size_t pos);
int decompress_buffer(u8 *input, size_t input_len, u8 **output, size_t *output_len);
int file_init(ugomemo_file **file, char *path, bool writable);
int file_read(ugomemo_file *file);
int file_read_le8(ugomemo_file *f, u8 *res);
int file_read_le16(ugomemo_file *f, u16 *res);
int file_read_le32(ugomemo_file *f, u32 *res);
int file_read_buf(ugomemo_file *f, u8 **res, size_t size);
int file_read_buf_absolute(ugomemo_file *f, u8 **res, size_t offset, size_t size);
int file_test_size(ugomemo_file *f, size_t size);
int file_increment_offset(ugomemo_file *f, size_t amount);
size_t file_get_offset(ugomemo_file *f);
int file_set_offset(ugomemo_file *f, size_t offset);
int file_sha256(ugomemo_file *f, size_t offset, size_t size, u8 *digest);
int file_sha1(ugomemo_file *f, size_t offset, size_t size, u8 *digest);
int file_crc32(ugomemo_file *f, size_t offset, size_t size, u32 *out);
int file_write(ugomemo_file *f, u8 *data, size_t size);
void file_free(ugomemo_file **file);

// crypto.c
void sha256_hash(u8 *data, size_t data_len, u8 *digest);
void sha1_hash(u8 *data, u32 data_len, u8 *digest);
u32 get_crc32(u8 *buffer, uint length);
int rsa_decrypt(const ugomemo_rsa_key *key, const u8 *sig_data, u8 *decrypted_out);
int rsa_encrypt(const ugomemo_rsa_key *key, const u8 *plaintext, u8 *signature_out);
int rsa_verify_signature(const ugomemo_rsa_key *key, const u8 *hash, const u8 *decrypted_signature);
void rsa_create_pkcs1_padding(const ugomemo_rsa_key *key, const u8 *hash, u8 *padded_out);
int rsa_generate_signature(const ugomemo_rsa_key *key, const u8 *file_data, size_t file_size,
                           u8 *signature_out, bool use_sha256);

// kwz_audio.c
uint kwz_decode_track(u8 *file_buffer, i16 *audio_buffer, u32 len, u32 offset, int initial_step_index);
int kwz_get_track(kwz_ctx *ctx, uint offset, u32 track_length, u32 initial_step_index);
void kwz_compute_track_digest(kwz_ctx *ctx, uint track_idx, u32 size, kwz_track_digest *out);
uint find_optimal_step_index_raw(u8 *data, u32 track_len, uint offset, uint *index, double *rms);

// kwz_video.c
int kwz_decode_frame(kwz_ctx *ctx, uint frame_index, rgb24_pixel *output,
                     u8 *prev_layer_a, u8 *prev_layer_b, u8 *prev_layer_c,
                     u8 *layer_a_out, u8 *layer_b_out, u8 *layer_c_out);

// kwz.c
int kwz_decode_fsid(u8 *data, char **fsid_p, char **fsid_ppm_p, bool decode_ppm);
int kwz_decode_filename(u8 *data, char **name);
int kwz_process(kwz_ctx *ctx);
int kwz_verify_file(kwz_ctx *ctx, const ugomemo_rsa_key *key);
void kwz_cleanup(kwz_ctx *ctx);

// kwz.c - convenience API (FFI-friendly)
kwz_ctx *kwz_open(const char *path);
u16  kwz_get_frame_count(const kwz_ctx *ctx);
u8   kwz_get_frame_speed(const kwz_ctx *ctx);
u64  kwz_get_creation_timestamp(const kwz_ctx *ctx);
u64  kwz_get_modified_timestamp(const kwz_ctx *ctx);
u32  kwz_get_flags(const kwz_ctx *ctx);
const char *kwz_get_root_username(const kwz_ctx *ctx);
const char *kwz_get_root_fsid(const kwz_ctx *ctx);
const char *kwz_get_root_filename(const kwz_ctx *ctx);
const char *kwz_get_parent_username(const kwz_ctx *ctx);
const char *kwz_get_parent_fsid(const kwz_ctx *ctx);
const char *kwz_get_parent_filename(const kwz_ctx *ctx);
const char *kwz_get_current_username(const kwz_ctx *ctx);
const char *kwz_get_current_fsid(const kwz_ctx *ctx);
const char *kwz_get_current_filename(const kwz_ctx *ctx);
u32  kwz_get_track_size(const kwz_ctx *ctx, int track);
u32  kwz_get_error_count(const kwz_ctx *ctx);
const char *kwz_get_error(const kwz_ctx *ctx, u32 index, int *severity);
const u8 *kwz_get_file_data(const kwz_ctx *ctx, size_t *size);
bool kwz_get_signature_valid(const kwz_ctx *ctx);

// kwz_video.c - convenience API
rgb24_pixel *kwz_decode_frame_alloc(kwz_ctx *ctx, uint frame_index);

// kwz_video.c - encoding support
u32 kwz_compress_layer(u8 *layer, u8 *out_data, size_t out_capacity,
                       kwz_tile_pos *tile_positions, const i16 *common_reverse);
void kwz_detect_frame_colors(rgb24_pixel *pixels, u8 paper_idx, kwz_frame_colors *out);
void kwz_classify_frame(rgb24_pixel *pixels, const kwz_frame_colors *colors,
                        u8 *layer_a, u8 *layer_b, u8 *layer_c);
void kwz_build_common_reverse_lookup(i16 *reverse);

// kwz_audio.c - convenience API
i16 *kwz_decode_track_alloc(kwz_ctx *ctx, int track, int step_index, u32 *out_sample_count);

// kwz_audio.c - encoding
int kwz_encode_audio(const i16 *pcm, int sample_count, int initial_step_index,
                     u8 **out_data, u32 *out_size);

// kwz_encode.c
int kwz_encode_frames_command(const char *bmp_dir, const char *output_path,
                              const char *reference_kwz_path,
                              const char *bgm_wav,
                              const ugomemo_rsa_key *key);

// ppm_audio.c
int ppm_decode_audio(const u8 *file_buffer, i16 *audio_buffer, int *output_size, uint offset, uint len);
int ppm_encode_audio(const i16 *pcm, int sample_count, u8 **out_data, u32 *out_size);

// ppm_video.c
int ppm_decode_thumbnail(u8 *buffer, rgb24_pixel *output_buffer);
int ppm_framerate_lookup(uint index, float *framerate);
int ppm_decode_frame(ppm_ctx *ctx, uint frame_index, rgb24_pixel *output,
                     u8 *prev_layer_1, u8 *prev_layer_2,
                     u8 *layer_1_out, u8 *layer_2_out);

// ppm_video.c - encoding support
void ppm_classify_frame(rgb24_pixel *pixels, u8 paper_color,
                        u8 layer_1_color, u8 layer_2_color,
                        u8 *layer_1, u8 *layer_2);
int ppm_compress_layer(u8 *layer, u8 *line_encodings,
                       u8 *layer_data, size_t *layer_data_len);
void ppm_pack_line_encodings(u8 *encodings, u8 *packed);
int ppm_encode_frame(u8 *layer_1, u8 *layer_2,
                     u8 paper_color, u8 layer_1_color, u8 layer_2_color,
                     u8 *frame_buf, size_t *frame_len);
int ppm_detect_paper_color(rgb24_pixel *first_frame, u8 *paper_color);
void ppm_detect_frame_colors(rgb24_pixel *pixels, u8 paper_color,
                             u8 *layer_1_color, u8 *layer_2_color);

// ppm.c
int ppm_format_filename(char *dest, u8 *data, u32 offset, u16 *edit_count);
int ppm_read_file(ppm_ctx *ctx);
int ppm_verify_file(ppm_ctx *ctx, const ugomemo_rsa_key *key);
void ppm_cleanup(ppm_ctx **ctx);

// ppm.c - convenience API (FFI-friendly)
ppm_ctx *ppm_open(const char *path);
void ppm_close(ppm_ctx *ctx);
u16  ppm_get_frame_count(const ppm_ctx *ctx);
u8   ppm_get_frame_speed(const ppm_ctx *ctx);
u64  ppm_get_timestamp(const ppm_ctx *ctx);
u16  ppm_get_lock(const ppm_ctx *ctx);
const char *ppm_get_root_username(const ppm_ctx *ctx);
const char *ppm_get_root_fsid(const ppm_ctx *ctx);
const char *ppm_get_parent_username(const ppm_ctx *ctx);
const char *ppm_get_parent_fsid(const ppm_ctx *ctx);
const char *ppm_get_parent_filename(const ppm_ctx *ctx);
const char *ppm_get_current_username(const ppm_ctx *ctx);
const char *ppm_get_current_fsid(const ppm_ctx *ctx);
const char *ppm_get_current_filename(const ppm_ctx *ctx);
u32  ppm_get_track_size(const ppm_ctx *ctx, int track);
u32  ppm_get_error_count(const ppm_ctx *ctx);
const char *ppm_get_error(const ppm_ctx *ctx, u32 index, int *severity);
const u8 *ppm_get_file_data(const ppm_ctx *ctx, size_t *size);
bool ppm_get_signature_valid(const ppm_ctx *ctx);

// ppm_video.c - convenience API
rgb24_pixel *ppm_decode_frame_alloc(ppm_ctx *ctx, uint frame_index);

// ppm_audio.c - convenience API
i16 *ppm_decode_track_alloc(ppm_ctx *ctx, int track, u32 *out_sample_count);

// ppm_encode.c
int ppm_compare_video(const char *ppm_path, const char *bmp_dir,
                      const ugomemo_rsa_key *key);
int ppm_encode_frames_command(const char *bmp_dir, const char *output_path,
                              const char *reference_ppm_path,
                              const char *bgm_wav, const char *se1_wav,
                              const char *se2_wav, const char *se3_wav,
                              const ugomemo_rsa_key *key);

// other_formats.c
typedef struct { u32 section_count; u32 palette_data_length; u32 image_data_length; u8 *menu_data; size_t menu_data_length; u8 *embed_data; size_t embed_data_length; } ugo_file;
#define NBF_WIDTH 256
#define NBF_HEIGHT 192
typedef struct { u32 section_count; u32 palette_length; u32 image_length; u8 *palette_data; u8 *image_data; u16 color_count; } nbf_file;
typedef struct { u32 section_count; u32 palette_length; u32 image_length; u8 *palette_data; u8 *image_data; u16 color_count; int width; int height; } npf_file;
typedef struct { char *url; u8 *data; u32 data_size; bool is_cert; } kwzpcf_entry;
typedef struct { kwzpcf_entry *entries; int entry_count; } kwzpcf_file;

int ugo_parse(u8 *data, size_t size, ugo_file *out);
int nbf_parse(u8 *data, size_t size, nbf_file *out);
int nbf_decode(nbf_file *nbf, rgb24_pixel *output);
int npf_parse(u8 *data, size_t size, npf_file *out);
int npf_decode(npf_file *npf, rgb24_pixel *output);
int fs2d_lst_decrypt(u8 *data, size_t size, u8 **output, size_t *output_len);
int fs3d_lst_decrypt(u8 *data, size_t size, u8 **output, size_t *output_len);
int kwzpcf_parse(u8 *data, size_t size, kwzpcf_file *out);
void kwzpcf_free(kwzpcf_file *f);

// str.c
char *utf16le_to_ascii(u8 *input, u32 input_len);
str2int_errno str2int(int *out, char *s, int base);
void print_hex(const char *label, const u8 *data, size_t len);

#endif
