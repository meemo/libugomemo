#pragma once

typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef uint32_t u32;

#include "tables.hpp"

std::vector<u8> file_buffer;

template<typename T>
T getInt(int pos) {
    // Relies on data being stored in little endian byte order.
    // This is almost all platforms.
    return *reinterpret_cast<T*>(file_buffer.data() + pos);
}
