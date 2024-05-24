
#include "neko_audio.h"

#ifndef NEKO_SOUND_IMPLEMENTATION
#define NEKO_SOUND_IMPLEMENTATION

#ifndef NEKO_SOUND_MINIMUM_BUFFERED_SAMPLES
#define NEKO_SOUND_MINIMUM_BUFFERED_SAMPLES 1024
#endif

// #define NEKO_SOUND_ALLOC(size) neko_safe_malloc(size)
// #define NEKO_SOUND_FREE(mem) neko_safe_free(mem)
#define NEKO_SOUND_ALLOC(size) malloc(size)
#define NEKO_SOUND_FREE(mem) free(mem)

// Platform detection.
#define NEKO_SOUND_WINDOWS 1
#define NEKO_SOUND_APPLE 2
#define NEKO_SOUND_SDL 3

// Use NEKO_SOUND_FORCE_SDL as a way to force NEKO_SOUND_PLATFORM_SDL.
#ifdef NEKO_SOUND_FORCE_SDL
#define NEKO_SOUND_PLATFORM_SDL
#endif

#ifndef NEKO_SOUND_PLATFORM
// Check the specific platform defines.
#ifdef NEKO_PLATFORM_WIN
#define NEKO_SOUND_PLATFORM NEKO_SOUND_WINDOWS
#elif defined(NEKO_PLATFORM_APPLE)
#define NEKO_SOUND_PLATFORM NEKO_SOUND_APPLE
#elif defined(NEKO_PLATFORM_SDL)
#define NEKO_SOUND_PLATFORM NEKO_SOUND_SDL
#else
// Detect the platform automatically.
#if defined(_WIN32)
#if !defined _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#if !defined _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#define NEKO_SOUND_PLATFORM NEKO_SOUND_WINDOWS
#elif defined(__APPLE__)
#define NEKO_SOUND_PLATFORM NEKO_SOUND_APPLE
#else
// Just use SDL on other esoteric platforms.
#define NEKO_SOUND_PLATFORM NEKO_SOUND_SDL
#endif
#endif
#endif

// Platform specific file inclusions.
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifndef _WAVEFORMATEX_
#include <mmreg.h>
#include <mmsystem.h>
#endif

#include <dsound.h>
#undef PlaySound

#ifdef _MSC_VER
#pragma comment(lib, "dsound.lib")
#endif

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <mach/mach_time.h>
#include <pthread.h>

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

#ifndef SDL_h_
// Define NEKO_SOUND_SDL_H to allow changing the SDL.h path.
#ifndef NEKO_SOUND_SDL_H
#define NEKO_SOUND_SDL_H <SDL.h>
#endif
#include NEKO_SOUND_SDL_H
#endif
#ifndef _WIN32
#include <alloca.h>
#endif

#else

#error Unsupported platform - please choose one of NEKO_SOUND_WINDOWS, NEKO_SOUND_APPLE, NEKO_SOUND_SDL.

#endif

#ifdef NEKO_SOUND_SCALAR_MODE

#include <limits.h>

#define NEKO_SOUND_SATURATE16(X) (int16_t)((X) > SHRT_MAX ? SHRT_MAX : ((X) < SHRT_MIN ? SHRT_MIN : (X)))

typedef struct __neko_sound_m128 {
    float a, b, c, d;
} __neko_sound_m128;

typedef struct __neko_sound_m128i {
    int32_t a, b, c, d;
} __neko_sound_m128i;

__neko_sound_m128 neko_sound_mm_set_ps(float e3, float e2, float e1, float e0) {
    __neko_sound_m128 a;
    a.a = e0;
    a.b = e1;
    a.c = e2;
    a.d = e3;
    return a;
}

__neko_sound_m128 neko_sound_mm_set1_ps(float e) {
    __neko_sound_m128 a;
    a.a = e;
    a.b = e;
    a.c = e;
    a.d = e;
    return a;
}

__neko_sound_m128 neko_sound_mm_load_ps(float const* mem_addr) {
    __neko_sound_m128 a;
    a.a = mem_addr[0];
    a.b = mem_addr[1];
    a.c = mem_addr[2];
    a.d = mem_addr[3];
    return a;
}

__neko_sound_m128 neko_sound_mm_add_ps(__neko_sound_m128 a, __neko_sound_m128 b) {
    __neko_sound_m128 c;
    c.a = a.a + b.a;
    c.b = a.b + b.b;
    c.c = a.c + b.c;
    c.d = a.d + b.d;
    return c;
}

__neko_sound_m128 neko_sound_mm_mul_ps(__neko_sound_m128 a, __neko_sound_m128 b) {
    __neko_sound_m128 c;
    c.a = a.a * b.a;
    c.b = a.b * b.b;
    c.c = a.c * b.c;
    c.d = a.d * b.d;
    return c;
}

__neko_sound_m128i neko_sound_mm_cvtps_epi32(__neko_sound_m128 a) {
    __neko_sound_m128i b;
    b.a = a.a;
    b.b = a.b;
    b.c = a.c;
    b.d = a.d;
    return b;
}

__neko_sound_m128i neko_sound_mm_unpacklo_epi32(__neko_sound_m128i a, __neko_sound_m128i b) {
    __neko_sound_m128i c;
    c.a = a.a;
    c.b = b.a;
    c.c = a.b;
    c.d = b.b;
    return c;
}

__neko_sound_m128i neko_sound_mm_unpackhi_epi32(__neko_sound_m128i a, __neko_sound_m128i b) {
    __neko_sound_m128i c;
    c.a = a.c;
    c.b = b.c;
    c.c = a.d;
    c.d = b.d;
    return c;
}

__neko_sound_m128i neko_sound_mm_packs_epi32(__neko_sound_m128i a, __neko_sound_m128i b) {
    union {
        int16_t c[8];
        __neko_sound_m128i m;
    } dst;
    dst.c[0] = NEKO_SOUND_SATURATE16(a.a);
    dst.c[1] = NEKO_SOUND_SATURATE16(a.b);
    dst.c[2] = NEKO_SOUND_SATURATE16(a.c);
    dst.c[3] = NEKO_SOUND_SATURATE16(a.d);
    dst.c[4] = NEKO_SOUND_SATURATE16(b.a);
    dst.c[5] = NEKO_SOUND_SATURATE16(b.b);
    dst.c[6] = NEKO_SOUND_SATURATE16(b.c);
    dst.c[7] = NEKO_SOUND_SATURATE16(b.d);
    return dst.m;
}

#else  // NEKO_SOUND_SCALAR_MODE

#include <emmintrin.h>
#include <xmmintrin.h>

#define __neko_sound_m128 __m128
#define __neko_sound_m128i __m128i

#define neko_sound_mm_set_ps _mm_set_ps
#define neko_sound_mm_set1_ps _mm_set1_ps
#define neko_sound_mm_load_ps _mm_load_ps
#define neko_sound_mm_add_ps _mm_add_ps
#define neko_sound_mm_mul_ps _mm_mul_ps
#define neko_sound_mm_cvtps_epi32 _mm_cvtps_epi32
#define neko_sound_mm_unpacklo_epi32 _mm_unpacklo_epi32
#define neko_sound_mm_unpackhi_epi32 _mm_unpackhi_epi32
#define neko_sound_mm_packs_epi32 _mm_packs_epi32

#endif  // NEKO_SOUND_SCALAR_MODE

#define NEKO_SOUND_ALIGN(X, Y) ((((size_t)X) + ((Y) - 1)) & ~((Y) - 1))
#define NEKO_SOUND_TRUNC(X, Y) ((size_t)(X) & ~((Y) - 1))

// -------------------------------------------------------------------------------------------------
// Doubly list.

typedef struct neko_sound_list_node_t {
    struct neko_sound_list_node_t* next /* = this */;
    struct neko_sound_list_node_t* prev /* = this */;
} neko_sound_list_node_t;

typedef struct neko_sound_list_t {
    neko_sound_list_node_t nodes;
} neko_sound_list_t;

#define NEKO_SOUND_OFFSET_OF(T, member) ((size_t)((uintptr_t)(&(((T*)0)->member))))
#define NEKO_SOUND_LIST_NODE(T, member, ptr) ((neko_sound_list_node_t*)((uintptr_t)ptr + NEKO_SOUND_OFFSET_OF(T, member)))
#define NEKO_SOUND_LIST_HOST(T, member, ptr) ((T*)((uintptr_t)ptr - NEKO_SOUND_OFFSET_OF(T, member)))

void neko_sound_list_init_node(neko_sound_list_node_t* node) {
    node->next = node;
    node->prev = node;
}

void neko_sound_list_init(neko_sound_list_t* list) { neko_sound_list_init_node(&list->nodes); }

void neko_sound_list_push_front(neko_sound_list_t* list, neko_sound_list_node_t* node) {
    node->next = list->nodes.next;
    node->prev = &list->nodes;
    list->nodes.next->prev = node;
    list->nodes.next = node;
}

void neko_sound_list_push_back(neko_sound_list_t* list, neko_sound_list_node_t* node) {
    node->prev = list->nodes.prev;
    node->next = &list->nodes;
    list->nodes.prev->next = node;
    list->nodes.prev = node;
}

void neko_sound_list_remove(neko_sound_list_node_t* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    neko_sound_list_init_node(node);
}

neko_sound_list_node_t* neko_sound_list_pop_front(neko_sound_list_t* list) {
    neko_sound_list_node_t* node = list->nodes.next;
    neko_sound_list_remove(node);
    return node;
}

neko_sound_list_node_t* neko_sound_list_pop_back(neko_sound_list_t* list) {
    neko_sound_list_node_t* node = list->nodes.prev;
    neko_sound_list_remove(node);
    return node;
}

int neko_sound_list_empty(neko_sound_list_t* list) { return list->nodes.next == list->nodes.prev && list->nodes.next == &list->nodes; }

neko_sound_list_node_t* neko_sound_list_begin(neko_sound_list_t* list) { return list->nodes.next; }

neko_sound_list_node_t* neko_sound_list_end(neko_sound_list_t* list) { return &list->nodes; }

neko_sound_list_node_t* neko_sound_list_front(neko_sound_list_t* list) { return list->nodes.next; }

neko_sound_list_node_t* neko_sound_list_back(neko_sound_list_t* list) { return list->nodes.prev; }

// -------------------------------------------------------------------------------------------------

