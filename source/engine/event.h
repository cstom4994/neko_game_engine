#pragma once

#include <variant>

#include "engine/base.hpp"
#include "engine/base/singleton.hpp"

#define Event_mt "event"

#define event_getdelegates(_handler, _evt) ((_handler)->delegate_map.get(_evt))

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

using event_variant_t = struct {
    int t;
    std::variant<std::monostate, u64, f64, void*> v;
};

typedef struct {
    event_enum type;
    event_variant_t p0;
    event_variant_t p1;
} event_t;

typedef int (*evt_callback_t)(void*, event_t);

typedef struct delegate_t {
    union {
        struct {
            void* receiver;
            evt_callback_t callback;
        };
        struct {
            int lua_ref;
            int lua_cb;
        };
    };
    lua_State* L;
} delegate_t;

class EventHandler : public neko::SingletonClass<EventHandler> {
    using delegate_list_t = Array<delegate_t>;

private:
    Queue<event_t> queue;
    HashMap<delegate_list_t> delegate_map;
    u64 prev_len;

public:
    void init() override;
    void fini() override;
    void update() override;

    void event_register(void* receiver, int evt, evt_callback_t cb, lua_State* L);
    int event_post(event_t evt);
    void event_dispatch(event_t evt);
    void event_pump();
};

int openlib_Event(lua_State* L);
