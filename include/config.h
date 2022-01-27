#ifndef LIBUGOMEMO_CONFIG_H
#define LIBUGOMEMO_CONFIG_H

/* Remove/comment if stdlib is not available to be used.
 *
 * Note: complete support is not guaranteed until the library is finished (low priority).
 */
#define __USE_STDLIB__

/* Remove/comment if the target's architecture is not little endian or isn't operating in little endian mode.
 *
 * Most targets will be little endian, however the library will not function correctly if this isn't specified.
 */
#define __LITTLE_ENDIAN__

#endif
