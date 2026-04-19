#include <stdlib.h>
#include <string.h>

#include <ugomemo.h>

// Shared UGAR magic for .ugo, .nbf, .npf
static const u8 UGAR_MAGIC[4] = { 'U', 'G', 'A', 'R' };

// XOR cipher key for FS2D .pls/.lst (64 bytes)
static const u8 FS2D_LST_KEY[64] = {
    0xF7, 0x4C, 0x6A, 0x3A, 0xFB, 0x82, 0xA6, 0x37,
    0x6E, 0x11, 0x38, 0xCF, 0xA0, 0xDD, 0x85, 0xC0,
    0xC7, 0x9B, 0xC4, 0xD8, 0xDD, 0x28, 0x8A, 0x87,
    0x53, 0x20, 0xEE, 0xE0, 0x0B, 0xEB, 0x43, 0xA0,
    0xDB, 0x55, 0x0F, 0x75, 0x36, 0x37, 0xEB, 0x35,
    0x6A, 0x34, 0x7F, 0xB5, 0x0F, 0x99, 0xF7, 0xEF,
    0x43, 0x25, 0xCE, 0xA0, 0x29, 0x46, 0xD9, 0xD4,
    0x4D, 0xBB, 0x04, 0x66, 0x68, 0x08, 0xF1, 0xF8,
};

