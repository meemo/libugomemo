#include <stdio.h>
#include <stdlib.h>

#include <libugomemo.h>

typedef struct ppm_file_new {
    /* Offset 0x0 (file header) */
    u8  magic[4];
    u32 animation_data_size;
    u32 sound_data_size;
    u16 frame_count;
    u16 format_version;
    /* Offset 0x10 (metadata) */
    u16 lock;
    u16 thumbnail_frame_index;
    s16 root_author_name[11];
    s16 parent_author_name[11];
    s16 current_author_name[11];
    u8  parent_author_ID[8];
    u8  current_author_ID[8];
    u8  parent_file_name[18];
    u8  current_file_name[18];
    u8  root_author_ID[8];
    u8  root_file_name_fragment[8];
    u32 timestamp;
    u16 unused_null;
    /* Offset 0xA0 */
    u8  thumbnail_data[0x600];
    /* Offset 0x6A0 (animation header) */
    u16 offset_table_size;
    u16 unused;  /* Always seen as null */
    u16 flags;
    /* Immediately following this section (offset 0x6A8) are the frame offsets sequentially. */
} ppm_file_new;

int write_ppm_track(u8 *file_buffer, int track_offset, int track_size) {
    FILE *output_file;
    s16 *output_buffer;
    int output_size;
    struct wav_header header = {
        { 'R', 'I', 'F', 'F' },
        0, /* Size of the audio data + 36 */
        { 'W', 'A', 'V', 'E' },
        { 'f', 'm', 't', ' ' },
        16,
        1,
        1,
        8192,
        8192 * 2,
        2,
        16,
        { 'd', 'a', 't', 'a' },
        0 /* Size of the audio data */
    };

    /* Larger than the largest possible audio value */
    output_buffer = (s16 *) malloc(0x100000);

    output_size = PPMDecodeTrack(file_buffer, output_buffer, track_offset, track_size);

    header.chunk_size = output_size + 36;
    header.subchunk_2_size = output_size;

    output_file = fopen("tests/out.wav", "wb");
    fwrite(&header, sizeof(wav_header), 1, output_file);
    fwrite(output_buffer, sizeof(s16), output_size, output_file);
    fclose(output_file);

    free(output_buffer);

    return 0;
}

int main(void) {
    FILE *input_file;
    long file_size;
    u8 *file_contents;
    ppm_file_new     *file_meta;
    ppm_sound_header *sound_header;
    int sound_offset;
    int bgm_offset;

    /* Read file into a buffer. */
    input_file = fopen("tests/samples/U19FA7_1307576D91807_000.ppm", "rb");

    fseek(input_file, 0, SEEK_END);
    file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    file_contents = (u8 *) malloc(file_size + 1);
    fread(file_contents, file_size, sizeof(u8), input_file);

    fclose(input_file);

    /* Set up references to file. */
    file_meta = (ppm_file_new *) file_contents;

    sound_offset = ROUND_MULT_4(0x6A0 + file_meta->animation_data_size + file_meta->frame_count);
    bgm_offset = sound_offset + sizeof(ppm_sound_header);

    sound_header = (ppm_sound_header *) &file_contents[sound_offset];

    printf("BGM size: %d\n", sound_header->bgm_size);
    printf("BGM offset: %d\n", bgm_offset);

    write_ppm_track(file_contents, sound_header->bgm_size, bgm_offset);

    return 0;
}
