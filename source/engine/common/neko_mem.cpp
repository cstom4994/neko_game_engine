#include "neko_mem.h"

#include <array>
#include <cassert>
#include <cerrno>
#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "engine/common/neko_types.h"
#include "engine/common/neko_util.h"
#include "engine/utility/logger.hpp"

namespace neko {

allocation_metrics g_allocation_metrics = {.total_allocated = 0, .total_free = 0};

u64 neko_mem_current_usage_bytes() { return g_allocation_metrics.total_allocated - g_allocation_metrics.total_free; }
f32 neko_mem_current_usage_mb() {
    u64 bytes = neko_mem_current_usage_bytes();
    return (f32)(bytes / 1048576.0f);
}

struct neko_mem_stack_t {
    void* memory;
    size_t capacity;
    size_t bytes_left;
};

#define NEKO_PTR_ADD(ptr, size) ((void*)(((char*)ptr) + (size)))
#define NEKO_PTR_SUB(ptr, size) ((void*)(((char*)ptr) - (size)))

neko_mem_stack_t* neko_mem_stack_create(void* memory_chunk, size_t size) {
    neko_mem_stack_t* stack = (neko_mem_stack_t*)memory_chunk;
    if (size < sizeof(neko_mem_stack_t)) return 0;
    *(size_t*)NEKO_PTR_ADD(memory_chunk, sizeof(neko_mem_stack_t)) = 0;
    stack->memory = NEKO_PTR_ADD(memory_chunk, sizeof(neko_mem_stack_t) + sizeof(size_t));
    stack->capacity = size - sizeof(neko_mem_stack_t) - sizeof(size_t);
    stack->bytes_left = stack->capacity;
    return stack;
}

void* neko_mem_stack_alloc(neko_mem_stack_t* stack, size_t size) {
    if (stack->bytes_left - sizeof(size_t) < size) return 0;
    void* user_mem = stack->memory;
    *(size_t*)NEKO_PTR_ADD(user_mem, size) = size;
    stack->memory = NEKO_PTR_ADD(user_mem, size + sizeof(size_t));
    stack->bytes_left -= sizeof(size_t) + size;
    return user_mem;
}

int neko_mem_stack_free(neko_mem_stack_t* stack, void* memory) {
    if (!memory) return 0;
    size_t size = *(size_t*)NEKO_PTR_SUB(stack->memory, sizeof(size_t));
    void* prev = NEKO_PTR_SUB(stack->memory, size + sizeof(size_t));
    if (prev != memory) return 0;
    stack->memory = prev;
    stack->bytes_left += sizeof(size_t) + size;
    return 1;
}

size_t neko_mem_stack_bytes_left(neko_mem_stack_t* stack) { return stack->bytes_left; }

struct neko_mem_frame_t {
    void* original;
    void* ptr;
    size_t capacity;
    size_t bytes_left;
};

neko_mem_frame_t* neko_mem_frame_create(void* memory_chunk, size_t size) {
    neko_mem_frame_t* frame = (neko_mem_frame_t*)memory_chunk;
    frame->original = NEKO_PTR_ADD(memory_chunk, sizeof(neko_mem_frame_t));
    frame->ptr = frame->original;
    frame->capacity = frame->bytes_left = size - sizeof(neko_mem_frame_t);
    return frame;
}

void* neko_mem_frame_alloc(neko_mem_frame_t* frame, size_t size) {
    if (frame->bytes_left < size) return 0;
    void* user_mem = frame->ptr;
    frame->ptr = NEKO_PTR_ADD(frame->ptr, size);
    frame->bytes_left -= size;
    return user_mem;
}

void neko_mem_frame_free(neko_mem_frame_t* frame) {
    frame->ptr = frame->original;
    frame->bytes_left = frame->capacity;
}

typedef struct neko_mem_heap_header_t {
    struct neko_mem_heap_header_t* next;
    struct neko_mem_heap_header_t* prev;
    size_t size;
} neko_mem_heap_header_t;

typedef struct neko_mem_alloc_info_t neko_mem_alloc_info_t;
struct neko_mem_alloc_info_t {
    const char* file;
    size_t size;
    int line;

