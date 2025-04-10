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



// bbox pool
struct CBBoxPoolElem : CEntityBase {
    mat3 wmat;
    BBox bbox;
    f32 selected;  // > 0.5 if and only if selected
};