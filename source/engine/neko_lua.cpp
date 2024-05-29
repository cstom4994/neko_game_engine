#include "neko_lua.hpp"

#include <stdarg.h>

#include <iostream>

namespace neko {
lua_State* g_L = NULL;

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
        neko_log_warning("[lua] err: %s", lua_tostring(__lua, -1));
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

    neko_log_warning("[lua] not find method \"%s\" with script_ref_%d", method_name, script_ref);

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
            neko_log_warning("[lua] cannot delete an instance that isn't a ScriptObject*");
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
    vsprintf(buffer, fmt, args);
    va_end(args);

    if (luaL_loadstring(g_L, buffer) != 0) {
        std::string err = lua_tostring(g_L, -1);
        neko_log_warning("[lua] could not evaluate string \"%s\":\n%s", buffer, err.c_str());
        lua_pop(g_L, 1);
    }
}

void lua_bind::loadFile(const_str pathToFile) {
    int res = luaL_loadfile(g_L, pathToFile);
    if (res != 0) {
        std::string err = lua_tostring(g_L, -1);
        neko_log_warning("[lua] could not load file: %s", err.c_str());
        lua_pop(g_L, 1);
    } else {
        res = lua_pcall(g_L, 0, 0, NULL);
        if (res != 0) {
            std::string err = lua_tostring(g_L, -1);
            neko_log_warning("[lua] could not compile supplied script file: %s", err.c_str());
            lua_pop(g_L, 1);
        }
    }
}

// TODO

}  // namespace neko
