#if !defined(NEKO_LUA_WRAPPER_HPP)
#define NEKO_LUA_WRAPPER_HPP

#include <bit>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <tuple>  // std::ignore
#include <typeindex>
#include <utility>
#include <variant>
#include <vector>

#include "base/common/base.hpp"
#include "base/common/reflection.hpp"
#include "luax.h"

namespace Neko {

namespace luabind {

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

inline lua_Integer luax_len(lua_State *L, i32 arg) {
    lua_len(L, arg);
    lua_Integer len = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return len;
}

inline void luax_geti(lua_State *L, i32 arg, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_gettable(L, arg);
}

// 将表值设置在堆栈顶部
inline void luax_set_number_field(lua_State *L, const char *key, lua_Number n) {
    lua_pushnumber(L, n);
    lua_setfield(L, -2, key);
}

inline void luax_set_int_field(lua_State *L, const char *key, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_setfield(L, -2, key);
}

inline void luax_set_string_field(lua_State *L, const char *key, const char *str) {
    lua_pushstring(L, str);
    lua_setfield(L, -2, key);
}

// 从表中获取值
inline lua_Number luax_number_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Number num = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return num;
}

inline lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optnumber(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

inline lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Integer num = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return num;
}

inline lua_Number luax_opt_int_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optinteger(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

inline String luax_string_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, -1, &len);
    lua_pop(L, 1);
    return {str, len};
}

inline String luax_opt_string_field(lua_State *L, i32 arg, const char *key, const char *fallback) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_optlstring(L, -1, fallback, &len);
    lua_pop(L, 1);
    return {str, len};
}

inline bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback = false) {
    i32 type = lua_getfield(L, arg, key);

    bool b = fallback;
    if (type != LUA_TNIL) {
        b = lua_toboolean(L, -1);
    }

    lua_pop(L, 1);
    return b;
}

inline String luax_check_string(lua_State *L, i32 arg) {
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, arg, &len);
    return {str, len};
}

inline String luax_opt_string(lua_State *L, i32 arg, String def) { return lua_isstring(L, arg) ? luax_check_string(L, arg) : def; }

