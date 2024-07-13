#include "neko.h"

#include <setjmp.h>

#include <cstdio>
#include <mutex>

#include "engine/neko.hpp"
#include "engine/neko_api.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_imgui.hpp"
#include "engine/neko_lua.h"
#include "engine/neko_platform.h"
#include "engine/neko_reflection.hpp"

// deps
#if defined(NEKO_PF_WIN)
#include <Windows.h>
#include <direct.h>
#endif

#include <miniz.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(NEKO_PF_WIN)
#include <direct.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#elif defined(NEKO_PF_WEB)
#include <unistd.h>

#elif defined(NEKO_PF_LINUX)
#include <sched.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#endif

NEKO_STATIC const char *__build_date = __DATE__;
NEKO_STATIC const char *mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
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
void *neko_malloc_init_impl(size_t sz) {
    void *data = neko_malloc(sz);
    memset(data, 0, sz);
    return data;
}

NEKO_API_DECL void neko_console_printf(neko_console_t *console, const char *fmt, ...) {
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

NEKO_API_DECL void neko_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static struct {
    void *udata;
    neko_log_lock_fn lock;
    int level;
    bool quiet;
} L;

static const char *level_strings[] = {"T", "D", "I", "W", "E"};

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

const char *log_level_string(int level) { return level_strings[level]; }

void log_set_lock(neko_log_lock_fn fn, void *udata) {
    L.lock = fn;
    L.udata = udata;
}

void log_set_quiet(bool enable) { L.quiet = enable; }

static void init_event(neko_log_event *ev, void *udata) {
    static u32 t = 0;
    if (!ev->time) {
        ev->time = ++t;
    }
    ev->udata = (FILE *)udata;
}

void neko_log(int level, const char *file, int line, const char *fmt, ...) {
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
        fprintf(ev.udata, "%-1s %s:%d: ", level_strings[ev.level], neko_util_get_filename(ev.file), ev.line);
        vfprintf(ev.udata, ev.fmt, ev.ap);
        fprintf(ev.udata, "\n");
        fflush(ev.udata);
        va_end(ev.ap);

        // console callback
        if (NULL != neko_instance() && NULL != neko_instance()->console) {
            va_start(ev.ap, fmt);
            char buffer[512] = NEKO_DEFAULT_VAL();
            vsnprintf(buffer, 512, ev.fmt, ev.ap);
            neko_console_printf(neko_instance()->console, "%-1s %s:%d: ", level_strings[ev.level], neko_util_get_filename(ev.file), ev.line);
            neko_console_printf(neko_instance()->console, buffer);
            neko_console_printf(neko_instance()->console, "\n");
            va_end(ev.ap);
        }
    }

    log_unlock();
}

/*========================
// neko_byte_buffer
========================*/

void neko_byte_buffer_init(neko_byte_buffer_t *buffer) {
    buffer->data = (u8 *)neko_safe_malloc(NEKO_BYTE_BUFFER_DEFAULT_CAPCITY);
    buffer->capacity = NEKO_BYTE_BUFFER_DEFAULT_CAPCITY;
    buffer->size = 0;
    buffer->position = 0;
}

neko_byte_buffer_t neko_byte_buffer_new() {
    neko_byte_buffer_t buffer;
    neko_byte_buffer_init(&buffer);
    return buffer;
}

void neko_byte_buffer_free(neko_byte_buffer_t *buffer) {
    if (buffer && buffer->data) {
        neko_safe_free(buffer->data);
    }
}

void neko_byte_buffer_clear(neko_byte_buffer_t *buffer) {
    buffer->size = 0;
    buffer->position = 0;
}

bool neko_byte_buffer_empty(neko_byte_buffer_t *buffer) { return (buffer->size == 0); }

size_t neko_byte_buffer_size(neko_byte_buffer_t *buffer) { return buffer->size; }

void neko_byte_buffer_resize(neko_byte_buffer_t *buffer, size_t sz) {

    // if (sz == 4096) NEKO_ASSERT(0);

    u8 *data = (u8 *)neko_safe_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;
    buffer->capacity = (u32)sz;
}

void neko_byte_buffer_copy_contents(neko_byte_buffer_t *dst, neko_byte_buffer_t *src) {
    neko_byte_buffer_seek_to_beg(dst);
    neko_byte_buffer_seek_to_beg(src);
    neko_byte_buffer_write_bulk(dst, src->data, src->size);
}

void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t *buffer) { buffer->position = 0; }

void neko_byte_buffer_seek_to_end(neko_byte_buffer_t *buffer) { buffer->position = buffer->size; }

void neko_byte_buffer_advance_position(neko_byte_buffer_t *buffer, size_t sz) { buffer->position += (u32)sz; }

void neko_byte_buffer_write_bulk(neko_byte_buffer_t *buffer, void *src, size_t size) {
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

void neko_byte_buffer_read_bulk(neko_byte_buffer_t *buffer, void **dst, size_t size) {
    memcpy(*dst, (buffer->data + buffer->position), size);
    buffer->position += (u32)size;
}

void neko_byte_buffer_write_str(neko_byte_buffer_t *buffer, const char *str) {
    // 写入字符串的大小
    u32 str_len = neko_string_length(str);
    neko_byte_buffer_write(buffer, uint16_t, str_len);

    size_t i;
    for (i = 0; i < str_len; ++i) {
        neko_byte_buffer_write(buffer, u8, str[i]);
    }
}

void neko_byte_buffer_read_str(neko_byte_buffer_t *buffer, char *str) {
    // 从缓冲区读取字符串的大小
    uint16_t sz;
    neko_byte_buffer_read(buffer, uint16_t, &sz);

    u32 i;
    for (i = 0; i < sz; ++i) {
        neko_byte_buffer_read(buffer, u8, &str[i]);
    }
    str[i] = '\0';
}

neko_result neko_byte_buffer_write_to_file(neko_byte_buffer_t *buffer, const char *output_path) { return neko_pf_write_file_contents(output_path, "wb", buffer->data, buffer->size); }

neko_result neko_byte_buffer_read_from_file(neko_byte_buffer_t *buffer, const char *file_path) {
    if (!buffer) return NEKO_RESULT_FAILURE;

    if (buffer->data) {
        neko_byte_buffer_free(buffer);
    }

    buffer->data = (u8 *)neko_pf_read_file_contents(file_path, "rb", (size_t *)&buffer->size);
    if (!buffer->data) {
        NEKO_ASSERT(false);
        return NEKO_RESULT_FAILURE;
    }

    buffer->position = 0;
    buffer->capacity = buffer->size;
    return NEKO_RESULT_SUCCESS;
}

NEKO_API_DECL void neko_byte_buffer_memset(neko_byte_buffer_t *buffer, u8 val) { memset(buffer->data, val, buffer->capacity); }

/*========================
// Dynamic Array
========================*/

NEKO_API_DECL void *neko_dyn_array_resize_impl(void *arr, size_t sz, size_t amount) {
    size_t capacity;

    if (arr) {
        capacity = amount;
    } else {
        capacity = 0;
    }

    // 仅使用标头信息创建新的 neko_dyn_array
    neko_dyn_array *data = (neko_dyn_array *)neko_realloc(arr ? neko_dyn_array_head(arr) : 0, capacity * sz + sizeof(neko_dyn_array));

    if (data) {
        if (!arr) {
            data->size = 0;
        }
        data->capacity = (s32)capacity;
        return ((s32 *)data + 2);
    }

    return NULL;
}

NEKO_API_DECL void **neko_dyn_array_init(void **arr, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array *data = (neko_dyn_array *)neko_malloc(val_len + sizeof(neko_dyn_array));  // Allocate capacity of one
        data->size = 0;
        data->capacity = 1;
        *arr = ((s32 *)data + 2);
    }
    return arr;
}

NEKO_API_DECL void neko_dyn_array_push_data(void **arr, void *val, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array_init(arr, val_len);
    }
    if (neko_dyn_array_need_grow(*arr, 1)) {
        s32 capacity = neko_dyn_array_capacity(*arr) * 2;

        // Create new neko_dyn_array with just the header information
        neko_dyn_array *data = (neko_dyn_array *)neko_realloc(neko_dyn_array_head(*arr), capacity * val_len + sizeof(neko_dyn_array));

        if (data) {
            data->capacity = capacity;
            *arr = ((s32 *)data + 2);
        }
    }
    size_t offset = neko_dyn_array_size(*arr);
    memcpy(((u8 *)(*arr)) + offset * val_len, val, val_len);
    neko_dyn_array_head(*arr)->size++;
}

/*========================
// Hash Table
========================*/

NEKO_API_DECL void __neko_hash_table_init_impl(void **ht, size_t sz) { *ht = neko_malloc(sz); }

/*========================
// Slot Array
========================*/

