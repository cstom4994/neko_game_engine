#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/base.hpp"
#include "engine/event.h"
#include "engine/scripting/lua_wrapper.hpp"

#if NEKO_AUDIO == 1

#include <miniaudio.h>

using namespace Neko::luabind;

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

#endif

struct App;

NEKO_API() void sound_init();
NEKO_API() void sound_fini();
NEKO_API() int sound_update_all(App *app, event_t evt);
int sound_postupdate(App *app, event_t evt);
NEKO_API() void sound_save_all(Store *s);
NEKO_API() void sound_load_all(Store *s);

void audio_play_event(String event);