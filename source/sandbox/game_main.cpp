

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

// engine
#include "engine/modules/neko_flecslua.h"
#include "engine/neko.h"
#include "engine/neko.hpp"
#include "engine/neko_api.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_engine.h"
#include "engine/neko_gl.h"
#include "engine/neko_imgui.hpp"
#include "engine/neko_lua.hpp"
#include "engine/neko_luabind.hpp"
#include "engine/neko_math.h"
#include "engine/neko_platform.h"
#include "engine/neko_reflection.hpp"

// game
#include "game_main.h"
#include "game_physics_math.hpp"

using namespace neko;

class sandbox_game;
class neko_filesystem_t;

typedef struct neko_client_userdata_t {

    ecs_world_t *w;

    neko_imgui_context_t imgui = NEKO_DEFAULT_VAL();
    neko_handle(neko_render_renderpass_t) main_rp = {0};
    neko_handle(neko_render_framebuffer_t) main_fbo = {0};
    neko_handle(neko_render_texture_t) main_rt = {0};
    // neko_fontbatch_t font_render_batch;
    neko_client_cvar_t cl_cvar = NEKO_DEFAULT_VAL();
    // neko_thread_atomic_int_t init_thread_flag;
    // neko_thread_ptr_t init_work_thread;

    neko::thread init_work_thread;
    neko::sema init_work_sema;
    std::atomic_int init_work_flag;

    neko_vec2_t cam = {512, 512};
    u8 debug_mode;
    f32 player_v = 100.f;
} neko_client_userdata_t;

static_assert(std::is_trivially_copyable_v<neko_client_userdata_t>);
static_assert(sizeof(neko_client_userdata_t));

namespace neko::reflection {
template <>
Type *type_of<neko_pf_running_desc_t>() {
    static Type type;
    type.name = "neko_pf_running_desc_t";
    type.destroy = [](void *obj) { delete static_cast<neko_pf_running_desc_t *>(obj); };
    type.copy = [](const void *obj) { return (void *)(new neko_pf_running_desc_t(*static_cast<const neko_pf_running_desc_t *>(obj))); };
    type.move = [](void *obj) { return (void *)(new neko_pf_running_desc_t(std::move(*static_cast<neko_pf_running_desc_t *>(obj)))); };

#define REFL_FIELDS(C, field) type.fields.insert({#field, {type_of<decltype(C::field)>(), offsetof(C, field)}})

    REFL_FIELDS(neko_pf_running_desc_t, title);
    REFL_FIELDS(neko_pf_running_desc_t, width);
    REFL_FIELDS(neko_pf_running_desc_t, height);
    REFL_FIELDS(neko_pf_running_desc_t, flags);
    REFL_FIELDS(neko_pf_running_desc_t, num_samples);
    REFL_FIELDS(neko_pf_running_desc_t, monitor_index);
    REFL_FIELDS(neko_pf_running_desc_t, vsync);
    REFL_FIELDS(neko_pf_running_desc_t, frame_rate);

    return &type;
};
}  // namespace neko::reflection

// user data

neko_client_userdata_t g_client_userdata = NEKO_DEFAULT_VAL();

void editor_dockspace(neko_ui_context_t *ctx);

NEKO_API_DECL void neko_console(neko_console_t *console, neko_ui_context_t *ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t *desc);

// test
void test_xml(const std::string &file);
void test_se();
void test_containers();

void print(const float &val) { std::printf("%f", val); }

void print(const std::string &val) { std::printf("%s", val.c_str()); }

#if 0

struct Comp(Position) {
    // Register reflectable fields ('props') using the `Prop` macro. This can be used in any aggregate
    // `struct`, doesn't have to be a `Comp`.
    neko_prop(float, x) = 0;
    neko_prop(float, y) = 0;
};

struct Comp(Name) {
    neko_prop(std::string, first) = "First";

    // Props can have additional attributes, see the `neko_prop_attribs` type. You can customize and add
    // your own attributes there.
    neko_prop(std::string, last, .exampleFlag = true) = "Last";

    int internal = 42;  // This is a non-prop field that doesn't show up in reflection. IMPORTANT NOTE:
    // All such non-prop fields must be at the end of the struct, with all props at
    // the front. Mixing props and non-props in the order is not allowed.
};

// After all compnonent types are defined, this macro must be called to be able to use
// `forEachComponentType` to iterate all component types
UseComponentTypes();

#endif

void draw_gui(neko_client_userdata_t *game_userdata);

