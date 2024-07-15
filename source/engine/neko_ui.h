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

#include "engine/neko.hpp"
#include "engine/neko_base.h"
#include "engine/neko_prelude.h"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// sokol-imgui
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <util/sokol_imgui.h>

#define LUI_COMMANDLIST_SIZE (256 * 1024)
#define LUI_ROOTLIST_SIZE 32
#define LUI_CONTAINERSTACK_SIZE 32
#define LUI_CLIPSTACK_SIZE 32
#define LUI_IDSTACK_SIZE 32
#define LUI_LAYOUTSTACK_SIZE 16
#define LUI_CONTAINERPOOL_SIZE 48
#define LUI_TREENODEPOOL_SIZE 48
#define LUI_MAX_WIDTHS 16
#define LUI_REAL float
#define LUI_REAL_FMT "%.3g"
#define LUI_SLIDER_FMT "%.2f"
#define LUI_MAX_FMT 127

#define lui_stack(T, n) \
    struct {            \
        int idx;        \
        T items[n];     \
    }
#define lui_min(a, b) ((a) < (b) ? (a) : (b))
#define lui_max(a, b) ((a) > (b) ? (a) : (b))
#define lui_clamp(x, a, b) lui_min(b, lui_max(a, x))

enum { LUI_CLIP_PART = 1, LUI_CLIP_ALL };

enum { LUI_COMMAND_JUMP = 1, LUI_COMMAND_CLIP, LUI_COMMAND_RECT, LUI_COMMAND_TEXT, LUI_COMMAND_ICON, LUI_COMMAND_MAX };

enum {
    LUI_COLOR_TEXT,
    LUI_COLOR_BORDER,
    LUI_COLOR_WINDOWBG,
    LUI_COLOR_TITLEBG,
    LUI_COLOR_TITLETEXT,
    LUI_COLOR_PANELBG,
    LUI_COLOR_BUTTON,
    LUI_COLOR_BUTTONHOVER,
    LUI_COLOR_BUTTONFOCUS,
    LUI_COLOR_BASE,
    LUI_COLOR_BASEHOVER,
    LUI_COLOR_BASEFOCUS,
    LUI_COLOR_SCROLLBASE,
    LUI_COLOR_SCROLLTHUMB,
    LUI_COLOR_MAX
};

enum { LUI_ICON_CLOSE = 1, LUI_ICON_CHECK, LUI_ICON_COLLAPSED, LUI_ICON_EXPANDED, LUI_ICON_MAX };

enum { LUI_RES_ACTIVE = (1 << 0), LUI_RES_SUBMIT = (1 << 1), LUI_RES_CHANGE = (1 << 2) };

enum {
    LUI_OPT_ALIGNCENTER = (1 << 0),
    LUI_OPT_ALIGNRIGHT = (1 << 1),
    LUI_OPT_NOINTERACT = (1 << 2),
    LUI_OPT_NOFRAME = (1 << 3),
    LUI_OPT_NORESIZE = (1 << 4),
    LUI_OPT_NOSCROLL = (1 << 5),
    LUI_OPT_NOCLOSE = (1 << 6),
    LUI_OPT_NOTITLE = (1 << 7),
    LUI_OPT_HOLDFOCUS = (1 << 8),
    LUI_OPT_AUTOSIZE = (1 << 9),
    LUI_OPT_POPUP = (1 << 10),
    LUI_OPT_CLOSED = (1 << 11),
    LUI_OPT_EXPANDED = (1 << 12)
};

enum { LUI_MOUSE_LEFT = (1 << 0), LUI_MOUSE_RIGHT = (1 << 1), LUI_MOUSE_MIDDLE = (1 << 2) };

enum { LUI_KEY_SHIFT = (1 << 0), LUI_KEY_CTRL = (1 << 1), LUI_KEY_ALT = (1 << 2), LUI_KEY_BACKSPACE = (1 << 3), LUI_KEY_RETURN = (1 << 4) };

