#include "neko_physics.h"

// Includes
#ifndef NEKO_PHYSICS_NO_CCD
#include "deps/ccd/src/ccd/ccd.h"
#include "deps/ccd/src/ccd/ccd_quat.h"
#include "deps/ccd/src/ccd/ccd_vec3.h"
#endif

// 2D AABB collision detection (rect. vs. rect.)
b32 neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b) {
    if (a->max.x > b->min.x && a->max.y > b->min.y && a->min.x < b->max.x && a->min.y < b->max.y) {
        return true;
    }

    return false;
}

/*==== Support Functions =====*/

// Poly
NEKO_API_DECL void neko_support_poly(const void* _o, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    const neko_poly_t* p = (neko_poly_t*)_o;

    // Bring direction vector into rotation space
    neko_quat qinv = neko_quat_inverse(xform->rotation);
    neko_vec3 d = neko_quat_rotate(qinv, *dir);

    // Iterate over all points, find dot farthest in direction of d
    double max_dot, dot = 0.0;
    max_dot = (double)-FLT_MAX;
    for (u32 i = 0; i < p->cnt; i++) {
        dot = (double)neko_vec3_dot(d, p->verts[i]);
        if (dot > max_dot) {
            *out = p->verts[i];
            max_dot = dot;
        }
    }

    // Transform support point by rotation and translation of object
    *out = neko_quat_rotate(xform->rotation, *out);
    *out = neko_vec3_mul(xform->scale, *out);
    *out = neko_vec3_add(xform->position, *out);
}

// Sphere
NEKO_API_DECL void neko_support_sphere(const void* _o, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    // Support function is made according to Gino van den Bergen's paper
    //  A Fast and Robust CCD Implementation for Collision Detection of
    //  Convex Objects
    const neko_sphere_t* s = (neko_sphere_t*)_o;
    float scl = neko_max(xform->scale.x, neko_max(xform->scale.z, xform->scale.y));
    *out = neko_vec3_add(neko_vec3_scale(neko_vec3_norm(*dir), scl * s->r), neko_vec3_add(xform->position, s->c));
}

// Ray
NEKO_API_DECL void neko_support_ray(const void* _r, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    const neko_ray_t* r = (neko_ray_t*)_r;

    // Bring direction vector into rotation space
    neko_quat qinv = neko_quat_inverse(xform->rotation);
    neko_vec3 d = neko_quat_rotate(qinv, *dir);

    neko_vec3 rs = r->p;
    neko_vec3 re = neko_vec3_add(r->p, neko_vec3_scale(r->d, r->len));

    if (neko_vec3_dot(rs, d) > neko_vec3_dot(re, d)) {
        *out = rs;
    } else {
        *out = re;
    }

    // Transform support point by rotation and translation of object
    *out = neko_quat_rotate(xform->rotation, *out);
    *out = neko_vec3_mul(xform->scale, *out);
    *out = neko_vec3_add(xform->position, *out);
}

// AABB
NEKO_API_DECL void neko_support_aabb(const void* _o, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    const neko_aabb_t* a = (neko_aabb_t*)_o;

    // Bring direction vector into rotation space
    neko_quat qinv = neko_quat_inverse(xform->rotation);
    neko_vec3 d = neko_quat_rotate(qinv, *dir);

    // Compute half coordinates and sign for aabb (scale by transform)
    const float hx = (a->max.x - a->min.x) * 0.5f * xform->scale.x;
    const float hy = (a->max.y - a->min.y) * 0.5f * xform->scale.y;
    const float hz = (a->max.z - a->min.z) * 0.5f * xform->scale.z;
    neko_vec3 s = neko_vec3_sign(d);

    // Compure support for aabb
    *out = neko_v3(s.x * hx, s.y * hy, s.z * hz);

    // Transform support point by rotation and translation of object
    *out = neko_quat_rotate(xform->rotation, *out);
    *out = neko_vec3_add(xform->position, *out);
}

// Cylinder
NEKO_API_DECL void neko_support_cylinder(const void* _o, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    // Support function is made according to Gino van den Bergen's paper
    //  A Fast and Robust CCD Implementation for Collision Detection of
    //  Convex Objects

    const neko_cylinder_t* c = (const neko_cylinder_t*)_o;

    // Bring direction vector into rotation space
    neko_quat qinv = neko_quat_inverse(xform->rotation);
    neko_vec3 d = neko_quat_rotate(qinv, *dir);

    // Compute support point (cylinder is standing on y axis, half height at origin)
    double zdist = sqrt(d.x * d.x + d.z * d.z);
    double hh = (double)c->height * 0.5 * xform->scale.y;
    if (zdist == 0.0) {
        *out = neko_v3(0.0, neko_vec3_signY(d) * hh, 0.0);
    } else {
        double r = (double)c->r / zdist;
        *out = neko_v3(r * d.x * xform->scale.x, neko_vec3_signY(d) * hh, r * d.z * xform->scale.z);
    }

    // Transform support point into world space
    *out = neko_quat_rotate(xform->rotation, *out);
    *out = neko_vec3_add(neko_vec3_add(xform->position, c->base), *out);
}

// Capsule
NEKO_API_DECL void neko_support_capsule(const void* _o, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    const neko_capsule_t* c = (const neko_capsule_t*)_o;

    // Bring direction vector into rotation space
    neko_quat qinv = neko_quat_inverse(xform->rotation);
    neko_vec3 d = neko_quat_rotate(qinv, *dir);

    // Compute support point (cone is standing on y axis, half height at origin)
    const float s = neko_max(xform->scale.x, xform->scale.z);
    *out = neko_vec3_scale(neko_vec3_norm(d), c->r * s);
    double hh = (double)c->height * 0.5 * xform->scale.y;

    if (neko_vec3_dot(d, NEKO_YAXIS) > 0.0) {
        out->y += hh;
    } else {
        out->y -= hh;
    }

    // Transform support point into world space
    *out = neko_quat_rotate(xform->rotation, *out);
    *out = neko_vec3_add(neko_vec3_add(xform->position, c->base), *out);
}

// Cone
NEKO_API_DECL void neko_support_cone(const void* _o, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out) {
    const neko_cone_t* c = (const neko_cone_t*)_o;

    // Bring direction vector into rotation space
    neko_quat qinv = neko_quat_inverse(xform->rotation);
    neko_vec3 d = neko_quat_rotate(qinv, *dir);

    // Compute support point (cone is standing on y axis, half height at origin)
    double sin_angle = c->r / sqrt((double)c->r * (double)c->r + (double)c->height * (double)c->height);
    double hh = (double)c->height * 0.5 * xform->scale.y;
    double len = sqrt(neko_vec3_len2(d));

    if (d.y > len * sin_angle) {
        *out = neko_v3(0.0f, (float)hh, 0.0f);
    } else {
        double s = sqrt(d.x * d.x + d.z * d.z);
        if (s > (double)NEKO_GJK_EPSILON) {
            double _d = (double)c->r / s;
            *out = neko_v3(d.x * _d * xform->scale.x, (float)-hh, d.z * _d * xform->scale.z);
        } else {
            *out = neko_v3(0.0, (float)-hh, 0.0);
        }
    }

    // Transform support point into world space
    *out = neko_quat_rotate(xform->rotation, *out);
    *out = neko_vec3_add(neko_vec3_add(xform->position, c->base), *out);
}

