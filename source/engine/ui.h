#pragma once

#include <array>
#include <cassert>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "engine/base.h"
#include "engine/draw.h"
#include "engine/ecs.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/luax.h"
#include "engine/math.h"
#include "engine/neko.hpp"
#include "engine/prelude.h"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

NEKO_SCRIPT(
        gui,

        /*
         * get root entity of which all gui entites are descendants
         *
         * this entity's transform is set up so that all its children
         * have screen pixel coordinates and stay in the camera's view
         */
        NEKO_EXPORT Entity gui_get_root();

        // gui

        NEKO_EXPORT void gui_add(Entity ent);

        NEKO_EXPORT void gui_remove(Entity ent);

        NEKO_EXPORT bool gui_has(Entity ent);

        NEKO_EXPORT void gui_set_color(Entity ent, Color color);

        NEKO_EXPORT Color gui_get_color(Entity ent);

        NEKO_EXPORT void gui_set_visible(Entity ent, bool visible);

        NEKO_EXPORT bool gui_get_visible(Entity ent);

        NEKO_EXPORT void gui_set_focusable(Entity ent, bool focusable);

        NEKO_EXPORT bool gui_get_focusable(Entity ent);

        NEKO_EXPORT void gui_set_captures_events(Entity ent, bool captures_events);

        NEKO_EXPORT bool gui_get_captures_events(Entity ent);

        typedef enum GuiAlign GuiAlign; enum GuiAlign{
                GA_MIN = 0,    // h: left, v: bottom
                GA_MID = 1,    // h: center, v: center
                GA_MAX = 2,    // h: right, v: top
                GA_TABLE = 3,  // h: left-right table, v: top-down table
                GA_NONE = 4,   // manual position
        };

        NEKO_EXPORT void gui_set_halign(Entity ent, GuiAlign align);

        NEKO_EXPORT GuiAlign gui_get_halign(Entity ent);

        NEKO_EXPORT void gui_set_valign(Entity ent, GuiAlign align);

        NEKO_EXPORT GuiAlign gui_get_valign(Entity ent);

        NEKO_EXPORT void gui_set_padding(Entity ent, LuaVec2 padding);  // h, v

        NEKO_EXPORT LuaVec2 gui_get_padding(Entity ent);  // h, v

        // entity_nil for no focus
        NEKO_EXPORT void gui_set_focused_entity(Entity ent); NEKO_EXPORT Entity gui_get_focused_entity(); NEKO_EXPORT void gui_set_focus(Entity ent, bool focus);
        NEKO_EXPORT bool gui_get_focus(Entity ent); NEKO_EXPORT bool gui_has_focus();  // whether any gui is focused

        NEKO_EXPORT void gui_fire_event_changed(Entity ent);

        NEKO_EXPORT bool gui_event_focus_enter(Entity ent); NEKO_EXPORT bool gui_event_focus_exit(Entity ent); NEKO_EXPORT bool gui_event_changed(Entity ent);  // input value changed
        NEKO_EXPORT MouseCode gui_event_mouse_down(Entity ent); NEKO_EXPORT MouseCode gui_event_mouse_up(Entity ent); NEKO_EXPORT KeyCode gui_event_key_down(Entity ent);
        NEKO_EXPORT KeyCode gui_event_key_up(Entity ent);

        // whether some gui element captured the current event
        NEKO_EXPORT bool gui_captured_event();

        // gui_rect

        NEKO_EXPORT void gui_rect_add(Entity ent); NEKO_EXPORT void gui_rect_remove(Entity ent); NEKO_EXPORT bool gui_rect_has(Entity ent);

        NEKO_EXPORT void gui_rect_set_size(Entity ent, LuaVec2 size); NEKO_EXPORT LuaVec2 gui_rect_get_size(Entity ent);

        NEKO_EXPORT void gui_rect_set_hfit(Entity ent, bool fit); NEKO_EXPORT bool gui_rect_get_hfit(Entity ent); NEKO_EXPORT void gui_rect_set_vfit(Entity ent, bool fit);
        NEKO_EXPORT bool gui_rect_get_vfit(Entity ent);

        NEKO_EXPORT void gui_rect_set_hfill(Entity ent, bool fill); NEKO_EXPORT bool gui_rect_get_hfill(Entity ent); NEKO_EXPORT void gui_rect_set_vfill(Entity ent, bool fill);
        NEKO_EXPORT bool gui_rect_get_vfill(Entity ent);

        // gui_text

        NEKO_EXPORT void gui_text_add(Entity ent); NEKO_EXPORT void gui_text_remove(Entity ent); NEKO_EXPORT bool gui_text_has(Entity ent);

        NEKO_EXPORT void gui_text_set_str(Entity ent, const char* str); NEKO_EXPORT const char* gui_text_get_str(Entity ent); NEKO_EXPORT void gui_text_set_cursor(Entity ent, int cursor);

        // gui_textedit

        NEKO_EXPORT void gui_textedit_add(Entity ent); NEKO_EXPORT void gui_textedit_remove(Entity ent); NEKO_EXPORT bool gui_textedit_has(Entity ent);

        NEKO_EXPORT void gui_textedit_set_cursor(Entity ent, unsigned int cursor); NEKO_EXPORT unsigned int gui_textedit_get_cursor(Entity ent);

        NEKO_EXPORT void gui_textedit_set_numerical(Entity ent, bool numerical); NEKO_EXPORT bool gui_textedit_get_numerical(Entity ent);
        NEKO_EXPORT Scalar gui_textedit_get_num(Entity ent);  // 0 if not numerical

)

void gui_event_clear();

void gui_init();
void gui_fini();
void gui_update_all();
void gui_draw_all();
void gui_key_down(KeyCode key);
void gui_key_up(KeyCode key);
void gui_char_down(unsigned int c);
void gui_mouse_down(MouseCode mouse);
void gui_mouse_up(MouseCode mouse);
void gui_save_all(Store* s);
void gui_load_all(Store* s);

#define neko_imgui_tree_max_elementsize sizeof(std::string)
#define neko_imgui_tree_max_tuple 3

