#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/entity.h"
#include "engine/graphics.h"


// can set scale, rotation, position -- 按该顺序应用的转换

NEKO_SCRIPT(transform,

            NEKO_EXPORT void transform_add(Entity ent);

            NEKO_EXPORT void transform_remove(Entity ent);

            NEKO_EXPORT bool transform_has(Entity ent);

            // 根转换具有父级 = entity_nil
            NEKO_EXPORT void transform_set_parent(Entity ent, Entity parent);

            NEKO_EXPORT Entity transform_get_parent(Entity ent);

            NEKO_EXPORT ecs_id_t transform_get_num_children(Entity ent);

            NEKO_EXPORT Entity * transform_get_children(Entity ent);
            // 脱离父项和所有子项
            NEKO_EXPORT void transform_detach_all(Entity ent);
            // destroy ent and all children
            NEKO_EXPORT void transform_destroy_rec(Entity ent);

            NEKO_EXPORT void transform_set_position(Entity ent, vec2 pos);

            NEKO_EXPORT vec2 transform_get_position(Entity ent);

            NEKO_EXPORT void transform_translate(Entity ent, vec2 trans);

            NEKO_EXPORT void transform_set_rotation(Entity ent, Scalar rot);

            NEKO_EXPORT Scalar transform_get_rotation(Entity ent);

            NEKO_EXPORT void transform_rotate(Entity ent, Scalar rot);

            NEKO_EXPORT void transform_set_scale(Entity ent, vec2 scale);

            NEKO_EXPORT vec2 transform_get_scale(Entity ent);

            NEKO_EXPORT vec2 transform_get_world_position(Entity ent);

            NEKO_EXPORT Scalar transform_get_world_rotation(Entity ent);

            NEKO_EXPORT vec2 transform_get_world_scale(Entity ent);

            NEKO_EXPORT mat3 transform_get_world_matrix(Entity ent);  // world-space

            NEKO_EXPORT mat3 transform_get_matrix(Entity ent);  // parent-space

            NEKO_EXPORT vec2 transform_local_to_world(Entity ent, vec2 v);

            NEKO_EXPORT vec2 transform_world_to_local(Entity ent, vec2 v);

            NEKO_EXPORT ecs_id_t transform_get_dirty_count(Entity ent);

            NEKO_EXPORT void transform_set_save_filter_rec(Entity ent, bool filter);

)

void transform_init();
void transform_fini();
void transform_update_all();
void transform_save_all(Store* s);
void transform_load_all(Store* s);

struct Tile {
    float x, y, u, v;
    float u0, v0, u1, v1;
    i32 flip_bits;
};

struct TilemapEntity {
    String identifier;
    float x, y;
};

using TilemapInt = unsigned char;

struct TilemapLayer {
    String identifier;
    AssetTexture image;
    Slice<Tile> tiles;
    Slice<TilemapEntity> entities;
    i32 c_width;
    i32 c_height;
    Slice<TilemapInt> int_grid;
    float grid_size;
};

struct TilemapLevel {
    String identifier;
    String iid;
    float world_x, world_y;
    float px_width, px_height;
    Slice<TilemapLayer> layers;
};

enum TileNodeFlags {
    TileNodeFlags_Open = 1 << 0,
    TileNodeFlags_Closed = 1 << 1,
};

struct TileNode {
    TileNode* prev;
    float g;  // cost so far
    u32 flags;

    i32 x, y;
    float cost;
    Slice<TileNode*> neighbors;
};

struct TileCost {
    TilemapInt cell;
    float value;
};

struct TilePoint {
    float x, y;
};

inline u64 tile_key(i32 x, i32 y) { return ((u64)x << 32) | (u64)y; }

class b2Body;
class b2World;

struct MapLdtk {
    Arena arena;
    Slice<TilemapLevel> levels;
    HashMap<AssetTexture> images;  // key: filepath
    HashMap<b2Body*> bodies;       // key: layer name
    HashMap<TileNode> graph;       // key: x, y
    PriorityQueue<TileNode*> frontier;
    float graph_grid_size;

    bool load(String filepath);
    void trash();
    void destroy_bodies(b2World* world);
    void make_collision(b2World* world, float meter, String layer_name, Slice<TilemapInt> walls);
    void make_graph(i32 bloom, String layer_name, Slice<TileCost> costs);
    TileNode* astar(TilePoint start, TilePoint goal);
};

#define SPRITE_SCALE 3

/*==========================
// Tiled draw
==========================*/

