#pragma once

#include "neko_os.h"
#include "neko_prelude.h"

struct Color {
    union {
        u8 rgba[4];
        struct {
            u8 r, g, b, a;
        };
    };
};

struct SplitLinesIterator {
    String data;
    String view;

    String operator*() const { return view; }
    SplitLinesIterator& operator++();
};

bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs);

struct SplitLines {
    String str;
    SplitLines(String s) : str(s) {}

    SplitLinesIterator begin();
    SplitLinesIterator end();
};

i32 utf8_size(u8 c);

struct Rune {
    u32 value;

    u32 charcode();
    bool is_whitespace();
    bool is_digit();
};

Rune rune_from_string(const char* buf);

struct UTF8Iterator {
    String str;
    u64 cursor;
    Rune rune;

    Rune operator*() const { return rune; }
    UTF8Iterator& operator++();
};

bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs);

struct UTF8 {
    String str;
    UTF8(String s) : str(s) {}

    UTF8Iterator begin();
    UTF8Iterator end();
};

struct StringBuilder {
    char* data;
    u64 len;       // does not include null term
    u64 capacity;  // includes null term

    StringBuilder();

    void trash();
    void reserve(u64 capacity);
    void clear();
    void swap_filename(String filepath, String file);
    void concat(String str, i32 times);

    StringBuilder& operator<<(String str);
    explicit operator String();
};

FORMAT_ARGS(1) String str_fmt(const char* fmt, ...);
FORMAT_ARGS(1) String tmp_fmt(const char* fmt, ...);

double string_to_double(String str);

enum HashMapKind : u8 {
    HashMapKind_None,
    HashMapKind_Some,
    HashMapKind_Tombstone,
};

#define HASH_MAP_LOAD_FACTOR 0.75f

template <typename T>
struct HashMap {
    u64* keys = nullptr;
    T* values = nullptr;
    HashMapKind* kinds = nullptr;
    u64 load = 0;
    u64 capacity = 0;

    void trash() {
        mem_free(keys);
        mem_free(values);
        mem_free(kinds);
    }

    u64 find_entry(u64 key) const {
        u64 index = key & (capacity - 1);
        u64 tombstone = (u64)-1;
        while (true) {
            HashMapKind kind = kinds[index];
            if (kind == HashMapKind_None) {
                return tombstone != (u64)-1 ? tombstone : index;
            } else if (kind == HashMapKind_Tombstone) {
                tombstone = index;
            } else if (keys[index] == key) {
                return index;
            }

            index = (index + 1) & (capacity - 1);
        }
    }

    void real_reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        HashMap<T> map = {};
        map.capacity = cap;

        size_t bytes = sizeof(u64) * cap;
        map.keys = (u64*)mem_alloc(bytes);
        memset(map.keys, 0, bytes);

        map.values = (T*)mem_alloc(sizeof(T) * cap);
        memset(map.values, 0, sizeof(T) * cap);

        map.kinds = (HashMapKind*)mem_alloc(sizeof(HashMapKind) * cap);
        memset(map.kinds, 0, sizeof(HashMapKind) * cap);

        for (u64 i = 0; i < capacity; i++) {
            HashMapKind kind = kinds[i];
            if (kind != HashMapKind_Some) {
                continue;
            }

            u64 index = map.find_entry(keys[i]);
            map.keys[index] = keys[i];
            map.values[index] = values[i];
            map.kinds[index] = HashMapKind_Some;
            map.load++;
        }

        mem_free(keys);
        mem_free(values);
        mem_free(kinds);
        *this = map;
    }

    void reserve(u64 capacity) {
        u64 n = (u64)(capacity / HASH_MAP_LOAD_FACTOR) + 1;

        // next pow of 2
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;

        real_reserve(n);
    }

    T* get(u64 key) {
        if (load == 0) {
            return nullptr;
        }
        u64 index = find_entry(key);
        return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
    }

    const T* get(u64 key) const {
        if (load == 0) {
            return nullptr;
        }
        u64 index = find_entry(key);
        return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
    }

    bool find_or_insert(u64 key, T** value) {
        if (load >= capacity * HASH_MAP_LOAD_FACTOR) {
            real_reserve(capacity > 0 ? capacity * 2 : 16);
        }

        u64 index = find_entry(key);
        bool exists = kinds[index] == HashMapKind_Some;
        if (!exists) {
            values[index] = {};
        }

        if (kinds[index] == HashMapKind_None) {
            load++;
            keys[index] = key;
            kinds[index] = HashMapKind_Some;
        }

        *value = &values[index];
        return exists;
    }

    T& operator[](u64 key) {
        T* value;
        find_or_insert(key, &value);
        return *value;
    }

    void unset(u64 key) {
        if (load == 0) {
            return;
        }

        u64 index = find_entry(key);
        if (kinds[index] != HashMapKind_None) {
            kinds[index] = HashMapKind_Tombstone;
        }
    }

    void clear() {
        memset(keys, 0, sizeof(u64) * capacity);
        memset(values, 0, sizeof(T) * capacity);
        memset(kinds, 0, sizeof(HashMapKind) * capacity);
        load = 0;
    }
};

template <typename T>
struct HashMapKV {
    u64 key;
    T* value;
};

template <typename T>
struct HashMapIterator {
    HashMap<T>* map;
    u64 cursor;

    HashMapKV<T> operator*() const {
        HashMapKV<T> kv;
        kv.key = map->keys[cursor];
        kv.value = &map->values[cursor];
        return kv;
    }

    HashMapIterator& operator++() {
        cursor++;
        while (cursor != map->capacity) {
            if (map->kinds[cursor] == HashMapKind_Some) {
                return *this;
            }
            cursor++;
        }

        return *this;
    }
};

template <typename T>
bool operator!=(HashMapIterator<T> lhs, HashMapIterator<T> rhs) {
    return lhs.map != rhs.map || lhs.cursor != rhs.cursor;
}

template <typename T>
HashMapIterator<T> begin(HashMap<T>& map) {
    HashMapIterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;

    for (u64 i = 0; i < map.capacity; i++) {
        if (map.kinds[i] == HashMapKind_Some) {
            it.cursor = i;
            break;
        }
    }

    return it;
}

template <typename T>
HashMapIterator<T> end(HashMap<T>& map) {
    HashMapIterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;
    return it;
}

template <typename T>
struct Array {
    T* data = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    T& operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void trash() { mem_free(data); }

    void reserve(u64 cap) {
        if (cap > capacity) {
            T* buf = (T*)mem_alloc(sizeof(T) * cap);
            memcpy(buf, data, sizeof(T) * len);
            mem_free(data);
            data = buf;
            capacity = cap;
        }
    }

    void resize(u64 n) {
        reserve(n);
        len = n;
    }

    void push(T item) {
        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }
        data[len] = item;
        len++;
    }

    T* begin() { return data; }
    T* end() { return &data[len]; }
};

template <typename T>
struct Queue {
    Mutex mtx = {};
    Cond cv = {};

    T* data = nullptr;
    u64 front = 0;
    u64 back = 0;
    u64 len = 0;
    u64 capacity = 0;

    void make() {
        mtx.make();
        cv.make();
    }

    void trash() {
        mtx.trash();
        cv.trash();
        mem_free(data);
    }

    void reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        T* buf = (T*)mem_alloc(sizeof(T) * cap);

