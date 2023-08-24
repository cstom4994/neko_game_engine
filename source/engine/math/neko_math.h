#ifndef NEKO_MATH_H
#define NEKO_MATH_H

#include <math.h>

#include "engine/common/neko_util.h"

// Defines
#define neko_pi 3.14159265358979323846264f
#define neko_tau 2.0 * neko_pi
#define neko_e 2.71828182845904523536f  // e

// Useful Utility
#define neko_v2(...) neko_vec2_ctor(__VA_ARGS__)
#define neko_v3(...) neko_vec3_ctor(__VA_ARGS__)
#define neko_v4(...) neko_vec4_ctor(__VA_ARGS__)

#define neko_v2_s(s) neko_vec2_ctor((s), (s))
#define neko_v3_s(s) neko_vec3_ctor((s), (s), (s))
#define neko_v4_s(s) neko_vec4_ctor((s), (s), (s), (s))

#define neko_v4_xy_v(x, y, v) neko_vec4_ctor((x), (y), (v).x, (v).y)

#define neko_x_axis neko_v3(1.f, 0.f, 0.f)
#define neko_y_axis neko_v3(0.f, 1.f, 0.f)
#define neko_z_axis neko_v3(0.f, 0.f, 1.f)

/*================================================================================
// Useful Common Functions
================================================================================*/

#define neko_rad_to_deg(rad) (f32)((rad * 180.0) / neko_pi)

#define neko_deg_to_rad(deg) (f32)((deg * neko_pi) / 180.0)

// Interpolation
// Source: https://codeplea.com/simple-interpolation

neko_static_inline f32 neko_interp_linear(f32 a, f32 b, f32 t) { return (a + t * (b - a)); }

neko_static_inline f32 neko_interp_smooth_step(f32 a, f32 b, f32 t) { return neko_interp_linear(a, b, t * t * (3.0 - 2.0 * t)); }

neko_static_inline f32 neko_interp_cosine(f32 a, f32 b, f32 t) { return neko_interp_linear(a, b, -cos(neko_pi * t) * 0.5 + 0.5); }

neko_static_inline f32 neko_interp_acceleration(f32 a, f32 b, f32 t) { return neko_interp_linear(a, b, t * t); }

neko_static_inline f32 neko_interp_deceleration(f32 a, f32 b, f32 t) { return neko_interp_linear(a, b, 1.0 - (1.0 - t) * (1.0 - t)); }

neko_static_inline f32 neko_interp_smoothstep(f32 a, f32 b, f32 t) { return neko_interp_linear(a, b, t * t * (3.0f - 2.0f * t)); }

neko_static_inline f32 neko_round(f32 val) { return floor(val + 0.5f); }

neko_static_inline f32 neko_map_range(f32 input_start, f32 input_end, f32 output_start, f32 output_end, f32 val) {
    f32 slope = (output_end - output_start) / (input_end - input_start);
    return (output_start + (slope * (val - input_start)));
}

/*================================================================================
// neko_vec2
================================================================================*/

typedef struct neko_vec2 {
    union {
        f32 xy[2];
        struct {
            f32 x, y;
        };
    };

    void set(float x_, float y_) {
        x = x_;
        y = y_;
    }

    neko_vec2 operator-() { return neko_vec2{-x, -y}; }

    void operator+=(const neko_vec2 &v) {
        x += v.x;
        y += v.y;
    }

    void operator-=(const neko_vec2 &v) {
        x -= v.x;
        y -= v.y;
    }

    void operator*=(float a) {
        x *= a;
        y *= a;
    }

    float Length() const { return sqrtf(x * x + y * y); }

} neko_vec2;

neko_static_inline neko_vec2 neko_vec2_ctor(f32 _x, f32 _y) {
    neko_vec2 v;
    v.x = _x;
    v.y = _y;
    return v;
}

neko_static_inline neko_vec2 neko_vec2_add(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x + v1.x, v0.y + v1.y); }

neko_static_inline neko_vec2 neko_vec2_sub(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x - v1.x, v0.y - v1.y); }

neko_static_inline neko_vec2 neko_vec2_mul(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x * v1.x, v0.y * v1.y); }

neko_static_inline neko_vec2 neko_vec2_div(neko_vec2 v0, neko_vec2 v1) { return neko_vec2_ctor(v0.x / v1.x, v0.y / v1.y); }

neko_static_inline neko_vec2 neko_vec2_scale(neko_vec2 v, f32 s) { return neko_vec2_ctor(v.x * s, v.y * s); }

neko_static_inline f32 neko_vec2_dot(neko_vec2 v0, neko_vec2 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y); }

neko_static_inline f32 neko_vec2_len(neko_vec2 v) { return (f32)sqrt(neko_vec2_dot(v, v)); }

neko_static_inline neko_vec2 neko_vec2_project_onto(neko_vec2 v0, neko_vec2 v1) {
    f32 dot = neko_vec2_dot(v0, v1);
    f32 len = neko_vec2_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return neko_vec2_scale(v1, dot / len);
}

neko_static_inline neko_vec2 neko_vec2_norm(neko_vec2 v) {
    f32 len = neko_vec2_len(v);
    return neko_vec2_scale(v, len != 0.f ? 1.0f / neko_vec2_len(v) : 1.f);
}

neko_static_inline f32 neko_vec2_dist(neko_vec2 a, neko_vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (sqrt(dx * dx + dy * dy));
}

neko_static_inline f32 neko_vec2_cross(neko_vec2 a, neko_vec2 b) { return a.x * b.y - a.y * b.x; }

neko_static_inline f32 neko_vec2_angle(neko_vec2 a, neko_vec2 b) { return acos(neko_vec2_dot(a, b) / (neko_vec2_len(a) * neko_vec2_len(b))); }

neko_static_inline bool neko_vec2_equal(neko_vec2 a, neko_vec2 b) { return (a.x == b.x && a.y == b.y); }

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
} neko_vec3;

neko_static_inline neko_vec3 neko_vec3_ctor(f32 _x, f32 _y, f32 _z) {
    neko_vec3 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    return v;
}

neko_static_inline neko_vec3 neko_vec3_add(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z); }

neko_static_inline neko_vec3 neko_vec3_sub(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z); }

neko_static_inline neko_vec3 neko_vec3_mul(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z); }

neko_static_inline neko_vec3 neko_vec3_div(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z); }

neko_static_inline neko_vec3 neko_vec3_scale(neko_vec3 v, f32 s) { return neko_vec3_ctor(v.x * s, v.y * s, v.z * s); }

neko_static_inline f32 neko_vec3_dot(neko_vec3 v0, neko_vec3 v1) {
    f32 dot = (f32)((v0.x * v1.x) + (v0.y * v1.y) + v0.z * v1.z);
    return dot;
}

neko_static_inline f32 neko_vec3_len(neko_vec3 v) { return (f32)sqrt(neko_vec3_dot(v, v)); }

neko_static_inline neko_vec3 neko_vec3_project_onto(neko_vec3 v0, neko_vec3 v1) {
    f32 dot = neko_vec3_dot(v0, v1);
    f32 len = neko_vec3_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return neko_vec3_scale(v1, dot / len);
}

neko_static_inline f32 neko_vec3_dist(neko_vec3 a, neko_vec3 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    f32 dz = (a.z - b.z);
    return (sqrt(dx * dx + dy * dy + dz * dz));
}

neko_static_inline neko_vec3 neko_vec3_norm(neko_vec3 v) {
    f32 len = neko_vec3_len(v);
    return len == 0.f ? v : neko_vec3_scale(v, 1.f / len);
}

neko_static_inline neko_vec3 neko_vec3_cross(neko_vec3 v0, neko_vec3 v1) { return neko_vec3_ctor(v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x); }

neko_static_inline void neko_vec3_scale_ip(neko_vec3 *vp, f32 s) {
    vp->x *= s;
    vp->y *= s;
    vp->z *= s;
}

neko_static_inline bool neko_vec3_equal(neko_vec3 a, neko_vec3 b) { return (a.x == b.x && a.y == b.y && a.z == b.z); }

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
} neko_vec4;

neko_static_inline neko_vec4 neko_vec4_ctor(f32 _x, f32 _y, f32 _z, f32 _w) {
    neko_vec4 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    v.w = _w;
    return v;
}

neko_static_inline neko_vec4 neko_vec4_add(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x + v1.y, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w); }

neko_static_inline neko_vec4 neko_vec4_sub(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x - v1.y, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w); }

neko_static_inline neko_vec4 neko_vec4_div(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w); }

neko_static_inline neko_vec4 neko_vec4_mul(neko_vec4 v0, neko_vec4 v1) { return neko_vec4_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w); }

neko_static_inline neko_vec4 neko_vec4_scale(neko_vec4 v, f32 s) { return neko_vec4_ctor(v.x / s, v.y / s, v.z / s, v.w / s); }

neko_static_inline f32 neko_vec4_dot(neko_vec4 v0, neko_vec4 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w); }

neko_static_inline f32 neko_vec4_len(neko_vec4 v) { return (f32)sqrt(neko_vec4_dot(v, v)); }

neko_static_inline neko_vec4 neko_vec4_project_onto(neko_vec4 v0, neko_vec4 v1) {
    f32 dot = neko_vec4_dot(v0, v1);
    f32 len = neko_vec4_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return neko_vec4_scale(v1, dot / len);
}

