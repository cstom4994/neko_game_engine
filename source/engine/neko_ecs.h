
#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "neko_prelude.h"

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

#if 0
NEKO_API_DECL ECS_COMPONENT_DECLARE(position_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(velocity_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(bounds_t);
NEKO_API_DECL ECS_COMPONENT_DECLARE(color_t);

NEKO_API_DECL void neko_ecs_com_init(ecs_world_t* world);
#endif

int neko_ecs_create_world(lua_State* L);

typedef struct ecs_s ecs_t;
typedef uint32_t ecs_id_t;

// NULL/invalid/undefined value
#define ECS_NULL ((ecs_id_t) - 1)

typedef int8_t ecs_ret_t;

#ifndef ECS_DT_TYPE
#define ECS_DT_TYPE double
#endif

typedef ECS_DT_TYPE ecs_dt_t;

ecs_t* ecs_init(lua_State* L);
ecs_t* ecs_new(size_t entity_count, void* mem_ctx);
void ecs_free(ecs_t* ecs);
void ecs_reset(ecs_t* ecs);  // 从 ECS 中删除所有实体 保留系统和组件

typedef void (*ecs_constructor_fn)(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args);
typedef void (*ecs_destructor_fn)(ecs_t* ecs, ecs_id_t entity_id, void* ptr);
typedef ecs_ret_t (*ecs_system_fn)(ecs_t* ecs, ecs_id_t* entities, int entity_count, ecs_dt_t dt, void* udata);
typedef void (*ecs_added_fn)(ecs_t* ecs, ecs_id_t entity_id, void* udata);
typedef void (*ecs_removed_fn)(ecs_t* ecs, ecs_id_t entity_id, void* udata);

ecs_id_t ecs_register_component(ecs_t* ecs, size_t size, ecs_constructor_fn constructor, ecs_destructor_fn destructor);

ecs_id_t ecs_register_system(ecs_t* ecs, ecs_system_fn system_cb, ecs_added_fn add_cb, ecs_removed_fn remove_cb, void* udata);
void ecs_require_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id);
void ecs_exclude_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id);
void ecs_enable_system(ecs_t* ecs, ecs_id_t sys_id);
void ecs_disable_system(ecs_t* ecs, ecs_id_t sys_id);
ecs_id_t ecs_create(ecs_t* ecs);
bool ecs_is_ready(ecs_t* ecs, ecs_id_t entity_id);
void ecs_destroy(ecs_t* ecs, ecs_id_t entity_id);
bool ecs_has(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);
void* ecs_add(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id, void* args);
void* ecs_get(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);
void ecs_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);
void ecs_queue_destroy(ecs_t* ecs, ecs_id_t entity_id);
void ecs_queue_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

ecs_ret_t ecs_update_system(ecs_t* ecs, ecs_id_t sys_id, ecs_dt_t dt);
ecs_ret_t ecs_update_systems(ecs_t* ecs, ecs_dt_t dt);

ecs_id_t ecs_component_w(ecs_t* registry, const_str component_name, size_t component_size, ecs_constructor_fn constructor, ecs_destructor_fn destructor);

#define ECS_COMPONENT(type, ctor, dtor) ecs_component_w(ENGINE_ECS(), #type, sizeof(type), ctor, dtor)
#define ECS_COMPONENT_DEFINE(type, ctor, dtor) ECS_COMPONENT_ID(type) = ECS_COMPONENT(type, ctor, dtor)

#define ECS_COMPONENT_ID(type) __NEKO_GEN_COMPONENT_##type
#define ECS_COMPONENT_DECL(type) ecs_id_t ECS_COMPONENT_ID(type)
#define ECS_COMPONENT_EXTERN(type) extern ecs_id_t ECS_COMPONENT_ID(type)

#endif