#include <stdlib.h>
#include <string.h>

#include <ugomemo.h>

#include <ugomemo/ppm_video_tables.h>

// ---- Decoding ----

int ppm_decode_thumbnail(u8 *buffer, rgb24_pixel *output_buffer) {
    u8 tile_y, tile_x, line, pixel;

    buffer += 0xA0;

    for (tile_y = 0; tile_y < 48; tile_y += 8)
    for (tile_x = 0; tile_x < 64; tile_x += 8)
    for (line   = 0; line   <  8; line   += 1)
    for (pixel  = 0; pixel  <  8; pixel  += 2) {
        output_buffer[((47 - (tile_y + line)) * 64) + (tile_x + pixel)]     = PPM_THUMBNAIL_PALETTE[*buffer & 0xF];
        output_buffer[((47 - (tile_y + line)) * 64) + (tile_x + pixel + 1)] = PPM_THUMBNAIL_PALETTE[*buffer >> 0x4];
        buffer++;
    }

    return UGOMEMO_OK;
}

int ppm_framerate_lookup(uint index, float *framerate) {
    if (index > 8) return 1;
    *framerate = PPM_FRAMERATES[index];
    return UGOMEMO_OK;
}

// Decompress a single PPM layer from the frame data
// layer_out: 256*192 u8 buffer (0=transparent, 1=pen color)
static int ppm_decompress_layer(u8 *layer_out, u8 *data, size_t data_size, size_t *offset, u8 *line_encodings) {
    size_t pos = *offset;

    for (int y = 0; y < PPM_FRAME_HEIGHT; y++) {
        u8 encoding = line_encodings[y];
        u8 *line = &layer_out[y * PPM_FRAME_WIDTH];

        switch (encoding) {
            case 0:
                // Empty line
                memset(line, 0, PPM_FRAME_WIDTH);
                break;

            case 1: {
                // Chunk-flagged compression
                memset(line, 0, PPM_FRAME_WIDTH);
                if (pos + 4 > data_size) return UGOMEMO_INPUT_ERROR;
                u32 chunk_flags = read_be32(data, pos);
                pos += 4;

                int pixel = 0;
                for (int c = 0; c < 32; c++) {
                    if (chunk_flags & 0x80000000) {
                        if (pos >= data_size) return UGOMEMO_INPUT_ERROR;
                        u8 chunk = data[pos++];
                        for (int bit = 0; bit < 8; bit++) {
                            line[pixel++] = (chunk >> bit) & 1;
                        }
                    } else {
                        pixel += 8;
                    }
                    chunk_flags <<= 1;
                }
                break;
            }

            case 2: {
                // Same as type 1 but pre-filled with 1s
                memset(line, 1, PPM_FRAME_WIDTH);
                if (pos + 4 > data_size) return UGOMEMO_INPUT_ERROR;
                u32 chunk_flags = read_be32(data, pos);
                pos += 4;

                int pixel = 0;
                for (int c = 0; c < 32; c++) {
                    if (chunk_flags & 0x80000000) {
                        if (pos >= data_size) return UGOMEMO_INPUT_ERROR;
                        u8 chunk = data[pos++];
                        for (int bit = 0; bit < 8; bit++) {
                            line[pixel++] = (chunk >> bit) & 1;
                        }
                    } else {
                        pixel += 8;
                    }
                    chunk_flags <<= 1;
                }
                break;
            }

            case 3: {
                // All chunks present, no flags
                int pixel = 0;
                while (pixel < PPM_FRAME_WIDTH) {
                    if (pos >= data_size) return UGOMEMO_INPUT_ERROR;
                    u8 chunk = data[pos++];
                    for (int bit = 0; bit < 8; bit++) {
                        line[pixel++] = (chunk >> bit) & 1;
                    }
                }
                break;
            }
        }
    }

    *offset = pos;
    return UGOMEMO_OK;
}

