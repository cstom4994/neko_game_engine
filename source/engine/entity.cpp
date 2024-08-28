
#include "engine/entity.h"

#include <stdbool.h>
#include <stdlib.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/edit.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/luax.hpp"
#include "engine/scripting.h"
#include "engine/test.h"
#include "engine/ui.h"

#if 1

// 为池 ID 提供 O(1) 操作的 ID 池的数据结构
typedef struct {
    size_t capacity;
    size_t size;
    ecs_id_t* array;
} ecs_stack_t;

struct ecs_s {
    lua_State* L;
    int system_table_ref;
};

static int __neko_ecs_lua_create_ent(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    // ecs_id_t e = ecs_create(w);
    ecs_id_t e = 0;
    lua_pushinteger(L, e);
    return 1;
}

typedef struct ecs_ent_iter_t {
    ecs_stack_t* pool;
    ecs_id_t index;
    bool check_ready;
} ecs_ent_iter_t;

LUA_FUNCTION(__neko_ecs_lua_ent_next) {
    ecs_t* w = (ecs_t*)lua_touserdata(L, lua_upvalueindex(1));
    ecs_ent_iter_t* it = (ecs_ent_iter_t*)lua_touserdata(L, lua_upvalueindex(2));
    if (it->index >= it->pool->capacity) {
        return 0;
    }
    ecs_id_t e = 0;
    for (; it->index < it->pool->capacity;) {
        e = it->pool->array[it->index];
        it->index++;
        // if (w->entities[e].ready == it->check_ready) {
        //     break;
        // }
    }
    lua_pushinteger(L, e);
    return 1;
}

LUA_FUNCTION(__neko_ecs_lua_ent_iterator) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    bool check_ready = lua_toboolean(L, 2);

    // neko_assert(ecs_is_not_null(w));

    ecs_ent_iter_t* it = (ecs_ent_iter_t*)lua_newuserdata(L, sizeof(ecs_ent_iter_t));

    // it->pool = &w->entity_pool;
    it->index = 0;
    it->check_ready = check_ready;

    lua_pushlightuserdata(L, w);
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, __neko_ecs_lua_ent_next, 2);
    return 1;
}

const ecs_id_t __neko_ecs_lua_component_id_w(lua_State* L, const_str component_name) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");  // # -5
    lua_getiuservalue(L, -1, NEKO_ECS_COMPONENTS_NAME);     // # -4
    lua_getfield(L, -1, "comp_map");                        // # -3
    lua_pushinteger(L, neko_hash_str(component_name));      // 使用 32 位哈希以适应 Lua 数字范围
    lua_gettable(L, -2);                                    // # -2
    lua_getfield(L, -1, "id");                              // # -1
    const ecs_id_t c = lua_tointeger(L, -1);
    lua_pop(L, 5);
    return c;
}

static int __neko_ecs_lua_attach(lua_State* L) {
    int n = lua_gettop(L);
    luaL_argcheck(L, n >= 3, 1, "lost the component name");

    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_id_t e = luaL_checkinteger(L, 2);

    Array<void*> ptrs;
    neko_defer(ptrs.trash());

    for (int i = 3; i <= n; i++) {
        if (lua_isstring(L, i)) {
            const_str component_name = lua_tostring(L, i);
            const ecs_id_t c = __neko_ecs_lua_component_id_w(L, component_name);
            // void* ptr = ecs_add(w, e, c, NULL);
            void* ptr = 0;
            ptrs.push(ptr);
        } else {
            console_log("argument %d is not a string", i);
        }
    }
    for (auto ptr : ptrs) {
        lua_pushlightuserdata(L, ptr);
    }
    return n - 2;
}

static int __neko_ecs_lua_get_com(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_UPVAL_N);
    return 2;
}

static int __neko_ecs_lua_gc(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    // ecs_free_i(w);
    console_log("ecs_lua_gc");
    return 0;
}

struct lua_system_userdata {
    String system_name;
};