neko_static_inline neko_vec4 neko_vec4_norm(neko_vec4 v) { return neko_vec4_scale(v, 1.0f / neko_vec4_len(v)); }

neko_static_inline f32 neko_vec4_dist(neko_vec4 v0, neko_vec4 v1) {
    f32 dx = (v0.x - v1.x);
    f32 dy = (v0.y - v1.y);
    f32 dz = (v0.z - v1.z);
    f32 dw = (v0.w - v1.w);
    return (sqrt(dx * dx + dy * dy + dz * dz + dw * dw));
}

neko_static_inline bool neko_vec4_equal(neko_vec4 a, neko_vec4 b) { return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w); }

/*================================================================================
// Useful Vector Functions
================================================================================*/

neko_static_inline neko_vec3 neko_v4_to_v3(neko_vec4 v) { return neko_v3(v.x, v.y, v.z); }

/*================================================================================
// Mat4x4
================================================================================*/

/*
    Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct neko_mat4 {
    f32 elements[16];
} neko_mat4;

neko_static_inline neko_mat4 neko_mat4_diag(f32 val) {
    neko_mat4 m;
    memset(m.elements, 0, sizeof(m.elements));
    m.elements[0 + 0 * 4] = val;
    m.elements[1 + 1 * 4] = val;
    m.elements[2 + 2 * 4] = val;
    m.elements[3 + 3 * 4] = val;
    return m;
}

#define neko_mat4_identity() neko_mat4_diag(1.0f)

neko_static_inline neko_mat4 neko_mat4_ctor() {
    neko_mat4 mat = {0};
    return mat;
}

neko_static_inline neko_mat4 neko_mat4_mul(neko_mat4 m0, neko_mat4 m1) {
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

neko_static_inline void neko_mat4_set_elements(neko_mat4 *m, f32 *elements, u32 count) {
    for (u32 i = 0; i < count; ++i) {
        m->elements[i] = elements[i];
    }
}

neko_static_inline neko_mat4 neko_mat4_transpose(neko_mat4 m) {
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

neko_static_inline neko_mat4 neko_mat4_inverse(neko_mat4 m) {
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

    f64 determinant = m.elements[0] * temp[0] + m.elements[1] * temp[4] + m.elements[2] * temp[8] + m.elements[3] * temp[12];
    determinant = 1.0 / determinant;

    for (int i = 0; i < 4 * 4; i++) res.elements[i] = temp[i] * determinant;

    return res;
}

/*
    f32 l : left
    f32 r : right
    f32 b : bottom
    f32 t : top
    f32 n : near
    f32 f : far
*/
neko_static_inline neko_mat4 neko_mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
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