typedef struct lui_Context lui_Context;
typedef unsigned lui_Id;
typedef LUI_REAL lui_Real;
typedef void *lui_Font;

typedef struct {
    int x, y;
} lui_Vec2;
typedef struct {
    int x, y, w, h;
} lui_Rect;
typedef struct {
    unsigned char r, g, b, a;
} lui_Color;
typedef struct {
    lui_Id id;
    int last_update;
} lui_PoolItem;

typedef struct {
    int type, size;
} lui_BaseCommand;
typedef struct {
    lui_BaseCommand base;
    void *dst;
} lui_JumpCommand;
typedef struct {
    lui_BaseCommand base;
    lui_Rect rect;
} lui_ClipCommand;
typedef struct {
    lui_BaseCommand base;
    lui_Rect rect;
    lui_Color color;
} lui_RectCommand;
typedef struct {
    lui_BaseCommand base;
    lui_Font font;
    lui_Vec2 pos;
    lui_Color color;
    char str[1];
} lui_TextCommand;
typedef struct {
    lui_BaseCommand base;
    lui_Rect rect;
    int id;
    lui_Color color;
} lui_IconCommand;

typedef union {
    int type;
    lui_BaseCommand base;
    lui_JumpCommand jump;
    lui_ClipCommand clip;
    lui_RectCommand rect;
    lui_TextCommand text;
    lui_IconCommand icon;
} lui_Command;

typedef struct {
    lui_Rect body;
    lui_Rect next;
    lui_Vec2 position;
    lui_Vec2 size;
    lui_Vec2 max;
    int widths[LUI_MAX_WIDTHS];
    int items;
    int item_index;
    int next_row;
    int next_type;
    int indent;
} lui_Layout;

typedef struct {
    lui_Command *head, *tail;
    lui_Rect rect;
    lui_Rect body;
    lui_Vec2 content_size;
    lui_Vec2 scroll;
    int zindex;
    int open;
} lui_Container;

typedef struct {
    lui_Font font;
    lui_Vec2 size;
    int padding;
    int spacing;
    int indent;
    int title_height;
    int scrollbar_size;
    int thumb_size;
    lui_Color colors[LUI_COLOR_MAX];
} lui_Style;

struct lui_Context {
    /* callbacks */
    int (*text_width)(lui_Font font, const char *str, int len);
    int (*text_height)(lui_Font font);
    void (*draw_frame)(lui_Context *ctx, lui_Rect rect, int colorid);
    /* core state */
    lui_Style _style;
    lui_Style *style;
    lui_Id hover;
    lui_Id focus;
    lui_Id last_id;
    lui_Rect last_rect;
    int last_zindex;
    int updated_focus;
    int frame;
    lui_Container *hover_root;
    lui_Container *next_hover_root;
    lui_Container *scroll_target;
    char number_edit_buf[LUI_MAX_FMT];
    lui_Id number_edit;
    /* stacks */
    lui_stack(char, LUI_COMMANDLIST_SIZE) command_list;
    lui_stack(lui_Container *, LUI_ROOTLIST_SIZE) root_list;
    lui_stack(lui_Container *, LUI_CONTAINERSTACK_SIZE) container_stack;
    lui_stack(lui_Rect, LUI_CLIPSTACK_SIZE) clip_stack;
    lui_stack(lui_Id, LUI_IDSTACK_SIZE) id_stack;
    lui_stack(lui_Layout, LUI_LAYOUTSTACK_SIZE) layout_stack;
    /* retained state pools */
    lui_PoolItem container_pool[LUI_CONTAINERPOOL_SIZE];
    lui_Container containers[LUI_CONTAINERPOOL_SIZE];
    lui_PoolItem treenode_pool[LUI_TREENODEPOOL_SIZE];
    /* input state */
    lui_Vec2 mouse_pos;
    lui_Vec2 last_mouse_pos;
    lui_Vec2 mouse_delta;
    lui_Vec2 scroll_delta;
    int mouse_down;
    int mouse_pressed;
    int key_down;
    int key_pressed;
    char input_text[32];
};