inline ImVec4 rgba_to_imvec(int r, int g, int b, int a = 255) {
    f32 newr = r / 255.f;
    f32 newg = g / 255.f;
    f32 newb = b / 255.f;
    f32 newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

namespace neko::cpp {
// same as std::as_const in c++17
template <class T>
constexpr std::add_const_t<T>& as_const(T& t) noexcept {
    return t;
}
}  // namespace neko::cpp

namespace neko::imgui {

// 这就是这个库实现的功能 只是类 neko::imgui::Auto_t<T> 的包装
template <typename T>
void Auto(T& anything, const std::string& name = std::string());

namespace detail {

template <typename T>
bool AutoExpand(const std::string& name, T& value);
template <typename Container>
bool AutoContainerTreeNode(const std::string& name, Container& cont);
template <typename Container>
bool AutoContainerValues(const std::string& name,
                         Container& cont);  // Container must have .size(), .begin() and .end() methods and ::value_type.
template <typename Container>
bool AutoMapContainerValues(const std::string& name,
                            Container& map);  // Same as above but that iterates over pairs
template <typename Container>
void AutoContainerPushFrontButton(Container& cont);
template <typename Container>
void AutoContainerPushBackButton(Container& cont);
template <typename Container>
void AutoContainerPopFrontButton(Container& cont);
template <typename Container>
void AutoContainerPopBackButton(Container& cont);
template <typename Key, typename var>
void AutoMapKeyValue(Key& key, var& value);

template <std::size_t I, typename... Args>
void AutoTupleRecurse(std::tuple<Args...>& tpl, std::enable_if_t<0 != I>* = 0);
template <std::size_t I, typename... Args>
inline void AutoTupleRecurse(std::tuple<Args...>& tpl, std::enable_if_t<0 == I>* = 0) {}  // End of recursion.
template <std::size_t I, typename... Args>
void AutoTupleRecurse(const std::tuple<Args...>& tpl, std::enable_if_t<0 != I>* = 0);
template <std::size_t I, typename... Args>
inline void AutoTupleRecurse(const std::tuple<Args...>& tpl, std::enable_if_t<0 == I>* = 0) {}  // End of recursion.
template <typename... Args>
void AutoTuple(const std::string& name, std::tuple<Args...>& tpl);
template <typename... Args>
void AutoTuple(const std::string& name, const std::tuple<Args...>& tpl);

template <typename T, std::size_t N>
using c_array_t = T[N];  // so arrays are regular types and can be used in macro

}  // namespace detail

template <typename T>
struct Auto_t {
    static void Auto(T& anything, const std::string& name) {
        // auto tuple = neko::cpp::pfr::structure_tie(anything);
        // neko::imgui::detail::AutoTuple("Struct " + name, tuple);
        static_assert("Auto not support struct!");
    }
};
}  // namespace neko::imgui

template <typename T>
inline void neko::imgui::Auto(T& anything, const std::string& name) {
    neko::imgui::Auto_t<T>::Auto(anything, name);
}

template <typename T>
bool neko::imgui::detail::AutoExpand(const std::string& name, T& value) {
    if (sizeof(T) <= neko_imgui_tree_max_elementsize) {
        ImGui::PushID(name.c_str());
        ImGui::Bullet();
        neko::imgui::Auto_t<T>::Auto(value, name);
        ImGui::PopID();
        return true;
    } else if (ImGui::TreeNode(name.c_str())) {
        neko::imgui::Auto_t<T>::Auto(value, name);
        ImGui::TreePop();
        return true;
    } else
        return false;
}

template <typename Container>
bool neko::imgui::detail::AutoContainerTreeNode(const std::string& name, Container& cont) {
    // std::size_t size = neko::imgui::detail::AutoContainerSize(cont);
    std::size_t size = cont.size();
    if (ImGui::CollapsingHeader(name.c_str())) {
        size_t elemsize = sizeof(decltype(*std::begin(cont)));
        ImGui::Text("大小: %d, 非动态元素大小: %d bytes", (int)size, (int)elemsize);
        return true;
    } else {
        f32 label_width = ImGui::CalcTextSize(name.c_str()).x + ImGui::GetTreeNodeToLabelSpacing() + 5;
        std::string sizetext = "(大小 = " + std::to_string(size) + ')';
        f32 sizet_width = ImGui::CalcTextSize(sizetext.c_str()).x;
        f32 avail_width = ImGui::GetContentRegionAvail().x;
        if (avail_width > label_width + sizet_width) {
            ImGui::SameLine(avail_width - sizet_width);
            ImGui::TextUnformatted(sizetext.c_str());
        }
        return false;
    }
}
template <typename Container>
bool neko::imgui::detail::AutoContainerValues(const std::string& name, Container& cont) {
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        ImGui::PushID(name.c_str());
        std::size_t i = 0;
        for (auto& elem : cont) {
            std::string itemname = "[" + std::to_string(i) + ']';
            neko::imgui::detail::AutoExpand(itemname, elem);
            ++i;
        }
        ImGui::PopID();
        ImGui::Unindent();
        return true;
    } else
        return false;
}
template <typename Container>
bool neko::imgui::detail::AutoMapContainerValues(const std::string& name, Container& cont) {
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        std::size_t i = 0;
        for (auto& elem : cont) {
            ImGui::PushID(i);
            AutoMapKeyValue(elem.first, elem.second);
            ImGui::PopID();
            ++i;
        }
        ImGui::Unindent();
        return true;
    } else
        return false;
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushFrontButton(Container& cont) {
    if (ImGui::SmallButton("Push Front")) cont.emplace_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushBackButton(Container& cont) {
    if (ImGui::SmallButton("Push Back ")) cont.emplace_back();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopFrontButton(Container& cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Front ")) cont.pop_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopBackButton(Container& cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Back  ")) cont.pop_back();
}
template <typename Key, typename var>
void neko::imgui::detail::AutoMapKeyValue(Key& key, var& value) {
    bool b_k = sizeof(Key) <= neko_imgui_tree_max_elementsize;
    bool b_v = sizeof(var) <= neko_imgui_tree_max_elementsize;
    if (b_k) {
        ImGui::TextUnformatted("[");
        ImGui::SameLine();
        neko::imgui::Auto_t<Key>::Auto(key, "");
        ImGui::SameLine();
        ImGui::TextUnformatted("]");
        if (b_v) ImGui::SameLine();
        neko::imgui::Auto_t<var>::Auto(value, "Value");
    } else {
        neko::imgui::Auto_t<Key>::Auto(key, "Key");
        neko::imgui::Auto_t<var>::Auto(value, "Value");
    }
}

template <std::size_t I, typename... Args>
void neko::imgui::detail::AutoTupleRecurse(std::tuple<Args...>& tpl, std::enable_if_t<0 != I>*) {
    neko::imgui::detail::AutoTupleRecurse<I - 1, Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + (std::is_const_v<type> ? "const " : "") + typeid(type).name();
    neko::imgui::detail::AutoExpand(str, std::get<I - 1>(tpl));
}
template <std::size_t I, typename... Args>
void neko::imgui::detail::AutoTupleRecurse(const std::tuple<Args...>& tpl, std::enable_if_t<0 != I>*) {
    neko::imgui::detail::AutoTupleRecurse<I - 1, const Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + "const " + typeid(type).name();
    neko::imgui::detail::AutoExpand(str, neko::cpp::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string& name, std::tuple<Args...>& tpl) {
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " (" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string& name,
                                    const std::tuple<Args...>& tpl)  // same but const
{
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " !(" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}

// 在此版本中将 templatespec 和 typespec 括在括号中
#define DEFINE_IMGUI_BEGIN_P(templatespec, typespec)                     \
    namespace neko::imgui {                                              \
    NEKO_VA_UNPACK templatespec struct Auto_t<NEKO_VA_UNPACK typespec> { \
        static void Auto(NEKO_VA_UNPACK typespec& var, const std::string& name) {

// 如果宏参数内部没有逗号 请使用不带括号的此版本
#define DEFINE_IMGUI_BEGIN(templatespec, typespec) DEFINE_IMGUI_BEGIN_P((templatespec), (typespec))

#define DEFINE_IMGUI_END() \
    }                      \
    }                      \
    ;                      \
    }

#define neko_imgui_def_inline_p(template_spec, type_spec, code) DEFINE_IMGUI_BEGIN_P(template_spec, type_spec) code DEFINE_IMGUI_END()

#define neko_imgui_def_inline(template_spec, type_spec, code) neko_imgui_def_inline_p((template_spec), (type_spec), code)

#define NEKO_GUI_NULLPTR_COLOR ImVec4(1.0, 0.5, 0.5, 1.0)

DEFINE_IMGUI_BEGIN(template <>, const_str) if (name.empty()) ImGui::TextUnformatted(var);
else ImGui::Text("%s=%s", name.c_str(), var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN_P((template <std::size_t N>), (const detail::c_array_t<char, N>)) if (name.empty()) ImGui::TextUnformatted(var, var + N - 1);
else ImGui::Text("%s=%s", name.c_str(), var);
DEFINE_IMGUI_END();

neko_imgui_def_inline(template <>, char*, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, char* const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, const_str const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

DEFINE_IMGUI_BEGIN(template <>, std::string) const std::size_t lines = var.find('\n');
if (var.find('\n') != std::string::npos)
    ImGui::InputTextMultiline(name.c_str(), const_cast<char*>(var.c_str()), 256);
else
    ImGui::InputText(name.c_str(), const_cast<char*>(var.c_str()), 256);
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN(template <>, const std::string) if (name.empty()) ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
else ImGui::Text("%s=%s", name.c_str(), var.c_str());
DEFINE_IMGUI_END();

neko_imgui_def_inline(template <>, float, ImGui::DragFloat(name.c_str(), &var););
neko_imgui_def_inline(template <>, int, ImGui::InputInt(name.c_str(), &var););
neko_imgui_def_inline(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int*)&var););
neko_imgui_def_inline(template <>, bool, ImGui::Checkbox(name.c_str(), &var););
neko_imgui_def_inline(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
neko_imgui_def_inline(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
neko_imgui_def_inline(template <>, const float, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const int, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const unsigned, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const bool, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const ImVec2, ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
neko_imgui_def_inline(template <>, const ImVec4, ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

#define INTERNAL_NUM(_c, _imn)                                                                             \
    neko_imgui_def_inline(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
    neko_imgui_def_inline(template <>, const _c, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name);)

INTERNAL_NUM(u8, U8);
INTERNAL_NUM(u16, U16);
INTERNAL_NUM(u64, U64);
INTERNAL_NUM(i8, S8);
INTERNAL_NUM(i16, S16);
INTERNAL_NUM(i64, S64);

neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 1>), ImGui::DragFloat(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 1>), ImGui::Text("%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 2>), ImGui::DragFloat2(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 2>), ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 3>), ImGui::DragFloat3(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 3>), ImGui::Text("%s(%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 4>), ImGui::DragFloat4(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 4>), ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 1>), ImGui::InputInt(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 1>), ImGui::Text("%s%d", (name.empty() ? "" : name + "=").c_str(), var[0]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 2>), ImGui::InputInt2(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 2>), ImGui::Text("%s(%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 3>), ImGui::InputInt3(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 3>), ImGui::Text("%s(%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 4>), ImGui::InputInt4(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 4>), ; ImGui::Text("%s(%d,%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

DEFINE_IMGUI_BEGIN(template <typename T>, T*) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, T* const) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
DEFINE_IMGUI_END();

neko_imgui_def_inline_p((template <typename T, std::size_t N>), (std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(std::array<T, N>*)(&var)););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(const std::array<T, N>*)(&var)););

DEFINE_IMGUI_BEGIN_P((template <typename T1, typename T2>), (std::pair<T1, T2>))
if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>) && (std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
    float width = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(width * 0.4 - 10);  // a bit less than half
    neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    ImGui::SameLine();
    neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
    ImGui::PopItemWidth();
} else {
    neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
}

DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN_P((template <typename T1, typename T2>), (const std::pair<T1, T2>)) neko::imgui::detail::AutoExpand<const T1>(name + ".first", var.first);
if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) ImGui::SameLine();
neko::imgui::detail::AutoExpand<const T2>(name + ".second", var.second);
DEFINE_IMGUI_END();

neko_imgui_def_inline(template <typename... Args>, std::tuple<Args...>, neko::imgui::detail::AutoTuple("Tuple " + name, var););
neko_imgui_def_inline(template <typename... Args>, const std::tuple<Args...>, neko::imgui::detail::AutoTuple("Tuple " + name, var););

DEFINE_IMGUI_BEGIN(template <typename T>, std::vector<T>) if (neko::imgui::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <>, std::vector<bool>) if (neko::imgui::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        bool b = var[i];
        ImGui::Bullet();
        neko::imgui::Auto_t<bool>::Auto(b, '[' + std::to_string(i) + ']');
        var[i] = b;
    }
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
    ImGui::Unindent();
}
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, const std::vector<T>) neko::imgui::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <>, const std::vector<bool>) if (neko::imgui::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        ImGui::Bullet();
        neko::imgui::Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
    ImGui::Unindent();
}
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, std::list<T>) if (neko::imgui::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, const std::list<T>) neko::imgui::detail::AutoContainerValues<const std::list<T>>("List " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, std::deque<T>) if (neko::imgui::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, const std::deque<T>) neko::imgui::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, std::forward_list<T>) if (neko::imgui::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN(template <typename T>, const std::forward_list<T>) neko::imgui::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, std::set<T>) neko::imgui::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
// todo insert
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN(template <typename T>, const std::set<T>) neko::imgui::detail::AutoContainerValues<const std::set<T>>("Set " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
// todo insert
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN(template <typename T>, const std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
// todo insert
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (const std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var);
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
// todo insert
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (const std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name, var);
DEFINE_IMGUI_END();

neko_imgui_def_inline(template <>, std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););
neko_imgui_def_inline(template <>, const std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););

