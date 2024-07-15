#include "neko_ui.h"

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/sokol_gl.h>

#include "deps/microui_atlas.inl"
#include "neko_app.h"
#include "neko_lua.h"
#include "neko_prelude.h"

#define lui_push(stk, val)                                                          \
    do {                                                                            \
        NEKO_EXPECT((stk).idx < (int)(sizeof((stk).items) / sizeof(*(stk).items))); \
        (stk).items[(stk).idx] = (val);                                             \
        (stk).idx++; /* incremented after incase `val` uses this value */           \
    } while (0)

#define lui_pop(stk)                \
    do {                            \
        NEKO_EXPECT((stk).idx > 0); \
        (stk).idx--;                \
    } while (0)

static lui_Rect unclipped_rect = {0, 0, 0x1000000, 0x1000000};

static lui_Style default_style = {
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
                {230, 230, 230, 255}, /* LUI_COLOR_TEXT */
                {25, 25, 25, 255},    /* LUI_COLOR_BORDER */
                {50, 50, 50, 255},    /* LUI_COLOR_WINDOWBG */
                {25, 25, 25, 255},    /* LUI_COLOR_TITLEBG */
                {240, 240, 240, 255}, /* LUI_COLOR_TITLETEXT */
                {0, 0, 0, 0},         /* LUI_COLOR_PANELBG */
                {75, 75, 75, 255},    /* LUI_COLOR_BUTTON */
                {95, 95, 95, 255},    /* LUI_COLOR_BUTTONHOVER */
                {115, 115, 115, 255}, /* LUI_COLOR_BUTTONFOCUS */
                {30, 30, 30, 255},    /* LUI_COLOR_BASE */
                {35, 35, 35, 255},    /* LUI_COLOR_BASEHOVER */
                {40, 40, 40, 255},    /* LUI_COLOR_BASEFOCUS */
                {43, 43, 43, 255},    /* LUI_COLOR_SCROLLBASE */
                {30, 30, 30, 255}     /* LUI_COLOR_SCROLLTHUMB */
        }};

lui_Vec2 lui_vec2(int x, int y) {
    lui_Vec2 res;
    res.x = x;
    res.y = y;
    return res;
}

lui_Rect lui_rect(int x, int y, int w, int h) {
    lui_Rect res;
    res.x = x;
    res.y = y;
    res.w = w;
    res.h = h;
    return res;
}

lui_Color lui_color(int r, int g, int b, int a) {
    lui_Color res;
    res.r = r;
    res.g = g;
    res.b = b;
    res.a = a;
    return res;
}

static lui_Rect expand_rect(lui_Rect rect, int n) { return lui_rect(rect.x - n, rect.y - n, rect.w + n * 2, rect.h + n * 2); }

static lui_Rect intersect_rects(lui_Rect r1, lui_Rect r2) {
    int x1 = lui_max(r1.x, r2.x);
    int y1 = lui_max(r1.y, r2.y);
    int x2 = lui_min(r1.x + r1.w, r2.x + r2.w);
    int y2 = lui_min(r1.y + r1.h, r2.y + r2.h);
    if (x2 < x1) {
        x2 = x1;
    }
    if (y2 < y1) {
        y2 = y1;
    }
    return lui_rect(x1, y1, x2 - x1, y2 - y1);
}

static int rect_overlaps_vec2(lui_Rect r, lui_Vec2 p) { return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h; }

static void draw_frame(lui_Context *ctx, lui_Rect rect, int colorid) {
    lui_draw_rect(ctx, rect, ctx->style->colors[colorid]);
    if (colorid == LUI_COLOR_SCROLLBASE || colorid == LUI_COLOR_SCROLLTHUMB || colorid == LUI_COLOR_TITLEBG) {
        return;
    }
    /* draw border */
    if (ctx->style->colors[LUI_COLOR_BORDER].a) {
        lui_draw_box(ctx, expand_rect(rect, 1), ctx->style->colors[LUI_COLOR_BORDER]);
    }
}

void lui_init(lui_Context *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->draw_frame = draw_frame;
    ctx->_style = default_style;
    ctx->style = &ctx->_style;
}

void lui_begin(lui_Context *ctx) {
    NEKO_EXPECT(ctx->text_width && ctx->text_height);
    ctx->command_list.idx = 0;
    ctx->root_list.idx = 0;
    ctx->scroll_target = NULL;
    ctx->hover_root = ctx->next_hover_root;
    ctx->next_hover_root = NULL;
    ctx->mouse_delta.x = ctx->mouse_pos.x - ctx->last_mouse_pos.x;
    ctx->mouse_delta.y = ctx->mouse_pos.y - ctx->last_mouse_pos.y;
    ctx->frame++;
}

static int compare_zindex(const void *a, const void *b) { return (*(lui_Container **)a)->zindex - (*(lui_Container **)b)->zindex; }

void lui_end(lui_Context *ctx) {
    int i, n;
    /* check stacks */
    NEKO_EXPECT(ctx->container_stack.idx == 0);
    NEKO_EXPECT(ctx->clip_stack.idx == 0);
    NEKO_EXPECT(ctx->id_stack.idx == 0);
    NEKO_EXPECT(ctx->layout_stack.idx == 0);

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
        lui_bring_to_front(ctx, ctx->next_hover_root);
    }

    /* reset input state */
    ctx->key_pressed = 0;
    ctx->input_text[0] = '\0';
    ctx->mouse_pressed = 0;
    ctx->scroll_delta = lui_vec2(0, 0);
    ctx->last_mouse_pos = ctx->mouse_pos;

    /* sort root containers by zindex */
    n = ctx->root_list.idx;
    qsort(ctx->root_list.items, n, sizeof(lui_Container *), compare_zindex);

    /* set root container jump commands */
    for (i = 0; i < n; i++) {
        lui_Container *cnt = ctx->root_list.items[i];
        /* if this is the first container then make the first command jump to it.
        ** otherwise set the previous container's tail to jump to this one */
        if (i == 0) {
            lui_Command *cmd = (lui_Command *)ctx->command_list.items;
            cmd->jump.dst = (char *)cnt->head + sizeof(lui_JumpCommand);
        } else {
            lui_Container *prev = ctx->root_list.items[i - 1];
            prev->tail->jump.dst = (char *)cnt->head + sizeof(lui_JumpCommand);
        }
        /* make the last container's tail jump to the end of command list */
        if (i == n - 1) {
            cnt->tail->jump.dst = ctx->command_list.items + ctx->command_list.idx;
        }
    }
}

