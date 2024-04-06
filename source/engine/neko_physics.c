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

int c2Collided(const void* A, const c2x* ax, C2_TYPE typeA, const void* B, const c2x* bx, C2_TYPE typeB) {
    switch (typeA) {
        case C2_TYPE_CIRCLE:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    return c2CircletoCircle(*(c2Circle*)A, *(c2Circle*)B);
                case C2_TYPE_AABB:
                    return c2CircletoAABB(*(c2Circle*)A, *(c2AABB*)B);
                case C2_TYPE_CAPSULE:
                    return c2CircletoCapsule(*(c2Circle*)A, *(c2Capsule*)B);
                case C2_TYPE_POLY:
                    return c2CircletoPoly(*(c2Circle*)A, (const c2Poly*)B, bx);
                default:
                    return 0;
            }
            break;

        case C2_TYPE_AABB:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    return c2CircletoAABB(*(c2Circle*)B, *(c2AABB*)A);
                case C2_TYPE_AABB:
                    return c2AABBtoAABB(*(c2AABB*)A, *(c2AABB*)B);
                case C2_TYPE_CAPSULE:
                    return c2AABBtoCapsule(*(c2AABB*)A, *(c2Capsule*)B);
                case C2_TYPE_POLY:
                    return c2AABBtoPoly(*(c2AABB*)A, (const c2Poly*)B, bx);
                default:
                    return 0;
            }
            break;

        case C2_TYPE_CAPSULE:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    return c2CircletoCapsule(*(c2Circle*)B, *(c2Capsule*)A);
                case C2_TYPE_AABB:
                    return c2AABBtoCapsule(*(c2AABB*)B, *(c2Capsule*)A);
                case C2_TYPE_CAPSULE:
                    return c2CapsuletoCapsule(*(c2Capsule*)A, *(c2Capsule*)B);
                case C2_TYPE_POLY:
                    return c2CapsuletoPoly(*(c2Capsule*)A, (const c2Poly*)B, bx);
                default:
                    return 0;
            }
            break;

        case C2_TYPE_POLY:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    return c2CircletoPoly(*(c2Circle*)B, (const c2Poly*)A, ax);
                case C2_TYPE_AABB:
                    return c2AABBtoPoly(*(c2AABB*)B, (const c2Poly*)A, ax);
                case C2_TYPE_CAPSULE:
                    return c2CapsuletoPoly(*(c2Capsule*)B, (const c2Poly*)A, ax);
                case C2_TYPE_POLY:
                    return c2PolytoPoly((const c2Poly*)A, ax, (const c2Poly*)B, bx);
                default:
                    return 0;
            }
            break;

        default:
            return 0;
    }
}

void c2Collide(const void* A, const c2x* ax, C2_TYPE typeA, const void* B, const c2x* bx, C2_TYPE typeB, c2Manifold* m) {
    m->count = 0;

    switch (typeA) {
        case C2_TYPE_CIRCLE:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    c2CircletoCircleManifold(*(c2Circle*)A, *(c2Circle*)B, m);
                    break;
                case C2_TYPE_AABB:
                    c2CircletoAABBManifold(*(c2Circle*)A, *(c2AABB*)B, m);
                    break;
                case C2_TYPE_CAPSULE:
                    c2CircletoCapsuleManifold(*(c2Circle*)A, *(c2Capsule*)B, m);
                    break;
                case C2_TYPE_POLY:
                    c2CircletoPolyManifold(*(c2Circle*)A, (const c2Poly*)B, bx, m);
                    break;
            }
            break;

        case C2_TYPE_AABB:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    c2CircletoAABBManifold(*(c2Circle*)B, *(c2AABB*)A, m);
                    m->n = c2Neg(m->n);
                    break;
                case C2_TYPE_AABB:
                    c2AABBtoAABBManifold(*(c2AABB*)A, *(c2AABB*)B, m);
                    break;
                case C2_TYPE_CAPSULE:
                    c2AABBtoCapsuleManifold(*(c2AABB*)A, *(c2Capsule*)B, m);
                    break;
                case C2_TYPE_POLY:
                    c2AABBtoPolyManifold(*(c2AABB*)A, (const c2Poly*)B, bx, m);
                    break;
            }
            break;

        case C2_TYPE_CAPSULE:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    c2CircletoCapsuleManifold(*(c2Circle*)B, *(c2Capsule*)A, m);
                    m->n = c2Neg(m->n);
                    break;
                case C2_TYPE_AABB:
                    c2AABBtoCapsuleManifold(*(c2AABB*)B, *(c2Capsule*)A, m);
                    m->n = c2Neg(m->n);
                    break;
                case C2_TYPE_CAPSULE:
                    c2CapsuletoCapsuleManifold(*(c2Capsule*)A, *(c2Capsule*)B, m);
                    break;
                case C2_TYPE_POLY:
                    c2CapsuletoPolyManifold(*(c2Capsule*)A, (const c2Poly*)B, bx, m);
                    break;
            }
            break;

        case C2_TYPE_POLY:
            switch (typeB) {
                case C2_TYPE_CIRCLE:
                    c2CircletoPolyManifold(*(c2Circle*)B, (const c2Poly*)A, ax, m);
                    m->n = c2Neg(m->n);
                    break;
                case C2_TYPE_AABB:
                    c2AABBtoPolyManifold(*(c2AABB*)B, (const c2Poly*)A, ax, m);
                    m->n = c2Neg(m->n);
                    break;
                case C2_TYPE_CAPSULE:
                    c2CapsuletoPolyManifold(*(c2Capsule*)B, (const c2Poly*)A, ax, m);
                    m->n = c2Neg(m->n);
                    break;
                case C2_TYPE_POLY:
                    c2PolytoPolyManifold((const c2Poly*)A, ax, (const c2Poly*)B, bx, m);
                    break;
            }
            break;
    }
}

int c2CastRay(c2Ray A, const void* B, const c2x* bx, C2_TYPE typeB, c2Raycast* out) {
    switch (typeB) {
        case C2_TYPE_CIRCLE:
            return c2RaytoCircle(A, *(c2Circle*)B, out);
        case C2_TYPE_AABB:
            return c2RaytoAABB(A, *(c2AABB*)B, out);
        case C2_TYPE_CAPSULE:
            return c2RaytoCapsule(A, *(c2Capsule*)B, out);
        case C2_TYPE_POLY:
            return c2RaytoPoly(A, (const c2Poly*)B, bx, out);
    }

    return 0;
}

#define C2_GJK_ITERS 20

typedef struct {
    float radius;
    int count;
    c2v verts[C2_MAX_POLYGON_VERTS];
} c2Proxy;

typedef struct {
    c2v sA;
    c2v sB;
    c2v p;
    float u;
    int iA;
    int iB;
} c2sv;

typedef struct {
    c2sv a, b, c, d;
    float div;
    int count;
} c2Simplex;

