
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

#include "engine/common/neko_util.h"
#include "engine/math/neko_math.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_internal.h"
#include "libs/imgui/imgui_stdlib.h"

#pragma region ImGuiAuto

#define __neko_imgui_tree_max_elementsize sizeof(std::string)
#define __neko_imgui_tree_max_tuple 3

neko_static_inline ImVec4 vec4_to_imvec4(const neko_vec4 &v4) { return {v4.x, v4.y, v4.z, v4.w}; }
neko_static_inline ImColor vec4_to_imcolor(const neko_vec4 &v4) { return {v4.x * 255.0f, v4.y * 255.0f, v4.z * 255.0f, v4.w * 255.0f}; }

neko_inline neko_color_t imvec_to_rgba(ImVec4 iv) {
    u8 newr = iv.x * 255;
    u8 newg = iv.y * 255;
    u8 newb = iv.z * 255;
    u8 newa = iv.w * 255;
    return neko_color_t{newr, newg, newb, newa};
}

namespace neko::imgui {

// 这就是这个库实现的功能 只是类 neko::imgui::Auto_t<T> 的包装
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
        // neko::imgui::detail::AutoTuple("Struct " + name, tuple);
        static_assert("Auto not support struct!");
    }
};
}  // namespace neko::imgui

template <typename T>
inline void neko::imgui::Auto(T &anything, const std::string &name) {
    neko::imgui::Auto_t<T>::Auto(anything, name);
}

