#pragma once

#include "base/common/color.hpp"
#include "engine/component.h"
#include "engine/ecs/entity.h"

// DECL_ENT(Camera, Scalar viewport_height;);

class Camera : public GameEntityBase {
public:
    static NativeEntityPool<Camera>* pool;

public:
    Scalar viewport_height;
};

DECL_ENT(uneditable, int what;);

// 所有 GUI 系统共有的一般功能/数据

DECL_ENT(Gui, bool setvisible;  // 外部设置可见性
         bool visible;          // 内部递归计算可见性
         bool updated_visible;  // 用于递归可见性计算
         bool focusable;        // can be focused
         bool captures_events;

         Color color;

         BBox bbox;  // 在实体空间中
         GuiAlign halign; GuiAlign valign; vec2 padding;);

DECL_ENT(Sprite, mat3 wmat;  // 要发送到着色器的世界变换矩阵

         vec2 size; vec2 texcell; vec2 texsize;

         int depth;);

DECL_ENT(Transform, vec2 position; Scalar rotation; vec2 scale; NativeEntity parent;  // 如果entity_nil 则为 root
         Array<NativeEntity> children;                                                // 如果为 NULL 则为空
         mat3 mat_cache;                                                              // 更新此内容
         mat3 worldmat_cache;                                                         // 在父子更新时缓存
         ecs_id_t dirty_count;);

// bbox pool
DECL_ENT(BBoxPoolElem, mat3 wmat; BBox bbox; Scalar selected;  // > 0.5 if and only if selected
);

DECL_ENT(GuiRect,
         mat3 wmat;

         vec2 size; bool visible; Color color;

         bool hfit; bool vfit; bool hfill; bool vfill;

         bool updated; int depth;  // for draw order -- child depth > parent depth
);

// info to send to shader program for each character
typedef struct TextChar TextChar;
struct TextChar {
    vec2 pos;         // position in space of text entity in size-less units
    vec2 cell;        // cell in font image
    float is_cursor;  // > 0 iff. this char is cursor
};

// info per text entity
DECL_ENT(Text,

         char* str;
         Array<TextChar> chars;  // per-character info buffered to shader
         vec2 bounds;            // max x, min y in size-less units

         int cursor;);

DECL_ENT(TextEdit, unsigned int cursor;  // 0 at beginning of string
         bool numerical;);