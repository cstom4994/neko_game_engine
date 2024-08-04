#ifndef LOGIC_SCRIPTING_LUA_LUA_CUSTOM_TYPES_HPP_
#define LOGIC_SCRIPTING_LUA_LUA_CUSTOM_TYPES_HPP_

#include <string>
#include <typeindex>
#include <vector>

#include "engine/luax.h"

extern std::unordered_map<std::type_index, std::string> usertypeNames;
int userdata_destructor(lua_State* L);

class Userdata {
public:
    virtual ~Userdata(){};
    virtual const std::string& getTypeName() const = 0;
};

class Bytearray : public Userdata {
    std::vector<u8> buffer;

public:
    Bytearray(size_t capacity);
    Bytearray(std::vector<u8> buffer);
    virtual ~Bytearray();

    const std::string& getTypeName() const override { return TYPENAME; }
    inline std::vector<u8>& data() { return buffer; }

    static int createMetatable(lua_State*);
    inline static std::string TYPENAME = "bytearray";
};

namespace lua {
template <lua_CFunction func>
int wrap(lua_State* L) {
    int result = 0;
    try {
        result = func(L);
    }
    // transform exception with description into lua_error
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    // Rethrow any other exception (lua error for example)
    catch (...) {
        throw;
    }
    return result;
}
}  // namespace lua

inline bool getglobal(lua_State* L, const std::string& name) {
    lua_getglobal(L, name.c_str());
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

template <class T>
inline T* touserdata(lua_State* L, int idx) {
    if (void* rawptr = lua_touserdata(L, idx)) {
        return static_cast<T*>(rawptr);
    }
    return nullptr;
}
template <class T, typename... Args>
inline int newuserdata(lua_State* L, Args&&... args) {
    const auto& found = usertypeNames.find(typeid(T));
    void* ptr = lua_newuserdata(L, sizeof(T));
    new (ptr) T(args...);

    if (found == usertypeNames.end()) {
        NEKO_ERROR(std::string("usertype is not registred: " + std::string(typeid(T).name())).c_str());
    } else if (getglobal(L, found->second)) {
        lua_setmetatable(L, -2);
    }
    return 1;
}

template <class T, lua_CFunction func>
inline void newusertype(lua_State* L, const std::string& name) {
    usertypeNames[typeid(T)] = name;
    func(L);

    lua_pushcfunction(L, userdata_destructor);
    lua_setfield(L, -2, "__gc");

    lua_setglobal(L, name.c_str());
}

#endif  // LOGIC_SCRIPTING_LUA_LUA_CUSTOM_TYPES_HPP_
