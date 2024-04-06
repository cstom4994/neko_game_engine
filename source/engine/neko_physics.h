
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

// this can be adjusted as necessary, but is highly recommended to be kept at 8.
// higher numbers will incur quite a bit of memory overhead, and convex shapes
// over 8 verts start to just look like spheres, which can be implicitly rep-
// resented as a point + radius. usually tools that generate polygons should be
// constructed so they do not output polygons with too many verts.
// Note: polygons in cute_c2 are all *convex*.
#define C2_MAX_POLYGON_VERTS 8

// 2d vector
typedef struct c2v {
    float x;
    float y;
} c2v;

// 2d rotation composed of cos/sin pair for a single angle
// We use two floats as a small optimization to avoid computing sin/cos unnecessarily
typedef struct c2r {
    float c;
    float s;
} c2r;

// 2d rotation matrix
typedef struct c2m {
    c2v x;
    c2v y;
} c2m;

// 2d transformation "x"
// These are used especially for c2Poly when a c2Poly is passed to a function.
// Since polygons are prime for "instancing" a c2x transform can be used to
// transform a polygon from local space to world space. In functions that take
// a c2x pointer (like c2PolytoPoly), these pointers can be NULL, which represents
// an identity transformation and assumes the verts inside of c2Poly are already
// in world space.
typedef struct c2x {
    c2v p;
    c2r r;
} c2x;

// 2d halfspace (aka plane, aka line)
typedef struct c2h {
    c2v n;    // normal, normalized
    float d;  // distance to origin from plane, or ax + by = d
} c2h;

typedef struct c2Circle {
    c2v p;
    float r;
} c2Circle;

typedef struct c2AABB {
    c2v min;
    c2v max;
} c2AABB;

// a capsule is defined as a line segment (from a to b) and radius r
typedef struct c2Capsule {
    c2v a;
    c2v b;
    float r;
} c2Capsule;

typedef struct c2Poly {
    int count;
    c2v verts[C2_MAX_POLYGON_VERTS];
    c2v norms[C2_MAX_POLYGON_VERTS];
} c2Poly;

// IMPORTANT:
// Many algorithms in this file are sensitive to the magnitude of the
// ray direction (c2Ray::d). It is highly recommended to normalize the
// ray direction and use t to specify a distance. Please see this link
// for an in-depth explanation: https://github.com/RandyGaul/cute_headers/issues/30
typedef struct c2Ray {
    c2v p;    // position
    c2v d;    // direction (normalized)
    float t;  // distance along d from position p to find endpoint of ray
} c2Ray;

typedef struct c2Raycast {
    float t;  // time of impact
    c2v n;    // normal of surface at impact (unit length)
} c2Raycast;

// position of impact p = ray.p + ray.d * raycast.t
#define c2Impact(ray, t) c2Add(ray.p, c2Mulvs(ray.d, t))

// contains all information necessary to resolve a collision, or in other words
// this is the information needed to separate shapes that are colliding. Doing
// the resolution step is *not* included in cute_c2.
typedef struct c2Manifold {
    int count;
    float depths[2];
    c2v contact_points[2];

    // always points from shape A to shape B (first and second shapes passed into
    // any of the c2***to***Manifold functions)
    c2v n;
} c2Manifold;

// This define allows exporting/importing of the header to a dynamic library.
// Here's an example.
// #define NEKO_C2_API extern "C" __declspec(dllexport)
#if !defined(NEKO_C2_API)
#define NEKO_C2_API
#endif

// boolean collision detection
// these versions are faster than the manifold versions, but only give a YES/NO result
NEKO_C2_API int c2CircletoCircle(c2Circle A, c2Circle B);
NEKO_C2_API int c2CircletoAABB(c2Circle A, c2AABB B);
NEKO_C2_API int c2CircletoCapsule(c2Circle A, c2Capsule B);
NEKO_C2_API int c2AABBtoAABB(c2AABB A, c2AABB B);
NEKO_C2_API int c2AABBtoCapsule(c2AABB A, c2Capsule B);
NEKO_C2_API int c2CapsuletoCapsule(c2Capsule A, c2Capsule B);
NEKO_C2_API int c2CircletoPoly(c2Circle A, const c2Poly* B, const c2x* bx);
NEKO_C2_API int c2AABBtoPoly(c2AABB A, const c2Poly* B, const c2x* bx);
NEKO_C2_API int c2CapsuletoPoly(c2Capsule A, const c2Poly* B, const c2x* bx);
NEKO_C2_API int c2PolytoPoly(const c2Poly* A, const c2x* ax, const c2Poly* B, const c2x* bx);

