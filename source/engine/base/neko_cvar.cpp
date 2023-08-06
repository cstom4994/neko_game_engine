// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#include "neko_cvar.hpp"

#include "engine/utility/logger.hpp"

namespace neko {

using namespace std::literals;

void __neko_engine_cvar_init(neko_engine_cvar_t *s) {

    meta::class_<neko_engine_cvar_t>({{"info", "全局变量"s}})
            .member_("ui_tweak", &neko_engine_cvar_t::ui_tweak, {.metadata{{"info", "是否打开TWEAK界面"s}}})
            .member_("ui_debug", &neko_engine_cvar_t::ui_debug, {.metadata{{"info", ""s}}})
            .member_("ui_imgui_debug", &neko_engine_cvar_t::ui_imgui_debug, {.metadata{{"info", "是否显示IMGUI示例窗口"s}}})
            .member_("ui_profiler", &neko_engine_cvar_t::ui_profiler, {.metadata{{"info", "是否显示帧检查器"s}}})
            .member_("ui_console", &neko_engine_cvar_t::ui_console, {.metadata{{"info", "是否显示控制台"s}}})
            .member_("ui_pack_editor", &neko_engine_cvar_t::ui_pack_editor, {.metadata{{"info", "是否显示包编辑器"s}}})
            .member_("ui_code_editor", &neko_engine_cvar_t::ui_code_editor, {.metadata{{"info", "是否显示脚本编辑器"s}}})
            .member_("ui_inspector", &neko_engine_cvar_t::ui_inspector, {.metadata{{"info", "是否显示检查器"s}}})
            .member_("ui_memory_profiler", &neko_engine_cvar_t::ui_memory_profiler, {.metadata{{"info", "是否显示内存检查器"s}}})

            .member_("draw_frame_graph", &neko_engine_cvar_t::draw_frame_graph, {.metadata{{"info", "是否显示帧率图"s}}})
            .member_("draw_background", &neko_engine_cvar_t::draw_background, {.metadata{{"info", "是否绘制背景"s}}})
            .member_("draw_background_grid", &neko_engine_cvar_t::draw_background_grid, {.metadata{{"info", "是否绘制背景网格"s}}})
            .member_("draw_load_zones", &neko_engine_cvar_t::draw_load_zones, {.metadata{{"info", "是否显示加载区域"s}}})
            .member_("draw_physics_debug", &neko_engine_cvar_t::draw_physics_debug, {.metadata{{"info", "是否开启物理调试"s}}})
            .member_("draw_b2d_shape", &neko_engine_cvar_t::draw_b2d_shape, {.metadata{{"info", ""s}}})
            .member_("draw_b2d_joint", &neko_engine_cvar_t::draw_b2d_joint, {.metadata{{"info", ""s}}})
            .member_("draw_b2d_aabb", &neko_engine_cvar_t::draw_b2d_aabb, {.metadata{{"info", ""s}}})
            .member_("draw_b2d_pair", &neko_engine_cvar_t::draw_b2d_pair, {.metadata{{"info", ""s}}})
            .member_("draw_b2d_centerMass", &neko_engine_cvar_t::draw_b2d_centerMass, {.metadata{{"info", ""s}}})
            .member_("draw_chunk_state", &neko_engine_cvar_t::draw_chunk_state, {.metadata{{"info", "是否显示区块状态"s}}})
            .member_("draw_debug_stats", &neko_engine_cvar_t::draw_debug_stats, {.metadata{{"info", "是否显示调试信息"s}}})
            .member_("draw_material_info", &neko_engine_cvar_t::draw_material_info, {.metadata{{"info", "是否显示材质信息"s}}})
            .member_("draw_detailed_material_info", &neko_engine_cvar_t::draw_detailed_material_info, {.metadata{{"info", "是否显示材质详细信息"s}}})
            .member_("draw_uinode_bounds", &neko_engine_cvar_t::draw_uinode_bounds, {.metadata{{"info", "是否绘制UINODE"s}}})
            .member_("draw_temperature_map", &neko_engine_cvar_t::draw_temperature_map, {.metadata{{"info", "是否绘制温度图"s}}})
            .member_("draw_cursor", &neko_engine_cvar_t::draw_cursor, {.metadata{{"info", "是否显示鼠标指针"s}}})
            .member_("draw_shaders", &neko_engine_cvar_t::draw_shaders, {.metadata{{"info", "是否启用光影"s}}})
            .member_("water_overlay", &neko_engine_cvar_t::water_overlay, {.metadata{{"info", "水渲染覆盖"s}}})
            .member_("water_showFlow", &neko_engine_cvar_t::water_showFlow, {.metadata{{"info", "是否绘制水渲染流程"s}}})
            .member_("water_pixelated", &neko_engine_cvar_t::water_pixelated, {.metadata{{"info", "启用水渲染像素化"s}}})
            .member_("lightingQuality", &neko_engine_cvar_t::lightingQuality, {.metadata{{"info", "光照质量"s}, {"imgui", "float_range"s}, {"max", 1.0f}, {"min", 0.0f}}})
            .member_("draw_light_overlay", &neko_engine_cvar_t::draw_light_overlay, {.metadata{{"info", "是否启用光照覆盖"s}}})
            .member_("simpleLighting", &neko_engine_cvar_t::simpleLighting, {.metadata{{"info", "是否启用光照简单采样"s}}})
            .member_("lightingEmission", &neko_engine_cvar_t::lightingEmission, {.metadata{{"info", "是否启用光照放射"s}}})
            .member_("lightingDithering", &neko_engine_cvar_t::lightingDithering, {.metadata{{"info", "是否启用光照抖动"s}}})
            .member_("tick_world", &neko_engine_cvar_t::tick_world, {.metadata{{"info", "是否启用世界更新"s}}})
            .member_("tick_box2d", &neko_engine_cvar_t::tick_box2d, {.metadata{{"info", "是否启用刚体物理更新"s}}})
            .member_("tick_temperature", &neko_engine_cvar_t::tick_temperature, {.metadata{{"info", "是否启用世界温度更新"s}}})
            .member_("hd_objects", &neko_engine_cvar_t::hd_objects, {.metadata{{"info", ""s}}})
            .member_("hd_objects_size", &neko_engine_cvar_t::hd_objects_size, {.metadata{{"info", ""s}}})

            .member_("cell_iter", &neko_engine_cvar_t::cell_iter, {.metadata{{"info", "Cell迭代次数"s}}})
            .member_("brush_size", &neko_engine_cvar_t::brush_size, {.metadata{{"info", "编辑器笔刷大小"s}}})
            .member_("debug_entities_test", &neko_engine_cvar_t::debug_entities_test, {.metadata{{"info", "是否启用实体调试"s}}});
}

void __neko_engine_cvar_reg(neko_engine_cvar_t *s) {

    try {

        const meta::scope cvar_scope = meta::local_scope_("cvar").typedef_<neko_engine_cvar_t>("__neko_engine_cvar");

        for (const auto &[type_name, type] : cvar_scope.get_typedefs()) {
            if (!type.is_class()) {
                continue;
            }

            const meta::class_type &class_type = type.as_class();
            const meta::metadata_map &class_metadata = class_type.get_metadata();

            for (const meta::member &member : class_type.get_members()) {
                const meta::metadata_map &member_metadata = member.get_metadata();

                auto f = [&]<typename T>(T arg) {
                    if (member.get_type().get_value_type() != meta::resolve_type<T>()) return;
                    auto f = member.get(s);
                    if (T *s = f.try_as<T>()) {
                        T t = *s;
                        // ImGui::Auto(t, member.get_name().c_str());
                        // if (t != *s) member.try_set(global.game->Iso.globaldef, t);
                        // ImGui::Text("    [%s]", member_metadata.at("info").as<std::string>().c_str());

                        // convar.Value("game_scale", the<engine>().eng()->render_scale);

                        // member_metadata.at("info").as<std::string>().c_str()
                        // the<convar>().var(member.get_name(), t);
                    } else {
                    }
                };

                using cvar_types_t = std::tuple<CVAR_TYPES()>;

                cvar_types_t ctt;
                std::apply([&](auto &&...args) { (f(args), ...); }, ctt);
            }
        }
    } catch (const std::exception ex) {
        neko_error(std::format("exception: {0}", ex.what()));
    }
}

}  // namespace neko