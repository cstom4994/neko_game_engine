
#include <assert.h>
#include <ffi.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#endif

#include "engine/neko.h"
#include "engine/neko_luabind.hpp"

namespace neko::lua_cffi {

#define INT_TYPE_LIST_(macro)                                                                                                                                        \
    macro(FFI_TYPE_UINT8, uint8_t, uint8) macro(FFI_TYPE_UINT16, uint16_t, uint16) macro(FFI_TYPE_UINT32, uint32_t, uint32) macro(FFI_TYPE_UINT64, uint64_t, uint64) \
            macro(FFI_TYPE_SINT8, int8_t, sint8) macro(FFI_TYPE_SINT16, int16_t, sint16) macro(FFI_TYPE_SINT32, int32_t, sint32) macro(FFI_TYPE_SINT64, int64_t, sint64)

#define FLOAT_TYPE_LIST_(macro) macro(FFI_TYPE_FLOAT, float, float) macro(FFI_TYPE_DOUBLE, double, double)

// ffi callback
struct closure {
    lua_State *L;
    ffi_closure *closure; /* released in __gc */
    void *exec_addr;      /* FFI 将调用此地址 */
    int fn_ref;           /* 注册表中对 Lua 闭包的引用 */
};

static void add_type(luaL_Buffer *B, ffi_type *type) {
    const char *typestr = "!BAD_TYPE";
    ffi_type **t;

    switch (type->type) {
#define CASE(ffi_type, c_type, ...) \
    case ffi_type:                  \
        typestr = #c_type;          \
        break;
        INT_TYPE_LIST_(CASE)
        FLOAT_TYPE_LIST_(CASE)
#undef CASE
        case FFI_TYPE_POINTER:
            typestr = "void *";
            break;
        case FFI_TYPE_VOID:
            typestr = "void";
            break;
        case FFI_TYPE_STRUCT:
            luaL_addstring(B, "struct { ");
            for (t = type->elements; *t != NULL; t++) {
                add_type(B, *t);
                luaL_addstring(B, "; ");
            }
            luaL_addchar(B, '}');
            return; /* do not push typestr */
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
#define CASE(ffi_type, c_type, ...)    \
    case sizeof(c_type _Complex):      \
        typestr = #c_type " _Complex"; \
        break;
        case FFI_TYPE_COMPLEX:
            switch (type->size) { FLOAT_TYPE_LIST_(CASE) }
#undef CASE
#endif
    }
    luaL_addstring(B, typestr);
}

static int type_tostr(lua_State *L) {
    ffi_type *type = (ffi_type *)luaL_checkudata(L, 1, "ffi_type");
    luaL_Buffer B;

    luaL_buffinit(L, &B);
    lua_pushfstring(L, "ffi_type: %p <", type);
    luaL_addstring(&B, lua_tostring(L, -1));
    add_type(&B, type);
    luaL_addchar(&B, '>');
    luaL_pushresult(&B);
    return 1;
}

static int alignof_(lua_State *L) {
    ffi_type *type;

    type = (ffi_type *)luaL_checkudata(L, 1, "ffi_type");
    lua_pushinteger(L, type->alignment);
    return 1;
}

static int makestruct(lua_State *L) {
    lua_Unsigned len = 0, i = 0;
    ffi_type *type;
    size_t *offsets;
    ffi_status status;
    int accept_name = 0;
    int ltype;

    luaL_checktype(L, 1, LUA_TTABLE);
    while ((ltype = lua_rawgeti(L, 1, ++i)) != LUA_TNIL) {
        if (accept_name && ltype == LUA_TSTRING) {
            assert(len > 0);
            lua_pushnumber(L, len);
            lua_rawset(L, 1);
            accept_name = 0;
            continue;
        }
        luaL_argcheck(L, luaL_testudata(L, -1, "ffi_type"), 1, "expect a sequence of ffi_type");
        lua_rawseti(L, 1, ++len);
        accept_name = 1;
    }
    /* erase table[len+1 .. i-1] */
    while (--i > len) {
        lua_pushnil(L);
        lua_rawseti(L, 1, i);
    }
    assert(len == lua_rawlen(L, 1));
    type = (ffi_type *)lua_newuserdata(L, sizeof *type + sizeof(ffi_type *) * (len + 1) + sizeof offsets[0] * len);
    luaL_setmetatable(L, "ffi_type");
    lua_pushvalue(L, 1);
    lua_setuservalue(L, -2);
    type->size = type->alignment = 0;
    type->type = FFI_TYPE_STRUCT;
    type->elements = (ffi_type **)(type + 1);

    for (i = 0; i < len; i++) {
        lua_rawgeti(L, 1, i + 1);
        type->elements[i] = (ffi_type *)luaL_checkudata(L, -1, "ffi_type");
        lua_pop(L, 1);
    }
    type->elements[len] = NULL;
    offsets = (size_t *)&type->elements[len + 1];
    status = ffi_get_struct_offsets(FFI_DEFAULT_ABI, type, offsets);
    if (status != FFI_OK) return luaL_error(L, "failed to get struct offsets");

    return 1;
}

static int getfield(lua_State *L) {
    size_t offset = 0;
    int top = lua_gettop(L);
    int i;

    if (top < 2) {
        return luaL_error(L, "expect 2 arguments, got %d", top);
    }
    for (i = 2; i <= top; i++) {
        lua_Integer idx, len;
        size_t *offsets;
        ffi_type *type = (ffi_type *)luaL_checkudata(L, 1, "ffi_type");

        luaL_argcheck(L, type->type == FFI_TYPE_STRUCT, 1, "type is not a struct");
        lua_getuservalue(L, 1);
        luaL_argcheck(L, lua_istable(L, -1), 1, "ffi_type is corrupted");
        len = lua_rawlen(L, -1);
        offsets = (size_t *)&type->elements[len + 1];
        if (lua_type(L, i) == LUA_TSTRING) {
            lua_pushvalue(L, i);
            if (lua_rawget(L, -2) != LUA_TNUMBER) {
                lua_pushfstring(L, "field '%s' undefined", lua_tostring(L, i));
                return luaL_argerror(L, i, lua_tostring(L, -1));
            }
            lua_replace(L, i);
        }
        idx = luaL_checkinteger(L, i);
        luaL_argcheck(L, 1 <= idx && idx <= len, i, "index out of bound");
        /* field table is at stack top */
        lua_rawgeti(L, -1, idx); /* type */
        lua_copy(L, -1, 1);      /* replace argument #1 */
        lua_pop(L, 2);           /* pop table and type */
        offset += offsets[idx - 1];
    }
    lua_settop(L, 1);
    lua_pushinteger(L, offset);
    return 2;
}

static int getnfields(lua_State *L) {
    lua_Integer len;
    ffi_type *type;

    type = (ffi_type *)luaL_checkudata(L, 1, "ffi_type");
    luaL_argcheck(L, type->type == FFI_TYPE_STRUCT, 1, "type is not a struct");
    lua_getuservalue(L, 1);
    assert(lua_istable(L, -1));
    len = lua_rawlen(L, -1);
    lua_pushinteger(L, len);
    return 1;
}

static void initobj_(lua_State *L, int type_idx) {
    luaL_setmetatable(L, "ffi_obj");
    assert(luaL_testudata(L, type_idx, "ffi_type"));
    lua_pushvalue(L, type_idx);
    lua_setuservalue(L, -2);
}

static int alloc(lua_State *L) {
    ffi_type *type = (ffi_type *)luaL_checkudata(L, 1, "ffi_type");
    lua_Integer len = luaL_optinteger(L, 2, 1);

    luaL_argcheck(L, len > 0, 2, "length must be greater than 0");
    lua_newuserdata(L, type->size * len);
    initobj_(L, 1);
    return 1;
}

static int sizeof_(lua_State *L) {
    ffi_type *type;
    size_t size;

    if (luaL_testudata(L, 1, "ffi_obj") != NULL) {
        size = lua_rawlen(L, 1);
    } else {
        type = (ffi_type *)luaL_checkudata(L, 1, "ffi_type");
        size = type->size;
    }
    lua_pushinteger(L, size);
    return 1;
}

static int typeof_(lua_State *L) {
    luaL_checkudata(L, 1, "ffi_obj");
    lua_getuservalue(L, 1);
    luaL_argcheck(L, luaL_testudata(L, -1, "ffi_type"), 1, "ffi_obj is corrupted");
    return 1;
}

static int ref_offset(lua_State *L) {
    void *obj;
    size_t offset;
    int ltype = lua_type(L, 1);

    luaL_argcheck(L, ltype == LUA_TUSERDATA || ltype == LUA_TLIGHTUSERDATA, 1, "expect userdata");
    obj = lua_touserdata(L, 1);
    offset = luaL_optinteger(L, 2, 0) * luaL_optinteger(L, 3, 1);
    if (ltype == LUA_TUSERDATA) {
        luaL_argcheck(L, 0 <= offset && offset < lua_rawlen(L, 1), 2, "offset out of bound");
    }
    lua_pushlightuserdata(L, (char *)obj + offset);

    return 1;
}

static void push_type_offset_(lua_State *L, int o, int k) {
    ffi_type *type;

    lua_getuservalue(L, o);
    type = (ffi_type *)luaL_checkudata(L, -1, "ffi_type");

    if (lua_isinteger(L, k)) {
        lua_Integer idx;

        idx = lua_tointeger(L, k);
        lua_pushinteger(L, (idx - 1) * type->size);
    } else {
        lua_pushcfunction(L, getfield);
        lua_insert(L, -2);
        lua_pushvalue(L, k);
        /* getfield type key */
        lua_call(L, 2, 2);
    }
}

static void cast2c(lua_State *L, int idx, void *addr, ffi_type *type);
static void cast2lua(lua_State *L, void *addr, ffi_type *type);

static int objindex(lua_State *L) {
    ffi_type *type;
    void *obj;
    size_t offset;

    obj = luaL_checkudata(L, 1, "ffi_obj");
    push_type_offset_(L, 1, 2);
    type = (ffi_type *)luaL_checkudata(L, -2, "ffi_type");
    offset = luaL_checkinteger(L, -1);
    luaL_argcheck(L, offset + type->size <= lua_rawlen(L, 1), 2, "access out of bound");
    cast2lua(L, (char *)obj + offset, type);
    return 1;
}

static int obj_newindex(lua_State *L) {
    ffi_type *type;
    void *obj;
    size_t offset;

    obj = luaL_checkudata(L, 1, "ffi_obj");
    push_type_offset_(L, 1, 2);
    type = (ffi_type *)luaL_checkudata(L, -2, "ffi_type");
    offset = luaL_checkinteger(L, -1);
    luaL_argcheck(L, offset + type->size <= lua_rawlen(L, 1), 2, "access out of bound");
    cast2c(L, 3, (char *)obj + offset, type);
    return 1;
}

static int obj_len(lua_State *L) {
    ffi_type *type;

    luaL_checkudata(L, 1, "ffi_obj");
    lua_getuservalue(L, 1);
    type = (ffi_type *)luaL_checkudata(L, -1, "ffi_type");
    lua_pushinteger(L, lua_rawlen(L, 1) / type->size);
    return 1;
}

static int obj_tostr(lua_State *L) {
    ffi_type *type;
    void *obj;
    size_t len;
    luaL_Buffer B;

    obj = luaL_checkudata(L, 1, "ffi_obj");
    len = lua_rawlen(L, 1);

    luaL_buffinit(L, &B);
    lua_pushfstring(L, "ffi_obj: %p <", obj);
    luaL_addvalue(&B);
    lua_getuservalue(L, 1);
    type = (ffi_type *)luaL_checkudata(L, -1, "ffi_type");
    add_type(&B, type);
    len /= type->size;
    if (len > 1) {
        luaL_addchar(&B, '[');
        lua_pushinteger(L, len);
        luaL_addvalue(&B);
        luaL_addchar(&B, ']');
    }
    luaL_addchar(&B, '>');
    luaL_pushresult(&B);
    return 1;
}

/* (ffi_obj | pointer) type [offset=0] -> value */
static int deref(lua_State *L) {
    void *obj = luaL_testudata(L, 1, "ffi_obj");
    ffi_type *type = (ffi_type *)luaL_checkudata(L, 2, "ffi_type");
    size_t offset = luaL_optinteger(L, 3, 0);
    void *p;

    if (obj == NULL) {
        luaL_argcheck(L, lua_islightuserdata(L, 1), 1, "expecting ffi_obj or light userdata");
        obj = lua_touserdata(L, 1);
    } else {
        luaL_argcheck(L, offset + type->size <= lua_rawlen(L, 1), 2, "offset out of bound");
    }
    switch (type->type) {
        case FFI_TYPE_STRUCT:
        case FFI_TYPE_COMPLEX:
            p = lua_newuserdata(L, type->size);
            initobj_(L, 2);
            memcpy(p, (char *)obj + offset, type->size);
            return 1;
    }
    cast2lua(L, (char *)obj + offset, type);
    return 1;
}

// 对于具有 N 个参数的 cif 分配的内存为:
// ffi_cif cif;
// ffi_type *atypes[N];
// 和 cif.arg_types 指向 atypes
// userdata 是一个表 其数组部分是参数类型序列 字段“ret”是返回类型
// {ret = rtype; atypes...} -> cif
static int makecif(lua_State *L) {
    int abi;
    ffi_cif *cif;
    ffi_type *rtype = &ffi_type_void, **atypes;
    unsigned int len, i;

    luaL_checktype(L, 1, LUA_TTABLE);
    len = lua_rawlen(L, 1);
    cif = (ffi_cif *)lua_newuserdata(L, sizeof *cif + len * sizeof(ffi_type *));
    luaL_setmetatable(L, "ffi_cif");
    lua_pushvalue(L, 1);
    lua_setuservalue(L, -2);
    atypes = (ffi_type **)(cif + 1);
    if (lua_getfield(L, 1, "ret") != LUA_TNIL) {
        rtype = (ffi_type *)luaL_checkudata(L, -1, "ffi_type");
    }
    lua_getfield(L, 1, "ABI");
    abi = luaL_optinteger(L, -1, FFI_DEFAULT_ABI);
    lua_pop(L, 2); /* rtype and ABI */
    for (i = 0; i < len; i++) {
        lua_rawgeti(L, 1, i + 1);
        atypes[i] = (ffi_type *)luaL_checkudata(L, -1, "ffi_type");
        lua_pop(L, 1);
    }
    ffi_prep_cif(cif, static_cast<ffi_abi>(abi), len, rtype, atypes);
    return 1;
}

static void add_cif(luaL_Buffer *B, ffi_cif *cif) {
    unsigned i;

    add_type(B, cif->rtype);
    if (cif->nargs == 0) {
        luaL_addstring(B, "()");
    } else {
        luaL_addchar(B, '(');
        i = 0;
        for (;;) {
            add_type(B, cif->arg_types[i]);
            if (++i == cif->nargs) {
                break;
            }
            luaL_addstring(B, ", ");
        }
        luaL_addchar(B, ')');
    }
}

static int cif_tostr(lua_State *L) {
    ffi_cif *cif = (ffi_cif *)luaL_checkudata(L, 1, "ffi_cif");
    luaL_Buffer B;

    luaL_buffinit(L, &B);
    lua_pushfstring(L, "ffi_cif: %p <", cif);
    luaL_addstring(&B, lua_tostring(L, -1));
    add_cif(&B, cif);
    luaL_addchar(&B, '>');
    luaL_pushresult(&B);
    return 1;
}

static int funccall(lua_State *L);

// 将 Lua 值转换为 C 指针
static int cast2ptr(lua_State *L, int idx, void **ptr) {
    int ltype = lua_type(L, idx);
    lua_CFunction fn;
    void *p;

    switch (ltype) {
        case LUA_TNIL:
            *ptr = NULL;
            break;
        case LUA_TSTRING:
            /* FFI discards the `const' qualifier;
             * the C function must not change the string. */
            *ptr = (void *)lua_tostring(L, idx);
            break;
        case LUA_TFUNCTION:
            fn = lua_tocfunction(L, idx);
            if (fn == funccall) { /* FFI func */
                lua_getupvalue(L, idx, 2);
                fn = lua_tocfunction(L, -1);
                lua_pop(L, 1);
            } else if (lua_getupvalue(L, idx, 1)) {
                /* C closure is not supported */
                return 0;
            }
            /* Lua function is not supported; user should create
             * closure explicitly.
             */
            if (fn == NULL) return 0;
            *ptr = (void *)fn;
            break;
        case LUA_TLIGHTUSERDATA:
            *ptr = lua_touserdata(L, idx);
            break;
        case LUA_TUSERDATA:
            p = lua_touserdata(L, idx);
            if (luaL_testudata(L, idx, "ffi_closure")) {
                *ptr = ((struct closure *)p)->exec_addr;
            } else {
                *ptr = p;
            }
            break;
        default:
            return 0;
    }
    return 1;
}

// 将数值转换为 C
#define CAST_CASE(ffi_type, c_type, ...) \
    case ffi_type:                       \
        *(c_type *)addr = n;             \
        return 1;
#define COMPLEX_CASE(ffi_type, c_type, ...) \
    case sizeof(c_type _Complex):           \
        *(c_type _Complex *)addr = n;       \
        return 1;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
#define COMPLEX_CASES      \
    case FFI_TYPE_COMPLEX: \
        switch (type->size) { FLOAT_TYPE_LIST_(COMPLEX_CASE) }
#else
#define COMPLEX_CASES
#endif

#define CAST_TMPL(NAME, TYPE)                             \
    static int NAME(TYPE n, void *addr, ffi_type *type) { \
        switch (type->type) {                             \
            INT_TYPE_LIST_(CAST_CASE)                     \
            FLOAT_TYPE_LIST_(CAST_CASE)                   \
            COMPLEX_CASES                                 \
        }                                                 \
        return 0;                                         \
    }

CAST_TMPL(castint2c, lua_Integer)
CAST_TMPL(castnum2c, lua_Number)

#undef CAST_TMPL
#undef COMPLEX_CASES
#undef COMPLEX_CASE
#undef CAST_CASE

// 将 C 值复制到 idx 处的用户数据中
static int cast2obj(lua_State *L, int idx, void *addr, ffi_type *type) {
    void *obj = luaL_testudata(L, idx, "ffi_obj");

    if (obj == NULL) return 0;
    lua_getuservalue(L, idx);
    if (luaL_testudata(L, -1, "ffi_type") == type) {
        memcpy(addr, obj, type->size);
        return 1;
    }
    return 0;
}

// 将 Lua 值转换为 C
static void cast2c(lua_State *L, int idx, void *addr, ffi_type *type) {
    int ltype = lua_type(L, idx);
    int rc = 0;

    if (ltype == LUA_TBOOLEAN) {
        rc = castint2c(lua_toboolean(L, idx), addr, type);
    } else if (ltype == LUA_TNUMBER) {
        rc = (lua_isinteger(L, idx)) ? castint2c(lua_tointeger(L, idx), addr, type) : castnum2c(lua_tonumber(L, idx), addr, type);
    } else if (type->type == FFI_TYPE_POINTER) {
        rc = cast2ptr(L, idx, (void **)addr);
    } else if (type->type == FFI_TYPE_STRUCT || type->type == FFI_TYPE_COMPLEX) {
        rc = cast2obj(L, idx, addr, type);
    }
    if (rc == 0) {
        luaL_Buffer B;

        luaL_buffinit(L, &B);
        luaL_addstring(&B, "expect ");
        add_type(&B, type);
        luaL_addstring(&B, ", got ");
        luaL_addstring(&B, lua_typename(L, ltype));
        luaL_pushresult(&B);
        luaL_argerror(L, idx, lua_tostring(L, -1));
    }
}

// 从 C 转换数值
#define CASE(ffi_type, c_type, ...) \
    case ffi_type:                  \
        *n = *(c_type *)addr;       \
        return 1;
#define CAST_TMPL(TYPE, NAME, TYPE_LIST)                   \
    static int NAME(TYPE *n, void *addr, ffi_type *type) { \
        switch (type->type) { TYPE_LIST(CASE) }            \
        return 0;                                          \
    }

CAST_TMPL(lua_Integer, cast2lua_int, INT_TYPE_LIST_)
CAST_TMPL(lua_Number, cast2lua_num, FLOAT_TYPE_LIST_)

#undef CAST_TMPL
#undef CASE

// 将 C 值转换为 Lua
static void cast2lua(lua_State *L, void *addr, ffi_type *type) {
    lua_Integer i;
    lua_Number n;
    void *p;

    switch (type->type) {
#define CASE(ffi_type, c_type, ...) case ffi_type:
        INT_TYPE_LIST_(CASE)
        if (cast2lua_int(&i, addr, type)) {
            lua_pushinteger(L, i);
            return;
        }
        break;
        FLOAT_TYPE_LIST_(CASE)
        if (cast2lua_num(&n, addr, type)) {
            lua_pushnumber(L, n);
            return;
        }
        break;
#undef CASE
        case FFI_TYPE_POINTER:
            p = *(void **)addr;
            (p == NULL) ? lua_pushnil(L) : lua_pushlightuserdata(L, p);
            return;
    }
    luaL_error(L, "cannot cast result to lua value");
}

static ffi_type *default_type_(lua_State *L, int idx) {
    int ltype = lua_type(L, idx);

    switch (ltype) {
        case LUA_TNIL:    /* as NULL pointer */
        case LUA_TSTRING: /* as const char * */
        case LUA_TLIGHTUSERDATA:
        case LUA_TUSERDATA:
            return &ffi_type_pointer;
        case LUA_TBOOLEAN: /* as int */
            return &ffi_type_sint;
        case LUA_TNUMBER:
            return (lua_isinteger(L, idx)) ? &ffi_type_sint : &ffi_type_double;
        default:
            return &ffi_type_void;
    }
}

static int funccall(lua_State *L) {
    lua_CFunction fn;
    ffi_cif *cif;
    ffi_type *rtype = NULL, **atypes = NULL;
    void **args;
    void *rvalue = NULL;
    ffi_status status = FFI_OK;
    unsigned ntotalargs = lua_gettop(L);
    unsigned i;
    int rtype_idx = 0;

    fn = lua_tocfunction(L, lua_upvalueindex(2));
    if (fn == NULL) {
        return luaL_error(L, "expect function, got %s", lua_typename(L, lua_type(L, -1)));
    }
    cif = (ffi_cif *)luaL_checkudata(L, lua_upvalueindex(1), "ffi_cif");
    lua_getuservalue(L, lua_upvalueindex(1));
    luaL_checktype(L, -1, LUA_TTABLE);
    lua_getfield(L, -1, "ret");
    lua_replace(L, -2);
    rtype = cif->rtype;
    rtype_idx = lua_gettop(L);
    args = (void **)alloca(sizeof args[0] * ntotalargs);
    if (ntotalargs > cif->nargs) {
        atypes = (ffi_type **)alloca(sizeof atypes[0] * ntotalargs);
    } else if (ntotalargs < cif->nargs) {
        luaL_error(L, "expect %d arguments, got %d", cif->nargs, ntotalargs);
    }
    for (i = 0; i < ntotalargs; i++) {
        ffi_type *type = (cif && i < cif->nargs) ? cif->arg_types[i] : default_type_(L, i + 1);
        if (atypes != NULL) {
            atypes[i] = type;
        }
        args[i] = alloca(type->size);
        cast2c(L, i + 1, args[i], type);
    }
    if (ntotalargs > cif->nargs) {
        int nfixedargs = cif->nargs;
        cif = (ffi_cif *)alloca(sizeof *cif);
        status = ffi_prep_cif_var(cif, FFI_DEFAULT_ABI, nfixedargs, ntotalargs, rtype, atypes);
    }
    if (status != FFI_OK) {
        return luaL_error(L, "failed to prepare cif");
    }
    assert(cif->rtype == rtype);
    switch (rtype->type) {
        case FFI_TYPE_VOID:
            ffi_call(cif, FFI_FN(*fn), NULL, args);
            return 0;
        case FFI_TYPE_STRUCT:
        case FFI_TYPE_COMPLEX:
            assert(rtype_idx != 0);
            rvalue = lua_newuserdata(L, (rtype->size > sizeof(ffi_arg) ? rtype->size : sizeof(ffi_arg)));
            initobj_(L, rtype_idx);
            ffi_call(cif, FFI_FN(*fn), rvalue, args);
            return 1;
    }
    if (rtype->size > sizeof(ffi_arg)) {
        return luaL_error(L, "return value not supported");
    }
    rvalue = (ffi_arg *)alloca(sizeof(ffi_arg));
    ffi_call(cif, FFI_FN(*fn), rvalue, args);
    cast2lua(L, rvalue, rtype);
    return 1;
}

static int loadlib(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_settop(L, 2);

    luaL_requiref(L, "package", luaopen_package, 0);
    lua_getfield(L, -1, "loadlib");
    if (!lua_isfunction(L, -1)) {
        return luaL_error(L, "cannot find package.loadlib");
    }
    lua_replace(L, -2); /* 3: package.loadlib */
    lua_pushnil(L);
    while (lua_next(L, 2) != 0) {
        /* 4: key;  5: value */
        if (lua_type(L, 4) != LUA_TSTRING || !luaL_testudata(L, 5, "ffi_cif")) {
            lua_pop(L, 1);
            continue;
        }
        lua_pushvalue(L, 3);
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 4);
        lua_call(L, 2, LUA_MULTRET); /* package.loadlib(libname, key) */
        /* 4:name 5:cif 6:(cfunc|true|nil 7:err1 err2) */
        if (lua_isnil(L, 6)) {
            lua_settop(L, 7);
            return lua_error(L);
        }
        if (!lua_iscfunction(L, 6)) {
            return luaL_error(L, "cannot load '%s'", lua_tostring(L, 4));
        }
        lua_pushcclosure(L, funccall, 2);
        /* 4:name 5:func */
        lua_pushvalue(L, 4);
        lua_insert(L, 5);
        lua_rawset(L, 2);
        /* 4:name */
    }
    lua_settop(L, 2);
    return 1;
}

