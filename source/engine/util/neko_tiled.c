
#include "neko_tiled.h"

#include "engine/util/neko_xml.h"

void neko_tiled_load(map_t *map, const_str tmx_path, const_str res_path) {

    neko_xml_document_t *doc = neko_xml_parse_file(tmx_path);
    if (!doc) {
        neko_log_error("Failed to parse XML: %s", neko_xml_get_error());
        return;
    }

    char tmx_root_path[256];
    if (NULL == res_path) {
        neko_util_get_dir_from_file(tmx_root_path, 256, tmx_path);
    } else {
        strcpy(tmx_root_path, res_path);
    }

    neko_xml_node_t *map_node = neko_xml_find_node(doc, "map");
    neko_assert(map_node, "Must have a map node!");

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "tileset"); neko_xml_node_iter_next(&it);) {
        tileset_t tileset = {0};

        tileset.first_gid = (u32)neko_xml_find_attribute(it.current, "firstgid")->value.number;

        char tileset_path[256];
        neko_snprintf(tileset_path, 256, "%s/%s", tmx_root_path, neko_xml_find_attribute(it.current, "source")->value.string);
        neko_xml_document_t *tileset_doc = neko_xml_parse_file(tileset_path);
        if (!tileset_doc) {
            neko_log_error("Failed to parse XML from %s: %s", tileset_path, neko_xml_get_error());
            return;
        }

        neko_xml_node_t *tileset_node = neko_xml_find_node(tileset_doc, "tileset");
        tileset.tile_width = (u32)neko_xml_find_attribute(tileset_node, "tilewidth")->value.number;
        tileset.tile_height = (u32)neko_xml_find_attribute(tileset_node, "tileheight")->value.number;
        tileset.tile_count = (u32)neko_xml_find_attribute(tileset_node, "tilecount")->value.number;

        neko_xml_node_t *image_node = neko_xml_find_node_child(tileset_node, "image");
        const char *image_path = neko_xml_find_attribute(image_node, "source")->value.string;

        char full_image_path[256];
        neko_snprintf(full_image_path, 256, "%s/%s", tmx_root_path, image_path);

        FILE *checker = fopen(full_image_path, "rb"); /* Check that the file exists. */
        if (!checker) {
            neko_log_error("Failed to fopen texture file: %s", full_image_path);
            return;
        }
        fclose(checker);

        void *tex_data = NULL;
        s32 w, h;
        u32 cc;
        neko_util_load_texture_data_from_file(full_image_path, &w, &h, &cc, &tex_data, false);

        neko_graphics_texture_desc_t tileset_tex_decl = {.width = (u32)w,
                                                         .height = (u32)h,
                                                         .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                         .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                         .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                         .num_mips = 0};

        tileset_tex_decl.data[0] = tex_data;

        tileset.texture = neko_graphics_texture_create(&tileset_tex_decl);

        tileset.width = w;
        tileset.height = h;

        neko_free(tex_data);

        neko_dyn_array_push(map->tilesets, tileset);
    }

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "layer"); neko_xml_node_iter_next(&it);) {
        neko_xml_node_t *layer_node = it.current;

        layer_t layer = {0};
        layer.tint = (neko_color_t){255, 255, 255, 255};

        layer.width = (u32)neko_xml_find_attribute(layer_node, "width")->value.number;
        layer.height = (u32)neko_xml_find_attribute(layer_node, "height")->value.number;

        neko_xml_attribute_t *tint_attrib = neko_xml_find_attribute(layer_node, "tintcolor");
        if (tint_attrib) {
            const char *hexstring = tint_attrib->value.string;
            u32 *cols = (u32 *)layer.tint.rgba;
            *cols = (u32)strtol(hexstring + 1, NULL, 16);
            layer.tint.a = 255;
        }

        neko_xml_node_t *data_node = neko_xml_find_node_child(layer_node, "data");

        const char *encoding = neko_xml_find_attribute(data_node, "encoding")->value.string;

        if (strcmp(encoding, "csv") != 0) {
            neko_log_error("%s", "Only CSV data encoding is supported.");
            return;
        }

        const char *data_text = data_node->text;

        const char *cd_ptr = data_text;

        layer.tiles = (tile_t *)neko_safe_malloc(layer.width * layer.height * sizeof(tile_t));

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

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "objectgroup"); neko_xml_node_iter_next(&it);) {
        neko_xml_node_t *object_group_node = it.current;

        object_group_t object_group = {0};
        object_group.color = (neko_color_t){255, 255, 255, 255};

        neko_xml_attribute_t *color_attrib = neko_xml_find_attribute(object_group_node, "color");
        if (color_attrib) {
            const char *hexstring = color_attrib->value.string;
            u32 *cols = (u32 *)object_group.color.rgba;
            *cols = (u32)strtol(hexstring + 1, NULL, 16);
            object_group.color.a = 128;
        }

        for (neko_xml_node_iter_t iit = neko_xml_new_node_child_iter(object_group_node, "object"); neko_xml_node_iter_next(&iit);) {
            neko_xml_node_t *object_node = iit.current;

            object_t object = {0};
            object.id = (s32)neko_xml_find_attribute(object_node, "id")->value.number;
            object.x = (s32)neko_xml_find_attribute(object_node, "x")->value.number;
            object.y = (s32)neko_xml_find_attribute(object_node, "y")->value.number;

            neko_xml_attribute_t *attrib;
            if (attrib = neko_xml_find_attribute(object_node, "width")) {
                object.width = attrib->value.number;
            } else {
                object.width = 1;
            }

            if (attrib = neko_xml_find_attribute(object_node, "height")) {
                object.height = attrib->value.number;
            } else {
                object.height = 1;
            }

            neko_dyn_array_push(object_group.objects, object);
        }

        neko_dyn_array_push(map->object_groups, object_group);
    }

    neko_xml_free(doc);
}

