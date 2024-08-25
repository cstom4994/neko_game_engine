#include "engine/ui.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/edit.h"
#include "engine/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/luax.hpp"
#include "vendor/atlas.h"

// deps
#include <stb_truetype.h>

const u32 ui_renderer_max_quads = 1024;

static engine_ui_renderer_t *ui_renderer;

static const char *neko_utf8_to_codepoint(const char *p, u32 *dst) {
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

engine_ui_renderer_t *neko_new_ui_renderer(u32 shader) {
    engine_ui_renderer_t *renderer = (engine_ui_renderer_t *)mem_alloc(sizeof(engine_ui_renderer_t));

    renderer->draw_call_count = 0;

    renderer->shader = shader;
    renderer->quad_count = 0;

    neko_vertex_buffer_t *buffer = neko_new_vertex_buffer((neko_vertex_buffer_flags_t)(NEKO_VERTEXBUFFER_DRAW_TRIANGLES | NEKO_VERTEXBUFFER_DYNAMIC_DRAW));

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

void neko_free_ui_renderer(engine_ui_renderer_t *renderer) {
    neko_free_vertex_buffer(renderer->vb);

    mem_free(renderer);
}

void engine_ui_renderer_push_quad(engine_ui_renderer_t *renderer, rect_t dst, rect_t src, neko_color_t color, float transparency, u32 mode) {
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

    if (/*mode == 2 ||*/ renderer->quad_count >= ui_renderer_max_quads) {
        neko_flush_ui_renderer(renderer);
    }
}

void neko_begin_ui_renderer(engine_ui_renderer_t *renderer, u32 width, u32 height) {
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

void neko_end_ui_renderer(engine_ui_renderer_t *renderer) {
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

void neko_flush_ui_renderer(engine_ui_renderer_t *renderer) {
    neko_bind_vertex_buffer_for_edit(NULL);
    neko_bind_shader(renderer->shader);

    neko_bind_texture(renderer->icon_texture, 0);
    neko_shader_set_int(renderer->shader, "atlas", 0);

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

float engine_ui_renderer_draw_text(engine_ui_renderer_t *renderer, const char *text, vec2 position, neko_color_t color, float transparency) {
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

        neko_gl_data_t *ogl = gfx_ogl();
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

void engine_ui_renderer_draw_rect(engine_ui_renderer_t *renderer, rect_t rect, neko_color_t color, float transparency) {
    rect_t dst = {rect.x, rect.y, rect.w, rect.h};

    rect_t ui_src = ui_atlas_lookup(UI_ATLAS_WHITE);

    rect_t src = {(float)ui_src.x, (float)ui_src.y, (float)ui_src.w, (float)ui_src.h};

    engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 0);
}

void engine_ui_renderer_draw_icon(engine_ui_renderer_t *renderer, u32 id, rect_t rect, neko_color_t color, float transparency) {
    rect_t src = renderer->icon_rects[id];

    i32 x = rect.x + (rect.w - src.w) / 2;
    i32 y = rect.y + (rect.h - src.h) / 2;

    engine_ui_renderer_push_quad(renderer, rect_t{(float)x, (float)y, src.w, src.h}, src, color, transparency, 1);
}

float engine_ui_text_width(engine_ui_renderer_t *renderer, const char *text) {
    i32 x = 0;
    const char *p = text;
    u32 codepoint;
    // while (*p) {
    // p = neko_utf8_to_codepoint(p, &codepoint);
    // neko_glyph_set_t* set = neko_get_glyph_set(renderer->font, codepoint);
    // stbtt_bakedchar* g = &set->glyphs[codepoint & 0xff];
    // x += g->xadvance;
    // }
    int len = neko_strlen(text);
    return (float)len * 15.f;
}

float engine_ui_tect_height(engine_ui_renderer_t *renderer) {
    // return renderer->font->height;
    return -2.0f;
}

void neko_set_ui_renderer_clip(engine_ui_renderer_t *renderer, rect_t rect) {
    neko_flush_ui_renderer(renderer);
    glScissor(rect.x, renderer->height - (rect.y + rect.h), rect.w, rect.h);
}

INPUT_WRAP_DEFINE(ui);

ui_context_t *ui_global_ctx() { return g_app->ui; }

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

static void neko_render_ui_text(const char *text, vec2 pos, Color256 color) {
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

i32 neko_ui_text_width(ui_font font, const char *text, i32 len) { return (i32)engine_ui_text_width(ui_renderer, text); }

i32 neko_ui_text_height(ui_font font) { return (i32)engine_ui_tect_height(ui_renderer); }

void neko_update_ui(ui_context_t *context) {
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
                    break;
                }
                case INPUT_WRAP_BUTTON_PRESSED: {
                    i32 code = 1 << e.mouse.button;
                    ui_input_mousedown(context, (i32)mouse_pos.x, (i32)(mouse_pos.y), code);
                    break;
                }
                case INPUT_WRAP_BUTTON_RELEASED: {
                    i32 code = 1 << e.mouse.button;
                    ui_input_mouseup(context, (i32)mouse_pos.x, (i32)(mouse_pos.y), code);
                    break;
                }
                case INPUT_WRAP_SCROLLED:
                    ui_input_scroll(context, (i32)(-e.scroll.x * 30.f), (i32)(-e.scroll.y * 30.f));
                    break;
                case INPUT_WRAP_CURSOR_MOVED: {
                    ui_input_mousemove(context, mouse_pos.x, mouse_pos.y);
                    break;
                }
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

void neko_render_ui(ui_context_t *context, u32 width, u32 height) {
    assert(context);

    neko_begin_ui_render(width, height);

    ui_Command *cmd = NULL;
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

engine_ui_renderer_t *neko_get_ui_renderer() { return ui_renderer; }

rect_t lua_ui_check_rect(lua_State *L, i32 arg) {
    rect_t rect = {};
    rect.x = luax_number_field(L, arg, "x");
    rect.y = luax_number_field(L, arg, "y");
    rect.w = luax_number_field(L, arg, "w");
    rect.h = luax_number_field(L, arg, "h");
    return rect;
}

void lua_ui_rect_push(lua_State *L, rect_t rect) {
    lua_createtable(L, 0, 4);
    luax_set_number_field(L, "x", rect.x);
    luax_set_number_field(L, "y", rect.y);
    luax_set_number_field(L, "w", rect.w);
    luax_set_number_field(L, "h", rect.h);
}

Color256 lua_ui_check_color(lua_State *L, i32 arg) {
    Color256 color = {};
    color.r = luax_number_field(L, arg, "r");
    color.g = luax_number_field(L, arg, "g");
    color.b = luax_number_field(L, arg, "b");
    color.a = luax_number_field(L, arg, "a");
    return color;
}

// mt_ui_container

static int mt_ui_container_rect(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_ui_rect_push(L, container->rect);
    return 1;
}

static int mt_ui_container_set_rect(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    container->rect = lua_ui_check_rect(L, 2);
    return 0;
}

static int mt_ui_container_body(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_ui_rect_push(L, container->body);
    return 1;
}

static int mt_ui_container_content_size(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushinteger(L, container->content_size.x);
    lua_pushinteger(L, container->content_size.y);
    return 2;
}

static int mt_ui_container_scroll(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushinteger(L, container->scroll.x);
    lua_pushinteger(L, container->scroll.y);
    return 2;
}

static int mt_ui_container_set_scroll(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    container->scroll.x = luaL_checknumber(L, 2);
    container->scroll.y = luaL_checknumber(L, 3);
    return 0;
}

static int mt_ui_container_zindex(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushinteger(L, container->zindex);
    return 1;
}

static int mt_ui_container_open(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushboolean(L, container->open);
    return 1;
}

int open_mt_ui_container(lua_State *L) {
    luaL_Reg reg[] = {
            {"rect", mt_ui_container_rect},
            {"set_rect", mt_ui_container_set_rect},
            {"body", mt_ui_container_body},
            {"content_size", mt_ui_container_content_size},
            {"scroll", mt_ui_container_scroll},
            {"set_scroll", mt_ui_container_set_scroll},
            {"zindex", mt_ui_container_zindex},
            {"open", mt_ui_container_open},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_ui_container", reg);
    return 0;
}

static int mt_ui_style_size(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    lua_pushinteger(L, style->size.x);
    lua_pushinteger(L, style->size.y);
    return 2;
}

static int mt_ui_style_set_size(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    style->size.x = luaL_checknumber(L, 2);
    style->size.y = luaL_checknumber(L, 3);
    return 0;
}

#define MT_UI_STYLE_GETSET(name)                                                  \
    static int mt_ui_style_##name(lua_State *L) {                                 \
        ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style"); \
        lua_pushinteger(L, style->name);                                          \
        return 1;                                                                 \
    }                                                                             \
    static int mt_ui_style_set_##name(lua_State *L) {                             \
        ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style"); \
        style->name = luaL_checknumber(L, 2);                                     \
        return 0;                                                                 \
    }

// MT_UI_STYLE_GETSET(padding);
MT_UI_STYLE_GETSET(spacing);
MT_UI_STYLE_GETSET(indent);
MT_UI_STYLE_GETSET(title_height);
MT_UI_STYLE_GETSET(scrollbar_size);
MT_UI_STYLE_GETSET(thumb_size);

static int mt_ui_style_color(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    lua_Integer colorid = luaL_checkinteger(L, 2);
    if (colorid < 0 || colorid >= UI_COLOR_MAX) {
        return luaL_error(L, "color id out of range");
    }

    lua_createtable(L, 0, 4);
    luax_set_number_field(L, "r", style->colors[colorid].r);
    luax_set_number_field(L, "g", style->colors[colorid].g);
    luax_set_number_field(L, "b", style->colors[colorid].b);
    luax_set_number_field(L, "a", style->colors[colorid].a);
    return 1;
}

static int mt_ui_style_set_color(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    lua_Integer colorid = luaL_checkinteger(L, 2);
    Color256 color = lua_ui_check_color(L, 3);

    if (colorid < 0 || colorid >= UI_COLOR_MAX) {
        return luaL_error(L, "color id out of range");
    }

    style->colors[colorid] = color;
    return 0;
}

int open_mt_ui_style(lua_State *L) {
    luaL_Reg reg[] = {
            {"size", mt_ui_style_size},
            {"set_size", mt_ui_style_set_size},
            // {"padding", mt_ui_style_padding},
            // {"set_padding", mt_ui_style_set_padding},
            {"spacing", mt_ui_style_spacing},
            {"set_spacing", mt_ui_style_set_spacing},
            {"indent", mt_ui_style_indent},
            {"set_indent", mt_ui_style_set_indent},
            {"title_height", mt_ui_style_title_height},
            {"set_title_height", mt_ui_style_set_title_height},
            {"scrollbar_size", mt_ui_style_scrollbar_size},
            {"set_scrollbar_size", mt_ui_style_set_scrollbar_size},
            {"thumb_size", mt_ui_style_thumb_size},
            {"set_thumb_size", mt_ui_style_set_thumb_size},
            {"color", mt_ui_style_color},
            {"set_color", mt_ui_style_set_color},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_ui_style", reg);
    return 0;
}

// mt_ui_ref

static int mt_ui_ref_gc(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_ui_ref");
    mem_free(ref);
    return 0;
}

static int mt_ui_ref_get(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_ui_ref");

    switch (ref->kind) {
        case MUIRefKind_Boolean:
            lua_pushboolean(L, ref->boolean);
            return 1;
        case MUIRefKind_Real:
            lua_pushnumber(L, ref->real);
            return 1;
        case MUIRefKind_String:
            lua_pushstring(L, ref->string);
            return 1;
        case MUIRefKind_Nil:
        default:
            return 0;
    }
}

static int mt_ui_ref_set(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_ui_ref");
    lua_ui_set_ref(L, ref, 2);
    return 0;
}

int open_mt_ui_ref(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_ui_ref_gc},
            {"get", mt_ui_ref_get},
            {"set", mt_ui_ref_set},
            {nullptr, nullptr},
    };
    luax_new_class(L, "mt_ui_ref", reg);
    return 0;
}

// ui api

void lua_ui_set_ref(lua_State *L, MUIRef *ref, i32 arg) {
    i32 type = lua_type(L, arg);
    switch (type) {
        case LUA_TBOOLEAN:
            ref->kind = MUIRefKind_Boolean;
            ref->boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            ref->kind = MUIRefKind_Real;
            ref->real = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            ref->kind = MUIRefKind_String;
            String s = luax_check_string(L, arg);
            if (s.len > array_size(ref->string) - 1) {
                s.len = array_size(ref->string) - 1;
            }
            memcpy(ref->string, s.data, s.len);
            ref->string[s.len] = '\0';
            break;
        }
        default:
            ref->kind = MUIRefKind_Nil;
    }
}

MUIRef *lua_ui_check_ref(lua_State *L, i32 arg, MUIRefKind kind) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, arg, "mt_ui_ref");

    if (ref->kind != kind) {
        memset(ref, 0, sizeof(MUIRef));
        ref->kind = kind;
    }

    return ref;
}

static ui_context_t *ui_ctx() { return g_app->ui; }

static int l_ui_set_focus(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    ui_set_focus(ui_ctx(), (ui_id)id);
    return 0;
}

static int l_ui_get_id(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_id id = ui_get_id(ui_ctx(), name.data, name.len);
    lua_pushinteger(L, (lua_Integer)id);
    return 1;
}

static int l_ui_push_id(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_push_id(ui_ctx(), name.data, name.len);
    return 0;
}

static int l_ui_pop_id(lua_State *L) {
    ui_pop_id(ui_ctx());
    return 0;
}

static int l_ui_push_clip_rect(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);
    ui_push_clip_rect(ui_ctx(), rect);
    return 0;
}

static int l_ui_pop_clip_rect(lua_State *L) {
    ui_pop_clip_rect(ui_ctx());
    return 0;
}

static int l_ui_get_clip_rect(lua_State *L) {
    rect_t rect = ui_get_clip_rect(ui_ctx());
    lua_ui_rect_push(L, rect);
    return 1;
}

static int l_ui_check_clip(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);

    i32 clip = ui_check_clip(ui_ctx(), rect);
    lua_pushinteger(L, clip);
    return 1;
}

