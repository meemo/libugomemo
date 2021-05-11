#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <cstdint>

#include "kwz-audio.hpp"
#include "misc.hpp"

void writeWAV(std::string t_path) {
    std::ofstream output_file(t_path, std::ios::binary);
    
    // Generate and write WAV header
    wav_hdr wav;
    wav.chunk_size = (uint32_t)(audio_buffer.size() + 36);
    wav.subchunk_2_size = (uint32_t)(audio_buffer.size() * 2);
    output_file.write(reinterpret_cast<const char*>(&wav), sizeof(wav));

    // Write audio data
    output_file.write(reinterpret_cast<const char*>(&audio_buffer[0]), audio_buffer.size() * 2);

    output_file.close();
}

uint32_t getUint32(int t_start) {
    return *reinterpret_cast<uint32_t*>(file_buffer.data() + t_start);
}

void getKSNMeta() {
    // Find sound section ("KSN" magic) offset
    // Traverse sections using sizes at the end of each header
    int offset = 0;

    while (offset < file_buffer.size()) {
        if (file_buffer[offset + 1] == 'S' && file_buffer[offset + 2] == 'N') {
            break;
        }
        offset += getUint32(offset + 4) + 8;
    }
    offset += getUint32(offset - 4);

    // Get track sizes
    bgm_size = getUint32(offset + 0xC);
    se_1_size = getUint32(offset + 0x10);
    se_2_size = getUint32(offset + 0x14);
    se_3_size = getUint32(offset + 0x18);
    se_4_size = getUint32(offset + 0x1C);

    // Calculate track offsets from sizes
    bgm_offset = offset + 0x24;
    se_1_offset = bgm_offset + bgm_size;
    se_2_offset = se_1_offset + se_1_size;
    se_3_offset = se_2_offset + se_2_size;
    se_4_offset = se_3_offset + se_3_size;
}

std::vector<int16_t> decodeTrack(int t_track_length, int t_track_offset, int step_index) {
    // https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#ksn-sound-data

    std::vector<int16_t> output;

    int16_t predictor = 0;
    int16_t sample = 0;
    int16_t step = 0;
    int16_t diff = 0;
    int track_offset_end = t_track_offset + t_track_length;

    int bit_pos = 0;
    int16_t byte = 0;
    
    for (int buffer_offset = t_track_offset; buffer_offset <= track_offset_end; buffer_offset++) {
        byte = file_buffer.at(buffer_offset);
        bit_pos = 0;

        while (bit_pos < 8) {
            if (step_index < 18 || bit_pos > 4) {
                // 2 bit sample
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
                // 4 bit sample
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
            
            // Add to output buffer
            output.push_back(predictor * 16);
        }
    }

    return output;
}