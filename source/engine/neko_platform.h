
#ifndef NEKO_PF_H
#define NEKO_PF_H

#include <time.h>

#include "engine/neko.h"
#include "engine/neko_math.h"

/*========================
// NEKO_PLATFORM
========================*/

#include <glfw/glfw3.h>

#if defined(_WIN32) || defined(_WIN64) || defined(NEKO_PF_WIN)

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

typedef unsigned __int64 tick_t;
#elif defined(NEKO_PF_APPLE)
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
typedef u64 tick_t;

#ifdef __APPLE__
#include <objc/message.h>
#include <objc/objc.h>
#endif

// #define GLFW_EXPOSE_NATIVE_COCOA
// #include <GLFW/glfw3native.h>

#else
#include <pthread.h>
#include <stdint.h>
#include <syscall.h>
typedef u64 tick_t;
#endif

// 一些 winapi 调用可能会失败 但我们没有任何已知的方法来“修复”该问题
// 其中一些调用不是致命的（例如如果无法移动窗口）因此我们只需断言 DEBUG_CHECK
#define DEBUG_CHECK(R) \
    if (!(R)) NEKO_ASSERT(false)

/*============================================================
// Platform Time
============================================================*/

typedef double deltatime_t;

typedef struct neko_pf_time_t {
    f32 max_fps;
    f32 elapsed;
    f32 previous;
    f32 update;
    f32 render;
    f32 delta;
    f32 frame;
} neko_pf_time_t;

/*============================================================
// Platform UUID
============================================================*/

#define NEKO_UUID_STR_SIZE_CONSTANT 32

// 33 characters, all set to 0
#define neko_uuid_temp_str_buffer() \
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

typedef struct neko_uuid_t {
    u8 bytes[16];
} neko_uuid_t;

/*============================================================
// Platform Window
============================================================*/

#define NEKO_WINDOW_FLAGS_NO_RESIZE 0x01
#define NEKO_WINDOW_FLAGS_FULLSCREEN 0x02

// Should have an internal resource cache of window handles (controlled by the platform api)

typedef struct neko_pf_running_desc_t {
    const_str title;
    u32 width;
    u32 height;
    u32 flags;
    u32 num_samples;  // 多重样本 0为禁用
    u32 monitor_index;
    b32 vsync;
    f32 frame_rate;
    b32 hdpi;
    bool center;
    bool running_background;
} neko_pf_running_desc_t;

typedef struct neko_pf_window_t {
    void *hndl;
    neko_vec2 framebuffer_size;
    neko_vec2 window_size;
    neko_vec2 window_position;
    b32 focus;
} neko_pf_window_t;

typedef struct neko_memory_info_t {
    u64 virtual_memory_used;
    u64 physical_memory_used;
    u64 peak_virtual_memory_used;
    u64 peak_physical_memory_used;
    u64 gpu_memory_used;   // kb
    u64 gpu_total_memory;  // kb
} neko_memory_info_t;

typedef enum neko_pf_cursor {
    NEKO_PF_CURSOR_ARROW,
    NEKO_PF_CURSOR_IBEAM,
    NEKO_PF_CURSOR_SIZE_NW_SE,
    NEKO_PF_CURSOR_SIZE_NE_SW,
    NEKO_PF_CURSOR_SIZE_NS,
    NEKO_PF_CURSOR_SIZE_WE,
    NEKO_PF_CURSOR_SIZE_ALL,
    NEKO_PF_CURSOR_HAND,
    NEKO_PF_CURSOR_NO,
    NEKO_PF_CURSOR_COUNT
} neko_pf_cursor;