static int l_ui_get_current_container(lua_State *L) {
    ui_container_t *container = ui_get_current_container(ui_ctx());
    luax_ptr_userdata(L, container, "mt_ui_container");
    return 1;
}

static int l_ui_get_container(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_container_t *container = ui_get_container(ui_ctx(), name.data);
    luax_ptr_userdata(L, container, "mt_ui_container");
    return 1;
}

static int l_ui_bring_to_front(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    ui_bring_to_front(ui_ctx(), container);
    return 0;
}

static int l_ui_set_clip(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);
    ui_set_clip(ui_ctx(), rect);
    return 0;
}

static int l_ui_draw_rect(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);
    Color256 color = lua_ui_check_color(L, 2);
    ui_draw_rect(ui_ctx(), rect, color);
    return 0;
}

static int l_ui_draw_box(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);
    Color256 color = lua_ui_check_color(L, 2);
    ui_draw_box(ui_ctx(), rect, color);
    return 0;
}

static int l_ui_draw_text(lua_State *L) {
    String str = luax_check_string(L, 1);
    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
    Color256 color = lua_ui_check_color(L, 4);

    vec2 pos = {(f32)x, (f32)y};
    ui_draw_text(ui_ctx(), nullptr, str.data, str.len, pos, color);
    return 0;
}

// static int l_ui_draw_icon(lua_State *L) {
//     lua_Integer id = luaL_checkinteger(L, 1);
//     rect_t rect = lua_ui_check_rect(L, 2);
//     Color256 color = lua_ui_check_color(L, 3);
//     ui_draw_icon(ui_ctx(), id, rect, color);
//     return 0;
// }

static int l_ui_layout_row(lua_State *L) {
    lua_Number height = luaL_checknumber(L, 2);

    i32 widths[UI_MAX_WIDTHS] = {};

    lua_Integer n = luax_len(L, 1);
    if (n > UI_MAX_WIDTHS) {
        n = UI_MAX_WIDTHS;
    }

    for (i32 i = 0; i < n; i++) {
        luax_geti(L, 1, i + 1);
        widths[i] = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    ui_layout_row(ui_ctx(), n, widths, height);
    return 0;
}

static int l_ui_layout_width(lua_State *L) {
    lua_Number width = luaL_checknumber(L, 1);
    ui_layout_width(ui_ctx(), width);
    return 0;
}

static int l_ui_layout_height(lua_State *L) {
    lua_Number height = luaL_checknumber(L, 1);
    ui_layout_height(ui_ctx(), height);
    return 0;
}

static int l_ui_layout_begin_column(lua_State *L) {
    ui_layout_begin_column(ui_ctx());
    return 0;
}

static int l_ui_layout_end_column(lua_State *L) {
    ui_layout_end_column(ui_ctx());
    return 0;
}

static int l_ui_layout_set_next(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);
    bool relative = lua_toboolean(L, 2);
    ui_layout_set_next(ui_ctx(), rect, relative);
    return 0;
}

static int l_ui_layout_next(lua_State *L) {
    rect_t rect = ui_layout_next(ui_ctx());
    lua_ui_rect_push(L, rect);
    return 1;
}

static int l_ui_draw_control_frame(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer colorid = luaL_checkinteger(L, 3);
    lua_Integer opt = luaL_checkinteger(L, 4);
    ui_draw_control_frame(ui_ctx(), id, rect, colorid, opt);
    return 0;
}

static int l_ui_draw_control_text(lua_State *L) {
    String str = luax_check_string(L, 1);
    rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer colorid = luaL_checkinteger(L, 3);
    lua_Integer opt = luaL_checkinteger(L, 4);
    ui_draw_control_text(ui_ctx(), str.data, rect, colorid, opt);
    return 0;
}

