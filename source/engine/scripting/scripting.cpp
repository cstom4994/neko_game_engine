#include "engine/scripting/scripting.h"

#include <stdlib.h>
#include <string.h>

#include "editor/editor.hpp"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/base/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/event.h"
#include "engine/input.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/luax.h"
#include "engine/scripting/scripting.h"
#include "engine/ui.h"

using namespace neko::luabind;

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

static const u8 g_lua_bootstrap_data[] = {
#include "bootstrap.lua.h"
};
static const u8 g_lua_nekogame_data[] = {
#include "nekogame.lua.h"
};
static const u8 g_lua_nekoeditor_data[] = {
#include "nekoeditor.lua.h"
};

// LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", g_lua_bootstrap_data);
// LUAOPEN_EMBED_DATA(open_embed_nekogame, "nekogame.lua", g_lua_nekogame_data);

extern "C" {
int luaopen_http(lua_State *L);

#if defined(NEKO_CFFI)
int luaopen_cffi(lua_State *L);
int luaopen_bit(lua_State *L);
#endif
}

void package_preload_embed(lua_State *L) {

    luaL_Reg preloads[] = {
#if defined(NEKO_CFFI)
            {"ffi", luaopen_cffi},
            {"bit", luaopen_bit},
#endif
            {"http", luaopen_http},
    };

    for (int i = 0; i < NEKO_ARR_SIZE(preloads); i++) {
        luax_package_preload(L, preloads[i].name, preloads[i].func);
    }
}
void luax_run_bootstrap(lua_State *L) {
    if (luaL_loadbuffer(L, (const_str)g_lua_bootstrap_data, neko_strlen((const_str)g_lua_bootstrap_data), "<bootstrap>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load bootstrap");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "bootstrap error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run bootstrap");
    }
    console_log("loaded bootstrap");
}
void luax_run_nekogame(lua_State *L) {
    if (luaL_loadbuffer(L, (const_str)g_lua_nekogame_data, neko_strlen((const_str)g_lua_nekogame_data), "<nekogame>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekogame");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekogame error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekogame");
    }
    console_log("loaded nekogame");

    if (luaL_loadbuffer(L, (const_str)g_lua_nekoeditor_data, neko_strlen((const_str)g_lua_nekoeditor_data), "<nekoeditor>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekoeditor");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekoeditor error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekoeditor");
    }
    console_log("loaded nekoeditor");
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

    LuaStruct<vec4>(L, "vec4");
    LuaStruct<gfx_texture_t>(L, "gfx_texture_t");
    LuaStruct<Color>(L, "Color");
    LuaStruct<NativeEntity>(L, "NativeEntity");
}

static const char **nekogame_ffi[] = {&nekogame_ffi_scalar, &nekogame_ffi_saveload, &nekogame_ffi_vec2, &nekogame_ffi_mat3, &nekogame_ffi_bbox, &nekogame_ffi_color, &nekogame_ffi_fs,
                                      &nekogame_ffi_game, &nekogame_ffi_system, &nekogame_ffi_input, &nekogame_ffi_entity, &nekogame_ffi_prefab, &nekogame_ffi_timing, &nekogame_ffi_transform,
                                      &nekogame_ffi_camera, &nekogame_ffi_sprite, &nekogame_ffi_tiled, &nekogame_ffi_gui, &nekogame_ffi_console,
                                      // &nekogame_ffi_sound,
                                      // &nekogame_ffi_physics,
                                      &nekogame_ffi_edit, &nekogame_ffi_inspector};

static const unsigned int n_nekogame_ffi = sizeof(nekogame_ffi) / sizeof(nekogame_ffi[0]);

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
// 如推入一个 vec2 应该为 _push_cdata("vec2 *", &v)
// 结果是堆栈上的 vec2 cdata (不是指针)
static void _push_cdata(const char *t, void *p) {
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
    char **argv;

    argc = game_get_argc();
    argv = game_get_argv();

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
}

int app_stop(App *app, event_t evt) {
    console_log("app_stop %p %s %f", app, event_string(evt.type), std::get<f64>(evt.p0.v));
    return 0;
}

