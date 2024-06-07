#include "engine/neko.h"
#include "engine/neko_api.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_common.h"
#include "engine/neko_ecs.h"
#include "engine/neko_engine.h"
#include "engine/neko_lua.hpp"
#include "engine/neko_luabind.hpp"

// game
#include "sandbox/game_editor.h"
#include "sandbox/game_main.h"

static int g_lua_callbacks_table_ref = LUA_NOREF;

static int __neko_bind_callback_save(lua_State* L) {
    // 检查传递给函数的参数是否是一个字符串和一个函数
    const_str identifier = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    // 如果之前没有保存的Lua函数引用，则创建一个新的table
    if (g_lua_callbacks_table_ref == LUA_NOREF) {
        lua_newtable(L);
        g_lua_callbacks_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    // 获取保存的table
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_lua_callbacks_table_ref);

    // 将传递给函数的Lua函数压入栈顶
    lua_pushvalue(L, 2);

    // 将函数放入table中，使用标识符作为键
    lua_setfield(L, -2, identifier);

    // 弹出保存的table
    lua_pop(L, 1);

    return 0;
}

static int __neko_bind_callback_call(lua_State* L) {
    // 检查之前是否有保存的Lua函数引用
    if (g_lua_callbacks_table_ref != LUA_NOREF) {
        // 获取标识符参数
        const_str identifier = luaL_checkstring(L, 1);

        // 获取保存的table
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_lua_callbacks_table_ref);

        // 获取table中对应标识符的Lua函数
        lua_getfield(L, -1, identifier);

        if (lua_isfunction(L, -1)) {
            // 将额外的参数依次压入栈顶
            int nargs = lua_gettop(L) - 1;  // 获取参数数量 减去标识符参数
            for (int i = 2; i <= nargs + 1; ++i) {
                lua_pushvalue(L, i);
            }
            lua_call(L, nargs, 0);  // 调用
        } else {
            // throw lua_exception_t(std::format("callback with identifier '{}' not found or is not a function.", identifier));
        }

        // 弹出保存的table
        lua_pop(L, 1);
    }

    return 0;
}

static bool __neko_bind_platform_key_pressed(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_keycode, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_key_pressed(cval);
}

static bool __neko_bind_platform_was_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_keycode, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_key_down(cval);
}

static bool __neko_bind_platform_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_keycode, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_key_down(cval);
}

static bool __neko_bind_platform_key_released(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_keycode, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_key_released(cval);
}

static bool __neko_bind_platform_was_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_was_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_pressed(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_mouse_pressed(cval);
}

static bool __neko_bind_platform_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_released(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(CL_GAME_USERDATA()->L, key);
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    return neko_platform_mouse_released(cval);
}

static int __neko_bind_platform_mouse_delta(lua_State* L) {
    neko_vec2 v2 = neko_platform_mouse_deltav();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_mouse_position(lua_State* L) {
    neko_vec2 v2 = neko_platform_mouse_positionv();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_mouse_wheel(lua_State* L) {
    neko_vec2 v2 = neko_platform_mouse_wheelv();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_window_size(lua_State* L) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_platform_window_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_framebuffer_size(lua_State* L) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_platform_framebuffer_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_set_window_title(lua_State* L) {
    u32 handle = lua_tointeger(L, -1);
    const_str title = lua_tostring(L, -2);
    neko_platform_set_window_title(handle, title);
    return 0;
}

NEKO_INLINE void neko_register_platform(lua_State* L) {

    neko_lua_auto_enum(L, neko_platform_keycode);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_INVALID);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_SPACE);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_APOSTROPHE); /* ' */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_COMMA);      /* , */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_MINUS);      /* - */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_PERIOD);     /* . */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_SLASH);      /* / */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_0);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_1);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_2);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_3);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_4);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_5);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_6);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_7);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_8);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_9);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_SEMICOLON); /* ; */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_EQUAL);     /* = */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_A);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_B);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_C);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_D);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_E);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_G);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_H);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_I);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_J);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_K);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_L);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_M);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_N);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_O);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_P);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_Q);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_R);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_S);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_T);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_U);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_V);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_W);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_X);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_Y);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_Z);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_LEFT_BRACKET);  /* [ */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_BACKSLASH);     /* \ */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_RIGHT_BRACKET); /* ] */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_GRAVE_ACCENT);  /* ` */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_WORLD_1);       /* non-US #1 */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_WORLD_2);       /* non-US #2 */
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_ESC);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_ENTER);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_TAB);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_BACKSPACE);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_INSERT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_DELETE);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_RIGHT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_LEFT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_DOWN);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_UP);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_PAGE_UP);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_PAGE_DOWN);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_HOME);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_END);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_CAPS_LOCK);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_SCROLL_LOCK);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_NUM_LOCK);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_PRINT_SCREEN);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_PAUSE);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F1);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F2);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F3);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F4);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F5);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F6);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F7);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F8);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F9);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F10);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F11);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F12);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F13);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F14);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F15);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F16);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F17);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F18);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F19);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F20);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F21);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F22);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F23);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F24);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_F25);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_0);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_1);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_2);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_3);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_4);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_5);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_6);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_7);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_8);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_9);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_DECIMAL);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_DIVIDE);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_MULTIPLY);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_SUBTRACT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_ADD);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_ENTER);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_KP_EQUAL);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_LEFT_SHIFT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_LEFT_CONTROL);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_LEFT_ALT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_LEFT_SUPER);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_RIGHT_SHIFT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_RIGHT_CONTROL);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_RIGHT_ALT);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_RIGHT_SUPER);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_MENU);
    neko_lua_auto_enum_value(L, neko_platform_keycode, NEKO_KEYCODE_COUNT);

    neko_lua_auto_enum(L, neko_platform_mouse_button_code);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, NEKO_MOUSE_LBUTTON);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, NEKO_MOUSE_RBUTTON);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, NEKO_MOUSE_MBUTTON);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, NEKO_MOUSE_BUTTON_CODE_COUNT);

    neko::lua_bind::bind("neko_key_pressed", &__neko_bind_platform_key_pressed);
    neko::lua_bind::bind("neko_was_key_down", &__neko_bind_platform_was_key_down);
    neko::lua_bind::bind("neko_was_mouse_down", &__neko_bind_platform_was_mouse_down);
    neko::lua_bind::bind("neko_key_down", &__neko_bind_platform_key_down);
    neko::lua_bind::bind("neko_key_released", &__neko_bind_platform_key_released);
    neko::lua_bind::bind("neko_mouse_down", &__neko_bind_platform_mouse_down);
    neko::lua_bind::bind("neko_mouse_pressed", &__neko_bind_platform_mouse_pressed);
    neko::lua_bind::bind("neko_mouse_released", &__neko_bind_platform_mouse_released);
    neko::lua_bind::bind("neko_mouse_moved", +[]() -> bool { return neko_platform_mouse_moved(); });
    neko::lua_bind::bind("neko_main_window", +[]() -> u32 { return neko_platform_main_window(); });
    neko::lua_bind::bind("neko_set_window_size", +[](u32 handle, u32 width, u32 height) { neko_platform_set_window_size(handle, width, height); });
    neko::lua_bind::bind("neko_set_mouse_position", +[](u32 handle, f64 x, f64 y) { neko_platform_mouse_set_position(handle, x, y); });

    //.def(+[](const_str path) -> std::string { return neko_engine_subsystem(platform)->get_path(path); }, "neko_file_path")
    //.def(+[](const_str title, u32 width, u32 height) -> neko_resource_handle { return neko_engine_subsystem(platform)->create_window(title, width, height); }, "neko_create_window")

    lua_register(L, "neko_mouse_delta", __neko_bind_platform_mouse_delta);
    lua_register(L, "neko_mouse_position", __neko_bind_platform_mouse_position);
    lua_register(L, "neko_mouse_wheel", __neko_bind_platform_mouse_wheel);

    lua_register(L, "neko_window_size", __neko_bind_platform_window_size);
    lua_register(L, "neko_framebuffer_size", __neko_bind_platform_framebuffer_size);
    lua_register(L, "neko_set_window_title", __neko_bind_platform_set_window_title);
}

#if 0

// box2d fixture

static int mt_b2_fixture_friction(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    float friction = fixture->GetFriction();
    lua_pushnumber(L, friction);
    return 1;
}

static int mt_b2_fixture_restitution(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    float restitution = fixture->GetRestitution();
    lua_pushnumber(L, restitution);
    return 1;
}

static int mt_b2_fixture_is_sensor(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    float sensor = fixture->IsSensor();
    lua_pushnumber(L, sensor);
    return 1;
}

static int mt_b2_fixture_set_friction(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    float friction = luaL_checknumber(L, 2);
    fixture->SetFriction(friction);
    return 0;
}

static int mt_b2_fixture_set_restitution(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    float restitution = luaL_checknumber(L, 2);
    fixture->SetRestitution(restitution);
    return 0;
}

static int mt_b2_fixture_set_sensor(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    bool sensor = lua_toboolean(L, 2);
    fixture->SetSensor(sensor);
    return 0;
}

static int mt_b2_fixture_body(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    b2Body* body = fixture->GetBody();

    neko_physics_userdata* pud = (neko_physics_userdata*)body->GetUserData().pointer;
    assert(pud != nullptr);
    pud->ref_count++;

    neko_physics p = physics_weak_copy(physics);
    p.body = body;

    luax_new_userdata(L, p, "mt_b2_body");
    return 1;
}

static int mt_b2_fixture_udata(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_fixture");
    b2Fixture* fixture = physics->fixture;

    physics_push_userdata(L, fixture->GetUserData().pointer);
    return 1;
}

static int open_mt_b2_fixture(lua_State* L) {
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

static int b2_body_unref(lua_State* L, bool destroy) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    if (physics->body != nullptr) {
        neko_physics_userdata* pud = (neko_physics_userdata*)physics->body->GetUserData().pointer;
        assert(pud != nullptr);
        pud->ref_count--;

        if (pud->ref_count == 0 || destroy) {
            physics_destroy_body(L, physics);
        }
    }

    return 0;
}

static int mt_b2_body_gc(lua_State* L) { return b2_body_unref(L, false); }

static int mt_b2_body_destroy(lua_State* L) { return b2_body_unref(L, true); }

static b2FixtureDef b2_fixture_def(lua_State* L, s32 arg) {
    bool sensor = luax_boolean_field(L, arg, "sensor");
    lua_Number density = luax_opt_number_field(L, arg, "density", 1);
    lua_Number friction = luax_opt_number_field(L, arg, "friction", 0.2);
    lua_Number restitution = luax_opt_number_field(L, arg, "restitution", 0);
    neko_physics_userdata* pud = physics_userdata(L);

    b2FixtureDef def = {};
    def.isSensor = sensor;
    def.density = (float)density;
    def.friction = (float)friction;
    def.restitution = (float)restitution;
    def.userData.pointer = (u64)pud;
    return def;
}

static int mt_b2_body_make_box_fixture(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;
    b2FixtureDef fixture_def = b2_fixture_def(L, 2);

    lua_Number x = luax_opt_number_field(L, 2, "x", 0);
    lua_Number y = luax_opt_number_field(L, 2, "y", 0);
    lua_Number w = luax_number_field(L, 2, "w");
    lua_Number h = luax_number_field(L, 2, "h");
    lua_Number angle = luax_opt_number_field(L, 2, "angle", 0);

    b2Vec2 pos = {(float)x / physics->meter, (float)y / physics->meter};

    b2PolygonShape box = {};
    box.SetAsBox((float)w / physics->meter, (float)h / physics->meter, pos, angle);
    fixture_def.shape = &box;

    neko_physics p = physics_weak_copy(physics);
    p.fixture = body->CreateFixture(&fixture_def);

    luax_new_userdata(L, p, "mt_b2_fixture");
    return 0;
}

static int mt_b2_body_make_circle_fixture(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;
    b2FixtureDef fixture_def = b2_fixture_def(L, 2);

    lua_Number x = luax_opt_number_field(L, 2, "x", 0);
    lua_Number y = luax_opt_number_field(L, 2, "y", 0);
    lua_Number radius = luax_number_field(L, 2, "radius");

    b2CircleShape circle = {};
    circle.m_radius = radius / physics->meter;
    circle.m_p = {(float)x / physics->meter, (float)y / physics->meter};
    fixture_def.shape = &circle;

    neko_physics p = physics_weak_copy(physics);
    p.fixture = body->CreateFixture(&fixture_def);

    luax_new_userdata(L, p, "mt_b2_fixture");
    return 0;
}

static int mt_b2_body_position(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    b2Vec2 pos = body->GetPosition();

    lua_pushnumber(L, pos.x * physics->meter);
    lua_pushnumber(L, pos.y * physics->meter);
    return 2;
}

static int mt_b2_body_velocity(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    b2Vec2 vel = body->GetLinearVelocity();

    lua_pushnumber(L, vel.x * physics->meter);
    lua_pushnumber(L, vel.y * physics->meter);
    return 2;
}

static int mt_b2_body_angle(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    lua_pushnumber(L, body->GetAngle());
    return 1;
}

static int mt_b2_body_linear_damping(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    lua_pushnumber(L, body->GetLinearDamping());
    return 1;
}

static int mt_b2_body_fixed_rotation(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    lua_pushboolean(L, body->IsFixedRotation());
    return 1;
}

static int mt_b2_body_apply_force(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    body->ApplyForceToCenter({x / physics->meter, y / physics->meter}, false);
    return 0;
}

static int mt_b2_body_apply_impulse(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    body->ApplyLinearImpulseToCenter({x / physics->meter, y / physics->meter}, false);
    return 0;
}

static int mt_b2_body_set_position(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    body->SetTransform({x / physics->meter, y / physics->meter}, body->GetAngle());
    return 0;
}

static int mt_b2_body_set_velocity(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    body->SetLinearVelocity({x / physics->meter, y / physics->meter});
    return 0;
}

static int mt_b2_body_set_angle(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float angle = luaL_checknumber(L, 2);

    body->SetTransform(body->GetPosition(), angle);
    return 0;
}

static int mt_b2_body_set_linear_damping(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float damping = luaL_checknumber(L, 2);

    body->SetLinearDamping(damping);
    return 0;
}

