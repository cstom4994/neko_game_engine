#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"

// can set scale, rotation, position -- 按该顺序应用的转换

NEKO_EXPORT void transform_set_position(CEntity ent, vec2 pos);
NEKO_EXPORT vec2 transform_get_position(CEntity ent);
NEKO_EXPORT void transform_translate(CEntity ent, vec2 trans);
NEKO_EXPORT void transform_set_rotation(CEntity ent, f32 rot);
NEKO_EXPORT f32 transform_get_rotation(CEntity ent);
NEKO_EXPORT void transform_rotate(CEntity ent, f32 rot);
NEKO_EXPORT void transform_set_scale(CEntity ent, vec2 scale);
NEKO_EXPORT vec2 transform_get_scale(CEntity ent);
NEKO_EXPORT vec2 transform_get_world_position(CEntity ent);
NEKO_EXPORT f32 transform_get_world_rotation(CEntity ent);
NEKO_EXPORT vec2 transform_get_world_scale(CEntity ent);
NEKO_EXPORT mat3 transform_get_world_matrix(CEntity ent);  // world-space
NEKO_EXPORT mat3 transform_get_matrix(CEntity ent);        // parent-space
NEKO_EXPORT vec2 transform_local_to_world(CEntity ent, vec2 v);
NEKO_EXPORT vec2 transform_world_to_local(CEntity ent, vec2 v);
NEKO_EXPORT EcsId transform_get_dirty_count(CEntity ent);
NEKO_EXPORT void transform_set_save_filter_rec(CEntity ent, bool filter);

void transform_add(CEntity ent);
void transform_remove(CEntity ent);
bool transform_has(CEntity ent);
void transform_set_parent(CEntity ent, CEntity parent);
CEntity transform_get_parent(CEntity ent);
EcsId transform_get_num_children(CEntity ent);
CEntity* transform_get_children(CEntity ent);
void transform_detach_all(CEntity ent);
void transform_destroy_rec(CEntity ent);  // destroy ent and all children

class Transform : public SingletonClass<Transform> {
public:
    void transform_init();
    void transform_fini();
    int transform_update_all(Event evt);
};

#include "engine/components/tiledmap.hpp"

// 如果当前没有摄像机 逆视图矩阵为单位矩阵
// 这意味着视图是世界中心的一个 2x2 单位框
NEKO_EXPORT void camera_add(CEntity ent);
NEKO_EXPORT void camera_remove(CEntity ent);
NEKO_EXPORT bool camera_has(CEntity ent);
NEKO_EXPORT void camera_set_edit_camera(CEntity ent);
NEKO_EXPORT void camera_set_current(CEntity ent, bool current);  // 设置/获取当前激活的摄像机 如果没有则返回 entity_nil
NEKO_EXPORT bool camera_get_current(CEntity ent);
NEKO_EXPORT void camera_set_current_camera(CEntity ent);
NEKO_EXPORT CEntity camera_get_current_camera();
NEKO_EXPORT void camera_set_viewport_height(CEntity ent, f32 height);  // 屏幕上垂直方向适配的世界单位数量
NEKO_EXPORT f32 camera_get_viewport_height(CEntity ent);
NEKO_EXPORT mat3 camera_get_inverse_view_matrix();

// screen-space coordinates <-> world coordinates transformations
vec2 camera_world_to_pixels(vec2 p);
vec2 camera_world_to_unit(vec2 p);
vec2 camera_pixels_to_world(vec2 p);
vec2 camera_unit_to_world(vec2 p);
const mat3* camera_get_inverse_view_matrix_ptr();  // for quick GLSL binding

class Camera : public SingletonClass<Camera> {
public:
    void camera_init();
    void camera_fini();
    int camera_update_all(Event evt);
};

NEKO_EXPORT void sprite_set_atlas(const char* filename);
NEKO_EXPORT const char* sprite_get_atlas();
NEKO_EXPORT void sprite_add(CEntity ent);
NEKO_EXPORT void sprite_remove(CEntity ent);
NEKO_EXPORT bool sprite_has(CEntity ent);
NEKO_EXPORT void sprite_set_size(CEntity ent, vec2 size);  // 以世界单位绘制的大小 中心位于变换位置。
NEKO_EXPORT vec2 sprite_get_size(CEntity ent);
NEKO_EXPORT void sprite_set_texcell(CEntity ent, vec2 texcell);  // 图集区域的左下角坐标 以像素为单位
NEKO_EXPORT vec2 sprite_get_texcell(CEntity ent);
NEKO_EXPORT void sprite_set_texsize(CEntity ent, vec2 texsize);  // 图集区域的大小 以像素为单位
NEKO_EXPORT vec2 sprite_get_texsize(CEntity ent);
NEKO_EXPORT void sprite_set_depth(CEntity ent, int depth);  // 较低深度的内容绘制在顶部
NEKO_EXPORT int sprite_get_depth(CEntity ent);

class Sprite : public SingletonClass<Sprite> {
public:
    void sprite_init();
    void sprite_fini();
    int sprite_update_all(Event evt);
    void sprite_draw_all();
};

void edit_set_editable(CEntity ent, bool editable);
bool edit_get_editable(CEntity ent);
void edit_set_grid_size(vec2 size);
vec2 edit_get_grid_size();
bool edit_bboxes_has(CEntity ent);
int edit_bboxes_get_num();
CEntity edit_bboxes_get_nth_ent(int n);
void edit_bboxes_set_selected(CEntity ent, bool selected);
void edit_line_add(vec2 a, vec2 b, f32 point_size, Color color);
void edit_set_enabled(bool e);
bool edit_get_enabled();

// 用于点击选择等
void edit_bboxes_update(CEntity ent, BBox bbox);  // 合并bbox
BBox edit_bboxes_get_nth_bbox(int n);
void edit_line_add_xy(vec2 p, f32 point_size, Color color);
int edit_clear(Event evt);

class Edit : public SingletonClass<Edit> {
public:
    void edit_init();
    void edit_fini();
    int edit_update_all(Event evt);
    void edit_draw_all();
};

int open_db(lua_State* L);

#endif
