#ifndef TABLES_H_
#define TABLES_H_

#include <types.h>

const float KWZ_FRAMERATES[11] = { 0.2, 0.5, 1, 2, 4, 6, 8, 12, 20, 24, 30 };

/* 0 is a placeholder because ppm framerates are indexed from 1 */
const float PPM_FRAMERATES[9] = { 0, 0.5, 1, 2, 4, 6, 12, 20, 30 };

/* 2 bit sample index table */
const int ADPCM_INDEX_TABLE_2[4] = { -1, 2, -1, 2 };

/* 4 bit sample index table */
const int ADPCM_INDEX_TABLE_4[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

const s16 ADPCM_STEP_TABLE[89] = {     7,     8,     9,    10,    11,    12,
                                      13,    14,    16,    17,    19,    21,
                                      23,    25,    28,    31,    34,    37,
                                      41,    45,    50,    55,    60,    66,
                                      73,    80,    88,    97,   107,   118,
                                     130,   143,   157,   173,   190,   209,
                                     230,   253,   279,   307,   337,   371,
                                     408,   449,   494,   544,   598,   658,
                                     724,   796,   876,   963,  1060,  1166,
                                    1282,  1411,  1552,  1707,  1878,  2066,
                                    2272,  2499,  2749,  3024,  3327,  3660,
                                    4026,  4428,  4871,  5358,  5894,  6484,
                                    7132,  7845,  8630,  9493, 10442, 11487,
                                   12635, 13899, 15289, 16818, 18500, 20350,
                                   22385, 24623, 27086, 29794, 32767 };

/* Common line table index values table */
const u16 LINE_INDEX_TABLE[32] = { 0x0000, 0x0CD0, 0x19A0, 0x02D9,
                                   0x088B, 0x0051, 0x00F3, 0x0009,
                                   0x001B, 0x0001, 0x0003, 0x05B2,
                                   0x1116, 0x00A2, 0x01E6, 0x0012,
                                   0x0036, 0x0002, 0x0006, 0x0B64,
                                   0x08DC, 0x0144, 0x00FC, 0x0024,
                                   0x001C, 0x0004, 0x0334, 0x099C,
                                   0x0668, 0x1338, 0x1004, 0x166C };

/* Common line table index values table, shift-rotated one place to the left */
const u16 LINE_INDEX_TABLE_SHIFTED[32] = { 0x0000, 0x0CD0, 0x19A0, 0x0003,
                                           0x02D9, 0x088B, 0x0051, 0x00F3,
                                           0x0009, 0x001B, 0x0001, 0x0006,
                                           0x05B2, 0x1116, 0x00A2, 0x01E6,
                                           0x0012, 0x0036, 0x0002, 0x02DC,
                                           0x0B64, 0x08DC, 0x0144, 0x00FC,
                                           0x0024, 0x001C, 0x099C, 0x0334,
                                           0x1338, 0x0668, 0x166C, 0x1004 };

#endif
