#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "engine/math.h"

/* ==== 2-D VECTOR ==== */

float neko_v2f_mag(neko_v2f_t v) { return sqrtf(v.x * v.x + v.y * v.y); }

neko_v2f_t neko_v2f_normalise(neko_v2f_t v) {
    const float l = neko_v2f_mag(v);

    if (l == 0.0) {
        return neko_v2f_t{0.0, 0.0};
    }

    return neko_v2f_t{v.x / l, v.y / l};
}

float neko_v2f_dist(neko_v2f_t a, neko_v2f_t b) {
    const float xd = a.x - b.x;
    const float yd = a.y - b.y;
    return sqrtf(xd * xd + yd * yd);
}

float neko_v2f_dot(neko_v2f_t a, neko_v2f_t b) { return a.x * b.x + a.y * b.y; }

i32 neko_v2i_mag(neko_v2i_t v) { return (i32)sqrt(v.x * v.x + v.y * v.y); }

neko_v2i_t neko_v2i_normalise(neko_v2i_t v) {
    i32 l = neko_v2i_mag(v);

    if (l == 0) {
        return neko_v2i_t{0, 0};
    }

    return neko_v2i_t{v.x / l, v.y / l};
}

i32 neko_v2i_dist(neko_v2i_t a, neko_v2i_t b) {
    const i32 xd = a.x - b.x;
    const i32 yd = a.y - b.y;
    return (i32)sqrt(xd * xd + yd * yd);
}

i32 neko_v2i_dot(neko_v2i_t a, neko_v2i_t b) { return a.x * b.x + a.y * b.y; }

u32 neko_v2u_mag(neko_v2u_t v) { return (u32)sqrt(v.x * v.x + v.y * v.y); }

neko_v2u_t neko_v2u_normalise(neko_v2u_t v) {
    u32 l = neko_v2u_mag(v);

    if (l == 0) {
        return neko_v2u_t{0, 0};
    }

    return neko_v2u_t{v.x / l, v.y / l};
}

u32 neko_v2u_dist(neko_v2u_t a, neko_v2u_t b) {
    const u32 xd = a.x - b.x;
    const u32 yd = a.y - b.y;
    return (u32)sqrt(xd * xd + yd * yd);
}

u32 neko_v2u_dot(neko_v2u_t a, neko_v2u_t b) { return a.x * b.x + a.y * b.y; }

/* ==== 3-D VECTOR ==== */

