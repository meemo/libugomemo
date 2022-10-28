/* SPDX-License-Identifier: Apache-2.0 */

#include <libugomemo.h>

/**
 * ppm_audio.c
 *
 * Functions for decoding audio from a PPM file.
 */


int PPMDecodeTrack(const u8 *file_buffer, s16 *audio_buffer, uint track_len, uint offset) {
    u8   sample = 0;

    u32  step = 0;
    s8   step_index = 0;
    s32  diff = 0;
    s32  predictor = 0;

    bool low_nibble = true;

    uint buffer_pos = offset;
    uint output_pos = 0;

    while (buffer_pos < (offset + track_len)) {
        if (low_nibble) {
            sample = file_buffer[buffer_pos] & 0xF;
        } else {
            sample = file_buffer[buffer_pos++] >> 0x4;
        }

        low_nibble = !low_nibble;

        step = ADPCM_STEP_TABLE[step_index];
        diff = step >> 3;

        if (sample & 1) diff +=  step >> 2;
        if (sample & 2) diff +=  step >> 1;
        if (sample & 4) diff +=  step;
        if (sample & 8) diff  = -diff;

        predictor += diff;
        CLAMP(PPM_PREDICTOR_MIN, predictor, PPM_PREDICTOR_MAX);

        step_index += ADPCM_INDEX_TABLE_4BIT[sample];
        CLAMP(PPM_STEP_INDEX_MIN, step_index, PPM_STEP_INDEX_MAX);

        audio_buffer[output_pos++] = predictor;
    }

    return output_pos * 2;
}
