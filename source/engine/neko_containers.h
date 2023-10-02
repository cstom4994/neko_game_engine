
#ifndef NEKO_CONTAINERS_H
#define NEKO_CONTAINERS_H

#include "engine/neko.h"

/*========================
// Byte Buffer
========================*/

/** @defgroup neko_byte_buffer Byte Buffer
 *  @ingroup neko_containers
 *  Byte Buffer
 */

#define NEKO_BYTE_BUFFER_DEFAULT_CAPCITY 1024

/** @addtogroup neko_byte_buffer
 */
typedef struct neko_byte_buffer_t {
    uint8_t* data;      // Buffer that actually holds all relevant byte data
    uint32_t size;      // Current size of the stored buffer data
    uint32_t position;  // Current read/write position in the buffer
    uint32_t capacity;  // Current max capacity for the buffer
} neko_byte_buffer_t;

// Generic "write" function for a byte buffer
#define neko_byte_buffer_write(__BB, __T, __VAL)              \
    do {                                                      \
        neko_byte_buffer_t* __BUFFER = __BB;                  \
        usize __SZ = sizeof(__T);                             \
        usize __TWS = __BUFFER->position + __SZ;              \
        if (__TWS >= (usize)__BUFFER->capacity) {             \
            usize __CAP = __BUFFER->capacity * 2;             \
            while (__CAP < __TWS) {                           \
                __CAP *= 2;                                   \
            }                                                 \
            neko_byte_buffer_resize(__BUFFER, __CAP);         \
        }                                                     \
        *(__T*)(__BUFFER->data + __BUFFER->position) = __VAL; \
        __BUFFER->position += (uint32_t)__SZ;                 \
        __BUFFER->size += (uint32_t)__SZ;                     \
    } while (0)

// Generic "read" function
#define neko_byte_buffer_read(__BUFFER, __T, __VAL_P)  \
    do {                                               \
        __T* __V = (__T*)(__VAL_P);                    \
        neko_byte_buffer_t* __BB = (__BUFFER);         \
        *(__V) = *(__T*)(__BB->data + __BB->position); \
        __BB->position += sizeof(__T);                 \
    } while (0)

// Defines variable and sets value from buffer in place
// Use to construct a new variable
#define neko_byte_buffer_readc(__BUFFER, __T, __NAME) \
    __T __NAME = neko_default_val();                  \
    neko_byte_buffer_read((__BUFFER), __T, &__NAME);

#define neko_byte_buffer_read_bulkc(__BUFFER, __T, __NAME, __SZ) \
    __T __NAME = neko_default_val();                             \
    __T* neko_macro_cat(__NAME, __LINE__) = &(__NAME);           \
    neko_byte_buffer_read_bulk(__BUFFER, (void**)&neko_macro_cat(__NAME, __LINE__), __SZ);

NEKO_API_DECL void neko_byte_buffer_init(neko_byte_buffer_t* buffer);
NEKO_API_DECL neko_byte_buffer_t neko_byte_buffer_new();
NEKO_API_DECL void neko_byte_buffer_free(neko_byte_buffer_t* buffer);
NEKO_API_DECL void neko_byte_buffer_clear(neko_byte_buffer_t* buffer);
NEKO_API_DECL bool neko_byte_buffer_empty(neko_byte_buffer_t* buffer);
NEKO_API_DECL size_t neko_byte_buffer_size(neko_byte_buffer_t* buffer);
NEKO_API_DECL void neko_byte_buffer_resize(neko_byte_buffer_t* buffer, size_t sz);
NEKO_API_DECL void neko_byte_buffer_copy_contents(neko_byte_buffer_t* dst, neko_byte_buffer_t* src);
NEKO_API_DECL void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t* buffer);
NEKO_API_DECL void neko_byte_buffer_seek_to_end(neko_byte_buffer_t* buffer);
NEKO_API_DECL void neko_byte_buffer_advance_position(neko_byte_buffer_t* buffer, size_t sz);
NEKO_API_DECL void neko_byte_buffer_write_str(neko_byte_buffer_t* buffer, const char* str);  // Expects a null terminated string
NEKO_API_DECL void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str);         // Expects an allocated string
NEKO_API_DECL void neko_byte_buffer_write_bulk(neko_byte_buffer_t* buffer, void* src, size_t sz);
NEKO_API_DECL void neko_byte_buffer_read_bulk(neko_byte_buffer_t* buffer, void** dst, size_t sz);
NEKO_API_DECL neko_result neko_byte_buffer_write_to_file(neko_byte_buffer_t* buffer, const char* output_path);  // Assumes that the output directory exists
NEKO_API_DECL neko_result neko_byte_buffer_read_from_file(neko_byte_buffer_t* buffer, const char* file_path);   // Assumes an allocated byte buffer
NEKO_API_DECL void neko_byte_buffer_memset(neko_byte_buffer_t* buffer, uint8_t val);