NEKO_API_DECL void **neko_slot_array_init(void **sa, size_t sz) {
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

NEKO_API_DECL void **neko_slot_map_init(void **sm) {
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

Allocator *g_allocator;

void *__neko_mem_safe_calloc(size_t count, size_t element_size, const char *file, int line) {
    size_t size = count * element_size;
    void *mem = g_allocator->alloc(size, file, line);
    memset(mem, 0, size);
    return mem;
}

void *__neko_mem_safe_realloc(void *ptr, size_t new_size, const char *file, int line) {

    NEKO_ASSERT(0);

    return NULL;

    // if (new_size == 0) {
    //     g_allocator->free(ptr);  // 如果新大小为 0 则直接释放原内存块并返回 NULL
    //     return NULL;
    // }
    // if (ptr == NULL) {
    //     return g_allocator->alloc(new_size, file, line);  // 如果原指针为空 则等同于 alloc
    // }
    // void *new_ptr = g_allocator->alloc(new_size, file, line);  // 分配新大小的内存块
    // if (new_ptr != NULL) {
    //     // 复制旧内存块中的数据到新内存块
    //     neko_mem_alloc_info_t *info = (neko_mem_alloc_info_t *)ptr - 1;
    //     size_t old_size = info->size;
    //     size_t copy_size = (old_size < new_size) ? old_size : new_size;
    //     memcpy(new_ptr, ptr, copy_size);
    //     g_allocator->free(ptr);  // 释放旧内存块
    // }
    // return new_ptr;
}

void *DebugAllocator::alloc(size_t bytes, const char *file, s32 line) {
    neko::lock_guard lock{&mtx};

    DebugAllocInfo *info = (DebugAllocInfo *)malloc(NEKO_OFFSET(DebugAllocInfo, buf[bytes]));
    info->file = file;
    info->line = line;
    info->size = bytes;
    info->prev = nullptr;
    info->next = head;
    if (head != nullptr) {
        head->prev = info;
    }
    head = info;
    return info->buf;
}

void DebugAllocator::free(void *ptr) {
    if (ptr == nullptr) {
        return;
    }

    neko::lock_guard lock{&mtx};

    DebugAllocInfo *info = (DebugAllocInfo *)((u8 *)ptr - NEKO_OFFSET(DebugAllocInfo, buf));

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

void *DebugAllocator::realloc(void *ptr, size_t new_size, const char *file, s32 line) {
    if (ptr == nullptr) {
        // If the pointer is null, just allocate new memory
        return alloc(new_size, file, line);
    }

    if (new_size == 0) {
        // If the new size is zero, free the memory and return null
        free(ptr);
        return nullptr;
    }

    neko::lock_guard lock{&mtx};

    DebugAllocInfo *old_info = (DebugAllocInfo *)((u8 *)ptr - offsetof(DebugAllocInfo, buf));

    // Allocate new memory block with the new size
    DebugAllocInfo *new_info = (DebugAllocInfo *)malloc(NEKO_OFFSET(DebugAllocInfo, buf[new_size]));
    if (new_info == nullptr) {
        return nullptr;  // Allocation failed
    }

    // Copy data from old memory block to new memory block
    size_t copy_size = old_info->size < new_size ? old_info->size : new_size;
    memcpy(new_info->buf, old_info->buf, copy_size);

    // Update new memory block information
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

    // Free the old memory block
    ::free(old_info);

    return new_info->buf;
}

void DebugAllocator::dump_allocs() {
    s32 allocs = 0;
    for (DebugAllocInfo *info = head; info != nullptr; info = info->next) {
        neko_printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size, info->file, info->line);
        allocs++;
    }
    neko_printf("  --- %d allocation(s) ---\n", allocs);
}

void *neko_capi_safe_malloc(size_t size) { return mem_alloc(size); }

/*========================
// Random
========================*/

#define NEKO_RAND_UPPER_MASK 0x80000000
#define NEKO_RAND_LOWER_MASK 0x7fffffff
#define NEKO_RAND_TEMPERING_MASK_B 0x9d2c5680
#define NEKO_RAND_TEMPERING_MASK_C 0xefc60000

NEKO_API_DECL void _neko_rand_seed_impl(neko_mt_rand_t *rand, uint64_t seed) {
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

NEKO_API_DECL int64_t neko_rand_gen_long(neko_mt_rand_t *rand) {
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

NEKO_API_DECL double neko_rand_gen(neko_mt_rand_t *rand) { return ((double)neko_rand_gen_long(rand) / (uint64_t)0xffffffff); }

NEKO_API_DECL int64_t neko_rand_gen_range_long(neko_mt_rand_t *rand, int32_t min, int32_t max) { return (int64_t)(floorf(neko_rand_gen_range(rand, (double)min, (double)max))); }

NEKO_API_DECL double neko_rand_gen_range(neko_mt_rand_t *rand, double min, double max) { return neko_map_range(0.0, 1.0, min, max, neko_rand_gen(rand)); }

NEKO_API_DECL neko_color_t neko_rand_gen_color(neko_mt_rand_t *rand) {
    neko_color_t c = NEKO_DEFAULT_VAL();
    c.r = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.g = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.b = (u8)neko_rand_gen_range_long(rand, 0, 255);
    c.a = (u8)neko_rand_gen_range_long(rand, 0, 255);
    return c;
}

//=============================
// Console
//=============================

NEKO_API_DECL void neko_console(neko_console_t *console, neko_ui_context_t *ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t *desc) {
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
        neko_ui_container_t *ctn = neko_ui_get_current_container(ctx);
        neko_ui_bring_to_front(ctx, ctn);
        neko_ui_window_end(ctx);
    }

    if (neko_ui_window_begin_ex(ctx, "neko_console_input", neko_ui_rect(screen.x, screen.y + console->y - sz, screen.w, sz), NULL, NULL,
                                NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_NORESIZE | NEKO_UI_OPT_NODOCK | NEKO_UI_OPT_NOHOVER | NEKO_UI_OPT_NOINTERACT)) {
        int len = strlen(console->cb[0]);
        neko_ui_layout_row(ctx, 3, neko_ui_widths(14, len * 7 + 2, 10), 0);
        neko_ui_text(ctx, "$>");
        neko_ui_text(ctx, console->cb[0]);

        s32 n;

        if (!console->open || !console->last_open_state) {
            goto console_input_handling_done;
        }

        // 处理文本输入
        n = NEKO_MIN(sizeof(*console->cb) - len - 1, (int32_t)strlen(ctx->input_text));

        if (neko_pf_key_pressed(NEKO_KEYCODE_UP)) {
            console->current_cb_idx++;
            if (console->current_cb_idx >= NEKO_ARR_SIZE(console->cb)) {
                console->current_cb_idx = NEKO_ARR_SIZE(console->cb) - 1;
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_pf_key_pressed(NEKO_KEYCODE_DOWN)) {
            console->current_cb_idx--;
            if (console->current_cb_idx <= 0) {
                console->current_cb_idx = 0;
                memset(&console->cb[0], 0, sizeof(*console->cb));
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_pf_key_pressed(NEKO_KEYCODE_ENTER)) {
            console->current_cb_idx = 0;
            neko_console_printf(console, "$ %s\n", console->cb[0]);

            memmove((uint8_t *)console->cb + sizeof(*console->cb), (uint8_t *)console->cb, sizeof(console->cb) - sizeof(*console->cb));

            if (console->cb[0][0] && console->commands) {
                char *tmp = console->cb[0];
                int argc = 1;
                while ((tmp = strchr(tmp, ' '))) {
                    argc++;
                    tmp++;
                }

                tmp = console->cb[0];
                char *last_pos = console->cb[0];
                char **argv = (char **)neko_safe_malloc(argc * sizeof(char *));
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
        } else if (neko_pf_key_pressed(NEKO_KEYCODE_BACKSPACE)) {
            console->current_cb_idx = 0;
            // 跳过 utf-8 连续字节
            while ((console->cb[0][--len] & 0xc0) == 0x80 && len > 0);
            console->cb[0][len] = '\0';
        } else if (n > 0 && !neko_pf_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
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
        if ((int)(neko_pf_elapsed_time() / 666.0f) & 1) neko_ui_text(ctx, "|");

        neko_ui_container_t *ctn = neko_ui_get_current_container(ctx);
        neko_ui_bring_to_front(ctx, ctn);

        neko_ui_window_end(ctx);
    }

    console->last_open_state = console->open;
}

static bool window = 1, embeded;
static int summons;

static void toggle_window(int argc, char **argv);
static void toggle_embedded(int argc, char **argv);
static void help(int argc, char **argv);
static void echo(int argc, char **argv);
static void spam(int argc, char **argv);
static void crash(int argc, char **argv);
void summon(int argc, char **argv);
void exec(int argc, char **argv);
void sz(int argc, char **argv);

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

void sz(int argc, char **argv) {
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

void toggle_window(int argc, char **argv) {
    if (window && embeded)
        neko_console_printf(&g_console, "Unable to turn off window, console is embeded!\n");
    else
        neko_console_printf(&g_console, "GUI Window turned %s\n", (window = !window) ? "on" : "off");
}

void toggle_embedded(int argc, char **argv) {
    if (!window && !embeded)
        neko_console_printf(&g_console, "Unable to embed into window, open window first!\n");
    else
        neko_console_printf(&g_console, "console embedded turned %s\n", (embeded = !embeded) ? "on" : "off");
}

void summon(int argc, char **argv) {
    neko_console_printf(&g_console, "A summoner has cast his spell! A window has appeared!!!!\n");
    summons++;
}

void crash(int argc, char **argv) {
    const_str trace_info = neko_pf_stacktrace();
    // neko_pf_msgbox(std::format("Crash...\n{0}", trace_info).c_str());
}

void spam(int argc, char **argv) {
    int count;
    if (argc != 3) goto spam_invalid_command;
    count = atoi(argv[2]);
    if (!count) goto spam_invalid_command;
    while (count--) neko_console_printf(&g_console, "%s\n", argv[1]);
    return;
spam_invalid_command:
    neko_console_printf(&g_console, "[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

void echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) neko_console_printf(&g_console, "%s ", argv[i]);
    neko_console_printf(&g_console, "\n");
}

void exec(int argc, char **argv) {
    if (argc != 2) return;
    // neko_vm_interpreter(argv[1]);
}

void help(int argc, char **argv) {
    for (int i = 0; i < NEKO_ARR_SIZE(commands); i++) {
        if (commands[i].name) neko_console_printf(&g_console, "* Command: %s\n", commands[i].name);
        if (commands[i].desc) neko_console_printf(&g_console, "- desc: %s\n", commands[i].desc);
    }
}

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    size_t wlen = wtf8_to_utf16_length(str.data(), str.size());
    if (wlen == (size_t)-1) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    wtf8_to_utf16(str.data(), str.size(), wresult.data(), wlen);
    return wresult;
}

std::string w2u(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    size_t len = wtf8_from_utf16_length(wstr.data(), wstr.size());
    std::string result(len, '\0');
    wtf8_from_utf16(wstr.data(), wstr.size(), result.data(), len);
    return result;
}
}  // namespace neko::wtf8

#if defined(NEKO_PF_WIN)

namespace neko::win {
std::wstring u2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    const int wlen = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (wlen <= 0) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wresult.data(), static_cast<int>(wresult.size()));
    return wresult;
}

std::string w2u(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    const int len = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, 0, 0);
    if (len <= 0) {
        return "";
    }
    std::string result(len, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), result.data(), static_cast<int>(result.size()), 0, 0);
    return result;
}

std::wstring a2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    const int wlen = ::MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (wlen <= 0) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    ::MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), wresult.data(), static_cast<int>(wresult.size()));
    return wresult;
}