void script_init() {
    PROFILE_FUNC();

    neko::timer timer;
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

    g_app->g_lua_callbacks_table_ref = LUA_NOREF;

    lua_channels_setup();

    event_register(eventhandler_instance(), g_app, quit, (evt_callback_t)app_stop, NULL);

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
    console_log(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());
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

        event_dispatch(eventhandler_instance(), event_t{.type = on_quit, .p0 = {.v = 199.14f}});
    }

    luax_get(ENGINE_LUA(), "neko", "game_fini");
    luax_pcall(ENGINE_LUA(), 0, 0);

    script_push_event("fini");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    lua_pop(L, 1);  // FFI
    lua_pop(L, 1);  // luax_msgh

    LuaVM vm{ENGINE_LUA()};

    vm.Fini(ENGINE_LUA());
}

int script_update_all(App *app, event_t evt) {
    lua_State *L = ENGINE_LUA();

    luax_get(ENGINE_LUA(), "neko", "game_pre_update");
    luax_pcall(ENGINE_LUA(), 0, 0);

    luax_get(ENGINE_LUA(), "neko", "game_loop");
    LuaPush<f32>(ENGINE_LUA(), get_timing_instance()->dt);
    luax_pcall(ENGINE_LUA(), 1, 0);

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

    luax_get(ENGINE_LUA(), "neko", "game_ui");
    luax_pcall(ENGINE_LUA(), 0, 0);

    script_push_event("draw_ui");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_draw_all() {
    lua_State *L = ENGINE_LUA();

    script_push_event("draw_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    luax_get(ENGINE_LUA(), "neko", "game_render");
    luax_pcall(ENGINE_LUA(), 0, 0);
}

void script_key_down(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_down");
    _push_cdata("KeyCode *", &key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_key_up(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_up");
    _push_cdata("KeyCode *", &key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_down(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_down");
    _push_cdata("MouseCode *", &mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_mouse_up(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_up");
    _push_cdata("MouseCode *", &mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_move(vec2 pos) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_move");
    _push_cdata("vec2 *", &pos);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_scroll(vec2 scroll) {
    lua_State *L = ENGINE_LUA();

    script_push_event("scroll");
    _push_cdata("vec2 *", &scroll);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_save_all(Store *s) {
    lua_State *L = ENGINE_LUA();

    Store *t;
    const char *str;

    if (store_child_save(&t, "script", s)) {
        // get string from Lua
        lua_getglobal(L, "ng");
        lua_getfield(L, -1, "__save_all");
        lua_remove(L, -2);
        errcheck(luax_pcall_nothrow(L, 0, 1));
        str = lua_tostring(L, -1);

        // save it
        string_save(&str, "str", t);

        // release
        lua_pop(L, 1);
    }
}

void script_load_all(Store *s) {
    lua_State *L = ENGINE_LUA();

    Store *t;
    char *str;

    if (store_child_load(&t, "script", s))
        if (string_load(&str, "str", NULL, t)) {
            // send it to Lua
            lua_getglobal(L, "ng");
            lua_getfield(L, -1, "__load_all");
            lua_remove(L, -2);
            lua_pushstring(L, str);
            errcheck(luax_pcall_nothrow(L, 1, 0));

            // release
            mem_free(str);
        }
}

#ifdef NEKO_IS_WIN32
#include <windows.h>

static void native_init_script_context_library(native_script_context_t *context, const char *assembly_path) {
    assert(context);

    context->handle = LoadLibraryA(assembly_path);
    if (!context->handle) {
        console_log("Failed to load script assembly: `%s'", assembly_path);
    }
}

static void *native_get_script_proc(native_script_context_t *context, const char *name) {
    assert(context);

    void *function = GetProcAddress((HMODULE)context->handle, name);
    if (!function) {
        console_log("Failed to locate function `%s'", name);
    }

    return function;
}

static void native_deinit_script_context_library(native_script_context_t *context) {
    assert(context);

    FreeLibrary((HMODULE)context->handle);
}

#else

#endif

#if 0

native_script_context_t *native_new_script_context(App *scene, const char *assembly_path) {
    assert(scene);

    native_script_context_t *new_ctx = (native_script_context_t *)mem_alloc(sizeof(native_script_context_t));

    new_ctx->scripts = NULL;
    new_ctx->script_count = 0;
    new_ctx->script_capacity = 0;

    new_ctx->scene = scene;

    native_init_script_context_library(new_ctx, assembly_path);

    return new_ctx;
}

void native_free_script_context(native_script_context_t *context) {
    assert(context);

    native_deinit_script_context_library(context);

    if (context->script_capacity > 0) {
        free(context->scripts);
    }

    free(context);
}

native_script_t *native_new_script(native_script_context_t *context, NativeEntity entity, const char *get_instance_size_name, const char *on_init_name, const char *on_update_name,
                                 const char *on_physics_update_name, const char *on_free_name, bool init_on_create) {
    assert(context);

    if (context->script_count >= context->script_capacity) {
        context->script_capacity = native_grow_capacity(context->script_capacity);
        context->scripts = (native_script_t *)mem_realloc(context->scripts, context->script_capacity * sizeof(native_script_t));
    }

    native_script_t *new_ctx = &context->scripts[context->script_count++];

    new_ctx->instance = NULL;

    new_ctx->entity = entity;

    NativeEntity *entity_ptr = native_get_entity_ptr(context->scene, entity);
    entity_ptr->script = new_ctx;

    new_ctx->get_instance_size_name = NULL;
    new_ctx->on_init_name = NULL;
    new_ctx->on_update_name = NULL;
    new_ctx->on_physics_update_name = NULL;
    new_ctx->on_free_name = NULL;

    new_ctx->on_init = NULL;
    new_ctx->on_update = NULL;
    new_ctx->on_physics_update = NULL;
    new_ctx->on_free = NULL;

    native_script_get_instance_size_f get_size = NULL;
    if (get_instance_size_name) {
        new_ctx->get_instance_size_name = native_copy_string(get_instance_size_name);

        get_size = native_get_script_proc(context, get_instance_size_name);
    }

    if (on_init_name) {
        new_ctx->on_init_name = native_copy_string(on_init_name);
        new_ctx->on_init = native_get_script_proc(context, on_init_name);
    }

    if (on_update_name) {
        new_ctx->on_update_name = native_copy_string(on_update_name);
        new_ctx->on_update = native_get_script_proc(context, on_update_name);
    }

    if (on_physics_update_name) {
        new_ctx->on_physics_update_name = native_copy_string(on_physics_update_name);
        new_ctx->on_physics_update = native_get_script_proc(context, on_physics_update_name);
    }

    if (on_free_name) {
        new_ctx->on_free_name = native_copy_string(on_free_name);
        new_ctx->on_free = native_get_script_proc(context, on_free_name);
    }

    if (get_size) {
        new_ctx->instance = malloc(get_size());
    }

    if (init_on_create && new_ctx->on_init) {
        new_ctx->on_init(context->scene, entity, new_ctx->instance);
    }

    return new_ctx;
}

void native_delete_script(native_script_context_t *context, native_script_t *script) {
    assert(context);
    assert(script);

    i32 index = -1;

    for (u32 i = 0; i < context->script_count; i++) {
        if (&context->scripts[i] == script) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    native_deinit_script(context, &context->scripts[index]);

    for (u32 i = index; i < context->script_count - 1; i++) {
        context->scripts[i] = context->scripts[i + 1];
    }

    context->script_count--;
}

void native_deinit_script(native_script_context_t *context, native_script_t *script) {
    assert(context);
    assert(script);

    if (script->on_free) {
        script->on_free(context->scene, script->entity, script->instance);
    }

    if (script->instance) {
        free(script->instance);
    }

    NativeEntity *entity_ptr = native_get_entity_ptr(context->scene, script->entity);
    entity_ptr->script = NULL;

    free(script->get_instance_size_name);
    free(script->on_init_name);
    free(script->on_update_name);
    free(script->on_free_name);
}

void native_init_scripts(native_script_context_t *context) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        if (script->on_init) {
            script->on_init(context->scene, script->entity, script->instance);
        }
    }
}

void native_update_scripts(native_script_context_t *context, double timestep) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        if (script->on_update) {
            script->on_update(context->scene, script->entity, script->instance, timestep);
        }
    }
}

void native_physics_update_scripts(native_script_context_t *context, double timestep) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        if (script->on_physics_update) {
            script->on_physics_update(context->scene, script->entity, script->instance, timestep);
        }
    }
}

void native_free_scripts(native_script_context_t *context) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        native_deinit_script(context, script);
    }
}

#endif