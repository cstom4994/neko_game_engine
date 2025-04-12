
#include "engine/bootstrap.h"

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
#include "engine/scripting/lua_wrapper.hpp"
#include "base/common/reflection.hpp"
#include "engine/scripting/scripting.h"
#include "engine/ui.h"
#include "engine/renderer/renderer.h"
#include "engine/editor.h"
#include "base/common/math.hpp"
#include "engine/input.h"
#include "engine/window.h"
#include "engine/renderer/shader.h"
#include "engine/components/camera.h"
#include "engine/components/edit.h"
#include "engine/components/sprite.h"
#include "engine/components/tiledmap.hpp"
#include "engine/components/transform.h"

#include <ranges>

#if 1

CBase gBase;

#endif

#if 1

RenderTarget renderview_source;
PostProcessor posteffect_dark;
PostProcessor posteffect_bright;
PostProcessor posteffect_blur_h;
PostProcessor posteffect_blur_v;
PostProcessor posteffect_composite;

Asset sprite_shader = {};

QuadRenderer quadrenderer;

extern void draw_gui();

// -------------------------------------------------------------------------

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    auto &CLGame = the<CL>();
    CLGame.state.width = width;
    CLGame.state.height = height;

    if (renderview_source.valid()) {
        renderview_source.resize(width, height);
        posteffect_dark.Resize(neko_v2(width, height));
        posteffect_bright.Resize(neko_v2(width, height));
        posteffect_blur_h.Resize(neko_v2(width, height));
        posteffect_blur_v.Resize(neko_v2(width, height));
        posteffect_composite.Resize(neko_v2(width, height));
        quadrenderer.renderer_resize(neko_v2(width, height));
    }

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

void CL::load_all_lua_scripts(lua_State *L) {
    PROFILE_FUNC();

    std::vector<std::string> files = {};

    bool ok = false;

#if defined(_DEBUG) || 1
    ok = the<VFS>().list_all_files("code", &files);
#else
    ok = the<VFS>().list_all_files("gamedata", &files);
#endif

    if (!ok) {
        neko_panic("failed to list all files");
    }

    auto is_internal_lua = [](const std::string &file) -> bool {
        static std::vector<std::string> internal_lua_list = {"nekomain.lua", "nekogame.lua", "nekoeditor.lua", "bootstrap.lua"};
        for (auto internal_lua : internal_lua_list)
            if (file.ends_with(internal_lua)) return true;
        return false;
    };

    auto filtered_files = files | std::views::filter([&](const std::string &file) { return file.ends_with(".lua") && !is_internal_lua(file); });

    for (const auto &file : filtered_files) {
        asset_load_kind(AssetKind_LuaRef, file, nullptr);
    }
}