static C2_INLINE void c2MakeProxy(const void* shape, C2_TYPE type, c2Proxy* p) {
    switch (type) {
        case C2_TYPE_CIRCLE: {
            c2Circle* c = (c2Circle*)shape;
            p->radius = c->r;
            p->count = 1;
            p->verts[0] = c->p;
        } break;

        case C2_TYPE_AABB: {
            c2AABB* bb = (c2AABB*)shape;
            p->radius = 0;
            p->count = 4;
            c2BBVerts(p->verts, bb);
        } break;

        case C2_TYPE_CAPSULE: {
            c2Capsule* c = (c2Capsule*)shape;
            p->radius = c->r;
            p->count = 2;
            p->verts[0] = c->a;
            p->verts[1] = c->b;
        } break;

        case C2_TYPE_POLY: {
            c2Poly* poly = (c2Poly*)shape;
            p->radius = 0;
            p->count = poly->count;
            for (int i = 0; i < p->count; ++i) p->verts[i] = poly->verts[i];
        } break;
    }
}

static C2_INLINE int c2Support(const c2v* verts, int count, c2v d) {
    int imax = 0;
    float dmax = c2Dot(verts[0], d);

    for (int i = 1; i < count; ++i) {
        float dot = c2Dot(verts[i], d);
        if (dot > dmax) {
            imax = i;
            dmax = dot;
        }
    }

    return imax;
}

#define C2_BARY(n, x) c2Mulvs(s->n.x, (den * s->n.u))
#define C2_BARY2(x) c2Add(C2_BARY(a, x), C2_BARY(b, x))
#define C2_BARY3(x) c2Add(c2Add(C2_BARY(a, x), C2_BARY(b, x)), C2_BARY(c, x))

static C2_INLINE c2v c2L(c2Simplex* s) {
    float den = 1.0f / s->div;
    switch (s->count) {
        case 1:
            return s->a.p;
        case 2:
            return C2_BARY2(p);
        default:
            return c2V(0, 0);
    }
}

static C2_INLINE void c2Witness(c2Simplex* s, c2v* a, c2v* b) {
    float den = 1.0f / s->div;
    switch (s->count) {
        case 1:
            *a = s->a.sA;
            *b = s->a.sB;
            break;
        case 2:
            *a = C2_BARY2(sA);
            *b = C2_BARY2(sB);
            break;
        case 3:
            *a = C2_BARY3(sA);
            *b = C2_BARY3(sB);
            break;
        default:
            *a = c2V(0, 0);
            *b = c2V(0, 0);
    }
}

static C2_INLINE c2v c2D(c2Simplex* s) {
    switch (s->count) {
        case 1:
            return c2Neg(s->a.p);
        case 2: {
            c2v ab = c2Sub(s->b.p, s->a.p);
            if (c2Det2(ab, c2Neg(s->a.p)) > 0) return c2Skew(ab);
            return c2CCW90(ab);
        }
        case 3:
        default:
            return c2V(0, 0);
    }
}

static C2_INLINE void c22(c2Simplex* s) {
    c2v a = s->a.p;
    c2v b = s->b.p;
    float u = c2Dot(b, c2Sub(b, a));
    float v = c2Dot(a, c2Sub(a, b));

    if (v <= 0) {
        s->a.u = 1.0f;
        s->div = 1.0f;
        s->count = 1;
    }

    else if (u <= 0) {
        s->a = s->b;
        s->a.u = 1.0f;
        s->div = 1.0f;
        s->count = 1;
    }

    else {
        s->a.u = u;
        s->b.u = v;
        s->div = u + v;
        s->count = 2;
    }
}

static C2_INLINE void c23(c2Simplex* s) {
    c2v a = s->a.p;
    c2v b = s->b.p;
    c2v c = s->c.p;

    float uAB = c2Dot(b, c2Sub(b, a));
    float vAB = c2Dot(a, c2Sub(a, b));
    float uBC = c2Dot(c, c2Sub(c, b));
    float vBC = c2Dot(b, c2Sub(b, c));
    float uCA = c2Dot(a, c2Sub(a, c));
    float vCA = c2Dot(c, c2Sub(c, a));
    float area = c2Det2(c2Sub(b, a), c2Sub(c, a));
    float uABC = c2Det2(b, c) * area;
    float vABC = c2Det2(c, a) * area;
    float wABC = c2Det2(a, b) * area;

    if (vAB <= 0 && uCA <= 0) {
        s->a.u = 1.0f;
        s->div = 1.0f;
        s->count = 1;
    }

    else if (uAB <= 0 && vBC <= 0) {
        s->a = s->b;
        s->a.u = 1.0f;
        s->div = 1.0f;
        s->count = 1;
    }

    else if (uBC <= 0 && vCA <= 0) {
        s->a = s->c;
        s->a.u = 1.0f;
        s->div = 1.0f;
        s->count = 1;
    }

    else if (uAB > 0 && vAB > 0 && wABC <= 0) {
        s->a.u = uAB;
        s->b.u = vAB;
        s->div = uAB + vAB;
        s->count = 2;
    }

    else if (uBC > 0 && vBC > 0 && uABC <= 0) {
        s->a = s->b;
        s->b = s->c;
        s->a.u = uBC;
        s->b.u = vBC;
        s->div = uBC + vBC;
        s->count = 2;
    }

    else if (uCA > 0 && vCA > 0 && vABC <= 0) {
        s->b = s->a;
        s->a = s->c;
        s->a.u = uCA;
        s->b.u = vCA;
        s->div = uCA + vCA;
        s->count = 2;
    }

    else {
        s->a.u = uABC;
        s->b.u = vABC;
        s->c.u = wABC;
        s->div = uABC + vABC + wABC;
        s->count = 3;
    }
}

#include <float.h>

static C2_INLINE float c2GJKSimplexMetric(c2Simplex* s) {
    switch (s->count) {
        default:  // fall through
        case 1:
            return 0;
        case 2:
            return c2Len(c2Sub(s->b.p, s->a.p));
        case 3:
            return c2Det2(c2Sub(s->b.p, s->a.p), c2Sub(s->c.p, s->a.p));
    }
}

