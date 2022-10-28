/* SPDX-License-Identifier: Apache-2.0 */

#include <libugomemo.h>

/**
 * kwz_audio.c
 *
 * Functions relating to encoding and decoding KWZ audio, as well as determining correct initial step index values for
 * incorrectly encoded audio (DSi Library, all flipnotes from Flipnote Studio 3D).
 */


/**
 * Decodes KWZ file audio data from a given buffer and position to another buffer as PCM16 with platform native
 * endianness (little endian in the vast majority of cases).
 *
 * Parameters:
 * - file_buffer: The buffer containing KWZ audio data to decode.
 * - audio_buffer: The buffer to store the decoded audio data.
 *      - We cannot predict the size of the decoded audio in advance, so we must allocate a buffer large enough to
 *        store the largest possible decoded audio data buffer; 16364 * 60 * 2 bytes in length.
 * - track_len: The length of the encoded track in bytes.
 * - offset: The position in the file buffer to start decoding audio data from.
 * - initial_step_index: The starting step index value used to decode the audio. Must be an integer between 0 and 40.
 *      - The optimal value varies especially on the origin audio, however the values of 0 and 40 are by far the most
 *        common and should be used if you are unsure.
 */
void KWZDecodeTrack(const u8 *file_buffer, u16 *audio_buffer, uint track_len, uint offset, s16 initial_step_index) {
    s16 step;
    s16 step_index = initial_step_index;
    s16 predictor = KWZ_INITIAL_PREDICTOR;
    s16 diff;
    u8  sample;
    u8  byte;

    uint bit_pos;
    uint output_pos = 0;
    uint buffer_pos = offset;

    while (buffer_pos < offset + track_len) {
        byte = READ_U8(file_buffer, buffer_pos++);
        bit_pos = 0;

        while (bit_pos < 8) {
            /*
               Determine if the sample to be decoded is 2 or 4 bits.

               If the step index less than the variable threshold (18), the track has been sufficiently
               flat to switch to 2 bit mode, otherwise we assume 4 bit mode.

               If bit_pos is greater than 4 (equal to 6), then there is no way to fit another 4 bit sample,
               so it must be 2 bits.
            */
            if (step_index < KWZ_VARIABLE_THRESHOLD || bit_pos > 4) {
                sample = byte & 0x3;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step;
                if (sample & 2) diff = -diff;

                predictor  += diff;
                step_index += ADPCM_INDEX_TABLE_2BIT[sample];

                byte   >>= 2;
                bit_pos += 2;
            } else {
                sample = byte & 0xF;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step >> 2;
                if (sample & 2) diff += step >> 1;
                if (sample & 4) diff += step;
                if (sample & 8) diff = -diff;

                predictor  += diff;
                step_index += ADPCM_INDEX_TABLE_4BIT[sample];

                byte   >>= 4;
                bit_pos += 4;
            }

            CLAMP(KWZ_STEP_INDEX_MIN, step_index, KWZ_STEP_INDEX_MAX);
            CLAMP(KWZ_PREDICTOR_MIN,  predictor,  KWZ_PREDICTOR_MAX );

            /* Scale the predictor before adding it to the output buffer. */
            audio_buffer[output_pos++] = (s16) predictor * KWZ_SCALING_FACTOR;
        }
    }
}
