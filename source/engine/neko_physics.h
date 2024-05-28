
#ifndef NEKO_PHYSICS_H
#define NEKO_PHYSICS_H

#include "neko.h"
#include "neko_math.h"

/*==== Collision Shapes ====*/

#define NEKO_PHYSICS_NO_CCD

// 3D shapes
typedef struct neko_line_t {
    neko_vec3 a, b;
} neko_line_t;

typedef struct neko_aabb_t {
    neko_vec3 min, max;
} neko_aabb_t;

typedef struct neko_sphere_t {
    neko_vec3 c;
    float r;
} neko_sphere_t;

typedef struct neko_plane_t {
    union {
        neko_vec3 p;
        struct {
            float a, b, c;
        };
    };
    float d;
} neko_plane_t;

typedef struct neko_capsule_t {
    neko_vec3 base;
    float r, height;
} neko_capsule_t;

typedef struct neko_ray_s {
    neko_vec3 p, d;
    float len;
} neko_ray_t;

typedef struct neko_poly_t {
    neko_vec3* verts;
    s32 cnt;
} neko_poly_t;

typedef union neko_frustum_t {
    struct {
        neko_vec4 l, r, t, b, n, f;
    };
    neko_vec4 pl[6];
    float v[24];
} neko_frustum_t;

typedef struct neko_cylinder_t {
    float r;
    neko_vec3 base;
    float height;
} neko_cylinder_t;

typedef struct neko_cone_t {
    float r;
    neko_vec3 base;
    float height;
} neko_cone_t;

// 2D shapes
typedef struct neko_circle_t {
    float r;
    neko_vec2 c;
} neko_circle_t;
// typedef struct neko_rect_t     {neko_vec2 min; neko_vec2 max;                                      } neko_rect_t;
// typedef struct neko_triangle_t {neko_vec2 a,b,c;                                                 } neko_triangle_t;
// typedef struct neko_pill_t     {neko_vec2 base; float r, height;                                 } neko_pill_t;

/*
    typedef struct _neko_collision_obj_handle_t
    {
        void* obj;
        neko_support_func_t support;
        neko_vqs* xform;
    } _neko_collision_obj_handle_t;

    // Wrap this, then expose collision callback to user through neko means

    // Internal support function for all ccd callbacks (will pass in user function through this cb),
    // this way I can keep everything consistent, only expose neko related APIs, wrap any 3rd party libs,
    // and possibly change internals in the future to custom implementations without breaking anything.
    void _neko_ccd_support_func(const void* _obj, const ccd_vec3_t* _dir, ccd_vec3_t* _out)
    {
        const _neko_collision_obj_handle_t* obj = (const _neko_collision_obj_handle_t)_obj;
        if (obj->support)
        {
            // Call user support function
            neko_vec3 dir = _neko_ccdv32gsv3(_dir);
            neko_vec3 out = neko_default_val();
            obj->support(obj->obj, obj->xform, &dir, &out);

            // Copy over out result for ccd
            _neko_gsv32ccdv3(&out, _out);
        }
    }
*/

// Need 2d collision shapes and responses with GJK+EPA

typedef struct neko_hit_t {
    s32 hit;
    union {
        // General case
        float depth;
        // Rays: Penetration (t0) and Extraction (t1) pts. along ray line
        struct {
            float t0, t1;
        };
        // GJK only
        struct {
            s32 hits, iterations;
            neko_vec3 p0, p1;
            float distance2;
        };
    };
    union {
        neko_vec3 p;
        neko_vec3 contact_point;
    };
    union {
        neko_vec3 n;
        neko_vec3 normal;
    };
} neko_hit_t;

// Constructors with args
#define neko_line(...) neko_ctor(neko_line_t, __VA_ARGS__)
#define neko_sphere(...) neko_ctor(neko_sphere_t, __VA_ARGS__)
#define neko_aabb(...) neko_ctor(neko_aabb_t, __VA_ARGS__)
#define neko_plane(...) neko_ctor(neko_plane_t, __VA_ARGS__)
#define neko_capsule(...) neko_ctor(neko_capsule_t, __VA_ARGS__)
#define neko_cone(...) neko_ctor(neko_cone_t, __VA_ARGS__)
#define neko_ray(...) neko_ctor(neko_ray_t, __VA_ARGS__)
#define neko_poly(...) neko_ctor(neko_poly_t, __VA_ARGS__)
#define neko_frustum(...) .neko_ctor(neko_frustum_t, __VA_ARGS__)
#define neko_cylinder(...) neko_ctor(neko_cylinder_t, __VA_ARGS__)
#define neko_hit(...) neko_ctor(neko_hit_t, __VA_ARGS__)

