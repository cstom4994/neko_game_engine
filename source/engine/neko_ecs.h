

#ifndef NEKO_ENGINE_NEKO_ECS_H
#define NEKO_ENGINE_NEKO_ECS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "neko.h"
#include "neko_lua.h"
#include "neko_math.h"

#define __neko_ecs_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_ecs_ent_index(id) ((u32)id)
#define __neko_ecs_ent_ver(id) ((u32)(id >> 32))

#define MAX_ENTITY_COUNT 100

// 测试 ECS 用
typedef struct CGameObjectTest {
    char name[64];
    bool active;
    bool visible;
    bool selected;
} CGameObjectTest;

enum ComponentType {
    COMPONENT_GAMEOBJECT,
    COMPONENT_TRANSFORM,
    COMPONENT_VELOCITY,
    COMPONENT_BOUNDS,
    COMPONENT_COLOR,

    COMPONENT_COUNT
};

#define ECS_WORLD_UDATA_NAME "__NEKO_ECS_WORLD"
#define ECS_WORLD (1)

typedef neko_vec2_t position_t;  // Position component
typedef neko_vec2_t velocity_t;  // Velocity component
typedef neko_vec2_t bounds_t;    // Bounds component
typedef neko_color_t color_t;    // Color component

#if 0
NEKO_API_DECL ECS_COMPONENT_DECLARE(position_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(velocity_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(bounds_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(color_t);

NEKO_API_DECL void neko_ecs_com_init(ecs_world_t* world);
#endif

#endif  // NEKO_ENGINE_NEKO_ECS_H
