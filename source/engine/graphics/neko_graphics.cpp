#include "engine/graphics/neko_graphics.h"

#include <map>

#include "engine/base/neko_engine.h"
#include "engine/common/neko_mem.h"
#include "engine/graphics/neko_camera.h"
#include "engine/graphics/neko_material.h"
#include "engine/platform/neko_platform.h"
#include "engine/utility/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <libs/stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <libs/stb/stb_image_write.h>

// #define STB_TRUETYPE_IMPLEMENTATION
#include <libs/stb/stb_truetype.h>

// Error rate to calculate how many segments we need to draw a smooth circle,
// taken from https://stackoverflow.com/a/2244088
#ifndef smooth_circle_error_rate
#define smooth_circle_error_rate 0.5f
#endif

const f32 deg2rad = neko_pi / 180.f;

neko_texture_parameter_desc neko_texture_parameter_desc_default() {
    neko_texture_parameter_desc desc = {};

    desc.texture_wrap_s = neko_repeat;
    desc.texture_wrap_t = neko_repeat;
    desc.min_filter = neko_linear;
    desc.mag_filter = neko_linear;
    desc.mipmap_filter = neko_linear;
    desc.generate_mips = true;
    desc.border_color[0] = 0.f;
    desc.border_color[1] = 0.f;
    desc.border_color[2] = 0.f;
    desc.border_color[3] = 0.f;
    desc.data = NULL;
    desc.texture_format = neko_texture_format_rgba8;
    desc.width = 0;
    desc.height = 0;
    desc.num_comps = 0;
    desc.flip_vertically_on_load = true;

    return desc;
}

void* neko_load_texture_data_from_file(const char* file_path, b32 flip_vertically_on_load) {
    // Load texture data
    stbi_set_flip_vertically_on_load(flip_vertically_on_load);

    // For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param
    // Could optimize this later
    s32 width, height, num_comps;
    void* texture_data = stbi_load(file_path, (s32*)&width, (s32*)&height, &num_comps, STBI_rgb_alpha);

    if (!texture_data) {
        neko_println("Warning: could not load texture: %s", file_path);
        return NULL;
    }

    return texture_data;
}

neko_resource(neko_font_t) __neko_construct_font_from_file(const char* file_path, f32 point_size) {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_font_t f = neko_default_val();

    stbtt_fontinfo font = neko_default_val();
    char* ttf = platform->read_file_contents(file_path, "rb", NULL);
    const u32 w = 512;
    const u32 h = 512;
    const u32 num_comps = 4;
    u8* alpha_bitmap = (u8*)neko_safe_malloc(w * h);
    u8* flipmap = (u8*)neko_safe_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);

    // 只加载 ascii
    s32 v = stbtt_BakeFontBitmap((u8*)ttf, 0, point_size, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f.glyphs);

    // Flip texture
    u32 r = h - 1;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            u32 i0 = i * w + j;
            u32 i1 = r * w * num_comps + j * num_comps;
            u8 a = alpha_bitmap[i0];
            flipmap[i1 + 0] = 255;
            flipmap[i1 + 1] = 255;
            flipmap[i1 + 2] = 255;
            flipmap[i1 + 3] = a;
        }
        r--;
    }

    neko_texture_parameter_desc desc = neko_texture_parameter_desc_default();
    desc.width = w;
    desc.height = h;
    desc.num_comps = num_comps;
    desc.data = flipmap;
    desc.texture_format = neko_texture_format_rgba8;
    desc.min_filter = neko_linear;

    // Generate atlas texture for bitmap with bitmap data
    f.texture = gfx->construct_texture(desc);

    if (v == 0) {
        neko_error(std::format("font failed to load: {0}, {1}", file_path, v));
    } else {
        neko_info(std::format("font successfully load: {0}, {1}", file_path, v));
    }

    neko_safe_free(ttf);
    neko_safe_free(alpha_bitmap);
    neko_safe_free(flipmap);

    neko_resource(neko_font_t) handle = neko_resource_cache_insert(gfx->font_cache, f);
    return handle;
}

neko_vec2 __neko_text_dimensions(const char* text, neko_resource(neko_font_t) handle) {
    neko_graphics_i* gfx = neko_engine_subsystem(graphics);
    neko_font_t* ft = neko_resource_cache_get_ptr(gfx->font_cache, handle);
    neko_vec2 pos = neko_v2(0.f, 0.f);
    neko_vec2 dims = neko_v2(pos.x, pos.y);
    while (text[0] != '\0') {
        char c = text[0];
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q = neko_default_val();
            stbtt_GetBakedQuad((stbtt_bakedchar*)ft->glyphs, ft->texture.width, ft->texture.height, c - 32, &pos.x, &pos.y, &q, 1);
            dims.x = pos.x;
            dims.y = neko_max(dims.y, (q.y1 - q.y0));
        }
        text++;
    }
    return dims;
}

void neko_rgb_to_hsv(u8 r, u8 g, u8 b, f32* h, f32* s, f32* v) {
    f32 fR = (f32)r / 255.f;
    f32 fG = (f32)g / 255.f;
    f32 fB = (f32)b / 255.f;

    f32 fCMax = neko_max(neko_max(fR, fG), fB);
    f32 fCMin = neko_min(neko_min(fR, fG), fB);
    f32 fDelta = fCMax - fCMin;

    if (fDelta > 0.f) {

        if (fCMax == fR) {
            *h = 60.f * (fmod(((fG - fB) / fDelta), 6));
        } else if (fCMax == fG) {
            *h = 60.f * (((fB - fR) / fDelta) + 2);
        } else if (fCMax == fB) {
            *h = 60.f * (((fR - fG) / fDelta) + 4);
        }

        if (fCMax > 0.f) {
            *s = fDelta / fCMax;
        } else {
            *s = 0;
        }

        *v = fCMax;
    } else {
        *h = 0;
        *s = 0;
        *v = fCMax;
    }

    if (*h < 0.f) {
        *h = 360.f + *h;
    }
}

