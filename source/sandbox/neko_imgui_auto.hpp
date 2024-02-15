
#ifndef NEKO_IMGUI_UTILS_HPP
#define NEKO_IMGUI_UTILS_HPP

#include <array>
#include <cassert>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "engine/neko.h"
#include "engine/neko_engine.h"

// ImGui
#include "libs/imgui/imgui.h"

#define neko_imgui_tree_max_elementsize sizeof(std::string)
#define neko_imgui_tree_max_tuple 3

neko_static_inline ImVec4 vec4_to_imvec4(const neko_vec4 &v4) { return {v4.x, v4.y, v4.z, v4.w}; }
neko_static_inline ImColor vec4_to_imcolor(const neko_vec4 &v4) { return {v4.x * 255.0f, v4.y * 255.0f, v4.z * 255.0f, v4.w * 255.0f}; }

neko_inline neko_color_t imvec_to_rgba(ImVec4 iv) {
    u8 newr = iv.x * 255;
    u8 newg = iv.y * 255;
    u8 newb = iv.z * 255;
    u8 newa = iv.w * 255;
    return neko_color_t{newr, newg, newb, newa};
}

neko_inline ImVec4 rgba_to_imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

namespace neko_imgui {

// 这就是这个库实现的功能 只是类 neko_imgui::Auto_t<T> 的包装
template <typename T>
void Auto(T &anything, const std::string &name = std::string());

// same as std::as_const in c++17
template <class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept {
    return t;
}

namespace detail {

template <typename T>
bool AutoExpand(const std::string &name, T &value);
template <typename Container>
bool AutoContainerTreeNode(const std::string &name, Container &cont);
template <typename Container>
bool AutoContainerValues(const std::string &name,
                         Container &cont);  // Container must have .size(), .begin() and .end() methods and ::value_type.
template <typename Container>
bool AutoMapContainerValues(const std::string &name,
                            Container &map);  // Same as above but that iterates over pairs
template <typename Container>
void AutoContainerPushFrontButton(Container &cont);
template <typename Container>
void AutoContainerPushBackButton(Container &cont);
template <typename Container>
void AutoContainerPopFrontButton(Container &cont);
template <typename Container>
void AutoContainerPopBackButton(Container &cont);
template <typename Key, typename var>
void AutoMapKeyValue(Key &key, var &value);

template <class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept {
    return t;
}  // same as std::as_const in c++17
template <std::size_t I, typename... Args>
void AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> * = 0);
template <std::size_t I, typename... Args>
inline void AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 == I> * = 0) {}  // End of recursion.
template <std::size_t I, typename... Args>
void AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> * = 0);
template <std::size_t I, typename... Args>
inline void AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 == I> * = 0) {}  // End of recursion.
template <typename... Args>
void AutoTuple(const std::string &name, std::tuple<Args...> &tpl);
template <typename... Args>
void AutoTuple(const std::string &name, const std::tuple<Args...> &tpl);

template <typename T, std::size_t N>
using c_array_t = T[N];  // so arrays are regular types and can be used in macro

}  // namespace detail

template <typename T>
struct Auto_t {
    static void Auto(T &anything, const std::string &name) {
        // auto tuple = neko::cpp::pfr::structure_tie(anything);
        // neko_imgui::detail::AutoTuple("Struct " + name, tuple);
        static_assert("Auto not support struct!");
    }
};
}  // namespace neko_imgui

template <typename T>
inline void neko_imgui::Auto(T &anything, const std::string &name) {
    neko_imgui::Auto_t<T>::Auto(anything, name);
}

template <typename T>
bool neko_imgui::detail::AutoExpand(const std::string &name, T &value) {
    if (sizeof(T) <= neko_imgui_tree_max_elementsize) {
        ImGui::PushID(name.c_str());
        ImGui::Bullet();
        neko_imgui::Auto_t<T>::Auto(value, name);
        ImGui::PopID();
        return true;
    } else if (ImGui::TreeNode(name.c_str())) {
        neko_imgui::Auto_t<T>::Auto(value, name);
        ImGui::TreePop();
        return true;
    } else
        return false;
}

