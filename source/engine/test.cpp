#include "engine/test.h"

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
#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/base/os.hpp"
#include "engine/base/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/reflection.hpp"
#include "engine/scripting/scripting.h"
#include "engine/ui.h"

#pragma region test

int test_xml(lua_State *L);

namespace Neko::luabind::__unittest {

#if 0

// System IDs
ecs_id_t System1;
ecs_id_t System2;
ecs_id_t System3;

// Register components
void register_components(ecs_t* ecs) {
    // PosComp = ecs_register_component(ecs, sizeof(pos_t), NULL, NULL);
    // VelComp = ecs_register_component(ecs, sizeof(vel_t), NULL, NULL);
    // RectComp = ecs_register_component(ecs, sizeof(rect_t), NULL, NULL);
}

// System that prints the entity IDs of entities associated with this system
ecs_ret_t system_update(ecs_t* ecs, ecs_id_t* entities, int entity_count, ecs_dt_t dt, void* udata) {
    (void)ecs;
    (void)dt;
    (void)udata;

    for (int id = 0; id < entity_count; id++) {
        printf("%u ", entities[id]);
    }

    printf("\n");

    return 0;
}

void register_systems(ecs_t* ecs) {
    System1 = ecs_register_system(ecs, system_update, NULL, NULL, NULL);
    System2 = ecs_register_system(ecs, system_update, NULL, NULL, NULL);
    System3 = ecs_register_system(ecs, system_update, NULL, NULL, NULL);

    ecs_require_component(ecs, System1, ECS_COMPONENT_ID(pos_t));

    ecs_require_component(ecs, System2, ECS_COMPONENT_ID(pos_t));
    ecs_require_component(ecs, System2, ECS_COMPONENT_ID(vel_t));

    ecs_require_component(ecs, System3, ECS_COMPONENT_ID(pos_t));
    ecs_require_component(ecs, System3, ECS_COMPONENT_ID(vel_t));
    ecs_require_component(ecs, System3, ECS_COMPONENT_ID(rect_t));
}

int TestEcs(lua_State* L) {
    // Creates concrete ECS instance
    // ecs_t* ecs = ecs_new(1024, NULL);
    ecs_t* ecs = ENGINE_ECS();

    // Register components and systems
    register_components(ecs);
    register_systems(ecs);

    // Create three entities
    ecs_id_t e1 = ecs_create(ecs);
    ecs_id_t e2 = ecs_create(ecs);
    ecs_id_t e3 = ecs_create(ecs);

    // Add components to entities
    printf("---------------------------------------------------------------\n");
    printf("Created entities: %u, %u, %u\n", e1, e2, e3);
    printf("---------------------------------------------------------------\n");

    printf("PosComp added to: %u\n", e1);
    ecs_add(ecs, e1, ECS_COMPONENT_ID(pos_t), NULL);

    printf("---------------------------------------------------------------\n");
    printf("PosComp added to: %u\n", e2);
    printf("VeloComp added to: %u\n", e2);

    ecs_add(ecs, e2, ECS_COMPONENT_ID(pos_t), NULL);
    ecs_add(ecs, e2, ECS_COMPONENT_ID(vel_t), NULL);

    printf("---------------------------------------------------------------\n");
    printf("PosComp added to: %u\n", e3);
    printf("VeloComp added to: %u\n", e3);
    printf("RectComp added to: %u\n", e3);

    ecs_add(ecs, e3, ECS_COMPONENT_ID(pos_t), NULL);
    ecs_add(ecs, e3, ECS_COMPONENT_ID(vel_t), NULL);
    ecs_add(ecs, e3, ECS_COMPONENT_ID(rect_t), NULL);

    printf("---------------------------------------------------------------\n");

    // Manually execute the systems
    printf("Executing system 1\n");
    ecs_update_system(ecs, System1, 0.0f);  // Output: e1 e2 e3

    printf("Executing system 2\n");
    ecs_update_system(ecs, System2, 0.0f);  // Output: e2 e3

    printf("Executing system 3\n");
    ecs_update_system(ecs, System3, 0.0f);  // Output: e3

    printf("---------------------------------------------------------------\n");

    // ecs_free(ecs);

    lua_pushboolean(L, 1);

    return 1;
}

#endif

int luaopen(lua_State *L) {

    // luaL_Reg lib[] = {
    //         {"LUASTRUCT_test_vec4", LUASTRUCT_test_vec4},
    //         {"TestAssetKind_1",
    //          +[](lua_State *L) {
    //              AssetKind type_val;
    //              neko_luabind_to(L, AssetKind, &type_val, 1);
    //              neko_printf("%d", type_val);
    //              lua_pushinteger(L, type_val);
    //              return 1;
    //          }},

    //         {"TestAssetKind_2",
    //          +[](lua_State *L) {
    //              AssetKind type_val = (AssetKind)lua_tointeger(L, 1);
    //              neko_luabind_push(L, AssetKind, &type_val);
    //              return 1;
    //          }},
    //         {"TestBinding_1", TestBinding_1},
    //         {"test_xml", test_xml},
    //         {NULL, NULL},
    // };
    // luaL_newlibtable(L, lib);
    // luaL_setfuncs(L, lib, 0);
    return 0;
}
}  // namespace Neko::luabind::__unittest

struct PointInner {
    int xx, yy;
};

NEKO_STRUCT(PointInner, _Fs(xx, "inner_xxx"), _Fs(yy, "inner_yyy"));

struct MyPoint {
    int x, y;
    PointInner inner;
};

NEKO_STRUCT(MyPoint, _Fs(x, "xxx"), _Fs(y, "yyy"), _Fs(inner, "inner_shit"));

struct MyRect {
    MyPoint p1, p2;
    u32 color;
};

NEKO_STRUCT(MyRect, _F(p1), _F(p2), _F(color));

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
        Neko::reflection::struct_foreach(obj, [depth](auto &&fieldName, auto &&value, auto &&info) { dumpObj(value, depth + 1, fieldName, info); });
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
    Neko::reflection::struct_apply(myStruct, [](auto &...args) { (..., As(args)); });

    MyRect rect{
            {0, 0},
            {8, 9},
            12345678,
    };

    dumpObj(rect);

    std::tuple<int, double, std::string> myTuple;
    constexpr std::size_t size = Neko::tuple_size_v<decltype(myTuple)>;

    std::cout << "Size of tuple: " << size << std::endl;
}

void test_se() { test_struct(); }

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        putc('\t', stdout);
    }
}

void print_xml_node(XMLNode *node, int indent) {
    print_indent(indent);
    printf("XML Node: %s\n", node->name.data);
    print_indent(indent);
    printf("\tText: %s\n", node->text.data);
    print_indent(indent);
    puts("\tAttributes:");
    for (auto kv : node->attributes) {
        XMLAttribute *attrib = kv.value;

        print_indent(indent);
        printf("\t\t%s: ", attrib->name.data);
        switch (attrib->type) {
            case XMLAttribute::NEKO_XML_ATTRIBUTE_NUMBER:
                printf("(number) %g\n", std::get<double>(attrib->value));
                break;
            case XMLAttribute::NEKO_XML_ATTRIBUTE_BOOLEAN:
                printf("(boolean) %s\n", std::get<bool>(attrib->value) ? "true" : "false");
                break;
            case XMLAttribute::NEKO_XML_ATTRIBUTE_STRING:
                printf("(string) %s\n", std::get<String>(attrib->value).data);
                break;
            default:
                break;  // Unreachable
        }
    }

    if (node->children.len > 0) {
        print_indent(indent);
        printf("\t = Children = \n");
        for (uint32_t i = 0; i < node->children.len; i++) {
            print_xml_node(&node->children[i], indent + 1);
        }
    }
}

int test_xml(lua_State *L) {
    XMLDoc doc;
    doc.ParseVFS("assets/maps/tileset.tsx");
    if (!doc.nodes.len) {
        printf("XML Parse Error\n");
    } else {
        for (uint32_t i = 0; i < doc.nodes.len; i++) {
            XMLNode *node = &doc.nodes[i];
            print_xml_node(node, 0);
        }
        doc.Trash();
    }
    return 0;
}

#pragma endregion test

#if 1

// void editor_dockspace(ui_context_t* ctx) {
//     u64 opt = UI_OPT_NOCLIP | UI_OPT_NOFRAME | UI_OPT_FORCESETRECT | UI_OPT_NOTITLE | UI_OPT_DOCKSPACE | UI_OPT_FULLSCREEN | UI_OPT_NOMOVE | UI_OPT_NOBRINGTOFRONT | UI_OPT_NOFOCUS |
//     UI_OPT_NORESIZE; ui_window_begin_ex(ctx, "Dockspace", ui_rect(350, 40, 600, 500), NULL, NULL, opt);
//     {
//         // Editor dockspace
//     }
//     ui_window_end(ctx);
// }

///////////////////////////////////////////////
//
//  测试UI

i32 button_custom(ui_context_t *ctx, const char *label) {
    // Do original button call
    i32 res = ui_button(ctx, label);

    // Draw inner shadows/highlights over button
    Color256 hc = NEKO_COLOR_WHITE, sc = color256(85, 85, 85, 255);
    rect_t r = ctx->last_rect;
    i32 w = 2;
    ui_draw_rect(ctx, neko_rect(r.x + w, r.y, r.w - 2 * w, w), hc);
    ui_draw_rect(ctx, neko_rect(r.x + w, r.y + r.h - w, r.w - 2 * w, w), sc);
    ui_draw_rect(ctx, neko_rect(r.x, r.y, w, r.h), hc);
    ui_draw_rect(ctx, neko_rect(r.x + r.w - w, r.y, w, r.h), sc);

    return res;
}

