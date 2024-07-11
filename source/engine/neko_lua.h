
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

#define LUA_FUNCTION(F) static int F(lua_State *L)
#define LUABIND_MODULE(...) static int luaopen(lua_State *L)

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

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

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

#if !defined(NEKO_LUA_HPP)
#define NEKO_LUA_HPP

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "engine/neko_lua.h"

#ifndef _WIN32
#include <stdint.h>
#define SPRINTF_F snprintf
#else
#define SPRINTF_F _snprintf_s
#endif

#define INHERIT_TABLE "inherit_table"

#define lua_value __lua_op_t

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

class neko_lua_tool_t {
public:
    static void function_call_err(const_str name, int params) { NEKO_WARN("[lua] invalid parameters to function: \"%s\" as %d", name, params); }

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
    static inline const_str get_name() { return name; }
    static inline const_str get_inherit_name() { return inherit_name; }
    static inline bool is_registed() { return name[0] != '\0'; }
    static inline bool is_inherit() { return inherit_name[0] != '\0'; }
    static char name[128];
    static char inherit_name[128];
};
template <typename T>
char lua_type_info_t<T>::name[128] = {0};
template <typename T>
char lua_type_info_t<T>::inherit_name[128] = {0};

template <typename T>
struct init_value_traits_t;

template <typename T>
struct init_value_traits_t {
    static inline T value() { return T(); }
};

template <typename T>
struct init_value_traits_t<const T *> {
    static inline T *value() { return NULL; }
};

template <typename T>
struct init_value_traits_t<const T &> {
    static inline T value() { return T(); }
};

template <>
struct init_value_traits_t<std::string> {
    static inline const_str value() { return ""; }
};

template <>
struct init_value_traits_t<const std::string &> {
    static inline const_str value() { return ""; }
};

// 包含能够连接到脚本表的必要信息的类
class ScriptReference {
public:
    ScriptReference();
    virtual ~ScriptReference();

    //
    // @return The ID which connects this instance to a script-represented version of this class.
    //		0 if it isn't connected (i.e. it isn't registered).
    inline int getId() const { return script_ref; }

    lua_State *__lua() const;

    //
    // @return The state associated with this script reference object.
    // inline lua_State* getCurrentState() const { return __lua; }

protected:
    int script_ref;
    // lua_State* __lua;
};

template <typename T>
struct __lua_op_t {
    static void push_stack(lua_State *ls_, const_str arg_) { lua_pushstring(ls_, arg_); }
    /*
    static int lua_to_value(lua_State* ls_, int pos_, char*& param_)
    {
         const_str  str = luaL_checkstring(ls_, pos_);
        param_ = (char*)str;
        return 0;
    }*/

    static bool pop(lua_State *L, T &value) {
        NEKO_WARN("[lua] cannot pop unknown type: %s", typeid(T).name());
        lua_pop(L, 1);
        return true;
    }

    static void push(lua_State *L, T &value) {
        NEKO_WARN("[lua] cannot push unknown type: %s", typeid(T).name());
        lua_pushnil(L);
    }
};

template <>
struct __lua_op_t<const_str> {
    static void push_stack(lua_State *ls_, const_str arg_) { lua_pushstring(ls_, arg_); }
    static int lua_to_value(lua_State *ls_, int pos_, char *&param_) {
        const_str str = luaL_checkstring(ls_, pos_);
        param_ = (char *)str;
        return 0;
    }

    static bool pop(lua_State *L, const_str &value) {
        bool ok = lua_isstring(L, -1) == 1;
        if (ok) value = lua_tostring(L, -1);

        lua_pop(L, 1);
        return ok;
    }

    static void push(lua_State *L, const_str &value) { lua_pushstring(L, value); }
};