template <typename Container>
bool neko_imgui::detail::AutoContainerTreeNode(const std::string &name, Container &cont) {
    // std::size_t size = neko_imgui::detail::AutoContainerSize(cont);
    std::size_t size = cont.size();
    if (ImGui::CollapsingHeader(name.c_str())) {
        size_t elemsize = sizeof(decltype(*std::begin(cont)));
        ImGui::Text("大小: %d, 非动态元素大小: %d bytes", (int)size, (int)elemsize);
        return true;
    } else {
        float label_width = ImGui::CalcTextSize(name.c_str()).x + ImGui::GetTreeNodeToLabelSpacing() + 5;
        std::string sizetext = "(大小 = " + std::to_string(size) + ')';
        float sizet_width = ImGui::CalcTextSize(sizetext.c_str()).x;
        float avail_width = ImGui::GetContentRegionAvail().x;
        if (avail_width > label_width + sizet_width) {
            ImGui::SameLine(avail_width - sizet_width);
            ImGui::TextUnformatted(sizetext.c_str());
        }
        return false;
    }
}
template <typename Container>
bool neko_imgui::detail::AutoContainerValues(const std::string &name, Container &cont) {
    if (neko_imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        ImGui::PushID(name.c_str());
        std::size_t i = 0;
        for (auto &elem : cont) {
            std::string itemname = "[" + std::to_string(i) + ']';
            neko_imgui::detail::AutoExpand(itemname, elem);
            ++i;
        }
        ImGui::PopID();
        ImGui::Unindent();
        return true;
    } else
        return false;
}
template <typename Container>
bool neko_imgui::detail::AutoMapContainerValues(const std::string &name, Container &cont) {
    if (neko_imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        std::size_t i = 0;
        for (auto &elem : cont) {
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
void neko_imgui::detail::AutoContainerPushFrontButton(Container &cont) {
    if (ImGui::SmallButton("Push Front")) cont.emplace_front();
}
template <typename Container>
void neko_imgui::detail::AutoContainerPushBackButton(Container &cont) {
    if (ImGui::SmallButton("Push Back ")) cont.emplace_back();
}
template <typename Container>
void neko_imgui::detail::AutoContainerPopFrontButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Front ")) cont.pop_front();
}
template <typename Container>
void neko_imgui::detail::AutoContainerPopBackButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Back  ")) cont.pop_back();
}
template <typename Key, typename var>
void neko_imgui::detail::AutoMapKeyValue(Key &key, var &value) {
    bool b_k = sizeof(Key) <= neko_imgui_tree_max_elementsize;
    bool b_v = sizeof(var) <= neko_imgui_tree_max_elementsize;
    if (b_k) {
        ImGui::TextUnformatted("[");
        ImGui::SameLine();
        neko_imgui::Auto_t<Key>::Auto(key, "");
        ImGui::SameLine();
        ImGui::TextUnformatted("]");
        if (b_v) ImGui::SameLine();
        neko_imgui::Auto_t<var>::Auto(value, "Value");
    } else {
        neko_imgui::Auto_t<Key>::Auto(key, "Key");
        neko_imgui::Auto_t<var>::Auto(value, "Value");
    }
}

template <std::size_t I, typename... Args>
void neko_imgui::detail::AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko_imgui::detail::AutoTupleRecurse<I - 1, Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + (std::is_const_v<type> ? "const " : "") + typeid(type).name();
    neko_imgui::detail::AutoExpand(str, std::get<I - 1>(tpl));
}
template <std::size_t I, typename... Args>
void neko_imgui::detail::AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko_imgui::detail::AutoTupleRecurse<I - 1, const Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + "const " + typeid(type).name();
    neko_imgui::detail::AutoExpand(str, neko_imgui::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void neko_imgui::detail::AutoTuple(const std::string &name, std::tuple<Args...> &tpl) {
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " (" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        neko_imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko_imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}
template <typename... Args>
void neko_imgui::detail::AutoTuple(const std::string &name,
                                   const std::tuple<Args...> &tpl)  // same but const
{
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " !(" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        neko_imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko_imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}

// 在此版本中将 templatespec 和 typespec 括在括号中
#define neko_imgui_def_begin_p(templatespec, typespec)                   \
    namespace neko_imgui {                                               \
    neko_va_unpack templatespec struct Auto_t<neko_va_unpack typespec> { \
        static void Auto(neko_va_unpack typespec &var, const std::string &name) {

// 如果宏参数内部没有逗号 请使用不带括号的此版本
#define neko_imgui_def_begin(templatespec, typespec) neko_imgui_def_begin_p((templatespec), (typespec))

#define neko_imgui_def_end() \
    }                        \
    }                        \
    ;                        \
    }

#define neko_imgui_def_inline_p(template_spec, type_spec, code) neko_imgui_def_begin_p(template_spec, type_spec) code neko_imgui_def_end()

#define neko_imgui_def_inline(template_spec, type_spec, code) neko_imgui_def_inline_p((template_spec), (type_spec), code)

#define NEKO_GUI_NULLPTR_COLOR ImVec4(1.0, 0.5, 0.5, 1.0)

neko_imgui_def_begin(template <>, const_str) if (name.empty()) ImGui::TextUnformatted(var);
else ImGui::Text("%s=%s", name.c_str(), var);
neko_imgui_def_end();

neko_imgui_def_begin_p((template <std::size_t N>), (const detail::c_array_t<char, N>)) if (name.empty()) ImGui::TextUnformatted(var, var + N - 1);
else ImGui::Text("%s=%s", name.c_str(), var);
neko_imgui_def_end();

neko_imgui_def_inline(template <>, char *, const_str tmp = var; neko_imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, char *const, const_str tmp = var; neko_imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, const_str const, const_str tmp = var; neko_imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_begin(template <>, std::string) const std::size_t lines = var.find('\n');
if (var.find('\n') != std::string::npos)
    ImGui::InputTextMultiline(name.c_str(), const_cast<char *>(var.c_str()), 256);
else
    ImGui::InputText(name.c_str(), const_cast<char *>(var.c_str()), 256);
neko_imgui_def_end();
neko_imgui_def_begin(template <>, const std::string) if (name.empty()) ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
else ImGui::Text("%s=%s", name.c_str(), var.c_str());
neko_imgui_def_end();

neko_imgui_def_inline(template <>, float, ImGui::DragFloat(name.c_str(), &var););
neko_imgui_def_inline(template <>, int, ImGui::InputInt(name.c_str(), &var););
neko_imgui_def_inline(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int *)&var););
neko_imgui_def_inline(template <>, bool, ImGui::Checkbox(name.c_str(), &var););
neko_imgui_def_inline(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
neko_imgui_def_inline(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
neko_imgui_def_inline(template <>, const float, neko_imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const int, neko_imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const unsigned, neko_imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const bool, neko_imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const ImVec2, ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
neko_imgui_def_inline(template <>, const ImVec4, ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

#define INTERNAL_NUM(_c, _imn)                                                                             \
    neko_imgui_def_inline(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
    neko_imgui_def_inline(template <>, const _c, neko_imgui::Auto_t<const std::string>::Auto(std::to_string(var), name);)

// INTERNAL_NUM(u8, U8);
// INTERNAL_NUM(u16, U16);
// INTERNAL_NUM(u64, U64);
// INTERNAL_NUM(s8, S8);
// INTERNAL_NUM(s16, S16);
// INTERNAL_NUM(s64, S64);

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

neko_imgui_def_begin(template <typename T>, T *) if (var != nullptr) neko_imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, T *const) if (var != nullptr) neko_imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
neko_imgui_def_end();

neko_imgui_def_inline_p((template <typename T, std::size_t N>), (std::array<T, N>), neko_imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const std::array<T, N>), neko_imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko_imgui::detail::AutoContainerValues("Array " + name, *(std::array<T, N> *)(&var)););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko_imgui::detail::AutoContainerValues("Array " + name, *(const std::array<T, N> *)(&var)););

neko_imgui_def_begin_p((template <typename T1, typename T2>),
                       (std::pair<T1, T2>)) if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>)&&(std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
    float width = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(width * 0.4 - 10);  // a bit less than half
    neko_imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    ImGui::SameLine();
    neko_imgui::detail::AutoExpand<T2>(name + ".second", var.second);
    ImGui::PopItemWidth();
}
else {
    neko_imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    neko_imgui::detail::AutoExpand<T2>(name + ".second", var.second);
}

neko_imgui_def_end();

neko_imgui_def_begin_p((template <typename T1, typename T2>), (const std::pair<T1, T2>)) neko_imgui::detail::AutoExpand<const T1>(name + ".first", var.first);
if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) ImGui::SameLine();
neko_imgui::detail::AutoExpand<const T2>(name + ".second", var.second);
neko_imgui_def_end();

