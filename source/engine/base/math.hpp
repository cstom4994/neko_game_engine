#pragma once

#include "engine/base/base.hpp"

/*========================
// NEKO_MATH
========================*/

// Defines
#define neko_pi 3.14159265358979323846264f
#define neko_tau 2.0 * neko_pi
#define neko_e 2.71828182845904523536f  // e
#define neko_epsilon (1e-6)

// 实用
#define neko_v2(...) vec2_ctor(__VA_ARGS__)
#define neko_v3(...) vec3_ctor(__VA_ARGS__)
#define neko_v4(...) vec4_ctor(__VA_ARGS__)

#define neko_v2s(__S) vec2_ctor((__S), (__S))
#define neko_v3s(__S) vec3_ctor((__S), (__S), (__S))
#define neko_v4s(__S) vec4_ctor((__S), (__S), (__S), (__S))

#define neko_v4_xy_v(__X, __Y, __V) vec4_ctor((__X), (__Y), (__V).x, (__V).y)
#define neko_v4_xyz_s(__XYZ, __S) vec4_ctor((__XYZ).x, (__XYZ).y, (__XYZ).z, (__S))

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
inline float neko_interp_linear(float a, float b, float t) { return (a + t * (b - a)); }

// Returns t based on v
inline float neko_interp_linear_inv(float a, float b, float v) { return (v - a) / (b - a); }

inline float neko_interp_smoothstep(float a, float b, float t) { return neko_interp_linear(a, b, t * t * (3.0f - 2.0f * t)); }

inline float neko_interp_cosine(float a, float b, float t) { return neko_interp_linear(a, b, (float)-cos(neko_pi * t) * 0.5f + 0.5f); }

inline float neko_interp_acceleration(float a, float b, float t) { return neko_interp_linear(a, b, t * t); }

inline float neko_interp_deceleration(float a, float b, float t) { return neko_interp_linear(a, b, 1.0f - (1.0f - t) * (1.0f - t)); }

inline float neko_round(float val) { return (float)floor(val + 0.5f); }

inline float neko_map_range(float input_start, float input_end, float output_start, float output_end, float val) {
    float slope = (output_end - output_start) / (input_end - input_start);
    return (output_start + (slope * (val - input_start)));
}

// 缓动来自：https://github.com/raysan5/raylib/blob/ea0f6c7a26f3a61f3be542aa8f066ce033766a9f/examples/others/easings.h
inline float neko_ease_cubic_in(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t * t + b);
}

inline float neko_ease_cubic_out(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * (t * t * t + 1.0f) + b);
}

inline float neko_ease_cubic_in_out(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f) {
        return (c / 2.0f * t * t * t + b);
    }
    t -= 2.0f;
    return (c / 2.0f * (t * t * t + 2.0f) + b);
}

/*================================================================================
// Vec2
================================================================================*/

struct Store;

NEKO_SCRIPT(scalar,

            typedef float Scalar;

            typedef float f32;

            typedef double f64;

            typedef uint16_t u16;

            typedef uint32_t u32;

            typedef uint64_t u64;

            typedef const char* const_str;

)

#ifdef M_PI
#define SCALAR_PI M_PI
#else
#define SCALAR_PI 3.14159265358979323846264338327950288
#endif

#define SCALAR_INFINITY INFINITY

#define SCALAR_EPSILON FLT_EPSILON

#define scalar_cos cosf
#define scalar_sin sinf
#define scalar_atan2 atan2f

#define scalar_sqrt sqrtf

#define scalar_min fminf
#define scalar_max fmaxf

#define scalar_floor floor

