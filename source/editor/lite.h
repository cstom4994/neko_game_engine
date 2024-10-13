// Lite - A lightweight text editor written in Lua
// modified from https://github.com/rxi/lite (MIT license)
//               https://github.com/r-lyeh/FWK (public domain)
// modified by KaoruXun(cstom4994) for NekoEngine

#ifndef NEKO_LITE_H
#define NEKO_LITE_H

#include "engine/asset.h"
#include "engine/bootstrap.h"
#include "engine/graphics.h"
#include "engine/scripting/luax.h"

// ----------------------------------------------------------------------------
// lite editor, platform details

#define LT_DATAPATH "./"

typedef struct lt_surface {
    int w, h;
    void *pixels;
    AssetTexture t;
} lt_surface;

typedef struct lt_rect {
    int x, y, width, height;
} lt_rect;

extern int lt_mx, lt_my, lt_wx, lt_wy, lt_ww, lt_wh;

lt_surface *lt_getsurface();

int lt_resizesurface(lt_surface *s, int ww, int wh);

// ----------------------------------------------------------------------------

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

#endif