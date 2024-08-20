#include "engine/draw.h"

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/font.h"
#include "engine/game.h"
#include "engine/gfx.h"
#include "engine/luax.h"
#include "engine/neko.hpp"
#include "engine/os.h"
#include "engine/prelude.h"

// deps
#include <stb_image.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#if 0

struct Renderer2D {
    Matrix4 matrices[32];
    u64 matrices_len;

    float clear_color[4];
    Color draw_colors[32];
    u64 draw_colors_len;

    u32 sampler;
};

static Renderer2D g_renderer;

void renderer_reset() {
    g_renderer.clear_color[0] = 0.0f;
    g_renderer.clear_color[1] = 0.0f;
    g_renderer.clear_color[2] = 0.0f;
    g_renderer.clear_color[3] = 1.0f;

    g_renderer.draw_colors[0].r = 255;
    g_renderer.draw_colors[0].g = 255;
    g_renderer.draw_colors[0].b = 255;
    g_renderer.draw_colors[0].a = 255;
    g_renderer.draw_colors_len = 1;

    g_renderer.matrices[0] = {};
    g_renderer.matrices[0].cols[0][0] = 1.0f;
    g_renderer.matrices[0].cols[1][1] = 1.0f;
    g_renderer.matrices[0].cols[2][2] = 1.0f;
    g_renderer.matrices[0].cols[3][3] = 1.0f;
    g_renderer.matrices_len = 1;

    // g_renderer.sampler = SG_INVALID_ID;
}

void renderer_use_sampler(u32 sampler) { g_renderer.sampler = sampler; }

void renderer_get_clear_color(float* rgba) { memcpy(rgba, g_renderer.clear_color, sizeof(float) * 4); }

void renderer_set_clear_color(float* rgba) { memcpy(g_renderer.clear_color, rgba, sizeof(float) * 4); }

void renderer_apply_color() {
    Color c = g_renderer.draw_colors[g_renderer.draw_colors_len - 1];
    // sgl_c4b(c.r, c.g, c.b, c.a);
}

bool renderer_push_color(Color c) {
    if (g_renderer.draw_colors_len == array_size(g_renderer.draw_colors)) {
        return false;
    }

    g_renderer.draw_colors[g_renderer.draw_colors_len++] = c;
    return true;
}

bool renderer_pop_color() {
    if (g_renderer.draw_colors_len == 1) {
        return false;
    }

    g_renderer.draw_colors_len--;
    return true;
}

bool renderer_push_matrix() {
    if (g_renderer.matrices_len == array_size(g_renderer.matrices)) {
        return false;
    }

    g_renderer.matrices[g_renderer.matrices_len] = g_renderer.matrices[g_renderer.matrices_len - 1];
    g_renderer.matrices_len++;
    return true;
}

bool renderer_pop_matrix() {
    if (g_renderer.matrices_len == 1) {
        return false;
    }

    g_renderer.matrices_len--;
    return true;
}

void renderer_translate(float x, float y) {
    Matrix4 top = renderer_peek_matrix();

#ifdef SSE_AVAILABLE
    __m128 xx = _mm_mul_ps(_mm_set1_ps(x), top.sse[0]);
    __m128 yy = _mm_mul_ps(_mm_set1_ps(y), top.sse[1]);
    top.sse[3] = _mm_add_ps(_mm_add_ps(xx, yy), _mm_add_ps(top.sse[2], top.sse[3]));
#else
    for (i32 i = 0; i < 4; i++) {
        top.cols[3][i] = x * top.cols[0][i] + y * top.cols[1][i] + top.cols[2][i] + top.cols[3][i];
    }
#endif

    renderer_set_top_matrix(top);
}

void renderer_rotate(float angle) {
    Matrix4 top = renderer_peek_matrix();

#ifdef SSE_AVAILABLE
    __m128 v0 = top.sse[0];
    __m128 v1 = top.sse[1];
    __m128 c = _mm_set1_ps(cos(-angle));
    __m128 s = _mm_set1_ps(sin(-angle));

    top.sse[0] = _mm_sub_ps(_mm_mul_ps(c, v0), _mm_mul_ps(s, v1));
    top.sse[1] = _mm_add_ps(_mm_mul_ps(s, v0), _mm_mul_ps(c, v1));
#else
    float c = cos(-angle);
    float s = sin(-angle);

    for (i32 i = 0; i < 4; i++) {
        float x = c * top.cols[0][i] - s * top.cols[1][i];
        float y = s * top.cols[0][i] + c * top.cols[1][i];
        top.cols[0][i] = x;
        top.cols[1][i] = y;
    }
#endif

    renderer_set_top_matrix(top);
}

void renderer_scale(float x, float y) {
    Matrix4 top = renderer_peek_matrix();

#ifdef SSE_AVAILABLE
    top.sse[0] = _mm_mul_ps(top.sse[0], _mm_set1_ps(x));
    top.sse[1] = _mm_mul_ps(top.sse[1], _mm_set1_ps(y));
#else
    for (i32 i = 0; i < 4; i++) {
        top.cols[0][i] *= x;
        top.cols[1][i] *= y;
    }
#endif

    renderer_set_top_matrix(top);
}

void renderer_push_quad(Vector4 pos, Vector4 tex) {
    Matrix4 top = renderer_peek_matrix();
    Vector4 a = vec4_mul_mat4(vec4_xy(pos.x, pos.y), top);
    Vector4 b = vec4_mul_mat4(vec4_xy(pos.x, pos.w), top);
    Vector4 c = vec4_mul_mat4(vec4_xy(pos.z, pos.w), top);
    Vector4 d = vec4_mul_mat4(vec4_xy(pos.z, pos.y), top);

    // sgl_v2f_t2f(a.x, a.y, tex.x, tex.y);
    // sgl_v2f_t2f(b.x, b.y, tex.x, tex.w);
    // sgl_v2f_t2f(c.x, c.y, tex.z, tex.w);
    // sgl_v2f_t2f(d.x, d.y, tex.z, tex.y);
}

void renderer_push_xy(float x, float y) {
    Matrix4 top = renderer_peek_matrix();
    Vector4 v = vec4_mul_mat4(vec4_xy(x, y), top);
    // sgl_v2f(v.x, v.y);
}

void draw_image(const Image *img, DrawDescription *desc) {
    bool ok = renderer_push_matrix();
    if (!ok) {
        return;
    }

    renderer_translate(desc->x, desc->y);
    renderer_rotate(desc->rotation);
    renderer_scale(desc->sx, desc->sy);

    // sgl_enable_texture();
    // sgl_texture({img->id}, {g_renderer.sampler});
    // sgl_begin_quads();

    float x0 = -desc->ox;
    float y0 = -desc->oy;
    float x1 = (desc->u1 - desc->u0) * img->width - desc->ox;
    float y1 = (desc->v1 - desc->v0) * img->height - desc->oy;

    renderer_apply_color();
    renderer_push_quad(vec4(x0, y0, x1, y1), vec4(desc->u0, desc->v0, desc->u1, desc->v1));

    // sgl_end();
    renderer_pop_matrix();
}

void draw_tilemap(const MapLdtk *tm) {
    PROFILE_FUNC();

    // sgl_enable_texture();
    renderer_apply_color();
    for (const TilemapLevel &level : tm->levels) {
        bool ok = false;
        DeferLoop(ok = renderer_push_matrix(), renderer_pop_matrix()) {
            if (!ok) {
                return;
            }

            renderer_translate(level.world_x, level.world_y);
            for (i32 i = level.layers.len - 1; i >= 0; i--) {
                const TilemapLayer &layer = level.layers[i];
                // sgl_texture({layer.image.id}, {g_renderer.sampler});
                // sgl_begin_quads();
                for (Tile tile : layer.tiles) {
                    float x0 = tile.x;
                    float y0 = tile.y;
                    float x1 = tile.x + layer.grid_size;
                    float y1 = tile.y + layer.grid_size;

                    renderer_push_quad(vec4(x0, y0, x1, y1), vec4(tile.u0, tile.v0, tile.u1, tile.v1));
                }
                // sgl_end();
            }
        }
    }
}

void draw_filled_rect(RectDescription *desc) {
    PROFILE_FUNC();

    bool ok = false;
    DeferLoop(ok = renderer_push_matrix(), renderer_pop_matrix()) {
        if (!ok) {
            return;
        }

        renderer_translate(desc->x, desc->y);
        renderer_rotate(desc->rotation);
        renderer_scale(desc->sx, desc->sy);

        // sgl_disable_texture();
        // sgl_begin_quads();

        float x0 = -desc->ox;
        float y0 = -desc->oy;
        float x1 = desc->w - desc->ox;
        float y1 = desc->h - desc->oy;

        renderer_apply_color();
        renderer_push_quad(vec4(x0, y0, x1, y1), vec4(0, 0, 0, 0));

        // sgl_end();
    }
}

void draw_line_rect(RectDescription *desc) {
    PROFILE_FUNC();

    bool ok = false;
    DeferLoop(ok = renderer_push_matrix(), renderer_pop_matrix()) {
        if (!ok) {
            return;
        }

        renderer_translate(desc->x, desc->y);
        renderer_rotate(desc->rotation);
        renderer_scale(desc->sx, desc->sy);

        // sgl_disable_texture();
        // sgl_begin_line_strip();

        float x0 = -desc->ox;
        float y0 = -desc->oy;
        float x1 = desc->w - desc->ox;
        float y1 = desc->h - desc->oy;

        Matrix4 top = renderer_peek_matrix();
        Vector4 a = vec4_mul_mat4(vec4_xy(x0, y0), top);
        Vector4 b = vec4_mul_mat4(vec4_xy(x0, y1), top);
        Vector4 c = vec4_mul_mat4(vec4_xy(x1, y1), top);
        Vector4 d = vec4_mul_mat4(vec4_xy(x1, y0), top);

        renderer_apply_color();
        // sgl_v2f(a.x, a.y);
        // sgl_v2f(b.x, b.y);
        // sgl_v2f(c.x, c.y);
        // sgl_v2f(d.x, d.y);
        // sgl_v2f(a.x, a.y);

        // sgl_end();
    }
}

