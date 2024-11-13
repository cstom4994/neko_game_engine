#pragma once

#include "base/common/mutex.hpp"

namespace Neko {

struct Allocator {
    size_t alloc_size;
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual void *alloc(size_t bytes, const char *file, i32 line) = 0;
    virtual void *realloc(void *ptr, size_t new_size, const char *file, i32 line) = 0;
    virtual void free(void *ptr) = 0;
};

struct HeapAllocator : Allocator {
    void make() {}
    void trash() {}
    void *alloc(size_t bytes, const char *, i32) { return malloc(bytes); }
    void *realloc(void *ptr, size_t new_size, const char *, i32) { return ::realloc(ptr, new_size); }
    void free(void *ptr) { ::free(ptr); }
};

struct DebugAllocInfo {
    const char *file;
    i32 line;
    size_t size;
    DebugAllocInfo *prev;
    DebugAllocInfo *next;
    alignas(16) u8 buf[1];
};

struct DebugAllocator : Allocator {
    DebugAllocInfo *head = nullptr;
    Mutex mtx;

    void make() {}
    void trash() {}
    void *alloc(size_t bytes, const char *file, i32 line);
    void *realloc(void *ptr, size_t new_size, const char *file, i32 line);
    void free(void *ptr);
    void dump_allocs(bool detailed);
};

extern Allocator *g_allocator;

inline void *__neko_mem_calloc(size_t count, size_t element_size, const char *file, int line) {
    size_t size = count * element_size;
    void *mem = g_allocator->alloc(size, file, line);
    memset(mem, 0, size);
    return mem;
}

#define mem_alloc(bytes) ::Neko::g_allocator->alloc(bytes, __FILE__, __LINE__)
#define mem_free(ptr) ::Neko::g_allocator->free((void *)ptr)
#define mem_realloc(ptr, size) ::Neko::g_allocator->realloc(ptr, size, __FILE__, __LINE__)
#define mem_calloc(count, element_size) ::Neko::__neko_mem_calloc(count, element_size, (char *)__FILE__, __LINE__)

inline void *DebugAllocator::alloc(size_t bytes, const char *file, i32 line) {
    LockGuard<Mutex> lock{mtx};

    DebugAllocInfo *info = (DebugAllocInfo *)::malloc(offsetof(DebugAllocInfo, buf[bytes]));
    neko_assert(info, "FAILED_TO_ALLOCATE");
    info->file = file;
    info->line = line;
    info->size = bytes;
    info->prev = nullptr;
    info->next = head;
    if (head != nullptr) {
        head->prev = info;
    }
    head = info;

    alloc_size += info->size;

    return info->buf;
}

inline void *DebugAllocator::realloc(void *ptr, size_t new_size, const char *file, i32 line) {
    if (ptr == nullptr) {
        // 如果指针为空 则分配新内存
        return this->alloc(new_size, file, line);
    }

    if (new_size == 0) {
        // 如果新大小为零 则释放内存并返回 null
        this->free(ptr);
        return nullptr;
    }

    LockGuard<Mutex> lock{mtx};

    DebugAllocInfo *old_info = (DebugAllocInfo *)((u8 *)ptr - offsetof(DebugAllocInfo, buf));

    alloc_size -= old_info->size;

    // 分配新大小的新内存块
    DebugAllocInfo *new_info = (DebugAllocInfo *)::malloc(NEKO_OFFSET(DebugAllocInfo, buf[new_size]));
    neko_assert(new_info, "FAILED_TO_ALLOCATE");
    if (new_info == nullptr) {
        return nullptr;  // 分配失败
    }

    // 将数据从旧内存块复制到新内存块
    size_t copy_size = old_info->size < new_size ? old_info->size : new_size;
    memcpy(new_info->buf, old_info->buf, copy_size);

    // 更新新的内存块信息
    new_info->file = file;
    new_info->line = line;
    new_info->size = new_size;
    new_info->prev = old_info->prev;
    new_info->next = old_info->next;
    if (new_info->prev != nullptr) {
        new_info->prev->next = new_info;
    } else {
        head = new_info;
    }
    if (new_info->next != nullptr) {
        new_info->next->prev = new_info;
    }

    alloc_size += new_size;

    // 释放旧内存块
    ::free(old_info);

    return new_info->buf;
}

inline void DebugAllocator::free(void *ptr) {
    if (ptr == nullptr) {
        return;
    }

    LockGuard<Mutex> lock{mtx};

    DebugAllocInfo *info = (DebugAllocInfo *)((u8 *)ptr - offsetof(DebugAllocInfo, buf));

    alloc_size -= info->size;

    if (info->prev == nullptr) {
        head = info->next;
    } else {
        info->prev->next = info->next;
    }

    if (info->next) {
        info->next->prev = info->prev;
    }

    ::free(info);
}

inline void DebugAllocator::dump_allocs(bool detailed) {
    i32 allocs = 0;
    for (DebugAllocInfo *info = head; info != nullptr; info = info->next) {
        if (detailed) printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size, info->file, info->line);
        allocs++;
    }
    neko_println("  --- leaks %d allocation(s) with %lld bytes ---", allocs, alloc_size);
}

}  // namespace Neko