/*
** Copyright (c) 2024 rxi
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
*/

#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define unused(x) ((void)(x))

#define expect(x)                                                                                   \
    do {                                                                                            \
        if (!(x)) {                                                                                 \
            fprintf(stderr, "Fatal error: %s:%d: assertion '%s' failed\n", __FILE__, __LINE__, #x); \
            abort();                                                                                \
        }                                                                                           \
    } while (0)

#define push(stk, val)                                                         \
    do {                                                                       \
        expect((stk).idx < (int)(sizeof((stk).items) / sizeof(*(stk).items))); \
        (stk).items[(stk).idx] = (val);                                        \
        (stk).idx++; /* incremented after incase `val` uses this value */      \
    } while (0)

#define pop(stk)               \
    do {                       \
        expect((stk).idx > 0); \
        (stk).idx--;           \
    } while (0)

static rect_t unclipped_rect = {0, 0, 0x1000000, 0x1000000};

static ui_Style default_style = {
        /* font | size | padding | spacing | indent */
        NULL,
        {68, 10},
        5,
        4,
        24,
        /* title_height | scrollbar_size | thumb_size */
        24,
        12,
        8,
        {
                {230, 230, 230, 255}, /* UI_COLOR_TEXT */
                {25, 25, 25, 255},    /* UI_COLOR_BORDER */
                {50, 50, 50, 255},    /* UI_COLOR_WINDOWBG */
                {25, 25, 25, 255},    /* UI_COLOR_TITLEBG */
                {240, 240, 240, 255}, /* UI_COLOR_TITLETEXT */
                {0, 0, 0, 0},         /* UI_COLOR_PANELBG */
                {75, 75, 75, 255},    /* UI_COLOR_BUTTON */
                {95, 95, 95, 255},    /* UI_COLOR_BUTTONHOVER */
                {115, 115, 115, 255}, /* UI_COLOR_BUTTONFOCUS */
                {30, 30, 30, 255},    /* UI_COLOR_BASE */
                {35, 35, 35, 255},    /* UI_COLOR_BASEHOVER */
                {40, 40, 40, 255},    /* UI_COLOR_BASEFOCUS */
                {43, 43, 43, 255},    /* UI_COLOR_SCROLLBASE */
                {30, 30, 30, 255}     /* UI_COLOR_SCROLLTHUMB */
        }};

static rect_t expand_rect(rect_t rect, int n) { return neko_rect(rect.x - n, rect.y - n, rect.w + n * 2, rect.h + n * 2); }

static rect_t intersect_rects(rect_t r1, rect_t r2) {
    int x1 = ui_max(r1.x, r2.x);
    int y1 = ui_max(r1.y, r2.y);
    int x2 = ui_min(r1.x + r1.w, r2.x + r2.w);
    int y2 = ui_min(r1.y + r1.h, r2.y + r2.h);
    if (x2 < x1) {
        x2 = x1;
    }
    if (y2 < y1) {
        y2 = y1;
    }
    return neko_rect(x1, y1, x2 - x1, y2 - y1);
}

static int rect_overlaps_vec2(rect_t r, vec2 p) { return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h; }

static void draw_frame(ui_context_t *ctx, rect_t rect, int colorid) {
    ui_draw_rect(ctx, rect, ctx->style->colors[colorid]);
    if (colorid == UI_COLOR_SCROLLBASE || colorid == UI_COLOR_SCROLLTHUMB || colorid == UI_COLOR_TITLEBG) {
        return;
    }
    /* draw border */
    if (ctx->style->colors[UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, expand_rect(rect, 1), ctx->style->colors[UI_COLOR_BORDER]);
    }
}

void ui_init(ui_context_t *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->draw_frame = draw_frame;
    ctx->_style = default_style;
    ctx->style = &ctx->_style;
}

void ui_begin(ui_context_t *ctx) {
    expect(ctx->text_width && ctx->text_height);
    ctx->command_list.idx = 0;
    ctx->root_list.idx = 0;
    ctx->scroll_target = NULL;
    ctx->hover_root = ctx->next_hover_root;
    ctx->next_hover_root = NULL;
    ctx->mouse_delta.x = ctx->mouse_pos.x - ctx->last_mouse_pos.x;
    ctx->mouse_delta.y = ctx->mouse_pos.y - ctx->last_mouse_pos.y;
    ctx->frame++;
}

static int compare_zindex(const void *a, const void *b) { return (*(ui_Container **)a)->zindex - (*(ui_Container **)b)->zindex; }

void ui_end(ui_context_t *ctx) {
    int i, n;
    /* check stacks */
    expect(ctx->container_stack.idx == 0);
    expect(ctx->clip_stack.idx == 0);
    expect(ctx->id_stack.idx == 0);
    expect(ctx->layout_stack.idx == 0);

    /* handle scroll input */
    if (ctx->scroll_target) {
        ctx->scroll_target->scroll.x += ctx->scroll_delta.x;
        ctx->scroll_target->scroll.y += ctx->scroll_delta.y;
    }

    /* unset focus if focus id was not touched this frame */
    if (!ctx->updated_focus) {
        ctx->focus = 0;
    }
    ctx->updated_focus = 0;

    /* bring hover root to front if mouse was pressed */
    if (ctx->mouse_pressed && ctx->next_hover_root && ctx->next_hover_root->zindex < ctx->last_zindex && ctx->next_hover_root->zindex >= 0) {
        ui_bring_to_front(ctx, ctx->next_hover_root);
    }

    /* reset input state */
    ctx->key_pressed = 0;
    ctx->input_text[0] = '\0';
    ctx->mouse_pressed = 0;
    ctx->scroll_delta = neko_v2(0, 0);
    ctx->last_mouse_pos = ctx->mouse_pos;

    /* sort root containers by zindex */
    n = ctx->root_list.idx;
    qsort(ctx->root_list.items, n, sizeof(ui_Container *), compare_zindex);

    /* set root container jump commands */
    for (i = 0; i < n; i++) {
        ui_Container *cnt = ctx->root_list.items[i];
        /* if this is the first container then make the first command jump to it.
        ** otherwise set the previous container's tail to jump to this one */
        if (i == 0) {
            ui_Command *cmd = (ui_Command *)ctx->command_list.items;
            cmd->jump.dst = (char *)cnt->head + sizeof(ui_JumpCommand);
        } else {
            ui_Container *prev = ctx->root_list.items[i - 1];
            prev->tail->jump.dst = (char *)cnt->head + sizeof(ui_JumpCommand);
        }
        /* make the last container's tail jump to the end of command list */
        if (i == n - 1) {
            cnt->tail->jump.dst = ctx->command_list.items + ctx->command_list.idx;
        }
    }
}

void ui_set_focus(ui_context_t *ctx, ui_id id) {
    ctx->focus = id;
    ctx->updated_focus = 1;
}

/* 32bit fnv-1a hash */
#define HASH_INITIAL 2166136261

static void hash(ui_id *hash, const void *data, int size) {
    const unsigned char *p = (unsigned char *)data;
    while (size--) {
        *hash = (*hash ^ *p++) * 16777619;
    }
}

ui_id ui_get_id(ui_context_t *ctx, const void *data, size_t size) {
    int idx = ctx->id_stack.idx;
    ui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : HASH_INITIAL;
    hash(&res, data, size);
    ctx->last_id = res;
    return res;
}

void ui_push_id(ui_context_t *ctx, const void *data, size_t size) { push(ctx->id_stack, ui_get_id(ctx, data, size)); }

void ui_pop_id(ui_context_t *ctx) { pop(ctx->id_stack); }

void ui_push_clip_rect(ui_context_t *ctx, rect_t rect) {
    rect_t last = ui_get_clip_rect(ctx);
    push(ctx->clip_stack, intersect_rects(rect, last));
}

void ui_pop_clip_rect(ui_context_t *ctx) { pop(ctx->clip_stack); }

rect_t ui_get_clip_rect(ui_context_t *ctx) {
    expect(ctx->clip_stack.idx > 0);
    return ctx->clip_stack.items[ctx->clip_stack.idx - 1];
}

int ui_check_clip(ui_context_t *ctx, rect_t r) {
    rect_t cr = ui_get_clip_rect(ctx);
    if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h || r.y + r.h < cr.y) {
        return UI_CLIP_ALL;
    }
    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y && r.y + r.h <= cr.y + cr.h) {
        return 0;
    }
    return UI_CLIP_PART;
}

