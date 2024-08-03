#include "neko_lua.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>

#include "neko_api.hpp"
#include "neko_app.h"
#include "neko_asset.h"
#include "neko_base.h"
#include "neko_lua_struct.h"
#include "neko_lua_wrap.h"
#include "neko_os.h"
#include "neko_prelude.h"

#if LUA_VERSION_NUM < 504

void *lua_newuserdatauv(lua_State *L_, size_t sz_, [[maybe_unused]] int nuvalue_) {
    NEKO_ASSERT(L_, nuvalue_ <= 1);
    return lua_newuserdata(L_, sz_);
}

int lua_getiuservalue(lua_State *const L_, int const idx_, int const n_) {
    if (n_ > 1) {
        lua_pushnil(L_);
        return LUA_TNONE;
    }

#if LUA_VERSION_NUM == 501
    lua_getfenv(L_, idx_);
    lua_getglobal(L_, LUA_LOADLIBNAME);
    if (lua_rawequal(L_, -2, -1) || lua_rawequal(L_, -2, LUA_GLOBALSINDEX)) {
        lua_pop(L_, 2);
        lua_pushnil(L_);

        return LUA_TNONE;
    } else {
        lua_pop(L_, 1);
    }
#else
    lua_getuservalue(L_, idx_);
#endif

    int const _uvType{lua_type(L_, -1)};

    return (LUA_VERSION_NUM == 502 && _uvType == LUA_TNIL) ? LUA_TNONE : _uvType;
}

int lua_setiuservalue(lua_State *L_, int idx_, int n_) {
    if (n_ > 1
#if LUA_VERSION_NUM == 501
        || lua_type(L_, -1) != LUA_TTABLE
#endif
    ) {
        lua_pop(L_, 1);
        return 0;
    }

#if LUA_VERSION_NUM == 501
    lua_setfenv(L_, idx_);
#else
    lua_setuservalue(L_, idx_);
#endif
    return 1;
}

#endif

void __neko_luabind_init(lua_State *L) {

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
}

void __neko_luabind_fini(lua_State *L) {

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
}

bool neko_lua_equal(lua_State *state, int index1, int index2) {
#if LUA_VERSION_NUM <= 501
    return lua_equal(state, index1, index2) == 1;
#else
    return lua_compare(state, index1, index2, LUA_OPEQ) == 1;
#endif
}

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
    return 0;
}

int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name) {
    neko_lua_preload(L, f, name);
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    lua_call(L, 1, 0);
    return 0;
}

void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name) {
    lua_getglobal(L, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, l, 0);
    lua_setglobal(L, name);
}

void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name) {
    lua_newtable(L);
    luaL_setfuncs(L, l, 0);
    lua_setglobal(L, name);
}

int neko_lua_get_table_pairs_count(lua_State *L, int index) {
    int count = 0;
    index = lua_absindex(L, index);
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        // 现在栈顶是value，下一个栈元素是key
        count++;
        lua_pop(L, 1);  // 移除value，保留key作为下一次迭代的key
    }
    return count;
}

namespace neko {

static size_t lua_mem_usage;

size_t neko_lua_mem_usage() { return lua_mem_usage; }

void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize) {
    if (!ptr) osize = 0;
    if (!nsize) {
        lua_mem_usage -= osize;
        mem_free(ptr);
        return NULL;
    }
    lua_mem_usage += (nsize - osize);
    return mem_realloc(ptr, nsize);
}

void neko_lua_run_string(lua_State *m_ls, const_str str_) {
    if (luaL_dostring(m_ls, str_)) {
        std::string err = lua_tool::dump_error(m_ls, "run_string ::lua_pcall_wrap failed str<%s>", str_);
        ::lua_pop(m_ls, 1);
        // NEKO_ERROR("%s", err.c_str());
    }
}

namespace {
std::string default_str("");
}

ScriptReference::ScriptReference() : script_ref(0) {}

ScriptReference::~ScriptReference() { NEKO_ASSERT(script_ref == 0, "Warning, you have deleted an instance without unregistering it"); }

lua_table::lua_table() : __lua(NULL), script_ref(0) {}

lua_table::lua_table(lua_State *L, int script_ref_) : __lua(L), script_ref(script_ref_) {}

