#pragma once

#include "engine/asset.h"
#include "engine/ecs/entity.h"

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

#define SPRITE_SCALE 1.0

/*==========================
// CTiledMap draw
==========================*/

bool tiled_load(TiledMap* map, const_str tmx_path, const_str res_path);
void tiled_unload(TiledMap* map);

typedef struct tiled_quad_t {
    u32 tileset_id;
    AssetTexture texture;
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
    Array<tiled_quad_t> quad_list;  // quad 绘制队列
} tiled_quad_list_t;

typedef struct tiled_renderer {

    GLuint vao;
    GLuint vbo;
    GLuint ib;

    AssetTexture batch_texture;             // 当前绘制所用贴图
    HashMap<tiled_quad_list_t> quad_table;  // 分层绘制哈希表 (tiled_quad_list_t)

    u32 quad_count;

    u64 map_asset;  // tiled data

    mat3 camera_mat;
} tiled_renderer;

void tiled_render_init(tiled_renderer* renderer);
void tiled_render_deinit(tiled_renderer* renderer);
void tiled_render_begin(tiled_renderer* renderer);
void tiled_render_flush(tiled_renderer* renderer);
void tiled_render_push(tiled_renderer* renderer, tiled_quad_t quad);
void tiled_render_draw(tiled_renderer* renderer);

struct CTiledMap;

int tiled_render(CTiledMap* tiled);

void tiled_add(CEntity ent);
void tiled_remove(CEntity ent);
bool tiled_has(CEntity ent);
void tiled_set_map(CEntity ent, const char* str);
const char* tiled_get_map(CEntity ent);
void tiled_map_edit(CEntity ent, u32 layer_idx, u32 x, u32 y, u32 id);

void tiled_init();
void tiled_fini();
int tiled_update_all(App* app, Event evt);
void tiled_draw_all();

class CTiledMap : public CEntityBase {
public:
    tiled_renderer* render;
    vec2 pos;
    String map_name;

public:
    static Asset tiled_shader;
};
