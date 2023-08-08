#include "neko_render_pass.h"

// fwd
void cp_pass(neko_command_buffer_t* cb, struct render_pass_i* pass, void* paramters);
void bp_pass(neko_command_buffer_t* cb, struct render_pass_i* pass, void* paramters);
void blur_pass(neko_command_buffer_t* cb, render_pass_i* _pass, void* _params);

const char* bp_v_src = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 tex_coord;
void main() {
   gl_Position = vec4(a_position, 0.0, 1.0);
   tex_coord = a_uv;
}
)glsl";

const char* bp_f_src = R"glsl(
#version 330 core
in vec2 tex_coord;
out vec4 frag_color;
uniform sampler2D u_tex;
void main() {
   frag_color = vec4(0.0, 0.0, 0.0, 1.0);
   vec3 tex_color = texture(u_tex, tex_coord).rgb;
   float brightness = dot(tex_color, vec3(0.2126, 0.7152, 0.0722));
   if (tex_color.b < 0.2 && brightness > 0.4) {
       vec3 op = clamp(tex_color, vec3(0), vec3(255));
       frag_color = vec4(op * 0.1, 1.0);
   }
}
)glsl";

// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
neko_global neko_vertex_attribute_type layout_bp[] = {neko_vertex_attribute_float2, neko_vertex_attribute_float2};

// Vertex data for triangle
neko_global f32 bp_v_data[] = {
        // Positions  UVs
        -1.0f, -1.0f, 0.0f, 0.0f,  // Top Left
        1.0f,  -1.0f, 1.0f, 0.0f,  // Top Right
        -1.0f, 1.0f,  0.0f, 1.0f,  // Bottom Left
        1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
};

neko_global u32 bp_i_data[] = {0, 2, 3, 3, 1, 0};

bright_filter_pass_t bright_filter_pass_ctor() {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    bright_filter_pass_t bp = {0};

    // Construct shaders resources
    bp.data.vb = gfx->construct_vertex_buffer(layout_bp, sizeof(layout_bp), bp_v_data, sizeof(bp_v_data));
    bp.data.ib = gfx->construct_index_buffer(bp_i_data, sizeof(bp_i_data));
    bp.data.shader = gfx->neko_shader_create("bright_filter_pass", bp_v_src, bp_f_src);
    bp.data.u_input_tex = gfx->construct_uniform(*bp.data.shader, "u_tex", neko_uniform_type_sampler2d);
    bp._base.pass = &bp_pass;

    // Construct render target to render into
    neko_vec2 ws = platform->window_size(platform->main_window());

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_wrap_s = neko_clamp_to_border;
    t_desc.texture_wrap_t = neko_clamp_to_border;
    t_desc.mag_filter = neko_linear;
    t_desc.min_filter = neko_linear;
    t_desc.texture_format = neko_texture_format_rgba16f;
    t_desc.generate_mips = false;
    t_desc.num_comps = 4;
    t_desc.data = NULL;
    t_desc.width = (s32)(ws.x / 8);
    t_desc.height = (s32)(ws.y / 8);

    // Two render targets for double buffered separable blur (For now, just set to window's viewport)
    bp.data.render_target = gfx->construct_texture(t_desc);

    return bp;
}

