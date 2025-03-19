
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

#include "base/cbase.hpp"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/os.hpp"
#include "base/common/profiler.hpp"
#include "base/common/singleton.hpp"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/event.h"
#include "engine/graphics.h"
#include "engine/imgui.hpp"
#include "base/scripting/lua_wrapper.hpp"
#include "base/scripting/scripting.h"
#include "engine/ui.h"
#include "engine/renderer/renderer.h"
#include "editor/editor.hpp"
#include "base/common/math.hpp"
#include "engine/input.h"
#include "engine/window.h"

// deps
#include "extern/sokol_time.h"

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
REFL_FIELDS(engine_cfg_t, lite_init_path); 
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

App *gApp;

#if 1

CBase gBase;

#endif

#if 1

static inline void perf() {
    ImGuiIO &io = ImGui::GetIO();
    const ImVec2 screenSize = io.DisplaySize;

    ImDrawList *draw_list = ImGui::GetForegroundDrawList();

    const ImVec2 footer_size(20, 20);
    const ImVec2 footer_pos(screenSize.x - footer_size.x - 20, 20);
    const ImU32 background_color = IM_COL32(0, 0, 0, 102);

    draw_list->AddRectFilled(footer_pos, footer_pos + footer_size, background_color);

    Neko::imgui::OutlineText(draw_list, ImVec2(screenSize.x - 60, footer_pos.y + 3), "%.1f FPS", io.Framerate);
}

// ECS_COMPONENT_DECL(pos_t);
// ECS_COMPONENT_DECL(vel_t);
// ECS_COMPONENT_DECL(rect_t);

unsigned int fbo;
unsigned int rbo;
unsigned int fbo_tex;
unsigned int quadVAO, quadVBO;
Asset posteffect_shader = {};
Asset sprite_shader = {};

Renderer *renderer;

extern void draw_gui();

// -------------------------------------------------------------------------

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
            LOG_ERROR("OpenGL (source: {}; type: {}): {}", s, t, message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            LOG_ERROR("OpenGL (source: {}; type: {}): {}", s, t, message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            LOG_ERROR("OpenGL (source: {}; type: {}): {}", s, t, message);
            break;
    }
}

void rescale_framebuffer(float width, float height) {
    if (fbo_tex == 0 || rbo == 0) return;

    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    gApp->cfg.width = width;
    gApp->cfg.height = height;

    rescale_framebuffer(width, height);

    // 更新视口
    glViewport(0, 0, width, height);
}

float posteffect_intensity = 2.0f;
int posteffect_enable = 1;