// 提供对外部函数的回调
// 该函数从user_data读取闭包 转换参数并调用相应的函数
static void closureproxy(ffi_cif *cif, void *ret, void **args, void *user_data) {
    struct closure *cl = (struct closure *)user_data;
    lua_State *L = cl->L;
    unsigned nargs = cif->nargs;
    unsigned i;
    ffi_type *rtype = cif->rtype;

    luaL_checkstack(L, nargs + 3, NULL);
    lua_rawgeti(L, LUA_REGISTRYINDEX, cl->fn_ref);
    /* push arguments */
    for (i = 0; i < nargs; i++) {
        cast2lua(L, args[i], cif->arg_types[i]);
    }
    if (rtype->type != FFI_TYPE_VOID) {
        lua_call(L, nargs, 1);
        cast2c(L, -1, ret, rtype);
        lua_pop(L, 1); /* pop return value from lua stack */
    } else {
        lua_call(L, nargs, 0);
    }
}

static int makeclosure(lua_State *L) {
    struct closure *cl;
    ffi_status status;
    ffi_cif *cif;

    cif = (ffi_cif *)luaL_checkudata(L, 1, "ffi_cif");
    cl = (struct closure *)lua_newuserdata(L, sizeof *cl);
    luaL_setmetatable(L, "ffi_closure");
    lua_pushvalue(L, 1);
    lua_setuservalue(L, -2);
    cl->L = L;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    cl->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    cl->closure = (ffi_closure *)ffi_closure_alloc(sizeof *cl->closure, &cl->exec_addr);
    if (cl->closure == NULL) {
        return luaL_error(L, "cannot allocate closure");
    }
    status = ffi_prep_closure_loc(cl->closure, cif, closureproxy, cl, cl->exec_addr);
    if (status != FFI_OK) {
        return luaL_error(L, "failed to prepare closure");
    }
    return 1;
}