neko_static_inline neko_mat4 neko_mat4_perspective(f32 fov, f32 asp_ratio, f32 n, f32 f) {
    // Zero matrix
    neko_mat4 m_res = neko_mat4_ctor();

    f32 q = 1.0f / tan(neko_deg_to_rad(0.5f * fov));
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

neko_static_inline neko_mat4 neko_mat4_translate(const neko_vec3 v) {
    neko_mat4 m_res = neko_mat4_identity();

    m_res.elements[0 + 4 * 3] = v.x;
    m_res.elements[1 + 4 * 3] = v.y;
    m_res.elements[2 + 4 * 3] = v.z;

    return m_res;
}

neko_static_inline neko_mat4 neko_mat4_scale(const neko_vec3 v) {
    neko_mat4 m_res = neko_mat4_identity();
    m_res.elements[0 + 0 * 4] = v.x;
    m_res.elements[1 + 1 * 4] = v.y;
    m_res.elements[2 + 2 * 4] = v.z;
    return m_res;
}

neko_static_inline neko_mat4 neko_mat4_rotate(f32 angle, neko_vec3 axis) {
    neko_mat4 m_res = neko_mat4_identity();

    f32 a = neko_deg_to_rad(angle);
    f32 c = cos(a);
    f32 s = sin(a);

    f32 x = axis.x;
    f32 y = axis.y;
    f32 z = axis.z;

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

neko_static_inline neko_mat4 neko_mat4_look_at(neko_vec3 position, neko_vec3 target, neko_vec3 up) {
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

neko_static_inline neko_vec3 neko_mat4_mul_vec3(neko_mat4 m, neko_vec3 v) {
    m = neko_mat4_transpose(m);
    return neko_vec3_ctor(m.elements[0 * 4 + 0] * v.x + m.elements[0 * 4 + 1] * v.y + m.elements[0 * 4 + 2] * v.z,
                          m.elements[1 * 4 + 0] * v.x + m.elements[1 * 4 + 1] * v.y + m.elements[1 * 4 + 2] * v.z,
                          m.elements[2 * 4 + 0] * v.x + m.elements[2 * 4 + 1] * v.y + m.elements[2 * 4 + 2] * v.z);
}

neko_static_inline neko_vec4 neko_mat4_mul_vec4(neko_mat4 m, neko_vec4 v) {
    m = neko_mat4_transpose(m);
    return neko_vec4_ctor(m.elements[0 * 4 + 0] * v.x + m.elements[0 * 4 + 1] * v.y + m.elements[0 * 4 + 2] * v.z + m.elements[0 * 4 + 3] * v.w,
                          m.elements[1 * 4 + 0] * v.x + m.elements[1 * 4 + 1] * v.y + m.elements[1 * 4 + 2] * v.z + m.elements[1 * 4 + 3] * v.w,
                          m.elements[2 * 4 + 0] * v.x + m.elements[2 * 4 + 1] * v.y + m.elements[2 * 4 + 2] * v.z + m.elements[2 * 4 + 3] * v.w,
                          m.elements[3 * 4 + 0] * v.x + m.elements[3 * 4 + 1] * v.y + m.elements[3 * 4 + 2] * v.z + m.elements[3 * 4 + 3] * v.w);
}

/*================================================================================
// Quaternion
================================================================================*/

typedef struct {
    union {
        f32 xyzw[4];
        struct {
            f32 x, y, z, w;
        };
    };
} neko_quat;

neko_static_inline neko_quat neko_quat_default() {
    neko_quat q;
    q.x = 0.f;
    q.y = 0.f;
    q.z = 0.f;
    q.w = 1.f;
    return q;
}

neko_static_inline neko_quat neko_quat_ctor(f32 _x, f32 _y, f32 _z, f32 _w) {
    neko_quat q;
    q.x = _x;
    q.y = _y;
    q.z = _z;
    q.w = _w;
    return q;
}

neko_static_inline neko_quat neko_quat_add(neko_quat q0, neko_quat q1) { return neko_quat_ctor(q0.x + q1.x, q0.y + q1.y, q0.z + q1.z, q0.w + q1.w); }

neko_static_inline neko_quat neko_quat_sub(neko_quat q0, neko_quat q1) { return neko_quat_ctor(q0.x - q1.x, q0.y - q1.y, q0.z - q1.z, q0.w - q1.w); }

neko_static_inline neko_quat neko_quat_mul(neko_quat q0, neko_quat q1) {
    return neko_quat_ctor(q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z, q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x, q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
                          q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z);
}

neko_static_inline neko_quat neko_quat_mul_list(u32 count, ...) {
    va_list ap;
    neko_quat q = neko_quat_default();
    va_start(ap, count);
    for (u32 i = 0; i < count; ++i) {
        q = neko_quat_mul(q, va_arg(ap, neko_quat));
    }
    va_end(ap);
    return q;
}

neko_static_inline neko_quat neko_quat_mul_quat(neko_quat q0, neko_quat q1) {
    return neko_quat_ctor(q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z, q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x, q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
                          q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z);
}

neko_static_inline neko_quat neko_quat_scale(neko_quat q, f32 s) { return neko_quat_ctor(q.x * s, q.y * s, q.z * s, q.w * s); }

neko_static_inline f32 neko_quat_dot(neko_quat q0, neko_quat q1) { return (f32)(q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w); }

neko_static_inline neko_quat neko_quat_conjugate(neko_quat q) { return (neko_quat_ctor(-q.x, -q.y, -q.z, q.w)); }

neko_static_inline f32 neko_quat_len(neko_quat q) { return (f32)sqrt(neko_quat_dot(q, q)); }

neko_static_inline neko_quat neko_quat_norm(neko_quat q) { return neko_quat_scale(q, 1.0f / neko_quat_len(q)); }

neko_static_inline neko_quat neko_quat_cross(neko_quat q0, neko_quat q1) {
    return neko_quat_ctor(q0.x * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y, q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z, q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x,
                          q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z);
}

// Inverse := Conjugate / Dot;
neko_static_inline neko_quat neko_quat_inverse(neko_quat q) { return (neko_quat_scale(neko_quat_conjugate(q), 1.0f / neko_quat_dot(q, q))); }

neko_static_inline neko_vec3 neko_quat_rotate(neko_quat q, neko_vec3 v) {
    // nVidia SDK implementation
    neko_vec3 qvec = neko_vec3_ctor(q.x, q.y, q.z);
    neko_vec3 uv = neko_vec3_cross(qvec, v);
    neko_vec3 uuv = neko_vec3_cross(qvec, uv);
    uv = neko_vec3_scale(uv, 2.f * q.w);
    uuv = neko_vec3_scale(uuv, 2.f);
    return (neko_vec3_add(v, neko_vec3_add(uv, uuv)));
}

neko_static_inline neko_quat neko_quat_angle_axis(f32 rad, neko_vec3 axis) {
    // Normalize axis
    neko_vec3 a = neko_vec3_norm(axis);

    // Get scalar
    f32 half_angle = 0.5f * rad;
    f32 s = sin(half_angle);

    return neko_quat_ctor(a.x * s, a.y * s, a.z * s, cos(half_angle));
}

neko_static_inline neko_quat neko_quat_slerp(neko_quat a, neko_quat b, f32 t) {
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
        f32 omega = acosf(c);
        f32 s = sinf(omega);
        sclp = sinf((1.0f - t) * omega) / s;
        sclq = sinf(t * omega) / s;
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

#define quat_axis_angle(axis, angle) neko_quat_angle_axis(angle, axis)

/*
 * @brief Convert given quaternion param into equivalent 4x4 rotation matrix
 * @note: From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
 */
neko_static_inline neko_mat4 neko_quat_to_mat4(neko_quat _q) {
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

neko_static_inline neko_quat neko_quat_from_euler(f32 yaw_deg, f32 pitch_deg, f32 roll_deg) {
    f32 yaw = neko_deg_to_rad(yaw_deg);
    f32 pitch = neko_deg_to_rad(pitch_deg);
    f32 roll = neko_deg_to_rad(roll_deg);

    neko_quat q;
    f32 cy = cos(yaw * 0.5f);
    f32 sy = sin(yaw * 0.5f);
    f32 cr = cos(roll * 0.5f);
    f32 sr = sin(roll * 0.5f);
    f32 cp = cos(pitch * 0.5f);
    f32 sp = sin(pitch * 0.5f);

    q.x = cy * sr * cp - sy * cr * sp;
    q.y = cy * cr * sp + sy * sr * cp;
    q.z = sy * cr * cp - cy * sr * sp;
    q.w = cy * cr * cp + sy * sr * sp;

    return q;
}

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
    neko_vec3 position;
    neko_quat rotation;
    neko_vec3 scale;
} neko_vqs;

neko_static_inline neko_vqs neko_vqs_ctor(neko_vec3 tns, neko_quat rot, neko_vec3 scl) {
    neko_vqs t;
    t.position = tns;
    t.rotation = rot;
    t.scale = scl;
    return t;
}

neko_static_inline neko_vqs neko_vqs_default() {
    neko_vqs t = neko_vqs_ctor(neko_vec3_ctor(0.0f, 0.0f, 0.0f), neko_quat_ctor(0.0f, 0.0f, 0.0f, 1.0f), neko_vec3_ctor(1.0f, 1.0f, 1.0f));
    return t;
}

// AbsScale = ParentScale * LocalScale
// AbsRot   = LocalRot * ParentRot
// AbsTrans = ParentPos + [ParentRot * (ParentScale * LocalPos)]
neko_static_inline neko_vqs neko_vqs_absolute_transform(const neko_vqs *local, const neko_vqs *parent) {
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
neko_static_inline neko_vqs neko_vqs_relative_transform(const neko_vqs *absolute, const neko_vqs *parent) {
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

neko_static_inline neko_mat4 neko_vqs_to_mat4(const neko_vqs *transform) {
    neko_mat4 mat = neko_mat4_identity();
    neko_mat4 trans = neko_mat4_translate(transform->position);
    neko_mat4 rot = neko_quat_to_mat4(transform->rotation);
    neko_mat4 scl = neko_mat4_scale(transform->scale);
    mat = neko_mat4_mul(mat, trans);
    mat = neko_mat4_mul(mat, rot);
    mat = neko_mat4_mul(mat, scl);
    return mat;
}

/*================================================================================
// Ray
================================================================================*/

typedef struct {
    neko_vec3 point;
    neko_vec3 direction;
} neko_ray;

neko_static_inline neko_ray neko_ray_ctor(neko_vec3 pt, neko_vec3 dir) {
    neko_ray r;
    r.point = pt;
    r.direction = dir;
    return r;
}

/*================================================================================
// Plane
================================================================================*/

typedef struct neko_plane_t {
    union {
        neko_vec3 n;
        struct {
            f32 a;
            f32 b;
            f32 c;
        };
    };

    f32 d;
} neko_plane_t;

/*================================================================================
// Mat22
================================================================================*/

struct neko_mat22 {
    neko_vec2 col1, col2;
};

neko_inline neko_mat22 neko_mat22_transpose(const neko_mat22 &mat) { return neko_mat22(neko_vec2{mat.col1.x, mat.col2.x}, neko_vec2{mat.col1.y, mat.col2.y}); }

neko_inline neko_mat22 neko_mat22_invert(const neko_mat22 &mat) {
    float a = mat.col1.x, b = mat.col2.x, c = mat.col1.y, d = mat.col2.y;
    neko_mat22 B;
    float det = a * d - b * c;
    neko_assert(det != 0.0f);
    det = 1.0f / det;
    B.col1.x = det * d;
    B.col2.x = -det * b;
    B.col1.y = -det * c;
    B.col2.y = det * a;
    return B;
}

neko_inline neko_mat22 neko_mat22_ctor() { return neko_mat22{}; }

neko_inline neko_mat22 neko_mat22_ctor(float angle) {
    neko_mat22 mat;
    float c = cosf(angle), s = sinf(angle);
    mat.col1.x = c;
    mat.col2.x = -s;
    mat.col1.y = s;
    mat.col2.y = c;
    return mat;
}

neko_inline neko_mat22 neko_mat22_ctor(const neko_vec2 &col1, const neko_vec2 &col2) { return neko_mat22{col1, col2}; }

neko_inline neko_vec2 neko_vec2_cross(const neko_vec2 &a, float s) { return neko_vec2{s * a.y, -s * a.x}; }

neko_inline neko_vec2 neko_vec2_cross(float s, const neko_vec2 &a) { return neko_vec2{-s * a.y, s * a.x}; }

neko_inline neko_vec2 operator*(const neko_mat22 &A, const neko_vec2 &v) { return neko_vec2{A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y}; }

neko_inline neko_vec2 operator+(const neko_vec2 &a, const neko_vec2 &b) { return neko_vec2{a.x + b.x, a.y + b.y}; }

neko_inline neko_vec2 operator-(const neko_vec2 &a, const neko_vec2 &b) { return neko_vec2{a.x - b.x, a.y - b.y}; }

neko_inline neko_vec2 operator*(float s, const neko_vec2 &v) { return neko_vec2{s * v.x, s * v.y}; }

neko_inline neko_mat22 operator+(const neko_mat22 &A, const neko_mat22 &B) { return neko_mat22(A.col1 + B.col1, A.col2 + B.col2); }

neko_inline neko_mat22 operator*(const neko_mat22 &A, const neko_mat22 &B) { return neko_mat22(A * B.col1, A * B.col2); }

neko_inline neko_vec2 neko_vec2_abs(const neko_vec2 &a) { return neko_vec2{fabsf(a.x), fabsf(a.y)}; }

neko_inline neko_mat22 neko_mat22_abs(const neko_mat22 &A) { return neko_mat22(neko_vec2_abs(A.col1), neko_vec2_abs(A.col2)); }

// Random number in range [-1,1]
neko_inline float neko_math_rand() {
    float r = (float)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = 2.0f * r - 1.0f;
    return r;
}

neko_inline float neko_math_rand(float lo, float hi) {
    float r = (float)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = (hi - lo) * r + lo;
    return r;
}

/*================================================================================
// Utils
================================================================================*/

/*
    min is top left of rect,
    max is bottom right
*/
typedef struct neko_rect_t {
    neko_vec2 min;
    neko_vec2 max;
} neko_rect_t;

neko_static_inline bool neko_rect_vs_rect(neko_rect_t a, neko_rect_t b) {
    if (a.max.x > b.min.x && a.max.y > b.min.y && a.min.x < b.max.x && a.min.y < b.max.y) {
        return true;
    }

    return false;
}

neko_inline f32 neko_sign(f32 x) { return x < 0.0f ? -1.0f : 1.0f; }

#define NEKO_MATH_USE_SSE 1
#if NEKO_MATH_USE_SSE
#include <pmmintrin.h>
#include <xmmintrin.h>
#endif

#define NEKO_MATH_PREFIX(name) neko_fast_##name

#define NEKO_MATH_SQRTF sqrtf
#define NEKO_MATH_SINF sinf
#define NEKO_MATH_COSF cosf
#define NEKO_MATH_TANF tanf
#define NEKO_MATH_ACOSF acosf

//----------------------------------------------------------------------//
// STRUCT DEFINITIONS:

typedef int neko_fast_bool;

// a 2-dimensional vector of floats
typedef union {
    float v[2];
    struct {
        float x, y;
    };
    struct {
        float w, h;
    };
} neko_fast_vec2;

// a 3-dimensional vector of floats
typedef union {
    float v[3];
    struct {
        float x, y, z;
    };
    struct {
        float w, h, d;
    };
    struct {
        float r, g, b;
    };
} neko_fast_vec3;

// a 4-dimensional vector of floats
typedef union {
    float v[4];
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };

#if NEKO_MATH_USE_SSE

    __m128 packed;

#endif
} neko_fast_vec4;

//-----------------------------//
// matrices are column-major

typedef union {
    float m[3][3];
} neko_fast_mat3;

typedef union {
    float m[4][4];

#if NEKO_MATH_USE_SSE

    __m128 packed[4];  // array of columns

#endif
} neko_fast_mat4;

//-----------------------------//

typedef union {
    float q[4];
    struct {
        float x, y, z, w;
    };

#if NEKO_MATH_USE_SSE

    __m128 packed;

#endif
} neko_fast_quaternion;

// typedefs:
typedef neko_fast_vec2 neko_voxel_vec2;
typedef neko_fast_vec3 neko_voxel_vec3;
typedef neko_fast_vec4 neko_voxel_vec4;
typedef neko_fast_mat3 neko_voxel_mat3;
typedef neko_fast_mat4 neko_voxel_mat4;
typedef neko_fast_quaternion neko_voxel_quaternion;

//----------------------------------------------------------------------//
// HELPER FUNCS:

#define NEKO_MATH_MIN(x, y) ((x) < (y) ? (x) : (y))
#define NEKO_MATH_MAX(x, y) ((x) > (y) ? (x) : (y))
#define NEKO_MATH_ABS(x) ((x) > 0 ? (x) : -(x))

neko_static_inline float NEKO_MATH_PREFIX(rad_to_deg)(float rad) { return rad * 57.2957795131f; }

neko_static_inline float NEKO_MATH_PREFIX(deg_to_rad)(float deg) { return deg * 0.01745329251f; }

#if NEKO_MATH_USE_SSE

neko_static_inline __m128 NEKO_MATH_PREFIX(mat4_mult_column_sse)(__m128 c1, neko_fast_mat4 m2) {
    __m128 result;

    result = _mm_mul_ps(_mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 0, 0)), m2.packed[0]);
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(c1, c1, _MM_SHUFFLE(1, 1, 1, 1)), m2.packed[1]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(c1, c1, _MM_SHUFFLE(2, 2, 2, 2)), m2.packed[2]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(c1, c1, _MM_SHUFFLE(3, 3, 3, 3)), m2.packed[3]));

    return result;
}

#endif

//----------------------------------------------------------------------//
// VECTOR FUNCTIONS:

// addition:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_add)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_vec2 result;

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_add)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_add)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_add_ps(v1.packed, v2.packed);

