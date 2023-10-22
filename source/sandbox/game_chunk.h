

#ifndef NEKO_GAME_CHUNK_H
#define NEKO_GAME_CHUNK_H

#include "engine/neko.h"
#include "engine/neko_engine.h"
#include "libs/glad/glad.h"

// game
#include "game_physics_math.hpp"
#include "neko_client_ecs.h"

#define g_window_width 1280
#define g_window_height 740

neko_global f32 g_scale = 8;

neko_global const s32 CHUNK_W = 64;
neko_global const s32 CHUNK_H = 64;

// neko engine cpp
using namespace neko;

f32 brush_size = 0.5f;
f32 world_gravity = 10.f;

typedef neko_color_t cell_color_t;

typedef u32 mat_id_t;

enum mat_physics_type {
    AIR = 0,
    SOLID = 1,
    SAND = 2,
    LIQUID = 3,
    GAS = 4,
    PASSABLE = 5,
    OBJECT = 6,
};

typedef struct particle_t {
    mat_id_t mat_id;
    f32 life_time;
    neko_vec2 velocity;
    cell_color_t color;
    bool has_been_updated_this_frame;
} particle_t;

typedef struct game_chunk_s {
    // neko_vec2 position; // 实际位置
    s32 index_x;  // 相对位置
    s32 index_y;  // 相对位置
    particle_t* world_particle_data;
    cell_color_t* texture_buffer;
    neko_texture_t tex;  // TODO: 23/10/20 一个区块一个贴图的立即渲染效率不高 后期改成把每个区块数据贴到一个大贴图上
} game_chunk_t;

// TODO: 23/10/20
// 因为在各种关于particle的update函数中 particle的位置永远是对于
// 某个区域块(chunk)而言的相对位置
// 即永远在0-64区间 这可以通过2^5二进制位来存储
// 一个chunk的相对位置只需要10个bit 而非两个u32

game_chunk_t game_chunk_ctor(s32 x, s32 y) {
    game_chunk_t ch;
    ch.index_x = x;
    ch.index_y = y;
    return ch;
}

struct mat_t;  // fwd

typedef particle_t (*particle_ctor_func)(void);
typedef void (*particle_update_func)(game_chunk_t* chunk, mat_t* mat, u32 x, u32 y);

typedef struct mat_t {
    mat_id_t id;

    char name[64];
    cell_color_t color;
    mat_physics_type phy_type;

    particle_ctor_func p_func;
    particle_update_func update_func;

    // neko_texture_t texture;
} mat_t;

neko_global neko_texture_t g_tex_ui = {0};
// neko_global neko_texture_t g_rt = {0};
// neko_global neko_texture_t g_test_ase = {0};
//  neko_global neko_frame_buffer_t g_fb = {0};
//  neko_global blur_pass_t g_blur_pass = {0};
//  neko_global bright_filter_pass_t g_bright_pass = {0};
//  neko_global composite_pass_t g_composite_pass = {0};

s32 l;
s32 l_check;

// For now, all particle information will simply be a value to determine its material mat_id
#define mat_id_empty (mat_id_t)0
#define mat_id_sand (mat_id_t)1
#define mat_id_water (mat_id_t)2
#define mat_id_salt (mat_id_t)3
#define mat_id_wood (mat_id_t)4
#define mat_id_fire (mat_id_t)5
#define mat_id_smoke (mat_id_t)6
#define mat_id_ember (mat_id_t)7
#define mat_id_steam (mat_id_t)8
#define mat_id_gunpowder (mat_id_t)9
#define mat_id_oil (mat_id_t)10
#define mat_id_lava (mat_id_t)11
#define mat_id_stone (mat_id_t)12
#define mat_id_acid (mat_id_t)13

// Colors
#define mat_col_empty \
    cell_color_t { 0, 0, 0, 0 }
#define mat_col_sand \
    cell_color_t { 150, 100, 50, 255 }
#define mat_col_salt \
    cell_color_t { 200, 180, 190, 255 }
#define mat_col_water \
    cell_color_t { 20, 100, 170, 200 }
#define mat_col_stone \
    cell_color_t { 120, 110, 120, 255 }
#define mat_col_wood \
    cell_color_t { 60, 40, 20, 255 }
#define mat_col_fire \
    cell_color_t { 150, 20, 0, 255 }
#define mat_col_smoke \
    cell_color_t { 50, 50, 50, 255 }
#define mat_col_ember \
    cell_color_t { 200, 120, 20, 255 }
#define mat_col_steam \
    cell_color_t { 220, 220, 250, 255 }
#define mat_col_gunpowder \
    cell_color_t { 60, 60, 60, 255 }
#define mat_col_oil \
    cell_color_t { 80, 70, 60, 255 }
#define mat_col_lava \
    cell_color_t { 200, 50, 0, 255 }
#define mat_col_acid \
    cell_color_t { 90, 200, 60, 255 }

typedef enum material_selection {
    mat_sel_sand = 0x00,
    mat_sel_water,
    mat_sel_salt,
    mat_sel_wood,
    mat_sel_fire,
    mat_sel_smoke,
    mat_sel_steam,
    mat_sel_gunpowder,
    mat_sel_oil,
    mat_sel_lava,
    mat_sel_stone,
    mat_sel_acid
} material_selection;

// Material selection for "painting" / default to sand
neko_global material_selection g_material_selection = mat_sel_sand;

// UI texture buffer
neko_global cell_color_t* g_ui_buffer = {0};

// Frame counter
neko_global u32 g_frame_counter = 0;

neko_global bool g_show_material_selection_panel = true;
neko_global bool g_show_frame_count = true;

neko_global neko_vec2 g_chunk_render_pos;

neko_hash_table(u32, game_chunk_t) g_chunk_data;

// pixelui
#include "game_pixelui.h"

// Handle for main window
// neko_global neko_resource_handle g_window;

#if 0

const char* pixel_v_src = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_texCoord;
uniform int u_flip_y;
out vec2 texCoord;
void main()
{
   gl_Position = vec4(a_pos, 0.0, 1.0);
   texCoord = vec2(a_texCoord.x, bool(u_flip_y) ? 1.0 - a_texCoord.y : a_texCoord.y);
}
)glsl";

const char* pixel_f_src = R"glsl(
#version 330 core
in vec2 texCoord;
out vec4 frag_color;
uniform sampler2D u_tex;

#define COLOR_STEP 32.0
#define PIXEL_SIZE 2.0

vec4 colorize(in vec4 color) {

    // Pixel art coloring
    vec3 nCol = normalize(color.rgb);
    float nLen = length(color.rgb);
    return vec4(nCol * round(nLen * COLOR_STEP) / COLOR_STEP, color.w);

}

void main()
{
   //frag_color = colorize(texture(u_tex, texCoord));
   frag_color = texture(u_tex, texCoord);
}
)glsl";

#endif

particle_t particle_empty();
particle_t particle_sand();
particle_t particle_water();
particle_t particle_salt();
particle_t particle_wood();
particle_t particle_fire();
particle_t particle_lava();
particle_t particle_smoke();
particle_t particle_ember();
particle_t particle_steam();
particle_t particle_gunpowder();
particle_t particle_oil();
particle_t particle_stone();
particle_t particle_acid();

void update_input();
bool update_ui();

// Particle updates
void chunk_update_particle(game_chunk_t* chunk);
void update_default(game_chunk_t* chunk, u32 x, u32 y);
void update_particle(game_chunk_t* chunk, mat_t* mat, u32 x, u32 y);

void chunk_init(game_chunk_t* chunk);
void chunk_destroy(game_chunk_t* chunk);
void chunk_update(game_chunk_t* chunk);
void chunk_update_mesh(game_chunk_t* chunk);

// Utilities for writing data into color buffer
void chunk_write_data(game_chunk_t* chunk, u32 idx, particle_t);

u8* neko_tex_rgba_to_alpha(const u8* data, s32 w, s32 h);
u8* neko_tex_alpha_to_thresholded(const u8* data, s32 w, s32 h, u8 threshold);
u8* neko_tex_thresholded_to_outlined(const u8* data, s32 w, s32 h);

typedef struct {
    short x, y;
} neko_tex_point;
neko_tex_point* neko_tex_extract_outline_path(u8* data, s32 w, s32 h, s32* point_count, neko_tex_point* reusable_outline);
void neko_tex_distance_based_path_simplification(neko_tex_point* outline, s32* outline_length, f32 distance_threshold);

void render_chunk_immediate(game_chunk_t* chunk);

s32 compute_idx(s32 x, s32 y) { return (y * CHUNK_W + x); }

neko_vec2 calc_chunk_position(game_chunk_t* chunk) {

    f32 x = (chunk->index_x) * CHUNK_W * g_scale;
    f32 y = (chunk->index_y) * CHUNK_H * g_scale;

    return neko_v2(x, y);
}

bool in_bounds(s32 x, s32 y) {
    if (x < 0 || x > (CHUNK_W - 1) || y < 0 || y > (CHUNK_H - 1)) return false;
    return true;
}

s32 random_val(s32 lower, s32 upper) {

    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand() % (upper - lower + 1) + lower);
}

bool is_empty(game_chunk_t* chunk, s32 x, s32 y) { return (in_bounds(x, y) && chunk->world_particle_data[compute_idx(x, y)].mat_id == mat_id_empty); }

particle_t get_particle_at(game_chunk_t* chunk, s32 x, s32 y) { return chunk->world_particle_data[compute_idx(x, y)]; }

bool completely_surrounded(game_chunk_t* chunk, s32 x, s32 y) {
    // Top
    if (in_bounds(x, y - 1) && !is_empty(chunk, x, y - 1)) {
        return false;
    }
    // Bottom
    if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1)) {
        return false;
    }
    // Left
    if (in_bounds(x - 1, y) && !is_empty(chunk, x - 1, y)) {
        return false;
    }
    // Right
    if (in_bounds(x + 1, y) && !is_empty(chunk, x + 1, y)) {
        return false;
    }
    // Top Left
    if (in_bounds(x - 1, y - 1) && !is_empty(chunk, x - 1, y - 1)) {
        return false;
    }
    // Top Right
    if (in_bounds(x + 1, y - 1) && !is_empty(chunk, x + 1, y - 1)) {
        return false;
    }
    // Bottom Left
    if (in_bounds(x - 1, y + 1) && !is_empty(chunk, x - 1, y + 1)) {
        return false;
    }
    // Bottom Right
    if (in_bounds(x + 1, y + 1) && !is_empty(chunk, x + 1, y + 1)) {
        return false;
    }

    return true;
}

bool is_in_liquid(game_chunk_t* chunk, s32 x, s32 y, s32* lx, s32* ly) {
    if (in_bounds(x, y) && (get_particle_at(chunk, x, y).mat_id == mat_id_water || get_particle_at(chunk, x, y).mat_id == mat_id_oil)) {
        *lx = x;
        *ly = y;
        return true;
    }
    if (in_bounds(x, y - 1) && (get_particle_at(chunk, x, y - 1).mat_id == mat_id_water || get_particle_at(chunk, x, y - 1).mat_id == mat_id_oil)) {
        *lx = x;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x, y + 1) && (get_particle_at(chunk, x, y + 1).mat_id == mat_id_water || get_particle_at(chunk, x, y + 1).mat_id == mat_id_oil)) {
        *lx = x;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x - 1, y) && (get_particle_at(chunk, x - 1, y).mat_id == mat_id_water || get_particle_at(chunk, x - 1, y).mat_id == mat_id_oil)) {
        *lx = x - 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x - 1, y - 1) && (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_water || get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_oil)) {
        *lx = x - 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x - 1, y + 1) && (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_water || get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_oil)) {
        *lx = x - 1;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x + 1, y) && (get_particle_at(chunk, x + 1, y).mat_id == mat_id_water || get_particle_at(chunk, x + 1, y).mat_id == mat_id_oil)) {
        *lx = x + 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x + 1, y - 1) && (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_water || get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_oil)) {
        *lx = x + 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x + 1, y + 1) && (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_water || get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_oil)) {
        *lx = x + 1;
        *ly = y + 1;
        return true;
    }
    return false;
}

bool is_in_water(game_chunk_t* chunk, s32 x, s32 y, s32* lx, s32* ly) {
    if (in_bounds(x, y) && (get_particle_at(chunk, x, y).mat_id == mat_id_water)) {
        *lx = x;
        *ly = y;
        return true;
    }
    if (in_bounds(x, y - 1) && (get_particle_at(chunk, x, y - 1).mat_id == mat_id_water)) {
        *lx = x;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x, y + 1) && (get_particle_at(chunk, x, y + 1).mat_id == mat_id_water)) {
        *lx = x;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x - 1, y) && (get_particle_at(chunk, x - 1, y).mat_id == mat_id_water)) {
        *lx = x - 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x - 1, y - 1) && (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_water)) {
        *lx = x - 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x - 1, y + 1) && (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_water)) {
        *lx = x - 1;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x + 1, y) && (get_particle_at(chunk, x + 1, y).mat_id == mat_id_water)) {
        *lx = x + 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x + 1, y - 1) && (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_water)) {
        *lx = x + 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x + 1, y + 1) && (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_water)) {
        *lx = x + 1;
        *ly = y + 1;
        return true;
    }
    return false;
}

typedef struct hsv_t {
    f32 h;
    f32 s;
    f32 v;
} hsv_t;