void draw_line_circle(float x, float y, float radius) {
    PROFILE_FUNC();

    // sgl_disable_texture();
    // sgl_begin_line_strip();

    renderer_apply_color();
    constexpr float tau = MATH_PI * 2.0f;
    for (float i = 0; i <= tau + 0.001f; i += tau / 36.0f) {
        float c = cosf(i) * radius;
        float s = sinf(i) * radius;
        renderer_push_xy(x + c, y + s);
    }

    // sgl_end();
}

void draw_line(float x0, float y0, float x1, float y1) {
    PROFILE_FUNC();

    // sgl_disable_texture();
    // sgl_begin_lines();

    renderer_apply_color();

    renderer_push_xy(x0, y0);
    renderer_push_xy(x1, y1);

    // sgl_end();
}

#endif

DrawDescription draw_description_args(lua_State* L, i32 arg_start) {
    DrawDescription dd;

    dd.x = (float)luaL_optnumber(L, arg_start + 0, 0);
    dd.y = (float)luaL_optnumber(L, arg_start + 1, 0);

    dd.rotation = (float)luaL_optnumber(L, arg_start + 2, 0);

    dd.sx = (float)luaL_optnumber(L, arg_start + 3, 1);
    dd.sy = (float)luaL_optnumber(L, arg_start + 4, 1);

    dd.ox = (float)luaL_optnumber(L, arg_start + 5, 0);
    dd.oy = (float)luaL_optnumber(L, arg_start + 6, 0);

    dd.u0 = (float)luaL_optnumber(L, arg_start + 7, 0);
    dd.v0 = (float)luaL_optnumber(L, arg_start + 8, 0);
    dd.u1 = (float)luaL_optnumber(L, arg_start + 9, 1);
    dd.v1 = (float)luaL_optnumber(L, arg_start + 10, 1);

    return dd;
}

RectDescription rect_description_args(lua_State* L, i32 arg_start) {
    RectDescription rd;

    rd.x = (float)luaL_optnumber(L, arg_start + 0, 0);
    rd.y = (float)luaL_optnumber(L, arg_start + 1, 0);
    rd.w = (float)luaL_optnumber(L, arg_start + 2, 0);
    rd.h = (float)luaL_optnumber(L, arg_start + 3, 0);

    rd.rotation = (float)luaL_optnumber(L, arg_start + 4, 0);

    rd.sx = (float)luaL_optnumber(L, arg_start + 5, 1);
    rd.sy = (float)luaL_optnumber(L, arg_start + 6, 1);

    rd.ox = (float)luaL_optnumber(L, arg_start + 7, 0);
    rd.oy = (float)luaL_optnumber(L, arg_start + 8, 0);

    return rd;
}

void draw_sprite(AseSprite* spr, DrawDescription* desc) {
    bool ok = false;

    DeferLoop(ok = true /*renderer_push_matrix()*/, 0 /*renderer_pop_matrix()*/) {
        if (!ok) {
            return;
        }

        idraw_t* idraw = &g_app->idraw;

        AseSpriteView view = {};
        ok = view.make(spr);
        if (!ok) {
            return;
        }

        // renderer_translate(desc->x, desc->y);
        // renderer_rotate(desc->rotation);
        // renderer_scale(desc->sx, desc->sy);

        // sgl_enable_texture();
        // sgl_texture({view.data.img.id}, {g_renderer.sampler});
        // sgl_begin_quads();

        idraw_texture(idraw, neko_texture_t{view.data.tex.id});

        // float x0 = -desc->ox;
        // float y0 = -desc->oy;
        // float x1 = (float)view.data.width - desc->ox;
        // float y1 = (float)view.data.height - desc->oy;

        AseSpriteFrame f = view.data.frames[view.frame()];

        idraw_rectvx(idraw, neko_v2(desc->x, desc->y), neko_v2(desc->x + (view.data.width * desc->sx), desc->y + (view.data.height * desc->sy)), neko_v2(f.u0, f.v0), neko_v2(f.u1, f.v1),
                          NEKO_COLOR_WHITE, R_PRIMITIVE_TRIANGLES);

        // renderer_apply_color();
        // renderer_push_quad(vec4(x0, y0, x1, y1), vec4(f.u0, f.v0, f.u1, f.v1));

        // sgl_end();
    }
}

/*=============================
// IDraw
=============================*/

// 立即绘制模式 静态数据的全局实例
neko_immediate_draw_static_data_t* g_idraw = NULL;

#define idraw() g_idraw

#ifndef idraw_smooth_circle_error_rate
#define idraw_smooth_circle_error_rate 0.5f
#endif

const f32 idraw_deg2rad = (f32)neko_pi / 180.f;

// Shaders
#if (defined NEKO_IS_WEB || defined NEKO_IS_ANDROID)
#define NEKO_IDRAW_GL_VERSION_STR "#version 300 es\n"
#else
#define NEKO_IDRAW_GL_VERSION_STR "#version 330 core\n"
#endif

#ifndef NEKO_IDRAW_UNIFORM_MVP_MATRIX
#define NEKO_IDRAW_UNIFORM_MVP_MATRIX "u_mvp"
#endif

#ifndef NEKO_IDRAW_UNIFORM_TEXTURE2D
#define NEKO_IDRAW_UNIFORM_TEXTURE2D "u_tex"
#endif

const char* idraw_v_fillsrc = NEKO_IDRAW_GL_VERSION_STR
        "precision mediump float;\n"
        "layout(location = 0) in vec3 a_position;\n"
        "layout(location = 1) in vec2 a_uv;\n"
        "layout(location = 2) in vec4 a_color;\n"
        "uniform mat4 " NEKO_IDRAW_UNIFORM_MVP_MATRIX
        ";\n"
        "out vec2 uv;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  gl_Position = " NEKO_IDRAW_UNIFORM_MVP_MATRIX
        "* vec4(a_position, 1.0);\n"
        "  uv = a_uv;\n"
        "  color = a_color;\n"
        "}\n";

const char* idraw_f_fillsrc = NEKO_IDRAW_GL_VERSION_STR
        "precision mediump float;\n"
        "in vec2 uv;\n"
        "in vec4 color;\n"
        "uniform sampler2D " NEKO_IDRAW_UNIFORM_TEXTURE2D
        ";\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  vec4 tex_col = texture(" NEKO_IDRAW_UNIFORM_TEXTURE2D
        ", uv);\n"
        "  if (tex_col.a < 0.5) discard;\n"
        "  frag_color = color * tex_col;\n"
        "}\n";

idraw_pipeline_state_attr_t idraw_pipeline_state_default() {
    idraw_pipeline_state_attr_t attr = NEKO_DEFAULT_VAL();
    attr.depth_enabled = false;
    attr.stencil_enabled = false;
    attr.blend_enabled = true;
    attr.face_cull_enabled = false;
    attr.prim_type = (u16)R_PRIMITIVE_TRIANGLES;
    return attr;
}

void idraw_reset(idraw_t* idraw) {
    command_buffer_clear(&idraw->commands);
    byte_buffer_clear(&idraw->vertices);
    neko_dyn_array_clear(idraw->indices);
    neko_dyn_array_clear(idraw->cache.modelview);
    neko_dyn_array_clear(idraw->cache.projection);
    neko_dyn_array_clear(idraw->cache.pipelines);
    neko_dyn_array_clear(idraw->cache.modes);
    neko_dyn_array_clear(idraw->vattributes);

    neko_dyn_array_push(idraw->cache.modelview, mat4_identity());
    neko_dyn_array_push(idraw->cache.projection, mat4_identity());
    neko_dyn_array_push(idraw->cache.modes, NEKO_IDRAW_MATRIX_MODELVIEW);

    idraw->cache.texture = idraw()->tex_default;
    idraw->cache.pipeline = idraw_pipeline_state_default();
    idraw->cache.pipeline.prim_type = 0x00;
    idraw->cache.uv = neko_v2(0.f, 0.f);
    idraw->cache.color = NEKO_COLOR_WHITE;
    idraw->flags = 0x00;
}

