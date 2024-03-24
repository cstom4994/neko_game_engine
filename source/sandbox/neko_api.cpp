

#include "neko_api.h"

#include <filesystem>
#include <string>

#include "engine/builtin/cute_spritebatch.h"
#include "engine/builtin/neko_fs.h"
#include "engine/builtin/neko_png.h"
#include "engine/neko.h"
#include "engine/neko_engine.h"
#include "engine/util/neko_asset.h"
#include "engine/util/neko_gfxt.h"
#include "engine/util/neko_lua.hpp"
#include "engine/util/neko_sprite.h"
#include "engine/util/neko_tiled.h"

// game
#include "sandbox/game_chunk.h"
#include "sandbox/neko_gui_auto.hpp"
#include "sandbox/neko_profiler.h"

// hpp
#include "sandbox/hpp/neko_static_refl.hpp"

#ifndef IS_HTML5
#include "engine/embed/ltn12_compressed.h"
#include "engine/embed/mbox_compressed.h"
#include "engine/embed/mime_compressed.h"
#include "engine/embed/socket_compressed.h"
#include "engine/embed/socket_ftp_compressed.h"
#include "engine/embed/socket_headers_compressed.h"
#include "engine/embed/socket_http_compressed.h"
#include "engine/embed/socket_smtp_compressed.h"
#include "engine/embed/socket_tp_compressed.h"
#include "engine/embed/socket_url_compressed.h"
#endif

extern neko_client_userdata_t g_client_userdata;

extern "C" int luaopen_socket_core(lua_State* L);
extern "C" int luaopen_mime_core(lua_State* L);
extern "C" int luaopen_neko_gui(lua_State* L);
extern "C" int luaopen_cffi(lua_State* L);
extern "C" int __neko_ecs_create_world(lua_State* L);
extern "C" int luaopen_imgui(lua_State* L);

extern int register_mt_imgui(lua_State* L);

#define LUAOPEN_EMBED_DATA(func, name, compressed_data, compressed_size)              \
    static int func(lua_State* L) {                                                   \
        s32 top = lua_gettop(L);                                                      \
                                                                                      \
        neko_string contents = stb_decompress_data(compressed_data, compressed_size); \
        neko_defer({ neko_safe_free(contents.data); });                               \
                                                                                      \
        if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {        \
            luaL_error(L, "%s", lua_tostring(L, -1));                                 \
            return 0;                                                                 \
        }                                                                             \
                                                                                      \
        if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {                              \
            luaL_error(L, "%s", lua_tostring(L, -1));                                 \
            return 0;                                                                 \
        }                                                                             \
                                                                                      \
        return lua_gettop(L) - top;                                                   \
    }

LUAOPEN_EMBED_DATA(open_embed_ltn12, "ltn12.lua", ltn12_compressed_data, ltn12_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_mbox, "mbox.lua", mbox_compressed_data, mbox_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_mime, "mime.lua", mime_compressed_data, mime_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket, "socket.lua", socket_compressed_data, socket_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_ftp, "socket.ftp.lua", socket_ftp_compressed_data, socket_ftp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_headers, "socket.headers.lua", socket_headers_compressed_data, socket_headers_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_http, "socket.http.lua", socket_http_compressed_data, socket_http_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_smtp, "socket.smtp.lua", socket_smtp_compressed_data, socket_smtp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_tp, "socket.tp.lua", socket_tp_compressed_data, socket_tp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_url, "socket.url.lua", socket_url_compressed_data, socket_url_compressed_size);

static void package_preload(lua_State* L, const char* name, lua_CFunction function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
}

void open_luasocket(lua_State* L) {
    package_preload(L, "socket.core", luaopen_socket_core);
    package_preload(L, "mime.core", luaopen_mime_core);

    package_preload(L, "ltn12", open_embed_ltn12);
    package_preload(L, "mbox", open_embed_mbox);
    package_preload(L, "mime", open_embed_mime);
    package_preload(L, "socket", open_embed_socket);
    package_preload(L, "socket.ftp", open_embed_socket_ftp);
    package_preload(L, "socket.headers", open_embed_socket_headers);
    package_preload(L, "socket.http", open_embed_socket_http);
    package_preload(L, "socket.smtp", open_embed_socket_smtp);
    package_preload(L, "socket.tp", open_embed_socket_tp);
    package_preload(L, "socket.url", open_embed_socket_url);
}

int neko_lua_add_package_path(lua_State* L, const std::string& str_) {
    std::string new_path = "package.path = package.path .. \"";
    if (str_.empty()) return -1;
    if (str_[0] != ';') new_path += ";";
    new_path += str_;
    if (str_[str_.length() - 1] != '/') new_path += "/";
    new_path += "?.lua\" ";
    neko_lua_wrap_run_string(L, new_path);
    return 0;
}

