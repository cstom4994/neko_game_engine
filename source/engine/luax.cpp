#include "engine/luax.hpp"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/bootstrap.h"
#include "engine/scripting.h"

namespace neko {

void neko_lua_run_string(lua_State *m_ls, const_str str_) {
    if (luaL_dostring(m_ls, str_)) {
        std::string err = lua_tool::dump_error(m_ls, "run_string ::lua_pcall_wrap failed str<%s>", str_);
        ::lua_pop(m_ls, 1);
        // console_log("%s", err.c_str());
    }
}

namespace luavalue {
void set(lua_State *L, int idx, value &v) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            v.emplace<std::monostate>();
            break;
        case LUA_TBOOLEAN:
            v.emplace<bool>(!!lua_toboolean(L, idx));
            break;
        case LUA_TLIGHTUSERDATA:
            v.emplace<void *>(lua_touserdata(L, idx));
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, idx)) {
                v.emplace<lua_Integer>(lua_tointeger(L, idx));
            } else {
                v.emplace<lua_Number>(lua_tonumber(L, idx));
            }
            break;
        case LUA_TSTRING: {
            size_t sz = 0;
            const char *str = lua_tolstring(L, idx, &sz);
            v.emplace<std::string>(str, sz);
            break;
        }
        case LUA_TFUNCTION: {
            lua_CFunction func = lua_tocfunction(L, idx);
            if (func == NULL || lua_getupvalue(L, idx, 1) != NULL) {
                luaL_error(L, "Only light C function can be serialized");
                return;
            }
            v.emplace<lua_CFunction>(func);
            break;
        }
        default:
            luaL_error(L, "Unsupport type %s to serialize", lua_typename(L, idx));
    }
}

void set(lua_State *L, int idx, table &t) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        size_t sz = 0;
        const char *str = luaL_checklstring(L, -2, &sz);
        std::pair<std::string, value> pair;
        pair.first.assign(str, sz);
        set(L, -1, pair.second);
        t.emplace(pair);
        lua_pop(L, 1);
    }
}

void get(lua_State *L, const value &v) {
    std::visit(
            [=](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    lua_pushnil(L);
                } else if constexpr (std::is_same_v<T, bool>) {
                    lua_pushboolean(L, arg);
                } else if constexpr (std::is_same_v<T, void *>) {
                    lua_pushlightuserdata(L, arg);
                } else if constexpr (std::is_same_v<T, lua_Integer>) {
                    lua_pushinteger(L, arg);
                } else if constexpr (std::is_same_v<T, lua_Number>) {
                    lua_pushnumber(L, arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    lua_pushlstring(L, arg.data(), arg.size());
                } else if constexpr (std::is_same_v<T, lua_CFunction>) {
                    lua_pushcfunction(L, arg);
                } else {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            },
            v);
}

void get(lua_State *L, const table &t) {
    lua_createtable(L, 0, static_cast<int>(t.size()));
    for (const auto &[k, v] : t) {
        lua_pushlstring(L, k.data(), k.size());
        get(L, v);
        lua_rawset(L, -3);
    }
}
}  // namespace luavalue

int vfs_lua_loader(lua_State *L) {
    const_str name = luaL_checkstring(L, 1);
    std::string path = name;
    std::replace(path.begin(), path.end(), '.', '/');

    bool ok = false;
    auto load_list = {"lite/data/"};
    for (auto p : load_list) {
        std::string load_path = p + path + ".lua";
        String contents = {};
        ok = vfs_read_entire_file(&contents, load_path.c_str());
        neko_println("fuck:%s", load_path.c_str());
        if (ok) {
            neko_defer(mem_free(contents.data));
            if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {
                lua_pushfstring(L, "vfs_lua_loader error loading module \"%s\"", name);
                lua_pop(L, 1);
            } else {
                console_log("vfs_lua_loader loaded : \"%s\"", path.c_str());
            }
            return 1;
        }
    }

    for (auto p : load_list) {
        std::string load_path = p + path + "/init.lua";
        String contents = {};
        ok = vfs_read_entire_file(&contents, load_path.c_str());
        neko_println("fuck:%s", load_path.c_str());
        if (ok) {
            neko_defer(mem_free(contents.data));
            if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {
                lua_pushfstring(L, "vfs_lua_loader error loading module \"%s\"", name);
                lua_pop(L, 1);
            } else {
                console_log("vfs_lua_loader loaded : \"%s\"", path.c_str());
            }
            return 1;
        }
    }

#if 0
    u8* data;
    u32 data_size;

    int result = neko_pak_item_data(&ENGINE_INTERFACE()->pack, path.c_str(), (const u8**)&data, &data_size);
    if (result == 0) {
        if (luaL_loadbuffer(L, (char*)data, data_size, name) != LUA_OK) {
            lua_pop(L, 1);
        } else {
            neko_println("loaded:%s", path.c_str());
        }

        neko_pak_item_free(&ENGINE_INTERFACE()->pack, data);
    }
#endif

    // lua_pushfstring(L, "vfs_lua_loader module \"%s\" not found", name);
    return 0;
}

}  // namespace neko

i32 luax_require_script(lua_State *L, String filepath) {
    PROFILE_FUNC();

    if (g_app->error_mode.load()) {
        return LUA_REFNIL;
    }

    console_log("%s", filepath.cstr());

    String contents;
    bool ok = vfs_read_entire_file(&contents, filepath);

    // 如果是读取绝对路径
    if (!ok) ok = read_entire_file_raw(&contents, filepath);

    if (!ok) {
        StringBuilder sb = {};
        neko_defer(sb.trash());
        fatal_error(String(sb << "failed to read script: " << filepath));
        return LUA_REFNIL;
    }
    neko_defer(mem_free(contents.data));

    lua_newtable(L);
    i32 module_table = lua_gettop(L);

    {
        PROFILE_BLOCK("load lua script");

        if (luaL_loadbuffer(L, contents.data, contents.len, filepath.data) != LUA_OK) {
            String error_message = luax_check_string(L, -1);
            lua_pop(L, 1);  // 弹出错误消息
            StringBuilder sb = {};
            neko_defer(sb.trash());
            fatal_error(String(sb << "Error loading script: " << filepath << "\n" << error_message));
            return LUA_REFNIL;
        }
    }

    // run script
    if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
        String error_message = luax_check_string(L, -1);
        lua_pop(L, 2);  // 弹出错误消息和模块表
        StringBuilder sb = {};
        neko_defer(sb.trash());
        fatal_error(String(sb << "Error running script: " << filepath << "\n" << error_message));
        return LUA_REFNIL;
    }

    // copy return results to module table
    i32 top = lua_gettop(L);
    for (i32 i = 1; i <= top - module_table; i++) {
        lua_seti(L, module_table, i);
    }

    return luaL_ref(L, LUA_REGISTRYINDEX);
}

