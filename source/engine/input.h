#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include "engine/base.hpp"
#include "base/common/singleton.hpp"
#include "engine/event.h"
#include "base/common/array.hpp"
#include "engine/input_keycode.h"

#include <GLFW/glfw3.h>

bool input_key_down(KeyCode key);
bool input_key_release(KeyCode key);
vec2 input_get_mouse_pos_pixels_fix();
vec2 input_get_mouse_pos_pixels();
vec2 input_get_mouse_pos_unit();
bool input_mouse_down(MouseCode mouse);
i32 keyboard_lookup(String str);

typedef void (*KeyCallback)(KeyCode key, int scancode, int mode);
void input_add_key_down_callback(KeyCallback f);
void input_add_key_up_callback(KeyCallback f);

typedef void (*CharCallback)(unsigned int c);
void input_add_char_down_callback(CharCallback f);

typedef void (*MouseCallback)(MouseCode mouse);
void input_add_mouse_down_callback(MouseCallback f);
void input_add_mouse_up_callback(MouseCallback f);

typedef void (*MouseMoveCallback)(vec2 pos);
void input_add_mouse_move_callback(MouseMoveCallback f);

typedef void (*ScrollCallback)(vec2 scroll);
void input_add_scroll_callback(ScrollCallback f);

class Input : public Neko::SingletonClass<Input> {
public:
    Array<KeyCallback> key_down_cbs;
    Array<KeyCallback> key_up_cbs;
    Array<CharCallback> char_down_cbs;
    Array<MouseCallback> mouse_down_cbs;
    Array<MouseCallback> mouse_up_cbs;
    Array<MouseMoveCallback> mouse_move_cbs;
    Array<ScrollCallback> scroll_cbs;

public:
    void init();
    void fini();
    void update();
};

typedef enum {
    INPUT_WRAP_NONE = 0,
    INPUT_WRAP_WINDOW_MOVED = 1 << 1,
    INPUT_WRAP_WINDOW_RESIZED = 1 << 2,
    INPUT_WRAP_WINDOW_CLOSED = 1 << 3,
    INPUT_WRAP_WINDOW_REFRESH = 1 << 4,
    INPUT_WRAP_WINDOW_FOCUSED = 1 << 5,
    INPUT_WRAP_WINDOW_DEFOCUSED = 1 << 6,
    INPUT_WRAP_WINDOW_ICONIFIED = 1 << 7,
    INPUT_WRAP_WINDOW_UNICONIFIED = 1 << 8,
    INPUT_WRAP_FRAMEBUFFER_RESIZED = 1 << 9,
    INPUT_WRAP_BUTTON_PRESSED = 1 << 10,
    INPUT_WRAP_BUTTON_RELEASED = 1 << 11,
    INPUT_WRAP_CURSOR_MOVED = 1 << 12,
    INPUT_WRAP_CURSOR_ENTERED = 1 << 13,
    INPUT_WRAP_CURSOR_LEFT = 1 << 14,
    INPUT_WRAP_SCROLLED = 1 << 15,
    INPUT_WRAP_KEY_PRESSED = 1 << 16,
    INPUT_WRAP_KEY_REPEATED = 1 << 17,
    INPUT_WRAP_KEY_RELEASED = 1 << 18,
    INPUT_WRAP_CODEPOINT_INPUT = 1 << 19
} INPUT_WRAP_type;

typedef struct INPUT_WRAP_event {
    INPUT_WRAP_type type;
    union {
        struct {
            int x;
            int y;
        } pos;
        struct {
            int width;
            int height;
        } size;
        struct {
            double x;
            double y;
        } scroll;
        struct {
            int key;
            int scancode;
            int mods;
        } keyboard;
        struct {
            int button;
            int mods;
        } mouse;
        unsigned int codepoint;
    };
} INPUT_WRAP_event;

#ifndef INPUT_WRAP_CAPACITY
#define INPUT_WRAP_CAPACITY 1024
#endif

typedef struct event_queue {
    INPUT_WRAP_event events[INPUT_WRAP_CAPACITY];
    size_t head;
    size_t tail;
} event_queue;

INPUT_WRAP_event* input_wrap_new_event(event_queue* equeue);
int input_wrap_next_e(event_queue* equeue, INPUT_WRAP_event* event);
void input_wrap_free_e(INPUT_WRAP_event* event);

#define INPUT_WRAP_DEFINE(NAME)                                              \
    static event_queue NAME##_input_queue = {{INPUT_WRAP_NONE}, 0, 0};       \
    static void NAME##_char_down(unsigned int c) {                           \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->type = INPUT_WRAP_CODEPOINT_INPUT;                            \
        event->codepoint = c;                                                \
    }                                                                        \
    static void NAME##_key_down(KeyCode key, int scancode, int mods) {       \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->keyboard.key = key;                                           \
        event->keyboard.scancode = scancode;                                 \
        event->keyboard.mods = mods;                                         \
        event->type = INPUT_WRAP_KEY_PRESSED;                                \
    }                                                                        \
    static void NAME##_key_up(KeyCode key, int scancode, int mods) {         \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->keyboard.key = key;                                           \
        event->keyboard.scancode = scancode;                                 \
        event->keyboard.mods = mods;                                         \
        event->type = INPUT_WRAP_KEY_RELEASED;                               \
    }                                                                        \
    static void NAME##_mouse_down(MouseCode mouse) {                         \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->mouse.button = mouse;                                         \
        event->type = INPUT_WRAP_BUTTON_PRESSED;                             \
    }                                                                        \
    static void NAME##_mouse_up(MouseCode mouse) {                           \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->mouse.button = mouse;                                         \
        event->type = INPUT_WRAP_BUTTON_RELEASED;                            \
    }                                                                        \
    static void NAME##_mouse_move(vec2 pos) {                                \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->type = INPUT_WRAP_CURSOR_MOVED;                               \
        event->pos.x = (int)pos.x;                                           \
        event->pos.y = (int)pos.y;                                           \
    }                                                                        \
    static void NAME##_scroll(vec2 scroll) {                                 \
        INPUT_WRAP_event* event = input_wrap_new_event(&NAME##_input_queue); \
        event->type = INPUT_WRAP_SCROLLED;                                   \
        event->scroll.x = scroll.x;                                          \
        event->scroll.y = scroll.y;                                          \
    }

#endif
