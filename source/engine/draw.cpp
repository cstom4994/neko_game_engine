#include "engine/draw.h"

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "base/common/vfs.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/graphics.h"
#include "base/scripting/lua_wrapper.hpp"
#include "engine/ecs/entity.h"
#include "engine/edit.h"

// deps
#include "extern/cute_aseprite.h"

// deps
#include "extern/stb_image.h"
#include "extern/stb_rect_pack.h"
#include "extern/stb_truetype.h"

using namespace Neko::luabind;

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
    if (g_renderer.draw_colors_len == NEKO_ARR_SIZE(g_renderer.draw_colors)) {
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
    if (g_renderer.matrices_len == NEKO_ARR_SIZE(g_renderer.matrices)) {
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

mat4 _rotate(float angle) {
    mat4 top = mat4_identity();

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
        float x = c * top.m[0][i] - s * top.m[1][i];
        float y = s * top.m[0][i] + c * top.m[1][i];
        top.m[0][i] = x;
        top.m[1][i] = y;
    }
#endif

    return top;
}

DrawDescription draw_description_args(lua_State *L, i32 arg_start) {
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

RectDescription rect_description_args(lua_State *L, i32 arg_start) {
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

void draw_sprite(AseSprite *spr, DrawDescription *desc) {
    bool ok = false;

    batch_renderer *b = the<CL>().batch;

    neko_assert(b);

    AseSpriteView view = {};
    ok = view.make(spr);
    if (!ok) {
        return;
    }

    GLuint gl_tex_id = view.data.tex.id;

    AseSpriteFrame f = view.data.frames[view.frame()];

    batch_texture(b, gl_tex_id);

    desc->x -= desc->ox;
    desc->y -= desc->oy;

    float u1 = f.u0;
    float v1 = f.v0;
    float u2 = f.u1;
    float v2 = f.v1;

    // 计算旋转角度的 cos 和 sin
    float cos_rot = std::cos(desc->rotation);
    float sin_rot = std::sin(desc->rotation);

    float hw = view.data.width * 0.5f * desc->sx;   // 半宽
    float hh = view.data.height * 0.5f * desc->sy;  // 半高

    // 计算四个角的旋转后坐标
    float x1 = -hw;
    float y1 = -hh;
    float x2 = hw;
    float y2 = hh;

    // 左上角
    float x1_rot = cos_rot * x1 - sin_rot * y1 + desc->x + hw;
    float y1_rot = sin_rot * x1 + cos_rot * y1 + desc->y + hh;

    // 右下角
    float x2_rot = cos_rot * x2 - sin_rot * y2 + desc->x + hw;
    float y2_rot = sin_rot * x2 + cos_rot * y2 + desc->y + hh;

    // 左下角
    float x3_rot = cos_rot * x1 - sin_rot * y2 + desc->x + hw;
    float y3_rot = sin_rot * x1 + cos_rot * y2 + desc->y + hh;

    // 右上角
    float x4_rot = cos_rot * x2 - sin_rot * y1 + desc->x + hw;
    float y4_rot = sin_rot * x2 + cos_rot * y1 + desc->y + hh;

    // 绘制两个三角形
    batch_push_vertex(b, x1_rot, y1_rot, u1, v1);  // 左上
    batch_push_vertex(b, x2_rot, y2_rot, u2, v2);  // 右下
    batch_push_vertex(b, x3_rot, y3_rot, u1, v2);  // 左下

    batch_push_vertex(b, x1_rot, y1_rot, u1, v1);  // 左上
    batch_push_vertex(b, x4_rot, y4_rot, u2, v1);  // 右上
    batch_push_vertex(b, x2_rot, y2_rot, u2, v2);  // 右下

    if (spr->effects.any()) {
        b->outline = spr->effects[0];
        b->glow = spr->effects[1];
        b->bloom = spr->effects[2];
        b->trans = spr->effects[3];
        batch_flush(b);
    } else {
        b->outline = b->glow = b->bloom = b->trans = false;
    }
}

static Asset font_shader = {};
static GLuint font_vbo, font_vao;

bool FontFamily::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }

    FontFamily f = {};
    f.ttf = contents;
    f.sb = {};
    *this = f;
    return true;
}

void FontFamily::trash() {
    for (auto [k, v] : ranges) {
        neko_deinit_texture(&v->tex);
    }
    sb.trash();
    ranges.trash();

    mem_free(ttf.data);
}

struct FontKey {
    float size;
    i32 ch;
};

static FontKey font_key(float size, i32 charcode) {
    FontKey fk = {};
    fk.size = size;
    fk.ch = (charcode / NEKO_ARR_SIZE(FontRange::chars)) * NEKO_ARR_SIZE(FontRange::chars);
    return fk;
}

static void make_font_range(FontRange *out, FontFamily *font, FontKey key) {
    PROFILE_FUNC();

    i32 width = 64;
    i32 height = 64;

    u8 *bitmap = nullptr;
    while (bitmap == nullptr) {
        PROFILE_BLOCK("try bake");

        bitmap = (u8 *)mem_alloc(width * height);
        i32 res = stbtt_BakeFontBitmap((u8 *)font->ttf.data, 0, key.size, bitmap, width, height, key.ch, NEKO_ARR_SIZE(out->chars), out->chars);
        if (res < 0) {  // 如果图片大小不够就扩大
            mem_free(bitmap);
            bitmap = nullptr;
            width *= 2;
            height *= 2;
        }
    }
    neko_defer(mem_free(bitmap));

    {
        PROFILE_BLOCK("make image");
        AssetTexture tex{};
        neko_init_texture_from_memory_uncompressed(&tex, bitmap, width, height, 1, TEXTURE_ALIASED);

        out->tex.id = tex.id;
        out->tex.width = width;
        out->tex.height = height;
    }

    LOG_INFO("created font range with id {}", out->tex.id);
}

static FontRange *get_range(FontFamily *font, FontKey key) {
    u64 hash = *(u64 *)&key;
    FontRange *range = font->ranges.get(hash);
    if (range == nullptr) {
        range = &font->ranges[hash];
        make_font_range(range, font, key);
    }

    return range;
}

stbtt_aligned_quad FontFamily::quad(u32 *img, float *x, float *y, float size, i32 ch, float xscale) {

    FontRange *range = get_range(this, font_key(size, ch));
    assert(range != nullptr);

    ch = ch % NEKO_ARR_SIZE(FontRange::chars);

    float xpos = 0;
    float ypos = 0;
    stbtt_aligned_quad q = {};
    stbtt_GetBakedQuad(range->chars, (i32)range->tex.width, (i32)range->tex.height, ch, &xpos, &ypos, &q, 1);

    stbtt_bakedchar *baked = range->chars + ch;
    *img = range->tex.id;
    *x = *x + baked->xadvance * xscale;
    return q;
}

float FontFamily::width(float size, String text) {
    float width = 0;
    for (Rune r : UTF8(text)) {
        u32 code = r.charcode();
        FontRange *range = get_range(this, font_key(size, code));
        assert(range != nullptr);

        const stbtt_bakedchar *baked = range->chars + (code % NEKO_ARR_SIZE(FontRange::chars));
        width += baked->xadvance;
    }
    return width;
}

FontFamily *neko_default_font() {
    if (the<CL>().default_font == nullptr) {
        the<CL>().default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
        the<CL>().default_font->load(the<CL>().state.default_font);
    }
    return the<CL>().default_font;
}

static void draw_font_line(FontFamily *font, bool draw_in_world, float size, float *start_x, float *start_y, String line, Color256 col, f32 scale) {
    float x = *start_x;
    float y = *start_y;

    GLuint font_program = font_shader.shader.id;

    glUseProgram(font_program);
    glUniform3f(glGetUniformLocation(font_program, "textColor"), col.r / 255.f, col.g / 255.f, col.b / 255.f);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(font_program, "text"), 0);

    if (draw_in_world) {
        glUniformMatrix3fv(glGetUniformLocation(font_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());
        glUniform1i(glGetUniformLocation(font_program, "mode"), 0);
    } else {
        mat4 cam_mat = mat4_ortho(0.0f, (float)the<CL>().state.width, (float)the<CL>().state.height, 0.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(font_program, "u_mvp"), 1, GL_FALSE, (const GLfloat *)cam_mat.elements);
        glUniform1i(glGetUniformLocation(font_program, "mode"), 1);
    }

    glBindVertexArray(font_vao);

    for (Rune r : UTF8(line)) {
        u32 tex_id = 0;
        float xx = x;
        float yy = y;
        stbtt_aligned_quad q = font->quad(&tex_id, &xx, &yy, size, r.charcode(), scale);

        GLuint gl_tex_id = tex_id;

        float xpos = x + q.x0 * scale;
        float ypos = y + q.y0 * scale;

        float w = (q.x1 - q.x0) * scale;
        float h = (q.y1 - q.y0) * scale;

        // clang-format off
        float vertices[6][4] = {
            {xpos,      ypos + h,   q.s0, q.t1}, 
            {xpos,      ypos,       q.s0, q.t0},     
            {xpos + w,  ypos,       q.s1, q.t0},
            {xpos,      ypos + h,   q.s0, q.t1}, 
            {xpos + w,  ypos,       q.s1, q.t0}, 
            {xpos + w,  ypos + h,   q.s1, q.t1}
        };
        // clang-format on

        glBindTexture(GL_TEXTURE_2D, gl_tex_id);
        glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x = xx;
        y = yy;
    }

    *start_y += size * scale;

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float draw_font(FontFamily *font, bool draw_in_world, float size, float x, float y, String text, Color256 col, f32 scale) {
    PROFILE_FUNC();

    y += size;

    for (String line : SplitLines(text)) {
        draw_font_line(font, draw_in_world, size, &x, &y, line, col, scale);
    }

    return y - size;
}

float draw_font_wrapped(FontFamily *font, bool draw_in_world, float size, float x, float y, String text, Color256 col, float limit, f32 scale) {
    PROFILE_FUNC();

    y += size;

    for (String line : SplitLines(text)) {
        font->sb.clear();
        StringScanner scan = line;

        for (String word = scan.next_string(); word != ""; word = scan.next_string()) {

            font->sb << word;

            float width = font->width(size, String(font->sb));
            if (width < limit) {
                font->sb << " ";
                continue;
            }

            font->sb.len -= word.len;
            font->sb.data[font->sb.len] = '\0';

            draw_font_line(font, draw_in_world, size, &x, &y, String(font->sb), col, scale);

            font->sb.clear();
            font->sb << word << " ";
        }

        draw_font_line(font, draw_in_world, size, &x, &y, String(font->sb), col, scale);
    }

    return y - size;
}

void font_init() {

    // 编译着色器并创建程序
    bool ok = asset_load_kind(AssetKind_Shader, "shader/font.glsl", &font_shader);
    error_assert(ok);

    glGenVertexArrays(1, &font_vao);
    glGenBuffers(1, &font_vbo);

    glBindVertexArray(font_vao);

    glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static Asset batch_shader = {};

batch_renderer *batch_init(int vertex_capacity) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_capacity, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));

    if (batch_shader.shader.id == 0) {
        bool ok = asset_load_kind(AssetKind_Shader, "shader/batch.glsl", &batch_shader);
        error_assert(ok);
    }

    batch_renderer *batch = (batch_renderer *)mem_alloc(sizeof(batch_renderer));

    asset_load(AssetLoadData{AssetKind_Image, false}, "assets/aliens.png", NULL);

    // batch->shader = program;
    batch->vao = vao;
    batch->vbo = vbo;
    batch->vertex_count = 0;
    batch->vertex_capacity = vertex_capacity;
    batch->vertices = (Vertex *)mem_alloc(sizeof(Vertex) * vertex_capacity);
    batch->texture = 0;
    batch->scale = 0;

    return batch;
}

