#ifndef NEKO_TEST_MODULES
#define NEKO_TEST_MODULES

#include "engine/base.h"
#include "engine/event.h"
#include "engine/scripting/luax.h"

#if 0

NEKO_SCRIPT(physics,

       // global

       NEKO_EXPORT void physics_set_gravity(vec2 g);
       NEKO_EXPORT vec2 physics_get_gravity();
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

       NEKO_EXPORT void physics_add(NativeEntity ent); // PB_DYNAMIC by default
       NEKO_EXPORT void physics_remove(NativeEntity ent);
       NEKO_EXPORT bool physics_has(NativeEntity ent);

       NEKO_EXPORT void physics_set_type(NativeEntity ent, PhysicsBody type);
       NEKO_EXPORT PhysicsBody physics_get_type(NativeEntity ent);

       // draws this object for one frame
       NEKO_EXPORT void physics_debug_draw(NativeEntity ent);


       // shape

       // shapes are indexed 0 - n
       typedef enum PhysicsShape PhysicsShape;
       enum PhysicsShape
       {
           PS_CIRCLE  = 0,
           PS_POLYGON = 1,
       };

       NEKO_EXPORT unsigned int physics_shape_add_circle(NativeEntity ent, Scalar r,
                                                    vec2 offset);

       // 'r' is a 'rounding radius' added polygon to the polygon smooth
       NEKO_EXPORT unsigned int physics_shape_add_box(NativeEntity ent, BBox b, Scalar r);
       NEKO_EXPORT unsigned int physics_shape_add_poly(NativeEntity ent,
                                                  unsigned int nverts,
                                                  const vec2 *verts,
                                                  Scalar r);

       NEKO_EXPORT unsigned int physics_get_num_shapes(NativeEntity ent);
       NEKO_EXPORT PhysicsShape physics_shape_get_type(NativeEntity ent, unsigned int i);
       NEKO_EXPORT void physics_shape_remove(NativeEntity ent, unsigned int i);

       // -1 if not a PS_POLYGON
       NEKO_EXPORT int physics_poly_get_num_verts(NativeEntity ent, unsigned int i);

       // modifies array in-place, returns number of convex hull vertices
       NEKO_EXPORT unsigned int physics_convex_hull(unsigned int nverts,
                                               vec2 *verts);

       NEKO_EXPORT void physics_shape_set_sensor(NativeEntity ent,
                                            unsigned int i,
                                            bool sensor);
       NEKO_EXPORT bool physics_shape_get_sensor(NativeEntity ent,
                                            unsigned int i);

       NEKO_EXPORT void physics_shape_set_surface_velocity(NativeEntity ent,
                                                      unsigned int i,
                                                      vec2 v);
       NEKO_EXPORT vec2 physics_shape_get_surface_velocity(NativeEntity ent,
                                                      unsigned int i);

       // dynamics

       NEKO_EXPORT void physics_set_mass(NativeEntity ent, Scalar mass);
       NEKO_EXPORT Scalar physics_get_mass(NativeEntity ent);

       NEKO_EXPORT void physics_set_freeze_rotation(NativeEntity ent, bool freeze);
       NEKO_EXPORT bool physics_get_freeze_rotation(NativeEntity ent);

       NEKO_EXPORT void physics_set_velocity(NativeEntity ent, vec2 vel);
       NEKO_EXPORT vec2 physics_get_velocity(NativeEntity ent);
       NEKO_EXPORT void physics_set_force(NativeEntity ent, vec2 force);
       NEKO_EXPORT vec2 physics_get_force(NativeEntity ent);

       NEKO_EXPORT void physics_set_angular_velocity(NativeEntity ent, Scalar ang_vel);
       NEKO_EXPORT Scalar physics_get_angular_velocity(NativeEntity ent);
       NEKO_EXPORT void physics_set_torque(NativeEntity ent, Scalar torque);
       NEKO_EXPORT Scalar physics_get_torque(NativeEntity ent);

       NEKO_EXPORT void physics_set_velocity_limit(NativeEntity ent, Scalar lim);
       NEKO_EXPORT Scalar physics_get_velocity_limit(NativeEntity ent);
       NEKO_EXPORT void physics_set_angular_velocity_limit(NativeEntity ent, Scalar lim);
       NEKO_EXPORT Scalar physics_get_angular_velocity_limit(NativeEntity ent);

       NEKO_EXPORT void physics_reset_forces(NativeEntity ent);
       NEKO_EXPORT void physics_apply_force(NativeEntity ent, vec2 force);
       NEKO_EXPORT void physics_apply_force_at(NativeEntity ent, vec2 force, vec2 at);
       NEKO_EXPORT void physics_apply_impulse(NativeEntity ent, vec2 impulse);
       NEKO_EXPORT void physics_apply_impulse_at(NativeEntity ent, vec2 impulse, vec2 at);


       // collisions

       typedef struct Collision Collision;
       struct Collision
       {
           NativeEntity a, b;
       };

       NEKO_EXPORT unsigned int physics_get_num_collisions(NativeEntity ent);
       NEKO_EXPORT Collision *physics_get_collisions(NativeEntity ent);


       // nearest query

       typedef struct NearestResult NearestResult;
       struct NearestResult
       {
           NativeEntity ent; // closest entity or entity_nil if none in range
           vec2 p; // closest point on shape surface
           Scalar d; // distance to point, negative if inside
           vec2 g; // gradient of distance function
       };

       NEKO_EXPORT NearestResult physics_nearest(vec2 point, Scalar max_dist);


    )

#endif

struct App;
NEKO_API() void physics_init();
NEKO_API() void physics_fini();
NEKO_API() int physics_update_all(App *app, event_t evt);
NEKO_API() int physics_post_update_all(App *app, event_t evt);
NEKO_API() void physics_draw_all();
NEKO_API() void physics_save_all(Store *s);
NEKO_API() void physics_load_all(Store *s);

#endif