static void push_layout(ui_context_t *ctx, rect_t body, vec2 scroll) {
    ui_Layout layout;
    int width = 0;
    memset(&layout, 0, sizeof(layout));
    layout.body = neko_rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
    layout.max = neko_v2(-0x1000000, -0x1000000);
    push(ctx->layout_stack, layout);
    ui_layout_row(ctx, 1, &width, 0);
}

static ui_Layout *get_layout(ui_context_t *ctx) { return &ctx->layout_stack.items[ctx->layout_stack.idx - 1]; }

static void pop_container(ui_context_t *ctx) {
    ui_Container *cnt = ui_get_current_container(ctx);
    ui_Layout *layout = get_layout(ctx);
    cnt->content_size.x = layout->max.x - layout->body.x;
    cnt->content_size.y = layout->max.y - layout->body.y;
    /* pop container, layout and id */
    pop(ctx->container_stack);
    pop(ctx->layout_stack);
    ui_pop_id(ctx);
}

ui_Container *ui_get_current_container(ui_context_t *ctx) {
    expect(ctx->container_stack.idx > 0);
    return ctx->container_stack.items[ctx->container_stack.idx - 1];
}

static ui_Container *get_container(ui_context_t *ctx, ui_id id, int opt) {
    ui_Container *cnt;
    /* try to get existing container from pool */
    int idx = ui_pool_get(ctx, ctx->container_pool, UI_CONTAINERPOOL_SIZE, id);
    if (idx >= 0) {
        if (ctx->containers[idx].open || ~opt & UI_OPT_CLOSED) {
            ui_pool_update(ctx, ctx->container_pool, idx);
        }
        return &ctx->containers[idx];
    }
    if (opt & UI_OPT_CLOSED) {
        return NULL;
    }
    /* container not found in pool: init new container */
    idx = ui_pool_init(ctx, ctx->container_pool, UI_CONTAINERPOOL_SIZE, id);
    cnt = &ctx->containers[idx];
    memset(cnt, 0, sizeof(*cnt));
    cnt->open = 1;
    ui_bring_to_front(ctx, cnt);
    return cnt;
}

ui_Container *ui_get_container(ui_context_t *ctx, const char *name) {
    ui_id id = ui_get_id(ctx, name, strlen(name));
    return get_container(ctx, id, 0);
}

