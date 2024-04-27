

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#include "neko.h"

NEKO_API_DECL unsigned char* neko_base64_encode(unsigned char* str);
NEKO_API_DECL unsigned char* neko_base64_decode(unsigned char* code);

#ifndef STRPOOL_U32
#define STRPOOL_U32 unsigned int
#endif
#ifndef STRPOOL_U64
#define STRPOOL_U64 unsigned long long
#endif

typedef struct strpool_t strpool_t;

typedef struct strpool_config_t {
    void* memctx;
    int ignore_case;
    int counter_bits;
    int index_bits;
    int entry_capacity;
    int block_capacity;
    int block_size;
    int min_length;
} strpool_config_t;

extern strpool_config_t const strpool_default_config;

void strpool_init(strpool_t* pool, strpool_config_t const* config);
void strpool_term(strpool_t* pool);

void strpool_defrag(strpool_t* pool);

STRPOOL_U64 strpool_inject(strpool_t* pool, char const* string, int length);
void strpool_discard(strpool_t* pool, STRPOOL_U64 handle);

int strpool_incref(strpool_t* pool, STRPOOL_U64 handle);
int strpool_decref(strpool_t* pool, STRPOOL_U64 handle);
int strpool_getref(strpool_t* pool, STRPOOL_U64 handle);

int strpool_isvalid(strpool_t const* pool, STRPOOL_U64 handle);

char const* strpool_cstr(strpool_t const* pool, STRPOOL_U64 handle);
int strpool_length(strpool_t const* pool, STRPOOL_U64 handle);

char* strpool_collate(strpool_t const* pool, int* count);
void strpool_free_collated(strpool_t const* pool, char* collated_ptr);

struct strpool_internal_hash_slot_t;
struct strpool_internal_entry_t;
struct strpool_internal_handle_t;
struct strpool_internal_block_t;

struct strpool_t {
    void* memctx;
    int ignore_case;
    int counter_shift;
    STRPOOL_U64 counter_mask;
    STRPOOL_U64 index_mask;

    int initial_entry_capacity;
    int initial_block_capacity;
    int block_size;
    int min_data_size;

    struct strpool_internal_hash_slot_t* hash_table;
    int hash_capacity;

    struct strpool_internal_entry_t* entries;
    int entry_capacity;
    int entry_count;

    struct strpool_internal_handle_t* handles;
    int handle_capacity;
    int handle_count;
    int handle_freelist_head;
    int handle_freelist_tail;

    struct strpool_internal_block_t* blocks;
    int block_capacity;
    int block_count;
    int current_block;
};

#endif
