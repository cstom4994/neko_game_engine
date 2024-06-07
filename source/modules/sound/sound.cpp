
#include "engine/neko_api.hpp"
#include "engine/neko_luabind.hpp"
#include "neko_sound.h"

// mt_sound

static ma_sound* sound_ma(lua_State* L) {
    neko_lua_sound* sound = *(neko_lua_sound**)luaL_checkudata(L, 1, "mt_sound");
    return &sound->ma;
}

static int mt_sound_gc(lua_State* L) {
    neko_lua_sound* sound = *(neko_lua_sound**)luaL_checkudata(L, 1, "mt_sound");

    if (ma_sound_at_end(&sound->ma)) {
        sound_fini(sound);
        neko_safe_free(sound);
    } else {
        sound->zombie = true;
        // g_app->garbage_sounds.push(sound); // TODO:: 建立 garbage_sounds 每帧清理
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

    neko_lua_sound* sound = sound_load(NULL, str.data);
    if (sound == nullptr) {
        return 0;
    }

    luax_ptr_userdata(L, sound, "mt_sound");
    return 1;
}