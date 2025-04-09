

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/ecs/entitybase.hpp"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"

// -------------------------------------------------------------------------

CEntityPool<CTransform> *CTransform__pool;

int type_transform;

static void _update_child(CTransform *parent, CEntity ent) {
    CTransform *transform;

    transform = CTransform__pool->Get(ent);
    error_assert(transform);
    transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    if (transform->children.len)
        for (auto &child : transform->children) {
            _update_child(transform, child);
        }
}
static void _modified(CTransform *transform) {
    CTransform *parent;

    ++transform->dirty_count;

    transform->mat_cache = mat3_scaling_rotation_translation(transform->scale, transform->rotation, transform->position);

    // 更新世界矩阵
    parent = CTransform__pool->Get(transform->parent);
    if (parent)
        transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    else
        transform->worldmat_cache = transform->mat_cache;

    // 更新子世界矩阵
    if (transform->children.len)
        for (auto &child : transform->children) {
            _update_child(transform, child);
        }
}

static void _detach(CTransform *p, CTransform *c) {
    // remove child -> parent link
    c->parent = entity_nil;

    // search for parent -> child link and remove it
    for (auto &child : p->children)
        if (CEntityEq(child, c->ent)) {
            // array_quick_remove(p->children, child - (CEntity *)array_begin(p->children));
            p->children.quick_remove(&child - p->children.begin());
            return;
        }

    _modified(c);
}

static void _detach_all(CTransform *t) {
    CTransform *p, *c;
    error_assert(t);

    // our parent
    if (!CEntityEq(t->parent, entity_nil)) {
        p = CTransform__pool->Get(t->parent);
        error_assert(p);
        _detach(p, t);
    }

    // our children -- unset each child's parent then clear children array
    if (t->children.len) {
        for (auto &child : t->children) {
            c = CTransform__pool->Get(child);
            error_assert(c);
            c->parent = entity_nil;
            _modified(c);
        }
        t->children.trash();
    }

    _modified(t);
}

void transform_add(CEntity ent) {
    CTransform *transform;

    if (CTransform__pool->Get(ent)) return;

    transform = CTransform__pool->Add(ent);
    transform->position = luavec2(0.0f, 0.0f);
    transform->rotation = 0.0f;
    transform->scale = luavec2(1.0f, 1.0f);

    transform->parent = entity_nil;
    transform->children = {};

    transform->dirty_count = 0;

    _modified(transform);
}
void transform_remove(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    if (transform) _detach_all(transform);
    CTransform__pool->Remove(ent);
}
bool transform_has(CEntity ent) { return CTransform__pool->Get(ent) != NULL; }

// 根转换具有父级 = entity_nil
void transform_set_parent(CEntity ent, CEntity parent) {
    CTransform *t, *oldp, *newp;

    if (CEntityEq(ent, parent)) return;  // can't be child of self

    t = CTransform__pool->Get(ent);
    error_assert(t);

    if (CEntityEq(t->parent, parent)) return;  // already set

    // detach from old
    if (!CEntityEq(t->parent, entity_nil)) {
        oldp = CTransform__pool->Get(t->parent);
        error_assert(oldp);
        _detach(oldp, t);
    }

    // attach to new
    t->parent = parent;
    if (!CEntityEq(parent, entity_nil)) {
        newp = CTransform__pool->Get(parent);
        error_assert(newp);
        if (!newp->children.len) {
            newp->children.reserve(4);  // TODO: 可以优化
        }
        newp->children.push(ent);
    }

    _modified(t);
}
CEntity transform_get_parent(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->parent;
}
EcsId transform_get_num_children(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->children.len;
}
CEntity *transform_get_children(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->children.len ? transform->children.begin() : NULL;
}
// 脱离父项和所有子项
void transform_detach_all(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    _detach_all(transform);
}
void transform_destroy_rec(CEntity ent) {
    CTransform *transform;

    transform = CTransform__pool->Get(ent);
    if (transform && transform->children.len)
        for (auto &child : transform->children) transform_destroy_rec(child);

    EcsEntityDel(ENGINE_LUA(), ent.id);
}

void transform_set_position(CEntity ent, vec2 pos) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    transform->position = pos;
    _modified(transform);
}
vec2 transform_get_position(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->position;
}
void transform_translate(CEntity ent, vec2 trans) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    transform->position = vec2_add(transform->position, trans);
    _modified(transform);
}

