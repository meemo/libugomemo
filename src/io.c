#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ugomemo.h>

#include <zlib.h>

// Home of the file data interaction abstraction

inline u8 read_le8(u8 *buf, size_t pos) {
    return buf[pos];
}

inline u16 read_le16(u8 *buf, size_t pos) {
    return (u16)buf[pos] | ((u16)buf[pos + 1] << 8);
}

inline u32 read_le32(u8 *buf, size_t pos) {
    return ((u32)buf[pos + 0] <<  0) | ((u32)buf[pos + 1] << 8) |
           ((u32)buf[pos + 2] << 16) | ((u32)buf[pos + 3] << 24);
}

inline u32 read_be32(u8 *buf, size_t pos) {
    return ((u32)buf[pos] << 24) | ((u32)buf[pos + 1] << 16) |
           ((u32)buf[pos + 2] << 8) | ((u32)buf[pos + 3]);
}

int decompress_buffer(u8 *input, size_t input_len, u8 **output, size_t *output_len) {
    int ret;

    if (!input || !output || !output_len || !input_len) {
        return Z_BUF_ERROR;
    }

    // Initialize with a reasonable buffer size
    size_t initial_size = input_len * 5;
    *output = calloc(1, initial_size);
    if (!*output) {
        return Z_MEM_ERROR;
    }

    z_stream strm = {0};

    if ((ret = inflateInit2(&strm, 16 + MAX_WBITS)) != Z_OK) {
        free(*output);
        *output = NULL;
        return ret;
    }

    strm.next_in = (Bytef *)input;
    strm.avail_in = input_len;
    strm.next_out = *output;
    strm.avail_out = initial_size;

    while (1) {
        ret = inflate(&strm, Z_NO_FLUSH);

        if (ret == Z_STREAM_END) {
            break;
        }

        // Corrupt gzip stream errors handled here
        if (ret != Z_OK) {
            inflateEnd(&strm);
            free(*output);
            *output = NULL;
            ERROR("gzip decompression error, ret=%d\n", ret);
            return UGOMEMO_INPUT_ERROR;
        }

        if (strm.avail_out == 0) {
            // Buffer too small, realloc with double size
            size_t new_size = initial_size * 2;
            uint8_t *new_buf = realloc(*output, new_size);
            if (!new_buf) {
                inflateEnd(&strm);
                free(*output);
                *output = NULL;
                return UGOMEMO_MEMORY_ERROR;
            }

            strm.next_out = new_buf + (initial_size - strm.avail_out);
            strm.avail_out = new_size - (initial_size - strm.avail_out);
            *output = new_buf;
            initial_size = new_size;
        }
    }

    *output_len = strm.total_out;
    inflateEnd(&strm);
    return UGOMEMO_OK;
}

int file_init(ugomemo_file **file, char *path, bool writable) {
    int res = UGOMEMO_OK;
    ugomemo_file *init_file;

    init_file = (ugomemo_file *) calloc(1, sizeof(ugomemo_file));
    if (init_file == NULL) {
        ERROR("Failed to allocate memory for file.\n");
        return UGOMEMO_MEMORY_ERROR;
    }

    init_file->path = path;
    init_file->is_writable = writable;
    init_file->current_offset = 0;

    if (writable)
        init_file->file = fopen(init_file->path, "wb");
    else
        init_file->file = fopen(init_file->path, "rb");

    if (!init_file->file) {
        ERROR("Failed to open file: %s\n", init_file->path);
        free(init_file);
        return UGOMEMO_INPUT_ERROR;
    }

    *file = init_file;

    return res;
}

int file_read(ugomemo_file *file) {
    int res = UGOMEMO_OK, decompress_count = 0;

    // Get file size
    fseek(file->file, 0L, SEEK_END);
    file->size = ftell(file->file);
    fseek(file->file, 0L, SEEK_SET);
    if (file->size == 0) {
        ERROR("File is 0 bytes\n");
        return UGOMEMO_INPUT_ERROR;
    } else if (file->size > ((size_t)1 << 47)) {
        ERROR("File is crazy huge (>16TB), you sure about this boss?\n");
        return UGOMEMO_INPUT_ERROR;
    }

    // Allocate memory
    file->data = (u8 *) calloc(file->size, sizeof(u8));
    if (file->data == NULL) {
        ERROR("Failed to allocate memory for file\n");
        return UGOMEMO_INPUT_ERROR;
    }

    // Read file data
    size_t bytes_read = fread(file->data, sizeof(u8), file->size, file->file);
    if (bytes_read != file->size) {
        ERROR("Failed to read entire file: got %zu of %zu bytes\n", bytes_read, file->size);
        free(file->data);
        fclose(file->file);
        file->file = NULL;
        return UGOMEMO_INPUT_ERROR;
    }
    fclose(file->file);
    file->file = NULL;

    // Decompress if needed, looping because of the double gzip oopsie
    file->was_compressed = false;
    while (file->data[0] == 0x1F) {
        if (decompress_count > 10) {
            ERROR("Failed to decompress file after 10 inflates\n");
            exit(UGOMEMO_INPUT_ERROR);
        }

        u8 *compressed_file_data = file->data;
        file->compressed_size = file->size;
        int res = decompress_buffer(
            compressed_file_data, file->compressed_size,
            &file->data, &file->size
        );
        if (res != 0 || file->data == NULL) {
            ERROR("Failed to decompress file. res=%d\n", res);
            return res;
        }
        free(compressed_file_data);
        file->was_compressed = true;
        decompress_count++;
    }

    return res;
}

