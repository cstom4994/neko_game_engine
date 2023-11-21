

#ifndef NEKO_SERIALIZE_HPP
#define NEKO_SERIALIZE_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <deque>
#include <exception>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "engine/neko.h"

class neko_serialize_swap_byte_base {
public:
    static bool should_swap() {
        static const u16 swapTest = 1;
        return (*((char*)&swapTest) == 1);
    }

    static void swap_bytes(u8& v1, u8& v2) {
        u8 tmp = v1;
        v1 = v2;
        v2 = tmp;
    }
};

template <class T, int S>
class neko_serialize_swap_byte : public neko_serialize_swap_byte_base {
public:
    static T swap(T v) {
        neko_assert(false);
        return v;
    }
};

template <class T>
class neko_serialize_swap_byte<T, 1> : public neko_serialize_swap_byte_base {
public:
    static T swap(T v) { return v; }
};

template <class T>
class neko_serialize_swap_byte<T, 2> : public neko_serialize_swap_byte_base {
public:
    static T swap(T v) {
        if (should_swap()) return ((u16)v >> 8) | ((u16)v << 8);
        return v;
    }
};

template <class T>
class neko_serialize_swap_byte<T, 4> : public neko_serialize_swap_byte_base {
public:
    static T swap(T v) {
        if (should_swap()) {
            return (neko_serialize_swap_byte<u16, 2>::swap((u32)v & 0xffff) << 16) | (neko_serialize_swap_byte<u16, 2>::swap(((u32)v & 0xffff0000) >> 16));
        }
        return v;
    }
};

template <class T>
class neko_serialize_swap_byte<T, 8> : public neko_serialize_swap_byte_base {
public:
    static T swap(T v) {
        if (should_swap()) return (((uint64_t)neko_serialize_swap_byte<u32, 4>::swap((u32)(v & 0xffffffffull))) << 32) | (neko_serialize_swap_byte<u32, 4>::swap((u32)(v >> 32)));
        return v;
    }
};

template <>
class neko_serialize_swap_byte<float, 4> : public neko_serialize_swap_byte_base {
public:
    static float swap(float v) {
        union {
            float f;
            u8 c[4];
        };
        f = v;
        if (should_swap()) {
            swap_bytes(c[0], c[3]);
            swap_bytes(c[1], c[2]);
        }
        return f;
    }
};

template <>
class neko_serialize_swap_byte<double, 8> : public neko_serialize_swap_byte_base {
public:
    static double swap(double v) {
        union {
            double f;
            u8 c[8];
        };
        f = v;
        if (should_swap()) {
            swap_bytes(c[0], c[7]);
            swap_bytes(c[1], c[6]);
            swap_bytes(c[2], c[5]);
            swap_bytes(c[3], c[4]);
        }
        return f;
    }
};

template <class T>
class neko_serialize {
public:
    neko_serialize(T& stream) : stream(stream) {}

public:
    template <class S>
    const neko_serialize& operator<<(const S& v) const {
        *this& v;
        return *this;
    }

    template <class S>
    neko_serialize& operator>>(S& v) {
        *this& v;
        return *this;
    }

public:
    template <class S>
    neko_serialize& operator&(S& v) {
        v.__neko_serialize(*this);
        return *this;
    }

    template <class S>
    const neko_serialize& operator&(const S& v) const {
        ((S&)v).__neko_serialize(*this);
        return *this;
    }

    template <class S, size_t N>
    neko_serialize& operator&(S (&v)[N]) {
        u32 len;
        *this& len;
        for (size_t i = 0; i < N; ++i) *this& v[i];
        return *this;
    }

    template <class S, size_t N>
    const neko_serialize& operator&(const S (&v)[N]) const {
        u32 len = N;
        *this& len;
        for (size_t i = 0; i < N; ++i) *this& v[i];
        return *this;
    }

#define SERIALIZER_FOR_POD(type)                        \
    neko_serialize& operator&(type& v) {                \
        stream.read((char*)&v, sizeof(type));           \
        if (!stream) {                                  \
            throw std::runtime_error("malformed data"); \
        }                                               \
        v = swap(v);                                    \
        return *this;                                   \
    }                                                   \
    const neko_serialize& operator&(type v) const {     \
        v = swap(v);                                    \
        stream.write((const char*)&v, sizeof(type));    \
        return *this;                                   \
    }

// 浮点值的特殊序列化器，不能直接交换浮点变量
// 因为它可能会导致 NaN 并读回错误的值（问题出现在 vstudio 上）
#define SERIALIZER_FOR_POD_FLOATINGPOINT(type)                                                                                   \
    neko_serialize& operator&(type& v) {                                                                                         \
        union {                                                                                                                  \
            type f;                                                                                                              \
            u8 c[sizeof(type)];                                                                                                  \
        };                                                                                                                       \
        stream.read((char*)&c[0], sizeof(type));                                                                                 \
        if (!stream) {                                                                                                           \
            throw std::runtime_error("malformed data");                                                                          \
        }                                                                                                                        \
        if (neko_serialize_swap_byte_base::should_swap()) {                                                                      \
            for (int i = 0; i < sizeof(type) / 2; ++i) neko_serialize_swap_byte_base::swap_bytes(c[i], c[sizeof(type) - 1 - i]); \
        }                                                                                                                        \
        v = f;                                                                                                                   \
        return *this;                                                                                                            \
    }                                                                                                                            \
    const neko_serialize& operator&(type v) const {                                                                              \
        union {                                                                                                                  \
            type f;                                                                                                              \
            u8 c[sizeof(type)];                                                                                                  \
        };                                                                                                                       \
        f = v;                                                                                                                   \
        if (neko_serialize_swap_byte_base::should_swap()) {                                                                      \
            for (int i = 0; i < sizeof(type) / 2; ++i) neko_serialize_swap_byte_base::swap_bytes(c[i], c[sizeof(type) - 1 - i]); \
        }                                                                                                                        \
        stream.write((const char*)&c[0], sizeof(type));                                                                          \
        return *this;                                                                                                            \
    }

