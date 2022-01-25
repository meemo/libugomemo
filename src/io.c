#include <stdio.h>

#include <libugomemo.h>

/* io.c
 *
 * This file contains functions to extract specific endianness values from file buffers regardless of the
 * target system's native endianness.
 *
 * The API is relatively simple:
 *
 *     getBuffer_{endianness}_{data type}({file buffer}, {starting position of desired value});
 *
 * - target endianness
 *     - `LE` (little endian) is the only implemented value
 *
 * - data type
 *     - Values:
 *     - `u8`
 *     - `u16
 *     - `u32`
 *     - This value is the type extracted from the file buffer, and will be the type of the returned value
 *
 * - file buffer
 *     - an u8 file buffer
 *
 * - starting position of desired value
 *     - the value to be returned will start at this position in the vector and extend as far as `data type`
 */

u8 getBuffer_LE_u8(const u8 *buffer, int pos) {
    return (u8) buffer[pos];
}

u16 getBuffer_LE_u16(const u8 *buffer, int pos) {
    return (u16) buffer[pos]
               | buffer[pos + 1] << 0x8;
}

u32 getBuffer_LE_u32(const u8 *buffer, int pos) {
    return (u32) buffer[pos]
               | buffer[pos + 1] << 0x08
               | buffer[pos + 2] << 0x10
               | buffer[pos + 3] << 0x18;
}