/*===================================
// Dynamic Array
===================================*/

/** @defgroup neko_dyn_array Dynamic Array
 *  @ingroup neko_containers
 *  Dynamic Array
 */

/** @addtogroup neko_dyn_array
 */
typedef struct neko_dyn_array {
    int32_t size;
    int32_t capacity;
} neko_dyn_array;

#define neko_dyn_array_head(__ARR) ((neko_dyn_array*)((uint8_t*)(__ARR) - sizeof(neko_dyn_array)))

#define neko_dyn_array_size(__ARR) (__ARR == NULL ? 0 : neko_dyn_array_head((__ARR))->size)

#define neko_dyn_array_capacity(__ARR) (__ARR == NULL ? 0 : neko_dyn_array_head((__ARR))->capacity)

#define neko_dyn_array_full(__ARR) ((neko_dyn_array_size((__ARR)) == neko_dyn_array_capacity((__ARR))))

#define neko_dyn_array_byte_size(__ARR) (neko_dyn_array_size((__ARR)) * sizeof(*__ARR))

NEKO_API_DECL void* neko_dyn_array_resize_impl(void* arr, size_t sz, size_t amount);

#define neko_dyn_array_need_grow(__ARR, __N) ((__ARR) == 0 || neko_dyn_array_size(__ARR) + (__N) >= neko_dyn_array_capacity(__ARR))

#define neko_dyn_array_grow(__ARR) neko_dyn_array_resize_impl((__ARR), sizeof(*(__ARR)), neko_dyn_array_capacity(__ARR) ? neko_dyn_array_capacity(__ARR) * 2 : 1)

#define neko_dyn_array_grow_size(__ARR, __SZ) neko_dyn_array_resize_impl((__ARR), (__SZ), neko_dyn_array_capacity(__ARR) ? neko_dyn_array_capacity(__ARR) * 2 : 1)

NEKO_API_DECL void** neko_dyn_array_init(void** arr, size_t val_len);

NEKO_API_DECL void neko_dyn_array_push_data(void** arr, void* val, size_t val_len);

neko_force_inline void neko_dyn_array_set_data_i(void** arr, void* val, size_t val_len, uint32_t offset) { memcpy(((char*)(*arr)) + offset * val_len, val, val_len); }

#define neko_dyn_array_push(__ARR, __ARRVAL)                               \
    do {                                                                   \
        neko_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));           \
        if (!(__ARR) || ((__ARR) && neko_dyn_array_need_grow(__ARR, 1))) { \
            *((void**)&(__ARR)) = neko_dyn_array_grow(__ARR);              \
        }                                                                  \
        (__ARR)[neko_dyn_array_size(__ARR)] = (__ARRVAL);                  \
        neko_dyn_array_head(__ARR)->size++;                                \
    } while (0)

#define neko_dyn_array_reserve(__ARR, __AMOUNT)                                                \
    do {                                                                                       \
        if ((!__ARR)) neko_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));                 \
        if ((!__ARR) || (size_t)__AMOUNT > neko_dyn_array_capacity(__ARR)) {                   \
            *((void**)&(__ARR)) = neko_dyn_array_resize_impl(__ARR, sizeof(*__ARR), __AMOUNT); \
        }                                                                                      \
    } while (0)

#define neko_dyn_array_empty(__ARR) (neko_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR))), (neko_dyn_array_size(__ARR) == 0))

#define neko_dyn_array_pop(__ARR)                    \
    do {                                             \
        if (__ARR && !neko_dyn_array_empty(__ARR)) { \
            neko_dyn_array_head(__ARR)->size -= 1;   \
        }                                            \
    } while (0)

#define neko_dyn_array_back(__ARR) *(__ARR + (neko_dyn_array_size(__ARR) ? neko_dyn_array_size(__ARR) - 1 : 0))

#define neko_dyn_array_for(__ARR, __T, __IT_NAME) for (__T* __IT_NAME = __ARR; __IT_NAME != neko_dyn_array_back(__ARR); ++__IT_NAME)

#define neko_dyn_array_new(__T) ((__T*)neko_dyn_array_resize_impl(NULL, sizeof(__T), 0))

#define neko_dyn_array_clear(__ARR)               \
    do {                                          \
        if (__ARR) {                              \
            neko_dyn_array_head(__ARR)->size = 0; \
        }                                         \
    } while (0)

#define neko_dyn_array(__T) __T*

#define neko_dyn_array_free(__ARR)                 \
    do {                                           \
        if (__ARR) {                               \
            neko_free(neko_dyn_array_head(__ARR)); \
            (__ARR) = NULL;                        \
        }                                          \
    } while (0)

