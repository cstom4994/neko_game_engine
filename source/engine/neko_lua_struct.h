
#ifndef NEKO_LUA_STRUCT_H
#define NEKO_LUA_STRUCT_H

#include "neko_lua.h"

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

// luabind

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

#endif