inline void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l) {
    luaL_newmetatable(L, mt_name);
    luaL_setfuncs(L, l, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

enum {
    LUAX_UD_TNAME = 1,
    LUAX_UD_PTR_SIZE = 2,
};

#if !defined(NEKO_LUAJIT)
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
#else
template <typename T>
void luax_new_userdata(lua_State *L, T data, const char *tname) {
    T *new_udata = (T *)lua_newuserdata(L, sizeof(T));

    // 为用户数据设置元表
    luaL_getmetatable(L, tname);
    lua_setmetatable(L, -2);

    memcpy(new_udata, &data, sizeof(T));

    // 额外的值使用lua_setfield将其存储在表中
    lua_newtable(L);

    lua_pushstring(L, tname);
    lua_setfield(L, -2, "tname");

    lua_pushnumber(L, sizeof(T));
    lua_setfield(L, -2, "size");

    // 将这个表作为用户数据的环境表存储
    lua_setfenv(L, -2);
}
#endif

#define luax_ptr_userdata luax_new_userdata

struct LuaNil {};

namespace detail {

template <typename T, typename I>
constexpr bool check_integral_limit(I i) {
    static_assert(std::is_integral_v<I>);
    static_assert(std::is_integral_v<T>);
    static_assert(sizeof(I) >= sizeof(T));
    if constexpr (sizeof(I) == sizeof(T)) {
        return true;
    } else if constexpr (std::numeric_limits<I>::is_signed == std::numeric_limits<T>::is_signed) {
        return i >= std::numeric_limits<T>::lowest() && i <= (std::numeric_limits<T>::max)();
    } else if constexpr (std::numeric_limits<I>::is_signed) {
        return static_cast<std::make_unsigned_t<I>>(i) >= std::numeric_limits<T>::lowest() && static_cast<std::make_unsigned_t<I>>(i) <= (std::numeric_limits<T>::max)();
    } else {
        return static_cast<std::make_signed_t<I>>(i) >= std::numeric_limits<T>::lowest() && static_cast<std::make_signed_t<I>>(i) <= (std::numeric_limits<T>::max)();
    }
}

union lua_maxalign_t {
    LUAI_MAXALIGN;
};
constexpr inline size_t lua_maxalign = std::alignment_of_v<lua_maxalign_t>;

template <typename T>
constexpr T *udata_align(void *storage) {
    if constexpr (std::alignment_of_v<T> > lua_maxalign) {
        uintptr_t mask = (uintptr_t)(std::alignment_of_v<T> - 1);
        storage = (void *)(((uintptr_t)storage + mask) & ~mask);
        return static_cast<T *>(storage);
    } else {
        return static_cast<T *>(storage);
    }
}

template <typename T>
constexpr T *udata_new(lua_State *L, int nupvalue) {
    if constexpr (std::alignment_of_v<T> > lua_maxalign) {
        void *storage = lua_newuserdatauv(L, sizeof(T) + std::alignment_of_v<T>, nupvalue);
        std::memset(storage, 0, sizeof(T));
        return udata_align<T>(storage);
    } else {
        void *storage = lua_newuserdatauv(L, sizeof(T), nupvalue);
        std::memset(storage, 0, sizeof(T));
        std::memset(storage, 0, sizeof(T));
        return udata_align<T>(storage);
    }
}

template <typename T>
T checklightud(lua_State *L, int arg) {
    luaL_checktype(L, arg, LUA_TLIGHTUSERDATA);
    return tolightud<T>(L, arg);
}

template <typename T>
T &toudata(lua_State *L, int arg) {
    return *udata_align<T>(lua_touserdata(L, arg));
}

template <typename T>
T *toudata_ptr(lua_State *L, int arg) {
    return udata_align<T>(lua_touserdata(L, arg));
}

template <typename T>
struct udata {};
template <typename T, typename = void>
struct udata_has_nupvalue : std::false_type {};
template <typename T>
struct udata_has_nupvalue<T, std::void_t<decltype(udata<T>::nupvalue)>> : std::true_type {};

template <typename T>
int destroyudata(lua_State *L) {
    toudata<T>(L, 1).~T();
    return 0;
}

template <typename T>
void getmetatable(lua_State *L) {
    if (luaL_newmetatable(L, reflection::name_v<T>.data())) {
        if constexpr (!std::is_trivially_destructible<T>::value) {
            lua_pushcfunction(L, destroyudata<T>);
            lua_setfield(L, -2, "__gc");
        }
        udata<T>::metatable(L);
    }
}

template <typename T, typename... Args>
T &newudata(lua_State *L, Args &&...args) {
    int nupvalue = 0;
    if constexpr (udata_has_nupvalue<T>::value) {
        nupvalue = udata<T>::nupvalue;
    }
    T *o = udata_new<T>(L, nupvalue);
    new (o) T(std::forward<Args>(args)...);
    getmetatable<T>(L);
    lua_setmetatable(L, -2);
    return *o;
}

template <typename T>
T &checkudata(lua_State *L, int arg, const_str tname = reflection::name_v<T>.data()) {
    return *udata_align<T>(luaL_checkudata(L, arg, tname));
}

inline std::string_view checkstrview(lua_State *L, int idx) {
    size_t len = 0;
    const char *buf = luaL_checklstring(L, idx, &len);
    return {buf, len};
}

template <typename T, typename I>
constexpr bool checklimit(I i) {
    static_assert(std::is_integral_v<I>);
    static_assert(std::is_integral_v<T>);
    static_assert(sizeof(I) >= sizeof(T));
    if constexpr (sizeof(I) == sizeof(T)) {
        return true;
    } else if constexpr (std::numeric_limits<I>::is_signed == std::numeric_limits<T>::is_signed) {
        return i >= std::numeric_limits<T>::lowest() && i <= (std::numeric_limits<T>::max)();
    } else if constexpr (std::numeric_limits<I>::is_signed) {
        return static_cast<std::make_unsigned_t<I>>(i) >= std::numeric_limits<T>::lowest() && static_cast<std::make_unsigned_t<I>>(i) <= (std::numeric_limits<T>::max)();
    } else {
        return static_cast<std::make_signed_t<I>>(i) >= std::numeric_limits<T>::lowest() && static_cast<std::make_signed_t<I>>(i) <= (std::numeric_limits<T>::max)();
    }
}

template <typename T>
T checkinteger(lua_State *L, int arg) {
    static_assert(std::is_trivial_v<T>);
    if constexpr (std::is_enum_v<T>) {
        using UT = std::underlying_type_t<T>;
        return static_cast<T>(checkinteger<UT>(L, arg));
    } else if constexpr (std::is_integral_v<T>) {
        lua_Integer r = luaL_checkinteger(L, arg);
        if constexpr (std::is_same_v<T, lua_Integer>) {
            return r;
        } else if constexpr (sizeof(T) >= sizeof(lua_Integer)) {
            return static_cast<T>(r);
        } else {
            if (checklimit<T>(r)) {
                return static_cast<T>(r);
            }
            luaL_error(L, "bad argument '#%d' limit exceeded", arg);
            // std::unreachable();
            neko_assert(0, "unreachable");
        }
    } else {
        return std::bit_cast<T>(checkinteger<lua_Integer>(L, arg));
    }
}
template <typename T, T def>
T optinteger(lua_State *L, int arg) {
    static_assert(std::is_trivial_v<T>);
    if constexpr (std::is_enum_v<T>) {
        using UT = std::underlying_type_t<T>;
        return static_cast<T>(optinteger<UT, std::to_underlying(def)>(L, arg));
    } else if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_same_v<T, lua_Integer>) {
            return luaL_optinteger(L, arg, def);
        } else if constexpr (sizeof(T) == sizeof(lua_Integer)) {
            lua_Integer r = optinteger<lua_Integer, static_cast<lua_Integer>(def)>(L, arg);
            return static_cast<T>(r);
        } else if constexpr (sizeof(T) < sizeof(lua_Integer)) {
            lua_Integer r = optinteger<lua_Integer, static_cast<lua_Integer>(def)>(L, arg);
            if (checklimit<T>(r)) {
                return static_cast<T>(r);
            }
            luaL_error(L, "bad argument '#%d' limit exceeded", arg);
            // std::unreachable();
            neko_assert(0, "unreachable");
        } else {
            static_assert(checklimit<lua_Integer>(def));
            lua_Integer r = optinteger<lua_Integer, static_cast<lua_Integer>(def)>(L, arg);
            return static_cast<T>(r);
        }
    } else {
        // If std::bit_cast were not constexpr, it would fail here, so let it fail.
        return std::bit_cast<T>(optinteger<lua_Integer, std::bit_cast<lua_Integer>(def)>(L, arg));
    }
}

template <typename T>
T tolightud(lua_State *L, int arg) {
    if constexpr (std::is_integral_v<T>) {
        uintptr_t r = std::bit_cast<uintptr_t>(tolightud<void *>(L, arg));
        if constexpr (std::is_same_v<T, uintptr_t>) {
            return r;
        } else if constexpr (sizeof(T) >= sizeof(uintptr_t)) {
            return static_cast<T>(r);
        } else {
            if (checklimit<T>(r)) {
                return static_cast<T>(r);
            }
            luaL_error(L, "bad argument #%d limit exceeded", arg);
            // std::unreachable();
            neko_assert(0, "unreachable");
        }
    } else if constexpr (std::is_same_v<T, void *>) {
        return lua_touserdata(L, arg);
    } else if constexpr (std::is_pointer_v<T>) {
        return static_cast<T>(tolightud<void *>(L, arg));
    } else {
        return std::bit_cast<T>(tolightud<void *>(L, arg));
    }
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_same_v<std::decay_t<T>, LuaNil>
{
    lua_pushnil(L);
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_same_v<T, lua_CFunction>
{
    lua_pushcfunction(L, x);
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_same_v<T, lua_CFunction>
{
    x = lua_tocfunction(L, N);
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_same_v<T, bool>
{
    lua_pushboolean(L, x);
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_same_v<T, bool>
{
    x = lua_toboolean(L, N);
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_integral_v<T> && !std::is_same_v<T, bool>
{
    lua_pushinteger(L, x);
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_integral_v<T> && !std::is_same_v<T, bool>
{
    x = static_cast<T>(lua_tointeger(L, N));
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_floating_point_v<T>
{
    lua_pushnumber(L, x);
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_floating_point_v<T>
{
    x = static_cast<T>(lua_tonumber(L, N));
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_same_v<T, const char *>
{
    lua_pushstring(L, x);
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_same_v<T, const char *>
{
    x = lua_tostring(L, N);
}

template <typename T>
auto Push(lua_State *L, T x)
    requires std::is_same_v<T, std::string>
{
    lua_pushstring(L, x.c_str());
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_same_v<T, std::string>
{
    x = lua_tostring(L, N);
}

template <typename T, std::size_t N>
auto Push(lua_State *L, T (&arr)[N]) {
    lua_newtable(L);  // 创建 Lua 表
    int i = 0;
    for (auto &x : arr) {
        lua_pushinteger(L, ++i);  // Lua 索引从 1 开始
        detail::Push(L, x);       // 对单个元素递归调用 Push
        lua_settable(L, -3);      // 设置键值对
    }
}

template <typename T>
auto Get(lua_State *L, int idx, T &arr)
    requires std::is_bounded_array_v<T>
{
    lua_pushvalue(L, idx);
    constexpr std::size_t N = std::extent_v<T>;  // 推断数组大小
    for (std::size_t i = 0; i < N; ++i) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        detail::Get(L, -1, arr[i]);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

template <typename T>
auto Push(lua_State *L, T &x)
    requires std::is_enum_v<T>
{
    return detail::Push(L, static_cast<typename std::underlying_type<T>::type>(x));
}

template <typename T>
auto Get(lua_State *L, int N, T &x)
    requires std::is_enum_v<T>
{
    typename std::underlying_type<T>::type new_x;
    detail::Get(L, N, new_x);
    x = static_cast<T>(new_x);
}

struct LuaStack {
    template <typename T>
    static inline void Push(lua_State *L, T &&any) {
        detail::Push(L, std::forward<T>(any));
    }
    template <typename T>
    static inline auto Get(lua_State *L, int index, T &value) {
        return detail::Get<T>(L, index, value);
    }
};

}  // namespace detail

namespace detail {

// 自动弹出栈元素 确保数量不变的辅助类
class StackGuard {
public:
    explicit StackGuard(lua_State *L) : m_L(L), m_count(::lua_gettop(L)) {}
    ~StackGuard() {
        int n = ::lua_gettop(m_L);
        assert(n >= m_count);
        if (n > m_count) {
            ::lua_pop(m_L, (n - m_count));
        }
    }

private:
    lua_State *m_L;
    int m_count;
};

}  // namespace detail

class LuaRef;

class LuaRefBase {
protected:
    lua_State *L;
    int m_ref;
    struct FromStackIndex {};

    // 不应该直接使用
    explicit LuaRefBase(lua_State *L, FromStackIndex) : L(L) { m_ref = luaL_ref(L, LUA_REGISTRYINDEX); }
    explicit LuaRefBase(lua_State *L, int ref) : L(L), m_ref(ref) {}
    ~LuaRefBase() { luaL_unref(L, LUA_REGISTRYINDEX, m_ref); }

public:
    virtual void Push() const { lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); }

    std::string tostring() const {
        lua_getglobal(L, "tostring");
        Push();
        lua_call(L, 1, 1);
        const char *str = lua_tostring(L, 1);
        lua_pop(L, 1);
        return std::string(str);
    }

    int Type() const {
        int result;
        Push();
        result = lua_type(L, -1);
        lua_pop(L, 1);
        return result;
    }

    inline bool IsNil() const { return Type() == LUA_TNIL; }
    inline bool IsNumber() const { return Type() == LUA_TNUMBER; }
    inline bool IsString() const { return Type() == LUA_TSTRING; }
    inline bool IsTable() const { return Type() == LUA_TTABLE; }
    inline bool IsFunction() const { return Type() == LUA_TFUNCTION; }
    inline bool IsUserdata() const { return Type() == LUA_TUSERDATA; }
    inline bool IsThread() const { return Type() == LUA_TTHREAD; }
    inline bool IsLightUserdata() const { return Type() == LUA_TLIGHTUSERDATA; }
    inline bool IsBool() const { return Type() == LUA_TBOOLEAN; }

    template <typename... Args>
    inline LuaRef const operator()(Args... args) const;

    template <typename... Args>
    inline void Call(int ret, Args... args) const;

    template <typename T>
    void Append(T v) const {
        Push();
        size_t len = lua_rawlen(L, -1);
        detail::LuaStack::Push(L, v);
        lua_rawseti(L, -2, ++len);
        lua_pop(L, 1);
    }

    template <typename T>
    T Cast() {
        detail::StackGuard p(L);
        Push();
        T t{};
        detail::LuaStack::Get(L, -1, t);
        return t;
    }

    template <typename T>
    operator T() {
        return Cast<T>();
    }
};

template <typename K>
class LuaTableElement : public LuaRefBase {
    friend class LuaRef;

private:
    K m_key;

public:
    LuaTableElement(lua_State *L, K key) : LuaRefBase(L, FromStackIndex()), m_key(key) {}

    void Push() const override {
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        detail::LuaStack::Push(L, m_key);
        lua_gettable(L, -2);
        lua_remove(L, -2);
    }

    // 为该表/键分配一个新值
    template <typename T>
    LuaTableElement &operator=(T v) {
        detail::StackGuard p(L);
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        detail::LuaStack::Push(L, m_key);
        detail::LuaStack::Push(L, v);
        lua_settable(L, -3);
        return *this;
    }

    template <typename NK>
    LuaTableElement<NK> operator[](NK key) const {
        Push();
        return LuaTableElement<NK>(L, key);
    }
};

namespace detail {

template <typename T>
auto Push(lua_State *L, LuaTableElement<T> const &e) {
    e.Push();
}

}  // namespace detail

class LuaRef : public LuaRefBase {
    friend LuaRefBase;

private:
    explicit LuaRef(lua_State *L, FromStackIndex fs) : LuaRefBase(L, fs) {}

public:
    LuaRef(lua_State *L) : LuaRefBase(L, LUA_REFNIL) {}

    LuaRef(lua_State *L, const std::string &global) : LuaRefBase(L, LUA_REFNIL) {
        lua_getglobal(L, global.c_str());
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LuaRef(LuaRef const &other) : LuaRefBase(other.L, LUA_REFNIL) {
        other.Push();
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LuaRef(LuaRef &&other) noexcept : LuaRefBase(other.L, other.m_ref) { other.m_ref = LUA_REFNIL; }

    LuaRef &operator=(LuaRef &&other) noexcept {
        if (this == &other) return *this;

        std::swap(L, other.L);
        std::swap(m_ref, other.m_ref);

        return *this;
    }

    LuaRef &operator=(LuaRef const &other) {
        if (this == &other) return *this;
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.Push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaRef &operator=(LuaTableElement<K> &&other) noexcept {
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.Push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaRef &operator=(LuaTableElement<K> const &other) {
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.Push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaTableElement<K> operator[](K key) const {
        Push();
        return LuaTableElement<K>(L, key);
    }

    static LuaRef FromStack(lua_State *L, int index = -1) {
        lua_pushvalue(L, index);
        return LuaRef(L, FromStackIndex());
    }

    static LuaRef NewTable(lua_State *L) {
        lua_newtable(L);
        return LuaRef(L, FromStackIndex());
    }

    static LuaRef GetGlobal(lua_State *L, char const *name) {
        lua_getglobal(L, name);
        return LuaRef(L, FromStackIndex());
    }
};

namespace detail {

inline auto Push(lua_State *L, LuaRef const &r) { r.Push(); }

}  // namespace detail

template <>
inline LuaRef const LuaRefBase::operator()() const {
    Push();
    luax_pcall(L, 0, 1);
    return LuaRef(L, FromStackIndex());
}

template <typename... Args>
inline LuaRef const LuaRefBase::operator()(Args... args) const {
    const int n = sizeof...(Args);
    Push();
    int dummy[] = {0, ((void)detail::LuaStack::Push<Args>(L, std::forward<Args>(args)), 0)...};
    std::ignore = dummy;
    luax_pcall(L, n, 1);
    return LuaRef(L, FromStackIndex());
}

template <>
inline void LuaRefBase::Call(int ret) const {
    Push();
    luax_pcall(L, 0, ret);
    return;  // 如果有返回值保留在 Lua 堆栈中
}

template <typename... Args>
inline void LuaRefBase::Call(int ret, Args... args) const {
    const int n = sizeof...(Args);
    Push();
    int dummy[] = {0, ((void)detail::LuaStack::Push<Args>(L, std::forward<Args>(args)), 0)...};
    std::ignore = dummy;
    luax_pcall(L, n, ret);
    return;  // 如果有返回值保留在 Lua 堆栈中
}

template <>
inline LuaRef LuaRefBase::Cast() {
    Push();
    return LuaRef(L, FromStackIndex());
}

template <lua_CFunction func>
int Wrap(lua_State *L) {
    int result = 0;
    try {
        result = func(L);
    }
    // 将带有描述的异常转换为 lua_error
    catch (std::exception &e) {
        luaL_error(L, e.what());
    }
    // 重新抛出任何其他异常 (例如lua_error)
    catch (...) {
        throw;
    }
    return result;
}

[[nodiscard]] static inline std::string_view PushFQN(lua_State *const L, int const t, int const last) {
    luaL_Buffer _b;
    luaL_buffinit(L, &_b);
    int i{1};
    for (; i < last; ++i) {
        lua_rawgeti(L, t, i);
        luaL_addvalue(&_b);
        luaL_addlstring(&_b, "/", 1);
    }
    if (i == last) {  // 添加最后一个值(如果间隔不为空)
        lua_rawgeti(L, t, i);
        luaL_addvalue(&_b);
    }
    luaL_pushresult(&_b);  // &b 此时会弹出 (替换为result)
    return lua_tostring(L, -1);
}

// 返回一些有助于识别对象的名称
[[nodiscard]] static inline int DiscoverObjectNameRecur(lua_State *L, int shortest_, int depth_) {
    static constexpr int kWhat{1};
    static constexpr int kResult{2};
    static constexpr int kCache{3};
    static constexpr int kFQN{4};

    if (shortest_ <= depth_ + 1) {
        return shortest_;
    }
    assert(lua_checkstack(L, 3));

    lua_pushvalue(L, -1);
    lua_rawget(L, kCache);

    if (!lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return shortest_;
    }

    lua_pop(L, 1);
    lua_pushvalue(L, -1);
    lua_pushinteger(L, 1);
    lua_rawset(L, kCache);

    lua_pushnil(L);
    while (lua_next(L, -2)) {

        ++depth_;
        lua_pushvalue(L, -2);
        lua_rawseti(L, kFQN, depth_);
        if (lua_rawequal(L, -1, kWhat)) {

            if (depth_ < shortest_) {
                shortest_ = depth_;
                std::ignore = PushFQN(L, kFQN, depth_);
                lua_replace(L, kResult);
            }

            lua_pop(L, 2);
            break;
        }
        switch (lua_type(L, -1)) {
            default:
                break;

            case LUA_TTABLE:
                shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);

                if (lua_getmetatable(L, -1)) {
                    if (lua_istable(L, -1)) {
                        ++depth_;
                        lua_pushstring(L, "__metatable");
                        lua_rawseti(L, kFQN, depth_);
                        shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);
                        lua_pushnil(L);
                        lua_rawseti(L, kFQN, depth_);
                        --depth_;
                    }
                    lua_pop(L, 1);
                }
                break;

            case LUA_TTHREAD:

                break;

            case LUA_TUSERDATA:

                if (lua_getmetatable(L, -1)) {
                    if (lua_istable(L, -1)) {
                        ++depth_;
                        lua_pushstring(L, "__metatable");
                        lua_rawseti(L, kFQN, depth_);
                        shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);
                        lua_pushnil(L);
                        lua_rawseti(L, kFQN, depth_);
                        --depth_;
                    }
                    lua_pop(L, 1);
                }

                {
                    int _uvi{1};
                    while (lua_getiuservalue(L, -1, _uvi) != LUA_TNONE) {
                        if (lua_istable(L, -1)) {
                            ++depth_;
                            lua_pushstring(L, "uservalue");
                            lua_rawseti(L, kFQN, depth_);
                            shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);
                            lua_pushnil(L);
                            lua_rawseti(L, kFQN, depth_);
                            --depth_;
                        }
                        lua_pop(L, 1);
                        ++_uvi;
                    }

                    lua_pop(L, 1);
                }
                break;
        }

        lua_pop(L, 1);

        lua_pushnil(L);
        lua_rawseti(L, kFQN, depth_);
        --depth_;
    }

    lua_pushvalue(L, -1);
    lua_pushnil(L);
    lua_rawset(L, kCache);
    return shortest_;
}

static inline int l_nameof(lua_State *L) {
    int const _what{lua_gettop(L)};
    if (_what > 1) {
        luaL_argerror(L, _what, "too many arguments.");
    }

    if (lua_type(L, 1) < LUA_TTABLE) {
        lua_pushstring(L, luaL_typename(L, 1));
        lua_insert(L, -2);
        return 2;
    }

    lua_pushnil(L);

    lua_newtable(L);  // 所有已访问表的缓存

    lua_newtable(L);  // 其内容是字符串 连接时会产生唯一的名称
    lua_pushstring(L, LUA_GNAME);
    lua_rawseti(L, -2, 1);

    lua_pushglobaltable(L);  // 开始搜索
    std::ignore = DiscoverObjectNameRecur(L, std::numeric_limits<int>::max(), 1);
    if (lua_isnil(L, 2)) {
        lua_pop(L, 1);
        lua_pushstring(L, "_R");
        lua_rawseti(L, -2, 1);
        lua_pushvalue(L, LUA_REGISTRYINDEX);
        std::ignore = DiscoverObjectNameRecur(L, std::numeric_limits<int>::max(), 1);
    }
    lua_pop(L, 3);
    lua_pushstring(L, luaL_typename(L, 1));
    lua_replace(L, -3);
    return 2;
}

struct LuaVM {

    struct Tools {

        static void check_arg_count(lua_State *L, int expected) {
            int n = lua_gettop(L);
            if (n != expected) {
                luaL_error(L, "Got %d arguments, expected %d", n, expected);
                return;
            }
            return;
        };

        template <typename T>
        static void ForEachTable(lua_State *L, T f, int index, int indent = 0) {
            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                // for (int i = 0; i < indent; i++) f(" ");
                if (lua_type(L, -2) == LUA_TSTRING) {
                    f(index, lua_tostring(L, -2));
                } else if (lua_type(L, -2) == LUA_TNUMBER) {
                    f(index, lua_tonumber(L, -2));
                } else {
                    f(index, lua_typename(L, lua_type(L, -2)));
                }

                if (lua_type(L, -1) == LUA_TSTRING) {
                    f(index, lua_tostring(L, -1));
                } else if (lua_type(L, -1) == LUA_TNUMBER) {
                    f(index, lua_tonumber(L, -1));
                } else if (lua_type(L, -1) == LUA_TTABLE) {
                    f(index, "table");
                    ForEachTable(L, f, lua_gettop(L), indent + 1);
                } else {
                    f(index, lua_typename(L, lua_type(L, -1)));
                }
                lua_pop(L, 1);
            }
        }

        template <typename T>
        static void ForEachStack(lua_State *L, T f) {
            int top = lua_gettop(L);
            for (int i = 1; i <= top; i++) {
                int t = lua_type(L, i);
                switch (t) {
                    case LUA_TSTRING:
                        f(i, lua_tostring(L, i));
                        break;
                    case LUA_TBOOLEAN:
                        f(i, lua_toboolean(L, i) ? "true" : "false");
                        break;
                    case LUA_TNUMBER:
                        f(i, lua_tonumber(L, i));
                        break;
                    case LUA_TTABLE:
                        f(i, "table:");
                        ForEachTable(L, f, i);
                        break;
                    default:
                        f(i, lua_typename(L, t));
                        break;
                }
            }
        }
    };

    static void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize) {
        static size_t lua_mem_usage;
        if (!ptr) osize = 0;
        if (!nsize) {
            lua_mem_usage -= osize;
            free(ptr);
            return NULL;
        }
        lua_mem_usage += (nsize - osize);
        return realloc(ptr, nsize);
    }

    lua_State *L;

    LuaVM() = default;
    LuaVM(lua_State *l) : L(l) {}

    inline lua_State *Create() {

#ifdef _DEBUG
        lua_State *L = ::lua_newstate(Allocf, NULL);
#else
        lua_State *L = ::luaL_newstate();
#endif

        ::luaL_openlibs(L);

#ifdef NEKO_CFFI
        lua_gc(L, LUA_GCSETPAUSE, 150);
#endif

        lua_pushinteger(L, 0);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");

        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");

        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");

        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_sizes");
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_fields");

        luaL_Reg builtin_funcs[] = {
                {"nameof", Wrap<l_nameof>},
        };

        for (auto f : builtin_funcs) {
            lua_register(L, f.name, f.func);
        }

        this->L = L;

        return L;
    }

    inline void Fini(lua_State *L) {
        if (L) {

            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");

            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");

            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");

            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_sizes");
            lua_pushnil(L);
            lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_fields");

            int top = lua_gettop(L);
            if (top != 0) {
                Tools::ForEachStack(L, []<typename T>(int i, T v) -> int {
                    std::cout << i << ':' << v << std::endl;
                    return 0;
                });
                printf("luastack memory leak\n");
            }
            ::lua_close(L);
        }
    }

    // template <typename T>
    operator lua_State *() { return L; }

    inline void operator()(const std::string &func) const {
        lua_getglobal(L, func.c_str());

        if (lua_pcall(L, 0, 0, 0) != 0) {
            std::string err = lua_tostring(L, -1);
            ::lua_pop(L, 1);
            printf("%s", err.c_str());
        }
    }

    inline void RunString(const std::string &str) {
        if (luaL_dostring(L, str.c_str())) {
            std::string err = lua_tostring(L, -1);
            ::lua_pop(L, 1);
            printf("%s", err.c_str());
        }
    }
};

namespace LuaValue {
template <class>
inline constexpr bool always_false_v = false;

using Value = std::variant<std::monostate,  // LUA_TNIL
                           bool,            // LUA_TBOOLEAN
                           void *,          // LUA_TLIGHTUSERDATA
                           lua_Integer,     // LUA_TNUMBER
                           lua_Number,      // LUA_TNUMBER
                           std::string,     // LUA_TSTRING
                           lua_CFunction    // LUA_TFUNCTION
                           >;
using Table = std::map<std::string, Value>;

inline void Set(lua_State *L, int idx, Value &v) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            v.emplace<std::monostate>();
            break;
        case LUA_TBOOLEAN:
            v.emplace<bool>(!!lua_toboolean(L, idx));
            break;
        case LUA_TLIGHTUSERDATA:
            v.emplace<void *>(lua_touserdata(L, idx));
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, idx)) {
                v.emplace<lua_Integer>(lua_tointeger(L, idx));
            } else {
                v.emplace<lua_Number>(lua_tonumber(L, idx));
            }
            break;
        case LUA_TSTRING: {
            size_t sz = 0;
            const char *str = lua_tolstring(L, idx, &sz);
            v.emplace<std::string>(str, sz);
            break;
        }
        case LUA_TFUNCTION: {
            lua_CFunction func = lua_tocfunction(L, idx);
            if (func == NULL || lua_getupvalue(L, idx, 1) != NULL) {
                luaL_error(L, "Only light C function can be serialized");
                return;
            }
            v.emplace<lua_CFunction>(func);
            break;
        }
        default:
            luaL_error(L, "Unsupport type %s to serialize", lua_typename(L, idx));
    }
}

inline void Set(lua_State *L, int idx, Table &t) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        size_t sz = 0;
        const char *str = luaL_checklstring(L, -2, &sz);
        std::pair<std::string, Value> pair;
        pair.first.assign(str, sz);
        Set(L, -1, pair.second);
        t.emplace(pair);
        lua_pop(L, 1);
    }
}

inline void Get(lua_State *L, const Value &v) {
    std::visit(
            [=](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    lua_pushnil(L);
                } else if constexpr (std::is_same_v<T, bool>) {
                    lua_pushboolean(L, arg);
                } else if constexpr (std::is_same_v<T, void *>) {
                    lua_pushlightuserdata(L, arg);
                } else if constexpr (std::is_same_v<T, lua_Integer>) {
                    lua_pushinteger(L, arg);
                } else if constexpr (std::is_same_v<T, lua_Number>) {
                    lua_pushnumber(L, arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    lua_pushlstring(L, arg.data(), arg.size());
                } else if constexpr (std::is_same_v<T, lua_CFunction>) {
                    lua_pushcfunction(L, arg);
                } else {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            },
            v);
}

inline void Get(lua_State *L, const Table &t) {
    lua_createtable(L, 0, static_cast<int>(t.size()));
    for (const auto &[k, v] : t) {
        lua_pushlstring(L, k.data(), k.size());
        Get(L, v);
        lua_rawset(L, -3);
    }
}

}  // namespace LuaValue

int vfs_lua_loader(lua_State *L);

struct LUASTRUCT_CDATA {
    int ref;
    size_t cdata_size;
    const_str type_name;
};

static inline void LuaStructSetMetatable(lua_State *L, const char *metatable, int index) {
    luaL_getmetatable(L, metatable);
    if (lua_isnoneornil(L, -1)) luaL_error(L, "The metatable for %s has not been defined", metatable);
    lua_setmetatable(L, index - 1);
}

inline int LuaStructNew(lua_State *L, const char *metatable, size_t size) {
    LUASTRUCT_CDATA *reference = (LUASTRUCT_CDATA *)lua_newuserdata(L, sizeof(LUASTRUCT_CDATA) + size);
    std::memset(reference, 0, sizeof(LUASTRUCT_CDATA) + size);
    reference->ref = LUA_NOREF;
    reference->cdata_size = size;
    reference->type_name = metatable;
    void *data = (void *)(reference + 1);
    memset(data, 0, size);
    LuaStructSetMetatable(L, metatable, -1);
    return 1;
}

// ParentIndex 是包含对象的堆栈索引 或者 0 表示不包含对象
inline int LuaStructNewRef(lua_State *L, const char *metatable, int parentIndex, const void *data) {
    LUASTRUCT_CDATA *reference = (LUASTRUCT_CDATA *)lua_newuserdata(L, sizeof(LUASTRUCT_CDATA) + sizeof(data));

    if (parentIndex != 0) {
        // 存储对包含对象的引用
        lua_pushvalue(L, parentIndex);
        reference->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        reference->ref = LUA_REFNIL;
    }

    *((const void **)(reference + 1)) = data;

    LuaStructSetMetatable(L, metatable, -1);

    return 1;
}

inline int LuaStructGC(lua_State *L, const char *metatable) {
    LUASTRUCT_CDATA *reference = (LUASTRUCT_CDATA *)luaL_checkudata(L, 1, metatable);
    // printf("LuaStructGC %s %d %p\n", metatable, reference->ref, reference);
    luaL_unref(L, LUA_REGISTRYINDEX, reference->ref);
    return 0;
}

inline int LuaStructIs(lua_State *L, const char *metatable, int index) {
    if (lua_type(L, index) != LUA_TUSERDATA) {
        return 0;
    }
    lua_getmetatable(L, index);
    luaL_getmetatable(L, metatable);
    int metatablesMatch = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    return metatablesMatch;
}

template <typename T>
int LuaStructIs(lua_State *L, int index) {
    const char *name = reflection::GetTypeName<T>();
    return LuaStructIs(L, name, index);
}

inline const char *LuaStructFieldname(lua_State *L, int index, size_t *length) {
    luaL_argcheck(L, lua_type(L, index) == LUA_TSTRING, index, "Field name must be a string");

    return lua_tolstring(L, index, length);
}

template <typename T>
auto LuaStructTodata_w(lua_State *L, const char *metatable, int index, bool required) -> T * {
    if (required == false && lua_isnoneornil(L, index)) {
        return NULL;
    }
    LUASTRUCT_CDATA *reference = (LUASTRUCT_CDATA *)luaL_checkudata(L, index, metatable);
    if (reference->ref == LUA_NOREF) {
        return (T *)(reference + 1);
    } else {
        return *((T **)(reference + 1));
    }
}

template <typename T>
auto LuaStructTodata(lua_State *L, int index, bool required = true) -> T * {
    const char *typeName = reflection::GetTypeName<T>();
    return LuaStructTodata_w<T>(L, typeName, index, required);
}

template <typename T>
void LuaStructPush(lua_State *L, const T &value) {
    const char *typeName = reflection::GetTypeName<T>();
    LuaStructNew(L, typeName, sizeof(T));
    T *ptr = LuaStructTodata_w<T>(L, typeName, -1, true);
    *ptr = value;
}

template <typename T>
struct is_struct {
    static constexpr bool value = std::is_class<T>::value;
};

template <typename T>
struct LuaStructAccess {
    static inline int Get(lua_State *L, const char *fieldName, T *data, int parentIndex, int set, int valueIndex) {
        if constexpr (is_struct<T>::value) {
            if (set) {
                *data = *LuaStructTodata<T>(L, valueIndex);
                return 0;
            } else {
                return LuaStructNewRef(L, reflection::GetTypeName<T>(), parentIndex, data);
            }
        } else {
            if (set) {
                detail::LuaStack::Get<T>(L, valueIndex, *data);
                return 0;
            } else {
                detail::LuaStack::Push(L, *data);
                return 1;
            }
        }
    }
};

enum { NEKOLUA_INVALID_TYPE = -1 };

typedef lua_Integer LuaTypeid;
typedef int (*neko_luabind_Pushfunc)(lua_State *, LuaTypeid, const void *);
typedef void (*neko_luabind_Tofunc)(lua_State *, LuaTypeid, void *, int);

struct LuaTypeinfo {
    const char *name;
    size_t size;
};

template <typename T>
LuaTypeid LuaType(lua_State *L) {

    const char *type = reflection::GetTypeName<T>();
    constexpr size_t size = sizeof(T);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_getfield(L, -1, type);

    if (lua_isnumber(L, -1)) {

        LuaTypeid id = lua_tointeger(L, -1);
        lua_pop(L, 2);
        return id;

    } else {

        lua_pop(L, 2);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");

        LuaTypeid id = lua_tointeger(L, -1);
        lua_pop(L, 1);
        id++;

        lua_pushinteger(L, id);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
        lua_pushinteger(L, id);
        lua_setfield(L, -2, type);
        lua_pop(L, 1);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
        lua_pushinteger(L, id);
        lua_pushstring(L, type);
        lua_settable(L, -3);
        lua_pop(L, 1);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");
        lua_pushinteger(L, id);
        lua_pushinteger(L, size);
        lua_settable(L, -3);
        lua_pop(L, 1);

        return id;
    }
}

inline auto TypeFind(lua_State *L, const char *type) -> LuaTypeid {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_getfield(L, -1, type);
    LuaTypeid id = lua_isnil(L, -1) ? NEKOLUA_INVALID_TYPE : lua_tointeger(L, -1);
    lua_pop(L, 2);
    return id;
}

template <typename T>
inline LuaTypeinfo GetLuaTypeinfo(lua_State *L, T id);

template <>
inline LuaTypeinfo GetLuaTypeinfo(lua_State *L, LuaTypeid id) {

    LuaTypeinfo typeinfo{};

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
    lua_pushinteger(L, id);
    lua_gettable(L, -2);

    typeinfo.name = lua_isnil(L, -1) ? "NEKOLUA_INVALID_TYPE" : lua_tostring(L, -1);
    lua_pop(L, 2);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");
    lua_pushinteger(L, id);
    lua_gettable(L, -2);

    typeinfo.size = lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
    lua_pop(L, 2);

    return typeinfo;
}

template <>
inline LuaTypeinfo GetLuaTypeinfo(lua_State *L, const char *name) {
    return GetLuaTypeinfo(L, TypeFind(L, name));
}

template <typename T>
inline void LuaStructAddType(lua_State *L, LuaTypeid type) {
    constexpr auto N = reflection::field_count<T>;

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_fields");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_sizes");
    lua_pushinteger(L, type);
    lua_pushinteger(L, N);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

inline void LuaStructAddField(lua_State *L, LuaTypeid type, const char *field_type, const char *field_name) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_sizes");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        size_t size = lua_tointeger(L, -1);
        lua_pop(L, 2);

        lua_newtable(L);

        lua_pushstring(L, field_type);
        lua_setfield(L, -2, "value");

        lua_pushstring(L, field_name);
        lua_setfield(L, -2, "name");

        lua_setfield(L, -2, field_name);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_fields");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        lua_pushstring(L, field_type);
        lua_getfield(L, -4, field_name);
        lua_settable(L, -3);

        lua_pop(L, 4);
        return;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "LuaStructAddValue: Struct '%s' not registered!", GetLuaTypeinfo(L, type).name);
    lua_error(L);
}

inline bool LuaTypeIsStruct(lua_State *L, LuaTypeid type) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);
    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return reg;
}

template <typename T>
inline void LuaStructCreate(lua_State *L, const char *fieldName, const char *type_name, size_t type_size, T fieldaccess) {

    using fieldaccess_func = T;

    if (fieldName) {
        lua_createtable(L, 0, 0);

        lua_pushinteger(L, type_size);
        lua_pushstring(L, type_name);

        lua_pushcclosure(
                L,
                [](lua_State *L) -> int {
                    size_t _type_size = lua_tointeger(L, lua_upvalueindex(1));
                    const char *_type_name = lua_tostring(L, lua_upvalueindex(2));
                    return LuaStructNew(L, _type_name, _type_size);
                },
                2);
        lua_setfield(L, -2, "new");

        lua_pushstring(L, type_name);
        lua_pushcclosure(
                L,
                [](lua_State *L) -> int {
                    const char *_type_name = lua_tostring(L, lua_upvalueindex(1));
                    luaL_getmetatable(L, _type_name);

                    int mt1_idx = lua_absindex(L, -1);
                    int mt2_idx = lua_absindex(L, 1);

                    lua_pushnil(L);
                    while (lua_next(L, mt2_idx) != 0) {
                        std::string_view key = luaL_checkstring(L, -2);
                        if (key == "__index" || key == "__newindex" || key == "__gc" || key == "__metatable") {
                            lua_pop(L, 1);
                            // printf("metatype with %s is not allow\n", key.data());
                            continue;
                        } else {
                            lua_pushvalue(L, -2);      // 复制 key
                            lua_insert(L, -2);         // 交换 key 和 value
                            lua_settable(L, mt1_idx);  // mt1[key] = value
                        }
                    }
                    return 0;
                },
                1);
        lua_setfield(L, -2, "metatype");

        lua_setfield(L, -2, fieldName);
    }

    // 创建实例元表
    luaL_newmetatable(L, type_name);

    lua_pushstring(L, type_name);
    lua_pushcclosure(
            L,
            [](lua_State *L) -> int {
                const char *_type_name = lua_tostring(L, lua_upvalueindex(1));
                return LuaStructGC(L, _type_name);
            },
            1);
    lua_setfield(L, -2, "__gc");

    lua_pushboolean(L, 0);
    lua_pushcclosure(L, fieldaccess, 1);
    lua_setfield(L, -2, "__index");

    lua_pushboolean(L, 1);
    lua_pushcclosure(L, fieldaccess, 1);
    lua_setfield(L, -2, "__newindex");

    lua_pop(L, 1);
}

template <typename T, std::size_t I>
int LuaStructField_w(lua_State *L, const char *typeName, const char *field, int set, T &v) {

    int index = 1;

    if constexpr (I < reflection::field_count<T>) {
        constexpr std::string_view name = reflection::field_name<T, I>;
        // lua_getfield(L, arg, name.data());

        if (name == field) {
            // printf("LUASTRUCT_FIELD_W %s.%s\n", typeName, field);
            auto &af = reflection::field_access<I>(v);
            return LuaStructAccess<reflection::field_type<T, I>>::Get(L, field, &af, index, set, index + 2);
        }

        // reflection::field_access<I>(v) = unpack<reflection::field_type<D, I>>(L, -1);
        // lua_pop(L, 1);
        return LuaStructField_w<T, I + 1>(L, typeName, field, set, v);
    }

    return luaL_error(L, "Invalid field %s.%s", typeName, field);
};

template <typename T>
void LuaStruct(lua_State *L, const char *fieldName) {

    auto fieldaccess = [](lua_State *L) -> int {
        int index = 1;
        int set = lua_toboolean(L, lua_upvalueindex(1));

        // printf("fieldaccess %d\n", set);

        const char *typeName = reflection::GetTypeName<T>();
        T *data = LuaStructTodata<T>(L, index);
        size_t length = 0;
        const char *field = LuaStructFieldname(L, index + 1, &length);

        // printf("fieldaccess %s\n", field);

        return LuaStructField_w<T, 0>(L, typeName, field, set, *data);
    };

    LuaStructCreate(L, fieldName, reflection::GetTypeName<T>(), sizeof(T), fieldaccess);

    // lua_setglobal(L, fieldName);

    LuaStructAddType<T>(L, LuaType<T>(L));
}

#define neko_lua_enum_has_value(L, type, value)                \
    const type __neko_lua_enum_value_temp_##value[] = {value}; \
    neko_lua_enum_has_value_type(L, LuaType<type>(L), __neko_lua_enum_value_temp_##value)

template <typename T, typename V>
    requires std::is_same_v<T, LuaTypeid>
inline bool LuaEnumHas(lua_State *L, T type, V v) {
    using VT = std::remove_cv_t<V>;
    if constexpr (std::is_integral_v<VT>) {
        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        if (!lua_isnil(L, -1)) {
            lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
            lua_pushinteger(L, type);
            lua_gettable(L, -2);
            size_t size = lua_tointeger(L, -1);
            lua_pop(L, 2);

            // lua_Integer lvalue = 0;
            // memcpy(&lvalue, v, size);

            lua_pushinteger(L, v);
            lua_gettable(L, -2);

            if (lua_isnil(L, -1)) {
                lua_pop(L, 3);
                return false;
            } else {
                lua_pop(L, 3);
                return true;
            }
        }
        lua_pop(L, 2);
        lua_pushfstring(L, "LuaEnumHas: Enum '%s' not registered!", GetLuaTypeinfo(L, type).name);
        lua_error(L);
        return false;
    } else if constexpr (std::is_same_v<std::decay_t<VT>, char const *>) {
        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, v);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 3);
                return false;
            } else {
                lua_pop(L, 3);
                return true;
            }
        }
        lua_pop(L, 2);
        lua_pushfstring(L, "LuaEnumHas: Enum '%s' not registered!", GetLuaTypeinfo(L, type).name);
        lua_error(L);
        return false;
    } else {
        static_assert(!v, "LuaEnumHas type assert");
    }
}

