
#include "neko_sprite.h"

#include "engine/neko.h"
#include "engine/neko_containers.h"
#include "engine/neko_engine.h"

// game
#include "hpp/neko_cpp_utils.hpp"

// Shaders
#ifdef NEKO_PLATFORM_WEB
#define NEKO_VERSION_STR "#version 300 es\n"
#else
#define NEKO_VERSION_STR "#version 330 core\n"
#endif

#define ROW_COL_CT 10

bool neko_sprite_load(neko_sprite* spr, const neko_string& filepath) {

    ase_t* ase = neko_aseprite_load_from_file(filepath.c_str());
    neko_defer([&] { neko_aseprite_free(ase); });

    if (NULL == ase) {
        neko_log_error("unable to load ase %s", filepath);
        return neko_default_val();
    }

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;

        frame->pixels[0] = (neko_color_t*)neko_safe_malloc((int)(sizeof(neko_color_t)) * ase->w * ase->h);

        spr->mem_used += (sizeof(neko_color_t)) * ase->w * ase->h;

        std::memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t* dst = frame->pixels[0];

        neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t* cel = frame->cels + j;

            neko_println_debug(" - %s", cel->layer->name);

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

    neko_array<neko_sprite_frame> frames = {};
    neko_array_reserve(&frames, ase->frame_count);

    neko_array<u8> pixels = {};
    neko_array_reserve(&pixels, ase->frame_count * rect);
    neko_defer([&] { neko_array_dctor(&pixels); });

    for (s32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t& frame = ase->frames[i];

        neko_sprite_frame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (f32)(i + 1) / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (f32)i / ase->frame_count;

        neko_array_push(&frames, sf);

        neko_color_t* data = frame.pixels[0];

        // 不知道是直接读取aseprite解析的数据还是像这样拷贝为一个贴图 在渲染时改UV来得快
        std::memcpy(pixels.data + (i * rect), &data[0].r, rect);
    }

    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

    neko_graphics_texture_desc_t t_desc = {};

    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h * ase->frame_count;
    // t_desc.num_comps = 4;

    // 大小为 ase->frame_count * rect
    // neko_tex_flip_vertically(ase->w, ase->h * ase->frame_count, (u8 *)pixels.data);
    t_desc.data[0] = pixels.data;

    neko_texture_t tex = neko_graphics_texture_create(&t_desc);

    // img.width = desc.width;
    // img.height = desc.height;

    neko_hashmap<neko_sprite_loop> by_tag;
    neko_hashmap_reserve(&by_tag, neko_hashmap_reserve_size((u64)ase->tag_count));

    for (s32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t& tag = ase->tags[i];

        neko_sprite_loop loop = {};

        for (s32 j = tag.from_frame; j <= tag.to_frame; j++) {
            neko_array_push(&loop.indices, j);
        }

        u64 key = neko::hash(tag.name /*, strlen(tag.name)*/);
        by_tag[key] = loop;
    }

    for (s32 i = 0; i < ase->layer_count; i++) {
        ase_layer_t& layer = ase->layers[i];
        neko_println_debug("%s", layer.name);
    }

    neko_log_trace(std::format("created sprite size({3}) with image id: {0} and {1} frames with {2} layers", tex.id, frames.len, ase->layer_count, (spr->mem_used + pixels.capacity * sizeof(u8)) / 1e6)
                           .c_str());

    neko_sprite s = {};
    s.img = tex;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *spr = s;

    return true;
}

void neko_sprite_end(neko_sprite* spr) {
    neko_array_dctor(&spr->frames);

    for (auto [k, v] : spr->by_tag) {
        neko_array_dctor(&v->indices);
    }
    neko_hashmap_dctor(&spr->by_tag);
}

void neko_sprite_renderer_play(neko_sprite_renderer* sr, const neko_string& tag) {
    neko_sprite_loop* loop = neko_hashmap_get(&sr->sprite->by_tag, neko::hash(tag.c_str()));
    sr->loop = loop;
    sr->current_frame = 0;
    sr->elapsed = 0;
}

