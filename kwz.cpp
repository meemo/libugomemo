#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <climits>
#include "kwz.hpp" 

char* readFile(std::string inputFileName) {
    // Read file to a char* buffer
    std::ifstream file(inputFileName, std::ifstream::binary);
    if (file) {
        // get length of file:
        file.seekg(0, file.end);
        file_size = (int)file.tellg();
        file.seekg(0, file.beg);
        // Make buffer and read data
        char* output_buffer = new char[file_size];
        file.read(output_buffer, file_size);
        file.close();
        //std::cout << "File read succesfully." << std::endl << std::endl;
        return output_buffer;
    }
    else {
        std::cout << "File could not be read." << std::endl;
        exit(1);
    }
}

void writeFile(std::string path, char* output_buffer, int length) {
    // Write a char* buffer to a file
    std::ofstream file(path, std::ifstream::binary);
    file.write(output_buffer, length);
    file.close();
}

char* getSubCharArray(int start, int end) {
    // Inclusive of end value
    int size = (end + 1) - (start + 1);
    char* output = new char[size];
    int index = 0;
    for (int i = start; i <= end; i++) {
        output[index] = file_buffer[i];
        index++;
    }
    return output;
}

std::string getHexString(int start, int end) {
    // End value is exclusive
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = start; i != end; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(file_buffer[i]));
    }
    return ss.str();
}

int findSectionOffset(std::string section) {
    int sectionOffset = 0;
    while (std::string(getSubCharArray(sectionOffset, sectionOffset + 2), 3) != section) {
        // Unsure about 8 byte skipping, may skip data.
        sectionOffset += 4;
    }
    return sectionOffset;
}

void getSectionOffsets() {
    // 1MB cutoff for now
    // TODO: find better value
    kfh_offset = 8; // Constant
    if (file_size > 1000000) {
        // Go through the entire file to find each section header
        // Faster on large files
        int section_offset = 0;
        while (section_offset < file_size) {
            std::string section = std::string(getSubCharArray(section_offset, section_offset + 2), 3);
            if (section == "KMI") {
                kmi_offset = section_offset;
            }
            else if (section == "KSN") {
                ksn_offset = section_offset;
            }
            else if (section == "KMC") {
                kmc_offset = section_offset;
            }
            else if (kmi_offset > 0 && ksn_offset > 0 && kmc_offset > 0) {
                // Early break if all are filled, saves a few cycles
                break;
            }
            section_offset += 4;
        }
    }
    else {
        // Go through the file to find each header from the start
        // Faster on small files
        ksn_offset = findSectionOffset("KSN");
        kmi_offset = findSectionOffset("KMI");
        kmc_offset = findSectionOffset("KMC");
    }
}

void generateLineTables() {
    int index = 0;
    for (uint8_t a = 0; a < 3; a++) {
        for (uint8_t b = 0; b < 3; b++) {
            for (uint8_t c = 0; c < 3; c++) {
                for (uint8_t d = 0; d < 3; d++) {
                    for (uint8_t e = 0; e < 3; e++) {
                        for (uint8_t f = 0; f < 3; f++) {
                            for (uint8_t g = 0; g < 3; g++) {
                                for (uint8_t h = 0; h < 3; h++) {
                                    line_table[index][0] = b;
                                    line_table[index][1] = a;
                                    line_table[index][2] = d;
                                    line_table[index][3] = c;
                                    line_table[index][4] = f;
                                    line_table[index][5] = e;
                                    line_table[index][6] = h;
                                    line_table[index][7] = g;
                                    index++;
                                };
                            };
                        };
                    };
                };
            };
        };
    };
    index = 0;
    for (uint8_t a = 0; a < 3; a++) {
        for (uint8_t b = 0; b < 3; b++) {
            for (uint8_t c = 0; c < 3; c++) {
                for (uint8_t d = 0; d < 3; d++) {
                    for (uint8_t e = 0; e < 3; e++) {
                        for (uint8_t f = 0; f < 3; f++) {
                            for (uint8_t g = 0; g < 3; g++) {
                                for (uint8_t h = 0; h < 3; h++) {
                                    line_table_shifted[index][0] = a;
                                    line_table_shifted[index][1] = d;
                                    line_table_shifted[index][2] = c;
                                    line_table_shifted[index][3] = f;
                                    line_table_shifted[index][4] = e;
                                    line_table_shifted[index][5] = h;
                                    line_table_shifted[index][6] = g;
                                    line_table_shifted[index][7] = b;
                                    index++;
                                };
                            };
                        };
                    };
                };
            };
        };
    };
}