void luax_stack_dump(lua_State *L) {
    i32 top = lua_gettop(L);
    printf("  --- lua stack (%d) ---\n", top);
    for (i32 i = 1; i <= top; i++) {
        printf("  [%d] (%s): ", i, luaL_typename(L, i));

        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                printf("%f\n", lua_tonumber(L, i));
                break;
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf("%d\n", lua_toboolean(L, i));
                break;
            case LUA_TNIL:
                printf("nil\n");
                break;
            default:
                printf("%p\n", lua_topointer(L, i));
                break;
        }
    }
}

void luax_get(lua_State *L, const_str tb, const_str field) {
    lua_getglobal(L, tb);
    lua_getfield(L, -1, field);
    lua_remove(L, -2);
}

void luax_pcall(lua_State *L, i32 args, i32 results) {
    if (lua_pcall(L, args, results, 1) != LUA_OK) {
        lua_pop(L, 1);
    }
}

void luax_neko_get(lua_State *L, const char *field) {
    lua_getglobal(L, "neko");
    lua_getfield(L, -1, field);
    lua_remove(L, -2);
}

int luax_msgh(lua_State *L) {
    if (g_app->error_mode.load()) {
        return 0;
    }

    // 获取错误消息
    String err = luax_check_string(L, -1);
    if (!err.len) {
        err = "Unknown error";
    }

    // 获取堆栈跟踪
    luaL_traceback(L, L, NULL, 1);

    // 打印堆栈跟踪
    String traceback = luax_check_string(L, -1);

    if (LockGuard lock{&g_app->error_mtx}) {
        g_app->fatal_error = to_cstr(err);
        g_app->traceback = to_cstr(traceback);

        fprintf(stderr, "================ Lua Error ================\n%s\n%s\n", g_app->fatal_error.data, g_app->traceback.data);

        for (u64 i = 0; i < g_app->traceback.len; i++) {
            if (g_app->traceback.data[i] == '\t') {
                g_app->traceback.data[i] = ' ';
            }
        }

        g_app->error_mode.store(true);
    }

    // 返回带有堆栈跟踪的错误消息
    return 1;
}

lua_Integer luax_len(lua_State *L, i32 arg) {
    lua_len(L, arg);
    lua_Integer len = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return len;
}

void luax_geti(lua_State *L, i32 arg, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_gettable(L, arg);
}

void luax_set_number_field(lua_State *L, const char *key, lua_Number n) {
    lua_pushnumber(L, n);
    lua_setfield(L, -2, key);
}

void luax_set_int_field(lua_State *L, const char *key, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_setfield(L, -2, key);
}

void luax_set_string_field(lua_State *L, const char *key, const char *str) {
    lua_pushstring(L, str);
    lua_setfield(L, -2, key);
}

lua_Number luax_number_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Number num = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optnumber(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Integer num = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_int_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optinteger(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

String luax_string_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, -1, &len);
    lua_pop(L, 1);
    return {str, len};
}

String luax_opt_string_field(lua_State *L, i32 arg, const char *key, const char *fallback) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_optlstring(L, -1, fallback, &len);
    lua_pop(L, 1);
    return {str, len};
}

bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback) {
    i32 type = lua_getfield(L, arg, key);

    bool b = fallback;
    if (type != LUA_TNIL) {
        b = lua_toboolean(L, -1);
    }

    lua_pop(L, 1);
    return b;
}

String luax_check_string(lua_State *L, i32 arg) {
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, arg, &len);
    return {str, len};
}

String luax_opt_string(lua_State *L, i32 arg, String def) { return lua_isstring(L, arg) ? luax_check_string(L, arg) : def; }

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack, String needle) {
    StringBuilder sb = {};
    neko_defer(sb.trash());

    sb << "expected one of: {";
    for (String s : haystack) {
        sb << "\"" << s << "\", ";
    }
    if (haystack.size() != 0) {
        sb.len -= 2;
    }
    sb << "} got: \"" << needle << "\".";

    return luaL_error(L, "%s", sb.data);
}

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l) {
    luaL_newmetatable(L, mt_name);
    luaL_setfuncs(L, l, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

static void lua_thread_proc(void *udata) {
    PROFILE_FUNC();

    LuaThread *lt = (LuaThread *)udata;

    // lua_State *L = lua_newstate(luaalloc, LA);
    // neko_defer(lua_close(L));

    neko::neko_luastate LS = neko::neko_lua_create();
    neko_defer(neko::neko_lua_fini(LS));

    auto L = LS.L;

    {
        PROFILE_BLOCK("open libs");
        luaL_openlibs(L);
    }

    {
        PROFILE_BLOCK("open api");
        open_neko_api(L);
    }

    {
        PROFILE_BLOCK("run bootstrap");
        luax_run_bootstrap(L);
    }

    String contents = lt->contents;

    {
        PROFILE_BLOCK("load chunk");
        if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) != LUA_OK) {
            String err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);

            mem_free(contents.data);
            mem_free(lt->name.data);
            return;
        }
    }

    mem_free(contents.data);
    mem_free(lt->name.data);

    {
        PROFILE_BLOCK("run chunk");
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            String err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);
        }
    }
}

void LuaThread::make(String code, String thread_name) {
    mtx.make();
    contents = to_cstr(code);
    name = to_cstr(thread_name);

    LockGuard lock{&mtx};
    thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
    if (LockGuard lock{&mtx}) {
        thread.join();
    }

    mtx.trash();
}

//

