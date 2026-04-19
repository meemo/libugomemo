#include <stdlib.h>
#include <string.h>

#include <ugomemo.h>

#include <ugomemo/kwz_video_tables.h>
#include <ugomemo/kwz_line_tables.h>

// KWZ_LINE_INDEX_SHIFTED is decoder-only (not in the shared tables header)
static const u16 KWZ_LINE_INDEX_SHIFTED[32] = {
    0x0000, 0x0CD0, 0x19A0, 0x0003, 0x02D9, 0x088B, 0x0051, 0x00F3,
    0x0009, 0x001B, 0x0001, 0x0006, 0x05B2, 0x1116, 0x00A2, 0x01E6,
    0x0012, 0x0036, 0x0002, 0x02DC, 0x0B64, 0x08DC, 0x0144, 0x00FC,
    0x0024, 0x001C, 0x099C, 0x0334, 0x1338, 0x0668, 0x166C, 0x1004,
};

// Bitpacked reader state
typedef struct {
    u8 *data;
    size_t offset;
    size_t size;
    u32 bit_value;
    int bit_index;
} kwz_bit_reader;

static inline void kwz_bit_reader_init(kwz_bit_reader *r, u8 *data, size_t offset, size_t size) {
    r->data = data;
    r->offset = offset;
    r->size = size;
    r->bit_value = 0;
    r->bit_index = 16;  // Forces initial load on first read
}

static inline u32 kwz_read_bits(kwz_bit_reader *r, int num_bits) {
    if (r->bit_index + num_bits > 16) {
        if (r->offset + 1 >= r->size) return 0;
        u32 next = (u32)r->data[r->offset] | ((u32)r->data[r->offset + 1] << 8);
        r->offset += 2;
        r->bit_value |= next << (16 - r->bit_index);
        r->bit_index -= 16;
    }
    u32 result = r->bit_value & ((1u << num_bits) - 1);
    r->bit_value >>= num_bits;
    r->bit_index += num_bits;
    return result;
}

