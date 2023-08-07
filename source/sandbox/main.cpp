

#include "engine/base/neko_cvar.hpp"
#include "engine/base/neko_ecs.h"
#include "engine/editor/neko_dbgui.hpp"
#include "engine/editor/neko_editor.hpp"
#include "engine/filesystem/neko_packer.h"
#include "engine/graphics/neko_particle.h"
#include "engine/graphics/neko_render_pass.h"
#include "engine/graphics/neko_sprite.h"
#include "engine/gui/imgui_impl/imgui_impl_glfw.h"
#include "engine/gui/imgui_impl/imgui_impl_opengl3.h"
#include "engine/gui/neko_imgui_utils.hpp"
#include "engine/gui/neko_text_renderer.hpp"
#include "engine/neko.h"
#include "engine/scripting/neko_scripting.h"
#include "engine/utility/defer.hpp"
#include "engine/utility/hash.hpp"
#include "engine/utility/logger.hpp"
#include "engine/utility/module.hpp"
#include "libs/glad/glad.h"

#define NEKO_BUILTIN_IMPL
#include "engine/gui/neko_builtin_font.h"

#define g_window_width 1420
#define g_window_height 880
neko_global const s32 g_texture_width = g_window_width / 4;
neko_global const s32 g_texture_height = g_window_height / 4;

// neko engine cpp
using namespace neko;

typedef neko_color_t cell_color_t;

typedef struct particle_t {
    u8 id;
    f32 life_time;
    neko_vec2 velocity;
    cell_color_t color;
    b32 has_been_updated_this_frame;
} particle_t;

// 测试 ECS 用
typedef struct {
    f32 x, y;
} CTransform;

typedef struct {
    f32 dx, dy;
} CVelocity;

typedef struct {
    u32 gl_id;
    f32 rotation;
} CSprite;

typedef enum {
    COMPONENT_TRANSFORM,
    COMPONENT_VELOCITY,
    COMPONENT_SPRITE,

    COMPONENT_COUNT
} ComponentType;

// Globals
neko_global neko_vertex_buffer_t g_vbo = {0};
neko_global neko_index_buffer_t g_ibo = {0};
neko_global neko_command_buffer_t g_cb = {0};
neko_global neko_shader_t g_shader = {0};
neko_global neko_uniform_t u_tex = {};
neko_global neko_uniform_t u_flip_y = {};
neko_global neko_texture_t g_tex = {0};
neko_global neko_texture_t g_tex_ui = {0};
neko_global neko_texture_t g_rt = {0};
neko_global neko_texture_t g_test_ase = {0};
neko_global neko_frame_buffer_t g_fb = {0};
neko_global blur_pass_t g_blur_pass = {0};
neko_global bright_filter_pass_t g_bright_pass = {0};
neko_global composite_pass_t g_composite_pass = {0};
neko_global neko_pack_reader g_pack_reader;

neko_resource(neko_font_t) g_test_font = {};

s32 l;
s32 l_check;

// For now, all particle information will simply be a value to determine its material id
#define mat_id_empty (u8)0
#define mat_id_sand (u8)1
#define mat_id_water (u8)2
#define mat_id_salt (u8)3
#define mat_id_wood (u8)4
#define mat_id_fire (u8)5
#define mat_id_smoke (u8)6
#define mat_id_ember (u8)7
#define mat_id_steam (u8)8
#define mat_id_gunpowder (u8)9
#define mat_id_oil (u8)10
#define mat_id_lava (u8)11
#define mat_id_stone (u8)12
#define mat_id_acid (u8)13

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

// World update processing structure
neko_global u8 *g_world_process_update_structure = {0};  // Every particle has id associated with it? Jeezuz...

// World particle data structure
neko_global particle_t *g_world_particle_data = {0};

// Texture buffers
neko_global cell_color_t *g_texture_buffer = {0};

// UI texture buffer
neko_global cell_color_t *g_ui_buffer = {0};

// Frame counter
neko_global u32 g_frame_counter = 0;

// World physics settings
neko_global f32 gravity = 10.f;

neko_global f32 g_selection_radius = 10.f;

neko_global bool g_show_material_selection_panel = true;
neko_global bool g_run_simulation = true;
neko_global bool g_show_frame_count = true;
neko_global bool g_use_post_processing = true;

// Handle for main window
neko_global neko_resource_handle g_window;

const char *v_src = R"glsl(
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

const char *f_src = R"glsl(
#version 330 core
in vec2 texCoord;
out vec4 frag_color;
uniform sampler2D u_tex;
void main()
{
   frag_color = texture(u_tex, texCoord);
}
)glsl";

// Forward Decls.
neko_result app_init();
neko_result app_update();
neko_result app_shutdown();

void imgui_init();
void imgui_new_frame();
void imgui_render();

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
b32 update_ui();

// Particle updates
void update_particle_sim();
void update_sand(u32 x, u32 y);
void update_water(u32 x, u32 y);
void update_salt(u32 x, u32 y);
void update_fire(u32 x, u32 y);
void update_smoke(u32 x, u32 y);
void update_ember(u32 x, u32 y);
void update_steam(u32 x, u32 y);
void update_gunpowder(u32 x, u32 y);
void update_oil(u32 x, u32 y);
void update_lava(u32 x, u32 y);
void update_acid(u32 x, u32 y);
void update_default(u32 x, u32 y);

// Utilities for writing data into color buffer
void write_data(u32 idx, particle_t);

typedef unsigned char neko_tex_uc;

neko_tex_uc *neko_tex_rgba_to_alpha(const neko_tex_uc *data, int w, int h);
neko_tex_uc *neko_tex_alpha_to_thresholded(const neko_tex_uc *data, int w, int h, neko_tex_uc threshold);
neko_tex_uc *neko_tex_thresholded_to_outlined(const neko_tex_uc *data, int w, int h);

typedef struct {
    short x, y;
} neko_tex_point;
neko_tex_point *neko_tex_extract_outline_path(neko_tex_uc *data, int w, int h, int *point_count, neko_tex_point *reusable_outline);
void neko_tex_distance_based_path_simplification(neko_tex_point *outline, int *outline_length, f32 distance_threshold);

// Rendering
void render_scene();

neko_global neko_sprite_renderer g_sr = {};
neko_global neko_sprite g_test_spr = {0};

neko_texture_t load_ase_texture_simple(const std::string &path) {

    ase_t *ase = cute_aseprite_load_from_file(path.c_str(), NULL);
    neko_defer([&] { cute_aseprite_free(ase); });

    if (NULL == ase) {
        neko_error("unable to load ase ", path);
        return neko_default_val();
    }

    neko_assert(ase->frame_count == 1, "load_ase_texture_simple used to load simple aseprite");

    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    neko_debug(std::format("load aseprite\n - frame_count {0}\n - palette.entry_count{1}\n - w={2} h={3}", ase->frame_count, ase->palette.entry_count, ase->w, ase->h));

    int bpp = 4;

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();

    t_desc.texture_format = neko_texture_format_rgba8;
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    t_desc.num_comps = 4;
    t_desc.data = ase->frames->pixels;

    neko_texture_t tex = gfx->construct_texture(t_desc);

    return tex;
}

neko_vec2 calculate_mouse_position() {
    neko_vec2 ws = neko_engine_instance()->ctx.platform->window_size(g_window);
    neko_vec2 pmp = neko_engine_instance()->ctx.platform->mouse_position();
    // Need to place mouse into frame
    f32 x_scale = pmp.x / (f32)ws.x;
    f32 y_scale = pmp.y / (f32)ws.y;
    return neko_vec2{x_scale * (f32)g_texture_width, y_scale * (f32)g_texture_height};
}

s32 random_val(s32 lower, s32 upper) {
    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand() % (upper - lower + 1) + lower);
}

s32 compute_idx(s32 x, s32 y) { return (y * g_texture_width + x); }

b32 in_bounds(s32 x, s32 y) {
    if (x < 0 || x > (g_texture_width - 1) || y < 0 || y > (g_texture_height - 1)) return false;
    return true;
}

b32 is_empty(s32 x, s32 y) { return (in_bounds(x, y) && g_world_particle_data[compute_idx(x, y)].id == mat_id_empty); }

particle_t get_particle_at(s32 x, s32 y) { return g_world_particle_data[compute_idx(x, y)]; }

b32 completely_surrounded(s32 x, s32 y) {
    // Top
    if (in_bounds(x, y - 1) && !is_empty(x, y - 1)) {
        return false;
    }
    // Bottom
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1)) {
        return false;
    }
    // Left
    if (in_bounds(x - 1, y) && !is_empty(x - 1, y)) {
        return false;
    }
    // Right
    if (in_bounds(x + 1, y) && !is_empty(x + 1, y)) {
        return false;
    }
    // Top Left
    if (in_bounds(x - 1, y - 1) && !is_empty(x - 1, y - 1)) {
        return false;
    }
    // Top Right
    if (in_bounds(x + 1, y - 1) && !is_empty(x + 1, y - 1)) {
        return false;
    }
    // Bottom Left
    if (in_bounds(x - 1, y + 1) && !is_empty(x - 1, y + 1)) {
        return false;
    }
    // Bottom Right
    if (in_bounds(x + 1, y + 1) && !is_empty(x + 1, y + 1)) {
        return false;
    }

    return true;
}

b32 is_in_liquid(s32 x, s32 y, s32 *lx, s32 *ly) {
    if (in_bounds(x, y) && (get_particle_at(x, y).id == mat_id_water || get_particle_at(x, y).id == mat_id_oil)) {
        *lx = x;
        *ly = y;
        return true;
    }
    if (in_bounds(x, y - 1) && (get_particle_at(x, y - 1).id == mat_id_water || get_particle_at(x, y - 1).id == mat_id_oil)) {
        *lx = x;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x, y + 1) && (get_particle_at(x, y + 1).id == mat_id_water || get_particle_at(x, y + 1).id == mat_id_oil)) {
        *lx = x;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x - 1, y) && (get_particle_at(x - 1, y).id == mat_id_water || get_particle_at(x - 1, y).id == mat_id_oil)) {
        *lx = x - 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x - 1, y - 1) && (get_particle_at(x - 1, y - 1).id == mat_id_water || get_particle_at(x - 1, y - 1).id == mat_id_oil)) {
        *lx = x - 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x - 1, y + 1) && (get_particle_at(x - 1, y + 1).id == mat_id_water || get_particle_at(x - 1, y + 1).id == mat_id_oil)) {
        *lx = x - 1;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x + 1, y) && (get_particle_at(x + 1, y).id == mat_id_water || get_particle_at(x + 1, y).id == mat_id_oil)) {
        *lx = x + 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x + 1, y - 1) && (get_particle_at(x + 1, y - 1).id == mat_id_water || get_particle_at(x + 1, y - 1).id == mat_id_oil)) {
        *lx = x + 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x + 1, y + 1) && (get_particle_at(x + 1, y + 1).id == mat_id_water || get_particle_at(x + 1, y + 1).id == mat_id_oil)) {
        *lx = x + 1;
        *ly = y + 1;
        return true;
    }
    return false;
}

