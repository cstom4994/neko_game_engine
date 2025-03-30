#include "engine/base.hpp"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <new>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "engine/bootstrap.h"
#include "engine/graphics.h"
#include "base/scripting/lua_wrapper.hpp"
#include "base/scripting/scripting.h"
#include "extern/luaalloc.h"

static void _error(const char *s) { script_error(s); }

void errorf(const char *fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char *s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char *)mem_alloc(n + 1);
    vsprintf(s, fmt, ap1);
    va_end(ap1);
    _error(s);
    mem_free(s);
}

// ===============================================================

#if 0

vec2 vec2_add(vec2 u, vec2 v) { return luavec2(u.x + v.x, u.y + v.y); }
vec2 vec2_sub(vec2 u, vec2 v) { return luavec2(u.x - v.x, u.y - v.y); }
vec2 vec2_mul(vec2 u, vec2 v) { return luavec2(u.x * v.x, u.y * v.y); }
vec2 vec2_div(vec2 u, vec2 v) { return luavec2(u.x / v.x, u.y / v.y); }


f32 vec2_len(vec2 v) { return float_sqrt(v.x * v.x + v.y * v.y); }

f32 vec2_dot(vec2 u, vec2 v) { return u.x * v.x + u.y * v.y; }
f32 vec2_dist(vec2 u, vec2 v) { return vec2_len(vec2_sub(u, v)); }

#endif

vec2 vec2_normalize(vec2 v) {
    if (v.x == 0 && v.y == 0) return v;
    return vec2_float_div(v, vec2_len(v));
}

vec2 vec2_neg(vec2 v) { return luavec2(-v.x, -v.y); }

f32 vec2_atan2(vec2 v) { return float_atan2(v.y, v.x); }

vec2 vec2_add(vec2 v0, vec2 v1) { return vec2_ctor(v0.x + v1.x, v0.y + v1.y); }

vec2 vec2_sub(vec2 v0, vec2 v1) { return vec2_ctor(v0.x - v1.x, v0.y - v1.y); }

vec2 vec2_mul(vec2 v0, vec2 v1) { return vec2_ctor(v0.x * v1.x, v0.y * v1.y); }

vec2 vec2_div(vec2 v0, vec2 v1) { return vec2_ctor(v0.x / v1.x, v0.y / v1.y); }

vec2 vec2_zero = {0.0, 0.0};

vec2 vec2_rot(vec2 v, f32 rot) { return luavec2(v.x * float_cos(rot) - v.y * float_sin(rot), v.x * float_sin(rot) + v.y * float_cos(rot)); }

vec2 vec2_float_mul(vec2 v, f32 f) { return luavec2(v.x * f, v.y * f); }
vec2 vec2_float_div(vec2 v, f32 f) { return luavec2(v.x / f, v.y / f); }
vec2 float_vec2_div(f32 f, vec2 v) { return luavec2(f / v.x, f / v.y); }

#undef luavec2
vec2 luavec2(f32 x, f32 y) { return vec2{x, y}; }

#if 0

mat3 mat3_mul(mat3 m, mat3 n) {
    return luamat3(m.m[0][0] * n.m[0][0] + m.m[1][0] * n.m[0][1] + m.m[2][0] * n.m[0][2], m.m[0][1] * n.m[0][0] + m.m[1][1] * n.m[0][1] + m.m[2][1] * n.m[0][2],
                   m.m[0][2] * n.m[0][0] + m.m[1][2] * n.m[0][1] + m.m[2][2] * n.m[0][2],

                   m.m[0][0] * n.m[1][0] + m.m[1][0] * n.m[1][1] + m.m[2][0] * n.m[1][2], m.m[0][1] * n.m[1][0] + m.m[1][1] * n.m[1][1] + m.m[2][1] * n.m[1][2],
                   m.m[0][2] * n.m[1][0] + m.m[1][2] * n.m[1][1] + m.m[2][2] * n.m[1][2],

                   m.m[0][0] * n.m[2][0] + m.m[1][0] * n.m[2][1] + m.m[2][0] * n.m[2][2], m.m[0][1] * n.m[2][0] + m.m[1][1] * n.m[2][1] + m.m[2][1] * n.m[2][2],
                   m.m[0][2] * n.m[2][0] + m.m[1][2] * n.m[2][1] + m.m[2][2] * n.m[2][2]);
}


