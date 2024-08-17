#include "engine/physics.h"

#include "engine/draw.h"
#include "engine/game.h"
#include "engine/lua_util.h"
#include "engine/luax.h"
#include "engine/os.h"

#ifdef NEKO_BOX2D

#include <box2d/box2d.h>
// #include <sokol_gfx.h>
// #include <util/sokol_gl.h>

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

                // if (poly->m_count > 0) {
                //     sgl_disable_texture();
                //     sgl_begin_line_strip();

                //     renderer_apply_color();

                //     for (i32 i = 0; i < poly->m_count; i++) {
                //         b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[i]);
                //         renderer_push_xy(pos.x * meter, pos.y * meter);
                //     }

                //     b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[0]);
                //     renderer_push_xy(pos.x * meter, pos.y * meter);

                //     sgl_end();
                // }
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
    lua_Number dt = luaL_optnumber(L, 2, timing_instance.dt);
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

#if 0

#include <stdlib.h>

#define CP_DATA_POINTER_TYPE Entity
#include <chipmunk/chipmunk.h>


// per-entity info
typedef struct PhysicsInfo PhysicsInfo;
struct PhysicsInfo
{
    EntityPoolElem pool_elem;

    PhysicsBody type;

    // store mass separately to convert to/from PB_DYNAMIC
    Scalar mass;

    // used to compute (angular) velocitiy for PB_KINEMATIC
    cpVect last_pos;
    cpFloat last_ang;

    // used to keep track of transform <-> physics update
    unsigned int last_dirty_count;

    cpBody *body;
    CArray *shapes;
    CArray *collisions;
};

// per-shape info for each shape attached to a physics entity
typedef struct ShapeInfo ShapeInfo;
struct ShapeInfo
{
    PhysicsShape type;
    cpShape *shape;
};

static cpSpace *space;
static Scalar period = 1.0 / 60.0; // 1.0 / simulation_frequency
static EntityPool *pool;

static EntityMap *debug_draw_map;

// -------------------------------------------------------------------------

// chipmunk utilities

static inline cpVect cpv_of_vec2(LuaVec2 v) { return cpv(v.x, v.y); }
static inline LuaVec2 vec2_of_cpv(cpVect v) { return luavec2(v.x, v.y); }

