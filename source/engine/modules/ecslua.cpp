//
// modify from flecs-lua (https://github.com/flecs-hub/flecs-lua)
// Copyright (c) 2020-2021 Randy <randy408@protonmail.com>
// Copyright (c) 2024 KaoruXun
//

#include <ctype.h> /* tolower() */

#define FLECS_LUA_IMPL
#include "ecslua.h"

ECS_COMPONENT_DECLARE(EcsLuaHost);

static int ecs_lua__ctx;
static int ecs_lua__world;

#define ECS_LUA_DEFAULT_CTX (&ecs_lua__ctx)
#define ECS_LUA_DEFAULT_WORLD (&ecs_lua__world)

#define ECS_LUA__KEEPOPEN 1
#define ECS_LUA__DYNAMIC 2 /* Loaded as Lua module */

#define ECS_LUA__LOG 0
#define ECS_LUA__ERROR 1
#define ECS_LUA__DEBUG 2
#define ECS_LUA__WARN 3

static ecs_lua_ctx *ctx_init(ecs_lua_ctx ctx);

ecs_lua_ctx *ecs_lua_get_context(lua_State *L, const ecs_world_t *world) {
    int type;
    ecs_lua_ctx *ctx;

    if (world) {
        type = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
        ecs_assert(type == LUA_TTABLE || type == LUA_TNIL, ECS_INTERNAL_ERROR, NULL);

        if (type == LUA_TNIL) return NULL;

        type = lua_rawgeti(L, -1, ECS_LUA_CONTEXT);
        ecs_assert(type == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

        ctx = (ecs_lua_ctx *)lua_touserdata(L, -1);
        lua_pop(L, 2);

        return ctx;
    }

    lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_CTX);
    ctx = (ecs_lua_ctx *)lua_touserdata(L, -1);
    lua_pop(L, 1);

    return ctx;
}

ecs_world_t *ecs_lua_get_world(lua_State *L) {
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, NULL);
    return ctx ? ctx->world : NULL;
}

bool ecs_lua_progress(lua_State *L, lua_Number delta_time) {
    ecs_lua__prolog(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, NULL);

    ecs_assert(ctx->progress_ref != LUA_NOREF, ECS_INVALID_PARAMETER, "progress callback is not set");

    if (ctx->progress_ref == LUA_NOREF) return false;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ctx->progress_ref);

    int type = lua_type(L, -1);
    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);
    if (type != LUA_TFUNCTION) return false;

    lua_pushnumber(L, delta_time);

    int ret = lua_pcall(L, 1, 1, 0);

    if (ret) {
        const char *err = lua_tostring(L, lua_gettop(L));
        ecs_os_err("progress() cb error (%d): %s", ret, err);
        lua_pop(L, 1);
        return false;
    }

    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

    int b = lua_toboolean(L, lua_gettop(L));
    lua_pop(L, 1);

    ecs_lua__epilog(L);

    return b;
}

void register_collectible(lua_State *L, ecs_world_t *w, int idx) {
    ecs_lua__prolog(L);
    idx = lua_absindex(L, idx);
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, w);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    /* registry[world].collect */
    type = lua_rawgeti(L, -1, ECS_LUA_COLLECT);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    lua_type(L, idx);
    lua_pushvalue(L, idx);
    lua_pushboolean(L, 1); /* dummy value */
    lua_settable(L, -3);   /* collect[obj] = true */

    lua_pop(L, 2);
    ecs_lua__epilog(L);
}

int ecs_lua_ref(lua_State *L, ecs_world_t *world) {
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ecs_get_world(world));
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ECS_LUA_REGISTRY);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    lua_pushvalue(L, -3);
    int ref = luaL_ref(L, -2);

    lua_pop(L, 3);

    return ref;
}

int ecs_lua_rawgeti(lua_State *L, ecs_world_t *world, int ref) {
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ecs_get_world(world));
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ECS_LUA_REGISTRY);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ref);

    lua_replace(L, -3);

    lua_pop(L, 1);

    return type;
}

void ecs_lua_unref(lua_State *L, ecs_world_t *world, int ref) {
    ecs_lua__prolog(L);

    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ecs_get_world(world));
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ECS_LUA_REGISTRY);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    luaL_unref(L, -1, ref);

    lua_pop(L, 2);

    ecs_lua__epilog(L);
}

/* Entity */
int new_entity(lua_State *L);
int new_id(lua_State *L);
int delete_entity(lua_State *L);
int new_tag(lua_State *L);
int entity_name(lua_State *L);
int set_name(lua_State *L);
int entity_symbol(lua_State *L);
int entity_fullpath(lua_State *L);
int lookup_entity(lua_State *L);
int lookup_child(lua_State *L);
int lookup_path(lua_State *L);
int lookup_fullpath(lua_State *L);
int lookup_symbol(lua_State *L);
int use_alias(lua_State *L);
int entity_has(lua_State *L);
int entity_owns(lua_State *L);
int is_alive(lua_State *L);
int is_valid(lua_State *L);
int get_alive(lua_State *L);
int ensure(lua_State *L);
int exists(lua_State *L);
int entity_add(lua_State *L);
int entity_remove(lua_State *L);
int clear_entity(lua_State *L);
int enable_entity(lua_State *L);
int disable_entity(lua_State *L);
int entity_count(lua_State *L);
int delete_children(lua_State *L);
int get_type(lua_State *L);
int get_typeid(lua_State *L);
int get_parent(lua_State *L);

int enable_component(lua_State *L);
int disable_component(lua_State *L);
int is_component_enabled(lua_State *L);

int add_pair(lua_State *L);
int remove_pair(lua_State *L);
int has_pair(lua_State *L);
int set_pair(lua_State *L);
int set_pair_object(lua_State *L);
int get_pair(lua_State *L);
int get_mut_pair(lua_State *L);
int get_pair_object(lua_State *L);
int get_mut_pair_object(lua_State *L);
int make_pair(lua_State *L);
int is_pair(lua_State *L);
int pair_object(lua_State *L);
int add_instanceof(lua_State *L);
int remove_instanceof(lua_State *L);
int add_childof(lua_State *L);
int remove_childof(lua_State *L);
int entity_override(lua_State *L);

int new_enum(lua_State *L);
int new_bitmask(lua_State *L);
int new_array(lua_State *L);
int new_struct(lua_State *L);
int new_alias(lua_State *L);

int get_func(lua_State *L);
int get_mut(lua_State *L);
int patch_func(lua_State *L);
int set_func(lua_State *L);

int new_ref(lua_State *L);
int get_ref(lua_State *L);

int singleton_get(lua_State *L);
int singleton_patch(lua_State *L);
int singleton_set(lua_State *L);

int new_prefab(lua_State *L);

/* Hierarchy */
int get_child_count(lua_State *L);
int set_scope(lua_State *L);
int get_scope(lua_State *L);
int set_name_prefix(lua_State *L);

/* Bulk */
int bulk_new(lua_State *L);

/* Iterator */
int iter_term(lua_State *L);
int iter_terms(lua_State *L);
int is_owned(lua_State *L);
int term_id(lua_State *L);
int filter_iter(lua_State *L);
int filter_next(lua_State *L);
int term_iter(lua_State *L);
int term_next(lua_State *L);
int iter_next(lua_State *L);

/* Query */
int query_gc(lua_State *L);
int query_new(lua_State *L);
int subquery_new(lua_State *L);
int query_iter(lua_State *L);
int query_next(lua_State *L);
int query_changed(lua_State *L);
int each_func(lua_State *L);

/* Snapshot */
int snapshot_take(lua_State *L);
int snapshot_restore(lua_State *L);
int snapshot_iter(lua_State *L);
int snapshot_next(lua_State *L);
int snapshot_gc(lua_State *L);

/* System */
int new_system(lua_State *L);
int new_trigger(lua_State *L);
int new_observer(lua_State *L);
int run_system(lua_State *L);
int set_system_context(lua_State *L);

/* Module */
int new_module(lua_State *L);
int import_handles(lua_State *L);

/* Log */
int print_log(lua_State *L);
int print_err(lua_State *L);
int print_dbg(lua_State *L);
int print_warn(lua_State *L);
int log_set_level(lua_State *L);
int log_enable_colors(lua_State *L);

/* Misc */
int assert_func(lua_State *L);
int sizeof_component(lua_State *L);
int l_is_primitive(lua_State *L);
int createtable(lua_State *L);
int zero_init_component(lua_State *L);
int get_world_ptr(lua_State *L);
int meta_constants(lua_State *L);

/* Time */
int get_time(lua_State *L);
int time_measure(lua_State *L);
int time__tostring(lua_State *L);

/* Timer */
int set_timeout(lua_State *L);
int get_timeout(lua_State *L);
int set_interval(lua_State *L);
int get_interval(lua_State *L);
int start_timer(lua_State *L);
int stop_timer(lua_State *L);
int set_rate_filter(lua_State *L);
int set_tick_source(lua_State *L);

/* Pipeline */
int new_pipeline(lua_State *L);
int set_pipeline(lua_State *L);
int get_pipeline(lua_State *L);
int progress(lua_State *L);
int progress_cb(lua_State *L);
int measure_frame_time(lua_State *L);
int measure_system_time(lua_State *L);
int set_target_fps(lua_State *L);
int set_time_scale(lua_State *L);
int reset_clock(lua_State *L);
int lquit(lua_State *L);
int set_threads(lua_State *L);
int get_threads(lua_State *L);
int get_thread_index(lua_State *L);  // compat

/* World */
int world_new(lua_State *L);
int world_fini(lua_State *L);
int world_gc(lua_State *L);
int world_info(lua_State *L);
int world_stats(lua_State *L);
int dim(lua_State *L);

/* EmmyLua */
int emmy_class(lua_State *L);

static const luaL_Reg ecs_lib[] = {{"new", new_entity},
                                   {"new_id", new_id},
                                   {"delete", delete_entity},
                                   {"tag", new_tag},
                                   {"name", entity_name},
                                   {"set_name", set_name},
                                   {"symbol", entity_symbol},
                                   {"fullpath", entity_fullpath},
                                   {"lookup", lookup_entity},
                                   {"lookup_child", lookup_child},
                                   {"lookup_path", lookup_path},
                                   {"lookup_fullpath", lookup_fullpath},
                                   {"lookup_symbol", lookup_symbol},
                                   {"use", use_alias},
                                   {"has", entity_has},
                                   {"owns", entity_owns},
                                   {"is_alive", is_alive},
                                   {"is_valid", is_valid},
                                   {"get_alive", get_alive},
                                   {"ensure", ensure},
                                   {"exists", exists},
                                   {"add", entity_add},
                                   {"remove", entity_remove},
                                   {"clear", clear_entity},
                                   {"enable", enable_entity},
                                   {"disable", disable_entity},
                                   {"count", entity_count},
                                   {"delete_children", delete_children},
                                   {"get_parent", get_parent},
                                   {"get_type", get_type},
                                   {"get_typeid", get_typeid},

                                   {"enable_component", enable_component},
                                   {"disable_component", disable_component},
                                   {"is_component_enabled", is_component_enabled},

                                   {"add_pair", add_pair},
                                   {"remove_pair", remove_pair},
                                   {"has_pair", has_pair},
                                   {"set_pair", set_pair},
                                   {"set_pair_object", set_pair_object},
                                   {"get_pair", get_pair},
                                   {"get_mut_pair", get_mut_pair},
                                   {"get_pair_object", get_pair_object},
                                   {"get_mut_pair_object", get_mut_pair_object},
                                   {"pair", make_pair},
                                   {"is_pair", is_pair},
                                   {"pair_object", pair_object},
                                   {"add_instanceof", add_instanceof},
                                   {"remove_instanceof", remove_instanceof},
                                   {"add_childof", add_childof},
                                   {"remove_childof", remove_childof},
                                   {"override", entity_override},
                                   {"add_owned", entity_override},  // compat

                                   {"enum", new_enum},
                                   {"bitmask", new_bitmask},
                                   {"array", new_array},
                                   {"struct", new_struct},
                                   {"alias", new_alias},

                                   {"get", get_func},
                                   {"get_mut", get_mut},
                                   {"patch", patch_func},
                                   {"set", set_func},
                                   {"ref", new_ref},
                                   {"get_ref", get_ref},

                                   {"singleton_get", singleton_get},
                                   {"singleton_patch", singleton_patch},
                                   {"singleton_set", singleton_set},

                                   {"prefab", new_prefab},

                                   {"get_child_count", get_child_count},
                                   {"set_scope", set_scope},
                                   {"get_scope", get_scope},
                                   {"set_name_prefix", set_name_prefix},

                                   {"bulk_new", bulk_new},

                                   {"term", iter_term},
                                   {"terms", iter_terms},
                                   {"column", iter_term},    // compat
                                   {"columns", iter_terms},  // compat
                                   {"is_owned", is_owned},
                                   {"column_entity", term_id},  // compat
                                   {"term_id", term_id},
                                   {"filter_iter", filter_iter},
                                   {"filter_next", filter_next},
                                   {"term_iter", term_iter},
                                   {"term_next", term_next},
                                   {"iter_next", iter_next},

                                   {"query", query_new},
                                   {"subquery", subquery_new},
                                   {"query_iter", query_iter},
                                   {"query_next", query_next},
                                   {"query_changed", query_changed},
                                   {"each", each_func},

                                   {"system", new_system},
                                   {"trigger", new_trigger},
                                   {"observer", new_observer},
                                   {"run", run_system},
                                   {"set_system_context", set_system_context},

                                   {"snapshot", snapshot_take},
                                   {"snapshot_restore", snapshot_restore},
                                   {"snapshot_iter", snapshot_iter},
                                   {"snapshot_next", snapshot_next},

                                   {"module", new_module},
                                   {"import", import_handles},

                                   {"log", print_log},
                                   {"err", print_err},
                                   {"dbg", print_dbg},
                                   {"warn", print_warn},
                                   {"log_set_level", log_set_level},
                                   {"tracing_enable", log_set_level},  // compat
                                   {"log_enable_colors", log_enable_colors},
                                   {"tracing_color_enable", log_enable_colors},  // compat

                                   {"assert", assert_func},
                                   {"sizeof", sizeof_component},
                                   {"is_primitive", l_is_primitive},
                                   {"createtable", createtable},
                                   {"zero_init", zero_init_component},
                                   {"world_ptr", get_world_ptr},
                                   {"meta_constants", meta_constants},

                                   {"get_time", get_time},
                                   {"time_measure", time_measure},

                                   {"set_timeout", set_timeout},
                                   {"get_timeout", get_timeout},
                                   {"set_interval", set_interval},
                                   {"get_interval", get_interval},
                                   {"start_timer", start_timer},
                                   {"stop_timer", stop_timer},
                                   {"set_rate_filter", set_rate_filter},
                                   {"set_tick_source", set_tick_source},

                                   {"pipeline", new_pipeline},
                                   {"set_pipeline", set_pipeline},
                                   {"get_pipeline", get_pipeline},
                                   {"progress", progress},
                                   {"progress_cb", progress_cb},
                                   {"measure_frame_time", measure_frame_time},
                                   {"measure_system_time", measure_system_time},
                                   {"set_target_fps", set_target_fps},
                                   {"set_time_scale", set_time_scale},
                                   {"reset_clock", reset_clock},
                                   {"quit", lquit},
                                   {"set_threads", set_threads},
                                   {"get_threads", get_threads},            // compat: get_stage_count
                                   {"get_thread_index", get_thread_index},  // compat: get_stage_id

                                   {"get_stage_count", get_threads},
                                   {"get_stage_id", get_thread_index},

                                   {"init", world_new},
                                   {"fini", world_fini},
                                   {"world_info", world_info},
                                   {"world_stats", world_stats},
                                   {"dim", dim},

                                   {"emmy_class", emmy_class},

#define XX(const) {#const, NULL},
                                   ECS_LUA_ENUMS(XX) ECS_LUA_MACROS(XX)
#undef XX
                                           {NULL, NULL}};

