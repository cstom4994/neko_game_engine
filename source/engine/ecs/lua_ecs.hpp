
#pragma once

#include "base/scripting/lua_wrapper.hpp"

namespace Neko {
namespace ecs {

using namespace luabind;

#define __neko_ecs_ent_id(index, ver) (((u64)ver << 32) | index)
#define __neko_ecs_ent_index(id) ((u32)id)
#define __neko_ecs_ent_ver(id) ((u32)(id >> 32))

enum ECS_WORLD_UPVALUES {
    WORLD_PROTO_ID = 1,  // 表映射name到tid
    WORLD_PROTO_DEFINE,  // 表存储name到组件注册原型
    WORLD_COMPONENTS,    // 表存储组件的实际内容 WORLD_COMPONENTS[tid][cid] = {...}
    WORLD_SYSTEMS,       // 表存储系统
    WORLD_MATCH_CTX,     // ud存储当前matchctx
    WORLD_KEY_EID,       // eid元表键名字面量
    WORLD_KEY_TID,       // tid元表键名字面量
    WORLD_KEY_UD,        // ud元表键名字面量
    WORLD_UPVAL_N
};

#define LINK_NIL (-1)
#define LINK_NONE (-2)

#define TYPE_MIN_ID 1    //
#define TYPE_MAX_ID 255  //
#define TYPE_COUNT 256   // 最大组件数量

#define ENTITY_MAX_COMPONENTS (64)  // 单个实体最大组件数量

#define NEKO_ECS_CORE "__NEKO_ECS_CORE"
#define ECS_WORLD_METATABLE "__NEKO_ECS_WORLD_METATABLE"
#define ECS_WORLD (1)

struct Component {
    int eid;  // 该组件附着的实体id
    int dirty_next;
    int dead_next;
};

struct ComponentPool {
    int cap;
    int free_idx;

    int dirty_head;
    int dirty_tail;

    int dead_head;
    int dead_tail;

    Component* buf;
};

// 每个Component类型都有一个数字id称为tid
// 每个Component实例都有一个数字id称为cid
// 根据tid和cid来找到某一个具体的Component实例
// struct ComponentPtr {
//     int tid;
//     int cid;
// };

struct Entity {
    int next;
    i8 components_count;                   // 当前实体拥有的组件数量
    unsigned char components[TYPE_COUNT];  // 第几个组件在index中的位置
    int components_index[ENTITY_MAX_COMPONENTS];
};

struct EcsWorld {
    int entity_cap;  // 容量

    // TODO:将ENTITY_FREE设置为FIFO 这可以在以后回收EID以避免某些错误
    int entity_free_id;  // 下一个闲置实体的id
    int entity_dead_id;
    int type_idx;  // 当前最大组件索引
    Entity* entity_buf;
    ComponentPool component_pool[TYPE_COUNT];  // 存储所有组件的标记数据
};

int l_ecs_create_world(lua_State* L);

// EcsId ecs_component_w(ecs_t* registry, const_str component_name, size_t component_size, ecs_constructor_fn constructor, ecs_destructor_fn destructor);

// #define ECS_COMPONENT(type, ctor, dtor) ecs_component_w(ENGINE_ECS(), #type, sizeof(type), ctor, dtor)
// #define ECS_COMPONENT_DEFINE(type, ctor, dtor) ECS_COMPONENT_ID(type) = ECS_COMPONENT(type, ctor, dtor)

// #define ECS_COMPONENT_ID(type) __NEKO_GEN_COMPONENT_##type
// #define ECS_COMPONENT_DECL(type) EcsId ECS_COMPONENT_ID(type)
// #define ECS_COMPONENT_EXTERN(type) extern EcsId ECS_COMPONENT_ID(type)

// ecs_t* ecs_init(lua_State* L);

int EcsRegister(lua_State* L, const_str name);
int EcsGetTid(lua_State* L, const char* name);

Entity* EcsEntityAlloc(EcsWorld* world);
int EcsComponentAlloc(EcsWorld* world, Entity* e, int tid);
Entity* EcsEntityNew(lua_State* L, const LuaRef& ref, lua_CFunction gc);
void EcsEntityDel(lua_State* L, int eid);
int EcsComponentSet(lua_State* L, Entity* e, int tid, const LuaRef& ref);
int EcsComponentSet(lua_State* L, Entity* e, const char* name, const LuaRef& ref);
LuaRef EcsComponentGet(lua_State* L, Entity* e, const char* name);
LuaRef EcsComponentGet(lua_State* L, Entity* e, int tid);

Entity* EcsGetEnt(lua_State* L, EcsWorld* w, int eid);

template <typename T>
int EcsRegisterCType(lua_State* L) {

    const char* type_name = reflection::GetTypeName<T>();

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    int ecs_ud = lua_gettop(L);

    lua_getiuservalue(L, ecs_ud, WORLD_PROTO_DEFINE);
    lua_pushstring(L, type_name);                   // 复制组件key
    lua_pushlightuserdata(L, entitypool_new<T>());  // 复制原型
    lua_rawset(L, -3);                              // WORLD_PROTO_DEFINE[key] = {...}
    lua_pop(L, 1);                                  // # pop WORLD_PROTO_DEFINE

    lua_pop(L, 1);  // # pop __NEKO_ECS_CORE

    return EcsRegister(L, type_name);
}

template <typename T>
int EcsGetTidCType(lua_State* L) {
    return EcsGetTid(L, reflection::GetTypeName<T>());
}

}  // namespace ecs
}  // namespace Neko