// void neko_console(neko_console_t* console, ui_context_t* ctx, rect_t screen, const ui_selector_desc_t* desc);

void draw_gui_auto_test();

void draw_gui() {

    PROFILE_FUNC();

    const f64 t = timing_get_elapsed();

    {
        // editor_dockspace(g_app->ui);

#if 0
        if (1) {

            ui_demo_window(g_app->ui, ui_rect(100, 100, 500, 500), NULL);
            ui_style_editor(g_app->ui, NULL, ui_rect(350, 250, 300, 240), NULL);

            const vec2 ws = neko_v2(600.f, 300.f);

            // const ui_style_sheet_t *ss = &game_userdata->style_sheet;

            const vec2 ss_ws = neko_v2(500.f, 300.f);
            ui_window_begin(g_app->ui, "Window", ui_rect((g_app->width - ss_ws.x) * 0.5f, (g_app->height - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
            {
                // Cache the current container
                ui_container_t* cnt = ui_get_current_container(g_app->ui);

                ui_layout_row(g_app->ui, 2, ui_widths(200, 0), 0);

                ui_text(g_app->ui, "A regular element button.");
                ui_button(g_app->ui, "button");

                ui_text(g_app->ui, "A regular element label.");
                ui_label(g_app->ui, "label");

                ui_text(g_app->ui, "Button with classes: {.c0 .btn}");

                ui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                ui_button_ex(g_app->ui, "hello?##btn", &selector_1, 0x00);

                ui_text(g_app->ui, "Label with id #lbl and class .c0");
                ui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                ui_label_ex(g_app->ui, "label##lbl", &selector_2, 0x00);

                const f32 m = cnt->body.w * 0.3f;
                // ui_layout_row(gui, 2, (int[]){m, -m}, 0);
                // ui_layout_next(gui); // Empty space at beginning
                ui_layout_row(g_app->ui, 1, ui_widths(0), 0);
                ui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                if (ui_button_ex(g_app->ui, "reload style sheet", &selector_3, 0x00)) {
                    // app_load_style_sheet(true);
                }

                button_custom(g_app->ui, "Hello?");
            }
            ui_window_end(g_app->ui);

            ui_window_begin(g_app->ui, "Idraw", ui_rect((g_app->width - ws.x) * 0.2f, (g_app->height - ws.y) * 0.5f, ws.x, ws.y));
            {
                // Cache the current container
                ui_container_t* cnt = ui_get_current_container(g_app->ui);

                // 绘制到当前窗口中的转换对象的自定义回调
                // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                // 正如我们将在“分割”窗口示例中看到的那样。
                // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                // 你不需要任何东西
                Color256 color = color256_alpha(NEKO_COLOR_RED, (uint8_t)NEKO_CLAMP((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                ui_draw_custom(g_app->ui, cnt->body, gui_cb, &color, sizeof(color));
            }
            ui_window_end(g_app->ui);
        }
#endif

        if (1) {

            // vec2 mp = g_app->ui.mouse_pos;
            // vec2 mw = g_app->ui.scroll_delta;
            vec2 md = {};
            // bool lock = g_app->ui.mouse_pressed;
            // bool moved = neko_os_mouse_moved();

            if (ui_begin_window(g_app->ui, "App", neko_rect(g_app->cfg.width - 210, 30, 200, 200))) {
                // l = g_app->ui->layout_stack[0];
                ui_layout_row(g_app->ui, 1, ui_widths(-1), 0);

                static f32 delta, fps = NEKO_DEFAULT_VAL();
                delta = get_timing_instance()->true_dt;
                fps = 1.f / delta;

                Color256 col = NEKO_COLOR_GREEN;
                // ui_textf_colored(g_app->ui, &col, "Neko %d", neko_buildnum());

                ui_labelf("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(ENGINE_LUA(), LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024),
                          get_timing_instance()->true_dt * 1000.f, 1.f / get_timing_instance()->true_dt);

                ui_labelf("FPS: %.2lf Delta: %.6lf", fps, delta * 1000.f);

                ui_labelf("测试中文");

                // ui_layout_row(&game_userdata->core_ui, 1, ui_widths(-1), 0);

                // ui_labelf("Position: <%.2f %.2f>", mp.x, mp.y);
                // ui_labelf("Wheel: <%.2f %.2f>", mw.x, mw.y);
                ui_labelf("Delta: <%.2f %.2f>", md.x, md.y);
                // ui_labelf("Lock: %zu", lock);
                // ui_labelf("Moved: %zu", moved);
                // ui_labelf("Hover: %zu", g_gui.mouse_is_hover);
                ui_labelf("Time: %f", t);

                // std::vector test_vector = {"hahah", "233", "中文?"};

                // ui::Auto(test_vector, "test_vector");

#if 0
                struct {
                    const char* str;
                    i32 val;
                } btns[] = {{"Left", NEKO_MOUSE_LBUTTON}, {"Right", NEKO_MOUSE_RBUTTON}, {"Middle", NEKO_MOUSE_MBUTTON}, {NULL}};

                bool mouse_down[3] = {0};
                bool mouse_pressed[3] = {0};
                bool mouse_released[3] = {0};

                // Query mouse held down states.
                mouse_down[NEKO_MOUSE_LBUTTON] = neko_os_mouse_down(NEKO_MOUSE_LBUTTON);
                mouse_down[NEKO_MOUSE_RBUTTON] = neko_os_mouse_down(NEKO_MOUSE_RBUTTON);
                mouse_down[NEKO_MOUSE_MBUTTON] = neko_os_mouse_down(NEKO_MOUSE_MBUTTON);

                // Query mouse release states.
                mouse_released[NEKO_MOUSE_LBUTTON] = neko_os_mouse_released(NEKO_MOUSE_LBUTTON);
                mouse_released[NEKO_MOUSE_RBUTTON] = neko_os_mouse_released(NEKO_MOUSE_RBUTTON);
                mouse_released[NEKO_MOUSE_MBUTTON] = neko_os_mouse_released(NEKO_MOUSE_MBUTTON);

                // Query mouse pressed states. Press is a single frame click.
                mouse_pressed[NEKO_MOUSE_LBUTTON] = neko_os_mouse_pressed(NEKO_MOUSE_LBUTTON);
                mouse_pressed[NEKO_MOUSE_RBUTTON] = neko_os_mouse_pressed(NEKO_MOUSE_RBUTTON);
                mouse_pressed[NEKO_MOUSE_MBUTTON] = neko_os_mouse_pressed(NEKO_MOUSE_MBUTTON);

                ui_layout_row(g_app->ui, 7, ui_widths(100, 100, 32, 100, 32, 100, 32), 0);
                for (u32 i = 0; btns[i].str; ++i) {
                    ui_labelf("%s: ", btns[i].str);
                    ui_labelf("pressed: ");
                    ui_labelf("%d", mouse_pressed[btns[i].val]);
                    ui_labelf("down: ");
                    ui_labelf("%d", mouse_down[btns[i].val]);
                    ui_labelf("released: ");
                    ui_labelf("%d", mouse_released[btns[i].val]);
                }

#endif

                ui_layout_row(g_app->ui, 1, ui_widths(-1), 0);
                {
                    // static neko_memory_info_t meminfo = NEKO_DEFAULT_VAL();
                    // TimedAction(60, { meminfo = neko_os_memory_info(); });

                    // ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                    // ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                    // ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                    lua_Integer kb = lua_gc(ENGINE_LUA(), LUA_GCCOUNT, 0);
                    lua_Integer bytes = lua_gc(ENGINE_LUA(), LUA_GCCOUNTB, 0);

                    ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                    ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                    // eventhandler_t *eh = eventhandler_instance();
                    // ui_labelf("Event queue: %llu", eh->prev_len);

                    // ui_labelf("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

                    // ui_labelf("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                    // ui_labelf("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                    // ui_labelf("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                    // ui_labelf("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                    // ui_labelf("GPU MemTotalAvailable: %.2lf mb", (f64)(meminfo.gpu_total_memory / 1024.0f));
                    // ui_labelf("GPU MemCurrentUsage: %.2lf mb", (f64)(meminfo.gpu_memory_used / 1024.0f));

                    // gfx_info_t* info = &neko_subsystem(render)->info;

                    // ui_labelf("OpenGL vendor: %s", info->vendor);
                    // ui_labelf("OpenGL version supported: %s", info->version);
                }

                ui_end_window(g_app->ui);
            }
        }
    }

    if (input_key_down(KC_GRAVE_ACCENT)) {
        g_console.open = !g_console.open;
    } else if (input_key_down(KC_TAB) && g_console.open) {
        g_console.autoscroll = !g_console.autoscroll;
    }

    vec2 fb = Neko::the<Game>().get_window_size();
    rect_t screen = neko_rect(0, 0, fb.x, fb.y);
    // ui_Layout l;
    //            if (embeded)
    //                screen = l.body;
    //            else
    //                screen = ui_rect(0, 0, fb.x, fb.y);
    neko_console(&g_console, g_app->ui, &screen, NULL);

    // draw_gui_auto_test();
}

#if 1
void draw_gui_auto_test() {
    ui_context_t *ui = g_app->ui;
    const vec2 ss_ws = neko_v2(500.f, 300.f);
    ui_begin_window(g_app->ui, "GUI Test", neko_rect((g_app->cfg.width - ss_ws.x) * 0.5f, (g_app->cfg.height - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
    {

        if (ui_header(ui, "1. String")) {

            ui::Auto("Hello Imgui::Auto() !");  // This is how this text is written as well.

            static std::string str = "Hello ui::Auto() for strings!";
            ui::Auto(str, "str");

            static std::string str2 = "ui::Auto()\n Automatically uses multiline input for strings!\n:)";
            ui::Auto(str2, "str2");

            static const std::string conststr = "Const types are not to be changed!";
            ui::Auto(conststr, "conststr");

            char *buffer = "To edit a string use std::string. Manual buffers are unwelcome here.";
            ui::Auto(buffer, "buffer");
        }
        if (ui_header(ui, "2. Numbers")) {

            static int i = 42;
            ui::Auto(i, "i");

            static float f = 3.14;
            ui::Auto(f, "f");

            //         static ImVec4 f4 = {1.5f, 2.1f, 3.4f, 4.3f};
            //         ui::Auto(f4, "f4");

            //         static const ImVec2 f2 = {1.f, 2.f};
            //         ui::Auto(f2, "f2");
        }
        if (ui_header(ui, "3. Containers")) {

            static std::vector<std::string> vec = {"First string", "Second str", ":)"};
            ui::Auto(vec, "vec");

            static const std::vector<float> constvec = {3, 1, 2.1f, 4, 3, 4, 5};
            ui::Auto(constvec, "constvec");  // Cannot change vector, nor values

            static std::vector<bool> bvec = {false, true, false, false};
            ui::Auto(bvec, "bvec");

            static const std::vector<bool> constbvec = {false, true, false, false};
            ui::Auto(constbvec, "constbvec");

            //         static std::map<int, float> map = {{3, 2}, {1, 2}};
            //         ui::Auto(map, "map");  // insert and other operations

            //             static std::deque<bool> deque = {false, true, false, false};
            //             ui::Auto(deque, "deque");

            //             static std::set<char*> set = {"set", "with", "char*"};  // for some reason, this does not work
            //             ui::Auto(set, "set");                          // the problem is with the const iterator, but

            //             static std::map<char*, std::string> map = {{"asd", "somevalue"}, {"bsd", "value"}};
            //             ui::Auto(map, "map");  // insert and other operations
            //         }
        }
        if (ui_header(ui, "4. Pointers and Arrays")) {

            static float *pf = nullptr;
            ui::Auto(pf, "pf");

            static int i = 10, *pi = &i;
            ui::Auto(pi, "pi");

            static const std::string cs = "I cannot be changed!", *cps = &cs;
            ui::Auto(cps, "cps");

            static std::string str = "I can be changed! (my pointee cannot)";
            static std::string *const strpc = &str;
            ui::Auto(strpc, "strpc");

            //         static std::array<float, 5> farray = {1.2, 3.4, 5.6, 7.8, 9.0};
            //         ui::Auto(farray, "std::array");

            static float farr[5] = {11.2, 3.4, 5.6, 7.8, 911.0};
            ui::Auto(farr, "float[5]");
        }
        if (ui_header(ui, "5. Pairs and Tuples")) {

            //         static std::pair<bool, ImVec2> pair = {true, {2.1f, 3.2f}};
            //         ui::Auto(pair, "pair");

            static std::pair<int, std::string> pair2 = {-3, "simple types appear next to each other in a pair"};
            ui::Auto(pair2, "pair2");

            //         ui::Auto(Neko::cpp::as_const(pair), "as_const(pair)");  // easy way to view as const

            //         std::tuple<const int, std::string, ImVec2> tuple = {42, "string in tuple", {3.1f, 3.2f}};
            //         ui::Auto(tuple, "tuple");

            //         const std::tuple<int, const char*, ImVec2> consttuple = {42, "Smaller tuples are inlined", {3.1f, 3.2f}};
            //         ui::Auto(consttuple, "consttuple");
        }
        if (ui_header(ui, "6. Structs!!")) {

            struct A {
                int i = 216;
                bool b = true;
            };
            static A a;
            ui::Auto(a, "a");

            ui::Auto(Neko::cpp::as_const(a), "as_const(a)");

            struct B {
                std::string str = "Unfortunatelly, cannot deduce const-ness from within a struct";
                const A a = A();
            };
            static B b;
            ui::Auto(b, "b");

            static std::vector<B> vec = {{"vector of structs!", A()}, B()};
            ui::Auto(vec, "vec");

            struct C {
                std::list<B> vec;
                A *a;
            };
            static C c = {{{"Container inside a struct!", A()}}, &a};
            ui::Auto(c, "c");
        }
        if (ui_header(ui, "Functions")) {
            void (*func)() = []() {
                auto col = NEKO_COLOR_GREEN;
                // ui_text_colored(g_app->ui, "Button pressed, function called :)", &col);
            };
            ui::Auto(func, "void(void) function");
        }

        if (ui_header(ui, "ui_file_browser")) {
            static std::string path = "D:/Projects/Neko/DevNew/bin";
            ui_file_browser(path);
        }
    }
    ui_end_window(g_app->ui);
}
#endif

#endif

#define NEKO_PROP

#if !defined(NEKO_PROP)

#include <string_view>

//
// Type names
//

#ifndef __FUNCSIG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

template <typename T>
constexpr std::string_view getTypeName() {
    constexpr auto prefixLength = 36, suffixLength = 1;
    const_str data = __FUNCSIG__;
    auto end = data;
    while (*end) {
        ++end;
    }
    return {data + prefixLength, size_t(end - data - prefixLength - suffixLength)};
}

//
// Component types list
//

template <int N>
struct neko_prop_component_type_counter : neko_prop_component_type_counter<N - 1> {
    static constexpr auto num = N;
};
template <>
struct neko_prop_component_type_counter<0> {
    static constexpr auto num = 0;
};
neko_prop_component_type_counter<0> numComponentTypes(neko_prop_component_type_counter<0>);

template <int I>
struct neko_prop_component_typelist;
template <>
struct neko_prop_component_typelist<0> {
    static void each(auto &&f) {}
};

inline constexpr auto maxNumComponentTypes = 32;

template <typename T>
inline constexpr auto isComponentType = false;

#define ComponentTypeListAdd(T)                                                                                                                       \
    template <>                                                                                                                                       \
    inline constexpr auto isComponentType<T> = true;                                                                                                  \
    constexpr auto ComponentTypeList_##T##_Size = decltype(numComponentTypes(neko_prop_component_type_counter<maxNumComponentTypes>()))::num + 1;     \
    static_assert(ComponentTypeList_##T##_Size < maxNumComponentTypes);                                                                               \
    neko_prop_component_type_counter<ComponentTypeList_##T##_Size> numComponentTypes(neko_prop_component_type_counter<ComponentTypeList_##T##_Size>); \
    template <>                                                                                                                                       \
    struct neko_prop_component_typelist<ComponentTypeList_##T##_Size> {                                                                               \
        static void each(auto &&f) {                                                                                                                  \
            neko_prop_component_typelist<ComponentTypeList_##T##_Size - 1>::each(f);                                                                  \
            f.template operator()<T>();                                                                                                               \
        }                                                                                                                                             \
    }

#define Comp(T)              \
    T;                       \
    ComponentTypeListAdd(T); \
    struct T

#define UseComponentTypes()                                                                                                                                           \
    static void forEachComponentType(auto &&f) {                                                                                                                      \
        neko_prop_component_typelist<decltype(numComponentTypes(neko_prop_component_type_counter<maxNumComponentTypes>()))::num>::each(std::forward<decltype(f)>(f)); \
    }

//
// Props
//

constexpr u32 props_hash(std::string_view str) {
    constexpr u32 offset = 2166136261;
    constexpr u32 prime = 16777619;
    auto result = offset;
    for (auto c : str) {
        result = (result ^ c) * prime;
    }
    return result;
}

struct neko_prop_attribs {
    std::string_view name;
    u32 nameHash = props_hash(name);

    bool exampleFlag = false;
};

inline constexpr auto maxNumProps = 24;

template <int N>
struct neko_prop_counter : neko_prop_counter<N - 1> {
    static constexpr auto num = N;
};
template <>
struct neko_prop_counter<0> {
    static constexpr auto num = 0;
};
[[maybe_unused]] static inline neko_prop_counter<0> numProps(neko_prop_counter<0>);

template <int N>
struct neko_prop_index {
    static constexpr auto index = N;
};

template <typename T, int N>
struct neko_prop_tag_wrapper {
    struct tag {
        static inline neko_prop_attribs attribs = T::getPropAttribs(neko_prop_index<N>{});
    };
};
template <typename T, int N>
struct neko_prop_tag_wrapper<const T, N> {
    using tag = typename neko_prop_tag_wrapper<T, N>::tag;
};
template <typename T, int N>
using neko_prop_tag = typename neko_prop_tag_wrapper<T, N>::tag;

#define neko_prop(type, name_, ...) neko_prop_named(#name_, type, name_, __VA_ARGS__)
#define neko_prop_named(nameStr, type, name_, ...)                                                                                                                                             \
    using name_##_Index = neko_prop_index<decltype(numProps(neko_prop_counter<maxNumProps>()))::num>;                                                                                          \
    static inline neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1> numProps(neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1>); \
    static std::type_identity<PROP_PARENS_1(PROP_PARENS_3 type)> propType(name_##_Index);                                                                                                      \
    static constexpr neko_prop_attribs getPropAttribs(name_##_Index) { return {.name = #name_, __VA_ARGS__}; };                                                                                \
    std::type_identity_t<PROP_PARENS_1(PROP_PARENS_3 type)> name_

#define PROP_PARENS_1(...) PROP_PARENS_2(__VA_ARGS__)
#define PROP_PARENS_2(...) NO##__VA_ARGS__
#define PROP_PARENS_3(...) PROP_PARENS_3 __VA_ARGS__
#define NOPROP_PARENS_3

template <auto memPtr>
struct neko_prop_containing_type {};
template <typename C, typename R, R C::*memPtr>
struct neko_prop_containing_type<memPtr> {
    using Type = C;
};
#define neko_prop_tag(field) neko_prop_tag<neko_prop_containing_type<&field>::Type, field##_Index::index>

struct __neko_prop_any {
    template <typename T>
    operator T() const;  // NOLINT(google-explicit-constructor)
};

template <typename Aggregate, typename Base = std::index_sequence<>, typename = void>
struct __neko_prop_count_fields : Base {};
template <typename Aggregate, int... Indices>
struct __neko_prop_count_fields<Aggregate, std::index_sequence<Indices...>,
                                std::void_t<decltype(Aggregate{{(static_cast<void>(Indices), std::declval<__neko_prop_any>())}..., {std::declval<__neko_prop_any>()}})>>
    : __neko_prop_count_fields<Aggregate, std::index_sequence<Indices..., sizeof...(Indices)>> {};
template <typename T>
constexpr int countFields() {
    return __neko_prop_count_fields<std::remove_cvref_t<T>>().size();
}

template <typename T>
concept neko_props = std::is_aggregate_v<T>;

template <neko_props T, typename F>
inline void forEachProp(T &val, F &&func) {
    if constexpr (requires { forEachField(const_cast<std::remove_cvref_t<T> &>(val), func); }) {
        forEachField(const_cast<std::remove_cvref_t<T> &>(val), func);
    } else if constexpr (requires { T::propType(neko_prop_index<0>{}); }) {
        constexpr auto n = countFields<T>();
        const auto call = [&]<typename Index>(Index index, auto &val) {
            if constexpr (requires { T::propType(index); }) {
                static_assert(std::is_same_v<typename decltype(T::propType(index))::type, std::remove_cvref_t<decltype(val)>>);
                func(neko_prop_tag<T, Index::index>{}, val);
            }
        };
#define C(i) call(neko_prop_index<i>{}, f##i)
        if constexpr (n == 1) {
            auto &[f0] = val;
            (C(0));
        } else if constexpr (n == 2) {
            auto &[f0, f1] = val;
            (C(0), C(1));
        } else if constexpr (n == 3) {
            auto &[f0, f1, f2] = val;
            (C(0), C(1), C(2));
        } else if constexpr (n == 4) {
            auto &[f0, f1, f2, f3] = val;
            (C(0), C(1), C(2), C(3));
        } else if constexpr (n == 5) {
            auto &[f0, f1, f2, f3, f4] = val;
            (C(0), C(1), C(2), C(3), C(4));
        } else if constexpr (n == 6) {
            auto &[f0, f1, f2, f3, f4, f5] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5));
        } else if constexpr (n == 7) {
            auto &[f0, f1, f2, f3, f4, f5, f6] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6));
        } else if constexpr (n == 8) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7));
        } else if constexpr (n == 9) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8));
        } else if constexpr (n == 10) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9));
        } else if constexpr (n == 11) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10));
        } else if constexpr (n == 12) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11));
        } else if constexpr (n == 13) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12));
        } else if constexpr (n == 14) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13));
        } else if constexpr (n == 15) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14));
        } else if constexpr (n == 16) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15));
        } else if constexpr (n == 17) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16));
        } else if constexpr (n == 18) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17));
        } else if constexpr (n == 19) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18));
        } else if constexpr (n == 20) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19));
        } else if constexpr (n == 21) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20));
        } else if constexpr (n == 22) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21));
        } else if constexpr (n == 23) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21), C(22));
        } else if constexpr (n == 24) {
            auto &[f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21), C(22), C(23));
        }
