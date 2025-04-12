
#pragma once

#include "engine/scripting/lua_wrapper.hpp"

#include "engine/draw.h"
#include "engine/sound.h"

namespace Neko {

namespace FontWrap {
void metatable(lua_State* L);
int open_mt_font(lua_State* L);
int neko_font_load(lua_State* L);
}  // namespace FontWrap

namespace SpriteWrap {
void metatable(lua_State* L);
int open_mt_sprite(lua_State* L);
int neko_sprite_load(lua_State* L);
}  // namespace SpriteWrap

namespace SoundWrap {
void metatable(lua_State* L);
int open_mt_sound(lua_State* L);
int neko_sound_load(lua_State* L);
}  // namespace SoundWrap

}  // namespace Neko

namespace Neko::luabind {
template <>
struct udata<AseSprite> {
    static inline int nupvalue = 1;
    static inline auto metatable = Neko::SpriteWrap::metatable;
};
}  // namespace Neko::luabind

namespace Neko::luabind {
template <>
struct udata<FontFamily> {
    static inline int nupvalue = 1;
    static inline auto metatable = Neko::FontWrap::metatable;
};
}  // namespace Neko::luabind

namespace Neko::luabind {
template <>
struct udata<SoundIndex> {
    static inline int nupvalue = 1;
    static inline auto metatable = Neko::SoundWrap::metatable;
};
}  // namespace Neko::luabind
