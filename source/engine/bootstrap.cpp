
#include "engine/bootstrap.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/event.h"
#include "engine/graphics.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/scripting.h"
#include "engine/ui.h"


// deps
#include "vendor/sokol_time.h"

#define REFL_FIELDS(C, field) type.fields.insert({#field, {type_of<decltype(C::field)>(), offsetof(C, field)}})

// clang-format off

REGISTER_TYPE_DF(engine_cfg_t, 
REFL_FIELDS(engine_cfg_t, show_editor); 
REFL_FIELDS(engine_cfg_t, show_demo_window); 
REFL_FIELDS(engine_cfg_t, show_gui); 
REFL_FIELDS(engine_cfg_t, shader_inspect);
REFL_FIELDS(engine_cfg_t, hello_ai_shit); 
REFL_FIELDS(engine_cfg_t, vsync); 
REFL_FIELDS(engine_cfg_t, is_hotfix); 
REFL_FIELDS(engine_cfg_t, title); 
REFL_FIELDS(engine_cfg_t, height);
REFL_FIELDS(engine_cfg_t, width); 
REFL_FIELDS(engine_cfg_t, game_proxy); 
REFL_FIELDS(engine_cfg_t, default_font); 
REFL_FIELDS(engine_cfg_t, dump_allocs_detailed);
REFL_FIELDS(engine_cfg_t, hot_reload);
REFL_FIELDS(engine_cfg_t, startup_load_scripts);
REFL_FIELDS(engine_cfg_t, fullscreen);
REFL_FIELDS(engine_cfg_t, debug_on);
REFL_FIELDS(engine_cfg_t, reload_interval);
REFL_FIELDS(engine_cfg_t, swap_interval);
REFL_FIELDS(engine_cfg_t, target_fps);
REFL_FIELDS(engine_cfg_t, batch_vertex_capacity);
);

// clang-format on

i32 neko_buildnum(void) {
    static const char *__build_date = __DATE__;
    static const char *mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char mond[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    i32 m = 0, d = 0, y = 0;
    static i32 b = 0;
    if (b != 0) return b;  // 优化
    for (m = 0; m < 11; m++) {
        if (!strncmp(&__build_date[0], mon[m], 3)) break;
        d += mond[m];
    }
    d += atoi(&__build_date[4]) - 1;
    y = atoi(&__build_date[7]) - 2023;
    b = d + (i32)((y - 1) * 365.25f);
    if (((y % 4) == 0) && m > 1) b += 1;
    b -= 211;
    return b;
}

// ECS_COMPONENT_DECL(pos_t);
// ECS_COMPONENT_DECL(vel_t);
// ECS_COMPONENT_DECL(rect_t);

App *g_app;

Allocator *g_allocator;

int main(int argc, char **argv) {
    game_run(argc, argv);
    return 0;
}

extern void draw_gui();

// -------------------------------------------------------------------------

static const char *scratch_filename = usr_path("scratch.lua");

bool _exists() {
    struct stat st;
    return stat(scratch_filename, &st) == 0;
}

// 自上次调用 _modified() 后是否已修改 一开始为 false
bool _modified() {
    struct stat st;
    static time_t prev_time = 0;
    bool modified;
    static bool first = true;

    if (stat(scratch_filename, &st) != 0) {
        first = false;
        return false;  // 不存在或错误
    }

    modified = !first && st.st_mtime != prev_time;
    prev_time = st.st_mtime;
    first = false;
    return modified;
}

void scratch_run() { script_run_file(scratch_filename); }

int scratch_update(App *app, event_t evt) {
    if (_modified()) scratch_run();
    return 0;
}

// -------------------------------------------------------------------------

static void _glfw_error_callback(int error, const char *desc) { fprintf(stderr, "glfw: %s\n", desc); }

static void _opengl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    g_app->cfg.width = width;
    g_app->cfg.height = height;
    // 更新视口
    glViewport(0, 0, width, height);
}

