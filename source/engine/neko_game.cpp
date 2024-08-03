#include "neko_game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "console.h"
#include "glew_glfw.h"
#include "system.h"

#ifdef CGAME_DEBUG_WINDOW
#include "debugwin.h"
#endif

#include "test/test.h"

// imgui
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// deps
#include "vendor/sokol_time.h"

GLFWwindow *game_window;

static bool quit = false;  // 如果为 true 则退出主循环
static int sargc = 0;
static char **sargv;
static Mutex g_init_mtx;

// -------------------------------------------------------------------------

static void _glfw_error_callback(int error, const char *desc) { fprintf(stderr, "glfw: %s\n", desc); }

static void _game_init() {

    g_init_mtx.make();
    LockGuard lock(&g_init_mtx);

#ifndef NDEBUG
    g_allocator = new DebugAllocator();
#else
    g_allocator = new HeapAllocator();
#endif

    g_allocator->make();

    os_high_timer_resolution();
    stm_setup();

    profile_setup();
    PROFILE_FUNC();

    // initialize glfw
    glfwSetErrorCallback(_glfw_error_callback);
    glfwInit();

    // create glfw window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    game_window = glfwCreateWindow(800, 600, "neko_game", NULL, NULL);
#ifdef CGAME_DEBUG_WINDOW
    debugwin_init();
#endif

    // activate OpenGL context
    glfwMakeContextCurrent(game_window);

    // initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();  // see http://www.opengl.org/wiki/OpenGL_Loading_Library

    // some GL settings
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.95f, 0.95f, 0.95f, 1.f);

    // random seed
    srand(time(NULL));

    // init systems
    console_puts("welcome to neko!");
    system_init();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(game_window, true);
    ImGui_ImplOpenGL3_Init();

    // init test
    test_init();
}

static void _game_deinit() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // deinit systems
    system_deinit();

    // deinit glfw
    glfwTerminate();

#ifdef USE_PROFILER
    profile_shutdown();
#endif

#ifndef NDEBUG
    DebugAllocator *allocator = dynamic_cast<DebugAllocator *>(g_allocator);
    if (allocator != nullptr) {
        allocator->dump_allocs();
    }
#endif

    g_allocator->trash();
    operator delete(g_allocator);

    neko_println("see ya");
}

static void _game_events() {
    glfwPollEvents();

    if (glfwWindowShouldClose(game_window)) game_quit();
}

static void _game_update() { system_update_all(); }

static void _game_draw() {
    static bool first = true;

    // don't draw first frame -- allow a full update
    if (first) {
        first = false;
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    system_draw_all();

    ImGui::Render();
    // int displayX, displayY;
    // glfwGetFramebufferSize(window, &displayX, &displayY);
    // glViewport(0, 0, displayX, displayY);
    // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(game_window);
}

// -------------------------------------------------------------------------

void game_run(int argc, char **argv) {
    sargc = argc;
    sargv = argv;

    _game_init();

    while (!quit) {
        _game_events();
        _game_update();
        _game_draw();
    }

    _game_deinit();
}

void game_set_bg_color(Color c) { glClearColor(c.r, c.g, c.b, 1.0); }

void game_set_window_size(CVec2 s) { glfwSetWindowSize(game_window, s.x, s.y); }
CVec2 game_get_window_size() {
    int w, h;
    glfwGetWindowSize(game_window, &w, &h);
    return vec2(w, h);
}
CVec2 game_unit_to_pixels(CVec2 p) {
    CVec2 hw = vec2_scalar_mul(game_get_window_size(), 0.5f);
    p = vec2_mul(p, hw);
    p = vec2(p.x + hw.x, p.y - hw.y);
    return p;
}
CVec2 game_pixels_to_unit(CVec2 p) {
    CVec2 hw = vec2_scalar_mul(game_get_window_size(), 0.5f);
    p = vec2(p.x - hw.x, p.y + hw.y);
    p = vec2_div(p, hw);
    return p;
}

void game_quit() { quit = true; }

int game_get_argc() { return sargc; }
char **game_get_argv() { return sargv; }
