#include "engine/physics.h"

#include "engine/base/color.hpp"
#include "engine/base/math.hpp"
#include "engine/component.h"

#ifdef NEKO_BOX2D

#include <box2d/box2d.h>

static void contact_run_cb(lua_State *L, i32 ref, i32 a, i32 b, i32 msgh) {
    if (ref != LUA_REFNIL) {
        assert(ref != 0);
        i32 type = lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (type != LUA_TFUNCTION) {
            luaL_error(L, "expected contact listener to be a callback");
            return;
        }
        i32 top = lua_gettop(L);
        lua_pushvalue(L, top + a);
        lua_pushvalue(L, top + b);
        lua_pcall(L, 2, 0, msgh);
    }
}

struct PhysicsContactListener : public b2ContactListener {
    lua_State *L = nullptr;
    Physics physics = {};
    i32 begin_contact_ref = LUA_REFNIL;
    i32 end_contact_ref = LUA_REFNIL;

    void setup_contact(b2Contact *contact, i32 *msgh, PhysicsUserData **pud_a, PhysicsUserData **pud_b) {
        lua_pushcfunction(L, luax_msgh);
        *msgh = lua_gettop(L);

        Physics a = physics_weak_copy(&physics);
        a.fixture = contact->GetFixtureA();

        Physics b = physics_weak_copy(&physics);
        b.fixture = contact->GetFixtureB();

        luax_new_userdata(L, a, "mt_b2_fixture");
        luax_new_userdata(L, b, "mt_b2_fixture");

        *pud_a = (PhysicsUserData *)a.fixture->GetUserData().pointer;
        *pud_b = (PhysicsUserData *)b.fixture->GetUserData().pointer;
    }

