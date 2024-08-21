#ifndef NEKO_BATCH_H
#define NEKO_BATCH_H

#include "engine/glew_glfw.h"

typedef struct {
    float position[2];
    float texcoord[2];
} Vertex;

struct batch_renderer {

    // vertex buffer data
    GLuint vao;
    GLuint vbo;
    int vertex_count;
    int vertex_capacity;
    Vertex *vertices;

    float scale;

    GLuint texture;
};

batch_renderer *batch_init(int vertex_capacity);
void batch_fini(batch_renderer *batch);
void batch_update_all(batch_renderer *batch);
void batch_draw_all(batch_renderer *batch);

void batch_flush(batch_renderer *renderer);
void batch_texture(batch_renderer *renderer, GLuint id);
void batch_push_vertex(batch_renderer *renderer, float x, float y, float u, float v);

#endif