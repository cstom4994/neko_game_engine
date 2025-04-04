
#ifndef NEKO_ECS_H
#define NEKO_ECS_H

#include "engine/base.hpp"
#include "engine/event.h"
#include "engine/ecs/lua_ecs.hpp"

// 测试 ECS 用
typedef struct CGameObjectTest {
    char name[64];
    bool active;
    bool visible;
    bool selected;
} CGameObjectTest;

typedef struct CEntity CEntity;

typedef uint32_t EcsId;

struct CEntity {
    EcsId id;
};

NEKO_EXPORT CEntity entity_nil;

CEntity entity_create(const String& name);
void entity_destroy(CEntity ent);
void entity_destroy_all();
bool entity_destroyed(CEntity ent);

class Entity : public SingletonClass<Entity> {
public:
    void entity_init();
    void entity_fini();
    int entity_update_all(Event evt);
};

inline bool CEntityEq(CEntity e, CEntity f) { return e.id == f.id; }

class CEntityMap {
public:
    int* arr;
    EcsId bound;     // 数组中的第一个空闲位置的索引 = 1 + 最大键
    EcsId capacity;  // 拥有堆空间的元素数量
    int def;         // 未设置键的值
};

NEKO_API() CEntityMap* entitymap_new(int def);  // def 为默认值
NEKO_API() void entitymap_clear(CEntityMap* emap);
NEKO_API() void entitymap_free(CEntityMap* emap);
NEKO_API() void entitymap_set(CEntityMap* emap, CEntity ent, int val);
NEKO_API() int entitymap_get(CEntityMap* emap, CEntity ent);

// 在内存中连续 可能会被重新定位/打乱 所以要小心
template <typename T>
struct CEntityPool {
    CEntityMap* emap;  // 只是数组索引的映射 如果不存在则为 -1
    Array<T> array;

    T* Add(CEntity ent) {
        T* elem = nullptr;

        if ((elem = this->Get(ent)) != nullptr) return elem;

        // 将元素添加到pool->array并在pool->emap中设置id
        u64 i = this->array.push(T{});
        elem = dynamic_cast<T*>(&this->array[i]);
        elem->ent = ent;
        entitymap_set(this->emap, ent, this->array.len - 1);
        return elem;
    }

    void Remove(CEntity ent) {
        int i = entitymap_get(this->emap, ent);
        if (i >= 0) {
            // 删除可能会与最后一个元素交换 在这里修复映射
            this->array.quick_remove(i);
            {
                CEntityBase* elem = dynamic_cast<CEntityBase*>(&this->array[i]);
                entitymap_set(this->emap, elem->ent, i);
            }
            // 删除映射
            entitymap_set(this->emap, ent, -1);
        }
    }

    // 如果未映射则为 NULL
    T* Get(CEntity ent) {
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

struct CEntityBase {
    CEntity ent;
};

// object_size 是每个元素的大小
template <typename T>
CEntityPool<T>* entitypool_new_(size_t object_size) {
    CEntityPool<T>* pool = (CEntityPool<T>*)mem_alloc(sizeof(CEntityPool<T>));
    memset(&pool->array, 0, sizeof(Array<T>));

    pool->emap = entitymap_new(-1);
    pool->array.reserve(2);

    return pool;
}

template <typename T>
auto entitypool_new() -> CEntityPool<T>* {
    return entitypool_new_<T>(sizeof(T));
}

template <typename T>
void entitypool_free(CEntityPool<T>* pool) {
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
T* entitypool_begin(CEntityPool<T>* pool) {
    return pool->array.begin();
}
template <typename T>
T* entitypool_end(CEntityPool<T>* pool) {
    return pool->array.end();
}  // one-past-end

// 0 索引
template <typename T>
T* entitypool_nth(CEntityPool<T>* pool, EcsId n) {
    return &pool->array[n];
}

template <typename T>
EcsId entitypool_size(CEntityPool<T>* pool) {
    return pool->array.len;
}

template <typename T>
void entitypool_clear(CEntityPool<T>* pool) {
    entitymap_clear(pool->emap);
    // array_clear(pool->array);
    pool->array.len = 0;
}

template <typename T>
void entitypool_sort(CEntityPool<T>* pool, int (*compar)(const void*, const void*)) {
    EcsId i, n;
    CEntityBase* elem;

    pool->array.sort(compar);  // 对内部数组排序

    // 重新映射 CEntity -> index
    n = pool->array.len;
    for (i = 0; i < n; ++i) {
        elem = dynamic_cast<CEntityBase*>(&pool->array.data[i]);
        entitymap_set(pool->emap, elem->ent, i);
    }
}

// elem must be /pointer to/ pointer to element
template <typename T>
void entitypool_elem_save(CEntityPool<T>* pool, void* elem) {
    // EntityPoolElem** p;

    // // save CEntity id
    // p = (EntityPoolElem**)elem;
    // entity_save(&(*p)->ent, "pool_elem", s);
}
template <typename T>
void entitypool_elem_load(CEntityPool<T>* pool, void* elem) {
    // CEntity ent;
    // EntityPoolElem** p;

    // // load CEntity id, add element with that key
    // error_assert(entity_load(&ent, "pool_elem", entity_nil, s), "saved EntityPoolElem entry must exist");
    // p = (EntityPoolElem**)elem;
    // *p = (EntityPoolElem*)entitypool_add(pool, ent);
    // (*p)->ent = ent;
}

template <typename T, class F>
auto entitypool_remove_destroyed(CEntityPool<T>* pool, F func) {
    do {
        EcsId i;
        CEntityBase* e;
        for (i = 0; i < entitypool_size(pool);) {
            e = dynamic_cast<CEntityBase*>(entitypool_nth(pool, i));
            if (entity_destroyed(e->ent))
                func(e->ent);
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

template <typename T>
CEntityPool<T>* EcsProtoGetCType(lua_State* L) {

    const char* type_name = reflection::GetTypeName<T>();

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    lua_getiuservalue(L, ecs_ud, WORLD_PROTO_DEFINE);
    lua_pushstring(L, type_name);
    lua_rawget(L, -2);

    CEntityPool<T>* pool = reinterpret_cast<CEntityPool<T>*>(lua_touserdata(L, -1));

    lua_pop(L, 1);  // pop WORLD_PROTO_DEFINE[type_name]
    lua_pop(L, 1);  // pop WORLD_PROTO_DEFINE
    lua_pop(L, 1);  // pop __NEKO_ECS_CORE

    return pool;
}

#endif