void lui_set_focus(lui_Context *ctx, lui_Id id) {
    ctx->focus = id;
    ctx->updated_focus = 1;
}

/* 32bit fnv-1a hash */
#define HASH_INITIAL 2166136261

static void hash(lui_Id *hash, const void *data, int size) {
    const unsigned char *p = (const unsigned char *)data;
    while (size--) {
        *hash = (*hash ^ *p++) * 16777619;
    }
}

lui_Id lui_get_id(lui_Context *ctx, const void *data, int size) {
    int idx = ctx->id_stack.idx;
    lui_Id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : HASH_INITIAL;
    hash(&res, data, size);
    ctx->last_id = res;
    return res;
}

void lui_push_id(lui_Context *ctx, const void *data, int size) { lui_push(ctx->id_stack, lui_get_id(ctx, data, size)); }

void lui_pop_id(lui_Context *ctx) { lui_pop(ctx->id_stack); }

void lui_push_clip_rect(lui_Context *ctx, lui_Rect rect) {
    lui_Rect last = lui_get_clip_rect(ctx);
    lui_push(ctx->clip_stack, intersect_rects(rect, last));
}

void lui_pop_clip_rect(lui_Context *ctx) { lui_pop(ctx->clip_stack); }

lui_Rect lui_get_clip_rect(lui_Context *ctx) {
    NEKO_EXPECT(ctx->clip_stack.idx > 0);
    return ctx->clip_stack.items[ctx->clip_stack.idx - 1];
}

int lui_check_clip(lui_Context *ctx, lui_Rect r) {
    lui_Rect cr = lui_get_clip_rect(ctx);
    if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h || r.y + r.h < cr.y) {
        return LUI_CLIP_ALL;
    }
    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y && r.y + r.h <= cr.y + cr.h) {
        return 0;
    }
    return LUI_CLIP_PART;
}

static void push_layout(lui_Context *ctx, lui_Rect body, lui_Vec2 scroll) {
    lui_Layout layout;
    int width = 0;
    memset(&layout, 0, sizeof(layout));
    layout.body = lui_rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
    layout.max = lui_vec2(-0x1000000, -0x1000000);
    lui_push(ctx->layout_stack, layout);
    lui_layout_row(ctx, 1, &width, 0);
}

static lui_Layout *get_layout(lui_Context *ctx) { return &ctx->layout_stack.items[ctx->layout_stack.idx - 1]; }

static void pop_container(lui_Context *ctx) {
    lui_Container *cnt = lui_get_current_container(ctx);
    lui_Layout *layout = get_layout(ctx);
    cnt->content_size.x = layout->max.x - layout->body.x;
    cnt->content_size.y = layout->max.y - layout->body.y;
    /* lui_pop container, layout and id */
    lui_pop(ctx->container_stack);
    lui_pop(ctx->layout_stack);
    lui_pop_id(ctx);
}

lui_Container *lui_get_current_container(lui_Context *ctx) {
    NEKO_EXPECT(ctx->container_stack.idx > 0);
    return ctx->container_stack.items[ctx->container_stack.idx - 1];
}

static lui_Container *get_container(lui_Context *ctx, lui_Id id, int opt) {
    lui_Container *cnt;
    /* try to get existing container from pool */
    int idx = lui_pool_get(ctx, ctx->container_pool, LUI_CONTAINERPOOL_SIZE, id);
    if (idx >= 0) {
        if (ctx->containers[idx].open || ~opt & LUI_OPT_CLOSED) {
            lui_pool_update(ctx, ctx->container_pool, idx);
        }
        return &ctx->containers[idx];
    }
    if (opt & LUI_OPT_CLOSED) {
        return NULL;
    }
    /* container not found in pool: init new container */
    idx = lui_pool_init(ctx, ctx->container_pool, LUI_CONTAINERPOOL_SIZE, id);
    cnt = &ctx->containers[idx];
    memset(cnt, 0, sizeof(*cnt));
    cnt->open = 1;
    lui_bring_to_front(ctx, cnt);
    return cnt;
}

lui_Container *lui_get_container(lui_Context *ctx, const char *name) {
    lui_Id id = lui_get_id(ctx, name, strlen(name));
    return get_container(ctx, id, 0);
}

void lui_bring_to_front(lui_Context *ctx, lui_Container *cnt) { cnt->zindex = ++ctx->last_zindex; }

/*============================================================================
** pool
**============================================================================*/

int lui_pool_init(lui_Context *ctx, lui_PoolItem *items, int len, lui_Id id) {
    int i, n = -1, f = ctx->frame;
    for (i = 0; i < len; i++) {
        if (items[i].last_update < f) {
            f = items[i].last_update;
            n = i;
        }
    }
    NEKO_EXPECT(n > -1);
    items[n].id = id;
    lui_pool_update(ctx, items, n);
    return n;
}

int lui_pool_get(lui_Context *ctx, lui_PoolItem *items, int len, lui_Id id) {
    int i;
    NEKO_UNUSED(ctx);
    for (i = 0; i < len; i++) {
        if (items[i].id == id) {
            return i;
        }
    }
    return -1;
}

void lui_pool_update(lui_Context *ctx, lui_PoolItem *items, int idx) { items[idx].last_update = ctx->frame; }

/*============================================================================
** input handlers
**============================================================================*/

void lui_input_mousemove(lui_Context *ctx, int x, int y) { ctx->mouse_pos = lui_vec2(x, y); }

void lui_input_mousedown(lui_Context *ctx, int x, int y, int btn) {
    lui_input_mousemove(ctx, x, y);
    ctx->mouse_down |= btn;
    ctx->mouse_pressed |= btn;
}

void lui_input_mouseup(lui_Context *ctx, int x, int y, int btn) {
    lui_input_mousemove(ctx, x, y);
    ctx->mouse_down &= ~btn;
}

void lui_input_scroll(lui_Context *ctx, int x, int y) {
    ctx->scroll_delta.x += x;
    ctx->scroll_delta.y += y;
}

void lui_input_keydown(lui_Context *ctx, int key) {
    ctx->key_pressed |= key;
    ctx->key_down |= key;
}

void lui_input_keyup(lui_Context *ctx, int key) { ctx->key_down &= ~key; }

