
#ifndef NEKO_PF_H
#define NEKO_PF_H

#include <string>

#include "engine/neko.h"
#include "engine/neko_math.h"

/*========================
// NEKO_PLATFORM
========================*/

#include <GLFW/glfw3.h>

#if defined(_WIN32) || defined(_WIN64) || defined(NEKO_IS_WIN32)

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
typedef unsigned __int64 tick_t;

#elif defined(NEKO_IS_APPLE)

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
#include <sys/syscall.h>
#include <syscall.h>
#include <unistd.h>

typedef u64 tick_t;

extern long int syscall(long int __sysno, ...);

#endif

// 一些 winapi 调用可能会失败 但我们没有任何已知的方法来“修复”该问题
// 其中一些调用不是致命的（例如如果无法移动窗口）因此我们只需断言 DEBUG_CHECK
#define DEBUG_CHECK(R) \
    if (!(R)) NEKO_ASSERT(false)

/*============================================================
// Platform Time
============================================================*/

typedef double deltatime_t;

typedef struct neko_os_time_t {
    f32 max_fps;
    f32 elapsed;
    f32 previous;
    f32 update;
    f32 render;
    f32 delta;
    f32 frame;
} neko_os_time_t;

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

typedef struct neko_os_running_desc_t {
    const_str title;
    u32 width;
    u32 height;
    u32 flags;
    u32 num_samples;  // 多重样本 0为禁用
    u32 monitor_index;
    bool vsync;
    f32 frame_rate;
    bool hdpi;
    bool center;
    bool running_background;
} neko_os_running_desc_t;

typedef struct neko_pf_window_t {
    void *hndl;
    neko_vec2 framebuffer_size;
    neko_vec2 window_size;
    neko_vec2 window_position;
    bool focus;
} neko_pf_window_t;

typedef struct neko_memory_info_t {
    u64 virtual_memory_used;
    u64 physical_memory_used;
    u64 peak_virtual_memory_used;
    u64 peak_physical_memory_used;
    u64 gpu_memory_used;   // kb
    u64 gpu_total_memory;  // kb
} neko_memory_info_t;

typedef enum neko_os_cursor {
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
} neko_os_cursor;

typedef enum neko_os_keycode {
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
} neko_os_keycode;

#define NEKO_KEYCODE_LCTRL NEKO_KEYCODE_LEFT_CONTROL
#define NEKO_KEYCODE_RCTRL NEKO_KEYCODE_RIGHT_CONTROL
#define NEKO_KEYCODE_LSHIFT NEKO_KEYCODE_LEFT_SHIFT
#define NEKO_KEYCODE_RSHIFT NEKO_KEYCODE_RIGHT_SHIFT
#define NEKO_KEYCODE_LALT NEKO_KEYCODE_LEFT_ALT
#define NEKO_KEYCODE_RALT NEKO_KEYCODE_RIGHT_ALT

typedef enum neko_os_mouse_button_code { NEKO_MOUSE_LBUTTON, NEKO_MOUSE_RBUTTON, NEKO_MOUSE_MBUTTON, NEKO_MOUSE_BUTTON_CODE_COUNT } neko_os_mouse_button_code;

typedef struct neko_os_mouse_t {
    bool button_map[NEKO_MOUSE_BUTTON_CODE_COUNT];
    bool prev_button_map[NEKO_MOUSE_BUTTON_CODE_COUNT];
    neko_vec2 position;
    neko_vec2 delta;
    neko_vec2 wheel;
    bool moved_this_frame;
    bool locked;
} neko_os_mouse_t;

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

typedef struct neko_os_gamepad_t {
    int16_t buttons[NEKO_PF_GAMEPAD_BUTTON_COUNT];
    f32 axes[NEKO_PF_JOYSTICK_AXIS_COUNT];
    int16_t present;
} neko_os_gamepad_t;

#define NEKO_PF_MAX_TOUCH 5

typedef struct neko_os_touchpoint_t {
    size_t id;
    neko_vec2 position;
    neko_vec2 delta;
    uint16_t changed;
    uint16_t pressed;
    uint16_t down;
} neko_os_touchpoint_t;