void batch_fini(batch_renderer *batch) {
    mem_free(batch->vertices);
    mem_free(batch);
}

int batch_update_all(Event evt) {

    AssetTexture tex_aliens = texture_get_ptr("assets/aliens.png");

    auto &game = the<CL>();

    struct {
        float x, y, w, h;
    } alien_uvs[] = {
            {2, 2, 24, 24}, {58, 2, 24, 24}, {114, 2, 24, 24}, {170, 2, 24, 24}, {2, 30, 24, 24},
    };

    struct {
        // position
        float px, py;
        // texcoords
        float tx, ty, tw, th;
    } ch = {
            .px = 0,
            .py = 48,
            .tx = alien_uvs[2].x,
            .ty = alien_uvs[2].y,
            .tw = alien_uvs[2].w,
            .th = alien_uvs[2].h,
    };

    batch_texture(game.batch, tex_aliens.id);

    float x1 = ch.px;
    float y1 = ch.py;
    float x2 = ch.px + 24;
    float y2 = ch.py + 24;

    float u1 = ch.tx / tex_aliens.width;
    float v1 = ch.ty / tex_aliens.height;
    float u2 = (ch.tx + ch.tw) / tex_aliens.width;
    float v2 = (ch.ty + ch.th) / tex_aliens.height;

    batch_push_vertex(game.batch, x1, y1, u1, v1);
    batch_push_vertex(game.batch, x2, y2, u2, v2);
    batch_push_vertex(game.batch, x1, y2, u1, v2);

    batch_push_vertex(game.batch, x1, y1, u1, v1);
    batch_push_vertex(game.batch, x2, y1, u2, v1);
    batch_push_vertex(game.batch, x2, y2, u2, v2);

    return 0;
}

