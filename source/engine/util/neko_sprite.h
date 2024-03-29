
#ifndef NEKO_SPRITE_H
#define NEKO_SPRITE_H

#include "engine/neko.h"
#include "engine/neko_engine.h"

// game
// #include "neko_hash.h"

typedef struct neko_aseprite_frame {
    s32 duration;
    f32 u0, v0, u1, v1;
} neko_aseprite_frame;

typedef struct neko_aseprite_loop {
    neko_dyn_array(s32) indices;
} neko_aseprite_loop;

typedef struct neko_aseprite {
    // neko_array<neko_sprite_frame> frames;
    neko_dyn_array(neko_aseprite_frame) frames;
    neko_hash_table(u64, neko_aseprite_loop) by_tag;
    neko_texture_t img;
    s32 width;
    s32 height;

    // #ifdef NEKO_DEBUG
    u64 mem_used;
    // #endif
} neko_aseprite;

typedef struct neko_aseprite_renderer {
    neko_aseprite* sprite;
    neko_aseprite_loop* loop;
    f32 elapsed;
    s32 current_frame;
} neko_aseprite_renderer;

NEKO_API_DECL bool neko_aseprite_load(neko_aseprite* spr, const_str filepath);
NEKO_API_DECL void neko_aseprite_end(neko_aseprite* spr);
NEKO_API_DECL void neko_aseprite_renderer_play(neko_aseprite_renderer* sr, const_str tag);
NEKO_API_DECL void neko_aseprite_renderer_update(neko_aseprite_renderer* sr, f32 dt);
NEKO_API_DECL void neko_aseprite_renderer_set_frame(neko_aseprite_renderer* sr, s32 frame);

typedef struct neko_particle_t {
    f64 x;
    f64 y;
    f64 speed;
    f64 direction;
} neko_particle_t;

typedef struct neko_particle_renderer {
    s32 particles_num;
    bool cycle_colors;
    f32 color_cycling_speed;
    neko_color_t particle_color;
    s32 particle_radius;
    f32 particle_spin;
    f32 particle_speed;
    f32 render_radius;

    s32 last_time;  // 上次更新屏幕的时间

    neko_dyn_array(neko_particle_t) particles_arr;
} neko_particle_renderer;

NEKO_API_DECL void neko_particle_ctor(neko_particle_t* par);
NEKO_API_DECL void neko_particle_update(neko_particle_t* par, int interval, float spin, float speed);
NEKO_API_DECL void neko_particle_draw(neko_particle_t* par, neko_command_buffer_t* cb, neko_vec2 trans, neko_color_t color, f32 render_radius, int particle_radius);

NEKO_API_DECL void neko_particle_renderer_construct(neko_particle_renderer* pr);
NEKO_API_DECL void neko_particle_renderer_free(neko_particle_renderer* pr);
NEKO_API_DECL void neko_particle_renderer_update(neko_particle_renderer* pr, int elapsed);
NEKO_API_DECL void neko_particle_renderer_draw(neko_particle_renderer* pr, neko_command_buffer_t* cb, neko_vec2 trans);

#endif