template <typename T, typename V>
inline bool LuaEnumHas(lua_State *L, V v) {
    return LuaEnumHas(L, LuaType<T>(L), v);
}

inline void LuaEnumAddType(lua_State *L, LuaTypeid type, size_t size) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
    lua_pushinteger(L, type);
    lua_pushinteger(L, size);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

inline void LuaEnumAddValue(lua_State *L, LuaTypeid type, const void *value, const char *name) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        size_t size = lua_tointeger(L, -1);
        lua_pop(L, 2);

        lua_newtable(L);

        lua_Integer lvalue = 0;
        memcpy(&lvalue, value, size);

        lua_pushinteger(L, lvalue);
        lua_setfield(L, -2, "value");

        lua_pushstring(L, name);
        lua_setfield(L, -2, "name");

        lua_setfield(L, -2, name);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        lua_pushinteger(L, lvalue);
        lua_getfield(L, -4, name);
        lua_settable(L, -3);

        lua_pop(L, 4);
        return;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "LuaEnumAddValue: Enum '%s' not registered!", GetLuaTypeinfo(L, type).name);
    lua_error(L);
}

inline bool LuaTypeIsEnum(lua_State *L, LuaTypeid type) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);
    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return reg;
}

template <typename Enum, int min_value = -64, int max_value = 64>
void LuaEnum(lua_State *L) {
    std::map<int, std::string> values;
    reflection::guess_enum_range<Enum, min_value>(values, std::make_integer_sequence<int, max_value - min_value>());
    reflection::guess_enum_bit_range<Enum>(values, std::make_integer_sequence<int, 32>());

    LuaTypeid id = LuaType<Enum>(L);
    LuaEnumAddType(L, id, sizeof(Enum));
    for (const auto &value : values) {
        const Enum enum_value[] = {(Enum)value.first};
        LuaEnumAddValue(L, id, enum_value, value.second.c_str());
    }
}

