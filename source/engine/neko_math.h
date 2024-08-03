
#ifndef NEKO_MATH_H
#define NEKO_MATH_H

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "neko_prelude.h"

#define neko_pi 3.14159265358979323846264f

#if 0

struct vec2 {
    vec2() {}
    vec2(f32 x, f32 y) {
        this->x = x;
        this->y = y;
    }
    f32 x;
    f32 y;
};

inline vec2 operator-(vec2 a) { return vec2(-a.x, -a.y); }
inline vec2 operator+(vec2 a, vec2 b) { return vec2(a.x + b.x, a.y + b.y); }
inline vec2 operator+=(vec2& a, vec2 b) {
    a = vec2(a.x + b.x, a.y + b.y);
    return a;
}
inline vec2 operator-(vec2 a, vec2 b) { return vec2(a.x - b.x, a.y - b.y); }
inline vec2 operator-=(vec2& a, vec2 b) {
    a = vec2(a.x - b.x, a.y - b.y);
    return a;
}
inline vec2 operator*(vec2 a, f32 b) { return vec2(a.x * b, a.y * b); }
inline vec2 operator*=(vec2& a, f32 b) {
    a = vec2(a.x * b, a.y * b);
    return a;
}
inline vec2 operator/(vec2 a, f32 b) { return vec2(a.x / b, a.y / b); }
inline vec2 operator/=(vec2& a, f32 b) {
    a = vec2(a.x / b, a.y / b);
    return a;
}

inline bool operator==(vec2 a, vec2 b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(vec2 a, vec2 b) { return !(a == b); }

inline f32 dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
inline f32 det2(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }
inline f32 len(vec2 v) { return sqrtf(dot(v, v)); }
inline f32 len_squared(vec2 v) { return dot(v, v); }
inline vec2 norm(vec2 v) {
    f32 l = len(v);
    return v * (1.0f / l);
}
inline vec2 safe_norm(vec2 v) {
    f32 l = len(v);
    return l == 0 ? vec2(0, 0) : v * (1.0f / l);
}
inline vec2 skew(vec2 v) { return vec2(-v.y, v.x); }

inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }
inline f32 min(f32 a, f32 b) { return a < b ? a : b; }
inline f32 max(f32 a, f32 b) { return a > b ? a : b; }
inline vec2 min(vec2 a, vec2 b) { return vec2(min(a.x, b.y), min(a.y, b.y)); }
inline vec2 max(vec2 a, vec2 b) { return vec2(max(a.x, b.y), max(a.y, b.y)); }
// f32 fabs(f32 a) { return a < 0 ? -a : a; }
inline vec2 abs(vec2 a) { return vec2(abs(a.x), abs(a.y)); }
inline f32 approach(f32 t, f32 target, f32 delta) { return t < target ? min(t + delta, target) : max(t - delta, target); }
inline f32 map(f32 t, f32 lo, f32 hi, f32 old_lo = 0, f32 old_hi = 1) { return lo + ((t - old_lo) / (old_hi - old_lo)) * (hi - lo); }
inline f32 smoothstep(f32 x) { return x * x * (3.0f - 2.0f * x); }
inline f32 ease_out_sin(f32 x) { return sinf((x * 3.14159265f) * 0.5f); }
inline f32 ease_in_sin(f32 x) { return 1.0f - cosf((x * 3.14159265f) * 0.5f); }
inline f32 ease_in_quart(f32 x) { return x * x * x * x; }
inline f32 ease_out_quart(f32 x) { return 1.0f - ease_in_quart(1.0f - x); }
inline f32 sign(f32 x) { return x >= 0 ? 1.0f : -1.0f; }
inline int sign(int x) { return x >= 0 ? 1 : -1; }
inline f32 lerp(f32 t, f32 a, f32 b) { return a + t * (b - a); }
inline vec2 lerp(f32 t, vec2 a, vec2 b) { return vec2(lerp(t, a.x, b.x), lerp(t, a.y, b.y)); }

inline f32 shortest_arc(vec2 a, vec2 b) {
    a = norm(a);
    b = norm(b);
    f32 c = dot(a, b);
    f32 s = det2(a, b);
    f32 theta = acosf(c);
    if (s > 0) {
        return theta;
    } else {
        return -theta;
    }
}

