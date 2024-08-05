#pragma once

#include "engine/luax.h"
#include "engine/neko.hpp"

struct lua_State;
void open_neko_api(lua_State *L);
void open_luasocket(lua_State *L);

namespace neko::lua {
void package_preload(lua_State *L);
void package_preload_embed(lua_State *L);
}  // namespace neko::lua

enum W_LUA_UPVALUES { NEKO_W_COMPONENTS_NAME = 1, NEKO_W_UPVAL_N };

struct W_LUA_REGISTRY_CONST {
    static constexpr i32 W_CORE_IDX = 1;                             // neko_instance_t* index
    static constexpr const_str W_CORE = "__NEKO_W_CORE";             // neko_instance_t* reg
    static constexpr const_str ENG_UDATA_NAME = "__NEKO_ENGINE_UD";  // neko_instance_t* udata
    static constexpr const_str CVAR_MAP = "cvar_map";

    static constexpr i32 CVAR_MAP_MAX = 64;
};

template <typename T>
struct neko_w_lua_variant {
    enum { NATIVE, LUA } stype;
    i32 type = LUA_TNONE;
    String cname;
    union {
        bool boolean;
        double number;
        const_str str;
    } data;

    neko_w_lua_variant(const_str _cname) : cname(_cname) {}

    neko_w_lua_variant(const_str _cname, T _v) : neko_w_lua_variant(_cname) {

        using TT = std::decay_t<T>;

        if constexpr (std::same_as<TT, bool>) {
            type = LUA_TBOOLEAN;
            data.boolean = _v;
        } else if constexpr (std::is_integral_v<TT>) {
            type = LUA_TNUMBER;
            data.number = _v;
        } else if constexpr (std::is_floating_point_v<TT>) {
            type = LUA_TNUMBER;
            data.number = _v;
        } else if constexpr (std::is_same_v<TT, String>) {
            type = LUA_TSTRING;
            data.str = _v.data;
        } else if constexpr (neko::is_pointer_to_const_char<TT>) {
            type = LUA_TSTRING;
            data.str = _v;
        } else {
            static_assert(std::is_void_v<TT>, "unsupported type for neko_w_lua_variant");
        }

        this->make();
    }

    template <typename C>
    neko_w_lua_variant &operator=(const C &value) {
        using TT = std::decay_t<C>;
        if constexpr (std::is_same_v<TT, i32> || std::is_same_v<TT, u32> || std::is_same_v<TT, f32> || std::is_same_v<TT, f64>) {
            type = LUA_TNUMBER;
            data.number = static_cast<double>(value);
        } else if constexpr (std::is_same_v<TT, const_str>) {
            type = LUA_TSTRING;
            data.str = value;
        } else if constexpr (std::is_same_v<TT, bool>) {
            type = LUA_TBOOLEAN;
            data.boolean = value;
            // } else if constexpr (std::is_pointer_v<TT>) {
            //     type = LUA_TUSERDATA;
            //     data.udata.ptr = reinterpret_cast<void *>(const_cast<TT>(value));
        } else {
            static_assert("unsupported type for neko_w_lua_variant::=");
        }
        this->make();
        return *this;
    }

    void push() {
        lua_State *L = ENGINE_LUA();
        switch (type) {
            case LUA_TNONE:
                neko_assert(type == LUA_TNONE);
                break;
            case LUA_TBOOLEAN:
                lua_pushboolean(L, data.boolean);
                break;
            case LUA_TNUMBER:
                lua_pushnumber(L, data.number);
                break;
            case LUA_TSTRING: {
                // neko::string s = luax_check_string(L, arg);
                // data.string = to_cstr(s);
                lua_pushstring(L, data.str);
                break;
            }
        }
    }

