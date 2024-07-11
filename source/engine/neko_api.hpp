#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/neko.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_engine.h"
#include "engine/neko_lua.h"
#include "engine/neko_math.h"

typedef struct {
    const char *name;
    lua_CFunction func;
} neko_luaL_reg;

typedef struct neko_tolua_boot_opt {
    std::string f;
    std::string output;
    std::string D;
    std::string E;
} neko_tolua_boot_opt;

NEKO_API_DECL void neko_tolua_boot(neko_tolua_boot_opt opt);

void neko_register(lua_State *L);

lua_Number luax_number_field(lua_State *L, s32 arg, const char *key);
lua_Number luax_opt_number_field(lua_State *L, s32 arg, const char *key, lua_Number fallback);

bool luax_boolean_field(lua_State *L, s32 arg, const char *key, bool fallback = false);

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l);

int luax_msgh(lua_State *L);

enum {
    LUAX_UD_TNAME = 1,
    LUAX_UD_PTR_SIZE = 2,
};

template <typename T>
void luax_new_userdata(lua_State *L, T data, const char *tname) {
    void *new_udata = lua_newuserdatauv(L, sizeof(T), 2);

    lua_pushstring(L, tname);
    lua_setiuservalue(L, -2, LUAX_UD_TNAME);

    lua_pushnumber(L, sizeof(T));
    lua_setiuservalue(L, -2, LUAX_UD_PTR_SIZE);

    memcpy(new_udata, &data, sizeof(T));
    luaL_setmetatable(L, tname);
}

#define luax_ptr_userdata luax_new_userdata

namespace neko::lua {
void package_preload(lua_State *L);
}

enum W_LUA_UPVALUES { NEKO_W_COMPONENTS_NAME = 1, NEKO_W_UPVAL_N };

struct W_LUA_REGISTRY_NAME {
    static constexpr const_str W_CORE = "__NEKO_W_CORE";             // neko_instance_t* reg
    static constexpr const_str ENG_UDATA_NAME = "__NEKO_ENGINE_UD";  // neko_instance_t* udata
    static constexpr const_str CVAR_MAP = "cvar_map";
};

struct neko_w_lua_variant {
    enum { NATIVE, LUA } stype;
    s32 type = LUA_TNONE;
    neko::string cname;
    union {
        bool boolean;
        double number;
        const_str str;
    } data;

    neko_w_lua_variant(const_str _cname) : cname(_cname) {}

    explicit neko_w_lua_variant(const_str _cname, bool _b) : neko_w_lua_variant(_cname) {
        type = LUA_TBOOLEAN;
        data.boolean = _b;
        this->make();
    }
    explicit neko_w_lua_variant(const_str _cname, double _n) : neko_w_lua_variant(_cname) {
        type = LUA_TNUMBER;
        data.number = _n;
        this->make();
    }
    explicit neko_w_lua_variant(const_str _cname, const_str _s) : neko_w_lua_variant(_cname) {
        type = LUA_TSTRING;
        data.str = _s;
        this->make();
    }

    // template <typename T>
    // operator T() {
    //     this->make();
    //     if constexpr (std::same_as<T, s32> || std::same_as<T, u32>) {
    //         NEKO_ASSERT(data.type == LUA_TNUMBER);
    //         return static_cast<T>(data.number);
    //     } else if constexpr (std::same_as<T, f32> || std::same_as<T, f64>) {
    //         NEKO_ASSERT(data.type == LUA_TNUMBER);
    //         return static_cast<T>(data.number);
    //     } else if constexpr (std::same_as<T, const_str>) {
    //         NEKO_ASSERT(data.type == LUA_TSTRING);
    //         return data.string.data;
    //     } else if constexpr (std::same_as<T, bool>) {
    //         NEKO_ASSERT(data.type == LUA_TBOOLEAN);
    //         return data.boolean;
    //     } else if constexpr (std::is_pointer_v<T>) {
    //         NEKO_ASSERT(data.type == LUA_TUSERDATA);
    //         return reinterpret_cast<T>(data.udata.ptr);
    //     } else {
    //         static_assert(std::is_same_v<T, void>, "Unsupported type for neko_w_lua_variant");
    //     }
    // }

