
#pragma once

#include "engine/ecs/entity.h"
#include "engine/component.h"

struct RectTexVertex {
    float position[2];
    float texcoord[2];
};

struct CRectangle : CEntityBase {
    vec2 pos;
    vec2 size;
};

static_assert(std::is_trivially_copyable_v<CRectangle>);

class RectangleBox : public SingletonClass<RectangleBox>, public ComponentTypeBase<CRectangle> {

    // Asset rectangle_shader{};

    GLuint rectangle_vao;
    GLuint rectangle_vbo;

    int vertex_capacity{256};
    RectTexVertex *vertices;
    int vertex_count;

public:
    CRectangle *ComponentAdd(CEntity ent) override;
    void ComponentRemove(CEntity ent) override;

    void init();
    void fini();
    void push_vertex(float x, float y, float u, float v);
    int update_all(Event evt);
    void draw(const AssetShader &shader, std::function<void(void)> setter);

    int Inspect(CEntity ent) override;
};