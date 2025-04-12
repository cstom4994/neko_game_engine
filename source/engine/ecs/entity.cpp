
#include "engine/ecs/entity.h"

#include <stdbool.h>
#include <stdlib.h>

#include "engine/editor.h"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/os.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/lua_ecs.hpp"
#include "engine/edit.h"
#include "engine/graphics.h"
#include "engine/imgui.hpp"
#include "engine/input.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/scripting.h"
#include "engine/test.h"
#include "engine/ui.h"
#include "engine/scripting/lua_util.h"

using namespace Neko::luabind;
using namespace Neko::ecs;

CEntity entity_nil = {0};  // 没有有效的实体具有此值
static EcsId counter = 1;

// 声明未使用的实体 ID
CEntity entity_create(const String& name) {
    CEntity ent;
    auto L = ENGINE_LUA();

    LuaRef table = LuaRef::NewTable(L);
    table["__name"] = name.cstr();

    EntityData* e = EcsEntityNew(L, table, NULL);
    ent.id = e - ENGINE_ECS()->entity_buf;
    return ent;
}

void entity_destroy_all() {}

bool entity_destroyed(CEntity ent) { return NULL == EcsGetEnt(ENGINE_LUA(), ENGINE_ECS(), ent.id); }

int wrap_EntityCreate(lua_State* L) {
    String name = luax_opt_string(L, 1, "something_unknown_from_lua");
    CEntity ent = entity_create(name);
    LuaPush<CEntity>(L, ent);
    return 1;
}

int wrap_EntityDestroy(lua_State* L) {
    CEntity* ent = LuaGet<CEntity>(L, 1);
    EcsEntityDel(ENGINE_LUA(), ent->id);
    return 0;
}

int wrap_EntityDestroyAll(lua_State* L) {
    entity_destroy_all();
    return 0;
}

int wrap_EntityDestroyed(lua_State* L) {
    CEntity* ent = LuaGet<CEntity>(L, 1);
    bool v = entity_destroyed(*ent);
    lua_pushboolean(L, v);
    return 1;
}

int wrap_CEntityEq(lua_State* L) {
    EcsId a = lua_tointeger(L, 1);
    EcsId b = lua_tointeger(L, 2);
    bool v = (a == b);
    lua_pushboolean(L, v);
    return 1;
}

int wrap_entity_set_save_filter(lua_State* L) {
    CEntity* ent = LuaGet<CEntity>(L, 1);
    bool filter = lua_toboolean(L, 2);
    // entity_set_save_filter(*ent, filter);
    return 0;
}

int wrap_entity_get_save_filter(lua_State* L) {
    CEntity* ent = LuaGet<CEntity>(L, 1);
    // bool v = entity_get_save_filter(*ent);
    bool v = true;
    lua_pushboolean(L, v);
    return 1;
}

void Entity::entity_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    EcsRegister(L, "CTag");

    {
        int* a = new int(7);

        LuaRef table = LuaRef::NewTable(L);

        table["__name"] = "NekoWorldRoot";

        table["testing"] = "iNIT C";
        table["afucking_ptr"] = a;

        EcsEntityNew(L, table, NULL);
    }

    // if (0) {
    //     LuaRef table = LuaRef::NewTable(L);
    //     table["__name"] = "TestDel";
    //     auto e = EcsEntityNew(L, table, NULL);
    //     EcsWorld* world = ENGINE_ECS();
    //     EcsEntityDead(world, e);
    //     EcsUpdate(L);
    // }

    // struct CTransform {};
    // int type_transform = EcsRegisterCType<CTransform>(L);
    //{
    //    LuaRef table = LuaRef::NewTable(L);
    //    table["testing"] = "Test fucking CTransform";
    //    EntityData* e = EcsEntityNew(L, table, NULL);
    //    LuaRef tb = LuaRef::NewTable(L);
    //    tb["testing"] = "i'm CTransform haha";
    //    tb["__my_ud"] = new int(114514);
    //    int cid1 = EcsComponentSet(L, e, type_transform, tb);
    //    LuaRef tb_read = EcsComponentGet(L, e, type_transform);
    //    int* test_ptr = tb_read["__my_ud"];
    //    LOG_TRACE("__my_ud {}", *test_ptr);
    //    // DumpLuaRef(tb_read);
    //}

    auto type = BUILD_TYPE(Entity)  //
                        .CClosure({
                                {"EntityCreate", wrap_EntityCreate},
                                {"EntityDestroy", wrap_EntityDestroy},
                                {"EntityDestroyAll", wrap_EntityDestroyAll},
                                {"EntityDestroyed", wrap_EntityDestroyed},
                                {"CEntityEq", wrap_CEntityEq},
                                {"entity_set_save_filter", wrap_entity_set_save_filter},
                                {"entity_get_save_filter", wrap_entity_get_save_filter},
                        })
                        .Build();
}

