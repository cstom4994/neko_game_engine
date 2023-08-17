

#include "engine/audio/neko_audio.h"
#include "engine/base/neko_component.h"
#include "engine/base/neko_cvar.hpp"
#include "engine/base/neko_ecs.h"
#include "engine/common/neko_hash.h"
#include "engine/editor/neko_dbgui.hpp"
#include "engine/editor/neko_editor.hpp"
#include "engine/editor/neko_profiler.hpp"
#include "engine/filesystem/neko_packer.h"
#include "engine/graphics/neko_particle.h"
#include "engine/graphics/neko_render_pass.h"
#include "engine/graphics/neko_sprite.h"
#include "engine/gui/neko_imgui_lua.hpp"
#include "engine/gui/neko_imgui_utils.hpp"
#include "engine/neko.h"
#include "engine/physics/neko_phy.h"
#include "engine/scripting/neko_scripting.h"
#include "engine/utility/logger.hpp"
#include "engine/utility/module.hpp"
#include "libs/glad/glad.h"

#define NEKO_BUILTIN_IMPL
#include "engine/gui/neko_builtin_font.h"

#define g_window_width 1420
#define g_window_height 880
neko_global const s32 g_scale = 4;
neko_global const s32 g_texture_width = g_window_width / g_scale;
neko_global const s32 g_texture_height = g_window_height / g_scale;

// neko engine cpp
using namespace neko;

using namespace std::string_literals;

typedef neko_color_t cell_color_t;

typedef struct particle_t {
    u8 id;
    f32 life_time;
    neko_vec2 velocity;
    cell_color_t color;
    bool has_been_updated_this_frame;
} particle_t;

// Globals
neko_global neko_vertex_buffer_t g_vbo = {0};
neko_global neko_index_buffer_t g_ibo = {0};
neko_global neko_command_buffer_t g_cb = {0};
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
neko_global neko_packreader_t *g_pack_reader;
neko_global neko_quad_batch_t g_batch = {0};

neko_global neko_shader_t g_custom_batch_shader = {0};
neko_global neko_quad_batch_t g_custom_batch = {0};
neko_global neko_resource(neko_material_t) g_custom_batch_mat = {0};

// 3d
neko_global neko_uniform_t u_color = {};
neko_global neko_uniform_t u_model = {};
neko_global neko_uniform_t u_view = {};
neko_global neko_uniform_t u_proj = {};

neko_global neko_vertex_buffer_t g_3d_vbo = {0};

neko_global neko_camera_t g_camera = {0};

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

neko_global bool g_show_material_selection_panel = true;
neko_global bool g_show_frame_count = true;

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

const char *v_3d_src = R"glsl(
#version 330 core
layout(location = 0) in vec3 a_pos;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0); 
}
)glsl";

const char *f_3d_src = R"glsl(
#version 330 core
uniform vec4 u_color;
out vec4 frag_color;

#define COLOR_STEP 6.0
#define PIXEL_SIZE 4.0

vec4 colorize(in vec4 color) {

    // Pixel art coloring
    vec3 nCol = normalize(color.rgb);
    float nLen = length(color.rgb);
    return vec4(nCol * round(nLen * COLOR_STEP) / COLOR_STEP, color.w);

}

//void mainImage( out vec4 fragColor, in vec2 fragCoord )
//{
//    // Pixel Sizing
//    float ratio = iResolution.y/720.0;
//    vec2 pixel = round(fragCoord / (PIXEL_SIZE * ratio)) * PIXEL_SIZE * ratio;
//    vec2 uv = pixel/iResolution.xy;
//    fragColor = colorize(texture(iChannel0, uv));
//}

void main()
{
    frag_color = colorize(u_color); 
}

)glsl";

// Forward Decls.
neko_result app_init();
neko_result app_update();
neko_result app_shutdown();

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

u8 *neko_tex_rgba_to_alpha(const u8 *data, s32 w, s32 h);
u8 *neko_tex_alpha_to_thresholded(const u8 *data, s32 w, s32 h, u8 threshold);
u8 *neko_tex_thresholded_to_outlined(const u8 *data, s32 w, s32 h);

typedef struct {
    short x, y;
} neko_tex_point;
neko_tex_point *neko_tex_extract_outline_path(u8 *data, s32 w, s32 h, s32 *point_count, neko_tex_point *reusable_outline);
void neko_tex_distance_based_path_simplification(neko_tex_point *outline, s32 *outline_length, f32 distance_threshold);

// Rendering
void render_scene();

void test_wang();
void test_sr();
void test_ut();
void test_rf();
void test_se();

// 基础容器测试

// 各种容器的自定义结构对象
typedef struct object_t {
    f32 float_value;
    u32 uint_value;
} object_t;

/*
    声明自定义哈希表类型：
    * key: u64 - 用于引用存储数据的密钥类型
    * val: object_t - 表中存储的值类型
    * hash_key_func: neko_hash_u64 - 用于散列密钥的散列函数（在 neko_util.h 中）
    * hash_comp_func: neko_hash_key_comp_std_type - 哈希比较函数（在 neko_util.h 中）
*/
neko_hash_table_decl(u64, object_t, neko_hash_u64, neko_hash_key_comp_std_type);

/*
    声明自定义槽数组类型：
    * type: object_t - 表中存储的值类型
*/
neko_slot_array_decl(object_t);

// Globals
neko_dyn_array(object_t) g_dyn_array;
neko_hash_table(u64, object_t) g_hash_table;
neko_slot_array(object_t) g_slot_array;
u32 g_cur_val;

// Util functions
void print_array(neko_dyn_array(object_t) *);
void print_slot_array(neko_slot_array(object_t) *);
void print_hash_table(neko_hash_table(u64, object_t) *);
void object_to_str(object_t *obj, char *str, usize str_sz);

neko_global neko_sprite g_test_spr = {0};

#pragma region CustomBatch

const char *quad_batch_custom_vert_src = R"glsl(
#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec4 a_color_two;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
out vec2 uv;
out vec4 color;
out vec4 color_two;
void main()
{
   gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
   uv = a_uv;
   color = a_color;
   color_two = a_color_two;
}
)glsl";

const char *quad_batch_custom_frag_src = R"glsl(
#version 330
uniform sampler2D u_tex;
uniform f32 u_alpha;
in vec2 uv;
in vec4 color;
in vec4 color_two;
out vec4 frag_color;
void main()
{
   frag_color = vec4((mix(color_two, color, 0.5) * texture(u_tex, uv)).rgb, u_alpha);
}
)glsl";

typedef struct quad_batch_custom_vert_t {
    neko_vec3 position;
    neko_vec2 uv;
    neko_vec4 color;
    neko_vec4 color_two;
} quad_batch_custom_vert_t;

typedef struct quad_batch_custom_info_t {
    neko_vqs transform;
    neko_vec4 uv;
    neko_vec4 color;
    neko_vec4 color_two;
} quad_batch_custom_info_t;

