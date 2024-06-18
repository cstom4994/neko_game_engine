
#include "sound.h"

#include "engine/neko_api.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_luabind.hpp"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_WEBAUDIO
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#undef neko_safe_malloc
#undef neko_safe_free
#undef neko_safe_realloc
#undef neko_safe_calloc

#define neko_safe_malloc(size) g_interface->common.__neko_mem_safe_alloc((size), (char*)__FILE__, __LINE__, NULL)
#define neko_safe_free(mem) g_interface->common.__neko_mem_safe_free((void*)mem, NULL)
#define neko_safe_realloc(ptr, size) g_interface->common.__neko_mem_safe_realloc((ptr), (size), (char*)__FILE__, __LINE__, NULL)
#define neko_safe_calloc(count, element_size) g_interface->common.__neko_mem_safe_calloc(count, element_size, (char*)__FILE__, __LINE__, NULL)

// #include "sandbox/hpp/neko_cpp_utils.hpp"

namespace neko::sound {

typedef struct neko_lua_sound {
    ma_sound ma;
    bool zombie;
    bool dead_end;
} neko_lua_sound;

static void on_sound_end(void* udata, ma_sound* ma) {
    neko_lua_sound* sound = (neko_lua_sound*)udata;
    if (sound->zombie) {
        sound->dead_end = true;
    }
}

neko_lua_sound* sound_load(ma_engine* audio_engine, const_str filepath) {

    ma_result res = MA_SUCCESS;

    neko_lua_sound* sound = (neko_lua_sound*)neko_safe_malloc(sizeof(neko_lua_sound));

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

void sound_fini(neko_lua_sound* sound) { ma_sound_uninit(&sound->ma); }

}  // namespace neko::sound

neko::array<neko::sound::neko_lua_sound*> garbage_sounds;
void* miniaudio_vfs;
ma_engine audio_engine;

// miniaudio vfs

struct AudioFile {
    u8* buf;
    u64 cursor;
    u64 len;
};

void* vfs_for_miniaudio() {
    ma_vfs_callbacks vtbl = {};

    vtbl.onOpen = [](ma_vfs* pVFS, const char* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile) -> ma_result {
        if (openMode & MA_OPEN_MODE_WRITE) {
            return MA_ERROR;
        }

        u64 len = 0;
        const_str data = g_interface->common.capi_vfs_read_file(NEKO_DEFAULT_PACK, pFilePath, &len);
        if (!data || !len) {
            return MA_ERROR;
        }

        AudioFile* file = (AudioFile*)neko_safe_malloc(sizeof(AudioFile));
        file->buf = (u8*)data;
        file->len = len;
        file->cursor = 0;

        *pFile = file;
        return MA_SUCCESS;
    };

    vtbl.onClose = [](ma_vfs* pVFS, ma_vfs_file file) -> ma_result {
        AudioFile* f = (AudioFile*)file;
        neko_safe_free(f->buf);
        neko_safe_free(f);
        return MA_SUCCESS;
    };

    vtbl.onRead = [](ma_vfs* pVFS, ma_vfs_file file, void* pDst, size_t sizeInBytes, size_t* pBytesRead) -> ma_result {
        AudioFile* f = (AudioFile*)file;

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

    vtbl.onWrite = [](ma_vfs* pVFS, ma_vfs_file file, const void* pSrc, size_t sizeInBytes, size_t* pBytesWritten) -> ma_result { return MA_NOT_IMPLEMENTED; };

    vtbl.onSeek = [](ma_vfs* pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) -> ma_result {
        AudioFile* f = (AudioFile*)file;

        s64 seek = 0;
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

    vtbl.onTell = [](ma_vfs* pVFS, ma_vfs_file file, ma_int64* pCursor) -> ma_result {
        AudioFile* f = (AudioFile*)file;
        *pCursor = f->cursor;
        return MA_SUCCESS;
    };

    vtbl.onInfo = [](ma_vfs* pVFS, ma_vfs_file file, ma_file_info* pInfo) -> ma_result {
        AudioFile* f = (AudioFile*)file;
        pInfo->sizeInBytes = f->len;
        return MA_SUCCESS;
    };

    ma_vfs_callbacks* ptr = (ma_vfs_callbacks*)neko_safe_malloc(sizeof(ma_vfs_callbacks));
    *ptr = vtbl;
    return ptr;
}

// mt_sound

static ma_sound* sound_ma(lua_State* L) {
    neko::sound::neko_lua_sound* sound = *(neko::sound::neko_lua_sound**)luaL_checkudata(L, 1, "mt_sound");
    return &sound->ma;
}

static int mt_sound_gc(lua_State* L) {
    neko::sound::neko_lua_sound* sound = *(neko::sound::neko_lua_sound**)luaL_checkudata(L, 1, "mt_sound");

    if (ma_sound_at_end(&sound->ma)) {
        sound_fini(sound);
        neko_safe_free(sound);
    } else {
        sound->zombie = true;
        garbage_sounds.push(sound);  // TODO:: 建立 garbage_sounds 每帧清理
    }

    return 0;
}

static int mt_sound_frames(lua_State* L) {
    unsigned long long frames = 0;
    ma_result res = ma_sound_get_length_in_pcm_frames(sound_ma(L), &frames);
    if (res != MA_SUCCESS) {
        return 0;
    }

    lua_pushinteger(L, (lua_Integer)frames);
    return 1;
}

static int mt_sound_start(lua_State* L) {
    ma_result res = ma_sound_start(sound_ma(L));
    if (res != MA_SUCCESS) {
        luaL_error(L, "failed to start sound");
    }

    return 0;
}

static int mt_sound_stop(lua_State* L) {
    ma_result res = ma_sound_stop(sound_ma(L));
    if (res != MA_SUCCESS) {
        luaL_error(L, "failed to stop sound");
    }

    return 0;
}

static int mt_sound_seek(lua_State* L) {
    lua_Number f = luaL_optnumber(L, 2, 0);

    ma_result res = ma_sound_seek_to_pcm_frame(sound_ma(L), f);
    if (res != MA_SUCCESS) {
        luaL_error(L, "failed to seek to frame");
    }

    return 0;
}

static int mt_sound_secs(lua_State* L) {
    float len = 0;
    ma_result res = ma_sound_get_length_in_seconds(sound_ma(L), &len);
    if (res != MA_SUCCESS) {
        return 0;
    }

    lua_pushnumber(L, len);
    return 1;
}

static int mt_sound_vol(lua_State* L) {
    lua_pushnumber(L, ma_sound_get_volume(sound_ma(L)));
    return 1;
}

static int mt_sound_set_vol(lua_State* L) {
    ma_sound_set_volume(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
    return 0;
}

static int mt_sound_pan(lua_State* L) {
    lua_pushnumber(L, ma_sound_get_pan(sound_ma(L)));
    return 1;
}

static int mt_sound_set_pan(lua_State* L) {
    ma_sound_set_pan(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
    return 0;
}

static int mt_sound_pitch(lua_State* L) {
    lua_pushnumber(L, ma_sound_get_pitch(sound_ma(L)));
    return 1;
}

static int mt_sound_set_pitch(lua_State* L) {
    ma_sound_set_pitch(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
    return 0;
}

static int mt_sound_loop(lua_State* L) {
    lua_pushboolean(L, ma_sound_is_looping(sound_ma(L)));
    return 1;
}

static int mt_sound_set_loop(lua_State* L) {
    ma_sound_set_looping(sound_ma(L), lua_toboolean(L, 2));
    return 0;
}

static int mt_sound_pos(lua_State* L) {
    ma_vec3f pos = ma_sound_get_position(sound_ma(L));
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

static int mt_sound_set_pos(lua_State* L) {
    lua_Number x = luaL_optnumber(L, 2, 0);
    lua_Number y = luaL_optnumber(L, 3, 0);
    ma_sound_set_position(sound_ma(L), (float)x, (float)y, 0.0f);
    return 0;
}

static int mt_sound_dir(lua_State* L) {
    ma_vec3f dir = ma_sound_get_direction(sound_ma(L));
    lua_pushnumber(L, dir.x);
    lua_pushnumber(L, dir.y);
    return 2;
}

static int mt_sound_set_dir(lua_State* L) {
    lua_Number x = luaL_optnumber(L, 2, 0);
    lua_Number y = luaL_optnumber(L, 3, 0);
    ma_sound_set_direction(sound_ma(L), (float)x, (float)y, 0.0f);
    return 0;
}

static int mt_sound_vel(lua_State* L) {
    ma_vec3f vel = ma_sound_get_velocity(sound_ma(L));
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    return 2;
}

static int mt_sound_set_vel(lua_State* L) {
    lua_Number x = luaL_optnumber(L, 2, 0);
    lua_Number y = luaL_optnumber(L, 3, 0);
    ma_sound_set_velocity(sound_ma(L), (float)x, (float)y, 0.0f);
    return 0;
}

static int mt_sound_set_fade(lua_State* L) {
    lua_Number from = luaL_optnumber(L, 2, 0);
    lua_Number to = luaL_optnumber(L, 3, 0);
    lua_Number ms = luaL_optnumber(L, 4, 0);
    ma_sound_set_fade_in_milliseconds(sound_ma(L), (float)from, (float)to, (u64)ms);
    return 0;
}

static int open_mt_sound(lua_State* L) {
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

static int neko_sound_load(lua_State* L) {
    neko::string str = neko::luax_check_string(L, 1);

    neko::sound::neko_lua_sound* sound = neko::sound::sound_load(NULL, str.data);
    if (sound == nullptr) {
        return 0;
    }

    luax_ptr_userdata(L, sound, "mt_sound");
    return 1;
}

void OnInit() {
    miniaudio_vfs = vfs_for_miniaudio();

    ma_engine_config ma_config = ma_engine_config_init();
    ma_config.channels = 2;
    ma_config.sampleRate = 44100;
    ma_config.pResourceManagerVFS = miniaudio_vfs;
    ma_result res = ma_engine_init(&ma_config, &audio_engine);
    if (res != MA_SUCCESS) {
        // NEKO_ERROR("%s", "failed to initialize audio engine");
    }
}

void OnFini() {
    ma_engine_uninit(&audio_engine);
    neko_safe_free(miniaudio_vfs);
}

void OnPostUpdate() {
    auto& sounds = garbage_sounds;
    for (u64 i = 0; i < sounds.len;) {
        auto* sound = sounds[i];

        if (sound->dead_end) {
            assert(sound->zombie);
            sound_fini(sound);
            neko_safe_free(sound);

            sounds[i] = sounds[sounds.len - 1];
            sounds.len--;
        } else {
            i++;
        }
    }
}

Neko_ModuleInterface* g_interface;

NEKO_DLL_EXPORT s32 Neko_OnModuleLoad(Neko_ModuleInterface* module_interface) {
    assert(module_interface != NULL);
    g_interface = module_interface;
    return 666;
}