// ray operations
// output is placed into the c2Raycast struct, which represents the hit location
// of the ray. the out param contains no meaningful information if these funcs
// return 0
NEKO_C2_API int c2RaytoCircle(c2Ray A, c2Circle B, c2Raycast* out);
NEKO_C2_API int c2RaytoAABB(c2Ray A, c2AABB B, c2Raycast* out);
NEKO_C2_API int c2RaytoCapsule(c2Ray A, c2Capsule B, c2Raycast* out);
NEKO_C2_API int c2RaytoPoly(c2Ray A, const c2Poly* B, const c2x* bx_ptr, c2Raycast* out);

// manifold generation
// These functions are (generally) slower than the boolean versions, but will compute one
// or two points that represent the plane of contact. This information is usually needed
// to resolve and prevent shapes from colliding. If no collision occured the count member
// of the manifold struct is set to 0.
NEKO_C2_API void c2CircletoCircleManifold(c2Circle A, c2Circle B, c2Manifold* m);
NEKO_C2_API void c2CircletoAABBManifold(c2Circle A, c2AABB B, c2Manifold* m);
NEKO_C2_API void c2CircletoCapsuleManifold(c2Circle A, c2Capsule B, c2Manifold* m);
NEKO_C2_API void c2AABBtoAABBManifold(c2AABB A, c2AABB B, c2Manifold* m);
NEKO_C2_API void c2AABBtoCapsuleManifold(c2AABB A, c2Capsule B, c2Manifold* m);
NEKO_C2_API void c2CapsuletoCapsuleManifold(c2Capsule A, c2Capsule B, c2Manifold* m);
NEKO_C2_API void c2CircletoPolyManifold(c2Circle A, const c2Poly* B, const c2x* bx, c2Manifold* m);
NEKO_C2_API void c2AABBtoPolyManifold(c2AABB A, const c2Poly* B, const c2x* bx, c2Manifold* m);
NEKO_C2_API void c2CapsuletoPolyManifold(c2Capsule A, const c2Poly* B, const c2x* bx, c2Manifold* m);
NEKO_C2_API void c2PolytoPolyManifold(const c2Poly* A, const c2x* ax, const c2Poly* B, const c2x* bx, c2Manifold* m);

typedef enum { C2_TYPE_CIRCLE, C2_TYPE_AABB, C2_TYPE_CAPSULE, C2_TYPE_POLY } C2_TYPE;

// This struct is only for advanced usage of the c2GJK function. See comments inside of the
// c2GJK function for more details.
typedef struct c2GJKCache {
    float metric;
    int count;
    int iA[3];
    int iB[3];
    float div;
} c2GJKCache;

// This is an advanced function, intended to be used by people who know what they're doing.
//
// Runs the GJK algorithm to find closest points, returns distance between closest points.
// outA and outB can be NULL, in this case only distance is returned. ax_ptr and bx_ptr
// can be NULL, and represent local to world transformations for shapes A and B respectively.
// use_radius will apply radii for capsules and circles (if set to false, spheres are
// treated as points and capsules are treated as line segments i.e. rays). The cache parameter
// should be NULL, as it is only for advanced usage (unless you know what you're doing, then
// go ahead and use it). iterations is an optional parameter.
//
// IMPORTANT NOTE:
// The GJK function is sensitive to large shapes, since it internally will compute signed area
// values. `c2GJK` is called throughout cute c2 in many ways, so try to make sure all of your
// collision shapes are not gigantic. For example, try to keep the volume of all your shapes
// less than 100.0f. If you need large shapes, you should use tiny collision geometry for all
// cute c2 function, and simply render the geometry larger on-screen by scaling it up.
NEKO_C2_API float c2GJK(const void* A, C2_TYPE typeA, const c2x* ax_ptr, const void* B, C2_TYPE typeB, const c2x* bx_ptr, c2v* outA, c2v* outB, int use_radius, int* iterations, c2GJKCache* cache);

// Stores results of a time of impact calculation done by `c2TOI`.
typedef struct c2TOIResult {
    int hit;         // 1 if shapes were touching at the TOI, 0 if they never hit.
    float toi;       // The time of impact between two shapes.
    c2v n;           // Surface normal from shape A to B at the time of impact.
    c2v p;           // Point of contact between shapes A and B at time of impact.
    int iterations;  // Number of iterations the solver underwent.
} c2TOIResult;

