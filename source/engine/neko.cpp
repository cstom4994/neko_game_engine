
#include "neko.h"

#include <csetjmp>
#include <format>

#include "engine/neko_component.h"
#include "engine/neko_engine.h"

// Use discrete GPU by default.
extern "C" {
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
__declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 0x00000001;
}

// Resource Creation
NEKO_API_DECL neko_handle(neko_graphics_texture_t) neko_graphics_texture_create(const neko_graphics_texture_desc_t* desc) { return neko_graphics()->api.texture_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_uniform_t) neko_graphics_uniform_create(const neko_graphics_uniform_desc_t* desc) { return neko_graphics()->api.uniform_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_shader_t) neko_graphics_shader_create(const neko_graphics_shader_desc_t* desc) { return neko_graphics()->api.shader_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_vertex_buffer_t) neko_graphics_vertex_buffer_create(const neko_graphics_vertex_buffer_desc_t* desc) { return neko_graphics()->api.vertex_buffer_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_index_buffer_t) neko_graphics_index_buffer_create(const neko_graphics_index_buffer_desc_t* desc) { return neko_graphics()->api.index_buffer_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_uniform_buffer_t) neko_graphics_uniform_buffer_create(const neko_graphics_uniform_buffer_desc_t* desc) {
    return neko_graphics()->api.uniform_buffer_create(desc);
}

NEKO_API_DECL neko_handle(neko_graphics_storage_buffer_t) neko_graphics_storage_buffer_create(const neko_graphics_storage_buffer_desc_t* desc) {
    return neko_graphics()->api.storage_buffer_create(desc);
}

NEKO_API_DECL neko_handle(neko_graphics_framebuffer_t) neko_graphics_framebuffer_create(const neko_graphics_framebuffer_desc_t* desc) { return neko_graphics()->api.framebuffer_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_renderpass_t) neko_graphics_renderpass_create(const neko_graphics_renderpass_desc_t* desc) { return neko_graphics()->api.renderpass_create(desc); }

NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_graphics_pipeline_create(const neko_graphics_pipeline_desc_t* desc) { return neko_graphics()->api.pipeline_create(desc); }

// Destroy
NEKO_API_DECL void neko_graphics_texture_destroy(neko_handle(neko_graphics_texture_t) hndl) { return neko_graphics()->api.texture_destroy(hndl); }

NEKO_API_DECL void neko_graphics_uniform_destroy(neko_handle(neko_graphics_uniform_t) hndl) { return neko_graphics()->api.uniform_destroy(hndl); }

NEKO_API_DECL void neko_graphics_shader_destroy(neko_handle(neko_graphics_shader_t) hndl) { return neko_graphics()->api.shader_destroy(hndl); }

NEKO_API_DECL void neko_graphics_vertex_buffer_destroy(neko_handle(neko_graphics_vertex_buffer_t) hndl) { return neko_graphics()->api.vertex_buffer_destroy(hndl); }

NEKO_API_DECL void neko_graphics_index_buffer_destroy(neko_handle(neko_graphics_index_buffer_t) hndl) { return neko_graphics()->api.index_buffer_destroy(hndl); }

NEKO_API_DECL void neko_graphics_uniform_buffer_destroy(neko_handle(neko_graphics_uniform_buffer_t) hndl) { return neko_graphics()->api.uniform_buffer_destroy(hndl); }

NEKO_API_DECL void neko_graphics_storage_buffer_destroy(neko_handle(neko_graphics_storage_buffer_t) hndl) { return neko_graphics()->api.storage_buffer_destroy(hndl); }

NEKO_API_DECL void neko_graphics_framebuffer_destroy(neko_handle(neko_graphics_framebuffer_t) hndl) { return neko_graphics()->api.framebuffer_destroy(hndl); }

NEKO_API_DECL void neko_graphics_renderpass_destroy(neko_handle(neko_graphics_renderpass_t) hndl) { return neko_graphics()->api.renderpass_destroy(hndl); }

NEKO_API_DECL void neko_graphics_pipeline_destroy(neko_handle(neko_graphics_pipeline_t) hndl) { return neko_graphics()->api.pipeline_destroy(hndl); }

// Resource Updates (main thread only)
NEKO_API_DECL void neko_graphics_vertex_buffer_update(neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc) {
    return neko_graphics()->api.vertex_buffer_update(hndl, desc);
}

NEKO_API_DECL void neko_graphics_index_buffer_update(neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc) {
    return neko_graphics()->api.index_buffer_update(hndl, desc);
}

NEKO_API_DECL void neko_graphics_storage_buffer_update(neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc) {
    return neko_graphics()->api.storage_buffer_update(hndl, desc);
}

NEKO_API_DECL void neko_graphics_texture_update(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) { return neko_graphics()->api.texture_update(hndl, desc); }

NEKO_API_DECL void neko_graphics_texture_read(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) { return neko_graphics()->api.texture_read(hndl, desc); }

/*==========================
// NEKO_OS
==========================*/

NEKO_API_DECL
void* _neko_malloc_init_impl(size_t sz) {
    void* data = neko_malloc(sz);
    memset(data, 0, sz);
    return data;
}

NEKO_API_DECL neko_os_api_t neko_os_api_new_default() {
    neko_os_api_t os = neko_default_val();
    os.malloc = malloc;
    os.malloc_init = _neko_malloc_init_impl;
    os.free = free;
    os.realloc = realloc;
    os.calloc = calloc;
    os.strdup = strdup;
    return os;
}

// logging

#define MAX_CALLBACKS 32
#define LOG_USE_COLOR

typedef struct {
    neko_log_fn fn;
    void* udata;
    int level;
} Callback;

static struct {
    void* udata;
    neko_log_lock_fn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;

static const char* level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static const char* level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"};

static void stdout_callback(neko_log_event* ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf, level_colors[ev->level], level_strings[ev->level], neko_fs_get_filename(ev->file), ev->line);
#else
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void file_callback(neko_log_event* ev) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void lock(void) {
    if (L.lock) {
        L.lock(true, L.udata);
    }
}

static void unlock(void) {
    if (L.lock) {
        L.lock(false, L.udata);
    }
}

const char* log_level_string(int level) { return level_strings[level]; }

void log_set_lock(neko_log_lock_fn fn, void* udata) {
    L.lock = fn;
    L.udata = udata;
}

void log_set_level(int level) { L.level = level; }

void log_set_quiet(bool enable) { L.quiet = enable; }

int log_add_callback(neko_log_fn fn, void* udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = Callback{fn, udata, level};
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE* fp, int level) { return log_add_callback(file_callback, fp, level); }

static void init_event(neko_log_event* ev, void* udata) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->udata = (FILE*)udata;
}

void log_log(int level, const char* file, int line, const char* fmt, ...) {
    neko_log_event ev = {
            .fmt = fmt,
            .file = file,
            .line = line,
            .level = level,
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback* cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}

/*========================
// neko_byte_buffer
========================*/

void neko_byte_buffer_init(neko_byte_buffer_t* buffer) {
    buffer->data = (u8*)neko_safe_malloc(NEKO_BYTE_BUFFER_DEFAULT_CAPCITY);
    buffer->capacity = NEKO_BYTE_BUFFER_DEFAULT_CAPCITY;
    buffer->size = 0;
    buffer->position = 0;
}

neko_byte_buffer_t neko_byte_buffer_new() {
    neko_byte_buffer_t buffer;
    neko_byte_buffer_init(&buffer);
    return buffer;
}

void neko_byte_buffer_free(neko_byte_buffer_t* buffer) {
    if (buffer && buffer->data) {
        neko_safe_free(buffer->data);
    }
}

void neko_byte_buffer_clear(neko_byte_buffer_t* buffer) {
    buffer->size = 0;
    buffer->position = 0;
}

bool neko_byte_buffer_empty(neko_byte_buffer_t* buffer) { return (buffer->size == 0); }

size_t neko_byte_buffer_size(neko_byte_buffer_t* buffer) { return buffer->size; }

void neko_byte_buffer_resize(neko_byte_buffer_t* buffer, size_t sz) {
    u8* data = (u8*)neko_safe_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;
    buffer->capacity = (u32)sz;
}

void neko_byte_buffer_copy_contents(neko_byte_buffer_t* dst, neko_byte_buffer_t* src) {
    neko_byte_buffer_seek_to_beg(dst);
    neko_byte_buffer_seek_to_beg(src);
    neko_byte_buffer_write_bulk(dst, src->data, src->size);
}

void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t* buffer) { buffer->position = 0; }

void neko_byte_buffer_seek_to_end(neko_byte_buffer_t* buffer) { buffer->position = buffer->size; }

void neko_byte_buffer_advance_position(neko_byte_buffer_t* buffer, size_t sz) { buffer->position += (u32)sz; }

void neko_byte_buffer_write_bulk(neko_byte_buffer_t* buffer, void* src, size_t size) {
    // Check for necessary resize
    size_t total_write_size = buffer->position + size;
    if (total_write_size >= (size_t)buffer->capacity) {
        size_t capacity = buffer->capacity * 2;
        while (capacity <= total_write_size) {
            capacity *= 2;
        }

        neko_byte_buffer_resize(buffer, capacity);
    }

    // memcpy data
    memcpy((buffer->data + buffer->position), src, size);

    buffer->size += (u32)size;
    buffer->position += (u32)size;
}

void neko_byte_buffer_read_bulk(neko_byte_buffer_t* buffer, void** dst, size_t size) {
    memcpy(*dst, (buffer->data + buffer->position), size);
    buffer->position += (u32)size;
}

void neko_byte_buffer_write_str(neko_byte_buffer_t* buffer, const char* str) {
    // Write size of string
    u32 str_len = neko_string_length(str);
    neko_byte_buffer_write(buffer, uint16_t, str_len);

    size_t i;
    for (i = 0; i < str_len; ++i) {
        neko_byte_buffer_write(buffer, u8, str[i]);
    }
}

