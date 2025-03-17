
#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "engine/base.hpp"
#include "engine/event.h"

// 测试 ECS 用
typedef struct CGameObjectTest {
    char name[64];
    bool active;
    bool visible;
    bool selected;
} CGameObjectTest;

enum ComponentTypeE {
    COMPONENT_GAMEOBJECT,
    COMPONENT_TRANSFORM,
    COMPONENT_VELOCITY,
    COMPONENT_BOUNDS,
    COMPONENT_COLOR,

    COMPONENT_COUNT
};

typedef struct NativeEntity NativeEntity;

typedef uint32_t ecs_id_t;

struct NativeEntity {
    ecs_id_t id;
};

NEKO_EXPORT NativeEntity entity_nil;

NativeEntity entity_create();
void entity_destroy(NativeEntity ent);
void entity_destroy_all();
bool entity_destroyed(NativeEntity ent);
void entity_set_save_filter(NativeEntity ent, bool filter);
bool entity_get_save_filter(NativeEntity ent);
void entity_clear_save_filters();

struct App;

NEKO_API() void entity_init();
NEKO_API() void entity_fini();
NEKO_API() int entity_update_all(App* app, event_t evt);

NEKO_API() void entity_save_all(App* app);
NEKO_API() void entity_load_all(App* app);
NEKO_API() void entity_load_all_begin();
NEKO_API() void entity_load_all_end();

inline bool native_entity_eq(NativeEntity e, NativeEntity f) { return e.id == f.id; }

class NativeEntityMap {
public:
    int* arr;
    ecs_id_t bound;     // 数组中的第一个空闲位置的索引 = 1 + 最大键
    ecs_id_t capacity;  // 拥有堆空间的元素数量
    int def;            // 未设置键的值
};

NEKO_API() NativeEntityMap* entitymap_new(int def);  // def 为默认值
NEKO_API() void entitymap_clear(NativeEntityMap* emap);
NEKO_API() void entitymap_free(NativeEntityMap* emap);
NEKO_API() void entitymap_set(NativeEntityMap* emap, NativeEntity ent, int val);
NEKO_API() int entitymap_get(NativeEntityMap* emap, NativeEntity ent);

// 在内存中连续 可能会被重新定位/打乱 所以要小心
template <typename T>
struct NativeEntityPool {
    NativeEntityMap* emap;  // 只是数组索引的映射 如果不存在则为 -1
    Array<T> array;

    T* Add(NativeEntity ent) {
        T* elem = nullptr;

        if ((elem = this->Get(ent)) != nullptr) return elem;

        // 将元素添加到pool->array并在pool->emap中设置id
        u64 i = this->array.push(T{});
        elem = dynamic_cast<T*>(&this->array[i]);
        elem->pool_elem.ent = ent;
        entitymap_set(this->emap, ent, this->array.len - 1);
        return elem;
    }

    void Remove(NativeEntity ent) {
        int i = entitymap_get(this->emap, ent);
        if (i >= 0) {
            // 删除可能会与最后一个元素交换 在这里修复映射
            this->array.quick_remove(i);
            {
                GameEntityBase* elem = dynamic_cast<GameEntityBase*>(&this->array[i]);
                entitymap_set(this->emap, elem->pool_elem.ent, i);
            }
            // 删除映射
            entitymap_set(this->emap, ent, -1);
        }
    }

    // 如果未映射则为 NULL
    T* Get(NativeEntity ent) {
        int i = entitymap_get(this->emap, ent);
        if (i >= 0) return &this->array[i];
        return NULL;
    }

    template <class F>
    auto ForEach(F func) {
        T* var;
        for (T* __end = (var = entitypool_begin(this), entitypool_end(this)); var != __end; ++var) {
            func(var);
        }
    }
};

// 该结构必须位于池元素的顶部
// struct Data
// {
//  EntityPoolElem pool_elem;
//  ...
// }
// 其中的值是EntityPool管理的元数据

class GameEntityBase {
public:
    struct EntityPoolElem {
        NativeEntity ent;
    } pool_elem;
};

#define DECL_ENT(T, ...)                  \
    class T : public GameEntityBase {     \
    public:                               \
        static NativeEntityPool<T>* pool; \
        __VA_ARGS__                       \
    };

// object_size 是每个元素的大小
template <typename T>
NativeEntityPool<T>* entitypool_new_(size_t object_size) {
    NativeEntityPool<T>* pool = (NativeEntityPool<T>*)mem_alloc(sizeof(NativeEntityPool<T>));
    memset(&pool->array, 0, sizeof(Array<T>));

    pool->emap = entitymap_new(-1);
    pool->array.reserve(2);

    return pool;
}