// From on: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
hsv_t rgb_to_hsv(cell_color_t c) {
    neko_vec3 cv = neko_vec3{(f32)c.r / 255.f, (f32)c.g / 255.f, (f32)c.b / 255.f};
    f32 fR = cv.x, fG = cv.y, fB = cv.z;

    f32 fCMax = neko_max(neko_max(fR, fG), fB);
    f32 fCMin = neko_min(neko_min(fR, fG), fB);
    f32 fDelta = fCMax - fCMin;

    hsv_t hsv;

    if (fDelta > 0) {
        if (fCMax == fR) {
            hsv.h = 60 * (fmod(((fG - fB) / fDelta), 6));
        } else if (fCMax == fG) {
            hsv.h = 60 * (((fB - fR) / fDelta) + 2);
        } else if (fCMax == fB) {
            hsv.h = 60 * (((fR - fG) / fDelta) + 4);
        }

        if (fCMax > 0) {
            hsv.s = fDelta / fCMax;
        } else {
            hsv.s = 0;
        }

        hsv.v = fCMax;
    } else {
        hsv.h = 0;
        hsv.s = 0;
        hsv.v = fCMax;
    }

    if (hsv.h < 0) {
        hsv.h = 360 + hsv.h;
    }

    return hsv;
}

// Implemented from: https://stackoverflow.com/questions/27374550/how-to-compare-color-object-and-get-closest-color-in-an-color
// distance between two hues:
f32 hue_dist(f32 h1, f32 h2) {
    f32 d = fabsf(h1 - h2);
    return d > 180.f ? 360.f - d : d;
}

// color brightness as perceived:
f32 brightness(cell_color_t c) { return ((f32)c.r * 0.299f + (f32)c.g * 0.587f + (f32)c.b * 0.114f) / 256.f; }

f32 color_num(cell_color_t c) {
    const f32 bright_factor = 100.0f;
    const f32 sat_factor = 0.1f;
    hsv_t hsv = rgb_to_hsv(c);
    return hsv.s * sat_factor + brightness(c) * bright_factor;
}

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

particle_t get_closest_particle_from_color(cell_color_t c) {
    particle_t p = particle_empty();
    f32 min_dist = f32_max;
    neko_vec4 c_vec = neko_vec4{(f32)c.r, (f32)c.g, (f32)c.b, (f32)c.a};
    u8 id = mat_id_empty;

    __check_dist_euclidean(c, mat_col_sand, particle_sand);
    __check_dist_euclidean(c, mat_col_water, particle_water);
    __check_dist_euclidean(c, mat_col_salt, particle_salt);
    __check_dist_euclidean(c, mat_col_wood, particle_wood);
    __check_dist_euclidean(c, mat_col_fire, particle_fire);
    __check_dist_euclidean(c, mat_col_smoke, particle_smoke);
    __check_dist_euclidean(c, mat_col_steam, particle_steam);
    __check_dist_euclidean(c, mat_col_gunpowder, particle_gunpowder);
    __check_dist_euclidean(c, mat_col_oil, particle_oil);
    __check_dist_euclidean(c, mat_col_lava, particle_lava);
    __check_dist_euclidean(c, mat_col_stone, particle_stone);
    __check_dist_euclidean(c, mat_col_acid, particle_acid);

    return p;
}

void drop_file_callback(void* platform_window, s32 count, const char** file_paths) {
    if (count < 1) return;

    // Just do first one for now...
    if (count > 1) count = 1;

    // We'll place at the mouse position as well, for shiggles
    neko_vec2 mp = calculate_mouse_position();

    for (s32 i = 0; i < count; ++i) {
        // Need to verify this IS an image first.
        char temp_file_extension_buffer[16] = {0};
        neko_util_get_file_extension(temp_file_extension_buffer, sizeof(temp_file_extension_buffer), file_paths[0]);
        if (neko_string_compare_equal(temp_file_extension_buffer, "png") || neko_string_compare_equal(temp_file_extension_buffer, "jpg") ||
            neko_string_compare_equal(temp_file_extension_buffer, "jpeg") || neko_string_compare_equal(temp_file_extension_buffer, "bmp")) {
            // Load texture into memory
            s32 _w, _h;
            u32 _n;
            void* texture_data = NULL;

            // Force texture data to 3 components
            neko_util_load_texture_data_from_file(file_paths[i], &_w, &_h, &_n, &texture_data, false);
            _n = 3;

            // Not sure what the format should be, so this is ...blah. Need to find a way to determine this beforehand.
            u8* data = (u8*)texture_data;

            s32 sx = (CHUNK_W - _w) / 2;
            s32 sy = (CHUNK_H - _h) / 2;

            // Now we need to process the data and place it into our particle/color buffers
            for (u32 h = 0; h < _h; ++h) {
                for (u32 w = 0; w < _w; ++w) {
                    cell_color_t c = {data[(h * _w + w) * _n + 0], data[(h * _w + w) * _n + 1], data[(h * _w + w) * _n + 2], 255};

                    // Get color of this pixel in the image
                    particle_t p = get_closest_particle_from_color(c);

                    // Let's place this thing in the center instead...
                    if (in_bounds(sx + w, sy + h)) {

                        u32 idx = compute_idx(sx + w, sy + h);
                        game_chunk_t* chunk = neko_hash_table_getp(g_chunk_data, 0);
                        chunk_write_data(chunk, idx, p);
                    }
                }
            }

            // Free texture data
            free(texture_data);
        }
    }
}

neko_inline neko_vec2 world_to_screen_pos(const neko_vec2& pos) {
    f32 screenX = pos.x * g_scale;
    f32 screenY = g_window_height - pos.y * g_scale;
    return neko_vec2{screenX, screenY};
}

neko_inline neko_vec2 screen_to_world_pos(const neko_vec2& pos) {
    f32 worldX = pos.x / g_scale;
    f32 worldY = (g_window_height - pos.y) / g_scale;
    return neko_vec2{worldX, worldY};
}

neko_hash_table(u8, mat_t) g_mat_data;

neko_inline mat_t mat_ctor(u64 id, const_str name, cell_color_t color, mat_physics_type type, particle_ctor_func pfunc, particle_update_func ufunc) {
    mat_t m;
    neko_snprintf(m.name, 64, "%s", name);
    m.id = id;
    m.color = color;
    m.phy_type = type;
    // m.texture = tex;
    m.p_func = pfunc;
    m.update_func = ufunc;
    return m;
}

void game_matdata_init() {

#define register_mat(id, name, color, phy_type, pfunc, ufunc) neko_hash_table_insert(g_mat_data, id, mat_ctor(id, name, color, phy_type, pfunc, ufunc))

    // clang-format off
    //      ID              名字         颜色          物理性质                构造函数              更新函数
    mat_t mat_list[] = {
        {mat_id_sand,       "sand",         {}, mat_physics_type::SAND,     particle_sand,      update_particle},
        {mat_id_water,      "water",        {}, mat_physics_type::LIQUID,   particle_water,     update_particle},
        {mat_id_salt,       "salt",         {}, mat_physics_type::SAND,     particle_salt,      update_particle},
        {mat_id_wood,       "wood",         {}, mat_physics_type::SOLID,    particle_wood,      update_particle},
        {mat_id_fire,       "fire",         {}, mat_physics_type::PASSABLE, particle_fire,      update_particle},
        {mat_id_smoke,      "smoke",        {}, mat_physics_type::GAS,      particle_smoke,     update_particle},
        {mat_id_ember,      "ember",        {}, mat_physics_type::SAND,     particle_ember,     update_particle},
        {mat_id_steam,      "steam",        {}, mat_physics_type::GAS,      particle_steam,     update_particle},
        {mat_id_gunpowder,  "gunpowder",    {}, mat_physics_type::SAND,     particle_gunpowder, update_particle},
        {mat_id_oil,        "oil",          {}, mat_physics_type::LIQUID,   particle_oil,       update_particle},
        {mat_id_lava,       "lava",         {}, mat_physics_type::LIQUID,   particle_lava,      update_particle},
        {mat_id_stone,      "stone",        {}, mat_physics_type::SOLID,    particle_lava,      update_particle},
        {mat_id_acid,       "acid",         {}, mat_physics_type::LIQUID,   particle_acid,      update_particle},
    };
    // clang-format on

    for (int i = 0; i < neko_arr_size(mat_list); i++) {
        neko_hash_table_insert(g_mat_data, mat_list[i].id, mat_list[i]);
    }
}

void game_matdata_destroy() { neko_hash_table_free(g_mat_data); }

void game_chunk_init() {

    g_chunk_render_pos = {600, 50};

    game_matdata_init();

    g_ui_buffer = (cell_color_t*)neko_safe_malloc(g_pixelui_texture_width * g_pixelui_texture_height * sizeof(cell_color_t));
    memset(g_ui_buffer, 0, g_pixelui_texture_width * g_pixelui_texture_height);

    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = g_pixelui_texture_width;
    t_desc.height = g_pixelui_texture_height;
    t_desc.data[0] = g_ui_buffer;

    g_tex_ui = neko_graphics_texture_create(&t_desc);

    neko_hash_table_insert(g_chunk_data, 0, game_chunk_ctor(0, 0));
    neko_hash_table_insert(g_chunk_data, 1, game_chunk_ctor(0, 1));
    neko_hash_table_insert(g_chunk_data, 2, game_chunk_ctor(1, 0));
    neko_hash_table_insert(g_chunk_data, 3, game_chunk_ctor(1, 1));

    for (neko_hash_table_iter it = neko_hash_table_iter_new(g_chunk_data); neko_hash_table_iter_valid(g_chunk_data, it); neko_hash_table_iter_advance(g_chunk_data, it)) {
        u32 k = neko_hash_table_iter_getk(g_chunk_data, it);
        game_chunk_t* chunk = neko_hash_table_iter_getp(g_chunk_data, it);

        chunk_init(chunk);
    }

    // Load UI font texture data from file
    construct_font_data(g_font_data);

    // Set up callback for dropping them files, yo.
    neko_platform_set_dropped_files_callback(neko_platform_main_window(), &drop_file_callback);
}

void game_chunk_destroy() {

    for (neko_hash_table_iter it = neko_hash_table_iter_new(g_chunk_data); neko_hash_table_iter_valid(g_chunk_data, it); neko_hash_table_iter_advance(g_chunk_data, it)) {
        u32 k = neko_hash_table_iter_getk(g_chunk_data, it);
        game_chunk_t* chunk = neko_hash_table_iter_getp(g_chunk_data, it);

        chunk_destroy(chunk);
    }

    neko_graphics_texture_destroy(g_tex_ui);
    neko_safe_free(g_ui_buffer);

    game_matdata_destroy();
}
void game_chunk_update() {

    bool ui_interaction = update_ui();

    if (!ui_interaction) {
        update_input();
    }

    for (neko_hash_table_iter it = neko_hash_table_iter_new(g_chunk_data); neko_hash_table_iter_valid(g_chunk_data, it); neko_hash_table_iter_advance(g_chunk_data, it)) {
        u32 k = neko_hash_table_iter_getk(g_chunk_data, it);
        game_chunk_t* chunk = neko_hash_table_iter_getp(g_chunk_data, it);

        chunk_update(chunk);
        chunk_update_mesh(chunk);
    }

    neko_immediate_draw_t* idraw = ((neko_client_ecs_userdata_s*)neko_ecs()->user_data)->idraw;

    const neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_rect_2d_textured_ext(idraw, 0, 0, fbs.x, fbs.y, 0, 1, 1, 0, g_tex_ui.id, NEKO_COLOR_WHITE);

    // Update frame counter
    g_frame_counter = (g_frame_counter + 1) % u32_max;
}

void chunk_init(game_chunk_t* chunk) {

    // chunk = (game_chunk_t*)neko_safe_malloc(sizeof(game_chunk_t));

    // Construct world data (for now, it'll just be the size of the screen)
    chunk->world_particle_data = (particle_t*)neko_safe_malloc(CHUNK_W * CHUNK_H * sizeof(particle_t));

    // Construct texture buffer data
    chunk->texture_buffer = (cell_color_t*)neko_safe_malloc(CHUNK_W * CHUNK_H * sizeof(cell_color_t));

    // Set buffers to 0
    memset(chunk->texture_buffer, 0, CHUNK_W * CHUNK_H * sizeof(cell_color_t));
    memset(chunk->world_particle_data, 0, CHUNK_W * CHUNK_H * sizeof(particle_t));

    // Construct texture resource from GPU
    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = CHUNK_W;
    t_desc.height = CHUNK_H;

    t_desc.data[0] = chunk->texture_buffer;

    chunk->tex = neko_graphics_texture_create(&t_desc);

    // Construct target for offscreen rendering
    // t_desc.data = NULL;
    // g_rt = gfx->construct_texture(t_desc);

    // Construct frame buffer
    // g_fb = gfx->construct_frame_buffer(g_rt);

    // Initialize render passes
    // g_blur_pass = blur_pass_ctor();
    // g_bright_pass = bright_filter_pass_ctor();
    // g_composite_pass = composite_pass_ctor();
}

void chunk_destroy(game_chunk_t* chunk) {

    neko_graphics_texture_destroy(chunk->tex);

    neko_safe_free(chunk->world_particle_data);
    neko_safe_free(chunk->texture_buffer);

    // neko_safe_free(chunk);
}

void chunk_update(game_chunk_t* chunk) {
    chunk_update_particle(chunk);
    render_chunk_immediate(chunk);
}

