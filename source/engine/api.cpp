#include "engine/api.hpp"

#include <box2d/box2d.h>

#include "engine/api.hpp"
#include "engine/asset.h"
#include "engine/base.h"
#include "engine/draw.h"
#include "engine/ecs.h"
#include "engine/game.h"
#include "engine/lua_custom_types.h"
#include "engine/lua_struct.h"
#include "engine/lua_util.h"
#include "engine/luabind.hpp"
#include "engine/luax.h"
#include "engine/os.h"
#include "engine/pak.h"
#include "engine/physics.h"
#include "engine/prelude.h"
#include "engine/reflection.hpp"
#include "engine/seri.h"
#include "engine/sound.h"
#include "engine/ui.h"
#include "vendor/sokol_time.h"

// mt_sampler

static int mt_sampler_gc(lua_State *L) {
    u32 *udata = (u32 *)luaL_checkudata(L, 1, "mt_sampler");
    u32 id = *udata;

    // if (id != SG_INVALID_ID) {
    //     sg_destroy_sampler({id});
    // }

    return 0;
}

static int mt_sampler_use(lua_State *L) {
    u32 *id = (u32 *)luaL_checkudata(L, 1, "mt_sampler");
    renderer_use_sampler(*id);
    return 0;
}

static int open_mt_sampler(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_sampler_gc},
            {"use", mt_sampler_use},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_sampler", reg);
    return 0;
}

// mt_thread

static int mt_thread_join(lua_State *L) {
    LuaThread *lt = *(LuaThread **)luaL_checkudata(L, 1, "mt_thread");
    lt->join();
    mem_free(lt);
    return 0;
}

