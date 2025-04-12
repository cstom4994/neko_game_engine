#include "luax.h"

#include "base/common/json.hpp"
#include "base/common/profiler.hpp"
#include "engine/scripting/scripting.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "base/cbase.hpp"
#include "base/common/logger.hpp"

using namespace Neko::luabind;

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

int g_lua_callbacks_table_ref = LUA_NOREF;  // LUA_NOREF

int __neko_bind_callback_save(lua_State *L) {
    const_str identifier = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (g_lua_callbacks_table_ref == LUA_NOREF) {
        lua_newtable(L);
        g_lua_callbacks_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_lua_callbacks_table_ref);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, identifier);
    lua_pop(L, 1);
    lua_pop(L, 2);
    return 0;
}

int __neko_bind_callback_call(lua_State *L) {
    if (g_lua_callbacks_table_ref != LUA_NOREF) {
        const_str identifier = luaL_checkstring(L, 1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_lua_callbacks_table_ref);
        lua_getfield(L, -1, identifier);
        if (lua_isfunction(L, -1)) {
            int nargs = lua_gettop(L) - 1;  // 获取参数数量 减去标识符参数
            for (int i = 2; i <= nargs + 1; ++i) {
                lua_pushvalue(L, i);
            }
            lua_call(L, nargs, 0);  // 调用
        } else {
            printf("callback with identifier '%s' not found or is not a function", identifier);
        }
        lua_pop(L, 1);
    } else {
        printf("callback table is noref");
    }
    return 0;
}

bool neko_lua_equal(lua_State *state, int index1, int index2) {
#if LUA_VERSION_NUM <= 501
    return lua_equal(state, index1, index2) == 1;
#else
    return lua_compare(state, index1, index2, LUA_OPEQ) == 1;
#endif
}

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
    return 0;
}

int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name) {
    neko_lua_preload(L, f, name);
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    lua_call(L, 1, 0);
    return 0;
}

void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name) {
    lua_getglobal(L, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, l, 0);
    lua_setglobal(L, name);
}

void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name) {
    lua_newtable(L);
    luaL_setfuncs(L, l, 0);
    lua_setglobal(L, name);
}

int neko_lua_get_table_pairs_count(lua_State *L, int index) {
    int count = 0;
    index = lua_absindex(L, index);
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        // 现在栈顶是value，下一个栈元素是key
        count++;
        lua_pop(L, 1);  // 移除value，保留key作为下一次迭代的key
    }
    return count;
}

void luax_neko_get(lua_State *L, const char *field) {
    lua_getglobal(L, "neko");
    lua_getfield(L, -1, field);
    lua_remove(L, -2);
}

int luax_msgh(lua_State *L) {
    if (gBase.error_mode.load()) {
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

    if (LockGuard<Mutex> lock{gBase.error_mtx}) {
        gBase.fatal_error_string = to_cstr(err);
        gBase.traceback = to_cstr(traceback);

        fprintf(stderr, "================ Lua Error ================\n%s\n%s\n", gBase.fatal_error_string.data, gBase.traceback.data);

        for (u64 i = 0; i < gBase.traceback.len; i++) {
            if (gBase.traceback.data[i] == '\t') {
                gBase.traceback.data[i] = ' ';
            }
        }

        gBase.error_mode.store(true);
    }

    // 返回带有堆栈跟踪的错误消息
    return 1;
}

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

static void lua_thread_proc(void *udata) {
    PROFILE_FUNC();

    LuaThread *lt = (LuaThread *)udata;

    // lua_State *L = lua_newstate(luaalloc, LA);
    // neko_defer(lua_close(L));

    LuaVM vm;
    vm.Create(true);

    lua_State *L = vm;
    neko_defer(vm.Fini(L));

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
    contents = to_cstr(code);
    name = to_cstr(thread_name);

    LockGuard<Mutex> lock{mtx};
    thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
    if (LockGuard<Mutex> lock{mtx}) {
        thread.join();
    }
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
            luax_new_userdata(L, udata.ptr, udata.tname.data);
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
}

void LuaChannel::send(LuaVariant item) {
    LockGuard<Mutex> lock{mtx};

    received.wait(lock, [this] { return len == items.len; });

    items[back] = item;
    back = (back + 1) % items.len;
    len++;

    g_channels.select.notify_all();
    sent.notify_one();
    sent_total++;

    received.wait(lock, [this] { return sent_total >= received_total + items.len; });
}

static LuaVariant lua_channel_dequeue(LuaChannel *ch) {
    LuaVariant item = ch->items[ch->front];
    ch->front = (ch->front + 1) % ch->items.len;
    ch->len--;

    ch->received.notify_all();
    ch->received_total++;

    return item;
}

LuaVariant LuaChannel::recv() {
    LockGuard<Mutex> lock{mtx};

    sent.wait(lock, [this] { return len == 0; });

    return lua_channel_dequeue(this);
}

bool LuaChannel::try_recv(LuaVariant *v) {
    LockGuard<Mutex> lock{mtx};

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

    LockGuard<Mutex> lock{g_channels.mtx};
    g_channels.by_name[fnv1a(name)] = chan;

    return chan;
}

LuaChannel *lua_channel_get(String name) {
    LockGuard<Mutex> lock{g_channels.mtx};

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
    LockGuard<Mutex> lock{mtx};

    while (true) {
        for (i32 i = 0; i < len; i++) {
            LockGuard<Mutex> lock{buf[i]->mtx};
            if (buf[i]->len > 0) {
                *v = lua_channel_dequeue(buf[i]);
                return buf[i];
            }
        }

        g_channels.select.wait(lock);
    }
}

void lua_channels_setup() {}

void lua_channels_shutdown() {
    for (auto [k, v] : g_channels.by_name) {
        LuaChannel *chan = *v;
        chan->trash();
        mem_free(chan);
    }
    g_channels.by_name.trash();
}

// lua json

namespace Neko {

void json_to_lua(lua_State *L, JSON *json) {
    switch (json->kind) {
        case JSONKind_Object: {
            lua_newtable(L);
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                lua_pushlstring(L, o->key.data, o->key.len);
                json_to_lua(L, &o->value);
                lua_rawset(L, -3);
            }
            break;
        }
        case JSONKind_Array: {
            lua_newtable(L);
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                json_to_lua(L, &a->value);
                lua_rawseti(L, -2, a->index + 1);
            }
            break;
        }
        case JSONKind_String: {
            lua_pushlstring(L, json->string.data, json->string.len);
            break;
        }
        case JSONKind_Number: {
            lua_pushnumber(L, json->number);
            break;
        }
        case JSONKind_Boolean: {
            lua_pushboolean(L, json->boolean);
            break;
        }
        case JSONKind_Null: {
            lua_pushnil(L);
            break;
        }
        default:
            break;
    }
}

