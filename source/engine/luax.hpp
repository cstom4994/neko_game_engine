
#ifndef NEKO_LUAX_HPP
#define NEKO_LUAX_HPP

#include <atomic>
#include <initializer_list>

#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/luax.h"
#include "engine/prelude.h"
#include "vendor/luaalloc.h"

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

namespace neko::lua {
void luax_run_bootstrap(lua_State *L);
void luax_run_nekogame(lua_State *L);
}  // namespace neko::lua

i32 luax_require_script(lua_State *L, String filepath);

void luax_stack_dump(lua_State *L);

void luax_get(lua_State *L, const_str tb, const_str field);
void luax_pcall(lua_State *L, i32 args, i32 results);

// get field in neko namespace
void luax_neko_get(lua_State *L, const char *field);

// message handler. prints error and traceback
int luax_msgh(lua_State *L);

lua_Integer luax_len(lua_State *L, i32 arg);
void luax_geti(lua_State *L, i32 arg, lua_Integer n);

// set table value at top of stack
void luax_set_number_field(lua_State *L, const char *key, lua_Number n);
void luax_set_int_field(lua_State *L, const char *key, lua_Integer n);
void luax_set_string_field(lua_State *L, const char *key, const char *str);

// get value from table
lua_Number luax_number_field(lua_State *L, i32 arg, const char *key);
lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key, lua_Number fallback);

lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key);
lua_Integer luax_opt_int_field(lua_State *L, i32 arg, const char *key, lua_Integer fallback);

String luax_string_field(lua_State *L, i32 arg, const char *key);
String luax_opt_string_field(lua_State *L, i32 arg, const char *key, const char *fallback);

bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback = false);

String luax_check_string(lua_State *L, i32 arg);
String luax_opt_string(lua_State *L, i32 arg, String def);

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack, String needle);
void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l);

inline void luax_package_preload(lua_State *L, const_str name, lua_CFunction function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
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

struct LuaThread {
    Mutex mtx;
    String contents;
    String name;
    Thread thread;

    void make(String code, String thread_name);
    void join();
};

struct lua_State;
struct LuaTableEntry;
struct LuaVariant {
    i32 type;
    union {
        bool boolean;
        double number;
        String string;
        Slice<LuaTableEntry> table;
        struct {
            void *ptr;
            String tname;
        } udata;
    };

    void make(lua_State *L, i32 arg);
    void trash();
    void push(lua_State *L);
};

struct LuaTableEntry {
    LuaVariant key;
    LuaVariant value;
};

struct LuaChannel {
    std::atomic<char *> name;

    Mutex mtx;
    Cond received;
    Cond sent;

    u64 received_total;
    u64 sent_total;

    Slice<LuaVariant> items;
    u64 front;
    u64 back;
    u64 len;

    void make(String n, u64 buf);
    void trash();
    void send(LuaVariant item);
    LuaVariant recv();
    bool try_recv(LuaVariant *v);
};

LuaChannel *lua_channel_make(String name, u64 buf);
LuaChannel *lua_channel_get(String name);
LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v);
void lua_channels_setup();
void lua_channels_shutdown();

typedef lua_State *luaref;

struct neko_luaref {
    luaref refL;

    void make(lua_State *L);
    void fini();

    bool isvalid(int ref);
    int ref(lua_State *L);
    void unref(int ref);
    void get(lua_State *L, int ref);
    void set(lua_State *L, int ref);
};

int __neko_bind_callback_save(lua_State *L);
int __neko_bind_callback_call(lua_State *L);
int __neko_bind_nameof(lua_State *L);

#endif

#if !defined(NEKO_LUA_UTIL_H)
#define NEKO_LUA_UTIL_H

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <format>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#ifndef _WIN32
#include <stdint.h>
#define SPRINTF_F snprintf
#else
#define SPRINTF_F _snprintf_s
#endif

#define INHERIT_TABLE "inherit_table"