typedef enum neko_pf_keycode {
    NEKO_KEYCODE_INVALID,
    NEKO_KEYCODE_SPACE,
    NEKO_KEYCODE_APOSTROPHE, /* ' */
    NEKO_KEYCODE_COMMA,      /* , */
    NEKO_KEYCODE_MINUS,      /* - */
    NEKO_KEYCODE_PERIOD,     /* . */
    NEKO_KEYCODE_SLASH,      /* / */
    NEKO_KEYCODE_0,
    NEKO_KEYCODE_1,
    NEKO_KEYCODE_2,
    NEKO_KEYCODE_3,
    NEKO_KEYCODE_4,
    NEKO_KEYCODE_5,
    NEKO_KEYCODE_6,
    NEKO_KEYCODE_7,
    NEKO_KEYCODE_8,
    NEKO_KEYCODE_9,
    NEKO_KEYCODE_SEMICOLON, /* ; */
    NEKO_KEYCODE_EQUAL,     /* = */
    NEKO_KEYCODE_A,
    NEKO_KEYCODE_B,
    NEKO_KEYCODE_C,
    NEKO_KEYCODE_D,
    NEKO_KEYCODE_E,
    NEKO_KEYCODE_F,
    NEKO_KEYCODE_G,
    NEKO_KEYCODE_H,
    NEKO_KEYCODE_I,
    NEKO_KEYCODE_J,
    NEKO_KEYCODE_K,
    NEKO_KEYCODE_L,
    NEKO_KEYCODE_M,
    NEKO_KEYCODE_N,
    NEKO_KEYCODE_O,
    NEKO_KEYCODE_P,
    NEKO_KEYCODE_Q,
    NEKO_KEYCODE_R,
    NEKO_KEYCODE_S,
    NEKO_KEYCODE_T,
    NEKO_KEYCODE_U,
    NEKO_KEYCODE_V,
    NEKO_KEYCODE_W,
    NEKO_KEYCODE_X,
    NEKO_KEYCODE_Y,
    NEKO_KEYCODE_Z,
    NEKO_KEYCODE_LEFT_BRACKET,  /* [ */
    NEKO_KEYCODE_BACKSLASH,     /* \ */
    NEKO_KEYCODE_RIGHT_BRACKET, /* ] */
    NEKO_KEYCODE_GRAVE_ACCENT,  /* ` */
    NEKO_KEYCODE_WORLD_1,       /* non-US #1 */
    NEKO_KEYCODE_WORLD_2,       /* non-US #2 */
    NEKO_KEYCODE_ESC,
    NEKO_KEYCODE_ENTER,
    NEKO_KEYCODE_TAB,
    NEKO_KEYCODE_BACKSPACE,
    NEKO_KEYCODE_INSERT,
    NEKO_KEYCODE_DELETE,
    NEKO_KEYCODE_RIGHT,
    NEKO_KEYCODE_LEFT,
    NEKO_KEYCODE_DOWN,
    NEKO_KEYCODE_UP,
    NEKO_KEYCODE_PAGE_UP,
    NEKO_KEYCODE_PAGE_DOWN,
    NEKO_KEYCODE_HOME,
    NEKO_KEYCODE_END,
    NEKO_KEYCODE_CAPS_LOCK,
    NEKO_KEYCODE_SCROLL_LOCK,
    NEKO_KEYCODE_NUM_LOCK,
    NEKO_KEYCODE_PRINT_SCREEN,
    NEKO_KEYCODE_PAUSE,
    NEKO_KEYCODE_F1,
    NEKO_KEYCODE_F2,
    NEKO_KEYCODE_F3,
    NEKO_KEYCODE_F4,
    NEKO_KEYCODE_F5,
    NEKO_KEYCODE_F6,
    NEKO_KEYCODE_F7,
    NEKO_KEYCODE_F8,
    NEKO_KEYCODE_F9,
    NEKO_KEYCODE_F10,
    NEKO_KEYCODE_F11,
    NEKO_KEYCODE_F12,
    NEKO_KEYCODE_F13,
    NEKO_KEYCODE_F14,
    NEKO_KEYCODE_F15,
    NEKO_KEYCODE_F16,
    NEKO_KEYCODE_F17,
    NEKO_KEYCODE_F18,
    NEKO_KEYCODE_F19,
    NEKO_KEYCODE_F20,
    NEKO_KEYCODE_F21,
    NEKO_KEYCODE_F22,
    NEKO_KEYCODE_F23,
    NEKO_KEYCODE_F24,
    NEKO_KEYCODE_F25,
    NEKO_KEYCODE_KP_0,
    NEKO_KEYCODE_KP_1,
    NEKO_KEYCODE_KP_2,
    NEKO_KEYCODE_KP_3,
    NEKO_KEYCODE_KP_4,
    NEKO_KEYCODE_KP_5,
    NEKO_KEYCODE_KP_6,
    NEKO_KEYCODE_KP_7,
    NEKO_KEYCODE_KP_8,
    NEKO_KEYCODE_KP_9,
    NEKO_KEYCODE_KP_DECIMAL,
    NEKO_KEYCODE_KP_DIVIDE,
    NEKO_KEYCODE_KP_MULTIPLY,
    NEKO_KEYCODE_KP_SUBTRACT,
    NEKO_KEYCODE_KP_ADD,
    NEKO_KEYCODE_KP_ENTER,
    NEKO_KEYCODE_KP_EQUAL,
    NEKO_KEYCODE_LEFT_SHIFT,
    NEKO_KEYCODE_LEFT_CONTROL,
    NEKO_KEYCODE_LEFT_ALT,
    NEKO_KEYCODE_LEFT_SUPER,
    NEKO_KEYCODE_RIGHT_SHIFT,
    NEKO_KEYCODE_RIGHT_CONTROL,
    NEKO_KEYCODE_RIGHT_ALT,
    NEKO_KEYCODE_RIGHT_SUPER,
    NEKO_KEYCODE_MENU,
    NEKO_KEYCODE_COUNT
} neko_pf_keycode;

