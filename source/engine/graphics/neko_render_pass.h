#ifndef NEKO_RENDER_PASS_H
#define NEKO_RENDER_PASS_H

#include "engine/neko.h"

// 渲染通道抽象接口
typedef struct render_pass_i {
    void (*pass)(neko_command_buffer_t *cb, struct render_pass_i *pass, void *paramters);
} render_pass_i;

typedef struct blur_data_t {
    u32 iterations;
    neko_texture_t blur_render_target_a;
    neko_texture_t blur_render_target_b;
    neko_texture_t small_blur_render_target_a;
    neko_texture_t small_blur_render_target_b;
    neko_texture_t medium_blur_render_target_a;
    neko_texture_t medium_blur_render_target_b;
    neko_texture_t large_blur_render_target_a;
    neko_texture_t large_blur_render_target_b;
    neko_shader_t horizontal_blur_shader;
    neko_shader_t vertical_blur_shader;
    neko_uniform_t u_input_tex;
    neko_uniform_t u_tex_size;
    neko_vertex_buffer_t vb;
    neko_index_buffer_t ib;
} blur_data_t;

typedef struct blur_pass_t {
    _base(render_pass_i);
    blur_data_t data;
} blur_pass_t;

// Use this to pass in parameters for the pass (will check for this)
typedef struct blur_pass_parameters_t {
    neko_texture_t input_texture;
} blur_pass_parameters_t;

// Used to construct new instance of a blur pass
blur_pass_t blur_pass_ctor();

typedef struct bright_filter_data_t {
    neko_shader_t shader;
    neko_uniform_t u_input_tex;
    neko_texture_t render_target;
    neko_vertex_buffer_t vb;
    neko_index_buffer_t ib;
} bright_filter_data_t;

typedef struct bright_filter_pass_t {
    _base(render_pass_i);
    bright_filter_data_t data;
} bright_filter_pass_t;

// Use this to pass in parameters for the pass (will check for this)
typedef struct bright_filter_pass_parameters_t {
    neko_texture_t input_texture;
} bright_filter_pass_parameters_t;

bright_filter_pass_t bright_filter_pass_ctor();

typedef struct composite_pass_data_t {
    neko_shader_t shader;
    neko_uniform_t u_input_tex;
    neko_uniform_t u_blur_tex;
    neko_uniform_t u_exposure;
    neko_uniform_t u_gamma;
    neko_uniform_t u_bloom_scalar;
    neko_uniform_t u_saturation;
    neko_texture_t render_target;
    neko_vertex_buffer_t vb;
    neko_index_buffer_t ib;
} composite_pass_data_t;

typedef struct composite_pass_t {
    _base(render_pass_i);
    composite_pass_data_t data;
} composite_pass_t;

// Use this to pass in parameters for the pass (will check for this)
typedef struct composite_pass_parameters_t {
    neko_texture_t input_texture;
    neko_texture_t blur_texture;
} composite_pass_parameters_t;

composite_pass_t composite_pass_ctor();

#endif