#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <climits>
#include "kwz.hpp" 

char* readFile(std::string t_file_name) {
    // Read file to a char* buffer
    std::ifstream file(t_file_name, std::ifstream::binary);
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

void writeFile(std::string t_path, char* t_output_buffer, int t_length) {
    // Write a char* buffer to a file
    std::ofstream file(t_path, std::ifstream::binary);
    file.write(t_output_buffer, t_length);
    file.close();
}

void writeWAV(std::string t_file_name) {
    std::ofstream output_file;
    output_file.open(t_file_name, std::ofstream::binary);
    wav_hdr wav;
    wav.chunk_size = audio_buffer_length + 36;
    wav.subchunk_2_size = audio_buffer_length * 2;
    output_file.write(reinterpret_cast<const char*>(&wav), sizeof(wav));
    //output_file.write(reinterpret_cast<const char*>(&audio_buffer), sizeof(audio_buffer));
    for (uint32_t i = 0; i < (uint32_t)audio_buffer_length; i++) {
        output_file.write(reinterpret_cast<const char*>(&audio_buffer[i]), sizeof(audio_buffer[i]));
    }
    output_file.close();
}

char* getSubCharArray(int t_start, int t_end) {
    // Inclusive of end value
    int size = (t_end + 1) - (t_start + 1);
    char* output = new char[size];
    for (int i = t_start; i <= t_end; i++) {
        output[i - t_start] = file_buffer[i];
    }
    return output;
}

std::string getHexString(int t_start, int t_end) {
    // Exclusive of end value
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = t_start; i != t_end; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(file_buffer[i]));
    }
    return ss.str();
}

