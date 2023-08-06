#include "engine/graphics/neko_material.h"

#include "engine/graphics/neko_graphics.h"

neko_resource(neko_material_t) neko_material_new(neko_shader_t shader) {
    neko_graphics_i* gfx = neko_engine_subsystem(graphics);
    neko_material_t mat = {0};
    mat.shader = shader;
    mat.uniforms = gfx->uniform_i->construct();
    neko_resource(neko_material_t) handle = neko_resource_cache_insert(gfx->material_cache, mat);
    return handle;
}

void __neko_material_i_set_uniform(neko_resource(neko_material_t) handle, neko_uniform_type type, const char* name, void* data) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_uniform_block_i* uapi = gfx->uniform_i;

    neko_material_t* mat = neko_resource_cache_get_ptr(gfx->material_cache, handle);

    // Either look for uniform or construct it
    // Look for uniform name in uniforms
    // Grab uniform from uniforms
    u64 hash_id = neko_hash_str64(name);
    neko_uniform_t uniform = {};
    neko_uniform_block_t* u_block = neko_slot_array_get_ptr(uapi->uniform_blocks, mat->uniforms.id);

    if (!neko_hash_table_exists(u_block->offset_lookup_table, hash_id)) {
        // Construct or get existing uniform
        uniform = gfx->construct_uniform(mat->shader, name, type);

        // Insert buffer position into offset table (which should be end)
        neko_hash_table_insert(u_block->uniforms, hash_id, uniform);
    } else {
        uniform = neko_hash_table_get(u_block->uniforms, hash_id);
    }

    usize sz = 0;
    switch (type) {
        case neko_uniform_type_mat4:
            sz = sizeof(neko_uniform_block_type(mat4));
            break;
        case neko_uniform_type_vec4:
            sz = sizeof(neko_uniform_block_type(vec4));
            break;
        case neko_uniform_type_vec3:
            sz = sizeof(neko_uniform_block_type(vec3));
            break;
        case neko_uniform_type_vec2:
            sz = sizeof(neko_uniform_block_type(vec2));
            break;
        case neko_uniform_type_float:
            sz = sizeof(neko_uniform_block_type(float));
            break;
        case neko_uniform_type_int:
            sz = sizeof(neko_uniform_block_type(int));
            break;
        case neko_uniform_type_sampler2d:
            sz = sizeof(neko_uniform_block_type(texture_sampler));
            break;
    };

    uapi->set_uniform(mat->uniforms, uniform, name, data, sz);
}

void __neko_material_i_bind_uniforms(neko_command_buffer_t* cb, neko_resource(neko_material_t) handle) {
    neko_graphics_i* gfx = neko_engine_subsystem(graphics);
    neko_material_t* mat = neko_resource_cache_get_ptr(gfx->material_cache, handle);
    gfx->uniform_i->bind_uniforms(cb, mat->uniforms);
}

neko_material_i __neko_material_i_new() {
    neko_material_i api = {0};
    api.construct = &neko_material_new;
    api.set_uniform = &__neko_material_i_set_uniform;
    api.bind_uniforms = &__neko_material_i_bind_uniforms;
    return api;
}