static int closuregc(lua_State *L) {
    struct closure *cl = (struct closure *)luaL_checkudata(L, 1, "ffi_closure");

    if (cl->closure) {
        ffi_closure_free(cl->closure);
        cl->closure = NULL;
    }
    if (cl->fn_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, cl->fn_ref);
        cl->fn_ref = LUA_NOREF;
    }
    return 0;
}

static int closure_tostr(lua_State *L) {
    struct closure *cl;
    ffi_cif *cif;
    luaL_Buffer B;

    cl = (struct closure *)luaL_checkudata(L, 1, "ffi_closure");

    luaL_buffinit(L, &B);
    lua_pushfstring(L, "ffi_closure %p <", cl);
    luaL_addvalue(&B);
    lua_getuservalue(L, 1);
    cif = static_cast<ffi_cif *>(luaL_checkudata(L, -1, "ffi_cif"));
    add_cif(&B, cif);
    luaL_addchar(&B, '>');
    luaL_pushresult(&B);
    return 1;
}

static void define_type(lua_State *L, int table, const char *name, ffi_type *type) {
    ffi_type *t = (ffi_type *)lua_newuserdata(L, sizeof(ffi_type));
    *t = *type;
    luaL_setmetatable(L, "ffi_type");
    lua_setfield(L, table, name);
}

