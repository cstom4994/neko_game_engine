
#ifndef NEKO_LUA_BASE_H
#define NEKO_LUA_BASE_H

#include <cstdlib>
#include <memory>
#include <string>
#include <utility>

#include "engine/common/neko_types.h"
#include "libs/lua/lua.hpp"

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

/*
** MSVC does not allow nested functions
** so function is wrapped in nested struct
*/
#ifdef _MSC_VER

#define neko_lua_auto_function_declare0(func, ret_t)                                           \
    struct __neko_lua_auto_wrap_##func {                                                       \
        static void __neko_lua_auto_##func(char *out, char *args) { *(ret_t *)out = func(); }; \
    }

#define neko_lua_auto_function_declare0_void(func, ret_t)                      \
    struct __neko_lua_auto_wrap_##func {                                       \
        static void __neko_lua_auto_##func(char *out, char *args) { func(); }; \
    }

#define neko_lua_auto_function_declare1(func, ret_t, arg0_t)        \
    struct __neko_lua_auto_wrap_##func {                            \
        static void __neko_lua_auto_##func(char *out, char *args) { \
            arg0_t a0 = *(arg0_t *)args;                            \
            *(ret_t *)out = func(a0);                               \
        };                                                          \
    }

#define neko_lua_auto_function_declare1_void(func, ret_t, arg0_t)   \
    struct __neko_lua_auto_wrap_##func {                            \
        static void __neko_lua_auto_##func(char *out, char *args) { \
            arg0_t a0 = *(arg0_t *)args;                            \
            func(a0);                                               \
        };                                                          \
    }

#define neko_lua_auto_function_declare2(func, ret_t, arg0_t, arg1_t) \
    struct __neko_lua_auto_wrap_##func {                             \
        static void __neko_lua_auto_##func(char *out, char *args) {  \
            arg0_t a0 = *(arg0_t *)args;                             \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));          \
            *(ret_t *)out = func(a0, a1);                            \
        };                                                           \
    }

#define neko_lua_auto_function_declare2_void(func, ret_t, arg0_t, arg1_t) \
    struct __neko_lua_auto_wrap_##func {                                  \
        static void __neko_lua_auto_##func(char *out, char *args) {       \
            arg0_t a0 = *(arg0_t *)args;                                  \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));               \
            func(a0, a1);                                                 \
        };                                                                \
    }

#define neko_lua_auto_function_declare3(func, ret_t, arg0_t, arg1_t, arg2_t) \
    struct __neko_lua_auto_wrap_##func {                                     \
        static void __neko_lua_auto_##func(char *out, char *args) {          \
            arg0_t a0 = *(arg0_t *)args;                                     \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                  \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t)); \
            *(ret_t *)out = func(a0, a1, a2);                                \
        };                                                                   \
    }

#define neko_lua_auto_function_declare3_void(func, ret_t, arg0_t, arg1_t, arg2_t) \
    struct __neko_lua_auto_wrap_##func {                                          \
        static void __neko_lua_auto_##func(char *out, char *args) {               \
            arg0_t a0 = *(arg0_t *)args;                                          \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                       \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));      \
            func(a0, a1, a2);                                                     \
        };                                                                        \
    }

#define neko_lua_auto_function_declare4(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)          \
    struct __neko_lua_auto_wrap_##func {                                                      \
        static void __neko_lua_auto_##func(char *out, char *args) {                           \
            arg0_t a0 = *(arg0_t *)args;                                                      \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                   \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                  \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t)); \
            *(ret_t *)out = func(a0, a1, a2, a3);                                             \
        };                                                                                    \
    }

#define neko_lua_auto_function_declare4_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)     \
    struct __neko_lua_auto_wrap_##func {                                                      \
        static void __neko_lua_auto_##func(char *out, char *args) {                           \
            arg0_t a0 = *(arg0_t *)args;                                                      \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                   \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                  \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t)); \
            func(a0, a1, a2, a3);                                                             \
        };                                                                                    \
    }