std::string w2a(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, 0, 0);
    if (len <= 0) {
        return "";
    }
    std::string result(len, '\0');
    ::WideCharToMultiByte(CP_ACP, 0, wstr.data(), static_cast<int>(wstr.size()), result.data(), static_cast<int>(result.size()), 0, 0);
    return result;
}

std::string a2u(std::string_view str) noexcept { return w2u(a2w(str)); }

std::string u2a(std::string_view str) noexcept { return w2a(u2w(str)); }
}  // namespace neko::win

#endif

namespace neko {

static char s_empty[1] = {0};

string_builder::string_builder() {
    data = s_empty;
    len = 0;
    capacity = 0;
}

void string_builder::trash() {
    if (data != s_empty) {
        neko_safe_free(data);
    }
}

void string_builder::reserve(u64 cap) {
    if (cap > capacity) {
        char *buf = (char *)neko_safe_malloc(cap);
        memset(buf, 0, cap);
        memcpy(buf, data, len);

        if (data != s_empty) {
            neko_safe_free(data);
        }

        data = buf;
        capacity = cap;
    }
}

void string_builder::clear() {
    len = 0;
    if (data != s_empty) {
        data[0] = 0;
    }
}

void string_builder::swap_filename(string filepath, string file) {
    clear();

    u64 slash = filepath.last_of('/');
    if (slash != (u64)-1) {
        string path = filepath.substr(0, slash + 1);
        *this << path;
    }

    *this << file;
}

void string_builder::concat(string str, s32 times) {
    for (s32 i = 0; i < times; i++) {
        *this << str;
    }
}

string_builder &string_builder::operator<<(string str) {
    u64 desired = len + str.len + 1;
    u64 cap = capacity;

    if (desired >= cap) {
        u64 growth = cap > 0 ? cap * 2 : 8;
        if (growth <= desired) {
            growth = desired;
        }

        reserve(growth);
    }

    memcpy(&data[len], str.data, str.len);
    len += str.len;
    data[len] = 0;
    return *this;
}

string_builder::operator string() { return {data, len}; }

struct arena_node {
    arena_node *next;
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

static arena_node *arena_block_make(u64 capacity) {
    u64 page = 4096 - offsetof(arena_node, buf);
    if (capacity < page) {
        capacity = page;
    }

    arena_node *a = (arena_node *)neko_safe_malloc(NEKO_OFFSET(arena_node, buf[capacity]));
    a->next = nullptr;
    a->allocd = 0;
    a->capacity = capacity;
    return a;
}

void arena::trash() {
    arena_node *a = head;
    while (a != nullptr) {
        arena_node *rm = a;
        a = a->next;
        neko_safe_free(rm);
    }
}

void *arena::bump(u64 size) {
    if (head == nullptr) {
        head = arena_block_make(size);
    }

    u64 next = 0;
    do {
        next = align_forward(head->allocd, 16);
        if (next + size <= head->capacity) {
            break;
        }

        arena_node *block = arena_block_make(size);
        block->next = head;

        head = block;
    } while (true);

    void *ptr = &head->buf[next];
    head->allocd = next + size;
    head->prev = next;
    return ptr;
}

void *arena::rebump(void *ptr, u64 old, u64 size) {
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

string arena::bump_string(string s) {
    if (s.len > 0) {
        char *cstr = (char *)bump(s.len + 1);
        memcpy(cstr, s.data, s.len);
        cstr[s.len] = '\0';
        return {cstr, s.len};
    } else {
        return {};
    }
}

static u32 read4(char *bytes) {
    u32 n;
    memcpy(&n, bytes, 4);
    return n;
}

static bool read_entire_file_raw(string *out, string filepath) {

    string path = to_cstr(filepath);
    neko_defer(neko_safe_free(path.data));

    FILE *file = fopen(path.data, "rb");
    if (file == nullptr) {
        return false;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = (char *)neko_safe_malloc(size + 1);
    size_t read = fread(buf, sizeof(char), size, file);
    fclose(file);

    if (read != size) {
        neko_safe_free(buf);
        return false;
    }

    buf[size] = 0;
    *out = {buf, size};
    return true;
}

struct IFileSystem {
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual bool mount(string filepath) = 0;
    virtual bool file_exists(string filepath) = 0;
    virtual bool read_entire_file(string *out, string filepath) = 0;
};

// TODO 统一管理全局变量
static std::unordered_map<std::string, IFileSystem *> g_filesystem_list;

struct DirectoryFileSystem : IFileSystem {
    void make() {}
    void trash() {}

    bool mount(string filepath) {
        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        s32 res = neko_pf_chdir(path.data);
        return res == 0;
    }

    bool file_exists(string filepath) {
        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        FILE *fp = fopen(path.data, "r");
        if (fp != nullptr) {
            fclose(fp);
            return true;
        }

        return false;
    }

    bool read_entire_file(string *out, string filepath) { return read_entire_file_raw(out, filepath); }

    // bool list_all_files(array<string> *files) { return list_all_files_help(files, ""); }
};

struct ZipFileSystem : IFileSystem {
    std::mutex mtx;
    mz_zip_archive zip = {};
    string zip_contents = {};

    void make() {}

    void trash() {
        if (zip_contents.data != nullptr) {
            mz_zip_reader_end(&zip);
            neko_safe_free(zip_contents.data);
        }
    }

    bool mount(string filepath) {

        string contents = {};
        bool contents_ok = read_entire_file_raw(&contents, filepath);
        if (!contents_ok) {
            return false;
        }

        bool success = false;
        neko_defer({
            if (!success) {
                neko_safe_free(contents.data);
            }
        });

        char *data = contents.data;
        char *end = &data[contents.len];

        constexpr s32 eocd_size = 22;
        char *eocd = end - eocd_size;
        if (read4(eocd) != 0x06054b50) {
            fprintf(stderr, "can't find EOCD record\n");
            return false;
        }

        u32 central_size = read4(&eocd[12]);
        if (read4(eocd - central_size) != 0x02014b50) {
            fprintf(stderr, "can't find central directory\n");
            return false;
        }

        u32 central_offset = read4(&eocd[16]);
        char *begin = eocd - central_size - central_offset;
        u64 zip_len = end - begin;
        if (read4(begin) != 0x04034b50) {
            fprintf(stderr, "can't read local file header\n");
            return false;
        }

        mz_bool zip_ok = mz_zip_reader_init_mem(&zip, begin, zip_len, 0);
        if (!zip_ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read zip: %s\n", mz_zip_get_error_string(err));
            return false;
        }

        zip_contents = contents;

        success = true;
        return true;
    }

    bool file_exists(string filepath) {

        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        std::unique_lock<std::mutex> lock(mtx);

        s32 i = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (i == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, i, &stat);
        if (!ok) {
            return false;
        }

        return true;
    }

    bool read_entire_file(string *out, string filepath) {

        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        std::unique_lock<std::mutex> lock(mtx);

        s32 file_index = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (file_index == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, file_index, &stat);
        if (!ok) {
            return false;
        }

        size_t size = stat.m_uncomp_size;
        char *buf = (char *)neko_safe_malloc(size + 1);

        ok = mz_zip_reader_extract_to_mem(&zip, file_index, buf, size, 0);
        if (!ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read file '%s': %s\n", path.data, mz_zip_get_error_string(err));
            neko_safe_free(buf);
            return false;
        }

        buf[size] = 0;
        *out = {buf, size};
        return true;
    }

    bool list_all_files(array<string> *files) {

        std::unique_lock<std::mutex> lock(mtx);

        for (u32 i = 0; i < mz_zip_reader_get_num_files(&zip); i++) {
            mz_zip_archive_file_stat file_stat;
            mz_bool ok = mz_zip_reader_file_stat(&zip, i, &file_stat);
            if (!ok) {
                return false;
            }

            string name = {file_stat.m_filename, strlen(file_stat.m_filename)};
            files->push(to_cstr(name));
        }

        return true;
    }
};

#ifdef __EMSCRIPTEN__
EM_JS(char *, web_mount_dir, (), { return stringToNewUTF8(spryMount); });

EM_ASYNC_JS(void, web_load_zip, (), {
    var dirs = spryMount.split("/");
    dirs.pop();

    var path = [];
    for (var dir of dirs) {
        path.push(dir);
        FS.mkdir(path.join("/"));
    }

    await fetch(spryMount).then(async function(res) {
        if (!res.ok) {
            throw new Error("failed to fetch " + spryMount);
        }

        var data = await res.arrayBuffer();
        FS.writeFile(spryMount, new Uint8Array(data));
    });
});

EM_ASYNC_JS(void, web_load_files, (), {
    var jobs = [];

    function spryWalkFiles(files, leading) {
        var path = leading.join("/");
        if (path != "") {
            FS.mkdir(path);
        }

        for (var entry of Object.entries(files)) {
            var key = entry[0];
            var value = entry[1];
            var filepath = [... leading, key ];
            if (typeof value == "object") {
                spryWalkFiles(value, filepath);
            } else if (value == 1) {
                var file = filepath.join("/");

                var job = fetch(file).then(async function(res) {
                    if (!res.ok) {
                        throw new Error("failed to fetch " + file);
                    }
                    var data = await res.arrayBuffer();
                    FS.writeFile(file, new Uint8Array(data));
                });

                jobs.push(job);
            }
        }
    }
    spryWalkFiles(spryFiles, []);

    await Promise.all(jobs);
});
#endif

// auto os_program_path() { return std::filesystem::current_path().string(); }

template <typename T>
static bool vfs_mount_type(std::string fsname, string mount) {
    void *ptr = neko_safe_malloc(sizeof(T));
    T *vfs = new (ptr) T();

    vfs->make();
    bool ok = vfs->mount(mount);
    if (!ok) {
        vfs->trash();
        neko_safe_free(vfs);
        return false;
    }

    g_filesystem_list.insert(std::make_pair(fsname, vfs));
    return true;
}

mount_result vfs_mount(const_str fsname, const_str filepath) {

    mount_result res = {};

#ifdef __EMSCRIPTEN__
    string mount_dir = web_mount_dir();
    neko_defer(neko_safe_free(mount_dir.data));

    if (mount_dir.ends_with(".zip")) {
        web_load_zip();
        res.ok = vfs_mount_type<ZipFileSystem>(mount_dir);
    } else {
        web_load_files();
        res.ok = vfs_mount_type<DirectoryFileSystem>(mount_dir);
    }

#else
    if (filepath == nullptr) {
        string path = os_program_path();

#ifndef NDEBUG
        NEKO_DEBUG_LOG("program path: %s", path.data);
#endif

        res.ok = vfs_mount_type<DirectoryFileSystem>(fsname, path);
    } else {
        string mount_dir = filepath;

        if (mount_dir.ends_with(".zip")) {
            res.ok = vfs_mount_type<ZipFileSystem>(fsname, mount_dir);
            res.is_fused = true;
        } else {
            res.ok = vfs_mount_type<DirectoryFileSystem>(fsname, mount_dir);
            res.can_hot_reload = res.ok;
        }
    }
#endif

    if (filepath != nullptr && !res.ok) {
        NEKO_ERROR("%s", tmp_fmt("failed to load: %s", filepath).data);
    }

    return res;
}

void vfs_fini(std::optional<std::string> name) {
    auto fini_fs = []<typename T>(T fs) {
        if constexpr (!is_pair<T>::value) {
            fs->trash();
            neko_safe_free(fs);
            NEKO_DEBUG_LOG("vfs_fini(%p)", fs);
        } else {
            fs.second->trash();
            neko_safe_free(fs.second);
            NEKO_DEBUG_LOG("vfs_fini(%s)", fs.first.c_str());
        }
    };
    if (!name.has_value()) {
        for (auto vfs : g_filesystem_list) fini_fs(vfs);
    } else {
        auto vfs = g_filesystem_list[name.value()];
        fini_fs(vfs);
    }
}

bool vfs_file_exists(std::string fsname, string filepath) { return g_filesystem_list[fsname]->file_exists(filepath); }

bool vfs_read_entire_file(std::string fsname, string *out, string filepath) { return g_filesystem_list[fsname]->read_entire_file(out, filepath); }

NEKO_API_DECL size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf) {
    size_t bytes_to_read = size * count;
    std::memcpy(dest, static_cast<const char *>(vf->data) + vf->offset, bytes_to_read);
    vf->offset += bytes_to_read;
    return count;
}

// #define SEEK_SET 0
// #define SEEK_CUR 1
// #define SEEK_END 2

NEKO_API_DECL int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence) {
    u64 new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = of;
            break;
        case SEEK_CUR:
            new_offset = vf->offset + of;
            break;
        case SEEK_END:
            new_offset = vf->len + of;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    if (new_offset < 0 || new_offset > vf->len) {
        errno = EINVAL;
        return -1;
    }
    vf->offset = new_offset;
    return 0;
}

NEKO_API_DECL u64 neko_capi_vfs_ftell(vfs_file *vf) { return vf->offset; }

NEKO_API_DECL vfs_file neko_capi_vfs_fopen(const_str path) {
    vfs_file vf{};
    vf.data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, path, &vf.len);
    return vf;
}

NEKO_API_DECL int neko_capi_vfs_fclose(vfs_file *vf) {
    NEKO_ASSERT(vf);
    neko_safe_free(vf->data);
    return 0;
}

NEKO_API_DECL bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath) { return vfs_file_exists(fsname, filepath); }

