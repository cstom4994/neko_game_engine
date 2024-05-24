#ifndef NEKO_AUDIO_H
#define NEKO_AUDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "engine/neko.h"

// -------------------------------------------------------------------------------------------------
// Error handling.

typedef enum neko_sound_error_t {
    NEKO_SOUND_ERROR_NONE,
    NEKO_SOUND_ERROR_IMPLEMENTATION_ERROR,
    NEKO_SOUND_ERROR_FILE_NOT_FOUND,
    NEKO_SOUND_ERROR_INVALID_SOUND,
    NEKO_SOUND_ERROR_HWND_IS_NULL,
    NEKO_SOUND_ERROR_DIRECTSOUND_CREATE_FAILED,
    NEKO_SOUND_ERROR_CREATESOUNDBUFFER_FAILED,
    NEKO_SOUND_ERROR_SETFORMAT_FAILED,
    NEKO_SOUND_ERROR_AUDIOCOMPONENTFINDNEXT_FAILED,
    NEKO_SOUND_ERROR_AUDIOCOMPONENTINSTANCENEW_FAILED,
    NEKO_SOUND_ERROR_FAILED_TO_SET_STREAM_FORMAT,
    NEKO_SOUND_ERROR_FAILED_TO_SET_RENDER_CALLBACK,
    NEKO_SOUND_ERROR_AUDIOUNITINITIALIZE_FAILED,
    NEKO_SOUND_ERROR_AUDIOUNITSTART_FAILED,
    NEKO_SOUND_ERROR_CANT_OPEN_AUDIO_DEVICE,
    NEKO_SOUND_ERROR_CANT_INIT_SDL_AUDIO,
    NEKO_SOUND_ERROR_THE_FILE_IS_NOT_A_WAV_FILE,
    NEKO_SOUND_ERROR_WAV_FILE_FORMAT_CHUNK_NOT_FOUND,
    NEKO_SOUND_ERROR_WAV_DATA_CHUNK_NOT_FOUND,
    NEKO_SOUND_ERROR_ONLY_PCM_WAV_FILES_ARE_SUPPORTED,
    NEKO_SOUND_ERROR_WAV_ONLY_MONO_OR_STEREO_IS_SUPPORTED,
    NEKO_SOUND_ERROR_WAV_ONLY_16_BITS_PER_SAMPLE_SUPPORTED,
    NEKO_SOUND_ERROR_CANNOT_SWITCH_MUSIC_WHILE_PAUSED,
    NEKO_SOUND_ERROR_CANNOT_CROSSFADE_WHILE_MUSIC_IS_PAUSED,
    NEKO_SOUND_ERROR_CANNOT_FADEOUT_WHILE_MUSIC_IS_PAUSED,
    NEKO_SOUND_ERROR_TRIED_TO_SET_SAMPLE_INDEX_BEYOND_THE_AUDIO_SOURCES_SAMPLE_COUNT,
    NEKO_SOUND_ERROR_STB_VORBIS_DECODE_FAILED,
    NEKO_SOUND_ERROR_OGG_UNSUPPORTED_CHANNEL_COUNT,
} neko_sound_error_t;

NEKO_API_DECL const char* neko_sound_error_as_string(neko_sound_error_t error);

// 为 `os_handle` 传入 NULL，除了 DirectSound 后端，这应该是 hwnd
// play_Frequency_in_Hz 取决于音频文件，44100 似乎没问题
// buffered_samples 被限制为至少 1024
NEKO_API_DECL neko_sound_error_t neko_sound_init(void* os_handle, unsigned play_frequency_in_Hz, int buffered_samples);
NEKO_API_DECL void neko_sound_shutdown();

// 每个游戏周期调用此函数一次
NEKO_API_DECL void neko_sound_update(float dt);
NEKO_API_DECL void neko_sound_set_global_volume(float volume_0_to_1);
NEKO_API_DECL void neko_sound_set_global_pan(float pan_0_to_1);
NEKO_API_DECL void neko_sound_set_global_pause(bool true_for_paused);

// 生成一个混合线程 专门用于在后台混合音频 如果你不调用这个函数 当调用`neko_sound_update`时 混合将会在主线程上发生
NEKO_API_DECL void neko_sound_spawn_mix_thread();

// f如果混合线程占用额外的 CPU 注意力而不执行任何操作 您可以强制手动休眠 可以根据需要进行调整 不是必需的
NEKO_API_DECL void neko_sound_mix_thread_sleep_delay(int milliseconds);

// 对于动态库

NEKO_API_DECL void* neko_sound_get_context_ptr();
NEKO_API_DECL void neko_sound_set_context_ptr(void* ctx);

// -------------------------------------------------------------------------------------------------
// 加载声音

typedef struct neko_sound_audio_source_t neko_sound_audio_source_t;