void neko_sprite_renderer_update(neko_sprite_renderer* sr, f32 dt) {
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

void neko_sprite_renderer_set_frame(neko_sprite_renderer* sr, s32 frame) {
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

void neko_particle_draw(neko_particle_t* par, neko_command_buffer_t* cb, CTransform* trans, neko_color_t color, f32 render_radius, int particle_radius) {
    neko_graphics_t* gfx = neko_instance()->ctx.graphics;

    f32 x = (par->x + 1) * render_radius + trans->x;
    f32 y = (par->y + 1) * render_radius + trans->y;

    // 暂时用立即渲染方法测试
    // gfx->immediate.draw_circle(cb, neko_vec2{x, y}, particle_radius, 0, neko_color_white);
}

void neko_particle_renderer_construct(neko_particle_renderer* pr) {
    pr->particles_arr = neko_dyn_array_new(neko_particle_t);

    pr->last_time = 0;
    pr->particles_num = 100;
    pr->cycle_colors = true;

    pr->color_cycling_speed = 0.001f;
    pr->particle_color = neko_color_t{0, 0, 0, 255};
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

void neko_particle_renderer_draw(neko_particle_renderer* pr, neko_command_buffer_t* cb, CTransform* trans) {
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
//     neko_imgui_help_marker(
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
//     neko_imgui_help_marker(
//             "If set to true, particle colors are cycled automatically.\n"
//             "Otherwise, the particle color may be defined manually\n"
//             "through the color picker below.\n");
//
//     ImGui::RadioButton("True ", &cycle, 1);
//     ImGui::SameLine();
//     static int cyclingSpeed = 5;
//     ImGui::SliderInt("Cycle speed", &cyclingSpeed, 1, 20);
//     ImGui::SameLine();
//     neko_imgui_help_marker("Adjust the color cycling speed.\n");
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
//     neko_imgui_help_marker("The smaller the radius, the smaller the particle is.\n");
//     ImGui::SliderInt("##Particle radius", &particleRadius, 1, 5);
//     ImGui::Spacing();
//     ImGui::Separator();
//     ImGui::Spacing();
//
//     // Particle spin
//     static int spin = 3;
//     ImGui::Text("Particle spin");
//     ImGui::SameLine();
//     neko_imgui_help_marker(
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
//     neko_imgui_help_marker("How fast should exploding particles diffuse?\n");
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

// Vertex data for quad
float v_data[] = {
        // Positions  UVs
        -0.5f, -0.5f, 0.0f, 0.0f,  // Top Left
        0.5f,  -0.5f, 1.0f, 0.0f,  // Top Right
        -0.5f, 0.5f,  0.0f, 1.0f,  // Bottom Left
        0.5f,  0.5f,  1.0f, 1.0f   // Bottom Right
};

// Index data for quad
u32 i_data[] = {
        0, 3, 2,  // First Triangle
        0, 1, 3   // Second Triangle
};

// Shaders
const char* v_src = NEKO_VERSION_STR
        R"(
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
precision mediump float;
out vec2 uv;
void main()
{
   gl_Position = vec4(a_pos, 0.0, 1.0);
   uv = a_uv;
}
)";

const char* f_src = NEKO_VERSION_STR
        R"(
precision mediump float;
uniform sampler2D u_tex;
in vec2 uv;
out vec4 frag_color;
void main()
{
   frag_color = texture(u_tex, uv);
}
)";

void neko_neko_fast_sprite_renderer_construct(neko_fast_sprite_renderer* render, u32 width, u32 height, void* data) {

    // Generate procedural texture data (checkered texture)

    if (width == 0 || height == 0 || NULL == data) {
        neko_color_t c0 = NEKO_COLOR_WHITE;
        neko_color_t c1 = neko_color(20, 50, 150, 255);
        neko_color_t pixels[ROW_COL_CT * ROW_COL_CT] = neko_default_val();
        for (u32 r = 0; r < ROW_COL_CT; ++r) {
            for (u32 c = 0; c < ROW_COL_CT; ++c) {
                const bool re = (r % 2) == 0;
                const bool ce = (c % 2) == 0;
                u32 idx = r * ROW_COL_CT + c;
                pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
            }
        }
        data = pixels;
        width = height = ROW_COL_CT;
    }

    // Create dynamic texture
    render->tex = neko_graphics_texture_create(neko_c_ref(neko_graphics_texture_desc_t, {.type = NEKO_GRAPHICS_TEXTURE_2D,
                                                                                         .width = width,
                                                                                         .height = height,
                                                                                         .data = data,
                                                                                         .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                                                         .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                                                         .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST}));

    // Construct sampler buffer
    neko_graphics_uniform_layout_desc_t uniform_decl = {.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D};

    render->u_tex = neko_graphics_uniform_create(neko_c_ref(neko_graphics_uniform_desc_t, {.stage = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, .name = "u_tex", .layout = &uniform_decl}));

    // Construct vertex buffer
    render->vbo = neko_graphics_vertex_buffer_create(neko_c_ref(neko_graphics_vertex_buffer_desc_t, {.data = v_data, .size = sizeof(v_data)}));

    // Construct index buffer
    render->ibo = neko_graphics_index_buffer_create(neko_c_ref(neko_graphics_index_buffer_desc_t, {.data = i_data, .size = sizeof(i_data)}));

    // Create shader
    neko_graphics_shader_source_desc_t shaders_decl[] = {{.type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src}, {.type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}};

    render->shader = neko_graphics_shader_create(neko_c_ref(neko_graphics_shader_desc_t, {.sources = shaders_decl, .size = 2 * sizeof(neko_graphics_shader_source_desc_t), .name = "quad"}));

    render->pip = neko_graphics_pipeline_create(
            neko_c_ref(neko_graphics_pipeline_desc_t, {.raster = {.shader = render->shader, .index_buffer_element_size = sizeof(u32)},
                                                       .layout = {.attrs = neko_arr_ref(neko_graphics_vertex_attribute_desc_t, {{
                                                                                                                                        .name = "a_pos",
                                                                                                                                        .format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,
                                                                                                                                },
                                                                                                                                {
                                                                                                                                        .name = "a_uv",
                                                                                                                                        .format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,
                                                                                                                                }}),
                                                                  .size = 2 * sizeof(neko_graphics_vertex_attribute_desc_t)}}));
}

void neko_neko_fast_sprite_renderer_draw(neko_fast_sprite_renderer* render, neko_command_buffer_t* cb) {

    // Bindings for all buffers: vertex, index, sampler
    neko_graphics_bind_desc_t binds = {.vertex_buffers = {.desc = neko_c_ref(neko_graphics_bind_vertex_buffer_desc_t, {.buffer = render->vbo})},
                                       .index_buffers = {.desc = neko_c_ref(neko_graphics_bind_index_buffer_desc_t, {.buffer = render->ibo})},
                                       .uniforms = {.desc = neko_c_ref(neko_graphics_bind_uniform_desc_t, {.uniform = render->u_tex, .data = &render->tex, .binding = 0})}};

    /* Render */
    neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        // neko_graphics_set_viewport(cb, 0, 0, (s32)fbs.x, (s32)fbs.y);
        // neko_graphics_clear(cb, &clear);
        neko_graphics_pipeline_bind(cb, render->pip);
        neko_graphics_apply_bindings(cb, &binds);
        neko_graphics_draw(cb, neko_c_ref(neko_graphics_draw_desc_t, {.start = 0, .count = 6}));
    }
    neko_graphics_renderpass_end(cb);
}

#pragma region neko_ase

neko_global const_str s_error_file = NULL;  // 正在解析的文件的文件路径 如果来自内存则为 NULL
neko_global const_str s_error_reason;       // 用于捕获 DEFLATE 解析期间的错误

#define __NEKO_ASEPRITE_FAIL() \
    do {                       \
        goto ase_err;          \
    } while (0)

#define __NEKO_ASEPRITE_CHECK(X, Y) \
    do {                            \
        if (!(X)) {                 \
            s_error_reason = Y;     \
            __NEKO_ASEPRITE_FAIL(); \
        }                           \
    } while (0)

#define __NEKO_ASEPRITE_CALL(X) \
    do {                        \
        if (!(X)) goto ase_err; \
    } while (0)

#define __NEKO_ASEPRITE_DEFLATE_MAX_BITLEN 15

// DEFLATE tables from RFC 1951
neko_global u8 s_fixed_table[288 + 32] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};                                                                                                                                                                                           // 3.2.6
neko_global u8 s_permutation_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};                                                                                 // 3.2.7
neko_global u8 s_len_extra_bits[29 + 2] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};                                                     // 3.2.5
neko_global u32 s_len_base[29 + 2] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};                              // 3.2.5
neko_global u8 s_dist_extra_bits[30 + 2] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0};                                         // 3.2.5
neko_global u32 s_dist_base[30 + 2] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};  // 3.2.5

