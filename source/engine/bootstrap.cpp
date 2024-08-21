
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/api.hpp"
#include "engine/asset.h"
#include "engine/base.h"
#include "engine/draw.h"
#include "engine/game.h"
#include "engine/luabind.hpp"
#include "engine/luax.hpp"
#include "engine/math.h"
#include "engine/base.hpp"
#include "engine/prelude.h"

// deps
#include "vendor/sokol_time.h"

#if 0

using namespace neko;

namespace neko::reflection {
template <>
Type *type_of<neko_os_running_desc_t>() {
    static Type type;
    type.name = "neko_os_running_desc_t";
    type.destroy = [](void *obj) { delete static_cast<neko_os_running_desc_t *>(obj); };
    type.copy = [](const void *obj) { return (void *)(new neko_os_running_desc_t(*static_cast<const neko_os_running_desc_t *>(obj))); };
    type.move = [](void *obj) { return (void *)(new neko_os_running_desc_t(std::move(*static_cast<neko_os_running_desc_t *>(obj)))); };

#define REFL_FIELDS(C, field) type.fields.insert({#field, {type_of<decltype(C::field)>(), offsetof(C, field)}})

    REFL_FIELDS(neko_os_running_desc_t, title);
    REFL_FIELDS(neko_os_running_desc_t, width);
    REFL_FIELDS(neko_os_running_desc_t, height);
    REFL_FIELDS(neko_os_running_desc_t, flags);
    REFL_FIELDS(neko_os_running_desc_t, num_samples);
    REFL_FIELDS(neko_os_running_desc_t, monitor_index);
    REFL_FIELDS(neko_os_running_desc_t, vsync);
    REFL_FIELDS(neko_os_running_desc_t, frame_rate);

    return &type;
};
}  // namespace neko::reflection

void game_init() {

    neko_game()->cvar.bg[0] = neko_game()->cvar.bg[1] = neko_game()->cvar.bg[2] = 28.f;

    {

        neko_lua_safe_dofile(ENGINE_LUA(), "main");

        luax_get(ENGINE_LUA(), "neko", "__define_default_callbacks");
        luax_pcall(ENGINE_LUA(), 0, 0);

        // 获取 neko.conf.table
        luax_get(ENGINE_LUA(), "neko", "conf");
        if (!lua_istable(ENGINE_LUA(), -1)) {
            console_log("%s", "neko_game is not a table");
        }

        neko::reflection::Any v = neko_os_running_desc_t{.title = "Neko Engine"};
        neko::lua::checktable_refl(ENGINE_LUA(), "app", v);

        neko_client_cvar_t &cvar = neko_game()->cvar;
        // if (lua_getfield(ENGINE_L(), -1, "cvar") == LUA_TNIL) throw std::exception("no cvar");
        // if (lua_istable(ENGINE_L(), -1)) {
        //     neko::static_refl::neko_type_info<neko_client_cvar_t>::ForEachVarOf(cvar, [](auto field, auto &&value) {
        //         static_assert(std::is_lvalue_reference_v<decltype(value)>);
        //         if (lua_getfield(ENGINE_L(), -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(ENGINE_L(), -1);
        //         lua_pop(ENGINE_L(), 1);
        //     });
        // } else {
        //     throw std::exception("no cvar table");
        // }
        // lua_pop(ENGINE_L(), 1);

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

        lua_pop(ENGINE_LUA(), 1);  // 弹出 neko.conf.table

        console_log("load game: %s %d %d", v.cast<neko_os_running_desc_t>().title, v.cast<neko_os_running_desc_t>().width, v.cast<neko_os_running_desc_t>().height);

        neko_os_set_window_title(neko_os_main_window(), v.cast<neko_os_running_desc_t>().title);
    }

    bool ok = ENGINE_INTERFACE()->pack.load("gamedir/res.pack", 0, false);
    neko_assert(ok == true);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    ENGINE_INTERFACE()->pack.get_data(".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    ENGINE_INTERFACE()->pack.get_data(".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);

    ENGINE_INTERFACE()->test_ase = neko_aseprite_simple(cat_data, cat_data_size);

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file("gamedir/assets/textures/yzh.png", &tex0, NULL, false, false);
    ENGINE_INTERFACE()->tex_hndl = neko_assets_create_asset(&ENGINE_INTERFACE()->am, neko_asset_texture_t, &tex0);

    ui_init(&ENGINE_INTERFACE()->ui, neko_os_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    // neko_asset_font_load_from_memory(font_data, font_data_size, &CL_GAME_INTERFACE()->font, 24);
    neko_asset_font_load_from_file("gamedir/assets/fonts/monocraft.ttf", &ENGINE_INTERFACE()->font, 24);

    ENGINE_INTERFACE()->pack.free_item(font_data);
    ENGINE_INTERFACE()->pack.free_item(cat_data);

    auto GUI_FONT_STASH = []() -> ui_font_stash_desc_t * {
        static ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &ENGINE_INTERFACE()->font}};
        static ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(ui_font_desc_t)};
        return &font_stash;
    }();

    ui_init_font_stash(&ENGINE_INTERFACE()->ui, GUI_FONT_STASH);

    ui_dock_ex(&ENGINE_INTERFACE()->ui, "Style_Editor", "Demo_Window", UI_SPLIT_TAB, 0.5f);

    neko_game()->imgui = neko_imgui_new(&ENGINE_INTERFACE()->cb, neko_os_main_window(), false);

    // Construct frame buffer
    neko_game()->r_main_fbo = gfx_framebuffer_create({});

    gfx_texture_desc_t main_rt_desc = {.width = (u32)neko_game()->DisplaySize.x,
                                               .height = (u32)neko_game()->DisplaySize.y,
                                               .format = R_TEXTURE_FORMAT_RGBA8,
                                               .wrap_s = GL_REPEAT,
                                               .wrap_t = GL_REPEAT,
                                               .min_filter = R_TEXTURE_FILTER_NEAREST,
                                               .mag_filter = R_TEXTURE_FILTER_NEAREST};
    neko_game()->r_main_rt = gfx_texture_create(main_rt_desc);

    gfx_renderpass_desc_t main_rp_desc = {.fbo = neko_game()->r_main_fbo, .color = &neko_game()->r_main_rt, .color_size = sizeof(neko_game()->r_main_rt)};
    neko_game()->r_main_rp = gfx_renderpass_create(main_rp_desc);

    luax_get(ENGINE_LUA(), "neko", "game_init");
    luax_pcall(ENGINE_LUA(), 0, 0);

    neko::timer timer;
    timer.start();
    luax_get(ENGINE_LUA(), "neko", "game_init_thread");
    luax_pcall(ENGINE_LUA(), 0, 0);
    timer.stop();
    console_log(std::format("game_init_thread loading done in {0:.3f} ms", timer.get()).c_str());
}

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

