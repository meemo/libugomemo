#ifndef LIBUGOMEMO_FUNCTIONS_H
#define LIBUGOMEMO_FUNCTIONS_H

/* kwz/kwz_audio.c */


/* kwz/kwz_meta.c */


/* kwz/kwz_video.c */


/* ppm/ppm_audio.c */


/* ppm/ppm_meta.c */


/* ppm/ppm_video.c */


/* base32.c */


/* base64.c */


/* io.c */
u8 getBuffer_LE_U8(const char *buffer, int pos);
u16 getBuffer_LE_U16(const char *buffer, int pos);
u32 getBuffer_LE_U32(const char *buffer, int pos);

/* math.c */
double isqrt(double x);
double rms(const s16 *data, int num_samples);
double std_dev(const s16 *data, int num_samples);

/* rsa.c */


/* sha1.c */


/* sha256.c */


#endif