template <typename T>
inline void LuaPush(lua_State *L, T &&v) {
    detail::LuaStack::Push(L, std::forward<T>(v));
}

template <typename T>
inline T LuaGet(lua_State *L, int index) {
    T t{};
    detail::LuaStack::Get(L, index, t);
    return t;
}

template <typename T>
    requires std::is_enum_v<T>
inline void LuaPush(lua_State *L, const T &v) {
    LuaTypePush(L, LuaType<T>(L), &v);
}

template <typename T>
    requires std::is_enum_v<T>
inline auto LuaGet(lua_State *L, int index) -> T {
    T type_val;
    LuaTypeTo(L, LuaType<T>(L), &type_val, index);
    return type_val;
}

template <typename T, std::size_t I>
void LuaRawStructGet(lua_State *L, int arg, T &v);

// template <typename T>
//     requires std::is_pointer_v<T>
// T LuaGet(lua_State *L, int arg) {
//     luaL_checktype(L, arg, LUA_TLIGHTUSERDATA);
//     return static_cast<T>(lua_touserdata(L, arg));
// }

template <>
inline std::string_view LuaGet<std::string_view>(lua_State *L, int arg) {
    size_t sz = 0;
    const char *str = luaL_checklstring(L, arg, &sz);
    return std::string_view(str, sz);
}

