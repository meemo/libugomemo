/* SPDX-License-Identifier: Apache-2.0 */

#ifndef LIBUGOMEMO_H_
#define LIBUGOMEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libugomemo_types.h>
#include <libugomemo_config.h>
#include <libugomemo_macros.h>
#include <libugomemo_functions.h>

#define KWZ_STEP_INDEX_MIN         0
#define KWZ_STEP_INDEX_MAX        79
#define KWZ_PREDICTOR_MIN      -2048
#define KWZ_PREDICTOR_MAX       2047
#define KWZ_SCALING_FACTOR        16  /* 16 bits per sample in the output of decoding. */
#define KWZ_VARIABLE_THRESHOLD    18
#define KWZ_INITIAL_PREDICTOR      0
#define KWZ_AUDIO_SAMPLE_RATE  16364  /* That's not a typo. */

#define KWZ_FRAME_WIDTH          320
#define KWZ_FRAME_HEIGHT         240

#define PPM_STEP_INDEX_MIN         0
#define PPM_STEP_INDEX_MAX        88
#define PPM_PREDICTOR_MIN     -32768
#define PPM_PREDICTOR_MAX      32767
#define PPM_SCALING_FACTOR        16  /* 16 bits per sample in the output of decoding. */
#define PPM_AUDIO_SAMPLE_RATE   8192

#define PPM_FRAME_WIDTH          256
#define PPM_FRAME_HEIGHT         192

#define PPM_THUMBNAIL_WIDTH       64
#define PPM_THUMBNAIL_HEIGHT      48

#ifdef __cplusplus
}
#endif

#endif
