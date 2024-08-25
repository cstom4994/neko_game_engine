
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
#include "engine/edit.h"
#include "engine/entity.h"
#include "engine/graphics.h"
#include "engine/luax.hpp"
#include "engine/scripting.h"
#include "engine/ui.h"

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

static bool quit = false;  // 如果为 true 则退出主循环
static int sargc = 0;
static char **sargv;
static Mutex g_init_mtx;

gfx_texture_t test_ase;

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

static void _opengl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

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
    glewInit();
    glGetError();  // see http://www.opengl.org/wiki/OpenGL_Loading_Library

#if defined(_DEBUG)
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
    timing_instance.startup = stm_now();
    timing_instance.last = stm_now();

    // init systems
    console_puts("welcome to neko!");
    system_init();

    g_app->cb = command_buffer_new();
    // g_app->idraw = neko_immediate_draw_new();

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

        // ui_init(&g_app->ui, 0);

        // 加载自定义字体文件 初始化 gui font stash
        // neko_asset_font_load_from_memory(font_data, font_data_size, &CL_GAME_INTERFACE()->font, 24);
        // neko_asset_font_load_from_file("gamedir/assets/fonts/monocraft.ttf", &font, 24);

        // pack.free_item(font_data);
        // pack.free_item(cat_data);

        // auto GUI_FONT_STASH = []() -> engine_ui_font_stash_desc_t * {
        //     static engine_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &font}};
        //     static engine_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(engine_ui_font_desc_t)};
        //     return &font_stash;
        // }();

        // engine_ui_init_font_stash(&g_app->ui, GUI_FONT_STASH);

        // ui_dock_ex(&g_app->ui, "Style_Editor", "Demo_Window", UI_SPLIT_TAB, 0.5f);

        g_app->ui = (ui_context_t *)mem_alloc(sizeof(ui_context_t));
        ui_init(g_app->ui);

        g_app->ui->style->colors[UI_COLOR_WINDOWBG] = color256(50, 50, 50, 200);

        g_app->ui->text_width = neko_ui_text_width;
        g_app->ui->text_height = neko_ui_text_height;
        neko_init_ui_renderer(gfx_create_program("ui_glsl", "shader/ui.vert", NULL, "shader/ui.frag"));
    }
}

static void _game_fini() {
    PROFILE_FUNC();

    {  // just for test

        mem_free(g_app->ui);
        neko_deinit_ui_renderer();
    }

    if (g_app->lite_init_path.len) {
        lt_fini();
        neko::neko_lua_fini(g_app->lite_L);
    }

    // neko_immediate_draw_static_data_free();
    // neko_immediate_draw_free(&g_app->idraw);
    command_buffer_free(&g_app->cb);

    // fini systems
    system_fini();

    // fini glfw
    glfwDestroyWindow(g_app->game_window);
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

        {

            // if (g_app->lite_init_path.len && g_app->lite_L) {
            //     if (ImGui::Begin("Lite")) {
            //         ImGuiWindow *window = ImGui::GetCurrentWindow();
            //         ImVec2 bounds = ImGui::GetContentRegionAvail();
            //         vec2 mouse_pos = input_get_mouse_pos_pixels_fix();  // 窗口内鼠标坐标
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

        script_draw_all();

        // Set up 2D camera for projection matrix
        // idraw_defaults(&g_app->idraw);
        // idraw_camera2d(&g_app->idraw, (u32)g_app->width, (u32)g_app->height);

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
            // gfx_set_viewport(&g_app->cb, 0, 0, (u32)g_app->width, (u32)g_app->height);
            // idraw_draw(&g_app->idraw, &g_app->cb);  // 立即模式绘制 idraw

            // f32 fy = draw_font(g_app->default_font, 16.f, 0.f, 0.f, "Hello World 测试中文，你好世界", NEKO_COLOR_WHITE);
            // fy = draw_font(g_app->default_font, 16.f, 0.f, 20.f, "我是第二行", NEKO_COLOR_WHITE);

            system_draw_all(NULL);

            // gfx_draw_func(&g_app->cb, system_draw_all);

            // ui_render(&g_app->ui, &g_app->cb);
        }

        auto ui = g_app->ui;

        neko_update_ui(ui);

        DeferLoop(ui_begin(ui), ui_end(ui)) {
            script_draw_ui();
            draw_gui();

            if (ui_begin_window(ui, "Hello World", neko_rect(10, 420, 400, 300))) {
                ui_label(ui, "Name");

                ui_end_window(ui);
            }
        }

        neko_render_ui(ui, g_app->width, g_app->height);

#else
        system_draw_all();
#endif

        if (g_app->lite_init_path.len && g_app->lite_L) lt_tick(g_app->lite_L);
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