static void kwz_decompress_layer_v2(
    u8 *layer,
    u8 *prev_layer,
    u8 *compressed_data,
    u16 compressed_size,
    bool is_diff,
    kwz_tile_pos *tile_positions
) {
    kwz_bit_reader reader;

    // When is_diff, the layer data is a delta against the previous frame.
    // Start from the previous layer content so tile type 5 (skip) preserves
    // unchanged tiles, while new tile data overwrites on top.
    if (is_diff && prev_layer != NULL) {
        memcpy(layer, prev_layer, KWZ_FRAME_WIDTH * KWZ_FRAME_HEIGHT);
    } else {
        memset(layer, 0, KWZ_FRAME_WIDTH * KWZ_FRAME_HEIGHT);
    }

    if (compressed_size == 0) return;

    kwz_bit_reader_init(&reader, compressed_data, 0, compressed_size);

    int t = 0;
    while (t < KWZ_TILE_COUNT) {
        int x = tile_positions[t].x;
        int y = tile_positions[t].y;

        int tile_type = kwz_read_bits(&reader, 3);
        u16 line_idx_a, line_idx_b;
        const u8 *line_a, *line_b;

        switch (tile_type) {
            case 0: {
                line_idx_a = KWZ_COMMON_LINE_INDEX[kwz_read_bits(&reader, 5)];
                line_a = KWZ_LINE_TABLE[line_idx_a];
                for (int row = 0; row < 8; row++)
                    memcpy(&layer[(y + row) * KWZ_FRAME_WIDTH + x], line_a, 8);
                break;
            }
            case 1: {
                line_idx_a = kwz_read_bits(&reader, 13);
                line_a = KWZ_LINE_TABLE[line_idx_a];
                for (int row = 0; row < 8; row++)
                    memcpy(&layer[(y + row) * KWZ_FRAME_WIDTH + x], line_a, 8);
                break;
            }
            case 2: {
                u32 idx = kwz_read_bits(&reader, 5);
                line_idx_a = KWZ_COMMON_LINE_INDEX[idx];
                line_idx_b = KWZ_LINE_INDEX_SHIFTED[idx];
                line_a = KWZ_LINE_TABLE[line_idx_a];
                line_b = KWZ_LINE_TABLE[line_idx_b];
                for (int row = 0; row < 8; row++) {
                    const u8 *line = (row & 1) ? line_b : line_a;
                    memcpy(&layer[(y + row) * KWZ_FRAME_WIDTH + x], line, 8);
                }
                break;
            }
            case 3: {
                line_idx_a = kwz_read_bits(&reader, 13);
                line_a = KWZ_LINE_TABLE[line_idx_a];
                line_b = KWZ_LINE_TABLE_SHIFTED[line_idx_a];
                for (int row = 0; row < 8; row++) {
                    const u8 *line = (row & 1) ? line_b : line_a;
                    memcpy(&layer[(y + row) * KWZ_FRAME_WIDTH + x], line, 8);
                }
                break;
            }
            case 4: {
                u8 flags = kwz_read_bits(&reader, 8);
                for (int row = 0; row < 8; row++) {
                    u16 li;
                    if (flags & (1 << row))
                        li = KWZ_COMMON_LINE_INDEX[kwz_read_bits(&reader, 5)];
                    else
                        li = kwz_read_bits(&reader, 13);
                    memcpy(&layer[(y + row) * KWZ_FRAME_WIDTH + x], KWZ_LINE_TABLE[li], 8);
                }
                break;
            }
            case 5: {
                u32 skip_count = kwz_read_bits(&reader, 5);
                for (u32 s = 0; s <= skip_count && t < KWZ_TILE_COUNT; s++) {
                    int sx = tile_positions[t].x;
                    int sy = tile_positions[t].y;
                    if (prev_layer != NULL) {
                        for (int row = 0; row < 8; row++)
                            memcpy(&layer[(sy + row) * KWZ_FRAME_WIDTH + sx],
                                   &prev_layer[(sy + row) * KWZ_FRAME_WIDTH + sx], 8);
                    }
                    if (s < skip_count) t++;
                }
                break;
            }
            case 6:
                break;
            case 7: {
                u8 pattern = kwz_read_bits(&reader, 2);
                u8 is_common = kwz_read_bits(&reader, 1);

                if (is_common) {
                    line_idx_a = KWZ_COMMON_LINE_INDEX[kwz_read_bits(&reader, 5)];
                    line_idx_b = KWZ_COMMON_LINE_INDEX[kwz_read_bits(&reader, 5)];
                    pattern = (pattern + 1) % 4;
                } else {
                    line_idx_a = kwz_read_bits(&reader, 13);
                    line_idx_b = kwz_read_bits(&reader, 13);
                }

                line_a = KWZ_LINE_TABLE[line_idx_a];
                line_b = KWZ_LINE_TABLE[line_idx_b];

                static const u8 patterns[4][8] = {
                    {0, 1, 0, 1, 0, 1, 0, 1},
                    {0, 0, 1, 0, 0, 1, 0, 0},
                    {0, 1, 0, 0, 1, 0, 0, 1},
                    {0, 1, 1, 0, 1, 1, 0, 1},
                };

                for (int row = 0; row < 8; row++) {
                    const u8 *line = patterns[pattern][row] ? line_b : line_a;
                    memcpy(&layer[(y + row) * KWZ_FRAME_WIDTH + x], line, 8);
                }
                break;
            }
        }
        t++;
    }
}

// Composite layers into RGB pixels
static void kwz_composite_frame(
    rgb24_pixel *output,
    u8 *layer_a, u8 *layer_b, u8 *layer_c,
    u32 flags
) {
    u8 paper_idx = flags & 0x0F;
    u8 la_c1 = (flags >> 8) & 0x0F;
    u8 la_c2 = (flags >> 12) & 0x0F;
    u8 lb_c1 = (flags >> 16) & 0x0F;
    u8 lb_c2 = (flags >> 20) & 0x0F;
    u8 lc_c1 = (flags >> 24) & 0x0F;
    u8 lc_c2 = (flags >> 28) & 0x0F;

    if (paper_idx > 6) paper_idx = 0;
    rgb24_pixel paper = KWZ_PALETTE[paper_idx];

    // Layer order: C (bottom), B (middle), A (top) by default
    // Depth sorting could be applied here but for basic output we use default order
    for (int i = 0; i < KWZ_FRAME_WIDTH * KWZ_FRAME_HEIGHT; i++) {
        output[i] = paper;

        // Layer C (bottom)
        if (layer_c[i] == 1 && lc_c1 < 7) output[i] = KWZ_PALETTE[lc_c1];
        else if (layer_c[i] == 2 && lc_c2 < 7) output[i] = KWZ_PALETTE[lc_c2];

        // Layer B (middle)
        if (layer_b[i] == 1 && lb_c1 < 7) output[i] = KWZ_PALETTE[lb_c1];
        else if (layer_b[i] == 2 && lb_c2 < 7) output[i] = KWZ_PALETTE[lb_c2];

        // Layer A (top)
        if (layer_a[i] == 1 && la_c1 < 7) output[i] = KWZ_PALETTE[la_c1];
        else if (layer_a[i] == 2 && la_c2 < 7) output[i] = KWZ_PALETTE[la_c2];
    }
}

