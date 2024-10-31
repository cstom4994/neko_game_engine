#pragma once

#include <malloc.h>

#include "engine/base/base.hpp"

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

template <typename T>
static inline std::string readable_bytes(T num) {
    char buffer[64];

    if (num >= 1e15) {
        snprintf(buffer, sizeof(buffer), "%.2f PB", (float)num / 1e15);
    } else if (num >= 1e12) {
        snprintf(buffer, sizeof(buffer), "%.2f TB", (float)num / 1e12);
    } else if (num >= 1e9) {
        snprintf(buffer, sizeof(buffer), "%.2f GB", (float)num / 1e9);
    } else if (num >= 1e6) {
        snprintf(buffer, sizeof(buffer), "%.2f MB", (float)num / 1e6);
    } else if (num >= 1e3) {
        snprintf(buffer, sizeof(buffer), "%.2f kB", (float)num / 1e3);
    } else {
        snprintf(buffer, sizeof(buffer), "%.2f B", (float)num);
    }

    return std::string(buffer);
}

static inline void perf() {
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 screenSize = io.DisplaySize;

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    const ImVec2 footer_size(screenSize.x, 20);
    const ImVec2 footer_pos(0, screenSize.y - footer_size.y);
    const ImU32 background_color = IM_COL32(0, 0, 0, 102);

    draw_list->AddRectFilled(footer_pos, footer_pos + footer_size, background_color);

    OutlineText(draw_list, ImVec2(screenSize.x - 60, footer_pos.y + 3), "%.1f FPS", io.Framerate);
}

}  // namespace imgui

}  // namespace Neko

void imgui_draw_post();
void imgui_draw_pre();
void imgui_init();
void imgui_fini();