// DEFINE_IMGUI_BEGIN(template <>, vec2_t) {
//     //    neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { neko::imgui::Auto(value, std::string(field.name)); });
//     ImGui::Text("%f %f", var.x, var.y);
// }
// DEFINE_IMGUI_END();

namespace neko::imgui {

inline ImVec4 neko_rgba2imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

inline void neko_imgui_help_marker(const_str desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

#if 1
inline void neko_imgui_file_browser(std::string& path) {

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
#endif

inline bool toggle(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    f32 height = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;

    f32 width = height * 2.f;
    f32 radius = height * 0.50f;

    const ImRect total_bb(pos, pos + ImVec2(width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id)) return false;

    f32 last_active_id_timer = g.LastActiveIdTimer;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
        g.LastActiveIdTimer = 0.f;
    }

    if (g.LastActiveIdTimer == 0.f && g.LastActiveId == id && !pressed) g.LastActiveIdTimer = last_active_id_timer;

    f32 t = *v ? 1.0f : 0.0f;

    if (g.LastActiveId == id) {
        f32 t_anim = ImSaturate(g.LastActiveIdTimer / 0.16f);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    const ImRect frame_bb(pos, pos + ImVec2(width, height));

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, col_bg, true, height * 0.5f);
    ImGui::RenderNavHighlight(total_bb, id);

    ImVec2 label_pos = ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y);
    ImGui::RenderText(label_pos, label);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius), radius - 1.5f, ImGui::GetColorU32(ImGuiCol_CheckMark), 36);

    return pressed;
}

inline bool button_scrollable_ex(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool is_pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

    const f32 offset = size.x >= label_size.x + style.FramePadding.x * 2.0f ? size.x + style.FramePadding.x
                                                                            : static_cast<int>(g.Time * 60.f) % static_cast<int>(label_size.x + size.x + style.FramePadding.x * 2.f + 4);
    const ImRect text = ImRect(ImVec2(bb.Min.x + size.x - offset + style.FramePadding.x * 2.f, bb.Min.y + style.FramePadding.y), bb.Max - style.FramePadding);

    ImGui::RenderTextClipped(text.Min, text.Max, label, NULL, &label_size, size.x >= label_size.x + style.FramePadding.x * 2.0f ? g.Style.ButtonTextAlign : ImVec2(0, 0), &bb);
    return is_pressed;
}

inline bool button_scrollable(const char* label, const ImVec2& size_arg) { return button_scrollable_ex(label, size_arg, ImGuiButtonFlags_None); }

struct InputTextCallback_UserData {
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = user_data->Str;
        neko_assert(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    } else if (user_data->ChainCallback) {
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

inline bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

inline bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                               void* user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

inline bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

template <typename T, typename... Args>
inline void TextFmt(T&& fmt, const Args&... args) {
    std::string str = std::format(std::forward<T>(fmt), args...);
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}

}  // namespace neko::imgui

namespace neko::imgui::util {

struct TableInteger {
    const char* name;
    lua_Integer value;
};

using GenerateAny = void (*)(lua_State* L);
struct TableAny {
    const char* name;
    GenerateAny value;
};

struct strbuf {
    char* data;
    size_t size;
};

struct input_context {
    lua_State* L;
    int callback;
};

lua_Integer field_tointeger(lua_State* L, int idx, lua_Integer i);
lua_Number field_tonumber(lua_State* L, int idx, lua_Integer i);
bool field_toboolean(lua_State* L, int idx, lua_Integer i);
ImTextureID get_texture_id(lua_State* L, int idx);
const char* format(lua_State* L, int idx);
strbuf* strbuf_create(lua_State* L, int idx);
strbuf* strbuf_get(lua_State* L, int idx);
int input_callback(ImGuiInputTextCallbackData* data);
void create_table(lua_State* L, std::span<TableInteger> l);
void set_table(lua_State* L, std::span<TableAny> l);
void struct_gen(lua_State* L, const char* name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters);
void flags_gen(lua_State* L, const char* name);
void init(lua_State* L);

}  // namespace neko::imgui::util

void imgui_init();
void imgui_fini();
void imgui_draw_pre();
void imgui_draw_post();

#ifndef NEKO_UI
#define NEKO_UI

// Modified from microui(https://github.com/rxi/microui) and ImGui(https://github.com/ocornut/imgui)

#define UI_SPLIT_SIZE 2.f
#define UI_MAX_CNT 48
#define UI_COMMANDLIST_SIZE (256 * 1024)
#define UI_ROOTLIST_SIZE 32
#define UI_CONTAINERSTACK_SIZE 32
#define UI_CLIPSTACK_SIZE 32
#define UI_IDSTACK_SIZE 32
#define UI_LAYOUTSTACK_SIZE 16
#define UI_CONTAINERPOOL_SIZE 48
#define UI_TREENODEPOOL_SIZE 48
#define UI_UI_SPLIT_SIZE 32
#define UI_UI_TAB_SIZE 32
#define UI_MAX_WIDTHS 16
#define UI_REAL f32
#define UI_REAL_FMT "%.3g"
#define UI_SLIDER_FMT "%.2f"
#define UI_MAX_FMT 127
#define UI_TAB_ITEM_MAX 24
#define UI_CLS_SELECTOR_MAX 4

#ifdef __cplusplus
#define ui_widths(...)                            \
    [&]() -> const i32* {                         \
        static i32 temp_widths[] = {__VA_ARGS__}; \
        return temp_widths;                       \
    }()
#else
#define ui_widths(...) \
    (int[]) { __VA_ARGS__ }
#endif

#define ui_stack(T, n) \
    struct {           \
        i32 idx;       \
        T items[n];    \
    }

enum { UI_CLIP_PART = 1, UI_CLIP_ALL };

enum { UI_COMMAND_JUMP = 1, UI_COMMAND_CLIP, UI_COMMAND_SHAPE, UI_COMMAND_TEXT, UI_COMMAND_ICON, UI_COMMAND_IMAGE, UI_COMMAND_CUSTOM, UI_COMMAND_PIPELINE, UI_COMMAND_UNIFORMS, UI_COMMAND_MAX };

enum { UI_SHAPE_RECT = 1, UI_SHAPE_CIRCLE, UI_SHAPE_TRIANGLE, UI_SHAPE_LINE };

enum { UI_TRANSFORM_WORLD = 0x00, UI_TRANSFORM_LOCAL };

enum { UI_GIZMO_TRANSLATE = 0x00, UI_GIZMO_ROTATE, UI_GIZMO_SCALE };

enum {
    UI_COLOR_BACKGROUND = 0x00,
    UI_COLOR_CONTENT,
    UI_COLOR_BORDER,
    UI_COLOR_SHADOW,
    UI_COLOR_CONTENT_BACKGROUND,
    UI_COLOR_CONTENT_SHADOW,
    UI_COLOR_CONTENT_BORDER,