namespace neko {

typedef int (*lua_function_t)(lua_State *L);

class lua_nil_t {};
struct cpp_void_t {};

template <typename T>
struct userdata_for_object_t {
    userdata_for_object_t(T *p_ = NULL) : obj(p_) {}
    T *obj;
};

size_t neko_lua_mem_usage();

void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize);

struct lua_tool {

    static std::string_view PushFQN(lua_State *const L, int const t, int const last) {
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

    static void check_arg_count(lua_State *L, int expected) {
        int n = lua_gettop(L);
        if (n != expected) {
            luaL_error(L, "Got %d arguments, expected %d", n, expected);
            return;
        }
        return;
    };

    static lua_State *mark_create(const_str mark_name) {
        lua_State *L = lua_newstate(Allocf, NULL);
        lua_newtable(L);
        lua_setglobal(L, mark_name);
        return L;
    }

    static void mark_table(lua_State *L, void *table, const_str mark_name) {
        lua_getglobal(L, mark_name);
        lua_pushlightuserdata(L, table);
        lua_pushboolean(L, 1);
        lua_settable(L, -3);
        lua_pop(L, 1);
    }

    static int mark_table_check(lua_State *L, void *table, const_str mark_name) {
        int result = 0;
        lua_getglobal(L, mark_name);
        lua_pushlightuserdata(L, table);
        lua_gettable(L, -2);
        if (!lua_isnil(L, -1)) {
            result = 1;
        }
        lua_pop(L, 2);
        return result;
    }

    static void dump_stack(lua_State *L) {
        int top = lua_gettop(L);
        lua_State *visited = mark_create("visited");

        for (int i = 1; i <= top; i++) {
            int t = lua_type(L, i);

            const char *name = get_object_name(L, i);
            printf("%d: [%s] ", i, name);

            switch (t) {
                case LUA_TSTRING:
                    printf("`%s'", lua_tostring(L, i));
                    break;
                case LUA_TBOOLEAN:
                    printf(lua_toboolean(L, i) ? "true" : "false");
                    break;
                case LUA_TNUMBER:
                    printf("`%g`", lua_tonumber(L, i));
                    break;
                case LUA_TTABLE:
                    printf("table {\n");
                    dump_table(L, i, 1, visited);
                    printf("}");
                    break;
                default:
                    printf("`%s`", lua_typename(L, t));
                    break;
            }
            printf("\n");
        }
        lua_close(visited);
    }

    static void dump_table(lua_State *L, int index, int depth, lua_State *visited) {
        // 确保索引是绝对的，以避免 lua_next 改变它
        index = lua_absindex(L, index);

        // 检查该表是否已经访问过
        if (mark_table_check(visited, (void *)lua_topointer(L, index), "visited")) {
            for (int i = 0; i < depth; i++) {
                printf("  ");
            }
            printf("[recursive table]\n");
            return;
        }
        // 标记该表为已访问
        mark_table(visited, (void *)lua_topointer(L, index), "visited");

        lua_pushnil(L);  // 首个键
        while (lua_next(L, index) != 0) {
            // 缩进以反映深度
            for (int i = 0; i < depth; i++) {
                printf("  ");
            }

            // 获取键和值的类型和名称
            const char *key_name = get_object_name(L, -2);
            const char *key_type = lua_typename(L, lua_type(L, -2));
            const char *value_type = lua_typename(L, lua_type(L, -1));

            printf("%s (%s) - %s: ", key_name, key_type, value_type);

            // 打印值
            switch (lua_type(L, -1)) {
                case LUA_TSTRING:
                    printf("`%s'", lua_tostring(L, -1));
                    break;
                case LUA_TBOOLEAN:
                    printf(lua_toboolean(L, -1) ? "true" : "false");
                    break;
                case LUA_TNUMBER:
                    printf("`%g`", lua_tonumber(L, -1));
                    break;
                case LUA_TTABLE:
                    printf("table {\n");
                    dump_table(L, lua_gettop(L), depth + 1, visited);
                    for (int i = 0; i < depth; i++) {
                        printf("  ");
                    }
                    printf("}");
                    break;
                default:
                    printf("`%s`", lua_typename(L, lua_type(L, -1)));
                    break;
            }
            printf("\n");

            lua_pop(L, 1);  // 弹出值，保留键以进行下一次迭代
        }
    }

    static const char *get_object_name(lua_State *L, int index) {
        const char *name = NULL;

        // 尝试通过局部变量名获取
        lua_Debug ar;
        if (lua_getstack(L, 1, &ar)) {
            int i = 1;
            const char *local_name;
            while ((local_name = lua_getlocal(L, &ar, i++)) != NULL) {
                if (lua_rawequal(L, index, -1)) {
                    name = local_name;
                    lua_pop(L, 1);
                    break;
                }
                lua_pop(L, 1);  // 弹出值
            }
        }

        // 尝试通过全局变量名获取
        if (!name) {
            lua_pushglobaltable(L);
            lua_pushnil(L);  // 首个键
            while (lua_next(L, -2) != 0) {
                if (lua_rawequal(L, index, -1)) {
                    name = lua_tostring(L, -2);
                    lua_pop(L, 2);  // 弹出值和键
                    break;
                }
                lua_pop(L, 1);  // 弹出值，保留键以进行下一次迭代
            }
            lua_pop(L, 1);  // 弹出全局表
        }

        return name ? name : "unknown";
    }

    static std::string dump_error(lua_State *ls_, const char *fmt, ...) {
        std::string ret;
        char buff[4096];

        va_list argp;
        va_start(argp, fmt);
#ifndef _WIN32
        vsnprintf(buff, sizeof(buff), fmt, argp);
#else
        vsnprintf_s(buff, sizeof(buff), sizeof(buff), fmt, argp);
#endif
        va_end(argp);

        ret = buff;
        SPRINTF_F(buff, sizeof(buff), " tracback:\n\t%s", lua_tostring(ls_, -1));
        ret += buff;

        return ret;
    }
};

namespace luabind {

template <typename T>
struct LuaStack;

template <class T>
inline void lua_push(lua_State *L, T t) {
    LuaStack<T>::push(L, t);
}

template <>
struct LuaStack<lua_State *> {
    static lua_State *get(lua_State *L, int) { return L; }
};

template <>
struct LuaStack<lua_CFunction> {
    static void push(lua_State *L, lua_CFunction f) { lua_pushcfunction(L, f); }
    static lua_CFunction get(lua_State *L, int index) { return lua_tocfunction(L, index); }
};

template <>
struct LuaStack<int> {
    static inline void push(lua_State *L, int value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline int get(lua_State *L, int index) { return static_cast<int>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<int const &> {
    static inline void push(lua_State *L, int value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline int get(lua_State *L, int index) { return static_cast<int>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<unsigned int> {
    static inline void push(lua_State *L, unsigned int value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline unsigned int get(lua_State *L, int index) { return static_cast<unsigned int>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<unsigned int const &> {
    static inline void push(lua_State *L, unsigned int value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline unsigned int get(lua_State *L, int index) { return static_cast<unsigned int>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<unsigned char> {
    static inline void push(lua_State *L, unsigned char value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline unsigned char get(lua_State *L, int index) { return static_cast<unsigned char>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<unsigned char const &> {
    static inline void push(lua_State *L, unsigned char value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline unsigned char get(lua_State *L, int index) { return static_cast<unsigned char>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<short> {
    static inline void push(lua_State *L, short value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline short get(lua_State *L, int index) { return static_cast<short>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<short const &> {
    static inline void push(lua_State *L, short value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline short get(lua_State *L, int index) { return static_cast<short>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<unsigned short> {
    static inline void push(lua_State *L, unsigned short value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline unsigned short get(lua_State *L, int index) { return static_cast<unsigned short>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<unsigned short const &> {
    static inline void push(lua_State *L, unsigned short value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline unsigned short get(lua_State *L, int index) { return static_cast<unsigned short>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<long> {
    static inline void push(lua_State *L, long value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline long get(lua_State *L, int index) { return static_cast<long>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<long const &> {
    static inline void push(lua_State *L, long value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline long get(lua_State *L, int index) { return static_cast<long>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<unsigned long> {
    static inline void push(lua_State *L, unsigned long value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
    static inline unsigned long get(lua_State *L, int index) { return static_cast<unsigned long>(luaL_checkinteger(L, index)); }
};

template <>
struct LuaStack<unsigned long const &> {
    static inline void push(lua_State *L, unsigned long value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline unsigned long get(lua_State *L, int index) { return static_cast<unsigned long>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<float> {
    static inline void push(lua_State *L, float value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline float get(lua_State *L, int index) { return static_cast<float>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<float const &> {
    static inline void push(lua_State *L, float value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline float get(lua_State *L, int index) { return static_cast<float>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<double> {
    static inline void push(lua_State *L, double value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline double get(lua_State *L, int index) { return static_cast<double>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<double const &> {
    static inline void push(lua_State *L, double value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
    static inline double get(lua_State *L, int index) { return static_cast<double>(luaL_checknumber(L, index)); }
};

template <>
struct LuaStack<bool> {
    static inline void push(lua_State *L, bool value) { lua_pushboolean(L, value ? 1 : 0); }
    static inline bool get(lua_State *L, int index) { return lua_toboolean(L, index) ? true : false; }
};

template <>
struct LuaStack<bool const &> {
    static inline void push(lua_State *L, bool value) { lua_pushboolean(L, value ? 1 : 0); }
    static inline bool get(lua_State *L, int index) { return lua_toboolean(L, index) ? true : false; }
};

template <>
struct LuaStack<char> {
    static inline void push(lua_State *L, char value) {
        char str[2] = {value, 0};
        lua_pushstring(L, str);
    }

    static inline char get(lua_State *L, int index) { return luaL_checkstring(L, index)[0]; }
};

template <>
struct LuaStack<char const &> {
    static inline void push(lua_State *L, char value) {
        char str[2] = {value, 0};
        lua_pushstring(L, str);
    }

    static inline char get(lua_State *L, int index) { return luaL_checkstring(L, index)[0]; }
};

template <>
struct LuaStack<char const *> {
    static inline void push(lua_State *L, char const *str) {
        if (str != 0)
            lua_pushstring(L, str);
        else
            lua_pushnil(L);
    }

    static inline char const *get(lua_State *L, int index) { return lua_isnil(L, index) ? 0 : luaL_checkstring(L, index); }
};

template <>
struct LuaStack<std::string> {
    static inline void push(lua_State *L, std::string const &str) { lua_pushlstring(L, str.c_str(), str.size()); }

    static inline std::string get(lua_State *L, int index) {
        size_t len;
        const char *str = luaL_checklstring(L, index, &len);
        return std::string(str, len);
    }
};

template <>
struct LuaStack<std::string const &> {
    static inline void push(lua_State *L, std::string const &str) { lua_pushlstring(L, str.c_str(), str.size()); }

    static inline std::string get(lua_State *L, int index) {
        size_t len;
        const char *str = luaL_checklstring(L, index, &len);
        return std::string(str, len);
    }
};

};  // namespace luabind

template <typename T>
T neko_lua_to(lua_State *L, int index) {
    if constexpr (std::same_as<T, i32> || std::same_as<T, u32>) {
        luaL_argcheck(L, lua_isnumber(L, index), index, "number expected");
        return static_cast<T>(lua_tointeger(L, index));
    } else if constexpr (std::same_as<T, f32> || std::same_as<T, f64>) {
        luaL_argcheck(L, lua_isnumber(L, index), index, "number expected");
        return static_cast<T>(lua_tonumber(L, index));
    } else if constexpr (std::same_as<T, const_str>) {
        luaL_argcheck(L, lua_isstring(L, index), index, "string expected");
        return lua_tostring(L, index);
    } else if constexpr (std::same_as<T, bool>) {
        luaL_argcheck(L, lua_isboolean(L, index), index, "boolean expected");
        return lua_toboolean(L, index) != 0;
    } else if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<T>(lua_topointer(L, index));
    } else {
        static_assert(std::is_same_v<T, void>, "Unsupported type for neko_lua_to");
    }
}

template <typename Iterable>
inline bool neko_lua_equal(lua_State *state, const Iterable &indices) {
    auto it = indices.begin();
    auto end = indices.end();
    if (it == end) return true;
    int cmp_index = *it++;
    while (it != end) {
        int index = *it++;
        if (!neko_lua_equal(state, cmp_index, index)) return false;
        cmp_index = index;
    }
    return true;
}

//! 表示void类型，由于void类型不能return，用void_ignore_t适配
template <typename T>
struct void_ignore_t;

template <typename T>
struct void_ignore_t {
    typedef T value_t;
};

template <>
struct void_ignore_t<void> {
    typedef cpp_void_t value_t;
};

#define ret(R) typename void_ignore_t<R>::value_t

inline lua_State *neko_lua_create() {

    // LuaAlloc *LA = luaalloc_create(
    //         +[](void *user, void *ptr, size_t osize, size_t nsize) -> void * {
    //             (void)user;
    //             (void)osize;
    //             if (nsize) return realloc(ptr, nsize);
    //             free(ptr);
    //             return nullptr;
    //         },
    //         nullptr);

    // neko_defer(luaalloc_delete(LA));

    lua_State *_L = ::lua_newstate(Allocf, NULL);
    ::luaL_openlibs(_L);

    __neko_luabind_init(_L);

    return _L;
}

inline void neko_lua_fini(lua_State *_L) {
    if (_L) {
        __neko_luabind_fini(_L);
        int top = lua_gettop(_L);
        if (top != 0) {
            lua_tool::dump_stack(_L);
            console_log("luastack memory leak");
        }
        ::lua_close(_L);
        _L = NULL;
    }
}

void neko_lua_run_string(lua_State *_L, const_str str_);

inline void neko_lua_run_string(lua_State *_L, const std::string &str_) { neko_lua_run_string(_L, str_.c_str()); }

inline void neko_lua_dump_stack(lua_State *_L) { lua_tool::dump_stack(_L); }

inline int neko_lua_add_package_path(lua_State *L, const std::string &str_) {
    std::string new_path = "package.path = package.path .. \"";
    if (str_.empty()) return -1;
    if (str_[0] != ';') new_path += ";";
    new_path += str_;
    if (str_[str_.length() - 1] != '/') new_path += "/";
    new_path += "?.lua\" ";
    neko_lua_run_string(L, new_path);
    return 0;
}

inline int neko_lua_load_file(lua_State *_L, const std::string &file_name_)  //
{
    if (luaL_dofile(_L, file_name_.c_str())) {
        std::string err = lua_tool::dump_error(_L, "cannot load file<%s>", file_name_.c_str());
        ::lua_pop(_L, 1);
        // console_log("%s", err.c_str());
    }

    return 0;
}

inline int neko_lua_safe_dofile(lua_State *state, const std::string &file) {
    neko_lua_run_string(state, std::format("xpcall(function ()\nrequire '{0}'\nend, function(err)\nprint(tostring(err))\nprint(debug.traceback(nil, 2))\n__neko_quit(1)\nend)\n", file));
    return 0;
}

inline void neko_lua_call(lua_State *_L, const char *func_name_) {
    lua_getglobal(_L, func_name_);

    if (lua_pcall(_L, 0, 0, 0) != 0) {
        std::string err = lua_tool::dump_error(_L, "lua_pcall_wrap failed func_name<%s>", func_name_);
        ::lua_pop(_L, 1);
        // console_log("%s", err.c_str());
    }
}

namespace luavalue {
template <class>
inline constexpr bool always_false_v = false;

using value = std::variant<std::monostate,  // LUA_TNIL
                           bool,            // LUA_TBOOLEAN
                           void *,          // LUA_TLIGHTUSERDATA
                           lua_Integer,     // LUA_TNUMBER
                           lua_Number,      // LUA_TNUMBER
                           std::string,     // LUA_TSTRING
                           lua_CFunction    // LUA_TFUNCTION
                           >;
using table = std::map<std::string, value>;

void set(lua_State *L, int idx, value &v);
void set(lua_State *L, int idx, table &v);
void get(lua_State *L, const value &v);
void get(lua_State *L, const table &v);
}  // namespace luavalue

int vfs_lua_loader(lua_State *L);

}  // namespace neko

#endif

#ifndef NEKO_LUA_REF_HPP
#define NEKO_LUA_REF_HPP

#include <string>
#include <tuple>  // std::ignore

namespace neko {

struct LuaNil {};

template <>
struct luabind::LuaStack<LuaNil> {
    static inline void push(lua_State *L, LuaNil const &nil) { lua_pushnil(L); }
};

class LuaRef;

class LuaRefBase {
protected:
    lua_State *L;
    int m_ref;

    struct lua_stack_auto_popper {
        lua_State *L;
        int m_count;
        lua_stack_auto_popper(lua_State *L, int count = 1) : L(L), m_count(count) {}
        ~lua_stack_auto_popper() { lua_pop(L, m_count); }
    };
    struct FromStackIndex {};

    // 不应该直接使用
    LuaRefBase(lua_State *L, FromStackIndex) : L(L) { m_ref = luaL_ref(L, LUA_REGISTRYINDEX); }
    LuaRefBase(lua_State *L, int ref) : L(L), m_ref(ref) {}
    ~LuaRefBase() { luaL_unref(L, LUA_REGISTRYINDEX, m_ref); }

public:
    virtual void push() const { lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); }

    std::string tostring() const {
        lua_getglobal(L, "tostring");
        push();
        lua_call(L, 1, 1);
        const char *str = lua_tostring(L, 1);
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
    LuaTableElement(lua_State *L, K key) : LuaRefBase(L, FromStackIndex()), m_key(key) {}

    void push() const override {
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        neko::luabind::LuaStack<K>::push(L, (K)m_key);
        lua_gettable(L, -2);
        lua_remove(L, -2);
    }

    // 为该表/键分配一个新值
    template <typename T>
    LuaTableElement &operator=(T v) {
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
    static inline void push(lua_State *L, LuaTableElement<K> const &e) { e.push(); }
};

class LuaRef : public LuaRefBase {
    friend LuaRefBase;

private:
    LuaRef(lua_State *L, FromStackIndex fs) : LuaRefBase(L, fs) {}

public:
    LuaRef(lua_State *L) : LuaRefBase(L, LUA_REFNIL) {}

    LuaRef(lua_State *L, const std::string &global) : LuaRefBase(L, LUA_REFNIL) {
        lua_getglobal(L, global.c_str());
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LuaRef(LuaRef const &other) : LuaRefBase(other.L, LUA_REFNIL) {
        other.push();
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
        other.push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaRef &operator=(LuaTableElement<K> &&other) noexcept {
        luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
        other.push();
        L = other.L;
        m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return *this;
    }

    template <typename K>
    LuaRef &operator=(LuaTableElement<K> const &other) {
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

    static LuaRef fromStack(lua_State *L, int index = -1) {
        lua_pushvalue(L, index);
        return LuaRef(L, FromStackIndex());
    }

    static LuaRef newTable(lua_State *L) {
        lua_newtable(L);
        return LuaRef(L, FromStackIndex());
    }

    static LuaRef getGlobal(lua_State *L, char const *name) {
        lua_getglobal(L, name);
        return LuaRef(L, FromStackIndex());
    }
};

template <>
struct luabind::LuaStack<LuaRef> {
    static inline void push(lua_State *L, LuaRef const &r) { r.push(); }
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
