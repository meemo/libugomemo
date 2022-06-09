#include <libugomemo.h>

/**
 * math.c
 *
 * Math functions for use in the library, primarily for .kwz audio.
 */


/**
 * Takes the square root of a number. Has a secondary implementation for when the math library is not available.
 *
 * The secondary implementation is slow, but the function will rarely be used in the library and can be swapped out
 * for a faster implementation if one exists in the target toolchain.
 *
 * Note: the function does not verify that the value is negative.
 *
 * Parameters:
 * - x: The number to take the square root of.
 *
 * Returns:
 * - The square root of x.
 */
double sqrt_(double x) {
#ifdef USE_STDLIB_
    #include <math.h>
    return sqrt(x);
#else
    /* Newton's method. */
    double result = x / 2;
    double temp = 0.0f;

    while (temp != result) {
        temp = result;
        result = (temp + x / temp) / 2;
    }

    return result;
#endif
}

/**
 * Finds the root mean square (RMS) of a set of numbers. Typed for use with kwz_audio.c functions.
 *
 * Parameters:
 * - data: The data to calculate the RMS of.
 * - length: The number of elements in `data`.
 *
 * Returns:
 * - The RMS value of the data.
 */
double rms_(const s16 *data, unsigned int num_samples) {
    double sum = 0.0f;
    unsigned int i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i] * data[i];
    }

    return sqrt_(sum / (double)num_samples);
}

/**
 * Calculates the standard deviation of a set of numbers. Typed for use with kwz_audio.c functions.
 *
 * Parameters:
 * - data: The data to calculate the standard deviation of.
 * - length: The number of elements in `data`.
 *
 * Returns:
 * - The standard deviation of the data.
 */
double stdDev_(const s16 *data, unsigned int num_samples) {
    double sum = 0.0f;
    double std_dev = 0.0f;
    double mean;
    unsigned int i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i];
    }

    mean = sum / (double)num_samples;

    for (i = 0; i < num_samples; i++) {
        std_dev += (data[i] - mean) * (data[i] - mean);
    }

    return sqrt_(std_dev / (double)num_samples);
}
