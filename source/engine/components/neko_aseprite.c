
#include "engine/neko_asset.h"
#include "engine/neko_common.h"

bool neko_aseprite_load(neko_aseprite *spr, const_str filepath) {

    ase_t *ase = neko_aseprite_load_from_file(filepath);

    if (NULL == ase) {
        neko_log_error("unable to load ase %s", filepath);
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
                neko_assert(found);
            }

            void *src = cel->cel_pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -neko_min(cx, 0);
            int ct = -neko_min(cy, 0);
            int dl = neko_max(cx, 0);
            int dt = neko_max(cy, 0);
            int dr = neko_min(ase->w, cw + cx);
            int db = neko_min(ase->h, ch + cy);
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

    neko_aseprite s = neko_default_val();

    // neko_array<neko_sprite_frame> frames = {};
    // neko_array_reserve(&frames, ase->frame_count);

    // neko_array<u8> pixels = {};
    // neko_array_reserve(&pixels, ase->frame_count * rect);
    //  neko_defer([&] { neko_array_dctor(&pixels); });

    u8 *pixels = neko_safe_malloc(ase->frame_count * rect);

    for (s32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t *frame = &ase->frames[i];

        neko_aseprite_frame sf = neko_default_val();
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

    neko_graphics_t *gfx = neko_instance()->ctx.graphics;

    neko_graphics_texture_desc_t t_desc = neko_default_val();

    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h * ase->frame_count;
    // t_desc.num_comps = 4;

    // 大小为 ase->frame_count * rect
    // neko_tex_flip_vertically(ase->w, ase->h * ase->frame_count, (u8 *)pixels.data);
    t_desc.data[0] = pixels;

    neko_texture_t tex = neko_graphics_texture_create(t_desc);

    neko_safe_free(pixels);

    // img.width = desc.width;
    // img.height = desc.height;

    // neko_hashmap<neko_sprite_loop> by_tag;
    // neko_hash_table(u64, neko_sprite_loop) by_tag;
    // neko_hashmap_reserve(&by_tag, neko_hashmap_reserve_size((u64)ase->tag_count));

    for (s32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t *tag = &ase->tags[i];

        neko_aseprite_loop loop = neko_default_val();

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

    // neko_log_trace(format("created sprite size({3}) with image id: {0} and {1} frames with {2} layers", tex.id, neko_dyn_array_size(s.frames), ase->layer_count,(spr->mem_used + pixels.capacity *
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

    ase_t *ase = neko_aseprite_load_from_memory(memory, size);

    if (NULL == ase) {
        neko_log_error("unable to load ase %p", memory);
        return (neko_texture_t){0};
    }

    neko_assert(ase->frame_count == 1);  // load_ase_texture_simple used to load simple aseprite

    neko_aseprite_default_blend_bind(ase);

    neko_graphics_t *gfx = neko_instance()->ctx.graphics;

    neko_log_trace("load aseprite - frame_count %d - palette.entry_count %d - w=%d h=%d", ase->frame_count, ase->palette.entry_count, ase->w, ase->h);

    s32 bpp = 4;

    neko_graphics_texture_desc_t t_desc = {};

    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    // t_desc.num_comps = 4;
    t_desc.data[0] = ase->frames->pixels[0];

    neko_tex_flip_vertically(ase->w, ase->h, (u8 *)(t_desc.data[0]));

    neko_texture_t tex = neko_graphics_texture_create(t_desc);

    neko_aseprite_free(ase);

    return tex;
}
