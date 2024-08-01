
#include "neko_ecs.h"

#include "engine/neko_app.h"
#include "engine/neko_lua.h"
#include "engine/neko_lua_wrap.h"

/*=============================
// ECS
=============================*/

enum ECS_LUA_UPVALUES { NEKO_ECS_COMPONENTS_NAME = 1, NEKO_ECS_UPVAL_N };

#define TYPE_MIN_ID 1
#define TYPE_MAX_ID 255
#define TYPE_COUNT 256

#define WORLD_PROTO_ID 1
#define WORLD_PROTO_DEFINE 2
#define WORLD_COMPONENTS 3
#define WORLD_MATCH_CTX 4
#define WORLD_KEY_EID 5
#define WORLD_KEY_TID 6
#define WORLD_UPVAL_N 6

#define LINK_NIL (-1)
#define LINK_NONE (-2)

#define ENTITY_MAX_COMPONENTS (64)  // 最大组件数量

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
    struct neko_ecs_world_t* w;
    int i;
    int kn;
    int keys[ENTITY_MAX_COMPONENTS];
};

struct neko_ecs_world_t {
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

static inline int component_add(lua_State* L, struct neko_ecs_world_t* w, struct entity* e, int tid) {
    int cid, i;
    struct component* c;
    struct component_pool* cp;
    struct component_ptr* ptr;
    if (e->components[tid] < ENTITY_MAX_COMPONENTS) {
        return luaL_error(L, "Entity(%d) already exist component(%d)", e - w->entity_buf, tid);
    }
    if (e->cn >= ENTITY_MAX_COMPONENTS) {
        return luaL_error(L, "Entity(%d) add to many components", e - w->entity_buf);
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

static inline void component_dead(struct neko_ecs_world_t* w, int tid, int cid) {
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

static inline void component_dirty(struct neko_ecs_world_t* w, int tid, int cid) {
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

static inline struct entity* entity_alloc(struct neko_ecs_world_t* w) {
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

static inline void entity_dead(struct neko_ecs_world_t* w, struct entity* e) {
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

static inline void entity_free(struct neko_ecs_world_t* w, struct entity* e) {
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

static inline struct entity* get_entity(lua_State* L, struct neko_ecs_world_t* w, int stk) {
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

static int __neko_ecs_world_end(lua_State* L) {
    struct neko_ecs_world_t* w;
    int type, tid;
    struct component_pool* cp;
    w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    tid = w->type_idx;  // 总数-1(索引)
    for (int i = 0; i <= tid; i++) {
        cp = &w->component_pool[i];
        mem_free(cp->buf);
    }
    mem_free(w->entity_buf);
    return 0;
}

static int __neko_ecs_world_register(lua_State* L) {
    struct neko_ecs_world_t* w;
    int type, tid;
    struct component_pool* cp;
    w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_new_entity(lua_State* L) {
    int i, components, proto_id;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_del_entity(lua_State* L) {
    int i;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity* e = get_entity(L, w, 2);
    entity_dead(w, e);
    return 0;
}

static int __neko_ecs_world_get_component(lua_State* L) {
    int i, top, proto_id, components;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_add_component(lua_State* L) {
    int tid, cid;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_remove_component(lua_State* L) {
    int i, proto_id;
    int tid, cid;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_touch_component(lua_State* L) {
    int eid, tid, cid;
    struct entity* e;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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
    struct neko_ecs_world_t* w = mctx->w;
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
    struct neko_ecs_world_t* w = mctx->w;
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
    struct neko_ecs_world_t* w = mctx->w;
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

static int __neko_ecs_world_match_component(lua_State* L) {
    int i;
    size_t sz;
    struct neko_ecs_world_t* w;
    enum match_mode mode;
    int (*iter)(lua_State* L) = NULL;
    struct match_ctx* mctx = NULL;
    const char* m = luaL_checklstring(L, 2, &sz);
    int top = lua_gettop(L);
    w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_update(lua_State* L) {
    int next;
    int t, components;
    struct component_pool* pool;
    struct neko_ecs_world_t* w = (struct neko_ecs_world_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_dump(lua_State* L) {
    int i;
    for (i = 1; i <= WORLD_COMPONENTS; i++) dump_upval(L, i);
    return 0;
}

int neko_ecs_create_world(lua_State* L) {
    int i;
    struct neko_ecs_world_t* w;
    struct match_ctx* mctx;
    w = (struct neko_ecs_world_t*)lua_newuserdatauv(L, sizeof(*w), WORLD_UPVAL_N);
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
                {"__gc", __neko_ecs_world_end},
                {"register", __neko_ecs_world_register},
                {"new", __neko_ecs_world_new_entity},
                {"del", __neko_ecs_world_del_entity},
                {"get", __neko_ecs_world_get_component},
                {"add", __neko_ecs_world_add_component},
                {"remove", __neko_ecs_world_remove_component},
                {"touch", __neko_ecs_world_touch_component},
                {"match", __neko_ecs_world_match_component},
                {"update", __neko_ecs_world_update},
                {"dump", __neko_ecs_world_dump},
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

enum class TYPEID { f_int8, f_int16, f_int32, f_int64, f_uint8, f_uint16, f_uint32, f_uint64, f_bool, f_ptr, f_float, f_double, f_COUNT };

#define TYPE_ID_(type) ((int)TYPEID::f_##type)

static inline void set_int8(void* p, lua_Integer v) { *(int8_t*)p = (int8_t)v; }
static inline void set_int16(void* p, lua_Integer v) { *(int16_t*)p = (int16_t)v; }
static inline void set_int32(void* p, lua_Integer v) { *(int32_t*)p = (int32_t)v; }
static inline void set_int64(void* p, lua_Integer v) { *(int64_t*)p = (int64_t)v; }
static inline void set_uint8(void* p, lua_Integer v) { *(int8_t*)p = (uint8_t)v; }
static inline void set_uint16(void* p, lua_Integer v) { *(int16_t*)p = (uint16_t)v; }
static inline void set_uint32(void* p, lua_Integer v) { *(int32_t*)p = (uint32_t)v; }
static inline void set_uint64(void* p, lua_Integer v) { *(int64_t*)p = (uint64_t)v; }
static inline void set_float(void* p, lua_Number v) { *(float*)p = (float)v; }
static inline void set_bool(void* p, int v) { *(int8_t*)p = v; }
static inline void set_ptr(void* p, void* v) { *(void**)p = v; }
static inline void set_double(void* p, lua_Number v) { *(float*)p = (double)v; }
static inline int8_t get_int8(void* p) { return *(int8_t*)p; }
static inline int16_t get_int16(void* p) { return *(int16_t*)p; }
static inline int32_t get_int32(void* p) { return *(int32_t*)p; }
static inline int64_t get_int64(void* p) { return *(int64_t*)p; }
static inline uint8_t get_uint8(void* p) { return *(uint8_t*)p; }
static inline uint16_t get_uint16(void* p) { return *(uint16_t*)p; }
static inline uint32_t get_uint32(void* p) { return *(uint32_t*)p; }
static inline uint64_t get_uint64(void* p) { return *(uint64_t*)p; }
static inline void* get_ptr(void* p) { return *(void**)p; }
static inline int get_bool(void* p) { return *(int8_t*)p; }
static inline float get_float(void* p) { return *(float*)p; }
static inline double get_double(void* p) { return *(double*)p; }

static inline int get_stride(int type) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            return sizeof(int8_t);
        case TYPEID::f_int16:
            return sizeof(int16_t);
        case TYPEID::f_int32:
            return sizeof(int32_t);
        case TYPEID::f_int64:
            return sizeof(int64_t);
        case TYPEID::f_uint8:
            return sizeof(uint8_t);
        case TYPEID::f_uint16:
            return sizeof(uint16_t);
        case TYPEID::f_uint32:
            return sizeof(uint32_t);
        case TYPEID::f_uint64:
            return sizeof(uint64_t);
        case TYPEID::f_ptr:
            return sizeof(void*);
        case TYPEID::f_bool:
            return sizeof(bool);
        case TYPEID::f_float:
            return sizeof(float);
        case TYPEID::f_double:
            return sizeof(double);
        default:
            return 0;
    }
}

static int setter(lua_State* L, void* p, int type, int offset) {
    p = (char*)p + offset * get_stride(type);
    switch (type) {
        case TYPE_ID_(int8):
            set_int8(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int16):
            set_int16(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int32):
            set_int32(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int64):
            set_int64(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint8):
            set_uint8(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint16):
            set_uint16(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint32):
            set_uint32(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint64):
            set_uint64(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(bool):
            set_bool(p, lua_toboolean(L, 2));
            break;
        case TYPE_ID_(ptr):
            set_ptr(p, lua_touserdata(L, 2));
            break;
        case TYPE_ID_(float):
            set_float(p, luaL_checknumber(L, 2));
            break;
        case TYPE_ID_(double):
            set_double(p, luaL_checknumber(L, 2));
            break;
    }
    return 0;
}

static inline int getter(lua_State* L, void* p, int type, int offset) {
    p = (char*)p + offset * get_stride(type);
    switch (type) {
        case TYPE_ID_(int8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPE_ID_(int16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPE_ID_(int32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPE_ID_(int64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPE_ID_(uint8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPE_ID_(uint16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPE_ID_(uint32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPE_ID_(uint64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPE_ID_(bool):
            lua_pushboolean(L, get_bool(p));
            break;
        case TYPE_ID_(ptr):
            lua_pushlightuserdata(L, get_ptr(p));
            break;
        case TYPE_ID_(float):
            lua_pushnumber(L, get_float(p));
            break;
        case TYPE_ID_(double):
            lua_pushnumber(L, get_double(p));
            break;
    }
    return 1;
}

#define CSB_FUNC(TYPE, OFF)                                                                                      \
    static int get_##TYPE##_##OFF(lua_State* L) { return getter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), OFF); } \
    static int set_##TYPE##_##OFF(lua_State* L) { return setter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), OFF); }

#define CSB_OFFSET(TYPE)                                                                                                                            \
    static int get_##TYPE##_offset(lua_State* L) { return getter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), lua_tointeger(L, lua_upvalueindex(1))); } \
    static int set_##TYPE##_offset(lua_State* L) { return setter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), lua_tointeger(L, lua_upvalueindex(1))); }

#define CSB(opt, TYPE)                                          \
    static int opt##ter_func_##TYPE(lua_State* L, int offset) { \
        switch (offset) {                                       \
            case 0:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_0);         \
                break;                                          \
            case 1:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_1);         \
                break;                                          \
            case 2:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_2);         \
                break;                                          \
            case 3:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_3);         \
                break;                                          \
            case 4:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_4);         \
                break;                                          \
            case 5:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_5);         \
                break;                                          \
            case 6:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_6);         \
                break;                                          \
            case 7:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_7);         \
                break;                                          \
            default:                                            \
                lua_pushinteger(L, offset);                     \
                lua_pushcclosure(L, opt##_##TYPE##_offset, 1);  \
                break;                                          \
        }                                                       \
        return 1;                                               \
    }

#define XX(TYPE)      \
    CSB_FUNC(TYPE, 0) \
    CSB_FUNC(TYPE, 1) \
    CSB_FUNC(TYPE, 2) \
    CSB_FUNC(TYPE, 3) \
    CSB_FUNC(TYPE, 4) \
    CSB_FUNC(TYPE, 5) \
    CSB_FUNC(TYPE, 6) \
    CSB_FUNC(TYPE, 7) \
    CSB_OFFSET(TYPE)  \
    CSB(get, TYPE)    \
    CSB(set, TYPE)

XX(float)
XX(double)
XX(int8)
XX(int16)
XX(int32)
XX(int64)
XX(uint8)
XX(uint16)
XX(uint32)
XX(uint64)
XX(bool)
XX(ptr)

#undef XX

static inline int get_value(lua_State* L, int type, int offset) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            getter_func_int8(L, offset);
            break;
        case TYPEID::f_int16:
            getter_func_int16(L, offset);
            break;
        case TYPEID::f_int32:
            getter_func_int32(L, offset);
            break;
        case TYPEID::f_int64:
            getter_func_int64(L, offset);
            break;
        case TYPEID::f_uint8:
            getter_func_uint8(L, offset);
            break;
        case TYPEID::f_uint16:
            getter_func_uint16(L, offset);
            break;
        case TYPEID::f_uint32:
            getter_func_uint32(L, offset);
            break;
        case TYPEID::f_uint64:
            getter_func_uint64(L, offset);
            break;
        case TYPEID::f_bool:
            getter_func_bool(L, offset);
            break;
        case TYPEID::f_ptr:
            getter_func_ptr(L, offset);
            break;
        case TYPEID::f_float:
            getter_func_float(L, offset);
            break;
        case TYPEID::f_double:
            getter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int getter_direct(lua_State* L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return get_value(L, type, offset);
}

static inline int set_value(lua_State* L, int type, int offset) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            setter_func_int8(L, offset);
            break;
        case TYPEID::f_int16:
            setter_func_int16(L, offset);
            break;
        case TYPEID::f_int32:
            setter_func_int32(L, offset);
            break;
        case TYPEID::f_int64:
            setter_func_int64(L, offset);
            break;
        case TYPEID::f_uint8:
            setter_func_uint8(L, offset);
            break;
        case TYPEID::f_uint16:
            setter_func_uint16(L, offset);
            break;
        case TYPEID::f_uint32:
            setter_func_uint32(L, offset);
            break;
        case TYPEID::f_uint64:
            setter_func_uint64(L, offset);
            break;
        case TYPEID::f_bool:
            setter_func_bool(L, offset);
            break;
        case TYPEID::f_ptr:
            setter_func_ptr(L, offset);
            break;
        case TYPEID::f_float:
            setter_func_float(L, offset);
            break;
        case TYPEID::f_double:
            setter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int setter_direct(lua_State* L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return set_value(L, type, offset);
}

#define LUATYPEID(type, typen)                 \
    lua_pushinteger(L, (int)TYPEID::f_##type); \
    lua_setfield(L, -2, #typen);

struct address_path {
    uint8_t type;
    uint8_t offset[1];
};

static const uint8_t* get_offset(const uint8_t* offset, size_t sz, int* output) {
    if (sz == 0) return NULL;
    if (offset[0] < 128) {
        *output = offset[0];
        return offset + 1;
    }
    int t = offset[0] & 0x7f;
    size_t i;
    int shift = 7;
    for (i = 1; i < sz; i++) {
        if (offset[i] < 128) {
            t |= offset[i] << shift;
            *output = t;
            return offset + i + 1;
        } else {
            t |= (offset[i] & 0x7f) << shift;
            shift += 7;
        }
    }
    return NULL;
}

static void* address_ptr(lua_State* L, int* type, int* offset) {
    size_t sz;
    const uint8_t* buf = (const uint8_t*)lua_tolstring(L, lua_upvalueindex(1), &sz);
    if (sz == 0 || buf[0] >= (int)TYPEID::f_COUNT) luaL_error(L, "Invalid type");
    void** p = (void**)lua_touserdata(L, 1);
    const uint8_t* endptr = buf + sz;
    sz--;
    const uint8_t* ptr = &buf[1];
    for (;;) {
        int off = 0;
        ptr = get_offset(ptr, sz, &off);
        if (ptr == NULL) luaL_error(L, "Invalid offset");
        sz = endptr - ptr;
        if (sz == 0) {
            *type = buf[0];
            *offset = off;
            return p;
        } else {
            p += off;
            if (*p == NULL) return NULL;
            p = (void**)*p;
        }
    }
}

static int get_indirect(lua_State* L) {
    int type;
    int offset;
    void* p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return getter(L, p, type, offset);
}

static int set_indirect(lua_State* L) {
    int type;
    int offset;
    void* p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return setter(L, p, type, offset);
}

static void address(lua_State* L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) luaL_error(L, "Invalid type %d", type);
    int top = lua_gettop(L);
    if (top <= 2) {
        luaL_error(L, "Need two or more offsets");
    }
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addchar(&b, type);
    int i;
    for (i = 2; i <= top; i++) {
        unsigned int offset = (unsigned int)luaL_checkinteger(L, i);
        if (i != top) {
            if (offset % sizeof(void*) != 0) luaL_error(L, "%d is not align to pointer", offset);
            offset /= sizeof(void*);
        } else {
            int stride = get_stride(type);
            if (offset % stride != 0) luaL_error(L, "%d is not align to %d", offset, stride);
            offset /= stride;
        }

        if (offset < 128) {
            luaL_addchar(&b, offset);
        } else {
            while (offset >= 128) {
                luaL_addchar(&b, (char)(0x80 | (offset & 0x7f)));
                offset >>= 7;
            }
            luaL_addchar(&b, offset);
        }
    }
    luaL_pushresult(&b);
}

static int open_csb(lua_State* L) {

    lua_newtable(L);
    lua_pushcclosure(
            L,
            [](lua_State* L) {
                int top = lua_gettop(L);
                if (top <= 2) {
                    getter_direct(L);
                    setter_direct(L);
                    return 2;
                } else {
                    address(L);
                    int cmd = lua_gettop(L);
                    lua_pushvalue(L, cmd);
                    lua_pushcclosure(L, get_indirect, 1);
                    lua_pushvalue(L, cmd);
                    lua_pushcclosure(L, set_indirect, 1);
                    return 2;
                }
            },
            0);
    lua_setfield(L, -2, "cstruct");

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                if (!lua_isuserdata(L, 1)) return luaL_error(L, "Need userdata at 1");
                char* p = (char*)lua_touserdata(L, 1);
                size_t off = luaL_checkinteger(L, 2);
                lua_pushlightuserdata(L, (void*)(p + off));
                return 1;
            },
            0);
    lua_setfield(L, -2, "offset");

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                lua_newtable(L);
                LUATYPEID(int8, int8_t);
                LUATYPEID(int16, int16_t);
                LUATYPEID(int32, int32_t);
                LUATYPEID(int64, int64_t);
                LUATYPEID(uint8, uint8_t);
                LUATYPEID(uint16, uint16_t);
                LUATYPEID(uint32, uint32_t);
                LUATYPEID(uint64, uint64_t);
                LUATYPEID(bool, bool);
                LUATYPEID(ptr, ptr);
                LUATYPEID(float, float);
                LUATYPEID(double, double);
                return 1;
            },
            0);
    lua_setfield(L, -2, "typeid");

    return 1;
}

static int luastruct_newudata(lua_State* L) {
    size_t sz = luaL_checkinteger(L, 1);
    lua_newuserdatauv(L, sz, 0);
    return 1;
}

static int open_embed_luastruct_test(lua_State* L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"udata", luastruct_newudata},
            {"NULL", NULL},
            {NULL, NULL},
    };
    luaL_newlib(L, l);
    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");
    return 1;
}

static const_str csb_lua = R"lua(
local CStructBridge = function()
    local w = Core.ecs_f()
    local core = w:csb_core()
    local M = {}
    local function parse_struct(code)
        local nest = {}
        local nest_n = 0
        code = code:gsub("(%b{})", function(v)
            nest_n = nest_n + 1
            nest[nest_n] = v
            return "{" .. nest_n .. "} "
        end)
        local names = {}
        local lines = {}
        local line_n = 0
        for line in code:gmatch "%s*(.-)%s*;" do
            line_n = line_n + 1
            line = line:gsub("%s+", " ")
            line = line:gsub(" ?%*%s*", " *")
            local prefix, array = line:match "^(.-)%s*(%b[])$"
            if array then
                array = math.tointeger(array:match "%[(%d+)%]") or 0
                line = prefix
            end
            local typestr, pointer, name = line:match "^(.-) (%**)([_%w]+)$"
            assert(typestr, line)
            local type_prefix, subtype = typestr:match "^([%w_]+)%s+(.+)"
            if type_prefix == "struct" or type_prefix == "union" then
                typestr = type_prefix
                local nesttypeid = subtype:match "^{(%d+)}$"
                if nesttypeid then
                    local nestcontent = assert(nest[tonumber(nesttypeid)]):match "^{(.*)}$"
                    subtype = parse_struct(nestcontent)
                    subtype.type = type_prefix
                end
            end
            if pointer == "" then
                pointer = nil
            end
            local t = {
                array = array,
                type = typestr,
                subtype = subtype,
                pointer = pointer,
                name = name
            }
            assert(names[name] == nil, name)
            names[name] = true
            lines[line_n] = t
        end

        return lines
    end

    local function parse(what, code, types)
        for typename, content in code:gmatch(what .. "%s+([_%w]+)%s*(%b{})%s*;") do
            assert(types[typename] == nil)
            local s = parse_struct(content:match "^{%s*(.-)%s*}$")
            s.type = what
            s.name = what .. " " .. typename
            types[s.name] = s
        end
    end

    local buildin_types = (function(map)
        local r = {}
        for k, v in pairs(map) do
            if type(k) == "number" then
                r[v] = true
            else
                r[k] = v
            end
        end
        return r
    end) {
        int = "int32_t",
        short = "int16_t",
        char = "int8_t",
        ["unsigned char"] = "uint8_t",
        ["unsigned short"] = "uint16_t",
        ["unsigned int"] = "uint32_t",
        "float",
        "double",
        "void",
        "bool",
        -- ["b"]= "bool",
        "int8_t",
        "int16_t",
        "int32_t",
        "int64_t",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        ["i8"] = "int8_t",
        ["i16"] = "int16_t",
        ["i32"] = "int32_t",
        ["i64"] = "int64_t",
        ["u8"] = "uint8_t",
        ["u16"] = "uint16_t",
        ["u32"] = "uint32_t",
        ["u64"] = "uint64_t"
    }

    local buildin_size = {
        int8_t = 1,
        int16_t = 2,
        int32_t = 4,
        int64_t = 8,
        uint8_t = 1,
        uint16_t = 2,
        uint32_t = 4,
        uint64_t = 8,
        float = 4,
        double = 8,
        ptr = 8,
        bool = 1
    }

    local buildin_id = core.typeid()

    for k, v in pairs(buildin_types) do
        if v ~= true then
            buildin_size[k] = buildin_size[v]
            buildin_id[k] = buildin_id[v]
        end
    end

    local function check_types(types)
        for k, t in pairs(types) do
            for idx, f in ipairs(t) do
                local typename = f.type
                if typename == "struct" or typename == "union" then
                    if type(f.subtype) == "string" then
                        local fullname = typename .. " " .. f.subtype
                        local subtype = types[fullname]
                        if not subtype then
                            error("Unknown " .. fullname)
                        end
                        assert(subtype.type == typename)
                        f.subtype = subtype
                    end
                else
                    if not buildin_types[typename] then
                        error("Unknown " .. typename)
                    end
                end
                if f.array == 0 and t[idx + 1] then
                    error("Array " .. f.name .. "[] must be the last field")
                end
            end
        end
    end

    local function calc_offset(types)
        local solve

        local function calc_align(t)
            local align = 0
            for _, f in ipairs(t) do
                if f.pointer then
                    f.size = buildin_size.ptr
                    f.align = f.size
                elseif f.subtype then
                    local subtype = solve(f.subtype)
                    f.size = subtype.size
                    f.align = subtype.align
                    if subtype.align > align then
                        align = subtype.align
                    end
                else
                    f.size = assert(buildin_size[f.type])
                    f.align = f.size
                    if f.align > align then
                        align = f.align
                    end
                end
                if f.array then
                    f.size = f.size * f.array
                end
            end
            return align
        end

        local function solve_struct(t)
            t.align = calc_align(t)
            local size = 0
            for _, f in ipairs(t) do
                if size % f.align ~= 0 then
                    size = (size // f.align + 1) * f.align
                end
                f.offset = size
                size = size + f.size
            end
            if size % t.align ~= 0 then
                size = (size // t.align + 1) * t.align
            end
            t.size = size
        end
        local function solve_union(t)
            t.align = align(t)
            local size = 0
            for _, f in ipairs(t) do
                f.offset = 0
                if f.size > size then
                    size = f.size
                end
            end
            t.size = size
        end
        do -- 解决局部函数
            local unsolved = {}
            local solved = {}
            function solve(t)
                local fullname = t.name
                if fullname then
                    if solved[fullname] then
                        return solved[fullname]
                    end
                    assert(not unsolved[fullname])
                    unsolved[fullname] = true
                end

                if t.type == "struct" then
                    solve_struct(t)
                else
                    solve_union(t)
                end

                if fullname then
                    solved[fullname] = t
                    unsolved[fullname] = nil
                end
                return t
            end
        end

        for k, t in pairs(types) do
            solve(t)
        end

        local function solve_pointer_size(t)
            for _, f in ipairs(t) do
                if f.pointer then
                    assert(f.pointer == "*")
                    if f.subtype then
                        f.pointer_size = f.subtype.size
                    else
                        f.pointer_size = buildin_size[f.type]
                    end
                end
            end
        end

        for k, t in pairs(types) do
            solve_pointer_size(t)
        end
    end

    local function keys_lookup(t)
        local keys = {}
        for _, f in ipairs(t) do
            keys[f.name] = f
        end
        t.keys = keys
        return keys
    end

    local function find_key(t, key)
        local keys = t.keys or keys_lookup(t)
        return assert(keys[key], key)
    end

    local function gen_check(types, k)
        local keys = {}
        local n = 0
        for name in k:gmatch "[^.]*" do
            n = n + 1
            keys[n] = name
        end
        local t = types[keys[1]]
        if t == nil then
            error(keys[1] .. " undefined")
        end
        local offset = {}
        local last_offset = 0
        local offset_n = 1
        local i = 2
        local typename
        while i <= n do
            local name = keys[i]
            local array_name, array_index = name:match "(.+)%[(%d+)]$"
            name = array_name or name

            local f = find_key(t, name)
            offset[offset_n] = last_offset + f.offset
            if f.pointer then
                assert(f.pointer == "*") -- todo: support "**"
                offset_n = offset_n + 1
                last_offset = 0
                typename = "ptr"
            elseif f.subtype then
                last_offset = last_offset + f.offset
                t = f.subtype
                assert(i ~= n)
            else
                assert(i == n)
                typename = f.type
            end

            if array_index then
                local index = tonumber(array_index)
                if f.pointer then
                    offset[offset_n] = index * f.pointer_size
                    offset_n = offset_n + 1
                else
                    last_offset = last_offset + index * f.size
                    offset[offset_n] = last_offset
                end
            end

            i = i + 1
        end
        local getter, setter = core.cstruct(buildin_id[typename], table.unpack(offset))
        return {getter, setter}
    end

    local function check(types)
        local function cache_check(self, k)
            local v = gen_check(types, k)
            self[k] = v
            return v
        end
        return setmetatable({}, {
            __index = cache_check
        })
    end

    local methods = {};
    methods.__index = methods

    function methods:dump()
        for _, s in pairs(self._types) do
            print(s.name, "size", s.size, "align", s.align)
            for _, f in ipairs(s) do
                local array = ""
                if f.array then
                    array = "[" .. f.array .. "]"
                end
                local typename = f.type
                if f.subtype then
                    typename = f.subtype.name or ("nest " .. f.subtype.type)
                end
                print(string.format("\t%3d : %s %s%s%s", f.offset, typename, (f.pointer or ""), f.name, array))
            end
        end
    end

    function methods:size(name)
        local t = assert(self._types[name])
        return t.size
    end

    function methods:getter(name)
        return self._check[name][1]
    end

    function methods:setter(name)
        return self._check[name][2]
    end

    function M.struct(code)
        local types = {}
        parse("struct", code, types)
        parse("union", code, types)
        check_types(types)
        calc_offset(types)

        local obj = {
            _types = types,
            _check = check(types)
        }

        return setmetatable(obj, methods)
    end

    M.s = {}

    M.s["CGameObject"] = M.struct([[
    struct CGameObject {
        int id;
        bool active;
        bool visible;
        bool selected;
    };
    ]])

    -- M.sz = M.s:size "struct CGameObject"

    M.CGameObject_get_id = M.s["CGameObject"]:getter "struct CGameObject.id"
    M.CGameObject_get_active = M.s["CGameObject"]:getter "struct CGameObject.active"
    M.CGameObject_get_visible = M.s["CGameObject"]:getter "struct CGameObject.visible"
    M.CGameObject_get_selected = M.s["CGameObject"]:getter "struct CGameObject.selected"

    M.CGameObject_set_id = M.s["CGameObject"]:setter "struct CGameObject.id"
    M.CGameObject_set_active = M.s["CGameObject"]:setter "struct CGameObject.active"
    M.CGameObject_set_visible = M.s["CGameObject"]:setter "struct CGameObject.visible"
    M.CGameObject_set_selected = M.s["CGameObject"]:setter "struct CGameObject.selected"

    -- M.test_obj = luastruct_test.udata(M.sz)

    M.new_obj = function(id, active, visible)
        local obj = luastruct_test.udata(M.s["CGameObject"]:size "struct CGameObject")
        M.CGameObject_set_id(obj, id)
        M.CGameObject_set_active(obj, active)
        M.CGameObject_set_visible(obj, visible)
        M.CGameObject_set_selected(obj, false)
        return obj
    end

    -- M.CGameObject_set_visible(M.test_obj, false)
    -- print(M.CGameObject_get_visible(M.test_obj))
    -- M.s:dump()

    M.generate_array_type = function(name, base_type, n)
        local code = "struct " .. name .. "{\n"
        for i = 1, n do
            code = code .. base_type .. " v" .. i .. ";\n"
        end
        code = code .. "};"
        M.s[name] = M.struct(code)
        return M.s[name]
    end
    M.getter = function(struct_name, field)
        return M.s[struct_name]:getter("struct " .. struct_name .. "." .. field)
    end
    M.setter = function(struct_name, field)
        return M.s[struct_name]:setter("struct " .. struct_name .. "." .. field)
    end
    M.size = function(struct_name)
        return M.s[struct_name]:size("struct " .. struct_name)
    end
    return M
end
return CStructBridge()
)lua";

int neko_ecs_lua_csb(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    std::string contents = csb_lua;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), "csb.lua") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load csb");
    }
    // lua_getglobal(L, "CStructBridge");
    // lua_pushvalue(L, 1);
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        const char* errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "csb error: %s\n", errorMsg);
        // lua_pop(L, 1);
        neko_panic("failed to run csb");
    }
    NEKO_INFO("loaded csb");
    return 1;
}

#define NEKO_ECS_IMPLEMENTATION

#ifdef NEKO_ECS_IMPLEMENTATION

#define ECS_MAX_COMPONENTS 32
#define ECS_MAX_SYSTEMS 32

#define ECS_MALLOC(size, ctx) (mem_alloc(size))
#define ECS_REALLOC(ptr, size, ctx) (mem_realloc(ptr, size))
#define ECS_FREE(ptr, ctx) (mem_free(ptr))

#if ECS_MAX_COMPONENTS <= 32
typedef uint32_t ecs_bitset_t;
#elif ECS_MAX_COMPONENTS <= 64
typedef uint64_t ecs_bitset_t;
#else
#define ECS_BITSET_WIDTH 64
#define ECS_BITSET_SIZE (((ECS_MAX_COMPONENTS - 1) / ECS_BITSET_WIDTH) + 1)

typedef struct {
    uint64_t array[ECS_BITSET_SIZE];
} ecs_bitset_t;
#endif  // ECS_MAX_COMPONENTS

// 打包数组实现的数据结构 提供用于添加删除和访问实体 ID 的 O(1) 函数
typedef struct {
    size_t capacity;
    size_t size;
    size_t* sparse;
    ecs_id_t* dense;
} ecs_sparse_set_t;

// 为池 ID 提供 O(1) 操作的 ID 池的数据结构
typedef struct {
    size_t capacity;
    size_t size;
    ecs_id_t* array;
} ecs_stack_t;

typedef struct {
    size_t capacity;
    size_t count;
    size_t size;
    void* data;
} ecs_array_t;

typedef struct {
    ecs_bitset_t comp_bits;
    bool ready;
} ecs_entity_t;

typedef struct {
    ecs_constructor_fn constructor;
    ecs_destructor_fn destructor;
} ecs_comp_t;

typedef struct {
    bool active;
    ecs_sparse_set_t entity_ids;
    ecs_system_fn system_cb;
    ecs_added_fn add_cb;
    ecs_removed_fn remove_cb;
    ecs_bitset_t require_bits;
    ecs_bitset_t exclude_bits;
    void* udata;
} ecs_sys_t;

struct ecs_s {
    lua_State* L;
    int system_table_ref;
    ecs_stack_t entity_pool;
    ecs_stack_t destroy_queue;
    ecs_stack_t remove_queue;
    ecs_entity_t* entities;
    size_t entity_count;
    ecs_comp_t comps[ECS_MAX_COMPONENTS];
    ecs_array_t comp_arrays[ECS_MAX_COMPONENTS];
    size_t comp_count;
    ecs_sys_t systems[ECS_MAX_SYSTEMS];
    size_t system_count;
    void* mem_ctx;
};

void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size);

// 用于刷新已损坏实体和已删除组件的内部函数
static void ecs_flush_destroyed(ecs_t* ecs);
static void ecs_flush_removed(ecs_t* ecs);

// 内部位设置功能
static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on);
static inline bool ecs_bitset_is_zero(ecs_bitset_t* set);
static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit);
static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline ecs_bitset_t ecs_bitset_or(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline ecs_bitset_t ecs_bitset_not(ecs_bitset_t* set);
static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline bool ecs_bitset_true(ecs_bitset_t* set);

// 内部稀疏集函数
static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity);
static void ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set);
static bool ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id);
static size_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id);
static bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id);

// 内部系统实体添加/删除功能
static bool ecs_entity_system_test(ecs_bitset_t* require_bits, ecs_bitset_t* exclude_bits, ecs_bitset_t* entity_bits);

// 内部ID池功能
static void ecs_stack_init(ecs_t* ecs, ecs_stack_t* pool, int capacity);
static void ecs_stack_free(ecs_t* ecs, ecs_stack_t* pool);
static void ecs_stack_push(ecs_t* ecs, ecs_stack_t* pool, ecs_id_t id);
static ecs_id_t ecs_stack_pop(ecs_stack_t* pool);
static int ecs_stack_size(ecs_stack_t* pool);

// 内部数组函数
static void ecs_array_init(ecs_t* ecs, ecs_array_t* array, size_t size, size_t capacity);
static void ecs_array_free(ecs_t* ecs, ecs_array_t* array);
static void ecs_array_resize(ecs_t* ecs, ecs_array_t* array, size_t capacity);

// 测试用函数 发布版应当屏蔽
static bool ecs_is_not_null(void* ptr) { return NULL != ptr; }
static bool ecs_is_valid_component_id(ecs_id_t id) { return id < ECS_MAX_COMPONENTS; }
static bool ecs_is_valid_system_id(ecs_id_t id) { return id < ECS_MAX_SYSTEMS; }
static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id) { return ecs->entities[entity_id].ready; }
static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id) { return comp_id < ecs->comp_count; }
static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id) { return sys_id < ecs->system_count; }

