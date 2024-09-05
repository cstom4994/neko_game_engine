
#ifndef NEKO_LUAX_HPP
#define NEKO_LUAX_HPP

#include <atomic>
#include <initializer_list>
#include <string>
#include <typeindex>
#include <vector>

#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/luax.h"
#include "vendor/luaalloc.h"

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

NEKO_API() void luax_run_bootstrap(lua_State *L);
NEKO_API() void luax_run_nekogame(lua_State *L);

i32 luax_require_script(lua_State *L, String filepath);

NEKO_API() void luax_stack_dump(lua_State *L);

NEKO_API() void luax_get(lua_State *L, const_str tb, const_str field);
NEKO_API() void luax_pcall(lua_State *L, i32 args, i32 results);

// get field in neko namespace
NEKO_API() void luax_neko_get(lua_State *L, const char *field);

// message handler. prints error and traceback
NEKO_API() int luax_msgh(lua_State *L);

NEKO_API() lua_Integer luax_len(lua_State *L, i32 arg);
NEKO_API() void luax_geti(lua_State *L, i32 arg, lua_Integer n);

// set table value at top of stack
NEKO_API() void luax_set_number_field(lua_State *L, const char *key, lua_Number n);
NEKO_API() void luax_set_int_field(lua_State *L, const char *key, lua_Integer n);
NEKO_API() void luax_set_string_field(lua_State *L, const char *key, const char *str);

// get value from table
NEKO_API() lua_Number luax_number_field(lua_State *L, i32 arg, const char *key);
NEKO_API() lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key, lua_Number fallback);

NEKO_API() lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key);
NEKO_API() lua_Integer luax_opt_int_field(lua_State *L, i32 arg, const char *key, lua_Integer fallback);

String luax_string_field(lua_State *L, i32 arg, const char *key);
String luax_opt_string_field(lua_State *L, i32 arg, const char *key, const char *fallback);

NEKO_API() bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback = false);

