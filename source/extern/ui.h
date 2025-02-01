/*
** Copyright (c) 2024 rxi
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See `microui.c` for details.
*/

#ifndef MICROUI_H
#define MICROUI_H

#include "base/common/base.hpp"
#include "base/common/color.hpp"
#include "base/common/math.hpp"

using namespace Neko;

#define UI_VERSION "2.02"

#define UI_COMMANDLIST_SIZE (256 * 1024)
#define UI_ROOTLIST_SIZE 32
#define UI_CONTAINERSTACK_SIZE 32
#define UI_CLIPSTACK_SIZE 32
#define UI_IDSTACK_SIZE 32
#define UI_LAYOUTSTACK_SIZE 16
#define UI_CONTAINERPOOL_SIZE 48
#define UI_TREENODEPOOL_SIZE 48
#define UI_MAX_WIDTHS 16
#define UI_REAL float
#define UI_REAL_FMT "%.3g"
#define UI_SLIDER_FMT "%.2f"
#define UI_MAX_FMT 127

#define ui_stack(T, n) \
    struct {           \
        int idx;       \
        T items[n];    \
    }
#define ui_min(a, b) ((a) < (b) ? (a) : (b))
#define ui_max(a, b) ((a) > (b) ? (a) : (b))
#define ui_clamp(x, a, b) ui_min(b, ui_max(a, x))

enum { UI_CLIP_PART = 1, UI_CLIP_ALL };

enum { UI_COMMAND_JUMP = 1, UI_COMMAND_CLIP, UI_COMMAND_RECT, UI_COMMAND_TEXT, UI_COMMAND_ICON, UI_COMMAND_MAX };

enum {
    UI_COLOR_TEXT,
    UI_COLOR_BORDER,
    UI_COLOR_WINDOWBG,
    UI_COLOR_TITLEBG,
    UI_COLOR_TITLETEXT,
    UI_COLOR_PANELBG,
    UI_COLOR_BUTTON,
    UI_COLOR_BUTTONHOVER,
    UI_COLOR_BUTTONFOCUS,
    UI_COLOR_BASE,
    UI_COLOR_BASEHOVER,
    UI_COLOR_BASEFOCUS,
    UI_COLOR_SCROLLBASE,
    UI_COLOR_SCROLLTHUMB,
    UI_COLOR_MAX
};

enum { UI_ICON_CLOSE = 1, UI_ICON_CHECK, UI_ICON_COLLAPSED, UI_ICON_EXPANDED, UI_ICON_MAX };

enum { UI_RES_ACTIVE = (1 << 0), UI_RES_SUBMIT = (1 << 1), UI_RES_CHANGE = (1 << 2) };

enum {
    UI_OPT_ALIGNCENTER = (1 << 0),
    UI_OPT_ALIGNRIGHT = (1 << 1),
    UI_OPT_NOINTERACT = (1 << 2),
    UI_OPT_NOFRAME = (1 << 3),
    UI_OPT_NORESIZE = (1 << 4),
    UI_OPT_NOSCROLL = (1 << 5),
    UI_OPT_NOCLOSE = (1 << 6),
    UI_OPT_NOTITLE = (1 << 7),
    UI_OPT_HOLDFOCUS = (1 << 8),
    UI_OPT_AUTOSIZE = (1 << 9),
    UI_OPT_POPUP = (1 << 10),
    UI_OPT_CLOSED = (1 << 11),
    UI_OPT_EXPANDED = (1 << 12)
};

enum { UI_MOUSE_LEFT = (1 << 0), UI_MOUSE_RIGHT = (1 << 1), UI_MOUSE_MIDDLE = (1 << 2) };

enum { UI_KEY_SHIFT = (1 << 0), UI_KEY_CTRL = (1 << 1), UI_KEY_ALT = (1 << 2), UI_KEY_BACKSPACE = (1 << 3), UI_KEY_RETURN = (1 << 4) };

typedef struct ui_context_t ui_context_t;
typedef unsigned ui_id;
typedef UI_REAL ui_real;
typedef void *ui_font;

typedef struct {
    ui_id id;
    int last_update;
} ui_PoolItem;

