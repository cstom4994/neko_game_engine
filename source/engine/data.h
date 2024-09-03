#ifndef NEKO_DATA_H
#define NEKO_DATA_H

#include <assert.h>
#include <stdint.h>

#include <variant>

#include "engine/base.h"

// #include "memory.h"
// #include "types.h"

#define GLOBAL_BUFFER_SIZE 134217728  // 128MB

#define ALLOCATOR_INIT(_buf, _data, _size) \
    do {                                   \
        (_buf)->allocator.data = _data;    \
        (_buf)->allocator.cap = _size;     \
        (_buf)->allocator.used = 0;        \
    } while (0)

#define allocator_isempty(_buf) ((_buf)->used == 0)
#define allocator_isfull(_buf) ((_buf)->cap == (_buf)->used)
#define allocator_unused(_buf) ((_buf)->cap - (_buf)->used)

#define list_foreach(_list, _node) for ((_node) = (_list)->next; (_list) != (_node); (_node) = (_node)->next)

#define hashmap_put(_m, _k, _v) _hashmap_put((_m), (key_t)(_k), (void*)(_v))
#define hashmap_get(_m, _k) _hashmap_get((_m), (key_t)(_k))
#define hashmap_isempty(_m) allocator_isempty(_m)
#define hashmap_remove(_m, _k) _hashmap_remove((_m), (key_t)(_k))
#define hashmap_foreach(_m, _key, _value)                                   \
    if (!hashmap_isempty(_m))                                               \
        for (unsigned int _i = 0; _i < (_m)->cap / sizeof(hash_node); _i++) \
            if (((hash_node*)(_m)->data)[_i].count > 0, (_key) = ((hash_node*)(_m)->data)[_i].key, (_value) = ((hash_node*)(_m)->data)[_i].data)

#define circbuffer_full(_b, _size) (((_b)->head + 1) % (_size) == (_b)->tail)
#define circbuffer_empty(_b, _size) ((_b)->head == (_b)->tail)
#define circbuffer_push(_b, _val, _size)      \
    do {                                      \
        if circbuffer_full (_b, _size) break; \
        int _next = (_b)->head + 1;           \
        if (_next >= (_size)) _next = 0;      \
        (_b)->buffer[(_b)->head] = _val;      \
        (_b)->head = _next;                   \
    } while (0)
#define circbuffer_pop(_b, _val, _size)        \
    do {                                       \
        if circbuffer_empty (_b, _size) break; \
        int _next = (_b)->tail + 1;            \
        if (_next >= _size) _next = 0;         \
        _val = (_b)->buffer[(_b)->tail];       \
        (_b)->tail = _next;                    \
    } while (0)

#define GLOBAL_SINGLETON(_type, _fname, _name)          \
    _type* _fname##_instance() {                        \
        static _type* manager = NULL;                   \
        if (!manager) {                                 \
            manager = (_type*)mem_alloc(sizeof(_type)); \
            memset(manager, 0, sizeof(_type));          \
            _fname##_init(manager, _name);              \
        }                                               \
        return manager;                                 \
    }

typedef uint64_t key_t;

// typedef struct {
//     int t;
//     union {
//         uint64_t data;
//         double nr;
//         void* ptr;
//     };
// } event_variant_t;

using event_variant_t = struct {
    int t;
    std::variant<u64, f64, void*> v;
};

typedef struct listnode_t {
    struct listnode_t* prev;
    struct listnode_t* next;
} listnode_t;

typedef struct allocator_t {
    char* data;
    size_t cap;
    size_t used;
} allocator_t;

typedef struct {
    allocator_t allocator;
    size_t element_size;
    void** free_list;
} pool_t;

typedef struct {
    allocator_t allocator;
    size_t read, write, end;
} buffer_t;

typedef struct {
    allocator_t allocator;
    size_t element_size;
} vector_t;

typedef struct {
    allocator_t allocator;
} hashmap_t;

typedef struct {
    buffer_t buffer;
    size_t element_size;
} queue_t;

typedef struct {
    key_t key;
    int count;
    void* data;
} hash_node;

inline void list_init(listnode_t* list) {
    list->prev = list;
    list->next = list;
}

inline void list_add(struct listnode_t* list, struct listnode_t* node) {
    node->prev = list->prev;
    node->next = list;
    list->prev->next = node;
    list->prev = node;
}

inline void list_remove(listnode_t* list, listnode_t* node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

void hashmap_init(hashmap_t* map);
int _hashmap_put(hashmap_t* m, key_t key, void* value);
void* _hashmap_get(hashmap_t* m, key_t key);
int _hashmap_remove(hashmap_t* m, key_t key);
void hashmap_free(hashmap_t* m);

void vector_init(vector_t* vec, size_t element_size);
void vector_add(vector_t* vec, void* ptr);
void vector_set(vector_t* vec, size_t index, void* ptr);
void* vector_get(vector_t* vec, size_t index);
void vector_remove(vector_t* vec, void* ptr);
int vector_pop(vector_t* vec, void* ptr);
void vector_free(vector_t* vec);

void queue_init(queue_t* q, size_t element_size);
void queue_free(queue_t* q);
int queue_put(queue_t* q, void* data);
int queue_get(queue_t* q, void* data);

void bip_init(buffer_t* b, void* data, size_t size);
void* bip_alloc(buffer_t* b, size_t size);
void bip_free(buffer_t* b, void* ptr);
void bip_clear(buffer_t* b);

int bip_write(buffer_t* b, size_t size);
void* bip_peek(buffer_t* b, size_t size);
int bip_read(buffer_t* b, size_t size);

void* object_alloc(size_t size);
void object_free(void* ptr);

inline uint32_t hash_int(uint64_t key) {
    /* Robert Jenkins' 32 bit Mix Function */
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    /* Knuth's Multiplicative Method */
    key = (key >> 3) * 2654435761;

    return (uint32_t)key;
}

/*
#define HANDLE(_ui32) (Handle){.value = (_ui32)}
#define NULL_HANDLE HANDLE(0)
#define MAX_HANDLE_ENTRIES 4096

typedef union {
    struct {
        uint32_t index : 12;
        uint32_t counter : 15;
        uint32_t type : 5;
    };
    uint32_t value;
} Handle;

typedef struct {
    uint32_t next : 12;
    uint32_t counter : 15;
    uint32_t active : 1;
    uint32_t end : 1;
    void* data;
} handle_entry;

typedef struct {
    handle_entry entries[MAX_HANDLE_ENTRIES];
    int count;
    uint32_t free_list;
} handle_manager;

handle_manager* handlemanager_init();
Handle handlemanager_add(handle_manager* manager, void* data, uint32_t type);
void handlemanager_update(handle_manager* manager, Handle handle, void* data);
void handlemanager_remove(handle_manager* manager, Handle handle);
int handlemanager_lookup(handle_manager* manager, Handle handle, void** out);
void* handlemanager_get(handle_manager* manager, Handle handle);
*/

#endif