
#ifndef NEKO_ASSET_H
#define NEKO_ASSET_H

#include "engine/neko.h"
#include "engine/neko_graphics.h"

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// Texture
typedef struct neko_asset_texture_t {
    neko_handle(neko_graphics_texture_t) hndl;
    neko_graphics_texture_desc_t desc;
} neko_asset_texture_t;

NEKO_API_DECL bool neko_asset_texture_load_from_file(const char* path, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data);
NEKO_API_DECL bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data);

// Font
typedef struct neko_baked_char_t {
    uint32_t codepoint;
    uint16_t x0, y0, x1, y1;
    float xoff, yoff, advance;
    uint32_t width, height;
} neko_baked_char_t;

typedef struct neko_asset_font_t {
    void* font_info;
    neko_baked_char_t glyphs[96];
    neko_asset_texture_t texture;
    float ascent;
    float descent;
    float line_gap;
} neko_asset_font_t;

NEKO_API_DECL bool neko_asset_font_load_from_file(const char* path, void* out, uint32_t point_size);
NEKO_API_DECL bool neko_asset_font_load_from_memory(const void* memory, size_t sz, void* out, uint32_t point_size);
NEKO_API_DECL neko_vec2 neko_asset_font_text_dimensions(const neko_asset_font_t* font, const char* text, int32_t len);
NEKO_API_DECL neko_vec2 neko_asset_font_text_dimensions_ex(const neko_asset_font_t* fp, const char* text, int32_t len, bool32_t include_past_baseline);
NEKO_API_DECL float neko_asset_font_max_height(const neko_asset_font_t* font);

// Mesh
neko_enum_decl(neko_asset_mesh_attribute_type, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT,
               NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR,
               NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT);

typedef struct neko_asset_mesh_layout_t {
    neko_asset_mesh_attribute_type type;  // Type of attribute
    uint32_t idx;                         // Optional index (for joint/weight/texcoord/color)
} neko_asset_mesh_layout_t;

typedef struct neko_asset_mesh_decl_t {
    neko_asset_mesh_layout_t* layout;  // Mesh attribute layout array
    size_t layout_size;                // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_asset_mesh_decl_t;

typedef struct neko_asset_mesh_primitive_t {
    neko_handle(neko_graphics_vertex_buffer_t) vbo;
    neko_handle(neko_graphics_index_buffer_t) ibo;
    uint32_t count;
} neko_asset_mesh_primitive_t;

typedef struct neko_asset_mesh_t {
    neko_dyn_array(neko_asset_mesh_primitive_t) primitives;
} neko_asset_mesh_t;

// Structured/packed raw mesh data
typedef struct neko_asset_mesh_raw_data_t {
    uint32_t prim_count;
    size_t* vertex_sizes;
    size_t* index_sizes;
    void** vertices;
    void** indices;
} neko_asset_mesh_raw_data_t;

NEKO_API_DECL bool neko_asset_mesh_load_from_file(const char* path, void* out, neko_asset_mesh_decl_t* decl, void* data_out, size_t data_size);
NEKO_API_DECL bool neko_util_load_gltf_data_from_file(const char* path, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, uint32_t* mesh_count);
NEKO_API_DECL bool neko_util_load_gltf_data_from_memory(const void* memory, size_t sz, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, uint32_t* mesh_count);

#endif