
#ifndef NEKO_LUA_H
#define NEKO_LUA_H

#include <atomic>
#include <initializer_list>

#include "engine/deps/luaalloc.h"
#include "neko_base.h"
#include "neko_os.h"
#include "neko_prelude.h"

// lua
#ifdef __cplusplus
extern "C" {
#endif
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#ifdef __cplusplus
}
#endif

#define LUA_FUNCTION(F) static int F(lua_State *L)
#define LUABIND_MODULE(...) static int luaopen(lua_State *L)

#define NEKO_LUA_AUTO_REGISTER_PREFIX "neko_luabind_"

void __neko_luabind_init(lua_State *L);
void __neko_luabind_fini(lua_State *L);

#define neko_luabind_type(L, type) neko_luabind_type_add(L, #type, sizeof(type))

enum { LUAA_INVALID_TYPE = -1 };

typedef lua_Integer neko_luabind_Type;
typedef int (*neko_luabind_Pushfunc)(lua_State *, neko_luabind_Type, const void *);
typedef void (*neko_luabind_Tofunc)(lua_State *, neko_luabind_Type, void *, int);

neko_luabind_Type neko_luabind_type_add(lua_State *L, const char *type, size_t size);
neko_luabind_Type neko_luabind_type_find(lua_State *L, const char *type);

const char *neko_luabind_typename(lua_State *L, neko_luabind_Type id);
size_t neko_luabind_typesize(lua_State *L, neko_luabind_Type id);

#define neko_luabind_push(L, type, c_in) neko_luabind_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_to(L, type, c_out, index) neko_luabind_to_type(L, neko_luabind_type(L, type), c_out, index)

#define neko_luabind_conversion(L, type, push_func, to_func) neko_luabind_conversion_type(L, neko_luabind_type(L, type), push_func, to_func);
#define neko_luabind_conversion_push(L, type, func) neko_luabind_conversion_push_type(L, neko_luabind_type(L, type), func)
#define neko_luabind_conversion_to(L, type, func) neko_luabind_conversion_to_type(L, neko_luabind_type(L, type), func)

#define neko_luabind_conversion_registered(L, type) neko_luabind_conversion_registered_type(L, neko_luabind_type(L, type));
#define neko_luabind_conversion_push_registered(L, type) neko_luabind_conversion_push_registered_typ(L, neko_luabind_type(L, type));
#define neko_luabind_conversion_to_registered(L, type) neko_luabind_conversion_to_registered_type(L, neko_luabind_type(L, type));

int neko_luabind_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
void neko_luabind_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

void neko_luabind_conversion_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Pushfunc push_func, neko_luabind_Tofunc to_func);
void neko_luabind_conversion_push_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Pushfunc func);
void neko_luabind_conversion_to_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Tofunc func);

bool neko_luabind_conversion_registered_type(lua_State *L, neko_luabind_Type type);
bool neko_luabind_conversion_push_registered_type(lua_State *L, neko_luabind_Type type);
bool neko_luabind_conversion_to_registered_type(lua_State *L, neko_luabind_Type type);