typedef struct deflate_t {
    u64 bits;
    int count;
    u32* words;
    int word_count;
    int word_index;
    int bits_left;

    int final_word_available;
    u32 final_word;

    char* out;
    char* out_end;
    char* begin;

    u32 lit[288];
    u32 dst[32];
    u32 len[19];
    u32 nlit;
    u32 ndst;
    u32 nlen;
} deflate_t;

neko_global int s_would_overflow(deflate_t* s, int num_bits) { return (s->bits_left + s->count) - num_bits < 0; }

neko_global char* s_ptr(deflate_t* s) {
    neko_assert(!(s->bits_left & 7));
    return (char*)(s->words + s->word_index) - (s->count / 8);
}

neko_global u64 s_peak_bits(deflate_t* s, int num_bits_to_read) {
    if (s->count < num_bits_to_read) {
        if (s->word_index < s->word_count) {
            u32 word = s->words[s->word_index++];
            s->bits |= (u64)word << s->count;
            s->count += 32;
            neko_assert(s->word_index <= s->word_count);
        }

        else if (s->final_word_available) {
            u32 word = s->final_word;
            s->bits |= (u64)word << s->count;
            s->count += s->bits_left;
            s->final_word_available = 0;
        }
    }

    return s->bits;
}

neko_global u32 s_consume_bits(deflate_t* s, int num_bits_to_read) {
    neko_assert(s->count >= num_bits_to_read);
    u32 bits = (u32)(s->bits & (((u64)1 << num_bits_to_read) - 1));
    s->bits >>= num_bits_to_read;
    s->count -= num_bits_to_read;
    s->bits_left -= num_bits_to_read;
    return bits;
}

neko_global u32 s_read_bits(deflate_t* s, int num_bits_to_read) {
    neko_assert(num_bits_to_read <= 32);
    neko_assert(num_bits_to_read >= 0);
    neko_assert(s->bits_left > 0);
    neko_assert(s->count <= 64);
    neko_assert(!s_would_overflow(s, num_bits_to_read));
    s_peak_bits(s, num_bits_to_read);
    u32 bits = s_consume_bits(s, num_bits_to_read);
    return bits;
}

