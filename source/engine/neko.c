
#include "neko.h"

#include <setjmp.h>

#include "engine/neko_engine.h"
#include "engine/neko_platform.h"

#if defined(NEKO_DISCRETE_GPU)
// Use discrete GPU by default.
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
__declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

NEKO_STATIC const char* __build_date = __DATE__;
NEKO_STATIC const char* mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
NEKO_STATIC const char mond[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

s32 neko_buildnum(void) {
    s32 m = 0, d = 0, y = 0;
    static s32 b = 0;

    // 优化
    if (b != 0) return b;

    for (m = 0; m < 11; m++) {
        if (!strncmp(&__build_date[0], mon[m], 3)) break;
        d += mond[m];
    }

    d += atoi(&__build_date[4]) - 1;
    y = atoi(&__build_date[7]) - 2022;
    b = d + (s32)((y - 1) * 365.25f);

    if (((y % 4) == 0) && m > 1) b += 1;

    b -= 151;

    return b;
}

/*==========================
// NEKO_OS
==========================*/

NEKO_API_DECL
void* neko_malloc_init_impl(size_t sz) {
    void* data = neko_malloc(sz);
    memset(data, 0, sz);
    return data;
}

NEKO_API_DECL void neko_console_printf(neko_console_t* console, const char* fmt, ...) {
    char tmp[512] = {0};
    va_list args;

    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    int n = sizeof(console->tb) - strlen(console->tb) - 1;
    int resize = strlen(tmp) - n;
    if (resize > 0) {
        memmove(console->tb, console->tb + resize, sizeof(console->tb) - resize);
        n = sizeof(console->tb) - strlen(console->tb) - 1;
    }
    strncat(console->tb, tmp, n);
}

// logging

void neko_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

#define MAX_CALLBACKS 8
#define LOG_USE_COLOR

typedef struct {
    neko_log_fn fn;
    void* udata;
    int level;
} neko_log_callback;

static struct {
    void* udata;
    neko_log_lock_fn lock;
    int level;
    bool quiet;
    neko_log_callback callbacks[MAX_CALLBACKS];
} L;

static const char* level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"};

static const char* level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m"};

static void log_lock(void) {
    if (L.lock) {
        L.lock(true, L.udata);
    }
}

static void log_unlock(void) {
    if (L.lock) {
        L.lock(false, L.udata);
    }
}

const char* log_level_string(int level) { return level_strings[level]; }

void log_set_lock(neko_log_lock_fn fn, void* udata) {
    L.lock = fn;
    L.udata = udata;
}

void log_set_quiet(bool enable) { L.quiet = enable; }

int log_add_callback(neko_log_fn fn, void* udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = (neko_log_callback){fn, udata, level};
            return 0;
        }
    }
    return -1;
}

static void init_event(neko_log_event* ev, void* udata) {
    static u32 t = 0;
    if (!ev->time) {
        ev->time = ++t;
    }
    ev->udata = (FILE*)udata;
}

void neko_log(int level, const char* file, int line, const char* fmt, ...) {
    neko_log_event ev = {
            .fmt = fmt,
            .file = file,
            .line = line,
            .level = level,
    };

    log_lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
#ifdef LOG_USE_COLOR
        fprintf(ev.udata, "%s[%-5s]\x1b[0m \x1b[90m%s:%d:\x1b[0m ", level_colors[ev.level], level_strings[ev.level], neko_fs_get_filename(ev.file), ev.line);
#else
        fprintf(ev.udata, "%-5s %s:%d: ", level_strings[ev.level], neko_fs_get_filename(ev.file), ev.line);
#endif
        vfprintf(ev.udata, ev.fmt, ev.ap);
        fprintf(ev.udata, "\n");
        fflush(ev.udata);

        if (NULL != neko_instance() && NULL != neko_instance()->console) {
            char buffer[512] = NEKO_DEFAULT_VAL();
            vsnprintf(buffer, 512, ev.fmt, ev.ap);
            neko_console_printf(neko_instance()->console, "%-5s %s:%d: ", level_strings[ev.level], neko_fs_get_filename(ev.file), ev.line);
            neko_console_printf(neko_instance()->console, buffer);
            neko_console_printf(neko_instance()->console, "\n");
        }
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        neko_log_callback* cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    log_unlock();
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

    // if (sz == 4096) NEKO_ASSERT(0);

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
    // 检查是否需要调整大小
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
    // 写入字符串的大小
    u32 str_len = neko_string_length(str);
    neko_byte_buffer_write(buffer, uint16_t, str_len);

    size_t i;
    for (i = 0; i < str_len; ++i) {
        neko_byte_buffer_write(buffer, u8, str[i]);
    }
}

