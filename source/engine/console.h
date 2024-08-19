#ifndef CONSOLE_H
#define CONSOLE_H

#include "engine/base.h"
#include "engine/ecs.h"
#include "engine/math.h"
#include "engine/prelude.h"

NEKO_SCRIPT(console,

            // 打印到此实体 添加到 gui_text 系统 (如果尚未在其中) 设置为entity_nil以禁用
            NEKO_EXPORT void console_set_entity(Entity ent);

            NEKO_EXPORT Entity console_get_entity();  // 如果没有设置则entity_nil

            NEKO_EXPORT void console_set_visible(bool visible);

            NEKO_EXPORT bool console_get_visible();

            NEKO_EXPORT void console_puts(const char* s);

            NEKO_EXPORT void console_printf(const char* fmt, ...);

)

void console_init();
void console_fini();

/*=============================
// Console
=============================*/

typedef void (*neko_console_func)(int argc, char** argv);

typedef struct neko_console_command_t {
    neko_console_func func;
    const char* name;
    const char* desc;
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

    neko_console_command_t* commands;
    int commands_len;
} neko_console_t;

extern neko_console_t g_console;

struct ui_context_t;
struct ui_selector_desc_t;
struct ui_rect_t;

void neko_console(neko_console_t* console, ui_context_t* ctx, ui_rect_t *rect, const ui_selector_desc_t* desc);

#endif
