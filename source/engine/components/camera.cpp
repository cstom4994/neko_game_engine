

#include "camera.h"

#include "base/common/profiler.hpp"
#include "base/common/singleton.hpp"
#include "engine/bootstrap.h"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"
#include "engine/components/transform.h"

// -------------------------------------------------------------------------

CCamera *Camera::ComponentAdd(CEntity ent) {
    CCamera *camera{};

    if (ComponentGetPtr(ent)) return nullptr;

    the<Transform>().ComponentAdd(ent);

    camera = ComponentTypeBase::EntityPool->Add(ent);
    camera->viewport_height = 1.0;

    if (CEntityEq(curr_camera, entity_nil)) {
        curr_camera = ent;
    }

    return camera;
}

void Camera::ComponentRemove(CEntity ent) {
    ComponentTypeBase::EntityPool->Remove(ent);

    if (CEntityEq(curr_camera, ent)) {
        curr_camera = entity_nil;
    }
}

void Camera::camera_set_edit_camera(CEntity ent) { edit_camera = ent; }

void Camera::camera_set_current(CEntity ent, bool current) {
    if (current) {
        curr_camera = ent;
    } else if (CEntityEq(curr_camera, ent)) {
        curr_camera = entity_nil;
    }
}

bool Camera::camera_get_current(CEntity ent) { return CEntityEq(curr_camera, ent); }

void Camera::camera_set_current_camera(CEntity ent) { curr_camera = ent; }

CEntity Camera::camera_get_current_camera() {
    if (edit_get_enabled()) {
        return edit_camera;
    } else {
        return curr_camera;
    }
}

void Camera::camera_set_viewport_height(CEntity ent, f32 height) {
    CCamera *camera = (CCamera *)ComponentGetPtr(ent);
    error_assert(camera);
    camera->viewport_height = height;
}

f32 Camera::camera_get_viewport_height(CEntity ent) {
    CCamera *camera = (CCamera *)ComponentGetPtr(ent);
    error_assert(camera);
    return camera->viewport_height;
}

DEFINE_IMGUI_BEGIN(template <>, CCamera) { ImGuiWrap::Auto(var.viewport_height, "viewport_height"); }
DEFINE_IMGUI_END()

int Camera::Inspect(CEntity ent) {

    CCamera *camera = ComponentGetPtr(ent);
    error_assert(camera);

    ImGuiWrap::Auto(camera, "CCamera");

    return 0;
}

vec2 Camera::camera_world_to_pixels(vec2 p) { return Neko::the<CL>().unit_to_pixels(camera_world_to_unit(p)); }

vec2 Camera::camera_world_to_unit(vec2 p) {
    // use cached inverse view matrix
    return mat3_transform(inverse_view_matrix, p);
}

vec2 Camera::camera_pixels_to_world(vec2 p) { return camera_unit_to_world(Neko::the<CL>().pixels_to_unit(p)); }

vec2 Camera::camera_unit_to_world(vec2 p) {
    CEntity cam = camera_get_current_camera();
    if (!CEntityEq(cam, entity_nil)) {
        return the<Transform>().transform_local_to_world(cam, p);
    } else {
        return p;
    }
}

// -------------------------------------------------------------------------

void Camera::camera_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    ComponentTypeBase::ComponentReg();

    curr_camera = entity_nil;
    edit_camera = entity_nil;
    inverse_view_matrix = mat3_identity();

    // clang-format off

    auto type = BUILD_TYPE(Camera)
        .MemberMethod("camera_world_to_pixels", this, &Camera::camera_world_to_pixels)
        .MemberMethod("camera_world_to_unit", this, &Camera::camera_world_to_unit)
        .MemberMethod("camera_pixels_to_world", this, &Camera::camera_pixels_to_world)
        .MemberMethod("camera_unit_to_world", this, &Camera::camera_unit_to_world)
        .MemberMethod<ComponentTypeBase>("camera_add", this, &ComponentTypeBase::WrapAdd)
        .MemberMethod<ComponentTypeBase>("camera_has", this, &ComponentTypeBase::ComponentHas)
        .MemberMethod("camera_remove", this, &Camera::ComponentRemove)
        .MemberMethod("camera_set_edit_camera", this, &Camera::camera_set_edit_camera)
        .MemberMethod("camera_set_current", this, &Camera::camera_set_current)
        .MemberMethod("camera_get_current", this, &Camera::camera_get_current)
        .MemberMethod("camera_set_current_camera", this, &Camera::camera_set_current_camera)
        .MemberMethod("camera_get_current_camera", this, &Camera::camera_get_current_camera)
        .MemberMethod("camera_set_viewport_height", this, &Camera::camera_set_viewport_height)
        .MemberMethod("camera_get_viewport_height", this, &Camera::camera_get_viewport_height)
        .MemberMethod("camera_get_inverse_view_matrix", this, &Camera::GetInverseViewMatrix)
        .Build();

    // clang-format on
}

void Camera::camera_fini() { entitypool_free(ComponentTypeBase::EntityPool); }

int Camera::camera_update_all(Event evt) {
    vec2 win_size;
    f32 aspect;
    CCamera *camera;
    CEntity cam;
    vec2 scale;
    static BBox bbox = {{-1, -1}, {1, 1}};

    entitypool_remove_destroyed(ComponentTypeBase::EntityPool, [this](CEntity ent) { ComponentRemove(ent); });

    win_size = Neko::the<CL>().get_window_size();
    aspect = win_size.x / win_size.y;

    entitypool_foreach(camera, ComponentTypeBase::EntityPool) {
        scale = luavec2(0.5 * aspect * camera->viewport_height, 0.5 * camera->viewport_height);
        the<Transform>().transform_set_scale(camera->ent, scale);

        edit_bboxes_update(camera->ent, bbox);
    }

    cam = camera_get_current_camera();
    if (CEntityEq(cam, entity_nil)) {
        inverse_view_matrix = mat3_identity();
    } else {
        inverse_view_matrix = mat3_inverse(the<Transform>().transform_get_world_matrix(cam));
    }

    return 0;
}

void camera_save_all(CL *app) {
    // Store *t, *camera_s;
    // CCamera *camera;

    // if (store_child_save(&t, "camera", s)) {
    //     if (entity_get_save_filter(curr_camera)) entity_save(&curr_camera, "curr_camera", t);

    //     mat3_save(&inverse_view_matrix, "inverse_view_matrix", t);

    //     entitypool_save_foreach(camera, camera_s, ComponentTypeBase::EntityPool, "pool", t) float_save(&camera->viewport_height, "viewport_height", camera_s);
    // }
}
void camera_load_all(CL *app) {
    // Store *t, *camera_s;
    // CCamera *camera;

    // if (store_child_load(&t, "camera", s)) {
    //     entity_load(&curr_camera, "curr_camera", curr_camera, t);

    //     mat3_load(&inverse_view_matrix, "inverse_view_matrix", mat3_identity(), t);

    //     entitypool_load_foreach(camera, camera_s, ComponentTypeBase::EntityPool, "pool", t) float_load(&camera->viewport_height, "viewport_height", 1, camera_s);
    // }
}
