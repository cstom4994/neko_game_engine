#include "base/scripting/scripting.h"

#include <stdlib.h>
#include <string.h>

#include "editor/editor.hpp"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/event.h"
#include "engine/input.h"
#include "base/scripting/lua_wrapper.hpp"
#include "base/scripting/luax.h"
#include "base/scripting/scripting.h"
#include "engine/ui.h"

using namespace Neko::luabind;

int load_embed_lua(lua_State *L, const u8 B[], const_str name) {
    if (luaL_loadbuffer(L, (const_str)B, neko_strlen((const_str)B), name) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    return 1;
}

#define LUAOPEN_EMBED_DATA(func, name, compressed_data) \
    static int func(lua_State *L) {                     \
        i32 top = lua_gettop(L);                        \
        load_embed_lua(L, compressed_data, name);       \
        return lua_gettop(L) - top;                     \
    }

extern const uint8_t *get_bootstrap_lua();
extern const uint8_t *get_nekogame_lua();
extern const uint8_t *get_nekoeditor_lua();

// LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
// LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", bootstrap_lua);
// LUAOPEN_EMBED_DATA(open_embed_nekogame, "nekogame.lua", g_lua_nekogame_data);

extern "C" {
int luaopen_http(lua_State *L);
int luaopen_ffi(lua_State *L);
int luaopen_source_gen_nekogame_luaot(lua_State *L);
int luaopen_source_gen_nekoeditor_luaot(lua_State *L);
}

void package_preload_embed(lua_State *L) {

    luaL_Reg preloads[] = {
            {"http", luaopen_http},
    };

    for (int i = 0; i < NEKO_ARR_SIZE(preloads); i++) {
        luax_package_preload(L, preloads[i].name, preloads[i].func);
    }
}

void luax_run_bootstrap(lua_State *L) {
    if (luaL_loadbuffer(L, (const_str)get_bootstrap_lua(), neko_strlen((const_str)get_bootstrap_lua()), "<bootstrap>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load bootstrap");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "bootstrap error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run bootstrap");
    }
    LOG_INFO("loaded bootstrap");
}

int luax_aot_load(lua_State *L, lua_CFunction f, const char *name) {
    neko_lua_preload(L, f, name);
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "luax_aot_load error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to load luaot %s", name);
    }
    LOG_INFO("loaded luaot {}", name);
    return 0;
}

void luax_run_nekogame(lua_State *L) {

#if 1

    if (luaL_loadbuffer(L, (const_str)get_nekogame_lua(), neko_strlen((const_str)get_nekogame_lua()), "<nekogame>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekogame");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekogame error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekogame");
    }
    LOG_INFO("loaded nekogame");

    if (luaL_loadbuffer(L, (const_str)get_nekoeditor_lua(), neko_strlen((const_str)get_nekoeditor_lua()), "<nekoeditor>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekoeditor");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekoeditor error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekoeditor");
    }
    LOG_INFO("loaded nekoeditor");

#else

    luax_aot_load(L, luaopen_source_gen_nekogame_luaot, "source_gen_nekogame_luaot");
    luax_aot_load(L, luaopen_source_gen_nekoeditor_luaot, "source_gen_nekoeditor_luaot");

#endif
}

#if LUA_VERSION_NUM < 504

void *lua_newuserdatauv(lua_State *L_, size_t sz_, int nuvalue_) {
    neko_assert(L_ && nuvalue_ <= 1);
    return lua_newuserdata(L_, sz_);
}

int lua_getiuservalue(lua_State *const L_, int const idx_, int const n_) {
    if (n_ > 1) {
        lua_pushnil(L_);
        return LUA_TNONE;
    }

#if LUA_VERSION_NUM == 501
    lua_getfenv(L_, idx_);
    lua_getglobal(L_, LUA_LOADLIBNAME);
    if (lua_rawequal(L_, -2, -1) || lua_rawequal(L_, -2, LUA_GLOBALSINDEX)) {
        lua_pop(L_, 2);
        lua_pushnil(L_);

        return LUA_TNONE;
    } else {
        lua_pop(L_, 1);
    }
#else
    lua_getuservalue(L_, idx_);
#endif

    int const _uvType = lua_type(L_, -1);

    return (LUA_VERSION_NUM == 502 && _uvType == LUA_TNIL) ? LUA_TNONE : _uvType;
}

int lua_setiuservalue(lua_State *L_, int idx_, int n_) {
    if (n_ > 1
#if LUA_VERSION_NUM == 501
        || lua_type(L_, -1) != LUA_TTABLE
#endif
    ) {
        lua_pop(L_, 1);
        return 0;
    }

#if LUA_VERSION_NUM == 501
    lua_setfenv(L_, idx_);
#else
    lua_setuservalue(L_, idx_);
#endif
    return 1;
}