    UI_COLOR_MAX
};

enum { UI_ICON_CLOSE = 1, UI_ICON_CHECK, UI_ICON_COLLAPSED, UI_ICON_EXPANDED, UI_ICON_MAX };

enum { UI_RES_ACTIVE = (1 << 0), UI_RES_SUBMIT = (1 << 1), UI_RES_CHANGE = (1 << 2) };

typedef enum ui_alt_drag_mode_type {
    UI_ALT_DRAG_QUAD = 0x00,  // Quadrants
    UI_ALT_DRAG_NINE,         // Nine splice the window
    UI_ALT_DRAG_SINGLE        // Single window drag (controls the width/height, leaving x/y position of window in tact)
} ui_alt_drag_mode_type;

enum {
    UI_OPT_NOSTYLESHADOW = (1ULL << 0),
    UI_OPT_NOSTYLEBORDER = (1ULL << 1),
    UI_OPT_NOINTERACT = (1ULL << 2),
    UI_OPT_NOFRAME = (1ULL << 3),
    UI_OPT_NORESIZE = (1ULL << 4),
    UI_OPT_NOSCROLL = (1ULL << 5),
    UI_OPT_NOCLOSE = (1ULL << 6),
    UI_OPT_NOTITLE = (1ULL << 7),
    UI_OPT_HOLDFOCUS = (1ULL << 8),
    UI_OPT_AUTOSIZE = (1ULL << 9),
    UI_OPT_POPUP = (1ULL << 10),
    UI_OPT_CLOSED = (1ULL << 11),
    UI_OPT_EXPANDED = (1ULL << 12),
    UI_OPT_NOHOVER = (1ULL << 13),
    UI_OPT_FORCESETRECT = (1ULL << 14),
    UI_OPT_NOFOCUS = (1ULL << 15),
    UI_OPT_FORCEFOCUS = (1ULL << 16),
    UI_OPT_NOMOVE = (1ULL << 17),
    UI_OPT_NOCLIP = (1ULL << 18),
    UI_OPT_NODOCK = (1ULL << 19),
    UI_OPT_FULLSCREEN = (1ULL << 20),
    UI_OPT_DOCKSPACE = (1ULL << 21),
    UI_OPT_NOBRINGTOFRONT = (1ULL << 22),
    UI_OPT_LEFTCLICKONLY = (1ULL << 23),
    UI_OPT_NOSWITCHSTATE = (1ULL << 24),
    UI_OPT_NOBORDER = (1ULL << 25),
    UI_OPT_ISCONTENT = (1ULL << 26),
    UI_OPT_NOCARET = (1ULL << 27),
    UI_OPT_NOSCROLLHORIZONTAL = (1ULL << 28),
    UI_OPT_NOSCROLLVERTICAL = (1ULL << 29),
    UI_OPT_NOSTYLEBACKGROUND = (1ULL << 30),
    UI_OPT_PARSEIDTAGONLY = (1ULL << 31)
};

enum { UI_MOUSE_LEFT = (1 << 0), UI_MOUSE_RIGHT = (1 << 1), UI_MOUSE_MIDDLE = (1 << 2) };

enum { UI_KEY_SHIFT = (1 << 0), UI_KEY_CTRL = (1 << 1), UI_KEY_ALT = (1 << 2), UI_KEY_BACKSPACE = (1 << 3), UI_KEY_RETURN = (1 << 4) };

#define UI_OPT_NOSTYLING (UI_OPT_NOSTYLEBORDER | UI_OPT_NOSTYLEBACKGROUND | UI_OPT_NOSTYLESHADOW)

typedef struct ui_context_t ui_context_t;
typedef u32 ui_id;
typedef UI_REAL ui_real;

struct FontFamily;

// Shapes
typedef struct {
    float x, y, w, h;
} ui_rect_t;
typedef struct {
    float radius;
    vec2 center;
} ui_circle_t;
typedef struct {
    vec2 points[3];
} ui_triangle_t;
typedef struct {
    vec2 start;
    vec2 end;
} ui_line_t;

typedef struct {
    ui_id id;
    i32 last_update;
} ui_pool_item_t;

typedef struct {
    i32 type, size;
} ui_basecommand_t;

typedef struct {
    ui_basecommand_t base;
    void* dst;
} ui_jumpcommand_t;

typedef struct {
    ui_basecommand_t base;
    ui_rect_t rect;
} ui_clipcommand_t;

typedef struct {
    ui_basecommand_t base;
    FontFamily* font;
    vec2 pos;
    Color256 color;
    char str[1];
} ui_textcommand_t;

typedef struct {
    ui_basecommand_t base;
    neko_handle(gfx_pipeline_t) pipeline;
    neko_idraw_layout_type layout_type;
    void* layout;
    size_t layout_sz;
} ui_pipelinecommand_t;

typedef struct {
    ui_basecommand_t base;
    void* data;
    size_t sz;
} ui_binduniformscommand_t;

typedef struct {
    ui_basecommand_t base;
    ui_rect_t rect;
    neko_handle(gfx_texture_t) hndl;
    vec4 uvs;
    Color256 color;
} ui_imagecommand_t;

struct ui_customcommand_t;

// Draw Callback
typedef void (*ui_draw_callback_t)(ui_context_t* ctx, struct ui_customcommand_t* cmd);

typedef struct ui_customcommand_t {
    ui_basecommand_t base;
    ui_rect_t clip;
    ui_rect_t viewport;
    ui_id hash;
    ui_id hover;
    ui_id focus;
    ui_draw_callback_t cb;
    void* data;
    size_t sz;
} ui_customcommand_t;

typedef struct {
    ui_basecommand_t base;
    u32 type;
    union {
        ui_rect_t rect;
        ui_circle_t circle;
        ui_triangle_t triangle;
        ui_line_t line;
    };
    Color256 color;
} ui_shapecommand_t;

// 注意 考虑到如何将异构类型推入字节缓冲区 这是浪费的
typedef union {
    i32 type;
    ui_basecommand_t base;
    ui_jumpcommand_t jump;
    ui_clipcommand_t clip;
    ui_shapecommand_t shape;
    ui_textcommand_t text;
    ui_imagecommand_t image;
    ui_customcommand_t custom;
    ui_pipelinecommand_t pipeline;
    ui_binduniformscommand_t uniforms;
} ui_command_t;

struct ui_context_t;

typedef void (*ui_on_draw_button_callback)(struct ui_context_t* ctx, ui_rect_t rect, ui_id id, bool hovered, bool focused, u64 opt, const char* label, i32 icon);

typedef enum {
    UI_LAYOUT_ANCHOR_TOPLEFT = 0x00,
    UI_LAYOUT_ANCHOR_TOPCENTER,
    UI_LAYOUT_ANCHOR_TOPRIGHT,
    UI_LAYOUT_ANCHOR_LEFT,
    UI_LAYOUT_ANCHOR_CENTER,
    UI_LAYOUT_ANCHOR_RIGHT,
    UI_LAYOUT_ANCHOR_BOTTOMLEFT,
    UI_LAYOUT_ANCHOR_BOTTOMCENTER,
    UI_LAYOUT_ANCHOR_BOTTOMRIGHT
} ui_layout_anchor_type;

typedef enum { UI_ALIGN_START = 0x00, UI_ALIGN_CENTER, UI_ALIGN_END } ui_align_type;

typedef enum { UI_JUSTIFY_START = 0x00, UI_JUSTIFY_CENTER, UI_JUSTIFY_END } ui_justification_type;

typedef enum { UI_DIRECTION_COLUMN = 0x00, UI_DIRECTION_ROW, UI_DIRECTION_COLUMN_REVERSE, UI_DIRECTION_ROW_REVERSE } ui_direction;

typedef struct ui_layout_t {
    ui_rect_t body;
    ui_rect_t next;
    vec2 position;
    vec2 size;
    vec2 max;
    i32 padding[4];
    i32 widths[UI_MAX_WIDTHS];
    i32 items;
    i32 item_index;
    i32 next_row;
    i32 next_type;
    i32 indent;

    // flex direction / justification / alignment
    i32 direction;
    i32 justify_content;
    i32 align_content;

} ui_layout_t;

// Forward decl.
struct ui_container_t;

typedef enum ui_split_node_type { UI_SPLIT_NODE_CONTAINER = 0x00, UI_SPLIT_NODE_SPLIT } ui_split_node_type;

enum { UI_SPLIT_NODE_CHILD = 0x00, UI_SPLIT_NODE_PARENT };

typedef struct ui_split_node_t {
    ui_split_node_type type;
    union {
        u32 split;
        struct ui_container_t* container;
    };
} ui_split_node_t;

typedef enum ui_split_type { UI_SPLIT_LEFT = 0x00, UI_SPLIT_RIGHT, UI_SPLIT_TOP, UI_SPLIT_BOTTOM, UI_SPLIT_TAB } ui_split_type;

typedef struct ui_split_t {
    ui_split_type type;  // UI_SPLIT_LEFT, UI_SPLIT_RIGHT, UI_SPLIT_TAB, UI_SPLIT_BOTTOM, UI_SPLIT_TOP
    float ratio;         // Split ratio between children [0.f, 1.f], (left node = ratio), right node = (1.f - ratio)
    ui_rect_t rect;
    ui_rect_t prev_rect;
    ui_split_node_t children[2];
    u32 parent;
    u32 id;
    u32 zindex;
    i32 frame;
} ui_split_t;

typedef enum ui_window_flags {
    UI_WINDOW_FLAGS_VISIBLE = (1 << 0),     //
    UI_WINDOW_FLAGS_FIRST_INIT = (1 << 1),  //
    UI_WINDOW_FLAGS_PUSH_ID = (1 << 2)      //
} ui_window_flags;

// Equidistantly sized tabs, based on rect of window
typedef struct ui_tab_item_t {
    ui_id tab_bar;
    u32 zindex;  // Sorting index in tab bar
    void* data;  // User set data pointer for this item
    u32 idx;     // Internal index
} ui_tab_item_t;

typedef struct ui_tab_bar_t {
    ui_tab_item_t items[UI_TAB_ITEM_MAX];
    u32 size;        // Current number of items in tab bar
    ui_rect_t rect;  // Cached sized for tab bar
    u32 focus;       // Focused item in tab bar
} ui_tab_bar_t;

typedef struct ui_container_t {
    ui_command_t *head, *tail;
    ui_rect_t rect;
    ui_rect_t body;
    vec2 content_size;
    vec2 scroll;
    i32 zindex;
    i32 open;
    ui_id id;
    ui_id split;  // If container is docked, then will have owning split to get sizing (0x00 for NULL)
    u32 tab_bar;
    u32 tab_item;
    struct ui_container_t* parent;  // Owning parent (for tabbing)
    u64 opt;
    u32 frame;
    u32 visible;
    i32 flags;
    char name[256];
} ui_container_t;

typedef enum {
    UI_ELEMENT_STATE_NEG = -1,
    UI_ELEMENT_STATE_DEFAULT = 0x00,
    UI_ELEMENT_STATE_HOVER,
    UI_ELEMENT_STATE_FOCUS,
    UI_ELEMENT_STATE_COUNT,
    UI_ELEMENT_STATE_ON_HOVER,
    UI_ELEMENT_STATE_ON_FOCUS,
    UI_ELEMENT_STATE_OFF_HOVER,
    UI_ELEMENT_STATE_OFF_FOCUS
} ui_element_state;

typedef enum ui_element_type {
    UI_ELEMENT_CONTAINER = 0x00,
    UI_ELEMENT_LABEL,
    UI_ELEMENT_TEXT,
    UI_ELEMENT_PANEL,
    UI_ELEMENT_INPUT,
    UI_ELEMENT_BUTTON,
    UI_ELEMENT_SCROLL,
    UI_ELEMENT_IMAGE,
    UI_ELEMENT_COUNT
} ui_element_type;

typedef enum { UI_PADDING_LEFT = 0x00, UI_PADDING_RIGHT, UI_PADDING_TOP, UI_PADDING_BOTTOM } ui_padding_type;

typedef enum { UI_MARGIN_LEFT = 0x00, UI_MARGIN_RIGHT, UI_MARGIN_TOP, UI_MARGIN_BOTTOM } ui_margin_type;

typedef enum {

    // Width/Height
    UI_STYLE_WIDTH = 0x00,
    UI_STYLE_HEIGHT,

    // Padding
    UI_STYLE_PADDING,
    UI_STYLE_PADDING_LEFT,
    UI_STYLE_PADDING_RIGHT,
    UI_STYLE_PADDING_TOP,
    UI_STYLE_PADDING_BOTTOM,

    UI_STYLE_MARGIN,  // Can set margin for all at once, if -1.f then will assume 'auto' to simulate standard css
    UI_STYLE_MARGIN_LEFT,
    UI_STYLE_MARGIN_RIGHT,
    UI_STYLE_MARGIN_TOP,
    UI_STYLE_MARGIN_BOTTOM,

    // Border Radius
    UI_STYLE_BORDER_RADIUS,
    UI_STYLE_BORDER_RADIUS_LEFT,
    UI_STYLE_BORDER_RADIUS_RIGHT,
    UI_STYLE_BORDER_RADIUS_TOP,
    UI_STYLE_BORDER_RADIUS_BOTTOM,

    // Border Width
    UI_STYLE_BORDER_WIDTH,
    UI_STYLE_BORDER_WIDTH_LEFT,
    UI_STYLE_BORDER_WIDTH_RIGHT,
    UI_STYLE_BORDER_WIDTH_TOP,
    UI_STYLE_BORDER_WIDTH_BOTTOM,

    // Text
    UI_STYLE_TEXT_ALIGN,

    // Flex
    UI_STYLE_DIRECTION,
    UI_STYLE_ALIGN_CONTENT,
    UI_STYLE_JUSTIFY_CONTENT,  // Justify runs parallel to direction (ex. for row, left to right)

    // Shadow
    UI_STYLE_SHADOW_X,
    UI_STYLE_SHADOW_Y,

    // Colors
    UI_STYLE_COLOR_BACKGROUND,
    UI_STYLE_COLOR_BORDER,
    UI_STYLE_COLOR_SHADOW,
    UI_STYLE_COLOR_CONTENT,
    UI_STYLE_COLOR_CONTENT_BACKGROUND,
    UI_STYLE_COLOR_CONTENT_BORDER,
    UI_STYLE_COLOR_CONTENT_SHADOW,

    // Font
    UI_STYLE_FONT,

    UI_STYLE_COUNT

} ui_style_element_type;

enum { UI_ANIMATION_DIRECTION_FORWARD = 0x00, UI_ANIMATION_DIRECTION_BACKWARD };

typedef struct {
    ui_style_element_type type;
    union {
        i32 value;
        Color256 color;
        FontFamily* font;
    };
} ui_style_element_t;

typedef struct ui_animation_t {
    i16 max;          // max time
    i16 time;         // current time
    i16 delay;        // delay
    i16 curve;        // curve type
    i16 direction;    // current direction
    i16 playing;      // whether or not active
    i16 iterations;   // number of iterations to play the animation
    i16 focus_state;  // cached focus_state from frame (want to delete this somehow)
    i16 hover_state;  // cached hover_state from frame (want to delete this somehow)
    i16 start_state;  // starting state for animation blend
    i16 end_state;    // ending state for animation blend
    i32 frame;        // current frame (to match)
} ui_animation_t;

typedef struct ui_style_t {
    // font
    FontFamily* font;

    // dimensions
    float size[2];
    i16 spacing;         // get rid of    (use padding)
    i16 indent;          // get rid of    (use margin)
    i16 title_height;    // get rid of    (use title_bar style)
    i16 scrollbar_size;  // get rid of    (use scroll style)
    i16 thumb_size;      // get rid of    (use various styles)

    // colors
    Color256 colors[UI_COLOR_MAX];

    // padding/margin
    i32 padding[4];
    i16 margin[4];

    // border
    i16 border_width[4];
    i16 border_radius[4];

    // flex direction / justification / alignment
    i16 direction;
    i16 justify_content;
    i16 align_content;

    // shadow amount each direction
    i16 shadow_x;
    i16 shadow_y;

} ui_style_t;

// Keep animation properties lists within style sheet to look up

typedef struct ui_animation_property_t {
    ui_style_element_type type;
    i16 time;
    i16 delay;
} ui_animation_property_t;

typedef struct ui_animation_property_list_t {
    neko_dyn_array(ui_animation_property_t) properties[3];
} ui_animation_property_list_t;

/*
   element type
   classes
   id

   ui_button(gui, "Text##.cls#id");
   ui_label(gui, "Title###title");

    button .class #id : hover {         // All of these styles get concat into one?
    }
*/

typedef struct {
    neko_dyn_array(ui_style_element_t) styles[3];
} ui_style_list_t;

typedef struct ui_style_sheet_t {
    ui_style_t styles[UI_ELEMENT_COUNT][3];  // default | hovered | focused
    neko_hash_table(ui_element_type, ui_animation_property_list_t) animations;

    neko_hash_table(u64, ui_style_list_t) cid_styles;
    neko_hash_table(u64, ui_animation_property_list_t) cid_animations;
} ui_style_sheet_t;

typedef struct ui_style_sheet_element_desc_t {

    struct {

        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;

    } all;

    struct {

        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;

    } def;

    struct {
        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;
    } hover;

    struct {
        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;
    } focus;

} ui_style_sheet_element_desc_t;

typedef ui_style_sheet_element_desc_t ui_inline_style_desc_t;

typedef struct ui_style_sheet_desc_t {
    ui_style_sheet_element_desc_t container;
    ui_style_sheet_element_desc_t button;
    ui_style_sheet_element_desc_t panel;
    ui_style_sheet_element_desc_t input;
    ui_style_sheet_element_desc_t text;
    ui_style_sheet_element_desc_t label;
    ui_style_sheet_element_desc_t scroll;
    ui_style_sheet_element_desc_t tab;
    ui_style_sheet_element_desc_t menu;
    ui_style_sheet_element_desc_t title;
    ui_style_sheet_element_desc_t image;
} ui_style_sheet_desc_t;

typedef enum ui_request_type {
    UI_SPLIT_NEW = 0x01,
    UI_CNT_MOVE,
    UI_CNT_FOCUS,
    UI_SPLIT_MOVE,
    UI_SPLIT_RESIZE_SW,
    UI_SPLIT_RESIZE_SE,
    UI_SPLIT_RESIZE_NW,
    UI_SPLIT_RESIZE_NE,
    UI_SPLIT_RESIZE_W,
    UI_SPLIT_RESIZE_E,
    UI_SPLIT_RESIZE_N,
    UI_SPLIT_RESIZE_S,
    UI_SPLIT_RESIZE_CENTER,
    UI_SPLIT_RATIO,
    UI_SPLIT_RESIZE_INVALID,
    UI_TAB_SWAP_LEFT,
    UI_TAB_SWAP_RIGHT
} ui_request_type;

typedef struct ui_request_t {
    ui_request_type type;
    union {
        ui_split_type split_type;
        ui_split_t* split;
        ui_container_t* cnt;
    };
    u32 frame;
} ui_request_t;

typedef struct ui_inline_style_stack_t {
    neko_dyn_array(ui_style_element_t) styles[3];
    neko_dyn_array(ui_animation_property_t) animations[3];
    neko_dyn_array(u32) style_counts;      // amount of styles to pop off at "top of stack" for each state
    neko_dyn_array(u32) animation_counts;  // amount of animations to pop off at "top of stack" for each state
} ui_inline_style_stack_t;

typedef struct ui_context_t {
    // Core state
    ui_style_t* style;              // Active style
    ui_style_sheet_t* style_sheet;  // Active style sheet
    ui_id hover;
    ui_id focus;
    ui_id last_id;
    ui_id state_switch_id;  // Id that had a state switch
    i32 switch_state;
    ui_id lock_focus;
    i32 last_hover_state;
    i32 last_focus_state;
    ui_id prev_hover;
    ui_id prev_focus;
    ui_rect_t last_rect;
    i32 last_zindex;
    i32 updated_focus;
    i32 frame;
    vec2 framebuffer_size;
    ui_rect_t viewport;
    ui_container_t* active_root;
    ui_container_t* hover_root;
    ui_container_t* next_hover_root;
    ui_container_t* scroll_target;
    ui_container_t* focus_root;
    ui_container_t* next_focus_root;
    ui_container_t* dockable_root;
    ui_container_t* prev_dockable_root;
    ui_container_t* docked_root;
    ui_container_t* undock_root;
    ui_split_t* focus_split;
    ui_split_t* next_hover_split;
    ui_split_t* hover_split;
    ui_id next_lock_hover_id;
    ui_id lock_hover_id;
    char number_edit_buf[UI_MAX_FMT];
    ui_id number_edit;
    ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(ui_request_t) requests;

    // Stacks
    ui_stack(u8, UI_COMMANDLIST_SIZE) command_list;
    ui_stack(ui_container_t*, UI_ROOTLIST_SIZE) root_list;
    ui_stack(ui_container_t*, UI_CONTAINERSTACK_SIZE) container_stack;
    ui_stack(ui_rect_t, UI_CLIPSTACK_SIZE) clip_stack;
    ui_stack(ui_id, UI_IDSTACK_SIZE) id_stack;
    ui_stack(ui_layout_t, UI_LAYOUTSTACK_SIZE) layout_stack;

    // Style sheet element stacks
    neko_hash_table(ui_element_type, ui_inline_style_stack_t) inline_styles;

    // Retained state pools
    ui_pool_item_t container_pool[UI_CONTAINERPOOL_SIZE];
    ui_container_t containers[UI_CONTAINERPOOL_SIZE];
    ui_pool_item_t treenode_pool[UI_TREENODEPOOL_SIZE];

    neko_slot_array(ui_split_t) splits;
    neko_slot_array(ui_tab_bar_t) tab_bars;

    // Input state
    vec2 mouse_pos;
    vec2 last_mouse_pos;
    vec2 mouse_delta;
    vec2 scroll_delta;
    i32 mouse_down;
    i32 mouse_pressed;
    i32 key_down;
    i32 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    idraw_t gui_idraw;
    idraw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(ui_id, ui_animation_t) animations;

    // Font stash
    neko_hash_table(u64, FontFamily*) font_stash;

    // Callbacks
    struct {
        ui_on_draw_button_callback button;
    } callbacks;

} ui_context_t;

typedef struct {
    // Core state
    ui_style_t* style;              // Active style
    ui_style_sheet_t* style_sheet;  // Active style sheet
    ui_id hover;
    ui_id focus;
    ui_id last_id;
    ui_id lock_focus;
    ui_rect_t last_rect;
    i32 last_zindex;
    i32 updated_focus;
    i32 frame;
    ui_container_t* hover_root;
    ui_container_t* next_hover_root;
    ui_container_t* scroll_target;
    ui_container_t* focus_root;
    ui_container_t* next_focus_root;
    ui_container_t* dockable_root;
    ui_container_t* prev_dockable_root;
    ui_container_t* docked_root;
    ui_container_t* undock_root;
    ui_split_t* focus_split;
    ui_split_t* next_hover_split;
    ui_split_t* hover_split;
    ui_id next_lock_hover_id;
    ui_id lock_hover_id;
    char number_edit_buf[UI_MAX_FMT];
    ui_id number_edit;
    ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(ui_request_t) requests;

    // Stacks
    ui_stack(ui_container_t*, UI_CONTAINERSTACK_SIZE) container_stack;

    neko_dyn_array(u8) command_list;
    neko_dyn_array(ui_container_t*) root_list;
    neko_dyn_array(ui_rect_t) clip_stack;
    neko_dyn_array(ui_id) id_stack;
    neko_dyn_array(ui_layout_t) layout_stack;

    // Retained state pools
    ui_pool_item_t container_pool[UI_CONTAINERPOOL_SIZE];
    ui_pool_item_t treenode_pool[UI_TREENODEPOOL_SIZE];

    neko_slot_array(ui_split_t) splits;
    neko_slot_array(ui_tab_bar_t) tab_bars;

    // Input state
    vec2 mouse_pos;
    vec2 last_mouse_pos;
    vec2 mouse_delta;
    vec2 scroll_delta;
    i16 mouse_down;
    i16 mouse_pressed;
    i16 key_down;
    i16 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    idraw_t gui_idraw;
    idraw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(ui_id, ui_animation_t) animations;

    // Callbacks
    struct {
        ui_on_draw_button_callback button;
    } callbacks;

} ui_context_pruned_t;

typedef struct {
    const char* key;
    FontFamily* font;
} ui_font_desc_t;

typedef struct {
    ui_font_desc_t* fonts;
    size_t size;
} ui_font_stash_desc_t;

typedef struct {
    const char* id;                            // Id selector
    const char* classes[UI_CLS_SELECTOR_MAX];  // Class selectors
} ui_selector_desc_t;

enum { UI_HINT_FLAG_NO_SCALE_BIAS_MOUSE = (1 << 0), UI_HINT_FLAG_NO_INVERT_Y = (1 << 1) };

typedef struct ui_hints_t {
    vec2 framebuffer_size;  // Overall framebuffer size
    ui_rect_t viewport;          // Viewport within framebuffer for gui context
    i32 flags;                   // Flags for hints
} ui_hints_t;

ui_rect_t ui_rect(float x, float y, float w, float h);

// Context

ui_context_t ui_new(u32 window_hndl);
void ui_init(ui_context_t* ctx, u32 window_hndl);
void ui_init_font_stash(ui_context_t* ctx, ui_font_stash_desc_t* desc);
ui_context_t ui_context_new(u32 window_hndl);
void ui_free(ui_context_t* ctx);
void ui_begin(ui_context_t* ctx, const ui_hints_t* hints);
void ui_end(ui_context_t* ctx, bool update);
void ui_render(ui_context_t* ctx, neko_command_buffer_t* cb);

// Util
void ui_renderpass_submit(ui_context_t* ctx, neko_command_buffer_t* cb, Color256 clear);
void ui_renderpass_submit_ex(ui_context_t* ctx, neko_command_buffer_t* cb, gfx_clear_action_t action);
void ui_parse_id_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz, u64 opt);
void ui_parse_label_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz);