static int mt_b2_body_set_fixed_rotation(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    bool fixed = lua_toboolean(L, 2);

    body->SetFixedRotation(fixed);
    return 0;
}

static int mt_b2_body_set_transform(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float angle = luaL_checknumber(L, 4);

    body->SetTransform({x / physics->meter, y / physics->meter}, angle);
    return 0;
}

static int mt_b2_body_draw_fixtures(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    draw_fixtures_for_body(body, physics->meter);

    return 0;
}

static int mt_b2_body_udata(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_body");
    b2Body* body = physics->body;

    physics_push_userdata(L, body->GetUserData().pointer);
    return 1;
}

static int open_mt_b2_body(lua_State* L) {
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

static int mt_b2_world_gc(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_world");
    physics_world_trash(L, physics);
    return 0;
}

static int mt_b2_world_step(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_world");
    // lua_Number dt = luaL_optnumber(L, 2, g_app->time.delta);
    lua_Number dt = lua_tonumber(L, 2);
    lua_Integer vel_iters = luaL_optinteger(L, 3, 6);
    lua_Integer pos_iters = luaL_optinteger(L, 4, 2);

    physics->world->Step((float)dt, (s32)vel_iters, (s32)pos_iters);
    return 0;
}

static b2BodyDef b2_body_def(lua_State* L, s32 arg, neko_physics* physics) {
    lua_Number x = luax_number_field(L, arg, "x");
    lua_Number y = luax_number_field(L, arg, "y");
    lua_Number vx = luax_opt_number_field(L, arg, "vx", 0);
    lua_Number vy = luax_opt_number_field(L, arg, "vy", 0);
    lua_Number angle = luax_opt_number_field(L, arg, "angle", 0);
    lua_Number linear_damping = luax_opt_number_field(L, arg, "linear_damping", 0);
    bool fixed_rotation = luax_boolean_field(L, arg, "fixed_rotation");
    neko_physics_userdata* pud = physics_userdata(L);

    b2BodyDef def = {};
    def.position.Set((float)x / physics->meter, (float)y / physics->meter);
    def.linearVelocity.Set((float)vx / physics->meter, (float)vy / physics->meter);
    def.angle = angle;
    def.linearDamping = linear_damping;
    def.fixedRotation = fixed_rotation;
    def.userData.pointer = (u64)pud;
    return def;
}

static int b2_make_body(lua_State* L, b2BodyType type) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_world");
    b2BodyDef body_def = b2_body_def(L, 2, physics);
    body_def.type = type;

    neko_physics p = physics_weak_copy(physics);
    p.body = physics->world->CreateBody(&body_def);

    luax_new_userdata(L, p, "mt_b2_body");
    return 1;
}

static int mt_b2_world_make_static_body(lua_State* L) { return b2_make_body(L, b2_staticBody); }

static int mt_b2_world_make_kinematic_body(lua_State* L) { return b2_make_body(L, b2_kinematicBody); }

static int mt_b2_world_make_dynamic_body(lua_State* L) { return b2_make_body(L, b2_dynamicBody); }

static int mt_b2_world_begin_contact(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_world");
    if (lua_type(L, 2) != LUA_TFUNCTION) {
        return luaL_error(L, "expected argument 2 to be a function");
    }

    physics_world_begin_contact(L, physics, 2);
    return 0;
}

static int mt_b2_world_end_contact(lua_State* L) {
    neko_physics* physics = (neko_physics*)luaL_checkudata(L, 1, "mt_b2_world");
    if (lua_type(L, 2) != LUA_TFUNCTION) {
        return luaL_error(L, "expected argument 2 to be a function");
    }

    physics_world_end_contact(L, physics, 2);
    return 0;
}

static int open_mt_b2_world(lua_State* L) {
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

static int neko_b2_world(lua_State* L) {
    lua_Number gx = luax_opt_number_field(L, 1, "gx", 0);
    lua_Number gy = luax_opt_number_field(L, 1, "gy", 9.81);
    lua_Number meter = luax_opt_number_field(L, 1, "meter", 16);

    b2Vec2 gravity = {(float)gx, (float)gy};

    neko_physics p = physics_world_make(L, gravity, meter);
    luax_new_userdata(L, p, "mt_b2_world");
    return 1;
}

#endif

struct neko_lua_handle_t {
    const_str name;
    size_t size;
    void* data;
};

static int __neko_bind_pack_construct(lua_State* L) {
    const_str name = lua_tostring(L, 1);
    const_str path = lua_tostring(L, 2);

    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_newuserdata(L, sizeof(neko_lua_handle_t));

    userdata_ptr->name = name;
    userdata_ptr->size = 0;

    userdata_ptr->data = neko_safe_malloc(sizeof(neko_packreader_t));

    int result = neko_pack_read(path, 0, false, (neko_packreader_t*)userdata_ptr->data);

    if (result) {
        const_str error_message = "neko_pack_check failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // 获取元表并设置
    // luaL_newmetatable(L, "neko_lua_handle__pack");  // 供测试的元表
    // lua_setmetatable(L, -2);

    neko_log_info("__neko_bind_pack_construct %s", userdata_ptr->name);
    return 1;
}

static int __neko_bind_pack_destroy(lua_State* L) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);

    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);
    neko_pack_destroy(pack);

    neko_safe_free(userdata_ptr->data);

    neko_log_info("__neko_bind_pack_destroy %s", userdata_ptr->name);
    return 0;
}

static int __neko_bind_pack_build(lua_State* L) {

    const_str path = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    lua_len(L, 2);                     // 获取table的长度
    int n = lua_tointeger(L, -1);      //
    lua_pop(L, 1);                     // 弹出长度值

    const_str* item_paths = (const_str*)neko_safe_malloc(n * sizeof(const_str));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);                 // 将index=i的元素压入堆栈顶部
        const_str str = lua_tostring(L, -1);  // # -1
        if (str != NULL) {
            item_paths[i - 1] = str;
        }
        lua_pop(L, 1);  // # -1
    }

    int result = neko_pack_build(path, n, item_paths, true);

    neko_safe_free(item_paths);

    if (result) {
        const_str error_message = "__neko_bind_pack_build failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    return 0;
}

static int __neko_bind_pack_info(lua_State* L) {

    const_str path = lua_tostring(L, 1);

    u8 pack_version;
    bool is_little_endian;
    u64 item_count;

    int result = neko_pack_info(path, &pack_version, &is_little_endian, &item_count);

    lua_pushinteger(L, pack_version);
    lua_pushboolean(L, is_little_endian);
    lua_pushinteger(L, item_count);

    return 3;
}

static int __neko_bind_pack_items(lua_State* L) {

    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);

    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);

    u64 item_count = neko_pack_item_count(pack);

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, neko_pack_item_path(pack, i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int __neko_bind_pack_assets_load(lua_State* L) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);
    const_str path = lua_tostring(L, 2);
    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);

    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_newuserdata(L, sizeof(neko_lua_handle_t));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    int result = neko_pack_item_data(pack, path, (const u8**)&assets_user_handle->data, (u32*)&assets_user_handle->size);

    if (result) {
        const_str error_message = "__neko_bind_pack_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // neko_log_info("__neko_bind_pack_assets_load %llu", assets_user_handle->size);

    // 获取元表并设置
    // luaL_newmetatable(L, "neko_lua_handle__assets");  // 供测试的元表
    // lua_setmetatable(L, -2);

    return 1;
}

static int __neko_bind_pack_assets_unload(lua_State* L) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);
    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);
    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_touserdata(L, 2);
    if (assets_user_handle && assets_user_handle->data)
        neko_pack_item_free(pack, assets_user_handle->data);
    else
        neko_log_warning("unknown assets unload %p", assets_user_handle);
    return 0;
}

static int __neko_bind_aseprite_render_create(lua_State* L) {
    neko_aseprite* sprite_data = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)lua_newuserdata(L, sizeof(neko_aseprite_renderer));
    memset(user_handle, 0, sizeof(neko_aseprite_renderer));
    user_handle->sprite = sprite_data;
    neko_aseprite_renderer_play(user_handle, "Idle");
    luaL_setmetatable(L, "mt_aseprite_renderer");
    return 1;
}

static int __neko_bind_aseprite_render_gc(lua_State* L) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    // neko_log_trace("aseprite_render __gc %p", user_handle);
    return 0;
}

static int __neko_bind_aseprite_render_update(lua_State* L) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    neko_t* engine = neko_instance();

    neko_aseprite_renderer_update(user_handle, engine->ctx.platform->time.delta);

    return 0;
}

static int __neko_bind_aseprite_render_update_animation(lua_State* L) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    const_str by_tag = lua_tostring(L, 2);
    neko_aseprite_renderer_play(user_handle, by_tag);
    return 0;
}

static int __neko_bind_aseprite_render(lua_State* L) {

    if (lua_gettop(L) != 4) {
        return luaL_error(L, "Function expects exactly 4 arguments");
    }

    luaL_argcheck(L, lua_gettop(L) == 4, 1, "expects exactly 4 arguments");

    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    auto xform = lua2struct::unpack<neko_vec2>(L, 2);

    int direction = neko::neko_lua_to<int>(L, 3);
    f32 scale = neko::neko_lua_to<f32>(L, 4);

    neko_t* engine = neko_instance();

    s32 index;
    if (user_handle->loop) {
        index = user_handle->loop->indices[user_handle->current_frame];
    } else {
        index = user_handle->current_frame;
    }

    neko_aseprite* spr = user_handle->sprite;
    neko_aseprite_frame f = spr->frames[index];

    if (direction)
        neko_idraw_rect_textured_ext(&CL_GAME_USERDATA()->idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u1, f.v0, f.u0, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);
    else
        neko_idraw_rect_textured_ext(&CL_GAME_USERDATA()->idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u0, f.v0, f.u1, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);

    return 0;
}

static int __neko_bind_aseprite_create(lua_State* L) {
    const_str file_path = lua_tostring(L, 1);

    neko_aseprite* user_handle = (neko_aseprite*)lua_newuserdata(L, sizeof(neko_aseprite));
    memset(user_handle, 0, sizeof(neko_aseprite));

    neko_aseprite_load(user_handle, file_path);

    luaL_setmetatable(L, "mt_aseprite");

    return 1;
}

static int __neko_bind_aseprite_gc(lua_State* L) {
    neko_aseprite* user_handle = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    if (user_handle->frames != NULL) neko_aseprite_end(user_handle);
    // neko_log_trace("aseprite __gc %p", user_handle);
    return 0;
}

static int __neko_bind_tiled_create(lua_State* L) {
    const_str map_path = lua_tostring(L, 1);
    const_str glsl_vs_src = lua_tostring(L, 2);
    const_str glsl_fs_src = lua_tostring(L, 3);

    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_newuserdata(L, sizeof(neko_tiled_renderer));
    memset(user_handle, 0, sizeof(neko_tiled_renderer));

    neko_tiled_load(&(user_handle->map), map_path, NULL);

    neko_tiled_render_init(&CL_GAME_USERDATA()->cb, user_handle, glsl_vs_src, glsl_fs_src);

    return 1;
}

static int __neko_bind_tiled_render(lua_State* L) {
    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)lua_touserdata(L, 1);

    neko_renderpass_t rp = R_RENDER_PASS_DEFAULT;
    neko_lua_auto_struct_to_member(L, neko_renderpass_t, id, &rp, 2);

    auto xform = lua2struct::unpack<neko_vec2>(L, 3);

    f32 l = lua_tonumber(L, 4);
    f32 r = lua_tonumber(L, 5);
    f32 t = lua_tonumber(L, 6);
    f32 b = lua_tonumber(L, 7);

    tiled_render->camera_mat = neko_mat4_ortho(l, r, b, t, -1.0f, 1.0f);

    neko_command_buffer_t* cb = &CL_GAME_USERDATA()->cb;

    neko_render_renderpass_begin(cb, rp);
    {
        neko_tiled_render_begin(cb, tiled_render);

        for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.layers); i++) {
            layer_t* layer = tiled_render->map.layers + i;
            for (u32 y = 0; y < layer->height; y++) {
                for (u32 x = 0; x < layer->width; x++) {
                    tile_t* tile = layer->tiles + (x + y * layer->width);
                    if (tile->id != 0) {
                        tileset_t* tileset = tiled_render->map.tilesets + tile->tileset_id;
                        u32 tsxx = (tile->id % (tileset->width / tileset->tile_width) - 1) * tileset->tile_width;
                        u32 tsyy = tileset->tile_height * ((tile->id - tileset->first_gid) / (tileset->width / tileset->tile_width));
                        neko_tiled_quad_t quad = {.tileset_id = tile->tileset_id,
                                                  .texture = tileset->texture,
                                                  .texture_size = {(f32)tileset->width, (f32)tileset->height},
                                                  .position = {(f32)(x * tileset->tile_width * SPRITE_SCALE) + xform.x, (f32)(y * tileset->tile_height * SPRITE_SCALE) + xform.y},
                                                  .dimentions = {(f32)(tileset->tile_width * SPRITE_SCALE), (f32)(tileset->tile_height * SPRITE_SCALE)},
                                                  .rectangle = {(f32)tsxx, (f32)tsyy, (f32)tileset->tile_width, (f32)tileset->tile_height},
                                                  .color = layer->tint,
                                                  .use_texture = true};
                        neko_tiled_render_push(cb, tiled_render, quad);
                    }
                }
            }
            neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        }

        for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
            object_group_t* group = tiled_render->map.object_groups + i;
            for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
                object_t* object = group->objects + ii;
                neko_tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE) + xform.x, (f32)(object->y * SPRITE_SCALE) + xform.y},
                                          .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
                                          .color = group->color,
                                          .use_texture = false};
                neko_tiled_render_push(cb, tiled_render, quad);
            }
            neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        }

        // for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        //     object_group_t *group = tiled_render->map.object_groups + i;
        //     for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
        //         object_t *object = group->objects + ii;
        //         auto draw_poly = [sprite_batch](c2Poly poly) {
        //             c2v *verts = poly.verts;
        //             int count = poly.count;
        //             for (int i = 0; i < count; ++i) {
        //                 int iA = i;
        //                 int iB = (i + 1) % count;
        //                 c2v a = verts[iA];
        //                 c2v b = verts[iB];
        //                 gl_line(sprite_batch, a.x, a.y, 0, b.x, b.y, 0);
        //             }
        //         };
        //         draw_poly(object->phy.poly);
        //     }
        // }
    }
    neko_render_renderpass_end(cb);

    return 0;
}