    void sync() {
        neko_assert(cname.data != NULL);

        lua_State *L = ENGINE_LUA();

        int n = lua_gettop(L);

        lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);  // # 1
        lua_getiuservalue(L, -1, W_LUA_UPVALUES::NEKO_W_COMPONENTS_NAME);  // # 2
        lua_getfield(L, -1, W_LUA_REGISTRY_CONST::CVAR_MAP);               // # 3
        lua_pushinteger(L, neko_hash_str(cname.data));                     // 使用 32 位哈希以适应 Lua 数字范围
        lua_gettable(L, -2);                                               // # 4

        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "type");  // # 5
            this->type = lua_tointeger(L, -1);
            lua_pop(L, 1);  // # pop 5

            lua_getfield(L, -1, "data");  // # 5
            switch (type) {
                case LUA_TNONE:
                    neko_assert(type == LUA_TNONE);
                    break;
                case LUA_TBOOLEAN:
                    data.boolean = lua_toboolean(L, -1);
                    break;
                case LUA_TNUMBER:
                    data.number = lua_tonumber(L, -1);
                    break;
                case LUA_TSTRING: {
                    // neko::string s = luax_check_string(L, arg);
                    // data.string = to_cstr(s);
                    data.str = lua_tostring(L, -1);
                    break;
                }
            }
            lua_pop(L, 1);  // # pop 5
        } else {
            console_log("cvar sync with no value : %s", cname.data);
        }

        lua_pop(L, 4);

        if (type == LUA_TNONE) {
            console_log("cvar sync with no type : %s", cname.data);
        }

        neko_assert(lua_gettop(L) == n);
    }

    template <typename C>
    auto get() -> C {
        using TT = std::decay_t<C>;
        if (type == LUA_TNONE) {
            console_log("trying to neko_w_lua_variant::get(%s) with LUA_TNONE", cname.data);
        }
        if constexpr (std::is_same_v<TT, i32> || std::is_same_v<TT, u32> || std::is_same_v<TT, f32> || std::is_same_v<TT, f64>) {
            neko_assert(type == LUA_TNUMBER);
            return data.number;
        } else if constexpr (std::is_same_v<TT, const_str>) {
            neko_assert(type == LUA_TSTRING);
            return data.str;
        } else if constexpr (std::is_same_v<TT, bool>) {
            neko_assert(type == LUA_TBOOLEAN);
            return data.boolean;
        } else {
            static_assert("unsupported type for neko_w_lua_variant::get");
        }
    }

    void make() {
        neko_assert(type != LUA_TNONE);

        lua_State *L = ENGINE_LUA();

        int n = lua_gettop(L);

        lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);  // # 1

        lua_getiuservalue(L, -1, NEKO_W_COMPONENTS_NAME);  // # 2
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, W_LUA_REGISTRY_CONST::CVAR_MAP);  // # 3
            if (lua_istable(L, -1)) {
                lua_pushinteger(L, neko_hash_str(cname.data));  // 使用 32 位哈希以适应 Lua 数字范围

                lua_gettable(L, -2);       // # 4
                if (lua_istable(L, -1)) {  // 如果表存在 修改字段
                    lua_pushinteger(L, type);
                    lua_setfield(L, -2, "type");
                    lua_pushstring(L, cname.data);
                    lua_setfield(L, -2, "name");
                    push();
                    lua_setfield(L, -2, "data");
                } else {            // 如果表不存在 创建一个新表并设置字段
                    lua_pop(L, 1);  // 弹出非表值

                    lua_createtable(L, 0, 2);
                    lua_pushinteger(L, type);
                    lua_setfield(L, -2, "type");
                    lua_pushstring(L, cname.data);
                    lua_setfield(L, -2, "name");
                    push();
                    lua_setfield(L, -2, "data");

                    lua_pushinteger(L, neko_hash_str(cname.data));  // 重新推入键
                    lua_pushvalue(L, -2);                           // 将新表复制到栈顶
                    lua_settable(L, -4);                            // 将新表设置到 CVAR_MAP 中
                }

                lua_pop(L, 1);  // # pop 4

                lua_pop(L, 1);  // # pop 3
            } else {
                console_log("%s", "failed to get W_LUA_REGISTRY_NAME::CVAR_MAP");
                lua_pop(L, 1);  // # pop 3
            }
            lua_pop(L, 1);  // # pop 2
        } else {
            console_log("%s", "failed to get upvalue NEKO_W_COMPONENTS_NAME");
            lua_pop(L, 1);  // # pop 2
        }

        lua_pop(L, 1);  // # pop 1

        neko_assert(lua_gettop(L) == n);
    }
};

static_assert(std::is_trivially_copyable_v<neko_w_lua_variant<f64>>);

#define CVAR(name, V) neko_w_lua_variant<decltype(V)> name(#name, V)
#define CVAR_REF(name, T)              \
    neko_w_lua_variant<T> name(#name); \
    name.sync()

// extern impl
extern int open_tools_spritepack(lua_State *L);
extern int open_filesys(lua_State *L);