void update_input() {

    if (neko_platform_key_pressed(NEKO_KEYCODE_I)) {
        g_show_material_selection_panel = !g_show_material_selection_panel;
    }

    if (neko_platform_key_pressed(NEKO_KEYCODE_F)) {
        g_show_frame_count = !g_show_frame_count;
    }

    f32 wx = 0, wy = 0;
    neko_platform_mouse_wheel(&wx, &wy);
    if (neko_platform_key_pressed(NEKO_KEYCODE_LEFT_BRACKET) || wy < 0.f) {
        brush_size = neko_clamp(brush_size - 0.5f, 0.5f, 50.f);
    }
    if (neko_platform_key_pressed(NEKO_KEYCODE_RIGHT_BRACKET) || wy > 0.f) {
        brush_size = neko_clamp(brush_size + 0.5f, 0.5f, 50.f);
    }

    if (neko_platform_key_pressed(NEKO_KEYCODE_PAGE_UP)) {
        g_scale += 1.f;
    } else if (neko_platform_key_pressed(NEKO_KEYCODE_PAGE_DOWN)) {
        g_scale -= 1.f;
    }

    if (neko_platform_key_pressed(NEKO_KEYCODE_W)) {
        g_chunk_render_pos.y += 10.f;
    }
    if (neko_platform_key_pressed(NEKO_KEYCODE_S)) {
        g_chunk_render_pos.y -= 10.f;
    }
    if (neko_platform_key_pressed(NEKO_KEYCODE_A)) {
        g_chunk_render_pos.x -= 10.f;
    }
    if (neko_platform_key_pressed(NEKO_KEYCODE_D)) {
        g_chunk_render_pos.x += 10.f;
    }

    // Clear data
    if (neko_platform_key_pressed(NEKO_KEYCODE_C)) {

        for (neko_hash_table_iter it = neko_hash_table_iter_new(g_chunk_data); neko_hash_table_iter_valid(g_chunk_data, it); neko_hash_table_iter_advance(g_chunk_data, it)) {
            u32 k = neko_hash_table_iter_getk(g_chunk_data, it);
            game_chunk_t* chunk = neko_hash_table_iter_getp(g_chunk_data, it);

            memset(chunk->texture_buffer, 0, sizeof(cell_color_t) * CHUNK_W * CHUNK_H);
            memset(chunk->world_particle_data, 0, sizeof(particle_t) * CHUNK_W * CHUNK_H);
        }
    }

    // Mouse input for testing
    if (neko_platform_mouse_down(NEKO_MOUSE_LBUTTON)) {
        neko_vec2 mp = neko_platform_mouse_positionv();

        for (neko_hash_table_iter it = neko_hash_table_iter_new(g_chunk_data); neko_hash_table_iter_valid(g_chunk_data, it); neko_hash_table_iter_advance(g_chunk_data, it)) {
            u32 k = neko_hash_table_iter_getk(g_chunk_data, it);
            game_chunk_t* chunk = neko_hash_table_iter_getp(g_chunk_data, it);

            // 此区块屏幕空间位置
            neko_vec2 position = neko_vec2_add(calc_chunk_position(chunk), g_chunk_render_pos);
            // 鼠标位置与此区块屏幕空间位置差值
            neko_vec2 peek_pos = neko_vec2_sub(mp, position);

            if ((peek_pos.x >= 0 && peek_pos.y >= 0) && (peek_pos.x <= CHUNK_W * g_scale && peek_pos.y <= CHUNK_H * g_scale)) {

                f32 mp_x = neko_clamp(peek_pos.x, 0.f, (f32)CHUNK_W * g_scale - 1.f) / g_scale;
                f32 mp_y = neko_clamp(peek_pos.y, 0.f, (f32)CHUNK_H * g_scale - 1.f) / g_scale;
                u32 max_idx = (CHUNK_W * CHUNK_H) - 1;
                s32 r_amt = random_val(1, 10000);
                const f32 R = brush_size * g_scale;

                // Spawn in a circle around the mouse
                for (u32 i = 0; i < r_amt; ++i) {
                    f32 ran = (f32)random_val(0, 100) / 100.f;
                    f32 r = R * sqrt(ran);
                    f32 theta = (f32)random_val(0, 100) / 100.f * 2.f * neko_pi;
                    f32 rx = cos((f32)theta) * r;
                    f32 ry = sin((f32)theta) * r;
                    s32 mpx = (s32)neko_clamp(mp_x + (f32)rx, 0.f, (f32)CHUNK_W - 1.f);
                    s32 mpy = (s32)neko_clamp(mp_y + (f32)ry, 0.f, (f32)CHUNK_H - 1.f);
                    s32 idx = mpy * (s32)CHUNK_W + mpx;
                    idx = neko_clamp(idx, 0, max_idx);

                    if (is_empty(chunk, mpx, mpy)) {
                        particle_t p = {0};
                        switch (g_material_selection) {
                            case mat_sel_sand:
                                p = particle_sand();
                                break;
                            case mat_sel_water:
                                p = particle_water();
                                break;
                            case mat_sel_salt:
                                p = particle_salt();
                                break;
                            case mat_sel_wood:
                                p = particle_wood();
                                break;
                            case mat_sel_fire:
                                p = particle_fire();
                                break;
                            case mat_sel_smoke:
                                p = particle_smoke();
                                break;
                            case mat_sel_steam:
                                p = particle_steam();
                                break;
                            case mat_sel_gunpowder:
                                p = particle_gunpowder();
                                break;
                            case mat_sel_oil:
                                p = particle_oil();
                                break;
                            case mat_sel_lava:
                                p = particle_lava();
                                break;
                            case mat_sel_stone:
                                p = particle_stone();
                                break;
                            case mat_sel_acid:
                                p = particle_acid();
                                break;
                        }
                        p.velocity = neko_vec2{(f32)random_val(-1, 1), (f32)random_val(-2, 5)};
                        chunk_write_data(chunk, idx, p);
                    }
                }
            }
        }
    }

    // Solid Erase
    if (neko_platform_mouse_down(NEKO_MOUSE_RBUTTON)) {
        neko_vec2 mp = neko_platform_mouse_positionv();

        for (neko_hash_table_iter it = neko_hash_table_iter_new(g_chunk_data); neko_hash_table_iter_valid(g_chunk_data, it); neko_hash_table_iter_advance(g_chunk_data, it)) {
            u32 k = neko_hash_table_iter_getk(g_chunk_data, it);
            game_chunk_t* chunk = neko_hash_table_iter_getp(g_chunk_data, it);

            // 此区块屏幕空间位置
            neko_vec2 position = neko_vec2_add(calc_chunk_position(chunk), g_chunk_render_pos);
            // 鼠标位置与此区块屏幕空间位置差值
            neko_vec2 peek_pos = neko_vec2_sub(mp, position);

            if ((peek_pos.x >= 0 && peek_pos.y >= 0) && (peek_pos.x <= CHUNK_W * g_scale && peek_pos.y <= CHUNK_H * g_scale)) {

                f32 mp_x = neko_clamp(peek_pos.x, 0.f, (f32)CHUNK_W * g_scale - 1.f) / g_scale;
                f32 mp_y = neko_clamp(peek_pos.y, 0.f, (f32)CHUNK_H * g_scale - 1.f) / g_scale;
                u32 max_idx = (CHUNK_W * CHUNK_H) - 1;
                const f32 R = brush_size * g_scale;

                // Erase in a circle pattern
                for (s32 i = -R; i < R; ++i) {
                    for (s32 j = -R; j < R; ++j) {
                        s32 rx = ((s32)mp_x + j);
                        s32 ry = ((s32)mp_y + i);
                        neko_vec2 r = neko_vec2{(f32)rx, (f32)ry};

                        if (in_bounds(rx, ry) && neko_vec2_dist(neko_v2(mp_x, mp_y), r) <= R) {
                            chunk_write_data(chunk, compute_idx(rx, ry), particle_empty());
                        }
                    }
                }
            }
        }
    }

    // Need to detect if mouse has entered the screen with payload...
}

void chunk_update_particle(game_chunk_t* chunk) {
    // Cache engine subsystem interfaces
    // neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    // neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Update frame counter (loop back to 0 if we roll past u32 max)
    bool frame_counter_even = ((g_frame_counter % 2) == 0);
    s32 ran = frame_counter_even ? 0 : 1;

    const f32 dt = neko_platform_delta_time();

    // through read data and update write buffer
    // update "bottom up", since all the data is edited "in place". Double buffering all data would fix this
    //  issue, however it requires double all of the data.
    for (u32 y = CHUNK_H - 1; y > 0; --y) {
        for (u32 x = ran ? 0 : CHUNK_W - 1; ran ? x < CHUNK_W : x > 0; ran ? ++x : --x) {
            // Current particle idx
            u32 read_idx = compute_idx(x, y);

            // Get material of particle at point
            u8 mat_id = get_particle_at(chunk, x, y).mat_id;

            // Update particle's lifetime (I guess just use frames)? Or should I have sublife?
            chunk->world_particle_data[read_idx].life_time += 1.f * dt;

            if (neko_hash_table_exists(g_mat_data, mat_id)) {
                mat_t* mat = neko_hash_table_getp(g_mat_data, mat_id);
                neko_assert(mat);
                mat->update_func(chunk, mat, x, y);
            } else {
                update_default(chunk, x, y);
            }
        }
    }

    // Can remove this loop later on by keeping update structure and setting that for the particle as it moves,
    // then at the end of frame just memsetting the entire structure to 0.
    for (u32 y = CHUNK_H - 1; y > 0; --y) {
        for (u32 x = ran ? 0 : CHUNK_W - 1; ran ? x < CHUNK_W : x > 0; ran ? ++x : --x) {
            // Set particle's update to false for next frame
            chunk->world_particle_data[compute_idx(x, y)].has_been_updated_this_frame = false;
        }
    }
}

void render_chunk_immediate(game_chunk_t* chunk) {

    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = CHUNK_W;
    t_desc.height = CHUNK_H;

    t_desc.data[0] = chunk->texture_buffer;
    neko_graphics_texture_update(chunk->tex, &t_desc);

    neko_immediate_draw_t* idraw = ((neko_client_ecs_userdata_s*)neko_ecs()->user_data)->idraw;

    const neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    neko_vec2 position = neko_vec2_add(calc_chunk_position(chunk), g_chunk_render_pos);

    neko_idraw_rect_2d_textured_ext(idraw, position.x, position.y, position.x + CHUNK_W * g_scale, position.y + CHUNK_H * g_scale, 0, 1, 1, 0, chunk->tex.id, NEKO_COLOR_WHITE);
}

