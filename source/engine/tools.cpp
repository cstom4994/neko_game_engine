
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_rect_pack.h>

#include "engine/luax.h"

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

static int limgcrop(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);  // todo : UTF-8 support
    int w, h, n;
    unsigned char *data = stbi_load(filename, &w, &h, &n, 0);
    if (data == NULL) {
        return luaL_error(L, "Can't parse %s", filename);
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
    unsigned char *img = stbi_load(filename, &width, &height, &comp, 0);
    if (comp != 4 || rect_invalid(*w, *h, *x, *y, width, height)) {
        stbi_image_free(img);
        luaL_error(L, "Load %s failed", filename);
    }
    *stride = width;
    return img;
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
            {"imginfo", limginfo}, {"imgcrop", limgcrop}, {"rectpack", lrectpack}, {"imgpack", limgpack}, {NULL, NULL},
    };
    luaL_newlib(L, l);
    return 1;
}

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(NEKO_IS_WIN32)
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef NEKO_IS_WIN32
#define SEP "\\"
#define ALLSEPS "\\/"
#define FUNC_STAT _stati64
#define FUNC_LSTAT FUNC_STAT
#define STRUCT_STAT struct _stat64
#define getcwd(d, s) _getcwd(d, s)
#define rmdir(p) _rmdir(p)
#else
#define SEP "/"
#define ALLSEPS "/"
#define FUNC_STAT stat
#define FUNC_LSTAT lstat
#define STRUCT_STAT struct stat
#endif

#define DIR_ITR "CO_DIR_ITR"

typedef struct diritr {
    int closed;  // 是否已经关闭
#if defined(NEKO_IS_WIN32)
    intptr_t handle;
    char *path;
    int first_time;
#else
    DIR *dir;
#endif
} diritr_t;

static void _diritr_close(diritr_t *itr) {
    if (itr->closed) return;
#if defined(NEKO_IS_WIN32)
    if (itr->handle != -1L) _findclose(itr->handle);
    if (itr->path) mem_free(itr->path);
#else
    if (itr->dir) closedir(itr->dir);
#endif
    itr->closed = 1;
}

static int l_diritr_close(lua_State *L) {
    diritr_t *itr = (diritr_t *)luaL_checkudata(L, 1, DIR_ITR);
    _diritr_close(itr);
    return 0;
}

static int _diritr_next(lua_State *L) {
    diritr_t *itr = (diritr_t *)lua_touserdata(L, lua_upvalueindex(1));
    if (itr->closed) luaL_error(L, "Scan a closed directory");
#if defined(NEKO_IS_WIN32)
    struct _finddata_t finddata;
    while (1) {
        if (itr->first_time) {
            itr->first_time = 0;
            itr->handle = _findfirst(itr->path, &finddata);
            if (itr->handle == -1L) {
                _diritr_close(itr);
                if (errno != ENOENT) {
                    lua_pushnil(L);
                    lua_pushstring(L, strerror(errno));
                    return 2;
                } else {
                    return 0;
                }
            }
        } else {
            if (_findnext(itr->handle, &finddata) == -1L) {
                _diritr_close(itr);
                return 0;
            }
        }
        if (strcmp(finddata.name, ".") == 0 || strcmp(finddata.name, "..") == 0) continue;
        lua_pushstring(L, finddata.name);
        return 1;
    }
#else
    struct dirent *ent;
    while (1) {
        ent = readdir(itr->dir);
        if (ent == NULL) {
            closedir(itr->dir);
            itr->closed = 1;
            return 0;
        }
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        lua_pushstring(L, ent->d_name);
        return 1;
    }
#endif
}

static int l_scandir(lua_State *L) {
    size_t pathsz;
    const char *path = luaL_checklstring(L, 1, &pathsz);
    diritr_t *itr = (diritr_t *)lua_newuserdata(L, sizeof(diritr_t));
    memset(itr, 0, sizeof(*itr));
    luaL_setmetatable(L, DIR_ITR);
#if defined(NEKO_IS_WIN32)
    itr->handle = -1L;
    itr->path = (char *)mem_alloc(pathsz + 5);
    strncpy(itr->path, path, pathsz);
    char ch = itr->path[pathsz - 1];
    if (!strchr(ALLSEPS, ch) && ch != ':') itr->path[pathsz++] = SEP[0];
    itr->path[pathsz++] = L'*';
    itr->path[pathsz] = '\0';
    itr->first_time = 1;
#else
    DIR *dir = opendir(path);
    if (dir == NULL) luaL_error(L, "cannot open %s: %s", path, strerror(errno));
    itr->dir = dir;
#endif
    lua_pushcclosure(L, _diritr_next, 1);
    return 1;
}

static int l_exists(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) == 0)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

static int l_getsize(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) == 0) {
        lua_pushinteger(L, (lua_Integer)st.st_size);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "getsize error: %s", strerror(errno));
        return 2;
    }
}

///////////////////////////////////////////////////////////////////////
// 消除一些系统的差异
#ifdef NEKO_IS_WIN32
// 取文件时间
static int win_getfiletime(const char *path, FILETIME *ftCreate, FILETIME *ftAccess, FILETIME *ftWrite) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }
    if (!GetFileTime(hFile, ftCreate, ftAccess, ftWrite)) {
        CloseHandle(hFile);
        return -1;
    } else {
        CloseHandle(hFile);
        return 0;
    }
}