#define neko_lua_auto_function_declare5(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)                   \
    struct __neko_lua_auto_wrap_##func {                                                                       \
        static void __neko_lua_auto_##func(char *out, char *args) {                                            \
            arg0_t a0 = *(arg0_t *)args;                                                                       \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                    \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                   \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                  \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t)); \
            *(ret_t *)out = func(a0, a1, a2, a3, a4);                                                          \
        };                                                                                                     \
    }

#define neko_lua_auto_function_declare5_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)              \
    struct __neko_lua_auto_wrap_##func {                                                                       \
        static void __neko_lua_auto_##func(char *out, char *args) {                                            \
            arg0_t a0 = *(arg0_t *)args;                                                                       \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                    \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                   \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                  \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t)); \
            func(a0, a1, a2, a3, a4);                                                                          \
        };                                                                                                     \
    }

#define neko_lua_auto_function_declare6(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                            \
    struct __neko_lua_auto_wrap_##func {                                                                                        \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                             \
            arg0_t a0 = *(arg0_t *)args;                                                                                        \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                     \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                    \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                   \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                  \
            arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t)); \
            *(ret_t *)out = func(a0, a1, a2, a3, a4, a5);                                                                       \
        };                                                                                                                      \
    }

#define neko_lua_auto_function_declare6_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                       \
    struct __neko_lua_auto_wrap_##func {                                                                                        \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                             \
            arg0_t a0 = *(arg0_t *)args;                                                                                        \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                     \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                    \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                   \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                  \
            arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t)); \
            func(a0, a1, a2, a3, a4, a5);                                                                                       \
        };                                                                                                                      \
    }

#define neko_lua_auto_function_declare7(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                     \
    struct __neko_lua_auto_wrap_##func {                                                                                                         \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                              \
            arg0_t a0 = *(arg0_t *)args;                                                                                                         \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                      \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                     \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                    \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                   \
            arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                  \
            arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t)); \
            *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6);                                                                                    \
        };                                                                                                                                       \
    }

#define neko_lua_auto_function_declare7_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                \
    struct __neko_lua_auto_wrap_##func {                                                                                                         \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                              \
            arg0_t a0 = *(arg0_t *)args;                                                                                                         \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                      \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                     \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                    \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                   \
            arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                  \
            arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t)); \
            func(a0, a1, a2, a3, a4, a5, a6);                                                                                                    \
        };                                                                                                                                       \
    }

#define neko_lua_auto_function_declare8(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                              \
    struct __neko_lua_auto_wrap_##func {                                                                                                                          \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                                               \
            arg0_t a0 = *(arg0_t *)args;                                                                                                                          \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                       \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                      \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                     \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                    \
            arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                   \
            arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                  \
            arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t)); \
            *(ret_t *)out = func(a0, a1, a2, a3, a4, a5, a6, a7);                                                                                                 \
        };                                                                                                                                                        \
    }

#define neko_lua_auto_function_declare8_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                         \
    struct __neko_lua_auto_wrap_##func {                                                                                                                          \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                                               \
            arg0_t a0 = *(arg0_t *)args;                                                                                                                          \
            arg1_t a1 = *(arg1_t *)(args + sizeof(arg0_t));                                                                                                       \
            arg2_t a2 = *(arg2_t *)(args + sizeof(arg0_t) + sizeof(arg1_t));                                                                                      \
            arg3_t a3 = *(arg3_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t));                                                                     \
            arg4_t a4 = *(arg4_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t));                                                    \
            arg5_t a5 = *(arg5_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t));                                   \
            arg6_t a6 = *(arg6_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t));                  \
            arg7_t a7 = *(arg7_t *)(args + sizeof(arg0_t) + sizeof(arg1_t) + sizeof(arg2_t) + sizeof(arg3_t) + sizeof(arg4_t) + sizeof(arg5_t) + sizeof(arg6_t)); \
            func(a0, a1, a2, a3, a4, a5, a6, a7);                                                                                                                 \
        };                                                                                                                                                        \
    }

#define neko_lua_auto_function_declare9(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                       \
    struct __neko_lua_auto_wrap_##func {                                                                                                                                           \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                                                                \
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
        };                                                                                                                                                                         \
    }