static int __neko_bind_tiled_unload(lua_State* L) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    neko_tiled_unload(&user_handle->map);
    return 0;
}

static int __neko_bind_tiled_load(lua_State* L) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    const_str path = lua_tostring(L, 2);
    neko_tiled_load(&user_handle->map, path, NULL);
    return 0;
}

static int __neko_bind_tiled_end(lua_State* L) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    neko_tiled_unload(&user_handle->map);
    neko_tiled_render_deinit(user_handle);
    return 0;
}

auto __neko_bind_tiled_get_objects(void* tiled_render_ud) {
    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)tiled_render_ud;
    std::map<std::string, std::vector<std::list<f32>>> data;
    for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        object_group_t* group = tiled_render->map.object_groups + i;
        data.insert({group->name, {}});
        for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
            object_t* object = group->objects + ii;
            auto& contain = data[group->name];
            contain.push_back(std::list{(f32)(object->x * SPRITE_SCALE), (f32)(object->y * SPRITE_SCALE), (f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)});
        }
    }
    return data;
}

#if 0
static int __neko_bind_pixelui_create(lua_State* L) {
    // const_str file_path = lua_tostring(L, 1);

    pixelui_t* user_handle = (pixelui_t*)lua_newuserdata(L, sizeof(pixelui_t));
    memset(user_handle, 0, sizeof(pixelui_t));

    // user_handle->show_frame_count = true;
    user_handle->show_material_selection_panel = true;

    pixelui_init(user_handle);

    return 1;
}

static int __neko_bind_pixelui_update(lua_State* L) {
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    update_ui(user_handle);
    return 0;
}

static int __neko_bind_pixelui_tex(lua_State* L) {
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    neko_lua_auto_struct_push_member(L, neko_texture_t, id, &user_handle->tex_ui);
    return 1;
}

static int __neko_bind_pixelui_end(lua_State* L) {
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    pixelui_destroy(user_handle);
    return 0;
}
#endif

static int __neko_bind_json_read(lua_State* L) {

    neko::string str = neko::luax_check_string(L, 1);

    neko::JSONDocument doc = {};
    doc.parse(str);
    neko_defer(doc.trash());

    if (doc.error.len != 0) {
        lua_pushnil(L);
        lua_pushlstring(L, doc.error.data, doc.error.len);
        return 2;
    }

    json_to_lua(L, &doc.root);
    return 1;
}

static int __neko_bind_json_write(lua_State* L) {
    lua_Integer width = luaL_optinteger(L, 2, 0);

    neko::string contents = {};
    neko::string err = lua_to_json_string(L, 1, &contents, width);
    if (err.len != 0) {
        lua_pushnil(L);
        lua_pushlstring(L, err.data, err.len);
        return 2;
    }

    lua_pushlstring(L, contents.data, contents.len);
    neko_safe_free(contents.data);
    return 1;
}

static int __neko_bind_gfxt_create(lua_State* L) {
    const_str pip_path = lua_tostring(L, 1);   // gamedir/assets/pipelines/simple.sf
    const_str gltf_path = lua_tostring(L, 2);  // gamedir/assets/meshes/Duck.gltf
    const_str tex_path = lua_tostring(L, 3);   // gamedir/assets/textures/DuckCM.png

    neko_draw_renderer* gfxt_render = (neko_draw_renderer*)lua_newuserdata(L, sizeof(neko_draw_renderer));
    memset(gfxt_render, 0, sizeof(neko_draw_renderer));

    // Load pipeline from resource file
    gfxt_render->pip = neko_draw_pipeline_load_from_file(pip_path);

    // Create material using this pipeline
    neko_draw_material_desc_t mat_decl = {.pip_func = {.hndl = &gfxt_render->pip}};
    gfxt_render->mat = neko_draw_material_create(&mat_decl);

    // Create mesh that uses the layout from the pipeline's requested mesh layout
    neko_draw_mesh_import_options_t mesh_decl = {.layout = gfxt_render->pip.mesh_layout,
                                                 .size = neko_dyn_array_size(gfxt_render->pip.mesh_layout) * sizeof(neko_draw_mesh_layout_t),
                                                 .index_buffer_element_size = gfxt_render->pip.desc.raster.index_buffer_element_size};

    gfxt_render->mesh = neko_draw_mesh_load_from_file(gltf_path, &mesh_decl);

    gfxt_render->texture = neko_draw_texture_load_from_file(tex_path, NULL, false, false);

    return 1;
}

static int __neko_bind_gfxt_update(lua_State* L) {
    neko_draw_renderer* gfxt_render = (neko_draw_renderer*)lua_touserdata(L, 1);

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    const f32 t = neko_platform_elapsed_time();

    neko_command_buffer_t* cb = &CL_GAME_USERDATA()->cb;

    // Camera for scene
    neko_camera_t cam = neko_camera_perspective();
    cam.transform.position = neko_v3(0.f, 6.f, 20.f);
    neko_vqs trans = {.translation = neko_v3(0.f, 0.f, -10.f), .rotation = neko_quat_angle_axis(t * 0.001f, NEKO_YAXIS), .scale = neko_v3s(0.1f)};
    neko_mat4 model = neko_vqs_to_mat4(&trans);
    neko_mat4 vp = neko_camera_get_view_projection(&cam, fbs.x, fbs.y);
    neko_mat4 mvp = neko_mat4_mul(vp, model);

    // Apply material uniforms
    neko_draw_material_set_uniform(&gfxt_render->mat, "u_mvp", &mvp);
    neko_draw_material_set_uniform(&gfxt_render->mat, "u_tex", &gfxt_render->texture);

    // Rendering
    neko_render_renderpass_begin(cb, R_RENDER_PASS_DEFAULT);
    {
        // Set view port
        neko_render_set_viewport(cb, 0, 0, (int)fbs.x, (int)fbs.y);

        // Bind material
        neko_draw_material_bind(cb, &gfxt_render->mat);

        // Bind material uniforms
        neko_draw_material_bind_uniforms(cb, &gfxt_render->mat);

        // Render mesh
        neko_draw_mesh_draw_material(cb, &gfxt_render->mesh, &gfxt_render->mat);
    }
    neko_render_renderpass_end(cb);

    return 0;
}

static int __neko_bind_gfxt_end(lua_State* L) {
    neko_draw_renderer* user_handle = (neko_draw_renderer*)lua_touserdata(L, 1);

    neko_draw_texture_destroy(&user_handle->texture);
    neko_draw_mesh_destroy(&user_handle->mesh);
    neko_draw_material_destroy(&user_handle->mat);
    neko_draw_pipeline_destroy(&user_handle->pip);

    return 0;
}

static int __neko_bind_draw_text(lua_State* L) {

    int numArgs = lua_gettop(L);  // 获取参数数量

    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);

    f32 scale = 0.f;

    if (numArgs > 3) scale = lua_tonumber(L, 4);

    neko_fontbatch_draw(&CL_GAME_USERDATA()->font_render_batch, CL_GAME_USERDATA()->fbs, text, x, y, 1, 1.f, 800.f, scale);

    return 0;
}

typedef struct {
    u64 image_id;  // NEKO_SPRITEBATCH_U64
    int depth;
    f32 x;
    f32 y;
    f32 sx;
    f32 sy;
    f32 c;
    f32 s;
} neko_sprite_t;

typedef struct {
    f32 x, y;
    f32 u, v;
} vertex_t;

#define SPRITE_VERTS_MAX (1024 * 10)

struct neko_sprite_batch_t {

    neko_spritebatch_t sb;

    neko_render_batch_context_t* sprite_batch;
    neko_render_batch_shader_t sprite_shader;
    neko_render_batch_renderable_t sprite_renderable;
    f32 sprite_projection[16];

    int call_count = 0;

    int sprite_verts_count;
    vertex_t sprite_verts[SPRITE_VERTS_MAX];

    neko_image_t* images;
    int images_count = 0;
};

static void make_sprite(neko_sprite_batch_t* user_handle, neko_sprite_t* sprite, u64 image_id, f32 x, f32 y, f32 scale, f32 angle_radians, int depth) {

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    // f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    // f32 y0 = (fbs.y / 2.f - y) / scale / 2.f;

    f32 x0 = x;
    f32 y0 = y;

    sprite->image_id = image_id;
    sprite->depth = depth;
    sprite->x = x0;
    sprite->y = y0;
    sprite->sx = (f32)user_handle->images[sprite->image_id].w * 2.0f * scale;
    sprite->sy = (f32)user_handle->images[sprite->image_id].h * 2.0f * scale;
    sprite->c = cosf(angle_radians);
    sprite->s = sinf(angle_radians);
}

static void push_sprite(neko_sprite_batch_t* user_handle, neko_sprite_t* sp) {
    neko_spritebatch_sprite_t s;
    s.image_id = sp->image_id;
    s.w = user_handle->images[sp->image_id].w;
    s.h = user_handle->images[sp->image_id].h;
    s.x = sp->x;
    s.y = sp->y;
    s.sx = sp->sx;
    s.sy = sp->sy;
    s.c = sp->c;
    s.s = sp->s;
    s.sort_bits = (u64)sp->depth;
    neko_spritebatch_push(&user_handle->sb, s);
}

static void batch_report(neko_spritebatch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata) {

    (void)texture_w;
    (void)texture_h;

    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)udata;

    ++user_handle->call_count;

    // printf("begin batch\n");
    // for (int i = 0; i < count; ++i) printf("\t%llu\n", sprites[i].texture_id);
    // printf("end batch\n");

    // build the draw call
    neko_render_batch_draw_call_t call;
    call.r = &user_handle->sprite_renderable;
    call.textures[0] = (u32)sprites[0].texture_id;
    call.texture_count = 1;

    // set texture uniform in shader
    neko_render_batch_send_texture(call.r->program, "u_sprite_texture", 0);

    // TODO: 附加排序 遮盖判定系统

    // build vertex buffer of quads from all sprite transforms
    call.verts = user_handle->sprite_verts + user_handle->sprite_verts_count;
    call.vert_count = count * 6;
    user_handle->sprite_verts_count += call.vert_count;
    assert(user_handle->sprite_verts_count < SPRITE_VERTS_MAX);

    vertex_t* verts = (vertex_t*)call.verts;
    for (int i = 0; i < count; ++i) {
        neko_spritebatch_sprite_t* s = sprites + i;

        neko_vec2 quad[] = {
                {-0.5f, 0.5f},
                {0.5f, 0.5f},
                {0.5f, -0.5f},
                {-0.5f, -0.5f},
        };

        for (int j = 0; j < 4; ++j) {
            f32 x = quad[j].x;
            f32 y = quad[j].y;

            // scale sprite about origin
            x *= s->sx;
            y *= s->sy;

            // rotate sprite about origin
            f32 x0 = s->c * x - s->s * y;
            f32 y0 = s->s * x + s->c * y;
            x = x0;
            y = y0;

            // translate sprite into the world
            x += s->x;
            y += s->y;

            quad[j].x = x;
            quad[j].y = y;
        }

        // output transformed quad into CPU buffer
        vertex_t* out_verts = verts + i * 6;

        out_verts[0].x = quad[0].x;
        out_verts[0].y = quad[0].y;
        out_verts[0].u = s->minx;
        out_verts[0].v = s->maxy;

        out_verts[1].x = quad[3].x;
        out_verts[1].y = quad[3].y;
        out_verts[1].u = s->minx;
        out_verts[1].v = s->miny;

        out_verts[2].x = quad[1].x;
        out_verts[2].y = quad[1].y;
        out_verts[2].u = s->maxx;
        out_verts[2].v = s->maxy;

        out_verts[3].x = quad[1].x;
        out_verts[3].y = quad[1].y;
        out_verts[3].u = s->maxx;
        out_verts[3].v = s->maxy;

        out_verts[4].x = quad[3].x;
        out_verts[4].y = quad[3].y;
        out_verts[4].u = s->minx;
        out_verts[4].v = s->miny;

        out_verts[5].x = quad[2].x;
        out_verts[5].y = quad[2].y;
        out_verts[5].u = s->maxx;
        out_verts[5].v = s->miny;
    }

    // 提交对draw_call的调用
    neko_render_batch_push_draw_call(user_handle->sprite_batch, call);
}

void get_pixels(u64 image_id, void* buffer, int bytes_to_fill, void* udata) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)udata;
    memcpy(buffer, user_handle->images[image_id].pix, bytes_to_fill);
}