lua_table::lua_table(const lua_table &other) : __lua(other.__lua), script_ref(0) {
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

ScriptObject *lua_table::getPointer(const_str key) const {
    if (script_ref == 0 || key == NULL) return NULL;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    ScriptObject *result = NULL;
    lua_value<ScriptObject *>::pop(__lua, result);
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

lua_table &lua_table::operator=(const lua_table &other) {
    if (other.script_ref != 0) {
        __lua = other.__lua;
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, other.script_ref);
        script_ref = luaL_ref(__lua, LUA_REGISTRYINDEX);
    }

    return *this;
}

lua_table_iter lua_table::getIterator() const { return lua_table_iter(__lua, script_ref); }

lua_table_iter::lua_table_iter() : script_ref(0), __lua(NULL), mNumPopsRequired(0) {}

lua_table_iter::lua_table_iter(lua_State *L, int script_ref_) : script_ref(script_ref_), __lua(L), mNumPopsRequired(0) {
    if (script_ref > 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_pushnil(__lua);
        mNumPopsRequired = 2;
    }
}

lua_table_iter::lua_table_iter(const lua_table_iter &it) : script_ref(it.script_ref), __lua(it.__lua), mNumPopsRequired(0) {
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

ScriptObject *lua_table_iter::getPointer() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    ScriptObject *ptr = NULL;
    lua_value<ScriptObject *>::pop(__lua, ptr);
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

lua_State *ScriptReference::__lua() const {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);
    return L;
}

ScriptInvoker::ScriptInvoker() : ScriptReference() {}

ScriptInvoker::~ScriptInvoker() {}

void ScriptInvoker::invoke(const_str method) {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

#ifdef _DEBUG
    int top1 = lua_gettop(L);
#endif
    if (!findAndPushMethod(method)) return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    if (lua_pcall(L, 1, 0, NULL) != 0) {
        NEKO_WARN("[lua] err: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

#ifdef _DEBUG
    int currentStack = lua_gettop(L);
    NEKO_ASSERT(top1 == currentStack, "The stack after the method call is corrupt");
#endif
}

bool ScriptInvoker::findAndPushMethod(const_str method_name) {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    if (method_name == NULL) return false;

    NEKO_ASSERT(script_ref != 0, "You must register this instance using 'registerObject' before you can invoke any script methods on it");

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, method_name);
        if (lua_isfunction(L, -1)) {
            lua_remove(L, -2);  // Only the reference to the method exists on the stack after this
            return true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    NEKO_WARN("[lua] not find method \"%s\" with script_ref_%d", method_name, script_ref);

    return false;
}

bool ScriptInvoker::isMethodDefined(const_str method_name) const {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    if (method_name == NULL) return false;

    NEKO_ASSERT(script_ref != 0, "You must register this instance before you can invoke any script methods on it");

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, method_name);
        if (lua_isfunction(L, -1)) {
            lua_pop(L, 2);
            return true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return false;
}

////////////////////////////////////////////////////////////////

lua_class_define_impl<ScriptObject> ScriptObject::lua_class_defs("ScriptObject", NULL);

lua_class_define *ScriptObject::getClassDef() const { return &lua_class_defs; }

lua_class_define_impl<ScriptObject> *ScriptObject::getStaticClassDef() { return &ScriptObject::lua_class_defs; }

////////////////////////////////////////////////////////////////

ScriptObject::ScriptObject() : ScriptInvoker(), obj_last_entry(NULL) {}

ScriptObject::~ScriptObject() {}

bool ScriptObject::registerObject() {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    NEKO_ASSERT(script_ref == 0, "You are trying to register the same object twice");

    // Get the global lua state
    script_ref = getClassDef()->instantiate(L, this);

    if (!onAdd()) {
        unregisterObject();
        return false;
    }

    return true;
}

bool ScriptObject::registerObject(int refId) {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    NEKO_ASSERT(script_ref == 0, "You are trying to register the same object twice");

    script_ref = refId;

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(L, "_instance");
    lua_pushlightuserdata(L, this);
    lua_settable(L, -3);

    lua_pop(L, 1);

    if (!onAdd()) {
        unregisterObject();
        return false;
    }

    return true;
}

void ScriptObject::unregisterObject() {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    NEKO_ASSERT(script_ref != 0, "You are trying to unregister the same object twice");

    onRemove();

    // Set _instance to nil
    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(L, "_instance");
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, script_ref);

    script_ref = 0;

    release_pointers();
}

bool ScriptObject::onAdd() {
    invoke("onAdd");
    return true;
}

void ScriptObject::onRemove() { invoke("onRemove", 10); }

void ScriptObject::detach_pointer(ScriptObjectEntry *entry) {
    ScriptObjectEntry *prev = entry->prev;
    ScriptObjectEntry *next = entry->next;

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

    mem_free(entry);
}

ScriptObjectEntry *ScriptObject::attach_pointer(ScriptObject **ptr) {
    if (obj_last_entry == NULL) {
        obj_last_entry = reinterpret_cast<ScriptObjectEntry *>(mem_alloc(sizeof(ScriptObjectEntry)));
        memset(obj_last_entry, 0, sizeof(ScriptObjectEntry));
    } else {
        ScriptObjectEntry *next = reinterpret_cast<ScriptObjectEntry *>(mem_alloc(sizeof(ScriptObjectEntry)));
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

    ScriptObjectEntry *entry = obj_last_entry;
    while (entry != NULL) {
        ScriptObjectEntry *prev = entry->prev;

        ScriptObject **ptr = entry->ptr;
        *ptr = NULL;
        mem_free(entry);

        entry = prev;
    }

    obj_last_entry = NULL;
}

int lua_delete(lua_State *L) {
    int numobjects = lua_gettop(L);
    for (int i = 0; i < numobjects; ++i) {
        ScriptObject *self;
        if (lua_value<ScriptObject *>::pop(L, self)) {
            self->unregisterObject();
            delete self;
        } else {
            NEKO_WARN("[lua] cannot delete an instance that isn't a ScriptObject*");
        }
    }

    return 0;
}

lua_State *lua_bind::L;

void lua_bind::initialize(lua_State *_L) {

    if (L != NULL) {
        NEKO_WARN("[lua] lua_bind already initialized");
        return;
    }

    L = _L;
    // 确保lua中有delete功能
    bind("delete", lua_delete);
    // 注册脚本对象
    bind<ScriptObject>();
}

lua_State *lua_bind::getLuaState() { return L; }

void lua_bind::evaluatef(const_str fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 1024, fmt, args);
    va_end(args);

    if (luaL_loadstring(L, buffer) != 0) {
        std::string err = lua_tostring(L, -1);
        NEKO_WARN("[lua] could not evaluate string \"%s\":\n%s", buffer, err.c_str());
        lua_pop(L, 1);
    }
}

void lua_bind::loadFile(const_str pathToFile) {
    int res = luaL_loadfile(L, pathToFile);
    if (res != 0) {
        std::string err = lua_tostring(L, -1);
        NEKO_WARN("[lua] could not load file: %s", err.c_str());
        lua_pop(L, 1);
    } else {
        res = lua_pcall(L, 0, 0, NULL);
        if (res != 0) {
            std::string err = lua_tostring(L, -1);
            NEKO_WARN("[lua] could not compile supplied script file: %s", err.c_str());
            lua_pop(L, 1);
        }
    }
}

namespace luavalue {
void set(lua_State *L, int idx, value &v) {
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

void set(lua_State *L, int idx, table &t) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        size_t sz = 0;
        const char *str = luaL_checklstring(L, -2, &sz);
        std::pair<std::string, value> pair;
        pair.first.assign(str, sz);
        set(L, -1, pair.second);
        t.emplace(pair);
        lua_pop(L, 1);
    }
}

void get(lua_State *L, const value &v) {
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

void get(lua_State *L, const table &t) {
    lua_createtable(L, 0, static_cast<int>(t.size()));
    for (const auto &[k, v] : t) {
        lua_pushlstring(L, k.data(), k.size());
        get(L, v);
        lua_rawset(L, -3);
    }
}
}  // namespace luavalue

int vfs_lua_loader(lua_State *L) {
    const_str name = luaL_checkstring(L, 1);
    std::string path = name;
    std::replace(path.begin(), path.end(), '.', '\\');

    neko_println("fuck:%s", path.c_str());

    bool ok = false;
    auto load_list = {"code\\libs\\"};
    for (auto p : load_list) {
        std::string load_path = p + path + ".lua";
        String contents = {};
        ok = vfs_read_entire_file(&contents, load_path.c_str());
        if (ok) {
            neko_defer(mem_free(contents.data));
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

    int result = neko_pak_item_data(&ENGINE_INTERFACE()->pack, path.c_str(), (const u8**)&data, &data_size);
    if (result == 0) {
        if (luaL_loadbuffer(L, (char*)data, data_size, name) != LUA_OK) {
            lua_pop(L, 1);
        } else {
            neko_println("loaded:%s", path.c_str());
        }

        neko_pak_item_free(&ENGINE_INTERFACE()->pack, data);
    }
#endif

    lua_pushfstring(L, "[lua] module \"%s\" not found", name);
    return 1;
}

}  // namespace neko

i32 luax_require_script(lua_State *L, String filepath) {
    PROFILE_FUNC();

    if (g_app->error_mode.load()) {
        return LUA_REFNIL;
    }

    String path = to_cstr(filepath);
    neko_defer(mem_free(path.data));

    String contents;
    bool ok = vfs_read_entire_file(&contents, filepath);

    // 如果是读取绝对路径
    if (!ok) ok = read_entire_file_raw(&contents, filepath);

    if (!ok) {
        StringBuilder sb = {};
        neko_defer(sb.trash());
        fatal_error(String(sb << "failed to read script: " << filepath));
        return LUA_REFNIL;
    }
    neko_defer(mem_free(contents.data));

    lua_newtable(L);
    i32 module_table = lua_gettop(L);

    {
        PROFILE_BLOCK("load lua script");

        if (luaL_loadbuffer(L, contents.data, contents.len, path.data) != LUA_OK) {
            fatal_error(luax_check_string(L, -1));
            return LUA_REFNIL;
        }
    }

    // run script
    if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
        lua_pop(L, 2);  // also pop module table
        return LUA_REFNIL;
    }

    // copy return results to module table
    i32 top = lua_gettop(L);
    for (i32 i = 1; i <= top - module_table; i++) {
        lua_seti(L, module_table, i);
    }

    return luaL_ref(L, LUA_REGISTRYINDEX);
}

void luax_stack_dump(lua_State *L) {
    i32 top = lua_gettop(L);
    printf("  --- lua stack (%d) ---\n", top);
    for (i32 i = 1; i <= top; i++) {
        printf("  [%d] (%s): ", i, luaL_typename(L, i));

        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                printf("%f\n", lua_tonumber(L, i));
                break;
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf("%d\n", lua_toboolean(L, i));
                break;
            case LUA_TNIL:
                printf("nil\n");
                break;
            default:
                printf("%p\n", lua_topointer(L, i));
                break;
        }
    }
}

void luax_pcall(lua_State *L, i32 args, i32 results) {
    if (lua_pcall(L, args, results, 1) != LUA_OK) {
        lua_pop(L, 1);
    }
}

void luax_neko_get(lua_State *L, const char *field) {
    lua_getglobal(L, "neko");
    lua_getfield(L, -1, field);
    lua_remove(L, -2);
}

int luax_msgh(lua_State *L) {
    if (g_app->error_mode.load()) {
        return 0;
    }

    String err = luax_check_string(L, -1);

    NEKO_DEBUG_LOG("luax_msgh");

    // traceback = debug.traceback(nil, 2)
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    lua_pushnil(L);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    String traceback = luax_check_string(L, -1);

    if (LockGuard lock{&g_app->error_mtx}) {
        g_app->fatal_error = to_cstr(err);
        g_app->traceback = to_cstr(traceback);

        fprintf(stderr, "%s\n", g_app->fatal_error.data);
        fprintf(stderr, "%s\n", g_app->traceback.data);

        for (u64 i = 0; i < g_app->traceback.len; i++) {
            if (g_app->traceback.data[i] == '\t') {
                g_app->traceback.data[i] = ' ';
            }
        }

        g_app->error_mode.store(true);
    }

    lua_pop(L, 2);  // traceback and error
    return 0;
}

lua_Integer luax_len(lua_State *L, i32 arg) {
    lua_len(L, arg);
    lua_Integer len = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return len;
}

void luax_geti(lua_State *L, i32 arg, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_gettable(L, arg);
}

void luax_set_number_field(lua_State *L, const char *key, lua_Number n) {
    lua_pushnumber(L, n);
    lua_setfield(L, -2, key);
}

void luax_set_int_field(lua_State *L, const char *key, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_setfield(L, -2, key);
}

void luax_set_string_field(lua_State *L, const char *key, const char *str) {
    lua_pushstring(L, str);
    lua_setfield(L, -2, key);
}

lua_Number luax_number_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Number num = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optnumber(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Integer num = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_int_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optinteger(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

String luax_string_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, -1, &len);
    lua_pop(L, 1);
    return {str, len};
}

String luax_opt_string_field(lua_State *L, i32 arg, const char *key, const char *fallback) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_optlstring(L, -1, fallback, &len);
    lua_pop(L, 1);
    return {str, len};
}

bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback) {
    i32 type = lua_getfield(L, arg, key);

    bool b = fallback;
    if (type != LUA_TNIL) {
        b = lua_toboolean(L, -1);
    }

    lua_pop(L, 1);
    return b;
}

String luax_check_string(lua_State *L, i32 arg) {
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, arg, &len);
    return {str, len};
}

String luax_opt_string(lua_State *L, i32 arg, String def) { return lua_isstring(L, arg) ? luax_check_string(L, arg) : def; }

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack, String needle) {
    StringBuilder sb = {};
    neko_defer(sb.trash());

    sb << "expected one of: {";
    for (String s : haystack) {
        sb << "\"" << s << "\", ";
    }
    if (haystack.size() != 0) {
        sb.len -= 2;
    }
    sb << "} got: \"" << needle << "\".";

    return luaL_error(L, "%s", sb.data);
}

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l) {
    luaL_newmetatable(L, mt_name);
    luaL_setfuncs(L, l, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

static void lua_thread_proc(void *udata) {
    PROFILE_FUNC();

    LuaThread *lt = (LuaThread *)udata;

    // lua_State *L = lua_newstate(luaalloc, LA);
    // neko_defer(lua_close(L));

    lua_State *L = neko::neko_lua_create();
    neko_defer(neko::neko_lua_fini(L));

    {
        PROFILE_BLOCK("open libs");
        luaL_openlibs(L);
    }

    {
        PROFILE_BLOCK("open api");
        open_neko_api(L);
    }

    {
        PROFILE_BLOCK("run bootstrap");
        neko::lua::luax_run_bootstrap(L);
    }

    String contents = lt->contents;

    {
        PROFILE_BLOCK("load chunk");
        if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) != LUA_OK) {
            String err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);

            mem_free(contents.data);
            mem_free(lt->name.data);
            return;
        }
    }

    mem_free(contents.data);
    mem_free(lt->name.data);

    {
        PROFILE_BLOCK("run chunk");
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            String err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);
        }
    }
}

