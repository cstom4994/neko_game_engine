#pragma once

#include "engine/base/base.hpp"
#include "engine/base/mem.hpp"

template <typename T>
struct Array {
    T *data = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    T &operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void trash() {
        mem_free(data);
        len = 0;
        capacity = 0;
    }

    void reserve(u64 cap) {
        if (cap > capacity) {
            T *buf = (T *)mem_alloc(sizeof(T) * cap);
            memcpy(buf, data, sizeof(T) * len);
            mem_free(data);
            data = buf;
            capacity = cap;
        }
    }

    void resize(u64 n) {
        reserve(n);
        len = n;
    }

    bool valid(u64 i) { return (i >= 0 && i < len); }

    u64 push(T item) {
        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }
        data[len] = item;
        return len++;
    }

    T *get_ptr(u64 i) { return &data[i]; }

    // 快速删除函数
    void quick_remove(u64 i) {
        assert(valid(i));             // 保证索引合法
        if (i < len - 1) {            // 如果不是最后一个元素
            data[i] = data[len - 1];  // 用最后一个元素覆盖要删除的元素
        }
        len--;  // 减少长度
    }

    void sort(int (*compar)(const void *, const void *)) {
        if (len > 1) {
            qsort(data, len, sizeof(T), compar);
        }
    }

    T *begin() { return data; }
    T *end() { return &data[len]; }
};