/*===================================
// Hash Table
===================================*/

/*
    If using struct for keys, requires struct to be word-aligned.
*/

#define NEKO_HASH_TABLE_HASH_SEED 0x31415296
#define NEKO_HASH_TABLE_INVALID_INDEX UINT32_MAX

typedef enum neko_hash_table_entry_state { NEKO_HASH_TABLE_ENTRY_INACTIVE = 0x00, NEKO_HASH_TABLE_ENTRY_ACTIVE = 0x01 } neko_hash_table_entry_state;

#define __neko_hash_table_entry(__HMK, __HMV) \
    struct {                                  \
        __HMK key;                            \
        __HMV val;                            \
        neko_hash_table_entry_state state;    \
    }

#define neko_hash_table(__HMK, __HMV)                 \
    struct {                                          \
        __neko_hash_table_entry(__HMK, __HMV) * data; \
        __HMK tmp_key;                                \
        __HMV tmp_val;                                \
        size_t stride;                                \
        size_t klpvl;                                 \
        size_t tmp_idx;                               \
    }*

// Need a way to create a temporary key so I can take the address of it

#define neko_hash_table_new(__K, __V) NULL

NEKO_API_DECL void __neko_hash_table_init_impl(void** ht, size_t sz);

#define neko_hash_table_init(__HT, __K, __V)                                                 \
    do {                                                                                     \
        size_t entry_sz = sizeof(*__HT->data);                                               \
        size_t ht_sz = sizeof(*__HT);                                                        \
        __neko_hash_table_init_impl((void**)&(__HT), ht_sz);                                 \
        memset((__HT), 0, ht_sz);                                                            \
        neko_dyn_array_reserve(__HT->data, 2);                                               \
        __HT->data[0].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                \
        __HT->data[1].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                \
        uintptr_t d0 = (uintptr_t) & ((__HT)->data[0]);                                      \
        uintptr_t d1 = (uintptr_t) & ((__HT)->data[1]);                                      \
        ptrdiff_t diff = (d1 - d0);                                                          \
        ptrdiff_t klpvl = (uintptr_t) & (__HT->data[0].state) - (uintptr_t)(&__HT->data[0]); \
        (__HT)->stride = (size_t)(diff);                                                     \
        (__HT)->klpvl = (size_t)(klpvl);                                                     \
    } while (0)

#define neko_hash_table_reserve(_HT, _KT, _VT, _CT) \
    do {                                            \
        if ((_HT) == NULL) {                        \
            neko_hash_table_init((_HT), _KT, _VT);  \
        }                                           \
        neko_dyn_array_reserve((_HT)->data, _CT);   \
    } while (0)

// ((__HT) != NULL ? (__HT)->size : 0) // neko_dyn_array_size((__HT)->data) : 0)
#define neko_hash_table_size(__HT) ((__HT) != NULL ? neko_dyn_array_size((__HT)->data) : 0)

#define neko_hash_table_capacity(__HT) ((__HT) != NULL ? neko_dyn_array_capacity((__HT)->data) : 0)

#define neko_hash_table_load_factor(__HT) (neko_hash_table_capacity(__HT) ? (float)(neko_hash_table_size(__HT)) / (float)(neko_hash_table_capacity(__HT)) : 0.f)

#define neko_hash_table_grow(__HT, __C) ((__HT)->data = neko_dyn_array_resize_impl((__HT)->data, sizeof(*((__HT)->data)), (__C)))

#define neko_hash_table_empty(__HT) ((__HT) != NULL ? neko_dyn_array_size((__HT)->data) == 0 : true)

#define neko_hash_table_clear(__HT)                                                \
    do {                                                                           \
        if ((__HT) != NULL) {                                                      \
            uint32_t capacity = neko_dyn_array_capacity((__HT)->data);             \
            for (uint32_t i = 0; i < capacity; ++i) {                              \
                (__HT)->data[i].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;            \
            }                                                                      \
            /*memset((__HT)->data, 0, neko_dyn_array_capacity((__HT)->data) * );*/ \
            neko_dyn_array_clear((__HT)->data);                                    \
        }                                                                          \
    } while (0)

#define neko_hash_table_free(__HT)             \
    do {                                       \
        if ((__HT) != NULL) {                  \
            neko_dyn_array_free((__HT)->data); \
            (__HT)->data = NULL;               \
            neko_free(__HT);                   \
            (__HT) = NULL;                     \
        }                                      \
    } while (0)