int kwz_decode_frame(kwz_ctx *ctx, uint frame_index, rgb24_pixel *output,
                     u8 *prev_layer_a, u8 *prev_layer_b, u8 *prev_layer_c,
                     u8 *layer_a_out, u8 *layer_b_out, u8 *layer_c_out) {
    kwz_kmi_entry *e;
    kwz_tile_pos tile_positions[KWZ_TILE_COUNT];
    size_t layer_offset;
    u32 flags;

    if (frame_index >= ctx->meta->frame_count) return UGOMEMO_INPUT_ERROR;

    e = &ctx->video->kmi[frame_index];
    flags = e->flags;

    kwz_compute_tile_positions(tile_positions);

    // Calculate offset into KMC data for this frame
    // Layer data is sequential: all frame 0 layers, all frame 1 layers, etc.
    layer_offset = 0;
    for (uint i = 0; i < frame_index; i++) {
        layer_offset += ctx->video->kmi[i].layer_a_size;
        layer_offset += ctx->video->kmi[i].layer_b_size;
        layer_offset += ctx->video->kmi[i].layer_c_size;
    }

    u32 kmc_data_size = ctx->meta->kmc_size - 4;  // subtract CRC32
    size_t frame_total = (size_t)e->layer_a_size + e->layer_b_size + e->layer_c_size;
    if (layer_offset + frame_total > kmc_data_size)
        return UGOMEMO_INPUT_ERROR;

    u8 *kmc = ctx->kmc_data;
    bool diff_a = !(flags & 0x10);
    bool diff_b = !(flags & 0x20);
    bool diff_c = !(flags & 0x40);

    // Decompress layer A
    kwz_decompress_layer_v2(layer_a_out, prev_layer_a,
        kmc + layer_offset, e->layer_a_size, diff_a, tile_positions);
    layer_offset += e->layer_a_size;

    // Decompress layer B
    kwz_decompress_layer_v2(layer_b_out, prev_layer_b,
        kmc + layer_offset, e->layer_b_size, diff_b, tile_positions);
    layer_offset += e->layer_b_size;

    // Decompress layer C
    kwz_decompress_layer_v2(layer_c_out, prev_layer_c,
        kmc + layer_offset, e->layer_c_size, diff_c, tile_positions);

    // Composite to RGB
    kwz_composite_frame(output, layer_a_out, layer_b_out, layer_c_out, flags);

    return UGOMEMO_OK;
}

rgb24_pixel *kwz_decode_frame_alloc(kwz_ctx *ctx, uint frame_index) {
    if (!ctx || !ctx->meta || frame_index >= ctx->meta->frame_count) return NULL;

    size_t layer_size = KWZ_FRAME_WIDTH * KWZ_FRAME_HEIGHT;

    rgb24_pixel *output = (rgb24_pixel *)calloc(layer_size, sizeof(rgb24_pixel));
    u8 *la = (u8 *)calloc(layer_size, 1);
    u8 *lb = (u8 *)calloc(layer_size, 1);
    u8 *lc = (u8 *)calloc(layer_size, 1);
    u8 *pla = (u8 *)calloc(layer_size, 1);
    u8 *plb = (u8 *)calloc(layer_size, 1);
    u8 *plc = (u8 *)calloc(layer_size, 1);

    if (!output || !la || !lb || !lc || !pla || !plb || !plc) {
        free(output); free(la); free(lb); free(lc);
        free(pla); free(plb); free(plc);
        return NULL;
    }

    // Decode all frames up to and including the requested one for correct diffing
    for (uint i = 0; i <= frame_index; i++) {
        int res = kwz_decode_frame(ctx, i, output, pla, plb, plc, la, lb, lc);
        if (res != UGOMEMO_OK) {
            if (i != frame_index) {
                memcpy(pla, la, layer_size);
                memcpy(plb, lb, layer_size);
                memcpy(plc, lc, layer_size);
                continue;
            }
            free(output); free(la); free(lb); free(lc);
            free(pla); free(plb); free(plc);
            return NULL;
        }
        if (i < frame_index) {
            memcpy(pla, la, layer_size);
            memcpy(plb, lb, layer_size);
            memcpy(plc, lc, layer_size);
        }
    }

    free(la); free(lb); free(lc);
    free(pla); free(plb); free(plc);
    return output;
}

