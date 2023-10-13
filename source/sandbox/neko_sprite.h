
#ifndef NEKO_SPRITE_H
#define NEKO_SPRITE_H

#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_engine.h"

// game
#include "neko_hash.h"

struct neko_sprite_frame {
    s32 duration;
    f32 u0, v0, u1, v1;
};

struct neko_sprite_loop {
    neko_array<s32> indices;
};

struct neko_sprite;

struct neko_sprite_renderer {
    neko_sprite* sprite;
    neko_sprite_loop* loop;
    f32 elapsed;
    s32 current_frame;
};

struct neko_sprite {
    neko_array<neko_sprite_frame> frames;
    neko_hashmap<neko_sprite_loop> by_tag;
    neko_texture_t img;
    s32 width;
    s32 height;

#ifdef NEKO_DEBUG
    u64 mem_used;
#endif
};

bool neko_sprite_load(neko_sprite* spr, const neko_string& filepath);
void neko_sprite_end(neko_sprite* spr);
void neko_sprite_renderer_play(neko_sprite_renderer* sr, const neko_string& tag);
void neko_sprite_renderer_update(neko_sprite_renderer* sr, f32 dt);
void neko_sprite_renderer_set_frame(neko_sprite_renderer* sr, s32 frame);

struct neko_particle_t {
    f64 x;
    f64 y;
    f64 speed;
    f64 direction;
};

struct neko_particle_renderer {
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
};

void neko_particle_ctor(neko_particle_t* par);
void neko_particle_update(neko_particle_t* par, int interval, float spin, float speed);
void neko_particle_draw(neko_particle_t* par, neko_command_buffer_t* cb, CTransform* trans, neko_color_t color, f32 render_radius, int particle_radius);

void neko_particle_renderer_construct(neko_particle_renderer* pr);
void neko_particle_renderer_free(neko_particle_renderer* pr);
void neko_particle_renderer_update(neko_particle_renderer* pr, int elapsed);
void neko_particle_renderer_draw(neko_particle_renderer* pr, neko_command_buffer_t* cb, CTransform* trans);

typedef struct neko_fast_sprite_renderer {
    neko_handle(neko_graphics_vertex_buffer_t) vbo = {0};
    neko_handle(neko_graphics_index_buffer_t) ibo = {0};
    neko_handle(neko_graphics_pipeline_t) pip = {0};
    neko_handle(neko_graphics_shader_t) shader = {0};
    neko_handle(neko_graphics_uniform_t) u_tex = {0};
    neko_handle(neko_graphics_texture_t) tex = {0};

} neko_fast_sprite_renderer;

void neko_neko_fast_sprite_renderer_construct(neko_fast_sprite_renderer* render, u32 width = 0, u32 height = 0, void* data = NULL);
void neko_neko_fast_sprite_renderer_draw(neko_fast_sprite_renderer* render, neko_command_buffer_t* cb);

#pragma region neko_ase

typedef struct ase_t ase_t;

ase_t* neko_aseprite_load_from_file(const char* path);
ase_t* neko_aseprite_load_from_memory(const void* memory, int size);
void neko_aseprite_free(ase_t* aseprite);
void neko_aseprite_default_blend_bind(ase_t* aseprite);  // 默认图块混合管线

#define __NEKO_ASEPRITE_MAX_LAYERS (64)
#define __NEKO_ASEPRITE_MAX_SLICES (128)
#define __NEKO_ASEPRITE_MAX_PALETTE_ENTRIES (1024)
#define __NEKO_ASEPRITE_MAX_TAGS (256)

typedef struct ase_frame_t ase_frame_t;
typedef struct ase_layer_t ase_layer_t;
typedef struct ase_cel_t ase_cel_t;
typedef struct ase_tag_t ase_tag_t;
typedef struct ase_slice_t ase_slice_t;
typedef struct ase_palette_entry_t ase_palette_entry_t;
typedef struct ase_palette_t ase_palette_t;
typedef struct ase_udata_t ase_udata_t;
typedef struct ase_cel_extra_chunk_t ase_cel_extra_chunk_t;
typedef struct ase_color_profile_t ase_color_profile_t;
typedef struct ase_fixed_t ase_fixed_t;
typedef struct ase_cel_extra_chunk_t ase_cel_extra_chunk_t;

struct ase_fixed_t {
    u16 a;
    u16 b;
};

struct ase_udata_t {
    int has_color;
    neko_color_t color;
    int has_text;
    const char* text;
};

typedef enum ase_layer_flags_t {
    NEKO_ASE_LAYER_FLAGS_VISIBLE = 0x01,
    NEKO_ASE_LAYER_FLAGS_EDITABLE = 0x02,
    NEKO_ASE_LAYER_FLAGS_LOCK_MOVEMENT = 0x04,
    NEKO_ASE_LAYER_FLAGS_BACKGROUND = 0x08,
    NEKO_ASE_LAYER_FLAGS_PREFER_LINKED_CELS = 0x10,
    NEKO_ASE_LAYER_FLAGS_COLLAPSED = 0x20,
    NEKO_ASE_LAYER_FLAGS_REFERENCE = 0x40,
} ase_layer_flags_t;

typedef enum ase_layer_type_t {
    NEKO_ASE_LAYER_TYPE_NORMAL,
    NEKO_ASE_LAYER_TYPE_GROUP,
    NEKO_ASE_LAYER_TYPE_TILEMAP,
} ase_layer_type_t;

