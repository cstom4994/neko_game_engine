
#ifndef NEKO_LUA_BASE_H
#define NEKO_LUA_BASE_H

#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "engine/common/neko_types.h"
#include "libs/lua/lua.hpp"

#ifndef _WIN32
#include <stdint.h>
#define SPRINTF_F snprintf
#else

#define SPRINTF_F _snprintf_s

struct strtoll_tool_t {
    static long do_strtoll(const char *s, const char *, int) { return atol(s); }
};
#define strtoll strtoll_tool_t::do_strtoll
#define strtoull (unsigned long)strtoll_tool_t::do_strtoll

#endif

#pragma region LuaA

#define NEKO_LUA_AUTO_REGISTER_PREFIX "neko_lua_auto_"

void __neko_lua_auto_open(lua_State *L);
void __neko_lua_auto_close(lua_State *L);

#define neko_lua_auto_type(L, type) neko_lua_auto_type_add(L, #type, sizeof(type))

enum { LUAA_INVALID_TYPE = -1 };

typedef lua_Integer neko_lua_auto_Type;
typedef int (*neko_lua_auto_Pushfunc)(lua_State *, neko_lua_auto_Type, const void *);
typedef void (*neko_lua_auto_Tofunc)(lua_State *, neko_lua_auto_Type, void *, int);

neko_lua_auto_Type neko_lua_auto_type_add(lua_State *L, const char *type, size_t size);
neko_lua_auto_Type neko_lua_auto_type_find(lua_State *L, const char *type);

const char *neko_lua_auto_typename(lua_State *L, neko_lua_auto_Type id);
size_t neko_lua_auto_typesize(lua_State *L, neko_lua_auto_Type id);

#define neko_lua_auto_push(L, type, c_in) neko_lua_auto_push_type(L, neko_lua_auto_type(L, type), c_in)
#define neko_lua_auto_to(L, type, c_out, index) neko_lua_auto_to_type(L, neko_lua_auto_type(L, type), c_out, index)

#define neko_lua_auto_conversion(L, type, push_func, to_func) neko_lua_auto_conversion_type(L, neko_lua_auto_type(L, type), push_func, to_func);
#define neko_lua_auto_conversion_push(L, type, func) neko_lua_auto_conversion_push_type(L, neko_lua_auto_type(L, type), func)
#define neko_lua_auto_conversion_to(L, type, func) neko_lua_auto_conversion_to_type(L, neko_lua_auto_type(L, type), func)

#define neko_lua_auto_conversion_registered(L, type) neko_lua_auto_conversion_registered_type(L, neko_lua_auto_type(L, type));
#define neko_lua_auto_conversion_push_registered(L, type) neko_lua_auto_conversion_push_registered_typ(L, neko_lua_auto_type(L, type));
#define neko_lua_auto_conversion_to_registered(L, type) neko_lua_auto_conversion_to_registered_type(L, neko_lua_auto_type(L, type));

int neko_lua_auto_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in);
void neko_lua_auto_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index);

void neko_lua_auto_conversion_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Pushfunc push_func, neko_lua_auto_Tofunc to_func);
void neko_lua_auto_conversion_push_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Pushfunc func);
void neko_lua_auto_conversion_to_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Tofunc func);

bool neko_lua_auto_conversion_registered_type(lua_State *L, neko_lua_auto_Type type);
bool neko_lua_auto_conversion_push_registered_type(lua_State *L, neko_lua_auto_Type type);
bool neko_lua_auto_conversion_to_registered_type(lua_State *L, neko_lua_auto_Type type);

