#ifndef LIBUGOMEMO_FUNCTIONS_H
#define LIBUGOMEMO_FUNCTIONS_H

#include <types.h>

/* kwz/kwz_audio.c */
void decodeKWZAudio(const u8 *file_buffer, u16 *audio_buffer, int len, int offset, int initial_step_index);

/* kwz/kwz_meta.c */


/* kwz/kwz_video.c */


/* ppm/ppm_audio.c */
void decodePPMAudio(const u8 *file_buffer, u8 *audio_buffer, int len, int offset);

/* ppm/ppm_meta.c */


/* ppm/ppm_video.c */


/* base32.c */


/* base64.c */


/* math.c */
double sqrt_(double x);
double rms_(const s16 *data, unsigned int num_samples);
double stdDev_(const s16 *data, unsigned int num_samples);

/* rsa.c */


/* crc32.c */


/* sha1.c */
void SHA1Init(sha1_ctx *context);
void SHA1Update(sha1_ctx *context, const u8 *data, u32 len);
void SHA1Transform(u32 state[5], const u8 buffer[64]);
void SHA1Final(u8 digest[20], sha1_ctx *context);
void SHA1(u8 *hash_out, const u8 *buffer, unsigned int len);

#endif