/*==== Collision Shapes ====*/

/* Line/Segment */

NEKO_API_DECL neko_line_t neko_line_closest_line(const neko_line_t* l, neko_vec3 p) {
    neko_vec3 cp = neko_line_closest_point(l, p);
    return neko_line(p, cp);
}

NEKO_API_DECL neko_vec3 neko_line_closest_point(const neko_line_t* l, neko_vec3 p) {
    neko_vec3 pt = neko_default_val();
    neko_vec3 ab = neko_vec3_sub(l->b, l->a);
    neko_vec3 pa = neko_vec3_sub(p, l->a);
    float t = neko_vec3_dot(pa, ab) / neko_vec3_dot(ab, ab);
    t = neko_clamp(t, 0.f, 1.f);
    pt = neko_vec3_add(l->a, neko_vec3_scale(ab, t));
    return pt;
}

NEKO_API_DECL neko_vec3 neko_line_direction(const neko_line_t* l) { return neko_vec3_norm(neko_vec3_sub(l->b, l->a)); }

/* Plane */

// Modified from: https://graphics.stanford.edu/~mdfisher/Code/Engine/Plane.cpp.html
NEKO_API_DECL neko_plane_t neko_plane_from_pt_normal(neko_vec3 pt, neko_vec3 n) {
    neko_plane_t p = neko_default_val();
    neko_vec3 nn = neko_vec3_norm(n);
    p.a = nn.x;
    p.b = nn.y;
    p.c = nn.z;
    p.d = -neko_vec3_dot(pt, nn);
    return p;
}

NEKO_API_DECL neko_plane_t neko_plane_from_pts(neko_vec3 a, neko_vec3 b, neko_vec3 c) {
    neko_vec3 n = neko_vec3_norm(neko_vec3_cross(neko_vec3_sub(b, a), neko_vec3_sub(c, a)));
    return neko_plane_from_pt_normal(a, n);
}

NEKO_API_DECL neko_vec3 neko_plane_normal(const neko_plane_t* p) { return neko_vec3_norm(neko_v3(p->a, p->b, p->c)); }

NEKO_API_DECL neko_vec3 neko_plane_closest_point(const neko_plane_t* p, neko_vec3 pt) { return neko_vec3_sub(pt, neko_vec3_scale(neko_plane_normal(p), neko_plane_signed_distance(p, pt))); }

NEKO_API_DECL float neko_plane_signed_distance(const neko_plane_t* p, neko_vec3 pt) { return (p->a * pt.x + p->b * pt.y + p->c * pt.z + p->d); }

NEKO_API_DECL float neko_plane_unsigned_distance(const neko_plane_t* p, neko_vec3 pt) { return fabsf(neko_plane_signed_distance(p, pt)); }

NEKO_API_DECL neko_plane_t neko_plane_normalized(const neko_plane_t* p) {
    neko_plane_t pn = neko_default_val();
    float d = sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
    pn.a = p->a / d;
    pn.b = p->b / d;
    pn.c = p->c / d;
    pn.d = p->d / d;
    return pn;
}

NEKO_API_DECL s32 neko_plane_vs_sphere(const neko_plane_t* a, neko_vqs* xform_a, const neko_sphere_t* b, neko_vqs* xform_b, struct neko_contact_info_t* res) {
    // Cache necesary transforms, matrices
    neko_mat4 mat = xform_a ? neko_vqs_to_mat4(xform_a) : neko_mat4_identity();
    neko_mat4 inv = xform_a ? neko_mat4_inverse(mat) : neko_mat4_identity();
    neko_vqs local = neko_vqs_relative_transform(xform_a, xform_b);

    // Transform sphere center into local cone space
    neko_vec3 sc = xform_a ? neko_mat4_mul_vec3(inv, xform_b ? neko_vec3_add(xform_b->position, b->c) : b->c) : b->c;

    // Determine closest point from sphere center to plane
    neko_vec3 cp = neko_plane_closest_point(a, sc);

    // Determine if sphere is intersecting this point
    float sb = xform_b ? neko_max(local.scale.x, neko_max(local.scale.y, local.scale.z)) : 1.f;
    neko_vec3 dir = neko_vec3_sub(cp, sc);
    neko_vec3 n = neko_vec3_norm(dir);
    float d = neko_vec3_len(dir);
    float r = sb * b->r;

    if (d > r) {
        return false;
    }

    // Construct contact information
    if (res) {
        res->hit = true;
        res->depth = (r - d);
        res->normal = neko_mat4_mul_vec3(mat, n);
        res->point = neko_mat4_mul_vec3(mat, cp);
    }

    return true;
}

/* Ray */

/* Frustum */

/* Hit */

#ifndef NEKO_PHYSICS_NO_CCD

#define _NEKO_COLLIDE_FUNC_IMPL(_TA, _TB, _F0, _F1)                                                                                                             \
    NEKO_API_DECL s32 neko_##_TA##_vs_##_TB(const neko_##_TA##_t* a, const neko_vqs* xa, const neko_##_TB##_t* b, const neko_vqs* xb, neko_contact_info_t* r) { \
        return _neko_ccd_gjk_internal(a, xa, (_F0), b, xb, (_F1), r);                                                                                           \
    }

/* Sphere */

_NEKO_COLLIDE_FUNC_IMPL(sphere, sphere, neko_support_sphere, neko_support_sphere);      // Sphere vs. Sphere
_NEKO_COLLIDE_FUNC_IMPL(sphere, cylinder, neko_support_sphere, neko_support_cylinder);  // Sphere vs. Cylinder
_NEKO_COLLIDE_FUNC_IMPL(sphere, cone, neko_support_sphere, neko_support_cone);          // Sphere vs. Cone
_NEKO_COLLIDE_FUNC_IMPL(sphere, aabb, neko_support_sphere, neko_support_aabb);          // Sphere vs. AABB
_NEKO_COLLIDE_FUNC_IMPL(sphere, capsule, neko_support_sphere, neko_support_capsule);    // Sphere vs. Capsule
_NEKO_COLLIDE_FUNC_IMPL(sphere, poly, neko_support_sphere, neko_support_poly);          // Sphere vs. Poly
_NEKO_COLLIDE_FUNC_IMPL(sphere, ray, neko_support_sphere, neko_support_ray);            // Sphere vs. Ray
/* AABB */

