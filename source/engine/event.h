#pragma once

#include <variant>

#include "engine/base.hpp"
#include "base/common/singleton.hpp"

#define Event_mt "event"

#define event_getdelegates(_handler, _evt) ((_handler)->m_delegate_map.get(_evt))

#define EVENT_TYPES   \
    X(none)           \
    X(quit)           \
    X(press)          \
    X(release)        \
    X(fileread)       \
    X(filewrite)      \
    X(preupdate)      \
    X(update)         \
    X(postupdate)     \
    X(predraw)        \
    X(draw)           \
    X(postdraw)       \
    X(pregui)         \
    X(gui)            \
    X(postgui)        \
    X(addcomponent)   \
    X(setcomponent)   \
    X(delcomponent)   \
    X(addtype)        \
    X(deltype)        \
    X(startcollision) \
    X(endcollision)

enum event_enum {
#define X(_name) on_##_name,
    EVENT_TYPES
#undef X
            NUM_EVENTS
};

static const char* event_string(event_enum id) {

#define X(x)       \
    case on_##x:   \
        return #x; \
        break;

    switch (id) {
        EVENT_TYPES;
        case NUM_EVENTS:
            break;
    }

#undef X

    static char buffer[32];
    sprintf(buffer, "unknown event_string: %d", id);
    return buffer;
}

enum event_mask {
#define X(_name) _name = 1 << on_##_name,
    EVENT_TYPES
#undef X
};

struct lua_State;

using EventVariant = struct {
    int t;
    std::variant<std::monostate, u64, f64, void*> v;
};

typedef struct {
    event_enum type;
    EventVariant p0;
    EventVariant p1;
} event_t;

typedef int (*EventCallback)(void*, event_t);

typedef struct EventDelegate {
    union {
        struct {
            void* receiver;
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
    Queue<event_t> m_evqueue;
    HashMap<DelegateArray> m_delegate_map;
    u64 m_prev_len;

public:
    void init();
    void fini();
    void update();

    void event_register(void* receiver, int evt, EventCallback cb, lua_State* L);
    int event_post(event_t evt);
    void event_dispatch(event_t evt);
    void event_pump();
};

int openlib_Event(lua_State* L);