void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str) {
    // Read in size of string from buffer
    uint16_t sz;
    neko_byte_buffer_read(buffer, uint16_t, &sz);

    u32 i;
    for (i = 0; i < sz; ++i) {
        neko_byte_buffer_read(buffer, u8, &str[i]);
    }
    str[i] = '\0';
}

neko_result neko_byte_buffer_write_to_file(neko_byte_buffer_t* buffer, const char* output_path) { return neko_platform_write_file_contents(output_path, "wb", buffer->data, buffer->size); }

neko_result neko_byte_buffer_read_from_file(neko_byte_buffer_t* buffer, const char* file_path) {
    if (!buffer) return NEKO_RESULT_FAILURE;

    if (buffer->data) {
        neko_byte_buffer_free(buffer);
    }

    buffer->data = (u8*)neko_platform_read_file_contents(file_path, "rb", (size_t*)&buffer->size);
    if (!buffer->data) {
        neko_assert(false);
        return NEKO_RESULT_FAILURE;
    }

    buffer->position = 0;
    buffer->capacity = buffer->size;
    return NEKO_RESULT_SUCCESS;
}

NEKO_API_DECL void neko_byte_buffer_memset(neko_byte_buffer_t* buffer, u8 val) { memset(buffer->data, val, buffer->capacity); }

/*========================
// Dynamic Array
========================*/

NEKO_API_DECL void* neko_dyn_array_resize_impl(void* arr, size_t sz, size_t amount) {
    size_t capacity;

    if (arr) {
        capacity = amount;
    } else {
        capacity = 0;
    }

    // Create new neko_dyn_array with just the header information
    neko_dyn_array* data = (neko_dyn_array*)neko_realloc(arr ? neko_dyn_array_head(arr) : 0, capacity * sz + sizeof(neko_dyn_array));

    if (data) {
        if (!arr) {
            data->size = 0;
        }
        data->capacity = (s32)capacity;
        return ((s32*)data + 2);
    }

    return NULL;
}

NEKO_API_DECL void** neko_dyn_array_init(void** arr, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array* data = (neko_dyn_array*)neko_malloc(val_len + sizeof(neko_dyn_array));  // Allocate capacity of one
        data->size = 0;
        data->capacity = 1;
        *arr = ((s32*)data + 2);
    }
    return arr;
}

NEKO_API_DECL void neko_dyn_array_push_data(void** arr, void* val, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array_init(arr, val_len);
    }
    if (neko_dyn_array_need_grow(*arr, 1)) {
        s32 capacity = neko_dyn_array_capacity(*arr) * 2;

        // Create new neko_dyn_array with just the header information
        neko_dyn_array* data = (neko_dyn_array*)neko_realloc(neko_dyn_array_head(*arr), capacity * val_len + sizeof(neko_dyn_array));

        if (data) {
            data->capacity = capacity;
            *arr = ((s32*)data + 2);
        }
    }
    size_t offset = neko_dyn_array_size(*arr);
    memcpy(((u8*)(*arr)) + offset * val_len, val, val_len);
    neko_dyn_array_head(*arr)->size++;
}

/*========================
// Hash Table
========================*/

NEKO_API_DECL void __neko_hash_table_init_impl(void** ht, size_t sz) { *ht = neko_malloc(sz); }

/*========================
// Slot Array
========================*/

NEKO_API_DECL void** neko_slot_array_init(void** sa, size_t sz) {
    if (*sa == NULL) {
        *sa = neko_malloc(sz);
        memset(*sa, 0, sz);
        return sa;
    } else {
        return NULL;
    }
}

/*========================
// Slot Map
========================*/

NEKO_API_DECL void** neko_slot_map_init(void** sm) {
    if (*sm == NULL) {
        (*sm) = neko_malloc(sizeof(size_t) * 2);
        memset((*sm), 0, sizeof(size_t) * 2);
        return sm;
    }
    return NULL;
}

/*========================
// NEKO_MEMORY
========================*/

neko_allocation_metrics g_allocation_metrics = {.total_allocated = 0, .total_free = 0};

neko_gc_t g_gc;

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
void* __neko_mem_safe_alloc(size_t size, const char* file, int line, size_t* statistics) {
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

void* __neko_mem_safe_calloc(size_t count, size_t element_size, const char* file, int line, size_t* statistics) {
    size_t size = count * element_size;
    void* mem = __neko_mem_safe_alloc(size, file, line, statistics);
    std::memset(mem, 0, size);
    return mem;
}

void __neko_mem_safe_free(void* mem, size_t* statistics) {
    if (!mem) return;

    neko_mem_alloc_info_t* info = (neko_mem_alloc_info_t*)mem - 1;
    info->prev->next = info->next;
    info->next->prev = info->prev;

    g_allocation_metrics.total_free += info->size;
    if (NULL != statistics) *statistics -= info->size;

    neko_free(info);
}

void* __neko_mem_safe_realloc(void* ptr, size_t new_size, const char* file, int line, size_t* statistics) {
    if (new_size == 0) {
        __neko_mem_safe_free(ptr, statistics);  // 如果新大小为 0 则直接释放原内存块并返回 NULL
        return NULL;
    }

    if (ptr == NULL) {
        return __neko_mem_safe_alloc(new_size, file, line, statistics);  // 如果原指针为空 则等同于 alloc
    }

    void* new_ptr = __neko_mem_safe_alloc(new_size, file, line, statistics);  // 分配新大小的内存块

    if (new_ptr != NULL) {
        // 复制旧内存块中的数据到新内存块

        neko_mem_alloc_info_t* info = (neko_mem_alloc_info_t*)ptr - 1;

        size_t old_size = info->size;
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);

        __neko_mem_safe_free(ptr, statistics);  // 释放旧内存块
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
            neko_log_warning(std::format("LEAKED {0} bytes from file \"{1}\" at line {2} from address {3}.", next->size, neko_fs_get_filename(next->file), next->line, (void*)(next + 1)).c_str());
        }
        leaks_size += next->size;
        next = next->next;
        leaks = 1;
    }

    if (leaks && detailed) {
        neko_log_info("memory leaks detected (see above).");
    }
    if (leaks) {
        double megabytes = neko_s_cast<double>(leaks_size) / 1048576;
        neko_log_warning(std::format("memory leaks detected with {0} bytes equal to {1:.4f} MB.", leaks_size, megabytes).c_str());
    } else {
        neko_log_info("no memory leaks detected.");
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

inline void* __neko_mem_safe_alloc(size_t new_size, char* file, int line) { return NEKO_MALLOC_FUNC(new_size); }

void* __neko_mem_safe_calloc(size_t count, size_t element_size, char* file, int line) { return NEKO_CALLOC_FUNC(count, new_size); }

inline void __neko_mem_safe_free(void* mem) { return NEKO_FREE_FUNC(mem); }

inline int NEKO_CHECK_FOR_LEAKS() { return 0; }
inline int NEKO_BYTES_IN_USE() { return 0; }

#endif

#pragma region GC

static size_t __neko_gc_hash(void* ptr) {
    uintptr_t ad = (uintptr_t)ptr;
    return (size_t)((13 * ad) ^ (ad >> 15));
}

static size_t __neko_gc_probe(neko_gc_t* gc, size_t i, size_t h) {
    long v = i - (h - 1);
    if (v < 0) {
        v = gc->nslots + v;
    }
    return v;
}

static neko_gc_ptr_t* __neko_gc_get_ptr(neko_gc_t* gc, void* ptr) {
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

static void __neko_gc_add_ptr(neko_gc_t* gc, void* ptr, size_t size, int flags, void (*dtor)(void*)) {

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

static void __neko_gc_rem_ptr(neko_gc_t* gc, void* ptr) {

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

static size_t __neko_gc_ideal_size(neko_gc_t* gc, size_t size) {
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

static int __neko_gc_rehash(neko_gc_t* gc, size_t new_size) {

    size_t i;
    neko_gc_ptr_t* old_items = gc->items;
    size_t old_size = gc->nslots;

    gc->nslots = new_size;
    gc->items = (neko_gc_ptr_t*)calloc(gc->nslots, sizeof(neko_gc_ptr_t));

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

static int __neko_gc_resize_more(neko_gc_t* gc) {
    size_t new_size = __neko_gc_ideal_size(gc, gc->nitems);
    size_t old_size = gc->nslots;
    return (new_size > old_size) ? __neko_gc_rehash(gc, new_size) : 1;
}

static int __neko_gc_resize_less(neko_gc_t* gc) {
    size_t new_size = __neko_gc_ideal_size(gc, gc->nitems);
    size_t old_size = gc->nslots;
    return (new_size < old_size) ? __neko_gc_rehash(gc, new_size) : 1;
}

static void __neko_gc_mark_ptr(neko_gc_t* gc, void* ptr) {

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
            for (k = 0; k < gc->items[i].size / sizeof(void*); k++) {
                __neko_gc_mark_ptr(gc, ((void**)gc->items[i].ptr)[k]);
            }
            return;
        }
        i = (i + 1) % gc->nslots;
        j++;
    }
}

static void __neko_gc_mark_stack(neko_gc_t* gc) {

    void *stk, *bot, *top, *p;
    bot = gc->bottom;
    top = &stk;

    if (bot == top) {
        return;
    }

    if (bot < top) {
        for (p = top; p >= bot; p = ((char*)p) - sizeof(void*)) {
            __neko_gc_mark_ptr(gc, *((void**)p));
        }
    }

    if (bot > top) {
        for (p = top; p <= bot; p = ((char*)p) + sizeof(void*)) {
            __neko_gc_mark_ptr(gc, *((void**)p));
        }
    }
}

static void __neko_gc_mark(neko_gc_t* gc) {

    size_t i, k;
    jmp_buf env;
    void (*volatile mark_stack)(neko_gc_t*) = __neko_gc_mark_stack;

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
            for (k = 0; k < gc->items[i].size / sizeof(void*); k++) {
                __neko_gc_mark_ptr(gc, ((void**)gc->items[i].ptr)[k]);
            }
            continue;
        }
    }

    memset(&env, 0, sizeof(jmp_buf));
    setjmp(env);
    mark_stack(gc);
}

void __neko_gc_sweep(neko_gc_t* gc) {

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

    gc->frees = (neko_gc_ptr_t*)realloc(gc->frees, sizeof(neko_gc_ptr_t) * gc->nfrees);
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

void neko_gc_start(neko_gc_t* gc, void* stk) {
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

void neko_gc_stop(neko_gc_t* gc) {
    __neko_gc_sweep(gc);
    free(gc->items);
    free(gc->frees);
}

void neko_gc_pause(neko_gc_t* gc) { gc->paused = 1; }

void neko_gc_resume(neko_gc_t* gc) { gc->paused = 0; }

void neko_gc_run(neko_gc_t* gc) {
    __neko_gc_mark(gc);
    __neko_gc_sweep(gc);
}

static void* __neko_gc_add(neko_gc_t* gc, void* ptr, size_t size, int flags, void (*dtor)(void*)) {

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

static void __neko_gc_rem(neko_gc_t* gc, void* ptr) {
    __neko_gc_rem_ptr(gc, ptr);
    __neko_gc_resize_less(gc);
    gc->mitems = gc->nitems + gc->nitems / 2 + 1;
}

void* neko_gc_alloc(neko_gc_t* gc, size_t size) { return __neko_gc_alloc_opt(gc, size, 0, NULL); }

void* neko_gc_calloc(neko_gc_t* gc, size_t num, size_t size) { return __neko_gc_calloc_opt(gc, num, size, 0, NULL); }

void* neko_gc_realloc(neko_gc_t* gc, void* ptr, size_t size) {

    neko_gc_ptr_t* p;
    void* qtr = realloc(ptr, size);

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
        void (*dtor)(void*) = p->dtor;
        __neko_gc_rem(gc, ptr);
        __neko_gc_add(gc, qtr, size, flags, dtor);
        return qtr;
    }

    return NULL;
}

void neko_gc_free(neko_gc_t* gc, void* ptr) {
    neko_gc_ptr_t* p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        if (p->dtor) {
            p->dtor(ptr);
        }
        free(ptr);
        __neko_gc_rem(gc, ptr);
    }
}

void* __neko_gc_alloc_opt(neko_gc_t* gc, size_t size, int flags, void (*dtor)(void*)) {
    void* ptr = malloc(size);
    if (ptr != NULL) {
        ptr = __neko_gc_add(gc, ptr, size, flags, dtor);
    }
    return ptr;
}

void* __neko_gc_calloc_opt(neko_gc_t* gc, size_t num, size_t size, int flags, void (*dtor)(void*)) {
    void* ptr = calloc(num, size);
    if (ptr != NULL) {
        ptr = __neko_gc_add(gc, ptr, num * size, flags, dtor);
    }
    return ptr;
}

void neko_gc_set_dtor(neko_gc_t* gc, void* ptr, void (*dtor)(void*)) {
    neko_gc_ptr_t* p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        p->dtor = dtor;
    }
}

void neko_gc_set_flags(neko_gc_t* gc, void* ptr, int flags) {
    neko_gc_ptr_t* p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        p->flags = flags;
    }
}

int neko_gc_get_flags(neko_gc_t* gc, void* ptr) {
    neko_gc_ptr_t* p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        return p->flags;
    }
    return 0;
}