void neko_hsv_to_rgb(f32 h, f32 s, f32 v, u8* r, u8* g, u8* b) {
    f32 fC = v * s;  // Chroma
    f32 fHPrime = fmod(h / 60.0, 6);
    f32 fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    f32 fM = v - fC;

    if (0 <= fHPrime && fHPrime < 1) {
        *r = fC;
        *g = fX;
        *b = 0;
    } else if (1 <= fHPrime && fHPrime < 2) {
        *r = fX;
        *g = fC;
        *b = 0;
    } else if (2 <= fHPrime && fHPrime < 3) {
        *r = 0;
        *g = fC;
        *b = fX;
    } else if (3 <= fHPrime && fHPrime < 4) {
        *r = 0;
        *g = fX;
        *b = fC;
    } else if (4 <= fHPrime && fHPrime < 5) {
        *r = fX;
        *g = 0;
        *b = fC;
    } else if (5 <= fHPrime && fHPrime < 6) {
        *r = fC;
        *g = 0;
        *b = fX;
    } else {
        *r = 0;
        *g = 0;
        *b = 0;
    }

    *r += fM;
    *g += fM;
    *b += fM;
}

/*=====================
// Uniform Block
======================*/

neko_resource(neko_uniform_block_t) __neko_uniform_block_t_new() {
    neko_uniform_block_i* uapi = neko_engine_instance()->ctx.graphics->uniform_i;
    neko_uniform_block_t ub = {0};
    ub.data = neko_byte_buffer_new();
    ub.offset_lookup_table = neko_hash_table_new(u64, u32);
    ub.uniforms = neko_hash_table_new(u64, neko_uniform_t);
    ub.count = 0;
    neko_resource(neko_uniform_block_t) res = {0};
    res.id = neko_slot_array_insert(uapi->uniform_blocks, ub);
    return res;
}

void __neko_uniform_block_t_set_uniform_from_shader(neko_resource(neko_uniform_block_t) u_block_h, neko_shader_t shader, neko_uniform_type type, const char* name, ...) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_uniform_block_i* uapi = gfx->uniform_i;

    // Either look for uniform or construct it
    // Look for uniform name in uniforms
    // Grab uniform from uniforms
    u64 hash_id = neko_hash_str64(name);
    neko_uniform_t uniform = {};
    neko_uniform_block_t* u_block = neko_slot_array_get_ptr(uapi->uniform_blocks, u_block_h.id);

    if (!neko_hash_table_exists(u_block->offset_lookup_table, hash_id)) {
        // Construct or get existing uniform
        uniform = gfx->construct_uniform(shader, name, type);

        // Insert buffer position into offset table (which should be end)
        neko_hash_table_insert(u_block->uniforms, hash_id, uniform);
    } else {
        uniform = neko_hash_table_get(u_block->uniforms, hash_id);
    }

    usize sz = 0;
    switch (type) {
        case neko_uniform_type_mat4: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(mat4) data = neko_uniform_block_type(mat4){va_arg(ap, neko_mat4)};
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(mat4)));
        } break;

        case neko_uniform_type_vec4: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(vec4) data = neko_uniform_block_type(vec4){va_arg(ap, neko_vec4)};
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(vec4)));
        } break;

        case neko_uniform_type_vec3: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(vec3) data = neko_uniform_block_type(vec3){va_arg(ap, neko_vec3)};
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(vec3)));
        } break;

        case neko_uniform_type_vec2: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(vec2) data = neko_uniform_block_type(vec2){va_arg(ap, neko_vec2)};
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(vec2)));
        } break;

        case neko_uniform_type_float: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(float) data = neko_uniform_block_type(float){(float)va_arg(ap, double)};
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(float)));
        } break;

        case neko_uniform_type_int: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(int) data = neko_uniform_block_type(int){va_arg(ap, int)};
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(int)));
        } break;

        case neko_uniform_type_sampler2d: {
            va_list ap;
            va_start(ap, name);
            neko_uniform_block_type(texture_sampler) data = {0};
            data.data = va_arg(ap, u32);
            data.slot = va_arg(ap, u32);
            va_end(ap);
            uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(neko_uniform_block_type(texture_sampler)));
        } break;
    };
}

void __neko_uniform_block_t_set_uniform(neko_resource(neko_uniform_block_t) u_block_h, neko_uniform_t uniform, const char* name, void* data, usize data_size) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    // Check for offset based on name, if doesn't exist create it and pass
    u64 hash_id = neko_hash_str64(name);
    b32 exists = true;

    neko_uniform_block_t* u_block = neko_slot_array_get_ptr(gfx->uniform_i->uniform_blocks, u_block_h.id);

    if (!neko_hash_table_exists(u_block->offset_lookup_table, hash_id)) {
        exists = false;

        // Seek to end of buffer
        neko_byte_buffer_seek_to_end(&u_block->data);

        // Insert buffer position into offset table (which should be end)
        neko_hash_table_insert(u_block->offset_lookup_table, hash_id, u_block->data.position);

        u_block->count++;
    }

    // Otherwise, we're going to overwrite existing data
    u32 offset = neko_hash_table_get(u_block->offset_lookup_table, hash_id);

    // Set buffer to offset position
    u_block->data.position = offset;

    // Write type into data
    neko_byte_buffer_write(&u_block->data, neko_uniform_t, uniform);
    // Write data size
    neko_byte_buffer_write(&u_block->data, usize, data_size);
    // Write data
    neko_byte_buffer_bulk_write(&u_block->data, data, data_size);

    // Subtract sizes since we were overwriting data and not appending if already exists
    if (exists) {
        usize total_size = sizeof(neko_uniform_t) + sizeof(usize) + data_size;
        u_block->data.size -= total_size;
    }

    // Seek back to end
    neko_byte_buffer_seek_to_end(&u_block->data);
}

