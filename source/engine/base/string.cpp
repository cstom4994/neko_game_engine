#include "engine/base/string.hpp"

static char s_empty[1] = {0};

StringBuilder::StringBuilder() {
    data = s_empty;
    len = 0;
    capacity = 0;
}

void StringBuilder::trash() {
    if (data != s_empty) {
        mem_free(data);
    }
}

void StringBuilder::reserve(u64 cap) {
    if (cap > capacity) {
        char *buf = (char *)mem_alloc(cap);
        memset(buf, 0, cap);
        memcpy(buf, data, len);

        if (data != s_empty) {
            mem_free(data);
        }

        data = buf;
        capacity = cap;
    }
}

void StringBuilder::clear() {
    len = 0;
    if (data != s_empty) {
        data[0] = 0;
    }
}

void StringBuilder::swap_filename(String filepath, String file) {
    clear();

    u64 slash = filepath.last_of('/');
    if (slash != (u64)-1) {
        String path = filepath.substr(0, slash + 1);
        *this << path;
    }

    *this << file;
}

void StringBuilder::concat(String str, i32 times) {
    for (i32 i = 0; i < times; i++) {
        *this << str;
    }
}

StringBuilder &StringBuilder::operator<<(String str) {
    u64 desired = len + str.len + 1;
    u64 cap = capacity;

    if (desired >= cap) {
        u64 growth = cap > 0 ? cap * 2 : 8;
        if (growth <= desired) {
            growth = desired;
        }

        reserve(growth);
    }

    memcpy(&data[len], str.data, str.len);
    len += str.len;
    data[len] = 0;
    return *this;
}

StringBuilder::operator String() { return {data, len}; }

String str_fmt(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len > 0) {
        char *data = (char *)mem_alloc(len + 1);
        va_start(args, fmt);
        vsnprintf(data, len + 1, fmt, args);
        va_end(args);
        return {data, (u64)len};
    }

    return {};
}

String tmp_fmt(const char *fmt, ...) {
    static char s_buf[1024] = {};

    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(s_buf, sizeof(s_buf), fmt, args);
    va_end(args);
    return {s_buf, (u64)len};
}

double string_to_double(String str) {
    double n = 0;
    double sign = 1;

    if (str.len == 0) {
        return n;
    }

    u64 i = 0;
    if (str.data[0] == '-' && str.len > 1 && is_digit(str.data[1])) {
        i++;
        sign = -1;
    }

    while (i < str.len) {
        if (!is_digit(str.data[i])) {
            break;
        }

        n = n * 10 + (str.data[i] - '0');
        i++;
    }

    if (i < str.len && str.data[i] == '.') {
        i++;
        double place = 10;
        while (i < str.len) {
            if (!is_digit(str.data[i])) {
                break;
            }

            n += (str.data[i] - '0') / place;
            place *= 10;
            i++;
        }
    }

    return n * sign;
}

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    size_t wlen = wtf8_to_utf16_length(str.data(), str.size());
    if (wlen == (size_t)-1) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    wtf8_to_utf16(str.data(), str.size(), wresult.data(), wlen);
    return wresult;
}

std::string w2u(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    size_t len = wtf8_from_utf16_length(wstr.data(), wstr.size());
    std::string result(len, '\0');
    wtf8_from_utf16(wstr.data(), wstr.size(), result.data(), len);
    return result;
}
}  // namespace neko::wtf8