static int __neko_bind_sprite_batch_create(lua_State* L) {

    int max_draw_calls = lua_tointeger(L, 1);

    if (!lua_istable(L, 2)) {
        lua_pushstring(L, "Argument is not a table");
        lua_error(L);
        return 0;
    }

    int count = luaL_len(L, 2);
    std::vector<const_str> texture_list;

    for (int i = 0; i < count; i++) {
        lua_rawgeti(L, 2, i + 1);
        const_str path = lua_tostring(L, -1);
        texture_list.push_back(path);

        lua_pop(L, 1);
    }

    const_str glsl_vs_src = lua_tostring(L, 3);
    const_str glsl_ps_src = lua_tostring(L, 4);

    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_newuserdata(L, sizeof(neko_sprite_batch_t));
    memset(user_handle, 0, sizeof(neko_sprite_batch_t));

    user_handle->sprite_batch = neko_render_batch_make_ctx(max_draw_calls);

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    neko_render_batch_vertex_data_t vd;
    neko_render_batch_make_vertex_data(&vd, 1024 * 1024, GL_TRIANGLES, sizeof(vertex_t), GL_DYNAMIC_DRAW);
    neko_render_batch_add_attribute(&vd, "in_pos", 2, R_BATCH_FLOAT, NEKO_OFFSET(vertex_t, x));
    neko_render_batch_add_attribute(&vd, "in_uv", 2, R_BATCH_FLOAT, NEKO_OFFSET(vertex_t, u));

    neko_render_batch_make_renderable(&user_handle->sprite_renderable, &vd);
    neko_render_batch_load_shader(&user_handle->sprite_shader, glsl_vs_src, glsl_ps_src);
    neko_render_batch_set_shader(&user_handle->sprite_renderable, &user_handle->sprite_shader);

    neko_render_batch_ortho_2d(fbs.x, fbs.y, 0, 0, user_handle->sprite_projection);

    neko_render_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

    user_handle->images_count = count;
    user_handle->images = (neko_image_t*)neko_safe_malloc(sizeof(neko_image_t) * count);

    for (s32 i = 0; i < user_handle->images_count; ++i) user_handle->images[i] = neko_image_load(game_assets(texture_list[i]).c_str());

    neko_spritebatch_config_t sb_config;
    neko_spritebatch_set_default_config(&sb_config);
    sb_config.pixel_stride = sizeof(u8) * 4;
    sb_config.atlas_width_in_pixels = 1024;
    sb_config.atlas_height_in_pixels = 1024;
    sb_config.atlas_use_border_pixels = 0;
    sb_config.ticks_to_decay_texture = 3;
    sb_config.lonely_buffer_count_till_flush = 1;
    sb_config.ratio_to_decay_atlas = 0.5f;
    sb_config.ratio_to_merge_atlases = 0.25f;

    sb_config.batch_callback = batch_report;                        // report batches of sprites from `neko_spritebatch_flush`
    sb_config.get_pixels_callback = get_pixels;                     // used to retrieve image pixels from `neko_spritebatch_flush` and `neko_spritebatch_defrag`
    sb_config.generate_texture_callback = generate_texture_handle;  // used to generate a texture handle from `neko_spritebatch_flush` and `neko_spritebatch_defrag`
    sb_config.delete_texture_callback = destroy_texture_handle;     // used to destroy a texture handle from `neko_spritebatch_defrag`

    neko_spritebatch_init(&user_handle->sb, &sb_config, user_handle);

    return 1;
}

static int __neko_bind_sprite_batch_render_ortho(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    f32 w = lua_tonumber(L, 2);
    f32 h = lua_tonumber(L, 3);
    f32 x = lua_tonumber(L, 4);
    f32 y = lua_tonumber(L, 5);

    neko_render_batch_ortho_2d(w, h, x / w, y / h, user_handle->sprite_projection);

    neko_render_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

    return 0;
}

static int __neko_bind_sprite_batch_make_sprite(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    auto image_id = lua_tointeger(L, 2);
    auto x = lua_tonumber(L, 3);
    auto y = lua_tonumber(L, 4);
    auto scale = lua_tonumber(L, 5);
    auto angle_radians = lua_tonumber(L, 6);
    auto depth = lua_tonumber(L, 7);

    neko_sprite_t* sprite_handle = (neko_sprite_t*)lua_newuserdata(L, sizeof(neko_sprite_t));
    memset(sprite_handle, 0, sizeof(neko_sprite_t));

    make_sprite(user_handle, sprite_handle, image_id, x, y, scale, angle_radians, depth);

    return 1;
}

static int __neko_bind_sprite_batch_push_sprite(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);
    neko_sprite_t* sprite_handle = (neko_sprite_t*)lua_touserdata(L, 2);
    push_sprite(user_handle, sprite_handle);
    return 0;
}

static int __neko_bind_sprite_batch_render_begin(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    user_handle->call_count = 0;

    return 0;
}

static int __neko_bind_sprite_batch_render_end(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    {
        neko_spritebatch_defrag(&user_handle->sb);
        neko_spritebatch_tick(&user_handle->sb);
        neko_spritebatch_flush(&user_handle->sb);
        user_handle->sprite_verts_count = 0;
    }

    neko_render_draw_batch(&CL_GAME_USERDATA()->cb, user_handle->sprite_batch, 0, 0, 0);

    return 0;
}

static int __neko_bind_sprite_batch_end(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    neko_spritebatch_term(&user_handle->sb);

    for (s32 i = 0; i < user_handle->images_count; ++i) {
        neko_image_free(*(user_handle->images + i));
    }

    neko_safe_free(user_handle->images);

    neko_render_batch_free(user_handle->sprite_batch);

    return 0;
}

// 测试 luacstruct 用
struct CGameObject {
    int id;
    bool active;
    bool visible;
    bool selected;
};

// template <>
// struct neko::static_refl::neko_type_info<CGameObject> : neko_type_info_base<CGameObject> {
//     static constexpr AttrList attrs = {};
//     static constexpr FieldList fields = {
//             rf_field{TSTR("id"), &rf_type::id},              //
//             rf_field{TSTR("active"), &rf_type::active},      //
//             rf_field{TSTR("visible"), &rf_type::visible},    //
//             rf_field{TSTR("selected"), &rf_type::selected},  //
//     };
// };

REGISTER_TYPE_DF(s8)
REGISTER_TYPE_DF(s16)
REGISTER_TYPE_DF(s32)
REGISTER_TYPE_DF(s64)
REGISTER_TYPE_DF(u8)
REGISTER_TYPE_DF(u16)
REGISTER_TYPE_DF(u32)
REGISTER_TYPE_DF(u64)
REGISTER_TYPE_DF(bool)
// REGISTER_TYPE_DF(b32)
REGISTER_TYPE_DF(f32)
REGISTER_TYPE_DF(f64)
REGISTER_TYPE_DF(const_str)

namespace neko::reflection {
template <>
Type* type_of<CGameObject>() {
    static Type type;
    type.name = "CGameObject";
    type.destroy = [](void* obj) { delete static_cast<CGameObject*>(obj); };
    type.copy = [](const void* obj) { return (void*)(new CGameObject(*static_cast<const CGameObject*>(obj))); };
    type.move = [](void* obj) { return (void*)(new CGameObject(std::move(*static_cast<CGameObject*>(obj)))); };
    type.fields.insert({"id", {type_of<decltype(CGameObject::id)>(), offsetof(CGameObject, id)}});
    type.fields.insert({"active", {type_of<decltype(CGameObject::active)>(), offsetof(CGameObject, active)}});
    type.fields.insert({"visible", {type_of<decltype(CGameObject::visible)>(), offsetof(CGameObject, visible)}});
    type.fields.insert({"selected", {type_of<decltype(CGameObject::selected)>(), offsetof(CGameObject, selected)}});
    // type.methods.insert({"say", type_ensure<&Person::say>()});
    return &type;
};
}  // namespace neko::reflection

DEFINE_IMGUI_BEGIN(template <>, CGameObject) {
    // neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { neko::imgui::Auto(value, std::string(field.name)); });

    auto f = [](std::string_view name, neko::reflection::Any& value) {
        // if (value.GetType() == neko::reflection::type_of<std::string_view>()) {
        //     std::cout << name << " = " << value.cast<std::string_view>() << std::endl;
        // } else if (value.GetType() == neko::reflection::type_of<std::size_t>()) {
        //     std::cout << name << " = " << value.cast<std::size_t>() << std::endl;
        // }
        neko::imgui::Auto(value, std::string(name));
    };

    neko::reflection::Any v = var;

    v.foreach (f);
}
DEFINE_IMGUI_END();

static int __neko_bind_gameobject_inspect(lua_State* L) {

    CGameObject* user_handle = (CGameObject*)lua_touserdata(L, 1);

    if (user_handle == NULL) return 0;

    // neko_println("gameobj %d %s %s %s", user_handle->id, NEKO_BOOL_STR(user_handle->active), NEKO_BOOL_STR(user_handle->visible), NEKO_BOOL_STR(user_handle->selected));

    ImGui::Text("GameObject_%d", user_handle->id);

    neko::imgui::Auto(user_handle, "CGameObject");

    return 0;
}

#if 0
static int __neko_bind_filesystem_create(lua_State* L) {
    neko_filesystem_t* filewatch = (neko_filesystem_t*)lua_newuserdata(L, sizeof(neko_filesystem_t));
    memset(filewatch, 0, sizeof(neko_filesystem_t));
    neko_filesystem_create_internal(filewatch, NULL);

    return 1;
}

static int __neko_bind_filesystem_destory(lua_State* L) {
    neko_filesystem_t* assetsys = (neko_filesystem_t*)lua_touserdata(L, 1);
    neko_filesystem_destroy_internal(assetsys);
    return 0;
}

static int __neko_bind_filewatch_create(lua_State* L) {
    neko_filesystem_t* assetsys = (neko_filesystem_t*)lua_touserdata(L, 1);

    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_newuserdata(L, sizeof(neko_filewatch_t));
    memset(filewatch, 0, sizeof(neko_filewatch_t));
    neko_filewatch_create_internal(filewatch, assetsys, NULL);
    return 1;
}

static int __neko_bind_filewatch_destory(lua_State* L) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_free_internal(filewatch);
    return 0;
}

static void watch_map_callback(neko_filewatch_update_t change, const_str virtual_path, void* udata) {
    std::string change_string;
    switch (change) {
        case FILEWATCH_DIR_ADDED:
            change_string = "FILEWATCH_DIR_ADDED";
            break;
        case FILEWATCH_DIR_REMOVED:
            change_string = "FILEWATCH_DIR_REMOVED";
            break;
        case FILEWATCH_FILE_ADDED:
            change_string = "FILEWATCH_FILE_ADDED";
            break;
        case FILEWATCH_FILE_REMOVED:
            change_string = "FILEWATCH_FILE_REMOVED";
            break;
        case FILEWATCH_FILE_MODIFIED:
            change_string = "FILEWATCH_FILE_MODIFIED";
            break;
    }

    const_str callback_funcname = (const_str)udata;

    lua_getglobal(CL_GAME_USERDATA()->L, callback_funcname);
    bool is_callback = lua_isfunction(CL_GAME_USERDATA()->L, -1);
    lua_pop(CL_GAME_USERDATA()->L, 1);

    if (is_callback) try {
            neko_lua_call<void>(CL_GAME_USERDATA()->L, callback_funcname, change_string, std::string(virtual_path));
        } catch (std::exception& ex) {
            neko_log_error("lua exception %s", ex.what());
        }
}

static int __neko_bind_filewatch_mount(lua_State* L) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str actual_path = lua_tostring(L, 2);
    const_str virtual_path = lua_tostring(L, 3);
    neko_filewatch_mount(filewatch, actual_path, virtual_path);
    return 0;
}

static int __neko_bind_filewatch_start_watch(lua_State* L) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str virtual_path = lua_tostring(L, 2);
    const_str callback_funcname = lua_tostring(L, 3);
    neko_filewatch_start_watching(filewatch, virtual_path, watch_map_callback, (void*)callback_funcname);
    return 0;
}

static int __neko_bind_filewatch_stop_watching(lua_State* L) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str virtual_path = lua_tostring(L, 2);
    neko_filewatch_stop_watching(filewatch, virtual_path);
    return 0;
}

static int __neko_bind_filewatch_update(lua_State* L) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_update(filewatch);
    return 0;
}

static int __neko_bind_filewatch_notify(lua_State* L) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_notify(filewatch);
    return 0;
}
#endif

static int __neko_bind_idraw_get(lua_State* L) {
    lua_pushlightuserdata(L, &CL_GAME_USERDATA()->idraw);
    return 1;
}

static int __neko_bind_idraw_draw(lua_State* L) {
    neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);
    return 1;
}

static int __neko_bind_idraw_defaults(lua_State* L) {
    neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
    return 0;
}

static int __neko_bind_idraw_camera2d(lua_State* L) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, w, h);
    return 0;
}

static int __neko_bind_idraw_camera3d(lua_State* L) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera3d(&CL_GAME_USERDATA()->idraw, w, h);
    return 0;
}

static int __neko_bind_idraw_camera2d_ex(lua_State* L) {
    f32 l = lua_tonumber(L, 1);
    f32 r = lua_tonumber(L, 2);
    f32 t = lua_tonumber(L, 3);
    f32 b = lua_tonumber(L, 4);
    neko_idraw_camera2d_ex(&CL_GAME_USERDATA()->idraw, l, r, t, b);
    return 0;
}

static int __neko_bind_idraw_rotatev(lua_State* L) {
    f32 angle = lua_tonumber(L, 1);
    f32 x = lua_tonumber(L, 2);
    f32 y = lua_tonumber(L, 3);
    f32 z = lua_tonumber(L, 4);
    neko_idraw_rotatev(&CL_GAME_USERDATA()->idraw, angle, neko_v3(x, y, z));
    return 0;
}

static int __neko_bind_idraw_box(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    f32 hx = lua_tonumber(L, 4);
    f32 hy = lua_tonumber(L, 5);
    f32 hz = lua_tonumber(L, 6);
    u8 r = lua_tointeger(L, 7);
    u8 g = lua_tointeger(L, 8);
    u8 b = lua_tointeger(L, 9);
    u8 a = lua_tointeger(L, 10);
    neko_render_primitive_type type_val;
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_render_primitive_type, &type_val, 11);
    neko_idraw_box(&CL_GAME_USERDATA()->idraw, x, y, z, hx, hy, hz, r, g, b, a, type_val);
    return 0;
}

static int __neko_bind_idraw_translatef(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    neko_idraw_translatef(&CL_GAME_USERDATA()->idraw, x, y, z);
    return 0;
}

static int __neko_bind_idraw_rectv(lua_State* L) {

    auto v1 = lua2struct::unpack<neko_vec2>(L, 1);
    auto v2 = lua2struct::unpack<neko_vec2>(L, 2);

    v2 = neko_vec2_add(v1, v2);

    neko_render_primitive_type type_val;
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_render_primitive_type, &type_val, 3);

    neko_color_t col = NEKO_COLOR_WHITE;

    if (lua_gettop(L) == 4) {
        col = lua2struct::unpack<neko_color_t>(L, 4);
    }

    neko_idraw_rectv(&CL_GAME_USERDATA()->idraw, v1, v2, col, type_val);
    return 0;
}

static int __neko_bind_idraw_rectvd(lua_State* L) {

    auto v1 = lua2struct::unpack<neko_vec2>(L, 1);
    auto v2 = lua2struct::unpack<neko_vec2>(L, 2);

    auto uv0 = lua2struct::unpack<neko_vec2>(L, 3);
    auto uv1 = lua2struct::unpack<neko_vec2>(L, 4);

    neko_render_primitive_type type_val;
    neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_render_primitive_type, &type_val, 5);

    neko_color_t col = lua2struct::unpack<neko_color_t>(L, 6);

    neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, v1, v2, uv0, uv1, col, type_val);
    return 0;
}