int neko_lua_auto_push_bool(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_char(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_signed_char(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_unsigned_char(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_short(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_unsigned_short(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_int(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_unsigned_int(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_unsigned_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_long_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_unsigned_long_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_float(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_double(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_long_double(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_char_ptr(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_const_char_ptr(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_void_ptr(lua_State *L, neko_lua_auto_Type, const void *c_in);
int neko_lua_auto_push_void(lua_State *L, neko_lua_auto_Type, const void *c_in);

void neko_lua_auto_to_bool(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_char(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_signed_char(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_unsigned_char(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_short(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_unsigned_short(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_int(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_unsigned_int(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_unsigned_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_long_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_unsigned_long_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_float(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_double(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_long_double(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_char_ptr(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_const_char_ptr(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
void neko_lua_auto_to_void_ptr(lua_State *L, neko_lua_auto_Type, void *c_out, int index);

/*
** Structs
*/
#define LUAA_INVALID_MEMBER_NAME NULL

#define neko_lua_auto_struct(L, type) neko_lua_auto_struct_type(L, neko_lua_auto_type(L, type))
#define neko_lua_auto_struct_member(L, type, member, member_type) neko_lua_auto_struct_member_type(L, neko_lua_auto_type(L, type), #member, neko_lua_auto_type(L, member_type), offsetof(type, member))

#define neko_lua_auto_struct_push(L, type, c_in) neko_lua_auto_struct_push_type(L, neko_lua_auto_type(L, type), c_in)
#define neko_lua_auto_struct_push_member(L, type, member, c_in) neko_lua_auto_struct_push_member_offset_type(L, neko_lua_auto_type(L, type), offsetof(type, member), c_in)
#define neko_lua_auto_struct_push_member_name(L, type, member, c_in) neko_lua_auto_struct_push_member_name_type(L, neko_lua_auto_type(L, type), member, c_in)

#define neko_lua_auto_struct_to(L, type, c_out, index) neko_lua_auto_struct_to_type(L, neko_lua_auto_type(L, type), c_out, index)
#define neko_lua_auto_struct_to_member(L, type, member, c_out, index) neko_lua_auto_struct_to_member_offset_type(L, neko_lua_auto_type(L, type), offsetof(type, member), c_out, index)
#define neko_lua_auto_struct_to_member_name(L, type, member, c_out, index) neko_lua_auto_struct_to_member_name_type(L, neko_lua_auto_type(L, type), member, c_out, index)

#define neko_lua_auto_struct_has_member(L, type, member) neko_lua_auto_struct_has_member_offset_type(L, neko_lua_auto_type(L, type), offsetof(type, member))
#define neko_lua_auto_struct_has_member_name(L, type, member) neko_lua_auto_struct_has_member_name_type(L, neko_lua_auto_type(L, type), member)

#define neko_lua_auto_struct_typeof_member(L, type, member) neko_lua_auto_struct_typeof_member_offset_type(L, neko_lua_auto_type(L, type), offsetof(type, member))
#define neko_lua_auto_struct_typeof_member_name(L, type, member) neko_lua_auto_struct_typeof_member_name_type(L, neko_lua_auto_type(L, type), member)

#define neko_lua_auto_struct_registered(L, type) neko_lua_auto_struct_registered_type(L, neko_lua_auto_type(L, type))
#define neko_lua_auto_struct_next_member_name(L, type, member) neko_lua_auto_struct_next_member_name_type(L, neko_lua_auto_type(L, type), member)

void neko_lua_auto_struct_type(lua_State *L, neko_lua_auto_Type type);
void neko_lua_auto_struct_member_type(lua_State *L, neko_lua_auto_Type type, const char *member, neko_lua_auto_Type member_type, size_t offset);

int neko_lua_auto_struct_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in);
int neko_lua_auto_struct_push_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset, const void *c_in);
int neko_lua_auto_struct_push_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member, const void *c_in);

void neko_lua_auto_struct_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index);
void neko_lua_auto_struct_to_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset, void *c_out, int index);
void neko_lua_auto_struct_to_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member, void *c_out, int index);

bool neko_lua_auto_struct_has_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset);
bool neko_lua_auto_struct_has_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

neko_lua_auto_Type neko_lua_auto_struct_typeof_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset);
neko_lua_auto_Type neko_lua_auto_struct_typeof_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

bool neko_lua_auto_struct_registered_type(lua_State *L, neko_lua_auto_Type type);

const char *neko_lua_auto_struct_next_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

#define neko_lua_auto_enum(L, type) neko_lua_auto_enum_type(L, neko_lua_auto_type(L, type), sizeof(type))

#define neko_lua_auto_enum_value(L, type, value)                    \
    const type __neko_lua_auto_enum_value_temp_##value[] = {value}; \
    neko_lua_auto_enum_value_type(L, neko_lua_auto_type(L, type), __neko_lua_auto_enum_value_temp_##value, #value)

#define neko_lua_auto_enum_value_name(L, type, value, name)         \
    const type __neko_lua_auto_enum_value_temp_##value[] = {value}; \
    neko_lua_auto_enum_value_type(L, neko_lua_auto_type(L, type), __neko_lua_auto_enum_value_temp_##value, name)

#define neko_lua_auto_enum_push(L, type, c_in) neko_lua_auto_enum_push_type(L, neko_lua_auto_type(L, type), c_in)
#define neko_lua_auto_enum_to(L, type, c_out, index) neko_lua_auto_enum_to_type(L, neko_lua_auto_type(L, type), c_out, index)

#define neko_lua_auto_enum_has_value(L, type, value)                \
    const type __neko_lua_auto_enum_value_temp_##value[] = {value}; \
    neko_lua_auto_enum_has_value_type(L, neko_lua_auto_type(L, type), __neko_lua_auto_enum_value_temp_##value)

#define neko_lua_auto_enum_has_name(L, type, name) neko_lua_auto_enum_has_name_type(L, neko_lua_auto_type(L, type), name)

#define neko_lua_auto_enum_registered(L, type) neko_lua_auto_enum_registered_type(L, neko_lua_auto_type(L, type))
#define neko_lua_auto_enum_next_value_name(L, type, member) neko_lua_auto_enum_next_value_name_type(L, neko_lua_auto_type(L, type), member)

void neko_lua_auto_enum_type(lua_State *L, neko_lua_auto_Type type, size_t size);
void neko_lua_auto_enum_value_type(lua_State *L, neko_lua_auto_Type type, const void *value, const char *name);

int neko_lua_auto_enum_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in);
void neko_lua_auto_enum_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index);

bool neko_lua_auto_enum_has_value_type(lua_State *L, neko_lua_auto_Type type, const void *value);
bool neko_lua_auto_enum_has_name_type(lua_State *L, neko_lua_auto_Type type, const char *name);

bool neko_lua_auto_enum_registered_type(lua_State *L, neko_lua_auto_Type type);
const char *neko_lua_auto_enum_next_value_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

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
#define LUAA_DECLARE(func, ret_t, count, suffix, ...) LUAA_APPLY(LUAA_JOIN3(neko_lua_auto_function_declare, count, suffix), (func, ret_t, ##__VA_ARGS__))
// #define LUAA_DECLARE(func, ret_t, count, suffix, ...) LUAA_APPLY(LUAA_JOIN3(neko_lua_auto_function_declare, count, suffix), (func, ret_t, ##__VA_ARGS__))
#define LUAA_REGISTER(L, func, ret_t, count, ...) LUAA_APPLY(LUAA_JOIN2(neko_lua_auto_function_register, count), (L, func, ret_t, ##__VA_ARGS__))

#define neko_lua_auto_function_declare0(func, ret_t) \
    void __neko_lua_auto_##func(void *out, void *args) { *(ret_t *)out = func(); }

#define neko_lua_auto_function_declare0_void(func, ret_t) \
    void __neko_lua_auto_##func(void *out, void *args) { func(); }

#define neko_lua_auto_function_declare1(func, ret_t, arg0_t) \
    void __neko_lua_auto_##func(void *out, void *args) {     \
        arg0_t a0 = *(arg0_t *)args;                         \
        *(ret_t *)out = func(a0);                            \
    }

#define neko_lua_auto_function_declare1_void(func, ret_t, arg0_t) \
    void __neko_lua_auto_##func(void *out, void *args) {          \
        arg0_t a0 = *(arg0_t *)args;                              \
        func(a0);                                                 \
    }

#define neko_lua_auto_function_declare2(func, ret_t, arg0_t, arg1_t) \
    void __neko_lua_auto_##func(void *out, void *args) {             \
        arg0_t a0 = *(arg0_t *)args;                                 \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));              \
        *(ret_t *)out = func(a0, a1);                                \
    }

#define neko_lua_auto_function_declare2_void(func, ret_t, arg0_t, arg1_t) \
    void __neko_lua_auto_##func(void *out, void *args) {                  \
        arg0_t a0 = *(arg0_t *)args;                                      \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                   \
        func(a0, a1);                                                     \
    }

#define neko_lua_auto_function_declare3(func, ret_t, arg0_t, arg1_t, arg2_t) \
    void __neko_lua_auto_##func(void *out, void *args) {                     \
        arg0_t a0 = *(arg0_t *)args;                                         \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                      \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));     \
        *(ret_t *)out = func(a0, a1, a2);                                    \
    }

#define neko_lua_auto_function_declare3_void(func, ret_t, arg0_t, arg1_t, arg2_t) \
    void __neko_lua_auto_##func(void *out, void *args) {                          \
        arg0_t a0 = *(arg0_t *)args;                                              \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                           \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));          \
        func(a0, a1, a2);                                                         \
    }

#define neko_lua_auto_function_declare4(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)      \
    void __neko_lua_auto_##func(void *out, void *args) {                                  \
        arg0_t a0 = *(arg0_t *)args;                                                      \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                   \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                  \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3);                                             \
    }

#define neko_lua_auto_function_declare4_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t) \
    void __neko_lua_auto_##func(void *out, void *args) {                                  \
        arg0_t a0 = *(arg0_t *)args;                                                      \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                   \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                  \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t)); \
        func(a0, a1, a2, a3);                                                             \
    }

#define neko_lua_auto_function_declare5(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)               \
    void __neko_lua_auto_##func(void *out, void *args) {                                                   \
        arg0_t a0 = *(arg0_t *)args;                                                                       \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                    \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                   \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                  \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4);                                                          \
    }