ecs_t* ecs_new_i(lua_State* L, ecs_t* ecs, size_t entity_count, void* mem_ctx) {
    NEKO_ASSERT(entity_count > 0 && L && ecs);

    memset(ecs, 0, sizeof(ecs_t));

    ecs->entity_count = entity_count;
    ecs->mem_ctx = mem_ctx;
    ecs->L = L;

    {
        lua_newtable(L);
        ecs->system_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    // Initialize entity pool and queues
    ecs_stack_init(ecs, &ecs->entity_pool, entity_count);
    ecs_stack_init(ecs, &ecs->destroy_queue, entity_count);
    ecs_stack_init(ecs, &ecs->remove_queue, entity_count * 2);

    // Allocate entity array
    ecs->entities = (ecs_entity_t*)ECS_MALLOC(ecs->entity_count * sizeof(ecs_entity_t), ecs->mem_ctx);

    // Zero entity array
    memset(ecs->entities, 0, ecs->entity_count * sizeof(ecs_entity_t));

    // Pre-populate the the ID pool
    for (ecs_id_t id = 0; id < entity_count; id++) {
        ecs_stack_push(ecs, &ecs->entity_pool, id);
    }

    return ecs;
}

ecs_t* ecs_new(size_t entity_count, void* mem_ctx) {
    ecs_t* ecs = (ecs_t*)ECS_MALLOC(sizeof(ecs_t), mem_ctx);
    ecs = ecs_new_i(ENGINE_LUA(), ecs, entity_count, mem_ctx);
    return ecs;
}

void ecs_free_i(ecs_t* ecs) {
    NEKO_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t entity_id = 0; entity_id < ecs->entity_count; entity_id++) {
        if (ecs->entities[entity_id].ready) ecs_destroy(ecs, entity_id);
    }

    ecs_stack_free(ecs, &ecs->entity_pool);
    ecs_stack_free(ecs, &ecs->destroy_queue);
    ecs_stack_free(ecs, &ecs->remove_queue);

    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++) {
        ecs_array_t* comp_array = &ecs->comp_arrays[comp_id];
        ecs_array_free(ecs, comp_array);
    }

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++) {
        ecs_sys_t* sys = &ecs->systems[sys_id];
        ecs_sparse_set_free(ecs, &sys->entity_ids);
    }

    ECS_FREE(ecs->entities, ecs->mem_ctx);
}

