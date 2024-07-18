
#include "engine/neko_api.hpp"
#include "engine/neko_app.h"
#include "engine/neko_base.h"
#include "engine/neko_lua.h"
#include "engine/neko_lua_wrap.h"
#include "engine/neko_luabind.hpp"
#include "engine/neko_reflection.hpp"
#include "engine/neko_ui.h"

static int g_lua_callbacks_table_ref = LUA_NOREF;

LUA_FUNCTION(__neko_bind_callback_save) {
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

LUA_FUNCTION(__neko_bind_callback_call) {
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

#if 0

static bool __neko_bind_platform_key_pressed(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_pressed(cval);
}

static bool __neko_bind_platform_was_key_down(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_down(cval);
}

static bool __neko_bind_platform_key_down(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_down(cval);
}

static bool __neko_bind_platform_key_released(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_released(cval);
}

static bool __neko_bind_platform_was_mouse_down(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_was_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_pressed(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_mouse_pressed(cval);
}

static bool __neko_bind_platform_mouse_down(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_released(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_mouse_released(cval);
}

LUA_FUNCTION(__neko_bind_platform_mouse_delta) {
    neko_vec2 v2 = neko_os_mouse_deltav();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_mouse_position) {
    neko_vec2 v2 = neko_os_mouse_positionv();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_mouse_wheel) {
    neko_vec2 v2 = neko_os_mouse_wheelv();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_window_size) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_os_window_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_framebuffer_size) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_os_framebuffer_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_set_window_title) {
    u32 handle = lua_tointeger(L, -1);
    const_str title = lua_tostring(L, -2);
    neko_os_set_window_title(handle, title);
    return 0;
}

inline void neko_register_platform(lua_State* L) {

    neko_luabind_enum(L, neko_os_keycode);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_INVALID);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SPACE);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_APOSTROPHE); /* ' */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_COMMA);      /* , */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_MINUS);      /* - */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PERIOD);     /* . */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SLASH);      /* / */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_0);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_1);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_2);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_3);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_4);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_5);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_6);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_7);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_8);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_9);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SEMICOLON); /* ; */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_EQUAL);     /* = */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_A);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_B);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_C);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_D);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_E);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_G);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_H);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_I);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_J);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_K);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_L);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_M);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_N);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_O);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_P);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_Q);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_R);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_S);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_T);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_U);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_V);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_W);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_X);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_Y);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_Z);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_BRACKET);  /* [ */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_BACKSLASH);     /* \ */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_BRACKET); /* ] */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_GRAVE_ACCENT);  /* ` */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_WORLD_1);       /* non-US #1 */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_WORLD_2);       /* non-US #2 */
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_ESC);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_ENTER);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_TAB);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_BACKSPACE);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_INSERT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_DELETE);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_DOWN);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_UP);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PAGE_UP);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PAGE_DOWN);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_HOME);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_END);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_CAPS_LOCK);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SCROLL_LOCK);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_NUM_LOCK);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PRINT_SCREEN);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PAUSE);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F1);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F2);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F3);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F4);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F5);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F6);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F7);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F8);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F9);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F10);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F11);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F12);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F13);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F14);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F15);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F16);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F17);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F18);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F19);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F20);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F21);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F22);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F23);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F24);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F25);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_0);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_1);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_2);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_3);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_4);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_5);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_6);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_7);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_8);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_9);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_DECIMAL);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_DIVIDE);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_MULTIPLY);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_SUBTRACT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_ADD);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_ENTER);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_EQUAL);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_SHIFT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_CONTROL);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_ALT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_SUPER);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_SHIFT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_CONTROL);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_ALT);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_SUPER);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_MENU);
    neko_luabind_enum_value(L, neko_os_keycode, NEKO_KEYCODE_COUNT);

    neko_luabind_enum(L, neko_os_mouse_button_code);
    neko_luabind_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_LBUTTON);
    neko_luabind_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_RBUTTON);
    neko_luabind_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_MBUTTON);
    neko_luabind_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_BUTTON_CODE_COUNT);

    neko::lua_bind::bind("neko_key_pressed", &__neko_bind_platform_key_pressed);
    neko::lua_bind::bind("neko_was_key_down", &__neko_bind_platform_was_key_down);
    neko::lua_bind::bind("neko_was_mouse_down", &__neko_bind_platform_was_mouse_down);
    neko::lua_bind::bind("neko_key_down", &__neko_bind_platform_key_down);
    neko::lua_bind::bind("neko_key_released", &__neko_bind_platform_key_released);
    neko::lua_bind::bind("neko_mouse_down", &__neko_bind_platform_mouse_down);
    neko::lua_bind::bind("neko_mouse_pressed", &__neko_bind_platform_mouse_pressed);
    neko::lua_bind::bind("neko_mouse_released", &__neko_bind_platform_mouse_released);
    neko::lua_bind::bind("neko_mouse_moved", +[]() -> bool { return neko_os_mouse_moved(); });
    neko::lua_bind::bind("neko_main_window", +[]() -> u32 { return neko_os_main_window(); });
    neko::lua_bind::bind("neko_set_window_size", +[](u32 handle, u32 width, u32 height) { neko_os_set_window_size(handle, width, height); });
    neko::lua_bind::bind("neko_set_mouse_position", +[](u32 handle, f64 x, f64 y) { neko_os_mouse_set_position(handle, x, y); });

    lua_register(L, "neko_mouse_delta", __neko_bind_platform_mouse_delta);
    lua_register(L, "neko_mouse_position", __neko_bind_platform_mouse_position);
    lua_register(L, "neko_mouse_wheel", __neko_bind_platform_mouse_wheel);

    lua_register(L, "neko_window_size", __neko_bind_platform_window_size);
    lua_register(L, "neko_framebuffer_size", __neko_bind_platform_framebuffer_size);
    lua_register(L, "neko_set_window_title", __neko_bind_platform_set_window_title);
}

LUA_FUNCTION(__neko_bind_aseprite_render_create) {
    neko_aseprite* sprite_data = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)lua_newuserdata(L, sizeof(neko_aseprite_renderer));
    memset(user_handle, 0, sizeof(neko_aseprite_renderer));
    user_handle->sprite = sprite_data;
    neko_aseprite_renderer_play(user_handle, "Idle");
    luaL_setmetatable(L, "mt_aseprite_renderer");
    return 1;
}

LUA_FUNCTION(__neko_bind_aseprite_render_gc) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    // NEKO_TRACE("aseprite_render __gc %p", user_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_render_update) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    neko_instance_t* engine = neko_instance();

    neko_aseprite_renderer_update(user_handle, engine->platform->time.delta);

    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_render_update_animation) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    const_str by_tag = lua_tostring(L, 2);
    neko_aseprite_renderer_play(user_handle, by_tag);
    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_render) {
    PROFILE_FUNC();

    if (lua_gettop(L) != 4) {
        return luaL_error(L, "Function expects exactly 4 arguments");
    }

    luaL_argcheck(L, lua_gettop(L) == 4, 1, "expects exactly 4 arguments");

    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    auto xform = lua2struct::unpack<neko_vec2>(L, 2);

    int direction = neko::neko_lua_to<int>(L, 3);
    f32 scale = neko::neko_lua_to<f32>(L, 4);

    neko_instance_t* engine = neko_instance();

    i32 index;
    if (user_handle->loop) {
        index = user_handle->loop->indices[user_handle->current_frame];
    } else {
        index = user_handle->current_frame;
    }

    neko_aseprite* spr = user_handle->sprite;
    neko_aseprite_frame f = spr->frames[index];

    if (direction)
        neko_idraw_rect_textured_ext(&ENGINE_INTERFACE()->idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u1, f.v0, f.u0, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);
    else
        neko_idraw_rect_textured_ext(&ENGINE_INTERFACE()->idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u0, f.v0, f.u1, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);

    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_create) {
    const_str file_path = lua_tostring(L, 1);

    neko_aseprite* user_handle = (neko_aseprite*)lua_newuserdata(L, sizeof(neko_aseprite));
    memset(user_handle, 0, sizeof(neko_aseprite));

    neko_aseprite_load(user_handle, file_path);

    luaL_setmetatable(L, "mt_aseprite");

    return 1;
}