// This is an advanced function, intended to be used by people who know what they're doing.
//
// Computes the time of impact from shape A and shape B. The velocity of each shape is provided
// by vA and vB respectively. The shapes are *not* allowed to rotate over time. The velocity is
// assumed to represent the change in motion from time 0 to time 1, and so the return value will
// be a number from 0 to 1. To move each shape to the colliding configuration, multiply vA and vB
// each by the return value. ax_ptr and bx_ptr are optional parameters to transforms for each shape,
// and are typically used for polygon shapes to transform from model to world space. Set these to
// NULL to represent identity transforms. iterations is an optional parameter. use_radius
// will apply radii for capsules and circles (if set to false, spheres are treated as points and
// capsules are treated as line segments i.e. rays).
//
// IMPORTANT NOTE:
// The c2TOI function can be used to implement a "swept character controller", but it can be
// difficult to do so. Say we compute a time of impact with `c2TOI` and move the shapes to the
// time of impact, and adjust the velocity by zeroing out the velocity along the surface normal.
// If we then call `c2TOI` again, it will fail since the shapes will be considered to start in
// a colliding configuration. There are many styles of tricks to get around this problem, and
// all of them involve giving the next call to `c2TOI` some breathing room. It is recommended
// to use some variation of the following algorithm:
//
// 1. Call c2TOI.
// 2. Move the shapes to the TOI.
// 3. Slightly inflate the size of one, or both, of the shapes so they will be intersecting.
//    The purpose is to make the shapes numerically intersecting, but not visually intersecting.
//    Another option is to call c2TOI with slightly deflated shapes.
//    See the function `c2Inflate` for some more details.
// 4. Compute the collision manifold between the inflated shapes (for example, use c2PolytoPolyManifold).
// 5. Gently push the shapes apart. This will give the next call to c2TOI some breathing room.
NEKO_C2_API c2TOIResult c2TOI(const void* A, C2_TYPE typeA, const c2x* ax_ptr, c2v vA, const void* B, C2_TYPE typeB, const c2x* bx_ptr, c2v vB, int use_radius);

// Inflating a shape.
//
// This is useful to numerically grow or shrink a polytope. For example, when calling
// a time of impact function it can be good to use a slightly smaller shape. Then, once
// both shapes are moved to the time of impact a collision manifold can be made from the
// slightly larger (and now overlapping) shapes.
//
// IMPORTANT NOTE
// Inflating a shape with sharp corners can cause those corners to move dramatically.
// Deflating a shape can avoid this problem, but deflating a very small shape can invert
// the planes and result in something that is no longer convex. Make sure to pick an
// appropriately small skin factor, for example 1.0e-6f.
NEKO_C2_API void c2Inflate(void* shape, C2_TYPE type, float skin_factor);

// Computes 2D convex hull. Will not do anything if less than two verts supplied. If
// more than C2_MAX_POLYGON_VERTS are supplied extras are ignored.
NEKO_C2_API int c2Hull(c2v* verts, int count);
NEKO_C2_API void c2Norms(c2v* verts, c2v* norms, int count);

// runs c2Hull and c2Norms, assumes p->verts and p->count are both set to valid values
NEKO_C2_API void c2MakePoly(c2Poly* p);

// Generic collision detection routines, useful for games that want to use some poly-
// morphism to write more generic-styled code. Internally calls various above functions.
// For AABBs/Circles/Capsules ax and bx are ignored. For polys ax and bx can define
// model to world transformations (for polys only), or be NULL for identity transforms.
NEKO_C2_API int c2Collided(const void* A, const c2x* ax, C2_TYPE typeA, const void* B, const c2x* bx, C2_TYPE typeB);
NEKO_C2_API void c2Collide(const void* A, const c2x* ax, C2_TYPE typeA, const void* B, const c2x* bx, C2_TYPE typeB, c2Manifold* m);
NEKO_C2_API int c2CastRay(c2Ray A, const void* B, const c2x* bx, C2_TYPE typeB, c2Raycast* out);

#ifdef _MSC_VER
#define C2_INLINE __forceinline
#else
#define C2_INLINE inline __attribute__((always_inline))
#endif