#undef C
    }
}

#endif

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

#define NEKO_LUA_FS
#ifdef NEKO_LUA_FS

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "engine/base.hpp"

#if defined(NEKO_IS_WIN32)
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef NEKO_IS_WIN32
#define SEP "\\"
#define ALLSEPS "\\/"
#define FUNC_STAT _stati64
#define FUNC_LSTAT FUNC_STAT
#define STRUCT_STAT struct _stat64
#define getcwd(d, s) _getcwd(d, s)
#define rmdir(p) _rmdir(p)
#else
#define SEP "/"
#define ALLSEPS "/"
#define FUNC_STAT stat
#define FUNC_LSTAT lstat
#define STRUCT_STAT struct stat
#endif

#define DIR_ITR "CO_DIR_ITR"

typedef struct diritr {
    int closed;  // 是否已经关闭
#if defined(NEKO_IS_WIN32)
    intptr_t handle;
    char *path;
    int first_time;
#else
    DIR *dir;
#endif
} diritr_t;

static void _diritr_close(diritr_t *itr) {
    if (itr->closed) return;
#if defined(NEKO_IS_WIN32)
    if (itr->handle != -1L) _findclose(itr->handle);
    if (itr->path) mem_free(itr->path);
#else
    if (itr->dir) closedir(itr->dir);
#endif
    itr->closed = 1;
}