LUA_FUNCTION(__neko_bind_aseprite_gc) {
    neko_aseprite* user_handle = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    if (user_handle->frames != NULL) neko_aseprite_end(user_handle);
    // NEKO_TRACE("aseprite __gc %p", user_handle);
    return 0;
}



LUA_FUNCTION(__neko_bind_tiled_create) {
    const_str map_path = lua_tostring(L, 1);
    const_str glsl_vs_src = lua_tostring(L, 2);
    const_str glsl_fs_src = lua_tostring(L, 3);

    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_newuserdata(L, sizeof(neko_tiled_renderer));
    memset(user_handle, 0, sizeof(neko_tiled_renderer));

    neko_tiled_load(&(user_handle->map), map_path, NULL);

    neko_tiled_render_init(&ENGINE_INTERFACE()->cb, user_handle, glsl_vs_src, glsl_fs_src);

    return 1;
}

LUA_FUNCTION(__neko_bind_tiled_render) {

    PROFILE_FUNC();

    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)lua_touserdata(L, 1);

    neko_renderpass_t rp = R_RENDER_PASS_DEFAULT;
    neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 2);

    auto xform = lua2struct::unpack<neko_vec2>(L, 3);

    f32 l = lua_tonumber(L, 4);
    f32 r = lua_tonumber(L, 5);
    f32 t = lua_tonumber(L, 6);
    f32 b = lua_tonumber(L, 7);

    tiled_render->camera_mat = neko_mat4_ortho(l, r, b, t, -1.0f, 1.0f);

    neko_command_buffer_t* cb = &ENGINE_INTERFACE()->cb;

    neko_render_renderpass_begin(cb, rp);
    {
        neko_tiled_render_begin(cb, tiled_render);

        PROFILE_BLOCK("tiled_render");

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

LUA_FUNCTION(__neko_bind_tiled_unload) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    neko_tiled_unload(&user_handle->map);
    return 0;
}

LUA_FUNCTION(__neko_bind_tiled_load) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    const_str path = lua_tostring(L, 2);
    neko_tiled_load(&user_handle->map, path, NULL);
    return 0;
}

LUA_FUNCTION(__neko_bind_tiled_end) {
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

#endif

#if 0
LUA_FUNCTION(__neko_bind_pixelui_create){
    // const_str file_path = lua_tostring(L, 1);

    pixelui_t* user_handle = (pixelui_t*)lua_newuserdata(L, sizeof(pixelui_t));
    memset(user_handle, 0, sizeof(pixelui_t));

    // user_handle->show_frame_count = true;
    user_handle->show_material_selection_panel = true;

    pixelui_init(user_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_pixelui_update){
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    update_ui(user_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_pixelui_tex){
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    neko_luabind_struct_push_member(L, neko_texture_t, id, &user_handle->tex_ui);
    return 1;
}

LUA_FUNCTION(__neko_bind_pixelui_end){
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    pixelui_fini(user_handle);
    return 0;
}
#endif

#if 0

static void fontbatch_metatable(lua_State* L) {
    // static luaL_Reg lib[] = {{"add", add}, {"set_recursive", set_recursive}, {"set_follow_symlinks", set_follow_symlinks}, {"set_filter", set_filter}, {"select", select}, {NULL, NULL}};
    // luaL_newlibtable(L, lib);
    // luaL_setfuncs(L, lib, 0);
    // lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__gc",
                             +[](lua_State* L) {
                                 neko_fontbatch_t* fontbatch = neko::lua::toudata_ptr<neko_fontbatch_t>(L, 1);
                                 neko_fontbatch_end(fontbatch);
                                 NEKO_TRACE("fontbatch __gc %p", fontbatch);
                                 return 0;
                             }},
                            {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

namespace neko::lua {
template <>
struct udata<neko_fontbatch_t> {
    static inline int nupvalue = 1;
    static inline auto metatable = fontbatch_metatable;
};
}  // namespace neko::lua

LUA_FUNCTION(__neko_bind_fontbatch_create) {

    neko::string font_vs = neko::luax_check_string(L, 1);
    neko::string font_ps = neko::luax_check_string(L, 2);

    neko_fontbatch_t& fontbatch = neko::lua::newudata<neko_fontbatch_t>(L);

    neko::string contents = {};
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, "gamedir/1.fnt");
    NEKO_ASSERT(ok);
    neko_fontbatch_init(&fontbatch, font_vs.data, font_ps.data, neko_game()->DisplaySize, "gamedir/1_0.png", contents.data, (i32)contents.len);
    neko_defer(neko_safe_free(contents.data));

    return 1;
}

LUA_FUNCTION(__neko_bind_fontbatch_draw) {
    neko_fontbatch_t& fontbatch = neko::lua::toudata<neko_fontbatch_t>(L, 1);

    neko_render_draw_batch(&ENGINE_INTERFACE()->cb, fontbatch.font_render, 0, 0, 0);

    return 0;
}

LUA_FUNCTION(__neko_bind_fontbatch_text) {
    int numArgs = lua_gettop(L);  // 获取参数数量
    neko_fontbatch_t& fontbatch = neko::lua::toudata<neko_fontbatch_t>(L, 1);
    auto v1 = lua2struct::unpack<neko_vec2>(L, 2);
    const_str text = lua_tostring(L, 3);

    f32 scale = 0.f;

    if (numArgs > 3) scale = lua_tonumber(L, 4);

    neko_fontbatch_draw(&fontbatch, neko_game()->DisplaySize, text, v1.x, v1.y, 1, 1.f, 800.f, scale);

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

    neko_batch_t sb;

    neko_render_batch_context_t* sprite_batch;
    neko_render_batch_shader_t sprite_shader;
    neko_render_batch_renderable_t sprite_renderable;
    f32 sprite_projection[16];

    int call_count = 0;

    int sprite_verts_count;
    vertex_t sprite_verts[SPRITE_VERTS_MAX];

    neko_image* images;
    int images_count = 0;
};

static void make_sprite(neko_sprite_batch_t* user_handle, neko_sprite_t* sprite, u64 image_id, f32 x, f32 y, f32 scale, f32 angle_radians, int depth) {

    neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());

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
    neko_batch_sprite_t s;
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
    neko_batch_push(&user_handle->sb, s);
}

static void batch_report(neko_batch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata) {

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
        neko_batch_sprite_t* s = sprites + i;

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

LUA_FUNCTION(__neko_bind_sprite_batch_create) {
    PROFILE_FUNC();

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

    neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());

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
    user_handle->images = (neko_image*)neko_safe_malloc(sizeof(neko_image) * count);

    for (i32 i = 0; i < user_handle->images_count; ++i) user_handle->images[i].load(texture_list[i]);

    neko_batch_config_t sb_config;
    neko_batch_set_default_config(&sb_config);
    sb_config.pixel_stride = sizeof(u8) * 4;
    sb_config.atlas_width_in_pixels = 1024;
    sb_config.atlas_height_in_pixels = 1024;
    sb_config.atlas_use_border_pixels = 0;
    sb_config.ticks_to_decay_texture = 3;
    sb_config.lonely_buffer_count_till_flush = 1;
    sb_config.ratio_to_decay_atlas = 0.5f;
    sb_config.ratio_to_merge_atlases = 0.25f;

    sb_config.batch_callback = batch_report;                        // report batches of sprites from `neko_batch_flush`
    sb_config.get_pixels_callback = get_pixels;                     // used to retrieve image pixels from `neko_batch_flush` and `neko_batch_defrag`
    sb_config.generate_texture_callback = generate_texture_handle;  // used to generate a texture handle from `neko_batch_flush` and `neko_batch_defrag`
    sb_config.delete_texture_callback = destroy_texture_handle;     // used to destroy a texture handle from `neko_batch_defrag`

    neko_batch_init(&user_handle->sb, &sb_config, user_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_sprite_batch_render_ortho) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    f32 w = lua_tonumber(L, 2);
    f32 h = lua_tonumber(L, 3);
    f32 x = lua_tonumber(L, 4);
    f32 y = lua_tonumber(L, 5);

    neko_render_batch_ortho_2d(w, h, x / w, y / h, user_handle->sprite_projection);

    neko_render_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_make_sprite) {
    PROFILE_FUNC();
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

LUA_FUNCTION(__neko_bind_sprite_batch_push_sprite) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);
    neko_sprite_t* sprite_handle = (neko_sprite_t*)lua_touserdata(L, 2);
    push_sprite(user_handle, sprite_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_render_begin) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    user_handle->call_count = 0;

    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_render_end) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    {
        neko_batch_defrag(&user_handle->sb);
        neko_batch_tick(&user_handle->sb);
        neko_batch_flush(&user_handle->sb);
        user_handle->sprite_verts_count = 0;
    }

    neko_render_draw_batch(&ENGINE_INTERFACE()->cb, user_handle->sprite_batch, 0, 0, 0);

    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_end) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    neko_batch_term(&user_handle->sb);

    for (i32 i = 0; i < user_handle->images_count; ++i) {
        (*(user_handle->images + i)).free();
    }

    neko_safe_free(user_handle->images);

    neko_render_batch_free(user_handle->sprite_batch);

    return 0;
}

