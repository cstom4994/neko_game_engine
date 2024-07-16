
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

/*
** Functions
*/

#define LUAA_EVAL(...) __VA_ARGS__

/* Join Three Strings */
#define LUAA_JOIN2(X, Y) X##Y
#define LUAA_JOIN3(X, Y, Z) X##Y##Z

/* workaround for MSVC VA_ARGS expansion */
#define LUAA_APPLY(FUNC, ARGS) LUAA_EVAL(FUNC ARGS)

/* Argument Counter */
#define LUAA_COUNT(...) LUAA_COUNT_COLLECT(_, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define LUAA_COUNT_COLLECT(...) LUAA_COUNT_SHIFT(__VA_ARGS__)
#define LUAA_COUNT_SHIFT(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _N, ...) _N

/* Detect Void */
#define LUAA_VOID(X) LUAA_JOIN2(LUAA_VOID_, X)
#define LUAA_VOID_void
#define LUAA_CHECK_N(X, N, ...) N
#define LUAA_CHECK(...) LUAA_CHECK_N(__VA_ARGS__, , )
#define LUAA_SUFFIX(X) LUAA_SUFFIX_CHECK(LUAA_VOID(X))
#define LUAA_SUFFIX_CHECK(X) LUAA_CHECK(LUAA_JOIN2(LUAA_SUFFIX_, X))
#define LUAA_SUFFIX_ ~, _void,

/* Declaration and Register Macros */
#define LUAA_DECLARE(func, ret_t, count, suffix, ...) LUAA_APPLY(LUAA_JOIN3(neko_luabind_function_declare, count, suffix), (func, ret_t, ##__VA_ARGS__))
// #define LUAA_DECLARE(func, ret_t, count, suffix, ...) LUAA_APPLY(LUAA_JOIN3(neko_luabind_function_declare, count, suffix), (func, ret_t, ##__VA_ARGS__))
#define LUAA_REGISTER(L, func, ret_t, count, ...) LUAA_APPLY(LUAA_JOIN2(neko_luabind_function_register, count), (L, func, ret_t, ##__VA_ARGS__))

#define neko_luabind_function_declare0(func, ret_t) \
    void __neko_luabind_##func(void *out, void *args) { *(ret_t *)out = func(); }

#define neko_luabind_function_declare0_void(func, ret_t) \
    void __neko_luabind_##func(void *out, void *args) { func(); }

#define neko_luabind_function_declare1(func, ret_t, arg0_t) \
    void __neko_luabind_##func(void *out, void *args) {     \
        arg0_t a0 = *(arg0_t *)args;                        \
        *(ret_t *)out = func(a0);                           \
    }

#define neko_luabind_function_declare1_void(func, ret_t, arg0_t) \
    void __neko_luabind_##func(void *out, void *args) {          \
        arg0_t a0 = *(arg0_t *)args;                             \
        func(a0);                                                \
    }

#define neko_luabind_function_declare2(func, ret_t, arg0_t, arg1_t) \
    void __neko_luabind_##func(void *out, void *args) {             \
        arg0_t a0 = *(arg0_t *)args;                                \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));             \
        *(ret_t *)out = func(a0, a1);                               \
    }

#define neko_luabind_function_declare2_void(func, ret_t, arg0_t, arg1_t) \
    void __neko_luabind_##func(void *out, void *args) {                  \
        arg0_t a0 = *(arg0_t *)args;                                     \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                  \
        func(a0, a1);                                                    \
    }

#define neko_luabind_function_declare3(func, ret_t, arg0_t, arg1_t, arg2_t) \
    void __neko_luabind_##func(void *out, void *args) {                     \
        arg0_t a0 = *(arg0_t *)args;                                        \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                     \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));    \
        *(ret_t *)out = func(a0, a1, a2);                                   \
    }

#define neko_luabind_function_declare3_void(func, ret_t, arg0_t, arg1_t, arg2_t) \
    void __neko_luabind_##func(void *out, void *args) {                          \
        arg0_t a0 = *(arg0_t *)args;                                             \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                          \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));         \
        func(a0, a1, a2);                                                        \
    }

#define neko_luabind_function_declare4(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)       \
    void __neko_luabind_##func(void *out, void *args) {                                   \
        arg0_t a0 = *(arg0_t *)args;                                                      \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                   \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                  \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3);                                             \
    }

#define neko_luabind_function_declare4_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)  \
    void __neko_luabind_##func(void *out, void *args) {                                   \
        arg0_t a0 = *(arg0_t *)args;                                                      \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                   \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                  \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t)); \
        func(a0, a1, a2, a3);                                                             \
    }

