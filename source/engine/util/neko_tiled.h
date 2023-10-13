
#ifndef NEKO_TILED_H
#define NEKO_TILED_H

#include "engine/neko_engine.h"

#define SPRITE_SCALE 2

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
} object_t;

typedef struct object_group_s {
    neko_dyn_array(object_t) objects;

    neko_color_t color;
} object_group_t;

typedef struct map_s {
    neko_dyn_array(tileset_t) tilesets;
    neko_dyn_array(object_group_t) object_groups;
    neko_dyn_array(layer_t) layers;
} map_t;

NEKO_API_DECL void neko_tiled_load(map_t* map, const_str tmx_path, const_str res_path);

typedef struct neko_tiled_quad_quad_s {
    neko_handle(neko_graphics_texture_t) texture;
    neko_vec2 texture_size;
    neko_vec2 position;
    neko_vec2 dimentions;
    neko_vec4 rectangle;
    neko_color_t color;
    bool use_texture;
} neko_tiled_quad_t;

NEKO_API_DECL void neko_tiled_render_init(neko_command_buffer_t* cb, const char* vert_src, const char* frag_src);
NEKO_API_DECL void neko_tiled_render_deinit(neko_command_buffer_t* cb);
NEKO_API_DECL void neko_tiled_render_begin(neko_command_buffer_t* cb);
NEKO_API_DECL void neko_tiled_render_flush(neko_command_buffer_t* cb);
NEKO_API_DECL void neko_tiled_render_push(neko_command_buffer_t* cb, neko_tiled_quad_t* quad);

#endif