void ecs_free(ecs_t* ecs) {
    ecs_free_i(ecs);
    ECS_FREE(ecs, ecs->mem_ctx);
}

void ecs_reset(ecs_t* ecs) {
    NEKO_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t entity_id = 0; entity_id < ecs->entity_count; entity_id++) {
        if (ecs->entities[entity_id].ready) ecs_destroy(ecs, entity_id);
    }

    ecs->entity_pool.size = 0;
    ecs->destroy_queue.size = 0;
    ecs->remove_queue.size = 0;

    memset(ecs->entities, 0, ecs->entity_count * sizeof(ecs_entity_t));

    for (ecs_id_t entity_id = 0; entity_id < ecs->entity_count; entity_id++) {
        ecs_stack_push(ecs, &ecs->entity_pool, entity_id);
    }

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++) {
        ecs->systems[sys_id].entity_ids.size = 0;
    }
}

ecs_id_t ecs_register_component(ecs_t* ecs, size_t size, ecs_constructor_fn constructor, ecs_destructor_fn destructor) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs->comp_count < ECS_MAX_COMPONENTS);
    NEKO_ASSERT(size > 0);

    ecs_id_t comp_id = ecs->comp_count;

    ecs_array_t* comp_array = &ecs->comp_arrays[comp_id];
    ecs_array_init(ecs, comp_array, size, ecs->entity_count);

    ecs->comps[comp_id].constructor = constructor;
    ecs->comps[comp_id].destructor = destructor;

    ecs->comp_count++;

    return comp_id;
}

