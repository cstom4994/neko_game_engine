
#include "math.hpp"

// ===============================================================

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

vec2 mat3_transform(mat3 m, vec2 v) { return luavec2(m.v[0] * v.x + m.v[3] * v.y + m.v[6], m.v[1] * v.x + m.v[4] * v.y + m.v[7]); }

// 按顺序应用 scale rot 和 trans 的矩阵
mat3 mat3_scaling_rotation_translation(vec2 scale, f32 rot, vec2 trans) {
    // clang-format off
    return luamat3(
        scale.x *  float_cos(rot),  scale.x * float_sin(rot), 0.0f,
        scale.y * -float_sin(rot),  scale.y * float_cos(rot), 0.0f,
        trans.x,                    trans.y,                  1.0f
    );
    // clang-format on
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
