
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
#include "base/common/reflection.hpp"
#include "base/scripting/scripting.h"
#include "engine/ui.h"
#include "engine/renderer/renderer.h"
#include "editor/editor.hpp"
#include "base/common/math.hpp"
#include "engine/input.h"
#include "engine/window.h"
#include "engine/renderer/shader.h"

// clang-format off

NEKO_STRUCT(CL::State, 
_Fs(show_editor,""),
_Fs(show_demo_window,""),
_Fs(show_gui,""),
_Fs(shader_inspect,""),
_Fs(hello_ai_shit,""),
_Fs(vsync,""),
_Fs(is_hotfix,""),
_Fs(game_proxy,""),
_Fs(default_font,""),
_Fs(title,""),
_Fs(height,""),
_Fs(width,""),
_Fs(dump_allocs_detailed,""),
_Fs(hot_reload,""),
_Fs(startup_load_scripts,""),
_Fs(fullscreen,""),
_Fs(debug_on,""),
_Fs(reload_interval,""),
_Fs(swap_interval,""),
_Fs(target_fps,""),
_Fs(batch_vertex_capacity,""),
_Fs(posteffect_intensity,"")
);

// clang-format on

#if 1

CBase gBase;

#endif

#if 1

unsigned int fbo;
unsigned int rbo;
unsigned int fbo_tex;
unsigned int quadVAO, quadVBO;
Asset posteffect_shader = {};
Asset sprite_shader = {};

QuadRenderer quadrenderer;

extern void draw_gui();

// -------------------------------------------------------------------------

void rescale_framebuffer(float width, float height) {
    if (fbo_tex == 0 || rbo == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    auto &CLGame = the<CL>();
    CLGame.state.width = width;
    CLGame.state.height = height;

    rescale_framebuffer(width, height);

    // 更新视口
    glViewport(0, 0, width, height);

    // 计算 ImGui 的缩放比例
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGuiIO &io = ImGui::GetIO();
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        float scale_x = fb_width / (float)width;
        float scale_y = fb_height / (float)height;

        io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
    }
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

DEFINE_IMGUI_BEGIN(template <>, String) { ImGui::Text("%s", var.cstr()); }
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, mat3) { ImGui::Text("%f %f %f\n%f %f %f\n%f %f %f\n", var.v[0], var.v[1], var.v[2], var.v[3], var.v[4], var.v[5], var.v[6], var.v[7], var.v[8]); }
DEFINE_IMGUI_END()

void state_inspector(CL::State &cvar) {
    auto func = []<typename S, typename Fields>(const char *name, auto &var, S &t, Fields &&fields) {
        Neko::ImGuiWrap::Auto(var, name);
        ImGui::Text("    [%s]", std::get<0>(fields));
    };
    reflection::struct_foreach_rec(func, cvar);
}

