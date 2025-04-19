#include "engine/input.h"

#include <ctype.h>
#include <stdbool.h>

#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/scripting/scripting.h"
#include "engine/scripting/lua_util.h"
#include "engine/ui.h"
#include "engine/components/camera.h"

using namespace Neko::luabind;

bool input_key_down(KeyCode key) {
    int glfwkey = (key);
    return glfwGetKey(the<CL>().window->glfwWindow(), glfwkey) == GLFW_PRESS;
}

bool input_key_release(KeyCode key) {
    int glfwkey = (key);
    return glfwGetKey(the<CL>().window->glfwWindow(), glfwkey) == GLFW_RELEASE;
}

vec2 input_get_mouse_pos_pixels_fix() {
    double x, y;
    glfwGetCursorPos(the<CL>().window->glfwWindow(), &x, &y);
    return luavec2(x, y);
}

vec2 input_get_mouse_pos_pixels() {
    double x, y;
    glfwGetCursorPos(the<CL>().window->glfwWindow(), &x, &y);
    return luavec2(x, -y);
}
vec2 input_get_mouse_pos_unit() { return Neko::the<CL>().pixels_to_unit(input_get_mouse_pos_pixels()); }

bool input_mouse_down(MouseCode mouse) {
    int glfwmouse = (mouse);
    return glfwGetMouseButton(the<CL>().window->glfwWindow(), glfwmouse) == GLFW_PRESS;
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
                (*f)((KeyCode)(key), scancode, mods);
            }
            break;
        }
        case GLFW_RELEASE: {
            for (auto f : Neko::the<Input>().key_up_cbs) {
                (*f)((KeyCode)(key), scancode, mods);
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
            for (auto f : Neko::the<Input>().mouse_down_cbs) (*f)((MouseCode)(mouse));
            break;
        }

        case GLFW_RELEASE: {
            for (auto f : Neko::the<Input>().mouse_up_cbs) (*f)((MouseCode)(mouse));
            break;
        }
    }
}

static void _scroll_callback(GLFWwindow* window, double x, double y) {
    for (auto f : Neko::the<Input>().scroll_cbs) (*f)(luavec2(x, y));
}

int wrap_input_keycode_str(lua_State* L) {
    KeyCode type_val = (KeyCode)lua_tointeger(L, 1);
    LuaPush<KeyCode>(L, type_val);
    return 1;
}
int wrap_input_key_down(lua_State* L) {
    KeyCode code = LuaGet<KeyCode>(L, 1);
    bool v = input_key_down(code);
    LuaPush(L, v);
    return 1;
}
int wrap_input_key_release(lua_State* L) {
    KeyCode code = LuaGet<KeyCode>(L, 1);
    bool v = input_key_release(code);
    LuaPush(L, v);
    return 1;
}
int wrap_input_get_mouse_pos_pixels_fix(lua_State* L) {
    vec2 v = input_get_mouse_pos_pixels_fix();
    LuaPush<vec2>(L, v);
    return 1;
}
int wrap_input_get_mouse_pos_pixels(lua_State* L) {
    vec2 v = input_get_mouse_pos_pixels();
    LuaPush<vec2>(L, v);
    return 1;
}
int wrap_input_get_mouse_pos_unit(lua_State* L) {
    vec2 v = input_get_mouse_pos_unit();
    LuaPush<vec2>(L, v);
    return 1;
}
int wrap_input_mouse_down(lua_State* L) {
    MouseCode code = LuaGet<MouseCode>(L, 1);
    bool v = input_mouse_down(code);
    LuaPush(L, v);
    return 1;
}