void generateSampleTables() {
    for (int sample = 0; sample != 4; sample++) {
        for (int step_index = 0; step_index != 90; step_index++) {
            int step = adpcm_step_table[step_index];
            int diff = step >> 3;
            if (sample & 1) {
                diff += step;
            }
            if (sample & 2) {
                diff = -diff;
            }
            adpcm_sample_table_2b[sample + 4 * step_index] = int16_t(diff);
        }
    }
    for (int sample = 0; sample != 16; sample++) {
        for (int step_index = 0; step_index != 90; step_index++) {
            int step = adpcm_step_table[step_index];
            int diff = step >> 3;
            if (sample & 4) {
                diff += step;
            }
            if (sample & 2) {
                diff += step >> 1;
            }
            if (sample & 8) {
                diff += step >> 2;
            }
            if (sample & 1) {
                diff = -diff;
            }
            adpcm_sample_table_4b[sample + 16 * step_index] = int16_t(diff);
        }
    }
}

uint8_t get8BitInt(int start) {
    return uint8_t(file_buffer[start]);
}

uint16_t get16BitInt(int start) {
    // Starts at `start` and gets a little endian uint16 (2 bytes)
    return uint16_t(uint8_t(file_buffer[start])) |
        uint16_t(uint8_t(file_buffer[start + 1])) << 8;
}

uint32_t get32BitInt(int start) {
    // Starts at `start` and gets a little endian uint32 (4 bytes)
    return uint32_t(uint8_t(file_buffer[start])) |
        uint32_t(uint8_t(file_buffer[start + 1])) << 8 |
        uint32_t(uint8_t(file_buffer[start + 2])) << 16 |
        uint32_t(uint8_t(file_buffer[start + 3])) << 24;
}

void decodeFileHeader() {
    kfh_section_size = get32BitInt(kfh_offset - 4);
    file_creation_timestamp = get32BitInt(kfh_offset + 0x4);
    file_last_edit_timestap = get32BitInt(kfh_offset + 0x8);
    app_version = get32BitInt(kfh_offset + 0xC);
    root_author_ID = getHexString(kfh_offset + 0x10, kfh_offset + 0x19);
    parent_author_ID = getHexString(kfh_offset + 0x1A, kfh_offset + 0x23);
    current_author_ID = getHexString(kfh_offset + 0x24, kfh_offset + 0x2D);
    root_author_name_raw = getSubCharArray(offset + 0x2E, offset + 0x44);
    parent_author_name_raw = getSubCharArray(offset + 0x44, offset + 0x5A);
    current_author_name_raw = getSubCharArray(offset + 0x5A, offset + 0x70);
    root_file_name = std::string(getSubCharArray(kfh_offset + 0x70, kfh_offset + 0x8C), 28);
    parent_file_name = std::string(getSubCharArray(kfh_offset + 0x8C, kfh_offset + 0xA8), 28);
    current_file_name = std::string(getSubCharArray(kfh_offset + 0xA8, kfh_offset + 0xC4), 28);
    frame_count = get16BitInt(kfh_offset + 0xC4);
    thumbnail_frame_index = get16BitInt(kfh_offset + 0xC6);
    framerate = framerates[get8BitInt(kfh_offset + 0xCA)];
    uint16_t raw_kfh_flags = get16BitInt(kfh_offset + 0xC8);
    is_locked = (raw_kfh_flags & 0x1) == 0;
    is_loop_playback = (raw_kfh_flags & 0x2) == 0;
    is_toolset = (raw_kfh_flags & 0x4) == 0;
    uint8_t raw_layer_visibility_flags = get8BitInt(kfh_offset + 0xCB);
    layer_a_invisible = (raw_layer_visibility_flags & 0x1) == 0;
    layer_b_invisible = (raw_layer_visibility_flags & 0x2) == 0;
    layer_c_invisible = (raw_layer_visibility_flags & 0x4) == 0;
}