static ecs_ret_t l_system_update(ecs_t* ecs, ecs_id_t* entities, int entity_count, f64 dt, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    lua_State* L = ecs->L;

    int systb = ecs->system_table_ref;

    neko_assert(systb != LUA_NOREF && L);

    lua_rawgeti(L, LUA_REGISTRYINDEX, systb);
    lua_getfield(L, -1, ud->system_name.cstr());
    if (lua_istable(L, -1)) {

        for (int id = 0; id < entity_count; id++) {
            lua_getfield(L, -1, "func_update");
            neko_assert(lua_isfunction(L, -1));
            lua_pushinteger(L, entities[id]);
            lua_pushlstring(L, ud->system_name.cstr(), ud->system_name.len);
            lua_call(L, 2, 0);  // 调用
        }

    } else {
        console_log("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);

    return 0;
}

static void l_system_add(ecs_t* ecs, ecs_id_t entity_id, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    // console_log("l_system_add %s ent_id %d", ud->system_name.cstr(), entity_id);

    lua_State* L = ecs->L;
    int systb = ecs->system_table_ref;

    neko_assert(systb != LUA_NOREF);

    lua_rawgeti(L, LUA_REGISTRYINDEX, systb);
    lua_getfield(L, -1, ud->system_name.cstr());
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "func_add");
        neko_assert(lua_isfunction(L, -1));
        lua_pushinteger(L, entity_id);
        lua_pushlstring(L, ud->system_name.cstr(), ud->system_name.len);
        lua_call(L, 2, 0);  // 调用
    } else {
        console_log("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);
}

static void l_system_remove(ecs_t* ecs, ecs_id_t entity_id, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    // console_log("l_system_remove %s ent_id %d", ud->system_name.cstr(), entity_id);

    lua_State* L = ecs->L;
    int systb = ecs->system_table_ref;

    neko_assert(systb != LUA_NOREF);

    lua_rawgeti(L, LUA_REGISTRYINDEX, systb);
    lua_getfield(L, -1, ud->system_name.cstr());
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "func_remove");
        neko_assert(lua_isfunction(L, -1));
        lua_pushinteger(L, entity_id);
        lua_pushlstring(L, ud->system_name.cstr(), ud->system_name.len);
        lua_call(L, 2, 0);  // 调用
    } else {
        console_log("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);
}

static int __neko_ecs_lua_system(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    neko_assert(w->L == L);

    ecs_id_t sys = u32_max;
    String system_name = luax_check_string(L, ECS_WORLD + 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, w->system_table_ref);
    if (lua_istable(L, -1)) {

        lua_createtable(L, 0, 2);

        lua_pushvalue(L, ECS_WORLD + 2);
        neko_assert(lua_isfunction(L, -1));
        lua_setfield(L, -2, "func_update");

        lua_pushvalue(L, ECS_WORLD + 3);
        neko_assert(lua_isfunction(L, -1));
        lua_setfield(L, -2, "func_add");

        lua_pushvalue(L, ECS_WORLD + 4);
        neko_assert(lua_isfunction(L, -1));
        lua_setfield(L, -2, "func_remove");

        lua_pushstring(L, system_name.cstr());
        lua_setfield(L, -2, "name");

        lua_setfield(L, -2, system_name.cstr());

        lua_system_userdata* ud = (lua_system_userdata*)mem_alloc(sizeof(lua_system_userdata));  // gc here
        ud->system_name = system_name;
        // sys = ecs_register_system(w, l_system_update, l_system_add, l_system_remove, (void*)ud);
    }

    neko_assert(sys != u32_max);
    lua_pushinteger(L, sys);
    return 1;
}

static int __neko_ecs_lua_system_require_component(lua_State* L) {
    int n = lua_gettop(L);
    luaL_argcheck(L, n >= 3, 1, "lost the component name");

    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    neko_assert(w->L == L);

    ecs_id_t sys = luaL_checkinteger(L, ECS_WORLD + 1);

    for (int i = 3; i <= n; i++) {
        if (lua_isstring(L, i)) {
            const_str comp_name = luaL_checkstring(L, i);
            ecs_id_t comp_id = __neko_ecs_lua_component_id_w(L, comp_name);
            // ecs_require_component(w, sys, comp_id);
        } else {
            console_log("argument %d is not a ecs_component", i);
        }
    }

    // lua_pushinteger(L, ret);
    return 0;
}

static int __neko_ecs_lua_get(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    neko_assert(w->L == L);
    ecs_id_t ent_id = lua_tointeger(L, ECS_WORLD + 1);
    ecs_id_t comp_id = lua_tointeger(L, ECS_WORLD + 2);
    // void* ptr = ecs_get(w, ent_id, comp_id);
    void* ptr = 0;
    lua_pushlightuserdata(L, ptr);
    return 1;
}

static int __neko_ecs_lua_system_run(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    neko_assert(w->L == L);
    ecs_id_t sys = lua_tointeger(L, ECS_WORLD + 1);
    f64 dt = lua_tonumber(L, ECS_WORLD + 2);
    // ecs_ret_t ret = ecs_update_system(w, sys, dt);
    int ret = 0;
    lua_pushinteger(L, ret);
    return 1;
}

ecs_id_t ecs_component_w(ecs_t* w, const_str component_name, size_t component_size) {
    PROFILE_FUNC();

    // ecs_id_t comp_id = ecs_register_component(w, component_size, constructor, destructor);
    ecs_id_t comp_id = 0;

    lua_State* L = w->L;

    // lua_getglobal(L, "__NEKO_ECS_CORE");  // # 1
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");

    lua_getiuservalue(L, -1, NEKO_ECS_COMPONENTS_NAME);
    if (lua_istable(L, -1)) {

        lua_getfield(L, -1, "comp_map");
        if (lua_istable(L, -1)) {
            lua_pushinteger(L, neko_hash_str(component_name));  // 使用 32 位哈希以适应 Lua 数字范围

            lua_createtable(L, 0, 2);

            lua_pushinteger(L, comp_id);
            lua_setfield(L, -2, "id");

            lua_pushstring(L, component_name);
            lua_setfield(L, -2, "name");

            // if (NULL == constructor && NULL == destructor) {
            //     lua_pushvalue(L, ECS_WORLD + 3);
            //     neko_assert(lua_isfunction(L, -1));
            //     lua_setfield(L, -2, "func_ctor");
            // }

            lua_settable(L, -3);
            lua_pop(L, 1);
        } else {
            console_log("%s", "failed to get comp_map");
            lua_pop(L, 1);
        }

        lua_getfield(L, -1, "comps");
        if (lua_istable(L, -1)) {
            // lua_pushinteger(L, neko_hash_str(component_name));  // 使用 32 位哈希以适应 Lua 数字范围

            // lua_createtable(L, 0, 2);

            // lua_pushinteger(L, comp_id);
            // lua_setfield(L, -2, "id");

            // lua_pushstring(L, component_name);
            // lua_setfield(L, -2, "name");

            // // if (NULL == constructor && NULL == destructor) {
            // //     lua_pushvalue(L, ECS_WORLD + 3);
            // //     neko_assert(lua_isfunction(L, -1));
            // //     lua_setfield(L, -2, "func_ctor");
            // // }

            // lua_settable(L, -3);
            lua_pop(L, 1);
        } else {
            console_log("%s", "failed to get comps");
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    } else {
        console_log("%s", "failed to get upvalue NEKO_ECS_COMPONENTS_NAME");
        lua_pop(L, 1);
    }
    lua_pop(L, 1);  // pop 1

    return comp_id;
}

static void l_component_ctor(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args) {
    // lua_system_userdata* ud = (lua_system_userdata*)udata;
    // console_log("l_system_remove %s ent_id %d", ud->system_name.cstr(), entity_id);

    lua_State* L = ecs->L;
    int systb = ecs->system_table_ref;

    neko_assert(systb != LUA_NOREF);
}

static int __neko_ecs_lua_component(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    neko_assert(w->L == L);

    ecs_id_t id = u32_max;
    String comp_name = luax_check_string(L, ECS_WORLD + 1);
    size_t size = luaL_checkinteger(L, ECS_WORLD + 2);

    // id = ecs_component_w(w, comp_name.cstr(), size, NULL, NULL);

    neko_assert(id != u32_max);
    lua_pushinteger(L, id);
    return 1;
}

static int __neko_ecs_lua_component_id(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    neko_assert(w->L == L);

    ecs_id_t id = u32_max;
    String comp_name = luax_check_string(L, ECS_WORLD + 1);

    id = __neko_ecs_lua_component_id_w(L, comp_name.cstr());

    neko_assert(id != u32_max);
    lua_pushinteger(L, id);
    return 1;
}

ecs_t* ecs_init(lua_State* L) {
    PROFILE_FUNC();

    neko_assert(L);
    ecs_t* w = (ecs_t*)lua_newuserdatauv(L, sizeof(ecs_t), NEKO_ECS_UPVAL_N);  // # -1
    // w = ecs_new_i(L, w, 1024, NULL);
    // if (w == NULL || w->entities == NULL) {
    //     console_log("failed to initialize ecs_t");
    //     return NULL;
    // }

    {
        lua_newtable(L);
        w->system_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    if (luaL_getmetatable(L, ECS_WORLD_UDATA_NAME) == LUA_TNIL) {  // # -2

        // clang-format off
        luaL_Reg ecs_mt[] = {
            {"__gc", __neko_ecs_lua_gc}, 
            {"create_ent", __neko_ecs_lua_create_ent}, 
            {"attach", __neko_ecs_lua_attach}, 
            {"get_com", __neko_ecs_lua_get_com}, 
            {"iter", __neko_ecs_lua_ent_iterator},
            {"system", __neko_ecs_lua_system},
            {"system_run", __neko_ecs_lua_system_run},
            {"system_require_component", __neko_ecs_lua_system_require_component},
            {"component", __neko_ecs_lua_component},
            {"get",__neko_ecs_lua_get},
            {"component_id",__neko_ecs_lua_component_id},
            {NULL, NULL}
        };
        // clang-format on

        lua_pop(L, 1);                // # pop -2
        luaL_newlibtable(L, ecs_mt);  // # -2
        luaL_setfuncs(L, ecs_mt, 0);
        lua_pushvalue(L, -1);                                      // # -3
        lua_setfield(L, -2, "__index");                            // pop -3
        lua_pushliteral(L, ECS_WORLD_UDATA_NAME);                  // # -3
        lua_setfield(L, -2, "__name");                             // pop -3
        lua_pushvalue(L, -1);                                      // # -3
        lua_setfield(L, LUA_REGISTRYINDEX, ECS_WORLD_UDATA_NAME);  // pop -3
    }
    lua_setmetatable(L, -2);  // pop -2

    lua_newtable(L);

    lua_pushstring(L, "comp_map");
    lua_createtable(L, 0, ENTITY_MAX_COMPONENTS);
    lua_settable(L, -3);

    lua_pushstring(L, "comps");
    lua_createtable(L, 0, ENTITY_MAX_COMPONENTS);
    lua_settable(L, -3);

    lua_setiuservalue(L, -2, NEKO_ECS_COMPONENTS_NAME);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, -2, NEKO_ECS_UPVAL_N);

    // lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");

    // lua_setglobal(L, "__NEKO_ECS_CORE");

    return w;
}

#endif

typedef struct DestroyEntry DestroyEntry;
struct DestroyEntry {
    NativeEntity ent;
    ecs_id_t pass;
};

NativeEntity entity_nil = {0};
static ecs_id_t counter = 1;

typedef struct ExistsPoolElem {
    EntityPoolElem pool_elem;
} ExistsPoolElem;

static NativeEntityPool* exists_pool;   // 所有现有实体
static NativeEntityMap* destroyed_map;  // 实体是否被销毁
static CArray* destroyed;         // 被销毁对象的 DestroyEntry 数组
static NativeEntityMap* unused_map;     // 未使用的数组中是否有条目
static CArray* unused;            // id 放在这里 _remove() 之后 可以复用
static NativeEntityMap* load_map;       // 保存的 ID 映射 --> 真实 ID

typedef enum SaveFilter {
    SF_SAVE,     // 保存此实体
    SF_NO_SAVE,  // 不要保存此实体
    SF_UNSET,    // 未设置过滤器 -- 使用默认值
} SaveFilter;

static NativeEntityMap* save_filter_map;
static SaveFilter save_filter_default = SF_SAVE;

static NativeEntity _generate_id() {
    NativeEntity ent;

    if (array_length(unused) > 0) {

        ent = array_top_val(NativeEntity, unused);
        entitymap_set(unused_map, ent, false);
        array_pop(unused);
    } else
        ent.id = counter++;
    error_assert(!native_entity_eq(ent, entity_nil));

    entitypool_add(exists_pool, ent);
    return ent;
}

NativeEntity entity_create() {
    NativeEntity ent;

    ent = _generate_id();

    return ent;
}

static void entity_remove(NativeEntity ent) {

    entitymap_set(destroyed_map, ent, false);
    array_add_val(NativeEntity, unused) = ent;
    entitymap_set(unused_map, ent, true);
}

void entity_destroy(NativeEntity ent) {
    if (!entitypool_get(exists_pool, ent)) return;
    if (entitymap_get(unused_map, ent)) return;
    if (entitymap_get(destroyed_map, ent)) return;

    entitypool_remove(exists_pool, ent);

    entitymap_set(destroyed_map, ent, true);
    array_add_val(DestroyEntry, destroyed) = DestroyEntry{{ent.id}, 0};
}

void entity_destroy_all() {
    ExistsPoolElem* exists;
    entitypool_foreach(exists, exists_pool) entity_destroy(exists->pool_elem.ent);
}

bool entity_destroyed(NativeEntity ent) { return entitymap_get(destroyed_map, ent); }

void entity_set_save_filter(NativeEntity ent, bool filter) {
    if (filter) {
        entitymap_set(save_filter_map, ent, SF_SAVE);
        save_filter_default = SF_NO_SAVE;
    } else
        entitymap_set(save_filter_map, ent, SF_NO_SAVE);
}
bool entity_get_save_filter(NativeEntity ent) {
    SaveFilter filter = (SaveFilter)entitymap_get(save_filter_map, ent);
    if (filter == SF_UNSET) filter = save_filter_default;
    return filter == SF_SAVE;
}
void entity_clear_save_filters() {
    entitymap_clear(save_filter_map);
    save_filter_default = SF_SAVE;
}

void entity_init() {
    PROFILE_FUNC();

    exists_pool = entitypool_new(ExistsPoolElem);
    destroyed_map = entitymap_new(false);
    destroyed = array_new(DestroyEntry);
    unused_map = entitymap_new(false);
    unused = array_new(NativeEntity);
    save_filter_map = entitymap_new(SF_UNSET);
}

void entity_fini() {
    entitymap_free(save_filter_map);
    array_free(unused);
    entitymap_free(unused_map);
    array_free(destroyed);
    entitymap_free(destroyed_map);
    entitypool_free(exists_pool);
}

void entity_update_all() {
    ecs_id_t i;
    DestroyEntry* entry;

    for (i = 0; i < array_length(destroyed);) {
        entry = (DestroyEntry*)array_get(destroyed, i);
        if (entry->pass == 0) {
            ++entry->pass;
            ++i;
        } else {
            entity_remove(entry->ent);
            array_quick_remove(destroyed, i);
        }
    }
}

void entity_save(NativeEntity* ent, const char* n, Store* s) {
    Store* t;

    if (!native_entity_eq(*ent, entity_nil) && !entity_get_save_filter(*ent)) error("filtered-out entity referenced in save!");

    if (store_child_save(&t, n, s)) uint_save(&ent->id, "id", t);
}
NativeEntity _entity_resolve_saved_id(ecs_id_t id) {
    NativeEntity ent, sav = {id};

    if (native_entity_eq(sav, entity_nil)) return entity_nil;

    ent.id = entitymap_get(load_map, sav);
    if (native_entity_eq(ent, entity_nil)) {
        ent = _generate_id();
        entitymap_set(load_map, sav, ent.id);
    }
    return ent;
}
bool entity_load(NativeEntity* ent, const char* n, NativeEntity d, Store* s) {
    Store* t;
    ecs_id_t id;

    if (!store_child_load(&t, n, s)) {
        *ent = d;
        return false;
    }

    uint_load(&id, "id", entity_nil.id, t);
    *ent = _entity_resolve_saved_id(id);
    return true;
}

void entity_load_all_begin() { load_map = entitymap_new(entity_nil.id); }
void entity_load_all_end() {
    entitymap_free(load_map);
    entity_clear_save_filters();
}

#undef native_entity_eq
bool native_entity_eq(NativeEntity e, NativeEntity f) { return e.id == f.id; }

void entity_save_all(Store* s) {
    DestroyEntry* entry;
    ExistsPoolElem* exists;
    Store *entity_s, *exists_s, *destroyed_s, *entry_s;

    if (store_child_save(&entity_s, "entity", s)) {
        entitypool_save_foreach(exists, exists_s, exists_pool, "exists_pool", entity_s);

        if (store_child_save(&destroyed_s, "destroyed", entity_s)) array_foreach(entry, destroyed) if (entity_get_save_filter(entry->ent)) if (store_child_save(&entry_s, NULL, destroyed_s)) {
                entity_save(&entry->ent, "ent", entry_s);
                uint_save(&entry->pass, "pass", entry_s);
            }
    }
}

void entity_load_all(Store* s) {
    DestroyEntry* entry;
    ExistsPoolElem* exists;
    Store *entity_s, *exists_s, *destroyed_s, *entry_s;

    if (store_child_load(&entity_s, "entity", s)) {
        entitypool_load_foreach(exists, exists_s, exists_pool, "exists_pool", entity_s);

        if (store_child_load(&destroyed_s, "destroyed", entity_s))
            while (store_child_load(&entry_s, NULL, destroyed_s)) {
                entry = (DestroyEntry*)array_add(destroyed);
                error_assert(entity_load(&entry->ent, "ent", entity_nil, entry_s));
                uint_load(&entry->pass, "pass", 0, entry_s);
            }
    }
}

struct NativeEntityPool {
    // just a map of indices into an array, -1 if doesn't exist
    NativeEntityMap* emap;
    CArray* array;
};

NativeEntityPool* entitypool_new_(size_t object_size) {
    NativeEntityPool* pool = (NativeEntityPool*)mem_alloc(sizeof(NativeEntityPool));

    pool->emap = entitymap_new(-1);
    pool->array = array_new_(object_size);

    return pool;
}
void entitypool_free(NativeEntityPool* pool) {
    array_free(pool->array);
    entitymap_free(pool->emap);
    mem_free(pool);
}

void* entitypool_add(NativeEntityPool* pool, NativeEntity ent) {
    EntityPoolElem* elem;

    if ((elem = (EntityPoolElem*)entitypool_get(pool, ent))) return elem;

    // add element to array and set id in map
    elem = (EntityPoolElem*)array_add(pool->array);
    elem->ent = ent;
    entitymap_set(pool->emap, ent, array_length(pool->array) - 1);
    return elem;
}
void entitypool_remove(NativeEntityPool* pool, NativeEntity ent) {
    int i;
    EntityPoolElem* elem;

    i = entitymap_get(pool->emap, ent);
    if (i >= 0) {
        // remove may swap with last element, so fix that mapping
        if (array_quick_remove(pool->array, i)) {
            elem = (EntityPoolElem*)array_get(pool->array, i);
            entitymap_set(pool->emap, elem->ent, i);
        }

        // remove mapping
        entitymap_set(pool->emap, ent, -1);
    }
}
void* entitypool_get(NativeEntityPool* pool, NativeEntity ent) {
    int i;

    i = entitymap_get(pool->emap, ent);
    if (i >= 0) return array_get(pool->array, i);
    return NULL;
}

void* entitypool_begin(NativeEntityPool* pool) { return array_begin(pool->array); }
void* entitypool_end(NativeEntityPool* pool) { return array_end(pool->array); }
void* entitypool_nth(NativeEntityPool* pool, ecs_id_t n) { return array_get(pool->array, n); }

ecs_id_t entitypool_size(NativeEntityPool* pool) { return array_length(pool->array); }

void entitypool_clear(NativeEntityPool* pool) {
    entitymap_clear(pool->emap);
    array_clear(pool->array);
}

void entitypool_sort(NativeEntityPool* pool, int (*compar)(const void*, const void*)) {
    ecs_id_t i, n;
    EntityPoolElem* elem;

    array_sort(pool->array, compar);

    // remap NativeEntity -> index
    n = array_length(pool->array);
    for (i = 0; i < n; ++i) {
        elem = (EntityPoolElem*)array_get(pool->array, i);
        entitymap_set(pool->emap, elem->ent, i);
    }
}

void entitypool_elem_save(NativeEntityPool* pool, void* elem, Store* s) {
    EntityPoolElem** p;

    // save NativeEntity id
    p = (EntityPoolElem**)elem;
    entity_save(&(*p)->ent, "pool_elem", s);
}
void entitypool_elem_load(NativeEntityPool* pool, void* elem, Store* s) {
    NativeEntity ent;
    EntityPoolElem** p;

    // load NativeEntity id, add element with that key
    error_assert(entity_load(&ent, "pool_elem", entity_nil, s), "saved EntityPoolElem entry must exist");
    p = (EntityPoolElem**)elem;
    *p = (EntityPoolElem*)entitypool_add(pool, ent);
    (*p)->ent = ent;
}

#define MIN_CAPACITY 2

static void entitymap_init_w(NativeEntityMap* emap) {
    ecs_id_t i;

    emap->bound = 0;                // bound <= capacity (so that maximum key < capacity)
    emap->capacity = MIN_CAPACITY;  // MIN_CAPACITY <= capacity
    emap->arr = (int*)mem_alloc(emap->capacity * sizeof(*emap->arr));

    for (i = 0; i < emap->capacity; ++i) emap->arr[i] = emap->def;
}

NativeEntityMap* entitymap_new(int def) {
    NativeEntityMap* emap;

    emap = (NativeEntityMap*)mem_alloc(sizeof(NativeEntityMap));
    emap->def = def;

    entitymap_init_w(emap);

    return emap;
}
void entitymap_clear(NativeEntityMap* emap) {
    mem_free(emap->arr);
    entitymap_init_w(emap);
}
void entitymap_free(NativeEntityMap* emap) {
    mem_free(emap->arr);
    mem_free(emap);
}

static void _grow(NativeEntityMap* emap) {
    ecs_id_t new_capacity, i, bound;

    // find next power of 2 (TODO: use log?)
    bound = emap->bound;
    for (new_capacity = emap->capacity; new_capacity < bound; new_capacity <<= 1);

    // grow, clear new
    emap->arr = (int*)mem_realloc(emap->arr, new_capacity * sizeof(*emap->arr));
    for (i = emap->capacity; i < new_capacity; ++i) emap->arr[i] = emap->def;
    emap->capacity = new_capacity;
}
static void _shrink(NativeEntityMap* emap) {
    ecs_id_t new_capacity, bound_times_4;

    if (emap->capacity <= MIN_CAPACITY) return;

    // halve capacity while bound is less than a fourth
    bound_times_4 = emap->bound << 2;
    if (bound_times_4 >= emap->capacity) return;
    for (new_capacity = emap->capacity; new_capacity > MIN_CAPACITY && bound_times_4 < new_capacity; new_capacity >>= 1);
    if (new_capacity < MIN_CAPACITY) new_capacity = MIN_CAPACITY;

    emap->arr = (int*)mem_realloc(emap->arr, new_capacity * sizeof(*emap->arr));
    emap->capacity = new_capacity;
}

void entitymap_set(NativeEntityMap* emap, NativeEntity ent, int val) {
    if (val == emap->def)  // deleting?
    {
        emap->arr[ent.id] = val;

        // possibly move bound down and shrink
        if (emap->bound == ent.id + 1) {
            while (emap->bound > 0 && emap->arr[emap->bound - 1] == emap->def) --emap->bound;
            _shrink(emap);
        }
    } else {
        // possibly move bound up and grow
        if (ent.id + 1 > emap->bound) {
            emap->bound = ent.id + 1;
            if (ent.id >= emap->capacity) _grow(emap);
        }

        emap->arr[ent.id] = val;
    }
}
int entitymap_get(NativeEntityMap* emap, NativeEntity ent) {
    if (ent.id >= emap->capacity) return emap->def;
    return emap->arr[ent.id];
}

static void load_all_lua_scripts(lua_State* L) {
    PROFILE_FUNC();

    Array<String> files = {};
    neko_defer({
        for (String str : files) {
            mem_free(str.data);
        }
        files.trash();
    });

    bool ok = false;

#if defined(_DEBUG)
    ok = vfs_list_all_files(NEKO_PACKS::LUACODE, &files);
#else
    ok = vfs_list_all_files(NEKO_PACKS::GAMEDATA, &files);
#endif

    if (!ok) {
        neko_panic("failed to list all files");
    }
    std::qsort(files.data, files.len, sizeof(String), [](const void* a, const void* b) -> int {
        String* lhs = (String*)a;
        String* rhs = (String*)b;
        return std::strcmp(lhs->data, rhs->data);
    });

    for (String file : files) {
        if (file.starts_with("script/") && !file.ends_with("nekomain.lua") && file.ends_with(".lua")) {
            asset_load_kind(AssetKind_LuaRef, file, nullptr);
        } else {
        }
    }
}

static void _key_down(KeyCode key, int scancode, int mode) {
    gui_key_down(key);
    script_key_down(key);
}
static void _key_up(KeyCode key, int scancode, int mode) {
    gui_key_up(key);
    script_key_up(key);
}
static void _char_down(unsigned int c) { gui_char_down(c); }
static void _mouse_down(MouseCode mouse) {
    gui_mouse_down(mouse);
    script_mouse_down(mouse);
}
static void _mouse_up(MouseCode mouse) {
    gui_mouse_up(mouse);
    script_mouse_up(mouse);
}
static void _mouse_move(vec2 pos) { script_mouse_move(pos); }
static void _scroll(vec2 scroll) { script_scroll(scroll); }

static batch_renderer* batch;

void system_init() {
    PROFILE_FUNC();

#if defined(_DEBUG)
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, "../gamedir");
    MountResult mount_luacode = vfs_mount(NEKO_PACKS::LUACODE, "../source/game");
#else
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, nullptr);
    MountResult mount_luacode = {true};
#endif

    script_init();

    lua_State* L = ENGINE_LUA();

    g_app->is_fused.store(mount.is_fused);

    if (!g_app->error_mode.load() && mount.ok && mount_luacode.ok) {
        asset_load_kind(AssetKind_LuaRef, "conf.lua", nullptr);
    }

    // if (!g_app->error_mode.load()) {
    //     luax_neko_get(L, "arg");
    //     lua_createtable(L, game_get_argc() - 1, 0);
    //     for (i32 i = 1; i < game_get_argc(); i++) {
    //         lua_pushstring(L, game_get_argv()[i]);
    //         lua_rawseti(L, -2, i);
    //     }
    //     if (luax_pcall(L, 1, 0) != LUA_OK) {
    //         lua_pop(L, 1);
    //     }
    // }

    lua_newtable(L);
    i32 conf_table = lua_gettop(L);

    if (!g_app->error_mode.load()) {
        luax_neko_get(L, "conf");
        lua_pushvalue(L, conf_table);
        luax_pcall(L, 1, 0);
    }

    g_app->win_console = g_app->win_console || luax_boolean_field(L, -1, "win_console", true);

    bool hot_reload = luax_boolean_field(L, -1, "hot_reload", true);
    bool startup_load_scripts = luax_boolean_field(L, -1, "startup_load_scripts", true);
    bool fullscreen = luax_boolean_field(L, -1, "fullscreen", false);
    lua_Number reload_interval = luax_opt_number_field(L, -1, "reload_interval", 0.1);
    lua_Number swap_interval = luax_opt_number_field(L, -1, "swap_interval", 1);
    lua_Number target_fps = luax_opt_number_field(L, -1, "target_fps", 0);
    g_app->width = luax_opt_number_field(L, -1, "window_width", 800);
    g_app->height = luax_opt_number_field(L, -1, "window_height", 600);
    String title = luax_opt_string_field(L, -1, "window_title", "NekoEngine");
    String default_font = luax_opt_string_field(L, -1, "default_font", "assets/fonts/font.ttf");
    String imgui_font = luax_opt_string_field(L, -1, "imgui_font", "");
    g_app->lite_init_path = luax_opt_string_field(L, -1, "lite_init_path", "");
    g_app->debug_on = luax_boolean_field(L, -1, "debug_on", true);
    g_app->dump_allocs_detailed = luax_boolean_field(L, -1, "dump_allocs_detailed", false);
    String game_proxy = luax_opt_string_field(L, -1, "game_proxy", "default");

    lua_pop(L, 1);  // conf table

    // 刷新状态
    game_set_window_size(luavec2(g_app->width, g_app->height));

    if (fnv1a(game_proxy) == "default"_hash) {
        neko::neko_lua_run_string(L, R"lua(
            function neko.before_quit() game_proxy_before_quit() end
            function neko.start(arg) game_proxy_start(arg) end
            function neko.frame(dt) game_proxy_frame(dt) end
        )lua");
        console_log("using default game proxy");
    }

    CVAR(conf_hot_reload, hot_reload);
    CVAR(conf_startup_load_scripts, startup_load_scripts);
    CVAR(conf_fullscreen, fullscreen);
    CVAR(conf_reload_interval, reload_interval);
    CVAR(conf_swap_interval, swap_interval);
    CVAR(conf_target_fps, target_fps);
    CVAR(conf_width, g_app->width);
    CVAR(conf_height, g_app->height);
    CVAR(conf_title, title);
    CVAR(conf_imgui_font, imgui_font);
    CVAR(conf_debug_on, g_app->debug_on);
    CVAR(conf_game_proxy, game_proxy);
    CVAR(conf_default_font, default_font);

    g_render = gfx_create();
    gfx_init(g_render);

    neko_default_font();

    input_init();
    entity_init();
    transform_init();
    camera_init();
    batch = batch_init(6000);
    sprite_init();
    tiled_init();
    font_init();
    gui_init();
    console_init();
    sound_init();
    physics_init();
    edit_init();

    g_app->hot_reload_enabled.store(mount.can_hot_reload && hot_reload);
    g_app->reload_interval.store((u32)(reload_interval * 1000));

    luax_run_nekogame(L);

    if (!g_app->error_mode.load() && startup_load_scripts && mount.ok && mount_luacode.ok) {
        load_all_lua_scripts(L);
    }

    // run main.lua
    asset_load_kind(AssetKind_LuaRef, "script/nekomain.lua", nullptr);

    luax_get(ENGINE_LUA(), "neko", "__define_default_callbacks");
    luax_pcall(ENGINE_LUA(), 0, 0);

    luax_get(ENGINE_LUA(), "neko", "game_init");
    luax_pcall(ENGINE_LUA(), 0, 0);

    // fire init event
    script_push_event("init");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    input_add_key_down_callback(_key_down);
    input_add_key_up_callback(_key_up);
    input_add_char_down_callback(_char_down);
    input_add_mouse_down_callback(_mouse_down);
    input_add_mouse_up_callback(_mouse_up);
    input_add_mouse_move_callback(_mouse_move);
    input_add_scroll_callback(_scroll);

    if (target_fps != 0) {
        timing_instance.target_ticks = 1000000000 / target_fps;
    }

#ifdef NEKO_IS_WIN32
    if (!g_app->win_console) {
        FreeConsole();
    }
#endif
}

void system_fini() {
    PROFILE_FUNC();

    edit_fini();
    script_fini();
    physics_fini();
    sound_fini();
    console_fini();
    tiled_fini();
    sprite_fini();
    batch_fini(batch);
    gui_fini();
    camera_fini();
    transform_fini();
    entity_fini();
    input_fini();

    if (g_app->default_font != nullptr) {
        g_app->default_font->trash();
        mem_free(g_app->default_font);
    }

    gfx_fini(g_render);

    neko_dyn_array_free(g_app->shader_array);

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        // if (g_app->default_font != nullptr) {
        //     g_app->default_font->trash();
        //     mem_free(g_app->default_font);
        // }

#if NEKO_AUDIO == 1
        for (Sound* sound : g_app->garbage_sounds) {
            sound->trash();
        }
        g_app->garbage_sounds.trash();
#endif

        assets_shutdown();
    }
}

