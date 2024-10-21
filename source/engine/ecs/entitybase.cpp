#include "entitybase.hpp"

#include "engine/component.h"
#include "engine/ecs/entity.h"

template <>
NativeEntityPool<Tiled> *EntityBase<Tiled>::pool;

template <>
NativeEntityPool<Camera> *EntityBase<Camera>::pool;

template <>
NativeEntityPool<uneditable> *EntityBase<uneditable>::pool;

template <>
NativeEntityPool<BBoxPoolElem> *EntityBase<BBoxPoolElem>::pool;

template <>
NativeEntityPool<Gui> *EntityBase<Gui>::pool;

template <>
NativeEntityPool<GuiRect> *EntityBase<GuiRect>::pool;

template <>
NativeEntityPool<Text> *EntityBase<Text>::pool;

template <>
NativeEntityPool<TextEdit> *EntityBase<TextEdit>::pool;

template <>
NativeEntityPool<Sprite> *EntityBase<Sprite>::pool;

template <>
NativeEntityPool<Transform> *EntityBase<Transform>::pool;
