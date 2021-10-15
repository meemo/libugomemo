#pragma once

#include <vector>

#include "types.hpp"

// Define functions
std::vector<s16> decodeTrack(int track_size, int track_offset, int step_index);

int findCorrectStepIndex(int track_size, int track_offset);
