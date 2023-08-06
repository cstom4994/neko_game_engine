

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/math/neko_math.h"
#include "engine/utility/logger.hpp"

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

#pragma region FMODWrapper

class event;

class event_instance {
    event *type;

    void start();
    void stop();
};

/**
   Models FMOD event descriptions
*/
struct event {
    FMOD::Studio::EventDescription *fmod_bank;
    std::string path;

    event_instance instance();
};

struct bank {
    /** Instance of the fmod bank object */
    FMOD::Studio::Bank *fmod_bank;

    std::unordered_map<std::string, event> event_map;

    bank(FMOD::Studio::Bank *fmod_bank);

    /** Load an event description, caching if already loaded */
    event *load_event(const char *path);
};

/** Singleton for storing banks */
class bank_manager {
    std::unordered_map<std::string, bank> bank_map;

    bank_manager();

public:
    static bank_manager &instance();

    /** Unload a bank */
    void unload(const char *bank_path);

    /** Returns a pointer to the bank if loaded, or null for a failure (error
      printed to console). Caches, so future calls will load the same bank. */
    bank *load(const char *path);
};

/** Initialise the fmod system */
void init_fmod_system();

/** Get the fmod system - asserts if not initialised */
FMOD::Studio::System *get_fmod_system();
FMOD::System *get_fmod_core_system();

struct fmod_exception {
    const char *message;
};

/** Checks the result, printing it out and throwing an 'fmod_exception' if the
    result is an error. */
inline void check_err(FMOD_RESULT res) {
    if (res != FMOD_OK) {
        // throw fmod_exception{FMOD_ErrorString(res)};
    }
}

#pragma endregion FMODWrapper

struct neko_fmod_impl {
    neko_fmod_impl();
    ~neko_fmod_impl();

    void Update();

    int mnNextChannelId;

    typedef std::map<std::string, FMOD::Sound *> SoundMap;
    typedef std::map<int, FMOD::Channel *> ChannelMap;
    typedef std::map<std::string, FMOD::Studio::EventInstance *> EventMap;
    typedef std::map<std::string, FMOD::Studio::Bank *> BankMap;

    BankMap mBanks;
    EventMap mEvents;
    SoundMap mSounds;
    ChannelMap mChannels;
};

class neko_fmod {
public:
    static void Init();
    static void Update();
    static void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    void LoadBank(const std::string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    FMOD::Studio::Bank *GetBank(const std::string &strBankName);
    void LoadEvent(const std::string &strEventName);
    void LoadSound(const std::string &strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const std::string &strSoundName);
    void Set3dListenerAndOrientation(const neko_vec3 &vPosition, const neko_vec3 &vLook, const neko_vec3 &vUp);
    int PlaySounds(const std::string &strSoundName, const neko_vec3 &vPos = neko_vec3{0, 0, 0}, float fVolumedB = 0.0f);
    void PlayEvent(const std::string &strEventName);
    FMOD::Studio::EventInstance *GetEvent(const std::string &strEventName);
    void StopChannel(int nChannelId);
    void StopEvent(const std::string &strEventName, bool bImmediate = false);
    void GetEventParameter(const std::string &strEventName, const std::string &strEventParameter, float *parameter);
    void SetEventParameter(const std::string &strEventName, const std::string &strParameterName, float fValue);
    void SetGlobalParameter(const std::string &strParameterName, float fValue);
    void GetGlobalParameter(const std::string &strEventParameter, float *parameter);
    void StopAllChannels();
    void SetChannel3dPosition(int nChannelId, const neko_vec3 &vPosition);
    void SetChannelVolume(int nChannelId, float fVolumedB);
    bool IsPlaying(int nChannelId) const;
    bool IsEventPlaying(const std::string &strEventName) const;
    float dbToVolume(float dB);
    float VolumeTodB(float volume);
    FMOD_VECTOR VectorToFmod(const neko_vec3 &vPosition);
};

}  // namespace neko

namespace neko {

void event_instance::start() { neko_assert(false && "Unimpl"); }

void event_instance::stop() { neko_assert(false && "Unimpl"); }

event_instance event::instance() {
    neko_assert(false && "Unimpl");
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
            std::cout << "Event name = " << name << std::endl;
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
    neko_assert(fmod_system != nullptr);
    return fmod_system;
}

FMOD::System *get_fmod_core_system() {
    neko_assert(lowLevelSystem != nullptr);
    return lowLevelSystem;
}

neko_fmod_impl::neko_fmod_impl() { init_fmod_system(); }

neko_fmod_impl::~neko_fmod_impl() {
    neko_fmod::ErrorCheck(get_fmod_system()->unloadAll());
    neko_fmod::ErrorCheck(get_fmod_system()->release());
}

void neko_fmod_impl::Update() {
    std::vector<ChannelMap::iterator> pStoppedChannels;
    for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it) {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying) {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto &it : pStoppedChannels) {
        mChannels.erase(it);
    }
    neko_fmod::ErrorCheck(get_fmod_system()->update());
}

neko_fmod_impl *sgpImplementation = nullptr;

void neko_fmod::Init() { sgpImplementation = new neko_fmod_impl; }

void neko_fmod::Update() { sgpImplementation->Update(); }

void neko_fmod::LoadSound(const std::string &strSoundName, bool b3d, bool bLooping, bool bStream) {
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt != sgpImplementation->mSounds.end()) return;
    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;
    FMOD::Sound *pSound = nullptr;
    neko_fmod::ErrorCheck(get_fmod_core_system()->createSound(strSoundName.c_str(), eMode, nullptr, &pSound));
    if (pSound) {
        sgpImplementation->mSounds[strSoundName] = pSound;
    }
}