const char* neko_sound_error_as_string(neko_sound_error_t error) {
    switch (error) {
        case NEKO_SOUND_ERROR_NONE:
            return "NEKO_SOUND_ERROR_NONE";
        case NEKO_SOUND_ERROR_IMPLEMENTATION_ERROR:
            return "NEKO_SOUND_ERROR_IMPLEMENTATION_ERROR";
        case NEKO_SOUND_ERROR_FILE_NOT_FOUND:
            return "NEKO_SOUND_ERROR_FILE_NOT_FOUND";
        case NEKO_SOUND_ERROR_INVALID_SOUND:
            return "NEKO_SOUND_ERROR_INVALID_SOUND";
        case NEKO_SOUND_ERROR_HWND_IS_NULL:
            return "NEKO_SOUND_ERROR_HWND_IS_NULL";
        case NEKO_SOUND_ERROR_DIRECTSOUND_CREATE_FAILED:
            return "NEKO_SOUND_ERROR_DIRECTSOUND_CREATE_FAILED";
        case NEKO_SOUND_ERROR_CREATESOUNDBUFFER_FAILED:
            return "NEKO_SOUND_ERROR_CREATESOUNDBUFFER_FAILED";
        case NEKO_SOUND_ERROR_SETFORMAT_FAILED:
            return "NEKO_SOUND_ERROR_SETFORMAT_FAILED";
        case NEKO_SOUND_ERROR_AUDIOCOMPONENTFINDNEXT_FAILED:
            return "NEKO_SOUND_ERROR_AUDIOCOMPONENTFINDNEXT_FAILED";
        case NEKO_SOUND_ERROR_AUDIOCOMPONENTINSTANCENEW_FAILED:
            return "NEKO_SOUND_ERROR_AUDIOCOMPONENTINSTANCENEW_FAILED";
        case NEKO_SOUND_ERROR_FAILED_TO_SET_STREAM_FORMAT:
            return "NEKO_SOUND_ERROR_FAILED_TO_SET_STREAM_FORMAT";
        case NEKO_SOUND_ERROR_FAILED_TO_SET_RENDER_CALLBACK:
            return "NEKO_SOUND_ERROR_FAILED_TO_SET_RENDER_CALLBACK";
        case NEKO_SOUND_ERROR_AUDIOUNITINITIALIZE_FAILED:
            return "NEKO_SOUND_ERROR_AUDIOUNITINITIALIZE_FAILED";
        case NEKO_SOUND_ERROR_AUDIOUNITSTART_FAILED:
            return "NEKO_SOUND_ERROR_AUDIOUNITSTART_FAILED";
        case NEKO_SOUND_ERROR_CANT_OPEN_AUDIO_DEVICE:
            return "NEKO_SOUND_ERROR_CANT_OPEN_AUDIO_DEVICE";
        case NEKO_SOUND_ERROR_CANT_INIT_SDL_AUDIO:
            return "NEKO_SOUND_ERROR_CANT_INIT_SDL_AUDIO";
        case NEKO_SOUND_ERROR_THE_FILE_IS_NOT_A_WAV_FILE:
            return "NEKO_SOUND_ERROR_THE_FILE_IS_NOT_A_WAV_FILE";
        case NEKO_SOUND_ERROR_WAV_FILE_FORMAT_CHUNK_NOT_FOUND:
            return "NEKO_SOUND_ERROR_WAV_FILE_FORMAT_CHUNK_NOT_FOUND";
        case NEKO_SOUND_ERROR_WAV_DATA_CHUNK_NOT_FOUND:
            return "NEKO_SOUND_ERROR_WAV_DATA_CHUNK_NOT_FOUND";
        case NEKO_SOUND_ERROR_ONLY_PCM_WAV_FILES_ARE_SUPPORTED:
            return "NEKO_SOUND_ERROR_ONLY_PCM_WAV_FILES_ARE_SUPPORTED";
        case NEKO_SOUND_ERROR_WAV_ONLY_MONO_OR_STEREO_IS_SUPPORTED:
            return "NEKO_SOUND_ERROR_WAV_ONLY_MONO_OR_STEREO_IS_SUPPORTED";
        case NEKO_SOUND_ERROR_WAV_ONLY_16_BITS_PER_SAMPLE_SUPPORTED:
            return "NEKO_SOUND_ERROR_WAV_ONLY_16_BITS_PER_SAMPLE_SUPPORTED";
        case NEKO_SOUND_ERROR_CANNOT_SWITCH_MUSIC_WHILE_PAUSED:
            return "NEKO_SOUND_ERROR_CANNOT_SWITCH_MUSIC_WHILE_PAUSED";
        case NEKO_SOUND_ERROR_CANNOT_CROSSFADE_WHILE_MUSIC_IS_PAUSED:
            return "NEKO_SOUND_ERROR_CANNOT_CROSSFADE_WHILE_MUSIC_IS_PAUSED";
        case NEKO_SOUND_ERROR_CANNOT_FADEOUT_WHILE_MUSIC_IS_PAUSED:
            return "NEKO_SOUND_ERROR_CANNOT_FADEOUT_WHILE_MUSIC_IS_PAUSED";
        case NEKO_SOUND_ERROR_TRIED_TO_SET_SAMPLE_INDEX_BEYOND_THE_AUDIO_SOURCES_SAMPLE_COUNT:
            return "NEKO_SOUND_ERROR_TRIED_TO_SET_SAMPLE_INDEX_BEYOND_THE_AUDIO_SOURCES_SAMPLE_COUNT";
        case NEKO_SOUND_ERROR_STB_VORBIS_DECODE_FAILED:
            return "NEKO_SOUND_ERROR_STB_VORBIS_DECODE_FAILED";
        case NEKO_SOUND_ERROR_OGG_UNSUPPORTED_CHANNEL_COUNT:
            return "NEKO_SOUND_ERROR_OGG_UNSUPPORTED_CHANNEL_COUN";
        default:
            return "UNKNOWN";
    }
}

// Cute sound context functions.

void neko_sound_mix();

typedef struct neko_sound_audio_source_t {
    int sample_rate;
    int sample_count;
    int channel_count;

    // Number of instances currently referencing this audio. Must be zero
    // in order to safely delete the audio. References are automatically
    // updated whenever playing instances are inserted into the context.
    int playing_count;

    // The actual raw audio samples in memory.
    void* channels[2];
} neko_sound_audio_source_t;

typedef struct neko_sound_inst_t {
    uint64_t id;
    bool is_music;
    bool active;
    bool paused;
    bool looped;
    float volume;
    float pan0;
    float pan1;
    uint64_t sample_index;
    neko_sound_audio_source_t* audio;
    neko_sound_list_node_t node;
} neko_sound_inst_t;

typedef enum neko_sound_music_state_t {
    NEKO_SOUND_MUSIC_STATE_NONE,
    NEKO_SOUND_MUSIC_STATE_PLAYING,
    NEKO_SOUND_MUSIC_STATE_FADE_OUT,
    NEKO_SOUND_MUSIC_STATE_FADE_IN,
    NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0,
    NEKO_SOUND_MUSIC_STATE_SWITCH_TO_1,
    NEKO_SOUND_MUSIC_STATE_CROSSFADE,
    NEKO_SOUND_MUSIC_STATE_PAUSED
} neko_sound_music_state_t;

#define NEKO_SOUND_PAGE_INSTANCE_COUNT 1024

typedef struct neko_sound_inst_page_t {
    struct neko_sound_inst_page_t* next;
    neko_sound_inst_t instances[NEKO_SOUND_PAGE_INSTANCE_COUNT];
} neko_sound_inst_page_t;

typedef struct neko_sound_context_t {
    float global_pan /* = 0.5f */;
    float global_volume /* = 1.0f */;
    bool global_pause /* = false */;
    float music_volume /* = 1.0f */;
    float sound_volume /* = 1.0f */;

    bool music_paused /* = false */;
    bool music_looped /* = true */;
    float t /* = 0 */;
    float fade /* = 0 */;
    float fade_switch_1 /* = 0 */;
    neko_sound_music_state_t music_state /* = MUSIC_STATE_NONE */;
    neko_sound_music_state_t music_state_to_resume_from_paused /* = MUSIC_STATE_NONE */;
    neko_sound_inst_t* music_playing /* = NULL */;
    neko_sound_inst_t* music_next /* = NULL */;

    int audio_sources_to_free_capacity /* = 0 */;
    int audio_sources_to_free_size /* = 0 */;
    neko_sound_audio_source_t** audio_sources_to_free /* = NULL */;
    uint64_t instance_id_gen /* = 1 */;
    hashtable_t instance_map;  // <uint64_t, neko_sound_audio_source_t*>
    neko_sound_inst_page_t* pages /* = NULL */;
    neko_sound_list_t playing_sounds;
    neko_sound_list_t free_sounds;

    unsigned latency_samples;
    int Hz;
    int bps;
    int wide_count;
    __neko_sound_m128* floatA;
    __neko_sound_m128* floatB;
    __neko_sound_m128i* samples;
    bool separate_thread;
    bool running;
    int sleep_milliseconds;

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

    DWORD last_cursor;
    unsigned running_index;
    int buffer_size;
    LPDIRECTSOUND dsound;
    LPDIRECTSOUNDBUFFER primary;
    LPDIRECTSOUNDBUFFER secondary;

    // data for neko_sound_mix thread, enable these with neko_sound_spawn_mix_thread
    CRITICAL_SECTION critical_section;

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE

    unsigned index0;  // read
    unsigned index1;  // write
    unsigned samples_in_circular_buffer;
    int sample_count;

    // platform specific stuff
    AudioComponentInstance inst;

    // data for neko_sound_mix thread, enable these with neko_sound_spawn_mix_thread
    pthread_t thread;
    pthread_mutex_t mutex;

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

    unsigned index0;  // read
    unsigned index1;  // write
    unsigned samples_in_circular_buffer;
    int sample_count;
    SDL_AudioDeviceID dev;

    // data for neko_sound_mix thread, enable these with neko_sound_spawn_mix_thread
    SDL_Thread* thread;
    SDL_mutex* mutex;

#endif
} neko_sound_context_t;

neko_sound_context_t* s_ctx = NULL;

void neko_sound_sleep(int milliseconds) {
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS
    Sleep(milliseconds);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE
    struct timespec ts = {0, milliseconds * 1000000};
    nanosleep(&ts, NULL);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL
    SDL_Delay(milliseconds);
#endif
}

static void* neko_sound_malloc16(size_t size) {
    void* p = NEKO_SOUND_ALLOC(size + 16);
    if (!p) return 0;
    unsigned char offset = (size_t)p & 15;
    p = (void*)NEKO_SOUND_ALIGN(p + 1, 16);
    *((char*)p - 1) = 16 - offset;
    neko_assert(!((size_t)p & 15));
    return p;
}

static void neko_sound_free16(void* p) {
    if (!p) return;
    NEKO_SOUND_FREE((char*)p - (((size_t)(*((char*)p - 1))) & 0xFF));
}

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL || NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE

static int neko_sound_samples_written() { return s_ctx->samples_in_circular_buffer; }

static int neko_sound_samples_unwritten() { return s_ctx->sample_count - s_ctx->samples_in_circular_buffer; }

