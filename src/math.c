#include <libugomemo.h>

/* math.c
 *
 * This file contains various math functions for use in the library.
 *
 * Functions have the prefix of i_ to avoid potential name collisions.
 */

/* i_sqrt()
 *
 * Take the square root of a number even if not using stdlib.
 *
 * The non-stdlib method is pretty, but it's functional, portable, will rarely be used in the library, and can be
 * easily swapped out for a faster method if one exists in the target toolchain.
 *
 * Note: does not verify that the value is negative
 *
 * Parameters:
 * - x: the number to take the square root of (double)
 *
 * Returns:
 * - the square root of x (double)
 */
double i_sqrt(double x) {
#ifdef __USE_STDLIB__
    #include <math.h>
    return sqrt(x);
#else
    double result = x / 2;
    double temp = 0;

    while (temp != result) {
        temp = result;
        result = (temp + x / temp) / 2;
    }

    return result;
#endif
}

/* i_rms()
 *
 * Find the root mean square (RMS) of a set of numbers. Typed for use by decoded kwz audio.
 *
 * Parameters:
 * - data: the data to calculate the RMS of (double*)
 * - length: the length of `data` (int)
 *
 * Returns:
 * - the RMS of the data (double)
 */
double i_rms(const s16 *data, int num_samples) {
    double sum = 0;
    int i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i] * data[i];
    }

    return i_sqrt(sum / (double)num_samples);
}

/* i_std_dev()
 *
 * Find the standard deviation of a set of numbers. Typed for use by decoded kwz audio.
 *
 * Parameters:
 * - data: the data to calculate the standard deviation of (double*)
 * - length: the length of `data` (int)
 *
 * Returns:
 * - the standard deviation of the data (double)
 */
double i_std_dev(const s16 *data, int num_samples) {
    double sum = 0;
    double mean = 0;
    double std_dev = 0;
    int i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i];
    }

    mean = sum / (double)num_samples;

    for (i = 0; i < num_samples; i++) {
        std_dev += (data[i] - mean) * (data[i] - mean);
    }

    return i_sqrt(std_dev / (double)num_samples);
}