void game_init(neko_client_userdata_t *game_userdata) {

    game_userdata->cl_cvar.bg[0] = game_userdata->cl_cvar.bg[1] = game_userdata->cl_cvar.bg[2] = 28.f;

    {

        neko_lua_safe_dofile(neko_instance()->L, "main");

        // 获取 neko_game.table
        lua_getglobal(neko_instance()->L, "neko_game");
        if (!lua_istable(neko_instance()->L, -1)) {
            NEKO_ERROR("%s", "neko_game is not a table");
        }

        neko::reflection::Any v = neko_pf_running_desc_t{.title = "Neko Engine"};
        neko::lua::checktable_refl(neko_instance()->L, "app", v);

        neko_client_cvar_t &cvar = game_userdata->cl_cvar;
        // if (lua_getfield(neko_instance()->L, -1, "cvar") == LUA_TNIL) throw std::exception("no cvar");
        // if (lua_istable(neko_instance()->L, -1)) {
        //     neko::static_refl::neko_type_info<neko_client_cvar_t>::ForEachVarOf(cvar, [](auto field, auto &&value) {
        //         static_assert(std::is_lvalue_reference_v<decltype(value)>);
        //         if (lua_getfield(neko_instance()->L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(neko_instance()->L, -1);
        //         lua_pop(neko_instance()->L, 1);
        //     });
        // } else {
        //     throw std::exception("no cvar table");
        // }
        // lua_pop(neko_instance()->L, 1);

        cvar.show_editor = false;
        cvar.show_demo_window = false;
        cvar.show_pack_editor = false;
        cvar.show_profiler_window = false;
        cvar.show_test_window = false;
        cvar.show_gui = false;
        cvar.shader_inspect = false;
        cvar.hello_ai_shit = false;
        cvar.vsync = false;
        cvar.is_hotfix = false;

        lua_pop(neko_instance()->L, 1);  // 弹出 neko_game.table

        NEKO_INFO("load game: %s %d %d", v.cast<neko_pf_running_desc_t>().title, v.cast<neko_pf_running_desc_t>().width, v.cast<neko_pf_running_desc_t>().height);

        neko_pf_set_window_title(neko_pf_main_window(), v.cast<neko_pf_running_desc_t>().title);
    }

    bool ok = neko_pack_read("gamedir/res.pack", 0, false, &ENGINE_INTERFACE()->pack);
    NEKO_ASSERT(ok == true);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    neko_pack_item_data(&ENGINE_INTERFACE()->pack, ".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    neko_pack_item_data(&ENGINE_INTERFACE()->pack, ".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);

    ENGINE_INTERFACE()->test_ase = neko_aseprite_simple(cat_data, cat_data_size);

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file("gamedir/assets/textures/yzh.png", &tex0, NULL, false, false);
    ENGINE_INTERFACE()->tex_hndl = neko_assets_create_asset(&ENGINE_INTERFACE()->am, neko_asset_texture_t, &tex0);

    neko_ui_init(&ENGINE_INTERFACE()->ui, neko_pf_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    // neko_asset_font_load_from_memory(font_data, font_data_size, &CL_GAME_INTERFACE()->font, 24);
    neko_asset_font_load_from_file("gamedir/assets/fonts/monocraft.ttf", &ENGINE_INTERFACE()->font, 24);

    neko_pack_item_free(&ENGINE_INTERFACE()->pack, font_data);
    neko_pack_item_free(&ENGINE_INTERFACE()->pack, cat_data);

    auto GUI_FONT_STASH = []() -> neko_ui_font_stash_desc_t * {
        static neko_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &ENGINE_INTERFACE()->font}};
        static neko_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_ui_font_desc_t)};
        return &font_stash;
    }();

    neko_ui_init_font_stash(&ENGINE_INTERFACE()->ui, GUI_FONT_STASH);

    neko_ui_dock_ex(&ENGINE_INTERFACE()->ui, "Style_Editor", "Demo_Window", NEKO_UI_SPLIT_TAB, 0.5f);

    game_userdata->imgui = neko_imgui_new(&ENGINE_INTERFACE()->cb, neko_pf_main_window(), false);

    // Construct frame buffer
    game_userdata->main_fbo = neko_render_framebuffer_create({});

    neko_render_texture_desc_t main_rt_desc = {.width = (u32)neko_game().DisplaySize.x,
                                               .height = (u32)neko_game().DisplaySize.y,
                                               .format = R_TEXTURE_FORMAT_RGBA8,
                                               .wrap_s = R_TEXTURE_WRAP_REPEAT,
                                               .wrap_t = R_TEXTURE_WRAP_REPEAT,
                                               .min_filter = R_TEXTURE_FILTER_NEAREST,
                                               .mag_filter = R_TEXTURE_FILTER_NEAREST};
    game_userdata->main_rt = neko_render_texture_create(main_rt_desc);

    neko_render_renderpass_desc_t main_rp_desc = {.fbo = game_userdata->main_fbo, .color = &game_userdata->main_rt, .color_size = sizeof(game_userdata->main_rt)};
    game_userdata->main_rp = neko_render_renderpass_create(main_rp_desc);

    // Neko_Module sound_module = neko_module_open("neko_module_sound");
    // auto sound_module_loader = reinterpret_cast<s32 (*)(Neko_Module *, Neko_ModuleInterface *)>(neko_module_get_symbol(sound_module, Neko_OnModuleLoad_FuncName));
    // s32 sss = sound_module_loader(&sound_module, CL_GAME_INTERFACE());
    // NEKO_TRACE("%d", sss);
    // sound_module.func.OnInit(neko_instance()->L);
    // neko_module_close(sound_module);

    // 初始化工作

    // static auto init_work = +[](void *user_data) {
    //     neko_client_userdata_t *cl = (neko_client_userdata_t *)user_data;

    //     neko::timer timer;
    //     timer.start();

    //     neko_lua_call(neko_instance()->L, "game_init_thread");

    //     timer.stop();
    //     NEKO_INFO(std::format("game_init_thread loading done in {0:.3f} ms", timer.get()).c_str());

    //     cl->init_work_sema.post();
    //     cl->init_work_flag.store(1);
    // };

    // game_userdata->init_work_sema.make();

    // game_userdata->init_work_flag.store(0);

    // game_userdata->init_work_thread.make(init_work, game_userdata);

    // thread_atomic_int_store(&game_userdata->init_thread_flag, 0);

    // game_userdata->init_work_thread = thread_init(init_work, &game_userdata->init_thread_flag, "init_work_thread", THREAD_STACK_SIZE_DEFAULT);

    game_userdata->w = ecs_init();

    ECS_IMPORT(game_userdata->w, FlecsLua);

    ecs_lua_set_state(game_userdata->w, neko_instance()->L);

    neko_lua_call(neko_instance()->L, "game_init");

    neko::timer timer;
    timer.start();
    neko_lua_call(neko_instance()->L, "game_init_thread");
    timer.stop();
    NEKO_INFO(std::format("game_init_thread loading done in {0:.3f} ms", timer.get()).c_str());
}