void neko_immediate_draw_static_data_init() {
    idraw() = neko_malloc_init(neko_immediate_draw_static_data_t);

    // Create uniform buffer
    gfx_uniform_layout_desc_t uldesc = NEKO_DEFAULT_VAL();
    uldesc.type = R_UNIFORM_MAT4;
    gfx_uniform_desc_t udesc = NEKO_DEFAULT_VAL();
    memcpy(udesc.name, NEKO_IDRAW_UNIFORM_MVP_MATRIX, sizeof(NEKO_IDRAW_UNIFORM_MVP_MATRIX));
    udesc.layout = &uldesc;
    idraw()->uniform = gfx_uniform_create(udesc);

    // Create sampler buffer
    gfx_uniform_layout_desc_t sldesc = NEKO_DEFAULT_VAL();
    sldesc.type = R_UNIFORM_SAMPLER2D;
    gfx_uniform_desc_t sbdesc = NEKO_DEFAULT_VAL();
    memcpy(sbdesc.name, NEKO_IDRAW_UNIFORM_TEXTURE2D, sizeof(NEKO_IDRAW_UNIFORM_TEXTURE2D));
    sbdesc.layout = &sldesc;
    idraw()->sampler = gfx_uniform_create(sbdesc);

    // Create default texture (4x4 white)
    Color256 pixels[16] = NEKO_DEFAULT_VAL();
    memset(pixels, 255, 16 * sizeof(Color256));

    gfx_texture_desc_t tdesc = NEKO_DEFAULT_VAL();
    tdesc.width = 4;
    tdesc.height = 4;
    tdesc.format = R_TEXTURE_FORMAT_RGBA8;
    tdesc.min_filter = R_TEXTURE_FILTER_NEAREST;
    tdesc.mag_filter = R_TEXTURE_FILTER_NEAREST;
    tdesc.data = pixels;

    idraw()->tex_default = gfx_texture_create(tdesc);

    // Create shader
    gfx_shader_source_desc_t vsrc;
    vsrc.type = GL_VERTEX_SHADER;
    vsrc.source = idraw_v_fillsrc;
    gfx_shader_source_desc_t fsrc;
    fsrc.type = GL_FRAGMENT_SHADER;
    fsrc.source = idraw_f_fillsrc;
    gfx_shader_source_desc_t idraw_sources[] = {vsrc, fsrc};

    gfx_shader_desc_t sdesc = NEKO_DEFAULT_VAL();
    sdesc.sources = idraw_sources;
    sdesc.size = sizeof(idraw_sources);
    memcpy(sdesc.name, "neko_immediate_default_fill_shader", sizeof("neko_immediate_default_fill_shader"));

    // Vertex attr layout
    gfx_vertex_attribute_desc_t idraw_vattrs[3] = NEKO_DEFAULT_VAL();
    idraw_vattrs[0].format = R_VERTEX_ATTRIBUTE_FLOAT3;
    memcpy(idraw_vattrs[0].name, "a_position", sizeof("a_position"));
    idraw_vattrs[1].format = R_VERTEX_ATTRIBUTE_FLOAT2;
    memcpy(idraw_vattrs[1].name, "a_uv", sizeof("a_uv"));
    idraw_vattrs[2].format = R_VERTEX_ATTRIBUTE_BYTE4;
    memcpy(idraw_vattrs[2].name, "a_color", sizeof("a_color"));

    // Iterate through attribute list, then create custom pipelines requested.
    neko_handle(gfx_shader_t) shader = gfx_shader_create(sdesc);

    // Pipelines
    for (u16 d = 0; d < 2; ++d)                  // Depth
        for (u16 s = 0; s < 2; ++s)              // Stencil
            for (u16 b = 0; b < 2; ++b)          // Blend
                for (u16 f = 0; f < 2; ++f)      // Face Cull
                    for (u16 p = 0; p < 2; ++p)  // Prim Type
                    {
                        idraw_pipeline_state_attr_t attr = NEKO_DEFAULT_VAL();

                        attr.depth_enabled = d;
                        attr.stencil_enabled = s;
                        attr.blend_enabled = b;
                        attr.face_cull_enabled = f;
                        attr.prim_type = p ? (u16)R_PRIMITIVE_TRIANGLES : (u16)R_PRIMITIVE_LINES;

                        // Create new pipeline based on this arrangement
                        gfx_pipeline_desc_t pdesc = NEKO_DEFAULT_VAL();
                        pdesc.raster.shader = shader;
                        pdesc.raster.index_buffer_element_size = sizeof(u16);
                        pdesc.raster.face_culling = attr.face_cull_enabled ? GL_BACK : 0x00;
                        pdesc.raster.primitive = (gfx_primitive_type)attr.prim_type;
                        pdesc.blend.func = attr.blend_enabled ? GL_FUNC_ADD : 0x00;
                        pdesc.blend.src = GL_SRC_ALPHA;
                        pdesc.blend.dst = GL_ONE_MINUS_SRC_ALPHA;
                        pdesc.depth.func = d ? GL_LESS : 0x00;
                        pdesc.stencil.func = s ? GL_ALWAYS : 0x00;
                        pdesc.stencil.ref = s ? 1 : 0x00;
                        pdesc.stencil.comp_mask = s ? 0xFF : 0x00;
                        pdesc.stencil.write_mask = s ? 0xFF : 0x00;
                        pdesc.stencil.sfail = s ? GL_KEEP : 0x00;
                        pdesc.stencil.dpfail = s ? GL_KEEP : 0x00;
                        pdesc.stencil.dppass = s ? GL_KEEP : 0x00;
                        pdesc.layout.attrs = idraw_vattrs;
                        pdesc.layout.size = sizeof(idraw_vattrs);

                        neko_handle(gfx_pipeline_t) hndl = gfx_pipeline_create(pdesc);
                        neko_hash_table_insert(idraw()->pipeline_table, attr, hndl);
                    }

    // Create vertex buffer
    gfx_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
    vdesc.data = NULL;
    vdesc.size = 0;
    vdesc.usage = GL_STREAM_DRAW;
    idraw()->vbo = gfx_vertex_buffer_create(vdesc);

    // mem_free(compressed_ttf_data);
    // mem_free(buf_decompressed_data);
    // mem_free(alpha_bitmap);
    // mem_free(flipmap);
}

void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data) { g_idraw = data; }

void neko_immediate_draw_static_data_free() {
    if (idraw()) {
        neko_hash_table_free(idraw()->pipeline_table);
        mem_free(idraw());
    }
}

neko_immediate_draw_static_data_t* neko_immediate_draw_static_data_get() { return g_idraw; }

// Create / Init / Shutdown / Free
idraw_t neko_immediate_draw_new() {
    if (!idraw()) {
        // Construct NEKO_IDRAW
        neko_immediate_draw_static_data_init();
    }

    idraw_t idraw = NEKO_DEFAULT_VAL();
    memset(&idraw, 0, sizeof(idraw));

    // Set idraw static data
    idraw.data = g_idraw;

    // Init cache
    idraw.cache.color = NEKO_COLOR_WHITE;

    // Init command buffer
    idraw.commands = command_buffer_new();  // Not totally sure on the syntax for new vs. create

    idraw.vertices = byte_buffer_new();

    // Set up cache
    idraw_reset(&idraw);

    return idraw;
}

void neko_immediate_draw_free(idraw_t* ctx) {
    byte_buffer_free(&ctx->vertices);
    neko_dyn_array_free(ctx->indices);
    neko_dyn_array_free(ctx->vattributes);
    command_buffer_free(&ctx->commands);
    neko_dyn_array_free(ctx->cache.pipelines);
    neko_dyn_array_free(ctx->cache.modelview);
    neko_dyn_array_free(ctx->cache.projection);
    neko_dyn_array_free(ctx->cache.modes);
}

neko_handle(gfx_pipeline_t) idraw_get_pipeline(idraw_t* idraw, idraw_pipeline_state_attr_t state) {
    // Bind pipeline
    neko_assert(neko_hash_table_key_exists(idraw()->pipeline_table, state));
    return neko_hash_table_get(idraw()->pipeline_table, state);
}

void neko_immediate_draw_set_pipeline(idraw_t* idraw) {
    if (idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES) {
        return;
    }

    // Check validity
    if (idraw->cache.pipeline.prim_type != (u16)R_PRIMITIVE_TRIANGLES && idraw->cache.pipeline.prim_type != (u16)R_PRIMITIVE_LINES) {
        idraw->cache.pipeline.prim_type = (u16)R_PRIMITIVE_TRIANGLES;
    }
    idraw->cache.pipeline.depth_enabled = NEKO_CLAMP(idraw->cache.pipeline.depth_enabled, 0, 1);
    idraw->cache.pipeline.stencil_enabled = NEKO_CLAMP(idraw->cache.pipeline.stencil_enabled, 0, 1);
    idraw->cache.pipeline.face_cull_enabled = NEKO_CLAMP(idraw->cache.pipeline.face_cull_enabled, 0, 1);
    idraw->cache.pipeline.blend_enabled = NEKO_CLAMP(idraw->cache.pipeline.blend_enabled, 0, 1);

    // Bind pipeline
    neko_assert(neko_hash_table_key_exists(idraw()->pipeline_table, idraw->cache.pipeline));
    gfx_pipeline_bind(&idraw->commands, neko_hash_table_get(idraw()->pipeline_table, idraw->cache.pipeline));
}

// Implemented from: https://stackoverflow.com/questions/27374550/how-to-compare-color-object-and-get-closest-color-in-an-color
// distance between two hues:
f32 neko_hue_dist(f32 h1, f32 h2) {
    f32 d = fabsf(h1 - h2);
    return d > 180.f ? 360.f - d : d;
}

static void idraw_rect_2d(idraw_t* idraw, vec2 a, vec2 b, vec2 uv0, vec2 uv1, Color256 color) {
    vec3 tl = neko_v3(a.x, a.y, 0.f);
    vec3 tr = neko_v3(b.x, a.y, 0.f);
    vec3 bl = neko_v3(a.x, b.y, 0.f);
    vec3 br = neko_v3(b.x, b.y, 0.f);

    vec2 tl_uv = neko_v2(uv0.x, uv1.y);
    vec2 tr_uv = neko_v2(uv1.x, uv1.y);
    vec2 bl_uv = neko_v2(uv0.x, uv0.y);
    vec2 br_uv = neko_v2(uv1.x, uv0.y);

    idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);
    {
        idraw_c4ubv(idraw, color);

        idraw_tc2fv(idraw, tl_uv);
        idraw_v3fv(idraw, tl);

        idraw_tc2fv(idraw, br_uv);
        idraw_v3fv(idraw, br);

        idraw_tc2fv(idraw, bl_uv);
        idraw_v3fv(idraw, bl);

        idraw_tc2fv(idraw, tl_uv);
        idraw_v3fv(idraw, tl);

        idraw_tc2fv(idraw, tr_uv);
        idraw_v3fv(idraw, tr);

        idraw_tc2fv(idraw, br_uv);
        idraw_v3fv(idraw, br);
    }
    idraw_end(idraw);
}