#endif

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

REGISTER_TYPE_DF(i8)
REGISTER_TYPE_DF(i16)
REGISTER_TYPE_DF(i32)
REGISTER_TYPE_DF(i64)
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
    neko::reflection::Any v = var;
    v.foreach ([](std::string_view name, neko::reflection::Any& value) {
        // if (value.GetType() == neko::reflection::type_of<std::string_view>()) {
        //     std::cout << name << " = " << value.cast<std::string_view>() << std::endl;
        // } else if (value.GetType() == neko::reflection::type_of<std::size_t>()) {
        //     std::cout << name << " = " << value.cast<std::size_t>() << std::endl;
        // }
        neko::imgui::Auto(value, std::string(name));
    });
}
DEFINE_IMGUI_END();

LUA_FUNCTION(__neko_bind_gameobject_inspect) {

    CGameObject* user_handle = (CGameObject*)lua_touserdata(L, 1);

    if (user_handle == NULL) return 0;

    // neko_println("gameobj %d %s %s %s", user_handle->id, NEKO_BOOL_STR(user_handle->active), NEKO_BOOL_STR(user_handle->visible), NEKO_BOOL_STR(user_handle->selected));

    ImGui::Text("GameObject_%d", user_handle->id);

    neko::imgui::Auto(user_handle, "CGameObject");

    return 0;
}

#if 0
LUA_FUNCTION(__neko_bind_filesystem_create){
    neko_filesystem_t* filewatch = (neko_filesystem_t*)lua_newuserdata(L, sizeof(neko_filesystem_t));
    memset(filewatch, 0, sizeof(neko_filesystem_t));
    neko_filesystem_create_internal(filewatch, NULL);

    return 1;
}

LUA_FUNCTION(__neko_bind_filesystem_fini){
    neko_filesystem_t* assetsys = (neko_filesystem_t*)lua_touserdata(L, 1);
    neko_filesystem_fini_internal(assetsys);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_create){
    neko_filesystem_t* assetsys = (neko_filesystem_t*)lua_touserdata(L, 1);

    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_newuserdata(L, sizeof(neko_filewatch_t));
    memset(filewatch, 0, sizeof(neko_filewatch_t));
    neko_filewatch_create_internal(filewatch, assetsys, NULL);
    return 1;
}

LUA_FUNCTION(__neko_bind_filewatch_fini){
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

    lua_getglobal(ENGINE_L(), callback_funcname);
    bool is_callback = lua_isfunction(ENGINE_L(), -1);
    lua_pop(ENGINE_L(), 1);

    if (is_callback) try {
            neko_lua_call<void>(ENGINE_L(), callback_funcname, change_string, std::string(virtual_path));
        } catch (std::exception& ex) {
            NEKO_ERROR("lua exception %s", ex.what());
        }
}

LUA_FUNCTION(__neko_bind_filewatch_mount){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str actual_path = lua_tostring(L, 2);
    const_str virtual_path = lua_tostring(L, 3);
    neko_filewatch_mount(filewatch, actual_path, virtual_path);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_start_watch){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str virtual_path = lua_tostring(L, 2);
    const_str callback_funcname = lua_tostring(L, 3);
    neko_filewatch_start_watching(filewatch, virtual_path, watch_map_callback, (void*)callback_funcname);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_stop_watching){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str virtual_path = lua_tostring(L, 2);
    neko_filewatch_stop_watching(filewatch, virtual_path);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_update){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_update(filewatch);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_notify){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_notify(filewatch);
    return 0;
}
#endif

#if 0

LUA_FUNCTION(__neko_bind_idraw_get) {
    lua_pushlightuserdata(L, &ENGINE_INTERFACE()->idraw);
    return 1;
}

LUA_FUNCTION(__neko_bind_idraw_draw) {
    PROFILE_FUNC();
    neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);
    return 1;
}