static int __neko_bind_idraw_text(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);

    neko_color_t col = neko_color(255, 50, 50, 255);

    if (lua_gettop(L) == 4) {
        col = lua2struct::unpack<neko_color_t>(L, 4);
    }

    neko_idraw_text(&CL_GAME_USERDATA()->idraw, x, y, text, NULL, false, col);
    return 0;
}

static int __neko_bind_idraw_camera(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    neko_camera_t camera;
    camera = neko_camera_default();
    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_camera(&CL_GAME_USERDATA()->idraw, &camera, (u32)fbs.x, (u32)fbs.y);
    return 0;
}

static int __neko_bind_idraw_depth_enabled(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_depth_enabled(&CL_GAME_USERDATA()->idraw, enable);
    return 0;
}

static int __neko_bind_idraw_face_cull_enabled(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_face_cull_enabled(&CL_GAME_USERDATA()->idraw, enable);
    return 0;
}

static int __neko_bind_idraw_texture(lua_State* L) {
    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(CL_GAME_USERDATA()->L, neko_texture_t, id, &rt, 1);
    neko_idraw_texture(&CL_GAME_USERDATA()->idraw, rt);
    return 0;
}

static int __neko_bind_render_framebuffer_create(lua_State* L) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    fbo = neko_render_framebuffer_create({});
    neko_lua_auto_struct_push_member(L, neko_framebuffer_t, id, &fbo);
    return 1;
}

static int __neko_bind_render_framebuffer_destroy(lua_State* L) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);
    neko_render_framebuffer_destroy(fbo);
    return 0;
}

static int __neko_bind_render_shader_create(lua_State* L) {

    const_str shader_name = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);              // 检查是否为table
    int n = neko_lua_get_table_pairs_count(L, 2);  //

    neko_render_shader_source_desc_t* sources = (neko_render_shader_source_desc_t*)neko_safe_malloc(n * sizeof(neko_render_shader_source_desc_t));

    const_str shader_type[] = {"VERTEX", "FRAGMENT", "COMPUTE"};

    int j = 0;

    for (int i = 0; i < 3; i++) {
        lua_pushstring(L, shader_type[i]);  // # -1
        lua_gettable(L, -2);                // pop # -1
        if (!lua_isnil(L, -1)) {
            const_str src = lua_tostring(L, -1);  // # -1
            if (src != NULL) {
                sources[j++] = neko_render_shader_source_desc_t{.type = (neko_render_shader_stage_type)(i + 1), .source = src};
            }
        }
        lua_pop(L, 1);  // # -1
    }

    NEKO_ASSERT(j == n);

    neko_shader_t shader_handle = NEKO_DEFAULT_VAL();

    neko_render_shader_desc_t shader_desc = {.sources = sources, .size = n * sizeof(neko_render_shader_source_desc_t), .name = "unknown"};
    strncpy(shader_desc.name, shader_name, sizeof(shader_desc.name) - 1);
    shader_desc.name[sizeof(shader_desc.name) - 1] = '\0';

    shader_handle = neko_render_shader_create(shader_desc);

    neko_safe_free(sources);

    neko_lua_auto_struct_push_member(L, neko_shader_t, id, &shader_handle);

    return 1;
}

static int __neko_bind_render_uniform_create(lua_State* L) {

    const_str uniform_name = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    int n = lua_rawlen(L, 2);          //

    neko_render_uniform_layout_desc_t* layouts = (neko_render_uniform_layout_desc_t*)neko_safe_malloc(n * sizeof(neko_render_uniform_layout_desc_t));
    memset(layouts, 0, n * sizeof(neko_render_uniform_layout_desc_t));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);  // 将index=i的元素压入堆栈顶部
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "type");  // # -1
        lua_gettable(L, -2);        // pop # -1
        if (!lua_isnil(L, -1)) {
            neko_lua_auto_to(L, neko_render_uniform_type, &layouts[i - 1].type, -1);
        }
        lua_pop(L, 1);  // # -1

        // lua_pushstring(L, "name");  // # -1
        // lua_gettable(L, -2);        // pop # -1
        // if (!lua_isnil(L, -1)) {
        //     const_str name = lua_tostring(L, -1);
        //     if (NULL != name) {
        //         strncpy(layouts[i - 1].fname, name, sizeof(layouts[i - 1].fname) - 1);
        //         layouts[i - 1].fname[sizeof(layouts[i - 1].fname) - 1] = '\0';
        //     }
        // }
        // lua_pop(L, 1);  // # -1

        lua_pushstring(L, "count");  // # -1
        lua_gettable(L, -2);         // pop # -1
        if (lua_isinteger(L, -1)) {
            layouts[i - 1].count = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);  // # -1

        lua_pop(L, 1);  // # -1
    }

    neko_render_uniform_desc_t u_desc = {
            // .name = "unknown",
            .layout = layouts,
    };

    if (!!strcmp(uniform_name, "default")) {
        NEKO_STR_CPY(u_desc.name, uniform_name);
    }

    if (lua_gettop(L) == 3) {
        neko_lua_auto_to(L, neko_render_shader_stage_type, &u_desc.stage, 3);
    }

    neko_uniform_t uniform_handle = NEKO_DEFAULT_VAL();

    // Create uniform
    uniform_handle = neko_render_uniform_create(u_desc);

    neko_safe_free(layouts);

    neko_lua_auto_struct_push_member(L, neko_uniform_t, id, &uniform_handle);

    return 1;
}

static int __neko_bind_render_pipeline_create(lua_State* L) {
    const_str pipeline_name = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);              // 检查是否为table
    int n = neko_lua_get_table_pairs_count(L, 2);  //
    const_str pipeline_type[] = {"compute", "layout", "raster"};

    neko_render_pipeline_desc_t pipeline_desc = NEKO_DEFAULT_VAL();

    for (int i = 0; i < NEKO_ARR_SIZE(pipeline_type); i++) {
        lua_pushstring(L, pipeline_type[i]);  // # -1
        lua_gettable(L, -2);                  // pop # -1
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            int n = neko_lua_get_table_pairs_count(L, -1);  //

            switch (neko::hash(pipeline_type[i])) {
                case neko::hash("compute"): {

                    neko_shader_t shader_handle = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "shader");  // # -1
                    lua_gettable(L, -2);          // pop # -1
                    if (!lua_isnil(L, -1)) {
                        neko_lua_auto_struct_to_member(L, neko_shader_t, id, &shader_handle, -1);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.compute.shader = shader_handle;
                } break;
                case neko::hash("layout"): {
                    neko_render_vertex_attribute_desc_t* vertex_attr = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "attrs");  // # -1
                    lua_gettable(L, -2);         // pop # -1
                    if (!lua_isnil(L, -1)) {
                        vertex_attr = (neko_render_vertex_attribute_desc_t*)lua_touserdata(L, -1);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.layout.attrs = vertex_attr;

                    lua_pushstring(L, "size");  // # -1
                    lua_gettable(L, -2);        // pop # -1
                    if (lua_isinteger(L, -1)) {
                        pipeline_desc.layout.size = (size_t)lua_tointeger(L, -1);
                    } else
                        NEKO_ASSERT(false);
                    lua_pop(L, 1);  // # -1
                } break;
                case neko::hash("raster"): {

                    neko_shader_t shader_handle = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "shader");  // # -1
                    lua_gettable(L, -2);          // pop # -1
                    if (!lua_isnil(L, -1)) {
                        neko_lua_auto_struct_to_member(L, neko_shader_t, id, &shader_handle, -1);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.raster.shader = shader_handle;

                    lua_pushstring(L, "index_buffer_element_size");  // # -1
                    lua_gettable(L, -2);                             // pop # -1
                    if (lua_isinteger(L, -1)) {
                        pipeline_desc.raster.index_buffer_element_size = lua_tointeger(L, -1);
                    } else
                        NEKO_ASSERT(false);
                    lua_pop(L, 1);  // # -1

                } break;
                default:
                    NEKO_ASSERT(false);
                    break;
            }
        }
        lua_pop(L, 1);  // # -1
    }

    neko_pipeline_t pipeline_handle = NEKO_DEFAULT_VAL();
    pipeline_handle = neko_render_pipeline_create(pipeline_desc);
    neko_lua_auto_struct_push_member(L, neko_pipeline_t, id, &pipeline_handle);
    return 1;
}

static int __neko_bind_render_vertex_buffer_create(lua_State* L) {
    const_str vertex_buffer_name = lua_tostring(L, 1);
    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);
    neko_vbo_t vertex_buffer_handle = NEKO_DEFAULT_VAL();
    neko_render_vertex_buffer_desc_t vertex_buffer_desc = {.data = data, .size = data_size};
    vertex_buffer_handle = neko_render_vertex_buffer_create(vertex_buffer_desc);
    neko_lua_auto_struct_push_member(L, neko_vbo_t, id, &vertex_buffer_handle);
    return 1;
}

static int __neko_bind_render_index_buffer_create(lua_State* L) {
    const_str index_buffer_name = lua_tostring(L, 1);
    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);
    neko_ibo_t index_buffer_handle = NEKO_DEFAULT_VAL();
    neko_render_index_buffer_desc_t index_buffer_desc = {.data = data, .size = data_size};
    index_buffer_handle = neko_render_index_buffer_create(index_buffer_desc);
    neko_lua_auto_struct_push_member(L, neko_ibo_t, id, &index_buffer_handle);
    return 1;
}

static int __neko_bind_render_vertex_attribute_create(lua_State* L) {
    const_str vertex_attribute_name = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int n = neko_lua_get_table_pairs_count(L, 2);  //

    neko_render_vertex_attribute_desc_t* sources = (neko_render_vertex_attribute_desc_t*)neko_safe_malloc(n * sizeof(neko_render_vertex_attribute_desc_t));
    memset(sources, 0, n * sizeof(neko_render_vertex_attribute_desc_t));

    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, 2, i + 1);  // 将index=i的元素压入堆栈顶部
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "name");  // # -1
        lua_gettable(L, -2);        // pop # -1
        if (!lua_isnil(L, -1)) {
            const_str src = lua_tostring(L, -1);  // # -1
            if (src != NULL) {
                NEKO_STR_CPY(sources[i].name, src);
            } else
                NEKO_ASSERT(false);
        }
        lua_pop(L, 1);  // # -1

        lua_pushstring(L, "format");  // # -1
        lua_gettable(L, -2);          // pop # -1
        if (!lua_isnil(L, -1)) {
            neko_lua_auto_to(L, neko_render_vertex_attribute_type, &sources[i].format, -1);
        }
        lua_pop(L, 1);  // # -1

        lua_pop(L, 1);  // # -1
    }

    neko_render_vertex_attribute_desc_t* push_userdata = (neko_render_vertex_attribute_desc_t*)lua_newuserdata(L, n * sizeof(neko_render_vertex_attribute_desc_t));
    memcpy(push_userdata, sources, n * sizeof(neko_render_vertex_attribute_desc_t));

    neko_safe_free(sources);

    lua_pushinteger(L, n * sizeof(neko_render_vertex_attribute_desc_t));

    return 2;
}

static int __neko_bind_render_storage_buffer_create(lua_State* L) {

    const_str storage_buffer_name = lua_tostring(L, 1);

    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);

    neko_storage_buffer_t storage_buffer_handle = NEKO_DEFAULT_VAL();

    neko_render_storage_buffer_desc_t storage_buffer_desc = {.data = data, .size = data_size, .name = "unknown", .usage = R_BUFFER_USAGE_DYNAMIC};

    if (storage_buffer_name != NULL) {
        NEKO_STR_CPY(storage_buffer_desc.name, storage_buffer_name);
    }

    storage_buffer_handle = neko_render_storage_buffer_create(storage_buffer_desc);

    neko_lua_auto_struct_push_member(L, neko_storage_buffer_t, id, &storage_buffer_handle);

    return 1;
}

static int __neko_bind_render_pipeline_bind(lua_State* L) {
    neko_pipeline_t pipeline_handle = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_pipeline_t, id, &pipeline_handle, 1);
    neko_render_pipeline_bind(&CL_GAME_USERDATA()->cb, pipeline_handle);
    return 0;
}