neko_global u32 s_rev16(u32 a) {
    a = ((a & 0xAAAA) >> 1) | ((a & 0x5555) << 1);
    a = ((a & 0xCCCC) >> 2) | ((a & 0x3333) << 2);
    a = ((a & 0xF0F0) >> 4) | ((a & 0x0F0F) << 4);
    a = ((a & 0xFF00) >> 8) | ((a & 0x00FF) << 8);
    return a;
}

// RFC 1951 section 3.2.2
neko_global u32 s_build(deflate_t* s, u32* tree, u8* lens, int sym_count) {
    int n, codes[16], first[16], counts[16] = {0};
    neko_unused(s);

    // Frequency count
    for (n = 0; n < sym_count; n++) counts[lens[n]]++;

    // Distribute codes
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; ++n) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }

    for (int i = 0; i < sym_count; ++i) {
        u8 len = lens[i];

        if (len != 0) {
            neko_assert(len < 16);
            u32 code = (u32)codes[len]++;
            u32 slot = (u32)first[len]++;
            tree[slot] = (code << (32 - (u32)len)) | (i << 4) | len;
        }
    }

    return (u32)first[15];
}

neko_global int s_stored(deflate_t* s) {
    char* p;

    // 3.2.3
    // skip any remaining bits in current partially processed byte
    s_read_bits(s, s->count & 7);

    // 3.2.4
    // read LEN and NLEN, should complement each other
    u16 LEN = (u16)s_read_bits(s, 16);
    u16 NLEN = (u16)s_read_bits(s, 16);
    __NEKO_ASEPRITE_CHECK(LEN == (u16)(~NLEN), "Failed to find LEN and NLEN as complements within stored (uncompressed) stream.");
    __NEKO_ASEPRITE_CHECK(s->bits_left / 8 <= (int)LEN, "Stored block extends beyond end of input stream.");
    p = s_ptr(s);
    std::memcpy(s->out, p, LEN);
    s->out += LEN;
    return 1;

ase_err:
    return 0;
}

// 3.2.6
neko_global int s_fixed(deflate_t* s) {
    s->nlit = s_build(s, s->lit, s_fixed_table, 288);
    s->ndst = s_build(0, s->dst, s_fixed_table + 288, 32);
    return 1;
}

