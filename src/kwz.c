#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ugomemo.h>

static const u8 KFH_MAGIC[4] = { 'K', 'F', 'H', 0x14 };
static const u8 KTN_MAGIC[4] = { 'K', 'T', 'N', 0x02 };
static const u8 KSN_MAGIC[4] = { 'K', 'S', 'N', 0x01 };
static const u8 KMI_MAGIC[4] = { 'K', 'M', 'I', 0x05 };
static const u8 KMC_MAGIC[4] = { 'K', 'M', 'C', 0x02 };
#define MAGIC_SIZE 4

#define KFH_SECTION 0
#define KTN_SECTION 1
#define KSN_SECTION 2
#define KMI_SECTION 3
#define KMC_SECTION 4
#define INVALID_SECTION -1

static inline int kwz_get_section_type(u8 *magic) {
    if (memcmp(magic, KFH_MAGIC, MAGIC_SIZE) == 0)
        return KFH_SECTION;
    else if (memcmp(magic, KTN_MAGIC, MAGIC_SIZE) == 0)
        return KTN_SECTION;
    else if (memcmp(magic, KSN_MAGIC, MAGIC_SIZE) == 0)
        return KSN_SECTION;
    else if (memcmp(magic, KMI_MAGIC, MAGIC_SIZE) == 0)
        return KMI_SECTION;
    else if (memcmp(magic, KMC_MAGIC, MAGIC_SIZE) == 0)
        return KMC_SECTION;
    else
        return INVALID_SECTION;
}

int kwz_decode_fsid(u8 *data, char **fsid_p, char **fsid_ppm_p, bool decode_ppm) {
    int res = UGOMEMO_OK;
    char *fsid;
    char *fsid_ppm;
    uint str_offset = 0;

    fsid = (char *) calloc(21, sizeof(char));
    if (fsid == NULL) {
        ERROR("Failed to allocate memory for KWZ FSID.\n");
        return UGOMEMO_MEMORY_ERROR;
    }

    for (uint i = 0; i < 10; i++) {
        snprintf(fsid + str_offset, 3, "%02X", data[i]);
        str_offset += 2;
    }
    *fsid_p = fsid;

    if (decode_ppm == false) return res;

    fsid_ppm = (char *) calloc(17, sizeof(char));
    if (fsid_ppm == NULL) {
        ERROR("Failed to allocate memory for PPM format KWZ FSID.\n");
        return UGOMEMO_MEMORY_ERROR;
    }

    str_offset = 0;
    for (uint j = 8; j > 0; j--) {
        snprintf(fsid_ppm + str_offset, 3, "%02X", data[j]);
        str_offset += 2;
    }
    *fsid_ppm_p = fsid_ppm;

    return res;
}

