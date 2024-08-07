#include "engine/sound.h"

#include "engine/game.h"
#include "engine/luax.h"

#if NEKO_AUDIO == 1

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

    res = ma_sound_init_from_file(&g_app->audio_engine, cpath.data, 0, nullptr, nullptr, &sound->ma);
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
        g_app->garbage_sounds.push(sound);
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

#endif

#if 0

#include <gorilla/ga.h>
#include <gorilla/gau.h>

typedef struct Sound Sound;
struct Sound
{
    EntityPoolElem pool_elem;

    char *path;
    ga_Handle *handle;
    gau_SampleSourceLoop *loop_src;
    bool finish_destroy;
    bool loop;
};

static EntityPool *pool;

static gau_Manager *mgr;
static ga_Mixer *mixer;
static ga_StreamManager *stream_mgr;

// -------------------------------------------------------------------------

static void _release(Sound *sound)
{
    // path
    mem_free(sound->path);
    sound->path = NULL;

    // handle
    if (sound->handle)
        ga_handle_destroy(sound->handle);
    sound->handle = NULL;
}

// figure out format from path -- uses extension
static const char *_format(const char *path)
{
    const char *dot = NULL, *c;
    for (c = path; *c; ++c)
        if (*c == '.')
            dot = c;
    if (dot)
        return dot + 1;
    error("unknown sound format for file '%s'", path);
}

// update gorilla loop state to actually reflect sound->loop
static void _update_loop(Sound *sound)
{
    if (sound->loop)
        gau_sample_source_loop_set(sound->loop_src, -1, 0);
    else
        gau_sample_source_loop_clear(sound->loop_src);
}

/* precondition: path must be good or NULL, handle must be good or NULL,
   doesn't allocate new path string if sound->path == path */
static void _set_path(Sound *sound, const char *path)
{
    bool prev_playing;
    const char *format;
    ga_Sound *src;
    ga_Handle *handle;
    gau_SampleSourceLoop *loop_src;

    // currently playing?
    prev_playing = sound->handle && ga_handle_playing(sound->handle);

    // try loading sound
    format = _format(path);
    handle = NULL;
    if (!strcmp(format, "ogg"))
        handle = gau_create_handle_buffered_file(mixer, stream_mgr, path,
                                                 format, NULL, NULL,
                                                 &loop_src);
    else if ((src = gau_load_sound_file(path, format)))
        handle = gau_create_handle_sound(mixer, src, NULL, NULL, &loop_src);
    if (!handle)
        error("couldn't load sound from path '%s', check path and format",
              path);
    error_assert(loop_src, "handle must have valid loop source");

    // set new
    _release(sound);
    if (sound->path != path)
    {
        sound->path = mem_alloc(strlen(path) + 1);
        strcpy(sound->path, path);
    }
    sound->handle = handle;

    // update loop
    sound->loop_src = loop_src;
    _update_loop(sound);

    // play new sound if old one was playing
    if (prev_playing)
        ga_handle_play(sound->handle);
}

void sound_add(Entity ent)
{
    Sound *sound;

    if (entitypool_get(pool, ent))
        return;

    sound = entitypool_add(pool, ent);
    sound->path = NULL;
    sound->handle = NULL;
    sound->loop_src = NULL;
    sound->loop = false;
    sound->finish_destroy = true;

    _set_path(sound, data_path("default.wav"));
}

void sound_remove(Entity ent)
{
    Sound *sound;

    sound = entitypool_get(pool, ent);
    if (!sound)
        return;

    _release(sound);
    entitypool_remove(pool, ent);
}

bool sound_has(Entity ent)
{
    return entitypool_get(pool, ent) != NULL;
}

void sound_set_path(Entity ent, const char *path)
{
    Sound *sound;

    sound =  entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    _set_path(sound, path);
}
const char *sound_get_path(Entity ent)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    return sound->path;
}

void sound_set_playing(Entity ent, bool playing)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    if (playing)
    {
        if (ga_handle_finished(sound->handle))
            _set_path(sound, sound->path); // can't reuse finished handles
        ga_handle_play(sound->handle);
    }
    else
        ga_handle_stop(sound->handle);
}
bool sound_get_playing(Entity ent)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    return ga_handle_playing(sound->handle);
}

void sound_set_seek(Entity ent, int seek)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    ga_handle_seek(sound->handle, seek);
}
int sound_get_seek(Entity ent)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    return ga_handle_tell(sound->handle, GA_TELL_PARAM_CURRENT);
}

