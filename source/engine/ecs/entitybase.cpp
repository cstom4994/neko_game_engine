#include "entitybase.hpp"

#include "engine/component.h"
#include "engine/ecs/entity.h"

NativeEntityPool<Tiled> *Tiled::pool;
NativeEntityPool<Camera> *Camera::pool;
NativeEntityPool<uneditable> *uneditable::pool;
NativeEntityPool<BBoxPoolElem> *BBoxPoolElem::pool;
NativeEntityPool<Gui> *Gui::pool;
NativeEntityPool<GuiRect> *GuiRect::pool;
NativeEntityPool<Text> *Text::pool;
NativeEntityPool<TextEdit> *TextEdit::pool;
NativeEntityPool<Sprite> *Sprite::pool;
NativeEntityPool<Transform> *Transform::pool;
