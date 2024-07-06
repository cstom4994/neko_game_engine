
#include "engine/neko_asset.h"
#include "engine/neko_common.h"

bool neko_aseprite_load(neko_aseprite *spr, const_str filepath) {

    ase_t *ase = neko_aseprite_load_from_file(filepath);

    if (NULL == ase) {
        NEKO_ERROR("unable to load ase %s", filepath);
        return false;
    }

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;

        frame->pixels[0] = (neko_color_t *)neko_safe_malloc((int)(sizeof(neko_color_t)) * ase->w * ase->h);

        spr->mem_used += (sizeof(neko_color_t)) * ase->w * ase->h;

        memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t *dst = frame->pixels[0];

        // neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t *cel = frame->cels + j;

            // neko_println_debug(" - %s", cel->layer->name);

            // 确定图块所在层与父层可视
            if (!(cel->layer->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE))) {
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

            void *src = cel->cel_pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -NEKO_MIN(cx, 0);
            int ct = -NEKO_MIN(cy, 0);
            int dl = NEKO_MAX(cx, 0);
            int dt = NEKO_MAX(cy, 0);
            int dr = NEKO_MIN(ase->w, cw + cx);
            int db = NEKO_MIN(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    neko_color_t src_color = s_color(ase, src, cw * sy + sx);
                    neko_color_t dst_color = dst[dst_index];
                    neko_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }

    s32 rect = ase->w * ase->h * 4;

    neko_aseprite s = NEKO_DEFAULT_VAL();

    // neko_array<neko_sprite_frame> frames = {};
    // neko_array_reserve(&frames, ase->frame_count);

    // neko_array<u8> pixels = {};
    // neko_array_reserve(&pixels, ase->frame_count * rect);
    //  neko_defer([&] { neko_array_dctor(&pixels); });

    u8 *pixels = neko_safe_malloc(ase->frame_count * rect);

    for (s32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t *frame = &ase->frames[i];

        neko_aseprite_frame sf = NEKO_DEFAULT_VAL();
        sf.duration = frame->duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (f32)(i + 1) / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (f32)i / ase->frame_count;

        neko_dyn_array_push(s.frames, sf);

        neko_color_t *data = frame->pixels[0];

        // 不知道是直接读取aseprite解析的数据还是像这样拷贝为一个贴图 在渲染时改UV来得快
        memcpy(pixels + (i * rect), &data[0].r, rect);
    }

    neko_render_texture_desc_t t_desc = NEKO_DEFAULT_VAL();

    t_desc.format = R_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = R_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = R_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h * ase->frame_count;
    // t_desc.num_comps = 4;

    // 大小为 ase->frame_count * rect
    // neko_tex_flip_vertically(ase->w, ase->h * ase->frame_count, (u8 *)pixels.data);
    t_desc.data[0] = pixels;

    neko_texture_t tex = neko_render_texture_create(t_desc);

    neko_safe_free(pixels);

    // img.width = desc.width;
    // img.height = desc.height;

    // neko_hashmap<neko_sprite_loop> by_tag;
    // neko_hash_table(u64, neko_sprite_loop) by_tag;
    // neko_hashmap_reserve(&by_tag, neko_hashmap_reserve_size((u64)ase->tag_count));

    for (s32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t *tag = &ase->tags[i];

        neko_aseprite_loop loop = NEKO_DEFAULT_VAL();

        for (s32 j = tag->from_frame; j <= tag->to_frame; j++) {
            neko_dyn_array_push(loop.indices, j);
        }

        u64 key = neko_hash_str64(tag->name /*, strlen(tag.name)*/);
        neko_hash_table_insert(s.by_tag, key, loop);
    }

    // for (s32 i = 0; i < ase->layer_count; i++) {
    //     ase_layer_t* layer = &ase->layers[i];
    //     neko_println_debug("%s", layer->name);
    // }

    // NEKO_TRACE(format("created sprite size({3}) with image id: {0} and {1} frames with {2} layers", tex.id, neko_dyn_array_size(s.frames), ase->layer_count,(spr->mem_used + pixels.capacity *
    // sizeof(u8)) / 1e6).c_str());

    s.img = tex;
    // s.frames = frames;
    //  s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *spr = s;

    neko_aseprite_free(ase);

    return true;
}

