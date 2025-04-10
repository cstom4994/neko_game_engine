#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"

struct CTransform;

template <typename T>
class ComponentTypeBase {
protected:
    int Tid;
    CEntityPool<T> *EntityPool;

public:
    inline int GetTid() const { return Tid; };
    virtual int Inspect(CEntity ent) = 0;
};

#include "engine/components/transform.h"
#include "engine/components/tiledmap.hpp"

// 如果当前没有摄像机 逆视图矩阵为单位矩阵
// 这意味着视图是世界中心的一个 2x2 单位框
void camera_add(CEntity ent);
void camera_remove(CEntity ent);
bool camera_has(CEntity ent);
void camera_set_edit_camera(CEntity ent);
void camera_set_current(CEntity ent, bool current);  // 设置/获取当前激活的摄像机 如果没有则返回 entity_nil
bool camera_get_current(CEntity ent);
void camera_set_current_camera(CEntity ent);
CEntity camera_get_current_camera();
void camera_set_viewport_height(CEntity ent, f32 height);  // 屏幕上垂直方向适配的世界单位数量
f32 camera_get_viewport_height(CEntity ent);
mat3 camera_get_inverse_view_matrix();

// screen-space coordinates <-> world coordinates transformations
vec2 camera_world_to_pixels(vec2 p);
vec2 camera_world_to_unit(vec2 p);
vec2 camera_pixels_to_world(vec2 p);
vec2 camera_unit_to_world(vec2 p);
const mat3 *camera_get_inverse_view_matrix_ptr();  // for quick GLSL binding

class Camera : public SingletonClass<Camera> {
public:
    void camera_init();
    void camera_fini();
    int camera_update_all(Event evt);
};

void sprite_set_atlas(const char *filename);
const char *sprite_get_atlas();
void sprite_add(CEntity ent);
void sprite_remove(CEntity ent);
bool sprite_has(CEntity ent);
void sprite_set_size(CEntity ent, vec2 size);  // 以世界单位绘制的大小 中心位于变换位置。
vec2 sprite_get_size(CEntity ent);
void sprite_set_texcell(CEntity ent, vec2 texcell);  // 图集区域的左下角坐标 以像素为单位
vec2 sprite_get_texcell(CEntity ent);
void sprite_set_texsize(CEntity ent, vec2 texsize);  // 图集区域的大小 以像素为单位
vec2 sprite_get_texsize(CEntity ent);
void sprite_set_depth(CEntity ent, int depth);  // 较低深度的内容绘制在顶部
int sprite_get_depth(CEntity ent);

class Sprite : public SingletonClass<Sprite> {
public:
    void sprite_init();
    void sprite_fini();
    int sprite_update_all(Event evt);
    void sprite_draw_all();
};

int open_db(lua_State *L);

void edit_init_impl(lua_State *L);
void edit_fini_impl();
void edit_update_impl();
void edit_draw_impl();

#endif