void LuaThread::make(String code, String thread_name) {
    mtx.make();
    contents = to_cstr(code);
    name = to_cstr(thread_name);

    LockGuard lock{&mtx};
    thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
    if (LockGuard lock{&mtx}) {
        thread.join();
    }

    mtx.trash();
}

//

void LuaVariant::make(lua_State *L, i32 arg) {
    type = lua_type(L, arg);

    switch (type) {
        case LUA_TBOOLEAN:
            boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            number = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            String s = luax_check_string(L, arg);
            string = to_cstr(s);
            break;
        }
        case LUA_TTABLE: {
            Array<LuaTableEntry> entries = {};
            entries.resize(luax_len(L, arg));

            lua_pushvalue(L, arg);
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                LuaVariant key = {};
                key.make(L, -2);

                LuaVariant value = {};
                value.make(L, -1);

                entries.push({key, value});
            }
            lua_pop(L, 1);

            table = Slice(entries);
            break;
        }
        case LUA_TUSERDATA: {
            i32 kind = lua_getiuservalue(L, arg, LUAX_UD_TNAME);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TSTRING) {
                return;
            }

            kind = lua_getiuservalue(L, arg, LUAX_UD_PTR_SIZE);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TNUMBER) {
                return;
            }

            String tname = luax_check_string(L, -2);
            u64 size = luaL_checkinteger(L, -1);

            if (size != sizeof(void *)) {
                return;
            }

            udata.ptr = *(void **)lua_touserdata(L, arg);
            udata.tname = to_cstr(tname);

            break;
        }
        default:
            break;
    }
}

