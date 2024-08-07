#include "engine/game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "console.h"
#include "engine/camera.h"
#include "engine/inspector.h"
#include "engine/script.h"
#include "engine/sprite.h"
#include "engine/system.h"
#include "engine/texture.h"
#include "engine/ui.h"
#include "glew_glfw.h"
#include "test/test.h"

// deps
#include "vendor/sokol_time.h"

static bool quit = false;  // 如果为 true 则退出主循环
static int sargc = 0;
static char **sargv;
static Mutex g_init_mtx;

// -------------------------------------------------------------------------

static const char *filename = usr_path("scratch.lua");

bool _exists() {
    struct stat st;
    return stat(filename, &st) == 0;
}

// 自上次调用 _modified() 后是否已修改 一开始为 false
bool _modified() {
    struct stat st;
    static time_t prev_time = 0;
    bool modified;
    static bool first = true;

    if (stat(filename, &st) != 0) {
        first = false;
        return false;  // 不存在或错误
    }

    modified = !first && st.st_mtime != prev_time;
    prev_time = st.st_mtime;
    first = false;
    return modified;
}

void scratch_run() { script_run_file(filename); }

void scratch_update() {
    if (_modified()) scratch_run();
}

typedef struct {
    // position
    float px, py;
    // texcoords
    float tx, ty, tw, th;
} batch_tex;

void batch_test_draw(BatchRenderer *renderer, Texture *tex, batch_tex a) {
    batch_texture(renderer, tex->id);

    float x1 = a.px;
    float y1 = a.py;
    float x2 = a.px + 48;
    float y2 = a.py + 48;

    float u1 = a.tx / tex->width;
    float v1 = a.ty / tex->height;
    float u2 = (a.tx + a.tw) / tex->width;
    float v2 = (a.ty + a.th) / tex->height;

    batch_push_vertex(renderer, x1, y1, u1, v1);
    batch_push_vertex(renderer, x2, y2, u2, v2);
    batch_push_vertex(renderer, x1, y2, u1, v2);

    batch_push_vertex(renderer, x1, y1, u1, v1);
    batch_push_vertex(renderer, x2, y1, u2, v1);
    batch_push_vertex(renderer, x2, y2, u2, v2);
}

// -------------------------------------------------------------------------

static void _glfw_error_callback(int error, const char *desc) { fprintf(stderr, "glfw: %s\n", desc); }

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // 更新视口
    glViewport(0, 0, width, height);
}

BatchRenderer renderer;

static void _game_init() {

    g_init_mtx.make();
    LockGuard lock(&g_init_mtx);

#ifndef NDEBUG
    g_allocator = new DebugAllocator();
#else
    g_allocator = new HeapAllocator();
#endif

    g_allocator->make();

    g_app = (App *)mem_alloc(sizeof(App));
    memset(g_app, 0, sizeof(App));

    g_app->gpu_mtx.make();

    g_app->error_mtx.make();

    os_high_timer_resolution();
    stm_setup();

    g_app->args.resize(sargc);
    for (i32 i = 0; i < sargc; i++) {
        g_app->args[i] = to_cstr(sargv[i]);
    }

#if defined(NDEBUG)
    console_log("neko %d", neko_buildnum());
#else
    console_log("neko %d (debug build) (%s, %s)", neko_buildnum(), LUA_VERSION, LUAJIT_VERSION);
#endif

    profile_setup();
    PROFILE_FUNC();

    // initialize glfw
    glfwSetErrorCallback(_glfw_error_callback);
    glfwInit();

    // create glfw window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_app->game_window = glfwCreateWindow(800, 600, "neko_game", NULL, NULL);

    // activate OpenGL context
    glfwMakeContextCurrent(g_app->game_window);

    // 注册窗口大小改变的回调函数
    glfwSetFramebufferSizeCallback(g_app->game_window, framebuffer_size_callback);

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

    // time set
    g_app->time.startup = stm_now();
    g_app->time.last = stm_now();

    // init systems
    console_puts("welcome to neko!");
    system_init();

    // init test
    test_init();

    assets_start_hot_reload();

    renderer = batch_init(6000);
    // tex_aliens = create_texture("gamedir/assets/aliens.png");
    texture_load("assets/aliens.png", false);
}