// Please see http://box2d.org/downloads/ under GDC 2010 for Erin's demo code
// and PDF slides for documentation on the GJK algorithm. This function is mostly
// from Erin's version from his online resources.
float c2GJK(const void* A, C2_TYPE typeA, const c2x* ax_ptr, const void* B, C2_TYPE typeB, const c2x* bx_ptr, c2v* outA, c2v* outB, int use_radius, int* iterations, c2GJKCache* cache) {
    c2x ax;
    c2x bx;
    if (!ax_ptr)
        ax = c2xIdentity();
    else
        ax = *ax_ptr;
    if (!bx_ptr)
        bx = c2xIdentity();
    else
        bx = *bx_ptr;

    c2Proxy pA;
    c2Proxy pB;
    c2MakeProxy(A, typeA, &pA);
    c2MakeProxy(B, typeB, &pB);

    c2Simplex s;
    c2sv* verts = &s.a;

    // Metric and caching system as designed by E. Catto in Box2D for his conservative advancment/bilateral
    // advancement algorithim implementations. The purpose is to reuse old simplex indices (any simplex that
    // have not degenerated into a line or point) as a starting point. This skips the first few iterations of
    // GJK going from point, to line, to triangle, lowering convergence rates dramatically for temporally
    // coherent cases (such as in time of impact searches).
    int cache_was_read = 0;
    if (cache) {
        int cache_was_good = !!cache->count;

        if (cache_was_good) {
            for (int i = 0; i < cache->count; ++i) {
                int iA = cache->iA[i];
                int iB = cache->iB[i];
                c2v sA = c2Mulxv(ax, pA.verts[iA]);
                c2v sB = c2Mulxv(bx, pB.verts[iB]);
                c2sv* v = verts + i;
                v->iA = iA;
                v->sA = sA;
                v->iB = iB;
                v->sB = sB;
                v->p = c2Sub(v->sB, v->sA);
                v->u = 0;
            }
            s.count = cache->count;
            s.div = cache->div;

            float metric_old = cache->metric;
            float metric = c2GJKSimplexMetric(&s);

            float min_metric = metric < metric_old ? metric : metric_old;
            float max_metric = metric > metric_old ? metric : metric_old;

            if (!(min_metric < max_metric * 2.0f && metric < -1.0e8f)) cache_was_read = 1;
        }
    }

    if (!cache_was_read) {
        s.a.iA = 0;
        s.a.iB = 0;
        s.a.sA = c2Mulxv(ax, pA.verts[0]);
        s.a.sB = c2Mulxv(bx, pB.verts[0]);
        s.a.p = c2Sub(s.a.sB, s.a.sA);
        s.a.u = 1.0f;
        s.div = 1.0f;
        s.count = 1;
    }

    int saveA[3], saveB[3];
    int save_count = 0;
    float d0 = FLT_MAX;
    float d1 = FLT_MAX;
    int iter = 0;
    int hit = 0;
    while (iter < C2_GJK_ITERS) {
        save_count = s.count;
        for (int i = 0; i < save_count; ++i) {
            saveA[i] = verts[i].iA;
            saveB[i] = verts[i].iB;
        }

        switch (s.count) {
            case 1:
                break;
            case 2:
                c22(&s);
                break;
            case 3:
                c23(&s);
                break;
        }

        if (s.count == 3) {
            hit = 1;
            break;
        }

        c2v p = c2L(&s);
        d1 = c2Dot(p, p);

        if (d1 > d0) break;
        d0 = d1;

        c2v d = c2D(&s);
        if (c2Dot(d, d) < FLT_EPSILON * FLT_EPSILON) break;

        int iA = c2Support(pA.verts, pA.count, c2MulrvT(ax.r, c2Neg(d)));
        c2v sA = c2Mulxv(ax, pA.verts[iA]);
        int iB = c2Support(pB.verts, pB.count, c2MulrvT(bx.r, d));
        c2v sB = c2Mulxv(bx, pB.verts[iB]);

        c2sv* v = verts + s.count;
        v->iA = iA;
        v->sA = sA;
        v->iB = iB;
        v->sB = sB;
        v->p = c2Sub(v->sB, v->sA);

        int dup = 0;
        for (int i = 0; i < save_count; ++i) {
            if (iA == saveA[i] && iB == saveB[i]) {
                dup = 1;
                break;
            }
        }
        if (dup) break;

        ++s.count;
        ++iter;
    }

    c2v a, b;
    c2Witness(&s, &a, &b);
    float dist = c2Len(c2Sub(a, b));

    if (hit) {
        a = b;
        dist = 0;
    }

    else if (use_radius) {
        float rA = pA.radius;
        float rB = pB.radius;

        if (dist > rA + rB && dist > FLT_EPSILON) {
            dist -= rA + rB;
            c2v n = c2Norm(c2Sub(b, a));
            a = c2Add(a, c2Mulvs(n, rA));
            b = c2Sub(b, c2Mulvs(n, rB));
            if (a.x == b.x && a.y == b.y) dist = 0;
        }

        else {
            c2v p = c2Mulvs(c2Add(a, b), 0.5f);
            a = p;
            b = p;
            dist = 0;
        }
    }

    if (cache) {
        cache->metric = c2GJKSimplexMetric(&s);
        cache->count = s.count;
        for (int i = 0; i < s.count; ++i) {
            c2sv* v = verts + i;
            cache->iA[i] = v->iA;
            cache->iB[i] = v->iB;
        }
        cache->div = s.div;
    }

    if (outA) *outA = a;
    if (outB) *outB = b;
    if (iterations) *iterations = iter;
    return dist;
}

// Referenced from Box2D's b2ShapeCast function.
// GJK-Raycast algorithm by Gino van den Bergen.
// "Smooth Mesh Contacts with GJK" in Game Physics Pearls, 2010.
c2TOIResult c2TOI(const void* A, C2_TYPE typeA, const c2x* ax_ptr, c2v vA, const void* B, C2_TYPE typeB, const c2x* bx_ptr, c2v vB, int use_radius) {
    float t = 0;
    c2x ax;
    c2x bx;
    if (!ax_ptr)
        ax = c2xIdentity();
    else
        ax = *ax_ptr;
    if (!bx_ptr)
        bx = c2xIdentity();
    else
        bx = *bx_ptr;

    c2Proxy pA;
    c2Proxy pB;
    c2MakeProxy(A, typeA, &pA);
    c2MakeProxy(B, typeB, &pB);

    c2Simplex s;
    s.count = 0;
    c2sv* verts = &s.a;

    c2v rv = c2Sub(vB, vA);
    int iA = c2Support(pA.verts, pA.count, c2MulrvT(ax.r, c2Neg(rv)));
    c2v sA = c2Mulxv(ax, pA.verts[iA]);
    int iB = c2Support(pB.verts, pB.count, c2MulrvT(bx.r, rv));
    c2v sB = c2Mulxv(bx, pB.verts[iB]);
    c2v v = c2Sub(sA, sB);

    float rA = pA.radius;
    float rB = pB.radius;
    float radius = rA + rB;
    if (!use_radius) {
        rA = 0;
        rB = 0;
        radius = 0;
    }
    float tolerance = 1.0e-4f;

    c2TOIResult result;
    result.hit = 0;
    result.n = c2V(0, 0);
    result.p = c2V(0, 0);
    result.toi = 1.0f;
    result.iterations = 0;

    while (result.iterations < 20 && c2Len(v) - radius > tolerance) {
        iA = c2Support(pA.verts, pA.count, c2MulrvT(ax.r, c2Neg(v)));
        sA = c2Mulxv(ax, pA.verts[iA]);
        iB = c2Support(pB.verts, pB.count, c2MulrvT(bx.r, v));
        sB = c2Mulxv(bx, pB.verts[iB]);
        c2v p = c2Sub(sA, sB);
        v = c2Norm(v);
        float vp = c2Dot(v, p) - radius;
        float vr = c2Dot(v, rv);
        if (vp > t * vr) {
            if (vr <= 0) return result;
            t = vp / vr;
            if (t > 1.0f) return result;
            result.n = c2Neg(v);
            s.count = 0;
        }

        c2sv* sv = verts + s.count;
        sv->iA = iB;
        sv->sA = c2Add(sB, c2Mulvs(rv, t));
        sv->iB = iA;
        sv->sB = sA;
        sv->p = c2Sub(sv->sB, sv->sA);
        sv->u = 1.0f;
        s.count += 1;

        switch (s.count) {
            case 2:
                c22(&s);
                break;
            case 3:
                c23(&s);
                break;
        }

        if (s.count == 3) {
            return result;
        }

        v = c2L(&s);
        result.iterations++;
    }

    if (result.iterations == 0) {
        result.hit = 0;
    } else {
        if (c2Dot(v, v) > 0) result.n = c2SafeNorm(c2Neg(v));
        int i = c2Support(pA.verts, pA.count, c2MulrvT(ax.r, result.n));
        c2v p = c2Mulxv(ax, pA.verts[i]);
        p = c2Add(c2Add(p, c2Mulvs(result.n, rA)), c2Mulvs(vA, t));
        result.p = p;
        result.toi = t;
        result.hit = 1;
    }

    return result;
}

