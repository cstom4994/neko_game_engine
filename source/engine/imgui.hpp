#pragma once

#include <malloc.h>

#include "base/common/base.hpp"
#include "base/scripting/scripting.h"

// imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

namespace Neko {

namespace imgui {

inline ImVec4 rgba_to_imvec(int r, int g, int b, int a = 255) {
    f32 newr = r / 255.f;
    f32 newg = g / 255.f;
    f32 newb = b / 255.f;
    f32 newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

static inline void OutlineTextColoredV(ImDrawList* drawList, const ImVec2& pos, ImU32 text_color, ImU32 outline_color, const char* fmt, va_list args) {
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

static inline void OutlineTextV(ImDrawList* drawList, const ImVec2& pos, const char* fmt, va_list args) { OutlineTextColoredV(drawList, pos, IM_COL32_WHITE, IM_COL32_BLACK, fmt, args); }

static inline void OutlineTextColored(ImDrawList* drawList, const ImVec2& pos, ImU32 text_color, ImU32 outline_color, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    OutlineTextColoredV(drawList, pos, text_color, outline_color, fmt, args);
    va_end(args);
}

static inline void OutlineText(ImDrawList* drawList, const ImVec2& pos, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    OutlineTextColoredV(drawList, pos, IM_COL32_WHITE, IM_COL32_BLACK, fmt, args);
    va_end(args);
}

}  // namespace imgui

}  // namespace Neko

namespace Neko::imgui::util {

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

}  // namespace Neko::imgui::util

struct GLFWwindow;

void imgui_draw_post();
void imgui_draw_pre();
void imgui_init(GLFWwindow* window);
void imgui_fini();

int open_imgui(lua_State* L);