void bp_pass(neko_command_buffer_t* cb, struct render_pass_i* _pass, void* _params) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    neko_vec2 ws = platform->window_size(platform->main_window());

    bright_filter_pass_t* bp = (bright_filter_pass_t*)_pass;
    if (!bp) {
        return;
    }

    // Can only use valid params
    bright_filter_pass_parameters_t* params = (bright_filter_pass_parameters_t*)_params;
    if (!params) {
        return;
    }

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_wrap_s = neko_clamp_to_border;
    t_desc.texture_wrap_t = neko_clamp_to_border;
    t_desc.mag_filter = neko_linear;
    t_desc.min_filter = neko_linear;
    t_desc.texture_format = neko_texture_format_rgba16f;
    t_desc.generate_mips = false;
    t_desc.num_comps = 4;
    t_desc.data = NULL;
    t_desc.width = (s32)(ws.x / 8);
    t_desc.height = (s32)(ws.y / 8);

    // Two render targets for double buffered separable blur (For now, just set to window's viewport)
    gfx->update_texture_data(&bp->data.render_target, t_desc);

    // Set frame buffer attachment for rendering
    gfx->set_frame_buffer_attachment(cb, bp->data.render_target, 0);

    // Set viewport
    gfx->set_viewport(cb, 0.f, 0.f, (u32)(ws.x / 8), (u32)(ws.y / 8));

    // Clear
    f32 cc[4] = {0.f, 0.f, 0.f, 1.f};
    gfx->set_view_clear(cb, (f32*)&cc);

    // Use the program
    gfx->bind_shader(cb, *bp->data.shader);
    {
        gfx->bind_texture(cb, bp->data.u_input_tex, params->input_texture, 0);
        gfx->bind_vertex_buffer(cb, bp->data.vb);
        gfx->bind_index_buffer(cb, bp->data.ib);
        gfx->draw_indexed(cb, 6, 0);
    }
}

/*===============
// Blur Shaders
================*/

const char* v_h_blur_src = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 position;
out vec2 tex_coord;
out vec2 v_blur_tex_coords[11];
uniform vec2 u_tex_size;
void main() {
    gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
    tex_coord = a_uv;
    position = a_position;
    vec2 center_tex_coords = a_position * 0.5 + 0.5;
    float pixel_size = 1.0 / u_tex_size.x;
    for (int i = -5; i <= 5; ++i) {
        v_blur_tex_coords[i + 5] = center_tex_coords + vec2(pixel_size * float(i), 0.0);
    }
}
)glsl";

const char* v_v_blur_src = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 position;
out vec2 tex_coord;
out vec2 v_blur_tex_coords[11];
uniform float u_blur_radius;
uniform vec2 u_tex_size;
void main() {
   gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
   tex_coord = a_uv;
   position = a_position;
   vec2 center_tex_coords = a_position * 0.5 + 0.5;
   float pixel_size = 1.0 / u_tex_size.y;
   for (int i = -5; i <= 5; ++i) {
       v_blur_tex_coords[i + 5] = center_tex_coords + vec2(0.0, pixel_size * float(i));
   }
}
)glsl";

const char* f_blur_src = R"glsl(
#version 330 core
in vec2 position;
in vec2 tex_coord;
in vec2 v_blur_tex_coords[11];
out vec4 frag_color;
uniform sampler2D u_tex;
void main() {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    frag_color += texture(u_tex, v_blur_tex_coords[0]) * 0.0093;
    frag_color += texture(u_tex, v_blur_tex_coords[1]) * 0.028002;
    frag_color += texture(u_tex, v_blur_tex_coords[2]) * 0.065984;
    frag_color += texture(u_tex, v_blur_tex_coords[3]) * 0.121703;
    frag_color += texture(u_tex, v_blur_tex_coords[4]) * 0.175713;
    frag_color += texture(u_tex, v_blur_tex_coords[5]) * 0.198596;
    frag_color += texture(u_tex, v_blur_tex_coords[6]) * 0.175713;
    frag_color += texture(u_tex, v_blur_tex_coords[7]) * 0.121703;
    frag_color += texture(u_tex, v_blur_tex_coords[8]) * 0.065984;
    frag_color += texture(u_tex, v_blur_tex_coords[9]) * 0.028002;
    frag_color += texture(u_tex, v_blur_tex_coords[10]) * 0.0093;
}
)glsl";

// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
neko_global neko_vertex_attribute_type layout_blur[] = {neko_vertex_attribute_float2, neko_vertex_attribute_float2};

// Vertex data for triangle
neko_global f32 v_data[] = {
        // Positions  UVs
        -1.0f, -1.0f, 0.0f, 0.0f,  // Top Left
        1.0f,  -1.0f, 1.0f, 0.0f,  // Top Right
        -1.0f, 1.0f,  0.0f, 1.0f,  // Bottom Left
        1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
};

