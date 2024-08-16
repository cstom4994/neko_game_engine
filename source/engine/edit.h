#ifndef EDIT_H
#define EDIT_H

#include <stdbool.h>

#include "engine/base.h"
#include "engine/ecs.h"
#include "engine/prelude.h"

NEKO_SCRIPT(
        edit,

        NEKO_EXPORT void edit_set_enabled(bool e);

        NEKO_EXPORT bool edit_get_enabled();

        // 无法选择不可编辑的实体
        NEKO_EXPORT void edit_set_editable(Entity ent, bool editable);

        NEKO_EXPORT bool edit_get_editable(Entity ent);

        // 每个维度上都是非负的 零意味着没有网格
        NEKO_EXPORT void edit_set_grid_size(LuaVec2 size);

        NEKO_EXPORT LuaVec2 edit_get_grid_size();

        // 用于点击选择等
        NEKO_EXPORT void edit_bboxes_update(Entity ent, BBox bbox);  // 合并bbox

        NEKO_EXPORT bool edit_bboxes_has(Entity ent);

        NEKO_EXPORT BBox edit_bboxes_get(Entity ent);

        NEKO_EXPORT unsigned int edit_bboxes_get_num();

        struct EntityBBoxPair {
            Entity ent;
            BBox bbox;
        };

        NEKO_EXPORT Entity edit_bboxes_get_nth_ent(unsigned int n); NEKO_EXPORT BBox edit_bboxes_get_nth_bbox(unsigned int n);

        NEKO_EXPORT void edit_bboxes_set_selected(Entity ent, bool selected);

        // 在两个世界空间坐标之间画一条线
        NEKO_EXPORT void edit_line_add(LuaVec2 a, LuaVec2 b, Scalar point_size, Color color);

)

void edit_clear();

void edit_init();
void edit_fini();
void edit_update_all();
void edit_draw_all();
void edit_save_all(Store *s);
void edit_load_all(Store *s);

#endif