void lui_input_text(lui_Context *ctx, const char *text) {
    int len = strlen(ctx->input_text);
    int size = strlen(text) + 1;
    NEKO_EXPECT(len + size <= (int)sizeof(ctx->input_text));
    memcpy(ctx->input_text + len, text, size);
}

/*============================================================================
** commandlist
**============================================================================*/

lui_Command *lui_push_command(lui_Context *ctx, int type, int size) {
    lui_Command *cmd = (lui_Command *)(ctx->command_list.items + ctx->command_list.idx);
    NEKO_EXPECT(ctx->command_list.idx + size < LUI_COMMANDLIST_SIZE);
    cmd->base.type = type;
    cmd->base.size = size;
    ctx->command_list.idx += size;
    return cmd;
}

int lui_next_command(lui_Context *ctx, lui_Command **cmd) {
    if (*cmd) {
        *cmd = (lui_Command *)(((char *)*cmd) + (*cmd)->base.size);
    } else {
        *cmd = (lui_Command *)ctx->command_list.items;
    }
    while ((char *)*cmd != ctx->command_list.items + ctx->command_list.idx) {
        if ((*cmd)->type != LUI_COMMAND_JUMP) {
            return 1;
        }
        *cmd = (lui_Command *)((*cmd)->jump.dst);
    }
    return 0;
}

static lui_Command *push_jump(lui_Context *ctx, lui_Command *dst) {
    lui_Command *cmd;
    cmd = lui_push_command(ctx, LUI_COMMAND_JUMP, sizeof(lui_JumpCommand));
    cmd->jump.dst = dst;
    return cmd;
}

void lui_set_clip(lui_Context *ctx, lui_Rect rect) {
    lui_Command *cmd;
    cmd = lui_push_command(ctx, LUI_COMMAND_CLIP, sizeof(lui_ClipCommand));
    cmd->clip.rect = rect;
}

void lui_draw_rect(lui_Context *ctx, lui_Rect rect, lui_Color color) {
    lui_Command *cmd;
    rect = intersect_rects(rect, lui_get_clip_rect(ctx));
    if (rect.w > 0 && rect.h > 0) {
        cmd = lui_push_command(ctx, LUI_COMMAND_RECT, sizeof(lui_RectCommand));
        cmd->rect.rect = rect;
        cmd->rect.color = color;
    }
}

void lui_draw_box(lui_Context *ctx, lui_Rect rect, lui_Color color) {
    lui_draw_rect(ctx, lui_rect(rect.x + 1, rect.y, rect.w - 2, 1), color);
    lui_draw_rect(ctx, lui_rect(rect.x + 1, rect.y + rect.h - 1, rect.w - 2, 1), color);
    lui_draw_rect(ctx, lui_rect(rect.x, rect.y, 1, rect.h), color);
    lui_draw_rect(ctx, lui_rect(rect.x + rect.w - 1, rect.y, 1, rect.h), color);
}

void lui_draw_text(lui_Context *ctx, lui_Font font, const char *str, int len, lui_Vec2 pos, lui_Color color) {
    lui_Command *cmd;
    lui_Rect rect = lui_rect(pos.x, pos.y, ctx->text_width(font, str, len), ctx->text_height(font));
    int clipped = lui_check_clip(ctx, rect);
    if (clipped == LUI_CLIP_ALL) {
        return;
    }
    if (clipped == LUI_CLIP_PART) {
        lui_set_clip(ctx, lui_get_clip_rect(ctx));
    }
    /* add command */
    if (len < 0) {
        len = strlen(str);
    }
    cmd = lui_push_command(ctx, LUI_COMMAND_TEXT, sizeof(lui_TextCommand) + len);
    memcpy(cmd->text.str, str, len);
    cmd->text.str[len] = '\0';
    cmd->text.pos = pos;
    cmd->text.color = color;
    cmd->text.font = font;
    /* reset clipping if it was set */
    if (clipped) {
        lui_set_clip(ctx, unclipped_rect);
    }
}

void lui_draw_icon(lui_Context *ctx, int id, lui_Rect rect, lui_Color color) {
    lui_Command *cmd;
    /* do clip command if the rect isn't fully contained within the cliprect */
    int clipped = lui_check_clip(ctx, rect);
    if (clipped == LUI_CLIP_ALL) {
        return;
    }
    if (clipped == LUI_CLIP_PART) {
        lui_set_clip(ctx, lui_get_clip_rect(ctx));
    }
    /* do icon command */
    cmd = lui_push_command(ctx, LUI_COMMAND_ICON, sizeof(lui_IconCommand));
    cmd->icon.id = id;
    cmd->icon.rect = rect;
    cmd->icon.color = color;
    /* reset clipping if it was set */
    if (clipped) {
        lui_set_clip(ctx, unclipped_rect);
    }
}

/*============================================================================
** layout
**============================================================================*/

enum class lui_layout_type { E_RELATIVE = 1, E_ABSOLUTE = 2 };

void lui_layout_begin_column(lui_Context *ctx) { push_layout(ctx, lui_layout_next(ctx), lui_vec2(0, 0)); }

void lui_layout_end_column(lui_Context *ctx) {
    lui_Layout *a, *b;
    b = get_layout(ctx);
    lui_pop(ctx->layout_stack);
    /* inherit position/next_row/max from child layout if they are greater */
    a = get_layout(ctx);
    a->position.x = lui_max(a->position.x, b->position.x + b->body.x - a->body.x);
    a->next_row = lui_max(a->next_row, b->next_row + b->body.y - a->body.y);
    a->max.x = lui_max(a->max.x, b->max.x);
    a->max.y = lui_max(a->max.y, b->max.y);
}

void lui_layout_row(lui_Context *ctx, int items, const int *widths, int height) {
    lui_Layout *layout = get_layout(ctx);
    if (widths) {
        NEKO_EXPECT(items <= LUI_MAX_WIDTHS);
        memcpy(layout->widths, widths, items * sizeof(widths[0]));
    }
    layout->items = items;
    layout->position = lui_vec2(layout->indent, layout->next_row);
    layout->size.y = height;
    layout->item_index = 0;
}

void lui_layout_width(lui_Context *ctx, int width) { get_layout(ctx)->size.x = width; }

void lui_layout_height(lui_Context *ctx, int height) { get_layout(ctx)->size.y = height; }