#define neko_lua_auto_function_declare5_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)          \
    void __neko_lua_auto_##func(void *out, void *args) {                                                   \
        arg0_t a0 = *(arg0_t *)args;                                                                       \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                    \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                   \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                  \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t)); \
        func(a0, a1, a2, a3, a4);                                                                          \
    }

#define neko_lua_auto_function_declare6(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                        \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                    \
        arg0_t a0 = *(arg0_t *)args;                                                                                        \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                     \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                    \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                   \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                  \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5);                                                                       \
    }

#define neko_lua_auto_function_declare6_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                   \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                    \
        arg0_t a0 = *(arg0_t *)args;                                                                                        \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                     \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                    \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                   \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                  \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t)); \
        func(a0, a1, a2, a3, a4, a5);                                                                                       \
    }

#define neko_lua_auto_function_declare7(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                 \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                     \
        arg0_t a0 = *(arg0_t *)args;                                                                                                         \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                      \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                     \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                    \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                   \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                  \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t)); \
        *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6);                                                                                    \
    }

#define neko_lua_auto_function_declare7_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                            \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                     \
        arg0_t a0 = *(arg0_t *)args;                                                                                                         \
        arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                      \
        arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                     \
        arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                    \
        arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                   \
        arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                  \
        arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t)); \
        func(a0, a1, a2, a3, a4, a5, a6);                                                                                                    \
    }

#define neko_lua_auto_function_declare8(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                          \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                                      \
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

#define neko_lua_auto_function_declare8_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                     \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                                      \
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

#define neko_lua_auto_function_declare9(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                   \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                                                       \
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

#define neko_lua_auto_function_declare9_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                              \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                                                       \
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

#define neko_lua_auto_function_declare10(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                           \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                                                                        \
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

#define neko_lua_auto_function_declare10_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                      \
    void __neko_lua_auto_##func(void *out, void *args) {                                                                                                                                        \
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

#define neko_lua_auto_function_register0(L, func, ret_t) neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 0)

#define neko_lua_auto_function_register1(L, func, ret_t, arg0_t) \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 1, neko_lua_auto_type(L, arg0_t))

#define neko_lua_auto_function_register2(L, func, ret_t, arg0_t, arg1_t) \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 2, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t))

#define neko_lua_auto_function_register3(L, func, ret_t, arg0_t, arg1_t, arg2_t)                                                                                                \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 3, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t), \
                                         neko_lua_auto_type(L, arg2_t))

#define neko_lua_auto_function_register4(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)                                                                                        \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 4, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t), \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t))

#define neko_lua_auto_function_register5(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)                                                                                \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 5, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t), \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t))

#define neko_lua_auto_function_register6(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                                                                        \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 6, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t), \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t))

#define neko_lua_auto_function_register7(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                                                \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 7, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t), \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), neko_lua_auto_type(L, arg6_t))

#define neko_lua_auto_function_register8(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                                                            \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 8, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t),                     \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), neko_lua_auto_type(L, arg6_t), \
                                         neko_lua_auto_type(L, arg7_t))

#define neko_lua_auto_function_register9(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                                    \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 9, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t),                     \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), neko_lua_auto_type(L, arg6_t), \
                                         neko_lua_auto_type(L, arg7_t), neko_lua_auto_type(L, arg8_t))

#define neko_lua_auto_function_register10(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                           \
    neko_lua_auto_function_register_type(L, func, __neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 10, neko_lua_auto_type(L, arg0_t), neko_lua_auto_type(L, arg1_t),                    \
                                         neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), neko_lua_auto_type(L, arg6_t), \
                                         neko_lua_auto_type(L, arg7_t), neko_lua_auto_type(L, arg8_t), neko_lua_auto_type(L, arg9_t))

#define neko_lua_auto_function(L, func, ret_t, ...)             \
    neko_lua_auto_function_declare(func, ret_t, ##__VA_ARGS__); \
    neko_lua_auto_function_register(L, func, ret_t, ##__VA_ARGS__)
#define neko_lua_auto_function_declare(func, ret_t, ...) LUAA_DECLARE(func, ret_t, LUAA_COUNT(__VA_ARGS__), LUAA_SUFFIX(ret_t), ##__VA_ARGS__)
#define neko_lua_auto_function_register(L, func, ret_t, ...) LUAA_REGISTER(L, func, ret_t, LUAA_COUNT(__VA_ARGS__), ##__VA_ARGS__)

enum { LUAA_RETURN_STACK_SIZE = 256, LUAA_ARGUMENT_STACK_SIZE = 2048 };

typedef void (*neko_lua_auto_Func)(void *, void *);

int neko_lua_auto_call(lua_State *L, void *func_ptr);
int neko_lua_auto_call_name(lua_State *L, const char *func_name);

void neko_lua_auto_function_register_type(lua_State *L, void *src_func, neko_lua_auto_Func auto_func, const char *name, neko_lua_auto_Type ret_tid, int num_args, ...);

#pragma endregion LuaA

void neko_lua_debug_setup(lua_State *lua, const char *name, const char *globalName, lua_CFunction readFunc, lua_CFunction writeFunc);
int neko_lua_debug_pcall(lua_State *lua, int nargs, int nresults, int msgh);

#define neko_lua_debug_dofile(lua, filename) (luaL_loadfile(lua, filename) || neko_lua_debug_pcall(lua, 0, LUA_MULTRET, 0))

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name);
int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name);
void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name);
void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name);

