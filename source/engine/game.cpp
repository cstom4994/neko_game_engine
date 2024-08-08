#include "engine/game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "engine/camera.h"
#include "engine/console.h"
#include "engine/glew_glfw.h"
#include "engine/inspector.h"
#include "engine/lite.h"
#include "engine/lua_util.h"
#include "engine/script.h"
#include "engine/sprite.h"
#include "engine/system.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "engine/ui.h"

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

void batch_test_draw(batch_renderer *renderer, Texture tex, batch_tex a) {
    batch_texture(renderer, tex.id);

    float x1 = a.px;
    float y1 = a.py;
    float x2 = a.px + 24;
    float y2 = a.py + 24;

    float u1 = a.tx / tex.width;
    float v1 = a.ty / tex.height;
    float u2 = (a.tx + a.tw) / tex.width;
    float v2 = (a.ty + a.th) / tex.height;

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

batch_renderer renderer;

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

    console_log("opengl vendor:   %s", glGetString(GL_VENDOR));
    console_log("opengl renderer: %s", glGetString(GL_RENDERER));
    console_log("opengl version:  %s", glGetString(GL_VERSION));

    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    console_log("opengl maximum texture size: %d", max_texture_size);

    if (max_texture_size < 2048) console_log("opengl maximum texture too small");

    // some GL settings
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.95f, 0.95f, 0.95f, 1.f);

    // random seed
    srand(time(NULL));

    // time set
    timing_instance.startup = stm_now();
    timing_instance.last = stm_now();

    // init systems
    console_puts("welcome to neko!");
    system_init();

    assets_start_hot_reload();

    renderer = batch_init(6000);

    asset_load(AssetLoadData{AssetKind_Image, false}, "assets/aliens.png", NULL);

    if (g_app->lite_init_path.len) {
        PROFILE_BLOCK("lite init");
        g_app->lite_L = neko::neko_lua_create();

        // lua_register(g_app->lite_L, "__neko_loader", neko::vfs_lua_loader);
        // const_str str = "table.insert(package.searchers, 2, __neko_loader) \n";
        // luaL_dostring(g_app->lite_L, str);

        lt_init(g_app->lite_L, g_app->game_window, g_app->lite_init_path.cstr(), __argc, __argv, window_scale(), "Windows");
    }
}

static void _game_fini() {
    PROFILE_FUNC();

    mem_free(renderer.vertices);

    neko::neko_lua_fini(g_app->lite_L);

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

    unsigned lt_none = 0u;
    unsigned lt_all = ~0u;
    lt_events = lt_none;

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

    ch = {
            .px = 0,
            .py = 48,
            .tx = alien_uvs[2].x,
            .ty = alien_uvs[2].y,
            .tw = alien_uvs[2].w,
            .th = alien_uvs[2].h,
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
            ImGui::Text("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(L, LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024), timing_instance.true_dt * 1000.f,
                        1.f / timing_instance.true_dt);

            ImGui::EndMainMenuBar();
        }

        for (uint32_t i = 0; i < neko_dyn_array_size(g_app->shader_array); ++i) {
            auto sp = g_app->shader_array[i];
            inspect_shader(sp.name, sp.id);
        }

        if (g_app->lite_init_path.len && g_app->lite_L) {

            if (ImGui::Begin("Lite")) {
                ImVec2 bounds = ImGui::GetContentRegionAvail();
                ImVec2 mouse_pos = ImGui::GetCursorPos();  // 窗口内鼠标坐标
                lt_mx = mouse_pos.x;
                lt_my = mouse_pos.y;
                lt_wx = 0;
                lt_wy = 0;
                lt_ww = bounds.x;
                lt_wh = bounds.y;

                if (lt_resizesurface(lt_getsurface(0), lt_ww, lt_wh)) {
                    // glfw_wrap__window_refresh_callback(g_app->game_window);
                }
                // fullscreen_quad_rgb( lt_getsurface(0)->t, 1.2f );
                // ui_texture_fit(lt_getsurface(0)->t, bounds);

                ImGui::Image((ImTextureID)lt_getsurface(0)->t.id, bounds);

                // if (!!nk_input_is_mouse_hovering_rect(&ui_ctx->input, ((struct nk_rect){lt_wx + 5, lt_wy + 5, lt_ww - 10, lt_wh - 10}))) {
                //     lt_events &= ~(1 << 31);
                // }
            }
            ImGui::End();
        }
    }

    if (g_app->lite_init_path.len && g_app->lite_L) lt_tick(g_app->lite_L);

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