void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str) {
    // 从缓冲区读取字符串的大小
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
        NEKO_ASSERT(false);
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

    // 仅使用标头信息创建新的 neko_dyn_array
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

    if (NEKO_MEM_CHECK && size < 64) NEKO_WARN("small size of memory blocks should use GC (%s:%d)", neko_fs_get_filename(file), line);

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
    memset(mem, 0, size);
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

#if 1
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
#endif

int neko_mem_check_leaks(bool detailed) {
    neko_mem_alloc_info_t* head = neko_mem_alloc_head();
    neko_mem_alloc_info_t* next = head->next;
    int leaks = 0;

    size_t leaks_size = 0;

    while (next != head) {
        if (!leaks && detailed) NEKO_INFO("memory leaks detected (see below).");
        if (detailed) {
            char info[128];
            neko_snprintf(info, 128, "LEAKED %zu bytes from file \"%s\" at line %d from address %p.", next->size, neko_fs_get_filename(next->file), next->line, (void*)(next + 1));
            neko_println("  | %s", info);
        }
        leaks_size += next->size;
        next = next->next;
        leaks = 1;
    }

    if (leaks) {
        f32 megabytes = (f32)leaks_size / 1048576.f;
        NEKO_WARN("memory leaks detected with %zu bytes equal to %.4f MB.", leaks_size, megabytes);
    } else {
        NEKO_INFO("no memory leaks detected.");
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

void __neko_mem_init(int argc, char** argv) {}

void __neko_mem_fini() { neko_mem_check_leaks(true); }

// typedef struct neko_memory_block_t {
//     u8* data;
//     size_t size;
// } neko_memory_block_t;

NEKO_API_DECL neko_memory_block_t neko_memory_block_new(size_t sz) {
    neko_memory_block_t mem = NEKO_DEFAULT_VAL();
    mem.data = (u8*)neko_malloc(sz);
    NEKO_ASSERT(mem.data);
    memset(mem.data, 0, sz);
    mem.size = sz;
    return mem;
}

NEKO_API_DECL void neko_memory_block_free(neko_memory_block_t* mem) {
    NEKO_ASSERT(mem);
    NEKO_ASSERT(mem->data);
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
    neko_linear_allocator_t la = NEKO_DEFAULT_VAL();
    la.memory = (u8*)neko_malloc(sz);
    memset(la.memory, 0, sz);
    la.offset = 0;
    la.total_size = sz;
    return la;
}

NEKO_API_DECL void neko_linear_allocator_free(neko_linear_allocator_t* la) {
    NEKO_ASSERT(la);
    NEKO_ASSERT(la->memory);
    neko_free(la->memory);
    la->memory = NULL;
}

NEKO_API_DECL void* neko_linear_allocator_allocate(neko_linear_allocator_t* la, size_t sz, size_t alignment) {
    NEKO_ASSERT(la);
    size_t padding = 0;
    size_t padding_address = 0;
    size_t cur_address = (size_t)la->memory + la->offset;

    // 计算所需的对齐方式
    if (alignment != 0 && la->offset % alignment != 0) {
        padding = neko_memory_calc_padding(cur_address, alignment);
    }

    // 无法分配 (没有足够的可用内存)
    if (la->offset + padding + sz > la->total_size) {
        return NULL;
    }

    // 申请内存并且返回地址
    la->offset += padding;
    size_t next_address = cur_address + padding;
    la->offset += sz;
    return (void*)next_address;
}

NEKO_API_DECL void neko_linear_allocator_clear(neko_linear_allocator_t* la) {
    NEKO_ASSERT(la);
    la->offset = 0;
}

/*================================================================================
// Stack Allocator
================================================================================*/

NEKO_API_DECL neko_stack_allocator_t neko_stack_allocator_new(size_t sz) {
    neko_stack_allocator_t alloc = NEKO_DEFAULT_VAL();
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
    neko_paged_allocator_t pa = NEKO_DEFAULT_VAL();
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
        neko_paged_allocator_page_t* page = (neko_paged_allocator_page_t*)neko_malloc_init_impl(pa->block_size * pa->blocks_per_page + sizeof(neko_paged_allocator_page_t));
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
    neko_heap_allocator_t ha = NEKO_DEFAULT_VAL();
    ha.memory = (neko_heap_allocator_header_t*)neko_malloc_init_impl(NEKO_HEAP_ALLOC_DEFAULT_SIZE);
    ha.memory->next = NULL;
    ha.memory->prev = NULL;
    ha.memory->size = NEKO_HEAP_ALLOC_DEFAULT_SIZE;

    ha.free_blocks = (neko_heap_allocator_free_block_t*)neko_malloc_init_impl(sizeof(neko_heap_allocator_free_block_t) * NEKO_HEAP_ALLOC_DEFAULT_CAPCITY);
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

b32 neko_util_load_texture_data_from_file(const char* file_path, s32* width, s32* height, u32* num_comps, void** data, b32 flip_vertically_on_load) {
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(file_path, "rb", &len);
    NEKO_ASSERT(file_data);
    b32 ret = neko_util_load_texture_data_from_memory(file_data, len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        NEKO_WARN("Could not load texture: %s", file_path);
    }
    neko_safe_free(file_data);
    return ret;
}

/*========================
// Random
========================*/

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
    neko_color_t c = NEKO_DEFAULT_VAL();
    c.r = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.g = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.b = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.a = (u8)neko_rand_gen_range_long(rand, 0, 255);
    return c;
}

//=============================
// CVar
//=============================

void __neko_config_init() {
    neko_cv() = (neko_config_t*)neko_malloc(sizeof(neko_config_t));
    neko_cv()->cvars = neko_dyn_array_new(neko_cvar_t);

    neko_cvar_new("test_cvar", __NEKO_CONFIG_TYPE_INT, 1001);

    neko_cvar_new_str("test_cvar_str", __NEKO_CONFIG_TYPE_STRING, "啊!能不能有中文?");

    // TODO: 23/8/10 控制台以及相关cvar注册
}

void __neko_config_free() {

    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        if (neko_cv()->cvars->type == __NEKO_CONFIG_TYPE_STRING) {
            neko_free(neko_cv()->cvars[i].value.s);
            neko_cv()->cvars[i].value.s = NULL;
        }
    }
    neko_dyn_array_free(neko_cv()->cvars);

    neko_free(neko_cv());
    neko_cv() = NULL;
}

neko_cvar_t* __neko_config_get(const_str name) {
    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        if (strcmp(neko_cv()->cvars[i].name, name) == 0) {
            return &neko_cv()->cvars[i];
        }
    }
    return NULL;
}

void neko_config_print() {
    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        neko_cvar_print(&neko_cv()->cvars[i]);
    }
}