// ==== Video Encoding ====

// ---- Bit Writer ----

typedef struct {
    u8 *data;
    size_t capacity;
    size_t byte_offset;
    u32 bit_value;
    int bit_count;
} kwz_bit_writer;

static void kwz_bw_init(kwz_bit_writer *w, u8 *data, size_t capacity) {
    w->data = data;
    w->capacity = capacity;
    w->byte_offset = 0;
    w->bit_value = 0;
    w->bit_count = 0;
}

static inline void kwz_bw_write(kwz_bit_writer *w, u32 value, int num_bits) {
    w->bit_value |= (value & ((1u << num_bits) - 1)) << w->bit_count;
    w->bit_count += num_bits;
    while (w->bit_count >= 16) {
        if (w->byte_offset + 2 > w->capacity) return;
        w->data[w->byte_offset++] = w->bit_value & 0xFF;
        w->data[w->byte_offset++] = (w->bit_value >> 8) & 0xFF;
        w->bit_value >>= 16;
        w->bit_count -= 16;
    }
}

static void kwz_bw_flush(kwz_bit_writer *w) {
    if (w->bit_count > 0) {
        if (w->byte_offset >= w->capacity) return;
        w->data[w->byte_offset++] = w->bit_value & 0xFF;
        if (w->bit_count > 8) {
            if (w->byte_offset >= w->capacity) return;
            w->data[w->byte_offset++] = (w->bit_value >> 8) & 0xFF;
        }
        w->bit_count = 0;
        w->bit_value = 0;
    }
}

// ---- Reverse Line Table Lookup ----

// Convert 8 pixel values (each 0-2) to a line table index.
// The line table is generated with a specific ordering:
//   lineTable[index] = [b, a, d, c, f, e, h, g]
//   where index = a*3^7 + b*3^6 + c*3^5 + d*3^4 + e*3^3 + f*3^2 + g*3 + h
// So given pixels [p0..p7]: a=p1, b=p0, c=p3, d=p2, e=p5, f=p4, g=p7, h=p6
static inline u16 pixels_to_line_index(const u8 *pixels) {
    return (u16)(pixels[1] * 2187 + pixels[0] * 729 +
                 pixels[3] * 243  + pixels[2] * 81  +
                 pixels[5] * 27   + pixels[4] * 9   +
                 pixels[7] * 3    + pixels[6]);
}

// Build reverse lookup: line_table_index -> common_index (or -1 if not common)
void kwz_build_common_reverse_lookup(i16 *reverse) {
    for (int i = 0; i < 6561; i++) reverse[i] = -1;
    for (int i = 0; i < 32; i++) {
        reverse[KWZ_COMMON_LINE_INDEX[i]] = (i16)i;
    }
}

// ---- Tile Encoding ----

// Calculate bit cost for tile type 4 encoding
static int cost_type4(u16 *line_indices, const i16 *common_reverse) {
    int cost = 3 + 8;  // tile type (3) + flags byte (8)
    for (int r = 0; r < 8; r++) {
        cost += (common_reverse[line_indices[r]] >= 0) ? 5 : 13;
    }
    return cost;
}

