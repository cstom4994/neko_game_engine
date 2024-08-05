
#include <string>

#include "engine/lua_util.h"
#include "engine/luabind.hpp"
#include "engine/luax.h"

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
static const u8 g_lua_bootstrap_data[] = {
#include "bootstrap.lua.h"
};

LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", g_lua_bootstrap_data);

#ifdef NEKO_LUASOCKET
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
#endif

extern "C" {
int luaopen_socket_core(lua_State* L);
int luaopen_mime_core(lua_State* L);
int luaopen_http(lua_State* L);
}

namespace neko::lua {
void package_preload_embed(lua_State* L) {

    luaL_Reg preloads[] = {
            {"common", open_embed_common},

            {"http", luaopen_http},

#ifdef NEKO_LUASOCKET
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
#endif
    };

    for (auto m : preloads) {
        luax_package_preload(L, m.name, m.func);
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
    console_log("loaded bootstrap");
}
}  // namespace neko::lua