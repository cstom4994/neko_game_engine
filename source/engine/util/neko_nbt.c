
#include "neko_nbt.h"

#include "engine/neko.h"

typedef struct {
    u8* buffer;
    size_t buffer_offset;
} __neko_nbt_read_stream_t;

static u8 __neko_nbt_get_byte(__neko_nbt_read_stream_t* stream) { return stream->buffer[stream->buffer_offset++]; }

static s16 __neko_nbt_get_int16(__neko_nbt_read_stream_t* stream) {
    u8 bytes[2];
    for (int i = 1; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(s16*)(bytes);
}

static s32 __neko_nbt_get_int32(__neko_nbt_read_stream_t* stream) {
    u8 bytes[4];
    for (int i = 3; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(s32*)(bytes);
}

static s64 __neko_nbt_get_int64(__neko_nbt_read_stream_t* stream) {
    u8 bytes[8];
    for (int i = 7; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(s64*)(bytes);
}

static float __neko_nbt_get_float(__neko_nbt_read_stream_t* stream) {
    u8 bytes[4];
    for (int i = 3; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(float*)(bytes);
}

static double __neko_nbt_get_double(__neko_nbt_read_stream_t* stream) {
    u8 bytes[8];
    for (int i = 7; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(double*)(bytes);
}

static neko_nbt_tag_t* __neko_nbt_parse(__neko_nbt_read_stream_t* stream, int parse_name, neko_nbt_tag_type_t override_type) {

    neko_nbt_tag_t* tag = (neko_nbt_tag_t*)neko_malloc(sizeof(neko_nbt_tag_t));

    if (override_type == NBT_NO_OVERRIDE) {
        tag->type = (neko_nbt_tag_type_t)__neko_nbt_get_byte(stream);
    } else {
        tag->type = override_type;
    }

    if (parse_name && tag->type != NBT_TYPE_END) {
        tag->name_size = __neko_nbt_get_int16(stream);
        tag->name = (char*)neko_malloc(tag->name_size + 1);
        for (size_t i = 0; i < tag->name_size; i++) {
            tag->name[i] = __neko_nbt_get_byte(stream);
        }
        tag->name[tag->name_size] = '\0';
    } else {
        tag->name = NULL;
        tag->name_size = 0;
    }

    switch (tag->type) {
        case NBT_TYPE_END: {
            break;
        }
        case NBT_TYPE_BYTE: {
            tag->tag_byte.value = __neko_nbt_get_byte(stream);
            break;
        }
        case NBT_TYPE_SHORT: {
            tag->tag_short.value = __neko_nbt_get_int16(stream);
            break;
        }
        case NBT_TYPE_INT: {
            tag->tag_int.value = __neko_nbt_get_int32(stream);
            break;
        }
        case NBT_TYPE_LONG: {
            tag->tag_long.value = __neko_nbt_get_int64(stream);
            break;
        }
        case NBT_TYPE_FLOAT: {
            tag->tag_float.value = __neko_nbt_get_float(stream);
            break;
        }
        case NBT_TYPE_DOUBLE: {
            tag->tag_double.value = __neko_nbt_get_double(stream);
            break;
        }
        case NBT_TYPE_BYTE_ARRAY: {
            tag->tag_byte_array.size = __neko_nbt_get_int32(stream);
            tag->tag_byte_array.value = (s8*)neko_malloc(tag->tag_byte_array.size);
            for (size_t i = 0; i < tag->tag_byte_array.size; i++) {
                tag->tag_byte_array.value[i] = __neko_nbt_get_byte(stream);
            }
            break;
        }
        case NBT_TYPE_STRING: {
            tag->tag_string.size = __neko_nbt_get_int16(stream);
            tag->tag_string.value = (char*)neko_malloc(tag->tag_string.size + 1);
            for (size_t i = 0; i < tag->tag_string.size; i++) {
                tag->tag_string.value[i] = __neko_nbt_get_byte(stream);
            }
            tag->tag_string.value[tag->tag_string.size] = '\0';
            break;
        }
        case NBT_TYPE_LIST: {
            tag->tag_list.type = (neko_nbt_tag_type_t)__neko_nbt_get_byte(stream);
            tag->tag_list.size = __neko_nbt_get_int32(stream);
            tag->tag_list.value = (neko_nbt_tag_t**)neko_malloc(tag->tag_list.size * sizeof(neko_nbt_tag_t*));
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                tag->tag_list.value[i] = __neko_nbt_parse(stream, 0, tag->tag_list.type);
            }
            break;
        }
        case NBT_TYPE_COMPOUND: {
            tag->tag_compound.size = 0;
            tag->tag_compound.value = NULL;
            for (;;) {
                neko_nbt_tag_t* inner_tag = __neko_nbt_parse(stream, 1, NBT_NO_OVERRIDE);

                if (inner_tag->type == NBT_TYPE_END) {
                    neko_nbt_free_tag(inner_tag);
                    break;
                } else {
                    tag->tag_compound.value = (neko_nbt_tag_t**)neko_realloc(tag->tag_compound.value, (tag->tag_compound.size + 1) * sizeof(neko_nbt_tag_t*));
                    tag->tag_compound.value[tag->tag_compound.size] = inner_tag;
                    tag->tag_compound.size++;
                }
            }
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            tag->tag_int_array.size = __neko_nbt_get_int32(stream);
            tag->tag_int_array.value = (s32*)neko_malloc(tag->tag_int_array.size * sizeof(s32));
            for (size_t i = 0; i < tag->tag_int_array.size; i++) {
                tag->tag_int_array.value[i] = __neko_nbt_get_int32(stream);
            }
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            tag->tag_long_array.size = __neko_nbt_get_int32(stream);
            tag->tag_long_array.value = (s64*)neko_malloc(tag->tag_long_array.size * sizeof(s64));
            for (size_t i = 0; i < tag->tag_long_array.size; i++) {
                tag->tag_long_array.value[i] = __neko_nbt_get_int64(stream);
            }
            break;
        }
        default: {
            neko_free(tag);
            return NULL;
        }
    }

    return tag;
}

neko_nbt_tag_t* neko_nbt_parse(neko_nbt_reader_t reader, int parse_flags) {

    u8* buffer = NULL;
    size_t buffer_size = 0;

    __neko_nbt_read_stream_t stream;

    u8 in_buffer[NEKO_NBT_BUFFER_SIZE];
    size_t bytes_read;
    do {
        bytes_read = reader.read(reader.userdata, in_buffer, NEKO_NBT_BUFFER_SIZE);
        buffer = (u8*)neko_realloc(buffer, buffer_size + bytes_read);
        memcpy(buffer + buffer_size, in_buffer, bytes_read);
        buffer_size += bytes_read;
    } while (bytes_read == NEKO_NBT_BUFFER_SIZE);

    stream.buffer = buffer;
    stream.buffer_offset = 0;

    neko_nbt_tag_t* tag = __neko_nbt_parse(&stream, 1, NBT_NO_OVERRIDE);

    neko_free(buffer);

    return tag;
}

typedef struct {
    u8* buffer;
    size_t offset;
    size_t size;
    size_t alloc_size;
} __neko_nbt_write_stream_t;

void __neko_nbt_put_byte(__neko_nbt_write_stream_t* stream, u8 value) {
    if (stream->offset >= stream->alloc_size - 1) {
        stream->buffer = (u8*)neko_realloc(stream->buffer, stream->alloc_size * 2);
        stream->alloc_size *= 2;
    }

    stream->buffer[stream->offset++] = value;
    stream->size++;
}

void __neko_nbt_put_int16(__neko_nbt_write_stream_t* stream, s16 value) {
    u8* value_array = (u8*)&value;
    for (int i = 1; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_int32(__neko_nbt_write_stream_t* stream, s32 value) {
    u8* value_array = (u8*)&value;
    for (int i = 3; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_int64(__neko_nbt_write_stream_t* stream, s64 value) {
    u8* value_array = (u8*)&value;
    for (int i = 7; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_float(__neko_nbt_write_stream_t* stream, float value) {
    u8* value_array = (u8*)&value;
    for (int i = 3; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_double(__neko_nbt_write_stream_t* stream, double value) {
    u8* value_array = (u8*)&value;
    for (int i = 7; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_write_tag(__neko_nbt_write_stream_t* stream, neko_nbt_tag_t* tag, int write_name, int write_type) {

    if (write_type) {
        __neko_nbt_put_byte(stream, tag->type);
    }

    if (write_name && tag->type != NBT_TYPE_END) {
        __neko_nbt_put_int16(stream, tag->name_size);
        for (size_t i = 0; i < tag->name_size; i++) {
            __neko_nbt_put_byte(stream, tag->name[i]);
        }
    }

    switch (tag->type) {
        case NBT_TYPE_END: {
            break;
        }
        case NBT_TYPE_BYTE: {
            __neko_nbt_put_byte(stream, tag->tag_byte.value);
            break;
        }
        case NBT_TYPE_SHORT: {
            __neko_nbt_put_int16(stream, tag->tag_short.value);
            break;
        }
        case NBT_TYPE_INT: {
            __neko_nbt_put_int32(stream, tag->tag_int.value);
            break;
        }
        case NBT_TYPE_LONG: {
            __neko_nbt_put_int64(stream, tag->tag_long.value);
            break;
        }
        case NBT_TYPE_FLOAT: {
            __neko_nbt_put_float(stream, tag->tag_float.value);
            break;
        }
        case NBT_TYPE_DOUBLE: {
            __neko_nbt_put_double(stream, tag->tag_double.value);
            break;
        }
        case NBT_TYPE_BYTE_ARRAY: {
            __neko_nbt_put_int32(stream, tag->tag_byte_array.size);
            for (size_t i = 0; i < tag->tag_byte_array.size; i++) {
                __neko_nbt_put_byte(stream, tag->tag_byte_array.value[i]);
            }
            break;
        }
        case NBT_TYPE_STRING: {
            __neko_nbt_put_int16(stream, tag->tag_string.size);
            for (size_t i = 0; i < tag->tag_string.size; i++) {
                __neko_nbt_put_byte(stream, tag->tag_string.value[i]);
            }
            break;
        }
        case NBT_TYPE_LIST: {
            __neko_nbt_put_byte(stream, tag->tag_list.type);
            __neko_nbt_put_int32(stream, tag->tag_list.size);
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                __neko_nbt_write_tag(stream, tag->tag_list.value[i], 0, 0);
            }
            break;
        }
        case NBT_TYPE_COMPOUND: {
            for (size_t i = 0; i < tag->tag_compound.size; i++) {
                __neko_nbt_write_tag(stream, tag->tag_compound.value[i], 1, 1);
            }
            __neko_nbt_put_byte(stream, 0);  // 结束标识
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            __neko_nbt_put_int32(stream, tag->tag_int_array.size);
            for (size_t i = 0; i < tag->tag_int_array.size; i++) {
                __neko_nbt_put_int32(stream, tag->tag_int_array.value[i]);
            }
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            __neko_nbt_put_int32(stream, tag->tag_long_array.size);
            for (size_t i = 0; i < tag->tag_long_array.size; i++) {
                __neko_nbt_put_int64(stream, tag->tag_long_array.value[i]);
            }
            break;
        }
        default: {
            break;
        }
    }
}

u32 __neko_nbt_crc_table[256];

int __neko_nbt_crc_table_computed = 0;

void __neko_nbt_make_crc_table(void) {
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (u32)n;
        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        __neko_nbt_crc_table[n] = c;
    }
    __neko_nbt_crc_table_computed = 1;
}

static u32 __neko_nbt_update_crc(u32 crc, u8* buf, size_t len) {
    u32 c = crc ^ 0xffffffffL;
    size_t n;

    if (!__neko_nbt_crc_table_computed) {
        __neko_nbt_make_crc_table();
    }

    for (n = 0; n < len; n++) {
        c = __neko_nbt_crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c ^ 0xffffffffL;
}

void neko_nbt_write(neko_nbt_writer_t writer, neko_nbt_tag_t* tag, int write_flags) {

    __neko_nbt_write_stream_t write_stream;
    write_stream.buffer = (u8*)neko_malloc(NEKO_NBT_BUFFER_SIZE);
    write_stream.offset = 0;
    write_stream.size = 0;
    write_stream.alloc_size = NEKO_NBT_BUFFER_SIZE;

    __neko_nbt_write_tag(&write_stream, tag, 1, 1);

    size_t bytes_left = write_stream.size;
    size_t offset = 0;
    while (bytes_left > 0) {
        size_t bytes_written = writer.write(writer.userdata, write_stream.buffer + offset, bytes_left);
        offset += bytes_written;
        bytes_left -= bytes_written;
    }

    neko_free(write_stream.buffer);
}

static neko_nbt_tag_t* __neko_nbt_new_tag_base(void) {
    neko_nbt_tag_t* tag = (neko_nbt_tag_t*)neko_malloc(sizeof(neko_nbt_tag_t));
    tag->name = NULL;
    tag->name_size = 0;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_byte(s8 value) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_BYTE;
    tag->tag_byte.value = value;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_short(s16 value) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_SHORT;
    tag->tag_short.value = value;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_int(s32 value) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_INT;
    tag->tag_int.value = value;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_long(s64 value) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_LONG;
    tag->tag_long.value = value;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_float(float value) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_FLOAT;
    tag->tag_float.value = value;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_double(double value) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_DOUBLE;
    tag->tag_double.value = value;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_byte_array(s8* value, size_t size) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_BYTE_ARRAY;
    tag->tag_byte_array.size = size;
    tag->tag_byte_array.value = (s8*)neko_malloc(size);

    memcpy(tag->tag_byte_array.value, value, size);

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_string(const char* value, size_t size) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_STRING;
    tag->tag_string.size = size;
    tag->tag_string.value = (char*)neko_malloc(size + 1);

    memcpy(tag->tag_string.value, value, size);
    tag->tag_string.value[tag->tag_string.size] = '\0';

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_list(neko_nbt_tag_type_t type) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_LIST;
    tag->tag_list.type = type;
    tag->tag_list.size = 0;
    tag->tag_list.value = NULL;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_compound(void) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_COMPOUND;
    tag->tag_compound.size = 0;
    tag->tag_compound.value = NULL;

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_int_array(s32* value, size_t size) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_INT_ARRAY;
    tag->tag_int_array.size = size;
    tag->tag_int_array.value = (s32*)neko_malloc(size * sizeof(s32));

    memcpy(tag->tag_int_array.value, value, size * sizeof(s32));

    return tag;
}

neko_nbt_tag_t* neko_nbt_new_tag_long_array(s64* value, size_t size) {
    neko_nbt_tag_t* tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_LONG_ARRAY;
    tag->tag_long_array.size = size;
    tag->tag_long_array.value = (s64*)neko_malloc(size * sizeof(s64));

    memcpy(tag->tag_long_array.value, value, size * sizeof(s64));

    return tag;
}

void neko_nbt_set_tag_name(neko_nbt_tag_t* tag, const char* name, size_t size) {
    if (tag->name) {
        neko_free(tag->name);
    }
    tag->name_size = size;
    tag->name = (char*)neko_malloc(size + 1);
    memcpy(tag->name, name, size);
    tag->name[tag->name_size] = '\0';
}

void neko_nbt_tag_list_append(neko_nbt_tag_t* list, neko_nbt_tag_t* value) {
    list->tag_list.value = (neko_nbt_tag_t**)neko_realloc(list->tag_list.value, (list->tag_list.size + 1) * sizeof(neko_nbt_tag_t*));
    list->tag_list.value[list->tag_list.size] = value;
    list->tag_list.size++;
}

neko_nbt_tag_t* neko_nbt_tag_list_get(neko_nbt_tag_t* tag, size_t index) { return tag->tag_list.value[index]; }

void neko_nbt_tag_compound_append(neko_nbt_tag_t* compound, neko_nbt_tag_t* value) {
    compound->tag_compound.value = (neko_nbt_tag_t**)neko_realloc(compound->tag_compound.value, (compound->tag_compound.size + 1) * sizeof(neko_nbt_tag_t*));
    compound->tag_compound.value[compound->tag_compound.size] = value;
    compound->tag_compound.size++;
}

neko_nbt_tag_t* neko_nbt_tag_compound_get(neko_nbt_tag_t* tag, const char* key) {
    for (size_t i = 0; i < tag->tag_compound.size; i++) {
        neko_nbt_tag_t* compare_tag = tag->tag_compound.value[i];

        if (memcmp(compare_tag->name, key, compare_tag->name_size) == 0) {
            return compare_tag;
        }
    }

    return NULL;
}

void neko_nbt_free_tag(neko_nbt_tag_t* tag) {
    switch (tag->type) {
        case NBT_TYPE_BYTE_ARRAY: {
            neko_free(tag->tag_byte_array.value);
            break;
        }
        case NBT_TYPE_STRING: {
            neko_free(tag->tag_string.value);
            break;
        }
        case NBT_TYPE_LIST: {
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                neko_nbt_free_tag(tag->tag_list.value[i]);
            }
            neko_free(tag->tag_list.value);
            break;
        }
        case NBT_TYPE_COMPOUND: {
            for (size_t i = 0; i < tag->tag_compound.size; i++) {
                neko_nbt_free_tag(tag->tag_compound.value[i]);
            }
            neko_free(tag->tag_compound.value);
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            neko_free(tag->tag_int_array.value);
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            neko_free(tag->tag_long_array.value);
            break;
        }
        default: {
            break;
        }
    }

    if (tag->name) {
        neko_free(tag->name);
    }

    neko_free(tag);
}
