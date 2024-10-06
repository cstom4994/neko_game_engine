
#include "engine/sound.h"

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

    res = ma_sound_init_from_file(&g_app->audio_engine, cpath.data, 0, nullptr, nullptr, &sound->ma);
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
        g_app->garbage_sounds.push(sound);
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

namespace neko {

void event_instance::start() { assert(false && "Unimpl"); }

void event_instance::stop() { assert(false && "Unimpl"); }

event_instance event::instance() {
    assert(false && "Unimpl");
    return event_instance{};
}

bank::bank(FMOD::Studio::Bank *fmod_bank) : fmod_bank(fmod_bank) {
    int num_events;
    check_err(fmod_bank->getEventCount(&num_events));
    std::cout << "Num events = " << num_events << std::endl;
    if (num_events > 0) {
        std::vector<FMOD::Studio::EventDescription *> event_descriptions(num_events);
        check_err(fmod_bank->getEventList(event_descriptions.data(), num_events, &num_events));

        for (int ii = 0; ii < num_events; ++ii) {
            // Get the event name
            int name_len;
            check_err(event_descriptions[ii]->getPath(nullptr, 0, &name_len));
            char *name_chars = new char[name_len];
            check_err(event_descriptions[ii]->getPath(name_chars, name_len, nullptr));
            std::string name(name_chars);
            delete[] name_chars;
            std::cout << "Event name = " << name.c_str() << std::endl;
            event_map.insert(std::make_pair(name, event{event_descriptions[ii], name}));
        }
    }
}

event *bank::load_event(const char *path) {
    const auto cached = event_map.find(path);
    return (cached != event_map.end()) ? &cached->second : nullptr;
}

bank_manager *bank_manager_instance = nullptr;

bank_manager::bank_manager() {}

bank_manager &bank_manager::instance() {
    if (!bank_manager_instance) {
        bank_manager_instance = new bank_manager();
    }
    return *bank_manager_instance;
}

bank *bank_manager::load(const char *path) {
    const auto cached = bank_map.find(path);
    if (cached != bank_map.end()) {
        return &cached->second;
    }
    const auto system = get_fmod_system();
    FMOD::Studio::Bank *fmod_bank = nullptr;
    check_err(system->loadBankFile(path, FMOD_STUDIO_LOAD_BANK_NORMAL, &fmod_bank));

    // make the bank & insert
    bank b{fmod_bank};
    return &bank_map.insert(std::make_pair(std::string(path), std::move(b))).first->second;
}

FMOD::Studio::System *fmod_system = nullptr;
FMOD::System *lowLevelSystem = nullptr;

void init_fmod_system() {
    check_err(FMOD::Studio::System::create(&fmod_system));

    check_err(fmod_system->getCoreSystem(&lowLevelSystem));
    check_err(lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));

