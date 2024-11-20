

#include "tiledmap.hpp"

#include "base/common/json.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/ecs/entitybase.hpp"

// deps
#include <box2d/box2d.h>

static const_str S_UNKNOWN = "unknown";

static bool layer_from_json(TilemapLayer *layer, JSON *json, bool *ok, Arena *arena, String filepath, HashMap<AssetTexture> *images) {
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

        AssetTexture *img = images->get(key);
        if (img != nullptr) {
            layer->image = *img;
        } else {
            AssetTexture create_img = {};
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

static bool level_from_json(TilemapLevel *level, JSON *json, bool *ok, Arena *arena, String filepath, HashMap<AssetTexture> *images) {
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
    HashMap<AssetTexture> images = {};
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
    neko_assert(filled.data);
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

// DECL_ENT(Tiled, tiled_renderer *render; vec2 pos; String map_name;);

Asset Tiled::tiled_shader = {};

bool tiled_load(TiledMap *map, const_str tmx_path, const_str res_path) {

    PROFILE_FUNC();

    map->doc.ParseVFS(tmx_path);
    // if (!map->doc) {
    //     neko_panic("Failed to parse XML");
    //     return false;
    // }

    char tmx_root_path[256];
    if (NULL == res_path) {
        neko_util_get_dir_from_file(tmx_root_path, 256, tmx_path);
    } else {
        strcpy(tmx_root_path, res_path);
    }

    XMLNode *map_node = map->doc.FindNode("map");
    neko_assert(map_node);  // Must have a map node!

    for (auto it = map_node->MakeChildIter("tileset"); it.Next();) {
        tileset_t tileset = {0};

        tileset.first_gid = it.current->Attribute<double>("firstgid");

        char tileset_path[256];
        neko_snprintf(tileset_path, 256, "%s/%s", tmx_root_path, it.current->Attribute<String>("source").data);
        XMLDoc tileset_doc;
        tileset_doc.ParseVFS(tileset_path);
        // if (!tileset_doc) {
        //     neko_panic("Failed to parse XML from %s", tileset_path);
        //     return false;
        // }

        XMLNode *tileset_node = tileset_doc.FindNode("tileset");
        tileset.tile_width = tileset_node->Attribute<double>("tilewidth");
        tileset.tile_height = tileset_node->Attribute<double>("tileheight");
        tileset.tile_count = tileset_node->Attribute<double>("tilecount");

        XMLNode *image_node = tileset_node->FindChild("image");
        String image_path = image_node->Attribute<String>("source");

        char full_image_path[256];
        neko_snprintf(full_image_path, 256, "%s/%s", tmx_root_path, image_path.data);

        bool ok = neko_capi_vfs_file_exists(NEKO_PACKS::GAMEDATA, full_image_path);
        if (!ok) {
            neko_panic("failed to load texture file: %s", full_image_path);
            return false;
        }

        size_t len = 0;
        const_str tex_data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, full_image_path, &len);
        neko_assert(tex_data);

        neko_init_texture_from_memory(&tileset.texture, (u8 *)tex_data, len, neko_texture_flags_t(NEKO_TEXTURE_ALIASED | NEKO_TEXTURE_NO_FLIP_VERTICAL));

        tileset.width = tileset.texture.width;
        tileset.height = tileset.texture.height;

        mem_free(tex_data);

        map->tilesets.push(tileset);

        tileset_doc.Trash();
    }

    for (auto it = map_node->MakeChildIter("layer"); it.Next();) {
        XMLNode *layer_node = it.current;

        layer_t layer = {0};
        layer.tint = color256(255, 255, 255, 255);

        layer.width = layer_node->Attribute<double>("width");
        layer.height = layer_node->Attribute<double>("height");

        XMLAttribute *tint_attrib = layer_node->FindAttribute("tintcolor");
        if (tint_attrib) {
            String hexstring = std::get<String>(tint_attrib->value);
            u32 *cols = (u32 *)layer.tint.rgba;
            *cols = (u32)strtol(hexstring.data + 1, NULL, 16);
            layer.tint.a = 255;
        }

        XMLNode *data_node = layer_node->FindChild("data");

        String encoding = data_node->Attribute<String>("encoding");

        if (strcmp(encoding.data, "csv") != 0) {
            neko_panic("%s", "Only CSV data encoding is supported.");
            return false;
        }

        String data_text = data_node->text;

        const char *cd_ptr = data_text.data;

        layer.tiles = (tile_t *)mem_alloc(layer.width * layer.height * sizeof(tile_t));

        for (u32 y = 0; y < layer.height; y++) {
            for (u32 x = 0; x < layer.width; x++) {
                u32 gid = (u32)strtod(cd_ptr, NULL);
                u32 tls_id = 0;

                u32 closest = 0;
                for (u32 i = 0; i < map->tilesets.len; i++) {
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

        map->layers.push(layer);
    }

    for (auto it = map_node->MakeChildIter("objectgroup"); it.Next();) {
        XMLNode *object_group_node = it.current;

        object_group_t object_group = {};
        object_group.color = color256(255, 255, 255, 255);

        // 对象组名字
        XMLAttribute *name_attrib = object_group_node->FindAttribute("name");
        if (name_attrib) {
            object_group.name = std::get<String>(name_attrib->value);
            // u32 *cols = (u32 *)object_group.color.rgba;
            //*cols = (u32)strtol(hexstring + 1, NULL, 16);
            // object_group.color.a = 128;
            console_log("objectgroup: %s", object_group.name.cstr());

        } else {
            object_group.name = "unknown";
        }

        // 对象组默认颜色
        XMLAttribute *color_attrib = object_group_node->FindAttribute("color");
        if (color_attrib) {
            String hexstring = std::get<String>(color_attrib->value);
            u32 *cols = (u32 *)object_group.color.rgba;
            *cols = (u32)strtol(hexstring.data + 1, NULL, 16);
            object_group.color.a = 128;
        }

        // 对象
        for (auto iit = object_group_node->MakeChildIter("object"); iit.Next();) {
            XMLNode *object_node = iit.current;

            object_t object{.defs_luatb = {ENGINE_LUA()}};
            object.id = object_node->Attribute<double>("id");
            object.x = object_node->Attribute<double>("x");
            object.y = object_node->Attribute<double>("y");

            XMLAttribute *attrib;
            if ((attrib = object_node->FindAttribute("width"))) {
                object.width = std::get<double>(attrib->value);
            } else {
                object.width = 1;
            }

            if ((attrib = object_node->FindAttribute("height"))) {
                object.height = std::get<double>(attrib->value);
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

            if ((attrib = object_node->FindAttribute("name"))) {
                object.name = std::get<String>(attrib->value);
            } else {
                object.name = S_UNKNOWN;
            }

            if ((attrib = object_node->FindAttribute("type"))) {
                object.class_name = std::get<String>(attrib->value);
            } else {
                object.class_name = S_UNKNOWN;
            }

            console_log("%s %s %s", object_group.name.cstr(), object.name.cstr(), object.class_name.cstr());

            XMLNode *properties = object_node->FindChild("properties");
            if (properties) {
                auto L = ENGINE_LUA();
                object.defs_luatb = LuaRef::NewTable(L);
                for (auto it = properties->MakeChildIter("property"); it.Next();) {
                    XMLNode *object_node = it.current;
                    String property_name = std::get<String>(object_node->FindAttribute("name")->value);
                    XMLAttribute *type_attr = object_node->FindAttribute("type");

                    if (type_attr) {
                        String type_name = std::get<String>(type_attr->value);
                    } else {

                        XMLAttribute *value_attr = object_node->FindAttribute("value");

                        if (!value_attr) continue;

                        // lua表
                        object.defs_luatb[property_name.cstr()] = std::get<String>(value_attr->value).cstr();

                        object.defs[fnv1a(property_name)] = std::get<String>(value_attr->value);

                        console_log("%s=%s", property_name.cstr(), object.defs[fnv1a(property_name)].cstr());
                    }
                }
            }

            object_group.objects.push(object);
        }

        map->object_groups.push(object_group);
    }

    return true;
}

void tiled_unload(TiledMap *map) {

    PROFILE_FUNC();

    for (u32 i = 0; i < map->tilesets.len; i++) {
        // gfx_texture_fini(map->tilesets[i].texture);
    }

    for (u32 i = 0; i < map->layers.len; i++) {
        mem_free(map->layers[i].tiles);
    }

    map->layers.trash();
    map->tilesets.trash();

    for (u32 i = 0; i < map->object_groups.len; i++) {
        for (u32 j = 0; j < map->object_groups[i].objects.len; j++) {
            map->object_groups[i].objects[j].defs.trash();
        }
        map->object_groups[i].objects.trash();
    }

    map->object_groups.trash();

    if (map->doc.nodes.len) {
        map->doc.Trash();
    }
}

void tiled_render_init(tiled_renderer *renderer) {

    // hashmap_init(&renderer->quad_table);

    GLuint sid = Tiled::tiled_shader.shader.id;

    PROFILE_FUNC();

    glUseProgram(sid);

    GLuint loc = glGetUniformLocation(sid, "inverse_view_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    loc = glGetUniformLocation(sid, "batch_texture");
    glUniform1i(loc, 0);

    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);

    glGenBuffers(1, &renderer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

    glBufferData(GL_ARRAY_BUFFER, BATCH_SIZE * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &renderer->ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ib);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, BATCH_SIZE * IND_PER_QUAD * sizeof(u32), NULL, GL_DYNAMIC_DRAW);

    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 2, "position", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 0));
    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 2, "uv", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 2));
    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 4, "color", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 4));
    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 1, "use_texture", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 8));
}

