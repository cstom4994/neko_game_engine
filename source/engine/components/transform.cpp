

#include "engine/bootstrap.h"
#include "engine/edit.h"
#include "engine/entity.h"

DECL_ENT(Transform, vec2 position; Scalar rotation; vec2 scale; NativeEntity parent;  // 如果entity_nil 则为 root
         CArray * children;                                                           // 如果为 NULL 则为空
         mat3 mat_cache;                                                              // 更新此内容
         mat3 worldmat_cache;                                                         // 在父子更新时缓存
         ecs_id_t dirty_count;);

static NativeEntityPool *pool_transform;

// -------------------------------------------------------------------------

static void _update_child(Transform *parent, NativeEntity ent) {
    Transform *transform;
    NativeEntity *child;

    transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    if (transform->children) array_foreach(child, transform->children) {
            _update_child(transform, *child);
        }
}
static void _modified(Transform *transform) {
    Transform *parent;
    NativeEntity *child;

    ++transform->dirty_count;

    transform->mat_cache = mat3_scaling_rotation_translation(transform->scale, transform->rotation, transform->position);

    // 更新世界矩阵
    parent = (Transform *)entitypool_get(pool_transform, transform->parent);
    if (parent)
        transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    else
        transform->worldmat_cache = transform->mat_cache;

    // 更新子世界矩阵
    if (transform->children) array_foreach(child, transform->children) {
            _update_child(transform, *child);
        }
}

static void _detach(Transform *p, Transform *c) {
    NativeEntity *child;

    // remove child -> parent link
    c->parent = entity_nil;

    // search for parent -> child link and remove it
    array_foreach(child, p->children) if (native_entity_eq(*child, c->pool_elem.ent)) {
        array_quick_remove(p->children, child - (NativeEntity *)array_begin(p->children));
        return;
    }

    _modified(c);
}

static void _detach_all(Transform *t) {
    NativeEntity *child;
    Transform *p, *c;
    error_assert(t);

    // our parent
    if (!native_entity_eq(t->parent, entity_nil)) {
        p = (Transform *)entitypool_get(pool_transform, t->parent);
        error_assert(p);
        _detach(p, t);
    }

    // our children -- unset each child's parent then clear children array
    if (t->children) {
        array_foreach(child, t->children) {
            c = (Transform *)entitypool_get(pool_transform, *child);
            error_assert(c);
            c->parent = entity_nil;
            _modified(c);
        }
        array_free(t->children);
        t->children = NULL;
    }

    _modified(t);
}

void transform_add(NativeEntity ent) {
    Transform *transform;

    if (entitypool_get(pool_transform, ent)) return;

    transform = (Transform *)entitypool_add(pool_transform, ent);
    transform->position = luavec2(0.0f, 0.0f);
    transform->rotation = 0.0f;
    transform->scale = luavec2(1.0f, 1.0f);

    transform->parent = entity_nil;
    transform->children = NULL;

    transform->dirty_count = 0;

    _modified(transform);
}
void transform_remove(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    if (transform) _detach_all(transform);
    entitypool_remove(pool_transform, ent);
}
bool transform_has(NativeEntity ent) { return entitypool_get(pool_transform, ent) != NULL; }

void transform_set_parent(NativeEntity ent, NativeEntity parent) {
    Transform *t, *oldp, *newp;

    if (native_entity_eq(ent, parent)) return;  // can't be child of self

    t = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(t);

    if (native_entity_eq(t->parent, parent)) return;  // already set

    // detach from old
    if (!native_entity_eq(t->parent, entity_nil)) {
        oldp = (Transform *)entitypool_get(pool_transform, t->parent);
        error_assert(oldp);
        _detach(oldp, t);
    }

    // attach to new
    t->parent = parent;
    if (!native_entity_eq(parent, entity_nil)) {
        newp = (Transform *)entitypool_get(pool_transform, parent);
        error_assert(newp);
        if (!newp->children) newp->children = array_new(NativeEntity);
        array_add_val(NativeEntity, newp->children) = ent;
    }

    _modified(t);
}
NativeEntity transform_get_parent(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->parent;
}
ecs_id_t transform_get_num_children(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->children ? array_length(transform->children) : 0;
}
NativeEntity *transform_get_children(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->children ? (NativeEntity *)array_begin(transform->children) : NULL;
}
void transform_detach_all(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    _detach_all(transform);
}
void transform_destroy_rec(NativeEntity ent) {
    Transform *transform;
    NativeEntity *child;

    transform = (Transform *)entitypool_get(pool_transform, ent);
    if (transform && transform->children) array_foreach(child, transform->children) transform_destroy_rec(*child);

    entity_destroy(ent);
}

void transform_set_position(NativeEntity ent, vec2 pos) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    transform->position = pos;
    _modified(transform);
}
vec2 transform_get_position(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->position;
}
void transform_translate(NativeEntity ent, vec2 trans) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    transform->position = vec2_add(transform->position, trans);
    _modified(transform);
}

