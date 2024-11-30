
#include "engine/sound.h"

#include "base/common/profiler.hpp"
#include "base/common/vfs.hpp"
#include "base/scripting/reflection.hpp"
#include "engine/bootstrap.h"

#if NEKO_AUDIO == 1

static void on_sound_end(void *udata, ma_sound *ma) {
    Sound *sound = (Sound *)udata;
    if (sound->zombie) {
        sound->dead_end = true;
    }
}

Sound *sound_load(String filepath) {
    PROFILE_FUNC();

    ma_result res = MA_SUCCESS;

    Sound *sound = (Sound *)mem_alloc(sizeof(Sound));

    String cpath = to_cstr(filepath);
    neko_defer(mem_free(cpath.data));

    res = ma_sound_init_from_file(&gApp->audio_engine, cpath.data, 0, nullptr, nullptr, &sound->ma);
    if (res != MA_SUCCESS) {
        mem_free(sound);
        return nullptr;
    }

    res = ma_sound_set_end_callback(&sound->ma, on_sound_end, sound);
    if (res != MA_SUCCESS) {
        mem_free(sound);
        return nullptr;
    }

    sound->zombie = false;
    sound->dead_end = false;
    return sound;
}

void Sound::trash() { ma_sound_uninit(&ma); }

// mt_sound

static ma_sound *sound_ma(lua_State *L) {
    Sound *sound = *(Sound **)luaL_checkudata(L, 1, "mt_sound");
    return &sound->ma;
}

static int mt_sound_gc(lua_State *L) {
    Sound *sound = *(Sound **)luaL_checkudata(L, 1, "mt_sound");

    if (ma_sound_at_end(&sound->ma)) {
        sound->trash();
        mem_free(sound);
    } else {
        sound->zombie = true;
        gApp->garbage_sounds.push(sound);
    }

    return 0;
}

static int mt_sound_frames(lua_State *L) {
    unsigned long long frames = 0;
    ma_result res = ma_sound_get_length_in_pcm_frames(sound_ma(L), &frames);
    if (res != MA_SUCCESS) {
        return 0;
    }

    lua_pushinteger(L, (lua_Integer)frames);
    return 1;
}

static int mt_sound_start(lua_State *L) {
    ma_result res = ma_sound_start(sound_ma(L));
    if (res != MA_SUCCESS) {
        luaL_error(L, "failed to start sound");
    }

    return 0;
}

static int mt_sound_stop(lua_State *L) {
    ma_result res = ma_sound_stop(sound_ma(L));
    if (res != MA_SUCCESS) {
        luaL_error(L, "failed to stop sound");
    }

    return 0;
}

static int mt_sound_seek(lua_State *L) {
    lua_Number f = luaL_optnumber(L, 2, 0);

    ma_result res = ma_sound_seek_to_pcm_frame(sound_ma(L), f);
    if (res != MA_SUCCESS) {
        luaL_error(L, "failed to seek to frame");
    }

    return 0;
}

static int mt_sound_secs(lua_State *L) {
    float len = 0;
    ma_result res = ma_sound_get_length_in_seconds(sound_ma(L), &len);
    if (res != MA_SUCCESS) {
        return 0;
    }

    lua_pushnumber(L, len);
    return 1;
}

static int mt_sound_vol(lua_State *L) {
    lua_pushnumber(L, ma_sound_get_volume(sound_ma(L)));
    return 1;
}

