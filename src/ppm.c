#include <string.h>

#include <ugomemo.h>

static const u8 PPM_MAGIC[4] = { 'P', 'A', 'R', 'A' };

// 0 = processed fine, 1 = invalid middle characters (set to #), 2 = out of range edit count (set to 999)
int ppm_format_filename(char *dest, u8 *data, u32 offset, u16 *edit_count) {
    uint str_offset = 0;
    int res = UGOMEMO_OK;
    u64 i;
    char c;

    // MAC fragment (3 bytes)
    for (i = 0; i < 3; i++) {
        snprintf(dest + str_offset, 3, "%02X", data[offset + i]);
        str_offset += 2;
    }

    dest[str_offset++] = '_';

    // 13-character string, replacing non uppercase hex characters with #
    for (i = offset + 3; i < offset + 3 + 13; i++){
        c = data[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
            res = 1;
            dest[str_offset++] = '#';
        } else {
            dest[str_offset++] = c;
        }
    }

    dest[str_offset++] = '_';

    // Edit count, setting out of range values to 999
    u16 edits = read_le16(data, offset + 16);
    if (edits < 999) {
        snprintf(dest + str_offset, 4, "%03u", edits);
    } else {
        snprintf(dest + str_offset, 4, "%03u", 999);
    }
    if (edit_count != NULL) *edit_count = edits;

    return res;
}

int ppm_decode_filename(u8 *data, char **output, bool fragment) {
    int res = UGOMEMO_OK;
    uint str_offset = 0;
    char *dest;

    if (!fragment) {
        /// Filename is normal, 24 characters + null terminator
        dest = (char *) calloc(PPM_FILENAME_STR_LENGTH + 1, sizeof(char));
        res = ppm_format_filename(dest, data, 0, NULL);
    } else {
        /// Filename is a fragment
        // <MAC fragment (3 bytes)>_<5 bytes as the first 10 characters>\0
        dest = (char *) calloc(6 + 1 + 10 + 1, sizeof(char));
        for (uint i = 0; i < 8; i++) {
            if (i == 3) dest[str_offset++] = '_';
            snprintf(dest + str_offset, 3, "%02X", data[i]);
            str_offset += 2;
        }
    }

    *output = dest;
    return res;
}

int ppm_decode_fsid(u8 *data, char **output) {
    int res = UGOMEMO_OK;
    char *fsid = (char *) calloc((8 * 2) + 1, sizeof(char));
    uint str_offset = 0;

    // REVERSE order bytes printed as uppercase hex
    for (uint i = PPM_FSID_LENGTH; i > 0; i--) {
        snprintf(fsid + str_offset, 3, "%02X", data[i - 1]);
        str_offset += 2;
    }

    *output = fsid;

    return res;
}