void transform_set_rotation(CEntity ent, f32 rot) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    transform->rotation = rot;
    _modified(transform);
}
f32 transform_get_rotation(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->rotation;
}
void transform_rotate(CEntity ent, f32 rot) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    transform->rotation += rot;
    _modified(transform);
}

void transform_set_scale(CEntity ent, vec2 scale) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    transform->scale = scale;
    _modified(transform);
}
vec2 transform_get_scale(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->scale;
}

vec2 transform_get_world_position(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return mat3_get_translation(transform->worldmat_cache);
}
f32 transform_get_world_rotation(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return mat3_get_rotation(transform->worldmat_cache);
}
vec2 transform_get_world_scale(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return mat3_get_scale(transform->worldmat_cache);
}

mat3 transform_get_world_matrix(CEntity ent) {
    CTransform *transform;

    if (CEntityEq(ent, entity_nil)) return mat3_identity();

    transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->worldmat_cache;
}

mat3 transform_get_matrix(CEntity ent) {
    CTransform *transform;

    if (CEntityEq(ent, entity_nil)) return mat3_identity();

    transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->mat_cache;
}

vec2 transform_local_to_world(CEntity ent, vec2 v) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return mat3_transform(transform->worldmat_cache, v);
}
vec2 transform_world_to_local(CEntity ent, vec2 v) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return mat3_transform(mat3_inverse(transform->worldmat_cache), v);
}

EcsId transform_get_dirty_count(CEntity ent) {
    CTransform *transform = CTransform__pool->Get(ent);
    error_assert(transform);
    return transform->dirty_count;
}

void transform_set_save_filter_rec(CEntity ent, bool filter) {
    CTransform *transform;

    // entity_set_save_filter(ent, filter);

    transform = CTransform__pool->Get(ent);
    error_assert(transform);
    if (transform->children.len)
        for (auto &child : transform->children) transform_set_save_filter_rec(child, filter);
}

// -------------------------------------------------------------------------

int wrap_transform_add(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    transform_add(*ent);
    return 0;
}
int wrap_transform_remove(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    transform_remove(*ent);
    return 0;
}
int wrap_transform_has(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    bool v = transform_has(*ent);
    lua_pushboolean(L, v);
    return 1;
}
int wrap_transform_set_parent(lua_State *L) {
    CEntity *a = LuaGet<CEntity>(L, 1);
    CEntity *b = LuaGet<CEntity>(L, 2);
    transform_set_parent(*a, *b);
    return 0;
}
int wrap_transform_get_parent(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    CEntity ret = transform_get_parent(*ent);
    LuaPush<CEntity>(L, ret);
    return 1;
}
int wrap_transform_get_num_children(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    EcsId v = transform_get_num_children(*ent);
    lua_pushinteger(L, v);
    return 1;
}
int wrap_transform_get_children(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    CEntity *v = transform_get_children(*ent);
    lua_pushinteger(L, v->id);
    return 1;
}
int wrap_transform_detach_all(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    transform_detach_all(*ent);
    return 0;
}
int wrap_transform_destroy_rec(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    transform_destroy_rec(*ent);
    return 0;
}

// -------------------------------------------------------------------------

static void _free_children_arrays() {
    CTransform *transform;

    entitypool_foreach(transform, CTransform__pool) if (transform->children.len) transform->children.trash();
}