void neko_aseprite_end(neko_aseprite *spr) {
    neko_dyn_array_free(spr->frames);

    for (neko_hash_table_iter it = neko_hash_table_iter_new(spr->by_tag); neko_hash_table_iter_valid(spr->by_tag, it); neko_hash_table_iter_advance(spr->by_tag, it)) {
        u64 key = neko_hash_table_iter_getk(spr->by_tag, it);
        neko_aseprite_loop *v = neko_hash_table_getp(spr->by_tag, key);
        neko_dyn_array_free(v->indices);
    }

    neko_hash_table_free(spr->by_tag);
}

void neko_aseprite_renderer_play(neko_aseprite_renderer *sr, const_str tag) {
    neko_aseprite_loop *loop = neko_hash_table_getp(sr->sprite->by_tag, neko_hash_str64(tag));
    if (loop != NULL) sr->loop = loop;
    sr->current_frame = 0;
    sr->elapsed = 0;
}

void neko_aseprite_renderer_update(neko_aseprite_renderer *sr, f32 dt) {
    s32 index;
    u64 len;
    if (sr->loop) {
        index = sr->loop->indices[sr->current_frame];
        len = neko_dyn_array_size(sr->loop->indices);
    } else {
        index = sr->current_frame;
        len = neko_dyn_array_size(sr->sprite->frames);
    }

    neko_aseprite_frame frame = sr->sprite->frames[index];

    sr->elapsed += dt * 1000;
    if (sr->elapsed > frame.duration) {
        if (sr->current_frame == len - 1) {
            sr->current_frame = 0;
        } else {
            sr->current_frame++;
        }

        sr->elapsed -= frame.duration;
    }
}

void neko_aseprite_renderer_set_frame(neko_aseprite_renderer *sr, s32 frame) {
    s32 len;
    if (sr->loop) {
        len = neko_dyn_array_size(sr->loop->indices);
    } else {
        len = neko_dyn_array_size(sr->sprite->frames);
    }

    if (0 <= frame && frame < len) {
        sr->current_frame = frame;
        sr->elapsed = 0;
    }
}

neko_texture_t neko_aseprite_simple(const void *memory, int size) {
    neko_image_t image = neko_image_load_mem(memory, size, "simple.ase");
    NEKO_ASSERT(image.w != 0 && image.h != 0, "good image");
    ase_t *ase = image.ase;
    neko_render_texture_desc_t t_desc = {};

    t_desc.format = R_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = R_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = R_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    // t_desc.num_comps = 4;
    t_desc.data[0] = ase->frames->pixels[0];

    neko_tex_flip_vertically(ase->w, ase->h, (u8 *)(t_desc.data[0]));
    neko_texture_t tex = neko_render_texture_create(t_desc);
    neko_aseprite_free(ase);
    return tex;
}

void neko_fontbatch_init(neko_fontbatch_t *font_batch, const_str font_vs, const_str font_ps, const neko_vec2_t fbs, const_str img_path, char *content, int content_size) {

    font_batch->font_scale = 3.0f;

    font_batch->font_render = neko_render_batch_make_ctx(32);

    neko_render_batch_vertex_data_t font_vd;
    neko_render_batch_make_vertex_data(&font_vd, 1024 * 1024, GL_TRIANGLES, sizeof(neko_font_vert_t), GL_DYNAMIC_DRAW);
    neko_render_batch_add_attribute(&font_vd, "in_pos", 2, R_BATCH_FLOAT, NEKO_OFFSET(neko_font_vert_t, x));
    neko_render_batch_add_attribute(&font_vd, "in_uv", 2, R_BATCH_FLOAT, NEKO_OFFSET(neko_font_vert_t, u));

    neko_render_batch_make_renderable(&font_batch->font_renderable, &font_vd);
    neko_render_batch_load_shader(&font_batch->font_shader, font_vs, font_ps);
    neko_render_batch_set_shader(&font_batch->font_renderable, &font_batch->font_shader);

    neko_render_batch_ortho_2d(fbs.x / font_batch->font_scale, fbs.y / font_batch->font_scale, 0, 0, font_batch->font_projection);

    neko_render_batch_send_matrix(&font_batch->font_shader, "u_mvp", font_batch->font_projection);

    neko_image_t img = neko_image_load(img_path);
    font_batch->font_tex_id = generate_texture_handle(img.pix, img.w, img.h, NULL);
    font_batch->font = neko_font_load_bmfont(font_batch->font_tex_id, content, content_size, 0);
    if (font_batch->font->atlas_w != img.w || font_batch->font->atlas_h != img.h) {
        NEKO_WARN("failed to load font");
    }
    neko_image_free(img);

    font_batch->font_verts = (neko_font_vert_t *)neko_safe_malloc(sizeof(neko_font_vert_t) * 1024 * 2);
}

