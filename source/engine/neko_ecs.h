

#ifndef NEKO_ENGINE_NEKO_ECS_H
#define NEKO_ENGINE_NEKO_ECS_H

// #include <flecs.h>

#include "neko.h"
#include "neko_math.h"

#ifndef NEKO_ENT_alignof
#define NEKO_ENT_alignof(type) \
    offsetof(                  \
            struct {           \
                char c;        \
                type d;        \
            },                 \
            d)
#endif

typedef unsigned short neko_entitytainer_entity;
#define NEKO_ENT_InvalidEntity ((neko_entitytainer_entity)0u)
#define NEKO_ENT_EntityFormat "%hu"
typedef unsigned short neko_entitytainer_entry;
#define NEKO_ENT_BucketMask 0x3fff
#define NEKO_ENT_BucketListBitCount 2
#define NEKO_ENT_BucketListOffset (sizeof(neko_entitytainer_entry) * 8 - NEKO_ENT_BucketListBitCount)
#define NEKO_ENT_NoFreeBucket ((neko_entitytainer_entity) - 1)
#define NEKO_ENT_ShrinkMargin 1
#define NEKO_ENT_MAX_BUCKET_LISTS 8
#define NEKO_ENT_DEFENSIVE_CHECKS 0
#define NEKO_ENT_DEFENSIVE_ASSERTS 0

struct neko_entitytainer_config {
    void* memory;
    int memory_size;
    int num_entries;
    int bucket_sizes[NEKO_ENT_MAX_BUCKET_LISTS];
    int bucket_list_sizes[NEKO_ENT_MAX_BUCKET_LISTS];
    int num_bucket_lists;
    bool remove_with_holes;
    bool keep_capacity_on_remove;
    // char  name[256];
};

typedef struct {
    neko_entitytainer_entity* bucket_data;
    int bucket_size;
    int total_buckets;
    int first_free_bucket;
    int used_buckets;
} neko_entitytainer_bucketlist;

typedef struct {
    struct neko_entitytainer_config config;
    neko_entitytainer_entry* entry_lookup;
    neko_entitytainer_entity* entry_parent_lookup;
    neko_entitytainer_bucketlist* bucket_lists;
    int num_bucket_lists;
    int entry_lookup_size;
    bool remove_with_holes;
    bool keep_capacity_on_remove;
} neko_entitytainer;

NEKO_API_DECL int neko_ent_needed_size(struct neko_entitytainer_config* config);
NEKO_API_DECL neko_entitytainer* neko_ent_create(struct neko_entitytainer_config* config);
NEKO_API_DECL neko_entitytainer* neko_ent_realloc(neko_entitytainer* entitytainer_old, void* memory, int memory_size, float growth);
NEKO_API_DECL bool neko_ent_needs_realloc(neko_entitytainer* entitytainer, float percent_free, int num_free_buckets);
NEKO_API_DECL void neko_ent_add_entity(neko_entitytainer* entitytainer, neko_entitytainer_entity entity);
NEKO_API_DECL void neko_ent_remove_entity(neko_entitytainer* entitytainer, neko_entitytainer_entity entity);
NEKO_API_DECL void neko_ent_reserve(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, int capacity);
NEKO_API_DECL void neko_ent_add_child(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child);
NEKO_API_DECL void neko_ent_add_child_at_index(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child, int index);
NEKO_API_DECL void neko_ent_remove_child_no_holes(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child);
NEKO_API_DECL void neko_ent_remove_child_with_holes(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child);
NEKO_API_DECL void neko_ent_get_children(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity** children, int* num_children, int* capacity);
NEKO_API_DECL int neko_ent_num_children(neko_entitytainer* entitytainer, neko_entitytainer_entity parent);
NEKO_API_DECL int neko_ent_get_child_index(neko_entitytainer* entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child);
NEKO_API_DECL neko_entitytainer_entity neko_ent_get_parent(neko_entitytainer* entitytainer, neko_entitytainer_entity child);
NEKO_API_DECL bool neko_ent_is_added(neko_entitytainer* entitytainer, neko_entitytainer_entity entity);
NEKO_API_DECL void neko_ent_remove_holes(neko_entitytainer* entitytainer, neko_entitytainer_entity entity);
NEKO_API_DECL int neko_ent_save(neko_entitytainer* entitytainer, unsigned char* buffer, int buffer_size);
NEKO_API_DECL neko_entitytainer* neko_ent_load(unsigned char* buffer, int buffer_size);
NEKO_API_DECL void neko_ent_load_into(neko_entitytainer* entitytainer_dst, const neko_entitytainer* entitytainer_src);

typedef u64 neko_ecs_ent;
typedef u32 neko_ecs_component_type;

#define neko_ecs_decl_system(name, mask_name, ctypes_count, ...)            \
    neko_ecs_component_type mask_name##_MASK[ctypes_count] = {__VA_ARGS__}; \
    u32 mask_name##_MASK_COUNT = ctypes_count;                              \
    void name(neko_ecs* ecs)

#define neko_ecs_get_mask(name) name##_MASK_COUNT, name##_MASK