LUA_FUNCTION(__neko_bind_idraw_defaults) {
    PROFILE_FUNC();
    neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera2d) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, w, h);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera3d) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera3d(&ENGINE_INTERFACE()->idraw, w, h);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera2d_ex) {
    f32 l = lua_tonumber(L, 1);
    f32 r = lua_tonumber(L, 2);
    f32 t = lua_tonumber(L, 3);
    f32 b = lua_tonumber(L, 4);
    neko_idraw_camera2d_ex(&ENGINE_INTERFACE()->idraw, l, r, t, b);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_rotatev) {
    f32 angle = lua_tonumber(L, 1);
    f32 x = lua_tonumber(L, 2);
    f32 y = lua_tonumber(L, 3);
    f32 z = lua_tonumber(L, 4);
    neko_idraw_rotatev(&ENGINE_INTERFACE()->idraw, angle, neko_v3(x, y, z));
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_box) {
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
    neko_luabind_to(ENGINE_LUA(), neko_render_primitive_type, &type_val, 11);
    neko_idraw_box(&ENGINE_INTERFACE()->idraw, x, y, z, hx, hy, hz, r, g, b, a, type_val);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_translatef) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    neko_idraw_translatef(&ENGINE_INTERFACE()->idraw, x, y, z);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_rectv) {

    auto v1 = lua2struct::unpack<neko_vec2>(L, 1);
    auto v2 = lua2struct::unpack<neko_vec2>(L, 2);

    v2 = neko_vec2_add(v1, v2);

    neko_render_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), neko_render_primitive_type, &type_val, 3);

    neko_color_t col = NEKO_COLOR_WHITE;

    if (lua_gettop(L) == 4) {
        col = lua2struct::unpack<neko_color_t>(L, 4);
    }

    neko_idraw_rectv(&ENGINE_INTERFACE()->idraw, v1, v2, col, type_val);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_rectvd) {

    auto v1 = lua2struct::unpack<neko_vec2>(L, 1);
    auto v2 = lua2struct::unpack<neko_vec2>(L, 2);

    auto uv0 = lua2struct::unpack<neko_vec2>(L, 3);
    auto uv1 = lua2struct::unpack<neko_vec2>(L, 4);

    neko_render_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), neko_render_primitive_type, &type_val, 5);

    neko_color_t col = lua2struct::unpack<neko_color_t>(L, 6);

    neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, v1, v2, uv0, uv1, col, type_val);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_text) {
    PROFILE_FUNC();
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);

    neko_color_t col = neko_color(255, 50, 50, 255);

    if (lua_gettop(L) == 4) {
        col = lua2struct::unpack<neko_color_t>(L, 4);
    }

    neko_idraw_text(&ENGINE_INTERFACE()->idraw, x, y, text, NULL, false, col);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    neko_camera_t camera;
    camera = neko_camera_default();
    neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());
    neko_idraw_camera(&ENGINE_INTERFACE()->idraw, &camera, (u32)fbs.x, (u32)fbs.y);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_depth_enabled) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_depth_enabled(&ENGINE_INTERFACE()->idraw, enable);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_face_cull_enabled) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_face_cull_enabled(&ENGINE_INTERFACE()->idraw, enable);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_texture) {
    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(ENGINE_LUA(), neko_texture_t, id, &rt, 1);
    neko_idraw_texture(&ENGINE_INTERFACE()->idraw, rt);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_framebuffer_create) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    fbo = neko_render_framebuffer_create({});
    neko_luabind_struct_push_member(L, neko_framebuffer_t, id, &fbo);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_framebuffer_fini) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);
    neko_render_framebuffer_fini(fbo);
    return 0;
}

#define NEKO_LUA_INSPECT_ITER(NAME)                                                                         \
    LUA_FUNCTION(__neko_bind_inspect_##NAME##_next) {                                                       \
        neko_gl_data_t* ogl = (neko_gl_data_t*)lua_touserdata(L, lua_upvalueindex(1));                      \
        neko_slot_array_iter* it = (neko_slot_array_iter*)lua_touserdata(L, lua_upvalueindex(2));           \
        if (!neko_slot_array_iter_valid(ogl->NAME, *it)) {                                                  \
            return 0;                                                                                       \
        }                                                                                                   \
        auto s = neko_slot_array_iter_get(ogl->NAME, *it);                                                  \
        neko_slot_array_iter_advance(ogl->NAME, *it);                                                       \
        lua_pushinteger(L, s.id);                                                                           \
        return 1;                                                                                           \
    }                                                                                                       \
                                                                                                            \
    LUA_FUNCTION(__neko_bind_inspect_##NAME##_iterator) {                                                   \
        neko_gl_data_t* ogl = neko_render_userdata();                                                       \
        neko_slot_array_iter* it = (neko_slot_array_iter*)lua_newuserdata(L, sizeof(neko_slot_array_iter)); \
        *it = neko_slot_array_iter_new(ogl->NAME);                                                          \
        lua_pushlightuserdata(L, ogl);                                                                      \
        lua_pushvalue(L, -2);                                                                               \
        lua_pushcclosure(L, __neko_bind_inspect_##NAME##_next, 2);                                          \
        return 1;                                                                                           \
    }

#define NEKO_LUA_INSPECT_REG(NAME) \
    { "inspect_" #NAME "_iter", __neko_bind_inspect_##NAME##_iterator }

NEKO_LUA_INSPECT_ITER(shaders)
NEKO_LUA_INSPECT_ITER(textures)
NEKO_LUA_INSPECT_ITER(vertex_buffers)
// NEKO_LUA_INSPECT_ITER(uniform_buffers)
// NEKO_LUA_INSPECT_ITER(storage_buffers)
NEKO_LUA_INSPECT_ITER(index_buffers)
NEKO_LUA_INSPECT_ITER(frame_buffers)
// NEKO_LUA_INSPECT_ITER(uniforms)
// NEKO_LUA_INSPECT_ITER(pipelines)
// NEKO_LUA_INSPECT_ITER(renderpasses)

void inspect_shader(const_str label, GLuint program);



LUA_FUNCTION(__neko_bind_inspect_shaders) {
    u32 shader_id = lua_tonumber(L, 1);
    inspect_shader(std::to_string(shader_id).c_str(), shader_id);
    return 0;
}

LUA_FUNCTION(__neko_bind_inspect_textures) {
    u32 textures_id = lua_tonumber(L, 1);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_shader_create) {

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

    neko_luabind_struct_push_member(L, neko_shader_t, id, &shader_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_render_uniform_create) {

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
            neko_luabind_to(L, neko_render_uniform_type, &layouts[i - 1].type, -1);
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
        neko_luabind_to(L, neko_render_shader_stage_type, &u_desc.stage, 3);
    }

    neko_uniform_t uniform_handle = NEKO_DEFAULT_VAL();

    // Create uniform
    uniform_handle = neko_render_uniform_create(u_desc);

    neko_safe_free(layouts);

    neko_luabind_struct_push_member(L, neko_uniform_t, id, &uniform_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_render_pipeline_create) {
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
                        neko_luabind_struct_to_member(L, neko_shader_t, id, &shader_handle, -1);
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
                        neko_luabind_struct_to_member(L, neko_shader_t, id, &shader_handle, -1);
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
    neko_luabind_struct_push_member(L, neko_pipeline_t, id, &pipeline_handle);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_vertex_buffer_create) {
    const_str vertex_buffer_name = lua_tostring(L, 1);
    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);
    neko_vbo_t vertex_buffer_handle = NEKO_DEFAULT_VAL();
    neko_render_vertex_buffer_desc_t vertex_buffer_desc = {.data = data, .size = data_size};
    vertex_buffer_handle = neko_render_vertex_buffer_create(vertex_buffer_desc);
    neko_luabind_struct_push_member(L, neko_vbo_t, id, &vertex_buffer_handle);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_index_buffer_create) {
    const_str index_buffer_name = lua_tostring(L, 1);
    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);
    neko_ibo_t index_buffer_handle = NEKO_DEFAULT_VAL();
    neko_render_index_buffer_desc_t index_buffer_desc = {.data = data, .size = data_size};
    index_buffer_handle = neko_render_index_buffer_create(index_buffer_desc);
    neko_luabind_struct_push_member(L, neko_ibo_t, id, &index_buffer_handle);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_vertex_attribute_create) {
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
            neko_luabind_to(L, neko_render_vertex_attribute_type, &sources[i].format, -1);
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

LUA_FUNCTION(__neko_bind_render_storage_buffer_create) {

    const_str storage_buffer_name = lua_tostring(L, 1);

    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);

    neko_storage_buffer_t storage_buffer_handle = NEKO_DEFAULT_VAL();

    neko_render_storage_buffer_desc_t storage_buffer_desc = {.data = data, .size = data_size, .name = "unknown", .usage = R_BUFFER_USAGE_DYNAMIC};

    if (storage_buffer_name != NULL) {
        NEKO_STR_CPY(storage_buffer_desc.name, storage_buffer_name);
    }

    storage_buffer_handle = neko_render_storage_buffer_create(storage_buffer_desc);

    neko_luabind_struct_push_member(L, neko_storage_buffer_t, id, &storage_buffer_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_render_pipeline_bind) {
    neko_pipeline_t pipeline_handle = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_pipeline_t, id, &pipeline_handle, 1);
    neko_render_pipeline_bind(&ENGINE_INTERFACE()->cb, pipeline_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_apply_bindings) {

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

                    u_desc = (neko_render_bind_uniform_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_uniform_desc_t));
                    memset(u_desc, 0, n * sizeof(neko_render_bind_uniform_desc_t));

                    binds.uniforms.desc = u_desc;
                    binds.uniforms.size = n == 1 ? 0 : n * sizeof(neko_render_bind_uniform_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "uniform");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (!lua_isnil(L, -1))
                            neko_luabind_struct_to_member(L, neko_uniform_t, id, &u_desc[i - 1].uniform, -1);
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
                    ib_desc = (neko_render_bind_image_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_image_buffer_desc_t));
                    memset(ib_desc, 0, n * sizeof(neko_render_bind_image_buffer_desc_t));

                    binds.image_buffers.desc = ib_desc;
                    binds.image_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_image_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "tex");  // # -1
                        lua_gettable(L, -2);       // pop # -1
                        if (!lua_isnil(L, -1))
                            neko_luabind_struct_to_member(L, neko_texture_t, id, &ib_desc[i - 1].tex, -1);
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
                    sb_desc = (neko_render_bind_storage_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_storage_buffer_desc_t));
                    memset(sb_desc, 0, n * sizeof(neko_render_bind_storage_buffer_desc_t));

                    binds.storage_buffers.desc = sb_desc;
                    binds.storage_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_storage_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_luabind_struct_to_member(L, neko_storage_buffer_t, id, &sb_desc[i - 1].buffer, -1);
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
                    vbo_desc = (neko_render_bind_vertex_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_vertex_buffer_desc_t));
                    memset(vbo_desc, 0, n * sizeof(neko_render_bind_vertex_buffer_desc_t));

                    binds.vertex_buffers.desc = vbo_desc;
                    binds.vertex_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_vertex_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_luabind_struct_to_member(L, neko_vbo_t, id, &vbo_desc[i - 1].buffer, -1);
                        } else
                            NEKO_ASSERT(false);
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("index_buffers"): {
                    ibo_desc = (neko_render_bind_index_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_index_buffer_desc_t));
                    memset(ibo_desc, 0, n * sizeof(neko_render_bind_index_buffer_desc_t));

                    binds.index_buffers.desc = ibo_desc;
                    binds.index_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_index_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_luabind_struct_to_member(L, neko_ibo_t, id, &ibo_desc[i - 1].buffer, -1);
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

    neko_render_apply_bindings(&ENGINE_INTERFACE()->cb, &binds);

    if (u_desc) neko_safe_free(u_desc);
    if (ib_desc) neko_safe_free(ib_desc);
    if (sb_desc) neko_safe_free(sb_desc);
    if (vbo_desc) neko_safe_free(vbo_desc);
    if (ibo_desc) neko_safe_free(ibo_desc);

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

