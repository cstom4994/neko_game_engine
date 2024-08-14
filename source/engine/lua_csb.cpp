
#include "engine/luax.h"

enum class TYPEID { f_int8, f_int16, f_int32, f_int64, f_uint8, f_uint16, f_uint32, f_uint64, f_bool, f_ptr, f_float, f_double, f_COUNT };

#define TYPE_ID_(type) ((int)TYPEID::f_##type)

static inline void set_int8(void* p, lua_Integer v) { *(int8_t*)p = (int8_t)v; }
static inline void set_int16(void* p, lua_Integer v) { *(int16_t*)p = (int16_t)v; }
static inline void set_int32(void* p, lua_Integer v) { *(int32_t*)p = (int32_t)v; }
static inline void set_int64(void* p, lua_Integer v) { *(int64_t*)p = (int64_t)v; }
static inline void set_uint8(void* p, lua_Integer v) { *(int8_t*)p = (uint8_t)v; }
static inline void set_uint16(void* p, lua_Integer v) { *(int16_t*)p = (uint16_t)v; }
static inline void set_uint32(void* p, lua_Integer v) { *(int32_t*)p = (uint32_t)v; }
static inline void set_uint64(void* p, lua_Integer v) { *(int64_t*)p = (uint64_t)v; }
static inline void set_float(void* p, lua_Number v) { *(float*)p = (float)v; }
static inline void set_bool(void* p, int v) { *(int8_t*)p = v; }
static inline void set_ptr(void* p, void* v) { *(void**)p = v; }
static inline void set_double(void* p, lua_Number v) { *(float*)p = (double)v; }
static inline int8_t get_int8(void* p) { return *(int8_t*)p; }
static inline int16_t get_int16(void* p) { return *(int16_t*)p; }
static inline int32_t get_int32(void* p) { return *(int32_t*)p; }
static inline int64_t get_int64(void* p) { return *(int64_t*)p; }
static inline uint8_t get_uint8(void* p) { return *(uint8_t*)p; }
static inline uint16_t get_uint16(void* p) { return *(uint16_t*)p; }
static inline uint32_t get_uint32(void* p) { return *(uint32_t*)p; }
static inline uint64_t get_uint64(void* p) { return *(uint64_t*)p; }
static inline void* get_ptr(void* p) { return *(void**)p; }
static inline int get_bool(void* p) { return *(int8_t*)p; }
static inline float get_float(void* p) { return *(float*)p; }
static inline double get_double(void* p) { return *(double*)p; }

static inline int get_stride(int type) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            return sizeof(int8_t);
        case TYPEID::f_int16:
            return sizeof(int16_t);
        case TYPEID::f_int32:
            return sizeof(int32_t);
        case TYPEID::f_int64:
            return sizeof(int64_t);
        case TYPEID::f_uint8:
            return sizeof(uint8_t);
        case TYPEID::f_uint16:
            return sizeof(uint16_t);
        case TYPEID::f_uint32:
            return sizeof(uint32_t);
        case TYPEID::f_uint64:
            return sizeof(uint64_t);
        case TYPEID::f_ptr:
            return sizeof(void*);
        case TYPEID::f_bool:
            return sizeof(bool);
        case TYPEID::f_float:
            return sizeof(float);
        case TYPEID::f_double:
            return sizeof(double);
        default:
            return 0;
    }
}

