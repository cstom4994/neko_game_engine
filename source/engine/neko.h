

#ifndef NEKO_H
#define NEKO_H

#include <assert.h>  // assert
#include <ctype.h>   // tolower
#include <float.h>   // FLT_MAX
#include <limits.h>  // INT32_MAX, UINT32_MAX
#include <malloc.h>  // alloca/_alloca
#include <math.h>    // floor, acos, sin, sqrt, tan
#include <stdarg.h>  // valist
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>  // ptrdiff_t
#include <stdint.h>  // standard types
#include <stdio.h>   // FILE
#include <stdio.h>
#include <stdlib.h>  // malloc, realloc, free
#include <string.h>  // memset
#include <time.h>    // time
#include <time.h>

/*========================
// Defines
========================*/

#ifndef neko_inline
#define neko_inline inline
#endif

#ifndef neko_local_persist
#define neko_local_persist static
#endif

#ifndef neko_global
#define neko_global static
#endif

#ifndef neko_static_inline
#define neko_static_inline static neko_inline
#endif

#if (defined _WIN32 || defined _WIN64)
#define neko_force_inline neko_inline
#elif (defined __APPLE__ || defined _APPLE)
#define neko_force_inline static __attribute__((always_inline))
#else
#define neko_force_inline neko_inline
#endif

#ifndef neko_private
#define neko_private(_result_type) static _result_type
#endif

#ifndef neko_public
#define neko_public(_result_type) _result_type
#endif

#ifndef neko_little_endian
#define neko_little_endian 1
#endif

#ifndef neko_bit
#define neko_bit(x) (1 << x)
#endif

/*===================
// NEKO_API_DECL
===================*/

#ifdef NEKO_API_DLL_EXPORT
#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C" __declspec(dllexport)
#else
#define NEKO_API_EXTERN extern __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C"
#define neko_cpp_src
#else
#define NEKO_API_EXTERN extern
#endif
#endif

#define NEKO_API_DECL NEKO_API_EXTERN
#define NEKO_API_PRIVATE NEKO_API_EXTERN

/*===================
// PLATFORM DEFINES
===================*/

/* Platform Android */
#if (defined __ANDROID__)

#define NEKO_PLATFORM_ANDROID

/* Platform Apple */
#elif (defined __APPLE__ || defined _APPLE)

#define NEKO_PLATFORM_APPLE

/* Platform Windows */
#elif (defined _WIN32 || defined _WIN64)

#define __USE_MINGW_ANSI_STDIO 1

// Necessary windows defines before including windows.h, because it's retarded.
#define OEMRESOURCE

#define NEKO_PLATFORM_WIN
#define NEKO_PLATFORM_WINDOWS
#include <windows.h>

#define WIN32_LEAN_AND_MEAN

/* Platform Linux */
#elif (defined linux || defined _linux || defined __linux__)

#define NEKO_PLATFORM_LINUX

/* Platform Emscripten */
#elif (defined __EMSCRIPTEN__)

#define NEKO_PLATFORM_WEB

/* Else - Platform Undefined and Unsupported or custom */

#endif

/*============================================================
// C primitive types
============================================================*/

#ifndef neko_cpp_src
#define false 0
#define true 1
#endif

#ifdef neko_cpp_src
typedef bool b8;
#else
#ifndef __bool_true_false_are_defined
typedef _Bool bool;
#endif
typedef bool b8;
#endif

typedef size_t usize;

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef s32 b32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef const char* const_str;
typedef int32_t bool32_t;

typedef bool32_t b32;
typedef intptr_t iptr;
typedef uintptr_t uptr;

#define u16_max UINT16_MAX
#define u32_max UINT32_MAX
#define s32_max INT32_MAX
#define f32_max FLT_MAX
#define f32_min FLT_MIN

// 提供于静态反射和序列化模块
enum neko_type_kind {
    TYPE_KIND_S8 = 0x00,
    TYPE_KIND_S16,
    TYPE_KIND_S32,
    TYPE_KIND_S64,
    TYPE_KIND_U8,
    TYPE_KIND_U16,
    TYPE_KIND_U32,
    TYPE_KIND_U64,
    TYPE_KIND_F32,
    TYPE_KIND_F64,
    TYPE_KIND_BOOL,
    TYPE_KIND_CHAR,
    TYPE_KIND_STR,
    TYPE_KIND_VOID,
    TYPE_KIND_POINTER,
    TYPE_KIND_ARRAY,
    TYPE_KIND_ENUM,
    TYPE_KIND_STRUCT,
    TYPE_KIND_COUNT
};

// Helper macro for compiling to nothing
#define neko_empty_instruction(...)

#define neko_array_size(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

#define neko_assert(x, ...)                                                                                            \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            neko_printf("assertion failed: (%s), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__); \
            __debugbreak();                                                                                            \
        }                                                                                                              \
    } while (0)

#if defined(__cplusplus)
#define neko_default_val() \
    {}
#else
#define neko_default_val() \
    { 0 }
#endif

// Helper macro for an in place for-range loop
#define neko_for_range_i(__COUNT) for (u32 i = 0; i < __COUNT; ++i)

// Helper macro for an in place for-range loop
#define neko_for_range_j(__COUNT) for (u32 j = 0; j < __COUNT; ++j)

#define neko_for_range(__COUNT) for (u32 neko_macro_token_paste(__T, __LINE__) = 0; neko_macro_token_paste(__T, __LINE__) < __COUNT; ++(neko_macro_token_paste(__T, __LINE__)))

#define neko_max(A, B) ((A) > (B) ? (A) : (B))

#define neko_min(A, B) ((A) < (B) ? (A) : (B))

#define neko_clamp(V, MIN, MAX) ((V) > (MAX) ? (MAX) : (V) < (MIN) ? (MIN) : (V))

#define neko_is_nan(V) ((V) != (V))

#define neko_bool_str(V) (V ? "true" : "false")

// Helpful macro for casting one type to another
#define neko_cast(A, B) ((A*)(B))

#define neko_r_cast reinterpret_cast
#define neko_s_cast static_cast
#define neko_c_cast const_cast

#ifdef __cplusplus
#define neko_ctor(TYPE, ...) (TYPE{__VA_ARGS__})
#else
#define neko_ctor(TYPE, ...) ((TYPE){__VA_ARGS__})
#endif

// Helpful marco for calculating offset (in bytes) of an element from a given structure type
#define neko_offset(TYPE, ELEMENT) ((size_t)(&(((TYPE*)(0))->ELEMENT)))

// macro for turning any given type into a const char* of itself
#define neko_to_str(TYPE) ((const char*)#TYPE)

#define neko_macro_token_paste(X, Y) X##Y
#define neko_macro_cat(X, Y) neko_macro_token_paste(X, Y)

#define neko_timed_action(INTERVAL, ...)                                     \
    do {                                                                     \
        static u32 neko_macro_cat(neko_macro_cat(__T, __LINE__), t) = 0;     \
        if (neko_macro_cat(neko_macro_cat(__T, __LINE__), t)++ > INTERVAL) { \
            neko_macro_cat(neko_macro_cat(__T, __LINE__), t) = 0;            \
            __VA_ARGS__                                                      \
        }                                                                    \
    } while (0)

