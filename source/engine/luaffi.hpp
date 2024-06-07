//
// The MIT License
//
// Copyright (c) 2020 Daniel "q66" Kolesa
// Copyright (c) 2024 KaoruXun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// https://github.com/q66/cffi-lua
//

#ifndef LUAFFI_HPP
#define LUAFFI_HPP

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>

/* OS; defined to be luajit compatible
 *
 * If undetected, it will still work, but the OS will be "Other"
 */

#define FFI_OS_OTHER 0
#define FFI_OS_WINDOWS 1
#define FFI_OS_LINUX 2
#define FFI_OS_OSX 3
#define FFI_OS_BSD 4
#define FFI_OS_POSIX 5

#if defined(_WIN32) && !defined(_XBOX_VER)
#define FFI_OS FFI_OS_WINDOWS
#define FFI_OS_NAME "Windows"
#elif defined(__linux__)
#define FFI_OS FFI_OS_LINUX
#define FFI_OS_NAME "Linux"
#elif defined(__MACH__) && defined(__APPLE__)
#define FFI_OS FFI_OS_OSX
#define FFI_OS_NAME "OSX"
#elif (defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)) && !defined(__ORBIS__)
#define FFI_OS FFI_OS_BSD
#define FFI_OS_NAME "BSD"
#elif (defined(__sun__) && defined(__svr4__)) || defined(__HAIKU__) || defined(__CYGWIN__)
#define FFI_OS FFI_OS_POSIX
#define FFI_OS_NAME "POSIX"
#ifdef __CYGWIN__
#define FFI_OS_CYGWIN 1
#endif
#else
#define FFI_OS FFI_OS_OTHER
#define FFI_OS_NAME "Other"
#endif

#if !defined(FFI_BIG_ENDIAN) && !defined(FFI_LITTLE_ENDIAN)
#error "Unknown machine endianness"
#elif defined(FFI_BIG_ENDIAN) && defined(FFI_LITTLE_ENDIAN)
#error "Choose just one endianness"
#endif

/* Arch; defined to be luajit compatible
 *
 * IF undetected, it will still work, but the arch will be "unknown"
 */

#define FFI_ARCH_UNKNOWN 0
#define FFI_ARCH_X86 1
#define FFI_ARCH_X64 2
#define FFI_ARCH_ARM 3
#define FFI_ARCH_ARM64 4
#define FFI_ARCH_PPC 5
#define FFI_ARCH_PPC64 6
#define FFI_ARCH_MIPS32 7
#define FFI_ARCH_MIPS64 8
/* these architectures are not defined in luajit */
#define FFI_ARCH_ALPHA 9
#define FFI_ARCH_HPPA 10
#define FFI_ARCH_IA64 11
#define FFI_ARCH_M68K 12
#define FFI_ARCH_MBLAZE 13
#define FFI_ARCH_OR1K 14
#define FFI_ARCH_RV32 15
#define FFI_ARCH_RV64 16
#define FFI_ARCH_SH4 17
#define FFI_ARCH_SPARC 18
#define FFI_ARCH_SPARC64 19
#define FFI_ARCH_S390 20

#define FFI_CPU(arch) (FFI_ARCH == FFI_ARCH_##arch)

#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define FFI_ARCH FFI_ARCH_X86
#define FFI_ARCH_NAME "x86"
#elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define FFI_ARCH FFI_ARCH_X64
#define FFI_ARCH_NAME "x64"
#elif defined(__arm__) || defined(__arm) || defined(__ARM__) || defined(__ARM)
#define FFI_ARCH FFI_ARCH_ARM
#if defined(FFI_BIG_ENDIAN)
#define FFI_ARCH_NAME "armeb"
#else
#define FFI_ARCH_NAME "arm"
#endif
#ifdef __SOFTFP__
#define FFI_ARCH_HAS_FPU 0
#endif
#if !__ARM_PCS_VFP
#define FFI_ARCH_SOFTFP 1
#endif
#elif defined(__aarch64__)
#define FFI_ARCH FFI_ARCH_ARM64
#if defined(FFI_BIG_ENDIAN)
#define FFI_ARCH_NAME "arm64be"
#else
#define FFI_ARCH_NAME "arm64"
#endif
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(__powerpc64) || defined(__ppc64) || defined(__PPC64) || defined(__POWERPC64__) || defined(__POWERPC64) || \
        defined(_M_PPC64)
#define FFI_ARCH FFI_ARCH_PPC64
#if defined(_CALL_ELF) && (_CALL_ELF == 2)
#define FFI_ARCH_PPC64_ELFV2 1
#endif
#if defined(FFI_LITTLE_ENDIAN)
#define FFI_ARCH_NAME "ppc64le"
#else
#define FFI_ARCH_NAME "ppc64"
#endif
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || defined(__powerpc) || defined(__ppc) || defined(__PPC) || defined(__POWERPC__) || defined(__POWERPC) || defined(_M_PPC)
#define FFI_ARCH FFI_ARCH_PPC
#if defined(FFI_LITTLE_ENDIAN)
#define FFI_ARCH_NAME "ppcle"
#else
#define FFI_ARCH_NAME "ppc"
#endif
#elif defined(__mips64__) || defined(__mips64) || defined(__MIPS64__) || defined(__MIPS64)
#define FFI_ARCH FFI_ARCH_MIPS64
#if __mips_isa_rev >= 6
#if defined(FFI_LITTLE_ENDIAN)
#define FFI_ARCH_NAME "mips64r6el"
#else
#define FFI_ARCH_NAME "mips64r6"
#endif
#else
#if defined(FFI_LITTLE_ENDIAN)
#define FFI_ARCH_NAME "mips64el"
#else
#define FFI_ARCH_NAME "mips64"
#endif
#endif
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(__MIPS)
#define FFI_ARCH FFI_ARCH_MIPS32
#if __mips_isa_rev >= 6
#if defined(FFI_LITTLE_ENDIAN)
#define FFI_ARCH_NAME "mips32r6el"
#else
#define FFI_ARCH_NAME "mips32r6"
#endif
#else
#if defined(FFI_LITTLE_ENDIAN)
#define FFI_ARCH_NAME "mipsel"
#else
#define FFI_ARCH_NAME "mips"
#endif
#endif
#ifdef __mips_soft_float
#define FFI_ARCH_SOFTFP 1
#define FFI_ARCH_HAS_FPU 0
#else
#define FFI_ARCH_SOFTFP 0
#define FFI_ARCH_HAS_FPU 1
#endif
#elif defined(__alpha__) || defined(__alpha)
#define FFI_ARCH FFI_ARCH_ALPHA
#define FFI_ARCH_NAME "alpha"
#elif defined(__hppa__) || defined(__HPPA__) || defined(__hppa)
#define FFI_ARCH FFI_ARCH_HPPA
#define FFI_ARCH_NAME "hppa"
#elif defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64) || defined(__itanium__)
#define FFI_ARCH FFI_ARCH_IA64
#define FFI_ARCH_NAME "ia64"
#elif defined(__m68k__) || defined(__MC68K__)
#define FFI_ARCH FFI_ARCH_M68K
#define FFI_ARCH_NAME "m68k"
#elif defined(__MICROBLAZE__)
#define FFI_ARCH FFI_ARCH_MBLAZE
#if defined(__MICROBLAZEEL__)
#define FFI_ARCH_NAME "microblazeel"
#else
#define FFI_ARCH_NAME "microblaze"
#endif
#elif defined(__OR1K__)
#define FFI_ARCH FFI_ARCH_OR1K
#define FFI_ARCH_NAME "or1k"
#elif defined(__riscv) || defined(__riscv__)
#if __riscv_xlen == 32
#define FFI_ARCH FFI_ARCH_RV32
#define FFI_ARCH_NAME "riscv32"
#else
#define FFI_ARCH FFI_ARCH_RV64
#define FFI_ARCH_NAME "riscv64"
#endif
#ifdef __riscv_float_abi_soft
#define FFI_ARCH_SOFTFP 1
#define FFI_ARCH_HAS_FPU 0
#else
#define FFI_ARCH_SOFTFP 0
#define FFI_ARCH_HAS_FPU 1
#endif
#elif defined(__sh__) && defined(__SH4__)
#define FFI_ARCH FFI_ARCH_SH4
#if defined(FFI_BIG_ENDIAN)
#define FFI_ARCH_NAME "sh4eb"
#else
#define FFI_ARCH_NAME "sh4"
#endif
#elif defined(__sparc__) || defined(__sparc)
#if defined(__sparc_v9__) || defined(__sparcv9) || defined(__arch64__)
#define FFI_ARCH FFI_ARCH_SPARC64
#define FFI_ARCH_NAME "sparc64"
#else
#define FFI_ARCH FFI_ARCH_SPARC
#define FFI_ARCH_NAME "sparc"
#endif
#elif defined(__s390__) || defined(__s390x__) || defined(__zarch__)
#define FFI_ARCH FFI_ARCH_S390
#if defined(__s390x__)
#define FFI_ARCH_NAME "s390x"
#else
#define FFI_ARCH_NAME "s390"
#endif
#else
#define FFI_ARCH FFI_ARCH_UNKNOWN
#define FFI_ARCH_NAME "unknown"
#endif

#if FFI_ARCH == FFI_ARCH_ARM && defined(__ARM_EABI__)
#define FFI_ARM_EABI 1
#endif

#if FFI_ARCH == FFI_ARCH_PPC && defined(__NO_FPRS__) && !defined(_SOFT_FLOAT)
#define FFI_PPC_SPE 1
#endif

#ifndef FFI_ARCH_HAS_FPU
#if defined(_SOFT_FLOAT) || defined(_SOFT_DOUBLE)
#define FFI_ARCH_HAS_FPU 0
#else
#define FFI_ARCH_HAS_FPU 1
#endif
#endif

#ifndef FFI_ARCH_SOFTFP
#if defined(_SOFT_FLOAT) || defined(_SOFT_DOUBLE)
#define FFI_ARCH_SOFTFP 1
#else
#define FFI_ARCH_SOFTFP 0
#endif
#endif

#if FFI_OS == FFI_OS_WINDOWS || defined(FFI_OS_CYGWIN)
#define FFI_WINDOWS_ABI 1
#endif

#ifdef _UWP
#define FFI_WINDOWS_UWP 1
#error "UWP support not yet implemented"
#endif

#if FFI_OS != FFI_OS_WINDOWS
#define FFI_USE_DLFCN 1
#endif

/* abi-specific features */

/* passing unions by value:
 *
 * - all 32-bit x86 except windows fastcall passes values on the stack
 * - windows fastcall may pass some in regs but always the same ones
 * - windows x64 ABI doesn't care about union contents for passing
 * - arm and aarch64 - composite types go in GPRs or on the stack
 * - ppc and ppc64 - composite types go in GPRs or on the stack
 * - mips - unions are like structs, structs go in GPRs or on the stack
 *
 * every other ABI is for now forbidden from passing unions by value since
 * it is not known whether it is safe to do so; usually this would need some
 * manual handling as the type of register used for passing may depend on the
 * type being passed (i.e. same-size same-alignment unions with different
 * fields may use different registers)
 *
 * aarch64 and ppc64le have a concept of homogenous aggregates, which means
 * unions may occasionally go in FPRs/VRRs; this is handled explicitly in
 * our implementation
 *
 * ppc64 and aarch64 currently disabled - buggy
 */
#if FFI_CPU(X86) || FFI_CPU(ARM) || FFI_CPU(PPC) || FFI_CPU(MIPS32) || FFI_CPU(MIPS64)
#define FFI_ABI_UNIONVAL 1
#elif defined(FFI_WINDOWS_ABI) && (FFI_ARCH == FFI_ARCH_X64)
#define FFI_ABI_UNIONVAL 1
#endif

/* some compiler bits */

#if defined(__GNUC__)
#if (__GNUC__ >= 5) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))
#define FFI_DIAGNOSTIC_PRAGMA_GCC 1
#endif
#define WARN_UNUSED_RET __attribute__((warn_unused_result))
#define RESTRICT __restrict__
#else
#define WARN_UNUSED_RET
#define RESTRICT
#endif

#if defined(__clang__)
#define FFI_DIAGNOSTIC_PRAGMA_CLANG 1
#endif

/* MSVC warnings we don't care about/are irrelevant to us */

