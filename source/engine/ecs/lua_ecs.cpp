#include "lua_ecs.hpp"

#include "base/common/logger.hpp"

namespace Neko {
namespace ecs {

using namespace luabind;

int EcsComponentHas(EntityData* e, int tid) { return e->components[tid] < ENTITY_MAX_COMPONENTS; }

void EcsComponentClear(EntityData* e, int tid) {
    int i = e->components[tid];
    if (i < ENTITY_MAX_COMPONENTS) {                 // 组件存在
        e->components[tid] = ENTITY_MAX_COMPONENTS;  // 移除组件
        e->components_index[i] = -1;                 // 清除索引
        --e->components_count;                       // 组件计数减少
    }
}

int EcsComponentAlloc(EcsWorld* world, EntityData* e, int tid) {
    if (e->components[tid] < ENTITY_MAX_COMPONENTS) {  // 组件已存在
        LOG_WARN("CEntity({}) already exist component({})", e - world->entity_buf, tid);
        return -1;
    }
    if (e->components_count >= ENTITY_MAX_COMPONENTS) {  // 超过组件上限
        LOG_WARN("CEntity({}) add to many components", e - world->entity_buf);
        return -1;
    }

    // 获取组件池
    ComponentPool* cp = &world->component_pool[tid];
    if (cp->free_idx >= cp->cap) {
        cp->cap *= 2;  // 扩容
        cp->buf = (ComponentData*)mem_realloc(cp->buf, cp->cap * sizeof(cp->buf[0]));
    }

    // 获取新组件
    ComponentData* c = &cp->buf[cp->free_idx++];
    c->eid = e - world->entity_buf;  // 记录组件附着的实体ID
    c->dirty_next = LINK_NONE;       // 活跃默认状态
    c->dead_next = LINK_NONE;        // 活跃默认状态
    int cid = c - cp->buf;           // 计算组件在组件池的索引

    // 将组件添加到实体中
    for (int i = 0; i < ENTITY_MAX_COMPONENTS; i++) {
        if (e->components_index[i] < 0) {  // 找到空位
            e->components[tid] = i;        // 编号为tid的组件在components_index中的位置为i
            e->components_index[i] = cid;  // components_index[i]存储该组件在组件池中的位置
            break;
        }
    }

    ++e->components_count;

    return cid;
}

void EcsComponentDead(EcsWorld* world, int tid, int cid) {
    ComponentPool* cp = &world->component_pool[tid];  // 获取索引为tid的组件标记数据
    ComponentData* c = &cp->buf[cid];                 // 获取索引为cid的组件

    if (c->dead_next != LINK_NONE) return;  // 判断是否已经死亡
    c->dead_next = LINK_NIL;                // 标记为死亡

    // 更新组件池
    if (cp->dead_tail == LINK_NIL) {  // 如果死亡链为空
        cp->dead_head = cid;
    } else {  // 否则修改dead_next
        cp->buf[cp->dead_tail].dead_next = cid;
    }
    cp->dead_tail = cid;
    return;
}

void EcsComponentDirty(EcsWorld* world, int tid, int cid) {
    ComponentPool* cp = &world->component_pool[tid];  // 获取索引为tid的组件标记数据
    ComponentData* c = &cp->buf[cid];                 // 获取索引为cid的组件

    if (c->dirty_next != LINK_NONE) return;  // 判断是否已经标记
    c->dirty_next = LINK_NIL;                // 标记

    // 更新组件池
    if (cp->dirty_tail == LINK_NIL) {
        cp->dirty_head = cid;
    } else {
        cp->buf[cp->dirty_tail].dirty_next = cid;
    }
    cp->dirty_tail = cid;
    return;
}

EntityData* EcsEntityAlloc(EcsWorld* world) {
    EntityData* e{};
    int eid = world->entity_free_id;
    if (eid < 0) {  // 如果 entity_free 是负值 表示当前没有可用的实体位
        int i = 0;
        int oldcap = world->entity_cap;  // 原实体容量
        int newcap = oldcap * 2;         // 现实体容量为两倍
        world->entity_cap = newcap;
        world->entity_buf = (EntityData*)mem_realloc(world->entity_buf, newcap * sizeof(world->entity_buf[0]));
        world->entity_free_id = oldcap + 1;  // 更新闲置实体id

        e = &world->entity_buf[oldcap];  // 可用实体位
        {                                // 更新 entity_buf 的初始状态
            for (i = world->entity_free_id; i < newcap - 1; i++) {
                world->entity_buf[i].components_count = -1;
                world->entity_buf[i].next = i + 1;
            }
            world->entity_buf[newcap - 1].components_count = -1;
            world->entity_buf[newcap - 1].next = LINK_NIL;
        }
    } else {                              // 有可用实体位
        e = &world->entity_buf[eid];      // 可用实体位的指针
        world->entity_free_id = e->next;  // 更新闲置实体id
    }
    e->components_count = 0;
    std::memset(e->components, ENTITY_MAX_COMPONENTS, sizeof(e->components));
    std::memset(e->components_index, -1, sizeof(e->components_index));
    e->next = LINK_NONE;
    ++world->entity_count;
    return e;
}

void EcsEntityDead(EcsWorld* world, EntityData* e) {
    if (e->components_count < 0) {  // 检查实体组件数量components_count是否小于0 如果是则表示该实体已经被标记为死亡或无效
        assert(e->next != LINK_NONE);
        return;
    }
    assert(e->next == LINK_NONE);  // 确保实体是已被分配的
    for (int tid = TYPE_MIN_ID; tid <= world->type_idx; tid++) {
        int i = e->components[tid];  //

        // i 有效值为 [0,ENTITY_MAX_COMPONENTS-1]
        if (i < ENTITY_MAX_COMPONENTS) EcsComponentDead(world, tid, e->components_index[i]);
    }
    e->next = world->entity_dead_id;
    world->entity_dead_id = e - world->entity_buf;
}

void EcsEntityFree(EcsWorld* world, EntityData* e) {
    assert(e->components_count == -1);  //
    e->next = world->entity_free_id;
    world->entity_free_id = e - world->entity_buf;
    --world->entity_count;
}

int EcsGetTid_w(lua_State* L, int stk, int proto_id) {
    stk = lua_absindex(L, stk);
    lua_pushvalue(L, stk);  // stk 应为 WORLD_PROTO_ID[] 表的键
    lua_gettable(L, proto_id);
    int id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    luaL_argcheck(L, id >= TYPE_MIN_ID, stk, "invalid type id");
    return id;
}

int EcsGetTid(lua_State* L, const char* name) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    // EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    lua_getiuservalue(L, ecs_ud, WORLD_PROTO_ID);

