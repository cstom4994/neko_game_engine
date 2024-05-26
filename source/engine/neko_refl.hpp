#ifndef NEKO_ENGINE_NEKO_REFL_HPP
#define NEKO_ENGINE_NEKO_REFL_HPP

#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "game_cvar.h"
#include "sandbox/hpp/neko_static_refl.hpp"

// decl static reflection
template <>
struct neko::static_refl::neko_type_info<neko_platform_running_desc_t> : neko_type_info_base<neko_platform_running_desc_t> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            rf_field{TSTR("title"), &rf_type::title},                  // 窗口标题
            rf_field{TSTR("width"), &rf_type::width},                  //
            rf_field{TSTR("height"), &rf_type::height},                //
            rf_field{TSTR("flags"), &rf_type::flags},                  //
            rf_field{TSTR("num_samples"), &rf_type::num_samples},      //
            rf_field{TSTR("monitor_index"), &rf_type::monitor_index},  //
            rf_field{TSTR("vsync"), &rf_type::vsync},                  // 启用 vsync
            rf_field{TSTR("frame_rate"), &rf_type::frame_rate},        // 限制帧率
    };
};

template <>
struct neko::static_refl::neko_type_info<neko_client_cvar_t> : neko_type_info_base<neko_client_cvar_t> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            rf_field{TSTR("show_editor"), &rf_type::show_editor},                    //
            rf_field{TSTR("show_demo_window"), &rf_type::show_demo_window},          //
            rf_field{TSTR("show_pack_editor"), &rf_type::show_pack_editor},          //
            rf_field{TSTR("show_profiler_window"), &rf_type::show_profiler_window},  //
            rf_field{TSTR("show_gui"), &rf_type::show_gui},                          //
            rf_field{TSTR("shader_inspect"), &rf_type::shader_inspect},              //
            rf_field{TSTR("hello_ai_shit"), &rf_type::hello_ai_shit},                //
            rf_field{TSTR("is_hotfix"), &rf_type::is_hotfix},                        //
            rf_field{TSTR("vsync"), &rf_type::vsync},                                //
            rf_field{TSTR("enable_nekolua"), &rf_type::enable_nekolua},              //
    };
};

#endif  // NEKO_ENGINE_NEKO_REFL_HPP
