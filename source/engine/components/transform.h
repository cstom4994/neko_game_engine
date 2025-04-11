
#pragma once

#include "engine/ecs/entity.h"
#include "engine/component.h"

struct CTransform : CEntityBase {
    vec2 position;
    f32 rotation;
    vec2 scale;
    CEntity parent;           // 如果entity_nil 则为 root
    Array<CEntity> children;  // 如果为 NULL 则为空
    mat3 mat_cache;           // 更新此内容
    mat3 worldmat_cache;      // 在父子更新时缓存
    EcsId dirty_count;
};

static_assert(std::is_trivially_copyable_v<CTransform>);

class Transform : public SingletonClass<Transform>, public ComponentTypeBase<CTransform> {
private:
    void UpdateChild(CTransform *parent, CEntity ent);
    void Modified(CTransform *transform);
    void Detach(CTransform *p, CTransform *c);
    void DetachAll(CTransform *t);

public:
    void transform_init();
    void transform_fini();
    int transform_update_all(Event evt);

    CTransform *transform_add(CEntity ent);
    void transform_remove(CEntity ent);
    bool transform_has(CEntity ent);
    void transform_set_parent(CEntity ent, CEntity parent);
    CEntity transform_get_parent(CEntity ent);
    EcsId transform_get_num_children(CEntity ent);
    CEntity *transform_get_children(CEntity ent);
    void transform_detach_all(CEntity ent);
    void transform_destroy_rec(CEntity ent);  // destroy ent and all children

    void transform_set_position(CEntity ent, vec2 pos);
    vec2 transform_get_position(CEntity ent);
    void transform_translate(CEntity ent, vec2 trans);
    void transform_set_rotation(CEntity ent, f32 rot);
    f32 transform_get_rotation(CEntity ent);
    void transform_rotate(CEntity ent, f32 rot);
    void transform_set_scale(CEntity ent, vec2 scale);
    vec2 transform_get_scale(CEntity ent);
    vec2 transform_get_world_position(CEntity ent);
    f32 transform_get_world_rotation(CEntity ent);
    vec2 transform_get_world_scale(CEntity ent);
    mat3 transform_get_world_matrix(CEntity ent);  // world-space
    mat3 transform_get_matrix(CEntity ent);        // parent-space
    vec2 transform_local_to_world(CEntity ent, vec2 v);
    vec2 transform_world_to_local(CEntity ent, vec2 v);
    EcsId transform_get_dirty_count(CEntity ent);
    void transform_set_save_filter_rec(CEntity ent, bool filter);

    CTransform *wrap_transform_add(CEntity ent);

    int Inspect(CEntity ent) override;
};