template <>
struct __lua_op_t<char *> {
    static void push_stack(lua_State *ls_, const_str arg_) { lua_pushstring(ls_, arg_); }
    static int lua_to_value(lua_State *ls_, int pos_, char *&param_) {
        const_str str = luaL_checkstring(ls_, pos_);
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

#define XX_TYPE(T, isfunc, checkfunc, tofunc, pushfunc)                         \
    template <>                                                                 \
    struct __lua_op_t<T> {                                                      \
        static bool pop(lua_State *L, T &value) {                               \
            bool ok = isfunc(L, -1) == 1;                                       \
            if (ok) value = (T)tofunc(L, -1);                                   \
            lua_pop(L, 1);                                                      \
            return ok;                                                          \
        }                                                                       \
        static void push(lua_State *L, T &value) { pushfunc(L, value); }        \
        static void push_stack(lua_State *ls_, T arg_) { pushfunc(ls_, arg_); } \
        static int get_ret_value(lua_State *ls_, int pos_, T &param_) {         \
            if (!isfunc(ls_, pos_)) {                                           \
                return -1;                                                      \
            }                                                                   \
            param_ = (T)tofunc(ls_, pos_);                                      \
            return 0;                                                           \
        }                                                                       \
        static int lua_to_value(lua_State *ls_, int pos_, T &param_) {          \
            param_ = (T)checkfunc(ls_, pos_);                                   \
            return 0;                                                           \
        }                                                                       \
    }

XX_TYPE(s8, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(s16, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(s32, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(s64, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(u8, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(u16, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(u32, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(u64, lua_isnumber, luaL_checkinteger, lua_tointeger, lua_pushinteger);
XX_TYPE(f32, lua_isnumber, luaL_checknumber, lua_tonumber, lua_pushnumber);
XX_TYPE(f64, lua_isnumber, luaL_checknumber, lua_tonumber, lua_pushnumber);

#undef XX_TYPE

#if 0

template <>
struct __lua_op_t<int64_t> {
    static void push_stack(lua_State* ls_, int64_t arg_) { lua_pushinteger(ls_, arg_); }

    static int get_ret_value(lua_State* ls_, int pos_, int64_t& param_) {
        if (!lua_isinteger(ls_, pos_)) {
            return -1;
        }
        param_ = lua_tointeger(ls_, pos_);
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, int64_t& param_) {
        param_ = luaL_checkinteger(ls_, pos_);
        return 0;
    }
};

template <>
struct __lua_op_t<uint64_t> {
    static void push_stack(lua_State* ls_, uint64_t arg_) {
        std::stringstream ss;
        ss << arg_;
        std::string str = ss.str();
        lua_pushlstring(ls_, str.c_str(), str.length());
    }

    static int get_ret_value(lua_State* ls_, int pos_, uint64_t& param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        size_t len = 0;
        const_str src = lua_tolstring(ls_, pos_, &len);
        param_ = (uint64_t)strtoull(src, NULL, 10);
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, uint64_t& param_) {
        size_t len = 0;
        const_str str = luaL_checklstring(ls_, pos_, &len);
        param_ = (uint64_t)strtoull(str, NULL, 10);
        return 0;
    }
};

#endif

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

    static bool pop(lua_State *L, bool &value) {
        bool ok = lua_isboolean(L, -1);
        if (ok) value = lua_toboolean(L, -1) != 0;

        lua_pop(L, 1);
        return false;
    }

    static void push(lua_State *L, bool &value) { lua_pushboolean(L, value ? 1 : 0); }
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
        const_str src = lua_tolstring(ls_, -1, &len);
        param_.assign(src, len);
        lua_pop(ls_, 1);

        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, std::string &param_) {
        size_t len = 0;
        const_str str = luaL_checklstring(ls_, pos_, &len);
        param_.assign(str, len);
        return 0;
    }

    static bool pop(lua_State *L, std::string &value) {
        bool ok = lua_isstring(L, -1) == 1;
        if (ok) value = std::string(lua_tostring(L, -1));

        lua_pop(L, 1);
        return ok;
    }

    static void push(lua_State *L, std::string &value) { lua_pushstring(L, value.c_str()); }
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
        const_str src = lua_tolstring(ls_, -1, &len);
        param_.assign(src, len);
        lua_pop(ls_, 1);

        return 0;
    }
    static int lua_to_value(lua_State *ls_, int pos_, std::string &param_) {
        size_t len = 0;
        const_str str = luaL_checklstring(ls_, pos_, &len);
        param_.assign(str, len);
        return 0;
    }
};

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

    static bool pop(lua_State *L, T *&value) {
        bool ok = lua_istable(L, -1);
        if (ok) {
            lua_pushstring(L, "_instance");
            lua_gettable(L, -2);

            ok = lua_isuserdata(L, -1) == 1;
            if (ok) {
                void *userdata = lua_touserdata(L, -1);
                ScriptReference *refValue = reinterpret_cast<ScriptReference *>(userdata);
                value = dynamic_cast<T *>(refValue);
                ok = value != NULL;
                if (!ok) {
                    NEKO_WARN("[lua] cannot cast pointer to \"%s\"", typeid(T).name());
                }
            } else if (lua_isnil(L, -1)) {
                NEKO_WARN("[lua] a \"%s\" instance you are trying to use is deleted", typeid(T).name());
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        return ok;
    }

    static void push(lua_State *L, T *&value) {
        if (value == NULL || value->getId() == 0) {
            lua_pushnil(L);
            return;
        }

        lua_rawgeti(L, LUA_REGISTRYINDEX, value->getId());
    }
};

template <typename T>
struct __lua_op_t<const T *> {
    static void push_stack(lua_State *ls_, const T *arg_) { __lua_op_t<T *>::push_stack(ls_, (T *)arg_); }

    static int get_ret_value(lua_State *ls_, int pos_, T *&param_) { return __lua_op_t<T *>::get_ret_value(ls_, pos_, param_); }

    static int lua_to_value(lua_State *ls_, int pos_, T *&param_) { return __lua_op_t<T *>::lua_to_value(ls_, pos_, param_); }
};

template <typename T>
struct __lua_op_t<std::vector<T>> {
    static void push_stack(lua_State *ls_, const std::vector<T> &arg_) {
        lua_newtable(ls_);
        typename std::vector<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            __lua_op_t<int>::push_stack(ls_, i);
            __lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State *ls_, int pos_, std::vector<T> &param_) {
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

    static int lua_to_value(lua_State *ls_, int pos_, std::vector<T> &param_) {
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

    static bool pop(lua_State *L, std::vector<T> &value) {
#ifdef _DEBUG
        int top1 = lua_gettop(L);
#endif
        bool ok = lua_istable(L, -1);
        if (ok) {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                T item;
                if (lua_value<T>::pop(L, item)) {
                    value.push_back(item);
                }
            }
        }

        lua_pop(L, 1);
        return false;
    }

    static void push(lua_State *L, std::vector<T> &value) {
        NEKO_WARN("[lua] cannot push a std::vector type back to LUA at the moment");
        lua_pushnil(L);
    }
};

template <typename T>
struct __lua_op_t<std::list<T>> {
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
struct __lua_op_t<std::set<T>> {
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
struct __lua_op_t<std::map<K, V>> {
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

class ScriptObject;
class lua_table;

class lua_table_iter {
public:
    lua_table_iter();

public:
    lua_table_iter(lua_State *L, int script_ref_);
    lua_table_iter(const lua_table_iter &it);
    ~lua_table_iter();

    bool hasNext();
    std::string getKey() const;
    std::string getString();
    double getDouble();
    float getFloat();
    int getInt();
    bool getBool();

    ScriptObject *getPointer();

    lua_table get_table();

private:
    int script_ref;
    lua_State *__lua;
    int mNumPopsRequired;
};

class lua_table {
public:
    lua_table();
    lua_table(lua_State *L, int script_ref_);
    lua_table(const lua_table &other);
    ~lua_table();

    std::string getString(const_str key) const;
    double getDouble(const_str key) const;
    float getFloat(const_str key) const;
    bool getBool(const_str key) const;
    int getInt(const_str key) const;
    ScriptObject *getPointer(const_str key) const;
    lua_table get_table(const_str key) const;

    lua_table_iter getIterator() const;

    inline int getId() const { return script_ref; }

    lua_table &operator=(const lua_table &other);

private:
    int script_ref;
    lua_State *__lua;
};

template <class T>
class ScriptObjectPtr;

template <class C>
struct lua_value<ScriptObjectPtr<C>> {
    static bool pop(lua_State *L, ScriptObjectPtr<C> &value) {
        bool ok = lua_istable(L, -1);
        if (ok) {
            lua_pushstring(L, "_instance");
            lua_gettable(L, -2);

            ok = lua_isuserdata(L, -1) == 1;
            if (ok) {
                void *userdata = lua_touserdata(L, -1);
                ScriptReference *refValue = reinterpret_cast<ScriptReference *>(userdata);
                value = dynamic_cast<C *>(refValue);
                ok = value.exists();
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        return ok;
    }

    static void push(lua_State *L, ScriptObjectPtr<C> &value) {
        if (!value.exists() || value->getId() == 0) {
            lua_pushnil(L);
            return;
        }

        lua_rawgeti(L, LUA_REGISTRYINDEX, value->getId());
        lua_getref(L, value->getId());
    }
};

template <>
struct lua_value<lua_table> {
    static bool pop(lua_State *L, lua_table &value) {
        bool ok = lua_istable(L, -1);
        if (ok) {
            int ref = luaL_ref(L, LUA_REGISTRYINDEX);
            value = lua_table(L, ref);
        }
        return ok;
    }

    static void push(lua_State *L, lua_table &value) {
        NEKO_WARN("[lua] cannot push a table back to LUA at the moment");
        lua_pushnil(L);
    }
};

class ScriptInvoker : public ScriptReference {
public:
    ScriptInvoker();
    virtual ~ScriptInvoker();

    // 为此实例调用脚本方法
    void invoke(const_str method_name);

    bool isMethodDefined(const_str method_name) const;

#define __lua __lua()

    template <typename P1>
    void invoke(const_str method_name, P1 p1) {

#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif

        if (!findAndPushMethod(method_name)) return;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);

        // Call script
        if (lua_pcall(__lua, 2, 0, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
        }
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
    }

    template <typename P1, typename P2>
    void invoke(const_str method_name, P1 p1, P2 p2) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);
        lua_value<P2>::push(__lua, p2);

        // Call script
        if (lua_pcall(__lua, 3, 0, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
        }
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
    }

    template <typename P1, typename P2, typename P3>
    void invoke(const_str method_name, P1 p1, P2 p2, P3 p3) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif

        if (!findAndPushMethod(method_name)) return;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);
        lua_value<P2>::push(__lua, p2);
        lua_value<P3>::push(__lua, p3);

        // Call script
        if (lua_pcall(__lua, 4, 0, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
        }

#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
    }

    template <typename P1, typename P2, typename P3, typename P4>
    void invoke(const_str method_name, P1 p1, P2 p2, P3 p3, P4 p4) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);
        lua_value<P2>::push(__lua, p2);
        lua_value<P3>::push(__lua, p3);
        lua_value<P4>::push(__lua, p4);

        // Call script
        if (lua_pcall(__lua, 5, 0, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
        }

#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
    }

    template <typename R>
    bool invoke_get(const_str method_name, R &result) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return false;

        // Push this
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);

        // Call script
        if (lua_pcall(__lua, 1, 1, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
            return false;
        }

        bool ok = lua_value<R>::pop(__lua, result);
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
        return ok;
    }

    template <typename R, typename P1>
    bool invoke_get(const_str method_name, P1 p1, R &result) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return false;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);

        // Call script
        if (lua_pcall(__lua, 2, 1, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
            return false;
        }

        bool ok = lua_value<R>::pop(__lua, result);
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
        return ok;
    }

    template <typename R, typename P1, typename P2>
    bool invoke_get(const_str method_name, P1 p1, P2 p2, R &result) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return false;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);
        lua_value<P2>::push(__lua, p2);

        // Call script
        if (lua_pcall(__lua, 3, 1, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
            return false;
        }

        bool ok = lua_value<R>::pop(__lua, result);
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
        return ok;
    }

    template <typename R, typename P1, typename P2, typename P3>
    bool invoke_get(const_str method_name, P1 p1, P2 p2, P3 p3, R &result) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return false;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);
        lua_value<P2>::push(__lua, p2);
        lua_value<P3>::push(__lua, p3);

        // Call script
        if (lua_pcall(__lua, 4, 1, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
            return false;
        }

        bool ok = lua_value<R>::pop(__lua, result);
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
        return ok;
    }

    template <typename R, typename P1, typename P2, typename P3, typename P4>
    bool invoke_get(const_str method_name, P1 p1, P2 p2, P3 p3, P4 p4, R &result) {
#ifdef _DEBUG
        int top1 = lua_gettop(__lua);
#endif
        if (!findAndPushMethod(method_name)) return false;

        // Push this + parameters
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_value<P1>::push(__lua, p1);
        lua_value<P2>::push(__lua, p2);
        lua_value<P3>::push(__lua, p3);
        lua_value<P4>::push(__lua, p4);

        // Call script
        if (lua_pcall(__lua, 5, 1, NULL) != 0) {
            NEKO_WARN("[lua] err: %s", lua_tostring(__lua, -1));
            lua_pop(__lua, 1);
            return false;
        }

        bool ok = lua_value<R>::pop(__lua, result);
#ifdef _DEBUG
        int top2 = lua_gettop(__lua);
        assert(top1 == top2 && "The stack has become corrupt");
#endif
        return ok;
    }

#undef __lua

private:
    bool findAndPushMethod(const_str method_name);
};

class method_pointer_wrapper {
public:
    lua_CFunction func;
};

// 不返回任何内容的方法的包装模板

template <class C>
class method_pointer_wrapper_void_0args : public method_pointer_wrapper {
public:
    void (C::*methodPtr)(void);
};

template <class C, typename P1>
class method_pointer_wrapper_void_1args : public method_pointer_wrapper {
public:
    void (C::*methodPtr)(P1);
};

template <class C, typename P1, typename P2>
class method_pointer_wrapper_void_2args : public method_pointer_wrapper {
public:
    void (C::*methodPtr)(P1, P2);
};

template <class C, typename P1, typename P2, typename P3>
class method_pointer_wrapper_void_3args : public method_pointer_wrapper {
public:
    void (C::*methodPtr)(P1, P2, P3);
};

template <class C, typename P1, typename P2, typename P3, typename P4>
class method_pointer_wrapper_void_4args : public method_pointer_wrapper {
public:
    void (C::*methodPtr)(P1, P2, P3, P4);
};

template <class C, typename R>
class method_pointer_wrapper_R_0args : public method_pointer_wrapper {
public:
    R (C::*methodPtr)(void);
};

template <class C, typename R, typename P1>
class method_pointer_wrapper_R_1args : public method_pointer_wrapper {
public:
    R (C::*methodPtr)(P1);
};

template <class C, typename R, typename P1, typename P2>
class method_pointer_wrapper_R_2args : public method_pointer_wrapper {
public:
    R (C::*methodPtr)(P1, P2);
};

template <class C, typename R, typename P1, typename P2, typename P3>
class method_pointer_wrapper_R_3args : public method_pointer_wrapper {
public:
    R (C::*methodPtr)(P1, P2, P3);
};

template <class C, typename R, typename P1, typename P2, typename P3, typename P4>
class method_pointer_wrapper_R_4args : public method_pointer_wrapper {
public:
    R (C::*methodPtr)(P1, P2, P3, P4);
};

template <class C>
int lua_method_void_0args(lua_State *L) {
    int params = lua_gettop(L);
    if (params <= 0) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        return 0;
    }

    C *self;

    bool ok = lua_value<C *>::pop(L, self);

    if (ok) {
        method_pointer_wrapper_void_0args<C> *wrapper = (method_pointer_wrapper_void_0args<C> *)lua_touserdata(L, lua_upvalueindex(1));
        (self->*wrapper->methodPtr)();
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
    }

    return 0;
}

template <class C, typename P1>
int lua_method_void_1args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 2) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    C *self;
    P1 p1;

    bool ok = true;
    ok &= lua_value<P1>::pop(L, p1);
    ok &= lua_value<C *>::pop(L, self);

    if (ok) {
        method_pointer_wrapper_void_1args<C, P1> *wrapper = (method_pointer_wrapper_void_1args<C, P1> *)lua_touserdata(L, lua_upvalueindex(1));
        (self->*wrapper->methodPtr)(p1);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
    }

    return 0;
}

template <class C, typename P1, typename P2>
int lua_method_void_2args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 3) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    C *self;
    P1 p1;
    P2 p2;

    bool ok = true;
    ok &= lua_value<P2>::pop(L, p2);
    ok &= lua_value<P1>::pop(L, p1);
    ok &= lua_value<C *>::pop(L, self);

    if (ok) {
        method_pointer_wrapper_void_2args<C, P1, P2> *wrapper = (method_pointer_wrapper_void_2args<C, P1, P2> *)lua_touserdata(L, lua_upvalueindex(1));
        (self->*wrapper->methodPtr)(p1, p2);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
    }

    return 0;
}

template <class C, typename P1, typename P2, typename P3>
int lua_method_void_3args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 4) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    C *self;
    P1 p1;
    P2 p2;
    P3 p3;

    bool ok = true;
    ok &= lua_value<P3>::pop(L, p3);
    ok &= lua_value<P2>::pop(L, p2);
    ok &= lua_value<P1>::pop(L, p1);
    ok &= lua_value<C *>::pop(L, self);

    if (ok) {
        method_pointer_wrapper_void_3args<C, P1, P2, P3> *wrapper = (method_pointer_wrapper_void_3args<C, P1, P2, P3> *)lua_touserdata(L, lua_upvalueindex(1));
        (self->*wrapper->methodPtr)(p1, p2, p3);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
    }

    return 0;
}

