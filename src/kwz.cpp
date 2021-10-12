#include <iostream>
#include <sstream>
#include <iterator>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <iomanip>

#include "kwz.hpp"
#include "tables.hpp"

/*
 * Read file contents into file_buffer
 *
 * `path` is the file path to be read
 */
void readFile(std::string path) {
    std::ifstream file(path, std::ios::binary);

    if (file) {
        // Stop eating newlines and whitespace in binary mode
        file.unsetf(std::ios::skipws);

        file.seekg(0, std::ios::end);
        std::streamoff file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        file_buffer.reserve(file_size);
        file_buffer.insert(file_buffer.begin(),
                           std::istream_iterator<u8>(file),
                           std::istream_iterator<u8>());

        file.close();
    }
    else {
        std::cout << "Failed to read file. " << std::endl;
        exit(-1);
    }
}

/*
 * Verify file is a valid KWZ file.
 *
 * TODO: Implement RSA signature verification: the last 256 bytes of file.
 *
 * Returns:
 * Boolean of if the file is a valid .kwz file.
 */
bool verifyFile() {
    bool result = false;

    // For now, only verify that size is greater than 0
    result = file_buffer.size() > 0;

    return result;
}

/*
 * Get a hex string of a certain section of bytes in file_buffer
 *
 * `start` is the starting index in file_buffer
 * `length` how many bytes to get the hex string of
 *
 * Returns:
 * String of the hex of the given region of file_buffer
 */
std::string getHexString(int start, int length) {
    std::stringstream stream;

    stream << std::hex << std::setfill('0');

    for (int i = start; i < start + length; i++) {
        stream << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(file_buffer[i]));
    }

    return stream.str();
}