neko_global int s_decode(deflate_t* s, u32* tree, int hi) {
    u64 bits = s_peak_bits(s, 16);
    u32 search = (s_rev16((u32)bits) << 16) | 0xFFFF;
    int lo = 0;
    while (lo < hi) {
        int guess = (lo + hi) >> 1;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    u32 key = tree[lo - 1];
    u32 len = (32 - (key & 0xF));
    neko_assert((search >> len) == (key >> len));

    s_consume_bits(s, key & 0xF);
    return (key >> 4) & 0xFFF;
}

// 3.2.7
neko_global int s_dynamic(deflate_t* s) {
    u8 lenlens[19] = {0};

    u32 nlit = 257 + s_read_bits(s, 5);
    u32 ndst = 1 + s_read_bits(s, 5);
    u32 nlen = 4 + s_read_bits(s, 4);

    for (u32 i = 0; i < nlen; ++i) lenlens[s_permutation_order[i]] = (u8)s_read_bits(s, 3);

    // Build the tree for decoding code lengths
    s->nlen = s_build(0, s->len, lenlens, 19);
    u8 lens[288 + 32];

    for (u32 n = 0; n < nlit + ndst;) {
        int sym = s_decode(s, s->len, (int)s->nlen);
        switch (sym) {
            case 16:
                for (u32 i = 3 + s_read_bits(s, 2); i; --i, ++n) lens[n] = lens[n - 1];
                break;
            case 17:
                for (u32 i = 3 + s_read_bits(s, 3); i; --i, ++n) lens[n] = 0;
                break;
            case 18:
                for (u32 i = 11 + s_read_bits(s, 7); i; --i, ++n) lens[n] = 0;
                break;
            default:
                lens[n++] = (u8)sym;
                break;
        }
    }

    s->nlit = s_build(s, s->lit, lens, (int)nlit);
    s->ndst = s_build(0, s->dst, lens + nlit, (int)ndst);
    return 1;
}

// 3.2.3
neko_global int s_block(deflate_t* s) {
    while (1) {
        int symbol = s_decode(s, s->lit, (int)s->nlit);

        if (symbol < 256) {
            __NEKO_ASEPRITE_CHECK(s->out + 1 <= s->out_end, "Attempted to overwrite out buffer while outputting a symbol.");
            *s->out = (char)symbol;
            s->out += 1;
        }

        else if (symbol > 256) {
            symbol -= 257;
            u32 length = s_read_bits(s, (int)(s_len_extra_bits[symbol])) + s_len_base[symbol];
            int distance_symbol = s_decode(s, s->dst, (int)s->ndst);
            u32 backwards_distance = s_read_bits(s, s_dist_extra_bits[distance_symbol]) + s_dist_base[distance_symbol];
            __NEKO_ASEPRITE_CHECK(s->out - backwards_distance >= s->begin, "Attempted to write before out buffer (invalid backwards distance).");
            __NEKO_ASEPRITE_CHECK(s->out + length <= s->out_end, "Attempted to overwrite out buffer while outputting a string.");
            char* src = s->out - backwards_distance;
            char* dst = s->out;
            s->out += length;

            switch (backwards_distance) {
                case 1:  // very common in images
                    std::memset(dst, *src, (size_t)length);
                    break;
                default:
                    while (length--) *dst++ = *src++;
            }
        }

        else
            break;
    }

    return 1;

ase_err:
    return 0;
}

// 3.2.3
neko_global int s_inflate(const void* in, int in_bytes, void* out, int out_bytes) {
    deflate_t* s = (deflate_t*)neko_safe_malloc(sizeof(deflate_t));
    s->bits = 0;
    s->count = 0;
    s->word_index = 0;
    s->bits_left = in_bytes * 8;

    // s->words is the in-pointer rounded up to a multiple of 4
    int first_bytes = (int)((((size_t)in + 3) & ~3) - (size_t)in);
    s->words = (u32*)((char*)in + first_bytes);
    s->word_count = (in_bytes - first_bytes) / 4;
    int last_bytes = ((in_bytes - first_bytes) & 3);

    for (int i = 0; i < first_bytes; ++i) s->bits |= (u64)(((u8*)in)[i]) << (i * 8);

    s->final_word_available = last_bytes ? 1 : 0;
    s->final_word = 0;
    for (int i = 0; i < last_bytes; i++) s->final_word |= ((u8*)in)[in_bytes - last_bytes + i] << (i * 8);

    s->count = first_bytes * 8;

    s->out = (char*)out;
    s->out_end = s->out + out_bytes;
    s->begin = (char*)out;

    int count = 0;
    u32 bfinal;
    do {
        bfinal = s_read_bits(s, 1);
        u32 btype = s_read_bits(s, 2);

        switch (btype) {
            case 0:
                __NEKO_ASEPRITE_CALL(s_stored(s));
                break;
            case 1:
                s_fixed(s);
                __NEKO_ASEPRITE_CALL(s_block(s));
                break;
            case 2:
                s_dynamic(s);
                __NEKO_ASEPRITE_CALL(s_block(s));
                break;
            case 3:
                __NEKO_ASEPRITE_CHECK(0, "Detected unknown block type within input stream.");
        }

        ++count;
    } while (!bfinal);

    neko_safe_free(s);
    return 1;

ase_err:
    neko_safe_free(s);
    return 0;
}

typedef struct ase_state_t {
    u8* in;
    u8* end;
} ase_state_t;

neko_global u8 s_read_uint8(ase_state_t* s) {
    neko_assert(s->in <= s->end + sizeof(u8));
    u8** p = &s->in;
    u8 value = **p;
    ++(*p);
    return value;
}

neko_global u16 s_read_uint16(ase_state_t* s) {
    neko_assert(s->in <= s->end + sizeof(u16));
    u8** p = &s->in;
    u16 value;
    value = (*p)[0];
    value |= (((u16)((*p)[1])) << 8);
    *p += 2;
    return value;
}

neko_global ase_fixed_t s_read_fixed(ase_state_t* s) {
    ase_fixed_t value;
    value.a = s_read_uint16(s);
    value.b = s_read_uint16(s);
    return value;
}

neko_global u32 s_read_uint32(ase_state_t* s) {
    neko_assert(s->in <= s->end + sizeof(u32));
    u8** p = &s->in;
    u32 value;
    value = (*p)[0];
    value |= (((u32)((*p)[1])) << 8);
    value |= (((u32)((*p)[2])) << 16);
    value |= (((u32)((*p)[3])) << 24);
    *p += 4;
    return value;
}

neko_global s16 s_read_int16(ase_state_t* s) { return (s16)s_read_uint16(s); }
neko_global s16 s_read_int32(ase_state_t* s) { return (s32)s_read_uint32(s); }

neko_global const char* s_read_string(ase_state_t* s) {
    int len = (int)s_read_uint16(s);
    char* bytes = (char*)neko_safe_malloc(len + 1);
    for (int i = 0; i < len; ++i) {
        bytes[i] = (char)s_read_uint8(s);
    }
    bytes[len] = 0;
    return bytes;
}

neko_global void s_skip(ase_state_t* ase, int num_bytes) {
    neko_assert(ase->in <= ase->end + num_bytes);
    ase->in += num_bytes;
}

neko_global char* s_fopen(const char* path, int* size) {
    char* data = 0;
    FILE* fp = fopen(path, "rb");
    int sz = 0;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (char*)neko_safe_malloc(sz + 1);
        fread(data, sz, 1, fp);
        data[sz] = 0;
        fclose(fp);
    }

    if (size) *size = sz;
    return data;
}

