
#pragma once

#include "engine/ecs/entity.h"
#include "engine/component.h"

struct CUnEditable : CEntityBase {
    int what;
};

static_assert(std::is_trivially_copyable_v<CUnEditable>);

// bbox pool
struct CBBoxPoolElem : CEntityBase {
    mat3 wmat;
    BBox bbox;
    f32 selected;  // > 0.5 if and only if selected
};

static_assert(std::is_trivially_copyable_v<CBBoxPoolElem>);

void edit_init_impl(lua_State *L);
void edit_fini_impl();
void edit_update_impl();
void edit_draw_impl();