void game_loop(neko_client_userdata_t *game_userdata) {

    PROFILE_FUNC();

    // int this_init_thread_flag = game_userdata->init_work_flag;

    // if (this_init_thread_flag == 0) {

    //     //        neko_vec2 fbs = neko_pf_framebuffer_sizev(neko_pf_main_window());
    //     const f32 t = neko_pf_elapsed_time();

    //     u8 tranp = 255;

    //     // tranp -= ((s32)t % 255);

    //     neko_render_clear_action_t clear = {.color = {game_userdata->cl_cvar.bg[0] / 255, game_userdata->cl_cvar.bg[1] / 255, game_userdata->cl_cvar.bg[2] / 255, 1.f}};
    //     neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, neko_renderpass_t{0});
    //     { neko_render_clear(&ENGINE_INTERFACE()->cb, clear); }
    //     neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

    //     // Set up 2D camera for projection matrix
    //     neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
    //     neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, (u32)neko_game().DisplaySize.x, (u32)neko_game().DisplaySize.y);

    //     // 底层图片
    //     char background_text[64] = "Project: unknown";

    //     neko_vec2 td = neko_asset_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
    //     neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

    //     neko_idraw_text(&ENGINE_INTERFACE()->idraw, (neko_game().DisplaySize.x - td.x) * 0.5f, (neko_game().DisplaySize.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false,
    //                     neko_color(255, 255, 255, 255));
    //     neko_idraw_texture(&ENGINE_INTERFACE()->idraw, ENGINE_INTERFACE()->test_ase);
    //     neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, neko_v2((neko_game().DisplaySize.x - ts.x) * 0.5f, (neko_game().DisplaySize.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f),
    //                       neko_v2(1.f, 0.f), neko_color(255, 255, 255, tranp), R_PRIMITIVE_TRIANGLES);

    //     neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, R_RENDER_PASS_DEFAULT);
    //     {
    //         neko_render_set_viewport(&ENGINE_INTERFACE()->cb, 0, 0, (u32)neko_game().DisplaySize.x, (u32)neko_game().DisplaySize.y);
    //         neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);  // 立即模式绘制 idraw
    //     }
    //     neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

    // } else
    {

        // NEKO_STATIC int init_retval = 1;
        // if (init_retval) {
        //     // init_retval = thread_join(game_userdata->init_work_thread);
        //     // thread_term(game_userdata->init_work_thread);
        //     game_userdata->init_work_thread.join();
        //     // game_userdata->init_work_sema.wait();
        //     game_userdata->init_work_sema.trash();
        //     NEKO_TRACE("init_work_thread done");
        // }

        f32 dt = neko_pf_delta_time();

        {
            PROFILE_BLOCK("lua_pre_update");
            neko_lua_call(neko_instance()->L, "game_pre_update");
        }

        if (neko_pf_key_pressed(NEKO_KEYCODE_ESC)) game_userdata->cl_cvar.show_editor ^= true;

        {
            PROFILE_BLOCK("lua_loop");
            neko_lua_call<void, f32>(neko_instance()->L, "game_loop", dt);
        }

        // Do rendering
        neko_render_clear_action_t clear = {.color = {game_userdata->cl_cvar.bg[0] / 255, game_userdata->cl_cvar.bg[1] / 255, game_userdata->cl_cvar.bg[2] / 255, 1.f}};
        neko_render_clear_action_t b_clear = {.color = {0.0f, 0.0f, 0.0f, 1.f}};

        neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, R_RENDER_PASS_DEFAULT);
        { neko_render_clear(&ENGINE_INTERFACE()->cb, clear); }
        neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

        neko_imgui_new_frame(&game_userdata->imgui);

        draw_gui(game_userdata);

        if (ImGui::Begin("Debug")) {
            neko::imgui::Auto<neko_vec2_t>(game_userdata->cam, "Cam");
            ImGui::Text("%lld", neko_lua_mem_usage());
            ImGui::End();
        }

        if (game_userdata->cl_cvar.show_demo_window) ImGui::ShowDemoWindow();

        if (game_userdata->cl_cvar.show_editor && ImGui::Begin("Cvar")) {

            neko_cvar_gui(game_userdata->cl_cvar);

            neko::imgui::toggle("帧检查器", &game_userdata->cl_cvar.show_profiler_window);

            ImGui::DragFloat("Max FPS", &neko_subsystem(platform)->time.max_fps, 5.0f, 30.0f, 2000.0f);

            ImGui::Separator();

            if (ImGui::Button("test_xml")) test_xml("gamedir/tests/test.xml");
            if (ImGui::Button("test_se")) test_se();
            if (ImGui::Button("test_containers")) test_containers();
#if 0
            if (ImGui::Button("test_ecs_cpp")) {
                forEachComponentType([&]<typename T>() {
                    constexpr auto typeName = getTypeName<T>();
                    std::printf("component type: %.*s\n", int(typeName.size()), typeName.data());

                    T val{};
                    std::printf("  props:\n");
                    forEachProp(val, [&](auto propTag, auto &propVal) {
                        std::printf("    %s: ", propTag.attribs.name.data());
                        print(propVal);
                        if (propTag.attribs.exampleFlag) {
                            std::printf(" (example flag set)");
                        }
                        std::printf("\n");
                    });
                });
            }
#endif

            ImGui::End();
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("engine")) {
                if (ImGui::Checkbox("vsync", &game_userdata->cl_cvar.vsync)) neko_pf_enable_vsync(game_userdata->cl_cvar.vsync);
                if (ImGui::MenuItem("quit")) {
                    neko_quit();
                }

                ImGui::EndMenu();
            }

            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "nEKO");

            if (ImGui::BeginMenu("File")) {
                ImGui::EndMenu();
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 210 - ImGui::GetScrollX());
            ImGui::Text("%.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::EndMainMenuBar();
        }

        //        sandbox_update(sb);

        neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
        neko_idraw_camera2d_ex(&ENGINE_INTERFACE()->idraw, game_userdata->cam.x, game_userdata->cam.x + neko_game().DisplaySize.x, game_userdata->cam.y,
                               game_userdata->cam.y + neko_game().DisplaySize.y);

        neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, game_userdata->main_rp);
        {
            neko_render_set_viewport(&ENGINE_INTERFACE()->cb, 0, 0, (u32)neko_game().DisplaySize.x, (u32)neko_game().DisplaySize.y);
            neko_render_clear(&ENGINE_INTERFACE()->cb, clear);
            neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);  // 立即模式绘制 idraw
        }
        neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

        neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
        neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, (u32)neko_game().DisplaySize.x, (u32)neko_game().DisplaySize.y);
        neko_idraw_texture(&ENGINE_INTERFACE()->idraw, game_userdata->main_rt);
        neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, neko_v2(0.0, 0.0), neko_v2((u32)neko_game().DisplaySize.x, (u32)neko_game().DisplaySize.y), neko_v2(0.0, 1.0), neko_v2(1.0, 0.0),
                          neko_color(255, 255, 255, 255), R_PRIMITIVE_TRIANGLES);

        neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, R_RENDER_PASS_DEFAULT);
        {
            neko_render_set_viewport(&ENGINE_INTERFACE()->cb, 0.0, 0.0, neko_game().DisplaySize.x, neko_game().DisplaySize.y);
            neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);  // 立即模式绘制 idraw
            neko_ui_render(&ENGINE_INTERFACE()->ui, &ENGINE_INTERFACE()->cb);
        }
        neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);
        {
            PROFILE_BLOCK("lua_render");
            neko_lua_call(neko_instance()->L, "game_render");
        }

        auto &module_list = ENGINE_INTERFACE()->modules;
        for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
            Neko_Module &module = module_list[i];
            module.func.OnUpdate(neko_instance()->L);
        }

        {
            PROFILE_BLOCK("imgui_submit");
            neko_imgui_render(&game_userdata->imgui);
        }
    }
}