        if (front < back) {
            memcpy(buf, &data[front], sizeof(T) * len);
        } else {
            u64 lhs = back;
            u64 rhs = (capacity - front);

            memcpy(buf, &data[front], sizeof(T) * rhs);
            memcpy(&buf[rhs], &data[0], sizeof(T) * lhs);
        }

        mem_free(data);

        data = buf;
        front = 0;
        back = len;
        capacity = cap;
    }

    void enqueue(T item) {
        LockGuard lock{&mtx};

        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }

        data[back] = item;
        back = (back + 1) % capacity;
        len++;

        cv.signal();
    }

    T demand() {
        LockGuard lock{&mtx};

        while (len == 0) {
            cv.wait(&mtx);
        }

        T item = data[front];
        front = (front + 1) % capacity;
        len--;

        return item;
    }
};

template <typename T>
struct PriorityQueue {
    T* data = nullptr;
    float* costs = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    void trash() {
        mem_free(data);
        mem_free(costs);
    }

    void reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        T* buf = (T*)mem_alloc(sizeof(T) * cap);
        memcpy(buf, data, sizeof(T) * len);
        mem_free(data);
        data = buf;

        float* cbuf = (float*)mem_alloc(sizeof(float) * cap);
        memcpy(cbuf, costs, sizeof(float) * len);
        mem_free(costs);
        costs = cbuf;

        capacity = cap;
    }

    void swap(i32 i, i32 j) {
        T t = data[i];
        data[i] = data[j];
        data[j] = t;

        float f = costs[i];
        costs[i] = costs[j];
        costs[j] = f;
    }

    void shift_up(i32 j) {
        while (j > 0) {
            i32 i = (j - 1) / 2;
            if (i == j || costs[i] < costs[j]) {
                break;
            }

            swap(i, j);
            j = i;
        }
    }

    void shift_down(i32 i, i32 n) {
        if (i < 0 || i > n) {
            return;
        }

        i32 j = 2 * i + 1;
        while (j >= 0 && j < n) {
            if (j + 1 < n && costs[j + 1] < costs[j]) {
                j = j + 1;
            }

            if (costs[i] < costs[j]) {
                break;
            }

            swap(i, j);
            i = j;
            j = 2 * i + 1;
        }
    }

    void push(T item, float cost) {
        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }

        data[len] = item;
        costs[len] = cost;
        len++;

        shift_up(len - 1);
    }

    bool pop(T* item) {
        if (len == 0) {
            return false;
        }

        *item = data[0];

        data[0] = data[len - 1];
        costs[0] = costs[len - 1];
        len--;

        shift_down(0, len);
        return true;
    }
};

struct ArenaNode;
struct Arena {
    ArenaNode* head;

    void trash();
    void* bump(u64 size);
    void* rebump(void* ptr, u64 old, u64 size);
    String bump_string(String s);
};

template <typename T>
struct Slice {
    T* data = nullptr;
    u64 len = 0;

    Slice() = default;
    explicit Slice(Array<T> arr) : data(arr.data), len(arr.len) {}

    T& operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    const T& operator[](size_t i) const {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void resize(u64 n) {
        T* buf = (T*)mem_alloc(sizeof(T) * n);
        memcpy(buf, data, sizeof(T) * len);
        mem_free(data);
        data = buf;
        len = n;
    }

    void resize(Arena* arena, u64 n) {
        T* buf = (T*)arena->rebump(data, sizeof(T) * len, sizeof(T) * n);
        data = buf;
        len = n;
    }

    T* begin() { return data; }
    T* end() { return &data[len]; }
    const T* begin() const { return data; }
    const T* end() const { return &data[len]; }
};

struct Scanner {
    char* data;
    u64 len;
    u64 pos;
    u64 end;

    Scanner(String str);
    String next_string();
    i32 next_int();
};

#if defined(_M_AMD64) || defined(__SSE__)
#ifndef SSE_AVAILABLE
#define SSE_AVAILABLE
#endif
#include <immintrin.h>
#endif

union Vector4 {
    struct {
        float x, y, z, w;
    };
    float arr[4];
#ifdef SSE_AVAILABLE
    __m128 sse;
#endif
};

inline Vector4 vec4(float x, float y, float z, float w) {
    Vector4 v = {};
#ifdef SSE_AVAILABLE
    v.sse = _mm_set_ps(w, z, y, x);
#else
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
#endif
    return v;
}

inline Vector4 vec4_xy(float x, float y) {
    Vector4 v = {};
#ifdef SSE_AVAILABLE
    v.sse = _mm_set_ps(1, 0, y, x);
#else
    v.x = x;
    v.y = y;
    v.w = 1;
#endif
    return v;
}

union Matrix4 {
    float cols[4][4];
    Vector4 vecs[4];
#ifdef SSE_AVAILABLE
    __m128 sse[4];
#endif
};

inline Vector4 vec4_mul_mat4(Vector4 v, Matrix4 m) {
#ifdef SSE_AVAILABLE
    __m128 mul0 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0x00), m.sse[0]);
    __m128 mul1 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0x55), m.sse[1]);
    __m128 mul2 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0xaa), m.sse[2]);
    __m128 mul3 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0xff), m.sse[3]);

    Vector4 out = {};
    out.sse = _mm_add_ps(_mm_add_ps(mul0, mul1), _mm_add_ps(mul2, mul3));
    return out;
#else
    Vector4 out = {};
    for (int i = 0; i < 4; i++) {
        out.x += v.arr[i] * m.cols[i][0];
        out.y += v.arr[i] * m.cols[i][1];
        out.z += v.arr[i] * m.cols[i][2];
        out.w += v.arr[i] * m.cols[i][3];
    }
    return out;
#endif
}

inline Matrix4 mat4_mul_mat4(Matrix4 lhs, Matrix4 rhs) {
    Matrix4 m = {};
    m.vecs[0] = vec4_mul_mat4(rhs.vecs[0], lhs);
    m.vecs[1] = vec4_mul_mat4(rhs.vecs[1], lhs);
    m.vecs[2] = vec4_mul_mat4(rhs.vecs[2], lhs);
    m.vecs[3] = vec4_mul_mat4(rhs.vecs[3], lhs);
    return m;
}

/*=============================
//
=============================*/

/*===================================
// String Utils
===================================*/

inline uint8_t wtf8_decode(const char* input, uint32_t* res) {
    uint8_t b1 = input[0];
    if (b1 <= 0x7F) {
        *res = b1;
        return 1;
    }
    if (b1 < 0xC2) {
        return 0;
    }
    uint32_t code_point = b1;
    uint8_t b2 = input[1];
    if ((b2 & 0xC0) != 0x80) {
        return 0;
    }
    code_point = (code_point << 6) | (b2 & 0x3F);
    if (b1 <= 0xDF) {
        *res = 0x7FF & code_point;
        return 2;
    }

    uint8_t b3 = input[2];
    if ((b3 & 0xC0) != 0x80) {
        return 0;
    }
    code_point = (code_point << 6) | (b3 & 0x3F);
    if (b1 <= 0xEF) {
        *res = 0xFFFF & code_point;
        return 3;
    }

    uint8_t b4 = input[3];
    if ((b4 & 0xC0) != 0x80) {
        return 0;
    }
    code_point = (code_point << 6) | (b4 & 0x3F);
    if (b1 <= 0xF4) {
        code_point &= 0x1FFFFF;
        if (code_point <= 0x10FFFF) {
            *res = code_point;
            return 4;
        }
    }
    return 0;
}