//=============================
// Console
//=============================

NEKO_API_DECL void neko_console(neko_console_t* console, neko_ui_context_t* ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t* desc) {
    if (console->open)
        console->y += (screen.h * console->size - console->y) * console->open_speed;
    else if (!console->open && console->y >= 1.0f)
        console->y += (0 - console->y) * console->close_speed;
    else
        return;

    const f32 sz = NEKO_MIN(console->y, 26);
    if (neko_ui_window_begin_ex(ctx, "neko_console_content", neko_ui_rect(screen.x, screen.y, screen.w, console->y - sz), NULL, NULL,
                                NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_NORESIZE | NEKO_UI_OPT_NODOCK | NEKO_UI_OPT_FORCEFOCUS | NEKO_UI_OPT_HOLDFOCUS)) {
        neko_ui_layout_row(ctx, 1, neko_ui_widths(-1), 0);
        neko_ui_text(ctx, console->tb);
        // neko_imgui_draw_text(console->tb, NEKO_COLOR_WHITE, 10.f, 10.f, true, NEKO_COLOR_BLACK);
        if (console->autoscroll) neko_ui_get_current_container(ctx)->scroll.y = sizeof(console->tb) * 7 + 100;
        neko_ui_container_t* ctn = neko_ui_get_current_container(ctx);
        neko_ui_bring_to_front(ctx, ctn);
        neko_ui_window_end(ctx);
    }

    if (neko_ui_window_begin_ex(ctx, "neko_console_input", neko_ui_rect(screen.x, screen.y + console->y - sz, screen.w, sz), NULL, NULL,
                                NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_NORESIZE | NEKO_UI_OPT_NODOCK | NEKO_UI_OPT_NOHOVER | NEKO_UI_OPT_NOINTERACT)) {
        int len = strlen(console->cb[0]);
        neko_ui_layout_row(ctx, 3, neko_ui_widths(14, len * 7 + 2, 10), 0);
        neko_ui_text(ctx, "$>");
        neko_ui_text(ctx, console->cb[0]);

        if (!console->open || !console->last_open_state) {
            goto console_input_handling_done;
        }

        // 处理文本输入
        int32_t n = NEKO_MIN(sizeof(*console->cb) - len - 1, (int32_t)strlen(ctx->input_text));

        if (neko_platform_key_pressed(NEKO_KEYCODE_UP)) {
            console->current_cb_idx++;
            if (console->current_cb_idx >= NEKO_ARR_SIZE(console->cb)) {
                console->current_cb_idx = NEKO_ARR_SIZE(console->cb) - 1;
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_DOWN)) {
            console->current_cb_idx--;
            if (console->current_cb_idx <= 0) {
                console->current_cb_idx = 0;
                memset(&console->cb[0], 0, sizeof(*console->cb));
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_ENTER)) {
            console->current_cb_idx = 0;
            neko_console_printf(console, "$ %s\n", console->cb[0]);

            memmove((uint8_t*)console->cb + sizeof(*console->cb), (uint8_t*)console->cb, sizeof(console->cb) - sizeof(*console->cb));

            if (console->cb[0][0] && console->commands) {
                char* tmp = console->cb[0];
                int argc = 1;
                while ((tmp = strchr(tmp, ' '))) {
                    argc++;
                    tmp++;
                }

                tmp = console->cb[0];
                char* last_pos = console->cb[0];
                char** argv = (char**)neko_safe_malloc(argc * sizeof(char*));
                int i = 0;
                while ((tmp = strchr(tmp, ' '))) {
                    *tmp = 0;
                    argv[i++] = last_pos;
                    last_pos = ++tmp;
                }
                argv[argc - 1] = last_pos;

                for (int i = 0; i < console->commands_len; i++) {
                    if (console->commands[i].name && console->commands[i].func && strcmp(argv[0], console->commands[i].name) == 0) {
                        console->commands[i].func(argc, argv);
                        goto console_command_found;
                    }
                }
                neko_console_printf(console, "[neko_console]: unrecognized command '%s'\n", argv[0]);
            console_command_found:
                console->cb[0][0] = '\0';
                neko_safe_free(argv);
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE)) {
            console->current_cb_idx = 0;
            // 跳过 utf-8 连续字节
            while ((console->cb[0][--len] & 0xc0) == 0x80 && len > 0);
            console->cb[0][len] = '\0';
        } else if (n > 0 && !neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
            console->current_cb_idx = 0;
            if (len + n + 1 < sizeof(*console->cb)) {
                memcpy(console->cb[0] + len, ctx->input_text, n);
                len += n;
                console->cb[0][len] = '\0';
            }
        }

    console_input_handling_done:

        // 闪烁光标
        neko_ui_get_layout(ctx)->body.x += len * 7 - 5;
        if ((int)(neko_platform_elapsed_time() / 666.0f) & 1) neko_ui_text(ctx, "|");

        neko_ui_container_t* ctn = neko_ui_get_current_container(ctx);
        neko_ui_bring_to_front(ctx, ctn);

        neko_ui_window_end(ctx);
    }

    console->last_open_state = console->open;
}

