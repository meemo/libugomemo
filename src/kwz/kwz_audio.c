#include <libugomemo.h>

/**
 * kwz_audio.c
 *
 * Functions relating to encoding and decoding KWZ audio, as well as determining correct initial step index values for
 * incorrectly encoded audio (DSi Library, all flipnotes from Flipnote Studio 3D).
 */


/* ========================================== Constants ========================================= */
#define STEP_INDEX_MIN      0
#define STEP_INDEX_MAX     79
#define PREDICTOR_MIN   -2048
#define PREDICTOR_MAX    2047
#define SCALING_FACTOR     16  /* 16 bits per sample in the output. */
#define VARIABLE_THRESHOLD 18
#define INITIAL_PREDICTOR   0

const int ADPCM_INDEX_2BIT[4] = { -1, 2, -1, 2 };

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


/**
 * Decodes KWZ file audio data from a given buffer and position to another buffer as PCM16 with platform native
 * endianness (little endian in the vast majority of cases).
 *
 * Parameters:
 * - file_buffer: The buffer containing KWZ audio data to decode.
 * - audio_buffer: The buffer to store the decoded audio data.
 *      - We cannot predict the size of the decoded audio in advance, so we must allocate a buffer large enough to
 *        store the largest possible decoded audio data buffer; 16364 * 60 * 2 bytes in length.
 * - track_length: The length of the encoded track in bytes.
 * - track_offset: The position in the file buffer to start decoding audio data from.
 * - initial_step_index: The starting step index value used to decode the audio. Must be an integer between 0 and 40.
 *      - The optimal value varies especially on the origin audio, however the values of 0 and 40 are by far the most
 *        common and should be used if you are unsure.
 */
void decodeKWZAudio(const u8  *file_buffer,
                         u16 *audio_buffer,
                         int  len,
                         int  offset,
                         int  initial_step_index) {
    s16 step_index = initial_step_index;
    s16 predictor = INITIAL_PREDICTOR;
    s16 step;
    s16 diff;

    u8 sample;
    u8 byte;

    int bit_pos;
    int output_pos = 0;
    int file_pos;

    for (file_pos = offset; file_pos < offset + len; file_pos++) {
        byte = READ_U8(file_buffer, file_pos);
        bit_pos = 0;

        while (bit_pos < 8) {
            /*
               Determine if the sample to be decoded is 2 or 4 bits.

               If the step index less than the variable threshold (18), the track has been sufficiently
               flat to switch to 2 bit mode, otherwise we assume 4 bit mode.

               If bit_pos is greater than 4 (equal to 6), then there is no way to fit another 4 bit sample,
               so it must be 2 bits.
            */
            if (step_index < VARIABLE_THRESHOLD || bit_pos > 4) {
                sample = byte & 0x3;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step;
                if (sample & 2) diff = -diff;

                predictor += diff;
                step_index += ADPCM_INDEX_2BIT[sample];

                byte >>= 2;
                bit_pos += 2;
            } else {
                sample = byte & 0xF;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step >> 2;
                if (sample & 2) diff += step >> 1;
                if (sample & 4) diff += step;
                if (sample & 8) diff = -diff;

                predictor += diff;
                step_index += ADPCM_INDEX_4BIT[sample];

                byte >>= 4;
                bit_pos += 4;
            }

            step_index = CLAMP(step_index, STEP_INDEX_MIN, STEP_INDEX_MAX);
            predictor  = CLAMP(predictor,  PREDICTOR_MIN,  PREDICTOR_MAX );

            /* Scale the predictor before adding it to the output buffer. */
            audio_buffer[output_pos++] = (s16)predictor * SCALING_FACTOR;
        }
    }
}