static int l_ui_mouse_over(lua_State *L) {
    rect_t rect = lua_ui_check_rect(L, 1);
    int res = ui_mouse_over(ui_ctx(), rect);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_update_control(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer opt = luaL_checkinteger(L, 3);
    ui_update_control(ui_ctx(), id, rect, opt);
    return 0;
}

static int l_ui_text(lua_State *L) {
    String text = luax_check_string(L, 1);
    ui_text(ui_ctx(), text.data);
    return 0;
}

static int l_ui_label(lua_State *L) {
    String text = luax_check_string(L, 1);
    ui_label(ui_ctx(), text.data);
    return 0;
}

static int l_ui_button(lua_State *L) {
    String text = luax_check_string(L, 1);
    lua_Integer icon = luaL_optinteger(L, 2, 0);
    lua_Integer opt = luaL_optinteger(L, 3, UI_OPT_ALIGNCENTER);
    i32 res = ui_button_ex(ui_ctx(), text.data, icon, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_checkbox(lua_State *L) {
    String text = luax_check_string(L, 1);
    MUIRef *ref = lua_ui_check_ref(L, 2, MUIRefKind_Boolean);

    i32 res = ui_checkbox(ui_ctx(), text.data, &ref->boolean);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_textbox_raw(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_String);
    lua_Integer id = luaL_checkinteger(L, 2);
    rect_t rect = lua_ui_check_rect(L, 3);
    i32 opt = luaL_optinteger(L, 4, 0);

    i32 res = ui_textbox_raw(ui_ctx(), ref->string, array_size(ref->string), id, rect, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_textbox(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_String);
    i32 opt = luaL_optinteger(L, 2, 0);

    i32 res = ui_textbox_ex(ui_ctx(), ref->string, array_size(ref->string), opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_slider(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_Real);
    ui_real low = (ui_real)luaL_checknumber(L, 2);
    ui_real high = (ui_real)luaL_checknumber(L, 3);
    ui_real step = (ui_real)luaL_optnumber(L, 4, 0);
    String fmt = luax_opt_string(L, 5, UI_SLIDER_FMT);
    i32 opt = luaL_optinteger(L, 6, UI_OPT_ALIGNCENTER);

    i32 res = ui_slider_ex(ui_ctx(), &ref->real, low, high, step, fmt.data, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_number(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_Real);
    ui_real step = (ui_real)luaL_checknumber(L, 2);
    String fmt = luax_opt_string(L, 3, UI_SLIDER_FMT);
    i32 opt = luaL_optinteger(L, 4, UI_OPT_ALIGNCENTER);

    i32 res = ui_number_ex(ui_ctx(), &ref->real, step, fmt.data, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_header(lua_State *L) {
    String text = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);
    i32 res = ui_header_ex(ui_ctx(), text.data, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_begin_treenode(lua_State *L) {
    String label = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);

    i32 res = ui_begin_treenode_ex(ui_ctx(), label.data, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_end_treenode(lua_State *L) {
    ui_end_treenode(ui_ctx());
    return 0;
}

static int l_ui_begin_window(lua_State *L) {
    String title = luax_check_string(L, 1);
    rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer opt = luaL_optinteger(L, 3, 0x00);

    i32 res = ui_begin_window_ex(ui_ctx(), title.data, rect, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_end_window(lua_State *L) {
    ui_end_window(ui_ctx());
    return 0;
}

static int l_ui_open_popup(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_open_popup(ui_ctx(), name.data);
    return 0;
}

static int l_ui_begin_popup(lua_State *L) {
    String name = luax_check_string(L, 1);
    i32 res = ui_begin_popup(ui_ctx(), name.data);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_end_popup(lua_State *L) {
    ui_end_popup(ui_ctx());
    return 0;
}

static int l_ui_begin_panel(lua_State *L) {
    String name = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);
    ui_begin_panel_ex(ui_ctx(), name.data, opt);
    return 0;
}

static int l_ui_end_panel(lua_State *L) {
    ui_end_panel(ui_ctx());
    return 0;
}

static int l_ui_get_hover(lua_State *L) {
    lua_pushinteger(L, ui_ctx()->hover);
    return 1;
}

static int l_ui_get_focus(lua_State *L) {
    lua_pushinteger(L, ui_ctx()->focus);
    return 1;
}

static int l_ui_get_last_id(lua_State *L) {
    lua_pushinteger(L, ui_ctx()->last_id);
    return 1;
}

static int l_ui_get_style(lua_State *L) {
    luax_ptr_userdata(L, ui_ctx()->style, "mt_ui_style");
    return 1;
}

static int l_ui_rect(lua_State *L) {
    lua_createtable(L, 0, 4);
    luax_set_int_field(L, "x", luaL_checknumber(L, 1));
    luax_set_int_field(L, "y", luaL_checknumber(L, 2));
    luax_set_int_field(L, "w", luaL_checknumber(L, 3));
    luax_set_int_field(L, "h", luaL_checknumber(L, 4));
    return 1;
}

static int l_ui_color(lua_State *L) {
    lua_createtable(L, 0, 4);
    luax_set_int_field(L, "r", luaL_checknumber(L, 1));
    luax_set_int_field(L, "g", luaL_checknumber(L, 2));
    luax_set_int_field(L, "b", luaL_checknumber(L, 3));
    luax_set_int_field(L, "a", luaL_checknumber(L, 4));
    return 1;
}

static int l_ui_ref(lua_State *L) {
    MUIRef *ref = (MUIRef *)mem_alloc(sizeof(MUIRef));
    lua_ui_set_ref(L, ref, 1);
    luax_ptr_userdata(L, ref, "mt_ui_ref");
    return 1;
}

static int l_ui_begin(lua_State *L) {
    ui_begin(ui_ctx());
    return 0;
}

static int l_ui_end(lua_State *L) {
    ui_end(ui_ctx());
    return 0;
}

int open_ui(lua_State *L) {
    luaL_Reg reg[] = {
            {"Begin", l_ui_begin},
            {"End", l_ui_end},

            {"set_focus", l_ui_set_focus},
            {"get_id", l_ui_get_id},
            {"push_id", l_ui_push_id},
            {"pop_id", l_ui_pop_id},
            {"push_clip_rect", l_ui_push_clip_rect},
            {"pop_clip_rect", l_ui_pop_clip_rect},
            {"get_clip_rect", l_ui_get_clip_rect},
            {"check_clip", l_ui_check_clip},
            {"get_current_container", l_ui_get_current_container},
            {"get_container", l_ui_get_container},
            {"bring_to_front", l_ui_bring_to_front},

            {"set_clip", l_ui_set_clip},
            {"draw_rect", l_ui_draw_rect},
            {"draw_box", l_ui_draw_box},
            {"draw_text", l_ui_draw_text},
            // {"draw_icon", l_ui_draw_icon},

            {"layout_row", l_ui_layout_row},
            {"layout_width", l_ui_layout_width},
            {"layout_height", l_ui_layout_height},
            {"layout_begin_column", l_ui_layout_begin_column},
            {"layout_end_column", l_ui_layout_end_column},
            {"layout_set_next", l_ui_layout_set_next},
            {"layout_next", l_ui_layout_next},

            {"draw_control_frame", l_ui_draw_control_frame},
            {"draw_control_text", l_ui_draw_control_text},
            {"mouse_over", l_ui_mouse_over},
            {"update_control", l_ui_update_control},

            {"text", l_ui_text},
            {"label", l_ui_label},
            {"button", l_ui_button},
            {"checkbox", l_ui_checkbox},
            {"textbox_raw", l_ui_textbox_raw},
            {"textbox", l_ui_textbox},
            {"slider", l_ui_slider},
            {"number", l_ui_number},
            {"header", l_ui_header},
            {"begin_treenode", l_ui_begin_treenode},
            {"end_treenode", l_ui_end_treenode},
            {"begin_window", l_ui_begin_window},
            {"end_window", l_ui_end_window},
            {"open_popup", l_ui_open_popup},
            {"begin_popup", l_ui_begin_popup},
            {"end_popup", l_ui_end_popup},
            {"begin_panel", l_ui_begin_panel},
            {"end_panel", l_ui_end_panel},

            // access
            {"get_hover", l_ui_get_hover},
            {"get_focus", l_ui_get_focus},
            {"get_last_id", l_ui_get_last_id},
            {"get_style", l_ui_get_style},

            // utility
            {"rect", l_ui_rect},
            {"color", l_ui_color},
            {"ref", l_ui_ref},
            {nullptr, nullptr},
    };

    luaL_newlib(L, reg);

    // luax_set_string_field(L, "VERSION", UI_VERSION);

    luax_set_int_field(L, "COMMANDLIST_SIZE", UI_COMMANDLIST_SIZE);
    luax_set_int_field(L, "ROOTLIST_SIZE", UI_ROOTLIST_SIZE);
    luax_set_int_field(L, "CONTAINERSTACK_SIZE", UI_CONTAINERSTACK_SIZE);
    luax_set_int_field(L, "CLIPSTACK_SIZE", UI_CLIPSTACK_SIZE);
    luax_set_int_field(L, "IDSTACK_SIZE", UI_IDSTACK_SIZE);
    luax_set_int_field(L, "LAYOUTSTACK_SIZE", UI_LAYOUTSTACK_SIZE);
    luax_set_int_field(L, "CONTAINERPOOL_SIZE", UI_CONTAINERPOOL_SIZE);
    luax_set_int_field(L, "TREENODEPOOL_SIZE", UI_TREENODEPOOL_SIZE);
    luax_set_int_field(L, "MAX_WIDTHS", UI_MAX_WIDTHS);
    luax_set_string_field(L, "REAL_FMT", UI_REAL_FMT);
    luax_set_string_field(L, "SLIDER_FMT", UI_SLIDER_FMT);
    luax_set_int_field(L, "MAX_FMT", UI_MAX_FMT);

    luax_set_int_field(L, "CLIP_PART", UI_CLIP_PART);
    luax_set_int_field(L, "CLIP_ALL", UI_CLIP_ALL);

    luax_set_int_field(L, "COMMAND_JUMP", UI_COMMAND_JUMP);
    luax_set_int_field(L, "COMMAND_CLIP", UI_COMMAND_CLIP);
    luax_set_int_field(L, "UI_COMMAND_RECT", UI_COMMAND_RECT);
    luax_set_int_field(L, "COMMAND_TEXT", UI_COMMAND_TEXT);
    luax_set_int_field(L, "COMMAND_ICON", UI_COMMAND_ICON);

    luax_set_int_field(L, "COLOR_TEXT", UI_COLOR_TEXT);
    luax_set_int_field(L, "COLOR_BORDER", UI_COLOR_BORDER);
    luax_set_int_field(L, "COLOR_WINDOWBG", UI_COLOR_WINDOWBG);
    luax_set_int_field(L, "COLOR_TITLEBG", UI_COLOR_TITLEBG);
    luax_set_int_field(L, "COLOR_TITLETEXT", UI_COLOR_TITLETEXT);
    luax_set_int_field(L, "COLOR_PANELBG", UI_COLOR_PANELBG);
    luax_set_int_field(L, "COLOR_BUTTON", UI_COLOR_BUTTON);
    luax_set_int_field(L, "COLOR_BUTTONHOVER", UI_COLOR_BUTTONHOVER);
    luax_set_int_field(L, "COLOR_BUTTONFOCUS", UI_COLOR_BUTTONFOCUS);
    luax_set_int_field(L, "COLOR_BASE", UI_COLOR_BASE);
    luax_set_int_field(L, "COLOR_BASEHOVER", UI_COLOR_BASEHOVER);
    luax_set_int_field(L, "COLOR_BASEFOCUS", UI_COLOR_BASEFOCUS);
    luax_set_int_field(L, "COLOR_SCROLLBASE", UI_COLOR_SCROLLBASE);
    luax_set_int_field(L, "COLOR_SCROLLTHUMB", UI_COLOR_SCROLLTHUMB);

    luax_set_int_field(L, "ICON_CLOSE", UI_ICON_CLOSE);
    luax_set_int_field(L, "ICON_CHECK", UI_ICON_CHECK);
    luax_set_int_field(L, "ICON_COLLAPSED", UI_ICON_COLLAPSED);
    luax_set_int_field(L, "ICON_EXPANDED", UI_ICON_EXPANDED);

    luax_set_int_field(L, "RES_ACTIVE", UI_RES_ACTIVE);
    luax_set_int_field(L, "RES_SUBMIT", UI_RES_SUBMIT);
    luax_set_int_field(L, "RES_CHANGE", UI_RES_CHANGE);

    luax_set_int_field(L, "OPT_ALIGNCENTER", UI_OPT_ALIGNCENTER);
    luax_set_int_field(L, "OPT_ALIGNRIGHT", UI_OPT_ALIGNRIGHT);
    luax_set_int_field(L, "OPT_NOINTERACT", UI_OPT_NOINTERACT);
    luax_set_int_field(L, "OPT_NOFRAME", UI_OPT_NOFRAME);
    luax_set_int_field(L, "OPT_NORESIZE", UI_OPT_NORESIZE);
    luax_set_int_field(L, "OPT_NOSCROLL", UI_OPT_NOSCROLL);
    luax_set_int_field(L, "OPT_NOCLOSE", UI_OPT_NOCLOSE);
    luax_set_int_field(L, "OPT_NOTITLE", UI_OPT_NOTITLE);
    luax_set_int_field(L, "OPT_HOLDFOCUS", UI_OPT_HOLDFOCUS);
    luax_set_int_field(L, "OPT_AUTOSIZE", UI_OPT_AUTOSIZE);
    luax_set_int_field(L, "OPT_POPUP", UI_OPT_POPUP);
    luax_set_int_field(L, "OPT_CLOSED", UI_OPT_CLOSED);
    luax_set_int_field(L, "OPT_EXPANDED", UI_OPT_EXPANDED);

    return 1;
}

#if 0


namespace neko::imgui::util {

struct TableInteger {
    const char* name;
    lua_Integer value;
};

using GenerateAny = void (*)(lua_State* L);
struct TableAny {
    const char* name;
    GenerateAny value;
};

struct strbuf {
    char* data;
    size_t size;
};

struct input_context {
    lua_State* L;
    int callback;
};

lua_Integer field_tointeger(lua_State* L, int idx, lua_Integer i);
lua_Number field_tonumber(lua_State* L, int idx, lua_Integer i);
bool field_toboolean(lua_State* L, int idx, lua_Integer i);
ImTextureID get_texture_id(lua_State* L, int idx);
const char* format(lua_State* L, int idx);
strbuf* strbuf_create(lua_State* L, int idx);
strbuf* strbuf_get(lua_State* L, int idx);
int input_callback(ImGuiInputTextCallbackData* data);
void create_table(lua_State* L, std::span<TableInteger> l);
void set_table(lua_State* L, std::span<TableAny> l);
void struct_gen(lua_State* L, const char* name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters);
void flags_gen(lua_State* L, const char* name);
void init(lua_State* L);

}  // namespace neko::imgui::util


namespace neko::imgui::util {

static lua_CFunction str_format = NULL;

lua_Integer field_tointeger(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return v;
}

lua_Number field_tonumber(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return v;
}

bool field_toboolean(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    bool v = !!lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

ImTextureID get_texture_id(lua_State* L, int idx) {
    // int lua_handle = (int)luaL_checkinteger(L, idx);
    // if (auto id = ImGui_ImplBgfx_GetTextureID(lua_handle)) {
    //     return *id;
    // }
    // luaL_error(L, "Invalid handle type TEXTURE");
    // std::unreachable();
    neko_assert(0);
    return 0;
}

const char* format(lua_State* L, int idx) {
    lua_pushcfunction(L, str_format);
    lua_insert(L, idx);
    lua_call(L, lua_gettop(L) - idx, 1);
    return lua_tostring(L, -1);
}

static void* strbuf_realloc(lua_State* L, void* ptr, size_t osize, size_t nsize) {
    void* ud;
    lua_Alloc allocator = lua_getallocf(L, &ud);
    return allocator(ud, ptr, osize, nsize);
}

static int strbuf_assgin(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    size_t newsize = 0;
    const char* newbuf = luaL_checklstring(L, 2, &newsize);
    newsize++;
    if (newsize > sbuf->size) {
        sbuf->data = (char*)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
        sbuf->size = newsize;
    }
    memcpy(sbuf->data, newbuf, newsize);
    return 0;
}

static int strbuf_resize(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    size_t newsize = (size_t)luaL_checkinteger(L, 2);
    sbuf->data = (char*)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
    sbuf->size = newsize;
    return 0;
}

static int strbuf_tostring(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    lua_pushstring(L, sbuf->data);
    return 1;
}

static int strbuf_release(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    strbuf_realloc(L, sbuf->data, sbuf->size, 0);
    sbuf->data = NULL;
    sbuf->size = 0;
    return 0;
}

static constexpr size_t kStrBufMinSize = 256;

strbuf* strbuf_create(lua_State* L, int idx) {
    size_t sz;
    const char* text = lua_tolstring(L, idx, &sz);
    auto sbuf = (strbuf*)lua_newuserdatauv(L, sizeof(strbuf), 0);
    if (text == NULL) {
        sbuf->size = kStrBufMinSize;
        sbuf->data = (char*)strbuf_realloc(L, NULL, 0, sbuf->size);
        sbuf->data[0] = '\0';
    } else {
        sbuf->size = (std::max)(sz + 1, kStrBufMinSize);
        sbuf->data = (char*)strbuf_realloc(L, NULL, 0, sbuf->size);
        memcpy(sbuf->data, text, sz + 1);
    }
    if (luaL_newmetatable(L, "ImGui::StringBuf")) {
        lua_pushcfunction(L, strbuf_tostring);
        lua_setfield(L, -2, "__tostring");
        lua_pushcfunction(L, strbuf_release);
        lua_setfield(L, -2, "__gc");
        static luaL_Reg l[] = {
                {"Assgin", strbuf_assgin},
                {"Resize", strbuf_resize},
                {NULL, NULL},
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
    return sbuf;
}

strbuf* strbuf_get(lua_State* L, int idx) {
    if (lua_type(L, idx) == LUA_TUSERDATA) {
        auto sbuf = (strbuf*)luaL_checkudata(L, idx, "ImGui::StringBuf");
        return sbuf;
    }
    luaL_checktype(L, idx, LUA_TTABLE);
    int t = lua_geti(L, idx, 1);
    if (t != LUA_TSTRING && t != LUA_TNIL) {
        auto sbuf = (strbuf*)luaL_checkudata(L, -1, "ImGui::StringBuf");
        lua_pop(L, 1);
        return sbuf;
    }
    auto sbuf = strbuf_create(L, -1);
    lua_replace(L, -2);
    lua_seti(L, idx, 1);
    return sbuf;
}

int input_callback(ImGuiInputTextCallbackData* data) {
    auto ctx = (input_context*)data->UserData;
    lua_State* L = ctx->L;
    lua_pushvalue(L, ctx->callback);
    // wrap_ImGuiInputTextCallbackData::pointer(L, *data);
    if (luax_pcall(L, 1, 1) != LUA_OK) {
        return 1;
    }
    lua_Integer retval = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return (int)retval;
}

void create_table(lua_State* L, std::span<TableInteger> l) {
    lua_createtable(L, 0, (int)l.size());
    for (auto const& e : l) {
        lua_pushinteger(L, e.value);
        lua_setfield(L, -2, e.name);
    }
}

void set_table(lua_State* L, std::span<TableAny> l) {
    for (auto const& e : l) {
        e.value(L);
        lua_setfield(L, -2, e.name);
    }
}

static void set_table(lua_State* L, std::span<luaL_Reg> l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (auto const& e : l) {
        for (int i = 0; i < nup; i++) {
            lua_pushvalue(L, -nup);
        }
        lua_pushcclosure(L, e.func, nup);
        lua_setfield(L, -(nup + 2), e.name);
    }
    lua_pop(L, nup);
}

static int make_flags(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int i, t;
    lua_Integer r = 0;
    for (i = 1; (t = lua_geti(L, 1, i)) != LUA_TNIL; i++) {
        if (t != LUA_TSTRING) luaL_error(L, "Flag name should be string, it's %s", lua_typename(L, t));
        if (lua_gettable(L, lua_upvalueindex(1)) != LUA_TNUMBER) {
            lua_geti(L, 1, i);
            luaL_error(L, "Invalid flag %s.%s", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, -1));
        }
        lua_Integer v = lua_tointeger(L, -1);
        lua_pop(L, 1);
        r |= v;
    }
    lua_pushinteger(L, r);
    return 1;
}

void struct_gen(lua_State* L, const char* name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters) {
    lua_newuserdatauv(L, sizeof(uintptr_t), 0);
    int ud = lua_gettop(L);
    lua_newtable(L);
    if (!setters.empty()) {
        static lua_CFunction setter_func = +[](lua_State* L) {
            lua_pushvalue(L, 2);
            if (LUA_TNIL == lua_gettable(L, lua_upvalueindex(1))) {
                return luaL_error(L, "%s.%s is invalid.", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, 2));
            }
            lua_pushvalue(L, 3);
            lua_call(L, 1, 0);
            return 0;
        };
        lua_createtable(L, 0, (int)setters.size());
        lua_pushvalue(L, ud);
        set_table(L, setters, 1);
        lua_pushstring(L, name);
        lua_pushcclosure(L, setter_func, 2);
        lua_setfield(L, -2, "__newindex");
    }
    if (!funcs.empty()) {
        lua_createtable(L, 0, (int)funcs.size());
        lua_pushvalue(L, ud);
        set_table(L, funcs, 1);
        lua_newtable(L);
    }
    static lua_CFunction getter_func = +[](lua_State* L) {
        lua_pushvalue(L, 2);
        if (LUA_TNIL == lua_gettable(L, lua_upvalueindex(1))) {
            return luaL_error(L, "%s.%s is invalid.", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, 2));
        }
        lua_call(L, 0, 1);
        return 1;
    };
    lua_createtable(L, 0, (int)getters.size());
    lua_pushvalue(L, ud);
    set_table(L, getters, 1);
    lua_pushstring(L, name);
    lua_pushcclosure(L, getter_func, 2);
    lua_setfield(L, -2, "__index");
    if (!funcs.empty()) {
        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
}

void flags_gen(lua_State* L, const char* name) {
    lua_pushstring(L, name);
    lua_pushcclosure(L, make_flags, 2);
}

void init(lua_State* L) {
    luaopen_string(L);
    lua_getfield(L, -1, "format");
    str_format = lua_tocfunction(L, -1);
    lua_pop(L, 2);
}

}  // namespace neko::imgui::util

#endif

static Entity gui_root;  // 所有 gui 都应该是它的子节点 以便随屏幕移动

static Entity focused;  // 当前聚焦的实体 如果没有则为entity_nil

static bool captured_event = false;

// --- common --------------------------------------------------------------

// 所有 GUI 系统共有的一般功能/数据

typedef struct Gui Gui;
struct Gui {
    EntityPoolElem pool_elem;

    bool setvisible;       // 外部设置可见性
    bool visible;          // 内部递归计算可见性
    bool updated_visible;  // 用于递归可见性计算
    bool focusable;        // can be focused
    bool captures_events;

    Color color;

    BBox bbox;  // 在实体空间中
    GuiAlign halign;
    GuiAlign valign;
    vec2 padding;
};

static EntityPool *pool_gui;

static EntityMap *focus_enter_map;
static EntityMap *focus_exit_map;
static EntityMap *changed_map;
static EntityMap *mouse_down_map;
static EntityMap *mouse_up_map;
static EntityMap *key_down_map;
static EntityMap *key_up_map;

Entity gui_get_root() { return gui_root; }

void gui_add(Entity ent) {
    Gui *gui;

    if (entitypool_get(pool_gui, ent)) return;  // 已经有gui

    transform_add(ent);

    gui = (Gui *)entitypool_add(pool_gui, ent);
    gui->visible = true;
    gui->setvisible = true;
    gui->focusable = false;
    gui->captures_events = true;
    gui->color = color_gray;
    gui->bbox = bbox(vec2_zero, luavec2(32, 32));
    gui->halign = GA_NONE;
    gui->valign = GA_NONE;
    gui->padding = luavec2(5, 5);
}

void gui_remove(Entity ent) { entitypool_remove(pool_gui, ent); }

bool gui_has(Entity ent) { return entitypool_get(pool_gui, ent) != NULL; }

void gui_set_color(Entity ent, Color color) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->color = color;
}
Color gui_get_color(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->color;
}

void gui_set_visible(Entity ent, bool visible) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->setvisible = visible;
}
bool gui_get_visible(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->visible;
}

void gui_set_focusable(Entity ent, bool focusable) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->focusable = focusable;
}
bool gui_get_focusable(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->focusable;
}

void gui_set_captures_events(Entity ent, bool captures_events) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->captures_events = captures_events;
}
bool gui_get_captures_events(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->captures_events;
}

void gui_set_halign(Entity ent, GuiAlign align) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->halign = align;
}
GuiAlign gui_get_halign(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->halign;
}
void gui_set_valign(Entity ent, GuiAlign align) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->valign = align;
}
GuiAlign gui_get_valign(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->valign;
}
void gui_set_padding(Entity ent, vec2 padding) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->padding = padding;
}
vec2 gui_get_padding(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->padding;
}

void gui_set_focused_entity(Entity ent) {
    if (entity_eq(focused, ent)) return;

    if (entity_eq(ent, entity_nil)) entitymap_set(focus_exit_map, focused, true);
    focused = ent;
    if (!entity_eq(focused, entity_nil)) entitymap_set(focus_enter_map, focused, true);
}
Entity gui_get_focused_entity() { return focused; }
void gui_set_focus(Entity ent, bool focus) {
    if (focus)
        gui_set_focused_entity(ent);
    else if (entity_eq(focused, ent))
        gui_set_focused_entity(entity_nil);
}
bool gui_get_focus(Entity ent) { return entity_eq(focused, ent); }
bool gui_has_focus() { return !entity_eq(focused, entity_nil); }

void gui_fire_event_changed(Entity ent) { entitymap_set(changed_map, ent, true); }

bool gui_event_focus_enter(Entity ent) { return entitymap_get(focus_enter_map, ent); }
bool gui_event_focus_exit(Entity ent) { return entitymap_get(focus_exit_map, ent); }
bool gui_event_changed(Entity ent) { return entitymap_get(changed_map, ent); }
MouseCode gui_event_mouse_down(Entity ent) { return (MouseCode)entitymap_get(mouse_down_map, ent); }
MouseCode gui_event_mouse_up(Entity ent) { return (MouseCode)entitymap_get(mouse_up_map, ent); }
KeyCode gui_event_key_down(Entity ent) { return (KeyCode)entitymap_get(key_down_map, ent); }
KeyCode gui_event_key_up(Entity ent) { return (KeyCode)entitymap_get(key_up_map, ent); }

bool gui_captured_event() { return captured_event; }

static void _common_init() {
    pool_gui = entitypool_new(Gui);
    focus_enter_map = entitymap_new(false);
    focus_exit_map = entitymap_new(false);
    changed_map = entitymap_new(false);
    mouse_down_map = entitymap_new(MC_NONE);
    mouse_up_map = entitymap_new(MC_NONE);
    key_down_map = entitymap_new(KC_NONE);
    key_up_map = entitymap_new(KC_NONE);
}
static void _common_fini() {
    entitymap_free(key_up_map);
    entitymap_free(key_down_map);
    entitymap_free(mouse_up_map);
    entitymap_free(mouse_down_map);
    entitymap_free(changed_map);
    entitymap_free(focus_enter_map);
    entitymap_free(focus_exit_map);
    entitypool_free(pool_gui);
}

static void _common_update_destroyed() {
    if (entity_destroyed(focused)) focused = entity_nil;
    entitypool_remove_destroyed(pool_gui, gui_remove);
}

static void _common_update_visible_rec(Gui *gui) {
    Gui *pgui;

    if (gui->updated_visible) return;

    // false visibility takes priority
    if (!gui->setvisible) {
        gui->visible = false;
        gui->updated_visible = true;
        return;
    }

    // if has parent, inherit
    pgui = (Gui *)entitypool_get(pool_gui, transform_get_parent(gui->pool_elem.ent));
    if (pgui) {
        _common_update_visible_rec(pgui);
        gui->visible = pgui->visible;
        gui->updated_visible = true;
        return;
    }

    // else just set
    gui->visible = true;
    gui->updated_visible = true;
}
static void _common_update_visible() {
    Gui *gui;

    entitypool_foreach(gui, pool_gui) gui->updated_visible = false;
    entitypool_foreach(gui, pool_gui) _common_update_visible_rec(gui);
}

static void _common_align(Gui *gui, GuiAlign halign, GuiAlign valign) {
    Gui *pgui;
    BBox b, pb;
    vec2 pos;
    Entity ent;
    Scalar mid, pmid;

    if (halign == GA_NONE && valign == GA_NONE) return;

    ent = gui->pool_elem.ent;

    // get parent-space bounding box and position
    b = bbox_transform(transform_get_matrix(ent), gui->bbox);
    pos = transform_get_position(ent);

    // get parent gui and its bounding box
    pgui = (Gui *)entitypool_get(pool_gui, transform_get_parent(ent));
    if (!pgui) return;
    pb = pgui->bbox;

    // macro to avoid repetition -- 'z' is vec2 axis member (x or y)
#define axis_align(align, z)                                       \
    switch (align) {                                               \
        case GA_MIN:                                               \
            pos.z = pb.min.z + gui->padding.z + pos.z - b.min.z;   \
            break;                                                 \
        case GA_MAX:                                               \
            pos.z = pb.max.z - gui->padding.z - (b.max.z - pos.z); \
            break;                                                 \
        case GA_MID:                                               \
            mid = 0.5 * (b.min.z + b.max.z);                       \
            pmid = 0.5 * (pb.min.z + pb.max.z);                    \
            pos.z = pmid - (mid - pos.z);                          \
            break;                                                 \
        default:                                                   \
            break;                                                 \
    }

    axis_align(halign, x);
    axis_align(valign, y);
    transform_set_position(ent, pos);
}

// move everything to top-left -- for fit calculations
static void _common_reset_align() {
    Gui *gui;
    entitypool_foreach(gui, pool_gui) _common_align(gui, gui->halign == GA_NONE ? GA_NONE : GA_MIN, gui->valign == GA_NONE ? GA_NONE : GA_MAX);
}

static void _common_update_align() {
    Gui *gui;
    entitypool_foreach(gui, pool_gui) _common_align(gui, gui->halign, gui->valign);
}

// attach root GUI entities to gui_root
static void _common_attach_root() {
    Gui *gui;
    Entity ent;

    entitypool_foreach(gui, pool_gui) {
        ent = gui->pool_elem.ent;
        if (!entity_eq(ent, gui_root) && entity_eq(transform_get_parent(ent), entity_nil)) transform_set_parent(ent, gui_root);
    }
}

static void _common_update_all() {
    Gui *gui;

    _common_attach_root();

    // update edit bboxes
    if (edit_get_enabled()) entitypool_foreach(gui, pool_gui) edit_bboxes_update(gui->pool_elem.ent, gui->bbox);
}

// 'focus_clear' is whether to clear focus if click outside
static void _common_mouse_event(EntityMap *emap, MouseCode mouse, bool focus_clear) {
    Gui *gui;
    vec2 m;
    mat3 t;
    Entity ent;
    bool some_focused = false;

    m = camera_unit_to_world(input_get_mouse_pos_unit());
    entitypool_foreach(gui, pool_gui) if (gui->visible && !(edit_get_enabled() && edit_get_editable(gui->pool_elem.ent))) {
        ent = gui->pool_elem.ent;

        t = mat3_inverse(transform_get_world_matrix(ent));
        if (bbox_contains(gui->bbox, mat3_transform(t, m))) {
            entitymap_set(emap, ent, mouse);

            if (gui->captures_events) captured_event = true;

            // focus?
            if (gui->focusable && mouse == MC_LEFT) {
                some_focused = true;
                gui_set_focused_entity(ent);
            }
        }
    }

    // none focused? clear
    if (focus_clear && !some_focused) gui_set_focused_entity(entity_nil);
}
static void _common_mouse_down(MouseCode mouse) { _common_mouse_event(mouse_down_map, mouse, true); }
static void _common_mouse_up(MouseCode mouse) { _common_mouse_event(mouse_up_map, mouse, false); }

static void _common_key_down(KeyCode key) {
    if (!entity_eq(focused, entity_nil)) {
        entitymap_set(key_down_map, focused, key);
        captured_event = true;
    }
}
static void _common_key_up(KeyCode key) {
    if (!entity_eq(focused, entity_nil)) {
        entitymap_set(key_up_map, focused, key);
        captured_event = true;
    }
}
static void _common_char_down(unsigned int c) {
    if (!entity_eq(focused, entity_nil)) captured_event = true;
}

static void _common_event_clear() {
    entitymap_clear(focus_enter_map);
    entitymap_clear(focus_exit_map);
    entitymap_clear(changed_map);
    entitymap_clear(mouse_down_map);
    entitymap_clear(mouse_up_map);
    entitymap_clear(key_down_map);
    entitymap_clear(key_up_map);
    captured_event = false;
}

static void _common_save_all(Store *s) {
    Store *t, *gui_s;
    Gui *gui;

    if (store_child_save(&t, "gui", s)) entitypool_save_foreach(gui, gui_s, pool_gui, "pool", t) {
            color_save(&gui->color, "color", gui_s);
            bool_save(&gui->visible, "visible", gui_s);
            bool_save(&gui->setvisible, "setvisible", gui_s);
            bool_save(&gui->focusable, "focusable", gui_s);
            bool_save(&gui->captures_events, "captures_events", gui_s);
            enum_save(&gui->halign, "halign", gui_s);
            enum_save(&gui->valign, "valign", gui_s);
            vec2_save(&gui->padding, "padding", gui_s);
        }
}
static void _common_load_all(Store *s) {
    Store *t, *gui_s;
    Gui *gui;

    if (store_child_load(&t, "gui", s)) entitypool_load_foreach(gui, gui_s, pool_gui, "pool", t) {
            color_load(&gui->color, "color", color_gray, gui_s);
            bool_load(&gui->visible, "visible", true, gui_s);
            bool_load(&gui->setvisible, "setvisible", true, gui_s);
            bool_load(&gui->focusable, "focusable", false, gui_s);
            bool_load(&gui->captures_events, "captures_events", true, gui_s);
            enum_load(&gui->halign, "halign", GA_NONE, gui_s);
            enum_load(&gui->valign, "valign", GA_NONE, gui_s);
            vec2_load(&gui->padding, "padding", luavec2(5, 5), gui_s);
        }

    _common_attach_root();
}

// --- rect ----------------------------------------------------------------

typedef struct Rect Rect;
struct Rect {
    EntityPoolElem pool_elem;

    mat3 wmat;

    vec2 size;
    bool visible;
    Color color;

    bool hfit;
    bool vfit;
    bool hfill;
    bool vfill;

    bool updated;
    int depth;  // for draw order -- child depth > parent depth
};

static EntityPool *rect_pool;

void gui_rect_add(Entity ent) {
    Rect *rect;

    if (entitypool_get(rect_pool, ent)) return;

    gui_add(ent);

    rect = (Rect *)entitypool_add(rect_pool, ent);
    rect->size = luavec2(64, 64);
    rect->hfit = true;
    rect->vfit = true;
    rect->hfill = false;
    rect->vfill = false;
}
void gui_rect_remove(Entity ent) { entitypool_remove(rect_pool, ent); }
bool gui_rect_has(Entity ent) { return entitypool_get(rect_pool, ent) != NULL; }

void gui_rect_set_size(Entity ent, vec2 size) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->size = size;
}
vec2 gui_rect_get_size(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->size;
}

void gui_rect_set_hfit(Entity ent, bool fit) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfit = fit;
}
bool gui_rect_get_hfit(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfit;
}
void gui_rect_set_vfit(Entity ent, bool fit) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfit = fit;
}
bool gui_rect_get_vfit(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfit;
}

void gui_rect_set_hfill(Entity ent, bool fill) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfill = fill;
}
bool gui_rect_get_hfill(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfill;
}
void gui_rect_set_vfill(Entity ent, bool fill) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfill = fill;
}
bool gui_rect_get_vfill(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfill;
}

static GLuint rect_program;
static GLuint rect_vao;
static GLuint rect_vbo;

static void _rect_init() {
    // init pool
    rect_pool = entitypool_new(Rect);

    // create shader program, load texture, bind parameters
    rect_program = gfx_create_program("rect_program", "shader/rect.vert", "shader/rect.geom", "shader/rect.frag");
    glUseProgram(rect_program);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &rect_vao);
    glBindVertexArray(rect_vao);
    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 3, "wmat1", Rect, wmat.m[0]);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 3, "wmat2", Rect, wmat.m[1]);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 3, "wmat3", Rect, wmat.m[2]);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 2, "size", Rect, size);
    gfx_bind_vertex_attrib(rect_program, GL_INT, 1, "visible", Rect, visible);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 4, "color", Rect, color);
}
static void _rect_fini() {
    // fini gl stuff
    glDeleteProgram(rect_program);
    glDeleteBuffers(1, &rect_vbo);
    glDeleteVertexArrays(1, &rect_vao);

    // fini pool
    entitypool_free(rect_pool);
}