_NEKO_COLLIDE_FUNC_IMPL(aabb, aabb, neko_support_aabb, neko_support_aabb);          // AABB vs. AABB
_NEKO_COLLIDE_FUNC_IMPL(aabb, cylinder, neko_support_aabb, neko_support_cylinder);  // AABB vs. Cylinder
_NEKO_COLLIDE_FUNC_IMPL(aabb, cone, neko_support_aabb, neko_support_cone);          // AABB vs. Cone
_NEKO_COLLIDE_FUNC_IMPL(aabb, sphere, neko_support_aabb, neko_support_sphere);      // AABB vs. Sphere
_NEKO_COLLIDE_FUNC_IMPL(aabb, capsule, neko_support_aabb, neko_support_capsule);    // AABB vs. Capsule
_NEKO_COLLIDE_FUNC_IMPL(aabb, poly, neko_support_aabb, neko_support_poly);          // AABB vs. Poly
_NEKO_COLLIDE_FUNC_IMPL(aabb, ray, neko_support_aabb, neko_support_ray);            // AABB vs. Ray

/* Capsule */

_NEKO_COLLIDE_FUNC_IMPL(capsule, capsule, neko_support_capsule, neko_support_capsule);    // Capsule vs. Capsule
_NEKO_COLLIDE_FUNC_IMPL(capsule, cylinder, neko_support_capsule, neko_support_cylinder);  // Capsule vs. Cylinder
_NEKO_COLLIDE_FUNC_IMPL(capsule, cone, neko_support_capsule, neko_support_cone);          // Capsule vs. Cone
_NEKO_COLLIDE_FUNC_IMPL(capsule, sphere, neko_support_capsule, neko_support_sphere);      // Capsule vs. Sphere
_NEKO_COLLIDE_FUNC_IMPL(capsule, aabb, neko_support_capsule, neko_support_aabb);          // Capsule vs. AABB
_NEKO_COLLIDE_FUNC_IMPL(capsule, poly, neko_support_capsule, neko_support_poly);          // Capsule vs. Poly
_NEKO_COLLIDE_FUNC_IMPL(capsule, ray, neko_support_capsule, neko_support_ray);            // Capsule vs. Ray

/* Poly */

_NEKO_COLLIDE_FUNC_IMPL(poly, poly, neko_support_poly, neko_support_poly);          // Poly vs. Poly
_NEKO_COLLIDE_FUNC_IMPL(poly, cylinder, neko_support_poly, neko_support_cylinder);  // Poly vs. Cylinder
_NEKO_COLLIDE_FUNC_IMPL(poly, cone, neko_support_poly, neko_support_cone);          // Poly vs. Cone
_NEKO_COLLIDE_FUNC_IMPL(poly, sphere, neko_support_poly, neko_support_sphere);      // Poly vs. Sphere
_NEKO_COLLIDE_FUNC_IMPL(poly, aabb, neko_support_poly, neko_support_aabb);          // Poly vs. AABB
_NEKO_COLLIDE_FUNC_IMPL(poly, capsule, neko_support_poly, neko_support_capsule);    // Poly vs. Capsule
_NEKO_COLLIDE_FUNC_IMPL(poly, ray, neko_support_poly, neko_support_ray);            // Poly vs. Ray

/* Cylinder */

_NEKO_COLLIDE_FUNC_IMPL(cylinder, cylinder, neko_support_cylinder, neko_support_cylinder);  // Cylinder vs. Cylinder
_NEKO_COLLIDE_FUNC_IMPL(cylinder, poly, neko_support_poly, neko_support_poly);              // Cylinder vs. Poly
_NEKO_COLLIDE_FUNC_IMPL(cylinder, cone, neko_support_cylinder, neko_support_cone);          // Cylinder vs. Cone
_NEKO_COLLIDE_FUNC_IMPL(cylinder, sphere, neko_support_cylinder, neko_support_sphere);      // Cylinder vs. Sphere
_NEKO_COLLIDE_FUNC_IMPL(cylinder, aabb, neko_support_cylinder, neko_support_aabb);          // Cylinder vs. AABB
_NEKO_COLLIDE_FUNC_IMPL(cylinder, capsule, neko_support_cylinder, neko_support_capsule);    // Cylinder vs. Capsule
_NEKO_COLLIDE_FUNC_IMPL(cylinder, ray, neko_support_cylinder, neko_support_ray);            // Cylinder vs. Ray

/* Cone */

_NEKO_COLLIDE_FUNC_IMPL(cone, cone, neko_support_cone, neko_support_cone);          // Cone vs. Cone
_NEKO_COLLIDE_FUNC_IMPL(cone, poly, neko_support_poly, neko_support_poly);          // Cone vs. Poly
_NEKO_COLLIDE_FUNC_IMPL(cone, cylinder, neko_support_cone, neko_support_cylinder);  // Cone vs. Cylinder
_NEKO_COLLIDE_FUNC_IMPL(cone, sphere, neko_support_cone, neko_support_sphere);      // Cone vs. Sphere
_NEKO_COLLIDE_FUNC_IMPL(cone, aabb, neko_support_cone, neko_support_aabb);          // Cone vs. AABB
_NEKO_COLLIDE_FUNC_IMPL(cone, capsule, neko_support_cone, neko_support_capsule);    // Cone vs. Capsule
_NEKO_COLLIDE_FUNC_IMPL(cone, ray, neko_support_cone, neko_support_ray);            // Cone vs. Ray

#endif

/*==== GKJ ====*/

// Need 2D GJK/EPA impl in external (modify from chipmunk 2d)

