#include "base/common/string.hpp"

#include "base/common/base.hpp"

namespace Neko {

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

    neko_assert(fmt != s_buf);  // 检测tmp_fmt是否被递归调用

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

static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

// 将3个8位字节编码为4个6位字符
static void b64_encode_block(char in[3], char out[4], int len) {
    out[0] = (char)cb64[(int)((in[0] & 0xfc) >> 2)];
    out[1] = (char)cb64[(int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4))];
    out[2] = (char)(len > 1 ? cb64[(int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6))] : '=');
    out[3] = (char)(len > 2 ? cb64[(int)(in[2] & 0x3f)] : '=');
}

char *b64_encode(const char *src, size_t srclen, size_t linelen, size_t &dstlen) {
    if (linelen == 0) linelen = std::numeric_limits<size_t>::max();
    size_t blocksout = 0, srcpos = 0;
    size_t adjustment = (srclen % 3) ? (3 - (srclen % 3)) : 0;
    size_t paddedlen = ((srclen + adjustment) / 3) * 4;
    dstlen = paddedlen + paddedlen / linelen;
    if (dstlen == 0) return nullptr;
    char *dst = (char *)mem_alloc(sizeof(char) * (dstlen + 1));
    size_t dstpos = 0;
    while (srcpos < srclen) {
        char in[3] = {0};
        char out[4] = {0};
        int len = 0;
        for (int i = 0; i < 3; i++) {
            if (srcpos >= srclen) break;
            in[i] = src[srcpos++];
            len++;
        }
        if (len > 0) {
            b64_encode_block(in, out, len);
            for (int i = 0; i < 4 && dstpos < dstlen; i++, dstpos++) dst[dstpos] = out[i];
            blocksout++;
        }
        if (blocksout >= linelen / 4 || srcpos >= srclen) {
            if (blocksout > 0 && dstpos < dstlen) dst[dstpos++] = '\n';
            blocksout = 0;
        }
    }
    dst[dstpos] = '\0';
    return dst;
}

static void b64_decode_block(char in[4], char out[3]) {
    out[0] = (char)(in[0] << 2 | in[1] >> 4);
    out[1] = (char)(in[1] << 4 | in[2] >> 2);
    out[2] = (char)(((in[2] << 6) & 0xc0) | in[3]);
}

char *b64_decode(const char *src, size_t srclen, size_t &size) {
    size_t paddedsize = (srclen / 4) * 3;
    char *dst = (char *)mem_alloc(sizeof(char) * (paddedsize));
    char *d = dst;
    char in[4] = {0};
    char out[3] = {0};
    size_t i, len, srcpos = 0;
    while (srcpos <= srclen) {
        for (len = 0, i = 0; i < 4 && srcpos <= srclen; i++) {
            char v = 0;
            while (srcpos <= srclen && v == 0) {
                v = src[srcpos++];
                v = (char)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
                if (v != 0) v = (char)((v == '$') ? 0 : v - 61);
            }
            if (srcpos <= srclen) {
                len++;
                if (v != 0) in[i] = (char)(v - 1);
            } else
                in[i] = 0;
        }
        if (len) {
            b64_decode_block(in, out);
            for (i = 0; i < len - 1; i++) *(d++) = out[i];
        }
    }
    size = (size_t)(ptrdiff_t)(d - dst);
    return dst;
}

}  // namespace Neko

namespace Neko::wtf8 {
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
}  // namespace Neko::wtf8
