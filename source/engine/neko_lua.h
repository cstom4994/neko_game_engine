
#ifndef NEKO_LUA_H
#define NEKO_LUA_H

#include "engine/neko.h"

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

#define NEKO_LUA_AUTO_REGISTER_PREFIX "neko_luabind_"

NEKO_API_DECL void __neko_luabind_init(lua_State *L);
NEKO_API_DECL void __neko_luabind_fini(lua_State *L);

#define neko_luabind_type(L, type) neko_luabind_type_add(L, #type, sizeof(type))

enum { LUAA_INVALID_TYPE = -1 };

typedef lua_Integer neko_luabind_Type;
typedef int (*neko_luabind_Pushfunc)(lua_State *, neko_luabind_Type, const void *);
typedef void (*neko_luabind_Tofunc)(lua_State *, neko_luabind_Type, void *, int);

NEKO_API_DECL neko_luabind_Type neko_luabind_type_add(lua_State *L, const char *type, size_t size);
NEKO_API_DECL neko_luabind_Type neko_luabind_type_find(lua_State *L, const char *type);

NEKO_API_DECL const char *neko_luabind_typename(lua_State *L, neko_luabind_Type id);
NEKO_API_DECL size_t neko_luabind_typesize(lua_State *L, neko_luabind_Type id);

#define neko_luabind_push(L, type, c_in) neko_luabind_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_to(L, type, c_out, index) neko_luabind_to_type(L, neko_luabind_type(L, type), c_out, index)

#define neko_luabind_conversion(L, type, push_func, to_func) neko_luabind_conversion_type(L, neko_luabind_type(L, type), push_func, to_func);
#define neko_luabind_conversion_push(L, type, func) neko_luabind_conversion_push_type(L, neko_luabind_type(L, type), func)
#define neko_luabind_conversion_to(L, type, func) neko_luabind_conversion_to_type(L, neko_luabind_type(L, type), func)

#define neko_luabind_conversion_registered(L, type) neko_luabind_conversion_registered_type(L, neko_luabind_type(L, type));
#define neko_luabind_conversion_push_registered(L, type) neko_luabind_conversion_push_registered_typ(L, neko_luabind_type(L, type));
#define neko_luabind_conversion_to_registered(L, type) neko_luabind_conversion_to_registered_type(L, neko_luabind_type(L, type));

NEKO_API_DECL int neko_luabind_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
NEKO_API_DECL void neko_luabind_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

NEKO_API_DECL void neko_luabind_conversion_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Pushfunc push_func, neko_luabind_Tofunc to_func);
NEKO_API_DECL void neko_luabind_conversion_push_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Pushfunc func);
NEKO_API_DECL void neko_luabind_conversion_to_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Tofunc func);

NEKO_API_DECL bool neko_luabind_conversion_registered_type(lua_State *L, neko_luabind_Type type);
NEKO_API_DECL bool neko_luabind_conversion_push_registered_type(lua_State *L, neko_luabind_Type type);
NEKO_API_DECL bool neko_luabind_conversion_to_registered_type(lua_State *L, neko_luabind_Type type);