void LuaVariant::make(lua_State *L, i32 arg) {
    type = lua_type(L, arg);

    switch (type) {
        case LUA_TBOOLEAN:
            boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            number = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            String s = luax_check_string(L, arg);
            string = to_cstr(s);
            break;
        }
        case LUA_TTABLE: {
            Array<LuaTableEntry> entries = {};
            entries.resize(luax_len(L, arg));

            lua_pushvalue(L, arg);
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                LuaVariant key = {};
                key.make(L, -2);

                LuaVariant value = {};
                value.make(L, -1);

                entries.push({key, value});
            }
            lua_pop(L, 1);

            table = Slice(entries);
            break;
        }
        case LUA_TUSERDATA: {
            i32 kind = lua_getiuservalue(L, arg, LUAX_UD_TNAME);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TSTRING) {
                return;
            }

            kind = lua_getiuservalue(L, arg, LUAX_UD_PTR_SIZE);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TNUMBER) {
                return;
            }

            String tname = luax_check_string(L, -2);
            u64 size = luaL_checkinteger(L, -1);

            if (size != sizeof(void *)) {
                return;
            }

            udata.ptr = *(void **)lua_touserdata(L, arg);
            udata.tname = to_cstr(tname);

            break;
        }
        default:
            break;
    }
}

void LuaVariant::trash() {
    switch (type) {
        case LUA_TSTRING: {
            mem_free(string.data);
            break;
        }
        case LUA_TTABLE: {
            for (LuaTableEntry e : table) {
                e.key.trash();
                e.value.trash();
            }
            mem_free(table.data);
        }
        case LUA_TUSERDATA: {
            mem_free(udata.tname.data);
        }
        default:
            break;
    }
}

void LuaVariant::push(lua_State *L) {
    switch (type) {
        case LUA_TBOOLEAN:
            lua_pushboolean(L, boolean);
            break;
        case LUA_TNUMBER:
            lua_pushnumber(L, number);
            break;
        case LUA_TSTRING:
            lua_pushlstring(L, string.data, string.len);
            break;
        case LUA_TTABLE: {
            lua_newtable(L);
            for (LuaTableEntry e : table) {
                e.key.push(L);
                e.value.push(L);
                lua_rawset(L, -3);
            }
            break;
        }
        case LUA_TUSERDATA: {
            luax_ptr_userdata(L, udata.ptr, udata.tname.data);
            break;
        }
        default:
            break;
    }
}

//

struct LuaChannels {
    Mutex mtx;
    Cond select;
    HashMap<LuaChannel *> by_name;
};

static LuaChannels g_channels = {};

void LuaChannel::make(String n, u64 buf) {
    mtx.make();
    sent.make();
    received.make();
    items.data = (LuaVariant *)mem_alloc(sizeof(LuaVariant) * (buf + 1));
    items.len = (buf + 1);
    front = 0;
    back = 0;
    len = 0;

    name.store(to_cstr(n).data);
}

void LuaChannel::trash() {
    for (i32 i = 0; i < len; i++) {
        items[front].trash();
        front = (front + 1) % items.len;
    }

    mem_free(items.data);
    mem_free(name.exchange(nullptr));
    mtx.trash();
    sent.trash();
    received.trash();
}

void LuaChannel::send(LuaVariant item) {
    LockGuard lock{&mtx};

    while (len == items.len) {
        received.wait(&mtx);
    }

    items[back] = item;
    back = (back + 1) % items.len;
    len++;

    g_channels.select.broadcast();
    sent.signal();
    sent_total++;

    while (sent_total >= received_total + items.len) {
        received.wait(&mtx);
    }
}

static LuaVariant lua_channel_dequeue(LuaChannel *ch) {
    LuaVariant item = ch->items[ch->front];
    ch->front = (ch->front + 1) % ch->items.len;
    ch->len--;

    ch->received.broadcast();
    ch->received_total++;

    return item;
}

LuaVariant LuaChannel::recv() {
    LockGuard lock{&mtx};

    while (len == 0) {
        sent.wait(&mtx);
    }

    return lua_channel_dequeue(this);
}

bool LuaChannel::try_recv(LuaVariant *v) {
    LockGuard lock{&mtx};

    if (len == 0) {
        return false;
    }

    *v = lua_channel_dequeue(this);
    return true;
}

LuaChannel *lua_channel_make(String name, u64 buf) {
    LuaChannel *chan = (LuaChannel *)mem_alloc(sizeof(LuaChannel));
    new (&chan->name) std::atomic<char *>();
    chan->make(name, buf);

    LockGuard lock{&g_channels.mtx};
    g_channels.by_name[fnv1a(name)] = chan;

    return chan;
}

LuaChannel *lua_channel_get(String name) {
    LockGuard lock{&g_channels.mtx};

    LuaChannel **chan = g_channels.by_name.get(fnv1a(name));
    if (chan == nullptr) {
        return nullptr;
    }

    return *chan;
}

LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v) {
    i32 len = lua_gettop(L);
    if (len == 0) {
        return nullptr;
    }

    LuaChannel *buf[16] = {};
    for (i32 i = 0; i < len; i++) {
        buf[i] = *(LuaChannel **)luaL_checkudata(L, i + 1, "mt_channel");
    }

    Mutex mtx = {};
    mtx.make();
    LockGuard lock{&mtx};

    while (true) {
        for (i32 i = 0; i < len; i++) {
            LockGuard lock{&buf[i]->mtx};
            if (buf[i]->len > 0) {
                *v = lua_channel_dequeue(buf[i]);
                return buf[i];
            }
        }

        g_channels.select.wait(&mtx);
    }
}

void lua_channels_setup() {
    g_channels.select.make();
    g_channels.mtx.make();
}

void lua_channels_shutdown() {
    for (auto [k, v] : g_channels.by_name) {
        LuaChannel *chan = *v;
        chan->trash();
        mem_free(chan);
    }
    g_channels.by_name.trash();
    g_channels.select.trash();
    g_channels.mtx.trash();
}

#define FREELIST 1

void neko_luaref::make(lua_State *L) {
    this->refL = lua_newthread(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, refL);
    lua_newtable(refL);
}

void neko_luaref::fini() {
    lua_State *L = (lua_State *)refL;
    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, refL);
}

bool neko_luaref::isvalid(int ref) {
    if (ref <= FREELIST || ref > lua_gettop(refL)) {
        return false;
    }
    lua_pushnil(refL);
    while (lua_next(refL, FREELIST)) {
        lua_pop(refL, 1);
        if (lua_tointeger(refL, -1) == ref) {
            lua_pop(refL, 1);
            return false;
        }
    }
    return true;
}

int neko_luaref::ref(lua_State *L) {
    if (!lua_checkstack(refL, 3)) {
        return LUA_NOREF;
    }
    lua_xmove(L, refL, 1);
    lua_pushnil(refL);
    if (!lua_next(refL, FREELIST)) {
        return lua_gettop(refL);
    }
    int r = (int)lua_tointeger(refL, -2);
    lua_pop(refL, 1);
    lua_pushnil(refL);
    lua_rawset(refL, FREELIST);
    lua_replace(refL, r);
    return r;
}

