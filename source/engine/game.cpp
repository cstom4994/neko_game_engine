#include "engine/game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "engine/batch.h"
#include "engine/camera.h"
#include "engine/console.h"
#include "engine/draw.h"
#include "engine/font.h"
#include "engine/gfx.h"
#include "engine/glew_glfw.h"
#include "engine/lite.h"
#include "engine/lua_util.h"
#include "engine/script.h"
#include "engine/sprite.h"
#include "engine/system.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "engine/ui.h"
#include "engine/vfs.h"

// deps
#include <cute_aseprite.h>

#include "vendor/sokol_time.h"

static bool quit = false;  // 如果为 true 则退出主循环
static int sargc = 0;
static char **sargv;
static Mutex g_init_mtx;

neko_texture_t test_ase;

extern void draw_gui();

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

// -------------------------------------------------------------------------

static void _glfw_error_callback(int error, const char *desc) { fprintf(stderr, "glfw: %s\n", desc); }

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    g_app->width = width;
    g_app->height = height;
    // 更新视口
    glViewport(0, 0, width, height);
}

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
    glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);

    // random seed
    srand(time(NULL));

    // time set
    timing_instance.startup = stm_now();
    timing_instance.last = stm_now();

    // init systems
    console_puts("welcome to neko!");
    system_init();

    g_app->cb = command_buffer_new();
    g_app->idraw = neko_immediate_draw_new();

    assets_start_hot_reload();

    if (g_app->lite_init_path.len) {
        PROFILE_BLOCK("lite init");
        g_app->lite_L = neko::neko_lua_create();

        // lua_register(g_app->lite_L, "__neko_loader", neko::vfs_lua_loader);
        // const_str str = "table.insert(package.searchers, 2, __neko_loader) \n";
        // luaL_dostring(g_app->lite_L, str);

        lt_init(g_app->lite_L, g_app->game_window, g_app->lite_init_path.cstr(), __argc, __argv, window_scale(), "Windows");
    }

    {  // just for test

        test_ase = neko_aseprite_simple("assets/cat.ase");

        ui_init(&g_app->ui, 0);

        // 加载自定义字体文件 初始化 gui font stash
        // neko_asset_font_load_from_memory(font_data, font_data_size, &CL_GAME_INTERFACE()->font, 24);
        // neko_asset_font_load_from_file("gamedir/assets/fonts/monocraft.ttf", &font, 24);

        // pack.free_item(font_data);
        // pack.free_item(cat_data);

        // auto GUI_FONT_STASH = []() -> neko_ui_font_stash_desc_t * {
        //     static neko_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &font}};
        //     static neko_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_ui_font_desc_t)};
        //     return &font_stash;
        // }();

        // neko_ui_init_font_stash(&g_app->ui, GUI_FONT_STASH);

        ui_dock_ex(&g_app->ui, "Style_Editor", "Demo_Window", UI_SPLIT_TAB, 0.5f);
    }
}

