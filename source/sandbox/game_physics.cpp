#include "game_physics.h"

#include <box2d/box2d.h>

#include "sandbox/hpp/neko_cpp_utils.hpp"

static void contact_run_cb(lua_State *L, s32 ref, s32 a, s32 b, s32 msgh) {
    if (ref != LUA_REFNIL) {
        assert(ref != 0);
        s32 type = lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (type != LUA_TFUNCTION) {
            luaL_error(L, "expected contact listener to be a callback");
            return;
        }
        s32 top = lua_gettop(L);
        lua_pushvalue(L, top + a);
        lua_pushvalue(L, top + b);
        lua_pcall(L, 2, 0, msgh);
    }
}

struct PhysicsContactListener : public b2ContactListener {
    lua_State *L = nullptr;
    neko_physics physics = {};
    s32 begin_contact_ref = LUA_REFNIL;
    s32 end_contact_ref = LUA_REFNIL;

    void setup_contact(b2Contact *contact, s32 *msgh, neko_physics_userdata **pud_a, neko_physics_userdata **pud_b) {
        lua_pushcfunction(L, luax_msgh);
        *msgh = lua_gettop(L);

        neko_physics a = physics_weak_copy(&physics);
        a.fixture = contact->GetFixtureA();

        neko_physics b = physics_weak_copy(&physics);
        b.fixture = contact->GetFixtureB();

        luax_new_userdata(L, a, "mt_b2_fixture");
        luax_new_userdata(L, b, "mt_b2_fixture");

        *pud_a = (neko_physics_userdata *)a.fixture->GetUserData().pointer;
        *pud_b = (neko_physics_userdata *)b.fixture->GetUserData().pointer;
    }

    void BeginContact(b2Contact *contact) {
        s32 msgh = 0;
        neko_physics_userdata *pud_a = nullptr;
        neko_physics_userdata *pud_b = nullptr;
        setup_contact(contact, &msgh, &pud_a, &pud_b);

        contact_run_cb(L, begin_contact_ref, -2, -1, msgh);
        if (pud_a) {
            contact_run_cb(L, pud_a->begin_contact_ref, -2, -1, msgh);
        }
        if (pud_b) {
            contact_run_cb(L, pud_b->begin_contact_ref, -1, -2, msgh);
        }

        lua_pop(L, 2);
    }

    void EndContact(b2Contact *contact) {
        s32 msgh = 0;
        neko_physics_userdata *pud_a = nullptr;
        neko_physics_userdata *pud_b = nullptr;
        setup_contact(contact, &msgh, &pud_a, &pud_b);

        contact_run_cb(L, end_contact_ref, -2, -1, msgh);
        if (pud_a) {
            contact_run_cb(L, pud_a->end_contact_ref, -2, -1, msgh);
        }
        if (pud_b) {
            contact_run_cb(L, pud_b->end_contact_ref, -1, -2, msgh);
        }

        lua_pop(L, 2);
    }
};

neko_physics physics_world_make(lua_State *L, b2Vec2 gravity, f32 meter) {
    neko_physics physics = {};
    physics.world = new b2World(gravity);
    physics.meter = meter;
    physics.contact_listener = new PhysicsContactListener;
    physics.contact_listener->L = L;
    physics.contact_listener->physics = physics_weak_copy(&physics);

    physics.world->SetContactListener(physics.contact_listener);

    return physics;
}

void physics_world_trash(lua_State *L, neko_physics *p) {
    if (p == nullptr) {
        return;
    }

    if (p->contact_listener->begin_contact_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, p->contact_listener->begin_contact_ref);
    }
    if (p->contact_listener->end_contact_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, p->contact_listener->end_contact_ref);
    }

    delete p->contact_listener;
    delete p->world;
    p->contact_listener = nullptr;
    p->world = nullptr;
}

void physics_world_begin_contact(lua_State *L, neko_physics *p, s32 arg) {
    if (p->contact_listener->begin_contact_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, p->contact_listener->begin_contact_ref);
    }

    lua_pushvalue(L, arg);
    s32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
    p->contact_listener->begin_contact_ref = ref;
}

