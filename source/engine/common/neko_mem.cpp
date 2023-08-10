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

neko_gc_t g_gc;

typedef struct neko_mem_heap_header_t {
    struct neko_mem_heap_header_t *next;
    struct neko_mem_heap_header_t *prev;
    size_t size;
} neko_mem_heap_header_t;

typedef struct neko_mem_alloc_info_t neko_mem_alloc_info_t;
struct neko_mem_alloc_info_t {
    const char *file;
    size_t size;
    int line;

    struct neko_mem_alloc_info_t *next;
    struct neko_mem_alloc_info_t *prev;
};

static neko_mem_alloc_info_t *neko_mem_alloc_head() {
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
void *__neko_mem_safe_alloc(size_t size, const char *file, int line, size_t *statistics) {
    neko_mem_alloc_info_t *mem = (neko_mem_alloc_info_t *)neko_malloc(sizeof(neko_mem_alloc_info_t) + size);

    if (!mem) return 0;

    mem->file = file;
    mem->line = line;
    mem->size = size;
    neko_mem_alloc_info_t *head = neko_mem_alloc_head();
    mem->prev = head;
    mem->next = head->next;
    head->next->prev = mem;
    head->next = mem;

    g_allocation_metrics.total_allocated += size;
    if (NULL != statistics) *statistics += size;

    return mem + 1;
}

void *__neko_mem_safe_calloc(size_t count, size_t element_size, const char *file, int line, size_t *statistics) {
    size_t size = count * element_size;
    void *mem = __neko_mem_safe_alloc(size, file, line, statistics);
    std::memset(mem, 0, size);
    return mem;
}

void __neko_mem_safe_free(void *mem, size_t *statistics) {
    if (!mem) return;

    neko_mem_alloc_info_t *info = (neko_mem_alloc_info_t *)mem - 1;
    info->prev->next = info->next;
    info->next->prev = info->prev;

    g_allocation_metrics.total_free += info->size;
    if (NULL != statistics) *statistics -= info->size;

    neko_free(info);
}

void *__neko_mem_safe_realloc(void *ptr, size_t new_size, const char *file, int line, size_t *statistics) {
    if (new_size == 0) {
        __neko_mem_safe_free(ptr, statistics);  // 如果新大小为 0 则直接释放原内存块并返回 NULL
        return NULL;
    }

    if (ptr == NULL) {
        return __neko_mem_safe_alloc(new_size, file, line, statistics);  // 如果原指针为空 则等同于 alloc
    }

    void *new_ptr = __neko_mem_safe_alloc(new_size, file, line, statistics);  // 分配新大小的内存块

    if (new_ptr != NULL) {
        // 复制旧内存块中的数据到新内存块

        neko_mem_alloc_info_t *info = (neko_mem_alloc_info_t *)ptr - 1;

        size_t old_size = info->size;
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);

        __neko_mem_safe_free(ptr, statistics);  // 释放旧内存块
    }

    return new_ptr;
}