#endif

void createStructTables(lua_State *L) {

    LuaEnum<KeyCode, -1, 350>(L);
    LuaEnum<MouseCode>(L);
    LuaEnum<neko_texture_flags_t>(L);

    LuaStruct<vec2>(L);
    LuaStruct<vec4>(L);
    LuaStruct<mat3>(L);
    LuaStruct<AssetTexture>(L);
    LuaStruct<Color>(L);
    LuaStruct<NativeEntity>(L);
    LuaStruct<BBox>(L);
}

static int _traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring") || !lua_isstring(L, -1)) return 1;
        lua_remove(L, 1);
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}

int luax_pcall_nothrow(lua_State *L, int nargs, int nresults) {
    int r;
#if 0
    int errfunc;
    // 将 _traceback 放在 function 和 args 下
    errfunc = lua_gettop(L) - nargs;
    lua_pushcfunction(L, _traceback);
    lua_insert(L, errfunc);
    // call, remove _traceback
    r = lua_pcall(L, nargs, nresults, errfunc);
    lua_remove(L, errfunc);
#endif
    r = lua_pcall(L, nargs, nresults, 1);
    return r;
}

void script_run_string(const char *s) {
    lua_State *L = ENGINE_LUA();
    luaL_loadstring(L, s);
    errcheck(luax_pcall_nothrow(L, 0, LUA_MULTRET));
}
void script_run_file(const char *filename) {
    lua_State *L = ENGINE_LUA();
    luaL_loadfile(L, filename);
    errcheck(luax_pcall_nothrow(L, 0, LUA_MULTRET));
}
void script_error(const char *s) {
    lua_State *L = ENGINE_LUA();
    luaL_error(L, s);
}

// 将对象推送为 cdata, t 必须是字符串形式的 FFI 类型说明符
// 如推入一个 vec2 应该为 ng_push_cdata("vec2 *", &v)
// 结果是堆栈上的 vec2 cdata (不是指针)
void ng_push_cdata(const char *t, void *p) {
    // just call __deref_cdata(t, p)
    lua_State *L = ENGINE_LUA();
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__deref_cdata");
    lua_remove(L, -2);
    lua_pushstring(L, t);
    lua_pushlightuserdata(L, p);
    errcheck(luax_pcall_nothrow(L, 2, 1));
}

void script_push_event(const char *event) {
    // call nekogame.__fire_event(event, ...)
    lua_State *L = ENGINE_LUA();
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__fire_event");
    lua_remove(L, -2);
    lua_pushstring(L, event);
}

// 将命令行参数转发为 nekogame_args[0], nekogame_args[1], ...
static void _forward_args() {
    lua_State *L = ENGINE_LUA();

    int i, argc;
    const char **argv;

    argc = gBase.GetArgc();
    argv = gBase.GetArgv();

    lua_createtable(L, argc, 0);
    for (i = 0; i < argc; ++i) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "nekogame_args");
}

static void _set_paths() {
    lua_State *L = ENGINE_LUA();
    lua_pushstring(L, data_path(""));
    lua_setglobal(L, "nekogame_data_path");
    lua_pushstring(L, usr_path(""));
    lua_setglobal(L, "nekogame_usr_path");
}

// LuaJIT FFI 解析器无法解析 'NEKO_EXPORT' -- 使其成为空白
static void _fix_exports(char *s) {
    static const char keyword[] = NEKO_STR(NEKO_EXPORT);
    unsigned int i;

    while ((s = strstr(s, keyword)))
        for (i = 0; i < sizeof(keyword) - 1; ++i) *s++ = ' ';
}

// 相当于:
// ffi = require 'ffi'
// ffi.cdef(nekogame_ffi[0] .. nekogame_ffi[1] .. ...)
// with 'NEKO_EXPORT's fixed -- after this nekogame.lua can bind the FFI
static void _load_nekogame_ffi() {
    lua_State *L = ENGINE_LUA();

#if 0
    unsigned int i;
    char *fixed;
    luaL_Buffer buf;  // 将累积 nekogame_ffi cdefs 到这里

    // get ffi.cdef
    lua_getglobal(L, "require");
    lua_pushstring(L, "ffi");
    errcheck(luax_pcall_nothrow(L, 1, 1));
    lua_getfield(L, lua_gettop(L), "cdef");

    // accumulate nekogame_ffi cdefs
    luaL_buffinit(L, &buf);
    for (i = 0; i < n_nekogame_ffi; ++i) {
        fixed = (char *)mem_alloc(strlen(*nekogame_ffi[i]) + 1);
        strcpy(fixed, *nekogame_ffi[i]);
        _fix_exports(fixed);
        luaL_addstring(&buf, fixed);
        mem_free(fixed);
    }
    luaL_pushresult(&buf);

    errcheck(luax_pcall_nothrow(L, 1, 0));
#endif
}

