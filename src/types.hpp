#pragma once

#include <cstdint>

typedef uint8_t  u8;
typedef int8_t   s8;

typedef uint16_t u16;
typedef int16_t  s16;

typedef uint32_t u32;
typedef int32_t  s32;

typedef uint64_t u64;
typedef int64_t  s64;

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
    u8 riff_header[4] = { 'R', 'I', 'F', 'F' };
    u32 chunk_size = 0; // Data size + 36
    u8 wave_header[4] = { 'W', 'A', 'V', 'E' };
    // fmt sub chunk
    u8 fmt_header[4] = { 'f', 'm', 't', ' ' };
    u32 subchunk_1_size = 16;
    u16 audio_format = 1; // Audio format, 1=PCM
    u16 num_channels = 1; // Number of channels, 1=Mono
    u32 sample_rate = 16364; // Sampling Frequency in Hz
    u32 byte_rate = 32728; // sample rate * number of channels * bits per sample / 8
    u16 block_align = 2; // 2=16-bit mono
    u16 bits_per_sample = 16; // Bits per sample
    // data sub chunk
    u8 data_header[4] = { 'd', 'a', 't', 'a' };
    u32 subchunk_2_size = 0; // Data size in bytes (*2 for 16 bit)
} wav_hdr;