// Internal functions
/*
bool neko_gjk_check_if_simplex_contains_origin(neko_gjk_simplex_t* simplex, neko_vec3* search_dir, neko_gjk_dimension dimension);
void neko_gjk_simplex_push(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t p);
void neko_gjk_simplex_push_back(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t p);
void neko_gjk_simplex_push_front(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t p);
void neko_gjk_simplex_insert(neko_gjk_simplex_t* simplex, u32 idx, neko_gjk_support_point_t p);
void neko_gjk_bary(neko_vec3 p, neko_vec3 a, neko_vec3 b, neko_vec3 c, float* u, float* v, float* w);
// neko_gjk_epa_edge_t neko_gjk_epa_find_closest_edge(neko_gjk_simplex_t* simplex);
neko_gjk_support_point_t neko_gjk_generate_support_point(const neko_gjk_collider_info_t* ci0, const neko_gjk_collider_info_t* ci1, neko_vec3 dir);
neko_gjk_epa_edge_t neko_gjk_epa_find_closest_edge(neko_dyn_array(neko_gjk_support_point_t) polytope);

// Modified from: https://github.com/Nightmask3/Physics-Framework/blob/master/PhysicsFramework/PhysicsManager.cpp
s32 neko_gjk(const neko_gjk_collider_info_t* ci0, const neko_gjk_collider_info_t* ci1, neko_gjk_dimension dimension, neko_contact_info_t* res)
{
    // Simplex simplex;
    neko_gjk_simplex_t simplex = neko_default_val();
    neko_vec3 search_dir = neko_v3s(1.f);
    neko_gjk_support_point_t new_pt = neko_gjk_generate_support_point(ci0, ci1, search_dir);

    // Stability check
    if (neko_vec3_dot(search_dir, new_pt.minkowski_hull_vert) >= neko_vec3_len(new_pt.minkowski_hull_vert) * 0.8f)
    {
        // the chosen direction is invalid, will produce (0,0,0) for a subsequent direction later
        search_dir = neko_v3(0.f, 1.f, 0.f);
        new_pt = neko_gjk_generate_support_point(ci0, ci1, search_dir);
    }
    neko_gjk_simplex_push(&simplex, new_pt);

    // Invert the search direction for the next point
    search_dir = neko_vec3_neg(search_dir);

    u32 iterationCount = 0;

    while (true)
    {
        if (iterationCount++ >= NEKO_GJK_MAX_ITERATIONS)
            return false;
        // Stability check
        // Error, for some reason the direction vector is broken
        if (neko_vec3_len(search_dir) <= 0.0001f)
            return false;

        // Add a new point to the simplex
        neko_gjk_support_point_t new_pt = neko_gjk_generate_support_point(ci0, ci1, search_dir);
        neko_gjk_simplex_push(&simplex, new_pt);

        // If projection of newly added point along the search direction has not crossed the origin,
        // the Minkowski Difference could not contain the origin, objects are not colliding
        if (neko_vec3_dot(new_pt.minkowski_hull_vert, search_dir) < 0.0f)
        {
            return false;
        }
        else
        {
            // If the new point IS past the origin, check if the simplex contains the origin,
            // If it doesn't modify search direction to point towards to origin
            if (neko_gjk_check_if_simplex_contains_origin(&simplex, &search_dir, dimension))
            {
                // Capture collision data using EPA if requested by user
                if (res)
                {
                    switch (dimension) {
                        case NEKO_GJK_DIMENSION_3D: *res = neko_gjk_epa(&simplex, ci0, ci1); break;
                        case NEKO_GJK_DIMENSION_2D: *res = neko_gjk_epa_2d(&simplex, ci0, ci1); break;
                    }
                    return res->hit;
                }
                else
                {
                    return true;
                }
            }
        }
    }
}

NEKO_API_DECL neko_gjk_support_point_t neko_gjk_generate_support_point(const neko_gjk_collider_info_t* ci0, const neko_gjk_collider_info_t* ci1, neko_vec3 dir)
{
   neko_gjk_support_point_t sp = {0};
   sp.support_a = ci0->func(ci0->collider, ci0->xform, dir);
   sp.support_b = ci1->func(ci1->collider, ci1->xform, neko_vec3_neg(dir));
   sp.minkowski_hull_vert = neko_vec3_sub(sp.support_a, sp.support_b);
   return sp;
}

// Closest point method taken from Erin Catto's GDC 2010 slides
// Returns the closest point
neko_vec3 neko_closest_point_on_line_from_target_point(neko_line_t line, neko_vec3 point, float* u, float* v)
{
    neko_vec3 line_seg = neko_vec3_sub(line.b, line.a);
    neko_vec3 normalized = neko_vec3_norm(line_seg);
    *v = neko_vec3_dot(neko_vec3_neg(line.a), normalized) / neko_vec3_len(line_seg);
    *u = neko_vec3_dot(line.b, normalized) / neko_vec3_len(line_seg);
    neko_vec3 closest_point;
    if (*u <= 0.0f)
    {
        closest_point = line.b;
    }
    else if (*v <= 0.0f)
    {
        closest_point = line.a;
    }
    else
    {
        closest_point = neko_vec3_add(neko_vec3_scale(line.a, *u), neko_vec3_scale(line.b, *v));
    }

    return closest_point;
}

void neko_gjk_simplex_push(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t p)
{
    simplex->ct = neko_min(simplex->ct + 1, 4);

    for (s32 i = simplex->ct - 1; i > 0; i--)
        simplex->points[i] = simplex->points[i - 1];

    simplex->points[0] = p;
}

void neko_gjk_simplex_push_back(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t p)
{
    if (simplex->ct >= 4) return;
    simplex->ct = neko_min(simplex->ct + 1, 4);
    simplex->points[simplex->ct - 1] = p;
}

void neko_gjk_simplex_push_front(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t p)
{
    if (simplex->ct == 3) {
        simplex->points[3] = simplex->points[2];
        simplex->points[2] = simplex->points[1];
        simplex->points[1] = simplex->points[0];
        simplex->points[0] = p;
    }
    else if (simplex->ct == 2) {
        simplex->points[2] = simplex->points[1];
        simplex->points[1] = simplex->points[0];
        simplex->points[0] = p;
    }
    simplex->ct = neko_min(simplex->ct + 1, 4);
}

void neko_gjk_simplex_insert(neko_gjk_simplex_t* simplex, u32 idx, neko_gjk_support_point_t p)
{
    // Need more points (this is where polytope comes into play, I think...)
    // Splice the simplex by index

    if (idx > 4) return;

    simplex->ct = neko_min(simplex->ct + 1, 4);

    for (s32 i = simplex->ct - 1; i > idx; i--)
        simplex->points[i] = simplex->points[i - 1];

    simplex->points[idx] = p;
}

void neko_gjk_simplex_clear(neko_gjk_simplex_t* simplex)
{
    simplex->ct = 0;
}

void neko_gjk_simplex_set1(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t a)
{
    simplex->ct = 1;
    simplex->a = a;
}

void neko_gjk_simplex_set2(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t a, neko_gjk_support_point_t b)
{
    simplex->ct = 2;
    simplex->a = a;
    simplex->b = b;
}

void neko_gjk_simplex_set3(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t a, neko_gjk_support_point_t b, neko_gjk_support_point_t c)
{
    simplex->ct = 3;
    simplex->a = a;
    simplex->b = b;
    simplex->c = c;
}

void neko_gjk_simplex_set4(neko_gjk_simplex_t* simplex, neko_gjk_support_point_t a, neko_gjk_support_point_t b, neko_gjk_support_point_t c, neko_gjk_support_point_t d)
{
    simplex->ct = 4;
    simplex->a = a;
    simplex->b = b;
    simplex->c = c;
    simplex->d = d;
}

bool neko_gjk_check_if_simplex_contains_origin(neko_gjk_simplex_t* simplex, neko_vec3* search_dir, neko_gjk_dimension dimension)
{
    // Line case
    if (simplex->ct == 2)
    {
        // Line cannot contain the origin, only search for the direction to it
        neko_vec3 new_point_to_old_point = neko_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        neko_vec3 new_point_to_origin = neko_vec3_neg(simplex->a.minkowski_hull_vert);

        // Method given by Erin Catto in GDC 2010 presentation
        float u = 0.0f, v = 0.0f;
        neko_line_t line = neko_line(simplex->a.minkowski_hull_vert, simplex->b.minkowski_hull_vert);
        neko_vec3 origin = neko_v3s(0.f);
        neko_vec3 closest_point = neko_closest_point_on_line_from_target_point(line, origin, &u, &v);

        // Test vertex region of new simplex point first as highest chance to be there
        if (v <= 0.0f)
        {
            neko_gjk_simplex_set1(simplex, simplex->a);
            *search_dir = neko_vec3_neg(closest_point);
            return false;
        }
        else if (u <= 0.0f)
        {
            neko_gjk_simplex_set1(simplex, simplex->b);
            *search_dir = neko_vec3_neg(closest_point);
            return false;
        }
        else
        {
            *search_dir = neko_vec3_neg(closest_point);
            return false;
        }
    }

    // Triangle case
    else if (simplex->ct == 3)
    {
        // Find the newly added features
        neko_vec3 new_point_to_origin = neko_vec3_neg(simplex->a.minkowski_hull_vert);
        neko_vec3 edge1 = neko_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);    // AB
        neko_vec3 edge2 = neko_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);    // AC
        // Find the normals to the triangle and the two edges
        neko_vec3 triangle_normal = neko_vec3_cross(edge1, edge2);  // ABC
        neko_vec3 edge1_normal = neko_vec3_cross(edge1, triangle_normal);
        neko_vec3 edge2_normal = neko_vec3_cross(triangle_normal, edge2);

        // If origin is closer to the area along the second edge normal (if same_dir(AB X ABC, -A))
        if (neko_vec3_dot(edge2_normal, new_point_to_origin) > 0.0f)
        {
            // If closer to the edge then find the normal that points towards the origin
            if (neko_vec3_dot(edge2, new_point_to_origin) > 0.0f)
            {
                // [AC]
                *search_dir = neko_vec3_triple_cross_product(edge2, new_point_to_origin, edge2);
                neko_gjk_simplex_clear(simplex);
                neko_gjk_simplex_set2(simplex, simplex->a, simplex->c);
                return false;
            }
            // If closer to the new simplex point
            else
            {
                // The "Star case" from the Muratori lecture
                // If new search direction should be along edge normal
                if (neko_vec3_dot(edge1, new_point_to_origin) > 0.0f)
                {
                    // [AB]
                    *search_dir = neko_vec3_triple_cross_product(edge1, new_point_to_origin, edge1);
                    neko_gjk_simplex_clear(simplex);
                    neko_gjk_simplex_set2(simplex, simplex->a, simplex->b);
                    return false;
                }
                else // If new search direction should be along the new Simplex point
                {
                    // Return new simplex point alone [A]
                    *search_dir = new_point_to_origin;
                    neko_gjk_simplex_clear(simplex);
                    neko_gjk_simplex_set1(simplex, simplex->a);
                    return false;
                }
            }
        }
        // In 2D, this is a "success" case, otherwise keep going for 3D
        else
        {
            // Max simplex dimension is a triangle
            if (dimension == NEKO_GJK_DIMENSION_2D)
            {
                return true;
            }

            // The "Star case" from the Muratori lecture
            // If closer to the first edge normal
            if (neko_vec3_dot(edge1_normal, new_point_to_origin) > 0.0f)
            {
                // If new search direction should be along edge normal
                if (neko_vec3_dot(edge1, new_point_to_origin) > 0.0f)
                {
                    // Return it as [A, B]
                    *search_dir = neko_vec3_triple_cross_product(edge1, new_point_to_origin, edge1);
                    neko_gjk_simplex_clear(simplex);
                    neko_gjk_simplex_set2(simplex, simplex->a, simplex->b);
                    return false;
                }
                else // If new search direction should be along the new Simplex point
                {
                    // Return new simplex point alone [A]
                    *search_dir = new_point_to_origin;
                    neko_gjk_simplex_clear(simplex);
                    neko_gjk_simplex_set1(simplex, simplex->a);
                    return false;
                }
            }
            else
            {
                // Check if it is above the triangle
                if (neko_vec3_dot(triangle_normal, new_point_to_origin) > 0.0f)
                {
                    // No need to change the simplex, return as [A, B, C]
                    *search_dir = triangle_normal;
                    return false;
                }
                else // Has to be below the triangle, all 4 other possible regions checked
                {
                    // Return simplex as [A, C, B]
                    *search_dir = neko_vec3_neg(triangle_normal);
                    neko_gjk_simplex_set3(simplex, simplex->a, simplex->c, simplex->b);
                    return false;
                }
            }
        }
    }
    // Tetrahedron for 3D case
    else if (simplex->ct == 4)
    {
        // the simplex is a tetrahedron, must check if it is outside any of the side triangles,
        // if it is then set the simplex equal to that triangle and continue, otherwise we know
        // there is an intersection and exit

        neko_vec3 edge1 = neko_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        neko_vec3 edge2 = neko_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        neko_vec3 edge3 = neko_vec3_sub(simplex->d.minkowski_hull_vert, simplex->a.minkowski_hull_vert);

        neko_vec3 face1_normal = neko_vec3_cross(edge1, edge2);
        neko_vec3 face2_normal = neko_vec3_cross(edge2, edge3);
        neko_vec3 face3_normal = neko_vec3_cross(edge3, edge1);

        neko_vec3 new_point_to_origin = neko_vec3_neg(simplex->a.minkowski_hull_vert);

        bool contains = true;
        if (neko_vec3_dot(face1_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is in front of first face, simplex is correct already
            // goto tag;
            contains = false;
        }

        if (!contains && neko_vec3_dot(face2_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is in front of second face, simplex is set to this triangle [A, C, D]
            neko_gjk_simplex_clear(simplex);
            neko_gjk_simplex_set3(simplex, simplex->a, simplex->c, simplex->d);
            contains = false;
        }

        if (!contains && neko_vec3_dot(face3_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is in front of third face, simplex is set to this triangle [A, D, B]
            neko_gjk_simplex_clear(simplex);
            neko_gjk_simplex_set3(simplex, simplex->a, simplex->d, simplex->b);
            contains = false;
        }

        // If reached here it means the simplex MUST contain the origin, intersection confirmed
        if (contains) {
            return true;
        }

        neko_vec3 edge1_normal = neko_vec3_cross(edge1, face1_normal);
        if (neko_vec3_dot(edge1_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is along the normal of edge1, set the simplex to that edge [A, B]
            *search_dir = neko_vec3_triple_cross_product(edge1, new_point_to_origin, edge1);
            neko_gjk_simplex_clear(simplex);
            neko_gjk_simplex_set2(simplex, simplex->a, simplex->b);
            return false;
        }

        neko_vec3 edge2Normal = neko_vec3_cross(face1_normal, edge2);
        if (neko_vec3_dot(edge2Normal, new_point_to_origin) > 0.0f)
        {
            // Origin is along the normal of edge2, set the simplex to that edge [A, C]
            *search_dir = neko_vec3_triple_cross_product(edge2, new_point_to_origin, edge2);
            neko_gjk_simplex_clear(simplex);
            neko_gjk_simplex_set2(simplex, simplex->a, simplex->c);
            return false;
        }

        // If reached here the origin is along the first face normal, set the simplex to this face [A, B, C]
        *search_dir = face1_normal;
        neko_gjk_simplex_clear(simplex);
        neko_gjk_simplex_set3(simplex, simplex->a, simplex->b, simplex->c);
        return false;
    }
    return false;
}


// Find barycentric coordinates of triangle with respect to p
NEKO_API_DECL void neko_gjk_bary(neko_vec3 p, neko_vec3 a, neko_vec3 b, neko_vec3 c, float* u, float* v, float* w)
{
    neko_vec3 v0 = neko_vec3_sub(b, a), v1 = neko_vec3_sub(c, a), v2 = neko_vec3_sub(p, a);
    float d00 = neko_vec3_dot(v0, v0);
    float d01 = neko_vec3_dot(v0, v1);
    float d11 = neko_vec3_dot(v1, v1);
    float d20 = neko_vec3_dot(v2, v0);
    float d21 = neko_vec3_dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    *v = (d11 * d20 - d01 * d21) / denom;
    *w = (d00 * d21 - d01 * d20) / denom;
    *u = 1.0f - *v - *w;
}

//Expanding Polytope Algorithm
NEKO_API_DECL neko_contact_info_t neko_gjk_epa(
    const neko_gjk_simplex_t* simplex,
    const neko_gjk_collider_info_t* ci0,
    const neko_gjk_collider_info_t* ci1
)
{
    neko_contact_info_t res = {0};

    // Cache pointers for collider 0
    void* c0 = ci0->collider;
    neko_gjk_support_func_t f0 = ci0->func;
    neko_phys_xform_t* xform_0 = ci0->xform;

    // Cache pointers for collider 1
    void* c1 = ci1->collider;
    neko_gjk_support_func_t f1 = ci1->func;
    neko_phys_xform_t* xform_1 = ci1->xform;

    // Array of polytope faces, each with 3 support points and a normal
    neko_gjk_polytope_face_t faces[NEKO_EPA_MAX_NUM_FACES] = {0};
    neko_vec3 bma = neko_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    neko_vec3 cma = neko_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    neko_vec3 dma = neko_vec3_sub(simplex->d.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    neko_vec3 cmb = neko_vec3_sub(simplex->c.minkowski_hull_vert, simplex->b.minkowski_hull_vert);
    neko_vec3 dmb = neko_vec3_sub(simplex->d.minkowski_hull_vert, simplex->b.minkowski_hull_vert);

    faces[0] = neko_ctor(neko_gjk_polytope_face_t, {simplex->a, simplex->b, simplex->c}, neko_vec3_norm(neko_vec3_cross(bma, cma)));    // ABC
    faces[1] = neko_ctor(neko_gjk_polytope_face_t, {simplex->a, simplex->c, simplex->d}, neko_vec3_norm(neko_vec3_cross(cma, dma)));    // ACD
    faces[2] = neko_ctor(neko_gjk_polytope_face_t, {simplex->a, simplex->d, simplex->b}, neko_vec3_norm(neko_vec3_cross(dma, bma)));    // ADB
    faces[3] = neko_ctor(neko_gjk_polytope_face_t, {simplex->b, simplex->d, simplex->c}, neko_vec3_norm(neko_vec3_cross(dmb, cmb)));    // BDC

    s32 num_faces = 4;
    s32 closest_face;

    for (s32 iterations = 0; iterations < NEKO_EPA_MAX_NUM_ITERATIONS; ++iterations)
    {
        //Find face that's closest to origin
        float min_dist = neko_vec3_dot(faces[0].points[0].minkowski_hull_vert, faces[0].normal);
        closest_face = 0;
        for (s32 i = 1; i < num_faces; ++i)
        {
            float dist = neko_vec3_dot(faces[i].points[0].minkowski_hull_vert, faces[i].normal);
            if (dist < min_dist)
            {
                min_dist = dist;
                closest_face = i;
            }
        }

        // Search normal to face that's closest to origin
        neko_vec3 search_dir = faces[closest_face].normal;
        neko_gjk_support_point_t p = neko_gjk_generate_support_point(ci0, ci1, search_dir);
        float depth = neko_vec3_dot(p.minkowski_hull_vert, search_dir);

        // Within tolerance, so extract contact information from hull
        if (depth - min_dist < NEKO_EPA_TOLERANCE)
        {
            // Cache local pointers
            neko_gjk_polytope_face_t* f = &faces[closest_face];
            neko_vec3* n = &f->normal;
            neko_gjk_support_point_t* sp0 = &f->points[0];
            neko_gjk_support_point_t* sp1 = &f->points[1];
            neko_gjk_support_point_t* sp2 = &f->points[2];
            neko_vec3* p0 = &sp0->minkowski_hull_vert;
            neko_vec3* p1 = &sp1->minkowski_hull_vert;
            neko_vec3* p2 = &sp2->minkowski_hull_vert;

            // Normal and depth information
            res.hit = true;
            res.normal = neko_vec3_norm(*n);
            res.depth = depth;

            // Get barycentric coordinates of resulting triangle face
            float u = 0.f, v = 0.f, w = 0.f;
            neko_gjk_bary(*n, *p0, *p1, *p2, &u, &v, &w);

            neko_vec3* support_0, *support_1, *support_2;

            // A Contact points
            support_0 = &sp0->support_a;
            support_1 = &sp1->support_a;
            support_2 = &sp2->support_a;

            // Contact point on collider a
            res.points[0] = neko_vec3_add(neko_vec3_add(neko_vec3_scale(*support_0, u), neko_vec3_scale(*support_1, v)), neko_vec3_scale(*support_2, w));

            // B Contact points
            support_0 = &sp0->support_b;
            support_1 = &sp1->support_b;
            support_2 = &sp2->support_b;

            // Contact point on collider b
            res.points[1] = neko_vec3_add(neko_vec3_add(neko_vec3_scale(*support_0, u), neko_vec3_scale(*support_1, v)), neko_vec3_scale(*support_2, w));

            return res;
        }

        // Fix Edges
        neko_gjk_support_point_t loose_edges[NEKO_EPA_MAX_NUM_LOOSE_EDGES][2];
        s32 num_loose_edges = 0;

        // Find all triangles that are facing p
        for (s32 i = 0; i < num_faces; ++i)
        {
            // Grab direction from first point on face at p to i
            neko_vec3 dir = neko_vec3_sub(p.minkowski_hull_vert, faces[i].points[0].minkowski_hull_vert);

            // Triangle i faces p, remove it
            if (neko_vec3_same_dir(faces[i].normal, dir))
            {
                // Add removed triangle's edges to loose edge list.
                // If it's already there, remove it (both triangles it belonged to are gone)
                for (s32 j = 0; j < 3; ++j)
                {
                    neko_gjk_support_point_t current_edge[2] = {faces[i].points[j], faces[i].points[(j + 1) % 3]};
                    bool found_edge = false;
                    for (s32 k = 0; k < num_loose_edges; ++k) //Check if current edge is already in list
                    {
                        //Edge is already in the list, remove it
                        if (neko_vec3_eq(loose_edges[k][1].minkowski_hull_vert, current_edge[0].minkowski_hull_vert)
                                && neko_vec3_eq(loose_edges[k][0].minkowski_hull_vert, current_edge[1].minkowski_hull_vert))
                        {
                            // Overwrite current edge with last edge in list
                            loose_edges[k][0] = loose_edges[num_loose_edges - 1][0];
                            loose_edges[k][1] = loose_edges[num_loose_edges - 1][1];
                            num_loose_edges--;

                             // Exit loop because edge can only be shared once
                            found_edge = true;
                            k = num_loose_edges;
                        }
                    }

                    // Add current edge to list (is unique)
                    if (!found_edge)
                    {
                        if (num_loose_edges >= NEKO_EPA_MAX_NUM_LOOSE_EDGES) break;
                        loose_edges[num_loose_edges][0] = current_edge[0];
                        loose_edges[num_loose_edges][1] = current_edge[1];
                        num_loose_edges++;
                    }
                }

                // Remove triangle i from list
                faces[i].points[0] = faces[num_faces - 1].points[0];
                faces[i].points[1] = faces[num_faces - 1].points[1];
                faces[i].points[2] = faces[num_faces - 1].points[2];
                faces[i].normal = faces[num_faces - 1].normal;
                num_faces--;
                i--;
            }
        }

        // Reconstruct polytope with p now added
        for (s32 i = 0; i < num_loose_edges; ++i)
        {
            if (num_faces >= NEKO_EPA_MAX_NUM_FACES) break;
            faces[num_faces].points[0] = loose_edges[i][0];
            faces[num_faces].points[1] = loose_edges[i][1];
            faces[num_faces].points[2] = p;
            neko_vec3 zmo = neko_vec3_sub(loose_edges[i][0].minkowski_hull_vert, loose_edges[i][1].minkowski_hull_vert);
            neko_vec3 zmp = neko_vec3_sub(loose_edges[i][0].minkowski_hull_vert, p.minkowski_hull_vert);
            faces[num_faces].normal = neko_vec3_norm(neko_vec3_cross(zmo, zmp));

            //Check for wrong normal to maintain CCW winding
            // In case dot result is only slightly < 0 (because origin is on face)
            float bias = 0.000001;
            if (neko_vec3_dot(faces[num_faces].points[0].minkowski_hull_vert, faces[num_faces].normal) + bias < 0.f){
                neko_gjk_support_point_t temp = faces[num_faces].points[0];
                faces[num_faces].points[0] = faces[num_faces].points[1];
                faces[num_faces].points[1] = temp;
                faces[num_faces].normal = neko_vec3_scale(faces[num_faces].normal, -1.f);
            }
            num_faces++;
        }
    }

    // Return most recent closest point
    float dot = neko_vec3_dot(faces[closest_face].points[0].minkowski_hull_vert, faces[closest_face].normal);
    neko_vec3 norm = neko_vec3_scale(faces[closest_face].normal, dot);
    res.hit = false;
    res.normal = neko_vec3_norm(norm);
    res.depth = neko_vec3_len(norm);
    return res;
}

// Expanding Polytope Algorithm 2D
NEKO_API_DECL neko_contact_info_t neko_gjk_epa_2d(
    const neko_gjk_simplex_t* simplex,
    const neko_gjk_collider_info_t* ci0,
    const neko_gjk_collider_info_t* ci1
)
{
    neko_dyn_array(neko_gjk_support_point_t) polytope = NULL;
    neko_contact_info_t res = neko_default_val();
    neko_gjk_support_point_t p = neko_default_val();

    // Copy over simplex into polytope array
    for (u32 i = 0; i < simplex->ct; ++i) {
        neko_dyn_array_push(polytope, simplex->points[i]);
    }

    // p = neko_gjk_generate_support_point(ci0, ci1, neko_v3s(1.f));
    // neko_gjk_simplex_push_front(simplex, p);
    neko_gjk_epa_edge_t e = neko_default_val();
    u32 iterations = 0;
    while (iterations < NEKO_EPA_MAX_NUM_ITERATIONS)
    {
        // Find closest edge to origin from simplex
        e = neko_gjk_epa_find_closest_edge(polytope);
        p = neko_gjk_generate_support_point(ci0, ci1, e.normal);

        double d = (double)neko_vec3_dot(p.minkowski_hull_vert, e.normal);

        // Success, calculate results
        if (d - e.distance < NEKO_EPA_TOLERANCE)
        {
            // Normal and depth information
            res.normal = neko_vec3_norm(e.normal);
            res.depth = d;
            break;
        }
        else
        {
            // Need to think about this more. This is fucked.
            // Need an "expanding" simplex, that only allows for unique points to be included.
            // This is just overwriting. I need to actually insert then push back.
            // neko_gjk_simplex_insert(simplex, e.index + 1, p);
            // Insert into polytope array at idx + 1
            for (u32 i = 0; i < neko_dyn_array_size(polytope); ++i)
            {
               neko_vec3* p = &polytope[i].minkowski_hull_vert;
               neko_printf("<%.2f, %.2f>, ", p->x, p->y);
            }
            neko_dyn_array_push(polytope, p);

            for (s32 i = neko_dyn_array_size(polytope) - 1; i > e.index + 1; i--)
                polytope[i] = polytope[i - 1];

            polytope[e.index + 1] = p;

            neko_println("pts after: ");
            for (u32 i = 0; i < neko_dyn_array_size(polytope); ++i)
            {
               neko_vec3* p = &polytope[i].minkowski_hull_vert;
               neko_printf("<%.2f, %.2f>, ", p->x, p->y);
            }
        }

        // Increment iterations
        iterations++;
    }

    // neko_vec3 line_vec = neko_vec3_sub(e.a.minkowski_hull_vert, e.b.minkowski_hull_vert);
    // neko_vec3 projO = neko_vec3_scale(neko_vec3_scale(line_vec, 1.f / neko_vec3_len2(line_vec)), neko_vec3_dot(line_vec, neko_vec3_neg(e.b.minkowski_hull_vert)));
    // float s, t;
    // s = sqrt(neko_vec3_len2(projO) / neko_vec3_len2(line_vec));
    // t = 1.f - s;
    // s32 next_idx = (e.index + 1) % simplex->ct;
    res.hit = true;
    // res.points[0] = neko_vec3_add(neko_vec3_scale(simplex->points[e.index].support_a, s), neko_vec3_scale(simplex->points[next_idx].support_a, t));
    // res.points[1] = neko_vec3_add(neko_vec3_scale(simplex->points[e.index].support_b, s), neko_vec3_scale(simplex->points[next_idx].support_b, t));

    return res;

}

neko_gjk_epa_edge_t neko_gjk_epa_find_closest_edge(neko_dyn_array(neko_gjk_support_point_t) polytope)
{
   neko_gjk_epa_edge_t res = neko_default_val();
   float min_dist = FLT_MAX;
   u32 min_index = 0;
   neko_vec3 min_normal = neko_default_val();
   for (u32 i = 0; i < neko_dyn_array_size(polytope); ++i)
   {
        u32 j = (i + 1) % neko_dyn_array_size(polytope);
        neko_gjk_support_point_t* a = &polytope[0];
        neko_gjk_support_point_t* b = &polytope[1];
        neko_vec3 dir = neko_vec3_sub(b->minkowski_hull_vert, a->minkowski_hull_vert);
        neko_vec3 norm = neko_vec3_norm(neko_v3(dir.y, -dir.x, 0.f));
        float dist = neko_vec3_dot(norm, a->minkowski_hull_vert);

        // If distance is negative, then we need to correct for winding order mistakes
        if (dist < 0) {
            dist *= -1;
            norm = neko_vec3_neg(norm);
        }

        // Check for min distance
        if (dist < min_dist) {
            min_dist = dist;
            min_normal = norm;
            min_index = j;
        }
   }
   res.index = min_index;
   res.normal = min_normal;
   res.distance = min_dist;
   return res;
}

neko_gjk_epa_edge_t neko_gjk_epa_find_closest_edge2(neko_gjk_simplex_t* simplex)
{
    neko_gjk_epa_edge_t result = neko_default_val();
    u32 next_idx = 0;
    float min_dist = FLT_MAX, curr_dist = 0.f;
    neko_vec3 norm, edge;
    neko_vec3 norm_3d;

    for (s32 i = 0; i < simplex->ct; ++i)
    {
        next_idx = (i + 1) % simplex->ct;
        edge = neko_vec3_sub(simplex->points[next_idx].minkowski_hull_vert, simplex->points[i].minkowski_hull_vert);
        norm_3d = neko_vec3_triple_cross_product(neko_v3(edge.x, edge.y, 0),
            neko_v3(simplex->points[i].minkowski_hull_vert.x, simplex->points[i].minkowski_hull_vert.y, 0.f),
            neko_v3(edge.x, edge.y, 0.f));
        norm.x = norm_3d.x;
        norm.y = norm_3d.y;

        norm = neko_vec3_norm(norm);
        curr_dist = neko_vec3_dot(norm, simplex->points[i].minkowski_hull_vert);
        if (curr_dist < min_dist)
        {
            min_dist = curr_dist;
            result.a = simplex->points[i];
            result.b = simplex->points[next_idx];
            result.normal = norm;
            result.distance = curr_dist;
            result.index = i;
        }
    }

    return result;
}
*/

