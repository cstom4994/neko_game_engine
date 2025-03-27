
#include "base/scripting/lua_wrapper.hpp"
#include "base/common/logger.hpp"

enum class TYPEID { tid_int8, tid_int16, tid_int32, tid_int64, tid_uint8, tid_uint16, tid_uint32, tid_uint64, tid_bool, tid_ptr, tid_float, tid_double, tid_COUNT };

#define TYPE_ID_(type) (TYPEID::tid_##type)

// 类型特征模板
template <typename T>
struct type_traits;

#define DEFINE_TYPE_TRAITS(type, tid)                  \
    template <>                                        \
    struct type_traits<type> {                         \
        using c_type = type;                           \
        static constexpr TYPEID type_id = tid;         \
        static constexpr size_t stride = sizeof(type); \
        static constexpr const char* name = #type;     \
    };

DEFINE_TYPE_TRAITS(int8_t, TYPEID::tid_int8)
DEFINE_TYPE_TRAITS(int16_t, TYPEID::tid_int16)
DEFINE_TYPE_TRAITS(int32_t, TYPEID::tid_int32)
DEFINE_TYPE_TRAITS(int64_t, TYPEID::tid_int64)
DEFINE_TYPE_TRAITS(uint8_t, TYPEID::tid_uint8)
DEFINE_TYPE_TRAITS(uint16_t, TYPEID::tid_uint16)
DEFINE_TYPE_TRAITS(uint32_t, TYPEID::tid_uint32)
DEFINE_TYPE_TRAITS(uint64_t, TYPEID::tid_uint64)
DEFINE_TYPE_TRAITS(bool, TYPEID::tid_bool)
DEFINE_TYPE_TRAITS(void*, TYPEID::tid_ptr)
DEFINE_TYPE_TRAITS(float, TYPEID::tid_float)
DEFINE_TYPE_TRAITS(double, TYPEID::tid_double)

// 通用设置函数模板
template <typename T>
void set_value(void* p, lua_State* L, int arg) {
    if constexpr (std::is_same_v<T, void*>) {
        *static_cast<T*>(p) = lua_touserdata(L, arg);
    } else if constexpr (std::is_floating_point_v<T>) {
        *static_cast<T*>(p) = static_cast<T>(luaL_checknumber(L, arg));
    } else if constexpr (std::is_same_v<T, bool>) {
        *static_cast<uint8_t*>(p) = lua_toboolean(L, arg);
    } else {
        *static_cast<T*>(p) = static_cast<T>(luaL_checkinteger(L, arg));
    }
}

// 通用获取函数模板
template <typename T>
void get_value(void* p, lua_State* L) {
    if constexpr (std::is_same_v<T, void*>) {
        lua_pushlightuserdata(L, *static_cast<T*>(p));
    } else if constexpr (std::is_floating_point_v<T>) {
        lua_pushnumber(L, *static_cast<T*>(p));
    } else if constexpr (std::is_same_v<T, bool>) {
        lua_pushboolean(L, *static_cast<uint8_t*>(p));
    } else {
        lua_pushinteger(L, *static_cast<T*>(p));
    }
}

// 类型分发器
template <TYPEID tid>
struct type_dispatcher;

#define DEFINE_TYPE_DISPATCHER(tid, type)                                                 \
    template <>                                                                           \
    struct type_dispatcher<tid> {                                                         \
        using c_type = type;                                                              \
        static void set(void* p, lua_State* L, int arg) { set_value<c_type>(p, L, arg); } \
        static void get(void* p, lua_State* L) { get_value<c_type>(p, L); }               \
        static constexpr size_t stride = type_traits<c_type>::stride;                     \
    };

DEFINE_TYPE_DISPATCHER(TYPEID::tid_int8, int8_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_int16, int16_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_int32, int32_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_int64, int64_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_uint8, uint8_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_uint16, uint16_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_uint32, uint32_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_uint64, uint64_t)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_bool, bool)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_ptr, void*)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_float, float)
DEFINE_TYPE_DISPATCHER(TYPEID::tid_double, double)

template <typename... Ts>
inline constexpr auto type_traits_tuple_impl = std::tuple<type_traits<Ts>...>{};

// 组合所有类型
inline constexpr auto type_traits_tuple = type_traits_tuple_impl<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, bool, void*, float, double>;

// 获得类型步长(sizeof大小)
static inline int get_stride(TYPEID type) {
    auto ff = [&]<typename T>(T& traits) -> std::optional<size_t> {
        // LOG_TRACE("get_stride {} {}", reflection::GetTypeName<decltype(traits)>(), traits.name);
        if (traits.type_id == type) {
            return traits.stride;
        }
        return std::nullopt;
    };
    size_t ret = std::apply(
            [&](auto&&... args) -> size_t {
                std::optional<size_t> result;
                ((result = ff(args) ? ff(args) : result), ...);  // 只有匹配的会覆盖 result
                return result.value_or(0);                       // 返回找到的值或默认值
            },
            type_traits_tuple);
    neko_assert(ret > 0);
    return (int)ret;
}

// 直接访问
template <TYPEID tid>
static int generic_getter(lua_State* L) {
    void* p = lua_touserdata(L, 1);
    const int offset = lua_tointeger(L, lua_upvalueindex(1));
    p = static_cast<u8*>(p) + offset * type_dispatcher<tid>::stride;
    type_dispatcher<tid>::get(p, L);
    return 1;
}