void Transform::transform_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    type_transform = EcsRegisterCType<CTransform>(L);

    CTransform__pool = EcsProtoGetCType<CTransform>(L);

    auto type = BUILD_TYPE(Transform)
                        .Method("transform_set_position", &transform_set_position)                //
                        .Method("transform_get_position", &transform_get_position)                //
                        .Method("transform_translate", &transform_translate)                      //
                        .Method("transform_set_rotation", &transform_set_rotation)                //
                        .Method("transform_get_rotation", &transform_get_rotation)                //
                        .Method("transform_rotate", &transform_rotate)                            //
                        .Method("transform_set_scale", &transform_set_scale)                      //
                        .Method("transform_get_scale", &transform_get_scale)                      //
                        .Method("transform_get_world_position", &transform_get_world_position)    //
                        .Method("transform_get_world_rotation", &transform_get_world_rotation)    //
                        .Method("transform_get_world_scale", &transform_get_world_scale)          //
                        .Method("transform_get_matrix", &transform_get_matrix)                    //
                        .Method("transform_local_to_world", &transform_local_to_world)            //
                        .Method("transform_world_to_local", &transform_world_to_local)            //
                        .Method("transform_get_dirty_count", &transform_get_dirty_count)          //
                        .Method("transform_set_save_filter_rec", &transform_set_save_filter_rec)  //
                        .CClosure({{"transform_add", wrap_transform_add},
                                   {"transform_remove", wrap_transform_remove},
                                   {"transform_has", wrap_transform_has},
                                   {"transform_set_parent", wrap_transform_set_parent},
                                   {"transform_get_parent", wrap_transform_get_parent},
                                   {"transform_get_num_children", wrap_transform_get_num_children},
                                   {"transform_get_children", wrap_transform_get_children},
                                   {"transform_detach_all", wrap_transform_detach_all},
                                   {"transform_destroy_rec", wrap_transform_destroy_rec}})
                        .Build();
}

void Transform::transform_fini() {
    _free_children_arrays();
    entitypool_free(CTransform__pool);
}

int Transform::transform_update_all(Event evt) {
    CTransform *transform;
    static BBox bbox = {{0, 0}, {0, 0}};

    entitypool_remove_destroyed(CTransform__pool, transform_remove);

    // update edit bbox
    if (edit_get_enabled()) entitypool_foreach(transform, CTransform__pool) edit_bboxes_update(transform->ent, bbox);

    return 0;
}

// save/load for just the children array
static void _children_save(CTransform *t, CL *app) {
    // Store *u;

    // if (store_child_save(&u, "children", s))
    //     if (t->children.len)
    //         for (auto &child : t->children)
    //             if (entity_get_save_filter(child)) entity_save(&child, NULL, u);
}
static void _children_load(CTransform *t, CL *app) {
    // Store *u;
    // CEntity child;

    // t->children = {};

    //// this is a little weird because we want NULL array when no children
    // if (store_child_load(&u, "children", s))
    //     if (entity_load(&child, NULL, entity_nil, u)) {
    //         // t->children = (Array<CEntity> *)mem_alloc(sizeof(Array<CEntity>));
    //         // *t->children = {};
    //         do {
    //             t->children.push(child);
    //         } while (entity_load(&child, NULL, entity_nil, u));
    //     }
}

void transform_save_all(CL *app) {
    // Store *t, *transform_s;
    // CTransform *transform;

    // if (store_child_save(&t, "transform", s)) entitypool_save_foreach(transform, transform_s, CTransform__pool, "pool", t) {
    //         vec2_save(&transform->position, "position", transform_s);
    //         float_save(&transform->rotation, "rotation", transform_s);
    //         vec2_save(&transform->scale, "scale", transform_s);

    //         if (entity_get_save_filter(transform->parent))
    //             entity_save(&transform->parent, "parent", transform_s);
    //         else
    //             entity_save(&entity_nil, "parent", transform_s);
    //         _children_save(transform, transform_s);

    //         mat3_save(&transform->mat_cache, "mat_cache", transform_s);
    //         mat3_save(&transform->worldmat_cache, "worldmat_cache", transform_s);

    //         uint_save(&transform->dirty_count, "dirty_count", transform_s);
    //     }
}
void transform_load_all(CL *app) {
    // Store *t, *transform_s;
    // CTransform *transform;

    // if (store_child_load(&t, "transform", s)) {
    //     entitypool_load_foreach(transform, transform_s, CTransform__pool, "pool", t) {
    //         vec2_load(&transform->position, "position", vec2_zero, transform_s);
    //         float_load(&transform->rotation, "rotation", 0, transform_s);
    //         vec2_load(&transform->scale, "scale", luavec2(1, 1), transform_s);

    //         entity_load(&transform->parent, "parent", entity_nil, transform_s);
    //         _children_load(transform, transform_s);

    //         mat3_load(&transform->mat_cache, "mat_cache", mat3_identity(), transform_s);
    //         mat3_load(&transform->worldmat_cache, "worldmat_cache", mat3_identity(), transform_s);

    //         uint_load(&transform->dirty_count, "dirty_count", 0, transform_s);
    //     }
    // }
}