static bool window = 1, embeded;
static int summons;

static void toggle_window(int argc, char** argv);
static void toggle_embedded(int argc, char** argv);
static void help(int argc, char** argv);
static void echo(int argc, char** argv);
static void spam(int argc, char** argv);
static void crash(int argc, char** argv);
void summon(int argc, char** argv);
void exec(int argc, char** argv);
void sz(int argc, char** argv);

neko_console_command_t commands[] = {{
                                             .func = echo,
                                             .name = "echo",
                                             .desc = "repeat what was entered",
                                     },
                                     {
                                             .func = spam,
                                             .name = "spam",
                                             .desc = "send the word arg1, arg2 amount of times",
                                     },
                                     {
                                             .func = help,
                                             .name = "help",
                                             .desc = "sends a list of commands",
                                     },
                                     {
                                             .func = toggle_window,
                                             .name = "window",
                                             .desc = "toggles gui window",
                                     },
                                     {
                                             .func = toggle_embedded,
                                             .name = "embed",
                                             .desc = "places the console inside the window",
                                     },
                                     {
                                             .func = summon,
                                             .name = "summon",
                                             .desc = "summons a gui window",
                                     },
                                     {
                                             .func = sz,
                                             .name = "sz",
                                             .desc = "change console size",
                                     },
                                     {
                                             .func = crash,
                                             .name = "crash",
                                             .desc = "test crashhhhhhhhh.....",
                                     },
                                     {
                                             .func = exec,
                                             .name = "exec",
                                             .desc = "run nekoscript",
                                     }};