String luax_check_string(lua_State *L, i32 arg);
String luax_opt_string(lua_State *L, i32 arg, String def);

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack, String needle);
void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l);

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
        lua_State *L = luaL_newstate();
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
    } else if constexpr (std::same_as<T, String>) {
        // luaL_argcheck(L, lua_isstring(L, index), index, "Neko::String expected");
        return luax_check_string(L, index);
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

struct neko_luastate {
    lua_State *L;
    LuaAlloc *LA;
};

inline neko_luastate neko_lua_create() {

    neko_luastate l = {};

    l.LA = luaalloc_create(
            +[](void *user, void *ptr, size_t osize, size_t nsize) -> void * {
                (void)user;
                (void)osize;
                if (nsize) return mem_realloc(ptr, nsize);
                mem_free(ptr);
                return nullptr;
            },
            nullptr);

#ifdef _DEBUG
    l.L = ::lua_newstate(luaalloc, l.LA);
#else
    l.L = ::luaL_newstate();
#endif

    ::luaL_openlibs(l.L);

    __neko_luabind_init(l.L);

    return l;
}

inline void neko_lua_fini(neko_luastate l) {
    if (l.L) {
        __neko_luabind_fini(l.L);
        int top = lua_gettop(l.L);
        if (top != 0) {
            lua_tool::dump_stack(l.L);
            console_log("luastack memory leak");
        }
        ::lua_close(l.L);
    }
    if (l.LA) {
        luaalloc_delete(l.LA);
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

enum W_LUA_UPVALUES { NEKO_W_COMPONENTS_NAME = 1, NEKO_W_UPVAL_N };

struct W_LUA_REGISTRY_CONST {
    static constexpr i32 W_CORE_IDX = 1;                             // neko_instance_t* index
    static constexpr const_str W_CORE = "__NEKO_W_CORE";             // neko_instance_t* reg
    static constexpr const_str ENG_UDATA_NAME = "__NEKO_ENGINE_UD";  // neko_instance_t* udata
    static constexpr const_str CVAR_MAP = "cvar_map";

    static constexpr i32 CVAR_MAP_MAX = 64;
};

#if 0

template <typename T>
struct neko_w_lua_variant {
    enum { NATIVE, LUA } stype;
    i32 type = LUA_TNONE;
    String cname;
    union {
        bool boolean;
        double number;
        const_str str;
    } data;

    neko_w_lua_variant(const_str _cname) : cname(_cname) {}

    neko_w_lua_variant(const_str _cname, T _v) : neko_w_lua_variant(_cname) {

        using TT = std::decay_t<T>;

        if constexpr (std::same_as<TT, bool>) {
            type = LUA_TBOOLEAN;
            data.boolean = _v;
        } else if constexpr (std::is_integral_v<TT>) {
            type = LUA_TNUMBER;
            data.number = _v;
        } else if constexpr (std::is_floating_point_v<TT>) {
            type = LUA_TNUMBER;
            data.number = _v;
        } else if constexpr (std::is_same_v<TT, String>) {
            type = LUA_TSTRING;
            data.str = _v.data;
        } else if constexpr (neko::is_pointer_to_const_char<TT>) {
            type = LUA_TSTRING;
            data.str = _v;
        } else {
            static_assert(std::is_void_v<TT>, "unsupported type for neko_w_lua_variant");
        }

        this->make();
    }

    template <typename C>
    neko_w_lua_variant &operator=(const C &value) {
        using TT = std::decay_t<C>;
        if constexpr (std::is_same_v<TT, i32> || std::is_same_v<TT, u32> || std::is_same_v<TT, f32> || std::is_same_v<TT, f64>) {
            type = LUA_TNUMBER;
            data.number = static_cast<double>(value);
        } else if constexpr (std::is_same_v<TT, const_str>) {
            type = LUA_TSTRING;
            data.str = value;
        } else if constexpr (std::is_same_v<TT, bool>) {
            type = LUA_TBOOLEAN;
            data.boolean = value;
            // } else if constexpr (std::is_pointer_v<TT>) {
            //     type = LUA_TUSERDATA;
            //     data.udata.ptr = reinterpret_cast<void *>(const_cast<TT>(value));
        } else {
            static_assert("unsupported type for neko_w_lua_variant::=");
        }
        this->make();
        return *this;
    }

    void push() {
        lua_State *L = ENGINE_LUA();
        switch (type) {
            case LUA_TNONE:
                neko_assert(type == LUA_TNONE);
                break;
            case LUA_TBOOLEAN:
                lua_pushboolean(L, data.boolean);
                break;
            case LUA_TNUMBER:
                lua_pushnumber(L, data.number);
                break;
            case LUA_TSTRING: {
                // neko::string s = luax_check_string(L, arg);
                // data.string = to_cstr(s);
                lua_pushstring(L, data.str);
                break;
            }
        }
    }

    void sync() {
        neko_assert(cname.data != NULL);

        lua_State *L = ENGINE_LUA();

        int n = lua_gettop(L);

        lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);  // # 1
        lua_getiuservalue(L, -1, W_LUA_UPVALUES::NEKO_W_COMPONENTS_NAME);  // # 2
        lua_getfield(L, -1, W_LUA_REGISTRY_CONST::CVAR_MAP);               // # 3
        lua_pushinteger(L, neko_hash_str(cname.data));                     // 使用 32 位哈希以适应 Lua 数字范围
        lua_gettable(L, -2);                                               // # 4

        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "type");  // # 5
            this->type = lua_tointeger(L, -1);
            lua_pop(L, 1);  // # pop 5

            lua_getfield(L, -1, "data");  // # 5
            switch (type) {
                case LUA_TNONE:
                    neko_assert(type == LUA_TNONE);
                    break;
                case LUA_TBOOLEAN:
                    data.boolean = lua_toboolean(L, -1);
                    break;
                case LUA_TNUMBER:
                    data.number = lua_tonumber(L, -1);
                    break;
                case LUA_TSTRING: {
                    // neko::string s = luax_check_string(L, arg);
                    // data.string = to_cstr(s);
                    data.str = lua_tostring(L, -1);
                    break;
                }
            }
            lua_pop(L, 1);  // # pop 5
        } else {
            console_log("cvar sync with no value : %s", cname.data);
        }

        lua_pop(L, 4);

        if (type == LUA_TNONE) {
            console_log("cvar sync with no type : %s", cname.data);
        }

        neko_assert(lua_gettop(L) == n);
    }

    template <typename C>
    auto get() -> C {
        using TT = std::decay_t<C>;
        if (type == LUA_TNONE) {
            console_log("trying to neko_w_lua_variant::get(%s) with LUA_TNONE", cname.data);
        }
        if constexpr (std::is_same_v<TT, i32> || std::is_same_v<TT, u32> || std::is_same_v<TT, f32> || std::is_same_v<TT, f64>) {
            neko_assert(type == LUA_TNUMBER);
            return data.number;
        } else if constexpr (std::is_same_v<TT, const_str>) {
            neko_assert(type == LUA_TSTRING);
            return data.str;
        } else if constexpr (std::is_same_v<TT, bool>) {
            neko_assert(type == LUA_TBOOLEAN);
            return data.boolean;
        } else {
            static_assert("unsupported type for neko_w_lua_variant::get");
        }
    }

    void make() {
        neko_assert(type != LUA_TNONE);

        lua_State *L = ENGINE_LUA();

        int n = lua_gettop(L);

        lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);  // # 1

        lua_getiuservalue(L, -1, NEKO_W_COMPONENTS_NAME);  // # 2
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, W_LUA_REGISTRY_CONST::CVAR_MAP);  // # 3
            if (lua_istable(L, -1)) {
                lua_pushinteger(L, neko_hash_str(cname.data));  // 使用 32 位哈希以适应 Lua 数字范围

                lua_gettable(L, -2);       // # 4
                if (lua_istable(L, -1)) {  // 如果表存在 修改字段
                    lua_pushinteger(L, type);
                    lua_setfield(L, -2, "type");
                    lua_pushstring(L, cname.data);
                    lua_setfield(L, -2, "name");
                    push();
                    lua_setfield(L, -2, "data");
                } else {            // 如果表不存在 创建一个新表并设置字段
                    lua_pop(L, 1);  // 弹出非表值

                    lua_createtable(L, 0, 2);
                    lua_pushinteger(L, type);
                    lua_setfield(L, -2, "type");
                    lua_pushstring(L, cname.data);
                    lua_setfield(L, -2, "name");
                    push();
                    lua_setfield(L, -2, "data");

                    lua_pushinteger(L, neko_hash_str(cname.data));  // 重新推入键
                    lua_pushvalue(L, -2);                           // 将新表复制到栈顶
                    lua_settable(L, -4);                            // 将新表设置到 CVAR_MAP 中
                }

                lua_pop(L, 1);  // # pop 4

                lua_pop(L, 1);  // # pop 3
            } else {
                console_log("%s", "failed to get W_LUA_REGISTRY_NAME::CVAR_MAP");
                lua_pop(L, 1);  // # pop 3
            }
            lua_pop(L, 1);  // # pop 2
        } else {
            console_log("%s", "failed to get upvalue NEKO_W_COMPONENTS_NAME");
            lua_pop(L, 1);  // # pop 2
        }

        lua_pop(L, 1);  // # pop 1

        neko_assert(lua_gettop(L) == n);
    }
};