#define NEKO_KEYCODE_LCTRL NEKO_KEYCODE_LEFT_CONTROL
#define NEKO_KEYCODE_RCTRL NEKO_KEYCODE_RIGHT_CONTROL
#define NEKO_KEYCODE_LSHIFT NEKO_KEYCODE_LEFT_SHIFT
#define NEKO_KEYCODE_RSHIFT NEKO_KEYCODE_RIGHT_SHIFT
#define NEKO_KEYCODE_LALT NEKO_KEYCODE_LEFT_ALT
#define NEKO_KEYCODE_RALT NEKO_KEYCODE_RIGHT_ALT

typedef enum neko_pf_mouse_button_code { NEKO_MOUSE_LBUTTON, NEKO_MOUSE_RBUTTON, NEKO_MOUSE_MBUTTON, NEKO_MOUSE_BUTTON_CODE_COUNT } neko_pf_mouse_button_code;

typedef struct neko_pf_mouse_t {
    b32 button_map[NEKO_MOUSE_BUTTON_CODE_COUNT];
    b32 prev_button_map[NEKO_MOUSE_BUTTON_CODE_COUNT];
    neko_vec2 position;
    neko_vec2 delta;
    neko_vec2 wheel;
    b32 moved_this_frame;
    b32 locked;
} neko_pf_mouse_t;

enum {
    NEKO_PF_GAMEPAD_BUTTON_A = 0x00,
    NEKO_PF_GAMEPAD_BUTTON_B,
    NEKO_PF_GAMEPAD_BUTTON_X,
    NEKO_PF_GAMEPAD_BUTTON_Y,
    NEKO_PF_GAMEPAD_BUTTON_LBUMPER,
    NEKO_PF_GAMEPAD_BUTTON_RBUMPER,
    NEKO_PF_GAMEPAD_BUTTON_BACK,
    NEKO_PF_GAMEPAD_BUTTON_START,
    NEKO_PF_GAMEPAD_BUTTON_GUIDE,
    NEKO_PF_GAMEPAD_BUTTON_LTHUMB,
    NEKO_PF_GAMEPAD_BUTTON_RTHUMB,
    NEKO_PF_GAMEPAD_BUTTON_DPUP,
    NEKO_PF_GAMEPAD_BUTTON_DPRIGHT,
    NEKO_PF_GAMEPAD_BUTTON_DPDOWN,
    NEKO_PF_GAMEPAD_BUTTON_DPLEFT,
    NEKO_PF_GAMEPAD_BUTTON_COUNT
};

#define NEKO_PF_GAMEPAD_BUTTON_LAST NEKO_PF_GAMEPAD_BUTTON_DPLEFT
#define NEKO_PF_GAMEPAD_BUTTON_CROSS NEKO_PF_GAMEPAD_BUTTON_A
#define NEKO_PF_GAMEPAD_BUTTON_CIRCLE NEKO_PF_GAMEPAD_BUTTON_B
#define NEKO_PF_GAMEPAD_BUTTON_SQUARE NEKO_PF_GAMEPAD_BUTTON_X
#define NEKO_PF_GAMEPAD_BUTTON_TRIANGLE NEKO_PF_GAMEPAD_BUTTON_Y

enum {
    NEKO_PF_JOYSTICK_AXIS_LEFT_X = 0x00,
    NEKO_PF_JOYSTICK_AXIS_LEFT_Y,
    NEKO_PF_JOYSTICK_AXIS_RIGHT_X,
    NEKO_PF_JOYSTICK_AXIS_RIGHT_Y,
    NEKO_PF_JOYSTICK_AXIS_LTRIGGER,
    NEKO_PF_JOYSTICK_AXIS_RTRIGGER,
    NEKO_PF_JOYSTICK_AXIS_COUNT
};

#define NEKO_PF_GAMEPAD_MAX 16

typedef struct neko_pf_gamepad_t {
    int16_t buttons[NEKO_PF_GAMEPAD_BUTTON_COUNT];
    f32 axes[NEKO_PF_JOYSTICK_AXIS_COUNT];
    int16_t present;
} neko_pf_gamepad_t;

#define NEKO_PF_MAX_TOUCH 5

typedef struct neko_pf_touchpoint_t {
    size_t id;
    neko_vec2 position;
    neko_vec2 delta;
    uint16_t changed;
    uint16_t pressed;
    uint16_t down;
} neko_pf_touchpoint_t;

