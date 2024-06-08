#ifndef GAME_EDITOR_H
#define GAME_EDITOR_H

#include <string>
#include <typeindex>
#include <vector>

#include "engine/neko_imgui.hpp"
#include "engine/neko_lua.hpp"

void render_uniform_variable(GLuint program, GLenum type, const char* name, GLint location);
void inspect_shader(const char* label, GLuint program);
void inspect_vertex_array(const char* label, GLuint vao);


#endif