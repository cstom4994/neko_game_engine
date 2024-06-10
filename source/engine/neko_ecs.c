
#include "neko_ecs.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko.h"
#include "engine/neko_lua.h"

/*=============================
// ECS
=============================*/

#define OUT_OF_MEMORY "out of memory"
#define OUT_OF_BOUNDS "index out of bounds"
#define FAILED_LOOKUP "lookup failed and returned null"
#define NOT_IMPLEMENTED "not implemented"
#define SOMETHING_TERRIBLE "something went terribly wrong"

#define ECS_ABORT(error)                                            \
    fprintf(stderr, "ABORT %s:%d %s\n", __FILE__, __LINE__, error); \
    abort();

#define ECS_ENSURE(cond, error)                            \
    if (!(cond)) {                                         \
        fprintf(stderr, "condition not met: %s\n", #cond); \
        ECS_ABORT(error);                                  \
    }

#ifndef NDEBUG
#define ECS_ASSERT(cond, error) ECS_ENSURE(cond, error);
#else
#define ECS_ASSERT(cond, error)
#endif

#define ECS_OFFSET(p, offset) ((void *)(((char *)(p)) + (offset)))

#define ECS_ARCHETYPE_INITIAL_CAPACITY 64

static inline void *ecs_malloc(size_t bytes) {
    void *mem = neko_safe_malloc(bytes);
    ECS_ENSURE(mem != NULL, OUT_OF_MEMORY);
    return mem;
}

static inline void *ecs_calloc(size_t items, size_t bytes) {
    void *mem = neko_safe_calloc(items, bytes);
    ECS_ENSURE(mem != NULL, OUT_OF_MEMORY);
    return mem;
}

static inline void ecs_realloc(void **mem, size_t bytes) {
    *mem = neko_safe_realloc(*mem, bytes);
    if (bytes != 0) {
        ECS_ENSURE(*mem != NULL, OUT_OF_MEMORY);
    }
}

static inline void ecs_free(void *mem) { neko_safe_free(mem); }

#define MAP_LOAD_FACTOR 0.5
#define MAP_COLLISION_THRESHOLD 30
#define MAP_TOMESTONE ((u32) - 1)

typedef struct ecs_bucket_t {
    const void *key;
    u32 index;
} ecs_bucket_t;

struct ecs_map_t {
    ecs_hash_fn hash;
    ecs_key_equal_fn key_equal;
    size_t key_size;
    size_t item_size;
    u32 count;
    u32 load_capacity;
    ecs_bucket_t *sparse;
    u32 *reverse_lookup;
    void *dense;
};

struct ecs_type_t {
    u32 capacity;
    u32 count;
    ecs_entity_t *elements;
};

struct ecs_signature_t {
    u32 count;
    ecs_entity_t components[];
};

typedef struct ecs_system_t {
    ecs_archetype_t *archetype;
    ecs_signature_t *sig;
    ecs_system_fn run;
} ecs_system_t;

struct ecs_edge_t {
    ecs_entity_t component;
    ecs_archetype_t *archetype;
};

struct ecs_edge_list_t {
    u32 capacity;
    u32 count;
    ecs_edge_t *edges;
};

typedef struct ecs_record_t {
    ecs_archetype_t *archetype;
    u32 row;
} ecs_record_t;

struct ecs_archetype_t {
    u32 capacity;
    u32 count;
    ecs_type_t *type;
    ecs_entity_t *entity_ids;
    void **components;
    ecs_edge_list_t *left_edges;
    ecs_edge_list_t *right_edges;
};

struct neko_ecs_t {
    ecs_map_t *entity_index;     // <ecs_entity_t, ecs_record_t>
    ecs_map_t *component_index;  // <ecs_entity_t, size_t>
    ecs_map_t *system_index;     // <ecs_entity_t, ecs_system_t>
    ecs_map_t *type_index;       // <ecs_type_t *, ecs_archetype_t *>
    ecs_archetype_t *root;
    ecs_entity_t next_entity_id;

    lua_State *L;
};

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
    struct neko_ecs_world_t *w;
    int i;
    int kn;
    int keys[ENTITY_MAX_COMPONENTS];
};

