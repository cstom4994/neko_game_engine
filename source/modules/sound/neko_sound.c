#include "neko_sound.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_WEBAUDIO
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

// #include "sandbox/hpp/neko_cpp_utils.hpp"

static void on_sound_end(void *udata, ma_sound *ma) {
    neko_lua_sound *sound = (neko_lua_sound *)udata;
    if (sound->zombie) {
        sound->dead_end = true;
    }
}

neko_lua_sound *sound_load(ma_engine *audio_engine, const_str filepath) {

    ma_result res = MA_SUCCESS;

    neko_lua_sound *sound = (neko_lua_sound *)neko_safe_malloc(sizeof(neko_lua_sound));

    // neko::string cpath = to_cstr(filepath);
    // neko_defer(neko_safe_free(cpath.data));

    res = ma_sound_init_from_file(audio_engine, filepath, 0, NULL, NULL, &sound->ma);
    if (res != MA_SUCCESS) {
        neko_safe_free(sound);
        return NULL;
    }

    res = ma_sound_set_end_callback(&sound->ma, on_sound_end, sound);
    if (res != MA_SUCCESS) {
        neko_safe_free(sound);
        return NULL;
    }

    sound->zombie = false;
    sound->dead_end = false;
    return sound;
}

void sound_fini(neko_lua_sound *sound) { ma_sound_uninit(&sound->ma); }