static int neko_sound_samples_to_mix() {
    int lat = s_ctx->latency_samples;
    int written = neko_sound_samples_written();
    int dif = lat - written;
    neko_assert(dif >= 0);
    if (dif) {
        int unwritten = neko_sound_samples_unwritten();
        return dif < unwritten ? dif : unwritten;
    }
    return 0;
}

#define NEKO_SOUND_SAMPLES_TO_BYTES(interleaved_sample_count) ((interleaved_sample_count) * s_ctx->bps)
#define NEKO_SOUND_BYTES_TO_SAMPLES(byte_count) ((byte_count) / s_ctx->bps)

static void neko_sound_push_bytes(void* data, int size) {
    int index1 = s_ctx->index1;
    int samples_to_write = NEKO_SOUND_BYTES_TO_SAMPLES(size);
    int sample_count = s_ctx->sample_count;

    int unwritten = neko_sound_samples_unwritten();
    if (unwritten < samples_to_write) samples_to_write = unwritten;
    int samples_to_end = sample_count - index1;

    if (samples_to_write > samples_to_end) {
        memcpy((char*)s_ctx->samples + NEKO_SOUND_SAMPLES_TO_BYTES(index1), data, NEKO_SOUND_SAMPLES_TO_BYTES(samples_to_end));
        memcpy(s_ctx->samples, (char*)data + NEKO_SOUND_SAMPLES_TO_BYTES(samples_to_end), size - NEKO_SOUND_SAMPLES_TO_BYTES(samples_to_end));
        s_ctx->index1 = (samples_to_write - samples_to_end) % sample_count;
    } else {
        memcpy((char*)s_ctx->samples + NEKO_SOUND_SAMPLES_TO_BYTES(index1), data, size);
        s_ctx->index1 = (s_ctx->index1 + samples_to_write) % sample_count;
    }

    s_ctx->samples_in_circular_buffer += samples_to_write;
}

static int neko_sound_pull_bytes(void* dst, int size) {
    int index0 = s_ctx->index0;
    int allowed_size = NEKO_SOUND_SAMPLES_TO_BYTES(neko_sound_samples_written());
    int sample_count = s_ctx->sample_count;
    int zeros = 0;

    if (allowed_size < size) {
        zeros = size - allowed_size;
        size = allowed_size;
    }

    int samples_to_read = NEKO_SOUND_BYTES_TO_SAMPLES(size);
    int samples_to_end = sample_count - index0;

    if (samples_to_read > samples_to_end) {
        memcpy(dst, ((char*)s_ctx->samples) + NEKO_SOUND_SAMPLES_TO_BYTES(index0), NEKO_SOUND_SAMPLES_TO_BYTES(samples_to_end));
        memcpy(((char*)dst) + NEKO_SOUND_SAMPLES_TO_BYTES(samples_to_end), s_ctx->samples, size - NEKO_SOUND_SAMPLES_TO_BYTES(samples_to_end));
        s_ctx->index0 = (samples_to_read - samples_to_end) % sample_count;
    } else {
        memcpy(dst, ((char*)s_ctx->samples) + NEKO_SOUND_SAMPLES_TO_BYTES(index0), size);
        s_ctx->index0 = (s_ctx->index0 + samples_to_read) % sample_count;
    }

    s_ctx->samples_in_circular_buffer -= samples_to_read;

    return zeros;
}

#endif

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

static DWORD WINAPI neko_sound_ctx_thread(LPVOID lpParameter) {
    (void)lpParameter;
    while (s_ctx->running) {
        neko_sound_mix();
        if (s_ctx->sleep_milliseconds)
            neko_sound_sleep(s_ctx->sleep_milliseconds);
        else
            YieldProcessor();
    }

    s_ctx->separate_thread = false;
    return 0;
}

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE

static void* neko_sound_ctx_thread(void* udata) {
    while (s_ctx->running) {
        neko_sound_mix();
        if (s_ctx->sleep_milliseconds)
            neko_sound_sleep(s_ctx->sleep_milliseconds);
        else
            pthread_yield_np();
    }

    s_ctx->separate_thread = 0;
    pthread_exit(0);
    return 0;
}

static OSStatus neko_sound_memcpy_to_coreaudio(void* udata, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames,
                                               AudioBufferList* ioData) {
    int bps = s_ctx->bps;
    int samples_requested_to_consume = inNumberFrames;
    AudioBuffer* buffer = ioData->mBuffers;

    neko_assert(ioData->mNumberBuffers == 1);
    neko_assert(buffer->mNumberChannels == 2);
    int byte_size = buffer->mDataByteSize;
    neko_assert(byte_size == samples_requested_to_consume * bps);

    int zero_bytes = neko_sound_pull_bytes(buffer->mData, byte_size);
    memset(((char*)buffer->mData) + (byte_size - zero_bytes), 0, zero_bytes);

    return noErr;
}

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

int neko_sound_ctx_thread(void* udata) {
    while (s_ctx->running) {
        neko_sound_mix();
        if (s_ctx->sleep_milliseconds)
            neko_sound_sleep(s_ctx->sleep_milliseconds);
        else
            neko_sound_sleep(1);
    }

    s_ctx->separate_thread = false;
    return 0;
}

static void neko_sound_sdl_audio_callback(void* udata, Uint8* stream, int len) {
    int zero_bytes = neko_sound_pull_bytes(stream, len);
    memset(stream + (len - zero_bytes), 0, zero_bytes);
}

#endif

static void s_add_page() {
    neko_sound_inst_page_t* page = (neko_sound_inst_page_t*)NEKO_SOUND_ALLOC(sizeof(neko_sound_inst_page_t));
    for (int i = 0; i < NEKO_SOUND_PAGE_INSTANCE_COUNT; ++i) {
        neko_sound_list_init_node(&page->instances[i].node);
        neko_sound_list_push_back(&s_ctx->free_sounds, &page->instances[i].node);
    }
    page->next = s_ctx->pages;
    s_ctx->pages = page;
}

neko_sound_error_t neko_sound_init(void* os_handle, unsigned play_frequency_in_Hz, int buffered_samples) {
    buffered_samples = buffered_samples < NEKO_SOUND_MINIMUM_BUFFERED_SAMPLES ? NEKO_SOUND_MINIMUM_BUFFERED_SAMPLES : buffered_samples;
    int sample_count = buffered_samples;
    int wide_count = (int)NEKO_SOUND_ALIGN(sample_count, 4);
    int bps = sizeof(uint16_t) * 2;

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

    int buffer_size = buffered_samples * bps;
    LPDIRECTSOUND dsound = NULL;
    LPDIRECTSOUNDBUFFER primary_buffer = NULL;
    LPDIRECTSOUNDBUFFER secondary_buffer = NULL;

    if (!os_handle) return NEKO_SOUND_ERROR_HWND_IS_NULL;
    {
        WAVEFORMATEX format = {0, 0, 0, 0, 0, 0, 0};
        DSBUFFERDESC bufdesc = {0, 0, 0, 0, 0, {0, 0, 0, 0}};
        HRESULT res = DirectSoundCreate(0, &dsound, 0);
        if (res != DS_OK) return NEKO_SOUND_ERROR_DIRECTSOUND_CREATE_FAILED;
        IDirectSound_SetCooperativeLevel(dsound, (HWND)os_handle, DSSCL_PRIORITY);
        bufdesc.dwSize = sizeof(bufdesc);
        bufdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

        res = IDirectSound_CreateSoundBuffer(dsound, &bufdesc, &primary_buffer, 0);
        if (res != DS_OK) NEKO_SOUND_ERROR_CREATESOUNDBUFFER_FAILED;

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 2;
        format.nSamplesPerSec = play_frequency_in_Hz;
        format.wBitsPerSample = 16;
        format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
        format.cbSize = 0;
        res = IDirectSoundBuffer_SetFormat(primary_buffer, &format);
        if (res != DS_OK) NEKO_SOUND_ERROR_SETFORMAT_FAILED;

        bufdesc.dwSize = sizeof(bufdesc);
        bufdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
        bufdesc.dwBufferBytes = buffer_size;
        bufdesc.lpwfxFormat = &format;
        res = IDirectSound_CreateSoundBuffer(dsound, &bufdesc, &secondary_buffer, 0);
        if (res != DS_OK) NEKO_SOUND_ERROR_SETFORMAT_FAILED;

        // Silence the initial audio buffer.
        void* region1;
        DWORD size1;
        void* region2;
        DWORD size2;
        res = IDirectSoundBuffer_Lock(secondary_buffer, 0, bufdesc.dwBufferBytes, &region1, &size1, &region2, &size2, DSBLOCK_ENTIREBUFFER);
        if (res == DS_OK) {
            memset(region1, 0, size1);
            IDirectSoundBuffer_Unlock(secondary_buffer, region1, size1, region2, size2);
        }
    }

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE

    AudioComponentDescription comp_desc = {0};
    comp_desc.componentType = kAudioUnitType_Output;
    comp_desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    comp_desc.componentFlags = 0;
    comp_desc.componentFlagsMask = 0;
    comp_desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent comp = AudioComponentFindNext(NULL, &comp_desc);
    if (!comp) return NEKO_SOUND_ERROR_AUDIOCOMPONENTFINDNEXT_FAILED;

    AudioStreamBasicDescription stream_desc = {0};
    stream_desc.mSampleRate = (double)play_frequency_in_Hz;
    stream_desc.mFormatID = kAudioFormatLinearPCM;
    stream_desc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
    stream_desc.mFramesPerPacket = 1;
    stream_desc.mChannelsPerFrame = 2;
    stream_desc.mBitsPerChannel = sizeof(uint16_t) * 8;
    stream_desc.mBytesPerPacket = bps;
    stream_desc.mBytesPerFrame = bps;
    stream_desc.mReserved = 0;

    AudioComponentInstance inst;
    OSStatus ret;
    AURenderCallbackStruct input;

    ret = AudioComponentInstanceNew(comp, &inst);
    if (ret != noErr) return NEKO_SOUND_ERROR_AUDIOCOMPONENTINSTANCENEW_FAILED;

    ret = AudioUnitSetProperty(inst, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &stream_desc, sizeof(stream_desc));
    if (ret != noErr) return NEKO_SOUND_ERROR_FAILED_TO_SET_STREAM_FORMAT;

    ret = AudioUnitInitialize(inst);
    if (ret != noErr) return NEKO_SOUND_ERROR_AUDIOUNITINITIALIZE_FAILED;

    ret = AudioOutputUnitStart(inst);
    if (ret != noErr) return NEKO_SOUND_ERROR_AUDIOUNITSTART_FAILED;

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

    SDL_AudioSpec wanted, have;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret < 0) return NEKO_SOUND_ERROR_CANT_INIT_SDL_AUDIO;