int _game_draw(App *app, event_t evt) {
    static bool first = true;

    lua_State *L = ENGINE_LUA();

    // 不绘制第一帧 等待完整更新
    if (first) {
        first = false;
        return 0;
    }

    luax_neko_get(L, "__timer_update");
    lua_pushnumber(L, get_timing_instance()->delta);
    luax_pcall(L, 1, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    imgui_draw_pre();

    if (!g_app->error_mode.load()) {

        script_draw_all();

        // 底层图片
        char background_text[64] = "Project: unknown";

        f32 td = g_app->default_font->width(22.f, background_text);
        // vec2 td = {};
        vec2 ts = neko_v2(512 + 128, 512 + 128);

        // idraw_text(&g_app->idraw, (g_app->width - td) * 0.5f, (g_app->height) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, color256(255, 255, 255, 255));
        // idraw_texture(&g_app->idraw, test_ase);
        // idraw_rectvd(&g_app->idraw, neko_v2((g_app->width - ts.x) * 0.5f, (g_app->height - ts.y) * 0.5f - 22.f - 50.f), ts, neko_v2(0.f, 1.f), neko_v2(1.f, 0.f), color256(255, 255, 255, 255),
        //              R_PRIMITIVE_TRIANGLES);

        // idraw_defaults(&g_app->idraw);
        {

            // script_draw_all();
            // tiled_draw_all();
            tiled_draw_all();

            sprite_draw_all();
            batch_draw_all(g_app->batch);
            edit_draw_all();
            physics_draw_all();
            gui_draw_all();

            f32 fy = draw_font(g_app->default_font, false, 16.f, 0.f, 0.f, "Hello World 测试中文，你好世界", NEKO_COLOR_WHITE);
            fy += draw_font(g_app->default_font, false, 16.f, 0.f, 20.f, "我是第二行", NEKO_COLOR_WHITE);
            fy += draw_font(g_app->default_font, true, 16.f, 0.f, 20.f, "这一行字 draw_in_world", NEKO_COLOR_WHITE);
        }

        auto ui = g_app->ui;

        // ImGui::ShowDemoWindow();

        neko_update_ui(ui);

        DeferLoop(ui_begin(ui), ui_end(ui)) {
            script_draw_ui();
            draw_gui();
        }

        neko_render_ui(ui, g_app->cfg.width, g_app->cfg.height);

    } else {

        auto font = neko_default_font();

        float x = 20;
        float y = 25;
        u64 font_size = 28;

        if (LockGuard<Mutex> lock{g_app->error_mtx}) {
            y = draw_font(font, false, font_size, x, y, "-- ! Neko Error ! --", NEKO_COLOR_WHITE);
            y += font_size;

            y = draw_font_wrapped(font, false, font_size, x, y, g_app->fatal_error, NEKO_COLOR_WHITE, g_app->cfg.width - x);
            y += font_size;

            if (g_app->traceback.data) {
                y = draw_font(font, false, font_size, x, y, g_app->traceback, NEKO_COLOR_WHITE);
                y += (font_size * 2);

                draw_font(font, false, font_size, x, y, "按下 Ctrl+C 复制以上堆栈信息", NEKO_COLOR_WHITE);

                if (input_key_down(KC_LEFT_CONTROL) && input_key_down(KC_C)) {
                    window_setclipboard(g_app->traceback.cstr());
                }
            }
        }
    }

    imgui_draw_post();

    neko_check_gl_error();

    glfwSwapBuffers(g_app->game_window);

    return 0;
}

static void _game_init(int argc, char **argv) {

#ifndef NDEBUG
    g_allocator = new DebugAllocator();
#else
    g_allocator = new HeapAllocator();
#endif

    g_allocator->make();

    g_app = new (mem_alloc(sizeof(App))) App();

    LockGuard<Mutex> lock(g_app->g_init_mtx);

    os_high_timer_resolution();
    stm_setup();

    g_app->args.resize(argc);
    for (i32 i = 0; i < argc; i++) {
        g_app->args[i] = to_cstr(argv[i]);
    }
    g_app->argc = argc;
    g_app->argv = argv;

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_app->game_window = glfwCreateWindow(800, 600, "neko_game", NULL, NULL);

    // activate OpenGL context
    glfwMakeContextCurrent(g_app->game_window);

    // 注册窗口大小改变的回调函数
    glfwSetFramebufferSizeCallback(g_app->game_window, framebuffer_size_callback);

    // initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        console_log("glewInit failed: ", glewGetErrorString(err));
    }
    bool gl_dsa_support = glewIsSupported("GL_ARB_direct_state_access");

#if defined(_DEBUG) && !defined(NEKO_IS_APPLE)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(_opengl_error_callback, 0);
#endif

    // some GL settings
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);

    // random seed
    srand(time(NULL));

    // time set
    get_timing_instance()->startup = stm_now();
    get_timing_instance()->last = stm_now();

    // init systems
    console_puts("welcome to neko!");
    system_init();

    assets_start_hot_reload();

    {  // just for test

        g_app->test_ase = neko_aseprite_simple("assets/cat.ase");

        g_app->ui = (ui_context_t *)mem_alloc(sizeof(ui_context_t));
        ui_init(g_app->ui);

        g_app->ui->style->colors[UI_COLOR_WINDOWBG] = color256(50, 50, 50, 200);

        g_app->ui->text_width = neko_ui_text_width;
        g_app->ui->text_height = neko_ui_text_height;
        neko_init_ui_renderer();
    }

    eventhandler_t *eh = eventhandler_instance();

    struct {
        int evt;
        evt_callback_t cb;
    } evt_list[] = {
            {preupdate, (evt_callback_t)assets_perform_hot_reload_changes},  //
            {preupdate, (evt_callback_t)edit_clear},                         //
            {preupdate, (evt_callback_t)timing_update},                      //

            {update, (evt_callback_t)scratch_update},
            {update, (evt_callback_t)script_update_all},
            {update, (evt_callback_t)keyboard_controlled_update_all},
            {update, (evt_callback_t)physics_update_all},
            {update, (evt_callback_t)transform_update_all},

            {update, (evt_callback_t)camera_update_all},
            {update, (evt_callback_t)gui_update_all},
            {update, (evt_callback_t)sprite_update_all},
            {update, (evt_callback_t)batch_update_all},
            {update, (evt_callback_t)sound_update_all},
            {update, (evt_callback_t)tiled_update_all},
            {update, (evt_callback_t)edit_update_all},

            {postupdate, (evt_callback_t)script_post_update_all},
            {postupdate, (evt_callback_t)physics_post_update_all},
            {postupdate, (evt_callback_t)sound_postupdate},
            {postupdate, (evt_callback_t)entity_update_all},
            {postupdate, (evt_callback_t)gui_event_clear},

            {draw, (evt_callback_t)_game_draw},
    };

    for (auto evt : evt_list) {
        event_register(eh, g_app, evt.evt, evt.cb, NULL);
    }
}