void neko_tiled_unload(map_t *map) {
    for (u32 i = 0; i < neko_dyn_array_size(map->tilesets); i++) {
        neko_graphics_texture_destroy(map->tilesets[i].texture);
    }

    for (u32 i = 0; i < neko_dyn_array_size(map->layers); i++) {
        neko_safe_free(map->layers[i].tiles);
    }

    neko_dyn_array_free(map->layers);
    neko_dyn_array_free(map->tilesets);
}

void neko_tiled_render_init(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, const char *vert_src, const char *frag_src) {

    if (NULL == vert_src || NULL == frag_src) {
        vert_src = neko_read_file_contents("data/shaders/sprite_vs.glsl", "rb", NULL);
        frag_src = neko_read_file_contents("data/shaders/sprite_fs.glsl", "rb", NULL);
    }

    neko_graphics_vertex_buffer_desc_t vb_decl = {
            .data = NULL,
            .size = BATCH_SIZE * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
            .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
    };

    renderer->vb = neko_graphics_vertex_buffer_create(&vb_decl);

    neko_graphics_index_buffer_desc_t ib_decl = {
            .data = NULL,
            .size = BATCH_SIZE * IND_PER_QUAD * sizeof(u32),
            .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
    };

    renderer->ib = neko_graphics_index_buffer_create(&ib_decl);

    if (!vert_src || !frag_src) {
        neko_log_error("%s", "Failed to load tiled renderer shaders.");
    }

    neko_graphics_uniform_desc_t u_desc = (neko_graphics_uniform_desc_t){
            .stage = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "batch_texture",
            .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D},
    };

    renderer->u_batch_tex = neko_graphics_uniform_create(&u_desc);

    renderer->shader = neko_graphics_shader_create(&(neko_graphics_shader_desc_t){.sources =
                                                                                          (neko_graphics_shader_source_desc_t[]){
                                                                                                  {.type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX, .source = vert_src},
                                                                                                  {.type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = frag_src},
                                                                                          },
                                                                                  .size = 2 * sizeof(neko_graphics_shader_source_desc_t),
                                                                                  .name = "tiled_sprite_shader"});

    renderer->u_camera =
            neko_graphics_uniform_create(&(neko_graphics_uniform_desc_t){.name = "tiled_sprite_camera", .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_MAT4}});

    renderer->pip = neko_graphics_pipeline_create(
            &(neko_graphics_pipeline_desc_t){.raster = {.shader = renderer->shader, .index_buffer_element_size = sizeof(uint32_t)},
                                             .layout = {.attrs =
                                                                (neko_graphics_vertex_attribute_desc_t[]){
                                                                        {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "position"},
                                                                        {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "uv"},
                                                                        {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4, .name = "color"},
                                                                        {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT, .name = "use_texture"},
                                                                },
                                                        .size = 4 * sizeof(neko_graphics_vertex_attribute_desc_t)},
                                             .blend = {.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD, .src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA, .dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA}});

    neko_safe_free(vert_src);
    neko_safe_free(frag_src);
}