#endif

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

static void init() {
    PROFILE_FUNC();
    // LockGuard lock(&g_init_mtx);

    neko::timer tm_init;
    tm_init.start();

    // ui_init();

    // renderer_reset();

    {
        PROFILE_BLOCK("neko.start");

        lua_State *L = g_app->L;

        if (!g_app->error_mode.load()) {
            luax_neko_get(L, "start");

            Slice<String> args = g_app->args;
            lua_createtable(L, args.len - 1, 0);
            for (u64 i = 1; i < args.len; i++) {
                lua_pushlstring(L, args[i].data, args[i].len);
                lua_rawseti(L, -2, i);
            }

            luax_pcall(L, 1, 0);
        }
    }

    g_app->gpu_mtx.lock();

    tm_init.stop();

    console_log("end of init in %.3f ms", tm_init.get());
}

#if 0

static void render() {
    PROFILE_FUNC();

    {
        PROFILE_BLOCK("begin render pass");

        sg_pass_action pass = {};
        pass.colors[0].load_action = SG_LOADACTION_CLEAR;
        pass.colors[0].store_action = SG_STOREACTION_STORE;
        if (g_app->error_mode.load()) {
            pass.colors[0].clear_value = {0.0f, 0.0f, 0.0f, 1.0f};
        } else {
            float rgba[4];
            renderer_get_clear_color(rgba);
            pass.colors[0].clear_value.r = rgba[0];
            pass.colors[0].clear_value.g = rgba[1];
            pass.colors[0].clear_value.b = rgba[2];
            pass.colors[0].clear_value.a = rgba[3];
        }

        {
            LockGuard lock{&g_app->gpu_mtx};
            // sg_begin_default_pass(pass, sapp_width(), sapp_height());
            sg_pass ps = {.action = {.colors = {{.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.f, 0.f, 0.f, 1.f}}}}, .swapchain = sglue_swapchain()};
            sg_begin_pass(&ps);
        }

        sgl_defaults();
        sgl_load_pipeline(g_pipeline);

        sgl_viewport(0, 0, sapp_width(), sapp_height(), true);
        sgl_ortho(0, sapp_widthf(), sapp_heightf(), 0, -1, 1);
    }

    if (g_app->error_mode.load()) {
        if (g_app->default_font == nullptr) {
            g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
            g_app->default_font->load_default();
        }

        renderer_reset();

        float x = 10;
        float y = 25;
        u64 font_size = 28;

        if (LockGuard lock{&g_app->error_mtx}) {
            y = draw_font(g_app->default_font, font_size, x, y, "-- ! Neko Error ! --");
            y += font_size;

            y = draw_font_wrapped(g_app->default_font, font_size, x, y, g_app->fatal_error, sapp_widthf() - x);
            y += font_size;

            if (g_app->traceback.data) {
                draw_font(g_app->default_font, font_size, x, y, g_app->traceback);
            }
        }
    } else {
        const int width = sapp_width();
        const int height = sapp_height();
        simgui_new_frame({width, height, sapp_frame_duration(), sapp_dpi_scale()});

        ui_begin();

        lua_State *L = g_app->L;



        {
            PROFILE_BLOCK("neko.frame");

            luax_neko_get(L, "frame");
            lua_pushnumber(L, timing_instance.delta);
            luax_pcall(L, 1, 0);
        }

        assert(lua_gettop(L) == 1);

        ui_end_and_present();


        sgimgui_draw(&sgimgui);
    }

    {
        PROFILE_BLOCK("gp render pass");
        int width = sapp_width(), height = sapp_height();
        float ratio = width / (float)height;

        sgp_begin(width, height);
        sgp_viewport(0, 0, width, height);

        // 绘图坐标空间
        sgp_project(-ratio, ratio, 1.0f, -1.0f);

        // 清除帧缓冲区
        sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        sgp_clear();

#if 0
        // 绘制一个可以旋转并改变颜色的动画矩形
        float time = sapp_frame_count() * sapp_frame_duration();
        float r = sinf(time) * 0.5 + 0.5, g = cosf(time) * 0.5 + 0.5;
        sgp_set_color(r, g, 0.3f, 1.0f);
        sgp_rotate_at(time, 0.0f, 0.0f);
        sgp_draw_filled_rect(-0.5f, -0.5f, 1.0f, 1.0f);
#endif
    }

    {
        PROFILE_BLOCK("end render pass");
        LockGuard lock{&g_app->gpu_mtx};

        // 将所有绘制命令分派至 Sokol GFX
        sgp_flush();
        // 完成绘制命令队列
        sgp_end();

        sgl_draw();

        sgl_error_t sgl_err = sgl_error();
        if (sgl_err != SGL_NO_ERROR) {
            neko_panic("a draw error occurred: %d", sgl_err);
        }

        if (!g_app->error_mode.load()) {
            simgui_render();
        }

        sg_end_pass();
        sg_commit();
    }
}

