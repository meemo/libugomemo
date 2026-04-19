#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ugomemo.h>

#include <ugomemo/kwz_video_tables.h>

// Transparent paper renders as (0,0,0) in BMP output
static const rgb24_pixel TRANSPARENT_PAPER_RGB = { 0x00, 0x00, 0x00 };

static inline bool kwz_rgb_eq_local(rgb24_pixel a, rgb24_pixel b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

// ---- File Assembly Helpers ----

static void write_le16(u8 *buf, size_t pos, u16 val) {
    buf[pos]     = val & 0xFF;
    buf[pos + 1] = (val >> 8) & 0xFF;
}

static void write_le32(u8 *buf, size_t pos, u32 val) {
    buf[pos]     = val & 0xFF;
    buf[pos + 1] = (val >> 8) & 0xFF;
    buf[pos + 2] = (val >> 16) & 0xFF;
    buf[pos + 3] = (val >> 24) & 0xFF;
}

// ---- Full KWZ Encoding ----

// Encoded audio tracks for KWZ
typedef struct {
    u8 *bgm;   u32 bgm_size;
    u8 *se1;   u32 se1_size;
    u8 *se2;   u32 se2_size;
    u8 *se3;   u32 se3_size;
    u8 *se4;   u32 se4_size;
} kwz_audio_tracks;

int kwz_encode_frames(const char *bmp_dir, u8 **out_data, size_t *out_size,
                      kwz_ctx *ref_ctx, kwz_audio_tracks *audio,
                      const ugomemo_rsa_key *key) {
    int res = UGOMEMO_OK;
    int frame_count = 0;
    rgb24_pixel *pixels = NULL;
    u8 *layer_a = NULL, *layer_b = NULL, *layer_c = NULL;
    u8 *comp_buf = NULL;  // compressed layer buffer
    u8 *kmc_data = NULL;  // all compressed frame data
    size_t kmc_total = 0;
    kwz_tile_pos tile_positions[KWZ_TILE_COUNT];
    i16 common_reverse[6561];

    // Per-frame metadata for KMI section
    typedef struct { u32 flags; u32 la_size, lb_size, lc_size; } frame_meta;
    frame_meta *frame_metas = NULL;

    res = count_bmp_files(bmp_dir, &frame_count);
    if (res != UGOMEMO_OK) return res;
    if (frame_count == 0 || frame_count > 999) {
        ERROR("Invalid frame count: %d\n", frame_count);
        return UGOMEMO_INPUT_ERROR;
    }

    // Allocate buffers
    pixels = (rgb24_pixel *)calloc(KWZ_LAYER_SIZE, sizeof(rgb24_pixel));
    layer_a = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
    layer_b = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
    layer_c = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
    comp_buf = (u8 *)malloc(KWZ_LAYER_SIZE * 4);  // generous
    kmc_data = (u8 *)malloc(frame_count * KWZ_LAYER_SIZE * 4);
    frame_metas = (frame_meta *)calloc(frame_count, sizeof(frame_meta));

    if (!pixels || !layer_a || !layer_b || !layer_c || !comp_buf || !kmc_data || !frame_metas) {
        res = UGOMEMO_MEMORY_ERROR;
        goto cleanup;
    }

    kwz_compute_tile_positions(tile_positions);
    kwz_build_common_reverse_lookup(common_reverse);

    char path[512];

    // Reference comparison buffers
    rgb24_pixel *ref_rgb = NULL;
    u8 *ref_la = NULL, *ref_lb = NULL, *ref_lc = NULL;
    u8 *ref_pla = NULL, *ref_plb = NULL, *ref_plc = NULL;
    int ref_total_diff = 0;
    if (ref_ctx) {
        ref_rgb = (rgb24_pixel *)calloc(KWZ_LAYER_SIZE, sizeof(rgb24_pixel));
        ref_la = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
        ref_lb = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
        ref_lc = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
        ref_pla = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
        ref_plb = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
        ref_plc = (u8 *)calloc(KWZ_LAYER_SIZE, 1);
    }

    // Encode all frames
    for (int i = 0; i < frame_count; i++) {
        snprintf(path, sizeof(path), "%s/%04d.bmp", bmp_dir, i + 1);
        res = read_bmp_pixels(path, pixels, KWZ_FRAME_WIDTH, KWZ_FRAME_HEIGHT);
        if (res != UGOMEMO_OK) goto cleanup_ref;

        // Detect per-frame paper color
        // (0,0,0) can ONLY come from transparent paper (index 6) since no
        // palette color produces pure black. Any (0,0,0) pixel means paper=6.
        int cw = 0, cb = 0, ct = 0;
        for (int p = 0; p < KWZ_LAYER_SIZE; p++) {
            if (kwz_rgb_eq_local(pixels[p], TRANSPARENT_PAPER_RGB)) ct++;
            else if (kwz_rgb_eq_local(pixels[p], KWZ_PALETTE[0])) cw++;
            else if (kwz_rgb_eq_local(pixels[p], KWZ_PALETTE[1])) cb++;
        }
        u8 paper_idx;
        if (ct > 0)
            paper_idx = 6;  // transparent paper
        else
            paper_idx = (cw >= cb) ? 0 : 1;

        // Detect all colors and assign to layers
        kwz_frame_colors fcolors;
        kwz_detect_frame_colors(pixels, paper_idx, &fcolors);

        // Classify pixels into layers
        kwz_classify_frame(pixels, &fcolors, layer_a, layer_b, layer_c);

        // Compress each layer
        size_t kmc_capacity = (size_t)frame_count * KWZ_LAYER_SIZE * 4;

        u32 la_size = kwz_compress_layer(layer_a, comp_buf, KWZ_LAYER_SIZE * 4,
                                         tile_positions, common_reverse);
        if (kmc_total + la_size > kmc_capacity) { res = UGOMEMO_MEMORY_ERROR; goto cleanup_ref; }
        memcpy(kmc_data + kmc_total, comp_buf, la_size);
        kmc_total += la_size;

        u32 lb_size = kwz_compress_layer(layer_b, comp_buf, KWZ_LAYER_SIZE * 4,
                                         tile_positions, common_reverse);
        if (kmc_total + lb_size > kmc_capacity) { res = UGOMEMO_MEMORY_ERROR; goto cleanup_ref; }
        memcpy(kmc_data + kmc_total, comp_buf, lb_size);
        kmc_total += lb_size;

        u32 lc_size = kwz_compress_layer(layer_c, comp_buf, KWZ_LAYER_SIZE * 4,
                                         tile_positions, common_reverse);
        if (kmc_total + lc_size > kmc_capacity) { res = UGOMEMO_MEMORY_ERROR; goto cleanup_ref; }
        memcpy(kmc_data + kmc_total, comp_buf, lc_size);
        kmc_total += lc_size;

        // Build KMI flags
        // Bits 4,5,6 SET = layer is NOT diffing (keyframe)
        u32 flags = fcolors.paper_idx & 0xF;
        flags |= 0x70;  // diff flags: all layers are keyframes
        flags |= ((u32)(fcolors.la_c1 & 0xF)) << 8;
        flags |= ((u32)(fcolors.la_c2 & 0xF)) << 12;
        flags |= ((u32)(fcolors.lb_c1 & 0xF)) << 16;
        flags |= ((u32)(fcolors.lb_c2 & 0xF)) << 20;
        flags |= ((u32)(fcolors.lc_c1 & 0xF)) << 24;
        flags |= ((u32)(fcolors.lc_c2 & 0xF)) << 28;

        frame_metas[i].flags = flags;
        frame_metas[i].la_size = la_size;
        frame_metas[i].lb_size = lb_size;
        frame_metas[i].lc_size = lc_size;

        // Compare with reference
        if (ref_ctx && ref_rgb && (uint)i < ref_ctx->meta->frame_count) {
            kwz_decode_frame(ref_ctx, i, ref_rgb, ref_pla, ref_plb, ref_plc,
                             ref_la, ref_lb, ref_lc);

            int pixel_diff = 0;
            for (int p = 0; p < KWZ_LAYER_SIZE; p++) {
                if (!kwz_rgb_eq_local(pixels[p], ref_rgb[p])) pixel_diff++;
            }
            if (pixel_diff > 0) {
                fprintf(stderr, "Frame %d: %d pixel differences\n", i, pixel_diff);
                ref_total_diff += pixel_diff;
            }

            memcpy(ref_pla, ref_la, KWZ_LAYER_SIZE);
            memcpy(ref_plb, ref_lb, KWZ_LAYER_SIZE);
            memcpy(ref_plc, ref_lc, KWZ_LAYER_SIZE);
        }
    }

    if (ref_ctx) {
        if (ref_total_diff == 0)
            fprintf(stderr, "Reference comparison: all %d frames match perfectly\n", frame_count);
        else
            fprintf(stderr, "Reference comparison: %d total pixel differences\n", ref_total_diff);
    }

    // ---- Assemble KWZ file ----
    {
        // Section sizes
        u32 kfh_section_size = KFH_SECTION_SIZE;  // 204

        // KTN: copy from reference or minimal
        u32 ktn_section_size = 4;  // default: CRC32 only
        bool has_ref_ktn = ref_ctx && ref_ctx->meta && ref_ctx->meta->ktn_size > 0;
        if (has_ref_ktn)
            ktn_section_size = (u32)ref_ctx->meta->ktn_size;
        u32 ktn_total = 8 + UGO_ROUND_UP_MULT_4(ktn_section_size);

        // KMC: CRC32 + compressed data
        if (kmc_total > UINT32_MAX - 4) { res = UGOMEMO_INTERNAL_ERROR; goto cleanup_ref; }
        u32 kmc_section_size = 4 + (u32)kmc_total;
        u32 kmc_padding = UGO_ROUND_UP_MULT_4(kmc_section_size + 8) - (kmc_section_size + 8);

        // KMI: 28 bytes per frame
        u32 kmi_section_size = frame_count * 28;
        u32 kmi_padding = UGO_ROUND_UP_MULT_4(kmi_section_size + 8) - (kmi_section_size + 8);

        // KSN: resolve audio sources (explicit > reference > empty)
        // If audio tracks are provided explicitly, build the KSN from scratch.
        // Otherwise, copy from reference or emit a minimal empty KSN.
        bool build_ksn = audio && (audio->bgm || audio->se1 || audio->se2 || audio->se3 || audio->se4);
        bool has_ref_ksn = !build_ksn && ref_ctx && ref_ctx->meta && ref_ctx->meta->ksn_size > 0;

        u32 ksn_audio_size = 0;
        u32 ksn_section_size;
        if (build_ksn) {
            ksn_audio_size = (audio->bgm ? audio->bgm_size : 0) +
                             (audio->se1 ? audio->se1_size : 0) +
                             (audio->se2 ? audio->se2_size : 0) +
                             (audio->se3 ? audio->se3_size : 0) +
                             (audio->se4 ? audio->se4_size : 0);
            ksn_section_size = 28 + ksn_audio_size;  // header(28) + audio data
        } else if (has_ref_ksn) {
            ksn_section_size = (u32)ref_ctx->meta->ksn_size;
        } else {
            ksn_section_size = 28;  // empty
        }
        u32 ksn_total = 8 + ksn_section_size;
        u32 ksn_padding = UGO_ROUND_UP_MULT_4(ksn_total) - ksn_total;

        size_t file_size = (8 + kfh_section_size) +
                           ktn_total +
                           (8 + kmc_section_size + kmc_padding) +
                           (8 + kmi_section_size + kmi_padding) +
                           ksn_total + ksn_padding +
                           KWZ_SIGNATURE_SIZE;

        u8 *file_data = (u8 *)calloc(file_size, 1);
        if (!file_data) { res = UGOMEMO_MEMORY_ERROR; goto cleanup_ref; }

        size_t pos = 0;

        // ---- KFH Section ----
        if (ref_ctx && ref_ctx->file && ref_ctx->meta && ref_ctx->meta->kfh_offset == 0) {
            // Copy the entire KFH section from reference
            memcpy(file_data + pos, ref_ctx->file->data, 8 + kfh_section_size);
            // Overwrite frame count with our count
            write_le16(file_data, pos + 8 + 0xC4, (u16)frame_count);
            // Recompute KFH CRC32
            u32 kfh_crc = get_crc32(file_data + pos + 8 + 4, kfh_section_size - 4);
            write_le32(file_data, pos + 8, kfh_crc);
            pos += 8 + kfh_section_size;
        } else {
            memcpy(file_data + pos, "KFH\x14", 4); pos += 4;
            write_le32(file_data, pos, kfh_section_size); pos += 4;
            size_t kfh_data_start = pos;
            pos += 4;  // CRC32 placeholder
            pos += 12; // timestamps + app version
            pos += 30; // author IDs
            pos += 66; // author names
            pos += 84; // filenames
            write_le16(file_data, pos, (u16)frame_count); pos += 2;
            write_le16(file_data, pos, 0); pos += 2;  // thumbnail index
            write_le16(file_data, pos, 0x02); pos += 2;  // flags
            file_data[pos++] = 8;  // speed
            file_data[pos++] = 0;  // visibility
            u32 kfh_crc = get_crc32(file_data + kfh_data_start + 4, kfh_section_size - 4);
            write_le32(file_data, kfh_data_start, kfh_crc);
        }

        // ---- KTN Section ----
        if (has_ref_ktn) {
            // Copy the entire KTN section (header + data) from reference
            size_t ref_ktn_start = ref_ctx->meta->ktn_offset;
            memcpy(file_data + pos, ref_ctx->file->data + ref_ktn_start, ktn_total);
            pos += ktn_total;
        } else {
            memcpy(file_data + pos, "KTN\x02", 4); pos += 4;
            write_le32(file_data, pos, ktn_section_size); pos += 4;
            write_le32(file_data, pos, 0); pos += 4;  // CRC32 of nothing
        }

        // ---- KMC Section ----
        memcpy(file_data + pos, "KMC\x02", 4); pos += 4;
        write_le32(file_data, pos, kmc_section_size); pos += 4;
        size_t kmc_crc_pos = pos;
        pos += 4;  // CRC32 placeholder
        memcpy(file_data + pos, kmc_data, kmc_total);
        u32 kmc_crc = get_crc32(file_data + kmc_crc_pos + 4, (uint)kmc_total);
        write_le32(file_data, kmc_crc_pos, kmc_crc);
        pos += kmc_total;
        pos += kmc_padding;

        // ---- KMI Section ----
        memcpy(file_data + pos, "KMI\x05", 4); pos += 4;
        write_le32(file_data, pos, kmi_section_size); pos += 4;

        bool has_ref_kmi = ref_ctx && ref_ctx->video && ref_ctx->video->kmi;

        for (int i = 0; i < frame_count; i++) {
            if (frame_metas[i].la_size > 0xFFFF || frame_metas[i].lb_size > 0xFFFF || frame_metas[i].lc_size > 0xFFFF) {
                ERROR("Compressed layer exceeds KWZ format limit of 65535 bytes\n");
                free(file_data);
                res = UGOMEMO_INPUT_ERROR;
                goto cleanup_ref;
            }
            write_le32(file_data, pos, frame_metas[i].flags); pos += 4;
            write_le16(file_data, pos, (u16)frame_metas[i].la_size); pos += 2;
            write_le16(file_data, pos, (u16)frame_metas[i].lb_size); pos += 2;
            write_le16(file_data, pos, (u16)frame_metas[i].lc_size); pos += 2;

            if (has_ref_kmi && (uint)i < ref_ctx->meta->frame_count) {
                // Copy frame author ID from reference (raw bytes at KMI offset)
                kwz_kmi_entry *re = &ref_ctx->video->kmi[i];
                // Author ID: write the raw 10 bytes from the reference file
                size_t ref_entry_offset = ref_ctx->meta->kmi_offset + 8 + (i * 28) + 10;
                memcpy(file_data + pos, ref_ctx->file->data + ref_entry_offset, 10);
                pos += 10;
                // Layer depths
                file_data[pos++] = re->layer_a_depth;
                file_data[pos++] = re->layer_b_depth;
                file_data[pos++] = re->layer_c_depth;
                // SFX flags
                file_data[pos++] = re->sfx_flags;
                // Unknown + camera flags
                write_le16(file_data, pos, re->unknown); pos += 2;
                write_le16(file_data, pos, re->camera_flags); pos += 2;
            } else {
                pos += 10;  // author ID
                pos += 3;   // depths
                pos += 1;   // SFX flags
                pos += 4;   // unknown + camera
            }
        }
        pos += kmi_padding;

        // ---- KSN Section ----
        if (build_ksn) {
            memcpy(file_data + pos, "KSN\x01", 4); pos += 4;
            write_le32(file_data, pos, ksn_section_size); pos += 4;
            write_le32(file_data, pos, 8); pos += 4;  // recorded speed
            write_le32(file_data, pos, audio->bgm ? audio->bgm_size : 0); pos += 4;
            write_le32(file_data, pos, audio->se1 ? audio->se1_size : 0); pos += 4;
            write_le32(file_data, pos, audio->se2 ? audio->se2_size : 0); pos += 4;
            write_le32(file_data, pos, audio->se3 ? audio->se3_size : 0); pos += 4;
            write_le32(file_data, pos, audio->se4 ? audio->se4_size : 0); pos += 4;
            // CRC32 placeholder - compute after writing audio data
            size_t ksn_crc_pos = pos;
            pos += 4;
            size_t audio_data_start = pos;
            if (audio->bgm && audio->bgm_size > 0) { memcpy(file_data + pos, audio->bgm, audio->bgm_size); pos += audio->bgm_size; }
            if (audio->se1 && audio->se1_size > 0) { memcpy(file_data + pos, audio->se1, audio->se1_size); pos += audio->se1_size; }
            if (audio->se2 && audio->se2_size > 0) { memcpy(file_data + pos, audio->se2, audio->se2_size); pos += audio->se2_size; }
            if (audio->se3 && audio->se3_size > 0) { memcpy(file_data + pos, audio->se3, audio->se3_size); pos += audio->se3_size; }
            if (audio->se4 && audio->se4_size > 0) { memcpy(file_data + pos, audio->se4, audio->se4_size); pos += audio->se4_size; }
            u32 ksn_crc = (ksn_audio_size > 0) ? get_crc32(file_data + audio_data_start, ksn_audio_size) : 0;
            write_le32(file_data, ksn_crc_pos, ksn_crc);
            pos += ksn_padding;
        } else if (has_ref_ksn) {
            size_t ref_ksn_start = ref_ctx->meta->ksn_offset;
            size_t ref_ksn_total = 8 + ksn_section_size;
            memcpy(file_data + pos, ref_ctx->file->data + ref_ksn_start, ref_ksn_total);
            pos += ref_ksn_total;
            pos += ksn_padding;
        } else {
            memcpy(file_data + pos, "KSN\x01", 4); pos += 4;
            write_le32(file_data, pos, ksn_section_size); pos += 4;
            write_le32(file_data, pos, 8); pos += 4;  // recorded speed
            pos += 20;  // 5 track sizes (all 0)
            write_le32(file_data, pos, 0); pos += 4;  // CRC32
        }

        // ---- Signature ----
        if (pos + KWZ_SIGNATURE_SIZE > file_size) {
            ERROR("File assembly exceeded calculated size\n");
            free(file_data);
            res = UGOMEMO_INTERNAL_ERROR;
            goto cleanup_ref;
        }
        if (key && key->private_exponent) {
            res = rsa_generate_signature(key, file_data, pos, file_data + pos, true);
            if (res != UGOMEMO_OK) res = UGOMEMO_OK;  // non-fatal
        }
        pos += KWZ_SIGNATURE_SIZE;

        *out_data = file_data;
        *out_size = pos;
    }

cleanup_ref:
    free(ref_rgb);
    free(ref_la); free(ref_lb); free(ref_lc);
    free(ref_pla); free(ref_plb); free(ref_plc);

cleanup:
    free(pixels);
    free(layer_a); free(layer_b); free(layer_c);
    free(comp_buf);
    free(kmc_data);
    free(frame_metas);
    return res;
}

int kwz_encode_frames_command(const char *bmp_dir, const char *output_path,
                              const char *reference_kwz_path,
                              const char *bgm_wav,
                              const ugomemo_rsa_key *key) {
    int res = UGOMEMO_OK;
    u8 *encoded_data = NULL;
    size_t encoded_size = 0;
    kwz_ctx *ref_ctx = NULL;
    ugomemo_file *ref_input = NULL;
    kwz_audio_tracks audio = {0};
    bool has_audio = false;

    if (reference_kwz_path) {
        res = file_init(&ref_input, (char *)reference_kwz_path, false);
        if (res != UGOMEMO_OK) return res;
        res = file_read(ref_input);
        if (res != UGOMEMO_OK) { file_free(&ref_input); return res; }

        ref_ctx = (kwz_ctx *)calloc(1, sizeof(kwz_ctx));
        if (!ref_ctx) { file_free(&ref_input); return UGOMEMO_MEMORY_ERROR; }
        ref_ctx->file = ref_input;

        res = kwz_process(ref_ctx);
        if (res != UGOMEMO_OK) {
            ERROR("Failed to parse reference KWZ\n");
            kwz_cleanup(ref_ctx);
            file_free(&ref_input);
            return res;
        }
    }

    // Encode WAV to KWZ ADPCM if provided
    if (bgm_wav) {
        i16 *pcm = NULL;
        int count = 0;
        if (read_wav_samples(bgm_wav, &pcm, &count) == UGOMEMO_OK) {
            kwz_encode_audio(pcm, count, 0, &audio.bgm, &audio.bgm_size);
            free(pcm);
            has_audio = true;
        }
    }

    res = kwz_encode_frames(bmp_dir, &encoded_data, &encoded_size, ref_ctx,
                            has_audio ? &audio : NULL, key);
    if (res != UGOMEMO_OK) {
        ERROR("Failed to encode frames\n");
        if (ref_ctx) kwz_cleanup(ref_ctx);
        if (ref_input) file_free(&ref_input);
        return res;
    }

    if (output_path) {
        ugomemo_file *output = NULL;
        res = file_init(&output, (char *)output_path, true);
        if (res != UGOMEMO_OK) {
            free(encoded_data);
            if (ref_ctx) kwz_cleanup(ref_ctx);
            if (ref_input) file_free(&ref_input);
            return res;
        }
        file_write(output, encoded_data, encoded_size);
        file_free(&output);
        fprintf(stderr, "Wrote %zu bytes to %s\n", encoded_size, output_path);
    }

    free(encoded_data);
    free(audio.bgm);
    free(audio.se1);
    free(audio.se2);
    free(audio.se3);
    free(audio.se4);
    if (ref_ctx) kwz_cleanup(ref_ctx);
    if (ref_input) file_free(&ref_input);
    return res;
}
