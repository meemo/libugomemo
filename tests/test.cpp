#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <iterator>

#include "../src/types.hpp"
#include "../src/io.hpp"

#include <tomcrypt.h>
#include <tfm.h>

std::string getHexString(unsigned char *input, int start, int length) {
    std::stringstream stream;

    stream << std::hex << std::setfill('0');

    for (int i = start; i < start + length; i++) {
        stream << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(input[i]));
    }

    return stream.str();
}

int main(int argc, char **argv) {
    std::vector<u8> file_buffer = readFile("samples/cmtpkbxgqmxcccc53sztrd5b4aen.kwz");
    hash_state hash_state_var;
    unsigned char * hash = (unsigned char *) malloc(64);

    sha1_init(&hash_state_var);
    sha1_process(&hash_state_var, file_buffer.data(), (unsigned long) file_buffer.size());
    sha1_done(&hash_state_var, hash);

    std::cout << getHexString(hash, 0, 20) << std::endl;
}