void CL::game_draw() {

    lua_State *L = ENGINE_LUA();

    luax_neko_get(L, "__timer_update");
    lua_pushnumber(L, GetTimeInfo().delta);
    luax_pcall(L, 1, 0);

    if (!gBase.error_mode.load()) {

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (edit_get_enabled() && ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko %d", neko_buildnum());

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 275 - ImGui::GetScrollX());
            ImGui::Text("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(L, LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024), GetTimeInfo().true_dt * 1000.f,
                        1.f / GetTimeInfo().true_dt);

            ImGui::EndMainMenuBar();
        }

        // script_draw_all();
        the<Tiled>().tiled_draw_all();

        the<EventHandler>().EventPushLuaType(OnDraw);

        the<Sprite>().sprite_draw_all();
        the<Batch>().batch_draw_all();
        the<Edit>().edit_draw_all();
        physics_draw_all();
        the<DebugDraw>().debug_draw_all();

        // 现在绑定回默认帧缓冲区并使用附加的帧缓冲区颜色纹理绘制一个四边形平面
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);  // 禁用深度测试，以便屏幕空间四边形不会因深度测试而被丢弃。
        // 清除所有相关缓冲区
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // 将透明颜色设置为白色
        glClear(GL_COLOR_BUFFER_BIT);

        GLuint sid = posteffect_shader.shader.id;
        glUseProgram(sid);

        int posteffect_enable = !edit_get_enabled();

        glUniform1f(glGetUniformLocation(sid, "intensity"), state.posteffect_intensity);
        glUniform1i(glGetUniformLocation(sid, "enable"), 0);
        glUniform1f(glGetUniformLocation(sid, "rt_w"), state.width);
        glUniform1f(glGetUniformLocation(sid, "rt_h"), state.height);

        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, fbo_tex);  // 使用颜色附件纹理作为四边形平面的纹理
        glDrawArrays(GL_TRIANGLES, 0, 6);

        {
            ImGuiIO &io = ImGui::GetIO();
            const ImVec2 screenSize = io.DisplaySize;
            ImDrawList *draw_list = ImGui::GetForegroundDrawList();
            const ImVec2 footer_size(60, 20);
            const ImVec2 footer_pos(screenSize.x - footer_size.x - 20, 20);
            const ImU32 background_color = IM_COL32(0, 0, 0, 102);

            // f32 fy = draw_font(default_font, false, 16.f, 0.f, 20.f, "Neko build " __DATE__, NEKO_COLOR_WHITE);

            // TexturedQuad quad = {
            //         .texture = NULL,
            //         .position = {0, 20},
            //         .dimentions = {180, 20},
            //         .rect = {0},
            //         .color = make_color(0x0000000, 100),
            // };

            // quadrenderer.renderer_push(&quad);

            // NEKO_INVOKE_ONCE(quadrenderer.renderer_push_light(light{.position = {100, 100}, .range = 1000.0f, .intensity = 20.0f}););

            // quadrenderer.renderer_flush();

            draw_list->AddRectFilled(footer_pos, footer_pos + footer_size, background_color);

            Neko::ImGuiWrap::OutlineText(draw_list, ImVec2(footer_pos.x + 3, footer_pos.y + 3), "%.1f FPS", io.Framerate);
        }

        auto ui = this->ui;
        neko_update_ui(ui);
        DeferLoop(ui_begin(ui), ui_end(ui)) {
            the<EventHandler>().EventPushLuaType(OnDrawUI);

            if (edit_get_enabled()) draw_gui();
        }
        neko_render_ui(ui, state.width, state.height);

        if (state.show_demo_window) ImGui::ShowDemoWindow();

        if (edit_get_enabled()) {
            ImGui::SetNextWindowViewport(devui_vp);
            if (ImGui::Begin("Hello")) {
                state_inspector(state);

                mat3 view = camera_get_inverse_view_matrix();
                ImGuiWrap::Auto(view);
            }
            ImGui::End();
        }

        inspector->luainspector_draw(ENGINE_LUA());

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

            y = draw_font_wrapped(font, false, font_size, x, y, gBase.fatal_error_string, NEKO_COLOR_WHITE, state.width - x);
            y += font_size;

            if (gBase.traceback.data) {
                y = draw_font(font, false, font_size, x, y, gBase.traceback, NEKO_COLOR_WHITE);
                y += (font_size * 2);

                y += draw_font(font, false, font_size, x, y, "按下 Ctrl+C 复制以上堆栈信息", NEKO_COLOR_WHITE);

                if (input_key_down(KC_LEFT_CONTROL) && input_key_down(KC_C)) {
                    window->SetClipboard(gBase.traceback.cstr());
                }
            }

            y += draw_font(font, false, font_size, x, y, "按下 Ctrl+R 忽视本次问题", NEKO_COLOR_WHITE);

            if (input_key_down(KC_LEFT_CONTROL) && input_key_down(KC_R)) {
                gBase.error_mode.store(false);
            }
        }
    }

    the<ImGuiRender>().imgui_draw_post();

    window->SwapBuffer();
}

