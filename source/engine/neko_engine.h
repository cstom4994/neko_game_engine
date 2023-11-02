
#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/neko.h"
#include "engine/neko_audio.h"
#include "engine/neko_graphics.h"
#include "engine/neko_platform.h"
// #include "engine/util/neko_console.h"

/*==========================
// NEKO_ENGINE / NEKO_APP
==========================*/

// Application descriptor for user application
typedef struct neko_game_desc_s {
    void (*init)();
    void (*update)();
    void (*shutdown)();
    neko_platform_running_desc_t window;
    b32 is_running;
    b32 debug_gfx;
    void* user_data;

    int argc;
    char** argv;

// Platform specific data
#ifdef NEKO_PLATFORM_ANDROID
    struct {
        void* activity;
        const char* internal_data_path;
    } android;
#endif

    neko_console_t* console;
    neko_config_t* config;
    neko_ecs* ecs;

} neko_game_desc_t;

typedef struct neko_context_t {
    neko_platform_t* platform;
    neko_graphics_t* graphics;
    neko_audio_t* audio;
    neko_game_desc_t game;
    neko_os_api_t os;
} neko_context_t;

typedef struct neko_t {
    neko_context_t ctx;
    void (*shutdown)();
} neko_t;

NEKO_API_DECL neko_t* neko_create(neko_game_desc_t app_desc);
NEKO_API_DECL void neko_destroy();
NEKO_API_DECL neko_t* neko_instance();
NEKO_API_DECL void neko_set_instance(neko_t*);
NEKO_API_DECL neko_context_t* neko_ctx();
NEKO_API_DECL neko_game_desc_t* neko_app();
NEKO_API_DECL void neko_frame();
NEKO_API_DECL void neko_quit();

// Engine utils
NEKO_API_DECL s32 neko_buildnum(void);

// Impl in game code
NEKO_API_DECL neko_game_desc_t neko_main(int32_t argc, char** argv);

#define neko_subsystem(__T) (neko_instance()->ctx.__T)

#define neko_cv() (neko_instance()->ctx.game.config)
#define neko_ecs() (neko_instance()->ctx.game.ecs)

#define neko_user_data(__T) (__T*)(neko_instance()->ctx.game.user_data)

#endif