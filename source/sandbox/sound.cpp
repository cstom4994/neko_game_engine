#include "sound.h"

#include "game_main.h"
#include "sandbox/hpp/neko_cpp_utils.hpp"

static void on_sound_end(void *udata, neko_sound *ma) {
    Sound *sound = (Sound *)udata;
    if (sound->zombie) {
        sound->dead_end = true;
    }
}

Sound *sound_load(neko::string filepath) {

    ma_result res = MA_SUCCESS;

    Sound *sound = (Sound *)neko_safe_malloc(sizeof(Sound));

    neko::string cpath = to_cstr(filepath);
    neko_defer(neko_safe_free(cpath.data));

    res = ma_sound_init_from_file(&CL_GAME_USERDATA()->audio_engine, cpath.data, 0, nullptr, nullptr, &sound->ma);
    if (res != MA_SUCCESS) {
        neko_safe_free(sound);
        return nullptr;
    }

    res = ma_sound_set_end_callback(&sound->ma, on_sound_end, sound);
    if (res != MA_SUCCESS) {
        neko_safe_free(sound);
        return nullptr;
    }

    sound->zombie = false;
    sound->dead_end = false;
    return sound;
}

void Sound::trash() { ma_sound_uninit(&ma); }