void decodeSoundHeader() {
    // Flipnote speed when recorded
    flipnote_speed_when_recorded = get32BitInt(ksn_offset + 0x4);
    // BGM size
    bgm_size =  get32BitInt(ksn_offset + 0x8);
    // Sound effect 1 (A) size
    se_1_size = get32BitInt(ksn_offset + 0xC);
    // Sound effect 2 (X) size
    se_2_size = get32BitInt(ksn_offset + 0x10);
    // Sound effect 3 (Y) size
    se_3_size = get32BitInt(ksn_offset + 0x14);
    // Sound effect 4 (up) size
    se_4_size = get32BitInt(ksn_offset + 0x18);
}

int readBits(int num_bits) {
    if (bit_index + num_bits > 16) {
        // readUint16() would read an uint16 from the compressed layer buffer, 
        // then increment the layer buffer pointer by 2
        uint16_t next_bits = get16BitInt(layer_buffer_pointer);
        layer_buffer_pointer += 2;
        bit_value |= next_bits << (16 - bit_index);
        bit_index -= 16;
    }
    int result = bit_value & ((1 << num_bits) - 1);
    bit_value >>= num_bits;
    bit_index += num_bits;
    return result;
}

void decodeFrame(int frame_index) {
    offset = kmi_offset + (frame_index * 28);

    struct frame_meta_struct frame_meta {
        // Flags
        get32BitInt(offset),
        // Layer A size
        get16BitInt(offset + 0x4),
        // Layer B size
        get16BitInt(offset + 0x6),
        // Layer C size
        get16BitInt(offset + 0x8),
        // Frame author ID
        getHexString(offset + 0xA, offset + 0x14),
        // Layer A depth
        get8BitInt(offset + 0x14),
        // Layer B depth
        get8BitInt(offset + 0x15),
        // Layer C depth
        get8BitInt(offset + 0x16),
        // Sound effect flags
        get8BitInt(offset + 0x17),
        // Camera flags
        get16BitInt(offset + 0x1A)
    };
    struct diffing_flags_struct diffing_flags {
        // Paper color index
        frame_meta.flags & 0xF,
        // Layer A diffing flag
        (frame_meta.flags >> 4) & 0x1,
        // Layer A diffing flag
        (frame_meta.flags >> 5) & 0x1,
        // Layer A diffing flag
        (frame_meta.flags >> 6) & 0x1,
        // Is frame based on prev frame
        (frame_meta.flags >> 7) & 0x1,
        // Layer A first color index
        (frame_meta.flags >> 8) & 0xF,
        // Layer A first color index
        (frame_meta.flags >> 12) & 0xF,
        // Layer B first color index
        (frame_meta.flags >> 16) & 0xF,
        // Layer B first color index
        (frame_meta.flags >> 20) & 0xF,
        // Layer C first color index
        (frame_meta.flags >> 24) & 0xF,
        // Layer C first color index
        (frame_meta.flags >> 28) & 0xF
    };
    struct sfx_flags_struct sfx_flags {
        // Is SFX1 used on this frame
        (frame_meta.sound_effect_flags & 0x1) != 0,
        // Is SFX2 used on this frame
        (frame_meta.sound_effect_flags & 0x2) != 0,
        // Is SFX3 used on this frame
        (frame_meta.sound_effect_flags & 0x4) != 0,
        // Is SFX4 used on this frame
        (frame_meta.sound_effect_flags & 0x8) != 0
    };

    // Recurse through all 3 layers
    for (int layer_index = 0; layer_index <= 3; layer_index++) {
        for (int large_tile_y = 0; large_tile_y < 240; large_tile_y += 128) {
            for (int large_tile_x = 0; large_tile_x < 320; large_tile_x += 128) {
                for (int tile_y = 0; tile_y < 128; tile_y += 8) {
                    int y = large_tile_y + tile_y;
                    // if the tile falls off the bottom of the frame, jump to the next large tile
                    if (y >= 240) break;

                    for (int tile_x = 0; tile_x < 128; tile_x += 8) {
                        int x = large_tile_x + tile_x;
                        // if the tile falls off the right of the frame, jump to the next small tile row
                        if (x >= 320) break;

                        // (x, y) is the position of the tile's top-left corner relative to the top-left of the image
                        // Decode frame based on tile type
                        int tile_type = readBits(3);
                        //switch (tile_type) {
                        //case 0:
                        //    int line_index = common_line_index_table[readBits(5)];
                        //    uint8_t a[8][8] = { 0 };
                        //    uint8_t tile[8][8] = { line_table[line_index][0] * 8 };
                        //    // Original:
                        //    // uint8_t tile[8][8] = { a, a, a, a, a, a, a, a };
                        //    // C++ does not allow clean setting of values in arrays, so we must do this.
                        //    for (int a_1 = 0; a_1 < 8; a_1++) {
                        //        for (int b_1 = 0; b_1 < 8; b_1++) {

                        //        }
                        //    }

                        //    
                        //case 1:

                        //case 2:

                        //case 3:

                        //case 4:

                        //case 5:

                        //case 6:
                        //    std::cout << "Tile type 6 detected, type is not used. Exiting." << std::endl;
                        //    exit(6);
                        //case 7:

                        //}
                    }
                }
            }
        }
    }
}

