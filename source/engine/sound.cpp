
#include "engine/sound.h"

#include "base/common/profiler.hpp"
#include "base/common/vfs.hpp"
#include "base/common/reflection.hpp"
#include "engine/bootstrap.h"

void *vfs_for_miniaudio();

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

    res = ma_sound_init_from_file(&the<CL>().audio_engine, cpath.data, 0, nullptr, nullptr, &sound->ma);
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
        the<CL>().garbage_sounds.push(sound);
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

void sound_init() {
    PROFILE_FUNC();

    {
        PROFILE_BLOCK("miniaudio");

        the<CL>().miniaudio_vfs = vfs_for_miniaudio();

        ma_engine_config ma_config = ma_engine_config_init();
        ma_config.channels = 2;
        ma_config.sampleRate = 44100;
        ma_config.pResourceManagerVFS = the<CL>().miniaudio_vfs;
        ma_result res = ma_engine_init(&ma_config, &the<CL>().audio_engine);
        if (res != MA_SUCCESS) {
            gBase.fatal_error("failed to initialize audio engine");
        }
    }

#if NEKO_AUDIO == 2
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
#endif

    auto L = ENGINE_LUA();

    lua_register(
            L, "audio_load_bank", +[](lua_State *L) {
                const_str name = lua_tostring(L, 1);
                int type = lua_tointeger(L, 2);
                // audio_load_bank(0, name, type);
                return 0;
            });
    lua_register(
            L, "audio_play_event", +[](lua_State *L) {
                const_str name = lua_tostring(L, 1);
                // audio_play_event(name);
                return 0;
            });
    lua_register(
            L, "audio_load_event", +[](lua_State *L) {
                const_str name = lua_tostring(L, 1);
                // audio_load_event(name);
                return 0;
            });
}

void sound_fini() {
    for (Sound *sound : the<CL>().garbage_sounds) {
        sound->trash();
    }
    the<CL>().garbage_sounds.trash();

    ma_engine_uninit(&the<CL>().audio_engine);
    mem_free(the<CL>().miniaudio_vfs);
}

int sound_update_all(Event evt) { return 0; }

int sound_postupdate(Event evt) {
    Array<Sound *> &sounds = the<CL>().garbage_sounds;
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
    return 0;
}

void *vfs_for_miniaudio() {
    ma_vfs_callbacks vtbl = {};

    vtbl.onOpen = [](ma_vfs *pVFS, const char *pFilePath, ma_uint32 openMode, ma_vfs_file *pFile) -> ma_result {
        String contents = {};

        if (openMode & MA_OPEN_MODE_WRITE) {
            return MA_ERROR;
        }

        bool ok = the<VFS>().read_entire_file(&contents, pFilePath);
        if (!ok) {
            return MA_ERROR;
        }

        AudioFile *file = (AudioFile *)mem_alloc(sizeof(AudioFile));
        file->buf = (u8 *)contents.data;
        file->len = contents.len;
        file->cursor = 0;

        *pFile = file;
        return MA_SUCCESS;
    };

    vtbl.onClose = [](ma_vfs *pVFS, ma_vfs_file file) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        mem_free(f->buf);
        mem_free(f);
        return MA_SUCCESS;
    };

    vtbl.onRead = [](ma_vfs *pVFS, ma_vfs_file file, void *pDst, size_t sizeInBytes, size_t *pBytesRead) -> ma_result {
        AudioFile *f = (AudioFile *)file;

        u64 remaining = f->len - f->cursor;
        u64 len = remaining < sizeInBytes ? remaining : sizeInBytes;
        memcpy(pDst, &f->buf[f->cursor], len);

        if (pBytesRead != nullptr) {
            *pBytesRead = len;
        }

        if (len != sizeInBytes) {
            return MA_AT_END;
        }

        return MA_SUCCESS;
    };

    vtbl.onWrite = [](ma_vfs *pVFS, ma_vfs_file file, const void *pSrc, size_t sizeInBytes, size_t *pBytesWritten) -> ma_result { return MA_NOT_IMPLEMENTED; };

    vtbl.onSeek = [](ma_vfs *pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) -> ma_result {
        AudioFile *f = (AudioFile *)file;

        i64 seek = 0;
        switch (origin) {
            case ma_seek_origin_start:
                seek = offset;
                break;
            case ma_seek_origin_end:
                seek = f->len + offset;
                break;
            case ma_seek_origin_current:
            default:
                seek = f->cursor + offset;
                break;
        }

        if (seek < 0 || seek > f->len) {
            return MA_ERROR;
        }

        f->cursor = (u64)seek;
        return MA_SUCCESS;
    };

    vtbl.onTell = [](ma_vfs *pVFS, ma_vfs_file file, ma_int64 *pCursor) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        *pCursor = f->cursor;
        return MA_SUCCESS;
    };

    vtbl.onInfo = [](ma_vfs *pVFS, ma_vfs_file file, ma_file_info *pInfo) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        pInfo->sizeInBytes = f->len;
        return MA_SUCCESS;
    };

    ma_vfs_callbacks *ptr = (ma_vfs_callbacks *)mem_alloc(sizeof(ma_vfs_callbacks));
    *ptr = vtbl;
    return ptr;
}
