

#include "base/common/profiler.hpp"
#include "base/common/singleton.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entitybase.hpp"
#include "engine/edit.h"

static CEntity curr_camera;
static CEntity edit_camera;

static mat3 inverse_view_matrix;  // 缓存逆视图矩阵

// -------------------------------------------------------------------------

void camera_add(CEntity ent) {
    Camera *camera;

    if (Camera::pool->Get(ent)) return;

    transform_add(ent);

    camera = Camera::pool->Add(ent);
    camera->viewport_height = 1.0;

    if (CEntityEq(curr_camera, entity_nil)) curr_camera = ent;
}
void camera_remove(CEntity ent) {
    Camera::pool->Remove(ent);

    if (CEntityEq(curr_camera, ent)) curr_camera = entity_nil;
}
bool camera_has(CEntity ent) { return Camera::pool->Get(ent) != NULL; }

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
    Camera *camera = (Camera *)Camera::pool->Get(ent);
    error_assert(camera);
    camera->viewport_height = height;
}
f32 camera_get_viewport_height(CEntity ent) {
    Camera *camera = (Camera *)Camera::pool->Get(ent);
    error_assert(camera);
    return camera->viewport_height;
}

mat3 camera_get_inverse_view_matrix() { return inverse_view_matrix; }

const mat3 *camera_get_inverse_view_matrix_ptr() { return &inverse_view_matrix; }

vec2 camera_world_to_pixels(vec2 p) { return Neko::the<Game>().unit_to_pixels(camera_world_to_unit(p)); }
vec2 camera_world_to_unit(vec2 p) {
    // use cached inverse view matrix
    return mat3_transform(inverse_view_matrix, p);
}
vec2 camera_pixels_to_world(vec2 p) { return camera_unit_to_world(Neko::the<Game>().pixels_to_unit(p)); }
vec2 camera_unit_to_world(vec2 p) {
    CEntity cam = camera_get_current_camera();
    if (!CEntityEq(cam, entity_nil)) return transform_local_to_world(cam, p);
    return p;
}

// -------------------------------------------------------------------------

void camera_init() {
    PROFILE_FUNC();

    Camera::pool = entitypool_new<Camera>();
    curr_camera = entity_nil;
    edit_camera = entity_nil;
    inverse_view_matrix = mat3_identity();
}

void camera_fini() { entitypool_free(Camera::pool); }

int camera_update_all(App *app, event_t evt) {
    vec2 win_size;
    f32 aspect;
    Camera *camera;
    CEntity cam;
    vec2 scale;
    static BBox bbox = {{-1, -1}, {1, 1}};

    entitypool_remove_destroyed(Camera::pool, camera_remove);

    win_size = Neko::the<Game>().get_window_size();
    aspect = win_size.x / win_size.y;

    entitypool_foreach(camera, Camera::pool) {
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

void camera_save_all(App* app) {
    // Store *t, *camera_s;
    // Camera *camera;

    // if (store_child_save(&t, "camera", s)) {
    //     if (entity_get_save_filter(curr_camera)) entity_save(&curr_camera, "curr_camera", t);

    //     mat3_save(&inverse_view_matrix, "inverse_view_matrix", t);

    //     entitypool_save_foreach(camera, camera_s, Camera::pool, "pool", t) float_save(&camera->viewport_height, "viewport_height", camera_s);
    // }
}
void camera_load_all(App* app) {
    // Store *t, *camera_s;
    // Camera *camera;

    // if (store_child_load(&t, "camera", s)) {
    //     entity_load(&curr_camera, "curr_camera", curr_camera, t);

    //     mat3_load(&inverse_view_matrix, "inverse_view_matrix", mat3_identity(), t);

    //     entitypool_load_foreach(camera, camera_s, Camera::pool, "pool", t) float_load(&camera->viewport_height, "viewport_height", 1, camera_s);
    // }
}