static int __neko_bind_render_apply_bindings(lua_State* L) {

    neko_render_bind_desc_t binds = NEKO_DEFAULT_VAL();

    const_str pipeline_type[] = {"uniforms", "image_buffers", "storage_buffers", "vertex_buffers", "index_buffers"};

    neko_render_bind_uniform_desc_t* u_desc = nullptr;
    neko_render_bind_image_buffer_desc_t* ib_desc = nullptr;
    neko_render_bind_storage_buffer_desc_t* sb_desc = nullptr;
    neko_render_bind_vertex_buffer_desc_t* vbo_desc = nullptr;
    neko_render_bind_index_buffer_desc_t* ibo_desc = nullptr;

    luaL_checktype(L, -1, LUA_TTABLE);

    f32 data;  // TODO

    for (int i = 0; i < NEKO_ARR_SIZE(pipeline_type); i++) {
        lua_pushstring(L, pipeline_type[i]);  // # -1
        lua_gettable(L, -2);                  // pop # -1
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            int n = neko_lua_get_table_pairs_count(L, -1);  //

            switch (neko::hash(pipeline_type[i])) {
                case neko::hash("uniforms"): {

                    u_desc = (neko_render_bind_uniform_desc_t*)malloc(n * sizeof(neko_render_bind_uniform_desc_t));
                    memset(u_desc, 0, n * sizeof(neko_render_bind_uniform_desc_t));

                    binds.uniforms.desc = u_desc;
                    binds.uniforms.size = n == 1 ? 0 : n * sizeof(neko_render_bind_uniform_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "uniform");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (!lua_isnil(L, -1))
                            neko_lua_auto_struct_to_member(L, neko_uniform_t, id, &u_desc[i - 1].uniform, -1);
                        else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "data");  // # -1
                        lua_gettable(L, -2);        // pop # -1
                        if (!lua_isnil(L, -1)) {
                            int type = lua_type(L, -1);
                            switch (type) {
                                case LUA_TUSERDATA:
                                    u_desc[i - 1].data = lua_touserdata(L, -1);
                                    break;
                                case LUA_TNUMBER:
                                    data = lua_tonumber(L, -1);
                                    u_desc[i - 1].data = &data;
                                    break;
                                default:
                                    NEKO_ASSERT(false);  // TODO
                                    break;
                            }
                        } else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            u_desc[i - 1].binding = neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("image_buffers"): {
                    ib_desc = (neko_render_bind_image_buffer_desc_t*)malloc(n * sizeof(neko_render_bind_image_buffer_desc_t));
                    memset(ib_desc, 0, n * sizeof(neko_render_bind_image_buffer_desc_t));

                    binds.image_buffers.desc = ib_desc;
                    binds.image_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_image_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "tex");  // # -1
                        lua_gettable(L, -2);       // pop # -1
                        if (!lua_isnil(L, -1))
                            neko_lua_auto_struct_to_member(L, neko_texture_t, id, &ib_desc[i - 1].tex, -1);
                        else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        ib_desc[i - 1].access = R_ACCESS_WRITE_ONLY;

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            ib_desc[i - 1].binding = neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("storage_buffers"): {
                    sb_desc = (neko_render_bind_storage_buffer_desc_t*)malloc(n * sizeof(neko_render_bind_storage_buffer_desc_t));
                    memset(sb_desc, 0, n * sizeof(neko_render_bind_storage_buffer_desc_t));

                    binds.storage_buffers.desc = sb_desc;
                    binds.storage_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_storage_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_lua_auto_struct_to_member(L, neko_storage_buffer_t, id, &sb_desc[i - 1].buffer, -1);
                        } else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            sb_desc[i - 1].binding = neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("vertex_buffers"): {
                    vbo_desc = (neko_render_bind_vertex_buffer_desc_t*)malloc(n * sizeof(neko_render_bind_vertex_buffer_desc_t));
                    memset(vbo_desc, 0, n * sizeof(neko_render_bind_vertex_buffer_desc_t));

                    binds.vertex_buffers.desc = vbo_desc;
                    binds.vertex_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_vertex_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_lua_auto_struct_to_member(L, neko_vbo_t, id, &vbo_desc[i - 1].buffer, -1);
                        } else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("index_buffers"): {
                    ibo_desc = (neko_render_bind_index_buffer_desc_t*)malloc(n * sizeof(neko_render_bind_index_buffer_desc_t));
                    memset(ibo_desc, 0, n * sizeof(neko_render_bind_index_buffer_desc_t));

                    binds.index_buffers.desc = ibo_desc;
                    binds.index_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_index_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_lua_auto_struct_to_member(L, neko_ibo_t, id, &ibo_desc[i - 1].buffer, -1);
                        } else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                default:
                    break;
            }
        }
        lua_pop(L, 1);  // # -1
    }

    neko_render_apply_bindings(&CL_GAME_USERDATA()->cb, &binds);

    if (u_desc) free(u_desc);
    if (ib_desc) free(ib_desc);
    if (sb_desc) free(sb_desc);
    if (vbo_desc) free(vbo_desc);
    if (ibo_desc) free(ibo_desc);

    return 0;
}

static int test_tex(lua_State* L) {

#define ROW_COL_CT 10

    neko_color_t c0 = NEKO_COLOR_WHITE;
    neko_color_t c1 = neko_color(20, 50, 150, 255);
    // neko_color_t pixels[ROW_COL_CT * ROW_COL_CT] = NEKO_DEFAULT_VAL();

    neko_color_t* pixels = (neko_color_t*)lua_newuserdata(L, sizeof(neko_color_t) * ROW_COL_CT * ROW_COL_CT);

    for (u32 r = 0; r < ROW_COL_CT; ++r) {
        for (u32 c = 0; c < ROW_COL_CT; ++c) {
            const bool re = (r % 2) == 0;
            const bool ce = (c % 2) == 0;
            u32 idx = r * ROW_COL_CT + c;
            pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
        }
    }

    return 1;
}

static int __neko_bind_render_dispatch_compute(lua_State* L) {
    f32 x_groups = lua_tonumber(L, 1);
    f32 y_groups = lua_tonumber(L, 2);
    f32 z_groups = lua_tonumber(L, 3);
    neko_render_dispatch_compute(&CL_GAME_USERDATA()->cb, x_groups, y_groups, z_groups);
    return 0;
}

static int __neko_bind_render_texture_create(lua_State* L) {
    u32 w = lua_tointeger(L, 1);
    u32 h = lua_tointeger(L, 2);
    neko_render_texture_desc_t texture_desc = {
            .width = w,                             // 纹理的宽度
            .height = h,                            // 纹理的高度
            .format = R_TEXTURE_FORMAT_RGBA8,       // 纹理数据的格式
            .wrap_s = R_TEXTURE_WRAP_REPEAT,        // 纹理 s 轴的包裹类型
            .wrap_t = R_TEXTURE_WRAP_REPEAT,        // 纹理 t 轴的包裹类型
            .min_filter = R_TEXTURE_FILTER_LINEAR,  // 纹理缩小过滤器
            .mag_filter = R_TEXTURE_FILTER_LINEAR   // 纹理放大滤镜
    };
    if (lua_gettop(L) == 3) {
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            lua_pushstring(L, "type");  // # -1
            lua_gettable(L, -2);        // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_lua_auto_to(L, neko_render_texture_type, &texture_desc.type, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "format");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_lua_auto_to(L, neko_render_texture_format_type, &texture_desc.format, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_s");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_lua_auto_to(L, neko_render_texture_wrapping_type, &texture_desc.wrap_s, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_t");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_lua_auto_to(L, neko_render_texture_wrapping_type, &texture_desc.wrap_t, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "min_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_lua_auto_to(L, neko_render_texture_filtering_type, &texture_desc.min_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "mag_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_lua_auto_to(L, neko_render_texture_filtering_type, &texture_desc.mag_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "data");  // # -1
            lua_gettable(L, -2);        // pop # -1
            if (!lua_isnil(L, -1)) {
                void* data = lua_touserdata(L, -1);
                texture_desc.data[0] = data;
            }
            lua_pop(L, 1);  // # -1
        }
    }
    neko_texture_t texture = NEKO_DEFAULT_VAL();
    texture = neko_render_texture_create(texture_desc);
    neko_lua_auto_struct_push_member(L, neko_texture_t, id, &texture);
    return 1;
}

static int __neko_bind_render_texture_destroy(lua_State* L) {
    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_texture_t, id, &rt, 1);
    neko_render_texture_destroy(rt);
    return 0;
}

static int __neko_bind_render_renderpass_create(lua_State* L) {

    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);

    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_texture_t, id, &rt, 2);

    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    rp = neko_render_renderpass_create(neko_render_renderpass_desc_t{
            .fbo = fbo,               // Frame buffer to bind for render pass
            .color = &rt,             // Color buffer array to bind to frame buffer
            .color_size = sizeof(rt)  // Size of color attachment array in bytes
    });
    neko_lua_auto_struct_push_member(L, neko_renderpass_t, id, &rp);
    return 1;
}

static int __neko_bind_render_renderpass_destroy(lua_State* L) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_render_renderpass_destroy(rp);
    return 0;
}

static int __neko_bind_render_renderpass_begin(lua_State* L) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_lua_auto_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_render_renderpass_begin(&CL_GAME_USERDATA()->cb, rp);
    return 0;
}

static int __neko_bind_render_renderpass_end(lua_State* L) {
    neko_render_renderpass_end(&CL_GAME_USERDATA()->cb);
    return 0;
}

static int __neko_bind_render_draw(lua_State* L) {
    neko_render_draw_desc_t draw_desc = NEKO_DEFAULT_VAL();

    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "start");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1))
            draw_desc.start = lua_tointeger(L, -1);
        else
            NEKO_ASSERT(false);
        lua_pop(L, 1);

        lua_pushstring(L, "count");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1))
            draw_desc.count = lua_tointeger(L, -1);
        else
            NEKO_ASSERT(false);
        lua_pop(L, 1);

        lua_pushstring(L, "instances");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1)) draw_desc.instances = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_pushstring(L, "base_vertex");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1)) draw_desc.base_vertex = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }

    neko_render_draw(&CL_GAME_USERDATA()->cb, &draw_desc);
    return 0;
}

static int __neko_bind_render_set_viewport(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 w = lua_tonumber(L, 3);
    f32 h = lua_tonumber(L, 4);
    neko_render_set_viewport(&CL_GAME_USERDATA()->cb, x, y, w, h);
    return 0;
}

static int __neko_bind_render_clear(lua_State* L) {
    f32 r = lua_tonumber(L, 1);
    f32 g = lua_tonumber(L, 2);
    f32 b = lua_tonumber(L, 3);
    f32 a = lua_tonumber(L, 4);
    neko_render_clear_action_t clear = {.color = {r, g, b, a}};
    neko_render_clear(&CL_GAME_USERDATA()->cb, clear);
    return 0;
}

static int l_base64_encode(lua_State* L) {
    const char* input = luaL_checkstring(L, 1);
    const char* encoded = neko_base64_encode(input);
    if (encoded) {
        lua_pushstring(L, (const char*)encoded);
        neko_safe_free(encoded);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int l_base64_decode(lua_State* L) {
    const char* input = luaL_checkstring(L, 1);
    const char* decoded = neko_base64_decode(input);
    if (decoded) {
        lua_pushstring(L, (const char*)decoded);
        neko_safe_free(decoded);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

NEKO_INLINE void neko_register_test(lua_State* L) {

    lua_register(L, "__neko_luainspector_init", neko::luainspector::luainspector_init);
    lua_register(L, "__neko_luainspector_draw", neko::luainspector::luainspector_draw);
    lua_register(L, "__neko_luainspector_get", neko::luainspector::luainspector_get);

    // neko::lua_bind::bind("neko_tiled_get_objects", &__neko_bind_tiled_get_objects);

    neko_lua_auto_enum(L, neko_projection_type);
    neko_lua_auto_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_ORTHOGRAPHIC);
    neko_lua_auto_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_PERSPECTIVE);

    neko_lua_auto_enum(L, neko_render_primitive_type);
    neko_lua_auto_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_LINES);
    neko_lua_auto_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_TRIANGLES);
    neko_lua_auto_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_QUADS);

    neko_lua_auto_struct(L, neko_vec2);
    neko_lua_auto_struct_member(L, neko_vec2, x, f32);
    neko_lua_auto_struct_member(L, neko_vec2, y, f32);
    neko_lua_auto_struct_member(L, neko_vec2, xy, f32[2]);

    neko_lua_auto_struct(L, neko_vec3);
    neko_lua_auto_struct_member(L, neko_vec3, x, f32);
    neko_lua_auto_struct_member(L, neko_vec3, y, f32);
    neko_lua_auto_struct_member(L, neko_vec3, z, f32);
    neko_lua_auto_struct_member(L, neko_vec3, xyz, f32[3]);

    neko_lua_auto_struct(L, neko_quat);
    neko_lua_auto_struct_member(L, neko_quat, x, f32);
    neko_lua_auto_struct_member(L, neko_quat, y, f32);
    neko_lua_auto_struct_member(L, neko_quat, z, f32);
    neko_lua_auto_struct_member(L, neko_quat, w, f32);

    neko_lua_auto_struct(L, neko_vqs);
    neko_lua_auto_struct_member(L, neko_vqs, position, neko_vec3);
    neko_lua_auto_struct_member(L, neko_vqs, translation, neko_vec3);
    neko_lua_auto_struct_member(L, neko_vqs, rotation, neko_quat);
    neko_lua_auto_struct_member(L, neko_vqs, scale, neko_vec3);

    neko_lua_auto_struct(L, neko_camera_t);
    neko_lua_auto_struct_member(L, neko_camera_t, transform, neko_vqs);
    neko_lua_auto_struct_member(L, neko_camera_t, fov, f32);
    neko_lua_auto_struct_member(L, neko_camera_t, aspect_ratio, f32);
    neko_lua_auto_struct_member(L, neko_camera_t, near_plane, f32);
    neko_lua_auto_struct_member(L, neko_camera_t, far_plane, f32);
    neko_lua_auto_struct_member(L, neko_camera_t, ortho_scale, f32);
    neko_lua_auto_struct_member(L, neko_camera_t, proj_type, neko_projection_type);

    neko_lua_auto_struct(L, neko_framebuffer_t);
    neko_lua_auto_struct_member(L, neko_framebuffer_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_texture_t);
    neko_lua_auto_struct_member(L, neko_texture_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_pipeline_t);
    neko_lua_auto_struct_member(L, neko_pipeline_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_uniform_t);
    neko_lua_auto_struct_member(L, neko_uniform_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_storage_buffer_t);
    neko_lua_auto_struct_member(L, neko_storage_buffer_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_shader_t);
    neko_lua_auto_struct_member(L, neko_shader_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_vbo_t);
    neko_lua_auto_struct_member(L, neko_vbo_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_ibo_t);
    neko_lua_auto_struct_member(L, neko_ibo_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_renderpass_t);
    neko_lua_auto_struct_member(L, neko_renderpass_t, id, unsigned int);

    // neko_lua_auto_struct(L, neko_sound_playing_sound_t);
    // neko_lua_auto_struct_member(L, neko_sound_playing_sound_t, id, unsigned long long);

    neko_lua_auto_enum(L, neko_render_vertex_attribute_type);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT4);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT3);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT2);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT4);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT3);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT2);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE4);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE3);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE2);
    neko_lua_auto_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE);

    neko_lua_auto_enum(L, neko_render_texture_type);
    neko_lua_auto_enum_value(L, neko_render_texture_type, R_TEXTURE_2D);
    neko_lua_auto_enum_value(L, neko_render_texture_type, R_TEXTURE_CUBEMAP);

    neko_lua_auto_enum(L, neko_render_shader_stage_type);
    neko_lua_auto_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_VERTEX);
    neko_lua_auto_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_FRAGMENT);
    neko_lua_auto_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_COMPUTE);

    neko_lua_auto_enum(L, neko_render_texture_wrapping_type);
    neko_lua_auto_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_REPEAT);
    neko_lua_auto_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_MIRRORED_REPEAT);
    neko_lua_auto_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_CLAMP_TO_EDGE);
    neko_lua_auto_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_CLAMP_TO_BORDER);

    neko_lua_auto_enum(L, neko_render_texture_filtering_type);
    neko_lua_auto_enum_value(L, neko_render_texture_filtering_type, R_TEXTURE_FILTER_NEAREST);
    neko_lua_auto_enum_value(L, neko_render_texture_filtering_type, R_TEXTURE_FILTER_LINEAR);

    neko_lua_auto_enum(L, neko_render_texture_format_type);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGB8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RG8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R32);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R32F);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA16F);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA32F);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_A8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH16);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH24);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH32F);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH24_STENCIL8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH32F_STENCIL8);
    neko_lua_auto_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_STENCIL8);

    neko_lua_auto_enum(L, neko_render_uniform_type);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_FLOAT);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_INT);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC2);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC3);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC4);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_MAT4);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_SAMPLER2D);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_USAMPLER2D);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_SAMPLERCUBE);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_IMAGE2D_RGBA32F);
    neko_lua_auto_enum_value(L, neko_render_uniform_type, R_UNIFORM_BLOCK);
}

