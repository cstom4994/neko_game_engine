#ifndef SPRITE_H
#define SPRITE_H

#include "engine/neko_ecs.h"
#include "engine/neko_base.h"
#include "engine/neko_prelude.h"

SCRIPT(sprite,

       NEKO_EXPORT void sprite_set_atlas(const char *filename);
       NEKO_EXPORT const char *sprite_get_atlas();

       NEKO_EXPORT void sprite_add(Entity ent); NEKO_EXPORT void sprite_remove(Entity ent); NEKO_EXPORT bool sprite_has(Entity ent);

       // size to draw in world units, centered at transform position
       NEKO_EXPORT void sprite_set_size(Entity ent, CVec2 size); NEKO_EXPORT CVec2 sprite_get_size(Entity ent);

       // bottom left corner of atlas region in pixels
       NEKO_EXPORT void sprite_set_texcell(Entity ent, CVec2 texcell); NEKO_EXPORT CVec2 sprite_get_texcell(Entity ent);

       // size of atlas region in pixels
       NEKO_EXPORT void sprite_set_texsize(Entity ent, CVec2 texsize); NEKO_EXPORT CVec2 sprite_get_texsize(Entity ent);

       // lower depth drawn on top
       NEKO_EXPORT void sprite_set_depth(Entity ent, int depth); NEKO_EXPORT int sprite_get_depth(Entity ent);

)

void sprite_init();
void sprite_deinit();
void sprite_update_all();
void sprite_draw_all();
void sprite_save_all(Store *s);
void sprite_load_all(Store *s);

#endif
