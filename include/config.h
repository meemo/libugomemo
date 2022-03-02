#ifndef LIBUGOMEMO_CONFIG_H
#define LIBUGOMEMO_CONFIG_H

/* Remove/comment if stdlib is not available to be used.
 *
 * Note: complete support is not guaranteed until the library is finished (low priority).
 */
#define USE_STDLIB_

/* Remove/comment if the target's architecture is not little endian or isn't operating in little endian mode.
 *
 * Most targets will be little endian, however if the target is big endian the library will not function correctly.
 *
 * Note: complete support is not guaranteed until the library is finished (very low priority).
 * If this is required for your target, please create an issue on github.
 */
#define LITTLE_ENDIAN_

#endif