void neko_luaref::unref(int ref) {
    if (ref <= FREELIST) {
        return;
    }
    int top = lua_gettop(refL);
    if (ref > top) {
        return;
    }
    if (ref < top) {
        lua_pushboolean(refL, 1);
        lua_rawseti(refL, FREELIST, ref);
        lua_pushnil(refL);
        lua_replace(refL, ref);
        return;
    }
    for (--top; top > FREELIST; --top) {
        if (LUA_TNIL == lua_rawgeti(refL, FREELIST, top)) {
            lua_pop(refL, 1);
            break;
        }
        lua_pop(refL, 1);
        lua_pushnil(refL);
        lua_rawseti(refL, FREELIST, top);
    }
    lua_settop(refL, top);
}

void neko_luaref::get(lua_State *L, int ref) {
    assert(isvalid(ref));
    lua_pushvalue(refL, ref);
    lua_xmove(refL, L, 1);
}

void neko_luaref::set(lua_State *L, int ref) {
    assert(isvalid(ref));
    lua_xmove(L, refL, 1);
    lua_replace(refL, ref);
}

int __neko_bind_callback_save(lua_State *L) {
    const_str identifier = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (g_app->g_lua_callbacks_table_ref == LUA_NOREF) {
        lua_newtable(L);
        g_app->g_lua_callbacks_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_app->g_lua_callbacks_table_ref);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, identifier);
    lua_pop(L, 1);
    lua_pop(L, 2);
    return 0;
}

int __neko_bind_callback_call(lua_State *L) {
    if (g_app->g_lua_callbacks_table_ref != LUA_NOREF) {
        const_str identifier = luaL_checkstring(L, 1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_app->g_lua_callbacks_table_ref);
        lua_getfield(L, -1, identifier);
        if (lua_isfunction(L, -1)) {
            int nargs = lua_gettop(L) - 1;  // 获取参数数量 减去标识符参数
            for (int i = 2; i <= nargs + 1; ++i) {
                lua_pushvalue(L, i);
            }
            lua_call(L, nargs, 0);  // 调用
        } else {
            console_log("callback with identifier '%s' not found or is not a function", identifier);
        }
        lua_pop(L, 1);
    } else {
        console_log("callback table is noref");
    }
    return 0;
}

// 返回一些有助于识别对象的名称
[[nodiscard]] static int DiscoverObjectNameRecur(lua_State *L, int shortest_, int depth_) {
    static constexpr int kWhat{1};
    static constexpr int kResult{2};
    static constexpr int kCache{3};
    static constexpr int kFQN{4};

    if (shortest_ <= depth_ + 1) {
        return shortest_;
    }
    neko_assert(lua_checkstack(L, 3));

    lua_pushvalue(L, -1);
    lua_rawget(L, kCache);

    if (!lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return shortest_;
    }

    lua_pop(L, 1);
    lua_pushvalue(L, -1);
    lua_pushinteger(L, 1);
    lua_rawset(L, kCache);

    lua_pushnil(L);
    while (lua_next(L, -2)) {

        ++depth_;
        lua_pushvalue(L, -2);
        lua_rawseti(L, kFQN, depth_);
        if (lua_rawequal(L, -1, kWhat)) {

            if (depth_ < shortest_) {
                shortest_ = depth_;
                std::ignore = neko::lua_tool::PushFQN(L, kFQN, depth_);
                lua_replace(L, kResult);
            }

            lua_pop(L, 2);
            break;
        }
        switch (lua_type(L, -1)) {
            default:
                break;

            case LUA_TTABLE:
                shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);

                if (lua_getmetatable(L, -1)) {
                    if (lua_istable(L, -1)) {
                        ++depth_;
                        lua_pushstring(L, "__metatable");
                        lua_rawseti(L, kFQN, depth_);
                        shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);
                        lua_pushnil(L);
                        lua_rawseti(L, kFQN, depth_);
                        --depth_;
                    }
                    lua_pop(L, 1);
                }
                break;

            case LUA_TTHREAD:

                break;

            case LUA_TUSERDATA:

                if (lua_getmetatable(L, -1)) {
                    if (lua_istable(L, -1)) {
                        ++depth_;
                        lua_pushstring(L, "__metatable");
                        lua_rawseti(L, kFQN, depth_);
                        shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);
                        lua_pushnil(L);
                        lua_rawseti(L, kFQN, depth_);
                        --depth_;
                    }
                    lua_pop(L, 1);
                }

                {
                    int _uvi{1};
                    while (lua_getiuservalue(L, -1, _uvi) != LUA_TNONE) {
                        if (lua_istable(L, -1)) {
                            ++depth_;
                            lua_pushstring(L, "uservalue");
                            lua_rawseti(L, kFQN, depth_);
                            shortest_ = DiscoverObjectNameRecur(L, shortest_, depth_);
                            lua_pushnil(L);
                            lua_rawseti(L, kFQN, depth_);
                            --depth_;
                        }
                        lua_pop(L, 1);
                        ++_uvi;
                    }

                    lua_pop(L, 1);
                }
                break;
        }

        lua_pop(L, 1);

        lua_pushnil(L);
        lua_rawseti(L, kFQN, depth_);
        --depth_;
    }

    lua_pushvalue(L, -1);
    lua_pushnil(L);
    lua_rawset(L, kCache);
    return shortest_;
}

int __neko_bind_nameof(lua_State *L) {
    int const _what{lua_gettop(L)};
    if (_what > 1) {
        luaL_argerror(L, _what, "too many arguments.");
    }

    if (lua_type(L, 1) < LUA_TTABLE) {
        lua_pushstring(L, luaL_typename(L, 1));
        lua_insert(L, -2);
        return 2;
    }

    lua_pushnil(L);

    lua_newtable(L);  // 所有已访问表的缓存

    lua_newtable(L);  // 其内容是字符串 连接时会产生唯一的名称
    lua_pushstring(L, LUA_GNAME);
    lua_rawseti(L, -2, 1);

    lua_pushglobaltable(L);  // 开始搜索
    std::ignore = DiscoverObjectNameRecur(L, std::numeric_limits<int>::max(), 1);
    if (lua_isnil(L, 2)) {
        lua_pop(L, 1);
        lua_pushstring(L, "_R");
        lua_rawseti(L, -2, 1);
        lua_pushvalue(L, LUA_REGISTRYINDEX);
        std::ignore = DiscoverObjectNameRecur(L, std::numeric_limits<int>::max(), 1);
    }
    lua_pop(L, 3);
    lua_pushstring(L, luaL_typename(L, 1));
    lua_replace(L, -3);
    return 2;
}