void __neko_uniform_block_t_bind_uniforms(neko_command_buffer_t* cb, neko_resource(neko_uniform_block_t) uniforms) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_uniform_block_t* u_block = neko_slot_array_get_ptr(gfx->uniform_i->uniform_blocks, uniforms.id);

    // Set data to beginning
    neko_byte_buffer_seek_to_beg(&u_block->data);

    // Rip through uniforms, determine size, then bind accordingly
    neko_for_range_i(u_block->count) {
        // Grab uniform
        neko_uniform_t uniform;
        neko_byte_buffer_bulk_read(&u_block->data, &uniform, sizeof(neko_uniform_t));
        // Grab data size
        usize sz;
        neko_byte_buffer_read(&u_block->data, usize, &sz);
        // Grab type
        neko_uniform_type type = uniform.type;

        // Grab data based on type and bind
        switch (type) {
            case neko_uniform_type_sampler2d: {
                neko_uniform_block_type(texture_sampler) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_texture_id(cb, uniform, value.data, value.slot);
            } break;

            case neko_uniform_type_mat4: {
                neko_uniform_block_type(mat4) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_uniform(cb, uniform, &value.data);
            } break;

            case neko_uniform_type_vec4: {
                neko_uniform_block_type(vec4) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_uniform(cb, uniform, &value.data);
            } break;

            case neko_uniform_type_vec3: {
                neko_uniform_block_type(vec3) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_uniform(cb, uniform, &value.data);
            } break;

            case neko_uniform_type_vec2: {
                neko_uniform_block_type(vec2) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_uniform(cb, uniform, &value.data);
            } break;

            case neko_uniform_type_float: {
                neko_uniform_block_type(float) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_uniform(cb, uniform, &value.data);
            } break;

            case neko_uniform_type_int: {
                neko_uniform_block_type(int) value = {0};
                neko_byte_buffer_bulk_read(&u_block->data, &value, sz);
                gfx->bind_uniform(cb, uniform, &value.data);
            } break;
        }
    }
}

neko_uniform_block_i __neko_uniform_block_i_new() {
    neko_uniform_block_i api = {0};
    api.construct = &__neko_uniform_block_t_new;
    api.set_uniform = &__neko_uniform_block_t_set_uniform;
    api.set_uniform_from_shader = &__neko_uniform_block_t_set_uniform_from_shader;
    api.bind_uniforms = &__neko_uniform_block_t_bind_uniforms;
    api.uniform_blocks = neko_slot_array_new(neko_uniform_block_t);
    return api;
}

/*===============================
// Immediate Operations
===============================*/

// Line as a quad (not sure about this, actually)
// Might want to use GL_LINES instead
void __neko_draw_line_3d_ext(neko_command_buffer_t* cb, neko_vec3 start, neko_vec3 end, neko_vec3 normal, f32 thickness, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    // Need cross for left and right on the plane
    neko_vec3 cross = neko_vec3_scale(neko_vec3_norm(neko_vec3_cross(neko_vec3_norm(neko_vec3_sub(end, start)), normal)), thickness / 2.f);

    neko_vec3 tl = neko_vec3_add(start, cross);  // 0
    neko_vec3 tr = neko_vec3_sub(start, cross);  // 1
    neko_vec3 bl = neko_vec3_add(end, cross);    // 2
    neko_vec3 br = neko_vec3_sub(end, cross);    // 3

    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.color_ubv(cb, color);
        gfx->immediate.disable_texture_2d(cb);
        gfx->immediate.vertex_3fv(cb, tl);
        gfx->immediate.vertex_3fv(cb, br);
        gfx->immediate.vertex_3fv(cb, bl);
        gfx->immediate.vertex_3fv(cb, tl);
        gfx->immediate.vertex_3fv(cb, tr);
        gfx->immediate.vertex_3fv(cb, br);
    }
    gfx->immediate.end(cb);
}

// Thickness line
void __neko_draw_line_2d_ext(neko_command_buffer_t* cb, neko_vec2 s, neko_vec2 e, f32 thickness, neko_color_t color) {
    __neko_draw_line_3d_ext(cb, neko_v3(s.x, s.y, 0.f), neko_v3(e.x, e.y, 0.f), neko_v3(0.f, 0.f, -1.f), thickness, color);
}

void __neko_draw_line_3d(neko_command_buffer_t* cb, neko_vec3 s, neko_vec3 e, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_lines);
    {
        gfx->immediate.color_ubv(cb, color);
        gfx->immediate.disable_texture_2d(cb);
        gfx->immediate.vertex_3fv(cb, s);
        gfx->immediate.vertex_3fv(cb, e);
    }
    gfx->immediate.end(cb);
}

void __neko_draw_line_2d(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color) { __neko_draw_line_3d(cb, neko_v3(x0, y0, 0.f), neko_v3(x1, y1, 0.f), color); }

void __neko_draw_triangle_3d(neko_command_buffer_t* cb, neko_vec3 a, neko_vec3 b, neko_vec3 c, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.color_ubv(cb, color);
        gfx->immediate.disable_texture_2d(cb);
        gfx->immediate.vertex_3fv(cb, a);
        gfx->immediate.vertex_3fv(cb, b);
        gfx->immediate.vertex_3fv(cb, c);
    }
    gfx->immediate.end(cb);
}

void __neko_draw_triangle_3d_ext(neko_command_buffer_t* cb, neko_vec3 a, neko_vec3 b, neko_vec3 c, neko_mat4 m, neko_color_t color) {
    neko_vec4 _a = neko_mat4_mul_vec4(m, neko_v4(a.x, a.y, a.z, 1.f));
    neko_vec4 _b = neko_mat4_mul_vec4(m, neko_v4(b.x, b.y, b.z, 1.f));
    neko_vec4 _c = neko_mat4_mul_vec4(m, neko_v4(c.x, c.y, c.z, 1.f));
    __neko_draw_triangle_3d(cb, neko_v3(_a.x, _a.y, _a.z), neko_v3(_b.x, _b.y, _b.z), neko_v3(_c.x, _c.y, _c.z), color);
}

