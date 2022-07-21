#include <libugomemo.h>

/**
 * ppm_audio.c
 *
 * This file contains functions for processing video (animation frames) data from a PPM file.
 */


/* ========================================== Constants ========================================= */
/* Framerate lookup table for KWZ files. 0.0f is a placeholder; framerates are indexed from 1. */
const float PPM_FRAMERATES[9] = { 0.0f, 0.5f, 1.0f, 2.0f, 4.0f, 6.0f, 12.0f, 20.0f, 30.0f };
/* ============================================================================================== */