void chunk_update_mesh(game_chunk_t* chunk) {

    neko_immediate_draw_t* idraw = ((neko_client_ecs_userdata_s*)neko_ecs()->user_data)->idraw;

    // std::lock_guard<std::mutex> locker(g_mutex_updatechunkmesh);

    /*
    if (chunk->rb != nullptr) b2world->DestroyBody(chunk->rb->body);
    */

    // 区块坐标转化为世界坐标
    // int chTx = chunk->x * CHUNK_W + loadZone.x;
    // int chTy = chunk->y * CHUNK_H + loadZone.y;

    // if (chTx < 0 || chTy < 0 || chTx + CHUNK_W >= width || chTy + CHUNK_H >= height) {
    //     return;
    // }

    // 优化掉无固态碰撞的区块
    bool foundAnything = true;
    for (int x = 0; x < CHUNK_W; x++) {
        for (int y = 0; y < CHUNK_H; y++) {

            // Material* mat = real_tiles[(x + chTx) + (y + chTy) * width].mat;
            // if (mat != nullptr && mat->physicsType == PhysicsType::SOLID) {
            //     foundAnything = true;
            //     goto found;
            // }

            if (get_particle_at(chunk, x, y).mat_id == mat_id_stone) {
                foundAnything = true;
                goto found;
            }

            // bool f = tiles[(x + meshZone.x) + (y + meshZone.y) * width].mat->physicsType == PhysicsType::SOLID;
            // foundAnything = foundAnything || f;
        }
    }

found:

    if (!foundAnything) {
        return;
    }

    // unsigned char* data = new unsigned char[CHUNK_W * CHUNK_H];
    // bool* edgeSeen = new bool[CHUNK_W * CHUNK_H];

    unsigned char data[CHUNK_W * CHUNK_H] = neko_default_val();
    bool edgeSeen[CHUNK_W * CHUNK_H] = neko_default_val();

    for (int y = 0; y < CHUNK_H; y++) {
        for (int x = 0; x < CHUNK_W; x++) {
            // data[x + y * CHUNK_W] = real_tiles[(x + chTx) + (y + chTy) * width].mat->physicsType == PhysicsType::SOLID;
            u8 mat_id = get_particle_at(chunk, x, y).mat_id;
            data[x + y * CHUNK_W] = (mat_id == mat_id_stone);
            edgeSeen[x + y * CHUNK_W] = false;
        }
    }

    // TPPLPoly和MarchingSquares::Result都是非聚合类 需要构造函数 使用STL来存储
    std::vector<std::vector<neko_vec2>> world_meshes = {};
    std::list<TPPLPoly> shapes;
    std::list<MarchingSquares::Result> results;

    int inn = 0;
    int lookIndex = 0;

    // TODO: 23/10/19 优化一下啦
    u32 test_count = 0;

    while (true) {
        // inn++;
        int lookX = lookIndex % CHUNK_W;
        int lookY = lookIndex / CHUNK_W;
        /*if (inn == 1) {
            lookX = CHUNK_W / 2;
            lookY = CHUNK_H / 2;
        }*/

        int edgeX = -1;
        int edgeY = -1;
        int size = CHUNK_W * CHUNK_H;

        for (int i = lookIndex; i < size; i++) {
            if (data[i] != 0) {

                int numBorders = 0;
                // if (i % CHUNK_W - 1 >= 0) numBorders += data[(i % CHUNK_W - 1) + i / CHUNK_W * CHUNK_W];
                // if (i / CHUNK_W - 1 >= 0) numBorders += data[(i % CHUNK_W)+(i / CHUNK_W - 1) * CHUNK_W];
                if (i % CHUNK_W + 1 < CHUNK_W) numBorders += data[(i % CHUNK_W + 1) + i / CHUNK_W * CHUNK_W];
                if (i / CHUNK_W + 1 < CHUNK_H) numBorders += data[(i % CHUNK_W) + (i / CHUNK_W + 1) * CHUNK_W];
                if (i / CHUNK_W + 1 < CHUNK_H && i % CHUNK_W + 1 < CHUNK_W) numBorders += data[(i % CHUNK_W + 1) + (i / CHUNK_W + 1) * CHUNK_W];

                // int val = value(i % CHUNK_W, i / CHUNK_W, CHUNK_W, height, data);
                if (numBorders != 3) {
                    edgeX = i % CHUNK_W;
                    edgeY = i / CHUNK_W;
                    break;
                }
            }
        }

        if (edgeX == -1) {
            break;
        }

        // MarchingSquares::Direction edge = MarchingSquares::FindEdge(CHUNK_W, CHUNK_H, data, lookX, lookY);

        lookX = edgeX;
        lookY = edgeY;

        lookIndex = lookX + lookY * CHUNK_W + 1;

        if (edgeSeen[lookX + lookY * CHUNK_W]) {
            inn--;
            continue;
        }

        int val = MarchingSquares::value(lookX, lookY, CHUNK_W, CHUNK_H, data);

        if (val == 0 || val == 15) {
            inn--;
            continue;
        }

        MarchingSquares::Result r = MarchingSquares::FindPerimeter(lookX, lookY, CHUNK_W, CHUNK_H, data);

        results.push_back(r);

        std::vector<neko_vec2> worldMesh;

        f32 lastX = (f32)r.initialX;
        f32 lastY = (f32)r.initialY;

        for (int i = 0; i < r.directions.size(); i++) {
            // if(r.directions[i].x != 0) r.directions[i].x = r.directions[i].x / abs(r.directions[i].x);
            // if(r.directions[i].y != 0) r.directions[i].y = r.directions[i].y / abs(r.directions[i].y);

            for (int ix = 0; ix < std::max(abs(r.directions[i].x), 1); ix++) {
                for (int iy = 0; iy < std::max(abs(r.directions[i].y), 1); iy++) {
                    int ilx = (int)(lastX + ix * (r.directions[i].x < 0 ? -1 : 1));
                    int ily = (int)(lastY - iy * (r.directions[i].y < 0 ? -1 : 1));

                    if (ilx < 0) ilx = 0;
                    if (ilx >= CHUNK_W) ilx = CHUNK_W - 1;

                    if (ily < 0) ily = 0;
                    if (ily >= CHUNK_H) ily = CHUNK_H - 1;

                    int ind = ilx + ily * CHUNK_W;
                    if (ind >= size) {
                        continue;
                    }
                    edgeSeen[ind] = true;

                    test_count++;
                }
            }

            lastX += (f32)r.directions[i].x;
            lastY -= (f32)r.directions[i].y;
            worldMesh.emplace_back(neko_v2(lastX, lastY));
        }

        // 优化
        f32 simplify_tolerance = 1.f;
        if (test_count >= 800) simplify_tolerance = 3.f;
        worldMesh = simplify(worldMesh, simplify_tolerance);

        // 优化单像素点
        if (worldMesh.size() < 3) continue;

        world_meshes.push_back(worldMesh);

        TPPLPoly poly;

        poly.Init((long)worldMesh.size());

        for (int i = 0; i < worldMesh.size(); i++) {
            poly[(int)worldMesh.size() - i - 1] = {worldMesh[i].x, worldMesh[i].y};
        }

        if (poly.GetOrientation() == TPPL_CW) {
            poly.SetHole(true);
        }

        shapes.push_back(poly);
    }

    // delete[] edgeSeen;
    // delete[] data;
    std::list<TPPLPoly> result;
    std::list<TPPLPoly> result2;

    TPPLPartition part;
    TPPLPartition part2;

    part.RemoveHoles(&shapes, &result2);

    part2.Triangulate_EC(&result2, &result);

    /*bool* solid = new bool[10 * 10];
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            solid[x + y * width] = rand() % 2 == 0;
        }
    }*/

    // Ps::MarchingSquares ms = Ps::MarchingSquares(solid, CHUNK_W, CHUNK_H);

    // Ps::MarchingSquares ms = Ps::MarchingSquares(texture);
    // worldMesh = ms.extract_simple(2);

    // chunk->polys.clear();

    neko_vec2 position = neko_vec2_add(calc_chunk_position(chunk), g_chunk_render_pos);

    std::for_each(result.begin(), result.end(), [&](TPPLPoly cur) {
        // 修正重叠点
        if (cur[0].x == cur[1].x && cur[1].x == cur[2].x) {
            cur[0].x += 0.01f;
        }
        if (cur[0].y == cur[1].y && cur[1].y == cur[2].y) {
            cur[0].y += 0.01f;
        }

        std::vector<neko_vec2> vec = {neko_v2((f32)cur[0].x, (f32)cur[0].y), neko_v2((f32)cur[1].x, (f32)cur[1].y), neko_v2((f32)cur[2].x, (f32)cur[2].y)};

        std::for_each(vec.begin(), vec.end(), [&](neko_vec2& v) {
            v.x *= g_scale;
            v.y *= g_scale;
            v.x += position.x;
            v.y += position.y;
        });

        //  worldTris.push_back(vec);
        //  b2PolygonShape sh;
        //  sh.Set(&vec[0], 3);
        //  chunk->polys.push_back(sh);

        neko_idraw_trianglev(idraw, vec[0], vec[1], vec[2], NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);
    });

    // neko_graphics_fc_text(std::to_string(test_count).c_str(), idraw->data->font_fc_default, 500, 100);

    neko_idraw_rectv(idraw, position, neko_v2(position.x + CHUNK_W * g_scale, position.y + CHUNK_H * g_scale), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);
}

void chunk_write_data(game_chunk_t* chunk, u32 idx, particle_t p) {
    // Write into particle data for mat_id value
    chunk->world_particle_data[idx] = p;
    chunk->texture_buffer[idx] = p.color;
}