template <typename T>
auto entitypool_new() -> NativeEntityPool<T>* {
    return entitypool_new_<T>(sizeof(T));
}

template <typename T>
void entitypool_free(NativeEntityPool<T>* pool) {
    pool->array.trash();
    entitymap_free(pool->emap);
    mem_free(pool);
}

// 由于元素是连续的 因此可以使用指针进行迭代:
//     for (ptr = entitypool_begin(pool), end = entitypool_end(pool);
//             ptr != end; ++ptr)
//         ... use ptr ...
// 如果在迭代期间添加/删除指针将无效
template <typename T>
T* entitypool_begin(NativeEntityPool<T>* pool) {
    return pool->array.begin();
}
template <typename T>
T* entitypool_end(NativeEntityPool<T>* pool) {
    return pool->array.end();
}  // one-past-end

// 0 索引
template <typename T>
T* entitypool_nth(NativeEntityPool<T>* pool, ecs_id_t n) {
    return &pool->array[n];
}

template <typename T>
ecs_id_t entitypool_size(NativeEntityPool<T>* pool) {
    return pool->array.len;
}

template <typename T>
void entitypool_clear(NativeEntityPool<T>* pool) {
    entitymap_clear(pool->emap);
    // array_clear(pool->array);
    pool->array.len = 0;
}

template <typename T>
void entitypool_sort(NativeEntityPool<T>* pool, int (*compar)(const void*, const void*)) {
    ecs_id_t i, n;
    GameEntityBase* elem;

    pool->array.sort(compar);  // 对内部数组排序

    // 重新映射 NativeEntity -> index
    n = pool->array.len;
    for (i = 0; i < n; ++i) {
        elem = dynamic_cast<GameEntityBase*>(&pool->array.data[i]);
        entitymap_set(pool->emap, elem->pool_elem.ent, i);
    }
}

// elem must be /pointer to/ pointer to element
template <typename T>
void entitypool_elem_save(NativeEntityPool<T>* pool, void* elem, App* app) {
    // EntityPoolElem** p;

    // // save NativeEntity id
    // p = (EntityPoolElem**)elem;
    // entity_save(&(*p)->ent, "pool_elem", s);
}
template <typename T>
void entitypool_elem_load(NativeEntityPool<T>* pool, void* elem, App* app) {
    // NativeEntity ent;
    // EntityPoolElem** p;

    // // load NativeEntity id, add element with that key
    // error_assert(entity_load(&ent, "pool_elem", entity_nil, s), "saved EntityPoolElem entry must exist");
    // p = (EntityPoolElem**)elem;
    // *p = (EntityPoolElem*)entitypool_add(pool, ent);
    // (*p)->ent = ent;
}

template <typename T, class F>
auto entitypool_remove_destroyed(NativeEntityPool<T>* pool, F func) {
    do {
        ecs_id_t i;
        GameEntityBase* e;
        for (i = 0; i < entitypool_size(pool);) {
            e = dynamic_cast<GameEntityBase*>(entitypool_nth(pool, i));
            if (entity_destroyed(e->pool_elem.ent))
                func(e->pool_elem.ent);
            else
                ++i;
        }
    } while (0);
}

#define entitypool_foreach(var, pool) for (void* __end = (var = (decltype(var))entitypool_begin(pool), entitypool_end(pool)); var != __end; ++var)

// #define entitypool_save_foreach(var, var_s, pool, n, s)                                 \
//     for (Store* pool##_s__ = NULL; !pool##_s__ && store_child_save(&pool##_s__, n, s);) \
//     entitypool_foreach(var, pool) if (entity_get_save_filter(((EntityPoolElem*)var)->ent)) if (store_child_save(&var_s, NULL, pool##_s__)) if ((entitypool_elem_save(pool, &var, var_s)), 1)
// #define entitypool_load_foreach(var, var_s, pool, n, s)                                 \
//     for (Store* pool##_s__ = NULL; !pool##_s__ && store_child_load(&pool##_s__, n, s);) \
//         while (store_child_load(&var_s, NULL, pool##_s__))                              \
//             if ((entitypool_elem_load(pool, &var, var_s)), 1)

#define entitypool_save_foreach(var, var_s, pool, n, s) if (0)
#define entitypool_load_foreach(var, var_s, pool, n, s) if (0)

void system_init();
void system_fini();

void prefab_save_all(App* app);
void prefab_load_all(App* app);

#endif
