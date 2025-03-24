
#include "window.h"
#include "bootstrap.h"

int game_set_window_vsync(bool vsync) {
    glfwSwapInterval(vsync);
    return 0;
}

int game_set_window_minsize(int width, int height) {
    glfwSetWindowSizeLimits(gApp->window->glfwWindow(), width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    return 0;
}

int game_get_window_width(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (gApp->window) {
        glfwGetWindowSize(gApp->window->glfwWindow(), &w, &h);
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
    if (gApp->window) {
        glfwGetWindowSize(gApp->window->glfwWindow(), &w, &h);
    }
#endif
    *val = h;
    return 0;
}

int game_set_window_position(int x, int y) {
#if !defined(__EMSCRIPTEN__)
    if (gApp->window) {
        glfwSetWindowPos(gApp->window->glfwWindow(), x, y);
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

void Window::create() {
    // create glfw window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(800, 600, "neko_game", NULL, NULL);

    // activate OpenGL context
    glfwMakeContextCurrent(window);
}
