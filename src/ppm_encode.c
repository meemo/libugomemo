#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ugomemo.h>

#include <ugomemo/adpcm.h>
#include <ugomemo/ppm_video_tables.h>

static inline bool rgb_eq(rgb24_pixel a, rgb24_pixel b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

// ---- Full PPM Encoding ----

// Encoded audio tracks for embedding in the PPM file.
// If a track's data is non-NULL it takes priority over reference audio.
typedef struct {
    u8 *bgm;   u32 bgm_size;
    u8 *se1;   u32 se1_size;
    u8 *se2;   u32 se2_size;
    u8 *se3;   u32 se3_size;
    u8 frame_speed;
    u8 frame_speed_when_recorded;
} ppm_audio_tracks;

// Encode a PPM file from a directory of BMP frames.
// The output is written as raw bytes to out_data/out_size (caller must free).
// If ref_ctx is non-NULL, the video stream is compared against the reference PPM
// and mismatches are reported. If audio is non-NULL, those tracks are embedded.
int ppm_encode_frames(const char *bmp_dir, u8 **out_data, size_t *out_size,
                      ppm_ctx *ref_ctx, ppm_audio_tracks *audio,
                      const ugomemo_rsa_key *key) {
    int res = UGOMEMO_OK;
    int frame_count = 0;
    rgb24_pixel *pixels = NULL;
    u8 *layer_1 = NULL, *layer_2 = NULL;
    u8 *frame_buf = NULL;
    u8 *anim_data = NULL;
    u32 *frame_offsets = NULL;
    u8 paper_color = 1, layer_1_color = 1, layer_2_color = 1;

    res = count_bmp_files(bmp_dir, &frame_count);
    if (res != UGOMEMO_OK) return res;
    if (frame_count == 0) {
        ERROR("No BMP files found in %s\n", bmp_dir);
        return UGOMEMO_INPUT_ERROR;
    }
    if (frame_count > 999) {
        ERROR("Too many frames (%d > 999)\n", frame_count);
        return UGOMEMO_INPUT_ERROR;
    }

    pixels = (rgb24_pixel *)calloc(PPM_LAYER_SIZE, sizeof(rgb24_pixel));
    layer_1 = (u8 *)calloc(PPM_LAYER_SIZE, sizeof(u8));
    layer_2 = (u8 *)calloc(PPM_LAYER_SIZE, sizeof(u8));
    // Max frame size: 1 header + 96 encoding bytes + 2 * (192 * 36) = ~13921 bytes
    frame_buf = (u8 *)malloc(PPM_LAYER_SIZE * 2);
    // Generous animation data buffer
    size_t anim_capacity = (size_t)frame_count * PPM_LAYER_SIZE * 2;
    anim_data = (u8 *)malloc(anim_capacity);
    frame_offsets = (u32 *)calloc(frame_count, sizeof(u32));

    if (!pixels || !layer_1 || !layer_2 || !frame_buf || !anim_data || !frame_offsets) {
        ERROR("Failed to allocate encoding buffers\n");
        res = UGOMEMO_MEMORY_ERROR;
        goto cleanup;
    }

    // Reference comparison buffers (composited RGB comparison)
    rgb24_pixel *ref_rgb = NULL;
    u8 *ref_l1 = NULL, *ref_l2 = NULL, *ref_prev_l1 = NULL, *ref_prev_l2 = NULL;
    int ref_total_diff = 0;
    if (ref_ctx) {
        ref_rgb = (rgb24_pixel *)calloc(PPM_LAYER_SIZE, sizeof(rgb24_pixel));
        ref_l1 = (u8 *)calloc(PPM_LAYER_SIZE, sizeof(u8));
        ref_l2 = (u8 *)calloc(PPM_LAYER_SIZE, sizeof(u8));
        ref_prev_l1 = (u8 *)calloc(PPM_LAYER_SIZE, sizeof(u8));
        ref_prev_l2 = (u8 *)calloc(PPM_LAYER_SIZE, sizeof(u8));
        if (!ref_rgb || !ref_l1 || !ref_l2 || !ref_prev_l1 || !ref_prev_l2) {
            res = UGOMEMO_MEMORY_ERROR;
            goto cleanup_ref;
        }
    }

    char path[512];

    // Encode all frames
    size_t total_anim_size = 0;

    for (int i = 0; i < frame_count; i++) {
        snprintf(path, sizeof(path), "%s/%04d.bmp", bmp_dir, i + 1);
        res = read_bmp_pixels(path, pixels, PPM_FRAME_WIDTH, PPM_FRAME_HEIGHT);
        if (res != UGOMEMO_OK) goto cleanup_ref;

        // Detect paper and layer colors per-frame (both can vary between frames)
        ppm_detect_paper_color(pixels, &paper_color);
        ppm_detect_frame_colors(pixels, paper_color, &layer_1_color, &layer_2_color);

        ppm_classify_frame(pixels, paper_color, layer_1_color, layer_2_color,
                           layer_1, layer_2);

        frame_offsets[i] = (u32)total_anim_size;

        size_t frame_len = 0;
        res = ppm_encode_frame(layer_1, layer_2, paper_color, layer_1_color, layer_2_color,
                               frame_buf, &frame_len);
        if (res != UGOMEMO_OK) goto cleanup_ref;

        if (total_anim_size + frame_len > anim_capacity) {
            ERROR("Animation data exceeded allocation\n");
            res = UGOMEMO_MEMORY_ERROR;
            goto cleanup_ref;
        }
        memcpy(anim_data + total_anim_size, frame_buf, frame_len);
        total_anim_size += frame_len;

        // Compare composited RGB with reference if available
        if (ref_ctx && (uint)i < ref_ctx->meta->frame_count) {
            res = ppm_decode_frame(ref_ctx, i, ref_rgb,
                                   ref_prev_l1, ref_prev_l2, ref_l1, ref_l2);
            if (res != UGOMEMO_OK) {
                ERROR("Failed to decode reference frame %d\n", i);
                goto cleanup_ref;
            }

            // Compare the BMP pixels (composited output) against the reference
            int pixel_diff = 0;
            for (int p = 0; p < PPM_LAYER_SIZE; p++) {
                if (!rgb_eq(pixels[p], ref_rgb[p])) pixel_diff++;
            }

            if (pixel_diff > 0) {
                fprintf(stderr, "Frame %d: %d pixel differences in composited output\n",
                        i, pixel_diff);
                ref_total_diff += pixel_diff;
            }

            memcpy(ref_prev_l1, ref_l1, PPM_LAYER_SIZE);
            memcpy(ref_prev_l2, ref_l2, PPM_LAYER_SIZE);
        }
    }

    if (ref_ctx) {
        if (ref_total_diff == 0)
            fprintf(stderr, "Reference comparison: all %d frames match perfectly\n", frame_count);
        else
            fprintf(stderr, "Reference comparison: %d total pixel differences\n", ref_total_diff);
    }

    // Now assemble the full PPM file
    {
        // Frame offset table size
        u16 offset_table_size = frame_count * 4;
        uint offset_table_padding = UGO_ROUND_UP_MULT_4(offset_table_size) - offset_table_size;
        uint anim_data_padding = UGO_ROUND_UP_MULT_4(total_anim_size) - total_anim_size;

        // SFX flags: 1 byte per frame (all zero = no sound effects)
        uint sfx_flags_size = frame_count;
        uint sfx_flags_padding = UGO_ROUND_UP_MULT_4(sfx_flags_size) - sfx_flags_size;

        // Calculate section sizes
        // animation_data_size field in header = anim_header(8) + offset_table + offset_table_padding + frame_data + frame_data_padding
        size_t animation_data_size = PPM_ANIM_HEADER_SIZE + offset_table_size + offset_table_padding
                                     + total_anim_size + anim_data_padding;

        // Resolve audio tracks: explicit audio > reference audio > silence
        u8 *snd_bgm = NULL, *snd_se1 = NULL, *snd_se2 = NULL, *snd_se3 = NULL;
        u32 snd_bgm_sz = 0, snd_se1_sz = 0, snd_se2_sz = 0, snd_se3_sz = 0;
        u8 snd_speed = 0, snd_speed_rec = 0;
        u8 *snd_padding = NULL;

        if (audio && audio->bgm) { snd_bgm = audio->bgm; snd_bgm_sz = audio->bgm_size; }
        else if (ref_ctx && ref_ctx->sound && ref_ctx->sound->bgm_size > 0)
            { snd_bgm = ref_ctx->sound->bgm; snd_bgm_sz = ref_ctx->sound->bgm_size; }

        if (audio && audio->se1) { snd_se1 = audio->se1; snd_se1_sz = audio->se1_size; }
        else if (ref_ctx && ref_ctx->sound && ref_ctx->sound->se1_size > 0)
            { snd_se1 = ref_ctx->sound->se1; snd_se1_sz = ref_ctx->sound->se1_size; }

        if (audio && audio->se2) { snd_se2 = audio->se2; snd_se2_sz = audio->se2_size; }
        else if (ref_ctx && ref_ctx->sound && ref_ctx->sound->se2_size > 0)
            { snd_se2 = ref_ctx->sound->se2; snd_se2_sz = ref_ctx->sound->se2_size; }

        if (audio && audio->se3) { snd_se3 = audio->se3; snd_se3_sz = audio->se3_size; }
        else if (ref_ctx && ref_ctx->sound && ref_ctx->sound->se3_size > 0)
            { snd_se3 = ref_ctx->sound->se3; snd_se3_sz = ref_ctx->sound->se3_size; }

        if (audio) { snd_speed = audio->frame_speed; snd_speed_rec = audio->frame_speed_when_recorded; }
        else if (ref_ctx && ref_ctx->sound)
            { snd_speed = ref_ctx->sound->frame_speed; snd_speed_rec = ref_ctx->sound->frame_speed_when_recorded;
              snd_padding = ref_ctx->sound->padding_bytes; }

        size_t sound_data_size = snd_bgm_sz + snd_se1_sz + snd_se2_sz + snd_se3_sz;

        // Total file size
        size_t file_size = PPM_FILE_HEADER_SIZE       // 0x10
                         + PPM_META_SIZE               // 0x90
                         + PPM_THUMBNAIL_SIZE          // 0x600
                         + animation_data_size
                         + sfx_flags_size + sfx_flags_padding
                         + PPM_SOUND_HEADER_SIZE
                         + sound_data_size
                         + PPM_SIGNATURE_SIZE + 16;    // signature + padding

        u8 *file_data = (u8 *)calloc(file_size, sizeof(u8));
        if (!file_data) {
            res = UGOMEMO_MEMORY_ERROR;
            goto cleanup_ref;
        }

        size_t pos = 0;

        // ---- File Header (0x10) ----
        memcpy(file_data + pos, "PARA", 4); pos += 4;
        // Animation data size (LE32)
        file_data[pos++] = (animation_data_size >>  0) & 0xFF;
        file_data[pos++] = (animation_data_size >>  8) & 0xFF;
        file_data[pos++] = (animation_data_size >> 16) & 0xFF;
        file_data[pos++] = (animation_data_size >> 24) & 0xFF;
        // Sound data size (LE32)
        file_data[pos++] = (sound_data_size >>  0) & 0xFF;
        file_data[pos++] = (sound_data_size >>  8) & 0xFF;
        file_data[pos++] = (sound_data_size >> 16) & 0xFF;
        file_data[pos++] = (sound_data_size >> 24) & 0xFF;
        // Frame count minus 1 (LE16)
        u16 fc_stored = (u16)(frame_count - 1);
        file_data[pos++] = (fc_stored >> 0) & 0xFF;
        file_data[pos++] = (fc_stored >> 8) & 0xFF;
        // Format version = 0x24 (LE16)
        file_data[pos++] = 0x24;
        file_data[pos++] = 0x00;

        // ---- Metadata (0x90) ----
        if (ref_ctx && ref_ctx->file && ref_ctx->file->data) {
            memcpy(file_data + pos, ref_ctx->file->data + PPM_META_OFFSET, PPM_META_SIZE);
        }
        pos += PPM_META_SIZE;

        // ---- Thumbnail (0x600) ----
        if (ref_ctx && ref_ctx->file && ref_ctx->file->data) {
            memcpy(file_data + pos, ref_ctx->file->data + PPM_THUMBNAIL_OFFSET, PPM_THUMBNAIL_SIZE);
        }
        pos += PPM_THUMBNAIL_SIZE;

        // ---- Animation Header (at 0x6A0) ----
        // u16 frame offset table size
        file_data[pos++] = (offset_table_size >> 0) & 0xFF;
        file_data[pos++] = (offset_table_size >> 8) & 0xFF;
        // u32 unknown + u16 flags: copy from reference if available
        if (ref_ctx && ref_ctx->meta) {
            u32 unk = ref_ctx->meta->frame_offset_table_unknown;
            file_data[pos++] = (unk >>  0) & 0xFF;
            file_data[pos++] = (unk >>  8) & 0xFF;
            file_data[pos++] = (unk >> 16) & 0xFF;
            file_data[pos++] = (unk >> 24) & 0xFF;
            u16 flags = ref_ctx->meta->frame_offset_table_flags;
            file_data[pos++] = (flags >> 0) & 0xFF;
            file_data[pos++] = (flags >> 8) & 0xFF;
        } else {
            pos += 4;  // unknown = 0
            file_data[pos++] = 0x40;
            file_data[pos++] = 0x00;
        }

        // ---- Frame Offset Table ----
        for (int i = 0; i < frame_count; i++) {
            file_data[pos++] = (frame_offsets[i] >>  0) & 0xFF;
            file_data[pos++] = (frame_offsets[i] >>  8) & 0xFF;
            file_data[pos++] = (frame_offsets[i] >> 16) & 0xFF;
            file_data[pos++] = (frame_offsets[i] >> 24) & 0xFF;
        }
        // Offset table padding
        pos += offset_table_padding;

        // ---- Animation Frame Data ----
        memcpy(file_data + pos, anim_data, total_anim_size);
        pos += total_anim_size;
        // Animation data padding
        pos += anim_data_padding;

        // ---- SFX Flags ----
        if (ref_ctx && ref_ctx->sfx_flags) {
            uint copy_count = frame_count < (int)ref_ctx->meta->frame_count
                            ? (uint)frame_count : ref_ctx->meta->frame_count;
            memcpy(file_data + pos, ref_ctx->sfx_flags, copy_count);
        }
        pos += sfx_flags_size + sfx_flags_padding;

        // ---- Sound Header (0x20) ----
        // Track sizes
        file_data[pos+0]  = (snd_bgm_sz >>  0) & 0xFF;
        file_data[pos+1]  = (snd_bgm_sz >>  8) & 0xFF;
        file_data[pos+2]  = (snd_bgm_sz >> 16) & 0xFF;
        file_data[pos+3]  = (snd_bgm_sz >> 24) & 0xFF;
        file_data[pos+4]  = (snd_se1_sz >>  0) & 0xFF;
        file_data[pos+5]  = (snd_se1_sz >>  8) & 0xFF;
        file_data[pos+6]  = (snd_se1_sz >> 16) & 0xFF;
        file_data[pos+7]  = (snd_se1_sz >> 24) & 0xFF;
        file_data[pos+8]  = (snd_se2_sz >>  0) & 0xFF;
        file_data[pos+9]  = (snd_se2_sz >>  8) & 0xFF;
        file_data[pos+10] = (snd_se2_sz >> 16) & 0xFF;
        file_data[pos+11] = (snd_se2_sz >> 24) & 0xFF;
        file_data[pos+12] = (snd_se3_sz >>  0) & 0xFF;
        file_data[pos+13] = (snd_se3_sz >>  8) & 0xFF;
        file_data[pos+14] = (snd_se3_sz >> 16) & 0xFF;
        file_data[pos+15] = (snd_se3_sz >> 24) & 0xFF;
        pos += 16;
        file_data[pos++] = snd_speed;
        file_data[pos++] = snd_speed_rec;
        if (snd_padding) memcpy(file_data + pos, snd_padding, 14);
        pos += 14;

        // Audio track data
        if (snd_bgm_sz > 0 && snd_bgm) { memcpy(file_data + pos, snd_bgm, snd_bgm_sz); pos += snd_bgm_sz; }
        if (snd_se1_sz > 0 && snd_se1) { memcpy(file_data + pos, snd_se1, snd_se1_sz); pos += snd_se1_sz; }
        if (snd_se2_sz > 0 && snd_se2) { memcpy(file_data + pos, snd_se2, snd_se2_sz); pos += snd_se2_sz; }
        if (snd_se3_sz > 0 && snd_se3) { memcpy(file_data + pos, snd_se3, snd_se3_sz); pos += snd_se3_sz; }

        // ---- Signature ----
        // Generate RSA signature if a private key is available, otherwise leave as zeros
        if (key && key->private_exponent) {
            res = rsa_generate_signature(key, file_data, pos, file_data + pos, false);
            if (res != UGOMEMO_OK) res = UGOMEMO_OK;  // non-fatal
        }
        pos += PPM_SIGNATURE_SIZE;

        // Signature padding (16 bytes, already zeroed)
        pos += 16;

        if (pos != file_size) {
            ERROR("PPM file assembly size mismatch: pos=%zu expected=%zu\n", pos, file_size);
            free(file_data);
            res = UGOMEMO_INTERNAL_ERROR;
            goto cleanup_ref;
        }

        *out_data = file_data;
        *out_size = file_size;
    }

cleanup_ref:
    free(ref_rgb);
    free(ref_l1);
    free(ref_l2);
    free(ref_prev_l1);
    free(ref_prev_l2);

cleanup:
    free(pixels);
    free(layer_1);
    free(layer_2);
    free(frame_buf);
    free(anim_data);
    free(frame_offsets);
    return res;
}

// Compare a PPM file's decoded video stream against frames re-encoded from BMPs.
// Returns UGOMEMO_OK if all frames match, UGOMEMO_INPUT_ERROR if there are differences.
int ppm_compare_video(const char *ppm_path, const char *bmp_dir, const ugomemo_rsa_key *key) {
    int res = UGOMEMO_OK;
    ugomemo_file *input = NULL;
    ppm_ctx *ctx = NULL;
    u8 *encoded_data = NULL;
    size_t encoded_size = 0;

    // Open and parse the reference PPM
    res = file_init(&input, (char *)ppm_path, false);
    if (res != UGOMEMO_OK) return res;

    res = file_read(input);
    if (res != UGOMEMO_OK) { file_free(&input); return res; }

    ctx = (ppm_ctx *)calloc(1, sizeof(ppm_ctx));
    if (!ctx) { file_free(&input); return UGOMEMO_MEMORY_ERROR; }
    ctx->file = input;

    res = ppm_read_file(ctx);
    if (res != UGOMEMO_OK) {
        ERROR("Failed to parse reference PPM\n");
        ppm_cleanup(&ctx);
        file_free(&input);
        return res;
    }

    // Encode from BMPs with comparison enabled
    res = ppm_encode_frames(bmp_dir, &encoded_data, &encoded_size, ctx, NULL, key);

    // Now also do a byte-level comparison of the frame data sections
    if (res == UGOMEMO_OK && encoded_data) {
        // Parse the encoded PPM to compare decoded frames
        ugomemo_file enc_file_storage = {0};
        enc_file_storage.data = encoded_data;
        enc_file_storage.size = encoded_size;

        ppm_ctx *enc_ctx = (ppm_ctx *)calloc(1, sizeof(ppm_ctx));
        if (enc_ctx) {
            enc_ctx->file = &enc_file_storage;
            int parse_res = ppm_read_file(enc_ctx);

            if (parse_res == UGOMEMO_OK) {
                // Compare decoded frames
                rgb24_pixel *rgb_ref = (rgb24_pixel *)calloc(PPM_LAYER_SIZE, sizeof(rgb24_pixel));
                rgb24_pixel *rgb_enc = (rgb24_pixel *)calloc(PPM_LAYER_SIZE, sizeof(rgb24_pixel));
                u8 *l1_r = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *l2_r = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *l1_e = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *l2_e = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *pl1_r = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *pl2_r = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *pl1_e = (u8 *)calloc(PPM_LAYER_SIZE, 1);
                u8 *pl2_e = (u8 *)calloc(PPM_LAYER_SIZE, 1);

                if (rgb_ref && rgb_enc && l1_r && l2_r && l1_e && l2_e &&
                    pl1_r && pl2_r && pl1_e && pl2_e) {
                    uint min_frames = ctx->meta->frame_count < enc_ctx->meta->frame_count
                                    ? ctx->meta->frame_count : enc_ctx->meta->frame_count;
                    int total_pixel_diff = 0;

                    for (uint i = 0; i < min_frames; i++) {
                        ppm_decode_frame(ctx, i, rgb_ref, pl1_r, pl2_r, l1_r, l2_r);
                        ppm_decode_frame(enc_ctx, i, rgb_enc, pl1_e, pl2_e, l1_e, l2_e);

                        int pixel_diff = 0;
                        for (int p = 0; p < PPM_LAYER_SIZE; p++) {
                            if (!rgb_eq(rgb_ref[p], rgb_enc[p])) pixel_diff++;
                        }
                        if (pixel_diff > 0) {
                            fprintf(stderr, "  Decoded frame %u: %d pixel differences\n", i, pixel_diff);
                            total_pixel_diff += pixel_diff;
                        }

                        memcpy(pl1_r, l1_r, PPM_LAYER_SIZE);
                        memcpy(pl2_r, l2_r, PPM_LAYER_SIZE);
                        memcpy(pl1_e, l1_e, PPM_LAYER_SIZE);
                        memcpy(pl2_e, l2_e, PPM_LAYER_SIZE);
                    }

                    if (total_pixel_diff == 0) {
                        fprintf(stderr, "Video comparison: %u frames match perfectly\n", min_frames);
                    } else {
                        fprintf(stderr, "Video comparison: %d total pixel differences across %u frames\n",
                                total_pixel_diff, min_frames);
                        res = UGOMEMO_INPUT_ERROR;
                    }
                }

                free(rgb_ref); free(rgb_enc);
                free(l1_r); free(l2_r); free(l1_e); free(l2_e);
                free(pl1_r); free(pl2_r); free(pl1_e); free(pl2_e);
            }

            // Don't free enc_file_storage.data - it's encoded_data, freed below
            enc_ctx->file = NULL;
            free(enc_ctx->meta);
            if (enc_ctx->sound) free(enc_ctx->sound);
            if (enc_ctx->signature) {
                free(enc_ctx->signature->decrypted_signature);
                free(enc_ctx->signature->sha1_computed);
                free(enc_ctx->signature);
            }
            ugomemo_error_list_free(&enc_ctx->errors);
            free(enc_ctx);
        }
    }

    free(encoded_data);
    ppm_cleanup(&ctx);
    file_free(&input);
    return res;
}

// Command: encode frames from BMP directory into a PPM file
// Helper: encode a WAV file to PPM ADPCM. Returns NULL on failure.
static u8 *encode_wav_track(const char *wav_path, u32 *adpcm_size) {
    i16 *pcm = NULL;
    int sample_count = 0;
    u8 *adpcm = NULL;

    if (read_wav_samples(wav_path, &pcm, &sample_count) != UGOMEMO_OK)
        return NULL;

    if (ppm_encode_audio(pcm, sample_count, &adpcm, adpcm_size) != UGOMEMO_OK) {
        free(pcm);
        return NULL;
    }

    free(pcm);
    return adpcm;
}

int ppm_encode_frames_command(const char *bmp_dir, const char *output_path,
                              const char *reference_ppm_path,
                              const char *bgm_wav, const char *se1_wav,
                              const char *se2_wav, const char *se3_wav,
                              const ugomemo_rsa_key *key) {
    int res = UGOMEMO_OK;
    u8 *encoded_data = NULL;
    size_t encoded_size = 0;
    ppm_ctx *ref_ctx = NULL;
    ugomemo_file *ref_input = NULL;
    ppm_audio_tracks audio = {0};
    bool has_audio = false;

    // If reference PPM provided, open it for comparison and section copying
    if (reference_ppm_path) {
        res = file_init(&ref_input, (char *)reference_ppm_path, false);
        if (res != UGOMEMO_OK) return res;

        res = file_read(ref_input);
        if (res != UGOMEMO_OK) { file_free(&ref_input); return res; }

        ref_ctx = (ppm_ctx *)calloc(1, sizeof(ppm_ctx));
        if (!ref_ctx) { file_free(&ref_input); return UGOMEMO_MEMORY_ERROR; }
        ref_ctx->file = ref_input;

        res = ppm_read_file(ref_ctx);
        if (res != UGOMEMO_OK) {
            ERROR("Failed to parse reference PPM\n");
            ppm_cleanup(&ref_ctx);
            file_free(&ref_input);
            return res;
        }
    }

    // Encode WAV files to ADPCM if provided
    if (bgm_wav) { audio.bgm = encode_wav_track(bgm_wav, &audio.bgm_size); has_audio = true; }
    if (se1_wav) { audio.se1 = encode_wav_track(se1_wav, &audio.se1_size); has_audio = true; }
    if (se2_wav) { audio.se2 = encode_wav_track(se2_wav, &audio.se2_size); has_audio = true; }
    if (se3_wav) { audio.se3 = encode_wav_track(se3_wav, &audio.se3_size); has_audio = true; }

    res = ppm_encode_frames(bmp_dir, &encoded_data, &encoded_size,
                            ref_ctx, has_audio ? &audio : NULL, key);
    if (res != UGOMEMO_OK) {
        ERROR("Failed to encode frames\n");
        goto cleanup;
    }

    if (output_path) {
        ugomemo_file *output = NULL;
        res = file_init(&output, (char *)output_path, true);
        if (res != UGOMEMO_OK) goto cleanup;

        file_write(output, encoded_data, encoded_size);
        file_free(&output);
        fprintf(stderr, "Wrote %zu bytes to %s\n", encoded_size, output_path);
    }

cleanup:
    free(encoded_data);
    free(audio.bgm);
    free(audio.se1);
    free(audio.se2);
    free(audio.se3);
    if (ref_ctx) ppm_cleanup(&ref_ctx);
    if (ref_input) file_free(&ref_input);
    return res;
}
