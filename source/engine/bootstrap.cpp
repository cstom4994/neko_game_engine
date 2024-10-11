
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
#include "engine/base.hpp"
#include "engine/base/os.hpp"
#include "engine/base/profiler.hpp"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/event.h"
#include "engine/graphics.h"
#include "engine/imgui.hpp"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/nativescript.hpp"
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

void fatal_error(String str) {
    if (!g_app->error_mode.load()) {
        LockGuard<Mutex> lock{g_app->error_mtx};

        g_app->fatal_error = to_cstr(str);
        fprintf(stderr, "%s\n", g_app->fatal_error.data);
        g_app->error_mode.store(true);
    }
}

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

native_script_context_t *sc;

unsigned int fbo;
unsigned int rbo;
unsigned int fbo_tex;
unsigned int quadVAO, quadVBO;
Asset posteffect_shader = {};

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

static void __stdcall gl_debug_callback(u32 source, u32 type, u32 id, u32 severity, i32 length, const char *message, const void *up) {

    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    const char *s;
    const char *t;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            s = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            s = "window system";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            s = "shader compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            s = "third party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            s = "application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            s = "other";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            t = "type error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            t = "deprecated behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            t = "undefined behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            t = "portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            t = "performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            t = "marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            t = "push group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            t = "pop group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            t = "other";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        case GL_DEBUG_SEVERITY_MEDIUM:
            console_log("OpenGL (source: %s; type: %s): %s", s, t, message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            console_log("OpenGL (source: %s; type: %s): %s", s, t, message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            console_log("OpenGL (source: %s; type: %s): %s", s, t, message);
            break;
    }
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

    if (!g_app->error_mode.load()) {

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        imgui_draw_pre();

        if (ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko %d", neko_buildnum());

            // if (g_app->debug_on) {
            // sgimgui_draw_menu(&sgimgui, "gfx");
            // }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 275 - ImGui::GetScrollX());
            ImGui::Text("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(L, LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024), get_timing_instance()->true_dt * 1000.f,
                        1.f / get_timing_instance()->true_dt);

            ImGui::EndMainMenuBar();
        }

        neko::imgui::perf();

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
            tiled_draw_all();

            script_draw_all();

            sprite_draw_all();
            batch_draw_all(g_app->batch);
            edit_draw_all();
            physics_draw_all();
            gui_draw_all();

            // 现在绑定回默认帧缓冲区并使用附加的帧缓冲区颜色纹理绘制一个四边形平面
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);  // 禁用深度测试，以便屏幕空间四边形不会因深度测试而被丢弃。
            // 清除所有相关缓冲区
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // 将透明颜色设置为白色
            glClear(GL_COLOR_BUFFER_BIT);

            GLuint sid = posteffect_shader.shader.id;
            glUseProgram(sid);

            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, fbo_tex);  // 使用颜色附件纹理作为四边形平面的纹理
            glDrawArrays(GL_TRIANGLES, 0, 6);

            f32 fy = draw_font(g_app->default_font, false, 16.f, 0.f, 20.f, "Hello World 测试中文，你好世界", NEKO_COLOR_WHITE);
            fy += draw_font(g_app->default_font, false, 16.f, 0.f, fy, "我是第二行", NEKO_COLOR_WHITE);
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

        imgui_draw_post();

    } else {

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

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

    neko_check_gl_error();

    glfwSwapBuffers(g_app->game_window);

    return 0;
}