static inline void _remove_body(cpBody *body)
{
    if (cpSpaceContainsBody(space, body))
        cpSpaceRemoveBody(space, body);
    cpBodyFree(body);
}
static inline void _remove_shape(cpShape *shape)
{
    if (cpSpaceContainsShape(space, shape))
        cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

// -------------------------------------------------------------------------

void physics_set_gravity(LuaVec2 g)
{
    cpSpaceSetGravity(space, cpv_of_vec2(g));
}
LuaVec2 physics_get_gravity()
{
    return vec2_of_cpv(cpSpaceGetGravity(space));
}

void physics_set_simulation_frequency(Scalar freq)
{
    period = 1.0 / freq;
}
Scalar physics_get_simulation_frequency()
{
    return 1.0 / period;
}

void physics_add(Entity ent)
{
    PhysicsInfo *info;

    if (entitypool_get(pool, ent))
        return; // already has physics

    transform_add(ent);

    info = entitypool_add(pool, ent);

    info->mass = 1.0;
    info->type = PB_DYNAMIC;

    // create, init cpBody
    info->body = cpSpaceAddBody(space, cpBodyNew(info->mass, 1.0));
    cpBodySetUserData(info->body, ent); // for cpBody -> Entity mapping
    cpBodySetPos(info->body, cpv_of_vec2(transform_get_position(ent)));
    cpBodySetAngle(info->body, transform_get_rotation(ent));
    info->last_dirty_count = transform_get_dirty_count(ent);

    // initially no shapes
    info->shapes = array_new(ShapeInfo);

    // initialize last_pos/last_ang info for kinematic bodies
    info->last_pos = cpBodyGetPos(info->body);
    info->last_ang = cpBodyGetAngle(info->body);

    info->collisions = NULL;
}

// remove chipmunk stuff (doesn't remove from pool)
static void _remove(PhysicsInfo *info)
{
    ShapeInfo *shapeInfo;

    array_foreach(shapeInfo, info->shapes)
        _remove_shape(shapeInfo->shape);
    array_free(info->shapes);

    _remove_body(info->body);
}

void physics_remove(Entity ent)
{
    PhysicsInfo *info;

    info = entitypool_get(pool, ent);
    if (!info)
        return;

    _remove(info);
    entitypool_remove(pool, ent);
}

bool physics_has(Entity ent)
{
    return entitypool_get(pool, ent) != NULL;
}

// calculate moment for a single shape
static Scalar _moment(cpBody *body, ShapeInfo *shapeInfo)
{
    Scalar mass = cpBodyGetMass(body);
    switch (shapeInfo->type)
    {
        case PS_CIRCLE:
            return cpMomentForCircle(mass, 0,
                                     cpCircleShapeGetRadius(shapeInfo->shape),
                                     cpCircleShapeGetOffset(shapeInfo->shape));

        case PS_POLYGON:
            return cpMomentForPoly(mass,
                                   cpPolyShapeGetNumVerts(shapeInfo->shape),
                                   ((cpPolyShape *) shapeInfo->shape)->verts,
                                   cpvzero);
    }
}

// recalculate moment for whole body by adding up shape moments
static void _recalculate_moment(PhysicsInfo *info)
{
    Scalar moment;
    ShapeInfo *shapeInfo;

    if (!info->body)
        return;
    if (array_length(info->shapes) == 0)
        return; // can't set moment to zero, just leave it alone

    moment = 0.0;
    array_foreach(shapeInfo, info->shapes)
        moment += _moment(info->body, shapeInfo);
    cpBodySetMoment(info->body, moment);
}

static void _set_type(PhysicsInfo *info, PhysicsBody type)
{
    if (info->type == type)
        return; // already set

    info->type = type;
    switch (type)
    {
        case PB_KINEMATIC:
            info->last_pos = cpBodyGetPos(info->body);
            info->last_ang = cpBodyGetAngle(info->body);
            // fall through

        case PB_STATIC:
            if (!cpBodyIsStatic(info->body))
            {
                cpSpaceRemoveBody(space, info->body);
                cpSpaceConvertBodyToStatic(space, info->body);
            }
            break;

        case PB_DYNAMIC:
            cpSpaceConvertBodyToDynamic(space, info->body, info->mass, 1.0);
            cpSpaceAddBody(space, info->body);
            _recalculate_moment(info);
            break;
    }
}
void physics_set_type(Entity ent, PhysicsBody type)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    _set_type(info, type);
}
PhysicsBody physics_get_type(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return info->type;
}

void physics_debug_draw(Entity ent)
{
    entitymap_set(debug_draw_map, ent, true);
}


// --- shape ---------------------------------------------------------------

static unsigned int _shape_add(Entity ent, PhysicsShape type, cpShape *shape)
{
    PhysicsInfo *info;
    ShapeInfo *shapeInfo;

    info = entitypool_get(pool, ent);
    error_assert(info);

    // init ShapeInfo
    shapeInfo = array_add(info->shapes);
    shapeInfo->type = type;
    shapeInfo->shape = shape;

    // init cpShape
    cpShapeSetBody(shape, info->body);
    cpSpaceAddShape(space, shape);
    cpShapeSetFriction(shapeInfo->shape, 1);
    cpShapeSetUserData(shapeInfo->shape, ent);

    // update moment
    if (!cpBodyIsStatic(info->body))
    {
        if (array_length(info->shapes) > 1)
            cpBodySetMoment(info->body, _moment(info->body, shapeInfo)
                            + cpBodyGetMoment(info->body));
        else
            cpBodySetMoment(info->body, _moment(info->body, shapeInfo));
    }

    return array_length(info->shapes) - 1;
}
unsigned int physics_shape_add_circle(Entity ent, Scalar r,
                                      LuaVec2 offset)
{
    cpShape *shape = cpCircleShapeNew(NULL, r, cpv_of_vec2(offset));
    return _shape_add(ent, PS_CIRCLE, shape);
}
unsigned int physics_shape_add_box(Entity ent, BBox b, Scalar r)
{
    cpShape *shape = cpBoxShapeNew3(NULL, cpBBNew(b.min.x, b.min.y,
                                                  b.max.x, b.max.y), r);
    return _shape_add(ent, PS_POLYGON, shape);
}
unsigned int physics_shape_add_poly(Entity ent,
                                    unsigned int nverts,
                                    const LuaVec2 *verts,
                                    Scalar r)
{
    unsigned int i;
    cpVect *cpverts;
    cpShape *shape;

    cpverts = mem_alloc(nverts * sizeof(cpVect));
    for (i = 0; i < nverts; ++i)
        cpverts[i] = cpv_of_vec2(verts[i]);
    nverts = cpConvexHull(nverts, cpverts, NULL, NULL, 0);
    shape = cpPolyShapeNew2(NULL, nverts, cpverts, cpvzero, r);
    mem_free(cpverts);
    return _shape_add(ent, PS_POLYGON, shape);
}

