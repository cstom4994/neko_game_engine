#pragma once

#include <miniaudio.h>

#include "engine/luax.h"
#include "engine/prelude.h"
#include "engine/base.h"
#include "engine/prelude.h"

struct Sound {
    ma_sound ma;
    bool zombie;
    bool dead_end;

    void trash();
};

Sound *sound_load(String filepath);

int open_mt_sound(lua_State *L);

inline int neko_sound_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    Sound *sound = sound_load(str);
    if (sound == nullptr) {
        return 0;
    }

    luax_ptr_userdata(L, sound, "mt_sound");
    return 1;
}

// SCRIPT(sound,
//
//        NEKO_EXPORT void sound_add(Entity ent);
//        NEKO_EXPORT void sound_remove(Entity ent);
//        NEKO_EXPORT bool sound_has(Entity ent);
//
//        NEKO_EXPORT void sound_set_path(Entity ent, const char *path);
//        NEKO_EXPORT const char *sound_get_path(Entity ent);
//
//        NEKO_EXPORT void sound_set_playing(Entity ent, bool playing);
//        NEKO_EXPORT bool sound_get_playing(Entity ent);
//
//        NEKO_EXPORT void sound_set_seek(Entity ent, int seek);
//        NEKO_EXPORT int sound_get_seek(Entity ent);
//
//        NEKO_EXPORT void sound_set_finish_destroy(Entity ent, bool finish_destroy);
//        NEKO_EXPORT bool sound_get_finish_destroy(Entity ent);
//
//        NEKO_EXPORT void sound_set_loop(Entity ent, bool loop);
//        NEKO_EXPORT bool sound_get_loop(Entity ent);
//
//        NEKO_EXPORT void sound_set_gain(Entity ent, Scalar gain);
//        NEKO_EXPORT Scalar sound_get_gain(Entity ent);
//     )

void sound_init();
void sound_fini();
void sound_update_all();
void sound_save_all(Store *s);
void sound_load_all(Store *s);