ecs_id_t ecs_register_system(ecs_t* ecs, ecs_system_fn system_cb, ecs_added_fn add_cb, ecs_removed_fn remove_cb, void* udata) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs->system_count < ECS_MAX_SYSTEMS);
    NEKO_ASSERT(NULL != system_cb);

    ecs_id_t sys_id = ecs->system_count;
    ecs_sys_t* sys = &ecs->systems[sys_id];

    ecs_sparse_set_init(ecs, &sys->entity_ids, ecs->entity_count);

    sys->active = true;
    sys->system_cb = system_cb;
    sys->add_cb = add_cb;
    sys->remove_cb = remove_cb;
    sys->udata = udata;

    ecs->system_count++;

    return sys_id;
}

void ecs_require_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_system_id(sys_id));
    NEKO_ASSERT(ecs_is_valid_component_id(comp_id));
    NEKO_ASSERT(ecs_is_system_ready(ecs, sys_id));
    NEKO_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // 设置指定组件的系统组件位
    ecs_sys_t* sys = &ecs->systems[sys_id];
    ecs_bitset_flip(&sys->require_bits, comp_id, true);
}

void ecs_exclude_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_system_id(sys_id));
    NEKO_ASSERT(ecs_is_valid_component_id(comp_id));
    NEKO_ASSERT(ecs_is_system_ready(ecs, sys_id));
    NEKO_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // 设置指定组件的系统组件位
    ecs_sys_t* sys = &ecs->systems[sys_id];
    ecs_bitset_flip(&sys->exclude_bits, comp_id, true);
}