#define CUSTOM_TYPES
#ifdef CUSTOM_TYPES

std::unordered_map<std::type_index, std::string> usertypeNames;

int userdata_destructor(lua_State *L) {
    if (auto obj = touserdata<Userdata>(L, 1)) {
        obj->~Userdata();
    }
    return 0;
}

Bytearray::Bytearray(size_t capacity) : buffer(capacity) { buffer.resize(capacity); }

Bytearray::Bytearray(std::vector<u8> buffer) : buffer(std::move(buffer)) {}

Bytearray::~Bytearray() {}

static int l_bytearray_append(lua_State *L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        auto value = lua_tointeger(L, 2);
        buffer->data().push_back(static_cast<u8>(value));
    }
    return 0;
}

static int l_bytearray_insert(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    auto value = lua_tointeger(L, 3);
    data.insert(data.begin() + index, static_cast<u8>(value));
    return 0;
}

static int l_bytearray_remove(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    data.erase(data.begin() + index);
    return 0;
}

static std::unordered_map<std::string, lua_CFunction> bytearray_methods{
        {"append", lua::wrap<l_bytearray_append>},
        {"insert", lua::wrap<l_bytearray_insert>},
        {"remove", lua::wrap<l_bytearray_remove>},
};

static int l_bytearray_meta_meta_call(lua_State *L) {
    if (lua_istable(L, 2)) {
        size_t len = lua_objlen(L, 2);
        std::vector<u8> buffer(len);
        buffer.resize(len);
        for (size_t i = 0; i < len; i++) {
            lua_rawgeti(L, -1, i + 1);
            buffer[i] = static_cast<u8>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        return newuserdata<Bytearray>(L, std::move(buffer));
    }
    auto size = lua_tointeger(L, 2);
    if (size < 0) {
        throw std::runtime_error("size can not be less than 0");
    }
    return newuserdata<Bytearray>(L, static_cast<size_t>(size));
}

static int l_bytearray_meta_index(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
    if (lua_isstring(L, 2)) {
        auto found = bytearray_methods.find(lua_tostring(L, 2));
        if (found != bytearray_methods.end()) {
            lua_pushcfunction(L, found->second);
            return 1;
        }
    }
    auto index = lua_tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    lua_pushinteger(L, data[index]);
    return 1;
}

static int l_bytearray_meta_newindex(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
    auto index = static_cast<size_t>(lua_tointeger(L, 2) - 1);
    auto value = lua_tointeger(L, 3);
    if (index >= data.size()) {
        if (index == data.size()) {
            data.push_back(static_cast<u8>(value));
        }
        return 0;
    }
    data[index] = static_cast<u8>(value);
    return 0;
}

static int l_bytearray_meta_len(lua_State *L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        lua_pushinteger(L, buffer->data().size());
        return 1;
    }
    return 0;
}

static int l_bytearray_meta_tostring(lua_State *L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto &data = buffer->data();
    if (data.size() > 512) {
        lua_pushstring(L, std::string("bytearray[" + std::to_string(data.size()) + "]{...}").c_str());
        return 1;
    } else {
        std::stringstream ss;
        ss << "bytearray[" << std::to_string(data.size()) << "]{";
        for (size_t i = 0; i < data.size(); i++) {
            if (i > 0) {
                ss << " ";
            }
            ss << static_cast<unsigned int>(data[i]);
        }
        ss << "}";
        lua_pushstring(L, ss.str().c_str());
        return 1;
    }
}

static int l_bytearray_meta_add(lua_State *L) {
    auto bufferA = touserdata<Bytearray>(L, 1);
    auto bufferB = touserdata<Bytearray>(L, 2);
    if (bufferA == nullptr || bufferB == nullptr) {
        return 0;
    }
    auto &dataA = bufferA->data();
    auto &dataB = bufferB->data();

    std::vector<u8> ab;
    ab.reserve(dataA.size() + dataB.size());
    ab.insert(ab.end(), dataA.begin(), dataA.end());
    ab.insert(ab.end(), dataB.begin(), dataB.end());
    return newuserdata<Bytearray>(L, std::move(ab));
}

int Bytearray::createMetatable(lua_State *L) {
    lua_createtable(L, 0, 6);
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_index>);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_newindex>);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_len>);
    lua_setfield(L, -2, "__len");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_tostring>);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_add>);
    lua_setfield(L, -2, "__add");

    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, lua::wrap<l_bytearray_meta_meta_call>);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    return 1;
}

void LuaVector::RegisterMetaTable(lua_State *L) {
    if (luaL_newmetatable(L, LUA_TYPE_NAME)) {
        lua_pushcfunction(L, New);
        lua_setfield(L, 1, "new");

        lua_pushcfunction(L, GarbageCollect);
        lua_setfield(L, 1, "__gc");

        lua_pushcfunction(L, ToString);
        lua_setfield(L, 1, "__tostring");

        lua_pushcfunction(L, Index);
        lua_setfield(L, 1, "__index");

        lua_pushcfunction(L, NewIndex);
        lua_setfield(L, 1, "__newindex");

        lua_pushcfunction(L, Len);
        lua_setfield(L, 1, "__len");

        lua_pushcfunction(L, Append);
        lua_setfield(L, 1, "append");

        lua_pushcfunction(L, Pop);
        lua_setfield(L, 1, "pop");

        lua_pushcfunction(L, Extend);
        lua_setfield(L, 1, "extend");

        lua_pushcfunction(L, Insert);
        lua_setfield(L, 1, "insert");

        lua_pushcfunction(L, Erase);
        lua_setfield(L, 1, "erase");

        lua_pushcfunction(L, Sort);
        lua_setfield(L, 1, "sort");

        lua_setglobal(L, LUA_TYPE_NAME);
    }
}

LuaVector *LuaVector::CheckArg(lua_State *L, int arg) { return static_cast<LuaVector *>(luaL_checkudata(L, arg, LUA_TYPE_NAME)); }

int LuaVector::New(lua_State *L) {
    auto p = static_cast<LuaVector *>(lua_newuserdata(L, sizeof(LuaVector)));
    new (p) LuaVector;
    luaL_setmetatable(L, LUA_TYPE_NAME);
    return 1;
}