int _game_draw(App *app, event_t evt) {

    auto &game = Neko::the<Game>();

    lua_State *L = ENGINE_LUA();

    luax_neko_get(L, "__timer_update");
    lua_pushnumber(L, get_timing_instance().delta);
    luax_pcall(L, 1, 0);

    if (!gBase.error_mode.load()) {

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko %d", neko_buildnum());

            // if (g_app->debug_on) {
            // sgimgui_draw_menu(&sgimgui, "gfx");
            // }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 275 - ImGui::GetScrollX());
            ImGui::Text("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(L, LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024), get_timing_instance().true_dt * 1000.f,
                        1.f / get_timing_instance().true_dt);

            ImGui::EndMainMenuBar();
        }

        perf();

        // 底层图片
        char background_text[64] = "Project: unknown";

        f32 td = gApp->default_font->width(22.f, background_text);
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
            batch_draw_all(gApp->batch);
            edit_draw_all();
            physics_draw_all();

            posteffect_enable = !edit_get_enabled();

            // 现在绑定回默认帧缓冲区并使用附加的帧缓冲区颜色纹理绘制一个四边形平面
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);  // 禁用深度测试，以便屏幕空间四边形不会因深度测试而被丢弃。
            // 清除所有相关缓冲区
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // 将透明颜色设置为白色
            glClear(GL_COLOR_BUFFER_BIT);

            GLuint sid = posteffect_shader.shader.id;
            glUseProgram(sid);

            glUniform1f(glGetUniformLocation(sid, "intensity"), posteffect_intensity);
            glUniform1i(glGetUniformLocation(sid, "enable"), posteffect_enable);

            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, fbo_tex);  // 使用颜色附件纹理作为四边形平面的纹理
            glDrawArrays(GL_TRIANGLES, 0, 6);
            f32 fy = draw_font(gApp->default_font, false, 16.f, 0.f, 20.f, "Hello World 测试中文，你好世界", NEKO_COLOR_WHITE);
            fy += draw_font(gApp->default_font, false, 16.f, 0.f, fy, "我是第二行", NEKO_COLOR_WHITE);
            fy += draw_font(gApp->default_font, true, 16.f, 0.f, 20.f, "这一行字 draw_in_world", NEKO_COLOR_WHITE);
        }

        auto ui = gApp->ui;

        // ImGui::ShowDemoWindow();

        ImGui::SetNextWindowViewport(gApp->devui_vp);
        if (ImGui::Begin("Hello")) {

            ImGui::InputFloat("posteffect_intensity", &posteffect_intensity);

#if 0
            if (ImGui::Button("fgd")) {
                Fgd fgd("neko_base.fgd");
                bool ok = fgd.parse();
                LOG_INFO("{}", ok);

                FgdClass *game_lua = nullptr;

                for (auto &[n, c] : fgd.classMap) {
                    LOG_INFO("{} {}", n.c_str(), c);

                    if (n == "game_lua") game_lua = c;
                }

                neko_assert(game_lua);

                LOG_INFO("game_lua: {} {}", game_lua->classType, game_lua->name.c_str());

                for (auto &keydef : game_lua->keyvalues) {
                    LOG_INFO("{}: {}", keydef.name.c_str(), keydef.description.c_str());
                }
            }
#endif

            ImGui::End();
        }

        // gameconsole_draw();

        neko_update_ui(ui);

        DeferLoop(ui_begin(ui), ui_end(ui)) {
            script_draw_ui();
            draw_gui();
        }

        neko_render_ui(ui, gApp->cfg.width, gApp->cfg.height);

        Neko::LuaInspector::luainspector_draw(ENGINE_LUA());

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

        if (LockGuard<Mutex> lock{gBase.error_mtx}) {
            y = draw_font(font, false, font_size, x, y, "-- ! Neko Error ! --", NEKO_COLOR_WHITE);
            y += font_size;

            y = draw_font_wrapped(font, false, font_size, x, y, gBase.fatal_error, NEKO_COLOR_WHITE, gApp->cfg.width - x);
            y += font_size;

            if (gBase.traceback.data) {
                y = draw_font(font, false, font_size, x, y, gBase.traceback, NEKO_COLOR_WHITE);
                y += (font_size * 2);

                draw_font(font, false, font_size, x, y, "按下 Ctrl+C 复制以上堆栈信息", NEKO_COLOR_WHITE);

                if (input_key_down(KC_LEFT_CONTROL) && input_key_down(KC_C)) {
                    window_setclipboard(gBase.traceback.cstr());
                }
            }
        }
    }

    TexturedQuad quad = {
            .texture = NULL,
            .position = {0, 0},
            .dimentions = {40, 40},
            .rect = {0},
            .color = make_color(0x6cafb5, 100),
    };

    renderer_push(renderer, &quad);

    NEKO_INVOKE_ONCE(renderer_push_light(renderer, light{.position = {100, 100}, .range = 1000.0f, .intensity = 20.0f}););

    renderer_flush(renderer);

    imgui_draw_post();

    neko_check_gl_error();

    game.WindowSwapBuffer();

    return 0;
}

void Game::SplashScreen() {

    if (1) {

        // String contents = {};
        // bool ok = vfs_read_entire_file(&contents, "assets/splash.png");
        // if (!ok) {
        //     return;
        // }
        // neko_defer(mem_free(contents.data));

        AssetTexture splash_texture = neko_aseprite_simple("assets/cat.ase");

        Asset splash_shader = {};

        bool ok = asset_load_kind(AssetKind_Shader, "shader/splash.glsl", &splash_shader);
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

        mat4 projection = mat4_ortho(-((float)gApp->cfg.width / 24.0f), (float)gApp->cfg.width / 24.0f, (float)gApp->cfg.height / 24.0f, -((float)gApp->cfg.height / 24.0f), -1.0f, 1.0f);
        mat4 model = mat4_scalev(vec3{splash_texture.width / 2.0f, splash_texture.height / 2.0f, 0.0f});

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        neko_bind_shader(sid);
        neko_bind_texture(&splash_texture, 0);
        neko_shader_set_int(sid, "image", 0);
        neko_shader_set_m4f(sid, "transform", model);
        neko_shader_set_m4f(sid, "projection", projection);

        neko_bind_vertex_buffer_for_draw(quad);
        neko_draw_vertex_buffer(quad);

        WindowSwapBuffer();

        neko_free_vertex_buffer(quad);

        // neko_free_texture(&splash_texture);

        neko_check_gl_error();
    }
}

