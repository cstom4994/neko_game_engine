
#ifndef NEKO_CLIENT_ECS
#define NEKO_CLIENT_ECS

#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_engine.h"
#include "engine/util/neko_gfxt.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_tiled.h"

// game
#include "neko_nui_auto.hpp"
#include "neko_sprite.h"

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

typedef struct neko_client_ecs_userdata_s {
    neko_command_buffer_t *cb;
    neko_immediate_draw_t *idraw;
    neko_gui_context_t *igui;
    neko_nui_ctx_t *nui;
} neko_client_ecs_userdata_t;

neko_ecs_decl_system(editor_system, EDITOR_SYSTEM, 1, COMPONENT_GAMEOBJECT) {

    neko_nui_ctx_t *nui = ((neko_client_ecs_userdata_s *)ecs->user_data)->nui;
    neko_nui_context *ctx = &nui->neko_nui_ctx;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(EDITOR_SYSTEM))) {
            CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(ecs, e, COMPONENT_GAMEOBJECT);

            if (gameobj->selected) {
                char editor_name[64];
                neko_snprintf(editor_name, 64, "GameObject_%s", gameobj->name);

                if (neko_nui_begin(ctx, editor_name, neko_nui_rect(200, 200, 300, 400),
                                   NEKO_NUI_WINDOW_BORDER | NEKO_NUI_WINDOW_MOVABLE | NEKO_NUI_WINDOW_SCALABLE | NEKO_NUI_WINDOW_MINIMIZABLE | NEKO_NUI_WINDOW_TITLE)) {

                    neko_nui_layout_row_static(ctx, 30, 150, 1);

                    neko_nui::nui_auto(gameobj->name, "name");
                    neko_nui::nui_auto(gameobj->visible, "visible");
                }
                neko_nui_end(ctx);
            }
        }
    }
}

neko_ecs_decl_system(movement_system, MOVEMENT_SYSTEM, 3, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_SPRITE) {
    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(MOVEMENT_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            CVelocity *velocity = (CVelocity *)neko_ecs_ent_get_component(ecs, e, COMPONENT_VELOCITY);
            neko_sprite_renderer *sprite = (neko_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            // Grab global instance of engine
            neko_t *engine = neko_instance();

            neko_sprite_renderer_update(sprite, engine->ctx.platform->time.delta);

            xform->x += velocity->dx;
            xform->y += velocity->dy;

            velocity->dx /= 2.0f;
            velocity->dy /= 2.0f;
        }
    }
}

neko_ecs_decl_system(sprite_render_system, SPRITE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_SPRITE) {

    neko_command_buffer_t *cb = ((neko_client_ecs_userdata_s *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_ecs_userdata_s *)ecs->user_data)->idraw;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(SPRITE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_sprite_renderer *sprite = (neko_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            neko_graphics_t *gfx = neko_instance()->ctx.graphics;

            s32 index;
            if (sprite->loop) {
                index = sprite->loop->indices[sprite->current_frame];
            } else {
                index = sprite->current_frame;
            }

            neko_sprite *spr = sprite->sprite;
            neko_sprite_frame f = spr->frames[index];

            neko_idraw_rect_2d_textured_ext(idraw, xform->x, xform->y, xform->x + spr->width * 2.f, xform->y + spr->height * 2.f, f.u0, f.v0, f.u1, f.v1, sprite->sprite->img.id, NEKO_COLOR_WHITE);
        }
    }
}

neko_ecs_decl_system(particle_render_system, PARTICLE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_PARTICLE) {

    neko_command_buffer_t *cb = ((neko_client_ecs_userdata_s *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_ecs_userdata_s *)ecs->user_data)->idraw;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(PARTICLE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_particle_renderer *particle_render = (neko_particle_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_PARTICLE);

            neko_graphics_t *gfx = neko_instance()->ctx.graphics;

            neko_particle_renderer_update(particle_render, neko_instance()->ctx.platform->time.elapsed);

            neko_particle_renderer_draw(particle_render, cb, xform);
        }
    }
}