#ifdef _MSC_VER
/* conditional expression is constant */
#pragma warning(disable : 4127)
/* unsafe CRT, used only once */
#pragma warning(disable : 4996)
#endif

/* MSVC and clang */

#define _CRT_SECURE_NO_WARNINGS 1

#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

/* allocation */

inline void *operator new(std::size_t, void *p) noexcept { return p; }
inline void *operator new[](std::size_t, void *p) noexcept { return p; }
inline void operator delete(void *, void *) noexcept {}
inline void operator delete[](void *, void *) noexcept {}

namespace util {

/* type traits */

namespace detail {
template <typename T>
struct remove_ref {
    using type = T;
};
template <typename T>
struct remove_ref<T &> {
    using type = T;
};
template <typename T>
struct remove_ref<T &&> {
    using type = T;
};
}  // namespace detail

template <typename T>
using remove_ref_t = typename detail::remove_ref<T>::type;

namespace detail {
template <typename T>
struct remove_const {
    using type = T;
};
template <typename T>
struct remove_const<T const> {
    using type = T;
};

template <typename T>
struct remove_volatile {
    using type = T;
};
template <typename T>
struct remove_volatile<T volatile> {
    using type = T;
};
}  // namespace detail

template <typename T>
using remove_const_t = typename detail::remove_const<T>::type;
template <typename T>
using remove_volatile_t = typename detail::remove_volatile<T>::type;
template <typename T>
using remove_cv_t = typename detail::remove_volatile<typename detail::remove_const<T>::type>::type;

namespace detail {
template <bool B, typename T, typename F>
struct conditional {
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};
}  // namespace detail

template <bool B, typename T, typename F>
using conditional_t = typename detail::conditional<B, T, F>::type;

/* these need to be adjusted if a larger type support
 * is added, e.g. 128-bit integers/floats and so on
 */
using max_aligned_t = conditional_t<(alignof(long double) > alignof(long long)), long double, long long>;

using biggest_t = conditional_t<(sizeof(long double) > sizeof(long long)), long double, long long>;

namespace detail {
template <typename>
struct integral {
    static constexpr bool value = false;
};
template <>
struct integral<bool> {
    static constexpr bool value = true;
};
template <>
struct integral<char> {
    static constexpr bool value = true;
};
template <>
struct integral<char16_t> {
    static constexpr bool value = true;
};
template <>
struct integral<char32_t> {
    static constexpr bool value = true;
};
template <>
struct integral<wchar_t> {
    static constexpr bool value = true;
};
template <>
struct integral<short> {
    static constexpr bool value = true;
};
template <>
struct integral<int> {
    static constexpr bool value = true;
};
template <>
struct integral<long> {
    static constexpr bool value = true;
};
template <>
struct integral<long long> {
    static constexpr bool value = true;
};
template <>
struct integral<signed char> {
    static constexpr bool value = true;
};
template <>
struct integral<unsigned char> {
    static constexpr bool value = true;
};
template <>
struct integral<unsigned short> {
    static constexpr bool value = true;
};
template <>
struct integral<unsigned int> {
    static constexpr bool value = true;
};
template <>
struct integral<unsigned long> {
    static constexpr bool value = true;
};
template <>
struct integral<unsigned long long> {
    static constexpr bool value = true;
};
}  // namespace detail

template <typename T>
struct is_int {
    static constexpr bool value = detail::integral<remove_cv_t<T>>::value;
};

namespace detail {
template <typename>
struct fpoint {
    static constexpr bool value = false;
};
template <>
struct fpoint<float> {
    static constexpr bool value = true;
};
template <>
struct fpoint<double> {
    static constexpr bool value = true;
};
template <>
struct fpoint<long double> {
    static constexpr bool value = true;
};
}  // namespace detail

template <typename T>
struct is_float {
    static constexpr bool value = detail::fpoint<remove_cv_t<T>>::value;
};

template <typename T>
struct is_arith {
    static constexpr bool value = is_int<T>::value || is_float<T>::value;
};

template <typename T, bool = is_arith<T>::value>
struct is_signed {
    static constexpr bool value = T(-1) < T(0);
};

template <typename T>
struct is_signed<T, false> {
    static constexpr bool value = false;
};

/* move semantics */

template <typename T>
constexpr inline remove_ref_t<T> &&move(T &&v) noexcept {
    return static_cast<remove_ref_t<T> &&>(v);
}

template <typename T>
constexpr inline T &&forward(remove_ref_t<T> &v) noexcept {
    return static_cast<T &&>(v);
}

template <typename T>
constexpr inline T &&forward(remove_ref_t<T> &&v) noexcept {
    return static_cast<T &&>(v);
}

/* assorted utils */

template <typename T, typename U = T>
constexpr inline T exchange(T &v, U &&nv) {
    T ov = move(v);
    v = forward<U>(nv);
    return ov;
}

template <typename T>
inline void swap(T &a, T &b) {
    auto o = move(a);
    a = move(b);
    b = move(o);
}

template <typename T>
inline T min(T a, T b) {
    return (a < b) ? a : b;
}

template <typename T>
inline T max(T a, T b) {
    return (a > b) ? a : b;
}

/* safe punning */

template <typename T, typename U>
inline T pun(U val) {
    T ret;
    std::memcpy(&ret, &val, sizeof(ret));
    return ret;
}

/* basic limits interface */

namespace detail {
template <typename T>
struct int_limits;

template <>
struct int_limits<char> {
    static constexpr char min = CHAR_MIN;
    static constexpr char max = CHAR_MAX;
};

template <>
struct int_limits<signed char> {
    static constexpr signed char min = SCHAR_MIN;
    static constexpr signed char max = SCHAR_MAX;
};

template <>
struct int_limits<unsigned char> {
    static constexpr unsigned char min = 0;
    static constexpr unsigned char max = UCHAR_MAX;
};

template <>
struct int_limits<short> {
    static constexpr short min = SHRT_MIN;
    static constexpr short max = SHRT_MAX;
};

template <>
struct int_limits<unsigned short> {
    static constexpr unsigned short min = 0;
    static constexpr unsigned short max = USHRT_MAX;
};

template <>
struct int_limits<int> {
    static constexpr int min = INT_MIN;
    static constexpr int max = INT_MAX;
};

template <>
struct int_limits<unsigned int> {
    static constexpr unsigned int min = 0;
    static constexpr unsigned int max = UINT_MAX;
};

template <>
struct int_limits<long> {
    static constexpr long min = LONG_MIN;
    static constexpr long max = LONG_MAX;
};

template <>
struct int_limits<unsigned long> {
    static constexpr unsigned long min = 0;
    static constexpr unsigned long max = ULONG_MAX;
};

template <>
struct int_limits<long long> {
    static constexpr long long min = LLONG_MIN;
    static constexpr long long max = LLONG_MAX;
};

template <>
struct int_limits<unsigned long long> {
    static constexpr unsigned long long min = 0;
    static constexpr unsigned long long max = ULLONG_MAX;
};

template <typename T, bool I, bool F>
struct limits_base {};

template <typename T>
struct limits_base<T, true, false> {
    static constexpr int radix = 2;
    static constexpr int digits = CHAR_BIT * sizeof(T) - is_signed<T>::value;
    static constexpr T min = int_limits<T>::min;
    static constexpr T max = int_limits<T>::max;
};

template <>
struct limits_base<float, false, true> {
    static constexpr int radix = FLT_RADIX;
    static constexpr int digits = FLT_MANT_DIG;
    static constexpr float min = FLT_MIN;
    static constexpr float max = FLT_MAX;
};

template <>
struct limits_base<double, false, true> {
    static constexpr int radix = FLT_RADIX;
    static constexpr int digits = DBL_MANT_DIG;
    static constexpr double min = DBL_MIN;
    static constexpr double max = DBL_MAX;
};

template <>
struct limits_base<long double, false, true> {
    static constexpr int radix = FLT_RADIX;
    static constexpr int digits = LDBL_MANT_DIG;
    static constexpr long double min = LDBL_MIN;
    static constexpr long double max = LDBL_MAX;
};

template <typename T>
struct limits : limits_base<T, is_int<T>::value, is_float<T>::value> {};
}  // namespace detail

template <typename T>
inline constexpr int limit_radix() {
    return detail::limits<T>::radix;
}

template <typename T>
inline constexpr int limit_digits() {
    return detail::limits<T>::digits;
}

template <typename T>
inline constexpr T limit_min() {
    return detail::limits<T>::min;
}

template <typename T>
inline constexpr T limit_max() {
    return detail::limits<T>::max;
}

/* simple writers for base 10 to avoid printf family */

std::size_t write_i(char *buf, std::size_t bufsize, long long v);
std::size_t write_u(char *buf, std::size_t bufsize, unsigned long long v);

/* a simple helper to align pointers to what we can represent */

inline void *ptr_align(void *p) {
    auto *up = static_cast<unsigned char *>(p);
    auto mod = pun<std::uintptr_t>(up) % alignof(max_aligned_t);
    if (mod) {
        up += alignof(max_aligned_t) - mod;
    }
    return up;
}

/* a refernce counted object; manages its own memory,
 * so it can avoid separately allocating the refcount
 */

template <typename T>
struct rc_obj {
    struct construct {};

    template <typename... A>
    rc_obj(construct, A &&...cargs) {
        auto *np = new unsigned char[sizeof(T) + get_rc_size()];
        /* initial acquire */
        *pun<std::size_t *>(np) = 1;
        /* store */
        np += get_rc_size();
        p_ptr = pun<T *>(np);
        /* construct */
        new (p_ptr) T(util::forward<A>(cargs)...);
    }

    rc_obj(rc_obj const &op) : p_ptr{op.p_ptr} { incr(); }

    ~rc_obj() { decr(); }

    rc_obj &operator=(rc_obj op) {
        swap(op);
        return *this;
    }

    T &operator*() const { return *p_ptr; }

    T *operator->() const { return p_ptr; }

    T *get() const { return p_ptr; }

    explicit operator bool() const { return (count() > 0); }

    std::size_t count() const { return *counter(); }

    bool unique() const { return (count() == 1); }

    void release() { decr(); }

    void swap(rc_obj &op) { util::swap(p_ptr, op.p_ptr); }

private:
    static constexpr std::size_t get_rc_size() { return (alignof(T) > sizeof(std::size_t)) ? alignof(T) : sizeof(std::size_t); }

    std::size_t *counter() const { return pun<std::size_t *>(pun<unsigned char *>(p_ptr) - get_rc_size()); }

    void incr() { ++*counter(); }

    void decr() {
        auto *ptr = counter();
        if (!--*ptr) {
            p_ptr->~T();
            delete[] pun<unsigned char *>(ptr);
        }
    }

    T *p_ptr;
};

template <typename T, typename... A>
rc_obj<T> make_rc(A &&...args) {
    return rc_obj<T>{typename rc_obj<T>::construct{}, util::forward<A>(args)...};
}

/* vector */

template <typename T>
struct vector {
    static constexpr std::size_t MIN_SIZE = 4;

    vector() {}
    ~vector() { drop(); }

    vector(vector const &v) { *this = v; }
    vector(vector &&v) { *this = move(v); }

    vector &operator=(vector const &v) {
        shrink(0);
        if (v.size() > capacity()) {
            reserve(v.size());
        }
        for (std::size_t i = 0; i < v.size(); ++i) {
            push_back(v[i]);
        }
        return *this;
    }

    vector &operator=(vector &&v) {
        swap(v);
        return *this;
    }

    void push_back(T const &v) {
        reserve(p_cap + 1);
        new (&p_buf[p_size++]) T(v);
    }

    void push_back(T &&v) {
        reserve(p_cap + 1);
        new (&p_buf[p_size++]) T(util::move(v));
    }

    void pop_back() {
        p_buf[p_size - 1].~T();
        --p_size;
    }

    template <typename... A>
    T &emplace_back(A &&...args) {
        reserve(p_cap + 1);
        new (&p_buf[p_size]) T(util::forward<A>(args)...);
        return p_buf[p_size++];
    }

    void reserve(std::size_t n) {
        if (n <= p_cap) {
            return;
        }
        if (n < MIN_SIZE) {
            n = MIN_SIZE;
        }
        T *np = pun<T *>(new unsigned char[n * sizeof(T)]);
        if (p_cap) {
            for (std::size_t i = 0; i < p_size; ++i) {
                new (&np[i]) T(util::move(p_buf[i]));
                p_buf[i].~T();
            }
            delete[] pun<unsigned char *>(p_buf);
        }
        p_buf = np;
        p_cap = n;
    }

