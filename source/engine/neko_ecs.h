

#ifndef NEKO_ENGINE_NEKO_ECS_H
#define NEKO_ENGINE_NEKO_ECS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "neko.h"
#include "neko_lua.h"
#include "neko_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u64 ecs_entity_t;

// -- MAP --------------------------------------------------------------------
// type unsafe hashtable

typedef struct ecs_map_t ecs_map_t;

typedef u32 (*ecs_hash_fn)(const void *);
typedef bool (*ecs_key_equal_fn)(const void *, const void *);

#define ECS_MAP(fn, k, v, capacity) ecs_map_new(sizeof(k), sizeof(v), ecs_map_hash_##fn, ecs_map_equal_##fn, capacity)

ecs_map_t *ecs_map_new(size_t key_size, size_t item_size, ecs_hash_fn hash_fn, ecs_key_equal_fn key_equal_fn, u32 capacity);
void ecs_map_free(ecs_map_t *map);
void *ecs_map_get(const ecs_map_t *map, const void *key);
void ecs_map_set(ecs_map_t *map, const void *key, const void *payload);
void ecs_map_remove(ecs_map_t *map, const void *key);
void *ecs_map_values(ecs_map_t *map);
u32 ecs_map_len(ecs_map_t *map);
u32 ecs_map_hash_intptr(const void *key);
u32 ecs_map_hash_string(const void *key);
u32 ecs_map_hash_type(const void *key);
bool ecs_map_equal_intptr(const void *a, const void *b);
bool ecs_map_equal_string(const void *a, const void *b);
bool ecs_map_equal_type(const void *a, const void *b);

#define ECS_MAP_VALUES_EACH(map, T, var, ...)                     \
    do {                                                          \
        u32 var##_count = ecs_map_len(map);                       \
        T *var##_values = ecs_map_values(map);                    \
        for (u32 var##_i = 0; var##_i < var##_count; var##_i++) { \
            T *var = &var##_values[var##_i];                      \
            __VA_ARGS__                                           \
        }                                                         \
    } while (0)

#ifndef NDEBUG
void ecs_map_inspect(ecs_map_t *map);  // 假设键和值都是整数
#endif

// -- TYPE -------------------------------------------------------------------
// 按排序顺序排列的组件 ID 集

typedef struct ecs_type_t ecs_type_t;

ecs_type_t *ecs_type_new(u32 capacity);
void ecs_type_free(ecs_type_t *type);
ecs_type_t *ecs_type_copy(const ecs_type_t *from);
u32 ecs_type_len(const ecs_type_t *type);
bool ecs_type_equal(const ecs_type_t *a, const ecs_type_t *b);
int32_t ecs_type_index_of(const ecs_type_t *type, ecs_entity_t e);
void ecs_type_add(ecs_type_t *type, ecs_entity_t e);
void ecs_type_remove(ecs_type_t *type, ecs_entity_t e);
bool ecs_type_is_superset(const ecs_type_t *super, const ecs_type_t *sub);

#define ECS_TYPE_ADD(type, e, s) ecs_type_add(type, (ecs_component_t){e, sizeof(s)});

