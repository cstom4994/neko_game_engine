

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/ecs/entitybase.hpp"
#include "engine/edit.h"

// -------------------------------------------------------------------------

static void _update_child(Transform *parent, CEntity ent) {
    Transform *transform;

    transform = Transform::pool->Get(ent);
    error_assert(transform);
    transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    if (transform->children.len)
        for (auto &child : transform->children) {
            _update_child(transform, child);
        }
}
static void _modified(Transform *transform) {
    Transform *parent;

    ++transform->dirty_count;

    transform->mat_cache = mat3_scaling_rotation_translation(transform->scale, transform->rotation, transform->position);

    // 更新世界矩阵
    parent = Transform::pool->Get(transform->parent);
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

static void _detach(Transform *p, Transform *c) {
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

static void _detach_all(Transform *t) {
    Transform *p, *c;
    error_assert(t);

    // our parent
    if (!CEntityEq(t->parent, entity_nil)) {
        p = Transform::pool->Get(t->parent);
        error_assert(p);
        _detach(p, t);
    }

    // our children -- unset each child's parent then clear children array
    if (t->children.len) {
        for (auto &child : t->children) {
            c = Transform::pool->Get(child);
            error_assert(c);
            c->parent = entity_nil;
            _modified(c);
        }
        t->children.trash();
    }

    _modified(t);
}

void transform_add(CEntity ent) {
    Transform *transform;

    if (Transform::pool->Get(ent)) return;

    transform = Transform::pool->Add(ent);
    transform->position = luavec2(0.0f, 0.0f);
    transform->rotation = 0.0f;
    transform->scale = luavec2(1.0f, 1.0f);

    transform->parent = entity_nil;
    transform->children = {};

    transform->dirty_count = 0;

    _modified(transform);
}
void transform_remove(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    if (transform) _detach_all(transform);
    Transform::pool->Remove(ent);
}
bool transform_has(CEntity ent) { return Transform::pool->Get(ent) != NULL; }

// 根转换具有父级 = entity_nil
void transform_set_parent(CEntity ent, CEntity parent) {
    Transform *t, *oldp, *newp;

    if (CEntityEq(ent, parent)) return;  // can't be child of self

    t = Transform::pool->Get(ent);
    error_assert(t);

    if (CEntityEq(t->parent, parent)) return;  // already set

    // detach from old
    if (!CEntityEq(t->parent, entity_nil)) {
        oldp = Transform::pool->Get(t->parent);
        error_assert(oldp);
        _detach(oldp, t);
    }

    // attach to new
    t->parent = parent;
    if (!CEntityEq(parent, entity_nil)) {
        newp = Transform::pool->Get(parent);
        error_assert(newp);
        if (!newp->children.len) {
            newp->children.reserve(4);  // TODO: 可以优化
        }
        newp->children.push(ent);
    }

    _modified(t);
}
CEntity transform_get_parent(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->parent;
}
EcsId transform_get_num_children(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->children.len;
}
CEntity *transform_get_children(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->children.len ? transform->children.begin() : NULL;
}
// 脱离父项和所有子项
void transform_detach_all(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    _detach_all(transform);
}
void transform_destroy_rec(CEntity ent) {
    Transform *transform;

    transform = Transform::pool->Get(ent);
    if (transform && transform->children.len)
        for (auto &child : transform->children) transform_destroy_rec(child);

    entity_destroy(ent);
}

void transform_set_position(CEntity ent, vec2 pos) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    transform->position = pos;
    _modified(transform);
}
vec2 transform_get_position(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->position;
}
void transform_translate(CEntity ent, vec2 trans) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    transform->position = vec2_add(transform->position, trans);
    _modified(transform);
}

void transform_set_rotation(CEntity ent, f32 rot) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    transform->rotation = rot;
    _modified(transform);
}
f32 transform_get_rotation(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->rotation;
}
void transform_rotate(CEntity ent, f32 rot) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    transform->rotation += rot;
    _modified(transform);
}

void transform_set_scale(CEntity ent, vec2 scale) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    transform->scale = scale;
    _modified(transform);
}
vec2 transform_get_scale(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->scale;
}

vec2 transform_get_world_position(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return mat3_get_translation(transform->worldmat_cache);
}
f32 transform_get_world_rotation(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return mat3_get_rotation(transform->worldmat_cache);
}
vec2 transform_get_world_scale(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return mat3_get_scale(transform->worldmat_cache);
}

mat3 transform_get_world_matrix(CEntity ent) {
    Transform *transform;

    if (CEntityEq(ent, entity_nil)) return mat3_identity();

    transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->worldmat_cache;
}

mat3 transform_get_matrix(CEntity ent) {
    Transform *transform;

    if (CEntityEq(ent, entity_nil)) return mat3_identity();

    transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->mat_cache;
}

vec2 transform_local_to_world(CEntity ent, vec2 v) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return mat3_transform(transform->worldmat_cache, v);
}
vec2 transform_world_to_local(CEntity ent, vec2 v) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return mat3_transform(mat3_inverse(transform->worldmat_cache), v);
}

EcsId transform_get_dirty_count(CEntity ent) {
    Transform *transform = Transform::pool->Get(ent);
    error_assert(transform);
    return transform->dirty_count;
}

void transform_set_save_filter_rec(CEntity ent, bool filter) {
    Transform *transform;

    // entity_set_save_filter(ent, filter);

    transform = Transform::pool->Get(ent);
    error_assert(transform);
    if (transform->children.len)
        for (auto &child : transform->children) transform_set_save_filter_rec(child, filter);
}

// -------------------------------------------------------------------------

static void _free_children_arrays() {
    Transform *transform;

    entitypool_foreach(transform, Transform::pool) if (transform->children.len) transform->children.trash();
}

void transform_init() {
    PROFILE_FUNC();

    Transform::pool = entitypool_new<Transform>();
}

void transform_fini() {
    _free_children_arrays();
    entitypool_free(Transform::pool);
}

int transform_update_all(App *app, event_t evt) {
    Transform *transform;
    static BBox bbox = {{0, 0}, {0, 0}};

    entitypool_remove_destroyed(Transform::pool, transform_remove);

    // update edit bbox
    if (edit_get_enabled()) entitypool_foreach(transform, Transform::pool) edit_bboxes_update(transform->ent, bbox);

    return 0;
}

// save/load for just the children array
static void _children_save(Transform *t, App *app) {
    // Store *u;

    // if (store_child_save(&u, "children", s))
    //     if (t->children.len)
    //         for (auto &child : t->children)
    //             if (entity_get_save_filter(child)) entity_save(&child, NULL, u);
}
static void _children_load(Transform *t, App *app) {
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

void transform_save_all(App *app) {
    // Store *t, *transform_s;
    // Transform *transform;

    // if (store_child_save(&t, "transform", s)) entitypool_save_foreach(transform, transform_s, Transform::pool, "pool", t) {
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
void transform_load_all(App *app) {
    // Store *t, *transform_s;
    // Transform *transform;

    // if (store_child_load(&t, "transform", s)) {
    //     entitypool_load_foreach(transform, transform_s, Transform::pool, "pool", t) {
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