void physics_world_end_contact(lua_State *L, neko_physics *p, s32 arg) {
    if (p->contact_listener->end_contact_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, p->contact_listener->end_contact_ref);
    }

    lua_pushvalue(L, arg);
    s32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
    p->contact_listener->end_contact_ref = ref;
}

neko_physics physics_weak_copy(neko_physics *p) {
    neko_physics physics = {};
    physics.world = p->world;
    physics.contact_listener = p->contact_listener;
    physics.meter = p->meter;
    return physics;
}

static void drop_physics_udata(lua_State *L, neko_physics_userdata *pud) {
    if (pud->type == LUA_TSTRING) {
        neko_safe_free(pud->str);
    }

    if (pud->begin_contact_ref != LUA_REFNIL) {
        assert(pud->begin_contact_ref != 0);
        luaL_unref(L, LUA_REGISTRYINDEX, pud->begin_contact_ref);
    }

    if (pud->end_contact_ref != LUA_REFNIL) {
        assert(pud->end_contact_ref != 0);
        luaL_unref(L, LUA_REGISTRYINDEX, pud->end_contact_ref);
    }
}

void physics_destroy_body(lua_State *L, neko_physics *physics) {
    neko::array<neko_physics_userdata *> puds = {};
    neko_defer(puds.trash());

    for (b2Fixture *f = physics->body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
        puds.push((neko_physics_userdata *)f->GetUserData().pointer);
    }

    puds.push((neko_physics_userdata *)physics->body->GetUserData().pointer);

    physics->world->DestroyBody(physics->body);
    physics->body = nullptr;

    for (neko_physics_userdata *pud : puds) {
        drop_physics_udata(L, pud);
        neko_safe_free(pud);
    }
}

neko_physics_userdata *physics_userdata(lua_State *L) {
    neko_physics_userdata *pud = (neko_physics_userdata *)neko_safe_malloc(sizeof(neko_physics_userdata));

    pud->type = lua_getfield(L, -1, "udata");
    switch (pud->type) {
        case LUA_TNUMBER:
            pud->num = luaL_checknumber(L, -1);
            break;
        case LUA_TSTRING:
            pud->str = neko::to_cstr(luaL_checkstring(L, -1)).data;
            break;
        default:
            break;
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "begin_contact");
    pud->begin_contact_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, -1, "end_contact");
    pud->end_contact_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    pud->ref_count = 1;
    return pud;
}

void physics_push_userdata(lua_State *L, u64 ptr) {
    neko_physics_userdata *pud = (neko_physics_userdata *)ptr;

    if (pud == nullptr) {
        lua_pushnil(L);
        return;
    }

    switch (pud->type) {
        case LUA_TNUMBER:
            lua_pushnumber(L, pud->num);
            break;
        case LUA_TSTRING:
            lua_pushstring(L, pud->str);
            break;
        default:
            lua_pushnil(L);
            break;
    }
}

void draw_fixtures_for_body(b2Body *body, f32 meter) {
    // for (b2Fixture *f = body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
    //     switch (f->GetType()) {
    //         case b2Shape::e_circle: {
    //             b2CircleShape *circle = (b2CircleShape *)f->GetShape();
    //             b2Vec2 pos = body->GetWorldPoint(circle->m_p);
    //             draw_line_circle(pos.x * meter, pos.y * meter, circle->m_radius * meter);
    //             break;
    //         }
    //         case b2Shape::e_polygon: {
    //             b2PolygonShape *poly = (b2PolygonShape *)f->GetShape();
    //             if (poly->m_count > 0) {
    //                 sgl_disable_texture();
    //                 sgl_begin_line_strip();
    //                 renderer_apply_color();
    //                 for (s32 i = 0; i < poly->m_count; i++) {
    //                     b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[i]);
    //                     renderer_push_xy(pos.x * meter, pos.y * meter);
    //                 }
    //                 b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[0]);
    //                 renderer_push_xy(pos.x * meter, pos.y * meter);
    //                 sgl_end();
    //             }
    //             break;
    //         }
    //         default:
    //             break;
    //     }
    // }
}