static void frame() {
    PROFILE_FUNC();


    g_app->gpu_mtx.unlock();
    render();
    assets_perform_hot_reload_changes();
    g_app->gpu_mtx.lock();

    memcpy(g_app->prev_key_state, g_app->key_state, sizeof(g_app->key_state));
    memcpy(g_app->prev_mouse_state, g_app->mouse_state, sizeof(g_app->mouse_state));
    g_app->prev_mouse_x = g_app->mouse_x;
    g_app->prev_mouse_y = g_app->mouse_y;
    g_app->scroll_x = 0;
    g_app->scroll_y = 0;

    Array<Sound *> &sounds = g_app->garbage_sounds;
    for (u64 i = 0; i < sounds.len;) {
        Sound *sound = sounds[i];

        if (sound->dead_end) {
            assert(sound->zombie);
            sound->trash();
            mem_free(sound);

            sounds[i] = sounds[sounds.len - 1];
            sounds.len--;
        } else {
            i++;
        }
    }
}

static void actually_cleanup() {
    PROFILE_FUNC();

    g_app->gpu_mtx.unlock();



    lua_State *L = g_app->L;

    {
        PROFILE_BLOCK("before quit");

        luax_neko_get(L, "before_quit");
        if (luax_pcall(L, 0, 0) != LUA_OK) {
            String err = luax_check_string(L, -1);
            neko_panic("%s", err.data);
        }
    }

    ui_trash();

    {
        PROFILE_BLOCK("lua close");

        lua_pop(L, 1);  // luax_msgh

        neko::neko_lua_fini(L);
        // lua_close(L);
        // luaalloc_delete(g_app->LA);
    }



    {
        PROFILE_BLOCK("audio uninit");
        ma_engine_uninit(&g_app->audio_engine);
        mem_free(g_app->miniaudio_vfs);
    }

    {
        PROFILE_BLOCK("destory sokol");
        sgimgui_discard(&sgimgui);
        simgui_shutdown();

        sgp_shutdown();

        sgl_destroy_pipeline(g_pipeline);
        sgl_shutdown();
        sg_shutdown();
    }




}

static void cleanup() {
    actually_cleanup();


}

#endif

// ECS_COMPONENT_DECL(pos_t);
// ECS_COMPONENT_DECL(vel_t);
// ECS_COMPONENT_DECL(rect_t);

static void neko_setup_w() {
    PROFILE_FUNC();

    // luax_neko_get(L, "__define_default_callbacks");
    // luax_pcall(L, 0, 0);
}

App *g_app;

Allocator *g_allocator;

int main(int argc, char **argv) {
    game_run(argc, argv);
    return 0;
}