void neko_fontbatch_end(neko_fontbatch_t *font_batch) {
    neko_safe_free(font_batch->font_verts);
    neko_font_free(font_batch->font);
    neko_render_batch_free(font_batch->font_render);
    destroy_texture_handle(font_batch->font_tex_id, NULL);
}

void neko_fontbatch_draw(neko_fontbatch_t *font_batch, const neko_vec2_t fbs, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale) {
    f32 text_w = (f32)neko_font_text_width(font_batch->font, text);
    f32 text_h = (f32)neko_font_text_height(font_batch->font, text);

    if (scale == 0.f) scale = font_batch->font_scale;

    neko_font_rect_t clip_rect;
    clip_rect.left = -fbs.x / scale * clip_region;
    clip_rect.right = fbs.x / scale * clip_region + 0.5f;
    clip_rect.top = fbs.y / scale * clip_region + 0.5f;
    clip_rect.bottom = -fbs.y / scale * clip_region;

    f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / scale + text_h / 2.f;
    f32 wrap_width = wrap_x - x0;

    neko_font_fill_vertex_buffer(font_batch->font, text, x0, y0, wrap_width, line_height, &clip_rect, font_batch->font_verts, 1024 * 2, &font_batch->font_vert_count);

    if (font_batch->font_vert_count) {
        neko_render_batch_draw_call_t call;
        call.textures[0] = (u32)font_batch->font->atlas_id;
        call.texture_count = 1;
        call.r = &font_batch->font_renderable;
        call.verts = font_batch->font_verts;
        call.vert_count = font_batch->font_vert_count;

        neko_render_batch_push_draw_call(font_batch->font_render, call);
    }
}

typedef struct ct_text {
    neko_fontbatch_t *fontbatch;
    const_str text;
} ct_text;