// Main API

void ui_set_focus(ui_context_t* ctx, ui_id id);
void ui_set_hover(ui_context_t* ctx, ui_id id);
ui_id ui_get_id(ui_context_t* ctx, const void* data, i32 size);
void ui_push_id(ui_context_t* ctx, const void* data, i32 size);
void ui_pop_id(ui_context_t* ctx);
void ui_push_clip_rect(ui_context_t* ctx, ui_rect_t rect);
void ui_pop_clip_rect(ui_context_t* ctx);
ui_rect_t ui_get_clip_rect(ui_context_t* ctx);
i32 ui_check_clip(ui_context_t* ctx, ui_rect_t r);
i32 ui_mouse_over(ui_context_t* ctx, ui_rect_t rect);
void ui_update_control(ui_context_t* ctx, ui_id id, ui_rect_t rect, u64 opt);

// Conatiner

ui_container_t* ui_get_current_container(ui_context_t* ctx);
ui_container_t* ui_get_container(ui_context_t* ctx, const char* name);
ui_container_t* ui_get_top_most_container(ui_context_t* ctx, ui_split_t* split);
ui_container_t* ui_get_container_ex(ui_context_t* ctx, ui_id id, u64 opt);
void ui_bring_to_front(ui_context_t* ctx, ui_container_t* cnt);
void ui_bring_split_to_front(ui_context_t* ctx, ui_split_t* split);
ui_split_t* ui_get_split(ui_context_t* ctx, ui_container_t* cnt);
ui_tab_bar_t* ui_get_tab_bar(ui_context_t* ctx, ui_container_t* cnt);
void ui_tab_item_swap(ui_context_t* ctx, ui_container_t* cnt, i32 direction);
ui_container_t* ui_get_root_container(ui_context_t* ctx, ui_container_t* cnt);
ui_container_t* ui_get_root_container_from_split(ui_context_t* ctx, ui_split_t* split);
ui_container_t* ui_get_parent(ui_context_t* ctx, ui_container_t* cnt);
void ui_current_container_close(ui_context_t* ctx);