static int __neko_bind_cvar(lua_State* L) {
    const_str name = lua_tostring(L, 1);

    int args = lua_gettop(L);

    // 检查是否已经存在
    neko_cvar_t* cv = __neko_config_get(name);
    if (NULL != cv) {
        if (args == 2) {
            neko_cvar_set(cv, neko::neko_lua_to<const_str>(L, 2));
            return 0;
        } else if (args == 1) {  // 读取
            switch (cv->type) {
                case __NEKO_CONFIG_TYPE_INT:
                    lua_pushinteger(L, cv->value.i);
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    lua_pushnumber(L, cv->value.f);
                    break;
                case __NEKO_CONFIG_TYPE_STRING:
                    lua_pushstring(L, cv->value.s);
                    break;
                case __NEKO_CONFIG_TYPE_COUNT:
                default:
                    break;
            }
            return 1;
        } else {
            const_str error_message = "__neko_bind_cvar failed";
            lua_pushstring(L, error_message);  // 将错误信息压入堆栈
            return lua_error(L);               // 抛出lua错误
        }
    } else {
        if (args == 1) {
            lua_pushnil(L);
            return 1;            // 如果不存在就返回nil
        } else if (args == 3) {  // 创建

            neko_cvar_type cval = neko_cvar_type::__NEKO_CONFIG_TYPE_COUNT;

            if (lua_type(L, 2) == LUA_TSTRING) {
                const_str type = lua_tostring(L, 2);
                lua_pushstring(CL_GAME_USERDATA()->L, type);
                neko_lua_auto_to(CL_GAME_USERDATA()->L, neko_cvar_type, &cval, -1);
                lua_pop(CL_GAME_USERDATA()->L, 1);
            } else if (lua_isinteger(L, 2)) {
                int type = lua_tointeger(L, 2);
                cval = (neko_cvar_type)type;
            }

            switch (cval) {
                case __NEKO_CONFIG_TYPE_INT:
                    neko_cvar_lnew(name, cval, neko::neko_lua_to<int>(L, 3));
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    neko_cvar_lnew(name, cval, neko::neko_lua_to<float>(L, 3));
                    break;
                case __NEKO_CONFIG_TYPE_STRING:
                    neko_cvar_lnew_str(name, cval, neko::neko_lua_to<const_str>(L, 3));
                    break;
                case __NEKO_CONFIG_TYPE_COUNT:
                default:
                    neko_log_warning(std::format("__neko_bind_cvar_new with a unknown type {0} {1}", name, (u8)cval).c_str());
                    break;
            }

            return 0;
        } else {
            const_str error_message = "__neko_bind_cvar failed";
            lua_pushstring(L, error_message);  // 将错误信息压入堆栈
            return lua_error(L);               // 抛出lua错误
        }
    }

    return 0;  // unreachable
}

static int __neko_bind_print(lua_State* L) {
    std::string str;
    int n = lua_gettop(L);
    int i;
    for (i = 1; i <= n; i++) {
        size_t l;
        const_str s = luaL_tolstring(L, i, &l);
        if (i > 1) str.append("\t");
        str.append(std::string(s, l));
        lua_pop(L, 1);
    }
    neko_log_info("[lua] %s", str.c_str());
    return 0;
}

void __neko_lua_bug(const_str message) { neko_log_info("[lua] %s", message); }
void __neko_lua_info(const_str message) { neko_log_info("[lua] %s", message); }
void __neko_lua_trace(const_str message) { neko_log_info("[lua] %s", message); }
void __neko_lua_error(const_str message) { neko_log_error("[lua] %s", message); }
void __neko_lua_warn(const_str message) { neko_log_warning("[lua] %s", message); }

// 返回包含路径和 isDirectory 对的表
int __neko_ls(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        neko_log_warning("invalid lua argument");
        return 0;
    }
    auto string = lua_tostring(L, 1);
    if (!std::filesystem::is_directory(string)) {
        neko_log_warning(std::format("{0} is not directory", string).c_str());
        return 0;
    }

    lua_newtable(L);
    int i = 0;
    for (auto& p : std::filesystem::directory_iterator(string)) {
        lua_pushnumber(L, i + 1);  // parent table index
        lua_newtable(L);
        lua_pushstring(L, "path");
        lua_pushstring(L, p.path().generic_string().c_str());
        lua_settable(L, -3);
        lua_pushstring(L, "isDirectory");
        lua_pushboolean(L, p.is_directory());
        lua_settable(L, -3);
        lua_settable(L, -3);
        i++;
    }
    return 1;
}

bool __neko_dolua(const_str file) { return neko::neko_lua_dofile(CL_GAME_USERDATA()->L, game_assets(file)); }

struct neko_lua_hook_pool g_lua_hook_pool = {NULL, 0};

struct neko_lua_hook_t think = {"think", NULL, 0, &think, NULL, neko_lua_hook_status::hook_update};

struct neko_lua_hook_callbacks* neko_lua_hook_callback_create(size_t dataSize, enum neko_lua_dataType dataType) {
    struct neko_lua_hook_callbacks* callback = (struct neko_lua_hook_callbacks*)malloc(sizeof(struct neko_lua_hook_callbacks));

    if (callback) {
        callback->data_size = dataSize;
        callback->data = malloc(dataSize);
        callback->data_type = dataType;
    } else {
        neko_log_warning("[lua] Callback \"?\" errored with: Memory allocation error");
    }

    return callback;
}

void neko_lua_hook_callback_set(struct neko_lua_hook_callbacks* callback, const void* data) { memcpy(callback->data, data, callback->data_size); }

void* neko_lua_hook_callback_get(const struct neko_lua_hook_callbacks* callback) { return callback->data; }

void neko_lua_hook_register(struct neko_lua_hook_t hookData) {
    struct neko_lua_hook_t* temp = (struct neko_lua_hook_t*)realloc(g_lua_hook_pool.hooks, (g_lua_hook_pool.count + 1) * sizeof(struct neko_lua_hook_t));

    if (temp) {
        g_lua_hook_pool.hooks = temp;
        g_lua_hook_pool.hooks[g_lua_hook_pool.count] = hookData;
        g_lua_hook_pool.count += 1;
    } else {
        neko_log_warning("[lua] Hook \"pool\" errored with: Memory allocation error");
    }
}

void neko_lua_hook_add(struct neko_lua_hook_t* instance, const_str name, void (*func)(lua_State*, struct neko_lua_hook_t* instance, int, struct neko_lua_hook_callbacks* callback), int ref) {
    for (size_t i = 0; i < instance->pool; ++i) {
        if (strcmp(instance->stack[i].name, name) == 0) {
            instance->stack[i].func = func;
            instance->stack[i].ref = ref;

            return;
        }
    }

    struct neko_lua_hook_stack* temp = (struct neko_lua_hook_stack*)realloc(instance->stack, (instance->pool + 1) * sizeof(struct neko_lua_hook_stack));

    if (temp) {
        instance->stack = temp;
        instance->stack[instance->pool].name = strdup(name);
        instance->stack[instance->pool].func = func;
        instance->stack[instance->pool].ref = ref;
        instance->pool += 1;
    } else {
        neko_log_warning("[lua] Hook \"%s\" errored with: Memory allocation error", instance->hookName);
    }
}

void neko_lua_hook_remove(struct neko_lua_hook_t* instance, const_str name) {
    for (size_t i = 0; i < instance->pool; ++i) {
        if (strcmp(instance->stack[i].name, name) == 0) {
            for (size_t j = i; j < instance->pool - 1; ++j) {
                instance->stack[j] = instance->stack[j + 1];
            }

            struct neko_lua_hook_stack* temp = (struct neko_lua_hook_stack*)malloc((instance->pool - 1) * sizeof(struct neko_lua_hook_stack));

            if (temp) {
                memcpy(temp, instance->stack, i * sizeof(struct neko_lua_hook_stack));
                memcpy(temp + i, instance->stack + i + 1, (instance->pool - i - 1) * sizeof(struct neko_lua_hook_stack));

                free(instance->stack);

                instance->stack = temp;
                instance->pool -= 1;
            } else {
                neko_log_warning("[lua] Hook \"%s\" errored with: Memory allocation error", instance->hookName);
            }

            return;
        }
    }

    neko_snprintfc(temp, 64, "'%s' not found", name);
    neko_log_warning("[lua] Hook \"%s\" errored with: %s", instance->hookName, temp);
}

void neko_lua_hook_run(struct neko_lua_hook_t* instance, lua_State* L) {
    if (!instance || !L) {
        neko_log_warning("[lua] Hook \"?\" errored with: Failed to get neko_lua_hook_t instance");

        return;
    }

    if (instance->handle) {
        instance->handle(instance, L);
    }

    for (size_t i = 0; i < instance->pool; ++i) {
        if (instance->stack[i].func) {
            if (instance->status != neko_lua_hook_status::hook_idle) {
                instance->stack[i].func(L, instance, i, instance->callback);
            }
        } else {
            neko_log_warning("[lua] Hook \"%s\" errored with: Could not find function reference", instance->hookName);
        }
    }

    if (instance->status == neko_lua_hook_status::hook_awaiting) {
        instance->status = neko_lua_hook_status::hook_idle;
    }
}

void neko_lua_hook_free(struct neko_lua_hook_t* instance, lua_State* L) {
    for (size_t i = 0; i < instance->pool; ++i) {
        if (instance->stack[i].ref != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, instance->stack[i].ref);
        }
    }

    instance->stack = NULL;
    instance->pool = 0;

    free(instance->stack);
}

void neko_lua_hook_luafunc(lua_State* L, struct neko_lua_hook_t* instance, int index, struct neko_lua_hook_callbacks* callback) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance->stack[index].ref);

    if (callback && callback->data) {
        for (size_t i = 0; i < callback->data_size; ++i) {
            void* value = ((char*)callback->data) + (i * callback->data_size);

            switch (callback->data_type) {
                case neko_lua_dataType::number:
                    lua_pushnumber(L, *(double*)value);
                    break;
                case neko_lua_dataType::string:
                    lua_pushstring(L, (const_str)value);
                    break;
                case neko_lua_dataType::integer:
                    lua_pushinteger(L, *(int*)value);
                    break;
                case neko_lua_dataType::lua_bool:
                    lua_pushboolean(L, *(int*)value);
                    break;
                case neko_lua_dataType::function:
                    lua_rawgeti(L, LUA_REGISTRYINDEX, *((int*)value));
                    break;
                default:
                    lua_pushnil(L);
                    break;
            }
        }
    }

    if (lua_pcall(L, callback ? callback->data_size : 0, LUA_MULTRET, 0) != LUA_OK) {
        neko_log_warning("[lua] Hook \"%s\" errored with: %s", instance->hookName, lua_tostring(L, -1));

        lua_pop(L, 1);

        return;
    }
}

struct neko_lua_hook_t* neko_lua_hook_find(const_str hookName) {
    for (size_t i = 0; i < g_lua_hook_pool.count; ++i) {
        if (strcmp(g_lua_hook_pool.hooks[i].hookName, hookName) == 0) {
            return &g_lua_hook_pool.hooks[i];
        }
    }

    return NULL;
}

int neko_lua_hook_bind_add(lua_State* L) {
    const_str hookName = luaL_checkstring(L, 1);
    const_str name = luaL_checkstring(L, 2);

    struct neko_lua_hook_t* instance = neko_lua_hook_find(hookName);

    if (instance) {
        if (lua_isfunction(L, 3)) {
            lua_pushvalue(L, 3);

            int ref = luaL_ref(L, LUA_REGISTRYINDEX);

            neko_lua_hook_add(instance->address, name, neko_lua_hook_luafunc, ref);
        } else {
            neko_log_warning("[lua] Hook \"%s\" errored with: Third argument must be a function", instance->hookName);
        }
    } else {
        neko_log_warning("[lua] Hook \"%s\" errored with: Not found", hookName);
    }

    return 0;
}

int neko_lua_hook_bind_remove(lua_State* L) {
    const_str hookName = luaL_checkstring(L, 1);
    const_str name = luaL_checkstring(L, 2);

    struct neko_lua_hook_t* instance = neko_lua_hook_find(hookName);

    if (instance) {
        neko_lua_hook_remove(instance->address, name);
    } else {
        neko_log_warning("[lua] Hook \"%s\" errored with: Not found", hookName);
    }

    return 0;
}

int neko_lua_hook_bind_run(lua_State* L) {
    for (size_t i = 0; i < g_lua_hook_pool.count; ++i) {
        neko_lua_hook_run(g_lua_hook_pool.hooks[i].address, L);
    }

    return 0;
}

int neko_lua_hook_bind_free(lua_State* L) {
    const_str hookName = luaL_checkstring(L, 1);

    struct neko_lua_hook_t* instance = neko_lua_hook_find(hookName);

    if (instance) {
        neko_lua_hook_free(instance->address, L);
    } else {
        neko_log_warning("[lua] Hook \"%s\" errored with: Not found", hookName);
    }

    return 0;
}