static int l_diritr_close(lua_State *L) {
    diritr_t *itr = (diritr_t *)luaL_checkudata(L, 1, DIR_ITR);
    _diritr_close(itr);
    return 0;
}

static int _diritr_next(lua_State *L) {
    diritr_t *itr = (diritr_t *)lua_touserdata(L, lua_upvalueindex(1));
    if (itr->closed) luaL_error(L, "Scan a closed directory");
#if defined(NEKO_IS_WIN32)
    struct _finddata_t finddata;
    while (1) {
        if (itr->first_time) {
            itr->first_time = 0;
            itr->handle = _findfirst(itr->path, &finddata);
            if (itr->handle == -1L) {
                _diritr_close(itr);
                if (errno != ENOENT) {
                    lua_pushnil(L);
                    lua_pushstring(L, strerror(errno));
                    return 2;
                } else {
                    return 0;
                }
            }
        } else {
            if (_findnext(itr->handle, &finddata) == -1L) {
                _diritr_close(itr);
                return 0;
            }
        }
        if (strcmp(finddata.name, ".") == 0 || strcmp(finddata.name, "..") == 0) continue;
        lua_pushstring(L, finddata.name);
        return 1;
    }
#else
    struct dirent *ent;
    while (1) {
        ent = readdir(itr->dir);
        if (ent == NULL) {
            closedir(itr->dir);
            itr->closed = 1;
            return 0;
        }
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        lua_pushstring(L, ent->d_name);
        return 1;
    }
#endif
}

static int l_scandir(lua_State *L) {
    size_t pathsz;
    const char *path = luaL_checklstring(L, 1, &pathsz);
    diritr_t *itr = (diritr_t *)lua_newuserdata(L, sizeof(diritr_t));
    memset(itr, 0, sizeof(*itr));
    luaL_setmetatable(L, DIR_ITR);
#if defined(NEKO_IS_WIN32)
    itr->handle = -1L;
    itr->path = (char *)mem_alloc(pathsz + 5);
    strncpy(itr->path, path, pathsz);
    char ch = itr->path[pathsz - 1];
    if (!strchr(ALLSEPS, ch) && ch != ':') itr->path[pathsz++] = SEP[0];
    itr->path[pathsz++] = L'*';
    itr->path[pathsz] = '\0';
    itr->first_time = 1;
#else
    DIR *dir = opendir(path);
    if (dir == NULL) luaL_error(L, "cannot open %s: %s", path, strerror(errno));
    itr->dir = dir;
#endif
    lua_pushcclosure(L, _diritr_next, 1);
    return 1;
}

static int l_exists(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) == 0)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

static int l_getsize(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) == 0) {
        lua_pushinteger(L, (lua_Integer)st.st_size);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "getsize error: %s", strerror(errno));
        return 2;
    }
}

///////////////////////////////////////////////////////////////////////
// 消除一些系统的差异
#ifdef NEKO_IS_WIN32
// 取文件时间
static int win_getfiletime(const char *path, FILETIME *ftCreate, FILETIME *ftAccess, FILETIME *ftWrite) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }
    if (!GetFileTime(hFile, ftCreate, ftAccess, ftWrite)) {
        CloseHandle(hFile);
        return -1;
    } else {
        CloseHandle(hFile);
        return 0;
    }
}

static __int64 secs_between_epochs = 11644473600; /* Seconds between 1.1.1601 and 1.1.1970 */
// 将文件时间转换成秒和纳秒
static void win_convert_filetime(FILETIME *time_in, time_t *time_out, long *nsec_out) {
    __int64 in = (int64_t)time_in->dwHighDateTime << 32 | time_in->dwLowDateTime;
    *nsec_out = (long)(in % 10000000) * 100; /* FILETIME is in units of 100 nsec. */
    *time_out = (time_t)((in / 10000000) - secs_between_epochs);
}
#elif __APPLE__
#define st_mtim st_atimespec
#define st_atim st_mtimespec
#define st_ctim st_ctimespec
#else
#endif