// Animation

ui_animation_t* ui_get_animation(ui_context_t* ctx, ui_id id, const ui_selector_desc_t* desc, i32 elementid);

ui_style_t ui_animation_get_blend_style(ui_context_t* ctx, ui_animation_t* anim, const ui_selector_desc_t* desc, i32 elementid);

// Style Sheet

ui_style_sheet_t ui_style_sheet_create(ui_context_t* ctx, ui_style_sheet_desc_t* desc);
void ui_style_sheet_fini(ui_style_sheet_t* ss);
void ui_set_element_style(ui_context_t* ctx, ui_element_type element, ui_element_state state, ui_style_element_t* style, size_t size);
void ui_style_sheet_set_element_styles(ui_style_sheet_t* style_sheet, ui_element_type element, ui_element_state state, ui_style_element_t* styles, size_t size);
void ui_set_style_sheet(ui_context_t* ctx, ui_style_sheet_t* style_sheet);

void ui_push_inline_style(ui_context_t* ctx, ui_element_type elementid, ui_inline_style_desc_t* desc);
void ui_pop_inline_style(ui_context_t* ctx, ui_element_type elementid);

ui_style_t* ui_push_style(ui_context_t* ctx, ui_style_t* style);
void ui_pop_style(ui_context_t* ctx, ui_style_t* style);

