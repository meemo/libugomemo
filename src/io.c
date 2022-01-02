#include <stdio.h>

#include "../include/libugomemo.h"

u8 getBuffer_LE_U8(const char *buffer, int pos) {
    return (u8) buffer[pos];
}

u16 getBuffer_LE_U16(const char *buffer, int pos) {
    return (u16) buffer[pos]
               | buffer[pos + 1] << 0x8;
}

u32 getBuffer_LE_U32(const char *buffer, int pos) {
    return (u32) buffer[pos]
               | buffer[pos + 1] << 0x08
               | buffer[pos + 2] << 0x10
               | buffer[pos + 3] << 0x18;
}
