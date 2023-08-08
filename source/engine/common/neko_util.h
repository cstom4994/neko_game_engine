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
#define neko_debug_build 1
#endif

#define neko_assert(x)                                                                                                 \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            neko_printf("assertion failed: (%s), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__); \
            abort();                                                                                                   \
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
#define neko_to_str(type) ((const char*)#type)

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

#define neko_concat(x, y) x##y

#define neko_invoke_once(...)                           \
    static char neko_concat(unused, __LINE__) = [&]() { \
        __VA_ARGS__;                                    \
        return '\0';                                    \
    }();                                                \
    (void)neko_concat(unused, __LINE__)

#define neko_bool_str(b) (bool(b) ? "true" : "false")

#define neko_enum_decl(NAME, ...) typedef enum NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME;

#define neko_enum_count(NAME) _neko_##NAME##_count

#define neko_move_only(class_name)                     \
    class_name(const class_name&) = delete;            \
    class_name& operator=(const class_name&) = delete; \
    class_name(class_name&&) = default;                \
    class_name& operator=(class_name&&) = default

#define neko_aligned_buffer(name) alignas(16) static const std::uint8_t name[]

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

#define neko_malloc_init(type) (type*)_neko_malloc_init_impl(sizeof(type))

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

neko_force_inline u32 neko_hash_u32(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

#define neko_hash_u32_ip(x, out)               \
    do {                                       \
        out = ((x >> 16) ^ x) * 0x45d9f3b;     \
        out = ((out >> 16) ^ out) * 0x45d9f3b; \
        out = (out >> 16) ^ out;               \
    } while (0)

neko_force_inline u32 neko_hash_u64(u64 x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (u32)x;
}

// source: http://www.cse.yorku.ca/~oz/hash.html
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

// 哈希映射容器

enum neko_hashmap_kind : u8 {
    neko_hashmap_kind_none,
    neko_hashmap_kind_some,
    neko_hashmap_kind_tombstone,
};

#define HASH_MAP_LOAD_FACTOR 0.75f

neko_inline u64 neko_hashmap_reserve_size(u64 size) {
    u64 n = (u64)(size / HASH_MAP_LOAD_FACTOR) + 1;

    // next pow of 2
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;

    return n;
}

template <typename T>
struct neko_hashmap {
    u64* keys = nullptr;
    T* values = nullptr;
    neko_hashmap_kind* kinds = nullptr;
    u64 load = 0;
    u64 capacity = 0;
    T& operator[](u64 key);
};

template <typename T>
void neko_hashmap_dctor(neko_hashmap<T>* map) {
    neko_safe_free(map->keys);
    neko_safe_free(map->values);
    neko_safe_free(map->kinds);
}

template <typename T>
u64 neko_hashmap_find_entry(neko_hashmap<T>* map, u64 key) {
    u64 index = key & (map->capacity - 1);
    u64 tombstone = (u64)-1;
    while (true) {
        neko_hashmap_kind kind = map->kinds[index];
        if (kind == neko_hashmap_kind_none) {
            return tombstone != (u64)-1 ? tombstone : index;
        } else if (kind == neko_hashmap_kind_tombstone) {
            tombstone = index;
        } else if (map->keys[index] == key) {
            return index;
        }

        index = (index + 1) & (map->capacity - 1);
    }
}

// capacity must be a power of 2
template <typename T>
void neko_hashmap_reserve(neko_hashmap<T>* old, u64 capacity) {
    if (capacity <= old->capacity) {
        return;
    }

    neko_hashmap<T> map = {};
    map.capacity = capacity;

    size_t bytes = sizeof(u64) * capacity;
    map.keys = (u64*)neko_safe_malloc(bytes);
    memset(map.keys, 0, bytes);

    map.values = (T*)neko_safe_malloc(sizeof(T) * capacity);
    memset(map.values, 0, sizeof(T) * capacity);

    map.kinds = (neko_hashmap_kind*)neko_safe_malloc(sizeof(neko_hashmap_kind) * capacity);
    memset(map.kinds, 0, sizeof(neko_hashmap_kind) * capacity);

    for (u64 i = 0; i < old->capacity; i++) {
        neko_hashmap_kind kind = old->kinds[i];
        if (kind != neko_hashmap_kind_some) {
            continue;
        }

        u64 index = neko_hashmap_find_entry(&map, old->keys[i]);
        map.keys[index] = old->keys[i];
        map.values[index] = old->values[i];
        map.kinds[index] = neko_hashmap_kind_some;
        map.load++;
    }

    neko_safe_free(old->keys);
    neko_safe_free(old->values);
    neko_safe_free(old->kinds);
    *old = map;
}

template <typename T>
T* neko_hashmap_get(neko_hashmap<T>* map, u64 key) {
    if (map->load == 0) {
        return nullptr;
    }

    u64 index = neko_hashmap_find_entry(map, key);
    return map->kinds[index] == neko_hashmap_kind_some ? &map->values[index] : nullptr;
}

template <typename T>
bool neko_hashmap_get(neko_hashmap<T>* map, u64 key, T** value) {
    if (map->load >= map->capacity * HASH_MAP_LOAD_FACTOR) {
        neko_hashmap_reserve(map, map->capacity > 0 ? map->capacity * 2 : 16);
    }

    u64 index = neko_hashmap_find_entry(map, key);
    bool exists = map->kinds[index] == neko_hashmap_kind_some;
    if (map->kinds[index] == neko_hashmap_kind_none) {
        map->load++;
        map->keys[index] = key;
        map->values[index] = {};
        map->kinds[index] = neko_hashmap_kind_some;
    }

    *value = &map->values[index];
    return exists;
}

template <typename T>
T& neko_hashmap<T>::operator[](u64 key) {
    T* value;
    neko_hashmap_get(this, key, &value);
    return *value;
}

template <typename T>
void neko_hashmap_unset(neko_hashmap<T>* map, u64 key) {
    if (map->load == 0) {
        return;
    }

    u64 index = neko_hashmap_find_entry(map, key);
    if (map->kinds[index] != neko_hashmap_kind_none) {
        map->kinds[index] = neko_hashmap_kind_tombstone;
    }
}

template <typename T>
void clear(neko_hashmap<T>* map) {
    memset(map->keys, 0, sizeof(u64) * map->capacity);
    memset(map->values, 0, sizeof(T) * map->capacity);
    memset(map->kinds, 0, sizeof(neko_hashmap_kind) * map->capacity);
    map->load = 0;
}

template <typename T>
struct neko_hashmap_kv {
    u64 key;
    T* value;
};

template <typename T>
struct neko_hashmap_iterator {
    neko_hashmap<T>* map;
    u64 cursor;

    neko_hashmap_kv<T> operator*() const {
        neko_hashmap_kv<T> kv;
        kv.key = map->keys[cursor];
        kv.value = &map->values[cursor];
        return kv;
    }

    neko_hashmap_iterator& operator++() {
        cursor++;
        while (cursor != map->capacity) {
            if (map->kinds[cursor] == neko_hashmap_kind_some) {
                return *this;
            }
            cursor++;
        }

        return *this;
    }
};

template <typename T>
bool operator!=(neko_hashmap_iterator<T> lhs, neko_hashmap_iterator<T> rhs) {
    return lhs.map != rhs.map || lhs.cursor != rhs.cursor;
}

template <typename T>
neko_hashmap_iterator<T> begin(neko_hashmap<T>& map) {
    neko_hashmap_iterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;

    for (u64 i = 0; i < map.capacity; i++) {
        if (map.kinds[i] == neko_hashmap_kind_some) {
            it.cursor = i;
            break;
        }
    }

    return it;
}

template <typename T>
neko_hashmap_iterator<T> end(neko_hashmap<T>& map) {
    neko_hashmap_iterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;
    return it;
}

// template <typename T>
// neko_hashmap_iterator<T> begin(const neko_hashmap<T>& map) {
//     neko_hashmap_iterator<T> it = {};
//     it.map = &map;
//     it.cursor = map.capacity;
//
//     for (u64 i = 0; i < map.capacity; i++) {
//         if (map.kinds[i] == neko_hashmap_kind_some) {
//             it.cursor = i;
//             break;
//         }
//     }
//
//     return it;
// }
//
// template <typename T>
// neko_hashmap_iterator<T> end(const neko_hashmap<T>& map) {
//     neko_hashmap_iterator<T> it = {};
//     it.map = &map;
//     it.cursor = map.capacity;
//     return it;
// }

#endif  // NEKO_UTIL_H