#define neko_luabind_function_declare5(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)                \
    void __neko_luabind_##func(void *out, void *args) {                                                    \
        arg0_t a0 = *(arg0_t *)args;                                                                       \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                    \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                   \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                  \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4);                                                          \
    }

#define neko_luabind_function_declare5_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)           \
    void __neko_luabind_##func(void *out, void *args) {                                                    \
        arg0_t a0 = *(arg0_t *)args;                                                                       \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                    \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                   \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                  \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t)); \
        func(a0, a1, a2, a3, a4);                                                                          \
    }

#define neko_luabind_function_declare6(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                         \
    void __neko_luabind_##func(void *out, void *args) {                                                                     \
        arg0_t a0 = *(arg0_t *)args;                                                                                        \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                     \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                    \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                   \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                  \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5);                                                                       \
    }

#define neko_luabind_function_declare6_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                    \
    void __neko_luabind_##func(void *out, void *args) {                                                                     \
        arg0_t a0 = *(arg0_t *)args;                                                                                        \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                     \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                    \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                   \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                  \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t)); \
        func(a0, a1, a2, a3, a4, a5);                                                                                       \
    }

#define neko_luabind_function_declare7(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                  \
    void __neko_luabind_##func(void *out, void *args) {                                                                                      \
        arg0_t a0 = *(arg0_t *)args;                                                                                                         \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                      \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                     \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                    \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                   \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                  \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6);                                                                                    \
    }

#define neko_luabind_function_declare7_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                             \
    void __neko_luabind_##func(void *out, void *args) {                                                                                      \
        arg0_t a0 = *(arg0_t *)args;                                                                                                         \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                      \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                     \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                    \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                   \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                  \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t)); \
        func(a0, a1, a2, a3, a4, a5, a6);                                                                                                    \
    }

#define neko_luabind_function_declare8(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                           \
    void __neko_luabind_##func(void *out, void *args) {                                                                                                       \
        arg0_t a0 = *(arg0_t *)args;                                                                                                                          \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                       \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                      \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                     \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                    \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                   \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                  \
        arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6, a7);                                                                                                 \
    }

#define neko_luabind_function_declare8_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                      \
    void __neko_luabind_##func(void *out, void *args) {                                                                                                       \
        arg0_t a0 = *(arg0_t *)args;                                                                                                                          \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                       \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                      \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                     \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                    \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                   \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                  \
        arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t)); \
        func(a0, a1, a2, a3, a4, a5, a6, a7);                                                                                                                 \
    }

#define neko_luabind_function_declare9(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                    \
    void __neko_luabind_##func(void *out, void *args) {                                                                                                                        \
        arg0_t a0 = *(arg0_t *)args;                                                                                                                                           \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                                        \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                                       \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                                      \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                                     \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                                    \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                                   \
        arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t));                  \
        arg8_t a8 = *(arg8_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t) + sizeof(arg7_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6, a7, a8);                                                                                                              \
    }

#define neko_luabind_function_declare9_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                               \
    void __neko_luabind_##func(void *out, void *args) {                                                                                                                        \
        arg0_t a0 = *(arg0_t *)args;                                                                                                                                           \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                                        \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                                       \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                                      \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                                     \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                                    \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                                   \
        arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t));                  \
        arg8_t a8 = *(arg8_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t) + sizeof(arg7_t)); \
        func(a0, a1, a2, a3, a4, a5, a6, a7, a8);                                                                                                                              \
    }

#define neko_luabind_function_declare10(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                            \
    void __neko_luabind_##func(void *out, void *args) {                                                                                                                                         \
        arg0_t a0 = *(arg0_t *)args;                                                                                                                                                            \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                                                         \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                                                        \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                                                       \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                                                      \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                                                     \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                                                    \
        arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t));                                   \
        arg8_t a8 = *(arg8_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t) + sizeof(arg7_t));                  \
        arg9_t a9 = *(arg9_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t) + sizeof(arg7_t) + sizeof(arg8_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);                                                                                                                           \
    }

#define neko_luabind_function_declare10_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                       \
    void __neko_luabind_##func(void *out, void *args) {                                                                                                                                         \
        arg0_t a0 = *(arg0_t *)args;                                                                                                                                                            \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                                                         \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                                                        \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                                                       \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                                                      \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                                                     \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                                                    \
        arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t));                                   \
        arg8_t a8 = *(arg8_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t) + sizeof(arg7_t));                  \
        arg9_t a9 = *(arg9_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t) + sizeof(arg7_t) + sizeof(arg8_t)); \
        func(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);                                                                                                                                           \
    }