typedef struct neko_os_touch_t {
    neko_os_touchpoint_t points[NEKO_PF_MAX_TOUCH];
    uint16_t size;  // Current number of touches active
} neko_os_touch_t;

typedef struct neko_os_input_t {
    bool key_map[NEKO_KEYCODE_COUNT];
    bool prev_key_map[NEKO_KEYCODE_COUNT];
    neko_os_mouse_t mouse;
    neko_os_touch_t touch;
    neko_os_gamepad_t gamepads[NEKO_PF_GAMEPAD_MAX];
} neko_os_input_t;

// Enumeration of all platform types
typedef enum neko_os_type { NEKO_PF_TYPE_UNKNOWN = 0, NEKO_PF_TYPE_WINDOWS, NEKO_PF_TYPE_LINUX, NEKO_PF_TYPE_MAC, NEKO_PF_TYPE_WEB } neko_os_type;

typedef enum neko_opengl_compatibility_flags {
    NEKO_OPENGL_COMPATIBILITY_FLAGS_LEGACY = 0,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_CORE = 1 << 1,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_COMPATIBILITY = 1 << 2,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_FORWARD = 1 << 3,
    NEKO_OPENGL_COMPATIBILITY_FLAGS_ES = 1 << 4,
} neko_opengl_compatibility_flags;

typedef enum neko_os_event_type { NEKO_PF_EVENT_MOUSE, NEKO_PF_EVENT_KEY, NEKO_PF_EVENT_TEXT, NEKO_PF_EVENT_WINDOW, NEKO_PF_EVENT_TOUCH, NEKO_PF_EVENT_APP } neko_os_event_type;

typedef enum neko_os_key_modifier_type {
    NEKO_PF_KEY_MODIFIER_NONE = 0x00,
    NEKO_PF_KEY_MODIFIER_SHIFT = 0x01,
    NEKO_PF_KEY_MODIFIER_CONTROL = 0x02,
    NEKO_PF_KEY_MODIFIER_ALT = 0x04
} neko_os_key_modifier_type;

typedef enum neko_os_key_action_type { NEKO_PF_KEY_PRESSED, NEKO_PF_KEY_DOWN, NEKO_PF_KEY_RELEASED } neko_os_key_action_type;

typedef struct neko_os_key_event_t {
    s32 codepoint;
    s32 scancode;
    neko_os_keycode keycode;
    neko_os_key_action_type action;
    neko_os_key_modifier_type modifier;
} neko_os_key_event_t;

typedef enum neko_os_mousebutton_action_type {
    NEKO_PF_MOUSE_BUTTON_PRESSED,
    NEKO_PF_MOUSE_BUTTON_DOWN,
    NEKO_PF_MOUSE_BUTTON_RELEASED,
    NEKO_PF_MOUSE_MOVE,
    NEKO_PF_MOUSE_ENTER,
    NEKO_PF_MOUSE_LEAVE,
    NEKO_PF_MOUSE_WHEEL
} neko_os_mousebutton_action_type;

typedef struct neko_os_mouse_event_t {
    s32 codepoint;
    union {
        neko_os_mouse_button_code button;
        neko_vec2 wheel;
        neko_vec2 move;
    };
    neko_os_mousebutton_action_type action;
} neko_os_mouse_event_t;

typedef enum neko_pf_window_action_type {
    NEKO_IS_WIN32DOW_RESIZE,
    NEKO_IS_WIN32DOW_LOSE_FOCUS,
    NEKO_IS_WIN32DOW_GAIN_FOCUS,
    NEKO_IS_WIN32DOW_CREATE,
    NEKO_IS_WIN32DOW_DESTROY
} neko_pf_window_action_type;

typedef struct neko_pf_window_event_t {
    u32 hndl;
    neko_pf_window_action_type action;
} neko_pf_window_event_t;

typedef struct neko_os_text_event_t {
    u32 codepoint;
} neko_os_text_event_t;

typedef enum neko_os_touch_action_type { NEKO_PF_TOUCH_DOWN, NEKO_PF_TOUCH_UP, NEKO_PF_TOUCH_MOVE, NEKO_PF_TOUCH_CANCEL } neko_os_touch_action_type;

