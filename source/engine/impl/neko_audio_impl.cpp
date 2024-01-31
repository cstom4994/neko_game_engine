
#include "engine/neko_audio.h"
#include "engine/neko_engine.h"

#define CUTE_SOUND_IMPLEMENTATION
#define CUTE_SOUND_PLATFORM_WINDOWS
//  #define CUTE_SOUND_ALLOC(size, ctx) neko_gc_malloc(&g_gc, size)
//  #define CUTE_SOUND_FREE(mem, ctx) neko_gc_free(&g_gc, mem)
//  #define HASHTABLE_MALLOC(ctx, size) (neko_gc_malloc(&g_gc, size))
//  #define HASHTABLE_FREE(ctx, ptr) (neko_gc_free(&g_gc, ptr))
// DDD #include "engine/builtin/cute_sound.h"

void __neko_audio_set_default_functions(struct neko_audio_s *audio);

neko_result __neko_audio_init(struct neko_audio_s *s) {

    neko_platform_t *platform = neko_subsystem(platform);

    // DDD HWND hwnd = (HWND)neko_platform_hwnd();
    // DDD cs_init(hwnd, 44100, 1024, NULL);

    // DDD cs_spawn_mix_thread();
    // cs_mix_thread_sleep_delay(1);

    return NEKO_RESULT_SUCCESS;
}
neko_result __neko_audio_shutdown(struct neko_audio_s *s) {

    // DDD cs_shutdown();

    return NEKO_RESULT_SUCCESS;
}
neko_result __neko_audio_update(struct neko_audio_s *s) {

    // DDD cs_update(neko_subsystem(platform)->time.delta);

    return NEKO_RESULT_SUCCESS;
}
neko_result __neko_audio_commit(struct neko_audio_s *s) { return NEKO_RESULT_SUCCESS; }

NEKO_API_DECL struct neko_audio_s *__neko_audio_construct() {
    neko_audio_t *audio = (neko_audio_t *)neko_safe_malloc(sizeof(neko_audio_t));
    neko_audio_data_t *data = (neko_audio_data_t *)neko_safe_malloc(sizeof(neko_audio_data_t));

    data->internal = NULL;
    // data->instance_cache = neko_resource_cache_new(neko_audio_instance_t);
    // data->audio_cache = neko_resource_cache_new(neko_audio_source_t);

    // Set data
    audio->user_data = NULL;
    audio->data = data;

    // Default audio max/min
    audio->max_audio_volume = 1.f;
    audio->min_audio_volume = 0.f;

    // Default internals
    __neko_audio_set_default_functions(audio);

    return audio;
}

NEKO_API_DECL void neko_audio_shutdown(neko_audio_t *audio) { __neko_audio_shutdown(audio); }

NEKO_API_DECL void neko_audio_destroy(neko_audio_t *audio) {
    neko_safe_free(audio->data);
    neko_safe_free(audio);
}

void __neko_audio_set_default_functions(struct neko_audio_s *audio) {

    audio->init = &__neko_audio_init;
    audio->update = &__neko_audio_update;
    // audio->shutdown = &__neko_audio_shutdown;
    audio->commit = &__neko_audio_commit;
}

namespace neko {

void fmod_event_instance::start() { assert(false && "Unimpl"); }

void fmod_event_instance::stop() { assert(false && "Unimpl"); }

fmod_event_instance fmod_event::instance() {
    assert(false && "Unimpl");
    return fmod_event_instance{};
}

fmod_bank::fmod_bank(FMOD::Studio::Bank *fmod_bank) : fmod_banks(fmod_bank) {
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
            std::cout << "Event name = " << name << std::endl;
            event_map.insert(std::make_pair(name, fmod_event{event_descriptions[ii], name}));
        }
    }
}

fmod_event *fmod_bank::load_event(const char *path) {
    const auto cached = event_map.find(path);
    return (cached != event_map.end()) ? &cached->second : nullptr;
}

fmod_bank_manager *bank_manager_instance = nullptr;

fmod_bank_manager::fmod_bank_manager() {}

fmod_bank_manager &fmod_bank_manager::instance() {
    if (!bank_manager_instance) {
        bank_manager_instance = new fmod_bank_manager();
    }
    return *bank_manager_instance;
}

