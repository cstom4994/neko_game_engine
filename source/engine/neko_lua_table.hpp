
#ifndef NEKO_LUA_REF_HPP
#define NEKO_LUA_REF_HPP

#include <string>
#include <tuple>  // std::ignore

#include "engine/neko_lua.h"
#include "engine/neko_lua_util.h"

namespace neko {

struct LuaNil {};

template <>
struct luabind::LuaStack<LuaNil> {
    static inline void push(lua_State* L, LuaNil const& nil) { lua_pushnil(L); }
};

class LuaRef;

class LuaRefBase {
protected:
    lua_State* L;
    int m_ref;

    struct lua_stack_auto_popper {
        lua_State* L;
        int m_count;
        lua_stack_auto_popper(lua_State* L, int count = 1) : L(L), m_count(count) {}
        ~lua_stack_auto_popper() { lua_pop(L, m_count); }
    };
    struct FromStackIndex {};

    // 不应该直接使用
    LuaRefBase(lua_State* L, FromStackIndex) : L(L) { m_ref = luaL_ref(L, LUA_REGISTRYINDEX); }
    LuaRefBase(lua_State* L, int ref) : L(L), m_ref(ref) {}
    ~LuaRefBase() { luaL_unref(L, LUA_REGISTRYINDEX, m_ref); }

public:
    virtual void push() const { lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); }

    std::string tostring() const {
        lua_getglobal(L, "tostring");
        push();
        lua_call(L, 1, 1);
        const char* str = lua_tostring(L, 1);
        lua_pop(L, 1);
        return std::string(str);
    }

    int type() const {
        int result;
        push();
        result = lua_type(L, -1);
        lua_pop(L, 1);
        return result;
    }

    inline bool isNil() const { return type() == LUA_TNIL; }
    inline bool isNumber() const { return type() == LUA_TNUMBER; }
    inline bool isString() const { return type() == LUA_TSTRING; }
    inline bool isTable() const { return type() == LUA_TTABLE; }
    inline bool isFunction() const { return type() == LUA_TFUNCTION; }
    inline bool isUserdata() const { return type() == LUA_TUSERDATA; }
    inline bool isThread() const { return type() == LUA_TTHREAD; }
    inline bool isLightUserdata() const { return type() == LUA_TLIGHTUSERDATA; }
    inline bool isBool() const { return type() == LUA_TBOOLEAN; }

    template <typename... Args>
    inline LuaRef const operator()(Args... args) const;

    template <typename... Args>
    inline void call(int ret, Args... args) const;

    template <typename T>
    void append(T v) const {
        push();
        size_t len = lua_rawlen(L, -1);
        neko::luabind::LuaStack<T>::push(L, v);
        lua_rawseti(L, -2, ++len);
        lua_pop(L, 1);
    }

    template <typename T>
    T cast() {
        lua_stack_auto_popper p(L);
        push();
        return neko::luabind::LuaStack<T>::get(L, -1);
    }

    template <typename T>
    operator T() {
        return cast<T>();
    }
};

template <typename K>
class LuaTableElement : public LuaRefBase {
    friend class LuaRef;

private:
    K m_key;

public:
    LuaTableElement(lua_State* L, K key) : LuaRefBase(L, FromStackIndex()), m_key(key) {}

    void push() const override {
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        neko::luabind::LuaStack<K>::push(L, (K)m_key);
        lua_gettable(L, -2);
        lua_remove(L, -2);
    }

    // 为该表/键分配一个新值
    template <typename T>
    LuaTableElement& operator=(T v) {
        lua_stack_auto_popper p(L);
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        neko::luabind::LuaStack<K>::push(L, m_key);
        neko::luabind::LuaStack<T>::push(L, v);
        lua_settable(L, -3);
        return *this;
    }

    template <typename NK>
    LuaTableElement<NK> operator[](NK key) const {
        push();
        return LuaTableElement<NK>(L, key);
    }
};

template <typename K>
struct neko::luabind::LuaStack<LuaTableElement<K> > {
    static inline void push(lua_State* L, LuaTableElement<K> const& e) { e.push(); }
};

class LuaRef : public LuaRefBase {
    friend LuaRefBase;

private:
    LuaRef(lua_State* L, FromStackIndex fs) : LuaRefBase(L, fs) {}

public:
    LuaRef(lua_State* L) : LuaRefBase(L, LUA_REFNIL) {}

    LuaRef(lua_State* L, const std::string& global) : LuaRefBase(L, LUA_REFNIL) {
        lua_getglobal(L, global.c_str());
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LuaRef(LuaRef const& other) : LuaRefBase(other.L, LUA_REFNIL) {
        other.push();
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LuaRef(LuaRef&& other) noexcept : LuaRefBase(other.L, other.m_ref) { other.m_ref = LUA_REFNIL; }

    LuaRef& operator=(LuaRef&& other) noexcept {
        if (this == &other) return *this;

        std::swap(L, other.L);
        std::swap(m_ref, other.m_ref);

        return *this;
    }

    LuaRef& operator=(LuaRef const& other) {
        if (this == &other) return *this;
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaRef& operator=(LuaTableElement<K>&& other) noexcept {
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaRef& operator=(LuaTableElement<K> const& other) {
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaTableElement<K> operator[](K key) const {
        push();
        return LuaTableElement<K>(L, key);
    }

    static LuaRef fromStack(lua_State* L, int index = -1) {
        lua_pushvalue(L, index);
        return LuaRef(L, FromStackIndex());
    }

    static LuaRef newTable(lua_State* L) {
        lua_newtable(L);
        return LuaRef(L, FromStackIndex());
    }

    static LuaRef getGlobal(lua_State* L, char const* name) {
        lua_getglobal(L, name);
        return LuaRef(L, FromStackIndex());
    }
};

template <>
struct luabind::LuaStack<LuaRef> {
    static inline void push(lua_State* L, LuaRef const& r) { r.push(); }
};

template <>
inline LuaRef const LuaRefBase::operator()() const {
    push();
    luax_pcall(L, 0, 1);
    return LuaRef(L, FromStackIndex());
}

template <typename... Args>
inline LuaRef const LuaRefBase::operator()(Args... args) const {
    const int n = sizeof...(Args);
    push();
    int dummy[] = {0, ((void)neko::luabind::LuaStack<Args>::push(L, std::forward<Args>(args)), 0)...};
    std::ignore = dummy;
    luax_pcall(L, n, 1);
    return LuaRef(L, FromStackIndex());
}

template <>
inline void LuaRefBase::call(int ret) const {
    push();
    luax_pcall(L, 0, ret);
    return;  // 如果有返回值保留在 Lua 堆栈中
}

template <typename... Args>
inline void LuaRefBase::call(int ret, Args... args) const {
    const int n = sizeof...(Args);
    push();
    int dummy[] = {0, ((void)neko::luabind::LuaStack<Args>::push(L, std::forward<Args>(args)), 0)...};
    std::ignore = dummy;
    luax_pcall(L, n, ret);
    return;  // 如果有返回值保留在 Lua 堆栈中
}

template <>
inline LuaRef LuaRefBase::cast() {
    push();
    return LuaRef(L, FromStackIndex());
}

};  // namespace neko

#endif
