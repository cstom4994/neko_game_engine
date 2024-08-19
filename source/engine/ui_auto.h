#pragma once

#include <filesystem>

#include "engine/game.h"

#if 1

#include <array>

#include "engine/ui.h"

#define neko_imgui_tree_max_elementsize sizeof(std::string)
#define neko_imgui_tree_max_tuple 3

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
    ui_context_t* ui = &g_app->ui;
    if (sizeof(T) <= neko_imgui_tree_max_elementsize) {
        ui_push_id(ui, name.c_str());
        neko::imgui::Auto_t<T>::Auto(value, name);
        ui_pop_id(ui);
        return true;
    } else if (ui_treenode_begin(ui, name.c_str())) {
        neko::imgui::Auto_t<T>::Auto(value, name);
        ui_treenode_end(ui);
        return true;
    } else
        return false;
}

template <typename Container>
bool neko::imgui::detail::AutoContainerTreeNode(const std::string& name, Container& cont) {
    // std::size_t size = neko::imgui::detail::AutoContainerSize(cont);
    ui_context_t* ui = &g_app->ui;
    std::size_t size = cont.size();
    if (ui_treenode_begin(ui, name.c_str())) {
        size_t elemsize = sizeof(decltype(*std::begin(cont)));
        ui_labelf("大小: %d, 非动态元素大小: %d bytes", (int)size, (int)elemsize);
        ui_treenode_end(ui);
        return true;
    } else {
        // f32 label_width = ImGui::CalcTextSize(name.c_str()).x + ImGui::GetTreeNodeToLabelSpacing() + 5;
        std::string sizetext = "(大小 = " + std::to_string(size) + ')';
        // f32 sizet_width = ImGui::CalcTextSize(sizetext.c_str()).x;
        // f32 avail_width = ImGui::GetContentRegionAvail().x;
        // if (avail_width > label_width + sizet_width) {
        //     ImGui::SameLine(avail_width - sizet_width);
        //     ui_text(ui,sizetext.c_str());
        // }
        ui_labelf(sizetext.c_str());
        return false;
    }
}
template <typename Container>
bool neko::imgui::detail::AutoContainerValues(const std::string& name, Container& cont) {
    ui_context_t* ui = &g_app->ui;
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        ui_push_id(ui, name.c_str());
        std::size_t i = 0;
        for (auto& elem : cont) {
            std::string itemname = "[" + std::to_string(i) + ']';
            neko::imgui::detail::AutoExpand(itemname, elem);
            ++i;
        }
        ui_pop_id(ui);
        return true;
    } else
        return false;
}
template <typename Container>
bool neko::imgui::detail::AutoMapContainerValues(const std::string& name, Container& cont) {
    ui_context_t* ui = &g_app->ui;
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        std::size_t i = 0;
        for (auto& elem : cont) {
            // ImGui::PushID(i);
            ui_push_id(ui, &i, sizeof(i));
            AutoMapKeyValue(elem.first, elem.second);
            ui_pop_id(ui);
            ++i;
        }
        return true;
    } else
        return false;
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushFrontButton(Container& cont) {
    ui_context_t* ui = &g_app->ui;
    if (ui_button(ui, "Push Front")) cont.emplace_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushBackButton(Container& cont) {
    ui_context_t* ui = &g_app->ui;
    if (ui_button(ui, "Push Back ")) cont.emplace_back();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopFrontButton(Container& cont) {
    ui_context_t* ui = &g_app->ui;
    if (!cont.empty() && ui_button(ui, "Pop Front ")) cont.pop_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopBackButton(Container& cont) {
    ui_context_t* ui = &g_app->ui;
    if (!cont.empty() && ui_button(ui, "Pop Back  ")) cont.pop_back();
}
template <typename Key, typename var>
void neko::imgui::detail::AutoMapKeyValue(Key& key, var& value) {
    ui_context_t* ui = &g_app->ui;
    bool b_k = sizeof(Key) <= neko_imgui_tree_max_elementsize;
    bool b_v = sizeof(var) <= neko_imgui_tree_max_elementsize;
    if (b_k) {
        ui_text(ui, "[");
        neko::imgui::Auto_t<Key>::Auto(key, "");
        ui_text(ui, "]");
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
    ui_context_t* ui = &g_app->ui;
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ui_text(ui, (name + " (" + std::to_string(tuple_size) + " bytes)").c_str());
        ui_push_id(ui, name.c_str());
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ui_pop_id(ui);
    } else if (ui_treenode_begin(ui, (name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ui_treenode_end(ui);
    }
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string& name,
                                    const std::tuple<Args...>& tpl)  // same but const
{
    ui_context_t* ui = &g_app->ui;
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ui_text(ui, (name + " !(" + std::to_string(tuple_size) + " bytes)").c_str());
        ui_push_id(ui, name.c_str());
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ui_pop_id(ui);
    } else if (ui_treenode_begin(ui, (name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ui_treenode_end(ui);
    }
}

// 在此版本中将 templatespec 和 typespec 括在括号中
#define DEFINE_UI_INSPECT(templatespec, typespec, ...)                                          \
    namespace neko::imgui {                                                                     \
    NEKO_VA_UNPACK templatespec struct Auto_t<NEKO_VA_UNPACK typespec> {                        \
        static void Auto(NEKO_VA_UNPACK typespec& var, const std::string& name) { __VA_ARGS__ } \
    };                                                                                          \
    }

DEFINE_UI_INSPECT((template <>), (const_str), ui_context_t* ui = &g_app->ui; if (name.empty()) ui_text(ui, var); else ui_labelf("%s=%s", name.c_str(), var););

DEFINE_UI_INSPECT((template <std::size_t N>), (const detail::c_array_t<char, N>), ui_context_t* ui = &g_app->ui; if (name.empty()) ui_text(ui, var /*, var + N - 1*/);
                  else ui_labelf("%s=%s", name.c_str(), var););

DEFINE_UI_INSPECT((template <>), (char*), const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

DEFINE_UI_INSPECT((template <>), (char* const), const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

DEFINE_UI_INSPECT((template <>), (const_str const), const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

DEFINE_UI_INSPECT(
        (template <>), (std::string), ui_context_t* ui = &g_app->ui; const std::size_t lines = var.find('\n'); if (var.find('\n') != std::string::npos) {
            ui_text(ui, name.c_str());
            ui_text(ui, const_cast<char*>(var.c_str()));
        } else {
            ui_text(ui, name.c_str());
            ui_input_text(ui, const_cast<char*>(var.c_str()));
        });

DEFINE_UI_INSPECT((template <>), (const std::string), ui_context_t* ui = &g_app->ui; if (name.empty()) ui_text(ui, var.c_str()); else ui_labelf("%s=%s", name.c_str(), var.c_str()););

DEFINE_UI_INSPECT((template <>), (float), ui_context_t* ui = &g_app->ui; ui_label(ui, name.c_str()); ui_slider(ui, &var, -500.f, 500.f););
// DEFINE_UI_INSPECT(template <>, int, ImGui::InputInt(name.c_str(), &var););
// DEFINE_UI_INSPECT(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int*)&var););
// DEFINE_UI_INSPECT(template <>, bool, ImGui::Checkbox(name.c_str(), &var););
// DEFINE_UI_INSPECT(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
// DEFINE_UI_INSPECT(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
// DEFINE_UI_INSPECT(template <>, const float, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
// DEFINE_UI_INSPECT(template <>, const int, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
// DEFINE_UI_INSPECT(template <>, const unsigned, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
// DEFINE_UI_INSPECT(template <>, const bool, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
// DEFINE_UI_INSPECT(template <>, const ImVec2, ui_labelf("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
// DEFINE_UI_INSPECT(template <>, const ImVec4, ui_labelf("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

// #define INTERNAL_NUM(_c, _imn)                                                                             \
//     DEFINE_UI_INSPECT(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
//     DEFINE_UI_INSPECT(template <>, const _c, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name);)

// INTERNAL_NUM(u8, U8);
// INTERNAL_NUM(u16, U16);
// INTERNAL_NUM(u64, U64);
// INTERNAL_NUM(i8, S8);
// INTERNAL_NUM(i16, S16);
// INTERNAL_NUM(i64, S64);

DEFINE_UI_INSPECT((template <>), (detail::c_array_t<float, 1>), ui_context_t* ui = &g_app->ui; ui_text(ui, name.c_str()); ui_slider(ui, &var[0], -500.f, 500.f););
DEFINE_UI_INSPECT((template <>), (const detail::c_array_t<float, 1>), ui_labelf("%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););

DEFINE_UI_INSPECT((template <>), (detail::c_array_t<int, 1>), ui_context_t* ui = &g_app->ui; ui_text(ui, name.c_str()); ui_slider(ui, (ui_real*)&var[0], -500.f, 500.f););
DEFINE_UI_INSPECT((template <>), (const detail::c_array_t<int, 1>), ui_labelf("%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););

DEFINE_UI_INSPECT((template <typename T>), (T*), ui_context_t* ui = &g_app->ui; if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var); else {
    ui_text(ui, "NULL:");
    Color256 col = NEKO_COLOR_RED;
    ui_text_colored(ui, name.c_str(), &col);
});

DEFINE_UI_INSPECT((template <typename T>), (T* const), ui_context_t* ui = &g_app->ui; if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var); else {
    ui_text(ui, "NULL:");
    Color256 col = NEKO_COLOR_RED;
    ui_text_colored(ui, name.c_str(), &col);
});

DEFINE_UI_INSPECT((template <typename T, std::size_t N>), (std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
DEFINE_UI_INSPECT((template <typename T, std::size_t N>), (const std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
DEFINE_UI_INSPECT((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(std::array<T, N>*)(&var)););
DEFINE_UI_INSPECT((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(const std::array<T, N>*)(&var)););

DEFINE_UI_INSPECT((template <typename T1, typename T2>), (std::pair<T1, T2>),
                  if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>) && (std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
                      // float width = ImGui::CalcItemWidth();
                      // ImGui::PushItemWidth(width * 0.4 - 10);  // a bit less than half
                      neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
                      neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
                      // ImGui::PopItemWidth();
                  } else {
                      neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
                      neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
                  }

);

DEFINE_UI_INSPECT((template <typename T1, typename T2>), (const std::pair<T1, T2>), neko::imgui::detail::AutoExpand<const T1>(name + ".first", var.first);
                  if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) { neko::imgui::detail::AutoExpand<const T2>(name + ".second", var.second); });

DEFINE_UI_INSPECT((template <typename... Args>), (std::tuple<Args...>), neko::imgui::detail::AutoTuple("Tuple " + name, var););
DEFINE_UI_INSPECT((template <typename... Args>), (const std::tuple<Args...>), neko::imgui::detail::AutoTuple("Tuple " + name, var););

DEFINE_UI_INSPECT((template <typename T>), (std::vector<T>), ui_context_t* ui = &g_app->ui; if (neko::imgui::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
    ui_push_id(ui, name.c_str());
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ui_pop_id(ui);
});

DEFINE_UI_INSPECT((template <>), (std::vector<bool>), ui_context_t* ui = &g_app->ui; if (neko::imgui::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
    for (int i = 0; i < var.size(); ++i) {
        bool b = var[i];
        neko::imgui::Auto_t<bool>::Auto(b, '[' + std::to_string(i) + ']');
        var[i] = b;
    }
    ui_push_id(ui, name.c_str());
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ui_pop_id(ui);
});

DEFINE_UI_INSPECT((template <typename T>), (const std::vector<T>), neko::imgui::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var););

DEFINE_UI_INSPECT((template <>), (const std::vector<bool>), ui_context_t* ui = &g_app->ui; if (neko::imgui::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
    for (int i = 0; i < var.size(); ++i) {
        neko::imgui::Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
});

DEFINE_UI_INSPECT((template <typename T>), (std::list<T>), ui_context_t* ui = &g_app->ui; if (neko::imgui::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
    ui_push_id(ui, name.c_str());
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ui_pop_id(ui);
});

DEFINE_UI_INSPECT((template <typename T>), (const std::list<T>), neko::imgui::detail::AutoContainerValues<const std::list<T>>("List " + name, var););

DEFINE_UI_INSPECT((template <typename T>), (std::deque<T>), ui_context_t* ui = &g_app->ui; if (neko::imgui::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
    ui_push_id(ui, name.c_str());
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ui_pop_id(ui);
});

DEFINE_UI_INSPECT((template <typename T>), (const std::deque<T>), neko::imgui::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var););

DEFINE_UI_INSPECT((template <typename T>), (std::forward_list<T>), ui_context_t* ui = &g_app->ui; if (neko::imgui::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
    ui_push_id(ui, name.c_str());
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    ui_pop_id(ui);
});
DEFINE_UI_INSPECT((template <typename T>), (const std::forward_list<T>), neko::imgui::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var););

// DEFINE_UI_INSPECT((template <typename T>), std::set<T>) neko::imgui::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
// // todo insert
// );
// DEFINE_UI_INSPECT((template <typename T>), const std::set<T>) neko::imgui::detail::AutoContainerValues<const std::set<T>>("Set " + name, var);
// );

// DEFINE_UI_INSPECT((template <typename T>), std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
// // todo insert
// );
// DEFINE_UI_INSPECT((template <typename T>), const std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var);
// );

// DEFINE_UI_INSPECT((template <typename K, typename V>), (std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
// // todo insert
// );
// DEFINE_UI_INSPECT((template <typename K, typename V>), (const std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var);
// );

// DEFINE_UI_INSPECT((template <typename K, typename V>), (std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
// // todo insert
// );
// DEFINE_UI_INSPECT((template <typename K, typename V>), (const std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name,
// var); );

DEFINE_UI_INSPECT((template <>), (std::add_pointer_t<void()>), ui_context_t* ui = &g_app->ui; if (ui_button(ui, name.c_str())) var(););
DEFINE_UI_INSPECT((template <>), (const std::add_pointer_t<void()>), ui_context_t* ui = &g_app->ui; if (ui_button(ui, name.c_str())) var(););

// DEFINE_UI_INSPECT(template <>, vec2_t) {
//     //    neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { neko::imgui::Auto(value, std::string(field.name)); });
//     ui_labelf("%f %f", var.x, var.y);
// }
// );

inline void ui_file_browser(std::string& path) {

    ui_context_t* ui = &g_app->ui;

    ui_labelf("Current Path: %s", path.c_str());

    if (ui_button(ui, "Parent Directory")) {
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
            if (ui_button(ui, (filename + "/").c_str())) path = entry_path.string();

        } else {
            if (ui_button(ui, filename.c_str())) path = entry_path.string();
        }
    }
}

#endif