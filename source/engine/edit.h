#ifndef EDIT_H
#define EDIT_H

#include <stdbool.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/bootstrap.h"
#include "engine/entity.h"
#include "engine/graphics.h"
#include "engine/luax.hpp"

NEKO_SCRIPT(
        edit,

        NEKO_EXPORT void edit_set_enabled(bool e);

        NEKO_EXPORT bool edit_get_enabled();

        // 无法选择不可编辑的实体
        NEKO_EXPORT void edit_set_editable(Entity ent, bool editable);

        NEKO_EXPORT bool edit_get_editable(Entity ent);

        // 每个维度上都是非负的 零意味着没有网格
        NEKO_EXPORT void edit_set_grid_size(vec2 size);

        NEKO_EXPORT vec2 edit_get_grid_size();

        // 用于点击选择等
        NEKO_EXPORT void edit_bboxes_update(Entity ent, BBox bbox);  // 合并bbox

        NEKO_EXPORT bool edit_bboxes_has(Entity ent);

        NEKO_EXPORT BBox edit_bboxes_get(Entity ent);

        NEKO_EXPORT unsigned int edit_bboxes_get_num();

        struct EntityBBoxPair {
            Entity ent;
            BBox bbox;
        };

        NEKO_EXPORT Entity edit_bboxes_get_nth_ent(unsigned int n); NEKO_EXPORT BBox edit_bboxes_get_nth_bbox(unsigned int n);

        NEKO_EXPORT void edit_bboxes_set_selected(Entity ent, bool selected);

        // 在两个世界空间坐标之间画一条线
        NEKO_EXPORT void edit_line_add(vec2 a, vec2 b, Scalar point_size, Color color);

)

void edit_clear();

void edit_init();
void edit_fini();
void edit_update_all();
void edit_draw_all();
void edit_save_all(Store *s);
void edit_load_all(Store *s);

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location);
void inspect_shader(const char *label, GLuint program);
void inspect_vertex_array(const char *label, GLuint vao);

#endif

// Lite - A lightweight text editor written in Lua
// modified from https://github.com/rxi/lite (MIT license)
//               https://github.com/r-lyeh/FWK (public domain)
// modified by KaoruXun(cstom4994) for NekoEngine

#ifndef NEKO_LITE_H
#define NEKO_LITE_H

#define NEKO_LITE

// ----------------------------------------------------------------------------
// lite editor, platform details

#define LT_DATAPATH "./"

#define lt_assert(x) neko_assert(x)

#define lt_realpath(p, q) file_pathabs(p)
#define lt_realpath_free(p)

#define lt_malloc(n) mem_alloc(n)
#define lt_calloc(n, m) mem_calloc(n, m)
#define lt_free(p) mem_free(p)
#define lt_memcpy(d, s, c) memcpy(d, s, c)
#define lt_memset(p, ch, c) memset(p, ch, c)

#define lt_time_ms() lt_time_now()
#define lt_sleep_ms(ms) os_sleep(ms)

#define lt_getclipboard(w) window_clipboard()
#define lt_setclipboard(w, s) window_setclipboard(s)

#define lt_window() g_app->game_window
// #define lt_setwindowmode(m) window_fullscreen(m == 2), (m < 2 && (window_maximize(m), 1))  // 0:normal,1:maximized,2:fullscreen
#define lt_setwindowmode(m)

#define lt_setwindowtitle(t)  // window_title(t)
#define lt_haswindowfocus() window_has_focus()
// #define lt_setcursor(shape) window_cursor_shape(lt_events & (1 << 31) ? CURSOR_SW_AUTO : shape + 1)  // 0:arrow,1:ibeam,2:sizeh,3:sizev,4:hand
#define lt_setcursor(shape)

#define lt_prompt(msg, title) (MessageBoxA(0, msg, title, MB_YESNO | MB_ICONWARNING) == IDYES)

typedef struct lt_surface {
    int w, h;
    void *pixels;
    AssetTexture t;
} lt_surface;

typedef struct lt_rect {
    int x, y, width, height;
} lt_rect;

extern int lt_mx, lt_my, lt_wx, lt_wy, lt_ww, lt_wh;

lt_surface *lt_getsurface(void *window);

int lt_resizesurface(lt_surface *s, int ww, int wh);

// ----------------------------------------------------------------------------

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

// ----------------------------------------------------------------------------
// lite/api.h

#define API_TYPE_FONT "Font"

// ----------------------------------------------------------------------------
// lite/renderer.h

typedef struct RenImage RenImage;
typedef struct RenFont RenFont;

typedef struct {
    uint8_t b, g, r, a;
} RenColor;
// typedef struct { int x, y, width, height; } RenRect;
typedef lt_rect RenRect;

void ren_init(void *win);
void ren_update_rects(RenRect *rects, int count);
void ren_set_clip_rect(RenRect rect);
void ren_get_size(int *x, int *y);

RenImage *ren_new_image(int width, int height);
void ren_free_image(RenImage *image);

RenFont *ren_load_font(const char *filename, float size);
void ren_free_font(RenFont *font);
void ren_set_font_tab_width(RenFont *font, int n);
int ren_get_font_tab_width(RenFont *font);
int ren_get_font_width(RenFont *font, const char *text);
int ren_get_font_height(RenFont *font);

void ren_draw_rect(RenRect rect, RenColor color);
void ren_draw_image(RenImage *image, RenRect *sub, int x, int y, RenColor color);
int ren_draw_text(RenFont *font, const char *text, int x, int y, RenColor color);

// ----------------------------------------------------------------------------
// lite/rencache.h

void rencache_show_debug(bool enable);
void rencache_free_font(RenFont *font);
void rencache_set_clip_rect(RenRect rect);
void rencache_draw_rect(RenRect rect, RenColor color);
int rencache_draw_text(RenFont *font, const char *text, int x, int y, RenColor color);
void rencache_invalidate(void);
void rencache_begin_frame(void);
void rencache_end_frame(void);

// neko lite
void lt_init(lua_State *L, void *handle, const char *pathdata, int argc, char **argv, float scale, const char *platform);
void lt_tick(struct lua_State *L);
void lt_fini();

NEKO_SCRIPT(console,

            // 打印到此实体 添加到 gui_text 系统 (如果尚未在其中) 设置为entity_nil以禁用
            NEKO_EXPORT void console_set_entity(Entity ent);

            NEKO_EXPORT Entity console_get_entity();  // 如果没有设置则entity_nil

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