void __neko_lua_print_error(lua_State* state, int result) {
    const char* message = lua_tostring(state, -1);
    neko_log_error("LuaScript ERROR:\n  %s", (message ? message : "no message"));

    if (result != 0) {
        switch (result) {
            case LUA_ERRRUN:
                neko_log_error("%s", "Lua Runtime error");
                break;
            case LUA_ERRSYNTAX:
                neko_log_error("%s", "Lua syntax error");
                break;
            case LUA_ERRMEM:
                neko_log_error("%s", "Lua was unable to allocate the required memory");
                break;
            case LUA_ERRFILE:
                neko_log_error("%s", "Lua was unable to find boot file");
                break;
            default:
                neko_log_error("Unknown lua error: %d", result);
        }
    }

    lua_pop(state, 1);
}

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

neko_inline neko_color_t __neko_lua_table_to_color(lua_State* L, int idx) {
    if (!lua_istable(L, idx)) {
        neko_log_warning("not a table on top");
        return neko_default_val();
    }

    lua_pushstring(L, "r");
    lua_gettable(L, idx);
    u8 r = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "g");
    lua_gettable(L, idx);
    u8 g = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "b");
    lua_gettable(L, idx);
    u8 b = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "a");
    lua_gettable(L, idx);
    u8 a = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return neko_color(r, g, b, a);
}

static int g_lua_callbacks_table_ref = LUA_NOREF;

static int __neko_bind_callback_save(lua_State* L) {
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

static int __neko_bind_callback_call(lua_State* L) {
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

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_keycode, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_key_pressed(cval);
}

static bool __neko_bind_platform_was_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_keycode, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_key_down(cval);
}

static bool __neko_bind_platform_key_down(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_keycode, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_key_down(cval);
}

static bool __neko_bind_platform_key_released(const_str key) {
    neko_platform_keycode cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_keycode, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_key_released(cval);
}

static bool __neko_bind_platform_was_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_was_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_pressed(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_mouse_pressed(cval);
}

static bool __neko_bind_platform_mouse_down(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

    return neko_platform_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_released(const_str key) {
    neko_platform_mouse_button_code cval;

    lua_pushstring(g_client_userdata.L, key);
    neko_lua_auto_to(g_client_userdata.L, neko_platform_mouse_button_code, &cval, -1);
    lua_pop(g_client_userdata.L, 1);

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

    //.def(+[](const_str path) -> std::string { return neko_engine_subsystem(platform)->get_path(path); }, "neko_file_path")
    //.def(+[](const_str title, u32 width, u32 height) -> neko_resource_handle { return neko_engine_subsystem(platform)->create_window(title, width, height); }, "neko_create_window")

    lua_register(L, "neko_mouse_delta", __neko_bind_platform_mouse_delta);
    lua_register(L, "neko_mouse_position", __neko_bind_platform_mouse_position);
    lua_register(L, "neko_mouse_wheel", __neko_bind_platform_mouse_wheel);

    lua_register(L, "neko_window_size", __neko_bind_platform_window_size);
    lua_register(L, "neko_framebuffer_size", __neko_bind_platform_framebuffer_size);
    lua_register(L, "neko_set_window_title", __neko_bind_platform_set_window_title);
}

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

    neko_pack_result result = neko_pack_read(path, 0, false, (neko_packreader_t*)userdata_ptr->data);

    if (!neko_pack_check(result)) {
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

    const char** item_paths = (const char**)neko_safe_malloc(n * sizeof(const char*));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);                   // 将index=i的元素压入堆栈顶部
        const char* str = lua_tostring(L, -1);  // # -1
        if (str != NULL) {
            item_paths[i - 1] = str;
        }
        lua_pop(L, 1);  // # -1
    }

    neko_pack_result result = neko_pack_build(path, n, item_paths, true);

    neko_safe_free(item_paths);

    if (!neko_pack_check(result)) {
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

    neko_pack_result result = neko_pack_info(path, &pack_version, &is_little_endian, &item_count);

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

    neko_pack_result result = neko_pack_item_data(pack, path, (const u8**)&assets_user_handle->data, (u32*)&assets_user_handle->size);

    if (!neko_pack_check(result)) {
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
        return luaL_error(L, "Function expects exactly three arguments");
    }

    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    neko_vec2 xform = __neko_lua_table_to_v2(L, 2);

    int direction = neko_lua_to<int>(L, 3);
    f32 scale = neko_lua_to<f32>(L, 4);

    neko_t* engine = neko_instance();

    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

    s32 index;
    if (user_handle->loop) {
        index = user_handle->loop->indices[user_handle->current_frame];
    } else {
        index = user_handle->current_frame;
    }

    neko_aseprite* spr = user_handle->sprite;
    neko_aseprite_frame f = spr->frames[index];

    if (direction)
        neko_idraw_rect_textured_ext(g_client_userdata.idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u1, f.v0, f.u0, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);
    else
        neko_idraw_rect_textured_ext(g_client_userdata.idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u0, f.v0, f.u1, f.v1, user_handle->sprite->img.id,
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

    neko_tiled_render_init(g_client_userdata.cb, user_handle, glsl_vs_src, glsl_fs_src);

    return 1;
}

static int __neko_bind_tiled_render(lua_State* L) {
    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)lua_touserdata(L, 1);

    neko_renderpass_t rp = NEKO_GRAPHICS_RENDER_PASS_DEFAULT;
    neko_lua_auto_struct_to_member(L, neko_renderpass_t, id, &rp, 2);

    neko_vec2 xform = __neko_lua_table_to_v2(L, 3);

    f32 l = lua_tonumber(L, 4);
    f32 r = lua_tonumber(L, 5);
    f32 t = lua_tonumber(L, 6);
    f32 b = lua_tonumber(L, 7);

    tiled_render->camera_mat = neko_mat4_ortho(l, r, b, t, -1.0f, 1.0f);

    neko_command_buffer_t* cb = g_client_userdata.cb;

    neko_graphics_renderpass_begin(cb, rp);
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

    neko_command_buffer_t* cb = g_client_userdata.cb;

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

    draw_text(g_client_userdata.test_font_bmfont, text, x, y, 1, 1.f, 800.f, scale);

    return 0;
}