void ui_bring_to_front(ui_context_t *ctx, ui_Container *cnt) { cnt->zindex = ++ctx->last_zindex; }

/*============================================================================
** pool
**============================================================================*/

int ui_pool_init(ui_context_t *ctx, ui_PoolItem *items, int len, ui_id id) {
    int i, n = -1, f = ctx->frame;
    for (i = 0; i < len; i++) {
        if (items[i].last_update < f) {
            f = items[i].last_update;
            n = i;
        }
    }
    expect(n > -1);
    items[n].id = id;
    ui_pool_update(ctx, items, n);
    return n;
}

int ui_pool_get(ui_context_t *ctx, ui_PoolItem *items, int len, ui_id id) {
    int i;
    unused(ctx);
    for (i = 0; i < len; i++) {
        if (items[i].id == id) {
            return i;
        }
    }
    return -1;
}

void ui_pool_update(ui_context_t *ctx, ui_PoolItem *items, int idx) { items[idx].last_update = ctx->frame; }

/*============================================================================
** input handlers
**============================================================================*/

void ui_input_mousemove(ui_context_t *ctx, int x, int y) { ctx->mouse_pos = neko_v2(x, y); }

void ui_input_mousedown(ui_context_t *ctx, int x, int y, int btn) {
    ui_input_mousemove(ctx, x, y);
    ctx->mouse_down |= btn;
    ctx->mouse_pressed |= btn;
}

void ui_input_mouseup(ui_context_t *ctx, int x, int y, int btn) {
    ui_input_mousemove(ctx, x, y);
    ctx->mouse_down &= ~btn;
}

void ui_input_scroll(ui_context_t *ctx, int x, int y) {
    ctx->scroll_delta.x += x;
    ctx->scroll_delta.y += y;
}

void ui_input_keydown(ui_context_t *ctx, int key) {
    ctx->key_pressed |= key;
    ctx->key_down |= key;
}

void ui_input_keyup(ui_context_t *ctx, int key) { ctx->key_down &= ~key; }

void ui_input_text(ui_context_t *ctx, const char *text) {
    size_t len = strlen(ctx->input_text);
    size_t size = strlen(text) + 1;
    expect(len + size <= sizeof(ctx->input_text));
    memcpy(ctx->input_text + len, text, size);
}

/*============================================================================
** commandlist
**============================================================================*/

ui_Command *ui_push_command(ui_context_t *ctx, int type, size_t size) {
    ui_Command *cmd = (ui_Command *)(ctx->command_list.items + ctx->command_list.idx);
    expect(ctx->command_list.idx + size < UI_COMMANDLIST_SIZE);
    cmd->base.type = type;
    cmd->base.size = size;
    ctx->command_list.idx += size;
    return cmd;
}

int ui_next_command(ui_context_t *ctx, ui_Command **cmd) {
    if (*cmd) {
        *cmd = (ui_Command *)(((char *)*cmd) + (*cmd)->base.size);
    } else {
        *cmd = (ui_Command *)ctx->command_list.items;
    }
    while ((char *)*cmd != ctx->command_list.items + ctx->command_list.idx) {
        if ((*cmd)->type != UI_COMMAND_JUMP) {
            return 1;
        }
        *cmd = (ui_Command *)(*cmd)->jump.dst;
    }
    return 0;
}

static ui_Command *push_jump(ui_context_t *ctx, ui_Command *dst) {
    ui_Command *cmd;
    cmd = ui_push_command(ctx, UI_COMMAND_JUMP, sizeof(ui_JumpCommand));
    cmd->jump.dst = dst;
    return cmd;
}

void ui_set_clip(ui_context_t *ctx, rect_t rect) {
    ui_Command *cmd;
    cmd = ui_push_command(ctx, UI_COMMAND_CLIP, sizeof(ui_ClipCommand));
    cmd->clip.rect = rect;
}

void ui_draw_rect(ui_context_t *ctx, rect_t rect, Color256 color) {
    ui_Command *cmd;
    rect = intersect_rects(rect, ui_get_clip_rect(ctx));
    if (rect.w > 0 && rect.h > 0) {
        cmd = ui_push_command(ctx, UI_COMMAND_RECT, sizeof(ui_RectCommand));
        cmd->rect.rect = rect;
        cmd->rect.color = color;
    }
}

void ui_draw_box(ui_context_t *ctx, rect_t rect, Color256 color) {
    ui_draw_rect(ctx, neko_rect(rect.x + 1, rect.y, rect.w - 2, 1), color);
    ui_draw_rect(ctx, neko_rect(rect.x + 1, rect.y + rect.h - 1, rect.w - 2, 1), color);
    ui_draw_rect(ctx, neko_rect(rect.x, rect.y, 1, rect.h), color);
    ui_draw_rect(ctx, neko_rect(rect.x + rect.w - 1, rect.y, 1, rect.h), color);
}

void ui_draw_text(ui_context_t *ctx, ui_font font, const char *str, int len, vec2 pos, Color256 color) {
    ui_Command *cmd;
    rect_t rect = neko_rect(pos.x, pos.y, ctx->text_width(font, str, len), ctx->text_height(font));
    int clipped = ui_check_clip(ctx, rect);
    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }
    /* add command */
    if (len < 0) {
        len = strlen(str);
    }
    cmd = ui_push_command(ctx, UI_COMMAND_TEXT, sizeof(ui_TextCommand) + len);
    memcpy(cmd->text.str, str, len);
    cmd->text.str[len] = '\0';
    cmd->text.pos = pos;
    cmd->text.color = color;
    cmd->text.font = font;
    /* reset clipping if it was set */
    if (clipped) {
        ui_set_clip(ctx, unclipped_rect);
    }
}

