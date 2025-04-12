
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