void transform_set_rotation(NativeEntity ent, Scalar rot) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    transform->rotation = rot;
    _modified(transform);
}
Scalar transform_get_rotation(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->rotation;
}
void transform_rotate(NativeEntity ent, Scalar rot) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    transform->rotation += rot;
    _modified(transform);
}

void transform_set_scale(NativeEntity ent, vec2 scale) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    transform->scale = scale;
    _modified(transform);
}
vec2 transform_get_scale(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->scale;
}

vec2 transform_get_world_position(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return mat3_get_translation(transform->worldmat_cache);
}
Scalar transform_get_world_rotation(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return mat3_get_rotation(transform->worldmat_cache);
}
vec2 transform_get_world_scale(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return mat3_get_scale(transform->worldmat_cache);
}

mat3 transform_get_world_matrix(NativeEntity ent) {
    Transform *transform;

    if (native_entity_eq(ent, entity_nil)) return mat3_identity();

    transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->worldmat_cache;
}
mat3 transform_get_matrix(NativeEntity ent) {
    Transform *transform;

    if (native_entity_eq(ent, entity_nil)) return mat3_identity();

    transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->mat_cache;
}

vec2 transform_local_to_world(NativeEntity ent, vec2 v) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return mat3_transform(transform->worldmat_cache, v);
}
vec2 transform_world_to_local(NativeEntity ent, vec2 v) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return mat3_transform(mat3_inverse(transform->worldmat_cache), v);
}

ecs_id_t transform_get_dirty_count(NativeEntity ent) {
    Transform *transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    return transform->dirty_count;
}

void transform_set_save_filter_rec(NativeEntity ent, bool filter) {
    Transform *transform;
    NativeEntity *child;

    entity_set_save_filter(ent, filter);

    transform = (Transform *)entitypool_get(pool_transform, ent);
    error_assert(transform);
    if (transform->children) array_foreach(child, transform->children) transform_set_save_filter_rec(*child, filter);
}

// -------------------------------------------------------------------------

static void _free_children_arrays() {
    Transform *transform;

    entitypool_foreach(transform, pool_transform) if (transform->children) array_free(transform->children);
}

void transform_init() {
    PROFILE_FUNC();

    pool_transform = entitypool_new(Transform);
}

void transform_fini() {
    _free_children_arrays();
    entitypool_free(pool_transform);
}

void transform_update_all() {
    Transform *transform;
    static BBox bbox = {{0, 0}, {0, 0}};

    entitypool_remove_destroyed(pool_transform, transform_remove);

    // update edit bbox
    if (edit_get_enabled()) entitypool_foreach(transform, pool_transform) edit_bboxes_update(transform->pool_elem.ent, bbox);
}

// save/load for just the children array
static void _children_save(Transform *t, Store *s) {
    Store *u;
    NativeEntity *child;

    if (store_child_save(&u, "children", s))
        if (t->children) array_foreach(child, t->children) if (entity_get_save_filter(*child)) entity_save(child, NULL, u);
}
static void _children_load(Transform *t, Store *s) {
    Store *u;
    NativeEntity child;

    t->children = NULL;

    // this is a little weird because we want NULL array when no children
    if (store_child_load(&u, "children", s))
        if (entity_load(&child, NULL, entity_nil, u)) {
            t->children = array_new(NativeEntity);
            do array_add_val(NativeEntity, t->children) = child;
            while (entity_load(&child, NULL, entity_nil, u));
        }
}

void transform_save_all(Store *s) {
    Store *t, *transform_s;
    Transform *transform;

    if (store_child_save(&t, "transform", s)) entitypool_save_foreach(transform, transform_s, pool_transform, "pool", t) {
            vec2_save(&transform->position, "position", transform_s);
            scalar_save(&transform->rotation, "rotation", transform_s);
            vec2_save(&transform->scale, "scale", transform_s);

            if (entity_get_save_filter(transform->parent))
                entity_save(&transform->parent, "parent", transform_s);
            else
                entity_save(&entity_nil, "parent", transform_s);
            _children_save(transform, transform_s);

            mat3_save(&transform->mat_cache, "mat_cache", transform_s);
            mat3_save(&transform->worldmat_cache, "worldmat_cache", transform_s);

            uint_save(&transform->dirty_count, "dirty_count", transform_s);
        }
}
void transform_load_all(Store *s) {
    Store *t, *transform_s;
    Transform *transform;

    if (store_child_load(&t, "transform", s)) {
        entitypool_load_foreach(transform, transform_s, pool_transform, "pool", t) {
            vec2_load(&transform->position, "position", vec2_zero, transform_s);
            scalar_load(&transform->rotation, "rotation", 0, transform_s);
            vec2_load(&transform->scale, "scale", luavec2(1, 1), transform_s);

            entity_load(&transform->parent, "parent", entity_nil, transform_s);
            _children_load(transform, transform_s);

            mat3_load(&transform->mat_cache, "mat_cache", mat3_identity(), transform_s);
            mat3_load(&transform->worldmat_cache, "worldmat_cache", mat3_identity(), transform_s);

            uint_load(&transform->dirty_count, "dirty_count", 0, transform_s);
        }
    }
}
