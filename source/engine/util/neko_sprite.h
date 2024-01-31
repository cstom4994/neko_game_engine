
#ifndef NEKO_SPRITE_H
#define NEKO_SPRITE_H

#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_engine.h"

// game
// #include "neko_hash.h"

typedef struct neko_sprite_frame {
    s32 duration;
    f32 u0, v0, u1, v1;
} neko_sprite_frame;

typedef struct neko_sprite_loop {
    neko_dyn_array(s32) indices;
} neko_sprite_loop;

typedef struct neko_sprite {
    // neko_array<neko_sprite_frame> frames;
    neko_dyn_array(neko_sprite_frame) frames;
    neko_hash_table(u64, neko_sprite_loop) by_tag;
    neko_texture_t img;
    s32 width;
    s32 height;

    // #ifdef NEKO_DEBUG
    u64 mem_used;
    // #endif
} neko_sprite;

typedef struct neko_sprite_renderer {
    neko_sprite* sprite;
    neko_sprite_loop* loop;
    f32 elapsed;
    s32 current_frame;
} neko_sprite_renderer;

NEKO_API_DECL bool neko_sprite_load(neko_sprite* spr, const_str filepath);
NEKO_API_DECL void neko_sprite_end(neko_sprite* spr);
NEKO_API_DECL void neko_sprite_renderer_play(neko_sprite_renderer* sr, const_str tag);
NEKO_API_DECL void neko_sprite_renderer_update(neko_sprite_renderer* sr, f32 dt);
NEKO_API_DECL void neko_sprite_renderer_set_frame(neko_sprite_renderer* sr, s32 frame);

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

typedef struct neko_fast_sprite_renderer {
    neko_handle(neko_graphics_vertex_buffer_t) vbo;
    neko_handle(neko_graphics_index_buffer_t) ibo;
    neko_handle(neko_graphics_pipeline_t) pip;
    neko_handle(neko_graphics_shader_t) shader;
    neko_handle(neko_graphics_uniform_t) u_tex;
    neko_handle(neko_graphics_texture_t) tex;

} neko_fast_sprite_renderer;

NEKO_API_DECL void neko_fast_sprite_renderer_construct(neko_fast_sprite_renderer* render, u32 width, u32 height, void* data);
NEKO_API_DECL void neko_fast_sprite_renderer_draw(neko_fast_sprite_renderer* render, neko_command_buffer_t* cb);

#endif
