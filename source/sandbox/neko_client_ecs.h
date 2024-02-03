
#ifndef NEKO_CLIENT_ECS
#define NEKO_CLIENT_ECS

#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_engine.h"
#include "engine/util/neko_gfxt.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_imgui.h"
#include "engine/util/neko_sprite.h"
#include "engine/util/neko_tiled.h"

typedef struct neko_client_userdata_s {
    neko_command_buffer_t *cb;
    neko_immediate_draw_t *idraw;
    neko_immediate_draw_static_data_t *idraw_sd;
    neko_core_ui_context_t *igui;
    neko_gui_ctx_t *nui;
    neko_graphics_custom_batch_context_t *sprite_batch;

    struct {
        // 热加载模块
        u32 module_count;
        void (*module_func[16])(void);
    } vm;
} neko_client_userdata_t;

#if defined(NEKO_CPP_SRC)

// game
#include "game_chunk.h"
#include "neko_nui_auto.hpp"

void print(const float &val) { std::printf("%f", val); }

void print(const std::string &val) { std::printf("%s", val.c_str()); }

// Register component types by wrapping the struct name in `Comp(...)`

struct Comp(Position) {
    // Register reflectable fields ('props') using the `Prop` macro. This can be used in any aggregate
    // `struct`, doesn't have to be a `Comp`.
    neko_prop(float, x) = 0;
    neko_prop(float, y) = 0;
};

struct Comp(Name) {
    neko_prop(std::string, first) = "First";

    // Props can have additional attributes, see the `neko_prop_attribs` type. You can customize and add
    // your own attributes there.
    neko_prop(std::string, last, .exampleFlag = true) = "Last";

    int internal = 42;  // This is a non-prop field that doesn't show up in reflection. IMPORTANT NOTE:
    // All such non-prop fields must be at the end of the struct, with all props at
    // the front. Mixing props and non-props in the order is not allowed.
};

// After all compnonent types are defined, this macro must be called to be able to use
// `forEachComponentType` to iterate all component types
UseComponentTypes();

typedef struct neko_ui_renderer {
    enum class type { LABEL } type;
    char text[128];
} neko_ui_renderer;

neko_ecs_decl_system(editor_system, EDITOR_SYSTEM, 1, COMPONENT_GAMEOBJECT) {

    neko_gui_ctx_t *nui = ((neko_client_userdata_t *)ecs->user_data)->nui;
    neko_gui_context *ctx = &nui->neko_gui_ctx;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(EDITOR_SYSTEM))) {
            CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(ecs, e, COMPONENT_GAMEOBJECT);

            if (gameobj->selected) {
                char editor_name[64];
                neko_snprintf(editor_name, 64, "GameObject_%s", gameobj->name);

                if (neko_gui_begin(ctx, editor_name, neko_gui_rect(200, 200, 300, 400),
                                   NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_MOVABLE | NEKO_GUI_WINDOW_SCALABLE | NEKO_GUI_WINDOW_MINIMIZABLE | NEKO_GUI_WINDOW_TITLE)) {

                    neko_gui_layout_row_static(ctx, 30, 150, 1);

                    neko_gui::gui_auto(gameobj->name, "name");
                    neko_gui::gui_auto(gameobj->visible, "visible");
                    neko_gui::gui_auto(gameobj->active, "active");
                }
                neko_gui_end(ctx);
            }
        }
    }
}

neko_ecs_decl_system(particle_render_system, PARTICLE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_PARTICLE) {

    neko_command_buffer_t *cb = ((neko_client_userdata_t *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_userdata_t *)ecs->user_data)->idraw;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(PARTICLE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_particle_renderer *particle_render = (neko_particle_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_PARTICLE);

            neko_graphics_t *gfx = neko_instance()->ctx.graphics;

            neko_particle_renderer_update(particle_render, neko_instance()->ctx.platform->time.elapsed);

            neko_particle_renderer_draw(particle_render, cb, {xform->x, xform->y});
        }
    }
}

neko_ecs_decl_system(ui_render_system, UI_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_UI) {

    neko_command_buffer_t *cb = ((neko_client_userdata_t *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_userdata_t *)ecs->user_data)->idraw;
    neko_core_ui_context_t *igui = ((neko_client_userdata_t *)ecs->user_data)->igui;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(UI_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(ecs, e, COMPONENT_GAMEOBJECT);

            if (!gameobj->visible || !gameobj->active) continue;
            // TODO
        }
    }
}

neko_ecs_decl_system(fast_sprite_render_system, FAST_SPRITE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_FAST_SPRITE) {

    neko_command_buffer_t *cb = ((neko_client_userdata_t *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_userdata_t *)ecs->user_data)->idraw;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(FAST_SPRITE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_fast_sprite_renderer *fs_render = (neko_fast_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_FAST_SPRITE);

            neko_fast_sprite_renderer_draw(fs_render, cb);
        }
    }
}


void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_GAMEOBJECT, 1000, sizeof(CGameObject), NULL);
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1000, sizeof(CTransform), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 200, sizeof(CVelocity), NULL);
    neko_ecs_register_component(ecs, COMPONENT_PARTICLE, 20, sizeof(neko_particle_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_UI, 20, sizeof(neko_ui_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_FAST_SPRITE, 20, sizeof(neko_fast_sprite_renderer), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems 将按照系统注册的顺序运行系统
    // 如果希望单独处理每个系统 也可以使用 ecs_run_system

    neko_ecs_register_system(ecs, particle_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    neko_ecs_register_system(ecs, ui_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    neko_ecs_register_system(ecs, fast_sprite_render_system, ECS_SYSTEM_RENDER_DEFERRED);
    neko_ecs_register_system(ecs, editor_system, ECS_SYSTEM_EDITOR);
}

#endif

#endif