unsigned int physics_get_num_shapes(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return array_length(info->shapes);
}
PhysicsShape physics_shape_get_type(Entity ent, unsigned int i)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    error_assert(i < array_length(info->shapes));
    return array_get_val(ShapeInfo, info->shapes, i).type;
}
void physics_shape_remove(Entity ent, unsigned int i)
{
    PhysicsInfo *info;
    ShapeInfo *shapeInfo;

    info = entitypool_get(pool, ent);
    error_assert(info);

    if (i >= array_length(info->shapes))
        return;

    shapeInfo = array_get(info->shapes, i);
    _remove_shape(array_get_val(ShapeInfo, info->shapes, i).shape);
    array_quick_remove(info->shapes, i);
    _recalculate_moment(info);
}

static ShapeInfo *_get_shape(PhysicsInfo *info, unsigned int i)
{
    error_assert(i < array_length(info->shapes),
                 "shape index should be in range");
    return array_get(info->shapes, i);
}

int physics_poly_get_num_verts(Entity ent, unsigned int i)
{
    PhysicsInfo *info;
    info = entitypool_get(pool, ent);
    error_assert(info);
    return cpPolyShapeGetNumVerts(_get_shape(info, i)->shape);
}

unsigned int physics_convex_hull(unsigned int nverts, LuaVec2 *verts)
{
    cpVect *cpverts;
    unsigned int i;

    cpverts = mem_alloc(nverts * sizeof(cpVect));
    for (i = 0; i < nverts; ++i)
        cpverts[i] = cpv_of_vec2(verts[i]);
    nverts = cpConvexHull(nverts, cpverts, NULL, NULL, 0);
    for (i = 0; i < nverts; ++i)
        verts[i] = vec2_of_cpv(cpverts[i]);
    mem_free(cpverts);
    return nverts;
}

void physics_shape_set_surface_velocity(Entity ent,
                                        unsigned int i,
                                        LuaVec2 v)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpShapeSetSurfaceVelocity(_get_shape(info, i)->shape, cpv_of_vec2(v));
}
LuaVec2 physics_shape_get_surface_velocity(Entity ent,
                                        unsigned int i)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return vec2_of_cpv(cpShapeGetSurfaceVelocity(_get_shape(info, i)->shape));
}

void physics_shape_set_sensor(Entity ent,
                              unsigned int i,
                              bool sensor)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpShapeSetSensor(_get_shape(info, i)->shape, sensor);
}
bool physics_shape_get_sensor(Entity ent,
                              unsigned int i)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpShapeGetSensor(_get_shape(info, i)->shape);
}

// --- dynamics ------------------------------------------------------------

void physics_set_mass(Entity ent, Scalar mass)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    if (mass <= SCALAR_EPSILON)
        return;

    cpBodySetMass(info->body, info->mass = mass);
    _recalculate_moment(info);
}
Scalar physics_get_mass(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return info->mass;
}

void physics_set_freeze_rotation(Entity ent, bool freeze)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    if (freeze)
    {
        cpBodySetAngVel(info->body, 0);
        cpBodySetMoment(info->body, SCALAR_INFINITY);
    }
    else
        _recalculate_moment(info);
}
bool physics_get_freeze_rotation(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    // TODO: do this a better way? maybe store separate flag
    return cpBodyGetMoment(info->body) == SCALAR_INFINITY;
}

