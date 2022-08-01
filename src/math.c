#include <libugomemo.h>

/**
 * math.c
 *
 * Math functions for use in the library, primarily for .kwz audio.
 *
 * Functions are named so that they will not interfere with stdlib functions.
 */


/**
 * Takes the square root of a number. Has a secondary implementation for when the math library is not available.
 *
 * This implementation is slow, however the function will very rarely be used in the library.
 *
 * Parameters:
 * - x: The number to take the square root of. Must be positive!
 *
 * Returns:
 * - The square root of x.
 */
double UGO_SQRT(double x) {
#ifdef sqrt
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
double UGO_RMS(const s16 *data, uint num_samples) {
    double sum = 0.0f;
    uint i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i] * data[i];
    }

    return UGO_SQRT(sum / (double)num_samples);
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
double UGO_STDDEV(const s16 *data, uint num_samples) {
    double sum = 0.0f;
    double std_dev = 0.0f;
    double mean;
    uint i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i];
    }

    mean = sum / (double)num_samples;

    for (i = 0; i < num_samples; i++) {
        std_dev += (data[i] - mean) * (data[i] - mean);
    }

    return UGO_SQRT(std_dev / (double)num_samples);
}