inline size_t wtf8_to_utf16_length(const char* input, size_t length) {
    size_t output_len = 0;
    uint32_t code_point;
    for (size_t i = 0; i < length;) {
        uint8_t n = wtf8_decode(&input[i], &code_point);
        if (n == 0) {
            return (size_t)-1;
        }
        if (code_point > 0xFFFF) {
            output_len += 2;
        } else {
            output_len += 1;
        }
        i += n;
    }
    return output_len;
}

inline void wtf8_to_utf16(const char* input, size_t length, wchar_t* output, size_t output_len) {
    uint32_t code_point;
    for (size_t i = 0; i < length;) {
        uint8_t n = wtf8_decode(&input[i], &code_point);
        NEKO_ASSERT(n > 0);
        if (code_point > 0x10000) {
            NEKO_ASSERT(code_point < 0x10FFFF);
            *output++ = (((code_point - 0x10000) >> 10) + 0xD800);
            *output++ = ((code_point - 0x10000) & 0x3FF) + 0xDC00;
            output_len -= 2;
        } else {
            *output++ = code_point;
            output_len -= 1;
        }
        i += n;
    }
    (void)output_len;
    NEKO_ASSERT(output_len == 0);
}

inline uint32_t wtf8_surrogate(const wchar_t* input, bool eof) {
    uint32_t u = input[0];
    if (u >= 0xD800 && u <= 0xDBFF && !eof) {
        uint32_t next = input[1];
        if (next >= 0xDC00 && next <= 0xDFFF) {
            return 0x10000 + ((u - 0xD800) << 10) + (next - 0xDC00);
        }
    }
    return u;
}

inline size_t wtf8_from_utf16_length(const wchar_t* input, size_t length) {
    size_t output_len = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t code_point = wtf8_surrogate(&input[i], length == i + 1);
        if (code_point == 0) {
            break;
        }
        if (code_point < 0x80) {
            output_len += 1;
        } else if (code_point < 0x800) {
            output_len += 2;
        } else if (code_point < 0x10000) {
            output_len += 3;
        } else {
            output_len += 4;
            i++;
        }
    }
    return output_len;
}

inline void wtf8_from_utf16(const wchar_t* input, size_t length, char* output, size_t output_len) {
    for (size_t i = 0; i < length; ++i) {
        uint32_t code_point = wtf8_surrogate(&input[i], length == i + 1);
        if (code_point == 0) {
            break;
        }
        if (code_point < 0x80) {
            *output++ = code_point;
            output_len -= 1;
        } else if (code_point < 0x800) {
            *output++ = 0xC0 | (code_point >> 6);
            *output++ = 0x80 | (code_point & 0x3F);
            output_len -= 2;
        } else if (code_point < 0x10000) {
            *output++ = 0xE0 | (code_point >> 12);
            *output++ = 0x80 | ((code_point >> 6) & 0x3F);
            *output++ = 0x80 | (code_point & 0x3F);
            output_len -= 3;
        } else {
            *output++ = 0xF0 | (code_point >> 18);
            *output++ = 0x80 | ((code_point >> 12) & 0x3F);
            *output++ = 0x80 | ((code_point >> 6) & 0x3F);
            *output++ = 0x80 | (code_point & 0x3F);
            output_len -= 4;
            i++;
        }
    }
    (void)output_len;
    NEKO_ASSERT(output_len == 0);
}

NEKO_FORCE_INLINE u32 neko_string_length(const char* txt) {
    u32 sz = 0;
    while (txt != NULL && txt[sz] != '\0') {
        sz++;
    }
    return sz;
}

#define neko_strlen(str) neko_string_length((const char*)str)

