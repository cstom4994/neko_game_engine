#include "lua_custom_types.h"

#include <sstream>
#include <typeindex>

// #include "lua_util.hpp"

std::unordered_map<std::type_index, std::string> usertypeNames;

int userdata_destructor(lua_State *L) {
    if (auto obj = touserdata<Userdata>(L, 1)) {
        obj->~Userdata();
    }
    return 0;
}

Bytearray::Bytearray(size_t capacity) : buffer(capacity) { buffer.resize(capacity); }

Bytearray::Bytearray(std::vector<u8> buffer) : buffer(std::move(buffer)) {}

Bytearray::~Bytearray() {}

static int l_bytearray_append(lua_State *L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        auto value = lua_tointeger(L, 2);
        buffer->data().push_back(static_cast<u8>(value));
    }
    return 0;
}

static int l_bytearray_insert(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    auto value = lua_tointeger(L, 3);
    data.insert(data.begin() + index, static_cast<u8>(value));
    return 0;
}

static int l_bytearray_remove(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
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

static int l_bytearray_meta_meta_call(lua_State *L) {
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

static int l_bytearray_meta_index(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
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

static int l_bytearray_meta_newindex(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
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

static int l_bytearray_meta_len(lua_State *L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        lua_pushinteger(L, buffer->data().size());
        return 1;
    }
    return 0;
}

static int l_bytearray_meta_tostring(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
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

static int l_bytearray_meta_add(lua_State *L) {
    auto bufferA = touserdata<Bytearray>(L, 1);
    auto bufferB = touserdata<Bytearray>(L, 2);
    if (bufferA == nullptr || bufferB == nullptr) {
        return 0;
    }
    auto &dataA = bufferA->data();
    auto &dataB = bufferB->data();

    std::vector<u8> ab;
    ab.reserve(dataA.size() + dataB.size());
    ab.insert(ab.end(), dataA.begin(), dataA.end());
    ab.insert(ab.end(), dataB.begin(), dataB.end());
    return newuserdata<Bytearray>(L, std::move(ab));
}

int Bytearray::createMetatable(lua_State *L) {
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

void LuaVector::RegisterMetaTable(lua_State *L) {
    if (luaL_newmetatable(L, LUA_TYPE_NAME)) {
        lua_pushcfunction(L, New);
        lua_setfield(L, 1, "new");

        lua_pushcfunction(L, GarbageCollect);
        lua_setfield(L, 1, "__gc");

        lua_pushcfunction(L, ToString);
        lua_setfield(L, 1, "__tostring");

        lua_pushcfunction(L, Index);
        lua_setfield(L, 1, "__index");

        lua_pushcfunction(L, NewIndex);
        lua_setfield(L, 1, "__newindex");

        lua_pushcfunction(L, Len);
        lua_setfield(L, 1, "__len");

        lua_pushcfunction(L, Append);
        lua_setfield(L, 1, "append");

        lua_pushcfunction(L, Pop);
        lua_setfield(L, 1, "pop");

        lua_pushcfunction(L, Extend);
        lua_setfield(L, 1, "extend");

        lua_pushcfunction(L, Insert);
        lua_setfield(L, 1, "insert");

        lua_pushcfunction(L, Erase);
        lua_setfield(L, 1, "erase");

        lua_pushcfunction(L, Sort);
        lua_setfield(L, 1, "sort");

        lua_setglobal(L, LUA_TYPE_NAME);
    }
}

LuaVector *LuaVector::CheckArg(lua_State *L, int arg) { return static_cast<LuaVector *>(luaL_checkudata(L, arg, LUA_TYPE_NAME)); }

int LuaVector::New(lua_State *L) {
    auto p = static_cast<LuaVector *>(lua_newuserdata(L, sizeof(LuaVector)));
    new (p) LuaVector;
    luaL_setmetatable(L, LUA_TYPE_NAME);
    return 1;
}

int LuaVector::GarbageCollect(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (p) {
        p->~LuaVector();
    }
    return 0;
}

int LuaVector::ToString(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    std::string str = "vector{";
    for (int i = 0; i < p->size(); i++) {
        if (i != 0) {
            str += ", ";
        }
        str += std::to_string(p->at(i));
    }
    str += "}";

    lua_pushstring(L, str.c_str());
    return 1;
}

int LuaVector::Index(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    switch (lua_type(L, 2)) {
        case LUA_TNUMBER: {
            lua_Integer idx = lua_tointeger(L, 2) - 1;  // convert lua indices to c indices
            if (0 <= idx && idx < p->size()) {
                lua_pushnumber(L, p->at(idx));
                return 1;
            }
            lua_pushnil(L);
            return 1;
        }
        case LUA_TSTRING: {
            const char *meta = lua_tostring(L, 2);
            luaL_getmetafield(L, 1, meta);
            return 1;
        }
        default: {
            lua_pushnil(L);
            return 1;
        }
    }
}

int LuaVector::NewIndex(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Integer idx = luaL_checkinteger(L, 2) - 1;  // convert lua indices to c indices
    lua_Number value = luaL_checknumber(L, 3);

    if (idx < 0) {
        return 0;
    }

    if (idx >= p->size()) {
        p->resize(idx + 1);
    }

    p->at(idx) = static_cast<float>(value);
    return 0;
}

int LuaVector::Len(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_pushinteger(L, static_cast<lua_Integer>(p->size()));
    return 1;
}

int LuaVector::Append(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Number value = luaL_checknumber(L, 2);
    p->push_back(static_cast<float>(value));

    return 0;
}

int LuaVector::Pop(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    if (p->empty()) {
        lua_pushnil(L);
    } else {
        float back = p->back();
        p->pop_back();
        lua_pushnumber(L, back);
    }

    return 1;
}

int LuaVector::Extend(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    const int numParams = lua_gettop(L);
    p->reserve(p->size() + numParams - 1);
    for (int i = 2; i <= numParams; i++) {
        lua_Number value = luaL_checknumber(L, i);
        p->push_back(static_cast<float>(value));
    }

    return 0;
}

int LuaVector::Insert(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Integer idx = luaL_checkinteger(L, 2) - 1;  // convert lua indices to c indices
    lua_Number value = luaL_checknumber(L, 3);

    p->insert(std::next(p->begin(), idx), static_cast<float>(value));

    return 0;
}

int LuaVector::Erase(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Integer idx = luaL_checkinteger(L, 2) - 1;  // convert lua indices to c indices

    p->erase(std::next(p->begin(), idx));

    return 0;
}

int LuaVector::Sort(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    std::sort(p->begin(), p->end());

    return 0;
}

int Test(lua_State *L) {
    LuaVector *v = LuaVector::CheckArg(L, 1);
    v->push_back(1);
    return 0;
}

int test_luavector() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    LuaVector::RegisterMetaTable(L);

    lua_register(L, "test", Test);

    const char *TEST_CODE = R"LUA(
local a = vector.new()
print(a)
print(type(a))
print(a[1])
a[1] = 3.14
print(a)
print(a[1], a[2])
print(#a)
a[2] = 6.28
print(a)
print(a[1], a[2])
print(#a)
for i, v in ipairs(a) do
    print(i, v)
end
a:append(9.42)
print(a)
a:extend(1, 2, 3)
print(a)
a:insert(2, 0.1234)
print(a)
a:sort()
print(a)
a:erase(2)
print(a)
while #a > 0 do
    local b = a:pop()
    print(a, b)
end
test(a)
print(a)
)LUA";

    if (luaL_dostring(L, TEST_CODE) != LUA_OK) {
        fprintf(stderr, "lua error: %s\n", lua_tostring(L, -1));
    }

    lua_close(L);
    return 0;
}

#define entityCacheSize sizeof(EntityCacheEntry)
typedef struct EntityCacheEntry {
    int entityId;
    int nuid;
    const char *ownerGuid;
    const char *ownerName;
    const char *filepath;
    double x;
    double y;
    double rotation;
    double velX;
    double velY;
    double currentHealth;
    double maxHealth;
} EntityCacheEntry;

EntityCacheEntry *entityEntries;
int entityCurrentSize = 0;
static int l_entityCacheSize(lua_State *L) {
    lua_pushinteger(L, entityCurrentSize);
    return 1;
}
static int l_entityCacheUsage(lua_State *L) {
    lua_pushnumber(L, entityCurrentSize * entityCacheSize);
    return 1;
}
static int l_entityCacheWrite(lua_State *L) {
    int entityId = luaL_checkinteger(L, 1);
    int nuid = lua_tointeger(L, 2);
    if (nuid == NULL) {
        nuid = -1;
    };
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->entityId == entityId) {
            entry->entityId = entityId;
            entry->nuid = nuid;
            entry->ownerGuid = luaL_checkstring(L, 3);
            entry->ownerName = luaL_checkstring(L, 4);
            entry->filepath = luaL_checkstring(L, 5);
            entry->x = luaL_checknumber(L, 6);
            entry->y = luaL_checknumber(L, 7);
            entry->rotation = luaL_checknumber(L, 8);
            entry->velX = luaL_checknumber(L, 9);
            entry->velY = luaL_checknumber(L, 10);
            entry->currentHealth = luaL_checknumber(L, 11);
            entry->maxHealth = luaL_checknumber(L, 12);
            return 0;
        };
    }
    ++entityCurrentSize;
    entityEntries = (EntityCacheEntry *)mem_realloc(entityEntries, entityCacheSize * entityCurrentSize);
    EntityCacheEntry *newEntry = entityEntries + (entityCurrentSize - 1);
    newEntry->entityId = entityId;
    newEntry->nuid = nuid;
    newEntry->ownerGuid = luaL_checkstring(L, 3);
    newEntry->ownerName = luaL_checkstring(L, 4);
    newEntry->filepath = luaL_checkstring(L, 5);
    newEntry->x = luaL_checknumber(L, 6);
    newEntry->y = luaL_checknumber(L, 7);
    newEntry->rotation = luaL_checknumber(L, 8);
    newEntry->velX = luaL_checknumber(L, 9);
    newEntry->velY = luaL_checknumber(L, 10);
    newEntry->currentHealth = luaL_checknumber(L, 11);
    newEntry->maxHealth = luaL_checknumber(L, 12);
    return 0;
}

static void l_createEntityCacheReturnTable(lua_State *L, EntityCacheEntry *entry) {
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, entry->entityId);
    lua_setfield(L, -2, "entityId");
    if (entry->nuid == -1) {
        lua_pushnil(L);
        lua_setfield(L, -2, "nuid");
    } else {
        lua_pushinteger(L, entry->nuid);
        lua_setfield(L, -2, "nuid");
    }
    lua_pushstring(L, entry->ownerGuid);
    lua_setfield(L, -2, "ownerGuid");

    lua_pushstring(L, entry->ownerName);
    lua_setfield(L, -2, "ownerName");

    lua_pushstring(L, entry->filepath);
    lua_setfield(L, -2, "filepath");

    lua_pushnumber(L, entry->x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, entry->y);
    lua_setfield(L, -2, "y");

    lua_pushnumber(L, entry->velX);
    lua_setfield(L, -2, "velX");

    lua_pushnumber(L, entry->velY);
    lua_setfield(L, -2, "velY");

    lua_pushnumber(L, entry->rotation);
    lua_setfield(L, -2, "rotation");

    lua_pushnumber(L, entry->currentHealth);
    lua_setfield(L, -2, "currentHealth");

    lua_pushnumber(L, entry->maxHealth);
    lua_setfield(L, -2, "maxHealth");
}
static int l_entityCacheReadByEntityId(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->entityId == idToSearch) {
            l_createEntityCacheReturnTable(L, entry);
            return 1;
        };
    }
    lua_pushnil(L);
    return 1;
}

