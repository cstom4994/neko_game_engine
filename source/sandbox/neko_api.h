#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/neko.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_engine.h"
#include "engine/neko_lua.h"
#include "engine/neko_math.h"

typedef struct neko_client_userdata_s {
    neko_command_buffer_t *cb;
    neko_immediate_draw_t *idraw;
    neko_immediate_draw_static_data_t *idraw_sd;
    neko_ui_context_t *core_ui;

    neko_packreader_t *pack;

    lua_State *L;

    neko_font_t *test_font_bmfont;
} neko_client_userdata_t;

typedef struct {
    const char *name;
    lua_CFunction func;
} neko_luaL_reg;

int neko_lua_hook_init(lua_State *L);

enum class neko_lua_dataType { number, string, integer, lua_bool, function };

enum class neko_lua_hook_status { hook_awaiting, hook_update, hook_idle };

struct neko_lua_hook_callbacks {
    size_t dataSize;
    void *data;
    enum neko_lua_dataType dataType;
};

struct neko_lua_hook_pool {
    struct neko_lua_hook_t *hooks;
    int count;
};

struct neko_lua_hook_t {
    const char *hookName;
    struct neko_lua_hook_stack *stack;
    size_t pool;
    struct neko_lua_hook_t *address;
    void (*handle)(struct neko_lua_hook_t *, lua_State *);
    enum neko_lua_hook_status status;
    struct neko_lua_hook_callbacks *callback;
};

struct neko_lua_hook_stack {
    const char *name;
    void (*func)(lua_State *, struct neko_lua_hook_t *instance, int, struct neko_lua_hook_callbacks *callback);
    int ref;
};

void neko_lua_hook_register(struct neko_lua_hook_t hookData);
void neko_lua_hook_add(struct neko_lua_hook_t *instance, const char *name, void (*func)(lua_State *, struct neko_lua_hook_t *instance, int, struct neko_lua_hook_callbacks *callback), int ref);
void neko_lua_hook_run(struct neko_lua_hook_t *instance, lua_State *L);
void neko_lua_hook_free(struct neko_lua_hook_t *instance, lua_State *L);
struct neko_lua_hook_t *neko_lua_hook_find(const char *hookName);
struct neko_lua_hook_callbacks *neko_lua_hook_callback_create(size_t dataSize, enum neko_lua_dataType dataType);
void *neko_lua_hook_callback_get(const struct neko_lua_hook_callbacks *callback);
void neko_lua_hook_callback_set(struct neko_lua_hook_callbacks *callback, const void *data);

extern struct neko_lua_hook_pool g_lua_hook_pool;

// TODO:
std::string game_assets(const std::string &path);
void draw_text(neko_font_t *font, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale);

void neko_register(lua_State *L);

neko_inline u64 generate_texture_handle(void *pixels, int w, int h, void *udata) {
    (void)udata;
    GLuint location;
    glGenTextures(1, &location);
    glBindTexture(GL_TEXTURE_2D, location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (u64)location;
}

neko_inline void destroy_texture_handle(u64 texture_id, void *udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

#endif