static void _game_fini() {
    PROFILE_FUNC();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fbo_tex);
    glDeleteRenderbuffers(1, &rbo);

    free_renderer(renderer);

    bool dump_allocs_detailed = gApp->cfg.dump_allocs_detailed;

    {  // just for test

        mem_free(gApp->ui);
        neko_deinit_ui_renderer();
    }

    // fini systems
    system_fini();

    // fini glfw
    glfwDestroyWindow(gApp->game_window);
    glfwTerminate();

#ifdef USE_PROFILER
    profile_shutdown();
#endif

    mem_free(gBase.fatal_error.data);
    mem_free(gBase.traceback.data);

    auto &eh = Neko::the<EventHandler>();
    eh.fini();

    Neko::modules::shutdown<EventHandler>();

    gApp->~App();
    mem_free(gApp);
}

bool CL_IsHostClient(void) { return true; }

void CL_Disconnect(void) {}

// -------------------------------------------------------------------------

bool CL_Init() {
    Neko::modules::initialize<Game>();
    auto &game = Neko::the<Game>();
    game.init();

    return true;
}

bool CL_Think() {
    App *app = gApp;
    auto &eh = Neko::the<EventHandler>();
    // if (glfwWindowShouldClose(gApp->game_window)) {
    //     game.quit();
    // }
    eh.event_pump();

    eh.event_dispatch(event_t{.type = on_preupdate});
    if (!gBase.error_mode.load()) {
        eh.event_dispatch(event_t{.type = on_update});
    }
    eh.event_dispatch(event_t{.type = on_postupdate});

    eh.event_dispatch(event_t{.type = on_draw});

    return true;
}

bool CL_Shutdown() {
    _game_fini();
    return true;
}

void Game::game_set_bg_color(Color c) { glClearColor(c.r, c.g, c.b, 1.0); }

void Game::set_window_size(vec2 s) { glfwSetWindowSize(gApp->game_window, s.x, s.y); }

vec2 Game::get_window_size() {
    int w, h;
    glfwGetWindowSize(gApp->game_window, &w, &h);
    return luavec2(w, h);
}
vec2 Game::unit_to_pixels(vec2 p) {
    vec2 hw = vec2_float_mul(get_window_size(), 0.5f);
    p = vec2_mul(p, hw);
    p = luavec2(p.x + hw.x, p.y - hw.y);
    return p;
}
vec2 Game::pixels_to_unit(vec2 p) {
    vec2 hw = vec2_float_mul(get_window_size(), 0.5f);
    p = luavec2(p.x - hw.x, p.y + hw.y);
    p = vec2_div(p, hw);
    return p;
}

void Game::quit() { gApp->g_quit = true; }

void Game::WindowSwapBuffer() { glfwSwapBuffers(gApp->game_window); }

Game::Game() {}