    void shrink(std::size_t n) {
        for (std::size_t i = n; i < p_size; ++i) {
            p_buf[i].~T();
        }
        p_size = n;
    }

    void clear() { shrink(0); }

    T &operator[](std::size_t i) { return p_buf[i]; }

    T const &operator[](std::size_t i) const { return p_buf[i]; }

    T &front() { return p_buf[0]; }

    T const &front() const { return p_buf[0]; }

    T &back() { return p_buf[p_size - 1]; }

    T const &back() const { return p_buf[p_size - 1]; }

    T *data() { return p_buf; }

    T const *data() const { return p_buf; }

    std::size_t size() const { return p_size; }

    std::size_t capacity() const { return p_cap; }

    bool empty() const { return p_size == 0; }

    void swap(vector &v) {
        util::swap(p_buf, v.p_buf);
        util::swap(p_size, v.p_size);
        util::swap(p_cap, v.p_cap);
    }

    void setlen(std::size_t len) { p_size = len; }

    void setbuf(T const *data, std::size_t len) {
        std::memcpy(p_buf, data, len);
        p_size = len;
    }

private:
    void drop() {
        shrink(0);
        delete[] pun<unsigned char *>(p_buf);
    }

    T *p_buf = nullptr;
    std::size_t p_size = 0, p_cap = 0;
};

/* string buffer */

struct strbuf {
    strbuf() {
        /* always terminated */
        p_buf.push_back('\0');
    }

    strbuf(strbuf const &b) { set(b); }

    strbuf(strbuf &&b) : p_buf(util::move(b.p_buf)) {}

    strbuf(char const *str, std::size_t n) { set(str, n); }

    strbuf(char const *str) : strbuf(str, std::strlen(str)) {}

    ~strbuf() {}

    strbuf &operator=(char const *str) {
        set(str, std::strlen(str));
        return *this;
    }

    strbuf &operator=(strbuf const &b) {
        set(b);
        return *this;
    }

    strbuf &operator=(strbuf &&b) {
        p_buf = util::move(b.p_buf);
        return *this;
    }

    void push_back(char c) {
        p_buf.back() = c;
        p_buf.push_back('\0');
    }

    void pop_back() {
        p_buf.pop_back();
        p_buf.back() = '\0';
    }

    void append(char const *str, std::size_t n) {
        auto sz = p_buf.size();
        p_buf.reserve(sz + n);
        std::memcpy(&p_buf[sz - 1], str, n);
        p_buf[n + sz - 1] = '\0';
        p_buf.setlen(sz + n);
    }

    void append(char const *str) { append(str, std::strlen(str)); }

    void append(char c) { push_back(c); }

    void append(strbuf const &b) { append(b.data(), b.size()); }

    void prepend(char const *str, std::size_t n) {
        auto sz = p_buf.size();
        p_buf.reserve(sz + n);
        std::memmove(&p_buf[n], &p_buf[0], sz);
        std::memcpy(&p_buf[0], str, n);
        p_buf.setlen(sz + n);
    }

    void prepend(char const *str) { prepend(str, std::strlen(str)); }

    void prepend(char c) {
        auto sz = p_buf.size();
        p_buf.reserve(sz + 1);
        std::memmove(&p_buf[1], &p_buf[0], sz);
        p_buf[0] = c;
        p_buf.setlen(sz + 1);
    }

    void prepend(strbuf const &b) { prepend(b.data(), b.size()); }

    void insert(char const *str, std::size_t n, std::size_t idx) {
        auto sz = p_buf.size();
        p_buf.reserve(sz + n);
        std::memmove(&p_buf[idx + n], &p_buf[idx], sz - idx);
        std::memcpy(&p_buf[idx], str, n);
        p_buf.setlen(sz + n);
    }

    void insert(char const *str, std::size_t idx) { insert(str, std::strlen(str), idx); }

    void insert(strbuf const &b, std::size_t idx) { insert(b.data(), b.size(), idx); }

    void remove(size_t idx, size_t n = 1) { std::memmove(&p_buf[idx], &p_buf[idx + n], p_buf.size() + 1 - idx - n); }

    void set(char const *str, std::size_t n) {
        p_buf.reserve(n + 1);
        std::memcpy(&p_buf[0], str, n);
        p_buf[n] = '\0';
        p_buf.setlen(n + 1);
    }

    void set(char const *str) { set(str, std::strlen(str)); }

    void set(strbuf const &b) { set(b.data(), b.size()); }

    void reserve(std::size_t n) { p_buf.reserve(n + 1); }

    void clear() {
        p_buf.clear();
        p_buf[0] = '\0';
        p_buf.setlen(1);
    }

    char operator[](std::size_t i) const { return p_buf[i]; }
    char &operator[](std::size_t i) { return p_buf[i]; }

    char front() const { return p_buf.front(); }
    char &front() { return p_buf.front(); }

    char back() const { return p_buf[size() - 1]; }
    char &back() { return p_buf[size() - 1]; }

    char const *data() const { return p_buf.data(); }
    char *data() { return p_buf.data(); }

    std::size_t size() const { return p_buf.size() - 1; }
    std::size_t capacity() const { return p_buf.capacity() - 1; }

    bool empty() const { return (size() == 0); }

    void swap(strbuf &b) { p_buf.swap(b.p_buf); }

    void setlen(std::size_t n) { p_buf.setlen(n + 1); }

    util::vector<char> const &raw() const { return p_buf; }
    util::vector<char> &raw() { return p_buf; }

private:
    util::vector<char> p_buf{};
};

/* hashtable */

template <typename K, typename V, typename HF, typename CF>
struct map {
private:
    static constexpr std::size_t CHUNK_SIZE = 64;
    static constexpr std::size_t DEFAULT_SIZE = 1024;

    struct entry {
        K key;
        V data;
    };

    struct bucket {
        entry value;
        bucket *next;
    };

public:
    map(std::size_t sz = DEFAULT_SIZE) : p_size{sz} {
        p_buckets = new bucket *[sz];
        std::memset(p_buckets, 0, sz * sizeof(bucket *));
    }

    ~map() {
        delete[] p_buckets;
        drop_chunks();
    }

    bool empty() const { return p_nelems == 0; }

    V &operator[](K const &key) {
        std::size_t h;
        bucket *b = find_bucket(key, h);
        if (!b) {
            b = add(h);
            b->value.key = key;
        }
        return b->value.data;
    }

    V *find(K const &key) const {
        std::size_t h;
        bucket *b = find_bucket(key, h);
        if (!b) {
            return nullptr;
        }
        return &b->value.data;
    }

    V &insert(K const &key, V const &value) {
        std::size_t h;
        bucket *b = find_bucket(key, h);
        if (!b) {
            b = add(h);
            b->value.key = key;
            b->value.data = value;
        }
        return b->value.data;
    }

    void clear() {
        if (!p_nelems) {
            return;
        }
        p_nelems = 0;
        p_unused = nullptr;
        std::memset(p_buckets, 0, p_size * sizeof(bucket *));
        drop_chunks();
    }

    void swap(map &m) {
        util::swap(p_size, m.p_size);
        util::swap(p_nelems, m.p_nelems);
        util::swap(p_buckets, m.p_buckets);
        util::swap(p_unused, m.p_unused);
        util::swap(p_chunks, m.p_chunks);
    }

    template <typename F>
    void for_each(F &&func) const {
        for (std::size_t i = 0; i < p_size; ++i) {
            for (bucket *b = p_buckets[i]; b; b = b->next) {
                func(b->value.key, b->value.data);
            }
        }
    }

private:
    bucket *add(std::size_t hash) {
        if (!p_unused) {
            chunk *nb = new chunk;
            nb->next = p_chunks;
            p_chunks = nb;
            for (std::size_t i = 0; i < CHUNK_SIZE - 1; ++i) {
                nb->buckets[i].next = &nb->buckets[i + 1];
            }
            nb->buckets[CHUNK_SIZE - 1].next = p_unused;
            p_unused = nb->buckets;
        }
        bucket *b = p_unused;
        p_unused = p_unused->next;
        b->next = p_buckets[hash];
        p_buckets[hash] = b;
        ++p_nelems;
        return b;
    }

    bucket *find_bucket(K const &key, std::size_t &h) const {
        h = HF{}(key) % p_size;
        for (bucket *b = p_buckets[h]; b; b = b->next) {
            if (CF{}(key, b->value.key)) {
                return b;
            }
        }
        return nullptr;
    }

    void drop_chunks() {
        for (chunk *nc; p_chunks; p_chunks = nc) {
            nc = p_chunks->next;
            delete p_chunks;
        }
    }

    struct chunk {
        bucket buckets[CHUNK_SIZE];
        chunk *next;
    };

    std::size_t p_size, p_nelems = 0;

    bucket **p_buckets;
    bucket *p_unused = nullptr;
    chunk *p_chunks = nullptr;
};

#if SIZE_MAX > 0xFFFF
/* fnv1a for 32/64bit values */
template <std::size_t offset_basis, std::size_t prime>
struct fnv1a {
    std::size_t operator()(char const *data) const {
        std::size_t slen = std::strlen(data);
        std::size_t hash = offset_basis;
        for (std::size_t i = 0; i < slen; ++i) {
            hash ^= std::size_t(data[i]);
            hash *= prime;
        }
        return hash;
    }
};
#else
/* pearson hash for smaller values */
static unsigned char const ph_lt[256] = {167, 49,  207, 184, 90,  134, 74,  211, 215, 76,  109, 126, 222, 97,  231, 1,   132, 204, 149, 249, 166, 33,  237, 100, 141, 186, 191, 112, 151, 203, 69,  87,
                                         65,  80,  157, 95,  58,  59,  82,  115, 171, 192, 24,  244, 225, 223, 102, 189, 164, 119, 216, 174, 68,  133, 7,   10,  159, 31,  255, 150, 41,  169, 161, 43,
                                         245, 235, 16,  94,  81,  162, 103, 53,  110, 135, 228, 86,  114, 144, 156, 241, 2,   253, 195, 128, 22,  105, 199, 250, 64,  13,  178, 63,  99,  39,  190, 130,
                                         163, 30,  122, 18,  168, 83,  220, 71,  129, 84,  3,   208, 155, 9,   242, 170, 51,  143, 56,  158, 176, 172, 148, 55,  227, 254, 247, 224, 50,  93,  54,  210,
                                         206, 234, 218, 229, 61,  26,  107, 32,  196, 217, 248, 138, 154, 212, 96,  40,  209, 38,  101, 73,  88,  125, 175, 187, 34,  62,  118, 66,  113, 46,  238, 42,
                                         202, 0,   179, 67,  47,  20,  152, 165, 17,  89,  48,  123, 219, 70,  91,  120, 177, 188, 145, 104, 92,  98,  44,  108, 4,   37,  139, 11,  214, 52,  221, 29,
                                         75,  19,  35,  124, 185, 28,  201, 230, 198, 131, 116, 153, 77,  72,  45,  226, 146, 12,  137, 21,  147, 25,  27,  180, 240, 200, 243, 194, 15,  183, 181, 233,
                                         213, 232, 136, 14,  252, 121, 85,  111, 106, 127, 197, 251, 205, 8,   60,  246, 140, 160, 239, 36,  6,   5,   142, 79,  57,  173, 182, 193, 117, 236, 23,  78};
struct pearson {
    std::size_t operator()(char const *data) const {
        std::size_t slen = std::strlen(data);
        std::size_t hash = 0;
        auto *udata = pun<unsigned char const *>(data);
        for (std::size_t j = 0; j < sizeof(std::size_t); ++j) {
            auto h = ph_lt[(udata[0] + j) % 256];
            for (std::size_t i = 1; i < slen; ++i) {
                h = ph_lt[h ^ udata[i]];
            }
            hash = ((hash << 8) | h);
        }
        return hash;
    }
};
#endif

#if SIZE_MAX > 0xFFFFFFFF
struct str_hash : fnv1a<14695981039346656037ULL, 1099511628211ULL> {};
#elif SIZE_MAX > 0xFFFF
struct str_hash : fnv1a<2166136261U, 16777619U> {};
#else
struct str_hash : pearson {};
#endif

struct str_equal {
    bool operator()(char const *k1, char const *k2) const { return !std::strcmp(k1, k2); }
};

template <typename V>
using str_map = map<char const *, V, str_hash, str_equal>;

} /* namespace util */