void ui_draw_icon(ui_context_t *ctx, int id, rect_t rect, Color256 color) {
    ui_Command *cmd;
    /* do clip command if the rect isn't fully contained within the cliprect */
    int clipped = ui_check_clip(ctx, rect);
    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }
    /* do icon command */
    cmd = ui_push_command(ctx, UI_COMMAND_ICON, sizeof(ui_IconCommand));
    cmd->icon.id = id;
    cmd->icon.rect = rect;
    cmd->icon.color = color;
    /* reset clipping if it was set */
    if (clipped) {
        ui_set_clip(ctx, unclipped_rect);
    }
}

/*============================================================================
** layout
**============================================================================*/

enum { UI_RELATIVE = 1, UI_ABSOLUTE = 2 };

void ui_layout_begin_column(ui_context_t *ctx) { push_layout(ctx, ui_layout_next(ctx), neko_v2(0, 0)); }

void ui_layout_end_column(ui_context_t *ctx) {
    ui_Layout *a, *b;
    b = get_layout(ctx);
    pop(ctx->layout_stack);
    /* inherit position/next_row/max from child layout if they are greater */
    a = get_layout(ctx);
    a->position.x = ui_max(a->position.x, b->position.x + b->body.x - a->body.x);
    a->next_row = ui_max(a->next_row, b->next_row + b->body.y - a->body.y);
    a->max.x = ui_max(a->max.x, b->max.x);
    a->max.y = ui_max(a->max.y, b->max.y);
}

void ui_layout_row(ui_context_t *ctx, int items, const int *widths, int height) {
    ui_Layout *layout = get_layout(ctx);
    if (widths) {
        expect(items <= UI_MAX_WIDTHS);
        memcpy(layout->widths, widths, items * sizeof(widths[0]));
    }
    layout->items = items;
    layout->position = neko_v2(layout->indent, layout->next_row);
    layout->size.y = height;
    layout->item_index = 0;
}

void ui_layout_width(ui_context_t *ctx, int width) { get_layout(ctx)->size.x = width; }

void ui_layout_height(ui_context_t *ctx, int height) { get_layout(ctx)->size.y = height; }

void ui_layout_set_next(ui_context_t *ctx, rect_t r, int relative) {
    ui_Layout *layout = get_layout(ctx);
    layout->next = r;
    layout->next_type = relative ? UI_RELATIVE : UI_ABSOLUTE;
}

rect_t ui_layout_next(ui_context_t *ctx) {
    ui_Layout *layout = get_layout(ctx);
    ui_Style *style = ctx->style;
    rect_t res;

    if (layout->next_type) {
        /* handle rect set by `ui_layout_set_next` */
        int type = layout->next_type;
        layout->next_type = 0;
        res = layout->next;
        if (type == UI_ABSOLUTE) {
            return (ctx->last_rect = res);
        }

    } else {
        /* handle next row */
        if (layout->item_index == layout->items) {
            ui_layout_row(ctx, layout->items, NULL, layout->size.y);
        }

        /* position */
        res.x = layout->position.x;
        res.y = layout->position.y;

        /* size */
        res.w = layout->items > 0 ? layout->widths[layout->item_index] : layout->size.x;
        res.h = layout->size.y;
        if (res.w == 0) {
            res.w = style->size.x + style->padding * 2;
        }
        if (res.h == 0) {
            res.h = style->size.y + style->padding * 2;
        }
        if (res.w < 0) {
            res.w += layout->body.w - res.x + 1;
        }
        if (res.h < 0) {
            res.h += layout->body.h - res.y + 1;
        }

        layout->item_index++;
    }

    /* update position */
    layout->position.x += res.w + style->spacing;
    layout->next_row = ui_max(layout->next_row, res.y + res.h + style->spacing);

    /* apply body offset */
    res.x += layout->body.x;
    res.y += layout->body.y;

    /* update max position */
    layout->max.x = ui_max(layout->max.x, res.x + res.w);
    layout->max.y = ui_max(layout->max.y, res.y + res.h);

    return (ctx->last_rect = res);
}

/*============================================================================
** controls
**============================================================================*/

static int in_hover_root(ui_context_t *ctx) {
    int i = ctx->container_stack.idx;
    while (i--) {
        if (ctx->container_stack.items[i] == ctx->hover_root) {
            return 1;
        }
        /* only root containers have their `head` field set; stop searching if we've
        ** reached the current root container */
        if (ctx->container_stack.items[i]->head) {
            break;
        }
    }
    return 0;
}

void ui_draw_control_frame(ui_context_t *ctx, ui_id id, rect_t rect, int colorid, int opt) {
    if (opt & UI_OPT_NOFRAME) {
        return;
    }
    colorid += (ctx->focus == id) ? 2 : (ctx->hover == id) ? 1 : 0;
    ctx->draw_frame(ctx, rect, colorid);
}