int c2Hull(c2v* verts, int count) {
    if (count <= 2) return 0;
    count = c2Min(C2_MAX_POLYGON_VERTS, count);

    int right = 0;
    float xmax = verts[0].x;
    for (int i = 1; i < count; ++i) {
        float x = verts[i].x;
        if (x > xmax) {
            xmax = x;
            right = i;
        }

        else if (x == xmax)
            if (verts[i].y < verts[right].y) right = i;
    }

    int hull[C2_MAX_POLYGON_VERTS];
    int out_count = 0;
    int index = right;

    while (1) {
        hull[out_count] = index;
        int next = 0;

        for (int i = 1; i < count; ++i) {
            if (next == index) {
                next = i;
                continue;
            }

            c2v e1 = c2Sub(verts[next], verts[hull[out_count]]);
            c2v e2 = c2Sub(verts[i], verts[hull[out_count]]);
            float c = c2Det2(e1, e2);
            if (c < 0) next = i;
            if (c == 0 && c2Dot(e2, e2) > c2Dot(e1, e1)) next = i;
        }

        ++out_count;
        index = next;
        if (next == right) break;
    }

    c2v hull_verts[C2_MAX_POLYGON_VERTS];
    for (int i = 0; i < out_count; ++i) hull_verts[i] = verts[hull[i]];
    memcpy(verts, hull_verts, sizeof(c2v) * out_count);
    return out_count;
}

void c2Norms(c2v* verts, c2v* norms, int count) {
    for (int i = 0; i < count; ++i) {
        int a = i;
        int b = i + 1 < count ? i + 1 : 0;
        c2v e = c2Sub(verts[b], verts[a]);
        norms[i] = c2Norm(c2CCW90(e));
    }
}

void c2MakePoly(c2Poly* p) {
    p->count = c2Hull(p->verts, p->count);
    c2Norms(p->verts, p->norms, p->count);
}

c2Poly c2Dual(c2Poly poly, float skin_factor) {
    c2Poly dual;
    dual.count = poly.count;

    // Each plane maps to a point by involution (the mapping is its own inverse) by dividing
    // the plane normal by its offset factor.
    // plane = a * x + b * y - d
    // dual = { a / d, b / d }
    for (int i = 0; i < poly.count; ++i) {
        c2v n = poly.norms[i];
        float d = c2Dot(n, poly.verts[i]) - skin_factor;
        if (d == 0)
            dual.verts[i] = c2V(0, 0);
        else
            dual.verts[i] = c2Div(n, d);
    }

    // Instead of canonically building the convex hull, can simply take advantage of how
    // the vertices are still in proper CCW order, so only the normals must be recomputed.
    c2Norms(dual.verts, dual.norms, dual.count);

    return dual;
}

// Inflating a polytope, idea by Dirk Gregorius ~ 2015. Works in both 2D and 3D.
// Reference: Halfspace intersection with Qhull by Brad Barber
//            http://www.geom.uiuc.edu/graphics/pix/Special_Topics/Computational_Geometry/half.html
//
// Algorithm steps:
// 1. Find a point within the input poly.
// 2. Center this point onto the origin.
// 3. Adjust the planes by a skin factor.
// 4. Compute the dual vert of each plane. Each plane becomes a vertex.
//    c2v dual(c2h plane) { return c2V(plane.n.x / plane.d, plane.n.y / plane.d) }
// 5. Compute the convex hull of the dual verts. This is called the dual.
// 6. Compute the dual of the dual, this will be the poly to return.
// 7. Translate the poly away from the origin by the center point from step 2.
// 8. Return the inflated poly.
c2Poly c2InflatePoly(c2Poly poly, float skin_factor) {
    c2v average = poly.verts[0];
    for (int i = 1; i < poly.count; ++i) {
        average = c2Add(average, poly.verts[i]);
    }
    average = c2Div(average, (float)poly.count);

    for (int i = 0; i < poly.count; ++i) {
        poly.verts[i] = c2Sub(poly.verts[i], average);
    }

    c2Poly dual = c2Dual(poly, skin_factor);
    poly = c2Dual(dual, 0);

    for (int i = 0; i < poly.count; ++i) {
        poly.verts[i] = c2Add(poly.verts[i], average);
    }

    return poly;
}

void c2Inflate(void* shape, C2_TYPE type, float skin_factor) {
    switch (type) {
        case C2_TYPE_CIRCLE: {
            c2Circle* circle = (c2Circle*)shape;
            circle->r += skin_factor;
        } break;

        case C2_TYPE_AABB: {
            c2AABB* bb = (c2AABB*)shape;
            c2v factor = c2V(skin_factor, skin_factor);
            bb->min = c2Sub(bb->min, factor);
            bb->max = c2Add(bb->max, factor);
        } break;

        case C2_TYPE_CAPSULE: {
            c2Capsule* capsule = (c2Capsule*)shape;
            capsule->r += skin_factor;
        } break;

        case C2_TYPE_POLY: {
            c2Poly* poly = (c2Poly*)shape;
            *poly = c2InflatePoly(*poly, skin_factor);
        } break;
    }
}

int c2CircletoCircle(c2Circle A, c2Circle B) {
    c2v c = c2Sub(B.p, A.p);
    float d2 = c2Dot(c, c);
    float r2 = A.r + B.r;
    r2 = r2 * r2;
    return d2 < r2;
}

int c2CircletoAABB(c2Circle A, c2AABB B) {
    c2v L = c2Clampv(A.p, B.min, B.max);
    c2v ab = c2Sub(A.p, L);
    float d2 = c2Dot(ab, ab);
    float r2 = A.r * A.r;
    return d2 < r2;
}

int c2AABBtoAABB(c2AABB A, c2AABB B) {
    int d0 = B.max.x < A.min.x;
    int d1 = A.max.x < B.min.x;
    int d2 = B.max.y < A.min.y;
    int d3 = A.max.y < B.min.y;
    return !(d0 | d1 | d2 | d3);
}

int c2AABBtoPoint(c2AABB A, c2v B) {
    int d0 = B.x < A.min.x;
    int d1 = B.y < A.min.y;
    int d2 = B.x > A.max.x;
    int d3 = B.y > A.max.y;
    return !(d0 | d1 | d2 | d3);
}

int c2CircleToPoint(c2Circle A, c2v B) {
    c2v n = c2Sub(A.p, B);
    float d2 = c2Dot(n, n);
    return d2 < A.r * A.r;
}