#if defined(FFI_DIAGNOSTIC_PRAGMA_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(FFI_DIAGNOSTIC_PRAGMA_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#if defined(FFI_DIAGNOSTIC_PRAGMA_CLANG)
#pragma clang diagnostic pop
#elif defined(FFI_DIAGNOSTIC_PRAGMA_GCC)
#pragma GCC diagnostic pop
#endif

#if LUA_VERSION_NUM < 501

#error This Lua version is not supported.

#elif LUA_VERSION_NUM == 501

/* lua 5.1 compat bits
 *
 * defines are used in case e.g. luajit is used which has these funcs
 */

static inline void luaL_setmetatable52(lua_State *L, char const *tname) {
    luaL_getmetatable(L, tname);
    lua_setmetatable(L, -2);
}

#ifdef luaL_setmetatable
#undef luaL_setmetatable
#endif
#define luaL_setmetatable luaL_setmetatable52

static inline void *luaL_testudata52(lua_State *L, int ud, char const *tname) {
    void *p = lua_touserdata(L, ud);
    if (!p || !lua_getmetatable(L, ud)) {
        return nullptr;
    }
    lua_getfield(L, LUA_REGISTRYINDEX, tname);
    if (lua_rawequal(L, -1, -2)) {
        lua_pop(L, 2);
        return p;
    }
    lua_pop(L, 2);
    return nullptr;
}

#ifdef luaL_testudata
#undef luaL_testudata
#endif
#define luaL_testudata luaL_testudata52

static inline std::size_t lua_rawlen52(lua_State *L, int index) { return lua_objlen(L, index); }

#ifdef lua_rawlen
#undef lua_rawlen
#endif
#define lua_rawlen lua_rawlen52

static inline void luaL_newlib52(lua_State *L, luaL_Reg const l[]) {
    lua_newtable(L);
    luaL_register(L, nullptr, l);
}

#ifdef luaL_newlib
#undef luaL_newlib
#endif
#define luaL_newlib luaL_newlib52

#endif /* LUA_VERSION_NUM == 501 */

#if LUA_VERSION_NUM < 503

#ifdef lua_isinteger
#undef lua_isinteger
#endif
#define lua_isinteger(L, idx) int(0)

#endif /* LUA_VERSION_NUM == 503 */

namespace lua {

static constexpr int CFFI_CTYPE_TAG = -128;
static constexpr char const CFFI_CDATA_MT[] = "cffi_cdata_handle";
static constexpr char const CFFI_LIB_MT[] = "cffi_lib_handle";
static constexpr char const CFFI_DECL_STOR[] = "cffi_decl_stor";
static constexpr char const CFFI_PARSER_STATE[] = "cffi_parser_state";

template <typename T>
static T *touserdata(lua_State *L, int index) {
    return static_cast<T *>(lua_touserdata(L, index));
}

static inline int type_error(lua_State *L, int narg, char const *tname) {
    lua_pushfstring(L, "%s expected, got %s", tname, lua_typename(L, lua_type(L, narg)));
    luaL_argcheck(L, false, narg, lua_tostring(L, -1));
    return 0;
}

static inline void mark_cdata(lua_State *L) { luaL_setmetatable(L, CFFI_CDATA_MT); }

static inline void mark_lib(lua_State *L) { luaL_setmetatable(L, CFFI_LIB_MT); }

#if LUA_VERSION_NUM < 503
/* 5.2 and older uses a simpler (unexposed) alignment */
union user_align_t {
    void *p;
    double d;
    long l;
};
#elif LUA_VERSION_NUM < 504
/* 5.3 does not expose this, so mirror its guts */
union user_align_t {
    lua_Number n;
    lua_Integer i;
    void *p;
    double d;
    long l;
};
#else
/* 5.4+ has the configured alignment in luaconf */
union user_align_t {
    LUAI_MAXALIGN;
};
#endif

} /* namespace lua */

#define LUA_BUG_MSG(L, msg) lua_pushfstring(L, "%s:%s: bug: %s", __FILE__, __LINE__, msg)

namespace lib {

using handle = void *;
using func = void (*)();

struct c_lib {
    handle h;
    int cache;
};

void load(c_lib *cl, char const *path, lua_State *L, bool global = false);

void close(c_lib *cl, lua_State *L);

void *get_sym(c_lib const *cl, lua_State *L, char const *name);

bool is_c(c_lib const *cl);

} /* namespace lib */

/* Force static linkage against libffi on Windows unless overridden */
#if defined(FFI_WINDOWS_ABI) && !defined(HAVE_LIBFFI_DLLIMPORT)
#define FFI_BUILDING 1
#endif

#if defined(FFI_DIAGNOSTIC_PRAGMA_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif

#include <ffi.h>

#if defined(FFI_DIAGNOSTIC_PRAGMA_CLANG)
#pragma clang diagnostic pop
#endif

namespace ffi {

namespace detail {
struct ffi_stor {
    alignas(alignof(util::max_aligned_t)) unsigned char data[sizeof(util::biggest_t)];
};

template <typename T>
static inline ffi_type *ffi_int() {
    bool is_signed = util::is_signed<T>::value;
    switch (sizeof(T)) {
        case 8:
            return is_signed ? &ffi_type_sint64 : &ffi_type_uint64;
        case 4:
            return is_signed ? &ffi_type_sint32 : &ffi_type_uint32;
        case 2:
            return is_signed ? &ffi_type_sint16 : &ffi_type_uint16;
        case 1:
            return is_signed ? &ffi_type_sint8 : &ffi_type_uint8;
        default:
            break;
    }
    assert(false);
    return nullptr;
}
} /* namespace detail */

using scalar_stor_t = detail::ffi_stor;

/* compile-time mappings from builtin types to libffi types
 *
 * this also allows catching bad types at compile time
 */

template <typename T>
struct ffi_traits;

template <>
struct ffi_traits<void> {
    static ffi_type *type() { return &ffi_type_void; }
};

template <typename T>
struct ffi_traits<T *> {
    static ffi_type *type() { return &ffi_type_pointer; }
};

template <typename T>
struct ffi_traits<T &> {
    static ffi_type *type() { return &ffi_type_pointer; }
};

template <typename T>
struct ffi_traits<T[]> {
    static ffi_type *type() { return &ffi_type_pointer; }
};

template <>
struct ffi_traits<bool> {
    static ffi_type *type() { return &ffi_type_uchar; }
};

template <>
struct ffi_traits<char> {
    static ffi_type *type() { return detail::ffi_int<char>(); }
};

template <>
struct ffi_traits<signed char> {
    static ffi_type *type() { return detail::ffi_int<signed char>(); }
};

template <>
struct ffi_traits<unsigned char> {
    static ffi_type *type() { return detail::ffi_int<unsigned char>(); }
};

template <>
struct ffi_traits<wchar_t> {
    static ffi_type *type() { return detail::ffi_int<wchar_t>(); }
};

template <>
struct ffi_traits<char16_t> {
    static ffi_type *type() { return detail::ffi_int<char16_t>(); }
};

template <>
struct ffi_traits<char32_t> {
    static ffi_type *type() { return detail::ffi_int<char32_t>(); }
};

template <>
struct ffi_traits<short> {
    static ffi_type *type() { return detail::ffi_int<short>(); }
};

template <>
struct ffi_traits<unsigned short> {
    static ffi_type *type() { return detail::ffi_int<unsigned short>(); }
};

template <>
struct ffi_traits<int> {
    static ffi_type *type() { return detail::ffi_int<int>(); }
};

template <>
struct ffi_traits<unsigned int> {
    static ffi_type *type() { return detail::ffi_int<unsigned int>(); }
};

template <>
struct ffi_traits<long> {
    static ffi_type *type() { return detail::ffi_int<long>(); }
};

template <>
struct ffi_traits<unsigned long> {
    static ffi_type *type() { return detail::ffi_int<unsigned long>(); }
};

template <>
struct ffi_traits<long long> {
    static ffi_type *type() { return detail::ffi_int<long long>(); }
};

template <>
struct ffi_traits<unsigned long long> {
    static ffi_type *type() { return detail::ffi_int<unsigned long long>(); }
};

template <>
struct ffi_traits<float> {
    static ffi_type *type() { return &ffi_type_float; }
};

template <>
struct ffi_traits<double> {
    static ffi_type *type() { return &ffi_type_double; }
};

template <>
struct ffi_traits<long double> {
    static ffi_type *type() { return &ffi_type_longdouble; }
};

template <typename T>
struct ffi_traits<T const> : ffi_traits<T> {};

template <typename T>
struct ffi_traits<T volatile> : ffi_traits<T> {};

template <typename T>
struct ffi_traits<T const volatile> : ffi_traits<T> {};

} /* namespace ffi */

namespace ast {

enum c_builtin {
    C_BUILTIN_INVALID = 0,

    C_BUILTIN_VOID,
    C_BUILTIN_PTR,

    C_BUILTIN_FUNC,
    C_BUILTIN_RECORD,
    C_BUILTIN_ARRAY,

    C_BUILTIN_VA_LIST,

    /* everything past this matches type.arith() */

    C_BUILTIN_ENUM,

    C_BUILTIN_BOOL,

    C_BUILTIN_CHAR,
    C_BUILTIN_SCHAR,
    C_BUILTIN_UCHAR,
    C_BUILTIN_SHORT,
    C_BUILTIN_USHORT,
    C_BUILTIN_INT,
    C_BUILTIN_UINT,
    C_BUILTIN_LONG,
    C_BUILTIN_ULONG,
    C_BUILTIN_LLONG,
    C_BUILTIN_ULLONG,

    C_BUILTIN_FLOAT,
    C_BUILTIN_DOUBLE,
    C_BUILTIN_LDOUBLE,
};

namespace detail {
template <typename T>
struct builtin_traits_base {
    using type = T;
    static ffi_type *libffi_type() { return ffi::ffi_traits<T>::type(); }
};
} /* namespace detail */

/* only defined for arithmetic types with direct mappings */
template <c_builtin>
struct builtin_traits;

template <>
struct builtin_traits<C_BUILTIN_VOID> : detail::builtin_traits_base<void> {};

template <>
struct builtin_traits<C_BUILTIN_PTR> : detail::builtin_traits_base<void *> {};

template <>
struct builtin_traits<C_BUILTIN_ARRAY> : detail::builtin_traits_base<char[]> {};

template <>
struct builtin_traits<C_BUILTIN_VA_LIST> {
    using type = va_list;

