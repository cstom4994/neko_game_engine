#ifndef NEKO_CAMERA_H
#define NEKO_CAMERA_H

#include "engine/base/neko_object.h"
#include "engine/common/neko_types.h"
#include "engine/math/neko_math.h"

typedef enum { neko_projection_type_orthographic, neko_projection_type_perspective } neko_projection_type;

// TODO(john): enums need to be supported with the reflection generation
typedef struct neko_camera_t {
    neko_vqs transform;
    f32 fov;
    f32 aspect_ratio;
    f32 near_plane;
    f32 far_plane;
    f32 ortho_scale;
    neko_projection_type proj_type;
} neko_camera_t;

neko_camera_t neko_camera_default();
neko_camera_t neko_camera_perspective();
neko_mat4 neko_camera_get_view(neko_camera_t* cam);
neko_mat4 neko_camera_get_projection(neko_camera_t* cam, s32 view_width, s32 view_height);
neko_mat4 neko_camera_get_view_projection(neko_camera_t* cam, s32 view_width, s32 view_height);
neko_vec3 neko_camera_forward(neko_camera_t* cam);
neko_vec3 neko_camera_backward(neko_camera_t* cam);
neko_vec3 neko_camera_up(neko_camera_t* cam);
neko_vec3 neko_camera_down(neko_camera_t* cam);
neko_vec3 neko_camera_right(neko_camera_t* cam);
neko_vec3 neko_camera_left(neko_camera_t* cam);
neko_vec3 neko_camera_unproject(neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height);
void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 picth);

// TODO: Implement world_to_screen and screen_to_world

#endif  // NEKO_CAMERA_H