void idraw_rect_textured(idraw_t* idraw, vec2 a, vec2 b, u32 tex_id, Color256 color) { idraw_rect_textured_ext(idraw, a.x, a.y, b.x, b.y, 0.f, 0.f, 1.f, 1.f, tex_id, color); }

void idraw_rect_textured_ext(idraw_t* idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, Color256 color) {

    neko_handle(gfx_texture_t) tex = NEKO_DEFAULT_VAL();
    tex.id = tex_id;

    idraw_texture(idraw, tex);
    idraw_rect_2d(idraw, neko_v2(x0, y0), neko_v2(x1, y1), neko_v2(u0, v0), neko_v2(u1, v1), color);
    idraw_texture(idraw, neko_texture_t{0});
}

// 核心顶点函数
void idraw_begin(idraw_t* idraw, gfx_primitive_type type) {
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES:
            type = R_PRIMITIVE_TRIANGLES;
            break;
        case R_PRIMITIVE_LINES:
            type = R_PRIMITIVE_LINES;
            break;
    }

    // 判断是否推入新的渲染管线
    if (idraw->cache.pipeline.prim_type == type) {
        return;
    }

    // 否则 刷新先前的渲染管线
    idraw_flush(idraw);

    // 设置原始类型
    idraw->cache.pipeline.prim_type = type;

    // 绑定渲染管线
    neko_immediate_draw_set_pipeline(idraw);
}

void idraw_end(idraw_t* idraw) {
    // TODO
}

mat4 idraw_get_modelview_matrix(idraw_t* idraw) { return idraw->cache.modelview[neko_dyn_array_size(idraw->cache.modelview) - 1]; }

mat4 idraw_get_projection_matrix(idraw_t* idraw) { return idraw->cache.projection[neko_dyn_array_size(idraw->cache.projection) - 1]; }

mat4 idraw_get_mvp_matrix(idraw_t* idraw) {
    mat4* mv = &idraw->cache.modelview[neko_dyn_array_size(idraw->cache.modelview) - 1];
    mat4* proj = &idraw->cache.projection[neko_dyn_array_size(idraw->cache.projection) - 1];
    return mat4_mul(*proj, *mv);
}

void idraw_flush(idraw_t* idraw) {
    // 如果顶点数据为空则不刷新
    if (byte_buffer_empty(&idraw->vertices)) {
        return;
    }

    // 设置mvp矩阵
    mat4 mv = idraw->cache.modelview[neko_dyn_array_size(idraw->cache.modelview) - 1];
    mat4 proj = idraw->cache.projection[neko_dyn_array_size(idraw->cache.projection) - 1];
    mat4 mvp = mat4_mul(proj, mv);

    // 更新顶点缓冲区 (使用命令缓冲区)
    gfx_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
    vdesc.data = idraw->vertices.data;
    vdesc.size = byte_buffer_size(&idraw->vertices);
    vdesc.usage = GL_STREAM_DRAW;

    gfx_vertex_buffer_request_update(&idraw->commands, idraw()->vbo, vdesc);

    // 计算绘制数量
    size_t vsz = sizeof(neko_immediate_vert_t);
    if (neko_dyn_array_size(idraw->vattributes)) {
        // 计算顶点步幅
        size_t stride = 0;
        for (u32 i = 0; i < neko_dyn_array_size(idraw->vattributes); ++i) {
            idraw_vattr_type type = idraw->vattributes[i];
            switch (type) {
                default:
                    break;
                case NEKO_IDRAW_VATTR_POSITION:
                    stride += sizeof(vec3);
                    break;
                case NEKO_IDRAW_VATTR_COLOR:
                    stride += sizeof(Color256);
                    break;
                case NEKO_IDRAW_VATTR_UV:
                    stride += sizeof(vec2);
                    break;
            }
        }
        vsz = stride;
    }

    u32 ct = byte_buffer_size(&idraw->vertices) / vsz;

    // 设置所有绑定数据
    gfx_bind_vertex_buffer_desc_t vbuffer = NEKO_DEFAULT_VAL();
    vbuffer.buffer = idraw()->vbo;

    gfx_bind_uniform_desc_t ubinds[2] = NEKO_DEFAULT_VAL();
    ubinds[0].uniform = idraw()->uniform;
    ubinds[0].data = &mvp;
    ubinds[1].uniform = idraw()->sampler;
    ubinds[1].data = &idraw->cache.texture;
    ubinds[1].binding = 0;

    // 所有缓冲区的绑定 vertex,uniform,sampler
    gfx_bind_desc_t binds = NEKO_DEFAULT_VAL();
    binds.vertex_buffers.desc = &vbuffer;

    // if (neko_dyn_array_empty(idraw->vattributes))
    if (~idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_UNIFORMS) {
        binds.uniforms.desc = ubinds;
        binds.uniforms.size = sizeof(ubinds);
    }

    // 绑定
    gfx_apply_bindings(&idraw->commands, &binds);

    // 提交绘制
    gfx_draw_desc_t draw = NEKO_DEFAULT_VAL();
    draw.start = 0;
    draw.count = ct;
    gfx_draw(&idraw->commands, draw);

    // 绘制后清理缓冲区
    byte_buffer_clear(&idraw->vertices);
}