void neko_fmod::UnLoadSound(const std::string &strSoundName) {
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end()) return;
    neko_fmod::ErrorCheck(tFoundIt->second->release());
    sgpImplementation->mSounds.erase(tFoundIt);
}

int neko_fmod::PlaySounds(const std::string &strSoundName, const neko_vec3 &vPosition, float fVolumedB) {
    int nChannelId = sgpImplementation->mnNextChannelId++;
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end()) {
        LoadSound(strSoundName);
        tFoundIt = sgpImplementation->mSounds.find(strSoundName);
        if (tFoundIt == sgpImplementation->mSounds.end()) {
            return nChannelId;
        }
    }
    FMOD::Channel *pChannel = nullptr;
    neko_fmod::ErrorCheck(get_fmod_core_system()->playSound(tFoundIt->second, nullptr, true, &pChannel));
    if (pChannel) {
        FMOD_MODE currMode;
        tFoundIt->second->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = VectorToFmod(vPosition);
            neko_fmod::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        neko_fmod::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        neko_fmod::ErrorCheck(pChannel->setPaused(false));
        sgpImplementation->mChannels[nChannelId] = pChannel;
    }
    return nChannelId;
}

void neko_fmod::SetChannel3dPosition(int nChannelId, const neko_vec3 &vPosition) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) return;

    FMOD_VECTOR position = VectorToFmod(vPosition);
    neko_fmod::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

void neko_fmod::SetChannelVolume(int nChannelId, float fVolumedB) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) return;

    neko_fmod::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

void neko_fmod::LoadBank(const std::string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
    auto tFoundIt = sgpImplementation->mBanks.find(strBankName);
    if (tFoundIt != sgpImplementation->mBanks.end()) return;
    FMOD::Studio::Bank *pBank;
    neko_fmod::ErrorCheck(get_fmod_system()->loadBankFile(strBankName.c_str(), flags, &pBank));
    if (pBank) {
        sgpImplementation->mBanks[strBankName] = pBank;
    }
}

FMOD::Studio::Bank *neko_fmod::GetBank(const std::string &strBankName) { return sgpImplementation->mBanks[strBankName]; }

void neko_fmod::LoadEvent(const std::string &strEventName) {
    auto tFoundit = sgpImplementation->mEvents.find(strEventName);
    if (tFoundit != sgpImplementation->mEvents.end()) return;
    FMOD::Studio::EventDescription *pEventDescription = NULL;
    neko_fmod::ErrorCheck(get_fmod_system()->getEvent(strEventName.c_str(), &pEventDescription));
    if (pEventDescription) {
        FMOD::Studio::EventInstance *pEventInstance = NULL;
        neko_fmod::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
        if (pEventInstance) {
            sgpImplementation->mEvents[strEventName] = pEventInstance;
        }
    }
}

void neko_fmod::PlayEvent(const std::string &strEventName) {
    auto tFoundit = sgpImplementation->mEvents.find(strEventName);
    if (tFoundit == sgpImplementation->mEvents.end()) {
        LoadEvent(strEventName);
        tFoundit = sgpImplementation->mEvents.find(strEventName);
        if (tFoundit == sgpImplementation->mEvents.end()) return;
    }
    tFoundit->second->start();
}

FMOD::Studio::EventInstance *neko_fmod::GetEvent(const std::string &strEventName) {
    auto tFoundit = sgpImplementation->mEvents.find(strEventName);
    if (tFoundit == sgpImplementation->mEvents.end()) {
        LoadEvent(strEventName);
        tFoundit = sgpImplementation->mEvents.find(strEventName);
        if (tFoundit == sgpImplementation->mEvents.end()) return nullptr;
    }
    return tFoundit->second;
}

void neko_fmod::StopEvent(const std::string &strEventName, bool bImmediate) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) return;
    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    neko_fmod::ErrorCheck(tFoundIt->second->stop(eMode));
}

bool neko_fmod::IsEventPlaying(const std::string &strEventName) const {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) return false;

    FMOD_STUDIO_PLAYBACK_STATE *state = NULL;
    if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
        return true;
    }
    return false;
}

void neko_fmod::GetEventParameter(const std::string &strEventName, const std::string &strParameterName, float *parameter) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) return;
    neko_fmod::ErrorCheck(tFoundIt->second->getParameterByName(strParameterName.c_str(), parameter));
    // CAudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

void neko_fmod::SetEventParameter(const std::string &strEventName, const std::string &strParameterName, float fValue) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) return;
    neko_fmod::ErrorCheck(tFoundIt->second->setParameterByName(strParameterName.c_str(), fValue));
}

void neko_fmod::SetGlobalParameter(const std::string &strParameterName, float fValue) { get_fmod_system()->setParameterByName(strParameterName.c_str(), fValue); }

void neko_fmod::GetGlobalParameter(const std::string &strParameterName, float *parameter) { get_fmod_system()->getParameterByName(strParameterName.c_str(), parameter); }

FMOD_VECTOR neko_fmod::VectorToFmod(const neko_vec3 &vPosition) {
    FMOD_VECTOR fVec;
    fVec.x = vPosition.x;
    fVec.y = vPosition.y;
    fVec.z = vPosition.z;
    return fVec;
}

int neko_fmod::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        neko_error("[Audio] FMOD Error: ", result);
        return neko_result_incomplete;
    }
    // cout << "FMOD all good" << endl;
    return neko_result_success;
}

float neko_fmod::dbToVolume(float dB) { return powf(10.0f, 0.05f * dB); }

float neko_fmod::VolumeTodB(float volume) { return 20.0f * log10f(volume); }

void neko_fmod::Shutdown() { delete sgpImplementation; }

}  // namespace neko