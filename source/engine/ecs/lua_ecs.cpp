#include "lua_ecs.hpp"

#include "base/common/logger.hpp"

namespace Neko {
namespace ecs {

using namespace luabind;

enum MatchMode {
    MATCH_ALL = 0,
    MATCH_DIRTY = 1,
    MATCH_DEAD = 2,
};

struct MatchCtx {
    EcsWorld* world;
    int i;
    int kn;
    int keys[ENTITY_MAX_COMPONENTS];
};

int EcsComponentHas(Entity* e, int tid) { return e->components[tid] < ENTITY_MAX_COMPONENTS; }

void EcsComponentClear(Entity* e, int tid) {
    int i = e->components[tid];
    if (i < ENTITY_MAX_COMPONENTS) {                 // 组件存在
        e->components[tid] = ENTITY_MAX_COMPONENTS;  // 移除组件
        e->components_index[i] = -1;                 // 清除索引
        --e->components_count;                       // 组件计数减少
    }
}

int EcsComponentAlloc(EcsWorld* world, Entity* e, int tid) {
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
        cp->buf = (Component*)mem_realloc(cp->buf, cp->cap * sizeof(cp->buf[0]));
    }

    // 获取新组件
    Component* c = &cp->buf[cp->free_idx++];
    c->eid = e - world->entity_buf;  // 记录组件附着的实体ID
    c->dirty_next = LINK_NONE;
    c->dead_next = LINK_NONE;
    int cid = c - cp->buf;  // 计算组件在组件池的索引

