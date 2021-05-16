#include <iostream>
#include <sstream>
#include <iterator>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <iomanip>

#include "kwz.hpp"
/*
 * Read file contents at `path` into file_buffer
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
 * TODO: Implement RSA signature verification; last 256 bytes of file.
 */
bool verifyFile() {
    bool result = false;

    // For now, only verify that size is greater than 0
    result = file_buffer.size() > 0;

    return result;
}

std::string getHexString(int start_pos, int length) {
    std::stringstream ss;

    ss << std::hex << std::setfill('0');

    for (int i = start_pos; i < start_pos + length; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(file_buffer[i]));
    }

    return ss.str();
}