static void _game_fini() {
    PROFILE_FUNC();

    mem_free(renderer.vertices);

    // fini systems
    system_fini();

    // fini glfw
    glfwTerminate();

    g_app->gpu_mtx.trash();

    g_app->error_mtx.trash();

#ifdef USE_PROFILER
    profile_shutdown();
#endif

    vfs_fini();

    mem_free(g_app->fatal_error.data);
    mem_free(g_app->traceback.data);

    for (String arg : g_app->args) {
        mem_free(arg.data);
    }
    mem_free(g_app->args.data);

    mem_free(g_app);

    g_init_mtx.trash();

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

    if (glfwWindowShouldClose(g_app->game_window)) game_quit();
}

static void _game_update() {
    assets_perform_hot_reload_changes();
    system_update_all();
}

static void _game_draw() {
    static bool first = true;

    lua_State *L = ENGINE_LUA();

    // don't draw first frame -- allow a full update
    if (first) {
        first = false;
        return;
    }

    // int width, height;
    // glfwGetWindowSize(g_app->game_window, &width, &height);
    // glViewport(0, 0, width, height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    imgui_draw_pre();

    // ImGui::ShowDemoWindow();

    system_draw_all();

    auto tex_aliens = texture_get_ptr("assets/aliens.png");

    struct {
        float x, y, w, h;
    } alien_uvs[] = {
            {2, 2, 24, 24}, {58, 2, 24, 24}, {114, 2, 24, 24}, {170, 2, 24, 24}, {2, 30, 24, 24},
    };

    batch_tex ch = {
            .px = 0,
            .py = 0,
            .tx = alien_uvs[1].x,
            .ty = alien_uvs[1].y,
            .tw = alien_uvs[1].w,
            .th = alien_uvs[1].h,
    };
    batch_test_draw(&renderer, tex_aliens, ch);

    batch_flush(&renderer);

    {
        if (ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko %d", neko_buildnum());

            if (g_app->debug_on) {
                // sgimgui_draw_menu(&sgimgui, "gfx");
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 275 - ImGui::GetScrollX());
            ImGui::Text("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(L, LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024), g_app->time.delta * 1000.f,
                        1.f / g_app->time.delta);

            ImGui::EndMainMenuBar();
        }

        for (uint32_t i = 0; i < neko_dyn_array_size(g_app->shader_array); ++i) {
            auto sp = g_app->shader_array[i];
            inspect_shader(sp.name, sp.id);
        }
    }

    imgui_draw_post();

    glfwSwapBuffers(g_app->game_window);
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

    _game_fini();
}

void game_set_bg_color(Color c) { glClearColor(c.r, c.g, c.b, 1.0); }

void game_set_window_size(CVec2 s) { glfwSetWindowSize(g_app->game_window, s.x, s.y); }

CVec2 game_get_window_size() {
    int w, h;
    glfwGetWindowSize(g_app->game_window, &w, &h);
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

int game_set_window_minsize(int width, int height) {
    glfwSetWindowSizeLimits(g_app->game_window, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    return 0;
}

int game_get_window_width(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (g_app->game_window) {
        glfwGetWindowSize(g_app->game_window, &w, &h);
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
    if (g_app->game_window) {
        glfwGetWindowSize(g_app->game_window, &w, &h);
    }
#endif
    *val = h;
    return 0;
}

int game_set_window_position(int x, int y) {
#if !defined(__EMSCRIPTEN__)
    if (g_app->game_window) {
        glfwSetWindowPos(g_app->game_window, x, y);
    }
#endif
    return 0;
}

static int get_current_monitor(GLFWmonitor **monitor, GLFWwindow *window) {
    int winpos[2] = {0};
    glfwGetWindowPos(window, &winpos[0], &winpos[1]);

    int monitors_size = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitors_size);

    for (int i = 0; i < monitors_size; ++i) {
        int monitorpos[2] = {0};
        glfwGetMonitorPos(monitors[i], &monitorpos[0], &monitorpos[1]);
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitors[i]);
        if (winpos[0] >= monitorpos[0] && winpos[0] < (monitorpos[0] + vidmode->width) && winpos[1] >= monitorpos[1] && winpos[1] < (monitorpos[1] + vidmode->height)) {
            *monitor = monitors[i];
            return 0;
        }
    }

    return 1;
}

int game_set_window_title(const char *title) {
#if defined(__EMSCRIPTEN__)
    emscripten_set_window_title(title);
#else
    glfwSetWindowTitle(g_app->game_window, title);
#endif
    return 0;
}

int game_set_window_vsync(bool vsync) {
    if (vsync) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    return 0;
}