typedef struct neko_pf_touch_t {
    neko_pf_touchpoint_t points[NEKO_PF_MAX_TOUCH];
    uint16_t size;  // Current number of touches active
} neko_pf_touch_t;

typedef struct neko_pf_input_t {
    b32 key_map[NEKO_KEYCODE_COUNT];
    b32 prev_key_map[NEKO_KEYCODE_COUNT];
    neko_pf_mouse_t mouse;
    neko_pf_touch_t touch;
    neko_pf_gamepad_t gamepads[NEKO_PF_GAMEPAD_MAX];
} neko_pf_input_t;

// Enumeration of all platform types
typedef enum neko_pf_type { NEKO_PF_TYPE_UNKNOWN = 0, NEKO_PF_TYPE_WINDOWS, NEKO_PF_TYPE_LINUX, NEKO_PF_TYPE_MAC, NEKO_PF_TYPE_WEB } neko_pf_type;

typedef enum neko_opengl_compatibility_flags {
    NEKO_OPENGL_COMPATIBILITY_FLAGS_LEGACY = 0,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_CORE = 1 << 1,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_COMPATIBILITY = 1 << 2,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_FORWARD = 1 << 3,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_ES = 1 << 4,
} neko_opengl_compatibility_flags;

typedef enum neko_pf_event_type { NEKO_PF_EVENT_MOUSE, NEKO_PF_EVENT_KEY, NEKO_PF_EVENT_TEXT, NEKO_PF_EVENT_WINDOW, NEKO_PF_EVENT_TOUCH, NEKO_PF_EVENT_APP } neko_pf_event_type;

typedef enum neko_pf_key_modifier_type {
    NEKO_PF_KEY_MODIFIER_NONE = 0x00,
    NEKO_PF_KEY_MODIFIER_SHIFT = 0x01,
    NEKO_PF_KEY_MODIFIER_CONTROL = 0x02,
    NEKO_PF_KEY_MODIFIER_ALT = 0x04
} neko_pf_key_modifier_type;

typedef enum neko_pf_key_action_type { NEKO_PF_KEY_PRESSED, NEKO_PF_KEY_DOWN, NEKO_PF_KEY_RELEASED } neko_pf_key_action_type;

typedef struct neko_pf_key_event_t {
    s32 codepoint;
    s32 scancode;
    neko_pf_keycode keycode;
    neko_pf_key_action_type action;
    neko_pf_key_modifier_type modifier;
} neko_pf_key_event_t;

typedef enum neko_pf_mousebutton_action_type {
    NEKO_PF_MOUSE_BUTTON_PRESSED,
    NEKO_PF_MOUSE_BUTTON_DOWN,
    NEKO_PF_MOUSE_BUTTON_RELEASED,
    NEKO_PF_MOUSE_MOVE,
    NEKO_PF_MOUSE_ENTER,
    NEKO_PF_MOUSE_LEAVE,
    NEKO_PF_MOUSE_WHEEL
} neko_pf_mousebutton_action_type;

typedef struct neko_pf_mouse_event_t {
    s32 codepoint;
    union {
        neko_pf_mouse_button_code button;
        neko_vec2 wheel;
        neko_vec2 move;
    };
    neko_pf_mousebutton_action_type action;
} neko_pf_mouse_event_t;

typedef enum neko_pf_window_action_type {
    NEKO_PF_WINDOW_RESIZE,
    NEKO_PF_WINDOW_LOSE_FOCUS,
    NEKO_PF_WINDOW_GAIN_FOCUS,
    NEKO_PF_WINDOW_CREATE,
    NEKO_PF_WINDOW_DESTROY
} neko_pf_window_action_type;

typedef struct neko_pf_window_event_t {
    u32 hndl;
    neko_pf_window_action_type action;
} neko_pf_window_event_t;

typedef struct neko_pf_text_event_t {
    u32 codepoint;
} neko_pf_text_event_t;

typedef enum neko_pf_touch_action_type { NEKO_PF_TOUCH_DOWN, NEKO_PF_TOUCH_UP, NEKO_PF_TOUCH_MOVE, NEKO_PF_TOUCH_CANCEL } neko_pf_touch_action_type;

typedef struct neko_pf_point_event_data_t {
    uintptr_t id;
    neko_vec2 position;
    uint16_t changed;
} neko_pf_point_event_data_t;

typedef struct neko_pf_touch_event_t {
    neko_pf_touch_action_type action;
    neko_pf_point_event_data_t point;
} neko_pf_touch_event_t;

typedef enum neko_pf_app_action_type { NEKO_PF_APP_START, NEKO_PF_APP_PAUSE, NEKO_PF_APP_RESUME, NEKO_PF_APP_STOP } neko_pf_app_action_type;

