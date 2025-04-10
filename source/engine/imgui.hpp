#pragma once

#include <malloc.h>

#include "base/common/base.hpp"
#include "base/scripting/scripting.h"

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

// 在此版本中将 templatespec 和 typespec 括在括号中
#define DEFINE_IMGUI_BEGIN_P(templatespec, typespec)                     \
    namespace Neko::ImGuiWrap {                                          \
    NEKO_VA_UNPACK templatespec struct Auto_t<NEKO_VA_UNPACK typespec> { \
        static void Auto(NEKO_VA_UNPACK typespec &var, const std::string &name) {

// 如果宏参数内部没有逗号 请使用不带括号的此版本
#define DEFINE_IMGUI_BEGIN(templatespec, typespec) DEFINE_IMGUI_BEGIN_P((templatespec), (typespec))

#define DEFINE_IMGUI_END() \
    }                      \
    }                      \
    ;                      \
    }

#define DEFINE_IMGUI_INLINE_P(template_spec, type_spec, code) DEFINE_IMGUI_BEGIN_P(template_spec, type_spec) code DEFINE_IMGUI_END()

#define DEFINE_IMGUI_INLINE(template_spec, type_spec, code) DEFINE_IMGUI_INLINE_P((template_spec), (type_spec), code)

#define NEKO_GUI_NULLPTR_COLOR ImVec4(1.0, 0.5, 0.5, 1.0)

