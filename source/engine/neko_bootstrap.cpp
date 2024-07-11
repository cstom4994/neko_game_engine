
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko.hpp"
#include "engine/neko_api.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_lua.h"
#include "engine/neko_math.h"

extern "C" {

#define __neko_boot_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_boot_ent_index(id) ((u32)id)
#define __neko_boot_ent_ver(id) ((u32)(id >> 32))

#define MAX_ENTITY_COUNT 100

// 测试 ECS 用
typedef struct CGameObjectTest {
    char name[64];
    bool active;
    bool visible;
    bool selected;
} CGameObjectTest;

enum ComponentType {
    COMPONENT_GAMEOBJECT,
    COMPONENT_TRANSFORM,
    COMPONENT_VELOCITY,
    COMPONENT_BOUNDS,
    COMPONENT_COLOR,

    COMPONENT_COUNT
};

#define ECS_WORLD_UDATA_NAME "__NEKO_ECS_WORLD"
#define ECS_WORLD (1)

typedef neko_vec2_t position_t;  // Position component
typedef neko_vec2_t velocity_t;  // Velocity component
typedef neko_vec2_t bounds_t;    // Bounds component
typedef neko_color_t color_t;    // Color component

#if 0
NEKO_API_DECL ECS_COMPONENT_DECLARE(position_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(velocity_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(bounds_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(color_t);

NEKO_API_DECL void neko_boot_com_init(ecs_world_t* world);
#endif

/*=============================
// ECS
=============================*/

#define MY_TABLE_KEY "my_integer_keyed_table"

enum ECS_LUA_UPVALUES { NEKO_ECS_COMPONENTS_NAME = 1, NEKO_ECS_UPVAL_N };

#if 1

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
    struct component *buf;
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
    struct neko_boot_world_t *w;
    int i;
    int kn;
    int keys[ENTITY_MAX_COMPONENTS];
};

struct neko_boot_world_t {
    int entity_cap;
    int entity_free;  // TODO:将ENTITY_FREE设置为FIFO 这可以在以后回收EID以避免某些错误
    int entity_dead;
    int type_idx;
    struct entity *entity_buf;
    struct component_pool component_pool[TYPE_COUNT];
};

static inline int component_has(struct entity *e, int tid) { return e->components[tid] < ENTITY_MAX_COMPONENTS; }

static inline void component_clr(struct entity *e, int tid) {
    unsigned char idx = e->components[tid];
    if (idx < ENTITY_MAX_COMPONENTS) {
        e->index[idx] = -1;
        e->components[tid] = ENTITY_MAX_COMPONENTS;
        --e->cn;
    }
}

static inline int component_add(lua_State *L, struct neko_boot_world_t *w, struct entity *e, int tid) {
    int cid, i;
    struct component *c;
    struct component_pool *cp;
    struct component_ptr *ptr;
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
        cp->buf = (struct component *)neko_safe_realloc(cp->buf, cp->cap * sizeof(cp->buf[0]));
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

static inline void component_dead(struct neko_boot_world_t *w, int tid, int cid) {
    struct component *c;
    struct component_pool *cp;
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

static inline void component_dirty(struct neko_boot_world_t *w, int tid, int cid) {
    struct component *c;
    struct component_pool *cp;
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

static inline struct entity *entity_alloc(struct neko_boot_world_t *w) {
    struct entity *e;
    int eid = w->entity_free;
    if (eid < 0) {
        int i = 0;
        int oldcap = w->entity_cap;
        int newcap = oldcap * 2;
        w->entity_cap = newcap;
        w->entity_buf = (struct entity *)neko_safe_realloc(w->entity_buf, newcap * sizeof(w->entity_buf[0]));
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

static inline void entity_dead(struct neko_boot_world_t *w, struct entity *e) {
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

static inline void entity_free(struct neko_boot_world_t *w, struct entity *e) {
    assert(e->cn == -1);
    e->next = w->entity_free;
    w->entity_free = e - w->entity_buf;
}

static inline int get_typeid(lua_State *L, int stk, int proto_id) {
    int id;
    stk = lua_absindex(L, stk);
    lua_pushvalue(L, stk);
    lua_gettable(L, proto_id);
    id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    luaL_argcheck(L, id >= TYPE_MIN_ID, stk, "invalid type");
    return id;
}

static inline struct entity *get_entity(lua_State *L, struct neko_boot_world_t *w, int stk) {
    struct entity *e;
    int eid = luaL_checkinteger(L, stk);
    luaL_argcheck(L, eid < w->entity_cap, 2, "eid is invalid");
    e = &w->entity_buf[eid];
    luaL_argcheck(L, e->cn >= 0, 2, "entity is dead");
    return e;
}

static inline int get_cid_in_entity(struct entity *e, int tid) {
    int i = e->components[tid];
    if (i >= ENTITY_MAX_COMPONENTS) return -1;
    return e->index[i];
}

static inline void update_cid_in_entity(struct entity *e, int tid, int cid) {
    int i = e->components[tid];
    assert(i < ENTITY_MAX_COMPONENTS);
    e->index[i] = cid;
}

static int __neko_boot_world_end(lua_State *L) {
    struct neko_boot_world_t *w;
    int type, tid;
    struct component_pool *cp;
    w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    tid = w->type_idx;  // 总数-1(索引)
    for (int i = 0; i <= tid; i++) {
        cp = &w->component_pool[i];
        neko_safe_free(cp->buf);
    }
    neko_safe_free(w->entity_buf);
    return 0;
}

static int __neko_boot_world_register(lua_State *L) {
    struct neko_boot_world_t *w;
    int type, tid;
    struct component_pool *cp;
    w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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
    cp->buf = (struct component *)neko_safe_malloc(cp->cap * sizeof(cp->buf[0]));
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

static int __neko_boot_world_new_entity(lua_State *L) {
    int i, components, proto_id;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = entity_alloc(w);
    int eid = e - w->entity_buf;
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    components = lua_gettop(L);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = components + 1;
    lua_pushnil(L); /* first key */
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

static int __neko_boot_world_del_entity(lua_State *L) {
    int i;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
    entity_dead(w, e);
    return 0;
}

static int __neko_boot_world_get_component(lua_State *L) {
    int i, top, proto_id, components;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
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

static int __neko_boot_world_add_component(lua_State *L) {
    int tid, cid;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
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

static int __neko_boot_world_remove_component(lua_State *L) {
    int i, proto_id;
    int tid, cid;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
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

static int __neko_boot_world_touch_component(lua_State *L) {
    int eid, tid, cid;
    struct entity *e;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static void push_result(lua_State *L, int *keys, int kn, struct entity *e) {
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

static inline struct entity *restrict_component(struct entity *ebuf, struct component *c, int *keys, int kn) {
    int i;
    struct entity *e;
    e = &ebuf[c->eid];
    for (i = 1; i < kn; i++) {
        if (!component_has(e, keys[i])) return NULL;
    }
    return e;
}

// match_all(match_ctx *mctx, nil)
static int match_all(lua_State *L) {
    int i;
    int mi, free;
    int kn, *keys;
    struct entity *e = NULL;
    struct component_pool *cp;
    struct match_ctx *mctx = (struct match_ctx *)lua_touserdata(L, 1);
    struct neko_boot_world_t *w = mctx->w;
    struct entity *entity_buf = w->entity_buf;
    kn = mctx->kn;
    keys = mctx->keys;
    cp = &w->component_pool[mctx->keys[0]];
    mi = mctx->i;
    free = cp->free;
    while (mi < free) {
        struct component *c = &cp->buf[mi++];
        if (c->dead_next == LINK_NONE) {
            e = restrict_component(entity_buf, c, keys, kn);
            if (e != NULL) break;
        }
    }
    mctx->i = mi;
    push_result(L, keys, kn, e);
    return kn;
}

static int match_dirty(lua_State *L) {
    int next;
    int kn, *keys;
    struct entity *e = NULL;
    struct component_pool *cp;
    struct match_ctx *mctx = (struct match_ctx *)lua_touserdata(L, 1);
    struct neko_boot_world_t *w = mctx->w;
    struct entity *entity_buf = w->entity_buf;
    keys = mctx->keys;
    kn = mctx->kn;
    next = mctx->i;
    cp = &w->component_pool[keys[0]];
    while (next != LINK_NIL) {
        struct component *c = &cp->buf[next];
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

static int match_dead(lua_State *L) {
    int next;
    int kn, *keys;
    struct entity *e = NULL;
    struct component_pool *cp;
    struct match_ctx *mctx = (struct match_ctx *)lua_touserdata(L, 1);
    struct neko_boot_world_t *w = mctx->w;
    struct entity *entity_buf = w->entity_buf;
    keys = mctx->keys;
    kn = mctx->kn;
    next = mctx->i;
    cp = &w->component_pool[keys[0]];
    while (next != LINK_NIL) {
        struct component *c = &cp->buf[next];
        next = c->dead_next;
        e = restrict_component(entity_buf, c, keys, kn);
        if (e != NULL) break;
    }
    mctx->i = next;
    push_result(L, keys, kn, e);
    return kn;
}

static int __neko_boot_world_match_component(lua_State *L) {
    int i;
    size_t sz;
    struct neko_boot_world_t *w;
    enum match_mode mode;
    int (*iter)(lua_State *L) = NULL;
    struct match_ctx *mctx = NULL;
    const char *m = luaL_checklstring(L, 2, &sz);
    int top = lua_gettop(L);
    w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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
    mctx = (struct match_ctx *)lua_touserdata(L, -1);
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

static int __neko_boot_world_update(lua_State *L) {
    int next;
    int t, components;
    struct component_pool *pool;
    struct neko_boot_world_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *entity_buf = w->entity_buf;
    // clear dead entity
    next = w->entity_dead;
    while (next != LINK_NIL) {
        struct entity *e;
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
        struct component *buf;
        struct component_pool *cp;
        cp = &pool[t];
        cp->dirty_head = LINK_NIL;
        cp->dirty_tail = LINK_NIL;
        cp->dead_head = LINK_NIL;
        cp->dead_tail = LINK_NIL;
        lua_geti(L, -1, t);
        buf = cp->buf;
        free = cp->free;
        for (r = 0; r < free; r++) {
            struct component *c;
            c = &buf[r];
            c->dirty_next = LINK_NONE;
            if (c->dead_next == LINK_NONE) {  // alive
                if (w != r) {
                    struct entity *e;
                    e = &entity_buf[c->eid];
                    buf[w] = *c;
                    lua_geti(L, -1, r);
                    lua_seti(L, -2, w);
                    update_cid_in_entity(e, t, w);
                }
                w++;
            } else {  // dead component
                struct entity *e;
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

static void print_value(lua_State *L, int stk, int tab) {
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

static void dump_table(lua_State *L, int stk, int tab) {
    lua_pushnil(L); /* first key */
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

static void dump_upval(lua_State *L, int upval) {
    const char *name[] = {
            "PROTO_ID",
            "PROTO_DEFINE",
            "COMPONENTS",
    };
    printf("==dump== %s\n", name[upval - 1]);
    if (lua_getiuservalue(L, ECS_WORLD, upval) == LUA_TTABLE) dump_table(L, lua_gettop(L), 0);
    lua_pop(L, 1);
}

static int __neko_boot_world_dump(lua_State *L) {
    int i;
    for (i = 1; i <= WORLD_COMPONENTS; i++) dump_upval(L, i);
    return 0;
}

int __neko_boot_create_world(lua_State *L) {
    int i;
    struct neko_boot_world_t *w;
    struct match_ctx *mctx;
    w = (struct neko_boot_world_t *)lua_newuserdatauv(L, sizeof(*w), WORLD_UPVAL_N);
    memset(w, 0, sizeof(*w));
    w->entity_cap = 128;
    w->entity_free = 0;
    w->entity_dead = LINK_NIL;
    w->type_idx = 0;
    w->entity_buf = (struct entity *)neko_safe_malloc(w->entity_cap * sizeof(w->entity_buf[0]));
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
                {"__gc", __neko_boot_world_end},
                {"register", __neko_boot_world_register},
                {"new", __neko_boot_world_new_entity},
                {"del", __neko_boot_world_del_entity},
                {"get", __neko_boot_world_get_component},
                {"add", __neko_boot_world_add_component},
                {"remove", __neko_boot_world_remove_component},
                {"touch", __neko_boot_world_touch_component},
                {"match", __neko_boot_world_match_component},
                {"update", __neko_boot_world_update},
                {"dump", __neko_boot_world_dump},
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

    mctx = (struct match_ctx *)lua_newuserdatauv(L, sizeof(*mctx), 1);
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);
    lua_setiuservalue(L, 1, WORLD_MATCH_CTX);

    lua_pushliteral(L, "__eid");
    lua_setiuservalue(L, 1, WORLD_KEY_EID);

    lua_pushliteral(L, "__tid");
    lua_setiuservalue(L, 1, WORLD_KEY_TID);

    return 1;
}

#endif

#if 0

static int __neko_boot_lua_create_ent(lua_State *L) {
    struct neko_boot_t *w = (struct neko_boot_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_entity_t e = ecs_entity(w);
    lua_pushinteger(L, e);
    return 1;
}

const ecs_entity_t __neko_boot_lua_component_id_w(lua_State *L, const_str component_name) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");  // # -5
    lua_getiuservalue(L, -1, NEKO_ECS_COMPONENTS_NAME);     // # -4
    lua_getfield(L, -1, "comp_map");                        // # -3
    lua_pushinteger(L, neko_hash_str(component_name));      // 使用 32 位哈希以适应 Lua 数字范围
    lua_gettable(L, -2);                                    // # -2
    lua_getfield(L, -1, "id");                              // # -1
    const ecs_entity_t c = lua_tointeger(L, -1);
    lua_pop(L, 5);
    return c;
}

static int __neko_boot_lua_attach(lua_State *L) {
    int n = lua_gettop(L);
    luaL_argcheck(L, n >= 3, 1, "lost the component name");

    struct neko_boot_t *w = (struct neko_boot_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_entity_t e = luaL_checkinteger(L, 2);

    for (int i = 3; i <= n; i++) {
        if (lua_isstring(L, i)) {
            const_str component_name = lua_tostring(L, i);
            const ecs_entity_t c = __neko_boot_lua_component_id_w(L, component_name);
            ecs_attach(w, e, c);
        } else {
            NEKO_WARN("argument %d is not a string", i);
        }
    }

    return 0;
}

static int __neko_boot_lua_get_com(lua_State *L) {
    struct neko_boot_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_UPVAL_N);
    return 2;
}

static int __neko_boot_lua_gc(lua_State *L) {
    struct neko_boot_t *w = (struct neko_boot_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_fini_i(w);
    NEKO_INFO("ecs_lua_gc");
    return 0;
}

neko_boot_t *ecs_init(lua_State *L) {
    NEKO_ASSERT(L);
    neko_boot_t *registry = (neko_boot_t *)lua_newuserdatauv(L, sizeof(neko_boot_t), NEKO_ECS_UPVAL_N);  // # -1
    registry = ecs_init_i(registry);
    if (registry == NULL) {
        NEKO_ERROR("%s", "failed to initialize neko_boot_t");
        return NULL;
    }
    registry->L = L;

    if (luaL_getmetatable(L, ECS_WORLD_UDATA_NAME) == LUA_TNIL) {  // # -2

        // clang-format off
        luaL_Reg world_mt[] = {
            {"__gc", __neko_boot_lua_gc}, 
            {"create_ent", __neko_boot_lua_create_ent}, 
            {"attach", __neko_boot_lua_attach}, 
            {"get_com", __neko_boot_lua_get_com}, 
            {NULL, NULL}
        };
        // clang-format on

        lua_pop(L, 1);                  // # pop -2
        luaL_newlibtable(L, world_mt);  // # -2
        luaL_setfuncs(L, world_mt, 0);
        lua_pushvalue(L, -1);                                      // # -3
        lua_setfield(L, -2, "__index");                            // pop -3
        lua_pushliteral(L, ECS_WORLD_UDATA_NAME);                  // # -3
        lua_setfield(L, -2, "__name");                             // pop -3
        lua_pushvalue(L, -1);                                      // # -3
        lua_setfield(L, LUA_REGISTRYINDEX, ECS_WORLD_UDATA_NAME);  // pop -3
    }
    lua_setmetatable(L, -2);  // pop -2

    // lua_newtable(L);  // components name 表
    // lua_setfield(L, LUA_REGISTRYINDEX, MY_TABLE_KEY);
    //
    // lua_setiuservalue(L, -2, NEKO_ECS_COMPONENTS_NAME);

    lua_newtable(L);
    lua_pushstring(L, "comp_map");
    lua_createtable(L, 0, ENTITY_MAX_COMPONENTS);
    lua_settable(L, -3);
    lua_setiuservalue(L, -2, NEKO_ECS_COMPONENTS_NAME);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, -2, NEKO_ECS_UPVAL_N);

    // lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");

    // lua_setglobal(L, "__NEKO_ECS_CORE");

    return registry;
}

ecs_entity_t ecs_component_w(neko_boot_t *registry, const_str component_name, size_t component_size) {

    ecs_map_set(registry->component_index, (void *)registry->next_entity_id, &(size_t){component_size});
    ecs_entity_t id = registry->next_entity_id++;

    lua_State *L = registry->L;

    // lua_getglobal(L, "__NEKO_ECS_CORE");  // # 1
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");

    lua_getiuservalue(L, -1, NEKO_ECS_COMPONENTS_NAME);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "comp_map");
        if (lua_istable(L, -1)) {
            lua_pushinteger(L, neko_hash_str(component_name));  // 使用 32 位哈希以适应 Lua 数字范围

            lua_createtable(L, 0, 2);
            lua_pushinteger(L, id);
            lua_setfield(L, -2, "id");
            lua_pushstring(L, component_name);
            lua_setfield(L, -2, "name");

            lua_settable(L, -3);
            lua_pop(L, 1);
        } else {
            NEKO_ERROR("%s", "failed to get comp_map");
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    } else {
        NEKO_ERROR("%s", "failed to get upvalue NEKO_ECS_COMPONENTS_NAME");
        lua_pop(L, 1);
    }
    lua_pop(L, 1);  // pop 1

    return id;
}

#endif
}

//=============================
// NEKO_ENGINE
//=============================

using namespace neko;

s32 random_val(s32 lower, s32 upper) {
    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand() % (upper - lower + 1) + lower);
}

#if 0
namespace neko::ecs_component {

void movement_system(ecs_view_t view, unsigned int row) {

    position_t* p = (position_t*)ecs_view(view, row, 0);
    velocity_t* v = (velocity_t*)ecs_view(view, row, 1);
    bounds_t* b = (bounds_t*)ecs_view(view, row, 2);
    color_t* c = (color_t*)ecs_view(view, row, 3);

    neko_vec2_t min = neko_vec2_add(*p, *v);
    neko_vec2_t max = neko_vec2_add(min, *b);

    auto ws = neko_game().DisplaySize;

    // Resolve collision and change velocity direction if necessary
    if (min.x < 0 || max.x >= ws.x) {
        v->x *= -1.f;
    }
    if (min.y < 0 || max.y >= ws.y) {
        v->y *= -1.f;
    }
    *p = neko_vec2_add(*p, *v);
}

void render_system(ecs_view_t view, unsigned int row) {
    position_t* p = (position_t*)ecs_view(view, row, 0);
    velocity_t* v = (velocity_t*)ecs_view(view, row, 1);
    bounds_t* b = (bounds_t*)ecs_view(view, row, 2);
    color_t* c = (color_t*)ecs_view(view, row, 3);

    auto pi = *p;
    auto bi = *b;

    neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, pi, bi, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), (neko_color_t)*c, R_PRIMITIVE_TRIANGLES);
}

void aseprite_render_system(ecs_view_t view, unsigned int row) { position_t* p = (position_t*)ecs_view(view, row, 0); }

}  // namespace neko::ecs_component

#endif

void neko_register(lua_State *L);

lua_State *neko_scripting_init() {
    neko::timer timer;
    timer.start();

    lua_State *L = neko_lua_create();

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = neko_lua_to<const_str>(L, -1);
                NEKO_ERROR("[lua] panic error: %s", msg);
                return 0;
            });
    neko_register(L);

    timer.stop();
    NEKO_INFO(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());

    return L;
}

void neko_scripting_end(lua_State *L) { neko_lua_fini(L); }

NEKO_API_DECL void neko_default_main_window_close_callback(void *window);

NEKO_API_DECL neko_instance_t *neko_instance() {
    NEKO_STATIC neko_instance_t g_neko_instance = NEKO_DEFAULT_VAL();
    return &g_neko_instance;
}

Neko_OnModuleLoad(Sound);
Neko_OnModuleLoad(Physics);
Neko_OnModuleLoad(Network);

NEKO_API_DECL lua_State *neko_lua_bootstrap(int argc, char **argv) {

    lua_State *L = neko_scripting_init();

    // neko_lua_safe_dofile(L, "startup");

    {
        neko::lua_bind::bind("__neko_file_path", +[](const_str path) -> std::string { return path; });
        lua_newtable(L);
        for (int n = 0; n < argc; ++n) {
            lua_pushstring(L, argv[n]);
            lua_rawseti(L, -2, n);
        }
        lua_setglobal(L, "arg");
    }

    if (argc != 0) {
        auto &module_list = ENGINE_INTERFACE()->modules;

        neko_dynlib m = {};
        s32 sss;
        sss = Neko_OnModuleLoad_Sound(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);
        sss = Neko_OnModuleLoad_Physics(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);
        sss = Neko_OnModuleLoad_Network(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);

        for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
            neko_dynlib &module = module_list[i];
            module.func.OnInit(L);
        }
    } else {
        neko_lua_run_string(L, R"lua(startup = require "startup")lua");
    }

    return L;
}

LUA_FUNCTION(__neko_bind_w_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_NAME::W_CORE);
    return 1;
}

static int __neko_w_lua_get_com(lua_State *L) {
    struct neko_instance_t *w = (neko_instance_t *)luaL_checkudata(L, ECS_WORLD, W_LUA_REGISTRY_NAME::ENG_UDATA_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_UPVAL_N);
    return 2;
}

static int __neko_w_lua_gc(lua_State *L) {
    struct neko_instance_t *w = (neko_instance_t *)luaL_checkudata(L, ECS_WORLD, W_LUA_REGISTRY_NAME::ENG_UDATA_NAME);
    // ecs_fini_i(w);
    NEKO_INFO("neko_instance_t _gc");
    return 0;
}

void neko_w_init() {

    lua_State *L = ENGINE_LUA();

    neko_instance_t *ins = (neko_instance_t *)lua_newuserdatauv(L, sizeof(neko_instance_t), NEKO_W_UPVAL_N);  // # -1
    // ins = neko_instance();

    if (luaL_getmetatable(L, W_LUA_REGISTRY_NAME::ENG_UDATA_NAME) == LUA_TNIL) {  // # -2

        // clang-format off
        luaL_Reg world_mt[] = {
            {"__gc", __neko_w_lua_gc}, 
            {"get_com", __neko_w_lua_get_com}, 
            {NULL, NULL}
        };
        // clang-format on

        lua_pop(L, 1);                  // # pop -2
        luaL_newlibtable(L, world_mt);  // # -2
        luaL_setfuncs(L, world_mt, 0);
        lua_pushvalue(L, -1);                                                     // # -3
        lua_setfield(L, -2, "__index");                                           // pop -3
        lua_pushstring(L, W_LUA_REGISTRY_NAME::ENG_UDATA_NAME);                   // # -3
        lua_setfield(L, -2, "__name");                                            // pop -3
        lua_pushvalue(L, -1);                                                     // # -3
        lua_setfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_NAME::ENG_UDATA_NAME);  // pop -3
    }
    lua_setmetatable(L, -2);  // pop -2

    lua_newtable(L);                                   // # 2
    lua_pushstring(L, W_LUA_REGISTRY_NAME::CVAR_MAP);  // # 3
    lua_createtable(L, 0, ENTITY_MAX_COMPONENTS);      // # 4
    lua_settable(L, -3);
    lua_setiuservalue(L, -2, NEKO_W_COMPONENTS_NAME);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, -2, NEKO_W_UPVAL_N);

    // lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_NAME::W_CORE);

    NEKO_ASSERT(lua_gettop(L) == 0);

    lua_register(L, "neko_w_f", __neko_bind_w_f);

    neko_w_lua_variant aaa("hahah641", 103.f);
    neko_w_lua_variant bbb("hah3211", "hashi");

    NEKO_ASSERT(lua_gettop(L) == 0);
}

NEKO_API_DECL neko_instance_t *neko_create(int argc, char **argv) {
    if (neko_instance() != NULL) {

#ifndef NDEBUG
        g_allocator = new DebugAllocator();
#else
        g_allocator = new HeapAllocator();
#endif

        g_allocator->make();

        neko_tm_init();

        profile_setup();
        PROFILE_FUNC();

        neko_instance()->console = &g_console;

        NEKO_INFO("neko engine build %d", neko_buildnum());

        {

            // 初始化应用程序并设置为运行
            mount_result mount;

#if defined(NEKO_DEBUG_BUILD)
            mount = neko::vfs_mount(NEKO_PACKS::GAMEDATA, "gamedir/../");
            mount = neko::vfs_mount(NEKO_PACKS::LUACODE, "gamedir/../");
#else
            mount = neko::vfs_mount(NEKO_PACKS::GAMEDATA, "gamedir/../gamedata.zip");
            mount = neko::vfs_mount(neko_pack_list::luacode, "gamedir/../luacode.zip");
#endif

            ENGINE_LUA() = neko_lua_bootstrap(argc, argv);

            neko_w_init();

            lua_channels_setup();

            ENGINE_ECS() = ecs_init();

            ECS_IMPORT(ENGINE_ECS(), FlecsLua);

            ecs_lua_set_state(ENGINE_ECS(), ENGINE_LUA());
        }

        // 初始化 cvars
        neko_w_lua_variant settings_window_width("settings.window.width", 1280.f);
        neko_w_lua_variant settings_window_height("settings.window.height", 720.f);
        neko_w_lua_variant settings_window_vsync("settings.window.vsync", false);
        neko_w_lua_variant settings_window_frame_rate("settings.window.frame_rate", 60.f);
        neko_w_lua_variant settings_window_hdpi("settings.window.hdpi", false);
        neko_w_lua_variant settings_window_center("settings.window.center", true);
        neko_w_lua_variant settings_window_running_background("settings.window.running_background", true);
        neko_w_lua_variant settings_window_monitor_index("settings.window.monitor_index", 0.f);
        neko_w_lua_variant settings_video_render_debug("settings.video.render.debug", 0.f);
        neko_w_lua_variant settings_video_render_hdpi("settings.video.render.hdpi", false);

        // 需要从用户那里传递视频设置
        neko_subsystem(platform) = neko_pf_create();

        // 此处平台的默认初始化
        neko_pf_init(neko_subsystem(platform));

        // 设置应用程序的帧速率
        neko_subsystem(platform)->time.max_fps = settings_window_frame_rate.get<f32>();

        neko_pf_running_desc_t window = {.title = "Neko Engine", .width = 1280, .height = 720, .vsync = false, .frame_rate = 60.f, .hdpi = false, .center = true, .running_background = true};

        // 构建主窗口
        neko_pf_window_create(&window);

        // 设置视频垂直同步
        neko_pf_enable_vsync(settings_window_vsync.get<bool>());

        // 构建图形API
        neko_subsystem(render) = neko_render_create();

        // 初始化图形
        neko_render_init(neko_subsystem(render));

        neko_module_interface_init(ENGINE_INTERFACE());

        {
            u32 w, h;
            u32 display_w, display_h;

            neko_pf_window_size(neko_pf_main_window(), &w, &h);
            neko_pf_framebuffer_size(neko_pf_main_window(), &display_w, &display_h);

            neko_game().DisplaySize = neko_v2((float)w, (float)h);
            neko_game().DisplayFramebufferScale = neko_v2((float)display_w / w, (float)display_h / h);
        }

#if 0
        {

            ENGINE_INTERFACE()->ecs = ecs_init(ENGINE_L());

            auto registry = ENGINE_INTERFACE()->ecs;
            const ecs_entity_t pos_component = ECS_COMPONENT(registry, position_t);
            const ecs_entity_t vel_component = ECS_COMPONENT(registry, velocity_t);
            const ecs_entity_t bou_component = ECS_COMPONENT(registry, bounds_t);
            const ecs_entity_t col_component = ECS_COMPONENT(registry, color_t);
            const ecs_entity_t obj_component = ECS_COMPONENT(registry, CGameObjectTest);

            for (int i = 0; i < 5; i++) {

                neko_vec2_t bounds = neko_v2((f32)random_val(10, 100), (f32)random_val(10, 100));

                position_t p = {(f32)random_val(0, (int32_t)neko_game().DisplaySize.x - (int32_t)bounds.x), (f32)random_val(0, (int32_t)neko_game().DisplaySize.y - (int32_t)bounds.y)};
                velocity_t v = {(f32)random_val(1, 10), (f32)random_val(1, 10)};
                bounds_t b = {(f32)random_val(10, 100), (f32)random_val(10, 100)};
                color_t c = {(u8)random_val(50, 255), (u8)random_val(50, 255), (u8)random_val(50, 255), 255};
                CGameObjectTest gameobj = NEKO_DEFAULT_VAL();

                neko_snprintf(gameobj.name, 64, "%s_%d", "Test_ent", i);
                gameobj.visible = true;
                gameobj.active = false;

                ecs_entity_t e = ecs_entity(registry);

                ecs_attach(registry, e, pos_component);
                ecs_attach(registry, e, vel_component);
                ecs_attach(registry, e, bou_component);
                ecs_attach(registry, e, col_component);
                ecs_attach(registry, e, obj_component);

                ecs_set(registry, e, pos_component, &p);
                ecs_set(registry, e, vel_component, &v);
                ecs_set(registry, e, bou_component, &b);
                ecs_set(registry, e, col_component, &c);
                ecs_set(registry, e, obj_component, &gameobj);
            }

            ECS_SYSTEM(registry, neko::ecs_component::movement_system, 4, pos_component, vel_component, bou_component, col_component);
            ECS_SYSTEM(registry, neko::ecs_component::render_system, 4, pos_component, vel_component, bou_component, col_component);

            // const ecs_entity_t test_component = ECS_COMPONENT(registry, uptr);
            // for (int i = 0; i < 1024; i++) {
            //     ecs_entity_t e = ecs_entity(registry);
            //     ecs_attach(registry, e, test_component);
            // }
        }
#endif

        neko_app(ENGINE_LUA());

        // neko_lua_safe_dofile(ENGINE_L(), "boot");

        std::string boot_code = R"lua(
startup = require "startup"

-- local w = neko.ecs_f()
-- local e = w:create_ent()
-- w:attach(e, "neko_client_userdata_t")
-- set_client_init(e)
-- print(e)

local w = boot.fetch_world("sandbox")

function boot_init()
    local game_userdata = sandbox_init()
    w:register("neko_client_userdata_t", {ud})
    local eid1 = w:new{
        neko_client_userdata_t = {
            ud = game_userdata
        }
    }
end

function boot_update()
    for cl in w:match("all", "neko_client_userdata_t") do
        sandbox_update(cl.ud)
    end
    w:update()
end

function boot_fini()
    for cl in w:match("all", "neko_client_userdata_t") do
        sandbox_fini(cl.ud)
    end
end
)lua";

        neko_lua_run_string(ENGINE_LUA(), boot_code.c_str());

        neko_lua_call(ENGINE_LUA(), "boot_init");
        neko_instance()->game.is_running = true;

        // 设置按下主窗口关闭按钮时的默认回调
        neko_pf_set_window_close_callback(neko_pf_main_window(), &neko_default_main_window_close_callback);
    }

    return neko_instance();
}

// 主框架函数
NEKO_API_DECL void neko_frame() {

    PROFILE_FUNC();

    {
        PROFILE_BLOCK("main_loop");

        //        uintptr_t profile_id_engine;
        //        profile_id_engine = neko_profiler_begin_scope(__FILE__, __LINE__, "engine");

        neko_pf_t *platform = neko_subsystem(platform);

        neko_pf_window_t *win = (neko_slot_array_getp(platform->windows, neko_pf_main_window()));

        // 帧开始时的缓存时间
        platform->time.elapsed = (f32)neko_pf_elapsed_time();
        platform->time.update = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;

        // 更新平台和流程输入
        neko_pf_update(platform);

        if (win->focus /*|| neko_instance()->game.window.running_background*/) {

            PROFILE_BLOCK("main_update");

            neko_lua_call(ENGINE_LUA(), "boot_update");

            // neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
            // neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, neko_game().DisplaySize.x, neko_game().DisplaySize.y);
            // ecs_step(ENGINE_INTERFACE()->ecs);

            // neko_instance()->post_update();
            {
                PROFILE_BLOCK("cmd_submit");
                neko_render_command_buffer_submit(&ENGINE_INTERFACE()->cb);
                neko_check_gl_error();
            }

            auto &module_list = ENGINE_INTERFACE()->modules;
            for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
                neko_dynlib &module = module_list[i];
                module.func.OnPostUpdate(ENGINE_LUA());
            }
        }

        // 清除所有平台事件
        neko_dyn_array_clear(platform->events);

        {
            PROFILE_BLOCK("swap_buffer");
            for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
                neko_pf_window_swap_buffer(it);
            }
            // neko_pf_window_swap_buffer(neko_pf_main_window());
        }

        // 帧锁定
        platform->time.elapsed = (f32)neko_pf_elapsed_time();
        platform->time.render = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;
        platform->time.frame = platform->time.update + platform->time.render;  // 总帧时间
        platform->time.delta = platform->time.frame / 1000.f;

        f32 target = (1000.f / platform->time.max_fps);

        if (platform->time.frame < target) {
            PROFILE_BLOCK("wait");
            neko_pf_sleep((f32)(target - platform->time.frame));
            platform->time.elapsed = (f32)neko_pf_elapsed_time();
            double wait_time = platform->time.elapsed - platform->time.previous;
            platform->time.previous = platform->time.elapsed;
            platform->time.frame += wait_time;
            platform->time.delta = platform->time.frame / 1000.f;
        }

        //        neko_profiler_end_scope(profile_id_engine);
    }

    if (!neko_instance()->game.is_running) {
        neko_fini();
        return;
    }
}

void neko_fini() {
    PROFILE_FUNC();

    // Shutdown application
    neko_lua_call(ENGINE_LUA(), "boot_fini");
    neko_instance()->game.is_running = false;

    lua_channels_shutdown();

    auto &module_list = ENGINE_INTERFACE()->modules;
    for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
        neko_dynlib &module = module_list[i];
        module.func.OnFini(ENGINE_LUA());
    }

    neko_module_interface_fini(ENGINE_INTERFACE());

    neko_scripting_end(ENGINE_LUA());

    neko::vfs_fini({});

    neko_render_shutdown(neko_subsystem(render));
    neko_render_destroy(neko_subsystem(render));

    neko_pf_shutdown(neko_subsystem(platform));
    neko_pf_destroy(neko_subsystem(platform));

    // __neko_config_free();

    profile_fini();

    // 在 app 结束后进行内存检查
#ifndef NDEBUG
    DebugAllocator *allocator = dynamic_cast<DebugAllocator *>(g_allocator);
    if (allocator != nullptr) {
        allocator->dump_allocs();
    }
#endif

    allocator->trash();
    operator delete(g_allocator);
}

NEKO_API_DECL void neko_default_main_window_close_callback(void *window) { neko_instance()->game.is_running = false; }

void neko_quit() {
#ifndef NEKO_PF_WEB
    neko_instance()->game.is_running = false;
#endif
}

void neko_module_interface_init(Neko_ModuleInterface *module_interface) {

    // module_interface->common.__neko_mem_safe_alloc = +[](size_t size, const char *file, int line) { return g_allocator->alloc(size, file, line); };
    // module_interface->common.__neko_mem_safe_free = +[](void *ptr) { g_allocator->free(ptr); };
    // module_interface->common.__neko_mem_safe_calloc = &__neko_mem_safe_calloc;
    // module_interface->common.__neko_mem_safe_realloc = &__neko_mem_safe_realloc;
    // module_interface->common.capi_vfs_read_file = &neko_capi_vfs_read_file;

    module_interface->cb = neko_command_buffer_new();
    module_interface->idraw = neko_immediate_draw_new();
    module_interface->am = neko_asset_manager_new();
}

void neko_module_interface_fini(Neko_ModuleInterface *module_interface) {

    neko_immediate_draw_free(&module_interface->idraw);
    neko_command_buffer_free(&module_interface->cb);
    neko_asset_manager_free(&module_interface->am);
}

//=============================
// MAIN
//=============================

s32 main(s32 argv, char **argc) {
    neko_instance_t *inst = neko_create(argv, argc);
    while (neko_instance()->game.is_running) {
        neko_frame();
    }
    return 0;
}