

#include "base/common/profiler.hpp"
#include "base/common/singleton.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entitybase.hpp"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"

static CEntity curr_camera;
static CEntity edit_camera;

static mat3 inverse_view_matrix;  // 缓存逆视图矩阵

CEntityPool<CCamera> *CCamera__pool;

int type_camera;

// -------------------------------------------------------------------------

void camera_add(CEntity ent) {
    CCamera *camera;

    if (CCamera__pool->Get(ent)) return;

    transform_add(ent);

    camera = CCamera__pool->Add(ent);
    camera->viewport_height = 1.0;

    if (CEntityEq(curr_camera, entity_nil)) curr_camera = ent;
}
void camera_remove(CEntity ent) {
    CCamera__pool->Remove(ent);

    if (CEntityEq(curr_camera, ent)) curr_camera = entity_nil;
}
bool camera_has(CEntity ent) { return CCamera__pool->Get(ent) != NULL; }

void camera_set_edit_camera(CEntity ent) { edit_camera = ent; }

void camera_set_current(CEntity ent, bool current) {
    if (current)
        curr_camera = ent;
    else if (CEntityEq(curr_camera, ent))
        curr_camera = entity_nil;
}
bool camera_get_current(CEntity ent) { return CEntityEq(curr_camera, ent); }
void camera_set_current_camera(CEntity ent) { curr_camera = ent; }
CEntity camera_get_current_camera() {
    if (edit_get_enabled()) return edit_camera;
    return curr_camera;
}

void camera_set_viewport_height(CEntity ent, f32 height) {
    CCamera *camera = (CCamera *)CCamera__pool->Get(ent);
    error_assert(camera);
    camera->viewport_height = height;
}
f32 camera_get_viewport_height(CEntity ent) {
    CCamera *camera = (CCamera *)CCamera__pool->Get(ent);
    error_assert(camera);
    return camera->viewport_height;
}

mat3 camera_get_inverse_view_matrix() { return inverse_view_matrix; }

const mat3 *camera_get_inverse_view_matrix_ptr() { return &inverse_view_matrix; }

vec2 camera_world_to_pixels(vec2 p) { return Neko::the<CL>().unit_to_pixels(camera_world_to_unit(p)); }
vec2 camera_world_to_unit(vec2 p) {
    // use cached inverse view matrix
    return mat3_transform(inverse_view_matrix, p);
}
vec2 camera_pixels_to_world(vec2 p) { return camera_unit_to_world(Neko::the<CL>().pixels_to_unit(p)); }
vec2 camera_unit_to_world(vec2 p) {
    CEntity cam = camera_get_current_camera();
    if (!CEntityEq(cam, entity_nil)) return transform_local_to_world(cam, p);
    return p;
}

// -------------------------------------------------------------------------

void Camera::camera_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    type_camera = EcsRegisterCType<CCamera>(L);

    CCamera__pool = EcsProtoGetCType<CCamera>(L);

    curr_camera = entity_nil;
    edit_camera = entity_nil;
    inverse_view_matrix = mat3_identity();

    auto type = BUILD_TYPE(Camera)
                        .Method("camera_world_to_pixels", &camera_world_to_pixels)                  //
                        .Method("camera_world_to_unit", &camera_world_to_unit)                      //
                        .Method("camera_pixels_to_world", &camera_pixels_to_world)                  //
                        .Method("camera_unit_to_world", &camera_unit_to_world)                      //
                        .Method("camera_add", &camera_add)                                          //
                        .Method("camera_remove", &camera_remove)                                    //
                        .Method("camera_has", &camera_has)                                          //
                        .Method("camera_set_edit_camera", &camera_set_edit_camera)                  //
                        .Method("camera_set_current", &camera_set_current)                          //
                        .Method("camera_get_current", &camera_get_current)                          //
                        .Method("camera_set_current_camera", &camera_set_current_camera)            //
                        .Method("camera_get_current_camera", &camera_get_current_camera)            //
                        .Method("camera_set_viewport_height", &camera_set_viewport_height)          //
                        .Method("camera_get_viewport_height", &camera_get_viewport_height)          //
                        .Method("camera_get_inverse_view_matrix", &camera_get_inverse_view_matrix)  //
                        .Build();
}

void Camera::camera_fini() { entitypool_free(CCamera__pool); }

int Camera::camera_update_all(Event evt) {
    vec2 win_size;
    f32 aspect;
    CCamera *camera;
    CEntity cam;
    vec2 scale;
    static BBox bbox = {{-1, -1}, {1, 1}};

    entitypool_remove_destroyed(CCamera__pool, camera_remove);

    win_size = Neko::the<CL>().get_window_size();
    aspect = win_size.x / win_size.y;

    entitypool_foreach(camera, CCamera__pool) {
        scale = luavec2(0.5 * aspect * camera->viewport_height, 0.5 * camera->viewport_height);
        transform_set_scale(camera->ent, scale);

        edit_bboxes_update(camera->ent, bbox);
    }

    cam = camera_get_current_camera();
    if (CEntityEq(cam, entity_nil))
        inverse_view_matrix = mat3_identity();
    else
        inverse_view_matrix = mat3_inverse(transform_get_world_matrix(cam));

    return 0;
}

void camera_save_all(CL *app) {
    // Store *t, *camera_s;
    // CCamera *camera;

    // if (store_child_save(&t, "camera", s)) {
    //     if (entity_get_save_filter(curr_camera)) entity_save(&curr_camera, "curr_camera", t);

    //     mat3_save(&inverse_view_matrix, "inverse_view_matrix", t);

    //     entitypool_save_foreach(camera, camera_s, CCamera__pool, "pool", t) float_save(&camera->viewport_height, "viewport_height", camera_s);
    // }
}
void camera_load_all(CL *app) {
    // Store *t, *camera_s;
    // CCamera *camera;

    // if (store_child_load(&t, "camera", s)) {
    //     entity_load(&curr_camera, "curr_camera", curr_camera, t);

    //     mat3_load(&inverse_view_matrix, "inverse_view_matrix", mat3_identity(), t);

    //     entitypool_load_foreach(camera, camera_s, CCamera__pool, "pool", t) float_load(&camera->viewport_height, "viewport_height", 1, camera_s);
    // }
}
