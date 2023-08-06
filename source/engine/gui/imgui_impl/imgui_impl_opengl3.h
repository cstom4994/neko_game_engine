// Copyright(c) 2022-2023, KaoruXun

// This source file include
// https://github.com/ocornut/imgui (MIT) by Omar Cornut

#pragma once

#include "engine/gui/neko_imgui_utils.hpp"

#ifndef IMGUI_DISABLE

bool ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
void ImGui_ImplOpenGL3_Shutdown();
void ImGui_ImplOpenGL3_NewFrame();
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

bool ImGui_ImplOpenGL3_CreateFontsTexture();
void ImGui_ImplOpenGL3_DestroyFontsTexture();
bool ImGui_ImplOpenGL3_CreateDeviceObjects();
void ImGui_ImplOpenGL3_DestroyDeviceObjects();

#endif  // #ifndef IMGUI_DISABLE
