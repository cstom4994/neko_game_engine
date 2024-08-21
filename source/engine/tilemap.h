
#ifndef NEKO_TILEMAP_H
#define NEKO_TILEMAP_H

#include "engine/base.h"
#include "engine/entity.h"
#include "engine/gfx.h"
#include "engine/math.h"
#include "engine/prelude.h"
#include "engine/seri.h"
#include "engine/texture.h"

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
    neko_texture_t texture;
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
    neko_texture_t batch_texture;                        // 当前绘制所用贴图
    neko_hash_table(u32, tiled_quad_list_t) quad_table;  // 分层绘制哈希表

    u32 quad_count;

    map_t map;  // tiled data

    LuaMat3 camera_mat;
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

#endif