static int open_mt_thread(lua_State *L) {
    luaL_Reg reg[] = {
            {"join", mt_thread_join},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_thread", reg);
    return 0;
}

// mt_channel

static LuaChannel *check_channel_udata(lua_State *L, i32 arg) {
    LuaChannel *chan = *(LuaChannel **)luaL_checkudata(L, arg, "mt_channel");
    return chan;
}

static int mt_channel_send(lua_State *L) {
    LuaChannel *chan = check_channel_udata(L, 1);

    LuaVariant v = {};
    v.make(L, 2);
    chan->send(v);

    return 0;
}

static int mt_channel_recv(lua_State *L) {
    LuaChannel *chan = check_channel_udata(L, 1);
    LuaVariant v = chan->recv();

    v.push(L);
    v.trash();
    return 1;
}

static int mt_channel_try_recv(lua_State *L) {
    LuaChannel *chan = check_channel_udata(L, 1);
    LuaVariant v = {};
    bool ok = chan->try_recv(&v);
    if (!ok) {
        lua_pushnil(L);
        lua_pushboolean(L, false);
        return 2;
    }

    v.push(L);
    v.trash();
    lua_pushboolean(L, true);
    return 2;
}

static int open_mt_channel(lua_State *L) {
    luaL_Reg reg[] = {
            {"send", mt_channel_send},
            {"recv", mt_channel_recv},
            {"try_recv", mt_channel_try_recv},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_channel", reg);
    return 0;
}

#if 0

// mt_image

static int mt_image_draw(lua_State *L) {
    Image img = check_asset_mt(L, 1, "mt_image").image;

    DrawDescription dd = draw_description_args(L, 2);
    draw_image(&img, &dd);
    return 0;
}

static int mt_image_width(lua_State *L) {
    Image img = check_asset_mt(L, 1, "mt_image").image;
    lua_pushnumber(L, img.width);
    return 1;
}

static int mt_image_height(lua_State *L) {
    Image img = check_asset_mt(L, 1, "mt_image").image;
    lua_pushnumber(L, img.height);
    return 1;
}

static int open_mt_image(lua_State *L) {
    luaL_Reg reg[] = {
            {"draw", mt_image_draw},
            {"width", mt_image_width},
            {"height", mt_image_height},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_image", reg);
    return 0;
}

// mt_font

static FontFamily *check_font_udata(lua_State *L, i32 arg) {
    FontFamily **udata = (FontFamily **)luaL_checkudata(L, arg, "mt_font");
    FontFamily *font = *udata;
    return font;
}

static int mt_font_gc(lua_State *L) {
    FontFamily *font = check_font_udata(L, 1);

    if (font != g_app->default_font) {
        font->trash();
        mem_free(font);
    }
    return 0;
}

static int mt_font_width(lua_State *L) {
    FontFamily *font = check_font_udata(L, 1);

    String text = luax_check_string(L, 2);
    lua_Number size = luaL_checknumber(L, 3);

    float w = font->width(size, text);

    lua_pushnumber(L, w);
    return 1;
}

static int mt_font_draw(lua_State *L) {
    FontFamily *font = check_font_udata(L, 1);

    String text = luax_check_string(L, 2);
    lua_Number x = luaL_optnumber(L, 3, 0);
    lua_Number y = luaL_optnumber(L, 4, 0);
    lua_Number size = luaL_optnumber(L, 5, 12);
    lua_Number wrap = luaL_optnumber(L, 6, -1);

    float bottom = 0;
    if (wrap < 0) {
        bottom = draw_font(font, (u64)size, (float)x, (float)y, text);
    } else {
        bottom = draw_font_wrapped(font, (u64)size, (float)x, (float)y, text, (float)wrap);
    }

    lua_pushnumber(L, bottom);
    return 1;
}

static int open_mt_font(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_font_gc},
            {"width", mt_font_width},
            {"draw", mt_font_draw},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_font", reg);
    return 0;
}

// mt_sprite

static Sprite *check_sprite_udata(lua_State *L, i32 arg) {
    Sprite *spr = (Sprite *)luaL_checkudata(L, arg, "mt_sprite");
    return spr;
}

static int mt_sprite_play(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    String tag = luax_check_string(L, 2);
    bool restart = lua_toboolean(L, 3);

    bool same = spr->play(tag);
    if (!same || restart) {
        spr->current_frame = 0;
        spr->elapsed = 0;
    }
    return 0;
}

static int mt_sprite_update(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    lua_Number dt = luaL_checknumber(L, 2);

    spr->update((float)dt);
    return 0;
}

static int mt_sprite_draw(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    DrawDescription dd = draw_description_args(L, 2);

    draw_sprite(spr, &dd);
    return 0;
}

static int mt_sprite_width(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    SpriteData data = check_asset(L, spr->sprite).sprite;

    lua_pushnumber(L, (lua_Number)data.width);
    return 1;
}

static int mt_sprite_height(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    SpriteData data = check_asset(L, spr->sprite).sprite;

    lua_pushnumber(L, (lua_Number)data.height);
    return 1;
}

static int mt_sprite_set_frame(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    lua_Integer frame = luaL_checknumber(L, 2);

    spr->set_frame((i32)frame);
    return 0;
}

static int mt_sprite_total_frames(lua_State *L) {
    Sprite *spr = check_sprite_udata(L, 1);
    SpriteData data = check_asset(L, spr->sprite).sprite;

    lua_pushinteger(L, data.frames.len);
    return 1;
}

static int open_mt_sprite(lua_State *L) {
    luaL_Reg reg[] = {
            {"play", mt_sprite_play},
            {"update", mt_sprite_update},
            {"draw", mt_sprite_draw},
            {"width", mt_sprite_width},
            {"height", mt_sprite_height},
            {"set_frame", mt_sprite_set_frame},
            {"total_frames", mt_sprite_total_frames},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_sprite", reg);
    return 0;
}

// mt_atlas_image

static AtlasImage *check_atlas_image_udata(lua_State *L, i32 arg) {
    AtlasImage *atlas_img = (AtlasImage *)luaL_checkudata(L, arg, "mt_atlas_image");
    return atlas_img;
}

static int mt_atlas_image_draw(lua_State *L) {
    AtlasImage *atlas_img = check_atlas_image_udata(L, 1);
    DrawDescription dd = draw_description_args(L, 2);

    dd.u0 = atlas_img->u0;
    dd.v0 = atlas_img->v0;
    dd.u1 = atlas_img->u1;
    dd.v1 = atlas_img->v1;

    draw_image(&atlas_img->img, &dd);
    return 0;
}

static int mt_atlas_image_width(lua_State *L) {
    AtlasImage *atlas_img = check_atlas_image_udata(L, 1);
    lua_pushnumber(L, atlas_img->width);
    return 1;
}

static int mt_atlas_image_height(lua_State *L) {
    AtlasImage *atlas_img = check_atlas_image_udata(L, 1);
    lua_pushnumber(L, atlas_img->height);
    return 1;
}

static int open_mt_atlas_image(lua_State *L) {
    luaL_Reg reg[] = {
            {"draw", mt_atlas_image_draw},
            {"width", mt_atlas_image_width},
            {"height", mt_atlas_image_height},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_atlas_image", reg);
    return 0;
}

// mt_atlas

static int mt_atlas_gc(lua_State *L) {
    Atlas *atlas = (Atlas *)luaL_checkudata(L, 1, "mt_atlas");
    atlas->trash();
    return 0;
}

static int mt_atlas_get_image(lua_State *L) {
    Atlas *atlas = (Atlas *)luaL_checkudata(L, 1, "mt_atlas");
    String name = luax_check_string(L, 2);

    AtlasImage *atlas_img = atlas->get(name);
    if (atlas_img == nullptr) {
        return 0;
    }

    luax_new_userdata(L, *atlas_img, "mt_atlas_image");
    return 1;
}

static int open_mt_atlas(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_atlas_gc},
            {"get_image", mt_atlas_get_image},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_atlas", reg);
    return 0;
}

// mt_tilemap

static int mt_tilemap_draw(lua_State *L) {
    MapLdtk tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;
    draw_tilemap(&tm);
    return 0;
}

static int mt_tilemap_entities(lua_State *L) {
    MapLdtk tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;

    u64 entities = 0;
    for (TilemapLevel &level : tm.levels) {
        for (TilemapLayer &layer : level.layers) {
            entities += layer.entities.len;
        }
    }

    lua_createtable(L, (i32)entities, 0);

    i32 i = 1;
    for (TilemapLevel &level : tm.levels) {
        for (TilemapLayer &layer : level.layers) {
            for (TilemapEntity &entity : layer.entities) {
                lua_createtable(L, 0, 3);

                luax_set_string_field(L, "id", entity.identifier.data);
                luax_set_number_field(L, "x", entity.x + level.world_x);
                luax_set_number_field(L, "y", entity.y + level.world_y);

                lua_rawseti(L, -2, i);
                i++;
            }
        }
    }

    return 1;
}

static int mt_tilemap_make_collision(lua_State *L) {
    Asset asset = check_asset_mt(L, 1, "mt_tilemap");

    Physics *physics = (Physics *)luaL_checkudata(L, 2, "mt_b2_world");
    String name = luax_check_string(L, 3);

    Array<TilemapInt> walls = {};
    neko_defer(walls.trash());

    walls.reserve(luax_len(L, 4));
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        lua_Number tile = luaL_checknumber(L, -1);
        walls.push((TilemapInt)tile);
    }

    asset.tilemap.make_collision(physics->world, physics->meter, name, Slice(walls));
    asset_write(asset);
    return 0;
}

static int mt_tilemap_draw_fixtures(lua_State *L) {
    MapLdtk tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;
    Physics *physics = (Physics *)luaL_checkudata(L, 2, "mt_b2_world");
    String name = luax_check_string(L, 3);

    b2Body **body = tm.bodies.get(fnv1a(name));
    if (body != nullptr) {
        draw_fixtures_for_body(*body, physics->meter);
    }

    return 0;
}

static int mt_tilemap_make_graph(lua_State *L) {
    Asset asset = check_asset_mt(L, 1, "mt_tilemap");

    String name = luax_check_string(L, 2);
    i32 bloom = (i32)luaL_optnumber(L, 4, 1);

    Array<TileCost> costs = {};
    neko_defer(costs.trash());

    lua_pushvalue(L, 3);

    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        lua_Number value = luaL_checknumber(L, -1);
        lua_Number key = luaL_checknumber(L, -2);

        TileCost cost = {};
        cost.cell = (TilemapInt)key;
        cost.value = (float)value;

        costs.push(cost);
    }
    lua_pop(L, 1);

    asset.tilemap.make_graph(bloom, name, Slice(costs));
    asset_write(asset);
    return 0;
}

static int mt_tilemap_astar(lua_State *L) {
    PROFILE_FUNC();

    Asset asset = check_asset_mt(L, 1, "mt_tilemap");
    neko_defer(asset_write(asset));

    lua_Number sx = luaL_checknumber(L, 2);
    lua_Number sy = luaL_checknumber(L, 3);
    lua_Number ex = luaL_checknumber(L, 4);
    lua_Number ey = luaL_checknumber(L, 5);

    TilePoint start = {};
    start.x = (i32)sx;
    start.y = (i32)sy;

    TilePoint goal = {};
    goal.x = (i32)ex;
    goal.y = (i32)ey;

    TileNode *end = asset.tilemap.astar(goal, start);

    {
        PROFILE_BLOCK("construct path");

        lua_newtable(L);

        i32 i = 1;
        for (TileNode *n = end; n != nullptr; n = n->prev) {
            lua_createtable(L, 0, 2);

            luax_set_number_field(L, "x", n->x * asset.tilemap.graph_grid_size);
            luax_set_number_field(L, "y", n->y * asset.tilemap.graph_grid_size);

            lua_rawseti(L, -2, i);
            i++;
        }
    }

    return 1;
}

static int open_mt_tilemap(lua_State *L) {
    luaL_Reg reg[] = {
            {"draw", mt_tilemap_draw},
            {"entities", mt_tilemap_entities},
            {"make_collision", mt_tilemap_make_collision},
            {"draw_fixtures", mt_tilemap_draw_fixtures},
            {"make_graph", mt_tilemap_make_graph},
            {"astar", mt_tilemap_astar},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_tilemap", reg);
    return 0;
}

#endif

// mt_pak

struct pak_assets_t {
    const_str name;
    size_t size;
    String data;
};

static neko_pak *check_pak_udata(lua_State *L, i32 arg) {
    neko_pak *udata = (neko_pak *)luaL_checkudata(L, arg, "mt_pak");
    if (!udata || !udata->item_count) {
        luaL_error(L, "cannot read pak");
    }
    return udata;
}

static int mt_pak_gc(lua_State *L) {
    neko_pak *pak = check_pak_udata(L, 1);
    pak->fini();
    console_log("pak __gc %p", pak);
    return 0;
}

static int mt_pak_items(lua_State *L) {
    neko_pak *pak = check_pak_udata(L, 1);

    u64 item_count = pak->get_item_count();

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, pak->get_item_path(i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int mt_pak_assets_load(lua_State *L) {
    neko_pak *pak = check_pak_udata(L, 1);

    const_str path = lua_tostring(L, 2);

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_newuserdata(L, sizeof(pak_assets_t));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    bool ok = pak->get_data(path, &assets_user_handle->data, (u32 *)&assets_user_handle->size);

    if (!ok) {
        const_str error_message = "mt_pak_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // asset_write(asset);

    return 1;
}

static int mt_pak_assets_unload(lua_State *L) {
    neko_pak *pak = check_pak_udata(L, 1);

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_touserdata(L, 2);

    if (assets_user_handle && assets_user_handle->data.len)
        pak->free_item(assets_user_handle->data);
    else
        console_log("unknown assets unload %p", assets_user_handle);

    // asset_write(asset);

    return 0;
}

static int open_mt_pak(lua_State *L) {
    // clang-format off
    luaL_Reg reg[] = {
            {"__gc", mt_pak_gc},
            {"items", mt_pak_items},
            {"assets_load", mt_pak_assets_load},
            {"assets_unload", mt_pak_assets_unload},
            {nullptr, nullptr},
    };
    // clang-format on

    luax_new_class(L, "mt_pak", reg);
    return 0;
}

// neko api

static int neko_registry_load(lua_State *L) {
    if (lua_gettop(L) < 2) {
        luaL_error(L, "expected a name and a value");
    }

    String name = luax_check_string(L, 1);

    // registry._LOADED
    luax_pushloadedtable(L);

    if (lua_type(L, 2) == LUA_TNIL) {
        lua_pushboolean(L, true);
    } else {
        lua_pushvalue(L, 2);
    }
    // _LOADED[name] = value
    lua_setfield(L, -2, name.data);

    lua_pushvalue(L, 2);
    return 1;
}

static int neko_require_lua_script(lua_State *L) {
    PROFILE_FUNC();

    String path = luax_check_string(L, 1);

    Asset asset = {};
    bool ok = asset_load_kind(AssetKind_LuaRef, path, &asset);
    if (!ok) {
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, asset.lua_ref);
    return 1;
}

static int neko_version(lua_State *L) {
    lua_pushinteger(L, neko_buildnum());
    return 1;
}

static int neko_set_console_window(lua_State *L) {
    g_app->win_console = lua_toboolean(L, 1);
    return 0;
}

static int neko_quit(lua_State *L) {
    (void)L;
    // sapp_request_quit();
    return 0;
}

static int neko_fatal_error(lua_State *L) {
    String msg = luax_check_string(L, 1);
    fatal_error(msg);
    return 0;
}

static int neko_platform(lua_State *L) {
#if defined(NEKO_IS_WEB)
    lua_pushliteral(L, "web");
#elif defined(NEKO_IS_WIN32)
    lua_pushliteral(L, "windows");
#elif defined(NEKO_IS_LINUX)
    lua_pushliteral(L, "linux");
#endif
    return 1;
}

static int neko_dt(lua_State *L) {
    lua_pushnumber(L, g_app->time.delta);
    return 1;
}

static int neko_fullscreen(lua_State *L) {
    // lua_pushboolean(L, sapp_is_fullscreen());
    return 1;
}

static int neko_toggle_fullscreen(lua_State *L) {
    // sapp_toggle_fullscreen();
    return 0;
}

static int neko_window_width(lua_State *L) {
    // float width = sapp_widthf();
    // lua_pushnumber(L, width);
    return 1;
}

static int neko_window_height(lua_State *L) {
    // float height = sapp_heightf();
    // lua_pushnumber(L, height);
    return 1;
}

static int neko_time(lua_State *L) {
    lua_pushinteger(L, stm_now());
    return 1;
}

static int neko_difftime(lua_State *L) {
    lua_Integer t2 = luaL_checkinteger(L, 1);
    lua_Integer t1 = luaL_checkinteger(L, 2);

    lua_pushinteger(L, stm_diff(t2, t1));
    return 1;
}

static int neko_elapsed(lua_State *L) {
    lua_pushnumber(L, stm_sec(stm_now() - g_app->time.startup));
    return 1;
}

static int neko_json_read(lua_State *L) {
    PROFILE_FUNC();

    String str = luax_check_string(L, 1);

    JSONDocument doc = {};
    doc.parse(str);
    neko_defer(doc.trash());

    if (doc.error.len != 0) {
        lua_pushnil(L);
        lua_pushlstring(L, doc.error.data, doc.error.len);
        return 2;
    }

    {
        PROFILE_BLOCK("json to lua");
        json_to_lua(L, &doc.root);
    }
    return 1;
}

static int neko_json_write(lua_State *L) {
    PROFILE_FUNC();

    lua_Integer width = luaL_optinteger(L, 2, 0);

    String contents = {};
    String err = lua_to_json_string(L, 1, &contents, width);
    if (err.len != 0) {
        lua_pushnil(L);
        lua_pushlstring(L, err.data, err.len);
        return 2;
    }

    lua_pushlstring(L, contents.data, contents.len);
    mem_free(contents.data);
    return 1;
}

static i32 keyboard_lookup(String str) {
    switch (fnv1a(str)) {
        case "space"_hash:
            return 32;
        case "'"_hash:
            return 39;
        case ","_hash:
            return 44;
        case "-"_hash:
            return 45;
        case "."_hash:
            return 46;
        case "/"_hash:
            return 47;
        case "0"_hash:
            return 48;
        case "1"_hash:
            return 49;
        case "2"_hash:
            return 50;
        case "3"_hash:
            return 51;
        case "4"_hash:
            return 52;
        case "5"_hash:
            return 53;
        case "6"_hash:
            return 54;
        case "7"_hash:
            return 55;
        case "8"_hash:
            return 56;
        case "9"_hash:
            return 57;
        case ";"_hash:
            return 59;
        case "="_hash:
            return 61;
        case "a"_hash:
            return 65;
        case "b"_hash:
            return 66;
        case "c"_hash:
            return 67;
        case "d"_hash:
            return 68;
        case "e"_hash:
            return 69;
        case "f"_hash:
            return 70;
        case "g"_hash:
            return 71;
        case "h"_hash:
            return 72;
        case "i"_hash:
            return 73;
        case "j"_hash:
            return 74;
        case "k"_hash:
            return 75;
        case "l"_hash:
            return 76;
        case "m"_hash:
            return 77;
        case "n"_hash:
            return 78;
        case "o"_hash:
            return 79;
        case "p"_hash:
            return 80;
        case "q"_hash:
            return 81;
        case "r"_hash:
            return 82;
        case "s"_hash:
            return 83;
        case "t"_hash:
            return 84;
        case "u"_hash:
            return 85;
        case "v"_hash:
            return 86;
        case "w"_hash:
            return 87;
        case "x"_hash:
            return 88;
        case "y"_hash:
            return 89;
        case "z"_hash:
            return 90;
        case "["_hash:
            return 91;
        case "\\"_hash:
            return 92;
        case "]"_hash:
            return 93;
        case "`"_hash:
            return 96;
        case "world_1"_hash:
            return 161;
        case "world_2"_hash:
            return 162;
        case "esc"_hash:
            return 256;
        case "enter"_hash:
            return 257;
        case "tab"_hash:
            return 258;
        case "backspace"_hash:
            return 259;
        case "insert"_hash:
            return 260;
        case "delete"_hash:
            return 261;
        case "right"_hash:
            return 262;
        case "left"_hash:
            return 263;
        case "down"_hash:
            return 264;
        case "up"_hash:
            return 265;
        case "pg_up"_hash:
            return 266;
        case "pg_down"_hash:
            return 267;
        case "home"_hash:
            return 268;
        case "end"_hash:
            return 269;
        case "caps_lock"_hash:
            return 280;
        case "scroll_lock"_hash:
            return 281;
        case "num_lock"_hash:
            return 282;
        case "print_screen"_hash:
            return 283;
        case "pause"_hash:
            return 284;
        case "f1"_hash:
            return 290;
        case "f2"_hash:
            return 291;
        case "f3"_hash:
            return 292;
        case "f4"_hash:
            return 293;
        case "f5"_hash:
            return 294;
        case "f6"_hash:
            return 295;
        case "f7"_hash:
            return 296;
        case "f8"_hash:
            return 297;
        case "f9"_hash:
            return 298;
        case "f10"_hash:
            return 299;
        case "f11"_hash:
            return 300;
        case "f12"_hash:
            return 301;
        case "f13"_hash:
            return 302;
        case "f14"_hash:
            return 303;
        case "f15"_hash:
            return 304;
        case "f16"_hash:
            return 305;
        case "f17"_hash:
            return 306;
        case "f18"_hash:
            return 307;
        case "f19"_hash:
            return 308;
        case "f20"_hash:
            return 309;
        case "f21"_hash:
            return 310;
        case "f22"_hash:
            return 311;
        case "f23"_hash:
            return 312;
        case "f24"_hash:
            return 313;
        case "f25"_hash:
            return 314;
        case "kp0"_hash:
            return 320;
        case "kp1"_hash:
            return 321;
        case "kp2"_hash:
            return 322;
        case "kp3"_hash:
            return 323;
        case "kp4"_hash:
            return 324;
        case "kp5"_hash:
            return 325;
        case "kp6"_hash:
            return 326;
        case "kp7"_hash:
            return 327;
        case "kp8"_hash:
            return 328;
        case "kp9"_hash:
            return 329;
        case "kp."_hash:
            return 330;
        case "kp/"_hash:
            return 331;
        case "kp*"_hash:
            return 332;
        case "kp-"_hash:
            return 333;
        case "kp+"_hash:
            return 334;
        case "kp_enter"_hash:
            return 335;
        case "kp="_hash:
            return 336;
        case "lshift"_hash:
            return 340;
        case "lctrl"_hash:
            return 341;
        case "lalt"_hash:
            return 342;
        case "lsuper"_hash:
            return 343;
        case "rshift"_hash:
            return 344;
        case "rctrl"_hash:
            return 345;
        case "ralt"_hash:
            return 346;
        case "rsuper"_hash:
            return 347;
        case "menu"_hash:
            return 348;
        default:
            return 0;
    }
}

static int neko_key_down(lua_State *L) {
    String str = luax_check_string(L, 1);
    i32 key = keyboard_lookup(str);
    bool is_down = g_app->key_state[key];
    lua_pushboolean(L, is_down);
    return 1;
}

static int neko_key_release(lua_State *L) {
    String str = luax_check_string(L, 1);
    i32 key = keyboard_lookup(str);
    bool is_release = !g_app->key_state[key] && g_app->prev_key_state[key];
    lua_pushboolean(L, is_release);
    return 1;
}

static int neko_key_press(lua_State *L) {
    String str = luax_check_string(L, 1);
    i32 key = keyboard_lookup(str);
    bool is_press = g_app->key_state[key] && !g_app->prev_key_state[key];
    lua_pushboolean(L, is_press);
    return 1;
}

static int neko_mouse_down(lua_State *L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    if (n >= 0 && n < array_size(g_app->mouse_state)) {
        lua_pushboolean(L, g_app->mouse_state[n]);
    } else {
        lua_pushboolean(L, false);
    }

    return 1;
}

static int neko_mouse_release(lua_State *L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    if (n >= 0 && n < array_size(g_app->mouse_state)) {
        bool is_release = !g_app->mouse_state[n] && g_app->prev_mouse_state[n];
        lua_pushboolean(L, is_release);
    } else {
        lua_pushboolean(L, false);
    }

    return 1;
}

static int neko_mouse_click(lua_State *L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    if (n >= 0 && n < array_size(g_app->mouse_state)) {
        bool is_click = g_app->mouse_state[n] && !g_app->prev_mouse_state[n];
        lua_pushboolean(L, is_click);
    } else {
        lua_pushboolean(L, false);
    }

    return 1;
}

static int neko_mouse_pos(lua_State *L) {
    lua_pushnumber(L, g_app->mouse_x);
    lua_pushnumber(L, g_app->mouse_y);
    return 2;
}

static int neko_mouse_delta(lua_State *L) {
    lua_pushnumber(L, g_app->mouse_x - g_app->prev_mouse_x);
    lua_pushnumber(L, g_app->mouse_y - g_app->prev_mouse_y);
    return 2;
}

static int neko_show_mouse(lua_State *L) {
    bool show = lua_toboolean(L, 1);
    // sapp_show_mouse(show);
    return 0;
}

static int neko_scroll_wheel(lua_State *L) {
    lua_pushnumber(L, g_app->scroll_x);
    lua_pushnumber(L, g_app->scroll_y);
    return 2;
}

static int neko_scissor_rect(lua_State *L) {
    lua_Number x = luaL_optnumber(L, 1, 0);
    lua_Number y = luaL_optnumber(L, 2, 0);
    // lua_Number w = luaL_optnumber(L, 3, sapp_widthf());
    // lua_Number h = luaL_optnumber(L, 4, sapp_heightf());

    // sgl_scissor_rectf(x, y, w, h, true);
    return 0;
}

static int neko_push_matrix(lua_State *L) {
    bool ok = renderer_push_matrix();
    return ok ? 0 : luaL_error(L, "matrix stack is full");
    return 0;
}

static int neko_pop_matrix(lua_State *L) {
    bool ok = renderer_pop_matrix();
    return ok ? 0 : luaL_error(L, "matrix stack is full");
    return 0;
}

static int neko_translate(lua_State *L) {
    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);

    renderer_translate((float)x, (float)y);
    return 0;
}

static int neko_rotate(lua_State *L) {
    lua_Number angle = luaL_checknumber(L, 1);

    renderer_rotate((float)angle);
    return 0;
}

static int neko_scale(lua_State *L) {
    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);

    renderer_scale((float)x, (float)y);
    return 0;
}

static int neko_clear_color(lua_State *L) {
    lua_Number r = luaL_checknumber(L, 1);
    lua_Number g = luaL_checknumber(L, 2);
    lua_Number b = luaL_checknumber(L, 3);
    lua_Number a = luaL_checknumber(L, 4);

    float rgba[4] = {
            (float)r / 255.0f,
            (float)g / 255.0f,
            (float)b / 255.0f,
            (float)a / 255.0f,
    };
    renderer_set_clear_color(rgba);

    return 0;
}

static int neko_push_color(lua_State *L) {
    lua_Number r = luaL_checknumber(L, 1);
    lua_Number g = luaL_checknumber(L, 2);
    lua_Number b = luaL_checknumber(L, 3);
    lua_Number a = luaL_checknumber(L, 4);

    Color color = {};
    color.r = (u8)r;
    color.g = (u8)g;
    color.b = (u8)b;
    color.a = (u8)a;

    bool ok = renderer_push_color(color);
    return ok ? 0 : luaL_error(L, "color stack is full");
}

static int neko_pop_color(lua_State *L) {
    bool ok = renderer_pop_color();
    return ok ? 0 : luaL_error(L, "color stack can't be less than 1");
}

#if 0

static int neko_default_font(lua_State *L) {
    if (g_app->default_font == nullptr) {
        g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
        g_app->default_font->load_default();
    }

    luax_ptr_userdata(L, g_app->default_font, "mt_font");
    return 1;
}

static int neko_draw_filled_rect(lua_State *L) {
    RectDescription rd = rect_description_args(L, 1);
    draw_filled_rect(&rd);
    return 0;
}

static int neko_draw_line_rect(lua_State *L) {
    RectDescription rd = rect_description_args(L, 1);
    draw_line_rect(&rd);
    return 0;
}

static int neko_draw_line_circle(lua_State *L) {
    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);
    lua_Number radius = luaL_checknumber(L, 3);

    draw_line_circle(x, y, radius);
    return 0;
}

static int neko_draw_line(lua_State *L) {
    lua_Number x0 = luaL_checknumber(L, 1);
    lua_Number y0 = luaL_checknumber(L, 2);
    lua_Number x1 = luaL_checknumber(L, 3);
    lua_Number y1 = luaL_checknumber(L, 4);

    draw_line(x0, y0, x1, y1);
    return 0;
}

#endif

static int neko_set_master_volume(lua_State *L) {
    lua_Number vol = luaL_checknumber(L, 1);
#if NEKO_AUDIO == 1
    ma_engine_set_volume(&g_app->audio_engine, (float)vol);
#endif
    return 0;
}

static int neko_get_channel(lua_State *L) {
    String contents = luax_check_string(L, 1);

    LuaChannel *chan = lua_channel_get(contents);
    if (chan == nullptr) {
        return 0;
    }

    luax_ptr_userdata(L, chan, "mt_channel");
    return 1;
}

static int neko_select(lua_State *L) {
    LuaVariant v = {};
    LuaChannel *chan = lua_channels_select(L, &v);
    if (chan == nullptr) {
        return 0;
    }

    v.push(L);
    v.trash();
    lua_pushstring(L, chan->name.load());
    return 2;
}

static int neko_thread_id(lua_State *L) {
    lua_pushinteger(L, this_thread_id());
    return 1;
}

static int neko_thread_sleep(lua_State *L) {
    PROFILE_FUNC();

    lua_Number secs = luaL_checknumber(L, 1);
    os_sleep((u32)(secs * 1000));
    return 0;
}

static int neko_program_path(lua_State *L) {
    String path = os_program_path();
    lua_pushlstring(L, path.data, path.len);
    return 1;
}

static int neko_program_dir(lua_State *L) {
    String path = os_program_dir();
    lua_pushlstring(L, path.data, path.len);
    return 1;
}

static int neko_is_fused(lua_State *L) {
    lua_pushboolean(L, g_app->is_fused.load());
    return 1;
}

static int neko_file_exists(lua_State *L) {
    String path = luax_check_string(L, 1);
    lua_pushboolean(L, vfs_file_exists(path));
    return 1;
}

static int neko_file_read(lua_State *L) {
    PROFILE_FUNC();

    String path = luax_check_string(L, 1);

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, path);
    if (!ok) {
        lua_pushnil(L);
        lua_pushboolean(L, false);
        return 2;
    }
    neko_defer(mem_free(contents.data));

    lua_pushlstring(L, contents.data, contents.len);
    lua_pushboolean(L, true);
    return 2;
}

static int neko_file_write(lua_State *L) {
    String path = luax_check_string(L, 1);
    String contents = luax_check_string(L, 2);

    FILE *f = neko_fopen(path.data, "w");
    if (f == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }
    neko_defer(neko_fclose(f));

    size_t written = neko_fwrite(contents.data, 1, contents.len, f);
    bool ok = written < contents.len;

    lua_pushboolean(L, ok);
    return 1;
}

#if 0

static sg_filter str_to_filter_mode(lua_State *L, String s) {
    switch (fnv1a(s)) {
        case "none"_hash:
            return SG_FILTER_NONE;
            break;
        case "nearest"_hash:
            return SG_FILTER_NEAREST;
            break;
        case "linear"_hash:
            return SG_FILTER_LINEAR;
            break;
        default:
            luax_string_oneof(L, {"none", "nearest", "linear"}, s);
            return _SG_FILTER_DEFAULT;
    }
}

static sg_wrap str_to_wrap_mode(lua_State *L, String s) {
    switch (fnv1a(s)) {
        case "repeat"_hash:
            return SG_WRAP_REPEAT;
            break;
        case "mirroredrepeat"_hash:
            return SG_WRAP_MIRRORED_REPEAT;
            break;
        case "clamp"_hash:
            return SG_WRAP_CLAMP_TO_EDGE;
            break;
        default:
            luax_string_oneof(L, {"repeat", "mirroredrepeat", "clamp"}, s);
            return _SG_WRAP_DEFAULT;
    }
}

static int neko_make_sampler(lua_State *L) {
    String min_filter = luax_opt_string_field(L, 1, "min_filter", "nearest");
    String mag_filter = luax_opt_string_field(L, 1, "mag_filter", "nearest");
    String mipmap_filter = luax_opt_string_field(L, 1, "mipmap_filter", "none");
    String wrap_u = luax_opt_string_field(L, 1, "wrap_u", "repeat");
    String wrap_v = luax_opt_string_field(L, 1, "wrap_v", "repeat");

    sg_sampler_desc desc = {};
    desc.min_filter = str_to_filter_mode(L, min_filter);
    desc.mag_filter = str_to_filter_mode(L, mag_filter);
    desc.mipmap_filter = str_to_filter_mode(L, mipmap_filter);
    desc.wrap_u = str_to_wrap_mode(L, wrap_u);
    desc.wrap_v = str_to_wrap_mode(L, wrap_v);

    sg_sampler sampler = sg_make_sampler(&desc);

    luax_new_userdata(L, sampler.id, "mt_sampler");
    return 1;
}

static int neko_default_sampler(lua_State *L) {
    u32 id = SG_INVALID_ID;
    luax_new_userdata(L, id, "mt_sampler");
    return 1;
}

#endif

static int neko_make_thread(lua_State *L) {
    String contents = luax_check_string(L, 1);
    LuaThread *lt = (LuaThread *)mem_alloc(sizeof(LuaThread));
    lt->make(contents, "thread");
    luax_ptr_userdata(L, lt, "mt_thread");
    return 1;
}

static int neko_make_channel(lua_State *L) {
    String name = luax_check_string(L, 1);
    lua_Integer len = luaL_optinteger(L, 2, 0);
    if (len < 0) {
        len = 0;
    }

    LuaChannel *chan = lua_channel_make(name, len);
    luax_ptr_userdata(L, chan, "mt_channel");
    return 1;
}

static int neko_image_load(lua_State *L) {
    String str = luax_check_string(L, 1);
    bool generate_mips = lua_toboolean(L, 2);

    AssetLoadData desc = {};
    desc.kind = AssetKind_Image;
    desc.generate_mips = generate_mips;

    Asset asset = {};
    bool ok = asset_load(desc, str, &asset);
    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, asset.hash, "mt_image");
    return 1;
}