static_assert(std::is_trivially_copyable_v<neko_w_lua_variant<f64>>);

#define CVAR(name, V) neko_w_lua_variant<decltype(V)> name(#name, V)
#define CVAR_REF(name, T)              \
    neko_w_lua_variant<T> name(#name); \
    name.sync()

#endif

#endif

#ifndef NEKO_LUA_REF_HPP
#define NEKO_LUA_REF_HPP

#include <string>
#include <tuple>  // std::ignore

namespace neko {

struct LuaNil {};

template <>
struct luabind::LuaStack<LuaNil> {
    static inline void push(lua_State *L, LuaNil const &any) { lua_pushnil(L); }
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
struct neko::luabind::LuaStack<LuaTableElement<K>> {
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

#ifndef NEKO_LUA_CUSTOM_TYPES_HPP
#define NEKO_LUA_CUSTOM_TYPES_HPP

extern std::unordered_map<std::type_index, std::string> usertypeNames;
int userdata_destructor(lua_State *L);

class Userdata {
public:
    virtual ~Userdata(){};
    virtual const std::string &getTypeName() const = 0;
};

class Bytearray : public Userdata {
    std::vector<u8> buffer;

public:
    Bytearray(size_t capacity);
    Bytearray(std::vector<u8> buffer);
    virtual ~Bytearray();

    const std::string &getTypeName() const override { return TYPENAME; }
    inline std::vector<u8> &data() { return buffer; }

    static int createMetatable(lua_State *);
    inline static std::string TYPENAME = "bytearray";
};

namespace lua {
template <lua_CFunction func>
int wrap(lua_State *L) {
    int result = 0;
    try {
        result = func(L);
    }
    // transform exception with description into lua_error
    catch (std::exception &e) {
        luaL_error(L, e.what());
    }
    // Rethrow any other exception (lua error for example)
    catch (...) {
        throw;
    }
    return result;
}
}  // namespace lua

inline bool getglobal(lua_State *L, const std::string &name) {
    lua_getglobal(L, name.c_str());
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

template <class T>
inline T *touserdata(lua_State *L, int idx) {
    if (void *rawptr = lua_touserdata(L, idx)) {
        return static_cast<T *>(rawptr);
    }
    return nullptr;
}
template <class T, typename... Args>
inline int newuserdata(lua_State *L, Args &&...args) {
    const auto &found = usertypeNames.find(typeid(T));
    void *ptr = lua_newuserdata(L, sizeof(T));
    new (ptr) T(args...);

    if (found == usertypeNames.end()) {
        console_log(std::string("usertype is not registred: " + std::string(typeid(T).name())).c_str());
    } else if (getglobal(L, found->second)) {
        lua_setmetatable(L, -2);
    }
    return 1;
}

template <class T, lua_CFunction func>
inline void newusertype(lua_State *L, const std::string &name) {
    usertypeNames[typeid(T)] = name;
    func(L);

    lua_pushcfunction(L, userdata_destructor);
    lua_setfield(L, -2, "__gc");

    lua_setglobal(L, name.c_str());
}

class LuaVector final : public std::vector<float> {
public:
    static constexpr const char *LUA_TYPE_NAME = "vector";

    static void RegisterMetaTable(lua_State *L);
    static LuaVector *CheckArg(lua_State *L, int arg);

private:
    static int New(lua_State *L);
    static int GarbageCollect(lua_State *L);
    static int ToString(lua_State *L);
    static int Index(lua_State *L);
    static int NewIndex(lua_State *L);
    static int Len(lua_State *L);
    static int Append(lua_State *L);
    static int Pop(lua_State *L);
    static int Extend(lua_State *L);
    static int Insert(lua_State *L);
    static int Erase(lua_State *L);
    static int Sort(lua_State *L);
};

#endif

#ifndef NEKO_LUABIND
#define NEKO_LUABIND

#include <bit>
#include <map>
#include <string>
#include <vector>

#include "engine/base.hpp"

namespace std {
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
}  // namespace std

namespace neko::lua {
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
}  // namespace neko::lua

#define DEFINE_LUAOPEN(name)                                                           \
    int luaopen_neko_##name(lua_State *L) { return neko::lua::__##name ::luaopen(L); } \
    static ::neko::lua::callfunc __init_##name(::neko::lua::register_module, "__neko." #name, luaopen_neko_##name);

#define DEFINE_LUAOPEN_EXTERN(name) \
    namespace neko::lua::__##name { \
        int luaopen(lua_State *L);  \
    }                               \
    DEFINE_LUAOPEN(name)

namespace neko::lua {
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

template <typename T>
void checktable_refl(lua_State *L, const_str tname, T &&v) {

#define FUCK_TYPES() i32, u32, bool, f32, bool, const_str, String

    if (lua_getfield(L, -1, tname) == LUA_TNIL) {
        console_log("[exception] no %s table", tname);
    }
    if (lua_istable(L, -1)) {
        auto f = [&L](std::string_view name, neko::reflection::Any &value) {
            static_assert(std::is_lvalue_reference_v<decltype(value)>);
            if (lua_getfield(L, -1, std::string(name).c_str()) != LUA_TNIL) {
                auto ff = [&]<typename S>(const_str name, neko::reflection::Any &var, S &t) {
                    if (value.GetType() == neko::reflection::type_of<S>()) {
                        S s = neko_lua_to<std::remove_reference_t<S>>(L, -1);
                        value.cast<S>() = s;
                    }
                };
                std::apply([&](auto &&...args) { (ff(std::string(name).c_str(), value, args), ...); }, std::tuple<FUCK_TYPES()>());
            }
            lua_pop(L, 1);
        };
        v.foreach (f);
    } else {
        console_log("[exception] no %s table", tname);
    }
    lua_pop(L, 1);
}

}  // namespace neko::lua

// lua2struct

namespace lua2struct {
namespace reflection {
template <unsigned short N>
struct cstring {
    constexpr explicit cstring(std::string_view str) noexcept : cstring{str, std::make_integer_sequence<unsigned short, N>{}} {}
    constexpr const char *data() const noexcept { return chars_; }
    constexpr unsigned short size() const noexcept { return N; }
    constexpr operator std::string_view() const noexcept { return {data(), size()}; }
    template <unsigned short... I>
    constexpr cstring(std::string_view str, std::integer_sequence<unsigned short, I...>) noexcept : chars_{str[I]..., '\0'} {}
    char chars_[static_cast<size_t>(N) + 1];
};
template <>
struct cstring<0> {
    constexpr explicit cstring(std::string_view) noexcept {}
    constexpr const char *data() const noexcept { return nullptr; }
    constexpr unsigned short size() const noexcept { return 0; }
    constexpr operator std::string_view() const noexcept { return {}; }
};

template <typename T>
struct wrapper {
    T a;
};
template <typename T>
wrapper(T) -> wrapper<T>;

template <typename T>
static inline T storage = {};

template <auto T>
constexpr auto main_name_of_pointer() {
    constexpr auto is_identifier = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; };
#if __GNUC__ && (!__clang__) && (!_MSC_VER)
    std::string_view str = __PRETTY_FUNCTION__;
    std::size_t start = str.rfind("::") + 2;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#elif __clang__
    std::string_view str = __PRETTY_FUNCTION__;
    std::size_t start = str.rfind(".") + 1;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#elif _MSC_VER
    std::string_view str = __FUNCSIG__;
    std::size_t start = str.rfind("->") + 2;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#else
    static_assert(false, "Not supported compiler");
#endif
}

struct any {
    constexpr any(int) {}
    template <typename T>
        requires std::is_copy_constructible_v<T>
    operator T &();
    template <typename T>
        requires std::is_move_constructible_v<T>
    operator T &&();
    template <typename T>
        requires(!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>)
    operator T();
};

template <typename T, std::size_t N>
constexpr std::size_t try_initialize_with_n() {
    return []<std::size_t... Is>(std::index_sequence<Is...>) { return requires { T{any(Is)...}; }; }(std::make_index_sequence<N>{});
}

template <typename T, std::size_t N = 0>
constexpr auto field_count_impl() {
    if constexpr (try_initialize_with_n<T, N>() && !try_initialize_with_n<T, N + 1>()) {
        return N;
    } else {
        return field_count_impl<T, N + 1>();
    }
}

template <typename T>
    requires std::is_aggregate_v<T>
static constexpr auto field_count = field_count_impl<T>();

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4101)
#endif

template <typename T, std::size_t I>
constexpr auto field_type_impl(T object) {
    constexpr auto N = field_count<T>;
    if constexpr (N == 0) {
        static_assert(N != 0, "the object has no fields");
    } else if constexpr (N == 1) {
        auto [v0] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0)>>>{};
    } else if constexpr (N == 2) {
        auto [v0, v1] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1)>>>{};
    } else if constexpr (N == 3) {
        auto [v0, v1, v2] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2)>>>{};
    } else if constexpr (N == 4) {
        auto [v0, v1, v2, v3] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3)>>>{};
    } else if constexpr (N == 5) {
        auto [v0, v1, v2, v3, v4] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4)>>>{};
    } else if constexpr (N == 6) {
        auto [v0, v1, v2, v3, v4, v5] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5)>>>{};
    } else if constexpr (N == 7) {
        auto [v0, v1, v2, v3, v4, v5, v6] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6)>>>{};
    } else if constexpr (N == 8) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7)>>>{};
    } else if constexpr (N == 9) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8)>>>{};
    } else if constexpr (N == 10) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9] = object;
        return std::type_identity<
                std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8), decltype(v9)>>>{};
    } else if constexpr (N == 11) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = object;
        return std::type_identity<std::tuple_element_t<
                I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8), decltype(v9), decltype(v10)>>>{};
    } else if constexpr (N == 12) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8),
                                                                     decltype(v9), decltype(v10), decltype(v11)>>>{};
    } else if constexpr (N == 13) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8),
                                                                     decltype(v9), decltype(v10), decltype(v11), decltype(v12)>>>{};
    } else if constexpr (N == 14) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8),
                                                                     decltype(v9), decltype(v10), decltype(v11), decltype(v12), decltype(v13)>>>{};
    } else if constexpr (N == 15) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8),
                                                                     decltype(v9), decltype(v10), decltype(v11), decltype(v12), decltype(v13), decltype(v14)>>>{};
    } else if constexpr (N == 16) {
        auto [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = object;
        return std::type_identity<std::tuple_element_t<I, std::tuple<decltype(v0), decltype(v1), decltype(v2), decltype(v3), decltype(v4), decltype(v5), decltype(v6), decltype(v7), decltype(v8),
                                                                     decltype(v9), decltype(v10), decltype(v11), decltype(v12), decltype(v13), decltype(v14), decltype(v15)>>>{};
    } else {
        static_assert(N <= 16, "the max of supported fields is 16");
    }
}

