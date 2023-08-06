// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef NEKO_HASH_HPP
#define NEKO_HASH_HPP

#include <intrin.h>

#include <string>
#include <vector>

namespace neko {

typedef unsigned long long hash_value;

static_assert(sizeof(hash_value) == 8 && sizeof(hash_value) == sizeof(size_t));

inline uint64_t hash_fnv(const void* data, int size) {
    const char* s = (const char*)data;
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
constexpr hash_value hash_fnv(const char* string) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; *string; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr hash_value hash_fnv(const char* string, const char* end) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; string != end; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr void next(hash_value& h) { h = xor64(h); }

inline hash_value hash_from_clock() {
    hash_value h = __rdtsc();
    next(h);
    return h;
}

template <typename T>
T next_range(hash_value& h, T min, T max) {
    if (max < min) return next_range(h, max, min);
    next(h);
    min += h % (max - min);
    return min;
}

template <typename T1, typename T2>
T1 next_range(hash_value& h, T1 min, T2 max) {
    return next_range<T1>(h, min, (T1)max);
}

inline float nextf(hash_value& h) {
    next(h);

    union {
        hash_value u_x;
        float u_f;
    };

    // 以某种方式操作浮点数的指数和分数 使得从 1（包括）和 2（不包括）中得到一个数字
    u_x = h;
    u_x &= 0x007FFFFF007FFFFF;
    u_x |= 0x3F8000003F800000;

    return u_f - 1.0f;
}

inline float nextf(hash_value& h, float min, float max) {
    if (max < min) return nextf(h, max, min);
    return min + nextf(h) * (max - min);
}

inline int nexti(hash_value& h) {
    next(h);

    union {
        hash_value u_x;
        int u_i;
    };

    u_x = h;

    return u_i;
}

template <class T>
inline hash_value hash_simple(T value) {
    static_assert(sizeof(T) <= sizeof(hash_value), "sizeof(T) can't be bigger than sizeof(hash_value)");
    union {
        hash_value u_h;
        T u_f;
    };
    u_h = 0;
    u_f = value;
    return u_h;
}

constexpr hash_value hash(unsigned char v) { return v; }
constexpr hash_value hash(unsigned int v) { return v; }
constexpr hash_value hash(unsigned long int v) { return v; }
constexpr hash_value hash(unsigned long long int v) { return v; }
constexpr hash_value hash(unsigned short int v) { return v; }
constexpr hash_value hash(bool v) { return v ? 1 : 0; }
inline hash_value hash(signed char v) { return hash_simple(v); }
inline hash_value hash(int v) { return hash_simple(v); }
inline hash_value hash(long int v) { return hash_simple(v); }
inline hash_value hash(long long int v) { return hash_simple(v); }
inline hash_value hash(short int v) { return hash_simple(v); }
inline hash_value hash(double v) { return hash_simple(v); }
inline hash_value hash(float v) { return hash_simple(v); }
inline hash_value hash(long double v) { return hash_simple(v); }
inline hash_value hash(wchar_t v) { return hash_simple(v); }
constexpr hash_value hash(char const* input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string& input) { return hash_fnv(input.c_str()); }

template <class T>
constexpr hash_value hash(const std::vector<T>& v) {
    hash_value h = 0;
    for (auto& o : v) {
        h ^= hash(o);
        h = xor64(h);
    }
    return h;
}

}  // namespace neko

#endif
