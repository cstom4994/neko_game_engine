#ifndef NEKO_TEST_MODULES
#define NEKO_TEST_MODULES

#include "engine/base.hpp"
#include "engine/event.h"
#include "base/scripting/luax.h"

struct App;
NEKO_API() void physics_init();
NEKO_API() void physics_fini();
NEKO_API() int physics_update_all(App *app, event_t evt);
NEKO_API() int physics_post_update_all(App *app, event_t evt);
NEKO_API() void physics_draw_all();

#endif