void lui_layout_set_next(lui_Context *ctx, lui_Rect r, int relative) {
    lui_Layout *layout = get_layout(ctx);
    layout->next = r;
    layout->next_type = (int)(relative ? lui_layout_type::E_RELATIVE : lui_layout_type::E_ABSOLUTE);
}

lui_Rect lui_layout_next(lui_Context *ctx) {
    lui_Layout *layout = get_layout(ctx);
    lui_Style *style = ctx->style;
    lui_Rect res;

    if (layout->next_type) {
        /* handle rect set by `lui_layout_set_next` */
        int type = layout->next_type;
        layout->next_type = 0;
        res = layout->next;
        if (type == (int)lui_layout_type::E_ABSOLUTE) {
            return (ctx->last_rect = res);
        }

    } else {
        /* handle next row */
        if (layout->item_index == layout->items) {
            lui_layout_row(ctx, layout->items, NULL, layout->size.y);
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
    layout->next_row = lui_max(layout->next_row, res.y + res.h + style->spacing);

    /* apply body offset */
    res.x += layout->body.x;
    res.y += layout->body.y;

    /* update max position */
    layout->max.x = lui_max(layout->max.x, res.x + res.w);
    layout->max.y = lui_max(layout->max.y, res.y + res.h);

    return (ctx->last_rect = res);
}

/*============================================================================
** controls
**============================================================================*/

static int in_hover_root(lui_Context *ctx) {
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

void lui_draw_control_frame(lui_Context *ctx, lui_Id id, lui_Rect rect, int colorid, int opt) {
    if (opt & LUI_OPT_NOFRAME) {
        return;
    }
    colorid += (ctx->focus == id) ? 2 : (ctx->hover == id) ? 1 : 0;
    ctx->draw_frame(ctx, rect, colorid);
}

void lui_draw_control_text(lui_Context *ctx, const char *str, lui_Rect rect, int colorid, int opt) {
    lui_Vec2 pos;
    lui_Font font = ctx->style->font;
    int tw = ctx->text_width(font, str, -1);
    lui_push_clip_rect(ctx, rect);
    pos.y = rect.y + (rect.h - ctx->text_height(font)) / 2;
    if (opt & LUI_OPT_ALIGNCENTER) {
        pos.x = rect.x + (rect.w - tw) / 2;
    } else if (opt & LUI_OPT_ALIGNRIGHT) {
        pos.x = rect.x + rect.w - tw - ctx->style->padding;
    } else {
        pos.x = rect.x + ctx->style->padding;
    }
    lui_draw_text(ctx, font, str, -1, pos, ctx->style->colors[colorid]);
    lui_pop_clip_rect(ctx);
}

int lui_mouse_over(lui_Context *ctx, lui_Rect rect) { return rect_overlaps_vec2(rect, ctx->mouse_pos) && rect_overlaps_vec2(lui_get_clip_rect(ctx), ctx->mouse_pos) && in_hover_root(ctx); }

void lui_update_control(lui_Context *ctx, lui_Id id, lui_Rect rect, int opt) {
    int mouseover = lui_mouse_over(ctx, rect);

    if (ctx->focus == id) {
        ctx->updated_focus = 1;
    }
    if (opt & LUI_OPT_NOINTERACT) {
        return;
    }
    if (mouseover && !ctx->mouse_down) {
        ctx->hover = id;
    }

    if (ctx->focus == id) {
        if (ctx->mouse_pressed && !mouseover) {
            lui_set_focus(ctx, 0);
        }
        if (!ctx->mouse_down && ~opt & LUI_OPT_HOLDFOCUS) {
            lui_set_focus(ctx, 0);
        }
    }

    if (ctx->hover == id) {
        if (ctx->mouse_pressed) {
            lui_set_focus(ctx, id);
        } else if (!mouseover) {
            ctx->hover = 0;
        }
    }
}

void lui_text(lui_Context *ctx, const char *text) {
    const char *start, *end, *p = text;
    int width = -1;
    lui_Font font = ctx->style->font;
    lui_Color color = ctx->style->colors[LUI_COLOR_TEXT];
    lui_layout_begin_column(ctx);
    lui_layout_row(ctx, 1, &width, ctx->text_height(font));
    do {
        lui_Rect r = lui_layout_next(ctx);
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
        lui_draw_text(ctx, font, start, end - start, lui_vec2(r.x, r.y), color);
        p = end + 1;
    } while (*end);
    lui_layout_end_column(ctx);
}

void lui_label(lui_Context *ctx, const char *text) { lui_draw_control_text(ctx, text, lui_layout_next(ctx), LUI_COLOR_TEXT, 0); }

int lui_button_ex(lui_Context *ctx, const char *label, int icon, int opt) {
    int res = 0;
    lui_Id id = label ? lui_get_id(ctx, label, strlen(label)) : lui_get_id(ctx, &icon, sizeof(icon));
    lui_Rect r = lui_layout_next(ctx);
    lui_update_control(ctx, id, r, opt);
    /* handle click */
    if (ctx->mouse_pressed == LUI_MOUSE_LEFT && ctx->focus == id) {
        res |= LUI_RES_SUBMIT;
    }
    /* draw */
    lui_draw_control_frame(ctx, id, r, LUI_COLOR_BUTTON, opt);
    if (label) {
        lui_draw_control_text(ctx, label, r, LUI_COLOR_TEXT, opt);
    }
    if (icon) {
        lui_draw_icon(ctx, icon, r, ctx->style->colors[LUI_COLOR_TEXT]);
    }
    return res;
}

int lui_checkbox(lui_Context *ctx, const char *label, int *state) {
    int res = 0;
    lui_Id id = lui_get_id(ctx, &state, sizeof(state));
    lui_Rect r = lui_layout_next(ctx);
    lui_Rect box = lui_rect(r.x, r.y, r.h, r.h);
    lui_update_control(ctx, id, r, 0);
    /* handle click */
    if (ctx->mouse_pressed == LUI_MOUSE_LEFT && ctx->focus == id) {
        res |= LUI_RES_CHANGE;
        *state = !*state;
    }
    /* draw */
    lui_draw_control_frame(ctx, id, box, LUI_COLOR_BASE, 0);
    if (*state) {
        lui_draw_icon(ctx, LUI_ICON_CHECK, box, ctx->style->colors[LUI_COLOR_TEXT]);
    }
    r = lui_rect(r.x + box.w, r.y, r.w - box.w, r.h);
    lui_draw_control_text(ctx, label, r, LUI_COLOR_TEXT, 0);
    return res;
}

int lui_textbox_raw(lui_Context *ctx, char *buf, int bufsz, lui_Id id, lui_Rect r, int opt) {
    int res = 0;
    lui_update_control(ctx, id, r, opt | LUI_OPT_HOLDFOCUS);

    if (ctx->focus == id) {
        /* handle text input */
        int len = strlen(buf);
        int n = lui_min(bufsz - len - 1, (int)strlen(ctx->input_text));
        if (n > 0) {
            memcpy(buf + len, ctx->input_text, n);
            len += n;
            buf[len] = '\0';
            res |= LUI_RES_CHANGE;
        }
        /* handle backspace */
        if (ctx->key_pressed & LUI_KEY_BACKSPACE && len > 0) {
            /* skip utf-8 continuation bytes */
            while ((buf[--len] & 0xc0) == 0x80 && len > 0);
            buf[len] = '\0';
            res |= LUI_RES_CHANGE;
        }
        /* handle return */
        if (ctx->key_pressed & LUI_KEY_RETURN) {
            lui_set_focus(ctx, 0);
            res |= LUI_RES_SUBMIT;
        }
    }

    /* draw */
    lui_draw_control_frame(ctx, id, r, LUI_COLOR_BASE, opt);
    if (ctx->focus == id) {
        lui_Color color = ctx->style->colors[LUI_COLOR_TEXT];
        lui_Font font = ctx->style->font;
        int textw = ctx->text_width(font, buf, -1);
        int texth = ctx->text_height(font);
        int ofx = r.w - ctx->style->padding - textw - 1;
        int textx = r.x + lui_min(ofx, ctx->style->padding);
        int texty = r.y + (r.h - texth) / 2;
        lui_push_clip_rect(ctx, r);
        lui_draw_text(ctx, font, buf, -1, lui_vec2(textx, texty), color);
        lui_draw_rect(ctx, lui_rect(textx + textw, texty, 1, texth), color);
        lui_pop_clip_rect(ctx);
    } else {
        lui_draw_control_text(ctx, buf, r, LUI_COLOR_TEXT, opt);
    }

    return res;
}

static int number_textbox(lui_Context *ctx, lui_Real *value, lui_Rect r, lui_Id id) {
    if (ctx->mouse_pressed == LUI_MOUSE_LEFT && ctx->key_down & LUI_KEY_SHIFT && ctx->hover == id) {
        ctx->number_edit = id;
        sprintf(ctx->number_edit_buf, LUI_REAL_FMT, *value);
    }
    if (ctx->number_edit == id) {
        int res = lui_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf), id, r, 0);
        if (res & LUI_RES_SUBMIT || ctx->focus != id) {
            *value = strtod(ctx->number_edit_buf, NULL);
            ctx->number_edit = 0;
        } else {
            return 1;
        }
    }
    return 0;
}

int lui_textbox_ex(lui_Context *ctx, char *buf, int bufsz, int opt) {
    lui_Id id = lui_get_id(ctx, &buf, sizeof(buf));
    lui_Rect r = lui_layout_next(ctx);
    return lui_textbox_raw(ctx, buf, bufsz, id, r, opt);
}

int lui_slider_ex(lui_Context *ctx, lui_Real *value, lui_Real low, lui_Real high, lui_Real step, const char *fmt, int opt) {
    char buf[LUI_MAX_FMT + 1];
    lui_Rect thumb;
    int x, w, res = 0;
    lui_Real last = *value, v = last;
    lui_Id id = lui_get_id(ctx, &value, sizeof(value));
    lui_Rect base = lui_layout_next(ctx);

    /* handle text input mode */
    if (number_textbox(ctx, &v, base, id)) {
        return res;
    }

    /* handle normal mode */
    lui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && (ctx->mouse_down | ctx->mouse_pressed) == LUI_MOUSE_LEFT) {
        v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
        if (step) {
            v = (((v + step / 2) / step)) * step;
        }
    }
    /* clamp and store value, update res */
    *value = v = lui_clamp(v, low, high);
    if (last != v) {
        res |= LUI_RES_CHANGE;
    }

    /* draw base */
    lui_draw_control_frame(ctx, id, base, LUI_COLOR_BASE, opt);
    /* draw thumb */
    w = ctx->style->thumb_size;
    x = (v - low) * (base.w - w) / (high - low);
    thumb = lui_rect(base.x + x, base.y, w, base.h);
    lui_draw_control_frame(ctx, id, thumb, LUI_COLOR_BUTTON, opt);
    /* draw text  */
    sprintf(buf, fmt, v);
    lui_draw_control_text(ctx, buf, base, LUI_COLOR_TEXT, opt);

    return res;
}

