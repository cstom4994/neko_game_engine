#ifndef EDIT_H
#define EDIT_H

#include <stdbool.h>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "base/scripting/lua_wrapper.hpp"

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location);
void inspect_shader(const char *label, GLuint program);
void inspect_vertex_array(const char *label, GLuint vao);

NEKO_SCRIPT(console,

            NEKO_EXPORT void console_set_visible(bool visible);

            NEKO_EXPORT bool console_get_visible();

            NEKO_EXPORT void console_puts(const char *s);

            NEKO_EXPORT void console_printf(const char *fmt, ...);

)

void console_init();
void console_fini();

#endif
