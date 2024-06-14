
#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <miniaudio.h>
// #include "engine/neko_api.hpp"
#include "engine/neko.h"
#include "engine/neko_engine.h"

// typedef ma_engine neko_audio_engine;
// typedef ma_sound neko_sound;

extern Neko_ModuleInterface* g_interface;

typedef struct neko_lua_sound {
    ma_sound ma;
    bool zombie;
    bool dead_end;
} neko_lua_sound;

neko_lua_sound* sound_load(ma_engine* audio_engine, const_str filepath);
void sound_fini(neko_lua_sound* sound);

#endif