static void _rect_update_child_first(Entity ent);

static void _rect_update_table_align(Rect *rect) {
    Entity rect_ent, *children;
    Gui *child;
    unsigned int nchildren, i;
    Scalar delta;
    BBox b;
    vec2 pos, curr;

    rect_ent = rect->pool_elem.ent;

    curr = vec2_zero;
    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui *)entitypool_get(pool_gui, children[i]);
        if (!(child && child->visible && (child->halign == GA_TABLE || child->valign == GA_TABLE))) continue;
        _rect_update_child_first(children[i]);

        b = bbox_transform(transform_get_matrix(children[i]), child->bbox);
        pos = transform_get_position(children[i]);

        if (child->halign == GA_TABLE) {
            delta = curr.x + child->padding.x - b.min.x;
            pos.x += delta;
            curr.x = b.max.x + delta;
        }
        if (child->valign == GA_TABLE) {
            delta = curr.y - child->padding.y - b.max.y;
            pos.y += delta;
            curr.y = b.min.y + delta;
        }

        transform_set_position(children[i], pos);
    }
}

static void _rect_update_fit(Rect *rect) {
    Entity rect_ent, *children;
    Gui *child;
    unsigned int nchildren, i;
    Scalar miny, maxx;
    BBox b;

    rect_ent = rect->pool_elem.ent;

    miny = 0;
    maxx = 0;

    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui *)entitypool_get(pool_gui, children[i]);
        if (!child || !child->visible) continue;
        _rect_update_child_first(children[i]);

        b = bbox_transform(transform_get_matrix(children[i]), child->bbox);
        if (rect->hfit) maxx = scalar_max(maxx, b.max.x + child->padding.x);
        if (rect->vfit) miny = scalar_min(miny, b.min.y - child->padding.y);
    }

    if (rect->hfit) rect->size.x = maxx;
    if (rect->vfit) rect->size.y = -miny;
}