lui_Vec2 lui_vec2(int x, int y);
lui_Rect lui_rect(int x, int y, int w, int h);
lui_Color lui_color(int r, int g, int b, int a);

void lui_init(lui_Context *ctx);
void lui_begin(lui_Context *ctx);
void lui_end(lui_Context *ctx);
void lui_set_focus(lui_Context *ctx, lui_Id id);
lui_Id lui_get_id(lui_Context *ctx, const void *data, int size);
void lui_push_id(lui_Context *ctx, const void *data, int size);
void lui_pop_id(lui_Context *ctx);
void lui_push_clip_rect(lui_Context *ctx, lui_Rect rect);
void lui_pop_clip_rect(lui_Context *ctx);
lui_Rect lui_get_clip_rect(lui_Context *ctx);
int lui_check_clip(lui_Context *ctx, lui_Rect r);
lui_Container *lui_get_current_container(lui_Context *ctx);
lui_Container *lui_get_container(lui_Context *ctx, const char *name);
void lui_bring_to_front(lui_Context *ctx, lui_Container *cnt);

int lui_pool_init(lui_Context *ctx, lui_PoolItem *items, int len, lui_Id id);
int lui_pool_get(lui_Context *ctx, lui_PoolItem *items, int len, lui_Id id);
void lui_pool_update(lui_Context *ctx, lui_PoolItem *items, int idx);

void lui_input_mousemove(lui_Context *ctx, int x, int y);
void lui_input_mousedown(lui_Context *ctx, int x, int y, int btn);
void lui_input_mouseup(lui_Context *ctx, int x, int y, int btn);
void lui_input_scroll(lui_Context *ctx, int x, int y);
void lui_input_keydown(lui_Context *ctx, int key);
void lui_input_keyup(lui_Context *ctx, int key);
void lui_input_text(lui_Context *ctx, const char *text);

lui_Command *lui_push_command(lui_Context *ctx, int type, int size);
int lui_next_command(lui_Context *ctx, lui_Command **cmd);
void lui_set_clip(lui_Context *ctx, lui_Rect rect);
void lui_draw_rect(lui_Context *ctx, lui_Rect rect, lui_Color color);
void lui_draw_box(lui_Context *ctx, lui_Rect rect, lui_Color color);
void lui_draw_text(lui_Context *ctx, lui_Font font, const char *str, int len, lui_Vec2 pos, lui_Color color);
void lui_draw_icon(lui_Context *ctx, int id, lui_Rect rect, lui_Color color);

void lui_layout_row(lui_Context *ctx, int items, const int *widths, int height);
void lui_layout_width(lui_Context *ctx, int width);
void lui_layout_height(lui_Context *ctx, int height);
void lui_layout_begin_column(lui_Context *ctx);
void lui_layout_end_column(lui_Context *ctx);
void lui_layout_set_next(lui_Context *ctx, lui_Rect r, int relative);
lui_Rect lui_layout_next(lui_Context *ctx);

void lui_draw_control_frame(lui_Context *ctx, lui_Id id, lui_Rect rect, int colorid, int opt);
void lui_draw_control_text(lui_Context *ctx, const char *str, lui_Rect rect, int colorid, int opt);
int lui_mouse_over(lui_Context *ctx, lui_Rect rect);
void lui_update_control(lui_Context *ctx, lui_Id id, lui_Rect rect, int opt);