    lua_pushstring(L, name);
    lua_gettable(L, -2);
    int id = lua_tointeger(L, -1);
    lua_pop(L, 1);

    neko_assert(id >= TYPE_MIN_ID && "invalid type id");

    lua_pop(L, 2);  // # pop __NEKO_ECS_CORE | WORLD_PROTO_ID

    return id;
}

EntityData* EcsGetEnt(lua_State* L, EcsWorld* w, int eid) {
    neko_assert(eid < w->entity_cap && "eid is invalid");
    EntityData* e = &w->entity_buf[eid];
    neko_assert(e->components_count >= 0 && "entity is dead");
    return e;
}

EntityData* EcsGetEnt_i(lua_State* L, EcsWorld* w, int stk) {
    EntityData* e;
    int eid = luaL_checkinteger(L, stk);
    luaL_argcheck(L, eid < w->entity_cap, 2, "eid is invalid");
    e = &w->entity_buf[eid];
    luaL_argcheck(L, e->components_count >= 0, 2, "entity is dead");
    return e;
}

int EcsEntityGetCid(EntityData* e, int tid) {
    int i = e->components[tid];
    if (i >= ENTITY_MAX_COMPONENTS) return -1;
    return e->components_index[i];
}

void EcsEntityUpdateCid(EntityData* e, int tid, int cid) {
    int i = e->components[tid];
    assert(i < ENTITY_MAX_COMPONENTS);
    e->components_index[i] = cid;
}

void EcsWorldInit_i(EcsWorld* world) {
    neko_assert(world);

    world->entity_cap = 128;
    world->entity_free_id = 0;
    world->entity_dead_id = LINK_NIL;
    world->type_idx = 0;
    world->entity_buf = (EntityData*)mem_alloc(world->entity_cap * sizeof(world->entity_buf[0]));
    for (int i = 0; i < world->entity_cap - 1; i++) {  // 更新 entity_buf 的初始状态
        world->entity_buf[i].components_count = -1;
        world->entity_buf[i].next = i + 1;
    }
    world->entity_buf[world->entity_cap - 1].components_count = -1;
    world->entity_buf[world->entity_cap - 1].next = LINK_NIL;
}

void EcsWorldFini_i(EcsWorld* w) {
    int tid;
    ComponentPool* cp;
    tid = w->type_idx;  // 总数-1(索引)
    for (int i = 0; i <= tid; i++) {
        cp = &w->component_pool[i];
        mem_free(cp->buf);
    }
    mem_free(w->entity_buf);
}

int EcsRegister(lua_State* L, const_str name) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    lua_getiuservalue(L, ecs_ud, WORLD_PROTO_ID);

    // 检查组件是否已声明
    lua_pushstring(L, name);         // 复制组件key
    int type = lua_gettable(L, -2);  // 检查 WORLD_PROTO_ID
    luaL_argcheck(L, type == LUA_TNIL, 1, "duplicated component type");
    lua_pop(L, 1);  // # pop WORLD_PROTO_ID[key]

