#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/base.hpp"
#include "engine/event.h"
#include "engine/scripting/lua_wrapper.hpp"

#include <miniaudio.h>

using namespace Neko::luabind;

struct AudioFile {
    u8 *buf;
    u64 cursor;
    u64 len;
};

struct SoundSource {
    ma_sound ma;
    bool zombie;  // 是否已经脱离lua的gc管理
    bool end;     //
    bool dead;
};

struct SoundIndex {
    SoundSource *ptr;
};

struct SoundGarbage {
    SoundSource *ptr;
};

class Sound : public SingletonClass<Sound> {
    void *miniaudio_vfs;
    ma_engine audio_engine;
    Array<SoundGarbage> garbage_sounds;
    Mutex garbage_collect_mutex;

public:
    void sound_init();
    void sound_fini();
    int OnUpdate(Event evt);
    int OnPostUpdate(Event evt);

    void GarbageCollect();

    inline u64 GarbageCount() const { return garbage_sounds.len; }

    inline void PushSoundGarbage(SoundGarbage g) {
        LockGuard<Mutex> lock(garbage_collect_mutex);
        garbage_sounds.push(g);
    }

    inline ma_engine *GetAudioEngine() { return &audio_engine; };
};