// Expects null terminated strings
NEKO_FORCE_INLINE bool neko_string_compare_equal(const char* txt, const char* cmp) {
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

NEKO_FORCE_INLINE bool neko_string_compare_equal_n(const char* txt, const char* cmp, u32 n) {
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

// NEKO_FORCE_INLINE char* neko_util_string_concat(char* s1, const char* s2) {
//     const size_t a = strlen(s1);
//     const size_t b = strlen(s2);
//     const size_t ab = a + b + 1;
//     s1 = (char*)neko_safe_realloc((void*)s1, ab);
//     memcpy(s1 + a, s2, b + 1);
//     return s1;
// }

NEKO_FORCE_INLINE void neko_util_str_to_lower(const char* src, char* buffer, size_t buffer_sz) {
    size_t src_sz = neko_string_length(src);
    size_t len = NEKO_MIN(src_sz, buffer_sz - 1);

    for (u32 i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
    if (len) buffer[len] = '\0';
}

NEKO_FORCE_INLINE bool neko_util_str_is_numeric(const char* str) {
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

NEKO_FORCE_INLINE void neko_util_get_file_extension(char* buffer, u32 buffer_size, const_str file_path) {
    NEKO_ASSERT(buffer && buffer_size);
    const_str extension = strrchr(file_path, '.');
    if (extension) {
        uint32_t extension_len = strlen(extension + 1);
        uint32_t len = (extension_len >= buffer_size) ? buffer_size - 1 : extension_len;
        memcpy(buffer, extension + 1, len);
        buffer[len] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

NEKO_FORCE_INLINE void neko_util_get_dir_from_file(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* end = (file_path + str_len);
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

NEKO_FORCE_INLINE const_str neko_util_get_filename(const_str path) {
    NEKO_ASSERT(path);
    int len = strlen(path);
    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '\\' || path[i] == '/') {
            return path + i + 1;
        }
    }
    return path;
}

NEKO_FORCE_INLINE void neko_util_string_substring(const char* src, char* dst, size_t sz, u32 start, u32 end) {
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

NEKO_FORCE_INLINE void neko_util_string_remove_character(const char* src, char* buffer, u32 buffer_size, char delimiter) {
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

NEKO_FORCE_INLINE void neko_util_string_replace(char* buffer, size_t buffer_sz, const char* replace, char fallback) {
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

NEKO_FORCE_INLINE void neko_util_string_replace_delim(const char* source_str, char* buffer, u32 buffer_size, char delimiter, char replace) {
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
NEKO_FORCE_INLINE u32 neko_hash_str(const char* str) {
    u32 hash = 5381;
    i32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

NEKO_FORCE_INLINE u64 neko_hash_str64(const char* str) {
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

NEKO_FORCE_INLINE bool neko_compare_bytes(void* b0, void* b1, size_t len) { return 0 == memcmp(b0, b1, len); }

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define NEKO_SIZE_T_BITS ((sizeof(size_t)) * 8)
#define NEKO_SIPHASH_C_ROUNDS 1
#define NEKO_SIPHASH_D_ROUNDS 1
#define neko_rotate_left(__V, __N) (((__V) << (__N)) | ((__V) >> (NEKO_SIZE_T_BITS - (__N))))
#define neko_rotate_right(__V, __N) (((__V) >> (__N)) | ((__V) << (NEKO_SIZE_T_BITS - (__N))))

NEKO_FORCE_INLINE size_t neko_hash_siphash_bytes(void* p, size_t len, size_t seed) {
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

NEKO_FORCE_INLINE size_t neko_hash_bytes(void* p, size_t len, size_t seed) {
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

/*========================
// Byte Buffer
========================*/

/** @defgroup neko_byte_buffer Byte Buffer
 *  @ingroup neko_containers
 *  Byte Buffer
 */

#define NEKO_BYTE_BUFFER_DEFAULT_CAPCITY 1024

/** @addtogroup neko_byte_buffer
 */
typedef struct neko_byte_buffer_t {
    u8* data;      // Buffer that actually holds all relevant byte data
    u32 size;      // Current size of the stored buffer data
    u32 position;  // Current read/write position in the buffer
    u32 capacity;  // Current max capacity for the buffer
} neko_byte_buffer_t;

// Generic "write" function for a byte buffer
#define neko_byte_buffer_write(__BB, __T, __VAL)              \
    do {                                                      \
        neko_byte_buffer_t* __BUFFER = __BB;                  \
        usize __SZ = sizeof(__T);                             \
        usize __TWS = __BUFFER->position + __SZ;              \
        if (__TWS >= (usize)__BUFFER->capacity) {             \
            usize __CAP = __BUFFER->capacity * 2;             \
            while (__CAP < __TWS) {                           \
                __CAP *= 2;                                   \
            }                                                 \
            neko_byte_buffer_resize(__BUFFER, __CAP);         \
        }                                                     \
        *(__T*)(__BUFFER->data + __BUFFER->position) = __VAL; \
        __BUFFER->position += (u32)__SZ;                      \
        __BUFFER->size += (u32)__SZ;                          \
    } while (0)

// Generic "read" function
#define neko_byte_buffer_read(__BUFFER, __T, __VAL_P)  \
    do {                                               \
        __T* __V = (__T*)(__VAL_P);                    \
        neko_byte_buffer_t* __BB = (__BUFFER);         \
        *(__V) = *(__T*)(__BB->data + __BB->position); \
        __BB->position += sizeof(__T);                 \
    } while (0)

// Defines variable and sets value from buffer in place
// Use to construct a new variable
#define neko_byte_buffer_readc(__BUFFER, __T, __NAME) \
    __T __NAME = NEKO_DEFAULT_VAL();                  \
    neko_byte_buffer_read((__BUFFER), __T, &__NAME);

#define neko_byte_buffer_read_bulkc(__BUFFER, __T, __NAME, __SZ) \
    __T __NAME = NEKO_DEFAULT_VAL();                             \
    __T* NEKO_CONCAT(__NAME, __LINE__) = &(__NAME);              \
    neko_byte_buffer_read_bulk(__BUFFER, (void**)&NEKO_CONCAT(__NAME, __LINE__), __SZ);

void neko_byte_buffer_init(neko_byte_buffer_t* buffer);
neko_byte_buffer_t neko_byte_buffer_new();
void neko_byte_buffer_free(neko_byte_buffer_t* buffer);
void neko_byte_buffer_clear(neko_byte_buffer_t* buffer);
bool neko_byte_buffer_empty(neko_byte_buffer_t* buffer);
size_t neko_byte_buffer_size(neko_byte_buffer_t* buffer);
void neko_byte_buffer_resize(neko_byte_buffer_t* buffer, size_t sz);
void neko_byte_buffer_copy_contents(neko_byte_buffer_t* dst, neko_byte_buffer_t* src);
void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t* buffer);
void neko_byte_buffer_seek_to_end(neko_byte_buffer_t* buffer);
void neko_byte_buffer_advance_position(neko_byte_buffer_t* buffer, size_t sz);
void neko_byte_buffer_write_str(neko_byte_buffer_t* buffer, const char* str);  // Expects a null terminated string
void neko_byte_buffer_read_str(neko_byte_buffer_t* buffer, char* str);         // Expects an allocated string
void neko_byte_buffer_write_bulk(neko_byte_buffer_t* buffer, void* src, size_t sz);
void neko_byte_buffer_read_bulk(neko_byte_buffer_t* buffer, void** dst, size_t sz);
bool neko_byte_buffer_write_to_file(neko_byte_buffer_t* buffer, const char* output_path);  // Assumes that the output directory exists
bool neko_byte_buffer_read_from_file(neko_byte_buffer_t* buffer, const char* file_path);   // Assumes an allocated byte buffer
void neko_byte_buffer_memset(neko_byte_buffer_t* buffer, u8 val);

/*===================================
// Dynamic Array
===================================*/

/** @defgroup neko_dyn_array Dynamic Array
 *  @ingroup neko_containers
 *  Dynamic Array
 */

/** @addtogroup neko_dyn_array
 */
typedef struct neko_dyn_array {
    int32_t size;
    int32_t capacity;
} neko_dyn_array;

#define neko_dyn_array_head(__ARR) ((neko_dyn_array*)((u8*)(__ARR) - sizeof(neko_dyn_array)))

#define neko_dyn_array_size(__ARR) (__ARR == NULL ? 0 : neko_dyn_array_head((__ARR))->size)

#define neko_dyn_array_capacity(__ARR) (__ARR == NULL ? 0 : neko_dyn_array_head((__ARR))->capacity)

#define neko_dyn_array_full(__ARR) ((neko_dyn_array_size((__ARR)) == neko_dyn_array_capacity((__ARR))))

#define neko_dyn_array_byte_size(__ARR) (neko_dyn_array_size((__ARR)) * sizeof(*__ARR))

void* neko_dyn_array_resize_impl(void* arr, size_t sz, size_t amount);

#define neko_dyn_array_need_grow(__ARR, __N) ((__ARR) == 0 || neko_dyn_array_size(__ARR) + (__N) >= neko_dyn_array_capacity(__ARR))

#define neko_dyn_array_grow(__ARR) neko_dyn_array_resize_impl((__ARR), sizeof(*(__ARR)), neko_dyn_array_capacity(__ARR) ? neko_dyn_array_capacity(__ARR) * 2 : 1)

#define neko_dyn_array_grow_size(__ARR, __SZ) neko_dyn_array_resize_impl((__ARR), (__SZ), neko_dyn_array_capacity(__ARR) ? neko_dyn_array_capacity(__ARR) * 2 : 1)

void** neko_dyn_array_init(void** arr, size_t val_len);

void neko_dyn_array_push_data(void** arr, void* val, size_t val_len);

NEKO_FORCE_INLINE void neko_dyn_array_set_data_i(void** arr, void* val, size_t val_len, u32 offset) { memcpy(((char*)(*arr)) + offset * val_len, val, val_len); }

#define neko_dyn_array_push(__ARR, __ARRVAL)                               \
    do {                                                                   \
        neko_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));           \
        if (!(__ARR) || ((__ARR) && neko_dyn_array_need_grow(__ARR, 1))) { \
            *((void**)&(__ARR)) = neko_dyn_array_grow(__ARR);              \
        }                                                                  \
        (__ARR)[neko_dyn_array_size(__ARR)] = (__ARRVAL);                  \
        neko_dyn_array_head(__ARR)->size++;                                \
    } while (0)

#define neko_dyn_array_reserve(__ARR, __AMOUNT)                                                \
    do {                                                                                       \
        if ((!__ARR)) neko_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));                 \
        if ((!__ARR) || (size_t)__AMOUNT > neko_dyn_array_capacity(__ARR)) {                   \
            *((void**)&(__ARR)) = neko_dyn_array_resize_impl(__ARR, sizeof(*__ARR), __AMOUNT); \
        }                                                                                      \
    } while (0)

#define neko_dyn_array_empty(__ARR) (neko_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR))), (neko_dyn_array_size(__ARR) == 0))

#define neko_dyn_array_pop(__ARR)                    \
    do {                                             \
        if (__ARR && !neko_dyn_array_empty(__ARR)) { \
            neko_dyn_array_head(__ARR)->size -= 1;   \
        }                                            \
    } while (0)

#define neko_dyn_array_back(__ARR) *(__ARR + (neko_dyn_array_size(__ARR) ? neko_dyn_array_size(__ARR) - 1 : 0))

#define neko_dyn_array_for(__ARR, __T, __IT_NAME) for (__T* __IT_NAME = __ARR; __IT_NAME != neko_dyn_array_back(__ARR); ++__IT_NAME)

#define neko_dyn_array_new(__T) ((__T*)neko_dyn_array_resize_impl(NULL, sizeof(__T), 0))

#define neko_dyn_array_clear(__ARR)               \
    do {                                          \
        if (__ARR) {                              \
            neko_dyn_array_head(__ARR)->size = 0; \
        }                                         \
    } while (0)

#define neko_dyn_array(__T) __T*

#define neko_dyn_array_free(__ARR)                \
    do {                                          \
        if (__ARR) {                              \
            mem_free(neko_dyn_array_head(__ARR)); \
            (__ARR) = NULL;                       \
        }                                         \
    } while (0)

/*===================================
// Hash Table
===================================*/

/*
    If using struct for keys, requires struct to be word-aligned.
*/

#define NEKO_HASH_TABLE_HASH_SEED 0x31415296
#define NEKO_HASH_TABLE_INVALID_INDEX UINT32_MAX

typedef enum neko_hash_table_entry_state { NEKO_HASH_TABLE_ENTRY_INACTIVE = 0x00, NEKO_HASH_TABLE_ENTRY_ACTIVE = 0x01 } neko_hash_table_entry_state;

#define __neko_hash_table_entry(__HMK, __HMV) \
    struct {                                  \
        __HMK key;                            \
        __HMV val;                            \
        neko_hash_table_entry_state state;    \
    }

#define neko_hash_table(__HMK, __HMV)                 \
    struct {                                          \
        __neko_hash_table_entry(__HMK, __HMV) * data; \
        __HMK tmp_key;                                \
        __HMV tmp_val;                                \
        size_t stride;                                \
        size_t klpvl;                                 \
        size_t tmp_idx;                               \
    }*

// Need a way to create a temporary key so I can take the address of it

#define neko_hash_table_new(__K, __V) NULL

void __neko_hash_table_init_impl(void** ht, size_t sz);

#define neko_hash_table_init(__HT, __K, __V)                                                 \
    do {                                                                                     \
        size_t entry_sz = sizeof(*__HT->data);                                               \
        size_t ht_sz = sizeof(*__HT);                                                        \
        __neko_hash_table_init_impl((void**)&(__HT), ht_sz);                                 \
        memset((__HT), 0, ht_sz);                                                            \
        neko_dyn_array_reserve(__HT->data, 2);                                               \
        __HT->data[0].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                \
        __HT->data[1].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                \
        uintptr_t d0 = (uintptr_t) & ((__HT)->data[0]);                                      \
        uintptr_t d1 = (uintptr_t) & ((__HT)->data[1]);                                      \
        ptrdiff_t diff = (d1 - d0);                                                          \
        ptrdiff_t klpvl = (uintptr_t) & (__HT->data[0].state) - (uintptr_t)(&__HT->data[0]); \
        (__HT)->stride = (size_t)(diff);                                                     \
        (__HT)->klpvl = (size_t)(klpvl);                                                     \
    } while (0)

#define neko_hash_table_reserve(_HT, _KT, _VT, _CT) \
    do {                                            \
        if ((_HT) == NULL) {                        \
            neko_hash_table_init((_HT), _KT, _VT);  \
        }                                           \
        neko_dyn_array_reserve((_HT)->data, _CT);   \
    } while (0)

// ((__HT) != NULL ? (__HT)->size : 0) // neko_dyn_array_size((__HT)->data) : 0)
#define neko_hash_table_size(__HT) ((__HT) != NULL ? neko_dyn_array_size((__HT)->data) : 0)

#define neko_hash_table_capacity(__HT) ((__HT) != NULL ? neko_dyn_array_capacity((__HT)->data) : 0)

#define neko_hash_table_load_factor(__HT) (neko_hash_table_capacity(__HT) ? (float)(neko_hash_table_size(__HT)) / (float)(neko_hash_table_capacity(__HT)) : 0.f)

#define neko_hash_table_grow(__HT, __C) ((__HT)->data = neko_dyn_array_resize_impl((__HT)->data, sizeof(*((__HT)->data)), (__C)))

#define neko_hash_table_empty(__HT) ((__HT) != NULL ? neko_dyn_array_size((__HT)->data) == 0 : true)

#define neko_hash_table_clear(__HT)                                                \
    do {                                                                           \
        if ((__HT) != NULL) {                                                      \
            u32 capacity = neko_dyn_array_capacity((__HT)->data);                  \
            for (u32 i = 0; i < capacity; ++i) {                                   \
                (__HT)->data[i].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;            \
            }                                                                      \
            /*memset((__HT)->data, 0, neko_dyn_array_capacity((__HT)->data) * );*/ \
            neko_dyn_array_clear((__HT)->data);                                    \
        }                                                                          \
    } while (0)

#define neko_hash_table_free(__HT)             \
    do {                                       \
        if ((__HT) != NULL) {                  \
            neko_dyn_array_free((__HT)->data); \
            (__HT)->data = NULL;               \
            mem_free(__HT);                    \
            (__HT) = NULL;                     \
        }                                      \
    } while (0)

// Find available slot to insert k/v pair into
#define neko_hash_table_insert(__HT, __HMK, __HMV)                                                                                                                                                    \
    do {                                                                                                                                                                                              \
        /* Check for null hash table, init if necessary */                                                                                                                                            \
        if ((__HT) == NULL) {                                                                                                                                                                         \
            neko_hash_table_init((__HT), (__HMK), (__HMV));                                                                                                                                           \
        }                                                                                                                                                                                             \
                                                                                                                                                                                                      \
        /* Grow table if necessary */                                                                                                                                                                 \
        u32 __CAP = neko_hash_table_capacity(__HT);                                                                                                                                                   \
        float __LF = neko_hash_table_load_factor(__HT);                                                                                                                                               \
        if (__LF >= 0.5f || !__CAP) {                                                                                                                                                                 \
            u32 NEW_CAP = __CAP ? __CAP * 2 : 2;                                                                                                                                                      \
            size_t ENTRY_SZ = sizeof((__HT)->tmp_key) + sizeof((__HT)->tmp_val) + sizeof(neko_hash_table_entry_state);                                                                                \
            neko_dyn_array_reserve((__HT)->data, NEW_CAP);                                                                                                                                            \
            /**((void **)&(__HT->data)) = neko_dyn_array_resize_impl(__HT->data, ENTRY_SZ, NEW_CAP);*/                                                                                                \
            /* Iterate through data and set state to null, from __CAP -> __CAP * 2 */                                                                                                                 \
            /* Memset here instead */                                                                                                                                                                 \
            for (u32 __I = __CAP; __I < NEW_CAP; ++__I) {                                                                                                                                             \
                (__HT)->data[__I].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                                                                                                             \
            }                                                                                                                                                                                         \
            __CAP = neko_hash_table_capacity(__HT);                                                                                                                                                   \
        }                                                                                                                                                                                             \
                                                                                                                                                                                                      \
        /* Get hash of key */                                                                                                                                                                         \
        (__HT)->tmp_key = (__HMK);                                                                                                                                                                    \
        size_t __HSH = neko_hash_bytes((void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), NEKO_HASH_TABLE_HASH_SEED);                                                                                \
        size_t __HSH_IDX = __HSH % __CAP;                                                                                                                                                             \
        (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;                                                                                                                                                \
        u32 c = 0;                                                                                                                                                                                    \
                                                                                                                                                                                                      \
        /* Find valid idx and place data */                                                                                                                                                           \
        while (c < __CAP && __HSH != neko_hash_bytes((void*)&(__HT)->tmp_key, sizeof((__HT)->tmp_key), NEKO_HASH_TABLE_HASH_SEED) && (__HT)->data[__HSH_IDX].state == NEKO_HASH_TABLE_ENTRY_ACTIVE) { \
            __HSH_IDX = ((__HSH_IDX + 1) % __CAP);                                                                                                                                                    \
            (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;                                                                                                                                            \
            ++c;                                                                                                                                                                                      \
        }                                                                                                                                                                                             \
        (__HT)->data[__HSH_IDX].key = (__HMK);                                                                                                                                                        \
        (__HT)->data[__HSH_IDX].val = (__HMV);                                                                                                                                                        \
        (__HT)->data[__HSH_IDX].state = NEKO_HASH_TABLE_ENTRY_ACTIVE;                                                                                                                                 \
        neko_dyn_array_head((__HT)->data)->size++;                                                                                                                                                    \
    } while (0)

// Need size difference between two entries
// Need size of key + val

NEKO_FORCE_INLINE u32 neko_hash_table_get_key_index_func(void** data, void* key, size_t key_len, size_t val_len, size_t stride, size_t klpvl) {
    if (!data || !key) return NEKO_HASH_TABLE_INVALID_INDEX;

    // Need a better way to handle this. Can't do it like this anymore.
    // Need to fix this. Seriously messing me up.
    u32 capacity = neko_dyn_array_capacity(*data);
    u32 size = neko_dyn_array_size(*data);
    if (!capacity || !size) return (size_t)NEKO_HASH_TABLE_INVALID_INDEX;
    size_t idx = (size_t)NEKO_HASH_TABLE_INVALID_INDEX;
    size_t hash = (size_t)neko_hash_bytes(key, key_len, NEKO_HASH_TABLE_HASH_SEED);
    size_t hash_idx = (hash % capacity);

    // Iterate through data
    for (size_t i = hash_idx, c = 0; c < capacity; ++c, i = ((i + 1) % capacity)) {
        size_t offset = (i * stride);
        void* k = ((char*)(*data) + (offset));
        size_t kh = neko_hash_bytes(k, key_len, NEKO_HASH_TABLE_HASH_SEED);
        bool comp = neko_compare_bytes(k, key, key_len);
        neko_hash_table_entry_state state = *(neko_hash_table_entry_state*)((char*)(*data) + offset + (klpvl));
        if (comp && hash == kh && state == NEKO_HASH_TABLE_ENTRY_ACTIVE) {
            idx = i;
            break;
        }
    }
    return (u32)idx;
}

// Get key at index
#define neko_hash_table_getk(__HT, __I) (((__HT))->data[(__I)].key)

// Get val at index
#define neko_hash_table_geti(__HT, __I) ((__HT)->data[(__I)].val)

// Could search for the index in the macro instead now. Does this help me?
#define neko_hash_table_get(__HT, __HTK)                                                                                                                                                             \
    ((__HT)->tmp_key = (__HTK), (neko_hash_table_geti((__HT), neko_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), \
                                                                                                 (__HT)->stride, (__HT)->klpvl))))

#define neko_hash_table_getp(__HT, __HTK)                                                                                                                                                  \
    ((__HT)->tmp_key = (__HTK),                                                                                                                                                            \
     ((__HT)->tmp_idx = (u32)neko_hash_table_get_key_index_func((void**)&(__HT->data), (void*)&(__HT->tmp_key), sizeof(__HT->tmp_key), sizeof(__HT->tmp_val), __HT->stride, __HT->klpvl)), \
     ((__HT)->tmp_idx != NEKO_HASH_TABLE_INVALID_INDEX ? &neko_hash_table_geti((__HT), (__HT)->tmp_idx) : NULL))

#define _neko_hash_table_key_exists_internal(__HT, __HTK) \
    ((__HT)->tmp_key = (__HTK),                           \
     (neko_hash_table_get_key_index_func((void**)&(__HT->data), (void*)&(__HT->tmp_key), sizeof(__HT->tmp_key), sizeof(__HT->tmp_val), __HT->stride, __HT->klpvl) != NEKO_HASH_TABLE_INVALID_INDEX))

// u32 neko_hash_table_get_key_index_func(void** data, void* key, size_t key_len, size_t val_len, size_t stride, size_t klpvl)

#define neko_hash_table_exists(__HT, __HTK) (__HT && _neko_hash_table_key_exists_internal((__HT), (__HTK)))

#define neko_hash_table_key_exists(__HT, __HTK) (neko_hash_table_exists((__HT), (__HTK)))

#define neko_hash_table_erase(__HT, __HTK)                                                                                                                                                     \
    do {                                                                                                                                                                                       \
        if ((__HT)) {                                                                                                                                                                          \
            /* Get idx for key */                                                                                                                                                              \
            (__HT)->tmp_key = (__HTK);                                                                                                                                                         \
            u32 __IDX = neko_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__HT)->stride, (__HT)->klpvl); \
            if (__IDX != NEKO_HASH_TABLE_INVALID_INDEX) {                                                                                                                                      \
                (__HT)->data[__IDX].state = NEKO_HASH_TABLE_ENTRY_INACTIVE;                                                                                                                    \
                if (neko_dyn_array_head((__HT)->data)->size) neko_dyn_array_head((__HT)->data)->size--;                                                                                        \
            }                                                                                                                                                                                  \
        }                                                                                                                                                                                      \
    } while (0)

/*===== Hash Table Iterator ====*/

typedef u32 neko_hash_table_iter;

NEKO_FORCE_INLINE u32 __neko_find_first_valid_iterator(void* data, size_t key_len, size_t val_len, u32 idx, size_t stride, size_t klpvl) {
    u32 it = (u32)idx;
    for (; it < (u32)neko_dyn_array_capacity(data); ++it) {
        size_t offset = (it * stride);
        neko_hash_table_entry_state state = *(neko_hash_table_entry_state*)((u8*)data + offset + (klpvl));
        if (state == NEKO_HASH_TABLE_ENTRY_ACTIVE) {
            break;
        }
    }
    return it;
}

/* Find first valid iterator idx */
#define neko_hash_table_iter_new(__HT) (__HT ? __neko_find_first_valid_iterator((__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), 0, (__HT)->stride, (__HT)->klpvl) : 0)

#define neko_hash_table_iter_valid(__HT, __IT) ((__IT) < neko_hash_table_capacity((__HT)))

// Have to be able to do this for hash table...
NEKO_FORCE_INLINE void __neko_hash_table_iter_advance_func(void** data, size_t key_len, size_t val_len, u32* it, size_t stride, size_t klpvl) {
    (*it)++;
    for (; *it < (u32)neko_dyn_array_capacity(*data); ++*it) {
        size_t offset = (size_t)(*it * stride);
        neko_hash_table_entry_state state = *(neko_hash_table_entry_state*)((u8*)*data + offset + (klpvl));
        if (state == NEKO_HASH_TABLE_ENTRY_ACTIVE) {
            break;
        }
    }
}

#define neko_hash_table_find_valid_iter(__HT, __IT) \
    ((__IT) = __neko_find_first_valid_iterator((void**)&(__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__IT), (__HT)->stride, (__HT)->klpvl))

#define neko_hash_table_iter_advance(__HT, __IT) (__neko_hash_table_iter_advance_func((void**)&(__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), &(__IT), (__HT)->stride, (__HT)->klpvl))

#define neko_hash_table_iter_get(__HT, __IT) neko_hash_table_geti(__HT, __IT)

#define neko_hash_table_iter_getp(__HT, __IT) (&(neko_hash_table_geti(__HT, __IT)))

#define neko_hash_table_iter_getk(__HT, __IT) (neko_hash_table_getk(__HT, __IT))

#define neko_hash_table_iter_getkp(__HT, __IT) (&(neko_hash_table_getk(__HT, __IT)))

/*===================================
// Slot Array
===================================*/

#define NEKO_SLOT_ARRAY_INVALID_HANDLE UINT32_MAX

#define neko_slot_array_handle_valid(__SA, __ID) ((__SA) && __ID < neko_dyn_array_size((__SA)->indices) && (__SA)->indices[__ID] != NEKO_SLOT_ARRAY_INVALID_HANDLE)

typedef struct __neko_slot_array_dummy_header {
    neko_dyn_array(u32) indices;
    neko_dyn_array(u32) data;
} __neko_slot_array_dummy_header;

#define neko_slot_array(__T)         \
    struct {                         \
        neko_dyn_array(u32) indices; \
        neko_dyn_array(__T) data;    \
        __T tmp;                     \
    }*

#define neko_slot_array_new(__T) NULL

NEKO_FORCE_INLINE u32 __neko_slot_array_find_next_available_index(neko_dyn_array(u32) indices) {
    u32 idx = NEKO_SLOT_ARRAY_INVALID_HANDLE;
    for (u32 i = 0; i < (u32)neko_dyn_array_size(indices); ++i) {
        u32 handle = indices[i];
        if (handle == NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            idx = i;
            break;
        }
    }
    if (idx == NEKO_SLOT_ARRAY_INVALID_HANDLE) {
        idx = neko_dyn_array_size(indices);
    }

    return idx;
}

void** neko_slot_array_init(void** sa, size_t sz);

#define neko_slot_array_init_all(__SA) \
    (neko_slot_array_init((void**)&(__SA), sizeof(*(__SA))), neko_dyn_array_init((void**)&((__SA)->indices), sizeof(u32)), neko_dyn_array_init((void**)&((__SA)->data), sizeof((__SA)->tmp)))

NEKO_FORCE_INLINE u32 neko_slot_array_insert_func(void** indices, void** data, void* val, size_t val_len, u32* ip) {
    // Find next available index
    u32 idx = __neko_slot_array_find_next_available_index((u32*)*indices);

    if (idx == neko_dyn_array_size(*indices)) {
        u32 v = 0;
        neko_dyn_array_push_data(indices, &v, sizeof(u32));
        idx = neko_dyn_array_size(*indices) - 1;
    }

    // Push data to array
    neko_dyn_array_push_data(data, val, val_len);

    // Set data in indices
    u32 bi = neko_dyn_array_size(*data) - 1;
    neko_dyn_array_set_data_i(indices, &bi, sizeof(u32), idx);

    if (ip) {
        *ip = idx;
    }

    return idx;
}

#define neko_slot_array_reserve(__SA, __NUM)            \
    do {                                                \
        neko_slot_array_init_all(__SA);                 \
        neko_dyn_array_reserve((__SA)->data, __NUM);    \
        neko_dyn_array_reserve((__SA)->indices, __NUM); \
    } while (0)

#define neko_slot_array_insert(__SA, __VAL) \
    (neko_slot_array_init_all(__SA), (__SA)->tmp = (__VAL), neko_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), (void*)&((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define neko_slot_array_insert_hp(__SA, __VAL, __hp) \
    (neko_slot_array_init_all(__SA), (__SA)->tmp = (__VAL), neko_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), (__hp)))

#define neko_slot_array_insert_no_init(__SA, __VAL) \
    ((__SA)->tmp = (__VAL), neko_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define neko_slot_array_size(__SA) ((__SA) == NULL ? 0 : neko_dyn_array_size((__SA)->data))

#define neko_slot_array_empty(__SA) (neko_slot_array_size(__SA) == 0)

#define neko_slot_array_clear(__SA)                \
    do {                                           \
        if ((__SA) != NULL) {                      \
            neko_dyn_array_clear((__SA)->data);    \
            neko_dyn_array_clear((__SA)->indices); \
        }                                          \
    } while (0)

#define neko_slot_array_exists(__SA, __SID) ((__SA) && (__SID) < (u32)neko_dyn_array_size((__SA)->indices) && (__SA)->indices[__SID] != NEKO_SLOT_ARRAY_INVALID_HANDLE)

#define neko_slot_array_get(__SA, __SID) ((__SA)->data[(__SA)->indices[(__SID) % neko_dyn_array_size(((__SA)->indices))]])

#define neko_slot_array_getp(__SA, __SID) (&(neko_slot_array_get(__SA, (__SID))))

#define neko_slot_array_free(__SA)                \
    do {                                          \
        if ((__SA) != NULL) {                     \
            neko_dyn_array_free((__SA)->data);    \
            neko_dyn_array_free((__SA)->indices); \
            (__SA)->indices = NULL;               \
            (__SA)->data = NULL;                  \
            mem_free((__SA));                     \
            (__SA) = NULL;                        \
        }                                         \
    } while (0)

#define neko_slot_array_erase(__SA, __id)                                                       \
    do {                                                                                        \
        u32 __H0 = (__id) /*% neko_dyn_array_size((__SA)->indices)*/;                           \
        if (neko_slot_array_size(__SA) == 1) {                                                  \
            neko_slot_array_clear(__SA);                                                        \
        } else if (!neko_slot_array_handle_valid(__SA, __H0)) {                                 \
            neko_println("Warning: Attempting to erase invalid slot array handle (%zu)", __H0); \
        } else {                                                                                \
            u32 __OG_DATA_IDX = (__SA)->indices[__H0];                                          \
            /* Iterate through handles until last index of data found */                        \
            u32 __H = 0;                                                                        \
            for (u32 __I = 0; __I < neko_dyn_array_size((__SA)->indices); ++__I) {              \
                if ((__SA)->indices[__I] == neko_dyn_array_size((__SA)->data) - 1) {            \
                    __H = __I;                                                                  \
                    break;                                                                      \
                }                                                                               \
            }                                                                                   \
                                                                                                \
            /* Swap and pop data */                                                             \
            (__SA)->data[__OG_DATA_IDX] = neko_dyn_array_back((__SA)->data);                    \
            neko_dyn_array_pop((__SA)->data);                                                   \
                                                                                                \
            /* Point new handle, Set og handle to invalid */                                    \
            (__SA)->indices[__H] = __OG_DATA_IDX;                                               \
            (__SA)->indices[__H0] = NEKO_SLOT_ARRAY_INVALID_HANDLE;                             \
        }                                                                                       \
    } while (0)

/*=== Slot Array Iterator ===*/

// Slot array iterator new
typedef u32 neko_slot_array_iter;

#define neko_slot_array_iter_valid(__SA, __IT) (__SA && neko_slot_array_exists(__SA, __IT))

NEKO_FORCE_INLINE void _neko_slot_array_iter_advance_func(neko_dyn_array(u32) indices, u32* it) {
    if (!indices) {
        *it = NEKO_SLOT_ARRAY_INVALID_HANDLE;
        return;
    }

    (*it)++;
    for (; *it < (u32)neko_dyn_array_size(indices); ++*it) {
        if (indices[*it] != NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            break;
        }
    }
}

NEKO_FORCE_INLINE u32 _neko_slot_array_iter_find_first_valid_index(neko_dyn_array(u32) indices) {
    if (!indices) return NEKO_SLOT_ARRAY_INVALID_HANDLE;

    for (u32 i = 0; i < (u32)neko_dyn_array_size(indices); ++i) {
        if (indices[i] != NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            return i;
        }
    }
    return NEKO_SLOT_ARRAY_INVALID_HANDLE;
}

#define neko_slot_array_iter_new(__SA) (_neko_slot_array_iter_find_first_valid_index((__SA) ? (__SA)->indices : 0))

#define neko_slot_array_iter_advance(__SA, __IT) _neko_slot_array_iter_advance_func((__SA) ? (__SA)->indices : NULL, &(__IT))

#define neko_slot_array_iter_get(__SA, __IT) neko_slot_array_get(__SA, __IT)

#define neko_slot_array_iter_getp(__SA, __IT) neko_slot_array_getp(__SA, __IT)

/*===================================
// Slot Map
===================================*/

#define neko_slot_map(__SMK, __SMV)     \
    struct {                            \
        neko_hash_table(__SMK, u32) ht; \
        neko_slot_array(__SMV) sa;      \
    }*

#define neko_slot_map_new(__SMK, __SMV) NULL

void** neko_slot_map_init(void** sm);

// Could return something, I believe?
#define neko_slot_map_insert(__SM, __SMK, __SMV)                 \
    do {                                                         \
        neko_slot_map_init((void**)&(__SM));                     \
        u32 __H = neko_slot_array_insert((__SM)->sa, ((__SMV))); \
        neko_hash_table_insert((__SM)->ht, (__SMK), __H);        \
    } while (0)

#define neko_slot_map_get(__SM, __SMK) (neko_slot_array_get((__SM)->sa, neko_hash_table_get((__SM)->ht, (__SMK))))

#define neko_slot_map_getp(__SM, __SMK) (neko_slot_array_getp((__SM)->sa, neko_hash_table_get((__SM)->ht, (__SMK))))

#define neko_slot_map_size(__SM) (neko_slot_array_size((__SM)->sa))

#define neko_slot_map_clear(__SM)              \
    do {                                       \
        if ((__SM) != NULL) {                  \
            neko_hash_table_clear((__SM)->ht); \
            neko_slot_array_clear((__SM)->sa); \
        }                                      \
    } while (0)

#define neko_slot_map_erase(__SM, __SMK)                    \
    do {                                                    \
        u32 __K = neko_hash_table_get((__SM)->ht, (__SMK)); \
        neko_hash_table_erase((__SM)->ht, (__SMK));         \
        neko_slot_array_erase((__SM)->sa, __K);             \
    } while (0)

#define neko_slot_map_free(__SM)              \
    do {                                      \
        if (__SM != NULL) {                   \
            neko_hash_table_free((__SM)->ht); \
            neko_slot_array_free((__SM)->sa); \
            mem_free((__SM));                 \
            (__SM) = NULL;                    \
        }                                     \
    } while (0)

#define neko_slot_map_capacity(__SM) (neko_hash_table_capacity((__SM)->ht))

/*=== Slot Map Iterator ===*/

typedef u32 neko_slot_map_iter;

/* Find first valid iterator idx */
#define neko_slot_map_iter_new(__SM) neko_hash_table_iter_new((__SM)->ht)

#define neko_slot_map_iter_valid(__SM, __IT) ((__IT) < neko_hash_table_capacity((__SM)->ht))

#define neko_slot_map_iter_advance(__SM, __IT) \
    __neko_hash_table_iter_advance_func((void**)&((__SM)->ht->data), sizeof((__SM)->ht->tmp_key), sizeof((__SM)->ht->tmp_val), &(__IT), (__SM)->ht->stride, (__SM)->ht->klpvl)

#define neko_slot_map_iter_getk(__SM, __IT) neko_hash_table_iter_getk((__SM)->ht, (__IT))
//(neko_hash_table_find_valid_iter(__SM->ht, __IT), neko_hash_table_geti((__SM)->ht, (__IT)))

#define neko_slot_map_iter_getkp(__SM, __IT) (neko_hash_table_find_valid_iter(__SM->ht, __IT), &(neko_hash_table_geti((__SM)->ht, (__IT))))

#define neko_slot_map_iter_get(__SM, __IT) ((__SM)->sa->data[neko_hash_table_iter_get((__SM)->ht, (__IT))])

// ((__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))])
// (neko_hash_table_find_valid_iter(__SM->ht, __IT), (__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))])

#define neko_slot_map_iter_getp(__SM, __IT) (&((__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))]))

// (neko_hash_table_find_valid_iter(__SM->ht, __IT), &((__SM)->sa->data[neko_hash_table_geti((__SM)->ht, (__IT))]))

/*===================================
// Command Buffer
===================================*/

typedef struct neko_command_buffer_t {
    u32 num_commands;
    neko_byte_buffer_t commands;
} neko_command_buffer_t;

NEKO_FORCE_INLINE neko_command_buffer_t neko_command_buffer_new() {
    neko_command_buffer_t cb = NEKO_DEFAULT_VAL();
    cb.commands = neko_byte_buffer_new();
    return cb;
}

#define neko_command_buffer_write(__CB, __CT, __C, __T, __VAL) \
    do {                                                       \
        neko_command_buffer_t* __cb = (__CB);                  \
        __cb->num_commands++;                                  \
        neko_byte_buffer_write(&__cb->commands, __CT, (__C));  \
        neko_byte_buffer_write(&__cb->commands, __T, (__VAL)); \
    } while (0)

NEKO_FORCE_INLINE void neko_command_buffer_clear(neko_command_buffer_t* cb) {
    cb->num_commands = 0;
    neko_byte_buffer_clear(&cb->commands);
}

NEKO_FORCE_INLINE void neko_command_buffer_free(neko_command_buffer_t* cb) { neko_byte_buffer_free(&cb->commands); }

#define neko_command_buffer_readc(__CB, __T, __NAME) \
    __T __NAME = NEKO_DEFAULT_VAL();                 \
    neko_byte_buffer_read(&(__CB)->commands, __T, &__NAME);

typedef neko_command_buffer_t neko_cmdbuf;