// Pools

i32 ui_pool_init(ui_context_t* ctx, ui_pool_item_t* items, i32 len, ui_id id);
i32 ui_pool_get(ui_context_t* ctx, ui_pool_item_t* items, i32 len, ui_id id);
void ui_pool_update(ui_context_t* ctx, ui_pool_item_t* items, i32 idx);

// Input

void ui_input_mousemove(ui_context_t* ctx, i32 x, i32 y);
void ui_input_mousedown(ui_context_t* ctx, i32 x, i32 y, i32 btn);
void ui_input_mouseup(ui_context_t* ctx, i32 x, i32 y, i32 btn);
void ui_input_scroll(ui_context_t* ctx, i32 x, i32 y);
void ui_input_keydown(ui_context_t* ctx, i32 key);
void ui_input_keyup(ui_context_t* ctx, i32 key);
void ui_input_text(ui_context_t* ctx, const char* text);

// Commands

ui_command_t* ui_push_command(ui_context_t* ctx, i32 type, i32 size);
i32 ui_next_command(ui_context_t* ctx, ui_command_t** cmd);
void ui_set_clip(ui_context_t* ctx, ui_rect_t rect);
void ui_set_pipeline(ui_context_t* ctx, neko_handle(gfx_pipeline_t) pip, void* layout, size_t layout_sz, neko_idraw_layout_type layout_type);
void ui_bind_uniforms(ui_context_t* ctx, gfx_bind_uniform_desc_t* uniforms, size_t uniforms_sz);