fmod_bank *fmod_bank_manager::load(const char *path) {
    const auto cached = bank_map.find(path);
    if (cached != bank_map.end()) {
        return &cached->second;
    }
    const auto system = get_fmod_system();
    FMOD::Studio::Bank *fmod_banks = nullptr;
    check_err(system->loadBankFile(path, FMOD_STUDIO_LOAD_BANK_NORMAL, &fmod_banks));

    // make the bank & insert
    fmod_bank b{fmod_banks};
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

fmod_impl::fmod_impl() { init_fmod_system(); }

fmod_impl::~fmod_impl() {
    fmod_audio::ErrorCheck(get_fmod_system()->unloadAll());
    fmod_audio::ErrorCheck(get_fmod_system()->release());
}

void fmod_impl::update() {
    std::vector<fmod_channel_map::iterator> pStoppedChannels;
    for (auto it = m_channels.begin(), itEnd = m_channels.end(); it != itEnd; ++it) {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying) {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto &it : pStoppedChannels) {
        m_channels.erase(it);
    }
    fmod_audio::ErrorCheck(get_fmod_system()->update());
}

fmod_impl *g_fmod_impl = nullptr;

void fmod_audio::Init() { g_fmod_impl = new fmod_impl; }

void fmod_audio::Update() { g_fmod_impl->update(); }

void fmod_audio::LoadSound(const std::string &strSoundName, bool b3d, bool bLooping, bool bStream) {
    auto tFoundIt = g_fmod_impl->m_sounds.find(strSoundName);
    if (tFoundIt != g_fmod_impl->m_sounds.end()) return;
    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;
    FMOD::Sound *pSound = nullptr;
    fmod_audio::ErrorCheck(get_fmod_core_system()->createSound(strSoundName.c_str(), eMode, nullptr, &pSound));
    if (pSound) {
        g_fmod_impl->m_sounds[strSoundName] = pSound;
    }
}

void fmod_audio::UnLoadSound(const std::string &strSoundName) {
    auto tFoundIt = g_fmod_impl->m_sounds.find(strSoundName);
    if (tFoundIt == g_fmod_impl->m_sounds.end()) return;
    fmod_audio::ErrorCheck(tFoundIt->second->release());
    g_fmod_impl->m_sounds.erase(tFoundIt);
}

int fmod_audio::PlaySounds(const std::string &strSoundName, const neko_vec3 &vPosition, float fVolumedB) {
    int nChannelId = g_fmod_impl->next_channel_id++;
    auto tFoundIt = g_fmod_impl->m_sounds.find(strSoundName);
    if (tFoundIt == g_fmod_impl->m_sounds.end()) {
        LoadSound(strSoundName);
        tFoundIt = g_fmod_impl->m_sounds.find(strSoundName);
        if (tFoundIt == g_fmod_impl->m_sounds.end()) {
            return nChannelId;
        }
    }
    FMOD::Channel *pChannel = nullptr;
    fmod_audio::ErrorCheck(get_fmod_core_system()->playSound(tFoundIt->second, nullptr, true, &pChannel));
    if (pChannel) {
        FMOD_MODE currMode;
        tFoundIt->second->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = VectorToFmod(vPosition);
            fmod_audio::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        fmod_audio::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        fmod_audio::ErrorCheck(pChannel->setPaused(false));
        g_fmod_impl->m_channels[nChannelId] = pChannel;
    }
    return nChannelId;
}

void fmod_audio::SetChannel3dPosition(int nChannelId, const neko_vec3 &vPosition) {
    auto tFoundIt = g_fmod_impl->m_channels.find(nChannelId);
    if (tFoundIt == g_fmod_impl->m_channels.end()) return;

    FMOD_VECTOR position = VectorToFmod(vPosition);
    fmod_audio::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

void fmod_audio::SetChannelVolume(int nChannelId, float fVolumedB) {
    auto tFoundIt = g_fmod_impl->m_channels.find(nChannelId);
    if (tFoundIt == g_fmod_impl->m_channels.end()) return;

    fmod_audio::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

void fmod_audio::LoadBank(const std::string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
    auto tFoundIt = g_fmod_impl->m_banks.find(strBankName);
    if (tFoundIt != g_fmod_impl->m_banks.end()) return;
    FMOD::Studio::Bank *pBank;
    fmod_audio::ErrorCheck(get_fmod_system()->loadBankFile(strBankName.c_str(), flags, &pBank));
    if (pBank) {
        g_fmod_impl->m_banks[strBankName] = pBank;
    }
}

FMOD::Studio::Bank *fmod_audio::GetBank(const std::string &strBankName) { return g_fmod_impl->m_banks[strBankName]; }

void fmod_audio::LoadEvent(const std::string &strEventName) {
    auto tFoundit = g_fmod_impl->m_events.find(strEventName);
    if (tFoundit != g_fmod_impl->m_events.end()) return;
    FMOD::Studio::EventDescription *pEventDescription = NULL;
    fmod_audio::ErrorCheck(get_fmod_system()->getEvent(strEventName.c_str(), &pEventDescription));
    if (pEventDescription) {
        FMOD::Studio::EventInstance *pEventInstance = NULL;
        fmod_audio::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
        if (pEventInstance) {
            g_fmod_impl->m_events[strEventName] = pEventInstance;
        }
    }
}

void fmod_audio::PlayEvent(const std::string &strEventName) {
    auto tFoundit = g_fmod_impl->m_events.find(strEventName);
    if (tFoundit == g_fmod_impl->m_events.end()) {
        LoadEvent(strEventName);
        tFoundit = g_fmod_impl->m_events.find(strEventName);
        if (tFoundit == g_fmod_impl->m_events.end()) return;
    }
    tFoundit->second->start();
}

FMOD::Studio::EventInstance *fmod_audio::GetEvent(const std::string &strEventName) {
    auto tFoundit = g_fmod_impl->m_events.find(strEventName);
    if (tFoundit == g_fmod_impl->m_events.end()) {
        LoadEvent(strEventName);
        tFoundit = g_fmod_impl->m_events.find(strEventName);
        if (tFoundit == g_fmod_impl->m_events.end()) return nullptr;
    }
    return tFoundit->second;
}

void fmod_audio::StopEvent(const std::string &strEventName, bool bImmediate) {
    auto tFoundIt = g_fmod_impl->m_events.find(strEventName);
    if (tFoundIt == g_fmod_impl->m_events.end()) return;
    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    fmod_audio::ErrorCheck(tFoundIt->second->stop(eMode));
}

bool fmod_audio::IsEventPlaying(const std::string &strEventName) const {
    auto tFoundIt = g_fmod_impl->m_events.find(strEventName);
    if (tFoundIt == g_fmod_impl->m_events.end()) return false;

    FMOD_STUDIO_PLAYBACK_STATE *state = NULL;
    if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
        return true;
    }
    return false;
}

void fmod_audio::GetEventParameter(const std::string &strEventName, const std::string &strParameterName, float *parameter) {
    auto tFoundIt = g_fmod_impl->m_events.find(strEventName);
    if (tFoundIt == g_fmod_impl->m_events.end()) return;
    fmod_audio::ErrorCheck(tFoundIt->second->getParameterByName(strParameterName.c_str(), parameter));
    // CAudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

void fmod_audio::SetEventParameter(const std::string &strEventName, const std::string &strParameterName, float fValue) {
    auto tFoundIt = g_fmod_impl->m_events.find(strEventName);
    if (tFoundIt == g_fmod_impl->m_events.end()) return;
    fmod_audio::ErrorCheck(tFoundIt->second->setParameterByName(strParameterName.c_str(), fValue));
}

void fmod_audio::SetGlobalParameter(const std::string &strParameterName, float fValue) { get_fmod_system()->setParameterByName(strParameterName.c_str(), fValue); }

void fmod_audio::GetGlobalParameter(const std::string &strParameterName, float *parameter) { get_fmod_system()->getParameterByName(strParameterName.c_str(), parameter); }

FMOD_VECTOR fmod_audio::VectorToFmod(const neko_vec3 &vPosition) {
    FMOD_VECTOR fVec;
    fVec.x = vPosition.x;
    fVec.y = vPosition.y;
    fVec.z = vPosition.z;
    return fVec;
}

int fmod_audio::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        neko_log_error("[Audio] FMOD Error: %s", result);
        return -1;
    }
    // cout << "FMOD all good" << endl;
    return 0;
}

float fmod_audio::dbToVolume(float dB) { return powf(10.0f, 0.05f * dB); }

float fmod_audio::VolumeTodB(float volume) { return 20.0f * log10f(volume); }

void fmod_audio::Shutdown() { delete g_fmod_impl; }

}  // namespace neko