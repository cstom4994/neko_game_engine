#ifndef NEKO_WINDOW
#define NEKO_WINDOW

#include "engine/base.hpp"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "base/scripting/lua_wrapper.hpp"

const char *window_clipboard();
int window_prompt(const char *msg, const char *title);
void window_setclipboard(const char *text);
void window_focus();
int window_has_focus();
double window_scale();

#endif