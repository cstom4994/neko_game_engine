#include "event.h"

#include "base/scripting/luax.h"

void EventHandler::init() {

    assert(NUM_EVENTS < 32);
    m_evqueue.make();
    m_delegate_map.reserve(NUM_EVENTS);
}

void EventHandler::fini() {

    m_evqueue.trash();

    for (int i = 0; i < NUM_EVENTS; i++) {
        DelegateArray* list = event_getdelegates(this, i);
        if (list == nullptr) continue;
        list->trash();
    }
    m_delegate_map.trash();
}

void EventHandler::update() {}

void EventHandler::event_register(void* receiver, int evt, EventCallback cb, lua_State* L) {
    for (int i = 0; i < NUM_EVENTS; i++) {
        if ((1 << i) & evt) {

            EventDelegate l = {};

            l.callback = cb;
            l.receiver = receiver;
            l.L = L;

            DelegateArray* list = event_getdelegates(this, i);
            if (list != nullptr) {
                list->push(l);
            } else {
                DelegateArray new_list = {};
                new_list.push(l);
                m_delegate_map[i] = new_list;
            }
        }
    }
}

// 立即调用事件监听器
void EventHandler::event_dispatch(event_t evt) {
    DelegateArray* list = event_getdelegates(this, evt.type);
    if (list == nullptr) {
        console_log("event_dispatch: no event cb called %s", event_string(evt.type));
        return;
    }
    for (auto l : *list) {
        if (l.L) {
            lua_rawgeti(l.L, LUA_REGISTRYINDEX, l.lua_cb);
            lua_rawgeti(l.L, LUA_REGISTRYINDEX, l.lua_ref);
            lua_pushinteger(l.L, evt.type);
            lua_pushinteger(l.L, std::get<u64>(evt.p0.v));  // TODO 换为 lua_value
            lua_pushinteger(l.L, std::get<u64>(evt.p1.v));
            lua_call(l.L, 4, 1);
            if (lua_toboolean(l.L, -1)) break;
        } else {
            error_assert(l.callback);
            if (l.callback(l.receiver, evt)) break;
        }
    }
}

// 将事件放入队列
int EventHandler::event_post(event_t evt) {
    m_evqueue.enqueue(evt);
    return 1;
}

// 清空事件队列并将事件分派给所有监听器
void EventHandler::event_pump() {
    m_prev_len = m_evqueue.len;
    while (m_evqueue.len > 0) {
        event_t evt = m_evqueue.demand();
        event_dispatch(evt);
    }
}

// event_t.listen(obj, evt, func [, filter])
static int w_event_listen(lua_State* L) {
    lua_pushvalue(L, 1);
    i64 ref = luaL_ref(L, LUA_REGISTRYINDEX);
    int evt = luaL_checkinteger(L, 2);
    i64 cb = 0;
    if (lua_isfunction(L, 3)) {
        lua_pushvalue(L, 3);
        cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    auto& eh = Neko::the<EventHandler>();
    eh.event_register(reinterpret_cast<void*>(ref), evt, reinterpret_cast<EventCallback>(cb), L);
    return 0;
}

// event_t(num, data)
static int w_event_new(lua_State* L) {
    f64 float64;
    event_t* evt = (event_t*)lua_newuserdata(L, sizeof(event_t));
    luaL_setmetatable(L, Event_mt);
    evt->type = (event_enum)luaL_checkinteger(L, 2);
    switch (lua_type(L, 3)) {
        case LUA_TNUMBER:
            float64 = lua_tonumber(L, 3);
            if (float64 == (int)float64)
                evt->p0.v = (u64)float64;  // uint64_t
            else
                evt->p0.v = (f64)float64;  // double
            break;
        case LUA_TNIL:
        case LUA_TNONE:
            evt->p0.v = (u64)0;
            break;
        default:
            lua_pushvalue(L, 3);
            evt->p0.v = (u64)luaL_ref(L, LUA_REGISTRYINDEX);
            break;
    }
    return 1;
}

// event_t.dispatch(evt, [, data])
static int w_event_postnow(lua_State* L) {
    event_t* evt = (event_t*)luaL_checkudata(L, 1, Event_mt);
    auto& eh = Neko::the<EventHandler>();
    eh.event_dispatch(*evt);
    return 0;
}

// event_t.post(evt [, data])
static int w_event_postlater(lua_State* L) {
    event_t* evt = (event_t*)luaL_checkudata(L, 1, Event_mt);
    auto& eh = Neko::the<EventHandler>();
    eh.event_post(*evt);
    return 0;
}

int openlib_Event(lua_State* L) {
    static luaL_Reg event_func[] = {
            {"post", w_event_postlater},
            {"dispatch", w_event_postnow},
            {"listen", w_event_listen},
            {NULL, NULL},
    };
    create_lua_class(L, Event_mt, w_event_new, event_func);

    lua_getglobal(L, Event_mt);
#define X(_name)                     \
    lua_pushstring(L, "on_" #_name); \
    lua_pushnumber(L, _name);        \
    lua_settable(L, -3);
    EVENT_TYPES
#undef X
    lua_setglobal(L, "event");
    return 0;
}