struct neko_ecs_world_t {
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

static inline int component_add(lua_State *L, struct neko_ecs_world_t *w, struct entity *e, int tid) {
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
        cp->buf = neko_safe_realloc(cp->buf, cp->cap * sizeof(cp->buf[0]));
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

static inline void component_dead(struct neko_ecs_world_t *w, int tid, int cid) {
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

static inline void component_dirty(struct neko_ecs_world_t *w, int tid, int cid) {
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

static inline struct entity *entity_alloc(struct neko_ecs_world_t *w) {
    struct entity *e;
    int eid = w->entity_free;
    if (eid < 0) {
        int i = 0;
        int oldcap = w->entity_cap;
        int newcap = oldcap * 2;
        w->entity_cap = newcap;
        w->entity_buf = neko_safe_realloc(w->entity_buf, newcap * sizeof(w->entity_buf[0]));
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

static inline void entity_dead(struct neko_ecs_world_t *w, struct entity *e) {
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

static inline void entity_free(struct neko_ecs_world_t *w, struct entity *e) {
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

static inline struct entity *get_entity(lua_State *L, struct neko_ecs_world_t *w, int stk) {
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

static int __neko_ecs_world_end(lua_State *L) {
    struct neko_ecs_world_t *w;
    int type, tid;
    struct component_pool *cp;
    w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    tid = w->type_idx;  // 总数-1(索引)
    for (int i = 0; i <= tid; i++) {
        cp = &w->component_pool[i];
        neko_safe_free(cp->buf);
    }
    neko_safe_free(w->entity_buf);
    return 0;
}

static int __neko_ecs_world_register(lua_State *L) {
    struct neko_ecs_world_t *w;
    int type, tid;
    struct component_pool *cp;
    w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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
    cp->buf = neko_safe_malloc(cp->cap * sizeof(cp->buf[0]));
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

static int __neko_ecs_world_new_entity(lua_State *L) {
    int i, components, proto_id;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_del_entity(lua_State *L) {
    int i;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
    entity_dead(w, e);
    return 0;
}

static int __neko_ecs_world_get_component(lua_State *L) {
    int i, top, proto_id, components;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_add_component(lua_State *L) {
    int tid, cid;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_remove_component(lua_State *L) {
    int i, proto_id;
    int tid, cid;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_touch_component(lua_State *L) {
    int eid, tid, cid;
    struct entity *e;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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
    struct match_ctx *mctx = lua_touserdata(L, 1);
    struct neko_ecs_world_t *w = mctx->w;
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
    struct match_ctx *mctx = lua_touserdata(L, 1);
    struct neko_ecs_world_t *w = mctx->w;
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
    struct match_ctx *mctx = lua_touserdata(L, 1);
    struct neko_ecs_world_t *w = mctx->w;
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

static int __neko_ecs_world_match_component(lua_State *L) {
    int i;
    size_t sz;
    struct neko_ecs_world_t *w;
    enum match_mode mode;
    int (*iter)(lua_State *L) = NULL;
    struct match_ctx *mctx = NULL;
    const char *m = luaL_checklstring(L, 2, &sz);
    int top = lua_gettop(L);
    w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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
    mctx = lua_touserdata(L, -1);
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

static int __neko_ecs_world_update(lua_State *L) {
    int next;
    int t, components;
    struct component_pool *pool;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
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

static int __neko_ecs_world_dump(lua_State *L) {
    int i;
    for (i = 1; i <= WORLD_COMPONENTS; i++) dump_upval(L, i);
    return 0;
}

int __neko_ecs_create_world(lua_State *L) {
    int i;
    struct neko_ecs_world_t *w;
    struct match_ctx *mctx;
    w = (struct neko_ecs_world_t *)lua_newuserdatauv(L, sizeof(*w), WORLD_UPVAL_N);
    memset(w, 0, sizeof(*w));
    w->entity_cap = 128;
    w->entity_free = 0;
    w->entity_dead = LINK_NIL;
    w->type_idx = 0;
    w->entity_buf = neko_safe_malloc(w->entity_cap * sizeof(w->entity_buf[0]));
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

static int __neko_ecs_lua_create_ent(lua_State *L) {
    struct neko_ecs_t *w = (struct neko_ecs_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_entity_t e = ecs_entity(w);
    lua_pushinteger(L, e);
    return 1;
}

const ecs_entity_t __neko_ecs_lua_component_id_w(lua_State *L, const_str component_name) {
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

static int __neko_ecs_lua_attach(lua_State *L) {
    int n = lua_gettop(L);
    luaL_argcheck(L, n >= 3, 1, "lost the component name");

    struct neko_ecs_t *w = (struct neko_ecs_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_entity_t e = luaL_checkinteger(L, 2);

    for (int i = 3; i <= n; i++) {
        if (lua_isstring(L, i)) {
            const_str component_name = lua_tostring(L, i);
            const ecs_entity_t c = __neko_ecs_lua_component_id_w(L, component_name);
            ecs_attach(w, e, c);
        } else {
            NEKO_WARN("argument %d is not a string", i);
        }
    }

    return 0;
}

static int __neko_ecs_lua_get_com(lua_State *L) {
    struct neko_ecs_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);

    lua_getiuservalue(L, 1, NEKO_ECS_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_UPVAL_N);

    return 2;
}

static int __neko_ecs_lua_gc(lua_State *L) {
    struct neko_ecs_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_fini_i(w);
    NEKO_INFO("ecs_lua_gc");
    return 0;
}

ecs_map_t *ecs_map_new(size_t key_size, size_t item_size, ecs_hash_fn hash_fn, ecs_key_equal_fn key_equal_fn, u32 capacity) {
    ecs_map_t *map = ecs_malloc(sizeof(ecs_map_t));
    map->hash = hash_fn;
    map->key_equal = key_equal_fn;
    map->sparse = ecs_calloc(sizeof(ecs_bucket_t), capacity);
    map->reverse_lookup = ecs_malloc(sizeof(u32) * (capacity * MAP_LOAD_FACTOR + 1));
    map->dense = ecs_malloc(item_size * (capacity * MAP_LOAD_FACTOR + 1));
    map->key_size = key_size;
    map->item_size = item_size;
    map->load_capacity = capacity;
    map->count = 0;
    return map;
}

void ecs_map_free(ecs_map_t *map) {
    ecs_free(map->sparse);
    ecs_free(map->reverse_lookup);
    ecs_free(map->dense);
    ecs_free(map);
}

static inline u32 next_pow_of_2(u32 n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

void *ecs_map_get(const ecs_map_t *map, const void *key) {
    u32 i = map->hash(key);
    ecs_bucket_t bucket = map->sparse[i % map->load_capacity];
    u32 collisions = 0;

    while (bucket.index != 0) {
        if (map->key_equal(bucket.key, key) && bucket.index != MAP_TOMESTONE) {
            break;
        }

        i += next_pow_of_2(collisions++);
        bucket = map->sparse[i % map->load_capacity];
    }

    if (bucket.index == 0 || bucket.index == MAP_TOMESTONE) {
        return NULL;
    }

    return ECS_OFFSET(map->dense, map->item_size * bucket.index);
}

static void ecs_map_grow(ecs_map_t *map, float growth_factor) {
    u32 new_capacity = map->load_capacity * growth_factor;
    ecs_bucket_t *new_sparse = ecs_calloc(sizeof(ecs_bucket_t), new_capacity);
    ecs_free(map->reverse_lookup);
    map->reverse_lookup = ecs_malloc(sizeof(u32) * (new_capacity * MAP_LOAD_FACTOR + 1));
    ecs_realloc(&map->dense, map->item_size * (new_capacity * MAP_LOAD_FACTOR + 1));

    for (u32 i = 0; i < map->load_capacity; i++) {
        ecs_bucket_t bucket = map->sparse[i];

        if (bucket.index != 0 && bucket.index != MAP_TOMESTONE) {
            u32 hashed = map->hash(bucket.key);
            ecs_bucket_t *other = &new_sparse[hashed % new_capacity];
            u32 collisions = 0;

            while (other->index != 0) {
                hashed += next_pow_of_2(collisions++);
                other = &new_sparse[hashed % new_capacity];
            }

            other->key = bucket.key;
            other->index = bucket.index;
            map->reverse_lookup[bucket.index] = hashed % new_capacity;
        }
    }

    ecs_free(map->sparse);
    map->sparse = new_sparse;
    map->load_capacity = new_capacity;
}

void ecs_map_set(ecs_map_t *map, const void *key, const void *payload) {
    u32 i = map->hash(key);
    ecs_bucket_t *bucket = &map->sparse[i % map->load_capacity];
    u32 collisions = 0;
    ecs_bucket_t *first_tomestone = NULL;

    while (bucket->index != 0) {
        if (map->key_equal(bucket->key, key) && bucket->index != MAP_TOMESTONE) {
            void *loc = ECS_OFFSET(map->dense, map->item_size * bucket->index);
            memcpy(loc, payload, map->item_size);
            return;
        }

        if (!first_tomestone && bucket->index == MAP_TOMESTONE) {
            first_tomestone = bucket;
        }

        i += next_pow_of_2(collisions++);
        ECS_ASSERT(collisions < MAP_COLLISION_THRESHOLD, "too many hash collisions");
        bucket = &map->sparse[i % map->load_capacity];
    }

    if (first_tomestone) {
        bucket = first_tomestone;
    }

    bucket->key = key;
    bucket->index = map->count + 1;
    void *loc = ECS_OFFSET(map->dense, map->item_size * bucket->index);
    memcpy(loc, payload, map->item_size);
    map->reverse_lookup[bucket->index] = i % map->load_capacity;
    map->count++;

    if (map->count >= map->load_capacity * MAP_LOAD_FACTOR) {
        ecs_map_grow(map, 2);
    }
}

void ecs_map_remove(ecs_map_t *map, const void *key) {
    u32 i = map->hash(key);
    ecs_bucket_t bucket = map->sparse[i % map->load_capacity];
    u32 next = 0;

    while (bucket.index != 0) {
        if (map->key_equal(bucket.key, key) && bucket.index != MAP_TOMESTONE) {
            break;
        }

        i += next_pow_of_2(next++);
        bucket = map->sparse[i % map->load_capacity];
    }

    if (bucket.index == 0 || bucket.index == MAP_TOMESTONE) {
        return;
    }

    char **tmp = alloca(sizeof(char *) * map->item_size);
    void *left = ECS_OFFSET(map->dense, map->item_size * bucket.index);
    void *right = ECS_OFFSET(map->dense, map->item_size * map->count);
    memcpy(tmp, left, map->item_size);
    memcpy(left, right, map->item_size);
    memcpy(right, tmp, map->item_size);

    map->sparse[map->reverse_lookup[map->count]].index = bucket.index;
    map->sparse[map->reverse_lookup[bucket.index]].index = MAP_TOMESTONE;

    u32 reverse_tmp = map->reverse_lookup[bucket.index];
    map->reverse_lookup[bucket.index] = map->reverse_lookup[map->count];
    map->reverse_lookup[map->count] = reverse_tmp;

    map->count--;
}

void *ecs_map_values(ecs_map_t *map) { return ECS_OFFSET(map->dense, map->item_size); }

u32 ecs_map_len(ecs_map_t *map) { return map->count; }

u32 ecs_map_hash_intptr(const void *key) {
    uintptr_t hashed = (uintptr_t)key;
    hashed = ((hashed >> 16) ^ hashed) * 0x45d9f3b;
    hashed = ((hashed >> 16) ^ hashed) * 0x45d9f3b;
    hashed = (hashed >> 16) ^ hashed;
    return hashed;
}

u32 ecs_map_hash_string(const void *key) {
    char *str = (char *)key;
    u32 hash = 5381;
    char c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

u32 ecs_map_hash_type(const void *key) {
    ecs_type_t *type = (ecs_type_t *)key;
    u32 hash = 5381;
    ECS_TYPE_EACH(type, e, { hash = ((hash << 5) + hash) + e; });
    return hash;
}

bool ecs_map_equal_intptr(const void *a, const void *b) { return a == b; }

bool ecs_map_equal_string(const void *a, const void *b) { return strcmp(a, b) == 0; }

bool ecs_map_equal_type(const void *a, const void *b) { return ecs_type_equal((ecs_type_t *)a, (ecs_type_t *)b); }

#ifndef NDEBUG
void ecs_map_inspect(ecs_map_t *map) {
    printf("\nmap: {\n"
           "  item_size: %lld bytes\n"
           "  count: %d items\n"
           "  load_capacity: %d\n",
           map->item_size, map->count, map->load_capacity);

    printf("  sparse: [\n");
    for (u32 i = 0; i < map->load_capacity; i++) {
        ecs_bucket_t bucket = map->sparse[i];
        printf("    %d: { key: %llu, index: %d }\n", i, (uintptr_t)bucket.key, bucket.index);
    }
    printf("  ]\n");

    printf("  dense: [\n");
    for (u32 i = 0; i < map->load_capacity * MAP_LOAD_FACTOR + 1; i++) {
        if (i == map->count + 1) {
            printf("    -- end of load --\n");
        }

        int item = *(int *)ECS_OFFSET(map->dense, map->item_size * i);
        printf("    %d: %d\n", i, item);
    }
    printf("  ]\n");

    printf("  reverse_lookup: [\n");
    for (u32 i = 0; i < map->load_capacity * MAP_LOAD_FACTOR + 1; i++) {
        if (i == map->count + 1) {
            printf("    -- end of load --\n");
        }

        printf("    %d: %d\n", i, map->reverse_lookup[i]);
    }
    printf("  ]\n");

    printf("}\n");
}
#endif  // NDEBUG

ecs_type_t *ecs_type_new(u32 capacity) {
    ecs_type_t *type = ecs_malloc(sizeof(ecs_type_t));
    type->elements = ecs_malloc(sizeof(ecs_entity_t) * capacity);
    type->capacity = capacity;
    type->count = 0;
    return type;
}

void ecs_type_free(ecs_type_t *type) {
    ecs_free(type->elements);
    ecs_free(type);
}

ecs_type_t *ecs_type_copy(const ecs_type_t *from) {
    ecs_type_t *type = ecs_malloc(sizeof(ecs_type_t));
    type->elements = ecs_malloc(sizeof(ecs_entity_t) * from->capacity);
    type->capacity = from->capacity;
    type->count = from->count;
    memcpy(type->elements, from->elements, sizeof(ecs_entity_t) * from->count);
    return type;
}

u32 ecs_type_len(const ecs_type_t *type) { return type->count; }

bool ecs_type_equal(const ecs_type_t *a, const ecs_type_t *b) {
    if (a == b) {
        return true;
    }

    if (a->count != b->count) {
        return false;
    }

    for (u32 i = 0; i < a->count; i++) {
        if (a->elements[i] != b->elements[i]) {
            return false;
        }
    }

    return true;
}

s32 ecs_type_index_of(const ecs_type_t *type, ecs_entity_t e) {
    for (u32 i = 0; i < type->count; i++) {
        if (type->elements[i] == e) {
            return i;
        }
    }

    return -1;
}

void ecs_type_add(ecs_type_t *type, ecs_entity_t e) {
    if (type->count == type->capacity) {
        if (type->capacity == 0) {
            type->capacity = 1;
        }

        const u32 growth = 2;
        ecs_realloc((void **)&type->elements, sizeof(ecs_entity_t) * type->capacity * growth);
        type->capacity *= growth;
    }

    u32 i = 0;
    while (i < type->count && type->elements[i] < e) {
        i++;
    }

    if (i < type->count && type->elements[i] == e) {
        return;
    }

    ecs_entity_t held = e;
    ecs_entity_t tmp;
    while (i < type->count) {
        tmp = type->elements[i];
        type->elements[i] = held;
        held = tmp;
        i++;
    }

    type->elements[i] = held;
    type->count++;
}

void ecs_type_remove(ecs_type_t *type, ecs_entity_t e) {
    u32 i = 0;
    while (i < type->count && type->elements[i] < e) {
        i++;
    }

    if (i == type->count || type->elements[i] != e) {
        return;
    }

    ECS_ASSERT(i < type->count, OUT_OF_BOUNDS);

    while (i < type->count - 1) {
        type->elements[i] = type->elements[i + 1];
        i++;
    }

    type->count--;
}

bool ecs_type_is_superset(const ecs_type_t *super, const ecs_type_t *sub) {
    u32 left = 0, right = 0;
    u32 super_len = ecs_type_len(super);
    u32 sub_len = ecs_type_len(sub);

    if (super_len < sub_len) {
        return false;
    }

    while (left < super_len && right < sub_len) {
        if (super->elements[left] < sub->elements[right]) {
            left++;
        } else if (super->elements[left] == sub->elements[right]) {
            left++;
            right++;
        } else {
            return false;
        }
    }

    return right == sub_len;
}

#ifndef NDEBUG
void ecs_type_inspect(ecs_type_t *type) {
    printf("\ntype: {\n");
    printf("  capacity: %d\n", type->capacity);
    printf("  count: %d\n", type->count);

    printf("  elements: [\n");
    for (u32 i = 0; i < type->count; i++) {
        printf("    %d: %llu\n", i, type->elements[i]);
    }
    printf("  ]\n");

    printf("}\n");
}
#endif

ecs_signature_t *ecs_signature_new(u32 count) {
    ecs_signature_t *sig = ecs_malloc(sizeof(ecs_signature_t) + (sizeof(ecs_entity_t) * count));
    sig->count = 0;
    return sig;
}

ecs_signature_t *ecs_signature_new_n(u32 count, ...) {
    ecs_signature_t *sig = ecs_signature_new(count);
    sig->count = count;

    va_list args;
    va_start(args, count);

    for (u32 i = 0; i < count; i++) {
        sig->components[i] = va_arg(args, ecs_entity_t);
    }

    va_end(args);

    return sig;
}

void ecs_signature_free(ecs_signature_t *sig) { ecs_free(sig); }

ecs_type_t *ecs_signature_as_type(const ecs_signature_t *sig) {
    ecs_type_t *type = ecs_type_new(sig->count);

    for (u32 i = 0; i < sig->count; i++) {
        ecs_type_add(type, sig->components[i]);
    }

    return type;
}

ecs_edge_list_t *ecs_edge_list_new(void) {
    ecs_edge_list_t *edge_list = ecs_malloc(sizeof(ecs_edge_list_t));
    edge_list->capacity = 8;
    edge_list->count = 0;
    edge_list->edges = ecs_malloc(sizeof(ecs_edge_t) * edge_list->capacity);
    return edge_list;
}

void ecs_edge_list_free(ecs_edge_list_t *edge_list) {
    ecs_free(edge_list->edges);
    ecs_free(edge_list);
}

u32 ecs_edge_list_len(const ecs_edge_list_t *edge_list) { return edge_list->count; }

void ecs_edge_list_add(ecs_edge_list_t *edge_list, ecs_edge_t edge) {
    if (edge_list->count == edge_list->capacity) {
        const u32 growth = 2;
        ecs_realloc((void **)&edge_list->edges, sizeof(ecs_edge_t) * edge_list->capacity * growth);
    }

    edge_list->edges[edge_list->count++] = edge;
}

void ecs_edge_list_remove(ecs_edge_list_t *edge_list, ecs_entity_t component) {
    ecs_edge_t *edges = edge_list->edges;

    u32 i = 0;
    while (i < edge_list->count && edges[i].component != component) {
        i++;
    }

    if (i == edge_list->count) {
        return;
    }

    ecs_edge_t tmp = edges[i];
    edges[i] = edges[edge_list->count];
    edges[edge_list->count--] = tmp;
}

static void ecs_archetype_resize_component_array(ecs_archetype_t *archetype, const ecs_map_t *component_index, u32 capacity) {
    u32 i = 0;
    ECS_TYPE_EACH(archetype->type, e, {
        size_t *component_size = ecs_map_get(component_index, (void *)e);
        ECS_ASSERT(component_size != NULL, FAILED_LOOKUP);
        ecs_realloc(&archetype->components[i], sizeof(*component_size) * capacity);
        archetype->capacity = capacity;
        i++;
    });
}

ecs_archetype_t *ecs_archetype_new(ecs_type_t *type, const ecs_map_t *component_index, ecs_map_t *type_index) {
    ECS_ENSURE(ecs_map_get(type_index, type) == NULL, "archetype already exists");

    ecs_archetype_t *archetype = ecs_malloc(sizeof(ecs_archetype_t));

    archetype->capacity = ECS_ARCHETYPE_INITIAL_CAPACITY;
    archetype->count = 0;
    archetype->type = type;
    archetype->entity_ids = ecs_malloc(sizeof(ecs_entity_t) * ECS_ARCHETYPE_INITIAL_CAPACITY);
    archetype->components = ecs_calloc(sizeof(void *), ecs_type_len(type));
    archetype->left_edges = ecs_edge_list_new();
    archetype->right_edges = ecs_edge_list_new();

    ecs_archetype_resize_component_array(archetype, component_index, ECS_ARCHETYPE_INITIAL_CAPACITY);
    ecs_map_set(type_index, type, &archetype);

    return archetype;
}

void ecs_archetype_free(ecs_archetype_t *archetype) {
    u32 component_count = ecs_type_len(archetype->type);
    for (u32 i = 0; i < component_count; i++) {
        ecs_free(archetype->components[i]);
    }
    ecs_free(archetype->components);

    ecs_type_free(archetype->type);
    ecs_edge_list_free(archetype->left_edges);
    ecs_edge_list_free(archetype->right_edges);
    ecs_free(archetype->entity_ids);
    ecs_free(archetype);
}

u32 ecs_archetype_add(ecs_archetype_t *archetype, const ecs_map_t *component_index, ecs_map_t *entity_index, ecs_entity_t e) {
    if (archetype->count == archetype->capacity) {
        const u32 growth = 2;
        ecs_realloc((void **)&archetype->entity_ids, sizeof(ecs_entity_t) * archetype->capacity * growth);
        ecs_archetype_resize_component_array(archetype, component_index, archetype->capacity * growth);
    }

    archetype->entity_ids[archetype->count] = e;
    ecs_map_set(entity_index, (void *)e, &(ecs_record_t){archetype, archetype->count});

    return archetype->count++;
}

u32 ecs_archetype_move_entity_right(ecs_archetype_t *left, ecs_archetype_t *right, const ecs_map_t *component_index, ecs_map_t *entity_index, u32 left_row) {
    ECS_ASSERT(left_row < left->count, OUT_OF_BOUNDS);
    ecs_entity_t removed = left->entity_ids[left_row];
    left->entity_ids[left_row] = left->entity_ids[left->count - 1];

    u32 right_row = ecs_archetype_add(right, component_index, entity_index, removed);

    u32 i = 0, j = 0;
    ECS_TYPE_EACH(left->type, e, {
        ECS_ASSERT(e >= right->type->elements[j], "elements in types mismatched");

        while (e != right->type->elements[j]) {
            j++;
        }

        size_t *component_size = ecs_map_get(component_index, (void *)e);
        ECS_ASSERT(component_size != NULL, FAILED_LOOKUP);
        void *left_component_array = left->components[i];
        void *right_component_array = right->components[j];

        void *insert_component = ECS_OFFSET(right_component_array, *component_size * right_row);
        void *remove_component = ECS_OFFSET(left_component_array, *component_size * left_row);
        void *swap_component = ECS_OFFSET(left_component_array, *component_size * (left->count - 1));

        memcpy(insert_component, remove_component, *component_size);
        memcpy(remove_component, swap_component, *component_size);

        i++;
    });

    left->count--;
    return right_row;
}

static inline void ecs_archetype_make_edges(ecs_archetype_t *left, ecs_archetype_t *right, ecs_entity_t component) {
    ecs_edge_list_add(left->right_edges, (ecs_edge_t){component, right});
    ecs_edge_list_add(right->left_edges, (ecs_edge_t){component, left});
}

static void ecs_archetype_insert_vertex_help(ecs_archetype_t *node, ecs_archetype_t *new_node) {
    u32 node_type_len = ecs_type_len(node->type);
    u32 new_type_len = ecs_type_len(new_node->type);

    if (node_type_len > new_type_len - 1) {
        return;
    }

    if (node_type_len < new_type_len - 1) {
        ECS_EDGE_LIST_EACH(node->right_edges, edge, { ecs_archetype_insert_vertex_help(edge.archetype, new_node); });
        return;
    }

    if (!ecs_type_is_superset(node->type, new_node->type)) {
        return;
    }

    u32 i;
    u32 new_node_type_len = ecs_type_len(new_node->type);
    for (i = 0; i < new_node_type_len && node->type->elements[i] == new_node->type->elements[i]; i++);
    ecs_archetype_make_edges(new_node, node, node->type->elements[i]);
}

ecs_archetype_t *ecs_archetype_insert_vertex(ecs_archetype_t *root, ecs_archetype_t *left_neighbour, ecs_type_t *new_vertex_type, ecs_entity_t component_for_edge, const ecs_map_t *component_index,
                                             ecs_map_t *type_index) {
    ecs_archetype_t *vertex = ecs_archetype_new(new_vertex_type, component_index, type_index);
    ecs_archetype_make_edges(left_neighbour, vertex, component_for_edge);
    ecs_archetype_insert_vertex_help(root, vertex);
    return vertex;
}

static ecs_archetype_t *ecs_archetype_traverse_and_create_help(ecs_archetype_t *vertex, const ecs_type_t *type, u32 stack_n, ecs_entity_t acc[], u32 acc_top, ecs_archetype_t *root,
                                                               const ecs_map_t *component_index, ecs_map_t *type_index) {
    if (stack_n == 0) {
        ECS_ASSERT(ecs_type_equal(vertex->type, type), SOMETHING_TERRIBLE);
        return vertex;
    }

    ECS_EDGE_LIST_EACH(vertex->right_edges, edge, {
        if (ecs_type_index_of(type, edge.component) != -1) {
            acc[acc_top] = edge.component;
            return ecs_archetype_traverse_and_create_help(edge.archetype, type, stack_n - 1, acc, acc_top + 1, root, component_index, type_index);
        }
    });

    u32 i;
    ecs_type_t *new_type = ecs_type_new(acc_top);
    for (i = 0; i < acc_top; i++) {
        ecs_type_add(new_type, acc[i]);
    }

    i = 0;
    ecs_entity_t new_component = 0;
    ECS_TYPE_EACH(type, e, {
        if (e != new_type->elements[i]) {
            new_component = e;
            ecs_type_add(new_type, new_component);
            acc[acc_top] = new_component;
            break;
        }

        i++;
    });

    ECS_ASSERT(new_component != 0, SOMETHING_TERRIBLE);
    ecs_archetype_t *new_vertex = ecs_archetype_insert_vertex(root, vertex, new_type, new_component, component_index, type_index);

    return ecs_archetype_traverse_and_create_help(new_vertex, type, stack_n - 1, acc, acc_top + 1, root, component_index, type_index);
}

ecs_archetype_t *ecs_archetype_traverse_and_create(ecs_archetype_t *root, const ecs_type_t *type, const ecs_map_t *component_index, ecs_map_t *type_index) {
    u32 len = ecs_type_len(type);
    ecs_entity_t *acc = alloca(sizeof(ecs_entity_t) * len);
    return ecs_archetype_traverse_and_create_help(root, type, len, acc, 0, root, component_index, type_index);
}

void ecs_inspect(neko_ecs_t *registry) {
    ecs_archetype_t *archetype = registry->root;
    printf("\narchetype: {\n");
    printf("  self: %p\n", (void *)archetype);
    printf("  capacity: %d\n", archetype->capacity);
    printf("  count: %d\n", archetype->count);

    printf("  type: %p\n", (void *)archetype->type);
    printf("  type: [ ");
    ECS_TYPE_EACH(archetype->type, e, { printf("%llu ", e); });
    printf("]\n");

    printf("  entity_ids: [\n");
    for (u32 i = 0; i < archetype->count; i++) {
        printf("    %llu\n", archetype->entity_ids[i]);
    }
    printf("  ]\n");

    printf("  left_edges: [\n");
    ECS_EDGE_LIST_EACH(archetype->left_edges, edge, { printf("    { %llu, %p }\n", edge.component, (void *)edge.archetype); });
    printf("  ]\n");

    printf("  right_edges: [\n");
    ECS_EDGE_LIST_EACH(archetype->right_edges, edge, { printf("    { %llu, %p }\n", edge.component, (void *)edge.archetype); });
    printf("  ]\n");

    printf("  components: [\n");
    u32 type_len = ecs_type_len(archetype->type);
    for (u32 i = 0; i < type_len; i++) {
        printf("    %d: [\n", i);
        for (u32 j = 0; j < archetype->capacity; j++) {
            if (j == archetype->count) {
                printf("      -- end of load --\n");
            }
            printf("      %d: %f\n", j, *(float *)ECS_OFFSET(archetype->components[i], j * sizeof(float)));
        }
        printf("    ]\n");
    }
    printf("  ]\n");

    printf("}\n");
}

neko_ecs_t *ecs_init_i(neko_ecs_t *registry) {
    // neko_ecs_t *registry = ecs_malloc(sizeof(neko_ecs_t));
    registry->entity_index = ECS_MAP(intptr, ecs_entity_t, ecs_record_t, 16);
    registry->component_index = ECS_MAP(intptr, ecs_entity_t, size_t, 8);
    registry->system_index = ECS_MAP(intptr, ecs_entity_t, ecs_system_t, 4);
    registry->type_index = ECS_MAP(type, ecs_type_t *, ecs_archetype_t *, 8);

    ecs_type_t *root_type = ecs_type_new(0);
    registry->root = ecs_archetype_new(root_type, registry->component_index, registry->type_index);
    registry->next_entity_id = 1;
    return registry;
}

neko_ecs_t *ecs_init(lua_State *L) {
    NEKO_ASSERT(L);
    neko_ecs_t *registry = (neko_ecs_t *)lua_newuserdatauv(L, sizeof(neko_ecs_t), NEKO_ECS_UPVAL_N);  // # -1
    registry = ecs_init_i(registry);
    if (registry == NULL) {
        NEKO_ERROR("%s", "failed to initialize neko_ecs_t");
        return NULL;
    }
    registry->L = L;

    if (luaL_getmetatable(L, ECS_WORLD_UDATA_NAME) == LUA_TNIL) {  // # -2
        luaL_Reg world_mt[] = {
                {"__gc", __neko_ecs_lua_gc}, {"create_ent", __neko_ecs_lua_create_ent}, {"attach", __neko_ecs_lua_attach}, {"get_com", __neko_ecs_lua_get_com}, {NULL, NULL},
        };
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

void ecs_fini_i(neko_ecs_t *registry) {
    ECS_MAP_VALUES_EACH(registry->system_index, ecs_system_t, system, { ecs_signature_free(system->sig); });
    ECS_MAP_VALUES_EACH(registry->type_index, ecs_archetype_t *, archetype, { ecs_archetype_free(*archetype); });
    ecs_map_free(registry->type_index);
    ecs_map_free(registry->entity_index);
    ecs_map_free(registry->component_index);
    ecs_map_free(registry->system_index);
}

// void ecs_fini(neko_ecs_t *registry) {
//     ecs_fini_i(registry);
//     free(registry);
// }

ecs_entity_t ecs_entity(neko_ecs_t *registry) {
    ecs_archetype_t *root = registry->root;
    u32 row = ecs_archetype_add(root, registry->component_index, registry->entity_index, registry->next_entity_id);
    ecs_map_set(registry->entity_index, (void *)registry->next_entity_id, &(ecs_record_t){root, row});
    return registry->next_entity_id++;
}

ecs_entity_t ecs_component_w(neko_ecs_t *registry, const_str component_name, size_t component_size) {

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

ecs_entity_t ecs_system(neko_ecs_t *registry, ecs_signature_t *signature, ecs_system_fn system) {
    ecs_type_t *type = ecs_signature_as_type(signature);
    ecs_archetype_t **maybe_archetype = ecs_map_get(registry->type_index, type);
    ecs_archetype_t *archetype;

    if (maybe_archetype == NULL) {
        archetype = ecs_archetype_traverse_and_create(registry->root, type, registry->component_index, registry->type_index);
    } else {
        archetype = *maybe_archetype;
        ecs_type_free(type);
    }

    ecs_map_set(registry->system_index, (void *)registry->next_entity_id, &(ecs_system_t){archetype, signature, system});
    return registry->next_entity_id++;
}

void ecs_attach(neko_ecs_t *registry, ecs_entity_t entity, ecs_entity_t component) {
    ecs_record_t *record = ecs_map_get(registry->entity_index, (void *)entity);

    if (record == NULL) {
        char err[255];
        sprintf(err, "attaching component %llu to unknown entity %llu", component, entity);
        ECS_ABORT(err);
    }

    ecs_type_t *init_type = record->archetype->type;
    ecs_type_t *fini_type = ecs_type_copy(init_type);
    ecs_type_add(fini_type, component);

    ecs_archetype_t **maybe_fini_archetype = ecs_map_get(registry->type_index, fini_type);
    ecs_archetype_t *fini_archetype;

    if (maybe_fini_archetype == NULL) {
        fini_archetype = ecs_archetype_insert_vertex(registry->root, record->archetype, fini_type, component, registry->component_index, registry->type_index);
    } else {
        ecs_type_free(fini_type);
        fini_archetype = *maybe_fini_archetype;
    }

    u32 new_row = ecs_archetype_move_entity_right(record->archetype, fini_archetype, registry->component_index, registry->entity_index, record->row);
    ecs_map_set(registry->entity_index, (void *)entity, &(ecs_record_t){fini_archetype, new_row});
}

void ecs_set(neko_ecs_t *registry, ecs_entity_t entity, ecs_entity_t component, const void *data) {
    size_t *component_size = ecs_map_get(registry->component_index, (void *)component);
    ECS_ENSURE(component_size != NULL, FAILED_LOOKUP);

    ecs_record_t *record = ecs_map_get(registry->entity_index, (void *)entity);
    ECS_ENSURE(record != NULL, FAILED_LOOKUP);

    s32 column = ecs_type_index_of(record->archetype->type, component);
    ECS_ENSURE(column != -1, OUT_OF_BOUNDS);

    void *component_array = record->archetype->components[column];
    void *element = ECS_OFFSET(component_array, *component_size * record->row);
    memcpy(element, data, *component_size);
}

static void ecs_step_help(ecs_archetype_t *archetype, const ecs_map_t *component_index, const ecs_signature_t *sig, ecs_system_fn run) {
    if (archetype == NULL) {
        return;
    }

    u32 *signature_to_index = alloca(sizeof(u32) * sig->count);
    u32 *component_sizes = alloca(sizeof(u32) * sig->count);

    for (u32 slow = 0; slow < sig->count; slow++) {
        u32 type_len = ecs_type_len(archetype->type);
        for (u32 fast = 0; fast < type_len; fast++) {
            ecs_entity_t component = archetype->type->elements[fast];
            if (component == sig->components[slow]) {
                size_t *component_size = ecs_map_get(component_index, (void *)component);
                ECS_ENSURE(component_size != NULL, FAILED_LOOKUP);

                component_sizes[slow] = *component_size;
                signature_to_index[slow] = fast;
                break;
            }
        }
    }

    ecs_view_t view = {archetype->components, signature_to_index, component_sizes};
    for (u32 i = 0; i < archetype->count; i++) {
        run(view, i);
    }

    ECS_EDGE_LIST_EACH(archetype->right_edges, edge, { ecs_step_help(edge.archetype, component_index, sig, run); });
}

void ecs_step(neko_ecs_t *registry) {
    ECS_MAP_VALUES_EACH(registry->system_index, ecs_system_t, sys, { ecs_step_help(sys->archetype, registry->component_index, sys->sig, sys->run); });
}

void *ecs_view(ecs_view_t view, u32 row, u32 column) {
    void *component_array = view.component_arrays[view.signature_to_index[column]];
    return ECS_OFFSET(component_array, view.component_sizes[column] * row);
}

#if 0

ECS_COMPONENT_DECLARE(position_t);
ECS_COMPONENT_DECLARE(velocity_t);
ECS_COMPONENT_DECLARE(bounds_t);
ECS_COMPONENT_DECLARE(color_t);

void neko_ecs_com_init(ecs_world_t *world) {
    // Register component with world
    ECS_COMPONENT_DEFINE(world, position_t);
    ECS_COMPONENT_DEFINE(world, velocity_t);
    ECS_COMPONENT_DEFINE(world, bounds_t);
    ECS_COMPONENT_DEFINE(world, color_t);
}

#endif