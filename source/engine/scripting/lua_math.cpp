
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/base/common/math.hpp"

#define NEKO_MATH_METATABLE "mt_math"

enum class LuaMathValueType {
    VEC2,
    VEC3,
};

struct LuaMathValue {
    LuaMathValueType type;
    union {
        vec2 v2;
        vec3 v3;
    } u;
};

constexpr auto UP_X = 1;
constexpr auto UP_Y = 2;
constexpr auto UP_Z = 3;
constexpr auto UP_W = 4;
constexpr auto UP_COUNT = 4;

struct LuaMathStack {
private:
    int cap = 0;
    int count = 0;
    LuaMathValue buf[64];

    LuaMathValue *PushValue(LuaMathValueType t) {
        reserve(1);
        auto *v = &data[count++];
        v->type = t;
        return v;
    }

public:
    bool released = false;
    LuaMathValue *data = nullptr;

    bool reset() {
        if (released) return false;
        count = 0;
        cap = sizeof(buf) / sizeof(buf[0]);
        if (data != buf) {
            mem_free(data);
            data = buf;
        }
        return true;
    }

    vec2 &new_vec2(int *id) {
        auto *val = PushValue(LuaMathValueType::VEC2);
        *id = val - data;
        return val->u.v2;
    }

    vec3 &new_vec3(int *id) {
        auto *val = PushValue(LuaMathValueType::VEC3);
        *id = val - data;
        return val->u.v3;
    }

    inline vec2 &get_vec2(lua_State *L, int arg) {
        auto &val = get(L, arg, LuaMathValueType::VEC2);
        return val.u.v2;
    }

    inline vec3 &get_vec3(lua_State *L, int arg) {
        auto &val = get(L, arg, LuaMathValueType::VEC3);
        return val.u.v3;
    }

    inline LuaMathValue &get(lua_State *L, int arg) {
        int id = luaL_checkinteger(L, arg);
        luaL_argcheck(L, id < count, arg, "invalid value");
        return data[id];
    }

    inline LuaMathValue &get(lua_State *L, int arg, LuaMathValueType t) {
        auto &v = get(L, arg);
        luaL_argcheck(L, v.type == t, arg, "invalid value");
        return v;
    }

    void reserve(int n = 1) {
        int need = count + n;
        if (need > cap) {
            while (cap < need) cap *= 2;
            if (data == buf) {
                data = (LuaMathValue *)mem_alloc(sizeof(data[0]) * cap);
                std::memcpy(data, buf, sizeof(buf));
            } else {
                void *ptr = data;
                data = (LuaMathValue *)mem_realloc(ptr, cap * sizeof(data[0]));
            }
        }
    }
};

static int wrap_mathstack_gc(lua_State *L) {
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    stack->reset();
    return 0;
}

static int wrap_mathstack_close(lua_State *L) {
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    if (stack->reset()) {
        int n = lua_rawlen(L, lua_upvalueindex(1));
        lua_pushvalue(L, 1);
        lua_seti(L, lua_upvalueindex(1), n + 1);
    }
    return 0;
}

static int wrap_mathstack_push(lua_State *L) {
    LuaMathStack *ctx;
    int n = lua_rawlen(L, lua_upvalueindex(1));  // 对象池表
    if (n > 0) {
        lua_geti(L, lua_upvalueindex(1), n);
        lua_pushnil(L);
        lua_seti(L, lua_upvalueindex(1), n);
        ctx = (LuaMathStack *)lua_touserdata(L, 1);
        ctx->released = false;
    } else {
        ctx = (LuaMathStack *)lua_newuserdatauv(L, sizeof(LuaMathStack), 0);
        ctx->data = nullptr;
        ctx->released = false;
        ctx->reset();
        luaL_getmetatable(L, NEKO_MATH_METATABLE);
        lua_setmetatable(L, -2);
    }
    return 1;
}