static void define_int_alias(lua_State *L, int table, const char *name, size_t max) {
    const char *actual_type = NULL;

    switch (max) {
        case INT8_MAX:
            actual_type = "sint8";
            break;
        case UINT8_MAX:
            actual_type = "uint8";
            break;
        case INT16_MAX:
            actual_type = "sint16";
            break;
        case UINT16_MAX:
            actual_type = "uint16";
            break;
        case INT32_MAX:
            actual_type = "sint32";
            break;
        case UINT32_MAX:
            actual_type = "uint32";
            break;
        case INT64_MAX:
            actual_type = "sint64";
            break;
        case UINT64_MAX:
            actual_type = "uint64";
            break;
        default:
            return; /* not supported */
    }
    lua_getfield(L, table, actual_type);
    lua_setfield(L, table, name);
}

static void define_types(lua_State *L, int table) {
    static const struct {
        const char *name;
        ffi_type *type;
    } types[] = {
#define DEFINE(NAME) {#NAME, &ffi_type_##NAME},
#define DEFINE_(_1, _2, NAME, ...) DEFINE(NAME)
            INT_TYPE_LIST_(DEFINE_) FLOAT_TYPE_LIST_(DEFINE_)
#undef DEFINE_
                    DEFINE(void) DEFINE(pointer)
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
                            DEFINE(complex_float) DEFINE(complex_double) DEFINE(complex_longdouble)
#endif
#undef DEFINE
    };
    static const struct {
        const char *name;
        size_t max;
    } aliases[] = {
            {"uchar", UCHAR_MAX}, {"schar", SCHAR_MAX}, {"char", CHAR_MAX},  {"uint", UINT_MAX},   {"sint", INT_MAX},          {"ushort", USHRT_MAX},
            {"sshort", SHRT_MAX}, {"ulong", ULONG_MAX}, {"slong", LONG_MAX}, {"size_t", SIZE_MAX}, {"ptrdiff_t", PTRDIFF_MAX}, {"intptr_t", INTPTR_MAX},
    };
    size_t i;

    for (i = 0; i < sizeof types / sizeof types[0]; i++) {
        define_type(L, table, types[i].name, types[i].type);
    }
    for (i = 0; i < sizeof aliases / sizeof aliases[0]; i++) {
        define_int_alias(L, table, aliases[i].name, aliases[i].max);
    }
}

