#pragma once

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;
    source.u = u;
    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];
    return dest.u;
}

// Input file buffer
char* file_buffer;

// Offsets
int offset = 0;
int kfh_offset = 0;
int kmi_offset = 0;
int ksn_offset = 0;
int kmc_offset = 0;

// Processing variables
// Video
int layer_buffer_pointer;
int current_layer = 0;
uint16_t bit_value = 0;
int bit_index = 0;

// Audio
// An output buffer with enough space for 60 seconds of audio at 16364 Hz
char* output_buffer = new char[16346 * 60];
int output_offset = 0;
int prev_diff = 0;
// Bugged initial state, set to 40 for console accurate audio
int prev_step_index = 40;

uint32_t flipnote_speed_when_recorded;
uint32_t bgm_size;
uint32_t se_1_size;
uint32_t se_2_size;
uint32_t se_3_size;
uint32_t se_4_size;

// File meta
int file_size = 0;
uint32_t kfh_section_size;
uint32_t file_creation_timestamp;
uint32_t file_last_edit_timestap;
uint32_t app_version;

// Not formatted as xxxx-xxxx-xxxx-xxxxxx, probably not needed?
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
double framerate = 0;
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

// Audio decoding tables
// 2 Bit ADPCM
const int16_t adpcm_step_table_2b[4] = { -1, 2, -1, 2 };
const int16_t adpcm_index_table_2b[4] = { -1, 2, -1, 2 };
int16_t adpcm_sample_table_2b[360] = { 0 };

// 4 Bit ADPCM
const int16_t adpcm_step_table_4b[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
const int16_t adpcm_index_table_4b[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
int16_t adpcm_sample_table_4b[1440] = { 0 };

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
