#include "engine/ui.h"

#include <assert.h>
#include <stb_truetype.h>
#include <stdlib.h>
#include <string.h>

#include "engine/font.h"
#include "engine/glew_glfw.h"
#include "engine/input.h"
#include "vendor/atlas.h"

const u32 ui_renderer_max_quads = 800;

static engine_ui_renderer_t* mu_renderer;

static const char* neko_utf8_to_codepoint(const char* p, u32* dst) {
    u32 res, n;
    switch (*p & 0xf0) {
        case 0xf0:
            res = *p & 0x07;
            n = 3;
            break;
        case 0xe0:
            res = *p & 0x0f;
            n = 2;
            break;
        case 0xd0:
        case 0xc0:
            res = *p & 0x1f;
            n = 1;
            break;
        default:
            res = *p;
            n = 0;
            break;
    }
    while (n--) {
        res = (res << 6) | (*(++p) & 0x3f);
    }
    *dst = res;
    return p + 1;
}

engine_ui_renderer_t* neko_new_ui_renderer(u32 shader) {
    engine_ui_renderer_t* renderer = (engine_ui_renderer_t*)mem_alloc(sizeof(engine_ui_renderer_t));

    renderer->draw_call_count = 0;

    renderer->shader = shader;
    renderer->quad_count = 0;

    neko_vertex_buffer_t* buffer = neko_new_vertex_buffer((neko_vertex_buffer_flags_t)(NEKO_VERTEXBUFFER_DRAW_TRIANGLES | NEKO_VERTEXBUFFER_DYNAMIC_DRAW));

    // renderer->font = font;

    neko_bind_vertex_buffer_for_edit(buffer);
    neko_push_vertices(buffer, NULL, (9 * 4) * ui_renderer_max_quads);
    neko_push_indices(buffer, NULL, 6 * ui_renderer_max_quads);
    neko_configure_vertex_buffer(buffer, 0, 2, 9, 0); /* vec2 position */
    neko_configure_vertex_buffer(buffer, 1, 2, 9, 2); /* vec2 uv */
    neko_configure_vertex_buffer(buffer, 2, 4, 9, 4); /* vec4 color */
    /* Layout 3 specifies a mode for the rendering.
     * 	0 = rectangle
     * 	1 = icon (using atlas)
     * 	2 = text (using supplied font) */
    neko_configure_vertex_buffer(buffer, 3, 1, 9, 8); /* float mode */
    neko_bind_vertex_buffer_for_edit(NULL);

    renderer->vb = buffer;

    return renderer;
}

void neko_free_ui_renderer(engine_ui_renderer_t* renderer) {
    neko_free_vertex_buffer(renderer->vb);

    mem_free(renderer);
}

void engine_ui_renderer_push_quad(engine_ui_renderer_t* renderer, neko_rect_t dst, neko_rect_t src, neko_color_t color, float transparency, u32 mode) {
    neko_bind_vertex_buffer_for_edit(renderer->vb);

    float tx = src.x;
    float ty = src.y;
    float tw = src.w;
    float th = src.h;

    if (mode == 1) {
        tx /= (float)MU_ATLAS_WIDTH;
        ty /= (float)MU_ATLAS_HEIGHT;
        tw /= (float)MU_ATLAS_WIDTH;
        th /= (float)MU_ATLAS_HEIGHT;
    } else if (mode == 2) {
        // neko_texture_t* atlas = neko_get_glyph_set(renderer->font, 'a')->atlas;

        // tx /= (float)atlas->width;
        // ty /= (float)atlas->height;
        // tw /= (float)atlas->width;
        // th /= (float)atlas->height;
    }

    neko_rgb_color_t col = neko_rgb_color_from_color(color);

    float verts[] = {dst.x, dst.y,        tx,          ty,    col.r,         col.g,       col.b,         transparency,  (float)mode, dst.x + dst.w, dst.y,        tx + tw,
                     ty,    col.r,        col.g,       col.b, transparency,  (float)mode, dst.x + dst.w, dst.y + dst.h, tx + tw,     ty + th,       col.r,        col.g,
                     col.b, transparency, (float)mode, dst.x, dst.y + dst.h, tx,          ty + th,       col.r,         col.g,       col.b,         transparency, (float)mode};

    const u32 index_offset = renderer->quad_count * 4;

    u32 indices[] = {index_offset + 3, index_offset + 2, index_offset + 1, index_offset + 3, index_offset + 1, index_offset + 0};

    neko_update_vertices(renderer->vb, verts, renderer->quad_count * 9 * 4, 9 * 4);
    neko_update_indices(renderer->vb, indices, renderer->quad_count * 6, 6);

    neko_bind_vertex_buffer_for_edit(NULL);

    renderer->quad_count++;

    if (renderer->quad_count >= ui_renderer_max_quads) {
        neko_flush_ui_renderer(renderer);
    }
}

