#pragma once

#include <string>
#include <vector>

#include "types.hpp"

// Define functions
std::vector<u8> readFile(std::string path);

void writeWAV(std::string path, std::vector<s16> audio, int sample_rate);

void kwzWriteThumbnail(std::vector<u8> buffer, int start, int size);