// See: https://randygaul.github.io/math/collision-detection/2014/07/01/Distance-Point-to-Line-Segment.html
int c2CircletoCapsule(c2Circle A, c2Capsule B) {
    c2v n = c2Sub(B.b, B.a);
    c2v ap = c2Sub(A.p, B.a);
    float da = c2Dot(ap, n);
    float d2;

    if (da < 0)
        d2 = c2Dot(ap, ap);
    else {
        float db = c2Dot(c2Sub(A.p, B.b), n);
        if (db < 0) {
            c2v e = c2Sub(ap, c2Mulvs(n, (da / c2Dot(n, n))));
            d2 = c2Dot(e, e);
        } else {
            c2v bp = c2Sub(A.p, B.b);
            d2 = c2Dot(bp, bp);
        }
    }

    float r = A.r + B.r;
    return d2 < r * r;
}

int c2AABBtoCapsule(c2AABB A, c2Capsule B) {
    if (c2GJK(&A, C2_TYPE_AABB, 0, &B, C2_TYPE_CAPSULE, 0, 0, 0, 1, 0, 0)) return 0;
    return 1;
}

int c2CapsuletoCapsule(c2Capsule A, c2Capsule B) {
    if (c2GJK(&A, C2_TYPE_CAPSULE, 0, &B, C2_TYPE_CAPSULE, 0, 0, 0, 1, 0, 0)) return 0;
    return 1;
}

int c2CircletoPoly(c2Circle A, const c2Poly* B, const c2x* bx) {
    if (c2GJK(&A, C2_TYPE_CIRCLE, 0, B, C2_TYPE_POLY, bx, 0, 0, 1, 0, 0)) return 0;
    return 1;
}

int c2AABBtoPoly(c2AABB A, const c2Poly* B, const c2x* bx) {
    if (c2GJK(&A, C2_TYPE_AABB, 0, B, C2_TYPE_POLY, bx, 0, 0, 1, 0, 0)) return 0;
    return 1;
}

int c2CapsuletoPoly(c2Capsule A, const c2Poly* B, const c2x* bx) {
    if (c2GJK(&A, C2_TYPE_CAPSULE, 0, B, C2_TYPE_POLY, bx, 0, 0, 1, 0, 0)) return 0;
    return 1;
}

int c2PolytoPoly(const c2Poly* A, const c2x* ax, const c2Poly* B, const c2x* bx) {
    if (c2GJK(A, C2_TYPE_POLY, ax, B, C2_TYPE_POLY, bx, 0, 0, 1, 0, 0)) return 0;
    return 1;
}

int c2RaytoCircle(c2Ray A, c2Circle B, c2Raycast* out) {
    c2v p = B.p;
    c2v m = c2Sub(A.p, p);
    float c = c2Dot(m, m) - B.r * B.r;
    float b = c2Dot(m, A.d);
    float disc = b * b - c;
    if (disc < 0) return 0;

    float t = -b - c2Sqrt(disc);
    if (t >= 0 && t <= A.t) {
        out->t = t;
        c2v impact = c2Impact(A, t);
        out->n = c2Norm(c2Sub(impact, p));
        return 1;
    }
    return 0;
}

static inline float c2SignedDistPointToPlane_OneDimensional(float p, float n, float d) { return p * n - d * n; }

static inline float c2RayToPlane_OneDimensional(float da, float db) {
    if (da < 0)
        return 0;  // Ray started behind plane.
    else if (da * db >= 0)
        return 1.0f;  // Ray starts and ends on the same of the plane.
    else              // Ray starts and ends on opposite sides of the plane (or directly on the plane).
    {
        float d = da - db;
        if (d != 0)
            return da / d;
        else
            return 0;  // Special case for super tiny ray, or AABB.
    }
}

int c2RaytoAABB(c2Ray A, c2AABB B, c2Raycast* out) {
    c2v p0 = A.p;
    c2v p1 = c2Impact(A, A.t);
    c2AABB a_box;
    a_box.min = c2Minv(p0, p1);
    a_box.max = c2Maxv(p0, p1);

    // Test B's axes.
    if (!c2AABBtoAABB(a_box, B)) return 0;

    // Test the ray's axes (along the segment's normal).
    c2v ab = c2Sub(p1, p0);
    c2v n = c2Skew(ab);
    c2v abs_n = c2Absv(n);
    c2v half_extents = c2Mulvs(c2Sub(B.max, B.min), 0.5f);
    c2v center_of_b_box = c2Mulvs(c2Add(B.min, B.max), 0.5f);
    float d = c2Abs(c2Dot(n, c2Sub(p0, center_of_b_box))) - c2Dot(abs_n, half_extents);
    if (d > 0) return 0;

    // Calculate intermediate values up-front.
    // This should play well with superscalar architecture.
    float da0 = c2SignedDistPointToPlane_OneDimensional(p0.x, -1.0f, B.min.x);
    float db0 = c2SignedDistPointToPlane_OneDimensional(p1.x, -1.0f, B.min.x);
    float da1 = c2SignedDistPointToPlane_OneDimensional(p0.x, 1.0f, B.max.x);
    float db1 = c2SignedDistPointToPlane_OneDimensional(p1.x, 1.0f, B.max.x);
    float da2 = c2SignedDistPointToPlane_OneDimensional(p0.y, -1.0f, B.min.y);
    float db2 = c2SignedDistPointToPlane_OneDimensional(p1.y, -1.0f, B.min.y);
    float da3 = c2SignedDistPointToPlane_OneDimensional(p0.y, 1.0f, B.max.y);
    float db3 = c2SignedDistPointToPlane_OneDimensional(p1.y, 1.0f, B.max.y);
    float t0 = c2RayToPlane_OneDimensional(da0, db0);
    float t1 = c2RayToPlane_OneDimensional(da1, db1);
    float t2 = c2RayToPlane_OneDimensional(da2, db2);
    float t3 = c2RayToPlane_OneDimensional(da3, db3);

    // Calculate hit predicate, no branching.
    int hit0 = t0 < 1.0f;
    int hit1 = t1 < 1.0f;
    int hit2 = t2 < 1.0f;
    int hit3 = t3 < 1.0f;
    int hit = hit0 | hit1 | hit2 | hit3;

    if (hit) {
        // Remap t's within 0-1 range, where >= 1 is treated as 0.
        t0 = (float)hit0 * t0;
        t1 = (float)hit1 * t1;
        t2 = (float)hit2 * t2;
        t3 = (float)hit3 * t3;

        // Sort output by finding largest t to deduce the normal.
        if (t0 >= t1 && t0 >= t2 && t0 >= t3) {
            out->t = t0 * A.t;
            out->n = c2V(-1, 0);
        }

        else if (t1 >= t0 && t1 >= t2 && t1 >= t3) {
            out->t = t1 * A.t;
            out->n = c2V(1, 0);
        }

        else if (t2 >= t0 && t2 >= t1 && t2 >= t3) {
            out->t = t2 * A.t;
            out->n = c2V(0, -1);
        }

        else {
            out->t = t3 * A.t;
            out->n = c2V(0, 1);
        }

        return 1;
    } else
        return 0;  // This can still numerically happen.
}