void decodeAudioTrack(int track_index, int track_length, int track_offset) {
    // Need to implement track mixing at some point.
    for (track_offset; track_offset < track_length; track_offset += 1) {
        int byte = file_buffer[track_offset];
        int bit_pos = 0;
        while (bit_pos < 8) {
            if (prev_step_index < 18 || bit_pos == 6) {
                // read 2 - bit sample
                int16_t sample = (byte >> bit_pos) & 0x3;
                // get diff
                int16_t step = adpcm_step_table[prev_step_index];
                int16_t diff = step >> 3;
                if (sample & 1) diff += step;
                if (sample & 2) diff = -diff;
                diff = prev_diff + diff;
                // get step index
                int step_index = prev_step_index + adpcm_index_table_2b[sample];
                bit_pos += 2;
            }
            else {
                // read 4 - bit sample
                int16_t sample = (byte >> bit_pos) & 0xF;
                // get diff
                int16_t step = adpcm_step_table[prev_step_index];
                int16_t diff = step >> 3;
                if (sample & 4) diff += step; 
                if (sample & 2) diff += step >> 1;
                if (sample & 1) diff += step >> 2;
                if (sample & 8) diff = -diff;
                diff = prev_diff + diff;
                // get step index
                int step_index = prev_step_index + adpcm_index_table_4b[sample];
                bit_pos += 4;
                // clamp step index and diff
                step_index = std::max(0, std::min(step_index, 79));
                diff = std::max(-2048, std::min((int) diff, 2047)) * 16;

                output_buffer[output_offset] = (char) diff;
                output_offset += 1;

                // set prev decoder state
                prev_step_index = (int) step_index;
                prev_diff = (int) diff;
            }
        }
    }
}

void extractThumbnail() {
    std::string file_name = "C:\\Users\\user\\Desktop\\kwz-cpp\\output\\" + current_file_name + ".jpg";
    int ktn_offset = findSectionOffset("KTN");
    int start_offset = ktn_offset + 12;
    // Need to verify KMC is always after KTN, or maybe check for smallest unused
    // offset and set that as end?
    int end_offset = kmc_offset;
    writeFile(file_name, getSubCharArray(start_offset, end_offset), end_offset - start_offset);
}

int main() {
    auto start_time = std::chrono::high_resolution_clock::now();
    file_buffer = readFile("flipnote path here");
    getSectionOffsets();
    decodeFileHeader();
    generateLineTables();
    generateSampleTables();
    decodeSoundHeader();

    // Valid KWZ files start with KFH
    if (file_buffer[2] == 'H') {
        std::cout << "Root file name: " << root_file_name << std::endl;
        std::cout << "Parent file name: " << parent_file_name << std::endl;
        std::cout << "Current file name: " << current_file_name << std::endl;
        std::cout << "App version: " << app_version << std::endl;
        std::cout << "File size: " << file_size << std::endl;
        std::cout << "Creation timestamp unconverted: " << file_creation_timestamp << std::endl;
        std::cout << "Last edit timestamp unconverted: " << file_last_edit_timestap << std::endl;
        std::cout << "File size: " << file_size << std::endl;
        std::cout << "Root author ID: " << root_author_ID << std::endl;
        std::cout << "Parent author ID: " << parent_author_ID << std::endl;
        std::cout << "Current author ID: " << current_author_ID << std::endl;
        std::cout << "Frame count: " << frame_count << std::endl;
        std::cout << "Framerate: " << framerate << std::endl;
        std::cout << "Is locked? " << is_locked << std::endl;


    }
    else {
        std::cout << "File is not a valid KWZ file." << std::endl;
        exit(1);
    }
    std::cout << std::endl << "Total execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << " microseconds." << std::endl;
}