void neko_begin_ui_renderer(engine_ui_renderer_t* renderer, u32 width, u32 height) {
    renderer->backup.blend = glIsEnabled(GL_BLEND);
    renderer->backup.cull_face = glIsEnabled(GL_CULL_FACE);
    renderer->backup.depth_test = glIsEnabled(GL_DEPTH_TEST);
    renderer->backup.scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    renderer->quad_count = 0;
    renderer->draw_call_count = 0;

    renderer->width = width;
    renderer->height = height;
    renderer->camera = neko_m4f_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

    glScissor(0.0f, 0.0f, (float)width, (float)height);
    glViewport(0, 0, width, height);
}

void neko_end_ui_renderer(engine_ui_renderer_t* renderer) {
    neko_flush_ui_renderer(renderer);

    if (renderer->backup.blend) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    if (renderer->backup.cull_face) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    if (renderer->backup.depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    if (renderer->backup.scissor_test) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void neko_flush_ui_renderer(engine_ui_renderer_t* renderer) {
    neko_bind_vertex_buffer_for_edit(NULL);
    neko_bind_shader(renderer->shader);

    neko_bind_texture(renderer->icon_texture, 0);
    neko_shader_set_int(renderer->shader, "atlas", 0);

    /* TODO: bind the appropriate atlases, not just one. */
    // neko_bind_texture(neko_get_glyph_set(renderer->font, 'a')->atlas, 1);
    // neko_shader_set_int(renderer->shader, "font", 1);

    neko_shader_set_m4f(renderer->shader, "camera", renderer->camera);

    neko_bind_vertex_buffer_for_draw(renderer->vb);
    neko_draw_vertex_buffer_custom_count(renderer->vb, renderer->quad_count * 6);
    neko_bind_vertex_buffer_for_draw(NULL);

    neko_bind_texture(NULL, 0);
    neko_bind_shader(NULL);

    renderer->quad_count = 0;
    renderer->draw_call_count++;
}

float engine_ui_renderer_draw_text(engine_ui_renderer_t* renderer, const char* text, neko_v2f_t position, neko_color_t color, float transparency) {
    const char* p = text;
    u32 code_point;
    // while (*p) {
    // p = neko_utf8_to_codepoint(p, &code_point);
    // neko_glyph_set_t* set = neko_get_glyph_set(renderer->font, code_point);
    // stbtt_bakedchar* g = &set->glyphs[code_point & 0xff];

    // neko_rect_t src = {(float)g->x0, (float)g->y0, (float)g->x1 - g->x0, (float)g->y1 - g->y0};

    // neko_rect_t dst = {position.x + g->xoff, position.y + g->yoff, src.w, src.h};

    // engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 2);
    // position.x += g->xadvance;
    // }
    return position.x;
}

void engine_ui_renderer_draw_rect(engine_ui_renderer_t* renderer, neko_rect_t rect, neko_color_t color, float transparency) {
    neko_rect_t dst = {rect.x, rect.y, rect.w, rect.h};

    mu_Rect mu_src = mu_atlas_lookup(MU_ATLAS_WHITE);

    neko_rect_t src = {(float)mu_src.x, (float)mu_src.y, (float)mu_src.w, (float)mu_src.h};

    engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 0);
}

void engine_ui_renderer_draw_icon(engine_ui_renderer_t* renderer, u32 id, neko_rect_t rect, neko_color_t color, float transparency) {
    neko_rect_t src = renderer->icon_rects[id];

    i32 x = rect.x + (rect.w - src.w) / 2;
    i32 y = rect.y + (rect.h - src.h) / 2;

    engine_ui_renderer_push_quad(renderer, neko_rect_t{(float)x, (float)y, src.w, src.h}, src, color, transparency, 1);
}

float engine_ui_text_width(engine_ui_renderer_t* renderer, const char* text) {
    i32 x = 0;
    const char* p = text;
    u32 codepoint;
    // while (*p) {
    // p = neko_utf8_to_codepoint(p, &codepoint);
    // neko_glyph_set_t* set = neko_get_glyph_set(renderer->font, codepoint);
    // stbtt_bakedchar* g = &set->glyphs[codepoint & 0xff];
    // x += g->xadvance;
    // }
    return (float)x;
}

float engine_ui_tect_height(engine_ui_renderer_t* renderer) {
    // return renderer->font->height;
    return 22.0f;
}

void neko_set_ui_renderer_clip(engine_ui_renderer_t* renderer, neko_rect_t rect) {
    neko_flush_ui_renderer(renderer);
    glScissor(rect.x, renderer->height - (rect.y + rect.h), rect.w, rect.h);
}

INPUT_WRAP_DEFINE(ui);

void neko_init_ui_renderer(u32 shader_id) {
    mu_renderer = neko_new_ui_renderer(shader_id);

    mu_renderer->icon_texture = neko_new_texture_from_memory_uncompressed(mu_atlas_texture, sizeof(mu_atlas_texture), MU_ATLAS_WIDTH, MU_ATLAS_HEIGHT, 1, NEKO_TEXTURE_ANTIALIASED);

    for (u32 i = MU_ICON_CLOSE; i <= MU_ICON_MAX; i++) {
        mu_Rect rect = mu_atlas_lookup(i);

        mu_renderer->icon_rects[i] = neko_rect_t{(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};
    }

#if 1
    input_add_key_down_callback(ui_key_down);
    input_add_key_up_callback(ui_key_up);
    input_add_char_down_callback(ui_char_down);
    input_add_mouse_down_callback(ui_mouse_down);
    input_add_mouse_up_callback(ui_mouse_up);
    input_add_mouse_move_callback(ui_mouse_move);
    input_add_scroll_callback(ui_scroll);
#endif
}

void neko_deinit_ui_renderer() {
    neko_free_texture(mu_renderer->icon_texture);

    neko_free_ui_renderer(mu_renderer);
}

static void neko_ui_renderer_push_quad(neko_rect_t dst, neko_rect_t src, mu_Color color, u32 mode) {
    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_push_quad(mu_renderer, dst, src, c, (float)color.a / 255.0f, mode);
}

static void neko_flush_ui_renderer() { neko_flush_ui_renderer(mu_renderer); }

static void neko_begin_ui_render(u32 width, u32 height) { neko_begin_ui_renderer(mu_renderer, width, height); }

static void neko_end_ui_render() { neko_end_ui_renderer(mu_renderer); }

static void neko_set_ui_renderer_clip(mu_Rect rect) {
    neko_rect_t r = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};

    neko_set_ui_renderer_clip(mu_renderer, r);
}

static void neko_render_ui_text(const char* text, mu_Vec2 pos, mu_Color color) {
    neko_v2f_t p = {(float)pos.x, (float)pos.y};

    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_draw_text(mu_renderer, text, p, c, (float)color.a / 255.0f);
}

static void neko_render_ui_rect(mu_Rect rect, mu_Color color) {
    neko_rect_t r = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};

    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_draw_rect(mu_renderer, r, c, (float)color.a / 255.0f);
}