inline int file_read_le8(ugomemo_file *f, u8 *res) {
    if (f->current_offset + 1 > f->size)
        return UGOMEMO_INPUT_ERROR;

    *res = f->data[f->current_offset];
    f->current_offset += 1;

    return UGOMEMO_OK;
}

inline int file_read_le16(ugomemo_file *f, u16 *res) {
    if (f->current_offset + 2 > f->size)
        return UGOMEMO_INPUT_ERROR;

    *res = (u16)f->data[f->current_offset] | ((u16)f->data[f->current_offset + 1] << 8);
    f->current_offset += 2;

    return UGOMEMO_OK;
}

inline int file_read_le32(ugomemo_file *f, u32 *res) {
    if (f->current_offset + 4 > f->size)
        return UGOMEMO_INPUT_ERROR;

    *res = ((u32)f->data[f->current_offset + 0] <<  0) | ((u32)f->data[f->current_offset + 1] << 8) |
           ((u32)f->data[f->current_offset + 2] << 16) | ((u32)f->data[f->current_offset + 3] << 24);
    f->current_offset += 4;

    return UGOMEMO_OK;
}

// This may seem like excessive abstraction, but this is mostly for the bounds checking and consistency
inline int file_read_buf(ugomemo_file *f, u8 **res, size_t size) {
    if (f->current_offset + size > f->size)
        return UGOMEMO_INPUT_ERROR;

    *res = f->data + f->current_offset;
    f->current_offset += size;

    return UGOMEMO_OK;
}

// Read from a specific offset not using the stored offset
inline int file_read_buf_absolute(ugomemo_file *f, u8 **res, size_t offset, size_t size) {
    if (offset + size > f->size)
        return UGOMEMO_INPUT_ERROR;

    *res = f->data + offset;

    return UGOMEMO_OK;
}

// Simply checks if that size fits in the remaining buffer
inline int file_test_size(ugomemo_file *f, size_t size) {
    if (f->current_offset + size > f->size)
        return UGOMEMO_INPUT_ERROR;
    else
        return UGOMEMO_OK;
}

inline int file_increment_offset(ugomemo_file *f, size_t amount) {
    if (f->current_offset + amount > f->size)
        return UGOMEMO_INPUT_ERROR;

    f->current_offset += amount;

    return UGOMEMO_OK;
}

inline size_t file_get_offset(ugomemo_file *f) {
    return f->current_offset;
}

inline int file_set_offset(ugomemo_file *f, size_t offset) {
    if (offset > f->size)
        return UGOMEMO_INPUT_ERROR;

    f->current_offset = offset;

    return UGOMEMO_OK;
}

int file_sha256(ugomemo_file *f, size_t offset, size_t size, u8 *digest) {
    if (offset + size > f->size)
        return UGOMEMO_INPUT_ERROR;

    sha256_hash(f->data + offset, size, digest);

    return UGOMEMO_OK;
}

int file_sha1(ugomemo_file *f, size_t offset, size_t size, u8 *digest) {
    if (offset + size > f->size)
        return UGOMEMO_INPUT_ERROR;

    sha1_hash(f->data + offset, size, digest);
    return UGOMEMO_OK;
}

int file_crc32(ugomemo_file *f, size_t offset, size_t size, u32 *out) {
    if (offset + size > f->size)
        return UGOMEMO_INPUT_ERROR;

    *out = get_crc32(f->data + offset, size);

    return UGOMEMO_OK;
}

int file_write(ugomemo_file *f, u8 *data, size_t size) {
    if (f == NULL || data == NULL) return UGOMEMO_INPUT_ERROR;
    if (size == 0) return UGOMEMO_OK;  // Nothing to do
    if (size > ((size_t)1 << 40)) return UGOMEMO_INPUT_ERROR;  // Unreasonable size

    fwrite(data, size, sizeof(u8), f->file);
    f->current_offset += size;

    return UGOMEMO_OK;
}

void file_free(ugomemo_file **file) {
    if (file == NULL || *file == NULL) return;
    ugomemo_file *f = *file;
    if (f->file != NULL) {
        fclose(f->file);
        f->file = NULL;
    }

    free(f->data);
    free(f);
    *file = NULL;
}