int lui_number_ex(lui_Context *ctx, lui_Real *value, lui_Real step, const char *fmt, int opt) {
    char buf[LUI_MAX_FMT + 1];
    int res = 0;
    lui_Id id = lui_get_id(ctx, &value, sizeof(value));
    lui_Rect base = lui_layout_next(ctx);
    lui_Real last = *value;

    /* handle text input mode */
    if (number_textbox(ctx, value, base, id)) {
        return res;
    }

    /* handle normal mode */
    lui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && ctx->mouse_down == LUI_MOUSE_LEFT) {
        *value += ctx->mouse_delta.x * step;
    }
    /* set flag if value changed */
    if (*value != last) {
        res |= LUI_RES_CHANGE;
    }

    /* draw base */
    lui_draw_control_frame(ctx, id, base, LUI_COLOR_BASE, opt);
    /* draw text  */
    sprintf(buf, fmt, *value);
    lui_draw_control_text(ctx, buf, base, LUI_COLOR_TEXT, opt);

    return res;
}

static int header(lui_Context *ctx, const char *label, int istreenode, int opt) {
    lui_Rect r;
    int active, expanded;
    lui_Id id = lui_get_id(ctx, label, strlen(label));
    int idx = lui_pool_get(ctx, ctx->treenode_pool, LUI_TREENODEPOOL_SIZE, id);
    int width = -1;
    lui_layout_row(ctx, 1, &width, 0);

    active = (idx >= 0);
    expanded = (opt & LUI_OPT_EXPANDED) ? !active : active;
    r = lui_layout_next(ctx);
    lui_update_control(ctx, id, r, 0);

    /* handle click */
    active ^= (ctx->mouse_pressed == LUI_MOUSE_LEFT && ctx->focus == id);

    /* update pool ref */
    if (idx >= 0) {
        if (active) {
            lui_pool_update(ctx, ctx->treenode_pool, idx);
        } else {
            memset(&ctx->treenode_pool[idx], 0, sizeof(lui_PoolItem));
        }
    } else if (active) {
        lui_pool_init(ctx, ctx->treenode_pool, LUI_TREENODEPOOL_SIZE, id);
    }

    /* draw */
    if (istreenode) {
        if (ctx->hover == id) {
            ctx->draw_frame(ctx, r, LUI_COLOR_BUTTONHOVER);
        }
    } else {
        lui_draw_control_frame(ctx, id, r, LUI_COLOR_BUTTON, 0);
    }
    lui_draw_icon(ctx, expanded ? LUI_ICON_EXPANDED : LUI_ICON_COLLAPSED, lui_rect(r.x, r.y, r.h, r.h), ctx->style->colors[LUI_COLOR_TEXT]);
    r.x += r.h - ctx->style->padding;
    r.w -= r.h - ctx->style->padding;
    lui_draw_control_text(ctx, label, r, LUI_COLOR_TEXT, 0);

    return expanded ? LUI_RES_ACTIVE : 0;
}

