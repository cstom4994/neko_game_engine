
#if !defined(NEKO_HPP)
#define NEKO_HPP

// #include <intrin.h>

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine/prelude.h"

#define NEKO_VA_COUNT(...) detail::va_count(__VA_ARGS__)

#if defined(__cplusplus)
#include <string>
#if defined(__cpp_char8_t)
template <typename T>
const_str u8Cpp20(T&& t) noexcept {
#pragma warning(disable : 26490)
    return reinterpret_cast<const_str>(t);
#pragma warning(default : 26490)
}
#define NEKO_STR(x) u8Cpp20(u8##x)
#else
#define NEKO_STR(x) u8##x
#endif
#else
#define NEKO_STR(x) x
#endif

#define NEKO_DYNAMIC_CAST(type, input_var, cast_var_name) \
    NEKO_ASSERT(value);                                   \
    type* cast_var_name = dynamic_cast<type*>(input_var); \
    NEKO_ASSERT(cast_var_name)
#define NEKO_STATIC_CAST(type, input_var, cast_var_name) \
    NEKO_ASSERT(value);                                  \
    type* cast_var_name = static_cast<type*>(input_var); \
    NEKO_ASSERT(cast_var_name)

#define NEKO_ENUM_FLAG(T)                                                                                                                                                  \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                     \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T& operator|=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator&=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator^=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define NEKO_MOVEONLY(class_name)                      \
    class_name(const class_name&) = delete;            \
    class_name& operator=(const class_name&) = delete; \
    class_name(class_name&&) = default;                \
    class_name& operator=(class_name&&) = default

namespace detail {
template <typename... Args>
constexpr std::size_t va_count(Args&&...) {
    return sizeof...(Args);
}
}  // namespace detail

#define neko_macro_overload(fun, a, ...)                                               \
    do {                                                                               \
        if (const bool a_ = (a); a_)                                                   \
            [&](auto&&... args) {                                                      \
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
inline void neko_addoffset(std::ptrdiff_t offset, T*& ptr) {
    if (!ptr) return;
    ptr = (T*)((unsigned char*)ptr + offset);
}

template <typename T>
inline T* neko_addoffset_r(std::ptrdiff_t offset, T* ptr) {
    if (!ptr) return nullptr;
    return (T*)((unsigned char*)ptr + offset);
}

template <typename T, size_t size>
class size_checker {
    static_assert(sizeof(T) == size, "check the size of integral types");
};

template class size_checker<i64, 8>;
template class size_checker<i32, 4>;
template class size_checker<i16, 2>;
template class size_checker<i8, 1>;
template class size_checker<u64, 8>;
template class size_checker<u32, 4>;
template class size_checker<u16, 2>;
template class size_checker<u8, 1>;

struct lua_State;

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
using function = std::function<T>;

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
concept concept_is_pair = requires(T t) {
    t.first;
    t.second;
};

template <class T>
struct is_pair : public std::false_type {};

template <class T1, class T2>
struct is_pair<std::pair<T1, T2>> : public std::true_type {};

template <class>
inline constexpr bool always_false = false;

}  // namespace neko

#if __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_debugging)
#include <debugging>
#else
#if defined(_WIN32)
#include <intrin.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#endif

#define NEKO_VA_UNPACK(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

#if defined(__clang__)
#define NEKO_DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__)
#define NEKO_DEBUGBREAK() __builtin_trap()
#elif defined(_MSC_VER)
#define NEKO_DEBUGBREAK() __debugbreak()
#else
#define NEKO_DEBUGBREAK()
#endif

namespace std {
inline void breakpoint() noexcept { NEKO_DEBUGBREAK(); }
inline bool is_debugger_present() noexcept {
#if defined(_WIN32)
    return IsDebuggerPresent() != 0;
#elif defined(__APPLE__)
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);
    info.kp_proc.p_flag = 0;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    if (sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) != 0) {
        return false;
    }
    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    return false;
#endif
}
inline void breakpoint_if_debugging() noexcept {
    if (is_debugger_present()) {
        breakpoint();
    }
}
}  // namespace std

#endif