template <std::size_t I>
constexpr auto &&field_access(auto &&object) {
    using T = std::remove_cvref_t<decltype(object)>;
    constexpr auto N = field_count<T>;
    if constexpr (N == 0) {
        static_assert(N != 0, "the object has no fields");
    } else if constexpr (N == 1) {
        auto &&[v0] = object;
        return std::get<I>(std::forward_as_tuple(v0));
    } else if constexpr (N == 2) {
        auto &&[v0, v1] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1));
    } else if constexpr (N == 3) {
        auto &&[v0, v1, v2] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2));
    } else if constexpr (N == 4) {
        auto &&[v0, v1, v2, v3] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3));
    } else if constexpr (N == 5) {
        auto &&[v0, v1, v2, v3, v4] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4));
    } else if constexpr (N == 6) {
        auto &&[v0, v1, v2, v3, v4, v5] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5));
    } else if constexpr (N == 7) {
        auto &&[v0, v1, v2, v3, v4, v5, v6] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6));
    } else if constexpr (N == 8) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7));
    } else if constexpr (N == 9) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8));
    } else if constexpr (N == 10) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9));
    } else if constexpr (N == 11) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10));
    } else if constexpr (N == 12) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11));
    } else if constexpr (N == 13) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12));
    } else if constexpr (N == 14) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13));
    } else if constexpr (N == 15) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14));
    } else if constexpr (N == 16) {
        auto &&[v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = object;
        return std::get<I>(std::forward_as_tuple(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15));
    } else {
        static_assert(N <= 16, "the max of supported fields is 16");
    }
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

template <typename T, std::size_t I>
constexpr auto field_name_impl() noexcept {
    constexpr auto name = main_name_of_pointer<wrapper{&field_access<I>(storage<T>)}>();
    return cstring<name.size()>{name};
}

template <typename T, std::size_t I>
    requires std::is_aggregate_v<T>
using field_type = typename decltype(field_type_impl<T, I>(std::declval<T>()))::type;

template <typename T, std::size_t I>
    requires std::is_aggregate_v<T>
static constexpr auto field_name = field_name_impl<T, I>();
}  // namespace reflection

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