float neko_v3f_mag(neko_v3f_t v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

neko_v3f_t neko_v3f_normalise(neko_v3f_t v) {
    const float l = neko_v3f_mag(v);

    if (l == 0.0) {
        return neko_v3f_t{0.0, 0.0, 0.0};
    }

    return neko_v3f_t{v.x / l, v.y / l, v.z / l};
}

float neko_v3f_dist(neko_v3f_t a, neko_v3f_t b) {
    const float xd = a.x - b.x;
    const float yd = a.y - b.y;
    const float zd = a.z - b.z;

    return sqrtf(xd * xd + yd * yd + zd * zd);
}

float neko_v3f_dot(neko_v3f_t a, neko_v3f_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

neko_v3f_t neko_v3f_cross(neko_v3f_t a, neko_v3f_t b) { return neko_v3f_t{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }

i32 neko_v3i_mag(neko_v3i_t v) { return (i32)sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

neko_v3i_t neko_v3i_normalise(neko_v3i_t v) {
    const i32 l = neko_v3i_mag(v);

    if (l == 0) {
        return neko_v3i_t{0, 0, 0};
    }

    return neko_v3i_t{v.x / l, v.y / l, v.z / l};
}

i32 neko_v3i_dist(neko_v3i_t a, neko_v3i_t b) {
    const i32 xd = a.x - b.x;
    const i32 yd = a.y - b.y;
    const i32 zd = a.z - b.z;

    return (i32)sqrt(xd * xd + yd * yd + zd * zd);
}

i32 neko_v3i_dot(neko_v3i_t a, neko_v3i_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

u32 neko_v3u_mag(neko_v3u_t v) { return (i32)sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

neko_v3u_t neko_v3u_normalise(neko_v3u_t v) {
    const u32 l = neko_v3u_mag(v);

    if (l == 0) {
        return neko_v3u_t{0, 0, 0};
    }

    return neko_v3u_t{v.x / l, v.y / l, v.z / l};
}

u32 neko_v3u_dist(neko_v3u_t a, neko_v3u_t b) {
    const u32 xd = a.x - b.x;
    const u32 yd = a.y - b.y;
    const u32 zd = a.z - b.z;

    return (u32)sqrt(xd * xd + yd * yd + zd * zd);
}

u32 neko_v3u_dot(neko_v3u_t a, neko_v3u_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

/* ==== 4-D VECTOR ====*/
float neko_v4f_mag(neko_v4f_t v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

neko_v4f_t neko_v4f_normalise(neko_v4f_t v) {
    const float l = neko_v4f_mag(v);

    if (l == 0.0) {
        return neko_v4f_t{0.0, 0.0, 0.0, 0.0};
    }

    return neko_v4f_t{v.x / l, v.y / l, v.z / l, v.w / l};
}

float neko_v4f_dist(neko_v4f_t a, neko_v4f_t b) {
    const float xd = a.x - b.x;
    const float yd = a.y - b.y;
    const float zd = a.z - b.z;
    const float wd = a.w - b.w;

    return sqrtf(xd * xd + yd * yd + zd * zd + wd * wd);
}

float neko_v4f_dot(neko_v4f_t a, neko_v4f_t b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

i32 neko_v4i_mag(neko_v4i v) { return (i32)sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

neko_v4i neko_v4i_normalise(neko_v4i v) {
    const i32 l = neko_v4i_mag(v);

    if (l == 0) {
        return neko_v4i{0, 0, 0, 0};
    }

    return neko_v4i{v.x / l, v.y / l, v.z / l, v.w / l};
}

i32 neko_v4i_dist(neko_v4i a, neko_v4i b) {
    const i32 xd = a.x - b.x;
    const i32 yd = a.y - b.y;
    const i32 zd = a.z - b.z;
    const i32 wd = a.w - b.w;

    return (i32)sqrt(xd * xd + yd * yd + zd * zd + wd * wd);
}

i32 neko_v4i_dot(neko_v4i a, neko_v4i b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

u32 neko_v4u_mag(neko_v4u v) { return (u32)sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

neko_v4u neko_v4u_normalise(neko_v4u v) {
    const u32 l = neko_v4u_mag(v);

    if (l == 0) {
        return neko_v4u{0, 0, 0, 0};
    }

    return neko_v4u{v.x / l, v.y / l, v.z / l, v.w / l};
}

u32 neko_v4u_dist(neko_v4u a, neko_v4u b) {
    const u32 xd = a.x - b.x;
    const u32 yd = a.y - b.y;
    const u32 zd = a.z - b.z;
    const u32 wd = a.w - b.w;

    return (u32)sqrt(xd * xd + yd * yd + zd * zd + wd * wd);
}

u32 neko_v4u_dot(neko_v4u a, neko_v4u b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

/* ==== TO-DEGREES ==== */
float neko_todeg(float rad) { return (rad * (180.0f / neko_pi)); }

neko_v2f_t neko_todeg_v2f(neko_v2f_t rad) { return neko_v2f_t{neko_todeg(rad.x), neko_todeg(rad.y)}; }

neko_v3f_t neko_todeg_v3f(neko_v3f_t rad) { return neko_v3f_t{neko_todeg(rad.x), neko_todeg(rad.y), neko_todeg(rad.z)}; }

neko_v4f_t neko_todeg_v4f(neko_v4f_t rad) { return neko_v4f_t{neko_todeg(rad.x), neko_todeg(rad.y), neko_todeg(rad.z), neko_todeg(rad.y)}; }

/* ==== TO-RADIANS ==== */
float neko_torad(float deg) { return (deg * (neko_pi / 180.0f)); }

neko_v2f_t neko_torad_v2f(neko_v2f_t deg) { return neko_v2f_t{neko_torad(deg.x), neko_todeg(deg.y)}; }

neko_v3f_t neko_torad_v3f(neko_v3f_t deg) { return neko_v3f_t{neko_torad(deg.x), neko_torad(deg.y), neko_torad(deg.z)}; }

neko_v4f_t neko_torad_v4f(neko_v4f_t deg) { return neko_v4f_t{neko_torad(deg.x), neko_torad(deg.y), neko_torad(deg.z), neko_torad(deg.y)}; }

/* ==== FOUR-BY-FOUR MATRIX ==== */

neko_m4f_t neko_new_mf4(float diagonal) {
    neko_m4f_t result;

    for (u32 x = 0; x < 4; x++) {
        for (u32 y = 0; y < 4; y++) {
            result.elements[x][y] = 0.0;
        }
    }

    result.elements[0][0] = diagonal;
    result.elements[1][1] = diagonal;
    result.elements[2][2] = diagonal;
    result.elements[3][3] = diagonal;

    return result;
}

neko_m4f_t neko_m4f_identity() { return neko_new_mf4(1.0); }

neko_m4f_t neko_m4f_multiply(neko_m4f_t a, neko_m4f_t b) {
    neko_m4f_t result = neko_m4f_identity();

    for (u32 row = 0; row < 4; row++) {
        for (u32 col = 0; col < 4; col++) {
            float sum = 0.0;

            for (u32 e = 0; e < 4; e++) {
                sum += a.elements[e][row] * b.elements[col][e];
            }

            result.elements[col][row] = sum;
        }
    }

    return result;
}

neko_m4f_t neko_m4f_translate(neko_m4f_t m, neko_v3f_t v) {
    neko_m4f_t result = neko_m4f_identity();

    result.elements[3][0] = v.x;
    result.elements[3][1] = v.y;
    result.elements[3][2] = v.z;

    return neko_m4f_multiply(m, result);
}

neko_m4f_t neko_m4f_rotate(neko_m4f_t m, float a, neko_v3f_t v) {
    neko_m4f_t result = neko_m4f_identity();

    const float r = neko_torad(a);
    const float c = cosf(r);
    const float s = sinf(r);

    const float omc = 1.0f - c;

    float x = v.x;
    float y = v.y;
    float z = v.z;

    result.elements[0][0] = x * x * omc + c;
    result.elements[0][1] = y * x * omc + z * s;
    result.elements[0][2] = x * z * omc - y * s;

    result.elements[1][0] = x * y * omc - z * s;
    result.elements[1][1] = y * y * omc + c;
    result.elements[1][2] = y * z * omc + x * s;

    result.elements[2][0] = x * z * omc + y * s;
    result.elements[2][1] = y * z * omc - x * s;
    result.elements[2][2] = z * z * omc + c;

    return neko_m4f_multiply(m, result);
}

neko_m4f_t neko_m4f_scale(neko_m4f_t m, neko_v3f_t v) {
    neko_m4f_t result = neko_m4f_identity();

    result.elements[0][0] = v.x;
    result.elements[1][1] = v.y;
    result.elements[2][2] = v.z;

    return neko_m4f_multiply(m, result);
}

neko_m4f_t neko_m4f_ortho(float left, float right, float bottom, float top, float near, float far) {
    neko_m4f_t result = neko_m4f_identity();

    result.elements[0][0] = 2.0f / (right - left);
    result.elements[1][1] = 2.0f / (top - bottom);
    result.elements[2][2] = 2.0f / (near - far);

    result.elements[3][0] = (left + right) / (left - right);
    result.elements[3][1] = (bottom + top) / (bottom - top);
    result.elements[3][2] = (far + near) / (far - near);

    return result;
}

neko_m4f_t neko_m4f_persp(float fov, float aspect, float near, float far) {
    neko_m4f_t result = neko_m4f_identity();

    const float q = 1.0f / tanf(neko_torad(0.5f * fov));
    const float a = q / aspect;

    const float b = (near + far) / (near - far);
    const float c = (2.0f * near * far) / (near - far);

    result.elements[0][0] = a;
    result.elements[1][1] = q;
    result.elements[2][2] = b;
    result.elements[2][3] = -1.0;
    result.elements[3][2] = c;

    return result;
}

neko_m4f_t neko_m4f_lookat(neko_v3f_t camera, neko_v3f_t object, neko_v3f_t up) {
    neko_m4f_t result = neko_m4f_identity();

    neko_v3f_t f = neko_v3f_normalise(neko_v3f_t{object.x - camera.x, object.y - camera.y, object.z - camera.z});

    neko_v3f_t u = neko_v3f_normalise(up);
    neko_v3f_t s = neko_v3f_normalise(neko_v3f_cross(f, u));
    u = neko_v3f_cross(s, f);

    result.elements[0][0] = s.x;
    result.elements[1][0] = s.y;
    result.elements[2][0] = s.z;
    result.elements[0][1] = u.x;
    result.elements[1][1] = u.y;
    result.elements[2][1] = u.z;
    result.elements[0][2] = -f.x;
    result.elements[1][2] = -f.y;
    result.elements[2][2] = -f.z;

    result.elements[3][0] = -neko_v3f_dot(s, camera);
    result.elements[3][1] = -neko_v3f_dot(u, camera);
    result.elements[3][2] = neko_v3f_dot(f, camera);

    return result;
}

void neko_m4f_decompose(neko_m4f_t matrix, neko_v3f_t* translation, neko_v3f_t* rotation, neko_v3f_t* scale) {
    assert(translation && rotation && scale);

    translation->x = matrix.elements[3][0];
    translation->y = matrix.elements[3][1];
    translation->z = matrix.elements[3][2];

    scale->x = matrix.elements[0][0];
    scale->y = matrix.elements[1][1];
    scale->z = matrix.elements[2][2];

    rotation->x = (180.0f / neko_pi) * atan2f(matrix.elements[1][2], matrix.elements[2][2]);
    rotation->y = (180.0f / neko_pi) * atan2f(-matrix.elements[0][2], sqrtf(matrix.elements[1][2] * matrix.elements[1][2] + matrix.elements[2][2] * matrix.elements[2][2]));
    rotation->z = (180.0f / neko_pi) * atan2f(matrix.elements[0][1], matrix.elements[0][0]);
}

neko_m4f_t neko_m4f_inverse(neko_m4f_t matrix) {
    const float determinant = matrix.elements[0][0] * matrix.elements[1][1] * matrix.elements[2][2] * matrix.elements[3][3] -
                              matrix.elements[0][0] * matrix.elements[1][1] * matrix.elements[2][3] * matrix.elements[3][2] +
                              matrix.elements[0][0] * matrix.elements[1][2] * matrix.elements[2][3] * matrix.elements[3][1] -
                              matrix.elements[0][0] * matrix.elements[1][2] * matrix.elements[2][1] * matrix.elements[3][3] +
                              matrix.elements[0][0] * matrix.elements[1][3] * matrix.elements[2][1] * matrix.elements[3][2] -
                              matrix.elements[0][0] * matrix.elements[1][3] * matrix.elements[2][2] * matrix.elements[3][1] -
                              matrix.elements[0][1] * matrix.elements[1][2] * matrix.elements[2][3] * matrix.elements[3][0] +
                              matrix.elements[0][1] * matrix.elements[1][2] * matrix.elements[2][0] * matrix.elements[3][3] -
                              matrix.elements[0][1] * matrix.elements[1][3] * matrix.elements[2][0] * matrix.elements[3][2] +
                              matrix.elements[0][1] * matrix.elements[1][3] * matrix.elements[2][2] * matrix.elements[3][0] -
                              matrix.elements[0][1] * matrix.elements[1][0] * matrix.elements[2][2] * matrix.elements[3][3] +
                              matrix.elements[0][1] * matrix.elements[1][0] * matrix.elements[2][3] * matrix.elements[3][2] +
                              matrix.elements[0][2] * matrix.elements[1][3] * matrix.elements[2][0] * matrix.elements[3][1] -
                              matrix.elements[0][2] * matrix.elements[1][3] * matrix.elements[2][1] * matrix.elements[3][0] +
                              matrix.elements[0][2] * matrix.elements[1][0] * matrix.elements[2][1] * matrix.elements[3][3] -
                              matrix.elements[0][2] * matrix.elements[1][0] * matrix.elements[2][3] * matrix.elements[3][1] +
                              matrix.elements[0][2] * matrix.elements[1][1] * matrix.elements[2][3] * matrix.elements[3][0] -
                              matrix.elements[0][2] * matrix.elements[1][1] * matrix.elements[2][0] * matrix.elements[3][3] -
                              matrix.elements[0][3] * matrix.elements[1][0] * matrix.elements[2][1] * matrix.elements[3][2] +
                              matrix.elements[0][3] * matrix.elements[1][0] * matrix.elements[2][2] * matrix.elements[3][1] -
                              matrix.elements[0][3] * matrix.elements[1][1] * matrix.elements[2][2] * matrix.elements[3][0] +
                              matrix.elements[0][3] * matrix.elements[1][1] * matrix.elements[2][0] * matrix.elements[3][2] -
                              matrix.elements[0][3] * matrix.elements[1][2] * matrix.elements[2][0] * matrix.elements[3][1] +
                              matrix.elements[0][3] * matrix.elements[1][2] * matrix.elements[2][1] * matrix.elements[3][0];

    if (determinant == 0.0f) {
        console_log("Cannot invert matrix");

        abort();
    }

    const float invdet = 1.0f / determinant;

    neko_m4f_t result;

    result.elements[0][0] = invdet * (matrix.elements[1][1] * (matrix.elements[2][2] * matrix.elements[3][3] - matrix.elements[2][3] * matrix.elements[3][2]) +
                                      matrix.elements[1][2] * (matrix.elements[2][3] * matrix.elements[3][1] - matrix.elements[2][1] * matrix.elements[3][3]) +
                                      matrix.elements[1][3] * (matrix.elements[2][1] * matrix.elements[3][2] - matrix.elements[2][2] * matrix.elements[3][1]));
    result.elements[0][1] = -invdet * (matrix.elements[0][1] * (matrix.elements[2][2] * matrix.elements[3][3] - matrix.elements[2][3] * matrix.elements[3][2]) +
                                       matrix.elements[0][2] * (matrix.elements[2][3] * matrix.elements[3][1] - matrix.elements[2][1] * matrix.elements[3][3]) +
                                       matrix.elements[0][3] * (matrix.elements[2][1] * matrix.elements[3][2] - matrix.elements[2][2] * matrix.elements[3][1]));
    result.elements[0][2] = invdet * (matrix.elements[0][1] * (matrix.elements[1][2] * matrix.elements[3][3] - matrix.elements[1][3] * matrix.elements[3][2]) +
                                      matrix.elements[0][2] * (matrix.elements[1][3] * matrix.elements[3][1] - matrix.elements[1][1] * matrix.elements[3][3]) +
                                      matrix.elements[0][3] * (matrix.elements[1][1] * matrix.elements[3][2] - matrix.elements[1][2] * matrix.elements[3][1]));
    result.elements[0][3] = -invdet * (matrix.elements[0][1] * (matrix.elements[1][2] * matrix.elements[2][3] - matrix.elements[1][3] * matrix.elements[2][2]) +
                                       matrix.elements[0][2] * (matrix.elements[1][3] * matrix.elements[2][1] - matrix.elements[1][1] * matrix.elements[2][3]) +
                                       matrix.elements[0][3] * (matrix.elements[1][1] * matrix.elements[2][2] - matrix.elements[1][2] * matrix.elements[2][1]));
    result.elements[1][0] = -invdet * (matrix.elements[1][0] * (matrix.elements[2][2] * matrix.elements[3][3] - matrix.elements[2][3] * matrix.elements[3][2]) +
                                       matrix.elements[1][2] * (matrix.elements[2][3] * matrix.elements[3][0] - matrix.elements[2][0] * matrix.elements[3][3]) +
                                       matrix.elements[1][3] * (matrix.elements[2][0] * matrix.elements[3][2] - matrix.elements[2][2] * matrix.elements[3][0]));
    result.elements[1][1] = invdet * (matrix.elements[0][0] * (matrix.elements[2][2] * matrix.elements[3][3] - matrix.elements[2][3] * matrix.elements[3][2]) +
                                      matrix.elements[0][2] * (matrix.elements[2][3] * matrix.elements[3][0] - matrix.elements[2][0] * matrix.elements[3][3]) +
                                      matrix.elements[0][3] * (matrix.elements[2][0] * matrix.elements[3][2] - matrix.elements[2][2] * matrix.elements[3][0]));
    result.elements[1][2] = -invdet * (matrix.elements[0][0] * (matrix.elements[1][2] * matrix.elements[3][3] - matrix.elements[1][3] * matrix.elements[3][2]) +
                                       matrix.elements[0][2] * (matrix.elements[1][3] * matrix.elements[3][0] - matrix.elements[1][0] * matrix.elements[3][3]) +
                                       matrix.elements[0][3] * (matrix.elements[1][0] * matrix.elements[3][2] - matrix.elements[1][2] * matrix.elements[3][0]));
    result.elements[1][3] = invdet * (matrix.elements[0][0] * (matrix.elements[1][2] * matrix.elements[2][3] - matrix.elements[1][3] * matrix.elements[2][2]) +
                                      matrix.elements[0][2] * (matrix.elements[1][3] * matrix.elements[2][0] - matrix.elements[1][0] * matrix.elements[2][3]) +
                                      matrix.elements[0][3] * (matrix.elements[1][0] * matrix.elements[2][2] - matrix.elements[1][2] * matrix.elements[2][0]));
    result.elements[2][0] = invdet * (matrix.elements[1][0] * (matrix.elements[2][1] * matrix.elements[3][3] - matrix.elements[2][3] * matrix.elements[3][1]) +
                                      matrix.elements[1][1] * (matrix.elements[2][3] * matrix.elements[3][0] - matrix.elements[2][0] * matrix.elements[3][3]) +
                                      matrix.elements[1][3] * (matrix.elements[2][0] * matrix.elements[3][1] - matrix.elements[2][1] * matrix.elements[3][0]));
    result.elements[2][1] = -invdet * (matrix.elements[0][0] * (matrix.elements[2][1] * matrix.elements[3][3] - matrix.elements[2][3] * matrix.elements[3][1]) +
                                       matrix.elements[0][1] * (matrix.elements[2][3] * matrix.elements[3][0] - matrix.elements[2][0] * matrix.elements[3][3]) +
                                       matrix.elements[0][3] * (matrix.elements[2][0] * matrix.elements[3][1] - matrix.elements[2][1] * matrix.elements[3][0]));
    result.elements[2][2] = invdet * (matrix.elements[0][0] * (matrix.elements[1][1] * matrix.elements[3][3] - matrix.elements[1][3] * matrix.elements[3][1]) +
                                      matrix.elements[0][1] * (matrix.elements[1][3] * matrix.elements[3][0] - matrix.elements[1][0] * matrix.elements[3][3]) +
                                      matrix.elements[0][3] * (matrix.elements[1][0] * matrix.elements[3][1] - matrix.elements[1][1] * matrix.elements[3][0]));
    result.elements[2][3] = -invdet * (matrix.elements[0][0] * (matrix.elements[1][1] * matrix.elements[2][3] - matrix.elements[1][3] * matrix.elements[2][1]) +
                                       matrix.elements[0][1] * (matrix.elements[1][3] * matrix.elements[2][0] - matrix.elements[1][0] * matrix.elements[2][3]) +
                                       matrix.elements[0][3] * (matrix.elements[1][0] * matrix.elements[2][1] - matrix.elements[1][1] * matrix.elements[2][0]));
    result.elements[3][0] = -invdet * (matrix.elements[1][0] * (matrix.elements[2][1] * matrix.elements[3][2] - matrix.elements[2][2] * matrix.elements[3][1]) +
                                       matrix.elements[1][1] * (matrix.elements[2][2] * matrix.elements[3][0] - matrix.elements[2][0] * matrix.elements[3][2]) +
                                       matrix.elements[1][2] * (matrix.elements[2][0] * matrix.elements[3][1] - matrix.elements[2][1] * matrix.elements[3][0]));
    result.elements[3][1] = invdet * (matrix.elements[0][0] * (matrix.elements[2][1] * matrix.elements[3][2] - matrix.elements[2][2] * matrix.elements[3][1]) +
                                      matrix.elements[0][1] * (matrix.elements[2][2] * matrix.elements[3][0] - matrix.elements[2][0] * matrix.elements[3][2]) +
                                      matrix.elements[0][2] * (matrix.elements[2][0] * matrix.elements[3][1] - matrix.elements[2][1] * matrix.elements[3][0]));
    result.elements[3][2] = -invdet * (matrix.elements[0][0] * (matrix.elements[1][1] * matrix.elements[3][2] - matrix.elements[1][2] * matrix.elements[3][1]) +
                                       matrix.elements[0][1] * (matrix.elements[1][2] * matrix.elements[3][0] - matrix.elements[1][0] * matrix.elements[3][2]) +
                                       matrix.elements[0][2] * (matrix.elements[1][0] * matrix.elements[3][1] - matrix.elements[1][1] * matrix.elements[3][0]));
    result.elements[3][3] = invdet * (matrix.elements[0][0] * (matrix.elements[1][1] * matrix.elements[2][2] - matrix.elements[1][2] * matrix.elements[2][1]) +
                                      matrix.elements[0][1] * (matrix.elements[1][2] * matrix.elements[2][0] - matrix.elements[1][0] * matrix.elements[2][2]) +
                                      matrix.elements[0][2] * (matrix.elements[1][0] * matrix.elements[2][1] - matrix.elements[1][1] * matrix.elements[2][0]));

    return result;
}

neko_v3f_t neko_v3f_transform(neko_v3f_t v, neko_m4f_t m) {
    return neko_v3f_t{
            .x = m.elements[0][0] * v.x + m.elements[0][1] * v.y + m.elements[0][2] * v.z + m.elements[0][3],
            .y = m.elements[1][0] * v.x + m.elements[1][1] * v.y + m.elements[1][2] * v.z + m.elements[1][3],
            .z = m.elements[2][0] * v.x + m.elements[2][1] * v.y + m.elements[2][2] * v.z + m.elements[2][3],
    };
}

neko_v4f_t neko_v4f_transform(neko_v4f_t v, neko_m4f_t m) {
    return neko_v4f_t{
            .x = m.elements[0][0] * v.x + m.elements[0][1] * v.y + m.elements[0][2] * v.z + m.elements[0][3] * v.w,
            .y = m.elements[1][0] * v.x + m.elements[1][1] * v.y + m.elements[1][2] * v.z + m.elements[1][3] * v.w,
            .z = m.elements[2][0] * v.x + m.elements[2][1] * v.y + m.elements[2][2] * v.z + m.elements[2][3] * v.w,
            .w = m.elements[3][0] * v.x + m.elements[3][1] * v.y + m.elements[3][2] * v.z + m.elements[3][3] * v.w,
    };
}