static int wrap_mathstack_pop(lua_State *L) {
    int n = 0;
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    LuaMathValue &v = stack->get(L, 2);
    if (lua_gettop(L) == 3) {
        switch (v.type) {
            case LuaMathValueType::VEC2:
                for (int i = 0; i < 2; i++) {
                    lua_pushvalue(L, lua_upvalueindex(i + 1));
                    const float component = (i == 0) ? v.u.v2.x : v.u.v2.y;
                    lua_pushnumber(L, component);
                    lua_settable(L, 3);
                }
                break;
            case LuaMathValueType::VEC3:
                for (int i = 0; i < 3; i++) {
                    lua_pushvalue(L, lua_upvalueindex(i + 1));
                    const float component = (i == 0) ? v.u.v3.x : (i == 1) ? v.u.v3.y : v.u.v3.z;
                    lua_pushnumber(L, component);
                    lua_settable(L, 3);
                }
                break;
        }
        return 0;
    } else {
        switch (v.type) {
            case LuaMathValueType::VEC2:
                lua_pushnumber(L, v.u.v2.x);
                lua_pushnumber(L, v.u.v2.y);
                return 2;
            case LuaMathValueType::VEC3:
                lua_pushnumber(L, v.u.v3.x);
                lua_pushnumber(L, v.u.v3.y);
                lua_pushnumber(L, v.u.v3.z);
                return 3;
            default:
                return 0;
        }
        return 0;
    }
}

static int wrap_mathstack_dot(lua_State *L) {
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    auto &v1 = stack->get(L, 2);
    auto &v2 = stack->get(L, 3);
    float result = 0.0f;
    luaL_argcheck(L, v1.type == v2.type, 3, "different type");
    switch (v1.type) {
        case LuaMathValueType::VEC2:
            result = vec2_dot(v1.u.v2, v2.u.v2);
            break;
        case LuaMathValueType::VEC3:
            result = vec3_dot(v1.u.v3, v2.u.v3);
            break;
        default:
            return luaL_error(L, "wtf");
            break;
    }
    lua_pushnumber(L, result);
    return 1;
}

static int wrap_mathstack_mul(lua_State *L) {
    int v3_id = INT_MAX;
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    stack->reserve();
    auto &v1 = stack->get(L, 2);
    auto &v2 = stack->get(L, 3);

    luaL_argcheck(L, v2.type == v1.type, 2, "vec mul need same type");
    if (v1.type == LuaMathValueType::VEC2) {
        auto &v3 = stack->new_vec2(&v3_id);
        v3 = vec2_mul(v1.u.v2, v2.u.v2);
    } else {
        auto &v3 = stack->new_vec3(&v3_id);
        v3 = vec3_mul(v1.u.v3, v2.u.v3);
    }

    lua_pushinteger(L, v3_id);
    return 1;
}

static float GetValueField(lua_State *L, int table, int field) {
    float v;
    lua_pushvalue(L, lua_upvalueindex(field));
    lua_gettable(L, table);
    v = luaL_optnumber(L, -1, 0.0f);
    lua_pop(L, 1);
    return v;
}

static int wrap_vec2(lua_State *L) {
    int id;
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    auto &val = stack->new_vec2(&id);
    if (lua_type(L, 2) == LUA_TTABLE) {
        val.x = GetValueField(L, 2, UP_X);
        val.y = GetValueField(L, 2, UP_Y);
    } else {
        val.x = luaL_checknumber(L, 2);
        val.y = luaL_checknumber(L, 3);
    }
    lua_pushinteger(L, id);
    return 1;
}