NEKO_API_DECL const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size) {
    string out;
    bool ok = vfs_read_entire_file(fsname, &out, filepath);
    if (!ok) return NULL;
    *size = out.len;
    return out.data;
}

s64 luax_len(lua_State *L, s32 arg) {
    lua_len(L, arg);
    lua_Integer len = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return len;
}

string luax_check_string(lua_State *L, s32 arg) {
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, arg, &len);
    return {str, len};
}

inline bool is_whitespace(char c) {
    switch (c) {
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

double string_to_double(string str) {
    double n = 0;
    double sign = 1;

    if (str.len == 0) {
        return n;
    }

    u64 i = 0;
    if (str.data[0] == '-' && str.len > 1 && is_digit(str.data[1])) {
        i++;
        sign = -1;
    }

    while (i < str.len) {
        if (!is_digit(str.data[i])) {
            break;
        }

        n = n * 10 + (str.data[i] - '0');
        i++;
    }

    if (i < str.len && str.data[i] == '.') {
        i++;
        double place = 10;
        while (i < str.len) {
            if (!is_digit(str.data[i])) {
                break;
            }

            n += (str.data[i] - '0') / place;
            place *= 10;
            i++;
        }
    }

    return n * sign;
}

enum JSONTok : s32 {
    JSONTok_Invalid,
    JSONTok_LBrace,    // {
    JSONTok_RBrace,    // }
    JSONTok_LBracket,  // [
    JSONTok_RBracket,  // ]
    JSONTok_Colon,     // :
    JSONTok_Comma,     // ,
    JSONTok_True,      // true
    JSONTok_False,     // false
    JSONTok_Null,      // null
    JSONTok_String,    // "[^"]*"
    JSONTok_Number,    // [0-9]+\.?[0-9]*
    JSONTok_Error,
    JSONTok_EOF,
};

const char *json_tok_string(JSONTok tok) {
    switch (tok) {
        case JSONTok_Invalid:
            return "Invalid";
        case JSONTok_LBrace:
            return "LBrace";
        case JSONTok_RBrace:
            return "RBrace";
        case JSONTok_LBracket:
            return "LBracket";
        case JSONTok_RBracket:
            return "RBracket";
        case JSONTok_Colon:
            return "Colon";
        case JSONTok_Comma:
            return "Comma";
        case JSONTok_True:
            return "True";
        case JSONTok_False:
            return "False";
        case JSONTok_Null:
            return "Null";
        case JSONTok_String:
            return "string";
        case JSONTok_Number:
            return "Number";
        case JSONTok_Error:
            return "Error";
        case JSONTok_EOF:
            return "EOF";
        default:
            return "?";
    }
}

const char *json_kind_string(JSONKind kind) {
    switch (kind) {
        case JSONKind_Null:
            return "Null";
        case JSONKind_Object:
            return "Object";
        case JSONKind_Array:
            return "Array";
        case JSONKind_String:
            return "string";
        case JSONKind_Number:
            return "Number";
        case JSONKind_Boolean:
            return "Boolean";
        default:
            return "?";
    }
};

struct JSONToken {
    JSONTok kind;
    string str;
    u32 line;
    u32 column;
};

struct JSONScanner {
    string contents;
    JSONToken token;
    u64 begin;
    u64 end;
    u32 line;
    u32 column;
};

static char json_peek(JSONScanner *scan, u64 offset) { return scan->contents.data[scan->end + offset]; }

static bool json_at_end(JSONScanner *scan) { return scan->end == scan->contents.len; }

static void json_next_char(JSONScanner *scan) {
    if (!json_at_end(scan)) {
        scan->end++;
        scan->column++;
    }
}

static void json_skip_whitespace(JSONScanner *scan) {
    while (true) {
        switch (json_peek(scan, 0)) {
            case '\n':
                scan->column = 0;
                scan->line++;
            case ' ':
            case '\t':
            case '\r':
                json_next_char(scan);
                break;
            default:
                return;
        }
    }
}

static string json_lexeme(JSONScanner *scan) { return scan->contents.substr(scan->begin, scan->end); }

static JSONToken json_make_tok(JSONScanner *scan, JSONTok kind) {
    JSONToken t = {};
    t.kind = kind;
    t.str = json_lexeme(scan);
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_err_tok(JSONScanner *scan, string msg) {
    JSONToken t = {};
    t.kind = JSONTok_Error;
    t.str = msg;
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_scan_ident(arena *a, JSONScanner *scan) {
    while (is_alpha(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    JSONToken t = {};
    t.str = json_lexeme(scan);

    if (t.str == "true") {
        t.kind = JSONTok_True;
    } else if (t.str == "false") {
        t.kind = JSONTok_False;
    } else if (t.str == "null") {
        t.kind = JSONTok_Null;
    } else {
        string_builder sb = {};
        neko_defer(sb.trash());

        string s = string(sb << "unknown identifier: '" << t.str << "'");
        return json_err_tok(scan, a->bump_string(s));
    }

    scan->token = t;
    return t;
}

static JSONToken json_scan_number(JSONScanner *scan) {
    if (json_peek(scan, 0) == '-' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '-'
    }

    while (is_digit(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    if (json_peek(scan, 0) == '.' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '.'

        while (is_digit(json_peek(scan, 0))) {
            json_next_char(scan);
        }
    }

    return json_make_tok(scan, JSONTok_Number);
}

static JSONToken json_scan_string(JSONScanner *scan) {
    while (json_peek(scan, 0) != '"' && !json_at_end(scan)) {
        json_next_char(scan);
    }

    if (json_at_end(scan)) {
        return json_err_tok(scan, "unterminated string");
    }

    json_next_char(scan);
    return json_make_tok(scan, JSONTok_String);
}

static JSONToken json_scan_next(arena *a, JSONScanner *scan) {
    json_skip_whitespace(scan);

    scan->begin = scan->end;

    if (json_at_end(scan)) {
        return json_make_tok(scan, JSONTok_EOF);
    }

    char c = json_peek(scan, 0);
    json_next_char(scan);

    if (is_alpha(c)) {
        return json_scan_ident(a, scan);
    }

    if (is_digit(c) || (c == '-' && is_digit(json_peek(scan, 0)))) {
        return json_scan_number(scan);
    }

    if (c == '"') {
        return json_scan_string(scan);
    }

    switch (c) {
        case '{':
            return json_make_tok(scan, JSONTok_LBrace);
        case '}':
            return json_make_tok(scan, JSONTok_RBrace);
        case '[':
            return json_make_tok(scan, JSONTok_LBracket);
        case ']':
            return json_make_tok(scan, JSONTok_RBracket);
        case ':':
            return json_make_tok(scan, JSONTok_Colon);
        case ',':
            return json_make_tok(scan, JSONTok_Comma);
    }

    string msg = tmp_fmt("unexpected character: '%c' (%d)", c, (int)c);
    string s = a->bump_string(msg);
    return json_err_tok(scan, s);
}

static string json_parse_next(arena *a, JSONScanner *scan, JSON *out);

static string json_parse_object(arena *a, JSONScanner *scan, JSONObject **out) {

    JSONObject *obj = nullptr;

    json_scan_next(a, scan);  // eat brace

    while (true) {
        if (scan->token.kind == JSONTok_RBrace) {
            *out = obj;
            json_scan_next(a, scan);
            return {};
        }

        string err = {};

        JSON key = {};
        err = json_parse_next(a, scan, &key);
        if (err.data != nullptr) {
            return err;
        }

        if (key.kind != JSONKind_String) {
            string msg = tmp_fmt("expected string as object key on line: %d. got: %s", (s32)scan->token.line, json_kind_string(key.kind));
            return a->bump_string(msg);
        }

        if (scan->token.kind != JSONTok_Colon) {
            string msg = tmp_fmt("expected colon on line: %d. got %s", (s32)scan->token.line, json_tok_string(scan->token.kind));
            return a->bump_string(msg);
        }

        json_scan_next(a, scan);

        JSON value = {};
        err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONObject *entry = (JSONObject *)a->bump(sizeof(JSONObject));
        entry->next = obj;
        entry->hash = fnv1a(key.str);
        entry->key = key.str;
        entry->value = value;

        obj = entry;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static string json_parse_array(arena *a, JSONScanner *scan, JSONArray **out) {

    JSONArray *arr = nullptr;

    json_scan_next(a, scan);  // eat bracket

    while (true) {
        if (scan->token.kind == JSONTok_RBracket) {
            *out = arr;
            json_scan_next(a, scan);
            return {};
        }

        JSON value = {};
        string err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONArray *el = (JSONArray *)a->bump(sizeof(JSONArray));
        el->next = arr;
        el->value = value;
        el->index = 0;

        if (arr != nullptr) {
            el->index = arr->index + 1;
        }

        arr = el;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static string json_parse_next(arena *a, JSONScanner *scan, JSON *out) {
    switch (scan->token.kind) {
        case JSONTok_LBrace: {
            out->kind = JSONKind_Object;
            return json_parse_object(a, scan, &out->object);
        }
        case JSONTok_LBracket: {
            out->kind = JSONKind_Array;
            return json_parse_array(a, scan, &out->array);
        }
        case JSONTok_String: {
            out->kind = JSONKind_String;
            out->str = scan->token.str.substr(1, scan->token.str.len - 1);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Number: {
            out->kind = JSONKind_Number;
            out->number = string_to_double(scan->token.str);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_True: {
            out->kind = JSONKind_Boolean;
            out->boolean = true;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_False: {
            out->kind = JSONKind_Boolean;
            out->boolean = false;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Null: {
            out->kind = JSONKind_Null;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Error: {
            string_builder sb = {};
            neko_defer(sb.trash());

            sb << scan->token.str << tmp_fmt(" on line %d:%d", (s32)scan->token.line, (s32)scan->token.column);

            return a->bump_string(string(sb));
        }
        default: {
            string msg = tmp_fmt("unknown json token: %s on line %d:%d", json_tok_string(scan->token.kind), (s32)scan->token.line, (s32)scan->token.column);
            return a->bump_string(msg);
        }
    }
}

void JSONDocument::parse(string contents) {

    arena = {};

    JSONScanner scan = {};
    scan.contents = contents;
    scan.line = 1;

    json_scan_next(&arena, &scan);

    string err = json_parse_next(&arena, &scan, &root);
    if (err.data != nullptr) {
        error = err;
        return;
    }

    if (scan.token.kind != JSONTok_EOF) {
        error = "expected EOF";
        return;
    }
}

void JSONDocument::trash() { arena.trash(); }

JSON JSON::lookup(string key, bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        for (JSONObject *o = object; o != nullptr; o = o->next) {
            if (o->hash == fnv1a(key)) {
                return o->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSON JSON::index(s32 i, bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        for (JSONArray *a = array; a != nullptr; a = a->next) {
            if (a->index == i) {
                return a->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSONObject *JSON::as_object(bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        return object;
    }

    *ok = false;
    return {};
}

JSONArray *JSON::as_array(bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        return array;
    }

    *ok = false;
    return {};
}

string JSON::as_string(bool *ok) {
    if (*ok && kind == JSONKind_String) {
        return str;
    }

    *ok = false;
    return {};
}

double JSON::as_number(bool *ok) {
    if (*ok && kind == JSONKind_Number) {
        return number;
    }

    *ok = false;
    return {};
}

JSONObject *JSON::lookup_object(string key, bool *ok) { return lookup(key, ok).as_object(ok); }

JSONArray *JSON::lookup_array(string key, bool *ok) { return lookup(key, ok).as_array(ok); }

string JSON::lookup_string(string key, bool *ok) { return lookup(key, ok).as_string(ok); }

double JSON::lookup_number(string key, bool *ok) { return lookup(key, ok).as_number(ok); }

double JSON::index_number(s32 i, bool *ok) { return index(i, ok).as_number(ok); }

static void json_write_string(string_builder &sb, JSON *json, s32 level) {
    switch (json->kind) {
        case JSONKind_Object: {
            sb << "{\n";
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                sb.concat("  ", level);
                sb << o->key;
                json_write_string(sb, &o->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "}";
            break;
        }
        case JSONKind_Array: {
            sb << "[\n";
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                sb.concat("  ", level);
                json_write_string(sb, &a->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "]";
            break;
        }
        case JSONKind_String:
            sb << "\"" << json->str << "\"";
            break;
        case JSONKind_Number:
            sb << tmp_fmt("%g", json->number);
            break;
        case JSONKind_Boolean:
            sb << (json->boolean ? "true" : "false");
            break;
        case JSONKind_Null:
            sb << "null";
            break;
        default:
            break;
    }
}

void json_write_string(string_builder *sb, JSON *json) { json_write_string(*sb, json, 1); }

void json_print(JSON *json) {
    string_builder sb = {};
    neko_defer(sb.trash());
    json_write_string(&sb, json);
    printf("%s\n", sb.data);
}

void json_to_lua(lua_State *L, JSON *json) {
    switch (json->kind) {
        case JSONKind_Object: {
            lua_newtable(L);
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                lua_pushlstring(L, o->key.data, o->key.len);
                json_to_lua(L, &o->value);
                lua_rawset(L, -3);
            }
            break;
        }
        case JSONKind_Array: {
            lua_newtable(L);
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                json_to_lua(L, &a->value);
                lua_rawseti(L, -2, a->index + 1);
            }
            break;
        }
        case JSONKind_String: {
            lua_pushlstring(L, json->str.data, json->str.len);
            break;
        }
        case JSONKind_Number: {
            lua_pushnumber(L, json->number);
            break;
        }
        case JSONKind_Boolean: {
            lua_pushboolean(L, json->boolean);
            break;
        }
        case JSONKind_Null: {
            lua_pushnil(L);
            break;
        }
        default:
            break;
    }
}

static void lua_to_json_string(string_builder &sb, lua_State *L, hashmap<bool> *visited, string *err, s32 width, s32 level) {
    auto indent = [&](s32 offset) {
        if (width > 0) {
            sb << "\n";
            sb.concat(" ", width * (level + offset));
        }
    };

    if (err->len != 0) {
        return;
    }

    s32 top = lua_gettop(L);
    switch (lua_type(L, top)) {
        case LUA_TTABLE: {
            uintptr_t ptr = (uintptr_t)lua_topointer(L, top);

            bool *visit = nullptr;
            bool exist = visited->find_or_insert(ptr, &visit);
            if (exist && *visit) {
                *err = "table has cycles";
                return;
            }

            *visit = true;

            lua_pushnil(L);
            if (lua_next(L, -2) == 0) {
                sb << "[]";
                return;
            }

            s32 key_type = lua_type(L, -2);

            if (key_type == LUA_TNUMBER) {
                sb << "[";

                indent(0);
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                s32 len = luax_len(L, top);
                assert(len > 0);
                s32 i = 1;
                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TNUMBER) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be numbers";
                        return;
                    }

                    sb << ",";
                    indent(0);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    i++;
                }
                indent(-1);
                sb << "]";

                if (i != len) {
                    *err = "array is not continuous";
                    return;
                }
            } else if (key_type == LUA_TSTRING) {
                sb << "{";
                indent(0);

                lua_pushvalue(L, -2);
                lua_to_json_string(sb, L, visited, err, width, level + 1);
                lua_pop(L, 1);
                sb << ":";
                if (width > 0) {
                    sb << " ";
                }
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TSTRING) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be strings";
                        return;
                    }

                    sb << ",";
                    indent(0);

                    lua_pushvalue(L, -2);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    lua_pop(L, 1);
                    sb << ":";
                    if (width > 0) {
                        sb << " ";
                    }
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                }
                indent(-1);
                sb << "}";
            } else {
                lua_pop(L, 2);  // key, value
                *err = "expected table keys to be strings or numbers";
                return;
            }

            visited->unset(ptr);
            break;
        }
        case LUA_TNIL:
            sb << "null";
            break;
        case LUA_TNUMBER:
            sb << tmp_fmt("%g", lua_tonumber(L, top));
            break;
        case LUA_TSTRING:
            sb << "\"" << luax_check_string(L, top) << "\"";
            break;
        case LUA_TBOOLEAN:
            sb << (lua_toboolean(L, top) ? "true" : "false");
            break;
        default:
            *err = "type is not serializable";
    }
}

string lua_to_json_string(lua_State *L, s32 arg, string *contents, s32 width) {
    string_builder sb = {};

    hashmap<bool> visited = {};
    neko_defer(visited.trash());

    string err = {};
    lua_pushvalue(L, arg);
    lua_to_json_string(sb, L, &visited, &err, width, 1);
    lua_pop(L, 1);

    if (err.len != 0) {
        sb.trash();
    }

    *contents = string(sb);
    return err;
}

#ifndef NEKO_PF_WIN
#include <errno.h>
#endif

#ifdef NEKO_PF_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef NEKO_PF_WIN

void mutex::make() { srwlock = {}; }
void mutex::trash() {}
void mutex::lock() { AcquireSRWLockExclusive(&srwlock); }
void mutex::unlock() { ReleaseSRWLockExclusive(&srwlock); }

bool mutex::try_lock() {
    BOOLEAN ok = TryAcquireSRWLockExclusive(&srwlock);
    return ok != 0;
}

void cond::make() { InitializeConditionVariable(&cv); }
void cond::trash() {}
void cond::signal() { WakeConditionVariable(&cv); }
void cond::broadcast() { WakeAllConditionVariable(&cv); }

void cond::wait(mutex *mtx) { SleepConditionVariableSRW(&cv, &mtx->srwlock, INFINITE, 0); }

bool cond::timed_wait(mutex *mtx, uint32_t ms) { return SleepConditionVariableSRW(&cv, &mtx->srwlock, ms, 0); }

void rwlock::make() { srwlock = {}; }
void rwlock::trash() {}
void rwlock::shared_lock() { AcquireSRWLockShared(&srwlock); }
void rwlock::shared_unlock() { ReleaseSRWLockShared(&srwlock); }
void rwlock::unique_lock() { AcquireSRWLockExclusive(&srwlock); }
void rwlock::unique_unlock() { ReleaseSRWLockExclusive(&srwlock); }

void sema::make(int n) { handle = CreateSemaphoreA(nullptr, n, LONG_MAX, nullptr); }
void sema::trash() { CloseHandle(handle); }
void sema::post(int n) { ReleaseSemaphore(handle, n, nullptr); }
void sema::wait() { WaitForSingleObjectEx(handle, INFINITE, false); }

void thread::make(thread_proc fn, void *udata) {
    DWORD id = 0;
    HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, &id);
    ptr = (void *)handle;
}

void thread::join() {
    WaitForSingleObject((HANDLE)ptr, INFINITE);
    CloseHandle((HANDLE)ptr);
}

uint64_t this_thread_id() { return GetCurrentThreadId(); }

#else

static struct timespec ms_from_now(u32 ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    unsigned long long tally = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
    tally += ms;

    ts.tv_sec = tally / 1000LL;
    ts.tv_nsec = (tally % 1000LL) * 1000000LL;

    return ts;
}

void mutex::make() { pthread_mutex_init(&pt, nullptr); }
void mutex::trash() { pthread_mutex_destroy(&pt); }
void mutex::lock() { pthread_mutex_lock(&pt); }
void mutex::unlock() { pthread_mutex_unlock(&pt); }

bool mutex::try_lock() {
    int res = pthread_mutex_trylock(&pt);
    return res == 0;
}

void cond::make() { pthread_cond_init(&pt, nullptr); }
void cond::trash() { pthread_cond_destroy(&pt); }
void cond::signal() { pthread_cond_signal(&pt); }
void cond::broadcast() { pthread_cond_broadcast(&pt); }
void cond::wait(mutex *mtx) { pthread_cond_wait(&pt, &mtx->pt); }

bool cond::timed_wait(mutex *mtx, uint32_t ms) {
    struct timespec ts = ms_from_now(ms);
    int res = pthread_cond_timedwait(&pt, &mtx->pt, &ts);
    return res == 0;
}

void rwlock::make() { pthread_rwlock_init(&pt, nullptr); }
void rwlock::trash() { pthread_rwlock_destroy(&pt); }
void rwlock::shared_lock() { pthread_rwlock_rdlock(&pt); }
void rwlock::shared_unlock() { pthread_rwlock_unlock(&pt); }
void rwlock::unique_lock() { pthread_rwlock_wrlock(&pt); }
void rwlock::unique_unlock() { pthread_rwlock_unlock(&pt); }

void sema::make(int n) {
    sem = (sem_t *)neko_safe_malloc(sizeof(sem_t));
    sem_init(sem, 0, n);
}

void sema::trash() {
    sem_destroy(sem);
    neko_safe_free(sem);
}

void sema::post(int n) {
    for (int i = 0; i < n; i++) {
        sem_post(sem);
    }
}

void sema::wait() { sem_wait(sem); }

void thread::make(thread_proc fn, void *udata) {
    pthread_t pt = {};
    pthread_create(&pt, nullptr, (void *(*)(void *))fn, udata);
    ptr = (void *)pt;
}

void thread::join() { pthread_join((pthread_t)ptr, nullptr); }

#endif

#ifdef NEKO_PF_LINUX

uint64_t this_thread_id() {
    thread_local uint64_t s_tid = syscall(SYS_gettid);
    return s_tid;
}

#endif  // NEKO_PF_LINUX

#ifdef NEKO_PF_WEB

uint64_t this_thread_id() { return 0; }

#endif  // NEKO_PF_WEB

static void lua_thread_proc(void *udata) {
    PROFILE_FUNC();

    LuaThread *lt = (LuaThread *)udata;

    // LuaAlloc *LA = luaalloc_create(nullptr, nullptr);
    // defer(luaalloc_delete(LA));

    // lua_State *L = luaL_newstate();
    // neko_defer(lua_close(L));

    lua_State *L = neko_lua_bootstrap(0, NULL);
    neko_defer(neko_lua_fini(L));

    {
        // PROFILE_BLOCK("open libs");
        // luaL_openlibs(L);
    }

    {
        // PROFILE_BLOCK("open api");
        // open_spry_api(L);
    }

    {
        // PROFILE_BLOCK("open luasocket");
        // open_luasocket(L);
    }

    {
        // PROFILE_BLOCK("run bootstrap");
        // luax_run_bootstrap(L);
    }

    string contents = lt->contents;

    {
        // PROFILE_BLOCK("load chunk");
        if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) != LUA_OK) {
            string err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);

            neko_safe_free(contents.data);
            neko_safe_free(lt->name.data);
            return;
        }
    }

    neko_safe_free(contents.data);
    neko_safe_free(lt->name.data);

    {
        // PROFILE_BLOCK("run chunk");
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            string err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);
        }
    }
}

void LuaThread::make(string code, string thread_name) {
    mtx.make();
    contents = to_cstr(code);
    name = to_cstr(thread_name);

    lock_guard lock{&mtx};
    thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
    if (lock_guard lock{&mtx}) {
        thread.join();
    }

    mtx.trash();
}

//

void lua_variant_wrap::make(lua_State *L, s32 arg) {
    data.type = lua_type(L, arg);

    switch (data.type) {
        case LUA_TBOOLEAN:
            data.boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            data.number = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            neko::string s = luax_check_string(L, arg);
            data.string = to_cstr(s);
            break;
        }
        case LUA_TTABLE: {
            array<lua_table_entry> entries = {};
            entries.resize(luax_len(L, arg));

            lua_pushvalue(L, arg);
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                lua_variant_wrap key = {};
                key.make(L, -2);

                lua_variant_wrap value = {};
                value.make(L, -1);

                entries.push({key, value});
            }
            lua_pop(L, 1);

            data.table = slice(entries);
            break;
        }
        case LUA_TUSERDATA: {
            s32 kind = lua_getiuservalue(L, arg, LUAX_UD_TNAME);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TSTRING) {
                return;
            }

            kind = lua_getiuservalue(L, arg, LUAX_UD_PTR_SIZE);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TNUMBER) {
                return;
            }

            neko::string tname = luax_check_string(L, -2);
            u64 size = luaL_checkinteger(L, -1);

            if (size != sizeof(void *)) {
                return;
            }

            data.udata.ptr = *(void **)lua_touserdata(L, arg);
            data.udata.tname = to_cstr(tname);

            break;
        }
        default:
            break;
    }
}

void lua_variant_wrap::trash() {
    switch (data.type) {
        case LUA_TSTRING: {
            neko_safe_free(data.string.data);
            break;
        }
        case LUA_TTABLE: {
            for (lua_table_entry e : data.table) {
                e.key.trash();
                e.value.trash();
            }
            neko_safe_free(data.table.data);
        }
        case LUA_TUSERDATA: {
            neko_safe_free(data.udata.tname.data);
        }
        default:
            break;
    }
}

void lua_variant_wrap::push(lua_State *L) {
    switch (data.type) {
        case LUA_TBOOLEAN:
            lua_pushboolean(L, data.boolean);
            break;
        case LUA_TNUMBER:
            lua_pushnumber(L, data.number);
            break;
        case LUA_TSTRING:
            lua_pushlstring(L, data.string.data, data.string.len);
            break;
        case LUA_TTABLE: {
            lua_newtable(L);
            for (lua_table_entry e : data.table) {
                e.key.push(L);
                e.value.push(L);
                lua_rawset(L, -3);
            }
            break;
        }
        case LUA_TUSERDATA: {
            luax_ptr_userdata(L, data.udata.ptr, data.udata.tname.data);
            break;
        }
        default:
            break;
    }
}

//

struct LuaChannels {
    neko::mutex mtx;
    neko::cond select;
    neko::hashmap<lua_channel *> by_name;
};

static LuaChannels g_channels = {};

void lua_channel::make(string n, u64 buf) {
    mtx.make();
    sent.make();
    received.make();
    items.data = (lua_variant_wrap *)neko_safe_malloc(sizeof(lua_variant_wrap) * (buf + 1));
    items.len = (buf + 1);
    front = 0;
    back = 0;
    len = 0;

    name.store(to_cstr(n).data);
}

void lua_channel::trash() {
    for (s32 i = 0; i < len; i++) {
        items[front].trash();
        front = (front + 1) % items.len;
    }

    neko_safe_free(items.data);
    neko_safe_free(name.exchange(nullptr));
    mtx.trash();
    sent.trash();
    received.trash();
}

void lua_channel::send(lua_variant_wrap item) {
    lock_guard lock{&mtx};

    while (len == items.len) {
        received.wait(&mtx);
    }

    items[back] = item;
    back = (back + 1) % items.len;
    len++;

    g_channels.select.broadcast();
    sent.signal();
    sent_total++;

    while (sent_total >= received_total + items.len) {
        received.wait(&mtx);
    }
}

static lua_variant_wrap lua_channel_dequeue(lua_channel *ch) {
    lua_variant_wrap item = ch->items[ch->front];
    ch->front = (ch->front + 1) % ch->items.len;
    ch->len--;

    ch->received.broadcast();
    ch->received_total++;

    return item;
}

lua_variant_wrap lua_channel::recv() {
    lock_guard lock{&mtx};

    while (len == 0) {
        sent.wait(&mtx);
    }

    return lua_channel_dequeue(this);
}

bool lua_channel::try_recv(lua_variant_wrap *v) {
    lock_guard lock{&mtx};

    if (len == 0) {
        return false;
    }

    *v = lua_channel_dequeue(this);
    return true;
}

lua_channel *lua_channel_make(string name, u64 buf) {
    lua_channel *chan = (lua_channel *)neko_safe_malloc(sizeof(lua_channel));
    new (&chan->name) std::atomic<char *>();
    chan->make(name, buf);

    lock_guard lock{&g_channels.mtx};
    g_channels.by_name[fnv1a(name)] = chan;

    return chan;
}

lua_channel *lua_channel_get(string name) {
    lock_guard lock{&g_channels.mtx};

    lua_channel **chan = g_channels.by_name.get(fnv1a(name));
    if (chan == nullptr) {
        return nullptr;
    }

    return *chan;
}

lua_channel *lua_channels_select(lua_State *L, lua_variant_wrap *v) {
    s32 len = lua_gettop(L);
    if (len == 0) {
        return nullptr;
    }

    lua_channel *buf[16] = {};
    for (s32 i = 0; i < len; i++) {
        buf[i] = *(lua_channel **)luaL_checkudata(L, i + 1, "mt_channel");
    }

    mutex mtx = {};
    mtx.make();
    lock_guard lock{&mtx};

    while (true) {
        for (s32 i = 0; i < len; i++) {
            lock_guard lock{&buf[i]->mtx};
            if (buf[i]->len > 0) {
                *v = lua_channel_dequeue(buf[i]);
                return buf[i];
            }
        }

        g_channels.select.wait(&mtx);
    }
}

void lua_channels_setup() {
    g_channels.select.make();
    g_channels.mtx.make();
}

void lua_channels_shutdown() {
    for (auto [k, v] : g_channels.by_name) {
        lua_channel *chan = *v;
        chan->trash();
        neko_safe_free(chan);
    }
    g_channels.by_name.trash();
    g_channels.select.trash();
    g_channels.mtx.trash();
}

s32 os_change_dir(const char *path) { return chdir(path); }

string os_program_dir() {
    string str = os_program_path();
    char *buf = str.data;

    for (s32 i = (s32)str.len; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i + 1] = 0;
            return {str.data, (u64)i + 1};
        }
    }

    return str;
}

#ifdef NEKO_PF_WIN

string os_program_path() {
    static char s_buf[2048];

    DWORD len = GetModuleFileNameA(NULL, s_buf, NEKO_ARR_SIZE(s_buf));

    for (s32 i = 0; s_buf[i]; i++) {
        if (s_buf[i] == '\\') {
            s_buf[i] = '/';
        }
    }

    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
    HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    neko_defer(CloseHandle(handle));

    FILETIME create = {};
    FILETIME access = {};
    FILETIME write = {};
    bool ok = GetFileTime(handle, &create, &access, &write);
    if (!ok) {
        return 0;
    }

    ULARGE_INTEGER time = {};
    time.LowPart = write.dwLowDateTime;
    time.HighPart = write.dwHighDateTime;

    return time.QuadPart;
}

void os_high_timer_resolution() { timeBeginPeriod(8); }
void os_sleep(u32 ms) { Sleep(ms); }
void os_yield() { YieldProcessor(); }

#endif  // NEKO_PF_WIN

#ifdef NEKO_PF_LINUX

string os_program_path() {
    static char s_buf[2048];
    s32 len = (s32)readlink("/proc/self/exe", s_buf, NEKO_ARR_SIZE(s_buf));
    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
    struct stat attrib = {};
    s32 err = stat(filename, &attrib);
    if (err == 0) {
        return (u64)attrib.st_mtime;
    } else {
        return 0;
    }
}

void os_high_timer_resolution() {}

void os_sleep(u32 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, &ts);
}

void os_yield() { sched_yield(); }

#endif  // NEKO_PF_LINUX

#ifdef NEKO_PF_WEB

string os_program_path() { return {}; }
u64 os_file_modtime(const char *filename) { return 0; }
void os_high_timer_resolution() {}
void os_sleep(u32 ms) {}
void os_yield() {}

#endif  // NEKO_PF_WEB

//=============================
// dylib loader
//=============================

#if (defined(_WIN32) || defined(_WIN64))
#define NEKO_DLL_LOADER_WIN_MAC_OTHER(win_def, mac_def, other_def) win_def
#define NEKO_DLL_LOADER_WIN_OTHER(win_def, other_def) win_def
#elif defined(__APPLE__)
#define NEKO_DLL_LOADER_WIN_MAC_OTHER(win_def, mac_def, other_def) mac_def
#define NEKO_DLL_LOADER_WIN_OTHER(win_def, other_def) other_def
#else
#define NEKO_DLL_LOADER_WIN_MAC_OTHER(win_def, mac_def, other_def) other_def
#define NEKO_DLL_LOADER_WIN_OTHER(win_def, other_def) other_def
#endif

neko_dynlib neko_module_open(const_str name) {
    neko_dynlib module;
    char filename[64] = {};
    const_str prefix = NEKO_DLL_LOADER_WIN_OTHER("", "lib");
    const_str suffix = NEKO_DLL_LOADER_WIN_MAC_OTHER(".dll", ".dylib", ".so");
    neko_snprintf(filename, 64, "%s%s%s", prefix, name, suffix);
    module.hndl = (void *)neko_pf_library_load(filename);
    return module;
}

void neko_module_close(neko_dynlib lib) { neko_pf_library_unload(lib.hndl); }

void *neko_module_get_symbol(neko_dynlib lib, const_str symbol_name) {
    void *symbol = (void *)neko_pf_library_proc_address(lib.hndl, symbol_name);
    return symbol;
}

bool neko_module_has_symbol(neko_dynlib lib, const_str symbol_name) {
    if (!lib.hndl || !symbol_name) return false;
    return neko_pf_library_proc_address(lib.hndl, symbol_name) != NULL;
}

#if 0
static std::string get_error_description() noexcept {
#if (defined(_WIN32) || defined(_WIN64))
    constexpr const size_t BUF_SIZE = 512;
    const auto error_code = GetLastError();
    if (!error_code) return "No error reported by GetLastError";
    char description[BUF_SIZE];
    const auto lang = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    const DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, lang, description, BUF_SIZE, nullptr);
    return (length == 0) ? "Unknown error (FormatMessage failed)" : description;
#else
    const auto description = dlerror();
    return (description == nullptr) ? "No error reported by dlerror" : description;
#endif
}
#endif

struct Profile {
    queue<TraceEvent> events;
    thread recv_thread;
};

static Profile g_profile = {};

static void profile_recv_thread(void *) {
    string_builder sb = {};
    sb.swap_filename(os_program_path(), "profile.json");

    FILE *f = fopen(sb.data, "w");
    sb.trash();

    neko_defer(fclose(f));

    fputs("[", f);
    while (true) {
        TraceEvent e = g_profile.events.demand();
        if (e.name == nullptr) {
            return;
        }

        fprintf(f,
                R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%hu},)"
                "\n",
                e.name, e.cat, e.ph, e.ts / 1000.f, e.tid);
    }
}