void tiled_render_deinit(tiled_renderer *renderer) {
    PROFILE_FUNC();

    for (auto kv : renderer->quad_table) {
        u32 k = kv.key;
        tiled_quad_list_t *quad_list = kv.value;

        quad_list->quad_list.trash();
    }

    renderer->quad_table.trash();

    glDeleteBuffers(1, &renderer->ib);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
}

void tiled_render_begin(tiled_renderer *renderer) { renderer->quad_count = 0; }

void tiled_render_flush(tiled_renderer *renderer) {

    PROFILE_FUNC();

    GLuint sid = Tiled::tiled_shader.shader.id;

    glUseProgram(sid);

    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ib);

    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 2, "position", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 0));
    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 2, "uv", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 2));
    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 4, "color", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 4));
    gfx_bind_vertex_attrib_auto(sid, GL_FLOAT, 1, "use_texture", FLOATS_PER_VERT * sizeof(f32), NEKO_INT2VOIDP(sizeof(f32) * 8));

    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, false, (float *)&renderer->camera_mat);

    GLuint tex_id = renderer->batch_texture.id;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    // glUniform1iv(glGetUniformLocation(sid, "batch_texture"), 1, 0);

    glBindImageTexture(0, tex_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glDrawRangeElementsBaseVertex(GL_TRIANGLES, 0, renderer->quad_count * IND_PER_QUAD, renderer->quad_count * IND_PER_QUAD, GL_UNSIGNED_INT, NEKO_INT2VOIDP(0), NULL);

    renderer->quad_count = 0;
}