template <template <typename...> class Template, typename Class>
struct is_instantiation : std::false_type {};
template <template <typename...> class Template, typename... Args>
struct is_instantiation<Template, Template<Args...>> : std::true_type {};
template <typename Class, template <typename...> class Template>
concept is_instantiation_of = is_instantiation<Template, Class>::value;

template <typename T>
    requires is_instantiation_of<T, std::map>
T LuaGet(lua_State *L, int arg) {
    arg = lua_absindex(L, arg);
    luaL_checktype(L, arg, LUA_TTABLE);
    T v;
    lua_pushnil(L);
    while (lua_next(L, arg)) {
        auto key = LuaGet<typename T::key_type>(L, -2);
        auto mapped = LuaGet<typename T::mapped_type>(L, -1);
        v.emplace(std::move(key), std::move(mapped));
        lua_pop(L, 1);
    }
    return v;
}

template <typename T>
    requires is_instantiation_of<T, std::vector>
T LuaGet(lua_State *L, int arg) {
    arg = lua_absindex(L, arg);
    luaL_checktype(L, arg, LUA_TTABLE);
    lua_Integer n = luaL_len(L, arg);
    T v;
    v.reserve((size_t)n);
    for (lua_Integer i = 1; i <= n; ++i) {
        lua_geti(L, arg, i);
        auto value = LuaGet<typename T::value_type>(L, -1);
        v.emplace_back(std::move(value));
        lua_pop(L, 1);
    }
    return v;
}