void physics_set_velocity(Entity ent, LuaVec2 vel)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetVel(info->body, cpv_of_vec2(vel));
}
LuaVec2 physics_get_velocity(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return vec2_of_cpv(cpBodyGetVel(info->body));
}
void physics_set_force(Entity ent, LuaVec2 force)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetForce(info->body, cpv_of_vec2(force));
}
LuaVec2 physics_get_force(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return vec2_of_cpv(cpBodyGetForce(info->body));
}

void physics_set_angular_velocity(Entity ent, Scalar ang_vel)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetAngVel(info->body, ang_vel);
}
Scalar physics_get_angular_velocity(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetAngVel(info->body);
}
void physics_set_torque(Entity ent, Scalar torque)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetTorque(info->body, torque);
}
Scalar physics_get_torque(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetTorque(info->body);
}

void physics_set_velocity_limit(Entity ent, Scalar lim)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetVelLimit(info->body, lim);
}
Scalar physics_get_velocity_limit(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetVelLimit(info->body);
}
void physics_set_angular_velocity_limit(Entity ent, Scalar lim)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetAngVelLimit(info->body, lim);
}
Scalar physics_get_angular_velocity_limit(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetAngVelLimit(info->body);
}

void physics_reset_forces(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodyResetForces(info->body);
}
void physics_apply_force(Entity ent, LuaVec2 force)
{
    physics_apply_force_at(ent, force, vec2_zero);
}
void physics_apply_force_at(Entity ent, LuaVec2 force, LuaVec2 at)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodyApplyForce(info->body, cpv_of_vec2(force), cpv_of_vec2(at));
}
void physics_apply_impulse(Entity ent, LuaVec2 impulse)
{
    physics_apply_impulse_at(ent, impulse, vec2_zero);
}
void physics_apply_impulse_at(Entity ent, LuaVec2 impulse, LuaVec2 at)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodyApplyImpulse(info->body, cpv_of_vec2(impulse), cpv_of_vec2(at));
}

// --- collisions ----------------------------------------------------------

static void _add_collision(cpBody *body, cpArbiter *arbiter, void *collisions)
{
    cpBody *ba, *bb;
    Collision *col;

    // get in right order
    cpArbiterGetBodies(arbiter, &ba, &bb);
    if (bb == body)
    {
        ba = body;
        bb = ba;
    }

    // save collision
    col = array_add(collisions);
    col->a = cpBodyGetUserData(ba);
    col->b = cpBodyGetUserData(bb);
}
static void _update_collisions(PhysicsInfo *info)
{
    if (info->collisions)
        return;

    // gather collisions
    info->collisions = array_new(Collision);
    cpBodyEachArbiter(info->body, _add_collision, info->collisions);
}

unsigned int physics_get_num_collisions(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    _update_collisions(info);
    return array_length(info->collisions);
}
Collision *physics_get_collisions(Entity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    _update_collisions(info);
    return array_begin(info->collisions);
}


// --- queries -------------------------------------------------------------

NearestResult physics_nearest(LuaVec2 point, Scalar max_dist)
{
    cpNearestPointQueryInfo info;
    NearestResult res;

    if (!cpSpaceNearestPointQueryNearest(space, cpv_of_vec2(point), max_dist,
                                         CP_ALL_LAYERS, CP_NO_GROUP, &info))
    {
        // no result
        res.ent = entity_nil;
        return res;
    }

    res.ent = cpShapeGetUserData(info.shape);
    res.p = vec2_of_cpv(info.p);
    res.d = info.d;
    res.g = vec2_of_cpv(info.g);
    return res;
}

// --- init/fini ---------------------------------------------------------

static GLuint program;
static GLuint vao;
static GLuint vbo;

void physics_init()
{
    // init pools, maps
    pool = entitypool_new(PhysicsInfo);
    debug_draw_map = entitymap_new(false);

    // init cpSpace
    space = cpSpaceNew();
    cpSpaceSetGravity(space, cpv(0, -9.8));

    // init draw stuff
    program = gfx_create_program(data_path("phypoly.vert"),
                                 NULL,
                                 data_path("phypoly.frag"));
    glUseProgram(program);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gfx_bind_vertex_attrib(program, GL_FLOAT, 2, "position", LuaVec2, x);
}
void physics_fini()
{
    PhysicsInfo *info;

    // clean up draw stuff
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // remove all bodies, shapes
    entitypool_foreach(info, pool)
        _remove(info);

    // fini cpSpace
    cpSpaceFree(space);

    // fini pools, maps
    entitymap_free(debug_draw_map);
    entitypool_free(pool);
}

