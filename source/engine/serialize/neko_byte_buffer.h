#ifndef NEKO_BYTE_BUFFER_H
#define NEKO_BYTE_BUFFER_H

#include "engine/common/neko_types.h"
#include "engine/math/neko_math.h"

#define neko_byte_buffer_default_capacity 1024

/* Byte buffer */
typedef struct neko_byte_buffer_t {
    u8* data;      // Buffer that actually holds all relevant byte data
    u32 size;      // Current size of the stored buffer data
    u32 position;  // Current read/write position in the buffer
    u32 capacity;  // Current max capacity for the buffer
} neko_byte_buffer_t;

// Generic "write" function for a byte buffer
#define neko_byte_buffer_write(bb, T, val)               \
    do {                                                 \
        neko_byte_buffer_t* _buffer = bb;                \
        usize sz = sizeof(T);                            \
        usize total_write_size = _buffer->position + sz; \
        if (total_write_size >= _buffer->capacity) {     \
            usize capacity = _buffer->capacity * 2;      \
            while (capacity < total_write_size) {        \
                capacity *= 2;                           \
            }                                            \
            neko_byte_buffer_resize(_buffer, capacity);  \
        }                                                \
        *(T*)(_buffer->data + _buffer->position) = val;  \
        _buffer->position += sz;                         \
        _buffer->size += sz;                             \
    } while (0)

// Generic "read" function
#define neko_byte_buffer_read(_buffer, T, _val_p) \
    do {                                          \
        T* _v = (T*)(_val_p);                     \
        neko_byte_buffer_t* _bb = (_buffer);      \
        *(_v) = *(T*)(_bb->data + _bb->position); \
        _bb->position += sizeof(T);               \
    } while (0)

// Defines variable and sets value from buffer in place
// Use to construct a new variable
#define neko_byte_buffer_readc(_buffer, T, name) \
    T name = neko_default_val();                 \
    neko_byte_buffer_read((_buffer), T, &name);

void neko_byte_buffer_init(neko_byte_buffer_t* buffer);
neko_byte_buffer_t neko_byte_buffer_new();
void neko_byte_buffer_free(neko_byte_buffer_t* buffer);
void neko_byte_buffer_clear(neko_byte_buffer_t* buffer);
void neko_byte_buffer_resize(neko_byte_buffer_t* buffer, usize sz);
void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t* buffer);
void neko_byte_buffer_seek_to_end(neko_byte_buffer_t* buffer);
void neko_byte_buffer_advance_position(neko_byte_buffer_t* buffer, usize sz);
void neko_byte_buffer_write_str(neko_byte_buffer_t* buffer, const char* str);  // Expects a null terminated string
void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str);         // Expects an allocated string
void neko_byte_buffer_bulk_write(neko_byte_buffer_t* buffer, void* src, u32 sz);
void neko_byte_buffer_bulk_read(neko_byte_buffer_t* buffer, void* dst, u32 sz);
neko_result neko_byte_buffer_write_to_file(neko_byte_buffer_t* buffer, const char* output_path);  // Assumes that the output directory exists
neko_result neko_byte_buffer_read_from_file(neko_byte_buffer_t* buffer, const char* file_path);   // Assumes an allocated byte buffer

#endif  // NEKO_BYTE_BUFFER_H