#else

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    result.w = v1.w + v2.w;

#endif

    return result;
}

// subtraction:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_sub)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_vec2 result;

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_sub)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_sub)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_sub_ps(v1.packed, v2.packed);

#else

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    result.w = v1.w - v2.w;

#endif

    return result;
}

// multiplication:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_mult)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_vec2 result;

    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_mult)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_mult)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_mul_ps(v1.packed, v2.packed);

#else

    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    result.w = v1.w * v2.w;

#endif

    return result;
}

// division:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_div)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_vec2 result;

    result.x = v1.x / v2.x;
    result.y = v1.y / v2.y;

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_div)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = v1.x / v2.x;
    result.y = v1.y / v2.y;
    result.z = v1.z / v2.z;

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_div)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_div_ps(v1.packed, v2.packed);

#else

    result.x = v1.x / v2.x;
    result.y = v1.y / v2.y;
    result.z = v1.z / v2.z;
    result.w = v1.w / v2.w;

#endif

    return result;
}

// scalar multiplication:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_scale)(neko_fast_vec2 v, float s) {
    neko_fast_vec2 result;

    result.x = v.x * s;
    result.y = v.y * s;

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_scale)(neko_fast_vec3 v, float s) {
    neko_fast_vec3 result;

    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_scale)(neko_fast_vec4 v, float s) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    __m128 scale = _mm_set1_ps(s);
    result.packed = _mm_mul_ps(v.packed, scale);

#else

    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    result.w = v.w * s;

#endif

    return result;
}

// dot product:

neko_static_inline float NEKO_MATH_PREFIX(vec2_dot)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    float result;

    result = v1.x * v2.x + v1.y * v2.y;

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(vec3_dot)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    float result;

    result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(vec4_dot)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    float result;

#if NEKO_MATH_USE_SSE

    __m128 r = _mm_mul_ps(v1.packed, v2.packed);
    r = _mm_hadd_ps(r, r);
    r = _mm_hadd_ps(r, r);
    _mm_store_ss(&result, r);

#else

    result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;

#endif

    return result;
}

// cross product

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_cross)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = (v1.y * v2.z) - (v1.z * v2.y);
    result.y = (v1.z * v2.x) - (v1.x * v2.z);
    result.z = (v1.x * v2.y) - (v1.y * v2.x);

    return result;
}

// length:

neko_static_inline float NEKO_MATH_PREFIX(vec2_length)(neko_fast_vec2 v) {
    float result;

    result = NEKO_MATH_SQRTF(NEKO_MATH_PREFIX(vec2_dot)(v, v));

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(vec3_length)(neko_fast_vec3 v) {
    float result;

    result = NEKO_MATH_SQRTF(NEKO_MATH_PREFIX(vec3_dot)(v, v));

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(vec4_length)(neko_fast_vec4 v) {
    float result;

    result = NEKO_MATH_SQRTF(NEKO_MATH_PREFIX(vec4_dot)(v, v));

    return result;
}

// normalize:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_normalize)(neko_fast_vec2 v) {
    neko_fast_vec2 result = {0};

    float invLen = NEKO_MATH_PREFIX(vec2_length)(v);
    if (invLen != 0.0f) {
        invLen = 1.0f / invLen;
        result.x = v.x * invLen;
        result.y = v.y * invLen;
    }

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_normalize)(neko_fast_vec3 v) {
    neko_fast_vec3 result = {0};

    float invLen = NEKO_MATH_PREFIX(vec3_length)(v);
    if (invLen != 0.0f) {
        invLen = 1.0f / invLen;
        result.x = v.x * invLen;
        result.y = v.y * invLen;
        result.z = v.z * invLen;
    }

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_normalize)(neko_fast_vec4 v) {
    neko_fast_vec4 result = {0};

    float len = NEKO_MATH_PREFIX(vec4_length)(v);
    if (len != 0.0f) {
#if NEKO_MATH_USE_SSE

        __m128 scale = _mm_set1_ps(len);
        result.packed = _mm_div_ps(v.packed, scale);

#else

        float invLen = 1.0f / len;

        result.x = v.x * invLen;
        result.y = v.y * invLen;
        result.z = v.z * invLen;
        result.w = v.w * invLen;

#endif
    }

    return result;
}

// distance:

neko_static_inline float NEKO_MATH_PREFIX(vec2_distance)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    float result;

    neko_fast_vec2 to = NEKO_MATH_PREFIX(vec2_sub)(v1, v2);
    result = NEKO_MATH_PREFIX(vec2_length)(to);

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(vec3_distance)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    float result;

    neko_fast_vec3 to = NEKO_MATH_PREFIX(vec3_sub)(v1, v2);
    result = NEKO_MATH_PREFIX(vec3_length)(to);

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(vec4_distance)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    float result;

    neko_fast_vec4 to = NEKO_MATH_PREFIX(vec4_sub)(v1, v2);
    result = NEKO_MATH_PREFIX(vec4_length)(to);

    return result;
}

// equality:

neko_static_inline neko_fast_bool NEKO_MATH_PREFIX(vec2_equals)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_bool result;

    result = (v1.x == v2.x) && (v1.y == v2.y);

    return result;
}

neko_static_inline neko_fast_bool NEKO_MATH_PREFIX(vec3_equals)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_bool result;

    result = (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);

    return result;
}

neko_static_inline neko_fast_bool NEKO_MATH_PREFIX(vec4_equals)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_bool result;

    // TODO: there are SIMD instructions for floating point equality, find a way to get a single bool from them
    result = (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z) && (v1.w == v2.w);

    return result;
}

// min:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_min)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_vec2 result;

    result.x = NEKO_MATH_MIN(v1.x, v2.x);
    result.y = NEKO_MATH_MIN(v1.y, v2.y);

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_min)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = NEKO_MATH_MIN(v1.x, v2.x);
    result.y = NEKO_MATH_MIN(v1.y, v2.y);
    result.z = NEKO_MATH_MIN(v1.z, v2.z);

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_min)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_min_ps(v1.packed, v2.packed);

#else

    result.x = NEKO_MATH_MIN(v1.x, v2.x);
    result.y = NEKO_MATH_MIN(v1.y, v2.y);
    result.z = NEKO_MATH_MIN(v1.z, v2.z);
    result.w = NEKO_MATH_MIN(v1.w, v2.w);

#endif

    return result;
}

// max:

