
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

void neko_particle_ctor(neko_particle_t *par) {
    par->m_x = 0;
    par->m_y = 0;
    par->m_direction = 2 * neko_pi * neko_rand_xorshf32() / neko_rand_xorshf32_max;
    par->m_speed = 0.04 * neko_rand_xorshf32() / neko_rand_xorshf32_max;
    par->m_speed *= par->m_speed;
}

void neko_particle_update(neko_particle_t *par, int interval, float spin, float speed) {
    par->m_direction += interval * spin;

    double xspeed = par->m_speed * cos(par->m_direction);
    double yspeed = par->m_speed * sin(par->m_direction);

    par->m_x += xspeed * interval * speed;
    par->m_y += yspeed * interval * speed;

    if (par->m_x < -1 || par->m_x > 1 || par->m_y < -1 || par->m_y > 1) {
        neko_particle_ctor(par);
    }

    if (neko_rand_xorshf32() < neko_rand_xorshf32_max / 100) {
        neko_particle_ctor(par);
    }
}

void neko_particle_draw(neko_particle_t *par, neko_command_buffer_t *cb, neko_color_t color, int particle_radius) {
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    neko_vec2 win_size = platform->window_size(platform->main_window());

    f32 x = (int)((par->m_x + 1) * (win_size.x) / 2);
    f32 y = (int)(par->m_y * (win_size.x) / 2 + (int)((win_size.y) / 2));

    // 暂时用立即渲染方法测试
    gfx->immediate.draw_circle(cb, neko_vec2{x, y}, particle_radius, 0, neko_color_white);
}

void neko_particle_renderer_construct(neko_particle_renderer *pr) {
    pr->particles_arr = neko_dyn_array_new(neko_particle_t);

    pr->last_time = 0;

    for (int i = 0; i < pr->particles_num; i++) {
        neko_particle_t par = {0};
        neko_particle_ctor(&par);
        neko_dyn_array_push(pr->particles_arr, par);
    }
}

void neko_particle_renderer_free(neko_particle_renderer *pr) { neko_dyn_array_free(pr->particles_arr); }

void neko_particle_renderer_update(neko_particle_renderer *pr, int elapsed) {
    // update_settings(settings);

    int interval = elapsed - pr->last_time;
    pr->last_time = elapsed;

    u32 sz = neko_dyn_array_size(pr->particles_arr);
    for (u32 i = 0; i < sz; ++i) {
        neko_particle_t *particle = &(pr->particles_arr)[i];
        neko_particle_update(particle, interval, pr->particle_spin, pr->particle_speed);
    }

    // if (settings.cycleColors) {
    //     color.x = (float)((1 + sin(elapsed * settings.colorCyclingSpeed * 2)) * 0.5);
    //     color.y = (float)((1 + sin(elapsed * settings.colorCyclingSpeed)) * 0.5);
    //     color.z = (float)((1 + sin(elapsed * settings.colorCyclingSpeed * 3)) * 0.5);
    // } else {
    //     color.x = settings.particleColor.x;
    //     color.y = settings.particleColor.y;
    //     color.z = settings.particleColor.z;
    // }

    // if (m_pParticles.size() != settings.nParticles) {
    //     int oldNParticles = (int)m_pParticles.size();
    //     m_pParticles.resize((size_t)settings.nParticles);
    //     if (oldNParticles < settings.nParticles) {
    //         int count = 0;
    //         for (int i = oldNParticles; i < settings.nParticles; i++) {
    //             m_pParticles[i] = new neko_particle();
    //             count++;
    //         }
    //     }
    // }
}

void neko_particle_renderer_draw(neko_particle_renderer *pr, neko_command_buffer_t *cb) {
    u32 sz = neko_dyn_array_size(pr->particles_arr);
    for (u32 i = 0; i < sz; ++i) {
        neko_particle_t *particle = &(pr->particles_arr)[i];
        neko_particle_draw(particle, cb, pr->particle_color, pr->particle_radius);
    }
}

//void show() {
//    ImGui::Begin("Settings");
//
//    // Number of particles
//    ImGui::Text("Number of particles");
//    ImGui::SameLine();
//    neko_imgui_help_marker(
//            "Control the number of simulated particles by\n"
//            "adjusting the slider.\n");
//    ImGui::SliderInt("##Number of particles", &nParticles, 1, 20000);
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//
//    // Should colors cycle?
//    static int cycle = 1;
//    ImGui::Text("Cycle colors");
//    ImGui::SameLine();
//    neko_imgui_help_marker(
//            "If set to true, particle colors are cycled automatically.\n"
//            "Otherwise, the particle color may be defined manually\n"
//            "through the color picker below.\n");
//
//    ImGui::RadioButton("True ", &cycle, 1);
//    ImGui::SameLine();
//    static int cyclingSpeed = 5;
//    ImGui::SliderInt("Cycle speed", &cyclingSpeed, 1, 20);
//    ImGui::SameLine();
//    neko_imgui_help_marker("Adjust the color cycling speed.\n");
//    colorCyclingSpeed = cyclingSpeed * 0.0001f;
//
//    ImGui::RadioButton("False", &cycle, 0);
//    ImGui::SameLine();
//    cycleColors = cycle != 0;
//    static ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
//    ImGui::ColorEdit3("", (float *)&color);
//    if (!cycleColors) {
//        particleColor = color;
//    }
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//
//    // Particle radius
//    ImGui::Text("Particle radius");
//    ImGui::SameLine();
//    neko_imgui_help_marker("The smaller the radius, the smaller the particle is.\n");
//    ImGui::SliderInt("##Particle radius", &particleRadius, 1, 5);
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//
//    // Particle spin
//    static int spin = 3;
//    ImGui::Text("Particle spin");
//    ImGui::SameLine();
//    neko_imgui_help_marker(
//            "How much should exploding particles \"spin\" relative\n"
//            "to the explosion center?\n");
//    ImGui::SliderInt("##Particle spin", &spin, 1, 8);
//    particleSpin = spin * 0.0001f;
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//
//    // Particle speed
//    static int speed = 5;
//    ImGui::Text("Particle speed");
//    ImGui::SameLine();
//    neko_imgui_help_marker("How fast should exploding particles diffuse?\n");
//    ImGui::SliderInt("##Particle speed", &speed, 1, 10);
//    particleSpeed = speed * 0.1f;
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//
//    // Window footer
//    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//
//    ImGui::End();
//}