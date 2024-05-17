

#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_math.h"
#include "sandbox/game_pixelui.h"
#include "sandbox/magic_pixel.h"

class Graphics {
public:
    static void DrawCircle(neko_immediate_draw_t* idraw, const neko_vec2_t center, const neko_vec4_t bounds, u16 radius);
};

class sandbox_object {
public:
    sandbox_object(neko_vec4_t rect);
    //    ~GameObject();
    void clean();
    void update();
    void render(f32 scale);
    void chunk_mask_update_mesh();
    neko_texture_t object_texture;
    neko_color_t* draw_buffer;
    neko_vec4_t rect;
    unsigned char* data;
    bool* edgeSeen;

private:
};

class Simulation;
class Game {
public:
    Game(neko_immediate_draw_t* idraw);
    ~Game();
    void PreUpdate();
    void Update();
    void LateUpdate();
    void Render();
    void Clean();
    void SetMaterial(int material);
    void Pause(int x);
    void ResetSimulation(int x);

    Simulation* get_sim() const { return simulation; }

public:
    static neko_immediate_draw_t* idraw;

    f32 fbs_scale;
    f32 draw_scale;

private:
    void Init(neko_immediate_draw_t* idraw);
    void CreateSimulation();
    void CreateViewPort();
    void CreateUI();
    void ResetVariables();
    Simulation* simulation;
    sandbox_object* viewport;
    pixelui_t pixelui;
    u32 tick_count;
    u16 draw_radius;
    material_type material;
    int count;
    bool debug_mode = false;
    bool paused = false;
};

///////////////////////////////////////////////
//
//  位图边缘检测算法

typedef struct {
    short x, y;
} neko_tex_point;

typedef s32 neko_tex_bool;

// 2d point type helpers
#define __neko_tex_point_add(result, a, b) \
    {                                      \
        (result).x = (a).x + (b).x;        \
        (result).y = (a).y + (b).y;        \
    }
#define __neko_tex_point_sub(result, a, b) \
    {                                      \
        (result).x = (a).x - (b).x;        \
        (result).y = (a).y - (b).y;        \
    }
#define __neko_tex_point_is_inside(a, w, h) ((a).x >= 0 && (a).y >= 0 && (a).x < (w) && (a).y < (h))
#define __neko_tex_point_is_next_to(a, b) ((a).x - (b).x <= 1 && (a).x - (b).x >= -1 && (a).y - (b).y <= 1 && (a).y - (b).y >= -1)

// direction type
typedef s32 neko_tex_direction;  // 8 cw directions: >, _|, v, |_, <, |", ^, "|
#define __neko_tex_direction_opposite(dir) ((dir + 4) & 7)
static const neko_tex_point neko_tex_direction_to_pixel_offset[] = {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};

// image manipulation functions
u8* neko_tex_rgba_to_alpha(const u8* data, s32 w, s32 h);
u8* neko_tex_alpha_to_thresholded(const u8* data, s32 w, s32 h, u8 threshold);
u8* neko_tex_dilate_thresholded(const u8* data, s32 w, s32 h);
u8* neko_tex_thresholded_to_outlined(const u8* data, s32 w, s32 h);

// outline path procedures
static neko_tex_bool neko_tex_find_first_filled_pixel(const u8* data, s32 w, s32 h, neko_tex_point* first);
static neko_tex_bool neko_tex_find_next_filled_pixel(const u8* data, s32 w, s32 h, neko_tex_point current, neko_tex_direction* dir, neko_tex_point* next);
neko_tex_point* neko_tex_extract_outline_path(u8* data, s32 w, s32 h, s32* point_count, neko_tex_point* reusable_outline);

void neko_tex_distance_based_path_simplification(neko_tex_point* outline, s32* outline_length, f32 distance_threshold);

#if 1

// color brightness as perceived:
f32 brightness(neko_color_t c);
f32 color_num(neko_color_t c);

#define __check_hsv(c0, c1, p_func)                                            \
    do {                                                                       \
        hsv_t hsv0 = rgb_to_hsv(c0);                                           \
        hsv_t hsv1 = rgb_to_hsv(c1);                                           \
        f32 d = abs(color_num(c0) - color_num(c1)) + hue_dist(hsv0.h, hsv1.h); \
        if (d < min_dist) {                                                    \
            min_dist = d;                                                      \
            p = p_func();                                                      \
        }                                                                      \
    } while (0)

#define __check_dist_euclidean(c0, c1, p_func)                                \
    do {                                                                      \
        neko_vec4 c0_vec = neko_vec4{(f32)c0.r, (f32)c0.g, (f32)c0.b, 255.f}; \
        neko_vec4 c1_vec = neko_vec4{(f32)c1.r, (f32)c1.g, (f32)c1.b, 255.f}; \
        f32 d = neko_vec4_dist(c0_vec, c1_vec);                               \
        if (d < min_dist) {                                                   \
            min_dist = d;                                                     \
            p = p_func();                                                     \
        }                                                                     \
    } while (0)

#define __check_dist(c0, c1, p_func)                                          \
    do {                                                                      \
        f32 rd = (f32)c0.r - (f32)c1.r;                                       \
        f32 gd = (f32)c0.g - (f32)c1.g;                                       \
        f32 bd = (f32)c0.b - (f32)c1.b;                                       \
        f32 sd = rd * rd + gd * gd + bd * bd;                                 \
        f32 d = pow(rd * 0.299, 2) + pow(gd * 0.587, 2) + pow(bd * 0.114, 2); \
        if (d < min_dist) {                                                   \
            min_dist = d;                                                     \
            p = p_func();                                                     \
        }                                                                     \
    } while (0)

// particle_t get_closest_particle_from_color(neko_color_t c);

#endif