mat3 mat3_inverse(mat3 m) {
    f32 det;
    mat3 inv;

    inv.m[0][0] = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];`
    inv.m[0][1] = m.m[0][2] * m.m[2][1] - m.m[0][1] * m.m[2][2];
    inv.m[0][2] = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
    inv.m[1][0] = m.m[1][2] * m.m[2][0] - m.m[1][0] * m.m[2][2];
    inv.m[1][1] = m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0];
    inv.m[1][2] = m.m[0][2] * m.m[1][0] - m.m[0][0] * m.m[1][2];
    inv.m[2][0] = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];
    inv.m[2][1] = m.m[0][1] * m.m[2][0] - m.m[0][0] * m.m[2][1];
    inv.m[2][2] = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];

    det = m.m[0][0] * inv.m[0][0] + m.m[0][1] * inv.m[1][0] + m.m[0][2] * inv.m[2][0];

    if (det <= 10e-8) return inv;  // TODO: figure out what to do if not invertible

    inv.m[0][0] /= det;
    inv.m[0][1] /= det;
    inv.m[0][2] /= det;
    inv.m[1][0] /= det;
    inv.m[1][1] /= det;
    inv.m[1][2] /= det;
    inv.m[2][0] /= det;
    inv.m[2][1] /= det;
    inv.m[2][2] /= det;

    return inv;
}

#endif

vec2 mat3_transform(mat3 m, vec2 v) { return luavec2(m.v[0] * v.x + m.v[3] * v.y + m.v[6], m.v[1] * v.x + m.v[4] * v.y + m.v[7]); }

// 按顺序应用 scale rot 和 trans 的矩阵
mat3 mat3_scaling_rotation_translation(vec2 scale, f32 rot, vec2 trans) {
    return luamat3(scale.x * float_cos(rot), scale.x * float_sin(rot), 0.0f, scale.y * -float_sin(rot), scale.y * float_cos(rot), 0.0f, trans.x, trans.y, 1.0f);
}

vec2 mat3_get_translation(mat3 m) { return luavec2(m.v[6], m.v[7]); }
f32 mat3_get_rotation(mat3 m) { return float_atan2(m.v[1], m.v[0]); }
vec2 mat3_get_scale(mat3 m) { return luavec2(float_sqrt(m.v[0] * m.v[0] + m.v[1] * m.v[1]), float_sqrt(m.v[3] * m.v[3] + m.v[4] * m.v[4])); }

#undef luamat3
mat3 luamat3(f32 m00, f32 m01, f32 m02, f32 m10, f32 m11, f32 m12, f32 m20, f32 m21, f32 m22) { return mat3{{m00, m01, m02, m10, m11, m12, m20, m21, m22}}; }

mat3 mat3_identity() { return mat3_diag(1.f); }

BBox bbox_merge(BBox a, BBox b) { return bbox(luavec2(float_min(a.min.x, b.min.x), float_min(a.min.y, b.min.y)), luavec2(float_max(a.max.x, b.max.x), float_max(a.max.y, b.max.y))); }
BBox bbox_bound(vec2 a, vec2 b) { return bbox(luavec2(float_min(a.x, b.x), float_min(a.y, b.y)), luavec2(float_max(a.x, b.x), float_max(a.y, b.y))); }
bool bbox_contains(BBox b, vec2 p) { return b.min.x <= p.x && p.x <= b.max.x && b.min.y <= p.y && p.y <= b.max.y; }

BBox bbox(vec2 min, vec2 max) {
    BBox bb;
    bb.min = min;
    bb.max = max;
    return bb;
}

BBox bbox_transform(mat3 m, BBox b) {
    vec2 v1, v2, v3, v4;

    v1 = mat3_transform(m, luavec2(b.min.x, b.min.y));
    v2 = mat3_transform(m, luavec2(b.max.x, b.min.y));
    v3 = mat3_transform(m, luavec2(b.max.x, b.max.y));
    v4 = mat3_transform(m, luavec2(b.min.x, b.max.y));

    return bbox_merge(bbox_bound(v1, v2), bbox_bound(v3, v4));
}

Color color_black = {0.0, 0.0, 0.0, 1.0};
Color color_white = {1.0, 1.0, 1.0, 1.0};
Color color_gray = {0.5, 0.5, 0.5, 1.0};
Color color_red = {1.0, 0.0, 0.0, 1.0};
Color color_green = {0.0, 1.0, 0.0, 1.0};
Color color_blue = {0.0, 0.0, 1.0, 1.0};
Color color_clear = {0.0, 0.0, 0.0, 0.0};

#undef color_opaque
Color color_opaque(f32 r, f32 g, f32 b) { return color(r, g, b, 1); }

#undef color
Color color(f32 r, f32 g, f32 b, f32 a) { return Color{r, g, b, a}; }

/*================================================================================
// Deps
================================================================================*/

#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#endif

#define SOKOL_TIME_IMPL
#include "extern/sokol_time.h"

#define STBI_MALLOC(sz) mem_alloc(sz)
#define STBI_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBI_FREE(p) mem_free(p)

#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"

#define STBIR_MALLOC(size, user_data) ((void)(user_data), mem_alloc(size))
#define STBIR_FREE(ptr, user_data) ((void)(user_data), mem_free(ptr))

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "extern/stb_image_resize2.h"

#define STBIW_MALLOC(sz) mem_alloc(sz)
#define STBIW_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBIW_FREE(p) mem_free(p)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "extern/stb_rect_pack.h"

#define STBTT_malloc(x, u) ((void)(u), mem_alloc(x))
#define STBTT_free(x, u) ((void)(u), mem_free(x))

#define STB_TRUETYPE_IMPLEMENTATION
#include "extern/stb_truetype.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#if NEKO_AUDIO == 1
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_WEBAUDIO
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