neko_global u32 i_data[] = {0, 2, 3, 3, 1, 0};

// All of these settings are being directly copied from my previous implementation in Enjon to save time
// GraphicsSubsystem: https://github.com/MrFrenik/Enjon/blob/master/Include/Graphics/GraphicsSubsystem.h
blur_pass_t blur_pass_ctor() {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    blur_pass_t bp = {0};
    bp.data.iterations = 1;

    // Set pass for base
    bp._base.pass = &blur_pass;

    // Initialize shaders and uniforms
    bp.data.horizontal_blur_shader = gfx->neko_shader_create("horizontal_blur_shader", v_h_blur_src, f_blur_src);
    bp.data.vertical_blur_shader = gfx->neko_shader_create("vertical_blur_shader", v_v_blur_src, f_blur_src);
    bp.data.u_input_tex = gfx->construct_uniform(*bp.data.horizontal_blur_shader, "u_tex", neko_uniform_type_sampler2d);
    bp.data.u_tex_size = gfx->construct_uniform(*bp.data.horizontal_blur_shader, "u_tex_size", neko_uniform_type_vec2);

    // Construct render target to render into
    neko_vec2 ws = platform->window_size(platform->main_window());

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_wrap_s = neko_clamp_to_border;
    t_desc.texture_wrap_t = neko_clamp_to_border;
    t_desc.mag_filter = neko_linear;
    t_desc.min_filter = neko_linear;
    t_desc.texture_format = neko_texture_format_rgba16f;
    t_desc.generate_mips = false;
    t_desc.num_comps = 4;
    t_desc.data = NULL;
    t_desc.width = (s32)(ws.x / 16);
    t_desc.height = (s32)(ws.y / 16);

    // Two render targets for double buffered separable blur (For now, just set to window's viewport)
    bp.data.blur_render_target_a = gfx->construct_texture(t_desc);
    bp.data.blur_render_target_b = gfx->construct_texture(t_desc);

    bp.data.vb = gfx->construct_vertex_buffer(layout_blur, sizeof(layout_blur), v_data, sizeof(v_data));
    bp.data.ib = gfx->construct_index_buffer(i_data, sizeof(i_data));

    return bp;
}

void blur_pass(neko_command_buffer_t* cb, render_pass_i* _pass, void* _params) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    neko_vec2 ws = platform->window_size(platform->main_window());

    blur_pass_t* bp = (blur_pass_t*)_pass;
    if (!bp) {
        return;
    }

    // Can only use valid params
    blur_pass_parameters_t* params = (blur_pass_parameters_t*)_params;
    if (!params) {
        return;
    }

    neko_vec2 tex_size = neko_vec2{ws.x / 16, ws.y / 16};

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_wrap_s = neko_clamp_to_border;
    t_desc.texture_wrap_t = neko_clamp_to_border;
    t_desc.mag_filter = neko_linear;
    t_desc.min_filter = neko_linear;
    t_desc.texture_format = neko_texture_format_rgba16f;
    t_desc.generate_mips = false;
    t_desc.num_comps = 4;
    t_desc.data = NULL;
    t_desc.width = (s32)(ws.x / 16);
    t_desc.height = (s32)(ws.y / 16);

    // Two render targets for double buffered separable blur (For now, just set to window's viewport)
    gfx->update_texture_data(&bp->data.blur_render_target_a, t_desc);
    gfx->update_texture_data(&bp->data.blur_render_target_b, t_desc);

    for (u32 i = 0; i < bp->data.iterations * 2; ++i) {
        b32 is_even = (i % 2 == 0);
        neko_texture_t* target = is_even ? &bp->data.blur_render_target_a : &bp->data.blur_render_target_b;
        neko_shader_t* shader = is_even ? bp->data.horizontal_blur_shader : bp->data.vertical_blur_shader;
        neko_texture_t* tex = (i == 0) ? &params->input_texture : is_even ? &bp->data.blur_render_target_b : &bp->data.blur_render_target_a;

        // Set frame buffer attachment for rendering
        gfx->set_frame_buffer_attachment(cb, *target, 0);

        // Set viewport
        gfx->set_viewport(cb, 0.f, 0.f, tex_size.x, tex_size.y);
        // Clear
        f32 cc[4] = {0.f, 0.f, 0.f, 1.f};
        gfx->set_view_clear(cb, (f32*)&cc);

        // Use the program
        gfx->bind_shader(cb, *shader);
        {
            gfx->bind_texture(cb, bp->data.u_input_tex, *tex, 0);
            gfx->bind_uniform(cb, bp->data.u_tex_size, &tex_size);
            gfx->bind_vertex_buffer(cb, bp->data.vb);
            gfx->bind_index_buffer(cb, bp->data.ib);
            gfx->draw_indexed(cb, 6, 0);
        }
    }
}