    /* special case; XXX is this good enough ABI-wise? */
    static ffi_type *libffi_type() { return &ffi_type_pointer; }
};

template <>
struct builtin_traits<C_BUILTIN_CHAR> : detail::builtin_traits_base<char> {};

template <>
struct builtin_traits<C_BUILTIN_SCHAR> : detail::builtin_traits_base<signed char> {};

template <>
struct builtin_traits<C_BUILTIN_UCHAR> : detail::builtin_traits_base<unsigned char> {};

template <>
struct builtin_traits<C_BUILTIN_SHORT> : detail::builtin_traits_base<short> {};

template <>
struct builtin_traits<C_BUILTIN_USHORT> : detail::builtin_traits_base<unsigned short> {};

template <>
struct builtin_traits<C_BUILTIN_INT> : detail::builtin_traits_base<int> {};

template <>
struct builtin_traits<C_BUILTIN_UINT> : detail::builtin_traits_base<unsigned int> {};

template <>
struct builtin_traits<C_BUILTIN_LONG> : detail::builtin_traits_base<long> {};

template <>
struct builtin_traits<C_BUILTIN_ULONG> : detail::builtin_traits_base<unsigned long> {};

template <>
struct builtin_traits<C_BUILTIN_LLONG> : detail::builtin_traits_base<long long> {};

template <>
struct builtin_traits<C_BUILTIN_ULLONG> : detail::builtin_traits_base<unsigned long long> {};

template <>
struct builtin_traits<C_BUILTIN_FLOAT> : detail::builtin_traits_base<float> {};

template <>
struct builtin_traits<C_BUILTIN_DOUBLE> : detail::builtin_traits_base<double> {};

template <>
struct builtin_traits<C_BUILTIN_LDOUBLE> : detail::builtin_traits_base<long double> {};

template <>
struct builtin_traits<C_BUILTIN_BOOL> : detail::builtin_traits_base<bool> {};

template <c_builtin t>
using builtin_t = typename builtin_traits<t>::type;

namespace detail {
template <typename T>
struct builtin_v_base {
    static constexpr c_builtin value = C_BUILTIN_INVALID;
};
template <>
struct builtin_v_base<void> {
    static constexpr c_builtin value = C_BUILTIN_VOID;
};
template <>
struct builtin_v_base<bool> {
    static constexpr c_builtin value = C_BUILTIN_BOOL;
};
template <>
struct builtin_v_base<char> {
    static constexpr c_builtin value = C_BUILTIN_CHAR;
};
template <>
struct builtin_v_base<signed char> {
    static constexpr c_builtin value = C_BUILTIN_SCHAR;
};
template <>
struct builtin_v_base<unsigned char> {
    static constexpr c_builtin value = C_BUILTIN_UCHAR;
};
template <>
struct builtin_v_base<short> {
    static constexpr c_builtin value = C_BUILTIN_SHORT;
};
template <>
struct builtin_v_base<unsigned short> {
    static constexpr c_builtin value = C_BUILTIN_USHORT;
};
template <>
struct builtin_v_base<int> {
    static constexpr c_builtin value = C_BUILTIN_INT;
};
template <>
struct builtin_v_base<unsigned int> {
    static constexpr c_builtin value = C_BUILTIN_UINT;
};
template <>
struct builtin_v_base<long> {
    static constexpr c_builtin value = C_BUILTIN_LONG;
};
template <>
struct builtin_v_base<unsigned long> {
    static constexpr c_builtin value = C_BUILTIN_ULONG;
};
template <>
struct builtin_v_base<long long> {
    static constexpr c_builtin value = C_BUILTIN_LLONG;
};
template <>
struct builtin_v_base<unsigned long long> {
    static constexpr c_builtin value = C_BUILTIN_ULLONG;
};
template <>
struct builtin_v_base<float> {
    static constexpr c_builtin value = C_BUILTIN_FLOAT;
};
template <>
struct builtin_v_base<double> {
    static constexpr c_builtin value = C_BUILTIN_DOUBLE;
};
template <>
struct builtin_v_base<long double> {
    static constexpr c_builtin value = C_BUILTIN_LDOUBLE;
};
template <typename T>
struct builtin_v_base<T *> {
    static constexpr c_builtin value = C_BUILTIN_PTR;
};

/* need this hack because some toolchains are garbage */
template <typename T>
using eq_type = util::conditional_t<
        sizeof(T) == sizeof(char), signed char,
        util::conditional_t<sizeof(T) == sizeof(short), short, util::conditional_t<sizeof(T) == sizeof(int), int, util::conditional_t<sizeof(T) == sizeof(long), long, long long>>>>;
template <typename T>
using eq_utype =
        util::conditional_t<sizeof(T) == sizeof(char), unsigned char,
                            util::conditional_t<sizeof(T) == sizeof(short), unsigned short,
                                                util::conditional_t<sizeof(T) == sizeof(int), unsigned int, util::conditional_t<sizeof(T) == sizeof(long), unsigned long, unsigned long long>>>>;

template <>
struct builtin_v_base<wchar_t> {
    static constexpr c_builtin value = util::is_signed<wchar_t>::value ? builtin_v_base<eq_type<wchar_t>>::value : builtin_v_base<eq_utype<wchar_t>>::value;
};
template <>
struct builtin_v_base<char16_t> {
    static constexpr c_builtin value = util::is_signed<char16_t>::value ? builtin_v_base<eq_type<char16_t>>::value : builtin_v_base<eq_utype<char16_t>>::value;
};
template <>
struct builtin_v_base<char32_t> {
    static constexpr c_builtin value = util::is_signed<char32_t>::value ? builtin_v_base<eq_type<char16_t>>::value : builtin_v_base<eq_utype<char32_t>>::value;
};
} /* namespace detail */

template <typename T>
static constexpr c_builtin builtin_v = detail::builtin_v_base<T>::value;

template <c_builtin t>
inline ffi_type *builtin_ffi_type() {
    return builtin_traits<t>::libffi_type();
}

enum c_cv {
    C_CV_CONST = 1 << 0,
    C_CV_VOLATILE = 1 << 1,
};

enum c_type_flags {
    C_TYPE_WEAK = 1 << 0,
    C_TYPE_CLOSURE = 1 << 1,
    C_TYPE_NOSIZE = 1 << 2,
    C_TYPE_VLA = 1 << 3,
    C_TYPE_REF = 1 << 4,
};

enum c_func_flags {
    C_FUNC_DEFAULT = 0,
    C_FUNC_CDECL,
    C_FUNC_FASTCALL,
    C_FUNC_STDCALL,
    C_FUNC_THISCALL,

    C_FUNC_VARIADIC = 1 << 8,
};

enum class c_object_type {
    INVALID = 0,
    FUNCTION,
    VARIABLE,
    CONSTANT,
    TYPEDEF,
    RECORD,
    ENUM,
    TYPE,
    PARAM,
};

enum class c_expr_type { INVALID = 0, INT, UINT, LONG, ULONG, LLONG, ULLONG, FLOAT, DOUBLE, LDOUBLE, STRING, CHAR, NULLPTR, BOOL, NAME, UNARY, BINARY, TERNARY };

static inline c_builtin to_builtin_type(c_expr_type v) {
    switch (v) {
        case c_expr_type::INT:
            return C_BUILTIN_INT;
        case c_expr_type::UINT:
            return C_BUILTIN_UINT;
        case c_expr_type::LONG:
            return C_BUILTIN_LONG;
        case c_expr_type::ULONG:
            return C_BUILTIN_ULONG;
        case c_expr_type::LLONG:
            return C_BUILTIN_LLONG;
        case c_expr_type::ULLONG:
            return C_BUILTIN_ULLONG;
        case c_expr_type::FLOAT:
            return C_BUILTIN_FLOAT;
        case c_expr_type::DOUBLE:
            return C_BUILTIN_DOUBLE;
        case c_expr_type::LDOUBLE:
            return C_BUILTIN_LDOUBLE;
        case c_expr_type::CHAR:
            return C_BUILTIN_CHAR;
        case c_expr_type::BOOL:
            return C_BUILTIN_BOOL;
        default:
            break;
    }
    return C_BUILTIN_INVALID;
}

/* don't forget to update precedences in parser when adding to this */
enum class c_expr_binop {
    INVALID = 0,
    ADD,  // +
    SUB,  // -
    MUL,  // *
    DIV,  // /
    MOD,  // %

    EQ,   // ==
    NEQ,  // !=
    GT,   // >
    LT,   // <
    GE,   // >=
    LE,   // <=

    AND,  // &&
    OR,   // ||

    BAND,  // &
    BOR,   // |
    BXOR,  // ^
    LSH,   // <<
    RSH    // >>
};

enum class c_expr_unop {
    INVALID = 0,

    UNM,  // -
    UNP,  // +

    NOT,  // !
    BNOT  // ~
};

/* this stores primitive values in a way that can be passed to the evaluator */
union c_value {
    /* fp primitives, unknown size */
    long double ld;
    double d;
    float f;
    /* signed int primitives, unknown size */
    long long ll;
    long l;
    int i;
    short s;
    char c;
    signed char sc;
    /* unsigned int primitives, unknown size */
    unsigned long long ull;
    unsigned long ul;
    unsigned int u;
    unsigned short us;
    unsigned char uc;
    /* booleans */
    bool b;
};

struct c_expr {
    c_expr(int flags = 0) : p_etype{int(c_expr_type::INVALID)}, p_flags{flags} {}

    c_expr(c_expr const &) = delete;
    c_expr(c_expr &&e) : p_etype{e.p_etype}, p_flags{e.p_flags} {
        e.p_etype = int(c_expr_type::INVALID);
        e.p_flags = 0;
        /* copy largest union */
        std::memcpy(&tern, &e.tern, sizeof(e.tern));
    }

    c_expr &operator=(c_expr const &) = delete;
    c_expr &operator=(c_expr &&e) {
        clear();
        p_etype = e.p_etype;
        p_flags = e.p_flags;
        e.p_etype = int(c_expr_type::INVALID);
        e.p_flags = 0;
        std::memcpy(&tern, &e.tern, sizeof(e.tern));
        return *this;
    }

    ~c_expr() { clear(); }

    struct unary {
        c_expr_unop op;
        c_expr *expr;
    };

    struct binary {
        c_expr_binop op;
        c_expr *lhs;
        c_expr *rhs;
    };

    struct ternary {
        c_expr *cond;
        c_expr *texpr;
        c_expr *fexpr;
    };

    union {
        unary un;
        binary bin;
        ternary tern;
        c_value val;
    };

    bool eval(lua_State *L, c_value &v, c_expr_type &et, bool promote) const WARN_UNUSED_RET;

    c_expr_type type() const { return c_expr_type(p_etype); }

    void type(c_expr_type tp) { p_etype = int(tp); }

    bool owns() const { return !bool(p_flags & C_TYPE_WEAK); }

private:
    int p_etype : 6;
    int p_flags : 6;

    void clear() {
        if (!owns()) {
            return;
        }
        switch (type()) {
            case c_expr_type::UNARY:
                delete un.expr;
                break;
            case c_expr_type::BINARY:
                delete bin.lhs;
                delete bin.rhs;
                break;
            case c_expr_type::TERNARY:
                delete tern.cond;
                delete tern.texpr;
                delete tern.fexpr;
                break;
            default:
                break;
        }
    }
};

using c_object_cont_f = void (*)(util::strbuf &, void *);

struct c_object {
    c_object() {}
    virtual ~c_object();

    virtual char const *name() const { return nullptr; }
    virtual c_object_type obj_type() const { return c_object_type::INVALID; }
    virtual void do_serialize(util::strbuf &, c_object_cont_f, void *) const {}

    void serialize(util::strbuf &sb) const { do_serialize(sb, nullptr, nullptr); }

    void serialize(lua_State *L) const {
        util::strbuf sb;
        serialize(sb);
        lua_pushlstring(L, sb.data(), sb.size());
    }

    template <typename T>
    T &as() {
        return *static_cast<T *>(this);
    }

    template <typename T>
    T const &as() const {
        return *static_cast<T const *>(this);
    }
};

struct c_function;
struct c_record;
struct c_enum;

struct c_type : c_object {
    c_type() : p_crec{nullptr}, p_ttype{C_BUILTIN_INVALID}, p_flags{0}, p_cv{0} {}

    c_type(c_builtin cbt, std::uint32_t qual) : p_crec{nullptr}, p_ttype{std::uint32_t(cbt)}, p_flags{0}, p_cv{qual} {}

    c_type(util::rc_obj<c_type> ctp, std::uint32_t qual, std::size_t arrlen, std::uint32_t flags) : p_asize{arrlen}, p_ttype{C_BUILTIN_ARRAY}, p_flags{flags}, p_cv{qual} {
        new (&p_ptr) util::rc_obj<c_type>{util::move(ctp)};
    }

    c_type(util::rc_obj<c_type> ctp, std::uint32_t qual, c_builtin cbt) : p_ttype{std::uint32_t(cbt)}, p_flags{0}, p_cv{qual} { new (&p_ptr) util::rc_obj<c_type>{util::move(ctp)}; }

    c_type(util::rc_obj<c_function> ctp, std::uint32_t qual, bool cb) : p_ttype{C_BUILTIN_FUNC}, p_flags{std::uint32_t(cb ? C_TYPE_CLOSURE : 0)}, p_cv{qual} {
        new (&p_func) util::rc_obj<c_function>{util::move(ctp)};
    }

    c_type(c_record const *ctp, std::uint32_t qual) : p_crec{ctp}, p_ttype{C_BUILTIN_RECORD}, p_flags{0}, p_cv{qual} {}

    c_type(c_enum const *ctp, std::uint32_t qual) : p_cenum{ctp}, p_ttype{C_BUILTIN_ENUM}, p_flags{0}, p_cv{qual} {}

    c_type(c_type const &tp) = delete;
    c_type(c_type &&);

    c_type &operator=(c_type const &tp) = delete;
    c_type &operator=(c_type &&);