neko_ecs_decl_system(tiled_render_system, TILED_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_TILED) {

    neko_command_buffer_t *cb = ((neko_client_ecs_userdata_s *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_ecs_userdata_s *)ecs->user_data)->idraw;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(TILED_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_tiled_renderer *tiled_render = (neko_tiled_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TILED);

            neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
            {
                neko_tiled_render_begin(cb, tiled_render);

                for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.layers); i++) {
                    layer_t *layer = tiled_render->map.layers + i;
                    for (u32 y = 0; y < layer->height; y++) {
                        for (u32 x = 0; x < layer->width; x++) {
                            tile_t *tile = layer->tiles + (x + y * layer->width);
                            if (tile->id != 0) {
                                tileset_t *tileset = tiled_render->map.tilesets + tile->tileset_id;
                                u32 tsxx = (tile->id % (tileset->width / tileset->tile_width) - 1) * tileset->tile_width;
                                u32 tsyy = tileset->tile_height * ((tile->id - tileset->first_gid) / (tileset->width / tileset->tile_width));
                                neko_tiled_quad_t quad = {.tileset_id = tile->tileset_id,
                                                          .texture = tileset->texture,
                                                          .texture_size = {(f32)tileset->width, (f32)tileset->height},
                                                          .position = {(f32)(x * tileset->tile_width * SPRITE_SCALE), (f32)(y * tileset->tile_height * SPRITE_SCALE)},
                                                          .dimentions = {(f32)(tileset->tile_width * SPRITE_SCALE), (f32)(tileset->tile_height * SPRITE_SCALE)},
                                                          .rectangle = {(f32)tsxx, (f32)tsyy, (f32)tileset->tile_width, (f32)tileset->tile_height},
                                                          .color = layer->tint,
                                                          .use_texture = true};
                                neko_tiled_render_push(cb, tiled_render, quad);
                            }
                        }
                    }
                }

                // for (u32 i = 0; i < neko_dyn_array_size(map.object_groups); i++) {
                //     object_group_t *group = map.object_groups + i;
                //     for (u32 ii = 0; ii < neko_dyn_array_size(map.object_groups[i].objects); ii++) {
                //         object_t *object = group->objects + ii;
                //         neko_tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE), (f32)(object->y * SPRITE_SCALE)},
                //                                   .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
                //                                   .color = group->color,
                //                                   .use_texture = false};
                //         neko_tiled_render_push(&g_cb, quad);
                //     }
                // }

                neko_tiled_render_draw(cb, tiled_render);
            }
            neko_graphics_renderpass_end(cb);
        }
    }
}

neko_ecs_decl_system(gfxt_render_system, GFXT_RENDER_SYSTEM, 3, COMPONENT_GAMEOBJECT, COMPONENT_TRANSFORM, COMPONENT_GFXT) {

    neko_command_buffer_t *cb = ((neko_client_ecs_userdata_s *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_ecs_userdata_s *)ecs->user_data)->idraw;

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    const f32 t = neko_platform_elapsed_time();

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(GFXT_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_gfxt_renderer *gfxt_render = (neko_gfxt_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_GFXT);

            CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(ecs, e, COMPONENT_GAMEOBJECT);

            if (!gameobj->visible) continue;

            // Camera for scene
            neko_camera_t cam = neko_camera_perspective();
            cam.transform.position = neko_v3(0.f, 6.f, 20.f);
            neko_vqs trans = {.translation = neko_v3(0.f, 0.f, -10.f), .rotation = neko_quat_angle_axis(t * 0.001f, NEKO_YAXIS), .scale = neko_v3s(0.1f)};
            neko_mat4 model = neko_vqs_to_mat4(&trans);
            neko_mat4 vp = neko_camera_get_view_projection(&cam, fbs.x, fbs.y);
            neko_mat4 mvp = neko_mat4_mul(vp, model);

            // Apply material uniforms
            neko_gfxt_material_set_uniform(&gfxt_render->mat, "u_mvp", &mvp);
            neko_gfxt_material_set_uniform(&gfxt_render->mat, "u_tex", &gfxt_render->texture);

            // Rendering
            neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
            {
                // Set view port
                neko_graphics_set_viewport(cb, 0, 0, (int)fbs.x, (int)fbs.y);

                // Bind material
                neko_gfxt_material_bind(cb, &gfxt_render->mat);

                // Bind material uniforms
                neko_gfxt_material_bind_uniforms(cb, &gfxt_render->mat);

                // Render mesh
                neko_gfxt_mesh_draw_material(cb, &gfxt_render->mesh, &gfxt_render->mat);
            }
            neko_graphics_renderpass_end(cb);
        }
    }
}