void system_update_all() {
    edit_clear();

    timing_update();

    scratch_update();

    script_update_all();

    keyboard_controlled_update_all();

    physics_update_all();
    transform_update_all();
    camera_update_all();
    gui_update_all();
    sprite_update_all();
    batch_update_all(batch);
    sound_update_all();
    tiled_update_all();

    edit_update_all();
    script_post_update_all();
    physics_post_update_all();

    entity_update_all();

    gui_event_clear();
}

void system_draw_all(command_buffer_t* cb) {
    // script_draw_all();
    // tiled_draw_all();
    tiled_draw_all();

    sprite_draw_all();
    batch_draw_all(batch);
    edit_draw_all();
    physics_draw_all();
    gui_draw_all();

    neko_check_gl_error();
}

// 以相同的顺序 保存/加载
static void _saveload_all(void* s, bool save) {
#define saveload(sys)              \
    if (save)                      \
        sys##_save_all((Store*)s); \
    else                           \
        sys##_load_all((Store*)s)

    entity_load_all_begin();

    saveload(entity);
    saveload(prefab);
    saveload(timing);
    saveload(transform);
    saveload(camera);
    saveload(sprite);
    saveload(physics);
    saveload(gui);
    saveload(edit);
    saveload(sound);

    saveload(keyboard_controlled);

    saveload(script);

    entity_load_all_end();
}

