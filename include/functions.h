#ifndef LIBUGOMEMO_FUNCTIONS_H
#define LIBUGOMEMO_FUNCTIONS_H

#include <types.h>

/* kwz/kwz_audio.c */
void KWZDecodeTrack(const u8 *file_buffer, u16 *audio_buffer, uint len, uint offset, int initial_step_index);

/* kwz/kwz_meta.c */


/* kwz/kwz_video.c */


/* ppm/ppm_audio.c */
int PPMDecodeTrack(const u8 *file_buffer, s16 *audio_buffer, uint offset, uint len);

/* ppm/ppm_meta.c */


/* ppm/ppm_video.c */


/* audio.c */


/* base32.c */


/* base64.c */


/* crc32.c */


/* math.c */
double UGO_SQRT(double x);
double UGO_RMS(const s16 *data, uint num_samples);
double UGO_STDDEV(const s16 *data, uint num_samples);

/* rsa.c */


/* sha1.c */
void SHA1Init(sha1_ctx *context);
void SHA1Update(sha1_ctx *context, const u8 *data, u32 len);
void SHA1Transform(u32 state[5], const u8 buffer[64]);
void SHA1Final(u8 digest[20], sha1_ctx *context);
void SHA1(u8 *hash_out, const u8 *buffer, uint len);

#endif