#define __neko_ecs_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_ecs_ent_index(id) ((u32)id)
#define __neko_ecs_ent_ver(id) ((u32)(id >> 32))

typedef void (*neko_ecs_system_func)(struct neko_ecs* ecs);
typedef void (*neko_ecs_component_destroy)(void* data);

typedef enum { ECS_SYSTEM_UPDATE, ECS_SYSTEM_RENDER_IMMEDIATE, ECS_SYSTEM_RENDER_DEFERRED, ECS_SYSTEM_EDITOR } neko_ecs_system_type;

typedef struct neko_ecs_stack {
    u32* data;
    u64 capacity;
    u64 top;
    b32 empty;
} neko_ecs_stack;

typedef struct neko_ecs_component_pool {
    void* data;
    u32 count;
    u32 size;

    neko_ecs_component_destroy destroy_func;

    neko_ecs_stack* indexes;

} neko_ecs_component_pool;

typedef struct neko_ecs_system {
    neko_ecs_system_func func;
    neko_ecs_system_type type;
} neko_ecs_system;

typedef struct neko_ecs {
    u32 max_entities;
    u32 component_count;
    u32 system_count;

    neko_ecs_stack* indexes;

    // max_index 用来优化
    u32 max_index;
    u32* versions;

    // components 是组件的索引
    // 最大值为 (实体数 * component_count)
    // 索引通过 (index * comp_count + comp_type) 实现
    // component_masks 的工作原理相同 只是检查 mask 是否启用
    u32* components;
    b32* component_masks;

    neko_ecs_component_pool* pool;

    neko_ecs_system* systems;
    u32 systems_top;

    // 额外数据
    void* user_data;
} neko_ecs;

NEKO_API_DECL neko_ecs* neko_ecs_make(u32 max_entities, u32 component_count, u32 system_count);
NEKO_API_DECL void neko_ecs_destroy(neko_ecs* ecs);
NEKO_API_DECL void neko_ecs_register_component(neko_ecs* ecs, neko_ecs_component_type component_type, u32 count, u32 size, neko_ecs_component_destroy destroy_func);
NEKO_API_DECL void neko_ecs_register_system(neko_ecs* ecs, neko_ecs_system_func func, neko_ecs_system_type type);
NEKO_API_DECL void neko_ecs_run_systems(neko_ecs* ecs, neko_ecs_system_type type);
NEKO_API_DECL void neko_ecs_run_system(neko_ecs* ecs, u32 system_index);
NEKO_API_DECL u32 neko_ecs_for_count(neko_ecs* ecs);
NEKO_API_DECL neko_ecs_ent neko_ecs_get_ent(neko_ecs* ecs, u32 index);
NEKO_API_DECL neko_ecs_ent neko_ecs_ent_make(neko_ecs* ecs);
NEKO_API_DECL void neko_ecs_ent_destroy(neko_ecs* ecs, neko_ecs_ent e);
NEKO_API_DECL void neko_ecs_ent_add_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type type, void* component_data);
NEKO_API_DECL void neko_ecs_ent_remove_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type type);
NEKO_API_DECL void* neko_ecs_ent_get_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type type);
NEKO_API_DECL b32 neko_ecs_ent_has_component(neko_ecs* ecs, neko_ecs_ent e, neko_ecs_component_type component_type);
NEKO_API_DECL b32 neko_ecs_ent_has_mask(neko_ecs* ecs, neko_ecs_ent e, u32 component_type_count, neko_ecs_component_type component_types[]);
NEKO_API_DECL b32 neko_ecs_ent_is_valid(neko_ecs* ecs, neko_ecs_ent e);
NEKO_API_DECL u32 neko_ecs_ent_get_version(neko_ecs* ecs, neko_ecs_ent e);
NEKO_API_DECL void neko_ecs_ent_print(neko_ecs* ecs, neko_ecs_ent e);

typedef struct neko_ecs_ent_view {
    neko_ecs* ecs;
    neko_ecs_ent ent;
    neko_ecs_component_type view_type;
    u32 i;
    b32 valid;
} neko_ecs_ent_view;

NEKO_API_DECL neko_ecs_ent_view neko_ecs_ent_view_single(neko_ecs* ecs, neko_ecs_component_type component_type);
NEKO_API_DECL b32 neko_ecs_ent_view_valid(neko_ecs_ent_view* view);
NEKO_API_DECL void neko_ecs_ent_view_next(neko_ecs_ent_view* view);

#define MAX_ENTITY_COUNT 100

// 测试 ECS 用
typedef struct {
    char name[64];
    bool active;
    bool visible;
    bool selected;
} CGameObjectTest;

// typedef struct {
//     f32 x, y;
// } CTransform;
//
// typedef struct {
//     f32 dx, dy;
// } CVelocity;

enum ComponentType {
    COMPONENT_GAMEOBJECT,
    COMPONENT_TRANSFORM,
    COMPONENT_VELOCITY,
    COMPONENT_BOUNDS,
    COMPONENT_COLOR,

    COMPONENT_COUNT
};

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
