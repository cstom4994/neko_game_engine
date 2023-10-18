
#ifndef NEKO_NUI_AUTO_HPP
#define NEKO_NUI_AUTO_HPP

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
#include "neko_nui.h"

#define neko_nui_tree_max_elementsize sizeof(std::string)
#define neko_nui_tree_max_tuple 3

namespace neko_nui {

neko_global neko_nui_context *ctx = nullptr;

// 这就是这个库实现的功能 只是类 neko_nui::auto_t<T> 的包装
template <typename T>
void nui_auto(T &anything, const std::string &name = std::string());

// same as std::as_const in c++17
template <class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept {
    return t;
}

namespace detail {

template <typename T>
bool auto_expand(const std::string &name, T &value);
template <typename Container>
bool auto_container_tree_node(const std::string &name, Container &cont);
template <typename Container>
bool auto_container_values(const std::string &name,
                           Container &cont);  // Container must have .size(), .begin() and .end() methods and ::value_type.
template <typename Container>
bool auto_map_container_values(const std::string &name,
                               Container &map);  // Same as above but that iterates over pairs

template <typename Container>
void auto_container_button_pushfront(Container &cont);
template <typename Container>
void auto_container_button_pushback(Container &cont);
template <typename Container>
void auto_container_button_popfront(Container &cont);
template <typename Container>
void auto_container_button_popback(Container &cont);

template <typename Key, typename var>
void auto_map_kv(Key &key, var &value);

template <class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept {
    return t;
}  // same as std::as_const in c++17

template <std::size_t I, typename... Args>
void auto_tuple_recurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> * = 0);
template <std::size_t I, typename... Args>
inline void auto_tuple_recurse(std::tuple<Args...> &tpl, std::enable_if_t<0 == I> * = 0) {}  // End of recursion.
template <std::size_t I, typename... Args>
void auto_tuple_recurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> * = 0);
template <std::size_t I, typename... Args>
inline void auto_tuple_recurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 == I> * = 0) {}  // End of recursion.

template <typename... Args>
void auto_tuple(const std::string &name, std::tuple<Args...> &tpl);
template <typename... Args>
void auto_tuple(const std::string &name, const std::tuple<Args...> &tpl);

template <typename T, std::size_t N>
using c_array_t = T[N];  // so arrays are regular types and can be used in macro

}  // namespace detail

template <typename T>
struct auto_t {
    static void Auto(T &anything, const std::string &name, const bool auto_layout = true) {
        // auto tuple = neko::cpp::pfr::structure_tie(anything);
        // neko_nui::detail::auto_tuple("Struct " + name, tuple);
        static_assert("Neko nui not support struct yet!");
    }
};
}  // namespace neko_nui

template <typename T>
inline void neko_nui::nui_auto(T &anything, const std::string &name) {
    neko_nui::auto_t<T>::Auto(anything, name);
}

template <typename T>
bool neko_nui::detail::auto_expand(const std::string &name, T &value) {
    if (sizeof(T) <= neko_nui_tree_max_elementsize) {
        neko_nui::auto_t<T>::Auto(value, name);
        return true;
    } else if (neko_nui_tree_push_id(ctx, NEKO_NUI_TREE_TAB, name.c_str(), NEKO_NUI_MINIMIZED, neko::hash(name))) {
        neko_nui::auto_t<T>::Auto(value, name);
        neko_nui_tree_pop(ctx);
        return true;
    } else
        return false;
}

