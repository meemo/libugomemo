#pragma once

/*
 * A wav header preconfirgured for signed 16 bit PCM audio.
 *
 * Only 2 values need to be set before the header is ready to be written:
 * - chunk_size: audio buffer size + 36
 * - subchunk_2_size: audio buffer size * 2
 *
 * Write the header then write each byte of the audio buffer to write a complete wav file.
 */
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

/*
 * Clamp value between min and max
 *
 * Parameters:
 * - <type>: the type of the input and output
 * - value: the value to clamp
 * - min: the min of the clamp
 * - max: the max of the clamp
 *
 * Returns:
 * <type> value of the clamp
 */
template<typename T>
T clampValue(T value, int min, int max) {
    if (value < min) value = (T)min;
    if (value > max) value = (T)max;
    return value;
}

// Define functions
void writeWAV(std::string t_path, std::vector<s16> t_input);

double findRMS(std::vector<int16_t> input);

std::vector<s16> decodeTrack(int track_size, int track_offset, int step_index);

int findCorrectStepIndex(int track_size, int track_offset);
