

#include "base/scripting/lua_wrapper.hpp"
#include "base/common/logger.hpp"
#include "engine/ecs/lua_ecs.hpp"

using namespace Neko;
using namespace Neko::ecs;

struct lua_system_userdata {
    String system_name;
};

int system_table_ref;

static int l_system_update(lua_State* L, EcsWorld* ecs, int* entities, int entity_count, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    int systb = system_table_ref;

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
        LOG_WARN("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);

    return 0;
}

static void l_system_add(lua_State* L, EcsWorld* ecs, int entity_id, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    // NEKO_DEBUG_LOG("l_system_add %s ent_id %d", ud->system_name.cstr(), entity_id);

    int systb = system_table_ref;

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
        LOG_WARN("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);
}

static void l_system_remove(lua_State* L, EcsWorld* ecs, int entity_id, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    // NEKO_DEBUG_LOG("l_system_remove %s ent_id %d", ud->system_name.cstr(), entity_id);

    int systb = system_table_ref;

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
        LOG_WARN("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);
}

static int __neko_ecs_lua_system(lua_State* L) {
    EcsWorld* w = (struct EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);

    int sys = u32_max;
    String system_name = luax_check_string(L, ECS_WORLD + 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, system_table_ref);
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

namespace Neko {
namespace ecs {

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
        EntityData* e = EcsEntityAlloc(w);
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
        EntityData* e = EcsGetEnt_i(L, w, 2);
        EcsEntityDead(w, e);
        return 0;
    }

    static int l_ecs_get_component(lua_State* L) {
        int i, top, proto_id, components;
        struct EcsWorld* w = (struct EcsWorld*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_METATABLE);
        EntityData* e = EcsGetEnt_i(L, w, 2);
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
        EntityData* e = EcsGetEnt_i(L, w, 2);
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
        EntityData* e = EcsGetEnt_i(L, w, 2);
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

        EntityData* e = &w->entity_buf[eid];
        cid = EcsEntityGetCid(e, tid);
        EcsComponentDirty(w, tid, cid);
        return 0;
    }

    // MatchFunc::*(MatchCtx *mctx, nil)
    struct MatchFunc {

        static EntityData* restrict_component(EntityData* ebuf, Component* c, int* keys, int kn) {
            EntityData* e = &ebuf[c->eid];
            for (int i = 1; i < kn; i++) {
                if (!EcsComponentHas(e, keys[i])) return NULL;
            }
            return e;
        }

        static void push_result(lua_State* L, int* keys, int kn, EntityData* e) {
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
            EntityData* e = NULL;
            ComponentPool* cp;
            MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, 1);
            EcsWorld* w = mctx->world;
            EntityData* entity_buf = w->entity_buf;
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
            EntityData* e = NULL;
            ComponentPool* cp;
            MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, 1);
            EcsWorld* w = mctx->world;
            EntityData* entity_buf = w->entity_buf;
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
            EntityData* e = NULL;
            ComponentPool* cp;
            MatchCtx* mctx = (MatchCtx*)lua_touserdata(L, 1);
            EcsWorld* w = mctx->world;
            EntityData* entity_buf = w->entity_buf;
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
        EntityData* entity_buf = w->entity_buf;
        int next = w->entity_dead_id;
        while (next != LINK_NIL) {
            EntityData* e;
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

int EcsCreateWorld(lua_State* L) {

    MatchCtx* mctx;
    EcsWorld* world = (EcsWorld*)lua_newuserdatauv(L, sizeof(*world), WORLD_UPVAL_N);
    std::memset(world, 0, sizeof(*world));
    int top = lua_gettop(L);

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
    lua_setiuservalue(L, top, WORLD_PROTO_ID);

    lua_createtable(L, 0, TYPE_COUNT);
    lua_setiuservalue(L, top, WORLD_PROTO_DEFINE);

    lua_createtable(L, TYPE_MAX_ID, 0);
    lua_setiuservalue(L, top, WORLD_COMPONENTS);

    lua_createtable(L, 0, 0);
    lua_setiuservalue(L, top, WORLD_SYSTEMS);

    mctx = (MatchCtx*)lua_newuserdatauv(L, sizeof(*mctx), 1);
    lua_pushvalue(L, top);
    lua_setiuservalue(L, -2, top);               // # match_ctx uv 1
    lua_setiuservalue(L, top, WORLD_MATCH_CTX);  // # l_ecs_t uv 4

    lua_pushliteral(L, "__eid");
    lua_setiuservalue(L, top, WORLD_KEY_EID);

    lua_pushliteral(L, "__tid");
    lua_setiuservalue(L, top, WORLD_KEY_TID);

    lua_pushliteral(L, "__ud");
    lua_setiuservalue(L, top, WORLD_KEY_UD);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, top, WORLD_UPVAL_N);

    lua_pushvalue(L, top);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);

    return 1;
}

}  // namespace ecs

}  // namespace Neko