static void _rect_update_child_first(Entity ent) {
    Rect *rect;
    Gui *gui;

    gui = (Gui *)entitypool_get(pool_gui, ent);
    if (!gui) return;

    rect = (Rect *)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_table_align(rect);
    _rect_update_fit(rect);

    gui->bbox = bbox_bound(vec2_zero, luavec2(rect->size.x, -rect->size.y));
}

static void _rect_update_parent_first(Entity ent);

static void _rect_update_fill(Rect *rect) {
    Entity ent;
    Gui *pgui, *gui;
    BBox b;
    Entity parent;

    ent = rect->pool_elem.ent;
    gui = (Gui *)entitypool_get(pool_gui, ent);
    if (!gui) return;

    if (!rect || !rect->visible || rect->updated || !(rect->hfill || rect->vfill)) return;

    parent = transform_get_parent(ent);
    pgui = (Gui *)entitypool_get(pool_gui, parent);
    if (!pgui) return;  // no parent to fill to

    _rect_update_parent_first(parent);
    b = bbox_transform(mat3_inverse(transform_get_matrix(ent)), pgui->bbox);

    if (rect->hfill) rect->size.x = b.max.x - gui->padding.x;
    if (rect->vfill) rect->size.y = -b.min.y + gui->padding.y;
}