void system_save_all(Store* s) { _saveload_all(s, true); }

void system_load_all(Store* s) { _saveload_all(s, false); }

static NativeEntity saved_root;

void prefab_save(const char* filename, NativeEntity root) {
    Store* s;

    saved_root = root;
    s = store_open();
    system_save_all(s);
    store_write_file(s, filename);
    store_close(s);
    saved_root = entity_nil;
}
NativeEntity prefab_load(const char* filename) {
    Store* s;
    NativeEntity root;

    s = store_open_file(filename);
    system_load_all(s);
    store_close(s);
    root = saved_root;
    saved_root = entity_nil;

    return root;
}

void prefab_save_all(Store* s) {
    Store* t;

    if (store_child_save(&t, "prefab", s)) entity_save(&saved_root, "saved_root", t);
}
void prefab_load_all(Store* s) {
    Store* t;

    if (store_child_load(&t, "prefab", s)) entity_load(&saved_root, "saved_root", entity_nil, t);
}

/*=============================
// ECS Lua
=============================*/

enum match_mode {
    MATCH_ALL = 0,
    MATCH_DIRTY = 1,
    MATCH_DEAD = 2,
};

struct component {
    int eid;
    int dirty_next;
    int dead_next;
};

struct component_pool {
    int cap;
    int free;
    int dirty_head;
    int dirty_tail;
    int dead_head;
    int dead_tail;
    struct component* buf;
};

