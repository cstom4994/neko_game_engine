#ifndef EDIT_H
#define EDIT_H

#include <stdbool.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/bootstrap.h"
#include "engine/entity.h"
#include "engine/graphics.h"
#include "engine/luax.hpp"

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location);
void inspect_shader(const char *label, GLuint program);
void inspect_vertex_array(const char *label, GLuint vao);

NEKO_SCRIPT(console,

            // 打印到此实体 添加到 gui_text 系统 (如果尚未在其中) 设置为entity_nil以禁用
            NEKO_EXPORT void console_set_entity(NativeEntity ent);

            NEKO_EXPORT NativeEntity console_get_entity();  // 如果没有设置则entity_nil

            NEKO_EXPORT void console_set_visible(bool visible);

            NEKO_EXPORT bool console_get_visible();

            NEKO_EXPORT void console_puts(const char *s);

            NEKO_EXPORT void console_printf(const char *fmt, ...);

)

void console_init();
void console_fini();

/*=============================
// Console
=============================*/

typedef void (*neko_console_func)(int argc, char **argv);

typedef struct neko_console_command_t {
    neko_console_func func;
    const char *name;
    const char *desc;
} neko_console_command_t;

typedef struct neko_console_t {
    char tb[2048];     // text buffer
    char cb[10][256];  // "command" buffer
    int current_cb_idx;

    f32 y;
    f32 size;
    f32 open_speed;
    f32 close_speed;

    bool open;
    int last_open_state;
    bool autoscroll;

    neko_console_command_t *commands;
    int commands_len;
} neko_console_t;

extern neko_console_t g_console;

struct ui_context_t;
struct ui_selector_desc_t;
struct rect_t;

void neko_console(neko_console_t *console, ui_context_t *ctx, rect_t *rect, const ui_selector_desc_t *desc);

#endif
