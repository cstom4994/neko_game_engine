
// neko_tolua based on tolua by Waldemar Celes (www.tecgraf.puc-rio.br/~celes/tolua)
// it's licensed under the terms of the MIT license

#include "engine/neko_tolua.h"

#if defined(NEKO_TOLUA)

/* Store at ubox
 * It stores, creating the corresponding table if needed,
 * the pair key/value in the corresponding ubox table
 */
static void storeatubox(lua_State *L, int lo) {
#if LUA_VERSION_NUM == 501
    lua_getfenv(L, lo);
    if (lua_rawequal(L, -1, TOLUA_NOPEER)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfenv(L, lo); /* stack: k,v,table  */
    };
    lua_insert(L, -3);
    lua_settable(L, -3); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a settable call */
    lua_pop(L, 1);
#elif LUA_VERSION_NUM > 501
    lua_getuservalue(L, lo);
    if (lua_rawequal(L, -1, TOLUA_NOPEER)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setuservalue(L, lo); /* stack: k,v,table  */
    };
    lua_insert(L, -3);
    lua_settable(L, -3); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a settable call */
    lua_pop(L, 1);
#else
    /* stack: key value (to be stored) */
    lua_pushstring(L, "neko_tolua_peers");
    lua_rawget(L, LUA_REGISTRYINDEX); /* stack: k v ubox */
    lua_pushvalue(L, lo);
    lua_rawget(L, -2); /* stack: k v ubox ubox[u] */
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);   /* stack: k v ubox */
        lua_newtable(L); /* stack: k v ubox table */
        lua_pushvalue(L, 1);
        lua_pushvalue(L, -2); /* stack: k v ubox table u table */
        lua_rawset(L, -4);    /* stack: k v ubox ubox[u]=table */
    }
    lua_insert(L, -4); /* put table before k */
    lua_pop(L, 1);     /* pop ubox */
    lua_rawset(L, -3); /* store at table */
    lua_pop(L, 1);     /* pop ubox[u] */
#endif
}

/* Module index function
 */
static int module_index_event(lua_State *L) {
    lua_pushstring(L, ".get");
    lua_rawget(L, -3);
    if (lua_istable(L, -1)) {
        lua_pushvalue(L, 2); /* key */
        lua_rawget(L, -2);
        if (lua_iscfunction(L, -1)) {
            lua_call(L, 0, 1);
            return 1;
        } else if (lua_istable(L, -1))
            return 1;
    }
    /* call old index meta event */
    if (lua_getmetatable(L, 1)) {
        lua_pushstring(L, "__index");
        lua_rawget(L, -2);
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 2);
        if (lua_isfunction(L, -1)) {
            lua_call(L, 2, 1);
            return 1;
        } else if (lua_istable(L, -1)) {
            lua_gettable(L, -3);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

/* Module newindex function
 */
static int module_newindex_event(lua_State *L) {
    lua_pushstring(L, ".set");
    lua_rawget(L, -4);
    if (lua_istable(L, -1)) {
        lua_pushvalue(L, 2); /* key */
        lua_rawget(L, -2);
        if (lua_iscfunction(L, -1)) {
            lua_pushvalue(L, 1); /* only to be compatible with non-static vars */
            lua_pushvalue(L, 3); /* value */
            lua_call(L, 2, 0);
            return 0;
        }
    }
    /* call old newindex meta event */
    if (lua_getmetatable(L, 1) && lua_getmetatable(L, -1)) {
        lua_pushstring(L, "__newindex");
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, 1);
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            lua_call(L, 3, 0);
        }
    }
    lua_settop(L, 3);
    lua_rawset(L, -3);
    return 0;
}

/* Class index function
 * If the object is a userdata (ie, an object), it searches the field in
 * the alternative table stored in the corresponding "ubox" table.
 */