void CL::game_draw() {

    lua_State *L = ENGINE_LUA();

    luax_neko_get(L, "__timer_update");
    lua_pushnumber(L, GetTimeInfo().delta);
    luax_pcall(L, 1, 0);

    if (!gBase.error_mode.load()) {

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderview_source.bind();
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
            glClear(GL_COLOR_BUFFER_BIT);

            the<Tiled>().tiled_draw_all();

            the<EventHandler>().EventPushLuaType(OnDraw);

            the<Sprite>().sprite_draw_all();
            the<Batch>().batch_draw_all();
            the<Editor>().edit_draw_all();
            the<DebugDraw>().debug_draw_all();
        }
        renderview_source.unbind();

        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!edit_get_enabled()) [[likely]] {

            posteffect_blur_h.GetRenderTarget().bind();
            posteffect_bright.FlushTarget(renderview_source, [](auto &shader) {});
            posteffect_blur_h.GetRenderTarget().unbind();

            posteffect_blur_v.GetRenderTarget().bind();
            posteffect_blur_h.Flush([](auto &shader) {});
            posteffect_blur_v.GetRenderTarget().unbind();

            posteffect_blur_v.Flush([](auto &shader) {});

            posteffect_composite.GetRenderTarget().bind();
            posteffect_dark.FlushTarget(renderview_source, [&](auto &shader) {
                neko_shader_set_float(shader.id, "intensity", state.posteffect_intensity);
                neko_shader_set_int(shader.id, "enable", 1);
            });
            posteffect_composite.GetRenderTarget().unbind();

            posteffect_composite.Flush([&](auto &shader) {
                neko_shader_set_float(shader.id, "u_exposure", state.posteffect_exposure);
                neko_shader_set_float(shader.id, "u_gamma", state.posteffect_gamma);
                neko_shader_set_float(shader.id, "u_bloom_scalar", state.posteffect_bloom_scalar);
                neko_shader_set_float(shader.id, "u_saturation", state.posteffect_saturation);
                neko_shader_set_int(shader.id, "u_blur_tex", 1);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, posteffect_blur_v.GetRenderTarget().output);
            });

            {
                ImGuiIO &io = ImGui::GetIO();
                const ImVec2 screenSize = io.DisplaySize;
                ImDrawList *draw_list = ImGui::GetForegroundDrawList();
                const ImVec2 footer_size(60, 20);
                const ImVec2 footer_pos(screenSize.x - footer_size.x - 20, 40);
                const ImU32 background_color = IM_COL32(0, 0, 0, 102);

                TexturedQuad quad = {
                        .texture = NULL,
                        .position = {screenSize.x - 180, 20},
                        .dimentions = {180, 20},
                        .rect = {0},
                        .color = make_color(0x0000000, 100),
                };

                quadrenderer.renderer_push(&quad);

                // NEKO_INVOKE_ONCE(quadrenderer.renderer_push_light(light{.position = {100, 100}, .range = 1000.0f, .intensity = 20.0f}););

                quadrenderer.renderer_flush();

                f32 fy = draw_font(default_font, false, 16.f, screenSize.x - 180.f, 20.f, "Neko build " __DATE__, NEKO_COLOR_WHITE);

                draw_list->AddRectFilled(footer_pos, footer_pos + footer_size, background_color);

                Neko::ImGuiWrap::OutlineText(draw_list, ImVec2(footer_pos.x + 3, footer_pos.y + 3), "%.1f FPS", io.Framerate);
            }
        } else {
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

            static bool opt_fullscreen = true;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen) {
                const ImGuiViewport *viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }

            ImGui::Begin("EditorDockSpace", nullptr, window_flags);

            dockspace_id = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Play", nullptr)) {

                if (ImGui::BeginCombo("Select Render View", renderview_sources[currentRenderViewIndex].name.cstr())) {
                    for (int i = 0; i < renderview_sources.size(); ++i) {
                        if (ImGui::Selectable(renderview_sources[i].name.cstr(), currentRenderViewIndex == i)) {
                            currentRenderViewIndex = i;
                        }
                    }
                    ImGui::EndCombo();
                }

                RenderViewSource &currentRenderViewSource = renderview_sources[currentRenderViewIndex];

                ImVec2 contentSize = ImGui::GetContentRegionAvail();
                float viewportAspectRatio = state.width / state.height;
                float scale = std::min(contentSize.x / state.width, contentSize.y / state.height);

                ImGui::Image((ImTextureID)currentRenderViewSource.rt->output, ImVec2(state.width * scale, state.height * scale), ImVec2(0, 1), ImVec2(1, 0));

                bool on_hovered = ImGui::IsItemHovered();
                auto &editor = the<Editor>();

                editor.ViewportIsOnEvent() = on_hovered;

                if (on_hovered) {

                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImVec2 cursorStartPos = ImGui::GetCursorScreenPos();
                    ImVec2 relativeMousePos = ImVec2(mousePos.x - cursorStartPos.x, mousePos.y - cursorStartPos.y);
                    float viewportMouseX = relativeMousePos.x / scale;
                    float viewportMouseY = (relativeMousePos.y + (state.height * scale)) / scale;
                    viewportMouseX = std::clamp(viewportMouseX, 0.0f, state.width);
                    viewportMouseY = std::clamp(viewportMouseY, 0.0f, state.height);

                    the<Editor>().PushEditorEvent(OnMouseMove, luavec2(viewportMouseX, viewportMouseY));

                    ImGui::Text("%f,%f", viewportMouseX, viewportMouseY);
                }
            }
            ImGui::End();

            the<Editor>().OnImGui();

            ImGui::End();  // EditorDockSpace
        }

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
                gBase.traceback.trash();
                gBase.fatal_error_string.trash();
            }
        }
    }

    the<ImGuiRender>().imgui_draw_post();

    window->SwapBuffer();
}