void ecs_enable_system(ecs_t* ecs, ecs_id_t sys_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_system_id(sys_id));
    NEKO_ASSERT(ecs_is_system_ready(ecs, sys_id));

    ecs_sys_t* sys = &ecs->systems[sys_id];
    sys->active = true;
}

void ecs_disable_system(ecs_t* ecs, ecs_id_t sys_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_system_id(sys_id));
    NEKO_ASSERT(ecs_is_system_ready(ecs, sys_id));

    ecs_sys_t* sys = &ecs->systems[sys_id];
    sys->active = false;
}

ecs_id_t ecs_create(ecs_t* ecs) {
    NEKO_ASSERT(ecs_is_not_null(ecs));

    ecs_stack_t* pool = &ecs->entity_pool;

    // If pool is empty, increase the number of entity IDs
    if (0 == ecs_stack_size(pool)) {
        size_t old_count = ecs->entity_count;
        size_t new_count = old_count + (old_count / 2) + 2;

        // Reallocates entities and zeros new ones
        ecs->entities = (ecs_entity_t*)ecs_realloc_zero(ecs, ecs->entities, old_count * sizeof(ecs_entity_t), new_count * sizeof(ecs_entity_t));

        // Push new entity IDs into the pool
        for (ecs_id_t id = old_count; id < new_count; id++) {
            ecs_stack_push(ecs, pool, id);
        }

        // Update entity count
        ecs->entity_count = new_count;
    }

    ecs_id_t entity_id = ecs_stack_pop(pool);
    ecs->entities[entity_id].ready = true;

    return entity_id;
}

