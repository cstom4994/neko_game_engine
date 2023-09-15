#ifndef NEKO_MEM_H
#define NEKO_MEM_H

#include <type_traits>

#include "engine/common/neko_types.h"

namespace neko {

void *__neko_mem_safe_alloc(size_t size, const char *file, int line, size_t *statistics = NULL);
void *__neko_mem_safe_calloc(size_t count, size_t element_size, const char *file, int line, size_t *statistics = NULL);
void __neko_mem_safe_free(void *mem, size_t *statistics = NULL);
void *__neko_mem_safe_realloc(void *ptr, size_t new_size, const char *file, int line, size_t *statistics = NULL);

#if 1

#ifndef neko_safe_malloc
#define neko_safe_malloc(size) ::neko::__neko_mem_safe_alloc((size), (char *)__FILE__, __LINE__)
#endif

#ifndef neko_safe_free
#define neko_safe_free(mem) ::neko::__neko_mem_safe_free(mem)
#endif

#ifndef neko_safe_realloc
#define neko_safe_realloc(ptr, size) ::neko::__neko_mem_safe_realloc((ptr), (size), (char *)__FILE__, __LINE__)
#endif

#ifndef neko_safe_calloc
#define neko_safe_calloc(count, element_size) ::neko::__neko_mem_safe_calloc(count, element_size, (char *)__FILE__, __LINE__)
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
#define TEST_NEW(_name, _class, ...)     \
    (_class *)ME_MALLOC(sizeof(_class)); \
    new ((void *)_name) _class(__VA_ARGS__)
#endif

template <typename T>
struct alloc {
    template <typename... Args>
    static T *safe_malloc(Args &&...args) {
        void *mem = neko_safe_malloc(sizeof(T));
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

int neko_mem_check_leaks(bool detailed);
int neko_mem_bytes_inuse();

typedef struct neko_allocation_metrics {
    u64 total_allocated;
    u64 total_free;
} neko_allocation_metrics;

extern neko_allocation_metrics g_allocation_metrics;

// GC 具体设计可以见 https://github.com/orangeduck/tgc

// GC 标记枚举
enum { NEKO_GC_MARK = 0x01, NEKO_GC_ROOT = 0x02, NEKO_GC_LEAF = 0x04 };

typedef struct {
    void *ptr;
    int flags;
    size_t size, hash;
    void (*dtor)(void *);
} neko_gc_ptr_t;

typedef struct {
    void *bottom;
    int paused;
    uintptr_t minptr, maxptr;
    neko_gc_ptr_t *items, *frees;
    double loadfactor, sweepfactor;
    size_t nitems, nslots, mitems, nfrees;
} neko_gc_t;

void neko_gc_start(neko_gc_t *gc, void *stk);
void neko_gc_stop(neko_gc_t *gc);
void neko_gc_pause(neko_gc_t *gc);
void neko_gc_resume(neko_gc_t *gc);
void neko_gc_run(neko_gc_t *gc);

// GC 公开函数
void *neko_gc_alloc(neko_gc_t *gc, size_t size);
void *neko_gc_calloc(neko_gc_t *gc, size_t num, size_t size);
void *neko_gc_realloc(neko_gc_t *gc, void *ptr, size_t size);
void neko_gc_free(neko_gc_t *gc, void *ptr);

void *__neko_gc_alloc_opt(neko_gc_t *gc, size_t size, int flags, void (*dtor)(void *));
void *__neko_gc_calloc_opt(neko_gc_t *gc, size_t num, size_t size, int flags, void (*dtor)(void *));

void neko_gc_set_dtor(neko_gc_t *gc, void *ptr, void (*dtor)(void *));
void neko_gc_set_flags(neko_gc_t *gc, void *ptr, int flags);
int neko_gc_get_flags(neko_gc_t *gc, void *ptr);
void (*neko_gc_get_dtor(neko_gc_t *gc, void *ptr))(void *);
size_t neko_gc_get_size(neko_gc_t *gc, void *ptr);

extern neko_gc_t g_gc;

void __neko_mem_init(int argc, char **argv);
void __neko_mem_end();
void __neko_mem_rungc();

}  // namespace neko

#endif