void (*neko_gc_get_dtor(neko_gc_t* gc, void* ptr))(void*) {
    neko_gc_ptr_t* p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        return p->dtor;
    }
    return NULL;
}

size_t neko_gc_get_size(neko_gc_t* gc, void* ptr) {
    neko_gc_ptr_t* p = __neko_gc_get_ptr(gc, ptr);
    if (p) {
        return p->size;
    }
    return 0;
}

#pragma endregion GC

void __neko_mem_init(int argc, char** argv) { neko_gc_start(&g_gc, &argc); }

void __neko_mem_end() {
    neko_gc_stop(&g_gc);
    neko_mem_check_leaks(false);
}

void __neko_mem_rungc() { neko_gc_run(&g_gc); }

// typedef struct neko_memory_block_t {
//     u8* data;
//     size_t size;
// } neko_memory_block_t;

NEKO_API_DECL neko_memory_block_t neko_memory_block_new(size_t sz) {
    neko_memory_block_t mem = neko_default_val();
    mem.data = (u8*)neko_malloc(sz);
    neko_assert(mem.data);
    memset(mem.data, 0, sz);
    mem.size = sz;
    return mem;
}

NEKO_API_DECL void neko_memory_block_free(neko_memory_block_t* mem) {
    neko_assert(mem);
    neko_assert(mem->data);
    neko_free(mem->data);
    mem->data = NULL;
    mem->size = 0;
}

// Modified from: https://github.com/mtrebi/memory-allocators/blob/master/includes/Utils.h
NEKO_API_DECL size_t neko_memory_calc_padding(size_t base_address, size_t alignment) {
    size_t mult = (base_address / alignment) + 1;
    size_t aligned_addr = mult * alignment;
    size_t padding = aligned_addr - base_address;
    return padding;
}

NEKO_API_DECL size_t neko_memory_calc_padding_w_header(size_t base_address, size_t alignment, size_t header_sz) {
    size_t padding = neko_memory_calc_padding(base_address, alignment);
    size_t needed_space = header_sz;

    if (padding < needed_space) {
        needed_space -= padding;

        if (needed_space % alignment > 0) {
            padding += alignment * (1 + (needed_space / alignment));
        } else {
            padding += alignment * (needed_space / alignment);
        }
    }

    return padding;
}

/*================================================================================
// Linear Allocator
================================================================================*/

// typedef struct neko_linear_allocator_t {
//     u8* memory;
//     size_t total_size;
//     size_t offset;
// } neko_linear_allocator_t;

NEKO_API_DECL neko_linear_allocator_t neko_linear_allocator_new(size_t sz) {
    neko_linear_allocator_t la = neko_default_val();
    la.memory = (u8*)neko_malloc(sz);
    memset(la.memory, 0, sz);
    la.offset = 0;
    la.total_size = sz;
    return la;
}

NEKO_API_DECL void neko_linear_allocator_free(neko_linear_allocator_t* la) {
    neko_assert(la);
    neko_assert(la->memory);
    neko_free(la->memory);
    la->memory = NULL;
}

NEKO_API_DECL void* neko_linear_allocator_allocate(neko_linear_allocator_t* la, size_t sz, size_t alignment) {
    neko_assert(la);
    size_t padding = 0;
    size_t padding_address = 0;
    size_t cur_address = (size_t)la->memory + la->offset;

    // Calculate alignment required
    if (alignment != 0 && la->offset % alignment != 0) {
        padding = neko_memory_calc_padding(cur_address, alignment);
    }

    // Cannot allocate (not enough memory available)
    if (la->offset + padding + sz > la->total_size) {
        return NULL;
    }

    // Allocate and return pointer
    la->offset += padding;
    size_t next_address = cur_address + padding;
    la->offset += sz;
    return (void*)next_address;
}

NEKO_API_DECL void neko_linear_allocator_clear(neko_linear_allocator_t* la) {
    neko_assert(la);
    la->offset = 0;
}

/*================================================================================
// Stack Allocator
================================================================================*/

NEKO_API_DECL neko_stack_allocator_t neko_stack_allocator_new(size_t sz) {
    neko_stack_allocator_t alloc = neko_default_val();
    alloc.memory = neko_memory_block_new(sz);
    return alloc;
}

NEKO_API_DECL void neko_stack_allocator_free(neko_stack_allocator_t* sa) {
    neko_stack_allocator_clear(sa);
    neko_memory_block_free(&sa->memory);
}

NEKO_API_DECL void* neko_stack_allocator_allocate(neko_stack_allocator_t* sa, size_t sz) {
    // Not enough memory available
    size_t total_size = sz + sizeof(neko_stack_allocator_header_t);
    if (total_size > (size_t)sa->memory.size - sa->offset) {
        return NULL;
    }

    // Create new entry and push
    size_t header_addr = (size_t)(sa->memory.data + sa->offset + sz);
    neko_stack_allocator_header_t* header = (neko_stack_allocator_header_t*)(sa->memory.data + sa->offset + sz);
    u8* data = (u8*)(sa->memory.data + sa->offset);
    header->size = (u32)sz;

    // Add this to the memory size
    sa->offset += total_size;

    // Return data
    return data;
}

NEKO_API_DECL void* neko_stack_allocator_pop(neko_stack_allocator_t* sa) {
    // If no entries left, then cannot pop
    if (sa->offset == 0) {
        return NULL;
    }

    // Move current size back
    neko_stack_allocator_header_t* header = (neko_stack_allocator_header_t*)(sa->memory.data + sa->offset - sizeof(neko_stack_allocator_header_t));
    void* data = (u8*)(sa->memory.data + sa->offset - sizeof(neko_stack_allocator_header_t) - header->size);
    size_t total_sz = (size_t)header->size + sizeof(neko_stack_allocator_header_t);

    // Set offset back
    sa->offset -= total_sz;

    // Return data
    return (void*)data;
}

NEKO_API_DECL void* neko_stack_allocator_peek(neko_stack_allocator_t* sa) {
    if (sa->offset == 0) {
        return NULL;
    }

    neko_stack_allocator_header_t* header = (neko_stack_allocator_header_t*)(sa->memory.data + sa->offset - sizeof(neko_stack_allocator_header_t));
    return (void*)(sa->memory.data + sa->offset - sizeof(neko_stack_allocator_header_t) - (size_t)header->size);
}

NEKO_API_DECL void neko_stack_allocator_clear(neko_stack_allocator_t* sa) {
    // Clear offset
    sa->offset = 0;
}

/*================================================================================
// Paged Allocator
================================================================================*/