#if 0

static int neko_font_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    FontFamily *font = (FontFamily *)mem_alloc(sizeof(FontFamily));
    bool ok = font->load(str);
    if (!ok) {
        mem_free(font);
        return 0;
    }

    luax_ptr_userdata(L, font, "mt_font");
    return 1;
}

static int neko_sprite_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    Asset asset = {};
    bool ok = asset_load_kind(AssetKind_Sprite, str, &asset);
    if (!ok) {
        return 0;
    }

    Sprite spr = {};
    spr.sprite = asset.hash;

    luax_new_userdata(L, spr, "mt_sprite");
    return 1;
}

static int neko_atlas_load(lua_State *L) {
    String str = luax_check_string(L, 1);
    bool generate_mips = lua_toboolean(L, 2);

    Atlas atlas = {};
    bool ok = atlas.load(str, generate_mips);
    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, atlas, "mt_atlas");
    return 1;
}

static int neko_tilemap_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    Asset asset = {};
    bool ok = asset_load_kind(AssetKind_Tilemap, str, &asset);
    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, asset.hash, "mt_tilemap");
    return 1;
}

#endif

static int neko_pak_load(lua_State *L) {
    String name = luax_check_string(L, 1);
    String path = luax_check_string(L, 2);

    neko_pak pak;

    bool ok = pak.load(path.cstr(), 0, false);

    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, pak, "mt_pak");
    return 1;
}

#if 1

#if 0

static bool __neko_bind_platform_key_pressed(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_pressed(cval);
}

static bool __neko_bind_platform_was_key_down(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_down(cval);
}

static bool __neko_bind_platform_key_down(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_down(cval);
}

static bool __neko_bind_platform_key_released(const_str key) {
    neko_os_keycode cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_keycode, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_key_released(cval);
}

static bool __neko_bind_platform_was_mouse_down(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_was_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_pressed(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_mouse_pressed(cval);
}

static bool __neko_bind_platform_mouse_down(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_mouse_down(cval);
}

static bool __neko_bind_platform_mouse_released(const_str key) {
    neko_os_mouse_button_code cval;

    lua_pushstring(ENGINE_LUA(), key);
    neko_luabind_to(ENGINE_LUA(), neko_os_mouse_button_code, &cval, -1);
    lua_pop(ENGINE_LUA(), 1);

    return neko_os_mouse_released(cval);
}

LUA_FUNCTION(__neko_bind_platform_mouse_delta) {
    neko_vec2 v2 = neko_os_mouse_deltav();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_mouse_position) {
    neko_vec2 v2 = neko_os_mouse_positionv();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_mouse_wheel) {
    neko_vec2 v2 = neko_os_mouse_wheelv();
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_window_size) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_os_window_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_framebuffer_size) {
    u32 handle = lua_tointeger(L, -1);
    neko_vec2 v2 = neko_os_framebuffer_sizev(handle);
    lua_pushnumber(L, v2.x);
    lua_pushnumber(L, v2.y);
    return 2;
}

LUA_FUNCTION(__neko_bind_platform_set_window_title) {
    u32 handle = lua_tointeger(L, -1);
    const_str title = lua_tostring(L, -2);
    neko_os_set_window_title(handle, title);
    return 0;
}

inline void neko_register_platform(lua_State* L) {

    neko_lua_enum(L, neko_os_keycode);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_INVALID);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SPACE);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_APOSTROPHE); /* ' */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_COMMA);      /* , */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_MINUS);      /* - */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PERIOD);     /* . */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SLASH);      /* / */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_0);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_1);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_2);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_3);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_4);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_5);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_6);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_7);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_8);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_9);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SEMICOLON); /* ; */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_EQUAL);     /* = */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_A);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_B);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_C);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_D);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_E);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_G);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_H);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_I);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_J);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_K);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_L);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_M);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_N);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_O);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_P);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_Q);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_R);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_S);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_T);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_U);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_V);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_W);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_X);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_Y);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_Z);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_BRACKET);  /* [ */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_BACKSLASH);     /* \ */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_BRACKET); /* ] */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_GRAVE_ACCENT);  /* ` */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_WORLD_1);       /* non-US #1 */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_WORLD_2);       /* non-US #2 */
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_ESC);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_ENTER);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_TAB);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_BACKSPACE);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_INSERT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_DELETE);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_DOWN);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_UP);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PAGE_UP);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PAGE_DOWN);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_HOME);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_END);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_CAPS_LOCK);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_SCROLL_LOCK);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_NUM_LOCK);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PRINT_SCREEN);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_PAUSE);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F1);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F2);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F3);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F4);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F5);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F6);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F7);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F8);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F9);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F10);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F11);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F12);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F13);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F14);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F15);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F16);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F17);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F18);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F19);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F20);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F21);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F22);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F23);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F24);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_F25);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_0);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_1);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_2);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_3);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_4);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_5);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_6);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_7);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_8);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_9);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_DECIMAL);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_DIVIDE);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_MULTIPLY);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_SUBTRACT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_ADD);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_ENTER);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_KP_EQUAL);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_SHIFT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_CONTROL);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_ALT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_LEFT_SUPER);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_SHIFT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_CONTROL);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_ALT);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_RIGHT_SUPER);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_MENU);
    neko_lua_enum_value(L, neko_os_keycode, NEKO_KEYCODE_COUNT);

    neko_lua_enum(L, neko_os_mouse_button_code);
    neko_lua_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_LBUTTON);
    neko_lua_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_RBUTTON);
    neko_lua_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_MBUTTON);
    neko_lua_enum_value(L, neko_os_mouse_button_code, NEKO_MOUSE_BUTTON_CODE_COUNT);

    neko::lua_bind::bind("neko_key_pressed", &__neko_bind_platform_key_pressed);
    neko::lua_bind::bind("neko_was_key_down", &__neko_bind_platform_was_key_down);
    neko::lua_bind::bind("neko_was_mouse_down", &__neko_bind_platform_was_mouse_down);
    neko::lua_bind::bind("neko_key_down", &__neko_bind_platform_key_down);
    neko::lua_bind::bind("neko_key_released", &__neko_bind_platform_key_released);
    neko::lua_bind::bind("neko_mouse_down", &__neko_bind_platform_mouse_down);
    neko::lua_bind::bind("neko_mouse_pressed", &__neko_bind_platform_mouse_pressed);
    neko::lua_bind::bind("neko_mouse_released", &__neko_bind_platform_mouse_released);
    neko::lua_bind::bind("neko_mouse_moved", +[]() -> bool { return neko_os_mouse_moved(); });
    neko::lua_bind::bind("neko_main_window", +[]() -> u32 { return neko_os_main_window(); });
    neko::lua_bind::bind("neko_set_window_size", +[](u32 handle, u32 width, u32 height) { neko_os_set_window_size(handle, width, height); });
    neko::lua_bind::bind("neko_set_mouse_position", +[](u32 handle, f64 x, f64 y) { neko_os_mouse_set_position(handle, x, y); });

    lua_register(L, "neko_mouse_delta", __neko_bind_platform_mouse_delta);
    lua_register(L, "neko_mouse_position", __neko_bind_platform_mouse_position);
    lua_register(L, "neko_mouse_wheel", __neko_bind_platform_mouse_wheel);

    lua_register(L, "neko_window_size", __neko_bind_platform_window_size);
    lua_register(L, "neko_framebuffer_size", __neko_bind_platform_framebuffer_size);
    lua_register(L, "neko_set_window_title", __neko_bind_platform_set_window_title);
}

LUA_FUNCTION(__neko_bind_aseprite_render_create) {
    neko_aseprite* sprite_data = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)lua_newuserdata(L, sizeof(neko_aseprite_renderer));
    memset(user_handle, 0, sizeof(neko_aseprite_renderer));
    user_handle->sprite = sprite_data;
    neko_aseprite_renderer_play(user_handle, "Idle");
    luaL_setmetatable(L, "mt_aseprite_renderer");
    return 1;
}

LUA_FUNCTION(__neko_bind_aseprite_render_gc) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    // console_log("aseprite_render __gc %p", user_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_render_update) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    neko_instance_t* engine = neko_instance();

    neko_aseprite_renderer_update(user_handle, engine->platform->time.delta);

    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_render_update_animation) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    const_str by_tag = lua_tostring(L, 2);
    neko_aseprite_renderer_play(user_handle, by_tag);
    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_render) {
    PROFILE_FUNC();

    if (lua_gettop(L) != 4) {
        return luaL_error(L, "Function expects exactly 4 arguments");
    }

    luaL_argcheck(L, lua_gettop(L) == 4, 1, "expects exactly 4 arguments");

    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    auto xform = lua2struct::unpack<neko_vec2>(L, 2);

    int direction = neko::neko_lua_to<int>(L, 3);
    f32 scale = neko::neko_lua_to<f32>(L, 4);

    neko_instance_t* engine = neko_instance();

    i32 index;
    if (user_handle->loop) {
        index = user_handle->loop->indices[user_handle->current_frame];
    } else {
        index = user_handle->current_frame;
    }

    neko_aseprite* spr = user_handle->sprite;
    neko_aseprite_frame f = spr->frames[index];

    if (direction)
        neko_idraw_rect_textured_ext(&ENGINE_INTERFACE()->idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u1, f.v0, f.u0, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);
    else
        neko_idraw_rect_textured_ext(&ENGINE_INTERFACE()->idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u0, f.v0, f.u1, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);

    return 0;
}

LUA_FUNCTION(__neko_bind_aseprite_create) {
    const_str file_path = lua_tostring(L, 1);

    neko_aseprite* user_handle = (neko_aseprite*)lua_newuserdata(L, sizeof(neko_aseprite));
    memset(user_handle, 0, sizeof(neko_aseprite));

    neko_aseprite_load(user_handle, file_path);

    luaL_setmetatable(L, "mt_aseprite");

    return 1;
}

LUA_FUNCTION(__neko_bind_aseprite_gc) {
    neko_aseprite* user_handle = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    if (user_handle->frames != NULL) neko_aseprite_end(user_handle);
    // console_log("aseprite __gc %p", user_handle);
    return 0;
}



LUA_FUNCTION(__neko_bind_tiled_create) {
    const_str map_path = lua_tostring(L, 1);
    const_str glsl_vs_src = lua_tostring(L, 2);
    const_str glsl_fs_src = lua_tostring(L, 3);

    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_newuserdata(L, sizeof(neko_tiled_renderer));
    memset(user_handle, 0, sizeof(neko_tiled_renderer));

    neko_tiled_load(&(user_handle->map), map_path, NULL);

    neko_tiled_render_init(&ENGINE_INTERFACE()->cb, user_handle, glsl_vs_src, glsl_fs_src);

    return 1;
}

LUA_FUNCTION(__neko_bind_tiled_render) {

    PROFILE_FUNC();

    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)lua_touserdata(L, 1);

    neko_renderpass_t rp = R_RENDER_PASS_DEFAULT;
    neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 2);

    auto xform = lua2struct::unpack<neko_vec2>(L, 3);

    f32 l = lua_tonumber(L, 4);
    f32 r = lua_tonumber(L, 5);
    f32 t = lua_tonumber(L, 6);
    f32 b = lua_tonumber(L, 7);

    tiled_render->camera_mat = neko_mat4_ortho(l, r, b, t, -1.0f, 1.0f);

    neko_command_buffer_t* cb = &ENGINE_INTERFACE()->cb;

    neko_render_renderpass_begin(cb, rp);
    {
        neko_tiled_render_begin(cb, tiled_render);

        PROFILE_BLOCK("tiled_render");

        for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.layers); i++) {
            layer_t* layer = tiled_render->map.layers + i;
            for (u32 y = 0; y < layer->height; y++) {
                for (u32 x = 0; x < layer->width; x++) {
                    tile_t* tile = layer->tiles + (x + y * layer->width);
                    if (tile->id != 0) {
                        tileset_t* tileset = tiled_render->map.tilesets + tile->tileset_id;
                        u32 tsxx = (tile->id % (tileset->width / tileset->tile_width) - 1) * tileset->tile_width;
                        u32 tsyy = tileset->tile_height * ((tile->id - tileset->first_gid) / (tileset->width / tileset->tile_width));
                        neko_tiled_quad_t quad = {.tileset_id = tile->tileset_id,
                                                  .texture = tileset->texture,
                                                  .texture_size = {(f32)tileset->width, (f32)tileset->height},
                                                  .position = {(f32)(x * tileset->tile_width * SPRITE_SCALE) + xform.x, (f32)(y * tileset->tile_height * SPRITE_SCALE) + xform.y},
                                                  .dimentions = {(f32)(tileset->tile_width * SPRITE_SCALE), (f32)(tileset->tile_height * SPRITE_SCALE)},
                                                  .rectangle = {(f32)tsxx, (f32)tsyy, (f32)tileset->tile_width, (f32)tileset->tile_height},
                                                  .color = layer->tint,
                                                  .use_texture = true};
                        neko_tiled_render_push(cb, tiled_render, quad);
                    }
                }
            }
            neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        }

        for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
            object_group_t* group = tiled_render->map.object_groups + i;
            for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
                object_t* object = group->objects + ii;
                neko_tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE) + xform.x, (f32)(object->y * SPRITE_SCALE) + xform.y},
                                          .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
                                          .color = group->color,
                                          .use_texture = false};
                neko_tiled_render_push(cb, tiled_render, quad);
            }
            neko_tiled_render_draw(cb, tiled_render);  // 一层渲染一次
        }

        // for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        //     object_group_t *group = tiled_render->map.object_groups + i;
        //     for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
        //         object_t *object = group->objects + ii;
        //         auto draw_poly = [sprite_batch](c2Poly poly) {
        //             c2v *verts = poly.verts;
        //             int count = poly.count;
        //             for (int i = 0; i < count; ++i) {
        //                 int iA = i;
        //                 int iB = (i + 1) % count;
        //                 c2v a = verts[iA];
        //                 c2v b = verts[iB];
        //                 gl_line(sprite_batch, a.x, a.y, 0, b.x, b.y, 0);
        //             }
        //         };
        //         draw_poly(object->phy.poly);
        //     }
        // }
    }
    neko_render_renderpass_end(cb);

    return 0;
}

LUA_FUNCTION(__neko_bind_tiled_unload) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    neko_tiled_unload(&user_handle->map);
    return 0;
}

LUA_FUNCTION(__neko_bind_tiled_load) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    const_str path = lua_tostring(L, 2);
    neko_tiled_load(&user_handle->map, path, NULL);
    return 0;
}

LUA_FUNCTION(__neko_bind_tiled_end) {
    neko_tiled_renderer* user_handle = (neko_tiled_renderer*)lua_touserdata(L, 1);
    neko_tiled_unload(&user_handle->map);
    neko_tiled_render_fini(user_handle);
    return 0;
}

auto __neko_bind_tiled_get_objects(void* tiled_render_ud) {
    neko_tiled_renderer* tiled_render = (neko_tiled_renderer*)tiled_render_ud;
    std::map<std::string, std::vector<std::list<f32>>> data;
    for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        object_group_t* group = tiled_render->map.object_groups + i;
        data.insert({group->name, {}});
        for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
            object_t* object = group->objects + ii;
            auto& contain = data[group->name];
            contain.push_back(std::list{(f32)(object->x * SPRITE_SCALE), (f32)(object->y * SPRITE_SCALE), (f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)});
        }
    }
    return data;
}

#endif