neko_static_inline neko_fast_vec2 NEKO_MATH_PREFIX(vec2_max)(neko_fast_vec2 v1, neko_fast_vec2 v2) {
    neko_fast_vec2 result;

    result.x = NEKO_MATH_MAX(v1.x, v2.x);
    result.y = NEKO_MATH_MAX(v1.y, v2.y);

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(vec3_max)(neko_fast_vec3 v1, neko_fast_vec3 v2) {
    neko_fast_vec3 result;

    result.x = NEKO_MATH_MAX(v1.x, v2.x);
    result.y = NEKO_MATH_MAX(v1.y, v2.y);
    result.z = NEKO_MATH_MAX(v1.z, v2.z);

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(vec4_max)(neko_fast_vec4 v1, neko_fast_vec4 v2) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_max_ps(v1.packed, v2.packed);

#else

    result.x = NEKO_MATH_MAX(v1.x, v2.x);
    result.y = NEKO_MATH_MAX(v1.y, v2.y);
    result.z = NEKO_MATH_MAX(v1.z, v2.z);
    result.w = NEKO_MATH_MAX(v1.w, v2.w);

#endif

    return result;
}

//----------------------------------------------------------------------//
// MATRIX FUNCTIONS:

// initialization:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_identity)() {
    neko_fast_mat3 result = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_identity)() {
    neko_fast_mat4 result = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

// addition:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_add)(neko_fast_mat3 m1, neko_fast_mat3 m2) {
    neko_fast_mat3 result;

    result.m[0][0] = m1.m[0][0] + m2.m[0][0];
    result.m[0][1] = m1.m[0][1] + m2.m[0][1];
    result.m[0][2] = m1.m[0][2] + m2.m[0][2];
    result.m[1][0] = m1.m[1][0] + m2.m[1][0];
    result.m[1][1] = m1.m[1][1] + m2.m[1][1];
    result.m[1][2] = m1.m[1][2] + m2.m[1][2];
    result.m[2][0] = m1.m[2][0] + m2.m[2][0];
    result.m[2][1] = m1.m[2][1] + m2.m[2][1];
    result.m[2][2] = m1.m[2][2] + m2.m[2][2];

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_add)(neko_fast_mat4 m1, neko_fast_mat4 m2) {
    neko_fast_mat4 result;

#if NEKO_MATH_USE_SSE

    result.packed[0] = _mm_add_ps(m1.packed[0], m2.packed[0]);
    result.packed[1] = _mm_add_ps(m1.packed[1], m2.packed[1]);
    result.packed[2] = _mm_add_ps(m1.packed[2], m2.packed[2]);
    result.packed[3] = _mm_add_ps(m1.packed[3], m2.packed[3]);

#else

    result.m[0][0] = m1.m[0][0] + m2.m[0][0];
    result.m[0][1] = m1.m[0][1] + m2.m[0][1];
    result.m[0][2] = m1.m[0][2] + m2.m[0][2];
    result.m[0][3] = m1.m[0][3] + m2.m[0][3];
    result.m[1][0] = m1.m[1][0] + m2.m[1][0];
    result.m[1][1] = m1.m[1][1] + m2.m[1][1];
    result.m[1][2] = m1.m[1][2] + m2.m[1][2];
    result.m[1][3] = m1.m[1][3] + m2.m[1][3];
    result.m[2][0] = m1.m[2][0] + m2.m[2][0];
    result.m[2][1] = m1.m[2][1] + m2.m[2][1];
    result.m[2][2] = m1.m[2][2] + m2.m[2][2];
    result.m[2][3] = m1.m[2][3] + m2.m[2][3];
    result.m[3][0] = m1.m[3][0] + m2.m[3][0];
    result.m[3][1] = m1.m[3][1] + m2.m[3][1];
    result.m[3][2] = m1.m[3][2] + m2.m[3][2];
    result.m[3][3] = m1.m[3][3] + m2.m[3][3];

#endif

    return result;
}

// subtraction:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_sub)(neko_fast_mat3 m1, neko_fast_mat3 m2) {
    neko_fast_mat3 result;

    result.m[0][0] = m1.m[0][0] - m2.m[0][0];
    result.m[0][1] = m1.m[0][1] - m2.m[0][1];
    result.m[0][2] = m1.m[0][2] - m2.m[0][2];
    result.m[1][0] = m1.m[1][0] - m2.m[1][0];
    result.m[1][1] = m1.m[1][1] - m2.m[1][1];
    result.m[1][2] = m1.m[1][2] - m2.m[1][2];
    result.m[2][0] = m1.m[2][0] - m2.m[2][0];
    result.m[2][1] = m1.m[2][1] - m2.m[2][1];
    result.m[2][2] = m1.m[2][2] - m2.m[2][2];

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_sub)(neko_fast_mat4 m1, neko_fast_mat4 m2) {
    neko_fast_mat4 result;

#if NEKO_MATH_USE_SSE

    result.packed[0] = _mm_sub_ps(m1.packed[0], m2.packed[0]);
    result.packed[1] = _mm_sub_ps(m1.packed[1], m2.packed[1]);
    result.packed[2] = _mm_sub_ps(m1.packed[2], m2.packed[2]);
    result.packed[3] = _mm_sub_ps(m1.packed[3], m2.packed[3]);

#else

    result.m[0][0] = m1.m[0][0] - m2.m[0][0];
    result.m[0][1] = m1.m[0][1] - m2.m[0][1];
    result.m[0][2] = m1.m[0][2] - m2.m[0][2];
    result.m[0][3] = m1.m[0][3] - m2.m[0][3];
    result.m[1][0] = m1.m[1][0] - m2.m[1][0];
    result.m[1][1] = m1.m[1][1] - m2.m[1][1];
    result.m[1][2] = m1.m[1][2] - m2.m[1][2];
    result.m[1][3] = m1.m[1][3] - m2.m[1][3];
    result.m[2][0] = m1.m[2][0] - m2.m[2][0];
    result.m[2][1] = m1.m[2][1] - m2.m[2][1];
    result.m[2][2] = m1.m[2][2] - m2.m[2][2];
    result.m[2][3] = m1.m[2][3] - m2.m[2][3];
    result.m[3][0] = m1.m[3][0] - m2.m[3][0];
    result.m[3][1] = m1.m[3][1] - m2.m[3][1];
    result.m[3][2] = m1.m[3][2] - m2.m[3][2];
    result.m[3][3] = m1.m[3][3] - m2.m[3][3];

#endif

    return result;
}

// multiplication:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_mult)(neko_fast_mat3 m1, neko_fast_mat3 m2) {
    neko_fast_mat3 result;

    result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[1][0] * m2.m[0][1] + m1.m[2][0] * m2.m[0][2];
    result.m[0][1] = m1.m[0][1] * m2.m[0][0] + m1.m[1][1] * m2.m[0][1] + m1.m[2][1] * m2.m[0][2];
    result.m[0][2] = m1.m[0][2] * m2.m[0][0] + m1.m[1][2] * m2.m[0][1] + m1.m[2][2] * m2.m[0][2];
    result.m[1][0] = m1.m[0][0] * m2.m[1][0] + m1.m[1][0] * m2.m[1][1] + m1.m[2][0] * m2.m[1][2];
    result.m[1][1] = m1.m[0][1] * m2.m[1][0] + m1.m[1][1] * m2.m[1][1] + m1.m[2][1] * m2.m[1][2];
    result.m[1][2] = m1.m[0][2] * m2.m[1][0] + m1.m[1][2] * m2.m[1][1] + m1.m[2][2] * m2.m[1][2];
    result.m[2][0] = m1.m[0][0] * m2.m[2][0] + m1.m[1][0] * m2.m[2][1] + m1.m[2][0] * m2.m[2][2];
    result.m[2][1] = m1.m[0][1] * m2.m[2][0] + m1.m[1][1] * m2.m[2][1] + m1.m[2][1] * m2.m[2][2];
    result.m[2][2] = m1.m[0][2] * m2.m[2][0] + m1.m[1][2] * m2.m[2][1] + m1.m[2][2] * m2.m[2][2];

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_mult)(neko_fast_mat4 m1, neko_fast_mat4 m2) {
    neko_fast_mat4 result;

#if NEKO_MATH_USE_SSE

    result.packed[0] = NEKO_MATH_PREFIX(mat4_mult_column_sse)(m2.packed[0], m1);
    result.packed[1] = NEKO_MATH_PREFIX(mat4_mult_column_sse)(m2.packed[1], m1);
    result.packed[2] = NEKO_MATH_PREFIX(mat4_mult_column_sse)(m2.packed[2], m1);
    result.packed[3] = NEKO_MATH_PREFIX(mat4_mult_column_sse)(m2.packed[3], m1);

#else

    result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[1][0] * m2.m[0][1] + m1.m[2][0] * m2.m[0][2] + m1.m[3][0] * m2.m[0][3];
    result.m[0][1] = m1.m[0][1] * m2.m[0][0] + m1.m[1][1] * m2.m[0][1] + m1.m[2][1] * m2.m[0][2] + m1.m[3][1] * m2.m[0][3];
    result.m[0][2] = m1.m[0][2] * m2.m[0][0] + m1.m[1][2] * m2.m[0][1] + m1.m[2][2] * m2.m[0][2] + m1.m[3][2] * m2.m[0][3];
    result.m[0][3] = m1.m[0][3] * m2.m[0][0] + m1.m[1][3] * m2.m[0][1] + m1.m[2][3] * m2.m[0][2] + m1.m[3][3] * m2.m[0][3];
    result.m[1][0] = m1.m[0][0] * m2.m[1][0] + m1.m[1][0] * m2.m[1][1] + m1.m[2][0] * m2.m[1][2] + m1.m[3][0] * m2.m[1][3];
    result.m[1][1] = m1.m[0][1] * m2.m[1][0] + m1.m[1][1] * m2.m[1][1] + m1.m[2][1] * m2.m[1][2] + m1.m[3][1] * m2.m[1][3];
    result.m[1][2] = m1.m[0][2] * m2.m[1][0] + m1.m[1][2] * m2.m[1][1] + m1.m[2][2] * m2.m[1][2] + m1.m[3][2] * m2.m[1][3];
    result.m[1][3] = m1.m[0][3] * m2.m[1][0] + m1.m[1][3] * m2.m[1][1] + m1.m[2][3] * m2.m[1][2] + m1.m[3][3] * m2.m[1][3];
    result.m[2][0] = m1.m[0][0] * m2.m[2][0] + m1.m[1][0] * m2.m[2][1] + m1.m[2][0] * m2.m[2][2] + m1.m[3][0] * m2.m[2][3];
    result.m[2][1] = m1.m[0][1] * m2.m[2][0] + m1.m[1][1] * m2.m[2][1] + m1.m[2][1] * m2.m[2][2] + m1.m[3][1] * m2.m[2][3];
    result.m[2][2] = m1.m[0][2] * m2.m[2][0] + m1.m[1][2] * m2.m[2][1] + m1.m[2][2] * m2.m[2][2] + m1.m[3][2] * m2.m[2][3];
    result.m[2][3] = m1.m[0][3] * m2.m[2][0] + m1.m[1][3] * m2.m[2][1] + m1.m[2][3] * m2.m[2][2] + m1.m[3][3] * m2.m[2][3];
    result.m[3][0] = m1.m[0][0] * m2.m[3][0] + m1.m[1][0] * m2.m[3][1] + m1.m[2][0] * m2.m[3][2] + m1.m[3][0] * m2.m[3][3];
    result.m[3][1] = m1.m[0][1] * m2.m[3][0] + m1.m[1][1] * m2.m[3][1] + m1.m[2][1] * m2.m[3][2] + m1.m[3][1] * m2.m[3][3];
    result.m[3][2] = m1.m[0][2] * m2.m[3][0] + m1.m[1][2] * m2.m[3][1] + m1.m[2][2] * m2.m[3][2] + m1.m[3][2] * m2.m[3][3];
    result.m[3][3] = m1.m[0][3] * m2.m[3][0] + m1.m[1][3] * m2.m[3][1] + m1.m[2][3] * m2.m[3][2] + m1.m[3][3] * m2.m[3][3];

#endif

    return result;
}