typedef struct neko_pf_app_event_t {
    neko_pf_app_action_type action;
} neko_pf_app_event_t;

// Platform events
typedef struct neko_pf_event_t {
    neko_pf_event_type type;
    union {
        neko_pf_key_event_t key;
        neko_pf_mouse_event_t mouse;
        neko_pf_window_event_t window;
        neko_pf_touch_event_t touch;
        neko_pf_app_event_t app;
        neko_pf_text_event_t text;
    };
    u32 idx;
} neko_pf_event_t;

// Necessary function pointer typedefs
typedef void (*neko_dropped_files_callback_t)(void *, s32 count, const char **file_paths);
typedef void (*neko_window_close_callback_t)(void *);
typedef void (*neko_character_callback_t)(void *, u32 code_point);
typedef void (*neko_framebuffer_resize_callback_t)(void *, s32 width, s32 height);

/*===============================================================================================
// Platform Interface
===============================================================================================*/

struct neko_pf_interface_s;

typedef struct neko_pf_t {
    // Time
    neko_pf_time_t time;

    // Input
    neko_pf_input_t input;

    // Window data and handles
    neko_slot_array(neko_pf_window_t) windows;

    // Events that user can poll
    neko_dyn_array(neko_pf_event_t) events;

    // Cursors
    void *cursors[NEKO_PF_CURSOR_COUNT];

    // Specific user data (for custom implementations)
    void *user_data;

    // Optional api for stable access across .dll boundaries
    struct neko_pf_interface_s *api;

} neko_pf_t;

/*===============================================================================================
// Platform API
===============================================================================================*/

/* == Platform Default API == */

// Platform Create / Destroy
NEKO_API_DECL neko_pf_t *neko_pf_create();
NEKO_API_DECL void neko_pf_destroy(neko_pf_t *platform);

// Platform Init / Update / Shutdown
NEKO_API_DECL void neko_pf_update(neko_pf_t *platform);  // Update platform layer

// Platform Util
NEKO_API_DECL const neko_pf_time_t *neko_pf_time();
NEKO_API_DECL f32 neko_pf_delta_time();
NEKO_API_DECL f32 neko_pf_frame_time();

// Platform UUID
NEKO_API_DECL neko_uuid_t neko_pf_uuid_generate();
NEKO_API_DECL void neko_pf_uuid_to_string(char *temp_buffer, const neko_uuid_t *uuid);  // Expects a temp buffer with at least 32 bytes
NEKO_API_DECL u32 neko_pf_uuid_hash(const neko_uuid_t *uuid);

// Platform Input
NEKO_API_DECL neko_pf_input_t *neko_pf_input();
NEKO_API_DECL void neko_pf_update_input(neko_pf_input_t *input);
NEKO_API_DECL void neko_pf_press_key(neko_pf_keycode code);
NEKO_API_DECL void neko_pf_release_key(neko_pf_keycode code);
NEKO_API_DECL bool neko_pf_was_key_down(neko_pf_keycode code);
NEKO_API_DECL void neko_pf_press_mouse_button(neko_pf_mouse_button_code code);
NEKO_API_DECL void neko_pf_release_mouse_button(neko_pf_mouse_button_code code);
NEKO_API_DECL bool neko_pf_was_mouse_down(neko_pf_mouse_button_code code);
NEKO_API_DECL void neko_pf_press_touch(u32 idx);
NEKO_API_DECL void neko_pf_release_touch(u32 idx);
NEKO_API_DECL bool neko_pf_was_touch_down(u32 idx);

NEKO_API_DECL bool neko_pf_key_pressed(neko_pf_keycode code);
NEKO_API_DECL bool neko_pf_key_down(neko_pf_keycode code);
NEKO_API_DECL bool neko_pf_key_released(neko_pf_keycode code);
NEKO_API_DECL bool neko_pf_touch_pressed(u32 idx);
NEKO_API_DECL bool neko_pf_touch_down(u32 idx);
NEKO_API_DECL bool neko_pf_touch_released(u32 idx);
NEKO_API_DECL bool neko_pf_mouse_pressed(neko_pf_mouse_button_code code);
NEKO_API_DECL bool neko_pf_mouse_down(neko_pf_mouse_button_code code);
NEKO_API_DECL bool neko_pf_mouse_released(neko_pf_mouse_button_code code);
NEKO_API_DECL neko_vec2 neko_pf_mouse_deltav();
NEKO_API_DECL void neko_pf_mouse_delta(f32 *x, f32 *y);
NEKO_API_DECL neko_vec2 neko_pf_mouse_positionv();
NEKO_API_DECL void neko_pf_mouse_position(s32 *x, s32 *y);
NEKO_API_DECL void neko_pf_mouse_wheel(f32 *x, f32 *y);
NEKO_API_DECL neko_vec2 neko_pf_mouse_wheelv();
NEKO_API_DECL bool neko_pf_mouse_moved();
NEKO_API_DECL bool neko_pf_mouse_locked();
NEKO_API_DECL neko_vec2 neko_pf_touch_deltav(u32 idx);
NEKO_API_DECL void neko_pf_touch_delta(u32 idx, f32 *x, f32 *y);
NEKO_API_DECL neko_vec2 neko_pf_touch_positionv(u32 idx);
NEKO_API_DECL void neko_pf_touch_position(u32 idx, f32 *x, f32 *y);

