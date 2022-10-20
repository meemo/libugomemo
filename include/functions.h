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


/* crc32.c */


/* math.c */
double UGO_SQRT(double x);
double UGO_RMS(const s16 *data, uint num_samples);
double UGO_STDDEV(const s16 *data, uint num_samples);

#endif