    // 新组件类型
    int tid = ++w->type_idx;
    luaL_argcheck(L, tid >= TYPE_MIN_ID && tid <= TYPE_MAX_ID, 1, "component type is too may");

    ComponentPool* cp = &w->component_pool[tid];
    cp->cap = 64;
    cp->free_idx = 0;
    cp->dirty_head = LINK_NIL;
    cp->dirty_tail = LINK_NIL;
    cp->dead_head = LINK_NIL;
    cp->dead_tail = LINK_NIL;
    cp->buf = (ComponentData*)mem_alloc(cp->cap * sizeof(cp->buf[0]));

    // 设置原型ID
    lua_pushstring(L, name);  // 复制组件key
    lua_pushinteger(L, tid);  // 组件tid
    lua_rawset(L, -3);        // WORLD_PROTO_ID[key] = tid
    lua_pop(L, 1);            // # pop WORLD_PROTO_ID

    lua_getiuservalue(L, ecs_ud, WORLD_COMPONENTS);
    lua_createtable(L, TYPE_COUNT, 0);
    lua_rawseti(L, -2, tid);  // WORLD_COMPONENTS[tid] = lua_createtable
    lua_pop(L, 1);            // # pop WORLD_COMPONENTS

    lua_pop(L, 1);  // # pop __NEKO_ECS_CORE

    return tid;
}

EntityData* EcsEntityNew(lua_State* L, const LuaRef& ref, lua_CFunction gc) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    EntityData* e = EcsEntityAlloc(w);
    int eid = e - w->entity_buf;
    lua_getiuservalue(L, ecs_ud, WORLD_COMPONENTS);
    int components = lua_gettop(L);
    // lua_getiuservalue(L, ecs_ud, WORLD_PROTO_ID);
    // int proto_id = components + 1;

    int tid = EcsGetTid(L, "CTag");

    if (ref.IsTable()) {

        ref.Push();  // {...}

        lua_getiuservalue(L, ecs_ud, WORLD_KEY_EID);
        lua_pushinteger(L, eid);
        lua_settable(L, -3);  // 组件内容表
        lua_getiuservalue(L, ecs_ud, WORLD_KEY_TID);
        lua_pushinteger(L, tid);
        lua_settable(L, -3);  // 组件内容表

        int cid = EcsComponentAlloc(w, e, tid);
        luaL_argcheck(L, cid >= 0, 2, "entity has duplicated component");

        lua_rawgeti(L, components, tid);  // WORLD_COMPONENTS[tid]
        lua_pushvalue(L, -2);             // 复制 {...}
        lua_rawseti(L, -2, cid);          // WORLD_COMPONENTS[tid][cid] = {...}

        lua_pop(L, 2);  // # pop WORLD_COMPONENTS[tid] | {...}

    } else if (ref.IsLightUserdata()) {

        lua_newtable(L);  // {...}

        lua_getiuservalue(L, ecs_ud, WORLD_KEY_EID);
        lua_pushinteger(L, eid);
        lua_settable(L, -3);  // 组件内容表
        lua_getiuservalue(L, ecs_ud, WORLD_KEY_TID);
        lua_pushinteger(L, tid);
        lua_settable(L, -3);  // 组件内容表

        lua_getiuservalue(L, ecs_ud, WORLD_KEY_UD);
        ref.Push();  // ud
        lua_settable(L, -3);

        if (NULL != gc) {
            lua_pushstring(L, "__gc");
            ref.Push();
            lua_pushcclosure(L, gc, 1);
            lua_settable(L, -3);
        }

        lua_pushvalue(L, -1);
        lua_setmetatable(L, -1);

        int cid = EcsComponentAlloc(w, e, tid);
        luaL_argcheck(L, cid >= 0, 2, "entity has duplicated component");

        lua_rawgeti(L, components, tid);  // WORLD_COMPONENTS[tid]
        lua_pushvalue(L, -2);             // 复制 {...}
        lua_rawseti(L, -2, cid);          // WORLD_COMPONENTS[tid][cid] = {...}

        lua_pop(L, 2);  // # pop WORLD_COMPONENTS[tid] | {...}
    } else {
        neko_assert(0 && "not impl");
    }

    lua_pop(L, 2);  // # pop WORLD_COMPONENTS | __NEKO_ECS_CORE

    return e;
}

void EcsEntityDel(lua_State* L, int eid) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);
    EntityData* e = EcsGetEnt(L, w, eid);
    EcsEntityDead(w, e);

    lua_pop(L, 1);
}

int EcsComponentSet(lua_State* L, EntityData* e, const char* name, const LuaRef& ref) {
    int tid = EcsGetTid(L, name);
    neko_assert(tid >= TYPE_MIN_ID);
    return EcsComponentSet(L, e, tid, ref);
}