#endif

    s_ctx = (neko_sound_context_t*)NEKO_SOUND_ALLOC(sizeof(neko_sound_context_t));
    s_ctx->global_pan = 0.5f;
    s_ctx->global_volume = 1.0f;
    s_ctx->global_pause = false;
    s_ctx->music_volume = 1.0f;
    s_ctx->sound_volume = 1.0f;
    s_ctx->music_looped = true;
    s_ctx->music_paused = false;
    s_ctx->t = 0;
    s_ctx->fade = 0;
    s_ctx->fade_switch_1 = 0;
    s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_NONE;
    s_ctx->music_state_to_resume_from_paused = NEKO_SOUND_MUSIC_STATE_NONE;
    s_ctx->music_playing = NULL;
    s_ctx->music_next = NULL;
    s_ctx->audio_sources_to_free_capacity = 32;
    s_ctx->audio_sources_to_free_size = 0;
    s_ctx->audio_sources_to_free = (neko_sound_audio_source_t**)NEKO_SOUND_ALLOC(sizeof(neko_sound_audio_source_t*) * s_ctx->audio_sources_to_free_capacity);
    s_ctx->instance_id_gen = 1;
    hashtable_init(&s_ctx->instance_map, sizeof(neko_sound_audio_source_t*), 1024, NULL);
    s_ctx->pages = NULL;
    neko_sound_list_init(&s_ctx->playing_sounds);
    neko_sound_list_init(&s_ctx->free_sounds);
    s_add_page();
    s_ctx->latency_samples = buffered_samples;
    s_ctx->Hz = play_frequency_in_Hz;
    s_ctx->bps = bps;
    s_ctx->wide_count = wide_count;
    s_ctx->floatA = (__neko_sound_m128*)neko_sound_malloc16(sizeof(__neko_sound_m128) * wide_count);
    s_ctx->floatB = (__neko_sound_m128*)neko_sound_malloc16(sizeof(__neko_sound_m128) * wide_count);
    s_ctx->samples = (__neko_sound_m128i*)neko_sound_malloc16(sizeof(__neko_sound_m128i) * wide_count);
    s_ctx->running = true;
    s_ctx->separate_thread = false;
    s_ctx->sleep_milliseconds = 0;

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

    s_ctx->last_cursor = 0;
    s_ctx->running_index = 0;
    s_ctx->buffer_size = buffer_size;
    s_ctx->dsound = dsound;
    s_ctx->primary = primary_buffer;
    s_ctx->secondary = secondary_buffer;
    InitializeCriticalSectionAndSpinCount(&s_ctx->critical_section, 0x00000400);

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE

    s_ctx->index0 = 0;
    s_ctx->index1 = 0;
    s_ctx->samples_in_circular_buffer = 0;
    s_ctx->sample_count = wide_count * 4;
    s_ctx->inst = inst;
    pthread_mutex_init(&s_ctx->mutex, NULL);

    input.inputProc = neko_sound_memcpy_to_coreaudio;
    input.inputProcRefCon = s_ctx;
    ret = AudioUnitSetProperty(inst, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));
    if (ret != noErr) return NEKO_SOUND_ERROR_FAILED_TO_SET_RENDER_CALLBACK;  // This leaks memory, oh well.

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

    SDL_memset(&wanted, 0, sizeof(wanted));
    SDL_memset(&have, 0, sizeof(have));
    wanted.freq = play_frequency_in_Hz;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 2; /* 1 = mono, 2 = stereo */
    wanted.samples = buffered_samples;
    wanted.callback = neko_sound_sdl_audio_callback;
    wanted.userdata = s_ctx;
    s_ctx->index0 = 0;
    s_ctx->index1 = 0;
    s_ctx->samples_in_circular_buffer = 0;
    s_ctx->sample_count = wide_count * 4;
    s_ctx->dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &have, 0);
    if (s_ctx->dev < 0) return NEKO_SOUND_ERROR_CANT_OPEN_AUDIO_DEVICE;  // This leaks memory, oh well.
    SDL_PauseAudioDevice(s_ctx->dev, 0);
    s_ctx->mutex = SDL_CreateMutex();

#endif

    return NEKO_SOUND_ERROR_NONE;
}

void neko_sound_lock();
void neko_sound_unlock();

void neko_sound_shutdown() {
    if (!s_ctx) return;
    if (s_ctx->separate_thread) {
        neko_sound_lock();
        s_ctx->running = false;
        neko_sound_unlock();
        while (s_ctx->separate_thread) neko_sound_sleep(1);
    }
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

    DeleteCriticalSection(&s_ctx->critical_section);
    IDirectSoundBuffer_Release(s_ctx->secondary);
    IDirectSoundBuffer_Release(s_ctx->primary);
    IDirectSoundBuffer_Release(s_ctx->dsound);

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

    SDL_DestroyMutex(s_ctx->mutex);
    SDL_CloseAudioDevice(s_ctx->dev);

#endif

    if (!neko_sound_list_empty(&s_ctx->playing_sounds)) {
        neko_sound_list_node_t* playing_node = neko_sound_list_begin(&s_ctx->playing_sounds);
        neko_sound_list_node_t* end_node = neko_sound_list_end(&s_ctx->playing_sounds);
        do {
            neko_sound_list_node_t* next_node = playing_node->next;
            neko_sound_inst_t* playing = NEKO_SOUND_LIST_HOST(neko_sound_inst_t, node, playing_node);
            neko_sound_audio_source_t* audio = playing->audio;
            if (audio) audio->playing_count = 0;
            playing_node = next_node;
        } while (playing_node != end_node);
    }

    neko_sound_inst_page_t* page = s_ctx->pages;
    while (page) {
        neko_sound_inst_page_t* next = page->next;
        NEKO_SOUND_FREE(page);
        page = next;
    }

    for (int i = 0; i < s_ctx->audio_sources_to_free_size; ++i) {
        neko_sound_audio_source_t* audio = s_ctx->audio_sources_to_free[i];
        neko_sound_free16(audio->channels[0]);
        NEKO_SOUND_FREE(audio);
    }
    NEKO_SOUND_FREE(s_ctx->audio_sources_to_free);

    neko_sound_free16(s_ctx->floatA);
    neko_sound_free16(s_ctx->floatB);
    neko_sound_free16(s_ctx->samples);
    hashtable_term(&s_ctx->instance_map);
    NEKO_SOUND_FREE(s_ctx);
    s_ctx = NULL;
}

static float s_smoothstep(float x) { return x * x * (3.0f - 2.0f * x); }

