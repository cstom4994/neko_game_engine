
#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/audio/neko_audio.h"
#include "engine/base/neko_engine.h"
#include "engine/filesystem/neko_packer.h"
#include "engine/graphics/neko_graphics.h"
#include "engine/platform/neko_platform.h"
#include "engine/scripting/neko_lua_base.h"
#include "engine/scripting/neko_lua_wrapper.hpp"

namespace neko {

neko_global lua_State* g_lua_bind;

static bool __neko_bind_platform_key_pressed(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_pressed(cval);
}

static bool __neko_bind_platform_was_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->was_key_down(cval);
}

static bool __neko_bind_platform_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_down(cval);
}

static bool __neko_bind_platform_key_released(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_released(cval);
}

static bool __neko_bind_platform_was_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->was_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_pressed(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->mouse_pressed(cval);
}

static bool __neko_bind_platform_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->mouse_down(cval);
}

static bool __neko_bind_platform_mouse_released(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->mouse_released(cval);
}

static int __neko_bind_platform_mouse_delta(lua_State* L) {
    neko_vec2 v2 = neko_engine_subsystem(platform)->mouse_delta();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_mouse_position(lua_State* L) {
    neko_vec2 v2 = neko_engine_subsystem(platform)->mouse_position();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_mouse_wheel(lua_State* L) {
    neko_vec2 v2 = neko_engine_subsystem(platform)->mouse_wheel();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_window_size(lua_State* L) {
    neko_resource_handle handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_engine_subsystem(platform)->window_size(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neko_bind_platform_frame_buffer_size(lua_State* L) {
    neko_resource_handle handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_engine_subsystem(platform)->frame_buffer_size(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

neko_inline void neko_register_platform(lua_wrapper::State& lua) {

    lua_State* L = lua.state();

    neko_lua_auto_enum(L, neko_platform_keycode);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_a);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_b);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_c);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_d);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_e);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_g);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_h);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_i);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_j);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_k);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_l);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_m);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_n);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_o);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_p);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_q);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_r);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_s);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_t);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_u);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_v);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_w);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_x);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_y);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_z);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lshift);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_rshift);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lalt);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_ralt);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lctrl);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_rctrl);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_bspace);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_bslash);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_qmark);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_tilde);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_comma);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_period);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_esc);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_space);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_left);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_up);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_right);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_down);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_zero);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_one);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_two);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_three);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_four);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_five);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_six);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_seven);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_eight);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_nine);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npzero);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npone);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_nptwo);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npthree);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npfour);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npfive);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npsix);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npseven);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npeight);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npnine);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_caps);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_delete);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_end);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f1);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f2);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f3);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f4);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f5);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f6);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f7);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f8);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f9);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f10);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f11);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f12);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_home);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_plus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_minus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lbracket);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_rbracket);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_semi_colon);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_enter);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_insert);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_pgup);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_pgdown);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_numlock);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_tab);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npmult);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npdiv);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npplus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npminus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npenter);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npdel);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_mute);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_volup);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_voldown);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_pause);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_print);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_count);

    neko_lua_auto_enum(L, neko_platform_mouse_button_code);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, neko_mouse_lbutton);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, neko_mouse_rbutton);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, neko_mouse_mbutton);
    neko_lua_auto_enum_value(L, neko_platform_mouse_button_code, neko_mouse_button_code_count);

    lua["neko_key_pressed"] = __neko_bind_platform_key_pressed;
    lua["neko_was_key_down"] = __neko_bind_platform_was_key_down;
    lua["neko_was_mouse_down"] = __neko_bind_platform_was_mouse_down;
    lua["neko_key_down"] = __neko_bind_platform_key_down;
    lua["neko_key_released"] = __neko_bind_platform_key_released;
    lua["neko_mouse_down"] = __neko_bind_platform_mouse_down;
    lua["neko_mouse_pressed"] = __neko_bind_platform_mouse_pressed;
    lua["neko_mouse_released"] = __neko_bind_platform_mouse_released;

    lua["neko_mouse_moved"] = +[]() -> bool { return neko_engine_subsystem(platform)->mouse_moved(); };

    lua["neko_file_path"] = +[](const_str path) -> neko_string { return neko_engine_subsystem(platform)->get_path(path); };

    lua["neko_create_window"] = +[](const_str title, u32 width, u32 height) -> neko_resource_handle { return neko_engine_subsystem(platform)->create_window(title, width, height); };

    lua["neko_main_window"] = +[]() -> neko_resource_handle { return neko_engine_subsystem(platform)->main_window(); };

    lua["neko_set_window_size"] = +[](neko_resource_handle handle, u32 width, u32 height) { neko_engine_subsystem(platform)->set_window_size(handle, width, height); };

    lua["neko_set_mouse_position"] = +[](neko_resource_handle handle, f64 x, f64 y) { neko_engine_subsystem(platform)->set_mouse_position(handle, x, y); };

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

    neko_debug("__neko_bind_pack_construct ", userdata_ptr->name);
    return 1;
}

