
#ifndef NEKO_DBGUI_HPP
#define NEKO_DBGUI_HPP

#include <map>
#include <ranges>

#include "engine/common/neko_containers.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_util.h"
#include "engine/gui/neko_imgui_utils.hpp"
#include "engine/utility/logger.hpp"
#include "engine/utility/module.hpp"

namespace neko {

using namespace neko::cpp;

typedef enum { neko_dbgui_result_success, neko_dbgui_result_in_progress } neko_dbgui_result;

using neko_dbgui_func = neko_function<neko_dbgui_result(neko_dbgui_result)>;

struct neko_dbgui_win {
    neko_string name;
    neko_dbgui_func func;
};

class dbgui : public module<dbgui> {
public:
public:
    dbgui() noexcept { __init(); }
    ~dbgui() noexcept { __end(); }

    neko_move_only(dbgui);

    void __init(){};
    void __end(){};
    void __update() { draw(); };

public:
    dbgui& create(const neko_string& name, const neko_dbgui_func& f) {
        // 创建 dbgui 实例
        dbgui_list.insert(std::make_pair(name, neko_dbgui_win{name, f}));
        return *this;
    }

    dbgui& func_change(const neko_string& name, const neko_dbgui_func& f) {
        if (dbgui_list.count(name) != 1) neko_assert(0);
        auto origin = dbgui_list[name];
        dbgui_list[name] = neko_dbgui_win{origin.name, func_combine(f, origin.func)};
        return *this;
    }

    void draw() const {

        auto dock_win = []() {
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            if (viewport) {
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
            }

            const ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground;

            const ImGuiDockNodeFlags dock_node_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode;

            if (ImGui::Begin("neko_editor_dockspace_window", nullptr, window_flags)) {
                ImGui::DockSpace(ImGui::GetID("neko_editor_dockspace"), ImVec2(0.f, 0.f), dock_node_flags);

                if (ImGui::BeginMenuBar()) {
                    neko_defer([] { ImGui::EndMenuBar(); });

                    ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko");

                    if (ImGui::BeginMenu("engine")) {
                        ImGui::Checkbox("CVar", &g_cvar.ui_tweak);
                        ImGui::EndMenu();
                    }

                    char fpsText[50];
                    snprintf(fpsText, sizeof(fpsText), "%.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(fpsText).x - ImGui::GetScrollX());

                    ImGui::Text(fpsText);
                }
            }
            ImGui::End();

            return 0;
        };

        dock_win();

        for (auto& d : dbgui_list) {
            neko_defer([&d] { d.second.func(neko_dbgui_result_in_progress); });
        }
    }

private:
    std::map<neko_string, neko_dbgui_win> dbgui_list;
};

}  // namespace neko

#endif  // !NEKO_DBGUI_HPP