#define neko_concat(x, y) neko_concat_impl(x, y)
#define neko_concat_impl(x, y) x##y

#define neko_invoke_once(...)                           \
    static char neko_concat(unused, __LINE__) = [&]() { \
        __VA_ARGS__;                                    \
        return '\0';                                    \
    }();                                                \
    (void)neko_concat(unused, __LINE__)

#define neko_int2voidp(I) (void*)(uintptr_t)(I)

#define neko_if(INIT, CHECK, ...) \
    do {                          \
        INIT;                     \
        if (CHECK) {              \
            __VA_ARGS__           \
        }                         \
    } while (0)

// Logging

#define neko_log_info(...) log_info(__VA_ARGS__)
#define neko_log_success(...) log_trace(__VA_ARGS__)
#define neko_log_warning(...) log_warn(__VA_ARGS__)
#define neko_log_error(MESSAGE, ...)                                                                   \
    do {                                                                                               \
        neko_println("ERROR::%s::%s(%zu)::" MESSAGE, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        neko_assert(false);                                                                            \
    } while (0)

#define neko_enum_flag_operator(T)                                                                                                                                         \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                     \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T& operator|=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator&=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator^=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define neko_move_only(class_name)                     \
    class_name(const class_name&) = delete;            \
    class_name& operator=(const class_name&) = delete; \
    class_name(class_name&&) = default;                \
    class_name& operator=(class_name&&) = default

#define neko_aligned_buffer(name) alignas(16) static const std::u8 name[]

#define neko_va_unpack(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

#define neko_offsetof(s, m) ((::size_t) & reinterpret_cast<char const volatile&>((((s*)0)->m)))

#if defined(_MSC_VER)
#define neko_unused(x) (void)x
#else
#define neko_unused(x) (void)(sizeof(x))
#endif

#if defined(__cplusplus)
#include <string>
#if defined(__cpp_char8_t)
template <typename T>
const char* u8Cpp20(T&& t) noexcept {
#pragma warning(disable : 26490)
    return reinterpret_cast<const char*>(t);
#pragma warning(default : 26490)
}
#define neko_str(x) u8Cpp20(u8##x)
#else
#define neko_str(x) u8##x
#endif
#else
#define neko_str(x) x
#endif

// Helper macro for typedefing a struture definition
#define neko_struct_def(name, ...) \
    typedef struct {               \
        __VA_ARGS__                \
    } name

// Definition for derived struct (based another parent struct)
#define neko_derive_def(name, parent, ...) neko_struct_def(name, parent _base; __VA_ARGS__)

#define neko_engine_check(statement) (statement) ? true : false

#define _base(base_type) base_type _base

#ifdef neko_cpp_src

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#ifndef neko_check_is_trivial
#define neko_check_is_trivial(type, err) static_assert(std::is_trivial<type>::value, err)
#endif

#define neko_malloc_init(type)                   \
    (type*)_neko_malloc_init_impl(sizeof(type)); \
    neko_check_is_trivial(type, "try to init a non-trivial object")

#define neko_malloc_init_ex(name, type)                              \
    neko_check_is_trivial(type, "try to init a non-trivial object"); \
    struct type* name = neko_malloc_init(type)

typedef std::string neko_string;
typedef std::string_view neko_string_view;

// 一种向任何指针添加字节偏移量的可移植且安全的方法
// https://stackoverflow.com/questions/15934111/portable-and-safe-way-to-add-byte-offset-to-any-pointer
template <typename T>
neko_inline void neko_addoffset(std::ptrdiff_t offset, T*& ptr) {
    if (!ptr) return;
    ptr = (T*)((unsigned char*)ptr + offset);
}

template <typename T>
neko_inline T* neko_addoffset_r(std::ptrdiff_t offset, T* ptr) {
    if (!ptr) return nullptr;
    return (T*)((unsigned char*)ptr + offset);
}

template <typename T, size_t size>
class size_checker {
    static_assert(sizeof(T) == size, "check the size of integral types");
};

template class size_checker<s64, 8>;
template class size_checker<s32, 4>;
template class size_checker<s16, 2>;
template class size_checker<s8, 1>;
template class size_checker<u64, 8>;
template class size_checker<u32, 4>;
template class size_checker<u16, 2>;
template class size_checker<u8, 1>;
template class size_checker<b32, 4>;

namespace neko {

template <typename V, typename Alloc = std::allocator<V>>
using vector = std::vector<V, Alloc>;

template <typename T>
struct cpp_remove_reference {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T&> {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T&&> {
    using type = T;
};

template <typename T>
constexpr typename cpp_remove_reference<T>::type&& cpp_move(T&& arg) noexcept {
    return (typename cpp_remove_reference<T>::type&&)arg;
}

template <typename T>
using initializer_list = std::initializer_list<T>;

template <typename T>
using neko_function = std::function<T>;

template <typename T>
using scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr scope<T> create_scope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr ref<T> create_ref(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
struct neko_named_func {
    std::string name;
    neko_function<T> func;
};

}  // namespace neko

#endif  // neko_cpp_src

#pragma region neko_mem

// Operating system function pointer
typedef struct neko_os_api_s {
    void* (*malloc)(size_t sz);
    void (*free)(void* ptr);
    void* (*realloc)(void* ptr, size_t sz);
    void* (*calloc)(size_t num, size_t sz);
    void* (*malloc_init)(size_t sz);
    char* (*strdup)(const char* str);
} neko_os_api_t;

// TODO: Check if all defaults need to be set, in case neko context will not be used

NEKO_API_DECL
void* _neko_malloc_init_impl(size_t sz);

// Default memory allocations
#define neko_malloc malloc
#define neko_free free
#define neko_realloc realloc
#define neko_calloc calloc

#define neko_malloc_init(__T) (__T*)_neko_malloc_init_impl(sizeof(__T))

NEKO_API_DECL neko_os_api_t neko_os_api_new_default();

#ifndef neko_os_api_new
#define neko_os_api_new neko_os_api_new_default
#endif

#ifndef neko_strdup
#define neko_strdup(__STR) (neko_ctx()->os.strdup(__STR))
#endif

NEKO_API_DECL void* __neko_mem_safe_alloc(size_t size, const char* file, int line, size_t* statistics);
NEKO_API_DECL void* __neko_mem_safe_calloc(size_t count, size_t element_size, const char* file, int line, size_t* statistics);
NEKO_API_DECL void __neko_mem_safe_free(void* mem, size_t* statistics);
NEKO_API_DECL void* __neko_mem_safe_realloc(void* ptr, size_t new_size, const char* file, int line, size_t* statistics);

#if 1

#define neko_safe_malloc(size) __neko_mem_safe_alloc((size), (char*)__FILE__, __LINE__, NULL)
#define neko_safe_free(mem) __neko_mem_safe_free(mem, NULL)
#define neko_safe_realloc(ptr, size) __neko_mem_safe_realloc((ptr), (size), (char*)__FILE__, __LINE__, NULL)
#define neko_safe_calloc(count, element_size) __neko_mem_safe_calloc(count, element_size, (char*)__FILE__, __LINE__, NULL)

#else

#define neko_safe_malloc neko_malloc
#define neko_safe_free neko_free
#define neko_safe_realloc neko_realloc
#define neko_safe_calloc neko_calloc

#endif

#ifdef neko_cpp_src

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
    void* ptr;
    int flags;
    size_t size, hash;
    void (*dtor)(void*);
} neko_gc_ptr_t;

typedef struct {
    void* bottom;
    int paused;
    uintptr_t minptr, maxptr;
    neko_gc_ptr_t *items, *frees;
    f64 loadfactor, sweepfactor;
    size_t nitems, nslots, mitems, nfrees;
} neko_gc_t;

void neko_gc_start(neko_gc_t* gc, void* stk);
void neko_gc_stop(neko_gc_t* gc);
void neko_gc_pause(neko_gc_t* gc);
void neko_gc_resume(neko_gc_t* gc);
void neko_gc_run(neko_gc_t* gc);

// GC 公开函数
void* neko_gc_alloc(neko_gc_t* gc, size_t size);
void* neko_gc_calloc(neko_gc_t* gc, size_t num, size_t size);
void* neko_gc_realloc(neko_gc_t* gc, void* ptr, size_t size);
void neko_gc_free(neko_gc_t* gc, void* ptr);

void* __neko_gc_alloc_opt(neko_gc_t* gc, size_t size, int flags, void (*dtor)(void*));
void* __neko_gc_calloc_opt(neko_gc_t* gc, size_t num, size_t size, int flags, void (*dtor)(void*));

void neko_gc_set_dtor(neko_gc_t* gc, void* ptr, void (*dtor)(void*));
void neko_gc_set_flags(neko_gc_t* gc, void* ptr, int flags);
int neko_gc_get_flags(neko_gc_t* gc, void* ptr);
void (*neko_gc_get_dtor(neko_gc_t* gc, void* ptr))(void*);
size_t neko_gc_get_size(neko_gc_t* gc, void* ptr);

extern neko_gc_t g_gc;

void __neko_mem_init(int argc, char** argv);
void __neko_mem_end();
void __neko_mem_rungc();

#pragma endregion

// logging

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    struct tm* time;
    FILE* udata;
    int line;
    int level;
} neko_log_event;

typedef void (*neko_log_fn)(neko_log_event* ev);
typedef void (*neko_log_lock_fn)(bool lock, void* udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

NEKO_API_DECL const char* log_level_string(int level);
NEKO_API_DECL void log_set_lock(neko_log_lock_fn fn, void* udata);
NEKO_API_DECL void log_set_level(int level);
NEKO_API_DECL void log_set_quiet(bool enable);
NEKO_API_DECL int log_add_callback(neko_log_fn fn, void* udata, int level);
NEKO_API_DECL int log_add_fp(FILE* fp, int level);

NEKO_API_DECL void log_log(int level, const char* file, int line, const char* fmt, ...);

/*============================================================
// Result
============================================================*/

typedef enum neko_result { NEKO_RESULT_SUCCESS, NEKO_RESULT_IN_PROGRESS, NEKO_RESULT_INCOMPLETE, NEKO_RESULT_FAILURE } neko_result;

/*===================================
// Resource Handles
===================================*/

// Useful typedefs for typesafe, internal resource handles

#define neko_handle(TYPE) neko_handle_##TYPE

#define neko_handle_decl(TYPE)                                        \
    typedef struct {                                                  \
        u32 id;                                                       \
    } neko_handle(TYPE);                                              \
    neko_inline neko_handle(TYPE) neko_handle_invalid_##TYPE() {      \
        neko_handle(TYPE) h;                                          \
        h.id = UINT32_MAX;                                            \
        return h;                                                     \
    }                                                                 \
                                                                      \
    neko_inline neko_handle(TYPE) neko_handle_create_##TYPE(u32 id) { \
        neko_handle(TYPE) h;                                          \
        h.id = id;                                                    \
        return h;                                                     \
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

neko_force_inline neko_hsv_t neko_hsv_ctor(f32 h, f32 s, f32 v) {
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

neko_force_inline neko_color_t neko_color_ctor(u8 r, u8 g, u8 b, u8 a) {
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

neko_force_inline neko_color_t neko_color_alpha(neko_color_t c, u8 a) { return neko_color(c.r, c.g, c.b, a); }

neko_force_inline neko_hsv_t neko_rgb2hsv(neko_color_t in) {
    f32 ir = (f32)in.r / 255.f;
    f32 ig = (f32)in.g / 255.f;
    f32 ib = (f32)in.b / 255.f;
    f32 ia = (f32)in.a / 255.f;

    neko_hsv_t out = neko_default_val();
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

neko_force_inline neko_color_t neko_hsv2rgb(neko_hsv_t in) {
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

/*===================================
// String Utils
===================================*/

neko_force_inline u32 neko_string_length(const char* txt) {
    u32 sz = 0;
    while (txt != NULL && txt[sz] != '\0') {
        sz++;
    }
    return sz;
}

#define neko_strlen neko_string_length

// Expects null terminated strings
neko_force_inline b32 neko_string_compare_equal(const char* txt, const char* cmp) {
    // Grab sizes of both strings
    u32 a_sz = neko_string_length(txt);
    u32 b_sz = neko_string_length(cmp);

    // Return false if sizes do not match
    if (a_sz != b_sz) {
        return false;
    }

    for (u32 i = 0; i < a_sz; ++i) {
        if (*txt++ != *cmp++) {
            return false;
        }
    };

    return true;
}

neko_force_inline b32 neko_string_compare_equal_n(const char* txt, const char* cmp, u32 n) {
    u32 a_sz = neko_string_length(txt);
    u32 b_sz = neko_string_length(cmp);

    // Not enough characters to do operation
    if (a_sz < n || b_sz < n) {
        return false;
    }

    for (u32 i = 0; i < n; ++i) {
        if (*txt++ != *cmp++) {
            return false;
        }
    };

    return true;
}

neko_force_inline void neko_util_str_to_lower(const char* src, char* buffer, size_t buffer_sz) {
    size_t src_sz = neko_string_length(src);
    size_t len = neko_min(src_sz, buffer_sz);

    for (u32 i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
}

neko_force_inline b32 neko_util_str_is_numeric(const char* str) {
    const char* at = str;
    while (at && *at) {
        while (*at == '\n' || *at == '\t' || *at == ' ' || *at == '\r') at++;
        ;
        char c = *at++;
        if (c < '0' || c > '9') {
            return false;
        }
    }

    return true;
}

// Will return a null buffer if file does not exist or allocation fails
NEKO_API_DECL char* neko_read_file_contents_into_string_null_term(const char* file_path, const char* mode, size_t* _sz);

neko_force_inline b32 neko_util_file_exists(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

neko_force_inline void neko_util_get_file_extension(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* at = (file_path + str_len - 1);
    while (*at != '.' && at != file_path) {
        at--;
    }

    if (*at == '.') {
        at++;
        u32 i = 0;
        while (*at) {
            char c = *at;
            buffer[i++] = *at++;
        }
        buffer[i] = '\0';
    }
}

neko_force_inline void neko_util_get_dir_from_file(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* end = (file_path + str_len);
    for (u32 i = 0; i < str_len; ++i) {
        if (file_path[i] == '/' || file_path[i] == '\\') {
            end = &file_path[i];
        }
    }

    size_t dir_len = end - file_path;
    memcpy(buffer, file_path, neko_min(buffer_size, dir_len + 1));
    if (dir_len + 1 <= buffer_size) {
        buffer[dir_len] = '\0';
    }
}

neko_force_inline void neko_util_get_file_name(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* file_start = file_path;
    const char* file_end = (file_path + str_len);
    for (u32 i = 0; i < str_len; ++i) {
        if (file_path[i] == '/' || file_path[i] == '\\') {
            file_start = &file_path[i + 1];
        } else if (file_path[i] == '.') {
            file_end = &file_path[i];
        }
    }

    size_t dir_len = file_end - file_start;
    memcpy(buffer, file_start, neko_min(buffer_size, dir_len + 1));
    if (dir_len + 1 <= buffer_size) {
        buffer[dir_len] = '\0';
    }
}

neko_force_inline void neko_util_string_substring(const char* src, char* dst, size_t sz, u32 start, u32 end) {
    u32 str_len = neko_string_length(src);
    if (end > str_len) {
        end = str_len;
    }
    if (start > str_len) {
        start = str_len;
    }

    const char* at = src + start;
    const char* e = src + end;
    u32 ct = 0;
    while (at && *at != '\0' && at != e) {
        dst[ct] = *at;
        at++;
        ct++;
    }
}

neko_force_inline void neko_util_string_remove_character(const char* src, char* buffer, u32 buffer_size, char delimiter) {
    u32 ct = 0;
    u32 str_len = neko_string_length(src);
    const char* at = src;
    while (at && *at != '\0' && ct < buffer_size) {
        char c = *at;
        if (c != delimiter) {
            buffer[ct] = c;
            ct++;
        }
        at++;
    }
}

neko_force_inline void neko_util_string_replace(char* buffer, size_t buffer_sz, const char* replace, char fallback) {
    // Replace all characters with characters of keyword, then the rest replace with spaces
    size_t len = neko_string_length(replace);
    for (u32 c = 0; c < buffer_sz; ++c) {
        if (c < len) {
            buffer[c] = replace[c];
        } else {
            buffer[c] = fallback;
        }
    }
}

neko_force_inline void neko_util_string_replace_delim(const char* source_str, char* buffer, u32 buffer_size, char delimiter, char replace) {
    u32 str_len = neko_string_length(source_str);
    const char* at = source_str;
    while (at && *at != '\0') {
        char c = *at;
        if (c == delimiter) {
            c = replace;
        }
        buffer[(at - source_str)] = c;
        at++;
    }
}

neko_force_inline void neko_util_normalize_path(const char* path, char* buffer, u32 buffer_size) {
    // Normalize the path somehow...
}

// Custom printf defines
#ifndef neko_printf

#ifdef __MINGW32__

#define neko_printf(__FMT, ...) __mingw_printf(__FMT, ##__VA_ARGS__)

#elif (defined NEKO_PLATFORM_ANDROID)

#include <android/log.h>

#define neko_printf(__FMT, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __FMT, ##__VA_ARGS__))

#else
neko_force_inline void neko_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#endif

#endif

#define neko_println(__FMT, ...)           \
    do {                                   \
        neko_printf(__FMT, ##__VA_ARGS__); \
        neko_printf("\n");                 \
    } while (0)

#ifndef neko_fprintf
neko_force_inline void neko_fprintf(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
}
#endif

neko_force_inline void neko_fprintln(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    neko_fprintf(fp, "\n");
}

neko_force_inline void neko_fprintln_t(FILE* fp, u32 tabs, const char* fmt, ...) {
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
neko_force_inline void neko_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
}
#endif

#define neko_transient_buffer(__N, __SZ) \
    char __N[__SZ] = neko_default_val(); \
    memset(__N, 0, __SZ);

#define neko_snprintfc(__NAME, __SZ, __FMT, ...) \
    char __NAME[__SZ] = neko_default_val();      \
    neko_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__);

neko_force_inline u32 neko_util_safe_truncate_u64(u64 value) {
    neko_assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

neko_force_inline u32 neko_hash_uint32_t(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

#define neko_hash_u32_ip(__X, __OUT)                 \
    do {                                             \
        __OUT = ((__X >> 16) ^ __X) * 0x45d9f3b;     \
        __OUT = ((__OUT >> 16) ^ __OUT) * 0x45d9f3b; \
        __OUT = (__OUT >> 16) ^ __OUT;               \
    } while (0)

neko_force_inline u32 neko_hash_u64(u64 x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (u32)x;
}

// Note: source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
neko_force_inline u32 neko_hash_str(const char* str) {
    u32 hash = 5381;
    s32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

neko_force_inline u64 neko_hash_str64(const char* str) {
    u32 hash1 = 5381;
    u32 hash2 = 52711;
    u32 i = neko_string_length(str);
    while (i--) {
        char c = str[i];
        hash1 = (hash1 * 33) ^ c;
        hash2 = (hash2 * 33) ^ c;
    }

    return (hash1 >> 0) * 4096 + (hash2 >> 0);
}

neko_force_inline bool neko_compare_bytes(void* b0, void* b1, size_t len) { return 0 == memcmp(b0, b1, len); }

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define NEKO_SIZE_T_BITS ((sizeof(size_t)) * 8)
#define NEKO_SIPHASH_C_ROUNDS 1
#define NEKO_SIPHASH_D_ROUNDS 1
#define neko_rotate_left(__V, __N) (((__V) << (__N)) | ((__V) >> (NEKO_SIZE_T_BITS - (__N))))
#define neko_rotate_right(__V, __N) (((__V) >> (__N)) | ((__V) << (NEKO_SIZE_T_BITS - (__N))))

neko_force_inline size_t neko_hash_siphash_bytes(void* p, size_t len, size_t seed) {
    unsigned char* d = (unsigned char*)p;
    size_t i, j;
    size_t v0, v1, v2, v3, data;

    // hash that works on 32- or 64-bit registers without knowing which we have
    // (computes different results on 32-bit and 64-bit platform)
    // derived from siphash, but on 32-bit platforms very different as it uses 4 32-bit state not 4 64-bit
    v0 = ((((size_t)0x736f6d65 << 16) << 16) + 0x70736575) ^ seed;
    v1 = ((((size_t)0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
    v2 = ((((size_t)0x6c796765 << 16) << 16) + 0x6e657261) ^ seed;
    v3 = ((((size_t)0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

#ifdef STBDS_TEST_SIPHASH_2_4
    // hardcoded with key material in the siphash test vectors
    v0 ^= 0x0706050403020100ull ^ seed;
    v1 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
    v2 ^= 0x0706050403020100ull ^ seed;
    v3 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
#endif

#define neko_sipround()                                  \
    do {                                                 \
        v0 += v1;                                        \
        v1 = neko_rotate_left(v1, 13);                   \
        v1 ^= v0;                                        \
        v0 = neko_rotate_left(v0, NEKO_SIZE_T_BITS / 2); \
        v2 += v3;                                        \
        v3 = neko_rotate_left(v3, 16);                   \
        v3 ^= v2;                                        \
        v2 += v1;                                        \
        v1 = neko_rotate_left(v1, 17);                   \
        v1 ^= v2;                                        \
        v2 = neko_rotate_left(v2, NEKO_SIZE_T_BITS / 2); \
        v0 += v3;                                        \
        v3 = neko_rotate_left(v3, 21);                   \
        v3 ^= v0;                                        \
    } while (0)

    for (i = 0; i + sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
        data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
        data |= (size_t)(d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16;  // discarded if size_t == 4

        v3 ^= data;
        for (j = 0; j < NEKO_SIPHASH_C_ROUNDS; ++j) neko_sipround();
        v0 ^= data;
    }
    data = len << (NEKO_SIZE_T_BITS - 8);
    switch (len - i) {
        case 7:
            data |= ((size_t)d[6] << 24) << 24;  // fall through
        case 6:
            data |= ((size_t)d[5] << 20) << 20;  // fall through
        case 5:
            data |= ((size_t)d[4] << 16) << 16;  // fall through
        case 4:
            data |= (d[3] << 24);  // fall through
        case 3:
            data |= (d[2] << 16);  // fall through
        case 2:
            data |= (d[1] << 8);  // fall through
        case 1:
            data |= d[0];  // fall through
        case 0:
            break;
    }
    v3 ^= data;
    for (j = 0; j < NEKO_SIPHASH_C_ROUNDS; ++j) neko_sipround();
    v0 ^= data;
    v2 ^= 0xff;
    for (j = 0; j < NEKO_SIPHASH_D_ROUNDS; ++j) neko_sipround();

#if 0
  return v0^v1^v2^v3;
#else
    return v1 ^ v2 ^ v3;  // slightly stronger since v0^v3 in above cancels out final round operation? I tweeted at the authors of SipHash about this but they didn't reply
#endif
}

neko_force_inline size_t neko_hash_bytes(void* p, size_t len, size_t seed) {
#if 0
  return neko_hash_siphash_bytes(p,len,seed);
#else
    unsigned char* d = (unsigned char*)p;

    // Len == 4 (off for now, so to force 64 bit hash)
    if (len == 4) {
        unsigned int hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
        hash ^= seed;
        hash *= 0xcc9e2d51;
        hash = (hash << 17) | (hash >> 15);
        hash *= 0x1b873593;
        hash ^= seed;
        hash = (hash << 19) | (hash >> 13);
        hash = hash * 5 + 0xe6546b64;
        hash ^= hash >> 16;
        hash *= 0x85ebca6b;
        hash ^= seed;
        hash ^= hash >> 13;
        hash *= 0xc2b2ae35;
        hash ^= hash >> 16;
        return (((size_t)hash << 16 << 16) | hash) ^ seed;
    } else if (len == 8 && sizeof(size_t) == 8) {
        size_t hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
        hash |= (size_t)(d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16;  // avoid warning if size_t == 4
        hash ^= seed;
        hash = (~hash) + (hash << 21);
        hash ^= neko_rotate_right(hash, 24);
        hash *= 265;
        hash ^= neko_rotate_right(hash, 14);
        hash ^= seed;
        hash *= 21;
        hash ^= neko_rotate_right(hash, 28);
        hash += (hash << 31);
        hash = (~hash) + (hash << 18);
        return hash;
    } else {
        return neko_hash_siphash_bytes(p, len, seed);
    }
#endif
}

/* Resource Loading Util */
NEKO_API_DECL bool32_t neko_util_load_texture_data_from_file(const char* file_path, s32* width, s32* height, u32* num_comps, void** data, bool32_t flip_vertically_on_load);
NEKO_API_DECL bool32_t neko_util_load_texture_data_from_memory(const void* memory, size_t sz, s32* width, s32* height, u32* num_comps, void** data, bool32_t flip_vertically_on_load);

/** @} */  // end of neko_util

/*========================
// NEKO_MEMORY
========================*/

#define neko_ptr_add(P, BYTES) (((u8*)P + (BYTES)))

typedef struct neko_memory_block_t {
    u8* data;
    size_t size;
} neko_memory_block_t;

NEKO_API_DECL neko_memory_block_t neko_memory_block_new(size_t sz);
NEKO_API_DECL void neko_memory_block_free(neko_memory_block_t* mem);
NEKO_API_DECL size_t neko_memory_calc_padding(size_t base_address, size_t alignment);
NEKO_API_DECL size_t neko_memory_calc_padding_w_header(size_t base_address, size_t alignment, size_t header_sz);

/*================================================================================
// Linear Allocator
================================================================================*/

typedef struct neko_linear_allocator_t {
    u8* memory;
    size_t total_size;
    size_t offset;
} neko_linear_allocator_t;

NEKO_API_DECL neko_linear_allocator_t neko_linear_allocator_new(size_t sz);
NEKO_API_DECL void neko_linear_allocator_free(neko_linear_allocator_t* la);
NEKO_API_DECL void* neko_linear_allocator_allocate(neko_linear_allocator_t* la, size_t sz, size_t alignment);
NEKO_API_DECL void neko_linear_allocator_clear(neko_linear_allocator_t* la);

/*================================================================================
// Stack Allocator
================================================================================*/

typedef struct neko_stack_allocator_header_t {
    u32 size;
} neko_stack_allocator_header_t;

typedef struct neko_stack_allocator_t {
    neko_memory_block_t memory;
    size_t offset;
} neko_stack_allocator_t;

NEKO_API_DECL neko_stack_allocator_t neko_stack_allocator_new(size_t sz);
NEKO_API_DECL void neko_stack_allocator_free(neko_stack_allocator_t* sa);
NEKO_API_DECL void* neko_stack_allocator_allocate(neko_stack_allocator_t* sa, size_t sz);
NEKO_API_DECL void* neko_stack_allocator_peek(neko_stack_allocator_t* sa);
NEKO_API_DECL void* neko_stack_allocator_pop(neko_stack_allocator_t* sa);
NEKO_API_DECL void neko_stack_allocator_clear(neko_stack_allocator_t* sa);

/*================================================================================
// Heap Allocator
================================================================================*/

#ifndef NEKO_HEAP_ALLOC_DEFAULT_SIZE
#define NEKO_HEAP_ALLOC_DEFAULT_SIZE 1024 * 1024 * 20
#endif

#ifndef NEKO_HEAP_ALLOC_DEFAULT_CAPCITY
#define NEKO_HEAP_ALLOC_DEFAULT_CAPCITY 1024
#endif

typedef struct neko_heap_allocator_header_t {
    struct neko_heap_allocator_header_t* next;
    struct neko_heap_allocator_header_t* prev;
    size_t size;
} neko_heap_allocator_header_t;

typedef struct neko_heap_allocator_free_block_t {
    neko_heap_allocator_header_t* header;
    size_t size;
} neko_heap_allocator_free_block_t;

typedef struct neko_heap_allocator_t {
    neko_heap_allocator_header_t* memory;
    neko_heap_allocator_free_block_t* free_blocks;
    u32 free_block_count;
    u32 free_block_capacity;
} neko_heap_allocator_t;

NEKO_API_DECL neko_heap_allocator_t neko_heap_allocate_new();
NEKO_API_DECL void neko_heap_allocator_free(neko_heap_allocator_t* ha);
NEKO_API_DECL void* neko_heap_allocator_allocate(neko_heap_allocator_t* ha, size_t sz);
NEKO_API_DECL void neko_heap_allocator_deallocate(neko_heap_allocator_t* ha, void* memory);

/*================================================================================
// Pool Allocator
================================================================================*/

/*================================================================================
// Paged Allocator
================================================================================*/

typedef struct neko_paged_allocator_block_t {
    struct neko_paged_allocator_block_t* next;
} neko_paged_allocator_block_t;

typedef struct neko_paged_allocator_page_t {
    struct neko_paged_allocator_page_t* next;
    struct neko_paged_allocator_block_t* data;
} neko_paged_allocator_page_t;

typedef struct neko_paged_allocator_t {
    u32 block_size;
    u32 blocks_per_page;
    neko_paged_allocator_page_t* pages;
    u32 page_count;
    neko_paged_allocator_block_t* free_list;
} neko_paged_allocator_t;

NEKO_API_DECL neko_paged_allocator_t neko_paged_allocator_new(size_t element_size, size_t elements_per_page);
NEKO_API_DECL void neko_paged_allocator_free(neko_paged_allocator_t* pa);
NEKO_API_DECL void* neko_paged_allocator_allocate(neko_paged_allocator_t* pa);
NEKO_API_DECL void neko_paged_allocator_deallocate(neko_paged_allocator_t* pa, void* data);
NEKO_API_DECL void neko_paged_allocator_clear(neko_paged_allocator_t* pa);

#include "engine/neko_math.h"

neko_inline const_str neko_fs_get_filename(const_str path) {
    int len = strlen(path);
    int flag = 0;

    for (int i = len - 1; i > 0; i--) {
        if (path[i] == '\\' || path[i] == '//' || path[i] == '/') {
            flag = 1;
            path = path + i + 1;
            break;
        }
    }
    return path;
}

#ifdef neko_cpp_src

#include <algorithm>
#include <chrono>
#include <fstream>

neko_force_inline auto neko_get_time() -> s64 {
    s64 ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

neko_force_inline f64 neko_get_time_d() {
    f64 t;
    // #ifdef _WIN32
    //     FILETIME ft;
    //     GetSystemTimeAsFileTime(&ft);
    //     t = (ft.dwHighDateTime * 4294967296.0 / 1e7) + ft.dwLowDateTime / 1e7;
    //     t -= 11644473600.0;
    // #else
    //     struct timeval tv;
    //     gettimeofday(&tv, NULL);
    //     t = tv.tv_sec + tv.tv_usec / 1e6;
    // #endif
    return t;
}

neko_static_inline time_t neko_get_time_mkgmtime(struct tm* unixdate) {
    neko_assert(unixdate != nullptr);
    time_t fakeUnixtime = mktime(unixdate);
    struct tm* fakeDate = gmtime(&fakeUnixtime);

    s32 nOffSet = fakeDate->tm_hour - unixdate->tm_hour;
    if (nOffSet > 12) {
        nOffSet = 24 - nOffSet;
    }
    return fakeUnixtime - nOffSet * 3600;
}

neko_static_inline auto neko_time_to_string(std::time_t now = std::time(nullptr)) -> std::string {
    const auto tp = std::localtime(&now);
    char buffer[32];
    return std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", tp) ? buffer : "1970-01-01_00:00:00";
}

#define neko_time_count(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

class neko_timer {
public:
    neko_inline void start() noexcept { startPos = std::chrono::high_resolution_clock::now(); }
    neko_inline void stop() noexcept {
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = static_cast<f64>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] neko_inline f64 get() const noexcept { return duration; }
    ~neko_timer() noexcept { stop(); }

private:
    f64 duration = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
};

neko_static_inline std::string neko_fs_normalize_path(const std::string& path, char delimiter = '/') {
    static constexpr char delims[] = "/\\";

    std::string norm;
    norm.reserve(path.size() / 2);  // random guess, should be benchmarked

    for (auto it = path.begin(); it != path.end(); it++) {
        if (std::any_of(std::begin(delims), std::end(delims), [it](auto c) { return c == *it; })) {
            if (norm.empty() || norm.back() != delimiter) norm.push_back(delimiter);
        } else if (*it == '.') {
            if (++it == path.end()) break;
            if (std::any_of(std::begin(delims), std::end(delims), [it](auto c) { return c == *it; })) {
                continue;
            }
            if (*it != '.') throw std::logic_error("bad path");
            if (norm.empty() || norm.back() != delimiter) throw std::logic_error("bad path");

            norm.pop_back();
            while (!norm.empty()) {
                norm.pop_back();
                if (norm.back() == delimiter) {
                    norm.pop_back();
                    break;
                }
            }
        } else
            norm.push_back(*it);
    }
    if (!norm.empty() && norm.back() != delimiter) norm.push_back(delimiter);
    return norm;
}

neko_inline bool neko_fs_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

neko_inline void neko_utils_write_ppm(const int width, const int height, unsigned char* buffer, const char* filename) {

    FILE* fp = fopen(filename, "wb");

    (void)fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int i = 0; i < width * height; ++i) {
        static unsigned char color[3];
        color[0] = buffer[i];
        color[1] = buffer[i];
        color[2] = buffer[i];
        (void)fwrite(color, 1, 3, fp);
    }

    (void)fclose(fp);
};

neko_inline void neko_tex_flip_vertically(int width, int height, u8* data) {
    u8 rgb[4];
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            int top = 4 * (x + y * width);
            int bottom = 4 * (x + (height - y - 1) * width);
            memcpy(rgb, data + top, sizeof(rgb));
            memcpy(data + top, data + bottom, sizeof(rgb));
            memcpy(data + bottom, rgb, sizeof(rgb));
        }
    }
}

#endif  // neko_cpp_src

/*================================================================================
// Random
================================================================================*/

#define NEKO_STATE_VECTOR_LENGTH 624
#define NEKO_STATE_VECTOR_M 397 /* changes to STATE_VECTOR_LENGTH also require changes to this */

typedef struct neko_mt_rand_t {
    u64 mt[NEKO_STATE_VECTOR_LENGTH];
    int32_t index;
} neko_mt_rand_t;

NEKO_API_DECL neko_mt_rand_t neko_rand_seed(uint64_t seed);
NEKO_API_DECL uint64_t neko_rand_gen_long(neko_mt_rand_t* rand);
NEKO_API_DECL double neko_rand_gen(neko_mt_rand_t* rand);
NEKO_API_DECL double neko_rand_gen_range(neko_mt_rand_t* rand, double min, double max);
NEKO_API_DECL uint64_t neko_rand_gen_range_long(neko_mt_rand_t* rand, int32_t min, int32_t max);
NEKO_API_DECL neko_color_t neko_rand_gen_color(neko_mt_rand_t* rand);

// xorshf32随机数算法
// http://www.iro.umontreal.ca/~lecuyer/myftp/papers/xorshift.pdf
// xyzw -> [0, 2^32 - 1]

neko_inline u32 neko_rand_xorshf32(void) {

    u32 __neko_rand_xorshf32_x = time(NULL), __neko_rand_xorshf32_y = time(NULL), __neko_rand_xorshf32_z = time(NULL), __neko_rand_xorshf32_w = time(NULL);

    // period 2^128 - 1
    u32 tmp = (__neko_rand_xorshf32_x ^ (__neko_rand_xorshf32_x << 15));
    __neko_rand_xorshf32_x = __neko_rand_xorshf32_y;
    __neko_rand_xorshf32_y = __neko_rand_xorshf32_z;
    __neko_rand_xorshf32_z = __neko_rand_xorshf32_w;

    __neko_rand_xorshf32_w = (__neko_rand_xorshf32_w ^ (__neko_rand_xorshf32_w >> 21) ^ (tmp ^ (tmp >> 4)));
    return __neko_rand_xorshf32_w;
}

#define neko_rand_xorshf32_max 0xFFFFFFFF

// Random number in range [-1,1]
neko_inline f32 neko_rand_xor() {
    f32 r = (f32)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = 2.0f * r - 1.0f;
    return r;
}

neko_inline f32 neko_rand_range_xor(f32 lo, f32 hi) {
    f32 r = (f32)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = (hi - lo) * r + lo;
    return r;
}

/*================================================================================
// Camera
================================================================================*/

typedef enum neko_projection_type { NEKO_PROJECTION_TYPE_ORTHOGRAPHIC, NEKO_PROJECTION_TYPE_PERSPECTIVE } neko_projection_type;

typedef struct neko_camera_t {
    neko_vqs transform;
    f32 fov;
    f32 aspect_ratio;
    f32 near_plane;
    f32 far_plane;
    f32 ortho_scale;
    neko_projection_type proj_type;
} neko_camera_t;

NEKO_API_DECL neko_camera_t neko_camera_default();
NEKO_API_DECL neko_camera_t neko_camera_perspective();
NEKO_API_DECL neko_mat4 neko_camera_get_view(const neko_camera_t* cam);
NEKO_API_DECL neko_mat4 neko_camera_get_proj(const neko_camera_t* cam, s32 view_width, s32 view_height);
NEKO_API_DECL neko_mat4 neko_camera_get_view_projection(const neko_camera_t* cam, s32 view_width, s32 view_height);
NEKO_API_DECL neko_vec3 neko_camera_forward(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_backward(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_up(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_down(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_right(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_left(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_screen_to_world(const neko_camera_t* cam, neko_vec3 coords, s32 view_x, s32 view_y, s32 view_width, s32 view_height);
NEKO_API_DECL neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height);
NEKO_API_DECL void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 picth);

/*================================================================================
// Utils
================================================================================*/

// AABBs
/*
    min is top left of rect,
    max is bottom right
*/
/*
typedef struct neko_aabb_t
{
    neko_vec2 min;
    neko_vec2 max;
} neko_aabb_t;

// Collision Resolution: Minimum Translation Vector
neko_force_inline
neko_vec2 neko_aabb_aabb_mtv(neko_aabb_t* a0, neko_aabb_t* a1)
{
    neko_vec2 diff = neko_v2(a0->min.x - a1->min.x, a0->min.y - a1->min.y);

    f32 l, r, b, t;
    neko_vec2 mtv = neko_v2(0.f, 0.f);

    l = a1->min.x - a0->max.x;
    r = a1->max.x - a0->min.x;
    b = a1->min.y - a0->max.y;
    t = a1->max.y - a0->min.y;

    mtv.x = fabsf(l) > r ? r : l;
    mtv.y = fabsf(b) > t ? t : b;

    if ( fabsf(mtv.x) <= fabsf(mtv.y)) {
        mtv.y = 0.f;
    } else {
        mtv.x = 0.f;
    }

    return mtv;
}

// 2D AABB collision detection (rect. vs. rect.)
neko_force_inline
b32 neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b)
{
    if (a->max.x > b->min.x &&
         a->max.y > b->min.y &&
         a->min.x < b->max.x &&
         a->min.y < b->max.y)
    {
        return true;
    }

    return false;
}

neko_force_inline
neko_vec4 neko_aabb_window_coords(neko_aabb_t* aabb, neko_camera_t* camera, neko_vec2 window_size)
{
    // AABB of the player
    neko_vec4 bounds = neko_default_val();
    neko_vec4 tl = neko_v4(aabb->min.x, aabb->min.y, 0.f, 1.f);
    neko_vec4 br = neko_v4(aabb->max.x, aabb->max.y, 0.f, 1.f);

    neko_mat4 view_mtx = neko_camera_get_view(camera);
    neko_mat4 proj_mtx = neko_camera_get_proj(camera, (s32)window_size.x, (s32)window_size.y);
    neko_mat4 vp = neko_mat4_mul(proj_mtx, view_mtx);

    // Transform verts
    tl = neko_mat4_mul_vec4(vp, tl);
    br = neko_mat4_mul_vec4(vp, br);

    // Perspective divide
    tl = neko_vec4_scale(tl, 1.f / tl.w);
    br = neko_vec4_scale(br, 1.f / br.w);

    // NDC [0.f, 1.f] and NDC
    tl.x = (tl.x * 0.5f + 0.5f);
    tl.y = (tl.y * 0.5f + 0.5f);
    br.x = (br.x * 0.5f + 0.5f);
    br.y = (br.y * 0.5f + 0.5f);

    // Window Space
    tl.x = tl.x * window_size.x;
    tl.y = neko_map_range(1.f, 0.f, 0.f, 1.f, tl.y) * window_size.y;
    br.x = br.x * window_size.x;
    br.y = neko_map_range(1.f, 0.f, 0.f, 1.f, br.y) * window_size.y;

    bounds = neko_v4(tl.x, tl.y, br.x, br.y);

    return bounds;
}
*/

/** @} */  // end of neko_math

/*========================
// NEKO_LEXER
========================*/

//==== [ Token ] ============================================================//

typedef enum neko_token_type {
    NEKO_TOKEN_UNKNOWN = 0x00,
    NEKO_TOKEN_LPAREN,
    NEKO_TOKEN_RPAREN,
    NEKO_TOKEN_LTHAN,
    NEKO_TOKEN_GTHAN,
    NEKO_TOKEN_SEMICOLON,
    NEKO_TOKEN_COLON,
    NEKO_TOKEN_COMMA,
    NEKO_TOKEN_EQUAL,
    NEKO_TOKEN_NOT,
    NEKO_TOKEN_HASH,
    NEKO_TOKEN_PIPE,
    NEKO_TOKEN_AMPERSAND,
    NEKO_TOKEN_LBRACE,
    NEKO_TOKEN_RBRACE,
    NEKO_TOKEN_LBRACKET,
    NEKO_TOKEN_RBRACKET,
    NEKO_TOKEN_MINUS,
    NEKO_TOKEN_PLUS,
    NEKO_TOKEN_ASTERISK,
    NEKO_TOKEN_BSLASH,
    NEKO_TOKEN_FSLASH,
    NEKO_TOKEN_QMARK,
    NEKO_TOKEN_SPACE,
    NEKO_TOKEN_PERCENT,
    NEKO_TOKEN_DOLLAR,
    NEKO_TOKEN_NEWLINE,
    NEKO_TOKEN_TAB,
    NEKO_TOKEN_UNDERSCORE,
    NEKO_TOKEN_SINGLE_LINE_COMMENT,
    NEKO_TOKEN_MULTI_LINE_COMMENT,
    NEKO_TOKEN_IDENTIFIER,
    NEKO_TOKEN_SINGLE_QUOTE,
    NEKO_TOKEN_DOUBLE_QUOTE,
    NEKO_TOKEN_STRING,
    NEKO_TOKEN_PERIOD,
    NEKO_TOKEN_NUMBER
} neko_token_type;

typedef struct neko_token_t {
    const char* text;
    neko_token_type type;
    u32 len;
} neko_token_t;

NEKO_API_DECL neko_token_t neko_token_invalid_token();
NEKO_API_DECL bool neko_token_compare_type(const neko_token_t* t, neko_token_type type);
NEKO_API_DECL bool neko_token_compare_text(const neko_token_t* t, const char* match);
NEKO_API_DECL void neko_token_print_text(const neko_token_t* t);
NEKO_API_DECL void neko_token_debug_print(const neko_token_t* t);
NEKO_API_DECL const char* neko_token_type_to_str(neko_token_type type);
NEKO_API_DECL bool neko_char_is_end_of_line(char c);
NEKO_API_DECL bool neko_char_is_white_space(char c);
NEKO_API_DECL bool neko_char_is_alpha(char c);
NEKO_API_DECL bool neko_char_is_numeric(char c);

NEKO_API_DECL neko_inline b8 neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
NEKO_API_DECL neko_inline b8 neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
NEKO_API_DECL neko_inline b8 neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
NEKO_API_DECL neko_inline b8 neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

//==== [ Lexer ] ============================================================//

typedef struct neko_lexer_t {
    const char* at;
    const char* contents;
    neko_token_t current_token;
    bool (*can_lex)(struct neko_lexer_t* lex);
    void (*eat_white_space)(struct neko_lexer_t* lex);
    neko_token_t (*next_token)(struct neko_lexer_t*);
    b32 skip_white_space;
    size_t size;           // Optional
    size_t contents_size;  // Optional
} neko_lexer_t;

NEKO_API_DECL void neko_lexer_set_contents(neko_lexer_t* lex, const char* contents);
NEKO_API_DECL neko_token_t neko_lexer_next_token(neko_lexer_t* lex);
NEKO_API_DECL bool neko_lexer_can_lex(neko_lexer_t* lex);
NEKO_API_DECL neko_token_t neko_lexer_current_token(const neko_lexer_t* lex);
NEKO_API_DECL bool neko_lexer_require_token_text(neko_lexer_t* lex, const char* match);
NEKO_API_DECL bool neko_lexer_require_token_type(neko_lexer_t* lex, neko_token_type type);
NEKO_API_DECL bool neko_lexer_current_token_compare_type(const neko_lexer_t* lex, neko_token_type type);
NEKO_API_DECL neko_token_t neko_lexer_peek(neko_lexer_t* lex);
NEKO_API_DECL bool neko_lexer_find_next_token_type(neko_lexer_t* lex, neko_token_type type);
NEKO_API_DECL neko_token_t neko_lexer_advance_before_next_token_type(neko_lexer_t* lex, neko_token_type type);

// C specific functions for lexing
NEKO_API_DECL neko_lexer_t neko_lexer_c_ctor(const char* contents);
NEKO_API_DECL bool neko_lexer_c_can_lex(neko_lexer_t* lex);
NEKO_API_DECL void neko_lexer_c_eat_white_space(neko_lexer_t* lex);
NEKO_API_DECL neko_token_t neko_lexer_c_next_token(neko_lexer_t* lex);
NEKO_API_DECL void neko_lexer_set_token(neko_lexer_t* lex, neko_token_t token);

#ifdef neko_cpp_src

template <typename T>
neko_inline void neko_swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
neko_static_inline void write_var(u8*& _buffer, T _var) {
    std::memcpy(_buffer, &_var, sizeof(T));
    _buffer += sizeof(T);
}

neko_static_inline void write_str(u8*& _buffer, const char* _str) {
    u32 len = (u32)strlen(_str);
    write_var(_buffer, len);
    std::memcpy(_buffer, _str, len);
    _buffer += len;
}

template <typename T>
neko_static_inline void read_var(u8*& _buffer, T& _var) {
    std::memcpy(&_var, _buffer, sizeof(T));
    _buffer += sizeof(T);
}

neko_static_inline char* read_string(u8*& _buffer) {
    u32 len;
    read_var(_buffer, len);
    char* str = new char[len + 1];
    std::memcpy(str, _buffer, len);
    str[len] = 0;
    _buffer += len;
    return str;
}

neko_static_inline const char* duplicate_string(const char* _str) {
    char* str = new char[strlen(_str) + 1];
    std::strcpy(str, _str);
    return str;
}

struct string_store {
    typedef std::unordered_map<std::string, u32> string_to_index_type;
    typedef std::unordered_map<u32, std::string> index_to_string_type;

    u32 total_size;
    string_to_index_type str_index_map;
    index_to_string_type strings;

    string_store() : total_size(0) {}

    void add_string(const char* _str) {
        string_to_index_type::iterator it = str_index_map.find(_str);
        if (it == str_index_map.end()) {
            u32 index = (u32)str_index_map.size();
            total_size += 4 + (u32)std::strlen(_str);
            str_index_map[_str] = index;
            strings[index] = _str;
        }
    }

    u32 get_string(const char* _str) { return str_index_map[_str]; }
};

template <typename T>
struct neko_span {
    neko_span() : __begin(nullptr), __end(nullptr) {}
    neko_span(T* begin, u32 len) : __begin(begin), __end(begin + len) {}
    neko_span(T* begin, T* end) : __begin(begin), __end(end) {}
    template <int N>
    neko_span(T (&value)[N]) : __begin(value), __end(value + N) {}
    T& operator[](u32 idx) const {
        neko_assert(__begin + idx < __end);
        return __begin[idx];
    }
    operator neko_span<const T>() const { return neko_span<const T>(__begin, __end); }
    void remove_prefix(u32 count) {
        neko_assert(count <= length());
        __begin += count;
    }
    void remove_suffix(u32 count) {
        neko_assert(count <= length());
        __end -= count;
    }
    [[nodiscard]] neko_span from_left(u32 count) const {
        neko_assert(count <= length());
        return neko_span(__begin + count, __end);
    }
    [[nodiscard]] neko_span from_right(u32 count) const {
        neko_assert(count <= length());
        return neko_span(__begin, __end - count);
    }
    T& back() {
        neko_assert(length() > 0);
        return *(__end - 1);
    }
    const T& back() const {
        neko_assert(length() > 0);
        return *(__end - 1);
    }
    bool equals(const neko_span<T>& rhs) {
        bool res = true;
        if (length() != rhs.length()) return false;
        for (const T& v : *this) {
            u32 i = u32(&v - __begin);
            if (v != rhs.__begin[i]) return false;
        }
        return true;
    }

    template <typename F>
    s32 find(const F& f) const {
        for (u32 i = 0, c = length(); i < c; ++i) {
            if (f(__begin[i])) return i;
        }
        return -1;
    }

    u32 length() const { return (u32)(__end - __begin); }

    T* begin() const { return __begin; }
    T* end() const { return __end; }

    T* __begin;
    T* __end;
};

#endif

#endif  // NEKO_H