// Contact info
typedef struct neko_contact_info_t {
    s32 hit;
    neko_vec3 normal;
    float depth;
    neko_vec3 point;
} neko_contact_info_t;

NEKO_API_DECL b32 neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b);

/* Line/Segment */
NEKO_API_DECL neko_line_t neko_line_closest_line(const neko_line_t* l, neko_vec3 p);
NEKO_API_DECL neko_vec3 neko_line_closest_point(const neko_line_t* l, neko_vec3 p);
NEKO_API_DECL neko_vec3 neko_line_direction(const neko_line_t* l);

/* Ray */

/* Plane */
NEKO_API_DECL neko_plane_t neko_plane_from_pt_normal(neko_vec3 pt, neko_vec3 n);
NEKO_API_DECL neko_plane_t neko_plane_from_pts(neko_vec3 a, neko_vec3 b, neko_vec3 c);
NEKO_API_DECL neko_vec3 neko_plane_normal(const neko_plane_t* p);
NEKO_API_DECL neko_vec3 neko_plane_closest_point(const neko_plane_t* p, neko_vec3 pt);
NEKO_API_DECL float neko_plane_signed_distance(const neko_plane_t* p, neko_vec3 pt);
NEKO_API_DECL float neko_plane_unsigned_distance(const neko_plane_t* p, neko_vec3 pt);
NEKO_API_DECL neko_plane_t neko_plane_normalized(const neko_plane_t* p);

