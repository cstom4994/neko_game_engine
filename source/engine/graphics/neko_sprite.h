
#ifndef NEKO_SPRITE_H
#define NEKO_SPRITE_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_util.h"
#include "engine/graphics/neko_graphics.h"
#include "libs/cute/cute_aseprite.h"

struct neko_sprite_frame {
    s32 duration;
    f32 u0, v0, u1, v1;
};

struct neko_sprite_loop {
    neko_array<s32> indices;
};

struct neko_sprite;

struct neko_sprite_renderer {
    neko_sprite *sprite;
    neko_sprite_loop *loop;
    f32 elapsed;
    s32 current_frame;
};

struct neko_sprite {
    neko_array<neko_sprite_frame> frames;
    neko_hashmap<neko_sprite_loop> by_tag;
    neko_texture_t img;
    s32 width;
    s32 height;
};

bool neko_sprite_load(neko_sprite *spr, neko_string filepath);
void neko_sprite_end(neko_sprite *spr);
void neko_sprite_renderer_play(neko_sprite_renderer *sr, neko_string tag);
void neko_sprite_renderer_update(neko_sprite_renderer *sr, float dt);
void neko_sprite_renderer_set_frame(neko_sprite_renderer *sr, s32 frame);

#endif