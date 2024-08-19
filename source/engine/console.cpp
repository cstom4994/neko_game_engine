#include "console.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/base.h"
#include "engine/game.h"
#include "engine/input.h"
#include "engine/transform.h"
#include "engine/ui.h"

#define LINE_LEN 128  // including newline, null char
#define NUM_LINES 20

// circular buffer of lines, 'top' is current top line
static char lines[NUM_LINES][LINE_LEN] = {{0}};
static int top = 0;

// text that displays console contents
static Entity text;

static void _update_text() {
    unsigned int i;
    char *buf, *c, *r;

    // entity exists?
    if (entity_eq(text, entity_nil)) return;

    // accumulate non-empty lines and set text string

    buf = (char*)mem_alloc(NUM_LINES * LINE_LEN);
    for (i = 1, c = buf; i <= NUM_LINES; ++i)
        for (r = lines[(top + i) % NUM_LINES]; *r; ++r) *c++ = *r;
    *c = '\0';

    gui_text_set_str(text, buf);

    mem_free(buf);
}

void console_set_entity(Entity ent) {
    text = ent;
    gui_text_add(text);
    _update_text();
}
Entity console_get_entity() { return text; }

void console_set_visible(bool visible) {
    if (!entity_eq(text, entity_nil)) gui_set_visible(transform_get_parent(text), visible);
}
bool console_get_visible() {
    if (!entity_eq(text, entity_nil)) return gui_get_visible(transform_get_parent(text));
    return false;
}

// write a string to console with wrapping
static void _write(const char* s) {
    static unsigned int curs = 0;  // cursor position
    static const char wrap_prefix[] = {26, ' ', '\0'};
    unsigned int width, tabstop;
    char c;
    bool wrap;

    // wrap at window width, but max out at alloc'd size
    width = game_get_window_size().x / 10;
    if (width > LINE_LEN - 2) width = LINE_LEN - 2;

    while (*s) {
        c = *s;

        // tab? jump to tabstop
        if (c == '\t') {
            c = ' ';
            tabstop = 4 * ((curs + 4) / 4);
            if (tabstop >= width) {
                // wrap at tab?
                curs = width;
                ++s;
            } else
                for (; curs < tabstop - 1 && curs < width; ++curs) lines[top][curs] = c;
        }

        // write char
        wrap = curs >= width && c != '\n';
        c = wrap ? '\n' : c;
        lines[top][curs++] = c;

        // if newline, close this line and go to next
        if (c == '\n') {
            lines[top][curs] = '\0';

            top = (top + 1) % NUM_LINES;
            curs = 0;
        }

        // add a prefix to wrapped lines
        if (wrap)
            _write(wrap_prefix);
        else
            ++s;
    }

    // close this line
    lines[top][curs] = '\0';

    _update_text();
}

// write a string to both stdout and the console
static void _print(const char* s) {
    // copy to stdout
    printf("%s", s);
    fflush(stdout);

    // write it
    _write(s);
}

void console_puts(const char* s) {
    _print(s);
    _print("\n");
}

void console_printf(const char* fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char* s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char*)mem_alloc(n + 1);
    vsprintf(s, fmt, ap1);
    va_end(ap1);
    _print(s);
    mem_free(s);
}

void console_init() {
    PROFILE_FUNC();

    text = entity_nil;
}

void console_fini() {}

void neko_console_printf(neko_console_t* console, const char* fmt, ...);

