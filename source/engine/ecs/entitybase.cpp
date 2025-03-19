#include "entitybase.hpp"

#include "engine/component.h"
#include "engine/ecs/entity.h"

CEntityPool<Tiled> *Tiled::pool;
CEntityPool<Camera> *Camera::pool;
CEntityPool<uneditable> *uneditable::pool;
CEntityPool<BBoxPoolElem> *BBoxPoolElem::pool;
CEntityPool<Sprite> *Sprite::pool;
CEntityPool<Transform> *Transform::pool;
