#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <ugomemo.h>

#include <ugomemo/adpcm.h>

uint kwz_decode_track(u8 *file_buffer, i16 *audio_buffer, u32 len, u32 offset, int initial_step_index) {
    i16 step_index = initial_step_index;
    i16 predictor = KWZ_INITIAL_PREDICTOR;
    i16 step, diff;

    u8  sample;
    u8  byte;

    uint bit_pos;
    uint output_pos = 0;
    uint file_pos;

    for (file_pos = offset; file_pos < offset + len; file_pos++) {
        byte = file_buffer[file_pos];
        bit_pos = 0;

        while (bit_pos < 8) {
            if (step_index < KWZ_VARIABLE_THRESHOLD || bit_pos > 4) {
                sample = byte & 0x3;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step;
                if (sample & 2) diff = -diff;

                predictor  += diff;
                step_index += ADPCM_INDEX_TABLE_2BIT[sample];

                byte   >>= 2;
                bit_pos += 2;
            } else {
                sample = byte & 0xF;

                step = ADPCM_STEP_TABLE[step_index];
                diff = step >> 3;

                if (sample & 1) diff += step >> 2;
                if (sample & 2) diff += step >> 1;
                if (sample & 4) diff += step;
                if (sample & 8) diff = -diff;

                predictor  += diff;
                step_index += ADPCM_INDEX_TABLE_4BIT[sample];

                byte   >>= 4;
                bit_pos += 4;
            }

            step_index = UGO_CLAMP(step_index, KWZ_STEP_INDEX_MIN, KWZ_STEP_INDEX_MAX);
            predictor = UGO_CLAMP(predictor, KWZ_PREDICTOR_MIN, KWZ_PREDICTOR_MAX);

            audio_buffer[output_pos++] = (i16) predictor * KWZ_SCALING_FACTOR;
        }
    }

    return output_pos;
}

static inline uint least_rms(double *step_index_rms) {
    uint result = 0;
    double least_rms_value = DBL_MAX;

    for (uint i = 0; i <= 40; i++) {
        if (step_index_rms[i] < least_rms_value) {
            least_rms_value = step_index_rms[i];
            result = i;
        }
    }

    return result;
}

static inline double track_rms(i16 *audio_buffer, u32 buffer_size) {
    double sum_squares = 0.0;

    for (uint i = 0; i < buffer_size; i++) {
        sum_squares += (double)audio_buffer[i] * audio_buffer[i];
    }

    return sum_squares / (double)buffer_size;
}

uint find_optimal_step_index_raw(u8 *data, u32 track_len, uint offset, uint *index, double *rms) {
    uint output_size;
    i16 *audio_buffer = (i16 *) calloc(KWZ_AUDIO_BUFFER_SIZE / sizeof(i16), sizeof(i16));
    double *step_index_rms = (double *) calloc(41, sizeof(double));

    for (int step_index = 0; step_index < 41; step_index++) {
        output_size = kwz_decode_track(data, audio_buffer, track_len, offset, step_index);
        step_index_rms[step_index] = track_rms(audio_buffer, output_size);

        // Reset buffer
        memset(audio_buffer, 0x0, KWZ_AUDIO_BUFFER_SIZE);
    }

    *index = least_rms(step_index_rms);
    *rms = step_index_rms[*index];

    free(audio_buffer);
    free(step_index_rms);

    return output_size;
}

static uint find_optimal_step_index(kwz_ctx *ctx, uint offset, u32 track_len, uint *index, double *rms) {
    return find_optimal_step_index_raw(ctx->file->data, track_len, offset, index, rms);
}

int kwz_get_track(kwz_ctx *ctx, uint offset, u32 track_length, u32 initial_step_index) {
    kwz_sound *sound = ctx->sound;
    uint output_len = 0;

    if (sound->audio_buffer == NULL) {
        sound->audio_buffer = (i16 *) calloc(KWZ_AUDIO_BUFFER_SIZE / sizeof(i16), sizeof(i16));
    }

    if (sound->audio_buffer == NULL) {
        fprintf(stderr, "Failed to allocate audio buffer\n");
        return UGOMEMO_MEMORY_ERROR;
    }

    if ((size_t)offset + track_length > ctx->file->size) return UGOMEMO_INPUT_ERROR;

    output_len = kwz_decode_track(ctx->file->data, sound->audio_buffer, track_length, offset, initial_step_index);
    sound->processed_audio_len = output_len;

    return UGOMEMO_OK;
}

