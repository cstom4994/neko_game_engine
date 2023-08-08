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
    bool ui_tweak;
    bool ui_debug;
    bool ui_imgui_debug;
    bool ui_profiler;
    bool ui_console;
    bool ui_pack_editor;
    bool ui_code_editor;
    bool ui_inspector;
    bool ui_memory_profiler;
    bool debug_entities_test;

    bool draw_shaders = true;
    bool tick_world = true;
    int cell_iter;
    f32 brush_size = 4.f;
    f32 world_gravity = 10.f;
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