
#ifndef NEKO_DBGUI_HPP
#define NEKO_DBGUI_HPP

#include <map>
#include <ranges>

#include "engine/common/neko_containers.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_util.h"
#include "engine/gui/neko_imgui_utils.hpp"
#include "engine/utility/enum.hpp"
#include "engine/utility/module.hpp"

namespace neko {

typedef enum { neko_dbgui_result_success, neko_dbgui_result_in_progress } neko_dbgui_result;

using neko_dbgui_func = neko_function<neko_dbgui_result(neko_dbgui_result)>;

ENUM_HPP_CLASS_DECL(neko_dbgui_flags, u32, (window = 1 << 0)(no_visible = 1 << 1));
ENUM_HPP_REGISTER_TRAITS(neko_dbgui_flags);

struct neko_dbgui_win {
    neko_string name;
    neko_dbgui_func func;
    cpp::bitflags::bitflags<neko_dbgui_flags> flags;
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
    dbgui& create(const neko_string& name, const neko_dbgui_func& f, neko_dbgui_flags flags = neko_dbgui_flags::window) {
        // 创建 dbgui 实例
        dbgui_list.insert(std::make_pair(name, neko_dbgui_win{name, f, flags}));
        return *this;
    }

    dbgui& update(const neko_string& name, const neko_dbgui_func& f) {
        if (dbgui_list.count(name) != 1) neko_assert(0);
        auto origin = dbgui_list[name];
        dbgui_list[name] = neko_dbgui_win{origin.name, func_combine(f, origin.func), origin.flags};
        return *this;
    }

    cpp::bitflags::bitflags<neko_dbgui_flags>& flags(const neko_string& name) {
        if (dbgui_list.count(name) != 1) neko_assert(0);
        return dbgui_list[name].flags;
    }

    void draw() const {

        // if (!g_cvar.ui_tweak) return;

        auto dock_win = []() {
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            if (viewport) {
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
            }

            const ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground;

            const ImGuiDockNodeFlags dock_node_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

            if (ImGui::Begin("neko_editor_dockspace_window", nullptr, window_flags)) {
                ImGui::DockSpace(ImGui::GetID("neko_editor_dockspace"), ImVec2(0.f, 0.f), dock_node_flags);

                if (ImGui::BeginMenuBar()) {
                    neko_defer([] { ImGui::EndMenuBar(); });

                    ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko");

                    if (ImGui::BeginMenu("File")) {
                        ImGui::EndMenu();
                    }

                    char fpsText[50];
                    snprintf(fpsText, sizeof(fpsText), "%.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    // ME_draw_text(fpsText, {255, 255, 255, 255}, the<engine>().eng()->windowWidth - ImGui::CalcTextSize(fpsText).x, 0);

                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(fpsText).x - ImGui::GetScrollX());

                    ImGui::Text(fpsText);
                }
            }
            ImGui::End();

            ImGui::PopStyleVar(3);

            return 0;
        };

        if ((dbgui_list.at("cvar").flags & neko_dbgui_flags::no_visible) != neko_dbgui_flags::no_visible) dock_win();

        for (auto& d : dbgui_list) {
            cpp::bitflags::bitflags<neko_dbgui_flags> flags(d.second.flags);
            if ((flags & neko_dbgui_flags::no_visible) == neko_dbgui_flags::no_visible) return;
            if (ImGui::Begin(d.first.c_str())) {
                neko_defer([&d] { d.second.func(neko_dbgui_result_in_progress); });
            }
            ImGui::End();
        }
    }

private:
    std::map<neko_string, neko_dbgui_win> dbgui_list;
};

}  // namespace neko

#endif  // !NEKO_DBGUI_HPP
