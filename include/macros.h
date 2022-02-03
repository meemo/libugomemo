#ifndef LIBUGOMEMO_MACROS_H
#define LIBUGOMEMO_MACROS_H

/* Clamps a number between an upper and lower bound.
 *
 * Parameters:
 * - x
 *     - The value to be clamped
 * - low
 *     - The lower bound on the clamp
 * - high
 *     - The upper bound on the clamp
 * Returns:
 * - The clamped value
 */
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Returns the minimum value between the two parameters. */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/* Returns the maximum value between the two parameters. */
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/* Rounds up a number to become a multiple of 4 */
#define ROUND_UP_MULT_4(x)  (((x) + 3) & ~3)

/* Interchanges positions of nibbles within a byte. Assumes x is a 1 byte (8 bit) type. */
#define REVERSE_NIBBLES(x)  (((x & 0x0F) << (4)) | ((x & 0xF0) >> (4)))

/* Performs an arithmetic right bit shift.
 *
 * Some (compiler) implementations may do a logical bit shift on signed values, so this guarantees an arithmetic shift.
 *
 * Equivalent function:
 * int SAR(int x, int n)  {
 *     if (x < 0 && n > 0) {
 *         return x >> n | ~(~0U >> n);
 *     }
 *     else {
 *          return x >> n;
 *     }
 * }
 *
 * Parameters:
 * - x: the value to perform the shift on
 * - n: the number of positions (bits) to shift by
 * - r: the value for the shifted result to be stored in
 * Returns:
 * - Nothing, the result is stored in r
 */
#define SAR(x, n, r)  (((x > 0) && (n > 0)) ? (r = ((x >> n) | ~(~0U >> n))) : (r = (x >> n)))

/* Performs a logical right bit shift. x must be unsigned, otherwise this may be an arithmetic shift. */
#define SHR(x, n)  ((x) >> (n))

#endif