NEKO_API_DECL neko_paged_allocator_t neko_paged_allocator_new(size_t block_size, size_t blocks_per_page) {
    neko_paged_allocator_t pa = neko_default_val();
    pa.block_size = block_size;
    pa.blocks_per_page = blocks_per_page;
    pa.pages = NULL;
    pa.page_count = 0;
    pa.free_list = NULL;
    return pa;
}

NEKO_API_DECL void neko_paged_allocator_free(neko_paged_allocator_t* pa) { neko_paged_allocator_clear(pa); }

NEKO_API_DECL void* neko_paged_allocator_allocate(neko_paged_allocator_t* pa) {
    if (pa->free_list) {
        neko_paged_allocator_block_t* data = pa->free_list;
        pa->free_list = data->next;
        return data;
    } else {
        neko_paged_allocator_page_t* page = (neko_paged_allocator_page_t*)_neko_malloc_init_impl(pa->block_size * pa->blocks_per_page + sizeof(neko_paged_allocator_page_t));
        pa->page_count++;

        page->next = pa->pages;
        page->data = (neko_paged_allocator_block_t*)neko_ptr_add(page, sizeof(neko_paged_allocator_page_t));
        pa->pages = page;

        // #define neko_ptr_add(P, BYTES) \
//     (((u8*)P + (BYTES)))

        u32 bppmo = pa->blocks_per_page - 1;
        for (u32 i = 0; i < bppmo; ++i) {
            neko_paged_allocator_block_t* node = (neko_paged_allocator_block_t*)neko_ptr_add(page->data, pa->block_size * i);
            neko_paged_allocator_block_t* next = (neko_paged_allocator_block_t*)neko_ptr_add(page->data, pa->block_size * (i + 1));
            node->next = next;
        }

        neko_paged_allocator_block_t* last = (neko_paged_allocator_block_t*)neko_ptr_add(page->data, pa->block_size * bppmo);
        last->next = NULL;

        pa->free_list = page->data->next;
        return page->data;
    }
}

NEKO_API_DECL void neko_paged_allocator_deallocate(neko_paged_allocator_t* pa, void* data) {
    ((neko_paged_allocator_block_t*)data)->next = pa->free_list;
    pa->free_list = ((neko_paged_allocator_block_t*)data);
}

NEKO_API_DECL void neko_paged_allocator_clear(neko_paged_allocator_t* pa) {
    neko_paged_allocator_page_t* page = pa->pages;
    for (u32 i = 0; i < pa->page_count; ++i) {
        neko_paged_allocator_page_t* next = page->next;
        neko_free(page);
        page = next;
    }
    pa->free_list = NULL;
    pa->page_count = 0;
}

/*================================================================================
// Heap Allocator
================================================================================*/

// #ifndef NEKO_HEAP_ALLOC_DEFAULT_SIZE
//     #define NEKO_HEAP_ALLOC_DEFAULT_SIZE 1024 * 1024 * 20
// #endif

// #ifndef NEKO_HEAP_ALLOC_DEFAULT_CAPCITY
//     #define NEKO_HEAP_ALLOC_DEFAULT_CAPCITY 1024
// #endif

// typedef struct neko_heap_allocator_header_t {
//     struct neko_heap_allocator_header_t* next;
//     struct neko_heap_allocator_header_t* prev;
//     size_t size;
// } neko_heap_allocator_header_t;

// typedef struct neko_heap_allocator_free_block_t {
//     neko_heap_allocator_header_t* header;
//     size_t size;
// } neko_heap_allocator_free_block_t;

// typedef struct neko_heap_allocator_t {
//     neko_heap_allocator_header_t* memory;
//     neko_heap_allocator_free_block_t* free_blocks;
//     u32 free_block_count;
//     u32 free_block_capacity;
// } neko_heap_allocator_t;

NEKO_API_DECL neko_heap_allocator_t neko_heap_allocate_new() {
    neko_heap_allocator_t ha = neko_default_val();
    ha.memory = (neko_heap_allocator_header_t*)_neko_malloc_init_impl(NEKO_HEAP_ALLOC_DEFAULT_SIZE);
    ha.memory->next = NULL;
    ha.memory->prev = NULL;
    ha.memory->size = NEKO_HEAP_ALLOC_DEFAULT_SIZE;

    ha.free_blocks = (neko_heap_allocator_free_block_t*)_neko_malloc_init_impl(sizeof(neko_heap_allocator_free_block_t) * NEKO_HEAP_ALLOC_DEFAULT_CAPCITY);
    ha.free_block_count = 1;
    ha.free_block_capacity = NEKO_HEAP_ALLOC_DEFAULT_CAPCITY;

    ha.free_blocks->header = ha.memory;
    ha.free_blocks->size = NEKO_HEAP_ALLOC_DEFAULT_SIZE;

    return ha;
}

NEKO_API_DECL void neko_heap_allocator_free(neko_heap_allocator_t* ha) {
    neko_free(ha->memory);
    neko_free(ha->free_blocks);
    ha->memory = NULL;
    ha->free_blocks = NULL;
}

NEKO_API_DECL void* neko_heap_allocator_allocate(neko_heap_allocator_t* ha, size_t sz) {
    size_t size_needed = sz + sizeof(neko_heap_allocator_header_t);
    neko_heap_allocator_free_block_t* first_fit = NULL;

    for (u32 i = 0; i < ha->free_block_count; ++i) {
        neko_heap_allocator_free_block_t* block = ha->free_blocks + i;
        if (block->size >= size_needed) {
            first_fit = block;
            break;
        }
    }

    if (!first_fit) {
        return NULL;
    }

    neko_heap_allocator_header_t* node = first_fit->header;
    neko_heap_allocator_header_t* new_node = (neko_heap_allocator_header_t*)neko_ptr_add(node, size_needed);
    node->size = size_needed;

    first_fit->size -= size_needed;
    first_fit->header = new_node;

    new_node->next = node->next;
    if (node->next) {
        node->next->prev = new_node;
    }
    node->next = new_node;
    new_node->prev = node;

    return neko_ptr_add(node, sizeof(neko_heap_allocator_header_t));
}

NEKO_API_DECL void neko_heap_allocator_deallocate(neko_heap_allocator_t* ha, void* memory) {
    // Fill this out...
}

/*========================
// Random
========================*/

/*=============================
// Camera
=============================*/

NEKO_API_DECL neko_camera_t neko_camera_default() {
    // Construct default camera parameters
    neko_camera_t cam = neko_default_val();
    cam.transform = neko_vqs_default();
    cam.transform.position.z = 1.f;
    cam.fov = 60.f;
    cam.near_plane = 0.1f;
    cam.far_plane = 1000.f;
    cam.ortho_scale = 1.f;
    cam.proj_type = NEKO_PROJECTION_TYPE_ORTHOGRAPHIC;
    return cam;
}

NEKO_API_DECL neko_camera_t neko_camera_perspective() {
    neko_camera_t cam = neko_camera_default();
    cam.proj_type = NEKO_PROJECTION_TYPE_PERSPECTIVE;
    cam.transform.position.z = 1.f;
    return cam;
}

