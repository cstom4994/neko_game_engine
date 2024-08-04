#pragma once

#include <box2d/box2d.h>

#include "engine/neko_lua.h"
#include "engine/neko_prelude.h"
#include "engine/neko_base.h"
#include "engine/neko_prelude.h"

struct PhysicsUserData {
    i32 begin_contact_ref;
    i32 end_contact_ref;

    i32 ref_count;
    i32 type;
    union {
        char *str;
        lua_Number num;
    };
};

struct PhysicsContactListener;
struct Physics {
    b2World *world;
    PhysicsContactListener *contact_listener;
    f32 meter;

    union {
        b2Body *body;
        b2Fixture *fixture;
    };
};

Physics physics_world_make(lua_State *L, b2Vec2 gravity, f32 meter);
void physics_world_trash(lua_State *L, Physics *p);
void physics_world_begin_contact(lua_State *L, Physics *p, i32 arg);
void physics_world_end_contact(lua_State *L, Physics *p, i32 arg);
Physics physics_weak_copy(Physics *p);

void physics_destroy_body(lua_State *L, Physics *physics);
PhysicsUserData *physics_userdata(lua_State *L);
void physics_push_userdata(lua_State *L, u64 ptr);
void draw_fixtures_for_body(b2Body *body, f32 meter);

int open_mt_b2_world(lua_State *L);
int open_mt_b2_body(lua_State *L);
int open_mt_b2_fixture(lua_State *L);

inline int neko_b2_world(lua_State *L) {
    lua_Number gx = luax_opt_number_field(L, 1, "gx", 0);
    lua_Number gy = luax_opt_number_field(L, 1, "gy", 9.81);
    lua_Number meter = luax_opt_number_field(L, 1, "meter", 16);

    b2Vec2 gravity = {(f32)gx, (f32)gy};

    Physics p = physics_world_make(L, gravity, meter);
    luax_new_userdata(L, p, "mt_b2_world");
    return 1;
}

#if 0