ase_t* neko_aseprite_load_from_file(const char* path) {
    s_error_file = path;
    int sz;
    void* file = s_fopen(path, &sz);
    if (!file) {
        neko_log_warning(std::format("Unable to find map file {0}", s_error_file ? s_error_file : "MEMORY").c_str());
        return NULL;
    }
    ase_t* aseprite = neko_aseprite_load_from_memory(file, sz);
    neko_safe_free(file);
    s_error_file = NULL;
    return aseprite;
}

ase_t* neko_aseprite_load_from_memory(const void* memory, int size) {
    ase_t* ase = (ase_t*)neko_safe_malloc(sizeof(ase_t));
    std::memset(ase, 0, sizeof(*ase));

    ase_state_t state = {0};
    ase_state_t* s = &state;
    s->in = (u8*)memory;
    s->end = s->in + size;

    s_skip(s, sizeof(u32));  // File size.
    int magic = (int)s_read_uint16(s);
    neko_assert(magic == 0xA5E0);

    ase->frame_count = (int)s_read_uint16(s);
    ase->w = s_read_uint16(s);
    ase->h = s_read_uint16(s);
    u16 bpp = s_read_uint16(s) / 8;
    if (bpp == 4)
        ase->mode = NEKO_ASE_MODE_RGBA;
    else if (bpp == 2)
        ase->mode = NEKO_ASE_MODE_GRAYSCALE;
    else {
        neko_assert(bpp == 1);
        ase->mode = NEKO_ASE_MODE_INDEXED;
    }
    u32 valid_layer_opacity = s_read_uint32(s) & 1;
    int speed = s_read_uint16(s);
    s_skip(s, sizeof(u32) * 2);  // Spec says skip these bytes, as they're zero'd.
    ase->transparent_palette_entry_index = s_read_uint8(s);
    s_skip(s, 3);  // Spec says skip these bytes.
    ase->number_of_colors = (int)s_read_uint16(s);
    ase->pixel_w = (int)s_read_uint8(s);
    ase->pixel_h = (int)s_read_uint8(s);
    ase->grid_x = (int)s_read_int16(s);
    ase->grid_y = (int)s_read_int16(s);
    ase->grid_w = (int)s_read_uint16(s);
    ase->grid_h = (int)s_read_uint16(s);
    s_skip(s, 84);  // For future use (set to zero).

    ase->frames = (ase_frame_t*)neko_safe_malloc((int)(sizeof(ase_frame_t)) * ase->frame_count);
    std::memset(ase->frames, 0, sizeof(ase_frame_t) * (size_t)ase->frame_count);

    ase_udata_t* last_udata = NULL;
    int was_on_tags = 0;
    int tag_index = 0;

    ase_layer_t* layer_stack[__NEKO_ASEPRITE_MAX_LAYERS];

    // 查看 aseprite 文件标准
    // https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md

    // Parse all chunks in the .aseprite file.
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        frame->ase = ase;
        s_skip(s, sizeof(u32));  // Frame size.
        magic = (int)s_read_uint16(s);
        neko_assert(magic == 0xF1FA);
        int chunk_count = (int)s_read_uint16(s);
        frame->duration_milliseconds = s_read_uint16(s);
        if (frame->duration_milliseconds == 0) frame->duration_milliseconds = speed;
        s_skip(s, 2);  // For future use (set to zero).
        u32 new_chunk_count = s_read_uint32(s);
        if (new_chunk_count) chunk_count = (int)new_chunk_count;

        for (int j = 0; j < chunk_count; ++j) {
            u32 chunk_size = s_read_uint32(s);
            u16 chunk_type = s_read_uint16(s);
            chunk_size -= (u32)(sizeof(u32) + sizeof(u16));
            u8* chunk_start = s->in;

            switch (chunk_type) {
                case 0x2004:  // Layer chunk.
                {
                    neko_assert(ase->layer_count < __NEKO_ASEPRITE_MAX_LAYERS);
                    ase_layer_t* layer = ase->layers + ase->layer_count++;
                    layer->flags = (ase_layer_flags_t)s_read_uint16(s);

                    // WORD Layer type
                    //  0 = Normal(image) layer
                    //  1 = Group
                    //  2 = Tilemap
                    layer->type = (ase_layer_type_t)s_read_uint16(s);

                    layer->parent = NULL;
                    int child_level = (int)s_read_uint16(s);
                    layer_stack[child_level] = layer;
                    if (child_level) {
                        layer->parent = layer_stack[child_level - 1];
                    }
                    s_skip(s, sizeof(u16));  // Default layer width in pixels (ignored).
                    s_skip(s, sizeof(u16));  // Default layer height in pixels (ignored).
                    int blend_mode = (int)s_read_uint16(s);
                    if (blend_mode) neko_log_warning("aseprite unknown blend mode encountered.");
                    layer->opacity = s_read_uint8(s) / 255.0f;
                    if (!valid_layer_opacity) layer->opacity = 1.0f;
                    s_skip(s, 3);  // For future use (set to zero).
                    layer->name = s_read_string(s);
                    last_udata = &layer->udata;
                } break;

                case 0x2005:  // Cel chunk.
                {
                    neko_assert(frame->cel_count < __NEKO_ASEPRITE_MAX_LAYERS);
                    ase_cel_t* cel = frame->cels + frame->cel_count++;
                    int layer_index = (int)s_read_uint16(s);
                    cel->layer = ase->layers + layer_index;
                    cel->x = s_read_int16(s);
                    cel->y = s_read_int16(s);
                    cel->opacity = s_read_uint8(s) / 255.0f;
                    int cel_type = (int)s_read_uint16(s);
                    s_skip(s, 7);  // For future (set to zero).
                    switch (cel_type) {
                        case 0:  // Raw cel.
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            cel->cel_pixels = neko_safe_malloc(cel->w * cel->h * bpp);
                            std::memcpy(cel->cel_pixels, s->in, (size_t)(cel->w * cel->h * bpp));
                            s_skip(s, cel->w * cel->h * bpp);
                            break;

                        case 1:  // Linked cel.
                            cel->is_linked = 1;
                            cel->linked_frame_index = s_read_uint16(s);
                            break;

                        case 2:  // Compressed image cel.
                        {
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            int zlib_byte0 = s_read_uint8(s);
                            int zlib_byte1 = s_read_uint8(s);
                            int deflate_bytes = (int)chunk_size - (int)(s->in - chunk_start);
                            void* pixels = s->in;
                            neko_assert((zlib_byte0 & 0x0F) == 0x08);  // Only zlib compression method (RFC 1950) is supported.
                            neko_assert((zlib_byte0 & 0xF0) <= 0x70);  // Innapropriate window size detected.
                            neko_assert(!(zlib_byte1 & 0x20));         // Preset dictionary is present and not supported.
                            int pixels_sz = cel->w * cel->h * bpp;
                            void* pixels_decompressed = neko_safe_malloc(pixels_sz);
                            int ret = s_inflate(pixels, deflate_bytes, pixels_decompressed, pixels_sz);
                            if (!ret) neko_log_warning(s_error_reason);
                            cel->cel_pixels = pixels_decompressed;
                            s_skip(s, deflate_bytes);
                        } break;
                    }
                    last_udata = &cel->udata;
                } break;

                case 0x2006:  // Cel extra chunk.
                {
                    ase_cel_t* cel = frame->cels + frame->cel_count;
                    cel->has_extra = 1;
                    cel->extra.precise_bounds_are_set = (int)s_read_uint32(s);
                    cel->extra.precise_x = s_read_fixed(s);
                    cel->extra.precise_y = s_read_fixed(s);
                    cel->extra.w = s_read_fixed(s);
                    cel->extra.h = s_read_fixed(s);
                    s_skip(s, 16);  // For future use (set to zero).
                } break;

                case 0x2007:  // Color profile chunk.
                {
                    ase->has_color_profile = 1;
                    ase->color_profile.type = (ase_color_profile_type_t)s_read_uint16(s);
                    ase->color_profile.use_fixed_gamma = (int)s_read_uint16(s) & 1;
                    ase->color_profile.gamma = s_read_fixed(s);
                    s_skip(s, 8);  // For future use (set to zero).
                    if (ase->color_profile.type == NEKO_ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC) {
                        // Use the embedded ICC profile.
                        ase->color_profile.icc_profile_data_length = s_read_uint32(s);
                        ase->color_profile.icc_profile_data = neko_safe_malloc(ase->color_profile.icc_profile_data_length);
                        std::memcpy(ase->color_profile.icc_profile_data, s->in, ase->color_profile.icc_profile_data_length);
                        s->in += ase->color_profile.icc_profile_data_length;
                    }
                } break;

                case 0x2018:  // Tags chunk.
                {
                    ase->tag_count = (int)s_read_uint16(s);
                    s_skip(s, 8);  // For future (set to zero).
                    neko_assert(ase->tag_count < __NEKO_ASEPRITE_MAX_TAGS);
                    for (int k = 0; k < ase->tag_count; ++k) {
                        ase_tag_t tag;
                        tag.from_frame = (int)s_read_uint16(s);
                        tag.to_frame = (int)s_read_uint16(s);
                        tag.loop_animation_direction = (ase_animation_direction_t)s_read_uint8(s);
                        s_skip(s, 8);  // For future (set to zero).
                        tag.r = s_read_uint8(s);
                        tag.g = s_read_uint8(s);
                        tag.b = s_read_uint8(s);
                        s_skip(s, 1);  // Extra byte (zero).
                        tag.name = s_read_string(s);
                        ase->tags[k] = tag;
                    }
                    was_on_tags = 1;
                } break;

                case 0x2019:  // Palette chunk.
                {
                    ase->palette.entry_count = (int)s_read_uint32(s);
                    neko_assert(ase->palette.entry_count <= __NEKO_ASEPRITE_MAX_PALETTE_ENTRIES);
                    int first_index = (int)s_read_uint32(s);
                    int last_index = (int)s_read_uint32(s);
                    s_skip(s, 8);  // For future (set to zero).
                    for (int k = first_index; k <= last_index; ++k) {
                        int has_name = s_read_uint16(s);
                        ase_palette_entry_t entry;
                        entry.color.r = s_read_uint8(s);
                        entry.color.g = s_read_uint8(s);
                        entry.color.b = s_read_uint8(s);
                        entry.color.a = s_read_uint8(s);
                        if (has_name) {
                            entry.color_name = s_read_string(s);
                        } else {
                            entry.color_name = NULL;
                        }
                        ase->palette.entries[k] = entry;
                    }
                } break;

                case 0x2020:  // Udata chunk.
                {
                    neko_assert(last_udata || was_on_tags);
                    if (was_on_tags && !last_udata) {
                        neko_assert(tag_index < ase->tag_count);
                        last_udata = &ase->tags[tag_index++].udata;
                    }
                    int flags = (int)s_read_uint32(s);
                    if (flags & 1) {
                        last_udata->has_text = 1;
                        last_udata->text = s_read_string(s);
                    }
                    if (flags & 2) {
                        last_udata->color.r = s_read_uint8(s);
                        last_udata->color.g = s_read_uint8(s);
                        last_udata->color.b = s_read_uint8(s);
                        last_udata->color.a = s_read_uint8(s);
                    }
                    last_udata = NULL;
                } break;

                case 0x2022:  // Slice chunk.
                {
                    int slice_count = (int)s_read_uint32(s);
                    int flags = (int)s_read_uint32(s);
                    s_skip(s, sizeof(u32));  // Reserved.
                    const char* name = s_read_string(s);
                    for (int k = 0; k < (int)slice_count; ++k) {
                        ase_slice_t slice = {0};
                        slice.name = name;
                        slice.frame_number = (int)s_read_uint32(s);
                        slice.origin_x = (int)s_read_int32(s);
                        slice.origin_y = (int)s_read_int32(s);
                        slice.w = (int)s_read_uint32(s);
                        slice.h = (int)s_read_uint32(s);
                        if (flags & 1) {
                            // It's a 9-patches slice.
                            slice.has_center_as_9_slice = 1;
                            slice.center_x = (int)s_read_int32(s);
                            slice.center_y = (int)s_read_int32(s);
                            slice.center_w = (int)s_read_uint32(s);
                            slice.center_h = (int)s_read_uint32(s);
                        } else if (flags & 2) {
                            // Has pivot information.
                            slice.has_pivot = 1;
                            slice.pivot_x = (int)s_read_int32(s);
                            slice.pivot_y = (int)s_read_int32(s);
                        }
                        neko_assert(ase->slice_count < __NEKO_ASEPRITE_MAX_SLICES);
                        ase->slices[ase->slice_count++] = slice;
                        last_udata = &ase->slices[ase->slice_count - 1].udata;
                    }
                } break;

                default:
                    s_skip(s, (int)chunk_size);
                    break;
            }

            u32 size_read = (u32)(s->in - chunk_start);
            neko_assert(size_read == chunk_size);
        }
    }

    return ase;
}