// 每个Component类型都有一个数字id称为tid
// 每个Component实例都有一个数字id称为cid
// 根据tid和cid来找到某一个具体的Component实例
struct component_ptr {
    int tid;
    int cid;
};

struct entity {
    int next;
    short cn;
    unsigned char components[32];
    int index[ENTITY_MAX_COMPONENTS];
};

struct match_ctx {
    struct l_ecs_world_t* w;
    int i;
    int kn;
    int keys[ENTITY_MAX_COMPONENTS];
};

struct l_ecs_world_t {
    int entity_cap;
    int entity_free;  // TODO:将ENTITY_FREE设置为FIFO 这可以在以后回收EID以避免某些错误
    int entity_dead;
    int type_idx;
    struct entity* entity_buf;
    struct component_pool component_pool[TYPE_COUNT];
};

static inline int component_has(struct entity* e, int tid) { return e->components[tid] < ENTITY_MAX_COMPONENTS; }

static inline void component_clr(struct entity* e, int tid) {
    unsigned char idx = e->components[tid];
    if (idx < ENTITY_MAX_COMPONENTS) {
        e->index[idx] = -1;
        e->components[tid] = ENTITY_MAX_COMPONENTS;
        --e->cn;
    }
}

static inline int component_add(lua_State* L, struct l_ecs_world_t* w, struct entity* e, int tid) {
    int cid, i;
    struct component* c;
    struct component_pool* cp;
    struct component_ptr* ptr;
    if (e->components[tid] < ENTITY_MAX_COMPONENTS) {
        return luaL_error(L, "NativeEntity(%d) already exist component(%d)", e - w->entity_buf, tid);
    }
    if (e->cn >= ENTITY_MAX_COMPONENTS) {
        return luaL_error(L, "NativeEntity(%d) add to many components", e - w->entity_buf);
    }
    // new component
    cp = &w->component_pool[tid];
    if (cp->free >= cp->cap) {
        cp->cap *= 2;
        cp->buf = (struct component*)mem_realloc(cp->buf, cp->cap * sizeof(cp->buf[0]));
    }
    c = &cp->buf[cp->free++];
    c->eid = e - w->entity_buf;
    c->dirty_next = LINK_NONE;
    c->dead_next = LINK_NONE;
    cid = c - cp->buf;
    // add component into entity
    for (i = 0; i < ENTITY_MAX_COMPONENTS; i++) {
        if (e->index[i] < 0) break;
    }
    assert(i < ENTITY_MAX_COMPONENTS);
    e->components[tid] = i;
    e->index[i] = cid;
    return cid;
}