static int __neko_bind_pack_destroy(lua_State* L) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);

    neko_packreader_t* pack = (neko_packreader_t*)(userdata_ptr->data);
    neko_pack_destroy(pack);

    neko_debug("__neko_bind_pack_destroy ", userdata_ptr->name);
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

    neko_debug("__neko_bind_pack_assets_load ", assets_user_handle->size);

    // 获取元表并设置
    luaL_newmetatable(L, "neko_lua_handle__assets");  // 供测试的元表
    lua_setmetatable(L, -2);

    return 1;
}

static int __neko_bind_graphics_fontcache_load(lua_State* L) {
    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_touserdata(L, 1);
    f64 font_size = lua_tonumber(L, 2);

    if (!assets_user_handle->size) {
        const_str error_message = "__neko_bind_graphics_fontcache_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    neko_font_index font_index = ((neko_engine_instance()->ctx.graphics))->fontcache_load(assets_user_handle->data, assets_user_handle->size, font_size);

    lua_pushinteger(L, font_index);

    return 1;
}

neko_inline void neko_register_pack(lua_wrapper::State& lua) {

    lua_State* L = lua.state();

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
        case neko::__NEKO_CONFIG_TYPE_INT:
            neko_cvar_lnew(name, cval, neko_lua_to<int>(L, 3));
            break;
        case neko::__NEKO_CONFIG_TYPE_FLOAT:
            neko_cvar_lnew(name, cval, neko_lua_to<float>(L, 3));
            break;
        case neko::__NEKO_CONFIG_TYPE_STRING:
            neko_cvar_lnew_str(name, cval, neko_lua_to<const_str>(L, 3));
            break;
        case neko::__NEKO_CONFIG_TYPE_COUNT:
        default:
            neko_warn(std::format("__neko_bind_cvar_new with a unknown type {0} {1}", name, (u8)cval));
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
        case neko::__NEKO_CONFIG_TYPE_INT:
            lua_pushinteger(L, cv->value.i);
            break;
        case neko::__NEKO_CONFIG_TYPE_FLOAT:
            lua_pushnumber(L, cv->value.f);
            break;
        case neko::__NEKO_CONFIG_TYPE_STRING:
            lua_pushstring(L, cv->value.s);
            break;
        case neko::__NEKO_CONFIG_TYPE_COUNT:
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

static void __neko_lua_bug(const_str message) { neko_debug("[lua] ", message); }
static void __neko_lua_info(const_str message) { neko_info("[lua] ", message); }
static void __neko_lua_trace(const_str message) { neko_trace("[lua] ", message); }
static void __neko_lua_error(const_str message) { neko_error("[lua] ", message); }
static void __neko_lua_warn(const_str message) { neko_warn("[lua] ", message); }

// returns table with pairs of path and isDirectory
static int __neko_ls(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        neko_warn("invalid lua argument");
        return 0;
    }
    auto string = lua_tostring(L, 1);
    if (!std::filesystem::is_directory(string)) {
        neko_warn(std::format("{0} is not directory", string));
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

static void __neko_add_packagepath(const char* p) {
    // neko_sc()->neko_lua.add_package_path(p);
}

neko_inline void neko_register_common(lua_wrapper::State& lua) {

    lua_State* L = lua.state();

    lua["log_trace"] = __neko_lua_trace;
    lua["log_debug"] = __neko_lua_bug;
    lua["log_error"] = __neko_lua_error;
    lua["log_warn"] = __neko_lua_warn;
    lua["log_info"] = __neko_lua_info;
    lua["add_packagepath"] = __neko_add_packagepath;

    lua["neko_is_debug"] = +[]() -> bool { return (bool)neko_is_debug(); };

    neko_lua_auto_struct(L, neko_application_desc_t);
    neko_lua_auto_struct_member(L, neko_application_desc_t, window_title, const char*);
    neko_lua_auto_struct_member(L, neko_application_desc_t, window_width, unsigned int);
    neko_lua_auto_struct_member(L, neko_application_desc_t, window_height, unsigned int);

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

neko_inline void neko_register_graphics(lua_wrapper::State& lua) {

    lua_State* L = lua.state();

    lua["neko_fontcache_push"] = +[](const_str text, const neko_font_index font, const f32 x, const f32 y) { ((neko_engine_instance()->ctx.graphics))->fontcache_push_x_y(text, font, x, y); };

    lua_register(L, "neko_fontcache_load", __neko_bind_graphics_fontcache_load);
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

neko_inline void neko_register_audio(lua_wrapper::State& lua) {
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

neko_inline void neko_register(lua_wrapper::State& lua) {

    g_lua_bind = lua.state();

    neko_register_common(lua);
    neko_register_platform(lua);
    neko_register_pack(lua);
    neko_register_graphics(lua);
    neko_register_audio(lua);
}

}  // namespace neko

#endif  // !NEKO_BINDING_ENGINE_H
