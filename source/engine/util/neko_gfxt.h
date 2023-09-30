
#ifndef NEKO_GFXT_H
#define NEKO_GFXT_H

//=== Raw data retrieval/Handles ===//
#ifndef NEKO_GFXT_HNDL
#define NEKO_GFXT_HNDL void*  // Default handle will just be a pointer to the data itself
#endif

// Config
#ifndef NEKO_GFXT_TEX_COORD_MAX
#define NEKO_GFXT_TEX_COORD_MAX 4
#endif

#ifndef NEKO_GFXT_COLOR_MAX
#define NEKO_GFXT_COLOR_MAX 4
#endif

#ifndef NEKO_GFXT_JOINT_MAX
#define NEKO_GFXT_JOINT_MAX 4
#endif

#ifndef NEKO_GFXT_WEIGHT_MAX
#define NEKO_GFXT_WEIGHT_MAX 4
#endif

// Custom UINT field
#ifndef NEKO_GFXT_CUSTOM_UINT_MAX
#define NEKO_GFXT_CUSTOM_UINT_MAX 4
#endif

#ifndef NEKO_GFXT_UNIFORM_VIEW_MATRIX
#define NEKO_GFXT_UNIFORM_VIEW_MATRIX "U_VIEW_MTX"
#endif

#ifndef NEKO_GFXT_UNIFORM_PROJECTION_MATRIX
#define NEKO_GFXT_UNIFORM_PROJECTION_MATRIX "U_PROJECTION_MTX"
#endif

#ifndef NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX
#define NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX "U_VIEW_PROJECTION_MTX"
#endif

#ifndef NEKO_GFXT_UNIFORM_MODEL_MATRIX
#define NEKO_GFXT_UNIFORM_MODEL_MATRIX "U_MODEL_MTX"
#endif

#ifndef NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX
#define NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX "U_INVERSE_MODEL_MTX"
#endif

#ifndef NEKO_GFXT_UNIFORM_VIEW_WORLD_POSITION
#define NEKO_GFXT_UNIFORM_VIEW_WORLD_POSITION "U_VIEW_WORLD_POSITION"
#endif

#ifndef NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX
#define NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX "U_MVP_MTX"
#endif

#ifndef NEKO_GFXT_UNIFORM_TIME
#define NEKO_GFXT_UNIFORM_TIME "U_TIME"
#endif

typedef void* (*neko_gfxt_raw_data_func)(NEKO_GFXT_HNDL hndl, void* user_data);

#define NEKO_GFXT_RAW_DATA(FUNC_DESC, T) ((T*)(FUNC_DESC)->func((FUNC_DESC)->hndl, (FUNC_DESC)->user_data))

typedef struct neko_gfxt_raw_data_func_desc_t {
    NEKO_GFXT_HNDL hndl;           // Handle used for retrieving data.
    neko_gfxt_raw_data_func func;  // User defined function for pipeline data retrieval
    void* user_data;             // Optional user data for function
} neko_gfxt_raw_data_func_desc_t;

//=== Uniforms/Uniform blocks ===//
typedef struct neko_gfxt_uniform_desc_t {
    char name[64];                        // Name of uniform (for binding to shader)
    neko_graphics_uniform_type type;        // Type of uniform: NEKO_GRAPHICS_UNIFORM_VEC2, NEKO_GRAPHICS_UNIFORM_VEC3, etc.
    uint32_t binding;                     // Binding for this uniform in shader
    neko_graphics_shader_stage_type stage;  // Shader stage for this uniform
    neko_graphics_access_type access_type;  // Access type for this uniform (compute only)
} neko_gfxt_uniform_desc_t;

typedef struct neko_gfxt_uniform_t {
    neko_handle(neko_graphics_uniform_t) hndl;  // Graphics handle resource for actual uniform
    uint32_t offset;                        // Individual offset for this uniform in material byte buffer data
    uint32_t binding;                       // Binding for this uniform
    size_t size;                            // Size of this uniform data in bytes
    neko_graphics_uniform_type type;          // Type of this uniform
    neko_graphics_access_type access_type;    // Access type of uniform (compute only)
} neko_gfxt_uniform_t;

typedef struct neko_gfxt_uniform_block_desc_t {
    neko_gfxt_uniform_desc_t* layout;  // Layout for all uniform data for this block to hold
    size_t size;                     // Size of layout in bytes
} neko_gfxt_uniform_block_desc_t;

typedef struct neko_gfxt_uniform_block_lookup_key_t {
    char name[64];
} neko_gfxt_uniform_block_lookup_key_t;

typedef struct neko_gfxt_uniform_block_t {
    neko_dyn_array(neko_gfxt_uniform_t) uniforms;  // Raw uniform handle array
    neko_hash_table(uint64_t, uint32_t) lookup;  // Index lookup table (used for byte buffer offsets in material uni. data)
    size_t size;                               // Total size of material data for entire block
} neko_gfxt_uniform_block_t;

//=== Texture ===//
typedef neko_handle(neko_graphics_texture_t) neko_gfxt_texture_t;

//=== Mesh ===//
typedef neko_asset_mesh_attribute_type neko_gfxt_mesh_attribute_type;
typedef neko_asset_mesh_layout_t neko_gfxt_mesh_layout_t;

/*
    typedef struct
    {
        union
        {
            void* interleave;
            struct
            {
                void* positions;
                void* normals;
                void* tangents;
                void* tex_coords[TEX_COORD_MAX];
                void* joints[JOINT_MAX];
                void* weights[WEIGHT_MAX];
            } non_interleave;
        } vertex;
        size_t vertex_size;
        void* indices;
    } neko_gfxt_mesh_primitive_data_t;
*/

typedef struct {
    void* data;
    size_t size;
} neko_gfxt_mesh_vertex_attribute_t;

typedef struct {
    neko_gfxt_mesh_vertex_attribute_t positions;  // All position data
    neko_gfxt_mesh_vertex_attribute_t normals;
    neko_gfxt_mesh_vertex_attribute_t tangents;
    neko_gfxt_mesh_vertex_attribute_t tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_gfxt_mesh_vertex_attribute_t colors[NEKO_GFXT_COLOR_MAX];
    neko_gfxt_mesh_vertex_attribute_t joints[NEKO_GFXT_JOINT_MAX];
    neko_gfxt_mesh_vertex_attribute_t weights[NEKO_GFXT_WEIGHT_MAX];
    neko_gfxt_mesh_vertex_attribute_t custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
    neko_gfxt_mesh_vertex_attribute_t indices;
    uint32_t count;  // Total count of indices
} neko_gfxt_mesh_vertex_data_t;

// Structured/packed raw mesh data
typedef struct neko_gfxt_mesh_raw_data_t {
    neko_dyn_array(neko_gfxt_mesh_vertex_data_t) primitives;  // All primitive data
} neko_gfxt_mesh_raw_data_t;

typedef struct neko_gfxt_mesh_import_options_t {
    neko_gfxt_mesh_layout_t* layout;     // Mesh attribute layout array
    size_t size;                       // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_gfxt_mesh_import_options_t;

NEKO_API_DECL void neko_gfxt_mesh_import_options_free(neko_gfxt_mesh_import_options_t* opt);

typedef struct neko_gfxt_mesh_desc_s {
    neko_gfxt_mesh_raw_data_t* meshes;  // Mesh data array
    size_t size;                      // Size of mesh data array in bytes
    bool32 keep_data;                 // Whether or not to free data after use
} neko_gfxt_mesh_desc_t;

typedef struct neko_gfxt_vertex_stream_s {
    neko_handle(neko_graphics_vertex_buffer_t) positions;
    neko_handle(neko_graphics_vertex_buffer_t) normals;
    neko_handle(neko_graphics_vertex_buffer_t) tangents;
    neko_handle(neko_graphics_vertex_buffer_t) colors[NEKO_GFXT_COLOR_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) joints[NEKO_GFXT_JOINT_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) weights[NEKO_GFXT_WEIGHT_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
} neko_gfxt_vertex_stream_t;

typedef struct neko_gfxt_mesh_primitive_s {
    neko_gfxt_vertex_stream_t stream;                 // All vertex data streams
    neko_handle(neko_graphics_index_buffer_t) indices;  // Index buffer
    uint32_t count;                                 // Total number of vertices
} neko_gfxt_mesh_primitive_t;

typedef struct neko_gfxt_mesh_s {
    neko_dyn_array(neko_gfxt_mesh_primitive_t) primitives;
    neko_gfxt_mesh_desc_t desc;
} neko_gfxt_mesh_t;

//=== Pipeline ===//
typedef struct neko_gfxt_pipeline_desc_s {
    neko_graphics_pipeline_desc_t pip_desc;      // Description for constructing pipeline object
    neko_gfxt_uniform_block_desc_t ublock_desc;  // Description for constructing uniform block object
} neko_gfxt_pipeline_desc_t;

typedef struct neko_gfxt_pipeline_s {
    neko_handle(neko_graphics_pipeline_t) hndl;  // Graphics handle resource for actual pipeline
    neko_gfxt_uniform_block_t ublock;          // Uniform block for holding all uniform data
    neko_dyn_array(neko_gfxt_mesh_layout_t) mesh_layout;
    neko_graphics_pipeline_desc_t desc;
} neko_gfxt_pipeline_t;

//=== Material ===//
typedef struct neko_gfxt_material_desc_s {
    neko_gfxt_raw_data_func_desc_t pip_func;  // Description for retrieving raw pipeline pointer data from handle.
} neko_gfxt_material_desc_t;

typedef struct neko_gfxt_material_s {
    neko_gfxt_material_desc_t desc;        // Material description object
    neko_byte_buffer_t uniform_data;       // Byte buffer of actual uniform data to send to GPU
    neko_byte_buffer_t image_buffer_data;  // Image buffer data
} neko_gfxt_material_t;

//=== Renderable ===//
typedef struct neko_gfxt_renderable_desc_s {
    neko_gfxt_raw_data_func_desc_t mesh;      // Description for retrieving raw mesh pointer data from handle.
    neko_gfxt_raw_data_func_desc_t material;  // Description for retrieving raw material pointer data from handle.
} neko_gfxt_renderable_desc_t;

typedef struct neko_gfxt_renderable_s {
    neko_gfxt_renderable_desc_t desc;  // Renderable description object
    neko_mat4 model_matrix;            // Model matrix for renderable
} neko_gfxt_renderable_t;

//=== Graphics scene ===//
typedef struct neko_gfxt_scene_s {
    neko_slot_array(neko_gfxt_renderable_t) renderables;
} neko_gfxt_scene_t;

//==== API =====//

//=== Creation ===//
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_create(const neko_gfxt_pipeline_desc_t* desc);
NEKO_API_DECL neko_gfxt_material_t neko_gfxt_material_create(neko_gfxt_material_desc_t* desc);
NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_create(const neko_gfxt_mesh_desc_t* desc);
NEKO_API_DECL void neko_gfxt_mesh_update_or_create(neko_gfxt_mesh_t* mesh, const neko_gfxt_mesh_desc_t* desc);
NEKO_API_DECL neko_gfxt_renderable_t neko_gfxt_renderable_create(const neko_gfxt_renderable_desc_t* desc);
NEKO_API_DECL neko_gfxt_uniform_block_t neko_gfxt_uniform_block_create(const neko_gfxt_uniform_block_desc_t* desc);
NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_create(neko_graphics_texture_desc_t* desc);

//=== Destruction ===//
NEKO_API_DECL void neko_gfxt_texture_destroy(neko_gfxt_texture_t* texture);
NEKO_API_DECL void neko_gfxt_material_destroy(neko_gfxt_material_t* material);
NEKO_API_DECL void neko_gfxt_mesh_destroy(neko_gfxt_mesh_t* mesh);
NEKO_API_DECL void neko_gfxt_uniform_block_destroy(neko_gfxt_uniform_block_t* ub);
NEKO_API_DECL void neko_gfxt_pipeline_destroy(neko_gfxt_pipeline_t* pipeline);

//=== Resource Loading ===//
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_file(const char* path);
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_memory(const char* data, size_t sz);
NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_load_from_file(const char* path, neko_graphics_texture_desc_t* desc, bool flip, bool keep_data);
NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_load_from_memory(const char* data, size_t sz, neko_graphics_texture_desc_t* desc, bool flip, bool keep_data);

//=== Copy ===//
NEKO_API_DECL neko_gfxt_material_t neko_gfxt_material_deep_copy(neko_gfxt_material_t* src);

//=== Pipeline API ===//
NEKO_API_DECL neko_gfxt_uniform_t* neko_gfxt_pipeline_get_uniform(neko_gfxt_pipeline_t* pip, const char* name);

//=== Material API ===//
NEKO_API_DECL void neko_gfxt_material_set_uniform(neko_gfxt_material_t* mat, const char* name, const void* data);
NEKO_API_DECL void neko_gfxt_material_bind(neko_command_buffer_t* cb, neko_gfxt_material_t* mat);
NEKO_API_DECL void neko_gfxt_material_bind_pipeline(neko_command_buffer_t* cb, neko_gfxt_material_t* mat);
NEKO_API_DECL void neko_gfxt_material_bind_uniforms(neko_command_buffer_t* cb, neko_gfxt_material_t* mat);
NEKO_API_DECL neko_gfxt_pipeline_t* neko_gfxt_material_get_pipeline(neko_gfxt_material_t* mat);

//=== Mesh API ===//
NEKO_API_DECL void neko_gfxt_mesh_draw_pipeline(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_pipeline_t* pip);
NEKO_API_DECL void neko_gfxt_mesh_draw_material(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_material_t* mat);
NEKO_API_DECL void neko_gfxt_mesh_draw_materials(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_material_t** mats, size_t mats_size);
NEKO_API_DECL void neko_gfxt_mesh_draw_layout(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_mesh_layout_t* layout, size_t layout_size);
NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_load_from_file(const char* file, neko_gfxt_mesh_import_options_t* options);
NEKO_API_DECL bool neko_gfxt_load_gltf_data_from_file(const char* path, neko_gfxt_mesh_import_options_t* options, neko_gfxt_mesh_raw_data_t** out, uint32_t* mesh_count);

// Util API
NEKO_API_DECL void* neko_gfxt_raw_data_default_impl(NEKO_GFXT_HNDL hndl, void* user_data);

// Mesh Generation API
NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_unit_quad_generate(neko_gfxt_mesh_import_options_t* options);
neko_handle(neko_graphics_texture_t) neko_gfxt_texture_generate_default();

/** @} */  // end of neko_graphics_extension_util

#ifdef NEKO_GFXT_IMPL
/*==== Implementation ====*/

// Creation/Destruction
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_create(const neko_gfxt_pipeline_desc_t* desc) {
    neko_gfxt_pipeline_t pip = neko_default_val();

    if (!desc) {
        neko_assert(false);
        return pip;
    }

    pip.hndl = neko_graphics_pipeline_create(&desc->pip_desc);
    pip.ublock = neko_gfxt_uniform_block_create(&desc->ublock_desc);
    pip.desc = desc->pip_desc;
    pip.desc.layout.attrs = neko_malloc(desc->pip_desc.layout.size);
    memcpy(pip.desc.layout.attrs, desc->pip_desc.layout.attrs, desc->pip_desc.layout.size);
    return pip;
}

NEKO_API_DECL neko_gfxt_uniform_block_t neko_gfxt_uniform_block_create(const neko_gfxt_uniform_block_desc_t* desc) {
    neko_gfxt_uniform_block_t block = neko_default_val();

    if (!desc) return block;

    // Iterate through layout, construct uniforms, place them into hash table
    uint32_t offset = 0;
    uint32_t image2D_offset = 0;
    uint32_t ct = desc->size / sizeof(neko_gfxt_uniform_desc_t);
    for (uint32_t i = 0; i < ct; ++i) {
        neko_gfxt_uniform_desc_t* ud = &desc->layout[i];

        neko_gfxt_uniform_t u = neko_default_val();
        neko_graphics_uniform_desc_t u_desc = neko_default_val();
        neko_graphics_uniform_layout_desc_t u_layout = neko_default_val();
        u_layout.type = ud->type;
        memcpy(u_desc.name, ud->name, 64);
        u_desc.layout = &u_layout;
        u.binding = ud->binding;
        u.type = ud->type;

        // Determine offset/hndl
        switch (ud->type) {
            case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
                u.offset = image2D_offset;
            } break;

            default: {
                u.hndl = neko_graphics_uniform_create(&u_desc);
                u.offset = offset;
            } break;
        }

        // Add to data offset based on type
        switch (ud->type) {
            default:
            case NEKO_GRAPHICS_UNIFORM_FLOAT:
                offset += sizeof(float);
                break;
            case NEKO_GRAPHICS_UNIFORM_INT:
                offset += sizeof(int32_t);
                break;
            case NEKO_GRAPHICS_UNIFORM_VEC2:
                offset += sizeof(neko_vec2);
                break;
            case NEKO_GRAPHICS_UNIFORM_VEC3:
                offset += sizeof(neko_vec3);
                break;
            case NEKO_GRAPHICS_UNIFORM_VEC4:
                offset += sizeof(neko_vec4);
                break;
            case NEKO_GRAPHICS_UNIFORM_MAT4:
                offset += sizeof(neko_mat4);
                break;
            case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
                offset += sizeof(neko_handle(neko_graphics_texture_t));
                break;
            case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
                offset += sizeof(neko_handle(neko_graphics_texture_t));
                break;
            case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
                image2D_offset += sizeof(neko_handle(neko_graphics_texture_t));
            } break;
        }

        // Add uniform to block with name as key
        uint64_t key = neko_hash_str64(ud->name);
        neko_dyn_array_push(block.uniforms, u);
        neko_hash_table_insert(block.lookup, key, neko_dyn_array_size(block.uniforms) - 1);
    }
    block.size = offset;

    return block;
}

NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_create(neko_graphics_texture_desc_t* desc) { return neko_graphics_texture_create(desc); }

NEKO_API_DECL neko_gfxt_material_t neko_gfxt_material_create(neko_gfxt_material_desc_t* desc) {
    neko_gfxt_material_t mat = neko_default_val();

    if (!desc) {
        neko_assert(false);
        return mat;
    }

    // Set desc information to defaults if not provided.
    if (!desc->pip_func.func) desc->pip_func.func = neko_gfxt_raw_data_default_impl;
    neko_gfxt_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&desc->pip_func, neko_gfxt_pipeline_t);
    neko_assert(pip);

    mat.desc = *desc;
    mat.uniform_data = neko_byte_buffer_new();
    mat.image_buffer_data = neko_byte_buffer_new();

    neko_byte_buffer_resize(&mat.uniform_data, pip->ublock.size);
    neko_byte_buffer_memset(&mat.uniform_data, 0);
    return mat;
}

NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_create(const neko_gfxt_mesh_desc_t* desc) {
    neko_gfxt_mesh_t mesh = neko_default_val();

    if (!desc) {
        return mesh;
    }

    const uint32_t mesh_count = desc->size / sizeof(neko_gfxt_mesh_raw_data_t);

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i) {
        neko_gfxt_mesh_raw_data_t* m = &desc->meshes[i];

        for (uint32_t p = 0; p < neko_dyn_array_size(m->primitives); ++p) {
            // Get raw vertex data
            neko_gfxt_mesh_vertex_data_t* vdata = &m->primitives[p];

            // Construct primitive
            neko_gfxt_mesh_primitive_t prim = neko_default_val();
            prim.count = vdata->count;

            // Positions
            if (vdata->positions.data) {
                neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                vdesc.data = vdata->positions.data;
                vdesc.size = vdata->positions.size;
                prim.stream.positions = neko_graphics_vertex_buffer_create(&vdesc);
                if (!desc->keep_data) {
                    neko_free(vdata->positions.data);
                }
            }

            // Normals
            if (vdata->normals.data) {
                neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                vdesc.data = vdata->normals.data;
                vdesc.size = vdata->normals.size;
                prim.stream.normals = neko_graphics_vertex_buffer_create(&vdesc);
                if (!desc->keep_data) {
                    neko_free(vdata->normals.data);
                }
            }

            // Tangents
            if (vdata->tangents.data) {
                neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                vdesc.data = vdata->tangents.data;
                vdesc.size = vdata->tangents.size;
                prim.stream.tangents = neko_graphics_vertex_buffer_create(&vdesc);
                if (!desc->keep_data) {
                    neko_free(vdata->tangents.data);
                }
            }

            // Texcoords
            for (uint32_t j = 0; j < NEKO_GFXT_TEX_COORD_MAX; ++j) {
                if (vdata->tex_coords[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->tex_coords[j].data;
                    vdesc.size = vdata->tex_coords[j].size;
                    prim.stream.tex_coords[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->tex_coords[j].data);
                    }
                }
            }

            // Colors
            for (uint32_t j = 0; j < NEKO_GFXT_COLOR_MAX; ++j) {
                if (vdata->colors[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->colors[j].data;
                    vdesc.size = vdata->colors[j].size;
                    prim.stream.colors[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->colors[j].data);
                    }
                }
            }

            // Joints
            for (uint32_t j = 0; j < NEKO_GFXT_JOINT_MAX; ++j) {
                if (vdata->joints[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->joints[j].data;
                    vdesc.size = vdata->joints[j].size;
                    prim.stream.joints[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->joints[j].data);
                    }
                }
            }

            // Weights
            for (uint32_t j = 0; j < NEKO_GFXT_WEIGHT_MAX; ++j) {
                if (vdata->weights[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->weights[j].data;
                    vdesc.size = vdata->weights[j].size;
                    prim.stream.weights[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->weights[j].data);
                    }
                }
            }

            // Index buffer decl
            neko_graphics_index_buffer_desc_t idesc = neko_default_val();
            idesc.data = vdata->indices.data;
            idesc.size = vdata->indices.size;

            // Construct index buffer for primitive
            prim.indices = neko_graphics_index_buffer_create(&idesc);

            if (!desc->keep_data) {
                neko_free(vdata->indices.data);
            }

            // Add primitive to mesh
            neko_dyn_array_push(mesh.primitives, prim);
        }

        if (!desc->keep_data) {
            neko_dyn_array_free(m->primitives);
        }
    }

    if (!desc->keep_data) {
        neko_free(desc->meshes);
    }

    return mesh;
}