void tiled_render_push(tiled_renderer *renderer, tiled_quad_t quad) {

    // PROFILE_FUNC();

    // 如果这个quad的tileset还不存在于quad_table中则插入一个
    // tileset_id为quad_table的键值

    // if (!neko_hash_table_exists(renderer->quad_table, quad.tileset_id))                       //
    //     neko_hash_table_insert(renderer->quad_table, quad.tileset_id, tiled_quad_list_t{0});  //

    //

    tiled_quad_list_t *quad_list = renderer->quad_table.get(quad.tileset_id);
    if (NULL == quad_list) {
        // *quad_list = tiled_quad_list_t{0};
        renderer->quad_table[quad.tileset_id] = tiled_quad_list_t{0};

        quad_list = renderer->quad_table.get(quad.tileset_id);
    }
    quad_list->quad_list.push(quad);
}

void tiled_render_draw(tiled_renderer *renderer) {

    PROFILE_FUNC();

    // TODO: 24/8/20 检测quad是否在屏幕视角范围外 进行剔除性优化

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ib);

    // iterate quads hash table
    for (auto kv : renderer->quad_table) {
        u32 k = kv.key;
        tiled_quad_list_t *quad_list = kv.value;

        for (u32 i = 0; i < quad_list->quad_list.len; i++) {

            tiled_quad_t *quad = &quad_list->quad_list[i];

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
                //         tiled_render_flush(cb);
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

            glBufferSubData(GL_ARRAY_BUFFER, renderer->quad_count * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32), VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32), verts);

            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, renderer->quad_count * IND_PER_QUAD * sizeof(u32), IND_PER_QUAD * sizeof(u32), indices);

            renderer->quad_count++;

            if (renderer->quad_count >= BATCH_SIZE) {
                tiled_render_flush(renderer);
            }
        }

        quad_list->quad_list.len = 0;

        tiled_render_flush(renderer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void tiled_map_edit_w(Tiled *tiled, u32 layer_idx, u32 x, u32 y, u32 id) {

    Asset asset = {};
    bool ok = asset_read(tiled->render->map_asset, &asset);
    error_assert(ok);

    TiledMap map = asset.tiledmap;

    error_assert(layer_idx < map.layers.len);
    layer_t *layer = &map.layers[layer_idx];
    error_assert(x < layer->width && y < layer->height);

    tile_t *tile = layer->tiles + (x + y * layer->width);

    tileset_t *tileset = &map.tilesets[tile->tileset_id];

    tile->id = id + tileset->first_gid;

    asset_write(asset);
}

void tiled_map_edit(NativeEntity ent, u32 layer_idx, u32 x, u32 y, u32 id) {
    Tiled *tiled = Tiled::pool->Get(ent);
    error_assert(tiled);

    tiled_map_edit_w(tiled, layer_idx, x, y, id);
}

int tiled_render(Tiled *tiled) {

    PROFILE_FUNC();

    // auto xform = lua2struct::unpack<neko_vec2>(L, 3);
    vec2 xform = tiled->pos;

    tiled->render->camera_mat = camera_get_inverse_view_matrix();

    Asset asset = {};
    bool ok = asset_read(tiled->render->map_asset, &asset);
    error_assert(ok);

    TiledMap map = asset.tiledmap;

    {
        tiled_render_begin(tiled->render);

        PROFILE_BLOCK("tiled_render");

        for (u32 i = 0; i < map.layers.len; i++) {
            layer_t *layer = &map.layers[i];
            for (u32 y = 0; y < layer->height; y++) {
                for (u32 x = 0; x < layer->width; x++) {
                    tile_t *tile = layer->tiles + (x + y * layer->width);
                    if (tile->id != 0) {
                        tileset_t *tileset = &map.tilesets[tile->tileset_id];
                        u32 tsxx = (tile->id % (tileset->width / tileset->tile_width) - 1) * tileset->tile_width;
                        u32 tsyy = tileset->tile_height * ((tile->id - tileset->first_gid) / (tileset->width / tileset->tile_width));
                        tiled_quad_t quad = {.tileset_id = tile->tileset_id,
                                             .texture = tileset->texture,
                                             .texture_size = {(f32)tileset->width, (f32)tileset->height},
                                             .position = {(f32)(x * tileset->tile_width * SPRITE_SCALE) + xform.x, (f32)(y * tileset->tile_height * SPRITE_SCALE) + xform.y},
                                             .dimentions = {(f32)(tileset->tile_width * SPRITE_SCALE), (f32)(tileset->tile_height * SPRITE_SCALE)},
                                             .rectangle = {(f32)tsxx, (f32)tsyy, (f32)tileset->tile_width, (f32)tileset->tile_height},
                                             .color = layer->tint,
                                             .use_texture = true};
                        tiled_render_push(tiled->render, quad);
                    }
                }
            }
            tiled_render_draw(tiled->render);  // 一层渲染一次
        }

        for (u32 i = 0; i < map.object_groups.len; i++) {
            object_group_t *group = &map.object_groups[i];
            for (u32 ii = 0; ii < map.object_groups[i].objects.len; ii++) {
                object_t *object = &group->objects[ii];
                tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE) + xform.x, (f32)(object->y * SPRITE_SCALE) + xform.y},
                                     .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
                                     .color = group->color,
                                     .use_texture = false};
                tiled_render_push(tiled->render, quad);
            }
            tiled_render_draw(tiled->render);  // 一层渲染一次
        }

        // for (u32 i = 0; i < neko_dyn_array_size(map.object_groups); i++) {
        //     object_group_t *group = map.object_groups + i;
        //     for (u32 ii = 0; ii < neko_dyn_array_size(map.object_groups[i].objects); ii++) {
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

    return 0;
}