bool ecs_is_ready(ecs_t* ecs, ecs_id_t entity_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));

    return ecs->entities[entity_id].ready;
}

void ecs_destroy(ecs_t* ecs, ecs_id_t entity_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++) {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        // Just attempting to remove the entity from the sparse set is faster
        // than calling ecs_entity_system_test
        if (ecs_sparse_set_remove(&sys->entity_ids, entity_id)) {
            if (sys->remove_cb) sys->remove_cb(ecs, entity_id, sys->udata);
        }
    }

    // Push entity ID back into pool
    ecs_stack_t* pool = &ecs->entity_pool;
    ecs_stack_push(ecs, pool, entity_id);

    // Loop through components and call the destructors
    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++) {
        if (ecs_bitset_test(&entity->comp_bits, comp_id)) {
            ecs_comp_t* comp = &ecs->comps[comp_id];

            if (comp->destructor) {
                void* ptr = ecs_get(ecs, entity_id, comp_id);
                comp->destructor(ecs, entity_id, ptr);
            }
        }
    }

    // Reset entity (sets bitset to 0 and ready to false)
    memset(entity, 0, sizeof(ecs_entity_t));
}

bool ecs_has(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_component_id(comp_id));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load  entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Return true if the component belongs to the entity
    return ecs_bitset_test(&entity->comp_bits, comp_id);
}

void* ecs_get(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_component_id(comp_id));
    NEKO_ASSERT(ecs_is_component_ready(ecs, comp_id));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Return pointer to component
    //  eid0,  eid1   eid2, ...
    // [comp0, comp1, comp2, ...]
    ecs_array_t* comp_array = &ecs->comp_arrays[comp_id];
    return (char*)comp_array->data + (comp_array->size * entity_id);
}

void* ecs_add(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id, void* args) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_component_id(comp_id));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));
    NEKO_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Load component
    ecs_array_t* comp_array = &ecs->comp_arrays[comp_id];
    ecs_comp_t* comp = &ecs->comps[comp_id];

    // Grow the component array
    ecs_array_resize(ecs, comp_array, entity_id);

    // Get pointer to component
    void* ptr = ecs_get(ecs, entity_id, comp_id);

    // Zero component
    memset(ptr, 0, comp_array->size);

    // Call constructor
    if (comp->constructor) comp->constructor(ecs, entity_id, ptr, args);

    // Set entity component bit that determines which systems this entity
    // belongs to
    ecs_bitset_flip(&entity->comp_bits, comp_id, true);

    // Add entity to systems
    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++) {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (ecs_entity_system_test(&sys->require_bits, &sys->exclude_bits, &entity->comp_bits)) {
            if (ecs_sparse_set_add(ecs, &sys->entity_ids, entity_id)) {
                if (sys->add_cb) sys->add_cb(ecs, entity_id, sys->udata);
            }
        }
    }

    // Return component
    return ptr;
}

void ecs_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_component_id(comp_id));
    NEKO_ASSERT(ecs_is_component_ready(ecs, comp_id));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++) {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (ecs_entity_system_test(&sys->require_bits, &sys->exclude_bits, &entity->comp_bits)) {
            if (ecs_sparse_set_remove(&sys->entity_ids, entity_id)) {
                if (sys->remove_cb) sys->remove_cb(ecs, entity_id, sys->udata);
            }
        }
    }

    ecs_comp_t* comp = &ecs->comps[comp_id];

    if (comp->destructor) {
        void* ptr = ecs_get(ecs, entity_id, comp_id);
        comp->destructor(ecs, entity_id, ptr);
    }

    // 重置相关组件掩码位
    ecs_bitset_flip(&entity->comp_bits, comp_id, false);
}

void ecs_queue_destroy(ecs_t* ecs, ecs_id_t entity_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    ecs_stack_push(ecs, &ecs->destroy_queue, entity_id);
}

void ecs_queue_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_entity_ready(ecs, entity_id));
    NEKO_ASSERT(ecs_has(ecs, entity_id, comp_id));

    ecs_stack_push(ecs, &ecs->remove_queue, entity_id);
    ecs_stack_push(ecs, &ecs->remove_queue, comp_id);
}

ecs_ret_t ecs_update_system(ecs_t* ecs, ecs_id_t sys_id, ecs_dt_t dt) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_valid_system_id(sys_id));
    NEKO_ASSERT(ecs_is_system_ready(ecs, sys_id));
    NEKO_ASSERT(dt >= 0.0f);

    ecs_sys_t* sys = &ecs->systems[sys_id];

    if (!sys->active) return 0;

    ecs_ret_t code = sys->system_cb(ecs, sys->entity_ids.dense, sys->entity_ids.size, dt, sys->udata);

    ecs_flush_destroyed(ecs);
    ecs_flush_removed(ecs);

    return code;
}

ecs_ret_t ecs_update_systems(ecs_t* ecs, ecs_dt_t dt) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(dt >= 0.0f);

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++) {
        ecs_ret_t code = ecs_update_system(ecs, sys_id, dt);

        if (0 != code) return code;
    }

    return 0;
}

void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size) {
    (void)ecs;

    ptr = ECS_REALLOC(ptr, new_size, ecs->mem_ctx);

    if (new_size > old_size && ptr) {
        size_t diff = new_size - old_size;
        void* start = ((char*)ptr) + old_size;
        memset(start, 0, diff);
    }

    return ptr;
}

// 用于刷新被破坏的实体和删除的组件的内部函数

static void ecs_flush_destroyed(ecs_t* ecs) {
    ecs_stack_t* destroy_queue = &ecs->destroy_queue;

    for (size_t i = 0; i < destroy_queue->size; i++) {
        ecs_id_t entity_id = destroy_queue->array[i];

        if (ecs_is_ready(ecs, entity_id)) ecs_destroy(ecs, entity_id);
    }

    destroy_queue->size = 0;
}

static void ecs_flush_removed(ecs_t* ecs) {
    ecs_stack_t* remove_queue = &ecs->remove_queue;

    for (size_t i = 0; i < remove_queue->size; i += 2) {
        ecs_id_t entity_id = remove_queue->array[i];

        if (ecs_is_ready(ecs, entity_id)) {
            ecs_id_t comp_id = remove_queue->array[i + 1];
            ecs_remove(ecs, entity_id, comp_id);
        }
    }

    remove_queue->size = 0;
}

// 内部位集函数

#if ECS_MAX_COMPONENTS <= 64

static inline bool ecs_bitset_is_zero(ecs_bitset_t* set) { return *set == 0; }

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on) {
    if (on)
        *set |= ((uint64_t)1 << bit);
    else
        *set &= ~((uint64_t)1 << bit);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit) { return *set & ((uint64_t)1 << bit); }

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2) { return *set1 & *set2; }

static inline ecs_bitset_t ecs_bitset_or(ecs_bitset_t* set1, ecs_bitset_t* set2) { return *set1 | *set2; }

static inline ecs_bitset_t ecs_bitset_not(ecs_bitset_t* set) { return ~(*set); }

static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2) { return *set1 == *set2; }

static inline bool ecs_bitset_true(ecs_bitset_t* set) { return *set; }

#else  // ECS_MAX_COMPONENTS

