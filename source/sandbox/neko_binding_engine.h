
#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include <filesystem>
#include <string>

#include "engine/neko.h"
#include "engine/neko_engine.h"
#include "engine/util/neko_asset.h"
#include "engine/util/neko_lua.hpp"
#include "engine/util/neko_sprite.h"
#include "engine/util/neko_tiled.h"

// game
#include "sandbox/game_chunk.h"
#include "sandbox/neko_gui_auto.hpp"

neko_global lua_State* g_lua_bind;

neko_inline neko_vec2 __neko_lua_table_to_v2(lua_State* L, int idx) {
    if (!lua_istable(L, idx)) {
        neko_log_warning("not a table on top");
        return neko_default_val();
    }

    lua_pushstring(L, "x");
    lua_gettable(L, idx);
    f32 x = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "y");
    lua_gettable(L, idx);
    f32 y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return {x, y};
}

static int g_lua_callbacks_table_ref = LUA_NOREF;

int __neko_bind_callback_save(lua_State* L) {
    // 检查传递给函数的参数是否是一个字符串和一个函数
    const char* identifier = luaL_checkstring(L, 1);
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

int __neko_bind_callback_call(lua_State* L) {
    // 检查之前是否有保存的Lua函数引用
    if (g_lua_callbacks_table_ref != LUA_NOREF) {
        // 获取标识符参数
        const char* identifier = luaL_checkstring(L, 1);

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
            throw lua_exception_t(std::format("callback with identifier '{}' not found or is not a function.", identifier));
        }

        // 弹出保存的table
        lua_pop(L, 1);
    }

    return 0;
}

static bool __neko_bind_platform_key_pressed(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_key_pressed(cval);
}

static bool __neko_bind_platform_was_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_key_down(cval);
}

static bool __neko_bind_platform_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_key_down(cval);
}

static bool __neko_bind_platform_key_released(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_key_released(cval);
}

static bool __neko_bind_platform_was_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_was_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_pressed(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_mouse_pressed(cval);
}