int lui_header_ex(lui_Context *ctx, const char *label, int opt) { return header(ctx, label, 0, opt); }

int lui_begin_treenode_ex(lui_Context *ctx, const char *label, int opt) {
    int res = header(ctx, label, 1, opt);
    if (res & LUI_RES_ACTIVE) {
        get_layout(ctx)->indent += ctx->style->indent;
        lui_push(ctx->id_stack, ctx->last_id);
    }
    return res;
}

void lui_end_treenode(lui_Context *ctx) {
    get_layout(ctx)->indent -= ctx->style->indent;
    lui_pop_id(ctx);
}

#define scrollbar(ctx, cnt, b, cs, x, y, w, h)                                    \
    do {                                                                          \
        /* only add scrollbar if content size is larger than body */              \
        int maxscroll = cs.y - b->h;                                              \
                                                                                  \
        if (maxscroll > 0 && b->h > 0) {                                          \
            lui_Rect base, thumb;                                                 \
            lui_Id id = lui_get_id(ctx, "!scrollbar" #y, 11);                     \
                                                                                  \
            /* get sizing / positioning */                                        \
            base = *b;                                                            \
            base.x = b->x + b->w;                                                 \
            base.w = ctx->style->scrollbar_size;                                  \
                                                                                  \
            /* handle input */                                                    \
            lui_update_control(ctx, id, base, 0);                                 \
            if (ctx->focus == id && ctx->mouse_down == LUI_MOUSE_LEFT) {          \
                cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;              \
            }                                                                     \
            /* clamp scroll to limits */                                          \
            cnt->scroll.y = lui_clamp(cnt->scroll.y, 0, maxscroll);               \
                                                                                  \
            /* draw base and thumb */                                             \
            ctx->draw_frame(ctx, base, LUI_COLOR_SCROLLBASE);                     \
            thumb = base;                                                         \
            thumb.h = lui_max(ctx->style->thumb_size, base.h * b->h / cs.y);      \
            thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll;            \
            ctx->draw_frame(ctx, thumb, LUI_COLOR_SCROLLTHUMB);                   \
                                                                                  \
            /* set this as the scroll_target (will get scrolled on mousewheel) */ \
            /* if the mouse is over it */                                         \
            if (lui_mouse_over(ctx, *b)) {                                        \
                ctx->scroll_target = cnt;                                         \
            }                                                                     \
        } else {                                                                  \
            cnt->scroll.y = 0;                                                    \
        }                                                                         \
    } while (0)

static void scrollbars(lui_Context *ctx, lui_Container *cnt, lui_Rect *body) {
    int sz = ctx->style->scrollbar_size;
    lui_Vec2 cs = cnt->content_size;
    cs.x += ctx->style->padding * 2;
    cs.y += ctx->style->padding * 2;
    lui_push_clip_rect(ctx, *body);
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
    lui_pop_clip_rect(ctx);
}

static void push_container_body(lui_Context *ctx, lui_Container *cnt, lui_Rect body, int opt) {
    if (~opt & LUI_OPT_NOSCROLL) {
        scrollbars(ctx, cnt, &body);
    }
    push_layout(ctx, expand_rect(body, -ctx->style->padding), cnt->scroll);
    cnt->body = body;
}

static void begin_root_container(lui_Context *ctx, lui_Container *cnt) {
    lui_push(ctx->container_stack, cnt);
    /* lui_push container to roots list and lui_push head command */
    lui_push(ctx->root_list, cnt);
    cnt->head = push_jump(ctx, NULL);
    /* set as hover root if the mouse is overlapping this container and it has a
    ** higher zindex than the current hover root */
    if (rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) && (!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex)) {
        ctx->next_hover_root = cnt;
    }
    /* clipping is reset here in case a root-container is made within
    ** another root-containers's begin/end block; this prevents the inner
    ** root-container being clipped to the outer */
    lui_push(ctx->clip_stack, unclipped_rect);
}

static void end_root_container(lui_Context *ctx) {
    /* lui_push tail 'goto' jump command and set head 'skip' command. the final steps
    ** on initing these are done in lui_end() */
    lui_Container *cnt = lui_get_current_container(ctx);
    cnt->tail = push_jump(ctx, NULL);
    cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;
    /* lui_pop base clip rect and container */
    lui_pop_clip_rect(ctx);
    pop_container(ctx);
}