// Find available slot to insert k/v pair into
#define neko_hash_table_insert(__HT, __HMK, __HMV)                                                                                                                                                    \
    do {                                                                                                                                                                                              \
        /* Check for null hash table, init if necessary */                                                                                                                                            \
        if ((__HT) == NULL) {                                                                                                                                                                         \
            neko_hash_table_init((__HT), (__HMK), (__HMV));                                                                                                                                           \
        }                                                                                                                                                                                             \
                                                                                                                                                                                                      \
        /* Grow table if necessary */                                                                                                                                                                 \
        uint32_t __CAP = neko_hash_table_capacity(__HT);                                                                                                                                              \
        float __LF = neko_hash_table_load_factor(__HT);                                                                                                                                               \
        if (__LF >= 0.5f || !__CAP) {                                                                                                                                                                 \
            uint32_t NEW_CAP = __CAP ? __CAP * 2 : 2;                                                                                                                                                 \
            size_t ENTRY_SZ = sizeof((__HT)->tmp_key) + sizeof((__HT)->tmp_val) + sizeof(neko_hash_table_entry_state);                                                                                \
            neko_dyn_array_reserve((__HT)->data, NEW_CAP);                                                                                                                                            \
            /**((void **)&(__HT->data)) = neko_dyn_array_resize_impl(__HT->data, ENTRY_SZ, NEW_CAP);*/                                                                                                \
            /* Iterate through data and set state to null, from __CAP -> __CAP * 2 */                                                                                                                 \
            /* Memset here instead */                                                                                                                                                                 \
            for (uint32_t __I = __CAP; __I < NEW_CAP; ++__I) {                                                                                                                                        \
                (__HT)->data[__I].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                                                                                                             \
            }                                                                                                                                                                                         \
            __CAP = neko_hash_table_capacity(__HT);                                                                                                                                                   \
        }                                                                                                                                                                                             \
                                                                                                                                                                                                      \
        /* Get hash of key */                                                                                                                                                                         \
        (__HT)->tmp_key = (__HMK);                                                                                                                                                                    \
        size_t __HSH = neko_hash_bytes((void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), NEKO_HASH_TABLE_HASH_SEED);                                                                                \
        size_t __HSH_IDX = __HSH % __CAP;                                                                                                                                                             \
        (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;                                                                                                                                                \
        uint32_t c = 0;                                                                                                                                                                               \
                                                                                                                                                                                                      \
        /* Find valid idx and place data */                                                                                                                                                           \
        while (c < __CAP && __HSH != neko_hash_bytes((void*)&(__HT)->tmp_key, sizeof((__HT)->tmp_key), NEKO_HASH_TABLE_HASH_SEED) && (__HT)->data[__HSH_IDX].state == NEKO_HASH_TABLE_ENTRY_ACTIVE) { \
            __HSH_IDX = ((__HSH_IDX + 1) % __CAP);                                                                                                                                                    \
            (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;                                                                                                                                            \
            ++c;                                                                                                                                                                                      \
        }                                                                                                                                                                                             \
        (__HT)->data[__HSH_IDX].key = (__HMK);                                                                                                                                                        \
        (__HT)->data[__HSH_IDX].val = (__HMV);                                                                                                                                                        \
        (__HT)->data[__HSH_IDX].state = NEKO_HASH_TABLE_ENTRY_ACTIVE;                                                                                                                                 \
        neko_dyn_array_head((__HT)->data)->size++;                                                                                                                                                    \
    } while (0)

// Need size difference between two entries
// Need size of key + val

neko_force_inline uint32_t neko_hash_table_get_key_index_func(void** data, void* key, size_t key_len, size_t val_len, size_t stride, size_t klpvl) {
    if (!data || !key) return NEKO_HASH_TABLE_INVALID_INDEX;

    // Need a better way to handle this. Can't do it like this anymore.
    // Need to fix this. Seriously messing me up.
    uint32_t capacity = neko_dyn_array_capacity(*data);
    size_t idx = (size_t)NEKO_HASH_TABLE_INVALID_INDEX;
    size_t hash = (size_t)neko_hash_bytes(key, key_len, NEKO_HASH_TABLE_HASH_SEED);
    size_t hash_idx = (hash % capacity);

    // Iterate through data
    for (size_t i = hash_idx, c = 0; c < capacity; ++c, i = ((i + 1) % capacity)) {
        size_t offset = (i * stride);
        void* k = ((char*)(*data) + (offset));
        size_t kh = neko_hash_bytes(k, key_len, NEKO_HASH_TABLE_HASH_SEED);
        bool comp = neko_compare_bytes(k, key, key_len);
        neko_hash_table_entry_state state = *(neko_hash_table_entry_state*)((char*)(*data) + offset + (klpvl));
        if (comp && hash == kh && state == NEKO_HASH_TABLE_ENTRY_ACTIVE) {
            idx = i;
            break;
        }
    }
    return (uint32_t)idx;
}

// Get key at index
#define neko_hash_table_getk(__HT, __I) (((__HT))->data[(__I)].key)

// Get val at index
#define neko_hash_table_geti(__HT, __I) ((__HT)->data[(__I)].val)

// Could search for the index in the macro instead now. Does this help me?
#define neko_hash_table_get(__HT, __HTK)                                                                                                                                                             \
    ((__HT)->tmp_key = (__HTK), (neko_hash_table_geti((__HT), neko_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), \
                                                                                                 (__HT)->stride, (__HT)->klpvl))))

