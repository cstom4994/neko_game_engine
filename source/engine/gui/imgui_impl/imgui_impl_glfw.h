// Copyright(c) 2022-2023, KaoruXun

// This source file include
// https://github.com/ocornut/imgui (MIT) by Omar Cornut

#pragma once

#include "engine/gui/neko_imgui_utils.hpp"

#ifndef IMGUI_DISABLE

struct GLFWwindow;
struct GLFWmonitor;

bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
bool ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks);
bool ImGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks);
void ImGui_ImplGlfw_Shutdown();
void ImGui_ImplGlfw_NewFrame();
void ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);
void ImGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window);
void ImGui_ImplGlfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);
void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused);
void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered);
void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y);
void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event);

#endif  // #ifndef IMGUI_DISABLE