void Game::init() {

    Neko::modules::initialize<EventHandler>();
    Neko::modules::initialize<Input>();

    gApp = new (mem_alloc(sizeof(App))) App();

    LockGuard<Mutex> lock(gApp->g_init_mtx);

#if defined(NDEBUG)
    LOG_INFO("neko {}", neko_buildnum());
#else
    LOG_INFO("neko {} (debug build) (Lua {}.{}.{}, {})", neko_buildnum(), LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE, LUAJIT_VERSION);
#endif

    // create glfw window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    gApp->game_window = glfwCreateWindow(800, 600, "neko_game", NULL, NULL);

    // activate OpenGL context
    glfwMakeContextCurrent(gApp->game_window);

    // 注册窗口大小改变的回调函数
    glfwSetFramebufferSizeCallback(gApp->game_window, framebuffer_size_callback);

    // initialize GLEW
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_INFO("Failed to initialize GLAD");
    }

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
    gApp->timing_instance.startup = stm_now();
    gApp->timing_instance.last = stm_now();

    // init systems
    console_puts("welcome to neko!");

    system_init();

    assets_start_hot_reload();

    {  // just for test

        gApp->ui = (ui_context_t *)mem_alloc(sizeof(ui_context_t));
        ui_init(gApp->ui);

        gApp->ui->style->colors[UI_COLOR_WINDOWBG] = color256(50, 50, 50, 200);

        gApp->ui->text_width = neko_ui_text_width;
        gApp->ui->text_height = neko_ui_text_height;
        neko_init_ui_renderer();
    }

    auto &eh = Neko::the<EventHandler>();

    struct {
        event_mask evt;
        EventCallback cb;
    } evt_list[] = {
            {event_mask::preupdate, (EventCallback)assets_perform_hot_reload_changes},  //
            {event_mask::preupdate, (EventCallback)edit_clear},                         //
            {event_mask::preupdate, (EventCallback)timing_update},                      //
            {event_mask::preupdate, (EventCallback)script_pre_update_all},              //
            {event_mask::preupdate, (EventCallback)gui_pre_update_all},                 //

            {event_mask::update, (EventCallback)script_update_all},
            {event_mask::update, (EventCallback)physics_update_all},
            {event_mask::update, (EventCallback)transform_update_all},

            {event_mask::update, (EventCallback)camera_update_all},
            {event_mask::update, (EventCallback)sprite_update_all},
            {event_mask::update, (EventCallback)batch_update_all},
            {event_mask::update, (EventCallback)sound_update_all},
            {event_mask::update, (EventCallback)tiled_update_all},
            {event_mask::update, (EventCallback)edit_update_all},

            {event_mask::postupdate, (EventCallback)script_post_update_all},
            {event_mask::postupdate, (EventCallback)physics_post_update_all},
            {event_mask::postupdate, (EventCallback)sound_postupdate},
            {event_mask::postupdate, (EventCallback)entity_update_all},

            {event_mask::draw, (EventCallback)_game_draw},
    };

    for (auto evt : evt_list) {
        eh.event_register(gApp, evt.evt, evt.cb, NULL);
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        LOG_INFO("framebuffer good");
    }

    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gApp->cfg.width, gApp->cfg.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gApp->cfg.width, gApp->cfg.height);
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

    ok = asset_load_kind(AssetKind_Shader, "shader/sprite2.glsl", &sprite_shader);
    error_assert(ok);

    renderer = new_renderer(sprite_shader.shader, neko_v2(gApp->cfg.width, gApp->cfg.height));

    neko_check_gl_error();
}

void Game::fini() {}
void Game::update() {}

