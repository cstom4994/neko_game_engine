#ifndef NEKO_WINDOW
#define NEKO_WINDOW

#include "engine/base.hpp"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "base/scripting/lua_wrapper.hpp"

class Window : public Neko::SingletonClass<Window> {
private:
    GLFWwindow *window;

public:
    void init() override;
    void fini() override;
    void update() override;

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

    inline void Query(int idx, i32 *width, i32 *height) {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        *width = w;
        *height = h;
    }
};

#endif