static void _game_fini() {
    PROFILE_FUNC();

    {  // just for test

        ui_free(&g_app->ui);
    }

    if (g_app->lite_init_path.len) {
        lt_fini();
        neko::neko_lua_fini(g_app->lite_L);
    }

    neko_immediate_draw_static_data_free();
    neko_immediate_draw_free(&g_app->idraw);
    command_buffer_free(&g_app->cb);

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
        allocator->dump_allocs(false);
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

    // 不绘制第一帧 等待完整更新
    if (first) {
        first = false;
        return;
    }

    luax_neko_get(L, "__timer_update");
    lua_pushnumber(L, timing_instance.delta);
    luax_pcall(L, 1, 0);

    // int width, height;
    // glfwGetWindowSize(g_app->game_window, &width, &height);
    // glViewport(0, 0, width, height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!g_app->error_mode.load()) {

        imgui_draw_pre();

        {

            // if (g_app->lite_init_path.len && g_app->lite_L) {
            //     if (ImGui::Begin("Lite")) {
            //         ImGuiWindow *window = ImGui::GetCurrentWindow();
            //         ImVec2 bounds = ImGui::GetContentRegionAvail();
            //         LuaVec2 mouse_pos = input_get_mouse_pos_pixels_fix();  // 窗口内鼠标坐标
            //         neko_assert(window);
            //         ImVec2 pos = window->Pos;
            //         ImVec2 size = window->Size;
            //         lt_mx = mouse_pos.x - pos.x;
            //         lt_my = mouse_pos.y - pos.y;
            //         lt_wx = pos.x;
            //         lt_wy = pos.y;
            //         lt_ww = size.x;
            //         lt_wh = size.y;
            //         if (lt_resizesurface(lt_getsurface(0), lt_ww, lt_wh)) {
            //             // glfw_wrap__window_refresh_callback(g_app->game_window);
            //         }
            //         // fullscreen_quad_rgb( lt_getsurface(0)->t, 1.2f );
            //         // ui_texture_fit(lt_getsurface(0)->t, bounds);
            //         ImGui::Image((ImTextureID)lt_getsurface(0)->t.id, bounds);
            //         // if (!!nk_input_is_mouse_hovering_rect(&g_app->ui_ctx->input, ((struct nk_rect){lt_wx + 5, lt_wy + 5, lt_ww - 10, lt_wh - 10}))) {
            //         //     lt_events &= ~(1 << 31);
            //         // }
            //     }
            //     ImGui::End();
            // }
        }

#if 1
        // gfx_clear_action_t clear = {.color = {NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f}};
        // gfx_renderpass_begin(&g_app->cb, neko_renderpass_t{0});
        // { gfx_clear(&g_app->cb, clear); }
        // gfx_renderpass_end(&g_app->cb);

        DeferLoop(ui_begin(&g_app->ui, NULL), ui_end(&g_app->ui, true)) {

            script_draw_ui();

            draw_gui();
        }

        script_draw_all();

        // Set up 2D camera for projection matrix
        idraw_defaults(&g_app->idraw);
        idraw_camera2d(&g_app->idraw, (u32)g_app->width, (u32)g_app->height);

        // 底层图片
        char background_text[64] = "Project: unknown";

        f32 td = g_app->default_font->width(22.f, background_text);
        // vec2 td = {};
        vec2 ts = neko_v2(512 + 128, 512 + 128);

        idraw_text(&g_app->idraw, (g_app->width - td) * 0.5f, (g_app->height) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, color256(255, 255, 255, 255));
        idraw_texture(&g_app->idraw, test_ase);
        idraw_rectvd(&g_app->idraw, neko_v2((g_app->width - ts.x) * 0.5f, (g_app->height - ts.y) * 0.5f - 22.f - 50.f), ts, neko_v2(0.f, 1.f), neko_v2(1.f, 0.f), color256(255, 255, 255, 255),
                     R_PRIMITIVE_TRIANGLES);

        idraw_defaults(&g_app->idraw);
        f32 fy = draw_font(&g_app->idraw, g_app->default_font, 40.f, 10.f, 20.f, "-- ! Neko ! --", NEKO_COLOR_WHITE);

        gfx_renderpass_begin(&g_app->cb, R_RENDER_PASS_DEFAULT);
        {
            // gfx_set_viewport(&g_app->cb, 0, 0, (u32)g_app->width, (u32)g_app->height);
            idraw_draw(&g_app->idraw, &g_app->cb);  // 立即模式绘制 idraw

            tiled_draw_all();

            gfx_draw_func(&g_app->cb, system_draw_all);

            ui_render(&g_app->ui, &g_app->cb);
        }
        gfx_renderpass_end(&g_app->cb);

        gfx_cmd_submit(&g_app->cb);
#else
        system_draw_all();
#endif

        if (g_app->lite_init_path.len && g_app->lite_L) lt_tick(g_app->lite_L);

        imgui_draw_post();
    }

    neko_check_gl_error();

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

void game_set_window_size(LuaVec2 s) { glfwSetWindowSize(g_app->game_window, s.x, s.y); }

LuaVec2 game_get_window_size() {
    int w, h;
    glfwGetWindowSize(g_app->game_window, &w, &h);
    return luavec2(w, h);
}
LuaVec2 game_unit_to_pixels(LuaVec2 p) {
    LuaVec2 hw = vec2_scalar_mul(game_get_window_size(), 0.5f);
    p = vec2_mul(p, hw);
    p = luavec2(p.x + hw.x, p.y - hw.y);
    return p;
}
LuaVec2 game_pixels_to_unit(LuaVec2 p) {
    LuaVec2 hw = vec2_scalar_mul(game_get_window_size(), 0.5f);
    p = luavec2(p.x - hw.x, p.y + hw.y);
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
    keyboard_controlled_add(camera);
}

AppTime timing_instance;

static f32 scale = 1.0f;
static bool paused = false;

f32 timing_get_elapsed() { return glfwGetTime() * 1000.0f; }

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