void ui_draw_control_text(ui_context_t *ctx, const char *str, rect_t rect, int colorid, int opt) {
    vec2 pos;
    ui_font font = ctx->style->font;
    int tw = ctx->text_width(font, str, -1);
    ui_push_clip_rect(ctx, rect);
    pos.y = rect.y + (rect.h - ctx->text_height(font)) / 2;
    if (opt & UI_OPT_ALIGNCENTER) {
        pos.x = rect.x + (rect.w - tw) / 2;
    } else if (opt & UI_OPT_ALIGNRIGHT) {
        pos.x = rect.x + rect.w - tw - ctx->style->padding;
    } else {
        pos.x = rect.x + ctx->style->padding;
    }
    ui_draw_text(ctx, font, str, -1, pos, ctx->style->colors[colorid]);
    ui_pop_clip_rect(ctx);
}

int ui_mouse_over(ui_context_t *ctx, rect_t rect) { return rect_overlaps_vec2(rect, ctx->mouse_pos) && rect_overlaps_vec2(ui_get_clip_rect(ctx), ctx->mouse_pos) && in_hover_root(ctx); }

void ui_update_control(ui_context_t *ctx, ui_id id, rect_t rect, int opt) {
    int mouseover = ui_mouse_over(ctx, rect);

    if (ctx->focus == id) {
        ctx->updated_focus = 1;
    }
    if (opt & UI_OPT_NOINTERACT) {
        return;
    }
    if (mouseover && !ctx->mouse_down) {
        ctx->hover = id;
    }

    if (ctx->focus == id) {
        if (ctx->mouse_pressed && !mouseover) {
            ui_set_focus(ctx, 0);
        }
        if (!ctx->mouse_down && ~opt & UI_OPT_HOLDFOCUS) {
            ui_set_focus(ctx, 0);
        }
    }

    if (ctx->hover == id) {
        if (ctx->mouse_pressed) {
            ui_set_focus(ctx, id);
        } else if (!mouseover) {
            ctx->hover = 0;
        }
    }
}

void ui_text(ui_context_t *ctx, const char *text) {
    const char *start, *end, *p = text;
    int width = -1;
    ui_font font = ctx->style->font;
    Color256 color = ctx->style->colors[UI_COLOR_TEXT];
    ui_layout_begin_column(ctx);
    ui_layout_row(ctx, 1, &width, ctx->text_height(font));
    do {
        rect_t r = ui_layout_next(ctx);
        int w = 0;
        start = end = p;
        do {
            const char *word = p;
            while (*p && *p != ' ' && *p != '\n') {
                p++;
            }
            w += ctx->text_width(font, word, p - word);
            if (w > r.w && end != start) {
                break;
            }
            w += ctx->text_width(font, p, 1);
            end = p++;
        } while (*end && *end != '\n');
        ui_draw_text(ctx, font, start, end - start, neko_v2(r.x, r.y), color);
        p = end + 1;
    } while (*end);
    ui_layout_end_column(ctx);
}

void ui_label(ui_context_t *ctx, const char *text) { ui_draw_control_text(ctx, text, ui_layout_next(ctx), UI_COLOR_TEXT, 0); }

int ui_button_ex(ui_context_t *ctx, const char *label, int icon, int opt) {
    int res = 0;
    ui_id id = label ? ui_get_id(ctx, label, strlen(label)) : ui_get_id(ctx, &icon, sizeof(icon));
    rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt);
    /* handle click */
    if (ctx->mouse_pressed == UI_MOUSE_LEFT && ctx->focus == id) {
        res |= UI_RES_SUBMIT;
    }
    /* draw */
    ui_draw_control_frame(ctx, id, r, UI_COLOR_BUTTON, opt);
    if (label) {
        ui_draw_control_text(ctx, label, r, UI_COLOR_TEXT, opt);
    }
    if (icon) {
        ui_draw_icon(ctx, icon, r, ctx->style->colors[UI_COLOR_TEXT]);
    }
    return res;
}

int ui_checkbox(ui_context_t *ctx, const char *label, int *state) {
    int res = 0;
    ui_id id = ui_get_id(ctx, &state, sizeof(state));
    rect_t r = ui_layout_next(ctx);
    rect_t box = neko_rect(r.x, r.y, r.h, r.h);
    ui_update_control(ctx, id, r, 0);
    /* handle click */
    if (ctx->mouse_pressed == UI_MOUSE_LEFT && ctx->focus == id) {
        res |= UI_RES_CHANGE;
        *state = !*state;
    }
    /* draw */
    ui_draw_control_frame(ctx, id, box, UI_COLOR_BASE, 0);
    if (*state) {
        ui_draw_icon(ctx, UI_ICON_CHECK, box, ctx->style->colors[UI_COLOR_TEXT]);
    }
    r = neko_rect(r.x + box.w, r.y, r.w - box.w, r.h);
    ui_draw_control_text(ctx, label, r, UI_COLOR_TEXT, 0);
    return res;
}