struct rotation {
    rotation() {}
    rotation(f32 angle) {
        s = sinf(angle);
        c = sinf(angle);
    }
    rotation(f32 s, f32 c) {
        this->s = s;
        this->c = c;
    }
    f32 s;
    f32 c;
};

inline rotation sincos(f32 a) {
    rotation r;
    r.c = cosf(a);
    r.s = sinf(a);
    return r;
}
inline f32 atan2_360(f32 y, f32 x) { return atan2f(-y, -x) + 3.14159265f; }
inline f32 atan2_360(rotation r) { return atan2_360(r.s, r.c); }
inline f32 atan2_360(vec2 v) { return atan2f(-v.y, -v.x) + 3.14159265f; }
inline vec2 mul(rotation a, vec2 b) { return vec2(a.c * b.x - a.s * b.y, a.s * b.x + a.c * b.y); }

struct halfspace {
    halfspace() {}
    halfspace(vec2 n, vec2 p) {
        this->n = n;
        c = dot(n, p);
    }
    halfspace(vec2 n, f32 c) {
        this->n = n;
        this->c = c;
    }
    vec2 n;
    f32 c;
};

inline f32 distance(halfspace h, vec2 p) { return dot(h.n, p) - h.c; }
inline vec2 intersect(vec2 a, vec2 b, f32 da, f32 db) { return a + (b - a) * (da / (da - db)); }
inline vec2 intersect(halfspace h, vec2 a, vec2 b) { return intersect(a, b, distance(h, a), distance(h, b)); }

inline vec2 bezier(vec2 a, vec2 b, vec2 c, f32 t) {
    f32 u = 1.0f - t;
    f32 ut = u * t;
    vec2 auu = a * u * u;
    vec2 but2 = b * ut * 2.0f;
    vec2 ctt = c * t * t;
    return auu + but2 + ctt;
}

inline vec2 bezier(vec2 a, vec2 b, vec2 c, vec2 d, f32 t) {
    f32 u = 1 - t;
    f32 tt = t * t;
    f32 uu = u * u;
    vec2 auuu = a * uu * u;
    vec2 buut3 = b * uu * t * 3.0f;
    vec2 cutt3 = c * u * tt * 3.0f;
    vec2 dttt = d * tt * t;
    return auuu + buut3 + cutt3 + dttt;
}

struct m2 {
    vec2 x;
    vec2 y;
};

inline m2 m2_identity() {
    m2 m;
    m.x = vec2(1, 0);
    m.y = vec2(0, 1);
    return m;
}

inline m2 m2_rotation(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    m2 m;
    m.x = vec2(c, -s);
    m.y = vec2(s, c);
    return m;
}
inline m2 m2_scale(f32 x_scale, f32 y_scale) {
    m2 m = m2_identity();
    m.x *= x_scale;
    m.y *= y_scale;
    return m;
}

inline vec2 mul(m2 m, vec2 v) { return vec2(m.x.x * v.x + m.y.x * v.y, m.x.y * v.x + m.y.y * v.y); }
inline m2 mul(m2 a, m2 b) {
    m2 c;
    c.x = mul(a, b.x);
    c.y = mul(a, b.y);
    return c;
}

struct aabb {
    aabb() {}
    aabb(vec2 min, vec2 max) {
        this->min = min;
        this->max = max;
    }
    aabb(vec2 p, f32 w, f32 h) {
        min = p - vec2(w, h);
        max = p + vec2(w, h);
    }
    vec2 min;
    vec2 max;
};

inline f32 width(aabb box) { return box.max.x - box.min.x; }
inline f32 height(aabb box) { return box.max.y - box.min.y; }
inline vec2 center(aabb box) { return (box.min + box.max) * 0.5f; }
inline vec2 top(aabb box) { return vec2((box.min.x + box.max.x) * 0.5f, box.max.y); }
inline vec2 bottom(aabb box) { return vec2((box.min.x + box.max.x) * 0.5f, box.min.y); }
inline vec2 left(aabb box) { return vec2(box.min.x, (box.min.y + box.max.y) * 0.5f); }
inline vec2 right(aabb box) { return vec2(box.max.x, (box.min.y + box.max.y) * 0.5f); }