// clang-format off
const_str image_names[] = {
        "gamedir/assets/textures/dragon_zombie.png",
        "gamedir/assets/textures/night_spirit.png",
};
// clang-format on

int images_count = sizeof(image_names) / sizeof(*image_names);
neko_png_image_t images[sizeof(image_names) / sizeof(*image_names)];

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

    spritebatch_t sb;

    neko_graphics_batch_context_t* sprite_batch;
    neko_graphics_batch_shader_t sprite_shader;
    neko_graphics_batch_renderable_t sprite_renderable;
    f32 sprite_projection[16];

    int call_count = 0;

    int sprite_verts_count;
    vertex_t sprite_verts[SPRITE_VERTS_MAX];
};

spritebatch_config_t get_demo_config() {
    spritebatch_config_t config;
    spritebatch_set_default_config(&config);
    config.pixel_stride = sizeof(u8) * 4;
    config.atlas_width_in_pixels = 1024;
    config.atlas_height_in_pixels = 1024;
    config.atlas_use_border_pixels = 0;
    config.ticks_to_decay_texture = 3;
    config.lonely_buffer_count_till_flush = 1;
    config.ratio_to_decay_atlas = 0.5f;
    config.ratio_to_merge_atlases = 0.25f;
    config.allocator_context = 0;
    return config;
}

void make_sprite(neko_sprite_t* sprite, NEKO_SPRITEBATCH_U64 image_id, f32 x, f32 y, f32 scale, f32 angle_radians, int depth) {

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / scale / 2.f;

    sprite->image_id = image_id;
    sprite->depth = depth;
    sprite->x = x0;
    sprite->y = y0;
    sprite->sx = (f32)images[sprite->image_id].w * 2.0f * scale;
    sprite->sy = (f32)images[sprite->image_id].h * 2.0f * scale;
    sprite->c = cosf(angle_radians);
    sprite->s = sinf(angle_radians);
}

void push_sprite(neko_sprite_batch_t* user_handle, neko_sprite_t* sp) {
    spritebatch_sprite_t s;
    s.image_id = sp->image_id;
    s.w = images[sp->image_id].w;
    s.h = images[sp->image_id].h;
    s.x = sp->x;
    s.y = sp->y;
    s.sx = sp->sx;
    s.sy = sp->sy;
    s.c = sp->c;
    s.s = sp->s;
    s.sort_bits = (NEKO_SPRITEBATCH_U64)sp->depth;
    spritebatch_push(&user_handle->sb, s);
}

