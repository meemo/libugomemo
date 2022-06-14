#ifndef LIBUGOMEMO_MACROS_H
#define LIBUGOMEMO_MACROS_H


/* =========================================== Misc. =========================================== */
/**
 * Clamps a number to ensure the value is between an upper and lower bound.
 *
 * Parameters:
 * - x:    The value to be clamped.
 * - low:  The lower bound on the clamp.
 * - high: The upper bound on the clamp.
 */
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Finds the minimum value between the two parameters. */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/* Finds the maximum value between the two parameters. */
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/* Rounds up a number to become a multiple of 4. */
#define ROUND_MULT_4(x)  (((x) + 3) & ~3)

/* Switches the positions of the 2 nibbles within a byte. x must be a u8. */
#define REVERSE_NIBBLES(x)  (((x & 0x0F) << (4)) | ((x & 0xF0) >> (4)))

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


/* =========================================== SHA-1 =========================================== */
/* Rotate (bits) left. */
#define ROL(value, bits)  (((value) << (bits)) | ((value)  >>  (32 - (bits))))

#ifdef LITTLE_ENDIAN_
#define blk0(i) (block->l[i] = (ROL(block->l[i], 24) & 0xFF00FF00) | (ROL(block->l[i], 8) & 0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i & 15] = ROL(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ \
                block->l[i & 15], 1))

/* Core operations. */
#define R0(v, w, x, y, z, i)  z += ((w & (x ^ y)) ^ y)       + blk0(i) + 0x5A827999 + ROL(v, 5); w = ROL(w, 30);
#define R1(v, w, x, y, z, i)  z += ((w & (x ^ y)) ^ y)       +  blk(i) + 0x5A827999 + ROL(v, 5); w = ROL(w, 30);
#define R2(v, w, x, y, z, i)  z += (w ^ x ^ y)               +  blk(i) + 0x6ED9EBA1 + ROL(v, 5); w = ROL(w, 30);
#define R3(v, w, x, y, z, i)  z += (((w | x) & y) | (w & x)) +  blk(i) + 0x8F1BBCDC + ROL(v, 5); w = ROL(w, 30);
#define R4(v, w, x, y, z, i)  z += (w ^ x ^ y)               +  blk(i) + 0xCA62C1D6 + ROL(v, 5); w = ROL(w, 30);
/* ============================================================================================== */


/* ============================================ IO ============================================= */
/**
 * The following macros are used for reading little endian integers of certain sizes from buffers regardless of the
 * target's native endianness.
 *
 * Each macro's name is in the format `READ_{size}({buffer}, {postition})` like GET_U32, where:
 * - size: A value of the format `{signed-ness}{number of bits}` like u32, where:
 *   - signed-ness: U is the only implemented value (representing unsigned integer).
 *   - number of bits: One of: 8, 16, or 32; the number of bits in the integer to extract.
 * - buffer: The buffer to extract the integer from. Unsigned 8 bit per element buffer expected.
 * - position: The position in the buffer to extract the integer from.
 *             MAKE SURE THIS WON'T EXCEED THE LENGTH OF THE BUFFER!
 */

/* Single bytes are too small for endianness to take effect. */
#define READ_U8(buffer, pos)    (u8) buffer[pos]

#ifdef LITTLE_ENDIAN_
#define READ_U16(buffer, pos)  *(u16 *) &buffer[pos]
#else
#define READ_U16(buffer, pos)   (u16) buffer[pos] | buffer[pos + 1] << 0x8
#endif

#ifdef LITTLE_ENDIAN_
#define READ_U32(buffer, pos)  *(u32 *) &buffer[pos]
#else
#define READ_U32(buffer, pos)   (u32) buffer[pos]             | buffer[pos + 1] << 0x08 \
                                    | buffer[pos + 2] << 0x10 | buffer[pos + 3] << 0x18
#endif


/* ============================================================================================== */


#endif