int LuaVector::GarbageCollect(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (p) {
        p->~LuaVector();
    }
    return 0;
}

int LuaVector::ToString(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    std::string str = "vector{";
    for (int i = 0; i < p->size(); i++) {
        if (i != 0) {
            str += ", ";
        }
        str += std::to_string(p->at(i));
    }
    str += "}";

    lua_pushstring(L, str.c_str());
    return 1;
}

int LuaVector::Index(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    switch (lua_type(L, 2)) {
        case LUA_TNUMBER: {
            lua_Integer idx = lua_tointeger(L, 2) - 1;  // convert lua indices to c indices
            if (0 <= idx && idx < p->size()) {
                lua_pushnumber(L, p->at(idx));
                return 1;
            }
            lua_pushnil(L);
            return 1;
        }
        case LUA_TSTRING: {
            const char *meta = lua_tostring(L, 2);
            luaL_getmetafield(L, 1, meta);
            return 1;
        }
        default: {
            lua_pushnil(L);
            return 1;
        }
    }
}

int LuaVector::NewIndex(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Integer idx = luaL_checkinteger(L, 2) - 1;  // convert lua indices to c indices
    lua_Number value = luaL_checknumber(L, 3);

    if (idx < 0) {
        return 0;
    }

    if (idx >= p->size()) {
        p->resize(idx + 1);
    }

    p->at(idx) = static_cast<float>(value);
    return 0;
}

int LuaVector::Len(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_pushinteger(L, static_cast<lua_Integer>(p->size()));
    return 1;
}

int LuaVector::Append(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Number value = luaL_checknumber(L, 2);
    p->push_back(static_cast<float>(value));

    return 0;
}

int LuaVector::Pop(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    if (p->empty()) {
        lua_pushnil(L);
    } else {
        float back = p->back();
        p->pop_back();
        lua_pushnumber(L, back);
    }

    return 1;
}

int LuaVector::Extend(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    const int numParams = lua_gettop(L);
    p->reserve(p->size() + numParams - 1);
    for (int i = 2; i <= numParams; i++) {
        lua_Number value = luaL_checknumber(L, i);
        p->push_back(static_cast<float>(value));
    }

    return 0;
}

int LuaVector::Insert(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Integer idx = luaL_checkinteger(L, 2) - 1;  // convert lua indices to c indices
    lua_Number value = luaL_checknumber(L, 3);

    p->insert(std::next(p->begin(), idx), static_cast<float>(value));

    return 0;
}

int LuaVector::Erase(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    lua_Integer idx = luaL_checkinteger(L, 2) - 1;  // convert lua indices to c indices

    p->erase(std::next(p->begin(), idx));

    return 0;
}

int LuaVector::Sort(lua_State *L) {
    auto p = CheckArg(L, 1);
    if (!p) {
        return 0;
    }

    std::sort(p->begin(), p->end());

    return 0;
}

int Test(lua_State *L) {
    LuaVector *v = LuaVector::CheckArg(L, 1);
    v->push_back(1);
    return 0;
}

int test_luavector() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    LuaVector::RegisterMetaTable(L);

    lua_register(L, "test", Test);

    const char *TEST_CODE = R"LUA(
local a = vector.new()
print(a)
print(type(a))
print(a[1])
a[1] = 3.14
print(a)
print(a[1], a[2])
print(#a)
a[2] = 6.28
print(a)
print(a[1], a[2])
print(#a)
for i, v in ipairs(a) do
    print(i, v)
end
a:append(9.42)
print(a)
a:extend(1, 2, 3)
print(a)
a:insert(2, 0.1234)
print(a)
a:sort()
print(a)
a:erase(2)
print(a)
while #a > 0 do
    local b = a:pop()
    print(a, b)
end
test(a)
print(a)
)LUA";

    if (luaL_dostring(L, TEST_CODE) != LUA_OK) {
        fprintf(stderr, "lua error: %s\n", lua_tostring(L, -1));
    }

    lua_close(L);
    return 0;
}

#define entityCacheSize sizeof(EntityCacheEntry)
typedef struct EntityCacheEntry {
    int entityId;
    int nuid;
    const char *ownerGuid;
    const char *ownerName;
    const char *filepath;
    double x;
    double y;
    double rotation;
    double velX;
    double velY;
    double currentHealth;
    double maxHealth;
} EntityCacheEntry;

EntityCacheEntry *entityEntries;
int entityCurrentSize = 0;
static int l_entityCacheSize(lua_State *L) {
    lua_pushinteger(L, entityCurrentSize);
    return 1;
}
static int l_entityCacheUsage(lua_State *L) {
    lua_pushnumber(L, entityCurrentSize * entityCacheSize);
    return 1;
}
static int l_entityCacheWrite(lua_State *L) {
    int entityId = luaL_checkinteger(L, 1);
    int nuid = lua_tointeger(L, 2);
    if (nuid == NULL) {
        nuid = -1;
    };
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->entityId == entityId) {
            entry->entityId = entityId;
            entry->nuid = nuid;
            entry->ownerGuid = luaL_checkstring(L, 3);
            entry->ownerName = luaL_checkstring(L, 4);
            entry->filepath = luaL_checkstring(L, 5);
            entry->x = luaL_checknumber(L, 6);
            entry->y = luaL_checknumber(L, 7);
            entry->rotation = luaL_checknumber(L, 8);
            entry->velX = luaL_checknumber(L, 9);
            entry->velY = luaL_checknumber(L, 10);
            entry->currentHealth = luaL_checknumber(L, 11);
            entry->maxHealth = luaL_checknumber(L, 12);
            return 0;
        };
    }
    ++entityCurrentSize;
    entityEntries = (EntityCacheEntry *)mem_realloc(entityEntries, entityCacheSize * entityCurrentSize);
    EntityCacheEntry *newEntry = entityEntries + (entityCurrentSize - 1);
    newEntry->entityId = entityId;
    newEntry->nuid = nuid;
    newEntry->ownerGuid = luaL_checkstring(L, 3);
    newEntry->ownerName = luaL_checkstring(L, 4);
    newEntry->filepath = luaL_checkstring(L, 5);
    newEntry->x = luaL_checknumber(L, 6);
    newEntry->y = luaL_checknumber(L, 7);
    newEntry->rotation = luaL_checknumber(L, 8);
    newEntry->velX = luaL_checknumber(L, 9);
    newEntry->velY = luaL_checknumber(L, 10);
    newEntry->currentHealth = luaL_checknumber(L, 11);
    newEntry->maxHealth = luaL_checknumber(L, 12);
    return 0;
}

