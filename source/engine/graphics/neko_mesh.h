#ifndef NEKO_MESH_H
#define NEKO_MESH_H

/*===================
// Mesh
===================*/

typedef struct neko_mesh_t {
    neko_vertex_buffer_t vbo;
    u32 vertex_count;
} neko_mesh_t;

neko_force_inline neko_mesh_t neko_mesh_t_new(neko_vertex_attribute_type *layout_data, usize layout_size, void *v_data, usize v_data_size) {
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_mesh_t mesh = {0};
    mesh.vbo = gfx->construct_vertex_buffer(layout_data, layout_size, v_data, v_data_size);
    mesh.vertex_count = v_data_size / sizeof(f32);
    return mesh;
}

#endif  // NEKO_MESH_H