template <typename T>
T unpack(lua_State *L, int arg);

template <typename T, std::size_t I>
void unpack_struct(lua_State *L, int arg, T &v);

template <typename T>
    requires(std::is_integral_v<T> && !std::same_as<T, bool>)
T unpack(lua_State *L, int arg) {
    lua_Integer r = luaL_checkinteger(L, arg);
    if constexpr (std::is_same_v<T, lua_Integer>) {
        return r;
    } else if constexpr (sizeof(T) >= sizeof(lua_Integer)) {
        return static_cast<T>(r);
    } else {
        if (check_integral_limit<T>(r)) {
            return static_cast<T>(r);
        }
        luaL_error(L, "unpack integer limit exceeded", arg);
    }
    neko_assert(false);
    return T{};
}

template <typename T>
    requires(std::same_as<T, bool>)
T unpack(lua_State *L, int arg) {
    luaL_checktype(L, arg, LUA_TBOOLEAN);
    return !!lua_toboolean(L, arg);
}

template <typename T>
    requires std::is_pointer_v<T>
T unpack(lua_State *L, int arg) {
    luaL_checktype(L, arg, LUA_TLIGHTUSERDATA);
    return static_cast<T>(lua_touserdata(L, arg));
}

template <>
inline float unpack<float>(lua_State *L, int arg) {
    return (float)luaL_checknumber(L, arg);
}