static void _rect_update_depth(Rect *rect) {
    Rect *prect;

    prect = (Rect *)entitypool_get(rect_pool, transform_get_parent(rect->pool_elem.ent));
    if (prect) {
        _rect_update_parent_first(prect->pool_elem.ent);
        rect->depth = prect->depth + 1;
    } else
        rect->depth = 0;
}

static void _rect_update_parent_first(Entity ent) {
    Rect *rect;
    Gui *gui;

    gui = (Gui *)entitypool_get(pool_gui, ent);
    if (!gui) return;

    rect = (Rect *)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_fill(rect);
    _rect_update_depth(rect);

    gui->bbox = bbox_bound(vec2_zero, luavec2(rect->size.x, -rect->size.y));
}

static void _rect_update_all() {
    Rect *rect;
    Gui *gui;

    entitypool_remove_destroyed(rect_pool, gui_rect_remove);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_child_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_parent_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) {
        gui = (Gui *)entitypool_get(pool_gui, rect->pool_elem.ent);
        error_assert(gui);

        // write gui bbox
        gui->bbox = bbox_bound(vec2_zero, luavec2(rect->size.x, -rect->size.y));

        // read gui properties
        rect->visible = gui->visible;
        rect->color = gui->color;
    }
}

static void _rect_update_wmat() {
    Rect *rect;
    entitypool_foreach(rect, rect_pool) rect->wmat = transform_get_world_matrix(rect->pool_elem.ent);
}

