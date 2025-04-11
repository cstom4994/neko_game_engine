
#include "transform.h"

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"

// -------------------------------------------------------------------------

void Transform::UpdateChild(CTransform *parent, CEntity ent) {
    CTransform *transform;

    transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    if (transform->children.len) {
        for (auto &child : transform->children) {
            UpdateChild(transform, child);
        }
    }
}

void Transform::Modified(CTransform *transform) {
    CTransform *parent;

    ++transform->dirty_count;

    transform->mat_cache = mat3_scaling_rotation_translation(transform->scale, transform->rotation, transform->position);

    // 更新世界矩阵
    parent = ComponentTypeBase::EntityPool->GetPtr(transform->parent);
    if (parent) {
        transform->worldmat_cache = mat3_mul(parent->worldmat_cache, transform->mat_cache);
    } else {
        transform->worldmat_cache = transform->mat_cache;
    }

    // 更新子世界矩阵
    if (transform->children.len) {
        for (auto &child : transform->children) {
            UpdateChild(transform, child);
        }
    }
}

void Transform::Detach(CTransform *p, CTransform *c) {
    // remove child -> parent link
    c->parent = entity_nil;

    // search for parent -> child link and remove it
    for (auto &child : p->children)
        if (CEntityEq(child, c->ent)) {
            // array_quick_remove(p->children, child - (CEntity *)array_begin(p->children));
            p->children.quick_remove(&child - p->children.begin());
            return;
        }

    Modified(c);
}

void Transform::DetachAll(CTransform *t) {
    CTransform *p, *c;
    error_assert(t);

    // our parent
    if (!CEntityEq(t->parent, entity_nil)) {
        p = ComponentTypeBase::EntityPool->GetPtr(t->parent);
        error_assert(p);
        Detach(p, t);
    }

    // our children -- unset each child's parent then clear children array
    if (t->children.len) {
        for (auto &child : t->children) {
            c = ComponentTypeBase::EntityPool->GetPtr(child);
            error_assert(c);
            c->parent = entity_nil;
            Modified(c);
        }
        t->children.trash();
    }

    Modified(t);
}

CTransform *Transform::transform_add(CEntity ent) {
    CTransform *transform;

    if (ComponentTypeBase::EntityPool->GetPtr(ent)) return nullptr;

    transform = ComponentTypeBase::EntityPool->Add(ent);
    transform->position = luavec2(0.0f, 0.0f);
    transform->rotation = 0.0f;
    transform->scale = luavec2(1.0f, 1.0f);

    transform->parent = entity_nil;
    transform->children = {};

    transform->dirty_count = 0;

    Modified(transform);

    return transform;
}

void Transform::transform_remove(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    if (transform) DetachAll(transform);
    ComponentTypeBase::EntityPool->Remove(ent);
}
bool Transform::transform_has(CEntity ent) { return ComponentTypeBase::EntityPool->GetPtr(ent) != NULL; }

// 根转换具有父级 = entity_nil
void Transform::transform_set_parent(CEntity ent, CEntity parent) {
    CTransform *t, *oldp, *newp;

    if (CEntityEq(ent, parent)) return;  // can't be child of self

    t = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(t);

    if (CEntityEq(t->parent, parent)) return;  // already set

    // detach from old
    if (!CEntityEq(t->parent, entity_nil)) {
        oldp = ComponentTypeBase::EntityPool->GetPtr(t->parent);
        error_assert(oldp);
        Detach(oldp, t);
    }

    // attach to new
    t->parent = parent;
    if (!CEntityEq(parent, entity_nil)) {
        newp = ComponentTypeBase::EntityPool->GetPtr(parent);
        error_assert(newp);
        if (!newp->children.len) {
            newp->children.reserve(4);  // TODO: 可以优化
        }
        newp->children.push(ent);
    }

    Modified(t);
}
CEntity Transform::transform_get_parent(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->parent;
}
EcsId Transform::transform_get_num_children(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->children.len;
}
CEntity *Transform::transform_get_children(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->children.len ? transform->children.begin() : NULL;
}
// 脱离父项和所有子项
void Transform::transform_detach_all(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    DetachAll(transform);
}
void Transform::transform_destroy_rec(CEntity ent) {
    CTransform *transform;

    transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    if (transform && transform->children.len) {
        for (auto &child : transform->children) transform_destroy_rec(child);
    }

    EcsEntityDel(ENGINE_LUA(), ent.id);
}

void Transform::transform_set_position(CEntity ent, vec2 pos) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    transform->position = pos;
    Modified(transform);
}
vec2 Transform::transform_get_position(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->position;
}
void Transform::transform_translate(CEntity ent, vec2 trans) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    transform->position = vec2_add(transform->position, trans);
    Modified(transform);
}

void Transform::transform_set_rotation(CEntity ent, f32 rot) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    transform->rotation = rot;
    Modified(transform);
}
f32 Transform::transform_get_rotation(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->rotation;
}
void Transform::transform_rotate(CEntity ent, f32 rot) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    transform->rotation += rot;
    Modified(transform);
}

void Transform::transform_set_scale(CEntity ent, vec2 scale) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    transform->scale = scale;
    Modified(transform);
}
vec2 Transform::transform_get_scale(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->scale;
}

vec2 Transform::transform_get_world_position(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return mat3_get_translation(transform->worldmat_cache);
}
f32 Transform::transform_get_world_rotation(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return mat3_get_rotation(transform->worldmat_cache);
}
vec2 Transform::transform_get_world_scale(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return mat3_get_scale(transform->worldmat_cache);
}

