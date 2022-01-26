#include <math.h>

#include <libugomemo.h>

/* math.c
 *
 * This file contains various math functions for use in the library, mainly for kwz_audio.c
 */

double rms(const s16 *data, int num_samples) {
    double sum = 0;
    int i;

    for (i = 0; i < num_samples; i++) {
        sum += data[i] * data[i];
    }

    return sqrt(sum / (double)num_samples);
}

double std_dev(const s16 *data, int num_samples) {
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

    return sqrt(std_dev / (double)num_samples);
}
