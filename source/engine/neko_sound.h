#pragma once

#include <miniaudio.h>

#include "engine/neko_lua.h"
#include "engine/neko_prelude.h"

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