
#include "cloud.h"

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"
#include "engine/components/transform.h"
#include "engine/components/camera.h"

CCloud *Cloud::ComponentAdd(CEntity ent) {

    CCloud *cloud = ComponentGetPtr(ent);
    if (cloud) return cloud;

    // the<Transform>().Add(ent);

    cloud = ComponentTypeBase::EntityPool->Add(ent);

    cloud->pos = neko_v2(0, 0);
    cloud->size = neko_v2(500, 300);

    return cloud;
}

void Cloud::ComponentRemove(CEntity ent) { ComponentTypeBase::EntityPool->Remove(ent); }

void Cloud::cloud_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    ComponentTypeBase::ComponentReg();

    bool ok = asset_load_kind(AssetKind_Shader, "@code/game/shader/cloud.glsl", &cloud_shader);
    error_assert(ok);

    GLuint sid = assets_get<AssetShader>(cloud_shader).id;

    glGenVertexArrays(1, &cloud_vao);
    glBindVertexArray(cloud_vao);

    glGenBuffers(1, &cloud_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cloud_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CloudVertex) * vertex_capacity, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CloudVertex), (void *)offsetof(CloudVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CloudVertex), (void *)offsetof(CloudVertex, texcoord));

    vertices = (CloudVertex *)mem_alloc(sizeof(CloudVertex) * vertex_capacity);

    // clang-format off

    auto type = BUILD_TYPE(Cloud)
        .MemberMethod<ComponentTypeBase>("cloud_add", this, &ComponentTypeBase::WrapAdd)
        .MemberMethod<ComponentTypeBase>("cloud_has", this, &ComponentTypeBase::ComponentHas)
        .MemberMethod("cloud_remove", this, &Cloud::ComponentRemove)
        .Build();

    // clang-format on
}

void Cloud::cloud_fini() {

    glDeleteBuffers(1, &cloud_vbo);
    glDeleteVertexArrays(1, &cloud_vao);

    entitypool_free(ComponentTypeBase::EntityPool);

    mem_free(vertices);
}

void Cloud::cloud_push_vertex(float x, float y, float u, float v) {
    if (vertex_count == vertex_capacity) {
        neko_assert(0);
    }

    vertices[vertex_count++] = CloudVertex{
            .position = {x, y},
            .texcoord = {u, v},
    };
}

int Cloud::cloud_update_all(Event evt) {

    entitypool_remove_destroyed(ComponentTypeBase::EntityPool, [this](CEntity ent) { ComponentRemove(ent); });

    ComponentTypeBase::EntityPool->ForEach([this](CCloud *cloud) {
        float x1 = cloud->pos.x;
        float y1 = cloud->pos.y;
        float x2 = x1 + cloud->size.x;
        float y2 = y1 - cloud->size.y;

        float u1 = 0.f;
        float v1 = 0.f;
        float u2 = 1.f;
        float v2 = 1.f;

        cloud_push_vertex(x1, y1, u1, v2);
        cloud_push_vertex(x2, y2, u2, v1);
        cloud_push_vertex(x1, y2, u1, v1);

        cloud_push_vertex(x1, y1, u1, v2);
        cloud_push_vertex(x2, y1, u2, v2);
        cloud_push_vertex(x2, y2, u2, v1);
    });

    return 0;
}

void Cloud::cloud_draw_all() {

    GLuint sid = assets_get<AssetShader>(cloud_shader).id;

    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)the<Camera>().GetInverseViewMatrixPtr());

    neko_shader_set_float(sid, "u_time", the<CL>().timing_get_elapsed() / 8000.f);

    neko_shader_set_v3f(sid, "u_groundColor", neko_v3(0.1, 0.1, 0.1));

    glBindBuffer(GL_ARRAY_BUFFER, cloud_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CloudVertex) * vertex_count, vertices);

    glBindVertexArray(cloud_vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    vertex_count = 0;
}

DEFINE_IMGUI_BEGIN(template <>, CCloud) {
    ImGuiWrap::Auto(var.pos, "pos");
    ImGuiWrap::Auto(var.size, "size");
}
DEFINE_IMGUI_END()

int Cloud::Inspect(CEntity ent) {
    CCloud *cloud = ComponentGetPtr(ent);
    error_assert(cloud);

    ImGuiWrap::Auto(cloud, "CCloud");
    return 0;
}