#if 0
LUA_FUNCTION(__neko_bind_pixelui_create){
    // const_str file_path = lua_tostring(L, 1);

    pixelui_t* user_handle = (pixelui_t*)lua_newuserdata(L, sizeof(pixelui_t));
    memset(user_handle, 0, sizeof(pixelui_t));

    // user_handle->show_frame_count = true;
    user_handle->show_material_selection_panel = true;

    pixelui_init(user_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_pixelui_update){
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    update_ui(user_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_pixelui_tex){
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    neko_luabind_struct_push_member(L, neko_texture_t, id, &user_handle->tex_ui);
    return 1;
}

LUA_FUNCTION(__neko_bind_pixelui_end){
    pixelui_t* user_handle = (pixelui_t*)lua_touserdata(L, 1);
    pixelui_fini(user_handle);
    return 0;
}
#endif

#if 0

static void fontbatch_metatable(lua_State* L) {
    // static luaL_Reg lib[] = {{"add", add}, {"set_recursive", set_recursive}, {"set_follow_symlinks", set_follow_symlinks}, {"set_filter", set_filter}, {"select", select}, {NULL, NULL}};
    // luaL_newlibtable(L, lib);
    // luaL_setfuncs(L, lib, 0);
    // lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__gc",
                             +[](lua_State* L) {
                                 neko_fontbatch_t* fontbatch = neko::lua::toudata_ptr<neko_fontbatch_t>(L, 1);
                                 neko_fontbatch_end(fontbatch);
                                 console_log("fontbatch __gc %p", fontbatch);
                                 return 0;
                             }},
                            {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

namespace neko::lua {
template <>
struct udata<neko_fontbatch_t> {
    static inline int nupvalue = 1;
    static inline auto metatable = fontbatch_metatable;
};
}  // namespace neko::lua

LUA_FUNCTION(__neko_bind_fontbatch_create) {

    neko::string font_vs = neko::luax_check_string(L, 1);
    neko::string font_ps = neko::luax_check_string(L, 2);

    neko_fontbatch_t& fontbatch = neko::lua::newudata<neko_fontbatch_t>(L);

    neko::string contents = {};
    bool ok = vfs_read_entire_file( &contents, "gamedir/1.fnt");
    neko_assert(ok);
    neko_fontbatch_init(&fontbatch, font_vs.data, font_ps.data, neko_game()->DisplaySize, "gamedir/1_0.png", contents.data, (i32)contents.len);
    neko_defer(neko_safe_free(contents.data));

    return 1;
}

LUA_FUNCTION(__neko_bind_fontbatch_draw) {
    neko_fontbatch_t& fontbatch = neko::lua::toudata<neko_fontbatch_t>(L, 1);

    neko_render_draw_batch(&ENGINE_INTERFACE()->cb, fontbatch.font_render, 0, 0, 0);

    return 0;
}

LUA_FUNCTION(__neko_bind_fontbatch_text) {
    int numArgs = lua_gettop(L);  // 获取参数数量
    neko_fontbatch_t& fontbatch = neko::lua::toudata<neko_fontbatch_t>(L, 1);
    auto v1 = lua2struct::unpack<neko_vec2>(L, 2);
    const_str text = lua_tostring(L, 3);

    f32 scale = 0.f;

    if (numArgs > 3) scale = lua_tonumber(L, 4);

    neko_fontbatch_draw(&fontbatch, neko_game()->DisplaySize, text, v1.x, v1.y, 1, 1.f, 800.f, scale);

    return 0;
}

typedef struct {
    u64 image_id;  // NEKO_SPRITEBATCH_U64
    int depth;
    f32 x;
    f32 y;
    f32 sx;
    f32 sy;
    f32 c;
    f32 s;
} neko_sprite_t;

typedef struct {
    f32 x, y;
    f32 u, v;
} vertex_t;

#define SPRITE_VERTS_MAX (1024 * 10)

struct neko_sprite_batch_t {

    neko_batch_t sb;

    neko_render_batch_context_t* sprite_batch;
    neko_render_batch_shader_t sprite_shader;
    neko_render_batch_renderable_t sprite_renderable;
    f32 sprite_projection[16];

    int call_count = 0;

    int sprite_verts_count;
    vertex_t sprite_verts[SPRITE_VERTS_MAX];

    neko_image* images;
    int images_count = 0;
};

static void make_sprite(neko_sprite_batch_t* user_handle, neko_sprite_t* sprite, u64 image_id, f32 x, f32 y, f32 scale, f32 angle_radians, int depth) {

    neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());

    // f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    // f32 y0 = (fbs.y / 2.f - y) / scale / 2.f;

    f32 x0 = x;
    f32 y0 = y;

    sprite->image_id = image_id;
    sprite->depth = depth;
    sprite->x = x0;
    sprite->y = y0;
    sprite->sx = (f32)user_handle->images[sprite->image_id].w * 2.0f * scale;
    sprite->sy = (f32)user_handle->images[sprite->image_id].h * 2.0f * scale;
    sprite->c = cosf(angle_radians);
    sprite->s = sinf(angle_radians);
}

static void push_sprite(neko_sprite_batch_t* user_handle, neko_sprite_t* sp) {
    neko_batch_sprite_t s;
    s.image_id = sp->image_id;
    s.w = user_handle->images[sp->image_id].w;
    s.h = user_handle->images[sp->image_id].h;
    s.x = sp->x;
    s.y = sp->y;
    s.sx = sp->sx;
    s.sy = sp->sy;
    s.c = sp->c;
    s.s = sp->s;
    s.sort_bits = (u64)sp->depth;
    neko_batch_push(&user_handle->sb, s);
}

static void batch_report(neko_batch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata) {

    (void)texture_w;
    (void)texture_h;

    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)udata;

    ++user_handle->call_count;

    // printf("begin batch\n");
    // for (int i = 0; i < count; ++i) printf("\t%llu\n", sprites[i].texture_id);
    // printf("end batch\n");

    // build the draw call
    neko_render_batch_draw_call_t call;
    call.r = &user_handle->sprite_renderable;
    call.textures[0] = (u32)sprites[0].texture_id;
    call.texture_count = 1;

    // set texture uniform in shader
    neko_render_batch_send_texture(call.r->program, "u_sprite_texture", 0);

    // TODO: 附加排序 遮盖判定系统

    // build vertex buffer of quads from all sprite transforms
    call.verts = user_handle->sprite_verts + user_handle->sprite_verts_count;
    call.vert_count = count * 6;
    user_handle->sprite_verts_count += call.vert_count;
    assert(user_handle->sprite_verts_count < SPRITE_VERTS_MAX);

    vertex_t* verts = (vertex_t*)call.verts;
    for (int i = 0; i < count; ++i) {
        neko_batch_sprite_t* s = sprites + i;

        neko_vec2 quad[] = {
                {-0.5f, 0.5f},
                {0.5f, 0.5f},
                {0.5f, -0.5f},
                {-0.5f, -0.5f},
        };

        for (int j = 0; j < 4; ++j) {
            f32 x = quad[j].x;
            f32 y = quad[j].y;

            // scale sprite about origin
            x *= s->sx;
            y *= s->sy;

            // rotate sprite about origin
            f32 x0 = s->c * x - s->s * y;
            f32 y0 = s->s * x + s->c * y;
            x = x0;
            y = y0;

            // translate sprite into the world
            x += s->x;
            y += s->y;

            quad[j].x = x;
            quad[j].y = y;
        }

        // output transformed quad into CPU buffer
        vertex_t* out_verts = verts + i * 6;

        out_verts[0].x = quad[0].x;
        out_verts[0].y = quad[0].y;
        out_verts[0].u = s->minx;
        out_verts[0].v = s->maxy;

        out_verts[1].x = quad[3].x;
        out_verts[1].y = quad[3].y;
        out_verts[1].u = s->minx;
        out_verts[1].v = s->miny;

        out_verts[2].x = quad[1].x;
        out_verts[2].y = quad[1].y;
        out_verts[2].u = s->maxx;
        out_verts[2].v = s->maxy;

        out_verts[3].x = quad[1].x;
        out_verts[3].y = quad[1].y;
        out_verts[3].u = s->maxx;
        out_verts[3].v = s->maxy;

        out_verts[4].x = quad[3].x;
        out_verts[4].y = quad[3].y;
        out_verts[4].u = s->minx;
        out_verts[4].v = s->miny;

        out_verts[5].x = quad[2].x;
        out_verts[5].y = quad[2].y;
        out_verts[5].u = s->maxx;
        out_verts[5].v = s->miny;
    }

    // 提交对draw_call的调用
    neko_render_batch_push_draw_call(user_handle->sprite_batch, call);
}

void get_pixels(u64 image_id, void* buffer, int bytes_to_fill, void* udata) {
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)udata;
    memcpy(buffer, user_handle->images[image_id].pix, bytes_to_fill);
}

LUA_FUNCTION(__neko_bind_sprite_batch_create) {
    PROFILE_FUNC();

    int max_draw_calls = lua_tointeger(L, 1);

    if (!lua_istable(L, 2)) {
        lua_pushstring(L, "Argument is not a table");
        lua_error(L);
        return 0;
    }

    int count = luaL_len(L, 2);
    std::vector<const_str> texture_list;

    for (int i = 0; i < count; i++) {
        lua_rawgeti(L, 2, i + 1);
        const_str path = lua_tostring(L, -1);
        texture_list.push_back(path);

        lua_pop(L, 1);
    }

    const_str glsl_vs_src = lua_tostring(L, 3);
    const_str glsl_ps_src = lua_tostring(L, 4);

    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_newuserdata(L, sizeof(neko_sprite_batch_t));
    memset(user_handle, 0, sizeof(neko_sprite_batch_t));

    user_handle->sprite_batch = neko_render_batch_make_ctx(max_draw_calls);

    neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());

    neko_render_batch_vertex_data_t vd;
    neko_render_batch_make_vertex_data(&vd, 1024 * 1024, GL_TRIANGLES, sizeof(vertex_t), GL_DYNAMIC_DRAW);
    neko_render_batch_add_attribute(&vd, "in_pos", 2, R_BATCH_FLOAT, NEKO_OFFSET(vertex_t, x));
    neko_render_batch_add_attribute(&vd, "in_uv", 2, R_BATCH_FLOAT, NEKO_OFFSET(vertex_t, u));

    neko_render_batch_make_renderable(&user_handle->sprite_renderable, &vd);
    neko_render_batch_load_shader(&user_handle->sprite_shader, glsl_vs_src, glsl_ps_src);
    neko_render_batch_set_shader(&user_handle->sprite_renderable, &user_handle->sprite_shader);

    neko_render_batch_ortho_2d(fbs.x, fbs.y, 0, 0, user_handle->sprite_projection);

    neko_render_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

    user_handle->images_count = count;
    user_handle->images = (neko_image*)neko_safe_malloc(sizeof(neko_image) * count);

    for (i32 i = 0; i < user_handle->images_count; ++i) user_handle->images[i].load(texture_list[i]);

    neko_batch_config_t sb_config;
    neko_batch_set_default_config(&sb_config);
    sb_config.pixel_stride = sizeof(u8) * 4;
    sb_config.atlas_width_in_pixels = 1024;
    sb_config.atlas_height_in_pixels = 1024;
    sb_config.atlas_use_border_pixels = 0;
    sb_config.ticks_to_decay_texture = 3;
    sb_config.lonely_buffer_count_till_flush = 1;
    sb_config.ratio_to_decay_atlas = 0.5f;
    sb_config.ratio_to_merge_atlases = 0.25f;

    sb_config.batch_callback = batch_report;                        // report batches of sprites from `neko_batch_flush`
    sb_config.get_pixels_callback = get_pixels;                     // used to retrieve image pixels from `neko_batch_flush` and `neko_batch_defrag`
    sb_config.generate_texture_callback = generate_texture_handle;  // used to generate a texture handle from `neko_batch_flush` and `neko_batch_defrag`
    sb_config.delete_texture_callback = destroy_texture_handle;     // used to destroy a texture handle from `neko_batch_defrag`

    neko_batch_init(&user_handle->sb, &sb_config, user_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_sprite_batch_render_ortho) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    f32 w = lua_tonumber(L, 2);
    f32 h = lua_tonumber(L, 3);
    f32 x = lua_tonumber(L, 4);
    f32 y = lua_tonumber(L, 5);

    neko_render_batch_ortho_2d(w, h, x / w, y / h, user_handle->sprite_projection);

    neko_render_batch_send_matrix(&user_handle->sprite_shader, "u_mvp", user_handle->sprite_projection);

    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_make_sprite) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    auto image_id = lua_tointeger(L, 2);
    auto x = lua_tonumber(L, 3);
    auto y = lua_tonumber(L, 4);
    auto scale = lua_tonumber(L, 5);
    auto angle_radians = lua_tonumber(L, 6);
    auto depth = lua_tonumber(L, 7);

    neko_sprite_t* sprite_handle = (neko_sprite_t*)lua_newuserdata(L, sizeof(neko_sprite_t));
    memset(sprite_handle, 0, sizeof(neko_sprite_t));

    make_sprite(user_handle, sprite_handle, image_id, x, y, scale, angle_radians, depth);

    return 1;
}

LUA_FUNCTION(__neko_bind_sprite_batch_push_sprite) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);
    neko_sprite_t* sprite_handle = (neko_sprite_t*)lua_touserdata(L, 2);
    push_sprite(user_handle, sprite_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_render_begin) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    user_handle->call_count = 0;

    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_render_end) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    {
        neko_batch_defrag(&user_handle->sb);
        neko_batch_tick(&user_handle->sb);
        neko_batch_flush(&user_handle->sb);
        user_handle->sprite_verts_count = 0;
    }

    neko_render_draw_batch(&ENGINE_INTERFACE()->cb, user_handle->sprite_batch, 0, 0, 0);

    return 0;
}

LUA_FUNCTION(__neko_bind_sprite_batch_end) {
    PROFILE_FUNC();
    neko_sprite_batch_t* user_handle = (neko_sprite_batch_t*)lua_touserdata(L, 1);

    neko_batch_term(&user_handle->sb);

    for (i32 i = 0; i < user_handle->images_count; ++i) {
        (*(user_handle->images + i)).free();
    }

    neko_safe_free(user_handle->images);

    neko_render_batch_free(user_handle->sprite_batch);

    return 0;
}

#endif

// 测试 luacstruct 用
struct CGameObject {
    int id;
    bool active;
    bool visible;
    bool selected;
};

// template <>
// struct neko::static_refl::neko_type_info<CGameObject> : neko_type_info_base<CGameObject> {
//     static constexpr AttrList attrs = {};
//     static constexpr FieldList fields = {
//             rf_field{TSTR("id"), &rf_type::id},              //
//             rf_field{TSTR("active"), &rf_type::active},      //
//             rf_field{TSTR("visible"), &rf_type::visible},    //
//             rf_field{TSTR("selected"), &rf_type::selected},  //
//     };
// };

REGISTER_TYPE_DF(i8)
REGISTER_TYPE_DF(i16)
REGISTER_TYPE_DF(i32)
REGISTER_TYPE_DF(i64)
REGISTER_TYPE_DF(u8)
REGISTER_TYPE_DF(u16)
REGISTER_TYPE_DF(u32)
REGISTER_TYPE_DF(u64)
REGISTER_TYPE_DF(bool)
// REGISTER_TYPE_DF(b32)
REGISTER_TYPE_DF(f32)
REGISTER_TYPE_DF(f64)
REGISTER_TYPE_DF(const_str)

namespace neko::reflection {
template <>
Type *type_of<CGameObject>() {
    static Type type;
    type.name = "CGameObject";
    type.destroy = [](void *obj) { delete static_cast<CGameObject *>(obj); };
    type.copy = [](const void *obj) { return (void *)(new CGameObject(*static_cast<const CGameObject *>(obj))); };
    type.move = [](void *obj) { return (void *)(new CGameObject(std::move(*static_cast<CGameObject *>(obj)))); };
    type.fields.insert({"id", {type_of<decltype(CGameObject::id)>(), offsetof(CGameObject, id)}});
    type.fields.insert({"active", {type_of<decltype(CGameObject::active)>(), offsetof(CGameObject, active)}});
    type.fields.insert({"visible", {type_of<decltype(CGameObject::visible)>(), offsetof(CGameObject, visible)}});
    type.fields.insert({"selected", {type_of<decltype(CGameObject::selected)>(), offsetof(CGameObject, selected)}});
    // type.methods.insert({"say", type_ensure<&Person::say>()});
    return &type;
};
}  // namespace neko::reflection

DEFINE_IMGUI_BEGIN(template <>, CGameObject) {
    // neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { neko::imgui::Auto(value, std::string(field.name)); });
    neko::reflection::Any v = var;
    v.foreach ([](std::string_view name, neko::reflection::Any &value) {
        // if (value.GetType() == neko::reflection::type_of<std::string_view>()) {
        //     std::cout << name << " = " << value.cast<std::string_view>() << std::endl;
        // } else if (value.GetType() == neko::reflection::type_of<std::size_t>()) {
        //     std::cout << name << " = " << value.cast<std::size_t>() << std::endl;
        // }
        neko::imgui::Auto(value, std::string(name));
    });
}
DEFINE_IMGUI_END();

LUA_FUNCTION(__neko_bind_gameobject_inspect) {

    CGameObject *user_handle = (CGameObject *)lua_touserdata(L, 1);

    if (user_handle == NULL) return 0;

    // neko_println("gameobj %d %s %s %s", user_handle->id, NEKO_BOOL_STR(user_handle->active), NEKO_BOOL_STR(user_handle->visible), NEKO_BOOL_STR(user_handle->selected));

    ImGui::Text("GameObject_%d", user_handle->id);

    neko::imgui::Auto(user_handle, "CGameObject");

    return 0;
}