NEKO_SCRIPT(
        vec2,

        typedef struct {
            union {
                struct {
                    f32 x, y;
                };
                f32 xy[2];
            };
        } vec2_t;

        typedef vec2_t vec2;

        NEKO_EXPORT vec2 luavec2(Scalar x, Scalar y);

        NEKO_EXPORT vec2 vec2_zero;

        NEKO_EXPORT vec2 vec2_add(vec2 u, vec2 v);

        NEKO_EXPORT vec2 vec2_sub(vec2 u, vec2 v);

        NEKO_EXPORT vec2 vec2_mul(vec2 u, vec2 v);  // u * v componentwise

        NEKO_EXPORT vec2 vec2_div(vec2 u, vec2 v);  // u / v componentwise

        NEKO_EXPORT vec2 vec2_scalar_mul(vec2 v, Scalar f);

        NEKO_EXPORT vec2 vec2_scalar_div(vec2 v, Scalar f);  // (v.x / f, v.y / f)

        NEKO_EXPORT vec2 scalar_vec2_div(Scalar f, vec2 v);  // (f / v.x, f / v.y)

        NEKO_EXPORT vec2 vec2_neg(vec2 v);

        NEKO_EXPORT Scalar vec2_len(vec2 v);

        NEKO_EXPORT vec2 vec2_normalize(vec2 v);

        NEKO_EXPORT Scalar vec2_dot(vec2 u, vec2 v);

        NEKO_EXPORT Scalar vec2_dist(vec2 u, vec2 v);

        NEKO_EXPORT vec2 vec2_rot(vec2 v, Scalar rot);

        NEKO_EXPORT Scalar vec2_atan2(vec2 v);

        NEKO_EXPORT void vec2_save(vec2* v, const char* name, Store* s);

        NEKO_EXPORT bool vec2_load(vec2* v, const char* name, vec2 d, Store* s);

)

#define luavec2(x, y) (vec2{(float)(x), (float)(y)})

NEKO_SCRIPT(
        mat3,

        /*
         * 按列优先顺序存储
         *
         *     m = /                                 \
         *         | m.m[0][0]  m.m[1][0]  m.m[2][0] |
         *         | m.m[0][1]  m.m[1][1]  m.m[2][1] |
         *         | m.m[0][2]  m.m[1][2]  m.m[2][2] |
         *         \                                 /
         */
        typedef struct mat3 mat3;
        struct mat3 {
            union {
                Scalar v[9];
                Scalar m[3][3];
            };
        };

        NEKO_EXPORT mat3 luamat3(Scalar m00, Scalar m01, Scalar m02, Scalar m10, Scalar m11, Scalar m12, Scalar m20, Scalar m21, Scalar m22);

        NEKO_EXPORT mat3 mat3_identity();  // 返回单位矩阵

        NEKO_EXPORT mat3 mat3_mul(mat3 m, mat3 n);

        // 按顺序应用 scale rot 和 trans 的矩阵
        NEKO_EXPORT mat3 mat3_scaling_rotation_translation(vec2 scale, Scalar rot, vec2 trans);

        NEKO_EXPORT vec2 mat3_get_translation(mat3 m);

        NEKO_EXPORT Scalar mat3_get_rotation(mat3 m);

        NEKO_EXPORT vec2 mat3_get_scale(mat3 m);

        NEKO_EXPORT mat3 mat3_inverse(mat3 m);

        NEKO_EXPORT vec2 mat3_transform(mat3 m, vec2 v);

        NEKO_EXPORT void mat3_save(mat3* m, const char* name, Store* s);

        NEKO_EXPORT bool mat3_load(mat3* m, const char* name, mat3 d, Store* s);

)

#define luamat3(m00, m01, m02, m10, m11, m12, m20, m21, m22) (mat3{.m = {{(m00), (m01), (m02)}, {(m10), (m11), (m12)}, {(m20), (m21), (m22)}}})

inline vec2 vec2_ctor(f32 _x, f32 _y) {
    vec2 v;
    v.x = _x;
    v.y = _y;
    return v;
}

inline bool vec2_nan(vec2 v) {
    if (v.x != v.x || v.y != v.y) return true;
    return false;
}

inline bool vec2_equals(vec2 v0, vec2 v1) { return (v0.x == v1.x && v0.y == v1.y); }

inline vec2 vec2_scale(vec2 v, f32 s) { return vec2_ctor(v.x * s, v.y * s); }

inline f32 vec2_dot(vec2 v0, vec2 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y); }

inline f32 vec2_len(vec2 v) { return (f32)sqrt(vec2_dot(v, v)); }

inline vec2 vec2_project_onto(vec2 v0, vec2 v1) {
    f32 dot = vec2_dot(v0, v1);
    f32 len = vec2_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return vec2_scale(v1, dot / len);
}

inline vec2 vec2_norm(vec2 v) {
    f32 len = vec2_len(v);
    return vec2_scale(v, len != 0.f ? 1.0f / vec2_len(v) : 1.f);
}

inline f32 vec2_dist(vec2 a, vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(sqrt(dx * dx + dy * dy));
}