    SERIALIZER_FOR_POD(bool)
    SERIALIZER_FOR_POD(char)
    SERIALIZER_FOR_POD(unsigned char)
    SERIALIZER_FOR_POD(short)
    SERIALIZER_FOR_POD(unsigned short)
    SERIALIZER_FOR_POD(int)
    SERIALIZER_FOR_POD(unsigned int)
    SERIALIZER_FOR_POD(long)
    SERIALIZER_FOR_POD(unsigned long)
    SERIALIZER_FOR_POD(long long)
    SERIALIZER_FOR_POD(unsigned long long)
    SERIALIZER_FOR_POD_FLOATINGPOINT(float)
    SERIALIZER_FOR_POD_FLOATINGPOINT(double)

#define SERIALIZER_FOR_STL(type)                                                               \
    template <class S>                                                                         \
    neko_serialize& operator&(type<S>& v) {                                                    \
        u32 len;                                                                               \
        *this& len;                                                                            \
        for (u32 i = 0; i < len; ++i) {                                                        \
            S value;                                                                           \
            *this& value;                                                                      \
            v.insert(v.end(), value);                                                          \
        }                                                                                      \
        return *this;                                                                          \
    }                                                                                          \
    template <class S>                                                                         \
    const neko_serialize& operator&(const type<S>& v) const {                                  \
        u32 len = v.size();                                                                    \
        *this& len;                                                                            \
        for (typename type<S>::const_iterator it = v.begin(); it != v.end(); ++it) *this&* it; \
        return *this;                                                                          \
    }

#define SERIALIZER_FOR_STL2(type)                                                                   \
    template <class T1, class T2>                                                                   \
    neko_serialize& operator&(type<T1, T2>& v) {                                                    \
        u32 len;                                                                                    \
        *this& len;                                                                                 \
        for (u32 i = 0; i < len; ++i) {                                                             \
            std::pair<T1, T2> value;                                                                \
            *this& value;                                                                           \
            v.insert(v.end(), value);                                                               \
        }                                                                                           \
        return *this;                                                                               \
    }                                                                                               \
    template <class T1, class T2>                                                                   \
    const neko_serialize& operator&(const type<T1, T2>& v) const {                                  \
        u32 len = v.size();                                                                         \
        *this& len;                                                                                 \
        for (typename type<T1, T2>::const_iterator it = v.begin(); it != v.end(); ++it) *this&* it; \
        return *this;                                                                               \
    }

    SERIALIZER_FOR_STL(std::vector)
    SERIALIZER_FOR_STL(std::deque)
    SERIALIZER_FOR_STL(std::list)
    SERIALIZER_FOR_STL(std::set)
    SERIALIZER_FOR_STL(std::multiset)
    SERIALIZER_FOR_STL2(std::map)
    SERIALIZER_FOR_STL2(std::multimap)

    template <class T1, class T2>
    neko_serialize& operator&(std::pair<T1, T2>& v) {
        *this& v.first& v.second;
        return *this;
    }

    template <class T1, class T2>
    const neko_serialize& operator&(const std::pair<T1, T2>& v) const {
        *this& v.first& v.second;
        return *this;
    }

    neko_serialize& operator&(std::string& v) {
        u32 len;
        *this& len;
        v.clear();
        char buffer[4096];
        u32 to_read = len;
        while (to_read != 0) {
            u32 l = std::min(to_read, (u32)sizeof(buffer));
            stream.read(buffer, l);
            if (!stream) throw std::runtime_error("malformed data");
            v += std::string(buffer, l);
            to_read -= l;
        }
        return *this;
    }

    const neko_serialize& operator&(const std::string& v) const {
        u32 len = v.length();
        *this& len;
        stream.write(v.c_str(), len);
        return *this;
    }

private:
    template <class S>
    S swap(const S& v) const {
        return neko_serialize_swap_byte<S, sizeof(S)>::swap(v);
    }

private:
    T& stream;
};

#endif  // NEKO_SERIALIZE_HPP