#if 0
LUA_FUNCTION(__neko_bind_filesystem_create){
    neko_filesystem_t* filewatch = (neko_filesystem_t*)lua_newuserdata(L, sizeof(neko_filesystem_t));
    memset(filewatch, 0, sizeof(neko_filesystem_t));
    neko_filesystem_create_internal(filewatch, NULL);

    return 1;
}

LUA_FUNCTION(__neko_bind_filesystem_fini){
    neko_filesystem_t* assetsys = (neko_filesystem_t*)lua_touserdata(L, 1);
    neko_filesystem_fini_internal(assetsys);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_create){
    neko_filesystem_t* assetsys = (neko_filesystem_t*)lua_touserdata(L, 1);

    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_newuserdata(L, sizeof(neko_filewatch_t));
    memset(filewatch, 0, sizeof(neko_filewatch_t));
    neko_filewatch_create_internal(filewatch, assetsys, NULL);
    return 1;
}

LUA_FUNCTION(__neko_bind_filewatch_fini){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_free_internal(filewatch);
    return 0;
}

static void watch_map_callback(neko_filewatch_update_t change, const_str virtual_path, void* udata) {
    std::string change_string;
    switch (change) {
        case FILEWATCH_DIR_ADDED:
            change_string = "FILEWATCH_DIR_ADDED";
            break;
        case FILEWATCH_DIR_REMOVED:
            change_string = "FILEWATCH_DIR_REMOVED";
            break;
        case FILEWATCH_FILE_ADDED:
            change_string = "FILEWATCH_FILE_ADDED";
            break;
        case FILEWATCH_FILE_REMOVED:
            change_string = "FILEWATCH_FILE_REMOVED";
            break;
        case FILEWATCH_FILE_MODIFIED:
            change_string = "FILEWATCH_FILE_MODIFIED";
            break;
    }

    const_str callback_funcname = (const_str)udata;

    lua_getglobal(ENGINE_L(), callback_funcname);
    bool is_callback = lua_isfunction(ENGINE_L(), -1);
    lua_pop(ENGINE_L(), 1);

    if (is_callback) try {
            neko_lua_call<void>(ENGINE_L(), callback_funcname, change_string, std::string(virtual_path));
        } catch (std::exception& ex) {
            console_log("lua exception %s", ex.what());
        }
}

LUA_FUNCTION(__neko_bind_filewatch_mount){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str actual_path = lua_tostring(L, 2);
    const_str virtual_path = lua_tostring(L, 3);
    neko_filewatch_mount(filewatch, actual_path, virtual_path);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_start_watch){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str virtual_path = lua_tostring(L, 2);
    const_str callback_funcname = lua_tostring(L, 3);
    neko_filewatch_start_watching(filewatch, virtual_path, watch_map_callback, (void*)callback_funcname);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_stop_watching){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    const_str virtual_path = lua_tostring(L, 2);
    neko_filewatch_stop_watching(filewatch, virtual_path);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_update){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_update(filewatch);
    return 0;
}

LUA_FUNCTION(__neko_bind_filewatch_notify){
    neko_filewatch_t* filewatch = (neko_filewatch_t*)lua_touserdata(L, 1);
    neko_filewatch_notify(filewatch);
    return 0;
}
#endif

#if 0

LUA_FUNCTION(__neko_bind_idraw_get) {
    lua_pushlightuserdata(L, &ENGINE_INTERFACE()->idraw);
    return 1;
}

LUA_FUNCTION(__neko_bind_idraw_draw) {
    PROFILE_FUNC();
    neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);
    return 1;
}

LUA_FUNCTION(__neko_bind_idraw_defaults) {
    PROFILE_FUNC();
    neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera2d) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, w, h);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera3d) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    neko_idraw_camera3d(&ENGINE_INTERFACE()->idraw, w, h);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera2d_ex) {
    f32 l = lua_tonumber(L, 1);
    f32 r = lua_tonumber(L, 2);
    f32 t = lua_tonumber(L, 3);
    f32 b = lua_tonumber(L, 4);
    neko_idraw_camera2d_ex(&ENGINE_INTERFACE()->idraw, l, r, t, b);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_rotatev) {
    f32 angle = lua_tonumber(L, 1);
    f32 x = lua_tonumber(L, 2);
    f32 y = lua_tonumber(L, 3);
    f32 z = lua_tonumber(L, 4);
    neko_idraw_rotatev(&ENGINE_INTERFACE()->idraw, angle, neko_v3(x, y, z));
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_box) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    f32 hx = lua_tonumber(L, 4);
    f32 hy = lua_tonumber(L, 5);
    f32 hz = lua_tonumber(L, 6);
    u8 r = lua_tointeger(L, 7);
    u8 g = lua_tointeger(L, 8);
    u8 b = lua_tointeger(L, 9);
    u8 a = lua_tointeger(L, 10);
    neko_render_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), neko_render_primitive_type, &type_val, 11);
    neko_idraw_box(&ENGINE_INTERFACE()->idraw, x, y, z, hx, hy, hz, r, g, b, a, type_val);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_translatef) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    neko_idraw_translatef(&ENGINE_INTERFACE()->idraw, x, y, z);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_rectv) {

    auto v1 = lua2struct::unpack<neko_vec2>(L, 1);
    auto v2 = lua2struct::unpack<neko_vec2>(L, 2);

    v2 = neko_vec2_add(v1, v2);

    neko_render_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), neko_render_primitive_type, &type_val, 3);

    neko_color_t col = NEKO_COLOR_WHITE;

    if (lua_gettop(L) == 4) {
        col = lua2struct::unpack<neko_color_t>(L, 4);
    }

    neko_idraw_rectv(&ENGINE_INTERFACE()->idraw, v1, v2, col, type_val);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_rectvd) {

    auto v1 = lua2struct::unpack<neko_vec2>(L, 1);
    auto v2 = lua2struct::unpack<neko_vec2>(L, 2);

    auto uv0 = lua2struct::unpack<neko_vec2>(L, 3);
    auto uv1 = lua2struct::unpack<neko_vec2>(L, 4);

    neko_render_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), neko_render_primitive_type, &type_val, 5);

    neko_color_t col = lua2struct::unpack<neko_color_t>(L, 6);

    neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, v1, v2, uv0, uv1, col, type_val);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_text) {
    PROFILE_FUNC();
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);

    neko_color_t col = neko_color(255, 50, 50, 255);

    if (lua_gettop(L) == 4) {
        col = lua2struct::unpack<neko_color_t>(L, 4);
    }

    neko_idraw_text(&ENGINE_INTERFACE()->idraw, x, y, text, NULL, false, col);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_camera) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    neko_camera_t camera;
    camera = neko_camera_default();
    neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());
    neko_idraw_camera(&ENGINE_INTERFACE()->idraw, &camera, (u32)fbs.x, (u32)fbs.y);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_depth_enabled) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_depth_enabled(&ENGINE_INTERFACE()->idraw, enable);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_face_cull_enabled) {
    bool enable = lua_toboolean(L, 1);
    neko_idraw_face_cull_enabled(&ENGINE_INTERFACE()->idraw, enable);
    return 0;
}

LUA_FUNCTION(__neko_bind_idraw_texture) {
    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(ENGINE_LUA(), neko_texture_t, id, &rt, 1);
    neko_idraw_texture(&ENGINE_INTERFACE()->idraw, rt);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_framebuffer_create) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    fbo = neko_render_framebuffer_create({});
    neko_luabind_struct_push_member(L, neko_framebuffer_t, id, &fbo);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_framebuffer_fini) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);
    neko_render_framebuffer_fini(fbo);
    return 0;
}



void inspect_shader(const_str label, GLuint program);



LUA_FUNCTION(__neko_bind_inspect_shaders) {
    u32 shader_id = lua_tonumber(L, 1);
    inspect_shader(std::to_string(shader_id).c_str(), shader_id);
    return 0;
}

LUA_FUNCTION(__neko_bind_inspect_textures) {
    u32 textures_id = lua_tonumber(L, 1);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_shader_create) {

    const_str shader_name = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);              // 检查是否为table
    int n = neko_lua_get_table_pairs_count(L, 2);  //

    neko_render_shader_source_desc_t* sources = (neko_render_shader_source_desc_t*)neko_safe_malloc(n * sizeof(neko_render_shader_source_desc_t));

    const_str shader_type[] = {"VERTEX", "FRAGMENT", "COMPUTE"};

    int j = 0;

    for (int i = 0; i < 3; i++) {
        lua_pushstring(L, shader_type[i]);  // # -1
        lua_gettable(L, -2);                // pop # -1
        if (!lua_isnil(L, -1)) {
            const_str src = lua_tostring(L, -1);  // # -1
            if (src != NULL) {
                sources[j++] = neko_render_shader_source_desc_t{.type = (neko_render_shader_stage_type)(i + 1), .source = src};
            }
        }
        lua_pop(L, 1);  // # -1
    }

    neko_assert(j == n);

    neko_shader_t shader_handle = NEKO_DEFAULT_VAL();

    neko_render_shader_desc_t shader_desc = {.sources = sources, .size = n * sizeof(neko_render_shader_source_desc_t), .name = "unknown"};
    strncpy(shader_desc.name, shader_name, sizeof(shader_desc.name) - 1);
    shader_desc.name[sizeof(shader_desc.name) - 1] = '\0';

    shader_handle = neko_render_shader_create(shader_desc);

    neko_safe_free(sources);

    neko_luabind_struct_push_member(L, neko_shader_t, id, &shader_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_render_uniform_create) {

    const_str uniform_name = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    int n = lua_rawlen(L, 2);          //

    neko_render_uniform_layout_desc_t* layouts = (neko_render_uniform_layout_desc_t*)neko_safe_malloc(n * sizeof(neko_render_uniform_layout_desc_t));
    memset(layouts, 0, n * sizeof(neko_render_uniform_layout_desc_t));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);  // 将index=i的元素压入堆栈顶部
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "type");  // # -1
        lua_gettable(L, -2);        // pop # -1
        if (!lua_isnil(L, -1)) {
            neko_luabind_to(L, neko_render_uniform_type, &layouts[i - 1].type, -1);
        }
        lua_pop(L, 1);  // # -1

        // lua_pushstring(L, "name");  // # -1
        // lua_gettable(L, -2);        // pop # -1
        // if (!lua_isnil(L, -1)) {
        //     const_str name = lua_tostring(L, -1);
        //     if (NULL != name) {
        //         strncpy(layouts[i - 1].fname, name, sizeof(layouts[i - 1].fname) - 1);
        //         layouts[i - 1].fname[sizeof(layouts[i - 1].fname) - 1] = '\0';
        //     }
        // }
        // lua_pop(L, 1);  // # -1

        lua_pushstring(L, "count");  // # -1
        lua_gettable(L, -2);         // pop # -1
        if (lua_isinteger(L, -1)) {
            layouts[i - 1].count = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);  // # -1

        lua_pop(L, 1);  // # -1
    }

    neko_render_uniform_desc_t u_desc = {
            // .name = "unknown",
            .layout = layouts,
    };

    if (!!strcmp(uniform_name, "default")) {
        NEKO_STR_CPY(u_desc.name, uniform_name);
    }

    if (lua_gettop(L) == 3) {
        neko_luabind_to(L, neko_render_shader_stage_type, &u_desc.stage, 3);
    }

    neko_uniform_t uniform_handle = NEKO_DEFAULT_VAL();

    // Create uniform
    uniform_handle = neko_render_uniform_create(u_desc);

    neko_safe_free(layouts);

    neko_luabind_struct_push_member(L, neko_uniform_t, id, &uniform_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_render_pipeline_create) {
    const_str pipeline_name = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);              // 检查是否为table
    int n = neko_lua_get_table_pairs_count(L, 2);  //
    const_str pipeline_type[] = {"compute", "layout", "raster"};

    neko_render_pipeline_desc_t pipeline_desc = NEKO_DEFAULT_VAL();

    for (int i = 0; i < NEKO_ARR_SIZE(pipeline_type); i++) {
        lua_pushstring(L, pipeline_type[i]);  // # -1
        lua_gettable(L, -2);                  // pop # -1
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            int n = neko_lua_get_table_pairs_count(L, -1);  //

            switch (neko::hash(pipeline_type[i])) {
                case neko::hash("compute"): {

                    neko_shader_t shader_handle = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "shader");  // # -1
                    lua_gettable(L, -2);          // pop # -1
                    if (!lua_isnil(L, -1)) {
                        neko_luabind_struct_to_member(L, neko_shader_t, id, &shader_handle, -1);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.compute.shader = shader_handle;
                } break;
                case neko::hash("layout"): {
                    neko_render_vertex_attribute_desc_t* vertex_attr = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "attrs");  // # -1
                    lua_gettable(L, -2);         // pop # -1
                    if (!lua_isnil(L, -1)) {
                        vertex_attr = (neko_render_vertex_attribute_desc_t*)lua_touserdata(L, -1);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.layout.attrs = vertex_attr;

                    lua_pushstring(L, "size");  // # -1
                    lua_gettable(L, -2);        // pop # -1
                    if (lua_isinteger(L, -1)) {
                        pipeline_desc.layout.size = (size_t)lua_tointeger(L, -1);
                    } else
                        neko_assert(false);
                    lua_pop(L, 1);  // # -1
                } break;
                case neko::hash("raster"): {

                    neko_shader_t shader_handle = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "shader");  // # -1
                    lua_gettable(L, -2);          // pop # -1
                    if (!lua_isnil(L, -1)) {
                        neko_luabind_struct_to_member(L, neko_shader_t, id, &shader_handle, -1);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.raster.shader = shader_handle;

                    lua_pushstring(L, "index_buffer_element_size");  // # -1
                    lua_gettable(L, -2);                             // pop # -1
                    if (lua_isinteger(L, -1)) {
                        pipeline_desc.raster.index_buffer_element_size = lua_tointeger(L, -1);
                    } else
                        neko_assert(false);
                    lua_pop(L, 1);  // # -1

                } break;
                default:
                    neko_assert(false);
                    break;
            }
        }
        lua_pop(L, 1);  // # -1
    }

    neko_pipeline_t pipeline_handle = NEKO_DEFAULT_VAL();
    pipeline_handle = neko_render_pipeline_create(pipeline_desc);
    neko_luabind_struct_push_member(L, neko_pipeline_t, id, &pipeline_handle);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_vertex_buffer_create) {
    const_str vertex_buffer_name = lua_tostring(L, 1);
    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);
    neko_vbo_t vertex_buffer_handle = NEKO_DEFAULT_VAL();
    neko_render_vertex_buffer_desc_t vertex_buffer_desc = {.data = data, .size = data_size};
    vertex_buffer_handle = neko_render_vertex_buffer_create(vertex_buffer_desc);
    neko_luabind_struct_push_member(L, neko_vbo_t, id, &vertex_buffer_handle);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_index_buffer_create) {
    const_str index_buffer_name = lua_tostring(L, 1);
    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);
    neko_ibo_t index_buffer_handle = NEKO_DEFAULT_VAL();
    neko_render_index_buffer_desc_t index_buffer_desc = {.data = data, .size = data_size};
    index_buffer_handle = neko_render_index_buffer_create(index_buffer_desc);
    neko_luabind_struct_push_member(L, neko_ibo_t, id, &index_buffer_handle);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_vertex_attribute_create) {
    const_str vertex_attribute_name = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int n = neko_lua_get_table_pairs_count(L, 2);  //

    neko_render_vertex_attribute_desc_t* sources = (neko_render_vertex_attribute_desc_t*)neko_safe_malloc(n * sizeof(neko_render_vertex_attribute_desc_t));
    memset(sources, 0, n * sizeof(neko_render_vertex_attribute_desc_t));

    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, 2, i + 1);  // 将index=i的元素压入堆栈顶部
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "name");  // # -1
        lua_gettable(L, -2);        // pop # -1
        if (!lua_isnil(L, -1)) {
            const_str src = lua_tostring(L, -1);  // # -1
            if (src != NULL) {
                NEKO_STR_CPY(sources[i].name, src);
            } else
                neko_assert(false);
        }
        lua_pop(L, 1);  // # -1

        lua_pushstring(L, "format");  // # -1
        lua_gettable(L, -2);          // pop # -1
        if (!lua_isnil(L, -1)) {
            neko_luabind_to(L, neko_render_vertex_attribute_type, &sources[i].format, -1);
        }
        lua_pop(L, 1);  // # -1

        lua_pop(L, 1);  // # -1
    }

    neko_render_vertex_attribute_desc_t* push_userdata = (neko_render_vertex_attribute_desc_t*)lua_newuserdata(L, n * sizeof(neko_render_vertex_attribute_desc_t));
    memcpy(push_userdata, sources, n * sizeof(neko_render_vertex_attribute_desc_t));

    neko_safe_free(sources);

    lua_pushinteger(L, n * sizeof(neko_render_vertex_attribute_desc_t));

    return 2;
}

LUA_FUNCTION(__neko_bind_render_storage_buffer_create) {

    const_str storage_buffer_name = lua_tostring(L, 1);

    void* data = lua_touserdata(L, 2);
    size_t data_size = lua_tointeger(L, 3);

    neko_storage_buffer_t storage_buffer_handle = NEKO_DEFAULT_VAL();

    neko_render_storage_buffer_desc_t storage_buffer_desc = {.data = data, .size = data_size, .name = "unknown", .usage = R_BUFFER_USAGE_DYNAMIC};

    if (storage_buffer_name != NULL) {
        NEKO_STR_CPY(storage_buffer_desc.name, storage_buffer_name);
    }

    storage_buffer_handle = neko_render_storage_buffer_create(storage_buffer_desc);

    neko_luabind_struct_push_member(L, neko_storage_buffer_t, id, &storage_buffer_handle);

    return 1;
}