template <typename Container>
bool neko_nui::detail::auto_container_tree_node(const std::string &name, Container &cont) {
    // std::size_t size = neko_nui::detail::AutoContainerSize(cont);
    std::size_t size = cont.size();
    if (neko_nui_tree_push_id(ctx, NEKO_NUI_TREE_TAB, name.c_str(), NEKO_NUI_MINIMIZED, neko::hash(name))) {
        size_t elemsize = sizeof(decltype(*std::begin(cont)));
        neko_nui_labelf_wrap(ctx, "size: %d, element size: %d bytes", (int)size, (int)elemsize);
        neko_nui_tree_pop(ctx);
        return true;
    } else {
        neko_nui_layout_row_dynamic(ctx, 30, 1);
        std::string sizetext = "(size = " + std::to_string(size) + ')';
        return false;
    }
}
template <typename Container>
bool neko_nui::detail::auto_container_values(const std::string &name, Container &cont) {
    if (neko_nui::detail::auto_container_tree_node(name, cont)) {
        std::size_t i = 0;
        for (auto &elem : cont) {
            std::string itemname = "[" + std::to_string(i) + ']';
            neko_nui::detail::auto_expand(itemname, elem);
            ++i;
        }
        return true;
    } else
        return false;
}
template <typename Container>
bool neko_nui::detail::auto_map_container_values(const std::string &name, Container &cont) {
    if (neko_nui::detail::auto_container_tree_node(name, cont)) {
        std::size_t i = 0;
        for (auto &elem : cont) {
            auto_map_kv(elem.first, elem.second);
            ++i;
        }
        return true;
    } else
        return false;
}
template <typename Container>
void neko_nui::detail::auto_container_button_pushfront(Container &cont) {
    if (neko_nui_button_label(ctx, "Push Front")) cont.emplace_front();
}
template <typename Container>
void neko_nui::detail::auto_container_button_pushback(Container &cont) {
    if (neko_nui_button_label(ctx, "Push Back")) cont.emplace_back();
}
template <typename Container>
void neko_nui::detail::auto_container_button_popfront(Container &cont) {
    if (!cont.empty() && neko_nui_button_label(ctx, "Pop Front")) cont.pop_front();
}
template <typename Container>
void neko_nui::detail::auto_container_button_popback(Container &cont) {
    if (!cont.empty() && neko_nui_button_label(ctx, "Pop Back")) cont.pop_back();
}
template <typename Key, typename var>
void neko_nui::detail::auto_map_kv(Key &key, var &value) {
    bool b_k = sizeof(Key) <= neko_nui_tree_max_elementsize;
    bool b_v = sizeof(var) <= neko_nui_tree_max_elementsize;
    if (b_k) {

        neko_nui_layout_row_dynamic(ctx, 20, 3);

        neko_nui_label_wrap(ctx, "[");
        neko_nui::auto_t<Key>::Auto(key, "");
        neko_nui_label_wrap(ctx, "]");

        // if (b_v) {
        //     neko_nui_layout_row_dynamic(ctx, 20, 1);
        // }
        neko_nui::auto_t<var>::Auto(value, "Value", true);
    } else {
        neko_nui::auto_t<Key>::Auto(key, "Key");
        neko_nui::auto_t<var>::Auto(value, "Value");
    }
}

template <std::size_t I, typename... Args>
void neko_nui::detail::auto_tuple_recurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko_nui::detail::auto_tuple_recurse<I - 1, Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + (std::is_const_v<type> ? "const " : "") + typeid(type).name();
    neko_nui::detail::auto_expand(str, std::get<I - 1>(tpl));
}
template <std::size_t I, typename... Args>
void neko_nui::detail::auto_tuple_recurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko_nui::detail::auto_tuple_recurse<I - 1, const Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + "const " + typeid(type).name();
    neko_nui::detail::auto_expand(str, std::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void neko_nui::detail::auto_tuple(const std::string &name, std::tuple<Args...> &tpl) {
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_nui_tree_max_elementsize && tuple_numelems <= neko_nui_tree_max_tuple) {
        neko_nui_label_wrap(ctx, (name + " (" + std::to_string(tuple_size) + " bytes)").c_str());
        neko_nui::detail::auto_tuple_recurse<tuple_numelems, Args...>(tpl);
    } else if (neko_nui_tree_push_id(ctx, NEKO_NUI_TREE_TAB, std::string(name + " (" + std::to_string(tuple_size) + " bytes)").c_str(), NEKO_NUI_MINIMIZED, neko::hash(name))) {
        neko_nui::detail::auto_tuple_recurse<tuple_numelems, Args...>(tpl);
        neko_nui_tree_pop(ctx);
    }
}
template <typename... Args>
void neko_nui::detail::auto_tuple(const std::string &name,
                                  const std::tuple<Args...> &tpl)  // same but const
{
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_nui_tree_max_elementsize && tuple_numelems <= neko_nui_tree_max_tuple) {
        neko_nui_label_wrap(ctx, (name + " !(" + std::to_string(tuple_size) + " bytes)").c_str());
        neko_nui::detail::auto_tuple_recurse<tuple_numelems, Args...>(tpl);
    } else if (neko_nui_tree_push_id(ctx, NEKO_NUI_TREE_TAB, std::string(name + " (" + std::to_string(tuple_size) + " bytes)").c_str(), NEKO_NUI_MINIMIZED, neko::hash(name))) {
        neko_nui::detail::auto_tuple_recurse<tuple_numelems, Args...>(tpl);
        neko_nui_tree_pop(ctx);
    }
}

