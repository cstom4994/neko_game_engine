#pragma once

#include <variant>

#include "engine/base.hpp"
#include "base/common/singleton.hpp"
#include "engine/input_keycode.h"



struct lua_State;

#define EVENT_TYPES   \
    X(None)           \
    X(Exit)           \
    X(KeyDown)        \
    X(KeyUp)          \
    X(MouseDown)      \
    X(MouseUp)        \
    X(MouseMove)      \
    X(MouseScroll)    \
    X(FileRead)       \
    X(FileWrite)      \
    X(PreUpdate)      \
    X(Update)         \
    X(PostUpdate)     \
    X(PreDraw)        \
    X(Draw)           \
    X(PostDraw)       \
    X(PreDrawUI)      \
    X(DrawUI)         \
    X(PostDrawUI)     \
    X(AddComponent)   \
    X(SetComponent)   \
    X(DelComponent)   \
    X(AddType)        \
    X(DelType)        \
    X(StartCollision) \
    X(EndCollision)

enum EventEnum {
#define X(name) On##name,
    EVENT_TYPES
#undef X
            NUM_EVENTS
};

enum EventMask {
#define X(name) name = 1 << On##name,
    EVENT_TYPES
#undef X
};

namespace Neko {

constexpr const char* Event_mt = "event_mt";

#define event_getdelegates(_handler, _evt) ((_handler)->m_delegate_map.get(_evt))

using EventVariant = struct {
    int t;
    std::variant<std::monostate, u64, f64, void*> v;
};

struct Event {
    EventEnum type;
    EventVariant p0;
    EventVariant p1;
};

// using EventCallback = std::function<int(Game*, Event)>;
typedef int (*EventCallback)(Event);

typedef struct EventDelegate {
    union {
        struct {
            // Game* receiver;
            EventCallback callback;
        };
        struct {
            int lua_ref;
            int lua_cb;
        };
    };
    lua_State* L;
} EventDelegate;

class EventHandler : public Neko::SingletonClass<EventHandler> {
    using DelegateArray = Array<EventDelegate>;

private:
    Queue<Event> m_evqueue;
    HashMap<DelegateArray> m_delegate_map;
    u64 m_prev_len;

    HashMap<String> eventNames;

public:
    void init();
    void fini();
    void update();

    void Register(int evt, EventCallback cb, lua_State* L);
    int Post(Event evt);
    void Dispatch(Event evt);
    void Pump();

    void EventPushLua(String event);

    template <typename... Args>
    inline void EventPushLuaArgs(lua_State* L, Args&&... args) {
        VaradicLuaPush(L, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void EventPushLuaType(EventEnum type, Args&&... args) {
        lua_State* L = ENGINE_LUA();
        String& name = eventNames[type];
        int n = sizeof...(args);
        EventPushLua(name.cstr());
        EventPushLuaArgs(L, std::forward<Args>(args)...);
        errcheck(L, luax_pcall_nothrow(L, 1 + n, 0));
    }

    String EventName(EventEnum type) { return eventNames[type]; }
};

int openlib_Event(lua_State* L);

}  // namespace Neko