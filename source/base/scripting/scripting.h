#ifndef SCRIPT_H
#define SCRIPT_H

#include "engine/base.hpp"
#include "engine/event.h"
#include "engine/input.h"
#include "base/scripting/luax.h"
#include "base/scripting/lua_wrapper.hpp"
#include "base/common/logger.hpp"

struct App;

NEKO_API() void script_run_string(const char *s);
NEKO_API() void script_run_file(const char *filename);
NEKO_API() void script_error(const char *s);

NEKO_API() void ng_push_cdata(const char *t, void *p);

NEKO_API() void script_init();
NEKO_API() void script_fini();
NEKO_API() int script_pre_update_all(App *app, event_t evt);
NEKO_API() int script_update_all(App *app, event_t evt);
NEKO_API() int script_post_update_all(App *app, event_t evt);
NEKO_API() void script_draw_ui();
NEKO_API() void script_draw_all();
NEKO_API() void script_key_down(KeyCode key);
NEKO_API() void script_key_up(KeyCode key);
NEKO_API() void script_mouse_down(MouseCode mouse);
NEKO_API() void script_mouse_up(MouseCode mouse);
NEKO_API() void script_mouse_move(vec2 pos);
NEKO_API() void script_scroll(vec2 scroll);
NEKO_API() void script_save_all(App* app);
NEKO_API() void script_load_all(App* app);

void luax_run_bootstrap(lua_State *L);
void luax_run_nekogame(lua_State *L);

NEKO_API() int luax_pcall_nothrow(lua_State *L, int nargs, int nresults);
NEKO_API() void script_push_event(const char *event);

#define errcheck(...)                                      \
    do                                                     \
        if (__VA_ARGS__) {                                 \
            LOG_INFO("lua: {}\n", lua_tostring(L, -1)); \
            lua_pop(L, 1);                                 \
            if (LockGuard<Mutex> lock{gBase.error_mtx}) {  \
                gBase.error_mode.store(true);              \
            }                                              \
        }                                                  \
    while (0)

#endif

#ifndef NEKO_LUA_STRUCT_H
#define NEKO_LUA_STRUCT_H

#include "base/scripting/lua_wrapper.hpp"

NEKO_API() void open_neko_api(lua_State *L);

namespace Neko::luabind {

template <typename T>
void checktable_refl(lua_State *L, const_str tname, T &&v) {

#define FUCK_TYPES() i32, u32, bool, f32, bool, const_str, String

    if (lua_getfield(L, -1, tname) == LUA_TNIL) {
        LOG_INFO("[exception] no {} table", tname);
    }
    if (lua_istable(L, -1)) {
        auto f = [&L](std::string_view name, Neko::reflection::Any &value) {
            static_assert(std::is_lvalue_reference_v<decltype(value)>);
            if (lua_getfield(L, -1, std::string(name).c_str()) != LUA_TNIL) {
                auto ff = [&]<typename S>(const_str name, Neko::reflection::Any &var, S &t) {
                    if (value.GetType() == Neko::reflection::type_of<S>()) {
                        S s{};
                        detail::LuaStack::Get(L, -1, s);
                        value.cast<S>() = s;
                    }
                };
                std::apply([&](auto &&...args) { (ff(std::string(name).c_str(), value, args), ...); }, std::tuple<FUCK_TYPES()>());
            }
            lua_pop(L, 1);
        };
        v.foreach (f);
    } else {
        LOG_INFO("[exception] no {} table", tname);
    }
    lua_pop(L, 1);
}

}  // namespace Neko::luabind

namespace Neko::luabind::detail {

template <typename T>
auto Get(lua_State *L, int idx, T &x)
    requires std::is_same_v<T, Neko::String>
{
    x = luax_check_string(L, idx);
    return;
}

}  // namespace Neko::luabind::detail

#endif