inline f32 vec2_dist2(vec2 a, vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(dx * dx + dy * dy);
}

inline f32 vec2_cross(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

inline f32 vec2_angle(vec2 a, vec2 b) { return (float)acos(vec2_dot(a, b) / (vec2_len(a) * vec2_len(b))); }

inline bool vec2_equal(vec2 a, vec2 b) { return (a.x == b.x && a.y == b.y); }

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

} vec3_t;

typedef vec3_t vec3;

inline vec3 vec3_ctor(f32 _x, f32 _y, f32 _z) {
    vec3 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    return v;
}

inline bool vec3_eq(vec3 v0, vec3 v1) { return (v0.x == v1.x && v0.y == v1.y && v0.z == v1.z); }

inline vec3 vec3_add(vec3 v0, vec3 v1) { return vec3_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z); }

inline vec3 vec3_sub(vec3 v0, vec3 v1) { return vec3_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z); }

inline vec3 vec3_mul(vec3 v0, vec3 v1) { return vec3_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z); }

inline vec3 vec3_div(vec3 v0, vec3 v1) { return vec3_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z); }

inline vec3 vec3_scale(vec3 v, f32 s) { return vec3_ctor(v.x * s, v.y * s, v.z * s); }

inline vec3 vec3_neg(vec3 v) { return vec3_scale(v, -1.f); }

inline f32 vec3_dot(vec3 v0, vec3 v1) {
    f32 dot = (f32)((v0.x * v1.x) + (v0.y * v1.y) + v0.z * v1.z);
    return dot;
}

inline bool vec3_same_dir(vec3 v0, vec3 v1) { return (vec3_dot(v0, v1) > 0.f); }

inline vec3 vec3_sign(vec3 v) { return (vec3_ctor(v.x < 0.f ? -1.f : v.x > 0.f ? 1.f : 0.f, v.y < 0.f ? -1.f : v.y > 0.f ? 1.f : 0.f, v.z < 0.f ? -1.f : v.z > 0.f ? 1.f : 0.f)); }

inline float vec3_signX(vec3 v) { return (v.x < 0.f ? -1.f : v.x > 0.f ? 1.f : 0.f); }

inline float vec3_signY(vec3 v) { return (v.y < 0.f ? -1.f : v.y > 0.f ? 1.f : 0.f); }

inline float vec3_signZ(vec3 v) { return (v.z < 0.f ? -1.f : v.z > 0.f ? 1.f : 0.f); }

inline f32 vec3_len(vec3 v) { return (f32)sqrt(vec3_dot(v, v)); }

inline f32 vec3_len2(vec3 v) { return (f32)(vec3_dot(v, v)); }

inline vec3 vec3_project_onto(vec3 v0, vec3 v1) {
    f32 dot = vec3_dot(v0, v1);
    f32 len = vec3_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return vec3_scale(v1, dot / len);
}

inline bool vec3_nan(vec3 v) {
    if (v.x != v.x || v.y != v.y || v.z != v.z) return true;
    return false;
}

inline f32 vec3_dist2(vec3 a, vec3 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    f32 dz = (a.z - b.z);
    return (dx * dx + dy * dy + dz * dz);
}

inline f32 vec3_dist(vec3 a, vec3 b) { return sqrt(vec3_dist2(a, b)); }

inline vec3 vec3_norm(vec3 v) {
    f32 len = vec3_len(v);
    return len == 0.f ? v : vec3_scale(v, 1.f / len);
}

inline vec3 vec3_cross(vec3 v0, vec3 v1) { return vec3_ctor(v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x); }

inline void vec3_scale_ip(vec3* vp, f32 s) {
    vp->x *= s;
    vp->y *= s;
    vp->z *= s;
}

inline float vec3_angle_between(vec3 v0, vec3 v1) { return acosf(vec3_dot(v0, v1)); }

inline float vec3_angle_between_signed(vec3 v0, vec3 v1) { return asinf(vec3_len(vec3_cross(v0, v1))); }

inline vec3 vec3_triple_cross_product(vec3 a, vec3 b, vec3 c) { return vec3_sub((vec3_scale(b, vec3_dot(c, a))), (vec3_scale(a, vec3_dot(c, b)))); }

/*================================================================================
// Vec4
================================================================================*/

typedef struct {
    f32 x, y, z, w;
} vec4_t;

typedef vec4_t vec4;