// typedef struct composite_pass_data_t
// {
//  neko_resource(neko_shader) shader;
//  neko_resource (neko_uniform) u_input_tex;
//  neko_resource(neko_texture) render_target;
//  neko_resource(neko_vertex_buffer) vb;
//  neko_resource(neko_index_buffer) ib;
// } composite_pass_data_t;

// typedef struct composite_pass_t
// {
//  _base(render_pass_i);
//  composite_pass_data_t data;
// } composite_pass_t;

// // Use this to pass in parameters for the pass (will check for this)
// typedef struct composite_pass_parameters_t
// {
//  neko_resource(neko_texture) input_texture;
// } composite_pass_parameters_t;

const char* cp_v_src = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 tex_coord;
void main() {
   gl_Position = vec4(a_position, 0.0, 1.0);
   tex_coord = a_uv;
}
)glsl";

const char* cp_f_src = R"glsl(
#version 330 core
in vec2 tex_coord;
out vec4 frag_color;
uniform sampler2D u_tex;
uniform sampler2D u_blur_tex;
uniform float u_exposure;
uniform float u_gamma;
uniform float u_bloom_scalar;
uniform float u_saturation;
float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.20;
vec3 uncharted_tone_map(vec3 x) {
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}
void main() {
   vec3 hdr = max(vec3(0.0), texture(u_tex, tex_coord).rgb);
   vec3 bloom = texture(u_blur_tex, tex_coord).rgb;
   hdr += bloom * u_bloom_scalar;
   vec3 result = vec3(1.0) - exp(-hdr * u_exposure);
   result = pow(result, vec3(1.0 / u_gamma));
   float lum = result.r * 0.2 + result.g * 0.7 + result.b * 0.1;
   vec3 diff = result.rgb - vec3(lum);
   frag_color = vec4(vec3(diff) * u_saturation + lum, 1.0);
   frag_color = vec4(hdr, 1.0);
}
)glsl";

// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
neko_global neko_vertex_attribute_type layout_cp[] = {neko_vertex_attribute_float2, neko_vertex_attribute_float2};

// Vertex data for triangle
neko_global f32 cp_v_data[] = {
        // Positions  UVs
        -1.0f, -1.0f, 0.0f, 0.0f,  // Top Left
        1.0f,  -1.0f, 1.0f, 0.0f,  // Top Right
        -1.0f, 1.0f,  0.0f, 1.0f,  // Bottom Left
        1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
};

neko_global u32 cp_i_data[] = {0, 2, 3, 3, 1, 0};

