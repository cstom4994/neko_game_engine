#include "engine/ui.h"

#include <assert.h>
#include <stb_truetype.h>
#include <stdlib.h>
#include <string.h>

#include "engine/font.h"
#include "engine/glew_glfw.h"
#include "engine/input.h"
#include "vendor/atlas.h"

const u32 ui_renderer_max_quads = 1024;

static engine_ui_renderer_t* ui_renderer;

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

    renderer->font = neko_default_font();

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

void engine_ui_renderer_push_quad(engine_ui_renderer_t* renderer, rect_t dst, rect_t src, neko_color_t color, float transparency, u32 mode) {
    neko_bind_vertex_buffer_for_edit(renderer->vb);

    float tx = src.x;
    float ty = src.y;
    float tw = src.w;
    float th = src.h;

    if (mode == 1) {
        tx /= (float)UI_ATLAS_WIDTH;
        ty /= (float)UI_ATLAS_HEIGHT;
        tw /= (float)UI_ATLAS_WIDTH;
        th /= (float)UI_ATLAS_HEIGHT;
    } else if (mode == 2) {
        // neko_texture_t* atlas = neko_get_glyph_set(renderer->font, 'a')->atlas;

        // tx *= (float)256;
        // ty *= (float)128;
        // tw *= (float)256;
        // th *= (float)128;
    }

    neko_rgb_color_t col = neko_rgb_color_from_color(color);

    float verts[] = {dst.x,         dst.y,         tx,      ty,      col.r, col.g, col.b, transparency, (float)mode,   //
                     dst.x + dst.w, dst.y,         tx + tw, ty,      col.r, col.g, col.b, transparency, (float)mode,   //
                     dst.x + dst.w, dst.y + dst.h, tx + tw, ty + th, col.r, col.g, col.b, transparency, (float)mode,   //
                     dst.x,         dst.y + dst.h, tx,      ty + th, col.r, col.g, col.b, transparency, (float)mode};  //

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
    renderer->camera = mat4_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

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
    neko_shader_set_int(renderer->shader, "font", 1);

    neko_shader_set_m4f(renderer->shader, "camera", renderer->camera);

    neko_bind_vertex_buffer_for_draw(renderer->vb);
    neko_draw_vertex_buffer_custom_count(renderer->vb, renderer->quad_count * 6);
    neko_bind_vertex_buffer_for_draw(NULL);

    neko_bind_texture(NULL, 0);
    neko_bind_shader(NULL);

    renderer->quad_count = 0;
    renderer->draw_call_count++;
}

float engine_ui_renderer_draw_text(engine_ui_renderer_t* renderer, const char* text, vec2 position, neko_color_t color, float transparency) {
    // const char* p = text;
    // u32 code_point;
    // while (*p) {
    //     p = neko_utf8_to_codepoint(p, &code_point);
    //     neko_glyph_set_t* set = neko_get_glyph_set(renderer->font, code_point);
    //     stbtt_bakedchar* g = &set->glyphs[code_point & 0xff];
    //     rect_t src = {(float)g->x0, (float)g->y0, (float)g->x1 - g->x0, (float)g->y1 - g->y0};
    //     rect_t dst = {position.x + g->xoff, position.y + g->yoff, src.w, src.h};
    //     engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 2);
    //     position.x += g->xadvance;
    // }

    float x = position.x;
    float y = position.y;

    for (Rune r : UTF8(text)) {

        u32 tex_id = 0;
        float xx = x;
        float yy = y;
        stbtt_aligned_quad q = renderer->font->quad(&tex_id, &xx, &yy, 16.f, r.charcode());

        neko_gl_data_t* ogl = gfx_ogl();
        GLuint gl_tex_id = neko_slot_array_get(ogl->textures, tex_id).id;

        float x1 = x + q.x0;
        float y1 = y + q.y0;
        float x2 = x + q.x1;
        float y2 = y + q.y1;

        float u1 = q.s0;
        float v1 = q.t0;
        float u2 = q.s1;
        float v2 = q.t1;

        float w = q.x1 - q.x0;
        float h = q.y1 - q.y0;

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gl_tex_id);

        rect_t src = {(float)q.s0, (float)q.t0, (float)q.s1 - q.s0, (float)q.t1 - q.t0};
        rect_t dst = {x1, y1, w, h};
        engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 2);

        x = xx;
        y = yy;

        position.x += w;
    }

    return position.x;
}

void engine_ui_renderer_draw_rect(engine_ui_renderer_t* renderer, rect_t rect, neko_color_t color, float transparency) {
    rect_t dst = {rect.x, rect.y, rect.w, rect.h};

    rect_t ui_src = ui_atlas_lookup(UI_ATLAS_WHITE);

    rect_t src = {(float)ui_src.x, (float)ui_src.y, (float)ui_src.w, (float)ui_src.h};

    engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 0);
}

