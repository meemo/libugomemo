#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#include "types.hpp"
#include "tables.hpp"
#include "templates.hpp"
#include "kwz_audio.hpp"

/**
 * Decode an audio track from a kwz format flipnote
 *
 * The audio is decoded from the IMA ADPCM-derived variable sample size format that the KWZ format uses
 *
 * For more details see:
 * https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#ksn-sound-data
 *
 * Parameters:
 * - track_size: the size of the track to decode
 * - track_offset: the location in file_buffer where the track is located
 * - initial_step_index: the initial step index to decode the audio track with.
 *   - Use 40 if flipnote is from Flipnote Studio 3D (and Gallery: World) or if you don't know what's correct
 *   - Use findCorrectStepIndex() to find value if flipnote is from DSi Library
 * Returns:
 * - s16 little endian PCM audio in a vector
 */
std::vector<s16> decodeTrack(std::vector<u8> buffer, int start_pos, int initial_step_index) {
    std::vector<s16> output;

    s16 step_index = (s16)initial_step_index;
    s16 predictor = 0;
    s16 step = 0;
    s16 diff = 0;

    u8 sample = 0;
    u8 byte = 0;

    int bit_pos = 0;

    for (int buffer_pos = start_pos; buffer_pos <= (start_pos + (int)buffer.size()); buffer_pos++) {
        byte = buffer[buffer_pos];
        bit_pos = 0;

        while (bit_pos < 8) {
            if (step_index < 18 || bit_pos > 4) {
                // Decode 2 bit sample
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
                // Decode 4 bit sample
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

            // Clamp step index and predictor
            step_index = clampValue(step_index, 0, 79);
            predictor = clampValue(predictor, -2048, 2047);

            // Add to output buffer
            output.push_back(predictor * 16);
        }
    }

    return output;
}

/**
 * Finds the correct initial step index for a given kwz format audio track
 * Intended for use with flipnote Flipnote Hatena that were converted to .kwz for Flipnote Studio 3D's DSi Library
 *
 * Only needs to be called on flipnotes with distorted audio as far as we know, however the difference when run
 * on a normal flipnote is negligible and can be done without worrying about ruining audio.
 *
 * For more details see:
 * https://github.com/meemo/kwz-restoration
 *
 * Parameters:
 * - track_size: the size of the track to process
 * - track_offfset: the position of the start of the track in file_buffer
 *
 * Returns:
 * - the correct initial step index as an int
 */
int findCorrectStepIndex(int track_size, int track_offset) {
    int result = -1;

    if (track_size > 0) {
        double step_index_rms[41] = { 0 };
        double least_rms_value = 100000;  // Higher than highest possible RMS

        // Decode the BGM track using every step index from 0-40 and record the RMS of the decoded samples
        for (int i = 0; i < 41; i++) {
            step_index_rms[i] = findRMS(decodeTrack(track_size, track_offset, i));
        }

        // Find the lowest RMS value recorded, which is the best initial step index
        for (int i = 0; i < 41; i++) {
            if (step_index_rms[i] < least_rms_value) {
                least_rms_value = step_index_rms[i];
                result = i;
            }
        }
    }

    return result;
}