int lui_begin_window_ex(lui_Context *ctx, const char *title, lui_Rect rect, int opt) {
    lui_Rect body;
    lui_Id id = lui_get_id(ctx, title, strlen(title));
    lui_Container *cnt = get_container(ctx, id, opt);
    if (!cnt || !cnt->open) {
        return 0;
    }
    lui_push(ctx->id_stack, id);

    if (cnt->rect.w == 0) {
        cnt->rect = rect;
    }
    begin_root_container(ctx, cnt);
    rect = body = cnt->rect;

    /* draw frame */
    if (~opt & LUI_OPT_NOFRAME) {
        ctx->draw_frame(ctx, rect, LUI_COLOR_WINDOWBG);
    }

    /* do title bar */
    if (~opt & LUI_OPT_NOTITLE) {
        lui_Rect tr = rect;
        tr.h = ctx->style->title_height;
        ctx->draw_frame(ctx, tr, LUI_COLOR_TITLEBG);

        /* do title text */
        if (~opt & LUI_OPT_NOTITLE) {
            lui_Id id = lui_get_id(ctx, "!title", 6);
            lui_update_control(ctx, id, tr, opt);
            lui_draw_control_text(ctx, title, tr, LUI_COLOR_TITLETEXT, opt);
            if (id == ctx->focus && ctx->mouse_down == LUI_MOUSE_LEFT) {
                cnt->rect.x += ctx->mouse_delta.x;
                cnt->rect.y += ctx->mouse_delta.y;
            }
            body.y += tr.h;
            body.h -= tr.h;
        }

        /* do `close` button */
        if (~opt & LUI_OPT_NOCLOSE) {
            lui_Id id = lui_get_id(ctx, "!close", 6);
            lui_Rect r = lui_rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
            tr.w -= r.w;
            lui_draw_icon(ctx, LUI_ICON_CLOSE, r, ctx->style->colors[LUI_COLOR_TITLETEXT]);
            lui_update_control(ctx, id, r, opt);
            if (ctx->mouse_pressed == LUI_MOUSE_LEFT && id == ctx->focus) {
                cnt->open = 0;
            }
        }
    }

    push_container_body(ctx, cnt, body, opt);

    /* do `resize` handle */
    if (~opt & LUI_OPT_NORESIZE) {
        int sz = ctx->style->title_height;
        lui_Id id = lui_get_id(ctx, "!resize", 7);
        lui_Rect r = lui_rect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
        lui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == LUI_MOUSE_LEFT) {
            cnt->rect.w = lui_max(96, cnt->rect.w + ctx->mouse_delta.x);
            cnt->rect.h = lui_max(64, cnt->rect.h + ctx->mouse_delta.y);
        }
    }

    /* resize to content size */
    if (opt & LUI_OPT_AUTOSIZE) {
        lui_Rect r = get_layout(ctx)->body;
        cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
        cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
    }

    /* close if this is a popup window and elsewhere was clicked */
    if (opt & LUI_OPT_POPUP && ctx->mouse_pressed && ctx->hover_root != cnt) {
        cnt->open = 0;
    }

    lui_push_clip_rect(ctx, cnt->body);
    return LUI_RES_ACTIVE;
}

void lui_end_window(lui_Context *ctx) {
    lui_pop_clip_rect(ctx);
    end_root_container(ctx);
}

void lui_open_popup(lui_Context *ctx, const char *name) {
    lui_Container *cnt = lui_get_container(ctx, name);
    /* set as hover root so popup isn't closed in begin_window_ex()  */
    ctx->hover_root = ctx->next_hover_root = cnt;
    /* position at mouse cursor, open and bring-to-front */
    cnt->rect = lui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 1, 1);
    cnt->open = 1;
    lui_bring_to_front(ctx, cnt);
}

int lui_begin_popup(lui_Context *ctx, const char *name) {
    int opt = LUI_OPT_POPUP | LUI_OPT_AUTOSIZE | LUI_OPT_NORESIZE | LUI_OPT_NOSCROLL | LUI_OPT_NOTITLE | LUI_OPT_CLOSED;
    return lui_begin_window_ex(ctx, name, lui_rect(0, 0, 0, 0), opt);
}

void lui_end_popup(lui_Context *ctx) { lui_end_window(ctx); }

void lui_begin_panel_ex(lui_Context *ctx, const char *name, int opt) {
    lui_Container *cnt;
    lui_push_id(ctx, name, strlen(name));
    cnt = get_container(ctx, ctx->last_id, opt);
    cnt->rect = lui_layout_next(ctx);
    if (~opt & LUI_OPT_NOFRAME) {
        ctx->draw_frame(ctx, cnt->rect, LUI_COLOR_PANELBG);
    }
    lui_push(ctx->container_stack, cnt);
    push_container_body(ctx, cnt, cnt->rect, opt);
    lui_push_clip_rect(ctx, cnt->body);
}

void lui_end_panel(lui_Context *ctx) {
    lui_pop_clip_rect(ctx);
    pop_container(ctx);
}

struct MicrouiState {
    lui_Context *ctx;
    u32 atlas;
};

static MicrouiState g_mui_state;

lui_Context *microui_ctx() { return g_mui_state.ctx; }

void microui_init() {
    lui_Context *ctx = (lui_Context *)mem_alloc(sizeof(lui_Context));
    lui_init(ctx);

    ctx->text_width = [](lui_Font font, const char *text, int len) -> int {
        if (len == -1) {
            len = strlen(text);
        }

        int res = 0;
        for (const char *p = text; *p && len--; p++) {
            res += lui_atlas_lookup(LUI_ATLAS_FONT + (unsigned char)*p).w;
        }
        return res;
    };

    ctx->text_height = [](lui_Font font) -> int { return 18; };

    g_mui_state.ctx = ctx;

    u32 *bitmap = (u32 *)mem_alloc(LUI_ATLAS_WIDTH * LUI_ATLAS_HEIGHT * 4);
    neko_defer(mem_free(bitmap));

    for (i32 i = 0; i < LUI_ATLAS_WIDTH * LUI_ATLAS_HEIGHT; i++) {
        bitmap[i] = 0x00FFFFFF | ((u32)lui_atlas_texture[i] << 24);
    }

    sg_image_desc desc = {};
    desc.width = LUI_ATLAS_WIDTH;
    desc.height = LUI_ATLAS_HEIGHT;
    desc.data.subimage[0][0].ptr = bitmap;
    desc.data.subimage[0][0].size = LUI_ATLAS_WIDTH * LUI_ATLAS_HEIGHT * 4;
    g_mui_state.atlas = sg_make_image(&desc).id;
}

void microui_trash() { mem_free(g_mui_state.ctx); }