NEKO_API_DECL int neko_luabind_push_bool(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_char(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_signed_char(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_unsigned_char(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_short(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_unsigned_short(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_int(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_unsigned_int(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_long(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_unsigned_long(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_long_long(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_unsigned_long_long(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_float(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_double(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_long_double(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_char_ptr(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_const_char_ptr(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_void_ptr(lua_State *L, neko_luabind_Type, const void *c_in);
NEKO_API_DECL int neko_luabind_push_void(lua_State *L, neko_luabind_Type, const void *c_in);

NEKO_API_DECL void neko_luabind_to_bool(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_char(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_signed_char(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_unsigned_char(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_short(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_unsigned_short(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_int(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_unsigned_int(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_unsigned_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_long_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_unsigned_long_long(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_float(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_double(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_long_double(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_char_ptr(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_const_char_ptr(lua_State *L, neko_luabind_Type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_to_void_ptr(lua_State *L, neko_luabind_Type, void *c_out, int index);

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

NEKO_API_DECL void neko_luabind_struct_type(lua_State *L, neko_luabind_Type type);
NEKO_API_DECL void neko_luabind_struct_member_type(lua_State *L, neko_luabind_Type type, const char *member, neko_luabind_Type member_type, size_t offset);

NEKO_API_DECL int neko_luabind_struct_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
NEKO_API_DECL int neko_luabind_struct_push_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset, const void *c_in);
NEKO_API_DECL int neko_luabind_struct_push_member_name_type(lua_State *L, neko_luabind_Type type, const char *member, const void *c_in);

NEKO_API_DECL void neko_luabind_struct_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);
NEKO_API_DECL void neko_luabind_struct_to_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset, void *c_out, int index);
NEKO_API_DECL void neko_luabind_struct_to_member_name_type(lua_State *L, neko_luabind_Type type, const char *member, void *c_out, int index);

NEKO_API_DECL bool neko_luabind_struct_has_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset);
NEKO_API_DECL bool neko_luabind_struct_has_member_name_type(lua_State *L, neko_luabind_Type type, const char *member);

NEKO_API_DECL neko_luabind_Type neko_luabind_struct_typeof_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset);
NEKO_API_DECL neko_luabind_Type neko_luabind_struct_typeof_member_name_type(lua_State *L, neko_luabind_Type type, const char *member);

NEKO_API_DECL bool neko_luabind_struct_registered_type(lua_State *L, neko_luabind_Type type);

NEKO_API_DECL const char *neko_luabind_struct_next_member_name_type(lua_State *L, neko_luabind_Type type, const char *member);

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

NEKO_API_DECL void neko_luabind_enum_type(lua_State *L, neko_luabind_Type type, size_t size);
NEKO_API_DECL void neko_luabind_enum_value_type(lua_State *L, neko_luabind_Type type, const void *value, const char *name);

NEKO_API_DECL int neko_luabind_enum_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
NEKO_API_DECL void neko_luabind_enum_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

NEKO_API_DECL bool neko_luabind_enum_has_value_type(lua_State *L, neko_luabind_Type type, const void *value);
NEKO_API_DECL bool neko_luabind_enum_has_name_type(lua_State *L, neko_luabind_Type type, const char *name);

NEKO_API_DECL bool neko_luabind_enum_registered_type(lua_State *L, neko_luabind_Type type);
NEKO_API_DECL const char *neko_luabind_enum_next_value_name_type(lua_State *L, neko_luabind_Type type, const char *member);

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

NEKO_API_DECL bool neko_lua_equal(lua_State *state, int index1, int index2);

NEKO_API_DECL int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name);
NEKO_API_DECL int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name);
NEKO_API_DECL void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name);
NEKO_API_DECL void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name);
NEKO_API_DECL int neko_lua_get_table_pairs_count(lua_State *L, int index);

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

NEKO_API_DECL int LUASTRUCT_new(lua_State *L, const char *metatable, size_t size);
NEKO_API_DECL int LUASTRUCT_newref(lua_State *L, const char *metatable, int parentIndex, const void *data);
NEKO_API_DECL int LUASTRUCT_is(lua_State *L, const char *metatable, int index);
NEKO_API_DECL void *LUASTRUCT_todata(lua_State *L, const char *metatable, int index, int required);

// neko_tolua based on tolua by Ariel Manzur (www.tecgraf.puc-rio.br/~celes/tolua)
// it's licensed under the terms of the MIT license
#define NEKO_TOLUA

#ifdef NEKO_TOLUA

#define NEKO_TOLUA_VERSION "neko-tolua-0.1"

#define neko_tolua_pushcppstring(x, y) neko_tolua_pushstring(x, y.c_str())
#define neko_tolua_iscppstring neko_tolua_isstring

#define neko_tolua_iscppstringarray neko_tolua_isstringarray
#define neko_tolua_pushfieldcppstring(L, lo, idx, s) neko_tolua_pushfieldstring(L, lo, idx, s.c_str())

#ifndef TEMPLATE_BIND
#define TEMPLATE_BIND(p)
#endif

#define TOLUA_TEMPLATE_BIND(p)

#define TOLUA_PROTECTED_DESTRUCTOR
#define TOLUA_PROPERTY_TYPE(p)

typedef int lua_Object;

struct neko_tolua_Error {
    int index;
    int array;
    const char *type;
};
typedef struct neko_tolua_Error neko_tolua_Error;

#define TOLUA_NOPEER LUA_REGISTRYINDEX /* for lua 5.1 */

NEKO_API_DECL const char *neko_tolua_typename(lua_State *L, int lo);
NEKO_API_DECL void neko_tolua_error(lua_State *L, const char *msg, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isnoobj(lua_State *L, int lo, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isvalue(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isvaluenil(lua_State *L, int lo, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isboolean(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isnumber(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isinteger(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isstring(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_istable(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isusertable(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isuserdata(lua_State *L, int lo, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isusertype(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isvaluearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isbooleanarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isnumberarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isintegerarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isstringarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_istablearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isuserdataarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
NEKO_API_DECL int neko_tolua_isusertypearray(lua_State *L, int lo, const char *type, int dim, int def, neko_tolua_Error *err);

NEKO_API_DECL void neko_tolua_open(lua_State *L);

NEKO_API_DECL void *neko_tolua_copy(lua_State *L, void *value, unsigned int size);
NEKO_API_DECL int neko_tolua_register_gc(lua_State *L, int lo);
NEKO_API_DECL int neko_tolua_default_collect(lua_State *L);

NEKO_API_DECL void neko_tolua_usertype(lua_State *L, const char *type);
NEKO_API_DECL void neko_tolua_beginmodule(lua_State *L, const char *name);
NEKO_API_DECL void neko_tolua_endmodule(lua_State *L);
NEKO_API_DECL void neko_tolua_module(lua_State *L, const char *name, int hasvar);
NEKO_API_DECL void neko_tolua_class(lua_State *L, const char *name, const char *base);
NEKO_API_DECL void neko_tolua_cclass(lua_State *L, const char *lname, const char *name, const char *base, lua_CFunction col);
NEKO_API_DECL void neko_tolua_function(lua_State *L, const char *name, lua_CFunction func);
NEKO_API_DECL void neko_tolua_constant(lua_State *L, const char *name, lua_Number value);
NEKO_API_DECL void neko_tolua_variable(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set);
NEKO_API_DECL void neko_tolua_array(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set);

/* NEKO_API_DECL void neko_tolua_set_call_event(lua_State* L, lua_CFunction func, char* type); */
/* NEKO_API_DECL void neko_tolua_addbase(lua_State* L, char* name, char* base); */

NEKO_API_DECL void neko_tolua_pushvalue(lua_State *L, int lo);
NEKO_API_DECL void neko_tolua_pushboolean(lua_State *L, int value);
NEKO_API_DECL void neko_tolua_pushnumber(lua_State *L, lua_Number value);
NEKO_API_DECL void neko_tolua_pushinteger(lua_State *L, lua_Integer value);
NEKO_API_DECL void neko_tolua_pushstring(lua_State *L, const char *value);
NEKO_API_DECL void neko_tolua_pushuserdata(lua_State *L, void *value);
NEKO_API_DECL void neko_tolua_pushusertype(lua_State *L, void *value, const char *type);
NEKO_API_DECL void neko_tolua_pushusertype_and_takeownership(lua_State *L, void *value, const char *type);
NEKO_API_DECL void neko_tolua_pushfieldvalue(lua_State *L, int lo, int index, int v);
NEKO_API_DECL void neko_tolua_pushfieldboolean(lua_State *L, int lo, int index, int v);
NEKO_API_DECL void neko_tolua_pushfieldnumber(lua_State *L, int lo, int index, lua_Number v);
NEKO_API_DECL void neko_tolua_pushfieldinteger(lua_State *L, int lo, int index, lua_Integer v);
NEKO_API_DECL void neko_tolua_pushfieldstring(lua_State *L, int lo, int index, const char *v);
NEKO_API_DECL void neko_tolua_pushfielduserdata(lua_State *L, int lo, int index, void *v);
NEKO_API_DECL void neko_tolua_pushfieldusertype(lua_State *L, int lo, int index, void *v, const char *type);
NEKO_API_DECL void neko_tolua_pushfieldusertype_and_takeownership(lua_State *L, int lo, int index, void *v, const char *type);

NEKO_API_DECL lua_Number neko_tolua_tonumber(lua_State *L, int narg, lua_Number def);
NEKO_API_DECL lua_Integer neko_tolua_tointeger(lua_State *L, int narg, lua_Integer def);
NEKO_API_DECL const char *neko_tolua_tostring(lua_State *L, int narg, const char *def);
NEKO_API_DECL void *neko_tolua_touserdata(lua_State *L, int narg, void *def);
NEKO_API_DECL void *neko_tolua_tousertype(lua_State *L, int narg, void *def);
NEKO_API_DECL int neko_tolua_tovalue(lua_State *L, int narg, int def);
NEKO_API_DECL int neko_tolua_toboolean(lua_State *L, int narg, int def);
NEKO_API_DECL lua_Number neko_tolua_tofieldnumber(lua_State *L, int lo, int index, lua_Number def);
NEKO_API_DECL lua_Integer neko_tolua_tofieldinteger(lua_State *L, int lo, int index, lua_Integer def);
NEKO_API_DECL const char *neko_tolua_tofieldstring(lua_State *L, int lo, int index, const char *def);
NEKO_API_DECL void *neko_tolua_tofielduserdata(lua_State *L, int lo, int index, void *def);
NEKO_API_DECL void *neko_tolua_tofieldusertype(lua_State *L, int lo, int index, void *def);
NEKO_API_DECL int neko_tolua_tofieldvalue(lua_State *L, int lo, int index, int def);
NEKO_API_DECL int neko_tolua_getfieldboolean(lua_State *L, int lo, int index, int def);

NEKO_API_DECL void neko_tolua_dobuffer(lua_State *L, char *B, unsigned int size, const char *name);

NEKO_API_DECL int class_gc_event(lua_State *L);

#ifdef __cplusplus
static inline const char *neko_tolua_tocppstring(lua_State *L, int narg, const char *def) {
    const char *s = neko_tolua_tostring(L, narg, def);
    return s ? s : "";
};
static inline const char *neko_tolua_tofieldcppstring(lua_State *L, int lo, int index, const char *def) {

    const char *s = neko_tolua_tofieldstring(L, lo, index, def);
    return s ? s : "";
};
#else
#define neko_tolua_tocppstring neko_tolua_tostring
#define neko_tolua_tofieldcppstring neko_tolua_tofieldstring
#endif

NEKO_API_DECL int neko_tolua_fast_isa(lua_State *L, int mt_indexa, int mt_indexb, int super_index);

#ifndef Mneko_tolua_new
#define Mneko_tolua_new(EXP) new EXP
#endif

#ifndef Mneko_tolua_delete
#define Mneko_tolua_delete(EXP) delete EXP
#endif

#ifndef Mneko_tolua_new_dim
#define Mneko_tolua_new_dim(EXP, len) new EXP[len]
#endif

#ifndef Mneko_tolua_delete_dim
#define Mneko_tolua_delete_dim(EXP) delete[] EXP
#endif

#ifndef neko_tolua_outside
#define neko_tolua_outside
#endif

#ifndef neko_tolua_owned
#define neko_tolua_owned
#endif

#endif  // NEKO_TOLUA

#endif