NEKO_API_DECL neko_sound_audio_source_t* neko_sound_load_wav(const char* path, neko_sound_error_t* err /* = NULL */);
NEKO_API_DECL neko_sound_audio_source_t* neko_sound_read_mem_wav(const void* memory, size_t size, neko_sound_error_t* err /* = NULL */);
NEKO_API_DECL void neko_sound_free_audio_source(neko_sound_audio_source_t* audio);

#ifdef STB_VORBIS_INCLUDE_STB_VORBIS_H
neko_sound_audio_source_t* neko_sound_load_ogg(const char* path, neko_sound_error_t* err /* = NULL */);
neko_sound_audio_source_t* neko_sound_read_mem_ogg(const void* memory, size_t size, neko_sound_error_t* err /* = NULL */);
#endif

// -------------------------------------------------------------------------------------------------
// Music sounds.

void neko_sound_music_play(neko_sound_audio_source_t* audio, float fade_in_time /* = 0 */);
void neko_sound_music_stop(float fade_out_time /* = 0 */);
void neko_sound_music_pause();
void neko_sound_music_resume();
void neko_sound_music_set_volume(float volume_0_to_1);
void neko_sound_music_set_loop(bool true_to_loop);
void neko_sound_music_switch_to(neko_sound_audio_source_t* audio, float fade_out_time /* = 0 */, float fade_in_time /* = 0 */);
void neko_sound_music_crossfade(neko_sound_audio_source_t* audio, float cross_fade_time /* = 0 */);
uint64_t neko_sound_music_get_sample_index();
neko_sound_error_t neko_sound_music_set_sample_index(uint64_t sample_index);

// -------------------------------------------------------------------------------------------------
// Playing sounds.

typedef struct neko_sound_playing_sound_t {
    uint64_t id;
} neko_sound_playing_sound_t;

typedef struct neko_sound_params_t {
    bool paused /* = false */;
    bool looped /* = false */;
    float volume /* = 1.0f */;
    float pan /* = 0.5f */;  // Can be from 0 to 1.
    float delay /* = 0 */;
} neko_sound_params_t;

NEKO_API_DECL neko_sound_params_t neko_sound_params_default();

NEKO_API_DECL neko_sound_playing_sound_t neko_sound_play_sound(neko_sound_audio_source_t* audio, neko_sound_params_t params);

NEKO_API_DECL bool neko_sound_is_active(neko_sound_playing_sound_t sound);
NEKO_API_DECL bool neko_sound_get_is_paused(neko_sound_playing_sound_t sound);
NEKO_API_DECL bool neko_sound_get_is_looped(neko_sound_playing_sound_t sound);
NEKO_API_DECL float neko_sound_get_volume(neko_sound_playing_sound_t sound);
NEKO_API_DECL uint64_t neko_sound_get_sample_index(neko_sound_playing_sound_t sound);
NEKO_API_DECL void neko_sound_set_is_paused(neko_sound_playing_sound_t sound, bool true_for_paused);
NEKO_API_DECL void neko_sound_set_is_looped(neko_sound_playing_sound_t sound, bool true_for_looped);
NEKO_API_DECL void neko_sound_set_volume(neko_sound_playing_sound_t sound, float volume_0_to_1);
NEKO_API_DECL neko_sound_error_t neko_sound_set_sample_index(neko_sound_playing_sound_t sound, uint64_t sample_index);

NEKO_API_DECL void neko_sound_set_playing_sounds_volume(float volume_0_to_1);
NEKO_API_DECL void neko_sound_stop_all_playing_sounds();

// -------------------------------------------------------------------------------------------------
// Global context.

NEKO_API_DECL void* neko_sound_get_global_context();
NEKO_API_DECL void neko_sound_set_global_context(void* context);

/*=====================
// Internal Audio Data
=====================*/

typedef struct neko_audio_data_t {

    // Any internal data required for audio API
    void* internal;

} neko_audio_data_t;

typedef struct neko_audio_s {
    /*============================================================
    // Audio Initilization / De-Initialization
    ============================================================*/

    neko_result (*init)(struct neko_audio_s*);
    // neko_result (*shutdown)(struct neko_audio_s*);
    neko_result (*update)(struct neko_audio_s*);
    neko_result (*commit)(struct neko_audio_s*);

    /*============================================================
    // Audio Instance Data
    ============================================================*/

    // Proably
    f32 max_audio_volume;
    f32 min_audio_volume;

    // All internal API specific data for audio system
    void* data;

    // Any custom user data (for custom API implementations)
    void* user_data;
} neko_audio_t;

// Extern internal functions
NEKO_API_DECL neko_audio_t* __neko_audio_construct();
NEKO_API_DECL void neko_audio_shutdown(neko_audio_t*);
NEKO_API_DECL void neko_audio_destroy(neko_audio_t*);

#endif  // NEKO_AUDIO_H