// adjust these primitives as seen fit
#include <math.h>
#include <string.h>  // memcpy
#define c2Sin(radians) sinf(radians)
#define c2Cos(radians) cosf(radians)
#define c2Sqrt(a) sqrtf(a)
#define c2Min(a, b) ((a) < (b) ? (a) : (b))
#define c2Max(a, b) ((a) > (b) ? (a) : (b))
#define c2Abs(a) ((a) < 0 ? -(a) : (a))
#define c2Clamp(a, lo, hi) c2Max(lo, c2Min(a, hi))
C2_INLINE void c2SinCos(float radians, float* s, float* c) {
    *c = c2Cos(radians);
    *s = c2Sin(radians);
}
#define c2Sign(a) (a < 0 ? -1.0f : 1.0f)

// The rest of the functions in the header-only portion are all for internal use
// and use the author's personal naming conventions. It is recommended to use one's
// own math library instead of the one embedded here in cute_c2, but for those
// curious or interested in trying it out here's the details:

// The Mul functions are used to perform multiplication. x stands for transform,
// v stands for vector, s stands for scalar, r stands for rotation, h stands for
// halfspace and T stands for transpose.For example c2MulxvT stands for "multiply
// a transform with a vector, and transpose the transform".

// vector ops
C2_INLINE c2v c2V(float x, float y) {
    c2v a;
    a.x = x;
    a.y = y;
    return a;
}
C2_INLINE c2v c2Add(c2v a, c2v b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}
C2_INLINE c2v c2Sub(c2v a, c2v b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}
C2_INLINE float c2Dot(c2v a, c2v b) { return a.x * b.x + a.y * b.y; }
C2_INLINE c2v c2Mulvs(c2v a, float b) {
    a.x *= b;
    a.y *= b;
    return a;
}
C2_INLINE c2v c2Mulvv(c2v a, c2v b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}
C2_INLINE c2v c2Div(c2v a, float b) { return c2Mulvs(a, 1.0f / b); }
C2_INLINE c2v c2Skew(c2v a) {
    c2v b;
    b.x = -a.y;
    b.y = a.x;
    return b;
}
C2_INLINE c2v c2CCW90(c2v a) {
    c2v b;
    b.x = a.y;
    b.y = -a.x;
    return b;
}
C2_INLINE float c2Det2(c2v a, c2v b) { return a.x * b.y - a.y * b.x; }
C2_INLINE c2v c2Minv(c2v a, c2v b) { return c2V(c2Min(a.x, b.x), c2Min(a.y, b.y)); }
C2_INLINE c2v c2Maxv(c2v a, c2v b) { return c2V(c2Max(a.x, b.x), c2Max(a.y, b.y)); }
C2_INLINE c2v c2Clampv(c2v a, c2v lo, c2v hi) { return c2Maxv(lo, c2Minv(a, hi)); }
C2_INLINE c2v c2Absv(c2v a) { return c2V(c2Abs(a.x), c2Abs(a.y)); }
C2_INLINE float c2Hmin(c2v a) { return c2Min(a.x, a.y); }
C2_INLINE float c2Hmax(c2v a) { return c2Max(a.x, a.y); }
C2_INLINE float c2Len(c2v a) { return c2Sqrt(c2Dot(a, a)); }
C2_INLINE c2v c2Norm(c2v a) { return c2Div(a, c2Len(a)); }
C2_INLINE c2v c2SafeNorm(c2v a) {
    float sq = c2Dot(a, a);
    return sq ? c2Div(a, c2Len(a)) : c2V(0, 0);
}
C2_INLINE c2v c2Neg(c2v a) { return c2V(-a.x, -a.y); }
C2_INLINE c2v c2Lerp(c2v a, c2v b, float t) { return c2Add(a, c2Mulvs(c2Sub(b, a), t)); }
C2_INLINE int c2Parallel(c2v a, c2v b, float kTol) {
    float k = c2Len(a) / c2Len(b);
    b = c2Mulvs(b, k);
    if (c2Abs(a.x - b.x) < kTol && c2Abs(a.y - b.y) < kTol) return 1;
    return 0;
}

// rotation ops
C2_INLINE c2r c2Rot(float radians) {
    c2r r;
    c2SinCos(radians, &r.s, &r.c);
    return r;
}
C2_INLINE c2r c2RotIdentity(void) {
    c2r r;
    r.c = 1.0f;
    r.s = 0;
    return r;
}
C2_INLINE c2v c2RotX(c2r r) { return c2V(r.c, r.s); }
C2_INLINE c2v c2RotY(c2r r) { return c2V(-r.s, r.c); }
C2_INLINE c2v c2Mulrv(c2r a, c2v b) { return c2V(a.c * b.x - a.s * b.y, a.s * b.x + a.c * b.y); }
C2_INLINE c2v c2MulrvT(c2r a, c2v b) { return c2V(a.c * b.x + a.s * b.y, -a.s * b.x + a.c * b.y); }
C2_INLINE c2r c2Mulrr(c2r a, c2r b) {
    c2r c;
    c.c = a.c * b.c - a.s * b.s;
    c.s = a.s * b.c + a.c * b.s;
    return c;
}
C2_INLINE c2r c2MulrrT(c2r a, c2r b) {
    c2r c;
    c.c = a.c * b.c + a.s * b.s;
    c.s = a.c * b.s - a.s * b.c;
    return c;
}