static void l_createEntityCacheReturnTable(lua_State *L, EntityCacheEntry *entry) {
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, entry->entityId);
    lua_setfield(L, -2, "entityId");
    if (entry->nuid == -1) {
        lua_pushnil(L);
        lua_setfield(L, -2, "nuid");
    } else {
        lua_pushinteger(L, entry->nuid);
        lua_setfield(L, -2, "nuid");
    }
    lua_pushstring(L, entry->ownerGuid);
    lua_setfield(L, -2, "ownerGuid");

    lua_pushstring(L, entry->ownerName);
    lua_setfield(L, -2, "ownerName");

    lua_pushstring(L, entry->filepath);
    lua_setfield(L, -2, "filepath");

    lua_pushnumber(L, entry->x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, entry->y);
    lua_setfield(L, -2, "y");

    lua_pushnumber(L, entry->velX);
    lua_setfield(L, -2, "velX");

    lua_pushnumber(L, entry->velY);
    lua_setfield(L, -2, "velY");

    lua_pushnumber(L, entry->rotation);
    lua_setfield(L, -2, "rotation");

    lua_pushnumber(L, entry->currentHealth);
    lua_setfield(L, -2, "currentHealth");

    lua_pushnumber(L, entry->maxHealth);
    lua_setfield(L, -2, "maxHealth");
}
static int l_entityCacheReadByEntityId(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->entityId == idToSearch) {
            l_createEntityCacheReturnTable(L, entry);
            return 1;
        };
    }
    lua_pushnil(L);
    return 1;
}

static int l_entityCacheReadByNuid(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->nuid == idToSearch) {
            l_createEntityCacheReturnTable(L, entry);
            return 1;
        };
    }
    lua_pushnil(L);
    return 1;
}

static int l_entityCacheDeleteByEntityId(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->entityId == idToSearch) {
            memmove(entityEntries + i + 1, entityEntries + i, ((entityCurrentSize - 1) - i) * entityCacheSize);
            entityCurrentSize--;
            entityEntries = (EntityCacheEntry *)mem_realloc(entityEntries, entityCacheSize * entityCurrentSize);
            lua_pushboolean(L, 1);
            return 1;
        };
    }
    lua_pushboolean(L, 0);
    return 1;
}

static int l_entityCacheDeleteByNuid(lua_State *L) {
    int idToSearch = luaL_checkinteger(L, 1);
    for (int i = 0; i < entityCurrentSize; i++) {
        EntityCacheEntry *entry = entityEntries + i;
        if (entry->nuid == idToSearch) {
            memmove(entityEntries + i + 1, entityEntries + i, ((entityCurrentSize - 1) - i) * entityCacheSize);
            entityCurrentSize--;
            entityEntries = (EntityCacheEntry *)mem_realloc(entityEntries, entityCacheSize * entityCurrentSize);
            lua_pushboolean(L, 1);
            return 1;
        };
    }
    lua_pushboolean(L, 0);
    return 1;
}

int luaopen_luaExtensions(lua_State *L) {
    static const luaL_Reg eCachelib[] = {{"set", l_entityCacheWrite},
                                         {"get", l_entityCacheReadByEntityId},
                                         {"getNuid", l_entityCacheReadByNuid},
                                         {"delete", l_entityCacheDeleteByEntityId},
                                         {"deleteNuid", l_entityCacheDeleteByNuid},
                                         {"size", l_entityCacheSize},
                                         {"usage", l_entityCacheUsage},
                                         {NULL, NULL}};
    // luaL_openlib(L, "EntityCache", eCachelib, 0);
    return 1;
}

#endif

namespace neko::lua::__luadb {

#define MAX_DEPTH 256
#define SHORT_STRING 1024
#define CONVERTER 2
#define REF_CACHE 3
#define REF_UNSOLVED 4
#define TAB_SPACE 4

#define UV_PROXY 1
#define UV_WEAK 2

#define NEKO_DATALUA_REGISTER "__neko_luadb"

typedef struct luadb {
    lua_State *L;
    const_str key;
} luadb;

static void luadb_get(lua_State *L, lua_State *Ldb, int index) {
    // 从 Ldb 复制表索引 并将 proxy 推入 L

    // luaL_checktype(Ldb, -1, LUA_TTABLE);
    const_str key = reinterpret_cast<const_str>(lua_topointer(Ldb, index));
    if (key == NULL) {
        luaL_error(L, "Not a table");
    }

    if (lua_rawgetp(Ldb, LUA_REGISTRYINDEX, key) == LUA_TNIL) {
        lua_pop(Ldb, 1);
        lua_pushvalue(Ldb, index);  // 将表放入 Ldb 的注册表中
        lua_rawsetp(Ldb, LUA_REGISTRYINDEX, key);
    } else {
        lua_pop(Ldb, 1);  // pop table
    }
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, Ldb) != LUA_TTABLE) {
        lua_pop(L, 1);
        luaL_error(L, "Not an invalid L %p", Ldb);
    }
    if (lua_rawgetp(L, -1, key) == LUA_TNIL) {
        lua_pop(L, 1);
        luadb *t = (luadb *)lua_newuserdata(L, sizeof(*t));
        // auto t = neko::lua::newudata<luadb>(L);
        t->L = Ldb;
        t->key = key;
        lua_pushvalue(L, lua_upvalueindex(UV_PROXY));  // 代理的元表

        lua_setmetatable(L, -2);
        lua_pushvalue(L, -1);
        lua_rawsetp(L, -3, key);  // 缓存
    }
    lua_replace(L, -2);  // 删除代理缓存表
}

// kv推入luadb
static luadb *luadb_pretable(lua_State *L) {
    luadb *t = (luadb *)lua_touserdata(L, 1);
    // auto t = neko::lua::toudata<luadb>(L, 1);
    if (t->L == NULL) {
        luaL_error(L, "invalid proxy object");
    }
    if (lua_rawgetp(t->L, LUA_REGISTRYINDEX, t->key) != LUA_TTABLE) {
        lua_pop(t->L, 1);
        luaL_error(L, "invalid external table %p of L(%p)", t->key, t->L);
    }
    switch (lua_type(L, 2)) {
        case LUA_TNONE:
        case LUA_TNIL:
            lua_pushnil(t->L);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, 2)) {
                lua_pushinteger(t->L, lua_tointeger(L, 2));
            } else {
                lua_pushnumber(t->L, lua_tonumber(L, 2));
            }
            break;
        case LUA_TSTRING:
            lua_pushstring(t->L, lua_tostring(L, 2));
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(t->L, lua_toboolean(L, 2));
            break;
        default:
            lua_pop(t->L, 1);
            luaL_error(L, "Unsupport key type %s", lua_typename(L, lua_type(L, 2)));
    }
    return t;
}

