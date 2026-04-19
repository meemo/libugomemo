#ifndef UGOMEMO_KWZ_CONSTANTS_H_
#define UGOMEMO_KWZ_CONSTANTS_H_

#include <ugomemo/types.h>

#define KFH_HEADER_SIZE 212
#define KTN_HEADER_SIZE 12
#define KMC_HEADER_SIZE 12
#define KMI_HEADER_SIZE 8
#define KSN_HEADER_SIZE 36
#define KFH_SECTION_SIZE (KFH_HEADER_SIZE - 8)
#define KWZ_SIGNATURE_SIZE 256
#define KWZ_FSID_LENGTH 10

// Rendered string length in bytes without null terminator
#define KWZ_FILENAME_LENGTH 28  // mevqfemz2wqivuinupxwgjo23vgn

#define KWZ_STEP_INDEX_MIN         0
#define KWZ_STEP_INDEX_MAX        79
#define KWZ_PREDICTOR_MIN      -2048
#define KWZ_PREDICTOR_MAX       2047
#define KWZ_SCALING_FACTOR        16  // 16 bits per sample in the output of decoding.
#define KWZ_VARIABLE_THRESHOLD    18
#define KWZ_INITIAL_PREDICTOR      0
#define KWZ_AUDIO_SAMPLE_RATE  16364  // That's not a typo, 64 not 84.

// 120 seconds of mono s16 (PCM) 16364hz audio, (120 * 16364 * sizeof(s16)) = 3927360
// Originally 60 seconds, changed to 120 because of memory bounds issue
#define KWZ_AUDIO_BUFFER_SIZE (1 << 22)  // 2^22 = 4MiB (nicest rounded up number)

// Lower bound for file sizes, just large enough to be able to process data in the code
// but leaves room for compressed kwc, for example
#define KWZ_MIN_FILE_SIZE (KFH_SECTION_SIZE + KWZ_SIGNATURE_SIZE)

// Upper bound for KWZ file sizes.
// They are known to get up to just over 4MiB in the gallery set, and ~5MiB in DSi Library
#define KWZ_MAX_FILE_SIZE (6 * 1024 * 1024)

#endif