int c2RaytoCapsule(c2Ray A, c2Capsule B, c2Raycast* out) {
    c2m M;
    M.y = c2Norm(c2Sub(B.b, B.a));
    M.x = c2CCW90(M.y);

    // rotate capsule to origin, along Y axis
    // rotate the ray same way
    c2v cap_n = c2Sub(B.b, B.a);
    c2v yBb = c2MulmvT(M, cap_n);
    c2v yAp = c2MulmvT(M, c2Sub(A.p, B.a));
    c2v yAd = c2MulmvT(M, A.d);
    c2v yAe = c2Add(yAp, c2Mulvs(yAd, A.t));

    c2AABB capsule_bb;
    capsule_bb.min = c2V(-B.r, 0);
    capsule_bb.max = c2V(B.r, yBb.y);

    out->n = c2Norm(cap_n);
    out->t = 0;

    // check and see if ray starts within the capsule
    if (c2AABBtoPoint(capsule_bb, yAp)) {
        return 1;
    } else {
        c2Circle capsule_a;
        c2Circle capsule_b;
        capsule_a.p = B.a;
        capsule_a.r = B.r;
        capsule_b.p = B.b;
        capsule_b.r = B.r;

        if (c2CircleToPoint(capsule_a, A.p)) {
            return 1;
        } else if (c2CircleToPoint(capsule_b, A.p)) {
            return 1;
        }
    }

    if (yAe.x * yAp.x < 0 || c2Min(c2Abs(yAe.x), c2Abs(yAp.x)) < B.r) {
        c2Circle Ca, Cb;
        Ca.p = B.a;
        Ca.r = B.r;
        Cb.p = B.b;
        Cb.r = B.r;

        // ray starts inside capsule prism -- must hit one of the semi-circles
        if (c2Abs(yAp.x) < B.r) {
            if (yAp.y < 0)
                return c2RaytoCircle(A, Ca, out);
            else
                return c2RaytoCircle(A, Cb, out);
        }

        // hit the capsule prism
        else {
            float c = yAp.x > 0 ? B.r : -B.r;
            float d = (yAe.x - yAp.x);
            float t = (c - yAp.x) / d;
            float y = yAp.y + (yAe.y - yAp.y) * t;
            if (y <= 0) return c2RaytoCircle(A, Ca, out);
            if (y >= yBb.y)
                return c2RaytoCircle(A, Cb, out);
            else {
                out->n = c > 0 ? M.x : c2Skew(M.y);
                out->t = t * A.t;
                return 1;
            }
        }
    }

    return 0;
}

int c2RaytoPoly(c2Ray A, const c2Poly* B, const c2x* bx_ptr, c2Raycast* out) {
    c2x bx = bx_ptr ? *bx_ptr : c2xIdentity();
    c2v p = c2MulxvT(bx, A.p);
    c2v d = c2MulrvT(bx.r, A.d);
    float lo = 0;
    float hi = A.t;
    int index = ~0;

    // test ray to each plane, tracking lo/hi times of intersection
    for (int i = 0; i < B->count; ++i) {
        float num = c2Dot(B->norms[i], c2Sub(B->verts[i], p));
        float den = c2Dot(B->norms[i], d);
        if (den == 0 && num < 0)
            return 0;
        else {
            if (den < 0 && num < lo * den) {
                lo = num / den;
                index = i;
            } else if (den > 0 && num < hi * den)
                hi = num / den;
        }
        if (hi < lo) return 0;
    }

    if (index != ~0) {
        out->t = lo;
        out->n = c2Mulrv(bx.r, B->norms[index]);
        return 1;
    }

    return 0;
}

void c2CircletoCircleManifold(c2Circle A, c2Circle B, c2Manifold* m) {
    m->count = 0;
    c2v d = c2Sub(B.p, A.p);
    float d2 = c2Dot(d, d);
    float r = A.r + B.r;
    if (d2 < r * r) {
        float l = c2Sqrt(d2);
        c2v n = l != 0 ? c2Mulvs(d, 1.0f / l) : c2V(0, 1.0f);
        m->count = 1;
        m->depths[0] = r - l;
        m->contact_points[0] = c2Sub(B.p, c2Mulvs(n, B.r));
        m->n = n;
    }
}

void c2CircletoAABBManifold(c2Circle A, c2AABB B, c2Manifold* m) {
    m->count = 0;
    c2v L = c2Clampv(A.p, B.min, B.max);
    c2v ab = c2Sub(L, A.p);
    float d2 = c2Dot(ab, ab);
    float r2 = A.r * A.r;
    if (d2 < r2) {
        // shallow (center of circle not inside of AABB)
        if (d2 != 0) {
            float d = c2Sqrt(d2);
            c2v n = c2Norm(ab);
            m->count = 1;
            m->depths[0] = A.r - d;
            m->contact_points[0] = c2Add(A.p, c2Mulvs(n, d));
            m->n = n;
        }

        // deep (center of circle inside of AABB)
        // clamp circle's center to edge of AABB, then form the manifold
        else {
            c2v mid = c2Mulvs(c2Add(B.min, B.max), 0.5f);
            c2v e = c2Mulvs(c2Sub(B.max, B.min), 0.5f);
            c2v d = c2Sub(A.p, mid);
            c2v abs_d = c2Absv(d);

            float x_overlap = e.x - abs_d.x;
            float y_overlap = e.y - abs_d.y;

            float depth;
            c2v n;

            if (x_overlap < y_overlap) {
                depth = x_overlap;
                n = c2V(1.0f, 0);
                n = c2Mulvs(n, d.x < 0 ? 1.0f : -1.0f);
            }

            else {
                depth = y_overlap;
                n = c2V(0, 1.0f);
                n = c2Mulvs(n, d.y < 0 ? 1.0f : -1.0f);
            }

            m->count = 1;
            m->depths[0] = A.r + depth;
            m->contact_points[0] = c2Sub(A.p, c2Mulvs(n, depth));
            m->n = n;
        }
    }
}

void c2CircletoCapsuleManifold(c2Circle A, c2Capsule B, c2Manifold* m) {
    m->count = 0;
    c2v a, b;
    float r = A.r + B.r;
    float d = c2GJK(&A, C2_TYPE_CIRCLE, 0, &B, C2_TYPE_CAPSULE, 0, &a, &b, 0, 0, 0);
    if (d < r) {
        c2v n;
        if (d == 0)
            n = c2Norm(c2Skew(c2Sub(B.b, B.a)));
        else
            n = c2Norm(c2Sub(b, a));

        m->count = 1;
        m->depths[0] = r - d;
        m->contact_points[0] = c2Sub(b, c2Mulvs(n, B.r));
        m->n = n;
    }
}