void neko_aseprite_default_blend_bind(ase_t* ase) {

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        frame->pixels[0] = (neko_color_t*)neko_safe_malloc((size_t)(sizeof(neko_color_t)) * ase->w * ase->h);
        std::memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t* dst = frame->pixels[0];

        neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t* cel = frame->cels + j;

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
}

void neko_aseprite_free(ase_t* ase) {
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        neko_safe_free(frame->pixels[0]);
        for (int j = 0; j < frame->cel_count; ++j) {
            ase_cel_t* cel = frame->cels + j;
            neko_safe_free(cel->cel_pixels);
            neko_safe_free((void*)cel->udata.text);
        }
    }
    for (int i = 0; i < ase->layer_count; ++i) {
        ase_layer_t* layer = ase->layers + i;
        neko_safe_free((void*)layer->name);
        neko_safe_free((void*)layer->udata.text);
    }
    for (int i = 0; i < ase->tag_count; ++i) {
        ase_tag_t* tag = ase->tags + i;
        neko_safe_free((void*)tag->name);
    }
    for (int i = 0; i < ase->slice_count; ++i) {
        ase_slice_t* slice = ase->slices + i;
        neko_safe_free((void*)slice->udata.text);
    }
    if (ase->slice_count) {
        neko_safe_free((void*)ase->slices[0].name);
    }
    for (int i = 0; i < ase->palette.entry_count; ++i) {
        neko_safe_free((void*)ase->palette.entries[i].color_name);
    }
    neko_safe_free(ase->color_profile.icc_profile_data);
    neko_safe_free(ase->frames);
    neko_safe_free(ase);
}

#pragma endregion