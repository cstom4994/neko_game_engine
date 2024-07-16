#include "neko_asset.h"

#include <cstdio>
#include <filesystem>
#include <new>
#include <unordered_map>

#include "neko.hpp"
#include "neko_app.h"
#include "neko_base.h"
#include "neko_common.h"
#include "neko_lua.h"
#include "neko_os.h"
#include "neko_prelude.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// miniz
#include <miniz.h>

// deps
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>
#include <sokol_gfx.h>
#include <stb_image.h>
#include <stb_image_resize2.h>

#define neko_little_endian 1

// embed
static const u8 g_font_monocrafte_data[] = {
#include "Monocraft.ttf.h"
};

bool Image::load(String filepath, bool generate_mips) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    i32 width = 0, height = 0, channels = 0;
    stbi_uc *data = nullptr;
    {
        PROFILE_BLOCK("stb_image load");
        data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &width, &height, &channels, 4);
    }
    if (!data) {
        return false;
    }
    neko_defer(stbi_image_free(data));

    sg_image_desc desc = {};
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.width = width;
    desc.height = height;
    desc.data.subimage[0][0].ptr = data;
    desc.data.subimage[0][0].size = width * height * 4;

    Array<u8 *> mips = {};
    neko_defer({
        for (u8 *mip : mips) {
            mem_free(mip);
        }
        mips.trash();
    });

    if (generate_mips) {
        mips.reserve(SG_MAX_MIPMAPS);

        u8 *prev = data;
        i32 w0 = width;
        i32 h0 = height;
        i32 w1 = w0 / 2;
        i32 h1 = h0 / 2;

        while (w1 > 1 && h1 > 1) {
            PROFILE_BLOCK("generate mip");

            u8 *mip = (u8 *)mem_alloc(w1 * h1 * 4);
            stbir_resize_uint8_linear(prev, w0, h0, 0, mip, w1, h1, 0, STBIR_RGBA);
            mips.push(mip);

            desc.data.subimage[0][mips.len].ptr = mip;
            desc.data.subimage[0][mips.len].size = w1 * h1 * 4;

            prev = mip;
            w0 = w1;
            h0 = h1;
            w1 /= 2;
            h1 /= 2;
        }
    }

    desc.num_mipmaps = mips.len + 1;

    u32 id = 0;
    {
        PROFILE_BLOCK("make image");
        LockGuard lock{&g_app->gpu_mtx};
        id = sg_make_image(desc).id;
    }

    Image img = {};
    img.id = id;
    img.width = width;
    img.height = height;
    img.has_mips = generate_mips;
    *this = img;

    NEKO_TRACE("created image (%dx%d, %d channels, mipmaps: %s) with id %d", width, height, channels, generate_mips ? "true" : "false", id);
    return true;
}

void Image::trash() {
    LockGuard lock{&g_app->gpu_mtx};
    sg_destroy_image({id});
}

bool SpriteData::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t *ase = nullptr;
    {
        PROFILE_BLOCK("aseprite load");
        ase = neko_aseprite_load_from_memory(contents.data, (i32)contents.len);
    }
    neko_defer(neko_aseprite_free(ase));

    Arena arena = {};

    i32 rect = ase->w * ase->h * 4;

    Slice<SpriteFrame> frames = {};
    frames.resize(&arena, ase->frame_count);

    Array<char> pixels = {};
    pixels.reserve(ase->frame_count * rect);
    neko_defer(pixels.trash());

    for (i32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t &frame = ase->frames[i];

        SpriteFrame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (float)i / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (float)(i + 1) / ase->frame_count;

        frames[i] = sf;
        memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
    }

    sg_image_desc desc = {};
    desc.width = ase->w;
    desc.height = ase->h * ase->frame_count;
    desc.data.subimage[0][0].ptr = pixels.data;
    desc.data.subimage[0][0].size = ase->frame_count * rect;

    u32 id = 0;
    {
        PROFILE_BLOCK("make image");
        LockGuard lock{&g_app->gpu_mtx};
        id = sg_make_image(desc).id;
    }

    Image img = {};
    img.id = id;
    img.width = desc.width;
    img.height = desc.height;

    HashMap<SpriteLoop> by_tag = {};
    by_tag.reserve(ase->tag_count);

    for (i32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t &tag = ase->tags[i];

        u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

        SpriteLoop loop = {};

        loop.indices.resize(&arena, len);
        for (i32 j = 0; j < len; j++) {
            loop.indices[j] = j + tag.from_frame;
        }

        by_tag[fnv1a(tag.name)] = loop;
    }

    NEKO_TRACE("created sprite with image id: %d and %llu frames", img.id, (unsigned long long)frames.len);

    SpriteData s = {};
    s.arena = arena;
    s.img = img;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *this = s;
    return true;
}

void SpriteData::trash() {
    by_tag.trash();
    arena.trash();
}

bool Sprite::play(String tag) {
    u64 key = fnv1a(tag);
    bool same = loop == key;
    loop = key;
    return same;
}

void Sprite::update(float dt) {
    SpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    i32 index = view.frame();
    SpriteFrame frame = view.data.frames[index];

    elapsed += dt * 1000;
    if (elapsed > frame.duration) {
        if (current_frame == view.len() - 1) {
            current_frame = 0;
        } else {
            current_frame++;
        }

        elapsed -= frame.duration;
    }
}

void Sprite::set_frame(i32 frame) {
    SpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    if (0 <= frame && frame < view.len()) {
        current_frame = frame;
        elapsed = 0;
    }
}

bool SpriteView::make(Sprite *spr) {
    Asset a = {};
    bool ok = asset_read(spr->sprite, &a);
    if (!ok) {
        return false;
    }

    SpriteData data = a.sprite;
    const SpriteLoop *res = data.by_tag.get(spr->loop);

    SpriteView view = {};
    view.sprite = spr;
    view.data = data;

    if (res != nullptr) {
        view.loop = *res;
    }

    *this = view;
    return true;
}

i32 SpriteView::frame() {
    if (loop.indices.data != nullptr) {
        return loop.indices[sprite->current_frame];
    } else {
        return sprite->current_frame;
    }
}

u64 SpriteView::len() {
    if (loop.indices.data != nullptr) {
        return loop.indices.len;
    } else {
        return data.frames.len;
    }
}

static bool layer_from_json(TilemapLayer *layer, JSON *json, bool *ok, Arena *arena, String filepath, HashMap<Image> *images) {
    PROFILE_FUNC();

    layer->identifier = arena->bump_string(json->lookup_string("__identifier", ok));
    layer->c_width = (i32)json->lookup_number("__cWid", ok);
    layer->c_height = (i32)json->lookup_number("__cHei", ok);
    layer->grid_size = json->lookup_number("__gridSize", ok);

    JSON tileset_rel_path = json->lookup("__tilesetRelPath", ok);

    JSONArray *int_grid_csv = json->lookup_array("intGridCsv", ok);

    JSONArray *grid_tiles = json->lookup_array("gridTiles", ok);
    JSONArray *auto_layer_tiles = json->lookup_array("autoLayerTiles", ok);

    JSONArray *arr_tiles = (grid_tiles != nullptr && grid_tiles->index != 0) ? grid_tiles : auto_layer_tiles;

    JSONArray *entity_instances = json->lookup_array("entityInstances", ok);

    if (tileset_rel_path.kind == JSONKind_String) {
        StringBuilder sb = {};
        neko_defer(sb.trash());
        sb.swap_filename(filepath, tileset_rel_path.as_string(ok));

        u64 key = fnv1a(String(sb));

        Image *img = images->get(key);
        if (img != nullptr) {
            layer->image = *img;
        } else {
            Image create_img = {};
            bool success = create_img.load(String(sb), false);
            if (!success) {
                return false;
            }

            layer->image = create_img;
            (*images)[key] = create_img;
        }
    }

    Slice<TilemapInt> grid = {};
    if (int_grid_csv != nullptr) {
        PROFILE_BLOCK("int grid");

        i32 len = int_grid_csv->index + 1;
        grid.resize(arena, len);
        for (JSONArray *a = int_grid_csv; a != nullptr; a = a->next) {
            grid[--len] = (TilemapInt)a->value.as_number(ok);
        }
    }
    layer->int_grid = grid;

    Slice<Tile> tiles = {};
    if (arr_tiles != nullptr) {
        PROFILE_BLOCK("tiles");

        i32 len = arr_tiles->index + 1;
        tiles.resize(arena, len);
        for (JSONArray *a = arr_tiles; a != nullptr; a = a->next) {
            JSON px = a->value.lookup("px", ok);
            JSON src = a->value.lookup("src", ok);

            Tile tile = {};
            tile.x = px.index_number(0, ok);
            tile.y = px.index_number(1, ok);

            tile.u = src.index_number(0, ok);
            tile.v = src.index_number(1, ok);

            tile.flip_bits = (i32)a->value.lookup_number("f", ok);
            tiles[--len] = tile;
        }
    }
    layer->tiles = tiles;

    for (Tile &tile : layer->tiles) {
        tile.u0 = tile.u / layer->image.width;
        tile.v0 = tile.v / layer->image.height;
        tile.u1 = (tile.u + layer->grid_size) / layer->image.width;
        tile.v1 = (tile.v + layer->grid_size) / layer->image.height;

        i32 FLIP_X = 1 << 0;
        i32 FLIP_Y = 1 << 1;

        if (tile.flip_bits & FLIP_X) {
            float tmp = tile.u0;
            tile.u0 = tile.u1;
            tile.u1 = tmp;
        }

        if (tile.flip_bits & FLIP_Y) {
            float tmp = tile.v0;
            tile.v0 = tile.v1;
            tile.v1 = tmp;
        }
    }

    Slice<TilemapEntity> entities = {};
    if (entity_instances != nullptr) {
        PROFILE_BLOCK("entities");

        i32 len = entity_instances->index + 1;
        entities.resize(arena, len);
        for (JSONArray *a = entity_instances; a != nullptr; a = a->next) {
            JSON px = a->value.lookup("px", ok);

            TilemapEntity entity = {};
            entity.x = px.index_number(0, ok);
            entity.y = px.index_number(1, ok);
            entity.identifier = arena->bump_string(a->value.lookup_string("__identifier", ok));

            entities[--len] = entity;
        }
    }
    layer->entities = entities;

    return true;
}

static bool level_from_json(TilemapLevel *level, JSON *json, bool *ok, Arena *arena, String filepath, HashMap<Image> *images) {
    PROFILE_FUNC();

    level->identifier = arena->bump_string(json->lookup_string("identifier", ok));
    level->iid = arena->bump_string(json->lookup_string("iid", ok));
    level->world_x = json->lookup_number("worldX", ok);
    level->world_y = json->lookup_number("worldY", ok);
    level->px_width = json->lookup_number("pxWid", ok);
    level->px_height = json->lookup_number("pxHei", ok);

    JSONArray *layer_instances = json->lookup_array("layerInstances", ok);

    Slice<TilemapLayer> layers = {};
    if (layer_instances != nullptr) {
        i32 len = layer_instances->index + 1;
        layers.resize(arena, len);
        for (JSONArray *a = layer_instances; a != nullptr; a = a->next) {
            TilemapLayer layer = {};
            bool success = layer_from_json(&layer, &a->value, ok, arena, filepath, images);
            if (!success) {
                return false;
            }
            layers[--len] = layer;
        }
    }
    level->layers = layers;

    return true;
}

bool map_ldtk::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool success = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, filepath);
    if (!success) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    bool ok = true;
    JSONDocument doc = {};
    doc.parse(contents);
    neko_defer(doc.trash());

    if (doc.error.len != 0) {
        return false;
    }

    Arena arena = {};
    HashMap<Image> images = {};
    bool created = false;
    neko_defer({
        if (!created) {
            for (auto [k, v] : images) {
                v->trash();
            }
            images.trash();
            arena.trash();
        }
    });

    JSONArray *arr_levels = doc.root.lookup_array("levels", &ok);

    Slice<TilemapLevel> levels = {};
    if (arr_levels != nullptr) {
        i32 len = arr_levels->index + 1;
        levels.resize(&arena, len);
        for (JSONArray *a = arr_levels; a != nullptr; a = a->next) {
            TilemapLevel level = {};
            bool success = level_from_json(&level, &a->value, &ok, &arena, filepath, &images);
            if (!success) {
                return false;
            }
            levels[--len] = level;
        }
    }

    if (!ok) {
        return false;
    }

    map_ldtk tilemap = {};
    tilemap.arena = arena;
    tilemap.levels = levels;
    tilemap.images = images;

    NEKO_TRACE("loaded tilemap with %llu levels", (unsigned long long)tilemap.levels.len);
    *this = tilemap;
    created = true;
    return true;
}

void map_ldtk::trash() {
    for (auto [k, v] : images) {
        v->trash();
    }
    images.trash();

    bodies.trash();
    graph.trash();
    frontier.trash();

    arena.trash();
}

void map_ldtk::destroy_bodies(b2World *world) {
    for (auto [k, v] : bodies) {
        world->DestroyBody(*v);
    }
}

static void make_collision_for_layer(b2Body *body, TilemapLayer *layer, float world_x, float world_y, float meter, Slice<TilemapInt> walls) {
    PROFILE_FUNC();

    auto is_wall = [layer, walls](i32 y, i32 x) {
        if (x >= layer->c_width || y >= layer->c_height) {
            return false;
        }

        for (TilemapInt n : walls) {
            if (layer->int_grid[y * layer->c_width + x] == n) {
                return true;
            }
        }

        return false;
    };

    Array<bool> filled = {};
    neko_defer(filled.trash());
    filled.resize(layer->c_width * layer->c_height);
    memset(filled.data, 0, layer->c_width * layer->c_height);
    for (i32 y = 0; y < layer->c_height; y++) {
        for (i32 x = 0; x < layer->c_width; x++) {
            i32 x0 = x;
            i32 y0 = y;
            i32 x1 = x;
            i32 y1 = y;

            if (!is_wall(y1, x1)) {
                continue;
            }

            if (filled[y1 * layer->c_width + x1]) {
                continue;
            }

            while (is_wall(y1, x1 + 1)) {
                x1++;
            }

            while (true) {
                bool walkable = false;
                for (i32 x = x0; x <= x1; x++) {
                    if (!is_wall(y1 + 1, x)) {
                        walkable = true;
                    }
                }

                if (walkable) {
                    break;
                }

                y1++;
            }

            for (i32 y = y0; y <= y1; y++) {
                for (i32 x = x0; x <= x1; x++) {
                    filled[y * layer->c_width + x] = true;
                }
            }

            float dx = (float)(x1 + 1 - x0) * layer->grid_size / 2.0f;
            float dy = (float)(y1 + 1 - y0) * layer->grid_size / 2.0f;

            b2Vec2 pos = {
                    (x0 * layer->grid_size + dx + world_x) / meter,
                    (y0 * layer->grid_size + dy + world_y) / meter,
            };

            b2PolygonShape box = {};
            box.SetAsBox(dx / meter, dy / meter, pos, 0.0f);

            b2FixtureDef def = {};
            def.friction = 0;
            def.shape = &box;

            body->CreateFixture(&def);
        }
    }
}

void map_ldtk::make_collision(b2World *world, float meter, String layer_name, Slice<TilemapInt> walls) {
    PROFILE_FUNC();

    b2Body *body = nullptr;
    {
        b2BodyDef def = {};
        def.position.x = 0;
        def.position.y = 0;
        def.fixedRotation = true;
        def.allowSleep = true;
        def.awake = false;
        def.type = b2_staticBody;
        def.gravityScale = 0;

        body = world->CreateBody(&def);
    }

    for (TilemapLevel &level : levels) {
        for (TilemapLayer &l : level.layers) {
            if (l.identifier == layer_name) {
                make_collision_for_layer(body, &l, level.world_x, level.world_y, meter, walls);
            }
        }
    }

    bodies[fnv1a(layer_name)] = body;
}

static float get_tile_cost(TilemapInt n, Slice<TileCost> costs) {
    for (TileCost cost : costs) {
        if (cost.cell == n) {
            return cost.value;
        }
    }
    return -1;
}

static void make_graph_for_layer(HashMap<TileNode> *graph, TilemapLayer *layer, float world_x, float world_y, Slice<TileCost> costs) {
    PROFILE_FUNC();

    for (i32 y = 0; y < layer->c_height; y++) {
        for (i32 x = 0; x < layer->c_width; x++) {
            float cost = get_tile_cost(layer->int_grid[y * layer->c_width + x], costs);
            if (cost > 0) {
                TileNode node = {};
                node.x = (i32)(x + world_x);
                node.y = (i32)(y + world_x);
                node.cost = cost;

                (*graph)[tile_key(node.x, node.y)] = node;
            }
        }
    }
}