    void BeginContact(b2Contact *contact) {
        i32 msgh = 0;
        PhysicsUserData *pud_a = nullptr;
        PhysicsUserData *pud_b = nullptr;
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
        i32 msgh = 0;
        PhysicsUserData *pud_a = nullptr;
        PhysicsUserData *pud_b = nullptr;
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

Physics physics_world_make(lua_State *L, b2Vec2 gravity, f32 meter) {
    Physics physics = {};
    physics.world = new b2World(gravity);
    physics.meter = meter;
    physics.contact_listener = new PhysicsContactListener;
    physics.contact_listener->L = L;
    physics.contact_listener->physics = physics_weak_copy(&physics);

    physics.world->SetContactListener(physics.contact_listener);

    return physics;
}

void physics_world_trash(lua_State *L, Physics *p) {
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

void physics_world_begin_contact(lua_State *L, Physics *p, i32 arg) {
    if (p->contact_listener->begin_contact_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, p->contact_listener->begin_contact_ref);
    }

    lua_pushvalue(L, arg);
    i32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
    p->contact_listener->begin_contact_ref = ref;
}

void physics_world_end_contact(lua_State *L, Physics *p, i32 arg) {
    if (p->contact_listener->end_contact_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, p->contact_listener->end_contact_ref);
    }

    lua_pushvalue(L, arg);
    i32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
    p->contact_listener->end_contact_ref = ref;
}

Physics physics_weak_copy(Physics *p) {
    Physics physics = {};
    physics.world = p->world;
    physics.contact_listener = p->contact_listener;
    physics.meter = p->meter;
    return physics;
}

static void drop_physics_udata(lua_State *L, PhysicsUserData *pud) {
    if (pud->type == LUA_TSTRING) {
        mem_free(pud->str);
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

void physics_destroy_body(lua_State *L, Physics *physics) {
    Array<PhysicsUserData *> puds = {};
    neko_defer(puds.trash());

    for (b2Fixture *f = physics->body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
        puds.push((PhysicsUserData *)f->GetUserData().pointer);
    }

    puds.push((PhysicsUserData *)physics->body->GetUserData().pointer);

    physics->world->DestroyBody(physics->body);
    physics->body = nullptr;

    for (PhysicsUserData *pud : puds) {
        drop_physics_udata(L, pud);
        mem_free(pud);
    }
}

PhysicsUserData *physics_userdata(lua_State *L) {
    PhysicsUserData *pud = (PhysicsUserData *)mem_alloc(sizeof(PhysicsUserData));

    pud->type = lua_getfield(L, -1, "udata");
    switch (pud->type) {
        case LUA_TNUMBER:
            pud->num = luaL_checknumber(L, -1);
            break;
        case LUA_TSTRING:
            pud->str = to_cstr(luaL_checkstring(L, -1)).data;
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
    PhysicsUserData *pud = (PhysicsUserData *)ptr;

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
    for (b2Fixture *f = body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
        switch (f->GetType()) {
            case b2Shape::e_circle: {
                b2CircleShape *circle = (b2CircleShape *)f->GetShape();
                b2Vec2 pos = body->GetWorldPoint(circle->m_p);
                // draw_line_circle(pos.x * meter, pos.y * meter, circle->m_radius * meter);
                break;
            }
            case b2Shape::e_polygon: {
                b2PolygonShape *poly = (b2PolygonShape *)f->GetShape();

                if (poly->m_count > 0) {
                    // sgl_disable_texture();
                    // sgl_begin_line_strip();
                    // renderer_apply_color();

                    for (i32 i = 0; i < poly->m_count; i++) {
                        b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[i]);
                        // renderer_push_xy(pos.x * meter, pos.y * meter);

                        edit_line_add_xy(luavec2(pos.x * meter, pos.y * meter), 1.f, color_red);
                    }

                    b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[0]);
                    edit_line_add_xy(luavec2(pos.x * meter, pos.y * meter), 1.f, color_red);

                    // sgl_end();
                }
                break;
            }
            default:
                break;
        }
    }
}

// box2d fixture

static int mt_b2_fixture_friction(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    f32 friction = fixture->GetFriction();
    lua_pushnumber(L, friction);
    return 1;
}

static int mt_b2_fixture_restitution(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    f32 restitution = fixture->GetRestitution();
    lua_pushnumber(L, restitution);
    return 1;
}

static int mt_b2_fixture_is_sensor(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    f32 sensor = fixture->IsSensor();
    lua_pushnumber(L, sensor);
    return 1;
}

static int mt_b2_fixture_set_friction(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    f32 friction = luaL_checknumber(L, 2);
    fixture->SetFriction(friction);
    return 0;
}

static int mt_b2_fixture_set_restitution(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    f32 restitution = luaL_checknumber(L, 2);
    fixture->SetRestitution(restitution);
    return 0;
}

static int mt_b2_fixture_set_sensor(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    bool sensor = lua_toboolean(L, 2);
    fixture->SetSensor(sensor);
    return 0;
}

static int mt_b2_fixture_body(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    b2Body *body = fixture->GetBody();

    PhysicsUserData *pud = (PhysicsUserData *)body->GetUserData().pointer;
    assert(pud != nullptr);
    pud->ref_count++;

    Physics p = physics_weak_copy(physics);
    p.body = body;

    luax_new_userdata(L, p, "mt_b2_body");
    return 1;
}

static int mt_b2_fixture_udata(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture *fixture = physics->fixture;

    physics_push_userdata(L, fixture->GetUserData().pointer);
    return 1;
}

int open_mt_b2_fixture(lua_State *L) {
    luaL_Reg reg[] = {
            {"friction", mt_b2_fixture_friction},
            {"restitution", mt_b2_fixture_restitution},
            {"is_sensor", mt_b2_fixture_is_sensor},
            {"set_friction", mt_b2_fixture_set_friction},
            {"set_restitution", mt_b2_fixture_set_restitution},
            {"set_sensor", mt_b2_fixture_set_sensor},
            {"body", mt_b2_fixture_body},
            {"udata", mt_b2_fixture_udata},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_b2_fixture", reg);
    return 0;
}

// box2d body

static int b2_body_unref(lua_State *L, bool destroy) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    if (physics->body != nullptr) {
        PhysicsUserData *pud = (PhysicsUserData *)physics->body->GetUserData().pointer;
        assert(pud != nullptr);
        pud->ref_count--;

        if (pud->ref_count == 0 || destroy) {
            physics_destroy_body(L, physics);
        }
    }

    return 0;
}

static int mt_b2_body_gc(lua_State *L) { return b2_body_unref(L, false); }

static int mt_b2_body_destroy(lua_State *L) { return b2_body_unref(L, true); }

static b2FixtureDef b2_fixture_def(lua_State *L, i32 arg) {
    bool sensor = luax_boolean_field(L, arg, "sensor");
    lua_Number density = luax_opt_number_field(L, arg, "density", 1);
    lua_Number friction = luax_opt_number_field(L, arg, "friction", 0.2);
    lua_Number restitution = luax_opt_number_field(L, arg, "restitution", 0);
    PhysicsUserData *pud = physics_userdata(L);

    b2FixtureDef def = {};
    def.isSensor = sensor;
    def.density = (f32)density;
    def.friction = (f32)friction;
    def.restitution = (f32)restitution;
    def.userData.pointer = (u64)pud;
    return def;
}

static int mt_b2_body_make_box_fixture(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;
    b2FixtureDef fixture_def = b2_fixture_def(L, 2);

    lua_Number x = luax_opt_number_field(L, 2, "x", 0);
    lua_Number y = luax_opt_number_field(L, 2, "y", 0);
    lua_Number w = luax_number_field(L, 2, "w");
    lua_Number h = luax_number_field(L, 2, "h");
    lua_Number angle = luax_opt_number_field(L, 2, "angle", 0);

    b2Vec2 pos = {(f32)x / physics->meter, (f32)y / physics->meter};

    b2PolygonShape box = {};
    box.SetAsBox((f32)w / physics->meter, (f32)h / physics->meter, pos, angle);
    fixture_def.shape = &box;

    Physics p = physics_weak_copy(physics);
    p.fixture = body->CreateFixture(&fixture_def);

    luax_new_userdata(L, p, "mt_b2_fixture");
    return 0;
}

static int mt_b2_body_make_circle_fixture(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;
    b2FixtureDef fixture_def = b2_fixture_def(L, 2);

    lua_Number x = luax_opt_number_field(L, 2, "x", 0);
    lua_Number y = luax_opt_number_field(L, 2, "y", 0);
    lua_Number radius = luax_number_field(L, 2, "radius");

    b2CircleShape circle = {};
    circle.m_radius = radius / physics->meter;
    circle.m_p = {(f32)x / physics->meter, (f32)y / physics->meter};
    fixture_def.shape = &circle;

    Physics p = physics_weak_copy(physics);
    p.fixture = body->CreateFixture(&fixture_def);

    luax_new_userdata(L, p, "mt_b2_fixture");
    return 0;
}

static int mt_b2_body_position(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    b2Vec2 pos = body->GetPosition();

    lua_pushnumber(L, pos.x * physics->meter);
    lua_pushnumber(L, pos.y * physics->meter);
    return 2;
}

static int mt_b2_body_velocity(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    b2Vec2 vel = body->GetLinearVelocity();

    lua_pushnumber(L, vel.x * physics->meter);
    lua_pushnumber(L, vel.y * physics->meter);
    return 2;
}

static int mt_b2_body_angle(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    lua_pushnumber(L, body->GetAngle());
    return 1;
}

static int mt_b2_body_linear_damping(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    lua_pushnumber(L, body->GetLinearDamping());
    return 1;
}

static int mt_b2_body_fixed_rotation(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    lua_pushboolean(L, body->IsFixedRotation());
    return 1;
}

static int mt_b2_body_apply_force(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 x = luaL_checknumber(L, 2);
    f32 y = luaL_checknumber(L, 3);

    body->ApplyForceToCenter({x / physics->meter, y / physics->meter}, false);
    return 0;
}

static int mt_b2_body_apply_impulse(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 x = luaL_checknumber(L, 2);
    f32 y = luaL_checknumber(L, 3);

    body->ApplyLinearImpulseToCenter({x / physics->meter, y / physics->meter}, false);
    return 0;
}

static int mt_b2_body_set_position(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 x = luaL_checknumber(L, 2);
    f32 y = luaL_checknumber(L, 3);

    body->SetTransform({x / physics->meter, y / physics->meter}, body->GetAngle());
    return 0;
}

static int mt_b2_body_set_velocity(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 x = luaL_checknumber(L, 2);
    f32 y = luaL_checknumber(L, 3);

    body->SetLinearVelocity({x / physics->meter, y / physics->meter});
    return 0;
}

static int mt_b2_body_set_angle(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 angle = luaL_checknumber(L, 2);

    body->SetTransform(body->GetPosition(), angle);
    return 0;
}

static int mt_b2_body_set_linear_damping(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 damping = luaL_checknumber(L, 2);

    body->SetLinearDamping(damping);
    return 0;
}

static int mt_b2_body_set_fixed_rotation(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    bool fixed = lua_toboolean(L, 2);

    body->SetFixedRotation(fixed);
    return 0;
}

static int mt_b2_body_set_transform(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    f32 x = luaL_checknumber(L, 2);
    f32 y = luaL_checknumber(L, 3);
    f32 angle = luaL_checknumber(L, 4);

    body->SetTransform({x / physics->meter, y / physics->meter}, angle);
    return 0;
}

static int mt_b2_body_draw_fixtures(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    draw_fixtures_for_body(body, physics->meter);

    return 0;
}

static int mt_b2_body_udata(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body *body = physics->body;

    physics_push_userdata(L, body->GetUserData().pointer);
    return 1;
}

int open_mt_b2_body(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_b2_body_gc},
            {"destroy", mt_b2_body_destroy},
            {"make_box_fixture", mt_b2_body_make_box_fixture},
            {"make_circle_fixture", mt_b2_body_make_circle_fixture},
            {"position", mt_b2_body_position},
            {"velocity", mt_b2_body_velocity},
            {"angle", mt_b2_body_angle},
            {"linear_damping", mt_b2_body_linear_damping},
            {"fixed_rotation", mt_b2_body_fixed_rotation},
            {"apply_force", mt_b2_body_apply_force},
            {"apply_impulse", mt_b2_body_apply_impulse},
            {"set_position", mt_b2_body_set_position},
            {"set_velocity", mt_b2_body_set_velocity},
            {"set_angle", mt_b2_body_set_angle},
            {"set_linear_damping", mt_b2_body_set_linear_damping},
            {"set_fixed_rotation", mt_b2_body_set_fixed_rotation},
            {"set_transform", mt_b2_body_set_transform},
            {"draw_fixtures", mt_b2_body_draw_fixtures},
            {"udata", mt_b2_body_udata},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_b2_body", reg);
    return 0;
}

// box2d world

static int mt_b2_world_gc(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
    physics_world_trash(L, physics);
    return 0;
}

static int mt_b2_world_step(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
    lua_Number dt = luaL_optnumber(L, 2, get_timing_instance()->dt);
    lua_Integer vel_iters = luaL_optinteger(L, 3, 6);
    lua_Integer pos_iters = luaL_optinteger(L, 4, 2);

    physics->world->Step((f32)dt, (i32)vel_iters, (i32)pos_iters);
    return 0;
}

static b2BodyDef b2_body_def(lua_State *L, i32 arg, Physics *physics) {
    lua_Number x = luax_number_field(L, arg, "x");
    lua_Number y = luax_number_field(L, arg, "y");
    lua_Number vx = luax_opt_number_field(L, arg, "vx", 0);
    lua_Number vy = luax_opt_number_field(L, arg, "vy", 0);
    lua_Number angle = luax_opt_number_field(L, arg, "angle", 0);
    lua_Number linear_damping = luax_opt_number_field(L, arg, "linear_damping", 0);
    bool fixed_rotation = luax_boolean_field(L, arg, "fixed_rotation");
    PhysicsUserData *pud = physics_userdata(L);

    b2BodyDef def = {};
    def.position.Set((f32)x / physics->meter, (f32)y / physics->meter);
    def.linearVelocity.Set((f32)vx / physics->meter, (f32)vy / physics->meter);
    def.angle = angle;
    def.linearDamping = linear_damping;
    def.fixedRotation = fixed_rotation;
    def.userData.pointer = (u64)pud;
    return def;
}

static int b2_make_body(lua_State *L, b2BodyType type) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
    b2BodyDef body_def = b2_body_def(L, 2, physics);
    body_def.type = type;

