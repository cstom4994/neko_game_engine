
#include "engine/neko_audio.h"

#include "engine/neko_engine.h"

/*=============================
// NEKO_AUDIO_IMPL
=============================*/

#define NEKO_SOUND_IMPLEMENTATION
#define NEKO_SOUND_PLATFORM_WINDOWS
// #define NEKO_SOUND_ALLOC(size, ctx) neko_safe_malloc((size_t)size)
// #define NEKO_SOUND_FREE(mem, ctx) neko_safe_free(mem)
// #define HASHTABLE_MALLOC(ctx, size) (neko_gc_malloc(&g_gc, size))
// #define HASHTABLE_FREE(ctx, ptr) (neko_gc_free(&g_gc, ptr))
#include "engine/builtin/cute_sound.h"

void __neko_audio_set_default_functions(struct neko_audio_s *audio);

neko_result __neko_audio_init(struct neko_audio_s *s) {

    neko_platform_t *platform = neko_subsystem(platform);

    HWND hwnd = (HWND)neko_platform_hwnd();
    neko_sound_init(hwnd, 44100, 1024, NULL);

    neko_sound_spawn_mix_thread();
    // neko_sound_mix_thread_sleep_delay(1);

    return NEKO_RESULT_SUCCESS;
}
neko_result __neko_audio_shutdown(struct neko_audio_s *s) {

    neko_sound_shutdown();

    return NEKO_RESULT_SUCCESS;
}
neko_result __neko_audio_update(struct neko_audio_s *s) {

    neko_sound_update(neko_subsystem(platform)->time.delta);

    return NEKO_RESULT_SUCCESS;
}
neko_result __neko_audio_commit(struct neko_audio_s *s) { return NEKO_RESULT_SUCCESS; }

NEKO_API_DECL neko_audio_t *__neko_audio_construct() {
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
