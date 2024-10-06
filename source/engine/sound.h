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

using namespace neko::luabind;

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

#if NEKO_AUDIO == 2

// FMOD header
#include <fmod.hpp>
#include <fmod_studio.hpp>

namespace FMOD {
namespace Studio {
class EventDescription;
class Bank;
}  // namespace Studio
}  // namespace FMOD

namespace neko {

class event;

class event_instance {
    event *type;

    void start();
    void stop();
};

struct event {
    FMOD::Studio::EventDescription *fmod_bank;
    std::string path;

    event_instance instance();
};

struct bank {
    FMOD::Studio::Bank *fmod_bank;
    std::unordered_map<std::string, event> event_map;
    bank(FMOD::Studio::Bank *fmod_bank);

    event *load_event(const char *path);
};

class bank_manager {
    std::unordered_map<std::string, bank> bank_map;

    bank_manager();

public:
    static bank_manager &instance();

    void unload(const char *bank_path);

    bank *load(const char *path);
};

void init_fmod_system();

FMOD::Studio::System *get_fmod_system();
FMOD::System *get_fmod_core_system();

struct fmod_exception {
    const char *message;
};

inline void check_err(FMOD_RESULT res) {
    if (res != FMOD_OK) {
        // throw fmod_exception{FMOD_ErrorString(res)};
    }
}

class Audio {

public:
    struct Implementation {
        Implementation();
        ~Implementation();

        void Update();

        int next_channel_id;

        typedef std::map<std::string, FMOD::Sound *> SoundMap;
        typedef std::map<int, FMOD::Channel *> ChannelMap;
        typedef std::map<std::string, FMOD::Studio::EventInstance *> EventMap;
        typedef std::map<std::string, FMOD::Studio::Bank *> BankMap;

        BankMap banks;
        SoundMap sounds;
        EventMap audio_events;
        ChannelMap audio_channels;
    };

public:
    static void Init();
    static void Update();
    static void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    static Implementation *fmod_impl;

    void LoadBank(const std::string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    FMOD::Studio::Bank *GetBank(const std::string &strBankName);
    void LoadEvent(const std::string &strEventName);
    void LoadSound(const std::string &strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const std::string &strSoundName);
    void Set3dListenerAndOrientation(const vec3 &vPosition, const vec3 &vLook, const vec3 &vUp);
    int PlaySounds(const std::string &strSoundName, const vec3 &vPos = vec3{0, 0, 0}, float fVolumedB = 0.0f);
    void PlayEvent(const std::string &strEventName);
    FMOD::Studio::EventInstance *GetEvent(const std::string &strEventName);
    void StopChannel(int channel_id);
    void StopEvent(const std::string &strEventName, bool bImmediate = false);
    void GetEventParameter(const std::string &strEventName, const std::string &strEventParameter, float *parameter);
    void SetEventParameter(const std::string &strEventName, const std::string &strParameterName, float fValue);
    void SetGlobalParameter(const std::string &strParameterName, float fValue);
    void GetGlobalParameter(const std::string &strEventParameter, float *parameter);
    void StopAllChannels();
    void SetChannel3dPosition(int channel_id, const vec3 &vPosition);
    void SetChannelVolume(int channel_id, float fVolumedB);
    bool IsPlaying(int channel_id) const;
    bool IsEventPlaying(const std::string &strEventName) const;
    float dbToVolume(float dB);
    float VolumeTodB(float volume);
    FMOD_VECTOR VectorToFmod(const vec3 &vPosition);
};

}  // namespace neko

#endif

struct App;

NEKO_API() void sound_init();
NEKO_API() void sound_fini();
NEKO_API() int sound_update_all(App *app, event_t evt);
int sound_postupdate(App *app, event_t evt);
NEKO_API() void sound_save_all(Store *s);
NEKO_API() void sound_load_all(Store *s);