template <typename T, std::size_t I>
void LuaRawStructGet(lua_State *L, int arg, T &v) {
    if constexpr (I < reflection::field_count<T>) {
        constexpr auto name = reflection::field_name<T, I>;
        lua_getfield(L, arg, name.data());
        reflection::field_access<I>(v) = LuaGet<reflection::field_type<T, I>>(L, -1);
        lua_pop(L, 1);
        LuaRawStructGet<T, I + 1>(L, arg, v);
    }
}

template <typename T, std::size_t I>
void LuaRawStructPush(lua_State *L, const T &v);

template <typename T, std::size_t I>
void LuaRawStructPush(lua_State *L, const T &v) {
    if constexpr (I < reflection::field_count<T>) {
        constexpr auto name = reflection::field_name<T, I>;
        using FT = reflection::field_type<T, I>;
        LuaPush(L, reflection::field_access<I>(v));
        lua_setfield(L, -2, name.data());
        LuaRawStructPush<T, I + 1>(L, v);
    }
}

template <typename T>
    requires(std::is_aggregate_v<T>)
inline void LuaPush(lua_State *L, const T &v) {
    bool is_reg_struct = LuaTypeIsStruct(L, LuaType<T>(L));
    if (is_reg_struct) {
        LuaStructPush<T>(L, v);
    } else {  // 未经过注册的聚合类
        assert(!"use LuaPushRaw");
    }
}