#define neko_hash_table_getp(__HT, __HTK)                                                                                                                                                       \
    ((__HT)->tmp_key = (__HTK),                                                                                                                                                                 \
     ((__HT)->tmp_idx = (uint32_t)neko_hash_table_get_key_index_func((void**)&(__HT->data), (void*)&(__HT->tmp_key), sizeof(__HT->tmp_key), sizeof(__HT->tmp_val), __HT->stride, __HT->klpvl)), \
     ((__HT)->tmp_idx != NEKO_HASH_TABLE_INVALID_INDEX ? &neko_hash_table_geti((__HT), (__HT)->tmp_idx) : NULL))

#define _neko_hash_table_key_exists_internal(__HT, __HTK) \
    ((__HT)->tmp_key = (__HTK),                           \
     (neko_hash_table_get_key_index_func((void**)&(__HT->data), (void*)&(__HT->tmp_key), sizeof(__HT->tmp_key), sizeof(__HT->tmp_val), __HT->stride, __HT->klpvl) != NEKO_HASH_TABLE_INVALID_INDEX))

// uint32_t neko_hash_table_get_key_index_func(void** data, void* key, size_t key_len, size_t val_len, size_t stride, size_t klpvl)

#define neko_hash_table_exists(__HT, __HTK) (__HT && _neko_hash_table_key_exists_internal((__HT), (__HTK)))

#define neko_hash_table_key_exists(__HT, __HTK) (neko_hash_table_exists((__HT), (__HTK)))

#define neko_hash_table_erase(__HT, __HTK)                                                                                                                                                          \
    do {                                                                                                                                                                                            \
        if ((__HT)) {                                                                                                                                                                               \
            /* Get idx for key */                                                                                                                                                                   \
            (__HT)->tmp_key = (__HTK);                                                                                                                                                              \
            uint32_t __IDX = neko_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__HT)->stride, (__HT)->klpvl); \
            if (__IDX != NEKO_HASH_TABLE_INVALID_INDEX) {                                                                                                                                           \
                (__HT)->data[__IDX].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                                                                                                         \
                if (neko_dyn_array_head((__HT)->data)->size) neko_dyn_array_head((__HT)->data)->size--;                                                                                             \
            }                                                                                                                                                                                       \
        }                                                                                                                                                                                           \
    } while (0)

/*===== Hash Table Iterator ====*/

typedef uint32_t neko_hash_table_iter;

neko_force_inline uint32_t __neko_find_first_valid_iterator(void* data, size_t key_len, size_t val_len, uint32_t idx, size_t stride, size_t klpvl) {
    uint32_t it = (uint32_t)idx;
    for (; it < (uint32_t)neko_dyn_array_capacity(data); ++it) {
        size_t offset = (it * stride);
        neko_hash_table_entry_state state = *(neko_hash_table_entry_state*)((uint8_t*)data + offset + (klpvl));
        if (state == NEKO_HASH_TABLE_ENTRY_ACTIVE) {
            break;
        }
    }
    return it;
}

/* Find first valid iterator idx */
#define neko_hash_table_iter_new(__HT) (__HT ? __neko_find_first_valid_iterator((__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), 0, (__HT)->stride, (__HT)->klpvl) : 0)

#define neko_hash_table_iter_valid(__HT, __IT) ((__IT) < neko_hash_table_capacity((__HT)))

// Have to be able to do this for hash table...
neko_force_inline void __neko_hash_table_iter_advance_func(void** data, size_t key_len, size_t val_len, uint32_t* it, size_t stride, size_t klpvl) {
    (*it)++;
    for (; *it < (uint32_t)neko_dyn_array_capacity(*data); ++*it) {
        size_t offset = (size_t)(*it * stride);
        neko_hash_table_entry_state state = *(neko_hash_table_entry_state*)((uint8_t*)*data + offset + (klpvl));
        if (state == NEKO_HASH_TABLE_ENTRY_ACTIVE) {
            break;
        }
    }
}

#define neko_hash_table_find_valid_iter(__HT, __IT) \
    ((__IT) = __neko_find_first_valid_iterator((void**)&(__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__IT), (__HT)->stride, (__HT)->klpvl))

