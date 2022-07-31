#ifndef LIBUGOMEMO_CONFIG_H
#define LIBUGOMEMO_CONFIG_H

/**
 * If the target toolchain does not have standard C memory management functions then redefine them here.
 *
 * Remember to include the header file that provides these new functions by replacing line 10.
 */
#include <stdlib.h>
#define UGO_MALLOC    malloc
#define UGO_REALLOC  realloc
#define UGO_CALLOC    calloc
#define UGO_FREE        free

/**
 * Remove or comment if the target's architecture is not little endian or isn't operating in little endian mode.
 *
 * Most targets will be little endian, however if the target is big endian the library will not function correctly
 * unless this is set correctly.
 *
 * Note: complete support is not guaranteed until the library is finished.
 * If support is missing for something required for your target then please create an issue on github.
 */
#define LITTLE_ENDIAN_

#endif
