#ifndef NEKO_CONSOLE_H
#define NEKO_CONSOLE_H

#include "engine/util/neko_gui.h"

// NEKO_API_DECL void neko_console_printf(neko_console_t* console, const char* fmt, ...);
// NEKO_API_DECL void neko_console(neko_console_t* console, neko_imgui_context_t* ctx, neko_imgui_rect_t screen, const neko_imgui_selector_desc_t* desc);

NEKO_API_DECL neko_inline void neko_console_printf(neko_console_t* console, const char* fmt, ...) {
    char tmp[512] = {0};
    va_list args;

    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    int n = sizeof(console->tb) - strlen(console->tb) - 1;
    int resize = strlen(tmp) - n;
    if (resize > 0) {
        memmove(console->tb, console->tb + resize, sizeof(console->tb) - resize);
        n = sizeof(console->tb) - strlen(console->tb) - 1;
    }
    strncat(console->tb, tmp, n);
}

#ifdef NEKO_CONSOLE_IMPL

NEKO_API_DECL void neko_console(neko_console_t* console, neko_imgui_context_t* ctx, neko_imgui_rect_t screen, const neko_imgui_selector_desc_t* desc) {
    if (console->open)
        console->y += (screen.h * console->size - console->y) * console->open_speed;
    else if (!console->open && console->y >= 1.0f)
        console->y += (0 - console->y) * console->close_speed;
    else
        return;

    const f32 sz = neko_min(console->y, 26);
    if (neko_imgui_window_begin_ex(ctx, "neko_console_content", neko_imgui_rect(screen.x, screen.y, screen.w, console->y - sz), NULL, NULL,
                                   NEKO_IMGUI_OPT_FORCESETRECT | NEKO_IMGUI_OPT_NOTITLE | NEKO_IMGUI_OPT_NORESIZE | NEKO_IMGUI_OPT_NODOCK | NEKO_IMGUI_OPT_FORCEFOCUS | NEKO_IMGUI_OPT_HOLDFOCUS)) {
        neko_imgui_layout_row(ctx, 1, neko_imgui_widths(-1), 0);
        neko_imgui_text(ctx, console->tb);
        if (console->autoscroll) neko_imgui_get_current_container(ctx)->scroll.y = sizeof(console->tb) * 7 + 100;
        neko_imgui_container_t* ctn = neko_imgui_get_current_container(ctx);
        neko_imgui_bring_to_front(ctx, ctn);
        neko_imgui_window_end(ctx);
    }

    if (neko_imgui_window_begin_ex(ctx, "neko_console_input", neko_imgui_rect(screen.x, screen.y + console->y - sz, screen.w, sz), NULL, NULL,
                                   NEKO_IMGUI_OPT_FORCESETRECT | NEKO_IMGUI_OPT_NOTITLE | NEKO_IMGUI_OPT_NORESIZE | NEKO_IMGUI_OPT_NODOCK | NEKO_IMGUI_OPT_NOHOVER | NEKO_IMGUI_OPT_NOINTERACT)) {
        int len = strlen(console->cb[0]);
        neko_imgui_layout_row(ctx, 3, neko_imgui_widths(14, len * 7 + 2, 10), 0);
        neko_imgui_text(ctx, "$>");
        neko_imgui_text(ctx, console->cb[0]);

        if (!console->open || !console->last_open_state) goto console_input_handling_done;

        // 处理文本输入
        int32_t n = neko_min(sizeof(*console->cb) - len - 1, (int32_t)strlen(ctx->input_text));

        if (neko_platform_key_pressed(NEKO_KEYCODE_UP)) {
            console->current_cb_idx++;
            if (console->current_cb_idx >= neko_arr_size(console->cb)) {
                console->current_cb_idx = neko_arr_size(console->cb) - 1;
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_DOWN)) {
            console->current_cb_idx--;
            if (console->current_cb_idx <= 0) {
                console->current_cb_idx = 0;
                memset(&console->cb[0], 0, sizeof(*console->cb));
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_ENTER)) {
            console->current_cb_idx = 0;
            neko_console_printf(console, "$ %s\n", console->cb[0]);

            memmove((uint8_t*)console->cb + sizeof(*console->cb), (uint8_t*)console->cb, sizeof(console->cb) - sizeof(*console->cb));

            if (console->cb[0][0] && console->commands) {
                char* tmp = console->cb[0];
                int argc = 1;
                while ((tmp = strchr(tmp, ' '))) {
                    argc++;
                    tmp++;
                }

                tmp = console->cb[0];
                char* last_pos = console->cb[0];
                char** argv = (char**)neko_safe_malloc(argc * sizeof(char*));
                int i = 0;
                while ((tmp = strchr(tmp, ' '))) {
                    *tmp = 0;
                    argv[i++] = last_pos;
                    last_pos = ++tmp;
                }
                argv[argc - 1] = last_pos;

                for (int i = 0; i < console->commands_len; i++) {
                    if (console->commands[i].name && console->commands[i].func && strcmp(argv[0], console->commands[i].name) == 0) {
                        console->commands[i].func(argc, argv);
                        goto console_command_found;
                    }
                }
                neko_console_printf(console, "[neko_console]: unrecognized command '%s'\n", argv[0]);
            console_command_found:
                console->cb[0][0] = '\0';
                neko_safe_free(argv);
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE)) {
            console->current_cb_idx = 0;
            // 跳过 utf-8 连续字节
            while ((console->cb[0][--len] & 0xc0) == 0x80 && len > 0)
                ;
            console->cb[0][len] = '\0';
        } else if (n > 0) {
            console->current_cb_idx = 0;
            if (len + n + 1 < sizeof(*console->cb)) {
                memcpy(console->cb[0] + len, ctx->input_text, n);
                len += n;
                console->cb[0][len] = '\0';
            }
        }

    console_input_handling_done:

        // 闪烁光标
        neko_imgui_get_layout(ctx)->body.x += len * 7 - 5;
        if ((int)(neko_platform_elapsed_time() / 666.0f) & 1) neko_imgui_text(ctx, "|");

        neko_imgui_container_t* ctn = neko_imgui_get_current_container(ctx);
        neko_imgui_bring_to_front(ctx, ctn);

        neko_imgui_window_end(ctx);
    }

    console->last_open_state = console->open;
}
#endif

#endif