
#include "rectangle.h"

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"
#include "engine/components/transform.h"
#include "engine/components/camera.h"

CRectangle *RectangleBox::ComponentAdd(CEntity ent) {

    CRectangle *rectangle = ComponentGetPtr(ent);
    if (rectangle) return rectangle;

    // the<Transform>().Add(ent);

    rectangle = ComponentTypeBase::EntityPool->Add(ent);

    rectangle->pos = neko_v2(0, 0);
    rectangle->size = neko_v2(500, 300);

    return rectangle;
}

void RectangleBox::ComponentRemove(CEntity ent) { ComponentTypeBase::EntityPool->Remove(ent); }

void RectangleBox::init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    ComponentTypeBase::ComponentReg();

    // GLuint sid = assets_get<AssetShader>(rectangle_shader).id;

    glGenVertexArrays(1, &rectangle_vao);
    glBindVertexArray(rectangle_vao);

    glGenBuffers(1, &rectangle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rectangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectTexVertex) * vertex_capacity, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RectTexVertex), (void *)offsetof(RectTexVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RectTexVertex), (void *)offsetof(RectTexVertex, texcoord));

    vertices = (RectTexVertex *)mem_alloc(sizeof(RectTexVertex) * vertex_capacity);

    // clang-format off

    auto type = BUILD_TYPE(RectangleBox)
        .MemberMethod<ComponentTypeBase>("rectangle_add", this, &ComponentTypeBase::WrapAdd)
        .MemberMethod<ComponentTypeBase>("rectangle_has", this, &ComponentTypeBase::ComponentHas)
        .MemberMethod("rectangle_remove", this, &RectangleBox::ComponentRemove)
        .Build();

    // clang-format on
}

void RectangleBox::fini() {

    glDeleteBuffers(1, &rectangle_vbo);
    glDeleteVertexArrays(1, &rectangle_vao);

    entitypool_free(ComponentTypeBase::EntityPool);

    mem_free(vertices);
}

void RectangleBox::push_vertex(float x, float y, float u, float v) {
    if (vertex_count == vertex_capacity) {
        neko_assert(0);
    }

    vertices[vertex_count++] = RectTexVertex{
            .position = {x, y},
            .texcoord = {u, v},
    };
}

int RectangleBox::update_all(Event evt) {

    entitypool_remove_destroyed(ComponentTypeBase::EntityPool, [this](CEntity ent) { ComponentRemove(ent); });

    ComponentTypeBase::EntityPool->ForEach([this](CRectangle *rectangle) {
        float x1 = rectangle->pos.x;
        float y1 = -rectangle->pos.y;
        float x2 = x1 + rectangle->size.x;
        float y2 = y1 - rectangle->size.y;

        float u1 = 0.f;
        float v1 = 0.f;
        float u2 = 1.f;
        float v2 = 1.f;

        push_vertex(x1, y1, u1, v2);
        push_vertex(x2, y2, u2, v1);
        push_vertex(x1, y2, u1, v1);

        push_vertex(x1, y1, u1, v2);
        push_vertex(x2, y1, u2, v2);
        push_vertex(x2, y2, u2, v1);
    });

    return 0;
}

void RectangleBox::draw(const AssetShader &shader, std::function<void(void)> setter) {

    GLuint sid = shader.id;

    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)the<Camera>().GetInverseViewMatrixPtr());

    setter();

    glBindBuffer(GL_ARRAY_BUFFER, rectangle_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RectTexVertex) * vertex_count, vertices);

    glBindVertexArray(rectangle_vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    vertex_count = 0;
}

DEFINE_IMGUI_BEGIN(template <>, CRectangle) {
    ImGuiWrap::Auto(var.pos, "pos");
    ImGuiWrap::Auto(var.size, "size");
}
DEFINE_IMGUI_END()

int RectangleBox::Inspect(CEntity ent) {
    CRectangle *rectangle = ComponentGetPtr(ent);
    error_assert(rectangle);

    ImGuiWrap::Auto(rectangle, "CRectangle");
    return 0;
}
