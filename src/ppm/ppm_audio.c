#include <libugomemo.h>

/**
 * ppm_audio.c
 *
 * Functions for decoding audio from a PPM file.
 */


/* ========================================== Constants ========================================= */
#define STEP_INDEX_MIN     0
#define STEP_INDEX_MAX    88
#define PREDICTOR_MIN -32768
#define PREDICTOR_MAX  32767
#define SCALING_FACTOR    16  /* 16 bits per sample in the output. */

const int ADPCM_INDEX_4BIT[16] = { -1, -1, -1, -1, 2, 4, 6, 8,
                                   -1, -1, -1, -1, 2, 4, 6, 8  };

const s16 ADPCM_STEP_TABLE[89] = {     7,     8,     9,    10,    11,    12,
                                      13,    14,    16,    17,    19,    21,
                                      23,    25,    28,    31,    34,    37,
                                      41,    45,    50,    55,    60,    66,
                                      73,    80,    88,    97,   107,   118,
                                     130,   143,   157,   173,   190,   209,
                                     230,   253,   279,   307,   337,   371,
                                     408,   449,   494,   544,   598,   658,
                                     724,   796,   876,   963,  1060,  1166,
                                    1282,  1411,  1552,  1707,  1878,  2066,
                                    2272,  2499,  2749,  3024,  3327,  3660,
                                    4026,  4428,  4871,  5358,  5894,  6484,
                                    7132,  7845,  8630,  9493, 10442, 11487,
                                   12635, 13899, 15289, 16818, 18500, 20350,
                                   22385, 24623, 27086, 29794, 32767         };
/* ============================================================================================== */


int PPMDecodeTrack(const u8   *file_buffer,
                         s16  *audio_buffer,
                         uint  offset,
                         uint  len) {
    u8   sample = 0;

    u32  step = 0;
    s8   step_index = 0;
    s32  diff = 0;
    s32  predictor = 0;

    bool low_nibble = true;

    uint buffer_pos = offset;
    uint output_pos = 0;

    while (buffer_pos < (offset + len)) {
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
        CLAMP(PREDICTOR_MIN, predictor, PREDICTOR_MAX);

        step_index += ADPCM_INDEX_4BIT[sample];
        CLAMP(STEP_INDEX_MIN, step_index, STEP_INDEX_MAX);

        audio_buffer[output_pos++] = predictor;
    }

    return output_pos * 2;
}