static inline void component_dead(struct l_ecs_world_t* w, int tid, int cid) {
    struct component* c;
    struct component_pool* cp;
    cp = &w->component_pool[tid];
    c = &cp->buf[cid];
    if (c->dead_next != LINK_NONE)  // already dead?
        return;
    c->dead_next = LINK_NIL;
    if (cp->dead_tail == LINK_NIL) {
        cp->dead_tail = cid;
        cp->dead_head = cid;
    } else {
        cp->buf[cp->dead_tail].dead_next = cid;
        cp->dead_tail = cid;
    }
    return;
}

static inline void component_dirty(struct l_ecs_world_t* w, int tid, int cid) {
    struct component* c;
    struct component_pool* cp;
    cp = &w->component_pool[tid];
    c = &cp->buf[cid];
    if (c->dirty_next != LINK_NONE)  // already diryt
        return;
    c->dirty_next = LINK_NIL;
    if (cp->dirty_tail == LINK_NIL) {
        cp->dirty_tail = cid;
        cp->dirty_head = cid;
    } else {
        cp->buf[cp->dirty_tail].dirty_next = cid;
        cp->dirty_tail = cid;
    }
    return;
}

static inline struct entity* entity_alloc(struct l_ecs_world_t* w) {
    struct entity* e;
    int eid = w->entity_free;
    if (eid < 0) {
        int i = 0;
        int oldcap = w->entity_cap;
        int newcap = oldcap * 2;
        w->entity_cap = newcap;
        w->entity_buf = (struct entity*)mem_realloc(w->entity_buf, newcap * sizeof(w->entity_buf[0]));
        w->entity_free = oldcap + 1;
        e = &w->entity_buf[oldcap];
        for (i = w->entity_free; i < newcap - 1; i++) {
            w->entity_buf[i].cn = -1;
            w->entity_buf[i].next = i + 1;
        }
        w->entity_buf[newcap - 1].cn = -1;
        w->entity_buf[newcap - 1].next = LINK_NIL;
    } else {
        e = &w->entity_buf[eid];
        w->entity_free = e->next;
    }
    e->cn = 0;
    memset(e->components, ENTITY_MAX_COMPONENTS, sizeof(e->components));
    memset(e->index, -1, sizeof(e->index));
    e->next = LINK_NONE;
    return e;
}

static inline void entity_dead(struct l_ecs_world_t* w, struct entity* e) {
    int t;
    if (e->cn < 0) {
        assert(e->next != LINK_NONE);
        return;
    }
    assert(e->next == LINK_NONE);
    for (t = TYPE_MIN_ID; t <= w->type_idx; t++) {
        int i = e->components[t];
        if (i < ENTITY_MAX_COMPONENTS) component_dead(w, t, e->index[i]);
    }
    e->next = w->entity_dead;
    w->entity_dead = e - w->entity_buf;
}

static inline void entity_free(struct l_ecs_world_t* w, struct entity* e) {
    assert(e->cn == -1);
    e->next = w->entity_free;
    w->entity_free = e - w->entity_buf;
}

static inline int get_typeid(lua_State* L, int stk, int proto_id) {
    int id;
    stk = lua_absindex(L, stk);
    lua_pushvalue(L, stk);
    lua_gettable(L, proto_id);
    id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    luaL_argcheck(L, id >= TYPE_MIN_ID, stk, "invalid type");
    return id;
}

static inline struct entity* get_entity(lua_State* L, struct l_ecs_world_t* w, int stk) {
    struct entity* e;
    int eid = luaL_checkinteger(L, stk);
    luaL_argcheck(L, eid < w->entity_cap, 2, "eid is invalid");
    e = &w->entity_buf[eid];
    luaL_argcheck(L, e->cn >= 0, 2, "entity is dead");
    return e;
}

static inline int get_cid_in_entity(struct entity* e, int tid) {
    int i = e->components[tid];
    if (i >= ENTITY_MAX_COMPONENTS) return -1;
    return e->index[i];
}

static inline void update_cid_in_entity(struct entity* e, int tid, int cid) {
    int i = e->components[tid];
    assert(i < ENTITY_MAX_COMPONENTS);
    e->index[i] = cid;
}

static int l_ecs_world_end(lua_State* L) {
    struct l_ecs_world_t* w;
    int type, tid;
    struct component_pool* cp;
    w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    tid = w->type_idx;  // 总数-1(索引)
    for (int i = 0; i <= tid; i++) {
        cp = &w->component_pool[i];
        mem_free(cp->buf);
    }
    mem_free(w->entity_buf);
    return 0;
}