void __neko_draw_triangle_2d(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color) {
    __neko_draw_triangle_3d(cb, neko_v3(a.x, a.y, 0.f), neko_v3(b.x, b.y, 0.f), neko_v3(c.x, c.y, 0.f), color);
}

// Draw a plane
void __neko_draw_rect_3d(neko_command_buffer_t* cb, neko_vec3 p, neko_vec3 n, neko_color_t color) {
    // Most intuitive way to draw a plane?
    // 3 points? 2 points (corners) and a normal?
    // Normal with a transformation matrix? (applied to a rect)
    // How to do da plane?
    // point, normal, scale?
}

void __neko_draw_rect_2d_impl(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color) {
    neko_vec3 tl = neko_v3(a.x, a.y, 0.f);
    neko_vec3 tr = neko_v3(b.x, a.y, 0.f);
    neko_vec3 bl = neko_v3(a.x, b.y, 0.f);
    neko_vec3 br = neko_v3(b.x, b.y, 0.f);

    neko_vec2 tl_uv = neko_v2(uv0.x, uv1.y);
    neko_vec2 tr_uv = neko_v2(uv1.x, uv1.y);
    neko_vec2 bl_uv = neko_v2(uv0.x, uv0.y);
    neko_vec2 br_uv = neko_v2(uv1.x, uv0.y);

    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.color_ubv(cb, color);

        gfx->immediate.texcoord_2fv(cb, tl_uv);
        gfx->immediate.vertex_3fv(cb, tl);

        gfx->immediate.texcoord_2fv(cb, br_uv);
        gfx->immediate.vertex_3fv(cb, br);

        gfx->immediate.texcoord_2fv(cb, bl_uv);
        gfx->immediate.vertex_3fv(cb, bl);

        gfx->immediate.texcoord_2fv(cb, tl_uv);
        gfx->immediate.vertex_3fv(cb, tl);

        gfx->immediate.texcoord_2fv(cb, tr_uv);
        gfx->immediate.vertex_3fv(cb, tr);

        gfx->immediate.texcoord_2fv(cb, br_uv);
        gfx->immediate.vertex_3fv(cb, br);
    }
    gfx->immediate.end(cb);
}

void __neko_draw_rect_2d_lines_impl(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color) {
    __neko_draw_line_2d(cb, x0, y0, x1, y0, color);  // tl -> tr
    __neko_draw_line_2d(cb, x1, y0, x1, y1, color);  // tr -> br
    __neko_draw_line_2d(cb, x1, y1, x0, y1, color);  // br -> bl
    __neko_draw_line_2d(cb, x0, y1, x0, y0, color);  // bl -> tl
}

void __neko_draw_rect_2d_lines(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_rect_2d_lines_impl(cb, x0, y0, x1, y1, color);
}

void __neko_draw_rect_2d(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_rect_2d_impl(cb, neko_v2(x0, y0), neko_v2(x1, y1), neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), color);
}

void __neko_draw_rect_2dv(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_rect_2d_impl(cb, a, b, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), color);
}

void __neko_draw_rect_2d_textured(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, u32 tex_id, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.enable_texture_2d(cb, tex_id);
    __neko_draw_rect_2d_impl(cb, a, b, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), color);
}

void __neko_draw_rect_2d_textured_ext(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.enable_texture_2d(cb, tex_id);
    __neko_draw_rect_2d_impl(cb, neko_v2(x0, y0), neko_v2(x1, y1), neko_v2(u0, v0), neko_v2(u1, v1), color);
}

void __neko_draw_box_vqs_impl(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color) {
    f32 width = 0.5f;
    f32 height = 0.5f;
    f32 length = 0.5f;
    f32 x = 0.f;
    f32 y = 0.f;
    f32 z = 0.f;

    // Preapply matrix transformations to all verts
    neko_mat4 mat = neko_vqs_to_mat4(&xform);

    neko_vec3 v0 = neko_v3(x - width / 2, y - height / 2, z + length / 2);
    neko_vec3 v1 = neko_v3(x + width / 2, y - height / 2, z + length / 2);
    neko_vec3 v2 = neko_v3(x - width / 2, y + height / 2, z + length / 2);
    neko_vec3 v3 = neko_v3(x + width / 2, y + height / 2, z + length / 2);
    neko_vec3 v4 = neko_v3(x - width / 2, y - height / 2, z - length / 2);
    neko_vec3 v5 = neko_v3(x - width / 2, y + height / 2, z - length / 2);
    neko_vec3 v6 = neko_v3(x + width / 2, y - height / 2, z - length / 2);
    neko_vec3 v7 = neko_v3(x + width / 2, y + height / 2, z - length / 2);

    neko_vec2 uv0 = neko_v2(0.f, 0.f);
    neko_vec2 uv1 = neko_v2(1.f, 0.f);
    neko_vec2 uv2 = neko_v2(0.f, 1.f);
    neko_vec2 uv3 = neko_v2(1.f, 1.f);

    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.push_matrix(cb, neko_matrix_model);
        {
            gfx->immediate.mat_mul(cb, mat);
            gfx->immediate.color_ubv(cb, color);

            // Front face
            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v2);  // Top Left

            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v3);  // Top Right
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v2);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right

            // Back face
            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v6);  // Bottom Left
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v5);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v7);  // Bottom Right

            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v6);  // Top Right
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v5);  // Top Left

            // Top face
            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v7);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v2);  // Bottom Left
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v3);  // Bottom Right

            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v7);  // Top Right
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v5);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right

            // Bottom face
            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v4);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left

            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v4);  // Top Right
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v1);  // Top Left

            // Right face
            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v7);  // Top Right
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v3);  // Top Left

            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v1);  // Bottom Left
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v7);  // Top Left

            // Left face
            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v2);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv2);
            gfx->immediate.vertex_3fv(cb, v5);  // Top Right

            gfx->immediate.texcoord_2fv(cb, uv0);
            gfx->immediate.vertex_3fv(cb, v4);  // Bottom Left
            gfx->immediate.texcoord_2fv(cb, uv1);
            gfx->immediate.vertex_3fv(cb, v0);  // Top Left
            gfx->immediate.texcoord_2fv(cb, uv3);
            gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right
        }
        gfx->immediate.pop_matrix(cb);
    }
    gfx->immediate.end(cb);
}