static __int64 secs_between_epochs = 11644473600; /* Seconds between 1.1.1601 and 1.1.1970 */
// 将文件时间转换成秒和纳秒
static void win_convert_filetime(FILETIME *time_in, time_t *time_out, long *nsec_out) {
    __int64 in = (int64_t)time_in->dwHighDateTime << 32 | time_in->dwLowDateTime;
    *nsec_out = (long)(in % 10000000) * 100; /* FILETIME is in units of 100 nsec. */
    *time_out = (time_t)((in / 10000000) - secs_between_epochs);
}
#elif __APPLE__
#define st_mtim st_atimespec
#define st_atim st_mtimespec
#define st_ctim st_ctimespec
#else
#endif

static int l_getmtime(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    time_t sec;
    long nsec;
#ifdef NEKO_IS_WIN32
    FILETIME ftWrite;
    if (win_getfiletime(path, NULL, NULL, &ftWrite) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %d", GetLastError());
        return 2;
    }
    win_convert_filetime(&ftWrite, &sec, &nsec);
#else
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %s", strerror(errno));
        return 2;
    }
    sec = st.st_mtim.tv_sec;
    nsec = st.st_mtim.tv_nsec;
#endif
    lua_pushnumber(L, (lua_Number)(sec + 1e-9 * nsec));
    return 1;
}

static int l_getatime(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    time_t sec;
    long nsec;
#ifdef NEKO_IS_WIN32
    FILETIME ftAccess;
    if (win_getfiletime(path, NULL, &ftAccess, NULL) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getatime error: %d", GetLastError());
        return 2;
    }
    win_convert_filetime(&ftAccess, &sec, &nsec);
#else
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %s", strerror(errno));
        return 2;
    }
    sec = st.st_atim.tv_sec;
    nsec = st.st_atim.tv_nsec;
#endif
    lua_pushnumber(L, (lua_Number)(sec + 1e-9 * nsec));
    return 1;
}

static int l_getctime(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    time_t sec;
    long nsec;
#ifdef NEKO_IS_WIN32
    FILETIME ftCreate;
    if (win_getfiletime(path, &ftCreate, NULL, NULL) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getatime error: %d", GetLastError());
        return 2;
    }
    win_convert_filetime(&ftCreate, &sec, &nsec);
#else
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %s", strerror(errno));
        return 2;
    }
    sec = st.st_ctim.tv_sec;
    nsec = st.st_ctim.tv_nsec;
#endif
    lua_pushnumber(L, (lua_Number)(sec + 1e-9 * nsec));
    return 1;
}

static int l_getmode(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) == 0) {
        lua_pushinteger(L, (lua_Integer)st.st_mode);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "getmode error: %s", strerror(errno));
        return 2;
    }
}

static int l_getlinkmode(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_LSTAT(path, &st) == 0) {
        lua_pushinteger(L, (lua_Integer)st.st_mode);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "getlinkmode error: %s", strerror(errno));
        return 2;
    }
}

static int l_mkdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int err;
#ifdef NEKO_IS_WIN32
    err = _mkdir(path);
#else
    err = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    if (err) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "mkdir error: %s", strerror(errno));
        return 2;
    } else {
        lua_pushboolean(L, 1);
        return 1;
    }
}

static int l_rmdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int err = rmdir(path);
    if (err) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "rmdir error: %s", strerror(errno));
        return 2;
    } else {
        lua_pushboolean(L, 1);
        return 1;
    }
}

static int l_chdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    if (neko_os_chdir(path)) {
        lua_pushnil(L);
        lua_pushfstring(L, "chdir error: %s", strerror(errno));
        return 2;
    } else {
        lua_pushboolean(L, 1);
        return 1;
    }
}

static int l_getcwd(lua_State *L) {
    char path[256];
    int size = 256;
    if (getcwd(path, size) != NULL) {
        lua_pushstring(L, path);
        return 1;
    }
    char *buff = NULL;
    int result;
    while (1) {
        size <<= 1;
        buff = (char *)mem_realloc(buff, size);
        if (buff == NULL) {
            lua_pushnil(L);
            lua_pushfstring(L, "getcwd error: realloc() failed");
            result = 2;
            break;
        }
        if (getcwd(buff, size) != NULL) {
            lua_pushstring(L, buff);
            result = 1;
            break;
        }
        if (errno != ERANGE) {
            lua_pushnil(L);
            lua_pushfstring(L, "getcwd error: %s", strerror(errno));
            result = 2;
            break;
        }
    }
    mem_free(buff);
    return result;
}

static const luaL_Reg dirmt[] = {{"__gc", l_diritr_close}, {NULL, NULL}};

static const luaL_Reg lib[] = {{"scandir", l_scandir},
                               {"exists", l_exists},
                               {"getsize", l_getsize},
                               {"getmtime", l_getmtime},
                               {"getatime", l_getatime},
                               {"getctime", l_getctime},
                               {"getmode", l_getmode},
                               {"getlinkmode", l_getlinkmode},
                               {"mkdir", l_mkdir},
                               {"rmdir", l_rmdir},
                               {"chdir", l_chdir},
                               {"getcwd", l_getcwd},
                               {NULL, NULL}};

static void init_consts(lua_State *L) {
    lua_pushliteral(L, SEP);
    lua_setfield(L, -2, "sep");
    lua_pushliteral(L, ALLSEPS);
    lua_setfield(L, -2, "allseps");
#ifdef NEKO_IS_WIN32
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "iswindows");
#endif
}

int open_filesys(lua_State *L) {
    luaL_checkversion(L);
    luaL_newmetatable(L, DIR_ITR);
    luaL_setfuncs(L, dirmt, 0);

    luaL_newlib(L, lib);
    init_consts(L);
    return 1;
}