static int _rect_depth_compare(const void *a, const void *b) {
    const Rect *ra = (Rect *)a, *rb = (Rect *)b;
    if (ra->depth == rb->depth) return ((int)ra->pool_elem.ent.id) - ((int)rb->pool_elem.ent.id);
    return ra->depth - rb->depth;
}

static void _rect_draw_all() {
    unsigned int nrects;

    // depth sort
    entitypool_sort(rect_pool, _rect_depth_compare);

    // bind shader program
    glUseProgram(rect_program);
    glUniformMatrix3fv(glGetUniformLocation(rect_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    // draw!
    glBindVertexArray(rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    nrects = entitypool_size(rect_pool);
    glBufferData(GL_ARRAY_BUFFER, nrects * sizeof(Rect), entitypool_begin(rect_pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nrects);
}

static void _rect_save_all(Store *s) {
    Store *t, *rect_s;
    Rect *rect;

    if (store_child_save(&t, "gui_rect", s)) entitypool_save_foreach(rect, rect_s, rect_pool, "pool", t) {
            vec2_save(&rect->size, "size", rect_s);
            color_save(&rect->color, "color", rect_s);
            bool_save(&rect->hfit, "hfit", rect_s);
            bool_save(&rect->vfit, "vfit", rect_s);
            bool_save(&rect->hfill, "hfill", rect_s);
            bool_save(&rect->vfill, "vfill", rect_s);
        }
}
static void _rect_load_all(Store *s) {
    Store *t, *rect_s;
    Rect *rect;

    if (store_child_load(&t, "gui_rect", s)) entitypool_load_foreach(rect, rect_s, rect_pool, "pool", t) {
            vec2_load(&rect->size, "size", luavec2(64, 64), rect_s);
            color_load(&rect->color, "color", color_gray, rect_s);
            bool_load(&rect->hfit, "hfit", true, rect_s);
            bool_load(&rect->vfit, "vfit", true, rect_s);
            bool_load(&rect->hfill, "hfill", false, rect_s);
            bool_load(&rect->vfill, "vfill", false, rect_s);
        }
}

// --- text ----------------------------------------------------------------

#define TEXT_GRID_W 16
#define TEXT_GRID_H 16

#define TEXT_FONT_W 10
#define TEXT_FONT_H 12

// info to send to shader program for each character
typedef struct TextChar TextChar;
struct TextChar {
    vec2 pos;         // position in space of text entity in size-less units
    vec2 cell;        // cell in font image
    float is_cursor;  // > 0 iff. this char is cursor
};

// info per text entity
typedef struct Text Text;
struct Text {
    EntityPoolElem pool_elem;

    char *str;
    CArray *chars;  // per-character info buffered to shader
    vec2 bounds;    // max x, min y in size-less units

    int cursor;
};

static EntityPool *text_pool;

static Scalar cursor_blink_time = 0;

static void _text_add_cursor(Text *text, vec2 pos) {
    TextChar *tc;

    // compute position in font grid
    tc = (TextChar *)array_add(text->chars);
    tc->pos = pos;
    tc->cell = luavec2(' ' % TEXT_GRID_W, TEXT_GRID_H - 1 - (' ' / TEXT_GRID_W));
    tc->is_cursor = 1;
}

// just update with existing string if str is NULL
static void _text_set_str(Text *text, const char *str) {
    char c;
    TextChar *tc;
    vec2 pos;
    int i = 0;

    // copy to struct?
    if (str) {
        mem_free(text->str);
        text->str = (char *)mem_alloc(strlen(str) + 1);
        strcpy(text->str, str);
    } else
        str = text->str;

    // create TextChar array and update bounds
    pos = luavec2(0, -1);
    text->bounds = luavec2(1, -1);
    array_clear(text->chars);
    while (*str) {
        if (i++ == text->cursor) _text_add_cursor(text, pos);

        c = *str++;
        switch (c) {
            case '\n':
                // next line
                pos.x = 0;
                pos.y -= 1;
                continue;
        }

        // compute position in font grid
        tc = (TextChar *)array_add(text->chars);
        tc->pos = pos;
        tc->cell = luavec2(c % TEXT_GRID_W, TEXT_GRID_H - 1 - (c / TEXT_GRID_W));
        tc->is_cursor = -1;

        // move ahead
        pos.x += 1;
        text->bounds.x = scalar_max(text->bounds.x, pos.x);
    }

    // cursor at end?
    if (i == text->cursor) {
        _text_add_cursor(text, pos);
        pos.x += 1;
        text->bounds.x = scalar_max(text->bounds.x, pos.x);
    }

    text->bounds.y = pos.y;
}

void gui_text_add(Entity ent) {
    Text *text;

    if (entitypool_get(text_pool, ent)) return;  // already has text

    gui_add(ent);

    text = (Text *)entitypool_add(text_pool, ent);
    text->chars = array_new(TextChar);
    text->str = NULL;  // _text_set_str(...) calls mem_free(text->str)
    text->cursor = -1;
    _text_set_str(text, "");
}
void gui_text_remove(Entity ent) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    if (text) {
        mem_free(text->str);
        array_free(text->chars);
    }
    entitypool_remove(text_pool, ent);
}
bool gui_text_has(Entity ent) { return entitypool_get(text_pool, ent) != NULL; }

void gui_text_set_str(Entity ent, const char *str) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    _text_set_str(text, str);
}
const char *gui_text_get_str(Entity ent) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    return text->str;
}

static void _text_set_cursor(Entity ent, int cursor) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    text->cursor = cursor;
    _text_set_str(text, NULL);
}

static GLuint text_program;
static GLuint text_vao;
static GLuint text_vbo;

static void _text_init() {
    // init pool
    text_pool = entitypool_new(Text);

    // create shader program, load texture, bind parameters
    text_program = gfx_create_program("text_program", "shader/text.vert", "shader/text.geom", "shader/text.frag");
    glUseProgram(text_program);

    asset_load(AssetLoadData{AssetKind_Image, true}, "assets/data/font1.png", NULL);

    glUniform1i(glGetUniformLocation(text_program, "tex0"), 0);
    glUniform2f(glGetUniformLocation(text_program, "inv_grid_size"), 1.0 / TEXT_GRID_W, 1.0 / TEXT_GRID_H);
    glUniform2f(glGetUniformLocation(text_program, "size"), TEXT_FONT_W, TEXT_FONT_H);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &text_vao);
    glBindVertexArray(text_vao);
    glGenBuffers(1, &text_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "pos", TextChar, pos);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "cell", TextChar, cell);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 1, "is_cursor", TextChar, is_cursor);
}
static void _text_fini() {
    Text *text;

    // fini gl stuff
    glDeleteProgram(text_program);
    glDeleteBuffers(1, &text_vbo);
    glDeleteVertexArrays(1, &text_vao);

    // fini pool
    entitypool_foreach(text, text_pool) {
        mem_free(text->str);
        array_free(text->chars);
    }
    entitypool_free(text_pool);
}

static void _text_update_all() {
    Text *text;
    Gui *gui;
    static vec2 size = {TEXT_FONT_W, TEXT_FONT_H};

    cursor_blink_time += 2 * timing_instance.true_dt;

    entitypool_remove_destroyed(text_pool, gui_text_remove);

    entitypool_foreach(text, text_pool) {
        // blink on when focus entered
        if (gui_event_focus_enter(text->pool_elem.ent)) cursor_blink_time = 1;

        // gui bbox
        gui = (Gui *)entitypool_get(pool_gui, text->pool_elem.ent);
        error_assert(gui);
        gui->bbox = bbox_bound(vec2_zero, vec2_mul(size, text->bounds));
    }
}