static int class_index_event(lua_State *L) {
    int t = lua_type(L, 1);
    if (t == LUA_TUSERDATA) {
/* Access alternative table */
#if LUA_VERSION_NUM == 501
        lua_getfenv(L, 1);
        if (!lua_rawequal(L, -1, TOLUA_NOPEER)) {
            lua_pushvalue(L, 2); /* key */
            lua_gettable(L, -2); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a gettable call */
            if (!lua_isnil(L, -1)) return 1;
        };
#elif LUA_VERSION_NUM > 501
        lua_getuservalue(L, 1);
        if (!lua_rawequal(L, -1, TOLUA_NOPEER)) {
            lua_pushvalue(L, 2); /* key */
            lua_gettable(L, -2); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a gettable call */
            if (!lua_isnil(L, -1)) return 1;
        };
#else
        lua_pushstring(L, "neko_tolua_peers");
        lua_rawget(L, LUA_REGISTRYINDEX); /* stack: obj key ubox */
        lua_pushvalue(L, 1);
        lua_rawget(L, -2); /* stack: obj key ubox ubox[u] */
        if (lua_istable(L, -1)) {
            lua_pushvalue(L, 2); /* key */
            lua_rawget(L, -2);   /* stack: obj key ubox ubox[u] value */
            if (!lua_isnil(L, -1)) return 1;
        }
#endif
        lua_settop(L, 2); /* stack: obj key */
        /* Try metatables */
        lua_pushvalue(L, 1);              /* stack: obj key obj */
        while (lua_getmetatable(L, -1)) { /* stack: obj key obj mt */
            lua_remove(L, -2);            /* stack: obj key mt */
            if (lua_isnumber(L, 2))       /* check if key is a numeric value */
            {
                /* try operator[] */
                lua_pushstring(L, ".geti");
                lua_rawget(L, -2); /* stack: obj key mt func */
                if (lua_isfunction(L, -1)) {
                    lua_pushvalue(L, 1);
                    lua_pushvalue(L, 2);
                    lua_call(L, 2, 1);
                    return 1;
                }
            } else {
                lua_pushvalue(L, 2); /* stack: obj key mt key */
                lua_rawget(L, -2);   /* stack: obj key mt value */
                if (!lua_isnil(L, -1))
                    return 1;
                else
                    lua_pop(L, 1);
                /* try C/C++ variable */
                lua_pushstring(L, ".get");
                lua_rawget(L, -2); /* stack: obj key mt tget */
                if (lua_istable(L, -1)) {
                    lua_pushvalue(L, 2);
                    lua_rawget(L, -2); /* stack: obj key mt value */
                    if (lua_iscfunction(L, -1)) {
                        lua_pushvalue(L, 1);
                        lua_pushvalue(L, 2);
                        lua_call(L, 2, 1);
                        return 1;
                    } else if (lua_istable(L, -1)) {
                        /* deal with array: create table to be returned and cache it in ubox */
                        void *u = *((void **)lua_touserdata(L, 1));
                        lua_newtable(L); /* stack: obj key mt value table */
                        lua_pushstring(L, ".self");
                        lua_pushlightuserdata(L, u);
                        lua_rawset(L, -3);       /* store usertype in ".self" */
                        lua_insert(L, -2);       /* stack: obj key mt table value */
                        lua_setmetatable(L, -2); /* set stored value as metatable */
                        lua_pushvalue(L, -1);    /* stack: obj key met table table */
                        lua_pushvalue(L, 2);     /* stack: obj key mt table table key */
                        lua_insert(L, -2);       /*  stack: obj key mt table key table */
                        storeatubox(L, 1);       /* stack: obj key mt table */
                        return 1;
                    }
                }
            }
            lua_settop(L, 3);
        }
        lua_pushnil(L);
        return 1;
    } else if (t == LUA_TTABLE) {
        module_index_event(L);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

/* Newindex function
 * It first searches for a C/C++ varaible to be set.
 * Then, it either stores it in the alternative ubox table (in the case it is
 * an object) or in the own table (that represents the class or module).
 */
static int class_newindex_event(lua_State *L) {
    int t = lua_type(L, 1);
    if (t == LUA_TUSERDATA) {
        /* Try accessing a C/C++ variable to be set */
        lua_getmetatable(L, 1);
        while (lua_istable(L, -1)) /* stack: t k v mt */
        {
            if (lua_isnumber(L, 2)) /* check if key is a numeric value */
            {
                /* try operator[] */
                lua_pushstring(L, ".seti");
                lua_rawget(L, -2); /* stack: obj key mt func */
                if (lua_isfunction(L, -1)) {
                    lua_pushvalue(L, 1);
                    lua_pushvalue(L, 2);
                    lua_pushvalue(L, 3);
                    lua_call(L, 3, 0);
                    return 0;
                }
            } else {
                lua_pushstring(L, ".set");
                lua_rawget(L, -2); /* stack: t k v mt tset */
                if (lua_istable(L, -1)) {
                    lua_pushvalue(L, 2);
                    lua_rawget(L, -2); /* stack: t k v mt tset func */
                    if (lua_iscfunction(L, -1)) {
                        lua_pushvalue(L, 1);
                        lua_pushvalue(L, 3);
                        lua_call(L, 2, 0);
                        return 0;
                    }
                    lua_pop(L, 1); /* stack: t k v mt tset */
                }
                lua_pop(L, 1);                /* stack: t k v mt */
                if (!lua_getmetatable(L, -1)) /* stack: t k v mt mt */
                    lua_pushnil(L);
                lua_remove(L, -2); /* stack: t k v mt */
            }
        }
        lua_settop(L, 3); /* stack: t k v */

        /* then, store as a new field */
        storeatubox(L, 1);
    } else if (t == LUA_TTABLE) {
        module_newindex_event(L);
    }
    return 0;
}

static int class_call_event(lua_State *L) {

    if (lua_istable(L, 1)) {
        lua_pushstring(L, ".call");
        lua_rawget(L, 1);
        if (lua_isfunction(L, -1)) {

            lua_insert(L, 1);
            lua_call(L, lua_gettop(L) - 1, 1);

            return 1;
        };
    };
    neko_tolua_error(L, "Attempt to call a non-callable object.", NULL);
    return 0;
};

static int do_operator(lua_State *L, const char *op) {
    if (lua_isuserdata(L, 1)) {
        /* Try metatables */
        lua_pushvalue(L, 1);              /* stack: op1 op2 */
        while (lua_getmetatable(L, -1)) { /* stack: op1 op2 op1 mt */
            lua_remove(L, -2);            /* stack: op1 op2 mt */
            lua_pushstring(L, op);        /* stack: op1 op2 mt key */
            lua_rawget(L, -2);            /* stack: obj key mt func */
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, 1);
                lua_pushvalue(L, 2);
                lua_call(L, 2, 1);
                return 1;
            }
            lua_settop(L, 3);
        }
    }
    neko_tolua_error(L, "Attempt to perform operation on an invalid operand", NULL);
    return 0;
}

static int class_add_event(lua_State *L) { return do_operator(L, ".add"); }

static int class_sub_event(lua_State *L) { return do_operator(L, ".sub"); }

static int class_mul_event(lua_State *L) { return do_operator(L, ".mul"); }

static int class_div_event(lua_State *L) { return do_operator(L, ".div"); }

static int class_lt_event(lua_State *L) { return do_operator(L, ".lt"); }

static int class_le_event(lua_State *L) { return do_operator(L, ".le"); }

static int class_eq_event(lua_State *L) {
    /* copying code from do_operator here to return false when no operator is found */
    if (lua_isuserdata(L, 1)) {
        /* Try metatables */
        lua_pushvalue(L, 1);              /* stack: op1 op2 */
        while (lua_getmetatable(L, -1)) { /* stack: op1 op2 op1 mt */
            lua_remove(L, -2);            /* stack: op1 op2 mt */
            lua_pushstring(L, ".eq");     /* stack: op1 op2 mt key */
            lua_rawget(L, -2);            /* stack: obj key mt func */
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, 1);
                lua_pushvalue(L, 2);
                lua_call(L, 2, 1);
                return 1;
            }
            lua_settop(L, 3);
        }
    }

    lua_settop(L, 3);
    lua_pushboolean(L, 0);
    return 1;
}

/*
static int class_gc_event (lua_State* L)
{
    void* u = *((void**)lua_touserdata(L,1));
    fprintf(stderr, "collecting: looking at %p\n", u);
    lua_pushstring(L,"neko_tolua_gc");
    lua_rawget(L,LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L,u);
    lua_rawget(L,-2);
    if (lua_isfunction(L,-1))
    {
        lua_pushvalue(L,1);
        lua_call(L,1,0);
        lua_pushlightuserdata(L,u);
        lua_pushnil(L);
        lua_rawset(L,-3);
    }
    lua_pop(L,2);
    return 0;
}
*/
int class_gc_event(lua_State *L) {
    if (lua_istable(L, 1)) return 0;
    void *u = *((void **)lua_touserdata(L, 1));
    int top;
    /*fprintf(stderr, "collecting: looking at %p\n", u);*/
    /*
    lua_pushstring(L,"neko_tolua_gc");
    lua_rawget(L,LUA_REGISTRYINDEX);
    */
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushlightuserdata(L, u);
    lua_rawget(L, -2);      /* stack: gc umt    */
    lua_getmetatable(L, 1); /* stack: gc umt mt */
    /*fprintf(stderr, "checking type\n");*/
    top = lua_gettop(L);
    if (neko_tolua_fast_isa(L, top, top - 1, lua_upvalueindex(2))) /* make sure we collect correct type */
    {
        /*fprintf(stderr, "Found type!\n");*/
        /* get gc function */
        lua_pushliteral(L, ".collector");
        lua_rawget(L, -2); /* stack: gc umt mt collector */
        if (lua_isfunction(L, -1)) {
            /*fprintf(stderr, "Found .collector!\n");*/
        } else {
            lua_pop(L, 1);
            /*fprintf(stderr, "Using default cleanup\n");*/
            lua_pushcfunction(L, neko_tolua_default_collect);
        }

        lua_pushvalue(L, 1); /* stack: gc umt mt collector u */
        lua_call(L, 1, 0);

        lua_pushlightuserdata(L, u); /* stack: gc umt mt u */
        lua_pushnil(L);              /* stack: gc umt mt u nil */
        lua_rawset(L, -5);           /* stack: gc umt mt */
    }
    lua_pop(L, 3);
    return 0;
}

