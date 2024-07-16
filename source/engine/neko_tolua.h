
// neko_tolua based on tolua by Ariel Manzur (www.tecgraf.puc-rio.br/~celes/tolua)
// it's licensed under the terms of the MIT license

#pragma once

#include "neko_lua.h"

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

const char *neko_tolua_typename(lua_State *L, int lo);
void neko_tolua_error(lua_State *L, const char *msg, neko_tolua_Error *err);
int neko_tolua_isnoobj(lua_State *L, int lo, neko_tolua_Error *err);
int neko_tolua_isvalue(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_isvaluenil(lua_State *L, int lo, neko_tolua_Error *err);
int neko_tolua_isboolean(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_isnumber(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_isinteger(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_isstring(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_istable(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_isusertable(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err);
int neko_tolua_isuserdata(lua_State *L, int lo, int def, neko_tolua_Error *err);
int neko_tolua_isusertype(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err);
int neko_tolua_isvaluearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_isbooleanarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_isnumberarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_isintegerarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_isstringarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_istablearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_isuserdataarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err);
int neko_tolua_isusertypearray(lua_State *L, int lo, const char *type, int dim, int def, neko_tolua_Error *err);

void neko_tolua_open(lua_State *L);

void *neko_tolua_copy(lua_State *L, void *value, unsigned int size);
int neko_tolua_register_gc(lua_State *L, int lo);
int neko_tolua_default_collect(lua_State *L);

void neko_tolua_usertype(lua_State *L, const char *type);
void neko_tolua_beginmodule(lua_State *L, const char *name);
void neko_tolua_endmodule(lua_State *L);
void neko_tolua_module(lua_State *L, const char *name, int hasvar);
void neko_tolua_class(lua_State *L, const char *name, const char *base);
void neko_tolua_cclass(lua_State *L, const char *lname, const char *name, const char *base, lua_CFunction col);
void neko_tolua_function(lua_State *L, const char *name, lua_CFunction func);
void neko_tolua_constant(lua_State *L, const char *name, lua_Number value);
void neko_tolua_variable(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set);
void neko_tolua_array(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set);

/*  void neko_tolua_set_call_event(lua_State* L, lua_CFunction func, char* type); */
/*  void neko_tolua_addbase(lua_State* L, char* name, char* base); */

void neko_tolua_pushvalue(lua_State *L, int lo);
void neko_tolua_pushboolean(lua_State *L, int value);
void neko_tolua_pushnumber(lua_State *L, lua_Number value);
void neko_tolua_pushinteger(lua_State *L, lua_Integer value);
void neko_tolua_pushstring(lua_State *L, const char *value);
void neko_tolua_pushuserdata(lua_State *L, void *value);
void neko_tolua_pushusertype(lua_State *L, void *value, const char *type);
void neko_tolua_pushusertype_and_takeownership(lua_State *L, void *value, const char *type);
void neko_tolua_pushfieldvalue(lua_State *L, int lo, int index, int v);
void neko_tolua_pushfieldboolean(lua_State *L, int lo, int index, int v);
void neko_tolua_pushfieldnumber(lua_State *L, int lo, int index, lua_Number v);
void neko_tolua_pushfieldinteger(lua_State *L, int lo, int index, lua_Integer v);
void neko_tolua_pushfieldstring(lua_State *L, int lo, int index, const char *v);
void neko_tolua_pushfielduserdata(lua_State *L, int lo, int index, void *v);
void neko_tolua_pushfieldusertype(lua_State *L, int lo, int index, void *v, const char *type);
void neko_tolua_pushfieldusertype_and_takeownership(lua_State *L, int lo, int index, void *v, const char *type);

lua_Number neko_tolua_tonumber(lua_State *L, int narg, lua_Number def);
lua_Integer neko_tolua_tointeger(lua_State *L, int narg, lua_Integer def);
const char *neko_tolua_tostring(lua_State *L, int narg, const char *def);
void *neko_tolua_touserdata(lua_State *L, int narg, void *def);
void *neko_tolua_tousertype(lua_State *L, int narg, void *def);
int neko_tolua_tovalue(lua_State *L, int narg, int def);
int neko_tolua_toboolean(lua_State *L, int narg, int def);
lua_Number neko_tolua_tofieldnumber(lua_State *L, int lo, int index, lua_Number def);
lua_Integer neko_tolua_tofieldinteger(lua_State *L, int lo, int index, lua_Integer def);
const char *neko_tolua_tofieldstring(lua_State *L, int lo, int index, const char *def);
void *neko_tolua_tofielduserdata(lua_State *L, int lo, int index, void *def);
void *neko_tolua_tofieldusertype(lua_State *L, int lo, int index, void *def);
int neko_tolua_tofieldvalue(lua_State *L, int lo, int index, int def);
int neko_tolua_getfieldboolean(lua_State *L, int lo, int index, int def);

void neko_tolua_dobuffer(lua_State *L, char *B, unsigned int size, const char *name);

int class_gc_event(lua_State *L);

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

int neko_tolua_fast_isa(lua_State *L, int mt_indexa, int mt_indexb, int super_index);

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