static bool __neko_bind_platform_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    return neko_platform_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_released(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

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

static int __neko_bind_platform_frame_buffer_size(lua_State* L) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_platform_framebuffer_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

neko_inline void neko_register_platform(lua_State* L) {

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

    neko_lua_wrap_register_t<>(L)
            .def(&__neko_bind_platform_key_pressed, "neko_key_pressed")
            .def(&__neko_bind_platform_was_key_down, "neko_was_key_down")
            .def(&__neko_bind_platform_was_mouse_down, "neko_was_mouse_down")
            .def(&__neko_bind_platform_key_down, "neko_key_down")
            .def(&__neko_bind_platform_key_released, "neko_key_released")
            .def(&__neko_bind_platform_mouse_down, "neko_mouse_down")
            .def(&__neko_bind_platform_mouse_pressed, "neko_mouse_pressed")
            .def(&__neko_bind_platform_mouse_released, "neko_mouse_released")
            .def(
                    +[]() -> bool { return neko_platform_mouse_moved(); }, "neko_mouse_moved")
            .def(
                    +[]() -> u32 { return neko_platform_main_window(); }, "neko_main_window")
            .def(
                    +[](u32 handle, u32 width, u32 height) { neko_platform_set_window_size(handle, width, height); }, "neko_set_window_size")
            .def(
                    +[](u32 handle, f64 x, f64 y) { neko_platform_mouse_set_position(handle, x, y); }, "neko_set_mouse_position");

    //.def(+[](const_str path) -> neko_string { return neko_engine_subsystem(platform)->get_path(path); }, "neko_file_path")
    //.def(+[](const_str title, u32 width, u32 height) -> neko_resource_handle { return neko_engine_subsystem(platform)->create_window(title, width, height); }, "neko_create_window")

    lua_register(L, "neko_mouse_delta", __neko_bind_platform_mouse_delta);
    lua_register(L, "neko_mouse_position", __neko_bind_platform_mouse_position);
    lua_register(L, "neko_mouse_wheel", __neko_bind_platform_mouse_wheel);

    lua_register(L, "neko_window_size", __neko_bind_platform_window_size);
    lua_register(L, "neko_frame_buffer_size", __neko_bind_platform_frame_buffer_size);
}

/*
static int __neko_bind_pack_construct(lua_State* L) {
    const_str name = lua_tostring(L, 1);
    const_str path = lua_tostring(L, 2);

    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_newuserdata(L, sizeof(neko_lua_handle_t));

    userdata_ptr->name = name;
    userdata_ptr->size = 0;

    neko_pack_result result = neko_pack_read(path, 0, false, (neko_packreader_t**)&userdata_ptr->data);

    if (!neko_pack_check(result)) {
        const_str error_message = "neko_pack_check failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // 获取元表并设置
    luaL_newmetatable(L, "neko_lua_handle__pack");  // 供测试的元表
    lua_setmetatable(L, -2);

    neko_log_info("__neko_bind_pack_construct %s", userdata_ptr->name);
    return 1;
}

static int __neko_bind_pack_destroy(lua_State* L) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);

    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);
    neko_pack_destroy(pack);

    neko_log_info("__neko_bind_pack_destroy %s", userdata_ptr->name);
    return 0;
}

static int __neko_bind_pack_assets_load(lua_State* L) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);
    const_str path = lua_tostring(L, 2);
    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);

    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_newuserdata(L, sizeof(neko_lua_handle_t));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    neko_pack_result result = neko_pack_item_data(pack, path, (const u8**)&assets_user_handle->data, (u32*)&assets_user_handle->size);

    if (!neko_pack_check(result)) {
        const_str error_message = "__neko_bind_pack_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // neko_log_info("__neko_bind_pack_assets_load %llu", assets_user_handle->size);

    // 获取元表并设置
    luaL_newmetatable(L, "neko_lua_handle__assets");  // 供测试的元表
    lua_setmetatable(L, -2);

    return 1;
}
*/

static int __neko_bind_aseprite_create(lua_State* L) {

    neko_sprite* sprite_data = (neko_sprite*)lua_touserdata(L, 1);

    neko_sprite_renderer* user_handle = (neko_sprite_renderer*)lua_newuserdata(L, sizeof(neko_sprite_renderer));
    memset(user_handle, 0, sizeof(neko_sprite_renderer));

    user_handle->sprite = sprite_data;

    neko_sprite_renderer_play(user_handle, "Idle");

    return 1;
}

static int __neko_bind_aseprite_update(lua_State* L) {
    neko_sprite_renderer* user_handle = (neko_sprite_renderer*)lua_touserdata(L, 1);

    neko_t* engine = neko_instance();

    neko_sprite_renderer_update(user_handle, engine->ctx.platform->time.delta);

    return 0;
}

static int __neko_bind_aseprite_update_animation(lua_State* L) {
    neko_sprite_renderer* user_handle = (neko_sprite_renderer*)lua_touserdata(L, 1);
    const_str by_tag = lua_tostring(L, 2);
    neko_sprite_renderer_play(user_handle, by_tag);
    return 0;
}

static int __neko_bind_aseprite_render(lua_State* L) {

    if (lua_gettop(L) != 3) {
        return luaL_error(L, "Function expects exactly three arguments");
    }

    neko_sprite_renderer* user_handle = (neko_sprite_renderer*)lua_touserdata(L, 1);

    neko_vec2 xform = __neko_lua_table_to_v2(L, 2);

    int direction = neko_lua_to<int>(L, 3);

    // int direction = 1;

    neko_t* engine = neko_instance();

    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

    s32 index;
    if (user_handle->loop) {
        index = user_handle->loop->indices[user_handle->current_frame];
    } else {
        index = user_handle->current_frame;
    }

    neko_sprite* spr = user_handle->sprite;
    neko_sprite_frame f = spr->frames[index];

    if (direction)
        neko_idraw_rect_textured_ext(&g_idraw, xform.x, xform.y, xform.x + spr->width * 4.f, xform.y + spr->height * 4.f, f.u1, f.v0, f.u0, f.v1, user_handle->sprite->img.id, NEKO_COLOR_WHITE);
    else
        neko_idraw_rect_textured_ext(&g_idraw, xform.x, xform.y, xform.x + spr->width * 4.f, xform.y + spr->height * 4.f, f.u0, f.v0, f.u1, f.v1, user_handle->sprite->img.id, NEKO_COLOR_WHITE);

    return 0;
}

static int __neko_bind_sprite_create(lua_State* L) {
    const_str file_path = lua_tostring(L, 1);

    neko_sprite* user_handle = (neko_sprite*)lua_newuserdata(L, sizeof(neko_sprite));
    memset(user_handle, 0, sizeof(neko_sprite));

    neko_sprite_load(user_handle, file_path);

    return 1;
}

static int __neko_bind_sprite_end(lua_State* L) {
    neko_sprite* user_handle = (neko_sprite*)lua_touserdata(L, 1);
    neko_sprite_end(user_handle);
    return 0;
}

static int __neko_bind_tiled_create(lua_State* L) {
    const_str map_path = lua_tostring(L, 1);

    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_newuserdata(L, sizeof(neko_tiled_renderer));
    memset(user_handle, 0, sizeof(neko_tiled_renderer));

    neko_tiled_load(&(user_handle->map), map_path, NULL);

    neko_tiled_render_init(&g_cb, user_handle,                                             //
                           game_assets("gamedir/assets/shaders/sprite_vs.glsl").c_str(),   //
                           game_assets("gamedir/assets/shaders/sprite_fs.glsl").c_str());  //

    return 1;
}

static int __neko_bind_tiled_render(lua_State* L) {
    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)lua_touserdata(L, 1);

    neko_vec2 xform = __neko_lua_table_to_v2(L, 2);

    neko_command_buffer_t* cb = &g_cb;

    neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
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

        // for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        //     object_group_t *group = tiled_render->map.object_groups + i;
        //     for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
        //         object_t *object = group->objects + ii;
        //         neko_tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE), (f32)(object->y * SPRITE_SCALE)},
        //                                   .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
        //                                   .color = group->color,
        //                                   .use_texture = false};
        //         neko_tiled_render_push(cb, tiled_render, quad);
        //     }
        //     neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        // }

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
    neko_graphics_renderpass_end(cb);

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

static int __neko_bind_fallingsand_create(lua_State* L) {
    // const_str file_path = lua_tostring(L, 1);

    neko_fallsand_render* user_handle = (neko_fallsand_render*)lua_newuserdata(L, sizeof(neko_fallsand_render));
    memset(user_handle, 0, sizeof(neko_fallsand_render));

    game_chunk_init(user_handle);

    return 1;
}

static int __neko_bind_fallingsand_update(lua_State* L) {
    neko_fallsand_render* user_handle = (neko_fallsand_render*)lua_touserdata(L, 1);

    neko_timer_do(t, neko_timed_action(500, printf("game_chunk_update : %llu\n", t);), { game_chunk_update(user_handle); });

    return 0;
}

static int __neko_bind_fallingsand_end(lua_State* L) {
    neko_fallsand_render* user_handle = (neko_fallsand_render*)lua_touserdata(L, 1);
    game_chunk_destroy(user_handle);
    return 0;
}

static int __neko_bind_gfxt_create(lua_State* L) {
    const_str pip_path = lua_tostring(L, 1);   // gamedir/assets/pipelines/simple.sf
    const_str gltf_path = lua_tostring(L, 2);  // gamedir/assets/meshes/Duck.gltf
    const_str tex_path = lua_tostring(L, 3);   // gamedir/assets/textures/DuckCM.png

    neko_gfxt_renderer* gfxt_render = (neko_gfxt_renderer*)lua_newuserdata(L, sizeof(neko_gfxt_renderer));
    memset(gfxt_render, 0, sizeof(neko_gfxt_renderer));

    // Load pipeline from resource file
    gfxt_render->pip = neko_gfxt_pipeline_load_from_file(pip_path);

    // Create material using this pipeline
    neko_gfxt_material_desc_t mat_decl = {.pip_func = {.hndl = &gfxt_render->pip}};
    gfxt_render->mat = neko_gfxt_material_create(&mat_decl);

    // Create mesh that uses the layout from the pipeline's requested mesh layout
    neko_gfxt_mesh_import_options_t mesh_decl = {.layout = gfxt_render->pip.mesh_layout,
                                                 .size = neko_dyn_array_size(gfxt_render->pip.mesh_layout) * sizeof(neko_gfxt_mesh_layout_t),
                                                 .index_buffer_element_size = gfxt_render->pip.desc.raster.index_buffer_element_size};

    gfxt_render->mesh = neko_gfxt_mesh_load_from_file(gltf_path, &mesh_decl);

    gfxt_render->texture = neko_gfxt_texture_load_from_file(tex_path, NULL, false, false);

    return 1;
}

static int __neko_bind_gfxt_update(lua_State* L) {
    neko_gfxt_renderer* gfxt_render = (neko_gfxt_renderer*)lua_touserdata(L, 1);

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    const f32 t = neko_platform_elapsed_time();

    neko_command_buffer_t* cb = &g_cb;

    // Camera for scene
    neko_camera_t cam = neko_camera_perspective();
    cam.transform.position = neko_v3(0.f, 6.f, 20.f);
    neko_vqs trans = {.translation = neko_v3(0.f, 0.f, -10.f), .rotation = neko_quat_angle_axis(t * 0.001f, NEKO_YAXIS), .scale = neko_v3s(0.1f)};
    neko_mat4 model = neko_vqs_to_mat4(&trans);
    neko_mat4 vp = neko_camera_get_view_projection(&cam, fbs.x, fbs.y);
    neko_mat4 mvp = neko_mat4_mul(vp, model);

    // Apply material uniforms
    neko_gfxt_material_set_uniform(&gfxt_render->mat, "u_mvp", &mvp);
    neko_gfxt_material_set_uniform(&gfxt_render->mat, "u_tex", &gfxt_render->texture);

    // Rendering
    neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        // Set view port
        neko_graphics_set_viewport(cb, 0, 0, (int)fbs.x, (int)fbs.y);

        // Bind material
        neko_gfxt_material_bind(cb, &gfxt_render->mat);

        // Bind material uniforms
        neko_gfxt_material_bind_uniforms(cb, &gfxt_render->mat);

        // Render mesh
        neko_gfxt_mesh_draw_material(cb, &gfxt_render->mesh, &gfxt_render->mat);
    }
    neko_graphics_renderpass_end(cb);

    return 0;
}