// Encode a single 8x8 tile, choosing the best tile type
static void encode_tile(kwz_bit_writer *w, u8 *layer, int tile_x, int tile_y,
                        const i16 *common_reverse) {
    // Extract the 8 line indices for this tile
    u16 line_indices[8];
    for (int r = 0; r < 8; r++) {
        u8 *row = &layer[(tile_y + r) * KWZ_FRAME_WIDTH + tile_x];
        line_indices[r] = pixels_to_line_index(row);
    }

    // Check if all lines are the same (type 0 or 1)
    bool all_same = true;
    for (int r = 1; r < 8; r++) {
        if (line_indices[r] != line_indices[0]) { all_same = false; break; }
    }

    if (all_same) {
        i16 ci = common_reverse[line_indices[0]];
        if (ci >= 0) {
            // Type 0: common index, 3+5 = 8 bits
            kwz_bw_write(w, 0, 3);
            kwz_bw_write(w, (u32)ci, 5);
        } else {
            // Type 1: direct index, 3+13 = 16 bits
            kwz_bw_write(w, 1, 3);
            kwz_bw_write(w, line_indices[0], 13);
        }
        return;
    }

    // Check type 7 patterns: two line values in a specific arrangement
    // patterns: 0=ABABABAB, 1=AABAAABA, 2=ABAABAAБ, 3=ABBABBAB
    static const u8 patterns[4][8] = {
        {0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 1, 0, 0, 1},
        {0, 1, 1, 0, 1, 1, 0, 1},
    };

    // Find up to 2 distinct line indices
    u16 val_a = line_indices[0];
    u16 val_b = val_a;
    bool two_values = true;
    for (int r = 0; r < 8; r++) {
        if (line_indices[r] != val_a) {
            if (val_b == val_a) {
                val_b = line_indices[r];
            } else if (line_indices[r] != val_b) {
                two_values = false;
                break;
            }
        }
    }

    if (two_values && val_a != val_b) {
        // Try each type 7 pattern
        for (int p = 0; p < 4; p++) {
            bool match = true;
            for (int r = 0; r < 8; r++) {
                u16 expected = patterns[p][r] ? val_b : val_a;
                if (line_indices[r] != expected) { match = false; break; }
            }

            if (match) {
                i16 ci_a = common_reverse[val_a];
                i16 ci_b = common_reverse[val_b];
                bool both_common = (ci_a >= 0 && ci_b >= 0);

                // Cost of type 7
                int cost_t7;
                if (both_common) {
                    // common path: 3+2+1+5+5 = 16 bits (pattern adjusted)
                    cost_t7 = 16;
                } else {
                    // direct path: 3+2+1+13+13 = 32 bits
                    cost_t7 = 32;
                }

                int cost_t4 = cost_type4(line_indices, common_reverse);

                if (cost_t7 <= cost_t4) {
                    kwz_bw_write(w, 7, 3);

                    if (both_common) {
                        // When is_common=1, pattern = (pattern + 1) % 4 in decoder
                        // So we need to adjust: encode_pattern = (p + 3) % 4
                        u8 adj_pattern = (p + 3) % 4;
                        kwz_bw_write(w, adj_pattern, 2);
                        kwz_bw_write(w, 1, 1);  // is_common = 1
                        kwz_bw_write(w, (u32)ci_a, 5);
                        kwz_bw_write(w, (u32)ci_b, 5);
                    } else {
                        kwz_bw_write(w, (u32)p, 2);
                        kwz_bw_write(w, 0, 1);  // is_common = 0
                        kwz_bw_write(w, val_a, 13);
                        kwz_bw_write(w, val_b, 13);
                    }
                    return;
                }
            }
        }
    }

    // Fallback: type 4 (per-line encoding with flags)
    kwz_bw_write(w, 4, 3);

    u8 flags = 0;
    for (int r = 0; r < 8; r++) {
        if (common_reverse[line_indices[r]] >= 0)
            flags |= (1 << r);
    }
    kwz_bw_write(w, flags, 8);

    for (int r = 0; r < 8; r++) {
        i16 ci = common_reverse[line_indices[r]];
        if (ci >= 0)
            kwz_bw_write(w, (u32)ci, 5);
        else
            kwz_bw_write(w, line_indices[r], 13);
    }
}

// Compress a full layer. Returns compressed data size.
u32 kwz_compress_layer(u8 *layer, u8 *out_data, size_t out_capacity,
                       kwz_tile_pos *tile_positions, const i16 *common_reverse) {
    kwz_bit_writer writer;
    kwz_bw_init(&writer, out_data, out_capacity);

    for (int t = 0; t < KWZ_TILE_COUNT; t++) {
        encode_tile(&writer, layer, tile_positions[t].x, tile_positions[t].y,
                    common_reverse);
    }

    kwz_bw_flush(&writer);
    return (u32)writer.byte_offset;
}

// ---- Color Classification for KWZ ----