void __neko_draw_box_impl(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color) {
    f32 width = half_extents.x;
    f32 height = half_extents.y;
    f32 length = half_extents.z;
    f32 x = origin.x;
    f32 y = origin.y;
    f32 z = origin.z;

    neko_vec3 v0 = neko_v3(x - width / 2, y - height / 2, z + length / 2);
    neko_vec3 v1 = neko_v3(x + width / 2, y - height / 2, z + length / 2);
    neko_vec3 v2 = neko_v3(x - width / 2, y + height / 2, z + length / 2);
    neko_vec3 v3 = neko_v3(x + width / 2, y + height / 2, z + length / 2);
    neko_vec3 v4 = neko_v3(x - width / 2, y - height / 2, z - length / 2);
    neko_vec3 v5 = neko_v3(x - width / 2, y + height / 2, z - length / 2);
    neko_vec3 v6 = neko_v3(x + width / 2, y - height / 2, z - length / 2);
    neko_vec3 v7 = neko_v3(x + width / 2, y + height / 2, z - length / 2);

    neko_vec2 uv0 = neko_v2(0.f, 0.f);
    neko_vec2 uv1 = neko_v2(1.f, 0.f);
    neko_vec2 uv2 = neko_v2(0.f, 1.f);
    neko_vec2 uv3 = neko_v2(1.f, 1.f);

    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.color_ubv(cb, color);

        // Front face
        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v2);  // Top Left

        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v3);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v2);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right

        // Back face
        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v5);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v7);  // Bottom Right

        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v6);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v5);  // Top Left

        // Top face
        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v7);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v2);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v3);  // Bottom Right

        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v7);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v5);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right

        // Bottom face
        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v4);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left

        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v4);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v1);  // Top Left

        // Right face
        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v7);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v3);  // Top Left

        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v7);  // Top Left

        // Left face
        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v2);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv2);
        gfx->immediate.vertex_3fv(cb, v5);  // Top Right

        gfx->immediate.texcoord_2fv(cb, uv0);
        gfx->immediate.vertex_3fv(cb, v4);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv1);
        gfx->immediate.vertex_3fv(cb, v0);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3);
        gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right
    }
    gfx->immediate.end(cb);
}

void __neko_draw_box(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_box_impl(cb, origin, half_extents, color);
}

void __neko_draw_box_vqs(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_box_vqs_impl(cb, xform, color);
}

void __neko_draw_box_textured_vqs(neko_command_buffer_t* cb, neko_vqs xform, u32 tex_id, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.enable_texture_2d(cb, tex_id);
    __neko_draw_box_vqs_impl(cb, xform, color);
}

void __neko_draw_box_textured(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, u32 tex_id, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.enable_texture_2d(cb, tex_id);
    __neko_draw_box_impl(cb, origin, half_extents, color);
}

void __neko_draw_box_lines_impl(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color) {
    // Draw individual 3d lines using vqs
    f32 width = half_extents.x;
    f32 height = half_extents.y;
    f32 length = half_extents.z;
    f32 x = origin.x;
    f32 y = origin.y;
    f32 z = origin.z;

    neko_vec3 v0 = neko_v3(x - width / 2, y - height / 2, z + length / 2);
    neko_vec3 v1 = neko_v3(x + width / 2, y - height / 2, z + length / 2);
    neko_vec3 v2 = neko_v3(x - width / 2, y + height / 2, z + length / 2);
    neko_vec3 v3 = neko_v3(x + width / 2, y + height / 2, z + length / 2);
    neko_vec3 v4 = neko_v3(x - width / 2, y - height / 2, z - length / 2);
    neko_vec3 v5 = neko_v3(x - width / 2, y + height / 2, z - length / 2);
    neko_vec3 v6 = neko_v3(x + width / 2, y - height / 2, z - length / 2);
    neko_vec3 v7 = neko_v3(x + width / 2, y + height / 2, z - length / 2);

    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_lines);
    {
        gfx->immediate.color_ubv(cb, color);

        // Front face
        gfx->immediate.draw_line_3d(cb, v0, v1, color);
        gfx->immediate.draw_line_3d(cb, v1, v3, color);
        gfx->immediate.draw_line_3d(cb, v3, v2, color);
        gfx->immediate.draw_line_3d(cb, v2, v0, color);

        // Back face
        gfx->immediate.draw_line_3d(cb, v4, v6, color);
        gfx->immediate.draw_line_3d(cb, v6, v7, color);
        gfx->immediate.draw_line_3d(cb, v7, v5, color);
        gfx->immediate.draw_line_3d(cb, v5, v4, color);

        // Right face
        gfx->immediate.draw_line_3d(cb, v1, v6, color);
        gfx->immediate.draw_line_3d(cb, v6, v7, color);
        gfx->immediate.draw_line_3d(cb, v7, v3, color);
        gfx->immediate.draw_line_3d(cb, v3, v1, color);

        // Left face
        gfx->immediate.draw_line_3d(cb, v4, v6, color);
        gfx->immediate.draw_line_3d(cb, v6, v1, color);
        gfx->immediate.draw_line_3d(cb, v1, v0, color);
        gfx->immediate.draw_line_3d(cb, v0, v4, color);

        // Bottom face
        gfx->immediate.draw_line_3d(cb, v5, v7, color);
        gfx->immediate.draw_line_3d(cb, v7, v3, color);
        gfx->immediate.draw_line_3d(cb, v3, v2, color);
        gfx->immediate.draw_line_3d(cb, v2, v5, color);

        // Top face
        gfx->immediate.draw_line_3d(cb, v0, v4, color);
        gfx->immediate.draw_line_3d(cb, v4, v5, color);
        gfx->immediate.draw_line_3d(cb, v5, v2, color);
        gfx->immediate.draw_line_3d(cb, v2, v0, color);
    }
    gfx->immediate.end(cb);
}

