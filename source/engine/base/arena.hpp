#pragma once

#include "engine/base/array.hpp"
#include "engine/base/base.hpp"
#include "engine/base/mem.hpp"
#include "engine/base/string.hpp"

struct ArenaNode {
    ArenaNode *next;
    u64 capacity;
    u64 allocd;
    u64 prev;
    u8 buf[1];
};

static u64 align_forward(u64 p, u32 align) {
    if ((p & (align - 1)) != 0) {
        p += align - (p & (align - 1));
    }
    return p;
}

static ArenaNode *arena_block_make(u64 capacity) {
    u64 page = 4096 - offsetof(ArenaNode, buf);
    if (capacity < page) {
        capacity = page;
    }

    ArenaNode *a = (ArenaNode *)mem_alloc(offsetof(ArenaNode, buf[capacity]));
    a->next = nullptr;
    a->allocd = 0;
    a->capacity = capacity;
    return a;
}

struct Arena {
    ArenaNode *head;

    inline void trash() {
        ArenaNode *a = head;
        while (a != nullptr) {
            ArenaNode *rm = a;
            a = a->next;
            mem_free(rm);
        }
    }

    inline void *bump(u64 size) {
        if (head == nullptr) {
            head = arena_block_make(size);
        }

        u64 next = 0;
        do {
            next = align_forward(head->allocd, 16);
            if (next + size <= head->capacity) {
                break;
            }

            ArenaNode *block = arena_block_make(size);
            block->next = head;

            head = block;
        } while (true);

        void *ptr = &head->buf[next];
        head->allocd = next + size;
        head->prev = next;
        return ptr;
    }

    inline void *rebump(void *ptr, u64 old, u64 size) {
        if (head == nullptr || ptr == nullptr || old == 0) {
            return bump(size);
        }

        if (&head->buf[head->prev] == ptr) {
            u64 resize = head->prev + size;
            if (resize <= head->capacity) {
                head->allocd = resize;
                return ptr;
            }
        }

        void *new_ptr = bump(size);

        u64 copy = old < size ? old : size;
        memmove(new_ptr, ptr, copy);

        return new_ptr;
    }

    inline String bump_string(String s) {
        if (s.len > 0) {
            char *cstr = (char *)bump(s.len + 1);
            memcpy(cstr, s.data, s.len);
            cstr[s.len] = '\0';
            return {cstr, s.len};
        } else {
            return {};
        }
    }
};

template <typename T>
struct Slice {
    T *data = nullptr;
    u64 len = 0;

    Slice() = default;
    explicit Slice(Array<T> arr) : data(arr.data), len(arr.len) {}

    T &operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    const T &operator[](size_t i) const {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void resize(u64 n) {
        T *buf = (T *)mem_alloc(sizeof(T) * n);
        memcpy(buf, data, sizeof(T) * len);
        mem_free(data);
        data = buf;
        len = n;
    }

    void resize(Arena *arena, u64 n) {
        T *buf = (T *)arena->rebump(data, sizeof(T) * len, sizeof(T) * n);
        data = buf;
        len = n;
    }

    T *begin() { return data; }
    T *end() { return &data[len]; }
    const T *begin() const { return data; }
    const T *end() const { return &data[len]; }
};
