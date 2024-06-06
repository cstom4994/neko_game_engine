
#include "engine/neko_asset.h"

void neko_fontbatch_init(neko_fontbatch_t *font_batch, const neko_vec2_t fbs, const_str img_path, char *content, int content_size) {

    font_batch->font_scale = 3.0f;

    font_batch->font_render = neko_render_batch_make_ctx(32);

    const char *font_vs =
            "#version 330\n"
            "uniform mat4 u_mvp;\n"
            "in vec2 in_pos; in vec2 in_uv;\n"
            "out vec2 v_uv;\n"
            "void main() {\n"
            "    v_uv = in_uv;\n"
            "    gl_Position = u_mvp * vec4(in_pos, 0, 1);\n"
            "}\n";
    const char *font_ps =
            "#version 330\n"
            "precision mediump float;\n"
            "uniform sampler2D u_sprite_texture;\n"
            "in vec2 v_uv; out vec4 out_col;\n"
            "void main() { out_col = texture(u_sprite_texture, v_uv); }\n";

    neko_render_batch_vertex_data_t font_vd;
    neko_render_batch_make_vertex_data(&font_vd, 1024 * 1024, GL_TRIANGLES, sizeof(neko_font_vert_t), GL_DYNAMIC_DRAW);
    neko_render_batch_add_attribute(&font_vd, "in_pos", 2, R_BATCH_FLOAT, NEKO_OFFSET(neko_font_vert_t, x));
    neko_render_batch_add_attribute(&font_vd, "in_uv", 2, R_BATCH_FLOAT, NEKO_OFFSET(neko_font_vert_t, u));

    neko_render_batch_make_renderable(&font_batch->font_renderable, &font_vd);
    neko_render_batch_load_shader(&font_batch->font_shader, font_vs, font_ps);
    neko_render_batch_set_shader(&font_batch->font_renderable, &font_batch->font_shader);

    neko_render_batch_ortho_2d(fbs.x / font_batch->font_scale, fbs.y / font_batch->font_scale, 0, 0, font_batch->font_projection);

    neko_render_batch_send_matrix(&font_batch->font_shader, "u_mvp", font_batch->font_projection);

    neko_image_t img = neko_image_load(img_path);
    font_batch->font_tex_id = generate_texture_handle(img.pix, img.w, img.h, NULL);
    font_batch->font = neko_font_load_bmfont(font_batch->font_tex_id, content, content_size, 0);
    if (font_batch->font->atlas_w != img.w || font_batch->font->atlas_h != img.h) {
        neko_log_warning("failed to load font");
    }
    neko_image_free(img);

    font_batch->font_verts = (neko_font_vert_t *)neko_safe_malloc(sizeof(neko_font_vert_t) * 1024 * 2);
}

void neko_fontbatch_end(neko_fontbatch_t *font_batch) {
    neko_safe_free(font_batch->font_verts);
    neko_font_free(font_batch->font);
    neko_render_batch_free(font_batch->font_render);
    destroy_texture_handle(font_batch->font_tex_id, NULL);
}

void neko_fontbatch_draw(neko_fontbatch_t *font_batch, const neko_vec2_t fbs, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale) {
    f32 text_w = (f32)neko_font_text_width(font_batch->font, text);
    f32 text_h = (f32)neko_font_text_height(font_batch->font, text);

    if (scale == 0.f) scale = font_batch->font_scale;

    neko_font_rect_t clip_rect;
    clip_rect.left = -fbs.x / scale * clip_region;
    clip_rect.right = fbs.x / scale * clip_region + 0.5f;
    clip_rect.top = fbs.y / scale * clip_region + 0.5f;
    clip_rect.bottom = -fbs.y / scale * clip_region;

    f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / scale + text_h / 2.f;
    f32 wrap_width = wrap_x - x0;

    neko_font_fill_vertex_buffer(font_batch->font, text, x0, y0, wrap_width, line_height, &clip_rect, font_batch->font_verts, 1024 * 2, &font_batch->font_vert_count);

    if (font_batch->font_vert_count) {
        neko_render_batch_draw_call_t call;
        call.textures[0] = (u32)font_batch->font->atlas_id;
        call.texture_count = 1;
        call.r = &font_batch->font_renderable;
        call.verts = font_batch->font_verts;
        call.vert_count = font_batch->font_vert_count;

        neko_render_batch_push_draw_call(font_batch->font_render, call);
    }
}

typedef struct ct_text {
    neko_fontbatch_t *fontbatch;
    const_str text;
} ct_text;
