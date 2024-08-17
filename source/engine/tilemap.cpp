
#include "engine/tilemap.h"

#include "engine/asset.h"
#include "engine/game.h"
#include "engine/seri.h"

// deps
#ifdef NEKO_BOX2D
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>
#endif

#include <stb_image.h>

static bool layer_from_json(TilemapLayer *layer, JSON *json, bool *ok, Arena *arena, String filepath, HashMap<Texture> *images) {
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

        Texture *img = images->get(key);
        if (img != nullptr) {
            layer->image = *img;
        } else {
            Texture create_img = {};
            neko_assert(false);

            // bool success = create_img.load(String(sb), false);
            // if (!success) {
            //     return false;
            // }

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

static bool level_from_json(TilemapLevel *level, JSON *json, bool *ok, Arena *arena, String filepath, HashMap<Texture> *images) {
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

bool MapLdtk::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool success = vfs_read_entire_file(&contents, filepath);
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
    HashMap<Texture> images = {};
    bool created = false;
    neko_defer({
        neko_assert(false);
        // if (!created) {
        //     for (auto [k, v] : images) {
        //         v->trash();
        //     }
        //     images.trash();
        //     arena.trash();
        // }
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

    MapLdtk tilemap = {};
    tilemap.arena = arena;
    tilemap.levels = levels;
    tilemap.images = images;

    console_log("loaded tilemap with %llu levels", (unsigned long long)tilemap.levels.len);
    *this = tilemap;
    created = true;
    return true;
}

void MapLdtk::trash() {
    for (auto [k, v] : images) {
        // v->trash();
        neko_assert(false);
    }
    images.trash();

    bodies.trash();
    graph.trash();
    frontier.trash();

    arena.trash();
}

void MapLdtk::destroy_bodies(b2World *world) {
#ifdef NEKO_BOX2D
    for (auto [k, v] : bodies) {
        world->DestroyBody(*v);
    }
#endif
}

#ifdef NEKO_BOX2D
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
#endif

void MapLdtk::make_collision(b2World *world, float meter, String layer_name, Slice<TilemapInt> walls) {
    PROFILE_FUNC();

#ifdef NEKO_BOX2D
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
#endif
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

void MapLdtk::make_graph(i32 bloom, String layer_name, Slice<TileCost> costs) {
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

static void astar_reset(MapLdtk *tm) {
    PROFILE_FUNC();

    tm->frontier.len = 0;

    for (auto [k, v] : tm->graph) {
        v->prev = nullptr;
        v->g = 0;
        v->flags = 0;
    }
}

TileNode *MapLdtk::astar(TilePoint start, TilePoint goal) {
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

void neko_tiled_load(map_t *map, const_str tmx_path, const_str res_path) {

    PROFILE_FUNC();

    map->doc = xml_parse_vfs(tmx_path);
    if (!map->doc) {
        neko_panic("Failed to parse XML: %s", xml_get_error());
        return;
    }

    char tmx_root_path[256];
    if (NULL == res_path) {
        neko_util_get_dir_from_file(tmx_root_path, 256, tmx_path);
    } else {
        strcpy(tmx_root_path, res_path);
    }

    xml_node_t *map_node = xml_find_node(map->doc, "map");
    neko_assert(map_node);  // Must have a map node!

    for (xml_node_iter_t it = xml_new_node_child_iter(map_node, "tileset"); xml_node_iter_next(&it);) {
        tileset_t tileset = {0};

        tileset.first_gid = (u32)xml_find_attribute(it.current, "firstgid")->value.number;

        char tileset_path[256];
        neko_snprintf(tileset_path, 256, "%s/%s", tmx_root_path, xml_find_attribute(it.current, "source")->value.string);
        xml_document_t *tileset_doc = xml_parse_vfs(tileset_path);
        if (!tileset_doc) {
            neko_panic("Failed to parse XML from %s: %s", tileset_path, xml_get_error());
            return;
        }

        xml_node_t *tileset_node = xml_find_node(tileset_doc, "tileset");
        tileset.tile_width = (u32)xml_find_attribute(tileset_node, "tilewidth")->value.number;
        tileset.tile_height = (u32)xml_find_attribute(tileset_node, "tileheight")->value.number;
        tileset.tile_count = (u32)xml_find_attribute(tileset_node, "tilecount")->value.number;

        xml_node_t *image_node = xml_find_node_child(tileset_node, "image");
        const char *image_path = xml_find_attribute(image_node, "source")->value.string;

        char full_image_path[256];
        neko_snprintf(full_image_path, 256, "%s/%s", tmx_root_path, image_path);

        bool ok = neko_capi_vfs_file_exists(NEKO_PACKS::GAMEDATA, full_image_path);
        if (!ok) {
            neko_panic("failed to load texture file: %s", full_image_path);
            return;
        }

        void *tex_data = NULL;
        i32 w, h;
        u32 cc;
        neko_util_load_texture_data_from_file(full_image_path, &w, &h, &cc, &tex_data, false);

        gfx_texture_desc_t tileset_tex_decl = {
                .width = (u32)w, .height = (u32)h, .format = R_TEXTURE_FORMAT_RGBA8, .min_filter = R_TEXTURE_FILTER_NEAREST, .mag_filter = R_TEXTURE_FILTER_NEAREST, .num_mips = 0};

        tileset_tex_decl.data[0] = tex_data;

        tileset.texture = gfx_texture_create(tileset_tex_decl);

        tileset.width = w;
        tileset.height = h;

        mem_free(tex_data);

        neko_dyn_array_push(map->tilesets, tileset);

        xml_free(tileset_doc);
    }

    for (xml_node_iter_t it = xml_new_node_child_iter(map_node, "layer"); xml_node_iter_next(&it);) {
        xml_node_t *layer_node = it.current;

        layer_t layer = {0};
        layer.tint = color256(255, 255, 255, 255);

        layer.width = (u32)xml_find_attribute(layer_node, "width")->value.number;
        layer.height = (u32)xml_find_attribute(layer_node, "height")->value.number;

        xml_attribute_t *tint_attrib = xml_find_attribute(layer_node, "tintcolor");
        if (tint_attrib) {
            const char *hexstring = tint_attrib->value.string;
            u32 *cols = (u32 *)layer.tint.rgba;
            *cols = (u32)strtol(hexstring + 1, NULL, 16);
            layer.tint.a = 255;
        }

        xml_node_t *data_node = xml_find_node_child(layer_node, "data");

        const char *encoding = xml_find_attribute(data_node, "encoding")->value.string;

        if (strcmp(encoding, "csv") != 0) {
            neko_panic("%s", "Only CSV data encoding is supported.");
            return;
        }

        const char *data_text = data_node->text;

        const char *cd_ptr = data_text;

        layer.tiles = (tile_t *)mem_alloc(layer.width * layer.height * sizeof(tile_t));

        for (u32 y = 0; y < layer.height; y++) {
            for (u32 x = 0; x < layer.width; x++) {
                u32 gid = (u32)strtod(cd_ptr, NULL);
                u32 tls_id = 0;

                u32 closest = 0;
                for (u32 i = 0; i < neko_dyn_array_size(map->tilesets); i++) {
                    if (map->tilesets[i].first_gid <= gid) {
                        if (map->tilesets[i].first_gid > closest) {
                            closest = map->tilesets[i].first_gid;
                            tls_id = i;
                        }
                    }
                }

                layer.tiles[x + y * layer.width].id = gid;
                layer.tiles[x + y * layer.width].tileset_id = tls_id;

                while (*cd_ptr && *cd_ptr != ',') {
                    cd_ptr++;
                }

                cd_ptr++; /* Skip the comma. */
            }
        }

        neko_dyn_array_push(map->layers, layer);
    }

    for (xml_node_iter_t it = xml_new_node_child_iter(map_node, "objectgroup"); xml_node_iter_next(&it);) {
        xml_node_t *object_group_node = it.current;

        object_group_t object_group = {0};
        object_group.color = color256(255, 255, 255, 255);

        // 对象组名字
        xml_attribute_t *name_attrib = xml_find_attribute(object_group_node, "name");
        if (name_attrib) {
            const char *namestring = name_attrib->value.string;

            object_group.name = name_attrib->value.string;
            // u32 *cols = (u32 *)object_group.color.rgba;
            //*cols = (u32)strtol(hexstring + 1, NULL, 16);
            // object_group.color.a = 128;
            console_log("objectgroup: %s", namestring);
        } else {
        }

        // 对象组默认颜色
        xml_attribute_t *color_attrib = xml_find_attribute(object_group_node, "color");
        if (color_attrib) {
            const char *hexstring = color_attrib->value.string;
            u32 *cols = (u32 *)object_group.color.rgba;
            *cols = (u32)strtol(hexstring + 1, NULL, 16);
            object_group.color.a = 128;
        }

        for (xml_node_iter_t iit = xml_new_node_child_iter(object_group_node, "object"); xml_node_iter_next(&iit);) {
            xml_node_t *object_node = iit.current;

            object_t object = {0};
            object.id = (i32)xml_find_attribute(object_node, "id")->value.number;
            object.x = (i32)xml_find_attribute(object_node, "x")->value.number;
            object.y = (i32)xml_find_attribute(object_node, "y")->value.number;

            xml_attribute_t *attrib;
            if ((attrib = xml_find_attribute(object_node, "width"))) {
                object.width = attrib->value.number;
            } else {
                object.width = 1;
            }

            if ((attrib = xml_find_attribute(object_node, "height"))) {
                object.height = attrib->value.number;
            } else {
                object.height = 1;
            }

#if 0
            object.phy_type = C2_TYPE_POLY;

            object.aabb = (c2AABB){c2V(object.x, object.y), c2V(object.width, object.height)};

            if (object.phy_type == C2_TYPE_POLY) {
                object.phy.poly.verts[0] = (top_left(object.aabb));
                object.phy.poly.verts[1] = (bottom_left(object.aabb));
                object.phy.poly.verts[2] = (bottom_right(object.aabb));
                object.phy.poly.verts[3] = c2Add(top_right(object.aabb), c2Mulvs(bottom_right(object.aabb), 0.5f));
                object.phy.poly.count = 4;
                c2Norms(object.phy.poly.verts, object.phy.poly.norms, object.phy.poly.count);
            }
#endif

            neko_dyn_array_push(object_group.objects, object);
        }

        neko_dyn_array_push(map->object_groups, object_group);
    }
}

void neko_tiled_unload(map_t *map) {

    PROFILE_FUNC();

    for (u32 i = 0; i < neko_dyn_array_size(map->tilesets); i++) {
        gfx_texture_fini(map->tilesets[i].texture);
    }

    for (u32 i = 0; i < neko_dyn_array_size(map->layers); i++) {
        mem_free(map->layers[i].tiles);
    }

    neko_dyn_array_free(map->layers);
    neko_dyn_array_free(map->tilesets);

    for (u32 i = 0; i < neko_dyn_array_size(map->object_groups); i++) {
        neko_dyn_array_free(map->object_groups[i].objects);
    }

    neko_dyn_array_free(map->object_groups);

    xml_free(map->doc);
}

void neko_tiled_render_init(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, const_str vert_src, const_str frag_src) {

    PROFILE_FUNC();

    gfx_vertex_buffer_desc_t vb_decl = {
            .data = NULL,
            .size = BATCH_SIZE * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
            .usage = R_BUFFER_USAGE_DYNAMIC,
    };

    renderer->vb = gfx_vertex_buffer_create(vb_decl);

    gfx_index_buffer_desc_t ib_decl = {
            .data = NULL,
            .size = BATCH_SIZE * IND_PER_QUAD * sizeof(u32),
            .usage = R_BUFFER_USAGE_DYNAMIC,
    };

    renderer->ib = gfx_index_buffer_create(ib_decl);

    if (!vert_src || !frag_src) {
        neko_panic("%s", "Failed to load tiled renderer shaders.");
    }

    gfx_uniform_layout_desc_t u_desc_layout = {.type = R_UNIFORM_SAMPLER2D};

    gfx_uniform_desc_t u_desc = gfx_uniform_desc_t{
            .stage = R_SHADER_STAGE_FRAGMENT,
            .name = "batch_texture",
            .layout = &u_desc_layout,
    };

    renderer->u_batch_tex = gfx_uniform_create(u_desc);

    gfx_shader_source_desc_t shader_src_des[] = {
            {.type = R_SHADER_STAGE_VERTEX, .source = vert_src},
            {.type = R_SHADER_STAGE_FRAGMENT, .source = frag_src},
    };

    renderer->shader = gfx_shader_create(gfx_shader_desc_t{.sources = shader_src_des, .size = 2 * sizeof(gfx_shader_source_desc_t), .name = "tiled_sprite_shader"});

    gfx_uniform_layout_desc_t u_cam_des = {.type = R_UNIFORM_MAT4};
    renderer->u_camera = gfx_uniform_create(gfx_uniform_desc_t{.name = "tiled_sprite_camera", .layout = &u_cam_des});

    gfx_vertex_attribute_desc_t vertex_attr_des[] = {
            {.name = "position", .format = R_VERTEX_ATTRIBUTE_FLOAT2},
            {.name = "uv", .format = R_VERTEX_ATTRIBUTE_FLOAT2},
            {
                    .name = "color",
                    .format = R_VERTEX_ATTRIBUTE_FLOAT4,
            },
            {.name = "use_texture", .format = R_VERTEX_ATTRIBUTE_FLOAT},
    };

    renderer->pip = gfx_pipeline_create(gfx_pipeline_desc_t{.blend = {.func = R_BLEND_EQUATION_ADD, .src = R_BLEND_MODE_SRC_ALPHA, .dst = R_BLEND_MODE_ONE_MINUS_SRC_ALPHA},
                                                            .raster = {.shader = renderer->shader, .index_buffer_element_size = sizeof(uint32_t)},
                                                            .layout = {.attrs = vertex_attr_des, .size = 4 * sizeof(gfx_vertex_attribute_desc_t)}});
}

void neko_tiled_render_deinit(neko_tiled_renderer *renderer) {

    PROFILE_FUNC();

    for (neko_hash_table_iter it = neko_hash_table_iter_new(renderer->quad_table); neko_hash_table_iter_valid(renderer->quad_table, it); neko_hash_table_iter_advance(renderer->quad_table, it)) {
        u32 k = neko_hash_table_iter_getk(renderer->quad_table, it);
        neko_tiled_quad_list_t quad_list = neko_hash_table_iter_get(renderer->quad_table, it);

        neko_dyn_array(neko_tiled_quad_t) v = quad_list.quad_list;

        neko_dyn_array_free(v);
    }

    neko_hash_table_free(renderer->quad_table);

    gfx_shader_fini(renderer->shader);
    // neko_command_buffer_free(cb);
}

void neko_tiled_render_begin(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // gfx_clear_desc_t clear = {.actions = &(gfx_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.0f}}};
    // gfx_clear(cb, &clear);

    renderer->quad_count = 0;
}

void neko_tiled_render_flush(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    PROFILE_FUNC();

    // const neko_vec2 ws = neko_pf_window_sizev(neko_os_main_window());
    // gfx_set_viewport(cb, 0, 0, ws.x, ws.y);

    // renderer->camera_mat = neko_mat4_ortho(0.0f, ws.x, ws.y, 0.0f, -1.0f, 1.0f);

    gfx_bind_vertex_buffer_desc_t vb_des = {.buffer = renderer->vb};
    gfx_bind_index_buffer_desc_t ib_des = {.buffer = renderer->ib};

    gfx_bind_image_buffer_desc_t imgb_des = {renderer->batch_texture, 0, R_ACCESS_READ_ONLY};

    // clang-format off

    gfx_bind_uniform_desc_t uniform_des[2] = {
        {.uniform = renderer->u_camera, .data = &renderer->camera_mat},
        {.uniform = renderer->u_batch_tex, .data = &renderer->batch_texture}
    };

    gfx_bind_desc_t binds = {
    .vertex_buffers = {.desc =&vb_des},
    .index_buffers = {.desc = &ib_des},
    .uniforms = {
        .desc = uniform_des,
        .size = 2 * sizeof(gfx_bind_uniform_desc_t)
    },
    .image_buffers = {
        .desc = &imgb_des,  
        .size = sizeof(gfx_bind_image_buffer_desc_t)
    }};
    // clang-format on

    gfx_pipeline_bind(cb, renderer->pip);
    gfx_apply_bindings(cb, &binds);

    gfx_draw(cb, gfx_draw_desc_t{.start = 0, .count = renderer->quad_count * IND_PER_QUAD});

    // neko_check_gl_error();

    renderer->quad_count = 0;
}

void neko_tiled_render_push(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, neko_tiled_quad_t quad) {

    // PROFILE_FUNC();

    // 如果这个quad的tileset还不存在于quad_table中则插入一个
    // tileset_id为quad_table的键值
    if (!neko_hash_table_exists(renderer->quad_table, quad.tileset_id))                            //
        neko_hash_table_insert(renderer->quad_table, quad.tileset_id, neko_tiled_quad_list_t{0});  //

    //
    neko_tiled_quad_list_t *quad_list = neko_hash_table_getp(renderer->quad_table, quad.tileset_id);
    neko_dyn_array_push(quad_list->quad_list, quad);
}

void neko_tiled_render_draw(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    PROFILE_FUNC();

    // TODO: 23/10/16 检测quad是否在屏幕视角范围外 进行剔除性优化

    // iterate quads hash table
    for (neko_hash_table_iter it = neko_hash_table_iter_new(renderer->quad_table); neko_hash_table_iter_valid(renderer->quad_table, it); neko_hash_table_iter_advance(renderer->quad_table, it)) {
        u32 k = neko_hash_table_iter_getk(renderer->quad_table, it);
        neko_tiled_quad_list_t quad_list = neko_hash_table_iter_get(renderer->quad_table, it);

        neko_dyn_array(neko_tiled_quad_t) v = quad_list.quad_list;

        u32 quad_size = neko_dyn_array_size(v);

        for (u32 i = 0; i < quad_size; i++) {

            neko_tiled_quad_t *quad = &v[i];

            f32 tx = 0.f, ty = 0.f, tw = 0.f, th = 0.f;

            if (quad->use_texture) {
                tx = (f32)quad->rectangle.x / quad->texture_size.x;
                ty = (f32)quad->rectangle.y / quad->texture_size.y;
                tw = (f32)quad->rectangle.z / quad->texture_size.x;
                th = (f32)quad->rectangle.w / quad->texture_size.y;

                // for (u32 i = 0; i < renderer->texture_count; i++) {
                //     if (renderer->textures[i].id == quad->texture.id) {
                //         tex_id = i;
                //         break;
                //     }
                // }

                //// 添加新Tiled贴图
                // if (tex_id == -1) {
                //     renderer->textures[renderer->texture_count] = quad->texture;
                //     tex_id = renderer->texture_count++;
                //     if (renderer->texture_count >= MAX_TEXTURES) {
                //         neko_tiled_render_flush(cb);
                //         tex_id = 0;
                //         renderer->textures[0] = quad->texture;
                //     }
                // }

                renderer->batch_texture = quad->texture;
            }

            const f32 x = quad->position.x;
            const f32 y = quad->position.y;
            const f32 w = quad->dimentions.x;
            const f32 h = quad->dimentions.y;

            const f32 r = (f32)quad->color.r / 255.0f;
            const f32 g = (f32)quad->color.g / 255.0f;
            const f32 b = (f32)quad->color.b / 255.0f;
            const f32 a = (f32)quad->color.a / 255.0f;

            f32 verts[] = {
                    x,     y,     tx,      ty,      r, g, b, a, (f32)quad->use_texture,  //
                    x + w, y,     tx + tw, ty,      r, g, b, a, (f32)quad->use_texture,  //
                    x + w, y + h, tx + tw, ty + th, r, g, b, a, (f32)quad->use_texture,  //
                    x,     y + h, tx,      ty + th, r, g, b, a, (f32)quad->use_texture   //
            };

            const u32 idx_off = renderer->quad_count * VERTS_PER_QUAD;

            u32 indices[] = {idx_off + 3, idx_off + 2, idx_off + 1,   //
                             idx_off + 3, idx_off + 1, idx_off + 0};  //

            gfx_vertex_buffer_request_update(cb, renderer->vb,
                                             gfx_vertex_buffer_desc_t{.data = verts,
                                                                      .size = VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
                                                                      .usage = R_BUFFER_USAGE_DYNAMIC,
                                                                      .update = {.type = R_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32)}});

            gfx_index_buffer_request_update(cb, renderer->ib,
                                            gfx_index_buffer_desc_t{.data = indices,
                                                                    .size = IND_PER_QUAD * sizeof(u32),
                                                                    .usage = R_BUFFER_USAGE_DYNAMIC,
                                                                    .update = {.type = R_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * IND_PER_QUAD * sizeof(u32)}});

            renderer->quad_count++;

            if (renderer->quad_count >= BATCH_SIZE) {
                neko_tiled_render_flush(cb, renderer);
            }
        }

        neko_dyn_array_clear(v);

        neko_tiled_render_flush(cb, renderer);
    }
}

int tiled_render(neko_command_buffer_t *cb, neko_tiled_renderer *tiled_render) {

    PROFILE_FUNC();

    // neko_tiled_renderer *tiled_render = (neko_tiled_renderer *)lua_touserdata(L, 1);

    neko_renderpass_t rp = R_RENDER_PASS_DEFAULT;
    // neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 2);

    // auto xform = lua2struct::unpack<neko_vec2>(L, 3);
    vec2 xform = {};

    // f32 l = lua_tonumber(L, 4);
    // f32 r = lua_tonumber(L, 5);
    // f32 t = lua_tonumber(L, 6);
    // f32 b = lua_tonumber(L, 7);

    f32 l = 0.f;
    f32 r = g_app->width;
    f32 t = 0.f;
    f32 b = g_app->height;

    tiled_render->camera_mat = mat4_ortho(l, r, b, t, -1.0f, 1.0f);

    gfx_renderpass_begin(cb, rp);
    {
        neko_tiled_render_begin(cb, tiled_render);

        PROFILE_BLOCK("tiled_render");

        for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.layers); i++) {
            layer_t *layer = tiled_render->map.layers + i;
            for (u32 y = 0; y < layer->height; y++) {
                for (u32 x = 0; x < layer->width; x++) {
                    tile_t *tile = layer->tiles + (x + y * layer->width);
                    if (tile->id != 0) {
                        tileset_t *tileset = tiled_render->map.tilesets + tile->tileset_id;
                        u32 tsxx = (tile->id % (tileset->width / tileset->tile_width) - 1) * tileset->tile_width;
                        u32 tsyy = tileset->tile_height * ((tile->id - tileset->first_gid) / (tileset->width / tileset->tile_width));
                        neko_tiled_quad_t quad = {.tileset_id = tile->tileset_id,
                                                  .texture = tileset->texture,
                                                  .texture_size = {(f32)tileset->width, (f32)tileset->height},
                                                  .position = {(f32)(x * tileset->tile_width * SPRITE_SCALE) + xform.x, (f32)(y * tileset->tile_height * SPRITE_SCALE) + xform.y},
                                                  .dimentions = {(f32)(tileset->tile_width * SPRITE_SCALE), (f32)(tileset->tile_height * SPRITE_SCALE)},
                                                  .rectangle = {(f32)tsxx, (f32)tsyy, (f32)tileset->tile_width, (f32)tileset->tile_height},
                                                  .color = layer->tint,
                                                  .use_texture = true};
                        neko_tiled_render_push(cb, tiled_render, quad);
                    }
                }
            }
            neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        }

        for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
            object_group_t *group = tiled_render->map.object_groups + i;
            for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
                object_t *object = group->objects + ii;
                neko_tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE) + xform.x, (f32)(object->y * SPRITE_SCALE) + xform.y},
                                          .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
                                          .color = group->color,
                                          .use_texture = false};
                neko_tiled_render_push(cb, tiled_render, quad);
            }
            neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        }

        // for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        //     object_group_t *group = tiled_render->map.object_groups + i;
        //     for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
        //         object_t *object = group->objects + ii;
        //         auto draw_poly = [sprite_batch](c2Poly poly) {
        //             c2v *verts = poly.verts;
        //             int count = poly.count;
        //             for (int i = 0; i < count; ++i) {
        //                 int iA = i;
        //                 int iB = (i + 1) % count;
        //                 c2v a = verts[iA];
        //                 c2v b = verts[iB];
        //                 gl_line(sprite_batch, a.x, a.y, 0, b.x, b.y, 0);
        //             }
        //         };
        //         draw_poly(object->phy.poly);
        //     }
        // }
    }
    gfx_renderpass_end(cb);

    return 0;
}