LUA_FUNCTION(__neko_bind_render_dispatch_compute) {
    f32 x_groups = lua_tonumber(L, 1);
    f32 y_groups = lua_tonumber(L, 2);
    f32 z_groups = lua_tonumber(L, 3);
    neko_render_dispatch_compute(&ENGINE_INTERFACE()->cb, x_groups, y_groups, z_groups);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_texture_create) {
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
                neko_luabind_to(L, neko_render_texture_type, &texture_desc.type, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "format");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_format_type, &texture_desc.format, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_s");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_wrapping_type, &texture_desc.wrap_s, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_t");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_wrapping_type, &texture_desc.wrap_t, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "min_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_filtering_type, &texture_desc.min_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "mag_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_filtering_type, &texture_desc.mag_filter, -1);
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
    neko_luabind_struct_push_member(L, neko_texture_t, id, &texture);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_texture_fini) {
    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_texture_t, id, &rt, 1);
    neko_render_texture_fini(rt);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_renderpass_create) {

    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);

    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_texture_t, id, &rt, 2);

    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    rp = neko_render_renderpass_create(neko_render_renderpass_desc_t{
            .fbo = fbo,               // Frame buffer to bind for render pass
            .color = &rt,             // Color buffer array to bind to frame buffer
            .color_size = sizeof(rt)  // Size of color attachment array in bytes
    });
    neko_luabind_struct_push_member(L, neko_renderpass_t, id, &rp);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_renderpass_fini) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_render_renderpass_fini(rp);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_renderpass_begin) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, rp);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_renderpass_end) {
    neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_draw) {
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

    neko_render_draw(&ENGINE_INTERFACE()->cb, draw_desc);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_set_viewport) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 w = lua_tonumber(L, 3);
    f32 h = lua_tonumber(L, 4);
    neko_render_set_viewport(&ENGINE_INTERFACE()->cb, x, y, w, h);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_clear) {
    f32 r = lua_tonumber(L, 1);
    f32 g = lua_tonumber(L, 2);
    f32 b = lua_tonumber(L, 3);
    f32 a = lua_tonumber(L, 4);
    neko_render_clear_action_t clear = {.color = {r, g, b, a}};
    neko_render_clear(&ENGINE_INTERFACE()->cb, clear);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_display_size) {
    neko_vec2 v1 = neko_game()->DisplaySize;
    // lua2struct::pack_struct<neko_vec2, 2>(L, v1);
    lua_pushnumber(L, v1.x);
    lua_pushnumber(L, v1.y);
    return 2;
}