void LuaVariant::trash() {
    switch (type) {
        case LUA_TSTRING: {
            mem_free(string.data);
            break;
        }
        case LUA_TTABLE: {
            for (LuaTableEntry e : table) {
                e.key.trash();
                e.value.trash();
            }
            mem_free(table.data);
        }
        case LUA_TUSERDATA: {
            mem_free(udata.tname.data);
        }
        default:
            break;
    }
}

void LuaVariant::push(lua_State *L) {
    switch (type) {
        case LUA_TBOOLEAN:
            lua_pushboolean(L, boolean);
            break;
        case LUA_TNUMBER:
            lua_pushnumber(L, number);
            break;
        case LUA_TSTRING:
            lua_pushlstring(L, string.data, string.len);
            break;
        case LUA_TTABLE: {
            lua_newtable(L);
            for (LuaTableEntry e : table) {
                e.key.push(L);
                e.value.push(L);
                lua_rawset(L, -3);
            }
            break;
        }
        case LUA_TUSERDATA: {
            luax_ptr_userdata(L, udata.ptr, udata.tname.data);
            break;
        }
        default:
            break;
    }
}

//

struct LuaChannels {
    Mutex mtx;
    Cond select;
    HashMap<LuaChannel *> by_name;
};

static LuaChannels g_channels = {};