struct ase_layer_t {
    ase_layer_flags_t flags;
    ase_layer_type_t type;
    const char* name;
    ase_layer_t* parent;
    float opacity;
    ase_udata_t udata;
};

struct ase_cel_extra_chunk_t {
    int precise_bounds_are_set;
    ase_fixed_t precise_x;
    ase_fixed_t precise_y;
    ase_fixed_t w, h;
};

struct ase_cel_t {
    ase_layer_t* layer;
    void* cel_pixels;  // 图块的pixels数据是唯一的
    int w, h;
    int x, y;
    float opacity;
    int is_linked;
    u16 linked_frame_index;
    int has_extra;
    ase_cel_extra_chunk_t extra;
    ase_udata_t udata;
};

struct ase_frame_t {
    ase_t* ase;
    int duration_milliseconds;
    int pixel_count;
    neko_color_t* pixels[__NEKO_ASEPRITE_MAX_LAYERS];  // 支持每层一个pixels数据
    int cel_count;
    ase_cel_t cels[__NEKO_ASEPRITE_MAX_LAYERS];
};

typedef enum ase_animation_direction_t {
    NEKO_ASE_ANIMATION_DIRECTION_FORWARDS,
    NEKO_ASE_ANIMATION_DIRECTION_BACKWORDS,
    NEKO_ASE_ANIMATION_DIRECTION_PINGPONG,
} ase_animation_direction_t;

struct ase_tag_t {
    int from_frame;
    int to_frame;
    ase_animation_direction_t loop_animation_direction;
    u8 r, g, b;
    const char* name;
    ase_udata_t udata;
};

struct ase_slice_t {
    const char* name;
    int frame_number;
    int origin_x;
    int origin_y;
    int w, h;

    int has_center_as_9_slice;
    int center_x;
    int center_y;
    int center_w;
    int center_h;

    int has_pivot;
    int pivot_x;
    int pivot_y;

    ase_udata_t udata;
};

struct ase_palette_entry_t {
    neko_color_t color;
    const char* color_name;
};

struct ase_palette_t {
    int entry_count;
    ase_palette_entry_t entries[__NEKO_ASEPRITE_MAX_PALETTE_ENTRIES];
};

typedef enum ase_color_profile_type_t {
    NEKO_ASE_COLOR_PROFILE_TYPE_NONE,
    NEKO_ASE_COLOR_PROFILE_TYPE_SRGB,
    NEKO_ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC,
} ase_color_profile_type_t;

struct ase_color_profile_t {
    ase_color_profile_type_t type;
    int use_fixed_gamma;
    ase_fixed_t gamma;
    u32 icc_profile_data_length;
    void* icc_profile_data;
};

typedef enum ase_mode_t { NEKO_ASE_MODE_RGBA, NEKO_ASE_MODE_GRAYSCALE, NEKO_ASE_MODE_INDEXED } ase_mode_t;

struct ase_t {
    ase_mode_t mode;
    int w, h;
    int transparent_palette_entry_index;
    int number_of_colors;
    int pixel_w;
    int pixel_h;
    int grid_x;
    int grid_y;
    int grid_w;
    int grid_h;
    int has_color_profile;
    ase_color_profile_t color_profile;
    ase_palette_t palette;

    int layer_count;
    ase_layer_t layers[__NEKO_ASEPRITE_MAX_LAYERS];

    int frame_count;
    ase_frame_t* frames;

    int tag_count;
    ase_tag_t tags[__NEKO_ASEPRITE_MAX_TAGS];

    int slice_count;
    ase_slice_t slices[__NEKO_ASEPRITE_MAX_SLICES];
};

neko_inline int s_mul_un8(int a, int b) {
    int t = (a * b) + 0x80;
    return (((t >> 8) + t) >> 8);
}

neko_inline neko_color_t s_blend(neko_color_t src, neko_color_t dst, u8 opacity) {
    src.a = (u8)s_mul_un8(src.a, opacity);
    int a = src.a + dst.a - s_mul_un8(src.a, dst.a);
    int r, g, b;
    if (a == 0) {
        r = g = b = 0;
    } else {
        r = dst.r + (src.r - dst.r) * src.a / a;
        g = dst.g + (src.g - dst.g) * src.a / a;
        b = dst.b + (src.b - dst.b) * src.a / a;
    }
    neko_color_t ret = {(u8)r, (u8)g, (u8)b, (u8)a};
    return ret;
}

neko_inline neko_color_t s_color(ase_t* ase, void* src, int index) {
    neko_color_t result;
    if (ase->mode == NEKO_ASE_MODE_RGBA) {
        result = ((neko_color_t*)src)[index];
    } else if (ase->mode == NEKO_ASE_MODE_GRAYSCALE) {
        u8 saturation = ((u8*)src)[index * 2];
        u8 a = ((u8*)src)[index * 2 + 1];
        result.r = result.g = result.b = saturation;
        result.a = a;
    } else {
        neko_assert(ase->mode == NEKO_ASE_MODE_INDEXED);
        u8 palette_index = ((u8*)src)[index];
        if (palette_index == ase->transparent_palette_entry_index) {
            result = {0, 0, 0, 0};
        } else {
            result = ase->palette.entries[palette_index].color;
        }
    }
    return result;
}

#pragma endregion

#endif