#define lui_button(ctx, label) lui_button_ex(ctx, label, 0, LUI_OPT_ALIGNCENTER)
#define lui_textbox(ctx, buf, bufsz) lui_textbox_ex(ctx, buf, bufsz, 0)
#define lui_slider(ctx, value, lo, hi) lui_slider_ex(ctx, value, lo, hi, 0, LUI_SLIDER_FMT, LUI_OPT_ALIGNCENTER)
#define lui_number(ctx, value, step) lui_number_ex(ctx, value, step, LUI_SLIDER_FMT, LUI_OPT_ALIGNCENTER)
#define lui_header(ctx, label) lui_header_ex(ctx, label, 0)
#define lui_begin_treenode(ctx, label) lui_begin_treenode_ex(ctx, label, 0)
#define lui_begin_window(ctx, title, rect) lui_begin_window_ex(ctx, title, rect, 0)
#define lui_begin_panel(ctx, name) lui_begin_panel_ex(ctx, name, 0)

void lui_text(lui_Context *ctx, const char *text);
void lui_label(lui_Context *ctx, const char *text);
int lui_button_ex(lui_Context *ctx, const char *label, int icon, int opt);
int lui_checkbox(lui_Context *ctx, const char *label, int *state);
int lui_textbox_raw(lui_Context *ctx, char *buf, int bufsz, lui_Id id, lui_Rect r, int opt);
int lui_textbox_ex(lui_Context *ctx, char *buf, int bufsz, int opt);
int lui_slider_ex(lui_Context *ctx, lui_Real *value, lui_Real low, lui_Real high, lui_Real step, const char *fmt, int opt);
int lui_number_ex(lui_Context *ctx, lui_Real *value, lui_Real step, const char *fmt, int opt);
int lui_header_ex(lui_Context *ctx, const char *label, int opt);
int lui_begin_treenode_ex(lui_Context *ctx, const char *label, int opt);
void lui_end_treenode(lui_Context *ctx);
int lui_begin_window_ex(lui_Context *ctx, const char *title, lui_Rect rect, int opt);
void lui_end_window(lui_Context *ctx);
void lui_open_popup(lui_Context *ctx, const char *name);
int lui_begin_popup(lui_Context *ctx, const char *name);
void lui_end_popup(lui_Context *ctx);
void lui_begin_panel_ex(lui_Context *ctx, const char *name, int opt);
void lui_end_panel(lui_Context *ctx);

struct sapp_event;

lui_Context *microui_ctx();
void microui_init();
void microui_trash();
void microui_sokol_event(const sapp_event *e);
void microui_begin();
void microui_end_and_present();

struct lua_State;
lui_Rect lua_lui_check_rect(lua_State *L, i32 arg);
void lua_lui_rect_push(lua_State *L, lui_Rect rect);
lui_Color lua_lui_check_color(lua_State *L, i32 arg);

enum MUIRefKind : i32 {
    MUIRefKind_Nil,
    MUIRefKind_Boolean,
    MUIRefKind_Real,
    MUIRefKind_String,
};

struct MUIRef {
    MUIRefKind kind;
    union {
        int boolean;
        lui_Real real;
        char string[512];
    };
};

void lua_lui_set_ref(lua_State *L, MUIRef *ref, i32 arg);
MUIRef *lua_lui_check_ref(lua_State *L, i32 arg, MUIRefKind kind);

#define neko_imgui_tree_max_elementsize sizeof(std::string)
#define neko_imgui_tree_max_tuple 3

// NEKO_STATIC_INLINE ImVec4 vec4_to_imvec4(const neko_vec4 &v4) { return {v4.x, v4.y, v4.z, v4.w}; }
// NEKO_STATIC_INLINE ImColor vec4_to_imcolor(const neko_vec4 &v4) { return {v4.x * 255.0f, v4.y * 255.0f, v4.z * 255.0f, v4.w * 255.0f}; }

// NEKO_INLINE neko_color_t imvec_to_rgba(ImVec4 iv) {
//     u8 newr = iv.x * 255;
//     u8 newg = iv.y * 255;
//     u8 newb = iv.z * 255;
//     u8 newa = iv.w * 255;
//     return neko_color_t{newr, newg, newb, newa};
// }

