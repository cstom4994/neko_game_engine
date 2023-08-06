#include "engine/graphics/neko_camera.h"

#include "engine/base/neko_engine.h"
#include "engine/platform/neko_platform.h"

neko_camera_t neko_camera_default() {
    // Construct default camera parameters
    neko_camera_t cam = neko_default_val();
    cam.transform = neko_vqs_default();
    cam.transform.position = neko_vec3{0.f, 0.f, -1.f};
    cam.fov = 60.f;
    cam.near_plane = 0.1f;
    cam.far_plane = 1000.f;
    cam.ortho_scale = 1.f;
    cam.proj_type = neko_projection_type_orthographic;
    return cam;
}

neko_camera_t neko_camera_perspective() {
    neko_camera_t cam = neko_camera_default();
    cam.proj_type = neko_projection_type_perspective;
    return cam;
}

neko_vec3 neko_camera_forward(neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_vec3{0.0f, 0.0f, -1.0f})); }

neko_vec3 neko_camera_backward(neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_vec3{0.0f, 0.0f, 1.0f})); }

neko_vec3 neko_camera_up(neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_vec3{0.0f, 1.0f, 0.0f})); }

neko_vec3 neko_camera_down(neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_vec3{0.0f, -1.0f, 0.0f})); }

neko_vec3 neko_camera_right(neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_vec3{1.0f, 0.0f, 0.0f})); }

neko_vec3 neko_camera_left(neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_vec3{-1.0f, 0.0f, 0.0f})); }

neko_vec3 neko_camera_unproject(neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height) {
    neko_vec3 wc = neko_default_val();

    // Get inverse of view projection from camera
    neko_mat4 inverse_vp = neko_mat4_inverse(neko_camera_get_view_projection(cam, view_width, view_height));

    f32 w_x = (f32)coords.x;
    f32 w_y = (f32)coords.y;
    f32 w_z = (f32)coords.z;

    // Transform from ndc
    neko_vec4 in;
    in.x = (w_x / (f32)view_width) * 2.f - 1.f;
    in.y = 1.f - (w_y / (f32)view_height) * 2.f;
    in.z = 2.f * w_z - 1.f;
    in.w = 1.f;

    // To world coords
    neko_vec4 out = neko_mat4_mul_vec4(inverse_vp, in);
    if (out.w == 0.f) {
        // Avoid div by zero
        return wc;
    }

    out.w = 1.f / out.w;
    wc = neko_vec3{out.x * out.w, out.y * out.w, out.z * out.w};

    return wc;
}

neko_mat4 neko_camera_get_view_projection(neko_camera_t* cam, s32 view_width, s32 view_height) {
    neko_mat4 view = neko_camera_get_view(cam);
    neko_mat4 proj = neko_camera_get_projection(cam, view_width, view_height);
    return neko_mat4_mul(proj, view);
}

neko_mat4 neko_camera_get_view(neko_camera_t* cam) {
    neko_vec3 up = neko_camera_up(cam);
    neko_vec3 forward = neko_camera_forward(cam);
    neko_vec3 target = neko_vec3_add(forward, cam->transform.position);
    return neko_mat4_look_at(cam->transform.position, target, up);
}

neko_mat4 neko_camera_get_projection(neko_camera_t* cam, s32 view_width, s32 view_height) {
    neko_mat4 proj_mat = neko_mat4_identity();

    switch (cam->proj_type) {
        case neko_projection_type_perspective: {
            proj_mat = neko_mat4_perspective(cam->fov, (f32)view_width / (f32)view_height, cam->near_plane, cam->far_plane);
        } break;

        // Don't like this...
        case neko_projection_type_orthographic: {
            f32 _ar = (f32)view_width / (f32)view_height;
            f32 distance = 0.5f * (cam->far_plane - cam->near_plane);
            const f32 ortho_scale = cam->ortho_scale;
            const f32 aspect_ratio = _ar;
            proj_mat = neko_mat4_ortho(-ortho_scale * aspect_ratio, ortho_scale * aspect_ratio, -ortho_scale, ortho_scale, -distance, distance);
            // (
            //  0.f,
            //  view_width,
            //  view_height,
            //  0.f,
            //  cam->near_plane,
            //  cam->far_plane
            //);
        } break;
    }

    return proj_mat;
}

void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 pitch) {
    neko_quat x = neko_quat_angle_axis(neko_deg_to_rad(yaw), neko_vec3{0.f, 1.f, 0.f});  // Absolute up
    neko_quat y = neko_quat_angle_axis(neko_deg_to_rad(pitch), neko_camera_right(cam));  // Relative right
    cam->transform.rotation = neko_quat_mul(neko_quat_mul(x, y), cam->transform.rotation);
}
