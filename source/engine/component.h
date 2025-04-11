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
    CEntityPool<T> *EntityPool;

public:
    inline int GetTid() const { return Tid; };
    virtual int Inspect(CEntity ent) = 0;
};

#endif