void __neko_draw_box_lines_vqs_impl(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color) {
    // Draw individual 3d lines using vqs
    f32 width = 0.5f;
    f32 height = 0.5f;
    f32 length = 0.5f;
    f32 x = 0.f;
    f32 y = 0.f;
    f32 z = 0.f;

    // Preapply matrix transformations to all verts
    neko_mat4 mat = neko_vqs_to_mat4(&xform);

    neko_vec3 v0 = neko_v3(x - width / 2, y - height / 2, z + length / 2);
    neko_vec3 v1 = neko_v3(x + width / 2, y - height / 2, z + length / 2);
    neko_vec3 v2 = neko_v3(x - width / 2, y + height / 2, z + length / 2);
    neko_vec3 v3 = neko_v3(x + width / 2, y + height / 2, z + length / 2);
    neko_vec3 v4 = neko_v3(x - width / 2, y - height / 2, z - length / 2);
    neko_vec3 v5 = neko_v3(x - width / 2, y + height / 2, z - length / 2);
    neko_vec3 v6 = neko_v3(x + width / 2, y - height / 2, z - length / 2);
    neko_vec3 v7 = neko_v3(x + width / 2, y + height / 2, z - length / 2);

    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.begin(cb, neko_lines);
    {
        gfx->immediate.push_matrix(cb, neko_matrix_model);
        {
            gfx->immediate.mat_mul(cb, mat);
            gfx->immediate.color_ubv(cb, color);

            // Front face
            gfx->immediate.draw_line_3d(cb, v0, v1, color);
            gfx->immediate.draw_line_3d(cb, v1, v3, color);
            gfx->immediate.draw_line_3d(cb, v3, v2, color);
            gfx->immediate.draw_line_3d(cb, v2, v0, color);

            // Back face
            gfx->immediate.draw_line_3d(cb, v4, v6, color);
            gfx->immediate.draw_line_3d(cb, v6, v7, color);
            gfx->immediate.draw_line_3d(cb, v7, v5, color);
            gfx->immediate.draw_line_3d(cb, v5, v4, color);

            // Right face
            gfx->immediate.draw_line_3d(cb, v1, v6, color);
            gfx->immediate.draw_line_3d(cb, v6, v7, color);
            gfx->immediate.draw_line_3d(cb, v7, v3, color);
            gfx->immediate.draw_line_3d(cb, v3, v1, color);

            // Left face
            gfx->immediate.draw_line_3d(cb, v4, v6, color);
            gfx->immediate.draw_line_3d(cb, v6, v1, color);
            gfx->immediate.draw_line_3d(cb, v1, v0, color);
            gfx->immediate.draw_line_3d(cb, v0, v4, color);

            // Bottom face
            gfx->immediate.draw_line_3d(cb, v5, v7, color);
            gfx->immediate.draw_line_3d(cb, v7, v3, color);
            gfx->immediate.draw_line_3d(cb, v3, v2, color);
            gfx->immediate.draw_line_3d(cb, v2, v5, color);

            // Top face
            gfx->immediate.draw_line_3d(cb, v0, v4, color);
            gfx->immediate.draw_line_3d(cb, v4, v5, color);
            gfx->immediate.draw_line_3d(cb, v5, v2, color);
            gfx->immediate.draw_line_3d(cb, v2, v0, color);
        }
        gfx->immediate.pop_matrix(cb);
    }
    gfx->immediate.end(cb);
}

void __neko_draw_box_lines(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_box_lines_impl(cb, origin, half_extents, color);
}

void __neko_draw_box_lines_vqs(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_box_lines_vqs_impl(cb, xform, color);
}

/*=========
// Sphere
=========*/

void __neko_draw_sphere_impl(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    s32 rings = 16;
    s32 slices = 16;

    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.color_ubv(cb, color);
        {
            for (s32 i = 0; i < (rings + 2); i++) {
                for (s32 j = 0; j < slices; j++) {
                    gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * sinf(deg2rad * (j * 360 / slices)),
                                             radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * i)), radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * cosf(deg2rad * (j * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                             radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                             radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * ((j + 1) * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * (j * 360 / slices)),
                                             radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                             radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * (j * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * sinf(deg2rad * (j * 360 / slices)),
                                             radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * i)), radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * cosf(deg2rad * (j * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                             radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i))),
                                             radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i))) * cosf(deg2rad * ((j + 1) * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                             radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                             radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * ((j + 1) * 360 / slices)));
                }
            }
        }
    }
    gfx->immediate.end(cb);
}

void __neko_draw_sphere(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_sphere_impl(cb, center, radius, color);
}

void __neko_draw_sphere_lines_impl(neko_command_buffer_t* cb, neko_vec3 origin, f32 radius, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    const s32 rings = 16;
    const s32 slices = 16;
    s32 numVertex = (rings + 2) * slices * 6;

    gfx->immediate.begin(cb, neko_lines);
    {
        gfx->immediate.color_ubv(cb, color);
        neko_for_range_i(rings + 2) {
            neko_for_range_j(slices) {
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * sinf(deg2rad * (j * 360 / slices)), radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * i)),
                                         radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * cosf(deg2rad * (j * 360 / slices)));
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                         radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                         radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * ((j + 1) * 360 / slices)));

                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                         radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                         radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * ((j + 1) * 360 / slices)));
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * (j * 360 / slices)),
                                         radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                         radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * (j * 360 / slices)));

                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * (j * 360 / slices)),
                                         radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                         radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * (j * 360 / slices)));
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * sinf(deg2rad * (j * 360 / slices)), radius * sinf(deg2rad * (270 + (180 / (rings + 1)) * i)),
                                         radius * cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * cosf(deg2rad * (j * 360 / slices)));
            }
        }
    }
    gfx->immediate.end(cb);
}

