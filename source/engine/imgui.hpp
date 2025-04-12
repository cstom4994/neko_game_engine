#pragma once

#include <malloc.h>

#include "base/common/base.hpp"
#include "engine/scripting/scripting.h"

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

// imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

namespace Neko {

namespace ImGuiWrap {

inline ImVec4 rgba_to_imvec(int r, int g, int b, int a = 255) {
    f32 newr = r / 255.f;
    f32 newg = g / 255.f;
    f32 newb = b / 255.f;
    f32 newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

static inline void OutlineTextColoredV(ImDrawList *drawList, const ImVec2 &pos, ImU32 text_color, ImU32 outline_color, const char *fmt, va_list args) {
    static char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    // 画出轮廓
    drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), outline_color, buffer);
    drawList->AddText(ImVec2(pos.x - 1, pos.y + 1), outline_color, buffer);
    drawList->AddText(ImVec2(pos.x + 1, pos.y - 1), outline_color, buffer);
    drawList->AddText(ImVec2(pos.x - 1, pos.y - 1), outline_color, buffer);

    // 绘制主要文字
    drawList->AddText(pos, text_color, buffer);
}

static inline void OutlineTextV(ImDrawList *drawList, const ImVec2 &pos, const char *fmt, va_list args) { OutlineTextColoredV(drawList, pos, IM_COL32_WHITE, IM_COL32_BLACK, fmt, args); }

static inline void OutlineTextColored(ImDrawList *drawList, const ImVec2 &pos, ImU32 text_color, ImU32 outline_color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    OutlineTextColoredV(drawList, pos, text_color, outline_color, fmt, args);
    va_end(args);
}

static inline void OutlineText(ImDrawList *drawList, const ImVec2 &pos, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    OutlineTextColoredV(drawList, pos, IM_COL32_WHITE, IM_COL32_BLACK, fmt, args);
    va_end(args);
}

}  // namespace ImGuiWrap

}  // namespace Neko

namespace Neko::ImGuiWrap::util {

struct TableInteger {
    const char *name;
    lua_Integer value;
};

using GenerateAny = void (*)(lua_State *L);
struct TableAny {
    const char *name;
    GenerateAny value;
};

struct strbuf {
    char *data;
    size_t size;
};

struct input_context {
    lua_State *L;
    int callback;
};

lua_Integer field_tointeger(lua_State *L, int idx, lua_Integer i);
lua_Number field_tonumber(lua_State *L, int idx, lua_Integer i);
bool field_toboolean(lua_State *L, int idx, lua_Integer i);
ImTextureID get_texture_id(lua_State *L, int idx);
const char *format(lua_State *L, int idx);
strbuf *strbuf_create(lua_State *L, int idx);
strbuf *strbuf_get(lua_State *L, int idx);
int input_callback(ImGuiInputTextCallbackData *data);
void create_table(lua_State *L, std::span<TableInteger> l);
void set_table(lua_State *L, std::span<TableAny> l);
void struct_gen(lua_State *L, const char *name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters);
void flags_gen(lua_State *L, const char *name);
void init(lua_State *L);

}  // namespace Neko::ImGuiWrap::util

struct GLFWwindow;

namespace Neko {

namespace ImGuiWrap {

class ImGuiRender : public SingletonClass<ImGuiRender> {
public:
    void imgui_draw_post();
    int imgui_draw_pre();
    void imgui_init();
    void imgui_fini();
};

}  // namespace ImGuiWrap

}  // namespace Neko

namespace Neko::ImGuiWrap {

// 这就是这个库实现的功能 只是类 Neko::ImGuiWrap::Auto_t<T> 的包装
template <typename T>
void Auto(T &anything, const std::string &name = std::string());

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
        // auto tuple = Neko::cpp::pfr::structure_tie(anything);
        // Neko::ImGuiWrap::detail::AutoTuple("Struct " + name, tuple);
        static_assert("Auto not support struct!");
    }
};
}  // namespace Neko::ImGuiWrap

template <typename T>
inline void Neko::ImGuiWrap::Auto(T &anything, const std::string &name) {
    Neko::ImGuiWrap::Auto_t<T>::Auto(anything, name);
}