void neko_sound_update(float dt) {
    if (!s_ctx->separate_thread) neko_sound_mix();

    switch (s_ctx->music_state) {
        case NEKO_SOUND_MUSIC_STATE_FADE_OUT: {
            s_ctx->t += dt;
            if (s_ctx->t >= s_ctx->fade) {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_NONE;
                s_ctx->music_playing->active = false;
                s_ctx->music_playing = NULL;
            } else {
                s_ctx->music_playing->volume = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                ;
            }
        } break;

        case NEKO_SOUND_MUSIC_STATE_FADE_IN: {
            s_ctx->t += dt;
            if (s_ctx->t >= s_ctx->fade) {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_PLAYING;
                s_ctx->t = s_ctx->fade;
            }
            s_ctx->music_playing->volume = s_smoothstep(1.0f - ((s_ctx->fade - s_ctx->t) / s_ctx->fade));
        } break;

        case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0: {
            s_ctx->t += dt;
            if (s_ctx->t >= s_ctx->fade) {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_SWITCH_TO_1;
                s_ctx->music_playing->active = false;
                s_ctx->music_playing->volume = 0;
                s_ctx->t = 0;
                s_ctx->fade = s_ctx->fade_switch_1;
                s_ctx->fade_switch_1 = 0;
                s_ctx->music_next->paused = false;
            } else {
                s_ctx->music_playing->volume = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                ;
            }
        } break;

        case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_1: {
            s_ctx->t += dt;
            if (s_ctx->t >= s_ctx->fade) {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_PLAYING;
                s_ctx->t = s_ctx->fade;
                s_ctx->music_next->volume = 1.0f;
                s_ctx->music_playing = s_ctx->music_next;
                s_ctx->music_next = NULL;
            } else {
                float t = s_smoothstep(1.0f - ((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                float volume = t;
                s_ctx->music_next->volume = volume;
            }
        } break;

        case NEKO_SOUND_MUSIC_STATE_CROSSFADE: {
            s_ctx->t += dt;
            if (s_ctx->t >= s_ctx->fade) {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_PLAYING;
                s_ctx->music_playing->active = false;
                s_ctx->music_next->volume = 1.0f;
                s_ctx->music_playing = s_ctx->music_next;
                s_ctx->music_next = NULL;
            } else {
                float t0 = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                float t1 = s_smoothstep(1.0f - ((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                float v0 = t0;
                float v1 = t1;
                s_ctx->music_playing->volume = v0;
                s_ctx->music_next->volume = v1;
            }
        } break;

        default:
            break;
    }
}

void neko_sound_set_global_volume(float volume_0_to_1) {
    if (volume_0_to_1 < 0) volume_0_to_1 = 0;
    s_ctx->global_volume = volume_0_to_1;
}

void neko_sound_set_global_pan(float pan_0_to_1) {
    if (pan_0_to_1 < 0) pan_0_to_1 = 0;
    if (pan_0_to_1 > 1) pan_0_to_1 = 1;
    s_ctx->global_pan = pan_0_to_1;
}

void neko_sound_set_global_pause(bool true_for_paused) { s_ctx->global_pause = true_for_paused; }

void neko_sound_lock() {
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS
    EnterCriticalSection(&s_ctx->critical_section);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE
    pthread_mutex_lock(&s_ctx->mutex);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL
    SDL_LockMutex(s_ctx->mutex);
#endif
}

void neko_sound_unlock() {
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS
    LeaveCriticalSection(&s_ctx->critical_section);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE
    pthread_mutex_unlock(&s_ctx->mutex);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL
    SDL_UnlockMutex(s_ctx->mutex);
#endif
}

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

static void neko_sound_dsound_get_bytes_to_fill(int* byte_to_lock, int* bytes_to_write) {
    DWORD play_cursor;
    DWORD write_cursor;
    DWORD lock;
    DWORD target_cursor;
    DWORD write;
    DWORD status;

    HRESULT hr = IDirectSoundBuffer_GetCurrentPosition(s_ctx->secondary, &play_cursor, &write_cursor);
    if (hr != DS_OK) {
        if (hr == DSERR_BUFFERLOST) {
            hr = IDirectSoundBuffer_Restore(s_ctx->secondary);
        }
        *byte_to_lock = write_cursor;
        *bytes_to_write = s_ctx->latency_samples * s_ctx->bps;
        if (!SUCCEEDED(hr)) {
            return;
        }
    }

    s_ctx->last_cursor = write_cursor;

    IDirectSoundBuffer_GetStatus(s_ctx->secondary, &status);
    if (!(status & DSBSTATUS_PLAYING)) {
        hr = IDirectSoundBuffer_Play(s_ctx->secondary, 0, 0, DSBPLAY_LOOPING);
        if (!SUCCEEDED(hr)) {
            return;
        }
    }

    lock = (s_ctx->running_index * s_ctx->bps) % s_ctx->buffer_size;
    target_cursor = (write_cursor + s_ctx->latency_samples * s_ctx->bps);
    if (target_cursor > (DWORD)s_ctx->buffer_size) target_cursor %= s_ctx->buffer_size;
    target_cursor = (DWORD)NEKO_SOUND_TRUNC(target_cursor, 16);

    if (lock > target_cursor) {
        write = (s_ctx->buffer_size - lock) + target_cursor;
    } else {
        write = target_cursor - lock;
    }

    *byte_to_lock = lock;
    *bytes_to_write = write;
}

static void neko_sound_dsound_memcpy_to_driver(int16_t* samples, int byte_to_lock, int bytes_to_write) {
    // copy mixer buffers to direct sound
    void* region1;
    DWORD size1;
    void* region2;
    DWORD size2;
    HRESULT hr = IDirectSoundBuffer_Lock(s_ctx->secondary, byte_to_lock, bytes_to_write, &region1, &size1, &region2, &size2, 0);
    if (hr == DSERR_BUFFERLOST) {
        IDirectSoundBuffer_Restore(s_ctx->secondary);
        hr = IDirectSoundBuffer_Lock(s_ctx->secondary, byte_to_lock, bytes_to_write, &region1, &size1, &region2, &size2, 0);
    }
    if (!SUCCEEDED(hr)) {
        return;
    }

    unsigned running_index = s_ctx->running_index;
    INT16* sample1 = (INT16*)region1;
    DWORD sample1_count = size1 / s_ctx->bps;
    memcpy(sample1, samples, sample1_count * sizeof(INT16) * 2);
    samples += sample1_count * 2;
    running_index += sample1_count;

    INT16* sample2 = (INT16*)region2;
    DWORD sample2_count = size2 / s_ctx->bps;
    memcpy(sample2, samples, sample2_count * sizeof(INT16) * 2);
    samples += sample2_count * 2;
    running_index += sample2_count;

    IDirectSoundBuffer_Unlock(s_ctx->secondary, region1, size1, region2, size2);
    s_ctx->running_index = running_index;
}

void neko_sound_dsound_dont_run_too_fast() {
    DWORD status;
    DWORD cursor;
    DWORD junk;
    HRESULT hr;

    hr = IDirectSoundBuffer_GetCurrentPosition(s_ctx->secondary, &junk, &cursor);
    if (hr != DS_OK) {
        if (hr == DSERR_BUFFERLOST) {
            IDirectSoundBuffer_Restore(s_ctx->secondary);
        }
        return;
    }

    // Prevent mixing thread from sending samples too quickly.
    while (cursor == s_ctx->last_cursor) {
        neko_sound_sleep(1);

        IDirectSoundBuffer_GetStatus(s_ctx->secondary, &status);
        if ((status & DSBSTATUS_BUFFERLOST)) {
            IDirectSoundBuffer_Restore(s_ctx->secondary);
            IDirectSoundBuffer_GetStatus(s_ctx->secondary, &status);
            if ((status & DSBSTATUS_BUFFERLOST)) {
                break;
            }
        }

        hr = IDirectSoundBuffer_GetCurrentPosition(s_ctx->secondary, &junk, &cursor);
        if (hr != DS_OK) {
            // Eek! Not much to do here I guess.
            return;
        }
    }
}

#endif  // NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

void neko_sound_mix() {
    __neko_sound_m128i* samples;
    __neko_sound_m128* floatA;
    __neko_sound_m128* floatB;
    __neko_sound_m128 zero;
    int wide_count;
    int samples_to_write;

    neko_sound_lock();

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

    int byte_to_lock;
    int bytes_to_write;
    neko_sound_dsound_get_bytes_to_fill(&byte_to_lock, &bytes_to_write);

    if (bytes_to_write < (int)s_ctx->latency_samples) goto unlock;
    samples_to_write = bytes_to_write / s_ctx->bps;

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE || NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

    int bytes_to_write;
    samples_to_write = neko_sound_samples_to_mix();
    if (!samples_to_write) goto unlock;
    bytes_to_write = samples_to_write * s_ctx->bps;

#endif

    // Clear mixer buffers.
    wide_count = (int)NEKO_SOUND_ALIGN(samples_to_write, 4) / 4;

    floatA = s_ctx->floatA;
    floatB = s_ctx->floatB;
    zero = neko_sound_mm_set1_ps(0.0f);

    for (int i = 0; i < wide_count; ++i) {
        floatA[i] = zero;
        floatB[i] = zero;
    }

    // Mix all playing sounds into the mixer buffers.
    if (!s_ctx->global_pause && !neko_sound_list_empty(&s_ctx->playing_sounds)) {
        neko_sound_list_node_t* playing_node = neko_sound_list_begin(&s_ctx->playing_sounds);
        neko_sound_list_node_t* end_node = neko_sound_list_end(&s_ctx->playing_sounds);
        do {
            neko_sound_list_node_t* next_node = playing_node->next;
            neko_sound_inst_t* playing = NEKO_SOUND_LIST_HOST(neko_sound_inst_t, node, playing_node);
            neko_sound_audio_source_t* audio = playing->audio;

            if (!playing->active || !s_ctx->running) goto remove;
            if (!audio) goto remove;
            if (playing->paused) goto get_next_playing_sound;

            {
                __neko_sound_m128* cA = (__neko_sound_m128*)audio->channels[0];
                __neko_sound_m128* cB = (__neko_sound_m128*)audio->channels[1];

                // Attempted to play a sound with no audio.
                // Make sure the audio file was loaded properly.
                neko_assert(cA);

                int mix_count = samples_to_write;
                int offset = (int)playing->sample_index;
                int remaining = audio->sample_count - offset;
                if (remaining < mix_count) mix_count = remaining;
                neko_assert(remaining > 0);

                float gpan0 = 1.0f - s_ctx->global_pan;
                float gpan1 = s_ctx->global_pan;
                float vA0 = playing->volume * playing->pan0 * gpan0 * s_ctx->global_volume;
                float vB0 = playing->volume * playing->pan1 * gpan1 * s_ctx->global_volume;
                if (!playing->is_music) {
                    vA0 *= s_ctx->sound_volume;
                    vB0 *= s_ctx->sound_volume;
                } else {
                    vA0 *= s_ctx->music_volume;
                    vB0 *= s_ctx->music_volume;
                }
                __neko_sound_m128 vA = neko_sound_mm_set1_ps(vA0);
                __neko_sound_m128 vB = neko_sound_mm_set1_ps(vB0);

                // Skip sound if it's delay is longer than mix_count and
                // handle various delay cases.
                int delay_offset = 0;
                if (offset < 0) {
                    int samples_till_positive = -offset;
                    int mix_leftover = mix_count - samples_till_positive;

                    if (mix_leftover <= 0) {
                        playing->sample_index += mix_count;
                        goto get_next_playing_sound;
                    } else {
                        offset = 0;
                        delay_offset = samples_till_positive;
                        mix_count = mix_leftover;
                    }
                }
                neko_assert(!(delay_offset & 3));

                // SIMD offets.
                int mix_wide = (int)NEKO_SOUND_ALIGN(mix_count, 4) / 4;
                int offset_wide = (int)NEKO_SOUND_TRUNC(offset, 4) / 4;
                int delay_wide = (int)NEKO_SOUND_ALIGN(delay_offset, 4) / 4;
                int sample_count = (mix_wide - 2 * delay_wide) * 4;
                (void)sample_count;

                // apply volume, load samples into float buffers
                switch (audio->channel_count) {
                    case 1:
                        for (int i = delay_wide; i < mix_wide - delay_wide; ++i) {
                            __neko_sound_m128 A = cA[i + offset_wide];
                            __neko_sound_m128 B = neko_sound_mm_mul_ps(A, vB);
                            A = neko_sound_mm_mul_ps(A, vA);
                            floatA[i] = neko_sound_mm_add_ps(floatA[i], A);
                            floatB[i] = neko_sound_mm_add_ps(floatB[i], B);
                        }
                        break;

                    case 2: {
                        for (int i = delay_wide; i < mix_wide - delay_wide; ++i) {
                            __neko_sound_m128 A = cA[i + offset_wide];
                            __neko_sound_m128 B = cB[i + offset_wide];

                            A = neko_sound_mm_mul_ps(A, vA);
                            B = neko_sound_mm_mul_ps(B, vB);
                            floatA[i] = neko_sound_mm_add_ps(floatA[i], A);
                            floatB[i] = neko_sound_mm_add_ps(floatB[i], B);
                        }
                    } break;
                }

                // playing list logic
                playing->sample_index += mix_count;
                neko_assert(playing->sample_index <= audio->sample_count);
                if (playing->sample_index == audio->sample_count) {
                    if (playing->looped) {
                        playing->sample_index = 0;
                        goto get_next_playing_sound;
                    }

                    goto remove;
                }
            }

        get_next_playing_sound:
            playing_node = next_node;
            continue;

        remove:
            playing->sample_index = 0;
            playing->active = false;

            if (playing->audio) {
                if (s_ctx->running) {
                    playing->audio->playing_count -= 1;
                } else {
                    playing->audio->playing_count = 0;
                }
                neko_assert(playing->audio->playing_count >= 0);
            }

            neko_sound_list_remove(playing_node);
            neko_sound_list_push_front(&s_ctx->free_sounds, playing_node);
            hashtable_remove(&s_ctx->instance_map, playing->id);
            playing_node = next_node;
            continue;
        } while (playing_node != end_node);
    }

    // load all floats into 16 bit packed interleaved samples
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS

    samples = s_ctx->samples;
    for (int i = 0; i < wide_count; ++i) {
        __neko_sound_m128i a = neko_sound_mm_cvtps_epi32(floatA[i]);
        __neko_sound_m128i b = neko_sound_mm_cvtps_epi32(floatB[i]);
        __neko_sound_m128i a0b0a1b1 = neko_sound_mm_unpacklo_epi32(a, b);
        __neko_sound_m128i a2b2a3b3 = neko_sound_mm_unpackhi_epi32(a, b);
        samples[i] = neko_sound_mm_packs_epi32(a0b0a1b1, a2b2a3b3);
    }
    neko_sound_dsound_memcpy_to_driver((int16_t*)samples, byte_to_lock, bytes_to_write);
    neko_sound_dsound_dont_run_too_fast();

#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE || NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL

    // Since the ctx->samples array is already in use as a ring buffer
    // reusing floatA to store output is a good way to temporarly store
    // the final samples. Then a single ring buffer push can be used
    // afterwards. Pretty hacky, but whatever :)
    samples = (__neko_sound_m128i*)floatA;
    for (int i = 0; i < wide_count; ++i) {
        __neko_sound_m128i a = neko_sound_mm_cvtps_epi32(floatA[i]);
        __neko_sound_m128i b = neko_sound_mm_cvtps_epi32(floatB[i]);
        __neko_sound_m128i a0b0a1b1 = neko_sound_mm_unpacklo_epi32(a, b);
        __neko_sound_m128i a2b2a3b3 = neko_sound_mm_unpackhi_epi32(a, b);
        samples[i] = neko_sound_mm_packs_epi32(a0b0a1b1, a2b2a3b3);
    }

    neko_sound_push_bytes(samples, bytes_to_write);

#endif

    // Free up any queue'd free's for audio sources at zero refcount.
    for (int i = 0; i < s_ctx->audio_sources_to_free_size;) {
        neko_sound_audio_source_t* audio = s_ctx->audio_sources_to_free[i];
        if (audio->playing_count == 0) {
            neko_sound_free16(audio->channels[0]);
            NEKO_SOUND_FREE(audio);
            s_ctx->audio_sources_to_free[i] = s_ctx->audio_sources_to_free[--s_ctx->audio_sources_to_free_size];
        } else {
            ++i;
        }
    }

unlock:
    neko_sound_unlock();
}

void neko_sound_spawn_mix_thread() {
    if (s_ctx->separate_thread) return;
    s_ctx->separate_thread = true;
#if NEKO_SOUND_PLATFORM == NEKO_SOUND_WINDOWS
    CreateThread(0, 0, neko_sound_ctx_thread, s_ctx, 0, 0);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_APPLE
    pthread_create(&s_ctx->thread, 0, neko_sound_ctx_thread, s_ctx);
#elif NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL
    s_ctx->thread = SDL_CreateThread(&neko_sound_ctx_thread, "CuteSoundThread", s_ctx);
#endif
}

void neko_sound_mix_thread_sleep_delay(int milliseconds) { s_ctx->sleep_milliseconds = milliseconds; }

void* neko_sound_get_context_ptr() { return (void*)s_ctx; }

void neko_sound_set_context_ptr(void* ctx) { s_ctx = (neko_sound_context_t*)ctx; }

// -------------------------------------------------------------------------------------------------
// Loaded sounds.

static void* neko_sound_read_file_to_memory(const char* path, int* size) {
    void* data = 0;
    FILE* fp = fopen(path, "rb");
    int sizeNum = 0;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        sizeNum = (int)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = NEKO_SOUND_ALLOC(sizeNum);
        fread(data, sizeNum, 1, fp);
        fclose(fp);
    }

    if (size) *size = sizeNum;
    return data;
}

static int neko_sound_four_cc(const char* CC, void* memory) {
    if (!memcmp(CC, memory, 4)) return 1;
    return 0;
}

static char* neko_sound_next(char* data) {
    uint32_t size = *(uint32_t*)(data + 4);
    size = (size + 1) & ~1;
    return data + 8 + size;
}

static void neko_sound_last_element(__neko_sound_m128* a, int i, int j, int16_t* samples, int offset) {
    switch (offset) {
        case 1:
            a[i] = neko_sound_mm_set_ps(samples[j], 0.0f, 0.0f, 0.0f);
            break;

        case 2:
            a[i] = neko_sound_mm_set_ps(samples[j], samples[j + 1], 0.0f, 0.0f);
            break;

        case 3:
            a[i] = neko_sound_mm_set_ps(samples[j], samples[j + 1], samples[j + 2], 0.0f);
            break;

        case 0:
            a[i] = neko_sound_mm_set_ps(samples[j], samples[j + 1], samples[j + 2], samples[j + 3]);
            break;
    }
}

neko_sound_audio_source_t* neko_sound_load_wav(const char* path, neko_sound_error_t* err /* = NULL */) {
    int size;
    void* wav = neko_sound_read_file_to_memory(path, &size);
    if (!wav) return NULL;
    neko_sound_audio_source_t* audio = neko_sound_read_mem_wav(wav, size, err);
    NEKO_SOUND_FREE(wav);
    return audio;
}

neko_sound_audio_source_t* neko_sound_read_mem_wav(const void* memory, size_t size, neko_sound_error_t* err) {
    if (err) *err = NEKO_SOUND_ERROR_NONE;
    if (!memory) {
        if (err) *err = NEKO_SOUND_ERROR_FILE_NOT_FOUND;
        return NULL;
    }

#pragma pack(push, 1)
    typedef struct {
        uint16_t wFormatTag;
        uint16_t nChannels;
        uint32_t nSamplesPerSec;
        uint32_t nAvgBytesPerSec;
        uint16_t nBlockAlign;
        uint16_t wBitsPerSample;
        uint16_t cbSize;
        uint16_t wValidBitsPerSample;
        uint32_t dwChannelMask;
        uint8_t SubFormat[18];
    } Fmt;
#pragma pack(pop)

    neko_sound_audio_source_t* audio = NULL;
    char* data = (char*)memory;
    char* end = data + size;
    if (!neko_sound_four_cc("RIFF", data)) {
        if (err) *err = NEKO_SOUND_ERROR_THE_FILE_IS_NOT_A_WAV_FILE;
        return NULL;
    }
    if (!neko_sound_four_cc("WAVE", data + 8)) {
        if (err) *err = NEKO_SOUND_ERROR_THE_FILE_IS_NOT_A_WAV_FILE;
        return NULL;
    }

    data += 12;

    while (1) {
        if (!(end > data)) {
            if (err) *err = NEKO_SOUND_ERROR_WAV_FILE_FORMAT_CHUNK_NOT_FOUND;
            return NULL;
        }
        if (neko_sound_four_cc("fmt ", data)) break;
        data = neko_sound_next(data);
    }

    Fmt fmt;
    fmt = *(Fmt*)(data + 8);
    if (fmt.wFormatTag != 1) {
        if (err) *err = NEKO_SOUND_ERROR_WAV_FILE_FORMAT_CHUNK_NOT_FOUND;
        return NULL;
    }
    if (!(fmt.nChannels == 1 || fmt.nChannels == 2)) {
        if (err) *err = NEKO_SOUND_ERROR_WAV_ONLY_MONO_OR_STEREO_IS_SUPPORTED;
        return NULL;
    }
    if (!(fmt.wBitsPerSample == 16)) {
        if (err) *err = NEKO_SOUND_ERROR_WAV_ONLY_16_BITS_PER_SAMPLE_SUPPORTED;
        return NULL;
    }
    if (!(fmt.nBlockAlign == fmt.nChannels * 2)) {
        if (err) *err = NEKO_SOUND_ERROR_IMPLEMENTATION_ERROR;
        return NULL;
    }

    while (1) {
        if (!(end > data)) {
            if (err) *err = NEKO_SOUND_ERROR_WAV_DATA_CHUNK_NOT_FOUND;
            return NULL;
        }
        if (neko_sound_four_cc("data", data)) break;
        data = neko_sound_next(data);
    }

    audio = (neko_sound_audio_source_t*)NEKO_SOUND_ALLOC(sizeof(neko_sound_audio_source_t));
    memset(audio, 0, sizeof(*audio));
    audio->sample_rate = (int)fmt.nSamplesPerSec;

    {
        int sample_size = *((uint32_t*)(data + 4));
        int sample_count = sample_size / (fmt.nChannels * sizeof(uint16_t));
        audio->sample_count = sample_count;
        audio->channel_count = fmt.nChannels;

        int wide_count = (int)NEKO_SOUND_ALIGN(sample_count, 4);
        wide_count /= 4;
        int wide_offset = sample_count & 3;
        int16_t* samples = (int16_t*)(data + 8);
        float* sample = (float*)NEKO_SOUND_ALLOC(sizeof(float) * 4 + 16);
        sample = (float*)NEKO_SOUND_ALIGN(sample, 16);

        switch (audio->channel_count) {
            case 1: {
                audio->channels[0] = neko_sound_malloc16(wide_count * sizeof(__neko_sound_m128));
                audio->channels[1] = 0;
                __neko_sound_m128* a = (__neko_sound_m128*)audio->channels[0];

                for (int i = 0, j = 0; i < wide_count - 1; ++i, j += 4) {
                    sample[0] = (float)samples[j];
                    sample[1] = (float)samples[j + 1];
                    sample[2] = (float)samples[j + 2];
                    sample[3] = (float)samples[j + 3];
                    a[i] = neko_sound_mm_load_ps(sample);
                }

                neko_sound_last_element(a, wide_count - 1, (wide_count - 1) * 4, samples, wide_offset);
            } break;

            case 2: {
                __neko_sound_m128* a = (__neko_sound_m128*)neko_sound_malloc16(wide_count * sizeof(__neko_sound_m128) * 2);
                __neko_sound_m128* b = a + wide_count;

                for (int i = 0, j = 0; i < wide_count - 1; ++i, j += 8) {
                    sample[0] = (float)samples[j];
                    sample[1] = (float)samples[j + 2];
                    sample[2] = (float)samples[j + 4];
                    sample[3] = (float)samples[j + 6];
                    a[i] = neko_sound_mm_load_ps(sample);

                    sample[0] = (float)samples[j + 1];
                    sample[1] = (float)samples[j + 3];
                    sample[2] = (float)samples[j + 5];
                    sample[3] = (float)samples[j + 7];
                    b[i] = neko_sound_mm_load_ps(sample);
                }

                neko_sound_last_element(a, wide_count - 1, (wide_count - 1) * 4, samples, wide_offset);
                neko_sound_last_element(b, wide_count - 1, (wide_count - 1) * 4 + 4, samples, wide_offset);
                audio->channels[0] = a;
                audio->channels[1] = b;
            } break;

            default:
                if (err) *err = NEKO_SOUND_ERROR_WAV_ONLY_MONO_OR_STEREO_IS_SUPPORTED;
                neko_assert(false);
        }

        NEKO_SOUND_FREE(sample);
    }

    if (err) *err = NEKO_SOUND_ERROR_NONE;
    return audio;
}

void neko_sound_free_audio_source(neko_sound_audio_source_t* audio) {
    if (s_ctx) {
        neko_sound_lock();
        if (audio->playing_count == 0) {
            neko_sound_free16(audio->channels[0]);
            NEKO_SOUND_FREE(audio);
        } else {
            if (s_ctx->audio_sources_to_free_size == s_ctx->audio_sources_to_free_capacity) {
                int new_capacity = s_ctx->audio_sources_to_free_capacity * 2;
                neko_sound_audio_source_t** new_sources = (neko_sound_audio_source_t**)NEKO_SOUND_ALLOC(new_capacity);
                memcpy(new_sources, s_ctx->audio_sources_to_free, sizeof(neko_sound_audio_source_t*) * s_ctx->audio_sources_to_free_size);
                NEKO_SOUND_FREE(s_ctx->audio_sources_to_free);
                s_ctx->audio_sources_to_free = new_sources;
                s_ctx->audio_sources_to_free_capacity = new_capacity;
            }
            s_ctx->audio_sources_to_free[s_ctx->audio_sources_to_free_size++] = audio;
        }
        neko_sound_unlock();
    } else {
        neko_assert(audio->playing_count == 0);
        neko_sound_free16(audio->channels[0]);
        NEKO_SOUND_FREE(audio);
    }
}

#ifdef STB_VORBIS_INCLUDE_STB_VORBIS_H

neko_sound_audio_source_t* neko_sound_read_mem_ogg(const void* memory, size_t length, neko_sound_error_t* err) {
    int16_t* samples = 0;
    neko_sound_audio_source_t* audio = NULL;
    int channel_count;
    int sample_rate;
    int sample_count = stb_vorbis_decode_memory((const unsigned char*)memory, (int)length, &channel_count, &sample_rate, &samples);
    if (sample_count <= 0) {
        if (err) *err = NEKO_SOUND_ERROR_STB_VORBIS_DECODE_FAILED;
        return NULL;
    }
    audio = (neko_sound_audio_source_t*)NEKO_SOUND_ALLOC(sizeof(neko_sound_audio_source_t));
    memset(audio, 0, sizeof(*audio));

    {
        int wide_count = (int)NEKO_SOUND_ALIGN(sample_count, 4) / 4;
        int wide_offset = sample_count & 3;
        float* sample = (float*)NEKO_SOUND_ALLOC(sizeof(float) * 4 + 16);
        sample = (float*)NEKO_SOUND_ALIGN(sample, 16);
        __neko_sound_m128* a = NULL;
        __neko_sound_m128* b = NULL;

        switch (channel_count) {
            case 1: {
                a = (__neko_sound_m128*)neko_sound_malloc16(wide_count * sizeof(__neko_sound_m128), NULL);
                b = 0;

                for (int i = 0, j = 0; i < wide_count - 1; ++i, j += 4) {
                    sample[0] = (float)samples[j];
                    sample[1] = (float)samples[j + 1];
                    sample[2] = (float)samples[j + 2];
                    sample[3] = (float)samples[j + 3];
                    a[i] = neko_sound_mm_load_ps(sample);
                }

                neko_sound_last_element(a, wide_count - 1, (wide_count - 1) * 4, samples, wide_offset);
            } break;

            case 2:
                a = (__neko_sound_m128*)neko_sound_malloc16(wide_count * sizeof(__neko_sound_m128) * 2, NULL);
                b = a + wide_count;

                for (int i = 0, j = 0; i < wide_count - 1; ++i, j += 8) {
                    sample[0] = (float)samples[j];
                    sample[1] = (float)samples[j + 2];
                    sample[2] = (float)samples[j + 4];
                    sample[3] = (float)samples[j + 6];
                    a[i] = neko_sound_mm_load_ps(sample);

                    sample[0] = (float)samples[j + 1];
                    sample[1] = (float)samples[j + 3];
                    sample[2] = (float)samples[j + 5];
                    sample[3] = (float)samples[j + 7];
                    b[i] = neko_sound_mm_load_ps(sample);
                }

                neko_sound_last_element(a, wide_count - 1, (wide_count - 1) * 4, samples, wide_offset);
                neko_sound_last_element(b, wide_count - 1, (wide_count - 1) * 4 + 4, samples, wide_offset);
                break;

            default:
                if (err) *err = NEKO_SOUND_ERROR_OGG_UNSUPPORTED_CHANNEL_COUNT;
                neko_assert(false);
        }

        audio->sample_rate = sample_rate;
        audio->sample_count = sample_count;
        audio->channel_count = channel_count;
        audio->channels[0] = a;
        audio->channels[1] = b;
        audio->playing_count = 0;
        NEKO_SOUND_FREE(samples);
    }

    if (err) *err = NEKO_SOUND_ERROR_NONE;
    return audio;
}

neko_sound_audio_source_t* neko_sound_load_ogg(const char* path, neko_sound_error_t* err) {
    int length;
    void* memory = neko_sound_read_file_to_memory(path, &length, NULL);
    if (!memory) return NULL;
    neko_sound_audio_source_t* audio = neko_sound_read_mem_ogg(memory, length, err);
    NEKO_SOUND_FREE(memory, NULL);
    return audio;
}

#if NEKO_SOUND_PLATFORM == NEKO_SOUND_SDL && defined(SDL_rwops_h_) && defined(NEKO_SOUND_SDL_RWOPS)

neko_sound_audio_source_t* neko_sound_load_ogg_rw(SDL_RWops* rw, neko_sound_error_t* err) {
    int length;
    void* memory = neko_sound_read_rw_to_memory(rw, &length);
    if (!memory) return NULL;
    neko_sound_audio_source_t* audio = neko_sound_read_ogg_wav(memory, length, err);
    NEKO_SOUND_FREE(memory);
    return audio;
}

#endif
#endif  // STB_VORBIS_INCLUDE_STB_VORBIS_H

// -------------------------------------------------------------------------------------------------
// Music sounds.

static void s_insert(neko_sound_inst_t* inst) {
    neko_sound_lock();
    neko_sound_list_push_back(&s_ctx->playing_sounds, &inst->node);
    inst->audio->playing_count += 1;
    inst->active = true;
    inst->id = s_ctx->instance_id_gen++;
    hashtable_insert(&s_ctx->instance_map, inst->id, inst);
    // s_on_make_playing(inst);
    neko_sound_unlock();
}

static neko_sound_inst_t* s_inst_music(neko_sound_audio_source_t* src, float volume) {
    if (neko_sound_list_empty(&s_ctx->free_sounds)) {
        s_add_page();
    }
    neko_assert(!neko_sound_list_empty(&s_ctx->free_sounds));
    neko_sound_inst_t* inst = NEKO_SOUND_LIST_HOST(neko_sound_inst_t, node, neko_sound_list_pop_back(&s_ctx->free_sounds));
    inst->is_music = true;
    inst->looped = s_ctx->music_looped;
    if (!s_ctx->music_paused) inst->paused = false;
    inst->volume = volume;
    inst->pan0 = 0.5f;
    inst->pan1 = 0.5f;
    inst->audio = src;
    inst->sample_index = 0;
    neko_sound_list_init_node(&inst->node);
    s_insert(inst);
    return inst;
}

static neko_sound_inst_t* s_inst(neko_sound_audio_source_t* src, neko_sound_params_t params) {
    if (neko_sound_list_empty(&s_ctx->free_sounds)) {
        s_add_page();
    }
    neko_assert(!neko_sound_list_empty(&s_ctx->free_sounds));
    neko_sound_inst_t* inst = NEKO_SOUND_LIST_HOST(neko_sound_inst_t, node, neko_sound_list_pop_back(&s_ctx->free_sounds));
    float pan = params.pan;
    if (pan > 1.0f)
        pan = 1.0f;
    else if (pan < 0.0f)
        pan = 0.0f;
    float panl = 1.0f - pan;
    float panr = pan;
    inst->is_music = false;
    inst->paused = params.paused;
    inst->looped = params.looped;
    inst->volume = params.volume;
    inst->pan0 = panl;
    inst->pan1 = panr;
    inst->audio = src;
    inst->sample_index = 0;
    neko_sound_list_init_node(&inst->node);
    s_insert(inst);
    return inst;
}

void neko_sound_music_play(neko_sound_audio_source_t* audio_source, float fade_in_time) {
    if (s_ctx->music_state != NEKO_SOUND_MUSIC_STATE_PLAYING) {
        neko_sound_music_stop(0);
    }

    if (fade_in_time < 0) fade_in_time = 0;
    if (fade_in_time) {
        s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_FADE_IN;
    } else {
        s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_PLAYING;
    }
    s_ctx->fade = fade_in_time;
    s_ctx->t = 0;

    neko_assert(s_ctx->music_playing == NULL);
    neko_assert(s_ctx->music_next == NULL);
    neko_sound_inst_t* inst = s_inst_music(audio_source, fade_in_time == 0 ? 1.0f : 0);
    s_ctx->music_playing = inst;
}

void neko_sound_music_stop(float fade_out_time) {
    if (fade_out_time < 0) fade_out_time = 0;

    if (fade_out_time == 0) {
        // Immediately turn off all music if no fade out time.
        if (s_ctx->music_playing) {
            s_ctx->music_playing->active = false;
            s_ctx->music_playing->paused = false;
        }
        if (s_ctx->music_next) {
            s_ctx->music_next->active = false;
            s_ctx->music_next->paused = false;
        }
        s_ctx->music_playing = NULL;
        s_ctx->music_next = NULL;
        s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_NONE;
    } else {
        switch (s_ctx->music_state) {
            case NEKO_SOUND_MUSIC_STATE_NONE:
                break;

            case NEKO_SOUND_MUSIC_STATE_PLAYING:
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_FADE_OUT;
                s_ctx->fade = fade_out_time;
                s_ctx->t = 0;
                break;

            case NEKO_SOUND_MUSIC_STATE_FADE_OUT:
                break;

            case NEKO_SOUND_MUSIC_STATE_FADE_IN: {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_FADE_OUT;
                s_ctx->t = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                s_ctx->fade = fade_out_time;
            } break;

            case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0: {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_FADE_OUT;
                s_ctx->t = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                s_ctx->fade = fade_out_time;
                s_ctx->music_next = NULL;
            } break;

            case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_1:
                // Fall-through.

            case NEKO_SOUND_MUSIC_STATE_CROSSFADE: {
                s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_FADE_OUT;
                s_ctx->t = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
                s_ctx->fade = fade_out_time;
                s_ctx->music_playing = s_ctx->music_next;
                s_ctx->music_next = NULL;
            } break;

            case NEKO_SOUND_MUSIC_STATE_PAUSED:
                neko_sound_music_stop(0);
        }
    }
}

void neko_sound_music_set_volume(float volume_0_to_1) {
    if (volume_0_to_1 < 0) volume_0_to_1 = 0;
    s_ctx->music_volume = volume_0_to_1;
    if (s_ctx->music_playing) s_ctx->music_playing->volume = volume_0_to_1;
    if (s_ctx->music_next) s_ctx->music_next->volume = volume_0_to_1;
}

void neko_sound_music_set_loop(bool true_to_loop) {
    s_ctx->music_looped = true_to_loop;
    if (s_ctx->music_playing) s_ctx->music_playing->looped = true_to_loop;
    if (s_ctx->music_next) s_ctx->music_next->looped = true_to_loop;
}

void neko_sound_music_pause() {
    if (s_ctx->music_state == NEKO_SOUND_MUSIC_STATE_PAUSED) return;
    if (s_ctx->music_playing) s_ctx->music_playing->paused = true;
    if (s_ctx->music_next) s_ctx->music_next->paused = true;
    s_ctx->music_paused = true;
    s_ctx->music_state_to_resume_from_paused = s_ctx->music_state;
    s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_PAUSED;
}

void neko_sound_music_resume() {
    if (s_ctx->music_state != NEKO_SOUND_MUSIC_STATE_PAUSED) return;
    if (s_ctx->music_playing) s_ctx->music_playing->paused = false;
    if (s_ctx->music_next) s_ctx->music_next->paused = false;
    s_ctx->music_state = s_ctx->music_state_to_resume_from_paused;
}

void neko_sound_music_switch_to(neko_sound_audio_source_t* audio_source, float fade_out_time, float fade_in_time) {
    if (fade_in_time < 0) fade_in_time = 0;
    if (fade_out_time < 0) fade_out_time = 0;

    switch (s_ctx->music_state) {
        case NEKO_SOUND_MUSIC_STATE_NONE:
            neko_sound_music_play(audio_source, fade_in_time);
            break;

        case NEKO_SOUND_MUSIC_STATE_PLAYING: {
            neko_assert(s_ctx->music_next == NULL);
            neko_sound_inst_t* inst = s_inst_music(audio_source, fade_in_time == 0 ? 1.0f : 0);
            s_ctx->music_next = inst;

            s_ctx->fade = fade_out_time;
            s_ctx->fade_switch_1 = fade_in_time;
            s_ctx->t = 0;
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0;
        } break;

        case NEKO_SOUND_MUSIC_STATE_FADE_OUT: {
            neko_assert(s_ctx->music_next == NULL);
            neko_sound_inst_t* inst = s_inst_music(audio_source, fade_in_time == 0 ? 1.0f : 0);
            s_ctx->music_next = inst;

            s_ctx->fade_switch_1 = fade_in_time;
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0;
        } break;

        case NEKO_SOUND_MUSIC_STATE_FADE_IN: {
            neko_assert(s_ctx->music_next == NULL);
            neko_sound_inst_t* inst = s_inst_music(audio_source, fade_in_time == 0 ? 1.0f : 0);
            s_ctx->music_next = inst;

            s_ctx->fade_switch_1 = fade_in_time;
            s_ctx->t = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0;
        } break;

        case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0: {
            neko_assert(s_ctx->music_next != NULL);
            neko_sound_inst_t* inst = s_inst_music(audio_source, fade_in_time == 0 ? 1.0f : 0);
            s_ctx->music_next->active = false;
            s_ctx->music_next = inst;
            s_ctx->fade_switch_1 = fade_in_time;
        } break;

        case NEKO_SOUND_MUSIC_STATE_CROSSFADE:  // Fall-through.
        case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_1: {
            neko_assert(s_ctx->music_next != NULL);
            neko_sound_inst_t* inst = s_inst_music(audio_source, fade_in_time == 0 ? 1.0f : 0);
            s_ctx->music_playing = s_ctx->music_next;
            s_ctx->music_next = inst;

            s_ctx->t = s_smoothstep(((s_ctx->fade - s_ctx->t) / s_ctx->fade));
            s_ctx->fade_switch_1 = fade_in_time;
            s_ctx->fade = fade_out_time;
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0;
        } break;

        case NEKO_SOUND_MUSIC_STATE_PAUSED:
            neko_sound_music_stop(0);
            neko_sound_music_switch_to(audio_source, fade_out_time, fade_in_time);
            break;
    }
}

void neko_sound_music_crossfade(neko_sound_audio_source_t* audio_source, float cross_fade_time) {
    if (cross_fade_time < 0) cross_fade_time = 0;

    switch (s_ctx->music_state) {
        case NEKO_SOUND_MUSIC_STATE_NONE:
            neko_sound_music_play(audio_source, cross_fade_time);

        case NEKO_SOUND_MUSIC_STATE_PLAYING: {
            neko_assert(s_ctx->music_next == NULL);
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_CROSSFADE;

            neko_sound_inst_t* inst = s_inst_music(audio_source, cross_fade_time == 0 ? 1.0f : 0);
            inst->paused = false;
            s_ctx->music_next = inst;

            s_ctx->fade = cross_fade_time;
            s_ctx->t = 0;
        } break;

        case NEKO_SOUND_MUSIC_STATE_FADE_OUT:
            neko_assert(s_ctx->music_next == NULL);
            // Fall-through.

        case NEKO_SOUND_MUSIC_STATE_FADE_IN: {
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_CROSSFADE;

            neko_sound_inst_t* inst = s_inst_music(audio_source, cross_fade_time == 0 ? 1.0f : 0);
            inst->paused = false;
            s_ctx->music_next = inst;

            s_ctx->fade = cross_fade_time;
        } break;

        case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_0: {
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_CROSSFADE;
            s_ctx->music_next->active = false;

            neko_sound_inst_t* inst = s_inst_music(audio_source, cross_fade_time == 0 ? 1.0f : 0);
            inst->paused = false;
            s_ctx->music_next = inst;

            s_ctx->fade = cross_fade_time;
        } break;

        case NEKO_SOUND_MUSIC_STATE_SWITCH_TO_1:  // Fall-through.
        case NEKO_SOUND_MUSIC_STATE_CROSSFADE: {
            s_ctx->music_state = NEKO_SOUND_MUSIC_STATE_CROSSFADE;
            s_ctx->music_playing->active = false;
            s_ctx->music_playing = s_ctx->music_next;

            neko_sound_inst_t* inst = s_inst_music(audio_source, cross_fade_time == 0 ? 1.0f : 0);
            inst->paused = false;
            s_ctx->music_next = inst;

            s_ctx->fade = cross_fade_time;
        } break;

        case NEKO_SOUND_MUSIC_STATE_PAUSED:
            neko_sound_music_stop(0);
            neko_sound_music_crossfade(audio_source, cross_fade_time);
    }
}

uint64_t neko_sound_music_get_sample_index() {
    if (s_ctx->music_playing)
        return 0;
    else
        return s_ctx->music_playing->sample_index;
}

neko_sound_error_t neko_sound_music_set_sample_index(uint64_t sample_index) {
    if (s_ctx->music_playing) return NEKO_SOUND_ERROR_INVALID_SOUND;
    if (sample_index > s_ctx->music_playing->audio->sample_count) return NEKO_SOUND_ERROR_TRIED_TO_SET_SAMPLE_INDEX_BEYOND_THE_AUDIO_SOURCES_SAMPLE_COUNT;
    s_ctx->music_playing->sample_index = sample_index;
    return NEKO_SOUND_ERROR_NONE;
}

// -------------------------------------------------------------------------------------------------
// Playing sounds.

neko_sound_params_t neko_sound_params_default() {
    neko_sound_params_t params;
    params.paused = false;
    params.looped = false;
    params.volume = 1.0f;
    params.pan = 0.5f;
    params.delay = 0.0f;
    return params;
}

static neko_sound_inst_t* s_get_inst(neko_sound_playing_sound_t sound) { return (neko_sound_inst_t*)hashtable_find(&s_ctx->instance_map, sound.id); }

neko_sound_playing_sound_t neko_sound_play_sound(neko_sound_audio_source_t* audio, neko_sound_params_t params) {
    neko_sound_inst_t* inst = s_inst(audio, params);
    neko_sound_playing_sound_t sound = {inst->id};
    return sound;
}

bool neko_sound_is_active(neko_sound_playing_sound_t sound) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return false;
    return inst->active;
}

bool neko_sound_get_is_paused(neko_sound_playing_sound_t sound) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return false;
    return inst->paused;
}

bool neko_sound_get_is_looped(neko_sound_playing_sound_t sound) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return false;
    return inst->looped;
}

float neko_sound_get_volume(neko_sound_playing_sound_t sound) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return 0;
    return inst->volume;
}

uint64_t neko_sound_get_sample_index(neko_sound_playing_sound_t sound) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return 0;
    return inst->sample_index;
}

