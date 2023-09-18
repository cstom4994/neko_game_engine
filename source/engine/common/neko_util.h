#ifndef NEKO_UTIL_H
#define NEKO_UTIL_H

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <type_traits>

#include "engine/common/neko_mem.h"
#include "engine/common/neko_types.h"

// Helper macro for compiling to nothing
#define neko_empty_instruction(...)

#define neko_array_size(arr) sizeof(arr) / sizeof(arr[0])

#if defined(DEBUG) || defined(_DEBUG)
#define neko_is_debug() 1
#else
#define neko_is_debug() 0
#endif

#define neko_assert(x)                                                                                                 \
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
#define neko_for_range_i(count) for (u32 i = 0; i < count; ++i)

// Helper macro for an in place for-range loop
#define neko_for_range_j(count) for (u32 j = 0; j < count; ++j)

#define neko_for_range(iter_type, iter_name, iter_end) for (iter_type iter_name = 0; iter_name < iter_end; ++iter_name)

#define neko_max(a, b) ((a) > (b) ? (a) : (b))
#define neko_min(a, b) ((a) < (b) ? (a) : (b))
#define neko_clamp(v, min, max) ((v) > (max) ? (max) : (v) < (min) ? (min) : (v))

// Helpful macro for casting one type to another
#define neko_cast(a, b) ((a*)(b))

#define neko_r_cast reinterpret_cast
#define neko_s_cast static_cast
#define neko_c_cast const_cast

// Helpful marco for calculating offset (in bytes) of an element from a given structure type
#define neko_offset(type, element) ((usize)(&(((type*)(0))->element)))

// macro for turning any given type into a const char* of itself
#define neko_to_str(type) ((const_str) #type)

#define neko_print_enabled 0

#if neko_print_enabled
#define neko_measure_time(label, ...)                                \
    do {                                                             \
        u32 __st = neko_platform_ticks();                            \
        __VA_ARGS__                                                  \
        neko_println("%s: %d", label, neko_platform_ticks() - __st); \
    } while (0)
#else
#define neko_measure_time(label, ...) __VA_ARGS__
#endif

#define neko_timed_action(interval, ...) \
    do {                                 \
        static u32 __t = 0;              \
        if (__t++ > interval) {          \
            __t = 0;                     \
            __VA_ARGS__                  \
        }                                \
    } while (0)

#define neko_concat(x, y) neko_concat_impl(x, y)
#define neko_concat_impl(x, y) x##y

#define neko_invoke_once(...)                           \
    static char neko_concat(unused, __LINE__) = [&]() { \
        __VA_ARGS__;                                    \
        return '\0';                                    \
    }();                                                \
    (void)neko_concat(unused, __LINE__)

#define neko_bool_str(b) (bool(b) ? "true" : "false")

#define neko_enum_decl(NAME, ...) typedef enum class NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME

#define neko_enum_count(NAME) _neko_##NAME##_count

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

#define neko_aligned_buffer(name) alignas(16) static const std::uint8_t name[]

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

/*===================================
// Memory Allocation Utils
===================================*/

#ifndef neko_malloc
#define neko_malloc(sz) malloc(sz)
#endif

#ifndef neko_free
#define neko_free(mem) free(mem)
#endif

#ifndef neko_realloc
#define neko_realloc(mem, sz) realloc(mem, sz)
#endif

#ifndef neko_calloc
#define neko_calloc(num, sz) calloc(num, sz)
#endif

#ifndef neko_check_is_trivial
#define neko_check_is_trivial(type) static_assert(std::is_trivial<type>::value)
#endif

#define neko_malloc_init(type)                   \
    (type*)_neko_malloc_init_impl(sizeof(type)); \
    neko_check_is_trivial(type, "try to init a non-trivial object")

#define neko_malloc_init_ex(name, type)                              \
    neko_check_is_trivial(type, "try to init a non-trivial object"); \
    struct type* name = neko_malloc_init(type)

