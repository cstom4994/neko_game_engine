
#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include "deps/miniaudio.h"
#include "neko_api.h"

typedef ma_engine neko_audio_engine;
typedef ma_sound neko_sound;

struct Sound {
    neko_sound ma;
    bool zombie;
    bool dead_end;

    void trash();
};

Sound *sound_load(neko::string filepath);

#endif