static int l_getmtime(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    time_t sec;
    long nsec;
#ifdef NEKO_IS_WIN32
    FILETIME ftWrite;
    if (win_getfiletime(path, NULL, NULL, &ftWrite) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %d", GetLastError());
        return 2;
    }
    win_convert_filetime(&ftWrite, &sec, &nsec);
#else
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %s", strerror(errno));
        return 2;
    }
    sec = st.st_mtim.tv_sec;
    nsec = st.st_mtim.tv_nsec;
#endif
    lua_pushnumber(L, (lua_Number)(sec + 1e-9 * nsec));
    return 1;
}

static int l_getatime(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    time_t sec;
    long nsec;
#ifdef NEKO_IS_WIN32
    FILETIME ftAccess;
    if (win_getfiletime(path, NULL, &ftAccess, NULL) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getatime error: %d", GetLastError());
        return 2;
    }
    win_convert_filetime(&ftAccess, &sec, &nsec);
#else
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %s", strerror(errno));
        return 2;
    }
    sec = st.st_atim.tv_sec;
    nsec = st.st_atim.tv_nsec;
#endif
    lua_pushnumber(L, (lua_Number)(sec + 1e-9 * nsec));
    return 1;
}

static int l_getctime(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    time_t sec;
    long nsec;
#ifdef NEKO_IS_WIN32
    FILETIME ftCreate;
    if (win_getfiletime(path, &ftCreate, NULL, NULL) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getatime error: %d", GetLastError());
        return 2;
    }
    win_convert_filetime(&ftCreate, &sec, &nsec);
#else
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) != 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "getmtime error: %s", strerror(errno));
        return 2;
    }
    sec = st.st_ctim.tv_sec;
    nsec = st.st_ctim.tv_nsec;
#endif
    lua_pushnumber(L, (lua_Number)(sec + 1e-9 * nsec));
    return 1;
}

static int l_getmode(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_STAT(path, &st) == 0) {
        lua_pushinteger(L, (lua_Integer)st.st_mode);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "getmode error: %s", strerror(errno));
        return 2;
    }
}

static int l_getlinkmode(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    STRUCT_STAT st;
    if (FUNC_LSTAT(path, &st) == 0) {
        lua_pushinteger(L, (lua_Integer)st.st_mode);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "getlinkmode error: %s", strerror(errno));
        return 2;
    }
}

static int l_mkdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int err;
#ifdef NEKO_IS_WIN32
    err = _mkdir(path);
#else
    err = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    if (err) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "mkdir error: %s", strerror(errno));
        return 2;
    } else {
        lua_pushboolean(L, 1);
        return 1;
    }
}

static int l_rmdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int err = rmdir(path);
    if (err) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "rmdir error: %s", strerror(errno));
        return 2;
    } else {
        lua_pushboolean(L, 1);
        return 1;
    }
}

static int l_chdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    if (neko_os_chdir(path)) {
        lua_pushnil(L);
        lua_pushfstring(L, "chdir error: %s", strerror(errno));
        return 2;
    } else {
        lua_pushboolean(L, 1);
        return 1;
    }
}

static int l_getcwd(lua_State *L) {
    char path[256];
    int size = 256;
    if (getcwd(path, size) != NULL) {
        lua_pushstring(L, path);
        return 1;
    }
    char *buff = NULL;
    int result;
    while (1) {
        size <<= 1;
        buff = (char *)mem_realloc(buff, size);
        if (buff == NULL) {
            lua_pushnil(L);
            lua_pushfstring(L, "getcwd error: realloc() failed");
            result = 2;
            break;
        }
        if (getcwd(buff, size) != NULL) {
            lua_pushstring(L, buff);
            result = 1;
            break;
        }
        if (errno != ERANGE) {
            lua_pushnil(L);
            lua_pushfstring(L, "getcwd error: %s", strerror(errno));
            result = 2;
            break;
        }
    }
    mem_free(buff);
    return result;
}

static const luaL_Reg dirmt[] = {{"__gc", l_diritr_close}, {NULL, NULL}};

static const luaL_Reg lib[] = {{"scandir", l_scandir},
                               {"exists", l_exists},
                               {"getsize", l_getsize},
                               {"getmtime", l_getmtime},
                               {"getatime", l_getatime},
                               {"getctime", l_getctime},
                               {"getmode", l_getmode},
                               {"getlinkmode", l_getlinkmode},
                               {"mkdir", l_mkdir},
                               {"rmdir", l_rmdir},
                               {"chdir", l_chdir},
                               {"getcwd", l_getcwd},
                               {NULL, NULL}};

static void init_consts(lua_State *L) {
    lua_pushliteral(L, SEP);
    lua_setfield(L, -2, "sep");
    lua_pushliteral(L, ALLSEPS);
    lua_setfield(L, -2, "allseps");
#ifdef NEKO_IS_WIN32
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "iswindows");
#endif
}

int open_filesys(lua_State *L) {
    luaL_checkversion(L);
    luaL_newmetatable(L, DIR_ITR);
    luaL_setfuncs(L, dirmt, 0);

    luaL_newlib(L, lib);
    init_consts(L);
    return 1;
}

#endif

#if 0

#include <stdlib.h>

#define CP_DATA_POINTER_TYPE NativeEntity
#include <chipmunk/chipmunk.h>


// per-entity info
typedef struct PhysicsInfo PhysicsInfo;
struct PhysicsInfo
{
    EntityPoolElem pool_elem;

    PhysicsBody type;

    // store mass separately to convert to/from PB_DYNAMIC
    Scalar mass;

    // used to compute (angular) velocitiy for PB_KINEMATIC
    cpVect last_pos;
    cpFloat last_ang;

    // used to keep track of transform <-> physics update
    unsigned int last_dirty_count;

    cpBody *body;
    CArray *shapes;
    CArray *collisions;
};

// per-shape info for each shape attached to a physics entity
typedef struct ShapeInfo ShapeInfo;
struct ShapeInfo
{
    PhysicsShape type;
    cpShape *shape;
};

static cpSpace *space;
static Scalar period = 1.0 / 60.0; // 1.0 / simulation_frequency
static NativeEntityPool *pool;

static NativeEntityMap *debug_draw_map;

// -------------------------------------------------------------------------

// chipmunk utilities

static inline cpVect cpv_of_vec2(vec2 v) { return cpv(v.x, v.y); }
static inline vec2 vec2_of_cpv(cpVect v) { return luavec2(v.x, v.y); }

static inline void _remove_body(cpBody *body)
{
    if (cpSpaceContainsBody(space, body))
        cpSpaceRemoveBody(space, body);
    cpBodyFree(body);
}
static inline void _remove_shape(cpShape *shape)
{
    if (cpSpaceContainsShape(space, shape))
        cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

// -------------------------------------------------------------------------

void physics_set_gravity(vec2 g)
{
    cpSpaceSetGravity(space, cpv_of_vec2(g));
}
vec2 physics_get_gravity()
{
    return vec2_of_cpv(cpSpaceGetGravity(space));
}

void physics_set_simulation_frequency(Scalar freq)
{
    period = 1.0 / freq;
}
Scalar physics_get_simulation_frequency()
{
    return 1.0 / period;
}

void physics_add(NativeEntity ent)
{
    PhysicsInfo *info;

    if (entitypool_get(pool, ent))
        return; // already has physics

    transform_add(ent);

    info = entitypool_add(pool, ent);

    info->mass = 1.0;
    info->type = PB_DYNAMIC;

    // create, init cpBody
    info->body = cpSpaceAddBody(space, cpBodyNew(info->mass, 1.0));
    cpBodySetUserData(info->body, ent); // for cpBody -> NativeEntity mapping
    cpBodySetPos(info->body, cpv_of_vec2(transform_get_position(ent)));
    cpBodySetAngle(info->body, transform_get_rotation(ent));
    info->last_dirty_count = transform_get_dirty_count(ent);

    // initially no shapes
    info->shapes = array_new(ShapeInfo);

    // initialize last_pos/last_ang info for kinematic bodies
    info->last_pos = cpBodyGetPos(info->body);
    info->last_ang = cpBodyGetAngle(info->body);

    info->collisions = NULL;
}

// remove chipmunk stuff (doesn't remove from pool)
static void _remove(PhysicsInfo *info)
{
    ShapeInfo *shapeInfo;

    array_foreach(shapeInfo, info->shapes)
        _remove_shape(shapeInfo->shape);
    array_free(info->shapes);

    _remove_body(info->body);
}

void physics_remove(NativeEntity ent)
{
    PhysicsInfo *info;

    info = entitypool_get(pool, ent);
    if (!info)
        return;

    _remove(info);
    entitypool_remove(pool, ent);
}

bool physics_has(NativeEntity ent)
{
    return entitypool_get(pool, ent) != NULL;
}

// calculate moment for a single shape
static Scalar _moment(cpBody *body, ShapeInfo *shapeInfo)
{
    Scalar mass = cpBodyGetMass(body);
    switch (shapeInfo->type)
    {
        case PS_CIRCLE:
            return cpMomentForCircle(mass, 0,
                                     cpCircleShapeGetRadius(shapeInfo->shape),
                                     cpCircleShapeGetOffset(shapeInfo->shape));

        case PS_POLYGON:
            return cpMomentForPoly(mass,
                                   cpPolyShapeGetNumVerts(shapeInfo->shape),
                                   ((cpPolyShape *) shapeInfo->shape)->verts,
                                   cpvzero);
    }
}

// recalculate moment for whole body by adding up shape moments
static void _recalculate_moment(PhysicsInfo *info)
{
    Scalar moment;
    ShapeInfo *shapeInfo;

    if (!info->body)
        return;
    if (array_length(info->shapes) == 0)
        return; // can't set moment to zero, just leave it alone

    moment = 0.0;
    array_foreach(shapeInfo, info->shapes)
        moment += _moment(info->body, shapeInfo);
    cpBodySetMoment(info->body, moment);
}

static void _set_type(PhysicsInfo *info, PhysicsBody type)
{
    if (info->type == type)
        return; // already set

    info->type = type;
    switch (type)
    {
        case PB_KINEMATIC:
            info->last_pos = cpBodyGetPos(info->body);
            info->last_ang = cpBodyGetAngle(info->body);
            // fall through

        case PB_STATIC:
            if (!cpBodyIsStatic(info->body))
            {
                cpSpaceRemoveBody(space, info->body);
                cpSpaceConvertBodyToStatic(space, info->body);
            }
            break;

        case PB_DYNAMIC:
            cpSpaceConvertBodyToDynamic(space, info->body, info->mass, 1.0);
            cpSpaceAddBody(space, info->body);
            _recalculate_moment(info);
            break;
    }
}
void physics_set_type(NativeEntity ent, PhysicsBody type)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    _set_type(info, type);
}
PhysicsBody physics_get_type(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return info->type;
}

