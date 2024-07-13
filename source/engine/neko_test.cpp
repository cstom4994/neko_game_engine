

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
#include "engine/neko.h"
#include "engine/neko_api.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_engine.h"
#include "engine/neko_imgui.hpp"
#include "engine/neko_lua.h"
#include "engine/neko_luabind.hpp"
#include "engine/neko_math.h"
#include "engine/neko_platform.h"
#include "engine/neko_reflection.hpp"

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

NEKO_API_DECL void neko_console(neko_console_t *console, neko_ui_context_t *ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t *desc);

void draw_gui() {

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

        if (neko_game()->cvar.show_gui) {

            neko_ui_demo_window(&ENGINE_INTERFACE()->ui, neko_ui_rect(100, 100, 500, 500), NULL);
            neko_ui_style_editor(&ENGINE_INTERFACE()->ui, NULL, neko_ui_rect(350, 250, 300, 240), NULL);

            const neko_vec2 ws = neko_v2(600.f, 300.f);

            // const neko_ui_style_sheet_t *ss = &game_userdata->style_sheet;

            const neko_vec2 ss_ws = neko_v2(500.f, 300.f);
            neko_ui_window_begin(&ENGINE_INTERFACE()->ui, "Window", neko_ui_rect((neko_game()->DisplaySize.x - ss_ws.x) * 0.5f, (neko_game()->DisplaySize.y - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
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

            neko_ui_window_begin(&ENGINE_INTERFACE()->ui, "Idraw", neko_ui_rect((neko_game()->DisplaySize.x - ws.x) * 0.2f, (neko_game()->DisplaySize.y - ws.y) * 0.5f, ws.x, ws.y));
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

        if (neko_game()->cvar.show_gui) {

            neko_vec2 mp = neko_pf_mouse_positionv();
            neko_vec2 mw = neko_pf_mouse_wheelv();
            neko_vec2 md = neko_pf_mouse_deltav();
            bool lock = neko_pf_mouse_locked();
            bool moved = neko_pf_mouse_moved();

            if (neko_ui_window_begin(&ENGINE_INTERFACE()->ui, "App", neko_ui_rect(neko_game()->DisplaySize.x - 210, 30, 200, 200))) {
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

                    // neko_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                    // neko_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                    // neko_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                    lua_Integer kb = lua_gc(ENGINE_LUA(), LUA_GCCOUNT, 0);
                    lua_Integer bytes = lua_gc(ENGINE_LUA(), LUA_GCCOUNTB, 0);

                    neko_ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                    neko_ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                    // neko_ui_labelf("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

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

#pragma region test

using namespace neko;

struct PointInner {
    int xx, yy;
};

NEKO_STRUCT(PointInner, _Fs(xx, "inner_xxx"), _Fs(yy, "inner_yyy"));

struct Point {
    int x, y;
    PointInner inner;
};

NEKO_STRUCT(Point, _Fs(x, "xxx"), _Fs(y, "yyy"), _Fs(inner, "inner_shit"));

struct Rect {
    Point p1, p2;
    u32 color;
};

NEKO_STRUCT(Rect, _F(p1), _F(p2), _F(color));

template <typename T, typename Fields = std::tuple<>>
void dumpObj(T &&obj, int depth = 0, const char *fieldName = "", Fields &&fields = std::make_tuple()) {
    auto indent = [depth] {
        for (int i = 0; i < depth; ++i) {
            std::cout << "    ";
        }
    };

    if constexpr (std::is_class_v<std::decay_t<T>>) {
        indent();
        std::cout << fieldName << (*fieldName ? ": {" : "{") << std::endl;
        neko::reflection::struct_foreach(obj, [depth](auto &&fieldName, auto &&value, auto &&info) { dumpObj(value, depth + 1, fieldName, info); });
        indent();
        std::cout << "}" << (depth == 0 ? "" : ",") << std::endl;
    } else {
        indent();
        std::cout << fieldName << ": " << obj << ", " << std::get<0>(fields) << "," << std::endl;
    }
}

struct MyStruct {
    int i;
    float f;
};

template <class T>
void As(T arg) {
    std::cout << arg << std::endl;
}

static void test_struct() {

    MyStruct myStruct{1001, 2.f};
    neko::reflection::struct_apply(myStruct, [](auto &...args) { (..., As(args)); });

    Rect rect{
            {0, 0},
            {8, 9},
            12345678,
    };

    dumpObj(rect);

    std::tuple<int, double, std::string> myTuple;
    constexpr std::size_t size = neko::tuple_size_v<decltype(myTuple)>;

    std::cout << "Size of tuple: " << size << std::endl;
}

void test_se() { test_struct(); }

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        putc('\t', stdout);
    }
}

void print_xml_node(neko_xml_node_t *node, int indent) {
    print_indent(indent);
    printf("XML Node: %s\n", node->name);
    print_indent(indent);
    printf("\tText: %s\n", node->text);
    print_indent(indent);
    puts("\tAttributes:");
    for (neko_hash_table_iter it = neko_hash_table_iter_new(node->attributes); neko_hash_table_iter_valid(node->attributes, it); neko_hash_table_iter_advance(node->attributes, it)) {
        neko_xml_attribute_t attrib = neko_hash_table_iter_get(node->attributes, it);

        print_indent(indent);
        printf("\t\t%s: ", attrib.name);
        switch (attrib.type) {
            case NEKO_XML_ATTRIBUTE_NUMBER:
                printf("(number) %g\n", attrib.value.number);
                break;
            case NEKO_XML_ATTRIBUTE_BOOLEAN:
                printf("(boolean) %s\n", attrib.value.boolean ? "true" : "false");
                break;
            case NEKO_XML_ATTRIBUTE_STRING:
                printf("(string) %s\n", attrib.value.string);
                break;
            default:
                break;  // Unreachable
        }
    }

    if (neko_dyn_array_size(node->children) > 0) {
        print_indent(indent);
        printf("\t = Children = \n");
        for (uint32_t i = 0; i < neko_dyn_array_size(node->children); i++) {
            print_xml_node(node->children + i, indent + 1);
        }
    }
}

void test_xml(const std::string &file) {

    neko_xml_document_t *doc = neko_xml_parse_file(file.c_str());
    if (!doc) {
        printf("XML Parse Error: %s\n", neko_xml_get_error());
    } else {
        for (uint32_t i = 0; i < neko_dyn_array_size(doc->nodes); i++) {
            neko_xml_node_t *node = doc->nodes + i;
            print_xml_node(node, 0);
        }
        neko_xml_free(doc);
    }
}

typedef struct custom_key_t {
    uint32_t uval;
    float fval;
} custom_key_t;

neko_dyn_array(uint32_t) arr = NULL;
neko_hash_table(float, uint32_t) ht = NULL;
neko_hash_table(custom_key_t, uint32_t) htc = NULL;
neko_slot_array(double) sa = NULL;
neko_slot_map(uint64_t, uint32_t) sm = NULL;
neko_byte_buffer_t bb = {0};

#define ITER_CT 5

// Keys for slot map
const char *smkeys[ITER_CT] = {"John", "Dick", "Harry", "Donald", "Wayne"};

void test_containers() {

    NEKO_INVOKE_ONCE([] {
        bb = neko_byte_buffer_new();

        neko_byte_buffer_write(&bb, uint32_t, ITER_CT);

        for (uint32_t i = 0; i < ITER_CT; ++i) {
            neko_dyn_array_push(arr, i);

            neko_hash_table_insert(ht, (float)i, i);

            custom_key_t k = {.uval = i, .fval = (float)i * 2.f};
            neko_hash_table_insert(htc, k, i * 2);

            neko_slot_array_insert(sa, (double)i * 3.f);

            neko_slot_map_insert(sm, neko_hash_str64(smkeys[i]), i);

            neko_byte_buffer_write(&bb, uint32_t, i);
        }

        neko_byte_buffer_seek_to_beg(&bb);
    }(););

    neko_printf("neko_dyn_array: [");
    for (uint32_t i = 0; i < neko_dyn_array_size(arr); ++i) {
        neko_printf("%zu, ", arr[i]);
    }
    neko_println("]");

    neko_println("neko_hash_table: [");
    for (neko_hash_table_iter it = neko_hash_table_iter_new(ht); neko_hash_table_iter_valid(ht, it); neko_hash_table_iter_advance(ht, it)) {
        float k = neko_hash_table_iter_getk(ht, it);
        uint32_t v = neko_hash_table_iter_get(ht, it);
        neko_println("  {k: %.2f, v: %zu},", k, v);
    }
    neko_println("]");

    neko_println("neko_hash_table: [");
    for (neko_hash_table_iter it = neko_hash_table_iter_new(htc); neko_hash_table_iter_valid(htc, it); neko_hash_table_iter_advance(htc, it)) {
        custom_key_t *kp = neko_hash_table_iter_getkp(htc, it);
        uint32_t v = neko_hash_table_iter_get(htc, it);
        neko_println("  {k: {%zu, %.2f}, v: %zu},", kp->uval, kp->fval, v);
    }
    neko_println("]");

    neko_println("neko_slot_array: [");
    for (neko_slot_array_iter it = neko_slot_array_iter_new(sa); neko_slot_array_iter_valid(sa, it); neko_slot_array_iter_advance(sa, it)) {
        double v = neko_slot_array_iter_get(sa, it);
        neko_println("  id: %zu, v: %.2f", it, v);
    }
    neko_println("]");

    neko_println("neko_slot_map (manual): [");
    for (uint32_t i = 0; i < ITER_CT; ++i) {
        uint32_t v = neko_slot_map_get(sm, neko_hash_str64(smkeys[i]));
        neko_println("k: %s, h: %lu, v: %zu", smkeys[i], neko_hash_str64(smkeys[i]), v);
    }
    neko_println("]");

    neko_println("neko_slot_map (iterator): [");
    for (neko_slot_map_iter it = neko_slot_map_iter_new(sm); neko_slot_map_iter_valid(sm, it); neko_slot_map_iter_advance(sm, it)) {
        uint64_t k = neko_slot_map_iter_getk(sm, it);
        uint32_t v = neko_slot_map_iter_get(sm, it);
        neko_println("k: %lu, v: %zu", k, v);
    }
    neko_println("]");

    neko_println("neko_byte_buffer_t: [");

    neko_byte_buffer_readc(&bb, uint32_t, ct);

    for (uint32_t i = 0; i < ct; ++i) {
        neko_byte_buffer_readc(&bb, uint32_t, v);
        neko_println("v: %zu", v);
    }
    neko_println("]");

    neko_byte_buffer_seek_to_beg(&bb);
}

#pragma endregion test

#if 0

void print(const float &val) { std::printf("%f", val); }
void print(const std::string &val) { std::printf("%s", val.c_str()); }

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