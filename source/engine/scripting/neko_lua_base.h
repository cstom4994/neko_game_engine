
#ifndef NEKO_LUA_BASE_H
#define NEKO_LUA_BASE_H

#include <cstdlib>
#include <string>

#include "engine/common/neko_types.h"
#include "libs/lua/lua.hpp"

#ifdef NEKO_PLATFORM_WINDOWS
#define strerror_r(errno, buf, len) strerror_s(buf, len, errno)
#endif

struct lua_mem_block {
    void *ptr;
    size_t size;
};

struct lua_allocator {
    struct lua_mem_block *blocks;
    size_t nb_blocks, size_blocks;
    size_t total_allocated;
};

struct lua_allocator *new_allocator(void);
void delete_allocator(struct lua_allocator *alloc);

void *lua_simple_alloc(void *ud, void *ptr, size_t osize, size_t nsize);
void *lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize);

#pragma region LuaA

#define LUAA_REGISTRYPREFIX "lautoc_"

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

#pragma region LuaMemSafe

#define SAFELUA_CANCELED (-2)
#define SAFELUA_FINISHED (-1)

/** Open a new Lua state that can be canceled */
lua_State *neko_lua_safe_open(void);
/** Close the Lua state and free resources */
void neko_lua_safe_close(lua_State *state);

/** Type for the cancel callback, used to check if this thread should stop */
typedef int (*neko_lua_safe_CancelCheck)(lua_State *, void *);

/** Type for the handler callback, used to free some resource */
typedef void (*neko_lua_safe_Handler)(int, void *);

int neko_lua_safe_pcallk(lua_State *state, int nargs, int nresults, int errfunc, int ctx, lua_KFunction k, neko_lua_safe_CancelCheck cancel, void *canceludata);
void neko_lua_safe_checkcancel(lua_State *state);
void neko_lua_safe_cancel(lua_State *state);
int neko_lua_safe_shouldcancel(lua_State *state);
void neko_lua_safe_add_handler(lua_State *state, neko_lua_safe_Handler handler, void *handlerudata);
int neko_lua_safe_remove_handler(lua_State *state, neko_lua_safe_Handler handler, void *handlerudata);

#pragma endregion LuaMemSafe

int luaopen_cstruct_core(lua_State *L);
int luaopen_cstruct_test(lua_State *L);
int luaopen_datalist(lua_State *L);

#include "neko_lua_register.h"

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