int ui_textbox_raw(ui_context_t *ctx, char *buf, int bufsz, ui_id id, rect_t r, int opt) {
    int res = 0;
    ui_update_control(ctx, id, r, opt | UI_OPT_HOLDFOCUS);

    if (ctx->focus == id) {
        /* handle text input */
        size_t len = strlen(buf);
        size_t n = ui_min(bufsz - len - 1, strlen(ctx->input_text));
        if (n > 0) {
            memcpy(buf + len, ctx->input_text, n);
            len += n;
            buf[len] = '\0';
            res |= UI_RES_CHANGE;
        }
        /* handle backspace */
        if (ctx->key_pressed & UI_KEY_BACKSPACE && len > 0) {
            /* skip utf-8 continuation bytes */
            while ((buf[--len] & 0xc0) == 0x80 && len > 0)
                ;
            buf[len] = '\0';
            res |= UI_RES_CHANGE;
        }
        /* handle return */
        if (ctx->key_pressed & UI_KEY_RETURN) {
            ui_set_focus(ctx, 0);
            res |= UI_RES_SUBMIT;
        }
    }

    /* draw */
    ui_draw_control_frame(ctx, id, r, UI_COLOR_BASE, opt);
    if (ctx->focus == id) {
        Color256 color = ctx->style->colors[UI_COLOR_TEXT];
        ui_font font = ctx->style->font;
        int textw = ctx->text_width(font, buf, -1);
        int texth = ctx->text_height(font);
        int ofx = r.w - ctx->style->padding - textw - 1;
        int textx = r.x + ui_min(ofx, ctx->style->padding);
        int texty = r.y + (r.h - texth) / 2;
        ui_push_clip_rect(ctx, r);
        ui_draw_text(ctx, font, buf, -1, neko_v2(textx, texty), color);
        ui_draw_rect(ctx, neko_rect(textx + textw, texty, 1, texth), color);
        ui_pop_clip_rect(ctx);
    } else {
        ui_draw_control_text(ctx, buf, r, UI_COLOR_TEXT, opt);
    }

    return res;
}

static int number_textbox(ui_context_t *ctx, ui_real *value, rect_t r, ui_id id) {
    if (ctx->mouse_pressed == UI_MOUSE_LEFT && ctx->key_down & UI_KEY_SHIFT && ctx->hover == id) {
        ctx->number_edit = id;
        sprintf(ctx->number_edit_buf, UI_REAL_FMT, *value);
    }
    if (ctx->number_edit == id) {
        int res = ui_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf), id, r, 0);
        if (res & UI_RES_SUBMIT || ctx->focus != id) {
            *value = strtod(ctx->number_edit_buf, NULL);
            ctx->number_edit = 0;
        } else {
            return 1;
        }
    }
    return 0;
}

int ui_textbox_ex(ui_context_t *ctx, char *buf, int bufsz, int opt) {
    ui_id id = ui_get_id(ctx, &buf, sizeof(buf));
    rect_t r = ui_layout_next(ctx);
    return ui_textbox_raw(ctx, buf, bufsz, id, r, opt);
}

int ui_slider_ex(ui_context_t *ctx, ui_real *value, ui_real low, ui_real high, ui_real step, const char *fmt, int opt) {
    char buf[UI_MAX_FMT + 1];
    rect_t thumb;
    int x, w, res = 0;
    ui_real last = *value, v = last;
    ui_id id = ui_get_id(ctx, &value, sizeof(value));
    rect_t base = ui_layout_next(ctx);

    /* handle text input mode */
    if (number_textbox(ctx, &v, base, id)) {
        return res;
    }

    /* handle normal mode */
    ui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && (ctx->mouse_down | ctx->mouse_pressed) == UI_MOUSE_LEFT) {
        v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
        if (step) {
            v = ((long long)((v + step / 2) / step)) * step;
        }
    }
    /* clamp and store value, update res */
    *value = v = ui_clamp(v, low, high);
    if (last != v) {
        res |= UI_RES_CHANGE;
    }

    /* draw base */
    ui_draw_control_frame(ctx, id, base, UI_COLOR_BASE, opt);
    /* draw thumb */
    w = ctx->style->thumb_size;
    x = (v - low) * (base.w - w) / (high - low);
    thumb = neko_rect(base.x + x, base.y, w, base.h);
    ui_draw_control_frame(ctx, id, thumb, UI_COLOR_BUTTON, opt);
    /* draw text  */
    sprintf(buf, fmt, v);
    ui_draw_control_text(ctx, buf, base, UI_COLOR_TEXT, opt);

    return res;
}

int ui_number_ex(ui_context_t *ctx, ui_real *value, ui_real step, const char *fmt, int opt) {
    char buf[UI_MAX_FMT + 1];
    int res = 0;
    ui_id id = ui_get_id(ctx, &value, sizeof(value));
    rect_t base = ui_layout_next(ctx);
    ui_real last = *value;

    /* handle text input mode */
    if (number_textbox(ctx, value, base, id)) {
        return res;
    }

    /* handle normal mode */
    ui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && ctx->mouse_down == UI_MOUSE_LEFT) {
        *value += ctx->mouse_delta.x * step;
    }
    /* set flag if value changed */
    if (*value != last) {
        res |= UI_RES_CHANGE;
    }

    /* draw base */
    ui_draw_control_frame(ctx, id, base, UI_COLOR_BASE, opt);
    /* draw text  */
    sprintf(buf, fmt, *value);
    ui_draw_control_text(ctx, buf, base, UI_COLOR_TEXT, opt);

    return res;
}

