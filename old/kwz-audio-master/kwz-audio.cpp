#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

#include "kwz-audio.hpp"

using namespace kwzAudio;

// Source: https://github.com/meemo/kwz-audio
// Compilation: `g++ -o kwz-audio -O3 kwz-audio.cpp`
// Usage: `./kwz-audio [track index] [input file path] [output file path (.wav)]`
// Note: program requires little endian platform (x86, ARM)

void readFile(std::string t_path) {
    std::ifstream file(t_path, std::ios::binary);
    
    if (file) {
        // Stop eating newlines and white space in binary mode
        file.unsetf(std::ios::skipws);

        file.seekg(0, std::ios::end);
        file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        file_buffer.reserve(file_size);
        file_buffer.insert(file_buffer.begin(),
                           std::istream_iterator<uint8_t>(file),
                           std::istream_iterator<uint8_t>());

        file.close();
    }
    else {
        std::cout << "Failed to read file. " << std::endl;
        exit(-1);
    }
}

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
            ksn_offset = offset;
            break;
        }
        offset += getUint32(offset + 4) + 8;
    }
    offset += getUint32(offset - 4);

    // Get track sizes
    bgm_size = getUint32(ksn_offset + 0xC);
    se_1_size = getUint32(ksn_offset + 0x10);
    se_2_size = getUint32(ksn_offset + 0x14);
    se_3_size = getUint32(ksn_offset + 0x18);
    se_4_size = getUint32(ksn_offset + 0x1C);

    // Calculate track offsets from sizes
    bgm_offset = ksn_offset + 0x24;
    se_1_offset = bgm_offset + bgm_size;
    se_2_offset = se_1_offset + se_1_size;
    se_3_offset = se_2_offset + se_2_size;
    se_4_offset = se_3_offset + se_3_size;
}

void decodeTrack(int t_track_length, int t_track_offset) {
    // https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#ksn-sound-data

    int16_t predictor = initial_step_index;
    int16_t step_index = initial_predictor;
    int16_t sample = initial_sample;
    int16_t step = initial_step;
    int16_t diff = initial_diff;
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
            step_index = clampValue(step_index, step_index_clamp_min, step_index_clamp_max);
            predictor = clampValue(predictor, predictor_clamp_min, predictor_clamp_max);
            
            // Add to output buffer
            audio_buffer.push_back(predictor * predictor_scale);
        }
    }
}

int main(int argc, char** argv) {
    int track_index = 0;
    std::string input_file_path = "";
    std::string output_file_path = "";

    // Read arguments
    if (argc > 4) {
        std::cout << "Too many arguments passed!" << std::endl;
        exit(-1);
    }
    else if (argc == 4) {
        track_index = std::atoi(argv[1]);
        input_file_path = argv[2];
        output_file_path = argv[3];
    }
    else {
        std::cout << "Too few paramters! " << std::endl;
        exit(-1);
    }

    readFile(input_file_path);

    // Quickly determine if input file is valid KWZ:
    // KWZ files begin with "KFH" (file header section magic)
    if (file_buffer[0] == 'K') {
        getKSNMeta();

        switch (track_index) {
        case 0: 
            // BGM
            decodeTrack(bgm_size, bgm_offset);
            break;
        case 1: 
            // Sound Effect 1 (A)
            decodeTrack(se_1_size, se_1_offset);
            break;
        case 2: 
            // Sound Effect 2 (X)
            decodeTrack(se_2_size, se_2_offset);
            break;
        case 3: 
            // Sound Effect 3 (Y)
            decodeTrack(se_3_size, se_3_offset);
            break;
        case 4: 
            // Sound Effect 4 (Up)
            decodeTrack(se_4_size, se_4_offset);
            break;
        }

        writeWAV(output_file_path);
    }
    else {
        std::cout << "Invalid file! " << std::endl;
    }
}
