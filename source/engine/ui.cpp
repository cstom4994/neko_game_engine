#include "engine/ui.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "base/scripting/lua_wrapper.hpp"
#include "extern/atlas.h"

// deps
#include "extern/stb_truetype.h"

using namespace Neko;

using namespace Neko::luabind;

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

engine_ui_renderer_t *neko_new_ui_renderer() {
    engine_ui_renderer_t *renderer = (engine_ui_renderer_t *)mem_alloc(sizeof(engine_ui_renderer_t));

    Asset ui_shader = {};
    bool ok = asset_load_kind(AssetKind_Shader, "shader/ui.glsl", &ui_shader);
    error_assert(ok);

    renderer->draw_call_count = 0;

    renderer->tex = 0;
    renderer->shader = ui_shader.shader.id;
    renderer->quad_count = 0;

    neko_vertex_buffer_t *buffer = neko_new_vertex_buffer((neko_vertex_buffer_flags_t)(NEKO_VERTEXBUFFER_DRAW_TRIANGLES | NEKO_VERTEXBUFFER_DYNAMIC_DRAW));

    renderer->font = neko_default_font();

    neko_bind_vertex_buffer_for_edit(buffer);
    neko_push_vertices(buffer, NULL, (9 * 4) * ui_renderer_max_quads);
    neko_push_indices(buffer, NULL, 6 * ui_renderer_max_quads);
    neko_configure_vertex_buffer(buffer, 0, 2, 9, 0); /* vec2 position */
    neko_configure_vertex_buffer(buffer, 1, 2, 9, 2); /* vec2 uv */
    neko_configure_vertex_buffer(buffer, 2, 4, 9, 4); /* vec4 color */
    //  布局指定渲染模式
    //  	0 = rectangle
    //   	1 = icon (using atlas)
    //  	2 = text (ascii)
    //  	3 = text (unicode)
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

    if (renderer->quad_count >= ui_renderer_max_quads || mode == 3) {
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

    neko_bind_texture({}, 0);
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
    //     rect_t src = {(f32)g->x0, (f32)g->y0, (f32)g->x1 - g->x0, (f32)g->y1 - g->y0};
    //     rect_t dst = {position.x + g->xoff, position.y + g->yoff, src.w, src.h};
    //     engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 2);
    //     position.x += g->xadvance;
    // }

    f32 x = position.x;
    f32 y = position.y;

    for (Rune r : UTF8(text)) {

        u32 tex_id = 0;
        f32 xx = x;
        f32 yy = y;
        u32 ch = r.charcode();
        stbtt_aligned_quad q = renderer->font->quad(&tex_id, &xx, &yy, 16.f, ch);

        u32 ch_tex_id = tex_id;

        f32 x1 = x + q.x0;
        f32 y1 = y + q.y0;
        f32 x2 = x + q.x1;
        f32 y2 = y + q.y1;

        f32 u1 = q.s0;
        f32 v1 = q.t0;
        f32 u2 = q.s1;
        f32 v2 = q.t1;

        f32 w = q.x1 - q.x0;
        f32 h = q.y1 - q.y0;

        renderer->tex = (renderer->tex << 32) | ch_tex_id;  // 前32bit 上一字贴图ID 后32bit 当前贴图ID

        rect_t src = {(f32)q.s0, (f32)q.t0, (f32)q.s1 - q.s0, (f32)q.t1 - q.t0};
        rect_t dst = {x1, y1, w, h};

        if (ch > 256) {
            if (renderer->quad_count > 0) {
                u32 last_tex = (u32)(renderer->tex >> 32);  // 前32bit 上一字贴图ID
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, last_tex);
                neko_flush_ui_renderer(renderer);
            }
            u32 tex = (u32)renderer->tex;  // 后32bit 当前贴图ID
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tex);
            engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 3);
        } else {
            u32 tex = (u32)renderer->tex;  // 后32bit 当前贴图ID
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tex);
            engine_ui_renderer_push_quad(renderer, dst, src, color, transparency, 2);
        }

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

float engine_ui_tect_height(engine_ui_renderer_t *renderer) { return -16.0f; }

void neko_set_ui_renderer_clip(engine_ui_renderer_t *renderer, rect_t rect) {
    neko_flush_ui_renderer(renderer);
    glScissor(rect.x, renderer->height - (rect.y + rect.h), rect.w, rect.h);
}

INPUT_WRAP_DEFINE(ui);

ui_context_t *ui_global_ctx() { return gApp->ui; }

void neko_init_ui_renderer() {
    ui_renderer = neko_new_ui_renderer();

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

static ui_context_t *ui_ctx() { return gApp->ui; }

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