static int header(ui_context_t *ctx, const char *label, int istreenode, int opt) {
    rect_t r;
    int active, expanded;
    ui_id id = ui_get_id(ctx, label, strlen(label));
    int idx = ui_pool_get(ctx, ctx->treenode_pool, UI_TREENODEPOOL_SIZE, id);
    int width = -1;
    ui_layout_row(ctx, 1, &width, 0);

    active = (idx >= 0);
    expanded = (opt & UI_OPT_EXPANDED) ? !active : active;
    r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, 0);

    /* handle click */
    active ^= (ctx->mouse_pressed == UI_MOUSE_LEFT && ctx->focus == id);

    /* update pool ref */
    if (idx >= 0) {
        if (active) {
            ui_pool_update(ctx, ctx->treenode_pool, idx);
        } else {
            memset(&ctx->treenode_pool[idx], 0, sizeof(ui_PoolItem));
        }
    } else if (active) {
        ui_pool_init(ctx, ctx->treenode_pool, UI_TREENODEPOOL_SIZE, id);
    }

    /* draw */
    if (istreenode) {
        if (ctx->hover == id) {
            ctx->draw_frame(ctx, r, UI_COLOR_BUTTONHOVER);
        }
    } else {
        ui_draw_control_frame(ctx, id, r, UI_COLOR_BUTTON, 0);
    }
    ui_draw_icon(ctx, expanded ? UI_ICON_EXPANDED : UI_ICON_COLLAPSED, neko_rect(r.x, r.y, r.h, r.h), ctx->style->colors[UI_COLOR_TEXT]);
    r.x += r.h - ctx->style->padding;
    r.w -= r.h - ctx->style->padding;
    ui_draw_control_text(ctx, label, r, UI_COLOR_TEXT, 0);

    return expanded ? UI_RES_ACTIVE : 0;
}

int ui_header_ex(ui_context_t *ctx, const char *label, int opt) { return header(ctx, label, 0, opt); }

int ui_begin_treenode_ex(ui_context_t *ctx, const char *label, int opt) {
    int res = header(ctx, label, 1, opt);
    if (res & UI_RES_ACTIVE) {
        get_layout(ctx)->indent += ctx->style->indent;
        push(ctx->id_stack, ctx->last_id);
    }
    return res;
}

void ui_end_treenode(ui_context_t *ctx) {
    get_layout(ctx)->indent -= ctx->style->indent;
    ui_pop_id(ctx);
}

#define scrollbar(ctx, cnt, b, cs, x, y, w, h)                                    \
    do {                                                                          \
        /* only add scrollbar if content size is larger than body */              \
        int maxscroll = cs.y - b->h;                                              \
                                                                                  \
        if (maxscroll > 0 && b->h > 0) {                                          \
            rect_t base, thumb;                                                   \
            ui_id id = ui_get_id(ctx, "!scrollbar" #y, 11);                       \
                                                                                  \
            /* get sizing / positioning */                                        \
            base = *b;                                                            \
            base.x = b->x + b->w;                                                 \
            base.w = ctx->style->scrollbar_size;                                  \
                                                                                  \
            /* handle input */                                                    \
            ui_update_control(ctx, id, base, 0);                                  \
            if (ctx->focus == id && ctx->mouse_down == UI_MOUSE_LEFT) {           \
                cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;              \
            }                                                                     \
            /* clamp scroll to limits */                                          \
            cnt->scroll.y = ui_clamp(cnt->scroll.y, 0, maxscroll);                \
                                                                                  \
            /* draw base and thumb */                                             \
            ctx->draw_frame(ctx, base, UI_COLOR_SCROLLBASE);                      \
            thumb = base;                                                         \
            thumb.h = ui_max(ctx->style->thumb_size, base.h * b->h / cs.y);       \
            thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll;            \
            ctx->draw_frame(ctx, thumb, UI_COLOR_SCROLLTHUMB);                    \
                                                                                  \
            /* set this as the scroll_target (will get scrolled on mousewheel) */ \
            /* if the mouse is over it */                                         \
            if (ui_mouse_over(ctx, *b)) {                                         \
                ctx->scroll_target = cnt;                                         \
            }                                                                     \
        } else {                                                                  \
            cnt->scroll.y = 0;                                                    \
        }                                                                         \
    } while (0)

static void scrollbars(ui_context_t *ctx, ui_Container *cnt, rect_t *body) {
    int sz = ctx->style->scrollbar_size;
    vec2 cs = cnt->content_size;
    cs.x += ctx->style->padding * 2;
    cs.y += ctx->style->padding * 2;
    ui_push_clip_rect(ctx, *body);
    /* resize body to make room for scrollbars */
    if (cs.y > cnt->body.h) {
        body->w -= sz;
    }
    if (cs.x > cnt->body.w) {
        body->h -= sz;
    }
    /* to create a horizontal or vertical scrollbar almost-identical code is
    ** used; only the references to `x|y` `w|h` need to be switched */
    scrollbar(ctx, cnt, body, cs, x, y, w, h);
    scrollbar(ctx, cnt, body, cs, y, x, h, w);
    ui_pop_clip_rect(ctx);
}

static void push_container_body(ui_context_t *ctx, ui_Container *cnt, rect_t body, int opt) {
    if (~opt & UI_OPT_NOSCROLL) {
        scrollbars(ctx, cnt, &body);
    }
    push_layout(ctx, expand_rect(body, -ctx->style->padding), cnt->scroll);
    cnt->body = body;
}

static void begin_root_container(ui_context_t *ctx, ui_Container *cnt) {
    push(ctx->container_stack, cnt);
    /* push container to roots list and push head command */
    push(ctx->root_list, cnt);
    cnt->head = push_jump(ctx, NULL);
    /* set as hover root if the mouse is overlapping this container and it has a
    ** higher zindex than the current hover root */
    if (rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) && (!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex)) {
        ctx->next_hover_root = cnt;
    }
    /* clipping is reset here in case a root-container is made within
    ** another root-containers's begin/end block; this prevents the inner
    ** root-container being clipped to the outer */
    push(ctx->clip_stack, unclipped_rect);
}

