#pragma once

// Defining functions
void readFile(std::string path);
std::string getHexString(int t_start, int t_end);

typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef uint32_t u32;

#include "tables.hpp"

std::vector<u8> file_buffer;

/*
 * Gets an integer of the given size from file_buffer
 *
 * Note: this relies on data being stored in little endian byte order.
 * This is the case on almost all platforms, so there is no need for concern.
 *
 * Parameters
 * <type>: the data type to be returned
 * pos: the starting location of the data in file_buffer to be returned
 */
template<typename T>
T getInt(int pos) {
    return *reinterpret_cast<T*>(file_buffer.data() + pos);
}