LUA_FUNCTION(__neko_bind_render_pipeline_bind) {
    neko_pipeline_t pipeline_handle = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_pipeline_t, id, &pipeline_handle, 1);
    neko_render_pipeline_bind(&ENGINE_INTERFACE()->cb, pipeline_handle);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_apply_bindings) {

    neko_render_bind_desc_t binds = NEKO_DEFAULT_VAL();

    const_str pipeline_type[] = {"uniforms", "image_buffers", "storage_buffers", "vertex_buffers", "index_buffers"};

    neko_render_bind_uniform_desc_t* u_desc = nullptr;
    neko_render_bind_image_buffer_desc_t* ib_desc = nullptr;
    neko_render_bind_storage_buffer_desc_t* sb_desc = nullptr;
    neko_render_bind_vertex_buffer_desc_t* vbo_desc = nullptr;
    neko_render_bind_index_buffer_desc_t* ibo_desc = nullptr;

    luaL_checktype(L, -1, LUA_TTABLE);

    f32 data;  // TODO

    for (int i = 0; i < NEKO_ARR_SIZE(pipeline_type); i++) {
        lua_pushstring(L, pipeline_type[i]);  // # -1
        lua_gettable(L, -2);                  // pop # -1
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            int n = neko_lua_get_table_pairs_count(L, -1);  //

            switch (neko::hash(pipeline_type[i])) {
                case neko::hash("uniforms"): {

                    u_desc = (neko_render_bind_uniform_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_uniform_desc_t));
                    memset(u_desc, 0, n * sizeof(neko_render_bind_uniform_desc_t));

                    binds.uniforms.desc = u_desc;
                    binds.uniforms.size = n == 1 ? 0 : n * sizeof(neko_render_bind_uniform_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "uniform");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (!lua_isnil(L, -1))
                            neko_luabind_struct_to_member(L, neko_uniform_t, id, &u_desc[i - 1].uniform, -1);
                        else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "data");  // # -1
                        lua_gettable(L, -2);        // pop # -1
                        if (!lua_isnil(L, -1)) {
                            int type = lua_type(L, -1);
                            switch (type) {
                                case LUA_TUSERDATA:
                                    u_desc[i - 1].data = lua_touserdata(L, -1);
                                    break;
                                case LUA_TNUMBER:
                                    data = lua_tonumber(L, -1);
                                    u_desc[i - 1].data = &data;
                                    break;
                                default:
                                    neko_assert(false);  // TODO
                                    break;
                            }
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            u_desc[i - 1].binding = neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("image_buffers"): {
                    ib_desc = (neko_render_bind_image_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_image_buffer_desc_t));
                    memset(ib_desc, 0, n * sizeof(neko_render_bind_image_buffer_desc_t));

                    binds.image_buffers.desc = ib_desc;
                    binds.image_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_image_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "tex");  // # -1
                        lua_gettable(L, -2);       // pop # -1
                        if (!lua_isnil(L, -1))
                            neko_luabind_struct_to_member(L, neko_texture_t, id, &ib_desc[i - 1].tex, -1);
                        else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        ib_desc[i - 1].access = R_ACCESS_WRITE_ONLY;

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            ib_desc[i - 1].binding = neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("storage_buffers"): {
                    sb_desc = (neko_render_bind_storage_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_storage_buffer_desc_t));
                    memset(sb_desc, 0, n * sizeof(neko_render_bind_storage_buffer_desc_t));

                    binds.storage_buffers.desc = sb_desc;
                    binds.storage_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_storage_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_luabind_struct_to_member(L, neko_storage_buffer_t, id, &sb_desc[i - 1].buffer, -1);
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            sb_desc[i - 1].binding = neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("vertex_buffers"): {
                    vbo_desc = (neko_render_bind_vertex_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_vertex_buffer_desc_t));
                    memset(vbo_desc, 0, n * sizeof(neko_render_bind_vertex_buffer_desc_t));

                    binds.vertex_buffers.desc = vbo_desc;
                    binds.vertex_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_vertex_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_luabind_struct_to_member(L, neko_vbo_t, id, &vbo_desc[i - 1].buffer, -1);
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case neko::hash("index_buffers"): {
                    ibo_desc = (neko_render_bind_index_buffer_desc_t*)neko_safe_malloc(n * sizeof(neko_render_bind_index_buffer_desc_t));
                    memset(ibo_desc, 0, n * sizeof(neko_render_bind_index_buffer_desc_t));

                    binds.index_buffers.desc = ibo_desc;
                    binds.index_buffers.size = n == 1 ? 0 : n * sizeof(neko_render_bind_index_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            neko_luabind_struct_to_member(L, neko_ibo_t, id, &ibo_desc[i - 1].buffer, -1);
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                default:
                    break;
            }
        }
        lua_pop(L, 1);  // # -1
    }

    neko_render_apply_bindings(&ENGINE_INTERFACE()->cb, &binds);

    if (u_desc) neko_safe_free(u_desc);
    if (ib_desc) neko_safe_free(ib_desc);
    if (sb_desc) neko_safe_free(sb_desc);
    if (vbo_desc) neko_safe_free(vbo_desc);
    if (ibo_desc) neko_safe_free(ibo_desc);

    return 0;
}

static int test_tex(lua_State* L) {

#define ROW_COL_CT 10

    neko_color_t c0 = NEKO_COLOR_WHITE;
    neko_color_t c1 = neko_color(20, 50, 150, 255);
    // neko_color_t pixels[ROW_COL_CT * ROW_COL_CT] = NEKO_DEFAULT_VAL();

    neko_color_t* pixels = (neko_color_t*)lua_newuserdata(L, sizeof(neko_color_t) * ROW_COL_CT * ROW_COL_CT);

    for (u32 r = 0; r < ROW_COL_CT; ++r) {
        for (u32 c = 0; c < ROW_COL_CT; ++c) {
            const bool re = (r % 2) == 0;
            const bool ce = (c % 2) == 0;
            u32 idx = r * ROW_COL_CT + c;
            pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
        }
    }

    return 1;
}

LUA_FUNCTION(__neko_bind_render_dispatch_compute) {
    f32 x_groups = lua_tonumber(L, 1);
    f32 y_groups = lua_tonumber(L, 2);
    f32 z_groups = lua_tonumber(L, 3);
    neko_render_dispatch_compute(&ENGINE_INTERFACE()->cb, x_groups, y_groups, z_groups);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_texture_create) {
    u32 w = lua_tointeger(L, 1);
    u32 h = lua_tointeger(L, 2);
    neko_render_texture_desc_t texture_desc = {
            .width = w,                             // 纹理的宽度
            .height = h,                            // 纹理的高度
            .format = R_TEXTURE_FORMAT_RGBA8,       // 纹理数据的格式
            .wrap_s = R_TEXTURE_WRAP_REPEAT,        // 纹理 s 轴的包裹类型
            .wrap_t = R_TEXTURE_WRAP_REPEAT,        // 纹理 t 轴的包裹类型
            .min_filter = R_TEXTURE_FILTER_LINEAR,  // 纹理缩小过滤器
            .mag_filter = R_TEXTURE_FILTER_LINEAR   // 纹理放大滤镜
    };
    if (lua_gettop(L) == 3) {
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            lua_pushstring(L, "type");  // # -1
            lua_gettable(L, -2);        // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_type, &texture_desc.type, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "format");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_format_type, &texture_desc.format, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_s");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_wrapping_type, &texture_desc.wrap_s, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_t");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_wrapping_type, &texture_desc.wrap_t, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "min_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_filtering_type, &texture_desc.min_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "mag_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, neko_render_texture_filtering_type, &texture_desc.mag_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "data");  // # -1
            lua_gettable(L, -2);        // pop # -1
            if (!lua_isnil(L, -1)) {
                void* data = lua_touserdata(L, -1);
                texture_desc.data[0] = data;
            }
            lua_pop(L, 1);  // # -1
        }
    }
    neko_texture_t texture = NEKO_DEFAULT_VAL();
    texture = neko_render_texture_create(texture_desc);
    neko_luabind_struct_push_member(L, neko_texture_t, id, &texture);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_texture_fini) {
    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_texture_t, id, &rt, 1);
    neko_render_texture_fini(rt);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_renderpass_create) {

    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);

    neko_texture_t rt = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_texture_t, id, &rt, 2);

    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    rp = neko_render_renderpass_create(neko_render_renderpass_desc_t{
            .fbo = fbo,               // Frame buffer to bind for render pass
            .color = &rt,             // Color buffer array to bind to frame buffer
            .color_size = sizeof(rt)  // Size of color attachment array in bytes
    });
    neko_luabind_struct_push_member(L, neko_renderpass_t, id, &rp);
    return 1;
}

LUA_FUNCTION(__neko_bind_render_renderpass_fini) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_render_renderpass_fini(rp);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_renderpass_begin) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, rp);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_renderpass_end) {
    neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_draw) {
    neko_render_draw_desc_t draw_desc = NEKO_DEFAULT_VAL();

    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "start");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1))
            draw_desc.start = lua_tointeger(L, -1);
        else
            neko_assert(false);
        lua_pop(L, 1);

        lua_pushstring(L, "count");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1))
            draw_desc.count = lua_tointeger(L, -1);
        else
            neko_assert(false);
        lua_pop(L, 1);

        lua_pushstring(L, "instances");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1)) draw_desc.instances = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_pushstring(L, "base_vertex");
        lua_gettable(L, -2);
        if (lua_isinteger(L, -1)) draw_desc.base_vertex = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }

    neko_render_draw(&ENGINE_INTERFACE()->cb, draw_desc);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_set_viewport) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 w = lua_tonumber(L, 3);
    f32 h = lua_tonumber(L, 4);
    neko_render_set_viewport(&ENGINE_INTERFACE()->cb, x, y, w, h);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_clear) {
    f32 r = lua_tonumber(L, 1);
    f32 g = lua_tonumber(L, 2);
    f32 b = lua_tonumber(L, 3);
    f32 a = lua_tonumber(L, 4);
    neko_render_clear_action_t clear = {.color = {r, g, b, a}};
    neko_render_clear(&ENGINE_INTERFACE()->cb, clear);
    return 0;
}

LUA_FUNCTION(__neko_bind_render_display_size) {
    neko_vec2 v1 = neko_game()->DisplaySize;
    // lua2struct::pack_struct<neko_vec2, 2>(L, v1);
    lua_pushnumber(L, v1.x);
    lua_pushnumber(L, v1.y);
    return 2;
}



inline void neko_register_test(lua_State* L) {

    // neko::lua_bind::bind("neko_tiled_get_objects", &__neko_bind_tiled_get_objects);

    neko_lua_enum(L, neko_projection_type);
    neko_lua_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_ORTHOGRAPHIC);
    neko_lua_enum_value(L, neko_projection_type, NEKO_PROJECTION_TYPE_PERSPECTIVE);

    neko_lua_enum(L, neko_render_primitive_type);
    neko_lua_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_LINES);
    neko_lua_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_TRIANGLES);
    neko_lua_enum_value(L, neko_render_primitive_type, R_PRIMITIVE_QUADS);

    neko_luabind_struct(L, neko_vec2);
    neko_luabind_struct_member(L, neko_vec2, x, f32);
    neko_luabind_struct_member(L, neko_vec2, y, f32);
    neko_luabind_struct_member(L, neko_vec2, xy, f32[2]);

    neko_luabind_struct(L, neko_vec3);
    neko_luabind_struct_member(L, neko_vec3, x, f32);
    neko_luabind_struct_member(L, neko_vec3, y, f32);
    neko_luabind_struct_member(L, neko_vec3, z, f32);
    neko_luabind_struct_member(L, neko_vec3, xyz, f32[3]);

    neko_luabind_struct(L, neko_quat);
    neko_luabind_struct_member(L, neko_quat, x, f32);
    neko_luabind_struct_member(L, neko_quat, y, f32);
    neko_luabind_struct_member(L, neko_quat, z, f32);
    neko_luabind_struct_member(L, neko_quat, w, f32);

    neko_luabind_struct(L, neko_vqs);
    neko_luabind_struct_member(L, neko_vqs, position, neko_vec3);
    neko_luabind_struct_member(L, neko_vqs, translation, neko_vec3);
    neko_luabind_struct_member(L, neko_vqs, rotation, neko_quat);
    neko_luabind_struct_member(L, neko_vqs, scale, neko_vec3);

    neko_luabind_struct(L, neko_camera_t);
    neko_luabind_struct_member(L, neko_camera_t, transform, neko_vqs);
    neko_luabind_struct_member(L, neko_camera_t, fov, f32);
    neko_luabind_struct_member(L, neko_camera_t, aspect_ratio, f32);
    neko_luabind_struct_member(L, neko_camera_t, near_plane, f32);
    neko_luabind_struct_member(L, neko_camera_t, far_plane, f32);
    neko_luabind_struct_member(L, neko_camera_t, ortho_scale, f32);
    neko_luabind_struct_member(L, neko_camera_t, proj_type, neko_projection_type);

    neko_luabind_struct(L, neko_framebuffer_t);
    neko_luabind_struct_member(L, neko_framebuffer_t, id, unsigned int);

    neko_luabind_struct(L, neko_texture_t);
    neko_luabind_struct_member(L, neko_texture_t, id, unsigned int);

    neko_luabind_struct(L, neko_pipeline_t);
    neko_luabind_struct_member(L, neko_pipeline_t, id, unsigned int);

    neko_luabind_struct(L, neko_uniform_t);
    neko_luabind_struct_member(L, neko_uniform_t, id, unsigned int);

    neko_luabind_struct(L, neko_storage_buffer_t);
    neko_luabind_struct_member(L, neko_storage_buffer_t, id, unsigned int);

    neko_luabind_struct(L, neko_shader_t);
    neko_luabind_struct_member(L, neko_shader_t, id, unsigned int);

    neko_luabind_struct(L, neko_vbo_t);
    neko_luabind_struct_member(L, neko_vbo_t, id, unsigned int);

    neko_luabind_struct(L, neko_ibo_t);
    neko_luabind_struct_member(L, neko_ibo_t, id, unsigned int);

    neko_luabind_struct(L, neko_renderpass_t);
    neko_luabind_struct_member(L, neko_renderpass_t, id, unsigned int);

    // neko_luabind_struct(L, neko_sound_playing_sound_t);
    // neko_luabind_struct_member(L, neko_sound_playing_sound_t, id, unsigned long long);

    neko_lua_enum(L, neko_render_vertex_attribute_type);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT4);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT3);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT2);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT4);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT3);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT2);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_UINT);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE4);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE3);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE2);
    neko_lua_enum_value(L, neko_render_vertex_attribute_type, R_VERTEX_ATTRIBUTE_BYTE);

    neko_lua_enum(L, neko_render_texture_type);
    neko_lua_enum_value(L, neko_render_texture_type, R_TEXTURE_2D);
    neko_lua_enum_value(L, neko_render_texture_type, R_TEXTURE_CUBEMAP);

    neko_lua_enum(L, neko_render_shader_stage_type);
    neko_lua_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_VERTEX);
    neko_lua_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_FRAGMENT);
    neko_lua_enum_value(L, neko_render_shader_stage_type, R_SHADER_STAGE_COMPUTE);

    neko_lua_enum(L, neko_render_texture_wrapping_type);
    neko_lua_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_REPEAT);
    neko_lua_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_MIRRORED_REPEAT);
    neko_lua_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_CLAMP_TO_EDGE);
    neko_lua_enum_value(L, neko_render_texture_wrapping_type, R_TEXTURE_WRAP_CLAMP_TO_BORDER);

    neko_lua_enum(L, neko_render_texture_filtering_type);
    neko_lua_enum_value(L, neko_render_texture_filtering_type, R_TEXTURE_FILTER_NEAREST);
    neko_lua_enum_value(L, neko_render_texture_filtering_type, R_TEXTURE_FILTER_LINEAR);

    neko_lua_enum(L, neko_render_texture_format_type);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGB8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RG8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R32);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R32F);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA16F);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_RGBA32F);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_A8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_R8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH16);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH24);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH32F);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH24_STENCIL8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_DEPTH32F_STENCIL8);
    neko_lua_enum_value(L, neko_render_texture_format_type, R_TEXTURE_FORMAT_STENCIL8);

    neko_lua_enum(L, neko_render_uniform_type);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_FLOAT);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_INT);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC2);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC3);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_VEC4);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_MAT4);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_SAMPLER2D);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_USAMPLER2D);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_SAMPLERCUBE);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_IMAGE2D_RGBA32F);
    neko_lua_enum_value(L, neko_render_uniform_type, R_UNIFORM_BLOCK);
}

#endif

