
#include "window.h"
#include "bootstrap.h"
#include "engine/base/common/profiler.hpp"
#include "engine/scripting/lua_util.h"

int game_set_window_vsync(bool vsync) {
    glfwSwapInterval(vsync);
    return 0;
}

int game_set_window_minsize(int width, int height) {
    glfwSetWindowSizeLimits(the<CL>().window->glfwWindow(), width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    return 0;
}

int game_get_window_width(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (the<CL>().window) {
        glfwGetWindowSize(the<CL>().window->glfwWindow(), &w, &h);
    }
#endif
    *val = w;
    return 0;
}

int game_get_window_height(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (the<CL>().window) {
        glfwGetWindowSize(the<CL>().window->glfwWindow(), &w, &h);
    }
#endif
    *val = h;
    return 0;
}

int game_set_window_position(int x, int y) {
#if !defined(__EMSCRIPTEN__)
    if (the<CL>().window) {
        glfwSetWindowPos(the<CL>().window->glfwWindow(), x, y);
    }
#endif
    return 0;
}

int Window::ShowMsgBox(const char *msg, const char *title) { return (MessageBoxA(0, msg, title, MB_YESNO | MB_ICONWARNING) == IDYES); }

const char *Window::GetClipboard() { return glfwGetClipboardString(window); }

void Window::SetClipboard(const char *text) { glfwSetClipboardString(window, text); }

void Window::Focus() { glfwFocusWindow(window); }

int Window::HasFocus() { return !!glfwGetWindowAttrib(window, GLFW_FOCUSED); }

void Window::ShowMouse(bool show) { glfwSetInputMode(window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED); }

double Window::Scale() {
    float xscale = 1, yscale = 1;
#ifndef NEKO_IS_APPLE  // @todo: remove silicon mac M1 hack
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
#endif
    return NEKO_MAX(xscale, yscale);
}

void Window::SetFramebufferSizeCallback(GLFWframebuffersizefun func) { glfwSetFramebufferSizeCallback(window, func); }

void Window::SwapBuffer() { glfwSwapBuffers(window); }

int wrap_window_clipboard(lua_State *L) {
    const_str v = the<Window>().GetClipboard();
    lua_pushstring(L, v);
    return 1;
}
int wrap_window_prompt(lua_State *L) {
    const_str msg = lua_tostring(L, 1);
    const_str title = lua_tostring(L, 2);
    int v = the<Window>().ShowMsgBox(msg, title);
    lua_pushinteger(L, v);
    return 1;
}
int wrap_window_setclipboard(lua_State *L) {
    const_str str = lua_tostring(L, 1);
    the<Window>().SetClipboard(str);
    return 0;
}
int wrap_window_focus(lua_State *L) {
    the<Window>().Focus();
    return 0;
}
int wrap_window_has_focus(lua_State *L) {
    int v = the<Window>().HasFocus();
    lua_pushinteger(L, v);
    return 1;
}
int wrap_window_scale(lua_State *L) {
    f64 v = the<Window>().Scale();
    lua_pushnumber(L, v);
    return 1;
}
int wrap_window_size(lua_State *L) {
    vec2 v = luavec2(the<CL>().state.width, the<CL>().state.height);
    LuaPush<vec2>(L, v);
    return 1;
}

void Window::init() {

    auto type = BUILD_TYPE(Window)
                        .CClosure({{"window_clipboard", wrap_window_clipboard},
                                   {"window_prompt", wrap_window_prompt},
                                   {"window_setclipboard", wrap_window_setclipboard},
                                   {"window_focus", wrap_window_focus},
                                   {"window_has_focus", wrap_window_has_focus},
                                   {"window_scale", wrap_window_scale},
                                   {"window_size", wrap_window_size}})
                        .Build();
}

void Window::create() {
    PROFILE_FUNC();

    // create glfw window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(800, 600, "neko_game", NULL, NULL);

    // activate OpenGL context
    glfwMakeContextCurrent(window);
}