static int l_entityCacheReadByNuid(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->nuid == idToSearch) {
            l_createEntityCacheReturnTable(L, entry);
            return 1;
        };
    }
    lua_pushnil(L);
    return 1;
}

static int l_entityCacheDeleteByEntityId(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->entityId == idToSearch) {
            memmove(entityEntries + i + 1, entityEntries + i, ((entityCurrentSize - 1) - i) * entityCacheSize);
            entityCurrentSize--;
            entityEntries = (EntityCacheEntry *)mem_realloc(entityEntries, entityCacheSize * entityCurrentSize);
            lua_pushboolean(L, 1);
            return 1;
        };
    }
    lua_pushboolean(L, 0);
    return 1;
}

static int l_entityCacheDeleteByNuid(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->nuid == idToSearch) {
            memmove(entityEntries + i + 1, entityEntries + i, ((entityCurrentSize - 1) - i) * entityCacheSize);
            entityCurrentSize--;
            entityEntries = (EntityCacheEntry *)mem_realloc(entityEntries, entityCacheSize * entityCurrentSize);
            lua_pushboolean(L, 1);
            return 1;
        };
    }
    lua_pushboolean(L, 0);
    return 1;
}

int luaopen_luaExtensions(lua_State *L) {
    static const luaL_Reg eCachelib[] = {{"set", l_entityCacheWrite},
                                         {"get", l_entityCacheReadByEntityId},
                                         {"getNuid", l_entityCacheReadByNuid},
                                         {"delete", l_entityCacheDeleteByEntityId},
                                         {"deleteNuid", l_entityCacheDeleteByNuid},
                                         {"size", l_entityCacheSize},
                                         {"usage", l_entityCacheUsage},
                                         {NULL, NULL}};
    // luaL_openlib(L, "EntityCache", eCachelib, 0);
    return 1;
}
