
#ifndef NEKO_COMPONENT_HPP
#define NEKO_COMPONENT_HPP

#include "engine/neko.hpp"
#include "engine/neko_asset.h"

struct ldtk_map_tile {
    float x, y, u, v;
    float u0, v0, u1, v1;
    s32 flip_bits;
};

struct ldtk_map_ent {
    String identifier;
    float x, y;
};

using ldtk_map_int = unsigned char;

struct ldtk_map_layer {
    String identifier;
    neko_image image;
    Slice<ldtk_map_tile> tiles;
    Slice<ldtk_map_ent> entities;
    s32 c_width;
    s32 c_height;
    Slice<ldtk_map_int> int_grid;
    float grid_size;
};

struct ldtk_map_level {
    String identifier;
    String iid;
    float world_x, world_y;
    float px_width, px_height;
    Slice<ldtk_map_layer> layers;
};

enum ldtk_map_node_flags {
    TileNodeFlags_Open = 1 << 0,
    TileNodeFlags_Closed = 1 << 1,
};

struct ldtk_map_node {
    ldtk_map_node *prev;
    float g;  // cost so far
    u32 flags;

    s32 x, y;
    float cost;
    Slice<ldtk_map_node *> neighbors;
};

struct ldtk_tile_cost {
    ldtk_map_int cell;
    float value;
};

struct ldtk_map_point {
    float x, y;
};

inline u64 tile_key(s32 x, s32 y) { return ((u64)x << 32) | (u64)y; }

class b2Body;
class b2World;

struct ldtk_map {
    Arena arena;
    Slice<ldtk_map_level> levels;
    HashMap<neko_image> images;  // key: filepath
    HashMap<b2Body *> bodies;      // key: layer name
    HashMap<ldtk_map_node> graph;  // key: x, y
    PriorityQueue<ldtk_map_node *> frontier;
    float graph_grid_size;

    bool load(String filepath);
    void trash();
    void destroy_bodies(b2World *world);
    void make_collision(b2World *world, float meter, String layer_name, Slice<ldtk_map_int> walls);
    void make_graph(s32 bloom, String layer_name, Slice<ldtk_tile_cost> costs);
    ldtk_map_node *astar(ldtk_map_point start, ldtk_map_point goal);
};

#endif