b32 is_in_water(s32 x, s32 y, s32 *lx, s32 *ly) {
    if (in_bounds(x, y) && (get_particle_at(x, y).id == mat_id_water)) {
        *lx = x;
        *ly = y;
        return true;
    }
    if (in_bounds(x, y - 1) && (get_particle_at(x, y - 1).id == mat_id_water)) {
        *lx = x;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x, y + 1) && (get_particle_at(x, y + 1).id == mat_id_water)) {
        *lx = x;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x - 1, y) && (get_particle_at(x - 1, y).id == mat_id_water)) {
        *lx = x - 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x - 1, y - 1) && (get_particle_at(x - 1, y - 1).id == mat_id_water)) {
        *lx = x - 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x - 1, y + 1) && (get_particle_at(x - 1, y + 1).id == mat_id_water)) {
        *lx = x - 1;
        *ly = y + 1;
        return true;
    }
    if (in_bounds(x + 1, y) && (get_particle_at(x + 1, y).id == mat_id_water)) {
        *lx = x + 1;
        *ly = y;
        return true;
    }
    if (in_bounds(x + 1, y - 1) && (get_particle_at(x + 1, y - 1).id == mat_id_water)) {
        *lx = x + 1;
        *ly = y - 1;
        return true;
    }
    if (in_bounds(x + 1, y + 1) && (get_particle_at(x + 1, y + 1).id == mat_id_water)) {
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

void drop_file_callback(void *platform_window, s32 count, const char **file_paths) {
    if (count < 1) return;

    // Just do first one for now...
    if (count > 1) count = 1;

    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    // We'll place at the mouse position as well, for shiggles
    neko_vec2 mp = calculate_mouse_position();

    for (s32 i = 0; i < count; ++i) {
        // Need to verify this IS an image first.
        char temp_file_extension_buffer[16] = {0};
        neko_util_get_file_extension(temp_file_extension_buffer, sizeof(temp_file_extension_buffer), file_paths[0]);
        if (neko_string_compare_equal(temp_file_extension_buffer, "png") || neko_string_compare_equal(temp_file_extension_buffer, "jpg") ||
            neko_string_compare_equal(temp_file_extension_buffer, "jpeg") || neko_string_compare_equal(temp_file_extension_buffer, "bmp")) {
            // Load texture into memory
            s32 _w, _h, _n;
            void *texture_data = NULL;

            // Force texture data to 3 components
            texture_data = gfx->load_texture_data_from_file(file_paths[i], false, neko_texture_format_rgb8, &_w, &_h, &_n);
            _n = 3;

            // Not sure what the format should be, so this is ...blah. Need to find a way to determine this beforehand.
            u8 *data = (u8 *)texture_data;

            s32 sx = (g_texture_width - _w) / 2;
            s32 sy = (g_texture_height - _h) / 2;

            // Now we need to process the data and place it into our particle/color buffers
            for (u32 h = 0; h < _h; ++h) {
                for (u32 w = 0; w < _w; ++w) {
                    cell_color_t c = {data[(h * _w + w) * _n + 0], data[(h * _w + w) * _n + 1], data[(h * _w + w) * _n + 2], 255};

                    // Get color of this pixel in the image
                    particle_t p = get_closest_particle_from_color(c);

                    // Let's place this thing in the center instead...
                    if (in_bounds(sx + w, sy + h)) {

                        u32 idx = compute_idx(sx + w, sy + h);
                        write_data(idx, p);
                    }
                }
            }

            // Free texture data
            neko_safe_free(texture_data);
        }
    }
}

neko_ecs_decl_mask(MOVEMENT_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_VELOCITY);
void movement_system(neko_ecs *ecs) {
    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(MOVEMENT_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            CVelocity *velocity = (CVelocity *)neko_ecs_ent_get_component(ecs, e, COMPONENT_VELOCITY);

            xform->x += velocity->dx;
            xform->y += velocity->dy;
        }
    }
}

neko_ecs_decl_mask(SPRITE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_SPRITE);
void sprite_render_system(neko_ecs *ecs) {
    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(SPRITE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            CSprite *sprite = (CSprite *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            // render_sprite(sprite->gl_id, sprite->rot);
        }
    }
}

void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1000, sizeof(CTransform), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 200, sizeof(CVelocity), NULL);
    neko_ecs_register_component(ecs, COMPONENT_SPRITE, 1000, sizeof(CSprite), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems will run the systems in the order they are registered
    // ecs_run_system is also available if you wish to handle each system seperately
    //
    // neko_ecs, function pointer to system (must take a parameter of neko_ecs), system type
    neko_ecs_register_system(ecs, movement_system, ECS_SYSTEM_UPDATE);
    neko_ecs_register_system(ecs, sprite_render_system, ECS_SYSTEM_RENDER);
}

neko_global neko_ecs *g_ecs;

neko_global font_index g_basic_font;

neko_global neko_engine_cvar_t g_cvar = {0};

neko_global neko_obj_swarm swarm;
neko_global neko_swarm_simulator_settings settings;

neko_global std::size_t imgui_mem_usage = 0;

