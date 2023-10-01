

#ifndef NEKO_HASH_H
#define NEKO_HASH_H

#include <intrin.h>

#include <string>
#include <vector>

#include "engine/neko.h"

// hash 计算相关函数

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
    T& operator[](const neko_string& key);
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
T& neko_hashmap<T>::operator[](const neko_string& key) {
    T* value;
    neko_hashmap_get(this, neko::hash(key.c_str()), &value);
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

// 基本泛型容器

template <typename T>
struct neko_array {
    T* data = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    T& operator[](size_t i) {
        neko_assert(i >= 0 && i < len);
        return data[i];
    }
};

template <typename T>
void neko_array_dctor(neko_array<T>* arr) {
    neko_safe_free(arr->data);
}

template <typename T>
void neko_array_reserve(neko_array<T>* arr, u64 capacity) {
    if (capacity > arr->capacity) {
        size_t bytes = sizeof(T) * capacity;
        T* buf = (T*)neko_safe_malloc(bytes);
        memcpy(buf, arr->data, sizeof(T) * arr->len);
        neko_safe_free(arr->data);
        arr->data = buf;
        arr->capacity = capacity;
    }
}

template <typename T>
void neko_array_resize(neko_array<T>* arr, u64 len) {
    neko_array_reserve(arr, len);
    arr->len = len;
}

template <typename T>
void neko_array_push(neko_array<T>* arr, T item) {
    if (arr->len == arr->capacity) {
        neko_array_reserve(arr, arr->capacity * 2 + 8);
    }
    arr->data[arr->len++] = item;
}

template <typename T>
void neko_array_pop(neko_array<T>* arr) {
    neko_assert(arr->len != 0);
    // 直接标记长度简短 优化方法
    arr->len--;
}

template <typename T>
T* begin(neko_array<T>& arr) {
    return arr.data;
}
template <typename T>
T* end(neko_array<T>& arr) {
    return &arr.data[arr.len];
}

#endif