void CL::SplashScreen() {
    AssetTexture splash_texture = texure_aseprite_simple("@gamedata/assets/cat.ase");
    Asset splash_shader = {};
    bool ok = asset_load_kind(AssetKind_Shader, "@code/game/shader/splash.glsl", &splash_shader);
    error_assert(ok);
    GLuint sid = assets_get<AssetShader>(splash_shader).id;

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
    texture_bind(&splash_texture, 0);
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
    texture_release(&splash_texture);
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

    the<Scripting>().script_init();

    lua_State *L = ENGINE_LUA();

    lua_newtable(L);
    i32 conf_table = lua_gettop(L);

    if (!gBase.error_mode.load()) {
        // luax_neko_get(L, "conf");
        lua_getglobal(L, "__neko_conf");
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

    using Modules = std::tuple<Entity, Transform, Camera, Batch, Sprite, Tiled, Font, ImGuiRender, Sound, Editor, DebugDraw>;

    Neko::modules::initializeModulesHelper(Modules{}, std::make_index_sequence<std::tuple_size_v<Modules>>{});

    the<Entity>().entity_init();
    the<Transform>().transform_init();
    the<Camera>().camera_init();
    the<Batch>().batch_init(state.batch_vertex_capacity);
    the<Sprite>().sprite_init();
    the<Tiled>().tiled_init();
    the<Font>().font_init();
    the<ImGuiRender>().imgui_init();
    the<Sound>().sound_init();
    the<Editor>().edit_init();
    the<DebugDraw>().debug_draw_init();

    gBase.reload_interval.store(state.reload_interval);

    luax_run_nekogame(L);

    if (!gBase.error_mode.load() && state.startup_load_scripts) {
        load_all_lua_scripts(L);
    }

    // run main.lua
    asset_load_kind(AssetKind_LuaRef, "@code/game/script/nekomain.lua", nullptr);

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

    auto _key_down = [](KeyCode key, int scancode, int mode) {
        the<EventHandler>().EventPushLuaType(OnKeyDown, key);
        if (the<Editor>().ViewportIsOnEvent()) the<Editor>().PushEditorEvent(OnKeyDown, key);
    };

    auto _key_up = [](KeyCode key, int scancode, int mode) {
        the<EventHandler>().EventPushLuaType(OnKeyUp, key);
        if (the<Editor>().ViewportIsOnEvent()) the<Editor>().PushEditorEvent(OnKeyUp, key);
    };

    auto _char_down = [](unsigned int c) { /*gui_char_down(c);*/ };

    auto _mouse_down = [](MouseCode mouse) {
        the<EventHandler>().EventPushLuaType(OnMouseDown, mouse);
        if (the<Editor>().ViewportIsOnEvent()) the<Editor>().PushEditorEvent(OnMouseDown, mouse);
    };

    auto _mouse_up = [](MouseCode mouse) {
        the<EventHandler>().EventPushLuaType(OnMouseUp, mouse);
        if (the<Editor>().ViewportIsOnEvent()) the<Editor>().PushEditorEvent(OnMouseUp, mouse);
    };

    auto _mouse_move = [](vec2 pos) { the<EventHandler>().EventPushLuaType(OnMouseMove, pos); };

    auto _scroll = [](vec2 scroll) {
        the<EventHandler>().EventPushLuaType(OnMouseScroll, scroll);
        if (the<Editor>().ViewportIsOnEvent()) the<Editor>().PushEditorEvent(OnMouseScroll, scroll);
    };

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
            {EventMask::PreUpdate, [](Event evt) -> int { return the<Input>().OnPreUpdate(); }},

            {EventMask::Update,
             [](Event) -> int {
                 the<EventHandler>().EventPushLuaType(OnUpdate);
                 return 0;
             }},
            {EventMask::Update, [](Event evt) -> int { return the<Transform>().transform_update_all(evt); }},

            {EventMask::Update, [](Event evt) -> int { return the<Camera>().camera_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Sprite>().sprite_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Batch>().batch_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Sound>().OnUpdate(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Tiled>().tiled_update_all(evt); }},
            {EventMask::Update, [](Event evt) -> int { return the<Editor>().edit_update_all(evt); }},

            {EventMask::PostUpdate,
             [](Event) -> int {
                 the<EventHandler>().EventPushLuaType(OnPostUpdate);
                 return 0;
             }},
            {EventMask::PostUpdate, [](Event evt) -> int { return the<Sound>().OnPostUpdate(evt); }},
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

    renderview_source.create(state.width, state.height);

    posteffect_dark.create("@code/game/shader/post_dark.glsl", false);
    posteffect_bright.create("@code/game/shader/post_bright.glsl", false);
    posteffect_blur_h.create("@code/game/shader/post_blur_h.glsl");
    posteffect_blur_v.create("@code/game/shader/post_blur_v.glsl");
    posteffect_composite.create("@code/game/shader/post_composite.glsl");

    bool ok = asset_load_kind(AssetKind_Shader, "@code/game/shader/sprite2.glsl", &sprite_shader);
    error_assert(ok);

    quadrenderer.new_renderer(assets_get<AssetShader>(sprite_shader), neko_v2(state.width, state.height));

    renderview_sources = {
            {&renderview_source, "Editor View"},                                //
            {&posteffect_blur_v.GetRenderTarget(), "posteffect_blur"},          //
            {&posteffect_composite.GetRenderTarget(), "posteffect_composite"},  //
    };
}

int CL::set_window_title(const char *title) {
#if defined(__EMSCRIPTEN__)
    emscripten_set_window_title(title);
#else
    glfwSetWindowTitle(window->glfwWindow(), title);
#endif
    return 0;
}

void CL::fini() {
    PROFILE_FUNC();

    renderview_source.release();

    posteffect_composite.release();
    posteffect_blur_h.release();
    posteffect_blur_v.release();
    posteffect_bright.release();
    posteffect_dark.release();

    quadrenderer.free_renderer();

    bool dump_allocs_detailed = state.dump_allocs_detailed;

    {  // just for test

        mem_free(this->ui);
        neko_deinit_ui_renderer();
    }

    the<DebugDraw>().debug_draw_fini();
    the<Editor>().edit_fini();
    the<Scripting>().script_fini();
    the<Sound>().sound_fini();
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

    gBase.traceback.trash();
    gBase.fatal_error_string.trash();

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

extern Neko::CBase gBase;

static void _glfw_error_callback(int error, const char *desc) { fprintf(stderr, "glfw: %s\n", desc); }

Int32 Main(int argc, const char *argv[]) {
    bool ok = true;
    gBase.Init(argc, argv);

    glfwSetErrorCallback(_glfw_error_callback);
    if (!glfwInit()) {
        ok = false;
    }

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

int main(int argc, const char *argv[]) {
    Main(argc, argv);
    return 0;
}

#if 0
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
#endif