NEKO_API_DECL neko_vec3 neko_camera_forward(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, 0.0f, -1.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_backward(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, 0.0f, 1.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_up(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, 1.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_down(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, -1.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_right(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(1.0f, 0.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_left(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(-1.0f, 0.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height) {
    // Transform world coords to screen coords to place billboarded UI elements in world
    neko_mat4 vp = neko_camera_get_view_projection(cam, view_width, view_height);
    neko_vec4 p4 = neko_v4(coords.x, coords.y, coords.z, 1.f);
    p4 = neko_mat4_mul_vec4(vp, p4);
    p4.x /= p4.w;
    p4.y /= p4.w;
    p4.z /= p4.w;

    // Bring into ndc
    p4.x = p4.x * 0.5f + 0.5f;
    p4.y = p4.y * 0.5f + 0.5f;

    // Bring into screen space
    p4.x = p4.x * (f32)view_width;
    p4.y = neko_map_range(1.f, 0.f, 0.f, 1.f, p4.y) * (f32)view_height;

    return neko_v3(p4.x, p4.y, p4.z);
}

NEKO_API_DECL neko_vec3 neko_camera_screen_to_world(const neko_camera_t* cam, neko_vec3 coords, s32 view_x, s32 view_y, s32 view_width, s32 view_height) {
    neko_vec3 wc = neko_default_val();

    // Get inverse of view projection from camera
    neko_mat4 inverse_vp = neko_mat4_inverse(neko_camera_get_view_projection(cam, view_width, view_height));
    f32 w_x = (f32)coords.x - (f32)view_x;
    f32 w_y = (f32)coords.y - (f32)view_y;
    f32 w_z = (f32)coords.z;

    // Transform from ndc
    neko_vec4 in;
    in.x = (w_x / (f32)view_width) * 2.f - 1.f;
    in.y = 1.f - (w_y / (f32)view_height) * 2.f;
    in.z = 2.f * w_z - 1.f;
    in.w = 1.f;

    // To world coords
    neko_vec4 out = neko_mat4_mul_vec4(inverse_vp, in);
    if (out.w == 0.f) {
        // Avoid div by zero
        return wc;
    }

    out.w = fabsf(out.w) > neko_epsilon ? 1.f / out.w : 0.0001f;
    wc = neko_v3(out.x * out.w, out.y * out.w, out.z * out.w);

    return wc;
}

NEKO_API_DECL neko_mat4 neko_camera_get_view_projection(const neko_camera_t* cam, s32 view_width, s32 view_height) {
    neko_mat4 view = neko_camera_get_view(cam);
    neko_mat4 proj = neko_camera_get_proj(cam, view_width, view_height);
    return neko_mat4_mul(proj, view);
}

NEKO_API_DECL neko_mat4 neko_camera_get_view(const neko_camera_t* cam) {
    neko_vec3 up = neko_camera_up(cam);
    neko_vec3 forward = neko_camera_forward(cam);
    neko_vec3 target = neko_vec3_add(forward, cam->transform.position);
    return neko_mat4_look_at(cam->transform.position, target, up);
}

NEKO_API_DECL neko_mat4 neko_camera_get_proj(const neko_camera_t* cam, s32 view_width, s32 view_height) {
    neko_mat4 proj_mat = neko_mat4_identity();

    switch (cam->proj_type) {
        case NEKO_PROJECTION_TYPE_PERSPECTIVE: {
            proj_mat = neko_mat4_perspective(cam->fov, (f32)view_width / (f32)view_height, cam->near_plane, cam->far_plane);
        } break;

        // Don't like this...
        case NEKO_PROJECTION_TYPE_ORTHOGRAPHIC: {
            f32 _ar = (f32)view_width / (f32)view_height;
            f32 distance = 0.5f * (cam->far_plane - cam->near_plane);
            const f32 ortho_scale = cam->ortho_scale;
            const f32 aspect_ratio = _ar;
            proj_mat = neko_mat4_ortho(-ortho_scale * aspect_ratio, ortho_scale * aspect_ratio, -ortho_scale, ortho_scale, -distance, distance);
        } break;
    }

    return proj_mat;
}

NEKO_API_DECL void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 pitch) {
    neko_quat x = neko_quat_angle_axis(neko_deg2rad(yaw), neko_v3(0.f, 1.f, 0.f));    // Absolute up
    neko_quat y = neko_quat_angle_axis(neko_deg2rad(pitch), neko_camera_right(cam));  // Relative right
    cam->transform.rotation = neko_quat_mul(neko_quat_mul(x, y), cam->transform.rotation);
}

/*=============================
// NEKO_UTIL
=============================*/

NEKO_API_DECL char* neko_read_file_contents(const char* file_path, const char* mode, size_t* _sz) {
    char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    size_t sz = 0;
    if (fp) {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = (char*)neko_safe_malloc(sz + 1);
        if (buffer) {
            fread(buffer, 1, sz, fp);
        }
        fclose(fp);
        buffer[sz] = '\0';
        if (_sz) *_sz = sz;
    }
    return buffer;
}

bool32_t neko_util_load_texture_data_from_file(const char* file_path, s32* width, s32* height, u32* num_comps, void** data, bool32_t flip_vertically_on_load) {
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(file_path, "rb", &len);
    neko_assert(file_data);
    bool32_t ret = neko_util_load_texture_data_from_memory(file_data, len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        neko_log_warning("Could not load texture: %s", file_path);
    }
    neko_safe_free(file_data);
    return ret;
}

/*========================
// NEKO_LEXER
========================*/

//==== [ Token ] ============================================================//

NEKO_API_DECL neko_token_t neko_token_invalid_token() {
    neko_token_t t = neko_default_val();
    t.text = "";
    t.type = NEKO_TOKEN_UNKNOWN;
    t.len = 0;
    return t;
}

NEKO_API_DECL bool neko_token_compare_type(const neko_token_t* t, neko_token_type type) { return (t->type == type); }

NEKO_API_DECL bool neko_token_compare_text(const neko_token_t* t, const char* match) { return (neko_string_compare_equal_n(t->text, match, t->len)); }

NEKO_API_DECL void neko_token_print_text(const neko_token_t* t) { neko_println("%.*s\n", t->len, t->text); }

NEKO_API_DECL void neko_token_debug_print(const neko_token_t* t) { neko_println("%s: %.*s", neko_token_type_to_str(t->type), t->len, t->text); }

NEKO_API_DECL const char* neko_token_type_to_str(neko_token_type type) {
    switch (type) {
        default:
        case NEKO_TOKEN_UNKNOWN:
            return neko_to_str(NEKO_TOKEN_UNKNOWN);
            break;
        case NEKO_TOKEN_LPAREN:
            return neko_to_str(NEKO_TOKEN_LPAREN);
            break;
        case NEKO_TOKEN_RPAREN:
            return neko_to_str(NEKO_TOKEN_RPAREN);
            break;
        case NEKO_TOKEN_LTHAN:
            return neko_to_str(NEKO_TOKEN_LTHAN);
            break;
        case NEKO_TOKEN_GTHAN:
            return neko_to_str(NEKO_TOKEN_GTHAN);
            break;
        case NEKO_TOKEN_SEMICOLON:
            return neko_to_str(NEKO_TOKEN_SEMICOLON);
            break;
        case NEKO_TOKEN_COLON:
            return neko_to_str(NEKO_TOKEN_COLON);
            break;
        case NEKO_TOKEN_COMMA:
            return neko_to_str(NEKO_TOKEN_COMMA);
            break;
        case NEKO_TOKEN_EQUAL:
            return neko_to_str(NEKO_TOKEN_EQUAL);
            break;
        case NEKO_TOKEN_NOT:
            return neko_to_str(NEKO_TOKEN_NOT);
            break;
        case NEKO_TOKEN_HASH:
            return neko_to_str(NEKO_TOKEN_HASH);
            break;
        case NEKO_TOKEN_PIPE:
            return neko_to_str(NEKO_TOKEN_PIPE);
            break;
        case NEKO_TOKEN_AMPERSAND:
            return neko_to_str(NEKO_TOKEN_AMPERSAND);
            break;
        case NEKO_TOKEN_LBRACE:
            return neko_to_str(NEKO_TOKEN_LBRACE);
            break;
        case NEKO_TOKEN_RBRACE:
            return neko_to_str(NEKO_TOKEN_RBRACE);
            break;
        case NEKO_TOKEN_LBRACKET:
            return neko_to_str(NEKO_TOKEN_LBRACKET);
            break;
        case NEKO_TOKEN_RBRACKET:
            return neko_to_str(NEKO_TOKEN_RBRACKET);
            break;
        case NEKO_TOKEN_MINUS:
            return neko_to_str(NEKO_TOKEN_MINUS);
            break;
        case NEKO_TOKEN_PLUS:
            return neko_to_str(NEKO_TOKEN_PLUS);
            break;
        case NEKO_TOKEN_ASTERISK:
            return neko_to_str(NEKO_TOKEN_ASTERISK);
            break;
        case NEKO_TOKEN_BSLASH:
            return neko_to_str(NEKO_TOKEN_BLASH);
            break;
        case NEKO_TOKEN_FSLASH:
            return neko_to_str(NEKO_TOKEN_FLASH);
            break;
        case NEKO_TOKEN_QMARK:
            return neko_to_str(NEKO_TOKEN_QMARK);
            break;
        case NEKO_TOKEN_DOLLAR:
            return neko_to_str(NEKO_TOKEN_DOLLAR);
            break;
        case NEKO_TOKEN_SPACE:
            return neko_to_str(NEKO_TOKEN_SPACE);
            break;
        case NEKO_TOKEN_NEWLINE:
            return neko_to_str(NEKO_TOKEN_NEWLINE);
            break;
        case NEKO_TOKEN_TAB:
            return neko_to_str(NEKO_TOKEN_TAB);
            break;
        case NEKO_TOKEN_SINGLE_LINE_COMMENT:
            return neko_to_str(NEKO_TOKEN_SINGLE_LINE_COMMENT);
            break;
        case NEKO_TOKEN_MULTI_LINE_COMMENT:
            return neko_to_str(NEKO_TOKEN_MULTI_LINE_COMMENT);
            break;
        case NEKO_TOKEN_IDENTIFIER:
            return neko_to_str(NEKO_TOKEN_IDENTIFIER);
            break;
        case NEKO_TOKEN_NUMBER:
            return neko_to_str(NEKO_TOKEN_NUMBER);
            break;
    }
}

NEKO_API_DECL bool neko_char_is_null_term(char c) { return (c == '\0'); }

NEKO_API_DECL bool neko_char_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }

NEKO_API_DECL bool neko_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_char_is_end_of_line(c)); }

NEKO_API_DECL bool neko_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }

NEKO_API_DECL bool neko_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

//==== [ Lexer ] ============================================================//

NEKO_API_DECL void neko_lexer_set_contents(neko_lexer_t* lex, const char* contents) {
    lex->at = contents;
    lex->current_token = neko_token_invalid_token();
}

NEKO_API_DECL bool neko_lexer_c_can_lex(neko_lexer_t* lex) {
    bool size_pass = lex->contents_size ? lex->size < lex->contents_size : true;
    return (size_pass && lex->at && !neko_char_is_null_term(*(lex->at)));
}

NEKO_API_DECL void neko_lexer_set_token(neko_lexer_t* lex, neko_token_t token) {
    lex->at = token.text;
    lex->current_token = token;
}

NEKO_API_DECL void neko_lexer_c_eat_white_space(neko_lexer_t* lex) {
    for (;;) {
        if (neko_char_is_white_space(*lex->at)) {
            lex->at++;
        }

        // Single line comment
        else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '/')) {
            lex->at += 2;
            while (*lex->at && !neko_char_is_end_of_line(*lex->at)) {
                lex->at++;
            }
        }

        // Multi-line comment
        else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '*')) {
            lex->at += 2;
            while (lex->at[0] && lex->at[1] && !(lex->at[0] == '*' && lex->at[1] == '/')) {
                lex->at++;
            }
            if (lex->at[0] == '*') {
                lex->at++;
            }
        }

        else {
            break;
        }
    }
}