template <TYPEID tid>
static int generic_setter(lua_State* L) {
    void* p = lua_touserdata(L, 1);
    const int offset = lua_tointeger(L, lua_upvalueindex(1));
    p = static_cast<u8*>(p) + offset * type_dispatcher<tid>::stride;
    type_dispatcher<tid>::set(p, L, 2);
    return 0;
}

static int getter_direct(lua_State* L) {
    TYPEID type = (TYPEID)luaL_checkinteger(L, 1);
    if (type < TYPEID::tid_int8 || type >= TYPEID::tid_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }

    lua_pushinteger(L, offset / stride);

    TYPEID ret = std::apply(
            [&](auto&&... args) -> TYPEID {
                TYPEID type_id = TYPEID::tid_COUNT;
                auto f = [&](auto&& traits) -> void {
                    if (type == traits.type_id) {
                        lua_pushcclosure(L, generic_getter<traits.type_id>, 1);  // 使用 traits.type_id 作为模板参数
                        type_id = traits.type_id;
                    }
                };
                (f(args), ...);  // 逗号表达式展开每个类型
                return type_id;  // 返回找到的类型 若没有则为 tid_COUNT
            },
            type_traits_tuple);

    if (ret == TYPEID::tid_COUNT) return luaL_error(L, "Unsupported type");

    return 1;
}

static int setter_direct(lua_State* L) {
    TYPEID type = (TYPEID)luaL_checkinteger(L, 1);
    if (type < TYPEID::tid_int8 || type >= TYPEID::tid_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }

    lua_pushinteger(L, offset / stride);

    TYPEID ret = std::apply(
            [&](auto&&... args) -> TYPEID {
                TYPEID type_id = TYPEID::tid_COUNT;
                auto f = [&](auto&& traits) -> void {
                    if (type == traits.type_id) {
                        lua_pushcclosure(L, generic_setter<traits.type_id>, 1);  // 使用 traits.type_id 作为模板参数
                        type_id = traits.type_id;
                    }
                };
                (f(args), ...);  // 逗号表达式展开每个类型
                return type_id;  // 返回找到的类型 若没有则为 tid_COUNT
            },
            type_traits_tuple);

    if (ret == TYPEID::tid_COUNT) return luaL_error(L, "Unsupported type");

    return 1;
}

// 偏移量相关函数生成
template <TYPEID tid, int N>
void register_offset(lua_State* L) {
    lua_pushinteger(L, N);
    lua_pushcclosure(L, &generic_getter<tid>, 1);
    lua_setfield(L, -2, ("get_" + std::to_string(N)).c_str());

    lua_pushinteger(L, N);
    lua_pushcclosure(L, &generic_setter<tid>, 1);
    lua_setfield(L, -2, ("set_" + std::to_string(N)).c_str());
}

// 注册所有偏移量函数
template <TYPEID tid>
void register_offsets(lua_State* L) {
    register_offset<tid, 0>(L);
    register_offset<tid, 1>(L);
    register_offset<tid, 2>(L);
    register_offset<tid, 3>(L);
    register_offset<tid, 4>(L);
    register_offset<tid, 5>(L);
    register_offset<tid, 6>(L);
    register_offset<tid, 7>(L);
}

// 类型注册入口
template <TYPEID tid>
void register_type(lua_State* L) {
    lua_newtable(L);
    register_offsets<tid>(L);
    // lua_setfield(L, -2, typeid(typename type_dispatcher<tid>::c_type).name());

    lua_pushinteger(L, (int)tid);
    lua_setfield(L, -2, "typeid");
}

#define SET_TYPEID(type, name)                   \
    lua_pushinteger(L, (int)TYPEID::tid_##type); \
    lua_setfield(L, -2, #name);

static int newudata(lua_State* L) {
    size_t sz = luaL_checkinteger(L, 1);
    lua_newuserdatauv(L, sz, 0);
    return 1;
}

int open_cdata(lua_State* L) {
    luaL_checkversion(L);

    luaL_Reg l[] = {
            {"udata", newudata},
            {"NULL", NULL},
            {NULL, NULL},
    };
    luaL_newlib(L, l);

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                int top = lua_gettop(L);
                if (top <= 2) {
                    getter_direct(L);
                    setter_direct(L);
                    return 2;
                } else {
                    return luaL_error(L, "unsupport unpack offset");
                }
            },
            0);
    lua_setfield(L, -2, "parser");

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                if (!lua_isuserdata(L, 1)) return luaL_error(L, "userdata should at #1");
                char* p = (char*)lua_touserdata(L, 1);
                size_t off = luaL_checkinteger(L, 2);
                lua_pushlightuserdata(L, (void*)(p + off));
                return 1;
            },
            0);
    lua_setfield(L, -2, "offset");

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                lua_newtable(L);
                std::apply(
                        [&](auto&&... args) {
                            auto f = [&]<typename T>(T& traits) {
                                const TYPEID type_id = traits.type_id;
                                register_type<type_id>(L);
                                lua_setfield(L, -2, traits.name);
                            };
                            int dummy[] = {0, ((void)f(args), 0)...};
                            std::ignore = dummy;
                        },
                        type_traits_tuple);
                return 1;
            },
            0);
    lua_setfield(L, -2, "ctype");

    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");

    return 1;
}