static int wrap_vec3(lua_State *L) {
    int id;
    LuaMathStack *stack = (LuaMathStack *)luaL_checkudata(L, 1, NEKO_MATH_METATABLE);
    auto &val = stack->new_vec3(&id);
    if (lua_type(L, 2) == LUA_TTABLE) {
        val.x = GetValueField(L, 2, UP_X);
        val.y = GetValueField(L, 2, UP_Y);
        val.z = GetValueField(L, 2, UP_Z);
    } else {
        val.x = luaL_checknumber(L, 2);
        val.y = luaL_checknumber(L, 3);
        val.z = luaL_checknumber(L, 4);
    }
    lua_pushinteger(L, id);
    return 1;
}

#define VEC_OP_FUNC(name, op, type)                                                                    \
    static int wrap_##name(lua_State *L) {                                                             \
        LuaMathStack *stack = static_cast<LuaMathStack *>(luaL_checkudata(L, 1, NEKO_MATH_METATABLE)); \
        type &v1 = stack->get_##type(L, 2);                                                            \
        type &v2 = stack->get_##type(L, 3);                                                            \
        int id;                                                                                        \
        type &result = stack->new_##type(&id);                                                         \
        result = op(v1, v2);                                                                           \
        lua_pushinteger(L, id);                                                                        \
        return 1;                                                                                      \
    }

VEC_OP_FUNC(vec3_cross, vec3_cross, vec3)
VEC_OP_FUNC(vec3_project_onto, vec3_project_onto, vec3)
VEC_OP_FUNC(vec3_add, vec3_add, vec3)
VEC_OP_FUNC(vec3_sub, vec3_sub, vec3)
VEC_OP_FUNC(vec3_mul, vec3_mul, vec3)
VEC_OP_FUNC(vec3_div, vec3_div, vec3)

int open_math(lua_State *L) {

    // clang-format off

    luaL_Reg funcs[] = {
        {"push", nullptr},
        {"pop", wrap_mathstack_pop},
        {"dot", wrap_mathstack_dot},
        {"mul", wrap_mathstack_mul},
        {"v2", wrap_vec2},
        {"v3", wrap_vec3},
        {"v3_cross", wrap_vec3_cross},
        {"v3_project_onto", wrap_vec3_project_onto},
        {"v3_add", wrap_vec3_add},
        {"v3_sub", wrap_vec3_sub},
        {"v3_mul", wrap_vec3_mul},
        {"v3_div", wrap_vec3_div},
        {NULL, NULL},
    };
    luaL_Reg func_upval[] = {
        {"push", wrap_mathstack_push},
        {NULL, NULL},
    };

    // clang-format on

    // 对象池表
    // 用来复用LuaMathStack对象
    lua_newtable(L);
    if (luaL_newmetatable(L, NEKO_MATH_METATABLE "_weaktable")) {
        lua_pushliteral(L, "kv");
        lua_setfield(L, -2, "__mode");
    }
    lua_setmetatable(L, -2);

    luaL_newlibtable(L, funcs);  // 创建空表 准备填充函数
    lua_pushliteral(L, "x");
    lua_pushliteral(L, "y");
    lua_pushliteral(L, "z");
    lua_pushliteral(L, "w");
    luaL_setfuncs(L, funcs, UP_COUNT);

    lua_pushvalue(L, -2);             // 复制对象池表
    luaL_setfuncs(L, func_upval, 1);  // 替换push为带upvalue的版本

    if (luaL_newmetatable(L, NEKO_MATH_METATABLE)) {
        lua_pushvalue(L, -2);                          // 主函数表
        lua_setfield(L, -2, "__index");                // 设置元方法 __index
        lua_pushvalue(L, -3);                          // 对象池表做upvalue
        lua_pushcclosure(L, wrap_mathstack_close, 1);  //
        lua_setfield(L, -2, "__close");                //
        lua_pushvalue(L, -3);                          // 对象池表做upvalue
        lua_pushcclosure(L, wrap_mathstack_gc, 1);     //
        lua_setfield(L, -2, "__gc");                   //
    }

    lua_pop(L, 1);       // 弹出元表
    lua_replace(L, -2);  // 用主函数表替换对象池表

    return 1;  // 返回主函数表
}
