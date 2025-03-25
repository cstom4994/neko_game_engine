
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/entitybase.hpp"

bool gui_has_focus() { return gui_captured_event(); }

bool gui_captured_event() { return ImGui::IsAnyItemHovered() || ImGui::IsAnyItemFocused(); }

int gui_pre_update_all(App *app, Event evt) {
    imgui_draw_pre();
    return 0;
}