// Drawing

void ui_draw_rect(ui_context_t* ctx, ui_rect_t rect, Color256 color);
void ui_draw_circle(ui_context_t* ctx, vec2 position, float radius, Color256 color);
void ui_draw_triangle(ui_context_t* ctx, vec2 a, vec2 b, vec2 c, Color256 color);
void ui_draw_box(ui_context_t* ctx, ui_rect_t rect, i16* width, Color256 color);
void ui_draw_line(ui_context_t* ctx, vec2 start, vec2 end, Color256 color);
void ui_draw_text(ui_context_t* ctx, FontFamily* font, const char* str, i32 len, vec2 pos, Color256 color, i32 shadow_x, i32 shadow_y, Color256 shadow_color);
void ui_draw_image(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, vec2 uv0, vec2 uv1, Color256 color);
void ui_draw_nine_rect(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, vec2 uv0, vec2 uv1, u32 left, u32 right, u32 top, u32 bottom, Color256 color);
void ui_draw_control_frame(ui_context_t* ctx, ui_id id, ui_rect_t rect, i32 elementid, u64 opt);
void ui_draw_control_text(ui_context_t* ctx, const char* str, ui_rect_t rect, const ui_style_t* style, u64 opt);
void ui_draw_custom(ui_context_t* ctx, ui_rect_t rect, ui_draw_callback_t cb, void* data, size_t sz);

// Layout

ui_layout_t* ui_get_layout(ui_context_t* ctx);
void ui_layout_row(ui_context_t* ctx, i32 items, const i32* widths, i32 height);
void ui_layout_row_ex(ui_context_t* ctx, i32 items, const i32* widths, i32 height, i32 justification);
void ui_layout_width(ui_context_t* ctx, i32 width);
void ui_layout_height(ui_context_t* ctx, i32 height);
void ui_layout_column_begin(ui_context_t* ctx);
void ui_layout_column_end(ui_context_t* ctx);
void ui_layout_set_next(ui_context_t* ctx, ui_rect_t r, i32 relative);
ui_rect_t ui_layout_peek_next(ui_context_t* ctx);
ui_rect_t ui_layout_next(ui_context_t* ctx);
ui_rect_t ui_layout_anchor(const ui_rect_t* parent, i32 width, i32 height, i32 xoff, i32 yoff, ui_layout_anchor_type type);

// Elements

#define ui_button(_CTX, _LABEL) ui_button_ex((_CTX), (_LABEL), NULL, UI_OPT_LEFTCLICKONLY)
#define ui_text(_CTX, _TXT) ui_text_ex((_CTX), (_TXT), 1, NULL, 0x00)
// #define ui_text_fc(_CTX, _TXT) ui_text_fc_ex((_CTX), (_TXT), (-1))
#define ui_textbox(_CTX, _BUF, _BUFSZ) ui_textbox_ex((_CTX), (_BUF), (_BUFSZ), NULL, 0x00)
#define ui_slider(_CTX, _VALUE, _LO, _HI) ui_slider_ex((_CTX), (_VALUE), (_LO), (_HI), 0, UI_SLIDER_FMT, NULL, 0x00)
#define ui_number(_CTX, _VALUE, _STEP) ui_number_ex((_CTX), (_VALUE), (_STEP), UI_SLIDER_FMT, NULL, 0x00)
#define ui_header(_CTX, _LABEL) ui_header_ex((_CTX), (_LABEL), NULL, 0x00)
#define ui_checkbox(_CTX, _LABEL, _STATE) ui_checkbox_ex((_CTX), (_LABEL), (_STATE), NULL, UI_OPT_LEFTCLICKONLY)
#define ui_treenode_begin(_CTX, _LABEL) ui_treenode_begin_ex((_CTX), (_LABEL), NULL, 0x00)
#define ui_window_begin(_CTX, _TITLE, _RECT) ui_window_begin_ex((_CTX), (_TITLE), (_RECT), 0, NULL, 0x00)
#define ui_popup_begin(_CTX, _TITLE, _RECT) ui_popup_begin_ex((_CTX), (_TITLE), (_RECT), NULL, 0x00)
#define ui_panel_begin(_CTX, _NAME) ui_panel_begin_ex((_CTX), (_NAME), NULL, 0x00)
#define ui_image(_CTX, _HNDL) ui_image_ex((_CTX), (_HNDL), neko_v2s(0.f), neko_v2s(1.f), NULL, 0x00)
#define ui_combo_begin(_CTX, _ID, _ITEM, _MAX) ui_combo_begin_ex((_CTX), (_ID), (_ITEM), (_MAX), NULL, 0x00)
#define ui_combo_item(_CTX, _NAME) ui_combo_item_ex((_CTX), (_NAME), NULL, 0x00)
#define ui_dock(_CTX, _DST, _SRC, _TYPE) ui_dock_ex((_CTX), (_DST), (_SRC), (_TYPE), 0.5f)
#define ui_undock(_CTX, _NAME) ui_undock_ex((_CTX), (_NAME))
#define ui_label(_CTX, _FMT, ...) (neko_snprintf((_CTX)->number_edit_buf, sizeof((_CTX)->number_edit_buf), _FMT, ##__VA_ARGS__), ui_label_ex((_CTX), (_CTX)->number_edit_buf, NULL, 0x00))
#define ui_labelf(STR, ...)                              \
    do {                                                 \
        neko_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__); \
        ui_label(&ui, BUFFER);                           \
    } while (0)

// Elements (Extended)

i32 ui_image_ex(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, vec2 uv0, vec2 uv1, const ui_selector_desc_t* desc, u64 opt);
i32 ui_text_ex(ui_context_t* ctx, const char* text, i32 text_wrap, const ui_selector_desc_t* desc, u64 opt);
i32 ui_label_ex(ui_context_t* ctx, const char* text, const ui_selector_desc_t* desc, u64 opt);
i32 ui_button_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);
i32 ui_checkbox_ex(ui_context_t* ctx, const char* label, i32* state, const ui_selector_desc_t* desc, u64 opt);
i32 ui_textbox_raw(ui_context_t* ctx, char* buf, i32 bufsz, ui_id id, ui_rect_t r, const ui_selector_desc_t* desc, u64 opt);
i32 ui_textbox_ex(ui_context_t* ctx, char* buf, i32 bufsz, const ui_selector_desc_t* desc, u64 opt);
i32 ui_slider_ex(ui_context_t* ctx, ui_real* value, ui_real low, ui_real high, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt);
i32 ui_number_ex(ui_context_t* ctx, ui_real* value, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt);
i32 ui_header_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);
i32 ui_treenode_begin_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);
void ui_treenode_end(ui_context_t* ctx);
i32 ui_window_begin_ex(ui_context_t* ctx, const char* title, ui_rect_t rect, bool* open, const ui_selector_desc_t* desc, u64 opt);
void ui_window_end(ui_context_t* ctx);
void ui_popup_open(ui_context_t* ctx, const char* name);
i32 ui_popup_begin_ex(ui_context_t* ctx, const char* name, ui_rect_t r, const ui_selector_desc_t* desc, u64 opt);
void ui_popup_end(ui_context_t* ctx);
void ui_panel_begin_ex(ui_context_t* ctx, const char* name, const ui_selector_desc_t* desc, u64 opt);
void ui_panel_end(ui_context_t* ctx);
i32 ui_combo_begin_ex(ui_context_t* ctx, const char* id, const char* current_item, i32 max_items, ui_selector_desc_t* desc, u64 opt);
void ui_combo_end(ui_context_t* ctx);
i32 ui_combo_item_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);

// Demos

i32 ui_style_editor(ui_context_t* ctx, ui_style_sheet_t* style_sheet, ui_rect_t rect, bool* open);
i32 ui_demo_window(ui_context_t* ctx, ui_rect_t rect, bool* open);

// Docking

void ui_dock_ex(ui_context_t* ctx, const char* dst, const char* src, i32 split_type, float ratio);
void ui_undock_ex(ui_context_t* ctx, const char* name);
void ui_dock_ex_cnt(ui_context_t* ctx, ui_container_t* dst, ui_container_t* src, i32 split_type, float ratio);
void ui_undock_ex_cnt(ui_context_t* ctx, ui_container_t* cnt);

#endif