static int mt_sound_set_vol(lua_State *L) {
    ma_sound_set_volume(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
    return 0;
}

static int mt_sound_pan(lua_State *L) {
    lua_pushnumber(L, ma_sound_get_pan(sound_ma(L)));
    return 1;
}

static int mt_sound_set_pan(lua_State *L) {
    ma_sound_set_pan(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
    return 0;
}

static int mt_sound_pitch(lua_State *L) {
    lua_pushnumber(L, ma_sound_get_pitch(sound_ma(L)));
    return 1;
}

static int mt_sound_set_pitch(lua_State *L) {
    ma_sound_set_pitch(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
    return 0;
}

static int mt_sound_loop(lua_State *L) {
    lua_pushboolean(L, ma_sound_is_looping(sound_ma(L)));
    return 1;
}

static int mt_sound_set_loop(lua_State *L) {
    ma_sound_set_looping(sound_ma(L), lua_toboolean(L, 2));
    return 0;
}

static int mt_sound_pos(lua_State *L) {
    ma_vec3f pos = ma_sound_get_position(sound_ma(L));
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

static int mt_sound_set_pos(lua_State *L) {
    lua_Number x = luaL_optnumber(L, 2, 0);
    lua_Number y = luaL_optnumber(L, 3, 0);
    ma_sound_set_position(sound_ma(L), (float)x, (float)y, 0.0f);
    return 0;
}

static int mt_sound_dir(lua_State *L) {
    ma_vec3f dir = ma_sound_get_direction(sound_ma(L));
    lua_pushnumber(L, dir.x);
    lua_pushnumber(L, dir.y);
    return 2;
}

static int mt_sound_set_dir(lua_State *L) {
    lua_Number x = luaL_optnumber(L, 2, 0);
    lua_Number y = luaL_optnumber(L, 3, 0);
    ma_sound_set_direction(sound_ma(L), (float)x, (float)y, 0.0f);
    return 0;
}

static int mt_sound_vel(lua_State *L) {
    ma_vec3f vel = ma_sound_get_velocity(sound_ma(L));
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    return 2;
}

static int mt_sound_set_vel(lua_State *L) {
    lua_Number x = luaL_optnumber(L, 2, 0);
    lua_Number y = luaL_optnumber(L, 3, 0);
    ma_sound_set_velocity(sound_ma(L), (float)x, (float)y, 0.0f);
    return 0;
}

static int mt_sound_set_fade(lua_State *L) {
    lua_Number from = luaL_optnumber(L, 2, 0);
    lua_Number to = luaL_optnumber(L, 3, 0);
    lua_Number ms = luaL_optnumber(L, 4, 0);
    ma_sound_set_fade_in_milliseconds(sound_ma(L), (float)from, (float)to, (u64)ms);
    return 0;
}

int open_mt_sound(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_sound_gc},           {"frames", mt_sound_frames},
            {"secs", mt_sound_secs},         {"start", mt_sound_start},
            {"stop", mt_sound_stop},         {"seek", mt_sound_seek},
            {"vol", mt_sound_vol},           {"set_vol", mt_sound_set_vol},
            {"pan", mt_sound_pan},           {"set_pan", mt_sound_set_pan},
            {"pitch", mt_sound_pitch},       {"set_pitch", mt_sound_set_pitch},
            {"loop", mt_sound_loop},         {"set_loop", mt_sound_set_loop},
            {"pos", mt_sound_pos},           {"set_pos", mt_sound_set_pos},
            {"dir", mt_sound_dir},           {"set_dir", mt_sound_set_dir},
            {"vel", mt_sound_vel},           {"set_vel", mt_sound_set_vel},
            {"set_fade", mt_sound_set_fade}, {nullptr, nullptr},
    };

    luax_new_class(L, "mt_sound", reg);
    return 0;
}

#endif

#if NEKO_AUDIO == 2

// FMOD header
#include <fmod.hpp>
#include <fmod_studio.hpp>

namespace Neko {

struct FMODAuidoEvent {
    FMOD::Studio::EventDescription *fmod_bank;
    std::string path;
};

struct FMODBank {
    FMOD::Studio::Bank *fmod_bank;
    std::unordered_map<std::string, FMODAuidoEvent> event_map;
    FMODBank(FMOD::Studio::Bank *fmod_bank);

    FMODAuidoEvent *load_event(const char *path);
};

class FMODAudio {
    FMOD::Studio::System *fmod_system = nullptr;
    FMOD::System *lowLevelSystem = nullptr;

    std::unordered_map<std::string, FMOD::Sound *> g_mapSoundPrecache;
    std::unordered_map<int, std::string> g_mapSoundPrecacheSV;

    int next_channel_id;

    HashMap<FMOD::Studio::Bank *> banks;
    HashMap<FMOD::Sound *> sounds;
    HashMap<FMOD::Studio::EventInstance *> audio_events;
    HashMap<FMOD::Channel *> audio_channels;

    std::unordered_map<std::string, FMODBank> bank_map;

    FMODBank *bank_load(const char *path);

public:
    void init_fmod_system();
    FMOD::Studio::System *get_fmod_system();
    FMOD::System *get_fmod_core_system();

public:
    void Init();
    void Update();
    void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    void LoadBank(const String &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    void LoadBankVfs(const String &filepath, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    FMOD::Studio::Bank *GetBank(const String &strBankName);
    void LoadEvent(const String &strEventName);
    void LoadSound(const String &strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const String &strSoundName);
    int PlaySounds(const String &strSoundName, const vec3 &vPos = vec3{0, 0, 0}, float fVolumedB = 0.0f);
    void PlayEvent(const String &strEventName);
    FMOD::Studio::EventInstance *GetEvent(const String &strEventName);
    void StopEvent(const String &strEventName, bool bImmediate = false);
    void GetEventParameter(const String &strEventName, const String &strEventParameter, float *parameter);
    void SetEventParameter(const String &strEventName, const String &strParameterName, float fValue);
    void SetGlobalParameter(const String &strParameterName, float fValue);
    void GetGlobalParameter(const String &strEventParameter, float *parameter);
    void StopChannel(int channel_id);
    void StopAllChannels();
    bool IsPlaying(int channel_id) const;
    void SetChannel3dPosition(int channel_id, const vec3 &vPosition);
    void SetChannelVolume(int channel_id, float fVolumedB);
    bool IsEventPlaying(const String &strEventName) const;
    float dbToVolume(float dB);
    float VolumeTodB(float volume);
    FMOD_VECTOR VectorToFmod(const vec3 &vPosition);
};

}  // namespace Neko

#endif

#if NEKO_AUDIO == 2

namespace Neko {

FMODBank::FMODBank(FMOD::Studio::Bank *fmod_bank) : fmod_bank(fmod_bank) {
    int num_events;
    FMODAudio::ErrorCheck(fmod_bank->getEventCount(&num_events));
    std::cout << "Num events = " << num_events << std::endl;
    if (num_events > 0) {
        std::vector<FMOD::Studio::EventDescription *> event_descriptions(num_events);
        FMODAudio::ErrorCheck(fmod_bank->getEventList(event_descriptions.data(), num_events, &num_events));

        for (int ii = 0; ii < num_events; ++ii) {
            // Get the event name
            int name_len;
            FMODAudio::ErrorCheck(event_descriptions[ii]->getPath(nullptr, 0, &name_len));
            char *name_chars = new char[name_len];
            FMODAudio::ErrorCheck(event_descriptions[ii]->getPath(name_chars, name_len, nullptr));
            std::string name(name_chars);
            delete[] name_chars;
            std::cout << "Event name = " << name.c_str() << std::endl;
            event_map.insert(std::make_pair(name, FMODAuidoEvent{event_descriptions[ii], name}));
        }
    }
}

FMODAuidoEvent *FMODBank::load_event(const char *path) {
    const auto cached = event_map.find(path);
    return (cached != event_map.end()) ? &cached->second : nullptr;
}

FMODBank *FMODAudio::bank_load(const char *path) {
    const auto cached = bank_map.find(path);
    if (cached != bank_map.end()) {
        return &cached->second;
    }
    const auto system = get_fmod_system();
    FMOD::Studio::Bank *fmod_bank = nullptr;
    FMODAudio::ErrorCheck(system->loadBankFile(path, FMOD_STUDIO_LOAD_BANK_NORMAL, &fmod_bank));

    // make the bank & insert
    FMODBank b{fmod_bank};
    return &bank_map.insert(std::make_pair(std::string(path), std::move(b))).first->second;
}

void FMODAudio::init_fmod_system() {
    FMODAudio::ErrorCheck(FMOD::Studio::System::create(&fmod_system));

    FMODAudio::ErrorCheck(fmod_system->getCoreSystem(&lowLevelSystem));
    FMODAudio::ErrorCheck(lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));

    FMODAudio::ErrorCheck(fmod_system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
}

FMOD::Studio::System *FMODAudio::get_fmod_system() {
    assert(fmod_system != nullptr);
    return fmod_system;
}

FMOD::System *FMODAudio::get_fmod_core_system() {
    assert(lowLevelSystem != nullptr);
    return lowLevelSystem;
}

void FMODAudio::Init() { init_fmod_system(); }

void FMODAudio::Update() {
    Array<u64> pStoppedChannels;
    for (auto c : audio_channels) {
        bool bIsPlaying = false;
        (*c.value)->isPlaying(&bIsPlaying);
        if (!bIsPlaying) {
            pStoppedChannels.push(c.key);
        }
    }
    for (auto &it : pStoppedChannels) {
        audio_channels.unset(it);
    }
    FMODAudio::ErrorCheck(get_fmod_system()->update());
}

void FMODAudio::LoadSound(const String &strSoundName, bool b3d, bool bLooping, bool bStream) {
    u64 key = fnv1a(strSoundName);
    auto sound = this->sounds.get(key);
    if (sound != nullptr) return;

    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;
    FMOD::Sound *pSound = nullptr;
    FMODAudio::ErrorCheck(get_fmod_core_system()->createSound(strSoundName.cstr(), eMode, nullptr, &pSound));
    if (pSound) {
        this->sounds[key] = pSound;
    }
}

void FMODAudio::UnLoadSound(const String &strSoundName) {
    u64 key = fnv1a(strSoundName);
    auto sound = this->sounds.get(key);
    if (sound == nullptr) return;
    FMODAudio::ErrorCheck((*sound)->release());
    this->sounds.unset(key);
}

int FMODAudio::PlaySounds(const String &strSoundName, const vec3 &vPosition, float fVolumedB) {
    int channel_id = this->next_channel_id++;
    u64 key = fnv1a(strSoundName);
    auto sound = this->sounds.get(key);
    if (sound == nullptr) {
        LoadSound(strSoundName);
        sound = this->sounds.get(key);
        if (sound == nullptr) {
            return channel_id;
        }
    }
    FMOD::Channel *pChannel = nullptr;
    FMODAudio::ErrorCheck(get_fmod_core_system()->playSound((*sound), nullptr, true, &pChannel));
    if (pChannel) {
        FMOD_MODE currMode;
        (*sound)->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = VectorToFmod(vPosition);
            FMODAudio::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        FMODAudio::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        FMODAudio::ErrorCheck(pChannel->setPaused(false));
        this->audio_channels[channel_id] = pChannel;
    }
    return channel_id;
}

void FMODAudio::StopChannel(int channel_id) {
    auto channel = this->audio_channels.get(channel_id);
    if (channel == nullptr) return;

    FMODAudio::ErrorCheck((*channel)->stop());
}

void FMODAudio::StopAllChannels() {
    for (auto channel : this->audio_channels) {
        FMODAudio::ErrorCheck((*channel.value)->stop());
    }
}

bool FMODAudio::IsPlaying(int channel_id) const {
    auto channel = this->audio_channels.get(channel_id);
    if (channel == nullptr) return false;
    bool playing;
    FMODAudio::ErrorCheck((*channel)->isPlaying(&playing));
    return playing;
}

void FMODAudio::SetChannel3dPosition(int channel_id, const vec3 &vPosition) {
    auto channel = this->audio_channels.get(channel_id);
    if (channel == nullptr) return;

    FMOD_VECTOR position = VectorToFmod(vPosition);
    FMODAudio::ErrorCheck((*channel)->set3DAttributes(&position, NULL));
}

void FMODAudio::SetChannelVolume(int channel_id, float fVolumedB) {
    auto channel = this->audio_channels.get(channel_id);
    if (channel == nullptr) return;

    FMODAudio::ErrorCheck((*channel)->setVolume(dbToVolume(fVolumedB)));
}

void FMODAudio::LoadBank(const String &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {

    u64 key = fnv1a(strBankName);
    auto bank = this->banks.get(key);
    if (bank != nullptr) return;

    FMOD::Studio::Bank *pBank;
    FMODAudio::ErrorCheck(get_fmod_system()->loadBankFile(strBankName.cstr(), flags, &pBank));
    if (pBank) {
        this->banks[key] = pBank;
    }
}

void FMODAudio::LoadBankVfs(const String &filepath, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {

    u64 key = fnv1a(filepath);
    auto bank = this->banks.get(key);
    if (bank != nullptr) return;

    FMOD::Studio::Bank *pBank;

    String contents = {};
    bool success = vfs_read_entire_file(&contents, filepath);
    if (!success) {
        return;
    }
    neko_defer(mem_free(contents.data));

    FMODAudio::ErrorCheck(get_fmod_system()->loadBankMemory(contents.data, contents.len, FMOD_STUDIO_LOAD_MEMORY, flags, &pBank));

    if (pBank) {
        this->banks[key] = pBank;
    }
}

FMOD::Studio::Bank *FMODAudio::GetBank(const String &strBankName) {
    u64 key = fnv1a(strBankName);
    return this->banks[key];
}

void FMODAudio::LoadEvent(const String &strEventName) {
    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event != nullptr) return;

    FMOD::Studio::EventDescription *pEventDescription = NULL;
    FMODAudio::ErrorCheck(get_fmod_system()->getEvent(strEventName.cstr(), &pEventDescription));
    if (pEventDescription) {
        FMOD::Studio::EventInstance *pEventInstance = NULL;
        FMODAudio::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
        if (pEventInstance) {
            this->audio_events[key] = pEventInstance;
        }
    }
}

void FMODAudio::PlayEvent(const String &strEventName) {

    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event == nullptr) {
        LoadEvent(strEventName);
        event = this->audio_events.get(key);
        if (event == nullptr) return;
    }
    (*event)->start();
}

FMOD::Studio::EventInstance *FMODAudio::GetEvent(const String &strEventName) {
    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event == nullptr) {
        LoadEvent(strEventName);
        event = this->audio_events.get(key);
        if (event == nullptr) return nullptr;
    }
    return *event;
}

void FMODAudio::StopEvent(const String &strEventName, bool bImmediate) {
    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event == nullptr) return;
    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    FMODAudio::ErrorCheck((*event)->stop(eMode));
}

bool FMODAudio::IsEventPlaying(const String &strEventName) const {
    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event == nullptr) return false;

    FMOD_STUDIO_PLAYBACK_STATE *state = NULL;
    if ((*event)->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
        return true;
    }
    return false;
}

void FMODAudio::GetEventParameter(const String &strEventName, const String &strParameterName, float *parameter) {
    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event == nullptr) return;
    FMODAudio::ErrorCheck((*event)->getParameterByName(strParameterName.cstr(), parameter));
    // CAudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

void FMODAudio::SetEventParameter(const String &strEventName, const String &strParameterName, float fValue) {
    u64 key = fnv1a(strEventName);
    auto event = this->audio_events.get(key);
    if (event == nullptr) return;
    FMODAudio::ErrorCheck((*event)->setParameterByName(strParameterName.cstr(), fValue));
}

void FMODAudio::SetGlobalParameter(const String &strParameterName, float fValue) { get_fmod_system()->setParameterByName(strParameterName.cstr(), fValue); }

void FMODAudio::GetGlobalParameter(const String &strParameterName, float *parameter) { get_fmod_system()->getParameterByName(strParameterName.cstr(), parameter); }

FMOD_VECTOR FMODAudio::VectorToFmod(const vec3 &vPosition) {
    FMOD_VECTOR fVec;
    fVec.x = vPosition.x;
    fVec.y = vPosition.y;
    fVec.z = vPosition.z;
    return fVec;
}

int FMODAudio::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        std::map<int, std::string> values;
        Neko::reflection::guess_enum_range<FMOD_RESULT, 0>(values, std::make_integer_sequence<int, 81>());
        console_log("FMOD Error: %d %s", result, values[result].c_str());
        return -1;
    }
    return 0;
}