static int luadb_copyvalue(lua_State *fromL, lua_State *toL, int index) {
    int t = lua_type(fromL, index);
    switch (t) {
        case LUA_TNIL:
            lua_pushnil(toL);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(fromL, index)) {
                lua_pushinteger(toL, lua_tointeger(fromL, index));
            } else {
                lua_pushnumber(toL, lua_tonumber(fromL, index));
            }
            break;
        case LUA_TSTRING:
            lua_pushstring(toL, lua_tostring(fromL, index));
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(toL, lua_toboolean(fromL, index));
            break;
        case LUA_TTABLE:
            luadb_get(toL, fromL, index);
            break;
        default:
            lua_pushfstring(toL, "Unsupport value type (%s)", lua_typename(fromL, lua_type(fromL, index)));
            return 1;
    }
    return 0;
}

int luaopen(lua_State *L) {
    luaL_checkversion(L);
    lua_newtable(L);

    int modindex = lua_gettop(L);

    lua_createtable(L, 0, 1);  // 创建代理元表 (upvalue 1:UV_PROXY)

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                struct luadb *t = luadb_pretable(L);
                lua_rawget(t->L, -2);
                if (luadb_copyvalue(t->L, L, -1)) {
                    lua_pop(t->L, 2);
                    return lua_error(L);
                }
                lua_pop(t->L, 2);
                return 1;
            },
            1);
    lua_setfield(L, -2, "__index");

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                lua_settop(L, 1);
                luadb *t = luadb_pretable(L);
                size_t n = lua_rawlen(t->L, -2);
                lua_pushinteger(L, n);
                lua_pop(t->L, 2);
                return 1;
            },
            1);
    lua_setfield(L, -2, "__len");

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                luadb *t = luadb_pretable(L);
                if (lua_next(t->L, -2) == 0) {
                    lua_pop(t->L, 1);
                    return 0;
                }
                if (luadb_copyvalue(t->L, L, -2)) {
                    lua_pop(t->L, 3);
                    return lua_error(L);
                }
                if (luadb_copyvalue(t->L, L, -1)) {
                    lua_pop(t->L, 3);
                    return lua_error(L);
                }
                return 2;
            },
            1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                lua_pushvalue(L, lua_upvalueindex(1));  // proxynext
                lua_pushvalue(L, 1);                    // table
                lua_pushnil(L);
                return 3;
            },
            1);
    lua_setfield(L, -2, "__pairs");

    static auto tostring = [](lua_State *L) {
        luadb *t = (luadb *)lua_touserdata(L, 1);
        // auto t = neko::lua::toudata<luadb>(L, 1);
        lua_pushfstring(L, "[luadb %p:%p]", t->L, t->key);
        return 1;
    };

    lua_pushcfunction(L, tostring);
    lua_setfield(L, -2, "__tostring");

    lua_createtable(L, 0, 1);  // 弱表 (upvalue 2:UV_WEAK)
    lua_pushstring(L, "kv");
    lua_setfield(L, -2, "__mode");

    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                const char *source = luaL_checkstring(L, 1);

                lua_newtable(L);  // 代理缓存表
                lua_pushvalue(L, lua_upvalueindex(UV_WEAK));
                lua_setmetatable(L, -2);

                auto newdb = [](lua_State *L, std::string source) {
                    lua_State *Ldb = luaL_newstate();
                    String contents = {};
                    bool ok = vfs_read_entire_file(&contents, source.c_str());
                    if (ok) {
                        neko_defer(mem_free(contents.data));
                        if (luaL_dostring(Ldb, contents.data)) {
                            // 引发错误
                            lua_pushstring(L, lua_tostring(Ldb, -1));
                            lua_close(Ldb);
                            lua_error(L);
                        }
                    }
                    lua_gc(Ldb, LUA_GCCOLLECT, 0);
                    return Ldb;
                };

                lua_State *Ldb = newdb(L, source);

                lua_rawsetp(L, LUA_REGISTRYINDEX, Ldb);

#if LUA_VERSION_NUM >= 502
                lua_rawgeti(Ldb, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
#else
                lua_pushvalue(L, LUA_GLOBALSINDEX);
#endif
                luadb_get(L, Ldb, -1);
                lua_pop(Ldb, 1);

                // 将 Ldb 记录到 __datalua 中以进行关闭
                lua_getfield(L, LUA_REGISTRYINDEX, NEKO_DATALUA_REGISTER);
                int n = lua_rawlen(L, -1);
                lua_pushlightuserdata(L, Ldb);
                lua_rawseti(L, -2, n + 1);
                lua_pop(L, 1);

                return 1;
            },
            2);
    lua_setfield(L, modindex, "open");

    lua_newtable(L);  // 将所有可扩展键保存到 NEKO_DATALUA_REGISTER 中
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_DATALUA_REGISTER);

    static auto closeall = [](lua_State *L) {
        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_DATALUA_REGISTER);
        int n = lua_rawlen(L, -1);
        int i;
        for (i = 1; i <= n; i++) {
            if (lua_rawgeti(L, -1, i) == LUA_TLIGHTUSERDATA) {
                const void *Ldb = lua_touserdata(L, -1);
                lua_pop(L, 1);
                if (lua_rawgetp(L, LUA_REGISTRYINDEX, Ldb) == LUA_TTABLE) {
                    lua_pop(L, 1);
                    lua_pushnil(L);
                    lua_rawsetp(L, LUA_REGISTRYINDEX, Ldb);  // 清除缓存
                    lua_close((lua_State *)Ldb);
                } else {
                    lua_pop(L, 1);
                }
            }
        }
        return 0;
    };

    lua_createtable(L, 0, 1);  // 关闭虚拟机时设置收集功能
    lua_pushcfunction(L, closeall);
    lua_setfield(L, -2, "__gc");

    lua_setmetatable(L, -2);

    return 1;
}
}  // namespace neko::lua::__luadb