typedef struct neko_os_point_event_data_t {
    uintptr_t id;
    neko_vec2 position;
    uint16_t changed;
} neko_os_point_event_data_t;

typedef struct neko_os_touch_event_t {
    neko_os_touch_action_type action;
    neko_os_point_event_data_t point;
} neko_os_touch_event_t;

typedef enum neko_os_app_action_type { NEKO_PF_APP_START, NEKO_PF_APP_PAUSE, NEKO_PF_APP_RESUME, NEKO_PF_APP_STOP } neko_os_app_action_type;

typedef struct neko_os_app_event_t {
    neko_os_app_action_type action;
} neko_os_app_event_t;

// Platform events
typedef struct neko_os_event_t {
    neko_os_event_type type;
    union {
        neko_os_key_event_t key;
        neko_os_mouse_event_t mouse;
        neko_pf_window_event_t window;
        neko_os_touch_event_t touch;
        neko_os_app_event_t app;
        neko_os_text_event_t text;
    };
    u32 idx;
} neko_os_event_t;

// Necessary function pointer typedefs
typedef void (*neko_dropped_files_callback_t)(void *, s32 count, const char **file_paths);
typedef void (*neko_window_close_callback_t)(void *);
typedef void (*neko_character_callback_t)(void *, u32 code_point);
typedef void (*neko_framebuffer_resize_callback_t)(void *, s32 width, s32 height);

/*===============================================================================================
// Platform Interface
===============================================================================================*/

struct neko_os_interface_s;

typedef struct neko_os_t {
    // Time
    neko_os_time_t time;

    // Input
    neko_os_input_t input;

    // Window data and handles
    neko_slot_array(neko_pf_window_t) windows;

    // Events that user can poll
    neko_dyn_array(neko_os_event_t) events;

    // Cursors
    void *cursors[NEKO_PF_CURSOR_COUNT];

    // Specific user data (for custom implementations)
    void *user_data;

    // Optional api for stable access across .dll boundaries
    struct neko_os_interface_s *api;

} neko_os_t;

/*===============================================================================================
// Platform API
===============================================================================================*/

/* == Platform Default API == */

// Platform Create / Destroy
NEKO_API_DECL neko_os_t *neko_os_create();
NEKO_API_DECL void neko_os_fini(neko_os_t *platform);

// Platform Init / Update / Shutdown
NEKO_API_DECL void neko_os_update(neko_os_t *platform);  // Update platform layer

// Platform Util
NEKO_API_DECL const neko_os_time_t *neko_os_time();
NEKO_API_DECL f32 neko_os_delta_time();
NEKO_API_DECL f32 neko_os_frame_time();

// Platform UUID
NEKO_API_DECL neko_uuid_t neko_os_uuid_generate();
NEKO_API_DECL void neko_os_uuid_to_string(char *temp_buffer, const neko_uuid_t *uuid);  // Expects a temp buffer with at least 32 bytes
NEKO_API_DECL u32 neko_os_uuid_hash(const neko_uuid_t *uuid);

// Platform Input
NEKO_API_DECL neko_os_input_t *neko_os_input();
NEKO_API_DECL void neko_os_update_input(neko_os_input_t *input);
NEKO_API_DECL void neko_os_press_key(neko_os_keycode code);
NEKO_API_DECL void neko_os_release_key(neko_os_keycode code);
NEKO_API_DECL bool neko_os_was_key_down(neko_os_keycode code);
NEKO_API_DECL void neko_os_press_mouse_button(neko_os_mouse_button_code code);
NEKO_API_DECL void neko_os_release_mouse_button(neko_os_mouse_button_code code);
NEKO_API_DECL bool neko_os_was_mouse_down(neko_os_mouse_button_code code);
NEKO_API_DECL void neko_os_press_touch(u32 idx);
NEKO_API_DECL void neko_os_release_touch(u32 idx);
NEKO_API_DECL bool neko_os_was_touch_down(u32 idx);