void batch_draw_all(batch_renderer *batch) { batch_flush(batch); }

void batch_flush(batch_renderer *renderer) {
    if (renderer->vertex_count == 0) {
        return;
    }

    GLuint sid = batch_shader.shader.id;

    glUseProgram(sid);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->texture);

    glUniform1i(glGetUniformLocation(sid, "outline_enable"), renderer->outline);
    glUniform1i(glGetUniformLocation(sid, "glow_enable"), renderer->glow);
    glUniform1i(glGetUniformLocation(sid, "bloom_enable"), renderer->bloom);
    glUniform1i(glGetUniformLocation(sid, "trans_enable"), renderer->trans);

    glUniform1i(glGetUniformLocation(sid, "u_texture"), 0);
    // glUniformMatrix4fv(glGetUniformLocation(sid, "u_mvp"), 1, GL_FALSE, (const GLfloat *)&renderer->mvp.cols[0]);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    glUniform1f(glGetUniformLocation(sid, "scale"), renderer->scale);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * renderer->vertex_count, renderer->vertices);

    glBindVertexArray(renderer->vao);
    glDrawArrays(GL_TRIANGLES, 0, renderer->vertex_count);

    renderer->vertex_count = 0;
}

void batch_texture(batch_renderer *renderer, GLuint id) {
    if (renderer->texture != id) {
        batch_flush(renderer);
        renderer->texture = id;
    }
}