int game_set_window_minsize(int width, int height) {
    glfwSetWindowSizeLimits(gApp->game_window, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    return 0;
}

int game_get_window_width(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (gApp->game_window) {
        glfwGetWindowSize(gApp->game_window, &w, &h);
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
    if (gApp->game_window) {
        glfwGetWindowSize(gApp->game_window, &w, &h);
    }
#endif
    *val = h;
    return 0;
}

int game_set_window_position(int x, int y) {
#if !defined(__EMSCRIPTEN__)
    if (gApp->game_window) {
        glfwSetWindowPos(gApp->game_window, x, y);
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

int Game::set_window_title(const char *title) {
#if defined(__EMSCRIPTEN__)
    emscripten_set_window_title(title);
#else
    glfwSetWindowTitle(gApp->game_window, title);
#endif
    return 0;
}

void test_native_script() {
    CEntity camera, block, player;
    unsigned int i, n_blocks;

    // add camera

    camera = entity_create("camera_test");

    transform_add(camera);

    camera_add(camera);

    // add some blocks

    n_blocks = rand() % 50;
    for (i = 0; i < n_blocks; ++i) {
        block = entity_create("block_test");

        transform_add(block);
        transform_set_position(block, luavec2((rand() % 25) - 12, (rand() % 9) - 4));

        sprite_add(block);
        sprite_set_texcell(block, luavec2(32.0f, 32.0f));
        sprite_set_texsize(block, luavec2(32.0f, 32.0f));
    }

    // add player

    player = entity_create("player_test");

    transform_add(player);
    transform_set_position(player, luavec2(0.0f, 0.0f));

    sprite_add(player);
    sprite_set_texcell(player, luavec2(0.0f, 32.0f));
    sprite_set_texsize(player, luavec2(32.0f, 32.0f));
}

static void load_all_lua_scripts(lua_State *L) {
    PROFILE_FUNC();

    Array<String> files = {};
    neko_defer({
        for (String str : files) {
            mem_free(str.data);
        }
        files.trash();
    });

    bool ok = false;

#if defined(_DEBUG) || 1
    ok = vfs_list_all_files(NEKO_PACKS::LUACODE, &files);
#else
    ok = vfs_list_all_files(NEKO_PACKS::GAMEDATA, &files);
#endif

    if (!ok) {
        neko_panic("failed to list all files");
    }
    std::qsort(files.data, files.len, sizeof(String), [](const void *a, const void *b) -> int {
        String *lhs = (String *)a;
        String *rhs = (String *)b;
        return std::strcmp(lhs->data, rhs->data);
    });

    for (String file : files) {
        if (file.starts_with("script/") && !file.ends_with("nekomain.lua") && file.ends_with(".lua")) {
            asset_load_kind(AssetKind_LuaRef, file, nullptr);
        }
    }
}

static void _key_down(KeyCode key, int scancode, int mode) {
    // gui_key_down(key);
    script_key_down(key);
}
static void _key_up(KeyCode key, int scancode, int mode) {
    // gui_key_up(key);
    script_key_up(key);
}
static void _char_down(unsigned int c) { /*gui_char_down(c);*/ }
static void _mouse_down(MouseCode mouse) {
    // gui_mouse_down(mouse);
    script_mouse_down(mouse);
}
static void _mouse_up(MouseCode mouse) {
    // gui_mouse_up(mouse);
    script_mouse_up(mouse);
}
static void _mouse_move(vec2 pos) { script_mouse_move(pos); }
static void _scroll(vec2 scroll) { script_scroll(scroll); }

void system_init() {
    PROFILE_FUNC();

    MountResult mount = {true, true, false};

    script_init();

    lua_State *L = ENGINE_LUA();

    gBase.is_fused.store(mount.is_fused);

    if (!gBase.error_mode.load() && mount.ok) {
        bool ok = asset_load_kind(AssetKind_LuaRef, "conf.lua", nullptr);
        if (!ok) {
            String conf = R"(
function neko.conf(t)

    t.app = {
        title = "ahaha",
        width = 1280.0,
        height = 720.0,
        game_proxy = "default",
        default_font = "assets/fonts/VonwaonBitmap-16px.ttf",
        -- lite_init_path = "D:/Projects/Neko/DevNew/gamedir/lite",
        dump_allocs_detailed = true,
        swap_interval = 1,
        target_fps = 120,
        reload_interval = 1,
        debug_on = false,
        batch_vertex_capacity = 2048
    }
end
)";
            luax_require_script_buffer(L, conf, "<builtin_conf>");
        }
    }

    lua_newtable(L);
    i32 conf_table = lua_gettop(L);

    if (!gBase.error_mode.load()) {
        luax_neko_get(L, "conf");
        lua_pushvalue(L, conf_table);
        luax_pcall(L, 1, 0);
    }

    gApp->win_console = gApp->win_console || luax_boolean_field(L, -1, "win_console", true);

    Neko::reflection::Any v = engine_cfg_t{.title = "NekoEngine", .hot_reload = true, .startup_load_scripts = true, .fullscreen = false};
    checktable_refl(ENGINE_LUA(), "app", v);
    gApp->cfg = v.cast<engine_cfg_t>();

    LOG_INFO("load game: {} {} {}", gApp->cfg.title.cstr(), gApp->cfg.width, gApp->cfg.height);

    lua_pop(L, 1);  // conf table

    auto &game = Neko::the<Game>();

    // 刷新状态
    game.set_window_size(luavec2(gApp->cfg.width, gApp->cfg.height));
    game.set_window_title(gApp->cfg.title.cstr());

    if (fnv1a(gApp->cfg.game_proxy) == "default"_hash) {
        // Neko::neko_lua_run_string(L, R"lua(
        // )lua");
        LOG_INFO("using default game proxy");
    }

    g_render = gfx_create();
    gfx_init(g_render);

    game.SplashScreen();

    neko_default_font();

    auto &input = Neko::the<Input>();
    input.init();

    entity_init();
    transform_init();
    camera_init();
    gApp->batch = batch_init(gApp->cfg.batch_vertex_capacity);
    sprite_init();
    tiled_init();
    font_init();
    imgui_init(gApp->game_window);
    console_init();
    sound_init();
    physics_init();
    edit_init();

    gBase.hot_reload_enabled.store(mount.can_hot_reload && gApp->cfg.hot_reload);
    gBase.reload_interval.store(gApp->cfg.reload_interval);

    luax_run_nekogame(L);

    Neko::LuaInspector::luainspector_init(ENGINE_LUA());
    lua_setglobal(L, "__neko_inspector");

    if (!gBase.error_mode.load() && gApp->cfg.startup_load_scripts && mount.ok) {
        load_all_lua_scripts(L);
    }

    // run main.lua
    asset_load_kind(AssetKind_LuaRef, "script/nekomain.lua", nullptr);

    luax_get(ENGINE_LUA(), "neko", "__define_default_callbacks");
    luax_pcall(ENGINE_LUA(), 0, 0);

    {
        PROFILE_BLOCK("neko.args");

        lua_State *L = ENGINE_LUA();

        if (!gBase.error_mode.load()) {
            luax_neko_get(L, "args");

            Slice<String> args = gBase.GetArgs();
            lua_createtable(L, args.len - 1, 0);
            for (u64 i = 1; i < args.len; i++) {
                lua_pushlstring(L, args[i].data, args[i].len);
                lua_rawseti(L, -2, i);
            }

            luax_pcall(L, 1, 0);
        }
    }

    // fire init event
    script_push_event("init");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    input_add_key_down_callback(_key_down);
    input_add_key_up_callback(_key_up);
    input_add_char_down_callback(_char_down);
    input_add_mouse_down_callback(_mouse_down);
    input_add_mouse_up_callback(_mouse_up);
    input_add_mouse_move_callback(_mouse_move);
    input_add_scroll_callback(_scroll);

    if (gApp->cfg.target_fps != 0) {
        gApp->timing_instance.target_ticks = 1000000000 / gApp->cfg.target_fps;
    }

#ifdef NEKO_IS_WIN32
    if (!gApp->win_console) {
        FreeConsole();
    }
#endif
}

void system_fini() {
    PROFILE_FUNC();

    edit_fini();
    script_fini();
    physics_fini();
    sound_fini();
    console_fini();
    tiled_fini();
    sprite_fini();
    batch_fini(gApp->batch);
    camera_fini();
    transform_fini();
    entity_fini();
    imgui_fini();
    auto &input = Neko::the<Input>();
    input.fini();

    if (gApp->default_font != nullptr) {
        gApp->default_font->trash();
        mem_free(gApp->default_font);
    }

    gfx_fini(g_render);

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        // if (g_app->default_font != nullptr) {
        //     g_app->default_font->trash();
        //     mem_free(g_app->default_font);
        // }

        assets_shutdown();
    }
}

AppTime get_timing_instance() { return gApp->timing_instance; }

f32 timing_get_elapsed() { return glfwGetTime() * 1000.0f; }

void timing_set_scale(f32 s) { gApp->scale = s; }
f32 timing_get_scale() { return gApp->scale; }

void timing_set_paused(bool p) { gApp->paused = p; }  // 暂停将刻度设置为 0 并在恢复时恢复它
bool timing_get_paused() { return gApp->paused; }

int timing_update(App *app, event_t evt) {

    // update dt
    static double last_time = -1;
    double curr_time;

    // first update?
    if (last_time < 0) last_time = glfwGetTime();

    curr_time = glfwGetTime();
    gApp->timing_instance.true_dt = curr_time - last_time;
    gApp->timing_instance.dt = gApp->paused ? 0.0f : gApp->scale * get_timing_instance().true_dt;
    last_time = curr_time;

    {
        AppTime *time = &gApp->timing_instance;

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

#endif

// 处理引擎状态变量和功能

namespace Neko {
Allocator *g_allocator = []() -> Allocator * {
#ifndef NDEBUG
    static DebugAllocator alloc;
#else
    static HeapAllocator alloc;
#endif
    return &alloc;
}();
}  // namespace Neko

extern Neko::CBase gBase;

static void _glfw_error_callback(int error, const char *desc) { fprintf(stderr, "glfw: %s\n", desc); }

bool EngineInit(int argc, const char *argv[]) {

    gBase.Init();

    gBase.SetArgs(argc, argv);

    gBase.LoadVFS("../gamedir");

    auto argsArray = BaseGetCommandLine(argc);

    glfwSetErrorCallback(_glfw_error_callback);

    if (!glfwInit()) {
        // Sys_ErrorPopup("SDL_Init returned an error");
        return false;
    }

    for (auto &s : argsArray) mem_free(s.data);
    argsArray.trash();

    // if (!SV_Init()) return false;
    if (!CL_Init()) return false;

    return true;
}

void Sys_Shutdown(void) {
    CL_Shutdown();
    gBase.UnLoadVFS();
    gBase.Fini();
    glfwTerminate();
}

void Sys_Frame() { CL_Think(); }

Int32 Main(int argc, const char *argv[]) {

    if (!EngineInit(argc, argv)) {
        LOG_INFO("Error during system initialization.\n");
        return -1;
    }

    GLFWwindow *window = gApp->game_window;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Sys_Frame();
    }

    Sys_Shutdown();

    return 0;
}

int main(int argc, const char *argv[]) {
    Main(argc, argv);
    return 0;
}