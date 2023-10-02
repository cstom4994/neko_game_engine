#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "engine/neko.h"

typedef u64 neko_ecs_ent;
typedef u32 neko_ecs_component_type;

#define neko_ecs_decl_mask(name, ctypes_count, ...)                    \
    neko_ecs_component_type name##_MASK[ctypes_count] = {__VA_ARGS__}; \
    u32 name##_MASK_COUNT = ctypes_count

#define neko_ecs_get_mask(name) name##_MASK_COUNT, name##_MASK

#define __neko_ecs_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_ecs_ent_index(id) ((u32)id)
#define __neko_ecs_ent_ver(id) ((u32)(id >> 32))

typedef void (*neko_ecs_system_func)(struct neko_ecs *ecs);
typedef void (*neko_ecs_component_destroy)(void *data);

typedef enum { ECS_SYSTEM_UPDATE, ECS_SYSTEM_RENDER_IMMEDIATE } neko_ecs_system_type;

typedef struct {
    u32 *data;
    u64 capacity;
    u64 top;
    b32 empty;
} neko_ecs_stack;

typedef struct {
    void *data;
    u32 count;
    u32 size;

    neko_ecs_component_destroy destroy_func;

    neko_ecs_stack *indexes;

} neko_ecs_component_pool;

typedef struct {
    neko_ecs_system_func func;
    neko_ecs_system_type type;
} neko_ecs_system;

typedef struct neko_ecs {
    u32 max_entities;
    u32 component_count;
    u32 system_count;

    neko_ecs_stack *indexes;

    // max_index 用来优化
    u32 max_index;
    u32 *versions;

    // components 是组件的索引
    // 最大值为 (实体数 * component_count)
    // 索引通过 (index * comp_count + comp_type) 实现
    // component_masks 的工作原理相同 只是检查 mask 是否启用
    u32 *components;
    b32 *component_masks;

    neko_ecs_component_pool *pool;

    neko_ecs_system *systems;
    u32 systems_top;

    // 额外数据
    void *user_data;
} neko_ecs;

NEKO_API_DECL neko_ecs *neko_ecs_make(u32 max_entities, u32 component_count, u32 system_count);
NEKO_API_DECL void neko_ecs_destroy(neko_ecs *ecs);
NEKO_API_DECL void neko_ecs_register_component(neko_ecs *ecs, neko_ecs_component_type component_type, u32 count, u32 size, neko_ecs_component_destroy destroy_func);
NEKO_API_DECL void neko_ecs_register_system(neko_ecs *ecs, neko_ecs_system_func func, neko_ecs_system_type type);
NEKO_API_DECL void neko_ecs_run_systems(neko_ecs *ecs, neko_ecs_system_type type);
NEKO_API_DECL void neko_ecs_run_system(neko_ecs *ecs, u32 system_index);
NEKO_API_DECL u32 neko_ecs_for_count(neko_ecs *ecs);
NEKO_API_DECL neko_ecs_ent neko_ecs_get_ent(neko_ecs *ecs, u32 index);
NEKO_API_DECL neko_ecs_ent neko_ecs_ent_make(neko_ecs *ecs);
NEKO_API_DECL void neko_ecs_ent_destroy(neko_ecs *ecs, neko_ecs_ent e);
NEKO_API_DECL void neko_ecs_ent_add_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type, void *component_data);
NEKO_API_DECL void neko_ecs_ent_remove_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type);
NEKO_API_DECL void *neko_ecs_ent_get_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type);
NEKO_API_DECL b32 neko_ecs_ent_has_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type component_type);
NEKO_API_DECL b32 neko_ecs_ent_has_mask(neko_ecs *ecs, neko_ecs_ent e, u32 component_type_count, neko_ecs_component_type component_types[]);
NEKO_API_DECL b32 neko_ecs_ent_is_valid(neko_ecs *ecs, neko_ecs_ent e);
NEKO_API_DECL u32 neko_ecs_ent_get_version(neko_ecs *ecs, neko_ecs_ent e);
NEKO_API_DECL void neko_ecs_ent_print(neko_ecs *ecs, neko_ecs_ent e);

#endif