// callbacks for cute_spritebatch.h
void batch_report(spritebatch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata) {

    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)udata;

    ++user_handle->call_count;

    (void)udata;
    (void)texture_w;
    (void)texture_h;
    // printf("begin batch\n");
    // for (int i = 0; i < count; ++i) printf("\t%llu\n", sprites[i].texture_id);
    // printf("end batch\n");

    // build the draw call
    neko_graphics_batch_draw_call_t call;
    call.r = &user_handle->sprite_renderable;
    call.textures[0] = (u32)sprites[0].texture_id;
    call.texture_count = 1;

    // set texture uniform in shader
    neko_graphics_batch_send_texture(call.r->program, "u_sprite_texture", 0);

    // NOTE:
    // perform any additional sorting here

    // build vertex buffer of quads from all sprite transforms
    call.verts = user_handle->sprite_verts + user_handle->sprite_verts_count;
    call.vert_count = count * 6;
    user_handle->sprite_verts_count += call.vert_count;
    assert(user_handle->sprite_verts_count < SPRITE_VERTS_MAX);

    vertex_t* verts = (vertex_t*)call.verts;
    for (int i = 0; i < count; ++i) {
        spritebatch_sprite_t* s = sprites + i;

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

    // submit call to cute_gl (does not get flushed to screen until `gl_flush` is called)
    neko_graphics_batch_push_draw_call(user_handle->sprite_batch, call);
}

void get_pixels(NEKO_SPRITEBATCH_U64 image_id, void* buffer, int bytes_to_fill, void* udata) {
    (void)udata;
    memcpy(buffer, images[image_id].pix, bytes_to_fill);
}

void load_images() {
    for (s32 i = 0; i < images_count; ++i) images[i] = neko_png_load(game_assets(image_names[i]).c_str());
}

void unload_images() {
    for (s32 i = 0; i < images_count; ++i) neko_png_free(images + i);
}

static int __neko_bind_sprite_batch_create(lua_State* L) {
    const_str glsl_vs_src = lua_tostring(L, 1);
    const_str glsl_ps_src = lua_tostring(L, 2);

    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_newuserdata(L, sizeof(neko_sprite_batch_t));
    memset(user_handle, 0, sizeof(neko_sprite_batch_t));

    user_handle->sprite_batch = neko_graphics_batch_make_ctx(32);

    // g_client_userdata.sprite_batch = sprite_batch;

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    neko_graphics_batch_vertex_data_t vd;
    neko_graphics_batch_make_vertex_data(&vd, 1024 * 1024, GL_TRIANGLES, sizeof(vertex_t), GL_DYNAMIC_DRAW);
    neko_graphics_batch_add_attribute(&vd, "in_pos", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(vertex_t, x));
    neko_graphics_batch_add_attribute(&vd, "in_uv", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(vertex_t, u));

    neko_graphics_batch_make_renderable(&user_handle->sprite_renderable, &vd);
    neko_graphics_batch_load_shader(&user_handle->sprite_shader, glsl_vs_src, glsl_ps_src);
    neko_graphics_batch_set_shader(&user_handle->sprite_renderable, &user_handle->sprite_shader);

    neko_graphics_batch_ortho_2d(fbs.x, fbs.y, 0, 0, user_handle->sprite_projection);

    neko_graphics_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

    load_images();

    spritebatch_config_t sb_config = get_demo_config();

    sb_config.batch_callback = batch_report;                        // report batches of sprites from `spritebatch_flush`
    sb_config.get_pixels_callback = get_pixels;                     // used to retrieve image pixels from `spritebatch_flush` and `spritebatch_defrag`
    sb_config.generate_texture_callback = generate_texture_handle;  // used to generate a texture handle from `spritebatch_flush` and `spritebatch_defrag`
    sb_config.delete_texture_callback = destroy_texture_handle;     // used to destroy a texture handle from `spritebatch_defrag`

    spritebatch_init(&user_handle->sb, &sb_config, user_handle);

    return 1;
}

static int __neko_bind_sprite_batch_render_ortho(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    f32 w = lua_tonumber(L, 2);
    f32 h = lua_tonumber(L, 3);
    f32 x = lua_tonumber(L, 4);
    f32 y = lua_tonumber(L, 5);

    neko_graphics_batch_ortho_2d(w, h, x / w, y / h, user_handle->sprite_projection);

    neko_graphics_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

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

    make_sprite(sprite_handle, image_id, x, y, scale, angle_radians, depth);

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
        neko_profiler_scope_auto("render_spritebatch");
        spritebatch_defrag(&user_handle->sb);
        spritebatch_tick(&user_handle->sb);
        spritebatch_flush(&user_handle->sb);
        user_handle->sprite_verts_count = 0;
    }

    neko_graphics_draw_batch(g_client_userdata.cb, user_handle->sprite_batch, 0, 0, 0);

    return 0;
}

static int __neko_bind_sprite_batch_end(lua_State* L) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    spritebatch_term(&user_handle->sb);
    unload_images();

    neko_graphics_batch_free(user_handle->sprite_batch);

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
    neko_fast_sprite_renderer_draw(user_handle, g_client_userdata.cb);
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

    neko_particle_renderer_draw(user_handle, g_client_userdata.cb, xform);
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

    if (user_handle == NULL) return 0;

    // neko_println("gameobj %d %s %s %s", user_handle->id, neko_bool_str(user_handle->active), neko_bool_str(user_handle->visible), neko_bool_str(user_handle->selected));

    ImGui::Text("GameObject_%d", user_handle->id);

    auto f = [](auto&& field, auto&& value) { neko_imgui::Auto(std::forward<decltype(value)&>(value), std::string(field.name).c_str()); };

    neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf((*user_handle), f);

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