void __neko_draw_sphere_lines_vqs(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    const s32 rings = 16;
    const s32 slices = 16;
    s32 numVertex = (rings + 2) * slices * 6;

    gfx->immediate.begin(cb, neko_lines);
    {
        gfx->immediate.color_ubv(cb, color);
        gfx->immediate.push_matrix(cb, neko_matrix_model);
        {
            gfx->immediate.mat_mul_vqs(cb, xform);
            neko_for_range_i(rings + 2) {
                neko_for_range_j(slices) {
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * sinf(deg2rad * (j * 360 / slices)), sinf(deg2rad * (270 + (180 / (rings + 1)) * i)),
                                             cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * cosf(deg2rad * (j * 360 / slices)));
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                             sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))), cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * ((j + 1) * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * ((j + 1) * 360 / slices)),
                                             sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))), cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * ((j + 1) * 360 / slices)));
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * (j * 360 / slices)), sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                             cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * (j * 360 / slices)));

                    gfx->immediate.vertex_3f(cb, cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * sinf(deg2rad * (j * 360 / slices)), sinf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))),
                                             cosf(deg2rad * (270 + (180 / (rings + 1)) * (i + 1))) * cosf(deg2rad * (j * 360 / slices)));
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * sinf(deg2rad * (j * 360 / slices)), sinf(deg2rad * (270 + (180 / (rings + 1)) * i)),
                                             cosf(deg2rad * (270 + (180 / (rings + 1)) * i)) * cosf(deg2rad * (j * 360 / slices)));
                }
            }
        }
        gfx->immediate.pop_matrix(cb);
    }
    gfx->immediate.end(cb);
}

// Draw sphere wires
void __neko_draw_sphere_lines(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    __neko_draw_sphere_lines_impl(cb, center, radius, color);
}

/*=========
// Circle
=========*/

void __neko_draw_circle_sector_impl(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 start_angle, s32 end_angle, s32 segments, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    if (radius <= 0.0f) {
        radius = 0.1f;
    }

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // swap values
        s32 tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        f32 th = acosf(2 * powf(1 - smooth_circle_error_rate / radius, 2) - 1);
        segments = (s32)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) {
            segments = 4;
        }
    }

    f32 step = (f32)(end_angle - start_angle) / (f32)segments;
    f32 angle = (f32)start_angle;

    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.color_ubv(cb, color);
        neko_for_range_i(segments) {
            gfx->immediate.vertex_2f(cb, center.x, center.y);
            gfx->immediate.vertex_2f(cb, center.x + sinf(deg2rad * angle) * radius, center.y + cosf(deg2rad * angle) * radius);
            gfx->immediate.vertex_2f(cb, center.x + sinf(deg2rad * (angle + step)) * radius, center.y + cosf(deg2rad * (angle + step)) * radius);
            angle += step;
        }
    }
    gfx->immediate.end(cb);
}

void __neko_draw_circle_sector(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 start_angle, s32 end_angle, s32 segments, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_circle_sector_impl(cb, center, radius, start_angle, end_angle, segments, color);
}

void __neko_draw_circle(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 segments, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.disable_texture_2d(cb);
    __neko_draw_circle_sector_impl(cb, center, radius, 0.f, 360.f, segments, color);
}

/*=========
// Text
=========*/

void __neko_draw_text_ext(neko_command_buffer_t* cb, f32 x, f32 y, const char* text, neko_resource(neko_font_t) handle, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_font_t* ft = neko_resource_cache_get_ptr(gfx->font_cache, handle);
    gfx->immediate.begin(cb, neko_triangles);
    {
        gfx->immediate.enable_texture_2d(cb, ft->texture.id);
        gfx->immediate.color_ubv(cb, color);
        while (text[0] != '\0') {
            char c = text[0];
            if (c >= 32 && c <= 127) {
                stbtt_aligned_quad q = neko_default_val();
                stbtt_GetBakedQuad((stbtt_bakedchar*)ft->glyphs, ft->texture.width, ft->texture.height, c - 32, &x, &y, &q, 1);

                neko_vec3 v0 = neko_v3(q.x0, q.y0, 0.f);  // TL
                neko_vec3 v1 = neko_v3(q.x1, q.y0, 0.f);  // TR
                neko_vec3 v2 = neko_v3(q.x0, q.y1, 0.f);  // BL
                neko_vec3 v3 = neko_v3(q.x1, q.y1, 0.f);  // BR

                neko_vec2 uv0 = neko_v2(q.s0, 1.f - q.t0);  // TL
                neko_vec2 uv1 = neko_v2(q.s1, 1.f - q.t0);  // TR
                neko_vec2 uv2 = neko_v2(q.s0, 1.f - q.t1);  // BL
                neko_vec2 uv3 = neko_v2(q.s1, 1.f - q.t1);  // BR

                gfx->immediate.texcoord_2fv(cb, uv0);
                gfx->immediate.vertex_3fv(cb, v0);

                gfx->immediate.texcoord_2fv(cb, uv3);
                gfx->immediate.vertex_3fv(cb, v3);

                gfx->immediate.texcoord_2fv(cb, uv2);
                gfx->immediate.vertex_3fv(cb, v2);

                gfx->immediate.texcoord_2fv(cb, uv0);
                gfx->immediate.vertex_3fv(cb, v0);

                gfx->immediate.texcoord_2fv(cb, uv1);
                gfx->immediate.vertex_3fv(cb, v1);

                gfx->immediate.texcoord_2fv(cb, uv3);
                gfx->immediate.vertex_3fv(cb, v3);
            }
            text++;
        }
    }
    gfx->immediate.end(cb);
}

