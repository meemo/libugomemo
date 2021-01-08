#pragma once

// General file variables
char* file_buffer;
int file_size;

int offset;
int kfh_offset;
int kmi_offset;
int ksn_offset;
int kmc_offset;

// Video processing variables
uint8_t layer_pixels[3][240][40][8] = { 0 };
int layer_buffer_pointer;
int current_layer;
uint16_t bit_value;
int bit_index;
int prev_decoded_frame;

// File meta
uint32_t kfh_section_size;
uint32_t file_creation_timestamp;
uint32_t file_last_edit_timestap;
uint32_t app_version;

// Not formatted as xxxx-xxxx-xxxx-xxxxxx
std::string root_author_ID;
std::string parent_author_ID;
std::string current_author_ID;

// Need to implement conversion from UTF-16 LE to something else, storing as char* for now
char* root_author_name_raw;
char* parent_author_name_raw;
char* current_author_name_raw;

// Sometimes file names are unconverted from PPM in DSi library notes, may need to implement conversion
std::string root_file_name;
std::string parent_file_name;
std::string current_file_name;
uint16_t frame_count;
uint16_t thumbnail_frame_index;
double framerate;
bool is_locked;
bool is_loop_playback;
bool is_toolset;
bool layer_a_invisible;
bool layer_b_invisible;
bool layer_c_invisible;

struct frame_meta_struct {
	uint32_t flags;
	uint16_t layer_a_size;
	uint16_t layer_b_size;
	uint16_t layer_c_size;
	std::string frame_author_id;
	uint8_t layer_a_depth;
	uint8_t layer_b_depth;
	uint8_t layer_c_depth;
	uint8_t sound_effect_flags;
	uint16_t camera_flags;
};

struct diffing_flags_struct {
	int paper_color_index;
	int layer_a_diffing_flag;
	int layer_b_diffing_flag;
	int layer_c_diffing_flag;
	int is_prev_frame;
	int layer_a_first_color_index;
	int layer_a_second_color_index;
	int layer_b_first_color_index;
	int layer_b_second_color_index;
	int layer_c_first_color_index;
	int layer_c_second_color_index;
};

struct sfx_flags_struct {
	bool sfx_1_used;
	bool sfx_2_used;
	bool sfx_3_used;
	bool sfx_4_used;
};

// All framerate values
const double framerates[11] = { 0.2, 0.5, 1, 2, 4, 6, 8, 12, 20, 24, 30 };

// Line tables
uint8_t line_table[6561][8] = { 0 };
uint8_t line_table_shifted[6561][8] = { 0 };

const uint16_t line_index_table_common[32] = { 0x0000, 0x0CD0, 0x19A0, 0x02D9, 0x088B, 0x0051, 0x00F3, 0x0009,
											   0x001B, 0x0001, 0x0003, 0x05B2, 0x1116, 0x00A2, 0x01E6, 0x0012,
											   0x0036, 0x0002, 0x0006, 0x0B64, 0x08DC, 0x0144, 0x00FC, 0x0024,
											   0x001C, 0x0004, 0x0334, 0x099C, 0x0668, 0x1338, 0x1004, 0x166C };

const uint16_t line_index_table_common_shifted[32] = { 0x0000, 0x0CD0, 0x19A0, 0x0003, 0x02D9, 0x088B, 0x0051, 0x00F3,
													   0x0009, 0x001B, 0x0001, 0x0006, 0x05B2, 0x1116, 0x00A2, 0x01E6,
													   0x0012, 0x0036, 0x0002, 0x02DC, 0x0B64, 0x08DC, 0x0144, 0x00FC,
													   0x0024, 0x001C, 0x099C, 0x0334, 0x1338, 0x0668, 0x166C, 0x1004 };

