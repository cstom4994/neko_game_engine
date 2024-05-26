
#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/neko.h"
#include "engine/neko_graphics.h"
#include "engine/neko_platform.h"

/*==========================
// NEKO_ENGINE / NEKO_APP
==========================*/

typedef struct neko_context_t {
    neko_platform_t* platform;
    neko_graphics_t* graphics;

    void (*shutdown)();
} neko_context_t;

typedef struct neko_t {
    neko_context_t ctx;

    void (*init)();
    void (*update)();
    void (*shutdown)();

    struct {

        // neko_platform_running_desc_t window;
        b32 is_running;
        b32 debug_gfx;
        void* user_data;

// Platform specific data
#ifdef NEKO_PLATFORM_ANDROID
        struct {
            void* activity;
            const char* internal_data_path;
        } android;
#endif

    } game;

    neko_console_t* console;
    neko_config_t* config;

    int argc;
    char** argv;
} neko_t;

NEKO_API_DECL neko_t* neko_create(int argc, char** argv);
NEKO_API_DECL void neko_destroy();
NEKO_API_DECL neko_t* neko_instance();
NEKO_API_DECL neko_context_t* neko_ctx();
NEKO_API_DECL void neko_frame();
NEKO_API_DECL void neko_quit();
NEKO_API_DECL s32 neko_buildnum(void);
// NEKO_API_DECL neko_game_desc_t* neko_main();
NEKO_API_DECL void neko_app();

#define neko_subsystem(__T) (neko_instance()->ctx.__T)

#define neko_cv() (neko_instance()->config)
// #define neko_ecs() (neko_instance()->ctx.game.ecs)

#define neko_user_data(__T) (__T*)(neko_instance()->ctx.game.user_data)

#endif