class neko_lua_t {
    enum STACK_MIN_NUM_e { STACK_MIN_NUM = 20 };

public:
    neko_lua_t(bool b = false) : m_ls(NULL), m_bEnableModFunc(b) {
        m_ls = ::luaL_newstate();
        ::luaL_openlibs(m_ls);

        __neko_lua_auto_open(m_ls);
    }
    virtual ~neko_lua_t() {
        if (m_ls) {
            __neko_lua_auto_close(m_ls);

            ::lua_close(m_ls);
            m_ls = NULL;
        }
    }
    void dump_stack() const { neko_lua_tool_t::dump_stack(m_ls); }
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
            std::string err = neko_lua_tool_t::dump_error(m_ls, "cannot load file<%s>", file_name_.c_str());
            ::lua_pop(m_ls, 1);
            throw lua_exception_t(err);
        }

        return 0;
    }
    template <typename T>
    void open_lib(T arg_);

    void run_string(const char *str_) {
        if (luaL_dostring(m_ls, str_)) {
            std::string err = neko_lua_tool_t::dump_error(m_ls, "neko_lua_t::run_string ::lua_pcall failed str<%s>", str_);
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
            std::string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
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
void neko_lua_t::open_lib(T arg_) {
    arg_(m_ls);
}

template <typename T>
int neko_lua_t::get_global_variable(const std::string &field_name_, T &ret_) {
    return get_global_variable<T>(field_name_.c_str(), ret_);
}

template <typename T>
int neko_lua_t::get_global_variable(const char *field_name_, T &ret_) {
    int ret = 0;

    lua_getglobal(m_ls, field_name_);
    ret = lua_op_t<T>::get_ret_value(m_ls, -1, ret_);

    lua_pop(m_ls, 1);
    return ret;
}

template <typename T>
int neko_lua_t::set_global_variable(const std::string &field_name_, const T &value_) {
    return set_global_variable<T>(field_name_.c_str(), value_);
}

template <typename T>
int neko_lua_t::set_global_variable(const char *field_name_, const T &value_) {
    lua_op_t<T>::push_stack(m_ls, value_);
    lua_setglobal(m_ls, field_name_);
    return 0;
}

template <typename T>
void neko_lua_t::reg(T a) {
    a(this->get_lua_state());
}

//! impl for common RET
template <typename RET>
RET_V neko_lua_t::call(const char *func_name_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    if (lua_pcall(m_ls, tmpArg + 0, 1, 0) != 0) {
        neko_string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg0] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);

    if (lua_pcall(m_ls, tmpArg + 1, 1, 0) != 0) {
        neko_string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg1] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_)

{
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);

    if (lua_pcall(m_ls, tmpArg + 2, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg2] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);

    if (lua_pcall(m_ls, tmpArg + 3, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg3] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    lua_op_t<ARG4>::push_stack(m_ls, arg4_);

    if (lua_pcall(m_ls, tmpArg + 4, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg4] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    lua_op_t<ARG5>::push_stack(m_ls, arg5_);

    if (lua_pcall(m_ls, tmpArg + 5, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg5] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    lua_op_t<ARG6>::push_stack(m_ls, arg6_);

    if (lua_pcall(m_ls, tmpArg + 6, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg6] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    lua_op_t<ARG7>::push_stack(m_ls, arg7_);

    if (lua_pcall(m_ls, tmpArg + 7, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg7] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_, const ARG8 &arg8_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    lua_op_t<ARG7>::push_stack(m_ls, arg7_);
    lua_op_t<ARG8>::push_stack(m_ls, arg8_);

    if (lua_pcall(m_ls, tmpArg + 8, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg8] get_ret_value failed  func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
RET_V neko_lua_t::call(const char *func_name_, const ARG1 &arg1_, const ARG2 &arg2_, const ARG3 &arg3_, const ARG4 &arg4_, const ARG5 &arg5_, const ARG6 &arg6_, const ARG7 &arg7_, const ARG8 &arg8_,
                       const ARG9 &arg9_) {
    RET_V ret = init_value_traits_t<RET_V>::value();

    int tmpArg = getFuncByName(func_name_);

    lua_op_t<ARG1>::push_stack(m_ls, arg1_);
    lua_op_t<ARG2>::push_stack(m_ls, arg2_);
    lua_op_t<ARG3>::push_stack(m_ls, arg3_);
    lua_op_t<ARG4>::push_stack(m_ls, arg4_);
    lua_op_t<ARG5>::push_stack(m_ls, arg5_);
    lua_op_t<ARG6>::push_stack(m_ls, arg6_);
    lua_op_t<ARG7>::push_stack(m_ls, arg7_);
    lua_op_t<ARG8>::push_stack(m_ls, arg8_);
    lua_op_t<ARG9>::push_stack(m_ls, arg9_);

    if (lua_pcall(m_ls, tmpArg + 9, 1, 0) != 0) {
        string err = neko_lua_tool_t::dump_error(m_ls, "lua_pcall failed func_name<%s>", func_name_);
        lua_pop(m_ls, 1);
        throw lua_exception_t(err);
    }

    if (lua_op_t<RET_V>::get_ret_value(m_ls, -1, ret)) {
        lua_pop(m_ls, 1);
        char buff[512];
        SPRINTF_F(buff, sizeof(buff), "callfunc [arg9] get_ret_value failed func_name<%s>", func_name_);
        throw lua_exception_t(buff);
    }

    lua_pop(m_ls, 1);

    return ret;
}

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

#endif