/*===== CCD ======*/

#ifndef NEKO_PHYSICS_NO_CCD

// Useful CCD conversions
void _neko_ccdv32gsv3(const ccd_vec3_t* _in, neko_vec3* _out) {
    // Safe check against NaNs
    if (neko_is_nan(_in->v[0]) || neko_is_nan(_in->v[1]) || neko_is_nan(_in->v[2])) return;
    *_out = neko_ctor(neko_vec3, (float)_in->v[0], (float)_in->v[1], (float)_in->v[2]);
}

void _neko_gsv32ccdv3(const neko_vec3* _in, ccd_vec3_t* _out) { ccdVec3Set(_out, CCD_REAL(_in->x), CCD_REAL(_in->y), CCD_REAL(_in->z)); }

NEKO_API_PRIVATE s32 _neko_ccd_gjk_internal(const void* c0, const neko_vqs* xform_a, neko_support_func_t f0, const void* c1, const neko_vqs* xform_b, neko_support_func_t f1, neko_contact_info_t* res

) {
    // Convert to appropriate gjk internals, then call ccd
    ccd_t ccd = neko_default_val();
    CCD_INIT(&ccd);

    // set up ccd_t struct
    ccd.support1 = _neko_ccd_support_func;  // support function for first object
    ccd.support2 = _neko_ccd_support_func;  // support function for second object
    ccd.max_iterations = 100;               // maximal number of iterations
    ccd.epa_tolerance = 0.0001;             // maximal tolerance for epa to succeed

    // Default transforms
    neko_vqs _xa = neko_vqs_default(), _xb = neko_vqs_default();

    // Collision object 1
    _neko_collision_obj_handle_t h0 = neko_default_val();
    h0.support = f0;
    h0.obj = c0;
    h0.xform = xform_a ? xform_a : &_xa;

    // Collision object 2
    _neko_collision_obj_handle_t h1 = neko_default_val();
    h1.support = f1;
    h1.obj = c1;
    h1.xform = xform_b ? xform_b : &_xb;

    // Call ccd, cache results into res for user
    ccd_real_t depth = CCD_REAL(0.0);
    ccd_vec3_t n = neko_ctor(ccd_vec3_t, 0.f, 0.f, 0.f), p = neko_ctor(ccd_vec3_t, 0.f, 0.f, 0.f);
    s32 r = ccdGJKPenetration(&h0, &h1, &ccd, &depth, &n, &p);
    b32 hit = r >= 0 && !neko_is_nan(n.v[0]) && !neko_is_nan(n.v[1]) && !neko_is_nan(n.v[2]);

    if (hit && res) {
        res->hit = true;
        res->depth = (float)depth;
        _neko_ccdv32gsv3(&p, &res->point);
        _neko_ccdv32gsv3(&n, &res->normal);
    }

    return r;
}

// typedef void (*ccd_support_fn)(const void *obj, const ccd_vec3_t *dir,
//                                ccd_vec3_t *vec);

// Internal support function for neko -> ccd
NEKO_API_DECL void _neko_ccd_support_func(const void* _obj, const ccd_vec3_t* _dir, ccd_vec3_t* _out) {
    const _neko_collision_obj_handle_t* obj = (const _neko_collision_obj_handle_t*)_obj;
    if (obj->support) {
        // Temp copy conversion for direction vector
        neko_vec3 dir = neko_default_val(), out = neko_default_val();
        _neko_ccdv32gsv3((const ccd_vec3_t*)_dir, &dir);

        // Call user provided support function
        // Think I found it...
        obj->support(obj->obj, obj->xform, &dir, &out);

        // Copy over out result for ccd
        _neko_gsv32ccdv3(&out, (ccd_vec3_t*)_out);
    }
}

#endif
