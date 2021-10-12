#pragma once

/*
 * Clamp value between min and max
 *
 * Parameters:
 * - <type>: the type of the input and output
 * - value: the value to clamp
 * - min: the min of the clamp
 * - max: the max of the clamp
 *
 * Returns:
 * <type> value of the clamp
 */
template<typename T>
T clampValue(T value, int min, int max) {
    if (value < min) value = (T)min;
    if (value > max) value = (T)max;
    return value;
}

// Define functions
void writeWAV(std::string t_path, std::vector<s16> t_input);

double findRMS(std::vector<int16_t> input);

std::vector<s16> decodeTrack(int track_size, int track_offset, int step_index);

int findCorrectStepIndex(int track_size, int track_offset);
