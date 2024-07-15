

#ifndef NEKO_H
#define NEKO_H

#include "neko_prelude.h"

/*=============================
// Const
=============================*/

struct NEKO_PACKS {
    static constexpr const_str GAMEDATA = "default_pack";
    static constexpr const_str LUACODE = "luacode";
};

/*=============================
// Memory
=============================*/

NEKO_API_DECL
void* neko_malloc_init_impl(size_t sz);

// Default memory allocations
#define neko_malloc malloc
#define neko_free free
#define neko_realloc realloc
#define neko_calloc calloc

#define neko_malloc_init(__T) (__T*)neko_malloc_init_impl(sizeof(__T))

#ifndef neko_strdup
#define neko_strdup(__STR) strdup(__STR)
#endif

NEKO_API_DECL void* __neko_mem_safe_calloc(size_t count, size_t element_size, const char* file, int line);
NEKO_API_DECL void* __neko_mem_safe_realloc(void* ptr, size_t new_size, const char* file, int line);

// logging

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    u32 time;
    FILE* udata;
    int line;
    int level;
} neko_log_event;

typedef void (*neko_log_fn)(neko_log_event* ev);
typedef void (*neko_log_lock_fn)(bool lock, void* udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

NEKO_API_DECL const char* log_level_string(int level);
NEKO_API_DECL void log_set_lock(neko_log_lock_fn fn, void* udata);
NEKO_API_DECL void log_set_quiet(bool enable);

NEKO_API_DECL void neko_log(int level, const char* file, int line, const char* fmt, ...);

/*============================================================
// Result
============================================================*/

typedef enum neko_result { NEKO_RESULT_SUCCESS, NEKO_RESULT_IN_PROGRESS, NEKO_RESULT_INCOMPLETE, NEKO_RESULT_FAILURE } neko_result;

/*===================================
// Resource Handles
===================================*/

// Useful typedefs for typesafe, internal resource handles

#define neko_handle(TYPE) neko_handle_##TYPE

#define neko_handle_decl(TYPE)                                              \
    typedef struct {                                                        \
        u32 id;                                                             \
    } neko_handle(TYPE);                                                    \
    NEKO_FORCE_INLINE neko_handle(TYPE) neko_handle_invalid_##TYPE() {      \
        neko_handle(TYPE) h;                                                \
        h.id = UINT32_MAX;                                                  \
        return h;                                                           \
    }                                                                       \
                                                                            \
    NEKO_FORCE_INLINE neko_handle(TYPE) neko_handle_create_##TYPE(u32 id) { \
        neko_handle(TYPE) h;                                                \
        h.id = id;                                                          \
        return h;                                                           \
    }

#define neko_handle_invalid(__TYPE) neko_handle_invalid_##__TYPE()

#define neko_handle_create(__TYPE, __ID) neko_handle_create_##__TYPE(__ID)

#define neko_handle_is_valid(HNDL) ((HNDL.id) != UINT32_MAX)

/*===================================
// Color
===================================*/

#define neko_hsv(...) neko_hsv_ctor(__VA_ARGS__)
#define neko_color(...) neko_color_ctor(__VA_ARGS__)

typedef struct neko_hsv_t {
    union {
        f32 hsv[3];
        struct {
            f32 h, s, v;
        };
    };
} neko_hsv_t;

NEKO_FORCE_INLINE neko_hsv_t neko_hsv_ctor(f32 h, f32 s, f32 v) {
    neko_hsv_t hsv;
    hsv.h = h;
    hsv.s = s;
    hsv.v = v;
    return hsv;
}

typedef struct neko_color_t {
    union {
        u8 rgba[4];
        struct {
            u8 r, g, b, a;
        };
    };
} neko_color_t;

