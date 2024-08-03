
#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "engine/neko_base.h"
#include "engine/neko_prelude.h"
#include "engine/script_export.h"

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

SCRIPT(
        entity,

        typedef struct Entity Entity;

        struct Entity { unsigned int id; };

        NEKO_EXPORT Entity entity_nil;  // 没有有效的实体具有此值

        NEKO_EXPORT Entity entity_create();  // 声明未使用的实体 ID

        NEKO_EXPORT void entity_destroy(Entity ent);  // 释放实体 ID

        NEKO_EXPORT void entity_destroy_all();

        NEKO_EXPORT bool entity_destroyed(Entity ent);

        NEKO_EXPORT bool entity_eq(Entity e, Entity f);

        //  如果为任何实体设置为 TRUE 则仅保存设置为 TRUE 的那些实体
        //  如果设置为 False 则不会保存此项的实体

        NEKO_EXPORT void entity_set_save_filter(Entity ent, bool filter);

        NEKO_EXPORT bool entity_get_save_filter(Entity ent);

        NEKO_EXPORT void entity_clear_save_filters();

        // save/load 仅适用于 ID
        NEKO_EXPORT void entity_save(Entity* ent, const char* name, Store* s);

        NEKO_EXPORT bool entity_load(Entity* ent, const char* name, Entity d, Store* s);

        // 获取用于合并的已解析 ID 供内部使用
        NEKO_EXPORT Entity _entity_resolve_saved_id(unsigned int id);

)

void entity_init();
void entity_deinit();
void entity_update_all();

void entity_save_all(Store* s);
void entity_load_all(Store* s);
void entity_load_all_begin();
void entity_load_all_end();

#define entity_eq(e, f) ((e).id == (f).id)

/*
 * map of Entity -> int
 */

typedef struct EntityMap EntityMap;

EntityMap* entitymap_new(int def);  // def is value for unset keys
void entitymap_clear(EntityMap* emap);
void entitymap_free(EntityMap* emap);

void entitymap_set(EntityMap* emap, Entity ent, int val);
int entitymap_get(EntityMap* emap, Entity ent);

/*
 * continuous in memory, may be relocated/shuffled so be careful
 */

typedef struct EntityPool EntityPool;

/*
 * this struct must be at the top of pool elements:
 *
 *     struct Data
 *     {
 *         EntityPoolElem pool_elem;
 *
 *         ...
 *     }
 *
 * values in it are metadata managed by EntityPool
 *
 * look at transform.c, sprite.c, etc. for examples
 */
typedef struct EntityPoolElem EntityPoolElem;
struct EntityPoolElem {
    Entity ent;  // key for this element
};

// object_size is size per element
EntityPool* entitypool_new_(size_t object_size);
#define entitypool_new(type) entitypool_new_(sizeof(type))
void entitypool_free(EntityPool* pool);

void* entitypool_add(EntityPool* pool, Entity ent);
void entitypool_remove(EntityPool* pool, Entity ent);
void* entitypool_get(EntityPool* pool, Entity ent);  // NULL if not mapped

/* since elements are contiguoous, can iterate with pointers:
 *
 *     for (ptr = entitypool_begin(pool), end = entitypool_end(pool);
 *             ptr != end; ++ptr)
 *         ... use ptr ...
 *
 * NOTE: if you add/remove during iteration the pointers are invalidated
 */
void* entitypool_begin(EntityPool* pool);
void* entitypool_end(EntityPool* pool);                  // one-past-end
void* entitypool_nth(EntityPool* pool, unsigned int n);  // 0-indexed

unsigned int entitypool_size(EntityPool* pool);

void entitypool_clear(EntityPool* pool);

// compare is a comparator function like for qsort(3)
void entitypool_sort(EntityPool* pool, int (*compar)(const void*, const void*));

// elem must be /pointer to/ pointer to element
void entitypool_elem_save(EntityPool* pool, void* elem, Store* s);
void entitypool_elem_load(EntityPool* pool, void* elem, Store* s);

/*
 * call 'func' on each destroyed Entity, generally done in
 * *_update_all() -- check transform.c, sprite.c, etc. for examples
 */
#define entitypool_remove_destroyed(pool, func)               \
    do {                                                      \
        unsigned int __i;                                     \
        EntityPoolElem* __e;                                  \
        for (__i = 0; __i < entitypool_size(pool);) {         \
            __e = (EntityPoolElem*)entitypool_nth(pool, __i); \
            if (entity_destroyed(__e->ent))                   \
                func(__e->ent);                               \
            else                                              \
                ++__i;                                        \
        }                                                     \
    } while (0)

/*
 * can be used as:
 *
 *     entitypool_foreach(var, pool)
 *         ... use var ...
 *
 * here var must name a variable of type 'pointer to element' declared
 * before -- do not use this if adding/removing from pool while
 * iterating
 *
 * elements are visited in order of increasing index
 */
#define entitypool_foreach(var, pool) for (void* __end = (var = (decltype(var))entitypool_begin(pool), entitypool_end(pool)); var != __end; ++var)

/*
 * save/load each element of an EntityPool -- var is the variable used
 * to iterate over the pool, var_s is the Store pointer to use for the
 * Stores created/loaded per element, pool is the EntityPool, n is the
 * name to save/load the entire pool under, and finally s is the
 * parent pool to save under
 *
 * respects entity save filtering
 *
 * see transform, sprite etc. for example usage
 */
#define entitypool_save_foreach(var, var_s, pool, n, s)                                 \
    for (Store* pool##_s__ = NULL; !pool##_s__ && store_child_save(&pool##_s__, n, s);) \
    entitypool_foreach(var, pool) if (entity_get_save_filter(((EntityPoolElem*)var)->ent)) if (store_child_save(&var_s, NULL, pool##_s__)) if ((entitypool_elem_save(pool, &var, var_s)), 1)
#define entitypool_load_foreach(var, var_s, pool, n, s)                                 \
    for (Store* pool##_s__ = NULL; !pool##_s__ && store_child_load(&pool##_s__, n, s);) \
        while (store_child_load(&var_s, NULL, pool##_s__))                              \
            if ((entitypool_elem_load(pool, &var, var_s)), 1)

#endif