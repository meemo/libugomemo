/* SPDX-License-Identifier: Apache-2.0 */

#ifndef LIBUGOMEMO_MACROS_H_
#define LIBUGOMEMO_MACROS_H_


/* =========================================== Misc. =========================================== */
/**
 * Clamps a number to ensure the value is between an upper and lower bound where low ≤ x ≤ high.
 *
 * Parameters:
 * - x:    The value to be clamped.
 * - low:  The lower bound on the clamp.
 * - high: The upper bound on the clamp.
 */
#define CLAMP(low, x, high)  (x < low ? (x = low) : (x > high ? (x = high) : (x = x)))

/* Rounds up a number to become a multiple of 4. */
#define ROUND_MULT_4(x)      (((x) + 3) & ~3)

/* Switches the positions of the 2 nibbles within a byte. x must be a u8. */
#define REVERSE_NIBBLES(x)   (((x & 0x0F) << (4)) | ((x & 0xF0) >> (4)))

/**
 * Some compilers compile a right bit shift to a logical bit shift instruction, which can cause unexpected behavior on
 * signed integers, so using this macro guarantees an arithmetic bit shift.
 *
 * Parameters:
 * - x: The value to perform the bit shift on.
 * - n: The number of bits to shift by.
 */
#define SAR(x, n)  (((x > 0) && (n > 0)) ? ((x >> n) | ~(~0U >> n)) : (x >> n))
/* ============================================================================================== */


/* ============================================ IO ============================================= */
/**
 * The following macros are used for reading little endian integers of certain sizes from buffers regardless of the
 * target's native endianness.
 *
 * Each reading macro's name is in the format `READ_{size}({buffer}, {postition})` like GET_U32, where:
 * - size: A value of the format `{signed-ness}{number of bits}` like u32, where:
 *   - signed-ness: U is the only implemented value (representing unsigned integer).
 *   - number of bits: One of: 8, 16, or 32; the number of bits in the integer to extract.
 * - buffer: The buffer to extract the integer from. u8 buffer expected.
 * - position: The position in the buffer to extract the integer from.
 *             MAKE SURE THIS WON'T EXCEED THE LENGTH OF THE BUFFER!
 */

/* Single bytes are too small for endianness to take effect. */
#define READ_U8(buffer, pos)    ((u8) (buffer[pos]))

#ifdef UGO_CFG_LITTLE_ENDIAN_TARGET
#define READ_U16(buffer, pos)  (*(u16 *) &buffer[pos])
#else
#define READ_U16(buffer, pos)   ((u16)(buffer[pos] | buffer[pos + 1] << 0x8))
#endif

#ifdef UGO_CFG_LITTLE_ENDIAN_TARGET
#define READ_U32(buffer, pos)  (*(u32 *) &buffer[pos])
#else
#define READ_U32(buffer, pos)   ((u32)(buffer[pos]              \
                                     | buffer[pos + 1] << 0x08  \
                                     | buffer[pos + 2] << 0x10  \
                                     | buffer[pos + 3] << 0x18))
#endif
/* ============================================================================================== */


#endif
