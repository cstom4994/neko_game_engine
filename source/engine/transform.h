#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "engine/base.h"
#include "engine/ecs.h"
#include "engine/prelude.h"


// can set scale, rotation, position -- 按该顺序应用的转换

NEKO_SCRIPT(transform,

            NEKO_EXPORT void transform_add(Entity ent);

            NEKO_EXPORT void transform_remove(Entity ent);

            NEKO_EXPORT bool transform_has(Entity ent);

            // 根转换具有父级 = entity_nil
            NEKO_EXPORT void transform_set_parent(Entity ent, Entity parent);

            NEKO_EXPORT Entity transform_get_parent(Entity ent);

            NEKO_EXPORT ecs_id_t transform_get_num_children(Entity ent);

            NEKO_EXPORT Entity * transform_get_children(Entity ent);
            // 脱离父项和所有子项
            NEKO_EXPORT void transform_detach_all(Entity ent);
            // destroy ent and all children
            NEKO_EXPORT void transform_destroy_rec(Entity ent);

            NEKO_EXPORT void transform_set_position(Entity ent, CVec2 pos);

            NEKO_EXPORT CVec2 transform_get_position(Entity ent);

            NEKO_EXPORT void transform_translate(Entity ent, CVec2 trans);

            NEKO_EXPORT void transform_set_rotation(Entity ent, Scalar rot);

            NEKO_EXPORT Scalar transform_get_rotation(Entity ent);

            NEKO_EXPORT void transform_rotate(Entity ent, Scalar rot);

            NEKO_EXPORT void transform_set_scale(Entity ent, CVec2 scale);

            NEKO_EXPORT CVec2 transform_get_scale(Entity ent);

            NEKO_EXPORT CVec2 transform_get_world_position(Entity ent);

            NEKO_EXPORT Scalar transform_get_world_rotation(Entity ent);

            NEKO_EXPORT CVec2 transform_get_world_scale(Entity ent);

            NEKO_EXPORT CMat3 transform_get_world_matrix(Entity ent);  // world-space

            NEKO_EXPORT CMat3 transform_get_matrix(Entity ent);  // parent-space

            NEKO_EXPORT CVec2 transform_local_to_world(Entity ent, CVec2 v);

            NEKO_EXPORT CVec2 transform_world_to_local(Entity ent, CVec2 v);

            NEKO_EXPORT ecs_id_t transform_get_dirty_count(Entity ent);

            NEKO_EXPORT void transform_set_save_filter_rec(Entity ent, bool filter);

)

void transform_init();
void transform_fini();
void transform_update_all();
void transform_save_all(Store *s);
void transform_load_all(Store *s);

#endif
