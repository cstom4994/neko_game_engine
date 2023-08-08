#ifndef NEKO_MEM_H
#define NEKO_MEM_H

#include <type_traits>

#include "engine/common/neko_types.h"

namespace neko {

#if 1

#ifndef neko_safe_malloc
#define neko_safe_malloc(size) ::neko::__neko_mem_safe_alloc((size), (char*)__FILE__, __LINE__)
#endif

#ifndef neko_safe_free
#define neko_safe_free(mem) ::neko::__neko_mem_safe_free(mem)
#endif

#ifndef neko_safe_realloc
#define neko_safe_realloc(ptr, size) ::neko::__neko_mem_safe_realloc((ptr), (size), (char*)__FILE__, __LINE__)
#endif

#ifndef neko_safe_calloc
#define neko_safe_calloc(count, element_size) ::neko::__neko_mem_safe_calloc(count, element_size, (char*)__FILE__, __LINE__)
#endif

#else

#ifndef neko_safe_malloc
#define neko_safe_malloc ME_MALLOC_FUNC
#endif

#ifndef neko_safe_free
#define neko_safe_free ME_FREE_FUNC
#endif

#endif

// 单纯用来测试的 new 和 delete
// 不用于开发目的

#ifndef TEST_NEW
#define TEST_NEW(_name, _class, ...)    \
    (_class*)ME_MALLOC(sizeof(_class)); \
    new ((void*)_name) _class(__VA_ARGS__)
#endif

template <typename T>
struct alloc {
    template <typename... Args>
    static T* safe_malloc(Args&&... args) {
        void* mem = neko_safe_malloc(sizeof(T));
        if (!mem) {
        }
        return new (mem) T(std::forward<Args>(args)...);
    }
};

#ifndef TEST_DELETE
#define TEST_DELETE(_name, _class) \
    {                              \
        _name->~_class();          \
        neko_safe_free(_name);     \
    }
#endif

void* __neko_mem_safe_alloc(size_t size, const char* file, int line, size_t* statistics = NULL);
void* __neko_mem_safe_calloc(size_t count, size_t element_size, const char* file, int line, size_t* statistics = NULL);
void __neko_mem_safe_free(void* mem, size_t* statistics = NULL);
void* __neko_mem_safe_realloc(void* ptr, size_t new_size, const char* file, int line, size_t* statistics = NULL);

int neko_mem_check_leaks(bool detailed);
int neko_mem_bytes_inuse();

typedef struct allocation_metrics {
    u64 total_allocated;
    u64 total_free;
} allocation_metrics;

extern allocation_metrics g_allocation_metrics;

void neko_mem_init(int argc, char* argv[]);
void neko_mem_end();
void neko_mem_rungc();

}  // namespace neko

#endif