neko_ecs_decl_system(ui_render_system, UI_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_UI) {

    neko_command_buffer_t *cb = ((neko_client_ecs_userdata_s *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_ecs_userdata_s *)ecs->user_data)->idraw;
    neko_gui_context_t *igui = ((neko_client_ecs_userdata_s *)ecs->user_data)->igui;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(UI_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_ui_renderer *ui_render = (neko_ui_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_UI);

            if (ui_render->type == neko_ui_renderer::type::LABEL) {
                neko_graphics_text(ui_render->text, igui->default_font, xform->x, xform->y);
            }
        }
    }
}

neko_ecs_decl_system(fast_sprite_render_system, FAST_SPRITE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_FAST_SPRITE) {

    neko_command_buffer_t *cb = ((neko_client_ecs_userdata_s *)ecs->user_data)->cb;
    neko_immediate_draw_t *idraw = ((neko_client_ecs_userdata_s *)ecs->user_data)->idraw;

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(FAST_SPRITE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_fast_sprite_renderer *fs_render = (neko_fast_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_FAST_SPRITE);

            neko_fast_sprite_renderer_draw(fs_render, cb);
        }
    }
}

void gfxt_render_system_df(void *data) {
    neko_gfxt_renderer *system = (neko_gfxt_renderer *)data;
    neko_gfxt_texture_destroy(&system->texture);
    neko_gfxt_mesh_destroy(&system->mesh);
    neko_gfxt_material_destroy(&system->mat);
    neko_gfxt_pipeline_destroy(&system->pip);
}

void tiled_render_system_df(void *data) {
    neko_tiled_renderer *system = (neko_tiled_renderer *)data;
    neko_tiled_unload(&system->map);
    neko_tiled_render_deinit(system);
}

void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_GAMEOBJECT, 1000, sizeof(CGameObject), NULL);
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1000, sizeof(CTransform), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 200, sizeof(CVelocity), NULL);
    neko_ecs_register_component(ecs, COMPONENT_SPRITE, 1000, sizeof(neko_sprite_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_PARTICLE, 20, sizeof(neko_particle_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_TILED, 20, sizeof(neko_tiled_renderer), tiled_render_system_df);
    neko_ecs_register_component(ecs, COMPONENT_GFXT, 20, sizeof(neko_gfxt_renderer), gfxt_render_system_df);
    neko_ecs_register_component(ecs, COMPONENT_UI, 20, sizeof(neko_ui_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_FAST_SPRITE, 20, sizeof(neko_fast_sprite_renderer), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems 将按照系统注册的顺序运行系统
    // 如果希望单独处理每个系统 也可以使用 ecs_run_system

    neko_ecs_register_system(ecs, movement_system, ECS_SYSTEM_UPDATE);
    neko_ecs_register_system(ecs, sprite_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    neko_ecs_register_system(ecs, particle_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    neko_ecs_register_system(ecs, tiled_render_system, ECS_SYSTEM_RENDER_DEFERRED);
    neko_ecs_register_system(ecs, gfxt_render_system, ECS_SYSTEM_RENDER_DEFERRED);
    neko_ecs_register_system(ecs, ui_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    neko_ecs_register_system(ecs, fast_sprite_render_system, ECS_SYSTEM_RENDER_DEFERRED);
    neko_ecs_register_system(ecs, editor_system, ECS_SYSTEM_EDITOR);
}

#endif