NEKO_API_DECL void neko_gfxt_mesh_update_or_create(neko_gfxt_mesh_t* mesh, const neko_gfxt_mesh_desc_t* desc) {
    if (!desc || !mesh) {
        return;
    }

    /*
    // Need to create mesh if not already done
    if (neko_dyn_array_empty(mesh->primitives)) {
        *mesh = neko_gfxt_mesh_create(desc);
        return;
    }
    */

    const uint32_t mesh_count = desc->size / sizeof(neko_gfxt_mesh_raw_data_t);

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i) {
        neko_gfxt_mesh_raw_data_t* m = &desc->meshes[i];

        for (uint32_t p = 0; p < neko_dyn_array_size(m->primitives); ++p) {
            // Get raw vertex data
            neko_gfxt_mesh_vertex_data_t* vdata = &m->primitives[p];

            // Construct or retrieve mesh primitive
            neko_gfxt_mesh_primitive_t* prim = NULL;
            if (neko_dyn_array_empty(mesh->primitives) || neko_dyn_array_size(mesh->primitives) < p) {
                neko_gfxt_mesh_primitive_t dprim = neko_default_val();
                neko_dyn_array_push(mesh->primitives, dprim);
            }
            prim = &mesh->primitives[p];

            // Set prim count
            prim->count = vdata->count;

            // Positions
            if (vdata->positions.data) {
                neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                vdesc.data = vdata->positions.data;
                vdesc.size = vdata->positions.size;

                // Update
                if (prim->stream.positions.id) {
                    neko_graphics_vertex_buffer_update(prim->stream.positions, &vdesc);
                }
                // Create
                else {
                    prim->stream.positions = neko_graphics_vertex_buffer_create(&vdesc);
                }
                if (!desc->keep_data) {
                    neko_free(vdata->positions.data);
                }
            }

            // Normals
            if (vdata->normals.data) {
                neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                vdesc.data = vdata->normals.data;
                vdesc.size = vdata->normals.size;

                // Update
                if (prim->stream.normals.id) {
                    neko_graphics_vertex_buffer_update(prim->stream.normals, &vdesc);
                } else {
                    prim->stream.normals = neko_graphics_vertex_buffer_create(&vdesc);
                }
                if (!desc->keep_data) {
                    neko_free(vdata->normals.data);
                }
            }

            // Tangents
            if (vdata->tangents.data) {
                neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                vdesc.data = vdata->tangents.data;
                vdesc.size = vdata->tangents.size;

                if (prim->stream.tangents.id) {
                    neko_graphics_vertex_buffer_update(prim->stream.tangents, &vdesc);
                } else {
                    prim->stream.tangents = neko_graphics_vertex_buffer_create(&vdesc);
                }
                if (!desc->keep_data) {
                    neko_free(vdata->tangents.data);
                }
            }

            // Texcoords
            for (uint32_t j = 0; j < NEKO_GFXT_TEX_COORD_MAX; ++j) {
                if (vdata->tex_coords[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->tex_coords[j].data;
                    vdesc.size = vdata->tex_coords[j].size;

                    if (prim->stream.tex_coords[j].id) {
                        neko_graphics_vertex_buffer_update(prim->stream.tex_coords[j], &vdesc);
                    } else {
                        prim->stream.tex_coords[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->tex_coords[j].data);
                    }
                }
            }

            // Colors
            for (uint32_t j = 0; j < NEKO_GFXT_COLOR_MAX; ++j) {
                if (vdata->colors[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->colors[j].data;
                    vdesc.size = vdata->colors[j].size;

                    if (prim->stream.colors[j].id) {
                        neko_graphics_vertex_buffer_update(prim->stream.colors[j], &vdesc);
                    } else {
                        prim->stream.colors[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->colors[j].data);
                    }
                }
            }

            // Joints
            for (uint32_t j = 0; j < NEKO_GFXT_JOINT_MAX; ++j) {
                if (vdata->joints[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->joints[j].data;
                    vdesc.size = vdata->joints[j].size;

                    if (prim->stream.joints[j].id) {
                        neko_graphics_vertex_buffer_update(prim->stream.joints[j], &vdesc);
                    } else {
                        prim->stream.joints[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->joints[j].data);
                    }
                }
            }

            // Weights
            for (uint32_t j = 0; j < NEKO_GFXT_WEIGHT_MAX; ++j) {
                if (vdata->weights[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->weights[j].data;
                    vdesc.size = vdata->weights[j].size;

                    if (prim->stream.weights[j].id) {
                        neko_graphics_vertex_buffer_update(prim->stream.weights[j], &vdesc);
                    } else {
                        prim->stream.weights[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->weights[j].data);
                    }
                }
            }

            // Custom uint
            for (uint32_t j = 0; j < NEKO_GFXT_CUSTOM_UINT_MAX; ++j) {
                if (vdata->custom_uint[j].data) {
                    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
                    vdesc.data = vdata->custom_uint[j].data;
                    vdesc.size = vdata->custom_uint[j].size;

                    if (prim->stream.custom_uint[j].id) {
                        neko_graphics_vertex_buffer_update(prim->stream.custom_uint[j], &vdesc);
                    } else {
                        prim->stream.custom_uint[j] = neko_graphics_vertex_buffer_create(&vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->custom_uint[j].data);
                    }
                }
            }

            // Index buffer decl
            neko_graphics_index_buffer_desc_t idesc = neko_default_val();
            idesc.data = vdata->indices.data;
            idesc.size = vdata->indices.size;

            // Construct index buffer for primitive
            if (prim->indices.id) {
                neko_graphics_index_buffer_update(prim->indices, &idesc);
            } else {
                prim->indices = neko_graphics_index_buffer_create(&idesc);
            }

            if (!desc->keep_data) {
                neko_free(vdata->indices.data);
            }
        }

        if (!desc->keep_data) {
            neko_dyn_array_free(m->primitives);
        }
    }

    if (!desc->keep_data) {
        neko_free(desc->meshes);
    }
}

NEKO_API_DECL neko_gfxt_renderable_t neko_gfxt_renderable_create(const neko_gfxt_renderable_desc_t* desc) {
    neko_gfxt_renderable_t rend = neko_default_val();

    if (!desc) {
        return rend;
    }

    rend.model_matrix = neko_mat4_identity();
    rend.desc = *desc;

    return rend;
}

//=== Destruction ===//
NEKO_API_DECL void neko_gfxt_texture_destroy(neko_gfxt_texture_t* texture) { neko_graphics_texture_destroy(*texture); }

NEKO_API_DECL void neko_gfxt_material_destroy(neko_gfxt_material_t* material) {
    // Destroy all material data
    neko_byte_buffer_free(&material->uniform_data);
    neko_byte_buffer_free(&material->image_buffer_data);
}

NEKO_API_DECL void neko_gfxt_mesh_destroy(neko_gfxt_mesh_t* mesh) {
    // Iterate through all primitives, destroy all vertex and index buffers
    for (uint32_t p = 0; p < neko_dyn_array_size(mesh->primitives); ++p) {
        neko_gfxt_mesh_primitive_t* prim = &mesh->primitives[p];

        // Free index buffer
        if (prim->indices.id) neko_graphics_index_buffer_destroy(prim->indices);

        // Free vertex stream
        if (prim->stream.positions.id) neko_graphics_vertex_buffer_destroy(prim->stream.positions);
        if (prim->stream.normals.id) neko_graphics_vertex_buffer_destroy(prim->stream.normals);
        if (prim->stream.tangents.id) neko_graphics_vertex_buffer_destroy(prim->stream.tangents);

        for (uint32_t i = 0; i < NEKO_GFXT_COLOR_MAX; ++i) {
            if (prim->stream.colors[i].id) neko_graphics_vertex_buffer_destroy(prim->stream.colors[i]);
        }

        for (uint32_t i = 0; i < NEKO_GFXT_TEX_COORD_MAX; ++i) {
            if (prim->stream.tex_coords[i].id) neko_graphics_vertex_buffer_destroy(prim->stream.tex_coords[i]);
        }

        for (uint32_t i = 0; i < NEKO_GFXT_JOINT_MAX; ++i) {
            if (prim->stream.joints[i].id) neko_graphics_vertex_buffer_destroy(prim->stream.joints[i]);
        }

        for (uint32_t i = 0; i < NEKO_GFXT_WEIGHT_MAX; ++i) {
            if (prim->stream.weights[i].id) neko_graphics_vertex_buffer_destroy(prim->stream.weights[i]);
        }
    }
}

NEKO_API_DECL void neko_gfxt_uniform_block_destroy(neko_gfxt_uniform_block_t* ub) {
    for (uint32_t i = 0; i < neko_dyn_array_size(ub->uniforms); ++i) {
        neko_gfxt_uniform_t* u = &ub->uniforms[i];
        neko_graphics_uniform_destroy(u->hndl);
    }

    neko_dyn_array_free(ub->uniforms);
    neko_hash_table_free(ub->lookup);
}

NEKO_API_DECL void neko_gfxt_pipeline_destroy(neko_gfxt_pipeline_t* pipeline) {
    // Destroy uniform block for pipeline
    neko_gfxt_uniform_block_destroy(&pipeline->ublock);

    // Free shaders (if responsible for them)
    neko_graphics_shader_destroy(pipeline->desc.raster.shader);

    // Destroy pipeline
    if (pipeline->desc.layout.attrs) neko_free(pipeline->desc.layout.attrs);
    if (pipeline->mesh_layout) neko_dyn_array_free(pipeline->mesh_layout);
    neko_graphics_pipeline_destroy(pipeline->hndl);
}

//=== Copy API ===//

NEKO_API_DECL neko_gfxt_material_t neko_gfxt_material_deep_copy(neko_gfxt_material_t* src) {
    neko_gfxt_material_t mat = neko_gfxt_material_create(&src->desc);
    neko_byte_buffer_copy_contents(&mat.uniform_data, &src->uniform_data);
    return mat;
}

//=== Pipeline API ===//
NEKO_API_DECL neko_gfxt_uniform_t* neko_gfxt_pipeline_get_uniform(neko_gfxt_pipeline_t* pip, const char* name) {
    uint64_t key = neko_hash_str64(name);
    if (!neko_hash_table_exists(pip->ublock.lookup, key)) {
        return NULL;
    }
    // Based on name, need to get uniform
    uint32_t uidx = neko_hash_table_get(pip->ublock.lookup, key);
    return &pip->ublock.uniforms[uidx];
}

//=== Material API ===//

NEKO_API_DECL
void neko_gfxt_material_set_uniform(neko_gfxt_material_t* mat, const char* name, const void* data) {
    if (!mat || !name || !data) return;

    neko_gfxt_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_gfxt_pipeline_t);
    neko_assert(pip);

    // Get key for name lookup
    uint64_t key = neko_hash_str64(name);
    if (!neko_hash_table_exists(pip->ublock.lookup, key)) {
        neko_timed_action(60, { neko_log_warning("Unable to find uniform: %s", name); });
        return;
    }

    // Based on name, need to get uniform
    uint32_t uidx = neko_hash_table_get(pip->ublock.lookup, key);
    neko_gfxt_uniform_t* u = &pip->ublock.uniforms[uidx];

    // Seek to beginning of data
    neko_byte_buffer_seek_to_beg(&mat->uniform_data);
    neko_byte_buffer_seek_to_beg(&mat->image_buffer_data);

    // Advance by offset
    switch (u->type) {
        case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:
            neko_byte_buffer_advance_position(&mat->image_buffer_data, u->offset);
            break;
        default:
            neko_byte_buffer_advance_position(&mat->uniform_data, u->offset);
            break;
    }

    switch (u->type) {
        case NEKO_GRAPHICS_UNIFORM_FLOAT:
            neko_byte_buffer_write(&mat->uniform_data, float, *(float*)data);
            break;
        case NEKO_GRAPHICS_UNIFORM_INT:
            neko_byte_buffer_write(&mat->uniform_data, int32_t, *(int32_t*)data);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC2:
            neko_byte_buffer_write(&mat->uniform_data, neko_vec2, *(neko_vec2*)data);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC3:
            neko_byte_buffer_write(&mat->uniform_data, neko_vec3, *(neko_vec3*)data);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC4:
            neko_byte_buffer_write(&mat->uniform_data, neko_vec4, *(neko_vec4*)data);
            break;
        case NEKO_GRAPHICS_UNIFORM_MAT4:
            neko_byte_buffer_write(&mat->uniform_data, neko_mat4, *(neko_mat4*)data);
            break;

        case NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE:
        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D: {
            neko_byte_buffer_write(&mat->uniform_data, neko_handle(neko_graphics_texture_t), *(neko_handle(neko_graphics_texture_t)*)data);
        } break;

        case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
            neko_byte_buffer_write(&mat->image_buffer_data, neko_handle(neko_graphics_texture_t), *(neko_handle(neko_graphics_texture_t)*)data);
        } break;
    }
}

NEKO_API_DECL neko_gfxt_pipeline_t* neko_gfxt_material_get_pipeline(neko_gfxt_material_t* mat) {
    neko_gfxt_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_gfxt_pipeline_t);
    return pip;
}

NEKO_API_DECL
void neko_gfxt_material_bind(neko_command_buffer_t* cb, neko_gfxt_material_t* mat) {
    neko_gfxt_material_bind_pipeline(cb, mat);
    neko_gfxt_material_bind_uniforms(cb, mat);
}

NEKO_API_DECL
void neko_gfxt_material_bind_pipeline(neko_command_buffer_t* cb, neko_gfxt_material_t* mat) {
    // Binds the pipeline
    neko_gfxt_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_gfxt_pipeline_t);
    neko_assert(pip);
    neko_graphics_pipeline_bind(cb, pip->hndl);
}

NEKO_API_DECL
void neko_gfxt_material_bind_uniforms(neko_command_buffer_t* cb, neko_gfxt_material_t* mat) {
    if (!mat) return;

    neko_gfxt_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_gfxt_pipeline_t);
    neko_assert(pip);

    // Grab uniform layout from pipeline
    for (uint32_t i = 0; i < neko_dyn_array_size(pip->ublock.uniforms); ++i) {
        neko_gfxt_uniform_t* u = &pip->ublock.uniforms[i];
        neko_graphics_bind_desc_t bind = neko_default_val();

        // Need to buffer these up so it's a single call...
        switch (u->type) {
            case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
                neko_graphics_bind_image_buffer_desc_t ibuffer[1];
                ibuffer[0].tex = *(neko_handle(neko_graphics_texture_t)*)(mat->image_buffer_data.data + u->offset);
                ibuffer[0].binding = u->binding;
                ibuffer[0].access = NEKO_GRAPHICS_ACCESS_WRITE_ONLY;
                bind.image_buffers.desc = ibuffer;
                bind.image_buffers.size = sizeof(ibuffer);
                neko_graphics_apply_bindings(cb, &bind);
            } break;

            default: {
                neko_graphics_bind_uniform_desc_t uniforms[1];
                uniforms[0].uniform = u->hndl;
                uniforms[0].data = (mat->uniform_data.data + u->offset);
                uniforms[0].binding = u->binding;
                bind.uniforms.desc = uniforms;
                bind.uniforms.size = sizeof(uniforms);
                neko_graphics_apply_bindings(cb, &bind);
            } break;
        }
    }
}

// Mesh API
NEKO_API_DECL void neko_gfxt_mesh_draw(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mp) {
    /*
    // For each primitive in mesh
    for (uint32_t i = 0; i < neko_dyn_array_size(mp->primitives); ++i)
    {
        neko_gfxt_mesh_primitive_t* prim = &mp->primitives[i];

        // Bindings for all buffers: vertex, index, uniform, sampler
        neko_graphics_bind_desc_t binds = neko_default_val();
        neko_graphics_bind_vertex_buffer_desc_t vdesc = neko_default_val();
        neko_graphics_bind_index_buffer_desc_t idesc = neko_default_val();
        vdesc.buffer = prim->vbo;
        idesc.buffer = prim->indices;
        binds.vertex_buffers.desc = &vdesc;
        binds.index_buffers.desc = &idesc;
        neko_graphics_draw_desc_t ddesc = neko_default_val();
        ddesc.start = 0;
        ddesc.count = prim->count;
        neko_graphics_apply_bindings(cb, &binds);
        neko_graphics_draw(cb, &ddesc);
    }
    */
}