void game_post_update() {}

void game_fini(neko_client_userdata_t *game_userdata) {

    neko_render_renderpass_destroy(game_userdata->main_rp);
    neko_render_texture_destroy(game_userdata->main_rt);
    neko_render_framebuffer_destroy(game_userdata->main_fbo);

    neko_lua_call(neko_instance()->L, "game_fini");

    neko_imgui_shutdown(&game_userdata->imgui);
    neko_ui_free(&ENGINE_INTERFACE()->ui);
    neko_pack_destroy(&ENGINE_INTERFACE()->pack);

    // ecs_fini(game_userdata->w);

#ifdef USE_PROFILER
    neko::profile_shutdown();
#endif
}

void editor_dockspace(neko_ui_context_t *ctx) {
    u64 opt = NEKO_UI_OPT_NOCLIP | NEKO_UI_OPT_NOFRAME | NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_DOCKSPACE | NEKO_UI_OPT_FULLSCREEN | NEKO_UI_OPT_NOMOVE |
              NEKO_UI_OPT_NOBRINGTOFRONT | NEKO_UI_OPT_NOFOCUS | NEKO_UI_OPT_NORESIZE;
    neko_ui_window_begin_ex(ctx, "Dockspace", neko_ui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Editor dockspace
    }
    neko_ui_window_end(ctx);
}

