
#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/neko.h"
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

    int argc;
    char** argv;
} neko_t;

NEKO_API_DECL neko_t* neko_create(int argc, char** argv);
NEKO_API_DECL void neko_fini();
NEKO_API_DECL neko_t* neko_instance();
NEKO_API_DECL void neko_frame();
NEKO_API_DECL void neko_quit();
NEKO_API_DECL s32 neko_buildnum(void);
NEKO_API_DECL void neko_app();

#define neko_subsystem(__T) (neko_instance()->__T)

#define neko_cv() (neko_instance()->config)

#define neko_userdata() (neko_instance()->game.user_data)

#endif