void engine_ui_renderer_draw_icon(engine_ui_renderer_t* renderer, u32 id, rect_t rect, neko_color_t color, float transparency) {
    rect_t src = renderer->icon_rects[id];

    i32 x = rect.x + (rect.w - src.w) / 2;
    i32 y = rect.y + (rect.h - src.h) / 2;

    engine_ui_renderer_push_quad(renderer, rect_t{(float)x, (float)y, src.w, src.h}, src, color, transparency, 1);
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
    int len = strlen(text);
    return (float)len * 6.f;
}

float engine_ui_tect_height(engine_ui_renderer_t* renderer) {
    // return renderer->font->height;
    return -2.0f;
}

void neko_set_ui_renderer_clip(engine_ui_renderer_t* renderer, rect_t rect) {
    neko_flush_ui_renderer(renderer);
    glScissor(rect.x, renderer->height - (rect.y + rect.h), rect.w, rect.h);
}

INPUT_WRAP_DEFINE(ui);

void neko_init_ui_renderer(u32 shader_id) {
    ui_renderer = neko_new_ui_renderer(shader_id);

    ui_renderer->icon_texture = neko_new_texture_from_memory_uncompressed(ui_atlas_texture, sizeof(ui_atlas_texture), UI_ATLAS_WIDTH, UI_ATLAS_HEIGHT, 1, NEKO_TEXTURE_ANTIALIASED);

    for (u32 i = UI_ICON_CLOSE; i <= UI_ICON_MAX; i++) {
        rect_t rect = ui_atlas_lookup(i);

        ui_renderer->icon_rects[i] = rect_t{(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};
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
    neko_free_texture(ui_renderer->icon_texture);

    neko_free_ui_renderer(ui_renderer);
}

static void neko_ui_renderer_push_quad(rect_t dst, rect_t src, Color256 color, u32 mode) {
    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_push_quad(ui_renderer, dst, src, c, (float)color.a / 255.0f, mode);
}

static void neko_flush_ui_renderer() { neko_flush_ui_renderer(ui_renderer); }

static void neko_begin_ui_render(u32 width, u32 height) { neko_begin_ui_renderer(ui_renderer, width, height); }

static void neko_end_ui_render() { neko_end_ui_renderer(ui_renderer); }

static void neko_set_ui_renderer_clip(rect_t rect) {
    rect_t r = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};

    neko_set_ui_renderer_clip(ui_renderer, r);
}

static void neko_render_ui_text(const char* text, vec2 pos, Color256 color) {
    vec2 p = {(float)pos.x, (float)pos.y};

    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_draw_text(ui_renderer, text, p, c, (float)color.a / 255.0f);
}

static void neko_render_ui_rect(rect_t rect, Color256 color) {
    rect_t r = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};

    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_draw_rect(ui_renderer, r, c, (float)color.a / 255.0f);
}

static void neko_render_ui_icon(i32 id, rect_t rect, Color256 color) {
    rect_t r = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};

    neko_rgb_color_t rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f};

    neko_color_t c = neko_color_from_rgb_color(rgb);

    engine_ui_renderer_draw_icon(ui_renderer, id, r, c, (float)color.a / 255.0f);
}

i32 neko_ui_text_width(ui_font font, const char* text, i32 len) { return (i32)engine_ui_text_width(ui_renderer, text); }

i32 neko_ui_text_height(ui_font font) { return (i32)engine_ui_tect_height(ui_renderer); }