static void end_root_container(ui_context_t *ctx) {
    /* push tail 'goto' jump command and set head 'skip' command. the final steps
    ** on initing these are done in ui_end() */
    ui_Container *cnt = ui_get_current_container(ctx);
    cnt->tail = push_jump(ctx, NULL);
    cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;
    /* pop base clip rect and container */
    ui_pop_clip_rect(ctx);
    pop_container(ctx);
}

int ui_begin_window_ex(ui_context_t *ctx, const char *title, rect_t rect, int opt) {
    rect_t body;
    ui_id id = ui_get_id(ctx, title, strlen(title));
    ui_Container *cnt = get_container(ctx, id, opt);
    if (!cnt || !cnt->open) {
        return 0;
    }
    push(ctx->id_stack, id);

    if (cnt->rect.w == 0) {
        cnt->rect = rect;
    }
    begin_root_container(ctx, cnt);
    rect = body = cnt->rect;

    /* draw frame */
    if (~opt & UI_OPT_NOFRAME) {
        ctx->draw_frame(ctx, rect, UI_COLOR_WINDOWBG);
    }

    /* do title bar */
    if (~opt & UI_OPT_NOTITLE) {
        rect_t tr = rect;
        tr.h = ctx->style->title_height;
        ctx->draw_frame(ctx, tr, UI_COLOR_TITLEBG);

        /* do title text */
        if (~opt & UI_OPT_NOTITLE) {
            ui_id id = ui_get_id(ctx, "!title", 6);
            ui_update_control(ctx, id, tr, opt);
            ui_draw_control_text(ctx, title, tr, UI_COLOR_TITLETEXT, opt);
            if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
                cnt->rect.x += ctx->mouse_delta.x;
                cnt->rect.y += ctx->mouse_delta.y;
            }
            body.y += tr.h;
            body.h -= tr.h;
        }

        /* do `close` button */
        if (~opt & UI_OPT_NOCLOSE) {
            ui_id id = ui_get_id(ctx, "!close", 6);
            rect_t r = neko_rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
            tr.w -= r.w;
            ui_draw_icon(ctx, UI_ICON_CLOSE, r, ctx->style->colors[UI_COLOR_TITLETEXT]);
            ui_update_control(ctx, id, r, opt);
            if (ctx->mouse_pressed == UI_MOUSE_LEFT && id == ctx->focus) {
                cnt->open = 0;
            }
        }
    }

    push_container_body(ctx, cnt, body, opt);

    /* do `resize` handle */
    if (~opt & UI_OPT_NORESIZE) {
        int sz = ctx->style->title_height;
        ui_id id = ui_get_id(ctx, "!resize", 7);
        rect_t r = neko_rect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
        ui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
            cnt->rect.w = ui_max(96, cnt->rect.w + ctx->mouse_delta.x);
            cnt->rect.h = ui_max(64, cnt->rect.h + ctx->mouse_delta.y);
        }
    }

    /* resize to content size */
    if (opt & UI_OPT_AUTOSIZE) {
        rect_t r = get_layout(ctx)->body;
        cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
        cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
    }

    /* close if this is a popup window and elsewhere was clicked */
    if (opt & UI_OPT_POPUP && ctx->mouse_pressed && ctx->hover_root != cnt) {
        cnt->open = 0;
    }

    ui_push_clip_rect(ctx, cnt->body);
    return UI_RES_ACTIVE;
}

void ui_end_window(ui_context_t *ctx) {
    ui_pop_clip_rect(ctx);
    end_root_container(ctx);
}

void ui_open_popup(ui_context_t *ctx, const char *name) {
    ui_Container *cnt = ui_get_container(ctx, name);
    /* set as hover root so popup isn't closed in begin_window_ex()  */
    ctx->hover_root = ctx->next_hover_root = cnt;
    /* position at mouse cursor, open and bring-to-front */
    cnt->rect = neko_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 1, 1);
    cnt->open = 1;
    ui_bring_to_front(ctx, cnt);
}

int ui_begin_popup(ui_context_t *ctx, const char *name) {
    int opt = UI_OPT_POPUP | UI_OPT_AUTOSIZE | UI_OPT_NORESIZE | UI_OPT_NOSCROLL | UI_OPT_NOTITLE | UI_OPT_CLOSED;
    return ui_begin_window_ex(ctx, name, neko_rect(0, 0, 0, 0), opt);
}

void ui_end_popup(ui_context_t *ctx) { ui_end_window(ctx); }

void ui_begin_panel_ex(ui_context_t *ctx, const char *name, int opt) {
    ui_Container *cnt;
    ui_push_id(ctx, name, strlen(name));
    cnt = get_container(ctx, ctx->last_id, opt);
    cnt->rect = ui_layout_next(ctx);
    if (~opt & UI_OPT_NOFRAME) {
        ctx->draw_frame(ctx, cnt->rect, UI_COLOR_PANELBG);
    }
    push(ctx->container_stack, cnt);
    push_container_body(ctx, cnt, cnt->rect, opt);
    ui_push_clip_rect(ctx, cnt->body);
}

void ui_end_panel(ui_context_t *ctx) {
    ui_pop_clip_rect(ctx);
    pop_container(ctx);
}