// Core pipeline functions
void idraw_blend_enabled(idraw_t* idraw, bool enabled) {
    // Push a new pipeline?
    if (idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || (bool)idraw->cache.pipeline.blend_enabled == enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    idraw_flush(idraw);

    // Set primitive type
    idraw->cache.pipeline.blend_enabled = enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(idraw);
}

// Core pipeline functions
void idraw_depth_enabled(idraw_t* idraw, bool enabled) {
    // Push a new pipeline?
    if (idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || idraw->cache.pipeline.depth_enabled == (u16)enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    idraw_flush(idraw);

    // Set primitive type
    idraw->cache.pipeline.depth_enabled = (u16)enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(idraw);
}

void idraw_stencil_enabled(idraw_t* idraw, bool enabled) {
    // Push a new pipeline?
    if (idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || idraw->cache.pipeline.stencil_enabled == (u16)enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    idraw_flush(idraw);

    // Set primitive type
    idraw->cache.pipeline.stencil_enabled = (u16)enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(idraw);
}

void idraw_face_cull_enabled(idraw_t* idraw, bool enabled) {
    // Push a new pipeline?
    if (idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || idraw->cache.pipeline.face_cull_enabled == (u16)enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    idraw_flush(idraw);

    // Set primitive type
    idraw->cache.pipeline.face_cull_enabled = (u16)enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(idraw);
}

void idraw_texture(idraw_t* idraw, neko_handle(gfx_texture_t) texture) {
    // Push a new pipeline?
    if (idraw->cache.texture.id == texture.id) {
        return;
    }

    // Otherwise, we need to flush previous content
    idraw_flush(idraw);

    // Set texture
    idraw->cache.texture = (texture.id && texture.id != UINT32_MAX) ? texture : idraw()->tex_default;
}

void idraw_pipeline_set(idraw_t* idraw, neko_handle(gfx_pipeline_t) pipeline) {
    idraw_flush(idraw);

    // Bind if valid
    if (pipeline.id) {
        gfx_pipeline_bind(&idraw->commands, pipeline);
        idraw->flags |= NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES;
    }
    // Otherwise we set back to cache, clear vattributes, clear flag
    else {
        idraw->flags &= ~NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES;
        neko_dyn_array_clear(idraw->vattributes);
        neko_immediate_draw_set_pipeline(idraw);
    }
}

void idraw_vattr_list(idraw_t* idraw, idraw_vattr_type* list, size_t sz) {
    idraw_flush(idraw);

    neko_dyn_array_clear(idraw->vattributes);
    u32 ct = sz / sizeof(idraw_vattr_type);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(idraw->vattributes, list[i]);
    }
}

#if 0
void idraw_vattr_list_mesh(idraw_t* idraw, neko_asset_mesh_layout_t* layout, size_t sz) {
    idraw_flush(idraw);

    neko_dyn_array_clear(idraw->vattributes);

#define VATTR_PUSH(TYPE)                                    \
    do {                                                    \
        neko_dyn_array_push(idraw->vattributes, TYPE); \
    } while (0)

    u32 ct = sz / sizeof(neko_asset_mesh_layout_t);
    for (u32 i = 0; i < ct; ++i) {
        neko_asset_mesh_attribute_type type = layout[i].type;
        switch (type) {
            default:
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION:
                VATTR_PUSH(NEKO_IDRAW_VATTR_POSITION);
                break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD:
                VATTR_PUSH(NEKO_IDRAW_VATTR_UV);
                break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR:
                VATTR_PUSH(NEKO_IDRAW_VATTR_COLOR);
                break;
        }
    }
}
#endif

void idraw_defaults(idraw_t* idraw) {
    idraw_flush(idraw);

    // 设置缓存的默认值
    idraw->cache.texture = idraw()->tex_default;
    idraw->cache.pipeline = idraw_pipeline_state_default();
    idraw->cache.pipeline.prim_type = 0x00;
    idraw->cache.uv = neko_v2(0.f, 0.f);
    idraw->cache.color = NEKO_COLOR_WHITE;
    neko_dyn_array_clear(idraw->vattributes);

    // 重置标志
    idraw->flags = 0x00;

    neko_immediate_draw_set_pipeline(idraw);
}

void idraw_tc2fv(idraw_t* idraw, vec2 uv) {
    // 设置缓存寄存器
    idraw->cache.uv = uv;
}

void idraw_tc2f(idraw_t* idraw, f32 u, f32 v) {
    // 设置缓存寄存器
    idraw_tc2fv(idraw, neko_v2(u, v));
}

void idraw_c4ub(idraw_t* idraw, u8 r, u8 g, u8 b, u8 a) {
    // 设置缓存颜色
    idraw->cache.color = color256(r, g, b, a);
}

void idraw_c4ubv(idraw_t* idraw, Color256 c) { idraw_c4ub(idraw, c.r, c.g, c.b, c.a); }

void idraw_v3fv(idraw_t* idraw, vec3 p) {
    if (neko_dyn_array_size(idraw->vattributes)) {
        // Iterate through attributes and push into stream
        for (u32 i = 0; i < neko_dyn_array_size(idraw->vattributes); ++i) {
            idraw_vattr_type type = idraw->vattributes[i];
            switch (type) {
                default: {
                } break;

                case NEKO_IDRAW_VATTR_POSITION: {
                    byte_buffer_write(&idraw->vertices, vec3, p);
                } break;

                case NEKO_IDRAW_VATTR_COLOR: {
                    byte_buffer_write(&idraw->vertices, Color256, idraw->cache.color);
                } break;

                case NEKO_IDRAW_VATTR_UV: {
                    byte_buffer_write(&idraw->vertices, vec2, idraw->cache.uv);
                } break;
            }
        }
    } else {
        neko_immediate_vert_t v = NEKO_DEFAULT_VAL();
        v.position = p;
        v.uv = idraw->cache.uv;
        v.color = idraw->cache.color;
        byte_buffer_write(&idraw->vertices, neko_immediate_vert_t, v);
    }
}

void idraw_v3f(idraw_t* idraw, f32 x, f32 y, f32 z) {
    // Push vert
    idraw_v3fv(idraw, neko_v3(x, y, z));
}

void idraw_v2f(idraw_t* idraw, f32 x, f32 y) {
    // Push vert
    idraw_v3f(idraw, x, y, 0.f);
}

void idraw_v2fv(idraw_t* idraw, vec2 v) {
    // Push vert
    idraw_v3f(idraw, v.x, v.y, 0.f);
}

void idraw_push_matrix_ex(idraw_t* idraw, idraw_matrix_type type, bool flush) {
    // Flush
    if (flush) {
        idraw_flush(idraw);
    }

    // Push mode
    neko_dyn_array_push(idraw->cache.modes, type);

    // Pop matrix off of stack
    switch (neko_dyn_array_back(idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            neko_dyn_array_push(idraw->cache.modelview, neko_dyn_array_back(idraw->cache.modelview));
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            neko_dyn_array_push(idraw->cache.projection, neko_dyn_array_back(idraw->cache.projection));
        } break;
    }
}

void idraw_push_matrix(idraw_t* idraw, idraw_matrix_type type) { idraw_push_matrix_ex(idraw, type, true); }

void idraw_pop_matrix_ex(idraw_t* idraw, bool flush) {
    // Flush
    if (flush) {
        idraw_flush(idraw);
    }

    // Pop matrix off of stack
    switch (neko_dyn_array_back(idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            if (neko_dyn_array_size(idraw->cache.modelview) > 1) {
                neko_dyn_array_pop(idraw->cache.modelview);
            }
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            if (neko_dyn_array_size(idraw->cache.projection) > 1) {
                neko_dyn_array_pop(idraw->cache.projection);
            }
        } break;
    }

    if (neko_dyn_array_size(idraw->cache.modes) > 1) {
        neko_dyn_array_pop(idraw->cache.modes);
    }
}

void idraw_pop_matrix(idraw_t* idraw) { idraw_pop_matrix_ex(idraw, true); }

void idraw_load_matrix(idraw_t* idraw, mat4 m) {
    // Load matrix at current mode
    switch (neko_dyn_array_back(idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            mat4* mat = &idraw->cache.modelview[neko_dyn_array_size(idraw->cache.modelview) - 1];
            *mat = m;
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            mat4* mat = &idraw->cache.projection[neko_dyn_array_size(idraw->cache.projection) - 1];
            *mat = m;
        } break;
    }
}

void idraw_mul_matrix(idraw_t* idraw, mat4 m) {
    static int i = 0;
    // Multiply current matrix at mode with m
    switch (neko_dyn_array_back(idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            mat4* mat = &idraw->cache.modelview[neko_dyn_array_size(idraw->cache.modelview) - 1];
            *mat = mat4_mul(*mat, m);
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            mat4* mat = &idraw->cache.projection[neko_dyn_array_size(idraw->cache.projection) - 1];
            *mat = mat4_mul(*mat, m);
        } break;
    }
}

void idraw_perspective(idraw_t* idraw, f32 fov, f32 aspect, f32 n, f32 f) {
    // Set current matrix at mode to perspective
    idraw_load_matrix(idraw, mat4_perspective(fov, aspect, n, f));
}

void idraw_ortho(idraw_t* idraw, f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    // Set current matrix at mode to ortho
    idraw_load_matrix(idraw, mat4_ortho(l, r, b, t, n, f));
}

void idraw_rotatef(idraw_t* idraw, f32 angle, f32 x, f32 y, f32 z) {
    // Rotate current matrix at mode
    idraw_mul_matrix(idraw, mat4_rotatev(angle, neko_v3(x, y, z)));
}

void idraw_rotatev(idraw_t* idraw, f32 angle, vec3 v) { idraw_rotatef(idraw, angle, v.x, v.y, v.z); }

void idraw_translatef(idraw_t* idraw, f32 x, f32 y, f32 z) {
    // Translate current matrix at mode
    idraw_mul_matrix(idraw, mat4_translate(x, y, z));
}

void idraw_translatev(idraw_t* idraw, vec3 v) {
    // Translate current matrix at mode
    idraw_mul_matrix(idraw, mat4_translate(v.x, v.y, v.z));
}

void idraw_scalef(idraw_t* idraw, f32 x, f32 y, f32 z) {
    // Scale current matrix at mode
    idraw_mul_matrix(idraw, mat4_scale(x, y, z));
}

void idraw_scalev(idraw_t* idraw, vec3 v) { idraw_mul_matrix(idraw, mat4_scalev(v)); }

void idraw_camera(idraw_t* idraw, mat4 m) {
    // 现在只需集成主窗口即可 将来需要占据视口堆栈的顶部
    idraw_load_matrix(idraw, m);
}

void idraw_camera2d(idraw_t* idraw, u32 width, u32 height) {
    // Flush previous
    idraw_flush(idraw);
    f32 l = 0.f, r = (f32)width, tp = 0.f, b = (f32)height;
    mat4 ortho = mat4_ortho(l, r, b, tp, -1.f, 1.f);
    idraw_load_matrix(idraw, ortho);
}

void idraw_camera2d_ex(idraw_t* idraw, f32 l, f32 r, f32 t, f32 b) {
    // Flush previous
    idraw_flush(idraw);
    mat4 ortho = mat4_ortho(l, r, b, t, -1.f, 1.f);
    idraw_load_matrix(idraw, ortho);
}

void idraw_camera3d(idraw_t* idraw, u32 width, u32 height) {
    // Flush previous
    idraw_flush(idraw);
    // neko_camera_t c = neko_camera_perspective();

    auto neko_camera_get_view = []() -> mat4 {
        vec3 up = neko_v3(0.0f, 1.0f, 0.0f);
        vec3 forward = neko_v3(0.0f, 0.0f, -1.0f);
        vec3 target = vec3_add(forward, vec3_ctor(0.0f, 0.0f, 0.0f));
        return mat4_look_at(vec3_ctor(0.0f, 0.0f, 0.0f), target, up);
    };

    mat4 view = neko_camera_get_view();
    mat4 proj = mat4_perspective(90.f, (f32)width / (f32)height, 0.1f, 1000.0f);

    idraw_camera(idraw, mat4_mul(proj, view));
}

// Shape Drawing Utils
void idraw_triangle(idraw_t* idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    idraw_trianglex(idraw, x0, y0, 0.f, x1, y1, 0.f, x2, y2, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, r, g, b, a, type);
}

void idraw_trianglev(idraw_t* idraw, vec2 a, vec2 b, vec2 c, Color256 color, gfx_primitive_type type) {
    idraw_triangle(idraw, a.x, a.y, b.x, b.y, c.x, c.y, color.r, color.g, color.b, color.a, type);
}

void idraw_trianglex(idraw_t* idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r, u8 g, u8 b, u8 a,
                          gfx_primitive_type type) {
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES: {
            idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);
            idraw_c4ub(idraw, r, g, b, a);
            idraw_tc2f(idraw, u0, v0);
            idraw_v3f(idraw, x0, y0, z0);
            idraw_tc2f(idraw, u1, v1);
            idraw_v3f(idraw, x1, y1, z1);
            idraw_tc2f(idraw, u2, v2);
            idraw_v3f(idraw, x2, y2, z2);
            idraw_end(idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            idraw_line3D(idraw, x0, y0, z0, x1, y1, z1, r, g, b, a);
            idraw_line3D(idraw, x1, y1, z1, x2, y2, z2, r, g, b, a);
            idraw_line3D(idraw, x2, y2, z2, x0, y0, z0, r, g, b, a);
        } break;
    }
}

void idraw_trianglevx(idraw_t* idraw, vec3 v0, vec3 v1, vec3 v2, vec2 uv0, vec2 uv1, vec2 uv2, Color256 color, gfx_primitive_type type) {
    idraw_trianglex(idraw, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, uv0.x, uv0.y, uv1.x, uv1.y, uv2.x, uv2.y, color.r, color.g, color.b, color.a, type);
}

void idraw_trianglevxmc(idraw_t* idraw, vec3 v0, vec3 v1, vec3 v2, vec2 uv0, vec2 uv1, vec2 uv2, Color256 c0, Color256 c1, Color256 c2, gfx_primitive_type type) {
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES: {
            idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);

            // First vert
            idraw_c4ub(idraw, c0.r, c0.g, c0.b, c0.a);
            idraw_tc2f(idraw, uv0.x, uv0.y);
            idraw_v3f(idraw, v0.x, v0.y, v0.z);

            // Second vert
            idraw_c4ub(idraw, c1.r, c1.g, c1.b, c1.a);
            idraw_tc2f(idraw, uv1.x, uv1.y);
            idraw_v3f(idraw, v1.x, v1.y, v1.z);

            // Third vert
            idraw_c4ub(idraw, c2.r, c2.g, c2.b, c2.a);
            idraw_tc2f(idraw, uv2.x, uv2.y);
            idraw_v3f(idraw, v2.x, v2.y, v2.z);

            idraw_end(idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            idraw_line3Dmc(idraw, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, c0.r, c0.g, c0.b, c0.a, c1.r, c1.g, c1.b, c1.a);
            idraw_line3Dmc(idraw, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, c1.r, c1.g, c1.b, c1.a, c2.r, c2.g, c2.b, c2.a);
            idraw_line3Dmc(idraw, v2.x, v2.y, v2.z, v0.x, v0.y, v0.z, c2.r, c2.g, c2.b, c2.a, c0.r, c0.g, c0.b, c0.a);
        } break;
    }
}

void idraw_line3Dmc(idraw_t* idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1) {
    idraw_begin(idraw, R_PRIMITIVE_LINES);

    idraw_tc2f(idraw, 0.f, 0.f);

    // First vert
    idraw_c4ub(idraw, r0, g0, b0, a0);
    idraw_v3f(idraw, x0, y0, z0);

    // Second vert
    idraw_c4ub(idraw, r1, g1, b1, a1);
    idraw_v3f(idraw, x1, y1, z1);
    idraw_end(idraw);
}

void idraw_line3D(idraw_t* idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r, u8 g, u8 b, u8 a) {
    idraw_begin(idraw, R_PRIMITIVE_LINES);
    idraw_tc2f(idraw, 0.f, 0.f);
    idraw_c4ub(idraw, r, g, b, a);
    idraw_v3f(idraw, x0, y0, z0);
    idraw_v3f(idraw, x1, y1, z1);
    idraw_end(idraw);
}

void idraw_line3Dv(idraw_t* idraw, vec3 s, vec3 e, Color256 color) { idraw_line3D(idraw, s.x, s.y, s.z, e.x, e.y, e.z, color.r, color.g, color.b, color.a); }

void idraw_line(idraw_t* idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a) { idraw_line3D(idraw, x0, y0, 0.f, x1, y1, 0.f, r, g, b, a); }

void idraw_linev(idraw_t* idraw, vec2 v0, vec2 v1, Color256 c) { idraw_line(idraw, v0.x, v0.y, v1.x, v1.y, c.r, c.g, c.b, c.a); }

void idraw_rectx(idraw_t* idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, gfx_primitive_type type) {
    // 不应该使用三角形 因为需要声明纹理坐标
    switch (type) {
        case R_PRIMITIVE_LINES: {
            // 第一个三角形
            idraw_line(idraw, l, b, r, b, _r, _g, _b, _a);
            idraw_line(idraw, r, b, r, t, _r, _g, _b, _a);
            idraw_line(idraw, r, t, l, t, _r, _g, _b, _a);
            idraw_line(idraw, l, t, l, b, _r, _g, _b, _a);
            // idraw_triangle(idraw, l, b, r, b, l, t, _r, _g, _b, _a, type);
            // 第二个三角形
            // idraw_triangle(idraw, r, b, r, t, l, t, _r, _g, _b, _a, type);
        } break;

        case R_PRIMITIVE_TRIANGLES: {
            idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);

            idraw_c4ub(idraw, _r, _g, _b, _a);

            // 第一个三角形
            idraw_tc2f(idraw, u0, v0);
            idraw_v2f(idraw, l, b);
            idraw_tc2f(idraw, u1, v0);
            idraw_v2f(idraw, r, b);
            idraw_tc2f(idraw, u0, v1);
            idraw_v2f(idraw, l, t);

            // 第二个三角形
            idraw_tc2f(idraw, u1, v0);
            idraw_v2f(idraw, r, b);
            idraw_tc2f(idraw, u1, v1);
            idraw_v2f(idraw, r, t);
            idraw_tc2f(idraw, u0, v1);
            idraw_v2f(idraw, l, t);

            idraw_end(idraw);

        } break;
    }
}