static void watch_map_callback(neko_filewatch_update_t change, const char* virtual_path, void* udata) {
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

    lua_getglobal(g_client_userdata.L, callback_funcname);
    bool is_callback = lua_isfunction(g_client_userdata.L, -1);
    lua_pop(g_client_userdata.L, 1);

    if (is_callback) try {
            neko_lua_wrap_call<void>(g_client_userdata.L, callback_funcname, change_string, std::string(virtual_path));
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

static int __neko_bind_idraw_get(lua_State* L) {
    lua_pushlightuserdata(L, g_client_userdata.idraw);
    return 1;
}

static int __neko_bind_idraw_draw(lua_State* L) {
    neko_idraw_draw(g_client_userdata.idraw, g_client_userdata.cb);
    return 1;
}

static int __neko_bind_idraw_defaults(lua_State* L) {
    neko_idraw_defaults(g_client_userdata.idraw);
    return 0;
}

static int __neko_bind_idraw_camera2d(lua_State* L) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera2d(g_client_userdata.idraw, w, h);
    return 0;
}

static int __neko_bind_idraw_camera3d(lua_State* L) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera3d(g_client_userdata.idraw, w, h);
    return 0;
}

static int __neko_bind_idraw_camera2d_ex(lua_State* L) {
    f32 l = lua_tonumber(L, 1);
    f32 r = lua_tonumber(L, 2);
    f32 t = lua_tonumber(L, 3);
    f32 b = lua_tonumber(L, 4);
    neko_idraw_camera2d_ex(g_client_userdata.idraw, l, r, t, b);
    return 0;
}

static int __neko_bind_idraw_rotatev(lua_State* L) {
    f32 angle = lua_tonumber(L, 1);
    f32 x = lua_tonumber(L, 2);
    f32 y = lua_tonumber(L, 3);
    f32 z = lua_tonumber(L, 4);
    neko_idraw_rotatev(g_client_userdata.idraw, angle, neko_v3(x, y, z));
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
    neko_graphics_primitive_type type_val;
    neko_lua_auto_to(g_client_userdata.L, neko_graphics_primitive_type, &type_val, 11);
    neko_idraw_box(g_client_userdata.idraw, x, y, z, hx, hy, hz, r, g, b, a, type_val);
    return 0;
}

static int __neko_bind_idraw_translatef(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    neko_idraw_translatef(g_client_userdata.idraw, x, y, z);
    return 0;
}

static int __neko_bind_idraw_rectv(lua_State* L) {

    neko_vec2 v1 = __neko_lua_table_to_v2(L, 1);
    neko_vec2 v2 = __neko_lua_table_to_v2(L, 2);

    v2 = neko_vec2_add(v1, v2);

    neko_graphics_primitive_type type_val;
    neko_lua_auto_to(g_client_userdata.L, neko_graphics_primitive_type, &type_val, 3);

    neko_color_t col = NEKO_COLOR_WHITE;

    if (lua_gettop(L) == 4) {
        col = __neko_lua_table_to_color(L, 4);
    }

    neko_idraw_rectv(g_client_userdata.idraw, v1, v2, col, type_val);
    return 0;
}

static int __neko_bind_idraw_rectvd(lua_State* L) {

    neko_vec2 v1 = __neko_lua_table_to_v2(L, 1);
    neko_vec2 v2 = __neko_lua_table_to_v2(L, 2);

    neko_vec2 uv0 = __neko_lua_table_to_v2(L, 3);
    neko_vec2 uv1 = __neko_lua_table_to_v2(L, 4);

    neko_graphics_primitive_type type_val;
    neko_lua_auto_to(g_client_userdata.L, neko_graphics_primitive_type, &type_val, 5);

    neko_color_t col = __neko_lua_table_to_color(L, 6);

    neko_idraw_rectvd(g_client_userdata.idraw, v1, v2, uv0, uv1, col, type_val);
    return 0;
}

static int __neko_bind_idraw_text(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);
    neko_idraw_text(g_client_userdata.idraw, x, y, text, NULL, false, 255, 50, 50, 255);
    return 0;
}

static int __neko_bind_idraw_camera(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    neko_camera_t camera;
    camera = neko_camera_default();
    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_camera(g_client_userdata.idraw, &camera, (u32)fbs.x, (u32)fbs.y);
    return 0;
}

