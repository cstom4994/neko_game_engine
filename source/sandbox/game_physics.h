#ifndef GAME_PHYSICS_H
#define GAME_PHYSICS_H

// #include <box2d/box2d.h>

#include "engine/neko.hpp"
#include "engine/neko_api.h"
#include "engine/neko_engine.h"

#if 0

struct neko_physics_userdata {
    s32 begin_contact_ref;
    s32 end_contact_ref;

    s32 ref_count;
    s32 type;
    union {
        char *str;
        lua_Number num;
    };
};

struct PhysicsContactListener;

struct neko_physics {
    b2World *world;
    PhysicsContactListener *contact_listener;
    f32 meter;

    union {
        b2Body *body;
        b2Fixture *fixture;
    };
};

neko_physics physics_world_make(lua_State *L, b2Vec2 gravity, f32 meter);
void physics_world_trash(lua_State *L, neko_physics *p);
void physics_world_begin_contact(lua_State *L, neko_physics *p, s32 arg);
void physics_world_end_contact(lua_State *L, neko_physics *p, s32 arg);
neko_physics physics_weak_copy(neko_physics *p);

void physics_destroy_body(lua_State *L, neko_physics *physics);
neko_physics_userdata *physics_userdata(lua_State *L);
void physics_push_userdata(lua_State *L, u64 ptr);
void draw_fixtures_for_body(b2Body *body, f32 meter);

#endif

#endif