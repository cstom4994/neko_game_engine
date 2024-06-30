#include "neko_lua.hpp"

#include <stdarg.h>

#include <iostream>

#include "engine/neko.hpp"
#include "engine/neko_engine.h"

namespace neko {
lua_State* g_L = NULL;

static size_t lua_mem_usage;

size_t neko_lua_mem_usage() { return lua_mem_usage; }

void* Allocf(void* ud, void* ptr, size_t osize, size_t nsize) {
    if (!ptr) osize = 0;
    if (!nsize) {
        lua_mem_usage -= osize;
        neko_safe_free(ptr);
        return NULL;
    }
    lua_mem_usage += (nsize - osize);
    return neko_safe_realloc(ptr, nsize);
}

void neko_lua_run_string(lua_State* m_ls, const_str str_) {
    if (luaL_dostring(m_ls, str_)) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "run_string ::lua_pcall_wrap failed str<%s>", str_);
        ::lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }
}

namespace {
std::string default_str("");
}

ScriptReference::ScriptReference() : script_ref(0), __lua(g_L) {}

ScriptReference::~ScriptReference() { NEKO_ASSERT(script_ref == 0, "Warning, you have deleted an instance without unregistering it"); }

lua_table::lua_table() : __lua(NULL), script_ref(0) {}

lua_table::lua_table(lua_State* L, int script_ref_) : __lua(L), script_ref(script_ref_) {}

lua_table::lua_table(const lua_table& other) : __lua(other.__lua), script_ref(0) {
    if (other.script_ref != 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, other.script_ref);
        script_ref = luaL_ref(__lua, LUA_REGISTRYINDEX);
    }
}

lua_table::~lua_table() {
    if (script_ref != 0) {
        luaL_unref(__lua, LUA_REGISTRYINDEX, script_ref);
        script_ref = 0;
    }
}

std::string lua_table::getString(const_str key) const {
    if (script_ref == 0 || key == NULL) return default_str;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    if (lua_isstring(__lua, -1)) {
        const_str str = lua_tostring(__lua, -1);
        std::string result(str);
        lua_pop(__lua, 2);
        return result;
    }

    lua_pop(__lua, 2);
    return default_str;
}

double lua_table::getDouble(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0.0;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    double result = 0.0;

    if (lua_isnumber(__lua, -1)) result = lua_tonumber(__lua, -1);

    lua_pop(__lua, 2);
    return result;
}

float lua_table::getFloat(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0.0f;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    float result = 0.0f;

    if (lua_isnumber(__lua, -1)) result = (float)lua_tonumber(__lua, -1);

    lua_pop(__lua, 2);
    return result;
}

bool lua_table::getBool(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0.0f;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    bool result = false;

    if (lua_isboolean(__lua, -1)) result = lua_toboolean(__lua, -1) != 0;

    lua_pop(__lua, 2);
    return result;
}

int lua_table::getInt(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    int result = 0;

    if (lua_isnumber(__lua, -1)) result = lua_tointeger(__lua, -1);

    lua_pop(__lua, 2);
    return result;
}

ScriptObject* lua_table::getPointer(const_str key) const {
    if (script_ref == 0 || key == NULL) return NULL;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    ScriptObject* result = NULL;
    lua_value<ScriptObject*>::pop(__lua, result);
    lua_pop(__lua, 1);
    return result;
}

lua_table lua_table::get_table(const_str key) const {
    if (script_ref == 0 || key == NULL) return lua_table();

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    if (lua_istable(__lua, -1)) {
        lua_table result(__lua, luaL_ref(__lua, LUA_REGISTRYINDEX));
        lua_pop(__lua, 1);
        return result;
    }

    lua_pop(__lua, 2);
    return lua_table(__lua, 0);
}

lua_table& lua_table::operator=(const lua_table& other) {
    if (other.script_ref != 0) {
        __lua = other.__lua;
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, other.script_ref);
        script_ref = luaL_ref(__lua, LUA_REGISTRYINDEX);
    }

    return *this;
}

