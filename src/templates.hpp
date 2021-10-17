#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "types.hpp"

/*
 * Clamp value between min and max
 *
 * Parameters:
 * - <type>: the type of the input and output
 * - input: the value to clamp
 * - min: the min of the clamp
 * - max: the max of the clamp
 * Returns:
 * <type> value of the clamp
 */
template<typename T>
T clampValue(T input, int min, int max) {
    if (input < min) input = (T)min;
    if (input > max) input = (T)max;
    return input;
}

/**
 * Gets an integer of the given size from a buffer
 *
 * Note: this relies on data being stored in little endian byte order.
 * This is the case on almost all platforms, so there is no need for concern.
 *
 * Parameters
 * - <type>: the data type to be returned
 * - input: a u8 vector to get data from
 * - pos: the starting location of the data in buffer to get an int from
 * Returns:
 * - an int of the specified size
 */
template<typename T>
T getInt(std::vector<u8> input, int pos) {
    return *reinterpret_cast<T*>(input.data() + pos);
}

/**
 * Find the RMS (root mean square) value of the input data
 *
 * Parameters:
 * - input: a vector of any type (preferably integer or float)
 * Returns:
 * - the RMS of the input data as a double
 */
template<typename T>
double findRMS(std::vector<T> input) {
    double rms = 0.0;

    // Square each value and add them together
    for (auto i = 0; i < (int)input.size(); i++) {
        rms += (double)input[i] * (double)input[i];
    }

    // The square root of the sum of the squares divided by the number of values
    return std::sqrt(rms / (double)input.size());
}

/*
 * Get a hex string of a certain section of bytes in file_buffer
 *
 * Parameters:
 * - buffer: an iterable object containing 1 byte (8 bit) values
 * - input: an iterable object of
 * - start: the starting index in input
 * `length` how many bytes to get the hex string of
 * Returns:
 * - the hex of the given data as a string
 */
template<typename Iterable>
std::string getHexString(Iterable input, int start, int length) {
    std::stringstream stream;

    stream << std::hex << std::setfill('0');

    for (int i = start; i < start + length; i++) {
        stream << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(input[i]));
    }

    return stream.str();
}
