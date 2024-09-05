#include "engine/data.h"

#include "engine/base.hpp"

#define HASHMAP_MAX_CHAIN_LENGTH 8
#define INITIAL_MAP_SIZE 4 * sizeof(hash_node)

#define MAP_SIZE(_map) ((_map)->allocator.cap / sizeof(hash_node))
#define MAP_ENTRY(_map, _i) (((hash_node*)m->allocator.data)[(_i)])
#define MAP_MASK(_map) (MAP_SIZE(_map) - 1)

void hashmap_init(hashmap_t* m) { ALLOCATOR_INIT(m, (char*)mem_calloc(INITIAL_MAP_SIZE, sizeof(hash_node)), INITIAL_MAP_SIZE); }

int hashmap_hash(hashmap_t* m, hashmap_key_t key) {
    const int mask = MAP_MASK(m);
    const u32 hash = hash_int(key);
    int curr = hash & mask;
    for (int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
        if (MAP_ENTRY(m, curr).count == 0) return curr;

        if (MAP_ENTRY(m, curr).count > 0 && (MAP_ENTRY(m, curr).key == key)) return curr;

        curr = (curr + 1) & mask;
    }

    return -1;
}

int hashmap_rehash(hashmap_t* m) {
    hash_node* temp = (hash_node*)mem_calloc(2 * MAP_SIZE(m), sizeof(hash_node));
    if (!temp) return -1;

    hash_node* curr = (hash_node*)m->allocator.data;
    m->allocator.data = (char*)temp;

    int old_size = MAP_SIZE(m);
    m->allocator.cap = 2 * m->allocator.cap;
    m->allocator.used = 0;

    int status;
    for (int i = 0; i < old_size; i++) {
        if (curr[i].count == 0) continue;

        status = _hashmap_put(m, curr[i].key, curr[i].data);
        if (status != 0) return status;
    }
    mem_free(curr);

    return 0;
}

int _hashmap_put(hashmap_t* m, hashmap_key_t key, void* value) {
    int index = hashmap_hash(m, key);
    while (index < 0) {
        if (hashmap_rehash(m) == -1) return -1;

        index = hashmap_hash(m, key);
    }
    if (MAP_ENTRY(m, index).count == 0) m->allocator.used += sizeof(hash_node);
    MAP_ENTRY(m, index) = hash_node{key, 1, value};

    return 0;
}

void* _hashmap_get(hashmap_t* m, hashmap_key_t key) {
    int curr = hash_int(key) & MAP_MASK(m);

    for (int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
        if (MAP_ENTRY(m, curr).count > 0) {
            if (MAP_ENTRY(m, curr).key == key) {
                return MAP_ENTRY(m, curr).data;
            }
        }
        curr = (curr + 1) & MAP_MASK(m);
    }

    return NULL;
}

int _hashmap_remove(hashmap_t* m, hashmap_key_t key) {
    int curr = hash_int(key) & MAP_MASK(m);
    for (int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
        if (MAP_ENTRY(m, curr).count > 0) {
            if (MAP_ENTRY(m, curr).key == key) {
                MAP_ENTRY(m, curr) = hash_node{NULL, 0, NULL};
                m->allocator.used -= sizeof(hash_node);
                return 1;
            }
        }
        curr = (curr + 1) & MAP_MASK(m);
    }
    return 0;
}

void hashmap_free(hashmap_t* m) { mem_free(m->allocator.data); }

void bip_init(buffer_t* buf, void* data, size_t size) {
    ALLOCATOR_INIT(buf, (char*)data, size);
    bip_clear(buf);
}

void* bip_alloc(buffer_t* buf, size_t size) {
    if (size > 0 && size < buf->allocator.cap) {
        size_t read = buf->read;    // Atomic
        size_t write = buf->write;  // Atomic
        if (write >= read) {
            size_t available = buf->allocator.cap - write - ((read > 0) ? 0 : 1);
            if (size <= available)
                return &buf->allocator.data[write];
            else {
                if (size < read)
                    return buf->allocator.data;
                else if ((write == read) && (size == read)) {
                    buf->read = 0;   // Atomic
                    buf->write = 0;  // Atomic
                    return buf->allocator.data;
                }
            }
        } else if ((write + size) < read)
            return &buf->allocator.data[write];
    }
    return NULL;
}