/* Sphere */
NEKO_API_DECL s32 neko_sphere_vs_sphere(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_sphere_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_sphere_vs_aabb(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_aabb_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_sphere_vs_poly(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_poly_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_sphere_vs_cylinder(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_cylinder_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_sphere_vs_cone(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_cone_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_sphere_vs_capsule(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_capsule_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_sphere_vs_ray(const neko_sphere_t* a, const neko_vqs* xform_a, const neko_ray_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);

/* Box */
// NEKO_API_DECL s32 neko_aabb_vs_aabb(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_aabb_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_aabb_vs_sphere(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_sphere_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_aabb_vs_poly(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_poly_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_aabb_vs_cylinder(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_cylinder_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_aabb_vs_cone(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_cone_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_aabb_vs_capsule(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_capsule_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_aabb_vs_ray(const neko_aabb_t* a, const neko_vqs* xform_a, const neko_ray_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);

/* Capsule */
NEKO_API_DECL s32 neko_capsule_vs_aabb(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_aabb_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_capsule_vs_sphere(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_sphere_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_capsule_vs_poly(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_poly_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_capsule_vs_cylinder(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_cylinder_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_capsule_vs_cone(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_cone_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_capsule_vs_capsule(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_capsule_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_capsule_vs_ray(const neko_capsule_t* capsule, const neko_vqs* xform_a, const neko_ray_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);

/* Poly */
NEKO_API_DECL s32 neko_poly_vs_poly(const neko_poly_t* a, const neko_vqs* xform_a, const neko_poly_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_poly_vs_sphere(const neko_poly_t* a, const neko_vqs* xform_a, const neko_sphere_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_poly_vs_aabb(const neko_poly_t* a, const neko_vqs* xform_a, const neko_aabb_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_poly_vs_cylinder(const neko_poly_t* a, const neko_vqs* xform_a, const neko_cylinder_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_poly_vs_cone(const neko_poly_t* a, const neko_vqs* xform_a, const neko_cone_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_poly_vs_capsule(const neko_poly_t* a, const neko_vqs* xform_a, const neko_capsule_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_poly_vs_ray(const neko_poly_t* a, const neko_vqs* xform_a, const neko_ray_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);

/* Frustum */

/* Cylinder */
NEKO_API_DECL s32 neko_cylinder_vs_cylinder(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_cylinder_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cylinder_vs_sphere(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_sphere_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cylinder_vs_aabb(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_aabb_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cylinder_vs_poly(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_poly_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cylinder_vs_cone(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_cone_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cylinder_vs_capsule(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_capsule_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cylinder_vs_ray(const neko_cylinder_t* a, const neko_vqs* xform_a, const neko_ray_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);

/* Cone */
NEKO_API_DECL s32 neko_cone_vs_cone(const neko_cone_t* a, const neko_vqs* xform_a, const neko_cone_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cone_vs_sphere(const neko_cone_t* a, const neko_vqs* xform_a, const neko_sphere_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cone_vs_aabb(const neko_cone_t* a, const neko_vqs* xform_a, const neko_aabb_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cone_vs_poly(const neko_cone_t* a, const neko_vqs* xform_a, const neko_poly_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cone_vs_cylinder(const neko_cone_t* a, const neko_vqs* xform_a, const neko_cylinder_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cone_vs_capsule(const neko_cone_t* a, const neko_vqs* xform_a, const neko_capsule_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);
NEKO_API_DECL s32 neko_cone_vs_ray(const neko_cone_t* a, const neko_vqs* xform_a, const neko_ray_t* b, const neko_vqs* xform_b, neko_contact_info_t* res);

// 2D Shapes (eventually)

/* Hit */

/*==== Support Functions ====*/

// Support function typedef for GJK collision detection
typedef void (*neko_support_func_t)(const void* collider, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);

NEKO_API_DECL void neko_support_poly(const void* p, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);
NEKO_API_DECL void neko_support_sphere(const void* s, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);
NEKO_API_DECL void neko_support_aabb(const void* a, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);
NEKO_API_DECL void neko_support_cylinder(const void* c, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);
NEKO_API_DECL void neko_support_cone(const void* c, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);
NEKO_API_DECL void neko_support_capsule(const void* c, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);
NEKO_API_DECL void neko_support_ray(const void* r, const neko_vqs* xform, const neko_vec3* dir, neko_vec3* out);

/*==== GJK ====*/

#define NEKO_GJK_FLT_MAX FLT_MAX      // 3.40282347E+38F
#define NEKO_GJK_EPSILON FLT_EPSILON  // 1.19209290E-07F
#define NEKO_GJK_MAX_ITERATIONS 64

#define NEKO_EPA_TOLERANCE 0.0001
#define NEKO_EPA_MAX_NUM_FACES 64
#define NEKO_EPA_MAX_NUM_LOOSE_EDGES 32
#define NEKO_EPA_MAX_NUM_ITERATIONS 64

typedef enum neko_gjk_dimension { NEKO_GJK_DIMENSION_2D, NEKO_GJK_DIMENSION_3D } neko_gjk_dimension;

typedef struct neko_gjk_support_point_t {
    neko_vec3 support_a;
    neko_vec3 support_b;
    neko_vec3 minkowski_hull_vert;
} neko_gjk_support_point_t;

typedef struct neko_gjk_simplex_t {
    union {
        neko_gjk_support_point_t points[4];
        struct {
            neko_gjk_support_point_t a;
            neko_gjk_support_point_t b;
            neko_gjk_support_point_t c;
            neko_gjk_support_point_t d;
        };
    };
    u32 ct;
} neko_gjk_simplex_t;

typedef struct neko_gjk_polytope_face_t {
    neko_gjk_support_point_t points[3];
    neko_vec3 normal;
} neko_gjk_polytope_face_t;

typedef struct neko_gjk_epa_edge_t {
    neko_vec3 normal;
    u32 index;
    float distance;
    neko_gjk_support_point_t a, b;
} neko_gjk_epa_edge_t;

// NEKO_API_DECL s32 neko_gjk(const neko_gjk_collider_info_t* ci0, const neko_gjk_collider_info_t* ci1, neko_gjk_dimension dimension, neko_contact_info_t* res);
// NEKO_API_DECL neko_contact_info_t neko_gjk_epa(const neko_gjk_simplex_t* simplex, const neko_gjk_collider_info_t* ci0, const neko_gjk_collider_info_t* ci1);
// NEKO_API_DECL neko_contact_info_t neko_gjk_epa_2d(const neko_gjk_simplex_t* simplex, const neko_gjk_collider_info_t* ci0, const neko_gjk_collider_info_t* ci1);
// NEKO_API_DECL neko_gjk_collider_info_t neko_gjk_collider_info(void* c, neko_support_func_t f, neko_phys_xform_t* t);
//
NEKO_API_PRIVATE s32 _neko_ccd_gjk_internal(const void* c0, const neko_vqs* xform_a, neko_support_func_t f0, const void* c1, const neko_vqs* xform_b, neko_support_func_t f1, neko_contact_info_t* res);

/*==== CCD ====*/

#ifndef NEKO_PHYSICS_NO_CCD

#include "deps/ccd/src/ccd/ccd_vec3.h"

// Internal collision object conversion handle
typedef struct _neko_collision_obj_handle_t {
    const void* obj;
    neko_support_func_t support;
    const neko_vqs* xform;
} _neko_collision_obj_handle_t;

// Internal support function for all ccd callbacks (will pass in user function through this cb),
// this way I can keep everything consistent, only expose neko related APIs, wrap any 3rd party libs,
// and possibly change internals in the future to custom implementations without breaking anything.
NEKO_API_DECL void _neko_ccd_support_func(const void* _obj, const ccd_vec3_t* _dir, ccd_vec3_t* _out);
NEKO_API_DECL s32 _neko_ccd_gjk(const _neko_collision_obj_handle_t* c0, const _neko_collision_obj_handle_t* c1, neko_contact_info_t* res);

#endif  // NEKO_PHYSICS_NO_CCD

/*==== Physics System ====*/

#define EMPTY_VAR(...) s32 __empty

/* Rigid Body */

// Just want a single collision shape...not sure exactly how to handle this.
// Probably keep the data for the collision shape within the scene itself (or physics system)? Then give handles to users for the data.

typedef enum neko_rigid_body_type { NEKO_RIGID_BODY_STATIC, NEKO_RIGID_BODY_DYNAMIC, NEKO_RIGID_BODY_KINEMATIC } neko_rigid_body_type;

typedef enum neko_rigid_body_state_flags {
    NEKO_RIGID_BODY_STATE_AWAKE = 0x001,
    NEKO_RIGID_BODY_STATE_ACTIVE = 0x002,
    NEKO_RIGID_BODY_STATE_ALLOW_SLEEP = 0x004,
    NEKO_RIGID_BODY_STATE_ISLAND = 0x010,
    NEKO_RIGID_BODY_STATE_STATIC = 0x020,
    NEKO_RIGID_BODY_STATE_DYNAMIC = 0x040,
    NEKO_RIGID_BODY_STATE_KINEMATIC = 0x080,
    NEKO_RIGID_BODY_STATE_LOCK_AXIS_X = 0x100,
    NEKO_RIGID_BODY_STATE_LOCK_AXIS_Y = 0x200,
    NEKO_RIGID_BODY_STATE_LOCK_AXIS_Z = 0x400
} neko_rigid_body_state_flags;

typedef struct neko_rigid_body_t {
    EMPTY_VAR();

    float mass;
    float inverve_mass;
    neko_vec3 linear_velocity;
    neko_vec3 angular_velocity;
    neko_vec3 force;
    neko_vec3 torque;
    neko_quat rotation;
    neko_vec3 local_center;
    neko_vec3 world_center;
    float sleep_time;
    float gravity_scale;
    u32 flags;

    u32 island_index;
    void* user_data;

} neko_rigid_body_t;

typedef struct neko_rigid_body_desc_t {
    EMPTY_VAR();
} neko_rigid_body_desc_t;

/* Collision Shape */

typedef struct neko_collision_shape_t {
    EMPTY_VAR();
} neko_collision_shape_t;

typedef struct neko_contact_constraint_t {
    EMPTY_VAR();
} neko_contact_constraint_t;

typedef struct neko_raycast_data_t {
    EMPTY_VAR();
} neko_raycast_data_t;

// The listener is used to gather information about two shapes colliding. Physics objects created in these callbacks
// Are not reported until following frame. Callbacks can be called quite frequently, so make them efficient.
typedef struct neko_contact_listener_t {
    void (*begin_contact)(const neko_contact_constraint_t*);
    void (*end_contact)(const neko_contact_constraint_t*);
} neko_contact_listener_t;

typedef struct neko_query_callback_t {
    bool (*report_shape)(neko_collision_shape_t* shape);
} neko_query_callback_t;

// Contact Manager
typedef struct neko_physics_contact_manager_t {
    EMPTY_VAR();
} neko_physics_contact_manager_t;

/* Physics Scene */

typedef struct neko_physics_scene_t {
    neko_physics_contact_manager_t contact_manager;
    neko_paged_allocator_t paged_allocator;
    neko_stack_allocator_t stack_allocator;
    neko_heap_allocator_t heap_allocator;
    neko_vec3 gravity;
    float delta_time;
    u32 iterations;
} neko_physics_scene_t;

NEKO_API_DECL neko_physics_scene_t neko_physics_scene_new();
NEKO_API_DECL void neko_physics_scene_step(neko_physics_scene_t* scene);
NEKO_API_DECL u32 neko_physics_scene_create_body(neko_physics_scene_t* scene, neko_rigid_body_desc_t* desc);
NEKO_API_DECL void neko_physics_scene_destroy_body(neko_physics_scene_t* scene, u32 id);
NEKO_API_DECL void neko_physics_scene_destroy_all_bodies(neko_physics_scene_t* scene);
NEKO_API_DECL neko_raycast_data_t neko_physics_scene_raycast(neko_physics_scene_t* scene, neko_query_callback_t* cb);

#endif  // NEKO_PHYSICS_H