// Both file header and metadata. 0 = success, 1 = error
// Errors are specifically for when they would affect file traversal
static int ppm_read_meta(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;
    ppm_meta *m;
    u8 *temp = NULL;
    u16 temp16;
    u32 temp32;

    m = (ppm_meta *) calloc(1, sizeof(ppm_meta));
    if (m == NULL) {
        ERROR("Failed to allocate memory for PPM meta ctx.\n");
        return UGOMEMO_MEMORY_ERROR;
    }
    ctx->meta = m;

    /// File header
    // First four bytes are the PARA magic
    READ_OR_ERR(file_read_buf, ctx->file, &temp, 4);
    if (memcmp(temp, (u8 *)PPM_MAGIC, 4) != 0) {
        CTX_ERROR(ctx, "File magic invalid");
        return UGOMEMO_INPUT_ERROR;  // Can't confidently say it's PPM
    }

    // u32 animation data size
    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    m->animation_data_size = temp32;
    if (temp32 > ctx->file->size) {
        CTX_ERROR(ctx, "Animation data size exceeds file size");
        res = UGOMEMO_INPUT_ERROR;
    } else if (temp32 > 736800) {
        CTX_NOTICE(ctx, "Animation data size exceeds app constant of 736800");
    }

    // u32 sound data size
    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    m->sound_data_size = temp32;
    if (8 + m->animation_data_size + temp32 + 128 > ctx->file->size) {
        CTX_ERROR(ctx, "Sound data size field exceeds the end of the file");
        res = UGOMEMO_INPUT_ERROR; // The above math is a rough approximation meant to catch big exceptions
    }

    // u16 frame count beginning at 0 (adding 1 to get a human understood number)
    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (temp16 != 0xFFFF) temp16 += 1;  // Avoid overflowing an invalid count
    if (temp16 > 1000) {
        CTX_ERROR(ctx, "Frame count is greater than 999");
        res = UGOMEMO_INPUT_ERROR;
    } else m->frame_count = temp16;

    // u16 format version that's always 0x24
    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (temp16 != 0x24) {
        CTX_ERROR(ctx, "Format version is not 0x24");
        res = UGOMEMO_INPUT_ERROR;
    } else m->format_version = temp16;

    /// End file header, begin metadata section
    // u16 lock flag
    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (!(m->lock == 0 || m->lock == 1)) {
        CTX_WARNING(ctx, "Lock flag is not 0 or 1");
        res = UGOMEMO_INPUT_ERROR;
    } else m->lock = temp16;

    // u16 thumbnail frame index (also starts at 0)
    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (temp16 != 0xFFFF) temp16 += 1;  // Avoid overflowing an invalid count
    if (temp16 > m->frame_count || temp16 == 0xFFFF) {
        CTX_ERROR(ctx, "Thumbnail frame index invalid (exceeds frame count or is 0xFFFF)"); return 1;
    } else m->thumbnail_frame_index = temp16;

    // wchar[11] UTF-16LE root author name
    READ_OR_ERR(file_read_buf, ctx->file, &temp, FLIPNOTE_USERNAME_LENGTH);
    m->root_username = utf16le_to_ascii(temp, FLIPNOTE_USERNAME_LENGTH);

    // wchar[11] UTF-16LE parent author name
    READ_OR_ERR(file_read_buf, ctx->file, &temp, FLIPNOTE_USERNAME_LENGTH);
    m->parent_username = utf16le_to_ascii(temp, FLIPNOTE_USERNAME_LENGTH);

    // wchar[11] UTF-16LE current author name
    READ_OR_ERR(file_read_buf, ctx->file, &temp, FLIPNOTE_USERNAME_LENGTH);
    m->current_username = utf16le_to_ascii(temp, FLIPNOTE_USERNAME_LENGTH);

    // 8 byte (reverse order) parent FSID
    READ_OR_ERR(file_read_buf, ctx->file, &temp, PPM_FSID_LENGTH);
    ppm_decode_fsid(temp, &m->parent_fsid);

    // 8 byte (reverse order) current FSID
    READ_OR_ERR(file_read_buf, ctx->file, &temp, PPM_FSID_LENGTH);
    ppm_decode_fsid(temp, &m->current_fsid);

    // 18 byte parent filename
    READ_OR_ERR(file_read_buf, ctx->file, &temp, PPM_FILENAME_LENGTH);
    ppm_decode_filename(temp, &m->parent_file_name, false);

    // 18 byte current filename
    READ_OR_ERR(file_read_buf, ctx->file, &temp, PPM_FILENAME_LENGTH);
    ppm_decode_filename(temp, &m->current_file_name, false);

    // 8 byte (reverse order) root FSID
    READ_OR_ERR(file_read_buf, ctx->file, &temp, PPM_FSID_LENGTH);
    ppm_decode_fsid(temp, &m->root_fsid);

    // 8 byte root filename fragment
    READ_OR_ERR(file_read_buf, ctx->file, &temp, PPM_FILENAME_FRAGMENT_LENGTH);
    ppm_decode_filename(temp, &m->root_file_name_fragment, true);

    // u32 last modified timestamp
    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    m->timestamp = ((u64)temp32) + DSI_EPOCH;  // Overflows happen frequently

    // u16 padding
    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    if (temp16 != 0) {
        CTX_WARNING(ctx, "Post meta section padding is not 0x0000");
        m->padding = temp16;
    }

    return res;
}

static int ppm_read_thumbmail(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;
    u8 *thumbnail_data;

    READ_OR_ERR(file_read_buf, ctx->file, &thumbnail_data, 0x600);
    ctx->thumbnail_data = thumbnail_data;

    return res;
}