#define neko_hash_table_iter_advance(__HT, __IT) (__neko_hash_table_iter_advance_func((void**)&(__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), &(__IT), (__HT)->stride, (__HT)->klpvl))

#define neko_hash_table_iter_get(__HT, __IT) neko_hash_table_geti(__HT, __IT)

#define neko_hash_table_iter_getp(__HT, __IT) (&(neko_hash_table_geti(__HT, __IT)))

#define neko_hash_table_iter_getk(__HT, __IT) (neko_hash_table_getk(__HT, __IT))

#define neko_hash_table_iter_getkp(__HT, __IT) (&(neko_hash_table_getk(__HT, __IT)))

/*===================================
// Slot Array
===================================*/

#define NEKO_SLOT_ARRAY_INVALID_HANDLE UINT32_MAX

#define neko_slot_array_handle_valid(__SA, __ID) (__ID < neko_dyn_array_size((__SA)->indices) && (__SA)->indices[__ID] != NEKO_SLOT_ARRAY_INVALID_HANDLE)

typedef struct __neko_slot_array_dummy_header {
    neko_dyn_array(uint32_t) indices;
    neko_dyn_array(uint32_t) data;
} __neko_slot_array_dummy_header;

#define neko_slot_array(__T)              \
    struct {                              \
        neko_dyn_array(uint32_t) indices; \
        neko_dyn_array(__T) data;         \
        __T tmp;                          \
    }*

#define neko_slot_array_new(__T) NULL

neko_force_inline uint32_t __neko_slot_array_find_next_available_index(neko_dyn_array(uint32_t) indices) {
    uint32_t idx = NEKO_SLOT_ARRAY_INVALID_HANDLE;
    for (uint32_t i = 0; i < (uint32_t)neko_dyn_array_size(indices); ++i) {
        uint32_t handle = indices[i];
        if (handle == NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            idx = i;
            break;
        }
    }
    if (idx == NEKO_SLOT_ARRAY_INVALID_HANDLE) {
        idx = neko_dyn_array_size(indices);
    }

    return idx;
}

NEKO_API_DECL void** neko_slot_array_init(void** sa, size_t sz);

#define neko_slot_array_init_all(__SA) \
    (neko_slot_array_init((void**)&(__SA), sizeof(*(__SA))), neko_dyn_array_init((void**)&((__SA)->indices), sizeof(uint32_t)), neko_dyn_array_init((void**)&((__SA)->data), sizeof((__SA)->tmp)))

neko_force_inline uint32_t neko_slot_array_insert_func(void** indices, void** data, void* val, size_t val_len, uint32_t* ip) {
    // Find next available index
    u32 idx = __neko_slot_array_find_next_available_index((uint32_t*)*indices);

    if (idx == neko_dyn_array_size(*indices)) {
        uint32_t v = 0;
        neko_dyn_array_push_data(indices, &v, sizeof(uint32_t));
        idx = neko_dyn_array_size(*indices) - 1;
    }

    // Push data to array
    neko_dyn_array_push_data(data, val, val_len);

    // Set data in indices
    uint32_t bi = neko_dyn_array_size(*data) - 1;
    neko_dyn_array_set_data_i(indices, &bi, sizeof(uint32_t), idx);

    if (ip) {
        *ip = idx;
    }

    return idx;
}

#define neko_slot_array_reserve(__SA, __NUM)            \
    do {                                                \
        neko_slot_array_init_all(__SA);                 \
        neko_dyn_array_reserve((__SA)->data, __NUM);    \
        neko_dyn_array_reserve((__SA)->indices, __NUM); \
    } while (0)

#define neko_slot_array_insert(__SA, __VAL) \
    (neko_slot_array_init_all(__SA), (__SA)->tmp = (__VAL), neko_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), (void*)&((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define neko_slot_array_insert_hp(__SA, __VAL, __hp) \
    (neko_slot_array_init_all(__SA), (__SA)->tmp = (__VAL), neko_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), (__hp)))

