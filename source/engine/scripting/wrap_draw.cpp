
#include "engine/scripting/wrap_meta.h"

#include "engine/bootstrap.h"

namespace Neko::FontWrap {

static FontFamily &to(lua_State *L, int idx) { return luabind::checkudata<FontFamily>(L, idx); }

static FontFamily &check_font_udata(lua_State *L, i32 arg) {
    FontFamily &udata = to(L, arg);
    return udata;
}

static int mt_font_gc(lua_State *L) {
    FontFamily &font = check_font_udata(L, 1);
    lua_getiuservalue(L, 1, 1);
    String name = luax_check_string(L, -1);
    font.trash();
    destroyudata<FontFamily>(L);
    return 0;
}

static int mt_font_width(lua_State *L) {
    FontFamily &font = check_font_udata(L, 1);

    String text = luax_check_string(L, 2);
    lua_Number size = luaL_checknumber(L, 3);

    float w = font.width(size, text);

    lua_pushnumber(L, w);
    return 1;
}

static int mt_font_draw(lua_State *L) {
    FontFamily &font = check_font_udata(L, 1);

    String text = luax_check_string(L, 2);
    lua_Number x = luaL_optnumber(L, 3, 0);
    lua_Number y = luaL_optnumber(L, 4, 0);
    lua_Number size = luaL_optnumber(L, 5, 12);
    bool draw_in_world = luaL_optnumber(L, 6, 0);
    lua_Number scale = luaL_optnumber(L, 7, 1);
    lua_Number wrap = luaL_optnumber(L, 8, -1);

    float bottom = 0;
    if (wrap < 0) {
        bottom = draw_font(&font, draw_in_world, (u64)size, (float)x, (float)y, text, NEKO_COLOR_WHITE, scale);
    } else {
        bottom = draw_font_wrapped(&font, draw_in_world, (u64)size, (float)x, (float)y, text, NEKO_COLOR_WHITE, (float)wrap, scale);
    }

    lua_pushnumber(L, bottom);
    return 1;
}

int open_mt_font(lua_State *L) { return 0; }

static int mt_close(lua_State *L) {
    auto &self = to(L, 1);
    return 0;
}

void metatable(lua_State *L) {
    static luaL_Reg lib[] = {
            {"width", mt_font_width},
            {"draw", mt_font_draw},
            {nullptr, nullptr},
    };
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__close", mt_close}, {"__gc", mt_font_gc}, {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

int neko_font_load(lua_State *L) {
    String str = luax_opt_string(L, 1, "default");
    if (str == "default") str = the<CL>().state.default_font;
    FontFamily &font = luabind::newudata<FontFamily>(L);
    lua_pushstring(L, str.cstr());
    lua_setiuservalue(L, -2, 1);
    bool ok = font.load(str);
    if (!ok) {
        return 0;
    }
    return 1;
}

}  // namespace Neko::FontWrap

namespace Neko::SpriteWrap {

static AseSprite &to(lua_State *L, int idx) { return luabind::checkudata<AseSprite>(L, idx); }

static AseSprite &check_sprite_udata(lua_State *L, i32 arg) {
    AseSprite &udata = to(L, 1);
    return udata;
}

static int mt_sprite_play(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    String tag = luax_check_string(L, 2);
    bool restart = lua_toboolean(L, 3);

    bool same = spr.play(tag);
    if (!same || restart) {
        spr.current_frame = 0;
        spr.elapsed = 0;
    }
    return 0;
}

static int mt_sprite_update(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    lua_Number dt = luaL_checknumber(L, 2);

    spr.OnPreUpdate((float)dt);
    return 0;
}

static int mt_sprite_draw(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    DrawDescription dd = draw_description_args(L, 2);

    draw_sprite(&spr, &dd);
    return 0;
}

static int mt_sprite_effect(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    int te = lua_type(L, 2);
    if (te == LUA_TSTRING) {
        const char *bits = lua_tostring(L, 2);
        spr.effects = Neko::util::BitSet<SpriteEffectCounts>(bits);
    } else if (te == LUA_TNUMBER) {
        int bits = lua_tointeger(L, 2);
        spr.effects = std::bitset<SpriteEffectCounts>(bits);
    } else {
        return luaL_error(L, "error mt_sprite_effect bitset type");
    }
    return 0;
}

static int mt_sprite_effect_pixelate(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    int te = lua_type(L, 2);
    if (te == LUA_TNUMBER) {
        spr.pixelate_value = lua_tonumber(L, 2);
    } else {
        return luaL_error(L, "error mt_sprite_effect_pixelate arg type");
    }
    return 0;
}

static int mt_sprite_width(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    AseSpriteData data = assets_get<AseSpriteData>(check_asset(L, spr.sprite));

    lua_pushnumber(L, (lua_Number)data.width);
    return 1;
}

static int mt_sprite_height(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    AseSpriteData data = assets_get<AseSpriteData>(check_asset(L, spr.sprite));

    lua_pushnumber(L, (lua_Number)data.height);
    return 1;
}

static int mt_sprite_set_frame(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    lua_Integer frame = luaL_checknumber(L, 2);

    spr.set_frame((i32)frame);
    return 0;
}

static int mt_sprite_total_frames(lua_State *L) {
    AseSprite &spr = check_sprite_udata(L, 1);
    AseSpriteData data = assets_get<AseSpriteData>(check_asset(L, spr.sprite));

    lua_pushinteger(L, data.frames.len);
    return 1;
}

int open_mt_sprite(lua_State *L) { return 0; }

static int mt_close(lua_State *L) {
    auto &self = to(L, 1);
    return 0;
}

static int mt_sprite_gc(lua_State *L) {
    auto &self = to(L, 1);
    destroyudata<AseSprite>(L);
    return 0;
}

void metatable(lua_State *L) {
    // clang-format off
    static luaL_Reg lib[] = {
        {"play", mt_sprite_play},   
        {"update", mt_sprite_update}, 
        {"draw", mt_sprite_draw},           
        {"effect", mt_sprite_effect},
        {"effect_pixelate", mt_sprite_effect_pixelate},
        {"width", mt_sprite_width},
        {"height", mt_sprite_height}, 
        {"set_frame", mt_sprite_set_frame}, 
        {"total_frames", mt_sprite_total_frames},
        {nullptr, nullptr},
    };
    // clang-format on
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__close", mt_close}, {"__gc", mt_sprite_gc}, {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

int neko_sprite_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    Asset asset = {};
    bool ok = asset_load_kind(AssetKind_AseSprite, str, &asset);
    if (!ok) {
        return 0;
    }

    AseSprite &spr = luabind::newudata<AseSprite>(L);
    lua_pushstring(L, str.cstr());
    lua_setiuservalue(L, -2, 1);

    spr.sprite = asset.hash;
    spr.make();

    return 1;
}

}  // namespace Neko::SpriteWrap