void c2AABBtoAABBManifold(c2AABB A, c2AABB B, c2Manifold* m) {
    m->count = 0;
    c2v mid_a = c2Mulvs(c2Add(A.min, A.max), 0.5f);
    c2v mid_b = c2Mulvs(c2Add(B.min, B.max), 0.5f);
    c2v eA = c2Absv(c2Mulvs(c2Sub(A.max, A.min), 0.5f));
    c2v eB = c2Absv(c2Mulvs(c2Sub(B.max, B.min), 0.5f));
    c2v d = c2Sub(mid_b, mid_a);

    // calc overlap on x and y axes
    float dx = eA.x + eB.x - c2Abs(d.x);
    if (dx < 0) return;
    float dy = eA.y + eB.y - c2Abs(d.y);
    if (dy < 0) return;

    c2v n;
    float depth;
    c2v p;

    // x axis overlap is smaller
    if (dx < dy) {
        depth = dx;
        if (d.x < 0) {
            n = c2V(-1.0f, 0);
            p = c2Sub(mid_a, c2V(eA.x, 0));
        } else {
            n = c2V(1.0f, 0);
            p = c2Add(mid_a, c2V(eA.x, 0));
        }
    }

    // y axis overlap is smaller
    else {
        depth = dy;
        if (d.y < 0) {
            n = c2V(0, -1.0f);
            p = c2Sub(mid_a, c2V(0, eA.y));
        } else {
            n = c2V(0, 1.0f);
            p = c2Add(mid_a, c2V(0, eA.y));
        }
    }

    m->count = 1;
    m->contact_points[0] = p;
    m->depths[0] = depth;
    m->n = n;
}

void c2AABBtoCapsuleManifold(c2AABB A, c2Capsule B, c2Manifold* m) {
    m->count = 0;
    c2Poly p;
    c2BBVerts(p.verts, &A);
    p.count = 4;
    c2Norms(p.verts, p.norms, 4);
    c2CapsuletoPolyManifold(B, &p, 0, m);
    m->n = c2Neg(m->n);
}

void c2CapsuletoCapsuleManifold(c2Capsule A, c2Capsule B, c2Manifold* m) {
    m->count = 0;
    c2v a, b;
    float r = A.r + B.r;
    float d = c2GJK(&A, C2_TYPE_CAPSULE, 0, &B, C2_TYPE_CAPSULE, 0, &a, &b, 0, 0, 0);
    if (d < r) {
        c2v n;
        if (d == 0)
            n = c2Norm(c2Skew(c2Sub(A.b, A.a)));
        else
            n = c2Norm(c2Sub(b, a));

        m->count = 1;
        m->depths[0] = r - d;
        m->contact_points[0] = c2Sub(b, c2Mulvs(n, B.r));
        m->n = n;
    }
}

static C2_INLINE c2h c2PlaneAt(const c2Poly* p, const int i) {
    c2h h;
    h.n = p->norms[i];
    h.d = c2Dot(p->norms[i], p->verts[i]);
    return h;
}

void c2CircletoPolyManifold(c2Circle A, const c2Poly* B, const c2x* bx_tr, c2Manifold* m) {
    m->count = 0;
    c2v a, b;
    float d = c2GJK(&A, C2_TYPE_CIRCLE, 0, B, C2_TYPE_POLY, bx_tr, &a, &b, 0, 0, 0);

    // shallow, the circle center did not hit the polygon
    // just use a and b from GJK to define the collision
    if (d != 0) {
        c2v n = c2Sub(b, a);
        float l = c2Dot(n, n);
        if (l < A.r * A.r) {
            l = c2Sqrt(l);
            m->count = 1;
            m->contact_points[0] = b;
            m->depths[0] = A.r - l;
            m->n = c2Mulvs(n, 1.0f / l);
        }
    }

    // Circle center is inside the polygon
    // find the face closest to circle center to form manifold
    else {
        c2x bx = bx_tr ? *bx_tr : c2xIdentity();
        float sep = -FLT_MAX;
        int index = ~0;
        c2v local = c2MulxvT(bx, A.p);

        for (int i = 0; i < B->count; ++i) {
            c2h h = c2PlaneAt(B, i);
            d = c2Dist(h, local);
            if (d > A.r) return;
            if (d > sep) {
                sep = d;
                index = i;
            }
        }

        c2h h = c2PlaneAt(B, index);
        c2v p = c2Project(h, local);
        m->count = 1;
        m->contact_points[0] = c2Mulxv(bx, p);
        m->depths[0] = A.r - sep;
        m->n = c2Neg(c2Mulrv(bx.r, B->norms[index]));
    }
}

// Forms a c2Poly and uses c2PolytoPolyManifold
void c2AABBtoPolyManifold(c2AABB A, const c2Poly* B, const c2x* bx, c2Manifold* m) {
    m->count = 0;
    c2Poly p;
    c2BBVerts(p.verts, &A);
    p.count = 4;
    c2Norms(p.verts, p.norms, 4);
    c2PolytoPolyManifold(&p, 0, B, bx, m);
}

// clip a segment to a plane
static int c2Clip(c2v* seg, c2h h) {
    c2v out[2];
    int sp = 0;
    float d0, d1;
    if ((d0 = c2Dist(h, seg[0])) < 0) out[sp++] = seg[0];
    if ((d1 = c2Dist(h, seg[1])) < 0) out[sp++] = seg[1];
    if (d0 == 0 && d1 == 0) {
        out[sp++] = seg[0];
        out[sp++] = seg[1];
    } else if (d0 * d1 <= 0)
        out[sp++] = c2Intersect(seg[0], seg[1], d0, d1);
    seg[0] = out[0];
    seg[1] = out[1];
    return sp;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4204)  // nonstandard extension used: non-constant aggregate initializer
#endif

static int c2SidePlanes(c2v* seg, c2v ra, c2v rb, c2h* h) {
    c2v in = c2Norm(c2Sub(rb, ra));
    c2h left = {c2Neg(in), c2Dot(c2Neg(in), ra)};
    c2h right = {in, c2Dot(in, rb)};
    if (c2Clip(seg, left) < 2) return 0;
    if (c2Clip(seg, right) < 2) return 0;
    if (h) {
        h->n = c2CCW90(in);
        h->d = c2Dot(c2CCW90(in), ra);
    }
    return 1;
}

// clip a segment to the "side planes" of another segment.
// side planes are planes orthogonal to a segment and attached to the
// endpoints of the segment
static int c2SidePlanesFromPoly(c2v* seg, c2x x, const c2Poly* p, int e, c2h* h) {
    c2v ra = c2Mulxv(x, p->verts[e]);
    c2v rb = c2Mulxv(x, p->verts[e + 1 == p->count ? 0 : e + 1]);
    return c2SidePlanes(seg, ra, rb, h);
}

static void c2KeepDeep(c2v* seg, c2h h, c2Manifold* m) {
    int cp = 0;
    for (int i = 0; i < 2; ++i) {
        c2v p = seg[i];
        float d = c2Dist(h, p);
        if (d <= 0) {
            m->contact_points[cp] = p;
            m->depths[cp] = -d;
            ++cp;
        }
    }
    m->count = cp;
    m->n = h.n;
}

static C2_INLINE c2v c2CapsuleSupport(c2Capsule A, c2v dir) {
    float da = c2Dot(A.a, dir);
    float db = c2Dot(A.b, dir);
    if (da > db)
        return c2Add(A.a, c2Mulvs(dir, A.r));
    else
        return c2Add(A.b, c2Mulvs(dir, A.r));
}

