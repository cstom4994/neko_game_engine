#pragma once

#include "engine/imgui.hpp"

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

namespace Neko::ImGuiWrap {
template <>
struct Auto_t<const_str> {
    static void Auto(const_str &var, const std::string &name) {
        if (name.empty())
            ImGui::TextUnformatted(var);
        else
            ImGui::Text("%s=%s", name.c_str(), var);
    }
};

template <std::size_t N>
struct Auto_t<const detail::c_array_t<char, N>> {
    static void Auto(const detail::c_array_t<char, N> &var, const std::string &name) {
        if (name.empty())
            ImGui::TextUnformatted(var, var + N - 1);
        else
            ImGui::Text("%s=%s", name.c_str(), var);
    }
};

// DEFINE_IMGUI_INLINE(template <>, char *, const_str tmp = var; Neko::ImGuiWrap::Auto_t<const_str>::Auto(tmp, name););
//
// DEFINE_IMGUI_INLINE(template <>, char *const, const_str tmp = var; Neko::ImGuiWrap::Auto_t<const_str>::Auto(tmp, name););
//
// DEFINE_IMGUI_INLINE(template <>, const_str const, const_str tmp = var; Neko::ImGuiWrap::Auto_t<const_str>::Auto(tmp, name););

template <>
struct Auto_t<std::string> {
    static void Auto(std::string &var, const std::string &name) {
        const std::size_t lines = var.find('\n');
        if (var.find('\n') != std::string::npos)
            ImGui::InputTextMultiline(name.c_str(), const_cast<char *>(var.c_str()), 256);
        else
            ImGui::InputText(name.c_str(), const_cast<char *>(var.c_str()), 256);
    }
};

template <>
struct Auto_t<const std::string> {
    static void Auto(const std::string &var, const std::string &name) {
        if (name.empty())
            ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
        else
            ImGui::Text("%s=%s", name.c_str(), var.c_str());
    }
};
}  // namespace Neko::ImGuiWrap

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

namespace Neko::ImGuiWrap {
template <typename T>
struct Auto_t<std::vector<T>> {
    static void Auto(std::vector<T> &var, const std::string &name) {
        if (Neko::ImGuiWrap::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
            ImGui::PushID(name.c_str());
            ImGui::Indent();
            Neko::ImGuiWrap::detail::AutoContainerPushBackButton(var);
            if (!var.empty()) ImGui::SameLine();
            Neko::ImGuiWrap::detail::AutoContainerPopBackButton(var);
            ImGui::PopID();
            ImGui::Unindent();
        }
    }
};

template <>
struct Auto_t<std::vector<bool>> {
    static void Auto(std::vector<bool> &var, const std::string &name) {
        if (Neko::ImGuiWrap::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
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
    }
};

template <typename T>
struct Auto_t<const std::vector<T>> {
    static void Auto(const std::vector<T> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var); }
};

template <>
struct Auto_t<const std::vector<bool>> {
    static void Auto(const std::vector<bool> &var, const std::string &name) {
        if (Neko::ImGuiWrap::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
            ImGui::Indent();
            for (int i = 0; i < var.size(); ++i) {
                ImGui::Bullet();
                Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
            }
            ImGui::Unindent();
        }
    }
};

template <typename T>
struct Auto_t<std::list<T>> {
    static void Auto(std::list<T> &var, const std::string &name) {
        if (Neko::ImGuiWrap::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
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
    }
};

template <typename T>
struct Auto_t<const std::list<T>> {
    static void Auto(const std::list<T> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoContainerValues<const std::list<T>>("List " + name, var); }
};

template <typename T>
struct Auto_t<std::deque<T>> {
    static void Auto(std::deque<T> &var, const std::string &name) {
        if (Neko::ImGuiWrap::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
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
    }
};

template <typename T>
struct Auto_t<const std::deque<T>> {
    static void Auto(const std::deque<T> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var); }
};

template <typename T>
struct Auto_t<std::forward_list<T>> {
    static void Auto(std::forward_list<T> &var, const std::string &name) {
        if (Neko::ImGuiWrap::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
            ImGui::PushID(name.c_str());
            ImGui::Indent();
            Neko::ImGuiWrap::detail::AutoContainerPushFrontButton(var);
            if (!var.empty()) ImGui::SameLine();
            Neko::ImGuiWrap::detail::AutoContainerPopFrontButton(var);
            ImGui::PopID();
            ImGui::Unindent();
        }
    }
};

template <typename T>
struct Auto_t<const std::forward_list<T>> {
    static void Auto(const std::forward_list<T> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var); }
};

template <typename T>
struct Auto_t<std::set<T>> {
    static void Auto(std::set<T> &var, const std::string &name) {
        Neko::ImGuiWrap::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
        // todo insert
    }
};

template <typename T>
struct Auto_t<const std::set<T>> {
    static void Auto(const std::set<T> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoContainerValues<const std::set<T>>("Set " + name, var); }
};

template <typename T>
struct Auto_t<std::unordered_set<T>> {
    static void Auto(std::unordered_set<T> &var, const std::string &name) {
        Neko::ImGuiWrap::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
        // todo insert
    }
};

template <typename T>
struct Auto_t<const std::unordered_set<T>> {
    static void Auto(const std::unordered_set<T> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var); }
};

template <typename K, typename V>
struct Auto_t<std::map<K, V>> {
    static void Auto(std::map<K, V> &var, const std::string &name) {
        Neko::ImGuiWrap::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
        // todo insert
    }
};

template <typename K, typename V>
struct Auto_t<const std::map<K, V>> {
    static void Auto(const std::map<K, V> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var); }
};

template <typename K, typename V>
struct Auto_t<std::unordered_map<K, V>> {
    static void Auto(std::unordered_map<K, V> &var, const std::string &name) {
        Neko::ImGuiWrap::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
        // todo insert
    }
};

template <typename K, typename V>
struct Auto_t<const std::unordered_map<K, V>> {
    static void Auto(const std::unordered_map<K, V> &var, const std::string &name) { Neko::ImGuiWrap::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name, var); }
};
}  // namespace Neko::ImGuiWrap

DEFINE_IMGUI_INLINE(template <>, std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););
DEFINE_IMGUI_INLINE(template <>, const std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););

DEFINE_IMGUI_BEGIN(template <>, vec2) { ImGui::Text("%s: %f,%f", name.c_str(), var.x, var.y); }
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, String) { ImGui::Text("%s", var.cstr()); }
DEFINE_IMGUI_END()

DEFINE_IMGUI_BEGIN(template <>, mat3) { ImGui::Text("%s:\n%f %f %f\n%f %f %f\n%f %f %f\n", name.c_str(), var.v[0], var.v[1], var.v[2], var.v[3], var.v[4], var.v[5], var.v[6], var.v[7], var.v[8]); }
DEFINE_IMGUI_END()