#define neko_lua_auto_function_declare9_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                  \
    struct __neko_lua_auto_wrap_##func {                                                                                                                                           \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                                                                \
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
        };                                                                                                                                                                         \
    }

#define neko_lua_auto_function_declare10(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                               \
    struct __neko_lua_auto_wrap_##func {                                                                                                                                                            \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                                                                                 \
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
        };                                                                                                                                                                                          \
    }

#define neko_lua_auto_function_declare10_void(func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                          \
    struct __neko_lua_auto_wrap_##func {                                                                                                                                                            \
        static void __neko_lua_auto_##func(char *out, char *args) {                                                                                                                                 \
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
        };                                                                                                                                                                                          \
    }

#define neko_lua_auto_function_register0(L, func, ret_t) \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 0)

#define neko_lua_auto_function_register1(L, func, ret_t, arg0_t) \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 1, neko_lua_auto_type(L, arg0_t))

#define neko_lua_auto_function_register2(L, func, ret_t, arg0_t, arg1_t)                                                                                                                          \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 2, neko_lua_auto_type(L, arg0_t), \
                                         neko_lua_auto_type(L, arg1_t))

#define neko_lua_auto_function_register3(L, func, ret_t, arg0_t, arg1_t, arg2_t)                                                                                                                  \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 3, neko_lua_auto_type(L, arg0_t), \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t))

#define neko_lua_auto_function_register4(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t)                                                                                                          \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 4, neko_lua_auto_type(L, arg0_t), \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t))

#define neko_lua_auto_function_register5(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t)                                                                                                  \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 5, neko_lua_auto_type(L, arg0_t), \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t))

#define neko_lua_auto_function_register6(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)                                                                                          \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 6, neko_lua_auto_type(L, arg0_t), \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t))

#define neko_lua_auto_function_register7(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t)                                                                                    \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 7, neko_lua_auto_type(L, arg0_t),   \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), \
                                         neko_lua_auto_type(L, arg6_t))

#define neko_lua_auto_function_register8(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t)                                                                            \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 8, neko_lua_auto_type(L, arg0_t),   \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), \
                                         neko_lua_auto_type(L, arg6_t), neko_lua_auto_type(L, arg7_t))

#define neko_lua_auto_function_register9(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t)                                                                    \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 9, neko_lua_auto_type(L, arg0_t),   \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), \
                                         neko_lua_auto_type(L, arg6_t), neko_lua_auto_type(L, arg7_t), neko_lua_auto_type(L, arg8_t))

#define neko_lua_auto_function_register10(L, func, ret_t, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t, arg6_t, arg7_t, arg8_t, arg9_t)                                                           \
    neko_lua_auto_function_register_type(L, func, (neko_lua_auto_Func)__neko_lua_auto_wrap_##func::__neko_lua_auto_##func, #func, neko_lua_auto_type(L, ret_t), 10, neko_lua_auto_type(L, arg0_t),  \
                                         neko_lua_auto_type(L, arg1_t), neko_lua_auto_type(L, arg2_t), neko_lua_auto_type(L, arg3_t), neko_lua_auto_type(L, arg4_t), neko_lua_auto_type(L, arg5_t), \
                                         neko_lua_auto_type(L, arg6_t), neko_lua_auto_type(L, arg7_t), neko_lua_auto_type(L, arg8_t), neko_lua_auto_type(L, arg9_t))

#else

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

#endif

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

}  // namespace neko

namespace neko {
class neko_lua_state_view {
public:
    neko_lua_state_view() noexcept : L{nullptr} {}
    neko_lua_state_view(lua_State *L) noexcept : L{L} {}

    lua_State *GetState() { return L; }
    bool Valid() const { return L != nullptr; }
    operator lua_State *() { return L; }

