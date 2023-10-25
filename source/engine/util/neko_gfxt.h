
#ifndef NEKO_GFXT_H
#define NEKO_GFXT_H

#include "engine/neko_engine.h"
#include "engine/util/neko_asset.h"

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
    void* user_data;               // Optional user data for function
} neko_gfxt_raw_data_func_desc_t;

//=== Uniforms/Uniform blocks ===//
typedef struct neko_gfxt_uniform_desc_t {
    char name[64];                          // Name of uniform (for binding to shader)
    neko_graphics_uniform_type type;        // Type of uniform: NEKO_GRAPHICS_UNIFORM_VEC2, NEKO_GRAPHICS_UNIFORM_VEC3, etc.
    uint32_t binding;                       // Binding for this uniform in shader
    neko_graphics_shader_stage_type stage;  // Shader stage for this uniform
    neko_graphics_access_type access_type;  // Access type for this uniform (compute only)
} neko_gfxt_uniform_desc_t;

typedef struct neko_gfxt_uniform_t {
    neko_handle(neko_graphics_uniform_t) hndl;  // Graphics handle resource for actual uniform
    uint32_t offset;                            // Individual offset for this uniform in material byte buffer data
    uint32_t binding;                           // Binding for this uniform
    size_t size;                                // Size of this uniform data in bytes
    neko_graphics_uniform_type type;            // Type of this uniform
    neko_graphics_access_type access_type;      // Access type of uniform (compute only)
} neko_gfxt_uniform_t;

typedef struct neko_gfxt_uniform_block_desc_t {
    neko_gfxt_uniform_desc_t* layout;  // Layout for all uniform data for this block to hold
    size_t size;                       // Size of layout in bytes
} neko_gfxt_uniform_block_desc_t;

typedef struct neko_gfxt_uniform_block_lookup_key_t {
    char name[64];
} neko_gfxt_uniform_block_lookup_key_t;

typedef struct neko_gfxt_uniform_block_t {
    neko_dyn_array(neko_gfxt_uniform_t) uniforms;  // Raw uniform handle array
    neko_hash_table(uint64_t, uint32_t) lookup;    // Index lookup table (used for byte buffer offsets in material uni. data)
    size_t size;                                   // Total size of material data for entire block
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
    neko_gfxt_mesh_layout_t* layout;   // Mesh attribute layout array
    size_t size;                       // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_gfxt_mesh_import_options_t;

NEKO_API_DECL void neko_gfxt_mesh_import_options_free(neko_gfxt_mesh_import_options_t* opt);

typedef struct neko_gfxt_mesh_desc_s {
    neko_gfxt_mesh_raw_data_t* meshes;  // Mesh data array
    size_t size;                        // Size of mesh data array in bytes
    b32 keep_data;                      // Whether or not to free data after use
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
    neko_gfxt_vertex_stream_t stream;                   // All vertex data streams
    neko_handle(neko_graphics_index_buffer_t) indices;  // Index buffer
    uint32_t count;                                     // Total number of vertices
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
    neko_gfxt_uniform_block_t ublock;            // Uniform block for holding all uniform data
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
NEKO_API_DECL neko_handle(neko_graphics_texture_t) neko_gfxt_texture_generate_default();

// ECS component
typedef struct neko_gfxt_renderer {
    neko_gfxt_pipeline_t pip;
    neko_gfxt_material_t mat;
    neko_gfxt_mesh_t mesh;
    neko_gfxt_texture_t texture;
} neko_gfxt_renderer;

#endif  // NEKO_GFXT_H

/*
 */