neko_inline void neko_lua_print_stack(lua_State *L) {
    int top = lua_gettop(L);  // 获取堆栈上的元素个数
    printf("Stack size: %d\n", top);

    for (int i = 1; i <= top; ++i) {
        int type = lua_type(L, i);                      // 获取元素的类型
        const char *type_name = lua_typename(L, type);  // 获取类型名称

        printf("[%d] Type: %s | Value: ", i, type_name);

        switch (type) {
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, i));  // 输出字符串值
                break;
            case LUA_TNUMBER:
                printf("%f\n", lua_tonumber(L, i));  // 输出数字值
                break;
            default:
                printf("Unknown\n");
                break;
        }
    }
    printf("\n");
}

int luaopen_cstruct_core(lua_State *L);
int luaopen_cstruct_test(lua_State *L);
int luaopen_datalist(lua_State *L);

namespace neko {

template <typename T>
T neko_lua_to(lua_State *L, int index) {
    if constexpr (std::same_as<T, s32> || std::same_as<T, u32>) {
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
    } else {
        static_assert(std::is_same_v<T, void>, "Unsupported type for neko_lua_to");
    }
}

neko_inline bool neko_lua_equal(lua_State *state, int index1, int index2) {
#if LUA_VERSION_NUM <= 501
    return lua_equal(state, index1, index2) == 1;
#else
    return lua_compare(state, index1, index2, LUA_OPEQ) == 1;
#endif
}

template <typename Iterable>
neko_inline bool neko_lua_equal(lua_State *state, const Iterable &indices) {
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

}  // namespace neko

namespace neko {

#define INHERIT_TABLE "inherit_table"

struct cpp_void_t {};

struct lua_string_tool_t {
    inline static const char *c_str(const std::string &s_) { return s_.c_str(); }
    inline static const char *c_str(const char *s_) { return s_; }
};

class lua_exception_t : public std::exception {
public:
    explicit lua_exception_t(const char *err_) : m_err(err_) {}
    explicit lua_exception_t(const std::string &err_) : m_err(err_) {}
    ~lua_exception_t() throw() {}