neko_static_inline neko_fast_vec3 NEKO_MATH_PREFIX(mat3_mult_vec3)(neko_fast_mat3 m, neko_fast_vec3 v) {
    neko_fast_vec3 result;

    result.x = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z;
    result.y = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z;
    result.z = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z;

    return result;
}

neko_static_inline neko_fast_vec4 NEKO_MATH_PREFIX(mat4_mult_vec4)(neko_fast_mat4 m, neko_fast_vec4 v) {
    neko_fast_vec4 result;

#if NEKO_MATH_USE_SSE

    result.packed = NEKO_MATH_PREFIX(mat4_mult_column_sse)(v.packed, m);

#else

    result.x = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w;
    result.y = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w;
    result.z = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w;
    result.w = m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w;

#endif

    return result;
}

// transpose:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_transpose)(neko_fast_mat3 m) {
    neko_fast_mat3 result;

    result.m[0][0] = m.m[0][0];
    result.m[0][1] = m.m[1][0];
    result.m[0][2] = m.m[2][0];
    result.m[1][0] = m.m[0][1];
    result.m[1][1] = m.m[1][1];
    result.m[1][2] = m.m[2][1];
    result.m[2][0] = m.m[0][2];
    result.m[2][1] = m.m[1][2];
    result.m[2][2] = m.m[2][2];

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_transpose)(neko_fast_mat4 m) {
    neko_fast_mat4 result = m;

#if NEKO_MATH_USE_SSE

    _MM_TRANSPOSE4_PS(result.packed[0], result.packed[1], result.packed[2], result.packed[3]);

#else

    result.m[0][0] = m.m[0][0];
    result.m[0][1] = m.m[1][0];
    result.m[0][2] = m.m[2][0];
    result.m[0][3] = m.m[3][0];
    result.m[1][0] = m.m[0][1];
    result.m[1][1] = m.m[1][1];
    result.m[1][2] = m.m[2][1];
    result.m[1][3] = m.m[3][1];
    result.m[2][0] = m.m[0][2];
    result.m[2][1] = m.m[1][2];
    result.m[2][2] = m.m[2][2];
    result.m[2][3] = m.m[3][2];
    result.m[3][0] = m.m[0][3];
    result.m[3][1] = m.m[1][3];
    result.m[3][2] = m.m[2][3];
    result.m[3][3] = m.m[3][3];

#endif

    return result;
}

// inverse:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_inv)(neko_fast_mat3 m) {
    neko_fast_mat3 result;

    float det;
    float a = m.m[0][0], b = m.m[0][1], c = m.m[0][2], d = m.m[1][0], e = m.m[1][1], f = m.m[1][2], g = m.m[2][0], h = m.m[2][1], i = m.m[2][2];

    result.m[0][0] = e * i - f * h;
    result.m[0][1] = -(b * i - h * c);
    result.m[0][2] = b * f - e * c;
    result.m[1][0] = -(d * i - g * f);
    result.m[1][1] = a * i - c * g;
    result.m[1][2] = -(a * f - d * c);
    result.m[2][0] = d * h - g * e;
    result.m[2][1] = -(a * h - g * b);
    result.m[2][2] = a * e - b * d;

    det = 1.0f / (a * result.m[0][0] + b * result.m[1][0] + c * result.m[2][0]);

    result.m[0][0] *= det;
    result.m[0][1] *= det;
    result.m[0][2] *= det;
    result.m[1][0] *= det;
    result.m[1][1] *= det;
    result.m[1][2] *= det;
    result.m[2][0] *= det;
    result.m[2][1] *= det;
    result.m[2][2] *= det;

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_inv)(neko_fast_mat4 mat) {
    // TODO: this function is not SIMD optimized, figure out how to do it

    neko_fast_mat4 result;

    float tmp[6];
    float det;
    float a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2], d = mat.m[0][3], e = mat.m[1][0], f = mat.m[1][1], g = mat.m[1][2], h = mat.m[1][3], i = mat.m[2][0], j = mat.m[2][1], k = mat.m[2][2],
          l = mat.m[2][3], m = mat.m[3][0], n = mat.m[3][1], o = mat.m[3][2], p = mat.m[3][3];

    tmp[0] = k * p - o * l;
    tmp[1] = j * p - n * l;
    tmp[2] = j * o - n * k;
    tmp[3] = i * p - m * l;
    tmp[4] = i * o - m * k;
    tmp[5] = i * n - m * j;

    result.m[0][0] = f * tmp[0] - g * tmp[1] + h * tmp[2];
    result.m[1][0] = -(e * tmp[0] - g * tmp[3] + h * tmp[4]);
    result.m[2][0] = e * tmp[1] - f * tmp[3] + h * tmp[5];
    result.m[3][0] = -(e * tmp[2] - f * tmp[4] + g * tmp[5]);

    result.m[0][1] = -(b * tmp[0] - c * tmp[1] + d * tmp[2]);
    result.m[1][1] = a * tmp[0] - c * tmp[3] + d * tmp[4];
    result.m[2][1] = -(a * tmp[1] - b * tmp[3] + d * tmp[5]);
    result.m[3][1] = a * tmp[2] - b * tmp[4] + c * tmp[5];

    tmp[0] = g * p - o * h;
    tmp[1] = f * p - n * h;
    tmp[2] = f * o - n * g;
    tmp[3] = e * p - m * h;
    tmp[4] = e * o - m * g;
    tmp[5] = e * n - m * f;

    result.m[0][2] = b * tmp[0] - c * tmp[1] + d * tmp[2];
    result.m[1][2] = -(a * tmp[0] - c * tmp[3] + d * tmp[4]);
    result.m[2][2] = a * tmp[1] - b * tmp[3] + d * tmp[5];
    result.m[3][2] = -(a * tmp[2] - b * tmp[4] + c * tmp[5]);

    tmp[0] = g * l - k * h;
    tmp[1] = f * l - j * h;
    tmp[2] = f * k - j * g;
    tmp[3] = e * l - i * h;
    tmp[4] = e * k - i * g;
    tmp[5] = e * j - i * f;

    result.m[0][3] = -(b * tmp[0] - c * tmp[1] + d * tmp[2]);
    result.m[1][3] = a * tmp[0] - c * tmp[3] + d * tmp[4];
    result.m[2][3] = -(a * tmp[1] - b * tmp[3] + d * tmp[5]);
    result.m[3][3] = a * tmp[2] - b * tmp[4] + c * tmp[5];

    det = 1.0f / (a * result.m[0][0] + b * result.m[1][0] + c * result.m[2][0] + d * result.m[3][0]);

