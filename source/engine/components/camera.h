
#pragma once

#include "engine/ecs/entity.h"
#include "engine/component.h"

struct CCamera : CEntityBase {
    f32 viewport_height;
};

static_assert(std::is_trivially_copyable_v<CCamera>);

class Camera : public SingletonClass<Camera>, public ComponentTypeBase<CCamera> {
private:
    CEntity curr_camera;
    CEntity edit_camera;

    mat3 inverse_view_matrix;  // 缓存逆视图矩阵

public:
    void camera_init();
    void camera_fini();
    int camera_update_all(Event evt);

    // 如果当前没有摄像机 逆视图矩阵为单位矩阵
    // 这意味着视图是世界中心的一个 2x2 单位框
    CCamera *ComponentAdd(CEntity ent) override;
    void ComponentRemove(CEntity ent) override;

    void camera_set_edit_camera(CEntity ent);
    void camera_set_current(CEntity ent, bool current);  // 设置/获取当前激活的摄像机 如果没有则返回 entity_nil
    bool camera_get_current(CEntity ent);
    void camera_set_current_camera(CEntity ent);
    CEntity camera_get_current_camera();
    void camera_set_viewport_height(CEntity ent, f32 height);  // 屏幕上垂直方向适配的世界单位数量
    f32 camera_get_viewport_height(CEntity ent);

    // screen-space coordinates <-> world coordinates transformations
    static vec2 camera_world_to_pixels(vec2 p);
    static vec2 camera_world_to_unit(vec2 p);
    static vec2 camera_pixels_to_world(vec2 p);
    static vec2 camera_unit_to_world(vec2 p);

    inline mat3 GetInverseViewMatrix() { return inverse_view_matrix; }

    inline const mat3 *GetInverseViewMatrixPtr() { return &inverse_view_matrix; }  // for quick GLSL binding

    int Inspect(CEntity ent) override;
};