void CL::SplashScreen() {
    AssetTexture splash_texture = neko_aseprite_simple("assets/cat.ase");
    Asset splash_shader = {};
    bool ok = asset_load_kind(AssetKind_Shader, "shader/splash.glsl", &splash_shader);
    error_assert(ok);
    GLuint sid = splash_shader.shader.id;

    // position uv
    // clang-format off
        float verts[] = {
            0.5f,  0.5f,  1.0f,  0.0f, 
            0.5f, -0.5f,  1.0f,  1.0f, 
           -0.5f, -0.5f,  0.0f,  1.0f, 
           -0.5f,  0.5f,  0.0f,  0.0f
        };
    // clang-format on

    u32 indices[] = {0, 1, 3, 1, 2, 3};

    VertexBuffer quad;
    quad.init_vb(vb_static | vb_tris);

    quad.bind_vb_for_edit(true);
    quad.push_vertices(verts, sizeof(verts) / sizeof(float));
    quad.push_indices(indices, sizeof(indices) / sizeof(u32));
    quad.configure_vb(0, 2, 4, 0);
    quad.configure_vb(1, 2, 4, 2);

    mat4 projection = mat4_ortho(-((float)state.width / 24.0f), (float)state.width / 24.0f, (float)state.height / 24.0f, -((float)state.height / 24.0f), -1.0f, 1.0f);
    mat4 model = mat4_scalev(vec3{splash_texture.width / 2.0f, splash_texture.height / 2.0f, 0.0f});

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    neko_bind_shader(sid);
    neko_bind_texture(&splash_texture, 0);
    neko_shader_set_int(sid, "image", 0);
    neko_shader_set_m4f(sid, "transform", model);
    neko_shader_set_m4f(sid, "projection", projection);

    quad.bind_vb_for_draw(true);
    quad.draw_vb();

    // auto font = neko_default_font();
    // char background_text[64] = "Project: unknown";
    // f32 td = font->width(22.f, background_text);
    // draw_font(font, false, 16.f, (state.width - td) * 0.5f, (state.height) * 0.5f + splash_texture.height / 2.f - 100.f, background_text, NEKO_COLOR_WHITE);

    window->SwapBuffer();

    quad.fini_vb();
    neko_release_texture(&splash_texture);
}

void CL::game_set_bg_color(Color c) { glClearColor(c.r, c.g, c.b, 1.0); }

void CL::set_window_size(vec2 s) { glfwSetWindowSize(window->glfwWindow(), s.x, s.y); }

vec2 CL::get_window_size() {
    int w, h;
    glfwGetWindowSize(window->glfwWindow(), &w, &h);
    return luavec2(w, h);
}
vec2 CL::unit_to_pixels(vec2 p) {
    vec2 hw = vec2_float_mul(get_window_size(), 0.5f);
    p = vec2_mul(p, hw);
    p = luavec2(p.x + hw.x, p.y - hw.y);
    return p;
}
vec2 CL::pixels_to_unit(vec2 p) {
    vec2 hw = vec2_float_mul(get_window_size(), 0.5f);
    p = luavec2(p.x - hw.x, p.y + hw.y);
    p = vec2_div(p, hw);
    return p;
}

void CL::quit() { g_quit = true; }

CL::CL() {}