template <class C, typename P1, typename P2, typename P3, typename P4>
int lua_method_void_3args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 5) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    C *self;
    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;

    bool ok = true;
    ok &= lua_value<P4>::pop(L, p4);
    ok &= lua_value<P3>::pop(L, p3);
    ok &= lua_value<P2>::pop(L, p2);
    ok &= lua_value<P1>::pop(L, p1);
    ok &= lua_value<C *>::pop(L, self);

    if (ok) {
        method_pointer_wrapper_void_4args<C, P1, P2, P3, P4> *wrapper = (method_pointer_wrapper_void_4args<C, P1, P2, P3, P4> *)lua_touserdata(L, lua_upvalueindex(1));
        (self->*wrapper->methodPtr)(p1, p2, p3, p4);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
    }

    lua_pop(L, params);
    return 0;
}

template <class C, typename R>
int lua_method_R_0args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 0) {
        C *self;

        bool ok = lua_value<C *>::pop(L, self);

        if (ok) {
            method_pointer_wrapper_R_0args<C, R> *wrapper = (method_pointer_wrapper_R_0args<C, R> *)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (self->*wrapper->methodPtr)();
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            neko_lua_tool_t::function_call_err(name, params);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <class C, typename R, typename P1>
int lua_method_R_1args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 1) {
        C *self;
        P1 p1;

        bool ok = true;
        ok &= lua_value<P1>::pop(L, p1);
        ok &= lua_value<C *>::pop(L, self);

        if (ok) {
            method_pointer_wrapper_R_1args<C, R, P1> *wrapper = (method_pointer_wrapper_R_1args<C, R, P1> *)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (self->*wrapper->methodPtr)(p1);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            neko_lua_tool_t::function_call_err(name, params);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <class C, typename R, typename P1, typename P2>
int lua_method_R_2args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 2) {
        C *self;
        P1 p1;
        P2 p2;

        bool ok = true;
        ok &= lua_value<P2>::pop(L, p2);
        ok &= lua_value<P1>::pop(L, p1);
        ok &= lua_value<C *>::pop(L, self);

        if (ok) {
            method_pointer_wrapper_R_2args<C, R, P1, P2> *wrapper = (method_pointer_wrapper_R_2args<C, R, P1, P2> *)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (self->*wrapper->methodPtr)(p1, p2);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            neko_lua_tool_t::function_call_err(name, params);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <class C, typename R, typename P1, typename P2, typename P3>
int lua_method_R_3args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 3) {
        C *self;
        P1 p1;
        P2 p2;
        P3 p3;

        bool ok = true;
        ok &= lua_value<P3>::pop(L, p3);
        ok &= lua_value<P2>::pop(L, p2);
        ok &= lua_value<P1>::pop(L, p1);
        ok &= lua_value<C *>::pop(L, self);

        if (ok) {
            method_pointer_wrapper_R_3args<C, R, P1, P2, P3> *wrapper = (method_pointer_wrapper_R_3args<C, R, P1, P2, P3> *)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (self->*wrapper->methodPtr)(p1, p2, p3);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            neko_lua_tool_t::function_call_err(name, params);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <class C, typename R, typename P1, typename P2, typename P3, typename P4>
int lua_method_R_3args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 4) {
        C *self;
        P1 p1;
        P2 p2;
        P3 p3;
        P4 p4;

        bool ok = true;
        ok &= lua_value<P4>::pop(L, p4);
        ok &= lua_value<P3>::pop(L, p3);
        ok &= lua_value<P2>::pop(L, p2);
        ok &= lua_value<P1>::pop(L, p1);
        ok &= lua_value<C *>::pop(L, self);

        if (ok) {
            method_pointer_wrapper_R_4args<C, R, P1, P2, P3, P4> *wrapper = (method_pointer_wrapper_R_4args<C, R, P1, P2, P3, P4> *)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (self->*wrapper->methodPtr)(p1, p2, p3, p4);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            neko_lua_tool_t::function_call_err(name, params);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <class C>
int lua_ScriptObject_call(lua_State *L) {
    // 类本身必须继承自 ScriptObject
    C *obj = new C();
    if (!obj->registerObject()) {
        NEKO_WARN("object could not be registered");
        delete obj;
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, obj->getId());
    return 1;
}

template <class C>
int lua_ScriptObject_init(lua_State *L) {
    int params = lua_gettop(L);
    assert(params == 1 && "Only 'self' is allowed as input to the init method");

    int script_ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
    C *obj = new C();
    if (!obj->registerObject(script_ref_)) {
        NEKO_WARN("object could not be registered");
        delete obj;
    }
    return 0;
}

NEKO_STATIC_INLINE int lua_function_void_0args(lua_State *L) {
    int params = lua_gettop(L);
    if (params != 0) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    typedef void (*F)(void);
    F func = (F)lua_touserdata(L, lua_upvalueindex(1));
    (*func)();
    return 0;
}

template <typename P1>
int lua_function_void_1args(lua_State *L) {
    int params = lua_gettop(L);
    if (params == 0) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        return 0;
    }

    P1 p1;

    bool ok = true;
    ok &= lua_value<P1>::pop(L, p1);

    if (ok) {
        typedef void (*F)(P1);
        F func = (F)lua_touserdata(L, lua_upvalueindex(1));
        (*func)(p1);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
    }

    return 0;
}

template <typename P1, typename P2>
int lua_function_void_2args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 2) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    P1 p1;
    P2 p2;

    bool ok = true;
    ok &= lua_value<P2>::pop(L, p2);
    ok &= lua_value<P1>::pop(L, p1);

    if (ok) {
        typedef void (*F)(P1, P2);
        F func = (F)lua_touserdata(L, lua_upvalueindex(1));
        (*func)(p1, p2);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
    }

    return 0;
}

template <typename P1, typename P2, typename P3>
int lua_function_void_3args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 3) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    P1 p1;
    P2 p2;
    P3 p3;

    bool ok = true;
    ok &= lua_value<P3>::pop(L, p3);
    ok &= lua_value<P2>::pop(L, p2);
    ok &= lua_value<P1>::pop(L, p1);

    if (ok) {
        typedef void (*F)(P1, P2, P3);
        F func = (F)lua_touserdata(L, lua_upvalueindex(1));
        (*func)(p1, p2, p3);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
    }

    return 0;
}

template <typename P1, typename P2, typename P3, typename P4>
int lua_function_void_4args(lua_State *L) {
    int params = lua_gettop(L);
    if (params < 4) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        return 0;
    }

    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;

    bool ok = true;
    ok &= lua_value<P4>::pop(L, p4);
    ok &= lua_value<P3>::pop(L, p3);
    ok &= lua_value<P2>::pop(L, p2);
    ok &= lua_value<P1>::pop(L, p1);

    if (ok) {
        typedef void (*F)(P1, P2, P3, P4);
        F func = (F)lua_touserdata(L, lua_upvalueindex(1));
        (*func)(p1, p2, p3, p4);
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
    }

    return 0;
}

