
#include <string>

#include "engine/neko_lua.h"
#include "engine/neko_lua_wrap.h"
#include "engine/neko_luabind.hpp"
#include "engine/neko_tolua.h"

int load_embed_lua(lua_State* L, const u8 B[], const_str name) {
    std::string contents = (const_str)B;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), name) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    return 1;
}

#define LUAOPEN_EMBED_DATA(func, name, compressed_data) \
    static int func(lua_State* L) {                     \
        i32 top = lua_gettop(L);                        \
        load_embed_lua(L, compressed_data, name);       \
        return lua_gettop(L) - top;                     \
    }

static const u8 g_lua_common_data[] = {
#include "common.lua.h"
};
static const u8 g_lua_prefabs_data[] = {
#include "prefabs.lua.h"
};
static const u8 g_lua_bootstrap_data[] = {
#include "bootstrap.lua.h"
};
static const u8 g_lua_gen_neko_api_data[] = {
#include "gen_neko_api.lua.h"
};
static const u8 ltn12_compressed_data[] = {
#include "ltn12.lua.h"
};
static const u8 mbox_compressed_data[] = {
#include "mbox.lua.h"
};
static const u8 mime_compressed_data[] = {
#include "mime.lua.h"
};
static const u8 socket_compressed_data[] = {
#include "socket.lua.h"
};
static const u8 socket_ftp_compressed_data[] = {
#include "socket.ftp.lua.h"
};
static const u8 socket_headers_compressed_data[] = {
#include "socket.headers.lua.h"
};
static const u8 socket_http_compressed_data[] = {
#include "socket.http.lua.h"
};
static const u8 socket_smtp_compressed_data[] = {
#include "socket.smtp.lua.h"
};
static const u8 socket_tp_compressed_data[] = {
#include "socket.tp.lua.h"
};
static const u8 socket_url_compressed_data[] = {
#include "socket.url.lua.h"
};

LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_prefabs, "prefabs.lua", g_lua_prefabs_data);
LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", g_lua_bootstrap_data);
LUAOPEN_EMBED_DATA(open_embed_gen_neko_api, "gen_neko_api.lua", g_lua_gen_neko_api_data);

LUAOPEN_EMBED_DATA(open_embed_ltn12, "ltn12.lua", ltn12_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_mbox, "mbox.lua", mbox_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_mime, "mime.lua", mime_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket, "socket.lua", socket_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket_ftp, "socket.ftp.lua", socket_ftp_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket_headers, "socket.headers.lua", socket_headers_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket_http, "socket.http.lua", socket_http_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket_smtp, "socket.smtp.lua", socket_smtp_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket_tp, "socket.tp.lua", socket_tp_compressed_data);
LUAOPEN_EMBED_DATA(open_embed_socket_url, "socket.url.lua", socket_url_compressed_data);

static void package_preload(lua_State* L, const_str name, lua_CFunction function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
}

extern "C" {
int luaopen_socket_core(lua_State* L);
int luaopen_mime_core(lua_State* L);
int luaopen_cffi(lua_State* L);
int luaopen_bit(lua_State* L);
int luaopen_http(lua_State* L);
int open_tools_spritepack(lua_State* L);
int luaopen_colibc_filesys(lua_State* L);
}

namespace neko::lua {
void package_preload(lua_State* L) {

    luaL_Reg preloads[] = {
            {"common", open_embed_common},
            {"prefabs", open_embed_prefabs},
            {"gen_neko_api", open_embed_gen_neko_api},

            {"__neko.spritepack", open_tools_spritepack},
            {"__neko.filesys", luaopen_colibc_filesys},

            {"ffi", luaopen_cffi},

            {"http", luaopen_http},

            {"enet", luaopen_enet},

            {"socket.core", luaopen_socket_core},
            {"mime.core", luaopen_mime_core},

            {"ltn12", open_embed_ltn12},
            {"mbox", open_embed_mbox},
            {"mime", open_embed_mime},
            {"socket", open_embed_socket},
            {"socket.ftp", open_embed_socket_ftp},
            {"socket.headers", open_embed_socket_headers},
            {"socket.http", open_embed_socket_http},
            {"socket.smtp", open_embed_socket_smtp},
            {"socket.tp", open_embed_socket_tp},
            {"socket.url", open_embed_socket_url},
    };

    for (auto m : preloads) {
        package_preload(L, m.name, m.func);
    }
}
void luax_run_bootstrap(lua_State* L) {
    std::string contents = (const_str)g_lua_bootstrap_data;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), "bootstrap.lua") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load bootstrap");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "bootstrap error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run bootstrap");
    }
    NEKO_INFO("loaded bootstrap");
}
}  // namespace neko::lua

int neko_tolua_boot_open(lua_State* L);

int neko_tolua_boot_open(lua_State* L) {
    neko_tolua_open(L);
    neko_tolua_module(L, NULL, 0);
    neko_tolua_beginmodule(L, NULL);

    {
        int top = lua_gettop(L);
        static const u8 B[] = {
#include "tolua.lua.h"
        };
        load_embed_lua(L, B, "tolua embedded: lua/tolua.lua");
        lua_settop(L, top);
    }

    {
        int top = lua_gettop(L);
        static const unsigned char B[] = R"lua(
local err, msg = xpcall(doit, debug.traceback)
if not err then
    print(msg)
    local _,_,label,msg = strfind(msg,"(.-:.-:%s*)(.*)")
    neko_tolua_error(msg,label)
else
    print("neko tolua generate ok")
end
        )lua";
        load_embed_lua(L, B, "tolua: embedded boot code");
        lua_settop(L, top);
    }

    neko_tolua_endmodule(L);
    return 1;
}