static int l_ecs_world_register(lua_State* L) {
    struct l_ecs_world_t* w;
    int type, tid;
    struct component_pool* cp;
    w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    // check if 'componentA' has been declared
    lua_pushvalue(L, 2);
    type = lua_gettable(L, -2);
    luaL_argcheck(L, type == LUA_TNIL, 1, "duplicated component type");
    lua_pop(L, 1);
    // new component type
    tid = ++w->type_idx;
    luaL_argcheck(L, tid >= TYPE_MIN_ID && tid <= TYPE_MAX_ID, 1, "component type is too may");
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    lua_createtable(L, 128, 0);
    lua_seti(L, -2, tid);
    lua_pop(L, 1);
    cp = &w->component_pool[tid];
    cp->cap = 64;
    cp->free = 0;
    cp->dirty_head = LINK_NIL;
    cp->dirty_tail = LINK_NIL;
    cp->dead_head = LINK_NIL;
    cp->dead_tail = LINK_NIL;
    cp->buf = (struct component*)mem_alloc(cp->cap * sizeof(cp->buf[0]));
    // set proto id
    lua_pushvalue(L, 2);
    lua_pushinteger(L, tid);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    ////////
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_DEFINE);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    return 0;
}

static int l_ecs_world_new_entity(lua_State* L) {
    int i, components, proto_id;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* e = entity_alloc(w);
    int eid = e - w->entity_buf;
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    components = lua_gettop(L);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = components + 1;
    lua_pushnil(L);  //  first key
    while (lua_next(L, 2) != 0) {
        int cid;
        int tid = get_typeid(L, -2, proto_id);
        lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_EID);
        lua_pushinteger(L, eid);
        lua_settable(L, -3);
        lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_TID);
        lua_pushinteger(L, tid);
        lua_settable(L, -3);
        cid = component_add(L, w, e, tid);
        luaL_argcheck(L, cid >= 0, 2, "entity has duplicated component");
        lua_geti(L, components, tid);
        lua_pushvalue(L, -2);
        lua_seti(L, -2, cid);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);
    lua_pushinteger(L, eid);
    return 1;
}

static int l_ecs_world_del_entity(lua_State* L) {
    int i;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* e = get_entity(L, w, 2);
    entity_dead(w, e);
    return 0;
}

static int l_ecs_world_get_component(lua_State* L) {
    int i, top, proto_id, components;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* e = get_entity(L, w, 2);
    top = lua_gettop(L);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = top + 1;
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    components = top + 2;
    for (i = 3; i <= top; i++) {
        int tid = get_typeid(L, i, proto_id);
        int cid = get_cid_in_entity(e, tid);
        if (cid >= 0) {
            lua_geti(L, components, tid);
            lua_geti(L, -1, cid);
            lua_replace(L, -2);
        } else {
            lua_pushnil(L);
        }
    }
    return (top - 3 + 1);
}

static int l_ecs_world_add_component(lua_State* L) {
    int tid, cid;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* e = get_entity(L, w, 2);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    tid = get_typeid(L, 3, lua_gettop(L));
    cid = component_add(L, w, e, tid);
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    lua_geti(L, -1, tid);
    lua_pushvalue(L, 4);
    lua_seti(L, -2, cid);
    lua_pop(L, 3);
    return 0;
}

static int l_ecs_world_remove_component(lua_State* L) {
    int i, proto_id;
    int tid, cid;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* e = get_entity(L, w, 2);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = lua_gettop(L);
    for (i = 3; i <= (proto_id - 1); i++) {
        tid = get_typeid(L, i, proto_id);
        cid = get_cid_in_entity(e, tid);
        component_dead(w, tid, cid);
    }
    lua_pop(L, 1);
    return 0;
}

static int l_ecs_world_touch_component(lua_State* L) {
    int eid, tid, cid;
    struct entity* e;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_EID);
    lua_gettable(L, 2);
    eid = luaL_checkinteger(L, -1);
    luaL_argcheck(L, eid > 0 && eid < w->entity_cap, 2, "invalid component");
    lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_TID);
    lua_gettable(L, 2);
    tid = luaL_checkinteger(L, -1);
    luaL_argcheck(L, tid >= TYPE_MIN_ID && tid <= TYPE_MAX_ID, 2, "invalid component");
    e = &w->entity_buf[eid];
    cid = get_cid_in_entity(e, tid);
    component_dirty(w, tid, cid);
    return 0;
}

static void push_result(lua_State* L, int* keys, int kn, struct entity* e) {
    int i;
    if (e != NULL) {  // match one
        int components;
        lua_getiuservalue(L, 1, 1);
        lua_getiuservalue(L, -1, WORLD_COMPONENTS);
        lua_replace(L, -2);
        components = lua_gettop(L);
        for (i = 0; i < kn; i++) {
            int tid = keys[i];
            int cid = get_cid_in_entity(e, tid);
            lua_geti(L, components, tid);
            lua_geti(L, -1, cid);
            lua_replace(L, -2);
        }
    } else {
        for (i = 0; i < kn; i++) lua_pushnil(L);
    }
}

static inline struct entity* restrict_component(struct entity* ebuf, struct component* c, int* keys, int kn) {
    int i;
    struct entity* e;
    e = &ebuf[c->eid];
    for (i = 1; i < kn; i++) {
        if (!component_has(e, keys[i])) return NULL;
    }
    return e;
}

// match_all(match_ctx *mctx, nil)
static int match_all(lua_State* L) {
    int i;
    int mi, free;
    int kn, *keys;
    struct entity* e = NULL;
    struct component_pool* cp;
    struct match_ctx* mctx = (struct match_ctx*)lua_touserdata(L, 1);
    struct l_ecs_world_t* w = mctx->w;
    struct entity* entity_buf = w->entity_buf;
    kn = mctx->kn;
    keys = mctx->keys;
    cp = &w->component_pool[mctx->keys[0]];
    mi = mctx->i;
    free = cp->free;
    while (mi < free) {
        struct component* c = &cp->buf[mi++];
        if (c->dead_next == LINK_NONE) {
            e = restrict_component(entity_buf, c, keys, kn);
            if (e != NULL) break;
        }
    }
    mctx->i = mi;
    push_result(L, keys, kn, e);
    return kn;
}

static int match_dirty(lua_State* L) {
    int next;
    int kn, *keys;
    struct entity* e = NULL;
    struct component_pool* cp;
    struct match_ctx* mctx = (struct match_ctx*)lua_touserdata(L, 1);
    struct l_ecs_world_t* w = mctx->w;
    struct entity* entity_buf = w->entity_buf;
    keys = mctx->keys;
    kn = mctx->kn;
    next = mctx->i;
    cp = &w->component_pool[keys[0]];
    while (next != LINK_NIL) {
        struct component* c = &cp->buf[next];
        next = c->dirty_next;
        if (c->dead_next == LINK_NONE) {
            e = restrict_component(entity_buf, c, keys, kn);
            if (e != NULL) break;
        }
    }
    mctx->i = next;
    push_result(L, keys, kn, e);
    return kn;
}

static int match_dead(lua_State* L) {
    int next;
    int kn, *keys;
    struct entity* e = NULL;
    struct component_pool* cp;
    struct match_ctx* mctx = (struct match_ctx*)lua_touserdata(L, 1);
    struct l_ecs_world_t* w = mctx->w;
    struct entity* entity_buf = w->entity_buf;
    keys = mctx->keys;
    kn = mctx->kn;
    next = mctx->i;
    cp = &w->component_pool[keys[0]];
    while (next != LINK_NIL) {
        struct component* c = &cp->buf[next];
        next = c->dead_next;
        e = restrict_component(entity_buf, c, keys, kn);
        if (e != NULL) break;
    }
    mctx->i = next;
    push_result(L, keys, kn, e);
    return kn;
}