composite_pass_t composite_pass_ctor() {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    composite_pass_t p = {0};

    // Construct shaders resources
    p.data.vb = gfx->construct_vertex_buffer(layout_cp, sizeof(layout_cp), cp_v_data, sizeof(cp_v_data));
    p.data.ib = gfx->construct_index_buffer(cp_i_data, sizeof(cp_i_data));
    p.data.shader = gfx->neko_shader_create("composite_pass", cp_v_src, cp_f_src);
    p.data.u_input_tex = gfx->construct_uniform(*p.data.shader, "u_tex", neko_uniform_type_sampler2d);
    p.data.u_blur_tex = gfx->construct_uniform(*p.data.shader, "u_blur_tex", neko_uniform_type_sampler2d);
    // p.data.u_exposure = gfx->construct_uniform(p.data.shader, "u_exposure", neko_uniform_type_float);
    // p.data.u_gamma = gfx->construct_uniform(p.data.shader, "u_gamma", neko_uniform_type_float);
    p.data.u_bloom_scalar = gfx->construct_uniform(*p.data.shader, "u_bloom_scalar", neko_uniform_type_float);
    // p.data.u_saturation = gfx->construct_uniform(p.data.shader, "u_saturation", neko_uniform_type_float);

    p._base.pass = &cp_pass;

    // Construct render target to render into
    neko_vec2 ws = platform->window_size(platform->main_window());

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_wrap_s = neko_clamp_to_border;
    t_desc.texture_wrap_t = neko_clamp_to_border;
    t_desc.mag_filter = neko_linear;
    t_desc.min_filter = neko_linear;
    t_desc.texture_format = neko_texture_format_rgba16f;
    t_desc.generate_mips = false;
    t_desc.num_comps = 4;
    t_desc.data = NULL;
    t_desc.width = (s32)(ws.x);
    t_desc.height = (s32)(ws.y);

    // Two render targets for double buffered separable blur (For now, just set to window's viewport)
    p.data.render_target = gfx->construct_texture(t_desc);

    return p;
}

void cp_pass(neko_command_buffer_t* cb, struct render_pass_i* _pass, void* _params) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    neko_vec2 ws = platform->window_size(platform->main_window());

    composite_pass_t* p = (composite_pass_t*)_pass;
    if (!p) {
        return;
    }

    // Can only use valid params
    composite_pass_parameters_t* params = (composite_pass_parameters_t*)_params;
    if (!params) {
        return;
    }

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_wrap_s = neko_clamp_to_border;
    t_desc.texture_wrap_t = neko_clamp_to_border;
    t_desc.mag_filter = neko_linear;
    t_desc.min_filter = neko_linear;
    t_desc.texture_format = neko_texture_format_rgba16f;
    t_desc.generate_mips = false;
    t_desc.num_comps = 4;
    t_desc.data = NULL;
    t_desc.width = (s32)(ws.x);
    t_desc.height = (s32)(ws.y);

    // Two render targets for double buffered separable blur (For now, just set to window's viewport)
    gfx->update_texture_data(&p->data.render_target, t_desc);

    // Set frame buffer attachment for rendering
    gfx->set_frame_buffer_attachment(cb, p->data.render_target, 0);

    // Set viewport
    gfx->set_viewport(cb, 0.f, 0.f, (u32)(ws.x), (u32)(ws.y));

    // Clear
    f32 cc[4] = {0.f, 0.f, 0.f, 1.f};
    gfx->set_view_clear(cb, (f32*)&cc);

    // Use the program
    gfx->bind_shader(cb, *p->data.shader);
    {
        f32 saturation = 2.0f;
        f32 gamma = 2.2f;
        f32 exposure = 0.5f;
        f32 bloom_scalar = 1.0f;

        gfx->bind_texture(cb, p->data.u_input_tex, params->input_texture, 0);
        gfx->bind_texture(cb, p->data.u_blur_tex, params->blur_texture, 1);
        gfx->bind_uniform(cb, p->data.u_saturation, &saturation);
        gfx->bind_uniform(cb, p->data.u_gamma, &gamma);
        gfx->bind_uniform(cb, p->data.u_exposure, &exposure);
        gfx->bind_uniform(cb, p->data.u_bloom_scalar, &bloom_scalar);
        gfx->bind_vertex_buffer(cb, p->data.vb);
        gfx->bind_index_buffer(cb, p->data.ib);
        gfx->draw_indexed(cb, 6, 0);
    }
}
