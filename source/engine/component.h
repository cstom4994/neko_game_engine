#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"

template <typename T>
class ComponentTypeBase {
protected:
    int Tid;
    CEntityPool<T>* EntityPool;

public:
    inline void ComponentReg() {
        auto L = ENGINE_LUA();
        ComponentTypeBase::Tid = EcsRegisterCType<T>(L);
        ComponentTypeBase::EntityPool = EcsProtoGetCType<T>(L);
    }

    inline int GetTid() const { return Tid; };
    virtual int Inspect(CEntity ent) = 0;

    virtual T* ComponentAdd(CEntity ent) = 0;
    virtual void ComponentRemove(CEntity ent) = 0;

    inline T* ComponentGetPtr(CEntity ent) { return ComponentTypeBase::EntityPool->GetPtr(ent); }

    inline bool ComponentHas(CEntity ent) { return ComponentGetPtr(ent) != nullptr; }

    inline T* WrapAdd(CEntity ent) {
        T* ptr = ComponentAdd(ent);

        auto L = ENGINE_LUA();

        EcsWorld* world = ENGINE_ECS();
        EntityData* e = EcsGetEnt(L, world, ent.id);
        LuaRef tb = LuaRef::NewTable(L);
        tb["__ud"] = ptr;
        int cid1 = EcsComponentSet(L, e, ComponentTypeBase::Tid, tb);

        return ptr;
    }
};

#endif