void tiled_add(NativeEntity ent) {

    if (Tiled::pool->Get(ent)) return;

    transform_add(ent);

    Tiled *tiled = Tiled::pool->Add(ent);

    tiled->render = (tiled_renderer *)mem_alloc(sizeof(tiled_renderer));
    memset(tiled->render, 0, sizeof(tiled_renderer));

    tiled_render_init(tiled->render);
}

void tiled_remove(NativeEntity ent) { Tiled::pool->Remove(ent); }

bool tiled_has(NativeEntity ent) { return Tiled::pool->Get(ent) != NULL; }

void tiled_init() {
    PROFILE_FUNC();

    Tiled::pool = entitypool_new<Tiled>();

    bool ok = asset_load_kind(AssetKind_Shader, "shader/tiled.glsl", &Tiled::tiled_shader);
    error_assert(ok);
}

void tiled_fini() {

    Tiled::pool->ForEach([](Tiled *tiled) {
        // tiled_unload(&tiled->render->map);
        tiled_render_deinit(tiled->render);
        mem_free(tiled->map_name.data);
        mem_free(tiled->render);
    });

    entitypool_free(Tiled::pool);
}

int tiled_update_all(App *app, event_t evt) {

    entitypool_remove_destroyed(Tiled::pool, tiled_remove);

    Tiled::pool->ForEach([](Tiled *tiled) { tiled->pos = transform_get_position(tiled->pool_elem.ent); });

    return 0;
}

void tiled_draw_all() {

    Tiled::pool->ForEach([](Tiled *tiled) { tiled_render(tiled); });
}

void tiled_set_map(NativeEntity ent, const char *str) {
    Tiled *tiled = Tiled::pool->Get(ent);
    error_assert(tiled);
    tiled->map_name = to_cstr(String(str));

    Asset asset = {};
    bool ok = asset_load_kind(AssetKind_Tiledmap, tiled->map_name, &asset);
    error_assert(ok);

    tiled->render->map_asset = asset.hash;
}

const char *tiled_get_map(NativeEntity ent) {
    Tiled *tiled = Tiled::pool->Get(ent);
    error_assert(tiled);
    return tiled->map_name.cstr();
}

void tiled_loadlua(NativeEntity ent) {
    Tiled *tiled = Tiled::pool->Get(ent);
    error_assert(tiled);

    Asset asset = {};
    bool ok = asset_read(tiled->render->map_asset, &asset);
    error_assert(ok);

    TiledMap map = asset.tiledmap;

    // LuaRef &table = map.object_groups;

    for (auto &obj_group : map.object_groups)
        for (object_t &obj : obj_group.objects) {
        }

    return;
}