int EcsComponentSet(lua_State* L, EntityData* e, int tid, const LuaRef& ref) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);
    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    int eid = e - w->entity_buf;
    lua_getiuservalue(L, ecs_ud, WORLD_COMPONENTS);
    int components = lua_gettop(L);

    int cid = -1;

    if (ref.IsTable()) {

        ref.Push();  // {...}

        lua_getiuservalue(L, ecs_ud, WORLD_KEY_EID);
        lua_pushinteger(L, eid);
        lua_settable(L, -3);  // 组件内容表
        lua_getiuservalue(L, ecs_ud, WORLD_KEY_TID);
        lua_pushinteger(L, tid);
        lua_settable(L, -3);  // 组件内容表

        cid = EcsComponentAlloc(w, e, tid);
        luaL_argcheck(L, cid >= 0, 2, "entity has duplicated component");

        lua_rawgeti(L, components, tid);  // WORLD_COMPONENTS[tid]
        lua_pushvalue(L, -2);             // 复制 {...}
        lua_rawseti(L, -2, cid);          // WORLD_COMPONENTS[tid][cid] = {...}

        lua_pop(L, 2);  // # pop WORLD_COMPONENTS[tid] | {...}
    } else {
        neko_assert(0 && "bug");
    }

    lua_pop(L, 2);  // # pop WORLD_COMPONENTS | __NEKO_ECS_CORE

    return cid;
}

LuaRef EcsComponentGet(lua_State* L, EntityData* e, const char* name) {
    int tid = EcsGetTid(L, name);
    return EcsComponentGet(L, e, tid);
}

LuaRef EcsComponentGet(lua_State* L, EntityData* e, int tid) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    lua_getiuservalue(L, ecs_ud, WORLD_COMPONENTS);
    int components = ecs_ud + 1;

    int cid = EcsEntityGetCid(e, tid);
    if (cid >= 0) {
        lua_rawgeti(L, components, tid);
        lua_rawgeti(L, -1, cid);
        lua_replace(L, -2);
    } else {
        lua_pushnil(L);
    }

    LuaRef ud = LuaRef::FromStack(L);

    lua_pop(L, 1);

    lua_pop(L, 2);

    return ud;
}

int EcsUpdate(lua_State* L) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);
    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    // 清除死亡实体
    EntityData* entity_buf = w->entity_buf;
    int next = w->entity_dead_id;
    while (next != LINK_NIL) {
        EntityData* e;
        e = &entity_buf[next];
        e->components_count = -1;
        next = e->next;
        EcsEntityFree(w, e);
    }
    w->entity_dead_id = LINK_NIL;  // 然后更新为 LINK_NIL

    // 清除死亡组件
    ComponentPool* pool = w->component_pool;
    lua_getiuservalue(L, ecs_ud, WORLD_COMPONENTS);
    for (int tid = 0; tid <= w->type_idx; tid++) {  // 遍寻世界所有组件
        ComponentPool* cp = &pool[tid];             // 组件池
        cp->dirty_head = LINK_NIL;
        cp->dirty_tail = LINK_NIL;
        cp->dead_head = LINK_NIL;
        cp->dead_tail = LINK_NIL;
        lua_rawgeti(L, -1, tid);  // push WORLD_COMPONENTS[tid]
        ComponentData* buf = cp->buf;
        int free_idx = cp->free_idx;  // 当前组件池第一个闲置位
        int w = 0, r = 0;
        for (r = 0; r < free_idx; r++) {
            ComponentData* c = &buf[r];
            c->dirty_next = LINK_NONE;
            if (c->dead_next == LINK_NONE) {  // 如果存活
                if (w != r) {
                    EntityData* e = &entity_buf[c->eid];
                    buf[w] = *c;
                    lua_rawgeti(L, -1, r);          // N = WORLD_COMPONENTS[tid][r]
                    lua_rawseti(L, -2, w);          // WORLD_COMPONENTS[tid][w] = N
                    EcsEntityUpdateCid(e, tid, w);  // 更新cid = w
                }
                w++;
            } else {  // 否则为标记的死组件
                EntityData* e = &entity_buf[c->eid];
                if (e->next == LINK_NONE) EcsComponentClear(e, tid);
            }
        }
        cp->free_idx = w;
        while (w < free_idx) {
            lua_pushnil(L);
            lua_rawseti(L, -2, w);  //  WORLD_COMPONENTS[tid][w] = nil 触发gc
            w++;
        }
        lua_pop(L, 1);  // pop WORLD_COMPONENTS[tid]
    }

    lua_pop(L, 2);  // # pop WORLD_COMPONENTS | NEKO_ECS_CORE

    return 0;
}

}  // namespace ecs
}  // namespace Neko