static inline bool kwz_rgb_eq(rgb24_pixel a, rgb24_pixel b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

// Transparent paper renders as (0,0,0) in BMP output
static const rgb24_pixel TRANSPARENT_PAPER_RGB = { 0x00, 0x00, 0x00 };

// Find the closest palette index for a given RGB pixel.
// paper_idx is needed to handle transparent paper (index 6) specially.
static u8 find_closest_palette_index(rgb24_pixel pixel, u8 paper_idx) {
    int best_dist = 0x7FFFFFFF;
    u8 best_idx = 0;

    // Check transparent paper first
    if (paper_idx == 6 && kwz_rgb_eq(pixel, TRANSPARENT_PAPER_RGB))
        return 6;

    for (int i = 0; i < 6; i++) {
        int d = abs(pixel.red - KWZ_PALETTE[i].red) +
                abs(pixel.green - KWZ_PALETTE[i].green) +
                abs(pixel.blue - KWZ_PALETTE[i].blue);
        if (d < best_dist) {
            best_dist = d;
            best_idx = (u8)i;
        }
    }
    return best_idx;
}

// Detect all colors in a frame and assign them to layer slots.
// Each layer gets up to 2 colors. Colors sorted by frequency (most common first).
void kwz_detect_frame_colors(rgb24_pixel *pixels, u8 paper_idx,
                             kwz_frame_colors *out) {
    int counts[7] = {0};

    for (int i = 0; i < KWZ_LAYER_SIZE; i++) {
        u8 idx = find_closest_palette_index(pixels[i], paper_idx);
        if (idx != paper_idx) counts[idx]++;
    }

    // Sort non-paper colors by frequency (descending)
    u8 sorted[6];
    int sorted_counts[6];
    int n = 0;
    for (int i = 0; i < 6; i++) {
        if (i == paper_idx || counts[i] == 0) continue;
        sorted[n] = (u8)i;
        sorted_counts[n] = counts[i];
        n++;
    }
    // Simple insertion sort
    for (int i = 1; i < n; i++) {
        int j = i;
        while (j > 0 && sorted_counts[j] > sorted_counts[j-1]) {
            u8 tc = sorted[j]; sorted[j] = sorted[j-1]; sorted[j-1] = tc;
            int tv = sorted_counts[j]; sorted_counts[j] = sorted_counts[j-1]; sorted_counts[j-1] = tv;
            j--;
        }
    }

    // Initialize output
    memset(out, 0, sizeof(*out));
    out->paper_idx = paper_idx;
    for (int i = 0; i < 7; i++) out->color_layer[i] = -1;

    // Assign colors to layer slots: A(c1,c2), B(c1,c2), C(c1,c2)
    // Up to 6 colors can be assigned (3 layers x 2 color slots)
    u8 *slots[6] = { &out->la_c1, &out->la_c2,
                     &out->lb_c1, &out->lb_c2,
                     &out->lc_c1, &out->lc_c2 };
    int slot_layer[6] = { 0, 0, 1, 1, 2, 2 };
    int slot_value[6] = { 1, 2, 1, 2, 1, 2 };

    for (int i = 0; i < n && i < 6; i++) {
        *slots[i] = sorted[i];
        out->color_layer[sorted[i]] = (i8)slot_layer[i];
        out->color_value[sorted[i]] = (u8)slot_value[i];
    }

    // Fill unused slots with 0 (they'll be ignored since no pixels use them)
}

// Classify a frame's pixels into layers A, B, C based on the color assignment.
void kwz_classify_frame(rgb24_pixel *pixels, const kwz_frame_colors *colors,
                        u8 *layer_a, u8 *layer_b, u8 *layer_c) {
    memset(layer_a, 0, KWZ_LAYER_SIZE);
    memset(layer_b, 0, KWZ_LAYER_SIZE);
    memset(layer_c, 0, KWZ_LAYER_SIZE);

    u8 *layers[3] = { layer_a, layer_b, layer_c };

    for (int i = 0; i < KWZ_LAYER_SIZE; i++) {
        u8 idx = find_closest_palette_index(pixels[i], colors->paper_idx);
        if (idx == colors->paper_idx) continue;

        i8 layer = colors->color_layer[idx];
        if (layer < 0) continue;  // unassigned color
        layers[layer][i] = colors->color_value[idx];
    }
}
