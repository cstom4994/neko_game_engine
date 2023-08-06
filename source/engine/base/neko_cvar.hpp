// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef ME_CVAR_HPP
#define ME_CVAR_HPP

#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <map>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include "engine/base/neko_meta.hpp"
#include "engine/common/neko_util.h"
#include "engine/utility/module.hpp"
#include "engine/utility/neko_cpp_utils.hpp"

namespace neko {

enum cmd_type { CVAR_VAR = 0, CVAR_FUNC = 1 };

#define CVAR_TYPES() bool, int, float

struct neko_engine_cvar_t {
    bool draw_frame_graph;
    bool draw_background;
    bool draw_background_grid;
    bool draw_load_zones;
    bool draw_physics_debug;
    bool draw_b2d_shape;
    bool draw_b2d_joint;
    bool draw_b2d_aabb;
    bool draw_b2d_pair;
    bool draw_b2d_centerMass;
    bool draw_chunk_state;
    bool draw_debug_stats;
    bool draw_material_info;
    bool draw_detailed_material_info;
    bool draw_uinode_bounds;
    bool draw_temperature_map;
    bool draw_cursor;

    bool ui_tweak;
    bool ui_debug;
    bool ui_imgui_debug;
    bool ui_profiler;
    bool ui_console;
    bool ui_pack_editor;
    bool ui_code_editor;
    bool ui_inspector;
    bool ui_memory_profiler;

    bool draw_shaders;
    int water_overlay;
    bool water_showFlow;
    bool water_pixelated;
    float lightingQuality;
    bool draw_light_overlay;
    bool simpleLighting;
    bool lightingEmission;
    bool lightingDithering;

    bool tick_world;
    bool tick_box2d;
    bool tick_temperature;
    bool hd_objects;

    int hd_objects_size;

    int cell_iter;
    int brush_size;

    bool debug_entities_test;
};

void __neko_engine_cvar_init(neko_engine_cvar_t* s);

void __neko_engine_cvar_reg(neko_engine_cvar_t* s);

template <typename T>
void neko_engine_cvar_init(neko_engine_cvar_t* s, T func) {
    // static_assert(std::is_function_v<T>);
    __neko_engine_cvar_init(s);
    func();
    __neko_engine_cvar_reg(s);
}

}  // namespace neko

#endif