int app_stop(App *app, event_t evt) {
    LOG_INFO("app_stop {} {}", event_string(evt.type), std::get<f64>(evt.p0.v));
    return 0;
}

void script_init() {
    PROFILE_FUNC();

    Neko::timer timer;
    timer.start();

    LuaVM vm;

    ENGINE_LUA() = vm.Create();

    auto L = ENGINE_LUA();

    open_neko_api(L);

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = lua_tostring(L, -1);
                printf("LUA: neko_panic error: %s", msg);
                return 0;
            });

    lua_pushcfunction(L, luax_msgh);  // 添加错误消息处理程序 始终位于堆栈底部

    _load_nekogame_ffi();
    _forward_args();
    _set_paths();

    gApp->g_lua_callbacks_table_ref = LUA_NOREF;

    lua_channels_setup();

    auto &eh = Neko::the<EventHandler>();
    eh.event_register(gApp, quit, (EventCallback)app_stop, NULL);

    luax_run_bootstrap(L);

    lua_register(L, "ecs_create", l_ecs_create_world);

    vm.RunString(R"(
        local __worlds = {}

        function fetch_world(name)
            local mw = __worlds[name]
            if not mw then
                mw = ecs_create()
                __worlds[name] = mw
            end
            return mw
        end

        gw = fetch_world("Admin")
    )");

    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");
    ENGINE_ECS() = (EcsWorld *)luaL_checkudata(L, -1, ECS_WORLD_OLD_UDATA_NAME);
    lua_pop(L, 1);

    timer.stop();
    LOG_INFO("lua init done in {0:.3f} ms", timer.get());
}

void script_fini() {
    lua_State *L = ENGINE_LUA();

    {
        PROFILE_BLOCK("before quit");

        luax_neko_get(L, "before_quit");
        if (lua_pcall(L, 0, 0, 1) != LUA_OK) {
            String err = luax_check_string(L, -1);
            neko_panic("%s", err.data);
        }

        auto &eh = Neko::the<EventHandler>();
        eh.event_dispatch(event_t{.type = on_quit, .p0 = {.v = 199.14f}});
    }

    script_push_event("fini");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    lua_pop(L, 1);  // luax_msgh

    LuaVM vm{ENGINE_LUA()};

    vm.Fini(ENGINE_LUA());
}

int script_pre_update_all(App *app, event_t evt) {
    lua_State *L = ENGINE_LUA();

    script_push_event("pre_update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    return 0;
}

int script_update_all(App *app, event_t evt) {
    lua_State *L = ENGINE_LUA();

    script_push_event("update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    return 0;
}

int script_post_update_all(App *app, event_t evt) {
    lua_State *L = ENGINE_LUA();

    script_push_event("post_update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    return 0;
}

void script_draw_ui() {
    lua_State *L = ENGINE_LUA();

    script_push_event("draw_ui");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_draw_all() {
    lua_State *L = ENGINE_LUA();

    script_push_event("draw_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_key_down(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_down");
    LuaPush<int>(L, key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_key_up(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_up");
    LuaPush<int>(L, key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_down(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_down");
    LuaPush<int>(L, mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_mouse_up(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_up");
    LuaPush<int>(L, mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_move(vec2 pos) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_move");
    // ng_push_cdata("vec2 *", &pos);
    LuaPush<vec2>(L, pos);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_scroll(vec2 scroll) {
    lua_State *L = ENGINE_LUA();

    script_push_event("scroll");
    // ng_push_cdata("vec2 *", &scroll);
    LuaPush<vec2>(L, scroll);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_save_all(App *app) {
    // lua_State *L = ENGINE_LUA();

    // Store *t;
    // const char *str;

    // if (store_child_save(&t, "script", s)) {
    //     // get string from Lua
    //     lua_getglobal(L, "ng");
    //     lua_getfield(L, -1, "__save_all");
    //     lua_remove(L, -2);
    //     errcheck(luax_pcall_nothrow(L, 0, 1));
    //     str = lua_tostring(L, -1);

    //    // save it
    //    string_save(&str, "str", t);

    //    // release
    //    lua_pop(L, 1);
    //}
}

void script_load_all(App *app) {
    // lua_State *L = ENGINE_LUA();

    // Store *t;
    // char *str;

    // if (store_child_load(&t, "script", s))
    //     if (string_load(&str, "str", NULL, t)) {
    //         // send it to Lua
    //         lua_getglobal(L, "ng");
    //         lua_getfield(L, -1, "__load_all");
    //         lua_remove(L, -2);
    //         lua_pushstring(L, str);
    //         errcheck(luax_pcall_nothrow(L, 1, 0));

    //        // release
    //        mem_free(str);
    //    }
}