static void lua_to_json_string(StringBuilder &sb, lua_State *L, HashMap<bool> *visited, String *err, i32 width, i32 level) {
    auto indent = [&](i32 offset) {
        if (width > 0) {
            sb << "\n";
            sb.concat(" ", width * (level + offset));
        }
    };

    if (err->len != 0) {
        return;
    }

    i32 top = lua_gettop(L);
    switch (lua_type(L, top)) {
        case LUA_TTABLE: {
            uintptr_t ptr = (uintptr_t)lua_topointer(L, top);

            bool *visit = nullptr;
            bool exist = visited->find_or_insert(ptr, &visit);
            if (exist && *visit) {
                *err = "table has cycles";
                return;
            }

            *visit = true;

            lua_pushnil(L);
            if (lua_next(L, -2) == 0) {
                sb << "[]";
                return;
            }

            i32 key_type = lua_type(L, -2);

            if (key_type == LUA_TNUMBER) {
                sb << "[";

                indent(0);
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                i32 len = luax_len(L, top);
                assert(len > 0);
                i32 i = 1;
                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TNUMBER) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be numbers";
                        return;
                    }

                    sb << ",";
                    indent(0);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    i++;
                }
                indent(-1);
                sb << "]";

                if (i != len) {
                    *err = "array is not continuous";
                    return;
                }
            } else if (key_type == LUA_TSTRING) {
                sb << "{";
                indent(0);

                lua_pushvalue(L, -2);
                lua_to_json_string(sb, L, visited, err, width, level + 1);
                lua_pop(L, 1);
                sb << ":";
                if (width > 0) {
                    sb << " ";
                }
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TSTRING) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be strings";
                        return;
                    }

                    sb << ",";
                    indent(0);

                    lua_pushvalue(L, -2);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    lua_pop(L, 1);
                    sb << ":";
                    if (width > 0) {
                        sb << " ";
                    }
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                }
                indent(-1);
                sb << "}";
            } else {
                lua_pop(L, 2);  // key, value
                *err = "expected table keys to be strings or numbers";
                return;
            }

            visited->unset(ptr);
            break;
        }
        case LUA_TNIL:
            sb << "null";
            break;
        case LUA_TNUMBER:
            sb << tmp_fmt("%g", lua_tonumber(L, top));
            break;
        case LUA_TSTRING:
            sb << "\"" << luax_check_string(L, top) << "\"";
            break;
        case LUA_TBOOLEAN:
            sb << (lua_toboolean(L, top) ? "true" : "false");
            break;
        case LUA_TFUNCTION:
            sb << tmp_fmt("\"func %p\"", lua_topointer(L, top));
            break;
        default:
            *err = "type is not serializable";
    }
}