static int ppm_read_animation_header(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;
    u16 temp16;
    u32 temp32;
    u8 *temp;
    uint extra_bytes;
    ppm_meta *m = ctx->meta;

    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    // TODO: bounds checking
    m->frame_offset_table_size = temp16;

    // Derive the frame data size from animation data size
    if (m->animation_data_size < m->frame_offset_table_size + PPM_ANIM_HEADER_SIZE) {
        CTX_ERROR(ctx, "Animation data size too small for offset table");
        return UGOMEMO_INPUT_ERROR;
    }
    m->frame_data_size = m->animation_data_size - m->frame_offset_table_size - PPM_ANIM_HEADER_SIZE;

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    if (temp32 != 0) {
        CTX_WARNING(ctx, "Frame offset table unknown value isn't 0");
        m->frame_offset_table_unknown = temp32;
    }

    READ_OR_ERR(file_read_le16, ctx->file, &temp16);
    // TODO: find if anything can be checked
    m->frame_offset_table_flags = temp16;

    READ_OR_ERR(file_read_buf, ctx->file, &ctx->frame_offset_table, m->frame_offset_table_size);

    // For memory alignment
    extra_bytes = UGO_ROUND_UP_MULT_4(m->frame_offset_table_size) - m->frame_offset_table_size;
    if (extra_bytes != 0) {
        READ_OR_ERR(file_read_buf, ctx->file, &temp, extra_bytes);
        m->animation_header_padding = temp;
        m->animation_header_padding_len = extra_bytes;
    }

    return res;
}

static int ppm_read_animation_data(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;
    u8 *temp;
    uint extra_bytes;
    ppm_meta *m = ctx->meta;

    READ_OR_ERR(file_read_buf, ctx->file, &ctx->animation_data, ctx->meta->frame_data_size);

    // For memory alignment
    extra_bytes = UGO_ROUND_UP_MULT_4(m->frame_data_size) - m->frame_data_size;
    if (extra_bytes != 0) {
        READ_OR_ERR(file_read_buf, ctx->file, &temp, extra_bytes);
        m->animation_data_padding = temp;
        m->animation_data_padding_len = extra_bytes;
    }

    return res;
}

static int ppm_read_sfx_flags(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;
    uint extra_bytes;
    u8 *temp;
    ppm_meta *m = ctx->meta;

    READ_OR_ERR(file_read_buf, ctx->file, &ctx->sfx_flags, ctx->meta->frame_count);

    // The frame count is very rarely a perfect multiple of 4, and the app likes to be aligned
    extra_bytes = UGO_ROUND_UP_MULT_4(m->frame_count) - m->frame_count;
    if (extra_bytes != 0) {
        READ_OR_ERR(file_read_buf, ctx->file, &temp, extra_bytes);
        m->sfx_flags_padding = temp;
        m->sfx_flags_padding_len = extra_bytes;
    }

    return res;
}