void batch_push_vertex(batch_renderer *renderer, float x, float y, float u, float v) {
    if (renderer->vertex_count == renderer->vertex_capacity) {
        batch_flush(renderer);
    }

    renderer->vertices[renderer->vertex_count++] = Vertex{
            .position = {x, y},
            .texcoord = {u, v},
    };
}

#define MAX_VERTS 3 * 2048

struct LineVertex {
    union {
        struct {
            vec3 pos;
            float width;
        };
        vec4 pos_width;
    };
    Color col;
};

struct DebugRenderer {
    Asset lines_shader = {};

    GLuint program_id;
    GLuint vao;
    GLuint vbo;

    GLuint view;
    GLuint viewport_size;
    GLuint aa_radius;

    GLuint pos_width;
    GLuint col;

    u32 buf_cap;
    u32 buf_len;
    LineVertex *buf;
};

DebugRenderer *debug_renderer;

void debug_draw_add_line(vec2 a, f32 line_width, Color color) {
    if (debug_renderer->buf_cap - debug_renderer->buf_len >= 1) {
        LineVertex *dst = debug_renderer->buf + debug_renderer->buf_len;
        *dst++ = LineVertex{{a.x, a.y, 0.0, line_width}, color};
        debug_renderer->buf_len += 1;
    } else {
        // printf("Buffer full: buf_cap=%d, buf_len=%d\n", debug_renderer->buf_cap, debug_renderer->buf_len);
    }
}

