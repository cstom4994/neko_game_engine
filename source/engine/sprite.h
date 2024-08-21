#ifndef SPRITE_H
#define SPRITE_H

#include "engine/base.h"
#include "engine/entity.h"
#include "engine/gfx.h"
#include "engine/glew_glfw.h"
#include "engine/prelude.h"
#include "engine/texture.h"

struct AseSpriteFrame {
    i32 duration;
    float u0, v0, u1, v1;
};

struct AseSpriteLoop {
    Slice<i32> indices;
};

struct AseSpriteData {
    Arena arena;
    Slice<AseSpriteFrame> frames;
    HashMap<AseSpriteLoop> by_tag;
    gfx_texture_t tex;
    i32 width;
    i32 height;

    bool load(String filepath);
    void trash();
};

struct AseSprite {
    u64 sprite;  // index into assets
    u64 loop;    // index into AseSpriteData::by_tag
    float elapsed;
    i32 current_frame;

    bool play(String tag);
    void update(float dt);
    void set_frame(i32 frame);
};

struct AseSpriteView {
    AseSprite *sprite;
    AseSpriteData data;
    AseSpriteLoop loop;

    bool make(AseSprite *spr);
    i32 frame();
    u64 len();
};

NEKO_SCRIPT(sprite,

            NEKO_EXPORT void sprite_set_atlas(const char *filename);

            NEKO_EXPORT const char *sprite_get_atlas();

            NEKO_EXPORT void sprite_add(Entity ent);

            NEKO_EXPORT void sprite_remove(Entity ent);

            NEKO_EXPORT bool sprite_has(Entity ent);

            // size to draw in world units, centered at transform position
            NEKO_EXPORT void sprite_set_size(Entity ent, LuaVec2 size);

            NEKO_EXPORT LuaVec2 sprite_get_size(Entity ent);

            // bottom left corner of atlas region in pixels
            NEKO_EXPORT void sprite_set_texcell(Entity ent, LuaVec2 texcell);

            NEKO_EXPORT LuaVec2 sprite_get_texcell(Entity ent);

            // size of atlas region in pixels
            NEKO_EXPORT void sprite_set_texsize(Entity ent, LuaVec2 texsize);

            NEKO_EXPORT LuaVec2 sprite_get_texsize(Entity ent);

            // lower depth drawn on top
            NEKO_EXPORT void sprite_set_depth(Entity ent, int depth);

            NEKO_EXPORT int sprite_get_depth(Entity ent);

)

void sprite_init();
void sprite_fini();
void sprite_update_all();
void sprite_draw_all();
void sprite_save_all(Store *s);
void sprite_load_all(Store *s);

int open_mt_sprite(lua_State *L);
int neko_sprite_load(lua_State *L);

#endif