struct circle {
    circle() {}
    circle(vec2 p, f32 r) {
        this->p = p;
        this->r = r;
    }
    circle(f32 x, f32 y, f32 r) {
        this->p = vec2(x, y);
        this->r = r;
    }
    vec2 p;
    f32 r;
};

#define POLYGON_MAX_VERTS 8

struct polygon {
    int count = 0;
    vec2 verts[POLYGON_MAX_VERTS];
    vec2 norms[POLYGON_MAX_VERTS];
    void compute_norms() {
        for (int i = 0; i < count; ++i) {
            int j = i + 1 < count ? i + 1 : 0;
            norms[i] = norm(skew(verts[i] - verts[j]));
        }
    }
};

inline bool point_in_poly(polygon poly, vec2 p) {
    for (int i = 0; i < poly.count; ++i) {
        f32 c = dot(poly.norms[i], poly.verts[i]);
        f32 dist = dot(poly.norms[i], p) - c;
        if (dist >= 0) return false;
    }
    return true;
}

struct raycast_output {
    f32 t;   // Time of impact.
    vec2 n;  // Normal of the surface at impact (unit length).
};

struct ray {
    ray() {}
    ray(vec2 p, vec2 d, f32 t) {
        this->p = p;
        this->d = d;
        this->t = t;
    }
    vec2 p;  // Start position.
    vec2 d;  // Direction of the ray (normalized)
    f32 t;   // Distance along d the ray travels.
    vec2 endpoint() { return p + d * t; }
    vec2 impact(raycast_output hit_data) { return p + d * hit_data.t; }
};

// Reflect vector d across vector n. See: http://paulbourke.net/geometry/reflected/
inline vec2 reflect(vec2 d, vec2 n) { return d - n * 2 * dot(d, n); }

inline bool raycast(ray r, circle circ, raycast_output* out) {
    vec2 e = r.p - circ.p;
    f32 rr = circ.r * circ.r;
    f32 b = dot(e, r.d);
    f32 c = dot(e, e) - rr;
    f32 discriminant = b * b - c;
    bool missed_circle = discriminant < 0;
    if (missed_circle) return false;
    f32 t = -b - sqrtf(discriminant);
    if (t >= 0 && t <= r.t) {
        out->t = t;
        vec2 impact = r.p + r.d * t;
        out->n = norm(impact - circ.p);
        return true;
    } else {
        return false;
    }
}

inline bool raycast(ray r, polygon poly, raycast_output* out = NULL) {
    f32 lo = 0;
    f32 hi = r.t;
    int index = ~0;

    for (int i = 0; i < poly.count; ++i) {
        // Calculate distance of point to plane.
        // This is a slight variation of dot(n, p) - c, where instead of pre-computing c as scalar,
        // we form a vector pointing from the ray's start point to a point on the plane. This is
        // functionally equivalent to dot(n, p) - c, but we don't have to store c inside of our poly.
        f32 distance = dot(poly.norms[i], poly.verts[i] - r.p);
        f32 denominator = dot(poly.norms[i], r.d);
        if (denominator == 0) {
            // Ray direction is parallel to this poly's face plane.
            // If the ray's start direction is outside the plane we know there's no intersection.
            if (distance > 0) {
                return false;
            }
        } else {
            f32 t = distance / denominator;
            if (denominator < 0) {
                // Ray is entering the plane.
                lo = max(lo, t);
                index = i;
            } else
                hi = min(hi, t);  // Ray is exiting the plane.
            bool ray_clipped_away = lo > hi;
            if (ray_clipped_away) {
                return false;
            }
        }
    }

    if (index != ~0) {
        if (out) {
            out->t = lo;
            out->n = poly.norms[index];
        }
        return true;
    } else {
        return false;
    }
}