static bool tilemap_rect_overlaps_graph(HashMap<TileNode> *graph, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 lhs = x0 <= x1 ? x0 : x1;
    i32 rhs = x0 <= x1 ? x1 : x0;
    i32 top = y0 <= y1 ? y0 : y1;
    i32 bot = y0 <= y1 ? y1 : y0;

    for (i32 y = top; y <= bot; y++) {
        for (i32 x = lhs; x <= rhs; x++) {
            if ((x == x0 && y == y0) || (x == x1 && y == y1)) {
                continue;
            }

            TileNode *node = graph->get(tile_key(x, y));
            if (node == nullptr) {
                return false;
            }
        }
    }

    return true;
}

static void create_neighbor_nodes(HashMap<TileNode> *graph, Arena *arena, i32 bloom) {
    PROFILE_FUNC();

    for (auto [k, v] : *graph) {
        i32 len = 0;
        Slice<TileNode *> neighbors = {};

        for (i32 y = -bloom; y <= bloom; y++) {
            for (i32 x = -bloom; x <= bloom; x++) {
                if (x == 0 && y == 0) {
                    continue;
                }

                i32 dx = v->x + x;
                i32 dy = v->y + y;
                TileNode *node = graph->get(tile_key(dx, dy));
                if (node != nullptr) {
                    bool ok = tilemap_rect_overlaps_graph(graph, v->x, v->y, dx, dy);
                    if (!ok) {
                        continue;
                    }

                    if (len == neighbors.len) {
                        i32 grow = len > 0 ? len * 2 : 8;
                        neighbors.resize(arena, grow);
                    }

                    neighbors[len] = node;
                    len++;
                }
            }
        }

        neighbors.resize(arena, len);
        v->neighbors = neighbors;
    }
}

void map_ldtk::make_graph(i32 bloom, String layer_name, Slice<TileCost> costs) {
    for (TilemapLevel &level : levels) {
        for (TilemapLayer &l : level.layers) {
            if (l.identifier == layer_name) {
                if (graph_grid_size == 0) {
                    graph_grid_size = l.grid_size;
                }
                make_graph_for_layer(&graph, &l, level.world_x, level.world_y, costs);
            }
        }
    }

    create_neighbor_nodes(&graph, &arena, bloom);
}

static float tile_distance(TileNode *lhs, TileNode *rhs) {
    float dx = lhs->x - rhs->x;
    float dy = lhs->y - rhs->y;
    return sqrtf(dx * dx + dy * dy);
}

static float tile_heuristic(TileNode *lhs, TileNode *rhs) {
    float D = 1;
    float D2 = 1.4142135f;

    float dx = (float)abs(lhs->x - rhs->x);
    float dy = (float)abs(lhs->y - rhs->y);
    return D * (dx + dy) + (D2 - 2 * D) * fminf(dx, dy);
}

static void astar_reset(map_ldtk *tm) {
    PROFILE_FUNC();

    tm->frontier.len = 0;

    for (auto [k, v] : tm->graph) {
        v->prev = nullptr;
        v->g = 0;
        v->flags = 0;
    }
}

TileNode *map_ldtk::astar(TilePoint start, TilePoint goal) {
    PROFILE_FUNC();

    astar_reset(this);

    i32 sx = (i32)(start.x / graph_grid_size);
    i32 sy = (i32)(start.y / graph_grid_size);
    i32 ex = (i32)(goal.x / graph_grid_size);
    i32 ey = (i32)(goal.y / graph_grid_size);

    TileNode *end = graph.get(tile_key(ex, ey));
    if (end == nullptr) {
        return nullptr;
    }

    TileNode *begin = graph.get(tile_key(sx, sy));
    if (begin == nullptr) {
        return nullptr;
    }

    float g = 0;
    float h = tile_heuristic(begin, end);
    float f = g + h;
    begin->g = 0;
    begin->flags |= TileNodeFlags_Open;
    frontier.push(begin, f);

    while (frontier.len != 0) {
        TileNode *top = nullptr;
        frontier.pop(&top);
        top->flags |= TileNodeFlags_Closed;

        if (top == end) {
            return top;
        }

        for (TileNode *next : top->neighbors) {
            if (next->flags & TileNodeFlags_Closed) {
                continue;
            }

            float g = top->g + next->cost * tile_distance(top, next);

            bool open = next->flags & TileNodeFlags_Open;
            if (!open || g < next->g) {
                float h = tile_heuristic(next, end);
                float f = g + h;

                next->g = g;
                next->prev = top;
                next->flags |= TileNodeFlags_Open;

                frontier.push(next, f);
            }
        }
    }

    return nullptr;
}

bool FontFamily::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, filepath);
    if (!ok) {
        return false;
    }

    FontFamily f = {};
    f.ttf = contents;
    f.sb = {};
    *this = f;
    return true;
}

void FontFamily::load_default() {
    PROFILE_FUNC();

    FontFamily f = {};
    f.ttf = {(const char *)g_font_monocrafte_data, sizeof(g_font_monocrafte_data)};
    f.sb = {};
    *this = f;
}

void FontFamily::trash() {
    for (auto [k, v] : ranges) {
        v->image.trash();
    }
    sb.trash();
    ranges.trash();
    if (ttf.data != (const char *)g_font_monocrafte_data) {
        mem_free(ttf.data);
    }
}

struct FontKey {
    float size;
    i32 ch;
};

static FontKey font_key(float size, i32 charcode) {
    FontKey fk = {};
    fk.size = size;
    fk.ch = (charcode / array_size(FontRange::chars)) * array_size(FontRange::chars);
    return fk;
}

static void make_font_range(FontRange *out, FontFamily *font, FontKey key) {
    PROFILE_FUNC();

    i32 width = 256;
    i32 height = 256;

    u8 *bitmap = nullptr;
    while (bitmap == nullptr) {
        PROFILE_BLOCK("try bake");

        bitmap = (u8 *)mem_alloc(width * height);
        i32 res = stbtt_BakeFontBitmap((u8 *)font->ttf.data, 0, key.size, bitmap, width, height, key.ch, array_size(out->chars), out->chars);
        if (res < 0) {
            mem_free(bitmap);
            bitmap = nullptr;
            width *= 2;
            height *= 2;
        }
    }
    neko_defer(mem_free(bitmap));

    u8 *image = (u8 *)mem_alloc(width * height * 4);
    neko_defer(mem_free(image));

    {
        PROFILE_BLOCK("convert rgba");

        for (i32 i = 0; i < width * height * 4; i += 4) {
            image[i + 0] = 255;
            image[i + 1] = 255;
            image[i + 2] = 255;
            image[i + 3] = bitmap[i / 4];
        }
    }

    u32 id = 0;
    {
        PROFILE_BLOCK("make image");

        sg_image_desc sg_image = {};
        sg_image.width = width;
        sg_image.height = height;
        sg_image.data.subimage[0][0].ptr = image;
        sg_image.data.subimage[0][0].size = width * height * 4;

        {
            LockGuard lock{&g_app->gpu_mtx};
            id = sg_make_image(sg_image).id;
        }
    }

    out->image.id = id;
    out->image.width = width;
    out->image.height = height;

    NEKO_TRACE("created font range with id %d", id);
}

static FontRange *get_range(FontFamily *font, FontKey key) {
    u64 hash = *(u64 *)&key;
    FontRange *range = font->ranges.get(hash);
    if (range == nullptr) {
        range = &font->ranges[hash];
        make_font_range(range, font, key);
    }

    return range;
}

stbtt_aligned_quad FontFamily::quad(u32 *img, float *x, float *y, float size, i32 ch) {
    FontRange *range = get_range(this, font_key(size, ch));
    assert(range != nullptr);

    ch = ch % array_size(FontRange::chars);

    float xpos = 0;
    float ypos = 0;
    stbtt_aligned_quad q = {};
    stbtt_GetBakedQuad(range->chars, (i32)range->image.width, (i32)range->image.height, ch, &xpos, &ypos, &q, 1);

    stbtt_bakedchar *baked = range->chars + ch;
    *img = range->image.id;
    *x = *x + baked->xadvance;
    return q;
}

float FontFamily::width(float size, String text) {
    float width = 0;
    for (Rune r : UTF8(text)) {
        u32 code = r.charcode();
        FontRange *range = get_range(this, font_key(size, code));
        assert(range != nullptr);

        const stbtt_bakedchar *baked = range->chars + (code % array_size(FontRange::chars));
        width += baked->xadvance;
    }
    return width;
}

bool Atlas::load(String filepath, bool generate_mips) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    Image img = {};
    HashMap<AtlasImage> by_name = {};

    for (String line : SplitLines(contents)) {
        switch (line.data[0]) {
            case 'a': {
                Scanner scan = line;
                scan.next_string();  // discard 'a'
                String filename = scan.next_string();

                StringBuilder sb = {};
                neko_defer(sb.trash());
                sb.swap_filename(filepath, filename);
                bool ok = img.load(String(sb), generate_mips);
                if (!ok) {
                    return false;
                }
                break;
            }
            case 's': {
                if (img.id == 0) {
                    return false;
                }

                Scanner scan = line;
                scan.next_string();  // discard 's'
                String name = scan.next_string();
                scan.next_string();  // discard origin x
                scan.next_string();  // discard origin y
                i32 x = scan.next_int();
                i32 y = scan.next_int();
                i32 width = scan.next_int();
                i32 height = scan.next_int();
                i32 padding = scan.next_int();
                i32 trimmed = scan.next_int();
                scan.next_int();  // discard trim x
                scan.next_int();  // discard trim y
                i32 trim_width = scan.next_int();
                i32 trim_height = scan.next_int();

                AtlasImage atlas_img = {};
                atlas_img.img = img;
                atlas_img.u0 = (x + padding) / (float)img.width;
                atlas_img.v0 = (y + padding) / (float)img.height;

                if (trimmed != 0) {
                    atlas_img.width = (float)trim_width;
                    atlas_img.height = (float)trim_height;
                    atlas_img.u1 = (x + padding + trim_width) / (float)img.width;
                    atlas_img.v1 = (y + padding + trim_height) / (float)img.height;
                } else {
                    atlas_img.width = (float)width;
                    atlas_img.height = (float)height;
                    atlas_img.u1 = (x + padding + width) / (float)img.width;
                    atlas_img.v1 = (y + padding + height) / (float)img.height;
                }

                by_name[fnv1a(name)] = atlas_img;

                break;
            }
            default:
                break;
        }
    }

    NEKO_TRACE("created atlas with image id: %d and %llu entries", img.id, (unsigned long long)by_name.load);

    Atlas a;
    a.by_name = by_name;
    a.img = img;
    *this = a;

    return true;
}

void Atlas::trash() {
    by_name.trash();
    img.trash();
}

AtlasImage *Atlas::get(String name) {
    u64 key = fnv1a(name);
    return by_name.get(key);
}

static u32 read4(char *bytes) {
    u32 n;
    memcpy(&n, bytes, 4);
    return n;
}

static bool read_entire_file_raw(String *out, String filepath) {
    PROFILE_FUNC();

    String path = to_cstr(filepath);
    neko_defer(mem_free(path.data));

    FILE *file = neko_fopen(path.data, "rb");
    if (file == nullptr) {
        NEKO_WARN("failed to load file %s", path.data);
        return false;
    }

    neko_fseek(file, 0L, SEEK_END);
    size_t size = neko_ftell(file);
    rewind(file);

    char *buf = (char *)mem_alloc(size + 1);
    size_t read = neko_fread(buf, sizeof(char), size, file);
    neko_fclose(file);

    if (read != size) {
        mem_free(buf);
        return false;
    }

    buf[size] = 0;
    *out = {buf, size};
    return true;
}

static bool list_all_files_help(Array<String> *files, String path) {
    PROFILE_FUNC();

    const_str p = path.len ? path.data : ".";

    for (const auto &entry : std::filesystem::recursive_directory_iterator(p)) {
        if (entry.is_regular_file()) files->push(str_fmt("%s", entry.path().string().data()));
    }

    // tinydir_dir dir;
    // if (path.len == 0) {
    //     tinydir_open(&dir, ".");
    // } else {
    //     tinydir_open(&dir, path.data);
    // }
    // neko_defer(tinydir_close(&dir));

    // while (dir.has_next) {
    //     tinydir_file file;
    //     tinydir_readfile(&dir, &file);

    //     if (strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0) {
    //         if (file.is_dir) {
    //             String s = str_fmt("%s%s/", path.data, file.name);
    //             neko_defer(mem_free(s.data));
    //             list_all_files_help(files, s);
    //         } else {
    //             files->push(str_fmt("%s%s", path.data, file.name));
    //         }
    //     }

    //     tinydir_next(&dir);
    // }

    return true;
}

struct FileSystem {
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual bool mount(String filepath) = 0;
    virtual bool file_exists(String filepath) = 0;
    virtual bool read_entire_file(String *out, String filepath) = 0;
    virtual bool list_all_files(Array<String> *files) = 0;
};

struct MyKeyHash {
    std::size_t operator()(const String &key) const { return std::hash<void *>()(key.data); }
};

static std::unordered_map<String, FileSystem *, MyKeyHash> g_filesystem_list;

struct DirectoryFileSystem : FileSystem {
    void make() {}
    void trash() {}

    bool mount(String filepath) {
        String path = to_cstr(filepath);
        neko_defer(mem_free(path.data));

        i32 res = os_change_dir(path.data);
        return res == 0;
    }

    bool file_exists(String filepath) {
        String path = to_cstr(filepath);
        neko_defer(mem_free(path.data));

        FILE *fp = neko_fopen(path.data, "r");
        if (fp != nullptr) {
            neko_fclose(fp);
            return true;
        }

        return false;
    }

    bool read_entire_file(String *out, String filepath) { return read_entire_file_raw(out, filepath); }

    bool list_all_files(Array<String> *files) { return list_all_files_help(files, ""); }
};

struct ZipFileSystem : FileSystem {
    Mutex mtx = {};
    mz_zip_archive zip = {};
    String zip_contents = {};

    void make() { mtx.make(); }

    void trash() {
        if (zip_contents.data != nullptr) {
            mz_zip_reader_end(&zip);
            mem_free(zip_contents.data);
        }

        mtx.trash();
    }

    bool mount(String filepath) {
        PROFILE_FUNC();

        String contents = {};
        bool contents_ok = read_entire_file_raw(&contents, filepath);
        if (!contents_ok) {
            return false;
        }

        bool success = false;
        neko_defer({
            if (!success) {
                mem_free(contents.data);
            }
        });

        char *data = contents.data;
        char *end = &data[contents.len];

        constexpr i32 eocd_size = 22;
        char *eocd = end - eocd_size;
        if (read4(eocd) != 0x06054b50) {
            fprintf(stderr, "can't find EOCD record\n");
            return false;
        }

        u32 central_size = read4(&eocd[12]);
        if (read4(eocd - central_size) != 0x02014b50) {
            fprintf(stderr, "can't find central directory\n");
            return false;
        }

        u32 central_offset = read4(&eocd[16]);
        char *begin = eocd - central_size - central_offset;
        u64 zip_len = end - begin;
        if (read4(begin) != 0x04034b50) {
            fprintf(stderr, "can't read local file header\n");
            return false;
        }

        mz_bool zip_ok = mz_zip_reader_init_mem(&zip, begin, zip_len, 0);
        if (!zip_ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read zip: %s\n", mz_zip_get_error_string(err));
            return false;
        }

        zip_contents = contents;

        success = true;
        return true;
    }

    bool file_exists(String filepath) {
        PROFILE_FUNC();

        String path = to_cstr(filepath);
        neko_defer(mem_free(path.data));

        LockGuard lock{&mtx};

        i32 i = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (i == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, i, &stat);
        if (!ok) {
            return false;
        }

        return true;
    }

    bool read_entire_file(String *out, String filepath) {
        PROFILE_FUNC();

        String path = to_cstr(filepath);
        neko_defer(mem_free(path.data));

        LockGuard lock{&mtx};

        i32 file_index = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (file_index == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, file_index, &stat);
        if (!ok) {
            return false;
        }

        size_t size = stat.m_uncomp_size;
        char *buf = (char *)mem_alloc(size + 1);

        ok = mz_zip_reader_extract_to_mem(&zip, file_index, buf, size, 0);
        if (!ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read file '%s': %s\n", path.data, mz_zip_get_error_string(err));
            mem_free(buf);
            return false;
        }

        buf[size] = 0;
        *out = {buf, size};
        return true;
    }

    bool list_all_files(Array<String> *files) {
        PROFILE_FUNC();

        LockGuard lock{&mtx};

        for (u32 i = 0; i < mz_zip_reader_get_num_files(&zip); i++) {
            mz_zip_archive_file_stat file_stat;
            mz_bool ok = mz_zip_reader_file_stat(&zip, i, &file_stat);
            if (!ok) {
                return false;
            }

            String name = {file_stat.m_filename, strlen(file_stat.m_filename)};
            files->push(to_cstr(name));
        }

        return true;
    }
};

#if defined(NEKO_IS_WEB)
EM_JS(char *, web_mount_dir, (), { return stringToNewUTF8(nekoMount); });