// Platform Events
NEKO_API_DECL bool neko_pf_poll_events(neko_pf_event_t *evt, b32 consume);
NEKO_API_DECL void neko_pf_add_event(neko_pf_event_t *evt);

// Platform Window
NEKO_API_DECL u32 neko_pf_window_create(const neko_pf_running_desc_t *desc);
NEKO_API_DECL u32 neko_pf_main_window();

typedef struct neko_pf_file_stats_t {
    u64 modified_time;
    u64 creation_time;
    u64 access_time;
} neko_pf_file_stats_t;

// Platform File IO (this all needs to be made available for impl rewrites)
NEKO_API_DECL char *neko_pf_read_file_contents_default_impl(const char *file_path, const char *mode, size_t *sz);
NEKO_API_DECL neko_result neko_pf_write_file_contents_default_impl(const char *file_path, const char *mode, void *data, size_t data_size);
NEKO_API_DECL bool neko_pf_file_exists_default_impl(const char *file_path);
NEKO_API_DECL bool neko_pf_dir_exists_default_impl(const char *dir_path);
NEKO_API_DECL s32 neko_pf_mkdir_default_impl(const char *dir_path, s32 opt);
NEKO_API_DECL s32 neko_pf_file_size_in_bytes_default_impl(const char *file_path);
NEKO_API_DECL void neko_pf_file_extension_default_impl(char *buffer, size_t buffer_sz, const char *file_path);
NEKO_API_DECL s32 neko_pf_file_delete_default_impl(const char *file_path);
NEKO_API_DECL s32 neko_pf_file_copy_default_impl(const char *src_path, const char *dst_path);
NEKO_API_DECL s32 neko_pf_file_compare_time(u64 time_a, u64 time_b);
NEKO_API_DECL neko_pf_file_stats_t neko_pf_file_stats(const char *file_path);
NEKO_API_DECL void *neko_pf_library_load_default_impl(const char *lib_path);
NEKO_API_DECL void neko_pf_library_unload_default_impl(void *lib);
NEKO_API_DECL void *neko_pf_library_proc_address_default_impl(void *lib, const char *func);
NEKO_API_DECL int neko_pf_chdir_default_impl(const char *path);

// Default file implementations
#define neko_pf_read_file_contents neko_pf_read_file_contents_default_impl
#define neko_pf_write_file_contents neko_pf_write_file_contents_default_impl
#define neko_pf_file_exists neko_pf_file_exists_default_impl
#define neko_pf_dir_exists neko_pf_dir_exists_default_impl
#define neko_pf_mkdir neko_pf_mkdir_default_impl
#define neko_pf_file_size_in_bytes neko_pf_file_size_in_bytes_default_impl
#define neko_pf_file_extension neko_pf_file_extension_default_impl
#define neko_pf_file_delete neko_pf_file_delete_default_impl
#define neko_pf_file_copy neko_pf_file_copy_default_impl
#define neko_pf_library_load neko_pf_library_load_default_impl
#define neko_pf_library_unload neko_pf_library_unload_default_impl
#define neko_pf_library_proc_address neko_pf_library_proc_address_default_impl
#define neko_pf_chdir neko_pf_chdir_default_impl

#if defined(NEKO_PF_LINUX) || defined(NEKO_PF_APPLE)
#define neko_fopen(filePath, mode) fopen(filePath, mode)
#define neko_fseek(file, offset, whence) fseeko(file, offset, whence)
#define neko_ftell(file) ftello(file)
#elif defined(NEKO_PF_WIN)
static inline FILE *neko_fopen(const char *filePath, const char *mode) {
    FILE *file;
    errno_t error = fopen_s(&file, filePath, mode);
    if (error != 0) return NULL;
    return file;
}
#define neko_fseek(file, offset, whence) _fseeki64(file, offset, whence)
#define neko_ftell(file) _ftelli64(file)
#else
#error Unsupported operating system
#endif
#define neko_fclose(file) fclose(file)

/* == Platform Dependent API == */