inline vec4 vec4_ctor(f32 _x, f32 _y, f32 _z, f32 _w) {
    vec4 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    v.w = _w;
    return v;
}

inline vec4 vec4_add(vec4 v0, vec4 v1) { return vec4_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w); }

inline vec4 vec4_sub(vec4 v0, vec4 v1) { return vec4_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w); }

inline vec4 vec4_mul(vec4 v0, vec4 v1) { return vec4_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w); }

inline vec4 vec4_div(vec4 v0, vec4 v1) { return vec4_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w); }

inline vec4 vec4_scale(vec4 v, f32 s) { return vec4_ctor(v.x * s, v.y * s, v.z * s, v.w * s); }

inline f32 vec4_dot(vec4 v0, vec4 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w); }

inline f32 vec4_len(vec4 v) { return (f32)sqrt(vec4_dot(v, v)); }

inline vec4 vec4_project_onto(vec4 v0, vec4 v1) {
    f32 dot = vec4_dot(v0, v1);
    f32 len = vec4_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return vec4_scale(v1, dot / len);
}

inline vec4 vec4_norm(vec4 v) { return vec4_scale(v, 1.0f / vec4_len(v)); }

inline f32 vec4_dist(vec4 v0, vec4 v1) {
    f32 dx = (v0.x - v1.x);
    f32 dy = (v0.y - v1.y);
    f32 dz = (v0.z - v1.z);
    f32 dw = (v0.w - v1.w);
    return (float)(sqrt(dx * dx + dy * dy + dz * dz + dw * dw));
}

/*================================================================================
// Useful Vector Functions
================================================================================*/

inline vec3 neko_v4tov3(vec4 v) { return neko_v3(v.x, v.y, v.z); }

inline vec2 neko_v3tov2(vec3 v) { return neko_v2(v.x, v.y); }

inline vec3 neko_v2tov3(vec2 v) { return neko_v3(v.x, v.y, 0.f); }

/*================================================================================
// Mat3x3
================================================================================*/

inline mat3 mat3_diag(float val) {
    mat3 m = NEKO_DEFAULT_VAL();
    m.v[0 + 0 * 3] = val;
    m.v[1 + 1 * 3] = val;
    m.v[2 + 2 * 3] = val;
    return m;
}

inline mat3 mat3_mul(mat3 m0, mat3 m1) {
    mat3 m = NEKO_DEFAULT_VAL();

    for (u32 y = 0; y < 3; ++y) {
        for (u32 x = 0; x < 3; ++x) {
            f32 sum = 0.0f;
            for (u32 e = 0; e < 3; ++e) {
                sum += m0.v[x + e * 3] * m1.v[e + y * 3];
            }
            m.v[x + y * 3] = sum;
        }
    }

    return m;
}

inline vec3 mat3_mul_vec3(mat3 m, vec3 v) { return vec3_ctor(m.v[0] * v.x + m.v[1] * v.y + m.v[2] * v.z, m.v[3] * v.x + m.v[4] * v.y + m.v[5] * v.z, m.v[6] * v.x + m.v[7] * v.y + m.v[8] * v.z); }

inline mat3 mat3_scale(float x, float y, float z) {
    mat3 m = NEKO_DEFAULT_VAL();
    m.v[0] = x;
    m.v[4] = y;
    m.v[8] = z;
    return m;
}

inline mat3 mat3_rotate(float radians, float x, float y, float z) {
    mat3 m = NEKO_DEFAULT_VAL();
    float s = sinf(radians), c = cosf(radians), c1 = 1.f - c;
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;
    m.v[0] = c1 * x * x + c;
    m.v[1] = c1 * xy - zs;
    m.v[2] = c1 * zx + ys;
    m.v[3] = c1 * xy + zs;
    m.v[4] = c1 * y * y + c;
    m.v[5] = c1 * yz - xs;
    m.v[6] = c1 * zx - ys;
    m.v[7] = c1 * yz + xs;
    m.v[8] = c1 * z * z + c;
    return m;
}

inline mat3 mat3_rotatev(float radians, vec3 axis) { return mat3_rotate(radians, axis.x, axis.y, axis.z); }