// 在此版本中将 templatespec 和 typespec 括在括号中
#define neko_nui_auto_def_begin_p(templatespec, typespec)                \
    namespace neko_nui {                                                 \
    neko_va_unpack templatespec struct auto_t<neko_va_unpack typespec> { \
        static void Auto(neko_va_unpack typespec &var, const std::string &name, const bool auto_layout = true) {

// 如果宏参数内部没有逗号 请使用不带括号的此版本
#define neko_nui_auto_def_begin(templatespec, typespec) neko_nui_auto_def_begin_p((templatespec), (typespec))

#define neko_nui_auto_def_end() \
    }                           \
    }                           \
    ;                           \
    }

#define neko_nui_auto_def_inline_p(template_spec, type_spec, code) neko_nui_auto_def_begin_p(template_spec, type_spec) code neko_nui_auto_def_end()

#define neko_nui_auto_def(template_spec, type_spec, code) neko_nui_auto_def_inline_p((template_spec), (type_spec), code)

neko_nui_auto_def_begin(template <>, const_str) if (name.empty()) neko_nui_label_wrap(ctx, var);
else neko_nui_labelf_wrap(ctx, "%s=%s", name.c_str(), var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin_p((template <std::size_t N>), (const detail::c_array_t<char, N>)) if (name.empty()) ImGui::TextUnformatted(var, var + N - 1);
else neko_nui_labelf_wrap(ctx, "%s=%s", name.c_str(), var);
neko_nui_auto_def_end();

neko_nui_auto_def(template <>, char *, const_str tmp = var; neko_nui::auto_t<const_str>::Auto(tmp, name););

neko_nui_auto_def(template <>, char *const, const_str tmp = var; neko_nui::auto_t<const_str>::Auto(tmp, name););

neko_nui_auto_def(template <>, const_str const, const_str tmp = var; neko_nui::auto_t<const_str>::Auto(tmp, name););

neko_nui_auto_def(template <>, std::string, {
    int len = var.length();  // TODO: 23/10/16 长度回调
    const std::size_t lines = var.find('\n');
    if (var.find('\n') != std::string::npos) {
        neko_nui_edit_string(ctx, NEKO_NUI_EDIT_MULTILINE, const_cast<char *>(var.c_str()), &len, 256, NULL);
    } else {
        neko_nui_edit_string(ctx, NEKO_NUI_EDIT_DEFAULT, const_cast<char *>(var.c_str()), &len, 256, NULL);
    }
});

neko_nui_auto_def(template <>, const std::string, {
    if (name.empty())
        neko_nui_text_wrap(ctx, var.c_str(), var.length());
    else
        neko_nui_labelf_wrap(ctx, "%s=%s", name.c_str(), var.c_str());
});

neko_nui_auto_def(template <>, float, {
    if (auto_layout) neko_nui_layout_row_dynamic(ctx, 30, 1);
    neko_nui_property_float(ctx, name.c_str(), INT_MIN, &var, INT_MAX, 1, 1);
});
neko_nui_auto_def(template <>, int, {
    if (auto_layout) neko_nui_layout_row_dynamic(ctx, 30, 1);
    neko_nui_property_int(ctx, name.c_str(), INT_MIN, &var, INT_MAX, 1, 1);
});
neko_nui_auto_def(template <>, bool, {
    if (auto_layout) neko_nui_layout_row_dynamic(ctx, 20, 1);
    neko_nui_checkbox_label(ctx, name.c_str(), (neko_nui_bool *)&var);
});
neko_nui_auto_def(template <>, unsigned int, {
    if (auto_layout) neko_nui_layout_row_dynamic(ctx, 30, 1);
    neko_nui_property_int(ctx, name.c_str(), 0, (int *)&var, UINT_MAX, 1, 1);
});

// neko_nui_auto_def(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
// neko_nui_auto_def(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
neko_nui_auto_def(template <>, const float, neko_nui::auto_t<const std::string>::Auto(std::to_string(var), name););
neko_nui_auto_def(template <>, const int, neko_nui::auto_t<const std::string>::Auto(std::to_string(var), name););
neko_nui_auto_def(template <>, const unsigned, neko_nui::auto_t<const std::string>::Auto(std::to_string(var), name););
neko_nui_auto_def(template <>, const bool, neko_nui::auto_t<const std::string>::Auto(std::to_string(var), name););

// neko_nui_auto_def(template <>, const ImVec2, neko_nui_labelf_wrap(ctx, "%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
// neko_nui_auto_def(template <>, const ImVec4, neko_nui_labelf_wrap(ctx, "%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

//#define INTERNAL_NUM(_c, _imn)                                                                         \
//    neko_nui_auto_def(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
//    neko_nui_auto_def(template <>, const _c, neko_nui::auto_t<const std::string>::Auto(std::to_string(var), name);)

// INTERNAL_NUM(u8, U8);
// INTERNAL_NUM(u16, U16);
// INTERNAL_NUM(u64, U64);
// INTERNAL_NUM(s8, S8);
// INTERNAL_NUM(s16, S16);
// INTERNAL_NUM(s64, S64);

neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<float, 1>), neko_nui_property_float(ctx, name.c_str(), f32_min, &var[0], f32_min, 0.1f, 1););
neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<float, 1>), neko_nui_labelf_wrap(ctx, "%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););
// neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<float, 2>), ImGui::DragFloat2(name.c_str(), &var[0]););
// neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<float, 2>), neko_nui_labelf_wrap(ctx, "%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
// neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<float, 3>), ImGui::DragFloat3(name.c_str(), &var[0]););
// neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<float, 3>), neko_nui_labelf_wrap(ctx, "%s(%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
// neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<float, 4>), ImGui::DragFloat4(name.c_str(), &var[0]););
// neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<float, 4>), neko_nui_labelf_wrap(ctx, "%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2],
// var[3]););

neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<int, 1>), neko_nui_property_int(ctx, name.c_str(), INT_MIN, &var[0], INT_MAX, 1, 1););
neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<int, 1>), neko_nui_labelf_wrap(ctx, "%s%d", (name.empty() ? "" : name + "=").c_str(), var[0]););

// neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<int, 2>), ImGui::InputInt2(name.c_str(), &var[0]););
// neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<int, 2>), neko_nui_labelf_wrap(ctx, "%s(%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
// neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<int, 3>), ImGui::InputInt3(name.c_str(), &var[0]););
// neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<int, 3>), neko_nui_labelf_wrap(ctx, "%s(%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
// neko_nui_auto_def_inline_p((template <>), (detail::c_array_t<int, 4>), ImGui::InputInt4(name.c_str(), &var[0]););
// neko_nui_auto_def_inline_p((template <>), (const detail::c_array_t<int, 4>), ; neko_nui_labelf_wrap(ctx, "%s(%d,%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2],
// var[3]););

neko_nui_auto_def(template <typename T>, T *, {
    if (var != nullptr)
        neko_nui::detail::auto_expand<T>("Pointer " + name, *var);
    else {
        neko_nui_style_push_color(ctx, &ctx->style.text.color, neko_nui_rgb(255, 128, 128));
        neko_nui_labelf_wrap(ctx, "%s=NULL", name.c_str());
        neko_nui_style_pop_color(ctx);
    }
});

neko_nui_auto_def(template <typename T>, T *const, {
    if (var != nullptr)
        neko_nui::detail::auto_expand<T>("Pointer " + name, *var);
    else {
        neko_nui_style_push_color(ctx, &ctx->style.text.color, neko_nui_rgb(255, 128, 128));
        neko_nui_labelf_wrap(ctx, "%s=NULL", name.c_str());
        neko_nui_style_pop_color(ctx);
    }
});

