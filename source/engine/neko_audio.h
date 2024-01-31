#ifndef NEKO_AUDIO_H
#define NEKO_AUDIO_H

#include "engine/neko.h"

// sound
// DDD #include "engine/builtin/cute_sound.h"

/*=====================
// Internal Audio Data
=====================*/

typedef struct neko_audio_data_t {

    // Any internal data required for audio API
    void *internal;

} neko_audio_data_t;

typedef struct neko_audio_s {
    /*============================================================
    // Audio Initilization / De-Initialization
    ============================================================*/

    neko_result (*init)(struct neko_audio_s *);
    // neko_result (*shutdown)(struct neko_audio_s*);
    neko_result (*update)(struct neko_audio_s *);
    neko_result (*commit)(struct neko_audio_s *);

    /*============================================================
    // Audio Instance Data
    ============================================================*/

    // Proably
    f32 max_audio_volume;
    f32 min_audio_volume;

    // All internal API specific data for audio system
    void *data;

    // Any custom user data (for custom API implementations)
    void *user_data;
} neko_audio_t;

// Extern internal functions
NEKO_API_DECL struct neko_audio_s *__neko_audio_construct();

NEKO_API_DECL void neko_audio_shutdown(neko_audio_t *);
NEKO_API_DECL void neko_audio_destroy(neko_audio_t *);

#ifdef NEKO_CPP_SRC

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/neko_math.h"

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

class fmod_event;

class fmod_event_instance {
    fmod_event *type;

    void start();
    void stop();
};

// FMOD事件
struct fmod_event {
    FMOD::Studio::EventDescription *fmod_bank;
    std::string path;
    fmod_event_instance instance();
};

struct fmod_bank {
    FMOD::Studio::Bank *fmod_banks;  // Fmod bank 对象的实例

    std::unordered_map<std::string, fmod_event> event_map;

    fmod_bank(FMOD::Studio::Bank *fmod_bank);

    fmod_event *load_event(const char *path);  // 加载事件描述 如果已加载则进行缓存
};

// 用于存储bank的singleton
class fmod_bank_manager {
    std::unordered_map<std::string, fmod_bank> bank_map;
    fmod_bank_manager();

public:
    static fmod_bank_manager &instance();
    void unload(const char *bank_path);  // 卸载 bank
    fmod_bank *load(const char *path);   // 加载返回指向存储体的指针
};

// 初始化fmod系统
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

#pragma endregion FMODWrapper

struct fmod_impl {
    fmod_impl();
    ~fmod_impl();

    void update();

    int next_channel_id;

    typedef std::map<std::string, FMOD::Sound *> fmod_sound_map;
    typedef std::map<int, FMOD::Channel *> fmod_channel_map;
    typedef std::map<std::string, FMOD::Studio::EventInstance *> fmod_event_map;
    typedef std::map<std::string, FMOD::Studio::Bank *> fmod_bank_map;

    fmod_bank_map m_banks;
    fmod_event_map m_events;
    fmod_sound_map m_sounds;
    fmod_channel_map m_channels;
};

class fmod_audio {
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

#endif

#endif  // NEKO_AUDIO_H