NEKO_API_DECL neko_token_t neko_lexer_c_next_token(neko_lexer_t* lex) {
    if (lex->skip_white_space) {
        lex->eat_white_space(lex);
    }

    neko_token_t t = neko_token_invalid_token();
    t.text = lex->at;
    t.len = 1;

    if (lex->can_lex(lex)) {
        char c = *lex->at;
        switch (c) {
            case '(': {
                t.type = NEKO_TOKEN_LPAREN;
                lex->at++;
            } break;
            case ')': {
                t.type = NEKO_TOKEN_RPAREN;
                lex->at++;
            } break;
            case '<': {
                t.type = NEKO_TOKEN_LTHAN;
                lex->at++;
            } break;
            case '>': {
                t.type = NEKO_TOKEN_GTHAN;
                lex->at++;
            } break;
            case ';': {
                t.type = NEKO_TOKEN_SEMICOLON;
                lex->at++;
            } break;
            case ':': {
                t.type = NEKO_TOKEN_COLON;
                lex->at++;
            } break;
            case ',': {
                t.type = NEKO_TOKEN_COMMA;
                lex->at++;
            } break;
            case '=': {
                t.type = NEKO_TOKEN_EQUAL;
                lex->at++;
            } break;
            case '!': {
                t.type = NEKO_TOKEN_NOT;
                lex->at++;
            } break;
            case '#': {
                t.type = NEKO_TOKEN_HASH;
                lex->at++;
            } break;
            case '|': {
                t.type = NEKO_TOKEN_PIPE;
                lex->at++;
            } break;
            case '&': {
                t.type = NEKO_TOKEN_AMPERSAND;
                lex->at++;
            } break;
            case '{': {
                t.type = NEKO_TOKEN_LBRACE;
                lex->at++;
            } break;
            case '}': {
                t.type = NEKO_TOKEN_RBRACE;
                lex->at++;
            } break;
            case '[': {
                t.type = NEKO_TOKEN_LBRACKET;
                lex->at++;
            } break;
            case ']': {
                t.type = NEKO_TOKEN_RBRACKET;
                lex->at++;
            } break;
            case '+': {
                t.type = NEKO_TOKEN_PLUS;
                lex->at++;
            } break;
            case '*': {
                t.type = NEKO_TOKEN_ASTERISK;
                lex->at++;
            } break;
            case '\\': {
                t.type = NEKO_TOKEN_BSLASH;
                lex->at++;
            } break;
            case '?': {
                t.type = NEKO_TOKEN_QMARK;
                lex->at++;
            } break;
            case '%': {
                t.type = NEKO_TOKEN_PERCENT;
                lex->at++;
            } break;
            case '$': {
                t.type = NEKO_TOKEN_DOLLAR;
                lex->at++;
            } break;
            case ' ': {
                t.type = NEKO_TOKEN_SPACE;
                lex->at++;
            } break;
            case '\n': {
                t.type = NEKO_TOKEN_NEWLINE;
                lex->at++;
            } break;
            case '\r': {
                t.type = NEKO_TOKEN_NEWLINE;
                lex->at++;
            } break;
            case '\t': {
                t.type = NEKO_TOKEN_TAB;
                lex->at++;
            } break;
            case '.': {
                t.type = NEKO_TOKEN_PERIOD;
                lex->at++;
            } break;

            case '-': {
                if (lex->at[1] && !neko_char_is_numeric(lex->at[1])) {
                    t.type = NEKO_TOKEN_MINUS;
                    lex->at++;
                } else {
                    lex->at++;
                    u32 num_decimals = 0;
                    while (lex->at[0] && (neko_char_is_numeric(lex->at[0]) || (lex->at[0] == '.' && num_decimals == 0) || lex->at[0] == 'f')) {
                        // Grab decimal
                        num_decimals = lex->at[0] == '.' ? num_decimals++ : num_decimals;

                        // Increment
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_NUMBER;
                }

            } break;

            case '/': {
                // Single line comment
                if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '/')) {
                    lex->at += 2;
                    while (lex->at[0] && !neko_char_is_end_of_line(lex->at[0])) {
                        lex->at++;
                    }
                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_SINGLE_LINE_COMMENT;
                }

                // Multi line comment
                else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '*')) {
                    lex->at += 2;
                    while (lex->can_lex(lex)) {
                        if (lex->at[0] == '*' && lex->at[1] == '/') {
                            lex->at += 2;
                            break;
                        }
                        lex->at++;
                    }
                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_MULTI_LINE_COMMENT;
                }
                // it's just a forward slash
                else {
                    t.type = NEKO_TOKEN_FSLASH;
                    lex->at++;
                }
            } break;

            case '"': {
                // Move forward after finding first quotation
                lex->at++;

                while (lex->at && *lex->at != '"') {
                    if (lex->at[0] == '\\' && lex->at[1]) {
                        lex->at++;
                    }
                    lex->at++;
                }

                // Move past quotation
                lex->at++;
                t.len = lex->at - t.text;
                t.type = NEKO_TOKEN_STRING;
            } break;

            // Alpha/Numeric/Identifier
            default: {
                if ((neko_char_is_alpha(c) || c == '_') && c != '-') {
                    while (neko_char_is_alpha(lex->at[0]) || neko_char_is_numeric(lex->at[0]) || lex->at[0] == '_') {
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_IDENTIFIER;
                } else if (neko_char_is_numeric(c) && c != '-') {
                    u32 num_decimals = 0;
                    while (neko_char_is_numeric(lex->at[0]) || (lex->at[0] == '.' && num_decimals == 0) || lex->at[0] == 'f') {
                        // Grab decimal
                        num_decimals = lex->at[0] == '.' ? num_decimals++ : num_decimals;

                        // Increment
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_NUMBER;
                } else {
                    t.type = NEKO_TOKEN_UNKNOWN;
                    lex->at++;
                }

            } break;
        }
    }

    // Set current token for lex
    lex->current_token = t;

    // Record size
    lex->size += t.len;

    return t;
}

NEKO_API_DECL neko_token_t neko_lexer_next_token(neko_lexer_t* lex) { return lex->next_token(lex); }

NEKO_API_DECL bool neko_lexer_can_lex(neko_lexer_t* lex) { return lex->can_lex(lex); }

NEKO_API_DECL neko_token_t neko_lexer_current_token(const neko_lexer_t* lex) { return lex->current_token; }

NEKO_API_DECL bool neko_lexer_current_token_compare_type(const neko_lexer_t* lex, neko_token_type type) { return (lex->current_token.type == type); }

NEKO_API_DECL neko_token_t neko_lexer_peek(neko_lexer_t* lex) {
    // Store current at and current token
    const char* at = lex->at;
    neko_token_t cur_t = neko_lexer_current_token(lex);

    // Get next token
    neko_token_t next_t = lex->next_token(lex);

    // Reset
    lex->current_token = cur_t;
    lex->at = at;

    // Return
    return next_t;
}

// Check to see if token type of next valid token matches 'match'. Restores place in lex if not.
NEKO_API_DECL bool neko_lexer_require_token_text(neko_lexer_t* lex, const char* match) {
    // Store current position and token
    const char* at = lex->at;
    neko_token_t cur_t = lex->current_token;

    // Get next token
    neko_token_t next_t = lex->next_token(lex);

    // Compare token text
    if (neko_token_compare_text(&next_t, match)) {
        return true;
    }

    // Error
    neko_println("error::neko_lexer_require_token_text::%.*s, expected: %s", cur_t.len, cur_t.text, match);

    // Reset
    lex->at = at;
    lex->current_token = cur_t;

    return false;
}

NEKO_API_DECL bool neko_lexer_require_token_type(neko_lexer_t* lex, neko_token_type type) {
    // Store current position and token
    const char* at = lex->at;
    neko_token_t cur_t = lex->current_token;

    // Get next token
    neko_token_t next_t = lex->next_token(lex);

    // Compare token type
    if (neko_token_compare_type(&next_t, type)) {
        return true;
    }

    // Error
    // neko_println("error::neko_lexer_require_token_type::%s, expected: %s", neko_token_type_to_str(next_t.type), neko_token_type_to_str(type));

    // Reset
    lex->at = at;
    lex->current_token = cur_t;

    return false;
}

// Advances until next token of given type is found
NEKO_API_DECL bool neko_lexer_find_next_token_type(neko_lexer_t* lex, neko_token_type type) {
    neko_token_t t = lex->next_token(lex);
    while (lex->can_lex(lex)) {
        if (neko_token_compare_type(&t, type)) {
            return true;
        }
        t = lex->next_token(lex);
    }
    return false;
}

NEKO_API_DECL neko_token_t neko_lexer_advance_before_next_token_type(neko_lexer_t* lex, neko_token_type type) {
    neko_token_t t = lex->current_token;
    neko_token_t peek_t = neko_lexer_peek(lex);

    // Continue right up until required token type
    while (!neko_token_compare_type(&peek_t, type)) {
        t = lex->next_token(lex);
        peek_t = neko_lexer_peek(lex);
    }

    return t;
}

NEKO_API_DECL neko_lexer_t neko_lexer_c_ctor(const char* contents) {
    neko_lexer_t lex = neko_default_val();
    lex.contents = contents;
    lex.at = contents;
    lex.can_lex = neko_lexer_c_can_lex;
    lex.eat_white_space = neko_lexer_c_eat_white_space;
    lex.next_token = neko_lexer_c_next_token;
    lex.skip_white_space = true;
    return lex;
}

/*================================================================================
// Random
================================================================================*/

#define NEKO_RAND_UPPER_MASK 0x80000000
#define NEKO_RAND_LOWER_MASK 0x7fffffff
#define NEKO_RAND_TEMPERING_MASK_B 0x9d2c5680
#define NEKO_RAND_TEMPERING_MASK_C 0xefc60000

NEKO_API_DECL void _neko_rand_seed_impl(neko_mt_rand_t* rand, uint64_t seed) {
    rand->mt[0] = seed & 0xffffffff;
    for (rand->index = 1; rand->index < NEKO_STATE_VECTOR_LENGTH; rand->index++) {
        rand->mt[rand->index] = (6069 * rand->mt[rand->index - 1]) & 0xffffffff;
    }
}

NEKO_API_DECL neko_mt_rand_t neko_rand_seed(uint64_t seed) {
    neko_mt_rand_t rand;
    _neko_rand_seed_impl(&rand, seed);
    return rand;
}