NEKO_INLINE ImVec4 rgba_to_imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

namespace neko::cpp {
// same as std::as_const in c++17
template <class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept {
    return t;
}
}  // namespace neko::cpp

namespace neko::imgui {

// 这就是这个库实现的功能 只是类 neko::imgui::Auto_t<T> 的包装
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
    neko::imgui::detail::AutoExpand(str, neko::cpp::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string &name, std::tuple<Args...> &tpl) {
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
void neko::imgui::detail::AutoTuple(const std::string &name,
                                    const std::tuple<Args...> &tpl)  // same but const
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
        static void Auto(NEKO_VA_UNPACK typespec &var, const std::string &name) {

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

neko_imgui_def_inline(template <>, char *, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, char *const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, const_str const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

DEFINE_IMGUI_BEGIN(template <>, std::string) const std::size_t lines = var.find('\n');
if (var.find('\n') != std::string::npos)
    ImGui::InputTextMultiline(name.c_str(), const_cast<char *>(var.c_str()), 256);
else
    ImGui::InputText(name.c_str(), const_cast<char *>(var.c_str()), 256);
DEFINE_IMGUI_END();
DEFINE_IMGUI_BEGIN(template <>, const std::string) if (name.empty()) ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
else ImGui::Text("%s=%s", name.c_str(), var.c_str());
DEFINE_IMGUI_END();

neko_imgui_def_inline(template <>, float, ImGui::DragFloat(name.c_str(), &var););
neko_imgui_def_inline(template <>, int, ImGui::InputInt(name.c_str(), &var););
neko_imgui_def_inline(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int *)&var););
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

DEFINE_IMGUI_BEGIN(template <typename T>, T *) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
DEFINE_IMGUI_END();

DEFINE_IMGUI_BEGIN(template <typename T>, T *const) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
DEFINE_IMGUI_END();

neko_imgui_def_inline_p((template <typename T, std::size_t N>), (std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(std::array<T, N> *)(&var)););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(const std::array<T, N> *)(&var)););

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

// DEFINE_IMGUI_BEGIN(template <>, neko_vec2_t) {
//     //    neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { neko::imgui::Auto(value, std::string(field.name)); });
//     ImGui::Text("%f %f", var.x, var.y);
// }
// DEFINE_IMGUI_END();

namespace neko::imgui {

NEKO_INLINE ImVec4 neko_rgba2imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

NEKO_INLINE void neko_imgui_help_marker(const_str desc) {
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
NEKO_INLINE void neko_imgui_file_browser(std::string &path) {

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

NEKO_INLINE bool toggle(const char *label, bool *v) {
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
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

NEKO_INLINE bool button_scrollable_ex(const char *label, const ImVec2 &size_arg, ImGuiButtonFlags flags) {
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
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

NEKO_INLINE bool button_scrollable(const char *label, const ImVec2 &size_arg) { return button_scrollable_ex(label, size_arg, ImGuiButtonFlags_None); }

struct InputTextCallback_UserData {
    std::string *Str;
    ImGuiInputTextCallback ChainCallback;
    void *ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData *data) {
    InputTextCallback_UserData *user_data = (InputTextCallback_UserData *)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string *str = user_data->Str;
        NEKO_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char *)str->c_str();
    } else if (user_data->ChainCallback) {
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

NEKO_INLINE bool InputText(const char *label, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputText(label, (char *)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

NEKO_INLINE bool InputTextMultiline(const char *label, std::string *str, const ImVec2 &size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                                    void *user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputTextMultiline(label, (char *)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

NEKO_INLINE bool InputTextWithHint(const char *label, const char *hint, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputTextWithHint(label, hint, (char *)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

template <typename T, typename... Args>
NEKO_INLINE void TextFmt(T &&fmt, const Args &...args) {
    std::string str = std::format(std::forward<T>(fmt), args...);
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}

}  // namespace neko::imgui