// --- update --------------------------------------------------------------

// step the space with fixed time step
static void _step()
{
    static Scalar remain = 0.0;

    remain += dt;
    while (remain >= period)
    {
        cpSpaceStep(space, period);
        remain -= period;
    }
}

static void _update_kinematics()
{
    PhysicsInfo *info;
    cpVect pos;
    cpFloat ang;
    Scalar invdt;
    Entity ent;

    if (dt <= FLT_EPSILON)
        return;
    invdt = 1 / dt;

    entitypool_foreach(info, pool)
        if (info->type == PB_KINEMATIC)
        {
            ent = info->pool_elem.ent;

            // move to transform
            pos = cpv_of_vec2(transform_get_position(ent));
            ang = transform_get_rotation(ent);
            cpBodySetPos(info->body, pos);
            cpBodySetAngle(info->body, ang);
            info->last_dirty_count = transform_get_dirty_count(ent);

            // update linear, angular velocities based on delta
            cpBodySetVel(info->body,
                         cpvmult(cpvsub(pos, info->last_pos), invdt));
            cpBodySetAngVel(info->body, (ang - info->last_ang) * invdt);
            cpSpaceReindexShapesForBody(space, info->body);

            // save current state for next computation
            info->last_pos = pos;
            info->last_ang = ang;
        }
}
void physics_update_all()
{
    PhysicsInfo *info;
    Entity ent;

    entitypool_remove_destroyed(pool, physics_remove);

    entitymap_clear(debug_draw_map);

    // simulate
    if (!timing_get_paused())
    {
        _update_kinematics();
        _step();
    }

    // synchronize transform <-> physics
    entitypool_foreach(info, pool)
    {
        ent = info->pool_elem.ent;

        // if transform is dirtier, move to it, else overwrite it
        if (transform_get_dirty_count(ent) != info->last_dirty_count)
        {
            cpBodySetVel(info->body, cpvzero);
            cpBodySetAngVel(info->body, 0.0f);
            cpBodySetPos(info->body, cpv_of_vec2(transform_get_position(ent)));
            cpBodySetAngle(info->body, transform_get_rotation(ent));
            cpSpaceReindexShapesForBody(space, info->body);
        }
        else if (info->type == PB_DYNAMIC)
        {
            transform_set_position(ent, vec2_of_cpv(cpBodyGetPos(info->body)));
            transform_set_rotation(ent, cpBodyGetAngle(info->body));
        }

        info->last_dirty_count = transform_get_dirty_count(ent);
    }
}

void physics_post_update_all()
{
    PhysicsInfo *info;

    entitypool_remove_destroyed(pool, physics_remove);

    // clear collisions
    entitypool_foreach(info, pool)
        if (info->collisions)
        {
            array_free(info->collisions);
            info->collisions = NULL;
        }
}

// --- draw ----------------------------------------------------------------

static void _circle_draw(PhysicsInfo *info, ShapeInfo *shapeInfo)
{
    static LuaVec2 verts[] = {
        {  1.0,  0.0 }, {  0.7071,  0.7071 },
        {  0.0,  1.0 }, { -0.7071,  0.7071 },
        { -1.0,  0.0 }, { -0.7071, -0.7071 },
        {  0.0, -1.0 }, {  0.7071, -0.7071 },
    }, offset;
    const unsigned int nverts = sizeof(verts) / sizeof(verts[0]);
    Scalar r;
    LuaMat3 wmat;

    wmat = transform_get_world_matrix(info->pool_elem.ent);
    offset = vec2_of_cpv(cpCircleShapeGetOffset(shapeInfo->shape));
    r = cpCircleShapeGetRadius(shapeInfo->shape);

    glUniformMatrix3fv(glGetUniformLocation(program, "wmat"),
                       1, GL_FALSE, (const GLfloat *) &wmat);
    glUniform2fv(glGetUniformLocation(program, "offset"),
                 1, (const GLfloat *) &offset);
    glUniform1f(glGetUniformLocation(program, "radius"), r);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(LuaVec2),
                 verts, GL_STREAM_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, nverts);
    glDrawArrays(GL_POINTS, 0, nverts);
}

