
#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/neko_cvar.h"
#include "engine/neko_engine.h"
#include "engine/util/neko_asset.h"
#include "engine/util/neko_lua.h"

neko_global lua_State* g_lua_bind;

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

    lua_register(L, "neko_pack_construct", __neko_bind_pack_construct);
    lua_register(L, "neko_pack_destroy", __neko_bind_pack_destroy);
    lua_register(L, "neko_pack_assets_load", __neko_bind_pack_assets_load);
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

static bool __neko_dolua(const_str file) { return neko_lua_wrap_do_file(g_lua_bind, __neko_game_get_path(file)); }

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

    //.def(&__neko_add_packagepath, "add_packagepath")
    //.def(+[]() -> bool { return (bool)neko_is_debug(); }, "neko_is_debug");

    // neko_lua_auto_struct(L, neko_application_desc_t);
    // neko_lua_auto_struct_member(L, neko_application_desc_t, window_title, const char*);
    // neko_lua_auto_struct_member(L, neko_application_desc_t, window_width, unsigned int);
    // neko_lua_auto_struct_member(L, neko_application_desc_t, window_height, unsigned int);

    lua_pushstring(L, __neko_game_get_path("data").c_str());
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

neko_inline void neko_register(lua_State* L) {

    g_lua_bind = L;

    neko_register_common(L);
    neko_register_platform(L);
    neko_register_pack(L);
    neko_register_graphics(L);
    neko_register_audio(L);
}

#endif  // !NEKO_BINDING_ENGINE_H