    int absindex(int idx);
    void arith(int op);
    lua_CFunction atpanic(lua_CFunction panicf);
    void call(int nargs, int nresults);
    void callk(int nargs, int nresults, lua_KContext ctx, lua_KFunction k);
    int checkstack(int n);
    int compare(int index1, int index2, int op);
    void concat(int n);
    void copy(int fromidx, int toidx);
    void createtable(int narr, int nrec);
    int dump(lua_Writer writer, void *data, int strip);
    int error();
    int gc(int what, int data);
    lua_Alloc getallocf(void **ud);
    int getfield(int index, const char *k);
    int getglobal(const char *name);
    lua_Hook gethook();
    int gethookcount();
    int gethookmask();
    int geti(int index, lua_Integer i);
    int getinfo(const char *what, lua_Debug *ar);
    const char *getlocal(const lua_Debug *ar, int n);
    int getmetatable(int index);
    int getstack(int level, lua_Debug *ar);
    int gettable(int index);
    int gettop();
    const char *getupvalue(int funcindex, int n);
    int getuservalue(int index);
    void insert(int index);
    int isboolean(int index);
    int iscfunction(int index);
    int isfunction(int index);
    int isinteger(int index);
    int islightuserdata(int index);
    int isnil(int index);
    int isnone(int index);
    int isnoneornil(int index);
    int isnumber(int index);
    int isstring(int index);
    int istable(int index);
    int isthread(int index);
    int isuserdata(int index);
    void len(int index);
    int load(lua_Reader reader, void *data, const char *chunkname, const char *mode);
    void newtable();
    void *newuserdata(size_t size);
    int next(int index);
    int pcall(int nargs, int nresults, int msgh);
    int pcallk(int nargs, int nresults, int msgh, lua_KContext ctx, lua_KFunction k);
    void pop(int n);
    void pushboolean(int b);
    void pushcclosure(lua_CFunction fn, int n);
    void pushcfunction(lua_CFunction f);
    const char *pushfstring(const char *fmt, ...);
    void pushglobaltable();
    void pushinteger(lua_Integer n);
    void pushlightuserdata(void *p);
    const char *pushlstring(const char *s, size_t len);
    void pushnil();
    void pushnumber(lua_Number n);
    const char *pushstring(const char *s);
    int pushthread();
    void pushvalue(int index);
    const char *pushvfstring(const char *fmt, va_list argp);
    int rawequal(int index1, int index2);
    int rawget(int index);
    int rawgeti(int index, lua_Integer n);
    int rawgetp(int index, const void *p);
    size_t rawlen(int index);
    void rawset(int index);
    void rawseti(int index, lua_Integer i);
    void rawsetp(int index, const void *p);
    void register_(const char *name, lua_CFunction f);
    void remove(int index);
    void replace(int index);
    int resume(lua_State *from, int nargs, int *nresults);
    void rotate(int idx, int n);
    void setallocf(lua_Alloc f, void *ud);
    void setfield(int index, const char *k);
    void setglobal(const char *name);
    void seti(int index, lua_Integer n);
    int setmetatable(int index);
    void settable(int index);
    void settop(int index);
    void setuservalue(int index);
    int status();
    size_t stringtonumber(const char *s);
    int toboolean(int index);
    lua_CFunction tocfunction(int index);
    lua_Integer tointeger(int index);
    lua_Integer tointegerx(int index, int *isnum);
    const char *tolstring(int index, size_t *len);
    lua_Number tonumber(int index);
    lua_Number tonumberx(int index, int *isnum);
    const void *topointer(int index);
    const char *tostring(int index);
    void *touserdata(int index);
    int type(int index);
    const char *typename_(int tp);
    void xmove(lua_State *from, lua_State *to, int n);
    int yield(int nresults);
    void sethook(lua_Hook f, int mask, int count);
    const char *setlocal(const lua_Debug *ar, int n);
    const char *setupvalue(int funcindex, int n);
    void argcheck(int cond, int arg, const char *extramsg);
    int argerror(int arg, const char *extramsg);
    void buffinit(luaL_Buffer *B);
    char *buffinitsize(luaL_Buffer *B, size_t sz);
    int callmeta(int obj, const char *e);
    void checkany(int arg);
    lua_Integer checkinteger(int arg);
    const char *checklstring(int arg, size_t *l);
    lua_Number checknumber(int arg);
    int checkoption(int arg, const char *def, const char *const lst[]);
    void checkstack(int sz, const char *msg);
    const char *checkstring(int arg);
    void checktype(int arg, int t);
    void *checkudata(int arg, const char *tname);
    void checkversion();
    int dofile(const char *filename);
    int dostring(const char *str);
    int error(const char *fmt, ...);
    int execresult(int stat);
    int fileresult(int stat, const char *fname);
    int getmetafield(int obj, const char *e);
    int getmetatable(const char *tname);
    int getsubtable(int idx, const char *fname);
    const char *gsub(const char *s, const char *p, const char *r);
    lua_Integer lenL(int index);
    int loadbuffer(const char *buff, size_t sz, const char *name);
    int loadbufferx(const char *buff, size_t sz, const char *name, const char *mode);
    int loadfile(const char *filename);
    int loadfilex(const char *filename, const char *mode);
    int loadstring(const char *s);
    template <size_t N>
    void newlib(const luaL_Reg (&l)[N]);
    template <size_t N>
    void newlibtable(const luaL_Reg (&l)[N]);
    int newmetatable(const char *tname);
    void openlibs();
    lua_Integer optinteger(int arg, lua_Integer d);
    const char *optlstring(int arg, const char *d, size_t *l);
    lua_Number optnumber(int arg, lua_Number d);
    const char *optstring(int arg, const char *d);
    int ref(int t);
    void requiref(const char *modname, lua_CFunction openf, int glb);
    void setfuncs(const luaL_Reg *l, int nup);
    void setmetatable(const char *tname);
    void *testudata(int arg, const char *tname);
    const char *tolstringL(int idx, size_t *len);
    const char *typenameL(int tp);
    void unref(int t, int ref);
    void where(int lvl);

protected:
    lua_State *L;
};
}  // namespace neko

