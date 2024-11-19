#include "entitybase.hpp"

#include "engine/component.h"
#include "engine/ecs/entity.h"

NativeEntityPool<Tiled> *Tiled::pool;
NativeEntityPool<Camera> *Camera::pool;
NativeEntityPool<uneditable> *uneditable::pool;
NativeEntityPool<BBoxPoolElem> *BBoxPoolElem::pool;
NativeEntityPool<Sprite> *Sprite::pool;
NativeEntityPool<Transform> *Transform::pool;
