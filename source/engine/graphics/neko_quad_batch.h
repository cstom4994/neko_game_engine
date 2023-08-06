#ifndef NEKO_QUAD_BATCH_H
#define NEKO_QUAD_BATCH_H

#include "engine/base/neko_engine.h"
#include "engine/graphics/neko_graphics.h"
#include "engine/graphics/neko_mesh.h"
#include "engine/math/neko_math.h"
#include "engine/serialize/neko_byte_buffer.h"

extern const char* __neko_default_quad_batch_vertex_src();

#define neko_quad_batch_default_vertex_src __neko_default_quad_batch_vertex_src()

extern const char* __neko_default_quad_batch_frag_src();

#define neko_quad_batch_default_frag_src __neko_default_quad_batch_frag_src()

typedef struct neko_quad_batch_t {
    neko_byte_buffer_t raw_vertex_data;
    neko_mesh_t mesh;
    neko_resource(neko_material_t) material;  // Pointer to a material instance
} neko_quad_batch_t;

/*
    typedef struct neko_quad_batch_t
    {
        neko_byte_buffer raw_vertex_data;
        neko_mesh_t mesh;
        neko_material_t material;
    } neko_quad_batch_t;
*/

// Default quad vertex structure
// Define your own to match custom materials
typedef struct neko_quad_batch_default_vert_t {
    neko_vec3 position;
    neko_vec2 uv;
    neko_vec4 color;
} neko_quad_batch_default_vert_t;

typedef struct neko_quad_batch_vert_into_t {
    neko_dyn_array(neko_vertex_attribute_type) layout;
} neko_quad_batch_vert_info_t;

// Generic api for submitting custom data for a quad batch
/*
    Quad Batch API

    * Common functionality for quad batch
    * To define CUSTOM functionality, override specific function and information for API:

        - neko_quad_batch_i.shader: neko_resource(neko_shader)
            * Default shader used for quad batch material
            * Define custom vertex/fragment source and then set this shader to construct materials for batches

        - neko_quad_batch_i.vert_info: neko_quad_batch_vert_info_t
            * Holds a neko_dyn_array(neko_vertex_attribute_type) for the vertex layout
            * Initialized by default
            * Reset this layout and then pass in custom vertex information for your custom shader and mesh layout

        - neko_quad_batch_i.add: func
            * This function requires certain parameters, and you can override this functionality with your specific data
                for adding information into the batch
                    * vertex_data: void*
                        - all the data used for your add function
                    * data_size: usize
                        - total size of data
            * I hope to make this part of the interface nicer in the future, but for now, this'll have to do.
*/
typedef struct neko_quad_batch_i {
    neko_quad_batch_t (*construct)(neko_resource(neko_material_t) mat);
    void (*begin)(neko_quad_batch_t*);
    void (*end)(neko_quad_batch_t*);
    void (*add)(neko_quad_batch_t*, void* quad_data);
    void (*submit)(neko_command_buffer_t*, neko_quad_batch_t*);
    void (*free)(neko_quad_batch_t*);
    void (*set_layout)(struct neko_quad_batch_i* api, void* layout, usize layout_size);
    void (*set_shader)(struct neko_quad_batch_i* api, neko_shader_t);
    neko_shader_t shader;
    neko_quad_batch_vert_info_t vert_info;
} neko_quad_batch_i;

extern neko_quad_batch_t neko_quad_batch_new(neko_resource(neko_material_t) mat);
extern void __neko_quad_batch_default_begin(neko_quad_batch_t* batch);

typedef struct neko_default_quad_info_t {
    neko_vqs transform;
    neko_vec4 uv;
    neko_vec4 color;
} neko_default_quad_info_t;

extern void __neko_quad_batch_add_raw_vert_data(neko_quad_batch_t* qb, void* data, usize data_size);
extern void __neko_quad_batch_default_add(neko_quad_batch_t* qb, void* quad_info_data);
extern void __neko_quad_batch_default_end(neko_quad_batch_t* batch);
extern void __neko_quad_batch_default_free(neko_quad_batch_t* batch);
extern void __neko_quad_batch_default_submit(neko_command_buffer_t* cb, neko_quad_batch_t* batch);
extern void __neko_quad_batch_i_set_layout(neko_quad_batch_i* api, void* layout, usize layout_size);
extern void __neko_quad_batch_i_set_shader(neko_quad_batch_i* api, neko_shader_t shader);
extern neko_quad_batch_i __neko_quad_batch_i_new();

#endif  // NEKO_QUAD_BATCH_H