static int kwz_get_audio_digest(kwz_ctx *ctx, uint track_idx, u8 *hash) {
    int res = UGOMEMO_OK;
    kwz_sound *sound = ctx->sound;
    uint offset = ctx->meta->ksn_offset + KSN_HEADER_SIZE + 8;
    u32 initial_step_index = 40;

    switch (track_idx) {
        case 0:  // BGM
            res = kwz_get_track(ctx, offset, sound->bgm_size, initial_step_index);
            break;
        case 1:  // SE1
            offset += sound->bgm_size;
            res = kwz_get_track(ctx, offset, sound->se1_size, initial_step_index);
            break;
        case 2:  // SE2
            offset += sound->bgm_size + sound->se1_size;
            res = kwz_get_track(ctx, offset, sound->se2_size, initial_step_index);
            break;
        case 3:  // SE3
            offset += sound->bgm_size + sound->se1_size + sound->se2_size;
            res = kwz_get_track(ctx, offset, sound->se3_size, initial_step_index);
            break;
        case 4:  // SE4
            offset += sound->bgm_size + sound->se1_size + sound->se2_size + sound->se3_size;
            res = kwz_get_track(ctx, offset, sound->se4_size, initial_step_index);
            break;
        default:
            fprintf(stderr, "Invalid KWZ track index.\n");
            return UGOMEMO_INPUT_ERROR;
    }
    if (res != 0) return res;

    if (hash == NULL) {
        fprintf(stderr, "KWZ audio hash is null!\n");
        return UGOMEMO_INPUT_ERROR;
    }

    sha256_hash((u8 *)sound->audio_buffer, sound->processed_audio_len * sizeof(i16), hash);

    free(sound->audio_buffer);
    sound->audio_buffer = NULL;

    return res;
}

void kwz_compute_track_digest(kwz_ctx *ctx, uint track_idx, u32 size, kwz_track_digest *out) {
    uint offset;

    memset(out, 0, sizeof(kwz_track_digest));
    out->encoded_size = size;

    if (size == 0) {
        out->has_data = false;
        return;
    }

    out->has_data = true;

    // Calculate optimal step index
    offset = ctx->meta->ksn_offset + KSN_HEADER_SIZE + 8;
    switch (track_idx) {
        case 1: offset += ctx->sound->bgm_size; break;
        case 2: offset += ctx->sound->bgm_size + ctx->sound->se1_size; break;
        case 3: offset += ctx->sound->bgm_size + ctx->sound->se1_size + ctx->sound->se2_size; break;
        case 4: offset += ctx->sound->bgm_size + ctx->sound->se1_size + ctx->sound->se2_size + ctx->sound->se3_size; break;
    }

    out->samples = find_optimal_step_index(ctx, offset, size, &out->best_step_index, &out->best_step_index_rms);

    if (kwz_get_audio_digest(ctx, track_idx, out->digest) != 0) {
        fprintf(stderr, "[DEBUG] Failed to get audio digest!\n");
        if (ctx->sound->audio_buffer) {
            free(ctx->sound->audio_buffer);
            ctx->sound->audio_buffer = NULL;
        }
        out->has_data = false;
    }
}

i16 *kwz_decode_track_alloc(kwz_ctx *ctx, int track, int step_index, u32 *out_sample_count) {
    if (!ctx || !ctx->sound || !ctx->file) return NULL;

    u32 track_size = kwz_get_track_size(ctx, track);
    if (track_size == 0) return NULL;

    // Calculate track offset
    u32 offset = ctx->meta->ksn_offset + KSN_HEADER_SIZE + 8;
    kwz_sound *s = ctx->sound;
    switch (track) {
        case 1: offset += s->bgm_size; break;
        case 2: offset += s->bgm_size + s->se1_size; break;
        case 3: offset += s->bgm_size + s->se1_size + s->se2_size; break;
        case 4: offset += s->bgm_size + s->se1_size + s->se2_size + s->se3_size; break;
    }

    // Auto brute-force optimal step index if requested
    if (step_index < 0) {
        uint best_si;
        double best_rms;
        find_optimal_step_index_raw(ctx->file->data, track_size, offset, &best_si, &best_rms);
        step_index = best_si;
    }

    if ((size_t)offset + track_size > ctx->file->size) return NULL;

    i16 *buffer = (i16 *)calloc(KWZ_AUDIO_BUFFER_SIZE / sizeof(i16), sizeof(i16));
    if (!buffer) return NULL;

    uint samples = kwz_decode_track(ctx->file->data, buffer, track_size, offset, step_index);
    if (out_sample_count) *out_sample_count = samples;

    return buffer;
}