int neko_mem_check_leaks(bool detailed) {
    neko_mem_alloc_info_t *head = neko_mem_alloc_head();
    neko_mem_alloc_info_t *next = head->next;
    int leaks = 0;

    std::size_t leaks_size = 0;

    while (next != head) {
        if (detailed) {
            neko_warn(std::format("LEAKED {0} bytes from file \"{1}\" at line {2} from address {3}.", next->size, neko_fs_get_filename(next->file), next->line, (void *)(next + 1)));
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
    neko_mem_alloc_info_t *head = neko_mem_alloc_head();
    neko_mem_alloc_info_t *next = head->next;
    int bytes = 0;

    while (next != head) {
        bytes += (int)next->size;
        next = next->next;
    }

    return bytes;
}

#else

inline void *__neko_mem_safe_alloc(size_t new_size, char *file, int line) { return NEKO_MALLOC_FUNC(new_size); }

void *__neko_mem_safe_calloc(size_t count, size_t element_size, char *file, int line) { return NEKO_CALLOC_FUNC(count, new_size); }

inline void __neko_mem_safe_free(void *mem) { return NEKO_FREE_FUNC(mem); }

inline int NEKO_CHECK_FOR_LEAKS() { return 0; }
inline int NEKO_BYTES_IN_USE() { return 0; }

#endif

#pragma region GC

static size_t __neko_gc_hash(void *ptr) {
    uintptr_t ad = (uintptr_t)ptr;
    return (size_t)((13 * ad) ^ (ad >> 15));
}

static size_t __neko_gc_probe(neko_gc_t *gc, size_t i, size_t h) {
    long v = i - (h - 1);
    if (v < 0) {
        v = gc->nslots + v;
    }
    return v;
}

static neko_gc_ptr_t *__neko_gc_get_ptr(neko_gc_t *gc, void *ptr) {
    size_t i, j, h;
    i = __neko_gc_hash(ptr) % gc->nslots;
    j = 0;
    while (1) {
        h = gc->items[i].hash;
        if (h == 0 || j > __neko_gc_probe(gc, i, h)) {
            return NULL;
        }
        if (gc->items[i].ptr == ptr) {
            return &gc->items[i];
        }
        i = (i + 1) % gc->nslots;
        j++;
    }
    return NULL;
}

static void __neko_gc_add_ptr(neko_gc_t *gc, void *ptr, size_t size, int flags, void (*dtor)(void *)) {

    neko_gc_ptr_t item, tmp;
    size_t h, p, i, j;

    i = __neko_gc_hash(ptr) % gc->nslots;
    j = 0;

    item.ptr = ptr;
    item.flags = flags;
    item.size = size;
    item.hash = i + 1;
    item.dtor = dtor;

    while (1) {
        h = gc->items[i].hash;
        if (h == 0) {
            gc->items[i] = item;
            return;
        }
        if (gc->items[i].ptr == item.ptr) {
            return;
        }
        p = __neko_gc_probe(gc, i, h);
        if (j >= p) {
            tmp = gc->items[i];
            gc->items[i] = item;
            item = tmp;
            j = p;
        }
        i = (i + 1) % gc->nslots;
        j++;
    }
}

static void __neko_gc_rem_ptr(neko_gc_t *gc, void *ptr) {

    size_t i, j, h, nj, nh;

    if (gc->nitems == 0) {
        return;
    }

    for (i = 0; i < gc->nfrees; i++) {
        if (gc->frees[i].ptr == ptr) {
            gc->frees[i].ptr = NULL;
        }
    }

    i = __neko_gc_hash(ptr) % gc->nslots;
    j = 0;

    while (1) {
        h = gc->items[i].hash;
        if (h == 0 || j > __neko_gc_probe(gc, i, h)) {
            return;
        }
        if (gc->items[i].ptr == ptr) {
            memset(&gc->items[i], 0, sizeof(neko_gc_ptr_t));
            j = i;
            while (1) {
                nj = (j + 1) % gc->nslots;
                nh = gc->items[nj].hash;
                if (nh != 0 && __neko_gc_probe(gc, nj, nh) > 0) {
                    memcpy(&gc->items[j], &gc->items[nj], sizeof(neko_gc_ptr_t));
                    memset(&gc->items[nj], 0, sizeof(neko_gc_ptr_t));
                    j = nj;
                } else {
                    break;
                }
            }
            gc->nitems--;
            return;
        }
        i = (i + 1) % gc->nslots;
        j++;
    }
}

enum { NEKO_GC_PRIMES_COUNT = 24 };

static const size_t __neko_gc_primes[NEKO_GC_PRIMES_COUNT] = {0,    1,    5,     11,    23,    53,     101,    197,    389,     683,     1259,    2417,
                                                              4733, 9371, 18617, 37097, 74093, 148073, 296099, 592019, 1100009, 2200013, 4400021, 8800019};

static size_t __neko_gc_ideal_size(neko_gc_t *gc, size_t size) {
    size_t i, last;
    size = (size_t)((double)(size + 1) / gc->loadfactor);
    for (i = 0; i < NEKO_GC_PRIMES_COUNT; i++) {
        if (__neko_gc_primes[i] >= size) {
            return __neko_gc_primes[i];
        }
    }
    last = __neko_gc_primes[NEKO_GC_PRIMES_COUNT - 1];
    for (i = 0;; i++) {
        if (last * i >= size) {
            return last * i;
        }
    }
    return 0;
}

static int __neko_gc_rehash(neko_gc_t *gc, size_t new_size) {

    size_t i;
    neko_gc_ptr_t *old_items = gc->items;
    size_t old_size = gc->nslots;

    gc->nslots = new_size;
    gc->items = (neko_gc_ptr_t *)calloc(gc->nslots, sizeof(neko_gc_ptr_t));

    if (gc->items == NULL) {
        gc->nslots = old_size;
        gc->items = old_items;
        return 0;
    }

    for (i = 0; i < old_size; i++) {
        if (old_items[i].hash != 0) {
            __neko_gc_add_ptr(gc, old_items[i].ptr, old_items[i].size, old_items[i].flags, old_items[i].dtor);
        }
    }

    free(old_items);

    return 1;
}

static int __neko_gc_resize_more(neko_gc_t *gc) {
    size_t new_size = __neko_gc_ideal_size(gc, gc->nitems);
    size_t old_size = gc->nslots;
    return (new_size > old_size) ? __neko_gc_rehash(gc, new_size) : 1;
}

static int __neko_gc_resize_less(neko_gc_t *gc) {
    size_t new_size = __neko_gc_ideal_size(gc, gc->nitems);
    size_t old_size = gc->nslots;
    return (new_size < old_size) ? __neko_gc_rehash(gc, new_size) : 1;
}

static void __neko_gc_mark_ptr(neko_gc_t *gc, void *ptr) {

    size_t i, j, h, k;

    if ((uintptr_t)ptr < gc->minptr || (uintptr_t)ptr > gc->maxptr) {
        return;
    }

    i = __neko_gc_hash(ptr) % gc->nslots;
    j = 0;

    while (1) {
        h = gc->items[i].hash;
        if (h == 0 || j > __neko_gc_probe(gc, i, h)) {
            return;
        }
        if (ptr == gc->items[i].ptr) {
            if (gc->items[i].flags & NEKO_GC_MARK) {
                return;
            }
            gc->items[i].flags |= NEKO_GC_MARK;
            if (gc->items[i].flags & NEKO_GC_LEAF) {
                return;
            }
            for (k = 0; k < gc->items[i].size / sizeof(void *); k++) {
                __neko_gc_mark_ptr(gc, ((void **)gc->items[i].ptr)[k]);
            }
            return;
        }
        i = (i + 1) % gc->nslots;
        j++;
    }
}

static void __neko_gc_mark_stack(neko_gc_t *gc) {

    void *stk, *bot, *top, *p;
    bot = gc->bottom;
    top = &stk;

    if (bot == top) {
        return;
    }

    if (bot < top) {
        for (p = top; p >= bot; p = ((char *)p) - sizeof(void *)) {
            __neko_gc_mark_ptr(gc, *((void **)p));
        }
    }

    if (bot > top) {
        for (p = top; p <= bot; p = ((char *)p) + sizeof(void *)) {
            __neko_gc_mark_ptr(gc, *((void **)p));
        }
    }
}

static void __neko_gc_mark(neko_gc_t *gc) {

    size_t i, k;
    jmp_buf env;
    void (*volatile mark_stack)(neko_gc_t *) = __neko_gc_mark_stack;

    if (gc->nitems == 0) {
        return;
    }

    for (i = 0; i < gc->nslots; i++) {
        if (gc->items[i].hash == 0) {
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_MARK) {
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_ROOT) {
            gc->items[i].flags |= NEKO_GC_MARK;
            if (gc->items[i].flags & NEKO_GC_LEAF) {
                continue;
            }
            for (k = 0; k < gc->items[i].size / sizeof(void *); k++) {
                __neko_gc_mark_ptr(gc, ((void **)gc->items[i].ptr)[k]);
            }
            continue;
        }
    }

    memset(&env, 0, sizeof(jmp_buf));
    setjmp(env);
    mark_stack(gc);
}

void __neko_gc_sweep(neko_gc_t *gc) {

    size_t i, j, k, nj, nh;

    if (gc->nitems == 0) {
        return;
    }

    gc->nfrees = 0;
    for (i = 0; i < gc->nslots; i++) {
        if (gc->items[i].hash == 0) {
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_MARK) {
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_ROOT) {
            continue;
        }
        gc->nfrees++;
    }

    gc->frees = (neko_gc_ptr_t *)realloc(gc->frees, sizeof(neko_gc_ptr_t) * gc->nfrees);
    if (gc->frees == NULL) {
        return;
    }

    i = 0;
    k = 0;
    while (i < gc->nslots) {
        if (gc->items[i].hash == 0) {
            i++;
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_MARK) {
            i++;
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_ROOT) {
            i++;
            continue;
        }

        gc->frees[k] = gc->items[i];
        k++;
        memset(&gc->items[i], 0, sizeof(neko_gc_ptr_t));

        j = i;
        while (1) {
            nj = (j + 1) % gc->nslots;
            nh = gc->items[nj].hash;
            if (nh != 0 && __neko_gc_probe(gc, nj, nh) > 0) {
                memcpy(&gc->items[j], &gc->items[nj], sizeof(neko_gc_ptr_t));
                memset(&gc->items[nj], 0, sizeof(neko_gc_ptr_t));
                j = nj;
            } else {
                break;
            }
        }
        gc->nitems--;
    }

    for (i = 0; i < gc->nslots; i++) {
        if (gc->items[i].hash == 0) {
            continue;
        }
        if (gc->items[i].flags & NEKO_GC_MARK) {
            gc->items[i].flags &= ~NEKO_GC_MARK;
        }
    }

    __neko_gc_resize_less(gc);

    gc->mitems = gc->nitems + (size_t)(gc->nitems * gc->sweepfactor) + 1;

    for (i = 0; i < gc->nfrees; i++) {
        if (gc->frees[i].ptr) {
            if (gc->frees[i].dtor) {
                gc->frees[i].dtor(gc->frees[i].ptr);
            }
            free(gc->frees[i].ptr);
        }
    }

    free(gc->frees);
    gc->frees = NULL;
    gc->nfrees = 0;
}

void neko_gc_start(neko_gc_t *gc, void *stk) {
    gc->bottom = stk;
    gc->paused = 0;
    gc->nitems = 0;
    gc->nslots = 0;
    gc->mitems = 0;
    gc->nfrees = 0;
    gc->maxptr = 0;
    gc->items = NULL;
    gc->frees = NULL;
    gc->minptr = UINTPTR_MAX;
    gc->loadfactor = 0.9;
    gc->sweepfactor = 0.5;
}

void neko_gc_stop(neko_gc_t *gc) {
    __neko_gc_sweep(gc);
    free(gc->items);
    free(gc->frees);
}

void neko_gc_pause(neko_gc_t *gc) { gc->paused = 1; }

void neko_gc_resume(neko_gc_t *gc) { gc->paused = 0; }

void neko_gc_run(neko_gc_t *gc) {
    __neko_gc_mark(gc);
    __neko_gc_sweep(gc);
}

static void *__neko_gc_add(neko_gc_t *gc, void *ptr, size_t size, int flags, void (*dtor)(void *)) {

    gc->nitems++;
    gc->maxptr = ((uintptr_t)ptr) + size > gc->maxptr ? ((uintptr_t)ptr) + size : gc->maxptr;
    gc->minptr = ((uintptr_t)ptr) < gc->minptr ? ((uintptr_t)ptr) : gc->minptr;

    if (__neko_gc_resize_more(gc)) {
        __neko_gc_add_ptr(gc, ptr, size, flags, dtor);
        if (!gc->paused && gc->nitems > gc->mitems) {
            neko_gc_run(gc);
        }
        return ptr;
    } else {
        gc->nitems--;
        free(ptr);
        return NULL;
    }
}

static void __neko_gc_rem(neko_gc_t *gc, void *ptr) {
    __neko_gc_rem_ptr(gc, ptr);
    __neko_gc_resize_less(gc);
    gc->mitems = gc->nitems + gc->nitems / 2 + 1;
}

void *neko_gc_alloc(neko_gc_t *gc, size_t size) { return __neko_gc_alloc_opt(gc, size, 0, NULL); }

void *neko_gc_calloc(neko_gc_t *gc, size_t num, size_t size) { return __neko_gc_calloc_opt(gc, num, size, 0, NULL); }

void *neko_gc_realloc(neko_gc_t *gc, void *ptr, size_t size) {

    neko_gc_ptr_t *p;
    void *qtr = realloc(ptr, size);

    if (qtr == NULL) {
        __neko_gc_rem(gc, ptr);
        return qtr;
    }

    if (ptr == NULL) {
        __neko_gc_add(gc, qtr, size, 0, NULL);
        return qtr;
    }

    p = __neko_gc_get_ptr(gc, ptr);

    if (p && qtr == ptr) {
        p->size = size;
        return qtr;
    }

    if (p && qtr != ptr) {
        int flags = p->flags;
        void (*dtor)(void *) = p->dtor;
        __neko_gc_rem(gc, ptr);
        __neko_gc_add(gc, qtr, size, flags, dtor);
        return qtr;
    }

    return NULL;
}

void neko_gc_free(neko_gc_t *gc, void *ptr) {
    neko_gc_ptr_t *p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        if (p->dtor) {
            p->dtor(ptr);
        }
        free(ptr);
        __neko_gc_rem(gc, ptr);
    }
}

void *__neko_gc_alloc_opt(neko_gc_t *gc, size_t size, int flags, void (*dtor)(void *)) {
    void *ptr = malloc(size);
    if (ptr != NULL) {
        ptr = __neko_gc_add(gc, ptr, size, flags, dtor);
    }
    return ptr;
}

void *__neko_gc_calloc_opt(neko_gc_t *gc, size_t num, size_t size, int flags, void (*dtor)(void *)) {
    void *ptr = calloc(num, size);
    if (ptr != NULL) {
        ptr = __neko_gc_add(gc, ptr, num * size, flags, dtor);
    }
    return ptr;
}

void neko_gc_set_dtor(neko_gc_t *gc, void *ptr, void (*dtor)(void *)) {
    neko_gc_ptr_t *p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        p->dtor = dtor;
    }
}

void neko_gc_set_flags(neko_gc_t *gc, void *ptr, int flags) {
    neko_gc_ptr_t *p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        p->flags = flags;
    }
}

int neko_gc_get_flags(neko_gc_t *gc, void *ptr) {
    neko_gc_ptr_t *p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        return p->flags;
    }
    return 0;
}

void (*neko_gc_get_dtor(neko_gc_t *gc, void *ptr))(void *) {
    neko_gc_ptr_t *p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        return p->dtor;
    }
    return NULL;
}

size_t neko_gc_get_size(neko_gc_t *gc, void *ptr) {
    neko_gc_ptr_t *p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        return p->size;
    }
    return 0;
}

#pragma endregion GC

void __neko_mem_init(int argc, char **argv) { neko_gc_start(&g_gc, &argc); }

void __neko_mem_end() {
    neko_gc_stop(&g_gc);
    neko_mem_check_leaks(false);
}

void __neko_mem_rungc() { neko_gc_run(&g_gc); }

}  // namespace neko