// Turn quaternion into mat3
inline mat3 mat3_rotateq(vec4 q) {
    mat3 m = NEKO_DEFAULT_VAL();
    float x2 = q.x * q.x, y2 = q.y * q.y, z2 = q.z * q.z, w2 = q.w * q.w;
    float xz = q.x * q.z, xy = q.x * q.y, yz = q.y * q.z, wz = q.w * q.z, wy = q.w * q.y, wx = q.w * q.x;
    m.v[0] = 1 - 2 * (y2 + z2);
    m.v[1] = 2 * (xy + wz);
    m.v[2] = 2 * (xz - wy);
    m.v[3] = 2 * (xy - wz);
    m.v[4] = 1 - 2 * (x2 + z2);
    m.v[5] = 2 * (yz + wx);
    m.v[6] = 2 * (xz + wy);
    m.v[7] = 2 * (yz - wx);
    m.v[8] = 1 - 2 * (x2 + y2);
    return m;
}

inline mat3 mat3_rsq(vec4 q, vec3 s) {
    mat3 mr = mat3_rotateq(q);
    mat3 ms = mat3_scale(s.x, s.y, s.z);
    return mat3_mul(mr, ms);
}

inline mat3 mat3_inverse(mat3 m) {
    mat3 r = NEKO_DEFAULT_VAL();

    double det = (double)(m.v[0 * 3 + 0] * (m.v[1 * 3 + 1] * m.v[2 * 3 + 2] - m.v[2 * 3 + 1] * m.v[1 * 3 + 2]) - m.v[0 * 3 + 1] * (m.v[1 * 3 + 0] * m.v[2 * 3 + 2] - m.v[1 * 3 + 2] * m.v[2 * 3 + 0]) +
                          m.v[0 * 3 + 2] * (m.v[1 * 3 + 0] * m.v[2 * 3 + 1] - m.v[1 * 3 + 1] * m.v[2 * 3 + 0]));

    double inv_det = det ? 1.0 / det : 0.0;

    r.v[0 * 3 + 0] = (m.v[1 * 3 + 1] * m.v[2 * 3 + 2] - m.v[2 * 3 + 1] * m.v[1 * 3 + 2]) * inv_det;
    r.v[0 * 3 + 1] = (m.v[0 * 3 + 2] * m.v[2 * 3 + 1] - m.v[0 * 3 + 1] * m.v[2 * 3 + 2]) * inv_det;
    r.v[0 * 3 + 2] = (m.v[0 * 3 + 1] * m.v[1 * 3 + 2] - m.v[0 * 3 + 2] * m.v[1 * 3 + 1]) * inv_det;
    r.v[1 * 3 + 0] = (m.v[1 * 3 + 2] * m.v[2 * 3 + 0] - m.v[1 * 3 + 0] * m.v[2 * 3 + 2]) * inv_det;
    r.v[1 * 3 + 1] = (m.v[0 * 3 + 0] * m.v[2 * 3 + 2] - m.v[0 * 3 + 2] * m.v[2 * 3 + 0]) * inv_det;
    r.v[1 * 3 + 2] = (m.v[1 * 3 + 0] * m.v[0 * 3 + 2] - m.v[0 * 3 + 0] * m.v[1 * 3 + 2]) * inv_det;
    r.v[2 * 3 + 0] = (m.v[1 * 3 + 0] * m.v[2 * 3 + 1] - m.v[2 * 3 + 0] * m.v[1 * 3 + 1]) * inv_det;
    r.v[2 * 3 + 1] = (m.v[2 * 3 + 0] * m.v[0 * 3 + 1] - m.v[0 * 3 + 0] * m.v[2 * 3 + 1]) * inv_det;
    r.v[2 * 3 + 2] = (m.v[0 * 3 + 0] * m.v[1 * 3 + 1] - m.v[1 * 3 + 0] * m.v[0 * 3 + 1]) * inv_det;

    return r;
}

/*================================================================================
// Mat4x4
================================================================================*/