void idraw_rect(idraw_t* idraw, f32 l, f32 b, f32 r, f32 t, u8 _r, u8 _g, u8 _b, u8 _a, gfx_primitive_type type) {
    idraw_rectx(idraw, l, b, r, t, 0.f, 0.f, 1.f, 1.f, _r, _g, _b, _a, type);
}

void idraw_rectv(idraw_t* idraw, vec2 bl, vec2 tr, Color256 color, gfx_primitive_type type) {
    idraw_rectx(idraw, bl.x, bl.y, tr.x, tr.y, 0.f, 0.f, 1.f, 1.f, color.r, color.g, color.b, color.a, type);
}

void idraw_rectvx(idraw_t* idraw, vec2 bl, vec2 tr, vec2 uv0, vec2 uv1, Color256 color, gfx_primitive_type type) {
    idraw_rectx(idraw, bl.x, bl.y, tr.x, tr.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

void idraw_rectvd(idraw_t* idraw, vec2 xy, vec2 wh, vec2 uv0, vec2 uv1, Color256 color, gfx_primitive_type type) {
    idraw_rectx(idraw, xy.x, xy.y, xy.x + wh.x, xy.y + wh.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

void idraw_rect3Dv(idraw_t* idraw, vec3 min, vec3 max, vec2 uv0, vec2 uv1, Color256 c, gfx_primitive_type type) {
    const vec3 vt0 = min;
    const vec3 vt1 = neko_v3(max.x, min.y, min.z);
    const vec3 vt2 = neko_v3(min.x, max.y, max.z);
    const vec3 vt3 = max;

    switch (type) {
        case R_PRIMITIVE_LINES: {
            idraw_line3Dv(idraw, vt0, vt1, c);
            idraw_line3Dv(idraw, vt1, vt2, c);
            idraw_line3Dv(idraw, vt2, vt3, c);
            idraw_line3Dv(idraw, vt3, vt0, c);
        } break;

        case R_PRIMITIVE_TRIANGLES: {
            idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);

            idraw_c4ub(idraw, c.r, c.g, c.b, c.a);

            const f32 u0 = uv0.x;
            const f32 u1 = uv1.x;
            const f32 v0 = uv0.y;
            const f32 v1 = uv1.y;

            // First triangle
            idraw_c4ub(idraw, c.r, c.g, c.b, c.a);
            idraw_tc2f(idraw, u0, v0);
            idraw_v3fv(idraw, vt0);
            idraw_tc2f(idraw, u0, v1);
            idraw_v3fv(idraw, vt2);
            idraw_tc2f(idraw, u1, v0);
            idraw_v3fv(idraw, vt1);

            // Second triangle
            idraw_c4ub(idraw, c.r, c.g, c.b, c.a);
            idraw_tc2f(idraw, u0, v1);
            idraw_v3fv(idraw, vt2);
            idraw_tc2f(idraw, u1, v1);
            idraw_v3fv(idraw, vt3);
            idraw_tc2f(idraw, u1, v0);
            idraw_v3fv(idraw, vt1);

            idraw_end(idraw);

        } break;
    }
}

void idraw_rect3Dvd(idraw_t* idraw, vec3 xyz, vec3 whd, vec2 uv0, vec2 uv1, Color256 c, gfx_primitive_type type) {
    idraw_rect3Dv(idraw, xyz, vec3_add(xyz, whd), uv0, uv1, c, type);
}

void idraw_circle_sector(idraw_t* idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    if (radius <= 0.0f) {
        radius = 0.1f;
    }

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        int32_t tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        f32 th = acosf(2 * powf(1 - idraw_smooth_circle_error_rate / radius, 2) - 1);
        segments = (int32_t)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) {
            segments = 4;
        }
    }

    f32 step = (f32)(end_angle - start_angle) / (f32)segments;
    f32 angle = (f32)start_angle;
    NEKO_FOR_RANGE_N(segments, i) {
        vec2 _a = neko_v2(cx, cy);
        vec2 _b = neko_v2(cx + sinf(idraw_deg2rad * angle) * radius, cy + cosf(idraw_deg2rad * angle) * radius);
        vec2 _c = neko_v2(cx + sinf(idraw_deg2rad * (angle + step)) * radius, cy + cosf(idraw_deg2rad * (angle + step)) * radius);
        idraw_trianglev(idraw, _a, _b, _c, color256(r, g, b, a), type);
        angle += step;
    }
}

void idraw_circle_sectorvx(idraw_t* idraw, vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, Color256 color, gfx_primitive_type type) {
    if (radius <= 0.0f) {
        radius = 0.1f;
    }

    // Cache elements of center vector
    f32 cx = c.x, cy = c.y, cz = c.z;

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        int32_t tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        f32 th = acosf(2 * powf(1 - idraw_smooth_circle_error_rate / radius, 2) - 1);
        segments = (int32_t)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) {
            segments = 4;
        }
    }

    f32 step = (f32)(end_angle - start_angle) / (f32)segments;
    f32 angle = (f32)start_angle;
    NEKO_FOR_RANGE_N(segments, i) {
        vec3 _a = neko_v3(cx, cy, cz);
        vec3 _b = neko_v3(cx + sinf(idraw_deg2rad * angle) * radius, cy + cosf(idraw_deg2rad * angle) * radius, cz);
        vec3 _c = neko_v3(cx + sinf(idraw_deg2rad * (angle + step)) * radius, cy + cosf(idraw_deg2rad * (angle + step)) * radius, cz);
        idraw_trianglevx(idraw, _a, _b, _c, neko_v2s(0.f), neko_v2s(0.5f), neko_v2s(1.f), color, type);
        angle += step;
    }
}