// ==== Audio Encoding ====

// Find best 2-bit code for a target diff
static u8 kwz_best_2bit(i16 target, i16 step, i16 *decoded_diff) {
    i16 base = step >> 3;
    // Possible diffs: code 0 = +base, code 1 = +(base+step), code 2 = -base, code 3 = -(base+step)
    i16 diffs[4] = { base, (i16)(base + step), (i16)(-base), (i16)(-(base + step)) };
    int best = 0;
    int best_err = abs(target - diffs[0]);
    for (int c = 1; c < 4; c++) {
        int err = abs(target - diffs[c]);
        if (err < best_err) { best_err = err; best = c; }
    }
    *decoded_diff = diffs[best];
    return (u8)best;
}

// Find best 4-bit code for a target diff (standard IMA ADPCM)
static u8 kwz_best_4bit(i16 target, i16 step, i16 *decoded_diff) {
    // Encode magnitude
    i16 diff = (target < 0) ? -target : target;
    u8 code = 0;

    if (diff >= step)       { code |= 4; diff -= step; }
    if (diff >= (step >> 1)) { code |= 2; diff -= (step >> 1); }
    if (diff >= (step >> 2)) { code |= 1; }
    if (target < 0)         code |= 8;

    // Decode to get actual diff (same as decoder)
    i16 dec = step >> 3;
    if (code & 1) dec += step >> 2;
    if (code & 2) dec += step >> 1;
    if (code & 4) dec += step;
    if (code & 8) dec = -dec;

    *decoded_diff = dec;
    return code;
}

// Encode 16-bit PCM to KWZ's custom variable-width ADPCM.
// Input is i16 PCM at 16364 Hz (already scaled to 16-bit).
// Caller must free *out_data.
int kwz_encode_audio(const i16 *pcm, int sample_count, int initial_step_index,
                     u8 **out_data, u32 *out_size) {
    // Estimate output size: worst case is all 2-bit = sample_count/4 bytes
    // More typical is mix of 2/4-bit. Over-allocate to be safe.
    u32 max_size = (u32)sample_count;  // generous upper bound
    u8 *output = (u8 *)calloc(max_size, 1);
    if (!output) return UGOMEMO_MEMORY_ERROR;

    i16 step_index = (i16)initial_step_index;
    i16 predictor = KWZ_INITIAL_PREDICTOR;  // 0

    u32 byte_pos = 0;
    u8 current_byte = 0;
    int bit_pos = 0;
    int sample_idx = 0;

    while (sample_idx < sample_count) {
        // Process one byte at a time
        current_byte = 0;
        bit_pos = 0;

        while (bit_pos < 8 && sample_idx < sample_count) {
            // Scale 16-bit PCM to 12-bit range for KWZ
            i16 target = (i16)(pcm[sample_idx] / KWZ_SCALING_FACTOR) - predictor;

            i16 step = ADPCM_STEP_TABLE[step_index];
            i16 decoded_diff;

            if (step_index < KWZ_VARIABLE_THRESHOLD || bit_pos > 4) {
                // 2-bit sample
                u8 code = kwz_best_2bit(target, step, &decoded_diff);
                current_byte |= (code & 0x3) << bit_pos;
                bit_pos += 2;

                predictor += decoded_diff;
                step_index += ADPCM_INDEX_TABLE_2BIT[code];
            } else {
                // 4-bit sample
                u8 code = kwz_best_4bit(target, step, &decoded_diff);
                current_byte |= (code & 0xF) << bit_pos;
                bit_pos += 4;

                predictor += decoded_diff;
                step_index += ADPCM_INDEX_TABLE_4BIT[code];
            }

            // Clamp state (same as decoder)
            if (step_index < KWZ_STEP_INDEX_MIN) step_index = KWZ_STEP_INDEX_MIN;
            if (step_index > KWZ_STEP_INDEX_MAX) step_index = KWZ_STEP_INDEX_MAX;
            if (predictor < KWZ_PREDICTOR_MIN) predictor = KWZ_PREDICTOR_MIN;
            if (predictor > KWZ_PREDICTOR_MAX) predictor = KWZ_PREDICTOR_MAX;

            sample_idx++;
        }

        output[byte_pos++] = current_byte;
    }

    *out_data = output;
    *out_size = byte_pos;
    return UGOMEMO_OK;
}
