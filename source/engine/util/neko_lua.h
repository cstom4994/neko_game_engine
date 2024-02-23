
#ifndef NEKO_LUA_H
#define NEKO_LUA_H

#include "engine/neko.h"

// lua
#include "libs/lua/lauxlib.h"
#include "libs/lua/lua.h"
#include "libs/lua/lualib.h"

#ifndef _WIN32
#include <stdint.h>
#define SPRINTF_F snprintf
#else

#define SPRINTF_F _snprintf_s

#endif

#pragma region LuaA

#define NEKO_LUA_AUTO_REGISTER_PREFIX "neko_lua_auto_"

NEKO_API_DECL void __neko_lua_auto_open(lua_State *L);
NEKO_API_DECL void __neko_lua_auto_close(lua_State *L);

#define neko_lua_auto_type(L, type) neko_lua_auto_type_add(L, #type, sizeof(type))

enum { LUAA_INVALID_TYPE = -1 };

typedef lua_Integer neko_lua_auto_Type;
typedef int (*neko_lua_auto_Pushfunc)(lua_State *, neko_lua_auto_Type, const void *);
typedef void (*neko_lua_auto_Tofunc)(lua_State *, neko_lua_auto_Type, void *, int);

NEKO_API_DECL neko_lua_auto_Type neko_lua_auto_type_add(lua_State *L, const char *type, size_t size);
NEKO_API_DECL neko_lua_auto_Type neko_lua_auto_type_find(lua_State *L, const char *type);

NEKO_API_DECL const char *neko_lua_auto_typename(lua_State *L, neko_lua_auto_Type id);
NEKO_API_DECL size_t neko_lua_auto_typesize(lua_State *L, neko_lua_auto_Type id);

#define neko_lua_auto_push(L, type, c_in) neko_lua_auto_push_type(L, neko_lua_auto_type(L, type), c_in)
#define neko_lua_auto_to(L, type, c_out, index) neko_lua_auto_to_type(L, neko_lua_auto_type(L, type), c_out, index)

#define neko_lua_auto_conversion(L, type, push_func, to_func) neko_lua_auto_conversion_type(L, neko_lua_auto_type(L, type), push_func, to_func);
#define neko_lua_auto_conversion_push(L, type, func) neko_lua_auto_conversion_push_type(L, neko_lua_auto_type(L, type), func)
#define neko_lua_auto_conversion_to(L, type, func) neko_lua_auto_conversion_to_type(L, neko_lua_auto_type(L, type), func)

#define neko_lua_auto_conversion_registered(L, type) neko_lua_auto_conversion_registered_type(L, neko_lua_auto_type(L, type));
#define neko_lua_auto_conversion_push_registered(L, type) neko_lua_auto_conversion_push_registered_typ(L, neko_lua_auto_type(L, type));
#define neko_lua_auto_conversion_to_registered(L, type) neko_lua_auto_conversion_to_registered_type(L, neko_lua_auto_type(L, type));

NEKO_API_DECL int neko_lua_auto_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in);
NEKO_API_DECL void neko_lua_auto_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index);

NEKO_API_DECL void neko_lua_auto_conversion_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Pushfunc push_func, neko_lua_auto_Tofunc to_func);
NEKO_API_DECL void neko_lua_auto_conversion_push_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Pushfunc func);
NEKO_API_DECL void neko_lua_auto_conversion_to_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Tofunc func);

NEKO_API_DECL bool neko_lua_auto_conversion_registered_type(lua_State *L, neko_lua_auto_Type type);
NEKO_API_DECL bool neko_lua_auto_conversion_push_registered_type(lua_State *L, neko_lua_auto_Type type);
NEKO_API_DECL bool neko_lua_auto_conversion_to_registered_type(lua_State *L, neko_lua_auto_Type type);

