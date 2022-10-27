#ifndef LIBUGOMEMO_FUNCTIONS_H_
#define LIBUGOMEMO_FUNCTIONS_H_

#include <types.h>

/* kwz_audio.c */
void KWZDecodeTrack(const u8 *file_buffer, u16 *audio_buffer, uint track_len, uint offset, s16 initial_step_index);

/* kwz_meta.c */


/* kwz_video.c */


/* ppm_audio.c */
int PPMDecodeTrack(const u8 *file_buffer, s16 *audio_buffer, uint track_len, uint offset);

/* ppm_meta.c */


/* ppm_video.c */
void PPMDecodeThumbnail(const u8 *buffer, u8 *output_buffer);

/* audio.c */


/* crc32.c */
void CRC32_init(crc32_state *state);
void CRC32_update(crc32_state *state, u8 input_byte);
void CRC32_finish(crc32_state *state, u32 *crc32_output);
void CRC32_calculate(u8 *buffer, uint buffer_len, u32 *crc32_output);

/* math.c */
double UGO_SQRT(double x);
double UGO_RMS(const s16 *data, uint num_samples);
double UGO_STDDEV(const s16 *data, uint num_samples);

#endif
