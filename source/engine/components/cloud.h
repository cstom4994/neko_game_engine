
#pragma once

#include "engine/ecs/entity.h"
#include "engine/component.h"

struct CloudVertex {
    float position[2];
    float texcoord[2];
};

struct CCloud : CEntityBase {
    vec2 pos;
    vec2 size;
};

static_assert(std::is_trivially_copyable_v<CCloud>);

class Cloud : public SingletonClass<Cloud>, public ComponentTypeBase<CCloud> {
    Asset cloud_shader{};
    GLuint cloud_vao;
    GLuint cloud_vbo;

    int vertex_capacity{256};
    CloudVertex *vertices;
    int vertex_count;

public:
    CCloud *Add(CEntity ent);
    void Remove(CEntity ent);
    bool Has(CEntity ent);

    CCloud *wrap_add(CEntity ent);

    void cloud_init();
    void cloud_fini();
    void cloud_push_vertex(float x, float y, float u, float v);
    int cloud_update_all(Event evt);
    void cloud_draw_all();

    int Inspect(CEntity ent) override;
};