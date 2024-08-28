
#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "engine/base.h"

#define __neko_ecs_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_ecs_ent_index(id) ((u32)id)
#define __neko_ecs_ent_ver(id) ((u32)(id >> 32))

#define MAX_ENTITY_COUNT 100

enum ECS_LUA_UPVALUES { NEKO_ECS_COMPONENTS_NAME = 1, NEKO_ECS_UPVAL_N };

#define TYPE_MIN_ID 1
#define TYPE_MAX_ID 255
#define TYPE_COUNT 256

#define WORLD_PROTO_ID 1
#define WORLD_PROTO_DEFINE 2
#define WORLD_COMPONENTS 3
#define WORLD_MATCH_CTX 4
#define WORLD_KEY_EID 5
#define WORLD_KEY_TID 6
#define WORLD_UPVAL_N 6

#define LINK_NIL (-1)
#define LINK_NONE (-2)

#define ENTITY_MAX_COMPONENTS (64)  // 最大组件数量

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

NEKO_API() int l_ecs_create_world(lua_State* L);

typedef struct ecs_s ecs_t;
typedef uint32_t ecs_id_t;

// NULL/invalid/undefined value
#define ECS_NULL ((ecs_id_t) - 1)

typedef int8_t ecs_ret_t;

NEKO_API() ecs_id_t ecs_component_w(ecs_t* registry, const_str component_name, size_t component_size);

// #define ECS_COMPONENT(type, ctor, dtor) ecs_component_w(ENGINE_ECS(), #type, sizeof(type), ctor, dtor)
// #define ECS_COMPONENT_DEFINE(type, ctor, dtor) ECS_COMPONENT_ID(type) = ECS_COMPONENT(type, ctor, dtor)

// #define ECS_COMPONENT_ID(type) __NEKO_GEN_COMPONENT_##type
// #define ECS_COMPONENT_DECL(type) ecs_id_t ECS_COMPONENT_ID(type)
// #define ECS_COMPONENT_EXTERN(type) extern ecs_id_t ECS_COMPONENT_ID(type)

NEKO_SCRIPT(
        entity,

        typedef struct NativeEntity NativeEntity;

        typedef uint32_t ecs_id_t;

        struct NativeEntity { ecs_id_t id; };

        NEKO_EXPORT NativeEntity entity_nil;  // 没有有效的实体具有此值

        NEKO_EXPORT NativeEntity entity_create();  // 声明未使用的实体 ID

        NEKO_EXPORT void entity_destroy(NativeEntity ent);  // 释放实体 ID

        NEKO_EXPORT void entity_destroy_all();

        NEKO_EXPORT bool entity_destroyed(NativeEntity ent);

        NEKO_EXPORT bool native_entity_eq(NativeEntity e, NativeEntity f);

        //  如果为任何实体设置为 TRUE 则仅保存设置为 TRUE 的那些实体
        //  如果设置为 False 则不会保存此项的实体

        NEKO_EXPORT void entity_set_save_filter(NativeEntity ent, bool filter);

        NEKO_EXPORT bool entity_get_save_filter(NativeEntity ent);

        NEKO_EXPORT void entity_clear_save_filters();

        // save/load 仅适用于 ID
        NEKO_EXPORT void entity_save(NativeEntity* ent, const char* name, Store* s);

        NEKO_EXPORT bool entity_load(NativeEntity* ent, const char* name, NativeEntity d, Store* s);

        // 获取用于合并的已解析 ID 供内部使用
        NEKO_EXPORT NativeEntity _entity_resolve_saved_id(ecs_id_t id);

)

NEKO_API() void entity_init();
NEKO_API() void entity_fini();
NEKO_API() void entity_update_all();

NEKO_API() void entity_save_all(Store* s);
NEKO_API() void entity_load_all(Store* s);
NEKO_API() void entity_load_all_begin();
NEKO_API() void entity_load_all_end();

#define native_entity_eq(e, f) ((e).id == (f).id)

typedef struct NativeEntityMap {
    int* arr;
    ecs_id_t bound;     // 1 + 最大键
    ecs_id_t capacity;  // 拥有堆空间的元素数量
    int def;            // 未设置键的值
} NativeEntityMap;

