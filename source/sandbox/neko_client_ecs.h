
#ifndef NEKO_CLIENT_ECS
#define NEKO_CLIENT_ECS

#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_ecs.h"
#include "engine/neko_engine.h"
#include "engine/util/neko_idraw.h"

// game
#include "neko_sprite.h"

typedef struct neko_client_ecs_userdata_s {
    neko_command_buffer_t *cb;
    neko_immediate_draw_t *idraw;
} neko_client_ecs_userdata_t;

neko_ecs_decl_mask(MOVEMENT_SYSTEM, 3, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_SPRITE);
void movement_system(neko_ecs *ecs) {
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

neko_ecs_decl_mask(SPRITE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_SPRITE);
void sprite_render_system(neko_ecs *ecs) {

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

            // gfx->immediate.draw_rect_textured_ext(cb, xform->x, xform->y, xform->x + spr->width * 4.f, xform->y + spr->height * 4.f, f.u0, f.v0, f.u1, f.v1, sprite->sprite->img.id,
            // neko_color_white);

            neko_idraw_rect_2d_textured_ext(idraw, xform->x, xform->y, xform->x + spr->width * 4.f, xform->y + spr->height * 4.f, f.u0, f.v0, f.u1, f.v1, sprite->sprite->img.id, NEKO_COLOR_WHITE);

            // neko_idraw_texture(&gsi, sprite->sprite->img.id);
        }
    }
}

neko_ecs_decl_mask(PARTICLE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_PARTICLE);
void particle_render_system(neko_ecs *ecs) {

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

void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1000, sizeof(CTransform), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 200, sizeof(CVelocity), NULL);
    neko_ecs_register_component(ecs, COMPONENT_SPRITE, 1000, sizeof(neko_sprite_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_PARTICLE, 20, sizeof(neko_particle_renderer), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems will run the systems in the order they are registered
    // ecs_run_system is also available if you wish to handle each system seperately
    //
    // neko_ecs, function pointer to system (must take a parameter of neko_ecs), system type
    neko_ecs_register_system(ecs, movement_system, ECS_SYSTEM_UPDATE);
    neko_ecs_register_system(ecs, sprite_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    neko_ecs_register_system(ecs, particle_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
}

#endif