void quad_batch_custom_add(neko_quad_batch_t *qb, void *quad_batch_custom_info_data) {
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;

    quad_batch_custom_info_t *quad_info = (quad_batch_custom_info_t *)(quad_batch_custom_info_data);
    if (!quad_info) {
        neko_assert(false);
    }

    neko_vqs transform = quad_info->transform;
    neko_vec4 uv = quad_info->uv;
    neko_vec4 color = quad_info->color;
    neko_vec4 color_two = quad_info->color_two;

    neko_mat4 model = neko_vqs_to_mat4(&transform);

    neko_vec3 _tl = neko_vec3{-0.5f, -0.5f, 0.f};
    neko_vec3 _tr = neko_vec3{0.5f, -0.5f, 0.f};
    neko_vec3 _bl = neko_vec3{-0.5f, 0.5f, 0.f};
    neko_vec3 _br = neko_vec3{0.5f, 0.5f, 0.f};
    neko_vec4 position = {0};
    quad_batch_custom_vert_t tl = {0};
    quad_batch_custom_vert_t tr = {0};
    quad_batch_custom_vert_t bl = {0};
    quad_batch_custom_vert_t br = {0};

    // Top Left
    position = neko_mat4_mul_vec4(model, neko_vec4{_tl.x, _tl.y, _tl.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    tl.position = neko_vec3{position.x, position.y, position.z};
    tl.uv = neko_vec2{uv.x, uv.y};
    tl.color = color;
    tl.color_two = color_two;

    // Top Right
    position = neko_mat4_mul_vec4(model, neko_vec4{_tr.x, _tr.y, _tr.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    tr.position = neko_vec3{position.x, position.y, position.z};
    tr.uv = neko_vec2{uv.z, uv.y};
    tr.color = color;
    tr.color_two = color_two;

    // Bottom Left
    position = neko_mat4_mul_vec4(model, neko_vec4{_bl.x, _bl.y, _bl.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    bl.position = neko_vec3{position.x, position.y, position.z};
    bl.uv = neko_vec2{uv.x, uv.w};
    bl.color = color;
    bl.color_two = color_two;

    // Bottom Right
    position = neko_mat4_mul_vec4(model, neko_vec4{_br.x, _br.y, _br.z, 1.0f});
    position = neko_vec4_scale(position, 1.0f / position.w);
    br.position = neko_vec3{position.x, position.y, position.z};
    br.uv = neko_vec2{uv.z, uv.w};
    br.color = color;
    br.color_two = color_two;

    quad_batch_custom_vert_t verts[] = {tl, br, bl, tl, tr, br};

    __neko_quad_batch_add_raw_vert_data(qb, verts, sizeof(verts));
}

#pragma endregion

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

    s32 bpp = 4;

    neko_texture_parameter_desc t_desc = neko_texture_parameter_desc_default();

    t_desc.texture_format = neko_texture_format_rgba8;
    t_desc.mag_filter = neko_nearest;
    t_desc.min_filter = neko_nearest;
    t_desc.generate_mips = false;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    t_desc.num_comps = 4;
    t_desc.data = ase->frames->pixels;

    neko_tex_flip_vertically(ase->w, ase->h, (u8 *)t_desc.data);

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
    return (neko_rand_xorshf32() % (upper - lower + 1) + lower);
}

s32 compute_idx(s32 x, s32 y) { return (y * g_texture_width + x); }

bool in_bounds(s32 x, s32 y) {
    if (x < 0 || x > (g_texture_width - 1) || y < 0 || y > (g_texture_height - 1)) return false;
    return true;
}

bool is_empty(s32 x, s32 y) { return (in_bounds(x, y) && g_world_particle_data[compute_idx(x, y)].id == mat_id_empty); }

particle_t get_particle_at(s32 x, s32 y) { return g_world_particle_data[compute_idx(x, y)]; }

bool completely_surrounded(s32 x, s32 y) {
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

bool is_in_liquid(s32 x, s32 y, s32 *lx, s32 *ly) {
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

bool is_in_water(s32 x, s32 y, s32 *lx, s32 *ly) {
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

neko_ecs_decl_mask(MOVEMENT_SYSTEM, 3, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_SPRITE);
void movement_system(neko_ecs *ecs) {
    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(MOVEMENT_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            CVelocity *velocity = (CVelocity *)neko_ecs_ent_get_component(ecs, e, COMPONENT_VELOCITY);
            neko_sprite_renderer *sprite = (neko_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            // Grab global instance of engine
            neko_engine *engine = neko_engine_instance();

            neko_sprite_renderer_update(sprite, engine->ctx.platform->time.delta);

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
            neko_sprite_renderer *sprite = (neko_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
            neko_command_buffer_t *cb = &g_cb;

            s32 index;
            if (sprite->loop) {
                index = sprite->loop->indices[sprite->current_frame];
            } else {
                index = sprite->current_frame;
            }

            neko_sprite *spr = sprite->sprite;
            neko_sprite_frame f = spr->frames[index];

            gfx->immediate.draw_rect_textured_ext(cb, 200.f, 200.f, 200.f + spr->width * 4.f, 200.f + spr->height * 4.f, f.u0, f.v0, f.u1, f.v1, sprite->sprite->img.id, neko_color_white);
        }
    }
}

void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1000, sizeof(CTransform), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 200, sizeof(CVelocity), NULL);
    neko_ecs_register_component(ecs, COMPONENT_SPRITE, 1000, sizeof(neko_sprite_renderer), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems will run the systems in the order they are registered
    // ecs_run_system is also available if you wish to handle each system seperately
    //
    // neko_ecs, function pointer to system (must take a parameter of neko_ecs), system type
    neko_ecs_register_system(ecs, movement_system, ECS_SYSTEM_UPDATE);
    neko_ecs_register_system(ecs, sprite_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
}

neko_global neko_font_index g_basic_font;

neko_global neko_obj_swarm swarm;
neko_global neko_swarm_simulator_settings settings;

// 内部音频数据的资源句柄 由于音频必须在单独的线程上运行 因此这是必要的
cs_audio_source_t *piano;
cs_sound_params_t params = cs_sound_params_default();

neko_sprite_renderer sprite_test = {};

namespace {

neko_phy_body bodies[200];
neko_phy_joint joints[100];

neko_phy_body *bomb = NULL;

s32 iterations = 20;
neko_vec2 gravity{0.0f, -10.0f};

s32 numBodies = 0;
s32 numJoints = 0;

s32 demoIndex = 0;

f32 timeStep = 1.f / 60.f;

neko_phy_world world(gravity, iterations);
}  // namespace

neko_inline neko_vec2 world_to_screen_pos(const neko_vec2 &pos) {
    f32 screenX = pos.x * g_scale;
    f32 screenY = g_window_height - pos.y * g_scale;
    return neko_vec2{screenX, screenY};
}

neko_inline neko_vec2 screen_to_world_pos(const neko_vec2 &pos) {
    f32 worldX = pos.x / g_scale;
    f32 worldY = (g_window_height - pos.y) / g_scale;
    return neko_vec2{worldX, worldY};
}

static void DrawBody(neko_phy_body *body) {
    neko_mat22 R = neko_mat22_ctor(body->rotation);
    neko_vec2 x = body->position;
    neko_vec2 h = 0.5f * body->width;

    neko_vec2 v1 = world_to_screen_pos(x + R * neko_vec2{-h.x, -h.y});
    neko_vec2 v2 = world_to_screen_pos(x + R * neko_vec2{h.x, -h.y});
    neko_vec2 v3 = world_to_screen_pos(x + R * neko_vec2{h.x, h.y});
    neko_vec2 v4 = world_to_screen_pos(x + R * neko_vec2{-h.x, h.y});

    // Graphics api instance
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_command_buffer_t *cb = &g_cb;

    neko_color_t color = body == bomb ? neko_color_red : neko_color_white;

    gfx->immediate.draw_line_ext(cb, v1, v2, 4.f, color);
    gfx->immediate.draw_line_ext(cb, v2, v3, 4.f, color);
    gfx->immediate.draw_line_ext(cb, v3, v4, 4.f, color);
    gfx->immediate.draw_line_ext(cb, v4, v1, 4.f, color);
}

static void DrawJoint(neko_phy_joint *joint) {
    neko_phy_body *b1 = joint->body1;
    neko_phy_body *b2 = joint->body2;

    neko_mat22 R1 = neko_mat22_ctor(b1->rotation);
    neko_mat22 R2 = neko_mat22_ctor(b2->rotation);

    neko_vec2 x1 = b1->position;
    neko_vec2 p1 = x1 + R1 * joint->localAnchor1;

    neko_vec2 x2 = b2->position;
    neko_vec2 p2 = x2 + R2 * joint->localAnchor2;

    // Graphics api instance
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_command_buffer_t *cb = &g_cb;

    neko_color_t color = neko_color_orange;

    gfx->immediate.draw_line_ext(cb, world_to_screen_pos(x1), world_to_screen_pos(p1), 3.f, color);
    gfx->immediate.draw_line_ext(cb, world_to_screen_pos(x2), world_to_screen_pos(p2), 3.f, color);
}

static void LaunchBomb() {
    if (!bomb) {
        bomb = bodies + numBodies;
        neko_body_set(bomb, neko_vec2{15.0f, 15.0f}, 50.0f);
        bomb->friction = 0.2f;
        world.neko_phy_create(bomb);
        ++numBodies;
    }

    neko_vec2 ws = neko_engine_instance()->ctx.platform->window_size(g_window);
    neko_vec2 pmp = neko_engine_instance()->ctx.platform->mouse_position();

    neko_vec2 pos = screen_to_world_pos(pmp);

    bomb->position.set(pos.x, pos.y);
    bomb->rotation = neko_math_rand(-1.5f, 1.5f);
    bomb->velocity = {neko_math_rand(-1.5f, 1.5f), neko_math_rand(-1.5f, 1.5f)};
    bomb->angularVelocity = neko_math_rand(-4.0f, 4.0f);
}

void init_demo(s32 i) {
    world.neko_phy_clear();
    numBodies = 0;
    numJoints = 0;
    bomb = NULL;

    neko_phy_body *b = bodies;
    neko_phy_joint *j = joints;

    if (i == 1) {  // A pyramid
        neko_body_set(b, neko_vec2{1000.0f, 5.0f}, neko_mass_max);
        b->position.set(200.0f, 5.f);
        b->friction = 4.f;
        world.neko_phy_create(b);
        ++b;
        ++numBodies;

        neko_vec2 x{15.0f, 20.f};
        neko_vec2 y;

        for (s32 i = 0; i < 12; ++i) {
            y = x;

            for (s32 j = i; j < 12; ++j) {
                neko_body_set(b, neko_vec2{15.0f, 15.0f}, 10.0f);
                b->friction = 0.2f;
                b->position = y;
                world.neko_phy_create(b);
                ++b;
                ++numBodies;

                y += neko_vec2{20.f, 0.0f};
            }

            // x += neko_vec2{0.5625f, 1.125f);
            x += neko_vec2{7.5f, 20.0f};
        }
    } else if (i == 2)  // A simple pendulum
    {
        neko_phy_body *b1 = b + 0;
        neko_body_set(b, neko_vec2{1000.0f, 10.0f}, neko_mass_max);
        b1->friction = 0.2f;
        b->position.set(200.0f, 10.5f);
        b1->rotation = 0.0f;
        world.neko_phy_create(b1);

        neko_phy_body *b2 = b + 1;
        neko_body_set(b2, neko_vec2{15.0f, 15.0f}, 100.0f);
        b2->friction = 0.2f;
        b2->position.set(40.0f, 100.0f);
        b2->rotation = 0.0f;
        world.neko_phy_create(b2);

        numBodies += 2;

        j->Set(b1, b2, neko_vec2{150.0f, 150.0f});
        world.neko_phy_create(j);

        numJoints += 1;
    } else if (i == 3) {  // A multi-pendulum
        neko_body_set(b, neko_vec2{1000.0f, 10.0f}, neko_mass_max);
        b->friction = 0.2f;
        b->position.set(0.0f, -0.5f * b->width.y);
        b->rotation = 0.0f;
        world.neko_phy_create(b);

        neko_phy_body *b1 = b;
        ++b;
        ++numBodies;

        f32 mass = 10.0f;

        // Tuning
        f32 frequencyHz = 4.0f;
        f32 dampingRatio = 0.7f;

        // frequency in radians
        f32 omega = 2.0f * neko_pi * frequencyHz;

        // damping coefficient
        f32 d = 2.0f * mass * dampingRatio * omega;

        // spring stiffness
        f32 k = mass * omega * omega;

        // magic formulas
        f32 softness = 1.0f / (d + timeStep * k);
        f32 biasFactor = timeStep * k / (d + timeStep * k);

        const f32 y = 100.0f;
        const f32 xx = 100.0f;

        for (s32 i = 0; i < 15; ++i) {
            neko_vec2 x{xx + i * 5.f, y};
            neko_body_set(b, neko_vec2{4.0f, 6.0f}, mass);
            b->friction = 0.2f;
            b->position = x;
            b->rotation = 0.0f;
            world.neko_phy_create(b);

            j->Set(b1, b, neko_vec2{f32(xx + i * 5.f), y});
            j->softness = softness;
            j->biasFactor = biasFactor;
            world.neko_phy_create(j);

            b1 = b;
            ++b;
            ++numBodies;
            ++j;
            ++numJoints;
        }
    } else if (i == 4) {

        neko_body_set(b, neko_vec2{1000.0f, 10.0f}, neko_mass_max);
        b->friction = 0.2f;
        b->position.set(0.0f, -0.5f * b->width.y);
        b->rotation = 0.0f;
        world.neko_phy_create(b);
        ++b;
        ++numBodies;

        const s32 numPlanks = 15;
        f32 mass = 50.0f;

        for (s32 i = 0; i < numPlanks; ++i) {
            neko_body_set(b, neko_vec2{50.0f, 15.f}, mass);
            b->friction = 0.2f;
            b->position.set(-8.5f + 1.25f * i, 5.0f);
            world.neko_phy_create(b);
            ++b;
            ++numBodies;
        }

        // Tuning
        f32 frequencyHz = 2.0f;
        f32 dampingRatio = 0.7f;

        // frequency in radians
        f32 omega = 2.0f * neko_pi * frequencyHz;

        // damping coefficient
        f32 d = 2.0f * mass * dampingRatio * omega;

        // spring stifness
        f32 k = mass * omega * omega;

        // magic formulas
        f32 softness = 1.0f / (d + timeStep * k);
        f32 biasFactor = timeStep * k / (d + timeStep * k);

        for (s32 i = 0; i < numPlanks; ++i) {
            j->Set(bodies + i, bodies + i + 1, neko_vec2{-9.125f + 50.25f * i, 15.0f});
            j->softness = softness;
            j->biasFactor = biasFactor;

            world.neko_phy_create(j);
            ++j;
            ++numJoints;
        }

        j->Set(bodies + numPlanks, bodies, neko_vec2{-9.125f + 50.25f * numPlanks, 15.0f});
        j->softness = softness;
        j->biasFactor = biasFactor;
        world.neko_phy_create(j);
        ++j;
        ++numJoints;
    } else if (i == 5) {

        neko_body_set(b, neko_vec2{1000.0f, 10.0f}, neko_mass_max);
        b->friction = 0.2f;
        b->position.set(0.0f, -0.5f * b->width.y);
        b->rotation = 0.0f;
        world.neko_phy_create(b);
        ++b;
        ++numBodies;

        for (s32 i = 0; i < 10; ++i) {
            neko_body_set(b, neko_vec2{15.0f, 15.0f}, 4.0f);
            b->friction = 0.2f;
            f32 x = neko_math_rand(-1.f, 1.f);
            b->position.set(x + 150.f, 0.51f + 16.f * i);
            world.neko_phy_create(b);
            ++b;
            ++numBodies;
        }
    }
}

// Here, we'll initialize all of our application data, which in this case is our graphics resources
neko_result app_init() {
    // Cache instance of api contexts from engine
    neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    neko_audio_i *audio = neko_engine_subsystem(audio);

    // Construct command buffer (the command buffer is used to allow for immediate drawing at any point in our program)
    g_cb = neko_command_buffer_new();

    // Construct shader from our source above
    neko_shader_t *_shader = gfx->neko_shader_create("g_shader", v_src, f_src);

    // Construct uniform for shader
    u_tex = gfx->construct_uniform(*_shader, "u_tex", neko_uniform_type_sampler2d);
    u_flip_y = gfx->construct_uniform(*_shader, "u_flip_y", neko_uniform_type_int);

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

    // 测试3d
    _shader = gfx->neko_shader_create("g_3d_shader", v_3d_src, f_3d_src);
    u_color = gfx->construct_uniform(*_shader, "u_color", neko_uniform_type_vec4);
    u_view = gfx->construct_uniform(*_shader, "u_view", neko_uniform_type_mat4);
    u_model = gfx->construct_uniform(*_shader, "u_model", neko_uniform_type_mat4);
    u_proj = gfx->construct_uniform(*_shader, "u_proj", neko_uniform_type_mat4);

    // Vertex data layout for our mesh (for this shader, it's a single float3 attribute for position)
    neko_vertex_attribute_type layout_3d[] = {neko_vertex_attribute_float3};

    // Vertex data for cube
    f32 v_3d_data[] = {                      // Position
                       -0.5f, -0.5f, -0.5f,  // First face
                       0.5f,  -0.5f, -0.5f,  //
                       0.5f,  0.5f,  -0.5f,  //
                       0.5f,  0.5f,  -0.5f,  //
                       -0.5f, 0.5f,  -0.5f,  //
                       -0.5f, -0.5f, -0.5f,  //

                       -0.5f, -0.5f, 0.5f,  // Second face
                       0.5f,  -0.5f, 0.5f,  //
                       0.5f,  0.5f,  0.5f,  //
                       0.5f,  0.5f,  0.5f,  //
                       -0.5f, 0.5f,  0.5f,  //
                       -0.5f, -0.5f, 0.5f,  //

                       -0.5f, 0.5f,  0.5f,   // Third face
                       -0.5f, 0.5f,  -0.5f,  //
                       -0.5f, -0.5f, -0.5f,  //
                       -0.5f, -0.5f, -0.5f,  //
                       -0.5f, -0.5f, 0.5f,   //
                       -0.5f, 0.5f,  0.5f,   //

                       0.5f,  0.5f,  0.5f,   // Fourth face
                       0.5f,  0.5f,  -0.5f,  //
                       0.5f,  -0.5f, -0.5f,  //
                       0.5f,  -0.5f, -0.5f,  //
                       0.5f,  -0.5f, 0.5f,   //
                       0.5f,  0.5f,  0.5f,   //

                       -0.5f, -0.5f, -0.5f,  // Fifth face
                       0.5f,  -0.5f, -0.5f,  //
                       0.5f,  -0.5f, 0.5f,   //
                       0.5f,  -0.5f, 0.5f,   //
                       -0.5f, -0.5f, 0.5f,   //
                       -0.5f, -0.5f, -0.5f,  //

                       -0.5f, 0.5f,  -0.5f,  // Sixth face
                       0.5f,  0.5f,  -0.5f,  //
                       0.5f,  0.5f,  0.5f,   //
                       0.5f,  0.5f,  0.5f,   //
                       -0.5f, 0.5f,  0.5f,   //
                       -0.5f, 0.5f,  -0.5f};
    // Construct vertex buffer
    g_3d_vbo = gfx->construct_vertex_buffer(layout_3d, sizeof(layout_3d), v_3d_data, sizeof(v_3d_data));

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

    // Construct camera parameters
    g_camera.transform = neko_vqs_default();
    g_camera.transform.position = neko_vec3{0.f, 0.f, -1.f};
    g_camera.fov = 60.f;
    g_camera.near_plane = 0.1f;
    g_camera.far_plane = 1000.f;
    g_camera.ortho_scale = 2.f;
    g_camera.proj_type = neko_projection_type_orthographic;

    g_test_ase = load_ase_texture_simple(neko_file_path("data/assets/textures/Sprite-0001.ase"));

    // Construct quad batch api and link up function pointers
    g_batch = neko_quad_batch_new(neko_resource_invalid(neko_material_t));

    // Set material texture uniform
    gfx->set_material_uniform_sampler2d(g_batch.material, "u_tex", g_test_ase, 0);

#ifdef custom_batch

    // 自定义batch
    neko_quad_batch_i *cqb = gfx->quad_batch_i;

    // Initialize quad batch API with custom implementation data
    neko_vertex_attribute_type qb_layout[] = {
            neko_vertex_attribute_float3,  // Position
            neko_vertex_attribute_float2,  // UV
            neko_vertex_attribute_float4,  // Color
            neko_vertex_attribute_float4   // Color2
    };

    g_custom_batch_shader = gfx->construct_shader(quad_batch_custom_vert_src, quad_batch_custom_frag_src);
    cqb->set_shader(cqb, g_custom_batch_shader);
    cqb->set_layout(cqb, qb_layout, sizeof(qb_layout));
    cqb->add = &quad_batch_custom_add;

    // Setup quad batch
    g_custom_batch_mat = neko_material_new(gfx->quad_batch_i->shader);

    // Set texture uniform for material
    gfx->set_material_uniform_sampler2d(g_custom_batch_mat, "u_tex", g_test_ase, 0);

    // Construct quad batch api and link up function pointers
    g_custom_batch = neko_quad_batch_new(g_custom_batch_mat);

#endif

    // 构建实例源
    piano = cs_load_wav(neko_file_path("data/assets/audio/otoha.wav"), NULL);

    // 基础容器测试
    g_cur_val = 0;

    // Allocate containers
    g_dyn_array = neko_dyn_array_new(object_t);
    g_slot_array = neko_slot_array_new(object_t);
    g_hash_table = neko_hash_table_new(u64, object_t);

    // neko_phy_create elements to containers
    for (g_cur_val = 0; g_cur_val < 10; ++g_cur_val) {
        object_t obj = {0};
        obj.float_value = (f32)g_cur_val;
        obj.uint_value = g_cur_val;

        // Push into dyn array
        neko_dyn_array_push(g_dyn_array, obj);

        // Push into slot array
        u32 id = neko_slot_array_insert(g_slot_array, obj);

        // Push into hash table
        neko_hash_table_insert(g_hash_table, (u64)g_cur_val, obj);
    }

    // Cache window handle from platform
    g_window = neko_engine_instance()->ctx.platform->main_window();

    // Initialize render passes
    g_blur_pass = blur_pass_ctor();
    g_bright_pass = bright_filter_pass_ctor();
    g_composite_pass = composite_pass_ctor();

    neko_pack_result pack_result = neko_pack_read(neko_file_path("data/resources.pack"), 0, 0, &g_pack_reader);

    if (pack_result != SUCCESS_PACK_RESULT) {
        // METADOT_ERROR("%d", pack_result);
        neko_assert(0);
    }

    neko_engine_cvar_init(&g_cvar, [] {});

    register_components(neko_engine_subsystem(ecs));
    register_systems(neko_engine_subsystem(ecs));

    // 测试用
    neko_ecs_ent e = neko_ecs_ent_make(neko_engine_subsystem(ecs));
    CTransform xform = {0, 0};
    CVelocity velocity = {5, 0};

    neko_sprite_load(&g_test_spr, neko_file_path("data/assets/textures/B_witch.ase"));

    sprite_test = {.sprite = &g_test_spr};
    neko_sprite_renderer_play(&sprite_test, "Charge_Loop");

    neko_ecs_ent_add_component(neko_engine_subsystem(ecs), e, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(neko_engine_subsystem(ecs), e, COMPONENT_VELOCITY, &velocity);
    neko_ecs_ent_add_component(neko_engine_subsystem(ecs), e, COMPONENT_SPRITE, &sprite_test);

    // neko_ecs_ent_destroy(ecs, e);

    neko_editor_create(g_cvar)
            .create("shader",
                    [&](neko_dbgui_result) {
                        neko_graphics_i *_gfx = neko_engine_instance()->ctx.graphics;
                        neko_platform_i *_platform = neko_engine_instance()->ctx.platform;

                        for (auto &[n, s] : (*_gfx->neko_shader_internal_list())) {
                            neko_editor_inspect_shader(std::to_string(n).c_str(), s->program_id);
                        }

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

                            lua_State *L = neko_sc()->neko_lua.state();
                            lua_gc(L, LUA_GCCOLLECT, 0);
                            lua_Integer kb = lua_gc(L, LUA_GCCOUNT, 0);
                            lua_Integer bytes = lua_gc(L, LUA_GCCOUNTB, 0);

                            ImGui::Text("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                            ImGui::Text("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                            ImGui::Text("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

                            static neko_platform_meminfo meminfo;

                            neko_timed_action(60, { meminfo = neko_engine_instance()->ctx.platform->get_meminfo(); });

                            ImGui::Text("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                            ImGui::Text("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                            ImGui::Text("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                            ImGui::Text("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                            ImGui::Text("Using renderer: %s", glGetString(GL_RENDERER));
                            ImGui::Text("OpenGL version supported: %s", glGetString(GL_VERSION));

                            neko_vec2 opengl_ver = _platform->get_opengl_ver();
                            ImGui::Text("OpenGL Version: %d.%d", (s32)opengl_ver.x, (s32)opengl_ver.y);
                        }

                        return neko_dbgui_result_in_progress;
                    })
            .create("Audio Test",
                    [&](neko_dbgui_result) {
                        neko_audio_i *audio = neko_engine_instance()->ctx.audio;

                        static cs_playing_sound_t pl;

                        if (ImGui::Button("播放/暂停")) {
                            // if (audio->is_playing(g_inst)) {
                            //     audio->pause(g_inst);
                            // } else {
                            //     audio->play(g_inst);
                            // }

                            pl = cs_play_sound(piano, params);
                        }

                        if (ImGui::Button("重新播放")) {
                            // audio->restart(g_inst);
                        }

                        if (ImGui::Button("停止播放")) {
                            // audio->stop(g_inst);
                            cs_sound_set_is_paused(pl, true);
                        }

                        if (ImGui::Button("Volume up")) {
                            // f32 cur_vol = audio->get_volume(g_inst);
                            // audio->set_volume(g_inst, cur_vol + 0.1f);
                        }

                        if (ImGui::Button("Volume down")) {
                            // f32 cur_vol = audio->get_volume(g_inst);
                            // audio->set_volume(g_inst, cur_vol - 0.1f);
                        }

                        // Can grab current runtime instance data as well
                        // neko_audio_instance_data_t id = audio->get_instance_data(g_inst);

                        // s32 sample_count = audio->get_sample_count(id.src);
                        // s32 sample_rate = audio->get_sample_rate(id.src);
                        // s32 num_channels = audio->get_num_channels(id.src);

                        // static char buf1[256] = neko_default_val();
                        // static char buf2[256] = neko_default_val();

                        // neko_timed_action(10, {
                        //     s32 min = 0, sec = 0;

                        //    // Runtime of source
                        //    audio->get_runtime(g_src, &min, &sec);
                        //    neko_snprintf(buf1, 256, sec < 10 ? "%d:0%d" : "%d:%d", min, sec);
                        //    // neko_println("Runtime: %s", buf);

                        //    // Get current play position
                        //    audio->convert_to_runtime(sample_count, sample_rate, num_channels, id.sample_position, &min, &sec);

                        //    neko_snprintf(buf2, 256, sec < 10 ? "%d:0%d" : "%d:%d", min, sec);
                        //    // neko_println("Play Time: %s", buf);
                        //});

                        // ImGui::Text(buf1);
                        // ImGui::Text(buf2);

                        return neko_dbgui_result_in_progress;
                    })
            .create("Containers Test", [&](neko_dbgui_result) {
                if (ImGui::Button("清空容器")) {
                    neko_dyn_array_clear(g_dyn_array);
                    neko_hash_table_clear(g_hash_table);
                    neko_slot_array_clear(g_slot_array);
                }

                if (ImGui::Button("插入元素")) {
                    object_t obj = neko_default_val();
                    obj.float_value = (f32)g_cur_val;
                    obj.uint_value = g_cur_val;

                    neko_dyn_array_push(g_dyn_array, obj);
                    neko_slot_array_insert(g_slot_array, obj);
                    neko_hash_table_insert(g_hash_table, (u64)g_cur_val, obj);

                    g_cur_val++;
                }

                if (ImGui::Button("删除元素")) {
                    neko_dyn_array_pop(g_dyn_array);
                    neko_slot_array_erase(g_slot_array, 0);  // slot 删除元素后 原排列索引不变
                }

                if (ImGui::Button("检查")) {
                    neko_println("");

                    print_array(&g_dyn_array);
                    print_slot_array(&g_slot_array);
                    print_hash_table(&g_hash_table);
                }

                return neko_dbgui_result_in_progress;
            });

    neko_lua_handle_t *font_handle = (neko_lua_handle_t *)neko_gc_alloc(&g_gc, sizeof(neko_lua_handle_t));
    neko_pack_item_data(g_pack_reader, ".\\fonts\\fusion-pixel.ttf", (const u8 **)&font_handle->data, (u32 *)&font_handle->size);
    g_basic_font = gfx->fontcache_load(font_handle->data, font_handle->size, 24.0f);

    g_test_font = gfx->construct_font_from_file(neko_file_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf"), 64.f);

    // Load UI font texture data from file
    construct_font_data(g_font_data);

    init_demo(1);

    // Set up callback for dropping them files, yo.
    platform->set_dropped_files_callback(platform->main_window(), &drop_file_callback);

    return neko_result_success;
}

neko_result app_update() {

    neko_profiler_scope_auto("client");

    // Grab global instance of engine
    neko_engine *engine = neko_engine_instance();

    // If we press the escape key, exit the application
    if (engine->ctx.platform->key_pressed(neko_keycode_esc)) {
        return neko_result_success;
    }

    // neko_timed_action(60, { neko_println("frame: %.5f ms", engine->ctx.platform->time.frame); });

    neko_invoke_once([] {
        the<dbgui>().update("shader", [](neko_dbgui_result) {
            if (ImGui::Button("test_wang")) test_wang();
            if (ImGui::Button("test_sr")) test_sr();
            if (ImGui::Button("test_ut")) test_ut();
            if (ImGui::Button("test_rf")) test_rf();
            if (ImGui::Button("test_se")) test_se();
            if (ImGui::Button("test_st")) neko_platform_print_callstack();
            if (ImGui::Button("test_cvars")) neko_config_print();
            if (ImGui::Button("test_rand")) neko_info(std::to_string(neko_rand_xorshf32()));
            ImGui::Image((void *)(intptr_t)g_tex.id, ImVec2(g_texture_width, g_texture_height), ImVec2(0, 0), ImVec2(1, 1));
            return neko_dbgui_result_in_progress;
        });
    }(););

    {
        neko_profiler_scope_auto("loop");

        // All application updates
        bool ui_interaction = update_ui();
        if (!ui_interaction) {
            update_input();
        }

        if (g_cvar.tick_world) {
            {
                neko_profiler_scope_auto("sim");
                update_particle_sim();
            }
            swarm.update(engine->ctx.platform->time.current, settings);
            {
                neko_profiler_scope_auto("ecs");
                neko_ecs_run_systems(neko_engine_subsystem(ecs), ECS_SYSTEM_UPDATE);
            }
            {
                neko_profiler_scope_auto("physics");
                world.neko_phy_step(engine->ctx.platform->time.delta * 2.f);
            }
        }
    }
    {
        neko_profiler_scope_auto("render");
        render_scene();
    }

    // Update frame counter
    g_frame_counter = (g_frame_counter + 1) % u32_max;

    settings.show();

    neko_invoke_once(lua_bind::l_G = neko_sc()->neko_lua.state(); lua_bind::imgui_init_lua(););
    neko_invoke_once(neko_sc()->neko_lua.dostring(R"(
      -- Main window
      local Window = ImGui.new("Window", "example title")
      -- Inner elements
      local Label = ImGui.new("Label", Window)
      local TabSelector = ImGui.new("TabSelector", Window)
      local Tab1 = TabSelector:AddTab("Tab1")
      local Tab2 = TabSelector:AddTab("Tab2")
      local Button = ImGui.new("Button", Tab1)
      local Slider = ImGui.new("Slider", Tab1, true) -- (boolean: 3) aligns it side-by-side !
      local ColorPicker = ImGui.new("ColorPicker", Tab2)
      Label.Text = "这个窗口由Lua脚本控制"
      Slider.Text = "Slider Example"
      Button.Text = "Button Example"
      ColorPicker.Text = "Color Picker Example"
      Slider.Min = -10
      Slider.Max = 10
      Button.Callback = function()
        ImGui.new("Label", Tab1).Text = "Hello world"
      end
          )"););
    for (auto &w : lua_bind::windows) {
        w.render();
    }

    return neko_result_in_progress;
}

neko_result app_shutdown() {

    // 释放容器
    neko_dyn_array_free(g_dyn_array);
    neko_hash_table_free(g_hash_table);
    neko_slot_array_free(g_slot_array);

    neko_pack_destroy(g_pack_reader);

    neko_info("app_shutdown");
    return neko_result_success;
}

void putpixel(s32 x, s32 y) {
    if (in_bounds(x, y)) {
        g_ui_buffer[compute_idx(x, y)] = cell_color_t{255, 255, 255, 255};
    }
}

// Function to put pixels
// at subsequence points
void drawCircle(s32 xc, s32 yc, s32 x, s32 y) {
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
void circleBres(s32 xc, s32 yc, s32 r) {
    s32 x = 0, y = r;
    s32 d = 3 - 2 * r;
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
        g_cvar.draw_shaders = !g_cvar.draw_shaders;
    }

    if (platform->key_pressed(neko_keycode_tab)) {
        g_cvar.ui_tweak = !g_cvar.ui_tweak;
    }

    if (platform->key_pressed(neko_keycode_z)) {
        LaunchBomb();
    }

    if (platform->key_pressed(neko_keycode_one)) {
        init_demo(1);
    }
    if (platform->key_pressed(neko_keycode_two)) {
        init_demo(2);
    }
    if (platform->key_pressed(neko_keycode_three)) {
        init_demo(3);
    }
    if (platform->key_pressed(neko_keycode_four)) {
        init_demo(4);
    }
    if (platform->key_pressed(neko_keycode_five)) {
        init_demo(5);
    }

    if (platform->key_down(neko_keycode_q)) {
        g_camera.ortho_scale += 0.1f;
    }
    if (platform->key_down(neko_keycode_e)) {
        g_camera.ortho_scale -= 0.1f;
    }
    if (platform->key_down(neko_keycode_a)) {
        g_camera.transform.position.x -= 0.1f;
    }
    if (platform->key_down(neko_keycode_d)) {
        g_camera.transform.position.x += 0.1f;
    }
    if (platform->key_down(neko_keycode_w)) {
        g_camera.transform.position.y += 0.1f;
    }
    if (platform->key_down(neko_keycode_s)) {
        g_camera.transform.position.y -= 0.1f;
    }

    f32 wx = 0, wy = 0;
    platform->mouse_wheel_x_y(&wx, &wy);
    if (platform->key_pressed(neko_keycode_lbracket) || wy < 0.f) {
        g_cvar.brush_size = neko_clamp(g_cvar.brush_size - 1.f, 1.f, 100.f);
    }
    if (platform->key_pressed(neko_keycode_rbracket) || wy > 0.f) {
        g_cvar.brush_size = neko_clamp(g_cvar.brush_size + 1.f, 1.f, 100.f);
    }

    if (platform->key_pressed(neko_keycode_p)) {
        g_cvar.tick_world = !g_cvar.tick_world;
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
        const f32 R = g_cvar.brush_size;

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
        const f32 R = g_cvar.brush_size;

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
    bool frame_counter_even = ((g_frame_counter % 2) == 0);
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
                    update_default(x, y);
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

bool in_rect(neko_vec2 p, neko_vec2 ro, neko_vec2 rd) {
    if (p.x < ro.x || p.x > ro.x + rd.x || p.y < ro.y || p.y > ro.y + rd.y) return false;
    return true;
}

bool gui_rect(cell_color_t *buffer, s32 _x, s32 _y, s32 _w, s32 _h, cell_color_t c) {
    neko_vec2 mp = calculate_mouse_position();

    for (u32 h = 0; h < _h; ++h) {
        for (u32 w = 0; w < _w; ++w) {
            if (in_bounds(_x + w, _y + h)) {
                buffer[compute_idx(_x + w, _y + h)] = c;
            }
        }
    }

    bool clicked = neko_engine_instance()->ctx.platform->mouse_pressed(neko_mouse_lbutton);

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

bool update_ui() {
    bool interaction = false;
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
        neko_snprintf(sim_state_str, sizeof(sim_state_str), "state: %s", g_cvar.tick_world ? "running" : "paused");
        draw_string_at(&g_font, g_ui_buffer, 10, 20, sim_state_str, strlen(sim_state_str), cell_color_t{255, 255, 255, 255});

        gfx->fontcache_push_x_y(std::format("test: {0} {1}", l_check, neko_buildnum()), g_basic_font, 40, 160);
    }

    // Draw circle around mouse pointer
    s32 R = g_cvar.brush_size;
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

    u8 *alpha = neko_tex_rgba_to_alpha((u8 *)g_texture_buffer, g_texture_width, g_texture_height);
    u8 *thresholded = neko_tex_alpha_to_thresholded(alpha, g_texture_width, g_texture_height, 90);
    u8 *outlined = neko_tex_thresholded_to_outlined(thresholded, g_texture_width, g_texture_height);
    neko_safe_free(alpha);
    neko_safe_free(thresholded);

    neko_tex_point *outline = neko_tex_extract_outline_path(outlined, g_texture_width, g_texture_height, &l, 0);
    while (l) {
        s32 l0 = l;
        neko_tex_distance_based_path_simplification(outline, &l, 0.5f);
        // printf("simplified outline: %d -> %d\n", l0, l);

        l_check = l;

        for (s32 i = 0; i < l; i++) {
            gfx->immediate.draw_line_ext(cb, neko_vec2_mul(last_point, {4.f, 4.f}), neko_vec2_mul(neko_vec2{(f32)outline[i].x, (f32)outline[i].y}, {4.f, 4.f}), 2.f, neko_color_white);

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

    neko_quad_batch_t *qb = &g_batch;
    neko_quad_batch_t *cqb = &g_custom_batch;
    neko_command_buffer_t *cb = &g_cb;

    const f32 _t = platform->elapsed_time();

    // Add some stuff to the quad batch
    gfx->quad_batch_begin(qb);
    {
        neko_for_range_i(100) {
            neko_for_range_j(100) {
                neko_default_quad_info_t quad_info = {0};
                quad_info.transform = neko_vqs_default();
                quad_info.transform.position = neko_vec3{(f32)i, (f32)j, 0.f};
                quad_info.uv = neko_vec4{0.f, 0.f, 1.f, 1.f};
                quad_info.color = i % 2 == 0 ? neko_vec4{1.f, 1.f, 1.f, 1.f} : neko_vec4{1.f, 0.f, 0.f, 1.f};
                gfx->quad_batch_add(qb, &quad_info);
            }
        }
    }
    gfx->quad_batch_end(qb);

#ifdef custom_batch
    // Add 10k items to batch
    gfx->quad_batch_begin(cqb);
    {
        neko_for_range_i(100) {
            neko_for_range_j(100) {
                // Instance of our custom quad batch info struct
                quad_batch_custom_info_t quad_info = {0};

                quad_info.transform = neko_vqs_default();
                quad_info.transform.position = neko_vec3{(f32)i, (f32)j, 0.f};
                quad_info.uv = neko_vec4{0.f, 0.f, 1.f, 1.f};
                quad_info.color = i % 2 == 0 ? neko_vec4{1.f, 1.f, 1.f, 1.f} : neko_vec4{1.f, 0.f, 0.f, 1.f};
                quad_info.color_two = neko_vec4{0.f, 1.f, 0.f, 1.f};

                gfx->quad_batch_add(cqb, &quad_info);
            }
        }
    }
    gfx->quad_batch_end(cqb);
#endif

    neko_shader_t _shader = *gfx->neko_shader_get("g_shader");

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
    const f32 t = platform->elapsed_time();
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
        gfx->bind_shader(cb, _shader);
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

        gfx->bind_shader(cb, _shader);
        gfx->bind_uniform(cb, u_flip_y, &flip_y);
        gfx->bind_vertex_buffer(cb, g_vbo);
        gfx->bind_index_buffer(cb, g_ibo);

        // Draw final composited image
        if (g_cvar.draw_shaders) {

            gfx->bind_texture(cb, u_tex, g_composite_pass.data.render_target, 0);
        } else {

            gfx->bind_texture(cb, u_tex, g_rt, 0);
        }
        gfx->draw_indexed(cb, 6, 0);

        // Draw UI on top
        gfx->bind_texture(cb, u_tex, g_tex_ui, 0);
        gfx->draw_indexed(cb, 6, 0);
    }

    {
        // Create model/view/projection matrices from camera
        neko_mat4 view_mtx = neko_camera_get_view(&g_camera);
        neko_mat4 proj_mtx = neko_camera_get_projection(&g_camera, ws.x, ws.y);
        neko_mat4 model_mtx = neko_mat4_scale(neko_vec3{1.f, 1.f, 1.f});

        // Set necessary dynamic uniforms for quad batch material (defined in default shader in neko_quad_batch.h)
        gfx->set_material_uniform_mat4(qb->material, "u_model", model_mtx);
        gfx->set_material_uniform_mat4(qb->material, "u_view", view_mtx);
        gfx->set_material_uniform_mat4(qb->material, "u_proj", proj_mtx);

        // Need to submit quad batch
        gfx->quad_batch_submit(cb, qb);
    }

#ifdef custom_batch
    {
        // Create model/view/projection matrices from camera
        neko_mat4 view_mtx = neko_camera_get_view(&g_camera);
        neko_mat4 proj_mtx = neko_camera_get_projection(&g_camera, ws.x, ws.y);
        neko_mat4 model_mtx = neko_mat4_scale(neko_vec3{1.f, 1.f, 1.f});

        // Set necessary dynamic uniforms for quad batch material (defined in default shader in neko_quad_batch.h)
        gfx->set_material_uniform_mat4(cqb->material, "u_model", model_mtx);
        gfx->set_material_uniform_mat4(cqb->material, "u_view", view_mtx);
        gfx->set_material_uniform_mat4(cqb->material, "u_proj", proj_mtx);
        gfx->set_material_uniform_float(cqb->material, "u_alpha", sin(t * 0.001f) * 0.5f + 0.5f);

        // Need to submit quad batch
        gfx->quad_batch_submit(cb, cqb);
    }
#endif

    // 3d
    {
        _shader = *gfx->neko_shader_get("g_3d_shader");

        // Bind shader
        gfx->bind_shader(cb, _shader);

        // Bind uniform for triangle color
        f32 color[4] = {sin(t * 0.001f), cos(t * 0.002f), 0.1f, 1.f};
        gfx->bind_uniform(cb, u_color, &color);

        // Create model/view/projection matrices
        neko_mat4 view_mtx = neko_mat4_mul(neko_mat4_rotate(15.f, neko_vec3{1.f, 0.f, 0.f}), neko_mat4_translate(neko_vec3{0.f, -0.9f, -2.5f}));
        // neko_mat4 proj_mtx = neko_mat4_perspective(60.f, ws.x / ws.y, 0.01f, 1000.f);

        // neko_mat4 view_mtx = neko_camera_get_view(&g_camera);
        neko_mat4 proj_mtx = neko_camera_get_projection(&g_camera, ws.x, ws.y);
        neko_mat4 model_mtx = neko_mat4_rotate(t * 0.01f, neko_vec3{0.f, 1.f, 0.f});

        // Bind matrix uniforms
        gfx->bind_uniform_mat4(cb, u_proj, proj_mtx);
        gfx->bind_uniform_mat4(cb, u_view, view_mtx);
        gfx->bind_uniform_mat4(cb, u_model, model_mtx);

        // Bind vertex buffer
        gfx->bind_vertex_buffer(cb, g_3d_vbo);

        // Draw
        gfx->draw(cb, 0, 36);
    }

    gfx->immediate.begin_drawing(cb);
    {

        neko_profiler_scope_auto("immediate_draw");

        neko_ecs_run_systems(neko_engine_subsystem(ecs), ECS_SYSTEM_RENDER_IMMEDIATE);

        gfx->immediate.begin_2d(cb);
        {
            swarm.draw(cb);

            // gfx->immediate.draw_line_ext(cb, {9.f, 9.f}, {400.f, 400.f}, 4.f, neko_color_white);
            // render_test();

            for (s32 i = 0; i < numBodies; ++i) DrawBody(bodies + i);

            for (s32 i = 0; i < numJoints; ++i) DrawJoint(joints + i);

            std::map<neko_phy_arbiter_key, neko_phy_arbiter>::const_iterator iter;
            for (iter = world.arbiters.begin(); iter != world.arbiters.end(); ++iter) {
                const neko_phy_arbiter &arbiter = iter->second;
                for (s32 i = 0; i < arbiter.numContacts; ++i) {
                    neko_vec2 p = arbiter.contacts[i].position;
                    // glVertex2f(p.x, p.y);
                    gfx->immediate.draw_circle(cb, world_to_screen_pos({p.x, p.y}), 5.f, 0, neko_color_green);
                }
            }
        }
        gfx->immediate.end_2d(cb);

        // neko_snprintfc(fps_text, 256, "fps: %.2f", 1000.f / platform->time.frame);
        // gfx->immediate.draw_text(cb, 410.f, 420.f, fps_text, neko_color_white);
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

    p->velocity.y = neko_clamp(p->velocity.y + (g_cvar.world_gravity * dt), -10.f, 10.f);

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
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_a = g_world_particle_data[read_idx];
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, tmp_a);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && (is_empty(x - 1, y + 1))) {
        u32 idx = compute_idx(x - 1, y + 1);
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_a = g_world_particle_data[read_idx];
        particle_t tmp_b = g_world_particle_data[idx];
        write_data(idx, tmp_a);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && (is_empty(x + 1, y + 1))) {
        u32 idx = compute_idx(x + 1, y + 1);
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y += (g_cvar.world_gravity * dt);
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

    p->velocity.y = neko_clamp(p->velocity.y + (g_cvar.world_gravity * dt), -10.f, 10.f);

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
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + 1);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && ((is_empty(x - 1, y + 1) || g_world_particle_data[bl_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y + 1);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && ((is_empty(x + 1, y + 1) || g_world_particle_data[br_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
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

    p->velocity.y = neko_clamp(p->velocity.y + (g_cvar.world_gravity * dt), -10.f, 10.f);
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
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + 1);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && ((is_empty(x - 1, y + 1) || g_world_particle_data[bl_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y + 1);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && ((is_empty(x + 1, y + 1) || g_world_particle_data[br_idx].id == mat_id_water))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
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
    p->velocity.y = neko_clamp(p->velocity.y - (g_cvar.world_gravity * dt), -2.f, 10.f);
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
        p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y - 1);
        write_data(compute_idx(x, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && ((is_empty(x - 1, y - 1) || get_particle_at(x - 1, y - 1).id == mat_id_water) || get_particle_at(x - 1, y - 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y - 1);
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && ((is_empty(x + 1, y - 1) || get_particle_at(x + 1, y - 1).id == mat_id_water) || get_particle_at(x + 1, y - 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (g_cvar.world_gravity * dt);
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
    p->velocity.y = neko_clamp(p->velocity.y - (g_cvar.world_gravity * dt), -2.f, 10.f);
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
        p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y - 1);
        write_data(compute_idx(x, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && get_particle_at(x - 1, y - 1).id != mat_id_smoke && get_particle_at(x - 1, y - 1).id != mat_id_wood && get_particle_at(x - 1, y - 1).id != mat_id_stone) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y - 1);
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && get_particle_at(x + 1, y - 1).id != mat_id_smoke && get_particle_at(x + 1, y - 1).id != mat_id_wood && get_particle_at(x + 1, y - 1).id != mat_id_stone) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (g_cvar.world_gravity * dt);
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

    p->velocity.y = neko_clamp(p->velocity.y - (g_cvar.world_gravity * dt), -2.f, 10.f);
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
        p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y - 1);
        write_data(compute_idx(x, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y - 1) && ((is_empty(x - 1, y - 1) || get_particle_at(x - 1, y - 1).id == mat_id_water) || get_particle_at(x - 1, y - 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x - 1, y - 1);
        write_data(compute_idx(x - 1, y - 1), *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y - 1) && ((is_empty(x + 1, y - 1) || get_particle_at(x + 1, y - 1).id == mat_id_water) || get_particle_at(x + 1, y + 1).id == mat_id_fire)) {
        p->velocity.x = random_val(0, 1) == 0 ? -1.2f : 1.2f;
        p->velocity.y -= (g_cvar.world_gravity * dt);
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
    p->velocity.y = neko_clamp(p->velocity.y - ((g_cvar.world_gravity * dt)) * 0.2f, -5.0f, 0.f);
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
        // p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = g_world_particle_data[compute_idx(vi_x, vi_y)];
        write_data(compute_idx(vi_x, vi_y), *p);
        write_data(read_idx, tmp_b);
    }

    // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
    else if (in_bounds(x, y + 1) && ((is_empty(x, y + 1) || (g_world_particle_data[b_idx].id == mat_id_water)))) {
        // p->velocity.y -= (g_cvar.world_gravity * dt);
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        particle_t tmp_b = g_world_particle_data[b_idx];
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x - 1, y + 1) && ((is_empty(x - 1, y + 1) || g_world_particle_data[bl_idx].id == mat_id_water))) {
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        // p->velocity.y -= (g_cvar.world_gravity * dt);
        particle_t tmp_b = g_world_particle_data[bl_idx];
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + 1, y + 1) && ((is_empty(x + 1, y + 1) || g_world_particle_data[br_idx].id == mat_id_water))) {
        // p->velocity.x = random_val(0, 1) == 0 ? -1.f : 1.f;
        // p->velocity.y -= (g_cvar.world_gravity * dt);
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

    p->velocity.y = neko_clamp(p->velocity.y + ((g_cvar.world_gravity * dt)), -10.f, 10.f);

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

    // If in water, then need to f32 upwards
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
        bool found = false;

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

    p->velocity.y = neko_clamp(p->velocity.y + (g_cvar.world_gravity * dt), -10.f, 10.f);

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

    // If in water, then need to f32 upwards
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
        bool found = false;

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

    p->velocity.y = neko_clamp(p->velocity.y + (g_cvar.world_gravity * dt), -10.f, 10.f);

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
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + u);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + l, y + u) && ((is_empty(x + l, y + u)))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x + l, y + u);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + r, y + u) && ((is_empty(x + r, y + u)))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x + r, y + u);
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    } else {
        particle_t tmp = *p;
        bool found = false;

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

    p->velocity.y = neko_clamp(p->velocity.y + (g_cvar.world_gravity * dt), -10.f, 10.f);

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
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x, y + u);
        write_data(b_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + l, y + u) && ((is_empty(x + l, y + u) || g_world_particle_data[bl_idx].id == mat_id_oil))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x + l, y + u);
        write_data(bl_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (in_bounds(x + r, y + u) && ((is_empty(x + r, y + u) || g_world_particle_data[br_idx].id == mat_id_oil))) {
        p->velocity.x = is_in_liquid(x, y, &lx, &ly) ? 0.f : random_val(0, 1) == 0 ? -1.f : 1.f;
        p->velocity.y += (g_cvar.world_gravity * dt);
        particle_t tmp_b = get_particle_at(x + r, y + u);
        write_data(br_idx, *p);
        write_data(read_idx, tmp_b);
    } else if (is_in_liquid(x, y, &lx, &ly) && random_val(0, 10) == 0) {
        particle_t tmp_b = get_particle_at(lx, ly);
        write_data(compute_idx(lx, ly), *p);
        write_data(read_idx, tmp_b);
    } else {
        particle_t tmp = *p;
        bool found = false;

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
u8 *neko_tex_rgba_to_alpha(const u8 *data, s32 w, s32 h) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[(y * w + x) * 4 + 3];
    return result;
}

u8 *neko_tex_alpha_to_thresholded(const u8 *data, s32 w, s32 h, u8 threshold) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
    return result;
}

u8 *neko_tex_dilate_thresholded(const u8 *data, s32 w, s32 h) {
    s32 x, y, dx, dy, cx, cy;
    u8 *result = (u8 *)neko_safe_malloc(w * h);
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

u8 *neko_tex_thresholded_to_outlined(const u8 *data, s32 w, s32 h) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
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
static neko_tex_bool neko_tex_find_first_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point *first) {
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

static neko_tex_bool neko_tex_find_next_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point current, neko_tex_direction *dir, neko_tex_point *next) {
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

neko_tex_point *neko_tex_extract_outline_path(u8 *data, s32 w, s32 h, s32 *point_count, neko_tex_point *reusable_outline) {
    neko_tex_point *outline = reusable_outline;
    if (!outline) outline = (neko_tex_point *)neko_safe_malloc(w * h * sizeof(neko_tex_point));

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

void neko_tex_distance_based_path_simplification(neko_tex_point *outline, s32 *outline_length, f32 distance_threshold) {
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

void print_array(neko_dyn_array(object_t) * arr) {
    char buffer[256] = {0};

    // Array
    neko_println("Array: [ ");

    // Loop through all elements of array
    u32 sz = neko_dyn_array_size(*arr);
    for (u32 i = 0; i < sz; ++i) {
        object_to_str(&(*arr)[i], buffer, 256);
        neko_println("\t%s", buffer);
    }
    neko_printf(" ]\n");
}

void print_slot_array(neko_slot_array(object_t) * sa) {
    // Slot Array
    neko_println("Slot Array: [ ");

    char buffer[256] = {0};

    for (neko_slot_array_iter(object_t) it = neko_slot_array_iter_new(*sa); neko_slot_array_iter_valid(*sa, it); neko_slot_array_iter_advance(*sa, it)) {
        object_t *obj = it.data;
        object_to_str(obj, buffer, 256);
        neko_println("\t%s, %zu", buffer, it.cur_idx);
    }

    neko_printf(" ]\n");
}

void print_hash_table(neko_hash_table(u64, object_t) * ht) {
    // Temp buffer for printing object
    char tmp_buffer[256] = {0};

    // Hash Table
    neko_println("Hash Table: [ ");

    // Use iterator to iterate through hash table and print elements
    for (neko_hash_table_iter(u64, object_t) it = neko_hash_table_iter_new(*ht); neko_hash_table_iter_valid(*ht, it); neko_hash_table_iter_advance(*ht, it)) {
        u64 k = it.data->key;
        object_t *v = &it.data->val;
        object_to_str(v, tmp_buffer, 256);
        neko_println("\t{ k: %zu, %s } ", k, tmp_buffer);
    }

    neko_println("]");
}

void object_to_str(object_t *obj, char *str, usize str_sz) { neko_snprintf(str, str_sz, "{ %.2f, %zu }", obj->float_value, obj->uint_value); }

#define STB_HBWANG_RAND() neko_rand_xorshf32()
#define STB_HBWANG_IMPLEMENTATION
#include "libs/stb/stb_herringbone_wang_tile.h"
#include "libs/stb/stb_image.h"
#include "libs/stb/stb_image_write.h"

void genwang(std::string filename, unsigned char *data, s32 xs, s32 ys, s32 w, s32 h) {
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
    auto buff = static_cast<unsigned char *>(malloc(3 * xs * yimg));
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

    unsigned char *data = stbi_load(neko_file_path("data/assets/textures/wang_test.png"), &w, &h, NULL, 3);

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

s32 main(s32 argc, char **argv) {

    neko_engine *engine = neko_engine_construct(argc, argv);
    if (nullptr != engine) {
        neko_result result = engine->engine_run();
        if (result != neko_result_success) {
            neko_warn("engine did not successfully finish running.");
            return -1;
        }
        neko_info("good");
    } else {
        neko_info("bad");
    }

    return 0;
}
