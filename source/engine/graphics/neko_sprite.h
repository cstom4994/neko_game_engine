
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

struct neko_particle_t {
    f64 m_x;
    f64 m_y;
    f64 m_speed;
    f64 m_direction;
};

struct neko_particle_renderer {
    int particles_num = 2500;
    bool cycle_colors = true;
    float color_cycling_speed = 0.001f;
    neko_color_t particle_color = neko_color_t{0, 0, 0, 255};
    int particle_radius = 2;
    float particle_spin = 0.0003f;
    float particle_speed = 0.5f;
    int blur_radius = 1;

    int last_time;  // 上次更新屏幕的时间

    neko_dyn_array(neko_particle_t) particles_arr;
};

void neko_particle_ctor(neko_particle_t *par);
void neko_particle_update(neko_particle_t *par, int interval, float spin, float speed);
void neko_particle_draw(neko_particle_t *par, neko_command_buffer_t *cb, neko_color_t color, int particle_radius);

void neko_particle_renderer_construct(neko_particle_renderer *pr);
void neko_particle_renderer_free(neko_particle_renderer *pr);
void neko_particle_renderer_update(neko_particle_renderer *pr, int elapsed);
void neko_particle_renderer_draw(neko_particle_renderer *pr, neko_command_buffer_t *cb);

#endif