#if NEKO_MATH_USE_SSE

    __m128 scale = _mm_set1_ps(det);
    result.packed[0] = _mm_mul_ps(result.packed[0], scale);
    result.packed[1] = _mm_mul_ps(result.packed[1], scale);
    result.packed[2] = _mm_mul_ps(result.packed[2], scale);
    result.packed[3] = _mm_mul_ps(result.packed[3], scale);

#else

    result.m[0][0] = result.m[0][0] * det;
    result.m[0][1] = result.m[0][1] * det;
    result.m[0][2] = result.m[0][2] * det;
    result.m[0][3] = result.m[0][3] * det;
    result.m[1][0] = result.m[1][0] * det;
    result.m[1][1] = result.m[1][1] * det;
    result.m[1][2] = result.m[1][2] * det;
    result.m[1][3] = result.m[1][3] * det;
    result.m[2][0] = result.m[2][0] * det;
    result.m[2][1] = result.m[2][1] * det;
    result.m[2][2] = result.m[2][2] * det;
    result.m[2][3] = result.m[2][3] * det;
    result.m[3][0] = result.m[3][0] * det;
    result.m[3][1] = result.m[3][1] * det;
    result.m[3][2] = result.m[3][2] * det;
    result.m[3][3] = result.m[3][3] * det;

#endif

    return result;
}

// translation:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_translate)(neko_fast_vec2 t) {
    neko_fast_mat3 result = NEKO_MATH_PREFIX(mat3_identity)();

    result.m[2][0] = t.x;
    result.m[2][1] = t.y;

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_translate)(neko_fast_vec3 t) {
    neko_fast_mat4 result = NEKO_MATH_PREFIX(mat4_identity)();

    result.m[3][0] = t.x;
    result.m[3][1] = t.y;
    result.m[3][2] = t.z;

    return result;
}

// scaling:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_scale)(neko_fast_vec2 s) {
    neko_fast_mat3 result = NEKO_MATH_PREFIX(mat3_identity)();

    result.m[0][0] = s.x;
    result.m[1][1] = s.y;

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_scale)(neko_fast_vec3 s) {
    neko_fast_mat4 result = NEKO_MATH_PREFIX(mat4_identity)();

    result.m[0][0] = s.x;
    result.m[1][1] = s.y;
    result.m[2][2] = s.z;

    return result;
}

// rotation:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat3_rotate)(float angle) {
    neko_fast_mat3 result = NEKO_MATH_PREFIX(mat3_identity)();

    float radians = NEKO_MATH_PREFIX(deg_to_rad)(angle);
    float sine = NEKO_MATH_SINF(radians);
    float cosine = NEKO_MATH_COSF(radians);

    result.m[0][0] = cosine;
    result.m[1][0] = sine;
    result.m[0][1] = -sine;
    result.m[1][1] = cosine;

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_rotate)(neko_fast_vec3 axis, float angle) {
    neko_fast_mat4 result = NEKO_MATH_PREFIX(mat4_identity)();

    axis = NEKO_MATH_PREFIX(vec3_normalize)(axis);

    float radians = NEKO_MATH_PREFIX(deg_to_rad)(angle);
    float sine = NEKO_MATH_SINF(radians);
    float cosine = NEKO_MATH_COSF(radians);
    float cosine2 = 1.0f - cosine;

    result.m[0][0] = axis.x * axis.x * cosine2 + cosine;
    result.m[0][1] = axis.x * axis.y * cosine2 + axis.z * sine;
    result.m[0][2] = axis.x * axis.z * cosine2 - axis.y * sine;
    result.m[1][0] = axis.y * axis.x * cosine2 - axis.z * sine;
    result.m[1][1] = axis.y * axis.y * cosine2 + cosine;
    result.m[1][2] = axis.y * axis.z * cosine2 + axis.x * sine;
    result.m[2][0] = axis.z * axis.x * cosine2 + axis.y * sine;
    result.m[2][1] = axis.z * axis.y * cosine2 - axis.x * sine;
    result.m[2][2] = axis.z * axis.z * cosine2 + cosine;

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_rotate_euler)(neko_fast_vec3 angles) {
    neko_fast_mat4 result = NEKO_MATH_PREFIX(mat4_identity)();

    neko_fast_vec3 radians;
    radians.x = NEKO_MATH_PREFIX(deg_to_rad)(angles.x);
    radians.y = NEKO_MATH_PREFIX(deg_to_rad)(angles.y);
    radians.z = NEKO_MATH_PREFIX(deg_to_rad)(angles.z);

    float sinX = NEKO_MATH_SINF(radians.x);
    float cosX = NEKO_MATH_COSF(radians.x);
    float sinY = NEKO_MATH_SINF(radians.y);
    float cosY = NEKO_MATH_COSF(radians.y);
    float sinZ = NEKO_MATH_SINF(radians.z);
    float cosZ = NEKO_MATH_COSF(radians.z);

    result.m[0][0] = cosY * cosZ;
    result.m[0][1] = cosY * sinZ;
    result.m[0][2] = -sinY;
    result.m[1][0] = sinX * sinY * cosZ - cosX * sinZ;
    result.m[1][1] = sinX * sinY * sinZ + cosX * cosZ;
    result.m[1][2] = sinX * cosY;
    result.m[2][0] = cosX * sinY * cosZ + sinX * sinZ;
    result.m[2][1] = cosX * sinY * sinZ - sinX * cosZ;
    result.m[2][2] = cosX * cosY;

    return result;
}

// to mat3:

neko_static_inline neko_fast_mat3 NEKO_MATH_PREFIX(mat4_top_left)(neko_fast_mat4 m) {
    neko_fast_mat3 result;

    result.m[0][0] = m.m[0][0];
    result.m[0][1] = m.m[0][1];
    result.m[0][2] = m.m[0][2];
    result.m[1][0] = m.m[1][0];
    result.m[1][1] = m.m[1][1];
    result.m[1][2] = m.m[1][2];
    result.m[2][0] = m.m[2][0];
    result.m[2][1] = m.m[2][1];
    result.m[2][2] = m.m[2][2];

    return result;
}

// projection:

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_perspective)(float fov, float aspect, float n, float f) {
    neko_fast_mat4 result = {0};

    float scale = NEKO_MATH_TANF(NEKO_MATH_PREFIX(deg_to_rad)(fov * 0.5f)) * n;

    float right = aspect * scale;
    float left = -right;
    float top = scale;
    float bot = -top;

    result.m[0][0] = n / right;
    result.m[1][1] = n / top;
    result.m[2][2] = -(f + n) / (f - n);
    result.m[3][2] = -2.0f * f * n / (f - n);
    result.m[2][3] = -1.0f;

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_orthographic)(float left, float right, float bot, float top, float n, float f) {
    neko_fast_mat4 result = NEKO_MATH_PREFIX(mat4_identity)();

    result.m[0][0] = 2.0f / (right - left);
    result.m[1][1] = 2.0f / (top - bot);
    result.m[2][2] = 2.0f / (n - f);

    result.m[3][0] = (left + right) / (left - right);
    result.m[3][1] = (bot + top) / (bot - top);
    result.m[3][2] = (n + f) / (n - f);

    return result;
}