void renderHandle(struct neko_lua_hook_t* instance, lua_State* L) {

    //     instance->status = hook_idle;

    int v = 10;

    instance->callback = neko_lua_hook_callback_create(sizeof(int), neko_lua_dataType::integer);
    neko_lua_hook_callback_set(instance->callback, &v);

    instance->status = neko_lua_hook_status::hook_update;
}

struct neko_lua_hook_t render = {"render", NULL, 0, &render, renderHandle, neko_lua_hook_status::hook_update};

NEKO_INLINE void neko_register_common(lua_State* L) {

    neko::lua_bind::bind("log_trace", &__neko_lua_trace);
    neko::lua_bind::bind("log_debug", &__neko_lua_bug);
    neko::lua_bind::bind("log_error", &__neko_lua_error);
    neko::lua_bind::bind("log_warn", &__neko_lua_warn);
    neko::lua_bind::bind("log_info", &__neko_lua_info);
    neko::lua_bind::bind("neko_rand", &neko_rand_xorshf32);
    neko::lua_bind::bind("neko_dolua", &__neko_dolua);

    neko::lua_bind::bind("neko_hash_str", +[](const_str str) { return neko_hash_str(str); });
    neko::lua_bind::bind("neko_hash_str64", +[](const_str str) { return neko_hash_str64(str); });
    neko::lua_bind::bind("__neko_quit", +[](int op) { return neko_quit(); });
    neko::lua_bind::bind("neko_platform_elapsed_time", +[]() { return neko_platform_elapsed_time(); });

    lua_pushstring(L, game_assets("gamedir").c_str());
    lua_setglobal(L, "neko_game_data_path");

    // const neko_luaL_reg luaReg[] = {{"hooks", neko_lua_hook_init}, {NULL, NULL}};
    // registerGlobals(L, luaReg);

    // const luaL_Reg luaCommon[] = {
    //         {"Vector", Vector},  //
    //         {"Color", Color},    //
    //         {NULL, NULL}         //
    // };

    // for (const luaL_Reg* reg = luaCommon; reg->name != NULL && reg->func != NULL; ++reg) {
    //     lua_pushcfunction(L, reg->func);
    //     lua_setglobal(L, reg->name);
    // }

    neko_lua_auto_enum(L, neko_cvar_type);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_INT);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_FLOAT);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_STRING);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_COUNT);

    lua_register(L, "__neko_ls", __neko_ls);
}

int register_mt_hooks(lua_State* L) {
    luaL_Reg reg[] = {{"add", neko_lua_hook_bind_add}, {"remove", neko_lua_hook_bind_remove}, {"run", neko_lua_hook_bind_run}, {"free", neko_lua_hook_bind_free}, {NULL, NULL}};
    luaL_newmetatable(L, "mt_hooks");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);            // # -1 复制一份 为了让 neko 主表设定
    lua_setfield(L, -2, "__index");  // # -2
    return 1;
}

int register_mt_aseprite_renderer(lua_State* L) {
    luaL_Reg reg[] = {
            {"__gc", __neko_bind_aseprite_render_gc},
            {"create", __neko_bind_aseprite_render_create},
            {"update", __neko_bind_aseprite_render_update},
            {"render", __neko_bind_aseprite_render},
            {"update_animation", __neko_bind_aseprite_render_update_animation},
            {NULL, NULL},
    };
    luaL_newmetatable(L, "mt_aseprite_renderer");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);            // # -1 复制一份 为了让 neko 主表设定
    lua_setfield(L, -2, "__index");  // # -2
    return 1;
}

int register_mt_aseprite(lua_State* L) {
    luaL_Reg reg[] = {
            {"__gc", __neko_bind_aseprite_gc},
            {"create", __neko_bind_aseprite_create},
            {NULL, NULL},
    };
    luaL_newmetatable(L, "mt_aseprite");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);            // # -1 复制一份 为了让 neko 主表设定
    lua_setfield(L, -2, "__index");  // # -2
    return 1;
}

typedef struct neko_lua_profiler_data {
    int linedefined;
    char source[LUA_IDSIZE];
} neko_lua_profiler_data;

typedef struct neko_lua_profiler_count {
    u32 total;
    u32 index;
    // u64 t;
} neko_lua_profiler_count;

static void neko_lua_profiler_hook(lua_State* L, lua_Debug* ar) {
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, L) != LUA_TUSERDATA) {
        lua_pop(L, 1);
        return;
    }
    neko_lua_profiler_count* p = (neko_lua_profiler_count*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    neko_lua_profiler_data* log = (neko_lua_profiler_data*)(p + 1);
    int index = p->index++;
    while (index >= p->total) {
        index -= p->total;
    }
    if (lua_getinfo(L, "S", ar) != 0) {
        log[index].linedefined = ar->linedefined;
        strcpy(log[index].source, ar->short_src);
    } else {
        log[index].linedefined = 1;
        strcpy(log[index].source, "[unknown]");
    }
}

static int neko_lua_profiler_start(lua_State* L) {
    lua_State* cL = L;  // 支持多线程
    int args = 0;
    if (lua_isthread(L, 1)) {
        cL = lua_tothread(L, 1);
        args = 1;
    }
    int c = luaL_optinteger(L, args + 1, 1000);    // 记录数量
    int ival = luaL_optinteger(L, args + 2, 100);  // 检测间隔
    neko_lua_profiler_count* p = (neko_lua_profiler_count*)lua_newuserdata(L, sizeof(neko_lua_profiler_count) + c * sizeof(neko_lua_profiler_data));
    p->total = c;
    p->index = 0;
    // p->t = neko_platform_elapsed_time();
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, cL);
    lua_sethook(cL, neko_lua_profiler_hook, LUA_MASKCOUNT, ival);
    return 0;
}

static int neko_lua_profiler_stop(lua_State* L) {
    lua_State* cL = L;  // 支持多线程
    if (lua_isthread(L, 1)) {
        cL = lua_tothread(L, 1);
    }
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, cL) != LUA_TNIL) {
        lua_pushnil(L);
        lua_rawsetp(L, LUA_REGISTRYINDEX, cL);
        lua_sethook(cL, NULL, 0, 0);
    } else {
        return luaL_error(L, "thread profiler not begin");
    }
    return 0;
}

static int neko_lua_profiler_info(lua_State* L) {
    lua_State* cL = L;
    if (lua_isthread(L, 1)) {
        cL = lua_tothread(L, 1);
    }
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, cL) != LUA_TUSERDATA) {
        return luaL_error(L, "thread profiler not begin");
    }
    neko_lua_profiler_count* p = (neko_lua_profiler_count*)lua_touserdata(L, -1);
    neko_lua_profiler_data* log = (neko_lua_profiler_data*)(p + 1);
    lua_newtable(L);
    int n = (p->index > p->total) ? p->total : p->index;
    int i;
    for (i = 0; i < n; i++) {
        luaL_getsubtable(L, -1, log[i].source);
        lua_rawgeti(L, -1, log[i].linedefined);
        int c = lua_tointeger(L, -1);
        lua_pushinteger(L, c + 1);
        // subtbl, c, c + 1
        lua_rawseti(L, -3, log[i].linedefined);
        lua_pop(L, 2);
    }
    lua_pushinteger(L, p->index);
    // lua_pushinteger(L, neko_platform_elapsed_time() - p->t);
    return 2;
}

static int __neko_bind_ecs_f(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");
    return 1;
}

static int open_embed_core(lua_State* L) {

    luaL_checkversion(L);

    luaL_Reg reg[] = {

            {"ecs_f", __neko_bind_ecs_f},

            {"tiled_create", __neko_bind_tiled_create},
            {"tiled_render", __neko_bind_tiled_render},
            {"tiled_end", __neko_bind_tiled_end},
            {"tiled_load", __neko_bind_tiled_load},
            {"tiled_unload", __neko_bind_tiled_unload},

            // {"pixelui_create", __neko_bind_pixelui_create},
            // {"pixelui_update", __neko_bind_pixelui_update},
            // {"pixelui_end", __neko_bind_pixelui_end},
            // {"pixelui_tex", __neko_bind_pixelui_tex},

            {"gfxt_create", __neko_bind_gfxt_create},
            {"gfxt_update", __neko_bind_gfxt_update},
            {"gfxt_end", __neko_bind_gfxt_end},

            {"draw_text", __neko_bind_draw_text},

            {"sprite_batch_create", __neko_bind_sprite_batch_create},
            {"sprite_batch_render_ortho", __neko_bind_sprite_batch_render_ortho},
            {"sprite_batch_render_begin", __neko_bind_sprite_batch_render_begin},
            {"sprite_batch_render_end", __neko_bind_sprite_batch_render_end},
            {"sprite_batch_make_sprite", __neko_bind_sprite_batch_make_sprite},
            {"sprite_batch_push_sprite", __neko_bind_sprite_batch_push_sprite},
            {"sprite_batch_end", __neko_bind_sprite_batch_end},

            {"gameobject_inspect", __neko_bind_gameobject_inspect},

            // {"filesystem_create", __neko_bind_filesystem_create},
            // {"filesystem_destory", __neko_bind_filesystem_destory},
            // {"filewatch_create", __neko_bind_filewatch_create},
            // {"filewatch_destory", __neko_bind_filewatch_destory},
            // {"filewatch_mount", __neko_bind_filewatch_mount},
            // {"filewatch_start", __neko_bind_filewatch_start_watch},
            // {"filewatch_stop", __neko_bind_filewatch_stop_watching},
            // {"filewatch_update", __neko_bind_filewatch_update},
            // {"filewatch_notify", __neko_bind_filewatch_notify},

            {"callback_save", __neko_bind_callback_save},
            {"callback_call", __neko_bind_callback_call},

            {"idraw_get", __neko_bind_idraw_get},
            {"idraw_draw", __neko_bind_idraw_draw},
            {"idraw_defaults", __neko_bind_idraw_defaults},
            {"idraw_rectv", __neko_bind_idraw_rectv},
            {"idraw_rectvd", __neko_bind_idraw_rectvd},
            {"idraw_text", __neko_bind_idraw_text},
            {"idraw_camera", __neko_bind_idraw_camera},
            {"idraw_camera2d", __neko_bind_idraw_camera2d},
            {"idraw_camera2d_ex", __neko_bind_idraw_camera2d_ex},
            {"idraw_camera3d", __neko_bind_idraw_camera3d},
            {"idraw_rotatev", __neko_bind_idraw_rotatev},
            {"idraw_box", __neko_bind_idraw_box},
            {"idraw_translatef", __neko_bind_idraw_translatef},
            {"idraw_depth_enabled", __neko_bind_idraw_depth_enabled},
            {"idraw_face_cull_enabled", __neko_bind_idraw_face_cull_enabled},
            {"idraw_texture", __neko_bind_idraw_texture},

            {"render_framebuffer_create", __neko_bind_render_framebuffer_create},
            {"render_framebuffer_destroy", __neko_bind_render_framebuffer_destroy},
            {"render_texture_create", __neko_bind_render_texture_create},
            {"render_texture_destroy", __neko_bind_render_texture_destroy},
            {"render_renderpass_create", __neko_bind_render_renderpass_create},
            {"render_renderpass_destroy", __neko_bind_render_renderpass_destroy},
            {"render_renderpass_begin", __neko_bind_render_renderpass_begin},
            {"render_renderpass_end", __neko_bind_render_renderpass_end},
            {"render_set_viewport", __neko_bind_render_set_viewport},
            {"render_clear", __neko_bind_render_clear},
            {"render_shader_create", __neko_bind_render_shader_create},
            {"render_uniform_create", __neko_bind_render_uniform_create},
            {"render_pipeline_create", __neko_bind_render_pipeline_create},
            {"render_storage_buffer_create", __neko_bind_render_storage_buffer_create},
            {"render_vertex_buffer_create", __neko_bind_render_vertex_buffer_create},
            {"render_index_buffer_create", __neko_bind_render_index_buffer_create},
            {"render_vertex_attribute_create", __neko_bind_render_vertex_attribute_create},
            {"render_pipeline_bind", __neko_bind_render_pipeline_bind},
            {"render_apply_bindings", __neko_bind_render_apply_bindings},
            {"render_dispatch_compute", __neko_bind_render_dispatch_compute},
            {"render_draw", __neko_bind_render_draw},
            {"gen_tex", test_tex},

            {"pack_construct", __neko_bind_pack_construct},
            {"pack_destroy", __neko_bind_pack_destroy},
            {"pack_build", __neko_bind_pack_build},
            {"pack_info", __neko_bind_pack_info},
            {"pack_items", __neko_bind_pack_items},
            {"pack_assets_load", __neko_bind_pack_assets_load},
            {"pack_assets_unload", __neko_bind_pack_assets_unload},

            // {"b2_world", neko_b2_world},

            {"cvar", __neko_bind_cvar},
            {"print", __neko_bind_print},

            {"base64_encode", l_base64_encode},
            {"base64_decode", l_base64_decode},
            {"json_read", __neko_bind_json_read},
            {"json_write", __neko_bind_json_write},

            {"profiler_start", neko_lua_profiler_start},
            {"profiler_stop", neko_lua_profiler_stop},
            {"profiler_info", neko_lua_profiler_info},

            {NULL, NULL},
    };

    luaL_newlib(L, reg);

    {
        register_mt_aseprite_renderer(L);
        lua_setfield(L, -2, "aseprite_render");
    }

    {
        register_mt_aseprite(L);
        lua_setfield(L, -2, "aseprite");
    }

    // {
    //     register_mt_imgui(L);
    //     lua_setfield(L, -2, "imgui");
    // }

    {
        register_mt_hooks(L);
        lua_setfield(L, -2, "hooks");

        // 注册引擎 hooks
        neko_lua_hook_register(think);
        neko_lua_hook_register(render);
    }

    // lua_CFunction mt_funcs[] = {
    //         // open_mt_b2_fixture,
    //         // open_mt_b2_body,
    //         // open_mt_b2_world,
    //         // open_mt_sound,
    // };

    // for (u32 i = 0; i < NEKO_ARR_SIZE(mt_funcs); i++) {
    //     mt_funcs[i](L);
    // }

    return 1;
}

namespace neko::lua_core {
static int luaopen(lua_State* L) { return open_embed_core(L); }
}  // namespace neko::lua_core

DEFINE_LUAOPEN(core)