void bootstrap_splash() {

    if (1) {

        String contents = {};
        bool ok = vfs_read_entire_file(&contents, "assets/splash.png");
        if (!ok) {
            return;
        }
        neko_defer(mem_free(contents.data));

        neko_texture_t *splash_texture = neko_new_texture_from_memory(contents.data, contents.len, NEKO_TEXTURE_ANTIALIASED);

        // neko_shader_t *shader = neko_load_shader(cfg.splash_shader);
        Asset splash_shader = {};

        ok = asset_load_kind(AssetKind_Shader, "shader/splash.glsl", &splash_shader);
        error_assert(ok);

        GLuint sid = splash_shader.shader.id;

        float verts[] = {/* position     UV */
                         0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, 0.5f, 0.0f, 0.0f};

        u32 indices[] = {0, 1, 3, 1, 2, 3};

        neko_vertex_buffer_t *quad = neko_new_vertex_buffer(neko_vertex_buffer_flags_t(NEKO_VERTEXBUFFER_STATIC_DRAW | NEKO_VERTEXBUFFER_DRAW_TRIANGLES));

        neko_bind_vertex_buffer_for_edit(quad);
        neko_push_vertices(quad, verts, sizeof(verts) / sizeof(float));
        neko_push_indices(quad, indices, sizeof(indices) / sizeof(u32));
        neko_configure_vertex_buffer(quad, 0, 2, 4, 0);
        neko_configure_vertex_buffer(quad, 1, 2, 4, 2);

        mat4 projection = mat4_ortho(-((float)g_app->cfg.width / 2.0f), (float)g_app->cfg.width / 2.0f, (float)g_app->cfg.height / 2.0f, -((float)g_app->cfg.height / 2.0f), -1.0f, 1.0f);
        mat4 model = mat4_scalev(vec3{splash_texture->width / 2.0f, splash_texture->height / 2.0f, 0.0f});

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        neko_bind_shader(sid);
        neko_bind_texture(splash_texture, 0);
        neko_shader_set_int(sid, "image", 0);
        neko_shader_set_m4f(sid, "transform", model);
        neko_shader_set_m4f(sid, "projection", projection);

        neko_bind_vertex_buffer_for_draw(quad);
        neko_draw_vertex_buffer(quad);

        // neko_update_application();
        glfwSwapBuffers(g_app->game_window);

        neko_free_vertex_buffer(quad);

        neko_free_texture(splash_texture);

        neko_check_gl_error();
    }
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_callback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
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

    neko::modules::initialize<EventHandler>();

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

    auto &eh = neko::the<EventHandler>();

    struct {
        int evt;
        evt_callback_t cb;
    } evt_list[] = {
            {preupdate, (evt_callback_t)assets_perform_hot_reload_changes},  //
            {preupdate, (evt_callback_t)edit_clear},                         //
            {preupdate, (evt_callback_t)timing_update},                      //

            {update, (evt_callback_t)scratch_update},
            {update, (evt_callback_t)script_update_all},
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
        eh.event_register(g_app, evt.evt, evt.cb, NULL);
    }

    sc = native_new_script_context(g_app, "game_debug_x64.dll");

    native_new_script(sc, {1}, "get_test_script_instance_size", "on_test_script_init", "on_test_script_update", "on_physics_update_name", "on_test_script_free", false);

    native_init_scripts(sc);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        console_log("framebuffer good");
    }

    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_app->cfg.width, g_app->cfg.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_app->cfg.width, g_app->cfg.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

    float quadVertices[] = {// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                            // positions   // texCoords
                            -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    bool ok = asset_load_kind(AssetKind_Shader, "shader/posteffect.glsl", &posteffect_shader);
    error_assert(ok);

    neko_check_gl_error();
}

static void _game_fini() {
    PROFILE_FUNC();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fbo_tex);
    glDeleteRenderbuffers(1, &rbo);

    bool dump_allocs_detailed = g_app->cfg.dump_allocs_detailed;

    {  // just for test

        mem_free(g_app->ui);
        neko_deinit_ui_renderer();
    }

    native_free_scripts(sc);

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

    auto &eh = neko::the<EventHandler>();
    eh.fini();

    neko::modules::shutdown<EventHandler>();

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
        auto &eh = neko::the<EventHandler>();
        glfwPollEvents();
        if (glfwWindowShouldClose(g_app->game_window)) game_quit();
        eh.event_pump();

        eh.event_dispatch(event_t{.type = on_preupdate});
        if (!g_app->error_mode.load()) {
            eh.event_dispatch(event_t{.type = on_update});
        }
        eh.event_dispatch(event_t{.type = on_postupdate});

        eh.event_dispatch(event_t{.type = on_draw});
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