static int luaopen(lua_State *L) {
    static const luaL_Reg lib_reg[] = {
            {"loadlib", loadlib}, {"makecif", makecif}, {"alloc", alloc}, {"struct", makestruct}, {"sizeof", sizeof_},      {"alignof", alignof_},
            {"typeof", typeof_},  {"field", getfield},  {"deref", deref}, {"ref", ref_offset},    {"closure", makeclosure}, {NULL, NULL},
    };
    static const luaL_Reg cif_reg[] = {
            {"__tostring", cif_tostr},
            {NULL, NULL},
    };
    static const luaL_Reg type_reg[] = {
            {"__index", getfield},
            {"__len", getnfields},
            {"__tostring", type_tostr},
            {NULL, NULL},
    };
    static const luaL_Reg obj_reg[] = {
            {"__index", objindex}, {"__newindex", obj_newindex}, {"__len", obj_len}, {"__tostring", obj_tostr}, {NULL, NULL},
    };
    static const luaL_Reg closure_reg[] = {
            {"__gc", closuregc},
            {"__tostring", closure_tostr},
            {NULL, NULL},
    };

#define INIT(X)                      \
    luaL_newmetatable(L, "ffi_" #X); \
    luaL_setfuncs(L, X##_reg, 0);

    INIT(cif);
    INIT(type);
    INIT(obj);
    INIT(closure);
#undef INIT

    luaL_newlib(L, lib_reg);
    define_types(L, lua_gettop(L));

    return 1;
}

}  // namespace neko::lua_cffi

DEFINE_LUAOPEN(cffi)

void ffi_module_open(lua_State *L);

namespace neko::lua_ffi {
static int luaopen(lua_State *L) {
    ffi_module_open(L);
    return 1;
}
}  // namespace neko::lua_ffi

DEFINE_LUAOPEN(ffi)