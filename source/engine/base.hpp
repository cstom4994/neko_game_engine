
#ifndef NEKO_BASE_H
#define NEKO_BASE_H

#include "base/common/base.hpp"
#include "base/common/string.hpp"
#include "base/common/math.hpp"
#include "base/common/arena.hpp"
#include "base/common/array.hpp"
#include "base/common/hashmap.hpp"
#include "base/common/mem.hpp"
#include "base/common/mutex.hpp"
#include "base/common/queue.hpp"
#include "base/common/util.hpp"

using namespace Neko;

FORMAT_ARGS(1)
inline void neko_panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(1);
}

NEKO_FORCE_INLINE void neko_util_str_to_lower(const char *src, char *buffer, size_t buffer_sz) {
    size_t src_sz = neko_strlen(src);
    size_t len = NEKO_MIN(src_sz, buffer_sz - 1);

    for (u32 i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
    if (len) buffer[len] = '\0';
}

NEKO_FORCE_INLINE bool neko_util_str_is_numeric(const char *str) {
    const char *at = str;
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

NEKO_FORCE_INLINE void neko_util_get_dir_from_file(char *buffer, u32 buffer_size, const char *file_path) {
    u32 str_len = neko_strlen(file_path);
    const char *end = (file_path + str_len);
    for (u32 i = 0; i < str_len; ++i) {
        if (file_path[i] == '/' || file_path[i] == '\\') {
            end = &file_path[i];
        }
    }

    size_t dir_len = end - file_path;
    memcpy(buffer, file_path, NEKO_MIN(buffer_size, dir_len + 1));
    if (dir_len + 1 <= buffer_size) {
        buffer[dir_len] = '\0';
    }
}

NEKO_FORCE_INLINE void neko_util_string_substring(const char *src, char *dst, size_t sz, u32 start, u32 end) {
    u32 str_len = neko_strlen(src);
    if (end > str_len) {
        end = str_len;
    }
    if (start > str_len) {
        start = str_len;
    }

    const char *at = src + start;
    const char *e = src + end;
    u32 ct = 0;
    while (at && *at != '\0' && at != e) {
        dst[ct] = *at;
        at++;
        ct++;
    }
}

NEKO_FORCE_INLINE void neko_util_string_remove_character(const char *src, char *buffer, u32 buffer_size, char delimiter) {
    u32 ct = 0;
    u32 str_len = neko_strlen(src);
    const char *at = src;
    while (at && *at != '\0' && ct < buffer_size) {
        char c = *at;
        if (c != delimiter) {
            buffer[ct] = c;
            ct++;
        }
        at++;
    }
}

NEKO_FORCE_INLINE void neko_util_string_replace(char *buffer, size_t buffer_sz, const char *replace, char fallback) {
    // Replace all characters with characters of keyword, then the rest replace with spaces
    size_t len = neko_strlen(replace);
    for (u32 c = 0; c < buffer_sz; ++c) {
        if (c < len) {
            buffer[c] = replace[c];
        } else {
            buffer[c] = fallback;
        }
    }
}

NEKO_FORCE_INLINE void neko_util_string_replace_delim(const char *source_str, char *buffer, u32 buffer_size, char delimiter, char replace) {
    u32 str_len = neko_strlen(source_str);
    const char *at = source_str;
    while (at && *at != '\0') {
        char c = *at;
        if (c == delimiter) {
            c = replace;
        }
        buffer[(at - source_str)] = c;
        at++;
    }
}

NEKO_FORCE_INLINE u32 neko_hash_u32(u32 x) {
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

NEKO_FORCE_INLINE u32 neko_hash_u64(u64 x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (u32)x;
}

// Note: source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
NEKO_FORCE_INLINE u32 neko_hash_str(const char *str) {
    u32 hash = 5381;
    i32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

NEKO_FORCE_INLINE u64 neko_hash_str64(const char *str) {
    u32 hash1 = 5381;
    u32 hash2 = 52711;
    u32 i = neko_strlen(str);
    while (i--) {
        char c = str[i];
        hash1 = (hash1 * 33) ^ c;
        hash2 = (hash2 * 33) ^ c;
    }

    return (hash1 >> 0) * 4096 + (hash2 >> 0);
}

NEKO_FORCE_INLINE bool neko_compare_bytes(void *b0, void *b1, size_t len) { return 0 == memcmp(b0, b1, len); }

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define NEKO_SIZE_T_BITS ((sizeof(size_t)) * 8)
#define NEKO_SIPHASH_C_ROUNDS 1
#define NEKO_SIPHASH_D_ROUNDS 1
#define neko_rotate_left(__V, __N) (((__V) << (__N)) | ((__V) >> (NEKO_SIZE_T_BITS - (__N))))
#define neko_rotate_right(__V, __N) (((__V) >> (__N)) | ((__V) << (NEKO_SIZE_T_BITS - (__N))))

NEKO_FORCE_INLINE size_t neko_hash_siphash_bytes(void *p, size_t len, size_t seed) {
    unsigned char *d = (unsigned char *)p;
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

NEKO_FORCE_INLINE size_t neko_hash_bytes(void *p, size_t len, size_t seed) {
#if 0
  return neko_hash_siphash_bytes(p,len,seed);
#else
    unsigned char *d = (unsigned char *)p;

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

#define NEKO_VA_COUNT(...) detail::va_count(__VA_ARGS__)

#define NEKO_DYNAMIC_CAST(type, input_var, cast_var_name)  \
    neko_assert(value);                                    \
    type *cast_var_name = dynamic_cast<type *>(input_var); \
    neko_assert(cast_var_name)
#define NEKO_STATIC_CAST(type, input_var, cast_var_name)  \
    neko_assert(value);                                   \
    type *cast_var_name = static_cast<type *>(input_var); \
    neko_assert(cast_var_name)

#define NEKO_ENUM_FLAG(T)                                                                                                                                                    \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                       \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T &operator|=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T &operator&=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T &operator^=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define NEKO_MOVEONLY(class_name)                       \
    class_name(const class_name &) = delete;            \
    class_name &operator=(const class_name &) = delete; \
    class_name(class_name &&) = default;                \
    class_name &operator=(class_name &&) = default

namespace detail {
template <typename... Args>
constexpr std::size_t va_count(Args &&...) {
    return sizeof...(Args);
}
}  // namespace detail

#define neko_macro_overload(fun, a, ...)                                               \
    do {                                                                               \
        if (const bool a_ = (a); a_)                                                   \
            [&](auto &&...args) {                                                      \
                const auto t = std::make_tuple(std::forward<decltype(args)>(args)...); \
                constexpr auto N = std::tuple_size<decltype(t)>::value;                \
                                                                                       \
                if constexpr (N == 0) {                                                \
                    fun(a_);                                                           \
                } else {                                                               \
                    fun(a_, __VA_ARGS__);                                              \
                }                                                                      \
            }(__VA_ARGS__);                                                            \
    } while (0)

#define neko_check_is_trivial(type, ...) static_assert(std::is_trivial<type>::value, __VA_ARGS__)

// 一种向任何指针添加字节偏移量的可移植且安全的方法
// https://stackoverflow.com/questions/15934111/portable-and-safe-way-to-add-byte-offset-to-any-pointer
template <typename T>
inline void neko_addoffset(std::ptrdiff_t offset, T *&ptr) {
    if (!ptr) return;
    ptr = (T *)((unsigned char *)ptr + offset);
}

template <typename T>
inline T *neko_addoffset_r(std::ptrdiff_t offset, T *ptr) {
    if (!ptr) return nullptr;
    return (T *)((unsigned char *)ptr + offset);
}

struct lua_State;

namespace Neko {

template <typename V, typename Alloc = std::allocator<V>>
using vector = std::vector<V, Alloc>;

template <typename T>
struct cpp_remove_reference {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T &> {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T &&> {
    using type = T;
};

template <typename T>
constexpr typename cpp_remove_reference<T>::type &&cpp_move(T &&arg) noexcept {
    return (typename cpp_remove_reference<T>::type &&)arg;
}

}  // namespace Neko

#define NEKO_VA_UNPACK(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

namespace Neko {}  // namespace Neko

namespace Neko {

// hash 计算相关函数

typedef uint64_t hash_value;

static_assert(sizeof(hash_value) == 8);

inline uint64_t hash_fnv(const void *data, int size) {
    const_str s = (const_str)data;
    uint64_t h = 14695981039346656037ULL;
    char c = 0;
    while (size--) {
        h = h ^ (uint64_t)(*s++);
        h = h * 1099511628211ULL;
    }
    return h;
}

// http://www.jstatsoft.org/article/view/v008i14/xorshift.pdf page 4
// https://en.wikipedia.org/wiki/Xorshift#xorshift
constexpr hash_value xor64(hash_value h) {
    h ^= 88172645463325252ULL;  // 与常数进行异或 因此种子 0 不会导致无限循环
    h ^= h >> 12;
    h ^= h << 25;
    h ^= h >> 27;
    return h * 0x2545F4914F6CDD1DULL;
}

// https://de.wikipedia.org/wiki/FNV_(Informatik)
constexpr hash_value hash_fnv(const_str string) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; *string; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr hash_value hash(char const *input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string &input) { return hash_fnv(input.c_str()); }

template <typename ForwardIterator, typename SpaceDetector>
constexpr ForwardIterator find_terminating_word(ForwardIterator begin, ForwardIterator end, SpaceDetector &&is_space_pred) {
    auto rend = std::reverse_iterator(begin);
    auto rbegin = std::reverse_iterator(end);

    int sp_size = 0;
    auto is_space = [&sp_size, &is_space_pred, &end](char c) {
        sp_size = is_space_pred(std::string_view{&c, static_cast<unsigned>(&*std::prev(end) - &c)});
        return sp_size > 0;
    };

    auto search = std::find_if(rbegin, rend, is_space);
    if (search == rend) {
        return begin;
    }
    ForwardIterator it = std::prev(search.base());
    it += sp_size;
    return it;
}

template <typename ForwardIt, typename OutputIt>
constexpr void copy(ForwardIt src_beg, ForwardIt src_end, OutputIt dest_beg, OutputIt dest_end) {
    while (src_beg != src_end && dest_beg != dest_end) {
        *dest_beg++ = *src_beg++;
    }
}

}  // namespace Neko

template <typename T>
static inline std::string readable_bytes(T num) {
    char buffer[64];

    if (num >= 1e15) {
        snprintf(buffer, sizeof(buffer), "%.2f PB", (float)num / 1e15);
    } else if (num >= 1e12) {
        snprintf(buffer, sizeof(buffer), "%.2f TB", (float)num / 1e12);
    } else if (num >= 1e9) {
        snprintf(buffer, sizeof(buffer), "%.2f GB", (float)num / 1e9);
    } else if (num >= 1e6) {
        snprintf(buffer, sizeof(buffer), "%.2f MB", (float)num / 1e6);
    } else if (num >= 1e3) {
        snprintf(buffer, sizeof(buffer), "%.2f kB", (float)num / 1e3);
    } else {
        snprintf(buffer, sizeof(buffer), "%.2f B", (float)num);
    }

    return std::string(buffer);
}

#endif