    ~c_type() { clear(); }

    c_object_type obj_type() const { return c_object_type::TYPE; }

    c_type copy() const {
        c_type ret{};
        ret.copy(*this);
        return ret;
    }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const;

    char const *name() const {
        switch (type()) {
            case C_BUILTIN_VOID:
                return "void";
            case C_BUILTIN_CHAR:
                return "char";
            case C_BUILTIN_SCHAR:
                return "signed char";
            case C_BUILTIN_UCHAR:
                return "unsigned char";
            case C_BUILTIN_SHORT:
                return "short";
            case C_BUILTIN_USHORT:
                return "unsigned short";
            case C_BUILTIN_INT:
                return "int";
            case C_BUILTIN_UINT:
                return "unsigned int";
            case C_BUILTIN_LONG:
                return "long";
            case C_BUILTIN_ULONG:
                return "unsigned long";
            case C_BUILTIN_LLONG:
                return "long long";
            case C_BUILTIN_ULLONG:
                return "unsigned long long";
            case C_BUILTIN_FLOAT:
                return "float";
            case C_BUILTIN_DOUBLE:
                return "double";
            case C_BUILTIN_LDOUBLE:
                return "long double";
            case C_BUILTIN_BOOL:
                return "bool";
            case C_BUILTIN_VA_LIST:
                return "va_list";
            default:
                break;
        }
        return nullptr;
    }

    c_builtin type() const { return c_builtin(p_ttype); }

    int cv() const { return p_cv; }

    bool vla() const { return p_flags & C_TYPE_VLA; }

    bool unbounded() const { return p_flags & C_TYPE_NOSIZE; }

    bool flex() const { return (unbounded() || vla()); }

    bool builtin_array() const { return type() == C_BUILTIN_ARRAY; }

    bool static_array() const { return builtin_array() && !flex(); }

    bool closure() const {
        switch (type()) {
            case C_BUILTIN_FUNC:
                return p_flags & C_TYPE_CLOSURE;
            case C_BUILTIN_PTR:
                return ptr_base().p_flags & C_TYPE_CLOSURE;
            default:
                break;
        }
        return false;
    }

    bool arith() const { return type() >= C_BUILTIN_ENUM; }

    bool byte() const {
        switch (type()) {
            case C_BUILTIN_CHAR:
            case C_BUILTIN_SCHAR:
            case C_BUILTIN_UCHAR:
                return true;
            default:
                break;
        }
        return false;
    }

    bool callable() const {
        auto tp = type();
        if (tp == C_BUILTIN_FUNC) {
            return true;
        }
        if (tp != C_BUILTIN_PTR) {
            return false;
        }
        return ptr_base().type() == C_BUILTIN_FUNC;
    }

    bool passable() const;

    bool integer() const { return arith() && (type() < C_BUILTIN_FLOAT); }

    bool ptr_like() const {
        switch (type()) {
            case C_BUILTIN_PTR:
            case C_BUILTIN_ARRAY:
            case C_BUILTIN_FUNC:
                return true;
            default:
                break;
        }
        return false;
    }

    bool is_ref() const { return p_flags & C_TYPE_REF; }

    c_type unref() const {
        auto ret = copy();
        ret.p_flags &= ~C_TYPE_REF;
        return ret;
    }

    c_type as_ref() const {
        auto ret = copy();
        ret.add_ref();
        return ret;
    }

    void add_ref() { p_flags |= C_TYPE_REF; }

    bool is_unsigned() const {
        auto *p = libffi_type();
        return ((p == &ffi_type_uint8) || (p == &ffi_type_uint16) || (p == &ffi_type_uint32) || (p == &ffi_type_uint64));
    }

    void cv(int qual) { p_cv |= std::uint32_t(qual); }

    c_type const &ptr_base() const { return *p_ptr; }

    c_type const &ptr_ref_base() const {
        if (is_ref()) {
            return *this;
        }
        return *p_ptr;
    }

    /* only use if you know it's callable() */
    util::rc_obj<c_function> const &function() const {
        if (type() == C_BUILTIN_FUNC) {
            return p_func;
        }
        return ptr_base().p_func;
    }

    c_record const &record() const { return *p_crec; }

    ffi_type *libffi_type() const;

    std::size_t alloc_size() const;

    std::size_t array_size() const { return p_asize; }

    bool is_same(c_type const &other, bool ignore_cv = false, bool ignore_ref = false) const;

    /* only use this with ref and ptr types */
    c_type as_type(int cbt) const {
        auto ret = copy();
        ret.p_ttype ^= ret.type();
        ret.p_ttype |= cbt;
        return ret;
    }

private:
    void clear();
    void copy(c_type const &);

    /* maybe a pointer? */
    union {
        util::rc_obj<c_type> p_ptr;
        util::rc_obj<c_function> p_func;
        c_record const *p_crec;
        c_enum const *p_cenum;
    };
    std::size_t p_asize = 0;
    std::uint32_t p_ttype : 5;
    std::uint32_t p_flags : 5;
    std::uint32_t p_cv : 2;
};

struct c_param : c_object {
    c_param(util::strbuf pname, c_type type) : p_name{util::move(pname)}, p_type{util::move(type)} {}

    c_object_type obj_type() const { return c_object_type::PARAM; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const;

    char const *name() const { return p_name.data(); }

    c_type const &type() const { return p_type; }

    ffi_type *libffi_type() const { return p_type.libffi_type(); }

    std::size_t alloc_size() const { return p_type.alloc_size(); }

private:
    util::strbuf p_name;
    c_type p_type;
};

struct c_function : c_object {
    c_function(c_type result, util::vector<c_param> params, std::uint32_t flags) : p_result{util::move(result)}, p_params{util::move(params)}, p_flags{flags} {}

    c_object_type obj_type() const { return c_object_type::FUNCTION; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const;

    char const *name() const { return nullptr; }

    c_type const &result() const { return p_result; }

    util::vector<c_param> const &params() const { return p_params; }

    ffi_type *libffi_type() const { return &ffi_type_pointer; }

    std::size_t alloc_size() const { return sizeof(void *); }

    bool is_same(c_function const &other) const;

    bool variadic() const { return !!(p_flags & C_FUNC_VARIADIC); }

    std::uint32_t callconv() const { return p_flags & 0xF; }

    void callconv(std::uint32_t conv) {
        p_flags ^= callconv();
        p_flags |= conv & 0xFF;
    }

private:
    c_type p_result;
    util::vector<c_param> p_params;
    std::uint32_t p_flags;
    bool p_variadic;
};

struct c_variable : c_object {
    c_variable(util::strbuf vname, util::strbuf sym, c_type vtype) : p_name{util::move(vname)}, p_sname{util::move(sym)}, p_type{util::move(vtype)} {}

    c_object_type obj_type() const { return c_object_type::VARIABLE; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const { p_type.do_serialize(o, cont, data); }

    char const *name() const { return p_name.data(); }

    char const *sym() const {
        if (!p_sname.empty()) {
            return p_sname.data();
        }
        return p_name.data();
    }

    c_type const &type() const { return p_type; }

    ffi_type *libffi_type() const { return p_type.libffi_type(); }

    std::size_t alloc_size() const { return p_type.alloc_size(); }

private:
    util::strbuf p_name;
    util::strbuf p_sname;
    c_type p_type;
};

struct c_constant : c_object {
    c_constant(util::strbuf cname, c_type ctype, c_value const &cval) : p_name{util::move(cname)}, p_type{util::move(ctype)}, p_value{cval} {}

    c_object_type obj_type() const { return c_object_type::CONSTANT; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const { p_type.do_serialize(o, cont, data); }

    char const *name() const { return p_name.data(); }

    c_type const &type() const { return p_type; }

    c_value const &value() const { return p_value; }

    ffi_type *libffi_type() const { return p_type.libffi_type(); }

    std::size_t alloc_size() const { return p_type.alloc_size(); }

private:
    util::strbuf p_name;
    c_type p_type;
    c_value p_value;
};

struct c_typedef : c_object {
    c_typedef(util::strbuf aname, c_type btype) : p_name{util::move(aname)}, p_type{util::move(btype)} {}

    c_object_type obj_type() const { return c_object_type::TYPEDEF; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const {
        /* typedefs are resolved to their base type */
        p_type.do_serialize(o, cont, data);
    }

    char const *name() const { return p_name.data(); }

    c_type const &type() const { return p_type; }

    ffi_type *libffi_type() const { return p_type.libffi_type(); }

    std::size_t alloc_size() const { return p_type.alloc_size(); }

private:
    util::strbuf p_name;
    c_type p_type;
};

/* represents a record type: can be a struct or a union */
struct c_record : c_object {
    struct field {
        field(util::strbuf nm, c_type &&tp) : name{util::move(nm)}, type(util::move(tp)) {}

        util::strbuf name;
        c_type type;
    };

    c_record(util::strbuf ename, util::vector<field> fields, bool is_uni = false) : p_name{util::move(ename)}, p_uni{is_uni} { set_fields(util::move(fields)); }

    c_record(util::strbuf ename, bool is_uni = false) : p_name{util::move(ename)}, p_uni{is_uni} {}

    ~c_record() {
        delete[] p_elements;
        delete[] p_felems;
    }

    c_object_type obj_type() const { return c_object_type::RECORD; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const {
        o.append(this->p_name);
        if (cont) {
            cont(o, data);
        }
    }

    char const *name() const { return p_name.data(); }

    /* invalid for opaque structs */
    ffi_type *libffi_type() const { return const_cast<ffi_type *>(&p_ffi_type); }

    std::size_t alloc_size() const { return libffi_type()->size; }

    bool is_same(c_record const &other) const;

    std::ptrdiff_t field_offset(char const *fname, c_type const *&fld) const;

    bool opaque() const { return !p_elements; }

    bool flexible(c_type const **outt = nullptr) const {
        if (p_fields.empty()) {
            return false;
        }
        auto &lf = p_fields.back();
        if (lf.type.type() == ast::C_BUILTIN_RECORD) {
            return lf.type.record().flexible(outt);
        }
        if (outt) {
            *outt = &lf.type;
        }
        return lf.type.flex();
    }

    bool passable() const {
        if (opaque()) {
            return false;
        }
#ifndef FFI_ABI_UNIONVAL
        if (is_union()) {
            return false;
        }
#endif
        bool ret = true;
        iter_fields([&ret](auto *, auto &type, std::size_t) {
            if (!type.passable()) {
                ret = false;
                return true;
            }
            return false;
        });
        return ret;
    }

    bool is_union() const { return p_uni; }

    /* it is the responsibility of the caller to ensure we're not redefining */
    void set_fields(util::vector<field> fields);

    void metatype(int mt, int mf) {
        p_metatype = mt;
        p_metaflags = mf;
    }

    int metatype(int &flags) const {
        flags = p_metaflags;
        return p_metatype;
    }

    template <typename F>
    void iter_fields(F &&cb) const {
        bool end = false;
        iter_fields(
                [](char const *fname, c_type const &type, std::size_t off, void *data) {
                    F &acb = *static_cast<F *>(data);
                    return acb(fname, type, off);
                },
                &cb, 0, end);
    }

    util::vector<field> const &raw_fields() const { return p_fields; }

private:
    std::size_t iter_fields(bool (*cb)(char const *fname, c_type const &type, std::size_t off, void *data), void *data, std::size_t base, bool &end) const;

    util::strbuf p_name;
    util::vector<field> p_fields{};
    ffi_type **p_elements = nullptr;
    ffi_type **p_felems = nullptr;
    ffi_type p_ffi_type{};
    ffi_type p_ffi_flex{};
    int p_metatype = LUA_REFNIL;
    int p_metaflags = 0;
    bool p_uni;
};

struct c_enum : c_object {
    struct field {
        field(util::strbuf nm, int val) : name{util::move(nm)}, value(val) {}

        util::strbuf name;
        int value; /* FIXME: make a c_expr */
    };

    c_enum(util::strbuf ename, util::vector<field> fields) : p_name{util::move(ename)} { set_fields(util::move(fields)); }

    c_enum(util::strbuf ename) : p_name{util::move(ename)} {}

    c_object_type obj_type() const { return c_object_type::ENUM; }

    void do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const {
        o.append(this->p_name);
        if (cont) {
            cont(o, data);
        }
    }

    char const *name() const { return p_name.data(); }

