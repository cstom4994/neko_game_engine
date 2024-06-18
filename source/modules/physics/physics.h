#ifndef GAME_PHYSICS_H
#define GAME_PHYSICS_H

#include <box2d/box2d.h>

#include "engine/neko.hpp"
#include "engine/neko_api.hpp"
#include "engine/neko_engine.h"

#if defined(_WIN32) || defined(_WIN64)
#define NEKO_DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define NEKO_DLL_EXPORT extern "C"
#endif

NEKO_API_EXTERN Neko_ModuleInterface *g_interface;

#undef neko_safe_malloc
#undef neko_safe_free
#undef neko_safe_realloc
#undef neko_safe_calloc

#define neko_safe_malloc(size) g_interface->common.__neko_mem_safe_alloc((size), (char *)__FILE__, __LINE__, NULL)
#define neko_safe_free(mem) g_interface->common.__neko_mem_safe_free((void *)mem, NULL)
#define neko_safe_realloc(ptr, size) g_interface->common.__neko_mem_safe_realloc((ptr), (size), (char *)__FILE__, __LINE__, NULL)
#define neko_safe_calloc(count, element_size) g_interface->common.__neko_mem_safe_calloc(count, element_size, (char *)__FILE__, __LINE__, NULL)

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
void physics_world_fini(lua_State *L, neko_physics *p);
void physics_world_begin_contact(lua_State *L, neko_physics *p, s32 arg);
void physics_world_end_contact(lua_State *L, neko_physics *p, s32 arg);
neko_physics physics_weak_copy(neko_physics *p);

void physics_destroy_body(lua_State *L, neko_physics *physics);
neko_physics_userdata *physics_userdata(lua_State *L);
void physics_push_userdata(lua_State *L, u64 ptr);
void draw_fixtures_for_body(b2Body *body, f32 meter);

#endif