EM_ASYNC_JS(void, web_load_zip, (), {
    var dirs = nekoMount.split("/");
    dirs.pop();

    var path = [];
    for (var dir of dirs) {
        path.push(dir);
        FS.mkdir(path.join("/"));
    }

    await fetch(nekoMount).then(async function(res) {
        if (!res.ok) {
            throw new Error("failed to fetch " + nekoMount);
        }

        var data = await res.arrayBuffer();
        FS.writeFile(nekoMount, new Uint8Array(data));
    });
});

EM_ASYNC_JS(void, web_load_files, (), {
    var jobs = [];

    function nekoWalkFiles(files, leading) {
        var path = leading.join("/");
        if (path != "") {
            FS.mkdir(path);
        }

        for (var entry of Object.entries(files)) {
            var key = entry[0];
            var value = entry[1];
            var filepath = [... leading, key ];
            if (typeof value == "object") {
                nekoWalkFiles(value, filepath);
            } else if (value == 1) {
                var file = filepath.join("/");

                var job = fetch(file).then(async function(res) {
                    if (!res.ok) {
                        throw new Error("failed to fetch " + file);
                    }
                    var data = await res.arrayBuffer();
                    FS.writeFile(file, new Uint8Array(data));
                });

                jobs.push(job);
            }
        }
    }
    nekoWalkFiles(nekoFiles, []);

    await Promise.all(jobs);
});
#endif

template <typename T>
static bool vfs_mount_type(String fsname, String mount) {
    void *ptr = mem_alloc(sizeof(T));
    T *vfs = new (ptr) T();

    vfs->make();
    bool ok = vfs->mount(mount);
    if (!ok) {
        vfs->trash();
        mem_free(vfs);
        return false;
    }

    g_filesystem_list.insert(std::make_pair(fsname, vfs));
    return true;
}

MountResult vfs_mount(const_str fsname, const char *filepath) {
    PROFILE_FUNC();

    MountResult res = {};

#if defined(NEKO_IS_WEB)
    String mount_dir = web_mount_dir();
    neko_defer(free(mount_dir.data));

    if (mount_dir.ends_with(".zip")) {
        web_load_zip();
        res.ok = vfs_mount_type<ZipFileSystem>(mount_dir);
    } else {
        web_load_files();
        res.ok = vfs_mount_type<DirectoryFileSystem>(mount_dir);
    }
#else
    if (filepath == nullptr) {
        String path = os_program_dir();

        NEKO_DEBUG_LOG("program path: %s", path.data);

        res.ok = vfs_mount_type<DirectoryFileSystem>(fsname, path);
    } else {
        String mount_dir = filepath;

        if (mount_dir.ends_with(".zip")) {
            res.ok = vfs_mount_type<ZipFileSystem>(fsname, mount_dir);
            res.is_fused = true;
        } else {
            res.ok = vfs_mount_type<DirectoryFileSystem>(fsname, mount_dir);
            res.can_hot_reload = res.ok;
        }
    }
#endif

    if (filepath != nullptr && !res.ok) {
        fatal_error(tmp_fmt("failed to load: %s", filepath));
    }

    return res;
}

void vfs_fini(std::optional<String> name) {
    auto fini_fs = []<typename T>(T fs) {
        if constexpr (!neko::is_pair<T>::value) {
            fs->trash();
            mem_free(fs);
            NEKO_DEBUG_LOG("vfs_fini(%p)", fs);
        } else {
            fs.second->trash();
            mem_free(fs.second);
            NEKO_DEBUG_LOG("vfs_fini(%s)", fs.first.data);
        }
    };
    if (!name.has_value()) {
        for (auto vfs : g_filesystem_list) fini_fs(vfs);
    } else {
        auto vfs = g_filesystem_list[name.value()];
        fini_fs(vfs);
    }
}

bool vfs_file_exists(String fsname, String filepath) { return g_filesystem_list[fsname]->file_exists(filepath); }

bool vfs_read_entire_file(String fsname, String *out, String filepath) {
    if (g_filesystem_list.find(fsname) == g_filesystem_list.end()) {
        NEKO_ERROR("failed to load vfs (%s):%s", fsname.data, filepath.data);
        return false;
    }
    return g_filesystem_list[fsname]->read_entire_file(out, filepath);
}

bool vfs_list_all_files(String fsname, Array<String> *files) { return g_filesystem_list[fsname]->list_all_files(files); }

struct AudioFile {
    u8 *buf;
    u64 cursor;
    u64 len;
};

void *vfs_for_miniaudio() {
    ma_vfs_callbacks vtbl = {};

    vtbl.onOpen = [](ma_vfs *pVFS, const char *pFilePath, ma_uint32 openMode, ma_vfs_file *pFile) -> ma_result {
        String contents = {};

        if (openMode & MA_OPEN_MODE_WRITE) {
            return MA_ERROR;
        }

        bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, pFilePath);
        if (!ok) {
            return MA_ERROR;
        }

        AudioFile *file = (AudioFile *)mem_alloc(sizeof(AudioFile));
        file->buf = (u8 *)contents.data;
        file->len = contents.len;
        file->cursor = 0;

        *pFile = file;
        return MA_SUCCESS;
    };

    vtbl.onClose = [](ma_vfs *pVFS, ma_vfs_file file) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        mem_free(f->buf);
        mem_free(f);
        return MA_SUCCESS;
    };

    vtbl.onRead = [](ma_vfs *pVFS, ma_vfs_file file, void *pDst, size_t sizeInBytes, size_t *pBytesRead) -> ma_result {
        AudioFile *f = (AudioFile *)file;

        u64 remaining = f->len - f->cursor;
        u64 len = remaining < sizeInBytes ? remaining : sizeInBytes;
        memcpy(pDst, &f->buf[f->cursor], len);

        if (pBytesRead != nullptr) {
            *pBytesRead = len;
        }

        if (len != sizeInBytes) {
            return MA_AT_END;
        }

        return MA_SUCCESS;
    };

    vtbl.onWrite = [](ma_vfs *pVFS, ma_vfs_file file, const void *pSrc, size_t sizeInBytes, size_t *pBytesWritten) -> ma_result { return MA_NOT_IMPLEMENTED; };

    vtbl.onSeek = [](ma_vfs *pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) -> ma_result {
        AudioFile *f = (AudioFile *)file;

        i64 seek = 0;
        switch (origin) {
            case ma_seek_origin_start:
                seek = offset;
                break;
            case ma_seek_origin_end:
                seek = f->len + offset;
                break;
            case ma_seek_origin_current:
            default:
                seek = f->cursor + offset;
                break;
        }

        if (seek < 0 || seek > f->len) {
            return MA_ERROR;
        }

        f->cursor = (u64)seek;
        return MA_SUCCESS;
    };

    vtbl.onTell = [](ma_vfs *pVFS, ma_vfs_file file, ma_int64 *pCursor) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        *pCursor = f->cursor;
        return MA_SUCCESS;
    };

    vtbl.onInfo = [](ma_vfs *pVFS, ma_vfs_file file, ma_file_info *pInfo) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        pInfo->sizeInBytes = f->len;
        return MA_SUCCESS;
    };

    ma_vfs_callbacks *ptr = (ma_vfs_callbacks *)mem_alloc(sizeof(ma_vfs_callbacks));
    *ptr = vtbl;
    return ptr;
}

size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf) {
    size_t bytes_to_read = size * count;
    std::memcpy(dest, static_cast<const char *>(vf->data) + vf->offset, bytes_to_read);
    vf->offset += bytes_to_read;
    return count;
}

// #define SEEK_SET 0
// #define SEEK_CUR 1
// #define SEEK_END 2

int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence) {
    u64 new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = of;
            break;
        case SEEK_CUR:
            new_offset = vf->offset + of;
            break;
        case SEEK_END:
            new_offset = vf->len + of;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    if (new_offset < 0 || new_offset > vf->len) {
        errno = EINVAL;
        return -1;
    }
    vf->offset = new_offset;
    return 0;
}

u64 neko_capi_vfs_ftell(vfs_file *vf) { return vf->offset; }

vfs_file neko_capi_vfs_fopen(const_str path) {
    vfs_file vf{};
    vf.data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, path, &vf.len);
    return vf;
}

int neko_capi_vfs_fclose(vfs_file *vf) {
    NEKO_ASSERT(vf);
    mem_free(vf->data);
    return 0;
}

bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath) { return vfs_file_exists(fsname, filepath); }

const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size) {
    String out;
    bool ok = vfs_read_entire_file(fsname, &out, filepath);
    if (!ok) return NULL;
    *size = out.len;
    return out.data;
}

enum JSONTok : i32 {
    JSONTok_Invalid,
    JSONTok_LBrace,    // {
    JSONTok_RBrace,    // }
    JSONTok_LBracket,  // [
    JSONTok_RBracket,  // ]
    JSONTok_Colon,     // :
    JSONTok_Comma,     // ,
    JSONTok_True,      // true
    JSONTok_False,     // false
    JSONTok_Null,      // null
    JSONTok_String,    // "[^"]*"
    JSONTok_Number,    // [0-9]+\.?[0-9]*
    JSONTok_Error,
    JSONTok_EOF,
};

const char *json_tok_string(JSONTok tok) {
    switch (tok) {
        case JSONTok_Invalid:
            return "Invalid";
        case JSONTok_LBrace:
            return "LBrace";
        case JSONTok_RBrace:
            return "RBrace";
        case JSONTok_LBracket:
            return "LBracket";
        case JSONTok_RBracket:
            return "RBracket";
        case JSONTok_Colon:
            return "Colon";
        case JSONTok_Comma:
            return "Comma";
        case JSONTok_True:
            return "True";
        case JSONTok_False:
            return "False";
        case JSONTok_Null:
            return "Null";
        case JSONTok_String:
            return "String";
        case JSONTok_Number:
            return "Number";
        case JSONTok_Error:
            return "Error";
        case JSONTok_EOF:
            return "EOF";
        default:
            return "?";
    }
}

const char *json_kind_string(JSONKind kind) {
    switch (kind) {
        case JSONKind_Null:
            return "Null";
        case JSONKind_Object:
            return "Object";
        case JSONKind_Array:
            return "Array";
        case JSONKind_String:
            return "String";
        case JSONKind_Number:
            return "Number";
        case JSONKind_Boolean:
            return "Boolean";
        default:
            return "?";
    }
};

struct JSONToken {
    JSONTok kind;
    String str;
    u32 line;
    u32 column;
};

struct JSONScanner {
    String contents;
    JSONToken token;
    u64 begin;
    u64 end;
    u32 line;
    u32 column;
};

static char json_peek(JSONScanner *scan, u64 offset) { return scan->contents.data[scan->end + offset]; }

static bool json_at_end(JSONScanner *scan) { return scan->end == scan->contents.len; }

static void json_next_char(JSONScanner *scan) {
    if (!json_at_end(scan)) {
        scan->end++;
        scan->column++;
    }
}

static void json_skip_whitespace(JSONScanner *scan) {
    while (true) {
        switch (json_peek(scan, 0)) {
            case '\n':
                scan->column = 0;
                scan->line++;
            case ' ':
            case '\t':
            case '\r':
                json_next_char(scan);
                break;
            default:
                return;
        }
    }
}

static String json_lexeme(JSONScanner *scan) { return scan->contents.substr(scan->begin, scan->end); }