static int __neko_bind_idraw_depth_enabled(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_depth_enabled(g_client_userdata.idraw, enable);
    return 0;
}

static int __neko_bind_idraw_face_cull_enabled(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_face_cull_enabled(g_client_userdata.idraw, enable);
    return 0;
}

static int __neko_bind_idraw_texture(lua_State* L) {
    neko_texture_t rt = neko_default_val();
    neko_lua_auto_struct_to_member(g_client_userdata.L, neko_texture_t, id, &rt, 1);
    neko_idraw_texture(g_client_userdata.idraw, rt);
    return 0;
}

static int __neko_bind_audio_load(lua_State* L) {
    const_str file = lua_tostring(L, 1);
    neko_sound_audio_source_t* audio_src = neko_sound_load_wav(file, NULL);
    lua_pushlightuserdata(L, audio_src);
    return 1;
}

static int __neko_bind_audio_unload(lua_State* L) {
    neko_sound_audio_source_t* audio = (neko_sound_audio_source_t*)lua_touserdata(L, 1);
    neko_sound_free_audio_source(audio);
    return 0;
}

static int __neko_bind_audio_play(lua_State* L) {
    neko_sound_audio_source_t* audio_src = (neko_sound_audio_source_t*)lua_touserdata(L, 1);
    neko_sound_params_t params = neko_sound_params_default();
    neko_sound_playing_sound_t pl = neko_sound_play_sound(audio_src, params);
    neko_lua_auto_struct_push_member(L, neko_sound_playing_sound_t, id, &pl);
    return 1;
}

static int __neko_bind_graphics_framebuffer_create(lua_State* L) {
    neko_framebuffer_t fbo = neko_default_val();
    fbo = neko_graphics_framebuffer_create(NULL);
    neko_lua_auto_struct_push_member(L, neko_framebuffer_t, id, &fbo);
    return 1;
}

static int __neko_bind_graphics_framebuffer_destroy(lua_State* L) {
    neko_framebuffer_t fbo = neko_default_val();
    neko_lua_auto_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);
    neko_graphics_framebuffer_destroy(fbo);
    return 0;
}

static int __neko_bind_graphics_texture_create(lua_State* L) {
    u32 w = lua_tointeger(L, 1);
    u32 h = lua_tointeger(L, 2);
    neko_texture_t rt = neko_default_val();
    rt = neko_graphics_texture_create(neko_c_ref(neko_graphics_texture_desc_t,
                                                 {
                                                         .width = w,                                         // Width of texture in pixels
                                                         .height = h,                                        // Height of texture in pixels
                                                         .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,       // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
                                                         .wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,        // Wrapping type for s axis of texture
                                                         .wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,        // Wrapping type for t axis of texture
                                                         .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR,  // Minification filter for texture
                                                         .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR   // Magnification filter for texture
                                                 }));
    neko_lua_auto_struct_push_member(L, neko_texture_t, id, &rt);
    return 1;
}

static int __neko_bind_graphics_texture_destroy(lua_State* L) {
    neko_texture_t rt = neko_default_val();
    neko_lua_auto_struct_to_member(L, neko_texture_t, id, &rt, 1);
    neko_graphics_texture_destroy(rt);
    return 0;
}

static int __neko_bind_graphics_renderpass_create(lua_State* L) {

    neko_framebuffer_t fbo = neko_default_val();
    neko_lua_auto_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);

    neko_texture_t rt = neko_default_val();
    neko_lua_auto_struct_to_member(L, neko_texture_t, id, &rt, 2);

    neko_renderpass_t rp = neko_default_val();
    rp = neko_graphics_renderpass_create(neko_c_ref(neko_graphics_renderpass_desc_t, {
                                                                                             .fbo = fbo,               // Frame buffer to bind for render pass
                                                                                             .color = &rt,             // Color buffer array to bind to frame buffer
                                                                                             .color_size = sizeof(rt)  // Size of color attachment array in bytes
                                                                                     }));
    neko_lua_auto_struct_push_member(L, neko_renderpass_t, id, &rp);
    return 1;
}

static int __neko_bind_graphics_renderpass_destroy(lua_State* L) {
    neko_renderpass_t rp = neko_default_val();
    neko_lua_auto_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_graphics_renderpass_destroy(rp);
    return 0;
}

static int __neko_bind_graphics_renderpass_begin(lua_State* L) {
    neko_renderpass_t rp = neko_default_val();
    neko_lua_auto_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_graphics_renderpass_begin(g_client_userdata.cb, rp);
    return 0;
}

static int __neko_bind_graphics_renderpass_end(lua_State* L) {
    neko_graphics_renderpass_end(g_client_userdata.cb);
    return 0;
}

