#include "engine/edit.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <typeindex>
#include <vector>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/scripting.h"
#include "engine/ui.h"

// deps
#include <stb_image.h>
#include <stb_rect_pack.h>
#include <stb_image_write.h>

static int limginfo(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int x, y, comp;
    if (!stbi_info(filename, &x, &y, &comp)) {
        return luaL_error(L, "%s", stbi_failure_reason());
    }
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, comp);
    return 3;
}

static int find_firstline(unsigned char *data, int w, int h) {
    int i;
    for (i = 0; i < w * h; i++) {
        if (data[i * 4 + 3] != 0) break;
    }
    return i / w;
}

static int find_lastline(unsigned char *data, int w, int h) {
    int i;
    for (i = w * h - 1; i >= 0; i--) {
        if (data[i * 4 + 3] != 0) break;
    }
    return i / w + 1;
}

static int find_firstrow(unsigned char *data, int w, int h) {
    int i, j;
    unsigned char *row = data;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            if (row[j * w * 4 + 3] != 0) {
                return i;
            }
        }
        row += 4;
    }
    return w;
}

static int find_lastrow(unsigned char *data, int w, int h) {
    int i, j;
    unsigned char *row = data + 4 * w;
    for (i = w - 1; i >= 0; i--) {
        row -= 4;
        for (j = 0; j < h; j++) {
            if (row[j * w * 4 + 3] != 0) {
                return i + 1;
            }
        }
    }
    return 0;
}

int spritepack_bake(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int w, h, n;
    // unsigned char *data = stbi_load(filename, &w, &h, &n, 0);

    String contents = {};
    bool ok = the<VFS>().read_entire_file(&contents, filename);
    if (!ok) {
        return luaL_error(L, "vfs can't open file %s", filename);
    }
    neko_defer(mem_free(contents.data));

    u8 *data = nullptr;
    {
        PROFILE_BLOCK("stb_image load");
        stbi_set_flip_vertically_on_load(false);
        data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &w, &h, &n, 0);
    }

    if (data == NULL) {
        return luaL_error(L, "can't parse %s", filename);
    }
    if (n != 4) {
        stbi_image_free(data);
        return luaL_error(L, "No alpha channel for %s", filename);
    }
    int firstline = find_firstline(data, w, h);
    if (firstline >= h) {
        stbi_image_free(data);
        // empty
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 4;
    }
    int lastline = find_lastline(data, w, h);
    int firstrow = find_firstrow(data, w, h);
    int lastrow = find_lastrow(data, w, h);
    stbi_image_free(data);
    lua_pushinteger(L, lastrow - firstrow);
    lua_pushinteger(L, lastline - firstline);
    lua_pushinteger(L, firstrow);
    lua_pushinteger(L, firstline);
    return 4;
}

static void read_rect(lua_State *L, int index, int id, stbrp_rect *rect) {
    if (lua_geti(L, index, id + 1) != LUA_TTABLE) {
        luaL_error(L, "Invalid rect at %d", id + 1);
    }
    if (lua_geti(L, -1, 1) != LUA_TNUMBER) {
        luaL_error(L, "Invalid rect at %d", id + 1);
    }
    int w = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (w <= 0) luaL_error(L, "Invalid rect %d width %d", id + 1, w);

    if (lua_geti(L, -1, 2) != LUA_TNUMBER) {
        luaL_error(L, "Invalid rect at %d", id + 1);
    }
    int h = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (h <= 0) luaL_error(L, "Invalid rect %d height %d", id + 1, h);
    rect->id = id;
    rect->w = w;
    rect->h = h;
    rect->x = 0;
    rect->y = 0;
    rect->was_packed = 0;
    lua_pop(L, 1);
}

static void write_rect(lua_State *L, int index, int id, stbrp_rect *rect) {
    if (rect->was_packed) {
        lua_geti(L, index, id + 1);
        lua_pushinteger(L, rect->x);
        lua_setfield(L, -2, "x");
        lua_pushinteger(L, rect->y);
        lua_setfield(L, -2, "y");
        lua_pop(L, 1);
    }
}