static int ppm_read_sound(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;
    ppm_sound *s;
    u32 temp32;
    u8 temp8;
    u8 *temp;

    s = (ppm_sound *) calloc(1, sizeof(ppm_sound));
    if (s == NULL) {
        ERROR("Failed to allocate memory for ppm sound ctx.\n");
        return UGOMEMO_MEMORY_ERROR;
    }
    ctx->sound = s;

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    // current_offset has advanced by 4 from reading bgm_size, so subtract 4
    s->bgm_offset = ctx->file->current_offset + PPM_SOUND_HEADER_SIZE - 4;
    s->bgm_size = temp32;
    if ((size_t)temp32 + s->bgm_offset > ctx->file->size) {
        CTX_WARNING(ctx, "BGM extends past the end of the file");
    }

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    if ((size_t)temp32 + s->bgm_offset + s->bgm_size > ctx->file->size) {
        CTX_WARNING(ctx, "SE1 extends past the end of the file");
    }
    s->se1_size = temp32;
    s->se1_offset = s->bgm_offset + s->bgm_size;
    if (s->se1_offset > ctx->file->size) {
        CTX_ERROR(ctx, "SE1 offset exceeds file size");
        return UGOMEMO_INPUT_ERROR;
    }

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    if ((size_t)temp32 + s->se1_offset + s->se1_size > ctx->file->size) {
        CTX_WARNING(ctx, "SE2 extends past the end of the file");
    }
    s->se2_size = temp32;
    s->se2_offset = s->se1_offset + s->se1_size;
    if (s->se2_offset > ctx->file->size) {
        CTX_ERROR(ctx, "SE2 offset exceeds file size");
        return UGOMEMO_INPUT_ERROR;
    }

    READ_OR_ERR(file_read_le32, ctx->file, &temp32);
    if ((size_t)temp32 + s->se2_offset + s->se2_size > ctx->file->size) {
        CTX_WARNING(ctx, "SE3 extends past the end of the file");
    }
    s->se3_size = temp32;
    s->se3_offset = s->se2_offset + s->se2_size;
    if (s->se3_offset > ctx->file->size) {
        CTX_ERROR(ctx, "SE3 offset exceeds file size");
        return UGOMEMO_INPUT_ERROR;
    }

    READ_OR_ERR(file_read_le8, ctx->file, &temp8);
    // TODO: check the range (weird subtraction from 8 thing)
    s->frame_speed = temp8;

    READ_OR_ERR(file_read_le8, ctx->file, &temp8);
    // TODO: check the range (weird subtraction from 8 thing)
    s->frame_speed_when_recorded = temp8;

    READ_OR_ERR(file_read_buf, ctx->file, &temp, 14);
    for (uint i = 0; i < 14; i++) {
        if (temp[i] != 0x00) {
            CTX_WARNING(ctx, "Sound section padding contains non-null byte(s)");
            s->padding_bytes = temp;
            s->has_padding_error = true;
            break;
        }
    }

    // Using this to consistently increment current offset counter
    READ_OR_ERR(file_read_buf, ctx->file, &s->bgm, s->bgm_size);
    READ_OR_ERR(file_read_buf, ctx->file, &s->se1, s->se1_size);
    READ_OR_ERR(file_read_buf, ctx->file, &s->se2, s->se2_size);
    READ_OR_ERR(file_read_buf, ctx->file, &s->se3, s->se3_size);

    return res;
}

    static int ppm_finish_reading_file(ppm_ctx *ctx) {
        int res = UGOMEMO_OK;
        ppm_signature *s;

        s = (ppm_signature *) calloc(1, sizeof(ppm_signature));
        if (s == NULL) {
            ERROR("Failed to allocate memory for ppm signature.\n");
            return UGOMEMO_MEMORY_ERROR;
        }
        ctx->signature = s;

        s->decrypted_signature = (u8 *) calloc(PPM_SIGNATURE_SIZE, sizeof(u8));
        if (s->decrypted_signature == NULL) {
            ERROR("Failed to allocate memory for decrypted signature.\n");
            return UGOMEMO_MEMORY_ERROR;
        }

        s->sha1_computed = (u8 *) calloc(1, SHA1_SIZE);
        if (s->sha1_computed == NULL) {
            ERROR("Failed to allocate memory for PPM signature sha256.\n");
            return UGOMEMO_MEMORY_ERROR;
        }

        READ_OR_ERR(file_read_buf, ctx->file, &s->encrypted_signature, PPM_SIGNATURE_SIZE);

        READ_OR_ERR(file_read_buf, ctx->file, &s->padding_bytes, 16);
        for (uint i = 0; i < 16; i++) {
            if (s->padding_bytes[i] != 0x00) {
                CTX_WARNING(ctx, "Signature padding bytes contain non-null value");
                break;
            }
        }

        return res;
    }

void ppm_cleanup(ppm_ctx **ctx) {
    if (ctx == NULL || *ctx == NULL) return;

    ppm_meta *m = (*ctx)->meta;
    if (m != NULL) {
        free(m->current_username);
        free(m->current_fsid);
        free(m->current_file_name);
        free(m->parent_username);
        free(m->parent_fsid);
        free(m->parent_file_name);
        free(m->root_username);
        free(m->root_fsid);
        free(m->root_file_name_fragment);
        free(m);
    }

    ppm_sound *s = (*ctx)->sound;
    if (s != NULL) {
        free(s);
    }

    ppm_signature *sig = (*ctx)->signature;
    if (sig != NULL) {
        free(sig->decrypted_signature);
        free(sig->sha1_computed);
        free(sig);
    }

    ugomemo_error_list_free(&(*ctx)->errors);

    free(*ctx);
    *ctx = NULL;
}