static int __neko_bind_gfxt_end(lua_State* L) {
    neko_gfxt_renderer* user_handle = (neko_gfxt_renderer*)lua_touserdata(L, 1);

    neko_gfxt_texture_destroy(&user_handle->texture);
    neko_gfxt_mesh_destroy(&user_handle->mesh);
    neko_gfxt_material_destroy(&user_handle->mat);
    neko_gfxt_pipeline_destroy(&user_handle->pip);

    return 0;
}

static int __neko_bind_draw_text(lua_State* L) {

    int numArgs = lua_gettop(L);  // 获取参数数量

    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);

    f32 scale = 0.f;

    if (numArgs > 3) scale = lua_tonumber(L, 4);

    draw_text(test_font_bmfont, text, x, y, 1, 1.f, 800.f, scale);

    return 0;
}

static int __neko_bind_custom_sprite_create(lua_State* L) {
    // const_str file_path = lua_tostring(L, 1);

    neko_fast_sprite_renderer* user_handle = (neko_fast_sprite_renderer*)lua_newuserdata(L, sizeof(neko_fast_sprite_renderer));
    memset(user_handle, 0, sizeof(neko_fast_sprite_renderer));

    neko_fast_sprite_renderer_construct(user_handle, 0, 0, NULL);

    return 1;
}