lua_table_iter lua_table::getIterator() const { return lua_table_iter(__lua, script_ref); }

lua_table_iter::lua_table_iter() : script_ref(0), __lua(NULL), mNumPopsRequired(0) {}

lua_table_iter::lua_table_iter(lua_State* L, int script_ref_) : script_ref(script_ref_), __lua(L), mNumPopsRequired(0) {
    if (script_ref > 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_pushnil(__lua);
        mNumPopsRequired = 2;
    }
}

lua_table_iter::lua_table_iter(const lua_table_iter& it) : script_ref(it.script_ref), __lua(it.__lua), mNumPopsRequired(0) {
    if (script_ref > 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_pushnil(__lua);
        mNumPopsRequired = 2;
    }
}

lua_table_iter::~lua_table_iter() {
    if (script_ref > 0) {
        lua_pop(__lua, mNumPopsRequired);
        script_ref = 0;
    }
}

bool lua_table_iter::hasNext() {
    if (script_ref <= 0) return false;

    if (mNumPopsRequired > 2) lua_pop(__lua, 1);

    bool next = lua_next(__lua, -2) != 0;
    if (next) mNumPopsRequired++;

    return next;
}

std::string lua_table_iter::getKey() const {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    std::string key = lua_tostring(__lua, -2);
    return key;
}

std::string lua_table_iter::getString() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    std::string val = lua_tostring(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

double lua_table_iter::getDouble() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    double val = lua_tonumber(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

float lua_table_iter::getFloat() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    float val = (float)lua_tonumber(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

int lua_table_iter::getInt() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    int val = lua_tointeger(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

bool lua_table_iter::getBool() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    bool val = lua_toboolean(__lua, -1) != 0;
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

ScriptObject* lua_table_iter::getPointer() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    ScriptObject* ptr = NULL;
    lua_value<ScriptObject*>::pop(__lua, ptr);
    mNumPopsRequired--;
    return ptr;
}

lua_table lua_table_iter::get_table() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    lua_table dict;
    lua_value<lua_table>::pop(__lua, dict);
    mNumPopsRequired--;
    return dict;
}

ScriptInvoker::ScriptInvoker() : ScriptReference() {}

ScriptInvoker::~ScriptInvoker() {}

void ScriptInvoker::invoke(const_str method) {
#ifdef _DEBUG
    int top1 = lua_gettop(__lua);
#endif
    if (!findAndPushMethod(method)) return;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    if (lua_pcall(__lua, 1, 0, NULL) != 0) {
        NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
        lua_pop(__lua, 1);
    }

#ifdef _DEBUG
    int currentStack = lua_gettop(__lua);
    NEKO_ASSERT(top1 == currentStack, "The stack after the method call is corrupt");
#endif
}

bool ScriptInvoker::findAndPushMethod(const_str method_name) {
    if (method_name == NULL) return false;

    NEKO_ASSERT(script_ref != 0, "You must register this instance using 'registerObject' before you can invoke any script methods on it");

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    if (lua_istable(__lua, -1)) {
        lua_getfield(__lua, -1, method_name);
        if (lua_isfunction(__lua, -1)) {
            lua_remove(__lua, -2);  // Only the reference to the method exists on the stack after this
            return true;
        }
        lua_pop(__lua, 1);
    }
    lua_pop(__lua, 1);

    NEKO_WARN("[lua] not find method \"%s\" with script_ref_%d", method_name, script_ref);

    return false;
}

bool ScriptInvoker::isMethodDefined(const_str method_name) const {
    if (method_name == NULL) return false;

    NEKO_ASSERT(script_ref != 0, "You must register this instance before you can invoke any script methods on it");

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    if (lua_istable(__lua, -1)) {
        lua_getfield(__lua, -1, method_name);
        if (lua_isfunction(__lua, -1)) {
            lua_pop(__lua, 2);
            return true;
        }
        lua_pop(__lua, 1);
    }
    lua_pop(__lua, 1);
    return false;
}

////////////////////////////////////////////////////////////////

lua_class_define_impl<ScriptObject> ScriptObject::lua_class_defs("ScriptObject", NULL);

lua_class_define* ScriptObject::getClassDef() const { return &lua_class_defs; }

lua_class_define_impl<ScriptObject>* ScriptObject::getStaticClassDef() { return &ScriptObject::lua_class_defs; }

////////////////////////////////////////////////////////////////

ScriptObject::ScriptObject() : ScriptInvoker(), obj_last_entry(NULL) {}

ScriptObject::~ScriptObject() {}

bool ScriptObject::registerObject() {
    NEKO_ASSERT(script_ref == 0, "You are trying to register the same object twice");

    // Get the global lua state
    script_ref = getClassDef()->instantiate(__lua, this);

    if (!onAdd()) {
        unregisterObject();
        return false;
    }

    return true;
}

bool ScriptObject::registerObject(int refId) {
    NEKO_ASSERT(script_ref == 0, "You are trying to register the same object twice");

    script_ref = refId;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, "_instance");
    lua_pushlightuserdata(__lua, this);
    lua_settable(__lua, -3);

    lua_pop(__lua, 1);

    if (!onAdd()) {
        unregisterObject();
        return false;
    }

    return true;
}

void ScriptObject::unregisterObject() {
    NEKO_ASSERT(script_ref != 0, "You are trying to unregister the same object twice");

    onRemove();

    // Set _instance to nil
    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, "_instance");
    lua_pushnil(__lua);
    lua_rawset(__lua, -3);
    lua_pop(__lua, 1);

    luaL_unref(__lua, LUA_REGISTRYINDEX, script_ref);

    script_ref = 0;

    release_pointers();
}

