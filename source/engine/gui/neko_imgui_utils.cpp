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

#include "engine/base/neko_engine.h"
#include "engine/common/neko_util.h"
#include "engine/gui/imgui_impl/imgui_impl_glfw.h"
#include "engine/gui/imgui_impl/imgui_impl_opengl3.h"
#include "engine/platform/neko_platform.h"
#include "libs/glad/glad.h"

namespace neko::imgui {

bool color_picker_3U32(const char* label, ImU32* color, ImGuiColorEditFlags flags) {
    float col[3];
    col[0] = (float)((*color >> 0) & 0xFF) / 255.0f;
    col[1] = (float)((*color >> 8) & 0xFF) / 255.0f;
    col[2] = (float)((*color >> 16) & 0xFF) / 255.0f;

    bool result = ImGui::ColorPicker3(label, col, flags);

    *color = ((ImU32)(col[0] * 255.0f)) | ((ImU32)(col[1] * 255.0f) << 8) | ((ImU32)(col[2] * 255.0f) << 16);

    return result;
}

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
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
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
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = borderColor;
    colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    // colors[ImGuiCol_ModalWindowDarkening] = bgColor;
    colors[ImGuiCol_DragDropTarget] = bgColor;
    colors[ImGuiCol_NavHighlight] = bgColor;
    colors[ImGuiCol_DockingPreview] = panelActiveColor;
    colors[ImGuiCol_Tab] = bgColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = bgColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;

    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
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

void neko_draw_text(std::string text, neko_color_t col, int x, int y, bool outline, neko_color_t outline_col) {

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList(viewport);

    if (outline) {

        auto outline_col_im = ImColor(outline_col.r, outline_col.g, outline_col.b, col.a);

        draw_list->AddText(ImVec2(x + 0, y - 1), outline_col_im, text.c_str());  // up
        draw_list->AddText(ImVec2(x + 0, y + 1), outline_col_im, text.c_str());  // down
        draw_list->AddText(ImVec2(x + 1, y + 0), outline_col_im, text.c_str());  // right
        draw_list->AddText(ImVec2(x - 1, y + 0), outline_col_im, text.c_str());  // left

        draw_list->AddText(ImVec2(x + 1, y + 1), outline_col_im, text.c_str());  // down-right
        draw_list->AddText(ImVec2(x - 1, y + 1), outline_col_im, text.c_str());  // down-left

        draw_list->AddText(ImVec2(x + 1, y - 1), outline_col_im, text.c_str());  // up-right
        draw_list->AddText(ImVec2(x - 1, y - 1), outline_col_im, text.c_str());  // up-left
    }

    draw_list->AddText(ImVec2(x, y), ImColor(col.r, col.g, col.b, col.a), text.c_str());  // base
}

}  // namespace neko

namespace neko {

neko_global std::size_t g_imgui_mem_usage = 0;

neko_private(void*) __neko_imgui_malloc(size_t sz, void* user_data) { return __neko_mem_safe_alloc((sz), (char*)__FILE__, __LINE__, &g_imgui_mem_usage); }

neko_private(void) __neko_imgui_free(void* ptr, void* user_data) { __neko_mem_safe_free(ptr, &g_imgui_mem_usage); }

std::size_t __neko_imgui_meminuse() { return g_imgui_mem_usage; }

void neko_imgui_init() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Get main window from platform
    GLFWwindow* win = (GLFWwindow*)platform->raw_window_handle(platform->main_window());

    ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.IniFilename = "imgui.ini";

    io.Fonts->AddFontFromFileTTF(neko_file_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf"), 20.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    neko_imgui_style();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init();
}

void neko_imgui_new_frame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void neko_imgui_render() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // TODO: 将这一切抽象出来并通过命令缓冲系统渲染
    ImGui::Render();
    neko_vec2 fbs = platform->frame_buffer_size(platform->main_window());
    glViewport(0, 0, fbs.x, fbs.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void neko_imgui_destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

}  // namespace neko