    check_err(fmod_system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
}

FMOD::Studio::System *get_fmod_system() {
    assert(fmod_system != nullptr);
    return fmod_system;
}

FMOD::System *get_fmod_core_system() {
    assert(lowLevelSystem != nullptr);
    return lowLevelSystem;
}

Audio::Implementation::Implementation() { init_fmod_system(); }

Audio::Implementation::~Implementation() {
    Audio::ErrorCheck(get_fmod_system()->unloadAll());
    Audio::ErrorCheck(get_fmod_system()->release());
}

void Audio::Implementation::Update() {
    std::vector<ChannelMap::iterator> pStoppedChannels;
    for (auto it = audio_channels.begin(), itEnd = audio_channels.end(); it != itEnd; ++it) {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying) {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto &it : pStoppedChannels) {
        audio_channels.erase(it);
    }
    Audio::ErrorCheck(get_fmod_system()->update());
}

Audio::Implementation *Audio::fmod_impl;

void Audio::Init() { fmod_impl = new Implementation; }

void Audio::Update() { fmod_impl->Update(); }

void Audio::LoadSound(const std::string &strSoundName, bool b3d, bool bLooping, bool bStream) {
    auto tFoundIt = fmod_impl->sounds.find(strSoundName);
    if (tFoundIt != fmod_impl->sounds.end()) return;
    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;
    FMOD::Sound *pSound = nullptr;
    Audio::ErrorCheck(get_fmod_core_system()->createSound(strSoundName.c_str(), eMode, nullptr, &pSound));
    if (pSound) {
        fmod_impl->sounds[strSoundName] = pSound;
    }
}

void Audio::UnLoadSound(const std::string &strSoundName) {
    auto tFoundIt = fmod_impl->sounds.find(strSoundName);
    if (tFoundIt == fmod_impl->sounds.end()) return;
    Audio::ErrorCheck(tFoundIt->second->release());
    fmod_impl->sounds.erase(tFoundIt);
}

int Audio::PlaySounds(const std::string &strSoundName, const vec3 &vPosition, float fVolumedB) {
    int channel_id = fmod_impl->next_channel_id++;
    auto tFoundIt = fmod_impl->sounds.find(strSoundName);
    if (tFoundIt == fmod_impl->sounds.end()) {
        LoadSound(strSoundName);
        tFoundIt = fmod_impl->sounds.find(strSoundName);
        if (tFoundIt == fmod_impl->sounds.end()) {
            return channel_id;
        }
    }
    FMOD::Channel *pChannel = nullptr;
    Audio::ErrorCheck(get_fmod_core_system()->playSound(tFoundIt->second, nullptr, true, &pChannel));
    if (pChannel) {
        FMOD_MODE currMode;
        tFoundIt->second->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = VectorToFmod(vPosition);
            Audio::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        Audio::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        Audio::ErrorCheck(pChannel->setPaused(false));
        fmod_impl->audio_channels[channel_id] = pChannel;
    }
    return channel_id;
}

void Audio::SetChannel3dPosition(int channel_id, const vec3 &vPosition) {
    auto tFoundIt = fmod_impl->audio_channels.find(channel_id);
    if (tFoundIt == fmod_impl->audio_channels.end()) return;

    FMOD_VECTOR position = VectorToFmod(vPosition);
    Audio::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

void Audio::SetChannelVolume(int channel_id, float fVolumedB) {
    auto tFoundIt = fmod_impl->audio_channels.find(channel_id);
    if (tFoundIt == fmod_impl->audio_channels.end()) return;

    Audio::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

void Audio::LoadBank(const std::string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
    auto tFoundIt = fmod_impl->banks.find(strBankName);
    if (tFoundIt != fmod_impl->banks.end()) return;
    FMOD::Studio::Bank *pBank;
    Audio::ErrorCheck(get_fmod_system()->loadBankFile(strBankName.c_str(), flags, &pBank));
    if (pBank) {
        fmod_impl->banks[strBankName] = pBank;
    }
}

FMOD::Studio::Bank *Audio::GetBank(const std::string &strBankName) { return fmod_impl->banks[strBankName]; }

void Audio::LoadEvent(const std::string &strEventName) {
    auto tFoundit = fmod_impl->audio_events.find(strEventName);
    if (tFoundit != fmod_impl->audio_events.end()) return;
    FMOD::Studio::EventDescription *pEventDescription = NULL;
    Audio::ErrorCheck(get_fmod_system()->getEvent(strEventName.c_str(), &pEventDescription));
    if (pEventDescription) {
        FMOD::Studio::EventInstance *pEventInstance = NULL;
        Audio::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
        if (pEventInstance) {
            fmod_impl->audio_events[strEventName] = pEventInstance;
        }
    }
}

void Audio::PlayEvent(const std::string &strEventName) {
    auto tFoundit = fmod_impl->audio_events.find(strEventName);
    if (tFoundit == fmod_impl->audio_events.end()) {
        LoadEvent(strEventName);
        tFoundit = fmod_impl->audio_events.find(strEventName);
        if (tFoundit == fmod_impl->audio_events.end()) return;
    }
    tFoundit->second->start();
}

FMOD::Studio::EventInstance *Audio::GetEvent(const std::string &strEventName) {
    auto tFoundit = fmod_impl->audio_events.find(strEventName);
    if (tFoundit == fmod_impl->audio_events.end()) {
        LoadEvent(strEventName);
        tFoundit = fmod_impl->audio_events.find(strEventName);
        if (tFoundit == fmod_impl->audio_events.end()) return nullptr;
    }
    return tFoundit->second;
}

void Audio::StopEvent(const std::string &strEventName, bool bImmediate) {
    auto tFoundIt = fmod_impl->audio_events.find(strEventName);
    if (tFoundIt == fmod_impl->audio_events.end()) return;
    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    Audio::ErrorCheck(tFoundIt->second->stop(eMode));
}

bool Audio::IsEventPlaying(const std::string &strEventName) const {
    auto tFoundIt = fmod_impl->audio_events.find(strEventName);
    if (tFoundIt == fmod_impl->audio_events.end()) return false;

    FMOD_STUDIO_PLAYBACK_STATE *state = NULL;
    if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
        return true;
    }
    return false;
}

void Audio::GetEventParameter(const std::string &strEventName, const std::string &strParameterName, float *parameter) {
    auto tFoundIt = fmod_impl->audio_events.find(strEventName);
    if (tFoundIt == fmod_impl->audio_events.end()) return;
    Audio::ErrorCheck(tFoundIt->second->getParameterByName(strParameterName.c_str(), parameter));
    // CAudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

void Audio::SetEventParameter(const std::string &strEventName, const std::string &strParameterName, float fValue) {
    auto tFoundIt = fmod_impl->audio_events.find(strEventName);
    if (tFoundIt == fmod_impl->audio_events.end()) return;
    Audio::ErrorCheck(tFoundIt->second->setParameterByName(strParameterName.c_str(), fValue));
}

void Audio::SetGlobalParameter(const std::string &strParameterName, float fValue) { get_fmod_system()->setParameterByName(strParameterName.c_str(), fValue); }

void Audio::GetGlobalParameter(const std::string &strParameterName, float *parameter) { get_fmod_system()->getParameterByName(strParameterName.c_str(), parameter); }

FMOD_VECTOR Audio::VectorToFmod(const vec3 &vPosition) {
    FMOD_VECTOR fVec;
    fVec.x = vPosition.x;
    fVec.y = vPosition.y;
    fVec.z = vPosition.z;
    return fVec;
}

int Audio::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        console_log("[Audio] FMOD Error: %d", result);
        return -1;
    }
    return 0;
}

float Audio::dbToVolume(float dB) { return powf(10.0f, 0.05f * dB); }

float Audio::VolumeTodB(float volume) { return 20.0f * log10f(volume); }

void Audio::Shutdown() { delete fmod_impl; }

void AudioEngineInit() {}

}  // namespace neko

