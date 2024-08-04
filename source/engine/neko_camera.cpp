#include "engine/neko_camera.h"

#include "edit.h"
#include "engine/neko_ecs.h"
#include "engine/neko_prelude.h"
#include "engine/neko_base.h"
#include "engine/neko_game.h"
#include "engine/neko_transform.h"

typedef struct Camera Camera;
struct Camera {
    EntityPoolElem pool_elem;

    Scalar viewport_height;
};

static Entity curr_camera;
static Entity edit_camera;

static CMat3 inverse_view_matrix;  // 缓存逆视图矩阵

static EntityPool *pool;

// -------------------------------------------------------------------------

void camera_add(Entity ent) {
    Camera *camera;

    if (entitypool_get(pool, ent)) return;

    transform_add(ent);

    camera = (Camera *)entitypool_add(pool, ent);
    camera->viewport_height = 1.0;

    if (entity_eq(curr_camera, entity_nil)) curr_camera = ent;
}
void camera_remove(Entity ent) {
    entitypool_remove(pool, ent);

    if (entity_eq(curr_camera, ent)) curr_camera = entity_nil;
}
bool camera_has(Entity ent) { return entitypool_get(pool, ent) != NULL; }

void camera_set_edit_camera(Entity ent) { edit_camera = ent; }

void camera_set_current(Entity ent, bool current) {
    if (current)
        curr_camera = ent;
    else if (entity_eq(curr_camera, ent))
        curr_camera = entity_nil;
}
bool camera_get_current(Entity ent) { return entity_eq(curr_camera, ent); }
void camera_set_current_camera(Entity ent) { curr_camera = ent; }
Entity camera_get_current_camera() {
    if (edit_get_enabled()) return edit_camera;
    return curr_camera;
}

void camera_set_viewport_height(Entity ent, Scalar height) {
    Camera *camera = (Camera *)entitypool_get(pool, ent);
    error_assert(camera);
    camera->viewport_height = height;
}
Scalar camera_get_viewport_height(Entity ent) {
    Camera *camera = (Camera *)entitypool_get(pool, ent);
    error_assert(camera);
    return camera->viewport_height;
}

CMat3 camera_get_inverse_view_matrix() { return inverse_view_matrix; }

const CMat3 *camera_get_inverse_view_matrix_ptr() { return &inverse_view_matrix; }

CVec2 camera_world_to_pixels(CVec2 p) { return game_unit_to_pixels(camera_world_to_unit(p)); }
CVec2 camera_world_to_unit(CVec2 p) {
    // use cached inverse view matrix
    return mat3_transform(inverse_view_matrix, p);
}
CVec2 camera_pixels_to_world(CVec2 p) { return camera_unit_to_world(game_pixels_to_unit(p)); }
CVec2 camera_unit_to_world(CVec2 p) {
    Entity cam = camera_get_current_camera();
    if (!entity_eq(cam, entity_nil)) return transform_local_to_world(cam, p);
    return p;
}

// -------------------------------------------------------------------------

void camera_init() {
    pool = entitypool_new(Camera);
    curr_camera = entity_nil;
    edit_camera = entity_nil;
    inverse_view_matrix = mat3_identity();
}
void camera_deinit() { entitypool_free(pool); }

void camera_update_all() {
    CVec2 win_size;
    Scalar aspect;
    Camera *camera;
    Entity cam;
    CVec2 scale;
    static BBox bbox = {{-1, -1}, {1, 1}};

    entitypool_remove_destroyed(pool, camera_remove);

    win_size = game_get_window_size();
    aspect = win_size.x / win_size.y;

    entitypool_foreach(camera, pool) {
        scale = vec2(0.5 * aspect * camera->viewport_height, 0.5 * camera->viewport_height);
        transform_set_scale(camera->pool_elem.ent, scale);

        edit_bboxes_update(camera->pool_elem.ent, bbox);
    }

    cam = camera_get_current_camera();
    if (entity_eq(cam, entity_nil))
        inverse_view_matrix = mat3_identity();
    else
        inverse_view_matrix = mat3_inverse(transform_get_world_matrix(cam));
}

void camera_save_all(Store *s) {
    Store *t, *camera_s;
    Camera *camera;

    if (store_child_save(&t, "camera", s)) {
        if (entity_get_save_filter(curr_camera)) entity_save(&curr_camera, "curr_camera", t);

        mat3_save(&inverse_view_matrix, "inverse_view_matrix", t);

        entitypool_save_foreach(camera, camera_s, pool, "pool", t) scalar_save(&camera->viewport_height, "viewport_height", camera_s);
    }
}
void camera_load_all(Store *s) {
    Store *t, *camera_s;
    Camera *camera;

    if (store_child_load(&t, "camera", s)) {
        entity_load(&curr_camera, "curr_camera", curr_camera, t);

        mat3_load(&inverse_view_matrix, "inverse_view_matrix", mat3_identity(), t);

        entitypool_load_foreach(camera, camera_s, pool, "pool", t) scalar_load(&camera->viewport_height, "viewport_height", 1, camera_s);
    }
}
