#include "engine/system.h"

#include <stdbool.h>

#include "console.h"
#include "edit.h"
#include "engine/api.hpp"
#include "engine/base.h"
#include "engine/batch.h"
#include "engine/camera.h"
#include "engine/game.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/lua_util.h"
#include "engine/os.h"
#include "engine/physics.h"
#include "engine/prefab.h"
#include "engine/script.h"
#include "engine/sound.h"
#include "engine/sprite.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "engine/ui.h"

static void load_all_lua_scripts(lua_State *L) {
    PROFILE_FUNC();

    Array<String> files = {};
    neko_defer({
        for (String str : files) {
            mem_free(str.data);
        }
        files.trash();
    });

    bool ok = false;

#if defined(_DEBUG)
    ok = vfs_list_all_files(NEKO_PACKS::LUACODE, &files);
#else
    ok = vfs_list_all_files(NEKO_PACKS::GAMEDATA, &files);
#endif

    if (!ok) {
        neko_panic("failed to list all files");
    }
    std::qsort(files.data, files.len, sizeof(String), [](const void *a, const void *b) -> int {
        String *lhs = (String *)a;
        String *rhs = (String *)b;
        return std::strcmp(lhs->data, rhs->data);
    });

    for (String file : files) {
        if (file.starts_with("script/") && !file.ends_with("nekomain.lua") && file.ends_with(".lua")) {
            asset_load_kind(AssetKind_LuaRef, file, nullptr);
        } else {
        }
    }
}

static void _key_down(KeyCode key, int scancode, int mode) {
    gui_key_down(key);
    script_key_down(key);
}
static void _key_up(KeyCode key, int scancode, int mode) {
    gui_key_up(key);
    script_key_up(key);
}
static void _char_down(unsigned int c) { gui_char_down(c); }
static void _mouse_down(MouseCode mouse) {
    gui_mouse_down(mouse);
    script_mouse_down(mouse);
}
static void _mouse_up(MouseCode mouse) {
    gui_mouse_up(mouse);
    script_mouse_up(mouse);
}
static void _mouse_move(LuaVec2 pos) { script_mouse_move(pos); }
static void _scroll(LuaVec2 scroll) { script_scroll(scroll); }

void system_init() {
    PROFILE_FUNC();

#if defined(_DEBUG)
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, "../gamedir");
    MountResult mount_luacode = vfs_mount(NEKO_PACKS::LUACODE, "../source/game");
#else
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, nullptr);
    MountResult mount_luacode = {true};