static int __neko_bind_custom_sprite_render(lua_State* L) {
    neko_fast_sprite_renderer* user_handle = (neko_fast_sprite_renderer*)lua_touserdata(L, 1);
    neko_fast_sprite_renderer_draw(user_handle, &g_cb);
    return 0;
}

static int __neko_bind_custom_sprite_end(lua_State* L) {
    neko_fast_sprite_renderer* user_handle = (neko_fast_sprite_renderer*)lua_touserdata(L, 1);
    return 0;
}

static int __neko_bind_particle_create(lua_State* L) {
    // const_str file_path = lua_tostring(L, 1);

    neko_particle_renderer* user_handle = (neko_particle_renderer*)lua_newuserdata(L, sizeof(neko_particle_renderer));
    memset(user_handle, 0, sizeof(neko_particle_renderer));

    neko_particle_renderer_construct(user_handle);

    return 1;
}

static int __neko_bind_particle_render(lua_State* L) {
    neko_particle_renderer* user_handle = (neko_particle_renderer*)lua_touserdata(L, 1);

    neko_vec2 xform = __neko_lua_table_to_v2(L, 2);

    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

    neko_particle_renderer_update(user_handle, neko_instance()->ctx.platform->time.elapsed);

    neko_particle_renderer_draw(user_handle, &g_cb, xform);
    return 0;
}

static int __neko_bind_particle_end(lua_State* L) {
    neko_particle_renderer* user_handle = (neko_particle_renderer*)lua_touserdata(L, 1);
    neko_particle_renderer_free(user_handle);
    return 0;
}

// 测试 luacstruct 用
struct CGameObject {
    int id;
    bool active;
    bool visible;
    bool selected;
};

template <>
struct neko::static_refl::neko_type_info<CGameObject> : neko_type_info_base<CGameObject> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            rf_field{TSTR("id"), &rf_type::id},              //
            rf_field{TSTR("active"), &rf_type::active},      //
            rf_field{TSTR("visible"), &rf_type::visible},    //
            rf_field{TSTR("selected"), &rf_type::selected},  //
    };
};