void CL::init() {

    LockGuard<Mutex> lock(g_init_mtx);

    window = &Neko::the<Window>();

#if defined(NDEBUG)
    LOG_INFO("neko {}", neko_buildnum());
#else
    LOG_INFO("neko {} (debug build) (Lua {}.{}.{}, {})", neko_buildnum(), LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE, LUAJIT_VERSION);
#endif

    window->create();
    window->SetFramebufferSizeCallback(framebuffer_size_callback);

    the<Renderer>().InitOpenGL();

    the<EventHandler>().init();

    // random seed
    srand(::time(NULL));

    // time set
    time.startup = TimeUtil::now();
    time.last = TimeUtil::now();

    PROFILE_FUNC();

    script_init();

    lua_State *L = ENGINE_LUA();

    if (!gBase.error_mode.load() /*&& mount.ok*/) {
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
        dump_allocs_detailed = true,
        swap_interval = 1,
        target_fps = 999,
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

    win_console = win_console || luax_boolean_field(L, -1, "win_console", true);

    CL::State v{.title = "NekoEngine", .hot_reload = true, .startup_load_scripts = true, .fullscreen = false};
    struct_foreach_luatable(ENGINE_LUA(), "app", v);
    state = v;

    LOG_INFO("load game: {} {} {}", state.title.cstr(), state.width, state.height);

    lua_pop(L, 1);  // conf table

    auto &game = Neko::the<CL>();

    // 刷新状态
    game.set_window_size(luavec2(state.width, state.height));
    game.set_window_title(state.title.cstr());

    if (fnv1a(state.game_proxy) == "default"_hash) {
        // Neko::neko_lua_run_string(L, R"lua(
        // )lua");
        LOG_INFO("using default game proxy");
    }

    game.SplashScreen();

    neko_default_font();

    auto &input = Neko::the<Input>();
    input.init();

    Neko::modules::initialize<Entity>();
    Neko::modules::initialize<Transform>();
    Neko::modules::initialize<Camera>();
    Neko::modules::initialize<Batch>();
    Neko::modules::initialize<Sprite>();
    Neko::modules::initialize<Tiled>();
    Neko::modules::initialize<ImGuiRender>();
    Neko::modules::initialize<Edit>();
    Neko::modules::initialize<DebugDraw>();

    the<Entity>().entity_init();
    the<Transform>().transform_init();
    the<Camera>().camera_init();
    the<Batch>().batch_init(state.batch_vertex_capacity);
    the<Sprite>().sprite_init();
    the<Tiled>().tiled_init();
    font_init();
    the<ImGuiRender>().imgui_init(window->glfwWindow());
    sound_init();
    physics_init();
    the<Edit>().edit_init();
    the<DebugDraw>().debug_draw_init();

    gBase.reload_interval.store(state.reload_interval);

    luax_run_nekogame(L);

    inspector = new Neko::LuaInspector();
    inspector->luainspector_init(L);

    if (!gBase.error_mode.load() && state.startup_load_scripts) {
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
    the<EventHandler>().EventPushLua("init");
    errcheck(L, luax_pcall_nothrow(L, 1, 0));

    auto _key_down = [](KeyCode key, int scancode, int mode) { the<EventHandler>().EventPushLuaType(OnKeyDown, key); };
    auto _key_up = [](KeyCode key, int scancode, int mode) { the<EventHandler>().EventPushLuaType(OnKeyUp, key); };
    auto _char_down = [](unsigned int c) { /*gui_char_down(c);*/ };
    auto _mouse_down = [](MouseCode mouse) { the<EventHandler>().EventPushLuaType(OnMouseDown, mouse); };
    auto _mouse_up = [](MouseCode mouse) { the<EventHandler>().EventPushLuaType(OnMouseUp, mouse); };
    auto _mouse_move = [](vec2 pos) { the<EventHandler>().EventPushLuaType(OnMouseMove, pos); };
    auto _scroll = [](vec2 scroll) { the<EventHandler>().EventPushLuaType(OnMouseScroll, scroll); };

    input_add_key_down_callback(_key_down);
    input_add_key_up_callback(_key_up);
    input_add_char_down_callback(_char_down);
    input_add_mouse_down_callback(_mouse_down);
    input_add_mouse_up_callback(_mouse_up);
    input_add_mouse_move_callback(_mouse_move);
    input_add_scroll_callback(_scroll);

    if (state.target_fps != 0) {
        time.target_ticks = 1000000000 / state.target_fps;
    }

#ifdef NEKO_IS_WIN32
    if (!win_console) {
        FreeConsole();
    }
#endif

    assets_start_hot_reload();

    {  // just for test

        this->ui = (ui_context_t *)mem_alloc(sizeof(ui_context_t));
        ui_init(this->ui);

        this->ui->style->colors[UI_COLOR_WINDOWBG] = color256(50, 50, 50, 200);

        this->ui->text_width = neko_ui_text_width;
        this->ui->text_height = neko_ui_text_height;
        neko_init_ui_renderer();
    }

    auto &eh = Neko::the<EventHandler>();

    struct {
        EventMask evt;
        EventCallback cb;
    } evt_list[] = {
            {EventMask::PreUpdate, assets_perform_hot_reload_changes},                            //
            {EventMask::PreUpdate, edit_clear},                                                   //
            {EventMask::PreUpdate, [](Event evt) -> int { return the<CL>().update_time(evt); }},  //
            {EventMask::PreUpdate,
             [](Event) -> int {
                 the<EventHandler>().EventPushLuaType(OnPreUpdate);
                 return 0;
             }},  //
            {EventMask::PreUpdate, [](Event evt) -> int { return the<ImGuiRender>().imgui_draw_pre(); }},

            {EventMask::Update,
             [](Event) -> int {
                 the<EventHandler>().EventPushLuaType(OnUpdate);
                 return 0;
             }},
            {EventMask::Update, physics_update_all},
            {EventMask::Update, [](Event evt) -> int { return the<Transform>().transform_update_all(evt); }},

            {EventMask::Update, [](Event evt) -> int { return the<Camera>().camera_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Sprite>().sprite_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Batch>().batch_update_all(evt); }},
            {EventMask::Update, sound_update_all},
            {EventMask::Update, [](Event evt) -> int { return the<Tiled>().tiled_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Edit>().edit_update_all(evt); }},

            {EventMask::PostUpdate,
             [](Event) -> int {
                 the<EventHandler>().EventPushLuaType(OnPostUpdate);
                 return 0;
             }},
            {EventMask::PostUpdate, physics_post_update_all},
            {EventMask::PostUpdate, sound_postupdate},
            {EventMask::PostUpdate, [](Event evt) -> int { return the<Entity>().entity_update_all(evt); }},

            {EventMask::Draw,
             [](Event) -> int {
                 the<CL>().game_draw();
                 return 0;
             }},
    };

    for (auto evt : evt_list) {
        eh.Register(evt.evt, evt.cb, NULL);
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        LOG_INFO("framebuffer good");
    }

    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.width, state.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.width, state.height);
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

    quadrenderer.new_renderer(sprite_shader.shader, neko_v2(state.width, state.height));
}

int CL::set_window_title(const char *title) {
#if defined(__EMSCRIPTEN__)
    emscripten_set_window_title(title);
#else
    glfwSetWindowTitle(window->glfwWindow(), title);
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

void CL::fini() {
    PROFILE_FUNC();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fbo_tex);
    glDeleteRenderbuffers(1, &rbo);

    quadrenderer.free_renderer();

    bool dump_allocs_detailed = state.dump_allocs_detailed;

    {  // just for test

        mem_free(this->ui);
        neko_deinit_ui_renderer();
    }

    delete inspector;
    inspector = nullptr;

    the<DebugDraw>().debug_draw_fini();
    the<Edit>().edit_fini();
    script_fini();
    physics_fini();
    sound_fini();
    the<Tiled>().tiled_fini();
    the<Sprite>().sprite_fini();
    the<Batch>().batch_fini();
    the<Camera>().camera_fini();
    the<Transform>().transform_fini();
    the<Entity>().entity_fini();
    the<ImGuiRender>().imgui_fini();
    auto &input = Neko::the<Input>();
    input.fini();

    if (default_font) {
        default_font->trash();
        mem_free(default_font);
    }

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        // if (g_app->default_font != nullptr) {
        //     g_app->default_font->trash();
        //     mem_free(g_app->default_font);
        // }

        assets_shutdown();
    }

    // fini glfw
    glfwDestroyWindow(window->glfwWindow());
    glfwTerminate();

#ifdef USE_PROFILER
    profile_shutdown();
#endif

    mem_free(gBase.fatal_error_string.data);
    mem_free(gBase.traceback.data);

    auto &eh = Neko::the<EventHandler>();
    eh.fini();

    Neko::modules::shutdown<EventHandler>();
}