void debug_draw_add_line(vec2 a, vec2 b, f32 line_width, Color color) {
    if (debug_renderer->buf_cap - debug_renderer->buf_len >= 2) {
        LineVertex *dst = debug_renderer->buf + debug_renderer->buf_len;  // 从 buf_len 偏移出正确的写入位置
        *dst++ = LineVertex{{a.x, a.y, 0.0, line_width}, color};
        *dst++ = LineVertex{{b.x, b.y, 0.0, line_width}, color};
        debug_renderer->buf_len += 2;  // 更新已使用的缓冲区长度
    } else {
        // printf("Buffer full: buf_cap=%d, buf_len=%d\n", debug_renderer->buf_cap, debug_renderer->buf_len);
    }
}

void debug_draw_init() {
    debug_renderer = mem_new<DebugRenderer>();

    bool ok = asset_load_kind(AssetKind_Shader, "shader/debug_line.glsl", &debug_renderer->lines_shader);
    error_assert(ok);

    debug_renderer->program_id = debug_renderer->lines_shader.shader.id;

    debug_renderer->pos_width = glGetAttribLocation(debug_renderer->program_id, "pos_width");
    debug_renderer->col = glGetAttribLocation(debug_renderer->program_id, "col");

    debug_renderer->view = glGetUniformLocation(debug_renderer->program_id, "inverse_view_matrix");
    debug_renderer->viewport_size = glGetUniformLocation(debug_renderer->program_id, "u_viewport_size");
    debug_renderer->aa_radius = glGetUniformLocation(debug_renderer->program_id, "u_aa_radius");

    GLuint binding_idx = 0;
    glCreateVertexArrays(1, &debug_renderer->vao);
    glCreateBuffers(1, &debug_renderer->vbo);
    glNamedBufferStorage(debug_renderer->vbo, MAX_VERTS * sizeof(LineVertex), NULL, GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(debug_renderer->vao, binding_idx, debug_renderer->vbo, 0, sizeof(LineVertex));

    glEnableVertexArrayAttrib(debug_renderer->vao, debug_renderer->pos_width);
    glEnableVertexArrayAttrib(debug_renderer->vao, debug_renderer->col);

    glVertexArrayAttribFormat(debug_renderer->vao, debug_renderer->pos_width, 4, GL_FLOAT, GL_FALSE, offsetof(LineVertex, pos_width));
    glVertexArrayAttribFormat(debug_renderer->vao, debug_renderer->col, 4, GL_FLOAT, GL_FALSE, offsetof(LineVertex, col));

    glVertexArrayAttribBinding(debug_renderer->vao, debug_renderer->pos_width, binding_idx);
    glVertexArrayAttribBinding(debug_renderer->vao, debug_renderer->col, binding_idx);

    debug_renderer->buf_cap = MAX_VERTS / 3;
    debug_renderer->buf_len = 0;
    debug_renderer->buf = (LineVertex *)mem_alloc(debug_renderer->buf_cap * sizeof(LineVertex));
}

void debug_draw_fini() {
    // glDeleteProgram(device->program_id);
    glDeleteBuffers(1, &debug_renderer->vbo);
    glDeleteVertexArrays(1, &debug_renderer->vao);

    mem_free(debug_renderer->buf);

    mem_del(debug_renderer);
    debug_renderer = nullptr;
}

u32 draw_line_update(const void *data, i32 n_elems, i32 elem_size) {
    glNamedBufferSubData(debug_renderer->vbo, 0, n_elems * elem_size, data);
    return n_elems;
}

void debug_draw_all() {

    debug_draw_circle({100.0f, 100.0f}, 10.0f, 32, 2.0, color_red);

    const i32 count = draw_line_update(debug_renderer->buf, debug_renderer->buf_len, sizeof(LineVertex));

    glUseProgram(debug_renderer->program_id);

    const mat3 *mat = camera_get_inverse_view_matrix_ptr();
    glUniformMatrix3fv(debug_renderer->view, 1, GL_FALSE, (const GLfloat *)mat);

    vec2 viewport{the<CL>().state.width, the<CL>().state.height};
    vec2 aa_radii{2.0f, 2.0f};
    glUniform2fv(debug_renderer->viewport_size, 1, reinterpret_cast<float *>(&viewport));
    glUniform2fv(debug_renderer->aa_radius, 1, reinterpret_cast<float *>(&aa_radii));

    glBindVertexArray(debug_renderer->vao);
    glDrawArrays(GL_LINES, 0, count);

    glBindVertexArray(0);
    glUseProgram(0);

    debug_renderer->buf_len = 0;
}

void debug_draw_circle(vec2 center, f32 radius, int segment_count, f32 line_width, Color color) {
    f32 angle_step = 2.0f * neko_pi / segment_count;
    vec2 prev_point = {center.x + radius, center.y};  // 起始点
    for (int i = 1; i <= segment_count; i++) {
        f32 angle = i * angle_step;
        vec2 next_point = {center.x + radius * cos(angle), center.y + radius * sin(angle)};
        debug_draw_add_line(prev_point, next_point, line_width, color);
        prev_point = next_point;
    }
    debug_draw_add_line(prev_point, {center.x + radius, center.y}, line_width, color);
}

void debug_draw_manifold(vec2 *points, u32 point_count, f32 line_width, Color color) {
    for (u32 i = 0; i < point_count - 1; ++i) {
        debug_draw_add_line(points[i], points[i + 1], line_width, color);
    }
}

void debug_draw_half_circle(vec2 center, f32 radius, vec2 direction, u32 segment_count, f32 line_width, Color color) {
    f32 angle_step = neko_pi / segment_count;                               // 半圆分段
    vec2 perpendicular = {-direction.y, direction.x};                       // 垂直于方向向量
    vec2 prev_point = vec2_add(center, vec2_scale(perpendicular, radius));  // 半圆起点
    for (u32 i = 1; i <= segment_count; ++i) {
        f32 angle = i * angle_step;
        vec2 next_point = {center.x + radius * (cosf(angle) * perpendicular.x + sinf(angle) * direction.x), center.y + radius * (cosf(angle) * perpendicular.y + sinf(angle) * direction.y)};
        debug_draw_add_line(prev_point, next_point, line_width, color);
        prev_point = next_point;
    }
}

void debug_draw_capsule(vec2 a, vec2 b, f32 radius, u32 segment_count, f32 line_width, Color color) {
    vec2 dir = vec2_normalize(vec2_sub(b, a));
    vec2 perpendicular = {-dir.y, dir.x};
    vec2 p1 = vec2_add(a, vec2_scale(perpendicular, radius));
    vec2 p2 = vec2_sub(a, vec2_scale(perpendicular, radius));
    vec2 p3 = vec2_add(b, vec2_scale(perpendicular, radius));
    vec2 p4 = vec2_sub(b, vec2_scale(perpendicular, radius));

    // 绘制中间的矩形部分
    debug_draw_add_line(p1, p3, line_width, color);
    debug_draw_add_line(p2, p4, line_width, color);

    // 绘制两端的半圆
    debug_draw_half_circle(a, radius, dir, segment_count, line_width, color);
    debug_draw_half_circle(b, radius, vec2_neg(dir), segment_count, line_width, color);
}