namespace neko {

#define neko_time_count(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

class timer {
public:
    inline void start() noexcept { startPos = std::chrono::high_resolution_clock::now(); }
    inline void stop() noexcept {
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = static_cast<f64>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] inline f64 get() const noexcept { return duration; }
    ~timer() noexcept { stop(); }

private:
    f64 duration = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
};

NEKO_FORCE_INLINE auto time() -> i64 {
    i64 ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

static inline auto time_to_string(std::time_t now = std::time(nullptr)) -> std::string {
    const auto tp = std::localtime(&now);
    char buffer[32];
    return std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", tp) ? buffer : "1970-01-01_00:00:00";
}

static inline std::string fs_normalize_path(const std::string& path, char delimiter = '/') {
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

}  // namespace neko

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
}  // namespace neko::wtf8

namespace neko::win {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
std::wstring a2w(std::string_view str) noexcept;
std::string w2a(std::wstring_view wstr) noexcept;
std::string a2u(std::string_view str) noexcept;
std::string u2a(std::string_view str) noexcept;
}  // namespace neko::win

template <typename F>
struct function_traits : public function_traits<decltype(&F::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> {
    using return_type = ReturnType;
    using pointer = ReturnType (*)(Args...);
    using std_function = std::function<ReturnType(Args...)>;
};

template <typename F>
typename function_traits<F>::std_function to_function(F& lambda) {
    return typename function_traits<F>::std_function(lambda);
}

namespace neko {

template <typename T>
class moveonly {
public:
    static_assert(std::is_default_constructible_v<T>);
    static_assert(std::is_trivially_copy_constructible_v<T>);
    static_assert(std::is_trivially_copy_assignable_v<T>);
    static_assert(std::is_trivially_move_constructible_v<T>);
    static_assert(std::is_trivially_move_assignable_v<T>);
    static_assert(std::is_swappable_v<T>);

    moveonly() = default;

    ~moveonly() = default;

    explicit moveonly(const T& val) noexcept { m_value = val; }

    moveonly(const moveonly&) = delete;

    moveonly& operator=(const moveonly&) = delete;

    moveonly(moveonly&& other) noexcept { m_value = std::exchange(other.m_value, T{}); }

    moveonly& operator=(moveonly&& other) noexcept {
        std::swap(m_value, other.m_value);
        return *this;
    }

    operator const T&() const { return m_value; }

    moveonly& operator=(const T& val) {
        m_value = val;
        return *this;
    }

    // MoveOnly<T> has the same memory layout as T
    // So it's perfectly safe to overload operator&
    T* operator&() { return &m_value; }

    const T* operator&() const { return &m_value; }

    T* operator->() { return &m_value; }

    const T* operator->() const { return &m_value; }

    bool operator==(const T& val) const { return m_value == val; }

    bool operator!=(const T& val) const { return m_value != val; }

private:
    T m_value{};
};

static_assert(sizeof(int) == sizeof(moveonly<int>));

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

struct format_str {
    constexpr format_str(const char* str) noexcept : str(str) {}
    const char* str;
};

template <format_str F>
constexpr auto operator""_f() {
    return [=]<typename... T>(T... args) { return std::format(F.str, args...); };
}

// 成员函数返回值类型确定
// https://stackoverflow.com/questions/26107041/how-can-i-determine-the-return-type-of-a-c11-member-function

template <typename T>
struct return_type;
template <typename R, typename... Args>
struct return_type<R (*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) volatile> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const volatile> {
    using type = R;
};
template <typename T>
using return_type_t = typename return_type<T>::type;

// std::function 合并方法

template <typename, typename...>
struct lastFnType;

template <typename F0, typename F1, typename... Fn>
struct lastFnType<F0, F1, Fn...> {
    using type = typename lastFnType<F1, Fn...>::type;
};

template <typename T1, typename T2>
struct lastFnType<function<T2(T1)>> {
    using type = T1;
};

template <typename T1, typename T2>
function<T1(T2)> func_combine(function<T1(T2)> conv) {
    return conv;
}

template <typename T1, typename T2, typename T3, typename... Fn>
auto func_combine(function<T1(T2)> conv1, function<T2(T3)> conv2, Fn... fn) -> function<T1(typename lastFnType<function<T2(T3)>, Fn...>::type)> {
    using In = typename lastFnType<function<T2(T3)>, Fn...>::type;

    return [=](In const& in) { return conv1(func_combine(conv2, fn...)(in)); };
}

template <typename T>
struct tuple_size;

template <typename... Args>
struct tuple_size<std::tuple<Args...>> {
    static constexpr std::size_t value = sizeof...(Args);
};

template <typename T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

template <typename T, std::size_t N>
constexpr bool is_pointer_to_const_char(T (&)[N]) {
    return std::is_same_v<const char, T>;
}

template <typename T>
constexpr bool is_pointer_to_const_char(T&&) {
    return std::is_same_v<const char*, T>;
}

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

}  // namespace neko

namespace detail {
// 某些旧版本的 GCC 需要
template <typename...>
struct voider {
    using type = void;
};

// std::void_t 将成为 C++17 的一部分 但在这里我还是自己实现吧
template <typename... T>
using void_t = typename voider<T...>::type;

template <typename T, typename U = void>
struct is_mappish_impl : std::false_type {};

template <typename T>
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])>> : std::true_type {};
}  // namespace detail

template <typename T>
struct neko_is_mappish : detail::is_mappish_impl<T>::type {};

template <class... Ts>
struct neko_overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
neko_overloaded(Ts...) -> neko_overloaded<Ts...>;