NEKO_API_DECL int neko_lua_auto_push_bool(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_char(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_signed_char(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_unsigned_char(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_short(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_unsigned_short(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_int(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_unsigned_int(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_unsigned_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_long_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_unsigned_long_long(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_float(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_double(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_long_double(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_char_ptr(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_const_char_ptr(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_void_ptr(lua_State *L, neko_lua_auto_Type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_push_void(lua_State *L, neko_lua_auto_Type, const void *c_in);

NEKO_API_DECL void neko_lua_auto_to_bool(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_char(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_signed_char(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_unsigned_char(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_short(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_unsigned_short(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_int(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_unsigned_int(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_unsigned_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_long_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_unsigned_long_long(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_float(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_double(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_long_double(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_char_ptr(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_const_char_ptr(lua_State *L, neko_lua_auto_Type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_to_void_ptr(lua_State *L, neko_lua_auto_Type, void *c_out, int index);

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

NEKO_API_DECL void neko_lua_auto_struct_type(lua_State *L, neko_lua_auto_Type type);
NEKO_API_DECL void neko_lua_auto_struct_member_type(lua_State *L, neko_lua_auto_Type type, const char *member, neko_lua_auto_Type member_type, size_t offset);

NEKO_API_DECL int neko_lua_auto_struct_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in);
NEKO_API_DECL int neko_lua_auto_struct_push_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset, const void *c_in);
NEKO_API_DECL int neko_lua_auto_struct_push_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member, const void *c_in);

NEKO_API_DECL void neko_lua_auto_struct_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_struct_to_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset, void *c_out, int index);
NEKO_API_DECL void neko_lua_auto_struct_to_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member, void *c_out, int index);

NEKO_API_DECL bool neko_lua_auto_struct_has_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset);
NEKO_API_DECL bool neko_lua_auto_struct_has_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

NEKO_API_DECL neko_lua_auto_Type neko_lua_auto_struct_typeof_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset);
NEKO_API_DECL neko_lua_auto_Type neko_lua_auto_struct_typeof_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

NEKO_API_DECL bool neko_lua_auto_struct_registered_type(lua_State *L, neko_lua_auto_Type type);

NEKO_API_DECL const char *neko_lua_auto_struct_next_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

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

NEKO_API_DECL void neko_lua_auto_enum_type(lua_State *L, neko_lua_auto_Type type, size_t size);
NEKO_API_DECL void neko_lua_auto_enum_value_type(lua_State *L, neko_lua_auto_Type type, const void *value, const char *name);

NEKO_API_DECL int neko_lua_auto_enum_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in);
NEKO_API_DECL void neko_lua_auto_enum_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index);

NEKO_API_DECL bool neko_lua_auto_enum_has_value_type(lua_State *L, neko_lua_auto_Type type, const void *value);
NEKO_API_DECL bool neko_lua_auto_enum_has_name_type(lua_State *L, neko_lua_auto_Type type, const char *name);

NEKO_API_DECL bool neko_lua_auto_enum_registered_type(lua_State *L, neko_lua_auto_Type type);
NEKO_API_DECL const char *neko_lua_auto_enum_next_value_name_type(lua_State *L, neko_lua_auto_Type type, const char *member);

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

#define neko_lua_register(FUNCTIONS)                              \
    for (unsigned i = 0; i < neko_arr_size(FUNCTIONS) - 1; ++i) { \
        lua_pushcfunction(L, FUNCTIONS[i].func);                  \
        lua_setglobal(L, FUNCTIONS[i].name);                      \
    }

neko_inline bool neko_lua_equal(lua_State *state, int index1, int index2) {
#if LUA_VERSION_NUM <= 501
    return lua_equal(state, index1, index2) == 1;
#else
    return lua_compare(state, index1, index2, LUA_OPEQ) == 1;
#endif
}

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name);
int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name);
void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name);
void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name);

NEKO_API_DECL int luaopen_cstruct_core(lua_State *L);
NEKO_API_DECL int luaopen_cstruct_test(lua_State *L);
NEKO_API_DECL int luaopen_datalist(lua_State *L);
NEKO_API_DECL int luaopen_ds_core(lua_State *L);

// neko
NEKO_API_DECL int luaopen_neko_ecs(lua_State *L);

#endif