inline void neko_register_test(lua_State* L) {

    // neko::lua_bind::bind("neko_tiled_get_objects", &__neko_bind_tiled_get_objects);

    neko_luabind_enum(L, neko_projection_type);
    neko_luabind_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_ORTHOGRAPHIC);
    neko_luabind_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_PERSPECTIVE);

    neko_luabind_enum(L, neko_render_primitive_type);
    neko_luabind_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_LINES);
    neko_luabind_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_TRIANGLES);
    neko_luabind_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_QUADS);

    neko_luabind_struct(L, neko_vec2);
    neko_luabind_struct_member(L, neko_vec2, x, f32);
    neko_luabind_struct_member(L, neko_vec2, y, f32);
    neko_luabind_struct_member(L, neko_vec2, xy, f32[2]);

    neko_luabind_struct(L, neko_vec3);
    neko_luabind_struct_member(L, neko_vec3, x, f32);
    neko_luabind_struct_member(L, neko_vec3, y, f32);
    neko_luabind_struct_member(L, neko_vec3, z, f32);
    neko_luabind_struct_member(L, neko_vec3, xyz, f32[3]);

    neko_luabind_struct(L, neko_quat);
    neko_luabind_struct_member(L, neko_quat, x, f32);
    neko_luabind_struct_member(L, neko_quat, y, f32);
    neko_luabind_struct_member(L, neko_quat, z, f32);
    neko_luabind_struct_member(L, neko_quat, w, f32);

    neko_luabind_struct(L, neko_vqs);
    neko_luabind_struct_member(L, neko_vqs, position, neko_vec3);
    neko_luabind_struct_member(L, neko_vqs, translation, neko_vec3);
    neko_luabind_struct_member(L, neko_vqs, rotation, neko_quat);
    neko_luabind_struct_member(L, neko_vqs, scale, neko_vec3);

    neko_luabind_struct(L, neko_camera_t);
    neko_luabind_struct_member(L, neko_camera_t, transform, neko_vqs);
    neko_luabind_struct_member(L, neko_camera_t, fov, f32);
    neko_luabind_struct_member(L, neko_camera_t, aspect_ratio, f32);
    neko_luabind_struct_member(L, neko_camera_t, near_plane, f32);
    neko_luabind_struct_member(L, neko_camera_t, far_plane, f32);
    neko_luabind_struct_member(L, neko_camera_t, ortho_scale, f32);
    neko_luabind_struct_member(L, neko_camera_t, proj_type, neko_projection_type);

    neko_luabind_struct(L, neko_framebuffer_t);
    neko_luabind_struct_member(L, neko_framebuffer_t, id, unsigned int);

    neko_luabind_struct(L, neko_texture_t);
    neko_luabind_struct_member(L, neko_texture_t, id, unsigned int);

    neko_luabind_struct(L, neko_pipeline_t);
    neko_luabind_struct_member(L, neko_pipeline_t, id, unsigned int);

    neko_luabind_struct(L, neko_uniform_t);
    neko_luabind_struct_member(L, neko_uniform_t, id, unsigned int);

    neko_luabind_struct(L, neko_storage_buffer_t);
    neko_luabind_struct_member(L, neko_storage_buffer_t, id, unsigned int);

    neko_luabind_struct(L, neko_shader_t);
    neko_luabind_struct_member(L, neko_shader_t, id, unsigned int);

    neko_luabind_struct(L, neko_vbo_t);
    neko_luabind_struct_member(L, neko_vbo_t, id, unsigned int);

    neko_luabind_struct(L, neko_ibo_t);
    neko_luabind_struct_member(L, neko_ibo_t, id, unsigned int);

    neko_luabind_struct(L, neko_renderpass_t);
    neko_luabind_struct_member(L, neko_renderpass_t, id, unsigned int);

    // neko_luabind_struct(L, neko_sound_playing_sound_t);
    // neko_luabind_struct_member(L, neko_sound_playing_sound_t, id, unsigned long long);

    neko_luabind_enum(L, neko_render_vertex_attribute_type);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT4);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT3);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT2);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT4);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT3);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT2);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE4);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE3);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE2);
    neko_luabind_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE);

    neko_luabind_enum(L, neko_render_texture_type);
    neko_luabind_enum_value(L, neko_render_texture_type, R_TEXTURE_2D);
    neko_luabind_enum_value(L, neko_render_texture_type, R_TEXTURE_CUBEMAP);

    neko_luabind_enum(L, neko_render_shader_stage_type);
    neko_luabind_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_VERTEX);
    neko_luabind_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_FRAGMENT);
    neko_luabind_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_COMPUTE);

    neko_luabind_enum(L, neko_render_texture_wrapping_type);
    neko_luabind_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_REPEAT);
    neko_luabind_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_MIRRORED_REPEAT);
    neko_luabind_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_CLAMP_TO_EDGE);
    neko_luabind_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_CLAMP_TO_BORDER);

    neko_luabind_enum(L, neko_render_texture_filtering_type);
    neko_luabind_enum_value(L, neko_render_texture_filtering_type, R_TEXTURE_FILTER_NEAREST);
    neko_luabind_enum_value(L, neko_render_texture_filtering_type, R_TEXTURE_FILTER_LINEAR);

    neko_luabind_enum(L, neko_render_texture_format_type);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGB8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RG8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R32);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R32F);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA16F);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA32F);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_A8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH16);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH24);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH32F);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH24_STENCIL8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH32F_STENCIL8);
    neko_luabind_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_STENCIL8);

    neko_luabind_enum(L, neko_render_uniform_type);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_FLOAT);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_INT);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC2);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC3);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC4);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_MAT4);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_SAMPLER2D);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_USAMPLER2D);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_SAMPLERCUBE);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_IMAGE2D_RGBA32F);
    neko_luabind_enum_value(L, neko_render_uniform_type, R_UNIFORM_BLOCK);
}

#endif

#if 0
LUA_FUNCTION(__neko_bind_cvar) {
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
                lua_pushstring(ENGINE_LUA(), type);
                neko_luabind_to(ENGINE_LUA(), neko_cvar_type, &cval, -1);
                lua_pop(ENGINE_LUA(), 1);
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
                    NEKO_WARN(std::format("__neko_bind_cvar_new with a unknown type {0} {1}", name, (u8)cval).c_str());
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
#endif

LUA_FUNCTION(__neko_bind_print) {
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
    NEKO_INFO("[lua] %s", str.c_str());
    return 0;
}

LUA_FUNCTION(__neko_bind_pack_build) {

    const_str path = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    lua_len(L, 2);                     // 获取table的长度
    int n = lua_tointeger(L, -1);      //
    lua_pop(L, 1);                     // 弹出长度值

    const_str* item_paths = (const_str*)mem_alloc(n * sizeof(const_str));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);                 // 将index=i的元素压入堆栈顶部
        const_str str = lua_tostring(L, -1);  // # -1
        if (str != NULL) {
            item_paths[i - 1] = str;
        }
        lua_pop(L, 1);  // # -1
    }

    bool ok = neko_pak_build(path, n, item_paths, true);

    mem_free(item_paths);

    if (!ok) {
        const_str error_message = "__neko_bind_pack_build failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    return 0;
}

LUA_FUNCTION(__neko_bind_pack_info) {
    const_str path = lua_tostring(L, 1);
    i32 buildnum;
    u64 item_count;
    bool ok = neko_pak_info(path, &buildnum, &item_count);
    if (ok) {
        lua_pushinteger(L, buildnum);
        lua_pushinteger(L, item_count);
        return 2;
    } else
        return 0;
}

LUA_FUNCTION(__neko_bind_vfs_read_file) {
    const_str path = lua_tostring(L, 1);

    String str;

    const_str vpks[] = {NEKO_PACKS::GAMEDATA, NEKO_PACKS::LUACODE};
    bool ok = false;
    for (auto vpk : vpks) {
        ok = vfs_read_entire_file(vpk, &str, path);
        if (ok) break;
    }

    if (ok) {
        const_str data = str.data;
        lua_pushstring(L, data);
        neko_defer(mem_free(str.data));
        return 1;
    } else {
        const_str error_message = "todo";
        lua_pushstring(L, error_message);
        return lua_error(L);
    }
}

void __neko_lua_bug(const_str message) { NEKO_INFO("[lua] %s", message); }
void __neko_lua_info(const_str message) { NEKO_INFO("[lua] %s", message); }
void __neko_lua_trace(const_str message) { NEKO_INFO("[lua] %s", message); }
void __neko_lua_error(const_str message) { NEKO_ERROR("[lua] %s", message); }
void __neko_lua_warn(const_str message) { NEKO_WARN("[lua] %s", message); }