#define neko_luabind_function_register0(L, func, ret_t) neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 0)

#define neko_luabind_function_register1(L, func, ret_t, arg0_t) neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 1, neko_luabind_type(L, arg0_t))

#define neko_luabind_function_register2(L, func, ret_t, arg0_t, arg1_t) \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 2, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t))

#define neko_luabind_function_register3(L, func, ret_t, arg0_t, arg1_t, arg2_t) \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 3, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t), neko_luabind_type(L, arg2_t))

#define neko_luabind_function_register4(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)                                                                                    \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 4, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t), \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t))

#define neko_luabind_function_register5(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)                                                                            \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 5, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t), \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t), neko_luabind_type(L, arg4_t))

#define neko_luabind_function_register6(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                                                                    \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 6, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t), \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t), neko_luabind_type(L, arg4_t), neko_luabind_type(L, arg5_t))

#define neko_luabind_function_register7(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                                            \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 7, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t), \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t), neko_luabind_type(L, arg4_t), neko_luabind_type(L, arg5_t), neko_luabind_type(L, arg6_t))

#define neko_luabind_function_register8(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                                                       \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 8, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t),                    \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t), neko_luabind_type(L, arg4_t), neko_luabind_type(L, arg5_t), neko_luabind_type(L, arg6_t), \
                                        neko_luabind_type(L, arg7_t))

#define neko_luabind_function_register9(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                               \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 9, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t),                    \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t), neko_luabind_type(L, arg4_t), neko_luabind_type(L, arg5_t), neko_luabind_type(L, arg6_t), \
                                        neko_luabind_type(L, arg7_t), neko_luabind_type(L, arg8_t))

#define neko_luabind_function_register10(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                      \
    neko_luabind_function_register_type(L, func, __neko_luabind_##func, #func, neko_luabind_type(L, ret_t), 10, neko_luabind_type(L, arg0_t), neko_luabind_type(L, arg1_t),                   \
                                        neko_luabind_type(L, arg2_t), neko_luabind_type(L, arg3_t), neko_luabind_type(L, arg4_t), neko_luabind_type(L, arg5_t), neko_luabind_type(L, arg6_t), \
                                        neko_luabind_type(L, arg7_t), neko_luabind_type(L, arg8_t), neko_luabind_type(L, arg9_t))

#define neko_luabind_function(L, func, ret_t, ...)             \
    neko_luabind_function_declare(func, ret_t, ##__VA_ARGS__); \
    neko_luabind_function_register(L, func, ret_t, ##__VA_ARGS__)
#define neko_luabind_function_declare(func, ret_t, ...) LUAA_DECLARE(func, ret_t, LUAA_COUNT(__VA_ARGS__), LUAA_SUFFIX(ret_t), ##__VA_ARGS__)
#define neko_luabind_function_register(L, func, ret_t, ...) LUAA_REGISTER(L, func, ret_t, LUAA_COUNT(__VA_ARGS__), ##__VA_ARGS__)

enum { LUAA_RETURN_STACK_SIZE = 256, LUAA_ARGUMENT_STACK_SIZE = 2048 };

typedef void (*neko_luabind_Func)(void *, void *);

int neko_luabind_call(lua_State *L, void *func_ptr);
int neko_luabind_call_name(lua_State *L, const char *func_name);

void neko_luabind_function_register_type(lua_State *L, void *src_func, neko_luabind_Func auto_func, const char *name, neko_luabind_Type ret_tid, int num_args, ...);

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

#define LUASTRUCT_REQUIRED 1
#define LUASTRUCT_OPTIONAL 0

#define IS_STRUCT(L, index, type) LUASTRUCT_is(L, #type, index)
#define CHECK_STRUCT(L, index, type) ((type *)LUASTRUCT_todata(L, #type, index, LUASTRUCT_REQUIRED))
#define OPTIONAL_STRUCT(L, index, type) ((type *)LUASTRUCT_todata(L, #type, index, LUASTRUCT_OPTIONAL))

#define PUSH_STRUCT(L, type, value)            \
    do {                                       \
        LUASTRUCT_new(L, #type, sizeof(type)); \
        *CHECK_STRUCT(L, -1, type) = (value);  \
    } while (0)

int LUASTRUCT_new(lua_State *L, const char *metatable, size_t size);
int LUASTRUCT_newref(lua_State *L, const char *metatable, int parentIndex, const void *data);
int LUASTRUCT_is(lua_State *L, const char *metatable, int index);
void *LUASTRUCT_todata(lua_State *L, const char *metatable, int index, int required);

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

#endif