#define ECS_TYPE_EACH(type, var, ...)                             \
    do {                                                          \
        u32 var##_count = ecs_type_len(type);                     \
        for (u32 var##_i = 0; var##_i < var##_count; var##_i++) { \
            ecs_entity_t var = type->elements[var##_i];           \
            __VA_ARGS__                                           \
        }                                                         \
    } while (0)

#ifndef NDEBUG
void ecs_type_inspect(ecs_type_t *type);
#endif

// -- SIGNATURE --------------------------------------------------------------
// component ids in a defined order

typedef struct ecs_signature_t ecs_signature_t;

ecs_signature_t *ecs_signature_new(u32 count);
ecs_signature_t *ecs_signature_new_n(u32 count, ...);
void ecs_signature_free(ecs_signature_t *sig);
ecs_type_t *ecs_signature_as_type(const ecs_signature_t *sig);

// -- EDGE LIST --------------------------------------------------------------
// archetype edges for graph traversal

typedef struct ecs_edge_t ecs_edge_t;
typedef struct ecs_edge_list_t ecs_edge_list_t;

ecs_edge_list_t *ecs_edge_list_new(void);
void ecs_edge_list_free(ecs_edge_list_t *edge_list);
u32 ecs_edge_list_len(const ecs_edge_list_t *edge_list);
void ecs_edge_list_add(ecs_edge_list_t *edge_list, ecs_edge_t edge);
void ecs_edge_list_remove(ecs_edge_list_t *edge_list, ecs_entity_t component);

#define ECS_EDGE_LIST_EACH(edge_list, var, ...)                   \
    do {                                                          \
        u32 var##_count = ecs_edge_list_len(edge_list);           \
        for (u32 var##_i = 0; var##_i < var##_count; var##_i++) { \
            ecs_edge_t var = edge_list->edges[var##_i];           \
            __VA_ARGS__                                           \
        }                                                         \
    } while (0)

// -- ARCHETYPE --------------------------------------------------------------
// graph vertex. archetypes are tables where columns represent component data
// and rows represent each entity. left edges point to other archetypes with
// one less component, and right edges point to archetypes that store one
// additional component.

typedef struct ecs_archetype_t ecs_archetype_t;

ecs_archetype_t *ecs_archetype_new(ecs_type_t *type, const ecs_map_t *component_index, ecs_map_t *type_index);
void ecs_archetype_free(ecs_archetype_t *archetype);
u32 ecs_archetype_add(ecs_archetype_t *archetype, const ecs_map_t *component_index, ecs_map_t *entity_index, ecs_entity_t e);
u32 ecs_archetype_move_entity_right(ecs_archetype_t *left, ecs_archetype_t *right, const ecs_map_t *component_index, ecs_map_t *entity_index, u32 left_row);
ecs_archetype_t *ecs_archetype_insert_vertex(ecs_archetype_t *root, ecs_archetype_t *left_neighbour, ecs_type_t *new_vertex_type, ecs_entity_t component_for_edge, const ecs_map_t *component_index,
                                             ecs_map_t *type_index);
ecs_archetype_t *ecs_archetype_traverse_and_create(ecs_archetype_t *root, const ecs_type_t *type, const ecs_map_t *component_index, ecs_map_t *type_index);

// -- ENTITY COMPONENT SYSTEM ------------------------------------------------
// 下面的函数是预期的公共 API

typedef struct ecs_view_t {
    void **component_arrays;
    u32 *signature_to_index;
    u32 *component_sizes;
} ecs_view_t;

typedef void (*ecs_system_fn)(ecs_view_t, u32);

typedef struct neko_ecs_t neko_ecs_t;

neko_ecs_t *ecs_init_i(neko_ecs_t *registry);
neko_ecs_t *ecs_init(lua_State *L);
void ecs_fini_i(neko_ecs_t *registry);
void ecs_fini(neko_ecs_t *registry);
ecs_entity_t ecs_entity(neko_ecs_t *registry);
ecs_entity_t ecs_component_w(neko_ecs_t *registry, const_str component_name, size_t component_size);
ecs_entity_t ecs_system(neko_ecs_t *registry, ecs_signature_t *signature, ecs_system_fn system);
void ecs_attach(neko_ecs_t *registry, ecs_entity_t entity, ecs_entity_t component);
void ecs_set(neko_ecs_t *registry, ecs_entity_t entity, ecs_entity_t component, const void *data);
void ecs_step(neko_ecs_t *registry);
void *ecs_view(ecs_view_t view, u32 row, u32 column);

#ifndef NDEBUG
void ecs_inspect(neko_ecs_t *registry);
#endif

#define ECS_COMPONENT(registry, T) ecs_component_w(registry, #T, sizeof(T));
#define ECS_SYSTEM(registry, system, n, ...) ecs_system(registry, ecs_signature_new_n(n, __VA_ARGS__), system)

#ifdef __cplusplus
}  // extern "C"
#endif

#define __neko_ecs_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_ecs_ent_index(id) ((u32)id)
#define __neko_ecs_ent_ver(id) ((u32)(id >> 32))

#define MAX_ENTITY_COUNT 100

// 测试 ECS 用
typedef struct {
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