static char mui_key_map(sapp_keycode code) {
    switch (code & 511) {
        case SAPP_KEYCODE_LEFT_SHIFT:
            return LUI_KEY_SHIFT;
        case SAPP_KEYCODE_RIGHT_SHIFT:
            return LUI_KEY_SHIFT;
        case SAPP_KEYCODE_LEFT_CONTROL:
            return LUI_KEY_CTRL;
        case SAPP_KEYCODE_RIGHT_CONTROL:
            return LUI_KEY_CTRL;
        case SAPP_KEYCODE_LEFT_ALT:
            return LUI_KEY_ALT;
        case SAPP_KEYCODE_RIGHT_ALT:
            return LUI_KEY_ALT;
        case SAPP_KEYCODE_ENTER:
            return LUI_KEY_RETURN;
        case SAPP_KEYCODE_BACKSPACE:
            return LUI_KEY_BACKSPACE;
        default:
            return 0;
    }
}

void microui_sokol_event(const sapp_event *e) {
    switch (e->type) {
        case SAPP_EVENTTYPE_CHAR: {
            char str[2] = {(char)(e->char_code % 256), 0};
            lui_input_text(g_mui_state.ctx, str);
            break;
        }
        case SAPP_EVENTTYPE_KEY_DOWN:
            lui_input_keydown(g_mui_state.ctx, mui_key_map(e->key_code));
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            lui_input_keyup(g_mui_state.ctx, mui_key_map(e->key_code));
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            lui_input_mousedown(g_mui_state.ctx, e->mouse_x, e->mouse_y, (1 << e->mouse_button));
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            lui_input_mouseup(g_mui_state.ctx, e->mouse_x, e->mouse_y, (1 << e->mouse_button));
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            lui_input_mousemove(g_mui_state.ctx, e->mouse_x, e->mouse_y);
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            lui_input_scroll(g_mui_state.ctx, e->scroll_x * 5, -e->scroll_y * 5);
            break;
        default:
            break;
    }
}

static void lui_push_quad(lui_Rect dst, lui_Rect src, lui_Color color) {
    sgl_begin_quads();

    float u0 = (float)src.x / (float)LUI_ATLAS_WIDTH;
    float v0 = (float)src.y / (float)LUI_ATLAS_HEIGHT;
    float u1 = (float)(src.x + src.w) / (float)LUI_ATLAS_WIDTH;
    float v1 = (float)(src.y + src.h) / (float)LUI_ATLAS_HEIGHT;

    float x0 = (float)dst.x;
    float y0 = (float)dst.y;
    float x1 = (float)(dst.x + dst.w);
    float y1 = (float)(dst.y + dst.h);

    sgl_c4b(color.r, color.g, color.b, color.a);
    sgl_v2f_t2f(x0, y0, u0, v0);
    sgl_v2f_t2f(x1, y0, u1, v0);
    sgl_v2f_t2f(x1, y1, u1, v1);
    sgl_v2f_t2f(x0, y1, u0, v1);

    sgl_end();
}

void microui_begin() { lui_begin(g_mui_state.ctx); }

void microui_end_and_present() {
    bool ok = false;
    if (g_mui_state.ctx->container_stack.idx != 0) {
        fatal_error("microui container stack is not empty");
    } else if (g_mui_state.ctx->clip_stack.idx != 0) {
        fatal_error("microui clip stack is not empty");
    } else if (g_mui_state.ctx->id_stack.idx != 0) {
        fatal_error("microui id stack is not empty");
    } else if (g_mui_state.ctx->layout_stack.idx != 0) {
        fatal_error("microui layout stack is not empty");
    } else {
        ok = true;
    }

    if (!ok) {
        return;
    }

    lui_end(g_mui_state.ctx);

    sgl_enable_texture();
    sgl_texture({g_mui_state.atlas}, {});

    {
        lui_Command *cmd = 0;
        while (lui_next_command(g_mui_state.ctx, &cmd)) {
            switch (cmd->type) {
                case LUI_COMMAND_TEXT: {
                    lui_Rect dst = {cmd->text.pos.x, cmd->text.pos.y, 0, 0};
                    for (const char *p = cmd->text.str; *p; p++) {
                        lui_Rect src = lui_atlas_lookup(LUI_ATLAS_FONT + (unsigned char)*p);
                        dst.w = src.w;
                        dst.h = src.h;
                        lui_push_quad(dst, src, cmd->text.color);
                        dst.x += dst.w;
                    }
                    break;
                }
                case LUI_COMMAND_RECT: {
                    lui_push_quad(cmd->rect.rect, lui_atlas_lookup(LUI_ATLAS_WHITE), cmd->rect.color);
                    break;
                }
                case LUI_COMMAND_ICON: {
                    lui_Rect rect = cmd->icon.rect;
                    lui_Rect src = lui_atlas_lookup(cmd->icon.id);

                    int x = rect.x + (rect.w - src.w) / 2;
                    int y = rect.y + (rect.h - src.h) / 2;
                    lui_push_quad(lui_rect(x, y, src.w, src.h), src, cmd->icon.color);
                    break;
                }
                case LUI_COMMAND_CLIP: {
                    lui_Rect rect = cmd->clip.rect;
                    sgl_scissor_rect(rect.x, rect.y, rect.w, rect.h, true);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

lui_Rect lua_lui_check_rect(lua_State *L, i32 arg) {
    lui_Rect rect = {};
    rect.x = luax_number_field(L, arg, "x");
    rect.y = luax_number_field(L, arg, "y");
    rect.w = luax_number_field(L, arg, "w");
    rect.h = luax_number_field(L, arg, "h");
    return rect;
}

void lua_lui_rect_push(lua_State *L, lui_Rect rect) {
    lua_createtable(L, 0, 4);
    luax_set_number_field(L, "x", rect.x);
    luax_set_number_field(L, "y", rect.y);
    luax_set_number_field(L, "w", rect.w);
    luax_set_number_field(L, "h", rect.h);
}

lui_Color lua_lui_check_color(lua_State *L, i32 arg) {
    lui_Color color = {};
    color.r = luax_number_field(L, arg, "r");
    color.g = luax_number_field(L, arg, "g");
    color.b = luax_number_field(L, arg, "b");
    color.a = luax_number_field(L, arg, "a");
    return color;
}

void lua_lui_set_ref(lua_State *L, MUIRef *ref, i32 arg) {
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

MUIRef *lua_lui_check_ref(lua_State *L, i32 arg, MUIRefKind kind) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, arg, "mt_lui_ref");

    if (ref->kind != kind) {
        memset(ref, 0, sizeof(MUIRef));
        ref->kind = kind;
    }

    return ref;
}