template <typename T>
bool Neko::ImGuiWrap::detail::AutoExpand(const std::string &name, T &value) {
    if (sizeof(T) <= neko_imgui_tree_max_elementsize) {
        ImGui::PushID(name.c_str());
        ImGui::Bullet();
        Neko::ImGuiWrap::Auto_t<T>::Auto(value, name);
        ImGui::PopID();
        return true;
    } else if (ImGui::TreeNode(name.c_str())) {
        Neko::ImGuiWrap::Auto_t<T>::Auto(value, name);
        ImGui::TreePop();
        return true;
    } else
        return false;
}

template <typename Container>
bool Neko::ImGuiWrap::detail::AutoContainerTreeNode(const std::string &name, Container &cont) {
    // std::size_t size = Neko::ImGuiWrap::detail::AutoContainerSize(cont);
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
bool Neko::ImGuiWrap::detail::AutoContainerValues(const std::string &name, Container &cont) {
    if (Neko::ImGuiWrap::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        ImGui::PushID(name.c_str());
        std::size_t i = 0;
        for (auto &elem : cont) {
            std::string itemname = "[" + std::to_string(i) + ']';
            Neko::ImGuiWrap::detail::AutoExpand(itemname, elem);
            ++i;
        }
        ImGui::PopID();
        ImGui::Unindent();
        return true;
    } else
        return false;
}
template <typename Container>
bool Neko::ImGuiWrap::detail::AutoMapContainerValues(const std::string &name, Container &cont) {
    if (Neko::ImGuiWrap::detail::AutoContainerTreeNode(name, cont)) {
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
void Neko::ImGuiWrap::detail::AutoContainerPushFrontButton(Container &cont) {
    if (ImGui::SmallButton("Push Front")) cont.emplace_front();
}
template <typename Container>
void Neko::ImGuiWrap::detail::AutoContainerPushBackButton(Container &cont) {
    if (ImGui::SmallButton("Push Back ")) cont.emplace_back();
}
template <typename Container>
void Neko::ImGuiWrap::detail::AutoContainerPopFrontButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Front ")) cont.pop_front();
}
template <typename Container>
void Neko::ImGuiWrap::detail::AutoContainerPopBackButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Back  ")) cont.pop_back();
}
template <typename Key, typename var>
void Neko::ImGuiWrap::detail::AutoMapKeyValue(Key &key, var &value) {
    bool b_k = sizeof(Key) <= neko_imgui_tree_max_elementsize;
    bool b_v = sizeof(var) <= neko_imgui_tree_max_elementsize;
    if (b_k) {
        ImGui::TextUnformatted("[");
        ImGui::SameLine();
        Neko::ImGuiWrap::Auto_t<Key>::Auto(key, "");
        ImGui::SameLine();
        ImGui::TextUnformatted("]");
        if (b_v) ImGui::SameLine();
        Neko::ImGuiWrap::Auto_t<var>::Auto(value, "Value");
    } else {
        Neko::ImGuiWrap::Auto_t<Key>::Auto(key, "Key");
        Neko::ImGuiWrap::Auto_t<var>::Auto(value, "Value");
    }
}

template <std::size_t I, typename... Args>
void Neko::ImGuiWrap::detail::AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    Neko::ImGuiWrap::detail::AutoTupleRecurse<I - 1, Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + (std::is_const_v<type> ? "const " : "") + typeid(type).name();
    Neko::ImGuiWrap::detail::AutoExpand(str, std::get<I - 1>(tpl));
}
template <std::size_t I, typename... Args>
void Neko::ImGuiWrap::detail::AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    Neko::ImGuiWrap::detail::AutoTupleRecurse<I - 1, const Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + "const " + typeid(type).name();
    Neko::ImGuiWrap::detail::AutoExpand(str, Neko::cpp::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void Neko::ImGuiWrap::detail::AutoTuple(const std::string &name, std::tuple<Args...> &tpl) {
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " (" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        Neko::ImGuiWrap::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        Neko::ImGuiWrap::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}
template <typename... Args>
void Neko::ImGuiWrap::detail::AutoTuple(const std::string &name,
                                        const std::tuple<Args...> &tpl)  // same but const
{
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " !(" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        Neko::ImGuiWrap::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        Neko::ImGuiWrap::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}

#include "engine/imgui_util.hpp"