void LuaChannel::make(String n, u64 buf) {
    mtx.make();
    sent.make();
    received.make();
    items.data = (LuaVariant *)mem_alloc(sizeof(LuaVariant) * (buf + 1));
    items.len = (buf + 1);
    front = 0;
    back = 0;
    len = 0;

    name.store(to_cstr(n).data);
}

void LuaChannel::trash() {
    for (i32 i = 0; i < len; i++) {
        items[front].trash();
        front = (front + 1) % items.len;
    }

    mem_free(items.data);
    mem_free(name.exchange(nullptr));
    mtx.trash();
    sent.trash();
    received.trash();
}

void LuaChannel::send(LuaVariant item) {
    LockGuard lock{&mtx};

    while (len == items.len) {
        received.wait(&mtx);
    }

    items[back] = item;
    back = (back + 1) % items.len;
    len++;

    g_channels.select.broadcast();
    sent.signal();
    sent_total++;

    while (sent_total >= received_total + items.len) {
        received.wait(&mtx);
    }
}

static LuaVariant lua_channel_dequeue(LuaChannel *ch) {
    LuaVariant item = ch->items[ch->front];
    ch->front = (ch->front + 1) % ch->items.len;
    ch->len--;

    ch->received.broadcast();
    ch->received_total++;

    return item;
}

LuaVariant LuaChannel::recv() {
    LockGuard lock{&mtx};

    while (len == 0) {
        sent.wait(&mtx);
    }

    return lua_channel_dequeue(this);
}

bool LuaChannel::try_recv(LuaVariant *v) {
    LockGuard lock{&mtx};

    if (len == 0) {
        return false;
    }

    *v = lua_channel_dequeue(this);
    return true;
}

LuaChannel *lua_channel_make(String name, u64 buf) {
    LuaChannel *chan = (LuaChannel *)mem_alloc(sizeof(LuaChannel));
    new (&chan->name) std::atomic<char *>();
    chan->make(name, buf);

    LockGuard lock{&g_channels.mtx};
    g_channels.by_name[fnv1a(name)] = chan;

    return chan;
}

LuaChannel *lua_channel_get(String name) {
    LockGuard lock{&g_channels.mtx};

    LuaChannel **chan = g_channels.by_name.get(fnv1a(name));
    if (chan == nullptr) {
        return nullptr;
    }

    return *chan;
}

LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v) {
    i32 len = lua_gettop(L);
    if (len == 0) {
        return nullptr;
    }

    LuaChannel *buf[16] = {};
    for (i32 i = 0; i < len; i++) {
        buf[i] = *(LuaChannel **)luaL_checkudata(L, i + 1, "mt_channel");
    }

    Mutex mtx = {};
    mtx.make();
    LockGuard lock{&mtx};

    while (true) {
        for (i32 i = 0; i < len; i++) {
            LockGuard lock{&buf[i]->mtx};
            if (buf[i]->len > 0) {
                *v = lua_channel_dequeue(buf[i]);
                return buf[i];
            }
        }

        g_channels.select.wait(&mtx);
    }
}

void lua_channels_setup() {
    g_channels.select.make();
    g_channels.mtx.make();
}

