#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <cmath>

#include "kwz-restoration.hpp"

inline uint32_t getUint16(int pos) {
    return *reinterpret_cast<uint16_t*>(file_buffer.data() + pos);
}

inline uint32_t getUint32(int pos) {
    return *reinterpret_cast<uint32_t*>(file_buffer.data() + pos);
}

inline int16_t clampValue(int16_t value, int min, int max) {
    if (value < min) value = min;
    if (value > max) value = max;
    return value;
}

void readFile(std::string path) {
    std::ifstream file(path, std::ios::binary);

    if (file) {
        // Stop eating newlines and whitespace in binary mode
        file.unsetf(std::ios::skipws);

        file.seekg(0, std::ios::end);
        std::streamoff file_size = file.tellg();
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

void writeWAV(std::string t_path, std::vector<int16_t> t_input) {
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

    // Get the square root of the sum of the squares divided by the number of values squared
    return std::sqrt(rms / (double)input.size());
}

std::vector<int16_t> decodeTrack(int track_size, int track_offset, int step_index) {
    // https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#ksn-sound-data

    std::vector<int16_t> output;

    int16_t predictor = 0;
    int16_t sample = 0;
    int16_t step = 0;
    int16_t diff = 0;

    auto bit_pos = 0;
    int8_t byte = 0;

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

            // Add to output buffer
            output.push_back(predictor * 16);
        }
    }

    return output;
}

bool getKSNMeta() {
    // Find sound section ("KSN " magic) offset by traversing the sections of the file
    // using the section sizes at the end of each section header
    int offset = 0;
    bool ksn_found = false;

    while (offset < (int)file_buffer.size()) {
        if (file_buffer[offset + 1] == 'S' && file_buffer[offset + 2] == 'N') {
            ksn_found = true;
            break;
        }
        else {
            // Add the section size + the size of the section header itself
            offset += getUint32(offset + 4) + 8;
        }
    }

    // Get track sizes
    track_sizes[0] = getUint32(offset + 0x0C);
    track_sizes[1] = getUint32(offset + 0x10);
    track_sizes[2] = getUint32(offset + 0x14);
    track_sizes[3] = getUint32(offset + 0x18);

    // Calculate track offsets from sizes
    track_offsets[0] = offset + 0x24;
    track_offsets[1] = track_offsets[0] + track_sizes[0];
    track_offsets[2] = track_offsets[1] + track_sizes[1];
    track_offsets[3] = track_offsets[2] + track_sizes[2];

    // Verify that the audio section spans to the end of the file
    bool ksn_valid = false;

    if (file_buffer.size() >= offset + getUint16(offset + 0x4)) {
        ksn_valid = true;
    }

    return ksn_valid && ksn_found;
}

int findCorrectStepIndex(int track_size, int track_offset) {
    int result = -1;

    if (track_size > 0) {
        double step_index_rms[41] = { 0 };
        double least_rms_value = 100000;  // Higher than highest possible RMS

        // Decode the BGM track using every step index from 0-40 and record the RMS of the track
        for (int i = 0; i < 41; i++) {
            step_index_rms[i] = findRMS(decodeTrack(track_size, track_offset, i));
        }

        // Find the lowest RMS value recorded, which is the correct step index
        for (int i = 0; i < 41; i++) {
            if (step_index_rms[i] < least_rms_value) {
                least_rms_value = step_index_rms[i];
                result = i;
            }
        }
    }

    return result;
}

int main(int argc, char** argv) {
    if (argc > 4 && argc < 1) {
        std::cout << "Invalid number of arguments passed!" << std::endl;
    }
    else {
        // Read file into file_buffer
        readFile(argv[1]);

        // Check for file header section magic
        if (file_buffer[0] == 'K') {
            // Only get step indexes if the file is valid
            if (getKSNMeta()) {
                // Print all step indexes
                std::cout << "Proper initial step indexes (-1 means no audio in the track): " << std::endl;

                std::cout << "BGM: " << findCorrectStepIndex(track_sizes[0], track_offsets[0]) << std::endl;
                std::cout << "SE1: " << findCorrectStepIndex(track_sizes[1], track_offsets[1]) << std::endl;
                std::cout << "SE2: " << findCorrectStepIndex(track_sizes[2], track_offsets[2]) << std::endl;
                std::cout << "SE3: " << findCorrectStepIndex(track_sizes[3], track_offsets[3]) << std::endl;

                // Write WAV if option passed
                if (argc == 3 || argc == 4) {
                    // Default to BGM if no track index specified
                    int index = 0;

                    // Set track index if specified
                    if (argc == 4) {
                        index = std::stoi(argv[3]);
                    }

                    // Verify index is in the valid range
                    if (index > 3 || index < -1) {
                        std::cout << "Track index is not in the valid range! See README.md" << std::endl;
                    }
                    else {
                        if (track_sizes[index] != 0) {
                            std::cout << "Writing properly decoded BGM track to WAV file" << std::endl;

                            int step_index = findCorrectStepIndex(track_sizes[index], track_offsets[index]);

                            writeWAV(argv[2], decodeTrack(track_sizes[index], track_offsets[index], step_index));
                        }
                        else {
                            std::cout << "The requested track index does not contain audio!" << std::endl;
                        }
                    }
                }
            }
            else {
                std::cout << "File contents are corrupt or incomplete." << std::endl;
            }
        }
        else {
            // No KFH section magic means the file isn't valid.
            std::cout << "File is not a valid .kwz or .kwz.gz file!" << std::endl;
        }
    }
}