neko_imgui_def_inline(template <typename... Args>, std::tuple<Args...>, neko_imgui::detail::AutoTuple("Tuple " + name, var););
neko_imgui_def_inline(template <typename... Args>, const std::tuple<Args...>, neko_imgui::detail::AutoTuple("Tuple " + name, var););

neko_imgui_def_begin(template <typename T>, std::vector<T>) if (neko_imgui::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko_imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko_imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <>, std::vector<bool>) if (neko_imgui::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        bool b = var[i];
        ImGui::Bullet();
        neko_imgui::Auto_t<bool>::Auto(b, '[' + std::to_string(i) + ']');
        var[i] = b;
    }
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko_imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko_imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, const std::vector<T>) neko_imgui::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <>, const std::vector<bool>) if (neko_imgui::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        ImGui::Bullet();
        neko_imgui::Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::list<T>) if (neko_imgui::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko_imgui::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    neko_imgui::detail::AutoContainerPushBackButton(var);
    neko_imgui::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko_imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, const std::list<T>) neko_imgui::detail::AutoContainerValues<const std::list<T>>("List " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::deque<T>) if (neko_imgui::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko_imgui::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    neko_imgui::detail::AutoContainerPushBackButton(var);
    neko_imgui::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko_imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, const std::deque<T>) neko_imgui::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::forward_list<T>) if (neko_imgui::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko_imgui::detail::AutoContainerPushFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko_imgui::detail::AutoContainerPopFrontButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();
neko_imgui_def_begin(template <typename T>, const std::forward_list<T>) neko_imgui::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::set<T>) neko_imgui::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin(template <typename T>, const std::set<T>) neko_imgui::detail::AutoContainerValues<const std::set<T>>("Set " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::unordered_set<T>) neko_imgui::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin(template <typename T>, const std::unordered_set<T>) neko_imgui::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin_p((template <typename K, typename V>), (std::map<K, V>)) neko_imgui::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin_p((template <typename K, typename V>), (const std::map<K, V>)) neko_imgui::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin_p((template <typename K, typename V>), (std::unordered_map<K, V>)) neko_imgui::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin_p((template <typename K, typename V>), (const std::unordered_map<K, V>)) neko_imgui::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name, var);
neko_imgui_def_end();

neko_imgui_def_inline(template <>, std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););
neko_imgui_def_inline(template <>, const std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););

namespace neko_imgui {

neko_inline ImVec4 neko_rgba2imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

neko_inline void neko_imgui_help_marker(const_str desc) {
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
neko_inline void neko_imgui_file_browser(std::string &path) {

    ImGui::Text("Current Path: %s", path.c_str());
    ImGui::Separator();

    if (ImGui::Button("Parent Directory")) {
        std::filesystem::path current_path(path);
        if (!current_path.empty()) {
            current_path = current_path.parent_path();
            path = current_path.string();
        }
    }

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        const auto &entry_path = entry.path();
        const auto &filename = entry_path.filename().string();
        if (entry.is_directory()) {
            if (ImGui::Selectable((filename + "/").c_str())) path = entry_path.string();

        } else {
            if (ImGui::Selectable(filename.c_str())) path = entry_path.string();
        }
    }
}
#endif

}  // namespace neko_imgui

#endif