static inline bool ecs_bitset_is_zero(ecs_bitset_t* set) {
    for (int i = 0; i < ECS_BITSET_SIZE; i++) {
        if (set->array[i] != 0) return false;
    }

    return true;
}

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on) {
    int index = bit / ECS_BITSET_WIDTH;

    if (on)
        set->array[index] |= ((uint64_t)1 << bit % ECS_BITSET_WIDTH);
    else
        set->array[index] &= ~((uint64_t)1 << bit % ECS_BITSET_WIDTH);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit) {
    int index = bit / ECS_BITSET_WIDTH;
    return set->array[index] & ((uint64_t)1 << bit % ECS_BITSET_WIDTH);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2) {
    ecs_bitset_t set;

    for (int i = 0; i < ECS_BITSET_SIZE; i++) {
        set.array[i] = set1->array[i] & set2->array[i];
    }

    return set;
}

static inline ecs_bitset_t ecs_bitset_or(ecs_bitset_t* set1, ecs_bitset_t* set2) {
    ecs_bitset_t set;

    for (int i = 0; i < ECS_BITSET_SIZE; i++) {
        set.array[i] = set1->array[i] | set2->array[i];
    }

    return set;
}

static inline ecs_bitset_t ecs_bitset_not(ecs_bitset_t* set) {
    ecs_bitset_t out;

    for (int i = 0; i < ECS_BITSET_SIZE; i++) {
        out.array[i] = ~set->array[i];
    }

    return out;
}

static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2) {
    for (int i = 0; i < ECS_BITSET_SIZE; i++) {
        if (set1->array[i] != set2->array[i]) {
            return false;
        }
    }

    return true;
}

static inline bool ecs_bitset_true(ecs_bitset_t* set) {
    for (int i = 0; i < ECS_BITSET_SIZE; i++) {
        if (set->array[i]) return true;
    }

    return false;
}

#endif  // ECS_MAX_COMPONENTS

// 内部稀疏集函数
static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(set));
    NEKO_ASSERT(capacity > 0);

    (void)ecs;

    set->capacity = capacity;
    set->size = 0;

    set->dense = (ecs_id_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
    set->sparse = (size_t*)ECS_MALLOC(capacity * sizeof(size_t), ecs->mem_ctx);

    memset(set->sparse, 0, capacity * sizeof(size_t));
}

static void ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    ECS_FREE(set->dense, ecs->mem_ctx);
    ECS_FREE(set->sparse, ecs->mem_ctx);
}

static bool ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    // Grow sparse set if necessary
    if (id >= set->capacity) {
        size_t old_capacity = set->capacity;
        size_t new_capacity = old_capacity;

        // Calculate new capacity
        while (new_capacity <= id) {
            new_capacity += (new_capacity / 2) + 2;
        }

        // Grow dense array
        set->dense = (ecs_id_t*)ECS_REALLOC(set->dense, new_capacity * sizeof(ecs_id_t), ecs->mem_ctx);

        // Grow sparse array and zero it
        set->sparse = (size_t*)ecs_realloc_zero(ecs, set->sparse, old_capacity * sizeof(size_t), new_capacity * sizeof(size_t));

        // Set the new capacity
        set->capacity = new_capacity;
    }

    // Check if ID exists within the set
    if (ECS_NULL != ecs_sparse_set_find(set, id)) return false;

    // Add ID to set
    set->dense[set->size] = id;
    set->sparse[id] = set->size;

    set->size++;

    return true;
}

static size_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id) {
    NEKO_ASSERT(ecs_is_not_null(set));

    if (set->sparse[id] < set->size && set->dense[set->sparse[id]] == id)
        return set->sparse[id];
    else
        return ECS_NULL;
}

static bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id) {
    NEKO_ASSERT(ecs_is_not_null(set));

    if (ECS_NULL == ecs_sparse_set_find(set, id)) return false;

    // Swap and remove (changes order of array)
    ecs_id_t tmp = set->dense[set->size - 1];
    set->dense[set->sparse[id]] = tmp;
    set->sparse[tmp] = set->sparse[id];

    set->size--;

    return true;
}

// 内部系统实体添加/删除功能
inline static bool ecs_entity_system_test(ecs_bitset_t* require_bits, ecs_bitset_t* exclude_bits, ecs_bitset_t* entity_bits) {
    if (!ecs_bitset_is_zero(exclude_bits)) {
        ecs_bitset_t overlap = ecs_bitset_and(entity_bits, exclude_bits);

        if (ecs_bitset_true(&overlap)) {
            return false;
        }
    }

    ecs_bitset_t entity_and_require = ecs_bitset_and(entity_bits, require_bits);
    return ecs_bitset_equal(&entity_and_require, require_bits);
}

// 内部ID池功能
inline static void ecs_stack_init(ecs_t* ecs, ecs_stack_t* stack, int capacity) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(stack));
    NEKO_ASSERT(capacity > 0);

    (void)ecs;

    stack->size = 0;
    stack->capacity = capacity;
    stack->array = (ecs_id_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
}

inline static void ecs_stack_free(ecs_t* ecs, ecs_stack_t* stack) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(stack));

    (void)ecs;

    ECS_FREE(stack->array, ecs->mem_ctx);
}

inline static void ecs_stack_push(ecs_t* ecs, ecs_stack_t* stack, ecs_id_t id) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(stack));
    NEKO_ASSERT(stack->capacity > 0);

    (void)ecs;

    if (stack->size == stack->capacity) {
        stack->capacity += (stack->capacity / 2) + 2;

        stack->array = (ecs_id_t*)ECS_REALLOC(stack->array, stack->capacity * sizeof(ecs_id_t), ecs->mem_ctx);
    }

    stack->array[stack->size++] = id;
}

inline static ecs_id_t ecs_stack_pop(ecs_stack_t* stack) {
    NEKO_ASSERT(ecs_is_not_null(stack));
    return stack->array[--stack->size];
}

inline static int ecs_stack_size(ecs_stack_t* stack) { return stack->size; }

static void ecs_array_init(ecs_t* ecs, ecs_array_t* array, size_t size, size_t capacity) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    memset(array, 0, sizeof(ecs_array_t));

    array->capacity = capacity;
    array->count = 0;
    array->size = size;
    array->data = ECS_MALLOC(size * capacity, ecs->mem_ctx);
}

static void ecs_array_free(ecs_t* ecs, ecs_array_t* array) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    ECS_FREE(array->data, ecs->mem_ctx);
}

static void ecs_array_resize(ecs_t* ecs, ecs_array_t* array, size_t capacity) {
    NEKO_ASSERT(ecs_is_not_null(ecs));
    NEKO_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    if (capacity >= array->capacity) {
        while (array->capacity <= capacity) {
            array->capacity += (array->capacity / 2) + 2;
        }

        array->data = ECS_REALLOC(array->data, array->capacity * array->size, ecs->mem_ctx);
    }
}

#endif  // NEKO_ECS_IMPLEMENTATION

static int __neko_ecs_lua_create_ent(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_id_t e = ecs_create(w);
    neko::__lua_op_t<ecs_id_t>::push(L, e);
    return 1;
}

struct ecs_ent_iter_t {
    ecs_stack_t* pool;
    ecs_id_t index;
    bool check_ready;
};

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
        if (w->entities[e].ready == it->check_ready) {
            break;
        }
    }
    neko::__lua_op_t<ecs_id_t>::push(L, e);
    return 1;
}

LUA_FUNCTION(__neko_ecs_lua_ent_iterator) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    bool check_ready = neko::__lua_op_t<bool>::get(L, 2);

    NEKO_ASSERT(ecs_is_not_null(w));

    ecs_ent_iter_t* it = (ecs_ent_iter_t*)lua_newuserdata(L, sizeof(ecs_ent_iter_t));

    it->pool = &w->entity_pool;
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
            void* ptr = ecs_add(w, e, c, NULL);
            ptrs.push(ptr);
        } else {
            NEKO_WARN("argument %d is not a string", i);
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
    ecs_free_i(w);
    NEKO_DEBUG_LOG("ecs_lua_gc");
    return 0;
}

struct lua_system_userdata {
    String system_name;
};