void Input::init() {
    PROFILE_FUNC();

    auto& w = Neko::the<Window>();

    auto L = ENGINE_LUA();

    glfwSetKeyCallback(w.glfwWindow(), _key_callback);
    glfwSetCharCallback(w.glfwWindow(), _char_callback);
    glfwSetMouseButtonCallback(w.glfwWindow(), _mouse_callback);
    glfwSetScrollCallback(w.glfwWindow(), _scroll_callback);

    glfwSetCursorPosCallback(w.glfwWindow(), [](GLFWwindow* window, double x, double y) {
        auto& input = the<Input>();
        for (auto f : input.mouse_move_cbs) (*f)(luavec2(x, -y));
    });

    {
        LuaTableAccess<InputMousePos>::Init(L);
        auto& meta = LuaTableAccess<InputMousePos>::GetMeta();

        lua_getglobal(L, "neko");

        lua_newtable(L);
        MousePosTable = LuaTableAccess<InputMousePos>::GetTable(L, -1);
        LuaTableAccess<InputMousePos>::Fields<lua_Number>(MousePosTable, "x") = 0.f;
        LuaTableAccess<InputMousePos>::Fields<lua_Number>(MousePosTable, "y") = 0.f;
        lua_setfield(L, -2, "__input_mouse_pos");

        lua_pop(L, 1);
    }

    auto type = BUILD_TYPE(Input)
                        .CClosure({{"input_keycode_str", wrap_input_keycode_str},
                                   {"input_key_down", wrap_input_key_down},
                                   {"input_key_release", wrap_input_key_release},
                                   {"input_get_mouse_pos_pixels_fix", wrap_input_get_mouse_pos_pixels_fix},
                                   {"input_get_mouse_pos_pixels", wrap_input_get_mouse_pos_pixels},
                                   {"input_get_mouse_pos_unit", wrap_input_get_mouse_pos_unit},
                                   {"input_mouse_down", wrap_input_mouse_down}})
                        .Build();
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

int Input::OnPreUpdate() {

    double x, y;
    glfwGetCursorPos(the<CL>().window->glfwWindow(), &x, &y);

    vec2 p = Camera::camera_pixels_to_world({(f32)x, -(f32)y});
    LuaTableAccess<InputMousePos>::Fields<lua_Number>(MousePosTable, "x") = p.x;
    LuaTableAccess<InputMousePos>::Fields<lua_Number>(MousePosTable, "y") = p.y;

    return 0;
}

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

i32 keyboard_lookup(String str) {
    switch (fnv1a(str)) {
        case "space"_hash:
            return 32;
        case "'"_hash:
            return 39;
        case ","_hash:
            return 44;
        case "-"_hash:
            return 45;
        case "."_hash:
            return 46;
        case "/"_hash:
            return 47;
        case "0"_hash:
            return 48;
        case "1"_hash:
            return 49;
        case "2"_hash:
            return 50;
        case "3"_hash:
            return 51;
        case "4"_hash:
            return 52;
        case "5"_hash:
            return 53;
        case "6"_hash:
            return 54;
        case "7"_hash:
            return 55;
        case "8"_hash:
            return 56;
        case "9"_hash:
            return 57;
        case ";"_hash:
            return 59;
        case "="_hash:
            return 61;
        case "a"_hash:
            return 65;
        case "b"_hash:
            return 66;
        case "c"_hash:
            return 67;
        case "d"_hash:
            return 68;
        case "e"_hash:
            return 69;
        case "f"_hash:
            return 70;
        case "g"_hash:
            return 71;
        case "h"_hash:
            return 72;
        case "i"_hash:
            return 73;
        case "j"_hash:
            return 74;
        case "k"_hash:
            return 75;
        case "l"_hash:
            return 76;
        case "m"_hash:
            return 77;
        case "n"_hash:
            return 78;
        case "o"_hash:
            return 79;
        case "p"_hash:
            return 80;
        case "q"_hash:
            return 81;
        case "r"_hash:
            return 82;
        case "s"_hash:
            return 83;
        case "t"_hash:
            return 84;
        case "u"_hash:
            return 85;
        case "v"_hash:
            return 86;
        case "w"_hash:
            return 87;
        case "x"_hash:
            return 88;
        case "y"_hash:
            return 89;
        case "z"_hash:
            return 90;
        case "["_hash:
            return 91;
        case "\\"_hash:
            return 92;
        case "]"_hash:
            return 93;
        case "`"_hash:
            return 96;
        case "world_1"_hash:
            return 161;
        case "world_2"_hash:
            return 162;
        case "esc"_hash:
            return 256;
        case "enter"_hash:
            return 257;
        case "tab"_hash:
            return 258;
        case "backspace"_hash:
            return 259;
        case "insert"_hash:
            return 260;
        case "delete"_hash:
            return 261;
        case "right"_hash:
            return 262;
        case "left"_hash:
            return 263;
        case "down"_hash:
            return 264;
        case "up"_hash:
            return 265;
        case "pg_up"_hash:
            return 266;
        case "pg_down"_hash:
            return 267;
        case "home"_hash:
            return 268;
        case "end"_hash:
            return 269;
        case "caps_lock"_hash:
            return 280;
        case "scroll_lock"_hash:
            return 281;
        case "num_lock"_hash:
            return 282;
        case "print_screen"_hash:
            return 283;
        case "pause"_hash:
            return 284;
        case "f1"_hash:
            return 290;
        case "f2"_hash:
            return 291;
        case "f3"_hash:
            return 292;
        case "f4"_hash:
            return 293;
        case "f5"_hash:
            return 294;
        case "f6"_hash:
            return 295;
        case "f7"_hash:
            return 296;
        case "f8"_hash:
            return 297;
        case "f9"_hash:
            return 298;
        case "f10"_hash:
            return 299;
        case "f11"_hash:
            return 300;
        case "f12"_hash:
            return 301;
        case "f13"_hash:
            return 302;
        case "f14"_hash:
            return 303;
        case "f15"_hash:
            return 304;
        case "f16"_hash:
            return 305;
        case "f17"_hash:
            return 306;
        case "f18"_hash:
            return 307;
        case "f19"_hash:
            return 308;
        case "f20"_hash:
            return 309;
        case "f21"_hash:
            return 310;
        case "f22"_hash:
            return 311;
        case "f23"_hash:
            return 312;
        case "f24"_hash:
            return 313;
        case "f25"_hash:
            return 314;
        case "kp0"_hash:
            return 320;
        case "kp1"_hash:
            return 321;
        case "kp2"_hash:
            return 322;
        case "kp3"_hash:
            return 323;
        case "kp4"_hash:
            return 324;
        case "kp5"_hash:
            return 325;
        case "kp6"_hash:
            return 326;
        case "kp7"_hash:
            return 327;
        case "kp8"_hash:
            return 328;
        case "kp9"_hash:
            return 329;
        case "kp."_hash:
            return 330;
        case "kp/"_hash:
            return 331;
        case "kp*"_hash:
            return 332;
        case "kp-"_hash:
            return 333;
        case "kp+"_hash:
            return 334;
        case "kp_enter"_hash:
            return 335;
        case "kp="_hash:
            return 336;
        case "lshift"_hash:
            return 340;
        case "lctrl"_hash:
            return 341;
        case "lalt"_hash:
            return 342;
        case "lsuper"_hash:
            return 343;
        case "rshift"_hash:
            return 344;
        case "rctrl"_hash:
            return 345;
        case "ralt"_hash:
            return 346;
        case "rsuper"_hash:
            return 347;
        case "menu"_hash:
            return 348;
        default:
            return 0;
    }
}
