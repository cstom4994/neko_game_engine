#include "engine/input.h"

#include <ctype.h>
#include <stdbool.h>

#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "base/scripting/scripting.h"
#include "engine/ui.h"

using namespace Neko::luabind;

static int _keycode_to_glfw(KeyCode key) { return key; }
static KeyCode _glfw_to_keycode(int key) { return (KeyCode)key; }
static int _mousecode_to_glfw(MouseCode mouse) { return mouse; }
static MouseCode _glfw_to_mousecode(int mouse) { return (MouseCode)mouse; }

bool input_key_down(KeyCode key) {
    int glfwkey = _keycode_to_glfw(key);
    return glfwGetKey(gApp->window->glfwWindow(), glfwkey) == GLFW_PRESS;
}

bool input_key_release(KeyCode key) {
    int glfwkey = _keycode_to_glfw(key);
    return glfwGetKey(gApp->window->glfwWindow(), glfwkey) == GLFW_RELEASE;
}

vec2 input_get_mouse_pos_pixels_fix() {
    double x, y;
    glfwGetCursorPos(gApp->window->glfwWindow(), &x, &y);
    return luavec2(x, y);
}

vec2 input_get_mouse_pos_pixels() {
    double x, y;
    glfwGetCursorPos(gApp->window->glfwWindow(), &x, &y);
    return luavec2(x, -y);
}
vec2 input_get_mouse_pos_unit() { return Neko::the<Game>().pixels_to_unit(input_get_mouse_pos_pixels()); }

bool input_mouse_down(MouseCode mouse) {
    int glfwmouse = _mousecode_to_glfw(mouse);
    return glfwGetMouseButton(gApp->window->glfwWindow(), glfwmouse) == GLFW_PRESS;
}

void input_add_key_down_callback(KeyCallback f) { Neko::the<Input>().key_down_cbs.push(f); }
void input_add_key_up_callback(KeyCallback f) { Neko::the<Input>().key_up_cbs.push(f); }
void input_add_char_down_callback(CharCallback f) { Neko::the<Input>().char_down_cbs.push(f); }
void input_add_mouse_down_callback(MouseCallback f) { Neko::the<Input>().mouse_down_cbs.push(f); }
void input_add_mouse_up_callback(MouseCallback f) { Neko::the<Input>().mouse_up_cbs.push(f); }
void input_add_mouse_move_callback(MouseMoveCallback f) { Neko::the<Input>().mouse_move_cbs.push(f); }
void input_add_scroll_callback(ScrollCallback f) { Neko::the<Input>().scroll_cbs.push(f); }

static void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    switch (action) {
        case GLFW_PRESS: {
            for (auto f : Neko::the<Input>().key_down_cbs) {
                (*f)(_glfw_to_keycode(key), scancode, mods);
            }
            break;
        }
        case GLFW_RELEASE: {
            for (auto f : Neko::the<Input>().key_up_cbs) {
                (*f)(_glfw_to_keycode(key), scancode, mods);
            }
            break;
        }
    }
}

static void _char_callback(GLFWwindow* window, unsigned int c) {
    for (auto f : Neko::the<Input>().char_down_cbs) {
        (*f)(c);
    }
}

static void _mouse_callback(GLFWwindow* window, int mouse, int action, int mods) {
    switch (action) {
        case GLFW_PRESS: {
            for (auto f : Neko::the<Input>().mouse_down_cbs) (*f)(_glfw_to_mousecode(mouse));
            break;
        }

        case GLFW_RELEASE: {
            for (auto f : Neko::the<Input>().mouse_up_cbs) (*f)(_glfw_to_mousecode(mouse));
            break;
        }
    }
}

static void _cursor_pos_callback(GLFWwindow* window, double x, double y) {
    for (auto f : Neko::the<Input>().mouse_move_cbs) (*f)(luavec2(x, -y));
}

static void _scroll_callback(GLFWwindow* window, double x, double y) {
    for (auto f : Neko::the<Input>().scroll_cbs) (*f)(luavec2(x, y));
}

void Input::init() {
    PROFILE_FUNC();

    auto& w = Neko::the<Window>();

    glfwSetKeyCallback(w.glfwWindow(), _key_callback);
    glfwSetCharCallback(w.glfwWindow(), _char_callback);
    glfwSetMouseButtonCallback(w.glfwWindow(), _mouse_callback);
    glfwSetCursorPosCallback(w.glfwWindow(), _cursor_pos_callback);
    glfwSetScrollCallback(w.glfwWindow(), _scroll_callback);
}

void Input::fini() {
    scroll_cbs.trash();

    mouse_move_cbs.trash();

    mouse_up_cbs.trash();
    mouse_down_cbs.trash();

    char_down_cbs.trash();

    key_up_cbs.trash();
    key_down_cbs.trash();
}

void Input::update() {}

INPUT_WRAP_event* input_wrap_new_event(event_queue* equeue) {
    INPUT_WRAP_event* event = equeue->events + equeue->head;
    equeue->head = (equeue->head + 1) % INPUT_WRAP_CAPACITY;
    assert(equeue->head != equeue->tail);
    memset(event, 0, sizeof(INPUT_WRAP_event));
    return event;
}

int input_wrap_next_e(event_queue* equeue, INPUT_WRAP_event* event) {
    memset(event, 0, sizeof(INPUT_WRAP_event));

    if (equeue->head != equeue->tail) {
        *event = equeue->events[equeue->tail];
        equeue->tail = (equeue->tail + 1) % INPUT_WRAP_CAPACITY;
    }

    return event->type != INPUT_WRAP_NONE;
}

void input_wrap_free_e(INPUT_WRAP_event* event) { memset(event, 0, sizeof(INPUT_WRAP_event)); }
