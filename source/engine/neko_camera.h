#ifndef CAMERA_H
#define CAMERA_H

#include "engine/neko_ecs.h"
#include "neko_base.h"
#include "script_export.h"

/*
 * if no current camera, the (inverse) view matrix is identity, which means
 * the view is a 2x2 unit box at the center of the world
 */

SCRIPT(camera,

       NEKO_EXPORT void camera_add(Entity ent);

       NEKO_EXPORT void camera_remove(Entity ent);

       NEKO_EXPORT bool camera_has(Entity ent);

       // set camera to use in edit mode -- not saved/loaded
       NEKO_EXPORT void camera_set_edit_camera(Entity ent);

       // set/get currently active camera -- entity_nil if none
       NEKO_EXPORT void camera_set_current(Entity ent, bool current);

       NEKO_EXPORT bool camera_get_current(Entity ent);

       NEKO_EXPORT void camera_set_current_camera(Entity ent);

       NEKO_EXPORT Entity camera_get_current_camera();

       // number of world units to fit vertically on screen
       NEKO_EXPORT void camera_set_viewport_height(Entity ent, Scalar height);

       NEKO_EXPORT Scalar camera_get_viewport_height(Entity ent);

       NEKO_EXPORT CMat3 camera_get_inverse_view_matrix();

       // screen-space coordinates <-> world coordinates transformations
       NEKO_EXPORT CVec2 camera_world_to_pixels(CVec2 p);

       NEKO_EXPORT CVec2 camera_world_to_unit(CVec2 p);

       NEKO_EXPORT CVec2 camera_pixels_to_world(CVec2 p);

       NEKO_EXPORT CVec2 camera_unit_to_world(CVec2 p);

)

const CMat3 *camera_get_inverse_view_matrix_ptr();  // for quick GLSL binding

void camera_init();
void camera_deinit();
void camera_update_all();
void camera_save_all(Store *s);
void camera_load_all(Store *s);

#endif
