
#pragma once

#include "base/scripting/lua_wrapper.hpp"

#include "engine/draw.h"

namespace Neko::mt_sprite {
void metatable(lua_State *L);
};

namespace Neko::mt_font {
void metatable(lua_State *L);
};

namespace Neko::luabind {
template <>
struct udata<AseSprite> {
    static inline int nupvalue = 1;
    static inline auto metatable = Neko::mt_sprite::metatable;
};
}  // namespace Neko::luabind

namespace Neko::luabind {
template <>
struct udata<FontFamily> {
    static inline int nupvalue = 1;
    static inline auto metatable = Neko::mt_font::metatable;
};
}  // namespace Neko::luabind

int wrap_batch_push_vertex(lua_State *L);
int wrap_batch_texture(lua_State *L);
int wrap_batch_flush(lua_State *L);
int wrap_draw_line(lua_State *L);
int wrap_draw_circle(lua_State *L);
int wrap_draw_capsule(lua_State *L);
int wrap_draw_aabb(lua_State *L);
int wrap_draw_half_circle(lua_State *L);
