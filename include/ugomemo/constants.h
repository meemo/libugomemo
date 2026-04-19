#ifndef UGOMEMO_CONSTANTS_H_
#define UGOMEMO_CONSTANTS_H_

// Constants not unique to any particular format

#define SHA256_SIZE 32
#define SHA1_SIZE 20
#define MD5_SIZE 16
#define CRC32_SIZE 4

#define DSI_EPOCH 946706400  // Seconds since January 1, 2000 00:00 UTC

#define FLIPNOTE_USERNAME_LENGTH (11 * 2)  // 10 characters + null terminator in UTF16-LE

#endif