// view matrix:

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_look)(neko_fast_vec3 pos, neko_fast_vec3 dir, neko_fast_vec3 up) {
    neko_fast_mat4 result;

    neko_fast_vec3 r = NEKO_MATH_PREFIX(vec3_normalize)(NEKO_MATH_PREFIX(vec3_cross)(up, dir));
    neko_fast_vec3 u = NEKO_MATH_PREFIX(vec3_cross)(dir, r);

    neko_fast_mat4 RUD = NEKO_MATH_PREFIX(mat4_identity)();
    RUD.m[0][0] = r.x;
    RUD.m[1][0] = r.y;
    RUD.m[2][0] = r.z;
    RUD.m[0][1] = u.x;
    RUD.m[1][1] = u.y;
    RUD.m[2][1] = u.z;
    RUD.m[0][2] = dir.x;
    RUD.m[1][2] = dir.y;
    RUD.m[2][2] = dir.z;

    neko_fast_vec3 oppPos = {-pos.x, -pos.y, -pos.z};
    result = NEKO_MATH_PREFIX(mat4_mult)(RUD, NEKO_MATH_PREFIX(mat4_translate)(oppPos));

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(mat4_lookat)(neko_fast_vec3 pos, neko_fast_vec3 target, neko_fast_vec3 up) {
    neko_fast_mat4 result;

    neko_fast_vec3 dir = NEKO_MATH_PREFIX(vec3_normalize)(NEKO_MATH_PREFIX(vec3_sub)(pos, target));
    result = NEKO_MATH_PREFIX(mat4_look)(pos, dir, up);

    return result;
}

//----------------------------------------------------------------------//
// QUATERNION FUNCTIONS:

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_identity)() {
    neko_fast_quaternion result;

    result.x = 0.0f;
    result.y = 0.0f;
    result.z = 0.0f;
    result.w = 1.0f;

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_add)(neko_fast_quaternion q1, neko_fast_quaternion q2) {
    neko_fast_quaternion result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_add_ps(q1.packed, q2.packed);

#else

    result.x = q1.x + q2.x;
    result.y = q1.y + q2.y;
    result.z = q1.z + q2.z;
    result.w = q1.w + q2.w;

#endif

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_sub)(neko_fast_quaternion q1, neko_fast_quaternion q2) {
    neko_fast_quaternion result;

#if NEKO_MATH_USE_SSE

    result.packed = _mm_sub_ps(q1.packed, q2.packed);

#else

    result.x = q1.x - q2.x;
    result.y = q1.y - q2.y;
    result.z = q1.z - q2.z;
    result.w = q1.w - q2.w;

#endif

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_mult)(neko_fast_quaternion q1, neko_fast_quaternion q2) {
    neko_fast_quaternion result;

#if NEKO_MATH_USE_SSE

    __m128 temp1;
    __m128 temp2;

    temp1 = _mm_shuffle_ps(q1.packed, q1.packed, _MM_SHUFFLE(3, 3, 3, 3));
    temp2 = q2.packed;
    result.packed = _mm_mul_ps(temp1, temp2);

    temp1 = _mm_xor_ps(_mm_shuffle_ps(q1.packed, q1.packed, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.0f, -0.0f, 0.0f, -0.0f));
    temp2 = _mm_shuffle_ps(q2.packed, q2.packed, _MM_SHUFFLE(0, 1, 2, 3));
    result.packed = _mm_add_ps(result.packed, _mm_mul_ps(temp1, temp2));

    temp1 = _mm_xor_ps(_mm_shuffle_ps(q1.packed, q1.packed, _MM_SHUFFLE(1, 1, 1, 1)), _mm_setr_ps(0.0f, 0.0f, -0.0f, -0.0f));
    temp2 = _mm_shuffle_ps(q2.packed, q2.packed, _MM_SHUFFLE(1, 0, 3, 2));
    result.packed = _mm_add_ps(result.packed, _mm_mul_ps(temp1, temp2));

    temp1 = _mm_xor_ps(_mm_shuffle_ps(q1.packed, q1.packed, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.0f, 0.0f, 0.0f, -0.0f));
    temp2 = _mm_shuffle_ps(q2.packed, q2.packed, _MM_SHUFFLE(2, 3, 0, 1));
    result.packed = _mm_add_ps(result.packed, _mm_mul_ps(temp1, temp2));

#else

    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;

#endif

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_scale)(neko_fast_quaternion q, float s) {
    neko_fast_quaternion result;

#if NEKO_MATH_USE_SSE

    __m128 scale = _mm_set1_ps(s);
    result.packed = _mm_mul_ps(q.packed, scale);

#else

    result.x = q.x * s;
    result.y = q.y * s;
    result.z = q.z * s;
    result.w = q.w * s;

#endif

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(quaternion_dot)(neko_fast_quaternion q1, neko_fast_quaternion q2) {
    float result;

#if NEKO_MATH_USE_SSE

    __m128 r = _mm_mul_ps(q1.packed, q2.packed);
    r = _mm_hadd_ps(r, r);
    r = _mm_hadd_ps(r, r);
    _mm_store_ss(&result, r);

#else

    result = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

#endif

    return result;
}

neko_static_inline float NEKO_MATH_PREFIX(quaternion_length)(neko_fast_quaternion q) {
    float result;

    result = NEKO_MATH_SQRTF(NEKO_MATH_PREFIX(quaternion_dot)(q, q));

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_normalize)(neko_fast_quaternion q) {
    neko_fast_quaternion result = {0};

    float len = NEKO_MATH_PREFIX(quaternion_length)(q);
    if (len != 0.0f) {
#if NEKO_MATH_USE_SSE

        __m128 scale = _mm_set1_ps(len);
        result.packed = _mm_div_ps(q.packed, scale);

#else

        float invLen = 1.0f / len;

        result.x = q.x * invLen;
        result.y = q.y * invLen;
        result.z = q.z * invLen;
        result.w = q.w * invLen;

#endif
    }

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_conjugate)(neko_fast_quaternion q) {
    neko_fast_quaternion result;

    result.x = -q.x;
    result.y = -q.y;
    result.z = -q.z;
    result.w = q.w;

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_inv)(neko_fast_quaternion q) {
    neko_fast_quaternion result;

    result.x = -q.x;
    result.y = -q.y;
    result.z = -q.z;
    result.w = q.w;

#if NEKO_MATH_USE_SSE

    __m128 scale = _mm_set1_ps(NEKO_MATH_PREFIX(quaternion_dot)(q, q));
    _mm_div_ps(result.packed, scale);

#else

    float invLen2 = 1.0f / NEKO_MATH_PREFIX(quaternion_dot)(q, q);

    result.x *= invLen2;
    result.y *= invLen2;
    result.z *= invLen2;
    result.w *= invLen2;

#endif

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_slerp)(neko_fast_quaternion q1, neko_fast_quaternion q2, float a) {
    neko_fast_quaternion result;

    float cosine = NEKO_MATH_PREFIX(quaternion_dot)(q1, q2);
    float angle = NEKO_MATH_ACOSF(cosine);

    float sine1 = NEKO_MATH_SINF((1.0f - a) * angle);
    float sine2 = NEKO_MATH_SINF(a * angle);
    float invSine = 1.0f / NEKO_MATH_SINF(angle);

    q1 = NEKO_MATH_PREFIX(quaternion_scale)(q1, sine1);
    q2 = NEKO_MATH_PREFIX(quaternion_scale)(q2, sine2);

    result = NEKO_MATH_PREFIX(quaternion_add)(q1, q2);
    result = NEKO_MATH_PREFIX(quaternion_scale)(result, invSine);

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_from_axis_angle)(neko_fast_vec3 axis, float angle) {
    neko_fast_quaternion result;

    float radians = NEKO_MATH_PREFIX(deg_to_rad)(angle * 0.5f);
    axis = NEKO_MATH_PREFIX(vec3_normalize)(axis);
    float sine = NEKO_MATH_SINF(radians);

    result.x = axis.x * sine;
    result.y = axis.y * sine;
    result.z = axis.z * sine;
    result.w = NEKO_MATH_COSF(radians);

    return result;
}

neko_static_inline neko_fast_quaternion NEKO_MATH_PREFIX(quaternion_from_euler)(neko_fast_vec3 angles) {
    neko_fast_quaternion result;

    neko_fast_vec3 radians;
    radians.x = NEKO_MATH_PREFIX(deg_to_rad)(angles.x * 0.5f);
    radians.y = NEKO_MATH_PREFIX(deg_to_rad)(angles.y * 0.5f);
    radians.z = NEKO_MATH_PREFIX(deg_to_rad)(angles.z * 0.5f);

    float sinx = NEKO_MATH_SINF(radians.x);
    float cosx = NEKO_MATH_COSF(radians.x);
    float siny = NEKO_MATH_SINF(radians.y);
    float cosy = NEKO_MATH_COSF(radians.y);
    float sinz = NEKO_MATH_SINF(radians.z);
    float cosz = NEKO_MATH_COSF(radians.z);

#if NEKO_MATH_USE_SSE

    __m128 packedx = _mm_setr_ps(sinx, cosx, cosx, cosx);
    __m128 packedy = _mm_setr_ps(cosy, siny, cosy, cosy);
    __m128 packedz = _mm_setr_ps(cosz, cosz, sinz, cosz);

    result.packed = _mm_mul_ps(_mm_mul_ps(packedx, packedy), packedz);

    packedx = _mm_shuffle_ps(packedx, packedx, _MM_SHUFFLE(0, 0, 0, 1));
    packedy = _mm_shuffle_ps(packedy, packedy, _MM_SHUFFLE(1, 1, 0, 1));
    packedz = _mm_shuffle_ps(packedz, packedz, _MM_SHUFFLE(2, 0, 2, 2));

    result.packed = _mm_addsub_ps(result.packed, _mm_mul_ps(_mm_mul_ps(packedx, packedy), packedz));

#else

    result.x = sinx * cosy * cosz - cosx * siny * sinz;
    result.y = cosx * siny * cosz + sinx * cosy * sinz;
    result.z = cosx * cosy * sinz - sinx * siny * cosz;
    result.w = cosx * cosy * cosz + sinx * siny * sinz;

#endif

    return result;
}

neko_static_inline neko_fast_mat4 NEKO_MATH_PREFIX(quaternion_to_mat4)(neko_fast_quaternion q) {
    neko_fast_mat4 result = NEKO_MATH_PREFIX(mat4_identity)();

    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx2 = q.x * x2;
    float xy2 = q.x * y2;
    float xz2 = q.x * z2;
    float yy2 = q.y * y2;
    float yz2 = q.y * z2;
    float zz2 = q.z * z2;
    float sx2 = q.w * x2;
    float sy2 = q.w * y2;
    float sz2 = q.w * z2;

    result.m[0][0] = 1.0f - (yy2 + zz2);
    result.m[0][1] = xy2 - sz2;
    result.m[0][2] = xz2 + sy2;
    result.m[1][0] = xy2 + sz2;
    result.m[1][1] = 1.0f - (xx2 + zz2);
    result.m[1][2] = yz2 - sx2;
    result.m[2][0] = xz2 - sy2;
    result.m[2][1] = yz2 + sx2;
    result.m[2][2] = 1.0f - (xx2 + yy2);

    return result;
}

#endif  // NEKO_MATH_H