SCRIPT(physics,

       // global

       NEKO_EXPORT void physics_set_gravity(CVec2 g);
       NEKO_EXPORT CVec2 physics_get_gravity();
       NEKO_EXPORT void physics_set_simulation_frequency(Scalar freq);
       NEKO_EXPORT Scalar physics_get_simulation_frequency();


       // add/remove body

       typedef enum PhysicsBody PhysicsBody;
       enum PhysicsBody
       {
           PB_STATIC    = 0,   // never (or rarely) moves -- eg. wall
                               
           PB_KINEMATIC = 1,   /* moves but not dynamic -- eg. moving platform,
                                  can be moved through transform system */
                               
           PB_DYNAMIC   = 2,   /* moves subject to dynamics -- eg. bowling pin,
                                  avoid moving through transform system
                                  use forces, velocities, etc. instead */
       };

       NEKO_EXPORT void physics_add(Entity ent); // PB_DYNAMIC by default
       NEKO_EXPORT void physics_remove(Entity ent);
       NEKO_EXPORT bool physics_has(Entity ent);

       NEKO_EXPORT void physics_set_type(Entity ent, PhysicsBody type);
       NEKO_EXPORT PhysicsBody physics_get_type(Entity ent);

       // draws this object for one frame
       NEKO_EXPORT void physics_debug_draw(Entity ent);


       // shape

       // shapes are indexed 0 - n
       typedef enum PhysicsShape PhysicsShape;
       enum PhysicsShape
       {
           PS_CIRCLE  = 0,
           PS_POLYGON = 1,
       };

       NEKO_EXPORT unsigned int physics_shape_add_circle(Entity ent, Scalar r,
                                                    CVec2 offset);

       // 'r' is a 'rounding radius' added polygon to the polygon smooth
       NEKO_EXPORT unsigned int physics_shape_add_box(Entity ent, BBox b, Scalar r);
       NEKO_EXPORT unsigned int physics_shape_add_poly(Entity ent,
                                                  unsigned int nverts,
                                                  const CVec2 *verts,
                                                  Scalar r);

       NEKO_EXPORT unsigned int physics_get_num_shapes(Entity ent);
       NEKO_EXPORT PhysicsShape physics_shape_get_type(Entity ent, unsigned int i);
       NEKO_EXPORT void physics_shape_remove(Entity ent, unsigned int i);

       // -1 if not a PS_POLYGON
       NEKO_EXPORT int physics_poly_get_num_verts(Entity ent, unsigned int i);

       // modifies array in-place, returns number of convex hull vertices
       NEKO_EXPORT unsigned int physics_convex_hull(unsigned int nverts,
                                               CVec2 *verts);

       NEKO_EXPORT void physics_shape_set_sensor(Entity ent,
                                            unsigned int i,
                                            bool sensor);
       NEKO_EXPORT bool physics_shape_get_sensor(Entity ent,
                                            unsigned int i);

       NEKO_EXPORT void physics_shape_set_surface_velocity(Entity ent,
                                                      unsigned int i,
                                                      CVec2 v);
       NEKO_EXPORT CVec2 physics_shape_get_surface_velocity(Entity ent,
                                                      unsigned int i);

       // dynamics

       NEKO_EXPORT void physics_set_mass(Entity ent, Scalar mass);
       NEKO_EXPORT Scalar physics_get_mass(Entity ent);

       NEKO_EXPORT void physics_set_freeze_rotation(Entity ent, bool freeze);
       NEKO_EXPORT bool physics_get_freeze_rotation(Entity ent);

       NEKO_EXPORT void physics_set_velocity(Entity ent, CVec2 vel);
       NEKO_EXPORT CVec2 physics_get_velocity(Entity ent);
       NEKO_EXPORT void physics_set_force(Entity ent, CVec2 force);
       NEKO_EXPORT CVec2 physics_get_force(Entity ent);

       NEKO_EXPORT void physics_set_angular_velocity(Entity ent, Scalar ang_vel);
       NEKO_EXPORT Scalar physics_get_angular_velocity(Entity ent);
       NEKO_EXPORT void physics_set_torque(Entity ent, Scalar torque);
       NEKO_EXPORT Scalar physics_get_torque(Entity ent);

       NEKO_EXPORT void physics_set_velocity_limit(Entity ent, Scalar lim);
       NEKO_EXPORT Scalar physics_get_velocity_limit(Entity ent);
       NEKO_EXPORT void physics_set_angular_velocity_limit(Entity ent, Scalar lim);
       NEKO_EXPORT Scalar physics_get_angular_velocity_limit(Entity ent);

       NEKO_EXPORT void physics_reset_forces(Entity ent);
       NEKO_EXPORT void physics_apply_force(Entity ent, CVec2 force);
       NEKO_EXPORT void physics_apply_force_at(Entity ent, CVec2 force, CVec2 at);
       NEKO_EXPORT void physics_apply_impulse(Entity ent, CVec2 impulse);
       NEKO_EXPORT void physics_apply_impulse_at(Entity ent, CVec2 impulse, CVec2 at);


       // collisions

       typedef struct Collision Collision;
       struct Collision
       {
           Entity a, b;
       };

       NEKO_EXPORT unsigned int physics_get_num_collisions(Entity ent);
       NEKO_EXPORT Collision *physics_get_collisions(Entity ent);


       // nearest query

       typedef struct NearestResult NearestResult;
       struct NearestResult
       {
           Entity ent; // closest entity or entity_nil if none in range
           CVec2 p; // closest point on shape surface
           Scalar d; // distance to point, negative if inside
           CVec2 g; // gradient of distance function
       };

       NEKO_EXPORT NearestResult physics_nearest(CVec2 point, Scalar max_dist);


    )

#endif

void physics_init();
void physics_fini();
void physics_update_all();
void physics_post_update_all();
void physics_draw_all();
void physics_save_all(Store *s);
void physics_load_all(Store *s);