int CL::update_time(Event evt) {

    // update dt
    static double last_time = -1;
    double curr_time;

    // first update?
    if (last_time < 0) last_time = glfwGetTime();

    curr_time = glfwGetTime();
    time.true_dt = curr_time - last_time;
    time.dt = paused ? 0.0f : scale * GetTimeInfo().true_dt;
    last_time = curr_time;

    {
        TimeInfo *time = &this->time;

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

                    u64 lap = TimeUtil::lap_time(&time->last);
                    time->delta += TimeUtil::to_seconds(lap);
                    time->accumulator += lap;

                    while (time->accumulator < target) {
                        os_yield();

                        u64 lap = TimeUtil::lap_time(&time->last);
                        time->delta += TimeUtil::to_seconds(lap);
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

Int32 Main(int argc, const char *argv[]) {
    bool ok = true;
    gBase.Init();
    gBase.SetArgs(argc, argv);
    gBase.LoadVFS("../gamedir");
    auto argsArray = BaseGetCommandLine(argc);
    glfwSetErrorCallback(_glfw_error_callback);
    if (!glfwInit()) {
        ok = false;
    }

    for (auto &s : argsArray) mem_free(s.data);
    argsArray.trash();

    // if (!SV_Init()) return false;
    Neko::modules::initialize<CL>();
    Neko::modules::initialize<Window>();
    Neko::modules::initialize<EventHandler>();
    Neko::modules::initialize<Input>();
    Neko::modules::initialize<Assets>();
    Neko::modules::initialize<Renderer>();

    auto &game = Neko::the<CL>();
    game.init();

    if (!ok) {
        printf("Error during system initialization.\n");
        return -1;
    }

    GLFWwindow *window = game.window->glfwWindow();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        auto &eh = Neko::the<EventHandler>();
        eh.Pump();
        eh.Dispatch(Event{.type = OnPreUpdate});
        if (!gBase.error_mode.load()) {
            eh.Dispatch(Event{.type = OnUpdate});
        }
        eh.Dispatch(Event{.type = OnPostUpdate});
        eh.Dispatch(Event{.type = OnDraw});
    }

    game.fini();

    gBase.UnLoadVFS();
    gBase.Fini();

    return 0;
}
