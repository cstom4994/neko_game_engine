#include "engine/serialize/neko_byte_buffer.h"

#include "engine/common/neko_mem.h"
#include "engine/common/neko_types.h"
#include "engine/common/neko_util.h"

void neko_byte_buffer_init(neko_byte_buffer_t* buffer) {
    buffer->data = (u8*)neko_malloc(neko_byte_buffer_default_capacity);
    buffer->capacity = neko_byte_buffer_default_capacity;
    buffer->size = 0;
    buffer->position = 0;
}

neko_byte_buffer_t neko_byte_buffer_new() {
    neko_byte_buffer_t buffer;
    neko_byte_buffer_init(&buffer);
    return buffer;
}

void neko_byte_buffer_free(neko_byte_buffer_t* buffer) {
    if (buffer && buffer->data) {
        neko_free(buffer->data);
    }
}

void neko_byte_buffer_clear(neko_byte_buffer_t* buffer) {
    buffer->size = 0;
    buffer->position = 0;
}

void neko_byte_buffer_resize(neko_byte_buffer_t* buffer, usize sz) {
    u8* data = (u8*)neko_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;
    buffer->capacity = sz;
}

void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t* buffer) { buffer->position = 0; }

void neko_byte_buffer_seek_to_end(neko_byte_buffer_t* buffer) { buffer->position = buffer->size; }

void neko_byte_buffer_advance_position(neko_byte_buffer_t* buffer, usize sz) { buffer->position += sz; }

void neko_byte_buffer_bulk_write(neko_byte_buffer_t* buffer, void* src, u32 size) {
    // Check for necessary resize
    u32 total_write_size = buffer->position + size;
    if (total_write_size >= buffer->capacity) {
        usize capacity = buffer->capacity * 2;
        while (capacity <= total_write_size) {
            capacity *= 2;
        }

        neko_byte_buffer_resize(buffer, capacity);
    }

    // memcpy data
    memcpy((buffer->data + buffer->position), src, size);

    buffer->size += size;
    buffer->position += size;
}

void neko_byte_buffer_bulk_read(neko_byte_buffer_t* buffer, void* dst, u32 size) {
    memcpy(dst, (buffer->data + buffer->position), size);
    buffer->position += size;
}

void neko_byte_buffer_write_str(neko_byte_buffer_t* buffer, const char* str) {
    // Write size of string
    u32 str_len = neko_string_length(str);
    neko_byte_buffer_write(buffer, u16, str_len);

    usize i;
    for (i = 0; i < str_len; ++i) {
        neko_byte_buffer_write(buffer, u8, str[i]);
    }
}

void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str) {
    // Read in size of string from buffer
    u16 sz;
    neko_byte_buffer_read(buffer, u16, &sz);

    u32 i;
    for (i = 0; i < sz; ++i) {
        neko_byte_buffer_read(buffer, u8, &str[i]);
    }
    str[i] = '\0';
}

neko_result neko_byte_buffer_write_to_file(neko_byte_buffer_t* buffer, const char* output_path) {
    FILE* fp = fopen(output_path, "wb");
    if (fp) {
        s32 ret = fwrite(buffer->data, sizeof(u8), buffer->size, fp);
        if (ret == buffer->size) {
            return neko_result_success;
        }
    }
    return neko_result_failure;
}

neko_result neko_byte_buffer_read_from_file(neko_byte_buffer_t* buffer, const char* file_path) {
    buffer->data = (u8*)neko_read_file_contents_into_string_null_term(file_path, "rb", (usize*)&buffer->size);
    if (!buffer->data) {
        neko_assert(false);
        return neko_result_failure;
    }
    buffer->position = 0;
    buffer->capacity = buffer->size;
    return neko_result_success;
}
