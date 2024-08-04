#include "engine/neko_system.h"

#include <stdbool.h>

#include "console.h"
#include "edit.h"
#include "engine/neko_base.h"
#include "engine/neko_camera.h"
#include "engine/neko_game.h"
#include "engine/neko_input.h"
#include "engine/neko_os.h"
#include "engine/neko_physics.h"
#include "engine/neko_prefab.h"
#include "engine/neko_script.h"
#include "engine/neko_sound.h"
#include "engine/neko_sprite.h"
#include "engine/neko_texture.h"
#include "engine/neko_transform.h"
#include "engine/neko_ui.h"
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