int bip_write(buffer_t* buf, size_t size) {
    buf->allocator.used += size;
    if (size > 0 && size < buf->allocator.cap) {
        size_t read = buf->read;    // Atomic
        size_t write = buf->write;  // Atomic
        if (write >= read) {
            size_t available = buf->allocator.cap - write - ((read > 0) ? 0 : 1);
            if (size <= available) {
                if (size < (buf->allocator.cap - write)) {
                    buf->write = write + size;  // Atomic
                    return 1;
                } else if (size == (buf->allocator.cap - write)) {
                    buf->write = 0;  // Atomic
                    return 1;
                }
            } else if (size < read) {
                buf->end = write;
                buf->write = size;  // Atomic
                return 1;
            }
        } else if ((write + size) < read) {
            buf->write = write + size;  // Atomic
            return 1;
        }
    } else if (size == 0)
        return 1;
    buf->allocator.used -= size;
    return 0;
}

void* bip_peek(buffer_t* buf, size_t size) {
    if (size > 0 && size < buf->allocator.cap) {
        size_t read = buf->read;    // Atomic
        size_t write = buf->write;  // Atomic
        if ((write >= read) && ((read + size) <= write))
            return &buf->allocator.data[read];
        else if (read < buf->allocator.cap) {
            size_t wrap = buf->end;  // Atomic
            if ((read + size) <= wrap)
                return &buf->allocator.data[read];
            else if (read == wrap && size <= write)
                return buf->allocator.data;
        }
    }
    return NULL;
}

int bip_read(buffer_t* buf, size_t size) {
    buf->allocator.used -= size;
    if (size > 0 && size < buf->allocator.cap) {
        size_t read = buf->read;    // Atomic
        size_t write = buf->write;  // Atomic
        if (read < write) {
            if (read + size <= write) {
                buf->read = read + size;  // Atomic
                return 1;
            }
        } else if (read > write) {
            size_t wrap = buf->end;  // Atomic
            if (read + size < wrap) {
                buf->read = read + size;  // Atomic
                return 1;
            } else if (read + size == wrap) {
                buf->end = buf->allocator.cap;
                buf->read = 0;
                return 1;
            } else if ((read == wrap) && (size <= write)) {
                buf->end = buf->allocator.cap;
                buf->read = size;
                return 1;
            }
        }
    } else if (size == 0)
        return 1;
    buf->allocator.used += size;
    return 0;
}

void bip_free(buffer_t* buf, void* ptr) { mem_free(buf->allocator.data); }

void bip_clear(buffer_t* buf) {
    buf->write = 0;
    buf->read = 0;
    buf->end = buf->allocator.cap;
}

#define INITIAL_QUEUE_SIZE 4096

// TODO: 互斥和cv
void queue_init(queue_t* q, size_t element_size) {
    bip_init((buffer_t*)q, mem_alloc(INITIAL_QUEUE_SIZE * element_size), INITIAL_QUEUE_SIZE * element_size);
    // TODO: 也许让它成为一个简单的循环缓冲区
    q->element_size = element_size;
}

void queue_free(queue_t* q) { bip_free((buffer_t*)q, NULL); }

int queue_put(queue_t* q, void* data) {
    size_t len = q->element_size;
    void* ptr = bip_alloc((buffer_t*)q, len);
    if (ptr) {
        memcpy(ptr, data, len);
        return bip_write((buffer_t*)q, len);
    }
    return 0;
}

int queue_get(queue_t* q, void* data) {
    size_t len = q->element_size;
    void* ptr = bip_peek((buffer_t*)q, len);
    if (ptr) {
        memcpy(data, ptr, len);
        return bip_read((buffer_t*)q, len);
    }
    return 0;
}

void* object_alloc(size_t size) { return mem_alloc(size); }

void object_free(void* ptr) { mem_free(ptr); }
