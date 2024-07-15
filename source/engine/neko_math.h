
#ifndef NEKO_MATH_H
#define NEKO_MATH_H

#include "neko.h"

/*========================
// NEKO_MATH
========================*/

// Defines
#define neko_pi 3.14159265358979323846264f
#define neko_tau 2.0 * neko_pi
#define neko_e 2.71828182845904523536f  // e
#define neko_epsilon (1e-6)

// 实用
#define neko_v2(...) neko_vec2_ctor(__VA_ARGS__)
#define neko_v3(...) neko_vec3_ctor(__VA_ARGS__)
#define neko_v4(...) neko_vec4_ctor(__VA_ARGS__)
#define neko_quat(...) neko_quat_ctor(__VA_ARGS__)

#define neko_v2s(__S) neko_vec2_ctor((__S), (__S))
#define neko_v3s(__S) neko_vec3_ctor((__S), (__S), (__S))
#define neko_v4s(__S) neko_vec4_ctor((__S), (__S), (__S), (__S))

#define neko_v4_xy_v(__X, __Y, __V) neko_vec4_ctor((__X), (__Y), (__V).x, (__V).y)
#define neko_v4_xyz_s(__XYZ, __S) neko_vec4_ctor((__XYZ).x, (__XYZ).y, (__XYZ).z, (__S))

#define NEKO_XAXIS neko_v3(1.f, 0.f, 0.f)
#define NEKO_YAXIS neko_v3(0.f, 1.f, 0.f)
#define NEKO_ZAXIS neko_v3(0.f, 0.f, 1.f)

/*================================================================================
// Useful Common Math Functions
================================================================================*/

#define neko_rad2deg(__R) (float)((__R * 180.0f) / neko_pi)

#define neko_deg2rad(__D) (float)((__D * neko_pi) / 180.0f)

// Interpolation
// Source: https://codeplea.com/simple-interpolation

// Returns v based on t
NEKO_FORCE_INLINE float neko_interp_linear(float a, float b, float t) { return (a + t * (b - a)); }

// Returns t based on v
NEKO_FORCE_INLINE float neko_interp_linear_inv(float a, float b, float v) { return (v - a) / (b - a); }

NEKO_FORCE_INLINE float neko_interp_smoothstep(float a, float b, float t) { return neko_interp_linear(a, b, t * t * (3.0f - 2.0f * t)); }

NEKO_FORCE_INLINE float neko_interp_cosine(float a, float b, float t) { return neko_interp_linear(a, b, (float)-cos(neko_pi * t) * 0.5f + 0.5f); }

NEKO_FORCE_INLINE float neko_interp_acceleration(float a, float b, float t) { return neko_interp_linear(a, b, t * t); }

NEKO_FORCE_INLINE float neko_interp_deceleration(float a, float b, float t) { return neko_interp_linear(a, b, 1.0f - (1.0f - t) * (1.0f - t)); }

NEKO_FORCE_INLINE float neko_round(float val) { return (float)floor(val + 0.5f); }

NEKO_FORCE_INLINE float neko_map_range(float input_start, float input_end, float output_start, float output_end, float val) {
    float slope = (output_end - output_start) / (input_end - input_start);
    return (output_start + (slope * (val - input_start)));
}

// 缓动来自：https://github.com/raysan5/raylib/blob/ea0f6c7a26f3a61f3be542aa8f066ce033766a9f/examples/others/easings.h
NEKO_FORCE_INLINE float neko_ease_cubic_in(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t * t + b);
}

NEKO_FORCE_INLINE float neko_ease_cubic_out(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * (t * t * t + 1.0f) + b);
}

NEKO_FORCE_INLINE float neko_ease_cubic_in_out(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f) {
        return (c / 2.0f * t * t * t + b);
    }
    t -= 2.0f;
    return (c / 2.0f * (t * t * t + 2.0f) + b);
}

/*================================================================================
// Vec2
================================================================================*/

/** @brief struct neko_vec2 in neko math */
typedef struct {
    union {
        f32 xy[2];
        struct {
            f32 x, y;
        };
    };
} neko_vec2_t;

typedef neko_vec2_t neko_vec2;

NEKO_FORCE_INLINE neko_vec2 neko_vec2_ctor(f32 _x, f32 _y) {
    neko_vec2 v;
    v.x = _x;
    v.y = _y;
    return v;
}

NEKO_FORCE_INLINE bool neko_vec2_nan(neko_vec2 v) {
    if (v.x != v.x || v.y != v.y) return true;
    return false;
}

NEKO_FORCE_INLINE neko_vec2 neko_vec2_add(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x + v1.x, v0.y + v1.y); }

NEKO_FORCE_INLINE neko_vec2 neko_vec2_sub(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x - v1.x, v0.y - v1.y); }

NEKO_FORCE_INLINE neko_vec2 neko_vec2_mul(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x * v1.x, v0.y * v1.y); }

NEKO_FORCE_INLINE neko_vec2 neko_vec2_div(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x / v1.x, v0.y / v1.y); }

NEKO_FORCE_INLINE bool neko_vec2_equals(neko_vec2 v0, neko_vec2 v1) { return (v0.x == v1.x && v0.y == v1.y); }

NEKO_FORCE_INLINE neko_vec2 neko_vec2_scale(neko_vec2 v, f32 s) { return neko_vec2_ctor(v.x * s, v.y * s); }

NEKO_FORCE_INLINE f32 neko_vec2_dot(neko_vec2 v0, neko_vec2 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y); }

NEKO_FORCE_INLINE f32 neko_vec2_len(neko_vec2 v) { return (f32)sqrt(neko_vec2_dot(v, v)); }

NEKO_FORCE_INLINE neko_vec2 neko_vec2_project_onto(neko_vec2 v0, neko_vec2 v1) {
    f32 dot = neko_vec2_dot(v0, v1);
    f32 len = neko_vec2_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return neko_vec2_scale(v1, dot / len);
}

NEKO_FORCE_INLINE neko_vec2 neko_vec2_norm(neko_vec2 v) {
    f32 len = neko_vec2_len(v);
    return neko_vec2_scale(v, len != 0.f ? 1.0f / neko_vec2_len(v) : 1.f);
}

NEKO_FORCE_INLINE f32 neko_vec2_dist(neko_vec2 a, neko_vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(sqrt(dx * dx + dy * dy));
}

NEKO_FORCE_INLINE f32 neko_vec2_dist2(neko_vec2 a, neko_vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(dx * dx + dy * dy);
}

NEKO_FORCE_INLINE f32 neko_vec2_cross(neko_vec2 a, neko_vec2 b) { return a.x * b.y - a.y * b.x; }

NEKO_FORCE_INLINE f32 neko_vec2_angle(neko_vec2 a, neko_vec2 b) { return (float)acos(neko_vec2_dot(a, b) / (neko_vec2_len(a) * neko_vec2_len(b))); }

NEKO_FORCE_INLINE bool neko_vec2_equal(neko_vec2 a, neko_vec2 b) { return (a.x == b.x && a.y == b.y); }

/*================================================================================
// Vec3
================================================================================*/

typedef struct {
    union {
        f32 xyz[3];
        struct {
            f32 x, y, z;
        };
    };

} neko_vec3_t;

typedef neko_vec3_t neko_vec3;

NEKO_FORCE_INLINE neko_vec3 neko_vec3_ctor(f32 _x, f32 _y, f32 _z) {
    neko_vec3 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    return v;
}