neko_force_inline void* _neko_malloc_init_impl(usize sz) {
    void* data = neko_safe_malloc(sz);
    memset(data, 0, sz);
    return data;
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

// Expects null terminated strings
neko_force_inline b8 neko_string_compare_equal(const char* txt, const char* cmp) {
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

neko_force_inline b8 neko_string_compare_equal_n(const char* txt, const char* cmp, u32 n) {
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

neko_force_inline void neko_util_str_to_lower(const char* src, char* buffer, usize buffer_sz) {
    u32 src_sz = neko_string_length(src);
    u32 len = neko_min(src_sz, buffer_sz);

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
        printf("\n%c", c);
        if (c >= '0' && c <= '9') {
            printf("not numeric\n");
            return false;
        }
    }

    return true;
}

// Will return a null buffer if file does not exist or allocation fails
neko_force_inline char* neko_read_file_contents_into_string_null_term(const char* file_path, const char* mode, usize* sz) {
    char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    if (fp) {
        fseek(fp, 0, SEEK_END);
        *sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = (char*)neko_safe_malloc(*sz + 1);
        if (buffer) {
            fread(buffer, 1, *sz, fp);
        }
        fclose(fp);
        buffer[*sz] = '0';
    }
    return buffer;
}

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
    while (*end != '/' && end != file_path) {
        end--;
    }
    memcpy(buffer, file_path, neko_min(buffer_size, (end - file_path) + 1));
}

neko_force_inline void neko_util_get_file_name(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* end = (file_path + str_len);
    const char* dot_at = end;
    while (*end != '.' && end != file_path) {
        end--;
    }
    const char* start = end;
    while (*start != '/' && start != file_path) {
        start--;
    }
    memcpy(buffer, start, (end - start));
}

neko_force_inline void neko_util_string_substring(const char* src, char* dst, usize sz, u32 start, u32 end) {
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

neko_force_inline void neko_util_string_replace(const char* source_str, char* buffer, u32 buffer_size, char delimiter, char replace) {
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

#ifdef __MINGW32__
#define neko_printf(fmt, ...) __mingw_printf(fmt, ##__VA_ARGS__)
#else
neko_force_inline void neko_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#endif

#define neko_println(fmt, ...)           \
    do {                                 \
        neko_printf(fmt, ##__VA_ARGS__); \
        neko_printf("\n");               \
    } while (0)

neko_force_inline void neko_fprintf(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
}

neko_force_inline void neko_fprintln(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    neko_fprintf(fp, "\n");
}

// neko_force_inline char*
// neko_sprintf
// (
//  const char* fmt,
//  ...
//)
// {
//  va_list args;
//  va_start(args, fmt);
//  usize sz = vsnprintf(NULL, 0, fmt, args);
//  char* ret = malloc(sz + 1);
//  vsnprintf(ret, sz, fmt, args);
//  va_end(args);
//  return ret;
// }

#ifdef __MINGW32__
#define neko_snprintf(name, size, fmt, ...) __mingw_snprintf(name, size, fmt, __VA_ARGS__)
#else
neko_force_inline void neko_snprintf(char* buffer, usize buffer_size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
}
#endif

#define neko_snprintfc(name, size, fmt, ...) \
    char name[size] = neko_default_val();    \
    neko_snprintf(name, size, fmt, __VA_ARGS__);

neko_force_inline bool neko_compare_bytes(void* b0, void* b1, size_t len) { return 0 == memcmp(b0, b1, len); }

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

    int32_t nOffSet = fakeDate->tm_hour - unixdate->tm_hour;
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
        duration = static_cast<double>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] neko_inline double get() const noexcept { return duration; }
    ~neko_timer() noexcept { stop(); }

private:
    double duration = 0;
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

neko_inline const char* neko_fs_get_filename(const char* path) {
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

// xorshf32随机数算法
// http://www.iro.umontreal.ca/~lecuyer/myftp/papers/xorshift.pdf
// xyzw -> [0, 2^32 - 1]

neko_static_inline u32 __neko_rand_xorshf32_x = time(nullptr), __neko_rand_xorshf32_y = time(nullptr), __neko_rand_xorshf32_z = time(nullptr), __neko_rand_xorshf32_w = time(nullptr);

neko_static_inline u32 neko_rand_xorshf32(void) {
    // period 2^128 - 1
    u32 tmp = (__neko_rand_xorshf32_x ^ (__neko_rand_xorshf32_x << 15));
    __neko_rand_xorshf32_x = __neko_rand_xorshf32_y;
    __neko_rand_xorshf32_y = __neko_rand_xorshf32_z;
    __neko_rand_xorshf32_z = __neko_rand_xorshf32_w;

    __neko_rand_xorshf32_w = (__neko_rand_xorshf32_w ^ (__neko_rand_xorshf32_w >> 21) ^ (tmp ^ (tmp >> 4)));
    return __neko_rand_xorshf32_w;
}

#define neko_rand_xorshf32_max 0xFFFFFFFF

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

#endif  // NEKO_UTIL_H
