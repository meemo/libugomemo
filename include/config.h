#ifndef LIBUGOMEMO_CONFIG_H_
#define LIBUGOMEMO_CONFIG_H_

/**
 * If the target toolchain does not have standard C memory management functions then they can be redefined here.
 *
 * Remember to include the header file that provides these new functions by replacing the appropriate #include.
 */
#include <stdlib.h>
#define UGO_MALLOC    malloc
#define UGO_REALLOC  realloc
#define UGO_CALLOC    calloc
#define UGO_FREE        free

#include <string.h>
#define UGO_MEMSET    memset
#define UGO_MEMCPY    memcpy

/**
 * Remove or comment if the target's architecture is not little endian or isn't operating in little endian mode.
 *
 * Most targets will be little endian, however if the target is big endian the library will not function correctly
 * unless this is set correctly.
 *
 * Note: complete support is not guaranteed until the library is finished.
 * If support is missing for something required for your project then please create an issue on github.
 */
#define UGO_CFG_LITTLE_ENDIAN_TARGET

/**
 * Uncomment to remove optimizations that increase speed at the cost of adding size to the final binary.
 *
 * Currently this just removes the CRC32 poly-8 table optimization. This may include KWZ/PPM audio sample tables later.
 */
#define UGO_CFG_LARGE_BINARY_SIZE

#endif