// Here, we'll initialize all of our application data, which in this case is our graphics resources
neko_result app_init() {
    // Cache instance of api contexts from engine
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // Construct command buffer (the command buffer is used to allow for immediate drawing at any point in our program)
    g_cb = neko_command_buffer_new();

    // Construct shader from our source above
    g_shader = gfx->construct_shader(v_src, f_src);

    // Construct uniform for shader
    u_tex = gfx->construct_uniform(g_shader, "u_tex", neko_uniform_type_sampler2d);
    u_flip_y = gfx->construct_uniform(g_shader, "u_flip_y", neko_uniform_type_int);

    // Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
    neko_vertex_attribute_type layout[] = {neko_vertex_attribute_float2, neko_vertex_attribute_float2};

    // Vertex data for triangle
    f32 v_data[] = {
            // Positions  UVs
            -1.0f, -1.0f, 0.0f, 0.0f,  // Top Left
            1.0f,  -1.0f, 1.0f, 0.0f,  // Top Right
            -1.0f, 1.0f,  0.0f, 1.0f,  // Bottom Left
            1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
    };

    u32 i_data[] = {0, 2, 3, 3, 1, 0};

    // Construct vertex buffer
    g_vbo = gfx->construct_vertex_buffer(layout, sizeof(layout), v_data, sizeof(v_data));
    // Construct index buffer
    g_ibo = gfx->construct_index_buffer(i_data, sizeof(i_data));

    // Construct world data (for now, it'll just be the size of the screen)
    g_world_particle_data = (particle_t *)neko_safe_malloc(g_texture_width * g_texture_height * sizeof(particle_t));

    // Construct texture buffer data
    g_texture_buffer = (cell_color_t *)neko_safe_malloc(g_texture_width * g_texture_height * sizeof(cell_color_t));

    g_ui_buffer = (cell_color_t *)neko_safe_malloc(g_texture_width * g_texture_height * sizeof(cell_color_t));

    // Set buffers to 0
    memset(g_texture_buffer, 0, g_texture_width * g_texture_height * sizeof(cell_color_t));
    memset(g_world_particle_data, 0, g_texture_width * g_texture_height * sizeof(particle_t));
    memset(g_ui_buffer, 0, g_texture_width * g_texture_height);

    // Construct texture resource from GPU
    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_format = neko_texture_format_rgba8;
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = g_texture_width;
    t_desc.height = g_texture_height;
    t_desc.num_comps = 4;
    t_desc.data = g_texture_buffer;

    g_tex = gfx->construct_texture(t_desc);

    // Construct texture resource from GPU
    t_desc = neko_texture_parameter_desc_default();
    t_desc.texture_format = neko_texture_format_rgba8;
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = g_texture_width;
    t_desc.height = g_texture_height;
    t_desc.num_comps = 4;
    t_desc.data = g_ui_buffer;

    g_tex_ui = gfx->construct_texture(t_desc);

    // Construct target for offscreen rendering
    t_desc.data = NULL;
    g_rt = gfx->construct_texture(t_desc);

    // Construct frame buffer
    g_fb = gfx->construct_frame_buffer(g_rt);

    // g_test_ase = load_ase_texture_simple(neko_file_path("data/assets/textures/B_witch.ase"));

    neko_sprite_load(&g_test_spr, neko_file_path("data/assets/textures/B_witch.ase"));

    g_sr.sprite = &g_test_spr;

    // sprite_renderer_set_frame(&g_sr, 0);

    neko_sprite_renderer_play(&g_sr, "Attack");

    // Cache window handle from platform
    g_window = neko_engine_instance()->ctx.platform->main_window();

    // Initialize render passes
    g_blur_pass = blur_pass_ctor();
    g_bright_pass = bright_filter_pass_ctor();
    g_composite_pass = composite_pass_ctor();
    neko_pack_result pack_result = neko_create_file_pack_reader(neko_file_path("data/resources.pack"), 0, 0, &g_pack_reader);

    if (pack_result != SUCCESS_PACK_RESULT) {
        // METADOT_ERROR("%d", pack_result);
        neko_assert(0);
    }

    neko_engine_cvar_init(&g_cvar, [] {});

    // 初始化 ecs
    g_ecs = neko_ecs_make(1000, COMPONENT_COUNT, 3);
    register_components(g_ecs);
    register_systems(g_ecs);

    // 测试用
    neko_ecs_ent e = neko_ecs_ent_make(g_ecs);
    CTransform xform = {0, 0};
    CVelocity velocity = {5, 0};
    neko_ecs_ent_add_component(g_ecs, e, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(g_ecs, e, COMPONENT_VELOCITY, &velocity);

    // neko_ecs_ent_destroy(ecs, e);

    // 启用 imgui
    imgui_init();

    neko_editor_create(g_cvar).create("shader", [&](u8) {
        neko_editor_inspect_shader("Hello glsl", g_shader.program_id);
        neko_editor_inspect_shader("horizontal_blur_shader", g_blur_pass.data.horizontal_blur_shader.program_id);
        neko_editor_inspect_shader("vertical_blur_shader", g_blur_pass.data.vertical_blur_shader.program_id);
        neko_editor_inspect_shader("g_bright_pass", g_bright_pass.data.shader.program_id);

        if (ImGui::Button("mem check")) neko_mem_check_leaks(false);

        {

            ImGui::Text("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
            ImGui::Text("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
            ImGui::Text("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

            GLint total_mem_kb = 0;
            glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);

            GLint cur_avail_mem_kb = 0;
            glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);

            ImGui::Text("GPU MemTotalAvailable: %.2lf mb", (f64)(cur_avail_mem_kb / 1024.0f));
            ImGui::Text("GPU MemCurrentUsage: %.2lf mb", (f64)((total_mem_kb - cur_avail_mem_kb) / 1024.0f));

            auto L = the<scripting>().neko_lua.get_lua_state();
            lua_gc(L, LUA_GCCOLLECT, 0);
            lua_Integer kb = lua_gc(L, LUA_GCCOUNT, 0);
            lua_Integer bytes = lua_gc(L, LUA_GCCOUNTB, 0);

            ImGui::Text("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
            ImGui::Text("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

            ImGui::Text("ImGui MemoryUsage: %.2lf mb", ((f64)imgui_mem_usage / 1048576.0));

            static neko_platform_meminfo meminfo;

            neko_timed_action(60, { meminfo = neko_engine_instance()->ctx.platform->get_meminfo(); });

            ImGui::Text("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
            ImGui::Text("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
            ImGui::Text("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
            ImGui::Text("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));
        }

        return 0;
    });

    the<text_renderer>().resize({g_window_width, g_window_height});

    auto ui_font = neko_assets_get(g_pack_reader, ".\\fonts\\fusion-pixel.ttf");
    g_basic_font = the<text_renderer>().load(ui_font.data, ui_font.size, 24.0f);

    g_test_font = gfx->construct_font_from_file(neko_file_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf"), 64.f);

    // Load UI font texture data from file
    construct_font_data(g_font_data);

    // Set up callback for dropping them files, yo.
    platform->set_dropped_files_callback(platform->main_window(), &drop_file_callback);

    return neko_result_success;
}

neko_result app_update() {
    // Grab global instance of engine
    neko_engine_t *engine = neko_engine_instance();

    // If we press the escape key, exit the application
    if (engine->ctx.platform->key_pressed(neko_keycode_esc)) {
        return neko_result_success;
    }

    if (engine->ctx.platform->key_pressed(neko_keycode_tab)) {
        the<dbgui>().flags("cvar") ^= neko_dbgui_flags::no_visible;
    }

    // neko_timed_action(60, { neko_println("frame: %.5f ms", engine->ctx.platform->time.frame); });

    neko_invoke_once([] {
        the<dbgui>().update("shader", [](s32) {
            ImGui::Image((void *)(intptr_t)g_tex.id, ImVec2(g_texture_width, g_texture_height), ImVec2(0, 0), ImVec2(1, 1));
            return 0;
        });
    }(););

    // All application updates
    b32 ui_interaction = update_ui();
    if (!ui_interaction) {
        update_input();
    }

    if (g_run_simulation) {
        update_particle_sim();

        neko_sprite_renderer_update(&g_sr, engine->ctx.platform->time.delta);

        swarm.update(engine->ctx.platform->time.current, settings);

        neko_ecs_run_systems(g_ecs, ECS_SYSTEM_UPDATE);
        neko_ecs_run_systems(g_ecs, ECS_SYSTEM_RENDER);
    }

    /*===============
    // Render scene
    ================*/
    render_scene();

    // Update frame counter
    g_frame_counter = (g_frame_counter + 1) % u32_max;

    imgui_new_frame();

    the<text_renderer>().drawcmd();

    the<dbgui>().draw();

    settings.show();

    // 渲染 imgui 内容
    imgui_render();

    return neko_result_in_progress;
}

neko_result app_shutdown() {

    neko_ecs_destroy(g_ecs);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    neko_destroy_pack_reader(g_pack_reader);

    neko_info("app_shutdown");
    return neko_result_success;
}

neko_private(void *) __neko_imgui_malloc(size_t sz, void *user_data) { return neko_mem_leak_check_alloc((sz), (char *)__FILE__, __LINE__, &imgui_mem_usage); }

neko_private(void) __neko_imgui_free(void *ptr, void *user_data) { neko_mem_leak_check_free(ptr, &imgui_mem_usage); }

void imgui_init() {
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // Get main window from platform
    GLFWwindow *win = (GLFWwindow *)platform->raw_window_handle(platform->main_window());

    ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.IniFilename = "imgui.ini";

    io.Fonts->AddFontFromFileTTF(neko_file_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf"), 20.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init();
}

void imgui_new_frame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_render() {
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // TODO: 将这一切抽象出来并通过命令缓冲系统渲染
    ImGui::Render();
    neko_vec2 fbs = platform->frame_buffer_size(platform->main_window());
    glViewport(0, 0, fbs.x, fbs.y);
    // glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void putpixel(int x, int y) {
    if (in_bounds(x, y)) {
        g_ui_buffer[compute_idx(x, y)] = cell_color_t{255, 255, 255, 255};
    }
}

// Function to put pixels
// at subsequence points
void drawCircle(int xc, int yc, int x, int y) {
    putpixel(xc + x, yc + y);
    putpixel(xc - x, yc + y);
    putpixel(xc + x, yc - y);
    putpixel(xc - x, yc - y);
    putpixel(xc + y, yc + x);
    putpixel(xc - y, yc + x);
    putpixel(xc + y, yc - x);
    putpixel(xc - y, yc - x);
}

// Function for circle-generation
// using Bresenham's algorithm
void circleBres(int xc, int yc, int r) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    drawCircle(xc, yc, x, y);
    while (y >= x) {
        // For each pixel we will
        // draw all eight pixels
        x++;

        // Check for decision parameter
        // and correspondingly
        // update d, x, y
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else
            d = d + 4 * x + 6;
        drawCircle(xc, yc, x, y);
    }
}

void update_input() {

    if (ImGui::GetIO().WantCaptureMouse) return;

    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    if (platform->key_pressed(neko_keycode_i)) {
        g_show_material_selection_panel = !g_show_material_selection_panel;
    }

    if (platform->key_pressed(neko_keycode_f)) {
        g_show_frame_count = !g_show_frame_count;
    }

    if (platform->key_pressed(neko_keycode_b)) {
        g_use_post_processing = !g_use_post_processing;
    }

    f32 wx = 0, wy = 0;
    platform->mouse_wheel(&wx, &wy);
    if (platform->key_pressed(neko_keycode_lbracket) || wy < 0.f) {
        g_selection_radius = neko_clamp(g_selection_radius - 1.f, 1.f, 100.f);
    }
    if (platform->key_pressed(neko_keycode_rbracket) || wy > 0.f) {
        g_selection_radius = neko_clamp(g_selection_radius + 1.f, 1.f, 100.f);
    }

    if (platform->key_pressed(neko_keycode_p)) {
        g_run_simulation = !g_run_simulation;
    }

    // Clear data
    if (platform->key_pressed(neko_keycode_c)) {
        memset(g_texture_buffer, 0, sizeof(cell_color_t) * g_texture_width * g_texture_height);
        memset(g_world_particle_data, 0, sizeof(particle_t) * g_texture_width * g_texture_height);
    }

    // Mouse input for testing
    if (platform->mouse_down(neko_mouse_lbutton)) {
        neko_vec2 mp = calculate_mouse_position();
        f32 mp_x = neko_clamp(mp.x, 0.f, (f32)g_texture_width - 1.f);
        f32 mp_y = neko_clamp(mp.y, 0.f, (f32)g_texture_height - 1.f);
        u32 max_idx = (g_texture_width * g_texture_height) - 1;
        s32 r_amt = random_val(1, 10000);
        const f32 R = g_selection_radius;

        // Spawn in a circle around the mouse
        for (u32 i = 0; i < r_amt; ++i) {
            f32 ran = (f32)random_val(0, 100) / 100.f;
            f32 r = R * sqrt(ran);
            f32 theta = (f32)random_val(0, 100) / 100.f * 2.f * neko_pi;
            f32 rx = cos((f32)theta) * r;
            f32 ry = sin((f32)theta) * r;
            s32 mpx = (s32)neko_clamp(mp_x + (f32)rx, 0.f, (f32)g_texture_width - 1.f);
            s32 mpy = (s32)neko_clamp(mp_y + (f32)ry, 0.f, (f32)g_texture_height - 1.f);
            s32 idx = mpy * (s32)g_texture_width + mpx;
            idx = neko_clamp(idx, 0, max_idx);

            if (is_empty(mpx, mpy)) {
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
                write_data(idx, p);
            }
        }
    }

    // Solid Erase
    if (platform->mouse_down(neko_mouse_rbutton)) {
        neko_vec2 mp = calculate_mouse_position();
        f32 mp_x = neko_clamp(mp.x, 0.f, (f32)g_texture_width - 1.f);
        f32 mp_y = neko_clamp(mp.y, 0.f, (f32)g_texture_height - 1.f);
        u32 max_idx = (g_texture_width * g_texture_height) - 1;
        const f32 R = g_selection_radius;

        // Erase in a circle pattern
        for (s32 i = -R; i < R; ++i) {
            for (s32 j = -R; j < R; ++j) {
                s32 rx = ((s32)mp_x + j);
                s32 ry = ((s32)mp_y + i);
                neko_vec2 r = neko_vec2{(f32)rx, (f32)ry};

                if (in_bounds(rx, ry) && neko_vec2_dist(mp, r) <= R) {
                    write_data(compute_idx(rx, ry), particle_empty());
                }
            }
        }
    }

    // Need to detect if mouse has entered the screen with payload...
}

void update_particle_sim() {
    // Cache engine subsystem interfaces
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // Update frame counter (loop back to 0 if we roll past u32 max)
    b32 frame_counter_even = ((g_frame_counter % 2) == 0);
    s32 ran = frame_counter_even ? 0 : 1;

    const f32 dt = platform->time.delta;

    // through read data and update write buffer
    // update "bottom up", since all the data is edited "in place". Double buffering all data would fix this
    //  issue, however it requires double all of the data.
    for (u32 y = g_texture_height - 1; y > 0; --y) {
        for (u32 x = ran ? 0 : g_texture_width - 1; ran ? x < g_texture_width : x > 0; ran ? ++x : --x) {
            // Current particle idx
            u32 read_idx = compute_idx(x, y);

            // Get material of particle at point
            u8 mat_id = get_particle_at(x, y).id;

            // Update particle's lifetime (I guess just use frames)? Or should I have sublife?
            g_world_particle_data[read_idx].life_time += 1.f * dt;

#define __mat_case(name)       \
    case mat_id_##name:        \
        update_##name##(x, y); \
        break

            switch (mat_id) {
                __mat_case(sand);
                __mat_case(water);
                __mat_case(salt);
                __mat_case(fire);
                __mat_case(smoke);
                __mat_case(ember);
                __mat_case(steam);
                __mat_case(gunpowder);
                __mat_case(oil);
                __mat_case(lava);
                __mat_case(acid);
                    // Do nothing for empty or default case
                default:
                case mat_id_empty: {
                    // update_default(w, h, i);
                } break;
            }
        }
    }
#undef __mat_case

    // Can remove this loop later on by keeping update structure and setting that for the particle as it moves,
    // then at the end of frame just memsetting the entire structure to 0.
    for (u32 y = g_texture_height - 1; y > 0; --y) {
        for (u32 x = ran ? 0 : g_texture_width - 1; ran ? x < g_texture_width : x > 0; ran ? ++x : --x) {
            // Set particle's update to false for next frame
            g_world_particle_data[compute_idx(x, y)].has_been_updated_this_frame = false;
        }
    }
}

void draw_glyph_at(font_t *f, cell_color_t *buffer, s32 x, s32 y, char c, cell_color_t col) {
    u8 *font_data = (u8 *)f->data;
    font_glyph_t g = get_glyph(f, c);

    // How to accurately place? I have width and height of glyph in texture, but need to convert this to RGBA data for ui buffer
    for (s32 h = 0; h < g.height; ++h) {
        for (s32 w = 0; w < g.width; ++w) {
            s32 _w = w + g.x;
            s32 _h = h + g.y;
            u8 a = font_data[(_h * f->width + _w) * f->num_comps + 0] == 0 ? 0 : 255;
            cell_color_t c = {font_data[(_h * f->width + _w) * f->num_comps + 0], font_data[(_h * f->width + _w) * f->num_comps + 1], font_data[(_h * f->width + _w) * f->num_comps + 2], a};
            if (in_bounds(x + w, y + h) && a) {
                buffer[compute_idx(x + w, y + h)] = col;
            }
        }
    }
}

void draw_string_at(font_t *f, cell_color_t *buffer, s32 x, s32 y, const char *str, usize len, cell_color_t col) {
    u8 *font_data = (u8 *)f->data;
    for (u32 i = 0; i < len; ++i) {
        font_glyph_t g = get_glyph(f, str[i]);
        draw_glyph_at(f, buffer, x, y, str[i], col);
        x += g.width + f->glyph_advance;  // Move by glyph width + advance
    }
}

b32 in_rect(neko_vec2 p, neko_vec2 ro, neko_vec2 rd) {
    if (p.x < ro.x || p.x > ro.x + rd.x || p.y < ro.y || p.y > ro.y + rd.y) return false;
    return true;
}

b32 gui_rect(cell_color_t *buffer, s32 _x, s32 _y, s32 _w, s32 _h, cell_color_t c) {
    neko_vec2 mp = calculate_mouse_position();

    for (u32 h = 0; h < _h; ++h) {
        for (u32 w = 0; w < _w; ++w) {
            if (in_bounds(_x + w, _y + h)) {
                buffer[compute_idx(_x + w, _y + h)] = c;
            }
        }
    }

    b32 clicked = neko_engine_instance()->ctx.platform->mouse_pressed(neko_mouse_lbutton);

    return in_rect(mp, neko_vec2{(f32)_x, (f32)_y}, neko_vec2{(f32)_w, (f32)_h}) && clicked;
}

#define __gui_interaction(x, y, w, h, c, str, id)                                                                                     \
    do {                                                                                                                              \
        if ((id) == g_material_selection) {                                                                                           \
            const s32 b = 2;                                                                                                          \
            gui_rect(g_ui_buffer, x - b / 2, y - b / 2, w + b, h + b, cell_color_t{200, 150, 20, 255});                               \
        }                                                                                                                             \
        neko_vec2 mp = calculate_mouse_position();                                                                                    \
        if (in_rect(mp, neko_vec2{(f32)(x), (f32)(y)}, neko_vec2{(w), (h)})) {                                                        \
            interaction |= true;                                                                                                      \
            char _str[] = (str);                                                                                                      \
            cell_color_t col = cell_color_t{255, 255, 255, 255};                                                                      \
            cell_color_t s_col = cell_color_t{10, 10, 10, 255};                                                                       \
            cell_color_t r_col = cell_color_t{5, 5, 5, 170};                                                                          \
            /*Draw rect around text as well for easier viewing*/                                                                      \
            gui_rect(g_ui_buffer, g_texture_width / 2 - 50, 15, 100, 20, r_col);                                                      \
            draw_string_at(&g_font, g_ui_buffer, g_texture_width / 2 + 1 - (sizeof(str) * 5) / 2, 20 - 1, _str, sizeof(_str), s_col); \
            draw_string_at(&g_font, g_ui_buffer, g_texture_width / 2 - (sizeof(str) * 5) / 2, 20, _str, sizeof(_str), col);           \
        }                                                                                                                             \
        if (gui_rect(g_ui_buffer, x, y, w, h, c)) {                                                                                   \
            g_material_selection = id;                                                                                                \
        }                                                                                                                             \
    } while (0)

b32 update_ui() {
    b32 interaction = false;
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // Cache transformed mouse position
    neko_vec2 mp = calculate_mouse_position();

    // Do ui stuff
    memset(g_ui_buffer, 0, g_texture_width * g_texture_height * sizeof(cell_color_t));

    // Material selection panel gui
    if (g_show_material_selection_panel) {
        const s32 offset = 12;
        s32 xoff = 20;
        s32 base = 10;

        // Sand Selection
        __gui_interaction(g_texture_width - xoff, base + offset * 0, 10, 10, mat_col_sand, "Sand", mat_sel_sand);
        __gui_interaction(g_texture_width - xoff, base + offset * 1, 10, 10, mat_col_water, "Water", mat_sel_water);
        __gui_interaction(g_texture_width - xoff, base + offset * 2, 10, 10, mat_col_smoke, "Smoke", mat_sel_smoke);
        __gui_interaction(g_texture_width - xoff, base + offset * 3, 10, 10, mat_col_fire, "Fire", mat_sel_fire);
        __gui_interaction(g_texture_width - xoff, base + offset * 4, 10, 10, mat_col_steam, "Steam", mat_sel_steam);
        __gui_interaction(g_texture_width - xoff, base + offset * 5, 10, 10, mat_col_oil, "Oil", mat_sel_oil);
        __gui_interaction(g_texture_width - xoff, base + offset * 6, 10, 10, mat_col_salt, "Salt", mat_sel_salt);
        __gui_interaction(g_texture_width - xoff, base + offset * 7, 10, 10, mat_col_wood, "Wood", mat_sel_wood);
        __gui_interaction(g_texture_width - xoff, base + offset * 8, 10, 10, mat_col_stone, "Stone", mat_sel_stone);
        __gui_interaction(g_texture_width - xoff, base + offset * 9, 10, 10, mat_col_lava, "Lava", mat_sel_lava);
        __gui_interaction(g_texture_width - xoff, base + offset * 10, 10, 10, mat_col_gunpowder, "GunPowder", mat_sel_gunpowder);
        __gui_interaction(g_texture_width - xoff, base + offset * 11, 10, 10, mat_col_acid, "Acid", mat_sel_acid);
    }

    if (g_show_frame_count) {

        char frame_time_str[256];
        neko_snprintf(frame_time_str, sizeof(frame_time_str), "frame: %.2f ms", platform->time.frame);
        draw_string_at(&g_font, g_ui_buffer, 10, 10, frame_time_str, strlen(frame_time_str), cell_color_t{255, 255, 255, 255});

        char sim_state_str[256];
        neko_snprintf(sim_state_str, sizeof(sim_state_str), "state: %s", g_run_simulation ? "running" : "paused");
        draw_string_at(&g_font, g_ui_buffer, 10, 20, sim_state_str, strlen(sim_state_str), cell_color_t{255, 255, 255, 255});

        the<text_renderer>().push(std::format("test: {0}", l_check), g_basic_font, 40, 160);
    }

    // Draw circle around mouse pointer
    s32 R = g_selection_radius;
    circleBres((s32)mp.x, (s32)mp.y, R);

    // Upload our updated texture data to GPU
    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = g_texture_width;
    t_desc.height = g_texture_height;
    t_desc.num_comps = 4;
    t_desc.data = g_ui_buffer;
    gfx->update_texture_data(&g_tex_ui, t_desc);

    return interaction;
}

static neko_vec2 last_point;

void render_test() {

    // Graphics api instance
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    // Platform api instance
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    neko_command_buffer_t *cb = &g_cb;

    unsigned char *alpha = neko_tex_rgba_to_alpha((neko_tex_uc *)g_texture_buffer, g_texture_width, g_texture_height);
    unsigned char *thresholded = neko_tex_alpha_to_thresholded(alpha, g_texture_width, g_texture_height, 90);
    unsigned char *outlined = neko_tex_thresholded_to_outlined(thresholded, g_texture_width, g_texture_height);
    neko_safe_free(alpha);
    neko_safe_free(thresholded);

    neko_tex_point *outline = neko_tex_extract_outline_path(outlined, g_texture_width, g_texture_height, &l, 0);
    while (l) {
        int l0 = l;
        neko_tex_distance_based_path_simplification(outline, &l, 0.5f);
        // printf("simplified outline: %d -> %d\n", l0, l);

        l_check = l;

        for (int i = 0; i < l; i++) {
            gfx->immediate.draw_line_ext(cb, neko_vec2_mul(last_point, {4.f, 4.f}), neko_vec2_mul({(f32)outline[i].x, (f32)outline[i].y}, {4.f, 4.f}), 2.f, neko_color_white);

            last_point = {(f32)outline[i].x, (f32)outline[i].y};
        }

        outline = neko_tex_extract_outline_path(outlined, g_texture_width, g_texture_height, &l, outline);
    };

    neko_safe_free(outline);
    neko_safe_free(outlined);
}

void render_scene() {
    // Graphics api instance
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    // Platform api instance
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    neko_command_buffer_t *cb = &g_cb;

    const f32 _t = platform->elapsed_time();

    // Upload our updated texture data to GPU
    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = g_texture_width;
    t_desc.height = g_texture_height;
    t_desc.num_comps = 4;
    t_desc.data = g_texture_buffer;
    gfx->update_texture_data(&g_tex, t_desc);

    const neko_vec2 ws = platform->window_size(g_window);
    neko_vec2 fbs = platform->frame_buffer_size(g_window);
    b32 flip_y = false;

    // Default state set up
    gfx->set_depth_enabled(cb, false);
    gfx->set_face_culling(cb, neko_face_culling_disabled);

    // Bind our render target and render offscreen
    gfx->bind_frame_buffer(cb, g_fb);
    {
        // Bind frame buffer attachment for rendering
        gfx->set_frame_buffer_attachment(cb, g_rt, 0);

        // Set clear color and clear screen
        f32 clear_color[4] = {0.1f, 0.1f, 0.1f, 1.f};
        gfx->set_view_clear(cb, clear_color);

        // This is to handle mac's retina high dpi for now until I fix that internally.
        gfx->set_viewport(cb, 0.f, 0.f, g_texture_width, g_texture_height);
        gfx->bind_shader(cb, g_shader);
        gfx->bind_uniform(cb, u_flip_y, &flip_y);
        gfx->bind_vertex_buffer(cb, g_vbo);
        gfx->bind_index_buffer(cb, g_ibo);
        gfx->bind_texture(cb, u_tex, g_tex, 0);
        gfx->draw_indexed(cb, 6, 0);
    }
    // Unbind offscreen buffer
    gfx->unbind_frame_buffer(cb);

    // Bind frame buffer for post processing
    gfx->bind_frame_buffer(cb, g_fb);
    {
        // Brightness pass
        {
            bright_filter_pass_parameters_t params = bright_filter_pass_parameters_t{g_rt};
            render_pass_i *p = neko_cast(render_pass_i, &g_bright_pass);
            p->pass(cb, p, &params);
        }

        // Blur pass
        {
            blur_pass_parameters_t params = blur_pass_parameters_t{g_bright_pass.data.render_target};
            render_pass_i *p = neko_cast(render_pass_i, &g_blur_pass);
            p->pass(cb, p, &params);
        }

        // composite pass w/ gamma correction
        {
            composite_pass_parameters_t params = composite_pass_parameters_t{g_rt, g_blur_pass.data.blur_render_target_b};
            render_pass_i *p = neko_cast(render_pass_i, &g_composite_pass);
            p->pass(cb, p, &params);
        }
    }
    gfx->unbind_frame_buffer(cb);

    // Back buffer Presentation
    {
        // Set clear color and clear screen
        f32 clear_color[4] = {0.2f, 0.2f, 0.2f, 1.f};
        gfx->set_view_clear(cb, clear_color);
        gfx->set_depth_enabled(cb, false);

        // This is to handle mac's retina high dpi for now until I fix that internally.
#if (defined NEKO_PLATFORM_APPLE)
        gfx->set_viewport(cb, (s32)ws.x * 2, (s32)ws.y * 2);
#else
        gfx->set_viewport(cb, 0.f, 0.f, (s32)ws.x, (s32)ws.y);
#endif

        f32 t = neko_engine_instance()->ctx.platform->elapsed_time() * neko_engine_instance()->ctx.platform->time.delta * 0.001f;
        flip_y = true;

        gfx->bind_shader(cb, g_shader);
        gfx->bind_uniform(cb, u_flip_y, &flip_y);
        gfx->bind_vertex_buffer(cb, g_vbo);
        gfx->bind_index_buffer(cb, g_ibo);

        // Draw final composited image
        if (g_use_post_processing) {

            gfx->bind_texture(cb, u_tex, g_composite_pass.data.render_target, 0);
        } else {

            gfx->bind_texture(cb, u_tex, g_rt, 0);
        }
        gfx->draw_indexed(cb, 6, 0);

        // Draw UI on top
        gfx->bind_texture(cb, u_tex, g_tex_ui, 0);
        gfx->draw_indexed(cb, 6, 0);
    }

    gfx->immediate.begin_drawing(cb);
    {
        // Draw 2d textured rect

        // gfx->immediate.draw_rect_textured(cb, {200.f, 200.f}, {600.f, 600.f}, g_sr.sprite->img.id, neko_color_white);

        s32 index;
        if (g_sr.loop) {
            index = g_sr.loop->indices[g_sr.current_frame];
        } else {
            index = g_sr.current_frame;
        }

        neko_sprite *spr = g_sr.sprite;
        neko_sprite_frame f = spr->frames[index];

        gfx->immediate.draw_rect_textured_ext(cb, 200.f, 200.f, 600.f, 600.f, f.u0, f.v0, f.u1, f.v1, g_sr.sprite->img.id, neko_color_white);

        gfx->immediate.begin_2d(cb);
        {
            swarm.draw(cb);

            // gfx->immediate.draw_line_ext(cb, {9.f, 9.f}, {400.f, 400.f}, 4.f, neko_color_white);
            // render_test();
        }
        gfx->immediate.end_2d(cb);
    }
    gfx->immediate.end_drawing(cb);

#if 0  // simple_immediate_rendering
    // neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    static f32 _z = -3.f;
    if (platform->key_pressed(neko_keycode_e)) {
        _z -= 0.1f;
    }
    if (platform->key_pressed(neko_keycode_q)) {
        _z += 0.1f;
    }

    // neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    // neko_command_buffer_t *cb = &g_cb;

    gfx->immediate.begin_drawing(cb);
    {
        gfx->immediate.clear(cb, 0.1f, 0.1f, 0.1f, 1.f);

        // Draw 2d textured rect
        for (s32 i = -5; i < 5; ++i) {
            f32 uv = (sinf(_t * i * 0.0001f) * 0.5f + 0.5f) * (10.f + i);
            neko_vec2 s = neko_v2(i * 200.f, (ws.y - 200.f) * 0.5f);
            neko_vec2 e = neko_v2(s.x + 200.f, s.y + 200.f);
            gfx->immediate.draw_rect_textured_ext(cb, s.x, s.y, e.x, e.y, 0.f, 0.f, uv, uv, g_test_ase.id, neko_color_white);
        }

        neko_camera_t cam = gfx->immediate.begin_3d(cb);
        {
            neko_vqs box_xform = neko_vqs_default();
            neko_vqs local_xform = neko_vqs_default();

            gfx->immediate.push_matrix(cb, neko_matrix_model);
            {
                neko_vec3 bp = neko_v3(0.f, 0.f, 0.f);
                neko_vec3 he = neko_v3_s(0.5f);

                box_xform.position = neko_v3(0.f, 0.f, _z);
                box_xform.rotation =
                        neko_quat_mul_list(3, neko_quat_angle_axis(_t * 0.001f, neko_y_axis), neko_quat_angle_axis(_t * 0.001f, neko_x_axis), neko_quat_angle_axis(_t * 0.001f, neko_z_axis));
                gfx->immediate.mat_mul_vqs(cb, box_xform);
                gfx->immediate.draw_box_textured(cb, bp, he, g_test_ase.id, neko_color_white);

                // Draw bounding box surrounding it
                gfx->immediate.draw_box_lines(cb, bp, neko_vec3_add(he, neko_v3_s(0.01f)), neko_color_white);
                gfx->immediate.draw_sphere(cb, bp, 0.45f, neko_color_alpha(neko_color_white, 100));
                gfx->immediate.draw_sphere_lines(cb, bp, 0.46f, neko_color_white);

                // Draw coordinate axis of box
                gfx->immediate.draw_line_3d(cb, neko_v3(0.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f), neko_color_red);
                gfx->immediate.draw_line_3d(cb, neko_v3(0.f, 0.f, 0.f), neko_v3(0.f, 1.f, 0.f), neko_color_green);
                gfx->immediate.draw_line_3d(cb, neko_v3(0.f, 0.f, 0.f), neko_v3(0.f, 0.f, 1.f), neko_color_blue);
            }
            gfx->immediate.pop_matrix(cb);

            f32 rs[3] = {0.6f, 1.f, 2.f};

            neko_color_t colors[3] = {neko_color_purple, neko_color_blue, neko_color_orange};

            f32 ts0 = _t * 0.0001f;
            f32 ts1 = _t * 0.0004f;
            f32 ts2 = _t * 0.0006f;
            neko_vec3 planet_positions[3] = {neko_vec3_add(box_xform.position, neko_vec3_scale(neko_v3(cos(ts0), sin(ts0), cos(ts0)), rs[0])),
                                             neko_vec3_add(box_xform.position, neko_vec3_scale(neko_v3(cos(ts1), sin(ts1), cos(ts1)), rs[1])),
                                             neko_vec3_add(box_xform.position, neko_vec3_scale(neko_v3(cos(ts2), sin(ts2), cos(ts2)), rs[2]))};

            // Planet
            neko_for_range_i(3) {
                gfx->immediate.push_matrix(cb, neko_matrix_model);
                {
                    local_xform.position = neko_vec3_add(planet_positions[(i + 1) % 3], planet_positions[i]);
                    local_xform.rotation = neko_quat_angle_axis(_t * 0.001f, neko_y_axis);
                    gfx->immediate.mat_mul_vqs(cb, local_xform);
                    gfx->immediate.draw_sphere(cb, neko_v3(0.f, 0.f, 0.f), 0.1f, colors[i]);
                    gfx->immediate.draw_sphere_lines(cb, neko_v3(0.f, 0.f, 0.f), 0.15f, neko_color_white);
                }
                gfx->immediate.pop_matrix(cb);
            }

            const f32 ts = 0.001f;
            neko_vqs local_xforms[3] = {neko_vqs_ctor(neko_v3(-0.2f, 0.5f, 0.f), neko_quat_angle_axis(neko_deg_to_rad(-180.f), neko_x_axis), neko_v3_s(ts)),
                                        neko_vqs_ctor(neko_v3(-0.1f, 0.0f, 0.5f), neko_quat_angle_axis(neko_deg_to_rad(-180.f), neko_x_axis), neko_v3_s(ts)),
                                        neko_vqs_ctor(neko_v3(0.5f, 0.0f, 0.f), neko_quat_angle_axis(neko_deg_to_rad(-180.f), neko_x_axis), neko_v3_s(ts))};

            // Make our text a child transform of the box
            gfx->immediate.push_state_attr(cb, neko_face_culling, neko_face_culling_disabled);
            {
                neko_snprintfc(rot_buffer, 256, "rotation: %.2f, %.2f, %.2f, %.2f", box_xform.rotation.x, box_xform.rotation.y, box_xform.rotation.z, box_xform.rotation.w);
                neko_snprintfc(trans_buffer, 256, "trans: %.2f, %.2f, %.2f", box_xform.position.x, box_xform.position.y, box_xform.position.z);
                neko_snprintfc(scale_buffer, 256, "scale: %.2f, %.2f, %.2f", box_xform.scale.x, box_xform.scale.y, box_xform.scale.z);

                char *buffers[3] = {rot_buffer, trans_buffer, scale_buffer};

                neko_for_range_i(3) {
                    gfx->immediate.push_matrix(cb, neko_matrix_model);
                    {
                        gfx->immediate.mat_mul_vqs(cb, neko_vqs_absolute_transform(&local_xforms[i], &box_xform));
                        gfx->immediate.draw_text_ext(cb, 0.f, 0.f, buffers[i], g_test_font, neko_color_green);
                    }
                    gfx->immediate.pop_matrix(cb);
                }
            }
            gfx->immediate.pop_state_attr(cb);
        }
        gfx->immediate.end_3d(cb);

        neko_snprintfc(fps_text, 256, "fps: %.2f", 1000.f / platform->time.frame);
        gfx->immediate.draw_text(cb, 10.f, 20.f, fps_text, neko_color_white);
    }
    gfx->immediate.end_drawing(cb);
#endif

    // Submit command buffer for rendering
    gfx->submit_command_buffer(cb);
}

void write_data(u32 idx, particle_t p) {
    // Write into particle data for id value
    g_world_particle_data[idx] = p;
    g_texture_buffer[idx] = p.color;
}

void update_salt(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 2;
    u32 spread_rate = 5;
    s32 lx, ly;

    p->velocity.y = neko_clamp(p->velocity.y + (gravity * dt), -10.f, 10.f);

    p->has_been_updated_this_frame = true;

    // If in liquid, chance to dissolve itself.
    if (is_in_liquid(x, y, &lx, &ly)) {
        if (random_val(0, 1000) == 0) {
            write_data(read_idx, particle_empty());
            return;
        }
    }

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    // if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && get_particle_at(x, y + 1).id != mat_id_water) {
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1)) {
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

    if (in_bounds(x + vx, y + vy) && (is_empty(x + vx, y + vy))) {
        write_data(v_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + 1) && ((is_empty(x, y + 1)))) {
        u32 idx = compute_idx(x, y + 1);
        p->velocity.y += (gravity * dt);
        particle_t tmp_a = g_world_particle_data[read_idx];
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, tmp_a);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && (is_empty(x - 1, y + 1))) {
        u32 idx = compute_idx(x - 1, y + 1);
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_a = g_world_particle_data[read_idx];
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, tmp_a);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && (is_empty(x + 1, y + 1))) {
        u32 idx = compute_idx(x + 1, y + 1);
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_a = g_world_particle_data[read_idx];
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, tmp_a);
        write_data(read_idx, tmp_b);
    }
}

void update_sand(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    p->velocity.y = neko_clamp(p->velocity.y + (gravity * dt), -10.f, 10.f);

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && get_particle_at(x, y + 1).id != mat_id_water) {
        p->velocity.y /= 2.f;
    }

    s32 vi_x = x + (s32)p->velocity.x;
    s32 vi_y = y + (s32)p->velocity.y;

    // Check to see if you can swap first with other element below you
    u32 b_idx = compute_idx(x, y + 1);
    u32 br_idx = compute_idx(x + 1, y + 1);
    u32 bl_idx = compute_idx(x - 1, y + 1);

    s32 lx, ly;

    particle_t tmp_a = g_world_particle_data[read_idx];

    // Physics (using velocity)
    if (in_bounds(vi_x, vi_y) &&
        ((is_empty(vi_x, vi_y) || (((g_world_particle_data[compute_idx(vi_x, vi_y)].id == mat_id_water) && !g_world_particle_data[compute_idx(vi_x, vi_y)].has_been_updated_this_frame &&
                                    neko_vec2_len(g_world_particle_data[compute_idx(vi_x, vi_y)].velocity) - neko_vec2_len(tmp_a.velocity) > 10.f))))) {

        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];

        // Try to throw water out
        if (tmp_b.id == mat_id_water) {

            s32 rx = random_val(-2, 2);
            tmp_b.velocity = neko_vec2{(f32)rx, -4.f};

            write_data(compute_idx(vi_x, vi_y), tmp_a);

            for (s32 i = -10; i < 0; ++i) {
                for (s32 j = -10; j < 10; ++j) {
                    if (is_empty(vi_x + j, vi_y + i)) {
                        write_data(compute_idx(vi_x + j, vi_y + i), tmp_b);
                        break;
                    }
                }
            }

            // Couldn't write there, so, uh, destroy it.
            write_data(read_idx, particle_empty());
        } else if (is_empty(vi_x, vi_y)) {
            write_data(compute_idx(vi_x, vi_y), tmp_a);
            write_data(read_idx, tmp_b);
        }
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + 1) && ((is_empty(x, y + 1) || (g_world_particle_data[b_idx].id == mat_id_water)))) {
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + 1);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && ((is_empty(x - 1, y + 1) || g_world_particle_data[bl_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y + 1);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && ((is_empty(x + 1, y + 1) || g_world_particle_data[br_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x + 1, y + 1);
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    }
}

void update_gunpowder(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    p->velocity.y = neko_clamp(p->velocity.y + (gravity * dt), -10.f, 10.f);
    // p->velocity.x = neko_clamp(p->velocity.x, -5.f, 5.f);

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && get_particle_at(x, y + 1).id != mat_id_water) {
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

    particle_t tmp_a = g_world_particle_data[read_idx];

    // Physics (using velocity)
    if (in_bounds(vi_x, vi_y) &&
        ((is_empty(vi_x, vi_y) || (((g_world_particle_data[compute_idx(vi_x, vi_y)].id == mat_id_water) && !g_world_particle_data[compute_idx(vi_x, vi_y)].has_been_updated_this_frame &&
                                    neko_vec2_len(g_world_particle_data[compute_idx(vi_x, vi_y)].velocity) - neko_vec2_len(tmp_a.velocity) > 10.f))))) {

        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];

        // Try to throw water out
        if (tmp_b.id == mat_id_water) {

            s32 rx = random_val(-2, 2);
            tmp_b.velocity = neko_vec2{(f32)rx, -4.f};

            write_data(compute_idx(vi_x, vi_y), tmp_a);

            for (s32 i = -10; i < 0; ++i) {
                for (s32 j = -10; j < 10; ++j) {
                    if (is_empty(vi_x + j, vi_y + i)) {
                        write_data(compute_idx(vi_x + j, vi_y + i), tmp_b);
                        break;
                    }
                }
            }

            // Couldn't write there, so, uh, destroy it.
            write_data(read_idx, particle_empty());
        } else if (is_empty(vi_x, vi_y)) {
            write_data(compute_idx(vi_x, vi_y), tmp_a);
            write_data(read_idx, tmp_b);
        }
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + 1) && ((is_empty(x, y + 1) || (g_world_particle_data[b_idx].id == mat_id_water)))) {
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + 1);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && ((is_empty(x - 1, y + 1) || g_world_particle_data[bl_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y + 1);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && ((is_empty(x + 1, y + 1) || g_world_particle_data[br_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x + 1, y + 1);
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    }
}

void update_steam(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    if (p->life_time > 10.f) {
        write_data(read_idx, particle_empty());
        return;
    }

    if (p->has_been_updated_this_frame) {
        return;
    }

    p->has_been_updated_this_frame = true;

    // Smoke rises over time. This might cause issues, actually...
    p->velocity.y = neko_clamp(p->velocity.y - (gravity * dt), -2.f, 10.f);
    p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 100.f, -1.f, 1.f);

    // Change color based on life_time
    p->color.r = (u8)(neko_clamp((neko_interp_linear(0.f, 10.f, p->life_time) / 10.f) * 255.f, 150.f, 255.f));
    p->color.g = (u8)(neko_clamp((neko_interp_linear(0.f, 10.f, p->life_time) / 10.f) * 255.f, 150.f, 255.f));
    p->color.b = (u8)(neko_clamp((neko_interp_linear(0.f, 10.f, p->life_time) / 10.f) * 255.f, 150.f, 255.f));
    p->color.a = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time) / 10.f) * 255.f, 10.f, 255.f));

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y - 1) && !is_empty(x, y - 1) && get_particle_at(x, y - 1).id != mat_id_water) {
        p->velocity.y /= 2.f;
    }

    s32 vi_x = x + (s32)p->velocity.x;
    s32 vi_y = y + (s32)p->velocity.y;

    if (in_bounds(vi_x, vi_y) && ((is_empty(vi_x, vi_y) || get_particle_at(vi_x, vi_y).id == mat_id_water || get_particle_at(vi_x, vi_y).id == mat_id_fire))) {

        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];

        // Try to throw water out
        if (tmp_b.id == mat_id_water) {

            tmp_b.has_been_updated_this_frame = true;

            s32 rx = random_val(-2, 2);
            tmp_b.velocity = neko_vec2{(f32)rx, -3.f};

            write_data(compute_idx(vi_x, vi_y), *p);
            write_data(read_idx, tmp_b);
        } else if (is_empty(vi_x, vi_y)) {
            write_data(compute_idx(vi_x, vi_y), *p);
            write_data(read_idx, tmp_b);
        }
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y - 1) && ((is_empty(x, y - 1) || (get_particle_at(x, y - 1).id == mat_id_water) || get_particle_at(x, y - 1).id == mat_id_fire))) {
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y - 1);
        write_data(compute_idx(x, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && ((is_empty(x - 1, y - 1) || get_particle_at(x - 1, y - 1).id == mat_id_water) || get_particle_at(x - 1, y - 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y - 1);
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && ((is_empty(x + 1, y - 1) || get_particle_at(x + 1, y - 1).id == mat_id_water) || get_particle_at(x + 1, y - 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x + 1, y - 1);
        write_data(compute_idx(x + 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    }
    // Can move if in liquid
    else if (in_bounds(x + 1, y) && (get_particle_at(x + 1, y).id == mat_id_water)) {
        u32 idx = compute_idx(x + 1, y);
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y) && (g_world_particle_data[compute_idx(x - 1, y)].id == mat_id_water)) {
        u32 idx = compute_idx(x - 1, y);
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else {
        write_data(read_idx, *p);
    }
}

void update_smoke(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    if (p->life_time > 10.f) {
        write_data(read_idx, particle_empty());
        return;
    }

    if (p->has_been_updated_this_frame) {
        return;
    }

    p->has_been_updated_this_frame = true;

    // Smoke rises over time. This might cause issues, actually...
    p->velocity.y = neko_clamp(p->velocity.y - (gravity * dt), -2.f, 10.f);
    p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 100.f, -1.f, 1.f);

    // Change color based on life_time
    p->color.r = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time * 0.5f) / 10.f) * 150.f, 0.f, 150.f));
    p->color.g = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time * 0.5f) / 10.f) * 120.f, 0.f, 120.f));
    p->color.b = (u8)(neko_clamp((neko_interp_linear(10.f, 0.f, p->life_time * 0.5f) / 10.f) * 100.f, 0.f, 100.f));

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y - 1) && !is_empty(x, y - 1) && get_particle_at(x, y - 1).id != mat_id_water) {
        p->velocity.y /= 2.f;
    }

    s32 vi_x = x + (s32)p->velocity.x;
    s32 vi_y = y + (s32)p->velocity.y;

    // if (in_bounds(vi_x, vi_y) && ((is_empty(vi_x, vi_y) || get_particle_at(vi_x, vi_y).id == mat_id_water || get_particle_at(vi_x, vi_y).id == mat_id_fire))) {
    if (in_bounds(vi_x, vi_y) && get_particle_at(vi_x, vi_y).id != mat_id_smoke) {

        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];

        // Try to throw water out
        if (tmp_b.id == mat_id_water) {

            tmp_b.has_been_updated_this_frame = true;

            s32 rx = random_val(-2, 2);
            tmp_b.velocity = neko_vec2{(f32)rx, -3.f};

            write_data(compute_idx(vi_x, vi_y), *p);
            write_data(read_idx, tmp_b);
        } else if (is_empty(vi_x, vi_y)) {
            write_data(compute_idx(vi_x, vi_y), *p);
            write_data(read_idx, tmp_b);
        }
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y - 1) && get_particle_at(x, y - 1).id != mat_id_smoke && get_particle_at(x, y - 1).id != mat_id_wood && get_particle_at(x, y - 1).id != mat_id_stone) {
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y - 1);
        write_data(compute_idx(x, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && get_particle_at(x - 1, y - 1).id != mat_id_smoke && get_particle_at(x - 1, y - 1).id != mat_id_wood && get_particle_at(x - 1, y - 1).id != mat_id_stone) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y - 1);
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && get_particle_at(x + 1, y - 1).id != mat_id_smoke && get_particle_at(x + 1, y - 1).id != mat_id_wood && get_particle_at(x + 1, y - 1).id != mat_id_stone) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x + 1, y - 1);
        write_data(compute_idx(x + 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    }
    // Can move if in liquid
    else if (in_bounds(x + 1, y) && get_particle_at(x + 1, y).id != mat_id_smoke && get_particle_at(x + 1, y).id != mat_id_wood && get_particle_at(x + 1, y).id != mat_id_stone) {
        u32 idx = compute_idx(x + 1, y);
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y) && get_particle_at(x - 1, y).id != mat_id_smoke && get_particle_at(x - 1, y).id != mat_id_wood && get_particle_at(x - 1, y).id != mat_id_stone) {
        u32 idx = compute_idx(x - 1, y);
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else {
        write_data(read_idx, *p);
    }
}

