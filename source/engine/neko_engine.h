
#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/neko.h"
#include "engine/neko_asset.h"
#include "engine/neko_ecs.h"
#include "engine/neko_lua.h"
#include "engine/neko_platform.h"
#include "engine/neko_render.h"

/*==========================
// NEKO_ENGINE / NEKO_APP
==========================*/

typedef struct neko_t {
    neko_platform_t* platform;
    neko_render_t* render;

    void (*init)();
    void (*update)();
    void (*shutdown)();
    void (*fini)();

    struct {
        b32 is_running;
        b32 debug_gfx;
        void* user_data;
    } game;

    neko_console_t* console;
    neko_config_t* config;

    lua_State* L;

    int argc;
    char** argv;
} neko_t;

NEKO_API_DECL neko_t* neko_create(int argc, char** argv);
NEKO_API_DECL void neko_fini();
NEKO_API_DECL void neko_frame();
NEKO_API_DECL void neko_quit();
NEKO_API_DECL void neko_app();

NEKO_API_DECL neko_t g_neko_instance;

NEKO_API_DECL neko_t* neko_instance();

#define neko_subsystem(__T) (neko_instance()->__T)

#define neko_cv() (neko_instance()->config)

#define neko_userdata() (neko_instance()->game.user_data)

typedef struct Neko_CommonInterface {
    void* (*__neko_mem_safe_alloc)(size_t size, const char* file, int line, size_t* statistics);
    void* (*__neko_mem_safe_calloc)(size_t count, size_t element_size, const char* file, int line, size_t* statistics);
    void (*__neko_mem_safe_free)(void* mem, size_t* statistics);
    void* (*__neko_mem_safe_realloc)(void* ptr, size_t new_size, const char* file, int line, size_t* statistics);
} Neko_CommonInterface;

typedef struct Neko_ModuleInterface {

    neko_command_buffer_t cb;
    neko_ui_context_t ui;
    neko_immediate_draw_t idraw;
    neko_asset_ascii_font_t font;
    neko_texture_t test_ase;
    neko_asset_manager_t am;
    neko_asset_t tex_hndl;
    neko_packreader_t pack;
    neko_packreader_t lua_pack;

    // neko_ui_style_sheet_t style_sheet;
    // ecs_world_t* ecs_world;
    // neko_dyn_array(ecs_entity_t) entities;
    neko_ecs_t* ecs;

    Neko_CommonInterface common;
} Neko_ModuleInterface;

NEKO_API_DECL void neko_module_interface_init(Neko_ModuleInterface* interface);

#endif