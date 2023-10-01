
#include "neko.h"

#include <csetjmp>
#include <format>

#include "engine/neko_component.h"
#include "engine/neko_ecs.h"
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
    fprintf(ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf, level_colors[ev->level], level_strings[ev->level], ev->file, ev->line);
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
    buffer->data = (uint8_t*)neko_malloc(NEKO_BYTE_BUFFER_DEFAULT_CAPCITY);
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
        neko_free(buffer->data);
    }
}

void neko_byte_buffer_clear(neko_byte_buffer_t* buffer) {
    buffer->size = 0;
    buffer->position = 0;
}

bool neko_byte_buffer_empty(neko_byte_buffer_t* buffer) { return (buffer->size == 0); }

size_t neko_byte_buffer_size(neko_byte_buffer_t* buffer) { return buffer->size; }

void neko_byte_buffer_resize(neko_byte_buffer_t* buffer, size_t sz) {
    uint8_t* data = (uint8_t*)neko_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;
    buffer->capacity = (uint32_t)sz;
}

void neko_byte_buffer_copy_contents(neko_byte_buffer_t* dst, neko_byte_buffer_t* src) {
    neko_byte_buffer_seek_to_beg(dst);
    neko_byte_buffer_seek_to_beg(src);
    neko_byte_buffer_write_bulk(dst, src->data, src->size);
}

void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t* buffer) { buffer->position = 0; }

void neko_byte_buffer_seek_to_end(neko_byte_buffer_t* buffer) { buffer->position = buffer->size; }

void neko_byte_buffer_advance_position(neko_byte_buffer_t* buffer, size_t sz) { buffer->position += (uint32_t)sz; }

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

    buffer->size += (uint32_t)size;
    buffer->position += (uint32_t)size;
}

void neko_byte_buffer_read_bulk(neko_byte_buffer_t* buffer, void** dst, size_t size) {
    memcpy(*dst, (buffer->data + buffer->position), size);
    buffer->position += (uint32_t)size;
}

void neko_byte_buffer_write_str(neko_byte_buffer_t* buffer, const char* str) {
    // Write size of string
    uint32_t str_len = neko_string_length(str);
    neko_byte_buffer_write(buffer, uint16_t, str_len);

    size_t i;
    for (i = 0; i < str_len; ++i) {
        neko_byte_buffer_write(buffer, uint8_t, str[i]);
    }
}

void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str) {
    // Read in size of string from buffer
    uint16_t sz;
    neko_byte_buffer_read(buffer, uint16_t, &sz);

    uint32_t i;
    for (i = 0; i < sz; ++i) {
        neko_byte_buffer_read(buffer, uint8_t, &str[i]);
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

NEKO_API_DECL void neko_byte_buffer_memset(neko_byte_buffer_t* buffer, uint8_t val) { memset(buffer->data, val, buffer->capacity); }

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
        data->capacity = (int32_t)capacity;
        return ((int32_t*)data + 2);
    }

    return NULL;
}

NEKO_API_DECL void** neko_dyn_array_init(void** arr, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array* data = (neko_dyn_array*)neko_malloc(val_len + sizeof(neko_dyn_array));  // Allocate capacity of one
        data->size = 0;
        data->capacity = 1;
        *arr = ((int32_t*)data + 2);
    }
    return arr;
}

NEKO_API_DECL void neko_dyn_array_push_data(void** arr, void* val, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array_init(arr, val_len);
    }
    if (neko_dyn_array_need_grow(*arr, 1)) {
        int32_t capacity = neko_dyn_array_capacity(*arr) * 2;

        // Create new neko_dyn_array with just the header information
        neko_dyn_array* data = (neko_dyn_array*)neko_realloc(neko_dyn_array_head(*arr), capacity * val_len + sizeof(neko_dyn_array));

        if (data) {
            data->capacity = capacity;
            *arr = ((int32_t*)data + 2);
        }
    }
    size_t offset = neko_dyn_array_size(*arr);
    memcpy(((uint8_t*)(*arr)) + offset * val_len, val, val_len);
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
//     uint8_t* data;
//     size_t size;
// } neko_memory_block_t;