template <>
inline std::string unpack<std::string>(lua_State *L, int arg) {
    size_t sz = 0;
    const char *str = luaL_checklstring(L, arg, &sz);
    return std::string(str, sz);
}

template <>
inline std::string_view unpack<std::string_view>(lua_State *L, int arg) {
    size_t sz = 0;
    const char *str = luaL_checklstring(L, arg, &sz);
    return std::string_view(str, sz);
}

template <typename T>
    requires std::is_aggregate_v<T>
T unpack(lua_State *L, int arg) {
    T v;
    unpack_struct<T, 0>(L, arg, v);
    return v;
}

template <template <typename...> class Template, typename Class>
struct is_instantiation : std::false_type {};
template <template <typename...> class Template, typename... Args>
struct is_instantiation<Template, Template<Args...>> : std::true_type {};
template <typename Class, template <typename...> class Template>
concept is_instantiation_of = is_instantiation<Template, Class>::value;

template <typename T>
    requires is_instantiation_of<T, std::map>
T unpack(lua_State *L, int arg) {
    arg = lua_absindex(L, arg);
    luaL_checktype(L, arg, LUA_TTABLE);
    T v;
    lua_pushnil(L);
    while (lua_next(L, arg)) {
        auto key = unpack<typename T::key_type>(L, -2);
        auto mapped = unpack<typename T::mapped_type>(L, -1);
        v.emplace(std::move(key), std::move(mapped));
        lua_pop(L, 1);
    }
    return v;
}

