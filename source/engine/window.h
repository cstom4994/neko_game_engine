#ifndef NEKO_WINDOW
#define NEKO_WINDOW

#include "engine/base.hpp"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/scripting/lua_wrapper.hpp"

class Window : public Neko::SingletonClass<Window> {
private:
    GLFWwindow *window;

public:

    void create();

    GLFWwindow *glfwWindow() const { return window; }

    operator GLFWwindow *() const { return glfwWindow(); }

    const char *GetClipboard();
    void SetClipboard(const char *text);
    int ShowMsgBox(const char *msg, const char *title);
    void Focus();
    int HasFocus();
    void ShowMouse(bool show);
    double Scale();
    void SetFramebufferSizeCallback(GLFWframebuffersizefun func);
    void SwapBuffer();
};

#endif