void Entity::entity_fini() {}

int Entity::entity_update_all(Event evt) {
    // EcsId i;
    // DestroyEntry* entry;

    // for (i = 0; i < array_length(destroyed);) {
    //     entry = (DestroyEntry*)array_get(destroyed, i);
    //     if (entry->pass == 0) {
    //         ++entry->pass;
    //         ++i;
    //     } else {
    //         entity_remove(entry->ent);
    //         array_quick_remove(destroyed, i);
    //     }
    // }

    EcsUpdate(ENGINE_LUA());

    return 0;
}

#define MIN_CAPACITY 2

static void entitymap_init_w(CEntityMap* emap) {
    EcsId i;

    emap->bound = 0;                // bound <= capacity (so that maximum key < capacity)
    emap->capacity = MIN_CAPACITY;  // MIN_CAPACITY <= capacity
    emap->arr = (int*)mem_alloc(emap->capacity * sizeof(*emap->arr));

    for (i = 0; i < emap->capacity; ++i) emap->arr[i] = emap->def;
}

CEntityMap* entitymap_new(int def) {
    CEntityMap* emap;

    emap = (CEntityMap*)mem_alloc(sizeof(CEntityMap));
    emap->def = def;

    entitymap_init_w(emap);

    return emap;
}
void entitymap_clear(CEntityMap* emap) {
    mem_free(emap->arr);
    entitymap_init_w(emap);
}
void entitymap_free(CEntityMap* emap) {
    mem_free(emap->arr);
    mem_free(emap);
}

static void _grow(CEntityMap* emap) {
    EcsId new_capacity, i, bound;

    // find next power of 2 (TODO: use log?)
    bound = emap->bound;
    for (new_capacity = emap->capacity; new_capacity < bound; new_capacity <<= 1);

    // grow, clear new
    emap->arr = (int*)mem_realloc(emap->arr, new_capacity * sizeof(*emap->arr));
    for (i = emap->capacity; i < new_capacity; ++i) emap->arr[i] = emap->def;
    emap->capacity = new_capacity;
}
static void _shrink(CEntityMap* emap) {
    EcsId new_capacity, bound_times_4;

    if (emap->capacity <= MIN_CAPACITY) return;

    // halve capacity while bound is less than a fourth
    bound_times_4 = emap->bound << 2;
    if (bound_times_4 >= emap->capacity) return;
    for (new_capacity = emap->capacity; new_capacity > MIN_CAPACITY && bound_times_4 < new_capacity; new_capacity >>= 1);
    if (new_capacity < MIN_CAPACITY) new_capacity = MIN_CAPACITY;

    emap->arr = (int*)mem_realloc(emap->arr, new_capacity * sizeof(*emap->arr));
    emap->capacity = new_capacity;
}

void entitymap_set(CEntityMap* emap, CEntity ent, int val) {
    if (val == emap->def)  // 判断删除操作
    {
        emap->arr[ent.id] = val;
        // 可能会向下移动并_shrink
        if (emap->bound == ent.id + 1) {
            while (emap->bound > 0 && emap->arr[emap->bound - 1] == emap->def) --emap->bound;
            _shrink(emap);
        }
    } else {
        // 可能会受到限制并_grow
        if (ent.id + 1 > emap->bound) {
            emap->bound = ent.id + 1;
            if (ent.id >= emap->capacity) _grow(emap);
        }
        emap->arr[ent.id] = val;
    }
}
int entitymap_get(CEntityMap* emap, CEntity ent) {
    if (ent.id >= emap->capacity) return emap->def;
    return emap->arr[ent.id];
}
