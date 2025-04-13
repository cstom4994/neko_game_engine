
#include "engine/sound.h"

#include "base/common/profiler.hpp"
#include "base/common/vfs.hpp"
#include "base/common/reflection.hpp"
#include "engine/bootstrap.h"

#include "engine/scripting/wrap_meta.h"

static void *vfs_for_miniaudio() {
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

static void on_sound_end(void *udata, ma_sound *ma) {
    SoundSource *sound = (SoundSource *)udata;
    if (sound->zombie) {
        sound->end = true;
    }
}

void sound_trash(SoundSource *v) {
    assert(v && !v->dead);
    ma_sound_uninit(&v->ma);
    v->dead = true;
    mem_del(v);
}

SoundSource *sound_load(String filepath) {
    PROFILE_FUNC();

    ma_result res = MA_SUCCESS;

    String cpath = to_cstr(filepath);
    neko_defer(mem_free(cpath.data));

    SoundSource *v = mem_new<SoundSource>();

    res = ma_sound_init_from_file(the<Sound>().GetAudioEngine(), cpath.data, 0, nullptr, nullptr, &v->ma);
    if (res != MA_SUCCESS) {
        sound_trash(v);
        return nullptr;
    }

    res = ma_sound_set_end_callback(&v->ma, on_sound_end, v);
    if (res != MA_SUCCESS) {
        sound_trash(v);
        return nullptr;
    }

    v->zombie = false;
    v->end = false;
    v->dead = false;

    return v;
}

namespace Neko {

namespace SoundWrap {

static SoundIndex &to(lua_State *L, int idx) { return luabind::checkudata<SoundIndex>(L, idx); }

static SoundIndex &check_soundindex_udata(lua_State *L, i32 arg) {
    SoundIndex &udata = to(L, arg);
    return udata;
}

static ma_sound *sound_ma(lua_State *L) {
    SoundIndex &idx = check_soundindex_udata(L, 1);
    return &idx.ptr->ma;
}

static int mt_sound_gc(lua_State *L) {
    SoundIndex &idx = check_soundindex_udata(L, 1);

    SoundSource *v = idx.ptr;
    neko_assert(v);

    // 检查是否已经播放完毕
    // 如果播放完毕则trash销毁对象
    // 否则PushSoundGarbag等待处理
    if (ma_sound_at_end(&v->ma)) {
        sound_trash(v);
        idx.ptr = nullptr;
    } else {
        v->zombie = true;
        SoundIndex garbage = {v};
        the<Sound>().PushSoundGarbage(garbage);
    }

    destroyudata<SoundIndex>(L);

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

static int mt_close(lua_State *L) {
    auto &self = to(L, 1);
    return 0;
}

void metatable(lua_State *L) {
    static luaL_Reg lib[] = {
            {"frames", mt_sound_frames},
            {"secs", mt_sound_secs},
            {"start", mt_sound_start},
            {"stop", mt_sound_stop},
            {"seek", mt_sound_seek},
            {"vol", mt_sound_vol},
            {"set_vol", mt_sound_set_vol},
            {"pan", mt_sound_pan},
            {"set_pan", mt_sound_set_pan},
            {"pitch", mt_sound_pitch},
            {"set_pitch", mt_sound_set_pitch},
            {"loop", mt_sound_loop},
            {"set_loop", mt_sound_set_loop},
            {"pos", mt_sound_pos},
            {"set_pos", mt_sound_set_pos},
            {"dir", mt_sound_dir},
            {"set_dir", mt_sound_set_dir},
            {"vel", mt_sound_vel},
            {"set_vel", mt_sound_set_vel},
            {"set_fade", mt_sound_set_fade},
            {nullptr, nullptr},
    };
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__close", mt_close}, {"__gc", mt_sound_gc}, {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

int open_mt_sound(lua_State *L) { return 0; }

int neko_sound_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    SoundIndex &idx = luabind::newudata<SoundIndex>(L);
    lua_pushstring(L, str.cstr());
    lua_setiuservalue(L, -2, 1);
    SoundSource *sound = sound_load(str);
    if (sound == nullptr) {
        return 0;
    }

    idx.ptr = sound;

    return 1;
}

}  // namespace SoundWrap

}  // namespace Neko

void Sound::sound_init() {
    PROFILE_FUNC();

    {
        PROFILE_BLOCK("miniaudio");

        miniaudio_vfs = vfs_for_miniaudio();

        ma_engine_config ma_config = ma_engine_config_init();
        ma_config.channels = 2;
        ma_config.sampleRate = 44100;
        ma_config.pResourceManagerVFS = miniaudio_vfs;
        ma_result res = ma_engine_init(&ma_config, &audio_engine);
        if (res != MA_SUCCESS) {
            gBase.fatal_error("failed to initialize audio engine");
        }
    }
}

void Sound::sound_fini() {
    GarbageCollect();
    garbage_sounds.trash();

    ma_engine_uninit(&audio_engine);
    mem_free(miniaudio_vfs);
}

int Sound::OnUpdate(Event evt) { return 0; }

int Sound::OnPostUpdate(Event evt) {
    PROFILE_FUNC();

    Array<SoundIndex> &sounds = garbage_sounds;
    for (u64 i = 0; i < sounds.len;) {
        SoundSource *v = sounds[i].ptr;

        if (v->end) {
            assert(v->zombie);
            sound_trash(v);
            sounds[i] = sounds[sounds.len - 1];
            sounds.len--;
        } else {
            i++;
        }
    }
    return 0;
}

void Sound::GarbageCollect() {
    for (SoundIndex &sound : garbage_sounds) {
        sound_trash(sound.ptr);
    }
    garbage_sounds.len = 0;
}
