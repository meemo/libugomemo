#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#include "kwz.hpp"
#include "audio.hpp"

s16 clampValue(s16 value, int min, int max) {
    if (value < min) value = min;
    if (value > max) value = max;
    return value;
}

void writeWAV(std::string t_path, std::vector<s16> t_input) {
    std::ofstream output_file(t_path, std::ios::binary);

    // Generate and write WAV header
    wav_hdr wav;
    wav.chunk_size = (uint32_t)(t_input.size() + 36);
    wav.subchunk_2_size = (uint32_t)(t_input.size() * 2);
    output_file.write(reinterpret_cast<const char*>(&wav), sizeof(wav));

    // Write audio data
    output_file.write(reinterpret_cast<const char*>(&t_input[0]), t_input.size() * 2);

    output_file.close();
}

double findRMS(std::vector<int16_t> input) {
    double rms = 0.0;

    // Square each value and add them together
    for (auto i = 0; i < (int)input.size(); i++) {
        rms += input[i] * input[i];
    }

    // The square root of the sum of the squares divided by the number of values
    return std::sqrt(rms / (double)input.size());
}

std::vector<s16> decodeTrack(int track_size, int track_offset, int step_index) {
    // https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#ksn-sound-data

    std::vector<s16> output;

    s16 predictor = 0;
    s16 sample = 0;
    s16 step = 0;
    s16 diff = 0;

    auto bit_pos = 0;
    u8 byte = 0;

    for (auto buffer_pos = track_offset; buffer_pos <= (track_offset + track_size); buffer_pos++) {
        byte = file_buffer[buffer_pos];
        bit_pos = 0;

        while (bit_pos < 8) {
            if (step_index < 18 || bit_pos > 4) {
                // Decode 2 bit sample
                sample = byte & 0x3;

                step = adpcm_step_table[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step;
                if (sample & 2) diff = -diff;

                predictor += diff;
                step_index += adpcm_index_table_2_bit[sample];

                byte >>= 2;
                bit_pos += 2;
            }
            else {
                // Decode 4 bit sample
                sample = byte & 0xF;

                step = adpcm_step_table[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step >> 2;
                if (sample & 2) diff += step >> 1;
                if (sample & 4) diff += step;
                if (sample & 8) diff = -diff;

                predictor += diff;
                step_index += adpcm_index_table_4_bit[sample];

                byte >>= 4;
                bit_pos += 4;
            }

            // Clamp step index and predictor
            step_index = clampValue(step_index, 0, 79);
            predictor = clampValue(predictor, -2048, 2047);

            // Scale sample and add to output buffer
            output.push_back(predictor * 16);
        }
    }

    return output;
}