void neko_console(neko_console_t* console, ui_context_t* ctx, ui_rect_t* rect, const ui_selector_desc_t* desc) {
    ui_rect_t screen = *rect;
    if (console->open)
        console->y += (screen.h * console->size - console->y) * console->open_speed;
    else if (!console->open && console->y >= 1.0f)
        console->y += (0 - console->y) * console->close_speed;
    else
        return;

    const f32 sz = NEKO_MIN(console->y, 26);
    if (ui_window_begin_ex(ctx, "neko_console_content", ui_rect(screen.x, screen.y, screen.w, console->y - sz), NULL, NULL,
                           UI_OPT_FORCESETRECT | UI_OPT_NOTITLE | UI_OPT_NORESIZE | UI_OPT_NODOCK | UI_OPT_FORCEFOCUS | UI_OPT_HOLDFOCUS)) {
        ui_layout_row(ctx, 1, ui_widths(-1), 0);
        ui_text(ctx, console->tb);
        // neko_imgui_draw_text(console->tb, NEKO_COLOR_WHITE, 10.f, 10.f, true, NEKO_COLOR_BLACK);
        if (console->autoscroll) ui_get_current_container(ctx)->scroll.y = sizeof(console->tb) * 7 + 100;
        ui_container_t* ctn = ui_get_current_container(ctx);
        ui_bring_to_front(ctx, ctn);
        ui_window_end(ctx);
    }

    if (ui_window_begin_ex(ctx, "neko_console_input", ui_rect(screen.x, screen.y + console->y - sz, screen.w, sz), NULL, NULL,
                           UI_OPT_FORCESETRECT | UI_OPT_NOTITLE | UI_OPT_NORESIZE | UI_OPT_NODOCK | UI_OPT_NOHOVER | UI_OPT_NOINTERACT)) {
        int len = strlen(console->cb[0]);
        ui_layout_row(ctx, 3, ui_widths(14, len * 7 + 2, 10), 0);
        ui_text(ctx, "$>");
        ui_text(ctx, console->cb[0]);

        i32 n;

        if (!console->open || !console->last_open_state) {
            goto console_input_handling_done;
        }

        // 处理文本输入
        n = NEKO_MIN(sizeof(*console->cb) - len - 1, (int32_t)strlen(ctx->input_text));

        if (input_key_down(KC_UP)) {
            console->current_cb_idx++;
            if (console->current_cb_idx >= NEKO_ARR_SIZE(console->cb)) {
                console->current_cb_idx = NEKO_ARR_SIZE(console->cb) - 1;
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (input_key_down(KC_DOWN)) {
            console->current_cb_idx--;
            if (console->current_cb_idx <= 0) {
                console->current_cb_idx = 0;
                memset(&console->cb[0], 0, sizeof(*console->cb));
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (input_key_down(KC_ENTER)) {
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
                char** argv = (char**)mem_alloc(argc * sizeof(char*));
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
                mem_free(argv);
            }
        } else if (input_key_down(KC_BACKSPACE)) {
            console->current_cb_idx = 0;
            // 跳过 utf-8 连续字节
            while ((console->cb[0][--len] & 0xc0) == 0x80 && len > 0);
            console->cb[0][len] = '\0';
        } else if (n > 0 && !input_key_down(KC_GRAVE_ACCENT)) {
            console->current_cb_idx = 0;
            if (len + n + 1 < sizeof(*console->cb)) {
                memcpy(console->cb[0] + len, ctx->input_text, n);
                len += n;
                console->cb[0][len] = '\0';
            }
        }

    console_input_handling_done:

        // 闪烁光标
        ui_get_layout(ctx)->body.x += len * 7 - 5;
        if ((int)(timing_get_elapsed() / 666.0f) & 1) ui_text(ctx, "|");

        ui_container_t* ctn = ui_get_current_container(ctx);
        ui_bring_to_front(ctx, ctn);

        ui_window_end(ctx);
    }

    console->last_open_state = console->open;
}

void neko_console_printf(neko_console_t* console, const char* fmt, ...) {
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

static bool window = 1, embeded;
static int summons;

static void toggle_window(int argc, char** argv);
static void toggle_embedded(int argc, char** argv);
static void help(int argc, char** argv);
static void echo(int argc, char** argv);
static void spam(int argc, char** argv);
static void crash(int argc, char** argv);
void summon(int argc, char** argv);
void exec(int argc, char** argv);
void sz(int argc, char** argv);

neko_console_command_t commands[] = {{
                                             .func = echo,
                                             .name = "echo",
                                             .desc = "repeat what was entered",
                                     },
                                     {
                                             .func = spam,
                                             .name = "spam",
                                             .desc = "send the word arg1, arg2 amount of times",
                                     },
                                     {
                                             .func = help,
                                             .name = "help",
                                             .desc = "sends a list of commands",
                                     },
                                     {
                                             .func = toggle_window,
                                             .name = "window",
                                             .desc = "toggles gui window",
                                     },
                                     {
                                             .func = toggle_embedded,
                                             .name = "embed",
                                             .desc = "places the console inside the window",
                                     },
                                     {
                                             .func = summon,
                                             .name = "summon",
                                             .desc = "summons a gui window",
                                     },
                                     {
                                             .func = sz,
                                             .name = "sz",
                                             .desc = "change console size",
                                     },
                                     {
                                             .func = crash,
                                             .name = "crash",
                                             .desc = "test crashhhhhhhhh.....",
                                     },
                                     {
                                             .func = exec,
                                             .name = "exec",
                                             .desc = "run nekoscript",
                                     }};

neko_console_t g_console = {
        .tb = "",
        .cb = {},
        .size = 0.4,
        .open_speed = 0.2,
        .close_speed = 0.3,
        .autoscroll = true,
        .commands = commands,
        .commands_len = NEKO_ARR_SIZE(commands),
};

void sz(int argc, char** argv) {
    if (argc != 2) {
        neko_console_printf(&g_console, "[sz]: needs 1 argument!\n");
        return;
    }
    f32 sz = atof(argv[1]);
    if (sz > 1 || sz < 0) {
        neko_console_printf(&g_console, "[sz]: number needs to be between (0, 1)");
        return;
    }
    g_console.size = sz;

    neko_console_printf(&g_console, "console size is now %f\n", sz);
}

void toggle_window(int argc, char** argv) {
    if (window && embeded)
        neko_console_printf(&g_console, "Unable to turn off window, console is embeded!\n");
    else
        neko_console_printf(&g_console, "GUI Window turned %s\n", (window = !window) ? "on" : "off");
}

void toggle_embedded(int argc, char** argv) {
    if (!window && !embeded)
        neko_console_printf(&g_console, "Unable to embed into window, open window first!\n");
    else
        neko_console_printf(&g_console, "console embedded turned %s\n", (embeded = !embeded) ? "on" : "off");
}

void summon(int argc, char** argv) {
    neko_console_printf(&g_console, "A summoner has cast his spell! A window has appeared!!!!\n");
    summons++;
}

void crash(int argc, char** argv) {
    // const_str trace_info = neko_os_stacktrace();
    // neko_os_msgbox(std::format("Crash...\n{0}", trace_info).c_str());
}

void spam(int argc, char** argv) {
    int count;
    if (argc != 3) goto spam_invalid_command;
    count = atoi(argv[2]);
    if (!count) goto spam_invalid_command;
    while (count--) neko_console_printf(&g_console, "%s\n", argv[1]);
    return;
spam_invalid_command:
    neko_console_printf(&g_console, "[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

void echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) neko_console_printf(&g_console, "%s ", argv[i]);
    neko_console_printf(&g_console, "\n");
}

void exec(int argc, char** argv) {
    if (argc != 2) return;
    // neko_vm_interpreter(argv[1]);
}

void help(int argc, char** argv) {
    for (int i = 0; i < NEKO_ARR_SIZE(commands); i++) {
        if (commands[i].name) neko_console_printf(&g_console, "* Command: %s\n", commands[i].name);
        if (commands[i].desc) neko_console_printf(&g_console, "- desc: %s\n", commands[i].desc);
    }
}