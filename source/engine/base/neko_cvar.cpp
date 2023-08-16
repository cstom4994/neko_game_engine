// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#include "neko_cvar.hpp"

#include "engine/base/neko_component.h"
#include "engine/base/neko_engine.h"
#include "engine/meta/neko_refl.hpp"
#include "engine/utility/logger.hpp"

namespace neko {

using namespace neko::cpp;

using namespace std::literals;

neko_engine_cvar_t g_cvar = {0};

void __neko_engine_cvar_init(neko_engine_cvar_t *s) {

    neko_refl_instance.RegisterType<neko::meta::range>();
    neko_refl_instance.AddField<&neko::meta::range::min_value>("min_value");
    neko_refl_instance.AddField<&neko::meta::range::max_value>("max_value");
    neko_refl_instance.AddConstructor<neko::meta::range, float, float>();

    neko_refl_instance.RegisterType<neko::meta::info>();
    neko_refl_instance.AddField<&neko::meta::info::info>("info");
    neko_refl_instance.AddConstructor<neko::meta::info, const_str>();

    neko_refl_instance.RegisterType<neko_engine_cvar_t>();
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_tweak>("ui_tweak", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否打开TWEAK界面"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_debug>("ui_debug", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{""})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_imgui_debug>("ui_imgui_debug", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否显示IMGUI示例窗口"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_profiler>("ui_profiler", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否显示帧检查器"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_console>("ui_console", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否显示控制台"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_pack_editor>("ui_pack_editor", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否显示包编辑器"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_inspector>("ui_inspector", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否显示检查器"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::ui_memory_profiler>("ui_memory_profiler", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否显示内存检查器"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::debug_entities_test>("debug_entities_test", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否启用实体调试"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::draw_shaders>("draw_shaders", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否启用光影"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::tick_world>("tick_world", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"是否启用世界更新"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::cell_iter>("cell_iter", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"Cell迭代次数"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::brush_size>("brush_size", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"编辑器笔刷大小"})});
    neko_refl_instance.AddField<&neko_engine_cvar_t::world_gravity>("world_gravity", {neko_refl_instance.MakeShared(Type_of<neko::meta::info>, TempArgsView{"世界重力"})});
}

void __neko_engine_cvar_reg(neko_engine_cvar_t *s) {}

void __neko_config_init() {
    neko_cv() = (neko_config_t *)neko_malloc(sizeof(neko_config_t));
    neko_cv()->cvars = neko_dyn_array_new(neko_cvar_t);

    neko_cvar_new("test_cvar", __NEKO_CONFIG_TYPE_INT, 1001);

    neko_cvar_new_str("test_cvar_str", __NEKO_CONFIG_TYPE_STRING, "啊!能不能有中文?");

    // TODO: 23/8/10 控制台以及相关cvar注册
}

void __neko_config_free() {

    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        if (neko_cv()->cvars->type == __NEKO_CONFIG_TYPE_STRING) {
            neko_free(neko_cv()->cvars[i].value.s);
            neko_cv()->cvars[i].value.s = NULL;
        }
    }
    neko_dyn_array_free(neko_cv()->cvars);

    neko_free(neko_cv());
    neko_cv() = NULL;
}

neko_cvar_t *__neko_config_get(const_str name) {
    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        if (strcmp(neko_cv()->cvars[i].name, name) == 0) {
            return &neko_cv()->cvars[i];
        }
    }
    return NULL;
}

void neko_config_print() {
    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        neko_cvar_print(&neko_cv()->cvars[i]);
    }
}

}  // namespace neko