void neko_tiled_render_deinit(neko_tiled_renderer *renderer) {

    for (neko_hash_table_iter it = neko_hash_table_iter_new(renderer->quad_table); neko_hash_table_iter_valid(renderer->quad_table, it); neko_hash_table_iter_advance(renderer->quad_table, it)) {
        u32 k = neko_hash_table_iter_getk(renderer->quad_table, it);
        neko_tiled_quad_list_t quad_list = neko_hash_table_iter_get(renderer->quad_table, it);

        neko_dyn_array(neko_tiled_quad_t) v = quad_list.quad_list;

        neko_dyn_array_free(v);
    }

    neko_hash_table_free(renderer->quad_table);

    neko_graphics_shader_destroy(renderer->shader);
    // neko_command_buffer_free(cb);
}

void neko_tiled_render_begin(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // neko_graphics_clear_desc_t clear = {.actions = &(neko_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.0f}}};
    // neko_graphics_clear(cb, &clear);

    renderer->quad_count = 0;
}

void neko_tiled_render_flush(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {
    const neko_vec2 ws = neko_platform_window_sizev(neko_platform_main_window());
    neko_graphics_set_viewport(cb, 0, 0, ws.x, ws.y);

    neko_mat4 camera_mat = neko_mat4_ortho(0.0f, ws.x, ws.y, 0.0f, -1.0f, 1.0f);

    // clang-format off
    neko_graphics_bind_desc_t binds = {
    .vertex_buffers = {&(neko_graphics_bind_vertex_buffer_desc_t){.buffer = renderer->vb}},
    .index_buffers = {.desc = &(neko_graphics_bind_index_buffer_desc_t){.buffer = renderer->ib}},
    .uniforms = {
        .desc = (neko_graphics_bind_uniform_desc_t[2]){
            {.uniform = renderer->u_camera, .data = &camera_mat},
            {.uniform = renderer->u_batch_tex, .data = &renderer->batch_texture}
        },
        .size = 2 * sizeof(neko_graphics_bind_uniform_desc_t)
    },
    .image_buffers = {
        .desc = &(neko_graphics_bind_image_buffer_desc_t){renderer->batch_texture, 0, NEKO_GRAPHICS_ACCESS_READ_ONLY},  
        .size = sizeof(neko_graphics_bind_image_buffer_desc_t)
    }};
    // clang-format on

    neko_graphics_pipeline_bind(cb, renderer->pip);
    neko_graphics_apply_bindings(cb, &binds);

    neko_graphics_draw(cb, &(neko_graphics_draw_desc_t){.start = 0, .count = renderer->quad_count * IND_PER_QUAD});

    neko_check_gl_error();

    renderer->quad_count = 0;
}

void neko_tiled_render_push(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, neko_tiled_quad_t quad) {

    if (!neko_hash_table_exists(renderer->quad_table, quad.tileset_id)) neko_hash_table_insert(renderer->quad_table, quad.tileset_id, (neko_tiled_quad_list_t){0});

    neko_tiled_quad_list_t *quad_list = neko_hash_table_getp(renderer->quad_table, quad.tileset_id);

    neko_dyn_array_push(quad_list->quad_list, quad);
}

void neko_tiled_render_draw(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

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

            f32 verts[] = {
                    x,     y,     tx,      ty,      r, g, b, 1.0f, (f32)quad->use_texture,  //
                    x + w, y,     tx + tw, ty,      r, g, b, 1.0f, (f32)quad->use_texture,  //
                    x + w, y + h, tx + tw, ty + th, r, g, b, 1.0f, (f32)quad->use_texture,  //
                    x,     y + h, tx,      ty + th, r, g, b, 1.0f, (f32)quad->use_texture   //
            };

            const u32 idx_off = renderer->quad_count * VERTS_PER_QUAD;

            u32 indices[] = {idx_off + 3, idx_off + 2, idx_off + 1,   //
                             idx_off + 3, idx_off + 1, idx_off + 0};  //

            neko_graphics_vertex_buffer_request_update(
                    cb, renderer->vb,
                    &(neko_graphics_vertex_buffer_desc_t){.data = verts,
                                                          .size = VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
                                                          .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
                                                          .update = {.type = NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32)}});

            neko_graphics_index_buffer_request_update(
                    cb, renderer->ib,
                    &(neko_graphics_index_buffer_desc_t){.data = indices,
                                                         .size = IND_PER_QUAD * sizeof(u32),
                                                         .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
                                                         .update = {.type = NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * IND_PER_QUAD * sizeof(u32)}});

            renderer->quad_count++;

            if (renderer->quad_count >= BATCH_SIZE) {
                neko_tiled_render_flush(cb, renderer);
            }
        }

        neko_dyn_array_clear(v);

        neko_tiled_render_flush(cb, renderer);
    }
}