NEKO_FORCE_INLINE neko_color_t neko_color_ctor(u8 r, u8 g, u8 b, u8 a) {
    neko_color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

#define NEKO_COLOR_BLACK neko_color(0, 0, 0, 255)
#define NEKO_COLOR_WHITE neko_color(255, 255, 255, 255)
#define NEKO_COLOR_RED neko_color(255, 0, 0, 255)
#define NEKO_COLOR_GREEN neko_color(0, 255, 0, 255)
#define NEKO_COLOR_BLUE neko_color(0, 0, 255, 255)
#define NEKO_COLOR_ORANGE neko_color(255, 100, 0, 255)
#define NEKO_COLOR_YELLOW neko_color(255, 255, 0, 255)
#define NEKO_COLOR_PURPLE neko_color(128, 0, 128, 255)
#define NEKO_COLOR_MAROON neko_color(128, 0, 0, 255)
#define NEKO_COLOR_BROWN neko_color(165, 42, 42, 255)

NEKO_FORCE_INLINE neko_color_t neko_color_alpha(neko_color_t c, u8 a) { return neko_color(c.r, c.g, c.b, a); }

NEKO_FORCE_INLINE neko_hsv_t neko_rgb2hsv(neko_color_t in) {
    f32 ir = (f32)in.r / 255.f;
    f32 ig = (f32)in.g / 255.f;
    f32 ib = (f32)in.b / 255.f;
    f32 ia = (f32)in.a / 255.f;

    neko_hsv_t out = NEKO_DEFAULT_VAL();
    f64 min, max, delta;

    min = ir < ig ? ir : ig;
    min = min < ib ? min : ib;

    max = ir > ig ? ir : ig;
    max = max > ib ? max : ib;

    out.v = max;  // v
    delta = max - min;
    if (delta < 0.00001) {
        out.s = 0;
        out.h = 0;  // undefined, maybe nan?
        return out;
    }

    if (max > 0.0) {            // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;  // its now undefined
        return out;
    }
    if (ir >= max)                  // > is bogus, just keeps compilor happy
        out.h = (ig - ib) / delta;  // between yellow & magenta
    else if (ig >= max)
        out.h = 2.0 + (ib - ir) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + (ir - ig) / delta;  // between magenta & cyan

    out.h *= 60.0;  // degrees

    if (out.h < 0.0) out.h += 360.0;

    return out;
}

NEKO_FORCE_INLINE neko_color_t neko_hsv2rgb(neko_hsv_t in) {
    f64 hh, p, q, t, ff;
    long i;
    neko_color_t out;

    if (in.s <= 0.0) {  // < is bogus, just shuts up warnings
        out.r = in.v * 255;
        out.g = in.v * 255;
        out.b = in.v * 255;
        out.a = 255;
        return out;
    }
    hh = in.h;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    u8 iv = in.v * 255;
    u8 it = t * 255;
    u8 ip = p * 255;
    u8 iq = q * 255;

    switch (i) {
        case 0:
            out.r = iv;
            out.g = it;
            out.b = ip;
            break;
        case 1:
            out.r = iq;
            out.g = iv;
            out.b = ip;
            break;
        case 2:
            out.r = ip;
            out.g = iv;
            out.b = it;
            break;

        case 3:
            out.r = ip;
            out.g = iq;
            out.b = iv;
            break;
        case 4:
            out.r = it;
            out.g = ip;
            out.b = iv;
            break;
        case 5:
        default:
            out.r = iv;
            out.g = ip;
            out.b = iq;
            break;
    }
    return out;
}

#define neko_println(__FMT, ...)           \
    do {                                   \
        neko_printf(__FMT, ##__VA_ARGS__); \
        neko_printf("\n");                 \
    } while (0)

#ifndef neko_fprintf
NEKO_FORCE_INLINE void neko_fprintf(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
}
#endif

NEKO_FORCE_INLINE void neko_fprintln(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    neko_fprintf(fp, "\n");
}

NEKO_FORCE_INLINE void neko_fprintln_t(FILE* fp, u32 tabs, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (u32 i = 0; i < tabs; ++i) {
        neko_fprintf(fp, "\t");
    }
    vfprintf(fp, fmt, args);
    va_end(args);
    neko_fprintf(fp, "\n");
}

#ifdef __MINGW32__
#define neko_snprintf(__NAME, __SZ, __FMT, ...) __mingw_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__)
#else
NEKO_FORCE_INLINE void neko_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
}
#endif

#define neko_transient_buffer(__N, __SZ) \
    char __N[__SZ] = NEKO_DEFAULT_VAL(); \
    memset(__N, 0, __SZ);

#define neko_snprintfc(__NAME, __SZ, __FMT, ...) \
    char __NAME[__SZ] = NEKO_DEFAULT_VAL();      \
    neko_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__);

NEKO_FORCE_INLINE u32 neko_util_safe_truncate_u64(u64 value) {
    NEKO_ASSERT(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

/*=============================
// Console
=============================*/

typedef void (*neko_console_func)(int argc, char** argv);

typedef struct neko_console_command_t {
    neko_console_func func;
    const char* name;
    const char* desc;
} neko_console_command_t;

typedef struct neko_console_t {
    char tb[2048];     // text buffer
    char cb[10][256];  // "command" buffer
    int current_cb_idx;

    f32 y;
    f32 size;
    f32 open_speed;
    f32 close_speed;

    bool open;
    int last_open_state;
    bool autoscroll;

    neko_console_command_t* commands;
    int commands_len;
} neko_console_t;

NEKO_API_DECL neko_console_t g_console;

NEKO_API_DECL void neko_console_printf(neko_console_t* console, const char* fmt, ...);

NEKO_API_DECL s32 neko_buildnum(void);

/*=============================
// C API
=============================*/

typedef struct vfs_file {
    const_str data;
    size_t len;
    u64 offset;
} vfs_file;

NEKO_API_DECL size_t neko_capi_vfs_fread(void* dest, size_t size, size_t count, vfs_file* vf);
NEKO_API_DECL int neko_capi_vfs_fseek(vfs_file* vf, u64 of, int whence);
NEKO_API_DECL u64 neko_capi_vfs_ftell(vfs_file* vf);
NEKO_API_DECL vfs_file neko_capi_vfs_fopen(const_str path);
NEKO_API_DECL int neko_capi_vfs_fclose(vfs_file* vf);

NEKO_API_DECL bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);
NEKO_API_DECL const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t* size);

#endif  // NEKO_H