mat3 Transform::transform_get_world_matrix(CEntity ent) {
    CTransform *transform;

    if (CEntityEq(ent, entity_nil)) return mat3_identity();

    transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->worldmat_cache;
}

mat3 Transform::transform_get_matrix(CEntity ent) {
    CTransform *transform;

    if (CEntityEq(ent, entity_nil)) return mat3_identity();

    transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->mat_cache;
}

vec2 Transform::transform_local_to_world(CEntity ent, vec2 v) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return mat3_transform(transform->worldmat_cache, v);
}
vec2 Transform::transform_world_to_local(CEntity ent, vec2 v) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return mat3_transform(mat3_inverse(transform->worldmat_cache), v);
}

EcsId Transform::transform_get_dirty_count(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    return transform->dirty_count;
}

void Transform::transform_set_save_filter_rec(CEntity ent, bool filter) {
    CTransform *transform;

    // entity_set_save_filter(ent, filter);

    transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);
    if (transform->children.len) {
        for (auto &child : transform->children) transform_set_save_filter_rec(child, filter);
    }
}

// -------------------------------------------------------------------------

CTransform *Transform::wrap_transform_add(CEntity ent) {
    CTransform *ptr = transform_add(ent);

    auto L = ENGINE_LUA();

    EcsWorld *world = ENGINE_ECS();
    EntityData *e = EcsGetEnt(L, world, ent.id);
    LuaRef tb = LuaRef::NewTable(L);
    tb["__ud"] = ptr;
    int cid1 = EcsComponentSet(L, e, the<Transform>().GetTid(), tb);

    return ptr;
}

// -------------------------------------------------------------------------

void Transform::transform_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    ComponentTypeBase::Tid = EcsRegisterCType<CTransform>(L);
    ComponentTypeBase::EntityPool = EcsProtoGetCType<CTransform>(L);

    // clang-format off

    auto type = BUILD_TYPE(Transform)
        .MemberMethod("transform_set_position", this, &Transform::transform_set_position)
        .MemberMethod("transform_get_position", this, &Transform::transform_get_position)
        .MemberMethod("transform_translate", this, &Transform::transform_translate)
        .MemberMethod("transform_set_rotation", this, &Transform::transform_set_rotation)
        .MemberMethod("transform_get_rotation", this, &Transform::transform_get_rotation)
        .MemberMethod("transform_rotate", this, &Transform::transform_rotate)
        .MemberMethod("transform_set_scale", this, &Transform::transform_set_scale)
        .MemberMethod("transform_get_scale", this, &Transform::transform_get_scale)
        .MemberMethod("transform_get_world_position", this, &Transform::transform_get_world_position)
        .MemberMethod("transform_get_world_rotation", this, &Transform::transform_get_world_rotation)
        .MemberMethod("transform_get_world_scale", this, &Transform::transform_get_world_scale)
        .MemberMethod("transform_get_matrix", this, &Transform::transform_get_matrix)
        .MemberMethod("transform_local_to_world", this, &Transform::transform_local_to_world)
        .MemberMethod("transform_world_to_local", this, &Transform::transform_world_to_local)
        .MemberMethod("transform_get_dirty_count", this, &Transform::transform_get_dirty_count)
        .MemberMethod("transform_set_save_filter_rec", this, &Transform::transform_set_save_filter_rec)

        .MemberMethod("transform_add", this, &Transform::wrap_transform_add)
        .MemberMethod("transform_remove", this, &Transform::transform_remove)
        .MemberMethod("transform_has", this, &Transform::transform_has)
        .MemberMethod("transform_set_parent", this, &Transform::transform_set_parent)
        .MemberMethod("transform_get_parent", this, &Transform::transform_get_parent)
        .MemberMethod("transform_get_num_children", this, &Transform::transform_get_num_children)
        .MemberMethod("transform_get_children", this, &Transform::transform_get_children)
        .MemberMethod("transform_detach_all", this, &Transform::transform_detach_all)
        .MemberMethod("transform_destroy_rec", this, &Transform::transform_destroy_rec)

        .Build();

    // clang-format on
}

void Transform::transform_fini() {
    CTransform *transform;
    entitypool_foreach(transform, ComponentTypeBase::EntityPool) if (transform->children.len) transform->children.trash();
    entitypool_free(ComponentTypeBase::EntityPool);
}

int Transform::transform_update_all(Event evt) {
    CTransform *transform;
    static BBox bbox = {{0, 0}, {0, 0}};

    entitypool_remove_destroyed(ComponentTypeBase::EntityPool, [this](CEntity ent) { transform_remove(ent); });

    // update edit bbox
    if (edit_get_enabled()) entitypool_foreach(transform, ComponentTypeBase::EntityPool) edit_bboxes_update(transform->ent, bbox);

    return 0;
}

DEFINE_IMGUI_BEGIN(template <>, CTransform) {
    ImGuiWrap::Auto(var.position, "position");
    ImGuiWrap::Auto(var.rotation, "rotation");
    ImGuiWrap::Auto(var.scale, "scale");
    ImGuiWrap::Auto(var.mat_cache, "mat_cache");
    ImGuiWrap::Auto(var.worldmat_cache, "worldmat_cache");
}
DEFINE_IMGUI_END()

int Transform::Inspect(CEntity ent) {
    CTransform *transform = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(transform);

    ImGuiWrap::Auto(transform, "CTransform");

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

    // if (store_child_save(&t, "transform", s)) entitypool_save_foreach(transform, transform_s, ComponentTypeBase::EntityPool, "pool", t) {
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
    //     entitypool_load_foreach(transform, transform_s, ComponentTypeBase::EntityPool, "pool", t) {
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