void __neko_draw_text(neko_command_buffer_t* cb, f32 x, f32 y, const char* text, neko_color_t color) {
    neko_graphics_i* gfx = neko_engine_subsystem(graphics);
    neko_resource(neko_font_t) ft = gfx->default_font();
    gfx->immediate.draw_text_ext(cb, x, y, text, ft, color);
}

/*============
// Matrix Ops
============*/

void __neko_push_camera(neko_command_buffer_t* cb, neko_camera_t camera) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    // 现在只需抓住主窗口即可 将来需要占据视口堆栈的顶部
    neko_platform_i* p = neko_engine_instance()->ctx.platform;
    neko_vec2 ws = p->window_size(p->main_window());
    gfx->immediate.push_matrix(cb, neko_matrix_vp);
    gfx->immediate.mat_mul(cb, neko_camera_get_view_projection(&camera, ws.x, ws.y));
}

void __neko_pop_camera(neko_command_buffer_t* cb) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.pop_matrix(cb);
}

neko_camera_t __neko_begin_3d(neko_command_buffer_t* cb) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_camera_t c = neko_camera_perspective();
    gfx->immediate.push_state(cb, gfx->immediate.default_pipeline_state(neko_immediate_mode_3d));
    gfx->immediate.push_camera(cb, c);
    return c;
}

void __neko_end_3d(neko_command_buffer_t* cb) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.pop_camera(cb);
    gfx->immediate.pop_state(cb);
}

neko_camera_t __neko_begin_2d(neko_command_buffer_t* cb) {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

    neko_vec2 ws = platform->window_size(platform->main_window());
    neko_vec2 hws = neko_vec2_scale(ws, 0.5f);
    neko_camera_t c = neko_camera_default();
    c.transform.position = neko_vec3_add(c.transform.position, neko_v3(hws.x, hws.y, 1.f));

    f32 l = -ws.x / 2.f;
    f32 r = ws.x / 2.f;
    f32 b = ws.y / 2.f;
    f32 t = -ws.y / 2.f;
    neko_mat4 ortho = neko_mat4_transpose(neko_mat4_ortho(l, r, b, t, 0.01f, 1000.f));
    ortho = neko_mat4_mul(ortho, neko_camera_get_view(&c));

    gfx->immediate.push_state(cb, gfx->immediate.default_pipeline_state(neko_immediate_mode_2d));
    gfx->immediate.push_matrix(cb, neko_matrix_vp);
    gfx->immediate.mat_mul(cb, ortho);

    return c;
}

void __neko_end_2d(neko_command_buffer_t* cb) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.pop_camera(cb);
    gfx->immediate.pop_state(cb);
}

void __neko_mat_rotatef(neko_command_buffer_t* cb, f32 rad, f32 x, f32 y, f32 z) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_mat4_rotate(rad, neko_v3(x, y, z)));
}

void __neko_mat_rotatev(neko_command_buffer_t* cb, f32 rad, neko_vec3 v) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_mat4_rotate(rad, v));
}

void __neko_mat_rotateq(neko_command_buffer_t* cb, neko_quat q) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_quat_to_mat4(q));
}

void __neko_mat_transf(neko_command_buffer_t* cb, f32 x, f32 y, f32 z) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_mat4_translate(neko_v3(x, y, z)));
}

void __neko_mat_transv(neko_command_buffer_t* cb, neko_vec3 v) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_mat4_translate(v));
}

void __neko_mat_scalef(neko_command_buffer_t* cb, f32 x, f32 y, f32 z) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_mat4_scale(neko_v3(x, y, z)));
}

void __neko_mat_scalev(neko_command_buffer_t* cb, neko_vec3 v) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_mat4_scale(v));
}

void __neko_mat_mul_vqs(neko_command_buffer_t* cb, neko_vqs xform) {
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    gfx->immediate.mat_mul(cb, neko_vqs_to_mat4(&xform));
}

neko_resource(neko_font_t) __neko_construct_default_font() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_font_t f = neko_default_val();

    stbtt_fontinfo font = neko_default_val();

    std::ifstream ttf_file(neko_file_path("data/assets/fonts/monocraft.ttf"), std::ios::binary);
    if (!ttf_file.is_open()) {
        neko_error("Failed to construct default font");
    }
    std::vector<char> buffer(std::istreambuf_iterator<char>(ttf_file), {});
    ttf_file.close();

    const u32 w = 512;
    const u32 h = 512;
    const u32 num_comps = 4;
    u8* alpha_bitmap = (u8*)neko_safe_malloc(w * h);
    u8* flipmap = (u8*)neko_safe_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);
    s32 v = stbtt_BakeFontBitmap((u8*)buffer.data(), 0, 28.f, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f.glyphs);  // no guarantee this fits!

    // Flip texture
    u32 r = h - 1;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            u32 i0 = i * w + j;
            u32 i1 = r * w * num_comps + j * num_comps;
            u8 a = alpha_bitmap[i0];
            flipmap[i1 + 0] = 255;
            flipmap[i1 + 1] = 255;
            flipmap[i1 + 2] = 255;
            flipmap[i1 + 3] = a;
        }
        r--;
    }

    neko_texture_parameter_desc desc = neko_texture_parameter_desc_default();
    desc.width = w;
    desc.height = h;
    desc.num_comps = num_comps;
    desc.data = flipmap;
    desc.texture_format = neko_texture_format_rgba8;
    desc.min_filter = neko_linear;

    // Generate atlas texture for bitmap with bitmap data
    f.texture = gfx->construct_texture(desc);

    neko_safe_free(alpha_bitmap);
    neko_safe_free(flipmap);

    neko_resource(neko_font_t) handle = neko_resource_cache_insert(gfx->font_cache, f);
    return handle;
}
