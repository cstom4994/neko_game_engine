#include "engine/batch.h"

#include "engine/asset.h"
#include "engine/camera.h"
#include "engine/gfx.h"
#include "engine/os.h"
#include "engine/texture.h"

// static batch_renderer batch;

typedef struct {
    // position
    float px, py;
    // texcoords
    float tx, ty, tw, th;
} batch_tex;

static GLuint batch_shader;

void batch_test_draw(batch_renderer *renderer, AssetTexture tex, batch_tex a) {
    batch_texture(renderer, tex.id);

    float x1 = a.px;
    float y1 = a.py;
    float x2 = a.px + 24;
    float y2 = a.py + 24;

    float u1 = a.tx / tex.width;
    float v1 = a.ty / tex.height;
    float u2 = (a.tx + a.tw) / tex.width;
    float v2 = (a.ty + a.th) / tex.height;

    batch_push_vertex(renderer, x1, y1, u1, v1);
    batch_push_vertex(renderer, x2, y2, u2, v2);
    batch_push_vertex(renderer, x1, y2, u1, v2);

    batch_push_vertex(renderer, x1, y1, u1, v1);
    batch_push_vertex(renderer, x2, y1, u2, v1);
    batch_push_vertex(renderer, x2, y2, u2, v2);
}

batch_renderer *batch_init(int vertex_capacity) {
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

    if (batch_shader == 0) {
        batch_shader = gfx_create_program("batch", "shader/batch.vert", NULL, "shader/batch.frag");
    }

    batch_renderer *batch = (batch_renderer *)mem_alloc(sizeof(batch_renderer));

    asset_load(AssetLoadData{AssetKind_Image, false}, "assets/aliens.png", NULL);

    // batch->shader = program;
    batch->vao = vao;
    batch->vbo = vbo;
    batch->vertex_count = 0;
    batch->vertex_capacity = vertex_capacity;
    batch->vertices = (Vertex *)mem_alloc(sizeof(Vertex) * vertex_capacity);
    batch->texture = 0;
    batch->scale = 0;

    return batch;
}

void batch_fini(batch_renderer *batch) { mem_free(batch->vertices); }

void batch_update_all(batch_renderer *batch) {
    auto tex_aliens = texture_get_ptr("assets/aliens.png");

    struct {
        float x, y, w, h;
    } alien_uvs[] = {
            {2, 2, 24, 24}, {58, 2, 24, 24}, {114, 2, 24, 24}, {170, 2, 24, 24}, {2, 30, 24, 24},
    };

    batch_tex ch = {
            .px = 0,
            .py = 48,
            .tx = alien_uvs[2].x,
            .ty = alien_uvs[2].y,
            .tw = alien_uvs[2].w,
            .th = alien_uvs[2].h,
    };
    batch_test_draw(batch, tex_aliens, ch);
}

void batch_draw_all(batch_renderer *batch) { batch_flush(batch); }

void batch_flush(batch_renderer *renderer) {
    if (renderer->vertex_count == 0) {
        return;
    }

    glUseProgram(batch_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->texture);

    glUniform1i(glGetUniformLocation(batch_shader, "u_texture"), 0);
    // glUniformMatrix4fv(glGetUniformLocation(batch_shader, "u_mvp"), 1, GL_FALSE, (const GLfloat *)&renderer->mvp.cols[0]);
    glUniformMatrix3fv(glGetUniformLocation(batch_shader, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    glUniform1f(glGetUniformLocation(batch_shader, "scale"), renderer->scale);

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