static void register_types(lua_State *L) {
    luaL_newmetatable(L, "ecs_type_t");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_ref_t");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_world_t");
    lua_pushcfunction(L, world_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_collect_t");
    lua_pushstring(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_readonly");
    lua_pushcfunction(L, ecs_lua__readonly);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, ecs_lua__readonly);
    lua_setfield(L, -2, "__usedindex");
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_query_t");
    lua_pushcfunction(L, query_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_snapshot_t");
    lua_pushcfunction(L, snapshot_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_time_t");
    lua_pushcfunction(L, time__tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pop(L, 1);
}

int luaopen_ecs(lua_State *L) {
    luaL_checkversion(L);
    luaL_newlibtable(L, ecs_lib);

    ecs_world_t *w;
    int type = lua_type(L, 1);
    int default_world = 0;

    if (type != LUA_TUSERDATA) default_world = 1;

    if (default_world) {
        register_types(L);

        w = ecs_lua_get_world(L);

        if (w == NULL) /* Loaded as Lua module */
        {
            w = ecs_init();

            ECS_IMPORT(w, FlecsLua);

            ecs_lua_ctx param = {.L = L, .world = w, .flags = ECS_LUA__DYNAMIC};

            ecs_lua_ctx *ctx = ctx_init(param);

            // ecs_singleton_set(w, EcsLuaHost, {L, ctx});

            EcsLuaHost p = {L, ctx};
            ecs_singleton_set_ptr(w, EcsLuaHost, &p);
        }

        ecs_world_t **ptr = (ecs_world_t **)lua_newuserdata(L, sizeof(ecs_world_t *));
        *ptr = w;

        luaL_setmetatable(L, "ecs_world_t");
        lua_pushvalue(L, -1);
        lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_WORLD);
    } else /* ecs.init() */
    {
        w = *(ecs_world_t **)lua_touserdata(L, 1);
        lua_pushvalue(L, 1);
    }

    /* registry[world] = { [cursors], [types], [collect], ... } */
    lua_createtable(L, 4, 0);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, w);

    if (default_world)
        lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_CTX);
    else
        lua_pushvalue(L, 1);

    lua_rawseti(L, -2, ECS_LUA_CONTEXT);

    lua_createtable(L, 128, 0);
    lua_rawseti(L, -2, ECS_LUA_CURSORS);

    lua_createtable(L, 128, 0);
    lua_rawseti(L, -2, ECS_LUA_TYPES);

    /* world[collect] = { [object1], [object2], ... } */
    lua_createtable(L, 0, 16);
    luaL_setmetatable(L, "ecs_collect_t");
    lua_rawseti(L, -2, ECS_LUA_COLLECT);

    lua_createtable(L, 0, 16);
    lua_rawseti(L, -2, ECS_LUA_REGISTRY);

    lua_pushvalue(L, -2); /* world userdata */
    lua_rawseti(L, -2, ECS_LUA_APIWORLD);

    lua_pop(L, 1); /* registry[world] */

    luaL_setfuncs(L, ecs_lib, 1);

#define XX(type)                           \
    lua_pushinteger(L, ecs_id(Ecs##type)); \
    lua_setfield(L, -2, #type);
    ECS_LUA_TYPEIDS(XX)
#undef XX

#define XX(type)                                \
    lua_pushinteger(L, ecs_id(ecs_##type##_t)); \
    lua_setfield(L, -2, #type);
    ECS_LUA_PRIMITIVES(XX)
#undef XX

#define XX(const)                   \
    lua_pushinteger(L, Ecs##const); \
    lua_setfield(L, -2, #const);
    ECS_LUA_BUILTINS(XX)
#undef XX

#define XX(const)                   \
    lua_pushinteger(L, Ecs##const); \
    lua_setfield(L, -2, #const);
    ECS_LUA_ENUMS(XX)
#undef XX

#define XX(const)                    \
    lua_pushinteger(L, ECS_##const); \
    lua_setfield(L, -2, #const);
    ECS_LUA_MACROS(XX)
#undef XX

    /*
        lua_pushinteger(L, ecs_id(ecs_meta_type_op_kind_t));
        lua_setfield(L, -2, "meta_type_op_kind_t");
    */
    lua_pushinteger(L, ecs_lookup_fullpath(w, "flecs.meta.ecs_meta_type_op_t"));
    lua_setfield(L, -2, "meta_type_op_t");

    return 1;
}

static ecs_lua_ctx *ctx_init(ecs_lua_ctx ctx) {
    lua_State *L = ctx.L;
    ecs_world_t *world = ctx.world;

    ecs_lua__prolog(L);

    ecs_lua_ctx *default_ctx = ecs_lua_get_context(L, NULL);

    if (default_ctx) return default_ctx;

    ecs_lua_ctx *lctx = (ecs_lua_ctx *)lua_newuserdata(L, sizeof(ecs_lua_ctx));

    lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_CTX);

    memcpy(lctx, &ctx, sizeof(ecs_lua_ctx));

    lctx->error = 0;
    lctx->progress_ref = LUA_NOREF;
    lctx->prefix_ref = LUA_NOREF;

    if (!(ctx.flags & ECS_LUA__DYNAMIC)) {
        luaL_requiref(L, "ecs", luaopen_ecs, 1);
        lua_pop(L, 1);
    }

    ecs_lua__epilog(L);

    return lctx;
}

static void ecs_lua_exit(lua_State *L) {
    if (!L) return;

    ecs_lua_ctx *ctx = ecs_lua_get_context(L, NULL);

    if (!(ctx->internal & ECS_LUA__KEEPOPEN)) lua_close(L);
}

static void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize) {
    if (!nsize) {
        ecs_os_free(ptr);
        return NULL;
    }

    return ecs_os_realloc(ptr, nsize);
}

lua_State *ecs_lua_get_state(ecs_world_t *world) {
    const EcsLuaHost *host = ecs_singleton_get(world, EcsLuaHost);

    if (!host) {
        lua_State *L = lua_newstate(Allocf, NULL);

        ecs_lua_ctx param = {L, world};

        ecs_lua_ctx *ctx = ctx_init(param);

        EcsLuaHost p = {L, ctx};
        ecs_singleton_set_ptr(world, EcsLuaHost, &p);

        host = ecs_singleton_get(world, EcsLuaHost);
    }

    ecs_assert(host != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_assert(host->L != NULL, ECS_INTERNAL_ERROR, NULL);

    return host->L;
}

int ecs_lua_set_state(ecs_world_t *world, lua_State *L) {
    ecs_assert(L != NULL, ECS_INVALID_PARAMETER, NULL);

    EcsLuaHost *host = ecs_singleton_get_mut(world, EcsLuaHost);

    ecs_lua_exit(host->L);

    ecs_lua_ctx param = {.L = L, .world = world, .internal = ECS_LUA__KEEPOPEN};

    host->L = L;
    host->ctx = ctx_init(param);

    ecs_singleton_modified(world, EcsLuaHost);

    return 0;
}

static ecs_entity_t ecs_id(ecs_meta_type_op_t);

void EcsLuaHost__OnRemove(ecs_iter_t *it) {
    EcsLuaHost *ptr = ecs_field(it, EcsLuaHost, 1);

    lua_State *L = ptr->L;
    if (L == NULL) return;

    ecs_world_t *wdefault = ecs_lua_get_world(L);

    if (wdefault == it->real_world) { /* This is the default world in this VM */
        lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_WORLD);
        luaL_callmeta(L, -1, "__gc");

        lua_close(L);
        ptr->L = NULL;
    }
}

void FlecsLuaImport(ecs_world_t *w) {
    ECS_MODULE(w, FlecsLua);

    ECS_IMPORT(w, FlecsMeta);

    ecs_set_name_prefix(w, "EcsLua");

    ECS_COMPONENT_DEFINE(w, EcsLuaHost);

    ECS_META_COMPONENT(w, EcsLuaWorldInfo);
    ECS_META_COMPONENT(w, EcsLuaGauge);
    ECS_META_COMPONENT(w, EcsLuaGauge_);
    ECS_META_COMPONENT(w, EcsLuaCounter);
    ECS_META_COMPONENT(w, EcsLuaWorldStats);
    ECS_META_COMPONENT(w, EcsLuaTermID);
    ECS_META_COMPONENT(w, EcsLuaTerm);

    ecs_entity_t old_scope = ecs_set_scope(w, 0);

    /* flecs only defines ecs_uptr_t */

    ecs_entity_desc_t ent_des_e = {.name = "uintptr_t", .symbol = "uintptr_t"};
    ecs_entity_t e = ecs_entity_init(w, &ent_des_e);
    EcsPrimitive pri = {.kind = EcsUPtr};
    ecs_set_ptr(w, e, EcsPrimitive, &pri);

    ecs_set_scope(w, old_scope);

    ecs_set_scope(w, ecs_lookup_fullpath(w, "flecs.meta"));

    // ECS_META_COMPONENT(w, ecs_meta_type_op_t);

    ecs_entity_desc_t ent_des_i = {.id = ecs_id(ecs_meta_type_op_t), .name = "ecs_meta_type_op_t", .use_low_id = true};
    ecs_entity_init(w, &ent_des_i);

    ecs_component_desc_t m_com_des = {.entity = ecs_id(ecs_meta_type_op_t),
                                      .type = {.size = sizeof(ecs_meta_type_op_t), .alignment = ECS_ALIGNOF(ecs_meta_type_op_t), .component = ecs_id(ecs_meta_type_op_t)}};

    ecs_id(ecs_meta_type_op_t) = ecs_component_init(w, &m_com_des);

    ecs_struct_desc_t i_struct_des = {.entity = ecs_id(ecs_meta_type_op_t),
                                      .members = {
                                              {.name = (char *)"kind", .type = ecs_id(ecs_i32_t)},
                                              {.name = (char *)"offset", .type = ecs_id(ecs_i32_t)},
                                              {.name = (char *)"count", .type = ecs_id(ecs_i32_t)},
                                              {.name = (char *)"name", .type = ecs_id(ecs_string_t)},
                                              {.name = (char *)"op_count", .type = ecs_id(ecs_i32_t)},
                                              {.name = (char *)"size", .type = ecs_id(ecs_i32_t)},
                                              {.name = (char *)"type", .type = ecs_id(ecs_entity_t)},
                                              {.name = (char *)"unit", .type = ecs_id(ecs_entity_t)},
                                      }};

    ecs_struct_init(w, &i_struct_des);

    ecs_struct_desc_t struct_des = {.entity = ecs_id(EcsMetaTypeSerialized),
                                    .members = {
                                            {.name = (char *)"*ops", .type = ecs_id(EcsVector)},
                                    }};
    ecs_struct_init(w, &struct_des);

    ecs_set_scope(w, old_scope);

    NEKO_TRACE("gauge size %zu;counter size %zu;metric size %zu;worldstats: %zu, world_stats: %zu", sizeof(EcsLuaGauge), sizeof(EcsLuaCounter), sizeof(ecs_metric_t), sizeof(EcsWorldStats),
               sizeof(ecs_world_stats_t));

    ecs_assert(sizeof(EcsLuaGauge) == sizeof(ecs_metric_t), ECS_INTERNAL_ERROR, NULL);
    ecs_assert(sizeof(EcsLuaCounter) == sizeof(ecs_metric_t), ECS_INTERNAL_ERROR, NULL);
    ecs_assert(sizeof(EcsLuaWorldStats) == sizeof(ecs_world_stats_t), ECS_INTERNAL_ERROR, NULL);

    ecs_type_hooks_t th = {
            .ctor = ecs_default_ctor,
            .on_remove = EcsLuaHost__OnRemove  // atfini's are called too early (e.g. before OnRemove observers) since at least v3.2.0
    };
    ecs_set_hooks_id(w, ecs_id(EcsLuaHost), &th);
}

int bulk_new(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t id = 0;
    lua_Integer count = 0;
    const ecs_entity_t *entities = NULL;

    int noreturn = 0;
    int args = lua_gettop(L);
    int last_type = lua_type(L, args);

    if (args == 2 && last_type == LUA_TBOOLEAN) /* bulk_new(count, noreturn) */
    {
        count = luaL_checkinteger(L, 1);
        noreturn = lua_toboolean(L, 2);
    } else if (args >= 2) /* bulk_new(component, count, [noreturn]) */
    {
        id = luaL_checkinteger(L, 1);

        count = luaL_checkinteger(L, 2);
        noreturn = lua_toboolean(L, 3);
    } else
        count = luaL_checkinteger(L, 1); /* bulk_new(count) */

    entities = ecs_bulk_new_w_id(w, id, count);

    if (noreturn) return 0;

    lua_createtable(L, count, 0);

    lua_Integer i;
    for (i = 0; i < count; i++) {
        lua_pushinteger(L, entities[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static const char *primitive_type_name(enum ecs_meta_type_op_kind_t kind) {
    switch (kind) {
        case EcsBool:
            return "boolean";
        case EcsChar:
        case EcsByte:
        case EcsU8:
        case EcsU16:
        case EcsU32:
        case EcsU64:
        case EcsI8:
        case EcsI16:
        case EcsI32:
        case EcsI64:
        case EcsUPtr:
        case EcsIPtr:
        case EcsEntity:
            return "integer";
        case EcsF32:
        case EcsF64:
            return "number";
        case EcsString:
            return "string";
        default:
            return "unknown";
    }
}

static const char *array_type_name(const ecs_world_t *world, ecs_meta_type_op_t *op) {
    return "array_type_name";
    const EcsArray *a = ecs_get(world, op->type, EcsArray);
    ecs_assert(a != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_entity_t type = a->type;

    // return ecs_get_name(world, type);

    const EcsMetaTypeSerialized *ser = ecs_get(world, type, EcsMetaTypeSerialized);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);

#ifndef NDEBUG
    int32_t count = ecs_vec_count(&ser->ops);
    ecs_assert(count >= 2, ECS_INVALID_PARAMETER, NULL);
#endif

    op = &ops[1];

    const char *name = NULL;

    if (op->kind == EcsOpPrimitive)
        name = primitive_type_name(op->kind);
    else if (op->type)
        name = ecs_get_name(world, op->type);

    return name;
}

char *ecs_type_to_emmylua(const ecs_world_t *world, ecs_entity_t type, bool struct_as_table) {
    const EcsMetaTypeSerialized *ser = ecs_get(world, type, EcsMetaTypeSerialized);
    ecs_assert(ser != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    const char *class_name = ecs_get_symbol(world, type);
    const char *field_name;
    const char *lua_type = "";
    const char *lua_type_suffix;
    char array_suffix[16];

    if (!class_name) class_name = ecs_get_name(world, type);

    ecs_assert(class_name, ECS_INVALID_PARAMETER, NULL);

    ecs_strbuf_list_append(&buf, "---@class %s\n", class_name);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);
    int32_t count = ecs_vec_count(&ser->ops);

    ecs_assert(ops[0].kind == EcsOpPush, ECS_INVALID_PARAMETER, NULL);

    int i, depth = 1;

    for (i = 1; i < count; i++) {
        ecs_meta_type_op_t *op = &ops[i];

        lua_type_suffix = NULL;

        if (op->count > 1) {
            snprintf(array_suffix, sizeof(array_suffix), "[%d]", op->count);
            lua_type_suffix = array_suffix;
        }

        switch (op->kind) {
            case EcsOpPush: {
                depth++;

                if (depth > 2) continue; /* nested struct */

                if (struct_as_table)
                    lua_type = "table";
                else
                    lua_type = NULL;

                break;
            }
            case EcsOpPop: {
                depth--;
                continue;
            }
            case EcsOpArray: {
                lua_type_suffix = "[]";
                lua_type = array_type_name(world, op);

                if (op->count > 1) {
                    snprintf(array_suffix, sizeof(array_suffix), "[%d]", op->count);
                    lua_type_suffix = array_suffix;
                }

                break;
            }
            case EcsOpVector: {
                lua_type_suffix = "[]";
                lua_type = array_type_name(world, op);
                break;
            }
            case EcsOpPrimitive: {
                lua_type = primitive_type_name(op->kind);
                break;
            }
            case EcsOpEnum:
            case EcsOpBitmask: {
                lua_type = "integer";
                break;
            }
            default:
                break;
        }

        if (!lua_type_suffix) lua_type_suffix = "";

        /* skip nested struct members */
        if (op->kind != EcsOpPush && depth > 1) continue;

        if (op->name)
            field_name = op->name;
        else
            field_name = "";

        if (!lua_type && op->type) {
            lua_type = ecs_get_symbol(world, op->type);
            if (!lua_type) lua_type = ecs_get_name(world, op->type);
        }

        ecs_strbuf_list_append(&buf, "---@field %s %s%s\n", field_name, lua_type, lua_type_suffix);
    }

    ecs_strbuf_list_append(&buf, "local %s = {}\n", class_name);

    return ecs_strbuf_get(&buf);
}

static const char *kind_str(enum ecs_meta_type_op_kind_t kind) {
    switch (kind) {
        case EcsOpArray:
            return "EcsOpArray";
        case EcsOpVector:
            return "EcsOpVector";
        case EcsOpPush:
            return "EcsOpPush";
        case EcsOpPop:
            return "EcsOpPop";
        case EcsOpScope:
            return "EcsOpScope";
        case EcsOpEnum:
            return "EcsOpEnum";
        case EcsOpBitmask:
            return "EcsOpBitmask";
        case EcsOpPrimitive:
            return "EcsOpPrimitive";
        case EcsOpBool:
            return "EcsOpBool";
        case EcsOpChar:
            return "EcsOpChar";
        case EcsOpByte:
            return "EcsOpByte";
        case EcsOpU8:
            return "EcsOpU8";
        case EcsOpU16:
            return "EcsOpU16";
        case EcsOpU32:
            return "EcsOpU32";
        case EcsOpU64:
            return "EcsOpU64";
        case EcsOpI8:
            return "EcsOpI8";
        case EcsOpI16:
            return "EcsOpI16";
        case EcsOpI32:
            return "EcsOpI32";
        case EcsOpI64:
            return "EcsOpI64";
        case EcsOpF32:
            return "EcsOpF32";
        case EcsOpF64:
            return "EcsOpF64";
        case EcsOpUPtr:
            return "EcsOpUPtr";
        case EcsOpIPtr:
            return "EcsOpIPtr";
        case EcsOpString:
            return "EcsOpString";
        case EcsOpEntity:
            return "EcsOpEntity";
        default:
            return "UNKNOWN";
    }
}

static char *str_type_ops(ecs_world_t *w, ecs_entity_t type, int recursive) {
    const EcsMetaTypeSerialized *ser = ecs_get(w, type, EcsMetaTypeSerialized);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);
    int count = ecs_vec_count(&ser->ops);

    int i, depth = 0;
    for (i = 0; i < count; i++) {
        ecs_meta_type_op_t *op = &ops[i];

        ecs_strbuf_append(&buf, "kind: %s\n", kind_str(op->kind));
        ecs_strbuf_append(&buf, "offset: %d\n", op->offset);
        ecs_strbuf_append(&buf, "count: %d\n", op->count);
        ecs_strbuf_append(&buf, "name: \"%s\"\n", op->name ? op->name : "(null)");
        ecs_strbuf_append(&buf, "op_count: %d\n", op->op_count);
        ecs_strbuf_append(&buf, "size: %d\n", op->size);

        const char *name = op->type ? ecs_get_name(w, op->type) : "null";
        ecs_strbuf_append(&buf, "type: %zu (\"%s\")\n", op->type, name ? name : "null");
        // printf("\n");

        if (op->kind == EcsOpPush) {
            depth++;
            // if(recursive) print_type_ops(w, type, 1);
        } else if (op->kind == EcsOpPop)
            depth--;
    }

    return ecs_strbuf_get(&buf);
}

int emmy_class(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t type = luaL_checkinteger(L, 1);
    bool b = false;

    /*    if(!lua_isnoneornil(L, 2))
        {
            const char *opts = luaL_checkstring(L, 2);
            b = strchr(opts, 't') ? true : false;
        }*/

    if (lua_gettop(L) > 1) b = lua_toboolean(L, 2);

    if (!ecs_get(w, type, EcsMetaTypeSerialized)) luaL_argerror(L, 1, "no metatype for component");
    char *str = str_type_ops(w, type, 0);  // ecs_type_to_emmylua(w, type, b);

    lua_pushstring(L, str);

    ecs_os_free(str);

    return 1;
}

static const char *checkname(lua_State *L, int arg) {
    int type = lua_type(L, arg);

    if (type == LUA_TSTRING)
        return luaL_checkstring(L, arg);
    else if (type != LUA_TNIL)
        luaL_argerror(L, arg, "expected string or nil for name");

    return NULL;
}

int new_entity(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = 0;

    const char *name = NULL;
    const char *components = NULL;
    int args = lua_gettop(L);

    if (!args) {
        e = ecs_new_id(w);
    } else if (args == 1) /* entity | name(string) */
    {
        int type = lua_type(L, 1);

        if (lua_isinteger(L, 1))
            e = luaL_checkinteger(L, 1);
        else if (type == LUA_TSTRING)
            name = luaL_checkstring(L, 1);
        else
            return luaL_argerror(L, 1, "expected entity or name");
    } else if (args == 2) {
        if (lua_isinteger(L, 1)) /* entity, name (string) */
        {
            luaL_checktype(L, 2, LUA_TSTRING);

            e = luaL_checkinteger(L, 1);
            name = luaL_checkstring(L, 2);
        } else /* name (string|nil), components */
        {
            name = checkname(L, 1);
            components = luaL_checkstring(L, 2);
        }
    } else if (args == 3) /* entity, name (string|nil), components */
    {
        e = luaL_checkinteger(L, 1);
        name = checkname(L, 2);
        components = luaL_checkstring(L, 3);
    } else
        return luaL_error(L, "too many arguments");

    if (e && ecs_is_alive(w, e) && name) { /* ecs.new(123, "name") is idempotent, components are ignored */
        const char *existing = ecs_get_name(w, e);

        if (existing) {
            if (!strcmp(existing, name)) {
                lua_pushinteger(L, e);
                return 1;
            }

            return luaL_error(L, "entity redefined with different name");
        }
    }

    if (!e && name) { /* ecs.new("name") is idempontent, components are ignored */
        e = ecs_lookup(w, name);

        if (e) {
            lua_pushinteger(L, e);
            return 1;
        }
    }

    /* create an entity, the following functions will take the same id */
    if (!e && args) e = ecs_new_id(w);

    if (e && !ecs_is_alive(w, e)) ecs_ensure(w, e);

    ecs_entity_t scope = ecs_get_scope(w);
    if (scope) ecs_add_pair(w, e, EcsChildOf, scope);

    if (components) {
        ecs_entity_desc_t desc = {
                .id = e, .add_expr = components,
                // XXX do we have to add it?
                //.add = { scope ? ecs_pair(EcsChildOf, scope) : 0 },
        };

        e = ecs_entity_init(w, &desc);
    }

    if (name) ecs_set_name(w, e, name);

    lua_pushinteger(L, e);

    return 1;
}

int new_id(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t id = ecs_new_id(w);

    lua_pushinteger(L, id);

    return 1;
}

int delete_entity(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity;

    if (lua_isinteger(L, 1)) {
        entity = luaL_checkinteger(L, 1);
        ecs_delete(w, entity);
    } else if (lua_type(L, 1) == LUA_TTABLE) {
        int len = lua_rawlen(L, 1);
        int i;
        for (i = 0; i < len; i++) {
            lua_rawgeti(L, 1, i + 1);
            entity = luaL_checkinteger(L, -1);
            ecs_delete(w, entity);
            lua_pop(L, 1);
        }

        return 0;
    }

    return 0;
}

int new_tag(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup(w, name);

    if (!e) e = ecs_set_name(w, e, name);

    lua_pushinteger(L, e);

    return 1;
}

int entity_name(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const char *name = ecs_get_name(w, e);

    lua_pushstring(L, name);

    return 1;
}

int set_name(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    const char *name = lua_isnoneornil(L, 2) ? NULL : luaL_checkstring(L, 2);

    e = ecs_set_name(w, e, name);

    lua_pushinteger(L, e);

    return 1;
}

int entity_symbol(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const char *symbol = ecs_get_symbol(w, e);

    if (!symbol) return 0;

    lua_pushstring(L, symbol);

    return 1;
}

int entity_fullpath(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    char *path = ecs_get_fullpath(w, e);

    lua_pushstring(L, path);

    ecs_os_free(path);

    return 1;
}

int lookup_entity(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup(w, name);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_child(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);

    ecs_entity_t e = ecs_lookup_child(w, parent, name);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_path(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = luaL_checkinteger(L, 1);
    const char *path = luaL_checkstring(L, 2);
    const char *sep = luaL_optstring(L, 3, ".");
    const char *prefix = luaL_optstring(L, 4, NULL);

    ecs_entity_t e = ecs_lookup_path_w_sep(w, parent, path, sep, prefix, false);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_fullpath(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup_fullpath(w, name);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_symbol(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup_symbol(w, name, true, false);

    lua_pushinteger(L, e);

    return 1;
}

int use_alias(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);

    ecs_set_alias(w, e, name);

    return 0;
}

int entity_has(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int args = lua_gettop(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    int b;

    if (args == 3) {
        ecs_entity_t relation = luaL_checkinteger(L, 2);
        ecs_entity_t object = luaL_checkinteger(L, 3);
        b = ecs_has_pair(w, e, relation, object);
    } else {
        ecs_entity_t to_check = luaL_checkinteger(L, 2);
        b = ecs_has_id(w, e, to_check);
    }

    lua_pushboolean(L, b);

    return 1;
}

int entity_owns(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t id = luaL_checkinteger(L, 2);

    int b = ecs_owns_id(w, e, id);

    lua_pushboolean(L, b);

    return 1;
}

int is_alive(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    int b = ecs_is_alive(w, e);

    lua_pushboolean(L, b);

    return 1;
}

int is_valid(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    int b = ecs_is_valid(w, e);

    lua_pushboolean(L, b);

    return 1;
}

int get_alive(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_entity_t id = ecs_get_alive(w, e);

    lua_pushinteger(L, id);

    return 1;
}

int ensure(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_ensure(w, e);

    return 0;
}

int exists(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    int b = ecs_exists(w, e);

    lua_pushboolean(L, b);

    return 1;
}

int entity_add(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int args = lua_gettop(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    if (args == 3) {
        ecs_entity_t relation = luaL_checkinteger(L, 2);
        ecs_entity_t object = luaL_checkinteger(L, 3);
        ecs_add_pair(w, e, relation, object);
    } else /* add(e, integer) */
    {
        ecs_entity_t to_add = luaL_checkinteger(L, 2);
        ecs_add_id(w, e, to_add);
    }

    return 0;
}

int entity_remove(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int args = lua_gettop(L);
    ecs_entity_t e = luaL_checkinteger(L, 1);

    if (args == 3) {
        ecs_entity_t relation = luaL_checkinteger(L, 2);
        ecs_entity_t object = luaL_checkinteger(L, 3);
        ecs_remove_pair(w, e, relation, object);
    } else {
        ecs_entity_t to_remove = luaL_checkinteger(L, 2);
        ecs_remove_id(w, e, to_remove);
    }

    return 0;
}

int clear_entity(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    ecs_clear(w, e);

    return 0;
}

int enable_entity(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    if (lua_gettop(L) > 1) {
        ecs_entity_t c = lua_tointeger(L, 2);
        ecs_enable_id(w, e, c, true);
    } else
        ecs_enable(w, e, true);

    return 0;
}

int disable_entity(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    if (lua_gettop(L) > 1) {
        ecs_entity_t c = lua_tointeger(L, 2);
        ecs_enable_id(w, e, c, false);
    } else
        ecs_enable(w, e, false);

    return 0;
}

int entity_count(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    int32_t count = ecs_count_id(w, e);

    lua_pushinteger(L, count);

    return 1;
}

int delete_children(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = lua_tointeger(L, 1);

    ecs_delete_children(w, parent);

    return 0;
}

int get_type(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = 0;
    const ecs_type_t *type = NULL;
    int from_entity = 0;

    e = luaL_checkinteger(L, 1);
    from_entity = lua_toboolean(L, 2);

    if (from_entity)
        type = ecs_get_type(w, e);
    else
        type = ecs_get_type(w, e);

    if (type) {
        void *ptr = lua_newuserdata(L, sizeof(ecs_type_t *));
        memcpy(ptr, &type, sizeof(ecs_type_t *));

        luaL_setmetatable(L, "ecs_type_t");
    } else
        lua_pushnil(L);

    return 1;
}

int get_typeid(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_entity_t type_id = ecs_get_typeid(w, e);

    lua_pushinteger(L, type_id);

    return 1;
}

int get_parent(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_entity_t parent = ecs_get_target(w, e, EcsChildOf, 0);

    lua_pushinteger(L, parent);

    return 1;
}

int enable_component(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_enable_id(w, e, c, true);

    return 0;
}

int disable_component(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_enable_id(w, e, c, false);

    return 0;
}

int is_component_enabled(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    int b = ecs_is_enabled_id(w, e, c);

    lua_pushboolean(L, b);

    return 1;
}

int add_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    ecs_add_id(w, e, ecs_pair(c, t));

    return 0;
}

int remove_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    ecs_remove_id(w, e, ecs_pair(c, t));

    return 0;
}

int has_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    int b = ecs_has_pair(w, e, c, t);

    lua_pushboolean(L, b);

    return 1;
}

int set_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    if (!e) {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if (scope) ecs_add_pair(w, e, EcsChildOf, scope);
    }

    void *ptr = ecs_get_mut_id(w, e, pair);

    ecs_lua_to_ptr(w, L, 4, relation, ptr);

    ecs_modified_id(w, e, pair);

    lua_pushinteger(L, e);

    return 1;
}

int set_pair_object(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    if (!e) {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if (scope) ecs_add_pair(w, e, EcsChildOf, scope);
    }

    void *ptr = ecs_get_mut_id(w, e, pair);

    ecs_lua_to_ptr(w, L, 4, object, ptr);

    ecs_modified_id(w, e, pair);

    lua_pushinteger(L, e);

    return 0;
}

int get_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    const void *ptr = ecs_get_id(w, e, pair);

    if (ptr)
        ecs_ptr_to_lua(w, L, relation, ptr);
    else
        lua_pushnil(L);

    return 1;
}

int get_mut_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    void *ptr = ecs_get_mut_id(w, e, pair);

    if (ptr)
        ecs_ptr_to_lua(w, L, relation, ptr);
    else
        lua_pushnil(L);

    return 1;
}

int get_pair_object(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    const void *ptr = ecs_get_id(w, e, pair);

    if (ptr)
        ecs_ptr_to_lua(w, L, object, ptr);
    else
        lua_pushnil(L);

    return 1;
}

int get_mut_pair_object(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    void *ptr = ecs_get_mut_id(w, e, pair);

    if (ptr)
        ecs_ptr_to_lua(w, L, object, ptr);
    else
        lua_pushnil(L);

    return 1;
}

int make_pair(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t predicate = luaL_checkinteger(L, 1);
    ecs_entity_t object = luaL_checkinteger(L, 2);

    ecs_entity_t pair = ecs_pair(predicate, object);

    lua_pushinteger(L, pair);

    return 1;
}

int is_pair(lua_State *L) {
    ecs_entity_t id = luaL_checkinteger(L, 1);

    int b = ECS_IS_PAIR(id);

    lua_pushboolean(L, b);

    return 1;
}

int pair_object(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t pair = luaL_checkinteger(L, 1);

    ecs_entity_t object = ecs_pair_object(w, pair);

    lua_pushinteger(L, object);

    return 1;
}

int add_instanceof(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t base = luaL_checkinteger(L, 2);

    ecs_add_pair(w, entity, EcsIsA, base);

    return 0;
}

int remove_instanceof(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t base = luaL_checkinteger(L, 2);

    ecs_remove_pair(w, entity, EcsIsA, base);

    return 0;
}

int add_childof(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t parent = luaL_checkinteger(L, 2);

    ecs_add_pair(w, entity, EcsChildOf, parent);

    return 0;
}

int remove_childof(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t parent = luaL_checkinteger(L, 2);

    ecs_remove_pair(w, entity, EcsChildOf, parent);

    return 0;
}

int entity_override(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    ecs_add_id(w, entity, ECS_OVERRIDE | component);

    return 0;
}

static void init_scope(ecs_world_t *w, ecs_entity_t id) {
    ecs_entity_t scope = ecs_get_scope(w);

    if (scope) ecs_add_pair(w, id, EcsChildOf, scope);
}

int new_enum(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    if (ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_desc_t com_ent_des{.use_low_id = true};
    ecs_entity_t component = ecs_entity_init(w, &com_ent_des);

    ecs_set_name(w, component, name);

    int err = ecs_meta_from_desc(w, component, EcsEnumType, desc);

    if (err) return luaL_argerror(L, 2, "invalid descriptor");

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_bitmask(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    if (ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_desc_t com_ent_des{.use_low_id = true};
    ecs_entity_t component = ecs_entity_init(w, &com_ent_des);

    ecs_set_name(w, component, name);

    int err = ecs_meta_from_desc(w, component, EcsBitmaskType, desc);

    if (err) return luaL_argerror(L, 2, "invalid descriptor");

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_array(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    ecs_entity_t element = luaL_checkinteger(L, 2);
    s32 count = luaL_checkinteger(L, 3);

    if (count < 0 || count > INT32_MAX) luaL_error(L, "element count out of range (%I)", count);

    if (ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_array_desc_t desc = {.type = element, .count = count};

    ecs_entity_t component = ecs_array_init(w, &desc);

    ecs_set_name(w, component, name);

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_struct(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    if (ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_desc_t com_ent_des{.use_low_id = true};
    ecs_entity_t component = ecs_entity_init(w, &com_ent_des);

    ecs_set_name(w, component, name);

    int err = ecs_meta_from_desc(w, component, EcsStructType, desc);

    if (err) return luaL_argerror(L, 2, "invalid descriptor");

    EcsMetaType mt_des{.kind = EcsStructType};
    ecs_set_ptr(w, component, EcsMetaType, &mt_des);

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_alias(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *alias = luaL_checkstring(L, 2);

    ecs_entity_t type_entity = ecs_lookup_fullpath(w, name);

    if (!type_entity) return luaL_argerror(L, 1, "component does not exist");

    if (!ecs_has(w, type_entity, EcsComponent)) return luaL_argerror(L, 1, "not a component");

    const EcsMetaType *meta = ecs_get(w, type_entity, EcsMetaType);

    if (!meta) return luaL_argerror(L, 1, "missing descriptor");

    if (ecs_lookup_fullpath(w, alias) || ecs_lookup(w, alias)) return luaL_argerror(L, 2, "alias already exists");

    ecs_entity_desc_t com_ent_des{.use_low_id = true};
    ecs_entity_t component = ecs_entity_init(w, &com_ent_des);

    ecs_set_name(w, component, alias);

    /* XXX: copy components? */

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int get_func(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    const void *ptr = ecs_get_id(w, e, component);

    if (ptr)
        ecs_ptr_to_lua(w, L, component, ptr);
    else
        lua_pushnil(L);

    return 1;
}

static int get_mutable(ecs_world_t *w, lua_State *L, ecs_entity_t e, ecs_entity_t component) {
    void *ptr = ecs_get_mut_id(w, e, component);

    if (ptr)
        ecs_ptr_to_lua(w, L, component, ptr);
    else
        lua_pushnil(L);

    return 1;
}

int get_mut(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    return get_mutable(w, L, e, component);
}

int patch_func(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);

    void *ptr = ecs_get_mut_id(w, e, component);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    return 0;
}

int set_func(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    if (!e) {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if (scope) ecs_add_pair(w, e, EcsChildOf, scope);
    }

    void *ptr = ecs_get_mut_id(w, e, component);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    ecs_modified_id(w, e, component);

    lua_pushinteger(L, e);

    return 1;
}

int new_ref(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);  // TODO: verify ref world vs api world
    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_ref_t *ref = (ecs_ref_t *)lua_newuserdata(L, sizeof(ecs_ref_t));
    luaL_setmetatable(L, "ecs_ref_t");

    *ref = ecs_ref_init_id(w, e, c);

    return 1;
}

int get_ref(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_ref_t *ref = (ecs_ref_t *)luaL_checkudata(L, 1, "ecs_ref_t");
    ecs_entity_t id = ref->id;

    if (lua_gettop(L) > 1) id = luaL_checkinteger(L, 2);

    // if(!id || !ref->id || id == ref->id) return luaL_argerror(L, )
    ecs_lua_assert(L, !id || !ref->id || id == ref->id, NULL);

    const void *ptr = ecs_ref_get_id(w, ref, id);

    ecs_ptr_to_lua(w, L, ref->id, ptr);

    return 1;
}

int singleton_get(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    const void *ptr = ecs_get_id(w, component, component);

    if (ptr)
        ecs_ptr_to_lua(w, L, component, ptr);
    else
        lua_pushnil(L);

    return 1;
}

int singleton_patch(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    void *ptr = ecs_get_mut_id(w, e, e);

    ecs_lua_to_ptr(w, L, 2, e, ptr);

    ecs_modified_id(w, e, e);

    return 1;
}

int singleton_set(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    void *ptr = ecs_get_mut_id(w, component, component);

    ecs_lua_to_ptr(w, L, 2, component, ptr);

    ecs_modified_id(w, component, component);

    return 1;
}

int new_prefab(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = 0;

    int args = lua_gettop(L);

    if (!args) {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if (scope) ecs_add_pair(w, e, EcsChildOf, scope);
        ecs_add_id(w, e, EcsPrefab);
    } else if (args <= 2) {
        const char *id = luaL_checkstring(L, 1);
        const char *sig = luaL_optstring(L, 2, NULL);

        ecs_entity_desc_t ent_des{.name = id, .add = {EcsPrefab}, .add_expr = sig};
        e = ecs_entity_init(w, &ent_des);
    } else
        return luaL_argerror(L, args, "too many arguments");

    lua_pushinteger(L, e);

    return 1;
}

ecs_iter_t *ecs_lua__checkiter(lua_State *L, int arg) {
    if (luaL_getmetafield(L, arg, "__ecs_iter") == LUA_TNIL) luaL_argerror(L, arg, "table is not an iterator");

    ecs_iter_t *it = (ecs_iter_t *)lua_touserdata(L, -1);
    lua_pop(L, 1);

    return it;
}

static inline void copy_term_id(ecs_term_id_t *dst, EcsLuaTermID *src) {
    dst->id = src->id;
    dst->trav = src->trav;
    dst->flags = src->flags;
}

static inline void copy_term(ecs_term_t *dst, EcsLuaTerm *src) {
    dst->id = src->id;

    dst->inout = (ecs_inout_kind_t)src->inout;
    copy_term_id(&dst->src, &src->src);
    copy_term_id(&dst->first, &src->first);
    copy_term_id(&dst->second, &src->second);
    dst->oper = (ecs_oper_kind_t)src->oper;
}

ecs_term_t checkterm(lua_State *L, const ecs_world_t *world, int arg) {
    ecs_term_t term = {0};

    int type = lua_type(L, arg);

    if (type == LUA_TTABLE) {
        EcsLuaTerm lua_term = {0};

        ecs_lua_to_ptr(world, L, arg, ecs_id(EcsLuaTerm), &lua_term);

        copy_term(&term, &lua_term);
    } else {
        term.id = luaL_checkinteger(L, arg);
    }

    if (ecs_term_finalize(world, &term)) luaL_argerror(L, arg, "invalid term");

    if (term.id == 0) luaL_argerror(L, arg, "empty term");

    return term;
}

static ecs_iter_t *get_iter_columns(lua_State *L, int arg) {
    ecs_iter_t *it = ecs_lua__checkiter(L, arg);

    luaL_getsubtable(L, arg, "columns");

    return it;
}

int iter_term(lua_State *L) {
    ecs_iter_t *it = get_iter_columns(L, 1);

    lua_Integer i = luaL_checkinteger(L, 2);

    if (i < 1 || i > it->field_count) return luaL_argerror(L, 2, "invalid term index");

    lua_geti(L, -1, i);

    return 1;
}

int iter_terms(lua_State *L) {
    ecs_iter_t *it = get_iter_columns(L, 1);

    int i;
    for (i = 1; i <= it->field_count; i++) {
        lua_geti(L, 2, i);
    }

    lua_getfield(L, 1, "entities");

    return it->field_count + 1;
}

int is_owned(lua_State *L) {
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if (col < 1 || col > it->field_count) luaL_argerror(L, 2, "invalid field index");

    int b = ecs_field_is_self(it, col);

    lua_pushboolean(L, b);

    return 1;
}

int term_id(lua_State *L) {
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if (col < 1 || col > it->field_count) luaL_argerror(L, 2, "invalid field index");

    ecs_entity_t e = ecs_field_id(it, col);

    lua_pushinteger(L, e);

    return 1;
}

int filter_iter(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_filter_t *filter = checkfilter(L, w, 1);

    ecs_iter_t it = ecs_filter_iter(w, filter);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int filter_next(lua_State *L) {
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_filter_next(it);

    if (b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

int term_iter(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_term_t term = checkterm(L, w, 1);

    ecs_iter_t it = ecs_term_iter(w, &term);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int term_next(lua_State *L) {
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_term_next(it);

    if (b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

bool ecs_lua_iter_next(lua_State *L, int idx);

int iter_next(lua_State *L) {
    int b = ecs_lua_iter_next(L, 1);

    lua_pushboolean(L, b);

    return 1;
}

int get_child_count(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_term_t it_term{ecs_pair(EcsChildOf, e)};
    ecs_iter_t it = ecs_term_iter(w, &it_term);

    int32_t count = 0;

    while (ecs_term_next(&it)) {
        count += it.count;
    }

    lua_pushinteger(L, count);

    return 1;
}

int set_scope(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t scope = luaL_checkinteger(L, 1);

    ecs_entity_t prev = ecs_set_scope(w, scope);

    lua_pushinteger(L, prev);

    return 1;
}

int get_scope(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = ecs_get_scope(w);

    lua_pushinteger(L, e);

    return 1;
}

int set_name_prefix(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    const char *prefix = NULL;

    if (!lua_isnil(L, 1)) prefix = luaL_checkstring(L, 1);

    const char *prev = ecs_set_name_prefix(w, prefix);

    lua_pushstring(L, prev);

    if (ctx->prefix_ref != LUA_NOREF) {
        ecs_lua_unref(L, w, ctx->prefix_ref);
        ctx->prefix_ref = LUA_NOREF;
    }

    lua_pushvalue(L, 1);
    ctx->prefix_ref = ecs_lua_ref(L, w);

    return 1;
}

typedef struct ecs_lua_col_t {
    ecs_entity_t type;
    size_t stride;
    bool readback, update;
    void *ptr;
    const EcsMetaTypeSerialized *ser;
    ecs_meta_cursor_t *cursor;
} ecs_lua_col_t;

typedef struct ecs_lua_each_t {
    ecs_iter_t *it;
    int32_t i;
    bool from_query, read_prev;
    ecs_lua_col_t cols[];
} ecs_lua_each_t;

static void serialize_type_op(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, lua_State *L);

static void serialize_type(const ecs_world_t *world, const ecs_vec_t *v_ops, const void *base, lua_State *L);

static void serialize_elements(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, int32_t elem_count, int32_t elem_size, lua_State *L);

static void serialize_type_ops(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, int32_t in_array, lua_State *L) {
    int i, depth = 0;

    for (i = 0; i < op_count; i++) {
        ecs_meta_type_op_t *op = &ops[i];

        if (in_array <= 0) {
            int32_t elem_count = op->count;

            if (elem_count > 1) { /* serialize inline array */
                serialize_elements(world, op, op->op_count, base, op->count, op->size, L);

                if (op->name) lua_setfield(L, -2, op->name);

                i += op->op_count - 1;
                continue;
            }
        }

        switch (op->kind) {
            case EcsOpPush: {
                depth++;
                in_array--;

                if (depth > 1) {
                    ecs_assert(op->name != NULL, ECS_INVALID_PARAMETER, NULL);
                    lua_pushstring(L, op->name);
                }

                int32_t member_count = ecs_map_count(&op->members->impl);
                lua_createtable(L, 0, member_count);
                break;
            }
            case EcsOpPop: {
                if (depth > 1) lua_settable(L, -3);

                depth--;
                in_array++;
                break;
            }
            default: {
                serialize_type_op(world, op, base, L);

                if (op->name && (in_array <= 0)) {
                    lua_setfield(L, -2, op->name);
                }

                break;
            }
        }
    }
}

static void serialize_primitive(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, lua_State *L) {
    switch (op->kind) {
        case EcsBool:
            lua_pushboolean(L, (int)*(bool *)base);
            break;
        case EcsChar:
            lua_pushinteger(L, *(char *)base);
            break;
        case EcsString:
            lua_pushstring(L, *(char **)base);
            break;
        case EcsByte:
            lua_pushinteger(L, *(uint8_t *)base);
            break;
        case EcsU8:
            lua_pushinteger(L, *(uint8_t *)base);
            break;
        case EcsU16:
            lua_pushinteger(L, *(uint16_t *)base);
            break;
        case EcsU32:
            lua_pushinteger(L, *(uint32_t *)base);
            break;
        case EcsU64:
            lua_pushinteger(L, *(uint64_t *)base);
            break;
        case EcsI8:
            lua_pushinteger(L, *(int8_t *)base);
            break;
        case EcsI16:
            lua_pushinteger(L, *(int16_t *)base);
            break;
        case EcsI32:
            lua_pushinteger(L, *(int32_t *)base);
            break;
        case EcsI64:
            lua_pushinteger(L, *(int64_t *)base);
            break;
        case EcsF32:
            lua_pushnumber(L, *(float *)base);
            break;
        case EcsF64:
            lua_pushnumber(L, *(double *)base);
            break;
        case EcsEntity:
            lua_pushinteger(L, *(ecs_entity_t *)base);
            break;
        case EcsIPtr:
            lua_pushinteger(L, *(intptr_t *)base);
            break;
        case EcsUPtr:
            lua_pushinteger(L, *(uintptr_t *)base);
            break;
        default:
            luaL_error(L, "unknown primitive (%d)", op->kind);
    }
}

static void serialize_elements(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, int32_t elem_count, int32_t elem_size, lua_State *L) {
    const void *ptr = base;

    lua_createtable(L, elem_count, 0);

    int i;
    for (i = 0; i < elem_count; i++) {
        serialize_type_ops(world, ops, op_count, ptr, 1, L);
        lua_rawseti(L, -2, i + 1);

        ptr = ECS_OFFSET(ptr, elem_size);
    }
}

static void serialize_type_elements(const ecs_world_t *world, ecs_entity_t type, const void *base, int32_t elem_count, lua_State *L) {
    const EcsMetaTypeSerialized *ser = ecs_get(world, type, EcsMetaTypeSerialized);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    const EcsComponent *comp = ecs_get(world, type, EcsComponent);
    ecs_assert(comp != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);
    int32_t op_count = ecs_vec_count(&ser->ops);

    serialize_elements(world, ops, op_count, base, elem_count, comp->size, L);
}

static void serialize_array(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, lua_State *L) {
    const EcsArray *a = ecs_get(world, op->type, EcsArray);
    ecs_assert(a != NULL, ECS_INTERNAL_ERROR, NULL);

    serialize_type_elements(world, a->type, base, op->count, L);
}

static void serialize_vector(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, lua_State *L) {
    ecs_vec_t *value = *(ecs_vec_t **)base;

    if (!value) {
        lua_pushnil(L);
        return;
    }

    const EcsVector *v = ecs_get(world, op->type, EcsVector);
    ecs_assert(v != NULL, ECS_INTERNAL_ERROR, NULL);

    const EcsComponent *comp = ecs_get(world, v->type, EcsComponent);
    ecs_assert(comp != NULL, ECS_INTERNAL_ERROR, NULL);

    int32_t count = ecs_vec_count(value);
    void *array = ecs_vec_first(value);

    serialize_type_elements(world, v->type, array, count, L);
}

static void serialize_type_op(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, lua_State *L) {
    switch (op->kind) {
        case EcsOpPush:
        case EcsOpPop:
            ecs_abort(ECS_INVALID_PARAMETER, NULL);
            break;

        case EcsOpEnum:
        case EcsOpBitmask:
            lua_pushinteger(L, *(int32_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpArray:
            serialize_array(world, op, ECS_OFFSET(base, op->offset), L);
            break;
        case EcsOpVector:
            serialize_vector(world, op, ECS_OFFSET(base, op->offset), L);
            break;
            // case EcsOpMap:
            //  serialize_map(world, op, ECS_OFFSET(base, op->offset), L);
            // break;

        case EcsOpPrimitive:
            serialize_primitive(world, op, ECS_OFFSET(base, op->offset), L);
            break;
            //////////////////////////
        case EcsOpBool:
            lua_pushboolean(L, (int)*(bool *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpChar:
            lua_pushinteger(L, *(char *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpByte:
            lua_pushinteger(L, *(uint8_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpU8:
            lua_pushinteger(L, *(uint8_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpU16:
            lua_pushinteger(L, *(uint16_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpU32:
            lua_pushinteger(L, *(uint32_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpU64:
            lua_pushinteger(L, *(uint64_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpI8:
            lua_pushinteger(L, *(int8_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpI16:
            lua_pushinteger(L, *(int16_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpI32:
            lua_pushinteger(L, *(int32_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpI64:
            lua_pushinteger(L, *(int64_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpF32:
            lua_pushnumber(L, *(float *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpF64:
            lua_pushnumber(L, *(double *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpEntity:
            lua_pushinteger(L, *(ecs_entity_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpIPtr:
            lua_pushinteger(L, *(intptr_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpUPtr:
            lua_pushinteger(L, *(uintptr_t *)ECS_OFFSET(base, op->offset));
            break;
        case EcsOpString:
            lua_pushstring(L, *(char **)ECS_OFFSET(base, op->offset));
            break;
    }
}

static void serialize_constants(ecs_world_t *world, ecs_meta_type_op_t *op, lua_State *L, const char *prefix, bool lowercase) {
    const ecs_map_t *map = NULL;
    const EcsEnum *enum_type = NULL;
    const EcsBitmask *bitmask_type = NULL;

    enum_type = ecs_get(world, op->type, EcsEnum);

    if (enum_type != NULL) {
        map = &enum_type->constants;
    } else {
        bitmask_type = ecs_get(world, op->type, EcsBitmask);
        ecs_assert(bitmask_type != NULL, ECS_INVALID_PARAMETER, NULL);

        map = &bitmask_type->constants;
    }

    void *ptr;
    ecs_map_key_t key;
    ecs_map_iter_t it = ecs_map_iter(map);

    const char *name;
    lua_Integer value;

    while (ecs_map_next(&it)) {
        ptr = ecs_map_ptr(&it);

        if (enum_type != NULL) {
            ecs_enum_constant_t *constant = (ecs_enum_constant_t *)ptr;
            name = constant->name;
            value = constant->value;
        } else {
            ecs_bitmask_constant_t *constant = (ecs_bitmask_constant_t *)ptr;
            name = constant->name;
            value = constant->value;
        }

        if (prefix) {
            const char *ptr = strstr(name, prefix);
            size_t len = strlen(prefix);

            if (ptr == name && ptr[len] != '\0') {
                name = ptr + len;
            }
        }

        if (lowercase) {
            char *p = ecs_os_strdup(name);

            int i;
            for (i = 0; p[i] != '\0'; i++) {
                p[i] = tolower(p[i]);
            }

            lua_pushstring(L, p);

            ecs_os_free(p);
        } else
            lua_pushstring(L, name);

        lua_pushinteger(L, value);

        lua_settable(L, -3);
    }
}

static void serialize_type(const ecs_world_t *world, const ecs_vec_t *v_ops, const void *base, lua_State *L) {
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(v_ops);
    int32_t count = ecs_vec_count(v_ops);

    serialize_type_ops(world, ops, count, base, 0, L);
}

static void update_type(const ecs_world_t *world, const ecs_vec_t *ser, const void *base, lua_State *L, int idx) {
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(ser);
    int32_t count = ecs_vec_count(ser);

    lua_pushvalue(L, idx);

    int i, depth = 0;

    for (i = 0; i < count; i++) {
        ecs_meta_type_op_t *op = &ops[i];

        switch (op->kind) {
            case EcsOpPush: {
                depth++;
                if (depth > 1) {
                    ecs_assert(op->name != NULL, ECS_INVALID_PARAMETER, NULL);
                    int t = lua_getfield(L, -1, op->name);
                    if (t != LUA_TTABLE) {
                        lua_pop(L, 1);
                        lua_newtable(L);
                        lua_pushvalue(L, -1);
                        lua_setfield(L, -3, op->name);
                    }
                }
                break;
            }
            case EcsOpPop: {
                if (depth > 1) lua_pop(L, 1);
                depth--;
                break;
            }
            default: {
                serialize_type_op(world, op, base, L);
                if (op->name && op != ops) lua_setfield(L, -2, op->name);
                break;
            }
        }
    }

    lua_pop(L, 1);
}

static void deserialize_type(lua_State *L, int idx, ecs_meta_cursor_t *c, int depth) {
    int ktype, vtype, ret, mtype, prev_ikey;
    bool in_array = false, designated_initializer = false;

    idx = lua_absindex(L, idx);

    vtype = lua_type(L, idx);
    mtype = 0;
    prev_ikey = 0;

    if (vtype == LUA_TTABLE) {
        ret = ecs_meta_push(c);
        ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

        in_array = ecs_meta_is_collection(c);

        lua_pushnil(L);
    } else
        luaL_checktype(L, idx, LUA_TTABLE);  // Error out (only struct/array components supported for now)

    while (lua_next(L, idx)) {
        ktype = lua_type(L, -2);
        vtype = lua_type(L, -1);

        if (!mtype) mtype = ktype;

        if (ktype == LUA_TSTRING) {
            const char *key = lua_tostring(L, -2);

            ecs_lua_dbg_pad(depth);
            ecs_lua_dbg("meta_member field: %s", key);

            ret = ecs_meta_member(c, key);

            if (ret) luaL_error(L, "field \"%s\" does not exist", key);
            if (mtype != ktype) luaL_error(L, "table has mixed key types (string key '%s')", key);
        } else if (ktype == LUA_TNUMBER) {
            lua_Integer key = lua_tointeger(L, -2) - 1;

            ecs_lua_dbg_pad(depth);
            ecs_lua_dbg("move idx: %lld", key);

            // prefer not to trigger error messages in flecs
            // XXX: double check we're never entering designated init mode after a string key was found
            if (in_array) {
                ret = ecs_meta_elem(c, key);
            } else if (designated_initializer) {
                if (key == (prev_ikey + 1)) {
                    ret = ecs_meta_next(c);
                    prev_ikey = key;
                } else
                    ret = 1;
            } else if (!key) {
                designated_initializer = true;
            } else  // designated initializer does not start with 0
            {
                ret = 1;
            }

            if (ret) luaL_error(L, "invalid index %I (Lua [%I])", key, key + 1);
            if (mtype != ktype) luaL_error(L, "table has mixed key types (int key [%I]", key + 1);  // XXX: move this up?
        } else
            luaL_error(L, "invalid key type '%s'", lua_typename(L, ktype));

        switch (vtype) {
            case LUA_TTABLE: {
                ecs_lua_dbg_pad(depth);
                ecs_lua_dbg("meta_push (nested)");
                // ecs_meta_push(c);
                // lua_pushnil(L);
                depth++;
                int top = lua_gettop(L);
                deserialize_type(L, top, c, depth);
                depth--;
                break;
            }
            case LUA_TNUMBER: {
                if (lua_isinteger(L, -1)) {
                    lua_Integer integer = lua_tointeger(L, -1);

                    ecs_lua_dbg_pad(depth);
                    ecs_lua_dbg("set_int: %lld", integer);

                    /* XXX: Workaround for U64 bounds check */
                    ecs_meta_scope_t *scope = &c->scope[c->depth];
                    ecs_meta_type_op_t *op = &scope->ops[scope->op_cur];

                    if (op->kind == EcsOpU64)
                        ret = ecs_meta_set_uint(c, integer);
                    else
                        ret = ecs_meta_set_int(c, integer);

                    if (ret) luaL_error(L, "failed to set integer (%I)", integer);
                } else {
                    lua_Number number = lua_tonumber(L, -1);

                    ecs_lua_dbg_pad(depth);
                    ecs_lua_dbg("set_float %f", number);
                    ret = ecs_meta_set_float(c, number);

                    if (ret) luaL_error(L, "failed to set float (%f)", number);
                }

                break;
            }
            case LUA_TBOOLEAN: {
                ecs_lua_dbg_pad(depth);
                ecs_lua_dbg("set_bool: %d", lua_toboolean(L, -1));
                ret = ecs_meta_set_bool(c, lua_toboolean(L, -1));

                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            case LUA_TSTRING: {
                ecs_lua_dbg_pad(depth);
                ecs_lua_dbg("set_string: %s", lua_tostring(L, -1));
                ret = ecs_meta_set_string(c, lua_tostring(L, -1));

                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            default: {
                if (ktype == LUA_TSTRING)
                    luaL_error(L, "invalid type for field '%s' (got %s)", lua_tostring(L, -2), lua_typename(L, vtype));
                else
                    luaL_error(L, "invalid type at index [%d] (got %s)", lua_tointeger(L, -2), lua_typename(L, vtype));
            }
        }

        lua_pop(L, 1);
    }

    ecs_lua_dbg_pad(depth);
    ecs_lua_dbg("meta_pop");
    ret = ecs_meta_pop(c);
    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
}

static void serialize_column(ecs_world_t *world, lua_State *L, const EcsMetaTypeSerialized *ser, const void *base, int32_t count) {
    int32_t op_count = ecs_vec_count(&ser->ops);
    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);

    serialize_elements(world, ops, op_count, base, count, ops->size, L);  // XXX: not sure about ops->size
}

static const EcsMetaTypeSerialized *get_serializer(lua_State *L, const ecs_world_t *world, ecs_entity_t type) {
    return ecs_get(world, type, EcsMetaTypeSerialized);
    // TODO: check if this optimization is still needed
    world = ecs_get_world(world);

    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_TYPES);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, type);

    ecs_ref_t *ref;

    if (ret != LUA_TNIL) {
        ecs_assert(ret == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

        ref = (ecs_ref_t *)lua_touserdata(L, -1);

        lua_pop(L, 3);
    } else {
        lua_pop(L, 1); /* -nil */
        ref = (ecs_ref_t *)lua_newuserdata(L, sizeof(ecs_ref_t));
        lua_rawseti(L, -2, type);

        *ref = ecs_ref_init_id(world, type, ecs_id(EcsMetaTypeSerialized));

        lua_pop(L, 2); /* -types, -world */
    }

    const EcsMetaTypeSerialized *ser = (EcsMetaTypeSerialized *)ecs_ref_get_id(world, ref, 0);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    return ser;
}

static int columns__len(lua_State *L) {
    ecs_iter_t *it = (ecs_iter_t *)lua_touserdata(L, lua_upvalueindex(1));

    lua_pushinteger(L, it->field_count);

    return 1;
}

static int columns__index(lua_State *L) {
    ecs_iter_t *it = (ecs_iter_t *)lua_touserdata(L, lua_upvalueindex(1));
    ecs_world_t *world = it->world;

    lua_Integer i = luaL_checkinteger(L, 2);

    if (i < 1 || i > it->field_count) luaL_argerror(L, 1, "invalid term index");

    if (!it->count) {
        lua_pushnil(L);
        return 1;
    }

    ecs_entity_t type = ecs_get_typeid(world, ecs_field_id(it, i));
    const EcsMetaTypeSerialized *ser = get_serializer(L, world, type);

    if (!ser) luaL_error(L, "term %d cannot be serialized", i);

    lua_settop(L, 1); /* (it.)columns */

    const void *base = ecs_field_w_size(it, 0, i);

    if (!ecs_field_is_self(it, i))
        serialize_type(world, &ser->ops, base, L);
    else
        serialize_column(world, L, ser, base, it->count);

    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, i);

    return 1;
}

static int entities__index(lua_State *L) {
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer i = luaL_checkinteger(L, 2);

    if (i < 1 || i > it->count) {
        if (!it->count) return luaL_error(L, "no matched entities");

        return luaL_error(L, "invalid index (%I)", i, it->count);
    }

    lua_pushinteger(L, it->entities[i - 1]);

    return 1;
}

/* expects "it" table at stack top */
static void push_columns(lua_State *L, ecs_iter_t *it) {
    if (!it->count) {
        lua_newtable(L);
        lua_setfield(L, -2, "columns");
        return;
    }

    /* it.columns[] */
    lua_createtable(L, it->field_count, 1);

    /* metatable */
    lua_createtable(L, 0, 2);

    lua_pushlightuserdata(L, it);
    lua_pushcclosure(L, columns__index, 1);
    lua_setfield(L, -2, "__index");

    lua_pushlightuserdata(L, it);
    lua_pushcclosure(L, columns__len, 1);
    lua_setfield(L, -2, "__len");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "columns");
}

/* expects "it" table at stack top */
static void push_iter_metadata(lua_State *L, ecs_iter_t *it) {
    lua_pushinteger(L, it->count);
    lua_setfield(L, -2, "count");

    lua_pushinteger(L, it->system);
    lua_setfield(L, -2, "system");

    lua_pushinteger(L, it->event);
    lua_setfield(L, -2, "event");

    lua_pushinteger(L, it->event_id);
    lua_setfield(L, -2, "event_id");

    //    lua_pushinteger(L, it->self);
    //    lua_setfield(L, -2, "self");

    lua_pushnumber(L, it->delta_time);
    lua_setfield(L, -2, "delta_time");

    lua_pushnumber(L, it->delta_system_time);
    lua_setfield(L, -2, "delta_system_time");

    lua_pushinteger(L, it->table_count);
    lua_setfield(L, -2, "table_count");

    lua_pushinteger(L, it->term_index);
    lua_setfield(L, -2, "term_index");

    if (it->system) {
        ecs_lua_callback *sys = (ecs_lua_callback *)it->binding_ctx;

        if (sys->param_ref >= 0) {
            int type = ecs_lua_rawgeti(L, it->world, sys->param_ref);
            ecs_assert(type != LUA_TNIL, ECS_INTERNAL_ERROR, NULL);
        } else
            lua_pushnil(L);

        // lua_pushvalue(L, -1);
        // lua_setfield(L, -3, "ctx");

        lua_setfield(L, -2, "param");
    }

    /* it.entities */
    lua_createtable(L, 0, 1);

    /* metatable */
    lua_createtable(L, 0, 2);

    lua_pushlightuserdata(L, it);
    lua_setfield(L, -2, "__ecs_iter");

    lua_pushcfunction(L, entities__index);
    lua_setfield(L, -2, "__index");

    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "entities");
}

/* expects table at stack top */
static ecs_iter_t *push_iter_metafield(lua_State *L, ecs_iter_t *it, bool copy) {
    /* metatable */
    lua_createtable(L, 0, 1);

    /* metatable.__ecs_iter = it */
    if (copy) {
        ecs_iter_t *ptr = (ecs_iter_t *)lua_newuserdata(L, sizeof(ecs_iter_t));
        memcpy(ptr, it, sizeof(ecs_iter_t));
        it = ptr;
    } else
        lua_pushlightuserdata(L, it);

    lua_setfield(L, -2, "__ecs_iter");
    lua_setmetatable(L, -2);

    return it;
}

/* Reset with a new base pointer */
static void meta_reset(ecs_meta_cursor_t *cursor, void *base) {
    ecs_assert(cursor != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(cursor->valid, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    cursor->depth = 0;
    cursor->scope[0].op_cur = 0;
    cursor->scope[0].elem_cur = 0;
    cursor->scope[0].ptr = base;
}

static ecs_meta_cursor_t *ecs_lua_cursor(lua_State *L, const ecs_world_t *world, ecs_entity_t type, void *base) {
    const ecs_world_t *real_world = ecs_get_world(world);

    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, real_world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_CURSORS);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, type);

    ecs_meta_cursor_t *cursor;

    if (ret == LUA_TUSERDATA) {
        cursor = (ecs_meta_cursor_t *)lua_touserdata(L, -1);
        lua_pop(L, 3);

        meta_reset(cursor, base);
    } else {
        lua_pop(L, 1);
        cursor = (ecs_meta_cursor_t *)lua_newuserdata(L, sizeof(ecs_meta_cursor_t));
        lua_rawseti(L, -2, type);
        lua_pop(L, 2);

        *cursor = ecs_meta_cursor(world, type, base);
    }

    return cursor;
}

static void deserialize_column(const ecs_world_t *world, lua_State *L, int idx, ecs_entity_t type, void *base, size_t stride, int32_t count) {
    ecs_meta_cursor_t *c = ecs_lua_cursor(L, world, type, base);

    int j;
    for (j = 0; j < count; j++) {
        meta_reset(c, (char *)base + j * stride);

        lua_rawgeti(L, idx, j + 1); /* columns[i+1][j+1] */
        deserialize_type(L, -1, c, 0);

        lua_pop(L, 1);
    }
}

void ecs_ptr_to_lua(const ecs_world_t *world, lua_State *L, ecs_entity_t type, const void *ptr) {
    const EcsMetaTypeSerialized *ser = get_serializer(L, world, type);
    serialize_type(world, &ser->ops, ptr, L);
}

void ecs_lua_to_ptr(const ecs_world_t *world, lua_State *L, int idx, ecs_entity_t type, void *ptr) {
    ecs_meta_cursor_t *c = ecs_lua_cursor(L, world, type, ptr);

    deserialize_type(L, idx, c, 0);
}

void ecs_lua_type_update(const ecs_world_t *world, lua_State *L, int idx, ecs_entity_t type, void *ptr) {
    const EcsMetaTypeSerialized *ser = get_serializer(L, world, type);

    update_type(world, &ser->ops, ptr, L, idx);
}

ecs_iter_t *ecs_iter_to_lua(ecs_iter_t *it, lua_State *L, bool copy) {
    /* it */
    lua_createtable(L, 0, 16);

    /* metatable.__ecs_iter */
    it = push_iter_metafield(L, it, copy);

    push_iter_metadata(L, it);
    push_columns(L, it);

    return it;
}

ecs_iter_t *ecs_lua_to_iter(lua_State *L, int idx) {
    ecs_lua_dbg("ECS_LUA_TO_ITER");
    ecs_lua__prolog(L);
    ecs_iter_t *it = ecs_lua__checkiter(L, idx);
    ecs_world_t *world = it->world;
    const ecs_world_t *real_world = ecs_get_world(world);

    if (lua_getfield(L, idx, "interrupted_by") == LUA_TNUMBER) it->interrupted_by = lua_tointeger(L, -1);

    lua_pop(L, 1);

    /* newly-returned iterators have it->count = 0 */
    if (!it->count) return it;

    luaL_getsubtable(L, idx, "columns");
    luaL_checktype(L, -1, LUA_TTABLE);

    int32_t i;
    for (i = 1; i <= it->field_count; i++) {
        if (it->next == ecs_query_next && ecs_field_is_readonly(it, i)) continue;

        int type = lua_rawgeti(L, -1, i); /* columns[i] */
        bool is_owned = ecs_field_is_self(it, i);

        if (type == LUA_TNIL) {
            ecs_lua_dbg("skipping empty term %d (not serialized?)", i);
            lua_pop(L, 1);
            continue;
        }

        if (is_owned) {
            ecs_assert(it->count == lua_rawlen(L, -1), ECS_INTERNAL_ERROR, NULL);
        }

        int32_t count = it->count;
        ecs_entity_t column_entity = ecs_get_typeid(world, ecs_field_id(it, i));
        void *base = ecs_field_w_size(it, 0, i);

        if (!is_owned)
            ecs_lua_to_ptr(world, L, -1, column_entity, base);
        else
            deserialize_column(world, L, -1, column_entity, base, ecs_field_size(it, i), count);

        lua_pop(L, 1); /* columns[i] */
    }

    lua_pop(L, 1); /* columns */

    ecs_lua__epilog(L);

    return it;
}

void ecs_lua_iter_update(lua_State *L, int idx, ecs_iter_t *it) {
    lua_pushvalue(L, idx);

    push_iter_metadata(L, it);

    lua_pushnil(L);
    lua_setfield(L, -2, "columns");

    push_columns(L, it);

    lua_pop(L, 1);
}

/* Progress the query iterator at the given index */
bool ecs_lua_iter_next(lua_State *L, int idx) {
    ecs_lua_dbg("ITER_NEXT");
    ecs_iter_t *it = ecs_lua_to_iter(L, idx);

    if (!ecs_iter_next(it)) return false;

    ecs_lua_iter_update(L, idx, it);

    return true;
}

int meta_constants(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t type = luaL_checkinteger(L, 1);
    const char *prefix = luaL_optstring(L, 3, NULL);
    const char *flags = luaL_optstring(L, 4, "");

    int lowercase = strchr(flags, 'l') ? 1 : 0;

    const EcsMetaType *meta = ecs_get(w, type, EcsMetaType);

    if (!meta) luaL_argerror(L, 1, "invalid type");
    if (meta->kind != EcsEnumType && meta->kind != EcsBitmaskType) luaL_argerror(L, 1, "not an enum/bitmask");

    const EcsMetaTypeSerialized *ser = get_serializer(L, w, type);
    ecs_meta_type_op_t *op = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);

    if (lua_type(L, 2) == LUA_TTABLE)
        lua_pushvalue(L, 2);
    else
        lua_newtable(L);

    serialize_constants(w, op, L, prefix, lowercase);

    return 1;
}

static void each_reset_columns(lua_State *L, ecs_lua_each_t *each) {
    ecs_iter_t *it = each->it;
    ecs_lua_col_t *col = each->cols;
    const ecs_world_t *world = it->world;

    each->i = 0;

    int i;
    for (i = 1; i <= it->field_count; i++, col++) {
        ecs_id_t field_id = ecs_field_id(it, i);
        ecs_assert(field_id != 0, ECS_INTERNAL_ERROR, NULL);

        col->type = ecs_get_typeid(world, field_id);
        col->stride = ecs_field_size(it, i);
        col->ptr = ecs_field_w_size(it, 0, i);
        col->ser = get_serializer(L, world, col->type);
        col->cursor = ecs_lua_cursor(L, it->world, col->type, col->ptr);

        if (!ecs_field_is_self(it, i)) col->stride = 0;

        if (it->next == ecs_query_next && ecs_field_is_readonly(it, i))
            col->readback = false;
        else
            col->readback = true;

        col->update = true;
    }
}

static int empty_next_func(lua_State *L) { return 0; }

static int next_func(lua_State *L) {
    ecs_lua_each_t *each = (ecs_lua_each_t *)lua_touserdata(L, lua_upvalueindex(1));
    ecs_lua_col_t *col = each->cols;
    ecs_iter_t *it = each->it;
    int idx, j, i = each->i;
    bool end = false;
    void *ptr;

    ecs_lua_dbg("each() i: %d", i);

    if (!each->read_prev) goto skip_readback;

    for (j = 0; j < it->field_count; j++, col++) {
        if (!col->readback) continue;

        ecs_lua_dbg("each() readback: %d", i - 1);

        idx = lua_upvalueindex(j + 2);
        ptr = ECS_OFFSET(col->ptr, col->stride * (i - 1));

        meta_reset(col->cursor, ptr);
        deserialize_type(L, idx, col->cursor, 0);
    }

    col = each->cols;

skip_readback:

    each->read_prev = true;

    if (i == it->count) {
        if (each->from_query) {
            if (ecs_lua_iter_next(L, 1))
                each_reset_columns(L, each);
            else
                end = true;
        } else
            end = true;
    }

    if (end) return 0;

    for (j = 0; j < it->field_count; j++, col++) {  // optimization: shared fields should be read back at the end
        if (!col->update) continue;

        idx = lua_upvalueindex(j + 2);
        ptr = ECS_OFFSET(col->ptr, col->stride * i);

        lua_pushvalue(L, idx);
        update_type(each->it->real_world, &col->ser->ops, ptr, L, idx);
    }

    lua_pushinteger(L, it->entities[i]);

    each->i++;

    return it->field_count + 1;
}

static int is_primitive(const EcsMetaTypeSerialized *ser) {
    ecs_assert(ser != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t *)ecs_vec_first(&ser->ops);
    int32_t count = ecs_vec_count(&ser->ops);

    if (count != 2) return 0;

    if (ops[1].kind = EcsOpPrimitive) return 1;

    return 0;
}

int each_func(lua_State *L) {
    ecs_lua_dbg("ecs.each()");
    ecs_world_t *w = ecs_lua_world(L);
    ecs_query_t *q = NULL;
    ecs_iter_t *it;
    int iter_idx = 1;

    if (lua_type(L, 1) == LUA_TUSERDATA) {
        q = checkquery(L, 1);
        ecs_iter_t iter = ecs_query_iter(w, q);
        int b = ecs_query_next(&iter);  // XXX: will next_func iterate and skip data?

        if (!b) /* no matching entities */
        {       /* must return an iterator, push one that ends it immediately */
            lua_pushcfunction(L, empty_next_func);
            lua_pushinteger(L, 0);
            lua_pushinteger(L, 1);
            return 3;
        }

        it = ecs_iter_to_lua(&iter, L, true);
        iter_idx = lua_gettop(L);
    } else
        it = ecs_lua__checkiter(L, 1);

    size_t size = sizeof(ecs_lua_each_t) + it->field_count * sizeof(ecs_lua_col_t);
    ecs_lua_each_t *each = (ecs_lua_each_t *)lua_newuserdata(L, size);

    each->it = it;
    each->from_query = q ? true : false;
    each->read_prev = false;

    each_reset_columns(L, each);

    int i;
    for (i = 1; i <= it->field_count; i++) {
        lua_newtable(L);

        const EcsMetaTypeSerialized *ser = each->cols[i].ser;
        // if(!ser) luaL_error(L, "col");
        // if(is_primitive(ser)) luaL_error(L, "primitive");
    }

    lua_pushcclosure(L, next_func, it->field_count + 1);

    /* it */
    lua_pushvalue(L, iter_idx);

    lua_pushinteger(L, 1);

    return 3;
}

int vararg2str(lua_State *L, int n, ecs_strbuf_t *buf) {
    lua_getglobal(L, "tostring");

    int i;
    for (i = 1; i <= n; i++) {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char *arg = lua_tostring(L, -1);

        if (!arg) return luaL_error(L, "expected string from 'tostring'");

        if (i > 1) ecs_strbuf_appendstr(buf, " ");

        ecs_strbuf_appendstr(buf, arg);

        lua_pop(L, 1);
    }

    return 0;
}

static int print_type(lua_State *L, int type) {
    int level = 1;
    lua_Debug ar = {0};

    if (lua_getstack(L, level, &ar)) lua_getinfo(L, "Sl", &ar);

    int n = lua_gettop(L);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    vararg2str(L, n, &buf);

    char *str = ecs_strbuf_get(&buf);

    switch (type) {
        case ECS_LUA__LOG:
            ecs_os_trace(ar.short_src, ar.currentline, str);
            break;
        case ECS_LUA__ERROR:
            ecs_os_err(ar.short_src, ar.currentline, str);
            break;
        case ECS_LUA__DEBUG:
            ecs_os_dbg(ar.short_src, ar.currentline, str);
            break;
        case ECS_LUA__WARN:
            ecs_os_warn(ar.short_src, ar.currentline, str);
            break;
        default:
            break;
    }

    ecs_os_free(str);

    return 0;
}

int print_log(lua_State *L) { return print_type(L, ECS_LUA__LOG); }

int print_err(lua_State *L) { return print_type(L, ECS_LUA__ERROR); }

int print_dbg(lua_State *L) { return print_type(L, ECS_LUA__DEBUG); }

int print_warn(lua_State *L) { return print_type(L, ECS_LUA__WARN); }

int log_set_level(lua_State *L) {
    int level = luaL_checkinteger(L, 1);

    ecs_log_set_level(level);

    return 0;
}

int log_enable_colors(lua_State *L) {
    int enable = lua_toboolean(L, 1);

    ecs_log_enable_colors(enable);

    return 0;
}

static void print_time(ecs_time_t *time, const char *str) {
#ifndef NDEBUG
    double sec = ecs_time_measure(time);
    ecs_lua_dbg("Lua %s took %f milliseconds", str, sec * 1000.0);
#endif
}

static ecs_world_t **world_buf(lua_State *L, const ecs_world_t *world) {
    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_APIWORLD);
    ecs_assert(ret == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t **wbuf = (ecs_world_t **)lua_touserdata(L, -1);
    ecs_world_t *prev_world = *wbuf;

    lua_pop(L, 2);

    return wbuf;
}

/* Used for systems, triggers and observers */
static void ecs_lua__callback(ecs_iter_t *it) {
    ecs_assert(it->binding_ctx != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t *w = it->world;
    ecs_lua_callback *cb = (ecs_lua_callback *)it->binding_ctx;
    const ecs_world_t *real_world = ecs_get_world(it->world);

    int stage_id = ecs_get_stage_id(w);
    int stage_count = ecs_get_stage_count(w);
    const char *name = ecs_get_name(it->world, it->system);

    ecs_assert(stage_id == 0, ECS_INTERNAL_ERROR, "Lua callbacks must run on the main thread");

    const EcsLuaHost *host = ecs_singleton_get(w, EcsLuaHost);
    ecs_assert(host != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_lua_dbg("Lua %s: \"%s\", %d terms, count %d, func ref %d", cb->type_name, name, it->field_count, it->count, cb->func_ref);

    lua_State *L = host->L;  // host->states[stage_id];

    ecs_lua_ctx *ctx = ecs_lua_get_context(L, real_world);

    ecs_lua__prolog(L);

    /* Since >2.3.2 it->world != the actual world, we have to
       swap the world pointer for all API calls with it->world (stage pointer)
    */
    ecs_world_t **wbuf = world_buf(L, real_world);

    ecs_world_t *prev_world = *wbuf;
    *wbuf = it->world;

    ecs_time_t time;

    int type = ecs_lua_rawgeti(L, w, cb->func_ref);

    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_iter_to_lua(it, L, false);

    print_time(&time, "iter serialization");

    lua_pushvalue(L, -1);
    int it_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_os_get_time(&time);

    int ret = lua_pcall(L, 1, 0, 0);

    *wbuf = prev_world;

    print_time(&time, "system");

    if (ret) {
        const char *err = lua_tostring(L, lua_gettop(L));
        /* TODO: switch to ecs_os_err() with message handler */
        ecs_lua_dbg("error in %s callback \"%s\" (%d): %s", cb->type_name, name, ret, err);
    }

    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

    lua_rawgeti(L, LUA_REGISTRYINDEX, it_ref);

    ecs_assert(lua_type(L, -1) == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_lua_to_iter(L, -1);

    print_time(&time, "iter deserialization");

    luaL_unref(L, LUA_REGISTRYINDEX, it_ref);
    lua_pop(L, 1);

    ecs_lua__epilog(L);
}

static int check_events(lua_State *L, ecs_world_t *w, ecs_entity_t *events, int arg) {
    ecs_entity_t event = 0;

    int type = lua_type(L, arg);

    if (type == LUA_TTABLE) {
        int len = luaL_len(L, arg);

        if (len > FLECS_EVENT_DESC_MAX) return luaL_argerror(L, arg, "too many events");

        int i;
        for (i = 1; i <= len; i++) {
            type = lua_rawgeti(L, arg, i);

            if (type != LUA_TNUMBER) return luaL_argerror(L, arg, "invalid event");

            event = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            if (!event || !ecs_is_valid(w, event)) return luaL_argerror(L, arg, "invalid event");

            events[i - 1] = event;
        }
    } else if (type == LUA_TNUMBER) {
        event = luaL_checkinteger(L, arg);

        if (!event || !ecs_is_valid(w, event)) return luaL_argerror(L, arg, "invalid event");

        events[0] = event;
    } else
        return luaL_argerror(L, arg, "invalid event type");

    return 1;
}

static int new_callback(lua_State *L, ecs_world_t *w, enum EcsLuaCallbackType type) {
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    ecs_entity_t e = 0;
    luaL_checktype(L, 1, LUA_TFUNCTION);
    const char *name = luaL_optstring(L, 2, NULL);
    /* phase, event or event[] expected for arg 3 */
    const char *signature = lua_type(L, 4) == LUA_TSTRING ? luaL_checkstring(L, 4) : NULL;

    ecs_lua_callback *cb = (ecs_lua_callback *)lua_newuserdata(L, sizeof(ecs_lua_callback));

    ecs_lua_ref(L, w);

    if (type == EcsLuaObserver) {
        ecs_entity_desc_t edesc = {.name = name};
        e = ecs_entity_init(w, &edesc);

        ecs_observer_desc_t desc = {.entity = e, .filter = {.expr = signature}, .callback = ecs_lua__callback, .binding_ctx = cb};

        if (signature == NULL) check_filter_desc(L, w, &desc.filter, 4);

        check_events(L, w, desc.events, 3);

        e = ecs_observer_init(w, &desc);

        cb->type_name = "observer";
    } else /* EcsLuaSystem */
    {
        ecs_entity_t phase = luaL_checkinteger(L, 3);

        ecs_entity_desc_t edesc = {0};
        edesc.name = name;
        edesc.add[0] = phase ? ecs_dependson(phase) : 0;
        edesc.add[1] = phase;
        e = ecs_entity_init(w, &edesc);

        ecs_system_desc_t desc = {0};
        desc.entity = e;
        desc.query.filter.expr = signature;
        desc.callback = ecs_lua__callback;
        desc.binding_ctx = cb;

        if (signature == NULL && !lua_isnoneornil(L, 4)) check_filter_desc(L, w, &desc.query.filter, 4);

        e = ecs_system_init(w, &desc);

        cb->type_name = "system";
    }

    if (!e) return luaL_error(L, "failed to create %s", cb->type_name);

    lua_pushvalue(L, 1);
    cb->func_ref = ecs_lua_ref(L, w);
    cb->param_ref = LUA_NOREF;
    cb->type = type;

    lua_pushinteger(L, e);

    return 1;
}

int new_system(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    return new_callback(L, w, EcsLuaSystem);
}

int new_trigger(lua_State *L) { return luaL_error(L, "use ecs.observer()"); }

int new_observer(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    return new_callback(L, w, EcsLuaObserver);
}

int run_system(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    lua_Number delta_time = luaL_checknumber(L, 2);

    ecs_lua_callback *sys = (ecs_lua_callback *)ecs_system_get_binding_ctx(w, system);

    if (sys == NULL) return luaL_argerror(L, 1, "not a Lua system");

    int tmp = sys->param_ref;

    if (!lua_isnoneornil(L, 3)) sys->param_ref = ecs_lua_ref(L, w);

    ecs_entity_t ret = ecs_run(w, system, delta_time, NULL);

    if (tmp != sys->param_ref) { /* Restore previous value */
        ecs_lua_unref(L, w, sys->param_ref);
        sys->param_ref = tmp;
    }

    lua_pushinteger(L, ret);

    return 1;
}

int set_system_context(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    if (lua_gettop(L) < 2) lua_pushnil(L);

    ecs_lua_callback *sys = (ecs_lua_callback *)ecs_system_get_binding_ctx(w, system);

    ecs_lua_unref(L, w, sys->param_ref);

    sys->param_ref = ecs_lua_ref(L, w);

    return 0;
}

static ecs_time_t checktime(lua_State *L, int arg) {
    lua_getfield(L, arg, "sec");
    lua_Integer sec = luaL_checkinteger(L, -1);
    if (sec > INT32_MAX) luaL_argerror(L, arg, "sec field out of range");

    lua_pop(L, 1);

    lua_getfield(L, arg, "nanosec");
    lua_Integer nanosec = luaL_checkinteger(L, -1);
    if (nanosec > INT32_MAX) luaL_argerror(L, arg, "nanosec out of range");

    ecs_time_t time = {.sec = (uint32_t)sec, .nanosec = (uint32_t)nanosec};

    return time;
}

int time__tostring(lua_State *L) {
    ecs_time_t time = checktime(L, 1);

    double sec = ecs_time_to_double(time);

    char str[32];
    snprintf(str, sizeof(str), "%f", sec);

    lua_pushstring(L, str);

    return 1;
}

static void pushtime(lua_State *L, ecs_time_t *time) {
    lua_createtable(L, 0, 3);

    lua_pushinteger(L, time->sec);
    lua_setfield(L, -2, "sec");

    lua_pushinteger(L, time->nanosec);
    lua_setfield(L, -2, "nanosec");

    luaL_setmetatable(L, "ecs_time_t");
}

int get_time(lua_State *L) {
    ecs_time_t time = {0};

    ecs_os_get_time(&time);

    pushtime(L, &time);

    return 1;
}

int time_measure(lua_State *L) {
    ecs_time_t start = checktime(L, 1);

    double time = ecs_time_measure(&start);

    lua_pushnumber(L, time);

    return 1;
}

int set_timeout(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);
    lua_Number timeout = luaL_checknumber(L, 2);

    ecs_entity_t e = ecs_set_timeout(w, timer, timeout);

    lua_pushinteger(L, e);

    return 1;
}

int get_timeout(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    lua_Number timeout = ecs_get_timeout(w, timer);

    lua_pushnumber(L, timeout);

    return 1;
}

int set_interval(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);
    lua_Number interval = luaL_checknumber(L, 2);

    ecs_entity_t e = ecs_set_interval(w, timer, interval);

    lua_pushinteger(L, e);

    return 1;
}

int get_interval(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    lua_Number interval = ecs_get_interval(w, timer);

    lua_pushnumber(L, interval);

    return 1;
}

int start_timer(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    ecs_start_timer(w, timer);

    return 0;
}

int stop_timer(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    ecs_stop_timer(w, timer);

    return 0;
}

int set_rate_filter(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t filter = luaL_checkinteger(L, 1);
    lua_Integer rate = luaL_checkinteger(L, 2);
    ecs_entity_t src = luaL_checkinteger(L, 3);

    ecs_entity_t e = ecs_set_rate(w, filter, rate, src);

    lua_pushinteger(L, e);

    return 1;
}

int set_tick_source(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    ecs_entity_t source = luaL_checkinteger(L, 2);

    ecs_set_tick_source(w, system, source);

    return 0;
}

int world_new(lua_State *L) {
    ecs_world_t *wdefault = ecs_lua_get_world(L);
    ecs_assert(wdefault != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t *w2 = ecs_init();

    ECS_IMPORT(w2, FlecsLua);

    ecs_lua_set_state(w2, L);

    lua_pushcfunction(L, luaopen_ecs);
    ecs_world_t **ptr = (ecs_world_t **)lua_newuserdata(L, sizeof(ecs_world_t *));
    *ptr = w2;

    luaL_setmetatable(L, "ecs_world_t");

    /* The __gc of the default world will collect all other worlds */
    register_collectible(L, wdefault, -1);

    lua_call(L, 1, 1);

    return 1;
}

int world_gc(lua_State *L) {
    ecs_world_t *wdefault = ecs_lua_get_world(L);
    ecs_world_t **ptr = (ecs_world_t **)lua_touserdata(L, 1);
    ecs_world_t *w = *ptr;

    if (!w) return 0;

    lua_rawgetp(L, LUA_REGISTRYINDEX, w);
    lua_rawgeti(L, -1, ECS_LUA_COLLECT);

    int idx = lua_absindex(L, -1);

    lua_pushnil(L);

    while (lua_next(L, idx)) {
        int ret = luaL_callmeta(L, -2, "__gc");
        ecs_assert(ret != 0, ECS_INTERNAL_ERROR, NULL);

        lua_pop(L, 2); /* callmeta pushes a value */
    }

    lua_pop(L, 2);

    if (w != wdefault) {
        /* registry[world] = nil */
        lua_pushnil(L);
        lua_rawsetp(L, LUA_REGISTRYINDEX, w);

        ecs_fini(w);
    }

    *ptr = NULL;

    return 0;
}

int world_fini(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    ecs_world_t *default_world = ecs_lua_get_world(L);

    if (w == default_world) return luaL_argerror(L, 0, "cannot destroy default world");

    luaL_callmeta(L, lua_upvalueindex(1), "__gc");

    return 0;
}

int world_info(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    const ecs_world_info_t *wi = ecs_get_world_info(w);

    ecs_assert(ecs_id(EcsLuaWorldInfo) != 0, ECS_INTERNAL_ERROR, NULL);

    EcsLuaWorldInfo world_info = {
            .last_component_id = wi->last_component_id,
            .min_id = wi->min_id,
            .max_id = wi->max_id,
            .delta_time_raw = wi->delta_time_raw,
            .delta_time = wi->delta_time,
            .time_scale = wi->time_scale,
            .target_fps = wi->target_fps,
            .frame_time_total = wi->frame_time_total,
            .system_time_total = wi->system_time_total,
            .merge_time_total = wi->merge_time_total,
            .world_time_total = wi->world_time_total,
            .world_time_total_raw = wi->world_time_total_raw,
            .frame_count_total = wi->frame_count_total,
            .merge_count_total = wi->merge_count_total,
            .pipeline_build_count_total = wi->pipeline_build_count_total,
            .systems_ran_frame = wi->systems_ran_frame,
    };

    ecs_ptr_to_lua(w, L, ecs_id(EcsLuaWorldInfo), &world_info);

    return 1;
}

int world_stats(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_assert(ecs_id(EcsLuaWorldStats) != 0, ECS_INTERNAL_ERROR, NULL);

    // sizeof(EcsLuaWorldStats)
    // sizeof(ecs_world_stats_t)

    EcsLuaWorldStats ws;
    ecs_world_stats_get(w, (ecs_world_stats_t *)&ws);

    ecs_ptr_to_lua(w, L, ecs_id(EcsLuaWorldStats), &ws);

    return 1;
}

int dim(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    lua_Integer count = luaL_checkinteger(L, 1);

    ecs_dim(w, count);

    return 0;
}

static ecs_snapshot_t *checksnapshot(lua_State *L, int arg) {
    ecs_snapshot_t **snapshot = (ecs_snapshot_t **)luaL_checkudata(L, arg, "ecs_snapshot_t");

    if (!*snapshot) luaL_argerror(L, arg, "snapshot was collected");

    return *snapshot;
}

int snapshot_gc(lua_State *L) {
    ecs_snapshot_t **ptr = (ecs_snapshot_t **)luaL_checkudata(L, 1, "ecs_snapshot_t");
    ecs_snapshot_t *snapshot = *ptr;

    if (snapshot) ecs_snapshot_free(snapshot);

    *ptr = NULL;

    return 0;
}

int snapshot_take(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_iter_t *it;
    ecs_snapshot_t *snapshot;

    if (lua_gettop(L) > 0) {
        it = ecs_lua__checkiter(L, 1);
        snapshot = ecs_snapshot_take_w_iter(it);
    } else
        snapshot = ecs_snapshot_take(w);

    ecs_snapshot_t **ptr = (ecs_snapshot_t **)lua_newuserdata(L, sizeof(ecs_snapshot_t *));
    *ptr = snapshot;

    /* Associate world with the object for sanity checks */
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setuservalue(L, -2);

    luaL_setmetatable(L, "ecs_snapshot_t");
    register_collectible(L, w, -1);

    return 1;
}

int snapshot_restore(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    ecs_snapshot_t *snapshot = checksnapshot(L, 1);

    ecs_lua_check_world(L, w, 1);

    ecs_snapshot_restore(w, snapshot);

    /* Snapshot data is moved into world and is considered freed */
    ecs_snapshot_t **ptr = (ecs_snapshot_t **)lua_touserdata(L, 1);
    *ptr = NULL;

    return 0;
}

int snapshot_iter(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    ecs_snapshot_t *snapshot = checksnapshot(L, 1);

    ecs_iter_t it = ecs_snapshot_iter(snapshot);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int snapshot_next(lua_State *L) {
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_snapshot_next(it);

    if (b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

ecs_query_t *checkquery(lua_State *L, int arg) {
    ecs_query_t **query = (ecs_query_t **)luaL_checkudata(L, arg, "ecs_query_t");

    if (!*query) luaL_argerror(L, arg, "query was collected");

    if (ecs_query_orphaned(*query)) luaL_argerror(L, arg, "parent query was collected");

    return *query;
}

int query_gc(lua_State *L) {
    ecs_query_t **ptr = (ecs_query_t **)luaL_checkudata(L, 1, "ecs_query_t");
    ecs_query_t *query = *ptr;

    if (query) ecs_query_fini(query);

    *ptr = NULL;

    return 0;
}

int query_new(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *sig = luaL_optstring(L, 1, NULL);

    ecs_query_desc_t desc = {.filter = {.expr = sig}};

    if (sig == NULL) check_filter_desc(L, w, &desc.filter, 1);

    ecs_query_t *query = ecs_query_init(w, &desc);

    if (!query) {
        lua_pushnil(L);
        return 1;
    }

    ecs_query_t **ptr = (ecs_query_t **)lua_newuserdata(L, sizeof(ecs_query_t *));
    *ptr = query;

    /* Associate world with the object for sanity checks */
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setuservalue(L, -2);

    luaL_setmetatable(L, "ecs_query_t");
    register_collectible(L, w, -1);

    return 1;
}

int subquery_new(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_query_t *parent = checkquery(L, 1);
    const char *sig = luaL_optstring(L, 2, NULL);

    ecs_query_desc_t desc = {.filter = {.expr = sig}, .parent = parent};

    if (sig == NULL) check_filter_desc(L, w, &desc.filter, 2);

    ecs_query_t *query = ecs_query_init(w, &desc);

    ecs_query_t **ptr = (ecs_query_t **)lua_newuserdata(L, sizeof(ecs_query_t *));
    *ptr = query;

    return 1;
}

int query_iter(lua_State *L) {
    ecs_lua_dbg("QUERY_iter");
    ecs_world_t *w = ecs_lua_world(L);
    ecs_query_t *query = checkquery(L, 1);

    ecs_lua_check_world(L, w, 1);

    ecs_iter_t it = ecs_query_iter(w, query);

    /* will push with no columns because it->count = 0 */
    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int query_next(lua_State *L) {
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);
    int b = ecs_query_next(it);

    if (b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

int query_changed(lua_State *L) {
    ecs_query_t *query = checkquery(L, 1);

    int b = ecs_query_changed(query, NULL);

    lua_pushboolean(L, b);

    return 1;
}

int new_pipeline(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 2);
    const char *expr = luaL_checkstring(L, 2);

    ecs_pipeline_desc_t desc = {0};
    ecs_entity_desc_t edesc = {0};

    edesc.name = name;

    desc.entity = ecs_entity_init(w, &edesc);
    desc.query.filter.expr = expr;

    ecs_entity_t pipeline_entity = ecs_pipeline_init(w, &desc);

    lua_pushinteger(L, pipeline_entity);

    return 1;
}

int set_pipeline(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t pipeline = luaL_checkinteger(L, 1);

    ecs_set_pipeline(w, pipeline);

    return 0;
}

int get_pipeline(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t pipeline = ecs_get_pipeline(w);

    lua_pushinteger(L, pipeline);

    return 1;
}

int measure_frame_time(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int b = lua_toboolean(L, 1);

    ecs_measure_frame_time(w, b);

    return 0;
}

int measure_system_time(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int b = lua_toboolean(L, 1);

    ecs_measure_system_time(w, b);

    return 0;
}

int set_target_fps(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    lua_Number fps = luaL_checknumber(L, 1);

    ecs_set_target_fps(w, fps);

    return 0;
}

int progress(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    lua_Number delta_time = luaL_checknumber(L, 1);

    int b = ecs_progress(w, delta_time);

    lua_pushboolean(L, b);

    return 1;
}

int progress_cb(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    luaL_checktype(L, 1, LUA_TFUNCTION);
    ctx->progress_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

int set_time_scale(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    lua_Number scale = luaL_checknumber(L, 1);

    ecs_set_time_scale(w, scale);

    return 0;
}

int reset_clock(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_reset_clock(w);

    return 0;
}

int lquit(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_quit(w);

    return 0;
}

int set_threads(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    lua_Integer threads = luaL_checkinteger(L, 1);

    ecs_set_threads(w, threads);

    return 0;
}

int get_threads(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int32_t threads = ecs_get_stage_count(w);

    lua_pushinteger(L, threads);

    return 1;
}

int get_thread_index(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    int32_t index = ecs_get_stage_id(w);

    lua_pushinteger(L, index);

    return 1;
}

typedef struct ecs_lua_module {
    ecs_lua_ctx *ctx;
    const char *name;
    int imported;
} ecs_lua_module;

static void import_entry_point(ecs_world_t *w) {
    ecs_lua_module *m = (ecs_lua_module *)ecs_get_ctx(w);
    ecs_lua_ctx *ctx = m->ctx;
    lua_State *L = ctx->L;

    m->imported = 1;

    ecs_component_desc_t desc = {0};
    ecs_entity_t e = ecs_module_init(w, m->name, &desc);

    ecs_set_scope(w, e);

    ctx->error = lua_pcall(L, 0, 0, 0);
}

static void export_handles(lua_State *L, int idx, ecs_world_t *w, ecs_entity_t e) {
    idx = lua_absindex(L, idx);

    luaL_checktype(L, idx, LUA_TTABLE);

    ecs_term_t it_term{.id = ecs_pair(EcsChildOf, e)};
    ecs_iter_t it = ecs_term_iter(w, &it_term);

    while (ecs_term_next(&it)) {
        int i, type;
        const char *name;

        for (i = 0; i < it.count; i++) {
            e = it.entities[i];
            name = ecs_get_name(it.world, e);
            if (!name) continue;

            lua_pushinteger(L, e);

            type = lua_getfield(L, idx, name);

            if (type != LUA_TNIL && type != LUA_TFUNCTION) {
                if (!lua_rawequal(L, -1, -2)) luaL_error(L, "export table conflict (%s)", name);
            }

            lua_pop(L, 1);

            lua_setfield(L, idx, name);
        }
    }
}

int new_module(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    const char *name = luaL_checkstring(L, 1);

    int func_idx = 2;

    if (lua_type(L, 2) == LUA_TTABLE) func_idx = 3;

    luaL_checktype(L, func_idx, LUA_TFUNCTION);

    char *module_name = ecs_module_path_from_c(name);

    ctx->error = 0;
    ecs_lua_module m = {.ctx = ctx, .name = module_name};

    void *orig = ecs_get_ctx(w);

    ecs_set_ctx(w, &m, NULL);
    ecs_entity_t e = ecs_import(w, import_entry_point, module_name);
    ecs_set_ctx(w, orig, NULL);

    ecs_os_free(module_name);

    if (ctx->error) return lua_error(L);

    if (func_idx == 3) export_handles(L, 2, w, e);

    lua_pushinteger(L, e);

    return 1;
}

int import_handles(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    const char *c_name = luaL_checkstring(L, 1);

    const char *name = ecs_module_path_from_c(c_name);
    ecs_entity_t e = ecs_lookup_fullpath(w, name);
    ecs_os_free((char *)name);

    if (!e || !ecs_has_id(w, e, EcsModule)) return luaL_argerror(L, 1, "no such module");

    if (lua_type(L, 2) != LUA_TTABLE) lua_createtable(L, 0, 4);

    export_handles(L, -1, w, e);

    return 1;
}

ecs_type_t checktype(lua_State *L, int arg) {
    ecs_type_t *type = (ecs_type_t *)luaL_checkudata(L, arg, "ecs_type_t");

    return *type;
}

int check_filter_desc(lua_State *L, const ecs_world_t *world, ecs_filter_desc_t *desc, int arg) {
    luaL_checktype(L, arg, LUA_TTABLE);

    ecs_term_t *terms = desc->terms;

    int fields = 0;
    int terms_type = lua_getfield(L, arg, "terms");
    int expr_type = lua_getfield(L, arg, "expr");

    if (terms_type != LUA_TNIL) {
        int i, len = 0;

        if (terms_type == LUA_TNUMBER) /* terms: integer */
        {
            terms[0] = checkterm(L, world, -2);
        } else if (terms_type == LUA_TTABLE) /* terms: ecs_term_t|ecs_term_t[] */
        {
            int type = lua_rawgeti(L, -2, 1); /* type(terms[1]) */
            lua_pop(L, 1);

            if (type != LUA_TNIL) /* terms: ecs_term_t[] */
            {
                len = luaL_len(L, -2);

                if (len > FLECS_TERM_DESC_MAX) return luaL_argerror(L, arg, "too many terms");

                for (i = 0; i < len; i++) {
                    lua_rawgeti(L, -2, i + 1);
                    terms[i] = checkterm(L, world, -1);
                    lua_pop(L, 1);
                }
            } else /* terms: ecs_term_t */
            {
                terms[0] = checkterm(L, world, -2);
            }
        } else
            return luaL_argerror(L, arg, "invalid term type");

        fields++;
    }

    if (expr_type != LUA_TNIL) {
        if (expr_type != LUA_TSTRING) return luaL_argerror(L, arg, "expected string (expr)");

        desc->expr = (char *)luaL_checkstring(L, -1);

        fields++;
    }

    lua_pop(L, 2);

    if (!fields) return luaL_argerror(L, arg, "empty filter");

    return 0;
}

ecs_filter_t *checkfilter(lua_State *L, ecs_world_t *world, int arg) {
    ecs_filter_desc_t filter_desc = {0};

    check_filter_desc(L, world, &filter_desc, arg);

    ecs_filter_t *filter = ecs_filter_init(world, &filter_desc);

    if (!filter) luaL_argerror(L, arg, "invalid filter");

    return filter;
}

int assert_func(lua_State *L) {
    if (lua_toboolean(L, 1)) return lua_gettop(L);

#ifdef NDEBUG
    return lua_gettop(L);
#endif

    luaL_checkany(L, 1);
    lua_remove(L, 1);
    lua_pushliteral(L, "assertion failed!");
    lua_settop(L, 1);

    int level = (int)luaL_optinteger(L, 2, 1);
    lua_settop(L, 1);

    if (lua_type(L, 1) == LUA_TSTRING && level > 0) {
        luaL_where(L, level);
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
    }

    return lua_error(L);
}

int sizeof_component(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const EcsComponent *ptr = ecs_get(w, e, EcsComponent);

    if (!ptr) luaL_argerror(L, 1, "not a component");

    lua_pushinteger(L, ptr->size);
    lua_pushinteger(L, ptr->alignment);

    return 2;
}

int l_is_primitive(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    luaL_argcheck(L, e != 0, 1, "expected non-zero entity");

    const EcsPrimitive *p = ecs_get(w, e, EcsPrimitive);

    if (p == NULL) return 0;

    lua_pushinteger(L, p->kind);

    return 1;
}

int createtable(lua_State *L) {
    int narr = luaL_optinteger(L, 1, 0);
    int nrec = luaL_optinteger(L, 2, 0);

    lua_createtable(L, narr, nrec);

    return 1;
}

int ecs_lua__readonly(lua_State *L) { return luaL_error(L, "Attempt to modify read-only table"); }

int zero_init_component(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    ecs_type_hooks_t hooks = {.ctor = ecs_default_ctor};

    ecs_set_hooks_id(w, component, &hooks);

    return 0;
}

int get_world_ptr(lua_State *L) {
    ecs_world_t *w = ecs_lua_world(L);

    lua_pushlightuserdata(L, w);

    return 1;
}

void ecs_lua__assert(lua_State *L, bool condition, const char *param, const char *condition_str) {
    if (!condition) luaL_error(L, "assert(%s) failed", condition_str);
}