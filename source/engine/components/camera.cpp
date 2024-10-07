

#include "engine/base/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"

DECL_ENT(Camera, Scalar viewport_height;);

static NativeEntity curr_camera;
static NativeEntity edit_camera;

static mat3 inverse_view_matrix;  // 缓存逆视图矩阵

static NativeEntityPool *pool_camera;

// -------------------------------------------------------------------------

void camera_add(NativeEntity ent) {
    Camera *camera;

    if (entitypool_get(pool_camera, ent)) return;

    transform_add(ent);

    camera = (Camera *)entitypool_add(pool_camera, ent);
    camera->viewport_height = 1.0;

    if (native_entity_eq(curr_camera, entity_nil)) curr_camera = ent;
}
void camera_remove(NativeEntity ent) {
    entitypool_remove(pool_camera, ent);

    if (native_entity_eq(curr_camera, ent)) curr_camera = entity_nil;
}
bool camera_has(NativeEntity ent) { return entitypool_get(pool_camera, ent) != NULL; }

void camera_set_edit_camera(NativeEntity ent) { edit_camera = ent; }

void camera_set_current(NativeEntity ent, bool current) {
    if (current)
        curr_camera = ent;
    else if (native_entity_eq(curr_camera, ent))
        curr_camera = entity_nil;
}
bool camera_get_current(NativeEntity ent) { return native_entity_eq(curr_camera, ent); }
void camera_set_current_camera(NativeEntity ent) { curr_camera = ent; }
NativeEntity camera_get_current_camera() {
    if (edit_get_enabled()) return edit_camera;
    return curr_camera;
}

void camera_set_viewport_height(NativeEntity ent, Scalar height) {
    Camera *camera = (Camera *)entitypool_get(pool_camera, ent);
    error_assert(camera);
    camera->viewport_height = height;
}
Scalar camera_get_viewport_height(NativeEntity ent) {
    Camera *camera = (Camera *)entitypool_get(pool_camera, ent);
    error_assert(camera);
    return camera->viewport_height;
}

mat3 camera_get_inverse_view_matrix() { return inverse_view_matrix; }

const mat3 *camera_get_inverse_view_matrix_ptr() { return &inverse_view_matrix; }

vec2 camera_world_to_pixels(vec2 p) { return game_unit_to_pixels(camera_world_to_unit(p)); }
vec2 camera_world_to_unit(vec2 p) {
    // use cached inverse view matrix
    return mat3_transform(inverse_view_matrix, p);
}
vec2 camera_pixels_to_world(vec2 p) { return camera_unit_to_world(game_pixels_to_unit(p)); }
vec2 camera_unit_to_world(vec2 p) {
    NativeEntity cam = camera_get_current_camera();
    if (!native_entity_eq(cam, entity_nil)) return transform_local_to_world(cam, p);
    return p;
}

// -------------------------------------------------------------------------

void camera_init() {
    PROFILE_FUNC();

    pool_camera = entitypool_new(Camera);
    curr_camera = entity_nil;
    edit_camera = entity_nil;
    inverse_view_matrix = mat3_identity();
}

void camera_fini() { entitypool_free(pool_camera); }

int camera_update_all(App *app, event_t evt) {
    vec2 win_size;
    Scalar aspect;
    Camera *camera;
    NativeEntity cam;
    vec2 scale;
    static BBox bbox = {{-1, -1}, {1, 1}};

    entitypool_remove_destroyed(pool_camera, camera_remove);

    win_size = game_get_window_size();
    aspect = win_size.x / win_size.y;

    entitypool_foreach(camera, pool_camera) {
        scale = luavec2(0.5 * aspect * camera->viewport_height, 0.5 * camera->viewport_height);
        transform_set_scale(camera->pool_elem.ent, scale);

        edit_bboxes_update(camera->pool_elem.ent, bbox);
    }

    cam = camera_get_current_camera();
    if (native_entity_eq(cam, entity_nil))
        inverse_view_matrix = mat3_identity();
    else
        inverse_view_matrix = mat3_inverse(transform_get_world_matrix(cam));

    return 0;
}

void camera_save_all(Store *s) {
    Store *t, *camera_s;
    Camera *camera;

    if (store_child_save(&t, "camera", s)) {
        if (entity_get_save_filter(curr_camera)) entity_save(&curr_camera, "curr_camera", t);

        mat3_save(&inverse_view_matrix, "inverse_view_matrix", t);

        entitypool_save_foreach(camera, camera_s, pool_camera, "pool", t) scalar_save(&camera->viewport_height, "viewport_height", camera_s);
    }
}
void camera_load_all(Store *s) {
    Store *t, *camera_s;
    Camera *camera;

    if (store_child_load(&t, "camera", s)) {
        entity_load(&curr_camera, "curr_camera", curr_camera, t);

        mat3_load(&inverse_view_matrix, "inverse_view_matrix", mat3_identity(), t);

        entitypool_load_foreach(camera, camera_s, pool_camera, "pool", t) scalar_load(&camera->viewport_height, "viewport_height", 1, camera_s);
    }
}