static int __neko_bind_gameobject_inspect(lua_State* L) {

    CGameObject* user_handle = (CGameObject*)lua_touserdata(L, 1);

    // neko_println("gameobj %d %s %s %s", user_handle->id, neko_bool_str(user_handle->active), neko_bool_str(user_handle->visible), neko_bool_str(user_handle->selected));

    if (ImGui::Begin("GameObject Inspector")) {

        ImGui::Text("GameObject_%d", user_handle->id);

        neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(*user_handle, [](auto field, auto&& value) { neko_imgui::Auto(value, std::string(field.name).c_str()); });
    }
    ImGui::End();

    return 0;
}

static int __neko_bind_assetsys_create(lua_State* L) {
    neko_assetsys_t* filewatch = (neko_assetsys_t*)lua_newuserdata(L, sizeof(neko_assetsys_t));
    memset(filewatch, 0, sizeof(neko_assetsys_t));
    neko_assetsys_create_internal(filewatch, NULL);

    return 1;
}

static int __neko_bind_assetsys_destory(lua_State* L) {
    neko_assetsys_t* assetsys = (neko_assetsys_t*)lua_touserdata(L, 1);
    neko_assetsys_destroy_internal(assetsys);
    return 0;
}

static int __neko_bind_filewatch_create(lua_State* L) {
    neko_assetsys_t* assetsys = (neko_assetsys_t*)lua_touserdata(L, 1);

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

void watch_map_callback(neko_filewatch_update_t change, const char* virtual_path, void* udata) {
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

    try {
        neko_lua_wrap_call<void>(L, callback_funcname, change_string, std::string(virtual_path));
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

// static int __neko_bind_filewatch_notify(lua_State* L) {
//     neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
//     neko_filewatch_notify(filewatch);
//     return 0;
// }

neko_inline void neko_register_test(lua_State* L) {

    //
    lua_register(L, "neko_aseprite_create", __neko_bind_aseprite_create);
    lua_register(L, "neko_aseprite_update", __neko_bind_aseprite_update);
    lua_register(L, "neko_aseprite_render", __neko_bind_aseprite_render);
    lua_register(L, "neko_aseprite_update_animation", __neko_bind_aseprite_update_animation);

    lua_register(L, "neko_sprite_create", __neko_bind_sprite_create);
    lua_register(L, "neko_sprite_end", __neko_bind_sprite_end);

    lua_register(L, "neko_tiled_create", __neko_bind_tiled_create);
    lua_register(L, "neko_tiled_render", __neko_bind_tiled_render);
    lua_register(L, "neko_tiled_end", __neko_bind_tiled_end);
    lua_register(L, "neko_tiled_load", __neko_bind_tiled_load);
    lua_register(L, "neko_tiled_unload", __neko_bind_tiled_unload);

    lua_register(L, "neko_fallingsand_create", __neko_bind_fallingsand_create);
    lua_register(L, "neko_fallingsand_update", __neko_bind_fallingsand_update);
    lua_register(L, "neko_fallingsand_end", __neko_bind_fallingsand_end);

    lua_register(L, "neko_gfxt_create", __neko_bind_gfxt_create);
    lua_register(L, "neko_gfxt_update", __neko_bind_gfxt_update);
    lua_register(L, "neko_gfxt_end", __neko_bind_gfxt_end);

    lua_register(L, "neko_draw_text", __neko_bind_draw_text);

    lua_register(L, "neko_custom_sprite_create", __neko_bind_custom_sprite_create);
    lua_register(L, "neko_custom_sprite_render", __neko_bind_custom_sprite_render);
    lua_register(L, "neko_custom_sprite_end", __neko_bind_custom_sprite_end);

    lua_register(L, "neko_particle_create", __neko_bind_particle_create);
    lua_register(L, "neko_particle_render", __neko_bind_particle_render);
    lua_register(L, "neko_particle_end", __neko_bind_particle_end);

    lua_register(L, "neko_gameobject_inspect", __neko_bind_gameobject_inspect);

    lua_register(L, "neko_assetsys_create", __neko_bind_assetsys_create);
    lua_register(L, "neko_assetsys_destory", __neko_bind_assetsys_destory);

    lua_register(L, "neko_filewatch_create", __neko_bind_filewatch_create);
    lua_register(L, "neko_filewatch_destory", __neko_bind_filewatch_destory);
    lua_register(L, "neko_filewatch_mount", __neko_bind_filewatch_mount);
    lua_register(L, "neko_filewatch_start", __neko_bind_filewatch_start_watch);
    lua_register(L, "neko_filewatch_stop", __neko_bind_filewatch_stop_watching);
    lua_register(L, "neko_filewatch_update", __neko_bind_filewatch_update);
    lua_register(L, "neko_filewatch_notify", __neko_bind_filewatch_notify);

    lua_register(L, "neko_callback_save", __neko_bind_callback_save);
    lua_register(L, "neko_callback_call", __neko_bind_callback_call);
}

#if 0

static int __neko_bind_graphics_fontcache_load(lua_State* L) {
    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_touserdata(L, 1);
    f64 font_size = lua_tonumber(L, 2);

    if (!assets_user_handle->size) {
        const_str error_message = "__neko_bind_graphics_fontcache_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    neko_font_index font_index = ((neko_instance()->ctx.graphics))->api.fontcache_load(assets_user_handle->data, assets_user_handle->size, font_size);

    lua_pushinteger(L, font_index);

    return 1;
}

static int __neko_bind_graphics_fontcache_set_default_font(lua_State* L) {
    neko_font_index font_index = lua_tointeger(L, 1);

    g_idraw.data->font_fc_default = font_index;

    return 0;
}

#endif

neko_inline void neko_register_pack(lua_State* L) {

    // lua_register(L, "neko_pack_construct", __neko_bind_pack_construct);
    // lua_register(L, "neko_pack_destroy", __neko_bind_pack_destroy);
    // lua_register(L, "neko_pack_assets_load", __neko_bind_pack_assets_load);
}

static int __neko_bind_cvar_new(lua_State* L) {
    const_str name = lua_tostring(L, 1);

    // 检查是否已经存在
    neko_cvar_t* cv = __neko_config_get(name);
    if (NULL != cv) {
        const_str error_message = "__neko_bind_cvar_new failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    neko_cvar_type cval = neko_cvar_type::__NEKO_CONFIG_TYPE_COUNT;

    if (lua_type(L, 2) == LUA_TSTRING) {
        const_str type = lua_tostring(L, 2);
        lua_pushstring(g_lua_bind, type);
        neko_lua_auto_to(g_lua_bind, neko_cvar_type, &cval, -1);
        lua_pop(g_lua_bind, 1);
    } else if (lua_isinteger(L, 2)) {
        int type = lua_tointeger(L, 2);
        cval = (neko_cvar_type)type;
    }

    switch (cval) {
        case __NEKO_CONFIG_TYPE_INT:
            neko_cvar_lnew(name, cval, neko_lua_to<int>(L, 3));
            break;
        case __NEKO_CONFIG_TYPE_FLOAT:
            neko_cvar_lnew(name, cval, neko_lua_to<float>(L, 3));
            break;
        case __NEKO_CONFIG_TYPE_STRING:
            neko_cvar_lnew_str(name, cval, neko_lua_to<const_str>(L, 3));
            break;
        case __NEKO_CONFIG_TYPE_COUNT:
        default:
            neko_log_warning(std::format("__neko_bind_cvar_new with a unknown type {0} {1}", name, (u8)cval).c_str());
            break;
    }
    return 0;
}

static int __neko_bind_cvar_get(lua_State* L) {
    const_str name = lua_tostring(L, 1);
    neko_cvar_t* cv = __neko_config_get(name);

    if (NULL == cv) {
        lua_pushnil(L);
        return 1;  // 如果不存在就返回nil
    }

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
}

static int __neko_bind_cvar_set(lua_State* L) {
    const_str name = lua_tostring(L, 1);

    // 检查是否存在
    neko_cvar_t* cv = __neko_config_get(name);
    if (NULL == cv) {
        const_str error_message = "__neko_bind_cvar_set failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    neko_cvar_set(cv, neko_lua_to<const_str>(L, 2));

    return 0;
}

static void __neko_lua_bug(const_str message) { neko_log_info("[lua] %s", message); }
static void __neko_lua_info(const_str message) { neko_log_info("[lua] %s", message); }
static void __neko_lua_trace(const_str message) { neko_log_info("[lua] %s", message); }
static void __neko_lua_error(const_str message) { neko_log_error("[lua] %s", message); }
static void __neko_lua_warn(const_str message) { neko_log_warning("[lua] %s", message); }

// returns table with pairs of path and isDirectory
static int __neko_ls(lua_State* L) {
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

static bool __neko_dolua(const_str file) { return neko_lua_wrap_do_file(g_lua_bind, game_assets(file)); }

// static void __neko_add_packagepath(const char* p) { neko_sc()->add_package_path(p); }

neko_inline void neko_register_common(lua_State* L) {

    neko_lua_wrap_register_t<>(L)
            .def(&__neko_lua_trace, "log_trace")
            .def(&__neko_lua_bug, "log_debug")
            .def(&__neko_lua_error, "log_error")
            .def(&__neko_lua_warn, "log_warn")
            .def(&__neko_lua_info, "log_info")
            .def(&neko_rand_xorshf32, "neko_rand")
            .def(&__neko_dolua, "neko_dolua");

    neko_lua_wrap_register_t<>(L).def(
            +[](const_str str) { return neko_hash_str64(str); }, "neko_hash");

    //.def(&__neko_add_packagepath, "add_packagepath")
    //.def(+[]() -> bool { return (bool)neko_is_debug(); }, "neko_is_debug");

    // neko_lua_auto_struct(L, neko_application_desc_t);
    // neko_lua_auto_struct_member(L, neko_application_desc_t, window_title, const char*);
    // neko_lua_auto_struct_member(L, neko_application_desc_t, window_width, unsigned int);
    // neko_lua_auto_struct_member(L, neko_application_desc_t, window_height, unsigned int);

    lua_pushstring(L, game_assets("gamedir").c_str());
    lua_setglobal(L, "neko_game_data_path");

    neko_lua_auto_struct(L, neko_vec2);
    neko_lua_auto_struct_member(L, neko_vec2, x, f32);
    neko_lua_auto_struct_member(L, neko_vec2, y, f32);
    neko_lua_auto_struct_member(L, neko_vec2, xy, f32[2]);

    neko_lua_auto_enum(L, neko_cvar_type);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_INT);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_FLOAT);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_STRING);
    neko_lua_auto_enum_value(L, neko_cvar_type, __NEKO_CONFIG_TYPE_COUNT);

    lua_register(L, "neko_cvar", __neko_bind_cvar_get);
    lua_register(L, "neko_cvar_new", __neko_bind_cvar_new);
    lua_register(L, "neko_cvar_set", __neko_bind_cvar_set);

    lua_register(L, "neko_ls", __neko_ls);
}

neko_inline void neko_register_graphics(lua_State* L) {

    // neko_lua_wrap_register_t<>(L).def(
    //         +[](const_str text, const neko_font_index font, const f32 x, const f32 y) { neko_graphics_fc_text(text, font, x, y); }, "neko_text");

    // lua_register(L, "neko_fontcache_load", __neko_bind_graphics_fontcache_load);
    // lua_register(L, "neko_fontcache_set_default_font", __neko_bind_graphics_fontcache_set_default_font);
}

// struct neko_lua_handle_wrap {
//     neko_lua_handle_wrap() {}
//     neko_lua_handle_wrap(neko_resource(neko_audio_source_t) as_audio_src) { save.audio_src = as_audio_src; }
//     neko_lua_handle_wrap(neko_resource(neko_audio_instance_t) as_audio_ins) { save.audio_ins = as_audio_ins; }
//     union {
//         neko_resource(neko_audio_source_t) audio_src;
//         neko_resource(neko_audio_instance_t) audio_ins;
//     } save;
// };

neko_inline void neko_register_audio(lua_State* L) {
    //
    // lua_register(L, "neko_audio_load", __neko_bind_audio_load);

    // lua["neko_lua_handle_wrap"]                                                                               //
    //         .setClass(lua_wrapper::UserdataMetatable<neko_lua_handle_wrap>()                                  //
    //                           .setConstructors<neko_lua_handle_wrap(),                                        //
    //                                            neko_lua_handle_wrap(neko_resource(neko_audio_source_t)),      //
    //                                            neko_lua_handle_wrap(neko_resource(neko_audio_instance_t))>()  //
    //                           .addProperty("save", &neko_lua_handle_wrap::save));                             //

    // lua["neko_audio_source_t"].setClass(
    //         lua_wrapper::UserdataMetatable<neko_resource(neko_audio_source_t)>().setConstructors<neko_resource(neko_audio_source_t)()>().addProperty("id", &neko_resource(neko_audio_source_t)::id));

    // lua["neko_audio_instance_t"].setClass(lua_wrapper::UserdataMetatable<neko_resource(neko_audio_instance_t)>().setConstructors<neko_resource(neko_audio_instance_t)()>().addProperty(
    //         "id", &neko_resource(neko_audio_instance_t)::id));

    // neko_audio_i* audio = neko_engine_subsystem(audio);

    // lua["neko_audio_load"] = [&audio](const_str path) -> neko_lua_handle_wrap { return audio->load_audio_source_from_file(path); };

    // lua["neko_audio_instance"] = [&audio](neko_lua_handle_wrap src, f32 volume) -> neko_lua_handle_wrap {
    //     neko_audio_instance_data_t inst = neko_audio_instance_data_new(src.save.audio_src);
    //     inst.volume = volume;    // range 0-1
    //     inst.loop = true;        // 是否循环
    //     inst.persistent = true;  // 实例完成后是否应保留在内存中 否则将从内存中清除
    //     return audio->construct_instance(inst);
    // };

    // lua["neko_audio_play"] = [&audio](neko_lua_handle_wrap inst) { audio->play(inst.save.audio_ins); };
}

class base_t {
public:
    base_t() : v(789) {}
    void dump() {
        // printf("in %s a:%dn", __FUNCTION__, v);
    }
    int v;
};

class foo_t : public base_t {
public:
    foo_t(int b) : a(b) {
        // printf("in %s b:%d this=%pn", __FUNCTION__, b, this);
    }

    ~foo_t() {
        // printf("in %sn", __FUNCTION__);
    }

    void print(int64_t a, base_t* p) const {
        // printf("in foo_t::print a:%ld p:%pn", (long)a, p);
    }

    static void dumy() {
        // printf("in %sn", __FUNCTION__);
    }
    int a;
};

//! lua talbe 可以自动转换为stl 对象
void dumy(std::map<std::string, std::string> ret, std::vector<int> a, std::list<std::string> b, std::set<int64_t> c) {
    printf("in %s begin ------------n", __FUNCTION__);
    for (std::map<std::string, std::string>::iterator it = ret.begin(); it != ret.end(); ++it) {
        printf("map:%s, val:%s:n", it->first.c_str(), it->second.c_str());
    }
    printf("in %s end ------------n", __FUNCTION__);
}

static void neko_register_test_oop(lua_State* L) {
    //! 注册基类函数, ctor() 为构造函数的类型
    neko_lua_wrap_register_t<base_t, void_ctor()>(L, "base_t")  //! 注册构造函数
            .def(&base_t::dump, "dump")                         //! 注册基类的函数
            .def(&base_t::v, "v");                              //! 注册基类的属性

    //! 注册子类，ctor(int) 为构造函数， foo_t为类型名称， base_t为继承的基类名称
    neko_lua_wrap_register_t<foo_t, void_ctor(int)>(L, "foo_t", "base_t")
            .def(&foo_t::print, "print")  //! 子类的函数
            .def(&foo_t::a, "a");         //! 子类的字段

    neko_lua_wrap_register_t<>(L).def(&dumy, "dumy");  //! 注册静态函数
                                                       /*支持注册基类
                                                       fflua_register_t<base_t, virtual_ctor()>(ls, "base_t");
                                                       */

    std::string test_script = R"(

function test_object(foo_obj)
    --测试构造
    base = base_t:new()
    -- 每个对象都有一个get_pointer获取指针
    -- print("base ptr:", base:get_pointer())
    -- 测试C++对象函数
    foo_obj:print(12333, base)
    base:delete()
    --基类的函数
    foo_obj:dump()
    -- 测试对象属性
    -- print("foo property", foo_obj.a)
    -- print("base property", foo_obj.v)
end

function test_ret_object(foo_obj)
    return foo_obj
end

function test_ret_base_object(foo_obj)
    return foo_obj
end

    )";

    neko_lua_wrap_run_string(L, test_script);

    foo_t* foo_ptr = new foo_t(456);
    neko_lua_wrap_call<void>(L, "test_object", foo_ptr);

    assert(foo_ptr == neko_lua_wrap_call<foo_t*>(L, "test_ret_object", foo_ptr));
    base_t* base_ptr = neko_lua_wrap_call<base_t*>(L, "test_ret_base_object", foo_ptr);
    assert(base_ptr == foo_ptr);
}

void __neko_bind_imgui(lua_State* L);

neko_inline void neko_register(lua_State* L) {

    g_lua_bind = L;

    neko_register_common(L);
    neko_register_platform(L);
    neko_register_pack(L);
    neko_register_graphics(L);
    neko_register_audio(L);
    neko_register_test(L);
    neko_register_test_oop(L);

    __neko_bind_imgui(L);
}

#endif  // !NEKO_BINDING_ENGINE_H