bool ScriptObject::onAdd() {
    invoke("onAdd");
    return true;
}

void ScriptObject::onRemove() { invoke("onRemove", 10); }

void ScriptObject::detach_pointer(ScriptObjectEntry* entry) {
    ScriptObjectEntry* prev = entry->prev;
    ScriptObjectEntry* next = entry->next;

    if (prev != NULL) {
        prev->next = next;
        if (next != NULL) {
            next->prev = prev;
        }

        if (entry == obj_last_entry) {
            obj_last_entry = entry->prev;
        }
    } else {
        if (next != NULL) {
            next->prev = NULL;
        }
    }

    free(entry);
}

ScriptObjectEntry* ScriptObject::attach_pointer(ScriptObject** ptr) {
    if (obj_last_entry == NULL) {
        obj_last_entry = reinterpret_cast<ScriptObjectEntry*>(malloc(sizeof(ScriptObjectEntry)));
        memset(obj_last_entry, 0, sizeof(ScriptObjectEntry));
    } else {
        ScriptObjectEntry* next = reinterpret_cast<ScriptObjectEntry*>(malloc(sizeof(ScriptObjectEntry)));
        memset(next, 0, sizeof(ScriptObjectEntry));
        obj_last_entry->next = next;
        next->prev = obj_last_entry;
        obj_last_entry = next;
    }

    obj_last_entry->ptr = ptr;
    return obj_last_entry;
}

void ScriptObject::release_pointers() {
    if (obj_last_entry == NULL) return;

    ScriptObjectEntry* entry = obj_last_entry;
    while (entry != NULL) {
        ScriptObjectEntry* prev = entry->prev;

        ScriptObject** ptr = entry->ptr;
        *ptr = NULL;
        free(entry);

        entry = prev;
    }

    obj_last_entry = NULL;
}

int lua_delete(lua_State* L) {
    int numobjects = lua_gettop(L);
    for (int i = 0; i < numobjects; ++i) {
        ScriptObject* self;
        if (lua_value<ScriptObject*>::pop(L, self)) {
            self->unregisterObject();
            delete self;
        } else {
            NEKO_WARN("[lua] cannot delete an instance that isn't a ScriptObject*");
        }
    }

    return 0;
}

