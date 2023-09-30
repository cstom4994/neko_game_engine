
#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/neko.h"

// 测试 ECS 用
typedef struct {
    f32 x, y;
} CTransform;

typedef struct {
    f32 dx, dy;
} CVelocity;

enum ComponentType {
    COMPONENT_TRANSFORM,
    COMPONENT_VELOCITY,
    COMPONENT_SPRITE,
    COMPONENT_PARTICLE,

    COMPONENT_COUNT
};

// namespace neko {
// namespace meta {
// struct range {
//     float min_value;
//     float max_value;
// };
// struct info {
//     const_str info;
// };
// }  // namespace meta
// }  // namespace neko

#endif  // !NEKO_COMPONENT_H
