
#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/modules/ecslua.h"
#include "engine/neko.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_imgui.hpp"
#include "engine/neko_platform.h"
#include "engine/neko_render.h"

/*==========================
// NEKO_ENGINE / NEKO_APP
==========================*/

typedef struct lua_State lua_State;

typedef struct Neko_ModuleFunc {
    int (*OnInit)(lua_State* L);
    int (*OnFini)(lua_State* L);
    int (*OnPostUpdate)(lua_State* L);
    int (*OnUpdate)(lua_State* L);
} Neko_ModuleFunc;

typedef struct neko_module {
    Neko_ModuleFunc func;
} neko_module;

typedef struct Neko_ModuleInterface {

    neko_command_buffer_t cb;
    neko_ui_context_t ui;
    neko_immediate_draw_t idraw;
    neko_asset_font_t font;
    neko_texture_t test_ase;
    neko_asset_manager_t am;
    neko_asset_t tex_hndl;
    neko_pak pack;

    neko_dyn_array(neko_module) modules;

} Neko_ModuleInterface;

typedef struct neko_instance_t {
    neko_pf_t* platform;
    neko_render_t* render;

    struct {
        bool is_running;
        u8 debug_level;
        neko_vec2_t DisplaySize;
        neko_vec2_t DisplayFramebufferScale;
        b32 profiler_enable;
        neko_imgui_context_t imgui;
        neko_client_cvar_t cvar = NEKO_DEFAULT_VAL();

        // client render
        neko_handle(neko_render_renderpass_t) r_main_rp = {0};
        neko_handle(neko_render_framebuffer_t) r_main_fbo = {0};
        neko_handle(neko_render_texture_t) r_main_rt = {0};
    } game;

    neko_console_t* console;

    Neko_ModuleInterface module_interface;

    ecs_world_t* W;
    lua_State* L;
} neko_instance_t;

NEKO_API_DECL lua_State* neko_lua_bootstrap(int argc, char** argv);
NEKO_API_DECL neko_instance_t* neko_create(int argc, char** argv);
NEKO_API_DECL void neko_fini();
NEKO_API_DECL void neko_frame();
NEKO_API_DECL void neko_quit();

NEKO_API_DECL neko_instance_t g_neko_instance;

NEKO_API_DECL neko_instance_t* neko_instance();

#define neko_subsystem(__T) (neko_instance()->__T)

#define neko_cv() (neko_instance()->config)
#define neko_game() (&neko_instance()->game)

NEKO_INLINE ecs_world_t*& ENGINE_ECS() { return neko_instance()->W; }
NEKO_INLINE lua_State*& ENGINE_LUA() { return neko_instance()->L; }
NEKO_INLINE Neko_ModuleInterface* ENGINE_INTERFACE() { return &neko_instance()->module_interface; }

NEKO_API_DECL void neko_module_interface_init(Neko_ModuleInterface* module_interface);
NEKO_API_DECL void neko_module_interface_fini(Neko_ModuleInterface* module_interface);

#define Neko_OnModuleLoad(N, ...) NEKO_API_DECL s32 Neko_OnModuleLoad_##N(neko_module* self, Neko_ModuleInterface* module_interface)

#endif