neko_nui_auto_def_inline_p((template <typename T, std::size_t N>), (std::array<T, N>), neko_nui::detail::auto_container_values("std::array " + name, var););
neko_nui_auto_def_inline_p((template <typename T, std::size_t N>), (const std::array<T, N>), neko_nui::detail::auto_container_values("std::array " + name, var););
neko_nui_auto_def_inline_p((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko_nui::detail::auto_container_values("c_array " + name, *(std::array<T, N> *)(&var)););
neko_nui_auto_def_inline_p((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko_nui::detail::auto_container_values("c_array " + name, *(const std::array<T, N> *)(&var)););

neko_nui_auto_def_begin_p((template <typename T1, typename T2>),
                          (std::pair<T1, T2>)) if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>)&&(std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
    neko_nui::detail::auto_expand<T1>(name + ".first", var.first);
    { neko_nui_layout_row_dynamic(ctx, 30, 1); }
    neko_nui::detail::auto_expand<T2>(name + ".second", var.second);
}
else {
    neko_nui::detail::auto_expand<T1>(name + ".first", var.first);
    neko_nui::detail::auto_expand<T2>(name + ".second", var.second);
}

neko_nui_auto_def_end();

neko_nui_auto_def_begin_p((template <typename T1, typename T2>), (const std::pair<T1, T2>)) neko_nui::detail::auto_expand<const T1>(name + ".first", var.first);
if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) {
    neko_nui_layout_row_dynamic(ctx, 30, 1);
}
neko_nui::detail::auto_expand<const T2>(name + ".second", var.second);
neko_nui_auto_def_end();

neko_nui_auto_def(template <typename... Args>, std::tuple<Args...>, neko_nui::detail::auto_tuple("Tuple " + name, var););
neko_nui_auto_def(template <typename... Args>, const std::tuple<Args...>, neko_nui::detail::auto_tuple("Tuple " + name, var););

