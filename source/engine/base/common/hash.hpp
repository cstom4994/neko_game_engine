#pragma once

#include "engine/base/common/base.hpp"
#include "engine/base/common/string.hpp"

namespace Neko {

inline u32 neko_hash_u32(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

inline u32 neko_hash_u64(u64 x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (u32)x;
}

// Note: source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
inline u32 neko_hash_str(const char *str) {
    u32 hash = 5381;
    i32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

inline u64 neko_hash_str64(const char *str) {
    u32 hash1 = 5381;
    u32 hash2 = 52711;
    u32 i = neko_strlen(str);
    while (i--) {
        char c = str[i];
        hash1 = (hash1 * 33) ^ c;
        hash2 = (hash2 * 33) ^ c;
    }

    return (hash1 >> 0) * 4096 + (hash2 >> 0);
}

}  // namespace Neko

template <>
struct std::hash<Neko::String> {
    std::size_t operator()(const Neko::String &k) const { return Neko::fnv1a(k); }
};