neko_console_t g_console = {
        .tb = "",
        .cb = {},
        .size = 0.4,
        .open_speed = 0.2,
        .close_speed = 0.3,
        .autoscroll = true,
        .commands = commands,
        .commands_len = NEKO_ARR_SIZE(commands),
};

void sz(int argc, char** argv) {
    if (argc != 2) {
        neko_console_printf(&g_console, "[sz]: needs 1 argument!\n");
        return;
    }
    f32 sz = atof(argv[1]);
    if (sz > 1 || sz < 0) {
        neko_console_printf(&g_console, "[sz]: number needs to be between (0, 1)");
        return;
    }
    g_console.size = sz;

    neko_console_printf(&g_console, "console size is now %f\n", sz);
}

void toggle_window(int argc, char** argv) {
    if (window && embeded)
        neko_console_printf(&g_console, "Unable to turn off window, console is embeded!\n");
    else
        neko_console_printf(&g_console, "GUI Window turned %s\n", (window = !window) ? "on" : "off");
}

void toggle_embedded(int argc, char** argv) {
    if (!window && !embeded)
        neko_console_printf(&g_console, "Unable to embed into window, open window first!\n");
    else
        neko_console_printf(&g_console, "console embedded turned %s\n", (embeded = !embeded) ? "on" : "off");
}

void summon(int argc, char** argv) {
    neko_console_printf(&g_console, "A summoner has cast his spell! A window has appeared!!!!\n");
    summons++;
}

void crash(int argc, char** argv) {
    const_str trace_info = neko_platform_stacktrace();
    // neko_platform_msgbox(std::format("Crash...\n{0}", trace_info).c_str());
}

void spam(int argc, char** argv) {
    int count;
    if (argc != 3) goto spam_invalid_command;
    count = atoi(argv[2]);
    if (!count) goto spam_invalid_command;
    while (count--) neko_console_printf(&g_console, "%s\n", argv[1]);
    return;
spam_invalid_command:
    neko_console_printf(&g_console, "[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

void echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) neko_console_printf(&g_console, "%s ", argv[i]);
    neko_console_printf(&g_console, "\n");
}

void exec(int argc, char** argv) {
    if (argc != 2) return;
    // neko_vm_interpreter(argv[1]);
}

void help(int argc, char** argv) {
    for (int i = 0; i < NEKO_ARR_SIZE(commands); i++) {
        if (commands[i].name) neko_console_printf(&g_console, "* Command: %s\n", commands[i].name);
        if (commands[i].desc) neko_console_printf(&g_console, "- desc: %s\n", commands[i].desc);
    }
}