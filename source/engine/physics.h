#pragma once

#include "engine/bootstrap.h"

#ifdef NEKO_BOX2D

#include <box2d/box2d.h>

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
struct Box2DDebugDraw;
struct Physics {
    b2World *world;
    PhysicsContactListener *contact_listener;
    f32 meter;
    Box2DDebugDraw *debugDraw;

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

#endif