// Based on Andrew's Algorithm from Ericson's Real-Time Collision Detection book.
inline int convex_hull(vec2* verts, int count) {
    count = min(count, POLYGON_MAX_VERTS);
    if (count < 3) {
        return 0;
    }

    // Sort lexicographically (on x-axis, then y-axis).
    for (int i = 0; i < count; ++i) {
        int lo = i;
        for (int j = i + 1; j < count; ++j) {
            if (verts[j].x < verts[lo].x) {
                lo = j;
            } else if (verts[j].x == verts[lo].x && verts[j].y < verts[lo].y) {
                lo = j;
            }
        }
        vec2 swap = verts[i];
        verts[i] = verts[lo];
        verts[lo] = swap;
    }

    int j = 2;
    int hull[POLYGON_MAX_VERTS + 1];
    hull[0] = 0;
    hull[1] = 1;

    // Find lower-half of hull.
    for (int i = 2; i < count; ++i) {
        while (j >= 2) {
            vec2 e0 = verts[hull[j - 1]] - verts[hull[j - 2]];
            vec2 e1 = verts[i] - verts[hull[j - 2]];
            if (det2(e0, e1) <= 0)
                --j;
            else
                break;
        }
        hull[j++] = i;
    }

    // Find top-half of hull.
    for (int i = count - 2, k = j + 1; i >= 0; --i) {
        while (j >= k) {
            vec2 e0 = verts[hull[j - 1]] - verts[hull[j - 2]];
            vec2 e1 = verts[i] - verts[hull[j - 2]];
            if (det2(e0, e1) <= 0)
                --j;
            else
                break;
        }
        hull[j++] = i;
    }

    --j;  // Pop the last vert off as it's a duplicate.
    if (j < 3) return 0;
    vec2 hull_verts[POLYGON_MAX_VERTS];
    for (int i = 0; i < j; ++i) hull_verts[i] = verts[hull[i]];
    memcpy(verts, hull_verts, sizeof(vec2) * j);
    return j;
}

struct collision_data {
    bool hit = false;
    vec2 hit_spot;
    f32 depth;
    vec2 normal;
};

inline collision_data circle_to_circle(circle circ_a, circle circ_b) {
    collision_data out;
    vec2 d = circ_b.p - circ_a.p;
    f32 d2 = dot(d, d);
    f32 r = circ_a.r + circ_b.r;
    if (d2 < r * r) {
        f32 l = len(d);
        d = l == 0 ? vec2(0, 1) : d * (1.0f / l);
        out.hit = true;
        out.hit_spot = circ_b.p - d * circ_b.r;
        out.depth = r - l;
        out.normal = d;
    }
    return out;
}

inline collision_data aabb_to_aabb(aabb a, aabb b) {
    collision_data out;
    vec2 mid_a = (a.min + a.max) * 0.5f;
    vec2 mid_b = (b.min + b.max) * 0.5f;
    vec2 ea = abs((a.max - a.min) * 0.5f);
    vec2 eb = abs((b.max - b.min) * 0.5f);
    vec2 d = mid_b - mid_a;

    // calc overlap on x and y axes
    f32 dx = ea.x + eb.x - abs(d.x);
    if (dx < 0) return out;
    f32 dy = ea.y + eb.y - abs(d.y);
    if (dy < 0) return out;

    vec2 n;
    f32 depth;
    vec2 p;

    if (dx < dy) {
        // x axis overlap is smaller
        depth = dx;
        if (d.x < 0) {
            n = vec2(-1.0f, 0);
            p = mid_a - vec2(ea.x, 0);
        } else {
            n = vec2(1.0f, 0);
            p = mid_a + vec2(ea.x, 0);
        }
    } else {
        // y axis overlap is smaller
        depth = dy;
        if (d.y < 0) {
            n = vec2(0, -1.0f);
            p = mid_a - vec2(0, ea.y);
        } else {
            n = vec2(0, 1.0f);
            p = mid_a + vec2(0, ea.y);
        }
    }

    out.hit = true;
    out.hit_spot = p;
    out.depth = depth;
    out.normal = n;
    return out;
}

inline f32 clamp(f32 v, f32 lo, f32 hi) { return min(max(v, lo), hi); }
inline vec2 clamp(vec2 v, vec2 lo, vec2 hi) { return vec2(clamp(v.x, lo.x, hi.x), clamp(v.y, lo.y, hi.y)); }