int ppm_verify_file(ppm_ctx *ctx, const ugomemo_rsa_key *key) {
    ppm_signature *s = ctx->signature;
    ugomemo_file *file = ctx->file;
    ugomemo_rsa_key builtin;
    int res;

    if (s == NULL || file == NULL) return UGOMEMO_INPUT_ERROR;
    if (file->size < PPM_SIGNATURE_SIZE + 16) return UGOMEMO_INPUT_ERROR;

    if (!key) {
        ugomemo_ppm_public_key(&builtin);
        key = &builtin;
    }

    if (file_sha1(file, 0, file->size - PPM_SIGNATURE_SIZE - 16, s->sha1_computed) != UGOMEMO_OK)
        return UGOMEMO_INPUT_ERROR;

    rsa_decrypt(key, s->encrypted_signature, s->decrypted_signature);

    res = rsa_verify_signature(key, s->sha1_computed, s->decrypted_signature);
    s->valid = (res == UGOMEMO_OK);
    if (!s->valid)
        s->verify_res = res - 10;

    return UGOMEMO_OK;
}

#define _RUN(fn)  do {                     \
        res = fn(ctx);                     \
        if (res != UGOMEMO_OK) return  res; \
} while (0)

int ppm_read_file(ppm_ctx *ctx) {
    int res = UGOMEMO_OK;

    _RUN(ppm_read_meta);
    _RUN(ppm_read_thumbmail);
    _RUN(ppm_read_animation_header);
    _RUN(ppm_read_animation_data);
    _RUN(ppm_read_sfx_flags);
    _RUN(ppm_read_sound);
    _RUN(ppm_finish_reading_file);

    return res;
}

// Convenience API for FFI

ppm_ctx *ppm_open(const char *path) {
    ugomemo_file *input = NULL;
    int res = file_init(&input, (char *)path, false);
    if (res != UGOMEMO_OK) return NULL;

    res = file_read(input);
    if (res != UGOMEMO_OK) { file_free(&input); return NULL; }

    ppm_ctx *ctx = (ppm_ctx *) calloc(1, sizeof(ppm_ctx));
    if (!ctx) { file_free(&input); return NULL; }
    ctx->file = input;

    res = ppm_read_file(ctx);
    if (res != UGOMEMO_OK) {
        ppm_cleanup(&ctx);
        file_free(&input);
        return NULL;
    }

    ppm_verify_file(ctx, NULL);

    return ctx;
}

void ppm_close(ppm_ctx *ctx) {
    if (!ctx) return;
    ugomemo_file *f = ctx->file;
    ppm_cleanup(&ctx);
    if (f) file_free(&f);
}

u16 ppm_get_frame_count(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->frame_count : 0;
}

u8 ppm_get_frame_speed(const ppm_ctx *ctx) {
    return (ctx && ctx->sound) ? ctx->sound->frame_speed : 0;
}

u64 ppm_get_timestamp(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->timestamp : 0;
}

u16 ppm_get_lock(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->lock : 0;
}

const char *ppm_get_root_username(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->root_username : NULL;
}

const char *ppm_get_root_fsid(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->root_fsid : NULL;
}

const char *ppm_get_parent_username(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->parent_username : NULL;
}

const char *ppm_get_parent_fsid(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->parent_fsid : NULL;
}

const char *ppm_get_parent_filename(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->parent_file_name : NULL;
}

const char *ppm_get_current_username(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->current_username : NULL;
}

const char *ppm_get_current_fsid(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->current_fsid : NULL;
}

const char *ppm_get_current_filename(const ppm_ctx *ctx) {
    return (ctx && ctx->meta) ? ctx->meta->current_file_name : NULL;
}

u32 ppm_get_track_size(const ppm_ctx *ctx, int track) {
    if (!ctx || !ctx->sound) return 0;
    switch (track) {
        case 0: return ctx->sound->bgm_size;
        case 1: return ctx->sound->se1_size;
        case 2: return ctx->sound->se2_size;
        case 3: return ctx->sound->se3_size;
        default: return 0;
    }
}

u32 ppm_get_error_count(const ppm_ctx *ctx) {
    return ctx ? ugomemo_error_count(&ctx->errors) : 0;
}

const char *ppm_get_error(const ppm_ctx *ctx, u32 index, int *severity) {
    return ctx ? ugomemo_error_get(&ctx->errors, index, severity) : NULL;
}

const u8 *ppm_get_file_data(const ppm_ctx *ctx, size_t *size) {
    if (!ctx || !ctx->file) { if (size) *size = 0; return NULL; }
    if (size) *size = ctx->file->size;
    return ctx->file->data;
}

bool ppm_get_signature_valid(const ppm_ctx *ctx) {
    return (ctx && ctx->signature) ? ctx->signature->valid : false;
}