static void _game_fini() {
    PROFILE_FUNC();

    bool dump_allocs_detailed = g_app->cfg.dump_allocs_detailed;

    {  // just for test

        mem_free(g_app->ui);
        neko_deinit_ui_renderer();
    }

    // fini systems
    system_fini();

    // fini glfw
    glfwDestroyWindow(g_app->game_window);
    glfwTerminate();

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

    eventhandler_fini(eventhandler_instance());

    g_app->~App();
    mem_free(g_app);

#ifndef NDEBUG
    DebugAllocator *allocator = dynamic_cast<DebugAllocator *>(g_allocator);
    if (allocator != nullptr) {
        allocator->dump_allocs(dump_allocs_detailed);
    }
#endif

    g_allocator->trash();
    operator delete(g_allocator);

    neko_println("see ya");
}

// -------------------------------------------------------------------------

void game_run(int argc, char **argv) {
    _game_init(argc, argv);
    while (!g_app->g_quit) {
        App *app = g_app;
        eventhandler_t *handler = eventhandler_instance();
        glfwPollEvents();
        if (glfwWindowShouldClose(g_app->game_window)) game_quit();
        event_pump(handler);

        event_dispatch(handler, event_t{.type = on_preupdate});
        if (!g_app->error_mode.load()) {
            event_dispatch(handler, event_t{.type = on_update});
        }
        event_dispatch(handler, event_t{.type = on_postupdate});

        event_dispatch(handler, event_t{.type = on_draw});
    }
    _game_fini();
}

void game_set_bg_color(Color c) { glClearColor(c.r, c.g, c.b, 1.0); }

void game_set_window_size(vec2 s) { glfwSetWindowSize(g_app->game_window, s.x, s.y); }

vec2 game_get_window_size() {
    int w, h;
    glfwGetWindowSize(g_app->game_window, &w, &h);
    return luavec2(w, h);
}
vec2 game_unit_to_pixels(vec2 p) {
    vec2 hw = vec2_scalar_mul(game_get_window_size(), 0.5f);
    p = vec2_mul(p, hw);
    p = luavec2(p.x + hw.x, p.y - hw.y);
    return p;
}
vec2 game_pixels_to_unit(vec2 p) {
    vec2 hw = vec2_scalar_mul(game_get_window_size(), 0.5f);
    p = luavec2(p.x - hw.x, p.y + hw.y);
    p = vec2_div(p, hw);
    return p;
}

void game_quit() { g_app->g_quit = true; }

int game_get_argc() { return g_app->argc; }
char **game_get_argv() { return g_app->argv; }

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
    NativeEntity camera, block, player;
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
        transform_set_position(block, luavec2((rand() % 25) - 12, (rand() % 9) - 4));

        sprite_add(block);
        sprite_set_texcell(block, luavec2(32.0f, 32.0f));
        sprite_set_texsize(block, luavec2(32.0f, 32.0f));
    }

    // add player

    player = entity_create();

    transform_add(player);
    transform_set_position(player, luavec2(0.0f, 0.0f));

    sprite_add(player);
    sprite_set_texcell(player, luavec2(0.0f, 32.0f));
    sprite_set_texsize(player, luavec2(32.0f, 32.0f));

    // who gets keyboard control?
    // keyboard_controlled_add(camera);
}

AppTime *get_timing_instance() { return &g_app->timing_instance; }

f32 timing_get_elapsed() { return glfwGetTime() * 1000.0f; }

void timing_set_scale(f32 s) { g_app->scale = s; }
f32 timing_get_scale() { return g_app->scale; }

void timing_set_paused(bool p) { g_app->paused = p; }
bool timing_get_paused() { return g_app->paused; }

int timing_update(App *app, event_t evt) {

    // update dt
    static double last_time = -1;
    double curr_time;

    // first update?
    if (last_time < 0) last_time = glfwGetTime();

    curr_time = glfwGetTime();
    get_timing_instance()->true_dt = curr_time - last_time;
    get_timing_instance()->dt = g_app->paused ? 0.0f : g_app->scale * get_timing_instance()->true_dt;
    last_time = curr_time;

    {
        AppTime *time = get_timing_instance();

#if 0
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

    return 0;
}

void timing_save_all(Store *s) {
    Store *t;

    if (store_child_save(&t, "timing", s)) scalar_save(&g_app->scale, "scale", t);
}
void timing_load_all(Store *s) {
    Store *t;

    if (store_child_load(&t, "timing", s)) scalar_load(&g_app->scale, "scale", 1, t);
}