String lua_to_json_string(lua_State *L, i32 arg, String *contents, i32 width) {
    StringBuilder sb = {};

    HashMap<bool> visited = {};
    neko_defer(visited.trash());

    String err = {};
    lua_pushvalue(L, arg);
    lua_to_json_string(sb, L, &visited, &err, width, 1);
    lua_pop(L, 1);

    if (err.len != 0) {
        sb.trash();
    }

    *contents = String(sb);
    return err;
}

size_t luax_dump_traceback(lua_State *L, char *buf, size_t sz, int is_show_var, int is_show_tmp_var, int top_max, int bottom_max) {
    size_t e = 0;
    int level = 1;
    int firstpart = 1;
    lua_Debug ar;
    int i = 0;

    auto get_params = [](lua_State *L, char nparams, char isvararg, lua_Debug *ar, char *buff, size_t buff_sz) -> size_t {
        int i = 0;
        const char *name;
        size_t e = 0;
        while ((name = lua_getlocal(L, ar, i++)) != NULL) {
            e += snprintf(buff + e, buff_sz - e, "%s, ", name);
            lua_pop(L, 1);
        }
        if (e > 0) e -= 2;
        buff[e] = '\0';
        if (isvararg) {
            e += snprintf(buff + e, buff_sz - e, ", ...");
        }
        return e;
    };

#define XX(buf, sz, e, fmt, ...)                                            \
    do {                                                                    \
        if (sz - e > 0) e += snprintf(buf + e, sz - e, fmt, ##__VA_ARGS__); \
    } while (0)

    while (lua_getstack(L, level++, &ar)) {
        if (level > top_max && firstpart) {
            if (!lua_getstack(L, level + bottom_max, &ar))
                level--;
            else {
                XX(buf, sz, e, "%s", "\n...");
                while (lua_getstack(L, level + bottom_max, &ar)) level++;
            }
            firstpart = 0;
            continue;
        }

        if (level > 2) XX(buf, sz, e, "\n");

        lua_getinfo(L, "Snl", &ar);
        XX(buf, sz, e, "[%d]%s:", level - 2, ar.short_src);
        if (ar.currentline > 0) XX(buf, sz, e, "%d:", ar.currentline);
        if (*ar.namewhat != '\0') {
            char parambuf[256] = {0};
            get_params(L, ar.nparams, ar.isvararg, &ar, parambuf, sizeof(parambuf));
            XX(buf, sz, e, " in function %s(%s)", ar.name, parambuf);
        } else {
            if (*ar.what == 'm')
                XX(buf, sz, e, " in main chunk");
            else if (*ar.what == 'C' || *ar.what == 't')
                XX(buf, sz, e, " ?");
            else
                XX(buf, sz, e, " in function <%s:%d>", ar.short_src, ar.linedefined);
        }

        if (!lua_checkstack(L, 1))  // 用于 lua_getlocal
            continue;

        i = 1;
        while (is_show_var) {
            const void *pointer;
            int type;
            lua_Debug arf;
            const char *name = lua_getlocal(L, &ar, i++);
            const char *type_name;
            if (name == NULL) break;
            if (!is_show_tmp_var && name[0] == '(') continue;
            type = lua_type(L, -1);
            type_name = lua_typename(L, type);
            XX(buf, sz, e, "\n\t%s(%s) : ", name, type_name);
            switch (type) {
                case LUA_TFUNCTION:
                    pointer = lua_topointer(L, -1);
                    lua_getinfo(L, ">Snl", &arf);
                    if (*arf.what == 'C' || *arf.what == 't')
                        XX(buf, sz, e, "%p %s@C", pointer, (arf.name != NULL ? arf.name : "defined"));
                    else
                        XX(buf, sz, e, "%p %s@%s:%d", pointer, (arf.name != NULL ? arf.name : "defined"), arf.short_src, arf.linedefined);
                    break;
                case LUA_TBOOLEAN:
                    XX(buf, sz, e, "%s", lua_toboolean(L, 1) ? "true" : "false");
                    lua_pop(L, 1);
                    break;
                case LUA_TNIL:
                    XX(buf, sz, e, "nil");
                    lua_pop(L, 1);
                    break;
                case LUA_TNUMBER:
                    XX(buf, sz, e, "%s", lua_tostring(L, -1));
                    lua_pop(L, 1);
                    break;
                case LUA_TSTRING:
                    XX(buf, sz, e, "\"%s\"", lua_tostring(L, -1));
                    lua_pop(L, 1);
                    break;
                default:
                    XX(buf, sz, e, "%p", lua_topointer(L, -1));
                    lua_pop(L, 1);
                    break;
            }
        }
    }

#undef XX

    return e;
}

}  // namespace Neko