NEKO_API_DECL neko_memory_block_t neko_memory_block_new(size_t sz) {
    neko_memory_block_t mem = neko_default_val();
    mem.data = (uint8_t*)neko_malloc(sz);
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
//     uint8_t* memory;
//     size_t total_size;
//     size_t offset;
// } neko_linear_allocator_t;

NEKO_API_DECL neko_linear_allocator_t neko_linear_allocator_new(size_t sz) {
    neko_linear_allocator_t la = neko_default_val();
    la.memory = (uint8_t*)neko_malloc(sz);
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
    uint8_t* data = (uint8_t*)(sa->memory.data + sa->offset);
    header->size = (uint32_t)sz;

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
    void* data = (uint8_t*)(sa->memory.data + sa->offset - sizeof(neko_stack_allocator_header_t) - header->size);
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
//     (((uint8_t*)P + (BYTES)))

        uint32_t bppmo = pa->blocks_per_page - 1;
        for (uint32_t i = 0; i < bppmo; ++i) {
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
    for (uint32_t i = 0; i < pa->page_count; ++i) {
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
//     uint32_t free_block_count;
//     uint32_t free_block_capacity;
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

    for (uint32_t i = 0; i < ha->free_block_count; ++i) {
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

NEKO_API_DECL neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, int32_t view_width, int32_t view_height) {
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
    p4.x = p4.x * (float)view_width;
    p4.y = neko_map_range(1.f, 0.f, 0.f, 1.f, p4.y) * (float)view_height;

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

#ifndef NEKO_NO_STB_RECT_PACK
#define STB_RECT_PACK_IMPLEMENTATION
#endif

#ifndef NEKO_NO_STB_TRUETYPE
// #define STBTT_RASTERIZER_VERSION 0
#define STB_TRUETYPE_IMPLEMENTATION
#endif

#ifndef NEKO_NO_STB_DEFINE
#define STB_DEFINE
#endif

#ifndef NEKO_NO_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#ifndef NEKO_NO_CGLTF
#define CGLTF_IMPLEMENTATION
#endif

// STB
#include "libs/stb/stb_image.h"
#include "libs/stb/stb_rect_pack.h"
#include "libs/stb/stb_truetype.h"

// CGLTF
#include "libs/cgltf/cgltf.h"

NEKO_API_DECL char* neko_read_file_contents_into_string_null_term(const char* file_path, const char* mode, size_t* _sz) {
    char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    size_t sz = 0;
    if (fp) {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = (char*)neko_malloc(sz + 1);
        if (buffer) {
            fread(buffer, 1, sz, fp);
        }
        fclose(fp);
        buffer[sz] = '\0';
        if (_sz) *_sz = sz;
    }
    return buffer;
}

bool32_t neko_util_load_texture_data_from_file(const char* file_path, int32_t* width, int32_t* height, uint32_t* num_comps, void** data, bool32_t flip_vertically_on_load) {
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(file_path, "rb", &len);
    neko_assert(file_data);
    bool32_t ret = neko_util_load_texture_data_from_memory(file_data, len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        neko_println("Warning: could not load texture: %s", file_path);
    }
    neko_free(file_data);
    return ret;
}

NEKO_API_DECL bool32_t neko_util_load_texture_data_from_memory(const void* memory, size_t sz, int32_t* width, int32_t* height, uint32_t* num_comps, void** data, bool32_t flip_vertically_on_load) {
    // Load texture data
    stbi_set_flip_vertically_on_load(flip_vertically_on_load);
    *data = stbi_load_from_memory((const stbi_uc*)memory, (int32_t)sz, (int32_t*)width, (int32_t*)height, (int32_t*)num_comps, STBI_rgb_alpha);
    if (!*data) {
        neko_free(*data);
        return false;
    }
    return true;
}

/*==========================
// NEKO_ASSET_TYPES
==========================*/

NEKO_API_DECL bool neko_asset_texture_load_from_file(const char* path, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data) {
    neko_asset_texture_t* t = (neko_asset_texture_t*)out;

    memset(&t->desc, 0, sizeof(neko_graphics_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    FILE* f = fopen(path, "rb");
    if (!f) {
        return false;
    }

    int32_t comp = 0;
    stbi_set_flip_vertically_on_load(t->desc.flip_y);
    *t->desc.data = (uint8_t*)stbi_load_from_file(f, (int32_t*)&t->desc.width, (int32_t*)&t->desc.height, (int32_t*)&comp, STBI_rgb_alpha);

    if (!t->desc.data) {
        fclose(f);
        return false;
    }

    t->hndl = neko_graphics_texture_create(&t->desc);

    if (!keep_data) {
        neko_free(*t->desc.data);
        *t->desc.data = NULL;
    }

    fclose(f);
    return true;
}

/*
bool neko_asset_texture_load_from_file(const char* path, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data)
{
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(path, "rb", &len);
    neko_assert(file_data);
    bool32_t ret = neko_asset_texture_load_from_memory(file_data, len, out, desc, flip_on_load, keep_data);
    neko_free(file_data);
    return ret;
}
 */

bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data) {
    neko_asset_texture_t* t = (neko_asset_texture_t*)out;

    memset(&t->desc, 0, sizeof(neko_graphics_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    int32_t num_comps = 0;
    bool32_t loaded = neko_util_load_texture_data_from_memory(memory, sz, (int32_t*)&t->desc.width, (int32_t*)&t->desc.height, (uint32_t*)&num_comps, (void**)&t->desc.data, t->desc.flip_y);

    if (!loaded) {
        return false;
    }

    t->hndl = neko_graphics_texture_create(&t->desc);

    if (!keep_data) {
        neko_free(t->desc.data);
        *t->desc.data = NULL;
    }

    return true;
}

bool neko_asset_font_load_from_file(const char* path, void* out, uint32_t point_size) {
    size_t len = 0;
    char* ttf = neko_platform_read_file_contents(path, "rb", &len);
    if (!point_size) {
        neko_println("Warning: Font: %s: Point size not declared. Setting to default 16.", path);
        point_size = 16;
    }
    bool ret = neko_asset_font_load_from_memory(ttf, len, out, point_size);
    if (!ret) {
        neko_println("Font Failed to Load: %s", path);
    } else {
        neko_println("Font Successfully Loaded: %s", path);
    }
    neko_free(ttf);
    return ret;
}

bool neko_asset_font_load_from_memory(const void* memory, size_t sz, void* out, uint32_t point_size) {
    neko_asset_font_t* f = (neko_asset_font_t*)out;

    if (!point_size) {
        neko_println("Warning: Font: Point size not declared. Setting to default 16.");
        point_size = 16;
    }

    // Poor attempt at an auto resized texture
    const uint32_t point_wh = neko_max(point_size, 32);
    const uint32_t w = (point_wh / 32 * 512) + (point_wh / 32 * 512) % 512;
    const uint32_t h = (point_wh / 32 * 512) + (point_wh / 32 * 512) % 512;

    const uint32_t num_comps = 4;
    u8* alpha_bitmap = (uint8_t*)neko_malloc(w * h);
    u8* flipmap = (uint8_t*)neko_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);
    s32 v = stbtt_BakeFontBitmap((u8*)memory, 0, (float)point_size, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f->glyphs);  // no guarantee this fits!

    // Flip texture
    u32 r = h - 1;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            u32 i0 = i * w + j;
            u32 i1 = i * w * num_comps + j * num_comps;
            u8 a = alpha_bitmap[i0];
            flipmap[i1 + 0] = 255;
            flipmap[i1 + 1] = 255;
            flipmap[i1 + 2] = 255;
            flipmap[i1 + 3] = a;
        }
        r--;
    }

    neko_graphics_texture_desc_t desc = neko_default_val();
    desc.width = w;
    desc.height = h;
    *desc.data = flipmap;
    desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;

    // Generate atlas texture for bitmap with bitmap data
    f->texture.hndl = neko_graphics_texture_create(&desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    bool success = false;
    if (v <= 0) {
        neko_println("Font Failed to Load, Baked Texture Was Too Small: %d", v);
    } else {
        neko_println("Font Successfully Loaded: %d", v);
        success = true;
    }

    neko_free(alpha_bitmap);
    neko_free(flipmap);
    return success;
}

NEKO_API_DECL float neko_asset_font_max_height(const neko_asset_font_t* fp) {
    if (!fp) return 0.f;
    float h = 0.f, x = 0.f, y = 0.f;
    const char* txt = "1l`'f()ABCDEFGHIJKLMNOjPQqSTU!";
    while (txt[0] != '\0') {
        char c = txt[0];
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q = neko_default_val();
            stbtt_GetBakedQuad((stbtt_bakedchar*)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);
            h = neko_max(neko_max(h, fabsf(q.y0)), fabsf(q.y1));
        }
        txt++;
    };
    return h;
}

NEKO_API_DECL neko_vec2 neko_asset_font_text_dimensions(const neko_asset_font_t* fp, const char* text, int32_t len) { return neko_asset_font_text_dimensions_ex(fp, text, len, 0); }

NEKO_API_DECL neko_vec2 neko_asset_font_text_dimensions_ex(const neko_asset_font_t* fp, const char* text, int32_t len, bool32_t include_past_baseline) {
    neko_vec2 dimensions = neko_v2s(0.f);

    if (!fp || !text) return dimensions;
    float x = 0.f;
    float y = 0.f;
    float y_under = 0;

    while (text[0] != '\0' && len--) {
        char c = text[0];
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q = neko_default_val();
            stbtt_GetBakedQuad((stbtt_bakedchar*)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);
            dimensions.x = neko_max(dimensions.x, x);
            dimensions.y = neko_max(dimensions.y, fabsf(q.y0));
            if (include_past_baseline) y_under = neko_max(y_under, fabsf(q.y1));
        }
        text++;
    };

    if (include_past_baseline) dimensions.y += y_under;
    return dimensions;
}

bool neko_util_load_gltf_data_from_file(const char* path, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, uint32_t* mesh_count) {
    // Use cgltf like a boss
    cgltf_options options = neko_default_val();
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(path, "rb", &len);
    neko_println("Loading GLTF: %s", path);

    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&options, file_data, (cgltf_size)len, &data);
    neko_free(file_data);

    if (result != cgltf_result_success) {
        neko_println("Mesh:LoadFromFile:Failed load gltf");
        cgltf_free(data);
        return false;
    }

    // Load buffers as well
    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        cgltf_free(data);
        neko_println("Mesh:LoadFromFile:Failed to load buffers");
        return false;
    }

    // Type of index data
    size_t index_element_size = decl ? decl->index_buffer_element_size : 0;

    // Temporary structures
    neko_dyn_array(neko_vec3) positions = NULL;
    neko_dyn_array(neko_vec3) normals = NULL;
    neko_dyn_array(neko_vec3) tangents = NULL;
    neko_dyn_array(neko_color_t) colors = NULL;
    neko_dyn_array(neko_vec2) uvs = NULL;
    neko_dyn_array(neko_asset_mesh_layout_t) layouts = NULL;
    neko_byte_buffer_t v_data = neko_byte_buffer_new();
    neko_byte_buffer_t i_data = neko_byte_buffer_new();

    // Allocate memory for buffers
    *mesh_count = data->meshes_count;
    *out = (neko_asset_mesh_raw_data_t*)neko_malloc(data->meshes_count * sizeof(neko_asset_mesh_raw_data_t));
    memset(*out, 0, sizeof(neko_asset_mesh_raw_data_t) * data->meshes_count);

    // Iterate through meshes in data
    for (uint32_t i = 0; i < data->meshes_count; ++i) {
        // Initialize mesh data
        neko_asset_mesh_raw_data_t* mesh = out[i];
        mesh->prim_count = data->meshes[i].primitives_count;
        mesh->vertex_sizes = (size_t*)neko_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->index_sizes = (size_t*)neko_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->vertices = (void**)neko_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->indices = (void**)neko_malloc(sizeof(size_t) * mesh->prim_count);

        // For each primitive in mesh
        for (uint32_t p = 0; p < data->meshes[i].primitives_count; ++p) {
            // Clear temp data from previous use
            neko_dyn_array_clear(positions);
            neko_dyn_array_clear(normals);
            neko_dyn_array_clear(tangents);
            neko_dyn_array_clear(uvs);
            neko_dyn_array_clear(colors);
            neko_dyn_array_clear(layouts);
            neko_byte_buffer_clear(&v_data);
            neko_byte_buffer_clear(&i_data);

#define __GLTF_PUSH_ATTR(ATTR, TYPE, COUNT, ARR, ARR_TYPE, LAYOUTS, LAYOUT_TYPE)                                                     \
    do {                                                                                                                             \
        int32_t N = 0;                                                                                                               \
        TYPE* BUF = (TYPE*)ATTR->buffer_view->buffer->data + ATTR->buffer_view->offset / sizeof(TYPE) + ATTR->offset / sizeof(TYPE); \
        neko_assert(BUF);                                                                                                            \
        TYPE V[COUNT] = neko_default_val();                                                                                          \
        /* For each vertex */                                                                                                        \
        for (uint32_t k = 0; k < ATTR->count; k++) {                                                                                 \
            /* For each element */                                                                                                   \
            for (int l = 0; l < COUNT; l++) {                                                                                        \
                V[l] = BUF[N + l];                                                                                                   \
            }                                                                                                                        \
            N += (int32_t)(ATTR->stride / sizeof(TYPE));                                                                             \
            /* Add to temp data array */                                                                                             \
            ARR_TYPE ELEM = neko_default_val();                                                                                      \
            memcpy((void*)&ELEM, (void*)V, sizeof(ARR_TYPE));                                                                        \
            neko_dyn_array_push(ARR, ELEM);                                                                                          \
        }                                                                                                                            \
        /* Push into layout */                                                                                                       \
        neko_asset_mesh_layout_t LAYOUT = neko_default_val();                                                                        \
        LAYOUT.type = LAYOUT_TYPE;                                                                                                   \
        neko_dyn_array_push(LAYOUTS, LAYOUT);                                                                                        \
    } while (0)

            // For each attribute in primitive
            for (uint32_t a = 0; a < data->meshes[i].primitives[p].attributes_count; ++a) {
                // Accessor for attribute data
                cgltf_accessor* attr = data->meshes[i].primitives[p].attributes[a].data;

                // Switch on type for reading data
                switch (data->meshes[i].primitives[p].attributes[a].type) {
                    case cgltf_attribute_type_position: {
                        __GLTF_PUSH_ATTR(attr, float, 3, positions, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION);
                    } break;

                    case cgltf_attribute_type_normal: {
                        __GLTF_PUSH_ATTR(attr, float, 3, normals, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                    } break;

                    case cgltf_attribute_type_tangent: {
                        __GLTF_PUSH_ATTR(attr, float, 3, tangents, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                    } break;

                    case cgltf_attribute_type_texcoord: {
                        __GLTF_PUSH_ATTR(attr, float, 2, uvs, neko_vec2, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                    } break;

                    case cgltf_attribute_type_color: {
                        __GLTF_PUSH_ATTR(attr, uint8_t, 4, colors, neko_color_t, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR);
                    } break;

                    // Not sure what to do with these for now
                    case cgltf_attribute_type_joints: {
                        // Push into layout
                        neko_asset_mesh_layout_t layout = neko_default_val();
                        layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT;
                        neko_dyn_array_push(layouts, layout);
                    } break;

                    case cgltf_attribute_type_weights: {
                        // Push into layout
                        neko_asset_mesh_layout_t layout = neko_default_val();
                        layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT;
                        neko_dyn_array_push(layouts, layout);
                    } break;

                    // Shouldn't hit here...
                    default: {
                    } break;
                }
            }

            // Indices for primitive
            cgltf_accessor* acc = data->meshes[i].primitives[p].indices;

#define __GLTF_PUSH_IDX(BB, ACC, TYPE)                                                                                            \
    do {                                                                                                                          \
        int32_t n = 0;                                                                                                            \
        TYPE* buf = (TYPE*)acc->buffer_view->buffer->data + acc->buffer_view->offset / sizeof(TYPE) + acc->offset / sizeof(TYPE); \
        neko_assert(buf);                                                                                                         \
        TYPE v = 0;                                                                                                               \
        /* For each index */                                                                                                      \
        for (uint32_t k = 0; k < acc->count; k++) {                                                                               \
            /* For each element */                                                                                                \
            for (int l = 0; l < 1; l++) {                                                                                         \
                v = buf[n + l];                                                                                                   \
            }                                                                                                                     \
            n += (int32_t)(acc->stride / sizeof(TYPE));                                                                           \
            /* Add to temp positions array */                                                                                     \
            switch (index_element_size) {                                                                                         \
                case 0:                                                                                                           \
                    neko_byte_buffer_write(BB, uint16_t, (uint16_t)v);                                                            \
                    break;                                                                                                        \
                case 2:                                                                                                           \
                    neko_byte_buffer_write(BB, uint16_t, (uint16_t)v);                                                            \
                    break;                                                                                                        \
                case 4:                                                                                                           \
                    neko_byte_buffer_write(BB, uint32_t, (uint32_t)v);                                                            \
                    break;                                                                                                        \
            }                                                                                                                     \
        }                                                                                                                         \
    } while (0)

            // If indices are available
            if (acc) {
                switch (acc->component_type) {
                    case cgltf_component_type_r_8:
                        __GLTF_PUSH_IDX(&i_data, acc, int8_t);
                        break;
                    case cgltf_component_type_r_8u:
                        __GLTF_PUSH_IDX(&i_data, acc, uint8_t);
                        break;
                    case cgltf_component_type_r_16:
                        __GLTF_PUSH_IDX(&i_data, acc, int16_t);
                        break;
                    case cgltf_component_type_r_16u:
                        __GLTF_PUSH_IDX(&i_data, acc, uint16_t);
                        break;
                    case cgltf_component_type_r_32u:
                        __GLTF_PUSH_IDX(&i_data, acc, uint32_t);
                        break;
                    case cgltf_component_type_r_32f:
                        __GLTF_PUSH_IDX(&i_data, acc, float);
                        break;

                    // Shouldn't hit here
                    default: {
                    } break;
                }
            } else {
                // Iterate over positions size, then just push back indices
                for (uint32_t i = 0; i < neko_dyn_array_size(positions); ++i) {
                    switch (index_element_size) {
                        default:
                        case 0:
                            neko_byte_buffer_write(&i_data, uint16_t, (uint16_t)i);
                            break;
                        case 2:
                            neko_byte_buffer_write(&i_data, uint16_t, (uint16_t)i);
                            break;
                        case 4:
                            neko_byte_buffer_write(&i_data, uint32_t, (uint32_t)i);
                            break;
                    }
                }
            }

            bool warnings[neko_enum_count(neko_asset_mesh_attribute_type)] = neko_default_val();

            // Grab mesh layout pointer to use
            neko_asset_mesh_layout_t* layoutp = decl ? decl->layout : layouts;
            uint32_t layout_ct = decl ? decl->layout_size / sizeof(neko_asset_mesh_layout_t) : neko_dyn_array_size(layouts);

            // Iterate layout to fill data buffers according to provided layout
            {
                uint32_t vct = 0;
                vct = neko_max(vct, neko_dyn_array_size(positions));
                vct = neko_max(vct, neko_dyn_array_size(colors));
                vct = neko_max(vct, neko_dyn_array_size(uvs));
                vct = neko_max(vct, neko_dyn_array_size(normals));
                vct = neko_max(vct, neko_dyn_array_size(tangents));

#define __NEKO_GLTF_WRITE_DATA(IT, VDATA, ARR, ARR_TYPE, ARR_DEF_VAL, LAYOUT_TYPE)              \
    do {                                                                                        \
        /* Grab data at index, if available */                                                  \
        if (IT < neko_dyn_array_size(ARR)) {                                                    \
            neko_byte_buffer_write(&(VDATA), ARR_TYPE, ARR[IT]);                                \
        } else {                                                                                \
            /* Write default value and give warning.*/                                          \
            neko_byte_buffer_write(&(VDATA), ARR_TYPE, ARR_DEF_VAL);                            \
            if (!warnings[LAYOUT_TYPE]) {                                                       \
                neko_println("Warning:Mesh:LoadFromFile:%s:Index out of range.", #LAYOUT_TYPE); \
                warnings[LAYOUT_TYPE] = true;                                                   \
            }                                                                                   \
        }                                                                                       \
    } while (0)

                for (uint32_t it = 0; it < vct; ++it) {
                    // For each attribute in layout
                    for (uint32_t l = 0; l < layout_ct; ++l) {
                        switch (layoutp[l].type) {
                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, positions, neko_vec3, neko_v3(0.f, 0.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, uvs, neko_vec2, neko_v2(0.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, colors, neko_color_t, NEKO_COLOR_WHITE, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, normals, neko_vec3, neko_v3(0.f, 0.f, 1.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, tangents, neko_vec3, neko_v3(0.f, 1.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                            } break;

                            default: {
                            } break;
                        }
                    }
                }
            }

            // Add to out data
            mesh->vertices[p] = neko_malloc(v_data.size);
            mesh->indices[p] = neko_malloc(i_data.size);
            mesh->vertex_sizes[p] = v_data.size;
            mesh->index_sizes[p] = i_data.size;

            // Copy data
            memcpy(mesh->vertices[p], v_data.data, v_data.size);
            memcpy(mesh->indices[p], i_data.data, i_data.size);
        }
    }

    // Free all data at the end
    cgltf_free(data);
    neko_dyn_array_free(positions);
    neko_dyn_array_free(normals);
    neko_dyn_array_free(tangents);
    neko_dyn_array_free(colors);
    neko_dyn_array_free(uvs);
    neko_dyn_array_free(layouts);
    neko_byte_buffer_free(&v_data);
    neko_byte_buffer_free(&i_data);
    return true;
}

bool neko_asset_mesh_load_from_file(const char* path, void* out, neko_asset_mesh_decl_t* decl, void* data_out, size_t data_size) {
    // Cast mesh data to use
    neko_asset_mesh_t* mesh = (neko_asset_mesh_t*)out;

    if (!neko_platform_file_exists(path)) {
        neko_println("Warning:MeshLoadFromFile:File does not exist: %s", path);
        return false;
    }

    // Mesh data to fill out
    uint32_t mesh_count = 0;
    neko_asset_mesh_raw_data_t* meshes = NULL;

    // Get file extension from path
    neko_transient_buffer(file_ext, 32);
    neko_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (neko_string_compare_equal(file_ext, "gltf")) {
        neko_util_load_gltf_data_from_file(path, decl, &meshes, &mesh_count);
    } else {
        neko_println("Warning:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return false;
    }

    // For now, handle meshes with only single mesh count
    if (mesh_count != 1) {
        // Error
        // Free all the memory
        return false;
    }

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i) {
        neko_asset_mesh_raw_data_t* m = &meshes[i];

        for (uint32_t p = 0; p < m->prim_count; ++p) {
            // Construct primitive
            neko_asset_mesh_primitive_t prim = neko_default_val();
            prim.count = m->index_sizes[p] / sizeof(uint16_t);

            // Vertex buffer decl
            neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
            vdesc.data = m->vertices[p];
            vdesc.size = m->vertex_sizes[p];

            // Construct vertex buffer for primitive
            prim.vbo = neko_graphics_vertex_buffer_create(&vdesc);

            // Index buffer decl
            neko_graphics_index_buffer_desc_t idesc = neko_default_val();
            idesc.data = m->indices[p];
            idesc.size = m->index_sizes[p];

            // Construct index buffer for primitive
            prim.ibo = neko_graphics_index_buffer_create(&idesc);

            // Add primitive to mesh
            neko_dyn_array_push(mesh->primitives, prim);
        }
    }

    // Free all mesh data
    return true;
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
                    uint32_t num_decimals = 0;
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
                    uint32_t num_decimals = 0;
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
        neko_ecs() = neko_ecs_make(1024, COMPONENT_COUNT, 2);

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
    static uint32_t curr_ticks = 0;
    static uint32_t prev_ticks = 0;

    // Cache platform pointer
    neko_platform_t* platform = neko_subsystem(platform);
    neko_audio_t* audio = neko_subsystem(audio);

    // Cache times at start of frame
    platform->time.elapsed = (float)neko_platform_elapsed_time();
    platform->time.update = platform->time.elapsed - platform->time.previous;
    platform->time.previous = platform->time.elapsed;

    // Update platform and process input
    neko_platform_update(platform);
    if (!neko_instance()->ctx.game.is_running) {
        neko_instance()->shutdown();
        return;
    }

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

    // Clear all platform events
    neko_dyn_array_clear(platform->events);

    // NOTE: This won't work forever. Must change eventually.
    // Swap all platform window buffers? Sure...
    for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
        neko_platform_window_swap_buffer(it);
    }

    // Frame locking (not sure if this should be done here, but it is what it is)
    platform->time.elapsed = (float)neko_platform_elapsed_time();
    platform->time.render = platform->time.elapsed - platform->time.previous;
    platform->time.previous = platform->time.elapsed;
    platform->time.frame = platform->time.update + platform->time.render;  // Total frame time
    platform->time.delta = platform->time.frame / 1000.f;

    float target = (1000.f / platform->time.max_fps);

    if (platform->time.frame < target) {
        neko_platform_sleep((float)(target - platform->time.frame));

        platform->time.elapsed = (float)neko_platform_elapsed_time();
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