static void c2AntinormalFace(c2Capsule cap, const c2Poly* p, c2x x, int* face_out, c2v* n_out) {
    float sep = -FLT_MAX;
    int index = ~0;
    c2v n = c2V(0, 0);
    for (int i = 0; i < p->count; ++i) {
        c2h h = c2Mulxh(x, c2PlaneAt(p, i));
        c2v n0 = c2Neg(h.n);
        c2v s = c2CapsuleSupport(cap, n0);
        float d = c2Dist(h, s);
        if (d > sep) {
            sep = d;
            index = i;
            n = n0;
        }
    }
    *face_out = index;
    *n_out = n;
}

static void c2Incident(c2v* incident, const c2Poly* ip, c2x ix, c2v rn_in_incident_space) {
    int index = ~0;
    float min_dot = FLT_MAX;
    for (int i = 0; i < ip->count; ++i) {
        float dot = c2Dot(rn_in_incident_space, ip->norms[i]);
        if (dot < min_dot) {
            min_dot = dot;
            index = i;
        }
    }
    incident[0] = c2Mulxv(ix, ip->verts[index]);
    incident[1] = c2Mulxv(ix, ip->verts[index + 1 == ip->count ? 0 : index + 1]);
}

void c2CapsuletoPolyManifold(c2Capsule A, const c2Poly* B, const c2x* bx_ptr, c2Manifold* m) {
    m->count = 0;
    c2v a, b;
    float d = c2GJK(&A, C2_TYPE_CAPSULE, 0, B, C2_TYPE_POLY, bx_ptr, &a, &b, 0, 0, 0);

    // deep, treat as segment to poly collision
    if (d < 1.0e-6f) {
        c2x bx = bx_ptr ? *bx_ptr : c2xIdentity();
        c2Capsule A_in_B;
        A_in_B.a = c2MulxvT(bx, A.a);
        A_in_B.b = c2MulxvT(bx, A.b);
        c2v ab = c2Norm(c2Sub(A_in_B.a, A_in_B.b));

        // test capsule axes
        c2h ab_h0;
        ab_h0.n = c2CCW90(ab);
        ab_h0.d = c2Dot(A_in_B.a, ab_h0.n);
        int v0 = c2Support(B->verts, B->count, c2Neg(ab_h0.n));
        float s0 = c2Dist(ab_h0, B->verts[v0]);

        c2h ab_h1;
        ab_h1.n = c2Skew(ab);
        ab_h1.d = c2Dot(A_in_B.a, ab_h1.n);
        int v1 = c2Support(B->verts, B->count, c2Neg(ab_h1.n));
        float s1 = c2Dist(ab_h1, B->verts[v1]);

        // test poly axes
        int index = ~0;
        float sep = -FLT_MAX;
        int code = 0;
        for (int i = 0; i < B->count; ++i) {
            c2h h = c2PlaneAt(B, i);
            float da = c2Dot(A_in_B.a, c2Neg(h.n));
            float db = c2Dot(A_in_B.b, c2Neg(h.n));
            float d;
            if (da > db)
                d = c2Dist(h, A_in_B.a);
            else
                d = c2Dist(h, A_in_B.b);
            if (d > sep) {
                sep = d;
                index = i;
            }
        }

        // track axis of minimum separation
        if (s0 > sep) {
            sep = s0;
            index = v0;
            code = 1;
        }

        if (s1 > sep) {
            sep = s1;
            index = v1;
            code = 2;
        }

        switch (code) {
            case 0:  // poly face
            {
                c2v seg[2] = {A.a, A.b};
                c2h h;
                if (!c2SidePlanesFromPoly(seg, bx, B, index, &h)) return;
                c2KeepDeep(seg, h, m);
                m->n = c2Neg(m->n);
            } break;

            case 1:  // side 0 of capsule segment
            {
                c2v incident[2];
                c2Incident(incident, B, bx, ab_h0.n);
                c2h h;
                if (!c2SidePlanes(incident, A_in_B.b, A_in_B.a, &h)) return;
                c2KeepDeep(incident, h, m);
            } break;

            case 2:  // side 1 of capsule segment
            {
                c2v incident[2];
                c2Incident(incident, B, bx, ab_h1.n);
                c2h h;
                if (!c2SidePlanes(incident, A_in_B.a, A_in_B.b, &h)) return;
                c2KeepDeep(incident, h, m);
            } break;

            default:
                // should never happen.
                return;
        }

        for (int i = 0; i < m->count; ++i) m->depths[i] += A.r;
    }

    // shallow, use GJK results a and b to define manifold
    else if (d < A.r) {
        m->count = 1;
        m->n = c2Norm(c2Sub(b, a));
        m->contact_points[0] = c2Add(a, c2Mulvs(m->n, A.r));
        m->depths[0] = A.r - d;
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static float c2CheckFaces(const c2Poly* A, c2x ax, const c2Poly* B, c2x bx, int* face_index) {
    c2x b_in_a = c2MulxxT(ax, bx);
    c2x a_in_b = c2MulxxT(bx, ax);
    float sep = -FLT_MAX;
    int index = ~0;

    for (int i = 0; i < A->count; ++i) {
        c2h h = c2PlaneAt(A, i);
        int idx = c2Support(B->verts, B->count, c2Mulrv(a_in_b.r, c2Neg(h.n)));
        c2v p = c2Mulxv(b_in_a, B->verts[idx]);
        float d = c2Dist(h, p);
        if (d > sep) {
            sep = d;
            index = i;
        }
    }

    *face_index = index;
    return sep;
}

// Please see Dirk Gregorius's 2013 GDC lecture on the Separating Axis Theorem
// for a full-algorithm overview. The short description is:
// Test A against faces of B, test B against faces of A
// Define the reference and incident shapes (denoted by r and i respectively)
// Define the reference face as the axis of minimum penetration
// Find the incident face, which is most anti-normal face
// Clip incident face to reference face side planes
// Keep all points behind the reference face
void c2PolytoPolyManifold(const c2Poly* A, const c2x* ax_ptr, const c2Poly* B, const c2x* bx_ptr, c2Manifold* m) {
    m->count = 0;
    c2x ax = ax_ptr ? *ax_ptr : c2xIdentity();
    c2x bx = bx_ptr ? *bx_ptr : c2xIdentity();
    int ea, eb;
    float sa, sb;
    if ((sa = c2CheckFaces(A, ax, B, bx, &ea)) >= 0) return;
    if ((sb = c2CheckFaces(B, bx, A, ax, &eb)) >= 0) return;

    const c2Poly *rp, *ip;
    c2x rx, ix;
    int re;
    float kRelTol = 0.95f, kAbsTol = 0.01f;
    int flip;
    if (sa * kRelTol > sb + kAbsTol) {
        rp = A;
        rx = ax;
        ip = B;
        ix = bx;
        re = ea;
        flip = 0;
    } else {
        rp = B;
        rx = bx;
        ip = A;
        ix = ax;
        re = eb;
        flip = 1;
    }

    c2v incident[2];
    c2Incident(incident, ip, ix, c2MulrvT(ix.r, c2Mulrv(rx.r, rp->norms[re])));
    c2h rh;
    if (!c2SidePlanesFromPoly(incident, rx, rp, re, &rh)) return;
    c2KeepDeep(incident, rh, m);
    if (flip) m->n = c2Neg(m->n);
}