// 返回包含路径和 isDirectory 对的表
int __neko_ls(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        NEKO_WARN("invalid lua argument");
        return 0;
    }
    auto string = lua_tostring(L, 1);
    if (!std::filesystem::is_directory(string)) {
        NEKO_WARN(std::format("{0} is not directory", string).c_str());
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

bool __neko_dolua(const_str file) { return neko::neko_lua_dofile(ENGINE_LUA(), file); }

void neko_tolua_boot(const_str f, const_str output);

inline void neko_register_common(lua_State* L) {

    neko::lua_bind::bind("log_trace", &__neko_lua_trace);
    neko::lua_bind::bind("log_debug", &__neko_lua_bug);
    neko::lua_bind::bind("log_error", &__neko_lua_error);
    neko::lua_bind::bind("log_warn", &__neko_lua_warn);
    neko::lua_bind::bind("log_info", &__neko_lua_info);
    // neko::lua_bind::bind("neko_rand", &neko_rand_xorshf32);
    neko::lua_bind::bind("neko_dolua", &__neko_dolua);

    neko::lua_bind::bind("neko_tolua_gen", +[](const_str f, const_str o) { neko_tolua_boot(f, o); });

    neko::lua_bind::bind("neko_hash_str", +[](const_str str) { return neko_hash_str(str); });
    neko::lua_bind::bind("neko_hash_str64", +[](const_str str) { return neko_hash_str64(str); });
    // neko::lua_bind::bind("__neko_quit", +[](int op) { return neko_quit(); });
    // neko::lua_bind::bind("neko_os_elapsed_time", +[]() { return neko_os_elapsed_time(); });

    // lua_pushstring(L, game_assets("gamedir").c_str());
    // lua_setglobal(L, "neko_game_data_path");

    // const neko_luaL_reg luaReg[] = {{"hooks", neko_lua_events_init}, {NULL, NULL}};
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

    // neko_luabind_enum(L, neko_cvar_type);
    // neko_luabind_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_INT);
    // neko_luabind_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_FLOAT);
    // neko_luabind_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_STRING);
    // neko_luabind_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_COUNT);

    lua_register(L, "__neko_ls", __neko_ls);
}

#if 0

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

#endif

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
    // p->t = neko_os_elapsed_time();
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
    // lua_pushinteger(L, neko_os_elapsed_time() - p->t);
    return 2;
}

LUA_FUNCTION(__neko_bind_ecs_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");
    return 1;
}

void createStructTables(lua_State* L);

int __neko_sandbox_create_world(lua_State* L);

static neko_luaref getLr(lua_State* L) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    neko_luaref ref;
    ref.refL = (luaref)lua_touserdata(L, 1);
    return ref;
}

static int ref_init(lua_State* L) {
    neko_luaref ref;
    ref.make(L);
    lua_pushlightuserdata(L, (void*)ref.refL);
    return 1;
}

static int ref_close(lua_State* L) {
    neko_luaref ref = getLr(L);
    ref.fini();
    return 0;
}

static int ref_isvalid(lua_State* L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    lua_pushboolean(L, ref.isvalid(r));
    return 1;
}

static int ref_ref(lua_State* L) {
    neko_luaref ref = getLr(L);
    lua_settop(L, 2);
    int r = ref.ref(L);
    if (r == LUA_NOREF) {
        return luaL_error(L, "Too many refs.");
    }
    if (r <= 1) {
        return luaL_error(L, "Unexpected error.");
    }
    lua_pushinteger(L, r);
    return 1;
}

static int ref_unref(lua_State* L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    ref.unref(r);
    return 0;
}

static int ref_get(lua_State* L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    if (!ref.isvalid(r)) {
        return luaL_error(L, "invalid ref: %d", r);
    }
    ref.get(L, r);
    return 1;
}

static int ref_set(lua_State* L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    lua_settop(L, 3);
    if (!ref.isvalid(r)) {
        return luaL_error(L, "invalid ref: %d", r);
    }
    ref.set(L, r);
    return 1;
}

static int LUASTRUCT_test_vec4(lua_State* L) {
    // GET_SELF;

    Vector4* v4 = CHECK_STRUCT(L, 1, Vector4);

    v4->x += 10.f;
    v4->y += 10.f;
    v4->z += 10.f;
    v4->w += 10.f;

    PUSH_STRUCT(L, Vector4, *v4);

    // RETURN_STATUS(FMOD_Studio_EventInstance_Set3DAttributes(self, attributes));

    return 1;
}

LUA_FUNCTION(__neko_bind_w_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);
    return 1;
}

static int __neko_w_lua_get_com(lua_State* L) {
    App* w = (App*)luaL_checkudata(L, W_LUA_REGISTRY_CONST::W_CORE_IDX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);
    lua_getiuservalue(L, 1, W_LUA_UPVALUES::NEKO_W_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, W_LUA_UPVALUES::NEKO_W_UPVAL_N);
    return 2;
}

static int __neko_w_lua_gc(lua_State* L) {
    App* w = (App*)luaL_checkudata(L, W_LUA_REGISTRY_CONST::W_CORE_IDX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);
    // ecs_fini_i(w);
    NEKO_DEBUG_LOG("App __gc %p", w);
    return 0;
}

void neko_w_init() {

    lua_State* L = ENGINE_LUA();

    App* ins = (App*)lua_newuserdatauv(L, sizeof(App), NEKO_W_UPVAL_N);  // # -1
    // ins = neko_instance();

    if (luaL_getmetatable(L, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME) == LUA_TNIL) {  // # -2

        // clang-format off
        luaL_Reg ins_mt[] = {
            {"__gc", __neko_w_lua_gc}, 
            {"get_com", __neko_w_lua_get_com}, 
            {NULL, NULL}
        };
        // clang-format on

        lua_pop(L, 1);                // # pop -2
        luaL_newlibtable(L, ins_mt);  // # -2
        luaL_setfuncs(L, ins_mt, 0);
        lua_pushvalue(L, -1);                                                      // # -3
        lua_setfield(L, -2, "__index");                                            // pop -3
        lua_pushstring(L, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);                   // # -3
        lua_setfield(L, -2, "__name");                                             // pop -3
        lua_pushvalue(L, -1);                                                      // # -3
        lua_setfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);  // pop -3
    }
    lua_setmetatable(L, -2);  // pop -2

    lua_newtable(L);                                            // # 2
    lua_pushstring(L, W_LUA_REGISTRY_CONST::CVAR_MAP);          // # 3
    lua_createtable(L, 0, W_LUA_REGISTRY_CONST::CVAR_MAP_MAX);  // # 4
    lua_settable(L, -3);
    lua_setiuservalue(L, -2, NEKO_W_COMPONENTS_NAME);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, -2, NEKO_W_UPVAL_N);

    // lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);

    NEKO_ASSERT(lua_gettop(L) == 0);

    lua_register(L, "neko_w_f", __neko_bind_w_f);

    neko_w_lua_variant<i64> version("neko_engine_version", neko_buildnum());

    NEKO_ASSERT(lua_gettop(L) == 0);
}

