#include "engine/graphics/neko_quad_batch.h"

#include "engine/graphics/neko_material.h"

neko_quad_batch_t neko_quad_batch_new(neko_resource(neko_material_t) handle) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_quad_batch_i* qbi = gfx->quad_batch_i;

    neko_quad_batch_t qb = {0};
    qb.raw_vertex_data = neko_byte_buffer_new();

    if (handle.id == neko_resource_invalid(neko_material_t).id) {
        // Terrible naming for these apis. Very confusing. Need to standardize this more.
        handle = gfx->material_i->construct(qbi->shader);
    }

    qb.material = handle;

    // Calculate layout size from layout
    void* layout = (void*)gfx->quad_batch_i->vert_info.layout;
    u32 layout_count = neko_dyn_array_size(gfx->quad_batch_i->vert_info.layout);

    // The data for will be empty for now
    qb.mesh.vbo = gfx->construct_vertex_buffer((neko_vertex_attribute_type*)layout, layout_count * sizeof(neko_vertex_attribute_type), NULL, 0);

    return qb;
}

void __neko_quad_batch_default_begin(neko_quad_batch_t* batch) {
    // Reset position of vertex data
    neko_byte_buffer_clear(&batch->raw_vertex_data);
    batch->mesh.vertex_count = 0;
}

void __neko_quad_batch_add_raw_vert_data(neko_quad_batch_t* qb, void* data, usize data_size) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_quad_batch_i* qbi = gfx->quad_batch_i;

    // Calculate layout size from layout
    void* layout = (void*)qbi->vert_info.layout;
    u32 layout_count = neko_dyn_array_size(qbi->vert_info.layout);

    // Calculate vert size
    usize vert_size = gfx->calculate_vertex_size_in_bytes((neko_vertex_attribute_type*)layout, layout_count);

    // In here, you need to know how to read/structure your data to parse the package
    u32 vert_count = data_size / vert_size;
    neko_byte_buffer_bulk_write(&qb->raw_vertex_data, data, data_size);
    qb->mesh.vertex_count += vert_count;
}

void __neko_quad_batch_default_add(neko_quad_batch_t* qb, void* quad_info_data) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    neko_default_quad_info_t* quad_info = (neko_default_quad_info_t*)(quad_info_data);
    if (!quad_info) {
        neko_assert(false);
    }

    neko_vqs transform = quad_info->transform;
    neko_vec4 uv = quad_info->uv;
    neko_vec4 color = quad_info->color;

    // Add as many vertices as you want into the batch...should I perhaps just call this a triangle batch instead?
    // For now, no rotation (just position and scale)
    neko_mat4 model = neko_vqs_to_mat4(&transform);

    neko_vec3 _tl = neko_vec3{-0.5f, -0.5f, 0.f};
    neko_vec3 _tr = neko_vec3{0.5f, -0.5f, 0.f};
    neko_vec3 _bl = neko_vec3{-0.5f, 0.5f, 0.f};
    neko_vec3 _br = neko_vec3{0.5f, 0.5f, 0.f};
    neko_vec4 position = {0};
    neko_quad_batch_default_vert_t tl = {0};
    neko_quad_batch_default_vert_t tr = {0};
    neko_quad_batch_default_vert_t bl = {0};
    neko_quad_batch_default_vert_t br = {0};

    // Top Left
    position = neko_mat4_mul_vec4(model, neko_vec4{_tl.x, _tl.y, _tl.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    tl.position = neko_vec3{position.x, position.y, position.z};
    tl.uv = neko_vec2{uv.x, uv.y};
    tl.color = color;

    // Top Right
    position = neko_mat4_mul_vec4(model, neko_vec4{_tr.x, _tr.y, _tr.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    tr.position = neko_vec3{position.x, position.y, position.z};
    tr.uv = neko_vec2{uv.z, uv.y};
    tr.color = color;

    // Bottom Left
    position = neko_mat4_mul_vec4(model, neko_vec4{_bl.x, _bl.y, _bl.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    bl.position = neko_vec3{position.x, position.y, position.z};
    bl.uv = neko_vec2{uv.x, uv.w};
    bl.color = color;

    // Bottom Right
    position = neko_mat4_mul_vec4(model, neko_vec4{_br.x, _br.y, _br.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    br.position = neko_vec3{position.x, position.y, position.z};
    br.uv = neko_vec2{uv.z, uv.w};
    br.color = color;

    neko_quad_batch_default_vert_t verts[] = {tl, br, bl, tl, tr, br};

    __neko_quad_batch_add_raw_vert_data(qb, verts, sizeof(verts));
}

void __neko_quad_batch_default_end(neko_quad_batch_t* batch) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->update_vertex_buffer_data(batch->mesh.vbo, batch->raw_vertex_data.data, batch->raw_vertex_data.size);
}

void __neko_quad_batch_default_free(neko_quad_batch_t* batch) {
    // Free byte buffer
    neko_byte_buffer_free(&batch->raw_vertex_data);
}

void __neko_quad_batch_default_submit(neko_command_buffer_t* cb, neko_quad_batch_t* batch) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    // Bind material shader
    gfx->bind_material_shader(cb, batch->material);

    // Not liking this too much...
    gfx->bind_material_uniforms(cb, batch->material);

    // Bind quad batch mesh vbo
    gfx->bind_vertex_buffer(cb, batch->mesh.vbo);

    // Draw
    gfx->draw(cb, 0, batch->mesh.vertex_count);
}

void __neko_quad_batch_i_set_layout(neko_quad_batch_i* api, void* layout, usize layout_size) {
    neko_dyn_array_clear(api->vert_info.layout);
    u32 count = layout_size / sizeof(neko_vertex_attribute_type);
    neko_for_range_i(count) { neko_dyn_array_push(api->vert_info.layout, ((neko_vertex_attribute_type*)layout)[i]); }
}

void __neko_quad_batch_i_set_shader(neko_quad_batch_i* api, neko_shader_t shader) { api->shader = shader; }

neko_quad_batch_i __neko_quad_batch_i_new() {
    neko_quad_batch_i api = {0};
    api.begin = &__neko_quad_batch_default_begin;
    api.end = &__neko_quad_batch_default_end;
    api.add = &__neko_quad_batch_default_add;
    api.begin = &__neko_quad_batch_default_begin;
    api.construct = &neko_quad_batch_new;
    api.free = &__neko_quad_batch_default_free;
    api.submit = &__neko_quad_batch_default_submit;
    api.set_layout = &__neko_quad_batch_i_set_layout;
    api.set_shader = &__neko_quad_batch_i_set_shader;

    api.vert_info.layout = neko_dyn_array_new(neko_vertex_attribute_type);
    neko_dyn_array_push(api.vert_info.layout, neko_vertex_attribute_float3);  // Position
    neko_dyn_array_push(api.vert_info.layout, neko_vertex_attribute_float2);  // UV
    neko_dyn_array_push(api.vert_info.layout, neko_vertex_attribute_float4);  // Color

    return api;
}

const char* __neko_default_quad_batch_vertex_src() {
    return R"glsl(
#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
out vec2 uv;
out vec4 color;
void main()
{
   gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
   uv = a_uv;
   color = a_color;
}
)glsl";
}

const char* __neko_default_quad_batch_frag_src() {
    return R"glsl(
#version 330 core
uniform sampler2D u_tex;
in vec2 uv;
in vec4 color;
out vec4 frag_color;
void main() { frag_color = color * texture(u_tex, uv); }
)glsl";
}