#endif

    script_init();

    lua_State *L = ENGINE_LUA();

    g_app->is_fused.store(mount.is_fused);

    if (!g_app->error_mode.load() && mount.ok && mount_luacode.ok) {
        asset_load_kind(AssetKind_LuaRef, "conf.lua", nullptr);
    }

    // if (!g_app->error_mode.load()) {
    //     luax_neko_get(L, "arg");
    //     lua_createtable(L, game_get_argc() - 1, 0);
    //     for (i32 i = 1; i < game_get_argc(); i++) {
    //         lua_pushstring(L, game_get_argv()[i]);
    //         lua_rawseti(L, -2, i);
    //     }
    //     if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
    //         lua_pop(L, 1);
    //     }
    // }

    lua_newtable(L);
    i32 conf_table = lua_gettop(L);

    if (!g_app->error_mode.load()) {
        luax_neko_get(L, "conf");
        lua_pushvalue(L, conf_table);
        luax_pcall(L, 1, 0);
    }

    g_app->win_console = g_app->win_console || luax_boolean_field(L, -1, "win_console", true);

    bool hot_reload = luax_boolean_field(L, -1, "hot_reload", true);
    bool startup_load_scripts = luax_boolean_field(L, -1, "startup_load_scripts", true);
    bool fullscreen = luax_boolean_field(L, -1, "fullscreen", false);
    lua_Number reload_interval = luax_opt_number_field(L, -1, "reload_interval", 0.1);
    lua_Number swap_interval = luax_opt_number_field(L, -1, "swap_interval", 1);
    lua_Number target_fps = luax_opt_number_field(L, -1, "target_fps", 0);
    g_app->width = luax_opt_number_field(L, -1, "window_width", 800);
    g_app->height = luax_opt_number_field(L, -1, "window_height", 600);
    String title = luax_opt_string_field(L, -1, "window_title", "NekoEngine");
    String imgui_font = luax_opt_string_field(L, -1, "imgui_font", "");
    g_app->lite_init_path = luax_opt_string_field(L, -1, "lite_init_path", "");
    g_app->debug_on = luax_boolean_field(L, -1, "debug_on", true);
    String game_proxy = luax_opt_string_field(L, -1, "game_proxy", "default");

    lua_pop(L, 1);  // conf table

    // 刷新状态
    game_set_window_size(vec2(g_app->width, g_app->height));

    if (fnv1a(game_proxy) == "default"_hash) {
        neko::neko_lua_run_string(L, R"lua(
            function neko.before_quit() game_proxy_before_quit() end
            function neko.start(arg) game_proxy_start(arg) end
            function neko.frame(dt) game_proxy_frame(dt) end
        )lua");
        console_log("using default game proxy");
    }

    CVAR(conf_hot_reload, hot_reload);
    CVAR(conf_startup_load_scripts, startup_load_scripts);
    CVAR(conf_fullscreen, fullscreen);
    CVAR(conf_reload_interval, reload_interval);
    CVAR(conf_swap_interval, swap_interval);
    CVAR(conf_target_fps, target_fps);
    CVAR(conf_width, g_app->width);
    CVAR(conf_height, g_app->height);
    CVAR(conf_title, title);
    CVAR(conf_imgui_font, imgui_font);
    CVAR(conf_debug_on, g_app->debug_on);
    CVAR(conf_game_proxy, game_proxy);

    g_render = gfx_create();
    gfx_init(g_render);

    input_init();
    entity_init();
    transform_init();
    camera_init();
    batch_init(6000);
    sprite_init();
    gui_init();
    imgui_init();
    console_init();
    sound_init();
    physics_init();
    edit_init();

    g_app->hot_reload_enabled.store(mount.can_hot_reload && hot_reload);
    g_app->reload_interval.store((u32)(reload_interval * 1000));

    if (!g_app->error_mode.load() && startup_load_scripts && mount.ok && mount_luacode.ok) {
        load_all_lua_scripts(L);
    }

    // run main.lua
    // String main_lua = {};
    // vfs_read_entire_file(&main_lua, "script/nekomain.lua");
    // neko_defer(mem_free(main_lua.data));
    // errcheck(luaL_loadbuffer(L, main_lua.cstr(), main_lua.len, "nekomain.lua"));
    // errcheck(luax_pcall_nothrow(L, 0, 0));
    asset_load_kind(AssetKind_LuaRef, "script/nekomain.lua", nullptr);

    // fire init event
    script_push_event("init");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    input_add_key_down_callback(_key_down);
    input_add_key_up_callback(_key_up);
    input_add_char_down_callback(_char_down);
    input_add_mouse_down_callback(_mouse_down);
    input_add_mouse_up_callback(_mouse_up);
    input_add_mouse_move_callback(_mouse_move);
    input_add_scroll_callback(_scroll);

    if (target_fps != 0) {
        timing_instance.target_ticks = 1000000000 / target_fps;
    }

#ifdef NEKO_IS_WIN32
    if (!g_app->win_console) {
        FreeConsole();
    }
#endif
}

void system_fini() {
    PROFILE_FUNC();

    edit_fini();
    script_fini();
    physics_fini();
    sound_fini();
    console_fini();
    sprite_fini();
    batch_fini();
    imgui_fini();
    gui_fini();
    camera_fini();
    transform_fini();
    entity_fini();
    input_fini();

    gfx_fini(g_render);

    neko_dyn_array_free(g_app->shader_array);

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        // if (g_app->default_font != nullptr) {
        //     g_app->default_font->trash();
        //     mem_free(g_app->default_font);
        // }

#if NEKO_AUDIO == 1
        for (Sound *sound : g_app->garbage_sounds) {
            sound->trash();
        }
        g_app->garbage_sounds.trash();
#endif

        assets_shutdown();
    }
}

void system_update_all() {
    edit_clear();

    timing_update();

    scratch_update();

    script_update_all();

    keyboard_controlled_update_all();

    physics_update_all();
    transform_update_all();
    camera_update_all();
    gui_update_all();
    sprite_update_all();
    batch_update_all();
    sound_update_all();

    edit_update_all();
    script_post_update_all();
    physics_post_update_all();

    entity_update_all();

    gui_event_clear();
}

void system_draw_all() {
    script_draw_all();
    sprite_draw_all();
    batch_draw_all();
    edit_draw_all();
    physics_draw_all();
    gui_draw_all();

    neko_check_gl_error();
}

// 以相同的顺序 保存/加载
static void _saveload_all(void *s, bool save) {
#define saveload(sys)               \
    if (save)                       \
        sys##_save_all((Store *)s); \
    else                            \
        sys##_load_all((Store *)s)

    entity_load_all_begin();

    saveload(entity);
    saveload(prefab);
    saveload(timing);
    saveload(transform);
    saveload(camera);
    saveload(sprite);
    saveload(physics);
    saveload(gui);
    saveload(edit);
    saveload(sound);

    saveload(keyboard_controlled);

    saveload(script);

    entity_load_all_end();
}

void system_save_all(Store *s) { _saveload_all(s, true); }

void system_load_all(Store *s) { _saveload_all(s, false); }