float FMODAudio::dbToVolume(float dB) { return powf(10.0f, 0.05f * dB); }

float FMODAudio::VolumeTodB(float volume) { return 20.0f * log10f(volume); }

void FMODAudio::Shutdown() {
    FMODAudio::ErrorCheck(get_fmod_system()->unloadAll());
    FMODAudio::ErrorCheck(get_fmod_system()->release());

    banks.trash();
    audio_events.trash();
    sounds.trash();
    audio_channels.trash();
}

}  // namespace Neko

static Neko::FMODAudio audio_fmod;

#endif

void audio_load_bank(int mode, String name, unsigned int type) {
    if (mode == 1) {
        audio_fmod.LoadBankVfs(name, type);
    } else {
        audio_fmod.LoadBank(name, type);
    }
}

FMOD::Studio::Bank *audio_load_event(String event) {
    audio_fmod.LoadEvent(event);
    return audio_fmod.GetBank(event);
}
void audio_play_event(String event) { audio_fmod.PlayEvent(event); }

void sound_init() {
    PROFILE_FUNC();

    {
#if NEKO_AUDIO == 1
        PROFILE_BLOCK("miniaudio");

        gApp->miniaudio_vfs = vfs_for_miniaudio();

        ma_engine_config ma_config = ma_engine_config_init();
        ma_config.channels = 2;
        ma_config.sampleRate = 44100;
        ma_config.pResourceManagerVFS = gApp->miniaudio_vfs;
        ma_result res = ma_engine_init(&ma_config, &gApp->audio_engine);
        if (res != MA_SUCCESS) {
            fatal_error("failed to initialize audio engine");
        }
#elif NEKO_AUDIO == 2
        audio_fmod.Init();
#endif
    }

    audio_load_bank(1, "assets/fmod/Build/Desktop/Master.bank", 0);
    audio_load_bank(1, "assets/fmod/Build/Desktop/Master.strings.bank", 0);

    String audio_event[] = {"event:/Music/Background1", "event:/Music/Title",   "event:/Player/Jump",    "event:/Player/Fly",      "event:/Player/Wind",    "event:/Player/Impact", "event:/World/Sand",
                            "event:/World/WaterFlow",   "event:/GUI/GUI_Hover", "event:/GUI/GUI_Slider", "event:/Player/Bow/Bow1", "event:/Player/Bow/Bow2"

    };

    for (auto s : audio_event) {
        audio_load_event(s);
    }

    // audio_play_event("event:/Player/Fly");
    // audio_play_event("event:/Player/Wind");
    // audio_play_event("event:/World/Sand");
    // audio_play_event("event:/World/WaterFlow");

    audio_play_event("event:/Music/Title");

    auto L = ENGINE_LUA();

    lua_register(
            L, "audio_load_bank", +[](lua_State *L) {
                const_str name = lua_tostring(L, 1);
                int type = lua_tointeger(L, 2);
                audio_load_bank(0, name, type);
                return 0;
            });
    lua_register(
            L, "audio_play_event", +[](lua_State *L) {
                const_str name = lua_tostring(L, 1);
                audio_play_event(name);
                return 0;
            });
    lua_register(
            L, "audio_load_event", +[](lua_State *L) {
                const_str name = lua_tostring(L, 1);
                audio_load_event(name);
                return 0;
            });
}

void sound_fini() {

#if NEKO_AUDIO == 1
    for (Sound *sound : gApp->garbage_sounds) {
        sound->trash();
    }
    gApp->garbage_sounds.trash();

    ma_engine_uninit(&gApp->audio_engine);
    mem_free(gApp->miniaudio_vfs);
#elif NEKO_AUDIO == 2
    audio_fmod.Shutdown();
#endif
}

int sound_update_all(App *app, event_t evt) { return 0; }

int sound_postupdate(App *app, event_t evt) {

#if NEKO_AUDIO == 1

    Array<Sound *> &sounds = app->garbage_sounds;
    for (u64 i = 0; i < sounds.len;) {
        Sound *sound = sounds[i];

        if (sound->dead_end) {
            assert(sound->zombie);
            sound->trash();
            mem_free(sound);

            sounds[i] = sounds[sounds.len - 1];
            sounds.len--;
        } else {
            i++;
        }
    }

#elif NEKO_AUDIO == 2
    audio_fmod.Update();
#endif

    return 0;
}

void sound_save_all(Store *s) {}
void sound_load_all(Store *s) {}