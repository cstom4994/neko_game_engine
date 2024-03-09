
#ifndef NEKO_TILED_H
#define NEKO_TILED_H

#include "engine/neko_engine.h"
#include "engine/util/neko_asset.h"

// c2
#include "engine/builtin/cute_c2.h"

#define SPRITE_SCALE 3

typedef struct tile_s {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_s {
    neko_handle(neko_graphics_texture_t) texture;
    u32 tile_count;
    u32 tile_width;
    u32 tile_height;
    u32 first_gid;

    u32 width, height;
} tileset_t;

typedef struct layer_s {
    tile_t* tiles;
    u32 width;
    u32 height;

    neko_color_t tint;
} layer_t;

typedef struct object_s {
    u32 id;
    s32 x, y, width, height;
    C2_TYPE phy_type;
    c2AABB aabb;
    union {
        c2AABB box;
        c2Poly poly;
    } phy;
} object_t;

typedef struct object_group_s {
    neko_dyn_array(object_t) objects;

    neko_color_t color;

    const_str name;
} object_group_t;

typedef struct map_s {
    neko_xml_document_t* doc;  // xml doc
    neko_dyn_array(tileset_t) tilesets;
    neko_dyn_array(object_group_t) object_groups;
    neko_dyn_array(layer_t) layers;
} map_t;

NEKO_API_DECL void neko_tiled_load(map_t* map, const_str tmx_path, const_str res_path);
NEKO_API_DECL void neko_tiled_unload(map_t* map);

typedef struct neko_tiled_quad_s {
    u32 tileset_id;
    neko_handle(neko_graphics_texture_t) texture;
    neko_vec2 texture_size;
    neko_vec2 position;
    neko_vec2 dimentions;
    neko_vec4 rectangle;
    neko_color_t color;
    bool use_texture;
} neko_tiled_quad_t;

#define BATCH_SIZE 2048

#define IND_PER_QUAD 6

#define VERTS_PER_QUAD 4   // 一次发送多少个verts数据
#define FLOATS_PER_VERT 9  // 每个verts数据的大小

typedef struct neko_tiled_quad_list_s {
    neko_dyn_array(neko_tiled_quad_t) quad_list;  // quad 绘制队列
} neko_tiled_quad_list_t;

typedef struct neko_tiled_renderer {
    neko_handle(neko_graphics_vertex_buffer_t) vb;
    neko_handle(neko_graphics_index_buffer_t) ib;
    neko_handle(neko_graphics_pipeline_t) pip;
    neko_handle(neko_graphics_shader_t) shader;
    neko_handle(neko_graphics_uniform_t) u_camera;
    neko_handle(neko_graphics_uniform_t) u_batch_tex;
    neko_handle(neko_graphics_texture_t) batch_texture;       // 当前绘制所用贴图
    neko_hash_table(u32, neko_tiled_quad_list_t) quad_table;  // 分层绘制哈希表

    u32 quad_count;

    map_t map;  // tiled data

    neko_mat4 camera_mat;
} neko_tiled_renderer;

NEKO_API_DECL void neko_tiled_render_init(neko_command_buffer_t* cb, neko_tiled_renderer* renderer, const_str vert_src, const_str frag_src);
NEKO_API_DECL void neko_tiled_render_deinit(neko_tiled_renderer* renderer);
NEKO_API_DECL void neko_tiled_render_begin(neko_command_buffer_t* cb, neko_tiled_renderer* renderer);
NEKO_API_DECL void neko_tiled_render_flush(neko_command_buffer_t* cb, neko_tiled_renderer* renderer);
NEKO_API_DECL void neko_tiled_render_push(neko_command_buffer_t* cb, neko_tiled_renderer* renderer, neko_tiled_quad_t quad);
NEKO_API_DECL void neko_tiled_render_draw(neko_command_buffer_t* cb, neko_tiled_renderer* renderer);

#endif
