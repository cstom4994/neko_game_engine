

#ifndef NEKO_HASH_H
#define NEKO_HASH_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_util.h"

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

void __neko_hash_table_init_impl(void** ht, size_t sz);

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
            neko_safe_free(__HT);              \
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

#endif