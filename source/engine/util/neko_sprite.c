
#include "neko_sprite.h"

#include "engine/neko.h"
#include "engine/neko_engine.h"

// builtin
#include "engine/builtin/neko_aseprite.h"

// game
// #include "hpp/neko_cpp_utils.hpp"

// Shaders
#ifdef NEKO_PLATFORM_WEB
#define NEKO_VERSION_STR "#version 300 es\n"
#else
#define NEKO_VERSION_STR "#version 330 core\n"
#endif

bool neko_aseprite_load(neko_aseprite* spr, const_str filepath) {

    ase_t* ase = neko_aseprite_load_from_file(filepath);

    if (NULL == ase) {
        neko_log_error("unable to load ase %s", filepath);
        return false;
    }

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;

        frame->pixels[0] = (neko_color_t*)neko_safe_malloc((int)(sizeof(neko_color_t)) * ase->w * ase->h);

        spr->mem_used += (sizeof(neko_color_t)) * ase->w * ase->h;

        memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t* dst = frame->pixels[0];

        // neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t* cel = frame->cels + j;

            // neko_println_debug(" - %s", cel->layer->name);

            // 确定图块所在层与父层可视
            if (!(cel->layer->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t* frame = ase->frames + cel->linked_frame_index;
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

            void* src = cel->cel_pixels;
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

    u8* pixels = neko_safe_malloc(ase->frame_count * rect);

    for (s32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t* frame = &ase->frames[i];

        neko_aseprite_frame sf = neko_default_val();
        sf.duration = frame->duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (f32)(i + 1) / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (f32)i / ase->frame_count;

        neko_dyn_array_push(s.frames, sf);

        neko_color_t* data = frame->pixels[0];

        // 不知道是直接读取aseprite解析的数据还是像这样拷贝为一个贴图 在渲染时改UV来得快
        memcpy(pixels + (i * rect), &data[0].r, rect);
    }

    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

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

    neko_texture_t tex = neko_graphics_texture_create(&t_desc);

    neko_safe_free(pixels);

    // img.width = desc.width;
    // img.height = desc.height;

    // neko_hashmap<neko_sprite_loop> by_tag;
    // neko_hash_table(u64, neko_sprite_loop) by_tag;
    // neko_hashmap_reserve(&by_tag, neko_hashmap_reserve_size((u64)ase->tag_count));

    for (s32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t* tag = &ase->tags[i];

        neko_aseprite_loop loop = neko_default_val();

        for (s32 j = tag->from_frame; j <= tag->to_frame; j++) {
            neko_dyn_array_push(loop.indices, j);
        }

        u64 key = neko_hash_str64(tag->name /*, strlen(tag.name)*/);
        neko_hash_table_insert(s.by_tag, key, loop);
    }

    for (s32 i = 0; i < ase->layer_count; i++) {
        ase_layer_t* layer = &ase->layers[i];
        neko_println_debug("%s", layer->name);
    }

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

void neko_aseprite_end(neko_aseprite* spr) {
    neko_dyn_array_free(spr->frames);

    for (neko_hash_table_iter it = neko_hash_table_iter_new(spr->by_tag); neko_hash_table_iter_valid(spr->by_tag, it); neko_hash_table_iter_advance(spr->by_tag, it)) {
        u64 key = neko_hash_table_iter_getk(spr->by_tag, it);
        neko_aseprite_loop* v = neko_hash_table_getp(spr->by_tag, key);
        neko_dyn_array_free(v->indices);
    }

    neko_hash_table_free(spr->by_tag);
}

void neko_aseprite_renderer_play(neko_aseprite_renderer* sr, const_str tag) {
    neko_aseprite_loop* loop = neko_hash_table_getp(sr->sprite->by_tag, neko_hash_str64(tag));
    if (loop != NULL) sr->loop = loop;
    sr->current_frame = 0;
    sr->elapsed = 0;
}

void neko_aseprite_renderer_update(neko_aseprite_renderer* sr, f32 dt) {
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

void neko_aseprite_renderer_set_frame(neko_aseprite_renderer* sr, s32 frame) {
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

void neko_particle_ctor(neko_particle_t* par) {
    par->x = 0;
    par->y = 0;
    par->direction = 2 * neko_pi * neko_rand_xorshf32() / neko_rand_xorshf32_max;
    par->speed = 0.04 * neko_rand_xorshf32() / neko_rand_xorshf32_max;
    par->speed *= par->speed;
}

void neko_particle_update(neko_particle_t* par, int interval, float spin, float speed) {
    par->direction += interval * spin;

    double xspeed = par->speed * cos(par->direction);
    double yspeed = par->speed * sin(par->direction);

    par->x += xspeed * interval * speed;
    par->y += yspeed * interval * speed;

    if (par->x < -1 || par->x > 1 || par->y < -1 || par->y > 1) {
        neko_particle_ctor(par);
    }

    if (neko_rand_xorshf32() < neko_rand_xorshf32_max / 100) {
        neko_particle_ctor(par);
    }
}

void neko_particle_draw(neko_particle_t* par, neko_command_buffer_t* cb, neko_vec2 trans, neko_color_t color, f32 render_radius, int particle_radius) {
    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

    f32 x = (par->x + 1) * render_radius + trans.x;
    f32 y = (par->y + 1) * render_radius + trans.y;

    // 暂时用立即渲染方法测试
    // gfx->immediate.draw_circle(cb, neko_vec2{x, y}, particle_radius, 0, neko_color_white);
}

void neko_particle_renderer_construct(neko_particle_renderer* pr) {
    pr->particles_arr = neko_dyn_array_new(neko_particle_t);

    pr->last_time = 0;
    pr->particles_num = 100;
    pr->cycle_colors = true;

    pr->color_cycling_speed = 0.001f;
    pr->particle_color = (neko_color_t){0, 0, 0, 255};
    pr->particle_radius = 2;
    pr->particle_spin = 0.0003f;
    pr->particle_speed = 0.5f;
    pr->render_radius = 64.f;

    for (int i = 0; i < pr->particles_num; i++) {
        neko_particle_t par = {0};
        neko_particle_ctor(&par);
        neko_dyn_array_push(pr->particles_arr, par);
    }
}

void neko_particle_renderer_free(neko_particle_renderer* pr) { neko_dyn_array_free(pr->particles_arr); }

void neko_particle_renderer_update(neko_particle_renderer* pr, int elapsed) {
    // update_settings(settings);

    int interval = elapsed - pr->last_time;
    pr->last_time = elapsed;

    u32 sz = neko_dyn_array_size(pr->particles_arr);
    for (u32 i = 0; i < sz; ++i) {
        neko_particle_t* particle = &(pr->particles_arr)[i];
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

void neko_particle_renderer_draw(neko_particle_renderer* pr, neko_command_buffer_t* cb, neko_vec2 trans) {
    u32 sz = neko_dyn_array_size(pr->particles_arr);
    for (u32 i = 0; i < sz; ++i) {
        neko_particle_t* particle = &(pr->particles_arr)[i];
        neko_particle_draw(particle, cb, trans, pr->particle_color, pr->render_radius, pr->particle_radius);
    }
}

// void show() {
//     ImGui::Begin("Settings");
//
//     // Number of particles
//     ImGui::Text("Number of particles");
//     ImGui::SameLine();
//     neko_core_ui_help_marker(
//             "Control the number of simulated particles by\n"
//             "adjusting the slider.\n");
//     ImGui::SliderInt("##Number of particles", &nParticles, 1, 20000);
//     ImGui::Spacing();
//     ImGui::Separator();
//     ImGui::Spacing();
//
//     // Should colors cycle?
//     static int cycle = 1;
//     ImGui::Text("Cycle colors");
//     ImGui::SameLine();
//     neko_core_ui_help_marker(
//             "If set to true, particle colors are cycled automatically.\n"
//             "Otherwise, the particle color may be defined manually\n"
//             "through the color picker below.\n");
//
//     ImGui::RadioButton("True ", &cycle, 1);
//     ImGui::SameLine();
//     static int cyclingSpeed = 5;
//     ImGui::SliderInt("Cycle speed", &cyclingSpeed, 1, 20);
//     ImGui::SameLine();
//     neko_core_ui_help_marker("Adjust the color cycling speed.\n");
//     colorCyclingSpeed = cyclingSpeed * 0.0001f;
//
//     ImGui::RadioButton("False", &cycle, 0);
//     ImGui::SameLine();
//     cycleColors = cycle != 0;
//     static ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
//     ImGui::ColorEdit3("", (float *)&color);
//     if (!cycleColors) {
//         particleColor = color;
//     }
//     ImGui::Spacing();
//     ImGui::Separator();
//     ImGui::Spacing();
//
//     // Particle radius
//     ImGui::Text("Particle radius");
//     ImGui::SameLine();
//     neko_core_ui_help_marker("The smaller the radius, the smaller the particle is.\n");
//     ImGui::SliderInt("##Particle radius", &particleRadius, 1, 5);
//     ImGui::Spacing();
//     ImGui::Separator();
//     ImGui::Spacing();
//
//     // Particle spin
//     static int spin = 3;
//     ImGui::Text("Particle spin");
//     ImGui::SameLine();
//     neko_core_ui_help_marker(
//             "How much should exploding particles \"spin\" relative\n"
//             "to the explosion center?\n");
//     ImGui::SliderInt("##Particle spin", &spin, 1, 8);
//     particleSpin = spin * 0.0001f;
//     ImGui::Spacing();
//     ImGui::Separator();
//     ImGui::Spacing();
//
//     // Particle speed
//     static int speed = 5;
//     ImGui::Text("Particle speed");
//     ImGui::SameLine();
//     neko_core_ui_help_marker("How fast should exploding particles diffuse?\n");
//     ImGui::SliderInt("##Particle speed", &speed, 1, 10);
//     particleSpeed = speed * 0.1f;
//     ImGui::Spacing();
//     ImGui::Separator();
//     ImGui::Spacing();
//
//     // Window footer
//     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//
//     ImGui::End();
// }
