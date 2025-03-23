#pragma once

#include "base/common/base.hpp"
#include "base/common/mem.hpp"

namespace Neko {

struct String {
    char* data = nullptr;
    u64 len = 0;

    String() = default;
    String(const char* cstr) : data((char*)cstr), len(strlen(cstr)) {}
    String(const char* cstr, u64 n) : data((char*)cstr), len(n) {}
    String(const std::string& str) : data((char*)str.data()), len(str.length()) {}

    inline bool is_cstr() { return data[len] == '\0'; }

    String substr(u64 i, u64 j) {
        assert(i <= j);
        assert(j <= (i64)len);
        return {&data[i], j - i};
    }

    inline bool equel(String& rhs) {
        if (this->len != rhs.len) {
            return false;
        }
        return memcmp(this->data, rhs.data, this->len) == 0;
    }

    // inline bool operator!=(String rhs) { return !(*this == rhs); }

    bool starts_with(String match) {
        if (len < match.len) {
            return false;
        }
        return substr(0, match.len).equel(match);
    }

    bool ends_with(String match) {
        if (len < match.len) {
            return false;
        }
        return substr(len - match.len, len).equel(match);
    }

    u64 first_of(char c) {
        for (u64 i = 0; i < len; i++) {
            if (data[i] == c) {
                return i;
            }
        }

        return (u64)-1;
    }

    u64 last_of(char c) {
        for (u64 i = len; i > 0; i--) {
            if (data[i - 1] == c) {
                return i - 1;
            }
        }

        return (u64)-1;
    }

    inline const_str cstr() const { return data; }

    char* begin() { return data; }
    char* end() { return &data[len]; }
};

inline bool operator==(String lhs, String rhs) {
    if (lhs.len != rhs.len) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

inline bool operator!=(String lhs, String rhs) { return !(lhs == rhs); }

String to_cstr(String str);

inline String to_cstr(std::string str) { return to_cstr(String(str.c_str())); };

inline constexpr u64 fnv1a(const char* str, u64 len) {
    u64 hash = 14695981039346656037u;
    for (u64 i = 0; i < len; i++) {
        hash ^= (u8)str[i];
        hash *= 1099511628211;
    }
    return hash;
}

inline u64 fnv1a(String str) { return fnv1a(str.data, str.len); }

inline constexpr u64 operator"" _hash(const char* str, size_t len) { return fnv1a(str, len); }

inline String to_cstr(String str) {
    char* buf = (char*)mem_alloc(str.len + 1);
    memcpy(buf, str.data, str.len);
    buf[str.len] = 0;
    return {buf, str.len};
}

char* b64_encode(const char* src, size_t srclen, size_t linelen, size_t& dstlen);
char* b64_decode(const char* src, size_t srclen, size_t& size);

struct SplitLinesIterator {
    String data;
    String view;

    String operator*() const { return view; }
    inline SplitLinesIterator& operator++() {
        if (&view.data[view.len] == &data.data[data.len]) {
            view = {&data.data[data.len], 0};
            return *this;
        }

        String next = {};
        next.data = view.data + view.len + 1;

        u64 end = 0;
        while (&next.data[end] < &data.data[data.len] && next.data[end] != '\n' && next.data[end] != 0) {
            end++;
        }
        next.len = end;

        view = next;
        return *this;
    }
};

inline bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs) {
    return lhs.data.data != rhs.data.data || lhs.data.len != rhs.data.len || lhs.view.data != rhs.view.data || lhs.view.len != rhs.view.len;
}

struct SplitLines {
    String str;
    SplitLines(String s) : str(s) {}

    inline SplitLinesIterator begin() {
        char* data = str.data;
        u64 end = 0;
        while (data[end] != '\n' && data[end] != 0) {
            end++;
        }

        String view = {str.data, end};
        return {str, view};
    }

    inline SplitLinesIterator end() {
        String view = {str.data + str.len, 0};
        return {str, view};
    }
};

inline i32 utf8_size(u8 c) {
    if (c == '\0') {
        return 0;
    }

    if ((c & 0xF8) == 0xF0) {
        return 4;
    } else if ((c & 0xF0) == 0xE0) {
        return 3;
    } else if ((c & 0xE0) == 0xC0) {
        return 2;
    } else {
        return 1;
    }
}

struct Rune {
    u32 value;

    inline u32 charcode() {
        u32 charcode = 0;

        u8 c0 = value >> 0;
        u8 c1 = value >> 8;
        u8 c2 = value >> 16;
        u8 c3 = value >> 24;

        switch (utf8_size(c0)) {
            case 1:
                charcode = c0;
                break;
            case 2:
                charcode = c0 & 0x1F;
                charcode = (charcode << 6) | (c1 & 0x3F);
                break;
            case 3:
                charcode = c0 & 0x0F;
                charcode = (charcode << 6) | (c1 & 0x3F);
                charcode = (charcode << 6) | (c2 & 0x3F);
                break;
            case 4:
                charcode = c0 & 0x07;
                charcode = (charcode << 6) | (c1 & 0x3F);
                charcode = (charcode << 6) | (c2 & 0x3F);
                charcode = (charcode << 6) | (c3 & 0x3F);
                break;
        }

        return charcode;
    }

