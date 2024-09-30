#ifndef SCRIPT_H
#define SCRIPT_H

#include "engine/base.h"
#include "engine/event.h"
#include "engine/input.h"
#include "engine/scripting/luax.h"

struct App;

NEKO_API() void script_run_string(const char *s);
NEKO_API() void script_run_file(const char *filename);
NEKO_API() void script_error(const char *s);

NEKO_API() void script_init();
NEKO_API() void script_fini();
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
NEKO_API() void script_save_all(Store *s);
NEKO_API() void script_load_all(Store *s);

void luax_run_bootstrap(lua_State *L);
void luax_run_nekogame(lua_State *L);

NEKO_API() int luax_pcall_nothrow(lua_State *L, int nargs, int nresults);
NEKO_API() void script_push_event(const char *event);

#define errcheck(...)                                         \
    do                                                        \
        if (__VA_ARGS__) {                                    \
            console_printf("lua: %s\n", lua_tostring(L, -1)); \
            lua_pop(L, 1);                                    \
            if (LockGuard<Mutex> lock{g_app->error_mtx}) {    \
                g_app->error_mode.store(true);                \
            }                                                 \
        }                                                     \
    while (0)

#endif

#ifndef NEKO_LUA_STRUCT_H
#define NEKO_LUA_STRUCT_H

#include "engine/scripting/lua_wrapper.hpp"

NEKO_API() void createStructTables(lua_State *L);

NEKO_API() void open_neko_api(lua_State *L);

struct App;

typedef u32 (*native_script_get_instance_size_f)();
typedef void (*native_script_init_f)(App *scene, NativeEntity entity, void *);
typedef void (*native_script_update_f)(App *scene, NativeEntity entity, void *, double);
typedef void (*native_script_physics_update_f)(App *scene, NativeEntity entity, void *, double);
typedef void (*native_script_free_f)(App *scene, NativeEntity entity, void *);

typedef struct native_script_t {
    void *instance;

    char *get_instance_size_name;
    char *on_init_name;
    char *on_update_name;
    char *on_physics_update_name;
    char *on_free_name;

    native_script_init_f on_init;
    native_script_update_f on_update;
    native_script_physics_update_f on_physics_update;
    native_script_free_f on_free;

    NativeEntity entity;
} native_script_t;

typedef struct native_script_context_t {
    native_script_t *scripts;
    u32 script_count;
    u32 script_capacity;

    App *scene;

    void *handle;
} native_script_context_t;

namespace neko::luabind {

template <typename T>
void checktable_refl(lua_State *L, const_str tname, T &&v) {

#define FUCK_TYPES() i32, u32, bool, f32, bool, const_str, String

    if (lua_getfield(L, -1, tname) == LUA_TNIL) {
        console_log("[exception] no %s table", tname);
    }
    if (lua_istable(L, -1)) {
        auto f = [&L](std::string_view name, neko::reflection::Any &value) {
            static_assert(std::is_lvalue_reference_v<decltype(value)>);
            if (lua_getfield(L, -1, std::string(name).c_str()) != LUA_TNIL) {
                auto ff = [&]<typename S>(const_str name, neko::reflection::Any &var, S &t) {
                    if (value.GetType() == neko::reflection::type_of<S>()) {
                        S s = detail::LuaStack<std::remove_reference_t<S>>::Get(L, -1);
                        value.cast<S>() = s;
                    }
                };
                std::apply([&](auto &&...args) { (ff(std::string(name).c_str(), value, args), ...); }, std::tuple<FUCK_TYPES()>());
            }
            lua_pop(L, 1);
        };
        v.foreach (f);
    } else {
        console_log("[exception] no %s table", tname);
    }
    lua_pop(L, 1);
}

}  // namespace neko::luabind

#endif