int ppm_decode_frame(ppm_ctx *ctx, uint frame_index, rgb24_pixel *output,
                     u8 *prev_layer_1, u8 *prev_layer_2,
                     u8 *layer_1_out, u8 *layer_2_out) {
    u8 *data = ctx->file->data;
    u8 *frame_offset_table = ctx->frame_offset_table;
    size_t anim_data_start;
    size_t frame_offset;
    size_t pos;
    u8 header;
    u8 paper_color, layer_1_color, layer_2_color;
    u8 frame_type, translate_flag;
    i8 translate_x = 0, translate_y = 0;
    u8 line_enc_1[192], line_enc_2[192];

    if (frame_index >= ctx->meta->frame_count) return UGOMEMO_INPUT_ERROR;

    // Animation data starts after the header section
    // Frame offset table starts at 0x06A8 within the file
    // Animation data starts after the offset table (rounded up to mult of 4)
    anim_data_start = 0x06A0 + 8 +
        UGO_ROUND_UP_MULT_4(ctx->meta->frame_offset_table_size) +
        ctx->meta->frame_offset_table_size;

    // Wait - the animation data offset calculation needs to account for the actual layout:
    // 0x06A0 = end of thumbnail = start of animation header
    // animation header = 8 bytes (u16 table_size + u32 unknown + u16 flags)
    // then offset table (table_size bytes)
    // then padding to mult of 4
    // then frame data
    anim_data_start = 0x06A0 + 8 + UGO_ROUND_UP_MULT_4(ctx->meta->frame_offset_table_size);

    // Read frame offset from table (u32 LE values)
    frame_offset = read_le32(frame_offset_table, frame_index * 4);
    pos = anim_data_start + frame_offset;

    if (pos >= ctx->file->size) return UGOMEMO_INPUT_ERROR;

    // Read frame header
    header = data[pos++];
    frame_type = (header >> 7) & 1;
    translate_flag = (header >> 5) & 3;
    layer_2_color = (header >> 3) & 3;
    layer_1_color = (header >> 1) & 3;
    paper_color = header & 1;

    if (frame_type == 0 && translate_flag != 0) {
        translate_x = (i8)data[pos++];
        translate_y = (i8)data[pos++];
    }

    // Read line encoding types (48 bytes per layer)
    for (int i = 0; i < 48; i++) {
        u8 byte = data[pos++];
        for (int b = 0; b < 8; b += 2) {
            int idx = i * 4 + b / 2;
            if (idx < 192)
                line_enc_1[idx] = (byte >> b) & 3;
        }
    }
    for (int i = 0; i < 48; i++) {
        u8 byte = data[pos++];
        for (int b = 0; b < 8; b += 2) {
            int idx = i * 4 + b / 2;
            if (idx < 192)
                line_enc_2[idx] = (byte >> b) & 3;
        }
    }

    // Decompress layers
    int dec_res;
    dec_res = ppm_decompress_layer(layer_1_out, data, ctx->file->size, &pos, line_enc_1);
    if (dec_res != UGOMEMO_OK) return dec_res;
    dec_res = ppm_decompress_layer(layer_2_out, data, ctx->file->size, &pos, line_enc_2);
    if (dec_res != UGOMEMO_OK) return dec_res;

    // Apply frame diffing (XOR with previous frame)
    if (frame_type == 0 && prev_layer_1 != NULL && prev_layer_2 != NULL) {
        for (int y = 0; y < PPM_FRAME_HEIGHT; y++) {
            int prev_y = y - translate_y;
            if (prev_y < 0 || prev_y >= PPM_FRAME_HEIGHT) continue;

            for (int x = 0; x < PPM_FRAME_WIDTH; x++) {
                int prev_x = x - translate_x;
                if (prev_x < 0 || prev_x >= PPM_FRAME_WIDTH) continue;

                layer_1_out[y * PPM_FRAME_WIDTH + x] ^=
                    prev_layer_1[prev_y * PPM_FRAME_WIDTH + prev_x];
                layer_2_out[y * PPM_FRAME_WIDTH + x] ^=
                    prev_layer_2[prev_y * PPM_FRAME_WIDTH + prev_x];
            }
        }
    }

    // Composite to RGB
    rgb24_pixel paper = PPM_PAPER_COLORS[paper_color & 1];
    rgb24_pixel inverse_paper = PPM_PAPER_COLORS[(paper_color & 1) ^ 1];

    for (int i = 0; i < PPM_FRAME_WIDTH * PPM_FRAME_HEIGHT; i++) {
        output[i] = paper;

        // Layer 1 drawn first
        if (layer_1_out[i]) {
            if (layer_1_color <= 1)
                output[i] = inverse_paper;
            else
                output[i] = PPM_LAYER_COLORS[layer_1_color];
        }

        // Layer 2 drawn on top
        if (layer_2_out[i]) {
            if (layer_2_color <= 1)
                output[i] = inverse_paper;
            else
                output[i] = PPM_LAYER_COLORS[layer_2_color];
        }
    }

    return UGOMEMO_OK;
}