neko_nui_auto_def_begin(template <typename T>, std::vector<T>) if (neko_nui::detail::auto_container_values<std::vector<T>>("Vector " + name, var)) {
    neko_nui::detail::auto_container_button_pushback(var);
    if (!var.empty()) {
        neko_nui_layout_row_dynamic(ctx, 30, 1);
    }
    neko_nui::detail::auto_container_button_popback(var);
}
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <>, std::vector<bool>) if (neko_nui::detail::auto_container_tree_node<std::vector<bool>>("Vector " + name, var)) {
    for (int i = 0; i < var.size(); ++i) {
        bool b = var[i];
        neko_nui::auto_t<bool>::Auto(b, '[' + std::to_string(i) + ']');
        var[i] = b;
    }
    neko_nui::detail::auto_container_button_pushback(var);
    if (!var.empty()) {
        neko_nui_layout_row_dynamic(ctx, 30, 1);
    }
    neko_nui::detail::auto_container_button_popback(var);
}
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, const std::vector<T>) neko_nui::detail::auto_container_values<const std::vector<T>>("Vector " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <>, const std::vector<bool>) if (neko_nui::detail::auto_container_tree_node<const std::vector<bool>>("Vector " + name, var)) {
    for (int i = 0; i < var.size(); ++i) {
        neko_nui::auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
}
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, std::list<T>) if (neko_nui::detail::auto_container_values<std::list<T>>("List " + name, var)) {
    neko_nui::detail::auto_container_button_pushfront(var);
    if (auto_layout) neko_nui_layout_row_dynamic(ctx, 30, 2);
    neko_nui::detail::auto_container_button_pushback(var);
    neko_nui::detail::auto_container_button_popfront(var);
    if (!var.empty()) {
        neko_nui_layout_row_dynamic(ctx, 30, 1);
    }
    neko_nui::detail::auto_container_button_popback(var);
}
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, const std::list<T>) neko_nui::detail::auto_container_values<const std::list<T>>("List " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, std::deque<T>) if (neko_nui::detail::auto_container_values<std::deque<T>>("Deque " + name, var)) {
    neko_nui::detail::auto_container_button_pushfront(var);
    if (auto_layout) neko_nui_layout_row_dynamic(ctx, 30, 2);
    neko_nui::detail::auto_container_button_pushback(var);
    neko_nui::detail::auto_container_button_popfront(var);
    if (!var.empty()) {
        neko_nui_layout_row_dynamic(ctx, 30, 1);
    }
    neko_nui::detail::auto_container_button_popback(var);
}
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, const std::deque<T>) neko_nui::detail::auto_container_values<const std::deque<T>>("Deque " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, std::forward_list<T>) if (neko_nui::detail::auto_container_values<std::forward_list<T>>("Forward list " + name, var)) {
    neko_nui::detail::auto_container_button_pushfront(var);
    if (!var.empty()) {
        neko_nui_layout_row_dynamic(ctx, 30, 1);
    }
    neko_nui::detail::auto_container_button_popfront(var);
}
neko_nui_auto_def_end();
neko_nui_auto_def_begin(template <typename T>, const std::forward_list<T>) neko_nui::detail::auto_container_values<const std::forward_list<T>>("Forward list " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, std::set<T>) neko_nui::detail::auto_container_values<std::set<T>>("Set " + name, var);
// todo insert
neko_nui_auto_def_end();
neko_nui_auto_def_begin(template <typename T>, const std::set<T>) neko_nui::detail::auto_container_values<const std::set<T>>("Set " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin(template <typename T>, std::unordered_set<T>) neko_nui::detail::auto_container_values<std::unordered_set<T>>("Unordered set " + name, var);
// todo insert
neko_nui_auto_def_end();
neko_nui_auto_def_begin(template <typename T>, const std::unordered_set<T>) neko_nui::detail::auto_container_values<const std::unordered_set<T>>("Unordered set " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin_p((template <typename K, typename V>), (std::map<K, V>)) neko_nui::detail::auto_map_container_values<std::map<K, V>>("Map " + name, var);
// todo insert
neko_nui_auto_def_end();
neko_nui_auto_def_begin_p((template <typename K, typename V>), (const std::map<K, V>)) neko_nui::detail::auto_map_container_values<const std::map<K, V>>("Map " + name, var);
neko_nui_auto_def_end();

neko_nui_auto_def_begin_p((template <typename K, typename V>), (std::unordered_map<K, V>)) neko_nui::detail::auto_map_container_values<std::unordered_map<K, V>>("Unordered map " + name, var);
// todo insert
neko_nui_auto_def_end();
neko_nui_auto_def_begin_p((template <typename K, typename V>), (const std::unordered_map<K, V>)) neko_nui::detail::auto_map_container_values<const std::unordered_map<K, V>>("Unordered map " + name,
                                                                                                                                                                             var);
neko_nui_auto_def_end();

neko_nui_auto_def(template <>, std::add_pointer_t<void()>, if (neko_nui_button_label(ctx, name.c_str())) var(););
neko_nui_auto_def(template <>, const std::add_pointer_t<void()>, if (neko_nui_button_label(ctx, name.c_str())) var(););

#if 0
void file_browser(std::string &path) {

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

#endif