typedef struct tile_t {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_t {
    neko_handle(gfx_texture_t) texture;
    u32 tile_count;
    u32 tile_width;
    u32 tile_height;
    u32 first_gid;

    u32 width, height;
} tileset_t;

typedef struct layer_t {
    tile_t* tiles;
    u32 width;
    u32 height;

    Color256 tint;
} layer_t;

typedef struct object_t {
    u32 id;
    i32 x, y, width, height;
    // C2_TYPE phy_type;
    // c2AABB aabb;
    // union {
    //     c2AABB box;
    //     c2Poly poly;
    // } phy;
} object_t;

typedef struct object_group_t {
    neko_dyn_array(object_t) objects;

    Color256 color;

    const_str name;
} object_group_t;

typedef struct map_t {
    xml_document_t* doc;  // xml doc
    neko_dyn_array(tileset_t) tilesets;
    neko_dyn_array(object_group_t) object_groups;
    neko_dyn_array(layer_t) layers;
} map_t;

void tiled_load(map_t* map, const_str tmx_path, const_str res_path);
void tiled_unload(map_t* map);

typedef struct tiled_quad_t {
    u32 tileset_id;
    gfx_texture_t texture;
    vec2 texture_size;
    vec2 position;
    vec2 dimentions;
    vec4 rectangle;
    Color256 color;
    bool use_texture;
} tiled_quad_t;

#define BATCH_SIZE 4096

#define IND_PER_QUAD 6

#define VERTS_PER_QUAD 4   // 一次发送多少个verts数据
#define FLOATS_PER_VERT 9  // 每个verts数据的大小

typedef struct tiled_quad_list_t {
    neko_dyn_array(tiled_quad_t) quad_list;  // quad 绘制队列
} tiled_quad_list_t;

typedef struct tiled_renderer {
    // neko_handle(gfx_vertex_buffer_t) vb;
    // neko_handle(gfx_index_buffer_t) ib;
    // neko_handle(gfx_pipeline_t) pip;
    // neko_handle(gfx_shader_t) shader;

    GLuint vao;
    GLuint vbo;
    GLuint ib;

    // GLuint shader;

    // neko_handle(gfx_uniform_t) u_camera;
    // neko_handle(gfx_uniform_t) u_batch_tex;
    gfx_texture_t batch_texture;                         // 当前绘制所用贴图
    neko_hash_table(u32, tiled_quad_list_t) quad_table;  // 分层绘制哈希表

    u32 quad_count;

    map_t map;  // tiled data

    mat3 camera_mat;
} tiled_renderer;

void tiled_render_init(command_buffer_t* cb, tiled_renderer* renderer);
void tiled_render_deinit(tiled_renderer* renderer);
void tiled_render_begin(command_buffer_t* cb, tiled_renderer* renderer);
void tiled_render_flush(command_buffer_t* cb, tiled_renderer* renderer);
void tiled_render_push(command_buffer_t* cb, tiled_renderer* renderer, tiled_quad_t quad);
void tiled_render_draw(command_buffer_t* cb, tiled_renderer* renderer);

struct Tiled;

int tiled_render(command_buffer_t* cb, Tiled* tiled);

NEKO_SCRIPT(tiled,

            NEKO_EXPORT void tiled_add(Entity ent);

            NEKO_EXPORT void tiled_remove(Entity ent);

            NEKO_EXPORT bool tiled_has(Entity ent);

            NEKO_EXPORT void tiled_set_map(Entity ent, const char* str);

            NEKO_EXPORT const char* tiled_get_map(Entity ent);

)

void tiled_init();
void tiled_fini();
void tiled_update_all();
void tiled_draw_all();

/*
 * if no current camera, the (inverse) view matrix is identity, which means
 * the view is a 2x2 unit box at the center of the world
 */

NEKO_SCRIPT(camera,

            NEKO_EXPORT void camera_add(Entity ent);

            NEKO_EXPORT void camera_remove(Entity ent);

            NEKO_EXPORT bool camera_has(Entity ent);

            // set camera to use in edit mode -- not saved/loaded
            NEKO_EXPORT void camera_set_edit_camera(Entity ent);

            // set/get currently active camera -- entity_nil if none
            NEKO_EXPORT void camera_set_current(Entity ent, bool current);

            NEKO_EXPORT bool camera_get_current(Entity ent);

            NEKO_EXPORT void camera_set_current_camera(Entity ent);

            NEKO_EXPORT Entity camera_get_current_camera();

            // number of world units to fit vertically on screen
            NEKO_EXPORT void camera_set_viewport_height(Entity ent, Scalar height);

            NEKO_EXPORT Scalar camera_get_viewport_height(Entity ent);

            NEKO_EXPORT mat3 camera_get_inverse_view_matrix();

            // screen-space coordinates <-> world coordinates transformations
            NEKO_EXPORT vec2 camera_world_to_pixels(vec2 p);

            NEKO_EXPORT vec2 camera_world_to_unit(vec2 p);

            NEKO_EXPORT vec2 camera_pixels_to_world(vec2 p);

            NEKO_EXPORT vec2 camera_unit_to_world(vec2 p);

)

const mat3* camera_get_inverse_view_matrix_ptr();  // for quick GLSL binding

void camera_init();
void camera_fini();
void camera_update_all();
void camera_save_all(Store* s);
void camera_load_all(Store* s);

#endif