// XOR cipher key for FS3D .lst (61 bytes)
static const u8 FS3D_LST_KEY[61] = {
    0xF7, 0x4C, 0x6A, 0x3A, 0xFB, 0x82, 0xA6, 0x37,
    0x6E, 0x11, 0x38, 0xCF, 0xA0, 0xDD, 0x85, 0xC0,
    0xC7, 0x9B, 0xC4, 0xD8, 0xDD, 0x28, 0x8A, 0x87,
    0x53, 0x20, 0xEE, 0xE0, 0x0B, 0xEB, 0x43, 0xA0,
    0xDB, 0x55, 0x0F, 0x75, 0x36, 0x37, 0xEB, 0x35,
    0x6A, 0x34, 0x7F, 0xB5, 0x0F, 0x99, 0xF7, 0xEF,
    0x43, 0x25, 0xCE, 0xA0, 0x29, 0x46, 0xD9, 0xD4,
    0x4D, 0xBB, 0x04, 0x66, 0x68,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// .ugo parser
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ugo_parse(u8 *data, size_t size, ugo_file *out) {
    if (size < 16) return UGOMEMO_INPUT_ERROR;
    if (memcmp(data, UGAR_MAGIC, 4) != 0) return UGOMEMO_INPUT_ERROR;

    out->section_count = read_le32(data, 4);
    out->palette_data_length = read_le32(data, 8);
    out->image_data_length = read_le32(data, 12);

    size_t menu_offset = 16;
    out->menu_data_length = out->palette_data_length;
    if (menu_offset + out->menu_data_length > size) return UGOMEMO_INPUT_ERROR;
    out->menu_data = data + menu_offset;

    size_t embed_offset = menu_offset + out->palette_data_length;
    out->embed_data_length = out->image_data_length;
    if (embed_offset + out->embed_data_length > size)
        out->embed_data_length = size - embed_offset;
    out->embed_data = data + embed_offset;

    return UGOMEMO_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// .nbf parser and decoder
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Convert RGB555 to RGB24
static inline rgb24_pixel rgb555_to_rgb24(u16 color) {
    rgb24_pixel p;
    p.red   = ((color >> 0) & 0x1F) << 3;
    p.green = ((color >> 5) & 0x1F) << 3;
    p.blue  = ((color >> 10) & 0x1F) << 3;
    return p;
}

int nbf_parse(u8 *data, size_t size, nbf_file *out) {
    if (size < 16) return UGOMEMO_INPUT_ERROR;
    if (memcmp(data, UGAR_MAGIC, 4) != 0) return UGOMEMO_INPUT_ERROR;

    out->section_count = read_le32(data, 4);
    out->palette_length = read_le32(data, 8);
    out->image_length = read_le32(data, 12);

    if (16 + out->palette_length + out->image_length > size) return UGOMEMO_INPUT_ERROR;

    out->palette_data = data + 16;
    out->image_data = data + 16 + out->palette_length;
    out->color_count = out->palette_length / 2;

    return UGOMEMO_OK;
}

int nbf_decode(nbf_file *nbf, rgb24_pixel *output) {
    for (int i = 0; i < NBF_WIDTH * NBF_HEIGHT; i++) {
        u8 idx = nbf->image_data[i];
        if (idx >= nbf->color_count) {
            output[i] = (rgb24_pixel){0, 0, 0};
        } else {
            u16 color = read_le16(nbf->palette_data, idx * 2);
            output[i] = rgb555_to_rgb24(color);
        }
    }
    return UGOMEMO_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// .npf parser and decoder
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int npf_parse(u8 *data, size_t size, npf_file *out) {
    if (size < 16) return UGOMEMO_INPUT_ERROR;
    if (memcmp(data, UGAR_MAGIC, 4) != 0) return UGOMEMO_INPUT_ERROR;

    out->section_count = read_le32(data, 4);
    out->palette_length = read_le32(data, 8);
    out->image_length = read_le32(data, 12);

    if (16 + out->palette_length + out->image_length > size) return UGOMEMO_INPUT_ERROR;

    out->palette_data = data + 16;
    out->image_data = data + 16 + out->palette_length;
    out->color_count = out->palette_length / 2;

    // Infer dimensions from image data length (each byte = 2 pixels)
    u32 total_pixels = out->image_length * 2;
    out->width = 0;
    out->height = 0;

    if (total_pixels == 32 * 32) {
        out->width = 32; out->height = 32;
    } else if (total_pixels == 64 * 48) {
        out->width = 64; out->height = 48;
    } else if (total_pixels == 256 * 192) {
        out->width = 256; out->height = 192;
    } else {
        for (int w = 256; w >= 8; w /= 2) {
            if (total_pixels % w == 0) {
                out->width = w;
                out->height = total_pixels / w;
                break;
            }
        }
    }

    return UGOMEMO_OK;
}

int npf_decode(npf_file *npf, rgb24_pixel *output) {
    int pixel = 0;
    int total = npf->width * npf->height;

    for (u32 i = 0; i < npf->image_length && pixel < total; i++) {
        u8 byte = npf->image_data[i];
        // Nibbles reversed: high nibble = pixel #1, low nibble = pixel #2
        u8 idx1 = byte >> 4;
        u8 idx2 = byte & 0x0F;

        if (pixel < total) {
            if (idx1 == 0) {
                output[pixel] = (rgb24_pixel){0, 0, 0};
            } else if (idx1 < npf->color_count) {
                u16 color = read_le16(npf->palette_data, idx1 * 2);
                output[pixel] = rgb555_to_rgb24(color);
            }
            pixel++;
        }
        if (pixel < total) {
            if (idx2 == 0) {
                output[pixel] = (rgb24_pixel){0, 0, 0};
            } else if (idx2 < npf->color_count) {
                u16 color = read_le16(npf->palette_data, idx2 * 2);
                output[pixel] = rgb555_to_rgb24(color);
            }
            pixel++;
        }
    }
    return UGOMEMO_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FS2D .pls/.lst XOR cipher
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int fs2d_lst_decrypt(u8 *data, size_t size, u8 **output, size_t *output_len) {
    if (size < 2) return UGOMEMO_INPUT_ERROR;

    size_t data_len = size - 2;
    u8 *decrypted = (u8 *) calloc(data_len + 1, sizeof(u8));
    if (decrypted == NULL) return UGOMEMO_MEMORY_ERROR;

    for (size_t i = 0; i < data_len; i++) {
        decrypted[i] = data[i] ^ FS2D_LST_KEY[i % 64];
    }
    decrypted[data_len] = '\0';

    size_t content_len = data_len;
    while (content_len > 0 && decrypted[content_len - 1] == '\0')
        content_len--;

    *output = decrypted;
    *output_len = content_len;
    return UGOMEMO_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FS3D .lst XOR cipher
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int fs3d_lst_decrypt(u8 *data, size_t size, u8 **output, size_t *output_len) {
    if (size < 6) return UGOMEMO_INPUT_ERROR;

    size_t data_len = size - 6;
    u8 *decrypted = (u8 *) calloc(data_len + 2, sizeof(u8));
    if (decrypted == NULL) return UGOMEMO_MEMORY_ERROR;

    for (size_t i = 0; i < data_len; i++) {
        decrypted[i] = data[6 + i] ^ FS3D_LST_KEY[i % 61];
    }
    decrypted[data_len] = '\0';

    size_t content_len = data_len;
    while (content_len > 0 && decrypted[content_len - 1] == '\0')
        content_len--;

    *output = decrypted;
    *output_len = content_len;
    return UGOMEMO_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// .kwzpcf parser
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int kwzpcf_parse(u8 *data, size_t size, kwzpcf_file *out) {
    size_t pos = 0;
    int capacity = 16;

    out->entries = (kwzpcf_entry *) calloc(capacity, sizeof(kwzpcf_entry));
    if (out->entries == NULL) return UGOMEMO_MEMORY_ERROR;
    out->entry_count = 0;

    while (pos + 8 <= size) {
        u8 *magic = data + pos;
        u32 section_size = read_le32(data, pos + 4);
        pos += 8;

        if (pos + section_size > size) break;

        if (out->entry_count >= capacity) {
            capacity *= 2;
            kwzpcf_entry *new_entries = realloc(out->entries, capacity * sizeof(kwzpcf_entry));
            if (new_entries == NULL) return UGOMEMO_MEMORY_ERROR;
            out->entries = new_entries;
        }

        kwzpcf_entry *e = &out->entries[out->entry_count];
        memset(e, 0, sizeof(kwzpcf_entry));

        if (memcmp(magic, "KPCF", 4) == 0) {
            e->is_cert = false;
            if (section_size >= 512) {
                e->url = (char *) calloc(513, sizeof(char));
                if (e->url) memcpy(e->url, data + pos, 512);
                e->data = data + pos + 512;
                e->data_size = section_size - 512;
            }
        } else if (memcmp(magic, "KDER", 4) == 0) {
            e->is_cert = true;
            e->url = NULL;
            e->data = data + pos;
            e->data_size = section_size;
        } else {
            pos += section_size;
            continue;
        }

        out->entry_count++;
        pos += section_size;
    }

    return UGOMEMO_OK;
}

void kwzpcf_free(kwzpcf_file *f) {
    if (f->entries != NULL) {
        for (int i = 0; i < f->entry_count; i++) {
            free(f->entries[i].url);
        }
        free(f->entries);
    }
}
