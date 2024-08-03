
#include "engine/neko_app.h"
#include "engine/neko_lua.h"
#include "engine/neko_physics.h"

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
    lua_Number dt = luaL_optnumber(L, 2, g_app->time.delta);
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