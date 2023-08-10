#ifndef NEKO_ENGINE_H
#define NEKO_ENGINE_H

#include "engine/base/neko_ecs.h"
#include "engine/common/neko_types.h"
#include "engine/platform/neko_platform.h"
#include "engine/scripting/neko_scripting.h"
#include "engine/utility/neko_static_refl.hpp"

// Forward Decl
struct neko_platform_i;
struct neko_graphics_i;
struct neko_audio_i;

// Application descriptor for user application
typedef struct neko_application_desc_t {
    const_str window_title;
    u32 window_width;
    u32 window_height;
    neko_window_flags window_flags;
    f32 frame_rate;
    bool enable_vsync;
    void *user_data;
} neko_application_desc_t;

template <>
struct neko::meta::static_refl::TypeInfo<neko_application_desc_t> : TypeInfoBase<neko_application_desc_t> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            Field{TSTR("window_title"), &Type::window_title},    // 窗口标题
            Field{TSTR("window_width"), &Type::window_width},    //
            Field{TSTR("window_height"), &Type::window_height},  //
            Field{TSTR("window_flags"), &Type::window_flags},    //
            Field{TSTR("frame_rate"), &Type::frame_rate},        // 限制帧率
            Field{TSTR("enable_vsync"), &Type::enable_vsync},    // 启用 vsync
    };
};

/*
    Game Engine Context:

    * This is the main context for the neko engine. Holds pointers to
        all interfaces registered with the engine, including the description
        for your application.
*/
typedef struct neko_engine_context_t {
    // 引擎基本接口

    // 脚本系统
    neko::neko_scripting scripting;

    struct neko_platform_i *platform;  // Main platform interface
    struct neko_graphics_i *graphics;
    struct neko_audio_i *audio;

    neko_application_desc_t app;
    neko_ecs *ecs;

    bool is_running;

    // 客户端暴露接口
    neko_result (*update)();
    neko_result (*shutdown)();
} neko_engine_context_t;

// Main engine interface
typedef struct neko_engine {
    neko_engine_context_t ctx;
    neko_result (*engine_run)();
    neko_result (*engine_shutdown)();
} neko_engine;

extern neko_engine *neko_engine_construct(int argc, char **argv);
extern neko_engine *neko_engine_instance();

#define neko_engine_subsystem(T) (neko_engine_instance()->ctx.T)

#define neko_sc() (&neko_engine_subsystem(scripting))

#define neko_engine_user_data(T) (T *)(neko_engine_instance()->ctx.app.user_data)

#endif  // NEKO_ENGINE_H
