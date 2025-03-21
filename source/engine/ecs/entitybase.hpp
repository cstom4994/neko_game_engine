#pragma once

#include "base/common/color.hpp"
#include "engine/component.h"
#include "engine/ecs/entity.h"

struct CCamera : CEntityBase {
    f32 viewport_height;
};

struct CUnEditable : CEntityBase {
    int what;
};

struct CSprite : CEntityBase {
    mat3 wmat;  // 要发送到着色器的世界变换矩阵

    vec2 size;
    vec2 texcell;
    vec2 texsize;

    int depth;
};

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

// bbox pool
struct BBoxPoolElem : CEntityBase {
    mat3 wmat;
    BBox bbox;
    f32 selected;  // > 0.5 if and only if selected
};