    // 将组件添加到实体中
    for (int i = 0; i < ENTITY_MAX_COMPONENTS; i++) {
        if (e->components_index[i] < 0) {  // 找到空位
            e->components[tid] = i;        // 编号为tid的组件在components_index中的位置为i
            e->components_index[i] = cid;  // components_index[i]存储该组件在组件池中的位置
            break;
        }
    }
    return cid;
}

void EcsComponentDead(EcsWorld* world, int tid, int cid) {
    ComponentPool* cp = &world->component_pool[tid];  // 获取索引为tid的组件标记数据
    Component* c = &cp->buf[cid];                     // 获取索引为cid的组件

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
    Component* c = &cp->buf[cid];                     // 获取索引为cid的组件

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

Entity* EcsEntityAlloc(EcsWorld* world) {
    Entity* e{};
    int eid = world->entity_free_id;
    if (eid < 0) {  // 如果 entity_free 是负值 表示当前没有可用的实体位
        int i = 0;
        int oldcap = world->entity_cap;  // 原实体容量
        int newcap = oldcap * 2;         // 现实体容量为两倍
        world->entity_cap = newcap;
        world->entity_buf = (Entity*)mem_realloc(world->entity_buf, newcap * sizeof(world->entity_buf[0]));
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
    return e;
}

void EcsEntityDead(EcsWorld* world, Entity* e) {
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

void EcsEntityFree(EcsWorld* world, Entity* e) {
    assert(e->components_count == -1);  //
    e->next = world->entity_free_id;
    world->entity_free_id = e - world->entity_buf;
}

int EcsGetTid_w(lua_State* L, int stk, int proto_id) {
    stk = lua_absindex(L, stk);
    lua_pushvalue(L, stk);  // stk 应为 WORLD_PROTO_ID[] 表的键
    lua_gettable(L, proto_id);
    int id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    luaL_argcheck(L, id >= TYPE_MIN_ID, stk, "invalid type");
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

    neko_assert(id >= TYPE_MIN_ID && "invalid type");

    lua_pop(L, 2);  // # pop __NEKO_ECS_CORE | WORLD_PROTO_ID

    return id;
}

Entity* EcsGetEnt(lua_State* L, EcsWorld* w, int eid) {
    neko_assert(eid < w->entity_cap && "eid is invalid");
    Entity* e = &w->entity_buf[eid];
    neko_assert(e->components_count >= 0 && "entity is dead");
    return e;
}

Entity* EcsGetEnt_i(lua_State* L, EcsWorld* w, int stk) {
    Entity* e;
    int eid = luaL_checkinteger(L, stk);
    luaL_argcheck(L, eid < w->entity_cap, 2, "eid is invalid");
    e = &w->entity_buf[eid];
    luaL_argcheck(L, e->components_count >= 0, 2, "entity is dead");
    return e;
}

int EcsEntityGetCid(Entity* e, int tid) {
    int i = e->components[tid];
    if (i >= ENTITY_MAX_COMPONENTS) return -1;
    return e->components_index[i];
}

void EcsEntityUpdateCid(Entity* e, int tid, int cid) {
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
    world->entity_buf = (Entity*)mem_alloc(world->entity_cap * sizeof(world->entity_buf[0]));
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
    cp->buf = (Component*)mem_alloc(cp->cap * sizeof(cp->buf[0]));

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

Entity* EcsEntityNew(lua_State* L, const LuaRef& ref, lua_CFunction gc) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ecs_ud, ECS_WORLD_METATABLE);

    Entity* e = EcsEntityAlloc(w);
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
    Entity* e = EcsGetEnt(L, w, eid);
    EcsEntityDead(w, e);

    lua_pop(L, 1);
}

int EcsComponentSet(lua_State* L, Entity* e, const char* name, const LuaRef& ref) {
    int tid = EcsGetTid(L, name);
    neko_assert(tid >= TYPE_MIN_ID);
    return EcsComponentSet(L, e, tid, ref);
}

int EcsComponentSet(lua_State* L, Entity* e, int tid, const LuaRef& ref) {
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

LuaRef EcsComponentGet(lua_State* L, Entity* e, const char* name) {
    int tid = EcsGetTid(L, name);
    return EcsComponentGet(L, e, tid);
}

LuaRef EcsComponentGet(lua_State* L, Entity* e, int tid) {

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

struct EcsLuaWrap {

    static int l_ecs_get_detail(lua_State* L) {
        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        lua_getiuservalue(L, 1, WORLD_PROTO_ID);
        lua_getiuservalue(L, 1, WORLD_PROTO_DEFINE);
        lua_getiuservalue(L, 1, WORLD_COMPONENTS);
        lua_getiuservalue(L, 1, WORLD_SYSTEMS);
        lua_getiuservalue(L, 1, WORLD_MATCH_CTX);
        lua_getiuservalue(L, 1, WORLD_KEY_EID);
        lua_getiuservalue(L, 1, WORLD_KEY_TID);
        lua_getiuservalue(L, 1, WORLD_KEY_UD);
        lua_getiuservalue(L, 1, WORLD_UPVAL_N);
        return WORLD_UPVAL_N;
    }

    static int l_ecs_end(lua_State* L) {
        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        EcsWorldFini_i(w);
        LOG_INFO("ecs_lua_gc");
        return 0;
    }

    static int l_ecs_register(lua_State* L) {

        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        const char* name = lua_tostring(L, 2);

        EcsRegister(L, name);

        // 设置原型
        lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_DEFINE);
        lua_pushstring(L, name);  // 复制组件key
        lua_pushvalue(L, 3);      // 复制原型
        lua_rawset(L, -3);        // WORLD_PROTO_DEFINE[key] = {...}
        lua_pop(L, 1);            // # pop WORLD_PROTO_DEFINE

        return 0;
    }

    static int l_ecs_new_entity(lua_State* L) {
        int components, proto_id;
        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        Entity* e = EcsEntityAlloc(w);
        int eid = e - w->entity_buf;
        lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
        components = lua_gettop(L);
        lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
        proto_id = components + 1;

        lua_pushnil(L);                // first key
        while (lua_next(L, 2) != 0) {  // new 的列表

            int tid = EcsGetTid_w(L, -2, proto_id);  // 通过WORLD_PROTO_ID[name]获取tid

            lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_EID);
            lua_pushinteger(L, eid);
            lua_settable(L, -3);  // 组件内容表
            lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_TID);
            lua_pushinteger(L, tid);
            lua_settable(L, -3);  // 组件内容表

            // {
            //     lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_UD);
            //     lua_pushlightuserdata(L, NULL);
            //     lua_settable(L, -3);  // 组件内容表
            // }

            int cid = EcsComponentAlloc(w, e, tid);
            luaL_argcheck(L, cid >= 0, 2, "entity has duplicated component");

            lua_rawgeti(L, components, tid);  // WORLD_COMPONENTS[tid]
            lua_pushvalue(L, -2);             // 复制 {...}
            lua_rawseti(L, -2, cid);          // WORLD_COMPONENTS[tid][cid] = {...}

            lua_pop(L, 2);  // # pop WORLD_COMPONENTS[tid] 和 {...}
        }

        lua_pushinteger(L, eid);
        return 1;
    }

    static int l_ecs_del_entity(lua_State* L) {
        struct EcsWorld* w = (struct EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        Entity* e = EcsGetEnt_i(L, w, 2);
        EcsEntityDead(w, e);
        return 0;
    }

    static int l_ecs_get_component(lua_State* L) {
        int i, top, proto_id, components;
        struct EcsWorld* w = (struct EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        Entity* e = EcsGetEnt_i(L, w, 2);
        top = lua_gettop(L);
        lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
        proto_id = top + 1;
        lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
        components = top + 2;
        for (i = 3; i <= top; i++) {
            int tid = EcsGetTid_w(L, i, proto_id);
            int cid = EcsEntityGetCid(e, tid);
            if (cid >= 0) {
                lua_rawgeti(L, components, tid);
                lua_rawgeti(L, -1, cid);
                lua_replace(L, -2);
            } else {
                lua_pushnil(L);
            }
        }
        return (top - 3 + 1);
    }

    static int l_ecs_add_component(lua_State* L) {
        int tid, cid;
        struct EcsWorld* w = (struct EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        Entity* e = EcsGetEnt_i(L, w, 2);
        lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
        tid = EcsGetTid_w(L, 3, lua_gettop(L));
        cid = EcsComponentAlloc(w, e, tid);
        lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
        lua_geti(L, -1, tid);
        lua_pushvalue(L, 4);
        lua_seti(L, -2, cid);
        lua_pop(L, 3);
        return 0;
    }

    static int l_ecs_remove_component(lua_State* L) {
        int i, proto_id;
        int tid, cid;
        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        Entity* e = EcsGetEnt_i(L, w, 2);
        lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
        proto_id = lua_gettop(L);
        for (i = 3; i <= (proto_id - 1); i++) {
            tid = EcsGetTid_w(L, i, proto_id);
            cid = EcsEntityGetCid(e, tid);
            EcsComponentDead(w, tid, cid);
        }
        lua_pop(L, 1);
        return 0;
    }

    static int l_ecs_touch_component(lua_State* L) {
        int eid, tid, cid;
        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);

        lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_EID);
        lua_gettable(L, 2);
        eid = luaL_checkinteger(L, -1);
        luaL_argcheck(L, eid > 0 && eid < w->entity_cap, 2, "invalid component");

        lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_TID);
        lua_gettable(L, 2);
        tid = luaL_checkinteger(L, -1);
        luaL_argcheck(L, tid >= TYPE_MIN_ID && tid <= TYPE_MAX_ID, 2, "invalid component");

        Entity* e = &w->entity_buf[eid];
        cid = EcsEntityGetCid(e, tid);
        EcsComponentDirty(w, tid, cid);
        return 0;
    }

    // MatchFunc::*(MatchCtx *mctx, nil)
    struct MatchFunc {

        static Entity* restrict_component(Entity* ebuf, Component* c, int* keys, int kn) {
            Entity* e = &ebuf[c->eid];
            for (int i = 1; i < kn; i++) {
                if (!EcsComponentHas(e, keys[i])) return NULL;
            }
            return e;
        }

        static void push_result(lua_State* L, int* keys, int kn, Entity* e) {
            int i;
            if (e != NULL) {  // match one
                int components;
                lua_getiuservalue(L, 1, 1);  // match_ctx uv 1
                lua_getiuservalue(L, -1, WORLD_COMPONENTS);
                lua_replace(L, -2);
                components = lua_gettop(L);
                for (i = 0; i < kn; i++) {
                    int tid = keys[i];
                    int cid = EcsEntityGetCid(e, tid);
                    lua_rawgeti(L, components, tid);
                    lua_rawgeti(L, -1, cid);
                    lua_replace(L, -2);
                }
            } else {
                for (i = 0; i < kn; i++) lua_pushnil(L);  // 终止迭代
            }
        }

        static int match_all(lua_State* L) {
            int* keys;
            Entity* e = NULL;
            ComponentPool* cp;
            MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, 1);
            EcsWorld* w = mctx->world;
            Entity* entity_buf = w->entity_buf;
            int kn = mctx->kn;
            keys = mctx->keys;
            cp = &w->component_pool[mctx->keys[0]];
            int mi = mctx->i;
            int free = cp->free_idx;
            while (mi < free) {
                Component* c = &cp->buf[mi++];
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
            Entity* e = NULL;
            ComponentPool* cp;
            MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, 1);
            EcsWorld* w = mctx->world;
            Entity* entity_buf = w->entity_buf;
            keys = mctx->keys;
            kn = mctx->kn;
            next = mctx->i;
            cp = &w->component_pool[keys[0]];
            while (next != LINK_NIL) {
                Component* c = &cp->buf[next];
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
            Entity* e = NULL;
            ComponentPool* cp;
            MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, 1);
            EcsWorld* w = mctx->world;
            Entity* entity_buf = w->entity_buf;
            keys = mctx->keys;
            kn = mctx->kn;
            next = mctx->i;
            cp = &w->component_pool[keys[0]];
            while (next != LINK_NIL) {
                Component* c = &cp->buf[next];
                next = c->dead_next;
                e = restrict_component(entity_buf, c, keys, kn);
                if (e != NULL) break;
            }
            mctx->i = next;
            push_result(L, keys, kn, e);
            return kn;
        }
    };

    static int l_ecs_match_component(lua_State* L) {

        size_t match_mode_name_sz;
        const char* match_mode_name = luaL_checklstring(L, ECS_WORLD + 1, &match_mode_name_sz);

        int top = lua_gettop(L);
        EcsWorld* world = (struct EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);

        lua_CFunction iter = NULL;
        MatchMode mode = MATCH_ALL;

        switch (match_mode_name_sz) {
            [[likely]] case 3:
                if (match_mode_name[0] == 'a' && match_mode_name[1] == 'l' && match_mode_name[2] == 'l') {
                    iter = MatchFunc::match_all;
                    mode = MATCH_ALL;
                }
                break;
            case 5:
                if (memcmp(match_mode_name, "dirty", 5) == 0) {
                    iter = MatchFunc::match_dirty;
                    mode = MATCH_DIRTY;
                }
                break;
            case 4:
                if (memcmp(match_mode_name, "dead", 4) == 0) {
                    iter = MatchFunc::match_dead;
                    mode = MATCH_DEAD;
                }
                break;
        }

        if (iter == NULL) return luaL_argerror(L, 2, "mode can only be[all,dirty,dead]");
        luaL_argcheck(L, top >= 3, 3, "lost the component name");
        luaL_argcheck(L, top < ENTITY_MAX_COMPONENTS, top, "too many component");

        lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);  // top + 1

        lua_pushcfunction(L, iter);                        // iter
        lua_getiuservalue(L, ECS_WORLD, WORLD_MATCH_CTX);  // iter + 1
        MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, -1);
        mctx->world = world;
        mctx->kn = 0;
        for (int i = 3; i <= top; i++) mctx->keys[mctx->kn++] = EcsGetTid_w(L, i, top + 1);
        switch (mode) {
            case MATCH_ALL:
                mctx->i = 0;  // 从0开始遍历
                break;
            case MATCH_DIRTY:
                mctx->i = world->component_pool[mctx->keys[0]].dirty_head;
                break;
            case MATCH_DEAD:
                mctx->i = world->component_pool[mctx->keys[0]].dead_head;
                break;
        }
        lua_pushinteger(L, 114514);  // iter + 2
        return 3;
    }

    static int l_ecs_update(lua_State* L) {

        EcsWorld* w = (EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);

        // 清除死亡实体
        Entity* entity_buf = w->entity_buf;
        int next = w->entity_dead_id;
        while (next != LINK_NIL) {
            Entity* e;
            e = &entity_buf[next];
            e->components_count = -1;
            next = e->next;
            EcsEntityFree(w, e);
        }

        // 清除死亡组件
        ComponentPool* pool = w->component_pool;
        lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
        for (int tid = 0; tid <= w->type_idx; tid++) {  // 遍寻世界所有组件
            ComponentPool* cp = &pool[tid];             // 组件池
            cp->dirty_head = LINK_NIL;
            cp->dirty_tail = LINK_NIL;
            cp->dead_head = LINK_NIL;
            cp->dead_tail = LINK_NIL;
            lua_rawgeti(L, -1, tid);  // push WORLD_COMPONENTS[tid]
            Component* buf = cp->buf;
            int free_idx = cp->free_idx;  // 当前组件池第一个闲置位
            int w = 0, r = 0;
            for (r = 0; r < free_idx; r++) {
                Component* c = &buf[r];
                c->dirty_next = LINK_NONE;
                if (c->dead_next == LINK_NONE) {  // 如果存活
                    if (w != r) {
                        Entity* e = &entity_buf[c->eid];
                        buf[w] = *c;
                        lua_rawgeti(L, -1, r);          // N = WORLD_COMPONENTS[tid][r]
                        lua_rawseti(L, -2, w);          // WORLD_COMPONENTS[tid][w] = N
                        EcsEntityUpdateCid(e, tid, w);  // 更新cid = w
                    }
                    w++;
                } else {  // 否则为标记的死组件
                    Entity* e = &entity_buf[c->eid];
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

    static int l_ecs_dump(lua_State* L) {
        int i;
        for (i = 1; i <= WORLD_COMPONENTS; i++) dump_upval(L, i);
        return 0;
    }
};

int l_ecs_create_world(lua_State* L) {

    MatchCtx* mctx;
    EcsWorld* world = (EcsWorld*)lua_newuserdatauv(L, sizeof(*world), WORLD_UPVAL_N);
    std::memset(world, 0, sizeof(*world));

    EcsWorldInit_i(world);

    if (luaL_getmetatable(L, ECS_WORLD_METATABLE) == LUA_TNIL) {
        luaL_Reg world_mt[] = {
                {"__index", NULL},
                {"__name", NULL},
                {"__gc", Wrap<EcsLuaWrap::l_ecs_end>},
                {"register", Wrap<EcsLuaWrap::l_ecs_register>},
                {"new", Wrap<EcsLuaWrap::l_ecs_new_entity>},
                {"del", Wrap<EcsLuaWrap::l_ecs_del_entity>},
                {"get", Wrap<EcsLuaWrap::l_ecs_get_component>},
                {"add", Wrap<EcsLuaWrap::l_ecs_add_component>},
                {"remove", Wrap<EcsLuaWrap::l_ecs_remove_component>},
                {"touch", Wrap<EcsLuaWrap::l_ecs_touch_component>},
                {"match", Wrap<EcsLuaWrap::l_ecs_match_component>},
                {"update", Wrap<EcsLuaWrap::l_ecs_update>},
                {"dump", Wrap<EcsLuaWrap::l_ecs_dump>},
                {"detail", Wrap<EcsLuaWrap::l_ecs_get_detail>},
                {NULL, NULL},
        };
        lua_pop(L, 1);  // # pop luaL_getmetatable=nil
        luaL_newlibtable(L, world_mt);
        luaL_setfuncs(L, world_mt, 0);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        lua_pushliteral(L, ECS_WORLD_METATABLE);
        lua_setfield(L, -2, "__name");
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, ECS_WORLD_METATABLE);
    }
    lua_setmetatable(L, -2);

    lua_createtable(L, 0, TYPE_COUNT);
    lua_setiuservalue(L, 1, WORLD_PROTO_ID);

    lua_createtable(L, 0, TYPE_COUNT);
    lua_setiuservalue(L, 1, WORLD_PROTO_DEFINE);

    lua_createtable(L, TYPE_MAX_ID, 0);
    lua_setiuservalue(L, 1, WORLD_COMPONENTS);

    lua_createtable(L, 0, 0);
    lua_setiuservalue(L, 1, WORLD_SYSTEMS);

    mctx = (MatchCtx*)lua_newuserdatauv(L, sizeof(*mctx), 1);
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);               // # match_ctx uv 1
    lua_setiuservalue(L, 1, WORLD_MATCH_CTX);  // # l_ecs_t uv 4

    lua_pushliteral(L, "__eid");
    lua_setiuservalue(L, 1, WORLD_KEY_EID);

    lua_pushliteral(L, "__tid");
    lua_setiuservalue(L, 1, WORLD_KEY_TID);

    lua_pushliteral(L, "__ud");
    lua_setiuservalue(L, 1, WORLD_KEY_UD);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, 1, WORLD_UPVAL_N);

    lua_pushvalue(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);

    return 1;
}

}  // namespace ecs
}  // namespace Neko