void sound_set_finish_destroy(Entity ent, bool finish_destroy)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    sound->finish_destroy = finish_destroy;
}
bool sound_get_finish_destroy(Entity ent)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    return sound->finish_destroy;
}

void sound_set_loop(Entity ent, bool loop)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    sound->loop = loop;
    _update_loop(sound);
}
bool sound_get_loop(Entity ent)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    return sound->loop;
}

void sound_set_gain(Entity ent, Scalar gain)
{
    Sound *sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    ga_handle_setParamf(sound->handle, GA_HANDLE_PARAM_GAIN, gain);
}
Scalar sound_get_gain(Entity ent)
{
    Sound *sound;
    gc_float32 v;

    sound = entitypool_get(pool, ent);
    error_assert(sound, "entity must be in sound system");
    error_assert(sound->handle, "sound must be valid");
    ga_handle_getParamf(sound->handle, GA_HANDLE_PARAM_GAIN, &v);
    return v;
}

// -------------------------------------------------------------------------

void sound_init()
{
    gc_initialize(NULL);
    mgr = gau_manager_create();
    mixer = gau_manager_mixer(mgr);
    stream_mgr = gau_manager_streamManager(mgr);

    pool = entitypool_new(Sound);
}
void sound_fini()
{
    Sound *sound;

    entitypool_foreach(sound, pool)
        _release(sound);
    entitypool_free(pool);

    gau_manager_destroy(mgr);
    gc_shutdown();
}

void sound_update_all()
{
    Sound *sound;

    // destroy finished sounds that have finish_destroy set
    entitypool_foreach(sound, pool)
        if (sound->finish_destroy
            && sound->handle && ga_handle_finished(sound->handle))
            entity_destroy(sound->pool_elem.ent);

    entitypool_remove_destroyed(pool, sound_remove);

    gau_manager_update(mgr);
}

void sound_save_all(Store *s)
{
    Store *t, *sound_s;
    Sound *sound;
    int seek;
    bool playing;
    Scalar gain;

    if (store_child_save(&t, "sound", s))
        entitypool_save_foreach(sound, sound_s, pool, "pool", t)
        {
            string_save((const char **) &sound->path, "path", sound_s);
            bool_save(&sound->finish_destroy, "finish_destroy", sound_s);
            bool_save(&sound->loop, "loop", sound_s);

            playing = ga_handle_playing(sound->handle);
            bool_save(&playing, "playing", sound_s);

            seek = ga_handle_tell(sound->handle, GA_TELL_PARAM_CURRENT);
            int_save(&seek, "seek", sound_s);

            ga_handle_getParamf(sound->handle, GA_HANDLE_PARAM_GAIN, &gain);
            scalar_save(&gain, "gain", sound_s);
        }
}
void sound_load_all(Store *s)
{
    Store *t, *sound_s;
    Sound *sound;
    int seek;
    bool playing;
    char *path;
    Scalar gain;

    if (store_child_load(&t, "sound", s))
        entitypool_load_foreach(sound, sound_s, pool, "pool", t)
        {
            string_load(&path, "path", NULL, sound_s);
            bool_load(&sound->finish_destroy, "finish_destroy", false, sound_s);
            bool_load(&sound->loop, "loop", false, sound_s);

            sound->path = NULL;
            sound->handle = NULL;
            _set_path(sound, path);

            bool_load(&playing, "playing", false, sound_s);
            if (playing)
                ga_handle_play(sound->handle);

            int_load(&seek, "seek", 0, sound_s);
            ga_handle_seek(sound->handle, seek);

            scalar_load(&gain, "gain", 1, sound_s);
            ga_handle_setParamf(sound->handle, GA_HANDLE_PARAM_GAIN, gain);
        }
}

#endif

void sound_init() {
    PROFILE_FUNC();

    {
#if NEKO_AUDIO == 1
        PROFILE_BLOCK("miniaudio");

        g_app->miniaudio_vfs = vfs_for_miniaudio();

        ma_engine_config ma_config = ma_engine_config_init();
        ma_config.channels = 2;
        ma_config.sampleRate = 44100;
        ma_config.pResourceManagerVFS = g_app->miniaudio_vfs;
        ma_result res = ma_engine_init(&ma_config, &g_app->audio_engine);
        if (res != MA_SUCCESS) {
            fatal_error("failed to initialize audio engine");
        }
#elif NEKO_AUDIO == 2

#endif
    }
}

void sound_fini() {}
void sound_update_all() {}
void sound_save_all(Store *s) {}
void sound_load_all(Store *s) {}