    template <typename T>
    neko_w_lua_variant &operator=(const T &value) {
        using TT = std::decay_t<T>;
        if constexpr (std::is_same_v<TT, s32> || std::is_same_v<TT, u32> || std::is_same_v<TT, f32> || std::is_same_v<TT, f64>) {
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
                NEKO_ASSERT(type == LUA_TNONE);
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
        NEKO_ASSERT(cname.data != NULL);

        lua_State *L = ENGINE_LUA();

        lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_NAME::W_CORE);   // # 1
        lua_getiuservalue(L, -1, W_LUA_UPVALUES::NEKO_W_COMPONENTS_NAME);  // # 2
        lua_getfield(L, -1, W_LUA_REGISTRY_NAME::CVAR_MAP);                // # 3
        lua_pushinteger(L, cname.hash());                                  // 使用 32 位哈希以适应 Lua 数字范围
        lua_gettable(L, -2);                                               // # 4

        lua_getfield(L, -1, "type");  // # 5
        this->type = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "data");  // # 5
        switch (type) {
            case LUA_TNONE:
                NEKO_ASSERT(type == LUA_TNONE);
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
        lua_pop(L, 1);

        lua_pop(L, 4);

        NEKO_ASSERT(lua_gettop(L) == 0);
    }

    template <typename T>
    auto get() -> T {
        using TT = std::decay_t<T>;
        if (type == LUA_TNONE) {
        }
        if constexpr (std::is_same_v<TT, s32> || std::is_same_v<TT, u32> || std::is_same_v<TT, f32> || std::is_same_v<TT, f64>) {
            NEKO_ASSERT(type == LUA_TNUMBER);
            return data.number;
        } else if constexpr (std::is_same_v<TT, const_str>) {
            NEKO_ASSERT(type == LUA_TSTRING);
            return data.str;
        } else if constexpr (std::is_same_v<TT, bool>) {
            NEKO_ASSERT(type == LUA_TBOOLEAN);
            return data.boolean;
        } else {
            static_assert("unsupported type for neko_w_lua_variant::get");
        }
    }

    void make() {
        NEKO_ASSERT(type != LUA_TNONE);

        lua_State *L = ENGINE_LUA();

        lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_NAME::W_CORE);  // # 1

        lua_getiuservalue(L, -1, NEKO_W_COMPONENTS_NAME);  // # 2
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, W_LUA_REGISTRY_NAME::CVAR_MAP);  // # 3
            if (lua_istable(L, -1)) {
                lua_pushinteger(L, cname.hash());  // 使用 32 位哈希以适应 Lua 数字范围

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

                    lua_pushinteger(L, cname.hash());  // 重新推入键
                    lua_pushvalue(L, -2);              // 将新表复制到栈顶
                    lua_settable(L, -4);               // 将新表设置到 CVAR_MAP 中
                }

                lua_pop(L, 1);  // # pop 4

                lua_pop(L, 1);  // # pop 3
            } else {
                NEKO_ERROR("%s", "failed to get W_LUA_REGISTRY_NAME::CVAR_MAP");
                lua_pop(L, 1);  // # pop 3
            }
            lua_pop(L, 1);  // # pop 2
        } else {
            NEKO_ERROR("%s", "failed to get upvalue NEKO_W_COMPONENTS_NAME");
            lua_pop(L, 1);  // # pop 2
        }

        lua_pop(L, 1);  // # pop 1

        NEKO_ASSERT(lua_gettop(L) == 0);
    }
};

static_assert(std::is_trivially_copyable_v<neko_w_lua_variant>);

#endif