#include <libugomemo.h>

/* kwz_audio.c
 *
 * This file contains functions relating to encoding and decoding kwz audio, as well as determining correct starting
 * decoder values in the case of decoding KWZ audio.
 */

/* decodeKWZAudio()
 *
 * Decodes kwz audio from a given buffer and position to a specified buffer.
 *
 * Parameters:
 * - file_buffer: The buffer containing the kwz audio (u8 pointer)
 * - audio_buffer: The buffer to store the decoded audio (u16 pointer)
 *      - We cannot predict the size of the decoded audio in advance, so we must allocate a buffer large enough to
 *        store the decoded audio, which is 16364Hz * 60 seconds bytes in length
 * - track_length: The length of the track in bytes
 * - track_offset: The starting position in the file buffer to start decoding from
 * - step_index: The starting step index value used to decode the audio
 *      - The optimal value varies especially on the origin audio, however the values of 0 and 40 are by far the most
 *        common
 * Returns:
 * - To be implemented: error codes. For now it always returns 0.
 */
int decodeKWZAudio(const u8  *file_buffer,
                         u16 *audio_buffer,
                         int  track_length,
                         int  track_offset,
                         int  step_index) {
    s16 predictor = 0;
    s16 step = 0;
    s16 diff = 0;

    u8 sample = 0;

    u8 byte = 0;

    int bit_pos = 0;
    int audio_buffer_pos = 0;
    int buffer_pos;

    for (buffer_pos = track_offset; buffer_pos < (track_offset + track_length); buffer_pos++) {
        byte = file_buffer[buffer_pos];
        bit_pos = 0;

        while (bit_pos < 8) {
            /* Determine if the sample is 2 or 4 bits.
             *
             * If the step index is under 18, the track has been sufficiently flat to switch to 2 bit mode, otherwise
             * we assume 4 bit mode.
             *
             * If bit_pos is greater than 4 (equal to 6), then there is no way to fit another 4 bit sample, so it must
             * be 2 bits.
             */
            if (step_index < 18 || bit_pos > 4) {
                sample = byte & 0x3;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step;
                if (sample & 2) diff = -diff;

                predictor += diff;
                step_index += ADPCM_INDEX_TABLE_2[sample];

                byte >>= 2;
                bit_pos += 2;
            }
            else {
                sample = byte & 0xF;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step >> 2;
                if (sample & 2) diff += step >> 1;
                if (sample & 4) diff += step;
                if (sample & 8) diff = -diff;

                predictor += diff;
                step_index += ADPCM_INDEX_TABLE_4[sample];

                byte >>= 4;
                bit_pos += 4;
            }

            /* Clamp step index and predictor */
            step_index = CLAMP(step_index, KWZ_AUDIO_STEP_INDEX_MIN, KWZ_AUDIO_STEP_INDEX_MAX);
            predictor  = CLAMP(predictor,  KWZ_AUDIO_PREDICTOR_MIN,  KWZ_AUDIO_PREDICTOR_MAX);

            /* Scale the predictor and add it to the output buffer */
            audio_buffer[audio_buffer_pos++] = (s16)predictor * KWZ_AUDIO_SCALING_FACTOR;
        }
    }

    return 0;
}
