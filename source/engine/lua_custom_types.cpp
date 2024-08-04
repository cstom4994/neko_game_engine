#include "lua_custom_types.hpp"

#include <sstream>
#include <typeindex>

// #include "lua_util.hpp"

std::unordered_map<std::type_index, std::string> usertypeNames;

int userdata_destructor(lua_State* L) {
    if (auto obj = touserdata<Userdata>(L, 1)) {
        obj->~Userdata();
    }
    return 0;
}

Bytearray::Bytearray(size_t capacity) : buffer(capacity) { buffer.resize(capacity); }

Bytearray::Bytearray(std::vector<u8> buffer) : buffer(std::move(buffer)) {}

Bytearray::~Bytearray() {}

static int l_bytearray_append(lua_State* L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        auto value = lua_tointeger(L, 2);
        buffer->data().push_back(static_cast<u8>(value));
    }
    return 0;
}

static int l_bytearray_insert(lua_State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    auto value = lua_tointeger(L, 3);
    data.insert(data.begin() + index, static_cast<u8>(value));
    return 0;
}

static int l_bytearray_remove(lua_State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    data.erase(data.begin() + index);
    return 0;
}

static std::unordered_map<std::string, lua_CFunction> bytearray_methods{
        {"append", lua::wrap<l_bytearray_append>},
        {"insert", lua::wrap<l_bytearray_insert>},
        {"remove", lua::wrap<l_bytearray_remove>},
};

static int l_bytearray_meta_meta_call(lua_State* L) {
    if (lua_istable(L, 2)) {
        size_t len = lua_objlen(L, 2);
        std::vector<u8> buffer(len);
        buffer.resize(len);
        for (size_t i = 0; i < len; i++) {
            lua_rawgeti(L, -1, i + 1);
            buffer[i] = static_cast<u8>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        return newuserdata<Bytearray>(L, std::move(buffer));
    }
    auto size = lua_tointeger(L, 2);
    if (size < 0) {
        throw std::runtime_error("size can not be less than 0");
    }
    return newuserdata<Bytearray>(L, static_cast<size_t>(size));
}

static int l_bytearray_meta_index(lua_State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    if (lua_isstring(L, 2)) {
        auto found = bytearray_methods.find(lua_tostring(L, 2));
        if (found != bytearray_methods.end()) {
            lua_pushcfunction(L, found->second);
            return 1;
        }
    }
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    lua_pushinteger(L, data[index]);
    return 1;
}

static int l_bytearray_meta_newindex(lua_State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    auto index = static_cast<size_t>(lua_tointeger(L, 2) - 1);
    auto value = lua_tointeger(L, 3);
    if (index >= data.size()) {
        if (index == data.size()) {
            data.push_back(static_cast<u8>(value));
        }
        return 0;
    }
    data[index] = static_cast<u8>(value);
    return 0;
}

static int l_bytearray_meta_len(lua_State* L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        lua_pushinteger(L, buffer->data().size());
        return 1;
    }
    return 0;
}

static int l_bytearray_meta_tostring(lua_State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    if (data.size() > 512) {
        lua_pushstring(L, std::string("bytearray[" + std::to_string(data.size()) + "]{...}").c_str());
        return 1;
    } else {
        std::stringstream ss;
        ss << "bytearray[" << std::to_string(data.size()) << "]{";
        for (size_t i = 0; i < data.size(); i++) {
            if (i > 0) {
                ss << " ";
            }
            ss << static_cast<unsigned int>(data[i]);
        }
        ss << "}";
        lua_pushstring(L, ss.str().c_str());
        return 1;
    }
}

static int l_bytearray_meta_add(lua_State* L) {
    auto bufferA = touserdata<Bytearray>(L, 1);
    auto bufferB = touserdata<Bytearray>(L, 2);
    if (bufferA == nullptr || bufferB == nullptr) {
        return 0;
    }
    auto& dataA = bufferA->data();
    auto& dataB = bufferB->data();

    std::vector<u8> ab;
    ab.reserve(dataA.size() + dataB.size());
    ab.insert(ab.end(), dataA.begin(), dataA.end());
    ab.insert(ab.end(), dataB.begin(), dataB.end());
    return newuserdata<Bytearray>(L, std::move(ab));
}

int Bytearray::createMetatable(lua_State* L) {
    lua_createtable(L, 0, 6);
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_index>);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_newindex>);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_len>);
    lua_setfield(L, -2, "__len");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_tostring>);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_add>);
    lua_setfield(L, -2, "__add");

    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_meta_call>);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    return 1;
}