template <typename T>
bool neko::imgui::detail::AutoExpand(const std::string &name, T &value) {
    if (sizeof(T) <= __neko_imgui_tree_max_elementsize) {
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
bool neko::imgui::detail::AutoContainerTreeNode(const std::string &name, Container &cont) {
    // std::size_t size = neko::imgui::detail::AutoContainerSize(cont);
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
bool neko::imgui::detail::AutoContainerValues(const std::string &name, Container &cont) {
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        ImGui::PushID(name.c_str());
        std::size_t i = 0;
        for (auto &elem : cont) {
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
bool neko::imgui::detail::AutoMapContainerValues(const std::string &name, Container &cont) {
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
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
void neko::imgui::detail::AutoContainerPushFrontButton(Container &cont) {
    if (ImGui::SmallButton("Push Front")) cont.emplace_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushBackButton(Container &cont) {
    if (ImGui::SmallButton("Push Back ")) cont.emplace_back();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopFrontButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Front ")) cont.pop_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopBackButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Back  ")) cont.pop_back();
}
template <typename Key, typename var>
void neko::imgui::detail::AutoMapKeyValue(Key &key, var &value) {
    bool b_k = sizeof(Key) <= __neko_imgui_tree_max_elementsize;
    bool b_v = sizeof(var) <= __neko_imgui_tree_max_elementsize;
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
void neko::imgui::detail::AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko::imgui::detail::AutoTupleRecurse<I - 1, Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + (std::is_const_v<type> ? "const " : "") + typeid(type).name();
    neko::imgui::detail::AutoExpand(str, std::get<I - 1>(tpl));
}
template <std::size_t I, typename... Args>
void neko::imgui::detail::AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko::imgui::detail::AutoTupleRecurse<I - 1, const Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + "const " + typeid(type).name();
    neko::imgui::detail::AutoExpand(str, ImGui::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string &name, std::tuple<Args...> &tpl) {
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= __neko_imgui_tree_max_elementsize && tuple_numelems <= __neko_imgui_tree_max_tuple) {
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
void neko::imgui::detail::AutoTuple(const std::string &name,
                                    const std::tuple<Args...> &tpl)  // same but const
{
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= __neko_imgui_tree_max_elementsize && tuple_numelems <= __neko_imgui_tree_max_tuple) {
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
#define __neko_imgui_def_begin_p(templatespec, typespec)                 \
    namespace neko::imgui {                                              \
    neko_va_unpack templatespec struct Auto_t<neko_va_unpack typespec> { \
        static void Auto(neko_va_unpack typespec &var, const std::string &name) {

// 如果宏参数内部没有逗号 请使用不带括号的此版本
#define __neko_imgui_def_begin(templatespec, typespec) __neko_imgui_def_begin_p((templatespec), (typespec))

#define __neko_imgui_def_end() \
    }                          \
    }                          \
    ;                          \
    }

#define __neko_imgui_def_inline_p(template_spec, type_spec, code) __neko_imgui_def_begin_p(template_spec, type_spec) code __neko_imgui_def_end()

#define __neko_imgui_def_inline(template_spec, type_spec, code) __neko_imgui_def_inline_p((template_spec), (type_spec), code)

#define NEKO_GUI_NULLPTR_COLOR ImVec4(1.0, 0.5, 0.5, 1.0)

__neko_imgui_def_begin(template <>, const_str) if (name.empty()) ImGui::TextUnformatted(var);
else ImGui::Text("%s=%s", name.c_str(), var);
__neko_imgui_def_end();

__neko_imgui_def_begin_p((template <std::size_t N>), (const detail::c_array_t<char, N>)) if (name.empty()) ImGui::TextUnformatted(var, var + N - 1);
else ImGui::Text("%s=%s", name.c_str(), var);
__neko_imgui_def_end();

__neko_imgui_def_inline(template <>, char *, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

__neko_imgui_def_inline(template <>, char *const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

__neko_imgui_def_inline(template <>, const_str const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

__neko_imgui_def_begin(template <>, std::string) const std::size_t lines = var.find('\n');
if (var.find('\n') != std::string::npos)
    ImGui::InputTextMultiline(name.c_str(), const_cast<char *>(var.c_str()), 256);
else
    ImGui::InputText(name.c_str(), const_cast<char *>(var.c_str()), 256);
__neko_imgui_def_end();
__neko_imgui_def_begin(template <>, const std::string) if (name.empty()) ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
else ImGui::Text("%s=%s", name.c_str(), var.c_str());
__neko_imgui_def_end();

__neko_imgui_def_inline(template <>, float, ImGui::DragFloat(name.c_str(), &var););
__neko_imgui_def_inline(template <>, int, ImGui::InputInt(name.c_str(), &var););
__neko_imgui_def_inline(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int *)&var););
__neko_imgui_def_inline(template <>, bool, ImGui::Checkbox(name.c_str(), &var););
__neko_imgui_def_inline(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
__neko_imgui_def_inline(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
__neko_imgui_def_inline(template <>, const float, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
__neko_imgui_def_inline(template <>, const int, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
__neko_imgui_def_inline(template <>, const unsigned, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
__neko_imgui_def_inline(template <>, const bool, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
__neko_imgui_def_inline(template <>, const ImVec2, ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
__neko_imgui_def_inline(template <>, const ImVec4, ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

#define INTERNAL_NUM(_c, _imn)                                                                               \
    __neko_imgui_def_inline(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
    __neko_imgui_def_inline(template <>, const _c, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name);)

INTERNAL_NUM(u8, U8);
INTERNAL_NUM(u16, U16);
INTERNAL_NUM(u64, U64);
INTERNAL_NUM(s8, S8);
INTERNAL_NUM(s16, S16);
INTERNAL_NUM(s64, S64);

__neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 1>), ImGui::DragFloat(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 1>), ImGui::Text("%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););
__neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 2>), ImGui::DragFloat2(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 2>), ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
__neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 3>), ImGui::DragFloat3(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 3>), ImGui::Text("%s(%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
__neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 4>), ImGui::DragFloat4(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 4>), ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

__neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 1>), ImGui::InputInt(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 1>), ImGui::Text("%s%d", (name.empty() ? "" : name + "=").c_str(), var[0]););
__neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 2>), ImGui::InputInt2(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 2>), ImGui::Text("%s(%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
__neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 3>), ImGui::InputInt3(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 3>), ImGui::Text("%s(%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
__neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 4>), ImGui::InputInt4(name.c_str(), &var[0]););
__neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 4>), ; ImGui::Text("%s(%d,%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

__neko_imgui_def_begin(template <typename T>, T *) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, T *const) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
__neko_imgui_def_end();

__neko_imgui_def_inline_p((template <typename T, std::size_t N>), (std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
__neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
__neko_imgui_def_inline_p((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(std::array<T, N> *)(&var)););
__neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(const std::array<T, N> *)(&var)););

__neko_imgui_def_begin_p((template <typename T1, typename T2>),
                         (std::pair<T1, T2>)) if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>)&&(std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
    float width = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(width * 0.4 - 10);  // a bit less than half
    neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    ImGui::SameLine();
    neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
    ImGui::PopItemWidth();
}
else {
    neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
}

__neko_imgui_def_end();

__neko_imgui_def_begin_p((template <typename T1, typename T2>), (const std::pair<T1, T2>)) neko::imgui::detail::AutoExpand<const T1>(name + ".first", var.first);
if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) ImGui::SameLine();
neko::imgui::detail::AutoExpand<const T2>(name + ".second", var.second);
__neko_imgui_def_end();

__neko_imgui_def_inline(template <typename... Args>, std::tuple<Args...>, neko::imgui::detail::AutoTuple("Tuple " + name, var););
__neko_imgui_def_inline(template <typename... Args>, const std::tuple<Args...>, neko::imgui::detail::AutoTuple("Tuple " + name, var););

__neko_imgui_def_begin(template <typename T>, std::vector<T>) if (neko::imgui::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
__neko_imgui_def_end();

__neko_imgui_def_begin(template <>, std::vector<bool>) if (neko::imgui::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
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
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, const std::vector<T>) neko::imgui::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin(template <>, const std::vector<bool>) if (neko::imgui::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        ImGui::Bullet();
        neko::imgui::Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
    ImGui::Unindent();
}
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, std::list<T>) if (neko::imgui::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
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
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, const std::list<T>) neko::imgui::detail::AutoContainerValues<const std::list<T>>("List " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, std::deque<T>) if (neko::imgui::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
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
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, const std::deque<T>) neko::imgui::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, std::forward_list<T>) if (neko::imgui::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
__neko_imgui_def_end();
__neko_imgui_def_begin(template <typename T>, const std::forward_list<T>) neko::imgui::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, std::set<T>) neko::imgui::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
// todo insert
__neko_imgui_def_end();
__neko_imgui_def_begin(template <typename T>, const std::set<T>) neko::imgui::detail::AutoContainerValues<const std::set<T>>("Set " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin(template <typename T>, std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
// todo insert
__neko_imgui_def_end();
__neko_imgui_def_begin(template <typename T>, const std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin_p((template <typename K, typename V>), (std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
// todo insert
__neko_imgui_def_end();
__neko_imgui_def_begin_p((template <typename K, typename V>), (const std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var);
__neko_imgui_def_end();

__neko_imgui_def_begin_p((template <typename K, typename V>), (std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
// todo insert
__neko_imgui_def_end();
__neko_imgui_def_begin_p((template <typename K, typename V>), (const std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name,
                                                                                                                                                                            var);
__neko_imgui_def_end();

__neko_imgui_def_inline(template <>, std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););
__neko_imgui_def_inline(template <>, const std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););

#pragma endregion ImGuiAuto

neko_inline ImVec4 neko_rgba2imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

namespace neko::imgui {

bool color_picker_3U32(const_str label, ImU32 *color, ImGuiColorEditFlags flags = 0);

void file_browser(std::string &path);

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

enum class alignment : unsigned char {
    kHorizontalCenter = 1 << 0,
    kVerticalCenter = 1 << 1,
    kCenter = kHorizontalCenter | kVerticalCenter,
};

inline void aligned_text(const std::string &text, alignment align, const float &width = 0.0F) {
    const auto alignment = neko_s_cast<unsigned char>(align);
    const auto text_size = ImGui::CalcTextSize(text.c_str());
    const auto wind_size = ImGui::GetContentRegionAvail();
    if (alignment & neko_s_cast<unsigned char>(alignment::kHorizontalCenter)) {
        if (width < 0.1F) {
            ImGui::SetCursorPosX((wind_size.x - text_size.x) * 0.5F);
        } else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - text_size.x) * 0.5F);
        }
    }
    if (alignment & neko_s_cast<unsigned char>(alignment::kVerticalCenter)) {
        ImGui::AlignTextToFramePadding();
    }

    ImGui::TextUnformatted(text.c_str());
}

inline auto check_button(const std::string &label, bool checked, const ImVec2 &size) -> bool {
    if (checked) {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
    } else {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive]);
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);
    }
    if (ImGui::Button(label.c_str(), size)) {
        checked = !checked;
    }

    ImGui::PopStyleColor(2);

    return checked;
}

inline auto button_tab(std::vector<std::string> &tabs, int &index) -> int {
    auto checked = 1 << index;
    std::string tab_names;
    std::for_each(tabs.begin(), tabs.end(), [&tab_names](const auto item) { tab_names += item; });
    const auto tab_width = ImGui::GetContentRegionAvail().x;
    const auto tab_btn_width = tab_width / neko_s_cast<float>(tabs.size());
    const auto h = ImGui::CalcTextSize(tab_names.c_str()).y;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, h);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, h);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);
    ImGui::BeginChild(tab_names.c_str(), {tab_width, h + ImGui::GetStyle().FramePadding.y * 2}, false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    for (int i = 0; i < tabs.size(); ++i) {
        auto &tab = tabs[i];

        // if current tab is checkd, uncheck otheres
        if (check_button(tab, checked & (1 << i), ImVec2{tab_btn_width, 0})) {
            checked = 0;
            checked = 1 << i;
        }

        if (i != tabs.size() - 1) {
            ImGui::SameLine();
        }
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
    ImGui::EndChild();

    index = 0;
    while (checked / 2) {
        checked = checked / 2;
        ++index;
    }

    return index;
}

inline void switch_button(std::string &&label, bool &checked) {
    float height = ImGui::GetFrameHeight();
    float width = height * 1.55F;
    float radius = height * 0.50F;
    const auto frame_width = ImGui::GetContentRegionAvail().x;

    aligned_text("    " + label, alignment::kVerticalCenter);
    ImGui::SameLine();

    ImGui::SetCursorPosX(frame_width - width);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton(label.c_str(), ImVec2(width, height))) {
        checked = !checked;
    }
    ImU32 col_bg = 0;
    if (checked) {
        col_bg = ImColor(ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
    } else {
        col_bg = ImColor(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    }
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), col_bg, radius);
    draw_list->AddCircleFilled(ImVec2(checked ? (pos.x + width - radius) : (pos.x + radius), pos.y + radius), radius - 1.5F, IM_COL32_WHITE);
}

inline void comb(std::string &&icon, std::string &&label, const std::vector<const_str> &items, int &index) {
    const auto p_w = ImGui::GetContentRegionAvail().x;
    aligned_text(icon + "    " + label, alignment::kVerticalCenter);
    ImGui::SameLine();
    ImGui::SetCursorPosX(p_w - 150.0F - ImGui::GetStyle().FramePadding.x);
    ImGui::SetNextItemWidth(150.0F);
    ImGui::Combo((std::string("##") + label).c_str(), &index, items.data(), neko_s_cast<int>(items.size()));
}

inline void input_int(std::string &&icon, std::string &&label, int &value) {
    const auto p_w = ImGui::GetContentRegionAvail().x;
    aligned_text(icon + "    " + label, alignment::kVerticalCenter);
    ImGui::SameLine();
    ImGui::SetCursorPosX(p_w - 100.0F - ImGui::GetStyle().FramePadding.x);
    ImGui::SetNextItemWidth(100.0F);
    ImGui::InputInt((std::string("##") + label).c_str(), &value);
}

inline void list_separator(float indent = 30.0F) {
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGuiContext &g = *GImGui;

    float thickness_draw = 1.0F;
    float thickness_layout = 0.0F;
    // Horizontal Separator
    float x1 = window->Pos.x + indent;
    float x2 = window->Pos.x + window->Size.x;

    // FIXME-WORKRECT: old hack (#205) until we decide of consistent behavior with WorkRect/Indent and Separator
    if (g.GroupStack.Size > 0 && g.GroupStack.back().WindowID == window->ID) {
        x1 += window->DC.Indent.x;
    }

    // We don't provide our width to the layout so that it doesn't get feed back into AutoFit
    const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + thickness_draw));
    ImGui::ItemSize(ImVec2(0.0F, thickness_layout));
    const bool item_visible = ImGui::ItemAdd(bb, 0);
    if (item_visible) {
        // Draw
        window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), ImGui::GetColorU32(ImGuiCol_Separator));
    }
}

}  // namespace neko::imgui

namespace neko {

void neko_imgui_style();

constexpr auto neko_imgui_color_from_byte = [](u8 r, u8 g, u8 b) { return ImVec4((f32)r / 255.0f, (f32)g / 255.0f, (f32)b / 255.0f, 1.0f); };

auto neko_imgui_collapsing_header = [](const_str name) -> bool {
    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ButtonHovered]);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);
    bool b = ImGui::CollapsingHeader(name);
    ImGui::PopStyleColor(3);
    return b;
};

u32 neko_draw_darken_color(u32 col, float brightness);

void neko_draw_text(std::string text, neko_color_t col, int x, int y, bool outline = false, neko_color_t outline_col = {0, 0, 0, 180});

void neko_draw_text_plate(std::string text, neko_color_t col, int x, int y, neko_color_t backcolor = {77, 77, 77, 140});

}  // namespace neko

namespace neko {

void neko_imgui_init();
void neko_imgui_new_frame();
void neko_imgui_render();
void neko_imgui_destroy();
std::size_t __neko_imgui_meminuse();

}  // namespace neko

#endif