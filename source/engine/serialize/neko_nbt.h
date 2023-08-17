

#ifndef NEKO_NBT_H
#define NEKO_NBT_H

#include "engine/common/neko_types.h"

#define NEKO_NBT_BUFFER_SIZE 32768

typedef enum {
    NBT_TYPE_END,
    NBT_TYPE_BYTE,
    NBT_TYPE_SHORT,
    NBT_TYPE_INT,
    NBT_TYPE_LONG,
    NBT_TYPE_FLOAT,
    NBT_TYPE_DOUBLE,
    NBT_TYPE_BYTE_ARRAY,
    NBT_TYPE_STRING,
    NBT_TYPE_LIST,
    NBT_TYPE_COMPOUND,
    NBT_TYPE_INT_ARRAY,
    NBT_TYPE_LONG_ARRAY,
    NBT_NO_OVERRIDE
} neko_nbt_tag_type_t;

typedef struct neko_nbt_tag_t neko_nbt_tag_t;

struct neko_nbt_tag_t {

    neko_nbt_tag_type_t type;

    char* name;
    size_t name_size;

    union {
        struct {
            s8 value;
        } tag_byte;
        struct {
            s16 value;
        } tag_short;
        struct {
            s32 value;
        } tag_int;
        struct {
            s64 value;
        } tag_long;
        struct {
            float value;
        } tag_float;
        struct {
            double value;
        } tag_double;
        struct {
            s8* value;
            size_t size;
        } tag_byte_array;
        struct {
            char* value;
            size_t size;
        } tag_string;
        struct {
            neko_nbt_tag_t** value;
            neko_nbt_tag_type_t type;
            size_t size;
        } tag_list;
        struct {
            neko_nbt_tag_t** value;
            size_t size;
        } tag_compound;
        struct {
            s32* value;
            size_t size;
        } tag_int_array;
        struct {
            s64* value;
            size_t size;
        } tag_long_array;
    };
};

typedef struct {
    size_t (*read)(void* userdata, u8* data, size_t size);
    void* userdata;
} neko_nbt_reader_t;

typedef struct {
    size_t (*write)(void* userdata, u8* data, size_t size);
    void* userdata;
} neko_nbt_writer_t;

typedef enum {
    NBT_PARSE_FLAG_USE_RAW = 1,
} neko_nbt_parse_flags_t;

typedef enum { NBT_WRITE_FLAG_USE_RAW = 1 } neko_nbt_write_flags_t;

neko_nbt_tag_t* neko_nbt_parse(neko_nbt_reader_t reader, int parse_flags);
void neko_nbt_write(neko_nbt_writer_t writer, neko_nbt_tag_t* tag, int write_flags);

neko_nbt_tag_t* neko_nbt_new_tag_byte(s8 value);
neko_nbt_tag_t* neko_nbt_new_tag_short(s16 value);
neko_nbt_tag_t* neko_nbt_new_tag_int(s32 value);
neko_nbt_tag_t* neko_nbt_new_tag_long(s64 value);
neko_nbt_tag_t* neko_nbt_new_tag_float(float value);
neko_nbt_tag_t* neko_nbt_new_tag_double(double value);
neko_nbt_tag_t* neko_nbt_new_tag_byte_array(s8* value, size_t size);
neko_nbt_tag_t* neko_nbt_new_tag_string(const char* value, size_t size);
neko_nbt_tag_t* neko_nbt_new_tag_list(neko_nbt_tag_type_t type);
neko_nbt_tag_t* neko_nbt_new_tag_compound(void);
neko_nbt_tag_t* neko_nbt_new_tag_int_array(s32* value, size_t size);
neko_nbt_tag_t* neko_nbt_new_tag_long_array(s64* value, size_t size);

void neko_nbt_set_tag_name(neko_nbt_tag_t* tag, const char* name, size_t size);

void neko_nbt_tag_list_append(neko_nbt_tag_t* list, neko_nbt_tag_t* value);
neko_nbt_tag_t* neko_nbt_tag_list_get(neko_nbt_tag_t* tag, size_t index);
void neko_nbt_tag_compound_append(neko_nbt_tag_t* compound, neko_nbt_tag_t* value);
neko_nbt_tag_t* neko_nbt_tag_compound_get(neko_nbt_tag_t* tag, const char* key);

void neko_nbt_free_tag(neko_nbt_tag_t* tag);

#endif