const char *window_clipboard() { return glfwGetClipboardString(g_app->game_window); }

void window_setclipboard(const char *text) { glfwSetClipboardString(g_app->game_window, text); }

void window_focus() { glfwFocusWindow(g_app->game_window); }

int window_has_focus() { return !!glfwGetWindowAttrib(g_app->game_window, GLFW_FOCUSED); }

double window_scale() {
    float xscale = 1, yscale = 1;
#ifndef NEKO_IS_APPLE  // @todo: remove silicon mac M1 hack
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
#endif
    return NEKO_MAX(xscale, yscale);
}

void test_native_script() {
    Entity camera, block, player;
    unsigned int i, n_blocks;

    // add camera

    camera = entity_create();

    transform_add(camera);

    camera_add(camera);

    // add some blocks

    n_blocks = rand() % 50;
    for (i = 0; i < n_blocks; ++i) {
        block = entity_create();

        transform_add(block);
        transform_set_position(block, vec2((rand() % 25) - 12, (rand() % 9) - 4));

        sprite_add(block);
        sprite_set_texcell(block, vec2(32.0f, 32.0f));
        sprite_set_texsize(block, vec2(32.0f, 32.0f));
    }

    // add player

    player = entity_create();

    transform_add(player);
    transform_set_position(player, vec2(0.0f, 0.0f));

    sprite_add(player);
    sprite_set_texcell(player, vec2(0.0f, 32.0f));
    sprite_set_texsize(player, vec2(32.0f, 32.0f));

    // who gets keyboard control?
    keyboard_controlled_add(camera);
}

AppTime timing_instance;

static f32 scale = 1.0f;
static bool paused = false;

void timing_set_scale(f32 s) { scale = s; }
f32 timing_get_scale() { return scale; }

void timing_set_paused(bool p) { paused = p; }
bool timing_get_paused() { return paused; }

static void _dt_update() {
    static double last_time = -1;
    double curr_time;

    // first update?
    if (last_time < 0) last_time = glfwGetTime();

    curr_time = glfwGetTime();
    timing_instance.true_dt = curr_time - last_time;
    timing_instance.dt = paused ? 0.0f : scale * timing_instance.true_dt;
    last_time = curr_time;

    {
        AppTime *time = &timing_instance;

#if !defined(NEKO_IS_WEB)
        if (time->target_ticks > 0) {
            u64 TICK_MS = 1000000;
            u64 TICK_US = 1000;

            u64 target = time->target_ticks;

            if (time->accumulator < target) {
                u64 ms = (target - time->accumulator) / TICK_MS;
                if (ms > 0) {
                    PROFILE_BLOCK("sleep");
                    os_sleep(ms - 1);
                }

                {
                    PROFILE_BLOCK("spin loop");

                    u64 lap = stm_laptime(&time->last);
                    time->delta += stm_sec(lap);
                    time->accumulator += lap;

                    while (time->accumulator < target) {
                        os_yield();

                        u64 lap = stm_laptime(&time->last);
                        time->delta += stm_sec(lap);
                        time->accumulator += lap;
                    }
                }
            }

            u64 fuzz = TICK_US * 100;
            while (time->accumulator >= target - fuzz) {
                if (time->accumulator < target + fuzz) {
                    time->accumulator = 0;
                } else {
                    time->accumulator -= target + fuzz;
                }
            }
        }
#endif
    }
}

void timing_update() { _dt_update(); }

void timing_save_all(Store *s) {
    Store *t;

    if (store_child_save(&t, "timing", s)) scalar_save(&scale, "scale", t);
}
void timing_load_all(Store *s) {
    Store *t;

    if (store_child_load(&t, "timing", s)) scalar_load(&scale, "scale", 1, t);
}