#pragma once

template <typename T>
T clampValue(T value, T min, T max) {
    if (value < min) value = min;
    if (value > max) value = max;
    return value;
}

void readFile(std::string t_path);

void writeWAV(std::string t_path);

uint32_t getUint32(int t_start);

void getKSNMeta();

void decodeTrack(int t_track_length, int t_track_offset);

namespace kwzAudio {
    std::vector<uint8_t> file_buffer;
    std::streamoff file_size;

    std::vector<int16_t> audio_buffer;

    int ksn_offset = 0;

    int bgm_size = 0;
    int bgm_offset = 0;

    int se_1_size = 0;
    int se_1_offset = 0;

    int se_2_size = 0;
    int se_2_offset = 0;

    int se_3_size = 0;
    int se_3_offset = 0;

    int se_4_size = 0;
    int se_4_offset = 0;

    // Initial Decoder State
    const int16_t initial_step_index = 0;
    const int16_t initial_predictor = 0;
    const int16_t initial_sample = 0;
    const int16_t initial_step = 0;
    const int16_t initial_diff = 0;

    // Clamping and Scale
    const int16_t step_index_clamp_min = 0;
    const int16_t step_index_clamp_max = 79;
    const int16_t predictor_clamp_min = -2048;
    const int16_t predictor_clamp_max = 2047;
    const int16_t predictor_scale = 16;

    // ADPCM Tables
    const int adpcm_index_table_2_bit[4] = { -1, 2, -1, 2 };

    const int adpcm_index_table_4_bit[16] = { -1, -1, -1, -1, 2, 4, 6, 8,
                                              -1, -1, -1, -1, 2, 4, 6, 8 };

    const int16_t adpcm_step_table[89] = { 7,     8,     9,    10,    11,    12,
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
                                       22385, 24623, 27086, 29794, 32767 };

    typedef struct wav_header {
        // RIFF chunk
        uint8_t riff_header[4] = { 'R', 'I', 'F', 'F' };
        uint32_t chunk_size = 0; // Data size + 36
        uint8_t wave_header[4] = { 'W', 'A', 'V', 'E' };
        // fmt sub chunk
        uint8_t fmt_header[4] = { 'f', 'm', 't', ' ' };
        uint32_t subchunk_1_size = 16;
        uint16_t audio_format = 1; // Audio format, 1=PCM
        uint16_t num_channels = 1; // Number of channels, 1=Mono
        uint32_t sample_rate = 16364; // Sampling Frequency in Hz
        uint32_t byte_rate = 32728; // sample rate * number of channels * bits per sample / 8
        uint16_t block_align = 2; // 2=16-bit mono
        uint16_t bits_per_sample = 16; // Bits per sample
        // data sub chunk
        uint8_t data_header[4] = { 'd', 'a', 't', 'a' };
        uint32_t subchunk_2_size = 0; // Data size in bytes (*2 for 16 bit)
    } wav_hdr;
}
