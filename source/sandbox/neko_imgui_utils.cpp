#include "neko_imgui_utils.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "libs/glad/glad.h"

// glfw
#include <GLFW/glfw3.h>

namespace neko::imgui {

void file_browser(std::string& path) {

    ImGui::Text("Current Path: %s", path.c_str());
    ImGui::Separator();

    if (ImGui::Button("Parent Directory")) {
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
            if (ImGui::Selectable((filename + "/").c_str())) path = entry_path.string();

        } else {
            if (ImGui::Selectable(filename.c_str())) path = entry_path.string();
        }
    }
}

}  // namespace neko::imgui

namespace neko {

void neko_imgui_style() {

    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 bgColor = neko_imgui_color_from_byte(37, 37, 38);
    const ImVec4 lightBgColor = neko_imgui_color_from_byte(82, 82, 85);
    const ImVec4 veryLightBgColor = neko_imgui_color_from_byte(90, 90, 95);

    const ImVec4 panelColor = neko_imgui_color_from_byte(51, 51, 55);
    const ImVec4 panelHoverColor = neko_imgui_color_from_byte(29, 151, 236);
    const ImVec4 panelActiveColor = neko_imgui_color_from_byte(0, 119, 200);

    const ImVec4 textColor = neko_imgui_color_from_byte(255, 255, 255);
    const ImVec4 textDisabledColor = neko_imgui_color_from_byte(151, 151, 151);
    const ImVec4 borderColor = neko_imgui_color_from_byte(78, 78, 78);

    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    colors[ImGuiCol_WindowBg] = bgColor;
    // colors[ImGuiCol_ChildBg] = bgColor;
    // colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    // colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    colors[ImGuiCol_MenuBarBg] = panelColor;
    colors[ImGuiCol_ScrollbarBg] = panelColor;
    colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    colors[ImGuiCol_CheckMark] = panelActiveColor;
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelHoverColor;
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;
    // colors[ImGuiCol_Separator] = borderColor;
    // colors[ImGuiCol_SeparatorHovered] = borderColor;
    // colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;

    style.WindowRounding = 4.0f;
    // style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    // style.GrabRounding = 4.0f;
    // style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 0.0f;
}

void neko_draw_text_plate(std::string text, neko_color_t col, int x, int y, neko_color_t backcolor) {
    auto text_size = ImGui::CalcTextSize(text.c_str());
    // R_RectangleFilled(target, x - 4, y - 4, x + text_size.x + 4, y + text_size.y + 4, backcolor);
    neko_draw_text(text, col, x, y);
}

u32 neko_draw_darken_color(u32 color, float brightness) {
    int a = (color >> 24) & 0xFF;
    int r = (int)(((color >> 16) & 0xFF) * brightness);
    int g = (int)(((color >> 8) & 0xFF) * brightness);
    int b = (int)((color & 0xFF) * brightness);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

void neko_draw_text(std::string text, neko_color_t col, int x, int y, bool outline, neko_color_t outline_col) {}

}  // namespace neko