#define neko_slot_array_insert_no_init(__SA, __VAL) \
    ((__SA)->tmp = (__VAL), neko_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define neko_slot_array_size(__SA) ((__SA) == NULL ? 0 : neko_dyn_array_size((__SA)->data))

#define neko_slot_array_empty(__SA) (neko_slot_array_size(__SA) == 0)

#define neko_slot_array_clear(__SA)                \
    do {                                           \
        if ((__SA) != NULL) {                      \
            neko_dyn_array_clear((__SA)->data);    \
            neko_dyn_array_clear((__SA)->indices); \
        }                                          \
    } while (0)

#define neko_slot_array_exists(__SA, __SID) ((__SA) && (__SID) < (uint32_t)neko_dyn_array_size((__SA)->indices) && (__SA)->indices[__SID] != NEKO_SLOT_ARRAY_INVALID_HANDLE)

#define neko_slot_array_get(__SA, __SID) ((__SA)->data[(__SA)->indices[(__SID) % neko_dyn_array_size(((__SA)->indices))]])

#define neko_slot_array_getp(__SA, __SID) (&(neko_slot_array_get(__SA, (__SID))))

#define neko_slot_array_free(__SA)                \
    do {                                          \
        if ((__SA) != NULL) {                     \
            neko_dyn_array_free((__SA)->data);    \
            neko_dyn_array_free((__SA)->indices); \
            (__SA)->indices = NULL;               \
            (__SA)->data = NULL;                  \
            neko_free((__SA));                    \
            (__SA) = NULL;                        \
        }                                         \
    } while (0)

#define neko_slot_array_erase(__SA, __id)                                                       \
    do {                                                                                        \
        uint32_t __H0 = (__id) /*% neko_dyn_array_size((__SA)->indices)*/;                      \
        if (neko_slot_array_size(__SA) == 1) {                                                  \
            neko_slot_array_clear(__SA);                                                        \
        } else if (!neko_slot_array_handle_valid(__SA, __H0)) {                                 \
            neko_println("Warning: Attempting to erase invalid slot array handle (%zu)", __H0); \
        } else {                                                                                \
            uint32_t __OG_DATA_IDX = (__SA)->indices[__H0];                                     \
            /* Iterate through handles until last index of data found */                        \
            uint32_t __H = 0;                                                                   \
            for (uint32_t __I = 0; __I < neko_dyn_array_size((__SA)->indices); ++__I) {         \
                if ((__SA)->indices[__I] == neko_dyn_array_size((__SA)->data) - 1) {            \
                    __H = __I;                                                                  \
                    break;                                                                      \
                }                                                                               \
            }                                                                                   \
                                                                                                \
            /* Swap and pop data */                                                             \
            (__SA)->data[__OG_DATA_IDX] = neko_dyn_array_back((__SA)->data);                    \
            neko_dyn_array_pop((__SA)->data);                                                   \
                                                                                                \
            /* Point new handle, Set og handle to invalid */                                    \
            (__SA)->indices[__H] = __OG_DATA_IDX;                                               \
            (__SA)->indices[__H0] = NEKO_SLOT_ARRAY_INVALID_HANDLE;                             \
        }                                                                                       \
    } while (0)

/*=== Slot Array Iterator ===*/

// Slot array iterator new
typedef uint32_t neko_slot_array_iter;

#define neko_slot_array_iter_valid(__SA, __IT) (__SA && neko_slot_array_exists(__SA, __IT))

neko_force_inline void _neko_slot_array_iter_advance_func(neko_dyn_array(uint32_t) indices, uint32_t* it) {
    if (!indices) {
        *it = NEKO_SLOT_ARRAY_INVALID_HANDLE;
        return;
    }

    (*it)++;
    for (; *it < (uint32_t)neko_dyn_array_size(indices); ++*it) {
        if (indices[*it] != NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            break;
        }
    }
}

neko_force_inline uint32_t _neko_slot_array_iter_find_first_valid_index(neko_dyn_array(uint32_t) indices) {
    if (!indices) return NEKO_SLOT_ARRAY_INVALID_HANDLE;

    for (uint32_t i = 0; i < (uint32_t)neko_dyn_array_size(indices); ++i) {
        if (indices[i] != NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            return i;
        }
    }
    return NEKO_SLOT_ARRAY_INVALID_HANDLE;
}

#define neko_slot_array_iter_new(__SA) (_neko_slot_array_iter_find_first_valid_index((__SA) ? (__SA)->indices : 0))

#define neko_slot_array_iter_advance(__SA, __IT) _neko_slot_array_iter_advance_func((__SA) ? (__SA)->indices : NULL, &(__IT))

#define neko_slot_array_iter_get(__SA, __IT) neko_slot_array_get(__SA, __IT)

#define neko_slot_array_iter_getp(__SA, __IT) neko_slot_array_getp(__SA, __IT)

/*===================================
// Slot Map
===================================*/

#define neko_slot_map(__SMK, __SMV)          \
    struct {                                 \
        neko_hash_table(__SMK, uint32_t) ht; \
        neko_slot_array(__SMV) sa;           \
    }*

#define neko_slot_map_new(__SMK, __SMV) NULL

NEKO_API_DECL void** neko_slot_map_init(void** sm);

// Could return something, I believe?
#define neko_slot_map_insert(__SM, __SMK, __SMV)                      \
    do {                                                              \
        neko_slot_map_init((void**)&(__SM));                          \
        uint32_t __H = neko_slot_array_insert((__SM)->sa, ((__SMV))); \
        neko_hash_table_insert((__SM)->ht, (__SMK), __H);             \
    } while (0)

#define neko_slot_map_get(__SM, __SMK) (neko_slot_array_get((__SM)->sa, neko_hash_table_get((__SM)->ht, (__SMK))))

#define neko_slot_map_getp(__SM, __SMK) (neko_slot_array_getp((__SM)->sa, neko_hash_table_get((__SM)->ht, (__SMK))))

#define neko_slot_map_size(__SM) (neko_slot_array_size((__SM)->sa))

#define neko_slot_map_clear(__SM)              \
    do {                                       \
        if ((__SM) != NULL) {                  \
            neko_hash_table_clear((__SM)->ht); \
            neko_slot_array_clear((__SM)->sa); \
        }                                      \
    } while (0)

#define neko_slot_map_erase(__SM, __SMK)                         \
    do {                                                         \
        uint32_t __K = neko_hash_table_get((__SM)->ht, (__SMK)); \
        neko_hash_table_erase((__SM)->ht, (__SMK));              \
        neko_slot_array_erase((__SM)->sa, __K);                  \
    } while (0)

#define neko_slot_map_free(__SM)              \
    do {                                      \
        if (__SM != NULL) {                   \
            neko_hash_table_free((__SM)->ht); \
            neko_slot_array_free((__SM)->sa); \
            neko_free((__SM));                \
            (__SM) = NULL;                    \
        }                                     \
    } while (0)

#define neko_slot_map_capacity(__SM) (neko_hash_table_capacity((__SM)->ht))

/*=== Slot Map Iterator ===*/

typedef uint32_t neko_slot_map_iter;

/* Find first valid iterator idx */
#define neko_slot_map_iter_new(__SM) neko_hash_table_iter_new((__SM)->ht)

#define neko_slot_map_iter_valid(__SM, __IT) ((__IT) < neko_hash_table_capacity((__SM)->ht))

#define neko_slot_map_iter_advance(__SM, __IT) \
    __neko_hash_table_iter_advance_func((void**)&((__SM)->ht->data), sizeof((__SM)->ht->tmp_key), sizeof((__SM)->ht->tmp_val), &(__IT), (__SM)->ht->stride, (__SM)->ht->klpvl)

#define neko_slot_map_iter_getk(__SM, __IT) neko_hash_table_iter_getk((__SM)->ht, (__IT))
//(neko_hash_table_find_valid_iter(__SM->ht, __IT), neko_hash_table_geti((__SM)->ht, (__IT)))

#define neko_slot_map_iter_getkp(__SM, __IT) (neko_hash_table_find_valid_iter(__SM->ht, __IT), &(neko_hash_table_geti((__SM)->ht, (__IT))))

#define neko_slot_map_iter_get(__SM, __IT) ((__SM)->sa->data[neko_hash_table_iter_get((__SM)->ht, (__IT))])

// ((__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))])
// (neko_hash_table_find_valid_iter(__SM->ht, __IT), (__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))])

#define neko_slot_map_iter_getp(__SM, __IT) (&((__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))]))

// (neko_hash_table_find_valid_iter(__SM->ht, __IT), &((__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))]))

/*===================================
// Command Buffer
===================================*/

typedef struct neko_command_buffer_t {
    uint32_t num_commands;
    neko_byte_buffer_t commands;
} neko_command_buffer_t;

neko_force_inline neko_command_buffer_t neko_command_buffer_new() {
    neko_command_buffer_t cb = neko_default_val();
    cb.commands = neko_byte_buffer_new();
    return cb;
}

#define neko_command_buffer_write(__CB, __CT, __C, __T, __VAL) \
    do {                                                       \
        neko_command_buffer_t* __cb = (__CB);                  \
        __cb->num_commands++;                                  \
        neko_byte_buffer_write(&__cb->commands, __CT, (__C));  \
        neko_byte_buffer_write(&__cb->commands, __T, (__VAL)); \
    } while (0)

neko_force_inline void neko_command_buffer_clear(neko_command_buffer_t* cb) {
    cb->num_commands = 0;
    neko_byte_buffer_clear(&cb->commands);
}

neko_force_inline void neko_command_buffer_free(neko_command_buffer_t* cb) { neko_byte_buffer_free(&cb->commands); }

#define neko_command_buffer_readc(__CB, __T, __NAME) \
    __T __NAME = neko_default_val();                 \
    neko_byte_buffer_read(&(__CB)->commands, __T, &__NAME);

#ifndef NEKO_NO_SHORT_NAME
typedef neko_command_buffer_t neko_cmdbuf;
#endif

#endif