void update_particle(game_chunk_t* chunk, mat_t* mat, u32 x, u32 y) {

    f32 dt = neko_platform_delta_time();

    u32 read_idx = compute_idx(x, y);
    particle_t* p = &chunk->world_particle_data[read_idx];
    u32 write_idx = read_idx;

    u64 mat_id = mat->id;

    // 固态材质
    if (mat->phy_type == mat_physics_type::SOLID) {
        update_default(chunk, x, y);
        return;
    }

    switch (mat_id) {
        case mat_id_sand: {

            // For water, same as sand, but we'll check immediate left and right as well

            u32 fall_rate = 4;

            p->velocity.y = neko_clamp(p->velocity.y + (world_gravity * dt), -10.f, 10.f);

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1) && get_particle_at(chunk, x, y + 1).mat_id != mat_id_water) {
                p->velocity.y /= 2.f;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            // Check to see if you can swap first with other element below you
            u32 b_idx = compute_idx(x, y + 1);
            u32 br_idx = compute_idx(x + 1, y + 1);
            u32 bl_idx = compute_idx(x - 1, y + 1);

            s32 lx, ly;

            particle_t tmp_a = chunk->world_particle_data[read_idx];

            // Physics (using velocity)
            if (in_bounds(vi_x, vi_y) &&                                                                 //
                ((is_empty(chunk, vi_x, vi_y) ||                                                         //
                  (((chunk->world_particle_data[compute_idx(vi_x, vi_y)].mat_id == mat_id_water) &&      //
                    !chunk->world_particle_data[compute_idx(vi_x, vi_y)].has_been_updated_this_frame &&  //
                    neko_vec2_len(chunk->world_particle_data[compute_idx(vi_x, vi_y)].velocity) - neko_vec2_len(tmp_a.velocity) > 10.f))))) {

                particle_t tmp_b = chunk->world_particle_data[compute_idx(vi_x, vi_y)];

                // Try to throw water out
                if (tmp_b.mat_id == mat_id_water) {

                    s32 rx = random_val(-2, 2);
                    tmp_b.velocity = neko_vec2{(f32)rx, -4.f};

                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), tmp_a);

                    for (s32 i = -10; i < 0; ++i) {
                        for (s32 j = -10; j < 10; ++j) {
                            if (is_empty(chunk, vi_x + j, vi_y + i)) {
                                chunk_write_data(chunk, compute_idx(vi_x + j, vi_y + i), tmp_b);
                                break;
                            }
                        }
                    }

                    // Couldn't write there, so, uh, destroy it.
                    chunk_write_data(chunk, read_idx, particle_empty());
                } else if (is_empty(chunk, vi_x, vi_y)) {
                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), tmp_a);
                    chunk_write_data(chunk, read_idx, tmp_b);
                }
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y + 1) && ((is_empty(chunk, x, y + 1) || (chunk->world_particle_data[b_idx].mat_id == mat_id_water)))) {
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y + 1);
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y + 1) && ((is_empty(chunk, x - 1, y + 1) || chunk->world_particle_data[bl_idx].mat_id == mat_id_water))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x - 1, y + 1);
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y + 1) && ((is_empty(chunk, x + 1, y + 1) || chunk->world_particle_data[br_idx].mat_id == mat_id_water))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + 1, y + 1);
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (is_in_liquid(chunk, x, y, &lx, &ly) && random_val(0, 10) == 0) {
                particle_t tmp_b = get_particle_at(chunk, lx, ly);
                chunk_write_data(chunk, compute_idx(lx, ly), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
        } break;
        case mat_id_salt: {

            u32 fall_rate = 2;
            u32 spread_rate = 5;
            s32 lx, ly;

            p->velocity.y = neko_clamp(p->velocity.y + (world_gravity * dt), -10.f, 10.f);

            p->has_been_updated_this_frame = true;

            // If in liquid, chance to dissolve itself.
            if (is_in_liquid(chunk, x, y, &lx, &ly)) {
                if (random_val(0, 1000) == 0) {
                    chunk_write_data(chunk, read_idx, particle_empty());
                    return;
                }
            }

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            // if (in_bounds(x, y + 1) && !is_empty(chunk,x, y + 1) && get_particle_at(chunk,x, y + 1).mat_id != mat_id_water) {
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1)) {
                p->velocity.y /= 2.f;
            }

            s32 r = 1;
            s32 l = -r;
            s32 u = fall_rate;
            s32 v_idx = compute_idx(x + (s32)p->velocity.x, y + (s32)p->velocity.y);
            s32 b_idx = compute_idx(x, y + u);
            s32 bl_idx = compute_idx(x + l, y + u);
            s32 br_idx = compute_idx(x + r, y + u);
            s32 l_idx = compute_idx(x + l, y);
            s32 r_idx = compute_idx(x + r, y);
            s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

            if (in_bounds(x + vx, y + vy) && (is_empty(chunk, x + vx, y + vy))) {
                chunk_write_data(chunk, v_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_in_liquid(chunk, x, y, &lx, &ly) && random_val(0, 10) == 0) {
                particle_t tmp_b = get_particle_at(chunk, lx, ly);
                chunk_write_data(chunk, compute_idx(lx, ly), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y + 1) && ((is_empty(chunk, x, y + 1)))) {
                u32 idx = compute_idx(x, y + 1);
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_a = chunk->world_particle_data[read_idx];
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, tmp_a);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y + 1) && (is_empty(chunk, x - 1, y + 1))) {
                u32 idx = compute_idx(x - 1, y + 1);
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_a = chunk->world_particle_data[read_idx];
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, tmp_a);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y + 1) && (is_empty(chunk, x + 1, y + 1))) {
                u32 idx = compute_idx(x + 1, y + 1);
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_a = chunk->world_particle_data[read_idx];
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, tmp_a);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
        } break;
        case mat_id_gunpowder: {

            // For water, same as sand, but we'll check immediate left and right as well
            u32 fall_rate = 4;

            p->velocity.y = neko_clamp(p->velocity.y + (world_gravity * dt), -10.f, 10.f);
            // p->velocity.x = neko_clamp(p->velocity.x, -5.f, 5.f);

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1) && get_particle_at(chunk, x, y + 1).mat_id != mat_id_water) {
                p->velocity.y /= 2.f;
                // p->velocity.x /= 1.2f;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            // Check to see if you can swap first with other element below you
            u32 b_idx = compute_idx(x, y + 1);
            u32 br_idx = compute_idx(x + 1, y + 1);
            u32 bl_idx = compute_idx(x - 1, y + 1);

            s32 lx, ly;

            particle_t tmp_a = chunk->world_particle_data[read_idx];

            // Physics (using velocity)
            if (in_bounds(vi_x, vi_y) && ((is_empty(chunk, vi_x, vi_y) || (((chunk->world_particle_data[compute_idx(vi_x, vi_y)].mat_id == mat_id_water) &&
                                                                            !chunk->world_particle_data[compute_idx(vi_x, vi_y)].has_been_updated_this_frame &&
                                                                            neko_vec2_len(chunk->world_particle_data[compute_idx(vi_x, vi_y)].velocity) - neko_vec2_len(tmp_a.velocity) > 10.f))))) {

                particle_t tmp_b = chunk->world_particle_data[compute_idx(vi_x, vi_y)];

                // Try to throw water out
                if (tmp_b.mat_id == mat_id_water) {

                    s32 rx = random_val(-2, 2);
                    tmp_b.velocity = neko_vec2{(f32)rx, -4.f};

                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), tmp_a);

                    for (s32 i = -10; i < 0; ++i) {
                        for (s32 j = -10; j < 10; ++j) {
                            if (is_empty(chunk, vi_x + j, vi_y + i)) {
                                chunk_write_data(chunk, compute_idx(vi_x + j, vi_y + i), tmp_b);
                                break;
                            }
                        }
                    }

                    // Couldn't write there, so, uh, destroy it.
                    chunk_write_data(chunk, read_idx, particle_empty());
                } else if (is_empty(chunk, vi_x, vi_y)) {
                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), tmp_a);
                    chunk_write_data(chunk, read_idx, tmp_b);
                }
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y + 1) && ((is_empty(chunk, x, y + 1) || (chunk->world_particle_data[b_idx].mat_id == mat_id_water)))) {
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y + 1);
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y + 1) && ((is_empty(chunk, x - 1, y + 1) || chunk->world_particle_data[bl_idx].mat_id == mat_id_water))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x - 1, y + 1);
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y + 1) && ((is_empty(chunk, x + 1, y + 1) || chunk->world_particle_data[br_idx].mat_id == mat_id_water))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + 1, y + 1);
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (is_in_liquid(chunk, x, y, &lx, &ly) && random_val(0, 10) == 0) {
                particle_t tmp_b = get_particle_at(chunk, lx, ly);
                chunk_write_data(chunk, compute_idx(lx, ly), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
        } break;
        case mat_id_steam: {

            // For water, same as sand, but we'll check immediate left and right as well
            u32 fall_rate = 4;

            if (p->life_time > 10.f) {
                chunk_write_data(chunk, read_idx, particle_empty());
                return;
            }

            if (p->has_been_updated_this_frame) {
                return;
            }

            p->has_been_updated_this_frame = true;

            // Smoke rises over time. This might cause issues, actually...
            p->velocity.y = neko_clamp(p->velocity.y - (world_gravity * dt), -2.f, 10.f);
            p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 100.f, -1.f, 1.f);

            // Change color based on life_time
            p->color.r = (u8)(neko_clamp((neko_interp_linear(0.f, 10.f, p->life_time) / 10.f) * 255.f, 150.f, 255.f));
            p->color.g = (u8)(neko_clamp((neko_interp_linear(0.f, 10.f, p->life_time) / 10.f) * 255.f, 150.f, 255.f));
            p->color.b = (u8)(neko_clamp((neko_interp_linear(0.f, 10.f, p->life_time) / 10.f) * 255.f, 150.f, 255.f));
            p->color.a = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time) / 10.f) * 255.f, 10.f, 255.f));

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y - 1) && !is_empty(chunk, x, y - 1) && get_particle_at(chunk, x, y - 1).mat_id != mat_id_water) {
                p->velocity.y /= 2.f;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            if (in_bounds(vi_x, vi_y) && ((is_empty(chunk, vi_x, vi_y) || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_water || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_fire))) {

                particle_t tmp_b = chunk->world_particle_data[compute_idx(vi_x, vi_y)];

                // Try to throw water out
                if (tmp_b.mat_id == mat_id_water) {

                    tmp_b.has_been_updated_this_frame = true;

                    s32 rx = random_val(-2, 2);
                    tmp_b.velocity = neko_vec2{(f32)rx, -3.f};

                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                    chunk_write_data(chunk, read_idx, tmp_b);
                } else if (is_empty(chunk, vi_x, vi_y)) {
                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                    chunk_write_data(chunk, read_idx, tmp_b);
                }
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y - 1) && ((is_empty(chunk, x, y - 1) || (get_particle_at(chunk, x, y - 1).mat_id == mat_id_water) || get_particle_at(chunk, x, y - 1).mat_id == mat_id_fire))) {
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y - 1);
                chunk_write_data(chunk, compute_idx(x, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y - 1) &&
                       ((is_empty(chunk, x - 1, y - 1) || get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_water) || get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_fire)) {
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x - 1, y - 1);
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y - 1) &&
                       ((is_empty(chunk, x + 1, y - 1) || get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_water) || get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_fire)) {
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + 1, y - 1);
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
            // Can move if in liquid
            else if (in_bounds(x + 1, y) && (get_particle_at(chunk, x + 1, y).mat_id == mat_id_water)) {
                u32 idx = compute_idx(x + 1, y);
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y) && (chunk->world_particle_data[compute_idx(x - 1, y)].mat_id == mat_id_water)) {
                u32 idx = compute_idx(x - 1, y);
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else {
                chunk_write_data(chunk, read_idx, *p);
            }
        } break;
        case mat_id_smoke: {

            // For water, same as sand, but we'll check immediate left and right as well
            u32 fall_rate = 4;

            if (p->life_time > 10.f) {
                chunk_write_data(chunk, read_idx, particle_empty());
                return;
            }

            if (p->has_been_updated_this_frame) {
                return;
            }

            p->has_been_updated_this_frame = true;

            // Smoke rises over time. This might cause issues, actually...
            p->velocity.y = neko_clamp(p->velocity.y - (world_gravity * dt), -2.f, 10.f);
            p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 100.f, -1.f, 1.f);

            // Change color based on life_time
            p->color.r = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time * 0.5f) / 10.f) * 150.f, 0.f, 150.f));
            p->color.g = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time * 0.5f) / 10.f) * 120.f, 0.f, 120.f));
            p->color.b = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time * 0.5f) / 10.f) * 100.f, 0.f, 100.f));

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y - 1) && !is_empty(chunk, x, y - 1) && get_particle_at(chunk, x, y - 1).mat_id != mat_id_water) {
                p->velocity.y /= 2.f;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            // if (in_bounds(vi_x, vi_y) && ((is_empty(chunk,vi_x, vi_y) || get_particle_at(chunk,vi_x, vi_y).mat_id == mat_id_water || get_particle_at(chunk,vi_x, vi_y).mat_id == mat_id_fire))) {
            if (in_bounds(vi_x, vi_y) && get_particle_at(chunk, vi_x, vi_y).mat_id != mat_id_smoke) {

                particle_t tmp_b = chunk->world_particle_data[compute_idx(vi_x, vi_y)];

                // Try to throw water out
                if (tmp_b.mat_id == mat_id_water) {

                    tmp_b.has_been_updated_this_frame = true;

                    s32 rx = random_val(-2, 2);
                    tmp_b.velocity = neko_vec2{(f32)rx, -3.f};

                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                    chunk_write_data(chunk, read_idx, tmp_b);
                } else if (is_empty(chunk, vi_x, vi_y)) {
                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                    chunk_write_data(chunk, read_idx, tmp_b);
                }
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y - 1) && get_particle_at(chunk, x, y - 1).mat_id != mat_id_smoke && get_particle_at(chunk, x, y - 1).mat_id != mat_id_wood &&
                     get_particle_at(chunk, x, y - 1).mat_id != mat_id_stone) {
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y - 1);
                chunk_write_data(chunk, compute_idx(x, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y - 1) && get_particle_at(chunk, x - 1, y - 1).mat_id != mat_id_smoke && get_particle_at(chunk, x - 1, y - 1).mat_id != mat_id_wood &&
                       get_particle_at(chunk, x - 1, y - 1).mat_id != mat_id_stone) {
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x - 1, y - 1);
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y - 1) && get_particle_at(chunk, x + 1, y - 1).mat_id != mat_id_smoke && get_particle_at(chunk, x + 1, y - 1).mat_id != mat_id_wood &&
                       get_particle_at(chunk, x + 1, y - 1).mat_id != mat_id_stone) {
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + 1, y - 1);
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
            // Can move if in liquid
            else if (in_bounds(x + 1, y) && get_particle_at(chunk, x + 1, y).mat_id != mat_id_smoke && get_particle_at(chunk, x + 1, y).mat_id != mat_id_wood &&
                     get_particle_at(chunk, x + 1, y).mat_id != mat_id_stone) {
                u32 idx = compute_idx(x + 1, y);
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y) && get_particle_at(chunk, x - 1, y).mat_id != mat_id_smoke && get_particle_at(chunk, x - 1, y).mat_id != mat_id_wood &&
                       get_particle_at(chunk, x - 1, y).mat_id != mat_id_stone) {
                u32 idx = compute_idx(x - 1, y);
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else {
                chunk_write_data(chunk, read_idx, *p);
            }
        } break;
        case mat_id_ember: {

            // For water, same as sand, but we'll check immediate left and right as well
            u32 fall_rate = 4;

            if (p->life_time > 0.5f) {
                chunk_write_data(chunk, read_idx, particle_empty());
                return;
            }

            if (p->has_been_updated_this_frame) {
                return;
            }

            p->has_been_updated_this_frame = true;

            p->velocity.y = neko_clamp(p->velocity.y - (world_gravity * dt), -2.f, 10.f);
            p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 100.f, -1.f, 1.f);

            // If directly on top of some wall, then replace it
            if (in_bounds(x, y + 1) && get_particle_at(chunk, x, y + 1).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x, y + 1), particle_fire());
            } else if (in_bounds(x + 1, y + 1) && get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x + 1, y + 1), particle_fire());
            } else if (in_bounds(x - 1, y + 1) && get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x - 1, y + 1), particle_fire());
            } else if (in_bounds(x - 1, y) && get_particle_at(chunk, x - 1, y).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x - 1, y), particle_fire());
            } else if (in_bounds(x + 1, y) && get_particle_at(chunk, x + 1, y).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x + 1, y), particle_fire());
            } else if (in_bounds(x + 1, y - 1) && get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), particle_fire());
            } else if (in_bounds(x - 1, y - 1) && get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_wood && random_val(0, 200) == 0) {
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), particle_fire());
            }

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y - 1) && !is_empty(chunk, x, y - 1) && get_particle_at(chunk, x, y - 1).mat_id != mat_id_water) {
                p->velocity.y /= 2.f;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            if (in_bounds(vi_x, vi_y) && (is_empty(chunk, vi_x, vi_y) || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_water || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_fire ||
                                          get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_smoke || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_ember)) {

                particle_t tmp_b = chunk->world_particle_data[compute_idx(vi_x, vi_y)];

                // Try to throw water out
                if (tmp_b.mat_id == mat_id_water) {

                    tmp_b.has_been_updated_this_frame = true;

                    s32 rx = random_val(-2, 2);
                    tmp_b.velocity = neko_vec2{(f32)rx, -3.f};

                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                    chunk_write_data(chunk, read_idx, tmp_b);
                } else if (is_empty(chunk, vi_x, vi_y)) {
                    chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                    chunk_write_data(chunk, read_idx, tmp_b);
                }
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y - 1) && ((is_empty(chunk, x, y - 1) || (get_particle_at(chunk, x, y - 1).mat_id == mat_id_water) || get_particle_at(chunk, x, y - 1).mat_id == mat_id_fire))) {
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y - 1);
                chunk_write_data(chunk, compute_idx(x, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y - 1) &&
                       ((is_empty(chunk, x - 1, y - 1) || get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_water) || get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_fire)) {
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x - 1, y - 1);
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y - 1) &&
                       ((is_empty(chunk, x + 1, y - 1) || get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_water) || get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_fire)) {
                p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
                p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + 1, y - 1);
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }
            // Can move if in liquid
            else if (in_bounds(x + 1, y) && (is_empty(chunk, x + 1, y) || get_particle_at(chunk, x + 1, y).mat_id == mat_id_fire)) {
                u32 idx = compute_idx(x + 1, y);
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y) && (is_empty(chunk, x - 1, y) || get_particle_at(chunk, x - 1, y).mat_id == mat_id_fire)) {
                u32 idx = compute_idx(x - 1, y);
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else {
                chunk_write_data(chunk, read_idx, *p);
            }
        } break;
        case mat_id_fire: {

            // For water, same as sand, but we'll check immediate left and right as well
            u32 fall_rate = 4;

            if (p->has_been_updated_this_frame) {
                return;
            }

            p->has_been_updated_this_frame = true;

            if (p->life_time > 0.2f) {
                if (random_val(0, 100) == 0) {
                    chunk_write_data(chunk, read_idx, particle_empty());
                    return;
                }
            }

            f32 st = sin(neko_platform_elapsed_time());
            // f32 grav_mul = random_val(0, 10) == 0 ? 2.f : 1.f;
            p->velocity.y = neko_clamp(p->velocity.y - ((world_gravity * dt)) * 0.2f, -5.0f, 0.f);
            // p->velocity.x = neko_clamp(st, -1.f, 1.f);
            p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 200.f, -0.5f, 0.5f);

            // Change color based on life_time

            if (random_val(0, (s32)(p->life_time * 100.f)) % 200 == 0) {
                s32 ran = random_val(0, 3);
                switch (ran) {
                    case 0:
                        p->color = cell_color_t{255, 80, 20, 255};
                        break;
                    case 1:
                        p->color = cell_color_t{250, 150, 10, 255};
                        break;
                    case 2:
                        p->color = cell_color_t{200, 150, 0, 255};
                        break;
                    case 3:
                        p->color = cell_color_t{100, 50, 2, 255};
                        break;
                }
            }

            if (p->life_time < 0.02f) {
                p->color.r = 200;
            } else {
                p->color.r = 255;
            }

            // In water, so create steam and DIE
            // Should also kill the water...
            s32 lx, ly;
            if (is_in_water(chunk, x, y, &lx, &ly)) {
                if (random_val(0, 1) == 0) {
                    s32 ry = random_val(-5, -1);
                    s32 rx = random_val(-5, 5);
                    for (s32 i = ry; i > -5; --i) {
                        for (s32 j = rx; j < 5; ++j) {
                            particle_t p = particle_steam();
                            if (in_bounds(x + j, y + i) && is_empty(chunk, x + j, y + i)) {
                                particle_t p = particle_steam();
                                chunk_write_data(chunk, compute_idx(x + j, y + i), p);
                            }
                        }
                    }
                    particle_t p = particle_steam();
                    chunk_write_data(chunk, read_idx, particle_empty());
                    chunk_write_data(chunk, read_idx, p);
                    chunk_write_data(chunk, compute_idx(lx, ly), particle_empty());
                    return;
                }
            }

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1) && (get_particle_at(chunk, x, y + 1).mat_id != mat_id_water || get_particle_at(chunk, x, y + 1).mat_id != mat_id_smoke)) {
                p->velocity.y /= 2.f;
            }

            if (random_val(0, 10) == 0) {
                // p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-1, 1) / 2.f, -1.f, 1.f);
            }
            // p->velocity.x = neko_clamp(p->velocity.x, -0.5f, 0.5f);

            // Kill fire underneath
            if (in_bounds(x, y + 3) && get_particle_at(chunk, x, y + 3).mat_id == mat_id_fire && random_val(0, 100) == 0) {
                chunk_write_data(chunk, compute_idx(x, y + 3), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
                return;
            }

            // Chance to kick itself up (to simulate flames)
            if (in_bounds(x, y + 1) && get_particle_at(chunk, x, y + 1).mat_id == mat_id_fire && in_bounds(x, y - 1) && get_particle_at(chunk, x, y - 1).mat_id == mat_id_empty) {
                if (random_val(0, 10) == 0 * p->life_time < 10.f && p->life_time > 1.f) {
                    s32 r = random_val(0, 1);
                    s32 rh = random_val(-10, -1);
                    s32 spread = 3;
                    for (s32 i = rh; i < 0; ++i) {
                        for (s32 j = r ? -spread : spread; r ? j < spread : j > -spread; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
                return;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            // Check to see if you can swap first with other element below you
            u32 b_idx = compute_idx(x, y + 1);
            u32 br_idx = compute_idx(x + 1, y + 1);
            u32 bl_idx = compute_idx(x - 1, y + 1);

            const s32 wood_chance = 100;
            const s32 gun_powder_chance = 1;
            const s32 oil_chance = 5;

            // Chance to spawn smoke above
            for (u32 i = 0; i < random_val(1, 10); ++i) {
                if (random_val(0, 500) == 0) {
                    if (in_bounds(x, y - 1) && is_empty(chunk, x, y - 1)) {
                        chunk_write_data(chunk, compute_idx(x, y - 1), particle_smoke());
                    } else if (in_bounds(x + 1, y - 1) && is_empty(chunk, x + 1, y - 1)) {
                        chunk_write_data(chunk, compute_idx(x + 1, y - 1), particle_smoke());
                    } else if (in_bounds(x - 1, y - 1) && is_empty(chunk, x - 1, y - 1)) {
                        chunk_write_data(chunk, compute_idx(x - 1, y - 1), particle_smoke());
                    }
                }
            }

            // Spawn embers
            if (random_val(0, 250) == 0 && p->life_time < 3.f) {
                for (u32 i = 0; i < random_val(1, 100); ++i) {
                    if (in_bounds(x, y - 1) && is_empty(chunk, x, y - 1)) {
                        particle_t e = particle_ember();
                        e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f};
                        chunk_write_data(chunk, compute_idx(x, y - 1), e);
                    } else if (in_bounds(x + 1, y - 1) && is_empty(chunk, x + 1, y - 1)) {
                        particle_t e = particle_ember();
                        e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f};
                        chunk_write_data(chunk, compute_idx(x + 1, y - 1), e);
                    } else if (in_bounds(x - 1, y - 1) && is_empty(chunk, x - 1, y - 1)) {
                        particle_t e = particle_ember();
                        e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f};
                        chunk_write_data(chunk, compute_idx(x - 1, y - 1), e);
                    }
                }
            }

            // If directly on top of some wall, then replace it
            if (in_bounds(x, y + 1) && ((get_particle_at(chunk, x, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                )) {
                chunk_write_data(chunk, compute_idx(x, y + 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x + 1, y + 1) && ((get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x + 1, y + 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x - 1, y + 1) && ((get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x - 1, y + 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x - 1, y) && ((get_particle_at(chunk, x - 1, y).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                       )) {
                chunk_write_data(chunk, compute_idx(x - 1, y), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x + 1, y) && ((get_particle_at(chunk, x + 1, y).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                       )) {
                chunk_write_data(chunk, compute_idx(x + 1, y), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x + 1, y - 1) && ((get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x - 1, y - 1) && ((get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                // particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), *p);
                                chunk_write_data(chunk, read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x, y - 1) && is_empty(chunk, x, y - 1)) {
                if (random_val(0, 50) == 0) {
                    chunk_write_data(chunk, read_idx, particle_empty());
                    return;
                }
            }

            if (in_bounds(vi_x, vi_y) && (is_empty(chunk, vi_x, vi_y) || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_fire || get_particle_at(chunk, vi_x, vi_y).mat_id == mat_id_smoke)) {
                // p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = chunk->world_particle_data[compute_idx(vi_x, vi_y)];
                chunk_write_data(chunk, compute_idx(vi_x, vi_y), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            }

            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y + 1) && ((is_empty(chunk, x, y + 1) || (chunk->world_particle_data[b_idx].mat_id == mat_id_water)))) {
                // p->velocity.y -= (world_gravity * dt);
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                particle_t tmp_b = chunk->world_particle_data[b_idx];
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y + 1) && ((is_empty(chunk, x - 1, y + 1) || chunk->world_particle_data[bl_idx].mat_id == mat_id_water))) {
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                // p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = chunk->world_particle_data[bl_idx];
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y + 1) && ((is_empty(chunk, x + 1, y + 1) || chunk->world_particle_data[br_idx].mat_id == mat_id_water))) {
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                // p->velocity.y -= (world_gravity * dt);
                particle_t tmp_b = chunk->world_particle_data[br_idx];
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x - 1, y - 1) && (chunk->world_particle_data[compute_idx(x - 1, y - 1)].mat_id == mat_id_water)) {
                u32 idx = compute_idx(x - 1, y - 1);
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + 1, y - 1) && (chunk->world_particle_data[compute_idx(x + 1, y - 1)].mat_id == mat_id_water)) {
                u32 idx = compute_idx(x + 1, y - 1);
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x, y - 1) && (chunk->world_particle_data[compute_idx(x, y - 1)].mat_id == mat_id_water)) {
                u32 idx = compute_idx(x, y - 1);
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                particle_t tmp_b = chunk->world_particle_data[idx];
                chunk_write_data(chunk, idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else {
                // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
                chunk_write_data(chunk, read_idx, *p);
            }
        } break;
        case mat_id_lava: {

            // For water, same as sand, but we'll check immediate left and right as well
            u32 fall_rate = 4;

            if (p->has_been_updated_this_frame) {
                return;
            }

            p->has_been_updated_this_frame = true;

            p->velocity.y = neko_clamp(p->velocity.y + ((world_gravity * dt)), -10.f, 10.f);

            // Change color based on life_time
            if (random_val(0, (s32)(p->life_time * 100.f)) % 200 == 0) {
                s32 ran = random_val(0, 3);
                switch (ran) {
                    case 0:
                        p->color = cell_color_t{255, 80, 20, 255};
                        break;
                    case 1:
                        p->color = cell_color_t{255, 100, 10, 255};
                        break;
                    case 2:
                        p->color = cell_color_t{255, 50, 0, 255};
                        break;
                    case 3:
                        p->color = cell_color_t{200, 50, 2, 255};
                        break;
                }
            }

            // In water, so create steam and DIE
            // Should also kill the water...
            s32 lx, ly;
            if (is_in_water(chunk, x, y, &lx, &ly)) {
                if (random_val(0, 1) == 0) {
                    s32 ry = random_val(-5, -1);
                    s32 rx = random_val(-5, 5);
                    for (s32 i = ry; i > -5; --i) {
                        for (s32 j = rx; j < 5; ++j) {
                            particle_t p = particle_steam();
                            if (in_bounds(x + j, y + i) && is_empty(chunk, x + j, y + i)) {
                                particle_t p = particle_steam();
                                chunk_write_data(chunk, compute_idx(x + j, y + i), p);
                            }
                        }
                    }
                    particle_t p = particle_steam();
                    chunk_write_data(chunk, read_idx, particle_empty());
                    chunk_write_data(chunk, read_idx, p);
                    chunk_write_data(chunk, compute_idx(lx, ly), particle_stone());
                    return;
                }
            }

            // Otherwise destroy anything that isn't fire or lava...eventually...

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1) && (get_particle_at(chunk, x, y + 1).mat_id != mat_id_water || get_particle_at(chunk, x, y + 1).mat_id != mat_id_smoke)) {
                p->velocity.y /= 2.f;
            }

            s32 vi_x = x + (s32)p->velocity.x;
            s32 vi_y = y + (s32)p->velocity.y;

            const s32 spread_rate = 1;
            s32 ran = random_val(0, 1);
            s32 r = spread_rate;
            s32 l = -r;
            s32 u = fall_rate;
            s32 v_idx = compute_idx(x + (s32)p->velocity.x, y + (s32)p->velocity.y);
            s32 b_idx = compute_idx(x, y + u);
            s32 bl_idx = compute_idx(x + l, y + u);
            s32 br_idx = compute_idx(x + r, y + u);
            s32 l_idx = compute_idx(x + l, y);
            s32 r_idx = compute_idx(x + r, y);
            s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

            const s32 wood_chance = 200;
            const s32 gun_powder_chance = 0;
            const s32 oil_chance = 5;

            // Chance to spawn smoke above
            for (u32 i = 0; i < random_val(1, 10); ++i) {
                if (random_val(0, 500) == 0) {
                    if (in_bounds(x, y - 1) && is_empty(chunk, x, y - 1)) {
                        chunk_write_data(chunk, compute_idx(x, y - 1), particle_smoke());
                    } else if (in_bounds(x + 1, y - 1) && is_empty(chunk, x + 1, y - 1)) {
                        chunk_write_data(chunk, compute_idx(x + 1, y - 1), particle_smoke());
                    } else if (in_bounds(x - 1, y - 1) && is_empty(chunk, x - 1, y - 1)) {
                        chunk_write_data(chunk, compute_idx(x - 1, y - 1), particle_smoke());
                    }
                }
            }

            // Spawn embers
            if (random_val(0, 250) == 0 && p->life_time < 3.f) {
                for (u32 i = 0; i < random_val(1, 100); ++i) {
                    if (in_bounds(x, y - 1) && is_empty(chunk, x, y - 1)) {
                        particle_t e = particle_ember();
                        e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f};
                        chunk_write_data(chunk, compute_idx(x, y - 1), e);
                    } else if (in_bounds(x + 1, y - 1) && is_empty(chunk, x + 1, y - 1)) {
                        particle_t e = particle_ember();
                        e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f};
                        chunk_write_data(chunk, compute_idx(x + 1, y - 1), e);
                    } else if (in_bounds(x - 1, y - 1) && is_empty(chunk, x - 1, y - 1)) {
                        particle_t e = particle_ember();
                        e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f};
                        chunk_write_data(chunk, compute_idx(x - 1, y - 1), e);
                    }
                }
            }

            // If directly on top of some wall, then replace it
            if (in_bounds(x, y + 1) && ((get_particle_at(chunk, x, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                )) {
                chunk_write_data(chunk, compute_idx(x, y + 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                // chunk_write_data(chunk,read_idx, particle_empty());
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x + 1, y + 1) && ((get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x + 1, y + 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x - 1, y + 1) && ((get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x - 1, y + 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x - 1, y) && ((get_particle_at(chunk, x - 1, y).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                       )) {
                chunk_write_data(chunk, compute_idx(x - 1, y), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x + 1, y) && ((get_particle_at(chunk, x + 1, y).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                       )) {
                chunk_write_data(chunk, compute_idx(x + 1, y), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x + 1, y - 1) && ((get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                break;
                            }
                        }
                    }
                }
            } else if (in_bounds(x - 1, y - 1) && ((get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_oil && random_val(0, oil_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), particle_fire());
                if (random_val(0, 5) == 0) {
                    s32 r = random_val(0, 1);
                    for (s32 i = -3; i < 2; ++i) {
                        for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                            s32 rx = j, ry = i;
                            if (in_bounds(x + rx, y + ry) && is_empty(chunk, x + rx, y + ry)) {
                                particle_t fp = particle_fire();
                                p->life_time += 0.1f;
                                chunk_write_data(chunk, compute_idx(x + rx, y + ry), fp);
                                break;
                            }
                        }
                    }
                }
            }

            // If in water, then need to f32 upwards
            // s32 lx, ly;
            // if (is_in_liquid(chunk,x, y, &lx, &ly) && in_bounds(x, y - 1) && get_particle_at(chunk,x, y - 1).mat_id == mat_id_water) {
            //  particle_t tmp = get_particle_at(chunk,x, y - 1);
            //  chunk_write_data(chunk,compute_idx(x, y - 1), *p);
            //  chunk_write_data(chunk,read_idx, tmp);
            //  // return;
            // }

            if (in_bounds(x + vx, y + vy) && (is_empty(chunk, x + vx, y + vy))) {
                chunk_write_data(chunk, v_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x, y + u)) {
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + r, y + u)) {
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + l, y + u)) {
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else {
                particle_t tmp = *p;
                bool found = false;

                for (u32 i = 0; i < fall_rate; ++i) {
                    for (s32 j = spread_rate; j > 0; --j) {
                        if (is_empty(chunk, x - j, y + i)) {
                            chunk_write_data(chunk, compute_idx(x - j, y + i), *p);
                            chunk_write_data(chunk, read_idx, particle_empty());
                            found = true;
                            break;
                        } else if (is_empty(chunk, x + j, y + i)) {
                            chunk_write_data(chunk, compute_idx(x + j, y + i), *p);
                            chunk_write_data(chunk, read_idx, particle_empty());
                            found = true;
                            break;
                        }
                    }
                }

                if (!found) {
                    chunk_write_data(chunk, read_idx, tmp);
                }
            }
        } break;
        case mat_id_oil: {

            u32 fall_rate = 2;
            s32 spread_rate = 4;

            p->velocity.y = neko_clamp(p->velocity.y + (world_gravity * dt), -10.f, 10.f);

            p->has_been_updated_this_frame = true;

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            // if (in_bounds(x, y + 1) && !is_empty(chunk,x, y + 1) && get_particle_at(chunk,x, y + 1).mat_id != mat_id_water) {
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1)) {
                p->velocity.y /= 2.f;
            }

            // Change color depending on pressure? Pressure would dictate how "deep" the water is, I suppose.
            if (random_val(0, (s32)(p->life_time * 100.f)) % 20 == 0) {
                f32 r = (f32)(random_val(0, 1)) / 2.f;
                p->color.r = (u8)(neko_interp_linear(0.2f, 0.25f, r) * 255.f);
                p->color.g = (u8)(neko_interp_linear(0.2f, 0.25f, r) * 255.f);
                p->color.b = (u8)(neko_interp_linear(0.2f, 0.25f, r) * 255.f);
            }

            s32 ran = random_val(0, 1);
            s32 r = ran ? spread_rate : -spread_rate;
            s32 l = -r;
            s32 u = fall_rate;
            s32 v_idx = compute_idx(x + (s32)p->velocity.x, y + (s32)p->velocity.y);
            s32 b_idx = compute_idx(x, y + u);
            s32 bl_idx = compute_idx(x + l, y + u);
            s32 br_idx = compute_idx(x + r, y + u);
            s32 l_idx = compute_idx(x + l, y);
            s32 r_idx = compute_idx(x + r, y);
            s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

            // If in water, then need to f32 upwards
            // s32 lx, ly;
            // if (is_in_liquid(chunk,x, y, &lx, &ly) && in_bounds(x, y - 1) && get_particle_at(chunk,x, y - 1).mat_id == mat_id_water) {
            //  particle_t tmp = get_particle_at(chunk,x, y - 1);
            //  chunk_write_data(chunk,compute_idx(x, y - 1), *p);
            //  chunk_write_data(chunk,read_idx, tmp);
            //  // return;
            // }

            if (in_bounds(x + vx, y + vy) && (is_empty(chunk, x + vx, y + vy))) {
                chunk_write_data(chunk, v_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x, y + u)) {
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + r, y + u)) {
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + l, y + u)) {
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else {
                particle_t tmp = *p;
                bool found = false;

                for (u32 i = 0; i < fall_rate; ++i) {
                    for (s32 j = spread_rate; j > 0; --j) {
                        if (is_empty(chunk, x - j, y + i)) {
                            chunk_write_data(chunk, compute_idx(x - j, y + i), *p);
                            chunk_write_data(chunk, read_idx, particle_empty());
                            found = true;
                            break;
                        } else if (is_empty(chunk, x + j, y + i)) {
                            chunk_write_data(chunk, compute_idx(x + j, y + i), *p);
                            chunk_write_data(chunk, read_idx, particle_empty());
                            found = true;
                            break;
                        }
                    }
                }

                if (!found) {
                    chunk_write_data(chunk, read_idx, tmp);
                }
            }
        } break;
        case mat_id_acid: {

            u32 fall_rate = 2;
            s32 spread_rate = 5;
            s32 lx, ly;

            p->velocity.y = neko_clamp(p->velocity.y + (world_gravity * dt), -10.f, 10.f);

            p->has_been_updated_this_frame = true;

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            // if (in_bounds(x, y + 1) && !is_empty(chunk,x, y + 1) && get_particle_at(chunk,x, y + 1).mat_id != mat_id_water) {
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1)) {
                p->velocity.y /= 2.f;
            }

            // Change color depending on pressure? Pressure would dictate how "deep" the water is, I suppose.
            if (random_val(0, (s32)(p->life_time * 100.f)) % 20 == 0) {
                f32 r = (f32)(random_val(0, 1)) / 2.f;
                p->color.r = (u8)(neko_interp_linear(0.05f, 0.06f, r) * 255.f);
                p->color.g = (u8)(neko_interp_linear(0.8f, 0.85f, r) * 255.f);
                p->color.b = (u8)(neko_interp_linear(0.1f, 0.12f, r) * 255.f);
            }

            const s32 wood_chance = 100;
            const s32 stone_chance = 300;
            const s32 sand_chance = 50;
            const s32 salt_chance = 20;

            // Random chance to die if in water
            if (is_in_water(chunk, x, y, &lx, &ly) && random_val(0, 250) == 0) {
                chunk_write_data(chunk, read_idx, particle_empty());
                return;
            }

            // If directly on top of some wall, then replace it
            if (in_bounds(x, y + 1) && ((get_particle_at(chunk, x, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                        (get_particle_at(chunk, x, y + 1).mat_id == mat_id_salt && random_val(0, salt_chance) == 0)

                                                )) {
                chunk_write_data(chunk, compute_idx(x, y + 1), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (in_bounds(x + 1, y + 1) && ((get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y + 1).mat_id == mat_id_salt && random_val(0, salt_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x + 1, y + 1), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (in_bounds(x - 1, y + 1) && ((get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y + 1).mat_id == mat_id_salt && random_val(0, salt_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x - 1, y + 1), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (in_bounds(x - 1, y) && ((get_particle_at(chunk, x - 1, y).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                               (get_particle_at(chunk, x - 1, y).mat_id == mat_id_salt && random_val(0, salt_chance) == 0)

                                                       )) {
                chunk_write_data(chunk, compute_idx(x - 1, y), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (in_bounds(x + 1, y) && ((get_particle_at(chunk, x + 1, y).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                               (get_particle_at(chunk, x + 1, y).mat_id == mat_id_salt && random_val(0, sand_chance) == 0)

                                                       )) {
                chunk_write_data(chunk, compute_idx(x + 1, y), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (in_bounds(x + 1, y - 1) && ((get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                                   (get_particle_at(chunk, x + 1, y - 1).mat_id == mat_id_salt && random_val(0, salt_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x + 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (in_bounds(x - 1, y - 1) && ((get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_sand && random_val(0, sand_chance) == 0) ||
                                                   (get_particle_at(chunk, x - 1, y - 1).mat_id == mat_id_salt && random_val(0, salt_chance) == 0)

                                                           )) {
                chunk_write_data(chunk, compute_idx(x - 1, y - 1), *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            }

            s32 ran = random_val(0, 1);
            s32 r = ran ? spread_rate : -spread_rate;
            s32 l = -r;
            s32 u = fall_rate;
            s32 v_idx = compute_idx(x + (s32)p->velocity.x, y + (s32)p->velocity.y);
            s32 b_idx = compute_idx(x, y + u);
            s32 bl_idx = compute_idx(x + l, y + u);
            s32 br_idx = compute_idx(x + r, y + u);
            s32 l_idx = compute_idx(x + l, y);
            s32 r_idx = compute_idx(x + r, y);
            s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

            // If touching wood or stone, destroy it

            if (in_bounds(x + vx, y + vy) && (is_empty(chunk, x + vx, y + vy))) {
                chunk_write_data(chunk, v_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x, y + u)) {
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + r, y + u)) {
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + l, y + u)) {
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y + u) && ((is_empty(chunk, x, y + u)))) {
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y + u);
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + l, y + u) && ((is_empty(chunk, x + l, y + u)))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + l, y + u);
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + r, y + u) && ((is_empty(chunk, x + r, y + u)))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + r, y + u);
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (is_in_liquid(chunk, x, y, &lx, &ly) && random_val(0, 10) == 0) {
                particle_t tmp_b = get_particle_at(chunk, lx, ly);
                chunk_write_data(chunk, compute_idx(lx, ly), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else {
                particle_t tmp = *p;
                bool found = false;

                // Don't try to spread if something is directly above you?
                if (completely_surrounded(chunk, x, y)) {
                    chunk_write_data(chunk, read_idx, tmp);
                    return;
                } else {
                    for (u32 i = 0; i < fall_rate; ++i) {
                        for (s32 j = spread_rate; j > 0; --j) {
                            if (in_bounds(x - j, y + i) && (is_empty(chunk, x - j, y + i) || get_particle_at(chunk, x - j, y + i).mat_id == mat_id_oil)) {
                                particle_t tmp = get_particle_at(chunk, x - j, y + i);
                                chunk_write_data(chunk, compute_idx(x - j, y + i), *p);
                                chunk_write_data(chunk, read_idx, tmp);
                                found = true;
                                break;
                            }
                            if (in_bounds(x + j, y + i) && (is_empty(chunk, x + j, y + i) || get_particle_at(chunk, x + j, y + i).mat_id == mat_id_oil)) {
                                particle_t tmp = get_particle_at(chunk, x + j, y + i);
                                chunk_write_data(chunk, compute_idx(x + j, y + i), *p);
                                chunk_write_data(chunk, read_idx, tmp);
                                found = true;
                                break;
                            }
                        }
                    }

                    if (!found) {
                        chunk_write_data(chunk, read_idx, tmp);
                    }
                }
            }
        } break;
        case mat_id_water: {

            u32 fall_rate = 2;
            s32 spread_rate = 5;

            p->velocity.y = neko_clamp(p->velocity.y + (world_gravity * dt), -10.f, 10.f);

            p->has_been_updated_this_frame = true;

            // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
            // if (in_bounds(x, y + 1) && !is_empty(chunk,x, y + 1) && get_particle_at(chunk,x, y + 1).mat_id != mat_id_water) {
            if (in_bounds(x, y + 1) && !is_empty(chunk, x, y + 1)) {
                p->velocity.y /= 2.f;
            }

            // Change color depending on pressure? Pressure would dictate how "deep" the water is, I suppose.
            if (random_val(0, (s32)(p->life_time * 100.f)) % 20 == 0) {
                f32 r = (f32)(random_val(0, 1)) / 2.f;
                p->color.r = (u8)(neko_interp_linear(0.1f, 0.15f, r) * 255.f);
                p->color.g = (u8)(neko_interp_linear(0.3f, 0.35f, r) * 255.f);
                p->color.b = (u8)(neko_interp_linear(0.7f, 0.8f, r) * 255.f);
            }

            s32 ran = random_val(0, 1);
            s32 r = ran ? spread_rate : -spread_rate;
            s32 l = -r;
            s32 u = fall_rate;
            s32 v_idx = compute_idx(x + (s32)p->velocity.x, y + (s32)p->velocity.y);
            s32 b_idx = compute_idx(x, y + u);
            s32 bl_idx = compute_idx(x + l, y + u);
            s32 br_idx = compute_idx(x + r, y + u);
            s32 l_idx = compute_idx(x + l, y);
            s32 r_idx = compute_idx(x + r, y);
            s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;
            s32 lx, ly;

            if (in_bounds(x + vx, y + vy) && (is_empty(chunk, x + vx, y + vy))) {
                chunk_write_data(chunk, v_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x, y + u)) {
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + r, y + u)) {
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            } else if (is_empty(chunk, x + l, y + u)) {
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, particle_empty());
            }
            // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
            else if (in_bounds(x, y + u) && ((is_empty(chunk, x, y + u) || (chunk->world_particle_data[b_idx].mat_id == mat_id_oil)))) {
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x, y + u);
                chunk_write_data(chunk, b_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + l, y + u) && ((is_empty(chunk, x + l, y + u) || chunk->world_particle_data[bl_idx].mat_id == mat_id_oil))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + l, y + u);
                chunk_write_data(chunk, bl_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (in_bounds(x + r, y + u) && ((is_empty(chunk, x + r, y + u) || chunk->world_particle_data[br_idx].mat_id == mat_id_oil))) {
                p->velocity.x = is_in_liquid(chunk, x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
                p->velocity.y += (world_gravity * dt);
                particle_t tmp_b = get_particle_at(chunk, x + r, y + u);
                chunk_write_data(chunk, br_idx, *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else if (is_in_liquid(chunk, x, y, &lx, &ly) && random_val(0, 10) == 0) {
                particle_t tmp_b = get_particle_at(chunk, lx, ly);
                chunk_write_data(chunk, compute_idx(lx, ly), *p);
                chunk_write_data(chunk, read_idx, tmp_b);
            } else {
                particle_t tmp = *p;
                bool found = false;

                // Don't try to spread if something is directly above you?
                if (completely_surrounded(chunk, x, y)) {
                    chunk_write_data(chunk, read_idx, tmp);
                    return;
                } else {
                    for (u32 i = 0; i < fall_rate; ++i) {
                        for (s32 j = spread_rate; j > 0; --j) {
                            if (in_bounds(x - j, y + i) && (is_empty(chunk, x - j, y + i) || get_particle_at(chunk, x - j, y + i).mat_id == mat_id_oil)) {
                                particle_t tmp = get_particle_at(chunk, x - j, y + i);
                                chunk_write_data(chunk, compute_idx(x - j, y + i), *p);
                                chunk_write_data(chunk, read_idx, tmp);
                                found = true;
                                break;
                            }
                            if (in_bounds(x + j, y + i) && (is_empty(chunk, x + j, y + i) || get_particle_at(chunk, x + j, y + i).mat_id == mat_id_oil)) {
                                particle_t tmp = get_particle_at(chunk, x + j, y + i);
                                chunk_write_data(chunk, compute_idx(x + j, y + i), *p);
                                chunk_write_data(chunk, read_idx, tmp);
                                found = true;
                                break;
                            }
                        }
                    }

                    if (!found) {
                        chunk_write_data(chunk, read_idx, tmp);
                    }
                }
            }
        } break;
        default:
            update_default(chunk, x, y);
            break;
    }
}

void update_default(game_chunk_t* chunk, u32 w, u32 h) {
    u32 read_idx = compute_idx(w, h);
    chunk_write_data(chunk, read_idx, get_particle_at(chunk, w, h));
}

particle_t particle_empty() {
    particle_t p = {0};
    p.mat_id = mat_id_empty;
    p.color = mat_col_empty;
    return p;
}

particle_t particle_sand() {
    particle_t p = {0};
    p.mat_id = mat_id_sand;
    // Random sand color
    f32 r = (f32)(random_val(0, 10)) / 10.f;
    p.color.r = (u8)(neko_interp_linear(0.8f, 1.f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.5f, 0.6f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.2f, 0.25f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_water() {
    particle_t p = {0};
    p.mat_id = mat_id_water;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.1f, 0.15f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.3f, 0.35f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.7f, 0.8f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_salt() {
    particle_t p = {0};
    p.mat_id = mat_id_salt;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.9f, 1.0f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.8f, 0.85f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.8f, 0.9f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_wood() {
    particle_t p = {0};
    p.mat_id = mat_id_wood;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.23f, 0.25f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.15f, 0.18f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.02f, 0.03f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_gunpowder() {
    particle_t p = {0};
    p.mat_id = mat_id_gunpowder;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.15f, 0.2f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.15f, 0.2f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.15f, 0.2f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_oil() {
    particle_t p = {0};
    p.mat_id = mat_id_oil;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.12f, 0.15f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.10f, 0.12f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.08f, 0.10f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_fire() {
    particle_t p = {0};
    p.mat_id = mat_id_fire;
    p.color = mat_col_fire;
    return p;
}

particle_t particle_lava() {
    particle_t p = {0};
    p.mat_id = mat_id_lava;
    p.color = mat_col_fire;
    return p;
}

particle_t particle_ember() {
    particle_t p = {0};
    p.mat_id = mat_id_ember;
    p.color = mat_col_ember;
    return p;
}

particle_t particle_smoke() {
    particle_t p = {0};
    p.mat_id = mat_id_smoke;
    p.color = mat_col_smoke;
    return p;
}

particle_t particle_steam() {
    particle_t p = {0};
    p.mat_id = mat_id_steam;
    p.color = mat_col_steam;
    return p;
}

particle_t particle_stone() {
    particle_t p = {0};
    p.mat_id = mat_id_stone;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.5f, 0.65f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.5f, 0.65f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.5f, 0.65f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_acid() {
    particle_t p = {0};
    p.mat_id = mat_id_acid;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.05f, 0.06f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.8f, 0.85f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.1f, 0.12f, r) * 255.f);
    p.color.a = 200;
    return p;
}

///////////////////////////////////////////////
//
//  位图边缘检测算法

#if 0

static neko_vec2 last_point;

void render_test() {

    u8* alpha = neko_tex_rgba_to_alpha((u8*)g_texture_buffer, CHUNK_W, CHUNK_H);
    u8* thresholded = neko_tex_alpha_to_thresholded(alpha, CHUNK_W, CHUNK_H, 90);
    u8* outlined = neko_tex_thresholded_to_outlined(thresholded, CHUNK_W, CHUNK_H);
    neko_safe_free(alpha);
    neko_safe_free(thresholded);

    neko_tex_point* outline = neko_tex_extract_outline_path(outlined, CHUNK_W, CHUNK_H, &l, 0);
    while (l) {
        s32 l0 = l;
        neko_tex_distance_based_path_simplification(outline, &l, 0.5f);
        // printf("simplified outline: %d -> %d\n", l0, l);

        l_check = l;

        for (s32 i = 0; i < l; i++) {
            // gfx->immediate.draw_line_ext(cb, neko_vec2_mul(last_point, {4.f, 4.f}), neko_vec2_mul(neko_vec2{(f32)outline[i].x, (f32)outline[i].y}, {4.f, 4.f}), 2.f, neko_color_white);

            last_point = {(f32)outline[i].x, (f32)outline[i].y};
        }

        outline = neko_tex_extract_outline_path(outlined, CHUNK_W, CHUNK_H, &l, outline);
    };

    neko_safe_free(outline);
    neko_safe_free(outlined);
}

#endif

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
u8* neko_tex_rgba_to_alpha(const u8* data, s32 w, s32 h) {
    u8* result = (u8*)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[(y * w + x) * 4 + 3];
    return result;
}

u8* neko_tex_alpha_to_thresholded(const u8* data, s32 w, s32 h, u8 threshold) {
    u8* result = (u8*)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
    return result;
}

u8* neko_tex_dilate_thresholded(const u8* data, s32 w, s32 h) {
    s32 x, y, dx, dy, cx, cy;
    u8* result = (u8*)neko_safe_malloc(w * h);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            result[y * w + x] = 0;
            for (dy = -1; dy <= 1; dy++) {
                for (dx = -1; dx <= 1; dx++) {
                    cx = x + dx;
                    cy = y + dy;
                    if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
                        if (data[cy * w + cx]) {
                            result[y * w + x] = 255;
                            dy = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

u8* neko_tex_thresholded_to_outlined(const u8* data, s32 w, s32 h) {
    u8* result = (u8*)neko_safe_malloc(w * h);
    s32 x, y;
    for (x = 0; x < w; x++) {
        result[x] = data[x];
        result[(h - 1) * w + x] = data[(h - 1) * w + x];
    }
    for (y = 1; y < h - 1; y++) {
        result[y * w] = data[y * w];
        for (x = 1; x < w - 1; x++) {
            if (data[y * w + x] && (!data[y * w + x - 1] || !data[y * w + x + 1] || !data[y * w + x - w] || !data[y * w + x + w])) {
                result[y * w + x] = 255;
            } else {
                result[y * w + x] = 0;
            }
        }
        result[y * w + w - 1] = data[y * w + w - 1];
    }
    return result;
}

// outline path procedures
static neko_tex_bool neko_tex_find_first_filled_pixel(const u8* data, s32 w, s32 h, neko_tex_point* first) {
    s32 x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (data[y * w + x]) {
                first->x = (short)x;
                first->y = (short)y;
                return 1;
            }
        }
    }
    return 0;
}

static neko_tex_bool neko_tex_find_next_filled_pixel(const u8* data, s32 w, s32 h, neko_tex_point current, neko_tex_direction* dir, neko_tex_point* next) {
    // turn around 180°, then make a clockwise scan for a filled pixel
    *dir = __neko_tex_direction_opposite(*dir);
    s32 i;
    for (i = 0; i < 8; i++) {
        __neko_tex_point_add(*next, current, neko_tex_direction_to_pixel_offset[*dir]);

        if (__neko_tex_point_is_inside(*next, w, h) && data[next->y * w + next->x]) return 1;

        // move to next angle (clockwise)
        *dir = *dir - 1;
        if (*dir < 0) *dir = 7;
    }
    return 0;
}

neko_tex_point* neko_tex_extract_outline_path(u8* data, s32 w, s32 h, s32* point_count, neko_tex_point* reusable_outline) {
    neko_tex_point* outline = reusable_outline;
    if (!outline) outline = (neko_tex_point*)neko_safe_malloc(w * h * sizeof(neko_tex_point));

    neko_tex_point current, next;

restart:
    if (!neko_tex_find_first_filled_pixel(data, w, h, &current)) {
        *point_count = 0;
        return outline;
    }

    s32 count = 0;
    neko_tex_direction dir = 0;

    while (__neko_tex_point_is_inside(current, w, h)) {
        data[current.y * w + current.x] = 0;  // clear the visited path
        outline[count++] = current;           // add our current point to the outline
        if (!neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) {
            // find loop connection
            neko_tex_bool found = 0;
            s32 i;
            for (i = 0; i < count / 2; i++)  // only allow big loops
            {
                if (__neko_tex_point_is_next_to(current, outline[i])) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                break;
            } else {
                // go backwards until we see outline pixels again
                dir = __neko_tex_direction_opposite(dir);
                count--;  // back up
                s32 prev;
                for (prev = count; prev >= 0; prev--) {
                    current = outline[prev];
                    outline[count++] = current;  // add our current point to the outline again
                    if (neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) break;
                }
            }
        }
        current = next;
    }

    if (count <= 2)  // too small, discard and try again!
        goto restart;
    *point_count = count;
    return outline;
}

void neko_tex_distance_based_path_simplification(neko_tex_point* outline, s32* outline_length, f32 distance_threshold) {
    s32 length = *outline_length;
    s32 l;
    for (l = length / 2 /*length - 1*/; l > 1; l--) {
        s32 a, b = l;
        for (a = 0; a < length; a++) {
            neko_tex_point ab;
            __neko_tex_point_sub(ab, outline[b], outline[a]);
            f32 lab = sqrtf((f32)(ab.x * ab.x + ab.y * ab.y));
            f32 ilab = 1.0f / lab;
            f32 abnx = ab.x * ilab, abny = ab.y * ilab;

            if (lab != 0.0f) {
                neko_tex_bool found = 1;
                s32 i = (a + 1) % length;
                while (i != b) {
                    neko_tex_point ai;
                    __neko_tex_point_sub(ai, outline[i], outline[a]);
                    f32 t = (abnx * ai.x + abny * ai.y) * ilab;
                    f32 distance = -abny * ai.x + abnx * ai.y;
                    if (t < 0.0f || t > 1.0f || distance > distance_threshold || -distance > distance_threshold) {
                        found = 0;
                        break;
                    }

                    if (++i == length) i = 0;
                }

                if (found) {
                    s32 i;
                    if (a < b) {
                        for (i = 0; i < length - b; i++) outline[a + i + 1] = outline[b + i];
                        length -= b - a - 1;
                    } else {
                        length = a - b + 1;
                        for (i = 0; i < length; i++) outline[i] = outline[b + i];
                    }
                    if (l >= length) l = length - 1;
                }
            }

            if (++b >= length) b = 0;
        }
    }
    *outline_length = length;
}

#if 0

#define STB_HBWANG_RAND() neko_rand_xorshf32()
#define STB_HBWANG_IMPLEMENTATION
#include "libs/stb/stb_herringbone_wang_tile.h"
#include "libs/stb/stb_image.h"
#include "libs/stb/stb_image_write.h"

void genwang(std::string filename, unsigned char* data, s32 xs, s32 ys, s32 w, s32 h) {
    stbhw_tileset ts;
    if (xs < 1 || xs > 1000) {
        fprintf(stderr, "xsize invalid or out of range\n");
        exit(1);
    }
    if (ys < 1 || ys > 1000) {
        fprintf(stderr, "ysize invalid or out of range\n");
        exit(1);
    }

    stbhw_build_tileset_from_image(&ts, data, w * 3, w, h);
    // allocate a buffer to create the final image to
    s32 yimg = ys + 4;
    auto buff = static_cast<unsigned char*>(malloc(3 * xs * yimg));
    stbhw_generate_image(&ts, NULL, buff, xs * 3, xs, yimg);
    stbi_write_png(filename.c_str(), xs, yimg, 3, buff, xs * 3);
    stbhw_free_tileset(&ts);
    free(buff);
}

void test_wang() {

    // mapgen {tile-file} {xsize} {ysize} {seed} [n]

    s32 xs = 128;
    s32 ys = 128;

    s32 n = 1;

    s32 w, h;

    unsigned char* data = stbi_load(neko_file_path("data/assets/textures/wang_test.png"), &w, &h, NULL, 3);

    neko_assert(data);

    printf("Output size: %dx%d\n", xs, ys);

    {
        u32 seed = neko_rand_xorshf32();

        // s32 num = seed + xs + 11 * (xs / -11) - 12 * (seed / 12);
        s32 num = xs % 11 + seed % 12;

        for (s32 i = 0; i < n; ++i) {

            auto filename = std::string("output_wang");
            filename = filename.substr(0, filename.size() - 4);
            filename = filename + std::to_string(seed) + "#" + std::to_string(i) + ".png";

            genwang(filename, data, xs, ys, w, h);
        }
    }

    free(data);
}

#endif

#endif  // !NEKO_GAME_CHUNK_H