static int lrectpack(lua_State *L) {
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);
    int n = lua_rawlen(L, 3);
    stbrp_rect *rect = (stbrp_rect *)lua_newuserdata(L, n * sizeof(stbrp_rect));
    int i;
    for (i = 0; i < n; i++) read_rect(L, 3, i, &rect[i]);

    stbrp_context ctx;
    stbrp_node *nodes = (stbrp_node *)lua_newuserdata(L, w * sizeof(stbrp_node));
    stbrp_init_target(&ctx, w, h, nodes, w);

    int r = stbrp_pack_rects(&ctx, rect, n);
    for (i = 0; i < n; i++) write_rect(L, 3, i, &rect[i]);
    lua_pushboolean(L, r);
    return 1;
}

static int geti(lua_State *L, int idx) {
    if (lua_geti(L, -1, idx) != LUA_TNUMBER) {
        luaL_error(L, "Invalid image");
    }
    int r = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return r;
}

static int geti_field(lua_State *L, const char *key) {
    if (lua_getfield(L, -1, key) != LUA_TNUMBER) {
        luaL_error(L, "Invalid image");
    }
    int r = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return r;
}

static inline int rect_invalid(int w, int h, int x, int y, int width, int height) { return !(x + w >= width || y + h >= height); }

static unsigned char *read_img(lua_State *L, int *w, int *h, int *x, int *y, int *stride, int *px, int *py) {
    if (lua_getfield(L, -1, "filename") != LUA_TSTRING) luaL_error(L, "No filename");
    const char *filename = lua_tostring(L, -1);
    lua_pop(L, 1);
    *w = geti(L, 1);
    *h = geti(L, 2);
    *x = geti(L, 3);
    *y = geti(L, 4);
    *px = geti_field(L, "x");
    *py = geti_field(L, "y");

    int width, height, comp;

    String contents = {};
    bool ok = the<VFS>().read_entire_file(&contents, filename);
    if (!ok) {
        luaL_error(L, "load %s failed", filename);
    }
    neko_defer(mem_free(contents.data));

    u8 *data = nullptr;
    {
        PROFILE_BLOCK("stb_image load");
        stbi_set_flip_vertically_on_load(false);
        data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &width, &height, &comp, 0);
    }

    if (comp != 4 || rect_invalid(*w, *h, *x, *y, width, height)) {
        stbi_image_free(data);
        luaL_error(L, "Load %s failed", filename);
    }
    *stride = width;
    return data;
}

static void copy_img(lua_State *L, int index, int id, unsigned char *canvas, int canvas_w, int canvas_h) {
    if (lua_geti(L, index, id + 1) != LUA_TTABLE) {
        luaL_error(L, "Invalid image at %d", id + 1);
    }
    int x, y, w, h, stride, px, py;
    unsigned char *img = read_img(L, &w, &h, &x, &y, &stride, &px, &py);
    int i;
    if (w + px >= canvas_w || h + py >= canvas_h) {
        stbi_image_free(img);
        luaL_error(L, "Invalid image %dx%d+%d+%d", w, h, px, py);
    }
    for (i = 0; i < h; i++) {
        memcpy(canvas + (canvas_w * (py + i) + px) * 4, img + (stride * (y + i) + x) * 4, w * 4);
    }

    stbi_image_free(img);
    lua_pop(L, 1);
}

static int limgpack(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    luaL_checktype(L, 4, LUA_TTABLE);
    unsigned char *data = (unsigned char *)lua_newuserdata(L, w * h * 4);
    memset(data, 0, w * h * 4);
    int i;
    int n = lua_rawlen(L, 4);
    for (i = 0; i < n; i++) copy_img(L, 4, i, data, w, h);
    if (!stbi_write_png(filename, w, h, 4, data, w * 4)) return luaL_error(L, "Can't write to %s", filename);
    return 0;
}

int open_tools_spritepack(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"imginfo", limginfo}, {"imgcrop", spritepack_bake}, {"rectpack", lrectpack}, {"imgpack", limgpack}, {NULL, NULL},
    };
    luaL_newlib(L, l);
    return 1;
}
