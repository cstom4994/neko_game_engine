
#include "engine/ecs/entity.h"

#include <stdbool.h>
#include <stdlib.h>

#include "editor/editor.hpp"
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
#include "base/scripting/lua_wrapper.hpp"
#include "base/scripting/scripting.h"
#include "engine/test.h"
#include "engine/ui.h"
#include "lua.h"

using namespace Neko::luabind;
using namespace Neko::ecs;

CEntity entity_nil = {0};  // 没有有效的实体具有此值
static EcsId counter = 1;

// 声明未使用的实体 ID
CEntity entity_create(const String& name) {
    CEntity ent;
    auto L = ENGINE_LUA();

    LuaRef table = LuaRef::NewTable(L);
    table["name"] = name.cstr();

    Entity* e = EcsEntityNew(L, table, NULL);
    ent.id = e - ENGINE_ECS()->entity_buf;
    return ent;
}

static void entity_remove(CEntity ent) { EcsEntityDel(ENGINE_LUA(), ent.id); }

// 释放实体 ID
void entity_destroy(CEntity ent) { entity_remove(ent); }

void entity_destroy_all() {}

bool entity_destroyed(CEntity ent) { return NULL == EcsGetEnt(ENGINE_LUA(), ENGINE_ECS(), ent.id); }

void entity_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    EcsRegister(L, "CTag");

    {
        int* a = new int(7);

        LuaRef table = LuaRef::NewTable(L);
        table["testing"] = "iNIT C";
        table["afucking_ptr"] = a;

        EcsEntityNew(L, table, NULL);
    }

    //struct CTransform {};

    //int type_transform = EcsRegisterCType<CTransform>(L);

    //{
    //    LuaRef table = LuaRef::NewTable(L);
    //    table["testing"] = "Test fucking CTransform";

    //    Entity* e = EcsEntityNew(L, table, NULL);

    //    LuaRef tb = LuaRef::NewTable(L);
    //    tb["testing"] = "i'm CTransform haha";
    //    tb["__my_ud"] = new int(114514);

    //    int cid1 = EcsComponentSet(L, e, type_transform, tb);

    //    LuaRef tb_read = EcsComponentGet(L, e, type_transform);
    //    int* test_ptr = tb_read["__my_ud"];
    //    LOG_TRACE("__my_ud {}", *test_ptr);
    //    // DumpLuaRef(tb_read);
    //}
}

void entity_fini() {}

int entity_update_all(App* app, Event evt) {
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