void idraw_circle(idraw_t* idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    idraw_circle_sector(idraw, cx, cy, radius, 0, 360, segments, r, g, b, a, type);
}

void idraw_circlevx(idraw_t* idraw, vec3 c, f32 radius, int32_t segments, Color256 color, gfx_primitive_type type) {
    idraw_circle_sectorvx(idraw, c, radius, 0, 360, segments, color, type);
}

void idraw_arc(idraw_t* idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    if (start_angle == end_angle) return;

    if (start_angle > end_angle) {
        f32 tmp = end_angle;
        end_angle = start_angle;
        start_angle = tmp;
    }

    if (radius_outer < radius_inner) {
        f32 tmp = radius_outer;
        radius_outer = radius_inner;
        radius_inner = tmp;
    }

    int32_t min_segments = (int32_t)((end_angle - start_angle) / 90.f);

    if (segments < min_segments) {
        f32 th = acosf(2 * powf(1 - idraw_smooth_circle_error_rate / radius_outer, 2) - 1);
        segments = (int32_t)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) segments = min_segments;
    }

    // Not a ring
    if (radius_inner <= 0.0f) {
        // BALLS
        idraw_circle_sector(idraw, cx, cy, radius_outer, (i32)start_angle, (i32)end_angle, segments, r, g, b, a, type);
        return;
    }

    f32 step = (end_angle - start_angle) / (f32)segments;
    f32 angle = start_angle;

    for (int i = 0; i < segments; i++) {
        f32 ar = neko_deg2rad(angle);
        f32 ars = neko_deg2rad((angle + step));

        idraw_trianglev(idraw, neko_v2(cx + sinf(ar) * radius_inner, cy + cosf(ar) * radius_inner), neko_v2(cx + sinf(ars) * radius_inner, cy + cosf(ars) * radius_inner),
                             neko_v2(cx + sinf(ar) * radius_outer, cy + cosf(ar) * radius_outer), color256(r, g, b, a), type);

        idraw_trianglev(idraw, neko_v2(cx + sinf(ars) * radius_inner, cy + cosf(ars) * radius_inner), neko_v2(cx + sinf(ars) * radius_outer, cy + cosf(ars) * radius_outer),
                             neko_v2(cx + sinf(ar) * radius_outer, cy + cosf(ar) * radius_outer), color256(r, g, b, a), type);

        angle += step;
    }
}

void idraw_box(idraw_t* idraw, f32 x, f32 y, f32 z, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    f32 width = hx;
    f32 height = hy;
    f32 length = hz;

    vec3 v0 = neko_v3(x - width, y - height, z + length);
    vec3 v1 = neko_v3(x + width, y - height, z + length);
    vec3 v2 = neko_v3(x - width, y + height, z + length);
    vec3 v3 = neko_v3(x + width, y + height, z + length);
    vec3 v4 = neko_v3(x - width, y - height, z - length);
    vec3 v5 = neko_v3(x - width, y + height, z - length);
    vec3 v6 = neko_v3(x + width, y - height, z - length);
    vec3 v7 = neko_v3(x + width, y + height, z - length);

    Color256 color = color256(r, g, b, a);

    switch (type) {
        case R_PRIMITIVE_TRIANGLES: {
            idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);
            {
                vec2 uv0 = neko_v2(0.f, 0.f);
                vec2 uv1 = neko_v2(1.f, 0.f);
                vec2 uv2 = neko_v2(0.f, 1.f);
                vec2 uv3 = neko_v2(1.f, 1.f);

                // Front Face
                idraw_trianglevx(idraw, v0, v1, v2, uv0, uv1, uv2, color, type);
                idraw_trianglevx(idraw, v3, v2, v1, uv3, uv2, uv1, color, type);

                // Back face
                idraw_trianglevx(idraw, v6, v5, v7, uv0, uv3, uv2, color, type);
                idraw_trianglevx(idraw, v6, v4, v5, uv0, uv1, uv3, color, type);

                // Top face
                idraw_trianglevx(idraw, v7, v2, v3, uv0, uv3, uv2, color, type);
                idraw_trianglevx(idraw, v7, v5, v2, uv0, uv1, uv3, color, type);

                // Bottom face
                idraw_trianglevx(idraw, v4, v1, v0, uv0, uv3, uv2, color, type);
                idraw_trianglevx(idraw, v4, v6, v1, uv0, uv1, uv3, color, type);

                // Right face
                idraw_trianglevx(idraw, v1, v7, v3, uv0, uv3, uv2, color, type);
                idraw_trianglevx(idraw, v1, v6, v7, uv0, uv1, uv3, color, type);

                // Left face
                idraw_trianglevx(idraw, v4, v2, v5, uv0, uv3, uv2, color, type);
                idraw_trianglevx(idraw, v4, v0, v2, uv0, uv1, uv3, color, type);
            }
            idraw_end(idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            Color256 color = color256(r, g, b, a);
            idraw_tc2f(idraw, 0.f, 0.f);

            // Front face
            idraw_line3Dv(idraw, v0, v1, color);
            idraw_line3Dv(idraw, v1, v3, color);
            idraw_line3Dv(idraw, v3, v2, color);
            idraw_line3Dv(idraw, v2, v0, color);

            // Back face
            idraw_line3Dv(idraw, v4, v6, color);
            idraw_line3Dv(idraw, v6, v7, color);
            idraw_line3Dv(idraw, v7, v5, color);
            idraw_line3Dv(idraw, v5, v4, color);

            // Right face
            idraw_line3Dv(idraw, v1, v6, color);
            idraw_line3Dv(idraw, v6, v7, color);
            idraw_line3Dv(idraw, v7, v3, color);
            idraw_line3Dv(idraw, v3, v1, color);

            // Left face
            idraw_line3Dv(idraw, v4, v6, color);
            idraw_line3Dv(idraw, v6, v1, color);
            idraw_line3Dv(idraw, v1, v0, color);
            idraw_line3Dv(idraw, v0, v4, color);

            // Bottom face
            idraw_line3Dv(idraw, v5, v7, color);
            idraw_line3Dv(idraw, v7, v3, color);
            idraw_line3Dv(idraw, v3, v2, color);
            idraw_line3Dv(idraw, v2, v5, color);

            // Top face
            idraw_line3Dv(idraw, v0, v4, color);
            idraw_line3Dv(idraw, v4, v5, color);
            idraw_line3Dv(idraw, v5, v2, color);
            idraw_line3Dv(idraw, v2, v0, color);

        } break;
    }
}

void idraw_sphere(idraw_t* idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    // Modified from: http://www.songho.ca/opengl/gl_sphere.html
    const u32 stacks = 32;
    const u32 sectors = 64;
    f32 sector_step = 2.f * (f32)neko_pi / (f32)sectors;
    f32 stack_step = (f32)neko_pi / (f32)stacks;
    struct {
        vec3 p;
        vec2 uv;
    } v0, v1, v2, v3;
    Color256 color = color256(r, g, b, a);

// TODO: Need to get these verts to be positioned correctly (translate then rotate all verts to correct for odd 90 degree rotation)
#define make_vert(V, I, J, XZ, SECANGLE)                   \
    do {                                                   \
        /* vertex position (x, y, z) */                    \
        V.p.x = cx + (XZ) * cosf((SECANGLE));              \
        V.p.z = cz + (XZ) * sinf((SECANGLE));              \
        /* vertex tex coord (s, t) range between [0, 1] */ \
        V.uv.x = (f32)(J) / sectors;                       \
        V.uv.y = (f32)(I) / stacks;                        \
    } while (0)

#define push_vert(V)                               \
    do {                                           \
        idraw_tc2f(idraw, V.s, V.t);     \
        idraw_v3f(idraw, V.x, V.y, V.z); \
    } while (0)

    for (u32 i = 0; i < stacks; ++i) {
        f32 sa0 = neko_pi / 2.f - i * stack_step;
        f32 sa1 = neko_pi / 2.f - (i + 1) * stack_step;
        f32 xz0 = radius * cosf(sa0);
        f32 xz1 = radius * cosf(sa1);
        f32 y0 = cy + radius * sinf(sa0);  // r * sin(u)
        f32 y1 = cy + radius * sinf(sa1);  // r * sin(u)

        v0.p.y = y0;
        v1.p.y = y0;
        v2.p.y = y1;
        v3.p.y = y1;

        for (u32 j = 0; j < sectors; ++j) {
            f32 sca0 = j * sector_step;  // starting from 0 to 2pi
            f32 sca1 = (j + 1) * sector_step;

            // Make verts
            make_vert(v0, i, j, xz0, sca0);
            make_vert(v1, i, j + 1, xz0, sca1);
            make_vert(v2, i + 1, j, xz1, sca0);
            make_vert(v3, i + 1, j + 1, xz1, sca1);

            // First triangle
            idraw_trianglevx(idraw, v0.p, v3.p, v2.p, v0.uv, v3.uv, v2.uv, color, type);
            // Second triangle
            idraw_trianglevx(idraw, v0.p, v1.p, v3.p, v0.uv, v1.uv, v3.uv, color, type);
        }
    }
}