typedef struct {
    int type, size;
} ui_BaseCommand;
typedef struct {
    ui_BaseCommand base;
    void *dst;
} ui_JumpCommand;
typedef struct {
    ui_BaseCommand base;
    rect_t rect;
} ui_ClipCommand;
typedef struct {
    ui_BaseCommand base;
    rect_t rect;
    Color256 color;
} ui_RectCommand;
typedef struct {
    ui_BaseCommand base;
    ui_font font;
    vec2 pos;
    Color256 color;
    char str[1];
} ui_TextCommand;
typedef struct {
    ui_BaseCommand base;
    rect_t rect;
    int id;
    Color256 color;
} ui_IconCommand;

typedef union {
    int type;
    ui_BaseCommand base;
    ui_JumpCommand jump;
    ui_ClipCommand clip;
    ui_RectCommand rect;
    ui_TextCommand text;
    ui_IconCommand icon;
} ui_Command;

typedef struct {
    rect_t body;
    rect_t next;
    vec2 position;
    vec2 size;
    vec2 max;
    int widths[UI_MAX_WIDTHS];
    int items;
    int item_index;
    int next_row;
    int next_type;
    int indent;
} ui_Layout;

typedef struct {
    ui_Command *head, *tail;
    rect_t rect;
    rect_t body;
    vec2 content_size;
    vec2 scroll;
    int zindex;
    int open;
} ui_Container;

typedef struct {
    ui_font font;
    vec2 size;
    int padding;
    int spacing;
    int indent;
    int title_height;
    int scrollbar_size;
    int thumb_size;
    Color256 colors[UI_COLOR_MAX];
} ui_Style;

struct ui_context_t {
    /* callbacks */
    int (*text_width)(ui_font font, const char *str, int len);
    int (*text_height)(ui_font font);
    void (*draw_frame)(ui_context_t *ctx, rect_t rect, int colorid);
    /* core state */
    ui_Style _style;
    ui_Style *style;
    ui_id hover;
    ui_id focus;
    ui_id last_id;
    rect_t last_rect;
    int last_zindex;
    int updated_focus;
    int frame;
    ui_Container *hover_root;
    ui_Container *next_hover_root;
    ui_Container *scroll_target;
    char number_edit_buf[UI_MAX_FMT];
    ui_id number_edit;
    /* stacks */
    ui_stack(char, UI_COMMANDLIST_SIZE) command_list;
    ui_stack(ui_Container *, UI_ROOTLIST_SIZE) root_list;
    ui_stack(ui_Container *, UI_CONTAINERSTACK_SIZE) container_stack;
    ui_stack(rect_t, UI_CLIPSTACK_SIZE) clip_stack;
    ui_stack(ui_id, UI_IDSTACK_SIZE) id_stack;
    ui_stack(ui_Layout, UI_LAYOUTSTACK_SIZE) layout_stack;
    /* retained state pools */
    ui_PoolItem container_pool[UI_CONTAINERPOOL_SIZE];
    ui_Container containers[UI_CONTAINERPOOL_SIZE];
    ui_PoolItem treenode_pool[UI_TREENODEPOOL_SIZE];
    /* input state */
    vec2 mouse_pos;
    vec2 last_mouse_pos;
    vec2 mouse_delta;
    vec2 scroll_delta;
    int mouse_down;
    int mouse_pressed;
    int key_down;
    int key_pressed;
    char input_text[32];
};

vec2 ui_vec2(int x, int y);
Color256 ui_color(int r, int g, int b, int a);

void ui_init(ui_context_t *ctx);
void ui_begin(ui_context_t *ctx);
void ui_end(ui_context_t *ctx);
void ui_set_focus(ui_context_t *ctx, ui_id id);
ui_id ui_get_id(ui_context_t *ctx, const void *data, size_t size);
void ui_push_id(ui_context_t *ctx, const void *data, size_t size);
void ui_pop_id(ui_context_t *ctx);
void ui_push_clip_rect(ui_context_t *ctx, rect_t rect);
void ui_pop_clip_rect(ui_context_t *ctx);
rect_t ui_get_clip_rect(ui_context_t *ctx);
int ui_check_clip(ui_context_t *ctx, rect_t r);
ui_Container *ui_get_current_container(ui_context_t *ctx);
ui_Container *ui_get_container(ui_context_t *ctx, const char *name);
void ui_bring_to_front(ui_context_t *ctx, ui_Container *cnt);

int ui_pool_init(ui_context_t *ctx, ui_PoolItem *items, int len, ui_id id);
int ui_pool_get(ui_context_t *ctx, ui_PoolItem *items, int len, ui_id id);
void ui_pool_update(ui_context_t *ctx, ui_PoolItem *items, int idx);