NEKO_API_DECL uint64_t neko_rand_gen_long(neko_mt_rand_t* rand) {
    uint64_t y;
    static uint64_t mag[2] = {0x0, 0x9908b0df}; /* mag[x] = x * 0x9908b0df for x = 0,1 */
    if (rand->index >= NEKO_STATE_VECTOR_LENGTH || rand->index < 0) {
        // generate NEKO_STATE_VECTOR_LENGTH words at a time
        int kk;
        if (rand->index >= NEKO_STATE_VECTOR_LENGTH + 1 || rand->index < 0) {
            _neko_rand_seed_impl(rand, 4357);
        }
        for (kk = 0; kk < NEKO_STATE_VECTOR_LENGTH - NEKO_STATE_VECTOR_M; kk++) {
            y = (rand->mt[kk] & NEKO_RAND_UPPER_MASK) | (rand->mt[kk + 1] & NEKO_RAND_LOWER_MASK);
            rand->mt[kk] = rand->mt[kk + NEKO_STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
        }
        for (; kk < NEKO_STATE_VECTOR_LENGTH - 1; kk++) {
            y = (rand->mt[kk] & NEKO_RAND_UPPER_MASK) | (rand->mt[kk + 1] & NEKO_RAND_LOWER_MASK);
            rand->mt[kk] = rand->mt[kk + (NEKO_STATE_VECTOR_M - NEKO_STATE_VECTOR_LENGTH)] ^ (y >> 1) ^ mag[y & 0x1];
        }
        y = (rand->mt[NEKO_STATE_VECTOR_LENGTH - 1] & NEKO_RAND_UPPER_MASK) | (rand->mt[0] & NEKO_RAND_LOWER_MASK);
        rand->mt[NEKO_STATE_VECTOR_LENGTH - 1] = rand->mt[NEKO_STATE_VECTOR_M - 1] ^ (y >> 1) ^ mag[y & 0x1];
        rand->index = 0;
    }
    y = rand->mt[rand->index++];
    y ^= (y >> 11);
    y ^= (y << 7) & NEKO_RAND_TEMPERING_MASK_B;
    y ^= (y << 15) & NEKO_RAND_TEMPERING_MASK_C;
    y ^= (y >> 18);
    return y;
}

NEKO_API_DECL double neko_rand_gen(neko_mt_rand_t* rand) { return ((double)neko_rand_gen_long(rand) / (uint64_t)0xffffffff); }

NEKO_API_DECL uint64_t neko_rand_gen_range_long(neko_mt_rand_t* rand, int32_t min, int32_t max) { return (uint64_t)(floorf(neko_rand_gen_range(rand, (double)min, (double)max))); }

NEKO_API_DECL double neko_rand_gen_range(neko_mt_rand_t* rand, double min, double max) { return neko_map_range(0.0, 1.0, min, max, neko_rand_gen(rand)); }

NEKO_API_DECL neko_color_t neko_rand_gen_color(neko_mt_rand_t* rand) {
    neko_color_t c = neko_default_val();
    c.r = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.g = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.b = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.a = (u8)neko_rand_gen_range_long(rand, 0, 255);
    return c;
}

#pragma region neko_ecs

#include "engine/neko_component.h"

neko_ecs_stack* neko_ecs_stack_make(u64 capacity) {
    neko_ecs_stack* s = (neko_ecs_stack*)neko_malloc(sizeof(neko_ecs_stack));
    s->data = (u32*)neko_malloc(sizeof(*s->data) * capacity);
    s->capacity = capacity;
    s->top = 0;
    s->empty = true;

    return s;
}

void neko_ecs_stack_destroy(neko_ecs_stack* s) {
    neko_free(s->data);
    neko_free(s);
}

b32 neko_ecs_stack_empty(neko_ecs_stack* s) { return s->empty; }

b32 neko_ecs_stack_full(neko_ecs_stack* s) { return s->top == s->capacity; }

u64 neko_ecs_stack_capacity(neko_ecs_stack* s) { return s->capacity; }

u64 neko_ecs_stack_top(neko_ecs_stack* s) { return s->top; }

u32 neko_ecs_stack_peek(neko_ecs_stack* s) {
    if (s->empty) {
        neko_println("Failed to peek, stack is full");
        return 0;
    }
    return s->data[s->top - 1];
}

void neko_ecs_stack_push(neko_ecs_stack* s, u32 val) {
    if (neko_ecs_stack_full(s)) {
        neko_println("Failed to push %u, stack is full", val);
        return;
    }

    s->empty = false;
    s->data[s->top++] = val;
}

u32 neko_ecs_stack_pop(neko_ecs_stack* s) {
    if (s->empty) {
        neko_println("Failed to pop, stack is empty");
        return 0;
    }

    if (s->top == 1) s->empty = true;
    return s->data[--s->top];
}

neko_ecs_component_pool neko_ecs_component_pool_make(u32 count, u32 size, neko_ecs_component_destroy destroy_func) {
    neko_ecs_component_pool pool;
    pool.data = neko_malloc(count * size);
    pool.count = count;
    pool.size = size;
    pool.destroy_func = destroy_func;
    pool.indexes = neko_ecs_stack_make(count);

    for (u32 i = count; i-- > 0;) {
        neko_ecs_stack_push(pool.indexes, i);
    }

    return pool;
}

void neko_ecs_component_pool_destroy(neko_ecs_component_pool* pool) {
    neko_free(pool->data);
    neko_ecs_stack_destroy(pool->indexes);
}

void neko_ecs_component_pool_push(neko_ecs_component_pool* pool, u32 index) {
    u8* ptr = (u8*)((u8*)pool->data + (index * pool->size));
    if (pool->destroy_func) pool->destroy_func(ptr);
    neko_ecs_stack_push(pool->indexes, index);
}

u32 neko_ecs_component_pool_pop(neko_ecs_component_pool* pool, void* data) {
    u32 index = neko_ecs_stack_pop(pool->indexes);
    u8* ptr = (u8*)((u8*)pool->data + (index * pool->size));
    memcpy(ptr, data, pool->size);  // 初始化组件的数据是以深拷贝进行
    return index;
}

neko_ecs* neko_ecs_make(u32 max_entities, u32 component_count, u32 system_count) {
    neko_ecs* ecs = (neko_ecs*)neko_malloc(sizeof(*ecs));
    ecs->max_entities = max_entities;
    ecs->component_count = component_count;
    ecs->system_count = system_count;
    ecs->indexes = neko_ecs_stack_make(max_entities);
    ecs->max_index = 0;
    ecs->versions = (u32*)neko_malloc(max_entities * sizeof(u32));
    ecs->components = (u32*)neko_malloc(max_entities * component_count * sizeof(u32));
    ecs->component_masks = (b32*)neko_malloc(max_entities * component_count * sizeof(b32));
    ecs->pool = (neko_ecs_component_pool*)neko_malloc(component_count * sizeof(*ecs->pool));
    ecs->systems = (neko_ecs_system*)neko_malloc(system_count * sizeof(*ecs->systems));
    ecs->systems_top = 0;

    for (u32 i = max_entities; i-- > 0;) {
        neko_ecs_stack_push(ecs->indexes, i);

        ecs->versions[i] = 0;
        for (u32 j = 0; j < component_count; j++) {
            ecs->components[i * component_count + j] = 0;
            ecs->component_masks[i * component_count + j] = 0;
        }
    }

    for (u32 i = 0; i < system_count; i++) {
        ecs->systems[i].func = NULL;
    }

    for (u32 i = 0; i < ecs->component_count; i++) {
        ecs->pool[i].data = NULL;
    }

    return ecs;
}

void neko_ecs_destroy(neko_ecs* ecs) {
    for (u32 i = 0; i < ecs->component_count; i++) {
        neko_ecs_component_pool_destroy(&ecs->pool[i]);
    }

    neko_ecs_stack_destroy(ecs->indexes);

    neko_free(ecs->versions);
    neko_free(ecs->components);
    neko_free(ecs->component_masks);
    neko_free(ecs->pool);
    neko_free(ecs->systems);

    neko_free(ecs);
}

void neko_ecs_register_component(neko_ecs* ecs, neko_ecs_component_type component_type, u32 count, u32 size, neko_ecs_component_destroy destroy_func) {
    if (ecs->pool[component_type].data != NULL) {
        neko_println("Registered Component type %u more than once.\n", component_type);
        return;
    }

    if (count * size <= 0) {
        neko_println("Registering Component type %u (count*size) is less than 0.\n", component_type);
        return;
    }

    ecs->pool[component_type] = neko_ecs_component_pool_make(count, size, destroy_func);
}

void neko_ecs_register_system(neko_ecs* ecs, neko_ecs_system_func func, neko_ecs_system_type type) {
    neko_ecs_system* sys = &ecs->systems[ecs->systems_top++];
    sys->func = func;
    sys->type = type;
}

void neko_ecs_run_systems(neko_ecs* ecs, neko_ecs_system_type type) {
    for (u32 i = 0; i < ecs->systems_top; i++) {
        neko_ecs_system* sys = &ecs->systems[i];
        if (sys->type == type) sys->func(ecs);
    }
}

void neko_ecs_run_system(neko_ecs* ecs, u32 system_index) { ecs->systems[system_index].func(ecs); }

u32 neko_ecs_for_count(neko_ecs* ecs) { return ecs->max_index + 1; }

neko_ecs_ent neko_ecs_get_ent(neko_ecs* ecs, u32 index) { return __neko_ecs_ent_id(index, ecs->versions[index]); }

neko_ecs_ent neko_ecs_ent_make(neko_ecs* ecs) {
    u32 index = neko_ecs_stack_pop(ecs->indexes);
    u32 ver = ecs->versions[index];

    if (index > ecs->max_index) ecs->max_index = index;

    return __neko_ecs_ent_id(index, ver);
}

void neko_ecs_ent_destroy(neko_ecs* ecs, neko_ecs_ent e) {
    u32 index = __neko_ecs_ent_index(e);

    ecs->versions[index]++;
    for (u32 i = 0; i < ecs->component_count; i++) {
        neko_ecs_ent_remove_component(ecs, e, i);
    }

    neko_ecs_stack_push(ecs->indexes, index);
}

void neko_ecs_ent_add_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type type, void* component_data) {
    u32 index = __neko_ecs_ent_index(e);

    if (neko_ecs_ent_has_component(ecs, e, type)) {
        neko_println("Component %u already exists on neko_ecs_ent %lu (Index %u)", type, e, index);
        return;
    }

    neko_ecs_component_pool* pool = &ecs->pool[type];
    u32 c_index = neko_ecs_component_pool_pop(pool, component_data);
    ecs->components[index * ecs->component_count + type] = c_index;
    ecs->component_masks[index * ecs->component_count + type] = true;
}

