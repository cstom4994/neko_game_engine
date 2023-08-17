#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "engine/common/neko_mem.h"
#include "engine/common/neko_util.h"

typedef u64 neko_ecs_ent;
typedef u32 neko_ecs_component_type;

#define neko_ecs_decl_mask(name, ctypes_count, ...)                    \
    neko_ecs_component_type name##_MASK[ctypes_count] = {__VA_ARGS__}; \
    u32 name##_MASK_COUNT = ctypes_count

#define neko_ecs_get_mask(name) name##_MASK_COUNT, name##_MASK

typedef enum { ECS_SYSTEM_UPDATE, ECS_SYSTEM_RENDER_IMMEDIATE } neko_ecs_system_type;

typedef struct neko_ecs neko_ecs;

typedef void (*neko_ecs_system_func)(struct neko_ecs *ecs);
typedef void (*neko_ecs_component_destroy)(void *data);

neko_ecs *neko_ecs_make(u32 max_entities, u32 component_count, u32 system_count);
void neko_ecs_destroy(neko_ecs *ecs);
void neko_ecs_register_component(neko_ecs *ecs, neko_ecs_component_type component_type, u32 count, u32 size, neko_ecs_component_destroy destroy_func);
void neko_ecs_register_system(neko_ecs *ecs, neko_ecs_system_func func, neko_ecs_system_type type);
void neko_ecs_run_systems(neko_ecs *ecs, neko_ecs_system_type type);
void neko_ecs_run_system(neko_ecs *ecs, u32 system_index);
u32 neko_ecs_for_count(neko_ecs *ecs);
neko_ecs_ent neko_ecs_get_ent(neko_ecs *ecs, u32 index);
neko_ecs_ent neko_ecs_ent_make(neko_ecs *ecs);
void neko_ecs_ent_destroy(neko_ecs *ecs, neko_ecs_ent e);
void neko_ecs_ent_add_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type, void *component_data);
void neko_ecs_ent_remove_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type);
void *neko_ecs_ent_get_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type);
bool neko_ecs_ent_has_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type component_type);
bool neko_ecs_ent_has_mask(neko_ecs *ecs, neko_ecs_ent e, u32 component_type_count, neko_ecs_component_type component_types[]);
bool neko_ecs_ent_is_valid(neko_ecs *ecs, neko_ecs_ent e);
u32 neko_ecs_ent_get_version(neko_ecs *ecs, neko_ecs_ent e);
void neko_ecs_ent_print(neko_ecs *ecs, neko_ecs_ent e);

#endif