    inline bool is_whitespace() {
        switch (value) {
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                return true;
        }
        return false;
    }

    inline bool is_digit() { return value >= '0' && value <= '9'; }
};

inline Rune rune_from_string(const char* data) {
    u32 rune = 0;
    i32 len = utf8_size(data[0]);
    for (i32 i = len - 1; i >= 0; i--) {
        rune <<= 8;
        rune |= (u8)(data[i]);
    }

    return {rune};
}

struct UTF8Iterator {
    String str;
    u64 cursor;
    Rune rune;

    Rune operator*() const { return rune; }

    static void next_rune(UTF8Iterator* it) {
        if (it->cursor == it->str.len) {
            it->rune.value = 0;
            return;
        }

        char* data = &it->str.data[it->cursor];
        i32 len = utf8_size(data[0]);
        Rune rune = rune_from_string(data);

        it->cursor += len;
        it->rune = rune;
    }

    inline UTF8Iterator& operator++() {
        next_rune(this);
        return *this;
    }
};

inline bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs) { return lhs.str.data != rhs.str.data || lhs.str.len != rhs.str.len || lhs.cursor != rhs.cursor || lhs.rune.value != rhs.rune.value; }

struct UTF8 {
    String str;
    UTF8(String s) : str(s) {}

    inline UTF8Iterator begin() {
        UTF8Iterator it = {};
        it.str = str;
        UTF8Iterator::next_rune(&it);
        return it;
    }

    inline UTF8Iterator end() { return {str, str.len, {}}; }
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

struct StringScanner {
    char* data;
    u64 len;
    u64 pos;
    u64 end;

    StringScanner(String str) {
        data = str.data;
        len = str.len;
        pos = 0;
        end = 0;
    }

    static void advance(StringScanner* s) { s->end += utf8_size(s->data[s->end]); }
    static bool at_end(StringScanner* s) { return s->end >= s->len; }

    static Rune peek(StringScanner* s) {
        if (at_end(s)) {
            return {0};
        } else {
            return rune_from_string(&s->data[s->end]);
        }
    }

    static void skip_whitespace(StringScanner* s) {
        while (peek(s).is_whitespace() && !at_end(s)) {
            advance(s);
        }
    }

    inline String next_string() {
        skip_whitespace(this);
        pos = end;

        if (at_end(this)) {
            return "";
        }

        while (!peek(this).is_whitespace() && !at_end(this)) {
            advance(this);
        }

        return {&data[pos], end - pos};
    }

    inline i32 next_int() {
        skip_whitespace(this);
        pos = end;

        if (at_end(this)) {
            return 0;
        }

        i32 sign = 1;
        if (peek(this).value == '-') {
            sign = -1;
            advance(this);
        }

        i32 num = 0;
        while (peek(this).is_digit()) {
            num *= 10;
            num += peek(this).value - '0';
            advance(this);
        }

        return num * sign;
    }
};

inline u32 neko_string_length(const char* txt) {
    u32 sz = 0;
    while (txt != NULL && txt[sz] != '\0') sz++;
    return sz;
}

#define neko_strlen(str) neko_string_length((const char*)str)

inline bool neko_string_compare_equal(const String& txt, const String& cmp) { return txt == cmp; }

inline bool neko_string_compare_equal(const char* txt, const char* cmp) {
    u32 a_sz = neko_strlen(txt);
    u32 b_sz = neko_strlen(cmp);
    if (a_sz != b_sz) return false;
    for (u32 i = 0; i < a_sz; ++i)
        if (*txt++ != *cmp++) {
            return false;
        }
    return true;
}

inline String StringCopy(const_str str, u32 len) {
    StringBuilder sb = {};
    sb << String(str, len);
    return String(sb);
}

inline bool neko_string_is_decimal(const_str str, u32 len) {
    u32 i = 0;
    if (str[0] == '-') i++;
    bool used_dot = false;
    for (; i < len; i++) {
        char c = str[i];
        if (c < '0' || c > '9') {
            if (c == '.' && !used_dot) {
                used_dot = true;
                continue;
            }
            return false;
        }
    }
    return true;
}

inline bool StringEqualN(const_str str_a, u32 len, const_str str_b) {
    for (u32 i = 0; i < len; i++) {
        if (str_a[i] != str_b[i]) return false;
    }
    return true;
}

inline bool neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }

inline bool is_whitespace(char c) {
    switch (c) {
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

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
        neko_assert(n > 0);
        if (code_point > 0x10000) {
            neko_assert(code_point < 0x10FFFF);
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
    neko_assert(output_len == 0);
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
    neko_assert(output_len == 0);
}

}  // namespace Neko

namespace Neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
}  // namespace Neko::wtf8

template <>
struct std::hash<Neko::String> {
    std::size_t operator()(const Neko::String& k) const { return Neko::fnv1a(k); }
};