    Physics p = physics_weak_copy(physics);
    p.body = physics->world->CreateBody(&body_def);

    luax_new_userdata(L, p, "mt_b2_body");
    return 1;
}

static int mt_b2_world_make_static_body(lua_State *L) { return b2_make_body(L, b2_staticBody); }

static int mt_b2_world_make_kinematic_body(lua_State *L) { return b2_make_body(L, b2_kinematicBody); }

static int mt_b2_world_make_dynamic_body(lua_State *L) { return b2_make_body(L, b2_dynamicBody); }

static int mt_b2_world_begin_contact(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
    if (lua_type(L, 2) != LUA_TFUNCTION) {
        return luaL_error(L, "expected argument 2 to be a function");
    }

    physics_world_begin_contact(L, physics, 2);
    return 0;
}

static int mt_b2_world_end_contact(lua_State *L) {
    Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
    if (lua_type(L, 2) != LUA_TFUNCTION) {
        return luaL_error(L, "expected argument 2 to be a function");
    }

    physics_world_end_contact(L, physics, 2);
    return 0;
}

int open_mt_b2_world(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_b2_world_gc},
            {"destroy", mt_b2_world_gc},
            {"step", mt_b2_world_step},
            {"make_static_body", mt_b2_world_make_static_body},
            {"make_kinematic_body", mt_b2_world_make_kinematic_body},
            {"make_dynamic_body", mt_b2_world_make_dynamic_body},
            {"begin_contact", mt_b2_world_begin_contact},
            {"end_contact", mt_b2_world_end_contact},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_b2_world", reg);
    return 0;
}

#endif