NEKO_API_DECL bool neko_os_key_pressed(neko_os_keycode code);
NEKO_API_DECL bool neko_os_key_down(neko_os_keycode code);
NEKO_API_DECL bool neko_os_key_released(neko_os_keycode code);
NEKO_API_DECL bool neko_os_touch_pressed(u32 idx);
NEKO_API_DECL bool neko_os_touch_down(u32 idx);
NEKO_API_DECL bool neko_os_touch_released(u32 idx);
NEKO_API_DECL bool neko_os_mouse_pressed(neko_os_mouse_button_code code);
NEKO_API_DECL bool neko_os_mouse_down(neko_os_mouse_button_code code);
NEKO_API_DECL bool neko_os_mouse_released(neko_os_mouse_button_code code);
NEKO_API_DECL neko_vec2 neko_os_mouse_deltav();
NEKO_API_DECL void neko_os_mouse_delta(f32 *x, f32 *y);
NEKO_API_DECL neko_vec2 neko_os_mouse_positionv();
NEKO_API_DECL void neko_os_mouse_position(s32 *x, s32 *y);
NEKO_API_DECL void neko_os_mouse_wheel(f32 *x, f32 *y);
NEKO_API_DECL neko_vec2 neko_os_mouse_wheelv();
NEKO_API_DECL bool neko_os_mouse_moved();
NEKO_API_DECL bool neko_os_mouse_locked();
NEKO_API_DECL neko_vec2 neko_os_touch_deltav(u32 idx);
NEKO_API_DECL void neko_os_touch_delta(u32 idx, f32 *x, f32 *y);
NEKO_API_DECL neko_vec2 neko_os_touch_positionv(u32 idx);
NEKO_API_DECL void neko_os_touch_position(u32 idx, f32 *x, f32 *y);

// Platform Events
NEKO_API_DECL bool neko_os_poll_events(neko_os_event_t *evt, bool consume);
NEKO_API_DECL void neko_os_add_event(neko_os_event_t *evt);

// Platform Window
NEKO_API_DECL u32 neko_pf_window_create(const neko_os_running_desc_t *desc);
NEKO_API_DECL u32 neko_os_main_window();

typedef struct neko_os_file_stats_t {
    u64 modified_time;
    u64 creation_time;
    u64 access_time;
} neko_os_file_stats_t;

// Platform File IO (this all needs to be made available for impl rewrites)
NEKO_API_DECL char *neko_os_read_file_contents(const char *file_path, const char *mode, size_t *sz);
NEKO_API_DECL neko_result neko_os_write_file_contents(const char *file_path, const char *mode, void *data, size_t data_size);
NEKO_API_DECL bool neko_os_file_exists(const char *file_path);
NEKO_API_DECL bool neko_os_dir_exists(const char *dir_path);
NEKO_API_DECL s32 neko_os_mkdir(const char *dir_path, s32 opt);
NEKO_API_DECL s32 neko_os_file_size_in_bytes(const char *file_path);
NEKO_API_DECL void neko_os_file_extension(char *buffer, size_t buffer_sz, const char *file_path);
NEKO_API_DECL s32 neko_os_file_delete(const char *file_path);
NEKO_API_DECL s32 neko_os_file_copy(const char *src_path, const char *dst_path);
NEKO_API_DECL s32 neko_os_file_compare_time(u64 time_a, u64 time_b);
NEKO_API_DECL neko_os_file_stats_t neko_os_file_stats(const char *file_path);
NEKO_API_DECL void *neko_os_library_load(const char *lib_path);
NEKO_API_DECL void neko_os_library_unload(void *lib);
NEKO_API_DECL void *neko_os_library_proc_address(void *lib, const char *func);
NEKO_API_DECL int neko_os_chdir(const char *path);

#if defined(NEKO_IS_APPLE)
#define neko_fopen(filePath, mode) fopen(filePath, mode)
#define neko_fseek(file, offset, whence) fseeko(file, offset, whence)
#define neko_ftell(file) ftello(file)
#elif defined(NEKO_IS_LINUX)
#define neko_fopen(filePath, mode) fopen(filePath, mode)
#define neko_fseek(file, offset, whence) fseek(file, offset, whence)
#define neko_ftell(file) ftell(file)
#elif defined(NEKO_IS_WIN32)
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

NEKO_API_DECL void neko_os_init(neko_os_t *platform);  // Initialize platform layer

NEKO_API_DECL void neko_os_update_internal(neko_os_t *platform);