#define NEKO_LUA_INSPECT_ITER(NAME)                                                                          \
    LUA_FUNCTION(__neko_bind_inspect_##NAME##_next) {                                                        \
        neko_gl_data_t *ogl = (neko_gl_data_t *)lua_touserdata(L, lua_upvalueindex(1));                      \
        neko_slot_array_iter *it = (neko_slot_array_iter *)lua_touserdata(L, lua_upvalueindex(2));           \
        if (!neko_slot_array_iter_valid(ogl->NAME, *it)) {                                                   \
            return 0;                                                                                        \
        }                                                                                                    \
        auto s = neko_slot_array_iter_get(ogl->NAME, *it);                                                   \
        neko_slot_array_iter_advance(ogl->NAME, *it);                                                        \
        lua_pushinteger(L, s.id);                                                                            \
        return 1;                                                                                            \
    }                                                                                                        \
                                                                                                             \
    LUA_FUNCTION(__neko_bind_inspect_##NAME##_iterator) {                                                    \
        neko_gl_data_t *ogl = neko_render_userdata();                                                        \
        neko_slot_array_iter *it = (neko_slot_array_iter *)lua_newuserdata(L, sizeof(neko_slot_array_iter)); \
        *it = neko_slot_array_iter_new(ogl->NAME);                                                           \
        lua_pushlightuserdata(L, ogl);                                                                       \
        lua_pushvalue(L, -2);                                                                                \
        lua_pushcclosure(L, __neko_bind_inspect_##NAME##_next, 2);                                           \
        return 1;                                                                                            \
    }

#define NEKO_LUA_INSPECT_REG(NAME) \
    { "inspect_" #NAME "_iter", __neko_bind_inspect_##NAME##_iterator }

// NEKO_LUA_INSPECT_ITER(shaders)
// NEKO_LUA_INSPECT_ITER(textures)
// NEKO_LUA_INSPECT_ITER(vertex_buffers)
// NEKO_LUA_INSPECT_ITER(uniform_buffers)
// NEKO_LUA_INSPECT_ITER(storage_buffers)
// NEKO_LUA_INSPECT_ITER(index_buffers)
// NEKO_LUA_INSPECT_ITER(frame_buffers)
// NEKO_LUA_INSPECT_ITER(uniforms)
// NEKO_LUA_INSPECT_ITER(pipelines)
// NEKO_LUA_INSPECT_ITER(renderpasses)

#if 0
LUA_FUNCTION(__neko_bind_cvar) {
    const_str name = lua_tostring(L, 1);

    int args = lua_gettop(L);

    // 检查是否已经存在
    neko_cvar_t* cv = __neko_config_get(name);
    if (NULL != cv) {
        if (args == 2) {
            neko_cvar_set(cv, neko::neko_lua_to<const_str>(L, 2));
            return 0;
        } else if (args == 1) {  // 读取
            switch (cv->type) {
                case __NEKO_CONFIG_TYPE_INT:
                    lua_pushinteger(L, cv->value.i);
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    lua_pushnumber(L, cv->value.f);
                    break;
                case __NEKO_CONFIG_TYPE_STRING:
                    lua_pushstring(L, cv->value.s);
                    break;
                case __NEKO_CONFIG_TYPE_COUNT:
                default:
                    break;
            }
            return 1;
        } else {
            const_str error_message = "__neko_bind_cvar failed";
            lua_pushstring(L, error_message);  // 将错误信息压入堆栈
            return lua_error(L);               // 抛出lua错误
        }
    } else {
        if (args == 1) {
            lua_pushnil(L);
            return 1;            // 如果不存在就返回nil
        } else if (args == 3) {  // 创建

            neko_cvar_type cval = neko_cvar_type::__NEKO_CONFIG_TYPE_COUNT;

            if (lua_type(L, 2) == LUA_TSTRING) {
                const_str type = lua_tostring(L, 2);
                lua_pushstring(ENGINE_LUA(), type);
                neko_luabind_to(ENGINE_LUA(), neko_cvar_type, &cval, -1);
                lua_pop(ENGINE_LUA(), 1);
            } else if (lua_isinteger(L, 2)) {
                int type = lua_tointeger(L, 2);
                cval = (neko_cvar_type)type;
            }

            switch (cval) {
                case __NEKO_CONFIG_TYPE_INT:
                    neko_cvar_lnew(name, cval, neko::neko_lua_to<int>(L, 3));
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    neko_cvar_lnew(name, cval, neko::neko_lua_to<float>(L, 3));
                    break;
                case __NEKO_CONFIG_TYPE_STRING:
                    neko_cvar_lnew_str(name, cval, neko::neko_lua_to<const_str>(L, 3));
                    break;
                case __NEKO_CONFIG_TYPE_COUNT:
                default:
                    console_log(std::format("__neko_bind_cvar_new with a unknown type {0} {1}", name, (u8)cval).c_str());
                    break;
            }

            return 0;
        } else {
            const_str error_message = "__neko_bind_cvar failed";
            lua_pushstring(L, error_message);  // 将错误信息压入堆栈
            return lua_error(L);               // 抛出lua错误
        }
    }

    return 0;  // unreachable
}
#endif

LUA_FUNCTION(__neko_bind_print) {
    std::string str;
    int n = lua_gettop(L);
    int i;
    for (i = 1; i <= n; i++) {
        size_t l;
        const_str s = luaL_tolstring(L, i, &l);
        if (i > 1) str.append("\t");
        str.append(std::string(s, l));
        lua_pop(L, 1);
    }
    console_log("[lua] %s", str.c_str());
    return 0;
}

LUA_FUNCTION(__neko_bind_pack_build) {

    const_str path = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    lua_len(L, 2);                     // 获取table的长度
    int n = lua_tointeger(L, -1);      //
    lua_pop(L, 1);                     // 弹出长度值

    const_str *item_paths = (const_str *)mem_alloc(n * sizeof(const_str));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);                 // 将index=i的元素压入堆栈顶部
        const_str str = lua_tostring(L, -1);  // # -1
        if (str != NULL) {
            item_paths[i - 1] = str;
        }
        lua_pop(L, 1);  // # -1
    }

    bool ok = neko_pak_build(path, n, item_paths, true);

    mem_free(item_paths);

    if (!ok) {
        const_str error_message = "__neko_bind_pack_build failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    return 0;
}

LUA_FUNCTION(__neko_bind_pack_info) {
    const_str path = lua_tostring(L, 1);
    i32 buildnum;
    u64 item_count;
    bool ok = neko_pak_info(path, &buildnum, &item_count);
    if (ok) {
        lua_pushinteger(L, buildnum);
        lua_pushinteger(L, item_count);
        return 2;
    } else
        return 0;
}

LUA_FUNCTION(__neko_bind_vfs_read_file) {
    const_str path = lua_tostring(L, 1);

    String str;

    bool ok = vfs_read_entire_file(&str, path);

    if (ok) {
        const_str data = str.data;
        lua_pushstring(L, data);
        neko_defer(mem_free(str.data));
        return 1;
    } else {
        const_str error_message = "todo";
        lua_pushstring(L, error_message);
        return lua_error(L);
    }
}

// 返回包含路径和 isDirectory 对的表
int __neko_ls(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        console_log("invalid lua argument");
        return 0;
    }
    auto string = lua_tostring(L, 1);
    if (!std::filesystem::is_directory(string)) {
        console_log(std::format("{0} is not directory", string).c_str());
        return 0;
    }

    lua_newtable(L);
    int i = 0;
    for (auto &p : std::filesystem::directory_iterator(string)) {
        lua_pushnumber(L, i + 1);  // parent table index
        lua_newtable(L);
        lua_pushstring(L, "path");
        lua_pushstring(L, p.path().generic_string().c_str());
        lua_settable(L, -3);
        lua_pushstring(L, "isDirectory");
        lua_pushboolean(L, p.is_directory());
        lua_settable(L, -3);
        lua_settable(L, -3);
        i++;
    }
    return 1;
}

bool __neko_dolua(const_str file) { return neko::neko_lua_dofile(ENGINE_LUA(), file); }

inline void neko_register_common(lua_State *L) {

    // neko::lua_bind::bind("neko_dolua", &__neko_dolua);

    // neko::lua_bind::bind("neko_hash_str", +[](const_str str) { return neko_hash_str(str); });
    // neko::lua_bind::bind("neko_hash_str64", +[](const_str str) { return neko_hash_str64(str); });

    neko_lua_enum(L, AssetKind);
    neko_lua_enum_value(L, AssetKind, AssetKind_None);
    neko_lua_enum_value(L, AssetKind, AssetKind_LuaRef);
    neko_lua_enum_value(L, AssetKind, AssetKind_Image);
    neko_lua_enum_value(L, AssetKind, AssetKind_Sprite);
    neko_lua_enum_value(L, AssetKind, AssetKind_Tilemap);

    lua_register(L, "__neko_ls", __neko_ls);
}

#if 0

int register_mt_aseprite_renderer(lua_State* L) {
    luaL_Reg reg[] = {
            {"__gc", __neko_bind_aseprite_render_gc},
            {"create", __neko_bind_aseprite_render_create},
            {"update", __neko_bind_aseprite_render_update},
            {"render", __neko_bind_aseprite_render},
            {"update_animation", __neko_bind_aseprite_render_update_animation},
            {NULL, NULL},
    };
    luaL_newmetatable(L, "mt_aseprite_renderer");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);            // # -1 复制一份 为了让 neko 主表设定
    lua_setfield(L, -2, "__index");  // # -2
    return 1;
}

int register_mt_aseprite(lua_State* L) {
    luaL_Reg reg[] = {
            {"__gc", __neko_bind_aseprite_gc},
            {"create", __neko_bind_aseprite_create},
            {NULL, NULL},
    };
    luaL_newmetatable(L, "mt_aseprite");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);            // # -1 复制一份 为了让 neko 主表设定
    lua_setfield(L, -2, "__index");  // # -2
    return 1;
}

#endif

typedef struct neko_lua_profiler_data {
    int linedefined;
    char source[LUA_IDSIZE];
} neko_lua_profiler_data;

typedef struct neko_lua_profiler_count {
    u32 total;
    u32 index;
    // u64 t;
} neko_lua_profiler_count;

static void neko_lua_profiler_hook(lua_State *L, lua_Debug *ar) {
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, L) != LUA_TUSERDATA) {
        lua_pop(L, 1);
        return;
    }
    neko_lua_profiler_count *p = (neko_lua_profiler_count *)lua_touserdata(L, -1);
    lua_pop(L, 1);
    neko_lua_profiler_data *log = (neko_lua_profiler_data *)(p + 1);
    int index = p->index++;
    while (index >= p->total) {
        index -= p->total;
    }
    if (lua_getinfo(L, "S", ar) != 0) {
        log[index].linedefined = ar->linedefined;
        strcpy(log[index].source, ar->short_src);
    } else {
        log[index].linedefined = 1;
        strcpy(log[index].source, "[unknown]");
    }
}

static int neko_lua_profiler_start(lua_State *L) {
    lua_State *cL = L;  // 支持多线程
    int args = 0;
    if (lua_isthread(L, 1)) {
        cL = lua_tothread(L, 1);
        args = 1;
    }
    int c = luaL_optinteger(L, args + 1, 1000);    // 记录数量
    int ival = luaL_optinteger(L, args + 2, 100);  // 检测间隔
    neko_lua_profiler_count *p = (neko_lua_profiler_count *)lua_newuserdata(L, sizeof(neko_lua_profiler_count) + c * sizeof(neko_lua_profiler_data));
    p->total = c;
    p->index = 0;
    // p->t = neko_os_elapsed_time();
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, cL);
    lua_sethook(cL, neko_lua_profiler_hook, LUA_MASKCOUNT, ival);
    return 0;
}

static int neko_lua_profiler_stop(lua_State *L) {
    lua_State *cL = L;  // 支持多线程
    if (lua_isthread(L, 1)) {
        cL = lua_tothread(L, 1);
    }
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, cL) != LUA_TNIL) {
        lua_pushnil(L);
        lua_rawsetp(L, LUA_REGISTRYINDEX, cL);
        lua_sethook(cL, NULL, 0, 0);
    } else {
        return luaL_error(L, "thread profiler not begin");
    }
    return 0;
}

static int neko_lua_profiler_info(lua_State *L) {
    lua_State *cL = L;
    if (lua_isthread(L, 1)) {
        cL = lua_tothread(L, 1);
    }
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, cL) != LUA_TUSERDATA) {
        return luaL_error(L, "thread profiler not begin");
    }
    neko_lua_profiler_count *p = (neko_lua_profiler_count *)lua_touserdata(L, -1);
    neko_lua_profiler_data *log = (neko_lua_profiler_data *)(p + 1);
    lua_newtable(L);
    int n = (p->index > p->total) ? p->total : p->index;
    int i;
    for (i = 0; i < n; i++) {
        luaL_getsubtable(L, -1, log[i].source);
        lua_rawgeti(L, -1, log[i].linedefined);
        int c = lua_tointeger(L, -1);
        lua_pushinteger(L, c + 1);
        // subtbl, c, c + 1
        lua_rawseti(L, -3, log[i].linedefined);
        lua_pop(L, 2);
    }
    lua_pushinteger(L, p->index);
    // lua_pushinteger(L, neko_os_elapsed_time() - p->t);
    return 2;
}

LUA_FUNCTION(__neko_bind_ecs_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");
    return 1;
}

void createStructTables(lua_State *L);

static neko_luaref getLr(lua_State *L) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    neko_luaref ref;
    ref.refL = (luaref)lua_touserdata(L, 1);
    return ref;
}

static int ref_init(lua_State *L) {
    neko_luaref ref;
    ref.make(L);
    lua_pushlightuserdata(L, (void *)ref.refL);
    return 1;
}

static int ref_close(lua_State *L) {
    neko_luaref ref = getLr(L);
    ref.fini();
    return 0;
}

static int ref_isvalid(lua_State *L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    lua_pushboolean(L, ref.isvalid(r));
    return 1;
}

static int ref_ref(lua_State *L) {
    neko_luaref ref = getLr(L);
    lua_settop(L, 2);
    int r = ref.ref(L);
    if (r == LUA_NOREF) {
        return luaL_error(L, "Too many refs.");
    }
    if (r <= 1) {
        return luaL_error(L, "Unexpected error.");
    }
    lua_pushinteger(L, r);
    return 1;
}

static int ref_unref(lua_State *L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    ref.unref(r);
    return 0;
}

static int ref_get(lua_State *L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    if (!ref.isvalid(r)) {
        return luaL_error(L, "invalid ref: %d", r);
    }
    ref.get(L, r);
    return 1;
}

static int ref_set(lua_State *L) {
    neko_luaref ref = getLr(L);
    int r = (int)luaL_checkinteger(L, 2);
    lua_settop(L, 3);
    if (!ref.isvalid(r)) {
        return luaL_error(L, "invalid ref: %d", r);
    }
    ref.set(L, r);
    return 1;
}

LUA_FUNCTION(__neko_bind_w_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);
    return 1;
}

static int __neko_w_lua_get_com(lua_State *L) {
    App *w = (App *)luaL_checkudata(L, W_LUA_REGISTRY_CONST::W_CORE_IDX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);
    lua_getiuservalue(L, 1, W_LUA_UPVALUES::NEKO_W_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, W_LUA_UPVALUES::NEKO_W_UPVAL_N);
    return 2;
}

static int __neko_w_lua_gc(lua_State *L) {
    App *w = (App *)luaL_checkudata(L, W_LUA_REGISTRY_CONST::W_CORE_IDX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);
    // ecs_fini_i(w);
    console_log("App __gc %p", w);
    return 0;
}

void neko_w_init() {

    lua_State *L = ENGINE_LUA();

    App *ins = (App *)lua_newuserdatauv(L, sizeof(App), NEKO_W_UPVAL_N);  // # -1
    // ins = neko_instance();

    if (luaL_getmetatable(L, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME) == LUA_TNIL) {  // # -2

        // clang-format off
        luaL_Reg ins_mt[] = {
            {"__gc", __neko_w_lua_gc}, 
            {"get_com", __neko_w_lua_get_com}, 
            {NULL, NULL}
        };
        // clang-format on

        lua_pop(L, 1);                // # pop -2
        luaL_newlibtable(L, ins_mt);  // # -2
        luaL_setfuncs(L, ins_mt, 0);
        lua_pushvalue(L, -1);                                                      // # -3
        lua_setfield(L, -2, "__index");                                            // pop -3
        lua_pushstring(L, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);                   // # -3
        lua_setfield(L, -2, "__name");                                             // pop -3
        lua_pushvalue(L, -1);                                                      // # -3
        lua_setfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);  // pop -3
    }
    lua_setmetatable(L, -2);  // pop -2

    lua_newtable(L);                                            // # 2
    lua_pushstring(L, W_LUA_REGISTRY_CONST::CVAR_MAP);          // # 3
    lua_createtable(L, 0, W_LUA_REGISTRY_CONST::CVAR_MAP_MAX);  // # 4
    lua_settable(L, -3);
    lua_setiuservalue(L, -2, NEKO_W_COMPONENTS_NAME);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, -2, NEKO_W_UPVAL_N);

    // lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);

    neko_assert(lua_gettop(L) == 0);

    lua_register(L, "neko_w_f", __neko_bind_w_f);

    neko_w_lua_variant<i64> version("neko_engine_version", neko_buildnum());

    neko_assert(lua_gettop(L) == 0);
}

LUA_FUNCTION(from_registry) {
    int n = lua_gettop(L);
    if (n == 1) {
        const char *key = luaL_checkstring(L, 1);
        lua_getfield(L, LUA_REGISTRYINDEX, key);
    } else {
        int idx = luaL_checkinteger(L, 1);
        const char *key = luaL_checkstring(L, 2);
        lua_rawgeti(L, LUA_REGISTRYINDEX, idx);
        lua_getfield(L, -1, key);
    }
    return 1;
}

