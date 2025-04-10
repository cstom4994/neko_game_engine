
#pragma once

#include "engine/ecs/entity.h"
#include "engine/component.h"

struct CSprite : CEntityBase {
    mat3 wmat;  // 要发送到着色器的世界变换矩阵

    vec2 size;
    vec2 texcell;
    vec2 texsize;

    int depth;
};

static_assert(std::is_trivially_copyable_v<CSprite>);

class Sprite : public SingletonClass<Sprite>, public ComponentTypeBase<CSprite> {
public:
    void sprite_init();
    void sprite_fini();
    int sprite_update_all(Event evt);
    void sprite_draw_all();

    void sprite_set_atlas(const char *filename);
    const char *sprite_get_atlas();
    CSprite *sprite_add(CEntity ent);
    void sprite_remove(CEntity ent);
    bool sprite_has(CEntity ent);
    void sprite_set_size(CEntity ent, vec2 size);  // 以世界单位绘制的大小 中心位于变换位置。
    vec2 sprite_get_size(CEntity ent);
    void sprite_set_texcell(CEntity ent, vec2 texcell);  // 图集区域的左下角坐标 以像素为单位
    vec2 sprite_get_texcell(CEntity ent);
    void sprite_set_texsize(CEntity ent, vec2 texsize);  // 图集区域的大小 以像素为单位
    vec2 sprite_get_texsize(CEntity ent);
    void sprite_set_depth(CEntity ent, int depth);  // 较低深度的内容绘制在顶部
    int sprite_get_depth(CEntity ent);

    CSprite *wrap_sprite_add(CEntity ent);

    int Inspect(CEntity ent) override;
};