NEKO_API() NativeEntityMap* entitymap_new(int def);  // def 为默认值
NEKO_API() void entitymap_clear(NativeEntityMap* emap);
NEKO_API() void entitymap_free(NativeEntityMap* emap);

NEKO_API() void entitymap_set(NativeEntityMap* emap, NativeEntity ent, int val);
NEKO_API() int entitymap_get(NativeEntityMap* emap, NativeEntity ent);

// 在内存中连续 可能会被重新定位/打乱 所以要小心
typedef struct NativeEntityPool NativeEntityPool;

// 该结构必须位于池元素的顶部
// struct Data
// {
//  EntityPoolElem pool_elem;
//  ...
// }
// 其中的值是EntityPool管理的元数据
typedef struct EntityPoolElem {
    NativeEntity ent;  // 该元素的键
} EntityPoolElem;

#define DECL_ENT(T, ...)          \
    typedef struct T {            \
        EntityPoolElem pool_elem; \
        __VA_ARGS__               \
    } T

// object_size 是每个元素的大小
NEKO_API() NativeEntityPool* entitypool_new_(size_t object_size);
#define entitypool_new(type) entitypool_new_(sizeof(type))
NEKO_API() void entitypool_free(NativeEntityPool* pool);

NEKO_API() void* entitypool_add(NativeEntityPool* pool, NativeEntity ent);
NEKO_API() void entitypool_remove(NativeEntityPool* pool, NativeEntity ent);
NEKO_API() void* entitypool_get(NativeEntityPool* pool, NativeEntity ent);  // 如果未映射则为 NULL

// 由于元素是连续的 因此可以使用指针进行迭代:
//     for (ptr = entitypool_begin(pool), end = entitypool_end(pool);
//             ptr != end; ++ptr)
//         ... use ptr ...
// 如果在迭代期间添加/删除指针将无效
NEKO_API() void* entitypool_begin(NativeEntityPool* pool);
NEKO_API() void* entitypool_end(NativeEntityPool* pool);              // one-past-end
NEKO_API() void* entitypool_nth(NativeEntityPool* pool, ecs_id_t n);  // 0-indexed

NEKO_API() ecs_id_t entitypool_size(NativeEntityPool* pool);

NEKO_API() void entitypool_clear(NativeEntityPool* pool);

NEKO_API() void entitypool_sort(NativeEntityPool* pool, int (*compar)(const void*, const void*));

// elem must be /pointer to/ pointer to element
NEKO_API() void entitypool_elem_save(NativeEntityPool* pool, void* elem, Store* s);
NEKO_API() void entitypool_elem_load(NativeEntityPool* pool, void* elem, Store* s);

/*
 * call 'func' on each destroyed NativeEntity, generally done in
 * *_update_all() -- check transform.c, sprite.c, etc. for examples
 */
#define entitypool_remove_destroyed(pool, func)               \
    do {                                                      \
        ecs_id_t __i;                                         \
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
 * save/load each element of an NativeEntityPool -- var is the variable used
 * to iterate over the pool, var_s is the Store pointer to use for the
 * Stores created/loaded per element, pool is the NativeEntityPool, n is the
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

#ifndef SYSTEM_H
#define SYSTEM_H

#include "engine/base.h"
#include "engine/entity.h"

NEKO_SCRIPT(system,

            NEKO_EXPORT void system_load_all(Store* f);
            NEKO_EXPORT void system_save_all(Store* f);

)

struct command_buffer_t;

void system_init();
void system_fini();
void system_update_all();
void system_draw_all(command_buffer_t* cb);

NEKO_SCRIPT(prefab,

            // 将所有过滤的实体保存为预制件 并将root作为根
            NEKO_EXPORT void prefab_save(const char* filename, NativeEntity root);

            // 加载已保存的预制件 返回已保存的根实体
            NEKO_EXPORT NativeEntity prefab_load(const char* filename);

)

void prefab_save_all(Store* s);
void prefab_load_all(Store* s);

#endif