LUA_FUNCTION(ltype) {
    lua_pushvalue(L, lua_upvalueindex(lua_type(L, 1) + 1));
    return 1;
}

static void typeclosure(lua_State *L) {
    static const char *typenames[] = {
            "nil",       // 0
            "boolean",   // 1
            "userdata",  // 2
            "number",    // 3
            "string",    // 4
            "table",     // 5
            "function",  // 6
            "userdata",  // 7
            "thread",    // 8
            "proto",     // 9
    };
    size_t i;
    const size_t n = array_size(typenames);
    for (i = 0; i < n; i++) {
        lua_pushstring(L, typenames[i]);
    }
    lua_pushcclosure(L, ltype, n);
}

static int open_embed_core(lua_State *L) {

    luaL_checkversion(L);

    luaL_Reg reg[] = {

            {"ecs_f", __neko_bind_ecs_f},

            {"gameobject_inspect", __neko_bind_gameobject_inspect},

            {"callback_save", __neko_bind_callback_save},
            {"callback_call", __neko_bind_callback_call},

            {"vfs_read_file", __neko_bind_vfs_read_file},

            {"print", __neko_bind_print},

            {"profiler_start", neko_lua_profiler_start},
            {"profiler_stop", neko_lua_profiler_stop},
            {"profiler_info", neko_lua_profiler_info},

            // luaref
            {"ref_init", ref_init},
            {"ref_close", ref_close},
            {"ref_isvalid", ref_isvalid},
            {"ref_ref", ref_ref},
            {"ref_unref", ref_unref},
            {"ref_get", ref_get},
            {"ref_set", ref_set},

            // pak
            {"pak_build", __neko_bind_pack_build},
            {"pak_info", __neko_bind_pack_info},

            // reg
            {"from_registry", from_registry},

            {"nameof", __neko_bind_nameof},

            // {"tiled_create", __neko_bind_tiled_create},
            // {"tiled_render", __neko_bind_tiled_render},
            // {"tiled_end", __neko_bind_tiled_end},
            // {"tiled_load", __neko_bind_tiled_load},
            // {"tiled_unload", __neko_bind_tiled_unload},

            // {"pixelui_create", __neko_bind_pixelui_create},
            // {"pixelui_update", __neko_bind_pixelui_update},
            // {"pixelui_end", __neko_bind_pixelui_end},
            // {"pixelui_tex", __neko_bind_pixelui_tex},

            // {"gfxt_create", __neko_bind_gfxt_create},
            // {"gfxt_update", __neko_bind_gfxt_update},
            // {"gfxt_end", __neko_bind_gfxt_end},

            // {"fontbatch_create", __neko_bind_fontbatch_create},
            // {"fontbatch_draw", __neko_bind_fontbatch_draw},
            // {"fontbatch_text", __neko_bind_fontbatch_text},

            // {"sprite_batch_create", __neko_bind_sprite_batch_create},
            // {"sprite_batch_render_ortho", __neko_bind_sprite_batch_render_ortho},
            // {"sprite_batch_render_begin", __neko_bind_sprite_batch_render_begin},
            // {"sprite_batch_render_end", __neko_bind_sprite_batch_render_end},
            // {"sprite_batch_make_sprite", __neko_bind_sprite_batch_make_sprite},
            // {"sprite_batch_push_sprite", __neko_bind_sprite_batch_push_sprite},
            // {"sprite_batch_end", __neko_bind_sprite_batch_end},

            // {"filesystem_create", __neko_bind_filesystem_create},
            // {"filesystem_fini", __neko_bind_filesystem_fini},
            // {"filewatch_create", __neko_bind_filewatch_create},
            // {"filewatch_fini", __neko_bind_filewatch_fini},
            // {"filewatch_mount", __neko_bind_filewatch_mount},
            // {"filewatch_start", __neko_bind_filewatch_start_watch},
            // {"filewatch_stop", __neko_bind_filewatch_stop_watching},
            // {"filewatch_update", __neko_bind_filewatch_update},
            // {"filewatch_notify", __neko_bind_filewatch_notify},

            // {"idraw_get", __neko_bind_idraw_get},
            // {"idraw_draw", __neko_bind_idraw_draw},
            // {"idraw_defaults", __neko_bind_idraw_defaults},
            // {"idraw_rectv", __neko_bind_idraw_rectv},
            // {"idraw_rectvd", __neko_bind_idraw_rectvd},
            // {"idraw_text", __neko_bind_idraw_text},
            // {"idraw_camera", __neko_bind_idraw_camera},
            // {"idraw_camera2d", __neko_bind_idraw_camera2d},
            // {"idraw_camera2d_ex", __neko_bind_idraw_camera2d_ex},
            // {"idraw_camera3d", __neko_bind_idraw_camera3d},
            // {"idraw_rotatev", __neko_bind_idraw_rotatev},
            // {"idraw_box", __neko_bind_idraw_box},
            // {"idraw_translatef", __neko_bind_idraw_translatef},
            // {"idraw_depth_enabled", __neko_bind_idraw_depth_enabled},
            // {"idraw_face_cull_enabled", __neko_bind_idraw_face_cull_enabled},
            // {"idraw_texture", __neko_bind_idraw_texture},

            // {"render_framebuffer_create", __neko_bind_render_framebuffer_create},
            // {"render_framebuffer_fini", __neko_bind_render_framebuffer_fini},
            // {"render_texture_create", __neko_bind_render_texture_create},
            // {"render_texture_fini", __neko_bind_render_texture_fini},
            // {"render_renderpass_create", __neko_bind_render_renderpass_create},
            // {"render_renderpass_fini", __neko_bind_render_renderpass_fini},
            // {"render_renderpass_begin", __neko_bind_render_renderpass_begin},
            // {"render_renderpass_end", __neko_bind_render_renderpass_end},
            // {"render_set_viewport", __neko_bind_render_set_viewport},
            // {"render_clear", __neko_bind_render_clear},
            // {"render_shader_create", __neko_bind_render_shader_create},
            // {"render_uniform_create", __neko_bind_render_uniform_create},
            // {"render_pipeline_create", __neko_bind_render_pipeline_create},
            // {"render_storage_buffer_create", __neko_bind_render_storage_buffer_create},
            // {"render_vertex_buffer_create", __neko_bind_render_vertex_buffer_create},
            // {"render_index_buffer_create", __neko_bind_render_index_buffer_create},
            // {"render_vertex_attribute_create", __neko_bind_render_vertex_attribute_create},
            // {"render_pipeline_bind", __neko_bind_render_pipeline_bind},
            // {"render_apply_bindings", __neko_bind_render_apply_bindings},
            // {"render_dispatch_compute", __neko_bind_render_dispatch_compute},
            // {"render_draw", __neko_bind_render_draw},
            // {"render_display_size", __neko_bind_render_display_size},
            // {"gen_tex", test_tex},

            // {"get_channel", __neko_bind_get_channel},
            // {"select", __neko_bind_select},
            // {"thread_id", __neko_bind_thread_id},
            // {"thread_sleep", __neko_bind_thread_sleep},
            // {"make_thread", __neko_bind_make_thread},
            // {"make_channel", __neko_bind_make_channel},

            // {"json_read", __neko_bind_json_read},
            // {"json_write", __neko_bind_json_write},

            {NULL, NULL}};

    luaL_newlib(L, reg);

#if 0
    luaL_Reg inspector_reg[] = {{"inspect_shaders", __neko_bind_inspect_shaders},

                                NEKO_LUA_INSPECT_REG(shaders),
                                NEKO_LUA_INSPECT_REG(textures),
                                NEKO_LUA_INSPECT_REG(vertex_buffers),
                                // NEKO_LUA_INSPECT_REG(uniform_buffers),
                                // NEKO_LUA_INSPECT_REG(storage_buffers),
                                NEKO_LUA_INSPECT_REG(index_buffers),
                                NEKO_LUA_INSPECT_REG(frame_buffers),
                                // NEKO_LUA_INSPECT_REG(uniforms),
                                // NEKO_LUA_INSPECT_REG(pipelines),
                                // NEKO_LUA_INSPECT_REG(renderpasses),
                                {NULL, NULL}};
    luaL_setfuncs(L, inspector_reg, 0);
#endif

    typeclosure(L);
    lua_setfield(L, -2, "ltype");

    createStructTables(L);

    return 1;
}

namespace neko::lua::__core {
LUABIND_MODULE() { return open_embed_core(L); }
}  // namespace neko::lua::__core

DEFINE_LUAOPEN(core)

#endif

DEFINE_LUAOPEN_EXTERN(inspector)
DEFINE_LUAOPEN_EXTERN(luadb)
// DEFINE_LUAOPEN_EXTERN(prefab)
// DEFINE_LUAOPEN_EXTERN(struct)
// DEFINE_LUAOPEN_EXTERN(struct_test)
DEFINE_LUAOPEN_EXTERN(unittest)

namespace neko::lua::__filewatch {
static filewatch::watch &to(lua_State *L, int idx) { return lua::checkudata<filewatch::watch>(L, idx); }

static lua_State *get_thread(lua_State *L) {
    lua_getiuservalue(L, 1, 1);
    lua_State *thread = lua_tothread(L, -1);
    lua_pop(L, 1);
    return thread;
}

static int add(lua_State *L) {
    auto &self = to(L, 1);
    auto pathstr = lua::checkstrview(L, 2);
#if defined(_WIN32)
    std::filesystem::path path{wtf8::u2w(pathstr)};
#else
    std::filesystem::path path{std::string{pathstr.data(), pathstr.size()}};
#endif
    std::error_code ec;
    std::filesystem::path abspath = std::filesystem::absolute(path, ec);
    if (ec) {
        lua_pushstring(L, std::format("error fs::absolute {0}", ec.value()).c_str());
        lua_error(L);
        return 0;
    }
    self.add(abspath.lexically_normal().generic_string<filewatch::watch::string_type::value_type>());
    return 0;
}

static int set_recursive(lua_State *L) {
    auto &self = to(L, 1);
    bool enable = lua_toboolean(L, 2);
    self.set_recursive(enable);
    lua_pushboolean(L, 1);
    return 1;
}

static int set_follow_symlinks(lua_State *L) {
    auto &self = to(L, 1);
    bool enable = lua_toboolean(L, 2);
    bool ok = self.set_follow_symlinks(enable);
    lua_pushboolean(L, ok);
    return 1;
}

static int set_filter(lua_State *L) {
    auto &self = to(L, 1);
    if (lua_isnoneornil(L, 2)) {
        bool ok = self.set_filter();
        lua_pushboolean(L, ok);
        return 1;
    }
    lua_State *thread = get_thread(L);
    lua_settop(L, 2);
    lua_xmove(L, thread, 1);
    if (lua_gettop(thread) > 1) {
        lua_replace(thread, 1);
    }
    bool ok = self.set_filter([=](const char *path) {
        lua_pushvalue(thread, 1);
        lua_pushstring(thread, path);
        if (LUA_OK != lua_pcall(thread, 1, 1, 0)) {
            lua_pop(thread, 1);
            return true;
        }
        bool r = lua_toboolean(thread, -1);
        lua_pop(thread, 1);
        return r;
    });
    lua_pushboolean(L, ok);
    return 1;
}

static int select(lua_State *L) {
    auto &self = to(L, 1);
    auto notify = self.select();
    if (!notify) {
        return 0;
    }
    switch (notify->flags) {
        case filewatch::notify::flag::modify:
            lua_pushstring(L, "modify");
            break;
        case filewatch::notify::flag::rename:
            lua_pushstring(L, "rename");
            break;
        default:
            // std::unreachable();
            neko_assert(0, "unreachable");
    }
    lua_pushlstring(L, notify->path.data(), notify->path.size());
    return 2;
}

static int mt_close(lua_State *L) {
    auto &self = to(L, 1);
    self.stop();
    return 0;
}

static void metatable(lua_State *L) {
    static luaL_Reg lib[] = {{"add", add}, {"set_recursive", set_recursive}, {"set_follow_symlinks", set_follow_symlinks}, {"set_filter", set_filter}, {"select", select}, {NULL, NULL}};
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__close", mt_close}, {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

static int create(lua_State *L) {
    lua::newudata<filewatch::watch>(L);
    lua_newthread(L);
    lua_setiuservalue(L, -2, 1);
    return 1;
}

LUABIND_MODULE() {
    static luaL_Reg lib[] = {{"create", create}, {NULL, NULL}};
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    return 1;
}
}  // namespace neko::lua::__filewatch

DEFINE_LUAOPEN(filewatch)

namespace neko::lua {
template <>
struct udata<filewatch::watch> {
    static inline int nupvalue = 1;
    static inline auto metatable = neko::lua::__filewatch::metatable;
};
}  // namespace neko::lua

static int open_neko(lua_State *L) {
    luaL_Reg reg[] = {
            // internal
            {"__registry_load", neko_registry_load},
            {"__registry_lua_script", neko_require_lua_script},

            // core
            {"version", neko_version},
            {"set_console_window", neko_set_console_window},
            {"quit", neko_quit},
            {"fatal_error", neko_fatal_error},
            {"platform", neko_platform},
            {"dt", neko_dt},
            {"fullscreen", neko_fullscreen},
            {"toggle_fullscreen", neko_toggle_fullscreen},
            {"window_width", neko_window_width},
            {"window_height", neko_window_height},
            {"time", neko_time},
            {"difftime", neko_difftime},
            {"elapsed", neko_elapsed},
            {"json_read", neko_json_read},
            {"json_write", neko_json_write},

            // input
            {"key_down", neko_key_down},
            {"key_release", neko_key_release},
            {"key_press", neko_key_press},
            {"mouse_down", neko_mouse_down},
            {"mouse_release", neko_mouse_release},
            {"mouse_click", neko_mouse_click},
            {"mouse_pos", neko_mouse_pos},
            {"mouse_delta", neko_mouse_delta},
            {"show_mouse", neko_show_mouse},
            {"scroll_wheel", neko_scroll_wheel},

            // draw
            {"scissor_rect", neko_scissor_rect},
            {"push_matrix", neko_push_matrix},
            {"pop_matrix", neko_pop_matrix},
            {"translate", neko_translate},
            {"rotate", neko_rotate},
            {"scale", neko_scale},
            {"clear_color", neko_clear_color},
            {"push_color", neko_push_color},
            {"pop_color", neko_pop_color},
            // {"default_font", neko_default_font},
            // {"default_sampler", neko_default_sampler},
            // {"draw_filled_rect", neko_draw_filled_rect},
            // {"draw_line_rect", neko_draw_line_rect},
            // {"draw_line_circle", neko_draw_line_circle},
            // {"draw_line", neko_draw_line},

            // audio
            {"set_master_volume", neko_set_master_volume},

            // concurrency
            {"get_channel", neko_get_channel},
            {"select", neko_select},
            {"thread_id", neko_thread_id},
            {"thread_sleep", neko_thread_sleep},

            // filesystem
            {"program_path", neko_program_path},
            {"program_dir", neko_program_dir},
            {"is_fused", neko_is_fused},
            {"file_exists", neko_file_exists},
            {"file_read", neko_file_read},
            {"file_write", neko_file_write},

            // construct types
            // {"make_sampler", neko_make_sampler},
            {"make_thread", neko_make_thread},
            {"make_channel", neko_make_channel},
            {"image_load", neko_image_load},
            // {"font_load", neko_font_load},
            // {"sound_load", neko_sound_load},
            // {"sprite_load", neko_sprite_load},
            // {"atlas_load", neko_atlas_load},
            // {"tilemap_load", neko_tilemap_load},
            {"pak_load", neko_pak_load},
            {"b2_world", neko_b2_world},
            // {"ecs_create", neko_ecs_create_world},

            {nullptr, nullptr},
    };

    luaL_newlib(L, reg);
    return 1;
}

void open_neko_api(lua_State *L) {
    // clang-format off
    lua_CFunction mt_funcs[] = {
        open_mt_sampler,
        open_mt_thread,
        open_mt_channel,
        // open_mt_image,
        // open_mt_font,
        // open_mt_sound,
        // open_mt_sprite,
        // open_mt_atlas_image,
        // open_mt_atlas,
        // open_mt_tilemap,
        open_mt_pak,
        open_mt_b2_fixture,
        open_mt_b2_body,
        open_mt_b2_world,
        // open_mt_lui_container,
        // open_mt_lui_style,
        // open_mt_lui_ref
    };
    // clang-format on

    for (u32 i = 0; i < array_size(mt_funcs); i++) {
        mt_funcs[i](L);
    }

    luaL_requiref(L, "neko", open_neko, 1);

    // open_microui(L);
    // lua_setfield(L, -2, "microui");

    lua_pop(L, 1);

    newusertype<Bytearray, Bytearray::createMetatable>(L, "bytearray");

    neko::lua::preload_module(L);
    neko::lua::package_preload(L);
    neko::lua::package_preload_embed(L);

    neko_register_common(L);
    neko_w_init();

    lua_register(L, "__neko_loader", neko::vfs_lua_loader);
    const_str str = "table.insert(package.searchers, 2, __neko_loader) \n";
    luaL_dostring(L, str);
}

namespace neko::lua {
void package_preload(lua_State *L) {
    luaL_Reg preloads[] = {
            {"__neko.spritepack", open_tools_spritepack},
            {"__neko.filesys", open_filesys},
    };
    for (auto m : preloads) {
        luax_package_preload(L, m.name, m.func);
    }
}
}  // namespace neko::lua