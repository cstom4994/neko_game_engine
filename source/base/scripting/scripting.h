#ifndef SCRIPT_H
#define SCRIPT_H

#include "engine/base.hpp"
#include "engine/event.h"
#include "engine/input.h"
#include "base/scripting/luax.h"
#include "base/scripting/lua_wrapper.hpp"
#include "base/common/logger.hpp"
#include "base/common/vfs.hpp"

struct App;

NEKO_API() void script_run_string(const char *s);
NEKO_API() void script_run_file(const char *filename);
NEKO_API() void script_error(const char *s);

NEKO_API() void ng_push_cdata(const char *t, void *p);

NEKO_API() void script_init();
NEKO_API() void script_fini();

NEKO_API() void script_save_all(App *app);
NEKO_API() void script_load_all(App *app);

void luax_run_bootstrap(lua_State *L);
void luax_run_nekogame(lua_State *L);

NEKO_API() int luax_pcall_nothrow(lua_State *L, int nargs, int nresults);

namespace Neko {

using LuaRef = luabind::LuaRef;
using LuaRefID = i32;

LuaRefID luax_require_script_buffer(lua_State *L, String &contents, String name = "<luax_require_script_buffer>");
LuaRefID luax_require_script(lua_State *L, String filepath);

class LuaBpFileSystem : public FileSystem {
public:
    String luabpPath;
    HashMap<String> luabp;

    ~LuaBpFileSystem();

    int vfs_load_luabp(lua_State *L);
    void trash();
    bool mount(lua_State *L, LuaRef &tb);
    bool file_exists(String filepath);
    bool read_entire_file(String *out, String filepath);
    bool list_all_files(Array<String> *files);
    u64 file_modtime(String filepath);
};

}  // namespace Neko

template <typename... Args>
void errcheck(lua_State *L, Args... args) {
    if ((... || args)) {
        LOG_INFO("lua: {}\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        if (LockGuard<Mutex> lock{gBase.error_mtx}) {
            gBase.error_mode.store(true);
        }
    }
}

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

template <typename... Ts>
std::tuple<Ts...> parse_args(lua_State *L) {
    std::tuple<Ts...> ret;
    util::static_for<0, sizeof...(Ts)>([&ret, &s]([[maybe_unused]] auto i) constexpr {
        using T = std::decay_t<decltype(std::get<i.value>(std::declval<std::tuple<Ts...>>()))>;
        auto &v = std::get<i.value>(ret);
        int idx = i.value + 1;
        if constexpr (std::is_same_v<T, bool>) {
            v = lua_toboolean(L, idx);
        } else if constexpr (std::is_same_v<T, float>) {
            v = lua_tonumber(L, idx);
        } else if constexpr (std::is_same_v<T, double>) {
            v = lua_tonumber(L, idx);
        } else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, int>) {
            v = lua_tointeger(L, idx);
        } else if constexpr (std::is_same_v<T, std::uint64_t> || std::is_same_v<T, unsigned int>) {
            v = lua_tointeger(L, idx);
        } else if constexpr (std::is_same_v<T, std::string>) {
            v = lua_tostring(L, idx);
        } else if constexpr (std::is_same_v<T, tz::lua::nil>) {

        } else {
            static_assert(std::is_void_v<T>, "Unrecognised lua argument type");
        }
    });
    return ret;
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