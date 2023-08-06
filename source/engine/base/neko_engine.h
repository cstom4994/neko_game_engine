#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/common/neko_types.h"
#include "engine/platform/neko_platform.h"

// Forward Decl
struct neko_platform_i;
struct neko_graphics_i;
struct neko_audio_i;

// Application descriptor for user application
typedef struct neko_application_desc_t {
    neko_result (*init)();
    neko_result (*update)();
    neko_result (*shutdown)();
    const char *window_title;
    u32 window_width;
    u32 window_height;
    neko::cpp::bitflags::bitflags<neko_window_flags> window_flags;
    f32 frame_rate;
    b32 enable_vsync;
    b32 is_running;
    struct {
        s32 argc;
        char **argv;
    } arg;
    void *user_data;
} neko_application_desc_t;

/*
    Game Engine Context:

    * This is the main context for the neko engine. Holds pointers to
        all interfaces registered with the engine, including the description
        for your application.
*/
typedef struct neko_engine_context_t {
    struct neko_platform_i *platform;  // Main platform interface
    struct neko_graphics_i *graphics;
    struct neko_audio_i *audio;
    neko_application_desc_t app;
} neko_engine_context_t;

// Main engine interface
typedef struct neko_engine_t {
    neko_engine_context_t ctx;
    neko_result (*run)();
    neko_result (*shutdown)();
} neko_engine_t;

extern neko_engine_t *neko_engine_construct(neko_application_desc_t app_desc);
extern neko_engine_t *neko_engine_instance();

#define neko_engine_subsystem(T) (neko_engine_instance()->ctx.T)

#define neko_engine_user_data(T) (T *)(neko_engine_instance()->ctx.app.user_data)

#endif  // NEKO_ENGINE_H