rgb24_pixel *ppm_decode_frame_alloc(ppm_ctx *ctx, uint frame_index) {
    if (!ctx || !ctx->meta || frame_index >= ctx->meta->frame_count) return NULL;

    size_t layer_size = PPM_FRAME_WIDTH * PPM_FRAME_HEIGHT;

    rgb24_pixel *output = (rgb24_pixel *)calloc(layer_size, sizeof(rgb24_pixel));
    u8 *l1 = (u8 *)calloc(layer_size, 1);
    u8 *l2 = (u8 *)calloc(layer_size, 1);
    u8 *pl1 = (u8 *)calloc(layer_size, 1);
    u8 *pl2 = (u8 *)calloc(layer_size, 1);

    if (!output || !l1 || !l2 || !pl1 || !pl2) {
        free(output); free(l1); free(l2); free(pl1); free(pl2);
        return NULL;
    }

    // Decode all frames up to and including the requested one for correct diffing
    for (uint i = 0; i <= frame_index; i++) {
        int res = ppm_decode_frame(ctx, i, output, pl1, pl2, l1, l2);
        if (res != UGOMEMO_OK) {
            if (i != frame_index) {
                memcpy(pl1, l1, layer_size);
                memcpy(pl2, l2, layer_size);
                continue;
            }
            free(output); free(l1); free(l2); free(pl1); free(pl2);
            return NULL;
        }
        if (i < frame_index) {
            memcpy(pl1, l1, layer_size);
            memcpy(pl2, l2, layer_size);
        }
    }

    free(l1); free(l2); free(pl1); free(pl2);
    return output;
}

// ---- Encoding ----