static bool is_valid_ppm_filename(u8 *data) {
    for (uint i = 3; i < PPM_FILENAME_LENGTH - 2; i++) {
        char c = data[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
            return false;
    }

    u16 edit_count = read_le16(data, PPM_FILENAME_LENGTH - 2);
    if (edit_count > 999)
        return false;

    for (uint i = PPM_FILENAME_LENGTH; i < KWZ_FILENAME_LENGTH; i++) {
        if (data[i] != 0x00)
            return false;
    }

    return true;
}

int kwz_decode_filename(u8 *data, char **name) {
    // Check for valid KWZ filename (0-5, a-z)
    bool is_kwz = true;
    for (uint i = 0; i < KWZ_FILENAME_LENGTH; i++) {
        char c = data[i];
        if (!((c >= '0' && c <= '5') || (c >= 'a' && c <= 'z'))) {
            is_kwz = false;
            break;
        }
    }

    if (is_kwz) {
        *name = (char *) calloc(KWZ_FILENAME_LENGTH + 1, sizeof(char));
        memcpy(*name, data, KWZ_FILENAME_LENGTH);
        return UGOMEMO_OK;
    }

    if (is_valid_ppm_filename(data)) {
        *name = (char *) calloc(PPM_FILENAME_STR_LENGTH + 1, sizeof(char));
        int res = ppm_format_filename(*name, data, 0, NULL);
        if (res != UGOMEMO_OK) {
            ERROR("FATAL: unhandled PPM filename decoding error\n");
            exit(UGOMEMO_INTERNAL_ERROR);
        }
        return 1;
    }

    // Invalid format - return as hex
    *name = (char *) calloc(2 + (KWZ_FILENAME_LENGTH * 2) + 1, sizeof(char));
    (*name)[0] = '0';
    (*name)[1] = 'x';
    for (uint i = 0; i < KWZ_FILENAME_LENGTH; i++) {
        snprintf((*name) + 2 + (i * 2), 3, "%02X", data[i]);
    }
    return 2;
}

int kwz_process_ksn(kwz_ctx *ctx, u32 section_size) {
    int res = UGOMEMO_OK;
    u8 *temp, *data = ctx->file->data;
    kwz_sound *sound = ctx->sound;
    kwz_meta *meta = ctx->meta;
    u32 crc32, calculated_crc32;
    uint extra_bytes;

    meta->ksn_offset = file_get_offset(ctx->file) - 8;
    meta->ksn_size = section_size;

    READ_OR_ERR(file_read_le32, ctx->file, &sound->recorded_speed);
    if (sound->recorded_speed > 10) CTX_WARNING(ctx, "KSN recorded speed too high");

    READ_OR_ERR(file_read_le32, ctx->file, &sound->bgm_size);
    if (sound->bgm_size > section_size) CTX_WARNING(ctx, "KSN BGM size invalid");

    READ_OR_ERR(file_read_le32, ctx->file, &sound->se1_size);
    if (sound->se1_size > section_size) CTX_WARNING(ctx, "KSN SE1 size invalid");

    READ_OR_ERR(file_read_le32, ctx->file, &sound->se2_size);
    if (sound->se2_size > section_size) CTX_WARNING(ctx, "KSN SE2 size invalid");

    READ_OR_ERR(file_read_le32, ctx->file, &sound->se3_size);
    if (sound->se3_size > section_size) CTX_WARNING(ctx, "KSN SE3 size invalid");

    READ_OR_ERR(file_read_le32, ctx->file, &sound->se4_size);
    if (sound->se4_size > section_size) CTX_WARNING(ctx, "KSN SE4 size invalid");

    READ_OR_ERR(file_read_le32, ctx->file, &crc32);
    meta->ksn_crc32 = crc32;

    sound->bgm_offset = file_get_offset(ctx->file);
    READ_OR_ERR(file_read_buf, ctx->file, &sound->bgm, sound->bgm_size);
    sound->se1_offset = sound->bgm_offset + sound->bgm_size;
    READ_OR_ERR(file_read_buf, ctx->file, &sound->se1, sound->se1_size);
    sound->se2_offset = sound->se1_offset + sound->se1_size;
    READ_OR_ERR(file_read_buf, ctx->file, &sound->se2, sound->se2_size);
    sound->se3_offset = sound->se2_offset + sound->se2_size;
    READ_OR_ERR(file_read_buf, ctx->file, &sound->se3, sound->se3_size);
    sound->se4_offset = sound->se3_offset + sound->se3_size;
    READ_OR_ERR(file_read_buf, ctx->file, &sound->se4, sound->se4_size);

    size_t offset = file_get_offset(ctx->file);
    extra_bytes = UGO_ROUND_UP_MULT_4(offset) - offset;
    READ_OR_ERR(file_read_buf, ctx->file, &temp, extra_bytes);

    if (extra_bytes != 0) {
        meta->ksn_extra_bytes = temp;
        meta->ksn_extra_bytes_len = extra_bytes;
    }

    calculated_crc32 = get_crc32(
        data + meta->ksn_offset + KSN_HEADER_SIZE,
        section_size - KSN_HEADER_SIZE + 8
    );
    meta->ksn_crc32_calculated = calculated_crc32;

    if (crc32 != calculated_crc32) {
        CTX_WARNING(ctx, "KSN section crc32 check failed");
    }

    return res;
}

int kwz_process_ktn(kwz_ctx *ctx, u32 section_size) {
    int res = UGOMEMO_OK;
    u8 *temp;
    kwz_meta *meta = ctx->meta;
    size_t offset, data_offset;
    u32 crc32, calculated_crc32;

    offset = file_get_offset(ctx->file);

    meta->ktn_offset = offset - 8;
    meta->ktn_size = section_size;

    READ_OR_ERR(file_read_le32, ctx->file, &crc32);
    meta->ktn_crc32 = crc32;

    data_offset = file_get_offset(ctx->file);
    READ_OR_ERR(file_crc32, ctx->file, data_offset, section_size - 4, &calculated_crc32);
    meta->ktn_crc32_calculated = calculated_crc32;
    READ_OR_ERR(file_sha256, ctx->file, data_offset, section_size - 4, meta->ktn_sha256);

    READ_OR_ERR(file_read_buf, ctx->file, &temp, section_size - 4);
    ctx->ktn_data = temp;

    if (crc32 != calculated_crc32) {
        CTX_WARNING(ctx, "KTN section crc32 check failed");
    }

    return res;
}

int kwz_process_kmc(kwz_ctx *ctx, u32 section_size) {
    int res = UGOMEMO_OK;
    u8 *temp;
    kwz_meta *meta = ctx->meta;
    size_t offset, data_offset;
    u32 crc32, calculated_crc32;
    uint extra_bytes;

    offset = file_get_offset(ctx->file);

    meta->kmc_offset = offset - 8;
    meta->kmc_size = section_size;

    READ_OR_ERR(file_read_le32, ctx->file, &crc32);
    meta->kmc_crc32 = crc32;

    data_offset = file_get_offset(ctx->file);
    READ_OR_ERR(file_crc32, ctx->file, data_offset, section_size - 4, &calculated_crc32);
    meta->kmc_crc32_calculated = calculated_crc32;
    READ_OR_ERR(file_sha256, ctx->file, data_offset, section_size - 4, meta->kmc_sha256);

    READ_OR_ERR(file_read_buf, ctx->file, &temp, section_size - 4);
    ctx->kmc_data = temp;

    offset = file_get_offset(ctx->file);
    extra_bytes = UGO_ROUND_UP_MULT_4(offset) - offset;
    READ_OR_ERR(file_read_buf, ctx->file, &temp, extra_bytes);

    if (extra_bytes != 0) {
        meta->kmc_extra_bytes = temp;
        meta->kmc_extra_bytes_len = extra_bytes;
    }

    if (crc32 != calculated_crc32) {
        CTX_WARNING(ctx, "KMC section crc32 check failed");
    }

    return res;
}

// Expects file object to be set up appropriately
static inline int kwz_decode_kmi(kwz_ctx *ctx) {
    int res = UGOMEMO_OK;
    kwz_video *v = ctx->video;
    kwz_kmi_entry *e;
    u32 temp32;
    u16 temp16;
    u8 temp8;
    u8 *temp;

    if (v == NULL)
        v = (kwz_video *) calloc(1, sizeof(kwz_video));
    if (v == NULL) {
        ERROR("Failed to allocate memory for KWZ video.\n");
        return UGOMEMO_MEMORY_ERROR;
    }
    ctx->video = v;

    v->kmi = (kwz_kmi_entry *) calloc(ctx->meta->frame_count + 1, sizeof(kwz_kmi_entry));
    if (v->kmi == NULL) {
        ERROR("Failed to allocate memory for KMI entries.\n");
        return UGOMEMO_MEMORY_ERROR;
    }

    for (uint i = 0; i < ctx->meta->frame_count; i++) {
        e = &(v->kmi[i]);

        // TODO: check these for bad values
        READ_OR_ERR(file_read_le32, ctx->file, &temp32);
        e->flags = temp32;

        READ_OR_ERR(file_read_le16, ctx->file, &temp16);
        e->layer_a_size = temp16;
        READ_OR_ERR(file_read_le16, ctx->file, &temp16);
        e->layer_b_size = temp16;
        READ_OR_ERR(file_read_le16, ctx->file, &temp16);
        e->layer_c_size = temp16;

        READ_OR_ERR(file_read_buf, ctx->file, &temp, 10);
        kwz_decode_fsid(temp, &e->fsid, NULL, false);

        READ_OR_ERR(file_read_le8, ctx->file, &temp8);
        e->layer_a_depth = temp8;
        READ_OR_ERR(file_read_le8, ctx->file, &temp8);
        e->layer_b_depth = temp8;
        READ_OR_ERR(file_read_le8, ctx->file, &temp8);
        e->layer_c_depth = temp8;

        READ_OR_ERR(file_read_le8, ctx->file, &temp8);
        e->sfx_flags = temp8;

        READ_OR_ERR(file_read_le16, ctx->file, &temp16);
        e->unknown = temp16;
        READ_OR_ERR(file_read_le16, ctx->file, &temp16);
        e->camera_flags = temp16;
    }

    return res;
}

int kwz_process_kmi(kwz_ctx *ctx, u32 section_size) {
    int res = UGOMEMO_OK;
    kwz_meta *meta = ctx->meta;
    u8 *temp;
    uint extra_bytes;
    size_t data_offset, end_offset;

    meta->kmi_offset = file_get_offset(ctx->file) - 8;
    meta->kmi_size = section_size;

    data_offset = file_get_offset(ctx->file);

    res = kwz_decode_kmi(ctx);
    if (res != UGOMEMO_OK) {
        ERROR("Failed to decode KMI entries.\n");
    }

    end_offset = file_get_offset(ctx->file);
    extra_bytes = UGO_ROUND_UP_MULT_4(end_offset) - end_offset;
    READ_OR_ERR(file_read_buf, ctx->file, &temp, extra_bytes);

    if (extra_bytes != 0) {
        meta->kmi_extra_bytes = temp;
        meta->kmi_extra_bytes_len = extra_bytes;
    }

    READ_OR_ERR(file_sha256, ctx->file, data_offset, section_size, meta->kmi_sha256);

    return res;
}

static void process_kwz_filename_res_error(kwz_ctx *ctx, int type, bool classified) {
    switch (type) {
        case 0:  // root
            if (classified) {
                CTX_WARNING(ctx, "Root filename is PPM format");
            } else {
                CTX_WARNING(ctx, "Root filename is unknown format");
            } break;
        case 1:  // parent
            if (classified) {
                CTX_WARNING(ctx, "Parent filename is PPM format");
            } else {
                CTX_WARNING(ctx, "Parent filename is unknown format");
            } break;
        case 2:  // current
            if (classified) {
                CTX_WARNING(ctx, "Current filename is PPM format");
            } else {
                CTX_WARNING(ctx, "Current filename is unknown format");
            } break;
        default:
            CTX_WARNING(ctx, "Unhandled filename error");
            break;
    }
}

static void process_kwz_filename_res(kwz_ctx *ctx, int res, int type) {
    switch (res) {
        case UGOMEMO_OK: break;  // Valid filename
        case 1:
            process_kwz_filename_res_error(ctx, type, true);
            break;
        case 2:
            process_kwz_filename_res_error(ctx, type, false);
            break;
        default:
            process_kwz_filename_res_error(ctx, -1, false);
            break;
    }
}

int kwz_process_kfh(kwz_ctx *ctx, u32 section_size) {
    int res = UGOMEMO_OK;
    kwz_meta *meta = ctx->meta;
    u8 *temp;
    u8 temp8;
    u16 temp16;
    u32 temp32;
    size_t offset;
    u32 crc32, calculated_crc32;

    offset = file_get_offset(ctx->file);
    meta->kfh_offset = offset - 8;
    meta->kfh_size = section_size;

    if (meta->kfh_offset != 0) {
        ERROR("KFH does not start at offset 0, invalid.\n");
        return UGOMEMO_INPUT_ERROR;
    }

    if (section_size != KFH_SECTION_SIZE) {
        CTX_ERROR(ctx, "KFH section size invalid");
        return UGOMEMO_INPUT_ERROR;  // Cannot safely proceed in general
    }

    READ_OR_ERR(file_read_le32, ctx->file, &crc32);
    if (file_crc32(ctx->file, offset + 4, KFH_SECTION_SIZE - 4, &calculated_crc32) != UGOMEMO_OK) {
        CTX_ERROR(ctx, "KFH section CRC32 out of bounds");
        return UGOMEMO_INPUT_ERROR;
    }

    meta->kfh_crc32_calculated = calculated_crc32;

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    meta->creation_timestamp = ((u64)temp32) + DSI_EPOCH;

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    meta->modified_timestamp = ((u64)temp32) + DSI_EPOCH;

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    meta->app_version = temp32;

    READ_OR_ERR(file_read_buf, ctx->file, &temp, KWZ_FSID_LENGTH);
    kwz_decode_fsid(temp, &meta->root_fsid, &meta->root_fsid_ppm, true);
    READ_OR_ERR(file_read_buf, ctx->file, &temp, KWZ_FSID_LENGTH);
    kwz_decode_fsid(temp, &meta->parent_fsid, &meta->parent_fsid_ppm, true);
    READ_OR_ERR(file_read_buf, ctx->file, &temp, KWZ_FSID_LENGTH);
    kwz_decode_fsid(temp, &meta->current_fsid, &meta->current_fsid_ppm, true);

    READ_OR_ERR(file_read_buf, ctx->file, &temp, FLIPNOTE_USERNAME_LENGTH);
    meta->root_username = utf16le_to_ascii(temp, 11);
    READ_OR_ERR(file_read_buf, ctx->file, &temp, FLIPNOTE_USERNAME_LENGTH);
    meta->parent_username = utf16le_to_ascii(temp, 11);
    READ_OR_ERR(file_read_buf, ctx->file, &temp, FLIPNOTE_USERNAME_LENGTH);
    meta->current_username = utf16le_to_ascii(temp, 11);

    READ_OR_ERR(file_read_buf, ctx->file, &temp, KWZ_FILENAME_LENGTH);
    res = kwz_decode_filename(temp, &meta->root_filename);
    process_kwz_filename_res(ctx, res, 0);
    READ_OR_ERR(file_read_buf, ctx->file, &temp, KWZ_FILENAME_LENGTH);
    res = kwz_decode_filename(temp, &meta->parent_filename);
    process_kwz_filename_res(ctx, res, 1);
    READ_OR_ERR(file_read_buf, ctx->file, &temp, KWZ_FILENAME_LENGTH);
    res = kwz_decode_filename(temp, &meta->current_filename);
    process_kwz_filename_res(ctx, res, 2);
    res = UGOMEMO_OK;  // handled above, reset

    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (temp16 > 999) {
        CTX_ERROR(ctx, "Frame count higher than possible");
        res = UGOMEMO_INPUT_ERROR;
    }
    meta->frame_count = temp16;

    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (temp16 != 0xFFFF) temp16 += 1;  // Human-preferred frame index only when overflow won't happen
    if (temp16 > meta->frame_count) CTX_WARNING(ctx, "Thumbnail frame index outside frame range");
    meta->thumbnail_frame_index = temp16;

    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    meta->flags = temp16;

    READ_OR_ERR(file_read_le8, ctx->file, &temp8);
    meta->frame_speed = temp8;

    READ_OR_ERR(file_read_le8, ctx->file, &temp8);
    meta->layer_vis_flags = temp8;

    // No extra bytes in this section

    if (crc32 != calculated_crc32) {
        CTX_WARNING(ctx, "KFH section crc32 check failed");
    } else meta->crc32 = crc32;

    return res;
}

int kwz_process_signature(kwz_ctx *ctx) {
    // Store the raw signature for later verification by the caller
    ctx->decrypted_signature = NULL;
    ctx->signature_sha256 = (u8 *) calloc(SHA256_SIZE, sizeof(u8));
    if (ctx->signature_sha256 == NULL) return UGOMEMO_MEMORY_ERROR;

    if (file_sha256(ctx->file, 0, ctx->file->size - KWZ_SIGNATURE_SIZE, ctx->signature_sha256) != UGOMEMO_OK)
        return UGOMEMO_INPUT_ERROR;

    return UGOMEMO_OK;
}

int kwz_verify_file(kwz_ctx *ctx, const ugomemo_rsa_key *key) {
    u8 decrypted[KWZ_SIGNATURE_SIZE];
    ugomemo_rsa_key builtin;

    if (ctx->signature_sha256 == NULL) return UGOMEMO_INPUT_ERROR;

    if (!key) {
        ugomemo_kwz_public_key(&builtin);
        key = &builtin;
    }

    rsa_decrypt(key, ctx->file->data + ctx->file->size - KWZ_SIGNATURE_SIZE, decrypted);

    ctx->decrypted_signature = (u8 *) calloc(KWZ_SIGNATURE_SIZE, sizeof(u8));
    if (ctx->decrypted_signature == NULL) return UGOMEMO_MEMORY_ERROR;
    memcpy(ctx->decrypted_signature, decrypted, KWZ_SIGNATURE_SIZE);

    return rsa_verify_signature(key, ctx->signature_sha256, decrypted);
}

static inline int kwz_process_init(kwz_ctx *ctx) {
    int res = UGOMEMO_OK;

    ctx->meta = (kwz_meta *) calloc(1, sizeof(kwz_meta));
    if (ctx->meta == NULL) {
        ERROR("Failed to allocate memory for kwz meta.\n");
        return UGOMEMO_MEMORY_ERROR;
    }

    ctx->sound = (kwz_sound *) calloc(1, sizeof(kwz_sound));
    if (ctx->sound == NULL) {
        ERROR("Failed to allocate memory for kwz sound.\n");
        free(ctx->meta);
        return UGOMEMO_MEMORY_ERROR;
    }

    // Compute file-level SHA256
    sha256_hash(ctx->file->data, ctx->file->size, ctx->file_sha256);

    return res;
}

static inline int check_for_missing_sections(kwz_ctx *ctx) {
    int res = UGOMEMO_OK;

    if (!ctx->kfh_parsed) {
        CTX_ERROR(ctx, "KFH section wasn't parsed");
        res = UGOMEMO_INPUT_ERROR;
    }

    if (!ctx->kmc_parsed) {
        CTX_ERROR(ctx, "KMC section wasn't parsed");
        res = UGOMEMO_INPUT_ERROR;
    }

    if (!ctx->kmi_parsed) {
        CTX_ERROR(ctx, "KMI section wasn't parsed");
        res = UGOMEMO_INPUT_ERROR;
    }

    if (!ctx->ksn_parsed) {
        CTX_NOTICE(ctx, "KSN section wasn't parsed (normal for .kwc)");
    }

    if (!ctx->ktn_parsed) {
        CTX_NOTICE(ctx, "KTN section wasn't parsed (normal for .kwc)");
    }

    return res;
}

int kwz_process(kwz_ctx *ctx) {
    int res = UGOMEMO_OK;
    bool running = true, processed = false;
    u8 *section_magic = NULL;
    u32 section_size = 0;
    size_t offset, file_size;

    res = kwz_process_init(ctx);
    if (res != UGOMEMO_OK) return res;

    file_size = ctx->file->size;

    while (running) {
        offset = file_get_offset(ctx->file);
        if (offset == file_size - KWZ_SIGNATURE_SIZE) {
            res = kwz_process_signature(ctx);
            if (res != UGOMEMO_OK) return res;
            break;
        }

        // Sections start with char[4] magic then u32 section size
        READ_OR_ERR(file_read_buf, ctx->file, &section_magic, MAGIC_SIZE);
        READ_OR_ERR(file_read_le32, ctx->file, &section_size);

        // Check section size relative to file size
        if (file_test_size(ctx->file, section_size)) {
            ERROR("Section size would run off end of the file. %u > %zu\n",
            section_size, ctx->file->size);
            return UGOMEMO_INPUT_ERROR;
        }

        processed = true;
        switch (kwz_get_section_type(section_magic)) {
            case KFH_SECTION:
                res = kwz_process_kfh(ctx, section_size);
                ctx->kfh_parsed = true;
                break;
            case KTN_SECTION:
                // Extra bytes are in the .jpg dsi library files
                section_size = UGO_ROUND_UP_MULT_4(section_size);
                res = kwz_process_ktn(ctx, section_size);
                ctx->ktn_parsed = true;
                break;
            case KSN_SECTION:
                res = kwz_process_ksn(ctx, section_size);
                ctx->ksn_parsed = true;
                break;
            case KMI_SECTION:
                res = kwz_process_kmi(ctx, section_size);
                ctx->kmi_parsed = true;
                break;
            case KMC_SECTION:
                res = kwz_process_kmc(ctx, section_size);
                ctx->kmc_parsed = true;
                break;
            default:
                ERROR("Failed to find next section! Bad section sizes?\n");
                ERROR("DEBUG: magic: 0x%02X%02X%02X%02X section_size=%u, offset=%lu, file_size=%lu\n",
                section_magic[0], section_magic[1], section_magic[2], section_magic[3],
                section_size, file_get_offset(ctx->file), ctx->file->size);
                processed = false;
                break;
        }
        if (res != UGOMEMO_OK) return res;
        if (processed == false) return UGOMEMO_INPUT_ERROR;
    }

    res = check_for_missing_sections(ctx);
    if (res != UGOMEMO_OK) return res;

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void kwz_cleanup_kmi(kwz_ctx *ctx, u32 frame_count) {
    if (ctx->video == NULL || ctx->video->kmi == NULL) {
        return;
    }

    for (u32 i = 0; i < frame_count; i++) {
        if (ctx->video->kmi[i].fsid != NULL) {
            free(ctx->video->kmi[i].fsid);
            ctx->video->kmi[i].fsid = NULL;
        }
    }

    free(ctx->video->kmi);
    ctx->video->kmi = NULL;
}

void kwz_cleanup(kwz_ctx *ctx) {
    kwz_meta *meta;
    kwz_sound *sound;
    u32 frame_count = 0;

    if (ctx == NULL) return;

    free(ctx->decrypted_signature);
    free(ctx->signature_sha256);

    sound = ctx->sound;
    if (sound != NULL) {
        free(sound);
    }

    meta = ctx->meta;
    if (meta != NULL) {
        frame_count = meta->frame_count;
        free(meta->current_username);
        free(meta->parent_username);
        free(meta->root_username);
        free(meta->current_filename);
        free(meta->parent_filename);
        free(meta->root_filename);
        free(meta->current_fsid);
        free(meta->parent_fsid);
        free(meta->root_fsid);
        free(meta->current_fsid_ppm);
        free(meta->parent_fsid_ppm);
        free(meta->root_fsid_ppm);
        free(meta);
    }

    kwz_cleanup_kmi(ctx, frame_count);
    free(ctx->video);

    ugomemo_error_list_free(&ctx->errors);

    free(ctx);
}

// Convenience API for FFI

kwz_ctx *kwz_open(const char *path) {
    ugomemo_file *input = NULL;
    int res = file_init(&input, (char *)path, false);
    if (res != UGOMEMO_OK) return NULL;

    res = file_read(input);
    if (res != UGOMEMO_OK) { file_free(&input); return NULL; }

    kwz_ctx *ctx = (kwz_ctx *) calloc(1, sizeof(kwz_ctx));
    if (!ctx) { file_free(&input); return NULL; }
    ctx->file = input;

    res = kwz_process(ctx);
    if (res != UGOMEMO_OK) {
        kwz_cleanup(ctx);
        file_free(&input);
        return NULL;
    }

    kwz_verify_file(ctx, NULL);

    return ctx;
}

u16 kwz_get_frame_count(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->frame_count : 0;
}

u8 kwz_get_frame_speed(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->frame_speed : 0;
}

u64 kwz_get_creation_timestamp(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->creation_timestamp : 0;
}

u64 kwz_get_modified_timestamp(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->modified_timestamp : 0;
}

u32 kwz_get_flags(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->flags : 0;
}

const char *kwz_get_root_username(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->root_username : NULL;
}

const char *kwz_get_root_fsid(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->root_fsid : NULL;
}

const char *kwz_get_root_filename(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->root_filename : NULL;
}

const char *kwz_get_parent_username(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->parent_username : NULL;
}

const char *kwz_get_parent_fsid(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->parent_fsid : NULL;
}

const char *kwz_get_parent_filename(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->parent_filename : NULL;
}

const char *kwz_get_current_username(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->current_username : NULL;
}

const char *kwz_get_current_fsid(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->current_fsid : NULL;
}

const char *kwz_get_current_filename(const kwz_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->current_filename : NULL;
}

u32 kwz_get_track_size(const kwz_ctx *ctx, int track) {
    if (!ctx || !ctx->sound) return 0;
    switch (track) {
        case 0: return ctx->sound->bgm_size;
        case 1: return ctx->sound->se1_size;
        case 2: return ctx->sound->se2_size;
        case 3: return ctx->sound->se3_size;
        case 4: return ctx->sound->se4_size;
        default: return 0;
    }
}

u32 kwz_get_error_count(const kwz_ctx *ctx) {
    return ctx ? ugomemo_error_count(&ctx->errors) : 0;
}

const char *kwz_get_error(const kwz_ctx *ctx, u32 index, int *severity) {
    return ctx ? ugomemo_error_get(&ctx->errors, index, severity) : NULL;
}

const u8 *kwz_get_file_data(const kwz_ctx *ctx, size_t *size) {
    if (!ctx || !ctx->file) { if (size) *size = 0; return NULL; }
    if (size) *size = ctx->file->size;
    return ctx->file->data;
}

bool kwz_get_signature_valid(const kwz_ctx *ctx) {
    if (!ctx || !ctx->decrypted_signature || !ctx->signature_sha256) return false;
    ugomemo_rsa_key key;
    ugomemo_kwz_public_key(&key);
    return rsa_verify_signature(&key, ctx->signature_sha256, ctx->decrypted_signature) == UGOMEMO_OK;
}