static void neko_render_ui_icon(i32 id, mu_Rect rect, mu_Color color) {
    neko_rect_t r = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};

    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_draw_icon(mu_renderer, id, r, c, (float)color.a / 255.0f);
}

i32 neko_ui_text_width(mu_Font font, const char* text, i32 len) { return (i32)engine_ui_text_width(mu_renderer, text); }

i32 neko_ui_text_height(mu_Font font) { return (i32)engine_ui_tect_height(mu_renderer); }

void neko_update_ui(mu_Context* context) {
    assert(context);

    auto mouse_pos_tmp = input_get_mouse_pos_pixels_fix();
    vec2 mouse_pos = {mouse_pos_tmp.x, mouse_pos_tmp.y};

    for (INPUT_WRAP_event e; input_wrap_next_e(&ui_input_queue, &e); input_wrap_free_e(&e))
        if (e.type) switch (e.type) {
                default:
                    break;
                case INPUT_WRAP_KEY_PRESSED:
                case INPUT_WRAP_KEY_REPEATED:
                    mu_input_keydown(context, e.keyboard.key);
                    break;
                case INPUT_WRAP_KEY_RELEASED:
                    mu_input_keyup(context, e.keyboard.key);
                    break;
                case INPUT_WRAP_CODEPOINT_INPUT: {
                    char txt[2] = {(char)(e.codepoint & 255), 0};
                    mu_input_text(context, txt);
                } break;
                case INPUT_WRAP_BUTTON_PRESSED: {
                    i32 code = 1 << e.mouse.button;
                    mu_input_mousedown(context, (i32)mouse_pos.x, (i32)(-mouse_pos.y), code);

                } break;
                case INPUT_WRAP_BUTTON_RELEASED: {
                    i32 code = 1 << e.mouse.button;
                    mu_input_mouseup(context, (i32)mouse_pos.x, (i32)(-mouse_pos.y), code);

                } break;
                case INPUT_WRAP_SCROLLED:
                    mu_input_scroll(context, (i32)(-e.scroll.x * 30.f), (i32)(-e.scroll.y * 30.f));
                    break;
                case INPUT_WRAP_CURSOR_MOVED: {
                    mu_input_mousemove(context, mouse_pos.x, mouse_pos.y);
                } break;
            }

#if 0


    if (neko_scrolled()) {
        mu_input_scroll(context, 0, neko_get_scroll_offset().y * -30.0f);
    }

    const char* text_input = neko_get_text_input();
    if (*text_input) {
        mu_input_text(context, text_input);
    }

    /* This is a really bad way of doing this.
     * TODO: figure out a better way. */
    if (neko_mouse_button_just_pressed(NEKO_MOUSE_BUTTON_LEFT)) {
        mu_input_mousedown(context, mouse_position.x, mouse_position.y, MU_MOUSE_LEFT);
    } else if (neko_mouse_button_just_released(NEKO_MOUSE_BUTTON_LEFT)) {
        mu_input_mouseup(context, mouse_position.x, mouse_position.y, MU_MOUSE_LEFT);
    }

    if (neko_mouse_button_just_pressed(NEKO_MOUSE_BUTTON_RIGHT)) {
        mu_input_mousedown(context, mouse_position.x, mouse_position.y, MU_MOUSE_RIGHT);
    } else if (neko_mouse_button_just_released(NEKO_MOUSE_BUTTON_RIGHT)) {
        mu_input_mouseup(context, mouse_position.x, mouse_position.y, MU_MOUSE_RIGHT);
    }

    if (neko_mouse_button_just_pressed(NEKO_MOUSE_BUTTON_MIDDLE)) {
        mu_input_mousedown(context, mouse_position.x, mouse_position.y, MU_MOUSE_MIDDLE);
    } else if (neko_mouse_button_just_released(NEKO_MOUSE_BUTTON_MIDDLE)) {
        mu_input_mouseup(context, mouse_position.x, mouse_position.y, MU_MOUSE_MIDDLE);
    }

    if (neko_key_just_pressed(NEKO_KEY_LEFT_CONTROL) || neko_key_just_pressed(NEKO_KEY_RIGHT_CONTROL)) {
        mu_input_keydown(context, MU_KEY_CTRL);
    } else if (neko_key_just_released(NEKO_KEY_LEFT_CONTROL) || neko_key_just_released(NEKO_KEY_RIGHT_CONTROL)) {
        mu_input_keyup(context, MU_KEY_CTRL);
    }

    if (neko_key_just_pressed(NEKO_KEY_LEFT_SHIFT) || neko_key_just_pressed(NEKO_KEY_RIGHT_SHIFT)) {
        mu_input_keydown(context, MU_KEY_SHIFT);
    } else if (neko_key_just_released(NEKO_KEY_LEFT_SHIFT) || neko_key_just_released(NEKO_KEY_RIGHT_SHIFT)) {
        mu_input_keyup(context, MU_KEY_SHIFT);
    }

    if (neko_key_just_pressed(NEKO_KEY_LEFT_ALT) || neko_key_just_pressed(NEKO_KEY_RIGHT_ALT)) {
        mu_input_keydown(context, MU_KEY_ALT);
    } else if (neko_key_just_released(NEKO_KEY_LEFT_ALT) || neko_key_just_released(NEKO_KEY_RIGHT_ALT)) {
        mu_input_keyup(context, MU_KEY_ALT);
    }

    if (neko_key_just_pressed(NEKO_KEY_ENTER)) {
        mu_input_keydown(context, MU_KEY_RETURN);
    } else if (neko_key_just_released(NEKO_KEY_ENTER)) {
        mu_input_keyup(context, MU_KEY_RETURN);
    }

    if (neko_key_just_pressed(NEKO_KEY_BACKSPACE)) {
        mu_input_keydown(context, MU_KEY_BACKSPACE);
    } else if (neko_key_just_released(NEKO_KEY_BACKSPACE)) {
        mu_input_keyup(context, MU_KEY_BACKSPACE);
    }

#endif
}

void neko_render_ui(mu_Context* context, u32 width, u32 height) {
    assert(context);

    neko_begin_ui_render(width, height);

    mu_Command* cmd = NULL;
    while (mu_next_command(context, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT:
                neko_render_ui_text(cmd->text.str, cmd->text.pos, cmd->text.color);
                break;
            case MU_COMMAND_RECT:
                neko_render_ui_rect(cmd->rect.rect, cmd->rect.color);
                break;
            case MU_COMMAND_ICON:
                neko_render_ui_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
                break;
            case MU_COMMAND_CLIP:
                neko_set_ui_renderer_clip(cmd->clip.rect);
                break;
        }
    }

    neko_end_ui_render();
}

engine_ui_renderer_t* neko_get_ui_renderer() { return mu_renderer; }