/* Register module events
 * It expects the metatable on the top of the stack
 */
void neko_tolua_moduleevents(lua_State *L) {
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, module_index_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, module_newindex_event);
    lua_rawset(L, -3);
}

/* Check if the object on the top has a module metatable
 */
int neko_tolua_ismodulemetatable(lua_State *L) {
    int r = 0;
    if (lua_getmetatable(L, -1)) {
        lua_pushstring(L, "__index");
        lua_rawget(L, -2);
        r = (lua_tocfunction(L, -1) == module_index_event);
        lua_pop(L, 2);
    }
    return r;
}

/* Register class events
 * It expects the metatable on the top of the stack
 */
void neko_tolua_classevents(lua_State *L) {
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, class_index_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, class_newindex_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__add");
    lua_pushcfunction(L, class_add_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__sub");
    lua_pushcfunction(L, class_sub_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__mul");
    lua_pushcfunction(L, class_mul_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__div");
    lua_pushcfunction(L, class_div_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__lt");
    lua_pushcfunction(L, class_lt_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__le");
    lua_pushcfunction(L, class_le_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__eq");
    lua_pushcfunction(L, class_eq_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__call");
    lua_pushcfunction(L, class_call_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__gc");
    lua_pushstring(L, "neko_tolua_gc_event");
    lua_rawget(L, LUA_REGISTRYINDEX);
    /*lua_pushcfunction(L,class_gc_event);*/
    lua_rawset(L, -3);
}

/* a fast check if a is b, without parameter validation
 i.e. if b is equal to a or a superclass of a. */
int neko_tolua_fast_isa(lua_State *L, int mt_indexa, int mt_indexb, int super_index) {
    int result;
    if (lua_rawequal(L, mt_indexa, mt_indexb))
        result = 1;
    else {
        if (super_index) {
            lua_pushvalue(L, super_index);
        } else {
            lua_pushliteral(L, "neko_tolua_super");
            lua_rawget(L, LUA_REGISTRYINDEX); /* stack: super */
        };
        lua_pushvalue(L, mt_indexa);      /* stack: super mta */
        lua_rawget(L, -2);                /* stack: super super[mta] */
        lua_pushvalue(L, mt_indexb);      /* stack: super super[mta] mtb */
        lua_rawget(L, LUA_REGISTRYINDEX); /* stack: super super[mta] typenameB */
        lua_rawget(L, -2);                /* stack: super super[mta] bool */
        result = lua_toboolean(L, -1);
        lua_pop(L, 3);
    }
    return result;
}

/* Push and returns the corresponding object typename */
const char *neko_tolua_typename(lua_State *L, int lo) {
    int tag = lua_type(L, lo);
    if (tag == LUA_TNONE)
        lua_pushstring(L, "[no object]");
    else if (tag != LUA_TUSERDATA && tag != LUA_TTABLE)
        lua_pushstring(L, lua_typename(L, tag));
    else if (tag == LUA_TUSERDATA) {
        if (!lua_getmetatable(L, lo))
            lua_pushstring(L, lua_typename(L, tag));
        else {
            lua_rawget(L, LUA_REGISTRYINDEX);
            if (!lua_isstring(L, -1)) {
                lua_pop(L, 1);
                lua_pushstring(L, "[undefined]");
            }
        }
    } else /* is table */
    {
        lua_pushvalue(L, lo);
        lua_rawget(L, LUA_REGISTRYINDEX);
        if (!lua_isstring(L, -1)) {
            lua_pop(L, 1);
            lua_pushstring(L, "table");
        } else {
            lua_pushstring(L, "class ");
            lua_insert(L, -2);
            lua_concat(L, 2);
        }
    }
    return lua_tostring(L, -1);
}

void neko_tolua_error(lua_State *L, const char *msg, neko_tolua_Error *err) {
    if (msg[0] == '#') {
        const char *expected = err->type;
        const char *provided = neko_tolua_typename(L, err->index);
        if (msg[1] == 'f') {
            int narg = err->index;
            if (err->array)
                luaL_error(L, "%s\n     argument #%d is array of '%s'; array of '%s' expected.\n", msg + 2, narg, provided, expected);
            else
                luaL_error(L, "%s\n     argument #%d is '%s'; '%s' expected.\n", msg + 2, narg, provided, expected);
        } else if (msg[1] == 'v') {
            if (err->array)
                luaL_error(L, "%s\n     value is array of '%s'; array of '%s' expected.\n", msg + 2, provided, expected);
            else
                luaL_error(L, "%s\n     value is '%s'; '%s' expected.\n", msg + 2, provided, expected);
        }
    } else
        luaL_error(L, msg);
}

/* the equivalent of lua_is* for usertable */
static int lua_isusertable(lua_State *L, int lo, const_str type) {
    int r = 0;
    if (lo < 0) lo = lua_gettop(L) + lo + 1;
    lua_pushvalue(L, lo);
    lua_rawget(L, LUA_REGISTRYINDEX); /* get registry[t] */
    if (lua_isstring(L, -1)) {
        r = strcmp(lua_tostring(L, -1), type) == 0;
        if (!r) {
            /* try const */
            lua_pushstring(L, "const ");
            lua_insert(L, -2);
            lua_concat(L, 2);
            r = lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), type) == 0;
        }
    }
    lua_pop(L, 1);
    return r;
}

int push_table_instance(lua_State *L, int lo) {

    if (lua_istable(L, lo)) {

        lua_pushstring(L, ".c_instance");
        lua_gettable(L, lo);
        if (lua_isuserdata(L, -1)) {

            lua_replace(L, lo);
            return 1;
        } else {

            lua_pop(L, 1);
            return 0;
        };
    } else {
        return 0;
    };

    return 0;
};

/* the equivalent of lua_is* for usertype */
static int lua_isusertype(lua_State *L, int lo, const char *type) {
    if (!lua_isuserdata(L, lo)) {
        if (!push_table_instance(L, lo)) {
            return 0;
        };
    };
    {
        /* check if it is of the same type */
        int r;
        const char *tn;
        if (lua_getmetatable(L, lo)) /* if metatable? */
        {
            lua_rawget(L, LUA_REGISTRYINDEX); /* get registry[mt] */
            tn = lua_tostring(L, -1);
            r = tn && (strcmp(tn, type) == 0);
            lua_pop(L, 1);
            if (r)
                return 1;
            else {
                /* check if it is a specialized class */
                lua_pushstring(L, "neko_tolua_super");
                lua_rawget(L, LUA_REGISTRYINDEX); /* get super */
                lua_getmetatable(L, lo);
                lua_rawget(L, -2); /* get super[mt] */
                if (lua_istable(L, -1)) {
                    int b;
                    lua_pushstring(L, type);
                    lua_rawget(L, -2); /* get super[mt][type] */
                    b = lua_toboolean(L, -1);
                    lua_pop(L, 3);
                    if (b) return 1;
                }
            }
        }
    }
    return 0;
}

int neko_tolua_isnoobj(lua_State *L, int lo, neko_tolua_Error *err) {
    if (lua_gettop(L) < abs(lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "[no object]";
    return 0;
}

int neko_tolua_isboolean(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isboolean(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "boolean";
    return 0;
}

int neko_tolua_isnumber(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnumber(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "number";
    return 0;
}

int neko_tolua_isinteger(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isinteger(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "integer";
    return 0;
}

int neko_tolua_isstring(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isstring(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "string";
    return 0;
}

int neko_tolua_istable(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_istable(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "table";
    return 0;
}

int neko_tolua_isusertable(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isusertable(L, lo, type)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = type;
    return 0;
}

int neko_tolua_isuserdata(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isuserdata(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "userdata";
    return 0;
}

int neko_tolua_isvaluenil(lua_State *L, int lo, neko_tolua_Error *err) {

    if (lua_gettop(L) < abs(lo)) return 0; /* somebody else should chack this */
    if (!lua_isnil(L, lo)) return 0;

    err->index = lo;
    err->array = 0;
    err->type = "value";
    return 1;
};

int neko_tolua_isvalue(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def || abs(lo) <= lua_gettop(L)) /* any valid index */
        return 1;
    err->index = lo;
    err->array = 0;
    err->type = "value";
    return 0;
}

int neko_tolua_isusertype(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isusertype(L, lo, type)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = type;
    return 0;
}

int neko_tolua_isvaluearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else
        return 1;
}

int neko_tolua_isbooleanarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isboolean(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "boolean";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

int neko_tolua_isnumberarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!lua_isnumber(L, -1) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "number";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

int neko_tolua_isintegerarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!lua_isinteger(L, -1) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "integer";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

int neko_tolua_isstringarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isstring(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "string";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

int neko_tolua_istablearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!lua_istable(L, -1) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "table";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

int neko_tolua_isuserdataarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isuserdata(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "userdata";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

int neko_tolua_isusertypearray(lua_State *L, int lo, const char *type, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isuserdata(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->type = type;
                err->array = 1;
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

#if 0
int neko_tolua_isbooleanfield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!(lua_isnil(L,-1) || lua_isboolean(L,-1)) &&
			  !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "boolean";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isnumberfield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!lua_isnumber(L,-1) &&
			  !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "number";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isstringfield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
 if (!(lua_isnil(L,-1) || lua_isstring(L,-1)) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "string";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_istablefield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i+1);
	lua_gettable(L,lo);
	if (! lua_istable(L,-1) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "table";
		return 0;
	}
	lua_pop(L,1);
}

int neko_tolua_isusertablefield
 (lua_State* L, int lo, const char* type, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (! lua_isusertable(L,-1,type) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = type;
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isuserdatafield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!(lua_isnil(L,-1) || lua_isuserdata(L,-1)) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "userdata";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isusertypefield
 (lua_State* L, int lo, const char* type, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!(lua_isnil(L,-1) || lua_isusertype(L,-1,type)) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->type = type;
		err->array = 1;
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

#endif

/* Create metatable
 * Create and register new metatable
 */
static int neko_tolua_newmetatable(lua_State *L, const_str name) {
    int r = luaL_newmetatable(L, name);

#ifdef LUA_VERSION_NUM /* only lua 5.1 */
    if (r) {
        lua_pushvalue(L, -1);
        lua_pushstring(L, name);
        lua_settable(L, LUA_REGISTRYINDEX); /* reg[mt] = type_name */
    };
#endif

    if (r) neko_tolua_classevents(L); /* set meta events */
    lua_pop(L, 1);
    return r;
}

/* Map super classes
 * It sets 'name' as being also a 'base', mapping all super classes of 'base' in 'name'
 */
static void mapsuper(lua_State *L, const char *name, const char *base) {
    /* push registry.super */
    lua_pushstring(L, "neko_tolua_super");
    lua_rawget(L, LUA_REGISTRYINDEX); /* stack: super */
    luaL_getmetatable(L, name);       /* stack: super mt */
    lua_rawget(L, -2);                /* stack: super table */
    if (lua_isnil(L, -1)) {
        /* create table */
        lua_pop(L, 1);
        lua_newtable(L);            /* stack: super table */
        luaL_getmetatable(L, name); /* stack: super table mt */
        lua_pushvalue(L, -2);       /* stack: super table mt table */
        lua_rawset(L, -4);          /* stack: super table */
    }

    /* set base as super class */
    lua_pushstring(L, base);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3); /* stack: super table */

    /* set all super class of base as super class of name */
    luaL_getmetatable(L, base); /* stack: super table base_mt */
    lua_rawget(L, -3);          /* stack: super table base_table */
    if (lua_istable(L, -1)) {
        /* traverse base table */
        lua_pushnil(L); /* first key */
        while (lua_next(L, -2) != 0) {
            /* stack: ... base_table key value */
            lua_pushvalue(L, -2); /* stack: ... base_table key value key */
            lua_insert(L, -2);    /* stack: ... base_table key key value */
            lua_rawset(L, -5);    /* stack: ... base_table key */
        }
    }
    lua_pop(L, 3); /* stack: <empty> */
}

/* creates a 'neko_tolua_ubox' table for base clases, and
// expects the metatable and base metatable on the stack */
static void set_ubox(lua_State *L) {

    /* mt basemt */
    if (!lua_isnil(L, -1)) {
        lua_pushstring(L, "neko_tolua_ubox");
        lua_rawget(L, -2);
    } else {
        lua_pushnil(L);
    };
    /* mt basemt base_ubox */
    if (!lua_isnil(L, -1)) {
        lua_pushstring(L, "neko_tolua_ubox");
        lua_insert(L, -2);
        /* mt basemt key ubox */
        lua_rawset(L, -4);
        /* (mt with ubox) basemt */
    } else {
        /* mt basemt nil */
        lua_pop(L, 1);
        lua_pushstring(L, "neko_tolua_ubox");
        lua_newtable(L);
        /* make weak value metatable for ubox table to allow userdata to be
        garbage-collected */
        lua_newtable(L);
        lua_pushliteral(L, "__mode");
        lua_pushliteral(L, "v");
        lua_rawset(L, -3);       /* stack: string ubox mt */
        lua_setmetatable(L, -2); /* stack:mt basemt string ubox */
        lua_rawset(L, -4);
    };
};

/* Map inheritance
 * It sets 'name' as derived from 'base' by setting 'base' as metatable of 'name'
 */
static void mapinheritance(lua_State *L, const char *name, const char *base) {
    /* set metatable inheritance */
    luaL_getmetatable(L, name);

    if (base && *base)
        luaL_getmetatable(L, base);
    else {

        if (lua_getmetatable(L, -1)) { /* already has a mt, we don't overwrite it */
            lua_pop(L, 2);
            return;
        };
        luaL_getmetatable(L, "neko_tolua_commonclass");
    };

    set_ubox(L);

    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

/* Object type
 */
static int neko_tolua_bnd_type(lua_State *L) {
    neko_tolua_typename(L, lua_gettop(L));
    return 1;
}

/* Take ownership
 */
static int neko_tolua_bnd_takeownership(lua_State *L) {
    int success = 0;
    if (lua_isuserdata(L, 1)) {
        if (lua_getmetatable(L, 1)) /* if metatable? */
        {
            lua_pop(L, 1); /* clear metatable off stack */
/* force garbage collection to avoid C to reuse a to-be-collected address */
#ifdef LUA_VERSION_NUM
            lua_gc(L, LUA_GCCOLLECT, 0);
#else
            lua_setgcthreshold(L, 0);
#endif

            success = neko_tolua_register_gc(L, 1);
        }
    }
    lua_pushboolean(L, success != 0);
    return 1;
}

/* Release ownership
 */
static int neko_tolua_bnd_releaseownership(lua_State *L) {
    int done = 0;
    if (lua_isuserdata(L, 1)) {
        void *u = *((void **)lua_touserdata(L, 1));
/* force garbage collection to avoid releasing a to-be-collected address */
#ifdef LUA_VERSION_NUM
        lua_gc(L, LUA_GCCOLLECT, 0);
#else
        lua_setgcthreshold(L, 0);
#endif
        lua_pushstring(L, "neko_tolua_gc");
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushlightuserdata(L, u);
        lua_rawget(L, -2);
        lua_getmetatable(L, 1);
        if (lua_rawequal(L, -1, -2)) /* check that we are releasing the correct type */
        {
            lua_pushlightuserdata(L, u);
            lua_pushnil(L);
            lua_rawset(L, -5);
            done = 1;
        }
    }
    lua_pushboolean(L, done != 0);
    return 1;
}

/* Type casting
 */
static int neko_tolua_bnd_cast(lua_State *L) {

    /* // old code
            void* v = neko_tolua_tousertype(L,1,NULL);
            const char* s = neko_tolua_tostring(L,2,NULL);
            if (v && s)
             neko_tolua_pushusertype(L,v,s);
            else
             lua_pushnil(L);
            return 1;
    */

    void *v;
    const char *s;
    if (lua_islightuserdata(L, 1)) {
        v = neko_tolua_touserdata(L, 1, NULL);
    } else {
        v = neko_tolua_tousertype(L, 1, 0);
    };

    s = neko_tolua_tostring(L, 2, NULL);
    if (v && s)
        neko_tolua_pushusertype(L, v, s);
    else
        lua_pushnil(L);
    return 1;
}

/* Inheritance
 */
static int neko_tolua_bnd_inherit(lua_State *L) {

    /* stack: lua object, c object */
    lua_pushstring(L, ".c_instance");
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    /* l_obj[".c_instance"] = c_obj */

    return 0;
};

#ifdef LUA_VERSION_NUM /* lua 5.1 */
static int neko_tolua_bnd_setpeer(lua_State *L) {

    /* stack: userdata, table */
    if (!lua_isuserdata(L, -2)) {
        lua_pushstring(L, "Invalid argument #1 to setpeer: userdata expected.");
        lua_error(L);
    };

    if (lua_isnil(L, -1)) {

        lua_pop(L, 1);
        lua_pushvalue(L, TOLUA_NOPEER);
    };
#if LUA_VERSION_NUM > 501
    lua_setuservalue(L, -1);
#else
    lua_setfenv(L, -2);
#endif

    return 0;
};

static int neko_tolua_bnd_getpeer(lua_State *L) {

    /* stack: userdata */
#if LUA_VERSION_NUM > 501
    lua_getuservalue(L, -1);
#else
    lua_getfenv(L, -1);
#endif
    if (lua_rawequal(L, -1, TOLUA_NOPEER)) {
        lua_pop(L, 1);
        lua_pushnil(L);
    };
    return 1;
};
#endif

/* static int class_gc_event (lua_State* L); */

void neko_tolua_open(lua_State *L) {
    int top = lua_gettop(L);
    lua_pushstring(L, "neko_tolua_opened");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isboolean(L, -1)) {
        lua_pushstring(L, "neko_tolua_opened");
        lua_pushboolean(L, 1);
        lua_rawset(L, LUA_REGISTRYINDEX);

#ifndef LUA_VERSION_NUM /* only prior to lua 5.1 */
        /* create peer object table */
        lua_pushstring(L, "neko_tolua_peers");
        lua_newtable(L);
        /* make weak key metatable for peers indexed by userdata object */
        lua_newtable(L);
        lua_pushliteral(L, "__mode");
        lua_pushliteral(L, "k");
        lua_rawset(L, -3);       /* stack: string peers mt */
        lua_setmetatable(L, -2); /* stack: string peers */
        lua_rawset(L, LUA_REGISTRYINDEX);
#endif

        /* create object ptr -> udata mapping table */
        lua_pushstring(L, "neko_tolua_ubox");
        lua_newtable(L);
        /* make weak value metatable for ubox table to allow userdata to be
           garbage-collected */
        lua_newtable(L);
        lua_pushliteral(L, "__mode");
        lua_pushliteral(L, "v");
        lua_rawset(L, -3);       /* stack: string ubox mt */
        lua_setmetatable(L, -2); /* stack: string ubox */
        lua_rawset(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, "neko_tolua_super");
        lua_newtable(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
        lua_pushstring(L, "neko_tolua_gc");
        lua_newtable(L);
        lua_rawset(L, LUA_REGISTRYINDEX);

        /* create gc_event closure */
        lua_pushstring(L, "neko_tolua_gc_event");
        lua_pushstring(L, "neko_tolua_gc");
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushstring(L, "neko_tolua_super");
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushcclosure(L, class_gc_event, 2);
        lua_rawset(L, LUA_REGISTRYINDEX);

        neko_tolua_newmetatable(L, "neko_tolua_commonclass");

        neko_tolua_module(L, NULL, 0);
        neko_tolua_beginmodule(L, NULL);
        neko_tolua_module(L, "tolua", 0);
        neko_tolua_beginmodule(L, "tolua");
        neko_tolua_function(L, "type", neko_tolua_bnd_type);
        neko_tolua_function(L, "takeownership", neko_tolua_bnd_takeownership);
        neko_tolua_function(L, "releaseownership", neko_tolua_bnd_releaseownership);
        neko_tolua_function(L, "cast", neko_tolua_bnd_cast);
        neko_tolua_function(L, "inherit", neko_tolua_bnd_inherit);
#ifdef LUA_VERSION_NUM /* lua 5.1 */
        neko_tolua_function(L, "setpeer", neko_tolua_bnd_setpeer);
        neko_tolua_function(L, "getpeer", neko_tolua_bnd_getpeer);
#endif

        neko_tolua_endmodule(L);
        neko_tolua_endmodule(L);
    }
    lua_settop(L, top);
}

/* Copy a C object
 */
void *neko_tolua_copy(lua_State *L, void *value, unsigned int size) {
    void *clone = (void *)malloc(size);
    if (clone)
        memcpy(clone, value, size);
    else
        neko_tolua_error(L, "insuficient memory", NULL);
    return clone;
}

/* Default collect function
 */
int neko_tolua_default_collect(lua_State *L) {
    void *self = neko_tolua_tousertype(L, 1, 0);
    free(self);
    return 0;
}

/* Do clone
 */
int neko_tolua_register_gc(lua_State *L, int lo) {
    int success = 1;
    void *value = *(void **)lua_touserdata(L, lo);
    lua_pushstring(L, "neko_tolua_gc");
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, value);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) /* make sure that object is not already owned */
        success = 0;
    else {
        lua_pushlightuserdata(L, value);
        lua_getmetatable(L, lo);
        lua_rawset(L, -4);
    }
    lua_pop(L, 2);
    return success;
}

/* Register a usertype
 * It creates the correspoding metatable in the registry, for both 'type' and 'const type'.
 * It maps 'const type' as being also a 'type'
 */
void neko_tolua_usertype(lua_State *L, const char *type) {
    char ctype[128] = "const ";
    strncat(ctype, type, 120);

    /* create both metatables */
    if (neko_tolua_newmetatable(L, ctype) && neko_tolua_newmetatable(L, type)) mapsuper(L, type, ctype); /* 'type' is also a 'const type' */
}

/* Begin module
 * It pushes the module (or class) table on the stack
 */
void neko_tolua_beginmodule(lua_State *L, const char *name) {
    if (name) {
        lua_pushstring(L, name);
        lua_rawget(L, -2);
    } else
        /*	 lua_pushvalue(L,LUA_GLOBALSINDEX);*/
        lua_pushglobaltable(L);
}

/* End module
 * It pops the module (or class) from the stack
 */
void neko_tolua_endmodule(lua_State *L) { lua_pop(L, 1); }

/* Map module
 * It creates a new module
 */
#if 1
void neko_tolua_module(lua_State *L, const char *name, int hasvar) {
    if (name) {
        /* tolua module */
        lua_pushstring(L, name);
        lua_rawget(L, -2);
        if (!lua_istable(L, -1)) /* check if module already exists */
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_rawset(L, -4); /* assing module into module */
        }
    } else {
        /* global table */
        /* lua_pushvalue(L,LUA_GLOBALSINDEX); */
        lua_pushglobaltable(L);
    }
    if (hasvar) {
        if (!neko_tolua_ismodulemetatable(L)) /* check if it already has a module metatable */
        {
            /* create metatable to get/set C/C++ variable */
            lua_newtable(L);
            neko_tolua_moduleevents(L);
            if (lua_getmetatable(L, -2)) lua_setmetatable(L, -2); /* set old metatable as metatable of metatable */
            lua_setmetatable(L, -2);
        }
    }
    lua_pop(L, 1); /* pop module */
}
#else
void neko_tolua_module(lua_State *L, const char *name, int hasvar) {
    if (name) {
        /* tolua module */
        lua_pushstring(L, name);
        lua_newtable(L);
    } else {
        /* global table */
        lua_pushvalue(L, LUA_GLOBALSINDEX);
    }
    if (hasvar) {
        /* create metatable to get/set C/C++ variable */
        lua_newtable(L);
        neko_tolua_moduleevents(L);
        if (lua_getmetatable(L, -2)) lua_setmetatable(L, -2); /* set old metatable as metatable of metatable */
        lua_setmetatable(L, -2);
    }
    if (name)
        lua_rawset(L, -3); /* assing module into module */
    else
        lua_pop(L, 1); /* pop global table */
}
#endif

static void push_collector(lua_State *L, const char *type, lua_CFunction col) {

    /* push collector function, but only if it's not NULL, or if there's no
       collector already */
    if (!col) return;
    luaL_getmetatable(L, type);
    lua_pushstring(L, ".collector");
    /*
    if (!col) {
        lua_pushvalue(L, -1);
        lua_rawget(L, -3);
        if (!lua_isnil(L, -1)) {
            lua_pop(L, 3);
            return;
        };
        lua_pop(L, 1);
    };
    //	*/
    lua_pushcfunction(L, col);

    lua_rawset(L, -3);
    lua_pop(L, 1);
};

/* Map C class
 * It maps a C class, setting the appropriate inheritance and super classes.
 */
void neko_tolua_cclass(lua_State *L, const char *lname, const char *name, const char *base, lua_CFunction col) {
    char cname[128] = "const ";
    char cbase[128] = "const ";
    strncat(cname, name, 120);
    strncat(cbase, base, 120);

    mapinheritance(L, name, base);
    mapinheritance(L, cname, name);

    mapsuper(L, cname, cbase);
    mapsuper(L, name, base);

    lua_pushstring(L, lname);

    push_collector(L, name, col);
    /*
    luaL_getmetatable(L,name);
    lua_pushstring(L,".collector");
    lua_pushcfunction(L,col);

    lua_rawset(L,-3);
    */

    luaL_getmetatable(L, name);
    lua_rawset(L, -3); /* assign class metatable to module */

    /* now we also need to store the collector table for the const
       instances of the class */
    push_collector(L, cname, col);
    /*
    luaL_getmetatable(L,cname);
    lua_pushstring(L,".collector");
    lua_pushcfunction(L,col);
    lua_rawset(L,-3);
    lua_pop(L,1);
    */
}

/* Add base
    * It adds additional base classes to a class (for multiple inheritance)
    * (not for now)
 void neko_tolua_addbase(lua_State* L, char* name, char* base) {

    char cname[128] = "const ";
    char cbase[128] = "const ";
    strncat(cname,name,120);
    strncat(cbase,base,120);

    mapsuper(L,cname,cbase);
    mapsuper(L,name,base);
};
*/

/* Map function
 * It assigns a function into the current module (or class)
 */
void neko_tolua_function(lua_State *L, const char *name, lua_CFunction func) {
    lua_pushstring(L, name);
    lua_pushcfunction(L, func);
    lua_rawset(L, -3);
}

/* sets the __call event for the class (expects the class' main table on top) */
/*	never really worked :(
 void neko_tolua_set_call_event(lua_State* L, lua_CFunction func, char* type) {

    lua_getmetatable(L, -1);
    //luaL_getmetatable(L, type);
    lua_pushstring(L,"__call");
    lua_pushcfunction(L,func);
    lua_rawset(L,-3);
    lua_pop(L, 1);
};
*/

/* Map constant number
 * It assigns a constant number into the current module (or class)
 */
void neko_tolua_constant(lua_State *L, const char *name, lua_Number value) {
    lua_pushstring(L, name);
    if ((long long)value == value)
        neko_tolua_pushinteger(L, value);
    else
        neko_tolua_pushnumber(L, value);
    lua_rawset(L, -3);
}

/* Map variable
 * It assigns a variable into the current module (or class)
 */
void neko_tolua_variable(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set) {
    /* get func */
    lua_pushstring(L, ".get");
    lua_rawget(L, -2);
    if (!lua_istable(L, -1)) {
        /* create .get table, leaving it at the top */
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushstring(L, ".get");
        lua_pushvalue(L, -2);
        lua_rawset(L, -4);
    }
    lua_pushstring(L, name);
    lua_pushcfunction(L, get);
    lua_rawset(L, -3); /* store variable */
    lua_pop(L, 1);     /* pop .get table */

    /* set func */
    if (set) {
        lua_pushstring(L, ".set");
        lua_rawget(L, -2);
        if (!lua_istable(L, -1)) {
            /* create .set table, leaving it at the top */
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushstring(L, ".set");
            lua_pushvalue(L, -2);
            lua_rawset(L, -4);
        }
        lua_pushstring(L, name);
        lua_pushcfunction(L, set);
        lua_rawset(L, -3); /* store variable */
        lua_pop(L, 1);     /* pop .set table */
    }
}

/* Access const array
 * It reports an error when trying to write into a const array
 */
static int const_array(lua_State *L) {
    luaL_error(L, "value of const array cannot be changed");
    return 0;
}

/* Map an array
 * It assigns an array into the current module (or class)
 */
void neko_tolua_array(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set) {
    lua_pushstring(L, ".get");
    lua_rawget(L, -2);
    if (!lua_istable(L, -1)) {
        /* create .get table, leaving it at the top */
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushstring(L, ".get");
        lua_pushvalue(L, -2);
        lua_rawset(L, -4);
    }
    lua_pushstring(L, name);

    lua_newtable(L); /* create array metatable */
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2); /* set the own table as metatable (for modules) */
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, get);
    lua_rawset(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, set ? set : const_array);
    lua_rawset(L, -3);

    lua_rawset(L, -3); /* store variable */
    lua_pop(L, 1);     /* pop .get table */
}

void neko_tolua_dobuffer(lua_State *L, char *B, unsigned int size, const char *name) { luaL_loadbuffer(L, B, size, name) || lua_pcall(L, 0, 0, 0); };

void neko_tolua_pushvalue(lua_State *L, int lo) { lua_pushvalue(L, lo); }

void neko_tolua_pushboolean(lua_State *L, int value) { lua_pushboolean(L, value); }

void neko_tolua_pushnumber(lua_State *L, lua_Number value) { lua_pushnumber(L, value); }

void neko_tolua_pushinteger(lua_State *L, lua_Integer value) { lua_pushinteger(L, value); }

void neko_tolua_pushstring(lua_State *L, const char *value) {
    if (value == NULL)
        lua_pushnil(L);
    else
        lua_pushstring(L, value);
}

void neko_tolua_pushuserdata(lua_State *L, void *value) {
    if (value == NULL)
        lua_pushnil(L);
    else
        lua_pushlightuserdata(L, value);
}

void neko_tolua_pushusertype(lua_State *L, void *value, const char *type) {
    if (value == NULL)
        lua_pushnil(L);
    else {
        luaL_getmetatable(L, type);
        lua_pushstring(L, "neko_tolua_ubox");
        lua_rawget(L, -2); /* stack: mt ubox */
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushstring(L, "neko_tolua_ubox");
            lua_rawget(L, LUA_REGISTRYINDEX);
        };
        lua_pushlightuserdata(L, value);
        lua_rawget(L, -2); /* stack: mt ubox ubox[u] */
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1); /* stack: mt ubox */
            lua_pushlightuserdata(L, value);
            *(void **)lua_newuserdata(L, sizeof(void *)) = value; /* stack: mt ubox u newud */
            lua_pushvalue(L, -1);                                 /* stack: mt ubox u newud newud */
            lua_insert(L, -4);                                    /* stack: mt newud ubox u newud */
            lua_rawset(L, -3);                                    /* stack: mt newud ubox */
            lua_pop(L, 1);                                        /* stack: mt newud */
                                                                  /*luaL_getmetatable(L,type);*/
            lua_pushvalue(L, -2);                                 /* stack: mt newud mt */
            lua_setmetatable(L, -2);                              /* stack: mt newud */
        } else {
            /* check the need of updating the metatable to a more specialized class */
            lua_insert(L, -2); /* stack: mt ubox[u] ubox */
            lua_pop(L, 1);     /* stack: mt ubox[u] */
            lua_pushstring(L, "neko_tolua_super");
            lua_rawget(L, LUA_REGISTRYINDEX); /* stack: mt ubox[u] super */
            lua_getmetatable(L, -2);          /* stack: mt ubox[u] super mt */
            lua_rawget(L, -2);                /* stack: mt ubox[u] super super[mt] */
            if (lua_istable(L, -1)) {
                lua_pushstring(L, type);       /* stack: mt ubox[u] super super[mt] type */
                lua_rawget(L, -2);             /* stack: mt ubox[u] super super[mt] flag */
                if (lua_toboolean(L, -1) == 1) /* if true */
                {
                    lua_pop(L, 3); /* mt ubox[u]*/
                } else {
                    /* type represents a more specilized type */
                    /*luaL_getmetatable(L,type);             // stack: mt ubox[u] super super[mt] flag mt */
                    lua_pushvalue(L, -5);    /* stack: mt ubox[u] super super[mt] flag mt */
                    lua_setmetatable(L, -5); /* stack: mt ubox[u] super super[mt] flag */
                    lua_pop(L, 3);           /* stack: mt ubox[u] */
                }
            } else {
                lua_pop(L, 2); /* stack: mt ubox[u] */
            }
        }
#if LUA_VERSION_NUM == 501
        lua_pushvalue(L, TOLUA_NOPEER);
        lua_setfenv(L, -2);
#elif LUA_VERSION_NUM > 501
        lua_pushvalue(L, TOLUA_NOPEER);
        lua_setuservalue(L, -2);
#endif

        lua_remove(L, -2); /* stack: ubox[u]*/
    }
}

void neko_tolua_pushusertype_and_takeownership(lua_State *L, void *value, const char *type) {
    neko_tolua_pushusertype(L, value, type);
    neko_tolua_register_gc(L, lua_gettop(L));
}

void neko_tolua_pushfieldvalue(lua_State *L, int lo, int index, int v) {
    lua_pushnumber(L, index);
    lua_pushvalue(L, v);
    lua_settable(L, lo);
}

void neko_tolua_pushfieldboolean(lua_State *L, int lo, int index, int v) {
    lua_pushnumber(L, index);
    lua_pushboolean(L, v);
    lua_settable(L, lo);
}

void neko_tolua_pushfieldnumber(lua_State *L, int lo, int index, lua_Number v) {
    lua_pushnumber(L, index);
    neko_tolua_pushnumber(L, v);
    lua_settable(L, lo);
}

void neko_tolua_pushfieldinteger(lua_State *L, int lo, int index, lua_Integer v) {
    lua_pushinteger(L, index);
    neko_tolua_pushinteger(L, v);
    lua_settable(L, lo);
}

void neko_tolua_pushfieldstring(lua_State *L, int lo, int index, const char *v) {
    lua_pushnumber(L, index);
    neko_tolua_pushstring(L, v);
    lua_settable(L, lo);
}

void neko_tolua_pushfielduserdata(lua_State *L, int lo, int index, void *v) {
    lua_pushnumber(L, index);
    neko_tolua_pushuserdata(L, v);
    lua_settable(L, lo);
}

void neko_tolua_pushfieldusertype(lua_State *L, int lo, int index, void *v, const char *type) {
    lua_pushnumber(L, index);
    neko_tolua_pushusertype(L, v, type);
    lua_settable(L, lo);
}

void neko_tolua_pushfieldusertype_and_takeownership(lua_State *L, int lo, int index, void *v, const char *type) {
    lua_pushnumber(L, index);
    neko_tolua_pushusertype(L, v, type);
    neko_tolua_register_gc(L, lua_gettop(L));
    lua_settable(L, lo);
}

lua_Number neko_tolua_tonumber(lua_State *L, int narg, lua_Number def) { return lua_gettop(L) < abs(narg) ? def : lua_tonumber(L, narg); }

lua_Integer neko_tolua_tointeger(lua_State *L, int narg, lua_Integer def) { return lua_gettop(L) < abs(narg) ? def : lua_tointeger(L, narg); }

const char *neko_tolua_tostring(lua_State *L, int narg, const char *def) { return lua_gettop(L) < abs(narg) ? def : lua_tostring(L, narg); }

void *neko_tolua_touserdata(lua_State *L, int narg, void *def) {

    /* return lua_gettop(L)<abs(narg) ? def : lua_touserdata(L,narg); */

    if (lua_gettop(L) < abs(narg)) {
        return def;
    };

    if (lua_islightuserdata(L, narg)) {

        return lua_touserdata(L, narg);
    };

    return neko_tolua_tousertype(L, narg, def);
}

extern int push_table_instance(lua_State *L, int lo);

void *neko_tolua_tousertype(lua_State *L, int narg, void *def) {
    if (lua_gettop(L) < abs(narg))
        return def;
    else {
        void *u;
        if (!lua_isuserdata(L, narg)) {
            if (!push_table_instance(L, narg)) return NULL;
        };
        u = lua_touserdata(L, narg);
        return (u == NULL) ? NULL : *((void **)u); /* nil represents NULL */
    }
}

int neko_tolua_tovalue(lua_State *L, int narg, int def) { return lua_gettop(L) < abs(narg) ? def : narg; }

int neko_tolua_toboolean(lua_State *L, int narg, int def) { return lua_gettop(L) < abs(narg) ? def : lua_toboolean(L, narg); }

lua_Number neko_tolua_tofieldnumber(lua_State *L, int lo, int index, lua_Number def) {
    double v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_tonumber(L, -1);
    lua_pop(L, 1);
    return v;
}

lua_Integer neko_tolua_tofieldinteger(lua_State *L, int lo, int index, lua_Integer def) {
    lua_Integer v;
    lua_pushinteger(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_tointeger(L, -1);
    lua_pop(L, 1);
    return v;
}

const char *neko_tolua_tofieldstring(lua_State *L, int lo, int index, const char *def) {
    const char *v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_tostring(L, -1);
    lua_pop(L, 1);
    return v;
}

void *neko_tolua_tofielduserdata(lua_State *L, int lo, int index, void *def) {
    void *v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_touserdata(L, -1);
    lua_pop(L, 1);
    return v;
}

void *neko_tolua_tofieldusertype(lua_State *L, int lo, int index, void *def) {
    void *v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : (*(void **)(lua_touserdata(L, -1))); /* lua_unboxpointer(L,-1); */
    lua_pop(L, 1);
    return v;
}

int neko_tolua_tofieldvalue(lua_State *L, int lo, int index, int def) {
    int v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lo;
    lua_pop(L, 1);
    return v;
}

int neko_tolua_getfieldboolean(lua_State *L, int lo, int index, int def) {
    int v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? 0 : lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

#endif