void neko_sound_set_is_paused(neko_sound_playing_sound_t sound, bool true_for_paused) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return;
    inst->paused = true_for_paused;
}

void neko_sound_set_is_looped(neko_sound_playing_sound_t sound, bool true_for_looped) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return;
    inst->looped = true_for_looped;
}

void neko_sound_set_volume(neko_sound_playing_sound_t sound, float volume_0_to_1) {
    if (volume_0_to_1 < 0) volume_0_to_1 = 0;
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return;
    inst->volume = volume_0_to_1;
}

neko_sound_error_t neko_sound_set_sample_index(neko_sound_playing_sound_t sound, uint64_t sample_index) {
    neko_sound_inst_t* inst = s_get_inst(sound);
    if (!inst) return NEKO_SOUND_ERROR_INVALID_SOUND;
    if (sample_index > inst->audio->sample_count) return NEKO_SOUND_ERROR_TRIED_TO_SET_SAMPLE_INDEX_BEYOND_THE_AUDIO_SOURCES_SAMPLE_COUNT;
    inst->sample_index = sample_index;
    return NEKO_SOUND_ERROR_NONE;
}

void neko_sound_set_playing_sounds_volume(float volume_0_to_1) {
    if (volume_0_to_1 < 0) volume_0_to_1 = 0;
    s_ctx->sound_volume = volume_0_to_1;
}

void neko_sound_stop_all_playing_sounds() {
    neko_sound_lock();

    // Set all playing sounds (that aren't music) active to false.
    if (neko_sound_list_empty(&s_ctx->playing_sounds)) {
        neko_sound_unlock();
        return;
    }
    neko_sound_list_node_t* playing_sound = neko_sound_list_begin(&s_ctx->playing_sounds);
    neko_sound_list_node_t* end = neko_sound_list_end(&s_ctx->playing_sounds);

    do {
        neko_sound_inst_t* inst = NEKO_SOUND_LIST_HOST(neko_sound_inst_t, node, playing_sound);
        neko_sound_list_node_t* next = playing_sound->next;
        if (inst != s_ctx->music_playing && inst != s_ctx->music_next) {
            inst->active = false;  // Let neko_sound_mix handle cleaning this up.
        }
        playing_sound = next;
    } while (playing_sound != end);

    neko_sound_unlock();
}

void* neko_sound_get_global_context() { return s_ctx; }

void neko_sound_set_global_context(void* context) { s_ctx = (neko_sound_context_t*)context; }

#endif