void neko_ecs_ent_remove_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type type) {
    u32 index = __neko_ecs_ent_index(e);

    if (!neko_ecs_ent_has_component(ecs, e, type)) {
        neko_println("Component %u doesn't exist on neko_ecs_ent %lu (Index %u)", type, e, index);
        return;
    }

    neko_ecs_component_pool* pool = &ecs->pool[type];
    neko_ecs_component_pool_push(pool, ecs->components[index * ecs->component_count + type]);
    ecs->component_masks[index * ecs->component_count + type] = false;
}

void* neko_ecs_ent_get_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type type) {
    u32 index = __neko_ecs_ent_index(e);

    if (!neko_ecs_ent_has_component(ecs, e, type)) {
        neko_println("Trying to get non existent component %u on neko_ecs_ent %lu (Index %u)", type, e, index);
        return NULL;
    }

    u32 c_index = ecs->components[index * ecs->component_count + type];
    u8* ptr = (u8*)((u8*)ecs->pool[type].data + (c_index * ecs->pool[type].size));
    return ptr;
}

b32 neko_ecs_ent_is_valid(neko_ecs* ecs, neko_ecs_ent e) { return ecs->versions[__neko_ecs_ent_index(e)] == __neko_ecs_ent_ver(e); }

b32 neko_ecs_ent_has_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type component_type) { return ecs->component_masks[__neko_ecs_ent_index(e) * ecs->component_count + component_type]; }

b32 neko_ecs_ent_has_mask(neko_ecs* ecs, neko_ecs_ent e, u32 component_type_count, neko_ecs_component_type component_types[]) {
    for (u32 i = 0; i < component_type_count; i++) {
        if (!neko_ecs_ent_has_component(ecs, e, component_types[i])) return false;
    }

    return true;
}

u32 neko_ecs_ent_get_version(neko_ecs* ecs, neko_ecs_ent e) { return ecs->versions[__neko_ecs_ent_index(e)]; }

void neko_ecs_ent_print(neko_ecs* ecs, neko_ecs_ent e) {
    u32 index = __neko_ecs_ent_index(e);

    printf("---- neko_ecs_ent ----\nIndex: %d\nVersion: %d\nMask: ", index, ecs->versions[index]);

    for (u32 i = ecs->component_count; i-- > 0;) {
        printf("%u", ecs->component_masks[index * ecs->component_count + i]);
    }
    printf("\n");

    for (u32 i = 0; i < ecs->component_count; i++) {
        if (neko_ecs_ent_has_component(ecs, e, i)) {
            printf("Component Type: %s (Index: %d)\n", neko::enum_name((ComponentType)i).data(), ecs->components[index * ecs->component_count + i]);
        }
    }

    printf("----------------\n");
}

#pragma endregion

/*=============================
// NEKO_ENGINE
=============================*/

NEKO_API_DECL void neko_default_app_func();

NEKO_API_DECL void neko_default_main_window_close_callback(void* window);

neko_global neko_t* _neko_instance = neko_default_val();

NEKO_API_DECL neko_t* neko_create(neko_game_desc_t app_desc) {
    if (neko_instance() == NULL) {
        // Check app desc for defaults
        if (app_desc.window.width == 0) app_desc.window.width = 800;
        if (app_desc.window.height == 0) app_desc.window.height = 600;
        if (app_desc.window.title == 0) app_desc.window.title = "Neko Engine";
        if (app_desc.window.frame_rate <= 0.f) app_desc.window.frame_rate = 60.f;
        if (app_desc.update == NULL) app_desc.update = &neko_default_app_func;
        if (app_desc.shutdown == NULL) app_desc.shutdown = &neko_default_app_func;
        if (app_desc.init == NULL) app_desc.init = &neko_default_app_func;

        __neko_mem_init(app_desc.argc, app_desc.argv);

        // Set up os api before all?
        neko_os_api_t os = neko_os_api_new();

        // Construct instance and set
        _neko_instance = (neko_t*)os.malloc(sizeof(neko_t));
        memset(_neko_instance, 0, sizeof(neko_t));

        // Set os api now allocated
        neko_instance()->ctx.os = os;

        // Set application description for framework
        neko_instance()->ctx.game = app_desc;

        // Set up function pointers
        neko_instance()->shutdown = &neko_destroy;

        // 初始化 cvars
        __neko_config_init();

        // Need to have video settings passed down from user
        neko_subsystem(platform) = neko_platform_create();

        // Enable graphics API debugging
        neko_subsystem(platform)->settings.video.graphics.debug = app_desc.debug_gfx;

        // Default initialization for platform here
        neko_platform_init(neko_subsystem(platform));

        // Set frame rate for application
        neko_subsystem(platform)->time.max_fps = app_desc.window.frame_rate;

        // Construct main window
        neko_platform_window_create(&app_desc.window);

        // Set vsync for video
        neko_platform_enable_vsync(app_desc.window.vsync);

        // Construct graphics api
        neko_subsystem(graphics) = neko_graphics_create();

        // Initialize graphics here
        neko_graphics_init(neko_subsystem(graphics));

        // Construct audio api
        neko_instance()->ctx.audio = __neko_audio_construct();

        // Initialize audio
        neko_instance()->ctx.audio->init(neko_instance()->ctx.audio);

        // 初始化 ecs
        neko_ecs() = neko_ecs_make(1024, COMPONENT_COUNT, 3);

        // Initialize application and set to running
        app_desc.init();
        neko_ctx()->game.is_running = true;

        // Set default callback for when main window close button is pressed
        neko_platform_set_window_close_callback(neko_platform_main_window(), &neko_default_main_window_close_callback);
    }

    return neko_instance();
}

NEKO_API_DECL void neko_set_instance(neko_t* neko) { _neko_instance = neko; }

NEKO_API_DECL neko_t* neko_instance() { return _neko_instance; }

NEKO_API_DECL neko_context_t* neko_ctx() { return &neko_instance()->ctx; }

NEKO_API_DECL neko_game_desc_t* neko_app() { return &neko_instance()->ctx.game; }

// Define main frame function for framework to step
NEKO_API_DECL void neko_frame() {
    // Remove these...
    static u32 curr_ticks = 0;
    static u32 prev_ticks = 0;

    // Cache platform pointer
    neko_platform_t* platform = neko_subsystem(platform);
    neko_audio_t* audio = neko_subsystem(audio);

    neko_platform_window_t* win = (neko_slot_array_getp(platform->windows, neko_platform_main_window()));

    // Cache times at start of frame
    platform->time.elapsed = (f32)neko_platform_elapsed_time();
    platform->time.update = platform->time.elapsed - platform->time.previous;
    platform->time.previous = platform->time.elapsed;

    // Update platform and process input
    neko_platform_update(platform);
    if (!neko_instance()->ctx.game.is_running) {
        neko_instance()->shutdown();
        return;
    }

    if (win->focus) {

        // Process application context
        neko_instance()->ctx.game.update();
        if (!neko_instance()->ctx.game.is_running) {
            neko_instance()->shutdown();
            return;
        }

        {
            // neko_profiler_scope_auto("audio");
            //  Audio update and commit
            if (audio) {
                if (audio->update) {
                    audio->update(audio);
                }
                if (audio->commit) {
                    audio->commit(audio);
                }
            }
        }

        // 感觉这玩意放这里不是很合理 之后再改
        neko_subsystem(graphics)->api.fontcache_draw();
    }

    // Clear all platform events
    neko_dyn_array_clear(platform->events);

    // NOTE: This won't work forever. Must change eventually.
    // Swap all platform window buffers? Sure...
    for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
        neko_platform_window_swap_buffer(it);
    }

    // Frame locking (not sure if this should be done here, but it is what it is)
    platform->time.elapsed = (f32)neko_platform_elapsed_time();
    platform->time.render = platform->time.elapsed - platform->time.previous;
    platform->time.previous = platform->time.elapsed;
    platform->time.frame = platform->time.update + platform->time.render;  // Total frame time
    platform->time.delta = platform->time.frame / 1000.f;

    f32 target = (1000.f / platform->time.max_fps);

    if (platform->time.frame < target) {
        neko_platform_sleep((f32)(target - platform->time.frame));

        platform->time.elapsed = (f32)neko_platform_elapsed_time();
        double wait_time = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;
        platform->time.frame += wait_time;
        platform->time.delta = platform->time.frame / 1000.f;
    }
}

void neko_destroy() {
    // Shutdown application
    neko_ctx()->game.shutdown();
    neko_ctx()->game.is_running = false;

    // Shutdown subsystems
    neko_ecs_destroy(neko_ecs());

    neko_graphics_shutdown(neko_subsystem(graphics));
    neko_graphics_destroy(neko_subsystem(graphics));

    neko_audio_shutdown(neko_subsystem(audio));
    neko_audio_destroy(neko_subsystem(audio));

    neko_platform_shutdown(neko_subsystem(platform));
    neko_platform_destroy(neko_subsystem(platform));

    __neko_config_free();

    // 在 app 结束后进行内存检查
    __neko_mem_end();
}

NEKO_API_DECL void neko_default_app_func() {
    // Nothing...
}

NEKO_API_DECL void neko_default_main_window_close_callback(void* window) { neko_instance()->ctx.game.is_running = false; }

void neko_quit() {
#ifndef NEKO_PLATFORM_WEB
    neko_instance()->ctx.game.is_running = false;
#endif
}