template <typename T>
    requires is_instantiation_of<T, std::vector>
T unpack(lua_State *L, int arg) {
    arg = lua_absindex(L, arg);
    luaL_checktype(L, arg, LUA_TTABLE);
    lua_Integer n = lua_len(L, arg);
    T v;
    v.reserve((size_t)n);
    for (lua_Integer i = 1; i <= n; ++i) {
        lua_geti(L, arg, i);
        auto value = unpack<typename T::value_type>(L, -1);
        v.emplace_back(std::move(value));
        lua_pop(L, 1);
    }
    return v;
}

template <typename T, std::size_t I>
void unpack_struct(lua_State *L, int arg, T &v) {
    if constexpr (I < reflection::field_count<T>) {
        constexpr auto name = reflection::field_name<T, I>;
        lua_getfield(L, arg, name.data());
        reflection::field_access<I>(v) = unpack<reflection::field_type<T, I>>(L, -1);
        lua_pop(L, 1);
        unpack_struct<T, I + 1>(L, arg, v);
    }
}

template <typename T>
void pack(lua_State *L, const T &v);

template <typename T, std::size_t I>
void pack_struct(lua_State *L, const T &v);

template <typename T>
    requires(std::is_integral_v<T> && !std::same_as<T, bool>)
void pack(lua_State *L, const T &v) {
    if constexpr (std::is_same_v<T, lua_Integer>) {
        lua_pushinteger(L, v);
    } else if constexpr (sizeof(T) <= sizeof(lua_Integer)) {
        lua_pushinteger(L, static_cast<lua_Integer>(v));
    } else {
        if (check_integral_limit<lua_Integer>(v)) {
            lua_pushinteger(L, static_cast<lua_Integer>(v));
        }
        luaL_error(L, "pack integer limit exceeded");
        neko_assert(false);
    }
}

template <typename T>
    requires(std::same_as<T, bool>)
void pack(lua_State *L, const T &v) {
    lua_pushboolean(L, v);
}

template <>
inline void pack<float>(lua_State *L, const float &v) {
    lua_pushnumber(L, (lua_Number)v);
}

template <typename T>
    requires std::is_aggregate_v<T>
void pack(lua_State *L, const T &v) {
    lua_createtable(L, 0, (int)reflection::field_count<T>);
    pack_struct<T, 0>(L, v);
}

template <typename T, std::size_t I>
void pack_struct(lua_State *L, const T &v) {
    if constexpr (I < reflection::field_count<T>) {
        constexpr auto name = reflection::field_name<T, I>;
        pack<reflection::field_type<T, I>>(L, reflection::field_access<I>(v));
        lua_setfield(L, -2, name.data());
        pack_struct<T, I + 1>(L, v);
    }
}
}  // namespace lua2struct

#endif