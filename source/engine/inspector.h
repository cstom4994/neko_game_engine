
#ifndef NEKO_INSPECTOR_H
#define NEKO_INSPECTOR_H

#include "engine/glew_glfw.h"

void render_uniform_variable(GLuint program, GLenum type, const char* name, GLint location);
void inspect_shader(const char* label, GLuint program);
void inspect_vertex_array(const char* label, GLuint vao);

#endif