const uint8_t palette[7][3] = { {0xff, 0xff, 0xff},   // White
								{0x10, 0x10, 0x10},   // Black
								{0xff, 0x10, 0x10},   // Red
								{0xff, 0xe7, 0x00},   // Yellow
								{0x00, 0x86, 0x31},   // Green
								{0x00, 0x38, 0xce},   // Blue
								{0xff, 0xff, 0xff} }; // Transparent 


// Audio processing variables
// Initial ADPCM State
const int initial_step_index = 40;
const int initial_predictor = 0;
const int initial_sample = 0;
const int initial_step = 0;
const int initial_diff = 0;

// ADPCM Clamping & Scale
const int16_t step_index_clamp_min = 0;
const int16_t step_index_clamp_max = 79;
const int16_t predictor_clamp_min = -2048;
const int16_t predictor_clamp_max = 2047;
const int16_t predictor_scale = 16;

// ADPCM Sampling
const int variable_threshold = 18;
const int8_t adpcm_index_table_2_bit[4] = { -1, 2, -1, 2 };
const int8_t adpcm_index_table_4_bit[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
const uint16_t adpcm_step_table[96] = { 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19,
										21, 23, 25, 28, 31, 34, 37, 41, 45, 50,
										55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
										143, 157, 173, 190, 209, 230, 253, 279, 307,
										337, 371, 408, 449, 494, 544, 598, 658, 724,
										796, 876, 963, 1060, 1166, 1282, 1411, 1552,
										1707, 1878, 2066, 2272, 2499, 2749, 3024,
										3327, 3660, 4026, 4428, 4871, 5358, 5894,
										6484, 7132, 7845, 8630, 9493, 10442, 11487,
										12635, 13899, 15289, 16818, 18500, 20350,
										22385, 24623, 27086, 29794, 32767, 0 };

uint32_t flipnote_speed_when_recorded = 0;
int bgm_size = 0;
int se_1_size = 0;
int se_2_size = 0;
int se_3_size = 0;
int se_4_size = 0;

const int16_t adpcm_step_table_2_bit[4] = { -1, 2, -1, 2 };

const int16_t adpcm_step_table_4_bit[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };

const int16_t adpcm_sample_table_2_bit[360] = {
										  0, 7, 0, -7, 1, 9, -1, -9, 1, 10, -1, -10, 1, 11, -1,
										 -11, 1, 12, -1, -12, 1, 13, -1, -13, 1, 14, -1, -14, 1,
										  15, -1, -15, 2, 18, -2, -18, 2, 19, -2, -19, 2, 21, -2,
										 -21, 2, 23, -2, -23, 2, 25, -2, -25, 3, 28, -3, -28, 3,
										  31, -3, -31, 3, 34, -3, -34, 4, 38, -4, -38, 4, 41, -4,
										 -41, 5, 46, -5, -46, 5, 50, -5, -50, 6, 56, -6, -56, 6,
										  61, -6, -61, 7, 67, -7, -67, 8, 74, -8, -74, 9, 82, -9,
										 -82, 10, 90, -10, -90, 11, 99, -11, -99, 12, 109, -12,
										 -109, 13, 120, -13, -120, 14, 132, -14, -132, 16, 146,
										 -16, -146, 17, 160, -17, -160, 19, 176, -19, -176, 21,
										  194, -21, -194, 23, 213, -23, -213, 26, 235, -26, -235,
										  28, 258, -28, -258, 31, 284, -31, -284, 34, 313, -34,
										 -313, 38, 345, -38, -345, 42, 379, -42, -379, 46, 417,
										 -46, -417, 51, 459, -51, -459, 56, 505, -56, -505, 61,
										  555, -61, -555, 68, 612, -68, -612, 74, 672, -74, -672,
										  82, 740, -82, -740, 90, 814, -90, -814, 99, 895, -99,
										 -895, 109, 985, -109, -985, 120, 1083, -120, -1083, 132,
										  1192, -132, -1192, 145, 1311, -145, -1311, 160, 1442,
										 -160, -1442, 176, 1587, -176, -1587, 194, 1746, -194,
										 -1746, 213, 1920, -213, -1920, 234, 2112, -234, -2112,
										  258, 2324, -258, -2324, 284, 2556, -284, -2556, 312,
										  2811, -312, -2811, 343, 3092, -343, -3092, 378, 3402,
										 -378, -3402, 415, 3742, -415, -3742, 457, 4117, -457,
										 -4117, 503, 4529, -503, -4529, 553, 4981, -553, -4981,
										  608, 5479, -608, -5479, 669, 6027, -669, -6027, 736,
										  6630, -736, -6630, 810, 7294, -810, -7294, 891, 8023,
										 -891, -8023, 980, 8825, -980, -8825, 1078, 9708, -1078,
										 -9708, 1186, 10679, -1186, -10679, 1305, 11747, -1305,
										 -11747, 1435, 12922, -1435, -12922, 1579, 14214, -1579,
										 -14214, 1737, 15636, -1737, -15636, 1911, 17200, -1911,
										 -17200, 2102, 18920, -2102, -18920, 2312, 20812, -2312,
										 -20812, 2543, 22893, -2543, -22893, 2798, 25183, -2798,
										 -25183, 3077, 27700, -3077, -27700, 3385, 30471, -3385,
										 -30471, 3724, -32018, -3724, 32018, 4095, -28674, -4095,
										  28674, 0, 0, 0, 0 };

const int16_t adpcm_sample_table_4_bit[1440] = {
										   0, 1, 3, 4, 7, 8, 10, 11, 0, -1, -3, -4, -7, -8, -10,
										  -11, 1, 3, 5, 7, 9, 11, 13, 15, -1, -3, -5, -7, -9, -11,
										  -13, -15, 1, 3, 5, 7, 10, 12, 14, 16, -1, -3, -5, -7,
										  -10, -12, -14, -16, 1, 3, 6, 8, 11, 13, 16, 18, -1, -3,
										  -6, -8, -11, -13, -16, -18, 1, 3, 6, 8, 12, 14, 17, 19,
										  -1, -3, -6, -8, -12, -14, -17, -19, 1, 4, 7, 10, 13, 16,
										   19, 22, -1, -4, -7, -10, -13, -16, -19, -22, 1, 4, 7,
										   10, 14, 17, 20, 23, -1, -4, -7, -10, -14, -17, -20, -23,
										   1, 4, 8, 11, 15, 18, 22, 25, -1, -4, -8, -11, -15, -18,
										  -22, -25, 2, 6, 10, 14, 18, 22, 26, 30, -2, -6, -10, -14,
										  -18, -22, -26, -30, 2, 6, 10, 14, 19, 23, 27, 31, -2, -6,
										  -10, -14, -19, -23, -27, -31, 2, 6, 11, 15, 21, 25, 30, 34,
										  -2, -6, -11, -15, -21, -25, -30, -34, 2, 7, 12, 17, 23, 28,
										   33, 38, -2, -7, -12, -17, -23, -28, -33, -38, 2, 7, 13,
										   18, 25, 30, 36, 41, -2, -7, -13, -18, -25, -30, -36, -41,
										   3, 9, 15, 21, 28, 34, 40, 46, -3, -9, -15, -21, -28, -34,
										  -40, -46, 3, 10, 17, 24, 31, 38, 45, 52, -3, -10, -17, -24,
										  -31, -38, -45, -52, 3, 10, 18, 25, 34, 41, 49, 56, -3, -10,
										  -18, -25, -34, -41, -49, -56, 4, 12, 21, 29, 38, 46, 55, 63,
										  -4, -12, -21, -29, -38, -46, -55, -63, 4, 13, 22, 31, 41,
										   50, 59, 68, -4, -13, -22, -31, -41, -50, -59, -68, 5, 15,
										   25, 35, 46, 56, 66, 76, -5, -15, -25, -35, -46, -56, -66,
										  -76, 5, 16, 27, 38, 50, 61, 72, 83, -5, -16, -27, -38, -50,
										  -61, -72, -83, 6, 18, 31, 43, 56, 68, 81, 93, -6, -18, -31,
										  -43, -56, -68, -81, -93, 6, 19, 33, 46, 61, 74, 88, 101, -6,
										  -19, -33, -46, -61, -74, -88, -101, 7, 22, 37, 52, 67, 82,
										   97, 112, -7, -22, -37, -52, -67, -82, -97, -112, 8, 24,
										   41, 57, 74, 90, 107, 123, -8, -24, -41, -57, -74, -90,
										  -107, -123, 9, 27, 45, 63, 82, 100, 118, 136, -9, -27, -45,
										  -63, -82, -100, -118, -136, 10, 30, 50, 70, 90, 110, 130,
										   150, -10, -30, -50, -70, -90, -110, -130, -150, 11, 33,
										   55, 77, 99, 121, 143, 165, -11, -33, -55, -77, -99, -121,
										  -143, -165, 12, 36, 60, 84, 109, 133, 157, 181, -12, -36,
										  -60, -84, -109, -133, -157, -181, 13, 39, 66, 92, 120,
										   146, 173, 199, -13, -39, -66, -92, -120, -146, -173,
										  -199, 14, 43, 73, 102, 132, 161, 191, 220, -14, -43, -73,
										  -102, -132, -161, -191, -220, 16, 48, 81, 113, 146, 178,
										   211, 243, -16, -48, -81, -113, -146, -178, -211, -243, 17,
										   52, 88, 123, 160, 195, 231, 266, -17, -52, -88, -123, -160,
										  -195, -231, -266, 19, 58, 97, 136, 176, 215, 254, 293, -19,
										  -58, -97, -136, -176, -215, -254, -293, 21, 64, 107, 150,
										   194, 237, 280, 323, -21, -64, -107, -150, -194, -237, -280,
										  -323, 23, 70, 118, 165, 213, 260, 308, 355, -23, -70, -118,
										  -165, -213, -260, -308, -355, 26, 78, 130, 182, 235, 287, 339,
										   391, -26, -78, -130, -182, -235, -287, -339, -391, 28, 85,
										   143, 200, 258, 315, 373, 430, -28, -85, -143, -200, -258,
										  -315, -373, -430, 31, 94, 157, 220, 284, 347, 410, 473, -31,
										  -94, -157, -220, -284, -347, -410, -473, 34, 103, 173, 242,
										   313, 382, 452, 521, -34, -103, -173, -242, -313, -382, -452,
										  -521, 38, 114, 191, 267, 345, 421, 498, 574, -38, -114, -191,
										  -267, -345, -421, -498, -574, 42, 126, 210, 294, 379, 463,
										   547, 631, -42, -126, -210, -294, -379, -463, -547, -631,
										   46, 138, 231, 323, 417, 509, 602, 694, -46, -138, -231,
										  -323, -417, -509, -602, -694, 51, 153, 255, 357, 459, 561,
										   663, 765, -51, -153, -255, -357, -459, -561, -663, -765,
										   56, 168, 280, 392, 505, 617, 729, 841, -56, -168, -280,
										  -392, -505, -617, -729, -841, 61, 184, 308, 431, 555, 678,
										   802, 925, -61, -184, -308, -431, -555, -678, -802, -925,
										   68, 204, 340, 476, 612, 748, 884, 1020, -68, -204, -340,
										  -476, -612, -748, -884, -1020, 74, 223, 373, 522, 672, 821,
										   971, 1120, -74, -223, -373, -522, -672, -821, -971, -1120,
										   82, 246, 411, 575, 740, 904, 1069, 1233, -82, -246, -411,
										  -575, -740, -904, -1069, -1233, 90, 271, 452, 633, 814, 995,
										   1176, 1357, -90, -271, -452, -633, -814, -995, -1176, -1357,
										   99, 298, 497, 696, 895, 1094, 1293, 1492, -99, -298, -497,
										  -696, -895, -1094, -1293, -1492, 109, 328, 547, 766, 985,
										   1204, 1423, 1642, -109, -328, -547, -766, -985, -1204, -1423,
										  -1642, 120, 360, 601, 841, 1083, 1323, 1564, 1804, -120, -360,
										  -601, -841, -1083, -1323, -1564, -1804, 132, 397, 662, 927,
										   1192, 1457, 1722, 1987, -132, -397, -662, -927, -1192, -1457,
										  -1722, -1987, 145, 436, 728, 1019, 1311, 1602, 1894, 2185,
										  -145, -436, -728, -1019, -1311, -1602, -1894, -2185, 160, 480,
										   801, 1121, 1442, 1762, 2083, 2403, -160, -480, -801, -1121,
										  -1442, -1762, -2083, -2403, 176, 528, 881, 1233, 1587, 1939,
										   2292, 2644, -176, -528, -881, -1233, -1587, -1939, -2292,
										  -2644, 194, 582, 970, 1358, 1746, 2134, 2522, 2910, -194,
										  -582, -970, -1358, -1746, -2134, -2522, -2910, 213, 639, 1066,
										   1492, 1920, 2346, 2773, 3199, -213, -639, -1066, -1492, -1920,
										  -2346, -2773, -3199, 234, 703, 1173, 1642, 2112, 2581, 3051,
										   3520, -234, -703, -1173, -1642, -2112, -2581, -3051, -3520,
										   258, 774, 1291, 1807, 2324, 2840, 3357, 3873, -258, -774,
										  -1291, -1807, -2324, -2840, -3357, -3873, 284, 852, 1420, 1988,
										   2556, 3124, 3692, 4260, -284, -852, -1420, -1988, -2556, -3124,
										  -3692, -4260, 312, 936, 1561, 2185, 2811, 3435, 4060, 4684,
										  -312, -936, -1561, -2185, -2811, -3435, -4060, -4684, 343,
										   1030, 1717, 2404, 3092, 3779, 4466, 5153, -343, -1030, -1717,
										  -2404, -3092, -3779, -4466, -5153, 378, 1134, 1890, 2646, 3402,
										   4158, 4914, 5670, -378, -1134, -1890, -2646, -3402, -4158,
										  -4914, -5670, 415, 1246, 2078, 2909, 3742, 4573, 5405, 6236,
										  -415, -1246, -2078, -2909, -3742, -4573, -5405, -6236, 457,
										   1372, 2287, 3202, 4117, 5032, 5947, 6862, -457, -1372, -2287,
										  -3202, -4117, -5032, -5947, -6862, 503, 1509, 2516, 3522, 4529,
										   5535, 6542, 7548, -503, -1509, -2516, -3522, -4529, -5535,
										  -6542, -7548, 553, 1660, 2767, 3874, 4981, 6088, 7195, 8302,
										  -553, -1660, -2767, -3874, -4981, -6088, -7195, -8302, 608,
										   1825, 3043, 4260, 5479, 6696, 7914, 9131, -608, -1825, -3043,
										  -4260, -5479, -6696, -7914, -9131, 669, 2008, 3348, 4687, 6027,
										   7366, 8706, 10045, -669, -2008, -3348, -4687, -6027, -7366,
										  -8706, -10045, 736, 2209, 3683, 5156, 6630, 8103, 9577, 11050,
										  -736, -2209, -3683, -5156, -6630, -8103, -9577, -11050, 810,
										   2431, 4052, 5673, 7294, 8915, 10536, 12157, -810, -2431,
										  -4052, -5673, -7294, -8915, -10536, -12157, 891, 2674, 4457,
										   6240, 8023, 9806, 11589, 13372, -891, -2674, -4457, -6240,
										  -8023, -9806, -11589, -13372, 980, 2941, 4902, 6863, 8825,
										   10786, 12747, 14708, -980, -2941, -4902, -6863, -8825,
										  -10786, -12747, -14708, 1078, 3235, 5393, 7550, 9708, 11865,
										   14023, 16180, -1078, -3235, -5393, -7550, -9708, -11865,
										  -14023, -16180, 1186, 3559, 5932, 8305, 10679, 13052, 15425,
										   17798, -1186, -3559, -5932, -8305, -10679, -13052, -15425,
										  -17798, 1305, 3915, 6526, 9136, 11747, 14357, 16968, 19578,
										  -1305, -3915, -6526, -9136, -11747, -14357, -16968, -19578,
										   1435, 4306, 7178, 10049, 12922, 15793, 18665, 21536, -1435,
										  -4306, -7178, -10049, -12922, -15793, -18665, -21536, 1579,
										   4737, 7896, 11054, 14214, 17372, 20531, 23689, -1579, -4737,
										  -7896, -11054, -14214, -17372, -20531, -23689, 1737, 5211,
										   8686, 12160, 15636, 19110, 22585, 26059, -1737, -5211,
										  -8686, -12160, -15636, -19110, -22585, -26059, 1911, 5733,
										   9555, 13377, 17200, 21022, 24844, 28666, -1911, -5733,
										  -9555, -13377, -17200, -21022, -24844, -28666, 2102, 6306,
										   10511, 14715, 18920, 23124, 27329, 31533, -2102, -6306,
										  -10511, -14715, -18920, -23124, -27329, -31533, 2312, 6937,
										   11562, 16187, 20812, 25437, 30062, -30849, -2312, -6937,
										  -11562, -16187, -20812, -25437, -30062, 30849, 2543, 7630,
										   12718, 17805, 22893, 27980, -32468, -27381, -2543, -7630,
										  -12718, -17805, -22893, -27980, 32468, 27381, 2798, 8394,
										   13990, 19586, 25183, 30779, -29161, -23565, -2798, -8394,
										  -13990, -19586, -25183, -30779, 29161, 23565, 3077, 9232,
										   15388, 21543, 27700, -31681, -25525, -19370, -3077, -9232,
										  -15388, -21543, -27700, 31681, 25525, 19370, 3385, 10156,
										   16928, 23699, 30471, -28294, -21522, -14751, -3385, -10156,
										  -16928, -23699, -30471, 28294, 21522, 14751, 3724, 11172,
										   18621, 26069, -32018, -24570, -17121, -9673, -3724, -11172,
										  -18621, -26069, 32018, 24570, 17121, 9673, 4095, 12286, 20478,
										   28669, -28674, -20483, -12291, -4100, -4095, -12286, -20478,
										  -28669, 28674, 20483, 12291, 4100, 0, 0, 0, 0, 0, 0, 0, 0, 0,
										   0, 0, 0, 0, 0, 0, 0 };

int16_t audio_buffer[16346 * 60] = { 0 };
int audio_buffer_length = 0;

typedef struct WAV_HEADER {
	// RIFF Chunk
	uint8_t riff_header[4] = { 'R', 'I', 'F', 'F' };
	uint32_t chunk_size = 0;
	uint8_t wave_header[4] = { 'W', 'A', 'V', 'E' };
	// fmt sub chunk
	uint8_t fmt_header[4] = { 'f', 'm', 't', ' ' };
	uint32_t subchunk_1_size = 16;
	uint16_t audio_format = 1; // Audio format, 1=PCM
	uint16_t num_channels = 1; // Number of channels, 1=Mono
	uint32_t sample_rate = 16364; // Sampling Frequency in Hz
	uint32_t bit_rate = 32653; // Bytes/second
	uint16_t block_align = 2; // 2=16-bit mono
	uint16_t bits_per_sample = 16; // Bits per sample
	// data sub chunk
	uint8_t data_header[4] = { 'd', 'a', 't', 'a' };
	uint32_t subchunk_2_size = 0; // Data length
} wav_hdr;