void physics_debug_draw(NativeEntity ent)
{
    entitymap_set(debug_draw_map, ent, true);
}


// --- shape ---------------------------------------------------------------

static unsigned int _shape_add(NativeEntity ent, PhysicsShape type, cpShape *shape)
{
    PhysicsInfo *info;
    ShapeInfo *shapeInfo;

    info = entitypool_get(pool, ent);
    error_assert(info);

    // init ShapeInfo
    shapeInfo = array_add(info->shapes);
    shapeInfo->type = type;
    shapeInfo->shape = shape;

    // init cpShape
    cpShapeSetBody(shape, info->body);
    cpSpaceAddShape(space, shape);
    cpShapeSetFriction(shapeInfo->shape, 1);
    cpShapeSetUserData(shapeInfo->shape, ent);

    // update moment
    if (!cpBodyIsStatic(info->body))
    {
        if (array_length(info->shapes) > 1)
            cpBodySetMoment(info->body, _moment(info->body, shapeInfo)
                            + cpBodyGetMoment(info->body));
        else
            cpBodySetMoment(info->body, _moment(info->body, shapeInfo));
    }

    return array_length(info->shapes) - 1;
}
unsigned int physics_shape_add_circle(NativeEntity ent, Scalar r,
                                      vec2 offset)
{
    cpShape *shape = cpCircleShapeNew(NULL, r, cpv_of_vec2(offset));
    return _shape_add(ent, PS_CIRCLE, shape);
}
unsigned int physics_shape_add_box(NativeEntity ent, BBox b, Scalar r)
{
    cpShape *shape = cpBoxShapeNew3(NULL, cpBBNew(b.min.x, b.min.y,
                                                  b.max.x, b.max.y), r);
    return _shape_add(ent, PS_POLYGON, shape);
}
unsigned int physics_shape_add_poly(NativeEntity ent,
                                    unsigned int nverts,
                                    const vec2 *verts,
                                    Scalar r)
{
    unsigned int i;
    cpVect *cpverts;
    cpShape *shape;

    cpverts = mem_alloc(nverts * sizeof(cpVect));
    for (i = 0; i < nverts; ++i)
        cpverts[i] = cpv_of_vec2(verts[i]);
    nverts = cpConvexHull(nverts, cpverts, NULL, NULL, 0);
    shape = cpPolyShapeNew2(NULL, nverts, cpverts, cpvzero, r);
    mem_free(cpverts);
    return _shape_add(ent, PS_POLYGON, shape);
}

unsigned int physics_get_num_shapes(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return array_length(info->shapes);
}
PhysicsShape physics_shape_get_type(NativeEntity ent, unsigned int i)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    error_assert(i < array_length(info->shapes));
    return array_get_val(ShapeInfo, info->shapes, i).type;
}
void physics_shape_remove(NativeEntity ent, unsigned int i)
{
    PhysicsInfo *info;
    ShapeInfo *shapeInfo;

    info = entitypool_get(pool, ent);
    error_assert(info);

    if (i >= array_length(info->shapes))
        return;

    shapeInfo = array_get(info->shapes, i);
    _remove_shape(array_get_val(ShapeInfo, info->shapes, i).shape);
    array_quick_remove(info->shapes, i);
    _recalculate_moment(info);
}

static ShapeInfo *_get_shape(PhysicsInfo *info, unsigned int i)
{
    error_assert(i < array_length(info->shapes),
                 "shape index should be in range");
    return array_get(info->shapes, i);
}

int physics_poly_get_num_verts(NativeEntity ent, unsigned int i)
{
    PhysicsInfo *info;
    info = entitypool_get(pool, ent);
    error_assert(info);
    return cpPolyShapeGetNumVerts(_get_shape(info, i)->shape);
}

unsigned int physics_convex_hull(unsigned int nverts, vec2 *verts)
{
    cpVect *cpverts;
    unsigned int i;

    cpverts = mem_alloc(nverts * sizeof(cpVect));
    for (i = 0; i < nverts; ++i)
        cpverts[i] = cpv_of_vec2(verts[i]);
    nverts = cpConvexHull(nverts, cpverts, NULL, NULL, 0);
    for (i = 0; i < nverts; ++i)
        verts[i] = vec2_of_cpv(cpverts[i]);
    mem_free(cpverts);
    return nverts;
}

void physics_shape_set_surface_velocity(NativeEntity ent,
                                        unsigned int i,
                                        vec2 v)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpShapeSetSurfaceVelocity(_get_shape(info, i)->shape, cpv_of_vec2(v));
}
vec2 physics_shape_get_surface_velocity(NativeEntity ent,
                                        unsigned int i)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return vec2_of_cpv(cpShapeGetSurfaceVelocity(_get_shape(info, i)->shape));
}

void physics_shape_set_sensor(NativeEntity ent,
                              unsigned int i,
                              bool sensor)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpShapeSetSensor(_get_shape(info, i)->shape, sensor);
}
bool physics_shape_get_sensor(NativeEntity ent,
                              unsigned int i)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpShapeGetSensor(_get_shape(info, i)->shape);
}

// --- dynamics ------------------------------------------------------------

void physics_set_mass(NativeEntity ent, Scalar mass)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    if (mass <= SCALAR_EPSILON)
        return;

    cpBodySetMass(info->body, info->mass = mass);
    _recalculate_moment(info);
}
Scalar physics_get_mass(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return info->mass;
}

void physics_set_freeze_rotation(NativeEntity ent, bool freeze)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    if (freeze)
    {
        cpBodySetAngVel(info->body, 0);
        cpBodySetMoment(info->body, SCALAR_INFINITY);
    }
    else
        _recalculate_moment(info);
}
bool physics_get_freeze_rotation(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    // TODO: do this a better way? maybe store separate flag
    return cpBodyGetMoment(info->body) == SCALAR_INFINITY;
}

void physics_set_velocity(NativeEntity ent, vec2 vel)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetVel(info->body, cpv_of_vec2(vel));
}
vec2 physics_get_velocity(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return vec2_of_cpv(cpBodyGetVel(info->body));
}
void physics_set_force(NativeEntity ent, vec2 force)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetForce(info->body, cpv_of_vec2(force));
}
vec2 physics_get_force(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return vec2_of_cpv(cpBodyGetForce(info->body));
}

void physics_set_angular_velocity(NativeEntity ent, Scalar ang_vel)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetAngVel(info->body, ang_vel);
}
Scalar physics_get_angular_velocity(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetAngVel(info->body);
}
void physics_set_torque(NativeEntity ent, Scalar torque)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetTorque(info->body, torque);
}
Scalar physics_get_torque(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetTorque(info->body);
}

void physics_set_velocity_limit(NativeEntity ent, Scalar lim)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetVelLimit(info->body, lim);
}
Scalar physics_get_velocity_limit(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetVelLimit(info->body);
}
void physics_set_angular_velocity_limit(NativeEntity ent, Scalar lim)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodySetAngVelLimit(info->body, lim);
}
Scalar physics_get_angular_velocity_limit(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    return cpBodyGetAngVelLimit(info->body);
}

void physics_reset_forces(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodyResetForces(info->body);
}
void physics_apply_force(NativeEntity ent, vec2 force)
{
    physics_apply_force_at(ent, force, vec2_zero);
}
void physics_apply_force_at(NativeEntity ent, vec2 force, vec2 at)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodyApplyForce(info->body, cpv_of_vec2(force), cpv_of_vec2(at));
}
void physics_apply_impulse(NativeEntity ent, vec2 impulse)
{
    physics_apply_impulse_at(ent, impulse, vec2_zero);
}
void physics_apply_impulse_at(NativeEntity ent, vec2 impulse, vec2 at)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);
    cpBodyApplyImpulse(info->body, cpv_of_vec2(impulse), cpv_of_vec2(at));
}

// --- collisions ----------------------------------------------------------