static int l_ecs_world_match_component(lua_State* L) {
    int i;
    size_t sz;
    struct l_ecs_world_t* w;
    enum match_mode mode;
    int (*iter)(lua_State* L) = NULL;
    struct match_ctx* mctx = NULL;
    const char* m = luaL_checklstring(L, 2, &sz);
    int top = lua_gettop(L);
    w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    switch (sz) {
        case 3:
            if (m[0] == 'a' && m[1] == 'l' && m[2] == 'l') {
                iter = match_all;
                mode = MATCH_ALL;
            }
            break;
        case 5:
            if (memcmp(m, "dirty", 5) == 0) {
                iter = match_dirty;
                mode = MATCH_DIRTY;
            }
            break;
        case 4:
            if (memcmp(m, "dead", 4) == 0) {
                iter = match_dead;
                mode = MATCH_DEAD;
            }
            break;
    }
    if (iter == NULL) return luaL_argerror(L, 2, "mode can only be[all,dirty,dead]");
    luaL_argcheck(L, top >= 3, 3, "lost the component name");
    luaL_argcheck(L, top < NEKO_ARR_SIZE(mctx->keys), top, "too many component");
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    lua_pushcfunction(L, iter);
    lua_getiuservalue(L, ECS_WORLD, WORLD_MATCH_CTX);
    mctx = (struct match_ctx*)lua_touserdata(L, -1);
    mctx->w = w;
    mctx->kn = 0;
    for (i = 3; i <= top; i++) mctx->keys[mctx->kn++] = get_typeid(L, i, top + 1);
    switch (mode) {
        case MATCH_ALL:
            mctx->i = 0;
            break;
        case MATCH_DIRTY:
            mctx->i = w->component_pool[mctx->keys[0]].dirty_head;
            break;
        case MATCH_DEAD:
            mctx->i = w->component_pool[mctx->keys[0]].dead_head;
            break;
    }
    lua_pushnil(L);
    return 3;
}

static int l_ecs_world_update(lua_State* L) {
    int next;
    int t, components;
    struct component_pool* pool;
    struct l_ecs_world_t* w = (struct l_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* entity_buf = w->entity_buf;
    // clear dead entity
    next = w->entity_dead;
    while (next != LINK_NIL) {
        struct entity* e;
        e = &entity_buf[next];
        e->cn = -1;
        next = e->next;
        entity_free(w, e);
    }
    pool = w->component_pool;
    // clear dead component
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    for (t = 0; t <= w->type_idx; t++) {
        int w = 0, r = 0, free;
        struct component* buf;
        struct component_pool* cp;
        cp = &pool[t];
        cp->dirty_head = LINK_NIL;
        cp->dirty_tail = LINK_NIL;
        cp->dead_head = LINK_NIL;
        cp->dead_tail = LINK_NIL;
        lua_geti(L, -1, t);
        buf = cp->buf;
        free = cp->free;
        for (r = 0; r < free; r++) {
            struct component* c;
            c = &buf[r];
            c->dirty_next = LINK_NONE;
            if (c->dead_next == LINK_NONE) {  // alive
                if (w != r) {
                    struct entity* e;
                    e = &entity_buf[c->eid];
                    buf[w] = *c;
                    lua_geti(L, -1, r);
                    lua_seti(L, -2, w);
                    update_cid_in_entity(e, t, w);
                }
                w++;
            } else {  // dead component
                struct entity* e;
                e = &entity_buf[c->eid];
                if (e->next == LINK_NONE) component_clr(e, t);
            }
        }
        cp->free = w;
        while (w < free) {
            lua_pushnil(L);
            lua_seti(L, -2, w);
            w++;
        }
        lua_pop(L, 1);
    }
    return 0;
}

static void print_value(lua_State* L, int stk, int tab) {
    switch (lua_type(L, stk)) {
        case LUA_TTABLE:
            printf("%*p=>", tab * 4, lua_topointer(L, stk));
            break;
        case LUA_TSTRING:
            printf("%*s=>", tab * 4, lua_tostring(L, stk));
            break;
        default:
            printf("%*lld=>", tab * 4, (long long)lua_tointeger(L, stk));
            break;
    }
}

static void dump_table(lua_State* L, int stk, int tab) {
    lua_pushnil(L);  //  first key
    while (lua_next(L, stk) != 0) {
        // print key
        print_value(L, -2, tab);
        switch (lua_type(L, -1)) {
            case LUA_TTABLE:
                printf("{\n");
                dump_table(L, lua_absindex(L, -1), tab + 1);
                printf("}\n");
                break;
            case LUA_TSTRING:
                printf("%*s\n", tab * 4, lua_tostring(L, -1));
                break;
            default:
                printf("%*lld\n", tab * 4, (long long)lua_tointeger(L, -1));
                break;
        }
        lua_pop(L, 1);
    }
}

static void dump_upval(lua_State* L, int upval) {
    const char* name[] = {
            "PROTO_ID",
            "PROTO_DEFINE",
            "COMPONENTS",
    };
    printf("==dump== %s\n", name[upval - 1]);
    if (lua_getiuservalue(L, ECS_WORLD, upval) == LUA_TTABLE) dump_table(L, lua_gettop(L), 0);
    lua_pop(L, 1);
}

static int l_ecs_world_dump(lua_State* L) {
    int i;
    for (i = 1; i <= WORLD_COMPONENTS; i++) dump_upval(L, i);
    return 0;
}

int l_ecs_create_world(lua_State* L) {
    int i;
    struct l_ecs_world_t* w;
    struct match_ctx* mctx;
    w = (struct l_ecs_world_t*)lua_newuserdatauv(L, sizeof(*w), WORLD_UPVAL_N);
    memset(w, 0, sizeof(*w));
    w->entity_cap = 128;
    w->entity_free = 0;
    w->entity_dead = LINK_NIL;
    w->type_idx = 0;
    w->entity_buf = (struct entity*)mem_alloc(w->entity_cap * sizeof(w->entity_buf[0]));
    for (i = 0; i < w->entity_cap - 1; i++) {
        w->entity_buf[i].cn = -1;
        w->entity_buf[i].next = i + 1;
    }
    w->entity_buf[w->entity_cap - 1].cn = -1;
    w->entity_buf[w->entity_cap - 1].next = LINK_NIL;
    if (luaL_getmetatable(L, ECS_WORLD_UDATA_NAME) == LUA_TNIL) {
        luaL_Reg world_mt[] = {
                {"__index", NULL},
                {"__name", NULL},
                {"__gc", l_ecs_world_end},
                {"register", l_ecs_world_register},
                {"new", l_ecs_world_new_entity},
                {"del", l_ecs_world_del_entity},
                {"get", l_ecs_world_get_component},
                {"add", l_ecs_world_add_component},
                {"remove", l_ecs_world_remove_component},
                {"touch", l_ecs_world_touch_component},
                {"match", l_ecs_world_match_component},
                {"update", l_ecs_world_update},
                {"dump", l_ecs_world_dump},
                {NULL, NULL},
        };
        lua_pop(L, 1);
        luaL_newlibtable(L, world_mt);
        luaL_setfuncs(L, world_mt, 0);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        lua_pushliteral(L, ECS_WORLD_UDATA_NAME);
        lua_setfield(L, -2, "__name");
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, ECS_WORLD_UDATA_NAME);
    }
    lua_setmetatable(L, -2);

    lua_createtable(L, 0, 128);
    lua_setiuservalue(L, 1, WORLD_PROTO_ID);

    lua_createtable(L, 0, 128);
    lua_setiuservalue(L, 1, WORLD_PROTO_DEFINE);

    lua_createtable(L, TYPE_MAX_ID, 0);
    lua_setiuservalue(L, 1, WORLD_COMPONENTS);

    mctx = (struct match_ctx*)lua_newuserdatauv(L, sizeof(*mctx), 1);
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);
    lua_setiuservalue(L, 1, WORLD_MATCH_CTX);

    lua_pushliteral(L, "__eid");
    lua_setiuservalue(L, 1, WORLD_KEY_EID);

    lua_pushliteral(L, "__tid");
    lua_setiuservalue(L, 1, WORLD_KEY_TID);

    return 1;
}