int neko_luabind_push_bool(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_char(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_signed_char(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_unsigned_char(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_short(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_unsigned_short(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_int(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_unsigned_int(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_long(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_unsigned_long(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_long_long(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_unsigned_long_long(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_float(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_double(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_long_double(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_char_ptr(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_const_char_ptr(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_void_ptr(lua_State *L, neko_luabind_Type, const void *c_in);
int neko_luabind_push_void(lua_State *L, neko_luabind_Type, const void *c_in);

void neko_luabind_to_bool(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_char(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_signed_char(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_unsigned_char(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_short(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_unsigned_short(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_int(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_unsigned_int(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_unsigned_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_long_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_unsigned_long_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_float(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_double(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_long_double(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_char_ptr(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_const_char_ptr(lua_State *L, neko_luabind_Type, void *c_out, int index);
void neko_luabind_to_void_ptr(lua_State *L, neko_luabind_Type, void *c_out, int index);

/*
** Structs
*/
#define LUAA_INVALID_MEMBER_NAME NULL

#define neko_luabind_struct(L, type) neko_luabind_struct_type(L, neko_luabind_type(L, type))
#define neko_luabind_struct_member(L, type, member, member_type) neko_luabind_struct_member_type(L, neko_luabind_type(L, type), #member, neko_luabind_type(L, member_type), offsetof(type, member))

#define neko_luabind_struct_push(L, type, c_in) neko_luabind_struct_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_struct_push_member(L, type, member, c_in) neko_luabind_struct_push_member_offset_type(L, neko_luabind_type(L, type), offsetof(type, member), c_in)
#define neko_luabind_struct_push_member_name(L, type, member, c_in) neko_luabind_struct_push_member_name_type(L, neko_luabind_type(L, type), member, c_in)

#define neko_luabind_struct_to(L, type, c_out, index) neko_luabind_struct_to_type(L, neko_luabind_type(L, type), c_out, index)
#define neko_luabind_struct_to_member(L, type, member, c_out, index) neko_luabind_struct_to_member_offset_type(L, neko_luabind_type(L, type), offsetof(type, member), c_out, index)
#define neko_luabind_struct_to_member_name(L, type, member, c_out, index) neko_luabind_struct_to_member_name_type(L, neko_luabind_type(L, type), member, c_out, index)

#define neko_luabind_struct_has_member(L, type, member) neko_luabind_struct_has_member_offset_type(L, neko_luabind_type(L, type), offsetof(type, member))
#define neko_luabind_struct_has_member_name(L, type, member) neko_luabind_struct_has_member_name_type(L, neko_luabind_type(L, type), member)

#define neko_luabind_struct_typeof_member(L, type, member) neko_luabind_struct_typeof_member_offset_type(L, neko_luabind_type(L, type), offsetof(type, member))
#define neko_luabind_struct_typeof_member_name(L, type, member) neko_luabind_struct_typeof_member_name_type(L, neko_luabind_type(L, type), member)

#define neko_luabind_struct_registered(L, type) neko_luabind_struct_registered_type(L, neko_luabind_type(L, type))
#define neko_luabind_struct_next_member_name(L, type, member) neko_luabind_struct_next_member_name_type(L, neko_luabind_type(L, type), member)

void neko_luabind_struct_type(lua_State *L, neko_luabind_Type type);
void neko_luabind_struct_member_type(lua_State *L, neko_luabind_Type type, const char *member, neko_luabind_Type member_type, size_t offset);

int neko_luabind_struct_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
int neko_luabind_struct_push_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset, const void *c_in);
int neko_luabind_struct_push_member_name_type(lua_State *L, neko_luabind_Type type, const char *member, const void *c_in);

void neko_luabind_struct_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);
void neko_luabind_struct_to_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset, void *c_out, int index);
void neko_luabind_struct_to_member_name_type(lua_State *L, neko_luabind_Type type, const char *member, void *c_out, int index);

bool neko_luabind_struct_has_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset);
bool neko_luabind_struct_has_member_name_type(lua_State *L, neko_luabind_Type type, const char *member);

neko_luabind_Type neko_luabind_struct_typeof_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset);
neko_luabind_Type neko_luabind_struct_typeof_member_name_type(lua_State *L, neko_luabind_Type type, const char *member);

bool neko_luabind_struct_registered_type(lua_State *L, neko_luabind_Type type);

const char *neko_luabind_struct_next_member_name_type(lua_State *L, neko_luabind_Type type, const char *member);

#define neko_luabind_enum(L, type) neko_luabind_enum_type(L, neko_luabind_type(L, type), sizeof(type))

#define neko_luabind_enum_value(L, type, value)                    \
    const type __neko_luabind_enum_value_temp_##value[] = {value}; \
    neko_luabind_enum_value_type(L, neko_luabind_type(L, type), __neko_luabind_enum_value_temp_##value, #value)

#define neko_luabind_enum_value_name(L, type, value, name)         \
    const type __neko_luabind_enum_value_temp_##value[] = {value}; \
    neko_luabind_enum_value_type(L, neko_luabind_type(L, type), __neko_luabind_enum_value_temp_##value, name)

#define neko_luabind_enum_push(L, type, c_in) neko_luabind_enum_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_enum_to(L, type, c_out, index) neko_luabind_enum_to_type(L, neko_luabind_type(L, type), c_out, index)

#define neko_luabind_enum_has_value(L, type, value)                \
    const type __neko_luabind_enum_value_temp_##value[] = {value}; \
    neko_luabind_enum_has_value_type(L, neko_luabind_type(L, type), __neko_luabind_enum_value_temp_##value)

#define neko_luabind_enum_has_name(L, type, name) neko_luabind_enum_has_name_type(L, neko_luabind_type(L, type), name)

#define neko_luabind_enum_registered(L, type) neko_luabind_enum_registered_type(L, neko_luabind_type(L, type))
#define neko_luabind_enum_next_value_name(L, type, member) neko_luabind_enum_next_value_name_type(L, neko_luabind_type(L, type), member)

void neko_luabind_enum_type(lua_State *L, neko_luabind_Type type, size_t size);
void neko_luabind_enum_value_type(lua_State *L, neko_luabind_Type type, const void *value, const char *name);

int neko_luabind_enum_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
void neko_luabind_enum_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

bool neko_luabind_enum_has_value_type(lua_State *L, neko_luabind_Type type, const void *value);
bool neko_luabind_enum_has_name_type(lua_State *L, neko_luabind_Type type, const char *name);

bool neko_luabind_enum_registered_type(lua_State *L, neko_luabind_Type type);
const char *neko_luabind_enum_next_value_name_type(lua_State *L, neko_luabind_Type type, const char *member);

#define neko_lua_register(FUNCTIONS)                              \
    for (unsigned i = 0; i < NEKO_ARR_SIZE(FUNCTIONS) - 1; ++i) { \
        lua_pushcfunction(L, FUNCTIONS[i].func);                  \
        lua_setglobal(L, FUNCTIONS[i].name);                      \
    }

bool neko_lua_equal(lua_State *state, int index1, int index2);

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name);
int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name);
void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name);
void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name);
int neko_lua_get_table_pairs_count(lua_State *L, int index);

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

namespace neko::lua {
void luax_run_bootstrap(lua_State *L);
}

i32 luax_require_script(lua_State *L, String filepath);

void luax_stack_dump(lua_State *L);

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

enum {
    LUAX_UD_TNAME = 1,
    LUAX_UD_PTR_SIZE = 2,
};

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

// extern impl
extern int luaopen_enet(lua_State *l);

#endif