static int open_embed_core(lua_State* L) {

    luaL_checkversion(L);

    luaL_Reg reg[] = {

            {"ecs_f", __neko_bind_ecs_f},
            {"sandbox_f", __neko_sandbox_create_world},

            // {"tiled_create", __neko_bind_tiled_create},
            // {"tiled_render", __neko_bind_tiled_render},
            // {"tiled_end", __neko_bind_tiled_end},
            // {"tiled_load", __neko_bind_tiled_load},
            // {"tiled_unload", __neko_bind_tiled_unload},

            // {"pixelui_create", __neko_bind_pixelui_create},
            // {"pixelui_update", __neko_bind_pixelui_update},
            // {"pixelui_end", __neko_bind_pixelui_end},
            // {"pixelui_tex", __neko_bind_pixelui_tex},

            // {"gfxt_create", __neko_bind_gfxt_create},
            // {"gfxt_update", __neko_bind_gfxt_update},
            // {"gfxt_end", __neko_bind_gfxt_end},

            // {"fontbatch_create", __neko_bind_fontbatch_create},
            // {"fontbatch_draw", __neko_bind_fontbatch_draw},
            // {"fontbatch_text", __neko_bind_fontbatch_text},

            // {"sprite_batch_create", __neko_bind_sprite_batch_create},
            // {"sprite_batch_render_ortho", __neko_bind_sprite_batch_render_ortho},
            // {"sprite_batch_render_begin", __neko_bind_sprite_batch_render_begin},
            // {"sprite_batch_render_end", __neko_bind_sprite_batch_render_end},
            // {"sprite_batch_make_sprite", __neko_bind_sprite_batch_make_sprite},
            // {"sprite_batch_push_sprite", __neko_bind_sprite_batch_push_sprite},
            // {"sprite_batch_end", __neko_bind_sprite_batch_end},

            {"gameobject_inspect", __neko_bind_gameobject_inspect},

            // {"filesystem_create", __neko_bind_filesystem_create},
            // {"filesystem_fini", __neko_bind_filesystem_fini},
            // {"filewatch_create", __neko_bind_filewatch_create},
            // {"filewatch_fini", __neko_bind_filewatch_fini},
            // {"filewatch_mount", __neko_bind_filewatch_mount},
            // {"filewatch_start", __neko_bind_filewatch_start_watch},
            // {"filewatch_stop", __neko_bind_filewatch_stop_watching},
            // {"filewatch_update", __neko_bind_filewatch_update},
            // {"filewatch_notify", __neko_bind_filewatch_notify},

            {"callback_save", __neko_bind_callback_save},
            {"callback_call", __neko_bind_callback_call},

            // {"idraw_get", __neko_bind_idraw_get},
            // {"idraw_draw", __neko_bind_idraw_draw},
            // {"idraw_defaults", __neko_bind_idraw_defaults},
            // {"idraw_rectv", __neko_bind_idraw_rectv},
            // {"idraw_rectvd", __neko_bind_idraw_rectvd},
            // {"idraw_text", __neko_bind_idraw_text},
            // {"idraw_camera", __neko_bind_idraw_camera},
            // {"idraw_camera2d", __neko_bind_idraw_camera2d},
            // {"idraw_camera2d_ex", __neko_bind_idraw_camera2d_ex},
            // {"idraw_camera3d", __neko_bind_idraw_camera3d},
            // {"idraw_rotatev", __neko_bind_idraw_rotatev},
            // {"idraw_box", __neko_bind_idraw_box},
            // {"idraw_translatef", __neko_bind_idraw_translatef},
            // {"idraw_depth_enabled", __neko_bind_idraw_depth_enabled},
            // {"idraw_face_cull_enabled", __neko_bind_idraw_face_cull_enabled},
            // {"idraw_texture", __neko_bind_idraw_texture},

            // {"render_framebuffer_create", __neko_bind_render_framebuffer_create},
            // {"render_framebuffer_fini", __neko_bind_render_framebuffer_fini},
            // {"render_texture_create", __neko_bind_render_texture_create},
            // {"render_texture_fini", __neko_bind_render_texture_fini},
            // {"render_renderpass_create", __neko_bind_render_renderpass_create},
            // {"render_renderpass_fini", __neko_bind_render_renderpass_fini},
            // {"render_renderpass_begin", __neko_bind_render_renderpass_begin},
            // {"render_renderpass_end", __neko_bind_render_renderpass_end},
            // {"render_set_viewport", __neko_bind_render_set_viewport},
            // {"render_clear", __neko_bind_render_clear},
            // {"render_shader_create", __neko_bind_render_shader_create},
            // {"render_uniform_create", __neko_bind_render_uniform_create},
            // {"render_pipeline_create", __neko_bind_render_pipeline_create},
            // {"render_storage_buffer_create", __neko_bind_render_storage_buffer_create},
            // {"render_vertex_buffer_create", __neko_bind_render_vertex_buffer_create},
            // {"render_index_buffer_create", __neko_bind_render_index_buffer_create},
            // {"render_vertex_attribute_create", __neko_bind_render_vertex_attribute_create},
            // {"render_pipeline_bind", __neko_bind_render_pipeline_bind},
            // {"render_apply_bindings", __neko_bind_render_apply_bindings},
            // {"render_dispatch_compute", __neko_bind_render_dispatch_compute},
            // {"render_draw", __neko_bind_render_draw},
            // {"render_display_size", __neko_bind_render_display_size},
            // {"gen_tex", test_tex},

            // {"get_channel", __neko_bind_get_channel},
            // {"select", __neko_bind_select},
            // {"thread_id", __neko_bind_thread_id},
            // {"thread_sleep", __neko_bind_thread_sleep},
            // {"make_thread", __neko_bind_make_thread},
            // {"make_channel", __neko_bind_make_channel},

            {"vfs_read_file", __neko_bind_vfs_read_file},

            {"print", __neko_bind_print},

            // {"json_read", __neko_bind_json_read},
            // {"json_write", __neko_bind_json_write},

            {"profiler_start", neko_lua_profiler_start},
            {"profiler_stop", neko_lua_profiler_stop},
            {"profiler_info", neko_lua_profiler_info},

            {"LUASTRUCT_test_vec4", LUASTRUCT_test_vec4},

            // luaref
            {"ref_init", ref_init},
            {"ref_close", ref_close},
            {"ref_isvalid", ref_isvalid},
            {"ref_ref", ref_ref},
            {"ref_unref", ref_unref},
            {"ref_get", ref_get},
            {"ref_set", ref_set},

            // pak
            {"pak_build", __neko_bind_pack_build},
            {"pak_info", __neko_bind_pack_info},

            {NULL, NULL}};

    luaL_newlib(L, reg);

#if 0
    luaL_Reg inspector_reg[] = {{"inspect_shaders", __neko_bind_inspect_shaders},

                                NEKO_LUA_INSPECT_REG(shaders),
                                NEKO_LUA_INSPECT_REG(textures),
                                NEKO_LUA_INSPECT_REG(vertex_buffers),
                                // NEKO_LUA_INSPECT_REG(uniform_buffers),
                                // NEKO_LUA_INSPECT_REG(storage_buffers),
                                NEKO_LUA_INSPECT_REG(index_buffers),
                                NEKO_LUA_INSPECT_REG(frame_buffers),
                                // NEKO_LUA_INSPECT_REG(uniforms),
                                // NEKO_LUA_INSPECT_REG(pipelines),
                                // NEKO_LUA_INSPECT_REG(renderpasses),
                                {NULL, NULL}};
    luaL_setfuncs(L, inspector_reg, 0);
#endif

    // {
    //     register_mt_aseprite_renderer(L);
    //     lua_setfield(L, -2, "aseprite_render");
    // }

    // {
    //     register_mt_aseprite(L);
    //     lua_setfield(L, -2, "aseprite");
    // }

    // {
    //     register_mt_imgui(L);
    //     lua_setfield(L, -2, "imgui");
    // }

    createStructTables(L);

    return 1;
}

namespace neko::lua::__core {
LUABIND_MODULE() { return open_embed_core(L); }
}  // namespace neko::lua::__core

DEFINE_LUAOPEN(core)