// Platform Util
NEKO_API_DECL double neko_os_elapsed_time();  // Returns time in ms since initialization of platform
NEKO_API_DECL void neko_os_sleep(f32 ms);     // Sleeps platform for time in ms

// Platform Video
NEKO_API_DECL void neko_os_enable_vsync(s32 enabled);

// Platform Input
NEKO_API_DECL void neko_os_process_input(neko_os_input_t *input);
NEKO_API_DECL u32 neko_os_key_to_codepoint(neko_os_keycode code);
NEKO_API_DECL neko_os_keycode neko_os_codepoint_to_key(u32 code);
NEKO_API_DECL void neko_os_mouse_set_position(u32 handle, f32 x, f32 y);
NEKO_API_DECL void neko_os_lock_mouse(u32 handle, bool lock);

NEKO_API_DECL neko_pf_window_t neko_pf_window_create_internal(const neko_os_running_desc_t *desc);
NEKO_API_DECL void neko_pf_window_swap_buffer(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_window_sizev(u32 handle);
NEKO_API_DECL void neko_pf_window_size(u32 handle, u32 *width, u32 *height);
NEKO_API_DECL u32 neko_pf_window_width(u32 handle);
NEKO_API_DECL u32 neko_pf_window_height(u32 handle);
NEKO_API_DECL bool neko_pf_window_fullscreen(u32 handle);
NEKO_API_DECL neko_vec2 neko_pf_window_positionv(u32 handle);
NEKO_API_DECL void neko_pf_window_position(u32 handle, u32 *x, u32 *y);
NEKO_API_DECL void neko_os_set_window_title(u32 handle, const_str title);
NEKO_API_DECL void neko_os_set_window_size(u32 handle, u32 width, u32 height);
NEKO_API_DECL void neko_os_set_window_sizev(u32 handle, neko_vec2 v);
NEKO_API_DECL void neko_os_set_window_fullscreen(u32 handle, bool fullscreen);
NEKO_API_DECL void neko_os_set_window_position(u32 handle, u32 x, u32 y);
NEKO_API_DECL void neko_os_set_window_positionv(u32 handle, neko_vec2 v);
NEKO_API_DECL void neko_os_set_cursor(u32 handle, neko_os_cursor cursor);
NEKO_API_DECL void *neko_os_raw_window_handle(u32 handle);
NEKO_API_DECL neko_vec2 neko_os_framebuffer_sizev(u32 handle);
NEKO_API_DECL void neko_os_framebuffer_size(u32 handle, u32 *w, u32 *h);
NEKO_API_DECL u32 neko_os_framebuffer_width(u32 handle);
NEKO_API_DECL u32 neko_os_framebuffer_height(u32 handle);
NEKO_API_DECL neko_vec2 neko_os_monitor_sizev(u32 id);
NEKO_API_DECL void neko_pf_window_set_clipboard(u32 handle, const_str str);
NEKO_API_DECL const_str neko_pf_window_get_clipboard(u32 handle);
NEKO_API_DECL neko_vec2 neko_os_get_window_dpi();

// Platform callbacks
NEKO_API_DECL void neko_os_set_framebuffer_resize_callback(u32 handle, neko_framebuffer_resize_callback_t cb);
NEKO_API_DECL void neko_os_set_dropped_files_callback(u32 handle, neko_dropped_files_callback_t cb);
NEKO_API_DECL void neko_os_set_window_close_callback(u32 handle, neko_window_close_callback_t cb);
NEKO_API_DECL void neko_os_set_character_callback(u32 handle, neko_character_callback_t cb);

NEKO_API_DECL void *neko_os_hwnd();
NEKO_API_DECL neko_memory_info_t neko_os_memory_info();
NEKO_API_DECL void neko_os_msgbox(const_str msg);

// Platform Internal
NEKO_API_DECL void neko_os_symbol_handler_init();
NEKO_API_DECL const_str neko_os_stacktrace();

/*============================================================
// Platform Native
============================================================*/

static inline u64 neko_get_thread_id() {
#if defined(NEKO_IS_WIN32)
    return (u64)GetCurrentThreadId();
#elif defined(NEKO_IS_LINUX)
    return (u64)syscall(SYS_gettid);
#elif defined(NEKO_IS_APPLE)
    return (mach_port_t)pthread_mach_thread_np(pthread_self());
#else
#error "Unsupported platform!"
#endif
}

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
}  // namespace neko::wtf8