C2_INLINE c2v c2Mulmv(c2m a, c2v b) {
    c2v c;
    c.x = a.x.x * b.x + a.y.x * b.y;
    c.y = a.x.y * b.x + a.y.y * b.y;
    return c;
}
C2_INLINE c2v c2MulmvT(c2m a, c2v b) {
    c2v c;
    c.x = a.x.x * b.x + a.x.y * b.y;
    c.y = a.y.x * b.x + a.y.y * b.y;
    return c;
}
C2_INLINE c2m c2Mulmm(c2m a, c2m b) {
    c2m c;
    c.x = c2Mulmv(a, b.x);
    c.y = c2Mulmv(a, b.y);
    return c;
}
C2_INLINE c2m c2MulmmT(c2m a, c2m b) {
    c2m c;
    c.x = c2MulmvT(a, b.x);
    c.y = c2MulmvT(a, b.y);
    return c;
}

// transform ops
C2_INLINE c2x c2xIdentity(void) {
    c2x x;
    x.p = c2V(0, 0);
    x.r = c2RotIdentity();
    return x;
}
C2_INLINE c2v c2Mulxv(c2x a, c2v b) { return c2Add(c2Mulrv(a.r, b), a.p); }
C2_INLINE c2v c2MulxvT(c2x a, c2v b) { return c2MulrvT(a.r, c2Sub(b, a.p)); }
C2_INLINE c2x c2Mulxx(c2x a, c2x b) {
    c2x c;
    c.r = c2Mulrr(a.r, b.r);
    c.p = c2Add(c2Mulrv(a.r, b.p), a.p);
    return c;
}
C2_INLINE c2x c2MulxxT(c2x a, c2x b) {
    c2x c;
    c.r = c2MulrrT(a.r, b.r);
    c.p = c2MulrvT(a.r, c2Sub(b.p, a.p));
    return c;
}
C2_INLINE c2x c2Transform(c2v p, float radians) {
    c2x x;
    x.r = c2Rot(radians);
    x.p = p;
    return x;
}

// halfspace ops
C2_INLINE c2v c2Origin(c2h h) { return c2Mulvs(h.n, h.d); }
C2_INLINE float c2Dist(c2h h, c2v p) { return c2Dot(h.n, p) - h.d; }
C2_INLINE c2v c2Project(c2h h, c2v p) { return c2Sub(p, c2Mulvs(h.n, c2Dist(h, p))); }
C2_INLINE c2h c2Mulxh(c2x a, c2h b) {
    c2h c;
    c.n = c2Mulrv(a.r, b.n);
    c.d = c2Dot(c2Mulxv(a, c2Origin(b)), c.n);
    return c;
}
C2_INLINE c2h c2MulxhT(c2x a, c2h b) {
    c2h c;
    c.n = c2MulrvT(a.r, b.n);
    c.d = c2Dot(c2MulxvT(a, c2Origin(b)), c.n);
    return c;
}
C2_INLINE c2v c2Intersect(c2v a, c2v b, float da, float db) { return c2Add(a, c2Mulvs(c2Sub(b, a), (da / (da - db)))); }

C2_INLINE void c2BBVerts(c2v* out, c2AABB* bb) {
    out[0] = bb->min;
    out[1] = c2V(bb->max.x, bb->min.y);
    out[2] = bb->max;
    out[3] = c2V(bb->min.x, bb->max.y);
}

C2_INLINE c2v top_left(c2AABB bb) { return c2V(bb.min.x, bb.max.y); }
C2_INLINE c2v top_right(c2AABB bb) { return c2V(bb.max.x, bb.max.y); }
C2_INLINE c2v bottom_left(c2AABB bb) { return c2V(bb.min.x, bb.min.y); }
C2_INLINE c2v bottom_right(c2AABB bb) { return c2V(bb.max.x, bb.min.y); }

#endif  // NEKO_PHYSICS_H