static JSONToken json_make_tok(JSONScanner *scan, JSONTok kind) {
    JSONToken t = {};
    t.kind = kind;
    t.str = json_lexeme(scan);
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_err_tok(JSONScanner *scan, String msg) {
    JSONToken t = {};
    t.kind = JSONTok_Error;
    t.str = msg;
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_scan_ident(Arena *a, JSONScanner *scan) {
    while (is_alpha(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    JSONToken t = {};
    t.str = json_lexeme(scan);

    if (t.str == "true") {
        t.kind = JSONTok_True;
    } else if (t.str == "false") {
        t.kind = JSONTok_False;
    } else if (t.str == "null") {
        t.kind = JSONTok_Null;
    } else {
        StringBuilder sb = {};
        neko_defer(sb.trash());

        String s = String(sb << "unknown identifier: '" << t.str << "'");
        return json_err_tok(scan, a->bump_string(s));
    }

    scan->token = t;
    return t;
}

static JSONToken json_scan_number(JSONScanner *scan) {
    if (json_peek(scan, 0) == '-' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '-'
    }

    while (is_digit(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    if (json_peek(scan, 0) == '.' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '.'

        while (is_digit(json_peek(scan, 0))) {
            json_next_char(scan);
        }
    }

    return json_make_tok(scan, JSONTok_Number);
}

static JSONToken json_scan_string(JSONScanner *scan) {
    while (json_peek(scan, 0) != '"' && !json_at_end(scan)) {
        json_next_char(scan);
    }

    if (json_at_end(scan)) {
        return json_err_tok(scan, "unterminated string");
    }

    json_next_char(scan);
    return json_make_tok(scan, JSONTok_String);
}

static JSONToken json_scan_next(Arena *a, JSONScanner *scan) {
    json_skip_whitespace(scan);

    scan->begin = scan->end;

    if (json_at_end(scan)) {
        return json_make_tok(scan, JSONTok_EOF);
    }

    char c = json_peek(scan, 0);
    json_next_char(scan);

    if (is_alpha(c)) {
        return json_scan_ident(a, scan);
    }

    if (is_digit(c) || (c == '-' && is_digit(json_peek(scan, 0)))) {
        return json_scan_number(scan);
    }

    if (c == '"') {
        return json_scan_string(scan);
    }

    switch (c) {
        case '{':
            return json_make_tok(scan, JSONTok_LBrace);
        case '}':
            return json_make_tok(scan, JSONTok_RBrace);
        case '[':
            return json_make_tok(scan, JSONTok_LBracket);
        case ']':
            return json_make_tok(scan, JSONTok_RBracket);
        case ':':
            return json_make_tok(scan, JSONTok_Colon);
        case ',':
            return json_make_tok(scan, JSONTok_Comma);
    }

    String msg = tmp_fmt("unexpected character: '%c' (%d)", c, (int)c);
    String s = a->bump_string(msg);
    return json_err_tok(scan, s);
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out);

static String json_parse_object(Arena *a, JSONScanner *scan, JSONObject **out) {
    PROFILE_FUNC();

    JSONObject *obj = nullptr;

    json_scan_next(a, scan);  // eat brace

    while (true) {
        if (scan->token.kind == JSONTok_RBrace) {
            *out = obj;
            json_scan_next(a, scan);
            return {};
        }

        String err = {};

        JSON key = {};
        err = json_parse_next(a, scan, &key);
        if (err.data != nullptr) {
            return err;
        }

        if (key.kind != JSONKind_String) {
            String msg = tmp_fmt("expected string as object key on line: %d. got: %s", (i32)scan->token.line, json_kind_string(key.kind));
            return a->bump_string(msg);
        }

        if (scan->token.kind != JSONTok_Colon) {
            String msg = tmp_fmt("expected colon on line: %d. got %s", (i32)scan->token.line, json_tok_string(scan->token.kind));
            return a->bump_string(msg);
        }

        json_scan_next(a, scan);

        JSON value = {};
        err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONObject *entry = (JSONObject *)a->bump(sizeof(JSONObject));
        entry->next = obj;
        entry->hash = fnv1a(key.string);
        entry->key = key.string;
        entry->value = value;

        obj = entry;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static String json_parse_array(Arena *a, JSONScanner *scan, JSONArray **out) {
    PROFILE_FUNC();

    JSONArray *arr = nullptr;

    json_scan_next(a, scan);  // eat bracket

    while (true) {
        if (scan->token.kind == JSONTok_RBracket) {
            *out = arr;
            json_scan_next(a, scan);
            return {};
        }

        JSON value = {};
        String err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONArray *el = (JSONArray *)a->bump(sizeof(JSONArray));
        el->next = arr;
        el->value = value;
        el->index = 0;

        if (arr != nullptr) {
            el->index = arr->index + 1;
        }

        arr = el;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out) {
    switch (scan->token.kind) {
        case JSONTok_LBrace: {
            out->kind = JSONKind_Object;
            return json_parse_object(a, scan, &out->object);
        }
        case JSONTok_LBracket: {
            out->kind = JSONKind_Array;
            return json_parse_array(a, scan, &out->array);
        }
        case JSONTok_String: {
            out->kind = JSONKind_String;
            out->string = scan->token.str.substr(1, scan->token.str.len - 1);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Number: {
            out->kind = JSONKind_Number;
            out->number = string_to_double(scan->token.str);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_True: {
            out->kind = JSONKind_Boolean;
            out->boolean = true;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_False: {
            out->kind = JSONKind_Boolean;
            out->boolean = false;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Null: {
            out->kind = JSONKind_Null;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Error: {
            StringBuilder sb = {};
            neko_defer(sb.trash());

            sb << scan->token.str << tmp_fmt(" on line %d:%d", (i32)scan->token.line, (i32)scan->token.column);

            return a->bump_string(String(sb));
        }
        default: {
            String msg = tmp_fmt("unknown json token: %s on line %d:%d", json_tok_string(scan->token.kind), (i32)scan->token.line, (i32)scan->token.column);
            return a->bump_string(msg);
        }
    }
}

void JSONDocument::parse(String contents) {
    PROFILE_FUNC();

    arena = {};

    JSONScanner scan = {};
    scan.contents = contents;
    scan.line = 1;

    json_scan_next(&arena, &scan);

    String err = json_parse_next(&arena, &scan, &root);
    if (err.data != nullptr) {
        error = err;
        return;
    }

    if (scan.token.kind != JSONTok_EOF) {
        error = "expected EOF";
        return;
    }
}

void JSONDocument::trash() {
    PROFILE_FUNC();
    arena.trash();
}

JSON JSON::lookup(String key, bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        for (JSONObject *o = object; o != nullptr; o = o->next) {
            if (o->hash == fnv1a(key)) {
                return o->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSON JSON::index(i32 i, bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        for (JSONArray *a = array; a != nullptr; a = a->next) {
            if (a->index == i) {
                return a->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSONObject *JSON::as_object(bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        return object;
    }

    *ok = false;
    return {};
}

JSONArray *JSON::as_array(bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        return array;
    }

    *ok = false;
    return {};
}

String JSON::as_string(bool *ok) {
    if (*ok && kind == JSONKind_String) {
        return string;
    }

    *ok = false;
    return {};
}

double JSON::as_number(bool *ok) {
    if (*ok && kind == JSONKind_Number) {
        return number;
    }

    *ok = false;
    return {};
}

JSONObject *JSON::lookup_object(String key, bool *ok) { return lookup(key, ok).as_object(ok); }

JSONArray *JSON::lookup_array(String key, bool *ok) { return lookup(key, ok).as_array(ok); }

String JSON::lookup_string(String key, bool *ok) { return lookup(key, ok).as_string(ok); }

double JSON::lookup_number(String key, bool *ok) { return lookup(key, ok).as_number(ok); }

double JSON::index_number(i32 i, bool *ok) { return index(i, ok).as_number(ok); }

static void json_write_string(StringBuilder &sb, JSON *json, i32 level) {
    switch (json->kind) {
        case JSONKind_Object: {
            sb << "{\n";
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                sb.concat("  ", level);
                sb << o->key;
                json_write_string(sb, &o->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "}";
            break;
        }
        case JSONKind_Array: {
            sb << "[\n";
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                sb.concat("  ", level);
                json_write_string(sb, &a->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "]";
            break;
        }
        case JSONKind_String:
            sb << "\"" << json->string << "\"";
            break;
        case JSONKind_Number:
            sb << tmp_fmt("%g", json->number);
            break;
        case JSONKind_Boolean:
            sb << (json->boolean ? "true" : "false");
            break;
        case JSONKind_Null:
            sb << "null";
            break;
        default:
            break;
    }
}

void json_write_string(StringBuilder *sb, JSON *json) { json_write_string(*sb, json, 1); }

void json_print(JSON *json) {
    StringBuilder sb = {};
    neko_defer(sb.trash());
    json_write_string(&sb, json);
    neko_println("%s", sb.data);
}

void json_to_lua(lua_State *L, JSON *json) {
    switch (json->kind) {
        case JSONKind_Object: {
            lua_newtable(L);
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                lua_pushlstring(L, o->key.data, o->key.len);
                json_to_lua(L, &o->value);
                lua_rawset(L, -3);
            }
            break;
        }
        case JSONKind_Array: {
            lua_newtable(L);
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                json_to_lua(L, &a->value);
                lua_rawseti(L, -2, a->index + 1);
            }
            break;
        }
        case JSONKind_String: {
            lua_pushlstring(L, json->string.data, json->string.len);
            break;
        }
        case JSONKind_Number: {
            lua_pushnumber(L, json->number);
            break;
        }
        case JSONKind_Boolean: {
            lua_pushboolean(L, json->boolean);
            break;
        }
        case JSONKind_Null: {
            lua_pushnil(L);
            break;
        }
        default:
            break;
    }
}

static void lua_to_json_string(StringBuilder &sb, lua_State *L, HashMap<bool> *visited, String *err, i32 width, i32 level) {
    auto indent = [&](i32 offset) {
        if (width > 0) {
            sb << "\n";
            sb.concat(" ", width * (level + offset));
        }
    };

    if (err->len != 0) {
        return;
    }

    i32 top = lua_gettop(L);
    switch (lua_type(L, top)) {
        case LUA_TTABLE: {
            uintptr_t ptr = (uintptr_t)lua_topointer(L, top);

            bool *visit = nullptr;
            bool exist = visited->find_or_insert(ptr, &visit);
            if (exist && *visit) {
                *err = "table has cycles";
                return;
            }

            *visit = true;

            lua_pushnil(L);
            if (lua_next(L, -2) == 0) {
                sb << "[]";
                return;
            }

            i32 key_type = lua_type(L, -2);

            if (key_type == LUA_TNUMBER) {
                sb << "[";

                indent(0);
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                i32 len = luax_len(L, top);
                assert(len > 0);
                i32 i = 1;
                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TNUMBER) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be numbers";
                        return;
                    }

                    sb << ",";
                    indent(0);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    i++;
                }
                indent(-1);
                sb << "]";

                if (i != len) {
                    *err = "array is not continuous";
                    return;
                }
            } else if (key_type == LUA_TSTRING) {
                sb << "{";
                indent(0);

                lua_pushvalue(L, -2);
                lua_to_json_string(sb, L, visited, err, width, level + 1);
                lua_pop(L, 1);
                sb << ":";
                if (width > 0) {
                    sb << " ";
                }
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TSTRING) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be strings";
                        return;
                    }

                    sb << ",";
                    indent(0);

                    lua_pushvalue(L, -2);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    lua_pop(L, 1);
                    sb << ":";
                    if (width > 0) {
                        sb << " ";
                    }
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                }
                indent(-1);
                sb << "}";
            } else {
                lua_pop(L, 2);  // key, value
                *err = "expected table keys to be strings or numbers";
                return;
            }

            visited->unset(ptr);
            break;
        }
        case LUA_TNIL:
            sb << "null";
            break;
        case LUA_TNUMBER:
            sb << tmp_fmt("%g", lua_tonumber(L, top));
            break;
        case LUA_TSTRING:
            sb << "\"" << luax_check_string(L, top) << "\"";
            break;
        case LUA_TBOOLEAN:
            sb << (lua_toboolean(L, top) ? "true" : "false");
            break;
        default:
            *err = "type is not serializable";
    }
}

String lua_to_json_string(lua_State *L, i32 arg, String *contents, i32 width) {
    StringBuilder sb = {};

    HashMap<bool> visited = {};
    neko_defer(visited.trash());

    String err = {};
    lua_pushvalue(L, arg);
    lua_to_json_string(sb, L, &visited, &err, width, 1);
    lua_pop(L, 1);

    if (err.len != 0) {
        sb.trash();
    }

    *contents = String(sb);
    return err;
}

/*==========================
// LZ77
==========================*/

enum {
    NEKO_LZ_EXCESS = 16,

    NEKO_LZ_WINDOW_BITS = 17,  // Hard-coded
    NEKO_LZ_WINDOW_SIZE = 1 << NEKO_LZ_WINDOW_BITS,
    NEKO_LZ_WINDOW_MASK = NEKO_LZ_WINDOW_SIZE - 1,

    NEKO_LZ_MIN_MATCH = 4,

    NEKO_LZ_HASH_BITS = 19,
    NEKO_LZ_HASH_SIZE = 1 << NEKO_LZ_HASH_BITS,
    NEKO_LZ_NIL = -1,
};

typedef struct NEKO_LZ_WORKMEM {
    int HashTable[NEKO_LZ_HASH_SIZE];
    int Prev[NEKO_LZ_WINDOW_SIZE];
} NEKO_LZ_WORKMEM;

// Utils

static inline u16 UnalignedLoad16(const void *p) { return *(const u16 *)(p); }
static inline u32 UnalignedLoad32(const void *p) { return *(const u32 *)(p); }
static inline void UnalignedStore16(void *p, u16 x) { *(u16 *)(p) = x; }
static inline void UnalignedCopy64(void *d, const void *s) { *(u64 *)(d) = *(const u64 *)(s); }

static inline void __neko_ulz_wild_copy(u8 *d, const u8 *s, int n) {
    UnalignedCopy64(d, s);

    for (int i = 8; i < n; i += 8) UnalignedCopy64(d + i, s + i);
}

static inline u32 __neko_ulz_hash32(const void *p) { return (UnalignedLoad32(p) * 0x9E3779B9) >> (32 - NEKO_LZ_HASH_BITS); }

static inline void __neko_ulz_encode_mod(u8 **p, u32 x) {
    while (x >= 128) {
        x -= 128;
        *(*p)++ = 128 + (x & 127);
        x >>= 7;
    }
    *(*p)++ = x;
}

static inline u32 __neko_ulz_decode_mod(const u8 **p) {
    u32 x = 0;
    for (int i = 0; i <= 21; i += 7) {
        const u32 c = *(*p)++;
        x += c << i;
        if (c < 128) break;
    }
    return x;
}

// LZ77

static int __neko_ulz_compress_fast(const u8 *in, int inlen, u8 *out, int outlen) {
    NEKO_LZ_WORKMEM *u = (NEKO_LZ_WORKMEM *)mem_realloc(0, sizeof(NEKO_LZ_WORKMEM));

    for (int i = 0; i < NEKO_LZ_HASH_SIZE; ++i) u->HashTable[i] = NEKO_LZ_NIL;

    u8 *op = out;
    int anchor = 0;

    int p = 0;
    while (p < inlen) {
        int best_len = 0;
        int dist = 0;

        const int max_match = inlen - p;
        if (max_match >= NEKO_LZ_MIN_MATCH) {
            const int limit = (p - NEKO_LZ_WINDOW_SIZE) > NEKO_LZ_NIL ? (p - NEKO_LZ_WINDOW_SIZE) : NEKO_LZ_NIL;

            const u32 h = __neko_ulz_hash32(&in[p]);
            int s = u->HashTable[h];
            u->HashTable[h] = p;

            if (s > limit && UnalignedLoad32(&in[s]) == UnalignedLoad32(&in[p])) {
                int len = NEKO_LZ_MIN_MATCH;
                while (len < max_match && in[s + len] == in[p + len]) ++len;

                best_len = len;
                dist = p - s;
            }
        }

        if (best_len == NEKO_LZ_MIN_MATCH && (p - anchor) >= (7 + 128)) best_len = 0;

        if (best_len >= NEKO_LZ_MIN_MATCH) {
            const int len = best_len - NEKO_LZ_MIN_MATCH;
            const int token = ((dist >> 12) & 16) + (len < 15 ? len : 15);

            if (anchor != p) {
                const int run = p - anchor;
                if (run >= 7) {
                    *op++ = (7 << 5) + token;
                    __neko_ulz_encode_mod(&op, run - 7);
                } else
                    *op++ = (run << 5) + token;

                __neko_ulz_wild_copy(op, &in[anchor], run);
                op += run;
            } else
                *op++ = token;

            if (len >= 15) __neko_ulz_encode_mod(&op, len - 15);

            UnalignedStore16(op, dist);
            op += 2;

            anchor = p + best_len;
            ++p;
            u->HashTable[__neko_ulz_hash32(&in[p])] = p++;
            u->HashTable[__neko_ulz_hash32(&in[p])] = p++;
            u->HashTable[__neko_ulz_hash32(&in[p])] = p++;
            p = anchor;
        } else
            ++p;
    }

    if (anchor != p) {
        const int run = p - anchor;
        if (run >= 7) {
            *op++ = 7 << 5;
            __neko_ulz_encode_mod(&op, run - 7);
        } else
            *op++ = run << 5;

        __neko_ulz_wild_copy(op, &in[anchor], run);
        op += run;
    }

    void *gc = mem_realloc(u, 0);
    return op - out;
}

static int __neko_ulz_compress(const u8 *in, int inlen, u8 *out, int outlen, int level) {
    if (level < 1 || level > 9) return 0;
    const int max_chain = (level < 9) ? 1 << level : 1 << 13;

    NEKO_LZ_WORKMEM *u = (NEKO_LZ_WORKMEM *)mem_realloc(0, sizeof(NEKO_LZ_WORKMEM));
    for (int i = 0; i < NEKO_LZ_HASH_SIZE; ++i) u->HashTable[i] = NEKO_LZ_NIL;

    u8 *op = out;
    int anchor = 0;

    int p = 0;
    while (p < inlen) {
        int best_len = 0;
        int dist = 0;

        const int max_match = inlen - p;
        if (max_match >= NEKO_LZ_MIN_MATCH) {
            const int limit = (p - NEKO_LZ_WINDOW_SIZE) > NEKO_LZ_NIL ? (p - NEKO_LZ_WINDOW_SIZE) : NEKO_LZ_NIL;
            int chainlen = max_chain;

            int s = u->HashTable[__neko_ulz_hash32(&in[p])];
            while (s > limit) {
                if (in[s + best_len] == in[p + best_len] && UnalignedLoad32(&in[s]) == UnalignedLoad32(&in[p])) {
                    int len = NEKO_LZ_MIN_MATCH;
                    while (len < max_match && in[s + len] == in[p + len]) ++len;

                    if (len > best_len) {
                        best_len = len;
                        dist = p - s;

                        if (len == max_match) break;
                    }
                }

                if (--chainlen == 0) break;

                s = u->Prev[s & NEKO_LZ_WINDOW_MASK];
            }
        }

        if (best_len == NEKO_LZ_MIN_MATCH && (p - anchor) >= (7 + 128)) best_len = 0;

        if (level >= 5 && best_len >= NEKO_LZ_MIN_MATCH && best_len < max_match && (p - anchor) != 6) {
            const int x = p + 1;
            const int target_len = best_len + 1;

            const int limit = (x - NEKO_LZ_WINDOW_SIZE) > NEKO_LZ_NIL ? (x - NEKO_LZ_WINDOW_SIZE) : NEKO_LZ_NIL;
            int chainlen = max_chain;

            int s = u->HashTable[__neko_ulz_hash32(&in[x])];
            while (s > limit) {
                if (in[s + best_len] == in[x + best_len] && UnalignedLoad32(&in[s]) == UnalignedLoad32(&in[x])) {
                    int len = NEKO_LZ_MIN_MATCH;
                    while (len < target_len && in[s + len] == in[x + len]) ++len;

                    if (len == target_len) {
                        best_len = 0;
                        break;
                    }
                }

                if (--chainlen == 0) break;

                s = u->Prev[s & NEKO_LZ_WINDOW_MASK];
            }
        }

        if (best_len >= NEKO_LZ_MIN_MATCH) {
            const int len = best_len - NEKO_LZ_MIN_MATCH;
            const int token = ((dist >> 12) & 16) + (len < 15 ? len : 15);

            if (anchor != p) {
                const int run = p - anchor;
                if (run >= 7) {
                    *op++ = (7 << 5) + token;
                    __neko_ulz_encode_mod(&op, run - 7);
                } else
                    *op++ = (run << 5) + token;

                __neko_ulz_wild_copy(op, &in[anchor], run);
                op += run;
            } else
                *op++ = token;

            if (len >= 15) __neko_ulz_encode_mod(&op, len - 15);

            UnalignedStore16(op, dist);
            op += 2;

            while (best_len-- != 0) {
                const u32 h = __neko_ulz_hash32(&in[p]);
                u->Prev[p & NEKO_LZ_WINDOW_MASK] = u->HashTable[h];
                u->HashTable[h] = p++;
            }
            anchor = p;
        } else {
            const u32 h = __neko_ulz_hash32(&in[p]);
            u->Prev[p & NEKO_LZ_WINDOW_MASK] = u->HashTable[h];
            u->HashTable[h] = p++;
        }
    }

    if (anchor != p) {
        const int run = p - anchor;
        if (run >= 7) {
            *op++ = 7 << 5;
            __neko_ulz_encode_mod(&op, run - 7);
        } else
            *op++ = run << 5;

        __neko_ulz_wild_copy(op, &in[anchor], run);
        op += run;
    }

    void *gc = mem_realloc(u, 0);
    return op - out;
}

static int __neko_ulz_decompress(const u8 *in, int inlen, u8 *out, int outlen) {
    u8 *op = out;
    const u8 *ip = in;
    const u8 *ip_end = ip + inlen;
    const u8 *op_end = op + outlen;

    while (ip < ip_end) {
        const int token = *ip++;

        if (token >= 32) {
            int run = token >> 5;
            if (run == 7) run += __neko_ulz_decode_mod(&ip);
            if ((op_end - op) < run || (ip_end - ip) < run)  // Overrun check
                return 0;

            __neko_ulz_wild_copy(op, ip, run);
            op += run;
            ip += run;
            if (ip >= ip_end) break;
        }

        int len = (token & 15) + NEKO_LZ_MIN_MATCH;
        if (len == (15 + NEKO_LZ_MIN_MATCH)) len += __neko_ulz_decode_mod(&ip);
        if ((op_end - op) < len)  // Overrun check
            return 0;

        const int dist = ((token & 16) << 12) + UnalignedLoad16(ip);
        ip += 2;
        u8 *cp = op - dist;
        if ((op - out) < dist)  // Range check
            return 0;

        if (dist >= 8) {
            __neko_ulz_wild_copy(op, cp, len);
            op += len;
        } else {
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            while (len-- != 4) *op++ = *cp++;
        }
    }

    return (ip == ip_end) ? op - out : 0;
}

u32 neko_lz_encode(const void *in, u32 inlen, void *out, u32 outlen, u32 flags) {
    int level = flags > 9 ? 9 : flags < 0 ? 0 : flags;  // [0..(6)..9]
    int rc = level ? __neko_ulz_compress((u8 *)in, (int)inlen, (u8 *)out, (int)outlen, level) : __neko_ulz_compress_fast((u8 *)in, (int)inlen, (u8 *)out, (int)outlen);
    return (u32)rc;
}
u32 neko_lz_decode(const void *in, u32 inlen, void *out, u32 outlen) { return (u32)__neko_ulz_decompress((u8 *)in, (int)inlen, (u8 *)out, (int)outlen); }
u32 neko_lz_bounds(u32 inlen, u32 flags) { return inlen + inlen / 255 + 16; }

/*==========================
// NEKO_PACK
==========================*/

#pragma region packer

// #include "deps/lz4/lz4.h"

static void destroy_pack_items(u64 item_count, neko_pak::item *items) {
    NEKO_ASSERT(item_count == 0 || (item_count > 0 && items));

    for (u64 i = 0; i < item_count; i++) mem_free(items[i].path);
    mem_free(items);
}

bool create_pack_items(vfs_file *packFile, u64 item_count, neko_pak::item **_items) {
    NEKO_ASSERT(packFile);
    NEKO_ASSERT(item_count > 0);
    NEKO_ASSERT(_items);

    neko_pak::item *items = (neko_pak::item *)mem_alloc(item_count * sizeof(neko_pak::item));

    if (!items) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    for (u64 i = 0; i < item_count; i++) {
        neko_pak::iteminfo info;

        size_t result = neko_capi_vfs_fread(&info, sizeof(neko_pak::iteminfo), 1, packFile);

        if (result != 1) {
            destroy_pack_items(i, items);
            return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
        }

        if (info.data_size == 0 || info.path_size == 0) {
            destroy_pack_items(i, items);
            return false; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        char *path = (char *)mem_alloc((info.path_size + 1) * sizeof(char));

        if (!path) {
            destroy_pack_items(i, items);
            return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
        }

        result = neko_capi_vfs_fread(path, sizeof(char), info.path_size, packFile);

        path[info.path_size] = 0;

        if (result != info.path_size) {
            destroy_pack_items(i, items);
            return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
        }

        i64 fileOffset = info.zip_size > 0 ? info.zip_size : info.data_size;

        int seekResult = neko_capi_vfs_fseek(packFile, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            destroy_pack_items(i, items);
            return false; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        neko_pak::item *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return true;
}

bool neko_pak::load(const_str file_path, u32 data_buffer_capacity, bool is_resources_directory) {
    NEKO_ASSERT(file_path);

    memset(this, 0, sizeof(neko_pak));

    this->zip_buffer = NULL;
    this->zip_size = 0;

    this->vf = neko_capi_vfs_fopen(file_path);

    if (!this->vf.data) {
        this->fini();
        return false; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
    }

    char header[neko_pak_head_size];
    i32 buildnum;

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), neko_pak_head_size, &this->vf);
    result += neko_capi_vfs_fread(&buildnum, sizeof(i32), 1, &this->vf);

    // 检查文件头大小
    if (result != neko_pak_head_size + 1) {
        this->fini();
        return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    // 检查文件头
    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K' || header[4] != 0 || header[5] != 0) {
        this->fini();
        return false;
    }

    // Skipping PATCH version check
    if (header[7] != !neko_little_endian) {
        this->fini();
        return false; /*BAD_FILE_ENDIANNESS_PACK_RESULT*/
    }

    u64 item_count;

    result = neko_capi_vfs_fread(&item_count, sizeof(u64), 1, &this->vf);

    if (result != 1) {
        this->fini();
        return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    if (item_count == 0) {
        this->fini();
        return false; /*BAD_DATA_SIZE_PACK_RESULT*/
    }

    neko_pak::item *items;

    bool ok = create_pack_items(&this->vf, item_count, &items);

    if (!ok) {
        this->fini();
        return false;
    }

    this->item_count = item_count;
    this->items = items;

    u8 *data_buffer;

    if (data_buffer_capacity > 0) {
        data_buffer = (u8 *)mem_alloc(data_buffer_capacity * sizeof(u8));

        if (!data_buffer) {
            this->fini();
            return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
        }
    } else {
        data_buffer = NULL;
    }

    this->data_buffer = data_buffer;
    this->data_size = data_buffer_capacity;

    NEKO_TRACE("load pack %s buildnum: %d (engine %d)", neko_util_get_filename(file_path), buildnum, neko_buildnum());

    return true;
}
void neko_pak::fini() {
    if (this->file_ref_count != 0) {
        NEKO_WARN("assets loader leaks detected %d refs", this->file_ref_count);
    }

    free_buffer();

    destroy_pack_items(this->item_count, this->items);
    if (this->vf.data) neko_capi_vfs_fclose(&this->vf);
}

static int neko_compare_pack_items(const void *_a, const void *_b) {
    const neko_pak::item *a = (neko_pak::item *)_a;
    const neko_pak::item *b = (neko_pak::item *)_b;

    int difference = (int)a->info.path_size - (int)b->info.path_size;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

u64 neko_pak::get_item_index(const_str path) {
    NEKO_ASSERT(path);
    NEKO_ASSERT(strlen(path) <= UINT8_MAX);

    neko_pak::item *search_item = &this->search_item;

    search_item->info.path_size = (u8)strlen(path);
    search_item->path = (char *)path;

    neko_pak::item *items = (neko_pak::item *)bsearch(search_item, this->items, this->item_count, sizeof(neko_pak::item), neko_compare_pack_items);

    if (!items) return u64_max;

    u64 index = items - this->items;
    return index;
}

bool neko_pak::get_data(u64 index, const u8 **data, u32 *size) {
    NEKO_ASSERT(index < this->item_count);
    NEKO_ASSERT(data);
    NEKO_ASSERT(size);

    neko_pak::iteminfo info = this->items[index].info;
    u8 *_data_buffer = this->data_buffer;

    if (_data_buffer) {
        if (info.data_size > this->data_size) {
            _data_buffer = (u8 *)mem_realloc(_data_buffer, info.data_size * sizeof(u8));

            if (!_data_buffer) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            this->data_buffer = _data_buffer;
            this->data_size = info.data_size;
        }
    } else {
        _data_buffer = (u8 *)mem_alloc(info.data_size * sizeof(u8));

        if (!_data_buffer) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

        this->data_buffer = _data_buffer;
        this->data_size = info.data_size;
    }

    u8 *zip_buffer = this->zip_buffer;

    if (zip_buffer) {
        if (info.zip_size > this->zip_size) {
            zip_buffer = (u8 *)mem_realloc(zip_buffer, info.zip_size * sizeof(u8));

            if (!zip_buffer) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            this->zip_buffer = zip_buffer;
            this->zip_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            zip_buffer = (u8 *)mem_alloc(info.zip_size * sizeof(u8));

            if (!zip_buffer) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            this->zip_buffer = zip_buffer;
            this->zip_size = info.zip_size;
        }
    }

    vfs_file *vf = &this->vf;

    i64 file_offset = (i64)(info.file_offset + sizeof(neko_pak::iteminfo) + info.path_size);

    int seek_result = neko_capi_vfs_fseek(vf, file_offset, SEEK_SET);

    if (seek_result != 0) return false; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/

    if (info.zip_size > 0) {
        size_t result = neko_capi_vfs_fread(zip_buffer, sizeof(u8), info.zip_size, vf);

        if (result != info.zip_size) return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/

        result = neko_lz_decode(zip_buffer, info.zip_size, _data_buffer, info.data_size);

        NEKO_TRACE("[assets] neko_lz_decode %u %u", info.zip_size, info.data_size);

        if (result < 0 || result != info.data_size) {
            return false; /*FAILED_TO_DECOMPRESS_PACK_RESULT*/
        }
    } else {
        size_t result = neko_capi_vfs_fread(_data_buffer, sizeof(u8), info.data_size, vf);

        if (result != info.data_size) return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    (*size) = info.data_size;
    (*data) = (u8 *)mem_alloc(info.data_size);
    memcpy((void *)(*data), _data_buffer, info.data_size);
    this->file_ref_count++;
    return true;
}

bool neko_pak::get_data(const_str path, const u8 **data, u32 *size) {
    NEKO_ASSERT(path);
    NEKO_ASSERT(data);
    NEKO_ASSERT(size);
    NEKO_ASSERT(strlen(path) <= UINT8_MAX);
    u64 index = this->get_item_index(path);
    if (index == u64_max) return false; /*FAILED_TO_GET_ITEM_PACK_RESULT*/
    return this->get_data(index, data, size);
}

void neko_pak::free_item(void *data) {
    mem_free(data);
    this->file_ref_count--;
}

void neko_pak::free_buffer() {
    mem_free(this->data_buffer);
    mem_free(this->zip_buffer);
    this->data_buffer = NULL;
    this->zip_buffer = NULL;
}

static void neko_pak_remove_item(u64 item_count, neko_pak::item *pack_items) {
    NEKO_ASSERT(item_count == 0 || (item_count > 0 && pack_items));
    for (u64 i = 0; i < item_count; i++) remove(pack_items[i].path);
}

bool neko_pak_unzip(const_str file_path, bool print_progress) {
    NEKO_ASSERT(file_path);

    neko_pak pak;

    int pack_result = pak.load(file_path, 128, false);

    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;

    u64 item_count = pak.item_count;
    neko_pak::item *items = pak.items;

    for (u64 i = 0; i < item_count; i++) {
        neko_pak::item *item = &items[i];

        if (print_progress) {
            NEKO_INFO("Unpacking %s", item->path);
        }

        const u8 *data_buffer;
        u32 data_size;

        pack_result = pak.get_data(i, &data_buffer, &data_size);

        if (pack_result != 0) {
            neko_pak_remove_item(i, items);
            pak.fini();
            return pack_result;
        }

        u8 path_size = item->info.path_size;

        char item_path[UINT8_MAX + 1];

        memcpy(item_path, item->path, path_size * sizeof(char));
        item_path[path_size] = 0;

        // 解压的时候路径节替换为 '-'
        for (u8 j = 0; j < path_size; j++)
            if (item_path[j] == '/' || item_path[j] == '\\' || (item_path[j] == '.' && j == 0)) item_path[j] = '-';

        FILE *item_file = neko_fopen(item_path, "wb");

        if (!item_file) {
            neko_pak_remove_item(i, items);
            pak.fini();
            return false; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        size_t result = fwrite(data_buffer, sizeof(u8), data_size, item_file);

        neko_fclose(item_file);

        if (result != data_size) {
            neko_pak_remove_item(i, items);
            pak.fini();
            return false; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        if (print_progress) {
            u32 raw_file_size = item->info.data_size;
            u32 zip_file_size = item->info.zip_size > 0 ? item->info.zip_size : item->info.data_size;

            total_raw_size += raw_file_size;
            total_zip_size += zip_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            neko_printf("(%u/%u bytes) [%d%%]\n", raw_file_size, zip_file_size, progress);
        }
    }

    pak.fini();

    if (print_progress) {
        neko_printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)total_raw_size, (long long unsigned int)total_zip_size);
    }

    return true;
}

bool neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, bool print_progress) {
    NEKO_ASSERT(pack_file);
    NEKO_ASSERT(item_count > 0);
    NEKO_ASSERT(item_paths);

    u32 buffer_size = 128;  // 提高初始缓冲大小

    u8 *item_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);
    if (!item_data) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    u8 *zip_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);
    if (!zip_data) {
        mem_free(item_data);
        return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
    }

    u64 total_zip_size = 0, total_raw_size = 0;

    for (u64 i = 0; i < item_count; i++) {
        char *item_path = item_paths[i];

        if (print_progress) {
            neko_printf("Packing \"%s\" file. ", item_path);
            fflush(stdout);
        }

        size_t path_size = strlen(item_path);

        if (path_size > UINT8_MAX) {
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        FILE *item_file = neko_fopen(item_path, "rb");

        if (!item_file) {
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        int seek_result = neko_fseek(item_file, 0, SEEK_END);

        if (seek_result != 0) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        u64 item_size = (u64)neko_ftell(item_file);

        if (item_size == 0 || item_size > UINT32_MAX) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        seek_result = neko_fseek(item_file, 0, SEEK_SET);

        if (seek_result != 0) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        if (item_size > buffer_size) {
            u8 *new_buffer = (u8 *)mem_realloc(item_data, item_size * sizeof(u8));

            if (!new_buffer) {
                neko_fclose(item_file);
                mem_free(zip_data);
                mem_free(item_data);
                return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
            }

            item_data = new_buffer;

            new_buffer = (u8 *)mem_realloc(zip_data, item_size * sizeof(u8));

            if (!new_buffer) {
                neko_fclose(item_file);
                mem_free(zip_data);
                mem_free(item_data);
                return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
            }

            zip_data = new_buffer;
        }

        size_t result = neko_fread(item_data, sizeof(u8), item_size, item_file);

        neko_fclose(item_file);

        if (result != item_size) {
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
        }

        size_t zip_size;

        if (item_size > 1) {

            const int max_dst_size = neko_lz_bounds(item_size, 0);
            zip_size = neko_lz_encode(item_data, item_size, zip_data, max_dst_size, 9);

            if (zip_size <= 0 || zip_size >= item_size) {
                zip_size = 0;
            }
        } else {
            zip_size = 0;
        }

        i64 file_offset = neko_ftell(pack_file);

        neko_pak::iteminfo info = {
                (u32)zip_size,
                (u32)item_size,
                (u64)file_offset,
                (u8)path_size,
        };

        result = fwrite(&info, sizeof(neko_pak::iteminfo), 1, pack_file);

        if (result != 1) {
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
        }

        result = fwrite(item_path, sizeof(char), info.path_size, pack_file);

        if (result != info.path_size) {
            mem_free(zip_data);
            mem_free(item_data);
            return false; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
        }

        if (zip_size > 0) {
            result = fwrite(zip_data, sizeof(u8), zip_size, pack_file);

            if (result != zip_size) {
                mem_free(zip_data);
                mem_free(item_data);
                return false; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
            }
        } else {
            result = fwrite(item_data, sizeof(u8), item_size, pack_file);

            if (result != item_size) {
                mem_free(zip_data);
                mem_free(item_data);
                return false; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
            }
        }

        if (print_progress) {
            u32 zip_file_size = zip_size > 0 ? (u32)zip_size : (u32)item_size;
            u32 raw_file_size = (u32)item_size;

            total_zip_size += zip_file_size;
            total_raw_size += raw_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            neko_printf("(%u/%u bytes) [%d%%]\n", zip_file_size, raw_file_size, progress);
            fflush(stdout);
        }
    }

    mem_free(zip_data);
    mem_free(item_data);

    if (print_progress) {
        int compression = (int)((1.0 - (f64)(total_zip_size) / (f64)total_raw_size) * 100.0);
        neko_printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)item_count, (long long unsigned int)total_zip_size, (long long unsigned int)total_raw_size,
                    compression);
    }

    return true;
}

static int neko_pak_compare_item_paths(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

bool neko_pak_build(const_str file_path, u64 file_count, const_str *file_paths, bool print_progress) {
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(file_count > 0);
    NEKO_ASSERT(file_paths);

    char **item_paths = (char **)mem_alloc(file_count * sizeof(char *));

    if (!item_paths) return false; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    u64 item_count = 0;

    for (u64 i = 0; i < file_count; i++) {
        bool already_added = false;

        for (u64 j = 0; j < item_count; j++) {
            if (i != j && strcmp(file_paths[i], item_paths[j]) == 0) already_added = true;
        }

        if (!already_added) item_paths[item_count++] = (char *)file_paths[i];
    }

    qsort(item_paths, item_count, sizeof(char *), neko_pak_compare_item_paths);

    FILE *pack_file = neko_fopen(file_path, "wb");

    if (!pack_file) {
        mem_free(item_paths);
        return false; /*FAILED_TO_CREATE_FILE_PACK_RESULT*/
    }

    char header[neko_pak_head_size] = {
            'P', 'A', 'C', 'K', 0, 0, 0, !neko_little_endian,
    };

    i32 buildnum = neko_buildnum();

    size_t write_result = fwrite(header, sizeof(u8), neko_pak_head_size, pack_file);
    write_result += fwrite(&buildnum, sizeof(i32), 1, pack_file);

    if (write_result != neko_pak_head_size + 1) {
        mem_free(item_paths);
        neko_fclose(pack_file);
        remove(file_path);
        return false; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    write_result = fwrite(&item_count, sizeof(u64), 1, pack_file);

    if (write_result != 1) {
        mem_free(item_paths);
        neko_fclose(pack_file);
        remove(file_path);
        return false; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    int pack_result = neko_write_pack_items(pack_file, item_count, item_paths, print_progress);

    mem_free(item_paths);
    neko_fclose(pack_file);

    if (pack_result != 0) {
        remove(file_path);
        return pack_result;
    }

    return true;
}

bool neko_pak_info(const_str file_path, u8 *pack_version, bool *isLittleEndian, u64 *_item_count) {
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(pack_version);
    NEKO_ASSERT(isLittleEndian);
    NEKO_ASSERT(_item_count);

    vfs_file vf = neko_capi_vfs_fopen(file_path);

    if (!vf.data) return false; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/

    char header[neko_pak_head_size];
    i32 buildnum;

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), neko_pak_head_size, &vf);
    result += neko_capi_vfs_fread(&buildnum, sizeof(i32), 1, &vf);

    if (result != neko_pak_head_size + 1) {
        neko_capi_vfs_fclose(&vf);
        return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        neko_capi_vfs_fclose(&vf);
        return false; /*BAD_FILE_TYPE_PACK_RESULT*/
    }

    u64 item_count;

    result = neko_capi_vfs_fread(&item_count, sizeof(u64), 1, &vf);

    neko_capi_vfs_fclose(&vf);

    if (result != 1) return false; /*FAILED_TO_READ_FILE_PACK_RESULT*/

    *pack_version = header[4];
    /*    *pack_version = header[5];
     *pack_version = header[6];*/
    *isLittleEndian = !header[7];
    *_item_count = item_count;
    return true;
}

#pragma endregion

#pragma region xml

#define __neko_xml_expect_not_end(c_)           \
    if (!*(c_)) {                               \
        neko_xml_emit_error("Unexpected end."); \
        return NULL;                            \
    }

typedef struct neko_xml_entity_t {
    char character;
    const_str name;
} neko_xml_entity_t;

static neko_xml_entity_t g_neko_xml_entities[] = {{'&', "&amp;"}, {'\'', "&apos;"}, {'"', "&quot;"}, {'<', "&lt;"}, {'>', "&gt;"}};
static const_str g_neko_xml_error = NULL;

static void neko_xml_emit_error(const_str error) { g_neko_xml_error = error; }

static char *neko_xml_copy_string(const_str str, u32 len) {
    char *r = (char *)mem_alloc(len + 1);
    if (!r) {
        neko_xml_emit_error("Out of memory!");
        return NULL;
    }
    r[len] = '\0';

    for (u32 i = 0; i < len; i++) {
        r[i] = str[i];
    }

    return r;
}

static bool neko_xml_string_is_decimal(const_str str, u32 len) {
    u32 i = 0;
    if (str[0] == '-') i++;

    bool used_dot = false;

    for (; i < len; i++) {
        char c = str[i];
        if (c < '0' || c > '9') {
            if (c == '.' && !used_dot) {
                used_dot = true;
                continue;
            }
            return false;
        }
    }

    return true;
}

static bool neko_xml_string_equal(const_str str_a, u32 len, const_str str_b) {
    for (u32 i = 0; i < len; i++) {
        if (str_a[i] != str_b[i]) return false;
    }

    return true;
}

static u64 neko_xml_hash_string(const_str str, u32 len) {
    u64 hash = 0, x = 0;

    for (u32 i = 0; i < len; i++) {
        hash = (hash << 4) + str[i];
        if ((x = hash & 0xF000000000LL) != 0) {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }

    return (hash & 0x7FFFFFFFFF);
}

static void neko_xml_node_free(neko_xml_node_t *node) {
    for (neko_hash_table_iter it = neko_hash_table_iter_new(node->attributes); neko_hash_table_iter_valid(node->attributes, it); neko_hash_table_iter_advance(node->attributes, it)) {
        neko_xml_attribute_t attrib = neko_hash_table_iter_get(node->attributes, it);

        if (attrib.type == NEKO_XML_ATTRIBUTE_STRING) {
            mem_free(attrib.value.string);
        }

        mem_free(attrib.name);
    }

    for (u32 i = 0; i < neko_dyn_array_size(node->children); i++) {
        neko_xml_node_free(node->children + i);
    }

    mem_free(node->name);
    mem_free(node->text);
    neko_hash_table_free(node->attributes);
    neko_dyn_array_free(node->children);
}

static char *neko_xml_process_text(const_str start, u32 length) {
    char *r = (char *)mem_alloc(length + 1);

    u32 len_sub = 0;

    for (u32 i = 0, ri = 0; i < length; i++, ri++) {
        bool changed = false;
        if (start[i] == '&') {
            for (u32 ii = 0; ii < sizeof(g_neko_xml_entities) / sizeof(*g_neko_xml_entities); ii++) {
                u32 ent_len = neko_string_length(g_neko_xml_entities[ii].name);
                if (neko_xml_string_equal(start + i, ent_len, g_neko_xml_entities[ii].name)) {
                    r[ri] = g_neko_xml_entities[ii].character;
                    i += ent_len - 1;
                    len_sub += ent_len - 1;
                    changed = true;
                    break;
                }
            }
        }

        if (!changed) r[ri] = start[i];
    }

    r[length - len_sub] = '\0';

    return r;
}

// 解析XML块 返回块中的节点数组
static neko_dyn_array(neko_xml_node_t) neko_xml_parse_block(const_str start, u32 length) {
    neko_dyn_array(neko_xml_node_t) r = neko_dyn_array_new(neko_xml_node_t);

    bool is_inside = false;

    for (const_str c = start; *c && c < start + length; c++) {
        if (*c == '<') {
            c++;
            __neko_xml_expect_not_end(c);

            if (*c == '?')  // 跳过XML头
            {
                c++;
                __neko_xml_expect_not_end(c);
                while (*c != '>') {
                    c++;
                    __neko_xml_expect_not_end(c);
                }
                continue;
            } else if (neko_xml_string_equal(c, 3, "!--"))  // 跳过注释
            {
                c++;
                __neko_xml_expect_not_end(c);
                c++;
                __neko_xml_expect_not_end(c);
                c++;
                __neko_xml_expect_not_end(c);
                while (!neko_xml_string_equal(c, 3, "-->")) {
                    c++;
                    __neko_xml_expect_not_end(c);
                }

                continue;
            }

            if (is_inside && *c == '/')
                is_inside = false;
            else
                is_inside = true;

            const_str node_name_start = c;
            u32 node_name_len = 0;

            neko_xml_node_t current_node = {0};

            current_node.attributes = neko_hash_table_new(u64, neko_xml_attribute_t);
            current_node.children = neko_dyn_array_new(neko_xml_node_t);

            if (is_inside) {
                for (; *c != '>' && *c != ' ' && *c != '/'; c++) node_name_len++;

                if (*c != '>') {
                    while (*c != '>' && *c != '/') {
                        while (neko_token_char_is_white_space(*c)) c++;

                        const_str attrib_name_start = c;
                        u32 attrib_name_len = 0;

                        while (neko_token_char_is_alpha(*c) || neko_token_char_is_numeric(*c) || *c == '_') {
                            c++;
                            attrib_name_len++;
                            __neko_xml_expect_not_end(c);
                        }

                        while (*c != '"') {
                            c++;
                            __neko_xml_expect_not_end(c);
                        }

                        c++;
                        __neko_xml_expect_not_end(c);

                        const_str attrib_text_start = c;
                        u32 attrib_text_len = 0;

                        while (*c != '"') {
                            c++;
                            attrib_text_len++;
                            __neko_xml_expect_not_end(c);
                        }

                        c++;
                        __neko_xml_expect_not_end(c);

                        neko_xml_attribute_t attrib = {0};
                        attrib.name = neko_xml_copy_string(attrib_name_start, attrib_name_len);

                        if (neko_xml_string_is_decimal(attrib_text_start, attrib_text_len)) {
                            attrib.type = NEKO_XML_ATTRIBUTE_NUMBER;
                            attrib.value.number = strtod(attrib_text_start, NULL);
                        } else if (neko_xml_string_equal(attrib_text_start, attrib_text_len, "true")) {
                            attrib.type = NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = true;
                        } else if (neko_xml_string_equal(attrib_text_start, attrib_text_len, "false")) {
                            attrib.type = NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = false;
                        } else {
                            attrib.type = NEKO_XML_ATTRIBUTE_STRING;
                            attrib.value.string = neko_xml_process_text(attrib_text_start, attrib_text_len);
                        }

                        neko_hash_table_insert(current_node.attributes, neko_xml_hash_string(attrib_name_start, attrib_name_len), attrib);
                    }
                }

                if (*c == '/')  // 对于没有任何文本的节点
                {
                    c++;
                    __neko_xml_expect_not_end(c);
                    current_node.name = neko_xml_copy_string(node_name_start, node_name_len);
                    neko_dyn_array_push(r, current_node);
                    is_inside = false;
                }
            } else {
                while (*c != '>') {
                    c++;
                    __neko_xml_expect_not_end(c);
                }
            }

            c++;
            __neko_xml_expect_not_end(c);

            if (is_inside) {
                const_str text_start = c;
                u32 text_len = 0;

                const_str end_start = c;
                u32 end_len = 0;

                current_node.name = neko_xml_copy_string(node_name_start, node_name_len);

                for (u32 i = 0; i < length; i++) {
                    if (*c == '<' && *(c + 1) == '/') {
                        c++;
                        __neko_xml_expect_not_end(c);
                        c++;
                        __neko_xml_expect_not_end(c);
                        end_start = c;
                        end_len = 0;
                        while (*c != '>') {
                            end_len++;
                            c++;
                            __neko_xml_expect_not_end(c);
                        }

                        if (neko_xml_string_equal(end_start, end_len, current_node.name)) {
                            break;
                        } else {
                            text_len += end_len + 2;
                            continue;
                        }
                    }

                    c++;
                    text_len++;

                    __neko_xml_expect_not_end(c);
                }

                current_node.children = neko_xml_parse_block(text_start, text_len);
                if (neko_dyn_array_size(current_node.children) == 0)
                    current_node.text = neko_xml_process_text(text_start, text_len);
                else
                    current_node.text = neko_xml_copy_string(text_start, text_len);

                neko_dyn_array_push(r, current_node);

                c--;
            }
        }
    }

    return r;
}

neko_xml_document_t *neko_xml_parse_file(const_str path) {
    u64 size;
    const_str source = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, path, &size);
    if (!source) {
        neko_xml_emit_error("Failed to load xml file!");
        return NULL;
    }
    neko_xml_document_t *doc = neko_xml_parse(source);
    mem_free(source);
    return doc;
}

neko_xml_document_t *neko_xml_parse(const_str source) {
    if (!source) return NULL;

    g_neko_xml_error = NULL;
    neko_xml_document_t *doc = (neko_xml_document_t *)mem_calloc(1, sizeof(neko_xml_document_t));
    if (!doc) {
        neko_xml_emit_error("Out of memory!");
        return NULL;
    }

    doc->nodes = neko_xml_parse_block(source, neko_string_length(source));

    if (g_neko_xml_error) {
        neko_xml_free(doc);
        return NULL;
    }

    return doc;
}

void neko_xml_free(neko_xml_document_t *document) {
    for (u32 i = 0; i < neko_dyn_array_size(document->nodes); i++) {
        neko_xml_node_free(document->nodes + i);
    }

    neko_dyn_array_free(document->nodes);
    mem_free(document);
}

neko_xml_attribute_t *neko_xml_find_attribute(neko_xml_node_t *node, const_str name) {
    if (!neko_hash_table_exists(node->attributes, neko_xml_hash_string(name, neko_string_length(name)))) {
        return NULL;
    } else {
        return neko_hash_table_getp(node->attributes, neko_xml_hash_string(name, neko_string_length(name)));
    }
}

neko_xml_node_t *neko_xml_find_node(neko_xml_document_t *doc, const_str name) {
    for (u32 i = 0; i < neko_dyn_array_size(doc->nodes); i++) {
        if (neko_string_compare_equal(name, doc->nodes[i].name)) {
            return doc->nodes + i;
        }
    }

    return NULL;
}

neko_xml_node_t *neko_xml_find_node_child(neko_xml_node_t *node, const_str name) {
    for (u32 i = 0; i < neko_dyn_array_size(node->children); i++) {
        if (neko_string_compare_equal(name, node->children[i].name)) {
            return node->children + i;
        }
    }

    return NULL;
}

const_str neko_xml_get_error() { return g_neko_xml_error; }

neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t *doc, const_str name) {
    neko_xml_node_iter_t it = {.doc = doc, .name = name, .idx = 0};

    return it;
}

neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t *parent, const_str name) {
    neko_xml_node_iter_t it = {.node = parent, .name = name, .idx = 0};

    return it;
}

bool neko_xml_node_iter_next(neko_xml_node_iter_t *iter) {
    if (iter->node) {
        for (u32 i = iter->idx; i < neko_dyn_array_size(iter->node->children); i++) {
            if (neko_string_compare_equal(iter->name, iter->node->children[i].name)) {
                iter->current = iter->node->children + i;
                iter->idx = i + 1;
                return true;
            }
        }

        return false;
    } else {
        for (u32 i = iter->idx; i < neko_dyn_array_size(iter->doc->nodes); i++) {
            if (neko_string_compare_equal(iter->name, iter->doc->nodes[i].name)) {
                iter->current = iter->doc->nodes + i;
                iter->idx = i + 1;
                return true;
            }
        }
        return false;
    }
}

#pragma endregion

// DEFLATE tables from RFC 1951
static uint8_t s_fixed_table[288 + 32] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};  // 3.2.6
static uint8_t s_permutation_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};                                                                                 // 3.2.7
static uint8_t s_len_extra_bits[29 + 2] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};                                                     // 3.2.5
static uint32_t s_len_base[29 + 2] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};                              // 3.2.5
static uint8_t s_dist_extra_bits[30 + 2] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0};                                         // 3.2.5
static uint32_t s_dist_base[30 + 2] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};  // 3.2.5

typedef struct deflate_t {
    uint64_t bits;
    int count;
    uint32_t *words;
    int word_count;
    int word_index;
    int bits_left;

    int final_word_available;
    uint32_t final_word;

    char *out;
    char *out_end;
    char *begin;

    uint32_t lit[288];
    uint32_t dst[32];
    uint32_t len[19];
    uint32_t nlit;
    uint32_t ndst;
    uint32_t nlen;
} deflate_t;

static int s_would_overflow(deflate_t *s, int num_bits) { return (s->bits_left + s->count) - num_bits < 0; }

static char *s_ptr(deflate_t *s) {
    NEKO_ASSERT(!(s->bits_left & 7));
    return (char *)(s->words + s->word_index) - (s->count / 8);
}

static uint64_t s_peak_bits(deflate_t *s, int num_bits_to_read) {
    if (s->count < num_bits_to_read) {
        if (s->word_index < s->word_count) {
            uint32_t word = s->words[s->word_index++];
            s->bits |= (uint64_t)word << s->count;
            s->count += 32;
            NEKO_ASSERT(s->word_index <= s->word_count);
        }

        else if (s->final_word_available) {
            uint32_t word = s->final_word;
            s->bits |= (uint64_t)word << s->count;
            s->count += s->bits_left;
            s->final_word_available = 0;
        }
    }

    return s->bits;
}

static uint32_t s_consume_bits(deflate_t *s, int num_bits_to_read) {
    NEKO_ASSERT(s->count >= num_bits_to_read);
    uint32_t bits = (uint32_t)(s->bits & (((uint64_t)1 << num_bits_to_read) - 1));
    s->bits >>= num_bits_to_read;
    s->count -= num_bits_to_read;
    s->bits_left -= num_bits_to_read;
    return bits;
}

static uint32_t s_read_bits(deflate_t *s, int num_bits_to_read) {
    NEKO_ASSERT(num_bits_to_read <= 32);
    NEKO_ASSERT(num_bits_to_read >= 0);
    NEKO_ASSERT(s->bits_left > 0);
    NEKO_ASSERT(s->count <= 64);
    NEKO_ASSERT(!s_would_overflow(s, num_bits_to_read));
    s_peak_bits(s, num_bits_to_read);
    uint32_t bits = s_consume_bits(s, num_bits_to_read);
    return bits;
}

static uint32_t s_rev16(uint32_t a) {
    a = ((a & 0xAAAA) >> 1) | ((a & 0x5555) << 1);
    a = ((a & 0xCCCC) >> 2) | ((a & 0x3333) << 2);
    a = ((a & 0xF0F0) >> 4) | ((a & 0x0F0F) << 4);
    a = ((a & 0xFF00) >> 8) | ((a & 0x00FF) << 8);
    return a;
}

// RFC 1951 section 3.2.2
static uint32_t s_build(deflate_t *s, uint32_t *tree, uint8_t *lens, int sym_count) {
    int n, codes[16], first[16], counts[16] = {0};
    NEKO_UNUSED(s);

    // Frequency count
    for (n = 0; n < sym_count; n++) counts[lens[n]]++;

    // Distribute codes
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; ++n) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }

    for (uint32_t i = 0; i < (uint32_t)sym_count; ++i) {
        uint8_t len = lens[i];

        if (len != 0) {
            NEKO_ASSERT(len < 16);
            uint32_t code = (uint32_t)codes[len]++;
            uint32_t slot = (uint32_t)first[len]++;
            tree[slot] = (code << (32 - (uint32_t)len)) | (i << 4) | len;
        }
    }

    return (uint32_t)first[15];
}

static int s_stored(deflate_t *s) {
    char *p;

    // 3.2.3
    // skip any remaining bits in current partially processed byte
    s_read_bits(s, s->count & 7);

    // 3.2.4
    // read LEN and NLEN, should complement each other
    uint16_t LEN = (uint16_t)s_read_bits(s, 16);
    uint16_t NLEN = (uint16_t)s_read_bits(s, 16);
    uint16_t TILDE_NLEN = ~NLEN;
    NEKO_ASSERT(LEN == TILDE_NLEN, "Failed to find LEN and NLEN as complements within stored (uncompressed) stream.");
    NEKO_ASSERT(s->bits_left / 8 <= (int)LEN, "Stored block extends beyond end of input stream.");
    p = s_ptr(s);
    memcpy(s->out, p, LEN);
    s->out += LEN;
    return 1;

ase_err:
    return 0;
}

// 3.2.6
static int s_fixed(deflate_t *s) {
    s->nlit = s_build(s, s->lit, s_fixed_table, 288);
    s->ndst = s_build(0, s->dst, s_fixed_table + 288, 32);
    return 1;
}

static int s_decode(deflate_t *s, uint32_t *tree, int hi) {
    uint64_t bits = s_peak_bits(s, 16);
    uint32_t search = (s_rev16((uint32_t)bits) << 16) | 0xFFFF;
    int lo = 0;
    while (lo < hi) {
        int guess = (lo + hi) >> 1;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    uint32_t key = tree[lo - 1];
    uint32_t len = (32 - (key & 0xF));
    NEKO_ASSERT((search >> len) == (key >> len));

    s_consume_bits(s, key & 0xF);
    return (key >> 4) & 0xFFF;
}

// 3.2.7
static int s_dynamic(deflate_t *s) {
    uint8_t lenlens[19] = {0};

    uint32_t nlit = 257 + s_read_bits(s, 5);
    uint32_t ndst = 1 + s_read_bits(s, 5);
    uint32_t nlen = 4 + s_read_bits(s, 4);

    for (uint32_t i = 0; i < nlen; ++i) lenlens[s_permutation_order[i]] = (uint8_t)s_read_bits(s, 3);

    // Build the tree for decoding code lengths
    s->nlen = s_build(0, s->len, lenlens, 19);
    uint8_t lens[288 + 32];

    for (uint32_t n = 0; n < nlit + ndst;) {
        int sym = s_decode(s, s->len, (int)s->nlen);
        switch (sym) {
            case 16:
                for (uint32_t i = 3 + s_read_bits(s, 2); i; --i, ++n) lens[n] = lens[n - 1];
                break;
            case 17:
                for (uint32_t i = 3 + s_read_bits(s, 3); i; --i, ++n) lens[n] = 0;
                break;
            case 18:
                for (uint32_t i = 11 + s_read_bits(s, 7); i; --i, ++n) lens[n] = 0;
                break;
            default:
                lens[n++] = (uint8_t)sym;
                break;
        }
    }

    s->nlit = s_build(s, s->lit, lens, (int)nlit);
    s->ndst = s_build(0, s->dst, lens + nlit, (int)ndst);
    return 1;
}

// 3.2.3
static int s_block(deflate_t *s) {
    while (1) {
        int symbol = s_decode(s, s->lit, (int)s->nlit);

        if (symbol < 256) {
            NEKO_ASSERT(s->out + 1 <= s->out_end, "Attempted to overwrite out buffer while outputting a symbol.");
            *s->out = (char)symbol;
            s->out += 1;
        }

        else if (symbol > 256) {
            symbol -= 257;
            uint32_t length = s_read_bits(s, (int)(s_len_extra_bits[symbol])) + s_len_base[symbol];
            int distance_symbol = s_decode(s, s->dst, (int)s->ndst);
            uint32_t backwards_distance = s_read_bits(s, s_dist_extra_bits[distance_symbol]) + s_dist_base[distance_symbol];
            NEKO_ASSERT(s->out - backwards_distance >= s->begin, "Attempted to write before out buffer (invalid backwards distance).");
            NEKO_ASSERT(s->out + length <= s->out_end, "Attempted to overwrite out buffer while outputting a string.");
            char *src = s->out - backwards_distance;
            char *dst = s->out;
            s->out += length;

            switch (backwards_distance) {
                case 1:  // very common in images
                    memset(dst, *src, (size_t)length);
                    break;
                default:
                    while (length--) *dst++ = *src++;
            }
        }

        else
            break;
    }

    return 1;

ase_err:
    return 0;
}

// 3.2.3
static int s_inflate(const void *in, int in_bytes, void *out, int out_bytes) {

    deflate_t *s = (deflate_t *)mem_alloc(sizeof(deflate_t));
    s->bits = 0;
    s->count = 0;
    s->word_index = 0;
    s->bits_left = in_bytes * 8;

    // s->words is the in-pointer rounded up to a multiple of 4
    int first_bytes = (int)((((size_t)in + 3) & (size_t)(~3)) - (size_t)in);
    s->words = (uint32_t *)((char *)in + first_bytes);
    s->word_count = (in_bytes - first_bytes) / 4;
    int last_bytes = ((in_bytes - first_bytes) & 3);

    for (int i = 0; i < first_bytes; ++i) s->bits |= (uint64_t)(((uint8_t *)in)[i]) << (i * 8);

    s->final_word_available = last_bytes ? 1 : 0;
    s->final_word = 0;
    for (int i = 0; i < last_bytes; i++) s->final_word |= ((uint8_t *)in)[in_bytes - last_bytes + i] << (i * 8);

    s->count = first_bytes * 8;

    s->out = (char *)out;
    s->out_end = s->out + out_bytes;
    s->begin = (char *)out;

    int count = 0;
    uint32_t bfinal;
    do {
        bfinal = s_read_bits(s, 1);
        uint32_t btype = s_read_bits(s, 2);

        switch (btype) {
            case 0:
                if (!(s_stored(s))) goto ase_err;
                break;
            case 1:
                s_fixed(s);
                if (!(s_block(s))) goto ase_err;
                break;
            case 2:
                s_dynamic(s);
                if (!(s_block(s))) goto ase_err;
                break;
            case 3:
                NEKO_ASSERT(0, "Detected unknown block type within input stream.");
        }

        ++count;
    } while (!bfinal);

    mem_free(s);
    return 1;

ase_err:
    mem_free(s);
    return 0;
}

typedef struct ase_state_t {
    uint8_t *in;
    uint8_t *end;
} ase_state_t;

static uint8_t s_read_uint8(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(uint8_t));
    uint8_t **p = &s->in;
    uint8_t value = **p;
    ++(*p);
    return value;
}

static uint16_t s_read_uint16(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(uint16_t));
    uint8_t **p = &s->in;
    uint16_t value;
    value = (*p)[0];
    value |= (((uint16_t)((*p)[1])) << 8);
    *p += 2;
    return value;
}

static ase_fixed_t s_read_fixed(ase_state_t *s) {
    ase_fixed_t value;
    value.a = s_read_uint16(s);
    value.b = s_read_uint16(s);
    return value;
}

static uint32_t s_read_uint32(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(uint32_t));
    uint8_t **p = &s->in;
    uint32_t value;
    value = (*p)[0];
    value |= (((uint32_t)((*p)[1])) << 8);
    value |= (((uint32_t)((*p)[2])) << 16);
    value |= (((uint32_t)((*p)[3])) << 24);
    *p += 4;
    return value;
}

#ifdef CUTE_ASPRITE_S_READ_UINT64
// s_read_uint64() is not currently used.
static uint64_t s_read_uint64(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(uint64_t));
    uint8_t **p = &s->in;
    uint64_t value;
    value = (*p)[0];
    value |= (((uint64_t)((*p)[1])) << 8);
    value |= (((uint64_t)((*p)[2])) << 16);
    value |= (((uint64_t)((*p)[3])) << 24);
    value |= (((uint64_t)((*p)[4])) << 32);
    value |= (((uint64_t)((*p)[5])) << 40);
    value |= (((uint64_t)((*p)[6])) << 48);
    value |= (((uint64_t)((*p)[7])) << 56);
    *p += 8;
    return value;
}
#endif

#define s_read_int16(s) (int16_t) s_read_uint16(s)
#define s_read_int32(s) (int32_t) s_read_uint32(s)

#ifdef CUTE_ASPRITE_S_READ_BYTES
// s_read_bytes() is not currently used.
static void s_read_bytes(ase_state_t *s, uint8_t *bytes, int num_bytes) {
    for (int i = 0; i < num_bytes; ++i) {
        bytes[i] = s_read_uint8(s);
    }
}
#endif

static const char *s_read_string(ase_state_t *s) {
    int len = (int)s_read_uint16(s);
    char *bytes = (char *)mem_alloc(len + 1);
    for (int i = 0; i < len; ++i) {
        bytes[i] = (char)s_read_uint8(s);
    }
    bytes[len] = 0;
    return bytes;
}

static void s_skip(ase_state_t *ase, int num_bytes) {
    NEKO_ASSERT(ase->in <= ase->end + num_bytes);
    ase->in += num_bytes;
}

static int s_mul_un8(int a, int b) {
    int t = (a * b) + 0x80;
    return (((t >> 8) + t) >> 8);
}

static ase_color_t s_blend(ase_color_t src, ase_color_t dst, uint8_t opacity) {
    src.a = (uint8_t)s_mul_un8(src.a, opacity);
    int a = src.a + dst.a - s_mul_un8(src.a, dst.a);
    int r, g, b;
    if (a == 0) {
        r = g = b = 0;
    } else {
        r = dst.r + (src.r - dst.r) * src.a / a;
        g = dst.g + (src.g - dst.g) * src.a / a;
        b = dst.b + (src.b - dst.b) * src.a / a;
    }
    ase_color_t ret = {(uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a};
    return ret;
}

static int s_min(int a, int b) { return a < b ? a : b; }

static int s_max(int a, int b) { return a < b ? b : a; }

static ase_color_t s_color(ase_t *ase, void *src, int index) {
    ase_color_t result;
    if (ase->mode == ASE_MODE_RGBA) {
        result = ((ase_color_t *)src)[index];
    } else if (ase->mode == ASE_MODE_GRAYSCALE) {
        uint8_t saturation = ((uint8_t *)src)[index * 2];
        uint8_t a = ((uint8_t *)src)[index * 2 + 1];
        result.r = result.g = result.b = saturation;
        result.a = a;
    } else {
        NEKO_ASSERT(ase->mode == ASE_MODE_INDEXED);
        uint8_t palette_index = ((uint8_t *)src)[index];
        if (palette_index == ase->transparent_palette_entry_index) {
            result.r = 0;
            result.g = 0;
            result.b = 0;
            result.a = 0;
        } else {
            result = ase->palette.entries[palette_index].color;
        }
    }
    return result;
}

ase_t *neko_aseprite_load_from_memory(const void *memory, int size) {
    ase_t *ase = (ase_t *)mem_alloc(sizeof(ase_t));
    memset(ase, 0, sizeof(*ase));

    ase_state_t state = {0};
    ase_state_t *s = &state;
    s->in = (uint8_t *)memory;
    s->end = s->in + size;

    s_skip(s, sizeof(uint32_t));  // File size.
    int magic = (int)s_read_uint16(s);
    NEKO_ASSERT(magic == 0xA5E0);

    ase->frame_count = (int)s_read_uint16(s);
    ase->w = s_read_uint16(s);
    ase->h = s_read_uint16(s);
    uint16_t bpp = s_read_uint16(s) / 8;
    if (bpp == 4)
        ase->mode = ASE_MODE_RGBA;
    else if (bpp == 2)
        ase->mode = ASE_MODE_GRAYSCALE;
    else {
        NEKO_ASSERT(bpp == 1);
        ase->mode = ASE_MODE_INDEXED;
    }
    uint32_t valid_layer_opacity = s_read_uint32(s) & 1;
    int speed = s_read_uint16(s);
    s_skip(s, sizeof(uint32_t) * 2);  // Spec says skip these bytes, as they're zero'd.
    ase->transparent_palette_entry_index = s_read_uint8(s);
    s_skip(s, 3);  // Spec says skip these bytes.
    ase->number_of_colors = (int)s_read_uint16(s);
    ase->pixel_w = (int)s_read_uint8(s);
    ase->pixel_h = (int)s_read_uint8(s);
    ase->grid_x = (int)s_read_int16(s);
    ase->grid_y = (int)s_read_int16(s);
    ase->grid_w = (int)s_read_uint16(s);
    ase->grid_h = (int)s_read_uint16(s);
    s_skip(s, 84);  // For future use (set to zero).

    ase->frames = (ase_frame_t *)mem_alloc((int)(sizeof(ase_frame_t)) * ase->frame_count);
    memset(ase->frames, 0, sizeof(ase_frame_t) * (size_t)ase->frame_count);

    ase_udata_t *last_udata = NULL;
    int was_on_tags = 0;
    int tag_index = 0;

    ase_layer_t *layer_stack[NEKO_ASEPRITE_MAX_LAYERS];

    // Parse all chunks in the .aseprite file.
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        frame->ase = ase;
        s_skip(s, sizeof(uint32_t));  // Frame size.
        magic = (int)s_read_uint16(s);
        NEKO_ASSERT(magic == 0xF1FA);
        int chunk_count = (int)s_read_uint16(s);
        frame->duration_milliseconds = s_read_uint16(s);
        if (frame->duration_milliseconds == 0) frame->duration_milliseconds = speed;
        s_skip(s, 2);  // For future use (set to zero).
        uint32_t new_chunk_count = s_read_uint32(s);
        if (new_chunk_count) chunk_count = (int)new_chunk_count;

        for (int j = 0; j < chunk_count; ++j) {
            uint32_t chunk_size = s_read_uint32(s);
            uint16_t chunk_type = s_read_uint16(s);
            chunk_size -= (uint32_t)(sizeof(uint32_t) + sizeof(uint16_t));
            uint8_t *chunk_start = s->in;

            switch (chunk_type) {
                case 0x0004:  // 旧调色板块(当调色板中没有带有 alpha 的颜色时使用)
                {
                    uint16_t nbPackets = s_read_uint16(s);
                    for (uint16_t k = 0; k < nbPackets; k++) {
                        uint16_t maxColor = 0;
                        uint16_t skip = (uint16_t)s_read_uint8(s);
                        uint16_t nbColors = (uint16_t)s_read_uint8(s);
                        if (nbColors == 0) nbColors = 256;

                        for (uint16_t l = 0; l < nbColors; l++) {
                            ase_palette_entry_t entry;
                            entry.color.r = s_read_uint8(s);
                            entry.color.g = s_read_uint8(s);
                            entry.color.b = s_read_uint8(s);
                            entry.color.a = 255;
                            entry.color_name = NULL;
                            ase->palette.entries[skip + l] = entry;
                            if (skip + l > maxColor) maxColor = skip + l;
                        }

                        ase->palette.entry_count = maxColor + 1;
                    }

                } break;
                case 0x2004:  // Layer chunk.
                {
                    NEKO_ASSERT(ase->layer_count < NEKO_ASEPRITE_MAX_LAYERS);
                    ase_layer_t *layer = ase->layers + ase->layer_count++;
                    layer->flags = (ase_layer_flags_t)s_read_uint16(s);
                    layer->type = (ase_layer_type_t)s_read_uint16(s);
                    layer->parent = NULL;
                    int child_level = (int)s_read_uint16(s);
                    layer_stack[child_level] = layer;
                    if (child_level) {
                        layer->parent = layer_stack[child_level - 1];
                    }
                    s_skip(s, sizeof(uint16_t));  // Default layer width in pixels (ignored).
                    s_skip(s, sizeof(uint16_t));  // Default layer height in pixels (ignored).
                    int blend_mode = (int)s_read_uint16(s);
                    if (blend_mode) NEKO_WARN("unknown blend mode encountered.");
                    layer->opacity = s_read_uint8(s) / 255.0f;
                    if (!valid_layer_opacity) layer->opacity = 1.0f;
                    s_skip(s, 3);  // For future use (set to zero).
                    layer->name = s_read_string(s);
                    last_udata = &layer->udata;
                } break;

                case 0x2005:  // Cel chunk.
                {
                    NEKO_ASSERT(frame->cel_count < NEKO_ASEPRITE_MAX_LAYERS);
                    ase_cel_t *cel = frame->cels + frame->cel_count++;
                    int layer_index = (int)s_read_uint16(s);
                    cel->layer = ase->layers + layer_index;
                    cel->x = s_read_int16(s);
                    cel->y = s_read_int16(s);
                    cel->opacity = s_read_uint8(s) / 255.0f;
                    int cel_type = (int)s_read_uint16(s);
                    s_skip(s, 7);  // For future (set to zero).
                    switch (cel_type) {
                        case 0:  // Raw cel.
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            cel->pixels = mem_alloc(cel->w * cel->h * bpp);
                            memcpy(cel->pixels, s->in, (size_t)(cel->w * cel->h * bpp));
                            s_skip(s, cel->w * cel->h * bpp);
                            break;

                        case 1:  // Linked cel.
                            cel->is_linked = 1;
                            cel->linked_frame_index = s_read_uint16(s);
                            break;

                        case 2:  // Compressed image cel.
                        {
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            int zlib_byte0 = s_read_uint8(s);
                            int zlib_byte1 = s_read_uint8(s);
                            int deflate_bytes = (int)chunk_size - (int)(s->in - chunk_start);
                            void *pixels = s->in;
                            NEKO_ASSERT((zlib_byte0 & 0x0F) == 0x08);  // Only zlib compression method (RFC 1950) is supported.
                            NEKO_ASSERT((zlib_byte0 & 0xF0) <= 0x70);  // Innapropriate window size detected.
                            NEKO_ASSERT(!(zlib_byte1 & 0x20));         // Preset dictionary is present and not supported.
                            int pixels_sz = cel->w * cel->h * bpp;
                            void *pixels_decompressed = mem_alloc(pixels_sz);
                            int ret = s_inflate(pixels, deflate_bytes, pixels_decompressed, pixels_sz);
                            if (!ret) NEKO_WARN("?");
                            cel->pixels = pixels_decompressed;
                            s_skip(s, deflate_bytes);
                        } break;
                    }
                    last_udata = &cel->udata;
                } break;

                case 0x2006:  // Cel extra chunk.
                {
                    ase_cel_t *cel = frame->cels + frame->cel_count;
                    cel->has_extra = 1;
                    cel->extra.precise_bounds_are_set = (int)s_read_uint32(s);
                    cel->extra.precise_x = s_read_fixed(s);
                    cel->extra.precise_y = s_read_fixed(s);
                    cel->extra.w = s_read_fixed(s);
                    cel->extra.h = s_read_fixed(s);
                    s_skip(s, 16);  // For future use (set to zero).
                } break;

                case 0x2007:  // Color profile chunk.
                {
                    ase->has_color_profile = 1;
                    ase->color_profile.type = (ase_color_profile_type_t)s_read_uint16(s);
                    ase->color_profile.use_fixed_gamma = (int)s_read_uint16(s) & 1;
                    ase->color_profile.gamma = s_read_fixed(s);
                    s_skip(s, 8);  // For future use (set to zero).
                    if (ase->color_profile.type == ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC) {
                        // Use the embedded ICC profile.
                        ase->color_profile.icc_profile_data_length = s_read_uint32(s);
                        ase->color_profile.icc_profile_data = mem_alloc(ase->color_profile.icc_profile_data_length);
                        memcpy(ase->color_profile.icc_profile_data, s->in, ase->color_profile.icc_profile_data_length);
                        s->in += ase->color_profile.icc_profile_data_length;
                    }
                } break;

                case 0x2018:  // Tags chunk.
                {
                    ase->tag_count = (int)s_read_uint16(s);
                    s_skip(s, 8);  // For future (set to zero).
                    NEKO_ASSERT(ase->tag_count < NEKO_ASEPRITE_MAX_TAGS);
                    for (int k = 0; k < ase->tag_count; ++k) {
                        ase->tags[k].from_frame = (int)s_read_uint16(s);
                        ase->tags[k].to_frame = (int)s_read_uint16(s);
                        ase->tags[k].loop_animation_direction = (ase_animation_direction_t)s_read_uint8(s);
                        ase->tags[k].repeat = s_read_uint16(s);
                        s_skip(s, 6);  // For future (set to zero).
                        ase->tags[k].r = s_read_uint8(s);
                        ase->tags[k].g = s_read_uint8(s);
                        ase->tags[k].b = s_read_uint8(s);
                        s_skip(s, 1);  // Extra byte (zero).
                        ase->tags[k].name = s_read_string(s);
                    }
                    was_on_tags = 1;
                } break;

                case 0x2019:  // Palette chunk.
                {
                    ase->palette.entry_count = (int)s_read_uint32(s);
                    NEKO_ASSERT(ase->palette.entry_count <= NEKO_ASEPRITE_MAX_PALETTE_ENTRIES);
                    int first_index = (int)s_read_uint32(s);
                    int last_index = (int)s_read_uint32(s);
                    s_skip(s, 8);  // For future (set to zero).
                    for (int k = first_index; k <= last_index; ++k) {
                        int has_name = s_read_uint16(s);
                        ase_palette_entry_t entry;
                        entry.color.r = s_read_uint8(s);
                        entry.color.g = s_read_uint8(s);
                        entry.color.b = s_read_uint8(s);
                        entry.color.a = s_read_uint8(s);
                        if (has_name) {
                            entry.color_name = s_read_string(s);
                        } else {
                            entry.color_name = NULL;
                        }
                        NEKO_ASSERT(k < NEKO_ASEPRITE_MAX_PALETTE_ENTRIES);
                        ase->palette.entries[k] = entry;
                    }
                } break;

                case 0x2020:  // Udata chunk.
                {
                    NEKO_ASSERT(last_udata || was_on_tags);
                    if (was_on_tags && !last_udata) {
                        NEKO_ASSERT(tag_index < ase->tag_count);
                        last_udata = &ase->tags[tag_index++].udata;
                    }
                    int flags = (int)s_read_uint32(s);
                    if (flags & 1) {
                        last_udata->has_text = 1;
                        last_udata->text = s_read_string(s);
                    }
                    if (flags & 2) {
                        last_udata->color.r = s_read_uint8(s);
                        last_udata->color.g = s_read_uint8(s);
                        last_udata->color.b = s_read_uint8(s);
                        last_udata->color.a = s_read_uint8(s);
                    }
                    last_udata = NULL;
                } break;

                case 0x2022:  // Slice chunk.
                {
                    int slice_count = (int)s_read_uint32(s);
                    int flags = (int)s_read_uint32(s);
                    s_skip(s, sizeof(uint32_t));  // Reserved.
                    const char *name = s_read_string(s);
                    for (int k = 0; k < (int)slice_count; ++k) {
                        ase_slice_t slice = {0};
                        slice.name = name;
                        slice.frame_number = (int)s_read_uint32(s);
                        slice.origin_x = (int)s_read_int32(s);
                        slice.origin_y = (int)s_read_int32(s);
                        slice.w = (int)s_read_uint32(s);
                        slice.h = (int)s_read_uint32(s);
                        if (flags & 1) {
                            // It's a 9-patches slice.
                            slice.has_center_as_9_slice = 1;
                            slice.center_x = (int)s_read_int32(s);
                            slice.center_y = (int)s_read_int32(s);
                            slice.center_w = (int)s_read_uint32(s);
                            slice.center_h = (int)s_read_uint32(s);
                        }
                        if (flags & 2) {
                            // Has pivot information.
                            slice.has_pivot = 1;
                            slice.pivot_x = (int)s_read_int32(s);
                            slice.pivot_y = (int)s_read_int32(s);
                        }
                        NEKO_ASSERT(ase->slice_count < NEKO_ASEPRITE_MAX_SLICES);
                        ase->slices[ase->slice_count++] = slice;
                        last_udata = &ase->slices[ase->slice_count - 1].udata;
                    }
                } break;

                default:
                    s_skip(s, (int)chunk_size);
                    break;
            }

            uint32_t size_read = (uint32_t)(s->in - chunk_start);
            NEKO_ASSERT(size_read == chunk_size);
        }
    }

    // Blend all cel pixels into each of their respective frames, for convenience.
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        frame->pixels = (ase_color_t *)mem_alloc((int)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t *dst = frame->pixels;
        for (int j = 0; j < frame->cel_count; ++j) {
            ase_cel_t *cel = frame->cels + j;
            if (!(cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE)) {
                continue;
            }
            if (cel->layer->parent && !(cel->layer->parent->flags & ASE_LAYER_FLAGS_VISIBLE)) {
                continue;
            }
            while (cel->is_linked) {
                ase_frame_t *frame = ase->frames + cel->linked_frame_index;
                int found = 0;
                for (int k = 0; k < frame->cel_count; ++k) {
                    if (frame->cels[k].layer == cel->layer) {
                        cel = frame->cels + k;
                        found = 1;
                        break;
                    }
                }
                NEKO_ASSERT(found);
            }
            void *src = cel->pixels;
            uint8_t opacity = (uint8_t)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -s_min(cx, 0);
            int ct = -s_min(cy, 0);
            int dl = s_max(cx, 0);
            int dt = s_max(cy, 0);
            int dr = s_min(ase->w, cw + cx);
            int db = s_min(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    ase_color_t src_color = s_color(ase, src, cw * sy + sx);
                    ase_color_t dst_color = dst[dst_index];
                    ase_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }

    return ase;
}

void neko_aseprite_free(ase_t *ase) {
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        mem_free(frame->pixels);
        for (int j = 0; j < frame->cel_count; ++j) {
            ase_cel_t *cel = frame->cels + j;
            mem_free(cel->pixels);
            mem_free((void *)cel->udata.text);
        }
    }
    for (int i = 0; i < ase->layer_count; ++i) {
        ase_layer_t *layer = ase->layers + i;
        mem_free((void *)layer->name);
        mem_free((void *)layer->udata.text);
    }
    for (int i = 0; i < ase->tag_count; ++i) {
        ase_tag_t *tag = ase->tags + i;
        mem_free((void *)tag->name);
    }
    for (int i = 0; i < ase->slice_count; ++i) {
        ase_slice_t *slice = ase->slices + i;
        mem_free((void *)slice->udata.text);
    }
    if (ase->slice_count) {
        mem_free((void *)ase->slices[0].name);
    }
    for (int i = 0; i < ase->palette.entry_count; ++i) {
        mem_free((void *)ase->palette.entries[i].color_name);
    }
    mem_free(ase->color_profile.icc_profile_data);
    mem_free(ase->frames);
    mem_free(ase);
}

struct FileChange {
    u64 key;
    u64 modtime;
};

struct Assets {
    HashMap<Asset> table;
    RWLock rw_lock;

    Mutex shutdown_mtx;
    Cond shutdown_notify;
    bool shutdown;

    Thread reload_thread;

    Mutex changes_mtx;
    Array<FileChange> changes;
    Array<FileChange> tmp_changes;
};

static Assets g_assets = {};

static void hot_reload_thread(void *) {
    u32 reload_interval = g_app->reload_interval.load();

    while (true) {
        PROFILE_BLOCK("hot reload");

        {
            LockGuard lock{&g_assets.shutdown_mtx};
            if (g_assets.shutdown) {
                return;
            }

            bool signaled = g_assets.shutdown_notify.timed_wait(&g_assets.shutdown_mtx, reload_interval);
            if (signaled) {
                return;
            }
        }

        {
            PROFILE_BLOCK("check for updates");

            g_assets.rw_lock.shared_lock();
            neko_defer(g_assets.rw_lock.shared_unlock());

            g_assets.tmp_changes.len = 0;

            for (auto [k, v] : g_assets.table) {
                PROFILE_BLOCK("read modtime");

                u64 modtime = os_file_modtime(v->name.data);
                if (modtime > v->modtime) {
                    FileChange change = {};
                    change.key = v->hash;
                    change.modtime = modtime;

                    g_assets.tmp_changes.push(change);
                }
            }
        }

        if (g_assets.tmp_changes.len > 0) {
            LockGuard lock{&g_assets.changes_mtx};
            for (FileChange change : g_assets.tmp_changes) {
                g_assets.changes.push(change);
            }
        }
    }
}

void assets_perform_hot_reload_changes() {
    LockGuard lock{&g_assets.changes_mtx};

    if (g_assets.changes.len == 0) {
        return;
    }

    PROFILE_BLOCK("perform hot reload");

    for (FileChange change : g_assets.changes) {
        Asset a = {};
        bool exists = asset_read(change.key, &a);
        assert(exists);

        a.modtime = change.modtime;

        bool ok = false;
        switch (a.kind) {
            case AssetKind_LuaRef: {
                luaL_unref(g_app->L, LUA_REGISTRYINDEX, a.lua_ref);
                a.lua_ref = luax_require_script(g_app->L, a.name);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                bool generate_mips = a.image.has_mips;
                a.image.trash();
                ok = a.image.load(a.name, generate_mips);
                break;
            }
            case AssetKind_Sprite: {
                a.sprite.trash();
                ok = a.sprite.load(a.name);
                break;
            }
            case AssetKind_Tilemap: {
                a.tilemap.trash();
                ok = a.tilemap.load(a.name);
                break;
            }
            default:
                continue;
                break;
        }

        if (!ok) {
            fatal_error(tmp_fmt("failed to hot reload: %s", a.name.data));
            return;
        }

        asset_write(a);
        NEKO_INFO("reloaded: %s", a.name.data);
    }

    g_assets.changes.len = 0;
}

void assets_shutdown() {
    if (g_app->hot_reload_enabled.load()) {
        {
            LockGuard lock{&g_assets.shutdown_mtx};
            g_assets.shutdown = true;
        }

        g_assets.shutdown_notify.signal();
        g_assets.reload_thread.join();
        g_assets.changes.trash();
        g_assets.tmp_changes.trash();
    }

    for (auto [k, v] : g_assets.table) {
        mem_free(v->name.data);

        switch (v->kind) {
            case AssetKind_Image:
                v->image.trash();
                break;
            case AssetKind_Sprite:
                v->sprite.trash();
                break;
            case AssetKind_Tilemap:
                v->tilemap.trash();
                break;
            default:
                break;
        }
    }
    g_assets.table.trash();

    g_assets.shutdown_notify.trash();
    g_assets.changes_mtx.trash();
    g_assets.shutdown_mtx.trash();
    g_assets.rw_lock.trash();
}

void assets_start_hot_reload() {
    g_assets.shutdown_notify.make();
    g_assets.changes_mtx.make();
    g_assets.shutdown_mtx.make();
    g_assets.rw_lock.make();

    if (g_app->hot_reload_enabled.load()) {
        g_assets.reload_thread.make(hot_reload_thread, nullptr);
    }
}

bool asset_load_kind(AssetKind kind, String filepath, Asset *out) {
    AssetLoadData data = {};
    data.kind = kind;

    return asset_load(data, filepath, out);
}

bool asset_load(AssetLoadData desc, String filepath, Asset *out) {
    PROFILE_FUNC();

    u64 key = fnv1a(filepath);

    {
        Asset asset = {};
        if (asset_read(key, &asset)) {
            if (out != nullptr) {
                *out = asset;
            }
            return true;
        }
    }

    {
        PROFILE_BLOCK("load new asset");

        Asset asset = {};
        asset.name = to_cstr(filepath);
        asset.hash = key;
        {
            PROFILE_BLOCK("asset modtime")
            asset.modtime = os_file_modtime(asset.name.data);
        }
        asset.kind = desc.kind;

        bool ok = false;
        switch (desc.kind) {
            case AssetKind_LuaRef: {
                asset.lua_ref = LUA_REFNIL;
                asset_write(asset);
                asset.lua_ref = luax_require_script(g_app->L, filepath);
                ok = true;
                break;
            }
            case AssetKind_Image:
                ok = asset.image.load(filepath, desc.generate_mips);
                break;
            case AssetKind_Sprite:
                ok = asset.sprite.load(filepath);
                break;
            case AssetKind_Tilemap:
                ok = asset.tilemap.load(filepath);
                break;
            default:
                break;
        }

        if (!ok) {
            mem_free(asset.name.data);
            return false;
        }

        asset_write(asset);

        if (out != nullptr) {
            *out = asset;
        }
        return true;
    }
}

bool asset_read(u64 key, Asset *out) {
    g_assets.rw_lock.shared_lock();
    neko_defer(g_assets.rw_lock.shared_unlock());

    const Asset *asset = g_assets.table.get(key);
    if (asset == nullptr) {
        return false;
    }

    *out = *asset;
    return true;
}

void asset_write(Asset asset) {
    g_assets.rw_lock.unique_lock();
    neko_defer(g_assets.rw_lock.unique_unlock());

    g_assets.table[asset.hash] = asset;
}

Asset check_asset(lua_State *L, u64 key) {
    Asset asset = {};
    if (!asset_read(key, &asset)) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}

Asset check_asset_mt(lua_State *L, i32 arg, const char *mt) {
    u64 *udata = (u64 *)luaL_checkudata(L, arg, mt);

    Asset asset = {};
    bool ok = asset_read(*udata, &asset);
    if (!ok) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}