static neko::Audio audio_fmod;

#endif

void audio_load_bank(std::string name, unsigned int type) { audio_fmod.LoadBank(name, type); }

void audio_load_event(std::string event) { audio_fmod.LoadEvent(event); }
void audio_play_event(std::string event) { audio_fmod.PlayEvent(event); }

void sound_init() {
    PROFILE_FUNC();

    {
#if NEKO_AUDIO == 1
        PROFILE_BLOCK("miniaudio");

        g_app->miniaudio_vfs = vfs_for_miniaudio();

        ma_engine_config ma_config = ma_engine_config_init();
        ma_config.channels = 2;
        ma_config.sampleRate = 44100;
        ma_config.pResourceManagerVFS = g_app->miniaudio_vfs;
        ma_result res = ma_engine_init(&ma_config, &g_app->audio_engine);
        if (res != MA_SUCCESS) {
            fatal_error("failed to initialize audio engine");
        }
#elif NEKO_AUDIO == 2
        audio_fmod.Init();
#endif
    }

    audio_load_bank("fmod/Build/Desktop/Master.bank", 0);
    audio_load_bank("fmod/Build/Desktop/Master.strings.bank", 0);

    std::string audio_event[] = {"event:/Music/Background1", "event:/Music/Title", "event:/Player/Jump",     "event:/Player/Fly",    "event:/Player/Wind",
                                 "event:/Player/Impact",     "event:/World/Sand",  "event:/World/WaterFlow", "event:/GUI/GUI_Hover", "event:/GUI/GUI_Slider"};

    for (auto s : audio_event) {
        audio_load_event(s);
    }

    // audio_play_event("event:/Player/Fly");
    // audio_play_event("event:/Player/Wind");
    // audio_play_event("event:/World/Sand");
    // audio_play_event("event:/World/WaterFlow");

    audio_play_event("event:/Music/Title");
}

void sound_fini() {

#if NEKO_AUDIO == 1
    for (Sound *sound : g_app->garbage_sounds) {
        sound->trash();
    }
    g_app->garbage_sounds.trash();

    ma_engine_uninit(&g_app->audio_engine);
    mem_free(g_app->miniaudio_vfs);
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