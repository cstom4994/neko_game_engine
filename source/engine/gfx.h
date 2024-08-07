#ifndef GFX_H
#define GFX_H

#include "engine/glew_glfw.h"

struct shader_pair {
    GLuint id;
    const char *name;
};

// compile link program given paths to shader files possibly NULL doesn't glUseProgram(...)
GLuint gfx_create_program(const char *name, const char *vert_path, const char *geom_path, const char *frag_path);
void gfx_free_program(GLuint program);

// get pointer offset of 'field' in struct 'type'
#define poffsetof(type, field) ((void *)(&((type *)0)->field))

// bind vertex attribute data to 'field' in struct 'type' -- gl_type
// is the GL_* type of the parameter (GL_FLOAT etc.), components is the
// number of components, param_name is a string containing the name of
// the parameter as it appears in the program
#define gfx_bind_vertex_attrib(program, gl_type, components, param_name, type, field)                    \
    do {                                                                                                 \
        GLuint a__ = glGetAttribLocation(program, param_name);                                           \
        glVertexAttribPointer(a__, components, gl_type, GL_FALSE, sizeof(type), poffsetof(type, field)); \
        glEnableVertexAttribArray(a__);                                                                  \
    } while (0)

typedef struct {
    float position[2];
    float texcoord[2];
} Vertex;

typedef struct {
    GLuint shader;

    // vertex buffer data
    GLuint vao;
    GLuint vbo;
    int vertex_count;
    int vertex_capacity;
    Vertex *vertices;

    // uniform values
    GLuint texture;
    // Matrix mvp;
} BatchRenderer;

BatchRenderer batch_init(int vertex_capacity);
void batch_flush(BatchRenderer *renderer);
void batch_texture(BatchRenderer *renderer, GLuint id);
void batch_push_vertex(BatchRenderer *renderer, float x, float y, float u, float v);

#endif