template <typename T>
    requires std::is_aggregate_v<T>
void LuaPushRaw(lua_State *L, const T &v) {
    lua_createtable(L, 0, (int)reflection::field_count<T>);
    LuaRawStructPush<T, 0>(L, v);
    return;
}

template <typename T>
    requires std::is_aggregate_v<T>
T LuaGetRaw(lua_State *L, int arg) {
    T v;
    LuaRawStructGet<T, 0>(L, arg, v);
    return v;
}

template <typename T>
    requires(is_struct<T>::value)
inline auto LuaGet(lua_State *L, int index) {
    bool is_reg_struct = LuaTypeIsStruct(L, LuaType<T>(L));
    T *v{};
    if (is_reg_struct) {
        v = LuaStructTodata<T>(L, index);
    } else {  // 未经过注册的聚合类
        assert(!"use LuaGetRaw");
    }
    return v;
}

inline int LuaTypePush(lua_State *L, LuaTypeid type_id, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        neko_luabind_Pushfunc func = (neko_luabind_Pushfunc)lua_touserdata(L, -1);
        lua_pop(L, 2);
        return func(L, type_id, c_in);
    }

    lua_pop(L, 2);

    // if (neko_luabind_struct_registered_type(L, type_id)) {
    //     return neko_luabind_struct_push_type(L, type_id, c_in);
    // }

    if (LuaTypeIsEnum(L, type_id)) {
        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
        lua_pushinteger(L, type_id);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {

            lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
            lua_pushinteger(L, type_id);
            lua_gettable(L, -2);
            size_t size = lua_tointeger(L, -1);
            lua_pop(L, 2);

            lua_Integer lvalue = 0;
            memcpy(&lvalue, c_in, size);

            lua_pushinteger(L, lvalue);
            lua_gettable(L, -2);

            if (!lua_isnil(L, -1)) {
                lua_getfield(L, -1, "name");
                lua_remove(L, -2);
                lua_remove(L, -2);
                lua_remove(L, -2);
                return 1;
            }

            lua_pop(L, 3);
            lua_pushfstring(L, "LuaTypePush: Enum '%s' value %d not registered!", GetLuaTypeinfo(L, type_id).name, lvalue);
            lua_error(L);
            return 0;
        }

        lua_pop(L, 2);
        lua_pushfstring(L, "LuaTypePush: Enum '%s' not registered!", GetLuaTypeinfo(L, type_id).name);
        lua_error(L);
        return 0;
    }

    lua_pushfstring(L, "LuaTypePush: conversion to Lua object from type '%s' not registered!", GetLuaTypeinfo(L, type_id).name);
    lua_error(L);
    return 0;
}