void neko_update_ui(ui_context_t* context) {
    assert(context);

    auto mouse_pos_tmp = input_get_mouse_pos_pixels_fix();
    vec2 mouse_pos = {mouse_pos_tmp.x, mouse_pos_tmp.y};

    for (INPUT_WRAP_event e; input_wrap_next_e(&ui_input_queue, &e); input_wrap_free_e(&e))
        if (e.type) switch (e.type) {
                default:
                    break;
                case INPUT_WRAP_KEY_PRESSED:
                case INPUT_WRAP_KEY_REPEATED:
                    ui_input_keydown(context, e.keyboard.key);
                    break;
                case INPUT_WRAP_KEY_RELEASED:
                    ui_input_keyup(context, e.keyboard.key);
                    break;
                case INPUT_WRAP_CODEPOINT_INPUT: {
                    char txt[2] = {(char)(e.codepoint & 255), 0};
                    ui_input_text(context, txt);
                } break;
                case INPUT_WRAP_BUTTON_PRESSED: {
                    i32 code = 1 << e.mouse.button;
                    ui_input_mousedown(context, (i32)mouse_pos.x, (i32)(mouse_pos.y), code);

                } break;
                case INPUT_WRAP_BUTTON_RELEASED: {
                    i32 code = 1 << e.mouse.button;
                    ui_input_mouseup(context, (i32)mouse_pos.x, (i32)(mouse_pos.y), code);

                } break;
                case INPUT_WRAP_SCROLLED:
                    ui_input_scroll(context, (i32)(-e.scroll.x * 30.f), (i32)(-e.scroll.y * 30.f));
                    break;
                case INPUT_WRAP_CURSOR_MOVED: {
                    ui_input_mousemove(context, mouse_pos.x, mouse_pos.y);
                } break;
            }

#if 0


    if (neko_scrolled()) {
        ui_input_scroll(context, 0, neko_get_scroll_offset().y * -30.0f);
    }

    const char* text_input = neko_get_text_input();
    if (*text_input) {
        ui_input_text(context, text_input);
    }

    /* This is a really bad way of doing this.
     * TODO: figure out a better way. */
    if (neko_mouse_button_just_pressed(NEKO_MOUSE_BUTTON_LEFT)) {
        ui_input_mousedown(context, mouse_position.x, mouse_position.y, UI_MOUSE_LEFT);
    } else if (neko_mouse_button_just_released(NEKO_MOUSE_BUTTON_LEFT)) {
        ui_input_mouseup(context, mouse_position.x, mouse_position.y, UI_MOUSE_LEFT);
    }

    if (neko_mouse_button_just_pressed(NEKO_MOUSE_BUTTON_RIGHT)) {
        ui_input_mousedown(context, mouse_position.x, mouse_position.y, UI_MOUSE_RIGHT);
    } else if (neko_mouse_button_just_released(NEKO_MOUSE_BUTTON_RIGHT)) {
        ui_input_mouseup(context, mouse_position.x, mouse_position.y, UI_MOUSE_RIGHT);
    }

    if (neko_mouse_button_just_pressed(NEKO_MOUSE_BUTTON_MIDDLE)) {
        ui_input_mousedown(context, mouse_position.x, mouse_position.y, UI_MOUSE_MIDDLE);
    } else if (neko_mouse_button_just_released(NEKO_MOUSE_BUTTON_MIDDLE)) {
        ui_input_mouseup(context, mouse_position.x, mouse_position.y, UI_MOUSE_MIDDLE);
    }

    if (neko_key_just_pressed(NEKO_KEY_LEFT_CONTROL) || neko_key_just_pressed(NEKO_KEY_RIGHT_CONTROL)) {
        ui_input_keydown(context, UI_KEY_CTRL);
    } else if (neko_key_just_released(NEKO_KEY_LEFT_CONTROL) || neko_key_just_released(NEKO_KEY_RIGHT_CONTROL)) {
        ui_input_keyup(context, UI_KEY_CTRL);
    }

    if (neko_key_just_pressed(NEKO_KEY_LEFT_SHIFT) || neko_key_just_pressed(NEKO_KEY_RIGHT_SHIFT)) {
        ui_input_keydown(context, UI_KEY_SHIFT);
    } else if (neko_key_just_released(NEKO_KEY_LEFT_SHIFT) || neko_key_just_released(NEKO_KEY_RIGHT_SHIFT)) {
        ui_input_keyup(context, UI_KEY_SHIFT);
    }

    if (neko_key_just_pressed(NEKO_KEY_LEFT_ALT) || neko_key_just_pressed(NEKO_KEY_RIGHT_ALT)) {
        ui_input_keydown(context, UI_KEY_ALT);
    } else if (neko_key_just_released(NEKO_KEY_LEFT_ALT) || neko_key_just_released(NEKO_KEY_RIGHT_ALT)) {
        ui_input_keyup(context, UI_KEY_ALT);
    }

    if (neko_key_just_pressed(NEKO_KEY_ENTER)) {
        ui_input_keydown(context, UI_KEY_RETURN);
    } else if (neko_key_just_released(NEKO_KEY_ENTER)) {
        ui_input_keyup(context, UI_KEY_RETURN);
    }

    if (neko_key_just_pressed(NEKO_KEY_BACKSPACE)) {
        ui_input_keydown(context, UI_KEY_BACKSPACE);
    } else if (neko_key_just_released(NEKO_KEY_BACKSPACE)) {
        ui_input_keyup(context, UI_KEY_BACKSPACE);
    }

#endif
}

void neko_render_ui(ui_context_t* context, u32 width, u32 height) {
    assert(context);

    neko_begin_ui_render(width, height);

    ui_Command* cmd = NULL;
    while (ui_next_command(context, &cmd)) {
        switch (cmd->type) {
            case UI_COMMAND_TEXT:
                neko_render_ui_text(cmd->text.str, cmd->text.pos, cmd->text.color);
                break;
            case UI_COMMAND_RECT:
                neko_render_ui_rect(cmd->rect.rect, cmd->rect.color);
                break;
            case UI_COMMAND_ICON:
                neko_render_ui_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
                break;
            case UI_COMMAND_CLIP:
                neko_set_ui_renderer_clip(cmd->clip.rect);
                break;
        }
    }

    neko_end_ui_render();
}

engine_ui_renderer_t* neko_get_ui_renderer() { return ui_renderer; }
