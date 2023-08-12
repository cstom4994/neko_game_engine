#ifndef NEKO_MATH_H
#define NEKO_MATH_H

#include <math.h>

#include "engine/common/neko_util.h"

// Defines
#define neko_pi 3.14159265358979323846264f
#define neko_tau 2.0 * neko_pi

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

neko_static_inline b32 neko_vec2_equal(neko_vec2 a, neko_vec2 b) { return (a.x == b.x && a.y == b.y); }

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

neko_static_inline b32 neko_rect_vs_rect(neko_rect_t a, neko_rect_t b) {
    if (a.max.x > b.min.x && a.max.y > b.min.y && a.min.x < b.max.x && a.min.y < b.max.y) {
        return true;
    }

    return false;
}

neko_inline f32 neko_sign(f32 x) { return x < 0.0f ? -1.0f : 1.0f; }

#endif  // NEKO_MATH_H