static ecs_ret_t l_system_update(ecs_t* ecs, ecs_id_t* entities, int entity_count, ecs_dt_t dt, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    lua_State* L = ecs->L;

    int systb = ecs->system_table_ref;

    NEKO_ASSERT(systb != LUA_NOREF && L);

    lua_rawgeti(L, LUA_REGISTRYINDEX, systb);
    lua_getfield(L, -1, ud->system_name.cstr());
    if (lua_istable(L, -1)) {

        for (int id = 0; id < entity_count; id++) {
            lua_getfield(L, -1, "func_update");
            NEKO_ASSERT(lua_isfunction(L, -1));
            lua_pushinteger(L, entities[id]);
            lua_pushlstring(L, ud->system_name.cstr(), ud->system_name.len);
            lua_call(L, 2, 0);  // 调用
        }

    } else {
        NEKO_WARN("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);

    return 0;
}

static void l_system_add(ecs_t* ecs, ecs_id_t entity_id, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    // NEKO_DEBUG_LOG("l_system_add %s ent_id %d", ud->system_name.cstr(), entity_id);

    lua_State* L = ecs->L;
    int systb = ecs->system_table_ref;

    NEKO_ASSERT(systb != LUA_NOREF);

    lua_rawgeti(L, LUA_REGISTRYINDEX, systb);
    lua_getfield(L, -1, ud->system_name.cstr());
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "func_add");
        NEKO_ASSERT(lua_isfunction(L, -1));
        neko::__lua_op_t<int>::push(L, entity_id);
        lua_pushlstring(L, ud->system_name.cstr(), ud->system_name.len);
        lua_call(L, 2, 0);  // 调用
    } else {
        NEKO_WARN("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);
}

static void l_system_remove(ecs_t* ecs, ecs_id_t entity_id, void* udata) {
    lua_system_userdata* ud = (lua_system_userdata*)udata;
    // NEKO_DEBUG_LOG("l_system_remove %s ent_id %d", ud->system_name.cstr(), entity_id);

    lua_State* L = ecs->L;
    int systb = ecs->system_table_ref;

    NEKO_ASSERT(systb != LUA_NOREF);

    lua_rawgeti(L, LUA_REGISTRYINDEX, systb);
    lua_getfield(L, -1, ud->system_name.cstr());
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "func_remove");
        NEKO_ASSERT(lua_isfunction(L, -1));
        neko::__lua_op_t<int>::push(L, entity_id);
        lua_pushlstring(L, ud->system_name.cstr(), ud->system_name.len);
        lua_call(L, 2, 0);  // 调用
    } else {
        NEKO_WARN("callback with identifier '%s' not found or is not a function", ud->system_name.cstr());
    }
    lua_pop(L, 1);
}

static int __neko_ecs_lua_system(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    NEKO_ASSERT(w->L == L);

    ecs_id_t sys = u32_max;
    String system_name = luax_check_string(L, ECS_WORLD + 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, w->system_table_ref);
    if (lua_istable(L, -1)) {

        lua_createtable(L, 0, 2);

        lua_pushvalue(L, ECS_WORLD + 2);
        NEKO_ASSERT(lua_isfunction(L, -1));
        lua_setfield(L, -2, "func_update");

        lua_pushvalue(L, ECS_WORLD + 3);
        NEKO_ASSERT(lua_isfunction(L, -1));
        lua_setfield(L, -2, "func_add");

        lua_pushvalue(L, ECS_WORLD + 4);
        NEKO_ASSERT(lua_isfunction(L, -1));
        lua_setfield(L, -2, "func_remove");

        lua_pushstring(L, system_name.cstr());
        lua_setfield(L, -2, "name");

        lua_setfield(L, -2, system_name.cstr());

        lua_system_userdata* ud = (lua_system_userdata*)mem_alloc(sizeof(lua_system_userdata));  // gc here
        ud->system_name = system_name;
        sys = ecs_register_system(w, l_system_update, l_system_add, l_system_remove, (void*)ud);
    }

    NEKO_ASSERT(sys != u32_max);
    lua_pushinteger(L, sys);
    return 1;
}

static int __neko_ecs_lua_system_require_component(lua_State* L) {
    int n = lua_gettop(L);
    luaL_argcheck(L, n >= 3, 1, "lost the component name");

    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    NEKO_ASSERT(w->L == L);

    ecs_id_t sys = luaL_checkinteger(L, ECS_WORLD + 1);

    for (int i = 3; i <= n; i++) {
        if (lua_isstring(L, i)) {
            const_str comp_name = luaL_checkstring(L, i);
            ecs_id_t comp_id = __neko_ecs_lua_component_id_w(L, comp_name);
            ecs_require_component(w, sys, comp_id);
        } else {
            NEKO_WARN("argument %d is not a ecs_component", i);
        }
    }

    // lua_pushinteger(L, ret);
    return 0;
}

static int __neko_ecs_lua_get(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    NEKO_ASSERT(w->L == L);
    ecs_id_t ent_id = lua_tointeger(L, ECS_WORLD + 1);
    ecs_id_t comp_id = lua_tointeger(L, ECS_WORLD + 2);
    void* ptr = ecs_get(w, ent_id, comp_id);
    lua_pushlightuserdata(L, ptr);
    return 1;
}

static int __neko_ecs_lua_system_run(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    NEKO_ASSERT(w->L == L);
    ecs_id_t sys = lua_tointeger(L, ECS_WORLD + 1);
    f64 dt = lua_tonumber(L, ECS_WORLD + 2);
    ecs_ret_t ret = ecs_update_system(w, sys, dt);
    lua_pushinteger(L, ret);
    return 1;
}

ecs_id_t ecs_component_w(ecs_t* w, const_str component_name, size_t component_size, ecs_constructor_fn constructor, ecs_destructor_fn destructor) {
    PROFILE_FUNC();

    ecs_id_t comp_id = ecs_register_component(w, component_size, constructor, destructor);

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
            //     NEKO_ASSERT(lua_isfunction(L, -1));
            //     lua_setfield(L, -2, "func_ctor");
            // }

            lua_settable(L, -3);
            lua_pop(L, 1);
        } else {
            NEKO_ERROR("%s", "failed to get comp_map");
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
            // //     NEKO_ASSERT(lua_isfunction(L, -1));
            // //     lua_setfield(L, -2, "func_ctor");
            // // }

            // lua_settable(L, -3);
            lua_pop(L, 1);
        } else {
            NEKO_ERROR("%s", "failed to get comps");
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    } else {
        NEKO_ERROR("%s", "failed to get upvalue NEKO_ECS_COMPONENTS_NAME");
        lua_pop(L, 1);
    }
    lua_pop(L, 1);  // pop 1

    return comp_id;
}

static void l_component_ctor(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args) {
    // lua_system_userdata* ud = (lua_system_userdata*)udata;
    // NEKO_DEBUG_LOG("l_system_remove %s ent_id %d", ud->system_name.cstr(), entity_id);

    lua_State* L = ecs->L;
    int systb = ecs->system_table_ref;

    NEKO_ASSERT(systb != LUA_NOREF);
}

static int __neko_ecs_lua_component(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    NEKO_ASSERT(w->L == L);

    ecs_id_t id = u32_max;
    String comp_name = luax_check_string(L, ECS_WORLD + 1);
    size_t size = luaL_checkinteger(L, ECS_WORLD + 2);

    id = ecs_component_w(w, comp_name.cstr(), size, NULL, NULL);

    NEKO_ASSERT(id != u32_max);
    lua_pushinteger(L, id);
    return 1;
}

static int __neko_ecs_lua_component_id(lua_State* L) {
    ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    NEKO_ASSERT(w->L == L);

    ecs_id_t id = u32_max;
    String comp_name = luax_check_string(L, ECS_WORLD + 1);

    id = __neko_ecs_lua_component_id_w(L, comp_name.cstr());

    NEKO_ASSERT(id != u32_max);
    lua_pushinteger(L, id);
    return 1;
}

ecs_t* ecs_init(lua_State* L) {
    PROFILE_FUNC();

    NEKO_ASSERT(L);
    ecs_t* w = (ecs_t*)lua_newuserdatauv(L, sizeof(ecs_t), NEKO_ECS_UPVAL_N);  // # -1
    w = ecs_new_i(L, w, 1024, NULL);
    if (w == NULL || w->entities == NULL) {
        NEKO_ERROR("failed to initialize ecs_t");
        return NULL;
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
            {"csb_core", open_csb},
            {"csb", neko_ecs_lua_csb},
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
