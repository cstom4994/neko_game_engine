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

            .member_("draw_shaders", &neko_engine_cvar_t::draw_shaders, {.metadata{{"info", "是否启用光影"s}}})
            .member_("tick_world", &neko_engine_cvar_t::tick_world, {.metadata{{"info", "是否启用世界更新"s}}})

            .member_("cell_iter", &neko_engine_cvar_t::cell_iter, {.metadata{{"info", "Cell迭代次数"s}}})
            .member_("brush_size", &neko_engine_cvar_t::brush_size, {.metadata{{"info", "编辑器笔刷大小"s}}})
            .member_("debug_entities_test", &neko_engine_cvar_t::debug_entities_test, {.metadata{{"info", "是否启用实体调试"s}}});

    meta::class_<neko_engine_cvar_t>({{"info", "全局变量"s}}).member_("world_gravity", &neko_engine_cvar_t::world_gravity, {.metadata{{"info", "世界重力"s}}});
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