// Modified from Raylib's implementation
void idraw_bezier(idraw_t* idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a) {
    vec2 start = neko_v2(x0, y0);
    vec2 end = neko_v2(x1, y1);
    vec2 previous = start;
    vec2 current = NEKO_DEFAULT_VAL();
    Color256 color = color256(r, g, b, a);
    const u32 bezier_line_divisions = 24;

    for (int i = 1; i <= bezier_line_divisions; i++) {
        current.y = neko_ease_cubic_in_out((f32)i, start.y, end.y - start.y, (f32)bezier_line_divisions);
        current.x = previous.x + (end.x - start.x) / (f32)bezier_line_divisions;
        idraw_linev(idraw, previous, current, color);
        previous = current;
    }
}

void idraw_cylinder(idraw_t* idraw, f32 x, f32 y, f32 z, f32 r_top, f32 r_bottom, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    if (sides < 3) sides = 3;

    int32_t numVertex = sides * 8;
    const f32 hh = height * 0.5f;

    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES: {
            idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);
            {
                idraw_c4ub(idraw, r, g, b, a);

                if (sides < 3) sides = 3;

                numVertex = sides * 6;

                if (r_top > 0) {
                    // Draw Body -------------------------------------------------------------------------------------
                    for (int i = 0; i < 360; i += 360 / sides) {
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(idraw_deg2rad * i) * r_bottom);  // Bottom Left
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh,
                                       z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);                                                                                // Bottom Right
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_top);  // Top Right

                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_top, y + hh, z + cosf(idraw_deg2rad * i) * r_top);                                        // Top Left
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(idraw_deg2rad * i) * r_bottom);                                  // Bottom Left
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_top);  // Top Right
                    }

                    // Draw Cap --------------------------------------------------------------------------------------
                    for (int i = 0; i < 360; i += 360 / sides) {
                        idraw_v3f(idraw, x + 0, y + hh, z + 0);
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_top, y + hh, z + cosf(idraw_deg2rad * i) * r_top);
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_top);
                    }
                } else {
                    // Draw Cone -------------------------------------------------------------------------------------
                    for (int i = 0; i < 360; i += 360 / sides) {
                        idraw_v3f(idraw, x + 0, y + hh, z + 0);
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(idraw_deg2rad * i) * r_bottom);
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);
                    }
                }

                // Draw Base -----------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360 / sides) {
                    idraw_v3f(idraw, x + 0, y - hh, z + 0);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(idraw_deg2rad * i) * r_bottom);
                }
            }
            idraw_end(idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            idraw_begin(idraw, R_PRIMITIVE_LINES);
            {
                idraw_c4ub(idraw, r, g, b, a);

                for (int32_t i = 0; i < 360; i += 360 / sides) {
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_bottom, y - hh, cosf(idraw_deg2rad * i) * r_bottom);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);

                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_top);

                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_top);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_top, y + hh, z + cosf(idraw_deg2rad * i) * r_top);

                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_top, y + hh, z + cosf(idraw_deg2rad * i) * r_top);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(idraw_deg2rad * i) * r_bottom);
                }

                // Draw Top/Bottom circles
                for (int i = 0; i < 360; i += 360 / sides) {
                    idraw_v3f(idraw, x, y - hh, z);
                    idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);

                    if (r_top) {
                        idraw_v3f(idraw, x + 0, y + hh, z + 0);
                        idraw_v3f(idraw, x + sinf(idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(idraw_deg2rad * (i + 360.0f / sides)) * r_top);
                    }
                }
            }
            idraw_end(idraw);

        } break;
    }
}

void idraw_cone(idraw_t* idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type) {
    idraw_cylinder(idraw, x, y, z, 0.f, radius, height, sides, r, g, b, a, type);
}

#if 0
void idraw_text(idraw_t* idraw, f32 x, f32 y, const char* text, const neko_asset_font_t* fp, bool flip_vertical, Color256 col) {
    // 如果没有指定字体 则使用默认字体
    if (!fp) {
        fp = &idraw()->font_default;
    }

    idraw_texture(idraw, fp->texture.hndl);

    mat4 rot = mat4_rotatev(neko_deg2rad(-180.f), NEKO_XAXIS);

    // Get total dimensions of text
    vec2 td = neko_asset_font_text_dimensions(fp, text, -1);
    f32 th = neko_asset_font_max_height(fp);

    // Move text to accomdate height
    y += NEKO_MAX(td.y, th);

    // Needs to be fixed in here. Not elsewhere.
    idraw_begin(idraw, R_PRIMITIVE_TRIANGLES);
    {
        idraw_c4ub(idraw, col.r, col.g, col.b, col.a);
        while (text[0] != '\0') {
            char c = text[0];
            if (c >= 32 && c <= 127) {
                stbtt_aligned_quad q = NEKO_DEFAULT_VAL();
                stbtt_GetBakedQuad((stbtt_bakedchar*)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);

                vec3 v0 = neko_v3(q.x0, q.y0, 0.f);  // TL
                vec3 v1 = neko_v3(q.x1, q.y0, 0.f);  // TR
                vec3 v2 = neko_v3(q.x0, q.y1, 0.f);  // BL
                vec3 v3 = neko_v3(q.x1, q.y1, 0.f);  // BR

                if (flip_vertical) {
                    v0 = mat4_mul_vec3(rot, v0);
                    v1 = mat4_mul_vec3(rot, v1);
                    v2 = mat4_mul_vec3(rot, v2);
                    v3 = mat4_mul_vec3(rot, v3);
                }

                vec2 uv0 = neko_v2(q.s0, q.t0);  // TL
                vec2 uv1 = neko_v2(q.s1, q.t0);  // TR
                vec2 uv2 = neko_v2(q.s0, q.t1);  // BL
                vec2 uv3 = neko_v2(q.s1, q.t1);  // BR

                idraw_tc2fv(idraw, uv0);
                idraw_v3fv(idraw, v0);

                idraw_tc2fv(idraw, uv3);
                idraw_v3fv(idraw, v3);

                idraw_tc2fv(idraw, uv2);
                idraw_v3fv(idraw, v2);

                idraw_tc2fv(idraw, uv0);
                idraw_v3fv(idraw, v0);

                idraw_tc2fv(idraw, uv1);
                idraw_v3fv(idraw, v1);

                idraw_tc2fv(idraw, uv3);
                idraw_v3fv(idraw, v3);
            }
            text++;
        }
    }
    idraw_end(idraw);
}
#endif

void idraw_text(idraw_t* idraw, f32 x, f32 y, const char* text, FontFamily* fp, bool flip_vertical, Color256 col) {
    //

    if (!fp) {
        fp = neko_default_font();
    }

    f32 fy = draw_font(idraw, fp, 22.f, x, y, text, col);
}

// View/Scissor 命令
void idraw_set_view_scissor(idraw_t* idraw, u32 x, u32 y, u32 w, u32 h) {
    // 刷新上一个
    idraw_flush(idraw);

    // Set graphics viewport scissor
    gfx_set_view_scissor(&idraw->commands, x, y, w, h);
}

// 最终提交/合并
void idraw_draw(idraw_t* idraw, command_buffer_t* cb) {
    // 最终刷新 可以改为 idraw_end() 的一部分
    idraw_flush(idraw);

    // 将 idraw 命令合并到 cb 末尾
    byte_buffer_write_bulk(&cb->commands, idraw->commands.commands.data, idraw->commands.commands.position);

    // 增加合并缓冲区的命令数量
    cb->num_commands += idraw->commands.num_commands;

    // 重置缓存
    idraw_reset(idraw);
}

void idraw_renderpass_submit(idraw_t* idraw, command_buffer_t* cb, vec4 viewport, Color256 c) {
    gfx_clear_action_t action = NEKO_DEFAULT_VAL();
    action.color[0] = (f32)c.r / 255.f;
    action.color[1] = (f32)c.g / 255.f;
    action.color[2] = (f32)c.b / 255.f;
    action.color[3] = (f32)c.a / 255.f;
    neko_renderpass_t pass = NEKO_DEFAULT_VAL();
    gfx_renderpass_begin(cb, pass);
    gfx_set_viewport(cb, (u32)viewport.x, (u32)viewport.y, (u32)viewport.z, (u32)viewport.w);
    gfx_clear(cb, action);
    idraw_draw(idraw, cb);
    gfx_renderpass_end(cb);
}

void idraw_renderpass_submit_ex(idraw_t* idraw, command_buffer_t* cb, vec4 viewport, gfx_clear_action_t action) {
    neko_renderpass_t pass = NEKO_DEFAULT_VAL();
    gfx_renderpass_begin(cb, pass);
    gfx_set_viewport(cb, (u32)viewport.x, (u32)viewport.y, (u32)viewport.z, (u32)viewport.w);
    gfx_clear(cb, action);
    idraw_draw(idraw, cb);
    gfx_renderpass_end(cb);
}