static void _text_draw_all() {
    vec2 hwin;
    Text *text;
    Gui *gui;
    mat3 wmat;
    vec2 pos;
    unsigned int nchars;

    hwin = vec2_scalar_mul(game_get_window_size(), 0.5);

    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);

    // TODO: 我不知道为什么引入neko_render后会有错误 在这里重新设置顶点属性指针可以修复
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "pos", TextChar, pos);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "cell", TextChar, cell);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 1, "is_cursor", TextChar, is_cursor);

    // bind shader program
    glUseProgram(text_program);
    // glUniform1i(glGetUniformLocation(text_program, "tex0"), 1);
    glUniform1f(glGetUniformLocation(text_program, "cursor_blink"), ((int)cursor_blink_time) & 1);
    glUniformMatrix3fv(glGetUniformLocation(text_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    texture_bind("assets/data/font1.png");

    // draw!
    entitypool_foreach(text, text_pool) {
        gui = (Gui *)entitypool_get(pool_gui, text->pool_elem.ent);
        error_assert(gui);
        if (!gui->visible) continue;
        glUniform4fv(glGetUniformLocation(text_program, "base_color"), 1, (const GLfloat *)&gui->color);

        wmat = transform_get_world_matrix(text->pool_elem.ent);
        glUniformMatrix3fv(glGetUniformLocation(text_program, "wmat"), 1, GL_FALSE, (const GLfloat *)&wmat);

        nchars = array_length(text->chars);
        glBufferData(GL_ARRAY_BUFFER, nchars * sizeof(TextChar), array_begin(text->chars), GL_STREAM_DRAW);

        glDrawArrays(GL_POINTS, 0, nchars);
    }

    // entitypool_foreach(text, text_pool) {
    //     gui = (Gui*)entitypool_get(gui_pool, text->pool_elem.ent);
    //     error_assert(gui);
    //     if (!gui->visible) continue;
    //     pos = transform_get_position(text->pool_elem.ent);
    //     const_str str = text->str;
    //     draw_font(&g_app->idraw, g_app->default_font, 16.f, pos.x, pos.y, str, NEKO_COLOR_WHITE);
    // }
}

static void _text_save_all(Store *s) {
    Store *t, *text_s;
    Text *text;

    if (store_child_save(&t, "gui_text", s)) entitypool_save_foreach(text, text_s, text_pool, "pool", t) {
            string_save((const char **)&text->str, "str", text_s);
            int_save(&text->cursor, "cursor", text_s);
        }
}
static void _text_load_all(Store *s) {
    Store *t, *text_s;
    Text *text;

    if (store_child_load(&t, "gui_text", s)) entitypool_load_foreach(text, text_s, text_pool, "pool", t) {
            text->chars = array_new(TextChar);
            string_load(&text->str, "str", "", text_s);
            int_load(&text->cursor, "cursor", -1, text_s);
            _text_set_str(text, NULL);
        }
}

// --- textedit ------------------------------------------------------------

typedef struct TextEdit TextEdit;
struct TextEdit {
    EntityPoolElem pool_elem;

    unsigned int cursor;  // 0 at beginning of string
    bool numerical;
};

EntityPool *textedit_pool;

void gui_textedit_add(Entity ent) {
    TextEdit *textedit;

    if (entitypool_get(textedit_pool, ent)) return;

    gui_text_add(ent);
    gui_set_focusable(ent, true);

    textedit = (TextEdit *)entitypool_add(textedit_pool, ent);
    textedit->cursor = 0;
    textedit->numerical = false;
}
void gui_textedit_remove(Entity ent) { entitypool_remove(textedit_pool, ent); }
bool gui_textedit_has(Entity ent) { return entitypool_get(textedit_pool, ent) != NULL; }

void gui_textedit_set_numerical(Entity ent, bool numerical) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->numerical = numerical;
}
bool gui_textedit_get_numerical(Entity ent) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->numerical;
}
Scalar gui_textedit_get_num(Entity ent) { return strtof(gui_text_get_str(ent), NULL); }

static void _textedit_fix_cursor(TextEdit *textedit) {
    unsigned int len = strlen(gui_text_get_str(textedit->pool_elem.ent));
    if (textedit->cursor > len) textedit->cursor = len;
}

void gui_textedit_set_cursor(Entity ent, unsigned int cursor) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->cursor = cursor;
    _textedit_fix_cursor(textedit);
}
unsigned int gui_textedit_get_cursor(Entity ent) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->cursor;
}

static void _textedit_init() { textedit_pool = entitypool_new(TextEdit); }
static void _textedit_fini() { entitypool_free(textedit_pool); }

static bool _textedit_set_str(TextEdit *textedit, const char *str) {
    gui_text_set_str(textedit->pool_elem.ent, str);
    entitymap_set(changed_map, textedit->pool_elem.ent, true);
    return true;
}

// common function for key/char events
static void _textedit_key_event(KeyCode key, unsigned int c) {
    Entity ent;
    TextEdit *textedit;
    const char *old;
    char *new_ptr = NULL;

    textedit = (TextEdit *)entitypool_get(textedit_pool, focused);
    if (!textedit) return;
    ent = textedit->pool_elem.ent;
    _textedit_fix_cursor(textedit);

    // blink on for feedback
    cursor_blink_time = 1;

    old = gui_text_get_str(ent);

    // confirm?
    if (key == KC_ENTER || key == KC_ESCAPE) {
        gui_set_focused_entity(entity_nil);
    }

    // left/right
    else if (key == KC_LEFT) {
        if (textedit->cursor > 0) --textedit->cursor;
    } else if (key == KC_RIGHT) {
        if (textedit->cursor < strlen(old)) ++textedit->cursor;
    }

    // remove char
    else if (key == KC_BACKSPACE || key == KC_DELETE) {
        if (key == KC_BACKSPACE)
            if (textedit->cursor > 0) --textedit->cursor;

        new_ptr = (char *)mem_alloc(strlen(old));  // 1 less, but 1 more for null
        strncpy(new_ptr, old, textedit->cursor);
        strcpy(&new_ptr[textedit->cursor], &old[textedit->cursor + 1]);
        _textedit_set_str(textedit, new_ptr);
    }

    // insert char
    else if (isprint(c)) {
        new_ptr = (char *)mem_alloc(strlen(old) + 2);  // 1 for new_ptr char, 1 for null
        strncpy(new_ptr, old, textedit->cursor);
        new_ptr[textedit->cursor] = (char)c;
        strcpy(&new_ptr[textedit->cursor + 1], &old[textedit->cursor]);
        if (_textedit_set_str(textedit, new_ptr)) ++textedit->cursor;
    }

    mem_free(new_ptr);
}

static void _textedit_char_down(unsigned int c) { _textedit_key_event(KC_NONE, c); }

static void _textedit_key_down(KeyCode key) { _textedit_key_event(key, 0); }

static void _textedit_update_all() {
    Entity ent;
    TextEdit *textedit;

    entitypool_remove_destroyed(textedit_pool, gui_textedit_remove);

    entitypool_foreach(textedit, textedit_pool) {
        ent = textedit->pool_elem.ent;
        _textedit_fix_cursor(textedit);

        // focus stuff
        if (gui_get_focus(ent))
            _text_set_cursor(ent, textedit->cursor);
        else
            _text_set_cursor(ent, -1);
    }
}

static void _textedit_save_all(Store *s) {
    Store *t, *textedit_s;
    TextEdit *textedit;

    if (store_child_save(&t, "gui_textedit", s)) entitypool_save_foreach(textedit, textedit_s, textedit_pool, "pool", t) {
            uint_save(&textedit->cursor, "cursor", textedit_s);
            bool_save(&textedit->numerical, "numerical", textedit_s);
        }
}
static void _textedit_load_all(Store *s) {
    Store *t, *textedit_s;
    TextEdit *textedit;

    if (store_child_load(&t, "gui_textedit", s)) entitypool_load_foreach(textedit, textedit_s, textedit_pool, "pool", t) {
            uint_load(&textedit->cursor, "cursor", 0, textedit_s);
            bool_load(&textedit->numerical, "numerical", false, textedit_s);
        }
}

// -------------------------------------------------------------------------

void gui_event_clear() { _common_event_clear(); }

static void _create_root() {
    gui_root = entity_create();
    transform_add(gui_root);
    transform_set_position(gui_root, luavec2(-1, 1));  // origin at top-left
    gui_rect_add(gui_root);
    gui_rect_set_hfit(gui_root, false);
    gui_rect_set_vfit(gui_root, false);
    gui_set_color(gui_root, color_clear);
    gui_set_captures_events(gui_root, false);
}

void gui_init() {
    PROFILE_FUNC();

    focused = entity_nil;
    _common_init();
    _rect_init();
    _text_init();
    font_init();
    _textedit_init();
    _create_root();
}

void gui_fini() {
    _textedit_fini();
    _text_fini();
    _rect_fini();
    _common_fini();
}

static void _update_root() {
    vec2 win_size;

    win_size = game_get_window_size();

    edit_set_editable(gui_root, false);

    // child of camera so GUI stays on screen
    transform_set_parent(gui_root, camera_get_current_camera());

    // use pixel coordinates
    transform_set_scale(gui_root, scalar_vec2_div(2, win_size));
    gui_rect_set_size(gui_root, win_size);
}

void gui_update_all() {
    _update_root();
    _common_update_destroyed();
    _common_update_visible();
    _common_reset_align();
    _textedit_update_all();
    _text_update_all();
    _rect_update_all();
    _common_update_align();
    _rect_update_wmat();
    _common_update_all();
}

void gui_draw_all() {
    _rect_draw_all();
    _text_draw_all();
    font_draw_all();
}

void gui_key_down(KeyCode key) {
    _common_key_down(key);
    _textedit_key_down(key);
}
void gui_char_down(unsigned int c) {
    _common_char_down(c);
    _textedit_char_down(c);
}
void gui_key_up(KeyCode key) { _common_key_up(key); }
void gui_mouse_down(MouseCode mouse) { _common_mouse_down(mouse); }
void gui_mouse_up(MouseCode mouse) { _common_mouse_up(mouse); }

void gui_save_all(Store *s) {
    _common_save_all(s);
    _rect_save_all(s);
    _text_save_all(s);
    _textedit_save_all(s);
}
void gui_load_all(Store *s) {
    _common_load_all(s);
    _rect_load_all(s);
    _text_load_all(s);
    _textedit_load_all(s);
}

void imgui_init() {
    PROFILE_FUNC();
    // CVAR_REF(conf_imgui_font, String);
    // if (neko_strlen(conf_imgui_font.data.str) > 0) {
    //     auto& io = ImGui::GetIO();
    //     ImFontConfig config;
    //     // config.PixelSnapH = 1;
    //     String ttf_file;
    //     vfs_read_entire_file(&ttf_file, conf_imgui_font.data.str);
    //     // neko_defer(mem_free(ttf_file.data));
    //     // void *ttf_data = ::mem_alloc(ttf_file.len);  // TODO:: imgui 内存方法接管
    //     // memcpy(ttf_data, ttf_file.data, ttf_file.len);
    //     io.Fonts->AddFontFromMemoryTTF(ttf_file.data, ttf_file.len, 12.0f, &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    // }
}
