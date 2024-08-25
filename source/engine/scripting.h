#ifndef SCRIPT_H
#define SCRIPT_H

#include "engine/base.h"
#include "engine/input.h"
#include "engine/luax.h"


NEKO_API() void script_run_string(const char *s);
NEKO_API() void script_run_file(const char *filename);
NEKO_API() void script_error(const char *s);

NEKO_API() void script_init();
NEKO_API() void script_fini();
NEKO_API() void script_update_all();
NEKO_API() void script_post_update_all();
NEKO_API() void script_draw_ui();
NEKO_API() void script_draw_all();
NEKO_API() void script_key_down(KeyCode key);
NEKO_API() void script_key_up(KeyCode key);
NEKO_API() void script_mouse_down(MouseCode mouse);
NEKO_API() void script_mouse_up(MouseCode mouse);
NEKO_API() void script_mouse_move(vec2 pos);
NEKO_API() void script_scroll(vec2 scroll);
NEKO_API() void script_save_all(Store *s);
NEKO_API() void script_load_all(Store *s);

NEKO_API() int luax_pcall_nothrow(lua_State *L, int nargs, int nresults);
NEKO_API() void script_push_event(const char *event);

#define errcheck(...)                                         \
    do                                                        \
        if (__VA_ARGS__) {                                    \
            console_printf("lua: %s\n", lua_tostring(L, -1)); \
            lua_pop(L, 1);                                    \
            if (LockGuard lock{&g_app->error_mtx}) {          \
                g_app->error_mode.store(true);                \
            }                                                 \
        }                                                     \
    while (0)

#endif

#ifndef NEKO_LUA_STRUCT_H
#define NEKO_LUA_STRUCT_H

#include "engine/luax.h"

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

NEKO_API() int LUASTRUCT_new(lua_State *L, const char *metatable, size_t size);
NEKO_API() int LUASTRUCT_newref(lua_State *L, const char *metatable, int parentIndex, const void *data);
NEKO_API() int LUASTRUCT_is(lua_State *L, const char *metatable, int index);
NEKO_API() void *LUASTRUCT_todata(lua_State *L, const char *metatable, int index, int required);

// luabind

#define neko_luabind_type(L, type) neko_luabind_type_add(L, #type, sizeof(type))

enum { LUAA_INVALID_TYPE = -1 };

typedef lua_Integer neko_luabind_Type;
typedef int (*neko_luabind_Pushfunc)(lua_State *, neko_luabind_Type, const void *);
typedef void (*neko_luabind_Tofunc)(lua_State *, neko_luabind_Type, void *, int);

NEKO_API() neko_luabind_Type neko_luabind_type_add(lua_State *L, const char *type, size_t size);
NEKO_API() neko_luabind_Type neko_luabind_type_find(lua_State *L, const char *type);
NEKO_API() const char *neko_luabind_typename(lua_State *L, neko_luabind_Type id);
NEKO_API() size_t neko_luabind_typesize(lua_State *L, neko_luabind_Type id);

#define neko_luabind_push(L, type, c_in) neko_luabind_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_to(L, type, c_out, index) neko_luabind_to_type(L, neko_luabind_type(L, type), c_out, index)

NEKO_API() int neko_luabind_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
NEKO_API() void neko_luabind_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

#define LUAA_INVALID_MEMBER_NAME NULL

#define neko_lua_enum(L, type) neko_lua_enum_type(L, neko_luabind_type(L, type), sizeof(type))

#define neko_lua_enum_value(L, type, value)                    \
    const type __neko_lua_enum_value_temp_##value[] = {value}; \
    neko_lua_enum_value_type(L, neko_luabind_type(L, type), __neko_lua_enum_value_temp_##value, #value)

#define neko_lua_enum_value_name(L, type, value, name)         \
    const type __neko_lua_enum_value_temp_##value[] = {value}; \
    neko_lua_enum_value_type(L, neko_luabind_type(L, type), __neko_lua_enum_value_temp_##value, name)

#define neko_lua_enum_push(L, type, c_in) neko_lua_enum_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_lua_enum_to(L, type, c_out, index) neko_lua_enum_to_type(L, neko_luabind_type(L, type), c_out, index)

#define neko_lua_enum_has_value(L, type, value)                \
    const type __neko_lua_enum_value_temp_##value[] = {value}; \
    neko_lua_enum_has_value_type(L, neko_luabind_type(L, type), __neko_lua_enum_value_temp_##value)

#define neko_lua_enum_has_name(L, type, name) neko_lua_enum_has_name_type(L, neko_luabind_type(L, type), name)

#define neko_lua_enum_registered(L, type) neko_lua_enum_registered_type(L, neko_luabind_type(L, type))
#define neko_lua_enum_next_value_name(L, type, member) neko_lua_enum_next_value_name_type(L, neko_luabind_type(L, type), member)

NEKO_API() void neko_lua_enum_type(lua_State *L, neko_luabind_Type type, size_t size);
NEKO_API() void neko_lua_enum_value_type(lua_State *L, neko_luabind_Type type, const void *value, const char *name);
NEKO_API() int neko_lua_enum_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
NEKO_API() void neko_lua_enum_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);
NEKO_API() bool neko_lua_enum_has_value_type(lua_State *L, neko_luabind_Type type, const void *value);
NEKO_API() bool neko_lua_enum_has_name_type(lua_State *L, neko_luabind_Type type, const char *name);
NEKO_API() bool neko_lua_enum_registered_type(lua_State *L, neko_luabind_Type type);
NEKO_API() const char *neko_lua_enum_next_value_name_type(lua_State *L, neko_luabind_Type type, const char *member);

NEKO_API() void createStructTables(lua_State *L);

NEKO_API() void open_neko_api(lua_State *L);

#endif