#include "engine/input.h"

#include <ctype.h>

#include "engine/prelude.h"
#include "glew_glfw.h"
#include "engine/base.h"
#include "engine/game.h"

static CArray *key_down_cbs;
static CArray *key_up_cbs;
static CArray *char_down_cbs;
static CArray *mouse_down_cbs;
static CArray *mouse_up_cbs;
static CArray *mouse_move_cbs;
static CArray *scroll_cbs;

static int _keycode_to_glfw(KeyCode key) { return key; }
static KeyCode _glfw_to_keycode(int key) { return (KeyCode)key; }
static int _mousecode_to_glfw(MouseCode mouse) { return mouse; }
static MouseCode _glfw_to_mousecode(int mouse) { return (MouseCode)mouse; }

bool input_keycode_is_char(KeyCode key) {
    switch (key) {
        case KC_SPACE:
        case KC_APOSTROPHE:
        case KC_COMMA:
        case KC_MINUS:
        case KC_PERIOD:
        case KC_SLASH:
        case KC_0:
        case KC_1:
        case KC_2:
        case KC_3:
        case KC_4:
        case KC_5:
        case KC_6:
        case KC_7:
        case KC_8:
        case KC_9:
        case KC_SEMICOLON:
        case KC_EQUAL:
        case KC_A:
        case KC_B:
        case KC_C:
        case KC_D:
        case KC_E:
        case KC_F:
        case KC_G:
        case KC_H:
        case KC_I:
        case KC_J:
        case KC_K:
        case KC_L:
        case KC_M:
        case KC_N:
        case KC_O:
        case KC_P:
        case KC_Q:
        case KC_R:
        case KC_S:
        case KC_T:
        case KC_U:
        case KC_V:
        case KC_W:
        case KC_X:
        case KC_Y:
        case KC_Z:
        case KC_LEFT_BRACKET:
        case KC_BACKSLASH:
        case KC_RIGHT_BRACKET:
        case KC_GRAVE_ACCENT:
        case KC_WORLD_1:
        case KC_WORLD_2:
            return true;

        default:
            return false;
    }
}

char input_keycode_to_char(KeyCode key) {
    if (input_keycode_is_char(key)) return tolower(key);
    return '\0';
}
KeyCode input_char_to_keycode(char c) { return (KeyCode)toupper(c); }

bool input_key_down(KeyCode key) {
    int glfwkey = _keycode_to_glfw(key);
    return glfwGetKey(g_app->game_window, glfwkey) == GLFW_PRESS;
}

CVec2 input_get_mouse_pos_pixels() {
    double x, y;
    glfwGetCursorPos(g_app->game_window, &x, &y);
    return vec2(x, -y);
}
CVec2 input_get_mouse_pos_unit() { return game_pixels_to_unit(input_get_mouse_pos_pixels()); }

bool input_mouse_down(MouseCode mouse) {
    int glfwmouse = _mousecode_to_glfw(mouse);
    return glfwGetMouseButton(g_app->game_window, glfwmouse) == GLFW_PRESS;
}

void input_add_key_down_callback(KeyCallback f) { array_add_val(KeyCallback, key_down_cbs) = f; }
void input_add_key_up_callback(KeyCallback f) { array_add_val(KeyCallback, key_up_cbs) = f; }
void input_add_char_down_callback(CharCallback f) { array_add_val(CharCallback, char_down_cbs) = f; }

void input_add_mouse_down_callback(MouseCallback f) { array_add_val(MouseCallback, mouse_down_cbs) = f; }
void input_add_mouse_up_callback(MouseCallback f) { array_add_val(MouseCallback, mouse_up_cbs) = f; }

void input_add_mouse_move_callback(MouseMoveCallback f) { array_add_val(MouseMoveCallback, mouse_move_cbs) = f; }

void input_add_scroll_callback(ScrollCallback f) { array_add_val(ScrollCallback, scroll_cbs) = f; }

static void _key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    KeyCallback *f;

    switch (action) {
        case GLFW_PRESS:
            array_foreach(f, key_down_cbs) (*f)(_glfw_to_keycode(key));
            break;

        case GLFW_RELEASE:
            array_foreach(f, key_up_cbs) (*f)(_glfw_to_keycode(key));
            break;
    }
}

static void _char_callback(GLFWwindow *window, unsigned int c) {
    CharCallback *f;

    array_foreach(f, char_down_cbs) (*f)(c);
}

static void _mouse_callback(GLFWwindow *window, int mouse, int action, int mods) {
    MouseCallback *f;

    switch (action) {
        case GLFW_PRESS:
            array_foreach(f, mouse_down_cbs) (*f)(_glfw_to_mousecode(mouse));
            break;

        case GLFW_RELEASE:
            array_foreach(f, mouse_up_cbs) (*f)(_glfw_to_mousecode(mouse));
            break;
    }
}

static void _cursor_pos_callback(GLFWwindow *window, double x, double y) {
    MouseMoveCallback *f;

    array_foreach(f, mouse_move_cbs) (*f)(vec2(x, -y));
}

static void _scroll_callback(GLFWwindow *window, double x, double y) {
    ScrollCallback *f;

    array_foreach(f, scroll_cbs) (*f)(vec2(x, y));
}

void input_init() {
    key_down_cbs = array_new(KeyCallback);
    key_up_cbs = array_new(KeyCallback);
    glfwSetKeyCallback(g_app->game_window, _key_callback);

    char_down_cbs = array_new(CharCallback);
    glfwSetCharCallback(g_app->game_window, _char_callback);

    mouse_down_cbs = array_new(MouseCallback);
    mouse_up_cbs = array_new(MouseCallback);
    glfwSetMouseButtonCallback(g_app->game_window, _mouse_callback);

    mouse_move_cbs = array_new(MouseMoveCallback);
    glfwSetCursorPosCallback(g_app->game_window, _cursor_pos_callback);

    scroll_cbs = array_new(ScrollCallback);
    glfwSetScrollCallback(g_app->game_window, _scroll_callback);
}

void input_fini() {
    array_free(scroll_cbs);

    array_free(mouse_move_cbs);

    array_free(mouse_up_cbs);
    array_free(mouse_down_cbs);

    array_free(char_down_cbs);

    array_free(key_up_cbs);
    array_free(key_down_cbs);
}
