#ifndef UGOMEMO_KWZ_VIDEO_TABLES_H_
#define UGOMEMO_KWZ_VIDEO_TABLES_H_

#include <ugomemo/types.h>

#define KWZ_FRAME_WIDTH  320
#define KWZ_FRAME_HEIGHT 240
#define KWZ_TILE_SIZE    8
#define KWZ_LARGE_TILE   128
#define KWZ_TILES_X      (KWZ_FRAME_WIDTH / KWZ_TILE_SIZE)   // 40
#define KWZ_TILES_Y      (KWZ_FRAME_HEIGHT / KWZ_TILE_SIZE)  // 30
#define KWZ_TILE_COUNT   (KWZ_TILES_X * KWZ_TILES_Y)         // 1200
#define KWZ_LAYER_SIZE   (KWZ_FRAME_WIDTH * KWZ_FRAME_HEIGHT)

typedef struct { int x, y; } kwz_tile_pos;

static const rgb24_pixel KWZ_PALETTE[7] = {
    { 0xFF, 0xFF, 0xFF },  // 0: white
    { 0x14, 0x14, 0x14 },  // 1: black
    { 0xFF, 0x17, 0x17 },  // 2: red
    { 0xFF, 0xE6, 0x00 },  // 3: yellow
    { 0x00, 0x82, 0x32 },  // 4: green
    { 0x06, 0xAE, 0xFF },  // 5: blue
    { 0x00, 0x00, 0x00 },  // 6: transparent (placeholder)
};

static const u16 KWZ_COMMON_LINE_INDEX[32] = {
    0x0000, 0x0CD0, 0x19A0, 0x02D9, 0x088B, 0x0051, 0x00F3, 0x0009,
    0x001B, 0x0001, 0x0003, 0x05B2, 0x1116, 0x00A2, 0x01E6, 0x0012,
    0x0036, 0x0002, 0x0006, 0x0B64, 0x08DC, 0x0144, 0x00FC, 0x0024,
    0x001C, 0x0004, 0x0334, 0x099C, 0x0668, 0x1338, 0x1004, 0x166C,
};

static inline void kwz_compute_tile_positions(kwz_tile_pos *positions) {
    int idx = 0;
    for (int lty = 0; lty < KWZ_FRAME_HEIGHT; lty += KWZ_LARGE_TILE) {
    for (int ltx = 0; ltx < KWZ_FRAME_WIDTH; ltx += KWZ_LARGE_TILE) {
    for (int ty = 0; ty < KWZ_LARGE_TILE; ty += KWZ_TILE_SIZE) {
        int y = lty + ty;
        if (y >= KWZ_FRAME_HEIGHT) break;
    for (int tx = 0; tx < KWZ_LARGE_TILE; tx += KWZ_TILE_SIZE) {
        int x = ltx + tx;
        if (x >= KWZ_FRAME_WIDTH) break;
        positions[idx].x = x;
        positions[idx].y = y;
        idx++;
    }}}}
}

#endif