    struct neko_mem_alloc_info_t* next;
    struct neko_mem_alloc_info_t* prev;
};

static neko_mem_alloc_info_t* neko_mem_alloc_head() {
    static neko_mem_alloc_info_t info;
    static int init;

    if (!init) {
        info.next = &info;
        info.prev = &info;
        init = 1;
    }

    return &info;
}

#if 1
void* neko_mem_leak_check_alloc(size_t size, const char* file, int line, size_t* statistics) {
    neko_mem_alloc_info_t* mem = (neko_mem_alloc_info_t*)neko_malloc(sizeof(neko_mem_alloc_info_t) + size);

    if (!mem) return 0;

    mem->file = file;
    mem->line = line;
    mem->size = size;
    neko_mem_alloc_info_t* head = neko_mem_alloc_head();
    mem->prev = head;
    mem->next = head->next;
    head->next->prev = mem;
    head->next = mem;

    g_allocation_metrics.total_allocated += size;
    if (NULL != statistics) *statistics += size;

    return mem + 1;
}

void* neko_mem_leak_check_calloc(size_t count, size_t element_size, const char* file, int line, size_t* statistics) {
    size_t size = count * element_size;
    void* mem = neko_mem_leak_check_alloc(size, file, line, statistics);
    std::memset(mem, 0, size);
    return mem;
}

void neko_mem_leak_check_free(void* mem, size_t* statistics) {
    if (!mem) return;

    neko_mem_alloc_info_t* info = (neko_mem_alloc_info_t*)mem - 1;
    info->prev->next = info->next;
    info->next->prev = info->prev;

    g_allocation_metrics.total_free += info->size;
    if (NULL != statistics) *statistics -= info->size;

    neko_free(info);
}

void* neko_mem_leak_check_realloc(void* ptr, size_t new_size, const char* file, int line, size_t* statistics) {
    if (new_size == 0) {
        neko_mem_leak_check_free(ptr, statistics);  // 如果新大小为 0 则直接释放原内存块并返回 NULL
        return NULL;
    }

    if (ptr == NULL) {
        return neko_mem_leak_check_alloc(new_size, file, line, statistics);  // 如果原指针为空 则等同于 alloc
    }

    void* new_ptr = neko_mem_leak_check_alloc(new_size, file, line, statistics);  // 分配新大小的内存块

    if (new_ptr != NULL) {
        // 复制旧内存块中的数据到新内存块

        neko_mem_alloc_info_t* info = (neko_mem_alloc_info_t*)ptr - 1;

        size_t old_size = info->size;
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);

        neko_mem_leak_check_free(ptr, statistics);  // 释放旧内存块
    }

    return new_ptr;
}

int neko_mem_check_leaks(bool detailed) {
    neko_mem_alloc_info_t* head = neko_mem_alloc_head();
    neko_mem_alloc_info_t* next = head->next;
    int leaks = 0;

    std::size_t leaks_size = 0;

    while (next != head) {
        if (detailed) {
            neko_warn(std::format("LEAKED {0} bytes from file \"{1}\" at line {2} from address {3}.", next->size, neko_fs_get_filename(next->file), next->line, (void*)(next + 1)));
        }
        leaks_size += next->size;
        next = next->next;
        leaks = 1;
    }

    if (leaks && detailed) {
        neko_trace("memory leaks detected (see above).");
    }
    if (leaks) {
        double megabytes = neko_s_cast<double>(leaks_size) / 1048576;
        neko_warn(std::format("memory leaks detected with {0} bytes equal to {1:.4f} MB.", leaks_size, megabytes));
    } else {
        neko_debug("no memory leaks detected.");
    }
    return leaks;
}

int neko_mem_bytes_inuse() {
    neko_mem_alloc_info_t* head = neko_mem_alloc_head();
    neko_mem_alloc_info_t* next = head->next;
    int bytes = 0;

    while (next != head) {
        bytes += (int)next->size;
        next = next->next;
    }

    return bytes;
}

#else

inline void* neko_mem_leak_check_alloc(size_t new_size, char* file, int line) { return NEKO_MALLOC_FUNC(new_size); }

void* neko_mem_leak_check_calloc(size_t count, size_t element_size, char* file, int line) { return NEKO_CALLOC_FUNC(count, new_size); }

inline void neko_mem_leak_check_free(void* mem) { return NEKO_FREE_FUNC(mem); }

inline int NEKO_CHECK_FOR_LEAKS() { return 0; }
inline int NEKO_BYTES_IN_USE() { return 0; }

#endif

void neko_mem_init(int argc, char* argv[]) {}

void neko_mem_end() { neko_mem_check_leaks(false); }

void neko_mem_rungc() {}

}  // namespace neko