void neko_tiled_load(map_t *map, const_str tmx_path, const_str res_path) {

    map->doc = neko_xml_parse_file(tmx_path);
    if (!map->doc) {
        NEKO_ERROR("Failed to parse XML: %s", neko_xml_get_error());
        return;
    }

    char tmx_root_path[256];
    if (NULL == res_path) {
        neko_util_get_dir_from_file(tmx_root_path, 256, tmx_path);
    } else {
        strcpy(tmx_root_path, res_path);
    }

    neko_xml_node_t *map_node = neko_xml_find_node(map->doc, "map");
    NEKO_ASSERT(map_node);  // Must have a map node!

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "tileset"); neko_xml_node_iter_next(&it);) {
        tileset_t tileset = {0};

        tileset.first_gid = (u32)neko_xml_find_attribute(it.current, "firstgid")->value.number;

        char tileset_path[256];
        neko_snprintf(tileset_path, 256, "%s/%s", tmx_root_path, neko_xml_find_attribute(it.current, "source")->value.string);
        neko_xml_document_t *tileset_doc = neko_xml_parse_file(tileset_path);
        if (!tileset_doc) {
            NEKO_ERROR("Failed to parse XML from %s: %s", tileset_path, neko_xml_get_error());
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

        bool ok = neko_capi_vfs_file_exists(NEKO_PACK_GAMEDATA, full_image_path);
        if (!ok) {
            NEKO_ERROR("failed to load texture file: %s", full_image_path);
            return;
        }

        void *tex_data = NULL;
        s32 w, h;
        u32 cc;
        neko_util_load_texture_data_from_file(full_image_path, &w, &h, &cc, &tex_data, false);

        neko_render_texture_desc_t tileset_tex_decl = {
                .width = (u32)w, .height = (u32)h, .format = R_TEXTURE_FORMAT_RGBA8, .min_filter = R_TEXTURE_FILTER_NEAREST, .mag_filter = R_TEXTURE_FILTER_NEAREST, .num_mips = 0};

        tileset_tex_decl.data[0] = tex_data;

        tileset.texture = neko_render_texture_create(tileset_tex_decl);

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
            NEKO_ERROR("%s", "Only CSV data encoding is supported.");
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

        // 对象组名字
        neko_xml_attribute_t *name_attrib = neko_xml_find_attribute(object_group_node, "name");
        if (name_attrib) {
            const char *namestring = name_attrib->value.string;

            object_group.name = name_attrib->value.string;
            // u32 *cols = (u32 *)object_group.color.rgba;
            //*cols = (u32)strtol(hexstring + 1, NULL, 16);
            // object_group.color.a = 128;
            neko_println("objectgroup: %s", namestring);
        } else {
        }

        // 对象组默认颜色
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
            if ((attrib = neko_xml_find_attribute(object_node, "width"))) {
                object.width = attrib->value.number;
            } else {
                object.width = 1;
            }

            if ((attrib = neko_xml_find_attribute(object_node, "height"))) {
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
    for (u32 i = 0; i < neko_dyn_array_size(map->tilesets); i++) {
        neko_render_texture_destroy(map->tilesets[i].texture);
    }

    for (u32 i = 0; i < neko_dyn_array_size(map->layers); i++) {
        neko_safe_free(map->layers[i].tiles);
    }

    neko_dyn_array_free(map->layers);
    neko_dyn_array_free(map->tilesets);

    neko_dyn_array_free(map->object_groups);

    neko_xml_free(map->doc);
}

void neko_tiled_render_init(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, const_str vert_src, const_str frag_src) {

    neko_render_vertex_buffer_desc_t vb_decl = {
            .data = NULL,
            .size = BATCH_SIZE * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
            .usage = R_BUFFER_USAGE_DYNAMIC,
    };

    renderer->vb = neko_render_vertex_buffer_create(vb_decl);

    neko_render_index_buffer_desc_t ib_decl = {
            .data = NULL,
            .size = BATCH_SIZE * IND_PER_QUAD * sizeof(u32),
            .usage = R_BUFFER_USAGE_DYNAMIC,
    };

    renderer->ib = neko_render_index_buffer_create(ib_decl);

    if (!vert_src || !frag_src) {
        NEKO_ERROR("%s", "Failed to load tiled renderer shaders.");
    }

    neko_render_uniform_desc_t u_desc = (neko_render_uniform_desc_t){
            .stage = R_SHADER_STAGE_FRAGMENT,
            .name = "batch_texture",
            .layout = &(neko_render_uniform_layout_desc_t){.type = R_UNIFORM_SAMPLER2D},
    };

    renderer->u_batch_tex = neko_render_uniform_create(u_desc);

    renderer->shader = neko_render_shader_create((neko_render_shader_desc_t){.sources =
                                                                                     (neko_render_shader_source_desc_t[]){
                                                                                             {.type = R_SHADER_STAGE_VERTEX, .source = vert_src},
                                                                                             {.type = R_SHADER_STAGE_FRAGMENT, .source = frag_src},
                                                                                     },
                                                                             .size = 2 * sizeof(neko_render_shader_source_desc_t),
                                                                             .name = "tiled_sprite_shader"});

    renderer->u_camera = neko_render_uniform_create((neko_render_uniform_desc_t){.name = "tiled_sprite_camera", .layout = &(neko_render_uniform_layout_desc_t){.type = R_UNIFORM_MAT4}});

    renderer->pip = neko_render_pipeline_create((neko_render_pipeline_desc_t){.raster = {.shader = renderer->shader, .index_buffer_element_size = sizeof(uint32_t)},
                                                                              .layout = {.attrs =
                                                                                                 (neko_render_vertex_attribute_desc_t[]){
                                                                                                         {.format = R_VERTEX_ATTRIBUTE_FLOAT2, .name = "position"},
                                                                                                         {.format = R_VERTEX_ATTRIBUTE_FLOAT2, .name = "uv"},
                                                                                                         {.format = R_VERTEX_ATTRIBUTE_FLOAT4, .name = "color"},
                                                                                                         {.format = R_VERTEX_ATTRIBUTE_FLOAT, .name = "use_texture"},
                                                                                                 },
                                                                                         .size = 4 * sizeof(neko_render_vertex_attribute_desc_t)},
                                                                              .blend = {.func = R_BLEND_EQUATION_ADD, .src = R_BLEND_MODE_SRC_ALPHA, .dst = R_BLEND_MODE_ONE_MINUS_SRC_ALPHA}});
}

void neko_tiled_render_deinit(neko_tiled_renderer *renderer) {

    for (neko_hash_table_iter it = neko_hash_table_iter_new(renderer->quad_table); neko_hash_table_iter_valid(renderer->quad_table, it); neko_hash_table_iter_advance(renderer->quad_table, it)) {
        u32 k = neko_hash_table_iter_getk(renderer->quad_table, it);
        neko_tiled_quad_list_t quad_list = neko_hash_table_iter_get(renderer->quad_table, it);

        neko_dyn_array(neko_tiled_quad_t) v = quad_list.quad_list;

        neko_dyn_array_free(v);
    }

    neko_hash_table_free(renderer->quad_table);

    neko_render_shader_destroy(renderer->shader);
    // neko_command_buffer_free(cb);
}

void neko_tiled_render_begin(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // neko_render_clear_desc_t clear = {.actions = &(neko_render_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.0f}}};
    // neko_render_clear(cb, &clear);

    renderer->quad_count = 0;
}

void neko_tiled_render_flush(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // const neko_vec2 ws = neko_pf_window_sizev(neko_pf_main_window());
    // neko_render_set_viewport(cb, 0, 0, ws.x, ws.y);

    // renderer->camera_mat = neko_mat4_ortho(0.0f, ws.x, ws.y, 0.0f, -1.0f, 1.0f);

    // clang-format off
    neko_render_bind_desc_t binds = {
    .vertex_buffers = {&(neko_render_bind_vertex_buffer_desc_t){.buffer = renderer->vb}},
    .index_buffers = {.desc = &(neko_render_bind_index_buffer_desc_t){.buffer = renderer->ib}},
    .uniforms = {
        .desc = (neko_render_bind_uniform_desc_t[2]){
            {.uniform = renderer->u_camera, .data = &renderer->camera_mat},
            {.uniform = renderer->u_batch_tex, .data = &renderer->batch_texture}
        },
        .size = 2 * sizeof(neko_render_bind_uniform_desc_t)
    },
    .image_buffers = {
        .desc = &(neko_render_bind_image_buffer_desc_t){renderer->batch_texture, 0, R_ACCESS_READ_ONLY},  
        .size = sizeof(neko_render_bind_image_buffer_desc_t)
    }};
    // clang-format on

    neko_render_pipeline_bind(cb, renderer->pip);
    neko_render_apply_bindings(cb, &binds);

    neko_render_draw(cb, &(neko_render_draw_desc_t){.start = 0, .count = renderer->quad_count * IND_PER_QUAD});

    // neko_check_gl_error();

    renderer->quad_count = 0;
}

void neko_tiled_render_push(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, neko_tiled_quad_t quad) {

    // 如果这个quad的tileset还不存在于quad_table中则插入一个
    // tileset_id为quad_table的键值
    if (!neko_hash_table_exists(renderer->quad_table, quad.tileset_id))                              //
        neko_hash_table_insert(renderer->quad_table, quad.tileset_id, (neko_tiled_quad_list_t){0});  //

    //
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

            neko_render_vertex_buffer_request_update(
                    cb, renderer->vb,
                    &(neko_render_vertex_buffer_desc_t){.data = verts,
                                                        .size = VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
                                                        .usage = R_BUFFER_USAGE_DYNAMIC,
                                                        .update = {.type = R_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32)}});

            neko_render_index_buffer_request_update(cb, renderer->ib,
                                                    &(neko_render_index_buffer_desc_t){.data = indices,
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
