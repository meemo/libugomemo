#ifndef UGOMEMO_MACROS_H_
#define UGOMEMO_MACROS_H_

#include <stdlib.h>
#include <stdio.h>

#define UGO_ROUND_UP_MULT_4(x) (((x) + 3) & ~3)
#define UGO_CLAMP(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))
#define UGO_REVERSE_NIBBLES(x) (((x & 0x0F) << 4) | ((x & 0xF0) >> 4))

#define READ_OR_ERR(func, ...) \
    do { \
        int res_temp = res; \
        res = func(__VA_ARGS__); \
        if (res != UGOMEMO_OK) { \
            CTX_ERROR(ctx, "file read failed or out of bounds"); \
            return UGOMEMO_INPUT_ERROR; \
        } \
        res = res_temp; \
    } while(0)

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); } while (0);

#endif