NEKO_API_DECL void neko_gfxt_mesh_primitive_draw_layout(neko_command_buffer_t* cb, neko_gfxt_mesh_primitive_t* prim, neko_gfxt_mesh_layout_t* layout, size_t layout_size) {
    if (!layout || !layout_size || !prim || !cb) {
        return;
    }

    neko_graphics_bind_vertex_buffer_desc_t vbos[8] = {0};  // Make this a define
    uint32_t l = 0;
    const uint32_t ct = layout_size / sizeof(neko_gfxt_mesh_layout_t);
    for (uint32_t a = 0; a < ct; ++a) {
        vbos[l].data_type = NEKO_GRAPHICS_VERTEX_DATA_NONINTERLEAVED;
        switch (layout[a].type) {
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                if (!prim->stream.positions.id) continue;
                vbos[l].buffer = prim->stream.positions;
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                if (!prim->stream.normals.id) continue;
                vbos[l].buffer = prim->stream.normals;
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                if (!prim->stream.tangents.id) continue;
                vbos[l].buffer = prim->stream.tangents;
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT: {
                if (!prim->stream.joints[0].id) continue;
                vbos[l].buffer = prim->stream.joints[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT: {
                if (!prim->stream.weights[0].id) continue;
                vbos[l].buffer = prim->stream.weights[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                if (!prim->stream.tex_coords[0].id) continue;
                vbos[l].buffer = prim->stream.tex_coords[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                if (!prim->stream.colors[0].id) continue;
                vbos[l].buffer = prim->stream.colors[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT: {
                if (!prim->stream.custom_uint[0].id) continue;
                vbos[l].buffer = prim->stream.custom_uint[0];
            } break;
        }
        ++l;
    }

    neko_graphics_bind_index_buffer_desc_t ibos = neko_default_val();
    ibos.buffer = prim->indices;

    // Bindings for all buffers: vertex, index, uniform, sampler
    neko_graphics_bind_desc_t binds = neko_default_val();

    // .vertex_buffers = {.desc = vbos, .size = sizeof(vbos)},
    binds.vertex_buffers.desc = vbos;
    binds.vertex_buffers.size = l * sizeof(neko_graphics_bind_vertex_buffer_desc_t);
    binds.index_buffers.desc = &ibos;

    neko_graphics_draw_desc_t ddesc = neko_default_val();
    ddesc.start = 0;
    ddesc.count = prim->count;

    neko_graphics_apply_bindings(cb, &binds);
    neko_graphics_draw(cb, &ddesc);
}

NEKO_API_DECL void neko_gfxt_mesh_draw_layout(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_mesh_layout_t* layout, size_t layout_size) {
    if (!layout || !mesh || !cb) {
        return;
    }

    uint32_t ct = layout_size / sizeof(neko_gfxt_mesh_layout_t);

    // For each primitive in mesh
    for (uint32_t i = 0; i < neko_dyn_array_size(mesh->primitives); ++i) {
        neko_gfxt_mesh_primitive_t* prim = &mesh->primitives[i];
        neko_gfxt_mesh_primitive_draw_layout(cb, prim, layout, layout_size);
    }
}

NEKO_API_DECL void neko_gfxt_mesh_draw_materials(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_material_t** mats, size_t mats_size) {
    // Iterate through primitives, draw each primitive with assigned mat
    if (!mats || !mats_size || !cb || !mesh) {
        return;
    }

    const uint32_t ct = mats_size / sizeof(neko_gfxt_material_t*);
    neko_gfxt_material_t* mat = NULL;

    // For each primitive in mesh
    for (uint32_t i = 0; i < neko_dyn_array_size(mesh->primitives); ++i) {
        neko_gfxt_mesh_primitive_t* prim = &mesh->primitives[i];

        // Get corresponding material, if available
        uint32_t mat_idx = i < ct ? i : ct - 1;
        mat = mats[mat_idx] ? mats[mat_idx] : mat;

        // Can't draw without a valid material present
        if (!mat) continue;

        // Bind material pipeline and uniforms
        neko_gfxt_material_bind(cb, mat);

        // Get pipeline
        neko_gfxt_pipeline_t* pip = neko_gfxt_material_get_pipeline(mat);

        neko_gfxt_mesh_primitive_draw_layout(cb, prim, pip->mesh_layout, neko_dyn_array_size(pip->mesh_layout) * sizeof(neko_gfxt_mesh_layout_t));
    }
}

NEKO_API_DECL void neko_gfxt_mesh_draw_material(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_material_t* mat) {
    if (!mat || !mesh || !cb) {
        return;
    }

    neko_gfxt_pipeline_t* pip = neko_gfxt_material_get_pipeline(mat);
    neko_gfxt_mesh_draw_layout(cb, mesh, pip->mesh_layout, neko_dyn_array_size(pip->mesh_layout) * sizeof(neko_gfxt_mesh_layout_t));
}

NEKO_API_DECL void neko_gfxt_mesh_draw_pipeline(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_pipeline_t* pip) {
    if (!pip || !mesh || !cb) {
        return;
    }

    neko_gfxt_mesh_draw_layout(cb, mesh, pip->mesh_layout, neko_dyn_array_size(pip->mesh_layout) * sizeof(neko_gfxt_mesh_layout_t));
}

// Util API
NEKO_API_DECL
void* neko_gfxt_raw_data_default_impl(NEKO_GFXT_HNDL hndl, void* user_data) { return hndl; }

NEKO_API_DECL void neko_gfxt_mesh_import_options_free(neko_gfxt_mesh_import_options_t* opt) {
    if (opt->layout) {
        neko_dyn_array_free(opt->layout);
    }
}

NEKO_API_DECL
neko_gfxt_mesh_t neko_gfxt_mesh_load_from_file(const char* path, neko_gfxt_mesh_import_options_t* options) {
    neko_gfxt_mesh_t mesh = neko_default_val();

    if (!neko_platform_file_exists(path)) {
        neko_println("Warning:GFXT:MeshLoadFromFile:File does not exist: %s", path);
        return mesh;
    }

    // Mesh data to fill out
    uint32_t mesh_count = 0;
    neko_gfxt_mesh_raw_data_t* meshes = NULL;

    // Get file extension from path
    neko_transient_buffer(file_ext, 32);
    neko_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (neko_string_compare_equal(file_ext, "gltf")) {
        neko_gfxt_load_gltf_data_from_file(path, options, &meshes, &mesh_count);
    }
    // GLB
    else if (neko_string_compare_equal(file_ext, "glb")) {
        neko_gfxt_load_gltf_data_from_file(path, options, &meshes, &mesh_count);
    } else {
        neko_println("Warning:GFXT:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return mesh;
    }

    neko_gfxt_mesh_desc_t mdesc = neko_default_val();
    mdesc.meshes = meshes;
    mdesc.size = mesh_count * sizeof(neko_gfxt_mesh_raw_data_t);

    mesh = neko_gfxt_mesh_create(&mdesc);
    mesh.desc = mdesc;

    return mesh;
}

NEKO_API_DECL bool neko_gfxt_load_gltf_data_from_file(const char* path, neko_gfxt_mesh_import_options_t* options, neko_gfxt_mesh_raw_data_t** out, uint32_t* mesh_count) {
    // Use cgltf like a boss
    cgltf_options cgltf_options = neko_default_val();
    size_t len = 0;
    char* file_data = NULL;

    // Get file extension from path
    neko_transient_buffer(file_ext, 32);
    neko_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (neko_string_compare_equal(file_ext, "gltf")) {
        file_data = neko_platform_read_file_contents(path, "rb", &len);
        neko_println("GFXT:Loading GLTF: %s", path);
    }
    // GLB
    else if (neko_string_compare_equal(file_ext, "glb")) {
        file_data = neko_platform_read_file_contents(path, "rb", &len);
        neko_println("GFXT:Loading GLTF: %s", path);
    } else {
        neko_println("Warning:GFXT:LoadGLTFDataFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return false;
    }

    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&cgltf_options, file_data, (cgltf_size)len, &data);
    neko_free(file_data);

    if (result != cgltf_result_success) {
        neko_println("GFXT:Mesh:LoadFromFile:Failed load gltf");
        cgltf_free(data);
        return false;
    }

    // Load buffers as well
    result = cgltf_load_buffers(&cgltf_options, data, path);
    if (result != cgltf_result_success) {
        cgltf_free(data);
        neko_println("GFXT:Mesh:LoadFromFile:Failed to load buffers");
        return false;
    }

    // Type of index data
    size_t index_element_size = options ? options->index_buffer_element_size : 0;

    // Temporary structures
    neko_dyn_array(neko_vec3) positions = NULL;
    neko_dyn_array(neko_vec3) normals = NULL;
    neko_dyn_array(neko_vec3) tangents = NULL;
    neko_dyn_array(neko_color_t) colors[NEKO_GFXT_COLOR_MAX] = neko_default_val();
    neko_dyn_array(neko_vec2) uvs[NEKO_GFXT_TEX_COORD_MAX] = neko_default_val();
    neko_dyn_array(float) weights[NEKO_GFXT_WEIGHT_MAX] = neko_default_val();
    neko_dyn_array(float) joints[NEKO_GFXT_JOINT_MAX] = neko_default_val();
    neko_dyn_array(neko_gfxt_mesh_layout_t) layouts = neko_default_val();
    neko_byte_buffer_t v_data = neko_byte_buffer_new();
    neko_byte_buffer_t i_data = neko_byte_buffer_new();
    neko_mat4 world_mat = neko_mat4_identity();

    // Allocate memory for buffers
    *mesh_count = data->meshes_count;
    *out = (neko_gfxt_mesh_raw_data_t*)neko_malloc(data->meshes_count * sizeof(neko_gfxt_mesh_raw_data_t));
    memset(*out, 0, sizeof(neko_gfxt_mesh_raw_data_t) * data->meshes_count);

    // For each node, for each mesh
    uint32_t i = 0;
    for (uint32_t _n = 0; _n < data->nodes_count; ++_n) {
        cgltf_node* node = &data->nodes[_n];
        if (node->mesh == NULL) continue;

        neko_println("Load mesh from node: %s", node->name);

        // Reset matrix
        world_mat = neko_mat4_identity();

        // neko_println("i: %zu, r: %zu, t: %zu, s: %zu, m: %zu", i, node->has_rotation, node->has_translation, node->has_scale, node->has_matrix);

        // Not sure what "local transform" does, since world gives me the actual world result...probably for animation
        if (node->has_rotation || node->has_translation || node->has_scale) {
            cgltf_node_transform_world(node, (float*)&world_mat);
        }

        // if (node->has_matrix)
        // {
        //  // Multiply local by world
        //  neko_mat4 tmp = neko_default_val();
        //  cgltf_node_transform_world(node, (float*)&tmp);
        //  world_mat = neko_mat4_mul(world_mat, tmp);
        // }

        // Do node mesh data
        cgltf_mesh* cmesh = node->mesh;
        {
            // Initialize mesh data
            neko_gfxt_mesh_raw_data_t* mesh = &((*out)[i]);
            bool warnings[neko_enum_count(neko_asset_mesh_attribute_type)] = neko_default_val();
            bool printed = false;

            // For each primitive in mesh
            for (uint32_t p = 0; p < cmesh->primitives_count; ++p) {
                cgltf_primitive* prim = &cmesh->primitives[p];

                // Mesh primitive to fill out
                neko_gfxt_mesh_vertex_data_t primitive = neko_default_val();

                // Clear temp data from previous use
                neko_dyn_array_clear(positions);
                neko_dyn_array_clear(normals);
                neko_dyn_array_clear(tangents);
                for (uint32_t ci = 0; ci < NEKO_GFXT_COLOR_MAX; ++ci) neko_dyn_array_clear(colors[ci]);
                for (uint32_t tci = 0; tci < NEKO_GFXT_TEX_COORD_MAX; ++tci) neko_dyn_array_clear(uvs[tci]);
                for (uint32_t wi = 0; wi < NEKO_GFXT_WEIGHT_MAX; ++wi) neko_dyn_array_clear(weights[wi]);
                for (uint32_t ji = 0; ji < NEKO_GFXT_JOINT_MAX; ++ji) neko_dyn_array_clear(joints[ji]);
                neko_dyn_array_clear(layouts);
                neko_byte_buffer_clear(&v_data);
                neko_byte_buffer_clear(&i_data);

// Collect all provided attribute data for each vertex that's available in gltf data
#define __GFXT_GLTF_PUSH_ATTR(ATTR, TYPE, COUNT, ARR, ARR_TYPE, LAYOUTS, LAYOUT_TYPE)                                                \
    do {                                                                                                                             \
        int32_t N = 0;                                                                                                               \
        TYPE* BUF = (TYPE*)ATTR->buffer_view->buffer->data + ATTR->buffer_view->offset / sizeof(TYPE) + ATTR->offset / sizeof(TYPE); \
        neko_assert(BUF);                                                                                                              \
        TYPE V[COUNT] = neko_default_val();                                                                                            \
        /* For each vertex */                                                                                                        \
        for (uint32_t k = 0; k < ATTR->count; k++) {                                                                                 \
            /* For each element */                                                                                                   \
            for (int l = 0; l < COUNT; l++) {                                                                                        \
                V[l] = BUF[N + l];                                                                                                   \
            }                                                                                                                        \
            N += (int32_t)(ATTR->stride / sizeof(TYPE));                                                                             \
            /* Add to temp data array */                                                                                             \
            ARR_TYPE ELEM = neko_default_val();                                                                                        \
            memcpy((void*)&ELEM, (void*)V, sizeof(ARR_TYPE));                                                                        \
            neko_dyn_array_push(ARR, ELEM);                                                                                            \
        }                                                                                                                            \
        /* Push into layout */                                                                                                       \
        neko_gfxt_mesh_layout_t LAYOUT = neko_default_val();                                                                             \
        LAYOUT.type = LAYOUT_TYPE;                                                                                                   \
        neko_dyn_array_push(LAYOUTS, LAYOUT);                                                                                          \
    } while (0)

                // For each attribute in primitive
                for (uint32_t a = 0; a < prim->attributes_count; ++a) {
                    // Accessor for attribute data
                    cgltf_accessor* attr = prim->attributes[a].data;

                    // Index for data
                    int32_t aidx = prim->attributes[a].index;

                    // Switch on type for reading data
                    switch (prim->attributes[a].type) {
                        case cgltf_attribute_type_position: {
                            int32_t N = 0;
                            float* BUF = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset / sizeof(float) + attr->offset / sizeof(float);
                            neko_assert(BUF);
                            float V[3] = neko_default_val();
                            /* For each vertex */
                            for (uint32_t k = 0; k < attr->count; k++) {
                                /* For each element */
                                for (int l = 0; l < 3; l++) {
                                    V[l] = BUF[N + l];
                                }
                                N += (int32_t)(attr->stride / sizeof(float));
                                /* Add to temp data array */
                                neko_vec3 ELEM = neko_default_val();
                                memcpy((void*)&ELEM, (void*)V, sizeof(neko_vec3));
                                // Transform into world space
                                ELEM = neko_mat4_mul_vec3(world_mat, ELEM);
                                neko_dyn_array_push(positions, ELEM);
                            }
                            /* Push into layout */
                            neko_gfxt_mesh_layout_t LAYOUT = neko_default_val();
                            LAYOUT.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION;
                            neko_dyn_array_push(layouts, LAYOUT);
                        } break;

                        case cgltf_attribute_type_normal: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 3, normals, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                        } break;

                        case cgltf_attribute_type_tangent: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 3, tangents, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                        } break;

                        case cgltf_attribute_type_texcoord: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 2, uvs[aidx], neko_vec2, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                        } break;

                        case cgltf_attribute_type_color: {
                            // Need to parse color as sRGB then convert to neko_color_t
                            int32_t N = 0;
                            float* BUF = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset / sizeof(float) + attr->offset / sizeof(float);
                            neko_assert(BUF);
                            float V[3] = neko_default_val();
                            /* For each vertex */
                            for (uint32_t k = 0; k < attr->count; k++) {
                                /* For each element */
                                for (int l = 0; l < 3; l++) {
                                    V[l] = BUF[N + l];
                                }
                                N += (int32_t)(attr->stride / sizeof(float));
                                /* Add to temp data array */
                                neko_color_t ELEM = neko_default_val();
                                // Need to convert over now
                                ELEM.r = (uint8_t)(V[0] * 255.f);
                                ELEM.g = (uint8_t)(V[1] * 255.f);
                                ELEM.b = (uint8_t)(V[2] * 255.f);
                                ELEM.a = 255;
                                neko_dyn_array_push(colors[aidx], ELEM);
                            }
                            /* Push into layout */
                            neko_gfxt_mesh_layout_t LAYOUT = neko_default_val();
                            LAYOUT.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR;
                            neko_dyn_array_push(layouts, LAYOUT);
                        } break;

                        // Not sure what to do with these for now
                        case cgltf_attribute_type_joints: {
                            // Push into layout
                            neko_gfxt_mesh_layout_t layout = neko_default_val();
                            layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT;
                            neko_dyn_array_push(layouts, layout);
                        } break;

                        case cgltf_attribute_type_weights: {
                            // Push into layout
                            neko_gfxt_mesh_layout_t layout = neko_default_val();
                            layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT;
                            neko_dyn_array_push(layouts, layout);
                        } break;

                        // Shouldn't hit here...
                        default: {
                        } break;
                    }
                }

                // Indices for primitive
                cgltf_accessor* acc = prim->indices;

#define __GFXT_GLTF_PUSH_IDX(BB, ACC, TYPE)                                                                                       \
    do {                                                                                                                          \
        int32_t n = 0;                                                                                                            \
        TYPE* buf = (TYPE*)acc->buffer_view->buffer->data + acc->buffer_view->offset / sizeof(TYPE) + acc->offset / sizeof(TYPE); \
        neko_assert(buf);                                                                                                           \
        TYPE v = 0;                                                                                                               \
        /* For each index */                                                                                                      \
        for (uint32_t k = 0; k < acc->count; k++) {                                                                               \
            /* For each element */                                                                                                \
            for (int l = 0; l < 1; l++) {                                                                                         \
                v = buf[n + l];                                                                                                   \
            }                                                                                                                     \
            n += (int32_t)(acc->stride / sizeof(TYPE));                                                                           \
            /* Add to temp positions array */                                                                                     \
            switch (index_element_size) {                                                                                         \
                case 0:                                                                                                           \
                    neko_byte_buffer_write(BB, uint16_t, (uint16_t)v);                                                              \
                    break;                                                                                                        \
                case 2:                                                                                                           \
                    neko_byte_buffer_write(BB, uint16_t, (uint16_t)v);                                                              \
                    break;                                                                                                        \
                case 4:                                                                                                           \
                    neko_byte_buffer_write(BB, uint32_t, (uint32_t)v);                                                              \
                    break;                                                                                                        \
            }                                                                                                                     \
        }                                                                                                                         \
    } while (0)

                // If indices are available
                if (acc) {
                    switch (acc->component_type) {
                        case cgltf_component_type_r_8:
                            __GFXT_GLTF_PUSH_IDX(&i_data, acc, int8_t);
                            break;
                        case cgltf_component_type_r_8u:
                            __GFXT_GLTF_PUSH_IDX(&i_data, acc, uint8_t);
                            break;
                        case cgltf_component_type_r_16:
                            __GFXT_GLTF_PUSH_IDX(&i_data, acc, int16_t);
                            break;
                        case cgltf_component_type_r_16u:
                            __GFXT_GLTF_PUSH_IDX(&i_data, acc, uint16_t);
                            break;
                        case cgltf_component_type_r_32u:
                            __GFXT_GLTF_PUSH_IDX(&i_data, acc, uint32_t);
                            break;
                        case cgltf_component_type_r_32f:
                            __GFXT_GLTF_PUSH_IDX(&i_data, acc, float);
                            break;

                        // Shouldn't hit here
                        default: {
                        } break;
                    }
                } else {
                    // Iterate over positions size, then just push back indices
                    for (uint32_t i = 0; i < neko_dyn_array_size(positions); ++i) {
                        switch (index_element_size) {
                            default:
                            case 0:
                                neko_byte_buffer_write(&i_data, uint16_t, (uint16_t)i);
                                break;
                            case 2:
                                neko_byte_buffer_write(&i_data, uint16_t, (uint16_t)i);
                                break;
                            case 4:
                                neko_byte_buffer_write(&i_data, uint32_t, (uint32_t)i);
                                break;
                        }
                    }
                }

                // Grab mesh layout pointer to use
                /*
                neko_gfxt_mesh_layout_t* layoutp = options ? options->layout : layouts;
                uint32_t layout_ct = options ? options->size / sizeof(neko_gfxt_mesh_layout_t) : neko_dyn_array_size(layouts);

                // Iterate layout to fill data buffers according to provided layout
                {
                    uint32_t vct = 0;
                    vct = neko_max(vct, neko_dyn_array_size(positions));
                    vct = neko_max(vct, neko_dyn_array_size(colors));
                    vct = neko_max(vct, neko_dyn_array_size(uvs));
                    vct = neko_max(vct, neko_dyn_array_size(normals));
                    vct = neko_max(vct, neko_dyn_array_size(tangents));

                    #define __GLTF_WRITE_DATA(IT, VDATA, ARR, ARR_TYPE, ARR_DEF_VAL, LAYOUT_TYPE)\
                        do {\
                            if (IT < neko_dyn_array_size(ARR)) {\
                                neko_byte_buffer_write(&(VDATA), ARR_TYPE, ARR[IT]);\
                            }\
                            else {\
                                neko_byte_buffer_write(&(VDATA), ARR_TYPE, ARR_DEF_VAL);\
                                if (!warnings[LAYOUT_TYPE]) {\
                                    warnings[LAYOUT_TYPE] = true;\
                                }\
                            }\
                        } while (0)

                    for (uint32_t it = 0; it < vct; ++it)
                    {
                        // For each attribute in layout
                        for (uint32_t l = 0; l < layout_ct; ++l)
                        {
                            switch (layoutp[l].type)
                            {
                                case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                                    __GLTF_WRITE_DATA(it, v_data, positions, neko_vec3, neko_v3(0.f, 0.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION);
                                } break;

                                case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                                    __GLTF_WRITE_DATA(it, v_data, uvs, neko_vec2, neko_v2(0.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                                } break;

                                case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                                    __GLTF_WRITE_DATA(it, v_data, colors, neko_color_t, NEKO_COLOR_WHITE, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR);
                                } break;

                                case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                                    __GLTF_WRITE_DATA(it, v_data, normals, neko_vec3, neko_v3(0.f, 0.f, 1.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                                } break;

                                case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                                    __GLTF_WRITE_DATA(it, v_data, tangents, neko_vec3, neko_v3(0.f, 1.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                                } break;

                                default:
                                {
                                } break;
                            }
                        }
                    }
                }

                // Add to out data
                mesh->vertices[p] = neko_malloc(v_data.size);
                mesh->indices[p] = neko_malloc(i_data.size);
                mesh->vertex_sizes[p] = v_data.size;
                mesh->index_sizes[p] = i_data.size;

                // Copy data
                memcpy(mesh->vertices[p], v_data.data, v_data.size);
                memcpy(mesh->indices[p], i_data.data, i_data.size);
                */

                /*
                    typedef struct
                    {
                        void* data;
                        size_t size;
                    } neko_gfxt_mesh_vertex_attribute_t;

                    typedef struct
                    {
                        neko_gfxt_mesh_vertex_attribute_t positions;         // All position data
                        neko_gfxt_mesh_vertex_attribute_t normals;
                        neko_gfxt_mesh_vertex_attribute_t tangents;
                        neko_gfxt_mesh_vertex_attribute_t tex_coords[NEKO_GFXT_TEX_COORD_MAX];
                        neko_gfxt_mesh_vertex_attribute_t joints[NEKO_GFXT_JOINT_MAX];
                        neko_gfxt_mesh_vertex_attribute_t weights[NEKO_GFXT_WEIGHT_MAX];
                        neko_gfxt_mesh_vertex_attribute_t indices;
                    } neko_gfxt_mesh_vertex_data_t;

                    // Structured/packed raw mesh data
                    typedef struct neko_gfxt_mesh_raw_data_t {
                        uint16_t prim_count;
                        size_t* vertex_sizes;
                        size_t* index_sizes;
                        void** vertices;
                        void** indices;

                        neko_dyn_array(neko_gfxt_mesh_vertex_data_t) primitives;   // All primitive data
                    } neko_gfxt_mesh_raw_data_t;
                */

                // Count
                primitive.count = prim->indices->count;

                // Indices
                primitive.indices.size = i_data.size;
                primitive.indices.data = neko_malloc(i_data.size);
                memcpy(primitive.indices.data, i_data.data, i_data.size);

                // Positions
                if (!neko_dyn_array_empty(positions)) {
                    primitive.positions.size = neko_dyn_array_size(positions) * sizeof(neko_vec3);
                    primitive.positions.data = neko_malloc(primitive.positions.size);
                    memcpy(primitive.positions.data, positions, primitive.positions.size);
                }

                // Normals
                if (!neko_dyn_array_empty(normals)) {
                    primitive.normals.size = neko_dyn_array_size(normals) * sizeof(neko_vec3);
                    primitive.normals.data = neko_malloc(primitive.normals.size);
                    memcpy(primitive.normals.data, normals, primitive.normals.size);
                }

                // Tangents
                if (!neko_dyn_array_empty(tangents)) {
                    primitive.tangents.size = neko_dyn_array_size(tangents) * sizeof(neko_vec3);
                    primitive.tangents.data = neko_malloc(primitive.tangents.size);
                    memcpy(primitive.tangents.data, tangents, primitive.tangents.size);
                }

                // Texcoords
                for (uint32_t tci = 0; tci < NEKO_GFXT_TEX_COORD_MAX; ++tci) {
                    if (!neko_dyn_array_empty(uvs[tci])) {
                        primitive.tex_coords[tci].size = neko_dyn_array_size(uvs[tci]) * sizeof(neko_vec2);
                        primitive.tex_coords[tci].data = neko_malloc(primitive.tex_coords[tci].size);
                        memcpy(primitive.tex_coords[tci].data, uvs[tci], primitive.tex_coords[tci].size);
                    } else {
                        break;
                    }
                }

                // Colors
                for (uint32_t ci = 0; ci < NEKO_GFXT_COLOR_MAX; ++ci) {
                    if (!neko_dyn_array_empty(colors[ci])) {
                        primitive.colors[ci].size = neko_dyn_array_size(colors[ci]) * sizeof(neko_color_t);
                        primitive.colors[ci].data = neko_malloc(primitive.colors[ci].size);
                        memcpy(primitive.colors[ci].data, colors[ci], primitive.colors[ci].size);
                    } else {
                        break;
                    }
                }

                // Joints
                for (uint32_t ji = 0; ji < NEKO_GFXT_JOINT_MAX; ++ji) {
                    if (!neko_dyn_array_empty(joints[ji])) {
                        primitive.joints[ji].size = neko_dyn_array_size(joints[ji]) * sizeof(float);
                        primitive.joints[ji].data = neko_malloc(primitive.joints[ji].size);
                        memcpy(primitive.joints[ji].data, joints[ji], primitive.joints[ji].size);
                    } else {
                        break;
                    }
                }

                // Weights
                for (uint32_t wi = 0; wi < NEKO_GFXT_WEIGHT_MAX; ++wi) {
                    if (!neko_dyn_array_empty(weights[wi])) {
                        primitive.weights[wi].size = neko_dyn_array_size(weights[wi]) * sizeof(float);
                        primitive.weights[wi].data = neko_malloc(primitive.weights[wi].size);
                        memcpy(primitive.weights[wi].data, weights[wi], primitive.weights[wi].size);
                    } else {
                        break;
                    }
                }

                // Add primitive to mesh
                neko_dyn_array_push(mesh->primitives, primitive);
            }

            if (!printed) {
                printed = true;
                if (warnings[NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION]) {
                    neko_log_warning("Mesh attribute: POSITION not found. Resorting to default.");
                }

                if (warnings[NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD]) {
                    neko_log_warning("Mesh attribute: TEXCOORD not found. Resorting to default.");
                }

                if (warnings[NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR]) {
                    neko_log_warning("Mesh attribute: COLOR not found. Resorting to default.");
                }

                if (warnings[NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL]) {
                    neko_log_warning("Mesh attribute: NORMAL not found. Resorting to default.");
                }

                if (warnings[NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT]) {
                    neko_log_warning("Mesh attribute: WEIGHTS not found. Resorting to default.");
                }
            }
        }

        // Increment i if successful
        i++;
    }

    neko_println("Finished loading mesh.");

    // Free all data at the end
    cgltf_free(data);
    neko_dyn_array_free(positions);
    neko_dyn_array_free(normals);
    neko_dyn_array_free(tangents);
    for (uint32_t ci = 0; ci < NEKO_GFXT_COLOR_MAX; ++ci) neko_dyn_array_free(colors[ci]);
    for (uint32_t tci = 0; tci < NEKO_GFXT_TEX_COORD_MAX; ++tci) neko_dyn_array_free(uvs[tci]);
    for (uint32_t wi = 0; wi < NEKO_GFXT_WEIGHT_MAX; ++wi) neko_dyn_array_free(weights[wi]);
    for (uint32_t ji = 0; ji < NEKO_GFXT_JOINT_MAX; ++ji) neko_dyn_array_free(joints[ji]);
    neko_dyn_array_free(layouts);
    neko_byte_buffer_free(&v_data);
    neko_byte_buffer_free(&i_data);
    return true;
}

NEKO_API_DECL
neko_gfxt_mesh_t neko_gfxt_mesh_unit_quad_generate(neko_gfxt_mesh_import_options_t* options) {
    neko_gfxt_mesh_t mesh = neko_default_val();

    neko_vec3 v_pos[] = {
            neko_v3(-1.0f, -1.0f, 0.f),  // Top Left
            neko_v3(+1.0f, -1.0f, 0.f),  // Top Right
            neko_v3(-1.0f, +1.0f, 0.f),  // Bottom Left
            neko_v3(+1.0f, +1.0f, 0.f)   // Bottom Right
    };

    // Vertex data for quad
    neko_vec2 v_uvs[] = {
            neko_v2(0.0f, 0.0f),  // Top Left
            neko_v2(1.0f, 0.0f),  // Top Right
            neko_v2(0.0f, 1.0f),  // Bottom Left
            neko_v2(1.0f, 1.0f)   // Bottom Right
    };

    neko_vec3 v_norm[] = {neko_v3(0.f, 0.f, 1.f), neko_v3(0.f, 0.f, 1.f), neko_v3(0.f, 0.f, 1.f), neko_v3(0.f, 0.f, 1.f)};

    neko_vec3 v_tan[] = {neko_v3(1.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f)};

    neko_color_t v_color[] = {NEKO_COLOR_WHITE, NEKO_COLOR_WHITE, NEKO_COLOR_WHITE, NEKO_COLOR_WHITE};

    // Index data for quad
    uint16_t i_data[] = {
            0, 3, 2,  // First Triangle
            0, 1, 3   // Second Triangle
    };

    // Mesh data
    neko_gfxt_mesh_raw_data_t mesh_data = neko_default_val();

    // Primitive to upload
    neko_gfxt_mesh_vertex_data_t vert_data = neko_default_val();
    vert_data.positions.data = v_pos;
    vert_data.positions.size = sizeof(v_pos);
    vert_data.normals.data = v_norm;
    vert_data.normals.size = sizeof(v_norm);
    vert_data.tangents.data = v_tan;
    vert_data.tangents.size = sizeof(v_tan);
    vert_data.colors[0].data = v_color;
    vert_data.colors[0].size = sizeof(v_color);
    vert_data.tex_coords[0].data = v_uvs;
    vert_data.tex_coords[0].size = sizeof(v_uvs);
    vert_data.indices.data = i_data;
    vert_data.indices.size = sizeof(i_data);
    vert_data.count = 6;

    // Push into primitives
    neko_dyn_array_push(mesh_data.primitives, vert_data);

    // If no decl, then just use default layout
    /*
    neko_gfxt_mesh_import_options_t* moptions = options ? options : &def_options;
    uint32_t ct = moptions->size / sizeof(neko_asset_mesh_layout_t);
    */

    neko_gfxt_mesh_desc_t mdesc = neko_default_val();
    mdesc.meshes = &mesh_data;
    mdesc.size = 1 * sizeof(neko_gfxt_mesh_raw_data_t);
    mdesc.keep_data = true;

    mesh = neko_gfxt_mesh_create(&mdesc);
    mesh.desc = mdesc;

    // Free data
    neko_dyn_array_free(mesh_data.primitives);

    return mesh;
}

neko_handle(neko_graphics_texture_t) neko_gfxt_texture_generate_default() {
// Generate procedural texture data (checkered texture)
#define NEKO_GFXT_ROW_COL_CT 5
    neko_color_t c0 = NEKO_COLOR_WHITE;
    neko_color_t c1 = neko_color(20, 50, 150, 255);
    neko_color_t pixels[NEKO_GFXT_ROW_COL_CT * NEKO_GFXT_ROW_COL_CT] = neko_default_val();
    for (uint32_t r = 0; r < NEKO_GFXT_ROW_COL_CT; ++r) {
        for (uint32_t c = 0; c < NEKO_GFXT_ROW_COL_CT; ++c) {
            const bool re = (r % 2) == 0;
            const bool ce = (c % 2) == 0;
            uint32_t idx = r * NEKO_GFXT_ROW_COL_CT + c;
            pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
        }
    }

    neko_graphics_texture_desc_t desc = neko_default_val();
    desc.width = NEKO_GFXT_ROW_COL_CT;
    desc.height = NEKO_GFXT_ROW_COL_CT;
    desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    desc.wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
    desc.wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
    *desc.data = pixels;

    // Create dynamic texture
    return neko_graphics_texture_create(&desc);
}

//=== Resource Loading ===//

typedef struct tmp_buffer_t {
    char txt[1024];
} tmp_buffer_t;

typedef struct neko_shader_io_data_t {
    char type[64];
    char name[64];
} neko_shader_io_data_t;

typedef struct neko_pipeline_parse_data_t {
    neko_dyn_array(neko_shader_io_data_t) io_list[3];
    neko_dyn_array(neko_gfxt_mesh_layout_t) mesh_layout;
    neko_dyn_array(neko_graphics_vertex_attribute_type) vertex_layout;
    char* code[3];
} neko_ppd_t;

#define neko_parse_warning(TXT, ...)     \
    do {                               \
        neko_printf("WARNING::");        \
        neko_printf(TXT, ##__VA_ARGS__); \
        neko_println("");                \
    } while (0)

#define neko_parse_error(TXT, ASSERT, ...) \
    do {                                 \
        neko_printf("ERROR::");            \
        neko_printf(TXT, ##__VA_ARGS__);   \
        neko_println("");                  \
        if (ASSERT) neko_assert(false);    \
    } while (0)

#define neko_parse_block(NAME, ...)                                                                    \
    do {                                                                                             \
        neko_println("neko_pipeline_load_from_file::parsing::%s", #NAME);                                \
        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {                                  \
            neko_println("error::neko_pipeline_load_from_file::error parsing raster from .sf resource"); \
            neko_assert(false);                                                                        \
        }                                                                                            \
                                                                                                     \
        uint32_t bc = 1;                                                                             \
        while (neko_lexer_can_lex(lex) && bc) {                                                        \
            neko_token_t token = neko_lexer_next_token(lex);                                             \
            switch (token.type) {                                                                    \
                case NEKO_TOKEN_LBRACE: {                                                              \
                    bc++;                                                                            \
                } break;                                                                             \
                case NEKO_TOKEN_RBRACE: {                                                              \
                    bc--;                                                                            \
                } break;                                                                             \
                                                                                                     \
                case NEKO_TOKEN_IDENTIFIER: {                                                          \
                    __VA_ARGS__                                                                      \
                }                                                                                    \
            }                                                                                        \
        }                                                                                            \
    } while (0)

const char* neko_get_vertex_attribute_string(neko_graphics_vertex_attribute_type type) {
    switch (type) {
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:
            return "float";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:
            return "vec2";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:
            return "vec3";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:
            return "vec4";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT:
            return "int";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:
            return "vec2";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:
            return "vec3";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:
            return "vec4";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:
            return "float";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:
            return "vec2";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:
            return "vec3";
            break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:
            return "vec4";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}

neko_graphics_vertex_attribute_type neko_get_vertex_attribute_from_token(const neko_token_t* t) {
    if (neko_token_compare_text(t, "float"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT;
    else if (neko_token_compare_text(t, "float2"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;
    else if (neko_token_compare_text(t, "float3"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3;
    else if (neko_token_compare_text(t, "float4"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4;
    else if (neko_token_compare_text(t, "uint4"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4;
    else if (neko_token_compare_text(t, "uint3"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3;
    else if (neko_token_compare_text(t, "uint2"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2;
    else if (neko_token_compare_text(t, "uint"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT;
    else if (neko_token_compare_text(t, "byte4"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4;
    else if (neko_token_compare_text(t, "byte3"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3;
    else if (neko_token_compare_text(t, "byte2"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2;
    else if (neko_token_compare_text(t, "byte"))
        return NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE;
    return (neko_graphics_vertex_attribute_type)0x00;
}

neko_graphics_uniform_type neko_uniform_type_from_token(const neko_token_t* t) {
    if (neko_token_compare_text(t, "float"))
        return NEKO_GRAPHICS_UNIFORM_FLOAT;
    else if (neko_token_compare_text(t, "int"))
        return NEKO_GRAPHICS_UNIFORM_INT;
    else if (neko_token_compare_text(t, "vec2"))
        return NEKO_GRAPHICS_UNIFORM_VEC2;
    else if (neko_token_compare_text(t, "vec3"))
        return NEKO_GRAPHICS_UNIFORM_VEC3;
    else if (neko_token_compare_text(t, "vec4"))
        return NEKO_GRAPHICS_UNIFORM_VEC4;
    else if (neko_token_compare_text(t, "mat4"))
        return NEKO_GRAPHICS_UNIFORM_MAT4;
    else if (neko_token_compare_text(t, "sampler2D"))
        return NEKO_GRAPHICS_UNIFORM_SAMPLER2D;
    else if (neko_token_compare_text(t, "usampler2D"))
        return NEKO_GRAPHICS_UNIFORM_USAMPLER2D;
    else if (neko_token_compare_text(t, "samplerCube"))
        return NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE;
    else if (neko_token_compare_text(t, "img2D_rgba32f"))
        return NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F;
    return (neko_graphics_uniform_type)0x00;
}

const char* neko_uniform_string_from_type(neko_graphics_uniform_type type) {
    switch (type) {
        case NEKO_GRAPHICS_UNIFORM_FLOAT:
            return "float";
            break;
        case NEKO_GRAPHICS_UNIFORM_INT:
            return "int";
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC2:
            return "vec2";
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC3:
            return "vec3";
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC4:
            return "vec4";
            break;
        case NEKO_GRAPHICS_UNIFORM_MAT4:
            return "mat4";
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
            return "sampler2D";
            break;
        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
            return "usampler2D";
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE:
            return "samplerCube";
            break;
        case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:
            return "image2D";
            break;
        default:
            return "UNKNOWN";
            break;
    }
    return (char*)0x00;
}

// Make this an extern function that can be bubbled up to the app
bool neko_parse_uniform_special_keyword(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_graphics_shader_stage_type stage, neko_gfxt_uniform_desc_t* uniform) {
    neko_token_t token = lex->current_token;

    // Determine if uniform is one of special key defines
    if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX, sizeof(NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX, sizeof(NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_MODEL_MATRIX")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_MODEL_MATRIX, sizeof(NEKO_GFXT_UNIFORM_MODEL_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX, sizeof(NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_PROJECTION_MATRIX")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_PROJECTION_MATRIX, sizeof(NEKO_GFXT_UNIFORM_PROJECTION_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_VIEW_MATRIX")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_VIEW_MATRIX, sizeof(NEKO_GFXT_UNIFORM_VIEW_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_TIME")) {
        uniform->type = NEKO_GRAPHICS_UNIFORM_FLOAT;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_TIME, sizeof(NEKO_GFXT_UNIFORM_TIME));
        return true;
    }

    return false;
}

bool neko_parse_uniforms(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_graphics_shader_stage_type stage) {
    uint32_t image_binding = 0;

    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Unable to parsing uniforms from .sf resource");
        return false;
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                neko_gfxt_uniform_desc_t uniform = {0};
                uniform.stage = stage;

                bool special = neko_parse_uniform_special_keyword(lex, desc, ppd, stage, &uniform);

                // Determine if uniform is one of special key defines
                if (!special) {
                    uniform.type = neko_uniform_type_from_token(&token);
                    switch (uniform.type) {
                        default:
                            break;

                        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
                        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
                        case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
                            uniform.binding = image_binding++;
                        } break;
                    }

                    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                        neko_log_warning("Unidentified token (Expected identifier)");
                        neko_token_debug_print(&lex->current_token);
                        return false;
                    }
                    token = lex->current_token;

                    memcpy(uniform.name, token.text, token.len);
                }

                // Add uniform to ublock descriptor
                neko_dyn_array_push(desc->ublock_desc.layout, uniform);

            } break;
        }
    }
    return true;
}

bool neko_parse_io(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_graphics_shader_stage_type type) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Expected opening left brace. Unable to parse io from .sf resource");
        return false;
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;
            case NEKO_TOKEN_IDENTIFIER: {
                neko_shader_io_data_t io = {0};
                memcpy(io.type, token.text, token.len);

                switch (type) {
                    case NEKO_GRAPHICS_SHADER_STAGE_VERTEX: {
                        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                            neko_log_warning("IO expected identifier name after type, shader stage vertex.");
                            neko_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        neko_dyn_array_push(ppd->io_list[0], io);
                    } break;

                    case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT: {
                        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                            neko_log_warning("IO expected identifier name after type, shader stage fragment.");
                            neko_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        neko_dyn_array_push(ppd->io_list[1], io);
                    } break;

                    case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE: {
                        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                            neko_log_warning("IO expected number after type, shader stage compute.");
                            neko_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        neko_dyn_array_push(ppd->io_list[2], io);
                    } break;
                }
            } break;
        }
    }
    return true;
}

bool neko_parse_code(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_graphics_shader_stage_type stage) {
    if (!neko_lexer_require_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Expected opening left brace");
        return false;
    }

    // Something is broken up here...
    uint32_t bc = 1;
    neko_token_t cur = neko_lexer_peek(lex);
    neko_token_t token = lex->current_token;
    while (neko_lexer_can_lex(lex) && bc) {
        token = lex->next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;
        }
    }

    // This is most likely incorrect...
    const size_t sz = (size_t)(token.text - cur.text);
    char* code = (char*)neko_malloc(sz);
    memset(code, 0, sz);
    memcpy(code, cur.text, sz - 1);

    // Need to parse through code and replace keywords with appropriate mappings
    neko_lexer_t clex = neko_lexer_c_ctor(code);
    while (clex.can_lex(&clex)) {
        neko_token_t tkn = clex.next_token(&clex);
        switch (tkn.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_MODEL_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_MODEL_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_VIEW_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_VIEW_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_PROJECTION_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_PROJECTION_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_TIME")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_TIME, (char)32);
                }
            };
        }
    }

    // neko_println("code: %s", code);

    switch (stage) {
        case NEKO_GRAPHICS_SHADER_STAGE_VERTEX:
            ppd->code[0] = code;
            break;
        case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT:
            ppd->code[1] = code;
            break;
        case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE:
            ppd->code[2] = code;
            break;
    }

    return true;
}

neko_gfxt_mesh_attribute_type neko_mesh_attribute_type_from_token(const neko_token_t* token) {
    if (neko_token_compare_text(token, "POSITION"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION;
    else if (neko_token_compare_text(token, "NORMAL"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL;
    else if (neko_token_compare_text(token, "COLOR"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR;
    else if (neko_token_compare_text(token, "TANGENT"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT;
    else if (neko_token_compare_text(token, "TEXCOORD0"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD1"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD2"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD3"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD4"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD5"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD6"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD7"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD8"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD9"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD10"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD11"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD12"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "UINT"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT;

    // Default
    return (neko_gfxt_mesh_attribute_type)0x00;
}

bool neko_parse_vertex_mesh_attributes(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_assert(false);
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        // neko_token_debug_print(&token);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                // Get attribute name
                if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                    neko_assert(false);
                }

                neko_token_t token_name = lex->current_token;
                // neko_token_debug_print(&token_name);

#define PUSH_ATTR(MESH_ATTR, VERT_ATTR)                              \
    do {                                                             \
        neko_gfxt_mesh_layout_t layout = neko_default_val();             \
        layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_##MESH_ATTR;      \
        neko_dyn_array_push(ppd->mesh_layout, layout);                 \
        neko_graphics_vertex_attribute_desc_t attr = neko_default_val(); \
        memcpy(attr.name, token_name.text, token_name.len);          \
        attr.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_##VERT_ATTR;      \
        neko_dyn_array_push(desc->pip_desc.layout.attrs, attr);        \
        /*neko_println("%s: %s", #MESH_ATTR, #VERT_ATTR);*/            \
    } while (0)

                if (neko_token_compare_text(&token, "POSITION"))
                    PUSH_ATTR(POSITION, FLOAT3);
                else if (neko_token_compare_text(&token, "NORMAL"))
                    PUSH_ATTR(NORMAL, FLOAT3);
                else if (neko_token_compare_text(&token, "COLOR"))
                    PUSH_ATTR(COLOR, BYTE4);
                else if (neko_token_compare_text(&token, "TANGENT"))
                    PUSH_ATTR(TANGENT, FLOAT3);
                else if (neko_token_compare_text(&token, "TEXCOORD"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD0"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD1"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD2"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD3"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD4"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD5"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD6"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD8"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD9"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD10"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD11"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD12"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "FLOAT"))
                    PUSH_ATTR(WEIGHT, FLOAT4);
                else if (neko_token_compare_text(&token, "FLOAT2"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "FLOAT3"))
                    PUSH_ATTR(POSITION, FLOAT3);
                else if (neko_token_compare_text(&token, "UINT"))
                    PUSH_ATTR(UINT, UINT);
                // else if (neko_token_compare_text(&token, "FLOAT4"))     PUSH_ATTR(TANGENT, FLOAT4);
                else {
                    neko_log_warning("Unidentified vertex attribute: %.*s: %.*s", token.len, token.text, token_name.len, token_name.text);
                    return false;
                }
            }
        }
    }
    return true;
}

bool neko_parse_vertex_attributes(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd) { return neko_parse_vertex_mesh_attributes(lex, desc, ppd); }

bool neko_parse_shader_stage(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_graphics_shader_stage_type stage) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_println("error::neko_pipeline_load_from_file::error parsing raster from .sf resource");
        neko_assert(false);
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                if (stage == NEKO_GRAPHICS_SHADER_STAGE_VERTEX && neko_token_compare_text(&token, "attributes")) {
                    neko_println("parsing attributes...");
                    if (!neko_parse_vertex_attributes(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse vertex attributes.");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "uniforms")) {
                    neko_println("parsing uniforms...");
                    if (!neko_parse_uniforms(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'uniforms' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "out")) {
                    neko_println("parsing out...");
                    if (!neko_parse_io(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'out' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "in")) {
                    neko_println("parsing in...");
                    if (!neko_parse_io(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'in' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "code")) {
                    neko_println("parsing code...");
                    if (!neko_parse_code(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'code' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }
            } break;
        }
    }
    return true;
}

bool neko_parse_compute_shader_stage(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::COMPUTE_SHADER_STAGE, {
        if (neko_token_compare_text(&token, "uniforms")) {
            if (!neko_parse_uniforms(lex, desc, ppd, NEKO_GRAPHICS_SHADER_STAGE_COMPUTE)) {
                neko_log_warning("Unable to parse 'uniforms' for compute shader");
                return false;
            }
        }

        else if (neko_token_compare_text(&token, "in")) {
            if (!neko_parse_io(lex, desc, ppd, NEKO_GRAPHICS_SHADER_STAGE_COMPUTE)) {
                neko_log_warning("Unable to parse 'in' for compute shader");
                return false;
            }
        }

        else if (neko_token_compare_text(&token, "code")) {
            if (!neko_parse_code(lex, desc, ppd, NEKO_GRAPHICS_SHADER_STAGE_COMPUTE)) {
                neko_log_warning("Unable to parse 'code' for compute shader");
                return false;
            }
        }
    });
    return true;
}

bool neko_parse_shader(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Unable to parse shader from .sf resource. Expected opening left brace.");
        return false;
    }

    // Braces
    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = lex->next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                // Vertex shader
                if (neko_token_compare_text(&token, "vertex")) {
                    neko_println("parsing vertex shader");
                    if (!neko_parse_shader_stage(lex, desc, ppd, NEKO_GRAPHICS_SHADER_STAGE_VERTEX)) {
                        neko_log_warning("Unable to parse shader stage: Vertex");
                        return false;
                    }
                }

                // Fragment shader
                else if (neko_token_compare_text(&token, "fragment")) {
                    neko_println("parsing fragment shader");
                    if (!neko_parse_shader_stage(lex, desc, ppd, NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT)) {
                        neko_log_warning("Unable to parse shader stage: Fragment");
                        return false;
                    }
                }

                // Compute shader
                else if (neko_token_compare_text(&token, "compute")) {
                    neko_println("parsing compute shader");
                    if (!neko_parse_shader_stage(lex, desc, ppd, NEKO_GRAPHICS_SHADER_STAGE_COMPUTE)) {
                        neko_log_warning("Unable to parse shader stage: Compute");
                        return false;
                    }
                }

            } break;
        }
    }
    return true;
}

bool neko_parse_depth(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::DEPTH, {
        // Depth function
        if (neko_token_compare_text(&token, "func")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                token = lex->current_token;
                neko_log_warning("Depth func type not found after function decl: %.*s", token.len, token.text);
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "LESS"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_LESS;
            else if (neko_token_compare_text(&token, "EQUAL"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_EQUAL;
            else if (neko_token_compare_text(&token, "LEQUAL"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_LEQUAL;
            else if (neko_token_compare_text(&token, "GREATER"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_GREATER;
            else if (neko_token_compare_text(&token, "NOTEQUAL"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_NOTEQUAL;
            else if (neko_token_compare_text(&token, "GEQUAL"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_GEQUAL;
            else if (neko_token_compare_text(&token, "ALWAYS"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_ALWAYS;
            else if (neko_token_compare_text(&token, "NEVER"))
                pdesc->pip_desc.depth.func = NEKO_GRAPHICS_DEPTH_FUNC_NEVER;
            else {
                token = lex->current_token;
                neko_log_warning("Func type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
        if (neko_token_compare_text(&token, "mask")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                token = lex->current_token;
                neko_log_warning("Depth mask type not found after function decl: %.*s", token.len, token.text);
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ENABLED"))
                pdesc->pip_desc.depth.mask = NEKO_GRAPHICS_DEPTH_MASK_ENABLED;
            else if (neko_token_compare_text(&token, "TRUE"))
                pdesc->pip_desc.depth.mask = NEKO_GRAPHICS_DEPTH_MASK_ENABLED;
            else if (neko_token_compare_text(&token, "true"))
                pdesc->pip_desc.depth.mask = NEKO_GRAPHICS_DEPTH_MASK_ENABLED;
            else if (neko_token_compare_text(&token, "DISABLED"))
                pdesc->pip_desc.depth.mask = NEKO_GRAPHICS_DEPTH_MASK_DISABLED;
            else if (neko_token_compare_text(&token, "FALSE"))
                pdesc->pip_desc.depth.mask = NEKO_GRAPHICS_DEPTH_MASK_DISABLED;
            else if (neko_token_compare_text(&token, "false"))
                pdesc->pip_desc.depth.mask = NEKO_GRAPHICS_DEPTH_MASK_DISABLED;
            else {
                token = lex->current_token;
                neko_log_warning("Mask type %.*s not valid.", token.len, token.text);
                return false;
            }
            neko_println("MASK: %zu", (uint32_t)pdesc->pip_desc.depth.mask);
        }
    });
    return true;
}

bool neko_parse_blend(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::BLEND, {
        // Blend function
        if (neko_token_compare_text(&token, "func")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Blend func type not found after function decl.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ADD"))
                pdesc->pip_desc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD;
            else if (neko_token_compare_text(&token, "SUBTRACT"))
                pdesc->pip_desc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_SUBTRACT;
            else if (neko_token_compare_text(&token, "REVERSE_SUBTRACT"))
                pdesc->pip_desc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT;
            else if (neko_token_compare_text(&token, "MIN"))
                pdesc->pip_desc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_MIN;
            else if (neko_token_compare_text(&token, "MAX"))
                pdesc->pip_desc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_MAX;
            else {
                neko_log_warning("Blend func type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Source blend
        else if (neko_token_compare_text(&token, "src")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Blend src type not found after decl.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ZERO"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ZERO;
            else if (neko_token_compare_text(&token, "ONE"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE;
            else if (neko_token_compare_text(&token, "SRC_COLOR"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_SRC_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_COLOR"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR;
            else if (neko_token_compare_text(&token, "DST_COLOR"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_DST_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_COLOR"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR;
            else if (neko_token_compare_text(&token, "SRC_ALPHA"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_ALPHA"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "DST_ALPHA"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_DST_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_ALPHA"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_COLOR"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_CONSTANT_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_COLOR"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.src = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else {
                neko_log_warning("Blend src type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Dest blend
        else if (neko_token_compare_text(&token, "dst")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Blend dst type not found after decl.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ZERO"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ZERO;
            else if (neko_token_compare_text(&token, "ONE"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE;
            else if (neko_token_compare_text(&token, "SRC_COLOR"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_SRC_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_COLOR"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR;
            else if (neko_token_compare_text(&token, "DST_COLOR"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_DST_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_COLOR"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR;
            else if (neko_token_compare_text(&token, "SRC_ALPHA"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_ALPHA"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "DST_ALPHA"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_DST_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_ALPHA"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_COLOR"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_CONSTANT_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_COLOR"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else {
                neko_log_warning("Blend dst type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
    });

    return true;
}

bool neko_parse_stencil(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::STENCIL, {
        // Function
        if (neko_token_compare_text(&token, "func")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil func type not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "LESS"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_LESS;
                else if (neko_token_compare_text(&token, "EQUAL"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_EQUAL;
                else if (neko_token_compare_text(&token, "LEQUAL"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_LEQUAL;
                else if (neko_token_compare_text(&token, "GREATER"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_GREATER;
                else if (neko_token_compare_text(&token, "NOTEQUAL"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_NOTEQUAL;
                else if (neko_token_compare_text(&token, "GEQUAL"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_GEQUAL;
                else if (neko_token_compare_text(&token, "ALWAYS"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_ALWAYS;
                else if (neko_token_compare_text(&token, "NEVER"))
                    pdesc->pip_desc.stencil.func = NEKO_GRAPHICS_STENCIL_FUNC_NEVER;
                else {
                    neko_log_warning("Stencil func type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }

        }

        // Reference value
        else if (neko_token_compare_text(&token, "ref")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                neko_log_warning("Stencil reference value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;
                neko_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.ref = atoi(TMP);
            }
        }

        // Component mask
        else if (neko_token_compare_text(&token, "comp_mask")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                neko_log_warning("Stencil component mask value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;
                neko_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.comp_mask = atoi(TMP);
            }
        }

        // Write mask
        else if (neko_token_compare_text(&token, "write_mask")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                neko_log_warning("Stencil write mask value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;
                neko_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.write_mask = atoi(TMP);
            }
        }

        // Stencil test failure
        else if (neko_token_compare_text(&token, "sfail")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil sfail value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "KEEP"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_KEEP;
                else if (neko_token_compare_text(&token, "ZERO"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_ZERO;
                else if (neko_token_compare_text(&token, "REPLACE"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_REPLACE;
                else if (neko_token_compare_text(&token, "INCR"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_INCR;
                else if (neko_token_compare_text(&token, "INCR_WRAP"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_INCR_WRAP;
                else if (neko_token_compare_text(&token, "DECR"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_DECR;
                else if (neko_token_compare_text(&token, "DECR_WRAP"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_DECR_WRAP;
                else if (neko_token_compare_text(&token, "INVERT"))
                    pdesc->pip_desc.stencil.sfail = NEKO_GRAPHICS_STENCIL_OP_INVERT;
                else {
                    neko_log_warning("Stencil sfail type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }
        }

        // Stencil test pass, Depth fail
        else if (neko_token_compare_text(&token, "dpfail")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil dpfail value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "KEEP"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_KEEP;
                else if (neko_token_compare_text(&token, "ZERO"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_ZERO;
                else if (neko_token_compare_text(&token, "REPLACE"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_REPLACE;
                else if (neko_token_compare_text(&token, "INCR"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_INCR;
                else if (neko_token_compare_text(&token, "INCR_WRAP"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_INCR_WRAP;
                else if (neko_token_compare_text(&token, "DECR"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_DECR;
                else if (neko_token_compare_text(&token, "DECR_WRAP"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_DECR_WRAP;
                else if (neko_token_compare_text(&token, "INVERT"))
                    pdesc->pip_desc.stencil.dpfail = NEKO_GRAPHICS_STENCIL_OP_INVERT;
                else {
                    neko_log_warning("Stencil dpfail type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }
        }

        // Stencil test pass, Depth pass
        else if (neko_token_compare_text(&token, "dppass")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil dppass value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "KEEP"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_KEEP;
                else if (neko_token_compare_text(&token, "ZERO"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_ZERO;
                else if (neko_token_compare_text(&token, "REPLACE"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_REPLACE;
                else if (neko_token_compare_text(&token, "INCR"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_INCR;
                else if (neko_token_compare_text(&token, "INCR_WRAP"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_INCR_WRAP;
                else if (neko_token_compare_text(&token, "DECR"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_DECR;
                else if (neko_token_compare_text(&token, "DECR_WRAP"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_DECR_WRAP;
                else if (neko_token_compare_text(&token, "INVERT"))
                    pdesc->pip_desc.stencil.dppass = NEKO_GRAPHICS_STENCIL_OP_INVERT;
                else {
                    neko_log_warning("Stencil dppass type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }
        }
    });
    return true;
}

bool neko_parse_raster(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::RASTER, {
        // Index Buffer Element Size
        if (neko_token_compare_text(&token, "index_buffer_element_size")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster index buffer element size not found.", token.len, token.text);
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "UINT32") || neko_token_compare_text(&token, "uint32_t") || neko_token_compare_text(&token, "u32")) {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint32_t);
            }

            else if (neko_token_compare_text(&token, "UINT16") || neko_token_compare_text(&token, "uint16_t") || neko_token_compare_text(&token, "u16")) {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint16_t);
            }

            // Default
            else {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint32_t);
            }
        }

        // Face culling
        if (neko_token_compare_text(&token, "face_culling")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster face culling type not found.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "FRONT"))
                pdesc->pip_desc.raster.face_culling = NEKO_GRAPHICS_FACE_CULLING_FRONT;
            else if (neko_token_compare_text(&token, "BACK"))
                pdesc->pip_desc.raster.face_culling = NEKO_GRAPHICS_FACE_CULLING_BACK;
            else if (neko_token_compare_text(&token, "FRONT_AND_BACK"))
                pdesc->pip_desc.raster.face_culling = NEKO_GRAPHICS_FACE_CULLING_FRONT_AND_BACK;
            else {
                neko_log_warning("Raster face culling type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Winding order
        if (neko_token_compare_text(&token, "winding_order")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster winding order type not found.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "CW"))
                pdesc->pip_desc.raster.winding_order = NEKO_GRAPHICS_WINDING_ORDER_CW;
            else if (neko_token_compare_text(&token, "CCW"))
                pdesc->pip_desc.raster.winding_order = NEKO_GRAPHICS_WINDING_ORDER_CCW;
            else {
                neko_log_warning("Raster winding order type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Primtive
        if (neko_token_compare_text(&token, "primitive")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster primitive type not found.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "LINES"))
                pdesc->pip_desc.raster.primitive = NEKO_GRAPHICS_PRIMITIVE_LINES;
            else if (neko_token_compare_text(&token, "TRIANGLES"))
                pdesc->pip_desc.raster.primitive = NEKO_GRAPHICS_PRIMITIVE_TRIANGLES;
            else if (neko_token_compare_text(&token, "QUADS"))
                pdesc->pip_desc.raster.primitive = NEKO_GRAPHICS_PRIMITIVE_QUADS;
            else {
                neko_log_warning("Raster primitive type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
    });
    return true;
}

bool neko_parse_pipeline(neko_lexer_t* lex, neko_gfxt_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    // Get next identifier
    while (lex->can_lex(lex)) {
        neko_token_t token = lex->next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "shader")) {
                    neko_println("parsing shader");
                    if (!neko_parse_shader(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse shader descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "raster")) {
                    if (!neko_parse_raster(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse raster descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "depth")) {
                    if (!neko_parse_depth(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse depth descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "stencil")) {
                    if (!neko_parse_stencil(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse stencil descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "blend")) {
                    if (!neko_parse_blend(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse blend descriptor");
                        return false;
                    }
                }

            } break;
        }
    }
    return true;
}

char* neko_pipeline_generate_shader_code(neko_gfxt_pipeline_desc_t* pdesc, neko_ppd_t* ppd, neko_graphics_shader_stage_type stage) {
    neko_println("GENERATING CODE...");

    // Get major/minor version of shader
    neko_graphics_info_t* ginfo = neko_graphics_info();
    neko_snprintfc(MAJMINSTR, 128, "#version %zu%zu0\n", ginfo->major_version, ginfo->minor_version);

// Shaders
#ifdef NEKO_PLATFORM_WEB
#define _NEKO_VERSION_STR "#version 300 es\n"
#else
#define _NEKO_VERSION_STR MAJMINSTR
#endif

    // Source code
    char* src = NULL;
    uint32_t sidx = 0;

    // Set sidx
    switch (stage) {
        case NEKO_GRAPHICS_SHADER_STAGE_VERTEX:
            sidx = 0;
            break;
        case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT:
            sidx = 1;
            break;
        case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE:
            sidx = 2;
            break;
    }

    // Early out for now...
    if (!ppd->code[sidx]) {
        return src;
    }

    // Shader header
    neko_snprintfc(shader_header, 512, "%s precision mediump float;\n", stage == NEKO_GRAPHICS_SHADER_STAGE_COMPUTE ? "#version 430\n" : _NEKO_VERSION_STR);

    // Generate shader code
    if (ppd->code[sidx]) {
        const size_t header_sz = (size_t)neko_string_length(shader_header);
        size_t total_sz = neko_string_length(ppd->code[sidx]) + header_sz + 2048;
        src = (char*)neko_malloc(total_sz);
        memset(src, 0, total_sz);
        strncat(src, shader_header, header_sz);

        // Attributes
        if (stage == NEKO_GRAPHICS_SHADER_STAGE_VERTEX) {
            for (uint32_t i = 0; i < neko_dyn_array_size(pdesc->pip_desc.layout.attrs); ++i) {
                const char* aname = pdesc->pip_desc.layout.attrs[i].name;
                const char* atype = neko_get_vertex_attribute_string(pdesc->pip_desc.layout.attrs[i].format);

                neko_snprintfc(ATTR, 64, "layout(location = %zu) in %s %s;\n", i, atype, aname);
                const size_t sz = neko_string_length(ATTR);
                strncat(src, ATTR, sz);
            }
        }

        // Compute shader image buffer binding
        uint32_t img_binding = 0;

        // Uniforms
        for (uint32_t i = 0; i < neko_dyn_array_size(pdesc->ublock_desc.layout); ++i) {
            neko_gfxt_uniform_desc_t* udesc = &pdesc->ublock_desc.layout[i];

            if (udesc->stage != stage) continue;

            switch (stage) {
                case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE: {
                    // Need to go from uniform type to string
                    const char* utype = neko_uniform_string_from_type(udesc->type);
                    const char* uname = udesc->name;

                    switch (udesc->type) {
                        default: {
                            neko_snprintfc(TMP, 64, "uniform %s %s;\n", utype, uname);
                            const size_t sz = neko_string_length(TMP);
                            strncat(src, TMP, sz);
                        } break;

                        case NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
                            neko_snprintfc(TMP, 64, "layout (rgba32f, binding = %zu) uniform image2D %s;\n", img_binding++, uname);
                            const size_t sz = neko_string_length(TMP);
                            strncat(src, TMP, sz);
                        } break;
                    }
                } break;

                default: {
                    // Need to go from uniform type to string
                    const char* utype = neko_uniform_string_from_type(udesc->type);
                    const char* uname = udesc->name;
                    neko_snprintfc(TMP, 64, "uniform %s %s;\n", utype, uname);
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                } break;
            }
        }

        // Out
        switch (stage) {
            case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT:
            case NEKO_GRAPHICS_SHADER_STAGE_VERTEX: {
                for (uint32_t i = 0; i < neko_dyn_array_size(ppd->io_list[sidx]); ++i) {
                    neko_shader_io_data_t* out = &ppd->io_list[sidx][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    neko_transient_buffer(TMP, 64);
                    if (stage == NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT) {
                        neko_snprintf(TMP, 64, "layout(location = %zu) out %s %s;\n", i, otype, oname);
                    } else {
                        neko_snprintf(TMP, 64, "out %s %s;\n", otype, oname);
                    }
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                }
            } break;

            default:
                break;
        }

        // In
        switch (stage) {
            case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT: {
                for (uint32_t i = 0; i < neko_dyn_array_size(ppd->io_list[0]); ++i) {
                    neko_shader_io_data_t* out = &ppd->io_list[0][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    neko_snprintfc(TMP, 64, "in %s %s;\n", otype, oname);
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                }
            } break;

            case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE: {
                neko_snprintfc(TMP, 64, "layout(");
                strncat(src, "layout(", 7);

                for (uint32_t i = 0; i < neko_dyn_array_size(ppd->io_list[2]); ++i) {
                    neko_shader_io_data_t* out = &ppd->io_list[2][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    neko_snprintfc(TMP, 64, "%s = %s%s", otype, oname, i == neko_dyn_array_size(ppd->io_list[2]) - 1 ? "" : ", ");
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                }

                strncat(src, ") in;\n", 7);
            } break;

            default:
                break;
        }

        // Code
        {
            const size_t sz = neko_string_length(ppd->code[sidx]);
            strncat(src, ppd->code[sidx], sz);
        }
    }

    return src;
}

NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_file(const char* path) {
    // Load file, generate lexer off of file data, parse contents for pipeline information
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(path, "rb", &len);
    neko_assert(file_data);
    neko_log_success("Parsing pipeline: %s", path);
    neko_gfxt_pipeline_t pip = neko_gfxt_pipeline_load_from_memory(file_data, len);
    neko_free(file_data);
    return pip;
}

NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_memory(const char* file_data, size_t sz) {
    // Cast to pip
    neko_gfxt_pipeline_t pip = neko_default_val();

    neko_ppd_t ppd = neko_default_val();
    neko_gfxt_pipeline_desc_t pdesc = neko_default_val();
    pdesc.pip_desc.raster.index_buffer_element_size = sizeof(uint32_t);

    neko_lexer_t lex = neko_lexer_c_ctor(file_data);
    while (lex.can_lex(&lex)) {
        neko_token_t token = lex.next_token(&lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "pipeline")) {
                    if (!neko_parse_pipeline(&lex, &pdesc, &ppd)) {
                        neko_log_warning("Unable to parse pipeline");
                        return pip;
                    }
                }
            } break;
        }
    }

    // Generate vertex shader code
    char* v_src = neko_pipeline_generate_shader_code(&pdesc, &ppd, NEKO_GRAPHICS_SHADER_STAGE_VERTEX);
    // neko_println("%s", v_src);

    // Generate fragment shader code
    char* f_src = neko_pipeline_generate_shader_code(&pdesc, &ppd, NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT);
    // neko_println("%s", f_src);

    // Generate compute shader code (need to check for this first)
    char* c_src = neko_pipeline_generate_shader_code(&pdesc, &ppd, NEKO_GRAPHICS_SHADER_STAGE_COMPUTE);
    // neko_println("%s", c_src);

    // Construct compute shader
    if (c_src) {
        neko_graphics_shader_desc_t sdesc = neko_default_val();
        neko_graphics_shader_source_desc_t source_desc[1] = neko_default_val();
        source_desc[0].type = NEKO_GRAPHICS_SHADER_STAGE_COMPUTE;
        source_desc[0].source = c_src;
        sdesc.sources = source_desc;
        sdesc.size = 1 * sizeof(neko_graphics_shader_source_desc_t);

        pdesc.pip_desc.compute.shader = neko_graphics_shader_create(&sdesc);
    }
    // Construct raster shader
    else {
        neko_graphics_shader_desc_t sdesc = neko_default_val();
        neko_graphics_shader_source_desc_t source_desc[2] = neko_default_val();
        source_desc[0].type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX;
        source_desc[0].source = v_src;
        source_desc[1].type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT;
        source_desc[1].source = f_src;
        sdesc.sources = source_desc;
        sdesc.size = 2 * sizeof(neko_graphics_shader_source_desc_t);

        pdesc.pip_desc.raster.shader = neko_graphics_shader_create(&sdesc);
    }

    // Set up layout
    pdesc.pip_desc.layout.size = neko_dyn_array_size(pdesc.pip_desc.layout.attrs) * sizeof(neko_graphics_vertex_attribute_desc_t);

    // Set up ublock
    pdesc.ublock_desc.size = neko_dyn_array_size(pdesc.ublock_desc.layout) * sizeof(neko_gfxt_uniform_desc_t);

    // Create pipeline
    pip = neko_gfxt_pipeline_create(&pdesc);

    // Create mesh layout
    if (ppd.mesh_layout) {
        for (uint32_t i = 0; i < neko_dyn_array_size(ppd.mesh_layout); ++i) {
            neko_dyn_array_push(pip.mesh_layout, ppd.mesh_layout[i]);
        }
    }

    // Free all malloc'd data
    if (v_src) neko_free(v_src);
    if (f_src) neko_free(f_src);
    if (c_src) neko_free(c_src);
    neko_dyn_array_free(pdesc.ublock_desc.layout);
    neko_dyn_array_free(pdesc.pip_desc.layout.attrs);
    neko_dyn_array_free(ppd.mesh_layout);
    neko_dyn_array_free(ppd.vertex_layout);

    for (uint32_t i = 0; i < 3; ++i) {
        if (ppd.code[i]) neko_free(ppd.code[i]);
        neko_dyn_array_free(ppd.io_list[i]);
    }

    return pip;
}

NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_load_from_file(const char* path, neko_graphics_texture_desc_t* desc, bool flip, bool keep_data) {
    neko_asset_texture_t tex = neko_default_val();
    neko_asset_texture_load_from_file(path, &tex, desc, flip, keep_data);
    if (desc) {
        *desc = tex.desc;
    }
    return tex.hndl;
}

NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_load_from_memory(const char* data, size_t sz, neko_graphics_texture_desc_t* desc, bool flip, bool keep_data) {
    neko_asset_texture_t tex = neko_default_val();
    neko_asset_texture_load_from_memory(data, sz, &tex, desc, flip, keep_data);
    if (desc) {
        *desc = tex.desc;
    }
    return tex.hndl;
}

#endif  // NEKO_GFXT_IMPL
#endif  // NEKO_GFXT_H

/*
 */
