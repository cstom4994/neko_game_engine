#include "neko_imgui_utils.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "engine/base/neko_engine.h"
#include "engine/common/neko_util.h"
#include "engine/gui/imgui_impl/imgui_impl_glfw.h"
#include "engine/gui/imgui_impl/imgui_impl_opengl3.h"
#include "engine/platform/neko_platform.h"
#include "libs/glad/glad.h"

namespace neko::imgui {

bool color_picker_3U32(const char* label, ImU32* color, ImGuiColorEditFlags flags) {
    float col[3];
    col[0] = (float)((*color >> 0) & 0xFF) / 255.0f;
    col[1] = (float)((*color >> 8) & 0xFF) / 255.0f;
    col[2] = (float)((*color >> 16) & 0xFF) / 255.0f;

    bool result = ImGui::ColorPicker3(label, col, flags);

    *color = ((ImU32)(col[0] * 255.0f)) | ((ImU32)(col[1] * 255.0f) << 8) | ((ImU32)(col[2] * 255.0f) << 16);

    return result;
}

void file_browser(std::string& path) {

    ImGui::Text("Current Path: %s", path.c_str());
    ImGui::Separator();

    if (ImGui::Button("Parent Directory")) {
        std::filesystem::path current_path(path);
        if (!current_path.empty()) {
            current_path = current_path.parent_path();
            path = current_path.string();
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        const auto& entry_path = entry.path();
        const auto& filename = entry_path.filename().string();
        if (entry.is_directory()) {
            if (ImGui::Selectable((filename + "/").c_str())) path = entry_path.string();

        } else {
            if (ImGui::Selectable(filename.c_str())) path = entry_path.string();
        }
    }
}

}  // namespace neko::imgui

namespace neko {

void neko_imgui_style() {

    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 bgColor = neko_imgui_color_from_byte(37, 37, 38);
    const ImVec4 lightBgColor = neko_imgui_color_from_byte(82, 82, 85);
    const ImVec4 veryLightBgColor = neko_imgui_color_from_byte(90, 90, 95);

    const ImVec4 panelColor = neko_imgui_color_from_byte(51, 51, 55);
    const ImVec4 panelHoverColor = neko_imgui_color_from_byte(29, 151, 236);
    const ImVec4 panelActiveColor = neko_imgui_color_from_byte(0, 119, 200);

    const ImVec4 textColor = neko_imgui_color_from_byte(255, 255, 255);
    const ImVec4 textDisabledColor = neko_imgui_color_from_byte(151, 151, 151);
    const ImVec4 borderColor = neko_imgui_color_from_byte(78, 78, 78);

    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    colors[ImGuiCol_MenuBarBg] = panelColor;
    colors[ImGuiCol_ScrollbarBg] = panelColor;
    colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    colors[ImGuiCol_CheckMark] = panelActiveColor;
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelHoverColor;
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = borderColor;
    colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    // colors[ImGuiCol_ModalWindowDarkening] = bgColor;
    colors[ImGuiCol_DragDropTarget] = bgColor;
    colors[ImGuiCol_NavHighlight] = bgColor;
    colors[ImGuiCol_DockingPreview] = panelActiveColor;
    colors[ImGuiCol_Tab] = bgColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = bgColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;

    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
}

void neko_draw_text_plate(std::string text, neko_color_t col, int x, int y, neko_color_t backcolor) {
    auto text_size = ImGui::CalcTextSize(text.c_str());
    // R_RectangleFilled(target, x - 4, y - 4, x + text_size.x + 4, y + text_size.y + 4, backcolor);
    neko_draw_text(text, col, x, y);
}

u32 neko_draw_darken_color(u32 color, float brightness) {
    int a = (color >> 24) & 0xFF;
    int r = (int)(((color >> 16) & 0xFF) * brightness);
    int g = (int)(((color >> 8) & 0xFF) * brightness);
    int b = (int)((color & 0xFF) * brightness);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

void neko_draw_text(std::string text, neko_color_t col, int x, int y, bool outline, neko_color_t outline_col) {

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList(viewport);

    if (outline) {

        auto outline_col_im = ImColor(outline_col.r, outline_col.g, outline_col.b, col.a);

        draw_list->AddText(ImVec2(x + 0, y - 1), outline_col_im, text.c_str());  // up
        draw_list->AddText(ImVec2(x + 0, y + 1), outline_col_im, text.c_str());  // down
        draw_list->AddText(ImVec2(x + 1, y + 0), outline_col_im, text.c_str());  // right
        draw_list->AddText(ImVec2(x - 1, y + 0), outline_col_im, text.c_str());  // left

        draw_list->AddText(ImVec2(x + 1, y + 1), outline_col_im, text.c_str());  // down-right
        draw_list->AddText(ImVec2(x - 1, y + 1), outline_col_im, text.c_str());  // down-left

        draw_list->AddText(ImVec2(x + 1, y - 1), outline_col_im, text.c_str());  // up-right
        draw_list->AddText(ImVec2(x - 1, y - 1), outline_col_im, text.c_str());  // up-left
    }

    draw_list->AddText(ImVec2(x, y), ImColor(col.r, col.g, col.b, col.a), text.c_str());  // base
}

}  // namespace neko

#if 1

void ShowAutoTestWindow() {

    if (ImGui::Begin("自动序列UI")) {

        auto myCollapsingHeader = [](const char* name) -> bool {
            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_Button]);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ButtonHovered]);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);
            bool b = ImGui::CollapsingHeader(name);
            ImGui::PopStyleColor(3);
            return b;
        };
        if (myCollapsingHeader("About ImGui::Auto()")) {
            ImGui::Auto(R"comment(
ImGui::Auto() is one simple function that can create GUI for almost any data structure using ImGui functions.
a. Your data is presented in tree-like structure that is defined by your type.
    The generated code is highly efficient.
b. Const types are display only, the user cannot modify them.
    Use ImGui::as_cost() to permit the user to modify a non-const type.
The following types are supported (with nesting):
1 Strings. Flavours of std::string and char*. Use std::string for input.
2 Numbers. Integers, Floating points, ImVec{2,4}
3 STL Containers. Standard containers are supported. (std::vector, std::map,...)
The contained type has to be supported. If it is not, see 8.
4 Pointers, arrays. Pointed type must be supported.
5 std::pair, std::tuple. Types used must be supported.
6 structs and simple classes! The struct is converted to a tuple, and displayed as such.
    * Requirements: C++14 compiler (GCC-5.0+, Clang, Visual Studio 2017 with /std:c++17, ...)
    * How? I'm using this libary https://github.com/apolukhin/magic_get (ImGui::Auto() is only VS2017 C++17 tested.)
7 Functions. A void(void) type function becomes simple button.
For other types, you can input the arguments and calculate the result.
Not yet implemented!
8 You can define ImGui::Auto() for your own type! Use the following macros:
    * ME_GUI_DEFINE_INLINE(template_spec, type_spec, code)  single line definition, teplates shouldn't have commas inside!
    * ME_GUI_DEFINE_BEGIN(template_spec, type_spec)        start multiple line definition, no commas in arguments!
    * ME_GUI_DEFINE_BEGIN_P(template_spec, type_spec)      start multiple line definition, can have commas, use parentheses!
    * ME_GUI_DEFINE_END                                    end multiple line definition with this.
where
    * template_spec   describes how the type is templated. For fully specialized, use "template<>" only
    * type_spec       is the type for witch you define the ImGui::Auto() function.
    * var             will be the generated function argument of type type_spec.
    * name            is the const std::string& given by the user, and/or generated by the caller ImGui::Auto function
Example:           ME_GUI_DEFINE_INLINE(template<>, bool, ImGui::Checkbox(name.c_str(), &var);)
Tipps: - You may use ImGui::Auto_t<type>::Auto(var, name) functions directly.
        - Check imdetail namespace for other helper functions.

The libary uses partial template specialization, so definitions can overlap, the most specialized will be chosen.
However, using large, nested structs can lead to slow compilation.
Container data, large structs and tuples are hidden by default.
)comment");
            if (ImGui::TreeNode("TODO-s")) {
                ImGui::Auto(R"todos(
    1   Insert items to (non-const) set, map, list, ect
    2   Call any function or function object.
    3   Deduce function arguments as a tuple. User can edit the arguments, call the function (button), and view return value.
    4   All of the above needs self ImGui::Auto() allocated memmory. Current plan is to prioritize low memory usage
        over user experiance. (Beacuse if you want good UI, you code it, not generate it.)
        Plan A:
            a)  Data is stored in a map. Key can be the presneted object's address.
            b)  Data is allocated when not already present in the map
            c)  std::unique_ptr<void, deleter> is used in the map with deleter function manually added (or equivalent something)
            d)  The first call for ImGui::Auto at every few frames should delete
                (some of, or) all data that was not accesed in the last (few) frames. How?
        This means after closing a TreeNode, and opening it, the temporary values might be deleted and recreated.
)todos");
                ImGui::TreePop();
            }
        }
        if (myCollapsingHeader("1. String")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    ImGui::Auto("Hello ImAuto() !"); //This is how this text is written as well.)code");
            ImGui::Auto("Hello ImAuto() !");  // This is how this text is written as well.

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::string str = "Hello ImGui::Auto() for strings!";
    ImGui::Auto(str, "asd");)code");
            static std::string str = "Hello ImGui::Auto() for strings!";
            ImGui::Auto(str, "str");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::string str2 = "ImGui::Auto()\n Automatically uses multiline input for strings!\n:)";
    ImGui::Auto(str2, "str2");)code");
            static std::string str2 = "ImGui::Auto()\n Automatically uses multiline input for strings!\n:)";
            ImGui::Auto(str2, "str2");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::string conststr = "Const types are not to be changed!";
    ImGui::Auto(conststr, "conststr");)code");
            static const std::string conststr = "Const types are not to be changed!";
            ImGui::Auto(conststr, "conststr");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    char * buffer = "To edit a string use std::string. Manual buffers are unwelcome here.";
    ImGui::Auto(buffer, "buffer");)code");
            char* buffer = "To edit a string use std::string. Manual buffers are unwelcome here.";
            ImGui::Auto(buffer, "buffer");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("2. Numbers")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static int i = 42;
    ImGui::Auto(i, "i");)code");
            static int i = 42;
            ImGui::Auto(i, "i");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static float f = 3.14;
    ImGui::Auto(f, "f");)code");
            static float f = 3.14;
            ImGui::Auto(f, "f");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static ImVec4 f4 = {1.5f,2.1f,3.4f,4.3f};
    ImGui::Auto(f4, "f4");)code");
            static ImVec4 f4 = {1.5f, 2.1f, 3.4f, 4.3f};
            ImGui::Auto(f4, "f4");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const ImVec2 f2 = {1.f,2.f};
    ImGui::Auto(f2, "f2");)code");
            static const ImVec2 f2 = {1.f, 2.f};
            ImGui::Auto(f2, "f2");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("3. Containers")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static std::vector<std::string> vec = { "First string","Second str",":)" };
    ImGui::Auto(vec,"vec");)code");
            static std::vector<std::string> vec = {"First string", "Second str", ":)"};
            ImGui::Auto(vec, "vec");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::vector<float> constvec = { 3,1,2.1f,4,3,4,5 };
    ImGui::Auto(constvec,"constvec");   //Cannot change vector, nor values)code");
            static const std::vector<float> constvec = {3, 1, 2.1f, 4, 3, 4, 5};
            ImGui::Auto(constvec, "constvec");  // Cannot change vector, nor values

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::vector<bool> bvec = { false, true, false, false };
    ImGui::Auto(bvec,"bvec");)code");
            static std::vector<bool> bvec = {false, true, false, false};
            ImGui::Auto(bvec, "bvec");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::vector<bool> constbvec = { false, true, false, false };
    ImGui::Auto(constbvec,"constbvec");
    )code");
            static const std::vector<bool> constbvec = {false, true, false, false};
            ImGui::Auto(constbvec, "constbvec");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::map<int, float> map = { {3,2},{1,2} };
    ImGui::Auto(map, "map");    // insert and other operations)code");
            static std::map<int, float> map = {{3, 2}, {1, 2}};
            ImGui::Auto(map, "map");  // insert and other operations

            if (ImGui::TreeNode("All cases")) {
                ImGui::Auto(R"code(
    static std::deque<bool> deque = { false, true, false, false };
    ImGui::Auto(deque,"deque");)code");
                static std::deque<bool> deque = {false, true, false, false};
                ImGui::Auto(deque, "deque");

                ImGui::NewLine();
                ImGui::Separator();

                ImGui::Auto(R"code(
    static std::set<char*> set = { "set","with","char*" };
    ImGui::Auto(set,"set");)code");
                static std::set<char*> set = {"set", "with", "char*"};  // for some reason, this does not work
                ImGui::Auto(set, "set");                                // the problem is with the const iterator, but

                ImGui::NewLine();
                ImGui::Separator();

                ImGui::Auto(R"code(
    static std::map<char*, std::string> map = { {"asd","somevalue"},{"bsd","value"} };
    ImGui::Auto(map, "map");    // insert and other operations)code");
                static std::map<char*, std::string> map = {{"asd", "somevalue"}, {"bsd", "value"}};
                ImGui::Auto(map, "map");  // insert and other operations

                ImGui::TreePop();
            }
            ImGui::Unindent();
        }
        if (myCollapsingHeader("4. Pointers and Arrays")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static float *pf = nullptr;
    ImGui::Auto(pf, "pf");)code");
            static float* pf = nullptr;
            ImGui::Auto(pf, "pf");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static int i=10, *pi=&i;
    ImGui::Auto(pi, "pi");)code");
            static int i = 10, *pi = &i;
            ImGui::Auto(pi, "pi");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::string cs= "I cannot be changed!", * cps=&cs;
    ImGui::Auto(cps, "cps");)code");
            static const std::string cs = "I cannot be changed!", *cps = &cs;
            ImGui::Auto(cps, "cps");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::string str = "I can be changed! (my pointee cannot)";
    static std::string *const strpc = &str;)code");
            static std::string str = "I can be changed! (my pointee cannot)";
            static std::string* const strpc = &str;
            ImGui::Auto(strpc, "strpc");

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::Auto(R"code(
    static std::array<float,5> farray = { 1.2, 3.4, 5.6, 7.8, 9.0 };
    ImGui::Auto(farray, "std::array");)code");
            static std::array<float, 5> farray = {1.2, 3.4, 5.6, 7.8, 9.0};
            ImGui::Auto(farray, "std::array");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static float farr[5] = { 1.2, 3.4, 5.6, 7.8, 9.0 };
    ImGui::Auto(farr, "float[5]");)code");
            static float farr[5] = {11.2, 3.4, 5.6, 7.8, 911.0};
            ImGui::Auto(farr, "float[5]");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("5. Pairs and Tuples")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static std::pair<bool, ImVec2> pair = { true,{2.1f,3.2f} };
    ImGui::Auto(pair, "pair");)code");
            static std::pair<bool, ImVec2> pair = {true, {2.1f, 3.2f}};
            ImGui::Auto(pair, "pair");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::pair<int, std::string> pair2 = { -3,"simple types appear next to each other in a pair" };
    ImGui::Auto(pair2, "pair2");)code");
            static std::pair<int, std::string> pair2 = {-3, "simple types appear next to each other in a pair"};
            ImGui::Auto(pair2, "pair2");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    ImGui::Auto(ImGui::as_const(pair), "as_const(pair)"); //easy way to view as const)code");
            ImGui::Auto(ImGui::as_const(pair), "as_const(pair)");  // easy way to view as const

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    std::tuple<const int, std::string, ImVec2> tuple = { 42, "string in tuple", {3.1f,3.2f} };
    ImGui::Auto(tuple, "tuple");)code");
            std::tuple<const int, std::string, ImVec2> tuple = {42, "string in tuple", {3.1f, 3.2f}};
            ImGui::Auto(tuple, "tuple");

            ImGui::NewLine();
            ImGui::Separator();

            //  ImGui::Auto(R"code(
            // const std::tuple<int, const char*, ImVec2> consttuple = { 42, "smaller tuples are inlined", {3.1f,3.2f} };
            // ImGui::Auto(consttuple, "consttuple");)code");
            //  const std::tuple<int, const char*, ImVec2> consttuple = { 42, "Smaller tuples are inlined", {3.1f,3.2f} };
            //  ImGui::Auto(consttuple, "consttuple");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("6. Structs!!")) {
            ImGui::Indent();

            //     ImGui::Auto(R"code(
            // struct A //Structs are automagically converted to tuples!
            // {
            //  int i = 216;
            //  bool b = true;
            // };
            // static A a;
            // ImGui::Auto("a", a);)code");
            //     struct A {
            //         int i = 216;
            //         bool b = true;
            //     };
            //     static A a;
            //     ImGui::Auto(a, "a");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // ImGui::Auto(ImGui::as_const(a), "as_const(a)");// const structs are possible)code");
            //     ImGui::Auto(ImGui::as_const(a), "as_const(a)");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // struct B
            // {
            //  std::string str = "Unfortunatelly, cannot deduce const-ness from within a struct";
            //  const A a = A();
            // };
            // static B b;
            // ImGui::Auto(b, "b");)code");
            //     struct B {
            //         std::string str = "Unfortunatelly, cannot deduce const-ness from within a struct";
            //         const A a = A();
            //     };
            //     static B b;
            //     ImGui::Auto(b, "b");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // static std::vector<B> vec = { {"vector of structs!", A()}, B() };
            // ImGui::Auto(vec, "vec");)code");
            //     static std::vector<B> vec = {{"vector of structs!", A()}, B()};
            //     ImGui::Auto(vec, "vec");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // struct C
            // {
            //  std::list<B> vec;
            //  A *a;
            // };
            // static C c = { {{"Container inside a struct!", A() }}, &a };
            // ImGui::Auto(c, "c");)code");
            //     struct C {
            //         std::list<B> vec;
            //         A *a;
            //     };
            //     static C c = {{{"Container inside a struct!", A()}}, &a};
            //     ImGui::Auto(c, "c");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("Functions")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    void (*func)() = []() { ImGui::TextColored(ImVec4(0.5, 1, 0.5, 1.0), "Button pressed, function called :)"); };
    ImGui::Auto(func, "void(void) function");)code");
            void (*func)() = []() {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5, 1, 0.5, 1.0), "Button pressed, function called :)");
            };
            ImGui::Auto(func, "void(void) function");

            ImGui::Unindent();
        }
    }
    ImGui::End();
}

#endif

namespace neko {

neko_global std::size_t g_imgui_mem_usage = 0;

neko_private(void*) __neko_imgui_malloc(size_t sz, void* user_data) { return __neko_mem_safe_alloc((sz), (char*)__FILE__, __LINE__, &g_imgui_mem_usage); }

neko_private(void) __neko_imgui_free(void* ptr, void* user_data) { __neko_mem_safe_free(ptr, &g_imgui_mem_usage); }

std::size_t __neko_imgui_meminuse() { return g_imgui_mem_usage; }

void neko_imgui_init() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Get main window from platform
    GLFWwindow* win = (GLFWwindow*)platform->raw_window_handle(platform->main_window());

    ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.IniFilename = "imgui.ini";

    io.Fonts->AddFontFromFileTTF(neko_file_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf"), 20.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    neko_imgui_style();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init();
}

void neko_imgui_new_frame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void neko_imgui_render() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // TODO: 将这一切抽象出来并通过命令缓冲系统渲染
    ImGui::Render();
    neko_vec2 fbs = platform->frame_buffer_size(platform->main_window());
    glViewport(0, 0, fbs.x, fbs.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void neko_imgui_destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

}  // namespace neko