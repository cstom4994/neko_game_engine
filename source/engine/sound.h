#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/base.hpp"
#include "engine/event.h"
#include "base/scripting/lua_wrapper.hpp"

#include <miniaudio.h>

using namespace Neko::luabind;

struct AudioFile {
    u8 *buf;
    u64 cursor;
    u64 len;
};

struct SoundSource {
    ma_sound ma;
    bool zombie;
    bool dead_end;

    void trash();
};

SoundSource *sound_load(String filepath);

int open_mt_sound(lua_State *L);

inline int neko_sound_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    SoundSource *sound = sound_load(str);
    if (sound == nullptr) {
        return 0;
    }

    luax_ptr_userdata(L, sound, "mt_sound");
    return 1;
}

class Sound : public SingletonClass<Sound> {
public:
    void sound_init();
    void sound_fini();
    int sound_update_all(Event evt);
    int sound_postupdate(Event evt);
};