static void _polygon_draw(PhysicsInfo *info, ShapeInfo *shapeInfo)
{
    unsigned int i, nverts;
    LuaVec2 *verts;
    LuaMat3 wmat;

    wmat = transform_get_world_matrix(info->pool_elem.ent);
    glUniformMatrix3fv(glGetUniformLocation(program, "wmat"),
                       1, GL_FALSE, (const GLfloat *) &wmat);

    glUniformMatrix3fv(glGetUniformLocation(program, "wmat"),
                       1, GL_FALSE, (const GLfloat *) &wmat);
    glUniform2f(glGetUniformLocation(program, "offset"), 0, 0);
    glUniform1f(glGetUniformLocation(program, "radius"), 1);

    // copy as LuaVec2 array
    nverts = cpPolyShapeGetNumVerts(shapeInfo->shape);
    verts = mem_alloc(nverts * sizeof(LuaVec2));
    for (i = 0; i < nverts; ++i)
        verts[i] = vec2_of_cpv(cpPolyShapeGetVert(shapeInfo->shape, i));

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(LuaVec2),
                 verts, GL_STREAM_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, nverts);
    glDrawArrays(GL_POINTS, 0, nverts);

    mem_free(verts);
}

void physics_draw_all()
{
    PhysicsInfo *info;
    ShapeInfo *shapeInfo;

    if (!edit_get_enabled())
        return;

    // bind program, update uniforms
    glUseProgram(program);
    glUniformMatrix3fv(glGetUniformLocation(program, "inverse_view_matrix"),
                       1, GL_FALSE,
                       (const GLfloat *) camera_get_inverse_view_matrix_ptr());

    // draw!
    entitypool_foreach(info, pool)
        if (entitymap_get(debug_draw_map, info->pool_elem.ent))
            array_foreach(shapeInfo, info->shapes)
                switch(shapeInfo->type)
                {
                    case PS_CIRCLE:
                        _circle_draw(info, shapeInfo);
                        break;

                    case PS_POLYGON:
                        _polygon_draw(info, shapeInfo);
                        break;
                }
}

// --- save/load -----------------------------------------------------------

// chipmunk data save/load helpers
static void _cpv_save(cpVect *cv, const char *n, Store *s)
{
    LuaVec2 v = vec2_of_cpv(*cv);
    vec2_save(&v, n, s);
}
static bool _cpv_load(cpVect *cv, const char *n, cpVect d, Store *s)
{
    LuaVec2 v;
    if (vec2_load(&v, n, vec2_zero, s))
    {
        *cv = cpv_of_vec2(v);
        return true;
    }
    *cv = d;
    return false;
}
static void _cpf_save(cpFloat *cf, const char *n, Store *s)
{
    Scalar f = *cf;
    scalar_save(&f, n, s);
}
static bool _cpf_load(cpFloat *cf, const char *n, cpFloat d, Store *s)
{
    Scalar f;
    bool r = scalar_load(&f, n, d, s);
    *cf = f;
    return r;
}

// properties aren't set if missing, so defaults don't really matter
#define _cpv_default cpvzero
#define _cpf_default 0.0f
#define bool_default false
#define uint_default 0

/*
 * note that UserData isn't directly save/load'd -- it is restored to
 * the Entity value on load separately
 */

// some hax to reduce typing for body properties save/load
#define body_prop_save(type, f, n, prop) \
    {                                    \
        type v;                          \
        v = cpBodyGet##prop(info->body); \
        f##_save(&v, n, body_s);         \
    }
