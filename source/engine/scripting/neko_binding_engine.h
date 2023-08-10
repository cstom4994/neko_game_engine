
#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/base/neko_engine.h"
#include "engine/filesystem/neko_packer.h"
#include "engine/platform/neko_platform.h"
#include "engine/scripting/neko_lua_base.h"

namespace neko {

neko_global lua_State* g_lua_bind;

static bool __neke_bind_platform_key_pressed(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_pressed(cval);
}

static bool __neke_bind_platform_was_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->was_key_down(cval);
}

static bool __neke_bind_platform_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_down(cval);
}

static bool __neke_bind_platform_key_released(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_released(cval);
}

static bool __neke_bind_platform_was_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->was_mouse_down(cval);
}

static bool __neke_bind_platform_mouse_pressed(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->mouse_pressed(cval);
}

static bool __neke_bind_platform_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->mouse_down(cval);
}

static bool __neke_bind_platform_mouse_released(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->mouse_released(cval);
}

static void __neke_bind_platform_set_mouse_position(neko_resource_handle handle, f64 x, f64 y) {
    // 封装 应该有更好的方法
    return ((neko_engine_instance()->ctx.platform))->set_mouse_position(handle, x, y);
}

static int __neke_bind_platform_mouse_delta(lua_State* L) {
    neko_vec2 v2 = ((neko_engine_instance()->ctx.platform))->mouse_delta();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neke_bind_platform_mouse_position(lua_State* L) {
    neko_vec2 v2 = ((neko_engine_instance()->ctx.platform))->mouse_position();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neke_bind_platform_mouse_wheel(lua_State* L) {
    neko_vec2 v2 = ((neko_engine_instance()->ctx.platform))->mouse_wheel();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neke_bind_platform_window_size(lua_State* L) {
    neko_resource_handle handle = lua_tointeger(L, -1);
    neko_vec2 v2 = ((neko_engine_instance()->ctx.platform))->window_size(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static int __neke_bind_platform_frame_buffer_size(lua_State* L) {
    neko_resource_handle handle = lua_tointeger(L, -1);
    neko_vec2 v2 = ((neko_engine_instance()->ctx.platform))->frame_buffer_size(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

static bool __neke_bind_platform_mouse_moved() {
    // 封装 应该有更好的方法
    return ((neko_engine_instance()->ctx.platform))->mouse_moved();
}

static neko_string __neke_bind_platform_file_path(const_str path) {
    // 封装 应该有更好的方法
    return ((neko_engine_instance()->ctx.platform))->get_path(path);
}

static neko_resource_handle __neke_bind_platform_create_window(const char* title, u32 width, u32 height) {
    // 封装 应该有更好的方法
    return ((neko_engine_instance()->ctx.platform))->create_window(title, width, height);
}

static neko_resource_handle __neke_bind_platform_main_window() {
    // 封装 应该有更好的方法
    return ((neko_engine_instance()->ctx.platform))->main_window();
}

static void __neke_bind_platform_set_window_size(neko_resource_handle handle, s32 width, s32 height) {
    // 封装 应该有更好的方法
    return ((neko_engine_instance()->ctx.platform))->set_window_size(handle, width, height);
}

neko_inline void neko_register_platform(lua_State* L) {

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

    neko_lua_register_t<>(L)
            .def(&__neke_bind_platform_key_pressed, "neko_key_pressed")                // key
            .def(&__neke_bind_platform_was_key_down, "neko_was_key_down")              //
            .def(&__neke_bind_platform_key_down, "neko_key_down")                      //
            .def(&__neke_bind_platform_key_released, "neko_key_released")              //
            .def(&__neke_bind_platform_was_mouse_down, "neko_was_mouse_down")          //
            .def(&__neke_bind_platform_mouse_down, "neko_mouse_down")                  //
            .def(&__neke_bind_platform_mouse_pressed, "neko_mouse_pressed")            //
            .def(&__neke_bind_platform_mouse_released, "neko_mouse_released")          //
            .def(&__neke_bind_platform_set_mouse_position, "neko_set_mouse_position")  //
            .def(&__neke_bind_platform_mouse_moved, "neko_mouse_moved")                //

            .def(&__neke_bind_platform_file_path, "neko_file_path")  // file path

            .def(&__neke_bind_platform_create_window, "neko_create_window")       // window
            .def(&__neke_bind_platform_main_window, "neko_main_window")           // window
            .def(&__neke_bind_platform_set_window_size, "neko_set_window_size");  // window

    lua_register(L, "neko_mouse_delta", __neke_bind_platform_mouse_delta);
    lua_register(L, "neko_mouse_position", __neke_bind_platform_mouse_position);
    lua_register(L, "neko_mouse_wheel", __neke_bind_platform_mouse_wheel);

    lua_register(L, "neko_window_size", __neke_bind_platform_window_size);
    lua_register(L, "neko_frame_buffer_size", __neke_bind_platform_frame_buffer_size);
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
    luaL_newmetatable(L, "test_pack_handle");  // 供测试的元表
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

neko_inline void neko_register_pack(lua_State* L) {
    lua_register(L, "neko_pack_construct", __neko_bind_pack_construct);
    lua_register(L, "neko_pack_destroy", __neko_bind_pack_destroy);
}

static int __neko_bind_cvar_new(lua_State* L) {
    const_str name = lua_tostring(L, 1);
    const_str type = lua_tostring(L, 2);

    // 检查是否已经存在
    neko_cvar_t* cv = __neko_config_get(name);
    if (NULL != cv) {
        const_str error_message = "__neko_bind_cvar_new failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    neko_cvar_type cval;
    lua_pushstring(g_lua_bind, type);
    neko_lua_auto_to(g_lua_bind, neko_cvar_type, &cval, -1);
    lua_pop(g_lua_bind, 1);

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
            neko_warn(std::format("__neko_bind_cvar_new with a unknown type {0} {1} {2}", name, type, (u8)cval));
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

neko_inline void neko_register_common(lua_State* L) {

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
}

neko_inline void neko_register(lua_State* L) {
    g_lua_bind = L;

    neko_register_common(L);
    neko_register_platform(L);
    neko_register_pack(L);
}

}  // namespace neko

#endif  // !NEKO_BINDING_ENGINE_H