static inline bool rgb_eq(rgb24_pixel a, rgb24_pixel b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

// Given an RGB pixel and the paper/layer colors, determine which layer it belongs to.
// Sets layer1[i]=1 if pixel matches layer1 color, layer2[i]=1 if matches layer2 color.
void ppm_classify_frame(rgb24_pixel *pixels, u8 paper_color,
                        u8 layer_1_color, u8 layer_2_color,
                        u8 *layer_1, u8 *layer_2) {
    rgb24_pixel paper = PPM_PAPER_COLORS[paper_color & 1];
    rgb24_pixel inverse_paper = PPM_PAPER_COLORS[(paper_color & 1) ^ 1];

    rgb24_pixel l1_rgb, l2_rgb;
    if (layer_1_color <= 1)
        l1_rgb = inverse_paper;
    else
        l1_rgb = PPM_LAYER_COLORS[layer_1_color];

    if (layer_2_color <= 1)
        l2_rgb = inverse_paper;
    else
        l2_rgb = PPM_LAYER_COLORS[layer_2_color];

    memset(layer_1, 0, PPM_LAYER_SIZE);
    memset(layer_2, 0, PPM_LAYER_SIZE);

    for (int i = 0; i < PPM_LAYER_SIZE; i++) {
        if (rgb_eq(pixels[i], paper)) {
            continue;  // background
        }
        // Layer 2 is drawn on top during decoding, so check it first for the
        // case where both layers use the same color: assign to layer 2.
        // But actually, during decoding, layer 1 is drawn first then layer 2
        // on top. If both colors are the same, any non-paper pixel could go
        // to either layer. We put it on layer 2 (the top one) to match the
        // decoder's compositing which checks layer 2 last.
        if (rgb_eq(pixels[i], l2_rgb)) {
            layer_2[i] = 1;
        } else if (rgb_eq(pixels[i], l1_rgb)) {
            layer_1[i] = 1;
        } else {
            // Pixel doesn't exactly match any known color.
            // Find nearest match using simple color distance.
            int d_paper = abs(pixels[i].red - paper.red) +
                          abs(pixels[i].green - paper.green) +
                          abs(pixels[i].blue - paper.blue);
            int d_l1 = abs(pixels[i].red - l1_rgb.red) +
                       abs(pixels[i].green - l1_rgb.green) +
                       abs(pixels[i].blue - l1_rgb.blue);
            int d_l2 = abs(pixels[i].red - l2_rgb.red) +
                       abs(pixels[i].green - l2_rgb.green) +
                       abs(pixels[i].blue - l2_rgb.blue);

            if (d_paper <= d_l1 && d_paper <= d_l2)
                continue;
            else if (d_l1 <= d_l2)
                layer_1[i] = 1;
            else
                layer_2[i] = 1;
        }
    }
}

// ---- Layer Line Compression ----

// Compress a single line (256 pixels, values 0 or 1) using the optimal encoding type.
// Returns the encoding type chosen, and writes compressed data to `out`.
// `out_len` is set to the number of bytes written.
static u8 compress_line(u8 *line, u8 *out, size_t *out_len) {
    int ones = 0;
    int chunks_used = 0;  // number of non-zero 8-pixel chunks

    for (int i = 0; i < 256; i++) {
        if (line[i]) ones++;
    }

    // Type 0: empty line
    if (ones == 0) {
        *out_len = 0;
        return 0;
    }

    // Count which chunks have any set pixels
    for (int c = 0; c < 32; c++) {
        bool has_data = false;
        for (int b = 0; b < 8; b++) {
            if (line[c * 8 + b]) { has_data = true; break; }
        }
        if (has_data) chunks_used++;
    }

    // Type 3: all chunks present (32 bytes, no flags)
    // Type 1: chunk-flagged (4 byte flags + chunks_used bytes)
    // Type 2: chunk-flagged inverted (4 byte flags + (32 - chunks_used) bytes for zero chunks)
    // But actually type 2 means line is pre-filled with 1s, then the chunk data overwrites.
    // So for type 2, we need chunks where the data differs from all-1s.

    int chunks_differ_from_ones = 0;
    for (int c = 0; c < 32; c++) {
        bool all_ones = true;
        for (int b = 0; b < 8; b++) {
            if (!line[c * 8 + b]) { all_ones = false; break; }
        }
        if (!all_ones) chunks_differ_from_ones++;
    }

    // Calculate sizes for each type
    size_t size_type1 = 4 + chunks_used;           // flags + data chunks
    size_t size_type2 = 4 + chunks_differ_from_ones; // flags + non-all-1 chunks
    size_t size_type3 = 32;                          // all 32 chunk bytes

    // Pick the smallest
    if (size_type1 <= size_type2 && size_type1 <= size_type3) {
        // Type 1: chunk-flagged, line starts as zeros
        u32 chunk_flags = 0;
        size_t pos = 4;

        for (int c = 0; c < 32; c++) {
            bool has_data = false;
            for (int b = 0; b < 8; b++) {
                if (line[c * 8 + b]) { has_data = true; break; }
            }

            if (has_data) {
                chunk_flags |= (1u << (31 - c));
                u8 chunk = 0;
                for (int b = 0; b < 8; b++) {
                    if (line[c * 8 + b]) chunk |= (1 << b);
                }
                out[pos++] = chunk;
            }
        }

        // Write chunk flags as big-endian u32
        out[0] = (chunk_flags >> 24) & 0xFF;
        out[1] = (chunk_flags >> 16) & 0xFF;
        out[2] = (chunk_flags >>  8) & 0xFF;
        out[3] = (chunk_flags >>  0) & 0xFF;

        *out_len = pos;
        return 1;
    } else if (size_type2 <= size_type3) {
        // Type 2: chunk-flagged, line starts as ones
        u32 chunk_flags = 0;
        size_t pos = 4;

        for (int c = 0; c < 32; c++) {
            bool all_ones = true;
            for (int b = 0; b < 8; b++) {
                if (!line[c * 8 + b]) { all_ones = false; break; }
            }

            if (!all_ones) {
                chunk_flags |= (1u << (31 - c));
                u8 chunk = 0;
                for (int b = 0; b < 8; b++) {
                    if (line[c * 8 + b]) chunk |= (1 << b);
                }
                out[pos++] = chunk;
            }
        }

        out[0] = (chunk_flags >> 24) & 0xFF;
        out[1] = (chunk_flags >> 16) & 0xFF;
        out[2] = (chunk_flags >>  8) & 0xFF;
        out[3] = (chunk_flags >>  0) & 0xFF;

        *out_len = pos;
        return 2;
    } else {
        // Type 3: all chunks, no flags
        size_t pos = 0;
        for (int c = 0; c < 32; c++) {
            u8 chunk = 0;
            for (int b = 0; b < 8; b++) {
                if (line[c * 8 + b]) chunk |= (1 << b);
            }
            out[pos++] = chunk;
        }
        *out_len = pos;
        return 3;
    }
}

// Compress a full layer (256x192 pixels).
// Returns encoded line types in line_encodings[192] and compressed data in layer_data.
// layer_data_len is set to the total compressed size.
int ppm_compress_layer(u8 *layer, u8 *line_encodings,
                       u8 *layer_data, size_t *layer_data_len) {
    size_t total = 0;
    u8 line_buf[256];  // temp buffer for one compressed line (max possible = 36 bytes for type 1)

    for (int y = 0; y < PPM_FRAME_HEIGHT; y++) {
        u8 *line = &layer[y * PPM_FRAME_WIDTH];
        size_t line_len = 0;
        u8 enc_type = compress_line(line, line_buf, &line_len);
        line_encodings[y] = enc_type;

        if (line_len > 0) {
            memcpy(layer_data + total, line_buf, line_len);
            total += line_len;
        }
    }

    *layer_data_len = total;
    return UGOMEMO_OK;
}

// Pack 192 2-bit line encoding values into 48 bytes
void ppm_pack_line_encodings(u8 *encodings, u8 *packed) {
    memset(packed, 0, 48);
    for (int i = 0; i < 192; i++) {
        int byte_idx = i / 4;
        int bit_offset = (i % 4) * 2;
        packed[byte_idx] |= (encodings[i] & 3) << bit_offset;
    }
}

// ---- Frame Encoding ----

// Encode a single frame from layer data.
// Writes into frame_buf starting at *frame_len, updates *frame_len.
// paper_color, layer_1_color, layer_2_color are the color indices.
int ppm_encode_frame(u8 *layer_1, u8 *layer_2,
                     u8 paper_color, u8 layer_1_color, u8 layer_2_color,
                     u8 *frame_buf, size_t *frame_len) {
    size_t pos = 0;

    // Frame header byte:
    // bit 7: frame type (1 = keyframe, no diffing)
    // bits 5-6: translate flag (0)
    // bits 3-4: layer 2 pen color
    // bits 1-2: layer 1 pen color
    // bit 0: paper color
    u8 header = 0;
    header |= (1 << 7);                     // frame type = 1 (keyframe)
    header |= (0 << 5);                     // no translation
    header |= ((layer_2_color & 3) << 3);
    header |= ((layer_1_color & 3) << 1);
    header |= (paper_color & 1);

    frame_buf[pos++] = header;

    // Line encoding types for both layers (48 bytes each)
    u8 line_enc_1[192], line_enc_2[192];
    u8 packed_enc_1[48], packed_enc_2[48];

    // Compress layers into temporary buffers
    // Max compressed size per layer: 192 * 36 = 6912 bytes (type 1 worst case)
    u8 *layer_1_data = (u8 *)malloc(192 * 36);
    u8 *layer_2_data = (u8 *)malloc(192 * 36);
    if (!layer_1_data || !layer_2_data) {
        free(layer_1_data);
        free(layer_2_data);
        return UGOMEMO_MEMORY_ERROR;
    }

    size_t l1_size = 0, l2_size = 0;
    ppm_compress_layer(layer_1, line_enc_1, layer_1_data, &l1_size);
    ppm_compress_layer(layer_2, line_enc_2, layer_2_data, &l2_size);

    // Pack line encodings
    ppm_pack_line_encodings(line_enc_1, packed_enc_1);
    ppm_pack_line_encodings(line_enc_2, packed_enc_2);

    // Write packed line encodings (48 + 48 = 96 bytes)
    memcpy(frame_buf + pos, packed_enc_1, 48);
    pos += 48;
    memcpy(frame_buf + pos, packed_enc_2, 48);
    pos += 48;

    // Write compressed layer data (layer 1 first, then layer 2)
    memcpy(frame_buf + pos, layer_1_data, l1_size);
    pos += l1_size;
    memcpy(frame_buf + pos, layer_2_data, l2_size);
    pos += l2_size;

    free(layer_1_data);
    free(layer_2_data);

    *frame_len = pos;
    return UGOMEMO_OK;
}

// ---- Color Detection ----

// Detect paper color from the first frame (most common known paper color).
int ppm_detect_paper_color(rgb24_pixel *first_frame, u8 *paper_color) {
    int count_black = 0, count_white = 0;

    for (int i = 0; i < PPM_LAYER_SIZE; i++) {
        if (rgb_eq(first_frame[i], PPM_PAPER_COLORS[0])) count_black++;
        else if (rgb_eq(first_frame[i], PPM_PAPER_COLORS[1])) count_white++;
    }

    *paper_color = (count_white >= count_black) ? 1 : 0;
    return UGOMEMO_OK;
}

// Detect layer colors for a single frame by scanning its pixels.
// Returns the PPM color indices for layer 1 and layer 2.
void ppm_detect_frame_colors(rgb24_pixel *pixels, u8 paper_color,
                             u8 *layer_1_color, u8 *layer_2_color) {
    rgb24_pixel paper_rgb = PPM_PAPER_COLORS[paper_color & 1];
    rgb24_pixel inverse_rgb = PPM_PAPER_COLORS[(paper_color & 1) ^ 1];

    // Track which palette colors appear in this frame
    bool has_inverse = false;
    bool has_red = false;
    bool has_blue = false;

    for (int i = 0; i < PPM_LAYER_SIZE; i++) {
        if (rgb_eq(pixels[i], paper_rgb)) continue;
        if (rgb_eq(pixels[i], inverse_rgb)) has_inverse = true;
        else if (rgb_eq(pixels[i], PPM_LAYER_COLORS[2])) has_red = true;
        else if (rgb_eq(pixels[i], PPM_LAYER_COLORS[3])) has_blue = true;
    }

    // Assign colors to layers. PPM convention: layer 1 first, layer 2 on top.
    // When multiple colors are present, pick two. Order: inverse, red, blue.
    u8 found[3];
    int found_count = 0;
    if (has_inverse) found[found_count++] = 1;  // inverse of paper
    if (has_red)     found[found_count++] = 2;
    if (has_blue)    found[found_count++] = 3;

    if (found_count == 0) {
        *layer_1_color = 1;
        *layer_2_color = 1;
    } else if (found_count == 1) {
        *layer_1_color = found[0];
        *layer_2_color = found[0];
    } else {
        *layer_1_color = found[0];
        *layer_2_color = found[1];
    }
}
