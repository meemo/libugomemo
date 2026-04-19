#include <stdlib.h>
#include <limits.h>

#include <ugomemo.h>

#include <ugomemo/adpcm.h>

// ---- Decoding ----

int ppm_decode_audio(const u8 *file_buffer, i16 *audio_buffer, int *output_size, uint offset, uint len) {
    u8   sample = 0;

    u32  step = 0;
    i32  diff = 0;

    bool low_nibble = true;

    if (len < 4) { *output_size = 0; return UGOMEMO_OK; }
    if (offset > UINT_MAX - len) return UGOMEMO_INPUT_ERROR;

    // First 4 bytes of each track contain initial decoder state:
    // i16 LE initial predictor, u8 initial step index, u8 unknown
    i32  predictor = (i16)read_le16((u8 *)file_buffer, offset);
    i8   step_index = (i8)read_le8((u8 *)file_buffer, offset + 2);
    // file_buffer[offset + 3] is unknown

    if (step_index < PPM_STEP_INDEX_MIN) step_index = PPM_STEP_INDEX_MIN;
    if (step_index > PPM_STEP_INDEX_MAX) step_index = PPM_STEP_INDEX_MAX;

    uint buffer_pos = offset + 4;
    uint output_pos = 0;

    while (buffer_pos < (offset + len)) {
        if (low_nibble) {
            sample = file_buffer[buffer_pos] & 0xF;
        } else {
            sample = file_buffer[buffer_pos++] >> 0x4;
        }

        low_nibble = !low_nibble;

        step = ADPCM_STEP_TABLE[step_index];
        diff = step >> 3;

        if (sample & 1) diff +=  step >> 2;
        if (sample & 2) diff +=  step >> 1;
        if (sample & 4) diff +=  step;
        if (sample & 8) diff  = -diff;

        predictor += diff;
        predictor = UGO_CLAMP(predictor, PPM_PREDICTOR_MIN, PPM_PREDICTOR_MAX);

        step_index += ADPCM_INDEX_TABLE_4BIT[sample];
        step_index = UGO_CLAMP(step_index, PPM_STEP_INDEX_MIN, PPM_STEP_INDEX_MAX);

        audio_buffer[output_pos++] = predictor;
    }

    *output_size = output_pos * sizeof(i16);  // In bytes

    return UGOMEMO_OK;
}

i16 *ppm_decode_track_alloc(ppm_ctx *ctx, int track, u32 *out_sample_count) {
    if (!ctx || !ctx->sound || !ctx->file) return NULL;

    u32 track_offset, track_size;
    switch (track) {
        case 0: track_offset = ctx->sound->bgm_offset; track_size = ctx->sound->bgm_size; break;
        case 1: track_offset = ctx->sound->se1_offset; track_size = ctx->sound->se1_size; break;
        case 2: track_offset = ctx->sound->se2_offset; track_size = ctx->sound->se2_size; break;
        case 3: track_offset = ctx->sound->se3_offset; track_size = ctx->sound->se3_size; break;
        default: return NULL;
    }

    if (track_size == 0) return NULL;

    // Allocate generously (2 samples per byte * 2 bytes per sample)
    i16 *buffer = (i16 *)calloc(track_size * 2, sizeof(i16));
    if (!buffer) return NULL;

    int output_bytes = 0;
    int res = ppm_decode_audio(ctx->file->data, buffer, &output_bytes, track_offset, track_size);
    if (res != UGOMEMO_OK) {
        free(buffer);
        return NULL;
    }

    if (out_sample_count) *out_sample_count = output_bytes / sizeof(i16);
    return buffer;
}

// ---- Encoding ----

// Encode PCM samples to PPM IMA ADPCM format (reversed nibbles).
// Output includes the 4-byte audio header (i16 predictor, u8 step_index, u8 unknown=0).
// Returns the encoded ADPCM data. Caller must free *out_data.
int ppm_encode_audio(const i16 *pcm, int sample_count, u8 **out_data, u32 *out_size) {
    // 4-byte header + ADPCM data (each sample = 4 bits, 2 per byte)
    u32 adpcm_data_size = (sample_count + 1) / 2;
    u32 total_size = 4 + adpcm_data_size;
    u8 *adpcm = (u8 *)calloc(total_size, 1);
    if (!adpcm) return UGOMEMO_MEMORY_ERROR;

    // Write 4-byte header: initial predictor=0, step_index=0, unknown=0
    // (all zeros, which calloc already provides)

    i32 predictor = 0;
    i8 step_index = 0;
    bool low_nibble = true;
    u32 byte_pos = 4;  // start after header

    for (int i = 0; i < sample_count; i++) {
        i32 target = pcm[i];
        i32 diff = target - predictor;
        u8 code = 0;

        // Encode sign
        if (diff < 0) {
            code = 8;
            diff = -diff;
        }

        // Encode magnitude
        i32 step = ADPCM_STEP_TABLE[step_index];
        i32 tempstep = step;

        if (diff >= tempstep) {
            code |= 4;
            diff -= tempstep;
        }
        tempstep >>= 1;

        if (diff >= tempstep) {
            code |= 2;
            diff -= tempstep;
        }
        tempstep >>= 1;

        if (diff >= tempstep) {
            code |= 1;
        }

        // Decode the code (same as decoder) to track predictor state exactly
        i32 decode_diff = step >> 3;
        if (code & 1) decode_diff += step >> 2;
        if (code & 2) decode_diff += step >> 1;
        if (code & 4) decode_diff += step;
        if (code & 8) decode_diff = -decode_diff;

        predictor += decode_diff;
        if (predictor < PPM_PREDICTOR_MIN) predictor = PPM_PREDICTOR_MIN;
        if (predictor > PPM_PREDICTOR_MAX) predictor = PPM_PREDICTOR_MAX;

        step_index += ADPCM_INDEX_TABLE_4BIT[code];
        if (step_index < PPM_STEP_INDEX_MIN) step_index = PPM_STEP_INDEX_MIN;
        if (step_index > PPM_STEP_INDEX_MAX) step_index = PPM_STEP_INDEX_MAX;

        // Pack with reversed nibble order: low nibble first, then high nibble
        if (low_nibble) {
            adpcm[byte_pos] = code & 0xF;
        } else {
            adpcm[byte_pos] |= (code & 0xF) << 4;
            byte_pos++;
        }
        low_nibble = !low_nibble;
    }

    *out_data = adpcm;
    *out_size = total_size;
    return UGOMEMO_OK;
}
