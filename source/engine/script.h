#ifndef SCRIPT_H
#define SCRIPT_H

#include "engine/base.h"
#include "engine/input.h"
#include "engine/prelude.h"

void script_run_string(const char *s);
void script_run_file(const char *filename);
void script_error(const char *s);

void script_init();
void script_fini();
void script_update_all();
void script_post_update_all();
void script_draw_all();
void script_key_down(KeyCode key);
void script_key_up(KeyCode key);
void script_mouse_down(MouseCode mouse);
void script_mouse_up(MouseCode mouse);
void script_mouse_move(LuaVec2 pos);
void script_scroll(LuaVec2 scroll);
void script_save_all(Store *s);
void script_load_all(Store *s);

int luax_pcall_nothrow(lua_State *L, int nargs, int nresults);
void script_push_event(const char *event);

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