int findSectionOffset(std::string t_section) {
    int sectionOffset = 0;
    while (std::string(getSubCharArray(sectionOffset, sectionOffset + 2), 3) != t_section) {
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

int16_t clampValue(int16_t t_input, int16_t t_min, int16_t t_max) {
    if (t_input > t_max) {
        return t_max;
    }
    else if (t_input < t_min) {
        return t_min;
    }
    else {
        return t_input;
    }
}

uint8_t get8BitInt(int t_start) {
    return uint8_t(file_buffer[t_start]);
}

uint16_t get16BitInt(int t_start) {
    // Starts at `start` and gets a little endian uint16 (2 bytes)
    return uint16_t(uint8_t(file_buffer[t_start])) |
        uint16_t(uint8_t(file_buffer[t_start + 1])) << 8;
}

uint32_t get32BitInt(int t_start) {
    // Starts at `start` and gets a little endian uint32 (4 bytes)
    return uint32_t(uint8_t(file_buffer[t_start])) |
        uint32_t(uint8_t(file_buffer[t_start + 1])) << 8 |
        uint32_t(uint8_t(file_buffer[t_start + 2])) << 16 |
        uint32_t(uint8_t(file_buffer[t_start + 3])) << 24;
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

int readBits(int t_num_bits) {
    if (bit_index + t_num_bits > 16) {
        // readUint16() would read an uint16 from the compressed layer buffer, 
        // then increment the layer buffer pointer by 2
        uint16_t next_bits = get16BitInt(layer_buffer_pointer);
        layer_buffer_pointer += 2;
        bit_value |= next_bits << (16 - bit_index);
        bit_index -= 16;
    }
    int result = bit_value & ((1 << t_num_bits) - 1);
    bit_value >>= t_num_bits;
    bit_index += t_num_bits;
    return result;
}

void decodeFrame(int t_frame_index) {
    offset = kmi_offset + (t_frame_index * 28);

    int skip_value = 0;
    int x;
    int y;
    int line_index;
    int line_index_a;
    int line_index_b;
    int tile_type;
    uint8_t mask = 0;

    uint8_t a[8] = { 0 };
    uint8_t b[8] = { 0 };

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
    // Not needed until sfx mixing is implemented.
    //struct sfx_flags_struct sfx_flags {
    //    // Is SFX1 used on this frame
    //    (frame_meta.sound_effect_flags & 0x1) != 0,
    //    // Is SFX2 used on this frame
    //    (frame_meta.sound_effect_flags & 0x2) != 0,
    //    // Is SFX3 used on this frame
    //    (frame_meta.sound_effect_flags & 0x4) != 0,
    //    // Is SFX4 used on this frame
    //    (frame_meta.sound_effect_flags & 0x8) != 0
    //};

    // Recurse through all 3 layers
    for (int layer_index = 0; layer_index < 3; layer_index++) {
        for (int large_tile_y = 0; large_tile_y < 240; large_tile_y += 128) {
            for (int large_tile_x = 0; large_tile_x < 320; large_tile_x += 128) {
                for (int tile_y = 0; tile_y < 128; tile_y += 8) {
                    y = large_tile_y + tile_y;
                    // if the tile falls off the bottom of the frame, jump to the next large tile
                    if (y >= 240) break;

                    for (int tile_x = 0; tile_x < 128; tile_x += 8) {
                        x = large_tile_x + tile_x;
                        // if the tile falls off the right of the frame, jump to the next small tile row
                        if (x >= 320) break;
                        // (x, y) is the position of the tile's top-left corner relative to the top-left of the image

                        if (skip_value) {
                            skip_value -= 1;
                            break;
                        }

                        uint8_t tile[8][8] = { 0 };
                        tile_type = readBits(3);

                        switch (tile_type) {
                        case 0:
                            // C++ does not allow clean setting of values in arrays
                            // so we must recurse through each position of the array 
                            // to assign values if we want things to look clean.
                            // For optimization, listing out each value is faster.
                            line_index = line_index_table_common[readBits(5)];
                            for (int i = 0; i < 8; i++) {
                                for (int j = 0; j < 8; j++) {
                                    tile[i][j] = line_table[line_index][j];
                                }
                            }
                            break;
                        case 1:
                            line_index = readBits(13);
                            for (int i = 0; i < 8; i++) {
                                for (int j = 0; j < 8; j++) {
                                    tile[i][j] = line_table[line_index][j];
                                }
                            }
                            break;
                        case 2:
                            line_index = readBits(5);
                            line_index_a = line_index_table_common[line_index];
                            line_index_b = line_index_table_common_shifted[line_index];
                            for (int i = 0; i < 8; i++) {
                                if (i == 0 || (i % 2) == 0) {
                                    for (int j = 0; j < 8; j++) {
                                        tile[i][j] = line_table[line_index_a][j];
                                    }
                                }
                                else {
                                    for (int j = 0; j < 8; j++) {
                                        tile[i][j] = line_table[line_index_b][j];
                                    }
                                }
                            }
                            break;
                        case 3:
                            line_index = readBits(13);
                            line_index_a = line_index_table_common[line_index];
                            line_index_b = line_index_table_common_shifted[line_index];
                            for (int i = 0; i < 8; i++) {
                                if (i == 0 || (i % 2) == 0) {
                                    for (int j = 0; j < 8; j++) {
                                        tile[i][j] = line_table[line_index_a][j];
                                    }
                                }
                                else {
                                    for (int j = 0; j < 8; j++) {
                                        tile[i][j] = line_table[line_index_b][j];
                                    }
                                }
                            }
                            break; 
                        case 4:
                            mask = readBits(8);
                            for (int line = 0; line < 8; line++) {
                                if (mask & (1 << line)) {
                                    line_index = line_index_table_common[readBits(5)];
                                }
                                else {
                                    line_index = readBits(13);
                                }
                                for (int i = 0; i < 8; i++) {
                                    tile[line][i] = line_table[line][i];
                                }
                            }
                            break;
                        case 5:
                            skip_value = readBits(5);
                            continue;
                        case 6:
                            std::cout << "Tile type 6 detected, type is not used." << std::endl;
                            break;
                        case 7:
                            // This entire case could use major optimization in the future
                            // by removing for loops and replacing with individually setting values.
                            uint8_t pattern = readBits(2);
                            uint8_t is_common = readBits(1);

                            if (is_common == 1) {
                                line_index_a = (int) line_index_table_common[readBits(5)];
                                line_index_b = (int) line_index_table_common[readBits(5)];
                                pattern = (pattern + 1) % 4;
                            }
                            else {
                                line_index_a = (int) readBits(13);
                                line_index_b = (int) readBits(13);
                            }

                            for (int i = 0; i < 8; i++) {
                                a[i] = line_table[line_index_a][i];
                            }

                            for (int i = 0; i < 8; i++) {
                                b[i] = line_table[line_index_b][i];
                            }

                            switch (pattern) {
                            case 0:
                                // A B A B A B A B
                                // [0] A
                                tile[0][0] = a[0];
                                tile[0][1] = a[1];
                                tile[0][2] = a[2];
                                tile[0][3] = a[3];
                                tile[0][4] = a[4];
                                tile[0][5] = a[5];
                                tile[0][6] = a[6];
                                tile[0][7] = a[7];
                                // [1] B
                                tile[1][0] = b[0];
                                tile[1][1] = b[1];
                                tile[1][2] = b[2];
                                tile[1][3] = b[3];
                                tile[1][4] = b[4];
                                tile[1][5] = b[5];
                                tile[1][6] = b[6];
                                tile[1][7] = b[7];
                                // [2] A
                                tile[2][2] = a[2];
                                tile[2][1] = a[1];
                                tile[2][2] = a[2];
                                tile[2][3] = a[3];
                                tile[2][4] = a[4];
                                tile[2][5] = a[5];
                                tile[2][6] = a[6];
                                tile[2][7] = a[7];
                                // [3] B
                                tile[3][0] = b[0];
                                tile[3][1] = b[1];
                                tile[3][2] = b[2];
                                tile[3][3] = b[3];
                                tile[3][4] = b[4];
                                tile[3][5] = b[5];
                                tile[3][6] = b[6];
                                tile[3][7] = b[7];
                                // [4] A
                                tile[4][0] = a[0];
                                tile[4][1] = a[1];
                                tile[4][2] = a[2];
                                tile[4][3] = a[3];
                                tile[4][4] = a[4];
                                tile[4][5] = a[5];
                                tile[4][6] = a[6];
                                tile[4][7] = a[7];
                                // [5] B
                                tile[5][0] = b[0];
                                tile[5][1] = b[1];
                                tile[5][2] = b[2];
                                tile[5][3] = b[3];
                                tile[5][4] = b[4];
                                tile[5][5] = b[5];
                                tile[5][6] = b[6];
                                tile[5][7] = b[7];
                                // [6] A
                                tile[6][0] = a[0];
                                tile[6][1] = a[1];
                                tile[6][2] = a[2];
                                tile[6][3] = a[3];
                                tile[6][6] = a[6];
                                tile[6][5] = a[5];
                                tile[6][6] = a[6];
                                tile[6][7] = a[7];
                                // [7] B
                                tile[7][0] = b[0];
                                tile[7][1] = b[1];
                                tile[7][2] = b[2];
                                tile[7][3] = b[3];
                                tile[7][4] = b[4];
                                tile[7][5] = b[5];
                                tile[7][6] = b[6];
                                tile[7][7] = b[7];
                                break;
                            case 1:
                                // A A B A A B A A 
                                for (int j = 0; j < 8; j++) {
                                    tile[0][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[1][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[2][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[3][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[4][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[5][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[6][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[7][j] = a[j];
                                }
                                break;
                            case 2:
                                // A B A A B A A B
                                for (int j = 0; j < 8; j++) {
                                    tile[0][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[1][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[2][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[3][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[4][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[5][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[6][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[7][j] = b[j];
                                }
                                break;
                            case 3:
                                // A B B A B B B A B
                                for (int j = 0; j < 8; j++) {
                                    tile[0][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[1][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[2][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[3][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[4][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[5][j] = b[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[6][j] = a[j];
                                }
                                for (int j = 0; j < 8; j++) {
                                    tile[7][j] = b[j];
                                }
                                break;
                            }
                            break;
                        }
                        // Copy from tile to pixel array
                        for (int line = 0; line < 8; line++) {
                            for (int i = 0; i < 8; i++) {
                                layer_pixels[layer_index][y + line][(int)(x / 8)][i] = tile[line][i];
                            }
                        }
                    }
                }
            }
        }
    }
    // Create output image
    uint8_t output_image[320][240] = { 0 };

    uint32_t layer_depths[3] = { frame_meta.layer_a_depth,
                                 frame_meta.layer_b_depth,
                                 frame_meta.layer_c_depth };
    //std::sort(layer_depths, layer_depths + 3);
    // Sort depths 
    
    // Convert table[8] to a pixel 

    prev_decoded_frame = t_frame_index;
}

void decodeAudioTrack(int t_track_length, int t_track_offset) {
    int output_offset = 0;
    int bit_pos = 0;

    int16_t byte = 0;

    int16_t predictor = initial_predictor;
    int16_t step_index = initial_step_index;
    int16_t sample = initial_sample;
    int16_t step = initial_step;
    int16_t diff = initial_diff;

    for (int track_offset = t_track_offset; track_offset <= (t_track_offset + t_track_length); track_offset++) {
        byte = get8BitInt(track_offset);
        bit_pos = 0;
        while (bit_pos < 8) {
            // 2 bit sample
            if (step_index < variable_threshold || bit_pos > 4) {
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
            // 4 bit sample
            else {
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
            // clamp step index and predictor
            step_index = clampValue(step_index, step_index_clamp_min, step_index_clamp_max);
            predictor = clampValue(predictor, predictor_clamp_min, predictor_clamp_max);

            audio_buffer[output_offset] = predictor * predictor_scale;
            output_offset++;

            audio_buffer_length++;
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
    file_buffer = readFile("C:\\Users\\Meemo\\Desktop\\Projects\\kwz-cpp\\samples\\cmtpkbxgqmxcccc53sztrd5b4aen.kwz");
    
    getSectionOffsets();
    decodeFileHeader();
    generateLineTables();
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

        // Automatically extracting track 0
        decodeAudioTrack(bgm_size, bgm_size + 0x24);
        writeWAV("");
    }
    else {
        std::cout << "File is not a valid KWZ file." << std::endl;
        exit(1);
    }
    std::cout << std::endl << "Total execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << " microseconds." << std::endl;
}