template <typename R>
int lua_function_R_0args(lua_State *L) {
    int params = lua_gettop(L);
    if (params != 0) {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pushnil(L);
        return 1;
    }

    typedef R (*F)();
    F func = (F)lua_touserdata(L, lua_upvalueindex(1));
    R ret = (*func)();
    lua_value<R>::push(L, ret);
    return 1;
}

template <typename R, typename P1>
int lua_function_R_1args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 0) {
        P1 p1;

        bool ok = lua_value<P1>::pop(L, p1);

        if (ok) {
            typedef R (*F)(P1);
            F func = (F)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (*func)(p1);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pushnil(L);
    }

    return 1;
}

template <typename R, typename P1, typename P2>
int lua_function_R_2args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 1) {
        P1 p1;
        P2 p2;

        bool ok = true;
        ok &= lua_value<P2>::pop(L, p2);
        ok &= lua_value<P1>::pop(L, p1);

        if (ok) {
            typedef R (*F)(P1, P2);
            F func = (F)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (*func)(p1, p2);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <typename R, typename P1, typename P2, typename P3>
int lua_function_R_3args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 3) {
        P1 p1;
        P2 p2;
        P3 p3;

        bool ok = true;
        ok &= lua_value<P3>::pop(L, p3);
        ok &= lua_value<P2>::pop(L, p2);
        ok &= lua_value<P1>::pop(L, p1);

        if (ok) {
            typedef R (*F)(P1, P2, P3);
            F func = (F)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (*func)(p1, p2, p3);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

template <typename R, typename P1, typename P2, typename P3, typename P4>
int lua_function_R_4args(lua_State *L) {
    int params = lua_gettop(L);
    if (params > 2) {
        P1 p1;
        P2 p2;
        P3 p3;
        P4 p4;

        bool ok = true;
        ok &= lua_value<P4>::pop(L, p4);
        ok &= lua_value<P3>::pop(L, p3);
        ok &= lua_value<P2>::pop(L, p2);
        ok &= lua_value<P1>::pop(L, p1);

        if (ok) {
            typedef R (*F)(P1, P2, P3, P4);
            F func = (F)lua_touserdata(L, lua_upvalueindex(1));
            R ret = (*func)(p1, p2, p3, p4);
            lua_value<R>::push(L, ret);
        } else {
            const_str name = lua_tostring(L, lua_upvalueindex(2));
            NEKO_WARN("[lua] invalid parameters to function \"%s\"", name);
            lua_pushnil(L);
        }
    } else {
        const_str name = lua_tostring(L, lua_upvalueindex(2));
        neko_lua_tool_t::function_call_err(name, params);
        lua_pop(L, params);
        lua_pushnil(L);
    }

    return 1;
}

class ScriptObject;

class lua_class_define {
public:
    virtual int instantiate(lua_State *L, ScriptObject *ptr) = 0;
    virtual const std::string &getClassName() const = 0;
    virtual const std::string &getMetaTableName() const = 0;
    virtual std::map<std::string, method_pointer_wrapper *> getMethods() const = 0;
};

template <class C>
class lua_class_define_impl : public lua_class_define {
public:
    lua_class_define_impl(const_str className, const lua_class_define *parent) : class_name(className), parent(parent) { meta_table_name = std::string("Prototype.") + class_name; }

    virtual ~lua_class_define_impl() {}

    virtual int instantiate(lua_State *L, ScriptObject *ptr) {
        int script_ref_ = 0;
#ifdef _DEBUG
        int top3 = lua_gettop(L);
#endif
        // 创建一个新实例并设置变量 _instance = <this>;
        lua_newtable(L);
        script_ref_ = luaL_ref(L, LUA_REGISTRYINDEX);

        lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref_);
        // lua_getref(L, script_ref_);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        luaL_getmetatable(L, meta_table_name.c_str());
        lua_setmetatable(L, -2);

        // 为该对象分配值 _instance
        lua_pushstring(L, "_instance");
        lua_pushlightuserdata(L, ptr);
        lua_settable(L, -3);

        lua_pop(L, 1);
#ifdef _DEBUG
        int top4 = lua_gettop(L);
        assert(top3 == top4 && "Could not create lua table. Corrupted stack");
#endif
        return script_ref_;
    }

    virtual const std::string &getClassName() const { return class_name; }

    virtual const std::string &getMetaTableName() const { return meta_table_name; }

    virtual std::map<std::string, method_pointer_wrapper *> getMethods() const { return methods; }

    lua_class_define_impl<C> &addMethod(const_str method_name, void (C::*methodPtr)()) {
        method_pointer_wrapper_void_0args<C> *wrapper = new method_pointer_wrapper_void_0args<C>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_void_0args<C>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    template <typename P1>
    lua_class_define_impl<C> &addMethod(const_str method_name, void (C::*methodPtr)(P1)) {
        method_pointer_wrapper_void_1args<C, P1> *wrapper = new method_pointer_wrapper_void_1args<C, P1>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_void_1args<C, P1>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    template <typename P1, typename P2>
    lua_class_define_impl<C> &addMethod(const_str method_name, void (C::*methodPtr)(P1, P2)) {
        method_pointer_wrapper_void_2args<C, P1, P2> *wrapper = new method_pointer_wrapper_void_2args<C, P1, P2>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_void_2args<C, P1, P2>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    template <typename P1, typename P2, typename P3>
    lua_class_define_impl<C> &addMethod(const_str method_name, void (C::*methodPtr)(P1, P2, P3)) {
        method_pointer_wrapper_void_3args<C, P1, P2, P3> *wrapper = new method_pointer_wrapper_void_3args<C, P1, P2, P3>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_void_3args<C, P1, P2, P3>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    // template <typename P1, typename P2, typename P3, typename P4>
    // lua_class_define_impl<C>& addMethod( const_str  method_name, void (C::*methodPtr)(P1, P2, P3, P4)) {
    //     method_pointer_wrapper_void_4args<C, P1, P2, P3, P4>* wrapper = new method_pointer_wrapper_void_4args<C, P1, P2, P3, P4>();
    //     wrapper->methodPtr = methodPtr;
    //     wrapper->func = &lua_method_void_4args<C, P1, P2, P3, P4>;
    //     addMethod(m_L, method_name, wrapper);
    //     return *this;
    // }

    //

    template <typename R>
    lua_class_define_impl<C> &addMethod(const_str method_name, R (C::*methodPtr)()) {
        method_pointer_wrapper_R_0args<C, R> *wrapper = new method_pointer_wrapper_R_0args<C, R>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_R_0args<C, R>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    template <typename R, typename P1>
    lua_class_define_impl<C> &addMethod(const_str method_name, R (C::*methodPtr)(P1)) {
        method_pointer_wrapper_R_1args<C, R, P1> *wrapper = new method_pointer_wrapper_R_1args<C, R, P1>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_R_1args<C, R, P1>;
        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    template <typename R, typename P1, typename P2>
    lua_class_define_impl<C> &addMethod(const_str method_name, R (C::*methodPtr)(P1, P2)) {
        method_pointer_wrapper_R_2args<C, R, P1, P2> *wrapper = new method_pointer_wrapper_R_2args<C, R, P1, P2>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_R_2args<C, R, P1, P2>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    template <typename R, typename P1, typename P2, typename P3>
    lua_class_define_impl<C> &addMethod(const_str method_name, R (C::*methodPtr)(P1, P2, P3)) {
        method_pointer_wrapper_R_3args<C, R, P1, P2, P3> *wrapper = new method_pointer_wrapper_R_3args<C, R, P1, P2, P3>();
        wrapper->methodPtr = methodPtr;
        wrapper->func = &lua_method_R_3args<C, R, P1, P2, P3>;

        addMethod(m_L, method_name, wrapper);
        return *this;
    }

    // template <typename R, typename P1, typename P2, typename P3, typename P4>
    // lua_class_define_impl<C>& addMethod( const_str  method_name, R (C::*methodPtr)(P1, P2, P3, P4)) {
    //     method_pointer_wrapper_R_4args<C, R, P1, P2, P3, P4>* wrapper = new method_pointer_wrapper_R_4args<C, R, P1, P2, P3, P4>();
    //     wrapper->methodPtr = methodPtr;
    //     wrapper->func = &lua_method_R_4args<C, R, P1, P2, P3, P4>;
    //     addMethod(m_L, method_name, wrapper);
    //     return *this;
    // }

    void registerClass(lua_State *L) {
        m_L = L;

#ifdef _DEBUG
        int top = lua_gettop(L);
#endif
        luaL_Reg requiredFuncs[] = {{"__call", lua_ScriptObject_call<C>}, {"init", lua_ScriptObject_init<C>}, {NULL, NULL}};

        luaL_newmetatable(L, meta_table_name.c_str());
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);

        luaL_getmetatable(L, meta_table_name.c_str());
        luaL_setfuncs(L, requiredFuncs, 0);
        lua_pop(L, 1);

        lua_newtable(L);
        luaL_getmetatable(L, meta_table_name.c_str());
        lua_setmetatable(L, -2);
        luaL_setfuncs(L, requiredFuncs, 0);
        lua_setglobal(L, class_name.c_str());

        // Inheritance?
        if (parent != NULL) {
            std::map<std::string, method_pointer_wrapper *> methods = parent->getMethods();
            std::map<std::string, method_pointer_wrapper *>::iterator it = methods.begin();
            std::map<std::string, method_pointer_wrapper *>::iterator end = methods.end();
            for (; it != end; ++it) {
                addMethod(L, it->first.c_str(), it->second);
            }

            // funcs 映射现在包含指向其余函数的名称和函数指针
            luaL_getmetatable(L, meta_table_name.c_str());
            lua_pop(L, 1);
        }

#ifdef _DEBUG
        int top2 = lua_gettop(L);
        assert(top == top2 && "Could not create lua table. Corrupted stack");
#endif
    }

private:
    void addMethod(lua_State *L, const_str method_name, method_pointer_wrapper *wrapper) {
        // 保存方法指针
        methods.insert(std::make_pair(std::string(method_name), wrapper));

#ifdef _DEBUG
        int top = lua_gettop(L);
#endif
        luaL_getmetatable(L, meta_table_name.c_str());
        lua_pushlightuserdata(L, wrapper);
        lua_pushstring(L, method_name);
        lua_pushcclosure(L, wrapper->func, 2);
        lua_setfield(L, -2, method_name);
        lua_pop(L, 1);

        lua_getglobal(L, class_name.c_str());
        lua_pushlightuserdata(L, wrapper);
        lua_pushstring(L, method_name);
        lua_pushcclosure(L, wrapper->func, 2);
        lua_setfield(L, -2, method_name);
        lua_pop(L, 1);

#ifdef _DEBUG
        int top2 = lua_gettop(L);
        assert(top == top2 && "Could not create lua table. Corrupted stack");
#endif
    }

private:
    std::string class_name;
    const lua_class_define *parent;
    std::string meta_table_name;
    std::map<std::string, method_pointer_wrapper *> methods;
    lua_State *m_L;
};

struct ScriptObjectEntry {
    ScriptObjectEntry() : prev(0), ptr(0), next(0) {}
    ~ScriptObjectEntry() {}

    ScriptObjectEntry *prev;
    ScriptObject **ptr;
    ScriptObjectEntry *next;
};

class ScriptObject : public ScriptInvoker {
public:
    virtual lua_class_define *getClassDef() const;
    static lua_class_define_impl<ScriptObject> *getStaticClassDef();

private:
    static lua_class_define_impl<ScriptObject> lua_class_defs;

public:
    ScriptObject();
    virtual ~ScriptObject();

    bool registerObject();
    bool registerObject(int script_ref_);
    void unregisterObject();

    virtual bool onAdd();
    virtual void onRemove();

public:
    void detach_pointer(ScriptObjectEntry *entry);
    ScriptObjectEntry *attach_pointer(ScriptObject **ptr);
    void release_pointers();

private:
    ScriptObjectEntry *obj_last_entry;
};

template <class T>
class ScriptObjectPtr {
public:
    ScriptObjectPtr() : obj_pointer(NULL), linkedlist_entry(NULL) {}

    ScriptObjectPtr(const ScriptObjectPtr<T> &other) : obj_pointer(const_cast<T *>(static_cast<const T *>(other.obj_pointer))), linkedlist_entry(NULL) {
        if (obj_pointer != NULL) linkedlist_entry = obj_pointer->attach_pointer(&obj_pointer);
    }

    ScriptObjectPtr(T *ptr) : obj_pointer(ptr), linkedlist_entry(NULL) {
        if (obj_pointer != NULL) linkedlist_entry = obj_pointer->attach_pointer(&obj_pointer);
    }

    virtual ~ScriptObjectPtr() {
        if (obj_pointer) obj_pointer->detach_pointer(linkedlist_entry);
    }

    void set(T *ptr) {
        if (obj_pointer != NULL) obj_pointer->detach_pointer(linkedlist_entry);
        obj_pointer = ptr;
        if (obj_pointer != NULL) linkedlist_entry = obj_pointer->attach_pointer(&obj_pointer);
    }

    T &operator*() {
        if (obj_pointer == NULL) return NULL;

        return *static_cast<T *>(obj_pointer);
    }

    const T &operator*() const {
        if (obj_pointer == NULL) return NULL;

        return *static_cast<T *>(obj_pointer);
    }

    T *operator->() {
        if (obj_pointer == NULL) return NULL;

        return static_cast<T *>(obj_pointer);
    }

    const T *operator->() const {
        if (obj_pointer == NULL) return NULL;

        return static_cast<T *>(obj_pointer);
    }

    T *get() const {
        if (obj_pointer == NULL) return NULL;

        return static_cast<T *>(obj_pointer);
    }

    ScriptObjectPtr<T> &operator=(T *ptr) {
        set(ptr);
        return *this;
    }

    ScriptObjectPtr<T> &operator=(ScriptObjectPtr<T> &other) {
        if (&other == this) return *this;

        set(static_cast<T *>(other.obj_pointer));
        return *this;
    }

    template <class U>
    ScriptObjectPtr<T> &operator=(ScriptObjectPtr<U> &other) {
        if (other.exists()) set(dynamic_cast<T *>(other.get()));

        set(NULL);
        return *this;
    }

    bool exists() const { return obj_pointer != NULL; }

    bool operator==(const ScriptObjectPtr<T> &other) const { return obj_pointer == other.obj_pointer; }

    template <class U>
    bool operator==(const ScriptObjectPtr<U> &other) const {
        return obj_pointer == static_cast<const ScriptObject *>(other.obj_pointer);
    }

private:
    ScriptObject *obj_pointer;
    ScriptObjectEntry *linkedlist_entry;
};

//
//
struct lua_bind {
    static void initialize(lua_State *_L);

    static void loadFile(const_str pathToFile);
    static void evaluatef(const_str fmt, ...);

    static lua_State *L;

    static lua_State *getLuaState();

    template <class C>
    static lua_class_define_impl<C> &bind() {
        C::getStaticClassDef()->registerClass(getLuaState());
        return *C::getStaticClassDef();
    }

    static inline void bind(const_str funcName, lua_CFunction function) { lua_register(L, funcName, function); }

    static inline void bind(const_str funcName, void (*funcPtr)(void)) {
        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_void_0args, 2);
        lua_setglobal(L, funcName);
    }

    template <typename P1>
    static void bind(const_str funcName, void (*funcPtr)(P1)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_void_1args<P1>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename P1, typename P2>
    static void bind(const_str funcName, void (*funcPtr)(P1, P2)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_void_2args<P1, P2>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename P1, typename P2, typename P3>
    static void bind(const_str funcName, void (*funcPtr)(P1, P2, P3)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_void_3args<P1, P2, P3>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename P1, typename P2, typename P3, typename P4>
    static void bind(const_str funcName, void (*funcPtr)(P1, P2, P3, P4)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_void_4args<P1, P2, P3, P4>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename R>
    static void bind(const_str funcName, R (*funcPtr)(void)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_R_0args<R>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename R, typename P1>
    static void bind(const_str funcName, R (*funcPtr)(P1)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_R_1args<R, P1>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename R, typename P1, typename P2>
    static void bind(const_str funcName, R (*funcPtr)(P1, P2)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_R_2args<R, P1, P2>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename R, typename P1, typename P2, typename P3>
    static void bind(const_str funcName, R (*funcPtr)(P1, P2, P3)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_R_3args<R, P1, P2, P3>, 2);
        lua_setglobal(L, funcName);
    }

    template <typename R, typename P1, typename P2, typename P3, typename P4>
    static void bind(const_str funcName, R (*funcPtr)(P1, P2, P3, P4)) {
        lua_State *L = getLuaState();

        lua_pushlightuserdata(L, reinterpret_cast<void *>(funcPtr));
        lua_pushstring(L, funcName);
        lua_pushcclosure(L, &lua_function_R_4args<R, P1, P2, P3, P4>, 2);
        lua_setglobal(L, funcName);
    }
};

struct strtoll_tool_t {
    static long do_strtoll(const char *s, const char *, int) { return atol(s); }
};
#define neko_lua_strtoll strtoll_tool_t::do_strtoll
#define neko_lua_strtoull (unsigned long)strtoll_tool_t::do_strtoll

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
    } else if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<T>(lua_topointer(L, index));
    } else {
        static_assert(std::is_same_v<T, void>, "Unsupported type for neko_lua_to");
    }
}

template <typename Iterable>
NEKO_INLINE bool neko_lua_equal(lua_State *state, const Iterable &indices) {
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

enum STACK_MIN_NUM_e { STACK_MIN_NUM = 20 };

NEKO_INLINE lua_State *neko_lua_create() {
    lua_State *m_ls = ::lua_newstate(Allocf, NULL);
    ::luaL_openlibs(m_ls);

    neko::lua_bind::initialize(m_ls);
    __neko_luabind_init(m_ls);

    return m_ls;
}

NEKO_INLINE void neko_lua_fini(lua_State *m_ls) {
    if (m_ls) {
        __neko_luabind_fini(m_ls);
        int top = lua_gettop(m_ls);
        if (top != 0) {
            neko_lua_tool_t::dump_stack(m_ls);
            NEKO_WARN("[lua] luastack isn't 0 which means that we have a memory leak somewhere");
        }
        ::lua_close(m_ls);
        m_ls = NULL;
    }
}

void neko_lua_run_string(lua_State *m_ls, const_str str_);

NEKO_STATIC_INLINE void neko_lua_run_string(lua_State *m_ls, const std::string &str_) { neko_lua_run_string(m_ls, str_.c_str()); }

NEKO_INLINE void neko_lua_dump_stack(lua_State *m_ls) { neko_lua_tool_t::dump_stack(m_ls); }

NEKO_INLINE int neko_lua_add_package_path(lua_State *L, const std::string &str_) {
    std::string new_path = "package.path = package.path .. \"";
    if (str_.empty()) return -1;
    if (str_[0] != ';') new_path += ";";
    new_path += str_;
    if (str_[str_.length() - 1] != '/') new_path += "/";
    new_path += "?.lua\" ";
    neko_lua_run_string(L, new_path);
    return 0;
}

NEKO_INLINE int neko_lua_load_file(lua_State *m_ls, const std::string &file_name_)  //
{
    if (luaL_dofile(m_ls, file_name_.c_str())) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "cannot load file<%s>", file_name_.c_str());
        ::lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    return 0;
}

NEKO_INLINE int neko_lua_pcall_wrap(lua_State *state, int argnum, int retnum, int msgh) {
    int result = lua_pcall(state, argnum, retnum, msgh);
    return result;
}

NEKO_INLINE int neko_lua_safe_dofile(lua_State *state, const std::string &file) {
    neko_lua_run_string(state, std::format("xpcall(function ()\nrequire '{0}'\nend, function(err)\nprint(tostring(err))\nprint(debug.traceback(nil, 2))\n__neko_quit(1)\nend)\n", file));
    return 0;
}

NEKO_INLINE bool neko_lua_dofile(lua_State *m_ls, const std::string &file) {
    int status = luaL_loadfile(m_ls, file.c_str());

    if (status) {
        const char *err = lua_tostring(m_ls, -1);
        NEKO_WARN("luaL_loadfile ret %d\n%s\n", status, err);
        lua_pop(m_ls, 1);
        return false;
    }

    status = neko_lua_pcall_wrap(m_ls, 0, LUA_MULTRET, 0);
    if (status) {
        const char *err = lua_tostring(m_ls, -1);
        NEKO_WARN("lua_pcall_wrap ret %d\n%s\n", status, err);
        lua_pop(m_ls, 1);
        return false;
    }
    return true;
}

template <typename T>
int neko_lua_get_global_variable(lua_State *m_ls, const std::string &field_name_, T &ret_);
template <typename T>
int neko_lua_get_global_variable(lua_State *m_ls, const char *field_name_, T &ret_);

template <typename T>
int neko_lua_set_global_variable(lua_State *m_ls, const std::string &field_name_, const T &value_);
template <typename T>
int neko_lua_set_global_variable(lua_State *m_ls, const char *field_name_, const T &value_);

NEKO_INLINE void neko_lua_register_raw_function(lua_State *m_ls, const char *func_name_, lua_function_t func_) {
    lua_checkstack(m_ls, STACK_MIN_NUM);

    lua_pushcfunction(m_ls, func_);
    lua_setglobal(m_ls, func_name_);
}

template <typename T>
void neko_lua_reg(lua_State *m_ls, T a);

NEKO_INLINE void neko_lua_call(lua_State *m_ls, const char *func_name_) {
    ::lua_getglobal(m_ls, func_name_);

    if (neko_lua_pcall_wrap(m_ls, 0, 0, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        ::lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }
}

NEKO_INLINE int __neko_lua_getFuncByName(lua_State *m_ls, const char *func_name_) {
    // lua_getglobal(m_ls, func_name_);
    // return 0;

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
            lua_getglobal(m_ls, begin);
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

template <typename T>
int neko_lua_get_global_variable(lua_State *m_ls, const std::string &field_name_, T &ret_) {
    return get_global_variable<T>(field_name_.c_str(), ret_);
}

template <typename T>
int neko_lua_get_global_variable(lua_State *m_ls, const char *field_name_, T &ret_) {
    int ret = 0;

    lua_getglobal(m_ls, field_name_);
    ret = __lua_op_t<T>::get_ret_value(m_ls, -1, ret_);

    lua_pop(m_ls, 1);
    return ret;
}

template <typename T>
int neko_lua_set_global_variable(lua_State *m_ls, const std::string &field_name_, const T &value_) {
    return set_global_variable<T>(field_name_.c_str(), value_);
}

template <typename T>
int neko_lua_set_global_variable(lua_State *m_ls, const char *field_name_, const T &value_) {
    __lua_op_t<T>::push_stack(m_ls, value_);
    lua_setglobal(m_ls, field_name_);
    return 0;
}

// template <typename T>
// void neko_lua_reg(lua_State *m_ls, T a) {
//     a(this->get_lua_state());
// }

//! impl for common RET
template <typename RET>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 0, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg0] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 1, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg1] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 2, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg2] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 3, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg3] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 4, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg4] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 5, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg5] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 6, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg6] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    __lua_op_t<ARG7>::push_stack(m_ls, arg7_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 7, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg7] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_,
                       const ARG8 &arg8_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    __lua_op_t<ARG7>::push_stack(m_ls, arg7_);
    __lua_op_t<ARG8>::push_stack(m_ls, arg8_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 8, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg8] get_ret_value failed  func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
ret(RET) neko_lua_call(lua_State *m_ls, const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_,
                       const ARG8 &arg8_, const ARG9 &arg9_) {
    ret(RET) ret = init_value_traits_t<ret(RET)>::value();

    int tmpArg = __neko_lua_getFuncByName(m_ls, func_name_);

    __lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    __lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    __lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    __lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    __lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    __lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    __lua_op_t<ARG7>::push_stack(m_ls, arg7_);
    __lua_op_t<ARG8>::push_stack(m_ls, arg8_);
    __lua_op_t<ARG9>::push_stack(m_ls, arg9_);

    if (neko_lua_pcall_wrap(m_ls, tmpArg + 9, 1, 0) != 0) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall_wrap failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        NEKO_ERROR("%s", err.c_str());
    }

    if (__lua_op_t<ret(RET)>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg9] get_ret_value failed func_name<%s>", func_name_);
        NEKO_ERROR("%s", buff);
    }

    lua_pop(m_ls, 1);

    return ret;
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

#define IMPLEMENT_LUA_CLASS(C, Parent)                                                    \
    neko::lua_class_define *C::getClassDef() const { return &C::lua_class_defs; }         \
    neko::lua_class_define_impl<C> *C::getStaticClassDef() { return &C::lua_class_defs; } \
    neko::lua_class_define_impl<C> C::lua_class_defs(#C, Parent::getStaticClassDef());

#define IMPLEMENT_LUA_CLASS_WITH_NAME(C, CName, Parent)                                   \
    neko::lua_class_define *C::getClassDef() const { return &C::lua_class_defs; }         \
    neko::lua_class_define_impl<C> *C::getStaticClassDef() { return &C::lua_class_defs; } \
    neko::lua_class_define_impl<C> C::lua_class_defs(CName, Parent::getStaticClassDef());

#define DEFINE_LUA_CLASS(C)                                     \
    virtual neko::lua_class_define *getClassDef() const;        \
    static neko::lua_class_define_impl<C> *getStaticClassDef(); \
    static neko::lua_class_define_impl<C> lua_class_defs;

#endif