NEKO_FORCE_INLINE bool neko_vec3_eq(neko_vec3 v0, neko_vec3 v1) { return (v0.x == v1.x && v0.y == v1.y && v0.z == v1.z); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_add(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_sub(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_mul(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_div(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_scale(neko_vec3 v, f32 s) { return neko_vec3_ctor(v.x * s, v.y * s, v.z * s); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_neg(neko_vec3 v) { return neko_vec3_scale(v, -1.f); }

NEKO_FORCE_INLINE f32 neko_vec3_dot(neko_vec3 v0, neko_vec3 v1) {
    f32 dot = (f32)((v0.x * v1.x) + (v0.y * v1.y) + v0.z * v1.z);
    return dot;
}

NEKO_FORCE_INLINE bool neko_vec3_same_dir(neko_vec3 v0, neko_vec3 v1) { return (neko_vec3_dot(v0, v1) > 0.f); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_sign(neko_vec3 v) {
    return (neko_vec3_ctor(v.x < 0.f ? -1.f : v.x > 0.f ? 1.f : 0.f, v.y < 0.f ? -1.f : v.y > 0.f ? 1.f : 0.f, v.z < 0.f ? -1.f : v.z > 0.f ? 1.f : 0.f));
}

NEKO_FORCE_INLINE float neko_vec3_signX(neko_vec3 v) { return (v.x < 0.f ? -1.f : v.x > 0.f ? 1.f : 0.f); }

NEKO_FORCE_INLINE float neko_vec3_signY(neko_vec3 v) { return (v.y < 0.f ? -1.f : v.y > 0.f ? 1.f : 0.f); }

NEKO_FORCE_INLINE float neko_vec3_signZ(neko_vec3 v) { return (v.z < 0.f ? -1.f : v.z > 0.f ? 1.f : 0.f); }

NEKO_FORCE_INLINE f32 neko_vec3_len(neko_vec3 v) { return (f32)sqrt(neko_vec3_dot(v, v)); }

NEKO_FORCE_INLINE f32 neko_vec3_len2(neko_vec3 v) { return (f32)(neko_vec3_dot(v, v)); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_project_onto(neko_vec3 v0, neko_vec3 v1) {
    f32 dot = neko_vec3_dot(v0, v1);
    f32 len = neko_vec3_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return neko_vec3_scale(v1, dot / len);
}

NEKO_FORCE_INLINE bool neko_vec3_nan(neko_vec3 v) {
    if (v.x != v.x || v.y != v.y || v.z != v.z) return true;
    return false;
}

NEKO_FORCE_INLINE f32 neko_vec3_dist2(neko_vec3 a, neko_vec3 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    f32 dz = (a.z - b.z);
    return (dx * dx + dy * dy + dz * dz);
}

NEKO_FORCE_INLINE f32 neko_vec3_dist(neko_vec3 a, neko_vec3 b) { return sqrt(neko_vec3_dist2(a, b)); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_norm(neko_vec3 v) {
    f32 len = neko_vec3_len(v);
    return len == 0.f ? v : neko_vec3_scale(v, 1.f / len);
}

NEKO_FORCE_INLINE neko_vec3 neko_vec3_cross(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x); }

NEKO_FORCE_INLINE void neko_vec3_scale_ip(neko_vec3* vp, f32 s) {
    vp->x *= s;
    vp->y *= s;
    vp->z *= s;
}

NEKO_FORCE_INLINE float neko_vec3_angle_between(neko_vec3 v0, neko_vec3 v1) { return acosf(neko_vec3_dot(v0, v1)); }

NEKO_FORCE_INLINE float neko_vec3_angle_between_signed(neko_vec3 v0, neko_vec3 v1) { return asinf(neko_vec3_len(neko_vec3_cross(v0, v1))); }

NEKO_FORCE_INLINE neko_vec3 neko_vec3_triple_cross_product(neko_vec3 a, neko_vec3 b, neko_vec3 c) {
    return neko_vec3_sub((neko_vec3_scale(b, neko_vec3_dot(c, a))), (neko_vec3_scale(a, neko_vec3_dot(c, b))));
}

/*================================================================================
// Vec4
================================================================================*/

typedef struct {
    union {
        f32 xyzw[4];
        struct {
            f32 x, y, z, w;
        };
    };
} neko_vec4_t;

typedef neko_vec4_t neko_vec4;

NEKO_FORCE_INLINE neko_vec4 neko_vec4_ctor(f32 _x, f32 _y, f32 _z, f32 _w) {
    neko_vec4 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    v.w = _w;
    return v;
}

NEKO_FORCE_INLINE neko_vec4 neko_vec4_add(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w); }

NEKO_FORCE_INLINE neko_vec4 neko_vec4_sub(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w); }

NEKO_FORCE_INLINE neko_vec4 neko_vec4_mul(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w); }

NEKO_FORCE_INLINE neko_vec4 neko_vec4_div(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w); }

NEKO_FORCE_INLINE neko_vec4 neko_vec4_scale(neko_vec4 v, f32 s) { return neko_vec4_ctor(v.x * s, v.y * s, v.z * s, v.w * s); }

NEKO_FORCE_INLINE f32 neko_vec4_dot(neko_vec4 v0, neko_vec4 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w); }

NEKO_FORCE_INLINE f32 neko_vec4_len(neko_vec4 v) { return (f32)sqrt(neko_vec4_dot(v, v)); }

NEKO_FORCE_INLINE neko_vec4 neko_vec4_project_onto(neko_vec4 v0, neko_vec4 v1) {
    f32 dot = neko_vec4_dot(v0, v1);
    f32 len = neko_vec4_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return neko_vec4_scale(v1, dot / len);
}

NEKO_FORCE_INLINE neko_vec4 neko_vec4_norm(neko_vec4 v) { return neko_vec4_scale(v, 1.0f / neko_vec4_len(v)); }

NEKO_FORCE_INLINE f32 neko_vec4_dist(neko_vec4 v0, neko_vec4 v1) {
    f32 dx = (v0.x - v1.x);
    f32 dy = (v0.y - v1.y);
    f32 dz = (v0.z - v1.z);
    f32 dw = (v0.w - v1.w);
    return (float)(sqrt(dx * dx + dy * dy + dz * dz + dw * dw));
}

/*================================================================================
// Useful Vector Functions
================================================================================*/

NEKO_FORCE_INLINE neko_vec3 neko_v4tov3(neko_vec4 v) { return neko_v3(v.x, v.y, v.z); }

NEKO_FORCE_INLINE neko_vec2 neko_v3tov2(neko_vec3 v) { return neko_v2(v.x, v.y); }

NEKO_FORCE_INLINE neko_vec3 neko_v2tov3(neko_vec2 v) { return neko_v3(v.x, v.y, 0.f); }

/*================================================================================
// Mat3x3
================================================================================*/

/*
    Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct neko_mat3 {
    f32 m[9];
} neko_mat3;

NEKO_FORCE_INLINE neko_mat3 neko_mat3_diag(float val) {
    neko_mat3 m = NEKO_DEFAULT_VAL();
    m.m[0 + 0 * 3] = val;
    m.m[1 + 1 * 3] = val;
    m.m[2 + 2 * 3] = val;
    return m;
}

#define neko_mat3_identity() neko_mat3_diag(1.f)

NEKO_FORCE_INLINE neko_mat3 neko_mat3_mul(neko_mat3 m0, neko_mat3 m1) {
    neko_mat3 m = NEKO_DEFAULT_VAL();

    for (u32 y = 0; y < 3; ++y) {
        for (u32 x = 0; x < 3; ++x) {
            f32 sum = 0.0f;
            for (u32 e = 0; e < 3; ++e) {
                sum += m0.m[x + e * 3] * m1.m[e + y * 3];
            }
            m.m[x + y * 3] = sum;
        }
    }

    return m;
}

NEKO_FORCE_INLINE neko_vec3 neko_mat3_mul_vec3(neko_mat3 m, neko_vec3 v) {
    return neko_vec3_ctor(m.m[0] * v.x + m.m[1] * v.y + m.m[2] * v.z, m.m[3] * v.x + m.m[4] * v.y + m.m[5] * v.z, m.m[6] * v.x + m.m[7] * v.y + m.m[8] * v.z);
}

NEKO_FORCE_INLINE neko_mat3 neko_mat3_scale(float x, float y, float z) {
    neko_mat3 m = NEKO_DEFAULT_VAL();
    m.m[0] = x;
    m.m[4] = y;
    m.m[8] = z;
    return m;
}

NEKO_FORCE_INLINE neko_mat3 neko_mat3_rotate(float radians, float x, float y, float z) {
    neko_mat3 m = NEKO_DEFAULT_VAL();
    float s = sinf(radians), c = cosf(radians), c1 = 1.f - c;
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;
    m.m[0] = c1 * x * x + c;
    m.m[1] = c1 * xy - zs;
    m.m[2] = c1 * zx + ys;
    m.m[3] = c1 * xy + zs;
    m.m[4] = c1 * y * y + c;
    m.m[5] = c1 * yz - xs;
    m.m[6] = c1 * zx - ys;
    m.m[7] = c1 * yz + xs;
    m.m[8] = c1 * z * z + c;
    return m;
}

NEKO_FORCE_INLINE neko_mat3 neko_mat3_rotatev(float radians, neko_vec3 axis) { return neko_mat3_rotate(radians, axis.x, axis.y, axis.z); }

// Turn quaternion into mat3
NEKO_FORCE_INLINE neko_mat3 neko_mat3_rotateq(neko_vec4 q) {
    neko_mat3 m = NEKO_DEFAULT_VAL();
    float x2 = q.x * q.x, y2 = q.y * q.y, z2 = q.z * q.z, w2 = q.w * q.w;
    float xz = q.x * q.z, xy = q.x * q.y, yz = q.y * q.z, wz = q.w * q.z, wy = q.w * q.y, wx = q.w * q.x;
    m.m[0] = 1 - 2 * (y2 + z2);
    m.m[1] = 2 * (xy + wz);
    m.m[2] = 2 * (xz - wy);
    m.m[3] = 2 * (xy - wz);
    m.m[4] = 1 - 2 * (x2 + z2);
    m.m[5] = 2 * (yz + wx);
    m.m[6] = 2 * (xz + wy);
    m.m[7] = 2 * (yz - wx);
    m.m[8] = 1 - 2 * (x2 + y2);
    return m;
}

NEKO_FORCE_INLINE neko_mat3 neko_mat3_rsq(neko_vec4 q, neko_vec3 s) {
    neko_mat3 mr = neko_mat3_rotateq(q);
    neko_mat3 ms = neko_mat3_scale(s.x, s.y, s.z);
    return neko_mat3_mul(mr, ms);
}

NEKO_FORCE_INLINE neko_mat3 neko_mat3_inverse(neko_mat3 m) {
    neko_mat3 r = NEKO_DEFAULT_VAL();

    double det = (double)(m.m[0 * 3 + 0] * (m.m[1 * 3 + 1] * m.m[2 * 3 + 2] - m.m[2 * 3 + 1] * m.m[1 * 3 + 2]) - m.m[0 * 3 + 1] * (m.m[1 * 3 + 0] * m.m[2 * 3 + 2] - m.m[1 * 3 + 2] * m.m[2 * 3 + 0]) +
                          m.m[0 * 3 + 2] * (m.m[1 * 3 + 0] * m.m[2 * 3 + 1] - m.m[1 * 3 + 1] * m.m[2 * 3 + 0]));

    double inv_det = det ? 1.0 / det : 0.0;

    r.m[0 * 3 + 0] = (m.m[1 * 3 + 1] * m.m[2 * 3 + 2] - m.m[2 * 3 + 1] * m.m[1 * 3 + 2]) * inv_det;
    r.m[0 * 3 + 1] = (m.m[0 * 3 + 2] * m.m[2 * 3 + 1] - m.m[0 * 3 + 1] * m.m[2 * 3 + 2]) * inv_det;
    r.m[0 * 3 + 2] = (m.m[0 * 3 + 1] * m.m[1 * 3 + 2] - m.m[0 * 3 + 2] * m.m[1 * 3 + 1]) * inv_det;
    r.m[1 * 3 + 0] = (m.m[1 * 3 + 2] * m.m[2 * 3 + 0] - m.m[1 * 3 + 0] * m.m[2 * 3 + 2]) * inv_det;
    r.m[1 * 3 + 1] = (m.m[0 * 3 + 0] * m.m[2 * 3 + 2] - m.m[0 * 3 + 2] * m.m[2 * 3 + 0]) * inv_det;
    r.m[1 * 3 + 2] = (m.m[1 * 3 + 0] * m.m[0 * 3 + 2] - m.m[0 * 3 + 0] * m.m[1 * 3 + 2]) * inv_det;
    r.m[2 * 3 + 0] = (m.m[1 * 3 + 0] * m.m[2 * 3 + 1] - m.m[2 * 3 + 0] * m.m[1 * 3 + 1]) * inv_det;
    r.m[2 * 3 + 1] = (m.m[2 * 3 + 0] * m.m[0 * 3 + 1] - m.m[0 * 3 + 0] * m.m[2 * 3 + 1]) * inv_det;
    r.m[2 * 3 + 2] = (m.m[0 * 3 + 0] * m.m[1 * 3 + 1] - m.m[1 * 3 + 0] * m.m[0 * 3 + 1]) * inv_det;

    return r;
}

/*================================================================================
// Mat4x4
================================================================================*/

/*
    Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct neko_mat4 {
    union {
        neko_vec4 rows[4];
        float m[4][4];
        float elements[16];
        struct {
            neko_vec4 right, up, dir, position;
        } v;
    };
} neko_mat4_t;

typedef neko_mat4_t neko_mat4;

NEKO_FORCE_INLINE neko_mat4 neko_mat4_diag(f32 val) {
    neko_mat4 m;
    memset(m.elements, 0, sizeof(m.elements));
    m.elements[0 + 0 * 4] = val;
    m.elements[1 + 1 * 4] = val;
    m.elements[2 + 2 * 4] = val;
    m.elements[3 + 3 * 4] = val;
    return m;
}

#define neko_mat4_identity() neko_mat4_diag(1.0f)

NEKO_FORCE_INLINE neko_mat4 neko_mat4_ctor() {
    neko_mat4 mat = NEKO_DEFAULT_VAL();
    return mat;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_elem(const float* elements) {
    neko_mat4 mat = neko_mat4_ctor();
    memcpy(mat.elements, elements, sizeof(f32) * 16);
    return mat;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_mul(neko_mat4 m0, neko_mat4 m1) {
    neko_mat4 m_res = neko_mat4_ctor();
    for (u32 y = 0; y < 4; ++y) {
        for (u32 x = 0; x < 4; ++x) {
            f32 sum = 0.0f;
            for (u32 e = 0; e < 4; ++e) {
                sum += m0.elements[x + e * 4] * m1.elements[e + y * 4];
            }
            m_res.elements[x + y * 4] = sum;
        }
    }

    return m_res;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_mul_list(uint32_t count, ...) {
    va_list ap;
    neko_mat4 m = neko_mat4_identity();
    va_start(ap, count);
    for (uint32_t i = 0; i < count; ++i) {
        m = neko_mat4_mul(m, va_arg(ap, neko_mat4));
    }
    va_end(ap);
    return m;
}

NEKO_FORCE_INLINE void neko_mat4_set_elements(neko_mat4* m, float* elements, uint32_t count) {
    for (u32 i = 0; i < count; ++i) {
        m->elements[i] = elements[i];
    }
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_ortho_norm(const neko_mat4* m) {
    neko_mat4 r = *m;
    r.v.right = neko_vec4_norm(r.v.right);
    r.v.up = neko_vec4_norm(r.v.up);
    r.v.dir = neko_vec4_norm(r.v.dir);
    return r;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_transpose(neko_mat4 m) {
    neko_mat4 t = neko_mat4_identity();

    // First row
    t.elements[0 * 4 + 0] = m.elements[0 * 4 + 0];
    t.elements[1 * 4 + 0] = m.elements[0 * 4 + 1];
    t.elements[2 * 4 + 0] = m.elements[0 * 4 + 2];
    t.elements[3 * 4 + 0] = m.elements[0 * 4 + 3];

    // Second row
    t.elements[0 * 4 + 1] = m.elements[1 * 4 + 0];
    t.elements[1 * 4 + 1] = m.elements[1 * 4 + 1];
    t.elements[2 * 4 + 1] = m.elements[1 * 4 + 2];
    t.elements[3 * 4 + 1] = m.elements[1 * 4 + 3];

    // Third row
    t.elements[0 * 4 + 2] = m.elements[2 * 4 + 0];
    t.elements[1 * 4 + 2] = m.elements[2 * 4 + 1];
    t.elements[2 * 4 + 2] = m.elements[2 * 4 + 2];
    t.elements[3 * 4 + 2] = m.elements[2 * 4 + 3];

    // Fourth row
    t.elements[0 * 4 + 3] = m.elements[3 * 4 + 0];
    t.elements[1 * 4 + 3] = m.elements[3 * 4 + 1];
    t.elements[2 * 4 + 3] = m.elements[3 * 4 + 2];
    t.elements[3 * 4 + 3] = m.elements[3 * 4 + 3];

    return t;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_inverse(neko_mat4 m) {
    neko_mat4 res = neko_mat4_identity();

    f32 temp[16];

    temp[0] = m.elements[5] * m.elements[10] * m.elements[15] - m.elements[5] * m.elements[11] * m.elements[14] - m.elements[9] * m.elements[6] * m.elements[15] +
              m.elements[9] * m.elements[7] * m.elements[14] + m.elements[13] * m.elements[6] * m.elements[11] - m.elements[13] * m.elements[7] * m.elements[10];

    temp[4] = -m.elements[4] * m.elements[10] * m.elements[15] + m.elements[4] * m.elements[11] * m.elements[14] + m.elements[8] * m.elements[6] * m.elements[15] -
              m.elements[8] * m.elements[7] * m.elements[14] - m.elements[12] * m.elements[6] * m.elements[11] + m.elements[12] * m.elements[7] * m.elements[10];

    temp[8] = m.elements[4] * m.elements[9] * m.elements[15] - m.elements[4] * m.elements[11] * m.elements[13] - m.elements[8] * m.elements[5] * m.elements[15] +
              m.elements[8] * m.elements[7] * m.elements[13] + m.elements[12] * m.elements[5] * m.elements[11] - m.elements[12] * m.elements[7] * m.elements[9];

    temp[12] = -m.elements[4] * m.elements[9] * m.elements[14] + m.elements[4] * m.elements[10] * m.elements[13] + m.elements[8] * m.elements[5] * m.elements[14] -
               m.elements[8] * m.elements[6] * m.elements[13] - m.elements[12] * m.elements[5] * m.elements[10] + m.elements[12] * m.elements[6] * m.elements[9];

    temp[1] = -m.elements[1] * m.elements[10] * m.elements[15] + m.elements[1] * m.elements[11] * m.elements[14] + m.elements[9] * m.elements[2] * m.elements[15] -
              m.elements[9] * m.elements[3] * m.elements[14] - m.elements[13] * m.elements[2] * m.elements[11] + m.elements[13] * m.elements[3] * m.elements[10];

    temp[5] = m.elements[0] * m.elements[10] * m.elements[15] - m.elements[0] * m.elements[11] * m.elements[14] - m.elements[8] * m.elements[2] * m.elements[15] +
              m.elements[8] * m.elements[3] * m.elements[14] + m.elements[12] * m.elements[2] * m.elements[11] - m.elements[12] * m.elements[3] * m.elements[10];

    temp[9] = -m.elements[0] * m.elements[9] * m.elements[15] + m.elements[0] * m.elements[11] * m.elements[13] + m.elements[8] * m.elements[1] * m.elements[15] -
              m.elements[8] * m.elements[3] * m.elements[13] - m.elements[12] * m.elements[1] * m.elements[11] + m.elements[12] * m.elements[3] * m.elements[9];

    temp[13] = m.elements[0] * m.elements[9] * m.elements[14] - m.elements[0] * m.elements[10] * m.elements[13] - m.elements[8] * m.elements[1] * m.elements[14] +
               m.elements[8] * m.elements[2] * m.elements[13] + m.elements[12] * m.elements[1] * m.elements[10] - m.elements[12] * m.elements[2] * m.elements[9];

    temp[2] = m.elements[1] * m.elements[6] * m.elements[15] - m.elements[1] * m.elements[7] * m.elements[14] - m.elements[5] * m.elements[2] * m.elements[15] +
              m.elements[5] * m.elements[3] * m.elements[14] + m.elements[13] * m.elements[2] * m.elements[7] - m.elements[13] * m.elements[3] * m.elements[6];

    temp[6] = -m.elements[0] * m.elements[6] * m.elements[15] + m.elements[0] * m.elements[7] * m.elements[14] + m.elements[4] * m.elements[2] * m.elements[15] -
              m.elements[4] * m.elements[3] * m.elements[14] - m.elements[12] * m.elements[2] * m.elements[7] + m.elements[12] * m.elements[3] * m.elements[6];

    temp[10] = m.elements[0] * m.elements[5] * m.elements[15] - m.elements[0] * m.elements[7] * m.elements[13] - m.elements[4] * m.elements[1] * m.elements[15] +
               m.elements[4] * m.elements[3] * m.elements[13] + m.elements[12] * m.elements[1] * m.elements[7] - m.elements[12] * m.elements[3] * m.elements[5];

    temp[14] = -m.elements[0] * m.elements[5] * m.elements[14] + m.elements[0] * m.elements[6] * m.elements[13] + m.elements[4] * m.elements[1] * m.elements[14] -
               m.elements[4] * m.elements[2] * m.elements[13] - m.elements[12] * m.elements[1] * m.elements[6] + m.elements[12] * m.elements[2] * m.elements[5];

    temp[3] = -m.elements[1] * m.elements[6] * m.elements[11] + m.elements[1] * m.elements[7] * m.elements[10] + m.elements[5] * m.elements[2] * m.elements[11] -
              m.elements[5] * m.elements[3] * m.elements[10] - m.elements[9] * m.elements[2] * m.elements[7] + m.elements[9] * m.elements[3] * m.elements[6];

    temp[7] = m.elements[0] * m.elements[6] * m.elements[11] - m.elements[0] * m.elements[7] * m.elements[10] - m.elements[4] * m.elements[2] * m.elements[11] +
              m.elements[4] * m.elements[3] * m.elements[10] + m.elements[8] * m.elements[2] * m.elements[7] - m.elements[8] * m.elements[3] * m.elements[6];

    temp[11] = -m.elements[0] * m.elements[5] * m.elements[11] + m.elements[0] * m.elements[7] * m.elements[9] + m.elements[4] * m.elements[1] * m.elements[11] -
               m.elements[4] * m.elements[3] * m.elements[9] - m.elements[8] * m.elements[1] * m.elements[7] + m.elements[8] * m.elements[3] * m.elements[5];

    temp[15] = m.elements[0] * m.elements[5] * m.elements[10] - m.elements[0] * m.elements[6] * m.elements[9] - m.elements[4] * m.elements[1] * m.elements[10] +
               m.elements[4] * m.elements[2] * m.elements[9] + m.elements[8] * m.elements[1] * m.elements[6] - m.elements[8] * m.elements[2] * m.elements[5];

    float determinant = m.elements[0] * temp[0] + m.elements[1] * temp[4] + m.elements[2] * temp[8] + m.elements[3] * temp[12];
    determinant = 1.0f / determinant;

    for (int i = 0; i < 4 * 4; i++) res.elements[i] = (float)(temp[i] * (float)determinant);

    return res;
}

// f32 l : left
// f32 r : right
// f32 b : bottom
// f32 t : top
// f32 n : near
// f32 f : far
NEKO_FORCE_INLINE neko_mat4 neko_mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    neko_mat4 m_res = neko_mat4_identity();

    // Main diagonal
    m_res.elements[0 + 0 * 4] = 2.0f / (r - l);
    m_res.elements[1 + 1 * 4] = 2.0f / (t - b);
    m_res.elements[2 + 2 * 4] = -2.0f / (f - n);

    // Last column
    m_res.elements[0 + 3 * 4] = -(r + l) / (r - l);
    m_res.elements[1 + 3 * 4] = -(t + b) / (t - b);
    m_res.elements[2 + 3 * 4] = -(f + n) / (f - n);

    return m_res;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_perspective(f32 fov, f32 asp_ratio, f32 n, f32 f) {
    // Zero matrix
    neko_mat4 m_res = neko_mat4_ctor();

    f32 q = 1.0f / (float)tan(neko_deg2rad(0.5f * fov));
    f32 a = q / asp_ratio;
    f32 b = (n + f) / (n - f);
    f32 c = (2.0f * n * f) / (n - f);

    m_res.elements[0 + 0 * 4] = a;
    m_res.elements[1 + 1 * 4] = q;
    m_res.elements[2 + 2 * 4] = b;
    m_res.elements[2 + 3 * 4] = c;
    m_res.elements[3 + 2 * 4] = -1.0f;

    return m_res;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_translatev(const neko_vec3 v) {
    neko_mat4 m_res = neko_mat4_identity();

    m_res.elements[0 + 4 * 3] = v.x;
    m_res.elements[1 + 4 * 3] = v.y;
    m_res.elements[2 + 4 * 3] = v.z;

    return m_res;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_translate(float x, float y, float z) { return neko_mat4_translatev(neko_v3(x, y, z)); }

NEKO_FORCE_INLINE neko_mat4 neko_mat4_scalev(const neko_vec3 v) {
    neko_mat4 m_res = neko_mat4_identity();
    m_res.elements[0 + 0 * 4] = v.x;
    m_res.elements[1 + 1 * 4] = v.y;
    m_res.elements[2 + 2 * 4] = v.z;
    return m_res;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_scale(float x, float y, float z) { return (neko_mat4_scalev(neko_v3(x, y, z))); }

// Assumes normalized axis
NEKO_FORCE_INLINE neko_mat4 neko_mat4_rotatev(float angle, neko_vec3 axis) {
    neko_mat4 m_res = neko_mat4_identity();

    float a = angle;
    float c = (float)cos(a);
    float s = (float)sin(a);

    neko_vec3 naxis = neko_vec3_norm(axis);
    float x = naxis.x;
    float y = naxis.y;
    float z = naxis.z;

    // First column
    m_res.elements[0 + 0 * 4] = x * x * (1 - c) + c;
    m_res.elements[1 + 0 * 4] = x * y * (1 - c) + z * s;
    m_res.elements[2 + 0 * 4] = x * z * (1 - c) - y * s;

    // Second column
    m_res.elements[0 + 1 * 4] = x * y * (1 - c) - z * s;
    m_res.elements[1 + 1 * 4] = y * y * (1 - c) + c;
    m_res.elements[2 + 1 * 4] = y * z * (1 - c) + x * s;

    // Third column
    m_res.elements[0 + 2 * 4] = x * z * (1 - c) + y * s;
    m_res.elements[1 + 2 * 4] = y * z * (1 - c) - x * s;
    m_res.elements[2 + 2 * 4] = z * z * (1 - c) + c;

    return m_res;
}

NEKO_FORCE_INLINE neko_mat4 neko_mat4_rotate(float angle, float x, float y, float z) { return neko_mat4_rotatev(angle, neko_v3(x, y, z)); }

NEKO_FORCE_INLINE neko_mat4 neko_mat4_look_at(neko_vec3 position, neko_vec3 target, neko_vec3 up) {
    neko_vec3 f = neko_vec3_norm(neko_vec3_sub(target, position));
    neko_vec3 s = neko_vec3_norm(neko_vec3_cross(f, up));
    neko_vec3 u = neko_vec3_cross(s, f);

    neko_mat4 m_res = neko_mat4_identity();
    m_res.elements[0 * 4 + 0] = s.x;
    m_res.elements[1 * 4 + 0] = s.y;
    m_res.elements[2 * 4 + 0] = s.z;

    m_res.elements[0 * 4 + 1] = u.x;
    m_res.elements[1 * 4 + 1] = u.y;
    m_res.elements[2 * 4 + 1] = u.z;

    m_res.elements[0 * 4 + 2] = -f.x;
    m_res.elements[1 * 4 + 2] = -f.y;
    m_res.elements[2 * 4 + 2] = -f.z;

    m_res.elements[3 * 4 + 0] = -neko_vec3_dot(s, position);
    ;
    m_res.elements[3 * 4 + 1] = -neko_vec3_dot(u, position);
    m_res.elements[3 * 4 + 2] = neko_vec3_dot(f, position);

    return m_res;
}

// Modified from github.com/CedricGuillemet/ImGuizmo/blob/master/ImGuizmo.cpp

NEKO_FORCE_INLINE void neko_mat4_decompose(const neko_mat4* m, float* translation, float* rotation, float* scale) {
    neko_mat4 mat = *m;

    scale[0] = neko_vec4_len(mat.v.right);
    scale[1] = neko_vec4_len(mat.v.up);
    scale[2] = neko_vec4_len(mat.v.dir);

    mat = neko_mat4_ortho_norm(&mat);

    rotation[0] = neko_rad2deg(atan2f(mat.m[1][2], mat.m[2][2]));
    rotation[1] = neko_rad2deg(atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2])));
    rotation[2] = neko_rad2deg(atan2f(mat.m[0][1], mat.m[0][0]));

    translation[0] = mat.v.position.x;
    translation[1] = mat.v.position.y;
    translation[2] = mat.v.position.z;
}

// Modified from github.com/CedricGuillemet/ImGuizmo/blob/master/ImGuizmo.cpp

NEKO_FORCE_INLINE neko_mat4 neko_mat4_recompose(const float* translation, const float* rotation, const float* scale) {
    neko_mat4 mat = neko_mat4_identity();

    neko_vec3 direction_unary[3] = {NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS};

    neko_mat4 rot[3] = {neko_mat4_identity(), neko_mat4_identity(), neko_mat4_identity()};
    for (uint32_t i = 0; i < 3; ++i) {
        rot[i] = neko_mat4_rotatev(neko_deg2rad(rotation[i]), direction_unary[i]);
    }

    mat = neko_mat4_mul_list(3, rot[2], rot[1], rot[0]);

    float valid_scale[3] = NEKO_DEFAULT_VAL();
    for (uint32_t i = 0; i < 3; ++i) {
        valid_scale[i] = fabsf(scale[i]) < neko_epsilon ? 0.001f : scale[i];
    }

    mat.v.right = neko_vec4_scale(mat.v.right, valid_scale[0]);
    mat.v.up = neko_vec4_scale(mat.v.up, valid_scale[1]);
    mat.v.dir = neko_vec4_scale(mat.v.dir, valid_scale[2]);
    mat.v.position = neko_v4(translation[0], translation[1], translation[2], 1.f);

    return mat;
}

NEKO_FORCE_INLINE neko_vec4 neko_mat4_mul_vec4(neko_mat4 m, neko_vec4 v) {
    return neko_vec4_ctor(m.elements[0 + 4 * 0] * v.x + m.elements[0 + 4 * 1] * v.y + m.elements[0 + 4 * 2] * v.z + m.elements[0 + 4 * 3] * v.w,
                          m.elements[1 + 4 * 0] * v.x + m.elements[1 + 4 * 1] * v.y + m.elements[1 + 4 * 2] * v.z + m.elements[1 + 4 * 3] * v.w,
                          m.elements[2 + 4 * 0] * v.x + m.elements[2 + 4 * 1] * v.y + m.elements[2 + 4 * 2] * v.z + m.elements[2 + 4 * 3] * v.w,
                          m.elements[3 + 4 * 0] * v.x + m.elements[3 + 4 * 1] * v.y + m.elements[3 + 4 * 2] * v.z + m.elements[3 + 4 * 3] * v.w);
}

NEKO_FORCE_INLINE neko_vec3 neko_mat4_mul_vec3(neko_mat4 m, neko_vec3 v) { return neko_v4tov3(neko_mat4_mul_vec4(m, neko_v4_xyz_s(v, 1.f))); }

/*================================================================================
// Quaternion
================================================================================*/

typedef struct {
    union {
        struct {
            union {
                neko_vec3 xyz;
                neko_vec3 axis;
            } axis;
            float a;
        } aa;
        neko_vec4 v;
        f32 xyzw[4];
        struct {
            f32 x, y, z, w;
        };
    };
} neko_quat_t;

typedef neko_quat_t neko_quat;

NEKO_FORCE_INLINE neko_quat neko_quat_default() {
    neko_quat q;
    q.x = 0.f;
    q.y = 0.f;
    q.z = 0.f;
    q.w = 1.f;
    return q;
}

NEKO_FORCE_INLINE neko_quat neko_quat_ctor(f32 _x, f32 _y, f32 _z, f32 _w) {
    neko_quat q;
    q.x = _x;
    q.y = _y;
    q.z = _z;
    q.w = _w;
    return q;
}

NEKO_FORCE_INLINE neko_quat neko_quat_add(neko_quat q0, neko_quat q1) { return neko_quat_ctor(q0.x + q1.x, q0.y + q1.y, q0.z + q1.z, q0.w + q1.w); }

NEKO_FORCE_INLINE neko_quat neko_quat_sub(neko_quat q0, neko_quat q1) { return neko_quat_ctor(q0.x - q1.x, q0.y - q1.y, q0.z - q1.z, q0.w - q1.w); }

NEKO_FORCE_INLINE neko_quat neko_quat_mul(neko_quat q0, neko_quat q1) {
    return neko_quat_ctor(q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z, q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x, q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
                          q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z);
}

NEKO_FORCE_INLINE neko_quat neko_quat_mul_list(u32 count, ...) {
    va_list ap;
    neko_quat q = neko_quat_default();
    va_start(ap, count);
    for (u32 i = 0; i < count; ++i) {
        q = neko_quat_mul(q, va_arg(ap, neko_quat));
    }
    va_end(ap);
    return q;
}

NEKO_FORCE_INLINE neko_quat neko_quat_mul_quat(neko_quat q0, neko_quat q1) {
    return neko_quat_ctor(q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z, q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x, q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
                          q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z);
}

NEKO_FORCE_INLINE neko_quat neko_quat_scale(neko_quat q, f32 s) { return neko_quat_ctor(q.x * s, q.y * s, q.z * s, q.w * s); }

NEKO_FORCE_INLINE f32 neko_quat_dot(neko_quat q0, neko_quat q1) { return (f32)(q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w); }

NEKO_FORCE_INLINE neko_quat neko_quat_conjugate(neko_quat q) { return (neko_quat_ctor(-q.x, -q.y, -q.z, q.w)); }

NEKO_FORCE_INLINE f32 neko_quat_len(neko_quat q) { return (f32)sqrt(neko_quat_dot(q, q)); }

NEKO_FORCE_INLINE neko_quat neko_quat_norm(neko_quat q) { return neko_quat_scale(q, 1.0f / neko_quat_len(q)); }

NEKO_FORCE_INLINE neko_quat neko_quat_cross(neko_quat q0, neko_quat q1) {
    return neko_quat_ctor(q0.x * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y, q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z, q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x,
                          q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z);
}

// Inverse := Conjugate / Dot;
NEKO_FORCE_INLINE neko_quat neko_quat_inverse(neko_quat q) { return (neko_quat_scale(neko_quat_conjugate(q), 1.0f / neko_quat_dot(q, q))); }

NEKO_FORCE_INLINE neko_quat neko_quat_angle_axis(f32 rad, neko_vec3 axis) {
    // Normalize axis
    neko_vec3 a = neko_vec3_norm(axis);

    // Get scalar
    f32 half_angle = 0.5f * rad;
    f32 s = (float)sin(half_angle);

    return neko_quat_ctor(a.x * s, a.y * s, a.z * s, (float)cos(half_angle));
}

NEKO_FORCE_INLINE neko_vec3 neko_quat_rotate(neko_quat q, neko_vec3 v) {
    // nVidia SDK implementation
    neko_vec3 qvec = neko_vec3_ctor(q.x, q.y, q.z);
    neko_vec3 uv = neko_vec3_cross(qvec, v);
    neko_vec3 uuv = neko_vec3_cross(qvec, uv);
    uv = neko_vec3_scale(uv, 2.f * q.w);
    uuv = neko_vec3_scale(uuv, 2.f);
    return (neko_vec3_add(v, neko_vec3_add(uv, uuv)));
}

NEKO_FORCE_INLINE neko_vec3 neko_quat_forward(neko_quat q) { return neko_quat_rotate(q, neko_v3(0.f, 0.f, -1.f)); }

NEKO_FORCE_INLINE neko_vec3 neko_quat_backward(neko_quat q) { return neko_quat_rotate(q, neko_v3(0.f, 0.f, 1.f)); }

NEKO_FORCE_INLINE neko_vec3 neko_quat_left(neko_quat q) { return neko_quat_rotate(q, neko_v3(-1.f, 0.f, 0.f)); }

NEKO_FORCE_INLINE neko_vec3 neko_quat_right(neko_quat q) { return neko_quat_rotate(q, neko_v3(1.f, 0.f, 0.f)); }

NEKO_FORCE_INLINE neko_vec3 neko_quat_up(neko_quat q) { return neko_quat_rotate(q, neko_v3(0.f, 1.f, 0.f)); }

NEKO_FORCE_INLINE neko_vec3 neko_quat_down(neko_quat q) { return neko_quat_rotate(q, neko_v3(0.f, -1.f, 0.f)); }

NEKO_FORCE_INLINE neko_quat neko_quat_from_to_rotation(neko_vec3 src, neko_vec3 dst) {
    src = neko_vec3_norm(src);
    dst = neko_vec3_norm(dst);
    const float d = neko_vec3_dot(src, dst);

    if (d >= 1.f) {
        return neko_quat_default();
    } else if (d <= -1.f) {
        // Orthonormalize, find axis of rotation
        neko_vec3 axis = neko_vec3_cross(src, NEKO_XAXIS);
        if (neko_vec3_len2(axis) < 1e-6) {
            axis = neko_vec3_cross(src, NEKO_YAXIS);
        }
        return neko_quat_angle_axis((float)neko_pi, neko_vec3_norm(axis));
    } else {
        const float s = sqrtf(neko_vec3_len2(src) * neko_vec3_len2(dst)) + neko_vec3_dot(src, dst);

        neko_vec3 axis = neko_vec3_cross(src, dst);

        return neko_quat_norm(neko_quat_ctor(axis.x, axis.y, axis.z, s));
    }
}

NEKO_FORCE_INLINE neko_quat neko_quat_look_rotation(neko_vec3 position, neko_vec3 target, neko_vec3 up) {
    const neko_vec3 forward = neko_vec3_norm(neko_vec3_sub(position, target));
    const neko_quat q0 = neko_quat_from_to_rotation(NEKO_ZAXIS, forward);
    if (neko_vec3_len2(neko_vec3_cross(forward, up)) < 1e-6) {
        return q0;
    }

    const neko_vec3 new_up = neko_quat_rotate(q0, up);
    const neko_quat q1 = neko_quat_from_to_rotation(new_up, up);

    return neko_quat_mul(q1, q0);
}

NEKO_FORCE_INLINE neko_quat neko_quat_slerp(neko_quat a, neko_quat b, f32 t) {
    f32 c = neko_quat_dot(a, b);
    neko_quat end = b;

    if (c < 0.0f) {
        // Reverse all signs
        c *= -1.0f;
        end.x *= -1.0f;
        end.y *= -1.0f;
        end.z *= -1.0f;
        end.w *= -1.0f;
    }

    // Calculate coefficients
    f32 sclp, sclq;
    if ((1.0f - c) > 0.0001f) {
        f32 omega = (float)acosf(c);
        f32 s = (float)sinf(omega);
        sclp = (float)sinf((1.0f - t) * omega) / s;
        sclq = (float)sinf(t * omega) / s;
    } else {
        sclp = 1.0f - t;
        sclq = t;
    }

    neko_quat q;
    q.x = sclp * a.x + sclq * end.x;
    q.y = sclp * a.y + sclq * end.y;
    q.z = sclp * a.z + sclq * end.z;
    q.w = sclp * a.w + sclq * end.w;

    return q;
}

#define quat_axis_angle(__AXS, __ANG) neko_quat_angle_axis(__ANG, __AXS)

/*
 * @brief Convert given quaternion param into equivalent 4x4 rotation matrix
 * @note: From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
 */
NEKO_FORCE_INLINE neko_mat4 neko_quat_to_mat4(neko_quat _q) {
    neko_mat4 mat = neko_mat4_identity();
    neko_quat q = neko_quat_norm(_q);

    f32 xx = q.x * q.x;
    f32 yy = q.y * q.y;
    f32 zz = q.z * q.z;
    f32 xy = q.x * q.y;
    f32 xz = q.x * q.z;
    f32 yz = q.y * q.z;
    f32 wx = q.w * q.x;
    f32 wy = q.w * q.y;
    f32 wz = q.w * q.z;

    mat.elements[0 * 4 + 0] = 1.0f - 2.0f * (yy + zz);
    mat.elements[1 * 4 + 0] = 2.0f * (xy - wz);
    mat.elements[2 * 4 + 0] = 2.0f * (xz + wy);

    mat.elements[0 * 4 + 1] = 2.0f * (xy + wz);
    mat.elements[1 * 4 + 1] = 1.0f - 2.0f * (xx + zz);
    mat.elements[2 * 4 + 1] = 2.0f * (yz - wx);

    mat.elements[0 * 4 + 2] = 2.0f * (xz - wy);
    mat.elements[1 * 4 + 2] = 2.0f * (yz + wx);
    mat.elements[2 * 4 + 2] = 1.0f - 2.0f * (xx + yy);

    return mat;
}

NEKO_FORCE_INLINE neko_quat neko_quat_from_euler(f32 yaw_deg, f32 pitch_deg, f32 roll_deg) {
    f32 yaw = neko_deg2rad(yaw_deg);
    f32 pitch = neko_deg2rad(pitch_deg);
    f32 roll = neko_deg2rad(roll_deg);

    neko_quat q;
    f32 cy = (float)cos(yaw * 0.5f);
    f32 sy = (float)sin(yaw * 0.5f);
    f32 cr = (float)cos(roll * 0.5f);
    f32 sr = (float)sin(roll * 0.5f);
    f32 cp = (float)cos(pitch * 0.5f);
    f32 sp = (float)sin(pitch * 0.5f);

    q.x = cy * sr * cp - sy * cr * sp;
    q.y = cy * cr * sp + sy * sr * cp;
    q.z = sy * cr * cp - cy * sr * sp;
    q.w = cy * cr * cp + sy * sr * sp;

    return q;
}

NEKO_FORCE_INLINE float neko_quat_pitch(neko_quat* q) { return atan2(2.0f * q->y * q->z + q->w * q->x, q->w * q->w - q->x * q->x - q->y * q->y + q->z * q->z); }

NEKO_FORCE_INLINE float neko_quat_yaw(neko_quat* q) { return asin(-2.0f * (q->x * q->z - q->w * q->y)); }

NEKO_FORCE_INLINE float neko_quat_roll(neko_quat* q) { return atan2(2.0f * q->x * q->y + q->z * q->w, q->x * q->x + q->w * q->w - q->y * q->y - q->z * q->z); }

NEKO_FORCE_INLINE neko_vec3 neko_quat_to_euler(neko_quat* q) { return neko_v3(neko_quat_yaw(q), neko_quat_pitch(q), neko_quat_roll(q)); }

/*================================================================================
// Transform (Non-Uniform Scalar VQS)
================================================================================*/

/*
    - This follows a traditional 'VQS' structure for complex object transformations,
        however it differs from the standard in that it allows for non-uniform
        scaling in the form of a vec3.
*/
// Source: https://www.eurosis.org/cms/files/conf/gameon-asia/gameon-asia2007/R-SESSION/G1.pdf

typedef struct {
    union {
        neko_vec3 position;
        neko_vec3 translation;
    };
    neko_quat rotation;
    neko_vec3 scale;
} neko_vqs_t;

typedef neko_vqs_t neko_vqs;

NEKO_FORCE_INLINE neko_vqs neko_vqs_ctor(neko_vec3 tns, neko_quat rot, neko_vec3 scl) {
    neko_vqs t;
    t.position = tns;
    t.rotation = rot;
    t.scale = scl;
    return t;
}

NEKO_FORCE_INLINE neko_vqs neko_vqs_default() {
    neko_vqs t = neko_vqs_ctor(neko_vec3_ctor(0.0f, 0.0f, 0.0f), neko_quat_ctor(0.0f, 0.0f, 0.0f, 1.0f), neko_vec3_ctor(1.0f, 1.0f, 1.0f));
    return t;
}

// AbsScale = ParentScale * LocalScale
// AbsRot   = LocalRot * ParentRot
// AbsTrans = ParentPos + [ParentRot * (ParentScale * LocalPos)]
NEKO_FORCE_INLINE neko_vqs neko_vqs_absolute_transform(const neko_vqs* local, const neko_vqs* parent) {
    if (!local || !parent) {
        return neko_vqs_default();
    }

    // Normalized rotations
    neko_quat p_rot_norm = neko_quat_norm(parent->rotation);
    neko_quat l_rot_norm = neko_quat_norm(local->rotation);

    // Scale
    neko_vec3 scl = neko_vec3_mul(local->scale, parent->scale);
    // Rotation
    neko_quat rot = neko_quat_norm(neko_quat_mul(p_rot_norm, l_rot_norm));
    // position
    neko_vec3 tns = neko_vec3_add(parent->position, neko_quat_rotate(p_rot_norm, neko_vec3_mul(parent->scale, local->position)));

    return neko_vqs_ctor(tns, rot, scl);
}

// RelScale = AbsScale / ParentScale
// RelRot   = Inverse(ParentRot) * AbsRot
// RelTrans = [Inverse(ParentRot) * (AbsPos - ParentPosition)] / ParentScale;
NEKO_FORCE_INLINE neko_vqs neko_vqs_relative_transform(const neko_vqs* absolute, const neko_vqs* parent) {
    if (!absolute || !parent) {
        return neko_vqs_default();
    }

    // Get inverse rotation normalized
    neko_quat p_rot_inv = neko_quat_norm(neko_quat_inverse(parent->rotation));
    // Normalized abs rotation
    neko_quat a_rot_norm = neko_quat_norm(absolute->rotation);

    // Scale
    neko_vec3 scl = neko_vec3_div(absolute->scale, parent->scale);
    // Rotation
    neko_quat rot = neko_quat_norm(neko_quat_mul(p_rot_inv, a_rot_norm));
    // position
    neko_vec3 tns = neko_vec3_div(neko_quat_rotate(p_rot_inv, neko_vec3_sub(absolute->position, parent->position)), parent->scale);

    return neko_vqs_ctor(tns, rot, scl);
}

NEKO_FORCE_INLINE neko_mat4 neko_vqs_to_mat4(const neko_vqs* transform) {
    neko_mat4 mat = neko_mat4_identity();
    neko_mat4 trans = neko_mat4_translatev(transform->position);
    neko_mat4 rot = neko_quat_to_mat4(transform->rotation);
    neko_mat4 scl = neko_mat4_scalev(transform->scale);
    mat = neko_mat4_mul(mat, trans);
    mat = neko_mat4_mul(mat, rot);
    mat = neko_mat4_mul(mat, scl);
    return mat;
}

NEKO_FORCE_INLINE neko_vqs neko_vqs_from_mat4(const neko_mat4* m) {
    neko_vec3 translation = neko_v3s(0.f), rotation = neko_v3s(0.f), scale = neko_v3s(1.f);
    neko_mat4_decompose(m, (float*)&translation, (float*)&rotation, (float*)&scale);
    return neko_vqs_ctor(translation, neko_quat_from_euler(rotation.x, rotation.y, rotation.z), scale);
}

NEKO_FORCE_INLINE neko_vec3 neko_vqs_forward(const neko_vqs* transform) { return (neko_quat_rotate(transform->rotation, neko_v3(0.0f, 0.0f, -1.0f))); }

NEKO_FORCE_INLINE neko_vec3 neko_vqs_backward(const neko_vqs* transform) { return (neko_quat_rotate(transform->rotation, neko_v3(0.0f, 0.0f, 1.0f))); }

NEKO_FORCE_INLINE neko_vec3 neko_vqs_left(const neko_vqs* transform) { return (neko_quat_rotate(transform->rotation, neko_v3(-1.0f, 0.0f, 0.0f))); }

NEKO_FORCE_INLINE neko_vec3 neko_vqs_right(const neko_vqs* transform) { return (neko_quat_rotate(transform->rotation, neko_v3(1.0f, 0.0f, 0.0f))); }

NEKO_FORCE_INLINE neko_vec3 neko_vqs_up(const neko_vqs* transform) { return (neko_quat_rotate(transform->rotation, neko_v3(0.0f, 1.0f, 0.0f))); }

NEKO_FORCE_INLINE neko_vec3 neko_vqs_down(const neko_vqs* transform) { return (neko_quat_rotate(transform->rotation, neko_v3(0.0f, -1.0f, 0.0f))); }

// AABBs
/*
    min is top left of rect,
    max is bottom right
*/

typedef struct neko_aabb_t {
    neko_vec2 min;
    neko_vec2 max;
} neko_aabb_t;

// Collision Resolution: Minimum Translation Vector
NEKO_FORCE_INLINE
neko_vec2 neko_aabb_aabb_mtv(neko_aabb_t* a0, neko_aabb_t* a1) {
    neko_vec2 diff = neko_v2(a0->min.x - a1->min.x, a0->min.y - a1->min.y);

    f32 l, r, b, t;
    neko_vec2 mtv = neko_v2(0.f, 0.f);

    l = a1->min.x - a0->max.x;
    r = a1->max.x - a0->min.x;
    b = a1->min.y - a0->max.y;
    t = a1->max.y - a0->min.y;

    mtv.x = fabsf(l) > r ? r : l;
    mtv.y = fabsf(b) > t ? t : b;

    if (fabsf(mtv.x) <= fabsf(mtv.y)) {
        mtv.y = 0.f;
    } else {
        mtv.x = 0.f;
    }

    return mtv;
}

// 2D AABB collision detection (rect. vs. rect.)
NEKO_FORCE_INLINE
bool neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b) {
    if (a->max.x > b->min.x && a->max.y > b->min.y && a->min.x < b->max.x && a->min.y < b->max.y) {
        return true;
    }

    return false;
}

/*
NEKO_FORCE_INLINE
neko_vec4 neko_aabb_window_coords(neko_aabb_t* aabb, neko_camera_t* camera, neko_vec2 window_size) {
    // AABB of the player
    neko_vec4 bounds = NEKO_DEFAULT_VAL();
    neko_vec4 tl = neko_v4(aabb->min.x, aabb->min.y, 0.f, 1.f);
    neko_vec4 br = neko_v4(aabb->max.x, aabb->max.y, 0.f, 1.f);

    neko_mat4 view_mtx = neko_camera_get_view(camera);
    neko_mat4 proj_mtx = neko_camera_get_proj(camera, (s32)window_size.x, (s32)window_size.y);
    neko_mat4 vp = neko_mat4_mul(proj_mtx, view_mtx);

    // Transform verts
    tl = neko_mat4_mul_vec4(vp, tl);
    br = neko_mat4_mul_vec4(vp, br);

    // Perspective divide
    tl = neko_vec4_scale(tl, 1.f / tl.w);
    br = neko_vec4_scale(br, 1.f / br.w);

    // NDC [0.f, 1.f] and NDC
    tl.x = (tl.x * 0.5f + 0.5f);
    tl.y = (tl.y * 0.5f + 0.5f);
    br.x = (br.x * 0.5f + 0.5f);
    br.y = (br.y * 0.5f + 0.5f);

    // Window Space
    tl.x = tl.x * window_size.x;
    tl.y = neko_map_range(1.f, 0.f, 0.f, 1.f, tl.y) * window_size.y;
    br.x = br.x * window_size.x;
    br.y = neko_map_range(1.f, 0.f, 0.f, 1.f, br.y) * window_size.y;

    bounds = neko_v4(tl.x, tl.y, br.x, br.y);

    return bounds;
}
*/

#endif