inline collision_data circle_to_aabb(circle a, aabb b) {
    collision_data out;
    vec2 L = clamp(a.p, b.min, b.max);
    vec2 ab = a.p - L;
    f32 d2 = dot(ab, ab);
    f32 r2 = a.r * a.r;

    if (d2 < r2) {
        if (d2 != 0) {
            // shallow (center of circle not inside of AABB)
            f32 d = sqrtf(d2);
            vec2 n = norm(ab);
            out.hit = true;
            out.depth = a.r - d;
            out.hit_spot = a.p + n * d;
            out.normal = n;
        } else {
            // deep (center of circle inside of AABB)
            // clamp circle's center to edge of AABB, then form the manifold
            vec2 mid = (b.min + b.max) * 0.5f;
            vec2 e = (b.max - b.min) * 0.5f;
            vec2 d = a.p - mid;
            vec2 abs_d = abs(d);

            f32 x_overlap = e.x - abs_d.x;
            f32 y_overlap = e.y - abs_d.y;

            f32 depth;
            vec2 n;

            if (x_overlap < y_overlap) {
                depth = x_overlap;
                n = vec2(1.0f, 0);
                n *= (d.x < 0 ? 1.0f : -1.0f);
            } else {
                depth = y_overlap;
                n = vec2(0, 1.0f);
                n *= (d.y < 0 ? 1.0f : -1.0f);
            }

            out.hit = true;
            out.depth = a.r + depth;
            out.hit_spot = a.p - n * depth;
            out.normal = n;
        }
    }

    return out;
}

inline vec2 calc_center_of_mass(polygon poly) {
    vec2 p0 = poly.verts[0];
    f32 area_sum = 0;
    const f32 inv3 = 1.0f / 3.0f;
    vec2 center_of_mass = vec2(0, 0);

    // Triangle fan of p0, p1, and p2.
    for (int i = 0; i < poly.count; ++i) {
        vec2 p1 = poly.verts[i];
        vec2 p2 = poly.verts[i + 1 == poly.count ? 0 : i + 1];

        // Sum the area of all triangles.
        f32 area_of_triangle = det2(p1 - p0, p2 - p0) * 0.5f;
        area_sum += area_of_triangle;

        // Center of mass is the area-weighted centroid.
        // Centroid is the average of all vertices.
        center_of_mass += (p0 + p1 + p2) * (area_of_triangle * inv3);
    }

    center_of_mass *= 1.0f / area_sum;
    return center_of_mass;
}

inline f32 calc_area(polygon poly) {
    vec2 p0 = poly.verts[0];
    f32 area = 0;

    // Triangle fan of p0, p1, and p2.
    for (int i = 0; i < poly.count; ++i) {
        vec2 p1 = poly.verts[i];
        vec2 p2 = poly.verts[i + 1 == poly.count ? 0 : i + 1];
        area += det2(p1 - p0, p2 - p0) * 0.5f;
    }

    return area;
}

struct sutherland_hodgman_output {
    polygon front;
    polygon back;
};

inline bool in_front(f32 distance, f32 epsilon) { return distance > epsilon; }
inline bool behind(f32 distance, f32 epsilon) { return distance < -epsilon; }
inline bool on(f32 distance, f32 epsilon) { return !in_front(distance, epsilon) && !behind(distance, epsilon); }

// See: https://gamedevelopment.tutsplus.com/tutorials/how-to-dynamically-slice-a-convex-shape--gamedev-14479
inline sutherland_hodgman_output sutherland_hodgman(halfspace split, polygon in, const f32 k_epsilon = 1.e-4f) {
    sutherland_hodgman_output out;
    vec2 a = in.verts[in.count - 1];
    f32 da = distance(split, a);

    for (int i = 0; i < in.count; ++i) {
        vec2 b = in.verts[i];
        f32 db = distance(split, b);

        if (in_front(db, k_epsilon)) {
            if (behind(da, k_epsilon)) {
                vec2 i = intersect(b, a, db, da);
                out.front.verts[out.front.count++] = i;
                out.back.verts[out.back.count++] = i;
            }
            out.front.verts[out.front.count++] = b;
        } else if (behind(db, k_epsilon)) {
            if (in_front(da, k_epsilon)) {
                vec2 i = intersect(a, b, da, db);
                out.front.verts[out.front.count++] = i;
                out.back.verts[out.back.count++] = i;
            } else if (on(da, k_epsilon)) {
                out.back.verts[out.back.count++] = a;
            }
            out.back.verts[out.back.count++] = b;
        } else {
            out.front.verts[out.front.count++] = b;
            if (on(da, k_epsilon)) {
                out.back.verts[out.back.count++] = b;
            }
        }

        a = b;
        da = db;
    }

    return out;
}

#endif

#endif