namespace neko {
class neko_lua_ref {
public:
    neko_lua_ref() noexcept : t{0}, ref{LUA_REFNIL} {}
    neko_lua_ref(neko_lua_state_view L, int t) : L{L}, t{t} { ref = L.ref(t); }
    neko_lua_ref(neko_lua_state_view L) : neko_lua_ref{L, LUA_REGISTRYINDEX} {}
    neko_lua_state_view GetView() const noexcept { return L; }
    bool IsNil() const noexcept { return ref == LUA_REFNIL; }
    int Get() { return L.rawgeti(t, ref); }
    ~neko_lua_ref() {
        if (IsNil()) return;
        L.unref(t, ref);
    }
    neko_lua_ref(neko_lua_ref &&rhs) noexcept : L{rhs.L}, t{rhs.t}, ref{rhs.ref} {
        rhs.L = neko_lua_state_view{};
        rhs.t = 0;
        rhs.ref = LUA_REFNIL;
    }
    neko_lua_ref(const neko_lua_ref &) = delete;
    neko_lua_ref &operator=(const neko_lua_ref &) = delete;
    neko_lua_ref &operator=(neko_lua_ref &&rhs) noexcept {
        std::swap(L, rhs.L);
        std::swap(t, rhs.t);
        std::swap(ref, rhs.ref);
        return *this;
    }

private:
    neko_lua_state_view L;
    int t;
    int ref;
};
}  // namespace neko

// ==========================

template <size_t N>
void neko::neko_lua_state_view::newlib(const luaL_Reg (&l)[N]) {
    luaL_newlib(L, l);
}
template <size_t N>
void neko::neko_lua_state_view::newlibtable(const luaL_Reg (&l)[N]) {
    luaL_newlibtable(L, l);
}

namespace neko {
class neko_lua_state : public neko_lua_state_view {
public:
    neko_lua_state();
    neko_lua_state(lua_State *main, neko_lua_state_view L);

    ~neko_lua_state();

    lua_State *GetMainState() { return main; }
    neko_lua_state_view MainView() { return main; }

    neko_lua_state newthread();

    operator lua_State *() { return GetMainState(); }
    // operator int() { return (int)GetMainState(); }

private:
    lua_State *main;
};
}  // namespace neko

#endif