static void _add_collision(cpBody *body, cpArbiter *arbiter, void *collisions)
{
    cpBody *ba, *bb;
    Collision *col;

    // get in right order
    cpArbiterGetBodies(arbiter, &ba, &bb);
    if (bb == body)
    {
        ba = body;
        bb = ba;
    }

    // save collision
    col = array_add(collisions);
    col->a = cpBodyGetUserData(ba);
    col->b = cpBodyGetUserData(bb);
}
static void _update_collisions(PhysicsInfo *info)
{
    if (info->collisions)
        return;

    // gather collisions
    info->collisions = array_new(Collision);
    cpBodyEachArbiter(info->body, _add_collision, info->collisions);
}

unsigned int physics_get_num_collisions(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    _update_collisions(info);
    return array_length(info->collisions);
}
Collision *physics_get_collisions(NativeEntity ent)
{
    PhysicsInfo *info = entitypool_get(pool, ent);
    error_assert(info);

    _update_collisions(info);
    return array_begin(info->collisions);
}


// --- queries -------------------------------------------------------------

NearestResult physics_nearest(vec2 point, Scalar max_dist)
{
    cpNearestPointQueryInfo info;
    NearestResult res;

    if (!cpSpaceNearestPointQueryNearest(space, cpv_of_vec2(point), max_dist,
                                         CP_ALL_LAYERS, CP_NO_GROUP, &info))
    {
        // no result
        res.ent = entity_nil;
        return res;
    }

    res.ent = cpShapeGetUserData(info.shape);
    res.p = vec2_of_cpv(info.p);
    res.d = info.d;
    res.g = vec2_of_cpv(info.g);
    return res;
}

// --- init/fini ---------------------------------------------------------

static GLuint program;
static GLuint vao;
static GLuint vbo;

void physics_init()
{
    // init pools, maps
    pool = entitypool_new(PhysicsInfo);
    debug_draw_map = entitymap_new(false);

    // init cpSpace
    space = cpSpaceNew();
    cpSpaceSetGravity(space, cpv(0, -9.8));

    // init draw stuff
    program = gfx_create_program(data_path("phypoly.vert"),
                                 NULL,
                                 data_path("phypoly.frag"));
    glUseProgram(program);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gfx_bind_vertex_attrib(program, GL_FLOAT, 2, "position", vec2, x);
}
void physics_fini()
{
    PhysicsInfo *info;

    // clean up draw stuff
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // remove all bodies, shapes
    entitypool_foreach(info, pool)
        _remove(info);

    // fini cpSpace
    cpSpaceFree(space);

    // fini pools, maps
    entitymap_free(debug_draw_map);
    entitypool_free(pool);
}

// --- update --------------------------------------------------------------

// step the space with fixed time step
static void _step()
{
    static Scalar remain = 0.0;

    remain += dt;
    while (remain >= period)
    {
        cpSpaceStep(space, period);
        remain -= period;
    }
}

static void _update_kinematics()
{
    PhysicsInfo *info;
    cpVect pos;
    cpFloat ang;
    Scalar invdt;
    NativeEntity ent;

    if (dt <= FLT_EPSILON)
        return;
    invdt = 1 / dt;

    entitypool_foreach(info, pool)
        if (info->type == PB_KINEMATIC)
        {
            ent = info->pool_elem.ent;

            // move to transform
            pos = cpv_of_vec2(transform_get_position(ent));
            ang = transform_get_rotation(ent);
            cpBodySetPos(info->body, pos);
            cpBodySetAngle(info->body, ang);
            info->last_dirty_count = transform_get_dirty_count(ent);

            // update linear, angular velocities based on delta
            cpBodySetVel(info->body,
                         cpvmult(cpvsub(pos, info->last_pos), invdt));
            cpBodySetAngVel(info->body, (ang - info->last_ang) * invdt);
            cpSpaceReindexShapesForBody(space, info->body);

            // save current state for next computation
            info->last_pos = pos;
            info->last_ang = ang;
        }
}
void physics_update_all()
{
    PhysicsInfo *info;
    NativeEntity ent;

    entitypool_remove_destroyed(pool, physics_remove);

    entitymap_clear(debug_draw_map);

    // simulate
    if (!timing_get_paused())
    {
        _update_kinematics();
        _step();
    }

    // synchronize transform <-> physics
    entitypool_foreach(info, pool)
    {
        ent = info->pool_elem.ent;

        // if transform is dirtier, move to it, else overwrite it
        if (transform_get_dirty_count(ent) != info->last_dirty_count)
        {
            cpBodySetVel(info->body, cpvzero);
            cpBodySetAngVel(info->body, 0.0f);
            cpBodySetPos(info->body, cpv_of_vec2(transform_get_position(ent)));
            cpBodySetAngle(info->body, transform_get_rotation(ent));
            cpSpaceReindexShapesForBody(space, info->body);
        }
        else if (info->type == PB_DYNAMIC)
        {
            transform_set_position(ent, vec2_of_cpv(cpBodyGetPos(info->body)));
            transform_set_rotation(ent, cpBodyGetAngle(info->body));
        }

        info->last_dirty_count = transform_get_dirty_count(ent);
    }
}

void physics_post_update_all()
{
    PhysicsInfo *info;

    entitypool_remove_destroyed(pool, physics_remove);

    // clear collisions
    entitypool_foreach(info, pool)
        if (info->collisions)
        {
            array_free(info->collisions);
            info->collisions = NULL;
        }
}

// --- draw ----------------------------------------------------------------

static void _circle_draw(PhysicsInfo *info, ShapeInfo *shapeInfo)
{
    static vec2 verts[] = {
        {  1.0,  0.0 }, {  0.7071,  0.7071 },
        {  0.0,  1.0 }, { -0.7071,  0.7071 },
        { -1.0,  0.0 }, { -0.7071, -0.7071 },
        {  0.0, -1.0 }, {  0.7071, -0.7071 },
    }, offset;
    const unsigned int nverts = sizeof(verts) / sizeof(verts[0]);
    Scalar r;
    mat3 wmat;

    wmat = transform_get_world_matrix(info->pool_elem.ent);
    offset = vec2_of_cpv(cpCircleShapeGetOffset(shapeInfo->shape));
    r = cpCircleShapeGetRadius(shapeInfo->shape);

    glUniformMatrix3fv(glGetUniformLocation(program, "wmat"),
                       1, GL_FALSE, (const GLfloat *) &wmat);
    glUniform2fv(glGetUniformLocation(program, "offset"),
                 1, (const GLfloat *) &offset);
    glUniform1f(glGetUniformLocation(program, "radius"), r);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(vec2),
                 verts, GL_STREAM_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, nverts);
    glDrawArrays(GL_POINTS, 0, nverts);
}

static void _polygon_draw(PhysicsInfo *info, ShapeInfo *shapeInfo)
{
    unsigned int i, nverts;
    vec2 *verts;
    mat3 wmat;

    wmat = transform_get_world_matrix(info->pool_elem.ent);
    glUniformMatrix3fv(glGetUniformLocation(program, "wmat"),
                       1, GL_FALSE, (const GLfloat *) &wmat);

    glUniformMatrix3fv(glGetUniformLocation(program, "wmat"),
                       1, GL_FALSE, (const GLfloat *) &wmat);
    glUniform2f(glGetUniformLocation(program, "offset"), 0, 0);
    glUniform1f(glGetUniformLocation(program, "radius"), 1);

    // copy as vec2 array
    nverts = cpPolyShapeGetNumVerts(shapeInfo->shape);
    verts = mem_alloc(nverts * sizeof(vec2));
    for (i = 0; i < nverts; ++i)
        verts[i] = vec2_of_cpv(cpPolyShapeGetVert(shapeInfo->shape, i));

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(vec2),
                 verts, GL_STREAM_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, nverts);
    glDrawArrays(GL_POINTS, 0, nverts);

    mem_free(verts);
}

void physics_draw_all()
{
    PhysicsInfo *info;
    ShapeInfo *shapeInfo;

    if (!edit_get_enabled())
        return;

    // bind program, update uniforms
    glUseProgram(program);
    glUniformMatrix3fv(glGetUniformLocation(program, "inverse_view_matrix"),
                       1, GL_FALSE,
                       (const GLfloat *) camera_get_inverse_view_matrix_ptr());

    // draw!
    entitypool_foreach(info, pool)
        if (entitymap_get(debug_draw_map, info->pool_elem.ent))
            array_foreach(shapeInfo, info->shapes)
                switch(shapeInfo->type)
                {
                    case PS_CIRCLE:
                        _circle_draw(info, shapeInfo);
                        break;

                    case PS_POLYGON:
                        _polygon_draw(info, shapeInfo);
                        break;
                }
}

// --- save/load -----------------------------------------------------------

// chipmunk data save/load helpers
static void _cpv_save(cpVect *cv, const char *n, Store *s)
{
    vec2 v = vec2_of_cpv(*cv);
    vec2_save(&v, n, s);
}
static bool _cpv_load(cpVect *cv, const char *n, cpVect d, Store *s)
{
    vec2 v;
    if (vec2_load(&v, n, vec2_zero, s))
    {
        *cv = cpv_of_vec2(v);
        return true;
    }
    *cv = d;
    return false;
}
static void _cpf_save(cpFloat *cf, const char *n, Store *s)
{
    Scalar f = *cf;
    scalar_save(&f, n, s);
}
static bool _cpf_load(cpFloat *cf, const char *n, cpFloat d, Store *s)
{
    Scalar f;
    bool r = scalar_load(&f, n, d, s);
    *cf = f;
    return r;
}

