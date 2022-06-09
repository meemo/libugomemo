#ifndef LIBUGOMEMO_MACROS_H
#define LIBUGOMEMO_MACROS_H

/**
 * Clamps a number to ensure the value is between an upper and lower bound.
 *
 * Parameters:
 * - x:    The value to be clamped.
 * - low:  The lower bound on the clamp.
 * - high: The upper bound on the clamp.
 * Returns:
 * - The clamped value.
 */
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Returns the minimum value between the two parameters. */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/* Returns the maximum value between the two parameters. */
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/* Rounds up a number to become a multiple of 4. */
#define ROUND_UP_MULT_4(x)  (((x) + 3) & ~3)

/* Switches the positions of the 2 nibbles within a byte. x must be a 1 byte (8 bit) type. */
#define REVERSE_NIBBLES(x)  (((x & 0x0F) << (4)) | ((x & 0xF0) >> (4)))

/**
 * Performs an arithmetic right bit shift.
 *
 * Some compilers may do a logical bit shift on signed values, so using this macro guarantees an arithmetic bit shift.
 *
 * Equivalent function:
 *
 * ```c
 * int SAR(int x, int n)  {
 *     if (x < 0 && n > 0) {
 *         return x >> n | ~(~0U >> n);
 *     }
 *     else {
 *          return x >> n;
 *     }
 * }
 * ```
 *
 * Parameters:
 * - x: The value to perform the bit shift on.
 * - n: The number of bits to shift by.
 *
 * "Return":
 * - The result of the operation.
 */
#define SAR(x, n)  (((x > 0) && (n > 0)) ? ((x >> n) | ~(~0U >> n)) : (x >> n))

/* === SHA-1 Macros === */
/* Rotate left. */
#define ROL(value, bits)  (((value) << (bits)) | ((value)  >>  (32 - (bits))))

#ifdef LITTLE_ENDIAN_
#define blk0(i) (block->l[i] = (ROL(block->l[i], 24) & 0xFF00FF00) | (ROL(block->l[i], 8) & 0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i & 15] = ROL(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ \
                block->l[i & 15], 1))

/* Core operations of SHA-1. */
#define R0(v, w, x, y, z, i)  z += ((w & (x ^ y)) ^ y)       + blk0(i) + 0x5A827999 + ROL(v, 5); w = ROL(w, 30);
#define R1(v, w, x, y, z, i)  z += ((w & (x ^ y)) ^ y)       +  blk(i) + 0x5A827999 + ROL(v, 5); w = ROL(w, 30);
#define R2(v, w, x, y, z, i)  z += (w ^ x ^ y)               +  blk(i) + 0x6ED9EBA1 + ROL(v, 5); w = ROL(w, 30);
#define R3(v, w, x, y, z, i)  z += (((w | x) & y) | (w & x)) +  blk(i) + 0x8F1BBCDC + ROL(v, 5); w = ROL(w, 30);
#define R4(v, w, x, y, z, i)  z += (w ^ x ^ y)               +  blk(i) + 0xCA62C1D6 + ROL(v, 5); w = ROL(w, 30);

#endif