#define body_prop_load(type, f, n, prop)                                          \
    {                                                                             \
        type v;                                                                   \
        if (f##_load(&v, n, f##_default, body_s)) cpBodySet##prop(info->body, v); \
    }

#define body_props_saveload(saveload)                           \
    body_prop_##saveload(cpFloat, _cpf, "mass", Mass);          \
    body_prop_##saveload(cpFloat, _cpf, "moment", Moment);      \
    /* body_prop_##saveload(cpVect, _cpv, Pos); */              \
    body_prop_##saveload(cpVect, _cpv, "vel", Vel);             \
    body_prop_##saveload(cpVect, _cpv, "force", Force);         \
    /* body_prop_##saveload(cpFloat, _cpf, Angle); */           \
    body_prop_##saveload(cpFloat, _cpf, "ang_vel", AngVel);     \
    body_prop_##saveload(cpFloat, _cpf, "torque", Torque);      \
    body_prop_##saveload(cpFloat, _cpf, "vel_limit", VelLimit); \
    body_prop_##saveload(cpFloat, _cpf, "ang_vel_limit", AngVelLimit);  

// save/load for just the body in a PhysicsInfo
static void _body_save(PhysicsInfo *info, Store *s)
{
    Store *body_s;

    if (store_child_save(&body_s, "body", s))
        body_props_saveload(save);
}
static void _body_load(PhysicsInfo *info, Store *s)
{
    Store *body_s;
    Entity ent;
    PhysicsBody type;

    error_assert(store_child_load(&body_s, "body", s),
                 "physics entry must have a saved body");

    // create, restore properties
    ent = info->pool_elem.ent;
    info->body = cpSpaceAddBody(space, cpBodyNew(info->mass, 1.0));
    body_props_saveload(load);
    cpBodySetUserData(info->body, ent);

    // force type change if non-default
    type = info->type;
    info->type = PB_DYNAMIC;
    _set_type(info, type);

    // restore position, angle based on transform
    cpBodySetPos(info->body, cpv_of_vec2(transform_get_position(ent)));
    cpBodySetAngle(info->body, transform_get_rotation(ent));
    info->last_dirty_count = transform_get_dirty_count(info->pool_elem.ent);
}

// save/load for special properties of each shape type

static void _circle_save(PhysicsInfo *info, ShapeInfo *shapeInfo,
                         Store *s)
{
    cpFloat radius;
    cpVect offset;

    radius = cpCircleShapeGetRadius(shapeInfo->shape);
    _cpf_save(&radius, "radius", s);
    offset = cpCircleShapeGetOffset(shapeInfo->shape);
    _cpv_save(&offset, "offset", s);
}
static void _circle_load(PhysicsInfo *info, ShapeInfo *shapeInfo,
                         Store *s)
{
    cpFloat radius;
    cpVect offset;

    _cpf_load(&radius, "radius", 1, s);
    _cpv_load(&offset, "offset", cpvzero, s);

    shapeInfo->shape = cpCircleShapeNew(info->body, radius, offset);
    cpSpaceAddShape(space, shapeInfo->shape);
}

static void _polygon_save(PhysicsInfo *info, ShapeInfo *shapeInfo,
                          Store *s)
{
    Store *verts_s;
    unsigned int n, i;
    Scalar r;
    cpVect v;

    n = cpPolyShapeGetNumVerts(shapeInfo->shape);
    uint_save(&n, "num_verts", s);

    r = cpPolyShapeGetRadius(shapeInfo->shape);
    scalar_save(&r, "radius", s);

    if (store_child_save(&verts_s, "verts", s))
        for (i = 0; i < n; ++i)
        {
            v = cpPolyShapeGetVert(shapeInfo->shape, i);
            _cpv_save(&v, NULL, verts_s);
        }
}
static void _polygon_load(PhysicsInfo *info, ShapeInfo *shapeInfo,
                          Store *s)
{
    Store *verts_s;
    unsigned int n, i;
    Scalar r;
    cpVect *vs;

    error_assert(uint_load(&n, "num_verts", 0, s),
                 "polygon shape type must have saved number of vertices");
    scalar_load(&r, "radius", 0, s);

    error_assert(store_child_load(&verts_s, "verts", s),
                 "polygon shape type must have saved list of vertices");
    vs = mem_alloc(n * sizeof(cpVect));
    for (i = 0; i < n; ++i)
        if (!_cpv_load(&vs[i], NULL, cpvzero, verts_s))
            error("polygon shape type saved number of vertices doesn't match"
                  "size of saved list of vertices");
    shapeInfo->shape = cpPolyShapeNew2(info->body, n, vs, cpvzero, r);
    cpSpaceAddShape(space, shapeInfo->shape);
    mem_free(vs);
}

// some hax to reduce typing for shape properties save/load
#define shape_prop_save(type, f, n, prop)       \
    {                                           \
        type v;                                 \
        v = cpShapeGet##prop(shapeInfo->shape); \
        f##_save(&v, n, shape_s);               \
    }
#define shape_prop_load(type, f, n, prop)                                                 \
    {                                                                                     \
        type v;                                                                           \
        if (f##_load(&v, n, f##_default, shape_s)) cpShapeSet##prop(shapeInfo->shape, v); \
    }
#define shape_props_saveload(saveload)                                          \
    shape_prop_##saveload(bool, bool, "sensor", Sensor);                        \
    shape_prop_##saveload(cpFloat, _cpf, "elasticity", Elasticity);             \
    shape_prop_##saveload(cpFloat, _cpf, "friction", Friction);                 \
    shape_prop_##saveload(cpVect, _cpv, "surface_velocity", SurfaceVelocity);   \
    shape_prop_##saveload(unsigned int, uint, "collision_type", CollisionType); \
    shape_prop_##saveload(unsigned int, uint, "group", Group);                  \
    shape_prop_##saveload(unsigned int, uint, "layers", Layers);        \

// save/load for all shapes in a PhysicsInfo
static void _shapes_save(PhysicsInfo *info, Store *s)
{
    Store *t, *shape_s;
    ShapeInfo *shapeInfo;

    if (store_child_save(&t, "shapes", s))
        array_foreach(shapeInfo, info->shapes)
            if (store_child_save(&shape_s, NULL, t))
            {
                // type-specific
                enum_save(&shapeInfo->type, "type", shape_s);
                switch (shapeInfo->type)
                {
                    case PS_CIRCLE:
                        _circle_save(info, shapeInfo, shape_s);
                        break;
                    case PS_POLYGON:
                        _polygon_save(info, shapeInfo, shape_s);
                        break;
                }

                // common
                shape_props_saveload(save);
            }
}
static void _shapes_load(PhysicsInfo *info, Store *s)
{
    Store *t, *shape_s;
    ShapeInfo *shapeInfo;

    info->shapes = array_new(ShapeInfo);

    if (store_child_load(&t, "shapes", s))
        while (store_child_load(&shape_s, NULL, t))
        {
            shapeInfo = array_add(info->shapes);

            // type-specific
            enum_load(&shapeInfo->type, "type", PS_CIRCLE, shape_s);
            switch (shapeInfo->type)
            {
                case PS_CIRCLE:
                    _circle_load(info, shapeInfo, shape_s);
                    break;
                case PS_POLYGON:
                    _polygon_load(info, shapeInfo, shape_s);
                    break;
            }

            // common
            shape_props_saveload(load);
            cpShapeSetUserData(shapeInfo->shape, info->pool_elem.ent);
        }
}

/* 
 * save/load for all data in a PhysicsInfo other than the the actual body,
 * shapes is handled here, the rest is done in functions above
 */
void physics_save_all(Store *s)
{
    Store *t, *info_s;
    PhysicsInfo *info;

    if (store_child_save(&t, "physics", s))
        entitypool_save_foreach(info, info_s, pool, "pool", t)
        {
            enum_save(&info->type, "type", info_s);
            scalar_save(&info->mass, "mass", info_s);

            _body_save(info, info_s);
            _shapes_save(info, info_s);
        }
}
void physics_load_all(Store *s)
{
    Store *t, *info_s;
    PhysicsInfo *info;

    if (store_child_load(&t, "physics", s))
        entitypool_load_foreach(info, info_s, pool, "pool", t)
        {
            enum_load(&info->type, "type", PB_DYNAMIC, info_s);
            scalar_load(&info->mass, "mass", 1, info_s);

            _body_load(info, info_s);
            _shapes_load(info, info_s);

            info->collisions = NULL;

            // set last_pos/last_ang info for kinematic bodies
            info->last_pos = cpBodyGetPos(info->body);
            info->last_ang = cpBodyGetAngle(info->body);
        }
}

#endif

void physics_init() { PROFILE_FUNC(); }
void physics_fini() {}
void physics_update_all() {}
void physics_post_update_all() {}
void physics_draw_all() {}
void physics_save_all(Store *s) {}
void physics_load_all(Store *s) {}