void ui_input_mousemove(ui_context_t *ctx, int x, int y);
void ui_input_mousedown(ui_context_t *ctx, int x, int y, int btn);
void ui_input_mouseup(ui_context_t *ctx, int x, int y, int btn);
void ui_input_scroll(ui_context_t *ctx, int x, int y);
void ui_input_keydown(ui_context_t *ctx, int key);
void ui_input_keyup(ui_context_t *ctx, int key);
void ui_input_text(ui_context_t *ctx, const char *text);

ui_Command *ui_push_command(ui_context_t *ctx, int type, size_t size);
int ui_next_command(ui_context_t *ctx, ui_Command **cmd);
void ui_set_clip(ui_context_t *ctx, rect_t rect);
void ui_draw_rect(ui_context_t *ctx, rect_t rect, Color256 color);
void ui_draw_box(ui_context_t *ctx, rect_t rect, Color256 color);
void ui_draw_text(ui_context_t *ctx, ui_font font, const char *str, int len, vec2 pos, Color256 color);
void ui_draw_icon(ui_context_t *ctx, int id, rect_t rect, Color256 color);

void ui_layout_row(ui_context_t *ctx, int items, const int *widths, int height);
void ui_layout_width(ui_context_t *ctx, int width);
void ui_layout_height(ui_context_t *ctx, int height);
void ui_layout_begin_column(ui_context_t *ctx);
void ui_layout_end_column(ui_context_t *ctx);
void ui_layout_set_next(ui_context_t *ctx, rect_t r, int relative);
rect_t ui_layout_next(ui_context_t *ctx);

void ui_draw_control_frame(ui_context_t *ctx, ui_id id, rect_t rect, int colorid, int opt);
void ui_draw_control_text(ui_context_t *ctx, const char *str, rect_t rect, int colorid, int opt);
int ui_mouse_over(ui_context_t *ctx, rect_t rect);
void ui_update_control(ui_context_t *ctx, ui_id id, rect_t rect, int opt);

#define ui_button(ctx, label) ui_button_ex(ctx, label, 0, UI_OPT_ALIGNCENTER)
#define ui_textbox(ctx, buf, bufsz) ui_textbox_ex(ctx, buf, bufsz, 0)
#define ui_slider(ctx, value, lo, hi) ui_slider_ex(ctx, value, lo, hi, 0, UI_SLIDER_FMT, UI_OPT_ALIGNCENTER)
#define ui_number(ctx, value, step) ui_number_ex(ctx, value, step, UI_SLIDER_FMT, UI_OPT_ALIGNCENTER)
#define ui_header(ctx, label) ui_header_ex(ctx, label, 0)
#define ui_begin_treenode(ctx, label) ui_begin_treenode_ex(ctx, label, 0)
#define ui_begin_window(ctx, title, rect) ui_begin_window_ex(ctx, title, rect, 0)
#define ui_begin_panel(ctx, name) ui_begin_panel_ex(ctx, name, 0)

void ui_text(ui_context_t *ctx, const char *text);
void ui_label(ui_context_t *ctx, const char *text);
int ui_button_ex(ui_context_t *ctx, const char *label, int icon, int opt);
int ui_checkbox(ui_context_t *ctx, const char *label, int *state);
int ui_textbox_raw(ui_context_t *ctx, char *buf, int bufsz, ui_id id, rect_t r, int opt);
int ui_textbox_ex(ui_context_t *ctx, char *buf, int bufsz, int opt);
int ui_slider_ex(ui_context_t *ctx, ui_real *value, ui_real low, ui_real high, ui_real step, const char *fmt, int opt);
int ui_number_ex(ui_context_t *ctx, ui_real *value, ui_real step, const char *fmt, int opt);
int ui_header_ex(ui_context_t *ctx, const char *label, int opt);
int ui_begin_treenode_ex(ui_context_t *ctx, const char *label, int opt);
void ui_end_treenode(ui_context_t *ctx);
int ui_begin_window_ex(ui_context_t *ctx, const char *title, rect_t rect, int opt);
void ui_end_window(ui_context_t *ctx);
void ui_open_popup(ui_context_t *ctx, const char *name);
int ui_begin_popup(ui_context_t *ctx, const char *name);
void ui_end_popup(ui_context_t *ctx);
void ui_begin_panel_ex(ui_context_t *ctx, const char *name, int opt);
void ui_end_panel(ui_context_t *ctx);

#endif