inline void LuaTypeTo(lua_State *L, LuaTypeid type_id, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        neko_luabind_Tofunc func = (neko_luabind_Tofunc)lua_touserdata(L, -1);
        lua_pop(L, 2);
        func(L, type_id, c_out, index);
        return;
    }

    lua_pop(L, 2);

    // if (neko_luabind_struct_registered_type(L, type_id)) {
    //     neko_luabind_struct_to_type(L, type_id, c_out, index);
    //     return;
    // }

    if (LuaTypeIsEnum(L, type_id)) {

        const char *name = lua_tostring(L, index);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
        lua_pushinteger(L, type_id);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {

            lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
            lua_pushinteger(L, type_id);
            lua_gettable(L, -2);
            size_t size = lua_tointeger(L, -1);
            lua_pop(L, 2);

            lua_pushstring(L, name);
            lua_gettable(L, -2);

            if (!lua_isnil(L, -1)) {
                lua_getfield(L, -1, "value");
                lua_Integer value = lua_tointeger(L, -1);
                lua_pop(L, 4);
                memcpy(c_out, &value, size);
                return;
            }

            lua_pop(L, 3);
            lua_pushfstring(L, "LuaTypeTo: Enum '%s' field '%s' not registered!", GetLuaTypeinfo(L, type_id).name, name);
            lua_error(L);
            return;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "LuaTypeTo: Enum '%s' not registered!", GetLuaTypeinfo(L, type_id).name);
        lua_error(L);
        return;
    }

    lua_pushfstring(L, "LuaTypeTo: conversion from Lua object to type '%s' not registered!", GetLuaTypeinfo(L, type_id).name);
    lua_error(L);
}

struct callfunc {
    template <typename F, typename... Args>
    callfunc(F f, Args... args) {
        f(std::forward<Args>(args)...);
    }
};

inline auto &usermodules() {
    static std::vector<luaL_Reg> v;
    return v;
}

inline void register_module(const char *name, lua_CFunction func) { usermodules().emplace_back(luaL_Reg{name, func}); }

inline int preload_module(lua_State *L) {
    luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
    for (const auto &m : usermodules()) {
        lua_pushcfunction(L, m.func);
        lua_setfield(L, -2, m.name);
    }
    lua_pop(L, 1);
    return 0;
}

};  // namespace luabind

}  // namespace Neko

#define DEFINE_LUAOPEN(name)                                                               \
    int luaopen_neko_##name(lua_State *L) { return Neko::luabind::__##name ::luaopen(L); } \
    static ::Neko::luabind::callfunc __init_##name(::Neko::luabind::register_module, "__neko." #name, luaopen_neko_##name);

#define DEFINE_LUAOPEN_EXTERN(name)     \
    namespace Neko::luabind::__##name { \
        int luaopen(lua_State *L);      \
    }                                   \
    DEFINE_LUAOPEN(name)

#endif