DEFINE_IMGUI_BEGIN(template <>, const_str) if (name.empty()) ImGui::TextUnformatted(var);
else ImGui::Text("%s=%s", name.c_str(), var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN_P((template <std::size_t N>), (const detail::c_array_t<char, N>)) if (name.empty()) ImGui::TextUnformatted(var, var + N - 1);
else ImGui::Text("%s=%s", name.c_str(), var);
DEFINE_IMGUI_END()

// DEFINE_IMGUI_INLINE(template <>, char *, const_str tmp = var; Neko::ImGuiWrap::Auto_t<const_str>::Auto(tmp, name););
//
// DEFINE_IMGUI_INLINE(template <>, char *const, const_str tmp = var; Neko::ImGuiWrap::Auto_t<const_str>::Auto(tmp, name););
//
// DEFINE_IMGUI_INLINE(template <>, const_str const, const_str tmp = var; Neko::ImGuiWrap::Auto_t<const_str>::Auto(tmp, name););

DEFINE_IMGUI_BEGIN(template <>, std::string) const std::size_t lines = var.find('\n');
if (var.find('\n') != std::string::npos)
    ImGui::InputTextMultiline(name.c_str(), const_cast<char *>(var.c_str()), 256);
else
    ImGui::InputText(name.c_str(), const_cast<char *>(var.c_str()), 256);
DEFINE_IMGUI_END()
DEFINE_IMGUI_BEGIN(template <>, const std::string) if (name.empty()) ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
else ImGui::Text("%s=%s", name.c_str(), var.c_str());
DEFINE_IMGUI_END()

DEFINE_IMGUI_INLINE(template <>, float, ImGui::DragFloat(name.c_str(), &var););
DEFINE_IMGUI_INLINE(template <>, int, ImGui::InputInt(name.c_str(), &var););
DEFINE_IMGUI_INLINE(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int *)&var););
DEFINE_IMGUI_INLINE(template <>, bool, ImGui::Checkbox(name.c_str(), &var););
DEFINE_IMGUI_INLINE(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
DEFINE_IMGUI_INLINE(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
DEFINE_IMGUI_INLINE(template <>, const float, Auto_t<const std::string>::Auto(std::to_string(var), name););
DEFINE_IMGUI_INLINE(template <>, const int, Auto_t<const std::string>::Auto(std::to_string(var), name););
DEFINE_IMGUI_INLINE(template <>, const unsigned, Auto_t<const std::string>::Auto(std::to_string(var), name););
DEFINE_IMGUI_INLINE(template <>, const bool, Auto_t<const std::string>::Auto(std::to_string(var), name););
DEFINE_IMGUI_INLINE(template <>, const ImVec2, ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
DEFINE_IMGUI_INLINE(template <>, const ImVec4, ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

#define INTERNAL_NUM(_c, _imn)                                                                           \
    DEFINE_IMGUI_INLINE(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
    DEFINE_IMGUI_INLINE(template <>, const _c, Auto_t<const std::string>::Auto(std::to_string(var), name);)

INTERNAL_NUM(u8, U8);
INTERNAL_NUM(u16, U16);
INTERNAL_NUM(u64, U64);
INTERNAL_NUM(i8, S8);
INTERNAL_NUM(i16, S16);
INTERNAL_NUM(i64, S64);

DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<float, 1>), ImGui::DragFloat(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<float, 1>), ImGui::Text("%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<float, 2>), ImGui::DragFloat2(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<float, 2>), ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<float, 3>), ImGui::DragFloat3(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<float, 3>), ImGui::Text("%s(%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<float, 4>), ImGui::DragFloat4(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<float, 4>), ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<int, 1>), ImGui::InputInt(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<int, 1>), ImGui::Text("%s%d", (name.empty() ? "" : name + "=").c_str(), var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<int, 2>), ImGui::InputInt2(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<int, 2>), ImGui::Text("%s(%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<int, 3>), ImGui::InputInt3(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<int, 3>), ImGui::Text("%s(%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
DEFINE_IMGUI_INLINE_P((template <>), (detail::c_array_t<int, 4>), ImGui::InputInt4(name.c_str(), &var[0]););
DEFINE_IMGUI_INLINE_P((template <>), (const detail::c_array_t<int, 4>), ; ImGui::Text("%s(%d,%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

DEFINE_IMGUI_BEGIN(template <typename T>, T *) if (var != nullptr) Neko::ImGuiWrap::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, T *const) if (var != nullptr) Neko::ImGuiWrap::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
DEFINE_IMGUI_END()

DEFINE_IMGUI_INLINE_P((template <typename T, std::size_t N>), (std::array<T, N>), Neko::ImGuiWrap::detail::AutoContainerValues("array " + name, var););
DEFINE_IMGUI_INLINE_P((template <typename T, std::size_t N>), (const std::array<T, N>), Neko::ImGuiWrap::detail::AutoContainerValues("array " + name, var););
DEFINE_IMGUI_INLINE_P((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), Neko::ImGuiWrap::detail::AutoContainerValues("Array " + name, *(std::array<T, N> *)(&var)););
DEFINE_IMGUI_INLINE_P((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), Neko::ImGuiWrap::detail::AutoContainerValues("Array " + name, *(const std::array<T, N> *)(&var)););

DEFINE_IMGUI_BEGIN_P((template <typename T1, typename T2>), (std::pair<T1, T2>))
if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>) && (std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
    float width = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(width * 0.4 - 10);  // a bit less than half
    Neko::ImGuiWrap::detail::AutoExpand<T1>(name + ".first", var.first);
    ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoExpand<T2>(name + ".second", var.second);
    ImGui::PopItemWidth();
} else {
    Neko::ImGuiWrap::detail::AutoExpand<T1>(name + ".first", var.first);
    Neko::ImGuiWrap::detail::AutoExpand<T2>(name + ".second", var.second);
}

DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN_P((template <typename T1, typename T2>), (const std::pair<T1, T2>)) Neko::ImGuiWrap::detail::AutoExpand<const T1>(name + ".first", var.first);
if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) ImGui::SameLine();
Neko::ImGuiWrap::detail::AutoExpand<const T2>(name + ".second", var.second);
DEFINE_IMGUI_END()

DEFINE_IMGUI_INLINE(template <typename... Args>, std::tuple<Args...>, Neko::ImGuiWrap::detail::AutoTuple("Tuple " + name, var););
DEFINE_IMGUI_INLINE(template <typename... Args>, const std::tuple<Args...>, Neko::ImGuiWrap::detail::AutoTuple("Tuple " + name, var););

DEFINE_IMGUI_BEGIN(template <typename T>, std::vector<T>) if (Neko::ImGuiWrap::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    Neko::ImGuiWrap::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, std::vector<bool>) if (Neko::ImGuiWrap::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        bool b = var[i];
        ImGui::Bullet();
        Auto_t<bool>::Auto(b, '[' + std::to_string(i) + ']');
        var[i] = b;
    }
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    Neko::ImGuiWrap::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
    ImGui::Unindent();
}
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, const std::vector<T>) Neko::ImGuiWrap::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, const std::vector<bool>) if (Neko::ImGuiWrap::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        ImGui::Bullet();
        Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
    ImGui::Unindent();
}
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, std::list<T>) if (Neko::ImGuiWrap::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    Neko::ImGuiWrap::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPushBackButton(var);
    Neko::ImGuiWrap::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, const std::list<T>) Neko::ImGuiWrap::detail::AutoContainerValues<const std::list<T>>("List " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, std::deque<T>) if (Neko::ImGuiWrap::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    Neko::ImGuiWrap::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPushBackButton(var);
    Neko::ImGuiWrap::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, const std::deque<T>) Neko::ImGuiWrap::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, std::forward_list<T>) if (Neko::ImGuiWrap::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    Neko::ImGuiWrap::detail::AutoContainerPushFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    Neko::ImGuiWrap::detail::AutoContainerPopFrontButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
DEFINE_IMGUI_END()
DEFINE_IMGUI_BEGIN(template <typename T>, const std::forward_list<T>) Neko::ImGuiWrap::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, std::set<T>) Neko::ImGuiWrap::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
// todo insert
DEFINE_IMGUI_END()
DEFINE_IMGUI_BEGIN(template <typename T>, const std::set<T>) Neko::ImGuiWrap::detail::AutoContainerValues<const std::set<T>>("Set " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <typename T>, std::unordered_set<T>) Neko::ImGuiWrap::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
// todo insert
DEFINE_IMGUI_END()
DEFINE_IMGUI_BEGIN(template <typename T>, const std::unordered_set<T>) Neko::ImGuiWrap::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (std::map<K, V>)) Neko::ImGuiWrap::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
// todo insert
DEFINE_IMGUI_END()
DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (const std::map<K, V>)) Neko::ImGuiWrap::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (std::unordered_map<K, V>)) Neko::ImGuiWrap::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
// todo insert
DEFINE_IMGUI_END()
DEFINE_IMGUI_BEGIN_P((template <typename K, typename V>), (const std::unordered_map<K, V>))
Neko::ImGuiWrap::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name, var);
DEFINE_IMGUI_END()

DEFINE_IMGUI_INLINE(template <>, std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););
DEFINE_IMGUI_INLINE(template <>, const std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););

DEFINE_IMGUI_BEGIN(template <>, vec2) { ImGui::Text("%s: %f,%f", name.c_str(), var.x, var.y); }
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, String) { ImGui::Text("%s", var.cstr()); }
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, mat3) { ImGui::Text("%s:\n%f %f %f\n%f %f %f\n%f %f %f\n", name.c_str(), var.v[0], var.v[1], var.v[2], var.v[3], var.v[4], var.v[5], var.v[6], var.v[7], var.v[8]); }
DEFINE_IMGUI_END()