static int __neko_bind_graphics_set_viewport(lua_State* L) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 w = lua_tonumber(L, 3);
    f32 h = lua_tonumber(L, 4);
    neko_graphics_set_viewport(g_client_userdata.cb, x, y, w, h);
    return 0;
}

static int __neko_bind_graphics_clear(lua_State* L) {
    f32 r = lua_tonumber(L, 1);
    f32 g = lua_tonumber(L, 2);
    f32 b = lua_tonumber(L, 3);
    f32 a = lua_tonumber(L, 4);
    neko_graphics_clear_action_t clear = {.color = {r, g, b, a}};
    neko_graphics_clear(g_client_userdata.cb, clear);
    return 0;
}

int neko_print_registry_list(lua_State* L) {
    lua_pushglobaltable(L);  // 将注册表表压入栈中
    printf("Registry contents:\n");
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        printf("%s  %s  ", lua_tostring(L, -2), lua_typename(L, lua_type(L, -1)));
        int type = lua_type(L, -1);
        switch (type) {
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, -1));
                break;
            case LUA_TNUMBER:
                printf("%f\n", lua_tonumber(L, -1));
                break;
            default:
                printf("Unknown\n");
                break;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);  // 弹出注册表表
    return 0;
}

neko_inline void neko_register_test(lua_State* L) {

    lua_register(L, "__neko_print_registry_list", neko_print_registry_list);

    neko_lua_wrap_register_t<>(L).def(&__neko_bind_tiled_get_objects, "neko_tiled_get_objects");

    neko_lua_auto_enum(L, neko_projection_type);
    neko_lua_auto_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_ORTHOGRAPHIC);
    neko_lua_auto_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_PERSPECTIVE);

    neko_lua_auto_enum(L, neko_graphics_primitive_type);
    neko_lua_auto_enum_value(L, neko_graphics_primitive_type, NEKO_GRAPHICS_PRIMITIVE_LINES);
    neko_lua_auto_enum_value(L, neko_graphics_primitive_type, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    neko_lua_auto_enum_value(L, neko_graphics_primitive_type, NEKO_GRAPHICS_PRIMITIVE_QUADS);

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

    neko_lua_auto_struct(L, neko_renderpass_t);
    neko_lua_auto_struct_member(L, neko_renderpass_t, id, unsigned int);

    neko_lua_auto_struct(L, neko_sound_playing_sound_t);
    neko_lua_auto_struct_member(L, neko_sound_playing_sound_t, id, unsigned long long);
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
        lua_pushstring(g_client_userdata.L, type);
        neko_lua_auto_to(g_client_userdata.L, neko_cvar_type, &cval, -1);
        lua_pop(g_client_userdata.L, 1);
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

bool __neko_dolua(const_str file) { return neko_lua_wrap_dofile(g_client_userdata.L, game_assets(file)); }

neko_inline void neko_register_common(lua_State* L) {

    neko_lua_wrap_register_t<>(L)
            .def(&__neko_lua_trace, "log_trace")
            .def(&__neko_lua_bug, "log_debug")
            .def(&__neko_lua_error, "log_error")
            .def(&__neko_lua_warn, "log_warn")
            .def(&__neko_lua_info, "log_info")
            .def(&neko_rand_xorshf32, "neko_rand")
            .def(&__neko_dolua, "neko_dolua");

    neko_lua_wrap_register_t<>(L)
            .def(
                    +[](const_str str) { return neko_hash_str64(str); }, "neko_hash")
            .def(
                    +[](int op) { return neko_quit(); }, "__neko_quit")
            .def(
                    +[]() { return neko_platform_elapsed_time(); }, "neko_platform_elapsed_time");

    lua_pushstring(L, game_assets("gamedir").c_str());
    lua_setglobal(L, "neko_game_data_path");

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

void neko_register_test_oop(lua_State* L) {
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

int open_neko(lua_State* L) {
    luaL_Reg reg[] = {

            {"ecs_create_world", __neko_ecs_create_world},

            {"tiled_create", __neko_bind_tiled_create},
            {"tiled_render", __neko_bind_tiled_render},
            {"tiled_end", __neko_bind_tiled_end},
            {"tiled_load", __neko_bind_tiled_load},
            {"tiled_unload", __neko_bind_tiled_unload},

            {"fallingsand_create", __neko_bind_fallingsand_create},
            {"fallingsand_update", __neko_bind_fallingsand_update},
            {"fallingsand_end", __neko_bind_fallingsand_end},

            {"gfxt_create", __neko_bind_gfxt_create},
            {"gfxt_update", __neko_bind_gfxt_update},
            {"gfxt_end", __neko_bind_gfxt_end},

            {"draw_text", __neko_bind_draw_text},

            {"custom_sprite_create", __neko_bind_custom_sprite_create},
            {"custom_sprite_render", __neko_bind_custom_sprite_render},
            {"custom_sprite_end", __neko_bind_custom_sprite_end},

            {"sprite_batch_create", __neko_bind_sprite_batch_create},
            {"sprite_batch_render_ortho", __neko_bind_sprite_batch_render_ortho},
            {"sprite_batch_render_begin", __neko_bind_sprite_batch_render_begin},
            {"sprite_batch_render_end", __neko_bind_sprite_batch_render_end},
            {"sprite_batch_make_sprite", __neko_bind_sprite_batch_make_sprite},
            {"sprite_batch_push_sprite", __neko_bind_sprite_batch_push_sprite},
            {"sprite_batch_end", __neko_bind_sprite_batch_end},

            {"particle_create", __neko_bind_particle_create},
            {"particle_render", __neko_bind_particle_render},
            {"particle_end", __neko_bind_particle_end},

            {"gameobject_inspect", __neko_bind_gameobject_inspect},

            {"assetsys_create", __neko_bind_assetsys_create},
            {"assetsys_destory", __neko_bind_assetsys_destory},

            {"filewatch_create", __neko_bind_filewatch_create},
            {"filewatch_destory", __neko_bind_filewatch_destory},
            {"filewatch_mount", __neko_bind_filewatch_mount},
            {"filewatch_start", __neko_bind_filewatch_start_watch},
            {"filewatch_stop", __neko_bind_filewatch_stop_watching},
            {"filewatch_update", __neko_bind_filewatch_update},
            {"filewatch_notify", __neko_bind_filewatch_notify},

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

            {"audio_load", __neko_bind_audio_load},
            {"audio_unload", __neko_bind_audio_unload},
            {"audio_play", __neko_bind_audio_play},

            {"graphics_framebuffer_create", __neko_bind_graphics_framebuffer_create},
            {"graphics_framebuffer_destroy", __neko_bind_graphics_framebuffer_destroy},
            {"graphics_texture_create", __neko_bind_graphics_texture_create},
            {"graphics_texture_destroy", __neko_bind_graphics_texture_destroy},
            {"graphics_renderpass_create", __neko_bind_graphics_renderpass_create},
            {"graphics_renderpass_destroy", __neko_bind_graphics_renderpass_destroy},
            {"graphics_renderpass_begin", __neko_bind_graphics_renderpass_begin},
            {"graphics_renderpass_end", __neko_bind_graphics_renderpass_end},
            {"graphics_set_viewport", __neko_bind_graphics_set_viewport},
            {"graphics_clear", __neko_bind_graphics_clear},

            {"pack_construct", __neko_bind_pack_construct},
            {"pack_destroy", __neko_bind_pack_destroy},
            {"pack_build", __neko_bind_pack_build},
            {"pack_info", __neko_bind_pack_info},
            {"pack_items", __neko_bind_pack_items},
            {"pack_assets_load", __neko_bind_pack_assets_load},
            {"pack_assets_unload", __neko_bind_pack_assets_unload},

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

    return 1;
}

static int __neko_loader(lua_State* L) {
    const_str name = luaL_checkstring(L, 1);
    std::string path = name;
    std::replace(path.begin(), path.end(), '.', '/');
    path += ".lua";

    neko_println("fuck:%s", path.c_str());

    neko_pack_result result;

    u8* data;
    u32 data_size;

    result = neko_pack_item_data(g_client_userdata.pack, path.c_str(), (const u8**)&data, &data_size);
    if (result == 0) {
        if (luaL_loadbuffer(L, (char*)data, data_size, name) != LUA_OK) {
            lua_pop(L, 1);
        } else {
            neko_println("loaded:%s", path.c_str());
        }

        neko_pack_item_free(g_client_userdata.pack, data);
    }

    return 1;
}

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

void neko_register(lua_State* L) {

    g_client_userdata.L = L;

    luaopen_cstruct_core(L);
    luaopen_cstruct_test(L);

    PRELOAD("neko_lua_ds.core", luaopen_ds_core);
    PRELOAD("neko_lua_datalist.core", luaopen_datalist);
    PRELOAD("neko_gui", luaopen_neko_gui);
    PRELOAD("cffi", luaopen_cffi);
    PRELOAD("imgui", luaopen_imgui);

    luaL_requiref(L, "neko", open_neko, 1);

    neko_register_common(L);
    neko_register_platform(L);
    neko_register_test(L);
    neko_register_test_oop(L);

    open_luasocket(L);

    // 自定义加载器
    lua_register(L, "__neko_loader", __neko_loader);
    const char* str = "table.insert(package.searchers, 2, __neko_loader) \n";
    luaL_dostring(L, str);

    neko_lua_add_package_path(L, "./");
}