    const char *what() const throw() { return m_err.c_str(); }

private:
    std::string m_err;
};

class neko_lua_wrap_tool_t {
public:
    static void dump_stack(lua_State *ls_) {
        int i;
        int top = lua_gettop(ls_);

        for (i = 1; i <= top; i++) {
            int t = lua_type(ls_, i);
            switch (t) {
                case LUA_TSTRING: {
                    printf("`%s'", lua_tostring(ls_, i));
                } break;
                case LUA_TBOOLEAN: {
                    printf(lua_toboolean(ls_, i) ? "true" : "false");
                } break;
                case LUA_TNUMBER: {
                    printf("`%g`", lua_tonumber(ls_, i));
                } break;
                case LUA_TTABLE: {
                    printf("table end\n");
                    lua_pushnil(ls_);
                    while (lua_next(ls_, i) != 0) {
                        printf("	%s - %s\n", lua_typename(ls_, lua_type(ls_, -2)), lua_typename(ls_, lua_type(ls_, -1)));
                        lua_pop(ls_, 1);
                    }
                    printf("table end");
                } break;
                default: {
                    printf("`%s`", lua_typename(ls_, t));
                } break;
            }
            printf(" ");
        }
        printf("\n");
    }
    static std::string dump_error(lua_State *ls_, const char *fmt, ...) {
        std::string ret;
        char buff[1024];

        va_list argp;
        va_start(argp, fmt);
#ifndef _WIN32
        vsnprintf(buff, sizeof(buff), fmt, argp);
#else
        vsnprintf_s(buff, sizeof(buff), sizeof(buff), fmt, argp);
#endif
        va_end(argp);

        ret = buff;
        SPRINTF_F(buff, sizeof(buff), " tracback:%s", lua_tostring(ls_, -1));
        ret += buff;

        return ret;
    }
};

typedef int (*lua_function_t)(lua_State *L);

class lua_nil_t {};

template <typename T>
struct userdata_for_object_t {
    userdata_for_object_t(T *p_ = NULL) : obj(p_) {}
    T *obj;
};

template <typename T>
struct lua_type_info_t {
    static void set_name(const std::string &name_, std::string inherit_name_ = "") {
        size_t n = name_.length() > sizeof(name) - 1 ? sizeof(name) - 1 : name_.length();
#ifndef _WIN32
        ::strncpy(name, name_.c_str(), n);
#else
        ::strncpy_s(name, name_.c_str(), n);
#endif
        if (false == inherit_name_.empty()) {
            n = inherit_name_.length() > sizeof(inherit_name) - 1 ? sizeof(inherit_name) - 1 : inherit_name_.length();
#ifndef _WIN32
            ::strncpy(inherit_name, inherit_name_.c_str(), n);
#else
            ::strncpy_s(inherit_name, inherit_name_.c_str(), n);
#endif
        }
    }
    inline static const char *get_name() { return name; }
    inline static const char *get_inherit_name() { return inherit_name; }
    inline static bool is_registed() { return name[0] != '\0'; }
    inline static bool is_inherit() { return inherit_name[0] != '\0'; }
    static char name[128];
    static char inherit_name[128];
};
template <typename T>
char lua_type_info_t<T>::name[128] = {0};
template <typename T>
char lua_type_info_t<T>::inherit_name[128] = {0};

template <typename ARG_TYPE>
struct basetype_ptr_traits_t;
template <>
struct basetype_ptr_traits_t<const std::string &> {
    typedef std::string arg_type_t;
};
template <>
struct basetype_ptr_traits_t<std::string &> {
    typedef std::string arg_type_t;
};
template <>
struct basetype_ptr_traits_t<std::string> {
    typedef std::string arg_type_t;
};
template <>
struct basetype_ptr_traits_t<const char *> {
    typedef char *arg_type_t;
};
template <>
struct basetype_ptr_traits_t<char *> {
    typedef char *arg_type_t;
};
template <>
struct basetype_ptr_traits_t<char> {
    typedef char arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned char> {
    typedef unsigned char arg_type_t;
};
template <>
struct basetype_ptr_traits_t<short> {
    typedef short arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned short> {
    typedef unsigned short arg_type_t;
};
template <>
struct basetype_ptr_traits_t<int> {
    typedef int arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned int> {
    typedef unsigned int arg_type_t;
};
template <>
struct basetype_ptr_traits_t<long> {
    typedef long arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned long> {
    typedef unsigned long arg_type_t;
};
template <>
struct basetype_ptr_traits_t<long long> {
    typedef long long arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned long long> {
    typedef unsigned long long arg_type_t;
};

template <>
struct basetype_ptr_traits_t<float> {
    typedef float arg_type_t;
};
template <>
struct basetype_ptr_traits_t<bool> {
    typedef bool arg_type_t;
};

template <>
struct basetype_ptr_traits_t<double> {
    typedef double arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const T &> {
    typedef T *arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<T &> {
    typedef T *arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<T *> {
    typedef T *arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const T *> {
    typedef T *arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<vector<T> > {
    typedef vector<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::list<T> > {
    typedef std::list<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::set<T> > {
    typedef std::set<T> arg_type_t;
};
template <typename K, typename V>
struct basetype_ptr_traits_t<std::map<K, V> > {
    typedef std::map<K, V> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<vector<T> &> {
    typedef vector<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::list<T> &> {
    typedef std::list<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::set<T> &> {
    typedef std::set<T> arg_type_t;
};
template <typename K, typename V>
struct basetype_ptr_traits_t<std::map<K, V> &> {
    typedef std::map<K, V> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const vector<T> &> {
    typedef vector<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const std::list<T> &> {
    typedef std::list<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const std::set<T> &> {
    typedef std::set<T> arg_type_t;
};
template <typename K, typename V>
struct basetype_ptr_traits_t<const std::map<K, V> &> {
    typedef std::map<K, V> arg_type_t;
};

//!-------------------------------------------------------------------------------------------------------------------------
template <typename ARG_TYPE>
struct p_t;

template <typename ARG_TYPE>
struct p_t {
    static ARG_TYPE r(ARG_TYPE a) { return a; }
    static ARG_TYPE &r(ARG_TYPE *a) { return *a; }
};
template <typename ARG_TYPE>
struct p_t<ARG_TYPE &> {
    static ARG_TYPE &r(ARG_TYPE &a) { return a; }
    static ARG_TYPE &r(ARG_TYPE *a) { return *a; }
};
//! #########################################################################################################################
template <typename ARG_TYPE>
struct reference_traits_t;

template <typename ARG_TYPE>
struct reference_traits_t {
    typedef ARG_TYPE arg_type_t;
};

template <>
struct reference_traits_t<const std::string &> {
    typedef std::string arg_type_t;
};

template <>
struct reference_traits_t<std::string &> {
    typedef std::string arg_type_t;
};

template <typename T>
struct reference_traits_t<const T *> {
    typedef T *arg_type_t;
};
template <typename T>
struct reference_traits_t<const T &> {
    typedef T arg_type_t;
};

template <>
struct reference_traits_t<const char *> {
    typedef char *arg_type_t;
};

template <typename T>
struct init_value_traits_t;

template <typename T>
struct init_value_traits_t {
    inline static T value() { return T(); }
};

template <typename T>
struct init_value_traits_t<const T *> {
    inline static T *value() { return NULL; }
};

template <typename T>
struct init_value_traits_t<const T &> {
    inline static T value() { return T(); }
};

template <>
struct init_value_traits_t<std::string> {
    inline static const char *value() { return ""; }
};

template <>
struct init_value_traits_t<const std::string &> {
    inline static const char *value() { return ""; }
};

template <typename T>
struct __lua_op_t {
    static void push_stack(lua_State *ls_, const char *arg_) { lua_pushstring(ls_, arg_); }
    /*
    static int lua_to_value(lua_State* ls_, int pos_, char*& param_)
    {
        const char* str = luaL_checkstring(ls_, pos_);
        param_ = (char*)str;
        return 0;
    }*/
};

template <>
struct __lua_op_t<const char *> {
    static void push_stack(lua_State *ls_, const char *arg_) { lua_pushstring(ls_, arg_); }
    static int lua_to_value(lua_State *ls_, int pos_, char *&param_) {
        const char *str = luaL_checkstring(ls_, pos_);
        param_ = (char *)str;
        return 0;
    }
};
template <>
struct __lua_op_t<char *> {
    static void push_stack(lua_State *ls_, const char *arg_) { lua_pushstring(ls_, arg_); }
    static int lua_to_value(lua_State *ls_, int pos_, char *&param_) {
        const char *str = luaL_checkstring(ls_, pos_);
        param_ = (char *)str;
        return 0;
    }
};

template <>
struct __lua_op_t<lua_nil_t> {
    static void push_stack(lua_State *ls_, const lua_nil_t &arg_) { lua_pushnil(ls_); }
};

template <>
struct __lua_op_t<cpp_void_t> {
    static int get_ret_value(lua_State *ls_, int pos_, cpp_void_t &param_) { return 0; }
};

template <>
struct __lua_op_t<int64_t> {
    static void push_stack(lua_State *ls_, int64_t arg_) {
#if LUA_VERSION_NUM >= 503
        lua_pushinteger(ls_, arg_);
#else
        stringstream ss;
        ss << arg_;
        string str = ss.str();
        lua_pushlstring(ls_, str.c_str(), str.length());
#endif
    }

    static int get_ret_value(lua_State *ls_, int pos_, int64_t &param_) {
#if LUA_VERSION_NUM >= 503
        if (!lua_isinteger(ls_, pos_)) {
            return -1;
        }
        param_ = lua_tointeger(ls_, pos_);
#else
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        size_t len = 0;
        const char *src = lua_tolstring(ls_, pos_, &len);
        param_ = (int64_t)strtoll(src, NULL, 10);
#endif
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, int64_t &param_) {
#if LUA_VERSION_NUM >= 503
        param_ = luaL_checkinteger(ls_, pos_);
#else
        size_t len = 0;
        const char *str = luaL_checklstring(ls_, pos_, &len);
        param_ = (int64_t)strtoll(str, NULL, 10);
#endif
        return 0;
    }
};

template <>
struct __lua_op_t<uint64_t> {
    static void push_stack(lua_State *ls_, uint64_t arg_) {
        std::stringstream ss;
        ss << arg_;
        std::string str = ss.str();
        lua_pushlstring(ls_, str.c_str(), str.length());
    }

    static int get_ret_value(lua_State *ls_, int pos_, uint64_t &param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        size_t len = 0;
        const char *src = lua_tolstring(ls_, pos_, &len);
        param_ = (uint64_t)strtoull(src, NULL, 10);
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, uint64_t &param_) {
        size_t len = 0;
        const char *str = luaL_checklstring(ls_, pos_, &len);
        param_ = (uint64_t)strtoull(str, NULL, 10);
        return 0;
    }
};

template <>
struct __lua_op_t<int8_t> {

    static void push_stack(lua_State *ls_, int8_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, int8_t &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (int8_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, int8_t &param_) {
        param_ = (int8_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};

template <>
struct __lua_op_t<uint8_t> {
    static void push_stack(lua_State *ls_, uint8_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, uint8_t &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (uint8_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, uint8_t &param_) {
        param_ = (uint8_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
#ifdef _WIN32

template <>
struct __lua_op_t<char> {

    static void push_stack(lua_State *ls_, char arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, char &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (char)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, char &param_) {
        param_ = (char)luaL_checknumber(ls_, pos_);
        return 0;
    }
};

#endif
template <>
struct __lua_op_t<int16_t> {
    static void push_stack(lua_State *ls_, int16_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, int16_t &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (int16_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, int16_t &param_) {
        param_ = (int16_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct __lua_op_t<uint16_t> {

    static void push_stack(lua_State *ls_, uint16_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, uint16_t &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (uint16_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, uint16_t &param_) {
        param_ = (uint16_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct __lua_op_t<int32_t> {
    static void push_stack(lua_State *ls_, int32_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, int32_t &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (int32_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, int32_t &param_) {
        param_ = (int32_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct __lua_op_t<uint32_t> {

    static void push_stack(lua_State *ls_, uint32_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, uint32_t &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (uint32_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, uint32_t &param_) {
        param_ = (uint32_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};

template <>
struct __lua_op_t<bool> {
    static void push_stack(lua_State *ls_, bool arg_) { lua_pushboolean(ls_, arg_); }

    static int get_ret_value(lua_State *ls_, int pos_, bool &param_) {
        //! nil 自动转换为false
        if (lua_isnil(ls_, pos_)) {
            param_ = false;
            return 0;
        }
        if (!lua_isboolean(ls_, pos_)) {
            return -1;
        }

        param_ = (0 != lua_toboolean(ls_, pos_));
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, bool &param_) {
        luaL_checktype(ls_, pos_, LUA_TBOOLEAN);
        param_ = (0 != lua_toboolean(ls_, pos_));
        return 0;
    }
};

template <>
struct __lua_op_t<std::string> {

    static void push_stack(lua_State *ls_, const std::string &arg_) { lua_pushlstring(ls_, arg_.c_str(), arg_.length()); }

    static int get_ret_value(lua_State *ls_, int pos_, std::string &param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        lua_pushvalue(ls_, pos_);
        size_t len = 0;
        const char *src = lua_tolstring(ls_, -1, &len);
        param_.assign(src, len);
        lua_pop(ls_, 1);

        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, std::string &param_) {
        size_t len = 0;
        const char *str = luaL_checklstring(ls_, pos_, &len);
        param_.assign(str, len);
        return 0;
    }
};

template <>
struct __lua_op_t<const std::string &> {
    static void push_stack(lua_State *ls_, const std::string &arg_) { lua_pushlstring(ls_, arg_.c_str(), arg_.length()); }

    static int get_ret_value(lua_State *ls_, int pos_, std::string &param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        lua_pushvalue(ls_, pos_);
        size_t len = 0;
        const char *src = lua_tolstring(ls_, -1, &len);
        param_.assign(src, len);
        lua_pop(ls_, 1);

        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, std::string &param_) {
        size_t len = 0;
        const char *str = luaL_checklstring(ls_, pos_, &len);
        param_.assign(str, len);
        return 0;
    }
};
template <>
struct __lua_op_t<float> {
    static void push_stack(lua_State *ls_, float arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, float &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (float)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, float &param_) {
        param_ = (float)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct __lua_op_t<double> {
    static void push_stack(lua_State *ls_, double arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State *ls_, int pos_, double &param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (double)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, double &param_) {
        param_ = (double)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
/*template<> struct __lua_op_t<long>
{

    static void push_stack(lua_State* ls_, long arg_)
    {
        lua_pushnumber(ls_, (lua_Number)arg_);
    }
    static int get_ret_value(lua_State* ls_, int pos_, long& param_)
    {
        if (!lua_isnumber(ls_, pos_))
        {
            return -1;
        }
        param_ = (long)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, long& param_)
    {
        param_ = (long)luaL_checknumber(ls_, pos_);
        return 0;
    }
};*/
template <>
struct __lua_op_t<void *> {
    static void push_stack(lua_State *ls_, void *arg_) { lua_pushlightuserdata(ls_, arg_); }

    static int get_ret_value(lua_State *ls_, int pos_, void *&param_) {
        if (!lua_isuserdata(ls_, pos_)) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "userdata param expected, but type<%s> provided", lua_typename(ls_, lua_type(ls_, pos_)));
            printf("%s\n", buff);
            return -1;
        }

        param_ = lua_touserdata(ls_, pos_);
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, void *&param_) {
        if (!lua_isuserdata(ls_, pos_)) {
            luaL_argerror(ls_, 1, "userdata param expected");
            return -1;
        }
        param_ = lua_touserdata(ls_, pos_);
        return 0;
    }
};

template <typename T>
struct __lua_op_t<T *> {
    static void push_stack(lua_State *ls_, T &arg_) {
        void *ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<T>));
        new (ptr) userdata_for_object_t<T>(&arg_);

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        lua_setmetatable(ls_, -2);
    }
    static void push_stack(lua_State *ls_, const T &arg_) {
        void *ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<const T>));
        new (ptr) userdata_for_object_t<const T>(&arg_);

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        lua_setmetatable(ls_, -2);
    }
    static void push_stack(lua_State *ls_, T *arg_) {
        void *ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<T>));
        new (ptr) userdata_for_object_t<T>(arg_);

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        lua_setmetatable(ls_, -2);
    }

    static int get_ret_value(lua_State *ls_, int pos_, T *&param_) {
        if (false == lua_type_info_t<T>::is_registed()) {
            luaL_argerror(ls_, pos_, "type not supported");
        }

        void *arg_data = lua_touserdata(ls_, pos_);

        if (NULL == arg_data) {
            printf("expect<%s> but <%s> NULL\n", lua_type_info_t<T>::get_name(), lua_typename(ls_, lua_type(ls_, pos_)));
            return -1;
        }

        if (0 == lua_getmetatable(ls_, pos_)) {
            return -1;
        }

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        if (0 == lua_rawequal(ls_, -1, -2)) {
            lua_getfield(ls_, -2, INHERIT_TABLE);
            if (0 == lua_rawequal(ls_, -1, -2)) {
                printf("expect<%s> but <%s> not equal\n", lua_type_info_t<T>::get_name(), lua_typename(ls_, lua_type(ls_, pos_)));
                lua_pop(ls_, 3);
                return -1;
            }
            lua_pop(ls_, 3);
        } else {
            lua_pop(ls_, 2);
        }
        T *ret_ptr = ((userdata_for_object_t<T> *)arg_data)->obj;
        if (NULL == ret_ptr) {
            return -1;
        }

        param_ = ret_ptr;
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, T *&param_) {
        if (false == lua_type_info_t<T>::is_registed()) {
            luaL_argerror(ls_, pos_, "type not supported");
        }
        void *arg_data = lua_touserdata(ls_, pos_);

        if (NULL == arg_data || 0 == lua_getmetatable(ls_, pos_)) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "`%s` arg1 connot be null", lua_type_info_t<T>::get_name());
            luaL_argerror(ls_, pos_, buff);
        }

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        if (0 == lua_rawequal(ls_, -1, -2)) {
            lua_getfield(ls_, -2, INHERIT_TABLE);
            if (0 == lua_rawequal(ls_, -1, -2)) {
                lua_pop(ls_, 3);
                char buff[128];
                SPRINTF_F(buff, sizeof(buff), "`%s` arg1 type not equal", lua_type_info_t<T>::get_name());
                luaL_argerror(ls_, pos_, buff);
            }
            lua_pop(ls_, 3);
        } else {
            lua_pop(ls_, 2);
        }

        T *ret_ptr = ((userdata_for_object_t<T> *)arg_data)->obj;
        if (NULL == ret_ptr) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "`%s` object ptr can't be null", lua_type_info_t<T>::get_name());
            luaL_argerror(ls_, pos_, buff);
        }

        param_ = ret_ptr;
        return 0;
    }
};

template <typename T>
struct __lua_op_t<const T *> {
    static void push_stack(lua_State *ls_, const T *arg_) { __lua_op_t<T *>::push_stack(ls_, (T *)arg_); }

    static int get_ret_value(lua_State *ls_, int pos_, T *&param_) { return __lua_op_t<T *>::get_ret_value(ls_, pos_, param_); }

    static int lua_to_value(lua_State *ls_, int pos_, T *&param_) { return __lua_op_t<T *>::lua_to_value(ls_, pos_, param_); }
};

template <typename T>
struct __lua_op_t<vector<T> > {
    static void push_stack(lua_State *ls_, const vector<T> &arg_) {
        lua_newtable(ls_);
        typename vector<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            __lua_op_t<int>::push_stack(ls_, i);
            __lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State *ls_, int pos_, vector<T> &param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (__lua_op_t<T>::get_ret_value(ls_, -1, param_[param_.size() - 1]) < 0) {
                return -1;
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, vector<T> &param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (__lua_op_t<T>::lua_to_value(ls_, -1, param_[param_.size() - 1]) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }
};

template <typename T>
struct __lua_op_t<std::list<T> > {
    static void push_stack(lua_State *ls_, const std::list<T> &arg_) {
        lua_newtable(ls_);
        typename std::list<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            __lua_op_t<int>::push_stack(ls_, i);
            __lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State *ls_, int pos_, std::list<T> &param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (__lua_op_t<T>::get_ret_value(ls_, -1, (param_.back())) < 0) {
                return -1;
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, std::list<T> &param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (__lua_op_t<T>::lua_to_value(ls_, -1, (param_.back())) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }
};

template <typename T>
struct __lua_op_t<std::set<T> > {
    static void push_stack(lua_State *ls_, const std::set<T> &arg_) {
        lua_newtable(ls_);
        typename std::set<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            __lua_op_t<int>::push_stack(ls_, i);
            __lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State *ls_, int pos_, std::set<T> &param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            T val = init_value_traits_t<T>::value();
            if (__lua_op_t<T>::get_ret_value(ls_, -1, val) < 0) {
                return -1;
            }
            param_.insert(val);
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, std::set<T> &param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            T val = init_value_traits_t<T>::value();
            if (__lua_op_t<T>::lua_to_value(ls_, -1, val) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            param_.insert(val);
            lua_pop(ls_, 1);
        }
        return 0;
    }
};
template <typename K, typename V>
struct __lua_op_t<std::map<K, V> > {
    static void push_stack(lua_State *ls_, const std::map<K, V> &arg_) {
        lua_newtable(ls_);
        typename std::map<K, V>::const_iterator it = arg_.begin();
        for (; it != arg_.end(); ++it) {
            __lua_op_t<K>::push_stack(ls_, it->first);
            __lua_op_t<V>::push_stack(ls_, it->second);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State *ls_, int pos_, std::map<K, V> &param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            K key = init_value_traits_t<K>::value();
            V val = init_value_traits_t<V>::value();

            if (__lua_op_t<K>::get_ret_value(ls_, -2, key) < 0 || __lua_op_t<V>::get_ret_value(ls_, -1, val) < 0) {
                return -1;
            }
            param_.insert(std::make_pair(key, val));
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State *ls_, int pos_, std::map<K, V> &param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            K key = init_value_traits_t<K>::value();
            V val = init_value_traits_t<V>::value();
            if (__lua_op_t<K>::get_ret_value(ls_, -2, key) < 0 || __lua_op_t<V>::get_ret_value(ls_, -1, val) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            param_.insert(std::make_pair(key, val));
            lua_pop(ls_, 1);
        }
        return 0;
    }
};

}  // namespace neko

namespace neko {
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

#define RET_V typename void_ignore_t<RET>::value_t

class neko_lua_wrap_t {
    enum STACK_MIN_NUM_e { STACK_MIN_NUM = 20 };

public:
    neko_lua_wrap_t(bool b = false) : m_ls(NULL), m_bEnableModFunc(b) {
        m_ls = ::luaL_newstate();
        ::luaL_openlibs(m_ls);

        __neko_lua_auto_open(m_ls);
    }
    virtual ~neko_lua_wrap_t() {
        if (m_ls) {
            __neko_lua_auto_close(m_ls);
            ::lua_close(m_ls);
            m_ls = NULL;
        }
    }
    void dump_stack() const { neko_lua_wrap_tool_t::dump_stack(m_ls); }
    void setModFuncFlag(bool b) { m_bEnableModFunc = b; }

    lua_State *get_lua_state() { return m_ls; }

    int add_package_path(const std::string &str_) {
        std::string new_path = "package.path = package.path .. \"";
        if (str_.empty()) {
            return -1;
        }

        if (str_[0] != ';') {
            new_path += ";";
        }

        new_path += str_;

        if (str_[str_.length() - 1] != '/') {
            new_path += "/";
        }

        new_path += "?.lua\" ";

        run_string(new_path);
        return 0;
    }

    int load_file(const std::string &file_name_)  //
    {
        if (luaL_dofile(m_ls, file_name_.c_str())) {
            std::string err = neko_lua_wrap_tool_t::dump_error(m_ls, "cannot load file<%s>", file_name_.c_str());
            ::lua_pop(m_ls, 1);
            throw lua_exception_t(err);
        }

        return 0;
    }

    inline int lua_pcall_wrap(lua_State *state, int argnum, int retnum) {
        int result = lua_pcall(state, argnum, retnum, 0);
        return result;
    }

    bool do_file(const std::string &file) {
        int status = luaL_loadfile(m_ls, file.c_str());

        if (status) {
            throw lua_exception_t(std::to_string(status));
            return false;
        }

        status = lua_pcall_wrap(m_ls, 0, LUA_MULTRET);
        if (status) {
            throw lua_exception_t(std::to_string(status));
            return false;
        }
        return true;
    }

    template <typename T>
    void open_lib(T arg_);

    void run_string(const char *str_) {
        if (luaL_dostring(m_ls, str_)) {
            std::string err = neko_lua_wrap_tool_t::dump_error(m_ls, "neko_lua_wrap_t::run_string ::lua_pcall failed str<%s>", str_);
            ::lua_pop(m_ls, 1);
            throw lua_exception_t(err);
        }
    }
    void run_string(const std::string &str_) { run_string(str_.c_str()); }

    template <typename T>
    int get_global_variable(const std::string &field_name_, T &ret_);
    template <typename T>
    int get_global_variable(const char *field_name_, T &ret_);

    template <typename T>
    int set_global_variable(const std::string &field_name_, const T &value_);
    template <typename T>
    int set_global_variable(const char *field_name_, const T &value_);

    void register_raw_function(const char *func_name_, lua_function_t func_) {
        lua_checkstack(m_ls, STACK_MIN_NUM);

        lua_pushcfunction(m_ls, func_);
        lua_setglobal(m_ls, func_name_);
    }

    template <typename T>
    void reg(T a);

    void call(const char *func_name_) {
        ::lua_getglobal(m_ls, func_name_);

        if (::lua_pcall(m_ls, 0, 0, 0) != 0) {
            std::string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
            ::lua_pop(m_ls, 1);
            throw lua_exception_t(err);
        }
    }

    template <typename RET>
    RET_V call(const char *func_name_);

    template <typename RET, typename ARG1>
    RET_V call(const char *func_name_, const ARG1 &arg1_);

    template <typename RET, typename ARG1, typename ARG2>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_, const ARG8 &arg8_);

    template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
    RET_V call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_, const ARG8 &arg8_,
               const ARG9 &arg9_);

private:
    int getFuncByName(const char *func_name_) {
        if (false == m_bEnableModFunc) {
            lua_getglobal(m_ls, func_name_);
            return 0;
        }
        char tmpBuff[512] = {0};
        char *begin = tmpBuff;
        for (unsigned int i = 0; i < sizeof(tmpBuff); ++i) {
            char c = func_name_[i];
            tmpBuff[i] = c;
            if (c == '\0') {
                break;
            }

            if (c == '.') {
                tmpBuff[i] = '\0';
                lua_getglobal(m_ls, lua_string_tool_t::c_str(begin));
                const char *begin2 = func_name_ + i + 1;
                lua_getfield(m_ls, -1, begin2);
                lua_remove(m_ls, -2);
                return 0;
            } else if (c == ':') {
                tmpBuff[i] = '\0';
                lua_getglobal(m_ls, begin);
                const char *begin2 = func_name_ + i + 1;
                lua_getfield(m_ls, -1, begin2);
                lua_pushvalue(m_ls, -2);
                lua_remove(m_ls, -3);
                return 1;
            }
        }

        lua_getglobal(m_ls, func_name_);
        return 0;
    }

private:
    lua_State *m_ls;
    bool m_bEnableModFunc;
};

template <typename T>
void neko_lua_wrap_t::open_lib(T arg_) {
    arg_(m_ls);
}

template <typename T>
int neko_lua_wrap_t::get_global_variable(const std::string &field_name_, T &ret_) {
    return get_global_variable<T>(field_name_.c_str(), ret_);
}

template <typename T>
int neko_lua_wrap_t::get_global_variable(const char *field_name_, T &ret_) {
    int ret = 0;

    lua_getglobal(m_ls, field_name_);
    ret = __lua_op_t<T>::get_ret_value(m_ls, -1, ret_);

    lua_pop(m_ls, 1);
    return ret;
}

template <typename T>
int neko_lua_wrap_t::set_global_variable(const std::string &field_name_, const T &value_) {
    return set_global_variable<T>(field_name_.c_str(), value_);
}

template <typename T>
int neko_lua_wrap_t::set_global_variable(const char *field_name_, const T &value_) {
    __lua_op_t<T>::push_stack(m_ls, value_);
    lua_setglobal(m_ls, field_name_);
    return 0;
}

template <typename T>
void neko_lua_wrap_t::reg(T a) {
    a(this->get_lua_state());
}

//! impl for common RET
template <typename RET>
RET_V neko_lua_wrap_t::call(const char *func_name_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    if (lua_pcall(m_ls, tmpArg + 0, 1, 0) != 0) {
        std::string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg0] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);

    if (lua_pcall(m_ls, tmpArg + 1, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg1] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_)

{
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);

    if (lua_pcall(m_ls, tmpArg + 2, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg2] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);

    if (lua_pcall(m_ls, tmpArg + 3, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg3] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);

    if (lua_pcall(m_ls, tmpArg + 4, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg4] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);

    if (lua_pcall(m_ls, tmpArg + 5, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg5] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);

    if (lua_pcall(m_ls, tmpArg + 6, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg6] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    __lua_op_t<ARG7>::push_stack(m_ls, arg7_);

    if (lua_pcall(m_ls, tmpArg + 7, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg7] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_,
                            const ARG8 &arg8_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    __lua_op_t<ARG7>::push_stack(m_ls, arg7_);
    __lua_op_t<ARG8>::push_stack(m_ls, arg8_);

    if (lua_pcall(m_ls, tmpArg + 8, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg8] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
RET_V neko_lua_wrap_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_,
                            const ARG8 &arg8_, const ARG9 &arg9_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    __lua_op_t<ARG7>::push_stack(m_ls, arg7_);
    __lua_op_t<ARG8>::push_stack(m_ls, arg8_);
    __lua_op_t<ARG9>::push_stack(m_ls, arg9_);

    if (lua_pcall(m_ls, tmpArg + 9, 1, 0) != 0) {
        string err = neko_lua_wrap_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (__lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg9] get_ret_value failed func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

}  // namespace neko

#endif