/*
    Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct mat4 {
    union {
        vec4 rows[4];
        float m[4][4];
        float elements[16];
        struct {
            vec4 right, up, dir, position;
        } v;
    };
} mat4_t;

typedef mat4_t mat4;

inline mat4 mat4_diag(f32 val) {
    mat4 m;
    memset(m.elements, 0, sizeof(m.elements));
    m.elements[0 + 0 * 4] = val;
    m.elements[1 + 1 * 4] = val;
    m.elements[2 + 2 * 4] = val;
    m.elements[3 + 3 * 4] = val;
    return m;
}

#define mat4_identity() mat4_diag(1.0f)

inline mat4 mat4_ctor() {
    mat4 mat = NEKO_DEFAULT_VAL();
    return mat;
}

inline mat4 mat4_elem(const float* elements) {
    mat4 mat = mat4_ctor();
    memcpy(mat.elements, elements, sizeof(f32) * 16);
    return mat;
}

inline mat4 mat4_mul(mat4 m0, mat4 m1) {
    mat4 m_res = mat4_ctor();
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

inline vec4 mat4_mulv(mat4 m, vec4 v) {
    f32 xyzw[4];
    for (int i = 0; i < 4; i++) {
        xyzw[i] = m.m[i][0] * v.x + m.m[i][1] * v.y + m.m[i][2] * v.z + m.m[i][3] * v.w;
    }
    return vec4{xyzw[0], xyzw[1], xyzw[2], xyzw[3]};
}

inline mat4 mat4_mul_list(uint32_t count, ...) {
    va_list ap;
    mat4 m = mat4_identity();
    va_start(ap, count);
    for (uint32_t i = 0; i < count; ++i) {
        m = mat4_mul(m, va_arg(ap, mat4));
    }
    va_end(ap);
    return m;
}

inline void mat4_set_elements(mat4* m, float* elements, uint32_t count) {
    for (u32 i = 0; i < count; ++i) {
        m->elements[i] = elements[i];
    }
}

inline mat4 mat4_ortho_norm(const mat4* m) {
    mat4 r = *m;
    r.v.right = vec4_norm(r.v.right);
    r.v.up = vec4_norm(r.v.up);
    r.v.dir = vec4_norm(r.v.dir);
    return r;
}

inline mat4 mat4_transpose(mat4 m) {
    mat4 t = mat4_identity();

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

inline mat4 mat4_inverse(mat4 m) {
    mat4 res = mat4_identity();

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
inline mat4 mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    mat4 m_res = mat4_identity();

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

inline mat4 mat4_perspective(f32 fov, f32 asp_ratio, f32 n, f32 f) {
    // Zero matrix
    mat4 m_res = mat4_ctor();

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

inline mat4 mat4_translatev(const vec3 v) {
    mat4 m_res = mat4_identity();

    m_res.elements[0 + 4 * 3] = v.x;
    m_res.elements[1 + 4 * 3] = v.y;
    m_res.elements[2 + 4 * 3] = v.z;

    return m_res;
}

inline mat4 mat4_translate(float x, float y, float z) { return mat4_translatev(neko_v3(x, y, z)); }

inline mat4 mat4_scalev(const vec3 v) {
    mat4 m_res = mat4_identity();
    m_res.elements[0 + 0 * 4] = v.x;
    m_res.elements[1 + 1 * 4] = v.y;
    m_res.elements[2 + 2 * 4] = v.z;
    return m_res;
}

inline mat4 mat4_scale(float x, float y, float z) { return (mat4_scalev(neko_v3(x, y, z))); }

// Assumes normalized axis
inline mat4 mat4_rotatev(float angle, vec3 axis) {
    mat4 m_res = mat4_identity();

    float a = angle;
    float c = (float)cos(a);
    float s = (float)sin(a);

    vec3 naxis = vec3_norm(axis);
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

inline mat4 mat4_rotate(float angle, float x, float y, float z) { return mat4_rotatev(angle, neko_v3(x, y, z)); }

inline mat4 mat4_look_at(vec3 position, vec3 target, vec3 up) {
    vec3 f = vec3_norm(vec3_sub(target, position));
    vec3 s = vec3_norm(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);

    mat4 m_res = mat4_identity();
    m_res.elements[0 * 4 + 0] = s.x;
    m_res.elements[1 * 4 + 0] = s.y;
    m_res.elements[2 * 4 + 0] = s.z;

    m_res.elements[0 * 4 + 1] = u.x;
    m_res.elements[1 * 4 + 1] = u.y;
    m_res.elements[2 * 4 + 1] = u.z;

    m_res.elements[0 * 4 + 2] = -f.x;
    m_res.elements[1 * 4 + 2] = -f.y;
    m_res.elements[2 * 4 + 2] = -f.z;

    m_res.elements[3 * 4 + 0] = -vec3_dot(s, position);
    ;
    m_res.elements[3 * 4 + 1] = -vec3_dot(u, position);
    m_res.elements[3 * 4 + 2] = vec3_dot(f, position);

    return m_res;
}

// Modified from github.com/CedricGuillemet/ImGuizmo/blob/master/ImGuizmo.cpp

inline void mat4_decompose(const mat4* m, float* translation, float* rotation, float* scale) {
    mat4 mat = *m;

    scale[0] = vec4_len(mat.v.right);
    scale[1] = vec4_len(mat.v.up);
    scale[2] = vec4_len(mat.v.dir);

    mat = mat4_ortho_norm(&mat);

    rotation[0] = neko_rad2deg(atan2f(mat.m[1][2], mat.m[2][2]));
    rotation[1] = neko_rad2deg(atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2])));
    rotation[2] = neko_rad2deg(atan2f(mat.m[0][1], mat.m[0][0]));

    translation[0] = mat.v.position.x;
    translation[1] = mat.v.position.y;
    translation[2] = mat.v.position.z;
}

// Modified from github.com/CedricGuillemet/ImGuizmo/blob/master/ImGuizmo.cpp

inline mat4 mat4_recompose(const float* translation, const float* rotation, const float* scale) {
    mat4 mat = mat4_identity();

    vec3 direction_unary[3] = {NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS};

    mat4 rot[3] = {mat4_identity(), mat4_identity(), mat4_identity()};
    for (uint32_t i = 0; i < 3; ++i) {
        rot[i] = mat4_rotatev(neko_deg2rad(rotation[i]), direction_unary[i]);
    }

    mat = mat4_mul_list(3, rot[2], rot[1], rot[0]);

    float valid_scale[3] = NEKO_DEFAULT_VAL();
    for (uint32_t i = 0; i < 3; ++i) {
        valid_scale[i] = fabsf(scale[i]) < neko_epsilon ? 0.001f : scale[i];
    }

    mat.v.right = vec4_scale(mat.v.right, valid_scale[0]);
    mat.v.up = vec4_scale(mat.v.up, valid_scale[1]);
    mat.v.dir = vec4_scale(mat.v.dir, valid_scale[2]);
    mat.v.position = neko_v4(translation[0], translation[1], translation[2], 1.f);

    return mat;
}

inline vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    return vec4_ctor(m.elements[0 + 4 * 0] * v.x + m.elements[0 + 4 * 1] * v.y + m.elements[0 + 4 * 2] * v.z + m.elements[0 + 4 * 3] * v.w,
                     m.elements[1 + 4 * 0] * v.x + m.elements[1 + 4 * 1] * v.y + m.elements[1 + 4 * 2] * v.z + m.elements[1 + 4 * 3] * v.w,
                     m.elements[2 + 4 * 0] * v.x + m.elements[2 + 4 * 1] * v.y + m.elements[2 + 4 * 2] * v.z + m.elements[2 + 4 * 3] * v.w,
                     m.elements[3 + 4 * 0] * v.x + m.elements[3 + 4 * 1] * v.y + m.elements[3 + 4 * 2] * v.z + m.elements[3 + 4 * 3] * v.w);
}

inline vec3 mat4_mul_vec3(mat4 m, vec3 v) { return neko_v4tov3(mat4_mul_vec4(m, neko_v4_xyz_s(v, 1.f))); }

// AABBs
/*
    min is top left of rect,
    max is bottom right
*/

typedef struct neko_aabb_t {
    vec2 min;
    vec2 max;
} neko_aabb_t;

// Collision Resolution: Minimum Translation Vector
inline vec2 neko_aabb_aabb_mtv(neko_aabb_t* a0, neko_aabb_t* a1) {
    vec2 diff = neko_v2(a0->min.x - a1->min.x, a0->min.y - a1->min.y);

    f32 l, r, b, t;
    vec2 mtv = neko_v2(0.f, 0.f);

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
inline bool neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b) {
    if (a->max.x > b->min.x && a->max.y > b->min.y && a->min.x < b->max.x && a->min.y < b->max.y) {
        return true;
    }

    return false;
}

typedef struct rect_t {
    float x, y, w, h;
} rect_t;

#define neko_rect(...) rect_ctor(__VA_ARGS__)

inline rect_t rect_ctor(f32 _x, f32 _y, f32 _w, f32 _h) {
    rect_t v;
    v.x = _x;
    v.y = _y;
    v.w = _w;
    v.h = _h;
    return v;
}