#ifndef MACROS_H_
#define MACROS_H_

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/* rounds up to become a multiple of 4 */
#define ROUND_UP_MULT_4(x)  (((x) + 3) & ~3)

#endif
