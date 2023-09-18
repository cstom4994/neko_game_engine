
#ifndef NEKO_STR_H
#define NEKO_STR_H

#include <string>
#include <string_view>

#include "engine/common/neko_types.h"

// template <int SIZE>
// struct __neko_static_string;

struct neko_stringview {
    neko_stringview() {}
    neko_stringview(const char *str) : begin(str), end(str ? str + std::strlen(str) : 0) {}
    neko_stringview(const char *str, u32 len) : begin(str), end(str + len) {}
    neko_stringview(const char *begin, const char *end) : begin(begin), end(end) {}
    // template <int N>
    // neko_stringview(const __neko_static_string<N> &str);

    u32 size() const { return u32(end - begin); }
    char operator[](u32 idx) {
        neko_assert(!end || begin + idx < end);
        return begin[idx];
    }
    char back() const {
        neko_assert(end && begin != end);
        return *(end - 1);
    }
    void removeSuffix(u32 count) {
        neko_assert(count <= size());
        end -= count;
    }
    void removePrefix(u32 count) {
        neko_assert(count <= size());
        begin += count;
    }
    bool empty() const { return begin == end || !begin[0]; }

    const char *begin = nullptr;
    const char *end = nullptr;
};

// template <int SIZE>
// struct __neko_static_string {
//     __neko_static_string() { data[0] = '\0'; }
//
//     template <typename... Args>
//     __neko_static_string(Args... args) {
//         data[0] = '\0';
//         append(args...);
//     }
//
//     template <typename... Args>
//     void append(Args... args) {
//         Span dst(data + stringLength(data), data + SIZE);
//         int tmp[] = {(add(args, dst), 0)...};
//         (void)tmp;
//     }
//
//     void operator=(const char *str) { copyString(data, str); }
//     bool operator<(const char *str) const { return compareString(data, str) < 0; }
//     bool operator==(const char *str) const { return equalStrings(data, str); }
//     bool operator!=(const char *str) const { return !equalStrings(data, str); }
//
//     bool empty() const { return data[0] == '\0'; }
//     operator const char *() const { return data; }
//
//     char data[SIZE];
//
// private:
//     template <int S>
//     void add(__neko_static_string<S> &value, Span<char> &dst) {
//         dst.m_begin = copyString(dst, value);
//     }
//     void add(StringView value, Span<char> &dst) { dst.m_begin = copyString(dst, value); }
//     void add(StableHash value, Span<char> &dst) { add(value.getHashValue(), dst); }
//     void add(float value, Span<char> &dst) { dst.m_begin = toCString(value, dst, 3); }
//     void add(double value, Span<char> &dst) { dst.m_begin = toCString(value, dst, 10); }
//     void add(u32 value, Span<char> &dst) { dst.m_begin = toCString(value, dst); }
//     void add(i32 value, Span<char> &dst) { dst.m_begin = toCString(value, dst); }
//     void add(u64 value, Span<char> &dst) { dst.m_begin = toCString(value, dst); }
//     void add(i64 value, Span<char> &dst) { dst.m_begin = toCString(value, dst); }
//
//     void add(char value, Span<char> &dst) {
//         if (dst.length() < 2) return;
//         dst[0] = value;
//         dst[1] = '\0';
//         dst.m_begin += 1;
//     }
// };

neko_inline bool neko_str_is_chinese_c(const char str) { return str & 0x80; }

neko_inline bool neko_str_is_chinese_str(const neko_string &str) {
    for (int i = 0; i < str.length(); i++)
        if (neko_str_is_chinese_c(str[i])) return true;
    return false;
}

neko_inline bool neko_str_equals(const char *a, const char *c) { return strcmp(a, c) == 0; }

neko_inline bool neko_str_starts_with(neko_string_view s, neko_string_view prefix) { return prefix.size() <= s.size() && (strncmp(prefix.data(), s.data(), prefix.size()) == 0); }

neko_inline bool neko_str_starts_with(neko_string_view s, char prefix) { return !s.empty() && s[0] == prefix; }

neko_inline bool neko_str_starts_with(const char *s, const char *prefix) { return strncmp(s, prefix, strlen(prefix)) == 0; }

neko_inline bool neko_str_ends_with(neko_string_view s, neko_string_view suffix) {
    return suffix.size() <= s.size() && strncmp(suffix.data(), s.data() + s.size() - suffix.size(), suffix.size()) == 0;
}

neko_inline bool neko_str_ends_with(neko_string_view s, char suffix) { return !s.empty() && s[s.size() - 1] == suffix; }

neko_inline bool neko_str_ends_with(const char *s, const char *suffix) {
    auto sizeS = strlen(s);
    auto sizeSuf = strlen(suffix);

    return sizeSuf <= sizeS && strncmp(suffix, s + sizeS - sizeSuf, sizeSuf) == 0;
}

neko_inline void neko_str_to_lower(char *s) {
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

neko_inline void neko_str_to_lower(neko_string &ss) {
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

neko_inline void neko_str_to_upper(char *s) {
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

neko_inline void neko_str_to_upper(neko_string &ss) {
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

neko_inline bool neko_str_replace_with(char *src, char what, char with) {
    for (int i = 0; true; ++i) {
        auto &id = src[i];
        if (id == '\0') return true;
        bool isWhat = id == what;
        id = isWhat * with + src[i] * (!isWhat);
    }
}

neko_inline bool neko_str_replace_with(neko_string &src, char what, char with) {
    for (int i = 0; i < src.size(); ++i) {
        auto &id = src.data()[i];
        bool isWhat = id == what;
        id = isWhat * with + src[i] * (!isWhat);
    }
    return true;
}

neko_inline bool neko_str_replace_with(neko_string &src, const char *what, const char *with) {
    neko_string out;
    size_t whatlen = strlen(what);
    out.reserve(src.size());
    size_t ind = 0;
    size_t lastInd = 0;
    while (true) {
        ind = src.find(what, ind);
        if (ind == neko_string::npos) {
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

neko_inline bool neko_str_replace_with(neko_string &src, const char *what, const char *with, int times) {
    for (int i = 0; i < times; ++i) neko_str_replace_with(src, what, with);
    return true;
}

neko_inline char *neko_str_copy(neko_span<char> dst, neko_stringview src) {
    if (dst.length() < 1) return dst.begin();

    neko_assert(dst.begin() >= src.end || dst.begin() <= src.begin);

    u32 length = dst.length();
    char *tmp = dst.begin();
    const char *srcp = src.begin;
    while (srcp != src.end && length > 1) {
        *tmp = *srcp;
        --length;
        ++tmp;
        ++srcp;
    }
    *tmp = 0;
    return tmp;
}

#endif  // !NEKO_STR_H
