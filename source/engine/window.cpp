
#include "window.h"
#include "bootstrap.h"

int game_set_window_vsync(bool vsync) {
    glfwSwapInterval(vsync);
    return 0;
}

const char *window_clipboard() { return glfwGetClipboardString(gApp->game_window); }

int window_prompt(const char *msg, const char *title) { return (MessageBoxA(0, msg, title, MB_YESNO | MB_ICONWARNING) == IDYES); }

void window_setclipboard(const char *text) { glfwSetClipboardString(gApp->game_window, text); }

void window_focus() { glfwFocusWindow(gApp->game_window); }

int window_has_focus() { return !!glfwGetWindowAttrib(gApp->game_window, GLFW_FOCUSED); }

double window_scale() {
    float xscale = 1, yscale = 1;
#ifndef NEKO_IS_APPLE  // @todo: remove silicon mac M1 hack
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
#endif
    return NEKO_MAX(xscale, yscale);
}