NEKO_API_DECL void neko_pf_init(neko_pf_t *platform);      // Initialize platform layer
NEKO_API_DECL void neko_pf_shutdown(neko_pf_t *platform);  // Shutdown platform layer

NEKO_API_DECL void neko_pf_update_internal(neko_pf_t *platform);

// Platform Util
NEKO_API_DECL double neko_pf_elapsed_time();  // Returns time in ms since initialization of platform
NEKO_API_DECL void neko_pf_sleep(f32 ms);     // Sleeps platform for time in ms

// Platform Video
NEKO_API_DECL void neko_pf_enable_vsync(s32 enabled);

// Platform Input
NEKO_API_DECL void neko_pf_process_input(neko_pf_input_t *input);
NEKO_API_DECL u32 neko_pf_key_to_codepoint(neko_pf_keycode code);
NEKO_API_DECL neko_pf_keycode neko_pf_codepoint_to_key(u32 code);
NEKO_API_DECL void neko_pf_mouse_set_position(u32 handle, f32 x, f32 y);
NEKO_API_DECL void neko_pf_lock_mouse(u32 handle, b32 lock);

NEKO_API_DECL neko_pf_window_t neko_pf_window_create_internal(const neko_pf_running_desc_t *desc);
NEKO_API_DECL void neko_pf_window_swap_buffer(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_window_sizev(u32 handle);
NEKO_API_DECL void neko_pf_window_size(u32 handle, u32 *width, u32 *height);
NEKO_API_DECL u32 neko_pf_window_width(u32 handle);
NEKO_API_DECL u32 neko_pf_window_height(u32 handle);
NEKO_API_DECL b32 neko_pf_window_fullscreen(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_window_positionv(u32 handle);
NEKO_API_DECL void neko_pf_window_position(u32 handle, u32 *x, u32 *y);
NEKO_API_DECL void neko_pf_set_window_title(u32 handle, const_str title);
NEKO_API_DECL void neko_pf_set_window_size(u32 handle, u32 width, u32 height);
NEKO_API_DECL void neko_pf_set_window_sizev(u32 handle, neko_vec2 v);
NEKO_API_DECL void neko_pf_set_window_fullscreen(u32 handle, b32 fullscreen);
NEKO_API_DECL void neko_pf_set_window_position(u32 handle, u32 x, u32 y);
NEKO_API_DECL void neko_pf_set_window_positionv(u32 handle, neko_vec2 v);
NEKO_API_DECL void neko_pf_set_cursor(u32 handle, neko_pf_cursor cursor);
NEKO_API_DECL void *neko_pf_raw_window_handle(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_framebuffer_sizev(u32 handle);
NEKO_API_DECL void neko_pf_framebuffer_size(u32 handle, u32 *w, u32 *h);
NEKO_API_DECL u32 neko_pf_framebuffer_width(u32 handle);
NEKO_API_DECL u32 neko_pf_framebuffer_height(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_monitor_sizev(u32 id);
NEKO_API_DECL void neko_pf_window_set_clipboard(u32 handle, const_str str);
NEKO_API_DECL const_str neko_pf_window_get_clipboard(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_get_window_dpi();

// Platform callbacks
NEKO_API_DECL void neko_pf_set_framebuffer_resize_callback(u32 handle, neko_framebuffer_resize_callback_t cb);
NEKO_API_DECL void neko_pf_set_dropped_files_callback(u32 handle, neko_dropped_files_callback_t cb);
NEKO_API_DECL void neko_pf_set_window_close_callback(u32 handle, neko_window_close_callback_t cb);
NEKO_API_DECL void neko_pf_set_character_callback(u32 handle, neko_character_callback_t cb);

NEKO_API_DECL void *neko_pf_hwnd();
NEKO_API_DECL neko_memory_info_t neko_pf_memory_info();
NEKO_API_DECL void neko_pf_msgbox(const_str msg);

// Platform Internal
NEKO_API_DECL void neko_pf_symbol_handler_init();
NEKO_API_DECL const_str neko_pf_stacktrace();

/*============================================================
// Platform Native
============================================================*/

static inline u64 neko_get_thread_id() {
#if defined(NEKO_PF_WIN)
    return (u64)GetCurrentThreadId();
#elif defined(NEKO_PF_LINUX)
    return (u64)syscall(SYS_gettid);
#elif defined(NEKO_PF_APPLE)
    return (mach_port_t)pthread_mach_thread_np(pthread_self());
#else
#error "Unsupported platform!"
#endif
}

/*============================================================
// Mutex and Locker
============================================================*/

#if defined(NEKO_PF_WIN)

NEKO_INLINE void __native_debug_output(const char *msg) { OutputDebugStringA(msg); }

#endif

/*===================================
// Thread
===================================*/

#define THREAD_HAS_ATOMIC 1
#define THREAD_STACK_SIZE_DEFAULT (0)
#define THREAD_SIGNAL_WAIT_INFINITE (-1)
#define THREAD_QUEUE_WAIT_INFINITE (-1)

union neko_thread_mutex_t {
    void *align;
    char data[64];
};

union neko_thread_signal_t {
    void *align;
    char data[116];
};

union neko_thread_atomic_int_t {
    void *align;
    long i;
};

union neko_thread_atomic_ptr_t {
    void *ptr;
};

union neko_thread_timer_t {
    void *data;
    char d[8];
};

typedef void *neko_thread_id_t;
NEKO_API_DECL neko_thread_id_t thread_current_thread_id(void);
NEKO_API_DECL void thread_yield(void);
NEKO_API_DECL void thread_set_high_priority(void);
NEKO_API_DECL void thread_exit(int return_code);

typedef void *neko_thread_ptr_t;
NEKO_API_DECL neko_thread_ptr_t thread_init(int (*thread_proc)(void *), void *user_data, char const *name, int stack_size);
NEKO_API_DECL void thread_term(neko_thread_ptr_t thread);
NEKO_API_DECL int thread_join(neko_thread_ptr_t thread);
NEKO_API_DECL int thread_detach(neko_thread_ptr_t thread);

typedef union neko_thread_mutex_t neko_thread_mutex_t;
NEKO_API_DECL void thread_mutex_init(neko_thread_mutex_t *mutex);
NEKO_API_DECL void thread_mutex_term(neko_thread_mutex_t *mutex);
NEKO_API_DECL void thread_mutex_lock(neko_thread_mutex_t *mutex);
NEKO_API_DECL void thread_mutex_unlock(neko_thread_mutex_t *mutex);

typedef union neko_thread_signal_t neko_thread_signal_t;
NEKO_API_DECL void thread_signal_init(neko_thread_signal_t *signal);
NEKO_API_DECL void thread_signal_term(neko_thread_signal_t *signal);
NEKO_API_DECL void thread_signal_raise(neko_thread_signal_t *signal);
NEKO_API_DECL int thread_signal_wait(neko_thread_signal_t *signal, int timeout_ms);

#if THREAD_HAS_ATOMIC
typedef union neko_thread_atomic_int_t neko_thread_atomic_int_t;
NEKO_API_DECL int thread_atomic_int_load(neko_thread_atomic_int_t *atomic);
NEKO_API_DECL void thread_atomic_int_store(neko_thread_atomic_int_t *atomic, int desired);
NEKO_API_DECL int thread_atomic_int_inc(neko_thread_atomic_int_t *atomic);
NEKO_API_DECL int thread_atomic_int_dec(neko_thread_atomic_int_t *atomic);
NEKO_API_DECL int thread_atomic_int_add(neko_thread_atomic_int_t *atomic, int value);
NEKO_API_DECL int thread_atomic_int_sub(neko_thread_atomic_int_t *atomic, int value);
NEKO_API_DECL int thread_atomic_int_swap(neko_thread_atomic_int_t *atomic, int desired);
NEKO_API_DECL int thread_atomic_int_compare_and_swap(neko_thread_atomic_int_t *atomic, int expected, int desired);

typedef union neko_thread_atomic_ptr_t neko_thread_atomic_ptr_t;
NEKO_API_DECL void *thread_atomic_ptr_load(neko_thread_atomic_ptr_t *atomic);
NEKO_API_DECL void thread_atomic_ptr_store(neko_thread_atomic_ptr_t *atomic, void *desired);
NEKO_API_DECL void *thread_atomic_ptr_swap(neko_thread_atomic_ptr_t *atomic, void *desired);
NEKO_API_DECL void *thread_atomic_ptr_compare_and_swap(neko_thread_atomic_ptr_t *atomic, void *expected, void *desired);
#endif

typedef union neko_thread_timer_t neko_thread_timer_t;
NEKO_API_DECL void thread_timer_init(neko_thread_timer_t *timer);
NEKO_API_DECL void thread_timer_term(neko_thread_timer_t *timer);
NEKO_API_DECL void thread_timer_wait(neko_thread_timer_t *timer, u64 nanoseconds);

typedef void *neko_thread_tls_t;
NEKO_API_DECL neko_thread_tls_t thread_tls_create(void);
NEKO_API_DECL void thread_tls_destroy(neko_thread_tls_t tls);
NEKO_API_DECL void thread_tls_set(neko_thread_tls_t tls, void *value);
NEKO_API_DECL void *thread_tls_get(neko_thread_tls_t tls);

#endif  // !NEKO_PF_H