// properties aren't set if missing, so defaults don't really matter
#define _cpv_default cpvzero
#define _cpf_default 0.0f
#define bool_default false
#define uint_default 0

/*
 * note that UserData isn't directly save/load'd -- it is restored to
 * the NativeEntity value on load separately
 */

// some hax to reduce typing for body properties save/load
#define body_prop_save(type, f, n, prop) \
    {                                    \
        type v;                          \
        v = cpBodyGet##prop(info->body); \
        f##_save(&v, n, body_s);         \
    }
#define body_prop_load(type, f, n, prop)                                          \
    {                                                                             \
        type v;                                                                   \
        if (f##_load(&v, n, f##_default, body_s)) cpBodySet##prop(info->body, v); \
    }

#define body_props_saveload(saveload)                           \
    body_prop_##saveload(cpFloat, _cpf, "mass", Mass);          \
    body_prop_##saveload(cpFloat, _cpf, "moment", Moment);      \
    /* body_prop_##saveload(cpVect, _cpv, Pos); */              \
    body_prop_##saveload(cpVect, _cpv, "vel", Vel);             \
    body_prop_##saveload(cpVect, _cpv, "force", Force);         \
    /* body_prop_##saveload(cpFloat, _cpf, Angle); */           \
    body_prop_##saveload(cpFloat, _cpf, "ang_vel", AngVel);     \
    body_prop_##saveload(cpFloat, _cpf, "torque", Torque);      \
    body_prop_##saveload(cpFloat, _cpf, "vel_limit", VelLimit); \
    body_prop_##saveload(cpFloat, _cpf, "ang_vel_limit", AngVelLimit);  

// save/load for just the body in a PhysicsInfo
static void _body_save(PhysicsInfo *info, Store *s)
{
    Store *body_s;

    if (store_child_save(&body_s, "body", s))
        body_props_saveload(save);
}
static void _body_load(PhysicsInfo *info, Store *s)
{
    Store *body_s;
    NativeEntity ent;
    PhysicsBody type;

    error_assert(store_child_load(&body_s, "body", s),
                 "physics entry must have a saved body");

    // create, restore properties
    ent = info->pool_elem.ent;
    info->body = cpSpaceAddBody(space, cpBodyNew(info->mass, 1.0));
    body_props_saveload(load);
    cpBodySetUserData(info->body, ent);

    // force type change if non-default
    type = info->type;
    info->type = PB_DYNAMIC;
    _set_type(info, type);

    // restore position, angle based on transform
    cpBodySetPos(info->body, cpv_of_vec2(transform_get_position(ent)));
    cpBodySetAngle(info->body, transform_get_rotation(ent));
    info->last_dirty_count = transform_get_dirty_count(info->pool_elem.ent);
}

// save/load for special properties of each shape type

static void _circle_save(PhysicsInfo *info, ShapeInfo *shapeInfo,
                         Store *s)
{
    cpFloat radius;
    cpVect offset;

    radius = cpCircleShapeGetRadius(shapeInfo->shape);
    _cpf_save(&radius, "radius", s);
    offset = cpCircleShapeGetOffset(shapeInfo->shape);
    _cpv_save(&offset, "offset", s);
}
static void _circle_load(PhysicsInfo *info, ShapeInfo *shapeInfo,
                         Store *s)
{
    cpFloat radius;
    cpVect offset;

    _cpf_load(&radius, "radius", 1, s);
    _cpv_load(&offset, "offset", cpvzero, s);

    shapeInfo->shape = cpCircleShapeNew(info->body, radius, offset);
    cpSpaceAddShape(space, shapeInfo->shape);
}

static void _polygon_save(PhysicsInfo *info, ShapeInfo *shapeInfo,
                          Store *s)
{
    Store *verts_s;
    unsigned int n, i;
    Scalar r;
    cpVect v;

    n = cpPolyShapeGetNumVerts(shapeInfo->shape);
    uint_save(&n, "num_verts", s);

    r = cpPolyShapeGetRadius(shapeInfo->shape);
    scalar_save(&r, "radius", s);

    if (store_child_save(&verts_s, "verts", s))
        for (i = 0; i < n; ++i)
        {
            v = cpPolyShapeGetVert(shapeInfo->shape, i);
            _cpv_save(&v, NULL, verts_s);
        }
}
static void _polygon_load(PhysicsInfo *info, ShapeInfo *shapeInfo,
                          Store *s)
{
    Store *verts_s;
    unsigned int n, i;
    Scalar r;
    cpVect *vs;

    error_assert(uint_load(&n, "num_verts", 0, s),
                 "polygon shape type must have saved number of vertices");
    scalar_load(&r, "radius", 0, s);

    error_assert(store_child_load(&verts_s, "verts", s),
                 "polygon shape type must have saved list of vertices");
    vs = mem_alloc(n * sizeof(cpVect));
    for (i = 0; i < n; ++i)
        if (!_cpv_load(&vs[i], NULL, cpvzero, verts_s))
            error("polygon shape type saved number of vertices doesn't match"
                  "size of saved list of vertices");
    shapeInfo->shape = cpPolyShapeNew2(info->body, n, vs, cpvzero, r);
    cpSpaceAddShape(space, shapeInfo->shape);
    mem_free(vs);
}

// some hax to reduce typing for shape properties save/load
#define shape_prop_save(type, f, n, prop)       \
    {                                           \
        type v;                                 \
        v = cpShapeGet##prop(shapeInfo->shape); \
        f##_save(&v, n, shape_s);               \
    }
#define shape_prop_load(type, f, n, prop)                                                 \
    {                                                                                     \
        type v;                                                                           \
        if (f##_load(&v, n, f##_default, shape_s)) cpShapeSet##prop(shapeInfo->shape, v); \
    }
#define shape_props_saveload(saveload)                                          \
    shape_prop_##saveload(bool, bool, "sensor", Sensor);                        \
    shape_prop_##saveload(cpFloat, _cpf, "elasticity", Elasticity);             \
    shape_prop_##saveload(cpFloat, _cpf, "friction", Friction);                 \
    shape_prop_##saveload(cpVect, _cpv, "surface_velocity", SurfaceVelocity);   \
    shape_prop_##saveload(unsigned int, uint, "collision_type", CollisionType); \
    shape_prop_##saveload(unsigned int, uint, "group", Group);                  \
    shape_prop_##saveload(unsigned int, uint, "layers", Layers);        \

// save/load for all shapes in a PhysicsInfo
static void _shapes_save(PhysicsInfo *info, Store *s)
{
    Store *t, *shape_s;
    ShapeInfo *shapeInfo;

    if (store_child_save(&t, "shapes", s))
        array_foreach(shapeInfo, info->shapes)
            if (store_child_save(&shape_s, NULL, t))
            {
                // type-specific
                enum_save(&shapeInfo->type, "type", shape_s);
                switch (shapeInfo->type)
                {
                    case PS_CIRCLE:
                        _circle_save(info, shapeInfo, shape_s);
                        break;
                    case PS_POLYGON:
                        _polygon_save(info, shapeInfo, shape_s);
                        break;
                }

                // common
                shape_props_saveload(save);
            }
}
static void _shapes_load(PhysicsInfo *info, Store *s)
{
    Store *t, *shape_s;
    ShapeInfo *shapeInfo;

    info->shapes = array_new(ShapeInfo);

    if (store_child_load(&t, "shapes", s))
        while (store_child_load(&shape_s, NULL, t))
        {
            shapeInfo = array_add(info->shapes);

            // type-specific
            enum_load(&shapeInfo->type, "type", PS_CIRCLE, shape_s);
            switch (shapeInfo->type)
            {
                case PS_CIRCLE:
                    _circle_load(info, shapeInfo, shape_s);
                    break;
                case PS_POLYGON:
                    _polygon_load(info, shapeInfo, shape_s);
                    break;
            }

            // common
            shape_props_saveload(load);
            cpShapeSetUserData(shapeInfo->shape, info->pool_elem.ent);
        }
}

/* 
 * save/load for all data in a PhysicsInfo other than the the actual body,
 * shapes is handled here, the rest is done in functions above
 */
void physics_save_all(Store *s)
{
    Store *t, *info_s;
    PhysicsInfo *info;

    if (store_child_save(&t, "physics", s))
        entitypool_save_foreach(info, info_s, pool, "pool", t)
        {
            enum_save(&info->type, "type", info_s);
            scalar_save(&info->mass, "mass", info_s);

            _body_save(info, info_s);
            _shapes_save(info, info_s);
        }
}
void physics_load_all(Store *s)
{
    Store *t, *info_s;
    PhysicsInfo *info;

    if (store_child_load(&t, "physics", s))
        entitypool_load_foreach(info, info_s, pool, "pool", t)
        {
            enum_load(&info->type, "type", PB_DYNAMIC, info_s);
            scalar_load(&info->mass, "mass", 1, info_s);

            _body_load(info, info_s);
            _shapes_load(info, info_s);

            info->collisions = NULL;

            // set last_pos/last_ang info for kinematic bodies
            info->last_pos = cpBodyGetPos(info->body);
            info->last_ang = cpBodyGetAngle(info->body);
        }
}

#endif

void physics_init() { PROFILE_FUNC(); }
void physics_fini() {}
int physics_update_all(App *app, event_t evt) { return 0; }
int physics_post_update_all(App *app, event_t evt) { return 0; }
void physics_draw_all() {}
void physics_save_all(Store *s) {}
void physics_load_all(Store *s) {}