///////////////////////////////////////////////
//
//  测试UI

s32 button_custom(neko_ui_context_t *ctx, const char *label) {
    // Do original button call
    s32 res = neko_ui_button(ctx, label);

    // Draw inner shadows/highlights over button
    neko_color_t hc = NEKO_COLOR_WHITE, sc = neko_color(85, 85, 85, 255);
    neko_ui_rect_t r = ctx->last_rect;
    s32 w = 2;
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x + w, r.y, r.w - 2 * w, w), hc);
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x + w, r.y + r.h - w, r.w - 2 * w, w), sc);
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x, r.y, w, r.h), hc);
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x + r.w - w, r.y, w, r.h), sc);

    return res;
}

void draw_gui(neko_client_userdata_t *game_userdata) {

    PROFILE_FUNC();

    const f64 t = neko_pf_elapsed_time();

    // Custom callback for immediate drawing directly into the gui window
    auto gui_cb = [](neko_ui_context_t *ctx, struct neko_ui_customcommand_t *cmd) {
        neko_immediate_draw_t *gui_idraw = &ctx->gui_idraw;  // Immediate draw list in gui context
        neko_vec2 fbs = ctx->framebuffer_size;               // Framebuffer size bound for gui context
        neko_color_t *color = (neko_color_t *)cmd->data;     // Grab custom data
        neko_asset_texture_t *tp = neko_assets_getp(&ENGINE_INTERFACE()->am, neko_asset_texture_t, ENGINE_INTERFACE()->tex_hndl);
        const f32 t = neko_pf_elapsed_time();

        // Set up an immedaite camera using our passed in cmd viewport (this is the clipped viewport of the gui window being drawn)
        neko_idraw_camera3d(gui_idraw, (u32)cmd->viewport.w, (u32)cmd->viewport.h);
        neko_idraw_blend_enabled(gui_idraw, true);
        neko_render_set_viewport(&gui_idraw->commands, cmd->viewport.x, fbs.y - cmd->viewport.h - cmd->viewport.y, cmd->viewport.w, cmd->viewport.h);
        neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        {
            neko_idraw_rotatev(gui_idraw, t * 0.001f, NEKO_YAXIS);
            neko_idraw_scalef(gui_idraw, 0.5f, 0.5f, 0.5f);
            neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, color->r, color->g, color->b, color->a, R_PRIMITIVE_LINES);
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Set up 2D camera for projection matrix
        neko_idraw_camera2d(gui_idraw, (u32)fbs.x, (u32)fbs.y);

        // Rect
        neko_idraw_rectv(gui_idraw, neko_v2(500.f, 50.f), neko_v2(600.f, 100.f), NEKO_COLOR_RED, R_PRIMITIVE_TRIANGLES);
        neko_idraw_rectv(gui_idraw, neko_v2(650.f, 50.f), neko_v2(750.f, 100.f), NEKO_COLOR_RED, R_PRIMITIVE_LINES);

        // Triangle
        neko_idraw_trianglev(gui_idraw, neko_v2(50.f, 50.f), neko_v2(100.f, 100.f), neko_v2(50.f, 100.f), NEKO_COLOR_WHITE, R_PRIMITIVE_TRIANGLES);
        neko_idraw_trianglev(gui_idraw, neko_v2(200.f, 50.f), neko_v2(300.f, 100.f), neko_v2(200.f, 100.f), NEKO_COLOR_WHITE, R_PRIMITIVE_LINES);

        // Lines
        neko_idraw_linev(gui_idraw, neko_v2(50.f, 20.f), neko_v2(500.f, 20.f), neko_color(0, 255, 0, 255));

        // Circle
        neko_idraw_circle(gui_idraw, 350.f, 170.f, 50.f, 20, 100, 150, 220, 255, R_PRIMITIVE_TRIANGLES);
        neko_idraw_circle(gui_idraw, 250.f, 170.f, 50.f, 20, 100, 150, 220, 255, R_PRIMITIVE_LINES);

        // Circle Sector
        neko_idraw_circle_sector(gui_idraw, 50.f, 150.f, 50.f, 0, 90, 32, 255, 255, 255, 255, R_PRIMITIVE_TRIANGLES);
        neko_idraw_circle_sector(gui_idraw, 150.f, 200.f, 50.f, 90, 270, 32, 255, 255, 255, 255, R_PRIMITIVE_LINES);

        // Box (with texture)
        neko_idraw_depth_enabled(gui_idraw, true);
        neko_idraw_face_cull_enabled(gui_idraw, true);
        neko_idraw_camera3d(gui_idraw, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        {
            neko_idraw_translatef(gui_idraw, -2.f, -1.f, -5.f);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.001f, NEKO_YAXIS);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.0005f, NEKO_ZAXIS);
            neko_idraw_texture(gui_idraw, tp->hndl);
            neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
            neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, R_PRIMITIVE_TRIANGLES);
            neko_idraw_texture(gui_idraw, neko_handle(neko_render_texture_t){0});
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Box (lines, no texture)
        neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        {
            neko_idraw_translatef(gui_idraw, 2.f, -1.f, -5.f);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.001f, NEKO_YAXIS);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.0008f, NEKO_ZAXIS);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.0009f, NEKO_XAXIS);
            neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
            neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 200, 100, 255, R_PRIMITIVE_LINES);
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Sphere (triangles, no texture)
        neko_idraw_camera3d(gui_idraw, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        {
            neko_idraw_translatef(gui_idraw, -2.f, -1.f, -5.f);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.001f, NEKO_YAXIS);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.0005f, NEKO_ZAXIS);
            neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
            neko_idraw_sphere(gui_idraw, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, R_PRIMITIVE_TRIANGLES);
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Sphere (lines)
        neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        {
            neko_idraw_translatef(gui_idraw, 2.f, -1.f, -5.f);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.001f, NEKO_YAXIS);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.0008f, NEKO_ZAXIS);
            neko_idraw_rotatev(gui_idraw, neko_pf_elapsed_time() * 0.0009f, NEKO_XAXIS);
            neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
            neko_idraw_sphere(gui_idraw, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, R_PRIMITIVE_LINES);
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Text (custom and default fonts)
        // neko_idraw_camera2D(gui_idraw, (u32)ws.x, (u32)ws.y);
        // neko_idraw_defaults(gui_idraw);
        // neko_idraw_text(gui_idraw, 410.f, 150.f, "Custom Font", &font, false, 200, 100, 50, 255);
        // neko_idraw_text(gui_idraw, 450.f, 200.f, "Default Font", NULL, false, 50, 100, 255, 255);
    };

    neko_ui_begin(&ENGINE_INTERFACE()->ui, NULL);
    {
        editor_dockspace(&ENGINE_INTERFACE()->ui);

        if (game_userdata->cl_cvar.show_gui) {

            neko_ui_demo_window(&ENGINE_INTERFACE()->ui, neko_ui_rect(100, 100, 500, 500), NULL);
            neko_ui_style_editor(&ENGINE_INTERFACE()->ui, NULL, neko_ui_rect(350, 250, 300, 240), NULL);

            const neko_vec2 ws = neko_v2(600.f, 300.f);

            // const neko_ui_style_sheet_t *ss = &game_userdata->style_sheet;

            const neko_vec2 ss_ws = neko_v2(500.f, 300.f);
            neko_ui_window_begin(&ENGINE_INTERFACE()->ui, "Window", neko_ui_rect((neko_game().DisplaySize.x - ss_ws.x) * 0.5f, (neko_game().DisplaySize.y - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
            {
                // Cache the current container
                neko_ui_container_t *cnt = neko_ui_get_current_container(&ENGINE_INTERFACE()->ui);

                neko_ui_layout_row(&ENGINE_INTERFACE()->ui, 2, neko_ui_widths(200, 0), 0);

                neko_ui_text(&ENGINE_INTERFACE()->ui, "A regular element button.");
                neko_ui_button(&ENGINE_INTERFACE()->ui, "button");

                neko_ui_text(&ENGINE_INTERFACE()->ui, "A regular element label.");
                neko_ui_label(&ENGINE_INTERFACE()->ui, "label");

                neko_ui_text(&ENGINE_INTERFACE()->ui, "Button with classes: {.c0 .btn}");

                neko_ui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                neko_ui_button_ex(&ENGINE_INTERFACE()->ui, "hello?##btn", &selector_1, 0x00);

                neko_ui_text(&ENGINE_INTERFACE()->ui, "Label with id #lbl and class .c0");
                neko_ui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                neko_ui_label_ex(&ENGINE_INTERFACE()->ui, "label##lbl", &selector_2, 0x00);

                const f32 m = cnt->body.w * 0.3f;
                // neko_ui_layout_row(gui, 2, (int[]){m, -m}, 0);
                // neko_ui_layout_next(gui); // Empty space at beginning
                neko_ui_layout_row(&ENGINE_INTERFACE()->ui, 1, neko_ui_widths(0), 0);
                neko_ui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                if (neko_ui_button_ex(&ENGINE_INTERFACE()->ui, "reload style sheet", &selector_3, 0x00)) {
                    // app_load_style_sheet(true);
                }

                button_custom(&ENGINE_INTERFACE()->ui, "Hello?");
            }
            neko_ui_window_end(&ENGINE_INTERFACE()->ui);

            neko_ui_window_begin(&ENGINE_INTERFACE()->ui, "Idraw", neko_ui_rect((neko_game().DisplaySize.x - ws.x) * 0.2f, (neko_game().DisplaySize.y - ws.y) * 0.5f, ws.x, ws.y));
            {
                // Cache the current container
                neko_ui_container_t *cnt = neko_ui_get_current_container(&ENGINE_INTERFACE()->ui);

                // 绘制到当前窗口中的转换对象的自定义回调
                // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                // 正如我们将在“分割”窗口示例中看到的那样。
                // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                // 你不需要任何东西
                neko_color_t color = neko_color_alpha(NEKO_COLOR_RED, (uint8_t)NEKO_CLAMP((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                neko_ui_draw_custom(&ENGINE_INTERFACE()->ui, cnt->body, gui_cb, &color, sizeof(color));
            }
            neko_ui_window_end(&ENGINE_INTERFACE()->ui);
        }

        if (neko_pf_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
            g_console.open = !g_console.open;
        } else if (neko_pf_key_pressed(NEKO_KEYCODE_TAB) && g_console.open) {
            g_console.autoscroll = !g_console.autoscroll;
        }

        neko_ui_layout_t l;

        if (game_userdata->cl_cvar.show_gui) {

            neko_vec2 mp = neko_pf_mouse_positionv();
            neko_vec2 mw = neko_pf_mouse_wheelv();
            neko_vec2 md = neko_pf_mouse_deltav();
            bool lock = neko_pf_mouse_locked();
            bool moved = neko_pf_mouse_moved();

            if (neko_ui_window_begin(&ENGINE_INTERFACE()->ui, "App", neko_ui_rect(neko_game().DisplaySize.x - 210, 30, 200, 200))) {
                l = *neko_ui_get_layout(&ENGINE_INTERFACE()->ui);
                neko_ui_layout_row(&ENGINE_INTERFACE()->ui, 1, neko_ui_widths(-1), 0);

                static f32 delta, fps = NEKO_DEFAULT_VAL();

                NEKO_TIMED_ACTION(5, {
                    delta = neko_pf_delta_time();
                    fps = 1.f / delta;
                });

                neko_ui_labelf("FPS: %.2lf Delta: %.6lf", fps, delta * 1000.f);

                // neko_ui_text_fc(&game_userdata->core_ui, "喵喵昂~");

                // neko_ui_layout_row(&game_userdata->core_ui, 1, neko_ui_widths(-1), 0);

                neko_ui_labelf("Position: <%.2f %.2f>", mp.x, mp.y);
                neko_ui_labelf("Wheel: <%.2f %.2f>", mw.x, mw.y);
                neko_ui_labelf("Delta: <%.2f %.2f>", md.x, md.y);
                neko_ui_labelf("Lock: %zu", lock);
                neko_ui_labelf("Moved: %zu", moved);
                // neko_ui_labelf("Hover: %zu", g_gui.mouse_is_hover);
                neko_ui_labelf("Time: %f", t);

                struct {
                    const char *str;
                    s32 val;
                } btns[] = {{"Left", NEKO_MOUSE_LBUTTON}, {"Right", NEKO_MOUSE_RBUTTON}, {"Middle", NEKO_MOUSE_MBUTTON}, {NULL}};

                bool mouse_down[3] = {0};
                bool mouse_pressed[3] = {0};
                bool mouse_released[3] = {0};

                // Query mouse held down states.
                mouse_down[NEKO_MOUSE_LBUTTON] = neko_pf_mouse_down(NEKO_MOUSE_LBUTTON);
                mouse_down[NEKO_MOUSE_RBUTTON] = neko_pf_mouse_down(NEKO_MOUSE_RBUTTON);
                mouse_down[NEKO_MOUSE_MBUTTON] = neko_pf_mouse_down(NEKO_MOUSE_MBUTTON);

                // Query mouse release states.
                mouse_released[NEKO_MOUSE_LBUTTON] = neko_pf_mouse_released(NEKO_MOUSE_LBUTTON);
                mouse_released[NEKO_MOUSE_RBUTTON] = neko_pf_mouse_released(NEKO_MOUSE_RBUTTON);
                mouse_released[NEKO_MOUSE_MBUTTON] = neko_pf_mouse_released(NEKO_MOUSE_MBUTTON);

                // Query mouse pressed states. Press is a single frame click.
                mouse_pressed[NEKO_MOUSE_LBUTTON] = neko_pf_mouse_pressed(NEKO_MOUSE_LBUTTON);
                mouse_pressed[NEKO_MOUSE_RBUTTON] = neko_pf_mouse_pressed(NEKO_MOUSE_RBUTTON);
                mouse_pressed[NEKO_MOUSE_MBUTTON] = neko_pf_mouse_pressed(NEKO_MOUSE_MBUTTON);

                neko_ui_layout_row(&ENGINE_INTERFACE()->ui, 7, neko_ui_widths(100, 100, 32, 100, 32, 100, 32), 0);
                for (u32 i = 0; btns[i].str; ++i) {
                    neko_ui_labelf("%s: ", btns[i].str);
                    neko_ui_labelf("pressed: ");
                    neko_ui_labelf("%d", mouse_pressed[btns[i].val]);
                    neko_ui_labelf("down: ");
                    neko_ui_labelf("%d", mouse_down[btns[i].val]);
                    neko_ui_labelf("released: ");
                    neko_ui_labelf("%d", mouse_released[btns[i].val]);
                }

                neko_ui_layout_row(&ENGINE_INTERFACE()->ui, 1, neko_ui_widths(-1), 0);
                {
                    static neko_memory_info_t meminfo = NEKO_DEFAULT_VAL();

                    NEKO_TIMED_ACTION(60, { meminfo = neko_pf_memory_info(); });

                    neko_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                    neko_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                    neko_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                    lua_Integer kb = lua_gc(neko_instance()->L, LUA_GCCOUNT, 0);
                    lua_Integer bytes = lua_gc(neko_instance()->L, LUA_GCCOUNTB, 0);

                    neko_ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                    neko_ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                    neko_ui_labelf("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

                    neko_ui_labelf("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                    neko_ui_labelf("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                    neko_ui_labelf("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                    neko_ui_labelf("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                    neko_ui_labelf("GPU MemTotalAvailable: %.2lf mb", (f64)(meminfo.gpu_total_memory / 1024.0f));
                    neko_ui_labelf("GPU MemCurrentUsage: %.2lf mb", (f64)(meminfo.gpu_memory_used / 1024.0f));

                    neko_render_info_t *info = &neko_subsystem(render)->info;

                    neko_ui_labelf("OpenGL vendor: %s", info->vendor);
                    neko_ui_labelf("OpenGL version supported: %s", info->version);
                }

                neko_ui_window_end(&ENGINE_INTERFACE()->ui);
            }
        }

        neko_vec2 fb = (&ENGINE_INTERFACE()->ui)->framebuffer_size;
        neko_ui_rect_t screen;
        //            if (embeded)
        //                screen = l.body;
        //            else
        //                screen = neko_ui_rect(0, 0, fb.x, fb.y);
        screen = neko_ui_rect(0, 0, fb.x, fb.y);
        neko_console(&g_console, &ENGINE_INTERFACE()->ui, screen, NULL);
    }
    neko_ui_end(&ENGINE_INTERFACE()->ui, true);
}

void neko_app(lua_State *L) {

    lua_register(
            L, "sandbox_init", +[](lua_State *L) {
                auto ud = neko::lua::udata_new<neko_client_userdata_t>(L, 1);
                PROFILE_FUNC();
                game_init(ud);
                return 1;
            });

    lua_register(
            L, "sandbox_update", +[](lua_State *L) {
                auto ud = neko::lua::toudata_ptr<neko_client_userdata_t>(L, 1);
                PROFILE_FUNC();
                game_loop(ud);
                return 0;
            });

    lua_register(
            L, "sandbox_fini", +[](lua_State *L) {
                auto ud = neko::lua::toudata_ptr<neko_client_userdata_t>(L, 1);
                PROFILE_FUNC();
                game_fini(ud);
                return 0;
            });
}