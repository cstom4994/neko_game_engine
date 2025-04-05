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
#include "base/common/os.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "base/scripting/lua_wrapper.hpp"
#include "base/common/reflection.hpp"
#include "base/scripting/scripting.h"
#include "engine/ui.h"

#pragma region test

int test_xml(lua_State *L);

namespace Neko::luabind::__unittest {

#if 0

// System IDs
EcsId System1;
EcsId System2;
EcsId System3;

// Register components
void register_components(ecs_t* ecs) {
    // PosComp = ecs_register_component(ecs, sizeof(pos_t), NULL, NULL);
    // VelComp = ecs_register_component(ecs, sizeof(vel_t), NULL, NULL);
    // RectComp = ecs_register_component(ecs, sizeof(rect_t), NULL, NULL);
}

// System that prints the entity IDs of entities associated with this system
ecs_ret_t system_update(ecs_t* ecs, EcsId* entities, int entity_count, ecs_dt_t dt, void* udata) {
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
    EcsId e1 = ecs_create(ecs);
    EcsId e2 = ecs_create(ecs);
    EcsId e3 = ecs_create(ecs);

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
    constexpr std::size_t size = Neko::util::tuple_size_v<decltype(myTuple)>;

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

void draw_gui_auto_test();

void draw_gui() {

    PROFILE_FUNC();

    const f64 t = the<CL>().timing_get_elapsed();

    {
        // editor_dockspace(g_app->ui);

#if 0
        if (1) {

            ui_demo_window(the<CL>().ui, ui_rect(100, 100, 500, 500), NULL);
            ui_style_editor(the<CL>().ui, NULL, ui_rect(350, 250, 300, 240), NULL);

            const vec2 ws = neko_v2(600.f, 300.f);

            // const ui_style_sheet_t *ss = &game_userdata->style_sheet;

            const vec2 ss_ws = neko_v2(500.f, 300.f);
            ui_window_begin(the<CL>().ui, "Window", ui_rect((the<CL>().width - ss_ws.x) * 0.5f, (the<CL>().height - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
            {
                // Cache the current container
                ui_container_t* cnt = ui_get_current_container(the<CL>().ui);

                ui_layout_row(the<CL>().ui, 2, ui_widths(200, 0), 0);

                ui_text(the<CL>().ui, "A regular element button.");
                ui_button(the<CL>().ui, "button");

                ui_text(the<CL>().ui, "A regular element label.");
                ui_label(the<CL>().ui, "label");

                ui_text(the<CL>().ui, "Button with classes: {.c0 .btn}");

                ui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                ui_button_ex(the<CL>().ui, "hello?##btn", &selector_1, 0x00);

                ui_text(the<CL>().ui, "Label with id #lbl and class .c0");
                ui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                ui_label_ex(the<CL>().ui, "label##lbl", &selector_2, 0x00);

                const f32 m = cnt->body.w * 0.3f;
                // ui_layout_row(gui, 2, (int[]){m, -m}, 0);
                // ui_layout_next(gui); // Empty space at beginning
                ui_layout_row(the<CL>().ui, 1, ui_widths(0), 0);
                ui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                if (ui_button_ex(the<CL>().ui, "reload style sheet", &selector_3, 0x00)) {
                    // app_load_style_sheet(true);
                }

                button_custom(the<CL>().ui, "Hello?");
            }
            ui_window_end(the<CL>().ui);

            ui_window_begin(the<CL>().ui, "Idraw", ui_rect((the<CL>().width - ws.x) * 0.2f, (the<CL>().height - ws.y) * 0.5f, ws.x, ws.y));
            {
                // Cache the current container
                ui_container_t* cnt = ui_get_current_container(the<CL>().ui);

                // 绘制到当前窗口中的转换对象的自定义回调
                // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                // 正如我们将在“分割”窗口示例中看到的那样。
                // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                // 你不需要任何东西
                Color256 color = color256_alpha(NEKO_COLOR_RED, (uint8_t)NEKO_CLAMP((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                ui_draw_custom(the<CL>().ui, cnt->body, gui_cb, &color, sizeof(color));
            }
            ui_window_end(the<CL>().ui);
        }
#endif

        if (1) {

            // vec2 mp = g_app->ui.mouse_pos;
            // vec2 mw = g_app->ui.scroll_delta;
            vec2 md = {};
            // bool lock = g_app->ui.mouse_pressed;
            // bool moved = neko_os_mouse_moved();

            if (ui_begin_window(the<CL>().ui, "Game", neko_rect(the<CL>().state.width - 210, 30, 200, 200))) {
                // l = g_app->ui->layout_stack[0];
                ui_layout_row(the<CL>().ui, 1, ui_widths(-1), 0);

                static f32 delta, fps = NEKO_DEFAULT_VAL();
                delta = the<CL>().GetTimeInfo().true_dt;
                fps = 1.f / delta;

                Color256 col = NEKO_COLOR_GREEN;
                // ui_textf_colored(g_app->ui, &col, "Neko %d", neko_buildnum());

                ui_labelf("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(ENGINE_LUA(), LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024),
                          the<CL>().GetTimeInfo().true_dt * 1000.f, 1.f / the<CL>().GetTimeInfo().true_dt);

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

                ui_layout_row(the<CL>().ui, 7, ui_widths(100, 100, 32, 100, 32, 100, 32), 0);
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

                ui_layout_row(the<CL>().ui, 1, ui_widths(-1), 0);
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

                ui_end_window(the<CL>().ui);
            }
        }
    }

    vec2 fb = Neko::the<CL>().get_window_size();
    rect_t screen = neko_rect(0, 0, fb.x, fb.y);

    // draw_gui_auto_test();
}

#if 1
void draw_gui_auto_test() {
    ui_context_t *ui = the<CL>().ui;
    const vec2 ss_ws = neko_v2(500.f, 300.f);
    ui_begin_window(the<CL>().ui, "GUI Test", neko_rect((the<CL>().state.width - ss_ws.x) * 0.5f, (the<CL>().state.height - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
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
    ui_end_window(the<CL>().ui);
}
#endif

#endif