void update_ember(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    if (p->life_time > 0.5f) {
        write_data(read_idx, particle_empty());
        return;
    }

    if (p->has_been_updated_this_frame) {
        return;
    }

    p->has_been_updated_this_frame = true;

    p->velocity.y = neko_clamp(p->velocity.y - (gravity * dt), -2.f, 10.f);
    p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-100, 100) / 100.f, -1.f, 1.f);

    // If directly on top of some wall, then replace it
    if (in_bounds(x, y + 1) && get_particle_at(x, y + 1).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x, y + 1), particle_fire());
    } else if (in_bounds(x + 1, y + 1) && get_particle_at(x + 1, y + 1).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x + 1, y + 1), particle_fire());
    } else if (in_bounds(x - 1, y + 1) && get_particle_at(x - 1, y + 1).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x - 1, y + 1), particle_fire());
    } else if (in_bounds(x - 1, y) && get_particle_at(x - 1, y).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x - 1, y), particle_fire());
    } else if (in_bounds(x + 1, y) && get_particle_at(x + 1, y).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x + 1, y), particle_fire());
    } else if (in_bounds(x + 1, y - 1) && get_particle_at(x + 1, y - 1).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x + 1, y - 1), particle_fire());
    } else if (in_bounds(x - 1, y - 1) && get_particle_at(x - 1, y - 1).id == mat_id_wood && random_val(0, 200) == 0) {
        write_data(compute_idx(x - 1, y - 1), particle_fire());
    }

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y - 1) && !is_empty(x, y - 1) && get_particle_at(x, y - 1).id != mat_id_water) {
        p->velocity.y /= 2.f;
    }

    s32 vi_x = x + (s32)p->velocity.x;
    s32 vi_y = y + (s32)p->velocity.y;

    if (in_bounds(vi_x, vi_y) && (is_empty(vi_x, vi_y) || get_particle_at(vi_x, vi_y).id == mat_id_water || get_particle_at(vi_x, vi_y).id == mat_id_fire ||
                                  get_particle_at(vi_x, vi_y).id == mat_id_smoke || get_particle_at(vi_x, vi_y).id == mat_id_ember)) {

        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];

        // Try to throw water out
        if (tmp_b.id == mat_id_water) {

            tmp_b.has_been_updated_this_frame = true;

            s32 rx = random_val(-2, 2);
            tmp_b.velocity = neko_vec2{(f32)rx, -3.f};

            write_data(compute_idx(vi_x, vi_y), *p);
            write_data(read_idx, tmp_b);
        } else if (is_empty(vi_x, vi_y)) {
            write_data(compute_idx(vi_x, vi_y), *p);
            write_data(read_idx, tmp_b);
        }
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y - 1) && ((is_empty(x, y - 1) || (get_particle_at(x, y - 1).id == mat_id_water) || get_particle_at(x, y - 1).id == mat_id_fire))) {
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y - 1);
        write_data(compute_idx(x, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && ((is_empty(x - 1, y - 1) || get_particle_at(x - 1, y - 1).id == mat_id_water) || get_particle_at(x - 1, y - 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y - 1);
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && ((is_empty(x + 1, y - 1) || get_particle_at(x + 1, y - 1).id == mat_id_water) || get_particle_at(x + 1, y + 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (gravity * dt);
        particle_t tmp_b = get_particle_at(x + 1, y - 1);
        write_data(compute_idx(x + 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    }
    // Can move if in liquid
    else if (in_bounds(x + 1, y) && (is_empty(x + 1, y) || get_particle_at(x + 1, y).id == mat_id_fire)) {
        u32 idx = compute_idx(x + 1, y);
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y) && (is_empty(x - 1, y) || get_particle_at(x - 1, y).id == mat_id_fire)) {
        u32 idx = compute_idx(x - 1, y);
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else {
        write_data(read_idx, *p);
    }
}

void update_fire(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    if (p->has_been_updated_this_frame) {
        return;
    }

    p->has_been_updated_this_frame = true;

    if (p->life_time > 0.2f) {
        if (random_val(0, 100) == 0) {
            write_data(read_idx, particle_empty());
            return;
        }
    }

    f32 st = sin(neko_engine_instance()->ctx.platform->elapsed_time());
    // f32 grav_mul = random_val(0, 10) == 0 ? 2.f : 1.f;
    p->velocity.y = neko_clamp(p->velocity.y - ((gravity * dt)) * 0.2f, -5.0f, 0.f);
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
    if (is_in_water(x, y, &lx, &ly)) {
        if (random_val(0, 1) == 0) {
            s32 ry = random_val(-5, -1);
            s32 rx = random_val(-5, 5);
            for (s32 i = ry; i > -5; --i) {
                for (s32 j = rx; j < 5; ++j) {
                    particle_t p = particle_steam();
                    if (in_bounds(x + j, y + i) && is_empty(x + j, y + i)) {
                        particle_t p = particle_steam();
                        write_data(compute_idx(x + j, y + i), p);
                    }
                }
            }
            particle_t p = particle_steam();
            write_data(read_idx, particle_empty());
            write_data(read_idx, p);
            write_data(compute_idx(lx, ly), particle_empty());
            return;
        }
    }

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && (get_particle_at(x, y + 1).id != mat_id_water || get_particle_at(x, y + 1).id != mat_id_smoke)) {
        p->velocity.y /= 2.f;
    }

    if (random_val(0, 10) == 0) {
        // p->velocity.x = neko_clamp(p->velocity.x + (f32)random_val(-1, 1) / 2.f, -1.f, 1.f);
    }
    // p->velocity.x = neko_clamp(p->velocity.x, -0.5f, 0.5f);

    // Kill fire underneath
    if (in_bounds(x, y + 3) && get_particle_at(x, y + 3).id == mat_id_fire && random_val(0, 100) == 0) {
        write_data(compute_idx(x, y + 3), *p);
        write_data(read_idx, particle_empty());
        return;
    }

    // Chance to kick itself up (to simulate flames)
    if (in_bounds(x, y + 1) && get_particle_at(x, y + 1).id == mat_id_fire && in_bounds(x, y - 1) && get_particle_at(x, y - 1).id == mat_id_empty) {
        if (random_val(0, 10) == 0 * p->life_time < 10.f && p->life_time > 1.f) {
            s32 r = random_val(0, 1);
            s32 rh = random_val(-10, -1);
            s32 spread = 3;
            for (s32 i = rh; i < 0; ++i) {
                for (s32 j = r ? -spread : spread; r ? j < spread : j > -spread; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
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
            if (in_bounds(x, y - 1) && is_empty(x, y - 1)) {
                write_data(compute_idx(x, y - 1), particle_smoke());
            } else if (in_bounds(x + 1, y - 1) && is_empty(x + 1, y - 1)) {
                write_data(compute_idx(x + 1, y - 1), particle_smoke());
            } else if (in_bounds(x - 1, y - 1) && is_empty(x - 1, y - 1)) {
                write_data(compute_idx(x - 1, y - 1), particle_smoke());
            }
        }
    }

    // Spawn embers
    if (random_val(0, 250) == 0 && p->life_time < 3.f) {
        for (u32 i = 0; i < random_val(1, 100); ++i) {
            if (in_bounds(x, y - 1) && is_empty(x, y - 1)) {
                particle_t e = particle_ember();
                e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f};
                write_data(compute_idx(x, y - 1), e);
            } else if (in_bounds(x + 1, y - 1) && is_empty(x + 1, y - 1)) {
                particle_t e = particle_ember();
                e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f};
                write_data(compute_idx(x + 1, y - 1), e);
            } else if (in_bounds(x - 1, y - 1) && is_empty(x - 1, y - 1)) {
                particle_t e = particle_ember();
                e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f};
                write_data(compute_idx(x - 1, y - 1), e);
            }
        }
    }

    // If directly on top of some wall, then replace it
    if (in_bounds(x, y + 1) &&
        ((get_particle_at(x, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x, y + 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
         (get_particle_at(x, y + 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                 )) {
        write_data(compute_idx(x, y + 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x + 1, y + 1) &&
               ((get_particle_at(x + 1, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x + 1, y + 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x + 1, y + 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y + 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x - 1, y + 1) &&
               ((get_particle_at(x - 1, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x - 1, y + 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x - 1, y + 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y + 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x - 1, y) &&
               ((get_particle_at(x - 1, y).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x - 1, y).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                (get_particle_at(x - 1, y).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x + 1, y) &&
               ((get_particle_at(x + 1, y).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x + 1, y).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                (get_particle_at(x + 1, y).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x + 1, y - 1) &&
               ((get_particle_at(x + 1, y - 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x + 1, y - 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x + 1, y - 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y - 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x - 1, y - 1) &&
               ((get_particle_at(x - 1, y - 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x - 1, y - 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x - 1, y - 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y - 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        // particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), *p);
                        write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x, y - 1) && is_empty(x, y - 1)) {
        if (random_val(0, 50) == 0) {
            write_data(read_idx, particle_empty());
            return;
        }
    }

    if (in_bounds(vi_x, vi_y) && (is_empty(vi_x, vi_y) || get_particle_at(vi_x, vi_y).id == mat_id_fire || get_particle_at(vi_x, vi_y).id == mat_id_smoke)) {
        // p->velocity.y -= (gravity * dt);
        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];
        write_data(compute_idx(vi_x, vi_y), *p);
        write_data(read_idx, tmp_b);
    }

    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + 1) && ((is_empty(x, y + 1) || (g_world_particle_data[b_idx].id == mat_id_water)))) {
        // p->velocity.y -= (gravity * dt);
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        particle_t tmp_b = g_world_particle_data[b_idx];
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && ((is_empty(x - 1, y + 1) || g_world_particle_data[bl_idx].id == mat_id_water))) {
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        // p->velocity.y -= (gravity * dt);
        particle_t tmp_b = g_world_particle_data[bl_idx];
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && ((is_empty(x + 1, y + 1) || g_world_particle_data[br_idx].id == mat_id_water))) {
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        // p->velocity.y -= (gravity * dt);
        particle_t tmp_b = g_world_particle_data[br_idx];
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && (g_world_particle_data[compute_idx(x - 1, y - 1)].id == mat_id_water)) {
        u32 idx = compute_idx(x - 1, y - 1);
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && (g_world_particle_data[compute_idx(x + 1, y - 1)].id == mat_id_water)) {
        u32 idx = compute_idx(x + 1, y - 1);
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x, y - 1) && (g_world_particle_data[compute_idx(x, y - 1)].id == mat_id_water)) {
        u32 idx = compute_idx(x, y - 1);
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, *p);
        write_data(read_idx, tmp_b);
    } else {
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        write_data(read_idx, *p);
    }
}

void update_lava(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    // For water, same as sand, but we'll check immediate left and right as well
    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 4;

    if (p->has_been_updated_this_frame) {
        return;
    }

    p->has_been_updated_this_frame = true;

    p->velocity.y = neko_clamp(p->velocity.y + ((gravity * dt)), -10.f, 10.f);

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
    if (is_in_water(x, y, &lx, &ly)) {
        if (random_val(0, 1) == 0) {
            s32 ry = random_val(-5, -1);
            s32 rx = random_val(-5, 5);
            for (s32 i = ry; i > -5; --i) {
                for (s32 j = rx; j < 5; ++j) {
                    particle_t p = particle_steam();
                    if (in_bounds(x + j, y + i) && is_empty(x + j, y + i)) {
                        particle_t p = particle_steam();
                        write_data(compute_idx(x + j, y + i), p);
                    }
                }
            }
            particle_t p = particle_steam();
            write_data(read_idx, particle_empty());
            write_data(read_idx, p);
            write_data(compute_idx(lx, ly), particle_stone());
            return;
        }
    }

    // Otherwise destroy anything that isn't fire or lava...eventually...

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && (get_particle_at(x, y + 1).id != mat_id_water || get_particle_at(x, y + 1).id != mat_id_smoke)) {
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
            if (in_bounds(x, y - 1) && is_empty(x, y - 1)) {
                write_data(compute_idx(x, y - 1), particle_smoke());
            } else if (in_bounds(x + 1, y - 1) && is_empty(x + 1, y - 1)) {
                write_data(compute_idx(x + 1, y - 1), particle_smoke());
            } else if (in_bounds(x - 1, y - 1) && is_empty(x - 1, y - 1)) {
                write_data(compute_idx(x - 1, y - 1), particle_smoke());
            }
        }
    }

    // Spawn embers
    if (random_val(0, 250) == 0 && p->life_time < 3.f) {
        for (u32 i = 0; i < random_val(1, 100); ++i) {
            if (in_bounds(x, y - 1) && is_empty(x, y - 1)) {
                particle_t e = particle_ember();
                e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f};
                write_data(compute_idx(x, y - 1), e);
            } else if (in_bounds(x + 1, y - 1) && is_empty(x + 1, y - 1)) {
                particle_t e = particle_ember();
                e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f};
                write_data(compute_idx(x + 1, y - 1), e);
            } else if (in_bounds(x - 1, y - 1) && is_empty(x - 1, y - 1)) {
                particle_t e = particle_ember();
                e.velocity = neko_vec2{(f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f};
                write_data(compute_idx(x - 1, y - 1), e);
            }
        }
    }

    // If directly on top of some wall, then replace it
    if (in_bounds(x, y + 1) &&
        ((get_particle_at(x, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x, y + 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
         (get_particle_at(x, y + 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                 )) {
        write_data(compute_idx(x, y + 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        // write_data(read_idx, particle_empty());
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x + 1, y + 1) &&
               ((get_particle_at(x + 1, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x + 1, y + 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x + 1, y + 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y + 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x - 1, y + 1) &&
               ((get_particle_at(x - 1, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x - 1, y + 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x - 1, y + 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y + 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x - 1, y) &&
               ((get_particle_at(x - 1, y).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x - 1, y).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                (get_particle_at(x - 1, y).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x + 1, y) &&
               ((get_particle_at(x + 1, y).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x + 1, y).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
                (get_particle_at(x + 1, y).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x + 1, y - 1) &&
               ((get_particle_at(x + 1, y - 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x + 1, y - 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x + 1, y - 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y - 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        break;
                    }
                }
            }
        }
    } else if (in_bounds(x - 1, y - 1) &&
               ((get_particle_at(x - 1, y - 1).id == mat_id_wood && random_val(0, wood_chance) == 0) ||
                (get_particle_at(x - 1, y - 1).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) || (get_particle_at(x - 1, y - 1).id == mat_id_oil && random_val(0, oil_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y - 1), particle_fire());
        if (random_val(0, 5) == 0) {
            s32 r = random_val(0, 1);
            for (s32 i = -3; i < 2; ++i) {
                for (s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j) {
                    s32 rx = j, ry = i;
                    if (in_bounds(x + rx, y + ry) && is_empty(x + rx, y + ry)) {
                        particle_t fp = particle_fire();
                        p->life_time += 0.1f;
                        write_data(compute_idx(x + rx, y + ry), fp);
                        break;
                    }
                }
            }
        }
    }

    // If in water, then need to float upwards
    // s32 lx, ly;
    // if (is_in_liquid(x, y, &lx, &ly) && in_bounds(x, y - 1) && get_particle_at(x, y - 1).id == mat_id_water) {
    //  particle_t tmp = get_particle_at(x, y - 1);
    //  write_data(compute_idx(x, y - 1), *p);
    //  write_data(read_idx, tmp);
    //  // return;
    // }

    if (in_bounds(x + vx, y + vy) && (is_empty(x + vx, y + vy))) {
        write_data(v_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x, y + u)) {
        write_data(b_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + r, y + u)) {
        write_data(br_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + l, y + u)) {
        write_data(bl_idx, *p);
        write_data(read_idx, particle_empty());
    } else {
        particle_t tmp = *p;
        b32 found = false;

        for (u32 i = 0; i < fall_rate; ++i) {
            for (s32 j = spread_rate; j > 0; --j) {
                if (is_empty(x - j, y + i)) {
                    write_data(compute_idx(x - j, y + i), *p);
                    write_data(read_idx, particle_empty());
                    found = true;
                    break;
                } else if (is_empty(x + j, y + i)) {
                    write_data(compute_idx(x + j, y + i), *p);
                    write_data(read_idx, particle_empty());
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            write_data(read_idx, tmp);
        }
    }
}

void update_oil(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 2;
    s32 spread_rate = 4;

    p->velocity.y = neko_clamp(p->velocity.y + (gravity * dt), -10.f, 10.f);

    p->has_been_updated_this_frame = true;

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    // if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && get_particle_at(x, y + 1).id != mat_id_water) {
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1)) {
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

    // If in water, then need to float upwards
    // s32 lx, ly;
    // if (is_in_liquid(x, y, &lx, &ly) && in_bounds(x, y - 1) && get_particle_at(x, y - 1).id == mat_id_water) {
    //  particle_t tmp = get_particle_at(x, y - 1);
    //  write_data(compute_idx(x, y - 1), *p);
    //  write_data(read_idx, tmp);
    //  // return;
    // }

    if (in_bounds(x + vx, y + vy) && (is_empty(x + vx, y + vy))) {
        write_data(v_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x, y + u)) {
        write_data(b_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + r, y + u)) {
        write_data(br_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + l, y + u)) {
        write_data(bl_idx, *p);
        write_data(read_idx, particle_empty());
    } else {
        particle_t tmp = *p;
        b32 found = false;

        for (u32 i = 0; i < fall_rate; ++i) {
            for (s32 j = spread_rate; j > 0; --j) {
                if (is_empty(x - j, y + i)) {
                    write_data(compute_idx(x - j, y + i), *p);
                    write_data(read_idx, particle_empty());
                    found = true;
                    break;
                } else if (is_empty(x + j, y + i)) {
                    write_data(compute_idx(x + j, y + i), *p);
                    write_data(read_idx, particle_empty());
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            write_data(read_idx, tmp);
        }
    }
}

void update_acid(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 2;
    s32 spread_rate = 5;
    s32 lx, ly;

    p->velocity.y = neko_clamp(p->velocity.y + (gravity * dt), -10.f, 10.f);

    p->has_been_updated_this_frame = true;

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    // if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && get_particle_at(x, y + 1).id != mat_id_water) {
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1)) {
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
    if (is_in_water(x, y, &lx, &ly) && random_val(0, 250) == 0) {
        write_data(read_idx, particle_empty());
        return;
    }

    // If directly on top of some wall, then replace it
    if (in_bounds(x, y + 1) &&
        ((get_particle_at(x, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x, y + 1).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
         (get_particle_at(x, y + 1).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x, y + 1).id == mat_id_salt && random_val(0, salt_chance) == 0)

                 )) {
        write_data(compute_idx(x, y + 1), *p);
        write_data(read_idx, particle_empty());
    } else if (in_bounds(x + 1, y + 1) &&
               ((get_particle_at(x + 1, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x + 1, y + 1).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                (get_particle_at(x + 1, y + 1).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x + 1, y + 1).id == mat_id_salt && random_val(0, salt_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y + 1), *p);
        write_data(read_idx, particle_empty());
    } else if (in_bounds(x - 1, y + 1) &&
               ((get_particle_at(x - 1, y + 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x - 1, y + 1).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                (get_particle_at(x - 1, y + 1).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x - 1, y + 1).id == mat_id_salt && random_val(0, salt_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y + 1), *p);
        write_data(read_idx, particle_empty());
    } else if (in_bounds(x - 1, y) &&
               ((get_particle_at(x - 1, y).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x - 1, y).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                (get_particle_at(x - 1, y).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x - 1, y).id == mat_id_salt && random_val(0, salt_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y), *p);
        write_data(read_idx, particle_empty());
    } else if (in_bounds(x + 1, y) &&
               ((get_particle_at(x + 1, y).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x + 1, y).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                (get_particle_at(x + 1, y).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x + 1, y).id == mat_id_salt && random_val(0, sand_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y), *p);
        write_data(read_idx, particle_empty());
    } else if (in_bounds(x + 1, y - 1) &&
               ((get_particle_at(x + 1, y - 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x + 1, y - 1).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                (get_particle_at(x + 1, y - 1).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x + 1, y - 1).id == mat_id_salt && random_val(0, salt_chance) == 0)

                        )) {
        write_data(compute_idx(x + 1, y - 1), *p);
        write_data(read_idx, particle_empty());
    } else if (in_bounds(x - 1, y - 1) &&
               ((get_particle_at(x - 1, y - 1).id == mat_id_wood && random_val(0, wood_chance) == 0) || (get_particle_at(x - 1, y - 1).id == mat_id_stone && random_val(0, stone_chance) == 0) ||
                (get_particle_at(x - 1, y - 1).id == mat_id_sand && random_val(0, sand_chance) == 0) || (get_particle_at(x - 1, y - 1).id == mat_id_salt && random_val(0, salt_chance) == 0)

                        )) {
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, particle_empty());
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

    if (in_bounds(x + vx, y + vy) && (is_empty(x + vx, y + vy))) {
        write_data(v_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x, y + u)) {
        write_data(b_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + r, y + u)) {
        write_data(br_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + l, y + u)) {
        write_data(bl_idx, *p);
        write_data(read_idx, particle_empty());
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + u) && ((is_empty(x, y + u)))) {
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + u);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + l, y + u) && ((is_empty(x + l, y + u)))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x + l, y + u);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + r, y + u) && ((is_empty(x + r, y + u)))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x + r, y + u);
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    } else {
        particle_t tmp = *p;
        b32 found = false;

        // Don't try to spread if something is directly above you?
        if (completely_surrounded(x, y)) {
            write_data(read_idx, tmp);
            return;
        } else {
            for (u32 i = 0; i < fall_rate; ++i) {
                for (s32 j = spread_rate; j > 0; --j) {
                    if (in_bounds(x - j, y + i) && (is_empty(x - j, y + i) || get_particle_at(x - j, y + i).id == mat_id_oil)) {
                        particle_t tmp = get_particle_at(x - j, y + i);
                        write_data(compute_idx(x - j, y + i), *p);
                        write_data(read_idx, tmp);
                        found = true;
                        break;
                    }
                    if (in_bounds(x + j, y + i) && (is_empty(x + j, y + i) || get_particle_at(x + j, y + i).id == mat_id_oil)) {
                        particle_t tmp = get_particle_at(x + j, y + i);
                        write_data(compute_idx(x + j, y + i), *p);
                        write_data(read_idx, tmp);
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                write_data(read_idx, tmp);
            }
        }
    }
}

void update_water(u32 x, u32 y) {
    f32 dt = neko_engine_instance()->ctx.platform->time.delta;

    u32 read_idx = compute_idx(x, y);
    particle_t *p = &g_world_particle_data[read_idx];
    u32 write_idx = read_idx;
    u32 fall_rate = 2;
    s32 spread_rate = 5;

    p->velocity.y = neko_clamp(p->velocity.y + (gravity * dt), -10.f, 10.f);

    p->has_been_updated_this_frame = true;

    // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
    // if (in_bounds(x, y + 1) && !is_empty(x, y + 1) && get_particle_at(x, y + 1).id != mat_id_water) {
    if (in_bounds(x, y + 1) && !is_empty(x, y + 1)) {
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

    if (in_bounds(x + vx, y + vy) && (is_empty(x + vx, y + vy))) {
        write_data(v_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x, y + u)) {
        write_data(b_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + r, y + u)) {
        write_data(br_idx, *p);
        write_data(read_idx, particle_empty());
    } else if (is_empty(x + l, y + u)) {
        write_data(bl_idx, *p);
        write_data(read_idx, particle_empty());
    }
    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + u) && ((is_empty(x, y + u) || (g_world_particle_data[b_idx].id == mat_id_oil)))) {
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + u);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + l, y + u) && ((is_empty(x + l, y + u) || g_world_particle_data[bl_idx].id == mat_id_oil))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x + l, y + u);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + r, y + u) && ((is_empty(x + r, y + u) || g_world_particle_data[br_idx].id == mat_id_oil))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (gravity * dt);
        particle_t tmp_b = get_particle_at(x + r, y + u);
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    } else {
        particle_t tmp = *p;
        b32 found = false;

        // Don't try to spread if something is directly above you?
        if (completely_surrounded(x, y)) {
            write_data(read_idx, tmp);
            return;
        } else {
            for (u32 i = 0; i < fall_rate; ++i) {
                for (s32 j = spread_rate; j > 0; --j) {
                    if (in_bounds(x - j, y + i) && (is_empty(x - j, y + i) || get_particle_at(x - j, y + i).id == mat_id_oil)) {
                        particle_t tmp = get_particle_at(x - j, y + i);
                        write_data(compute_idx(x - j, y + i), *p);
                        write_data(read_idx, tmp);
                        found = true;
                        break;
                    }
                    if (in_bounds(x + j, y + i) && (is_empty(x + j, y + i) || get_particle_at(x + j, y + i).id == mat_id_oil)) {
                        particle_t tmp = get_particle_at(x + j, y + i);
                        write_data(compute_idx(x + j, y + i), *p);
                        write_data(read_idx, tmp);
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                write_data(read_idx, tmp);
            }
        }
    }
}

void update_default(u32 w, u32 h) {
    u32 read_idx = compute_idx(w, h);
    write_data(read_idx, get_particle_at(w, h));
}

particle_t particle_empty() {
    particle_t p = {0};
    p.id = mat_id_empty;
    p.color = mat_col_empty;
    return p;
}

particle_t particle_sand() {
    particle_t p = {0};
    p.id = mat_id_sand;
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
    p.id = mat_id_water;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.1f, 0.15f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.3f, 0.35f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.7f, 0.8f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_salt() {
    particle_t p = {0};
    p.id = mat_id_salt;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.9f, 1.0f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.8f, 0.85f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.8f, 0.9f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_wood() {
    particle_t p = {0};
    p.id = mat_id_wood;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.23f, 0.25f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.15f, 0.18f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.02f, 0.03f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_gunpowder() {
    particle_t p = {0};
    p.id = mat_id_gunpowder;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.15f, 0.2f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.15f, 0.2f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.15f, 0.2f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_oil() {
    particle_t p = {0};
    p.id = mat_id_oil;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.12f, 0.15f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.10f, 0.12f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.08f, 0.10f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_fire() {
    particle_t p = {0};
    p.id = mat_id_fire;
    p.color = mat_col_fire;
    return p;
}

particle_t particle_lava() {
    particle_t p = {0};
    p.id = mat_id_lava;
    p.color = mat_col_fire;
    return p;
}

particle_t particle_ember() {
    particle_t p = {0};
    p.id = mat_id_ember;
    p.color = mat_col_ember;
    return p;
}

particle_t particle_smoke() {
    particle_t p = {0};
    p.id = mat_id_smoke;
    p.color = mat_col_smoke;
    return p;
}

particle_t particle_steam() {
    particle_t p = {0};
    p.id = mat_id_steam;
    p.color = mat_col_steam;
    return p;
}

particle_t particle_stone() {
    particle_t p = {0};
    p.id = mat_id_stone;
    f32 r = (f32)(random_val(0, 1)) / 2.f;
    p.color.r = (u8)(neko_interp_linear(0.5f, 0.65f, r) * 255.f);
    p.color.g = (u8)(neko_interp_linear(0.5f, 0.65f, r) * 255.f);
    p.color.b = (u8)(neko_interp_linear(0.5f, 0.65f, r) * 255.f);
    p.color.a = 255;
    return p;
}

particle_t particle_acid() {
    particle_t p = {0};
    p.id = mat_id_acid;
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

typedef int neko_tex_bool;

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
typedef int neko_tex_direction;  // 8 cw directions: >, _|, v, |_, <, |", ^, "|
#define __neko_tex_direction_opposite(dir) ((dir + 4) & 7)
static const neko_tex_point neko_tex_direction_to_pixel_offset[] = {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};

// image manipulation functions
neko_tex_uc *neko_tex_rgba_to_alpha(const neko_tex_uc *data, int w, int h) {
    neko_tex_uc *result = (neko_tex_uc *)neko_safe_malloc(w * h);
    int x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[(y * w + x) * 4 + 3];
    return result;
}

neko_tex_uc *neko_tex_alpha_to_thresholded(const neko_tex_uc *data, int w, int h, neko_tex_uc threshold) {
    neko_tex_uc *result = (neko_tex_uc *)neko_safe_malloc(w * h);
    int x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
    return result;
}

neko_tex_uc *neko_tex_dilate_thresholded(const neko_tex_uc *data, int w, int h) {
    int x, y, dx, dy, cx, cy;
    neko_tex_uc *result = (neko_tex_uc *)neko_safe_malloc(w * h);
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

neko_tex_uc *neko_tex_thresholded_to_outlined(const neko_tex_uc *data, int w, int h) {
    neko_tex_uc *result = (neko_tex_uc *)neko_safe_malloc(w * h);
    int x, y;
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
static neko_tex_bool neko_tex_find_first_filled_pixel(const neko_tex_uc *data, int w, int h, neko_tex_point *first) {
    int x, y;
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

static neko_tex_bool neko_tex_find_next_filled_pixel(const neko_tex_uc *data, int w, int h, neko_tex_point current, neko_tex_direction *dir, neko_tex_point *next) {
    // turn around 180°, then make a clockwise scan for a filled pixel
    *dir = __neko_tex_direction_opposite(*dir);
    int i;
    for (i = 0; i < 8; i++) {
        __neko_tex_point_add(*next, current, neko_tex_direction_to_pixel_offset[*dir]);

        if (__neko_tex_point_is_inside(*next, w, h) && data[next->y * w + next->x]) return 1;

        // move to next angle (clockwise)
        *dir = *dir - 1;
        if (*dir < 0) *dir = 7;
    }
    return 0;
}

neko_tex_point *neko_tex_extract_outline_path(neko_tex_uc *data, int w, int h, int *point_count, neko_tex_point *reusable_outline) {
    neko_tex_point *outline = reusable_outline;
    if (!outline) outline = (neko_tex_point *)neko_safe_malloc(w * h * sizeof(neko_tex_point));

    neko_tex_point current, next;

restart:
    if (!neko_tex_find_first_filled_pixel(data, w, h, &current)) {
        *point_count = 0;
        return outline;
    }

    int count = 0;
    neko_tex_direction dir = 0;

    while (__neko_tex_point_is_inside(current, w, h)) {
        data[current.y * w + current.x] = 0;  // clear the visited path
        outline[count++] = current;           // add our current point to the outline
        if (!neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) {
            // find loop connection
            neko_tex_bool found = 0;
            int i;
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
                int prev;
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

void neko_tex_distance_based_path_simplification(neko_tex_point *outline, int *outline_length, f32 distance_threshold) {
    int length = *outline_length;
    int l;
    for (l = length / 2 /*length - 1*/; l > 1; l--) {
        int a, b = l;
        for (a = 0; a < length; a++) {
            neko_tex_point ab;
            __neko_tex_point_sub(ab, outline[b], outline[a]);
            f32 lab = sqrtf((f32)(ab.x * ab.x + ab.y * ab.y));
            f32 ilab = 1.0f / lab;
            f32 abnx = ab.x * ilab, abny = ab.y * ilab;

            if (lab != 0.0f) {
                neko_tex_bool found = 1;
                int i = (a + 1) % length;
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
                    int i;
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

int main(int argc, char **argv) {
    neko_application_desc_t app = {0};
    app.window_title = "Hello Neko";
    app.window_width = g_window_width;
    app.window_height = g_window_height;
    app.init = &app_init;
    app.update = &app_update;
    app.shutdown = &app_shutdown;
    app.window_flags = neko_window_flags::resizable;
    app.frame_rate = 60;
    app.arg = {argc, argv};
    app.enable_vsync = false;

    // Construct internal instance of our engine
    neko_engine_t *engine = neko_engine_construct(app);

    // Run the internal engine loop until completion
    neko_result result = engine->run();

    // Check result of engine after exiting loop
    if (result != neko_result_success) {
        neko_warn("Error: Engine did not successfully finish running.");
        return -1;
    }

    neko_info("good");

    return 0;
}
