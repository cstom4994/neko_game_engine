#include "engine/system.h"

#include <stdbool.h>

#include "console.h"
#include "edit.h"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/game.h"
#include "engine/input.h"
#include "engine/os.h"
#include "engine/physics.h"
#include "engine/prefab.h"
#include "engine/script.h"
#include "engine/sound.h"
#include "engine/sprite.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "engine/ui.h"
#include "test/keyboard_controlled.h"

static void _key_down(KeyCode key) {
    gui_key_down(key);
    script_key_down(key);
}
static void _key_up(KeyCode key) {
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
static void _mouse_move(CVec2 pos) { script_mouse_move(pos); }
static void _scroll(CVec2 scroll) { script_scroll(scroll); }

void system_init() {

#if defined(_DEBUG)
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, "./gamedir");
    MountResult mount_luacode = vfs_mount(NEKO_PACKS::LUACODE, "./source/game");
#else
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, nullptr);
    MountResult mount_luacode = vfs_mount(NEKO_PACKS::LUACODE, "code.zip");
#endif

    input_init();
    entity_init();
    transform_init();
    camera_init();
    texture_init();
    sprite_init();
    gui_init();
    console_init();
    sound_init();
    physics_init();
    edit_init();
    script_init();

    lua_State *L = ENGINE_LUA();

    g_app->is_fused.store(mount.is_fused);

    if (!g_app->error_mode.load() && mount.ok && mount_luacode.ok) {
        asset_load_kind(AssetKind_LuaRef, "main.lua", nullptr);
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
    g_app->debug_on = luax_boolean_field(L, -1, "debug_on", true);
    String game_proxy = luax_opt_string_field(L, -1, "game_proxy", "default");

    lua_pop(L, 1);  // conf table

    // 刷新状态
    neko_api_set_window_size(g_app->width, g_app->height);

    // run main.lua
    errcheck(luaL_loadfile(L, "./source/game/script/main.lua"));
    errcheck(luax_pcall_nothrow(L, 0, 0));

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
}

void system_fini() {
    edit_fini();
    script_fini();
    physics_fini();
    sound_fini();
    console_fini();
    sprite_fini();
    gui_fini();
    texture_fini();
    camera_fini();
    transform_fini();
    entity_fini();
    input_fini();

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        // if (g_app->default_font != nullptr) {
        //     g_app->default_font->trash();
        //     mem_free(g_app->default_font);
        // }

        for (Sound *sound : g_app->garbage_sounds) {
            sound->trash();
        }
        g_app->garbage_sounds.trash();

        assets_shutdown();
    }
}

void system_update_all() {
    edit_clear();

    timing_update();

    texture_update();
    scratch_update();

    script_update_all();

    keyboard_controlled_update_all();

    physics_update_all();
    transform_update_all();
    camera_update_all();
    gui_update_all();
    sprite_update_all();
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
    edit_draw_all();
    physics_draw_all();
    gui_draw_all();
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