void lua_bind::initialize(lua_State* L) {
    g_L = L;
    // 确保lua中有delete功能
    bind("delete", lua_delete);
    // 注册脚本对象
    bind<ScriptObject>();
}

lua_State* lua_bind::getLuaState() { return g_L; }

void lua_bind::evaluatef(const_str fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 1024, fmt, args);
    va_end(args);

    if (luaL_loadstring(g_L, buffer) != 0) {
        std::string err = lua_tostring(g_L, -1);
        NEKO_WARN("[lua] could not evaluate string \"%s\":\n%s", buffer, err.c_str());
        lua_pop(g_L, 1);
    }
}

void lua_bind::loadFile(const_str pathToFile) {
    int res = luaL_loadfile(g_L, pathToFile);
    if (res != 0) {
        std::string err = lua_tostring(g_L, -1);
        NEKO_WARN("[lua] could not load file: %s", err.c_str());
        lua_pop(g_L, 1);
    } else {
        res = lua_pcall(g_L, 0, 0, NULL);
        if (res != 0) {
            std::string err = lua_tostring(g_L, -1);
            NEKO_WARN("[lua] could not compile supplied script file: %s", err.c_str());
            lua_pop(g_L, 1);
        }
    }
}

namespace luavalue {
void set(lua_State* L, int idx, value& v) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            v.emplace<std::monostate>();
            break;
        case LUA_TBOOLEAN:
            v.emplace<bool>(!!lua_toboolean(L, idx));
            break;
        case LUA_TLIGHTUSERDATA:
            v.emplace<void*>(lua_touserdata(L, idx));
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
            const char* str = lua_tolstring(L, idx, &sz);
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

void set(lua_State* L, int idx, table& t) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        size_t sz = 0;
        const char* str = luaL_checklstring(L, -2, &sz);
        std::pair<std::string, value> pair;
        pair.first.assign(str, sz);
        set(L, -1, pair.second);
        t.emplace(pair);
        lua_pop(L, 1);
    }
}

void get(lua_State* L, const value& v) {
    std::visit(
            [=](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    lua_pushnil(L);
                } else if constexpr (std::is_same_v<T, bool>) {
                    lua_pushboolean(L, arg);
                } else if constexpr (std::is_same_v<T, void*>) {
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

void get(lua_State* L, const table& t) {
    lua_createtable(L, 0, static_cast<int>(t.size()));
    for (const auto& [k, v] : t) {
        lua_pushlstring(L, k.data(), k.size());
        get(L, v);
        lua_rawset(L, -3);
    }
}
}  // namespace luavalue

int vfs_lua_loader(lua_State* L) {
    const_str name = luaL_checkstring(L, 1);
    std::string path = name;
    std::replace(path.begin(), path.end(), '.', '/');

    // neko_println("fuck:%s", path.c_str());

    bool ok = false;
    auto load_list = {"source/lua/game/", "source/lua/libs/"};
    for (auto p : load_list) {
        std::string load_path = p + path + ".lua";
        neko::string contents = {};
        ok = vfs_read_entire_file(NEKO_PACK_LUACODE, &contents, load_path.c_str());
        if (ok) {
            neko_defer(neko_safe_free(contents.data));
            if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {
                lua_pushfstring(L, "[lua] error loading module \"%s\"", name);
                lua_pop(L, 1);
            } else {
                NEKO_TRACE("[lua] loaded : \"%s\"", path.c_str());
            }
            return 1;
        }
    }

#if 0
    u8* data;
    u32 data_size;

    int result = neko_pack_item_data(&ENGINE_INTERFACE()->pack, path.c_str(), (const u8**)&data, &data_size);
    if (result == 0) {
        if (luaL_loadbuffer(L, (char*)data, data_size, name) != LUA_OK) {
            lua_pop(L, 1);
        } else {
            neko_println("loaded:%s", path.c_str());
        }

        neko_pack_item_free(&ENGINE_INTERFACE()->pack, data);
    }
#endif

    lua_pushfstring(L, "[lua] module \"%s\" not found", name);
    return 1;
}

}  // namespace neko