    util::vector<field> const &fields() const { return p_fields; }

    ffi_type *libffi_type() const {
        /* TODO: support for large enums */
        return &ffi_type_sint;
    }

    std::size_t alloc_size() const { return sizeof(int); }

    bool opaque() const { return p_opaque; }

    /* it is the responsibility of the caller to ensure we're not redefining */
    void set_fields(util::vector<field> fields) {
        assert(p_fields.empty());
        assert(p_opaque);

        p_fields = util::move(fields);
        p_opaque = false;
    }

private:
    util::strbuf p_name;
    util::vector<field> p_fields{};
    bool p_opaque = true;
};

struct decl_store {
    decl_store() {}
    decl_store(decl_store &ds) : p_base(&ds) {}
    ~decl_store() { drop(); }

    decl_store &operator=(decl_store const &) = delete;

    /* takes ownership of the pointer */
    c_object const *add(c_object *decl);
    void commit();
    void drop();

    c_object const *lookup(char const *name) const;
    c_object *lookup(char const *name);

    std::size_t request_name(char *buf, std::size_t bufsize);

    static decl_store &get_main(lua_State *L) {
        lua_getfield(L, LUA_REGISTRYINDEX, lua::CFFI_DECL_STOR);
        auto *ds = lua::touserdata<decl_store>(L, -1);
        assert(ds);
        lua_pop(L, 1);
        return *ds;
    }

private:
    struct obj_ptr {
        c_object *value = nullptr;
        obj_ptr(c_object *v) : value{v} {}
        ~obj_ptr() { delete value; }
        obj_ptr(obj_ptr &&v) { value = util::exchange(v.value, nullptr); }
        obj_ptr(obj_ptr const &v) = delete;
    };
    decl_store *p_base = nullptr;
    util::vector<obj_ptr> p_dlist{};
    util::str_map<c_object *> p_dmap{};
    std::size_t name_counter = 0;
};

c_type from_lua_type(lua_State *L, int index);

} /* namespace ast */

namespace parser {

void init(lua_State *L);

void parse(lua_State *L, char const *input, char const *iend = nullptr, int paridx = -1);

ast::c_type parse_type(lua_State *L, char const *input, char const *iend = nullptr, int paridx = -1);

ast::c_expr_type parse_number(lua_State *L, ast::c_value &v, char const *input, char const *iend = nullptr);

} /* namespace parser */

#include <cstddef>
#include <cstring>

namespace ffi {

enum metatype_flag {
    /* all versions */
    METATYPE_FLAG_ADD = 1 << 0,
    METATYPE_FLAG_SUB = 1 << 1,
    METATYPE_FLAG_MUL = 1 << 2,
    METATYPE_FLAG_DIV = 1 << 3,
    METATYPE_FLAG_MOD = 1 << 4,
    METATYPE_FLAG_POW = 1 << 5,
    METATYPE_FLAG_UNM = 1 << 6,
    METATYPE_FLAG_CONCAT = 1 << 7,
    METATYPE_FLAG_LEN = 1 << 8,
    METATYPE_FLAG_EQ = 1 << 9,
    METATYPE_FLAG_LT = 1 << 10,
    METATYPE_FLAG_LE = 1 << 11,
    METATYPE_FLAG_INDEX = 1 << 12,
    METATYPE_FLAG_NEWINDEX = 1 << 13,
    METATYPE_FLAG_CALL = 1 << 14,
    METATYPE_FLAG_GC = 1 << 15,
    METATYPE_FLAG_TOSTRING = 1 << 16,

    /* only for ctypes */
    METATYPE_FLAG_NEW = 1 << 17,

#if LUA_VERSION_NUM > 501
    /* lua 5.2+ */
    METATYPE_FLAG_PAIRS = 1 << 18,

#if LUA_VERSION_NUM == 502
    /* lua 5.2 only */
    METATYPE_FLAG_IPAIRS = 1 << 19,
#endif

#if LUA_VERSION_NUM > 502
    /* lua 5.3+ */
    METATYPE_FLAG_IDIV = 1 << 20,
    METATYPE_FLAG_BAND = 1 << 21,
    METATYPE_FLAG_BOR = 1 << 22,
    METATYPE_FLAG_BXOR = 1 << 23,
    METATYPE_FLAG_BNOT = 1 << 24,
    METATYPE_FLAG_SHL = 1 << 25,
    METATYPE_FLAG_SHR = 1 << 26,

    METATYPE_FLAG_NAME = 1 << 27,
#if LUA_VERSION_NUM > 503
    METATYPE_FLAG_CLOSE = 1 << 28,
#endif /* LUA_VERSION_NUM > 503 */
#endif /* LUA_VERSION_NUM > 502 */
#endif /* LUA_VERSION_NUM > 501 */
};

static inline constexpr auto metafield_name(metatype_flag flag) {
    switch (flag) {
        case METATYPE_FLAG_ADD:
            return "__add";
        case METATYPE_FLAG_SUB:
            return "__sub";
        case METATYPE_FLAG_MUL:
            return "__mul";
        case METATYPE_FLAG_DIV:
            return "__div";
        case METATYPE_FLAG_MOD:
            return "__mod";
        case METATYPE_FLAG_POW:
            return "__pow";
        case METATYPE_FLAG_UNM:
            return "__unm";
        case METATYPE_FLAG_CONCAT:
            return "__concat";
        case METATYPE_FLAG_LEN:
            return "__len";
        case METATYPE_FLAG_EQ:
            return "__eq";
        case METATYPE_FLAG_LT:
            return "__lt";
        case METATYPE_FLAG_LE:
            return "__le";
        case METATYPE_FLAG_INDEX:
            return "__index";
        case METATYPE_FLAG_NEWINDEX:
            return "__newindex";
        case METATYPE_FLAG_CALL:
            return "__call";
        case METATYPE_FLAG_GC:
            return "__gc";
        case METATYPE_FLAG_TOSTRING:
            return "__tostring";
        case METATYPE_FLAG_NEW:
            return "__new";
#if LUA_VERSION_NUM > 501
        case METATYPE_FLAG_PAIRS:
            return "__pairs";
#if LUA_VERSION_NUM == 502
        case METATYPE_FLAG_IPAIRS:
            return "__ipairs";
#endif
#if LUA_VERSION_NUM > 502
        case METATYPE_FLAG_IDIV:
            return "__idiv";
        case METATYPE_FLAG_BAND:
            return "__band";
        case METATYPE_FLAG_BOR:
            return "__bor";
        case METATYPE_FLAG_BXOR:
            return "__bxor";
        case METATYPE_FLAG_BNOT:
            return "__bnot";
        case METATYPE_FLAG_SHL:
            return "__shl";
        case METATYPE_FLAG_SHR:
            return "__shr";
        case METATYPE_FLAG_NAME:
            return "__name";
#if LUA_VERSION_NUM > 503
        case METATYPE_FLAG_CLOSE:
            return "__close";
#endif /* LUA_VERSION_NUM > 503 */
#endif /* LUA_VERSION_NUM > 502 */
#endif /* LUA_VERSION_NUM > 501 */
        default:
            break;
    }
    return "";
}

static_assert(((sizeof(lua_Integer) <= sizeof(ffi::scalar_stor_t)) && (alignof(lua_Integer) <= alignof(ffi::scalar_stor_t)) && util::is_int<lua_Integer>::value), "unsupported lua_Integer type");

/* lua_Number is supported either as a float or as an integer */
static_assert(((sizeof(lua_Number) <= sizeof(ffi::scalar_stor_t)) && (alignof(lua_Number) <= alignof(ffi::scalar_stor_t)) && (util::is_float<lua_Number>::value || util::is_int<lua_Number>::value)),
              "unsupported lua_Number type");

struct cdata {
    ast::c_type decl;
    int gc_ref;
    /* auxiliary data that can be used by different cdata
     *
     * vararg functions store the number of arguments they have storage
     * prepared for here to avoid reallocating every time
     */
    int aux;

    template <typename D>
    cdata(D &&tp) : decl{util::forward<D>(tp)} {}

    /* we always allocate enough userdata so that after the regular fields
     * of struct cdata, there is a data section, with enough space so that
     * the requested type can fit aligned to maximum scalar alignment we are
     * storing
     *
     * this is important, because lua_newuserdata may return misaligned pointers
     * (it only guarantees alignment of typically 8, while we typically need 16)
     * so we have to overallocate by a bit, then manually align the data
     */
    void *as_ptr() { return util::ptr_align(this + 1); }

    template <typename T>
    T &as() {
        return *static_cast<T *>(as_ptr());
    }

    void *as_deref_ptr() {
        if (decl.is_ref()) {
            return as<void *>();
        }
        return as_ptr();
    }

    template <typename T>
    T &as_deref() {
        return *static_cast<T *>(as_deref_ptr());
    }

    void *address_of() {
        if (decl.ptr_like()) {
            return as<void *>();
        }
        return as_deref_ptr();
    }
};

struct ctype {
    ast::c_type decl;
    int ct_tag;

    template <typename D>
    ctype(D &&tp) : decl{util::forward<D>(tp)} {}
};

inline constexpr std::size_t cdata_pad_size() {
    auto csz = sizeof(cdata);
    auto mod = (csz % alignof(lua::user_align_t));
    /* size in multiples of lua userdata alignment */
    if (mod) {
        csz += alignof(lua::user_align_t) - mod;
    }
    /* this should typically not happen, unless configured that way */
    if (alignof(lua::user_align_t) >= alignof(util::max_aligned_t)) {
        return csz;
    }
    /* add the difference for internal alignment */
    csz += (alignof(util::max_aligned_t) - alignof(lua::user_align_t));
    /* this is what we will allocate, plus the data size */
    return csz;
}

struct closure_data {
    ffi_cif cif; /* closure data needs its own cif */
    int fref = LUA_REFNIL;
    lua_State *L = nullptr;
    ffi_closure *closure = nullptr;

    ~closure_data() {
        if (!closure) {
            return;
        }
        luaL_unref(L, LUA_REGISTRYINDEX, fref);
        ffi_closure_free(closure);
    }
};

/* data used for function types */
struct fdata {
    void (*sym)();
    closure_data *cd; /* only for callbacks, otherwise nullptr */
    ffi_cif cif;
    ffi::scalar_stor_t rarg;