namespace neko {

// hash 计算相关函数

typedef unsigned long long hash_value;

static_assert(sizeof(hash_value) == 8 && sizeof(hash_value) == sizeof(size_t));

inline uint64_t hash_fnv(const void* data, int size) {
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

constexpr hash_value hash(char const* input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string& input) { return hash_fnv(input.c_str()); }

template <typename ForwardIterator, typename SpaceDetector>
constexpr ForwardIterator find_terminating_word(ForwardIterator begin, ForwardIterator end, SpaceDetector&& is_space_pred) {
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

// template <typename T>
// struct alloc {
//     template <typename... Args>
//     static T* safe_malloc(Args&&... args) {
//         void* mem = neko_safe_malloc(sizeof(T));
//         if (!mem) {
//         }
//         return new (mem) T(std::forward<Args>(args)...);
//     }
// };

inline bool str_is_chinese_c(const char str) { return str & 0x80; }

inline bool str_is_chinese_str(const std::string& str) {
    for (int i = 0; i < str.length(); i++)
        if (str_is_chinese_c(str[i])) return true;
    return false;
}

inline bool str_equals(const char* a, const char* c) { return strcmp(a, c) == 0; }

inline bool str_starts_with(std::string_view s, std::string_view prefix) { return prefix.size() <= s.size() && (strncmp(prefix.data(), s.data(), prefix.size()) == 0); }

inline bool str_starts_with(std::string_view s, char prefix) { return !s.empty() && s[0] == prefix; }

inline bool str_starts_with(const char* s, const char* prefix) { return strncmp(s, prefix, strlen(prefix)) == 0; }

inline bool str_ends_with(std::string_view s, std::string_view suffix) { return suffix.size() <= s.size() && strncmp(suffix.data(), s.data() + s.size() - suffix.size(), suffix.size()) == 0; }

inline bool str_ends_with(std::string_view s, char suffix) { return !s.empty() && s[s.size() - 1] == suffix; }

inline bool str_ends_with(const char* s, const char* suffix) {
    auto sizeS = strlen(s);
    auto sizeSuf = strlen(suffix);

    return sizeSuf <= sizeS && strncmp(suffix, s + sizeS - sizeSuf, sizeSuf) == 0;
}

inline void str_to_lower(char* s) {
    int l = strlen(s);
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::tolower(s[ind]);
        s[ind + 1] = std::tolower(s[ind + 1]);
        s[ind + 2] = std::tolower(s[ind + 2]);
        s[ind + 3] = std::tolower(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) {
        s[ind++] = std::tolower(s[ind]);
    }
}

inline void str_to_lower(std::string& ss) {
    int l = ss.size();
    auto s = ss.data();
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::tolower(s[ind]);
        s[ind + 1] = std::tolower(s[ind + 1]);
        s[ind + 2] = std::tolower(s[ind + 2]);
        s[ind + 3] = std::tolower(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) s[ind++] = std::tolower(s[ind]);
}

inline void str_to_upper(char* s) {
    int l = strlen(s);
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::toupper(s[ind]);
        s[ind + 1] = std::toupper(s[ind + 1]);
        s[ind + 2] = std::toupper(s[ind + 2]);
        s[ind + 3] = std::toupper(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) {
        s[ind++] = std::toupper(s[ind]);
    }
}

inline void str_to_upper(std::string& ss) {
    int l = ss.size();
    auto s = ss.data();
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::toupper(s[ind]);
        s[ind + 1] = std::toupper(s[ind + 1]);
        s[ind + 2] = std::toupper(s[ind + 2]);
        s[ind + 3] = std::toupper(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) s[ind++] = std::toupper(s[ind]);
}

inline bool str_replace_with(char* src, char what, char with) {
    for (int i = 0; true; ++i) {
        auto& id = src[i];
        if (id == '\0') return true;
        bool isWhat = id == what;
        id = isWhat * with + src[i] * (!isWhat);
    }
}

inline bool str_replace_with(std::string& src, char what, char with) {
    for (int i = 0; i < src.size(); ++i) {
        auto& id = src.data()[i];
        bool isWhat = id == what;
        id = isWhat * with + src[i] * (!isWhat);
    }
    return true;
}

inline bool str_replace_with(std::string& src, const char* what, const char* with) {
    std::string out;
    size_t whatlen = strlen(what);
    out.reserve(src.size());
    size_t ind = 0;
    size_t lastInd = 0;
    while (true) {
        ind = src.find(what, ind);
        if (ind == std::string::npos) {
            out += src.substr(lastInd);
            break;
        }
        out += src.substr(lastInd, ind - lastInd) + with;
        ind += whatlen;
        lastInd = ind;
    }
    src = out;
    return true;
}

inline bool str_replace_with(std::string& src, const char* what, const char* with, int times) {
    for (int i = 0; i < times; ++i) str_replace_with(src, what, with);
    return true;
}

}  // namespace neko

typedef struct engine_cvar_t {
    bool show_editor;
    bool show_demo_window;
    bool show_pack_editor;
    bool show_profiler_window;
    bool show_test_window;
    bool show_gui;
    bool shader_inspect;
    bool hello_ai_shit;
    bool vsync;
    bool is_hotfix;

    // 实验性功能开关
    bool enable_nekolua;

    f32 bg[3];  //
} neko_client_cvar_t;

void neko_cvar_gui(neko_client_cvar_t& cvar);

namespace neko {}  // namespace neko

#endif