namespace neko::win {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
std::wstring a2w(std::string_view str) noexcept;
std::string w2a(std::wstring_view wstr) noexcept;
std::string a2u(std::string_view str) noexcept;
std::string u2a(std::string_view str) noexcept;
}  // namespace neko::win

#if defined(NEKO_IS_WIN32)

NEKO_INLINE void __native_debug_output(const char *msg) { OutputDebugStringA(msg); }

#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

struct Mutex {
#ifdef _WIN32
    SRWLOCK srwlock;
#else
    pthread_mutex_t pt;
#endif

    void make();
    void trash();
    void lock();
    void unlock();
    bool try_lock();
};

struct Cond {
#ifdef _WIN32
    CONDITION_VARIABLE cv;
#else
    pthread_cond_t pt;
#endif

    void make();
    void trash();
    void signal();
    void broadcast();
    void wait(Mutex *mtx);
    bool timed_wait(Mutex *mtx, uint32_t ms);
};

struct RWLock {
#if _WIN32
    SRWLOCK srwlock;
#else
    pthread_rwlock_t pt;
#endif

    void make();
    void trash();
    void shared_lock();
    void shared_unlock();
    void unique_lock();
    void unique_unlock();
};

struct Sema {
#ifdef _WIN32
    HANDLE handle;
#else
    sem_t *sem;
#endif

    void make(int n = 0);
    void trash();
    void post(int n = 1);
    void wait();
};

typedef void (*ThreadProc)(void *);

struct Thread {
    void *ptr = nullptr;

    void make(ThreadProc fn, void *udata);
    void join();
};

struct LockGuard {
    Mutex *mtx;

    LockGuard(Mutex *mtx) : mtx(mtx) { mtx->lock(); };
    ~LockGuard() { mtx->unlock(); };
    LockGuard(LockGuard &&) = delete;
    LockGuard &operator=(LockGuard &&) = delete;

    operator bool() { return true; }
};

uint64_t this_thread_id();

struct Allocator {
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual void *alloc(size_t bytes, const char *file, i32 line) = 0;
    virtual void *realloc(void *ptr, size_t new_size, const char *file, i32 line) = 0;
    virtual void free(void *ptr) = 0;
};

struct HeapAllocator : Allocator {
    void make() {}
    void trash() {}
    void *alloc(size_t bytes, const char *, i32) { return malloc(bytes); }
    void *realloc(void *ptr, size_t new_size, const char *, i32) { return ::realloc(ptr, new_size); }
    void free(void *ptr) { ::free(ptr); }
};

struct DebugAllocInfo {
    const char *file;
    i32 line;
    size_t size;
    DebugAllocInfo *prev;
    DebugAllocInfo *next;
    alignas(16) u8 buf[1];
};

struct DebugAllocator : Allocator {
    DebugAllocInfo *head = nullptr;
    Mutex mtx = {};

    void make() { mtx.make(); }
    void trash() { mtx.trash(); }
    void *alloc(size_t bytes, const char *file, i32 line);
    void *realloc(void *ptr, size_t new_size, const char *file, i32 line);
    void free(void *ptr);
    void dump_allocs();
};

void *__neko_mem_safe_calloc(size_t count, size_t element_size, const char *file, int line);

extern Allocator *g_allocator;

#define mem_alloc(bytes) g_allocator->alloc(bytes, __FILE__, __LINE__)
#define mem_free(ptr) g_allocator->free((void *)ptr)
#define mem_realloc(ptr, size) g_allocator->realloc(ptr, size, __FILE__, __LINE__)
#define mem_calloc(count, element_size) __neko_mem_safe_calloc(count, element_size, (char *)__FILE__, __LINE__)

i32 os_change_dir(const char *path);
String os_program_dir();
String os_program_path();
u64 os_file_modtime(const char *filename);
void os_high_timer_resolution();
void os_sleep(u32 ms);
void os_yield();

#endif  // !NEKO_PF_H
