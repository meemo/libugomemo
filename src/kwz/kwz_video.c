#include <libugomemo.h>

/**
 * kwz_video.c
 *
 * This file contains functions for processing video (animation frames) data from a KWZ file.
 */


/* ========================================== Constants ========================================= */
/* Framerate lookup table for KWZ files. */
const float KWZ_FRAMERATES[11] = { 0.2f, 0.5f, 1.0f, 2.0f, 4.0f, 6.0f, 8.0f, 12.0f, 20.0f, 24.0f, 30.0f };

/* Common line table index values. */
const u16 LINE_INDEX_TABLE[32] = { 0x0000, 0x0CD0, 0x19A0, 0x02D9,
                                   0x088B, 0x0051, 0x00F3, 0x0009,
                                   0x001B, 0x0001, 0x0003, 0x05B2,
                                   0x1116, 0x00A2, 0x01E6, 0x0012,
                                   0x0036, 0x0002, 0x0006, 0x0B64,
                                   0x08DC, 0x0144, 0x00FC, 0x0024,
                                   0x001C, 0x0004, 0x0334, 0x099C,
                                   0x0668, 0x1338, 0x1004, 0x166C };

/* Common line table index values, shift-rotated one place to the left. */
const u16 LINE_INDEX_TABLE_SHIFTED[32] = { 0x0000, 0x0CD0, 0x19A0, 0x0003,
                                           0x02D9, 0x088B, 0x0051, 0x00F3,
                                           0x0009, 0x001B, 0x0001, 0x0006,
                                           0x05B2, 0x1116, 0x00A2, 0x01E6,
                                           0x0012, 0x0036, 0x0002, 0x02DC,
                                           0x0B64, 0x08DC, 0x0144, 0x00FC,
                                           0x0024, 0x001C, 0x099C, 0x0334,
                                           0x1338, 0x0668, 0x166C, 0x1004 };
/* ============================================================================================== */