void lua_channels_shutdown() {
    for (auto [k, v] : g_channels.by_name) {
        LuaChannel *chan = *v;
        chan->trash();
        mem_free(chan);
    }
    g_channels.by_name.trash();
    g_channels.select.trash();
    g_channels.mtx.trash();
}

#define FREELIST 1

void neko_luaref::make(lua_State *L) {
    this->refL = lua_newthread(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, refL);
    lua_newtable(refL);
}

void neko_luaref::fini() {
    lua_State *L = (lua_State *)refL;
    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, refL);
}

bool neko_luaref::isvalid(int ref) {
    if (ref <= FREELIST || ref > lua_gettop(refL)) {
        return false;
    }
    lua_pushnil(refL);
    while (lua_next(refL, FREELIST)) {
        lua_pop(refL, 1);
        if (lua_tointeger(refL, -1) == ref) {
            lua_pop(refL, 1);
            return false;
        }
    }
    return true;
}

int neko_luaref::ref(lua_State *L) {
    if (!lua_checkstack(refL, 3)) {
        return LUA_NOREF;
    }
    lua_xmove(L, refL, 1);
    lua_pushnil(refL);
    if (!lua_next(refL, FREELIST)) {
        return lua_gettop(refL);
    }
    int r = (int)lua_tointeger(refL, -2);
    lua_pop(refL, 1);
    lua_pushnil(refL);
    lua_rawset(refL, FREELIST);
    lua_replace(refL, r);
    return r;
}

void neko_luaref::unref(int ref) {
    if (ref <= FREELIST) {
        return;
    }
    int top = lua_gettop(refL);
    if (ref > top) {
        return;
    }
    if (ref < top) {
        lua_pushboolean(refL, 1);
        lua_rawseti(refL, FREELIST, ref);
        lua_pushnil(refL);
        lua_replace(refL, ref);
        return;
    }
    for (--top; top > FREELIST; --top) {
        if (LUA_TNIL == lua_rawgeti(refL, FREELIST, top)) {
            lua_pop(refL, 1);
            break;
        }
        lua_pop(refL, 1);
        lua_pushnil(refL);
        lua_rawseti(refL, FREELIST, top);
    }
    lua_settop(refL, top);
}

void neko_luaref::get(lua_State *L, int ref) {
    assert(isvalid(ref));
    lua_pushvalue(refL, ref);
    lua_xmove(refL, L, 1);
}

void neko_luaref::set(lua_State *L, int ref) {
    assert(isvalid(ref));
    lua_xmove(L, refL, 1);
    lua_replace(refL, ref);
}

int __neko_bind_callback_save(lua_State *L) {
    const_str identifier = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (g_app->g_lua_callbacks_table_ref == LUA_NOREF) {
        lua_newtable(L);
        g_app->g_lua_callbacks_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_app->g_lua_callbacks_table_ref);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, identifier);
    lua_pop(L, 1);
    lua_pop(L, 2);
    return 0;
}

int __neko_bind_callback_call(lua_State *L) {
    if (g_app->g_lua_callbacks_table_ref != LUA_NOREF) {
        const_str identifier = luaL_checkstring(L, 1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_app->g_lua_callbacks_table_ref);
        lua_getfield(L, -1, identifier);
        if (lua_isfunction(L, -1)) {
            int nargs = lua_gettop(L) - 1;  // 获取参数数量 减去标识符参数
            for (int i = 2; i <= nargs + 1; ++i) {
                lua_pushvalue(L, i);
            }
            lua_call(L, nargs, 0);  // 调用
        } else {
            NEKO_WARN("callback with identifier '%s' not found or is not a function", identifier);
        }
        lua_pop(L, 1);
    } else {
        NEKO_WARN("callback table is noref");
    }
    return 0;
}

// 返回一些有助于识别对象的名称
[[nodiscard]] static int DiscoverObjectNameRecur(lua_State *L, int shortest_, int depth_) {
    static constexpr int kWhat{1};
    static constexpr int kResult{2};
    static constexpr int kCache{3};
    static constexpr int kFQN{4};

    if (shortest_ <= depth_ + 1) {
        return shortest_;
    }
    NEKO_ASSERT(lua_checkstack(L, 3));

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
                std::ignore = neko::lua_tool::PushFQN(L, kFQN, depth_);
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

int __neko_bind_nameof(lua_State *L) {
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