#pragma once

#include "base/common/color.hpp"
#include "engine/component.h"
#include "engine/ecs/entity.h"

// DECL_ENT(Camera, f32 viewport_height;);

class Camera : public GameEntityBase {
public:
    static NativeEntityPool<Camera>* pool;

public:
    f32 viewport_height;
};

DECL_ENT(uneditable, int what;);

DECL_ENT(Sprite, mat3 wmat;  // 要发送到着色器的世界变换矩阵

         vec2 size; vec2 texcell; vec2 texsize;

         int depth;);

DECL_ENT(Transform, vec2 position; f32 rotation; vec2 scale; NativeEntity parent;  // 如果entity_nil 则为 root
         Array<NativeEntity> children;                                             // 如果为 NULL 则为空
         mat3 mat_cache;                                                           // 更新此内容
         mat3 worldmat_cache;                                                      // 在父子更新时缓存
         ecs_id_t dirty_count;);

// bbox pool
DECL_ENT(BBoxPoolElem, mat3 wmat; BBox bbox; f32 selected;  // > 0.5 if and only if selected
);