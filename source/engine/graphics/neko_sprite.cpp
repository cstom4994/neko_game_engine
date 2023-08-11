
#include "neko_sprite.h"

#include "engine/base/neko_engine.h"
#include "engine/common/neko_hash.h"
#include "engine/utility/logger.hpp"
#include "engine/utility/neko_cpp_utils.hpp"

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "libs/cute/cute_aseprite.h"

bool neko_sprite_load(neko_sprite *spr, neko_string filepath) {

    ase_t *ase = cute_aseprite_load_from_file(filepath.c_str(), NULL);
    neko_defer([&] { cute_aseprite_free(ase); });

    if (NULL == ase) {
        neko_error("unable to load ase ", filepath);
        return neko_default_val();
    }

    s32 rect = ase->w * ase->h * 4;

    neko_array<neko_sprite_frame> frames = {};
    neko_array_reserve(&frames, ase->frame_count);

    neko_array<char> pixels = {};
    neko_array_reserve(&pixels, ase->frame_count * rect);
    neko_defer([&] { neko_array_dctor(&pixels); });

    for (s32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t &frame = ase->frames[i];

        neko_sprite_frame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (f32)(i + 1) / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (f32)i / ase->frame_count;

        neko_array_push(&frames, sf);
        std::memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
    }

    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();

    t_desc.texture_format = neko_texture_format_rgba8;
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = ase->w;
    t_desc.height = ase->h * ase->frame_count;
    t_desc.num_comps = 4;

    // 大小为 ase->frame_count * rect
    // neko_tex_flip_vertically(ase->w, ase->h * ase->frame_count, (u8 *)pixels.data);
    t_desc.data = pixels.data;

    neko_texture_t tex = gfx->construct_texture(t_desc);

    // img.width = desc.width;
    // img.height = desc.height;

    neko_hashmap<neko_sprite_loop> by_tag;
    neko_hashmap_reserve(&by_tag, neko_hashmap_reserve_size((u64)ase->tag_count));

    for (s32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t &tag = ase->tags[i];

        neko_sprite_loop loop = {};

        for (s32 j = tag.from_frame; j <= tag.to_frame; j++) {
            neko_array_push(&loop.indices, j);
        }

        u64 key = neko::hash(tag.name /*, strlen(tag.name)*/);
        by_tag[key] = loop;
    }

    neko_debug(std::format("created sprite with image id: {0} and {1} frames", tex.id, frames.len));

    neko_sprite s = {};
    s.img = tex;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *spr = s;

    return true;
}

void neko_sprite_end(neko_sprite *spr) {
    neko_array_dctor(&spr->frames);

    for (auto [k, v] : spr->by_tag) {
        neko_array_dctor(&v->indices);
    }
    neko_hashmap_dctor(&spr->by_tag);
}

void neko_sprite_renderer_play(neko_sprite_renderer *sr, neko_string tag) {
    neko_sprite_loop *loop = neko_hashmap_get(&sr->sprite->by_tag, neko::hash(tag.c_str()));
    sr->loop = loop;
    sr->current_frame = 0;
    sr->elapsed = 0;
}

void neko_sprite_renderer_update(neko_sprite_renderer *sr, float dt) {
    s32 index;
    u64 len;
    if (sr->loop) {
        index = sr->loop->indices[sr->current_frame];
        len = sr->loop->indices.len;
    } else {
        index = sr->current_frame;
        len = sr->sprite->frames.len;
    }

    neko_sprite_frame frame = sr->sprite->frames[index];

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

void neko_sprite_renderer_set_frame(neko_sprite_renderer *sr, s32 frame) {
    s32 len;
    if (sr->loop) {
        len = sr->loop->indices.len;
    } else {
        len = sr->sprite->frames.len;
    }

    if (0 <= frame && frame < len) {
        sr->current_frame = frame;
        sr->elapsed = 0;
    }
}