static int setter(lua_State* L, void* p, int type, int offset) {
    p = (char*)p + offset * get_stride(type);
    switch (type) {
        case TYPE_ID_(int8):
            set_int8(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int16):
            set_int16(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int32):
            set_int32(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int64):
            set_int64(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint8):
            set_uint8(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint16):
            set_uint16(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint32):
            set_uint32(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint64):
            set_uint64(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(bool):
            set_bool(p, lua_toboolean(L, 2));
            break;
        case TYPE_ID_(ptr):
            set_ptr(p, lua_touserdata(L, 2));
            break;
        case TYPE_ID_(float):
            set_float(p, luaL_checknumber(L, 2));
            break;
        case TYPE_ID_(double):
            set_double(p, luaL_checknumber(L, 2));
            break;
    }
    return 0;
}

static inline int getter(lua_State* L, void* p, int type, int offset) {
    p = (char*)p + offset * get_stride(type);
    switch (type) {
        case TYPE_ID_(int8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPE_ID_(int16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPE_ID_(int32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPE_ID_(int64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPE_ID_(uint8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPE_ID_(uint16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPE_ID_(uint32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPE_ID_(uint64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPE_ID_(bool):
            lua_pushboolean(L, get_bool(p));
            break;
        case TYPE_ID_(ptr):
            lua_pushlightuserdata(L, get_ptr(p));
            break;
        case TYPE_ID_(float):
            lua_pushnumber(L, get_float(p));
            break;
        case TYPE_ID_(double):
            lua_pushnumber(L, get_double(p));
            break;
    }
    return 1;
}

#define CSB_FUNC(TYPE, OFF)                                                                                      \
    static int get_##TYPE##_##OFF(lua_State* L) { return getter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), OFF); } \
    static int set_##TYPE##_##OFF(lua_State* L) { return setter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), OFF); }

#define CSB_OFFSET(TYPE)                                                                                                                            \
    static int get_##TYPE##_offset(lua_State* L) { return getter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), lua_tointeger(L, lua_upvalueindex(1))); } \
    static int set_##TYPE##_offset(lua_State* L) { return setter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), lua_tointeger(L, lua_upvalueindex(1))); }

#define CSB(opt, TYPE)                                          \
    static int opt##ter_func_##TYPE(lua_State* L, int offset) { \
        switch (offset) {                                       \
            case 0:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_0);         \
                break;                                          \
            case 1:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_1);         \
                break;                                          \
            case 2:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_2);         \
                break;                                          \
            case 3:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_3);         \
                break;                                          \
            case 4:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_4);         \
                break;                                          \
            case 5:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_5);         \
                break;                                          \
            case 6:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_6);         \
                break;                                          \
            case 7:                                             \
                lua_pushcfunction(L, opt##_##TYPE##_7);         \
                break;                                          \
            default:                                            \
                lua_pushinteger(L, offset);                     \
                lua_pushcclosure(L, opt##_##TYPE##_offset, 1);  \
                break;                                          \
        }                                                       \
        return 1;                                               \
    }

#define XX(TYPE)      \
    CSB_FUNC(TYPE, 0) \
    CSB_FUNC(TYPE, 1) \
    CSB_FUNC(TYPE, 2) \
    CSB_FUNC(TYPE, 3) \
    CSB_FUNC(TYPE, 4) \
    CSB_FUNC(TYPE, 5) \
    CSB_FUNC(TYPE, 6) \
    CSB_FUNC(TYPE, 7) \
    CSB_OFFSET(TYPE)  \
    CSB(get, TYPE)    \
    CSB(set, TYPE)

XX(float)
XX(double)
XX(int8)
XX(int16)
XX(int32)
XX(int64)
XX(uint8)
XX(uint16)
XX(uint32)
XX(uint64)
XX(bool)
XX(ptr)

#undef XX

static inline int get_value(lua_State* L, int type, int offset) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            getter_func_int8(L, offset);
            break;
        case TYPEID::f_int16:
            getter_func_int16(L, offset);
            break;
        case TYPEID::f_int32:
            getter_func_int32(L, offset);
            break;
        case TYPEID::f_int64:
            getter_func_int64(L, offset);
            break;
        case TYPEID::f_uint8:
            getter_func_uint8(L, offset);
            break;
        case TYPEID::f_uint16:
            getter_func_uint16(L, offset);
            break;
        case TYPEID::f_uint32:
            getter_func_uint32(L, offset);
            break;
        case TYPEID::f_uint64:
            getter_func_uint64(L, offset);
            break;
        case TYPEID::f_bool:
            getter_func_bool(L, offset);
            break;
        case TYPEID::f_ptr:
            getter_func_ptr(L, offset);
            break;
        case TYPEID::f_float:
            getter_func_float(L, offset);
            break;
        case TYPEID::f_double:
            getter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int getter_direct(lua_State* L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return get_value(L, type, offset);
}

static inline int set_value(lua_State* L, int type, int offset) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            setter_func_int8(L, offset);
            break;
        case TYPEID::f_int16:
            setter_func_int16(L, offset);
            break;
        case TYPEID::f_int32:
            setter_func_int32(L, offset);
            break;
        case TYPEID::f_int64:
            setter_func_int64(L, offset);
            break;
        case TYPEID::f_uint8:
            setter_func_uint8(L, offset);
            break;
        case TYPEID::f_uint16:
            setter_func_uint16(L, offset);
            break;
        case TYPEID::f_uint32:
            setter_func_uint32(L, offset);
            break;
        case TYPEID::f_uint64:
            setter_func_uint64(L, offset);
            break;
        case TYPEID::f_bool:
            setter_func_bool(L, offset);
            break;
        case TYPEID::f_ptr:
            setter_func_ptr(L, offset);
            break;
        case TYPEID::f_float:
            setter_func_float(L, offset);
            break;
        case TYPEID::f_double:
            setter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int setter_direct(lua_State* L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return set_value(L, type, offset);
}

#define LUATYPEID(type, typen)                 \
    lua_pushinteger(L, (int)TYPEID::f_##type); \
    lua_setfield(L, -2, #typen);

struct address_path {
    uint8_t type;
    uint8_t offset[1];
};

static const uint8_t* get_offset(const uint8_t* offset, size_t sz, int* output) {
    if (sz == 0) return NULL;
    if (offset[0] < 128) {
        *output = offset[0];
        return offset + 1;
    }
    int t = offset[0] & 0x7f;
    size_t i;
    int shift = 7;
    for (i = 1; i < sz; i++) {
        if (offset[i] < 128) {
            t |= offset[i] << shift;
            *output = t;
            return offset + i + 1;
        } else {
            t |= (offset[i] & 0x7f) << shift;
            shift += 7;
        }
    }
    return NULL;
}

static void* address_ptr(lua_State* L, int* type, int* offset) {
    size_t sz;
    const uint8_t* buf = (const uint8_t*)lua_tolstring(L, lua_upvalueindex(1), &sz);
    if (sz == 0 || buf[0] >= (int)TYPEID::f_COUNT) luaL_error(L, "Invalid type");
    void** p = (void**)lua_touserdata(L, 1);
    const uint8_t* endptr = buf + sz;
    sz--;
    const uint8_t* ptr = &buf[1];
    for (;;) {
        int off = 0;
        ptr = get_offset(ptr, sz, &off);
        if (ptr == NULL) luaL_error(L, "Invalid offset");
        sz = endptr - ptr;
        if (sz == 0) {
            *type = buf[0];
            *offset = off;
            return p;
        } else {
            p += off;
            if (*p == NULL) return NULL;
            p = (void**)*p;
        }
    }
}

static int get_indirect(lua_State* L) {
    int type;
    int offset;
    void* p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return getter(L, p, type, offset);
}

static int set_indirect(lua_State* L) {
    int type;
    int offset;
    void* p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return setter(L, p, type, offset);
}

static void address(lua_State* L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) luaL_error(L, "Invalid type %d", type);
    int top = lua_gettop(L);
    if (top <= 2) {
        luaL_error(L, "Need two or more offsets");
    }
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addchar(&b, type);
    int i;
    for (i = 2; i <= top; i++) {
        unsigned int offset = (unsigned int)luaL_checkinteger(L, i);
        if (i != top) {
            if (offset % sizeof(void*) != 0) luaL_error(L, "%d is not align to pointer", offset);
            offset /= sizeof(void*);
        } else {
            int stride = get_stride(type);
            if (offset % stride != 0) luaL_error(L, "%d is not align to %d", offset, stride);
            offset /= stride;
        }

        if (offset < 128) {
            luaL_addchar(&b, offset);
        } else {
            while (offset >= 128) {
                luaL_addchar(&b, (char)(0x80 | (offset & 0x7f)));
                offset >>= 7;
            }
            luaL_addchar(&b, offset);
        }
    }
    luaL_pushresult(&b);
}

int lua_csb_core(lua_State* L) {

    lua_newtable(L);
    lua_pushcclosure(
            L,
            [](lua_State* L) {
                int top = lua_gettop(L);
                if (top <= 2) {
                    getter_direct(L);
                    setter_direct(L);
                    return 2;
                } else {
                    address(L);
                    int cmd = lua_gettop(L);
                    lua_pushvalue(L, cmd);
                    lua_pushcclosure(L, get_indirect, 1);
                    lua_pushvalue(L, cmd);
                    lua_pushcclosure(L, set_indirect, 1);
                    return 2;
                }
            },
            0);
    lua_setfield(L, -2, "cstruct");

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                if (!lua_isuserdata(L, 1)) return luaL_error(L, "Need userdata at 1");
                char* p = (char*)lua_touserdata(L, 1);
                size_t off = luaL_checkinteger(L, 2);
                lua_pushlightuserdata(L, (void*)(p + off));
                return 1;
            },
            0);
    lua_setfield(L, -2, "offset");

    lua_pushcclosure(
            L,
            [](lua_State* L) {
                lua_newtable(L);
                LUATYPEID(int8, int8_t);
                LUATYPEID(int16, int16_t);
                LUATYPEID(int32, int32_t);
                LUATYPEID(int64, int64_t);
                LUATYPEID(uint8, uint8_t);
                LUATYPEID(uint16, uint16_t);
                LUATYPEID(uint32, uint32_t);
                LUATYPEID(uint64, uint64_t);
                LUATYPEID(bool, bool);
                LUATYPEID(ptr, ptr);
                LUATYPEID(float, float);
                LUATYPEID(double, double);
                return 1;
            },
            0);
    lua_setfield(L, -2, "typeid");

    return 1;
}

static int luastruct_newudata(lua_State* L) {
    size_t sz = luaL_checkinteger(L, 1);
    lua_newuserdatauv(L, sz, 0);
    return 1;
}

static int open_embed_luastruct_test(lua_State* L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"udata", luastruct_newudata},
            {"NULL", NULL},
            {NULL, NULL},
    };
    luaL_newlib(L, l);
    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");
    return 1;
}

static const u8 csb_lua_data[] = {
#include "csb.lua.h"
};

int lua_csb(lua_State* L) {
    // ecs_t* w = (ecs_t*)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    std::string contents = (const_str)csb_lua_data;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), "csb.lua") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load csb");
    }
    // lua_getglobal(L, "CStructBridge");
    // lua_pushvalue(L, 1);
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        const char* errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "csb error: %s\n", errorMsg);
        // lua_pop(L, 1);
        neko_panic("failed to run csb");
    }
    console_log("loaded csb");
    return 1;
}