    ffi::scalar_stor_t *args() { return util::pun<ffi::scalar_stor_t *>(this + 1); }
};

static inline cdata &newcdata(lua_State *L, ast::c_type const &tp, std::size_t vals) {
    auto ssz = cdata_pad_size() + vals;
    auto *cd = static_cast<cdata *>(lua_newuserdata(L, ssz));
    new (cd) cdata{tp.copy()};
    cd->gc_ref = LUA_REFNIL;
    cd->aux = 0;
    lua::mark_cdata(L);
    return *cd;
}

template <typename... A>
static inline ctype &newctype(lua_State *L, A &&...args) {
    auto *cd = static_cast<ctype *>(lua_newuserdata(L, sizeof(ctype)));
    new (cd) ctype{ast::c_type{util::forward<A>(args)...}};
    cd->ct_tag = lua::CFFI_CTYPE_TAG;
    lua::mark_cdata(L);
    return *cd;
}

static inline bool iscdata(lua_State *L, int idx) {
    auto *p = static_cast<ctype *>(luaL_testudata(L, idx, lua::CFFI_CDATA_MT));
    return p && (p->ct_tag != lua::CFFI_CTYPE_TAG);
}

static inline bool isctype(lua_State *L, int idx) {
    auto *p = static_cast<ctype *>(luaL_testudata(L, idx, lua::CFFI_CDATA_MT));
    return p && (p->ct_tag == lua::CFFI_CTYPE_TAG);
}

static inline bool iscval(lua_State *L, int idx) { return luaL_testudata(L, idx, lua::CFFI_CDATA_MT); }

static inline bool isctype(cdata const &cd) { return cd.gc_ref == lua::CFFI_CTYPE_TAG; }

static inline cdata &checkcdata(lua_State *L, int idx) {
    auto ret = static_cast<cdata *>(luaL_checkudata(L, idx, lua::CFFI_CDATA_MT));
    if (isctype(*ret)) {
        lua::type_error(L, idx, "cdata");
    }
    return *ret;
}

static inline cdata *testcval(lua_State *L, int idx) { return static_cast<cdata *>(luaL_testudata(L, idx, lua::CFFI_CDATA_MT)); }

static inline cdata *testcdata(lua_State *L, int idx) {
    auto ret = static_cast<cdata *>(luaL_testudata(L, idx, lua::CFFI_CDATA_MT));
    if (!ret || isctype(*ret)) {
        return nullptr;
    }
    return ret;
}

static inline cdata &tocdata(lua_State *L, int idx) { return *lua::touserdata<ffi::cdata>(L, idx); }

/* careful with this; use only if you're sure you have cdata at the index */
static inline std::size_t cdata_value_size(lua_State *L, int idx) {
    auto &cd = tocdata(L, idx);
    if (cd.decl.vla()) {
        /* VLAs only exist on lua side, they are always allocated by us, so
         * we can be sure they are contained within the lua-allocated block
         *
         * the VLA memory consists of the following:
         * - the cdata sequence with overallocation padding
         * - the section where the pointer to data is stored
         * - and finally the VLA memory itself
         *
         * that means we take the length of the userdata and remove everything
         * that is not the raw array data, and that is our final length
         */
        return (lua_rawlen(L, idx) - cdata_pad_size() - sizeof(ffi::scalar_stor_t));
    } else {
        /* otherwise the size is known, so fall back to that */
        return cd.decl.alloc_size();
    }
}

void destroy_cdata(lua_State *L, cdata &cd);
void destroy_closure(lua_State *L, closure_data *cd);

int call_cif(cdata &fud, lua_State *L, std::size_t largs);

enum conv_rule { RULE_CONV = 0, RULE_PASS, RULE_CAST, RULE_RET };

/* this pushes a value from `value` on the Lua stack; its type
 * and necessary conversions are done based on the info in `tp` and `rule`
 *
 * `lossy` implies that numbers will always be converted to a lua number
 */
int to_lua(lua_State *L, ast::c_type const &tp, void const *value, int rule, bool ffi_ret, bool lossy = false);

/* a unified version of from_lua that combines together the complex aggregate
 * initialization logic and simple conversions from scalar types, resulting
 * in an all in one function that can take care of storing the C value of
 * a Lua value inside a piece of memory
 */
void from_lua(lua_State *L, ast::c_type const &decl, void *stor, int idx);

void get_global(lua_State *L, lib::c_lib const *dl, const char *sname);
void set_global(lua_State *L, lib::c_lib const *dl, char const *sname, int idx);

void make_cdata(lua_State *L, ast::c_type const &decl, int rule, int idx);

static inline bool metatype_getfield(lua_State *L, int mt, char const *fname) {
    luaL_getmetatable(L, lua::CFFI_CDATA_MT);
    lua_getfield(L, -1, "__ffi_metatypes");
    lua_rawgeti(L, -1, mt);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, fname);
        if (!lua_isnil(L, -1)) {
            lua_insert(L, -4);
            lua_pop(L, 3);
            return true;
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 3);
    return false;
}

template <typename T>
static inline bool test_arith(lua_State *L, int idx, T &out) {
    auto *cd = testcdata(L, idx);
    if (!cd) {
        if (util::is_int<T>::value) {
            if (lua_type(L, idx) == LUA_TNUMBER) {
                out = T(lua_tointeger(L, idx));
                return true;
            } else {
                return false;
            }
        }
        if (lua_type(L, idx) == LUA_TNUMBER) {
            out = T(lua_tointeger(L, idx));
            return true;
        }
        return false;
    }
    auto gf = [](int itp, void *av, T &rv) {
        switch (itp) {
            case ast::C_BUILTIN_ENUM:
                /* TODO: large enums */
                rv = T(*static_cast<int *>(av));
                break;
            case ast::C_BUILTIN_BOOL:
                rv = T(*static_cast<bool *>(av));
                break;
            case ast::C_BUILTIN_CHAR:
                rv = T(*static_cast<char *>(av));
                break;
            case ast::C_BUILTIN_SCHAR:
                rv = T(*static_cast<signed char *>(av));
                break;
            case ast::C_BUILTIN_UCHAR:
                rv = T(*static_cast<unsigned char *>(av));
                break;
            case ast::C_BUILTIN_SHORT:
                rv = T(*static_cast<short *>(av));
                break;
            case ast::C_BUILTIN_USHORT:
                rv = T(*static_cast<unsigned short *>(av));
                break;
            case ast::C_BUILTIN_INT:
                rv = T(*static_cast<int *>(av));
                break;
            case ast::C_BUILTIN_UINT:
                rv = T(*static_cast<unsigned int *>(av));
                break;
            case ast::C_BUILTIN_LONG:
                rv = T(*static_cast<long *>(av));
                break;
            case ast::C_BUILTIN_ULONG:
                rv = T(*static_cast<unsigned long *>(av));
                break;
            case ast::C_BUILTIN_LLONG:
                rv = T(*static_cast<long long *>(av));
                break;
            case ast::C_BUILTIN_ULLONG:
                rv = T(*static_cast<unsigned long long *>(av));
                break;
            case ast::C_BUILTIN_FLOAT:
                rv = T(*static_cast<float *>(av));
                break;
            case ast::C_BUILTIN_DOUBLE:
                rv = T(*static_cast<double *>(av));
                break;
            case ast::C_BUILTIN_LDOUBLE:
                rv = T(*static_cast<long double *>(av));
                break;
            default:
                return false;
        }
        return true;
    };
    int tp = cd->decl.type();
    if (cd->decl.is_ref()) {
        if (gf(tp, *static_cast<void **>(cd->as_ptr()), out)) {
            return true;
        }
    } else if (gf(tp, cd->as_ptr(), out)) {
        return true;
    }
    return false;
}

template <typename T>
static inline T check_arith(lua_State *L, int idx) {
    T outv{};
    if (!test_arith<T>(L, idx, outv)) {
        lua::type_error(L, idx, util::is_int<T>::value ? "integer" : "number");
    }
    return outv;
}

static inline ast::c_expr_type check_arith_expr(lua_State *L, int idx, ast::c_value &iv) {
    auto *cd = testcdata(L, idx);
    if (!cd) {
        /* some logic for conversions of lua numbers into cexprs */
#if LUA_VERSION_NUM >= 503
        static_assert(sizeof(lua_Integer) <= sizeof(long long), "invalid lua_Integer format");
        if (lua_isinteger(L, idx)) {
            if (util::is_signed<lua_Integer>::value) {
                if (sizeof(lua_Integer) <= sizeof(int)) {
                    iv.i = int(lua_tointeger(L, idx));
                    return ast::c_expr_type::INT;
                } else if (sizeof(lua_Integer) <= sizeof(long)) {
                    iv.l = long(lua_tointeger(L, idx));
                    return ast::c_expr_type::LONG;
                } else {
                    using LL = long long;
                    iv.ll = LL(lua_tointeger(L, idx));
                    return ast::c_expr_type::LLONG;
                }
            } else {
                if (sizeof(lua_Integer) <= sizeof(unsigned int)) {
                    using U = unsigned int;
                    iv.u = U(lua_tointeger(L, idx));
                    return ast::c_expr_type::UINT;
                } else if (sizeof(lua_Integer) <= sizeof(unsigned long)) {
                    using UL = unsigned long;
                    iv.ul = UL(lua_tointeger(L, idx));
                    return ast::c_expr_type::ULONG;
                } else {
                    using ULL = unsigned long long;
                    iv.ull = ULL(lua_tointeger(L, idx));
                    return ast::c_expr_type::ULLONG;
                }
            }
        }
#endif
        static_assert(util::is_int<lua_Number>::value ? (sizeof(lua_Number) <= sizeof(long long)) : (sizeof(lua_Number) <= sizeof(long double)), "invalid lua_Number format");
        auto n = luaL_checknumber(L, idx);
        if (util::is_int<lua_Number>::value) {
            if (util::is_signed<lua_Number>::value) {
                if (sizeof(lua_Number) <= sizeof(int)) {
                    iv.i = int(n);
                    return ast::c_expr_type::INT;
                } else if (sizeof(lua_Number) <= sizeof(long)) {
                    iv.l = long(n);
                    return ast::c_expr_type::LONG;
                } else {
                    using LL = long long;
                    iv.ll = LL(n);
                    return ast::c_expr_type::LLONG;
                }
            } else {
                if (sizeof(lua_Number) <= sizeof(unsigned int)) {
                    using U = unsigned int;
                    iv.u = U(n);
                    return ast::c_expr_type::UINT;
                } else if (sizeof(lua_Number) <= sizeof(unsigned long)) {
                    using UL = unsigned long;
                    iv.ul = UL(n);
                    return ast::c_expr_type::ULONG;
                } else {
                    using ULL = unsigned long long;
                    iv.ull = ULL(n);
                    return ast::c_expr_type::ULLONG;
                }
            }
        } else if (sizeof(lua_Number) <= sizeof(float)) {
            iv.f = float(n);
            return ast::c_expr_type::FLOAT;
        } else if (sizeof(lua_Number) <= sizeof(double)) {
            iv.d = double(n);
            return ast::c_expr_type::DOUBLE;
        }
        using LD = long double;
        iv.ld = LD(n);
        return ast::c_expr_type::LDOUBLE;
    }
    auto gf = [](int itp, void *av, ast::c_value &v) {
        switch (itp) {
            case ast::C_BUILTIN_ENUM:
                /* TODO: large enums */
                v.i = *static_cast<int *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_BOOL:
                v.i = *static_cast<bool *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_CHAR:
                v.i = *static_cast<char *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_SCHAR:
                v.i = *static_cast<signed char *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_UCHAR:
                v.i = *static_cast<unsigned char *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_SHORT:
                v.i = *static_cast<short *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_USHORT:
                v.i = *static_cast<unsigned short *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_INT:
                v.i = *static_cast<int *>(av);
                return ast::c_expr_type::INT;
            case ast::C_BUILTIN_UINT:
                v.u = *static_cast<unsigned int *>(av);
                return ast::c_expr_type::UINT;
            case ast::C_BUILTIN_LONG:
                v.l = *static_cast<long *>(av);
                return ast::c_expr_type::LONG;
            case ast::C_BUILTIN_ULONG:
                v.ul = *static_cast<unsigned long *>(av);
                return ast::c_expr_type::ULONG;
            case ast::C_BUILTIN_LLONG:
                v.ll = *static_cast<long long *>(av);
                return ast::c_expr_type::LLONG;
            case ast::C_BUILTIN_ULLONG:
                v.ull = *static_cast<unsigned long long *>(av);
                return ast::c_expr_type::ULLONG;
            case ast::C_BUILTIN_FLOAT:
                v.f = *static_cast<float *>(av);
                return ast::c_expr_type::FLOAT;
            case ast::C_BUILTIN_DOUBLE:
                v.d = *static_cast<double *>(av);
                return ast::c_expr_type::DOUBLE;
            case ast::C_BUILTIN_LDOUBLE:
                v.ld = *static_cast<long double *>(av);
                return ast::c_expr_type::LDOUBLE;
            default:
                return ast::c_expr_type::INVALID;
        }
    };
    ast::c_expr_type ret;
    int tp = cd->decl.type();
    if (cd->decl.is_ref()) {
        ret = gf(tp, *static_cast<void **>(cd->as_ptr()), iv);
    } else {
        ret = gf(tp, cd->as_ptr(), iv);
    }
    if (ret == ast::c_expr_type::INVALID) {
        luaL_checknumber(L, idx);
    }
    /* unreachable */
    return ret;
}

static inline cdata &make_cdata_arith(lua_State *L, ast::c_expr_type et, ast::c_value const &cv) {
    auto bt = ast::to_builtin_type(et);
    if (bt == ast::C_BUILTIN_INVALID) {
        luaL_error(L, "invalid value type");
    }
    auto tp = ast::c_type{bt, 0};
    auto as = tp.alloc_size();
    auto &cd = newcdata(L, util::move(tp), as);
    std::memcpy(cd.as_ptr(), &cv, as);
    return cd;
}

static inline char const *lua_serialize(lua_State *L, int idx) {
    auto *cd = testcdata(L, idx);
    if (cd) {
        /* it's ok to mess up the lua stack, this is only used for errors */
        cd->decl.serialize(L);
        return lua_tostring(L, -1);
    }
    return lua_typename(L, lua_type(L, idx));
}

} /* namespace ffi */

#endif /* LIBFFI_HH */
