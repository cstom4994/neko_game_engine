#include "gfx.h"

#include <stdio.h>
#include <stdlib.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/console.h"
#include "engine/game.h"

// deps
#include <stb_image.h>

static GLint gfx_compile_shader(GLuint shader, const char *filename) {
    char log[512];
    GLint status;

    String contents = {};

    bool ok = vfs_read_entire_file(&contents, filename);
    neko_defer(mem_free(contents.data));

    neko_assert(ok);

    console_printf("gfx: compiling shader '%s' ...", filename);

    glShaderSource(shader, 1, (const GLchar **)&contents.data, NULL);
    glCompileShader(shader);

    // log
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    console_printf(status ? " successful\n" : " unsuccessful\n");
    glGetShaderInfoLog(shader, 512, NULL, log);
    console_printf("%s", log);

    return status;
}

GLuint gfx_create_program(const_str name, const char *vert_path, const char *geom_path, const char *frag_path) {
    GLuint vert, geom, frag, program;

#define compile(shader, type)                                     \
    if (shader##_path) {                                          \
        shader = glCreateShader(type);                            \
        if (!gfx_compile_shader(shader, shader##_path)) return 0; \
    }

    compile(vert, GL_VERTEX_SHADER);
    compile(geom, GL_GEOMETRY_SHADER);
    compile(frag, GL_FRAGMENT_SHADER);

    program = glCreateProgram();

    if (vert_path) glAttachShader(program, vert);
    if (geom_path) glAttachShader(program, geom);
    if (frag_path) glAttachShader(program, frag);

    glLinkProgram(program);

    // GL will automatically detach and free shaders when program is deleted
    if (vert_path) glDeleteShader(vert);
    if (geom_path) glDeleteShader(geom);
    if (frag_path) glDeleteShader(frag);

    shader_pair pair = {program, name};
    neko_dyn_array_push(g_app->shader_array, pair);

    return program;
}

batch_renderer batch_init(int vertex_capacity) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_capacity, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));

    GLuint program = gfx_create_program("batch", "shader/batch.vert", NULL, "shader/batch.frag");

    return batch_renderer{
            .shader = program,
            .vao = vao,
            .vbo = vbo,
            .vertex_count = 0,
            .vertex_capacity = vertex_capacity,
            .vertices = (Vertex *)mem_alloc(sizeof(Vertex) * vertex_capacity),
            .texture = 0,
    };
}

void batch_flush(batch_renderer *renderer) {
    if (renderer->vertex_count == 0) {
        return;
    }

    glUseProgram(renderer->shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->texture);

    glUniform1i(glGetUniformLocation(renderer->shader, "u_texture"), 0);
    // glUniformMatrix4fv(glGetUniformLocation(renderer->shader, "u_mvp"), 1, GL_FALSE, (const GLfloat *)&renderer->mvp.cols[0]);
    glUniformMatrix3fv(glGetUniformLocation(renderer->shader, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    glUniform1f(glGetUniformLocation(renderer->shader, "scale"), renderer->scale);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * renderer->vertex_count, renderer->vertices);

    glBindVertexArray(renderer->vao);
    glDrawArrays(GL_TRIANGLES, 0, renderer->vertex_count);

    renderer->vertex_count = 0;
}

void batch_texture(batch_renderer *renderer, GLuint id) {
    if (renderer->texture != id) {
        batch_flush(renderer);
        renderer->texture = id;
    }
}

void batch_push_vertex(batch_renderer *renderer, float x, float y, float u, float v) {
    if (renderer->vertex_count == renderer->vertex_capacity) {
        batch_flush(renderer);
    }

    renderer->vertices[renderer->vertex_count++] = Vertex{
            .position = {x, y},
            .texcoord = {u, v},
    };
}