void profile_setup() {
    g_profile.events.make();
    g_profile.events.reserve(256);
    g_profile.recv_thread.make(profile_recv_thread, nullptr);
}

void profile_fini() {
    g_profile.events.enqueue({});
    g_profile.recv_thread.join();
    g_profile.events.trash();
}

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
typedef struct {
    uint32_t initialized;
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
} neko_tm_state_t;
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
typedef struct {
    uint32_t initialized;
    mach_timebase_info_data_t timebase;
    uint64_t start;
} neko_tm_state_t;
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
typedef struct {
    uint32_t initialized;
    double start;
} neko_tm_state_t;
#else  // linux
#include <time.h>
typedef struct {
    uint32_t initialized;
    uint64_t start;
} neko_tm_state_t;
#endif
static neko_tm_state_t g_tm;

NEKO_API_DECL void neko_tm_init(void) {
    memset(&g_tm, 0, sizeof(g_tm));
    g_tm.initialized = 0xABCDEF01;
#if defined(_WIN32)
    QueryPerformanceFrequency(&g_tm.freq);
    QueryPerformanceCounter(&g_tm.start);
#elif defined(__APPLE__) && defined(__MACH__)
    mach_timebase_info(&g_tm.timebase);
    g_tm.start = mach_absolute_time();
#elif defined(__EMSCRIPTEN__)
    g_tm.start = emscripten_get_now();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    g_tm.start = (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
#endif
}

#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
int64_t __int64_muldiv(int64_t value, int64_t numer, int64_t denom) {
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}
#endif

NEKO_API_DECL uint64_t neko_tm_now(void) {
    NEKO_ASSERT(g_tm.initialized == 0xABCDEF01);
    uint64_t now;
#if defined(_WIN32)
    LARGE_INTEGER qpc_t;
    QueryPerformanceCounter(&qpc_t);
    now = (uint64_t)__int64_muldiv(qpc_t.QuadPart - g_tm.start.QuadPart, 1000000000, g_tm.freq.QuadPart);
#elif defined(__APPLE__) && defined(__MACH__)
    const uint64_t mach_now = mach_absolute_time() - g_tm.start;
    now = (uint64_t)_stm_int64_muldiv((int64_t)mach_now, (int64_t)g_tm.timebase.numer, (int64_t)g_tm.timebase.denom);
#elif defined(__EMSCRIPTEN__)
    double js_now = emscripten_get_now() - g_tm.start;
    now = (uint64_t)(js_now * 1000000.0);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    now = ((uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec) - g_tm.start;
#endif
    return now;
}

Instrument::Instrument(const char *cat, const char *name) : cat(cat), name(name), tid(this_thread_id()) {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'B';
    e.ts = neko_tm_now();
    e.tid = tid;

    g_profile.events.enqueue(e);
}

Instrument::~Instrument() {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'E';
    e.ts = neko_tm_now();
    e.tid = tid;

    g_profile.events.enqueue(e);
}

}  // namespace neko

NEKO_STRUCT(neko_client_cvar_t,                         //
            _Fs(show_editor, "Is show editor"),         //
            _Fs(show_demo_window, "Is show nui demo"),  //
            _Fs(show_pack_editor, "pack editor"),       //
            _Fs(show_profiler_window, "profiler"),      //
            _Fs(show_gui, "neko gui"),                  //
            _Fs(shader_inspect, "shaders"),             //
            _Fs(hello_ai_shit, "Test AI"),              //
            _Fs(bg, "bg color")                         //
);

template <typename T, typename Fields = std::tuple<>>
void __neko_cvar_gui_internal(T &&obj, int depth = 0, const char *fieldName = "", Fields &&fields = std::make_tuple()) {
    if constexpr (std::is_class_v<std::decay_t<T>>) {
        neko::reflection::struct_foreach(obj, [depth](auto &&fieldName, auto &&value, auto &&info) { __neko_cvar_gui_internal(value, depth + 1, fieldName, info); });
    } else {

        auto ff = [&]<typename S>(const char *name, auto &var, S &t) {
            if constexpr (std::is_same_v<std::decay_t<decltype(var)>, S>) {
                neko::imgui::Auto(var, name);
                ImGui::Text("    [%s]", std::get<0>(fields));
            }
        };

#define CVAR_TYPES() bool, s32, f32, f32 *
        std::apply([&](auto &&...args) { (ff(fieldName, obj, args), ...); }, std::tuple<CVAR_TYPES()>());
    }
}

void neko_cvar_gui(neko_client_cvar_t &cvar) {
    __neko_cvar_gui_internal(cvar);

    // for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
    //     {
    //         switch ((&neko_cv()->cvars[i])->type) {
    //             default:
    //             case __NEKO_CONFIG_TYPE_STRING:
    //                 neko::imgui::Auto((&neko_cv()->cvars[i])->value.s, (&neko_cv()->cvars[i])->name);
    //                 break;
    //             case __NEKO_CONFIG_TYPE_FLOAT:
    //                 neko::imgui::Auto((&neko_cv()->cvars[i])->value.f, (&neko_cv()->cvars[i])->name);
    //                 break;
    //             case __NEKO_CONFIG_TYPE_INT:
    //                 neko::imgui::Auto((&neko_cv()->cvars[i])->value.i, (&neko_cv()->cvars[i])->name);
    //                 break;
    //         };
    //     };
    // }
}
