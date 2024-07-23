#include "neko_api.hpp"

#include <box2d/box2d.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include <util/sokol_gl.h>

#include "neko_app.h"
#include "neko_asset.h"
#include "neko_base.h"
#include "neko_draw.h"
#include "neko_ecs.h"
#include "neko_lua.h"
#include "neko_lua_wrap.h"
#include "neko_os.h"
#include "neko_physics.h"
#include "neko_prelude.h"
#include "neko_sound.h"
#include "neko_tolua.h"
#include "neko_ui.h"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

// mt_sampler

static int mt_sampler_gc(lua_State *L) {
    u32 *udata = (u32 *)luaL_checkudata(L, 1, "mt_sampler");
    u32 id = *udata;

    if (id != SG_INVALID_ID) {
        sg_destroy_sampler({id});
    }

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
    map_ldtk tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;
    draw_tilemap(&tm);
    return 0;
}

static int mt_tilemap_entities(lua_State *L) {
    map_ldtk tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;

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
    map_ldtk tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;
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

// mt_lui_container

static int mt_lui_container_rect(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lua_lui_rect_push(L, container->rect);
    return 1;
}

static int mt_lui_container_set_rect(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    container->rect = lua_lui_check_rect(L, 2);
    return 0;
}

static int mt_lui_container_body(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lua_lui_rect_push(L, container->body);
    return 1;
}

static int mt_lui_container_content_size(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lua_pushinteger(L, container->content_size.x);
    lua_pushinteger(L, container->content_size.y);
    return 2;
}

static int mt_lui_container_scroll(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lua_pushinteger(L, container->scroll.x);
    lua_pushinteger(L, container->scroll.y);
    return 2;
}

static int mt_lui_container_set_scroll(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    container->scroll.x = luaL_checknumber(L, 2);
    container->scroll.y = luaL_checknumber(L, 3);
    return 0;
}

static int mt_lui_container_zindex(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lua_pushinteger(L, container->zindex);
    return 1;
}

static int mt_lui_container_open(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lua_pushboolean(L, container->open);
    return 1;
}

static int open_mt_lui_container(lua_State *L) {
    luaL_Reg reg[] = {
            {"rect", mt_lui_container_rect},
            {"set_rect", mt_lui_container_set_rect},
            {"body", mt_lui_container_body},
            {"content_size", mt_lui_container_content_size},
            {"scroll", mt_lui_container_scroll},
            {"set_scroll", mt_lui_container_set_scroll},
            {"zindex", mt_lui_container_zindex},
            {"open", mt_lui_container_open},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_lui_container", reg);
    return 0;
}

static int mt_lui_style_size(lua_State *L) {
    lui_Style *style = *(lui_Style **)luaL_checkudata(L, 1, "mt_lui_style");
    lua_pushinteger(L, style->size.x);
    lua_pushinteger(L, style->size.y);
    return 2;
}

static int mt_lui_style_set_size(lua_State *L) {
    lui_Style *style = *(lui_Style **)luaL_checkudata(L, 1, "mt_lui_style");
    style->size.x = luaL_checknumber(L, 2);
    style->size.y = luaL_checknumber(L, 3);
    return 0;
}

#define MT_LUI_STYLE_GETSET(name)                                                \
    static int mt_lui_style_##name(lua_State *L) {                               \
        lui_Style *style = *(lui_Style **)luaL_checkudata(L, 1, "mt_lui_style"); \
        lua_pushinteger(L, style->name);                                         \
        return 1;                                                                \
    }                                                                            \
    static int mt_lui_style_set_##name(lua_State *L) {                           \
        lui_Style *style = *(lui_Style **)luaL_checkudata(L, 1, "mt_lui_style"); \
        style->name = luaL_checknumber(L, 2);                                    \
        return 0;                                                                \
    }

MT_LUI_STYLE_GETSET(padding);
MT_LUI_STYLE_GETSET(spacing);
MT_LUI_STYLE_GETSET(indent);
MT_LUI_STYLE_GETSET(title_height);
MT_LUI_STYLE_GETSET(scrollbar_size);
MT_LUI_STYLE_GETSET(thumb_size);

static int mt_lui_style_color(lua_State *L) {
    lui_Style *style = *(lui_Style **)luaL_checkudata(L, 1, "mt_lui_style");
    lua_Integer colorid = luaL_checkinteger(L, 2);
    if (colorid < 0 || colorid >= LUI_COLOR_MAX) {
        return luaL_error(L, "color id out of range");
    }

    lua_createtable(L, 0, 4);
    luax_set_number_field(L, "r", style->colors[colorid].r);
    luax_set_number_field(L, "g", style->colors[colorid].g);
    luax_set_number_field(L, "b", style->colors[colorid].b);
    luax_set_number_field(L, "a", style->colors[colorid].a);
    return 1;
}

static int mt_lui_style_set_color(lua_State *L) {
    lui_Style *style = *(lui_Style **)luaL_checkudata(L, 1, "mt_lui_style");
    lua_Integer colorid = luaL_checkinteger(L, 2);
    lui_Color color = lua_lui_check_color(L, 3);

    if (colorid < 0 || colorid >= LUI_COLOR_MAX) {
        return luaL_error(L, "color id out of range");
    }

    style->colors[colorid] = color;
    return 0;
}

static int open_mt_lui_style(lua_State *L) {
    luaL_Reg reg[] = {
            {"size", mt_lui_style_size},
            {"set_size", mt_lui_style_set_size},
            {"padding", mt_lui_style_padding},
            {"set_padding", mt_lui_style_set_padding},
            {"spacing", mt_lui_style_spacing},
            {"set_spacing", mt_lui_style_set_spacing},
            {"indent", mt_lui_style_indent},
            {"set_indent", mt_lui_style_set_indent},
            {"title_height", mt_lui_style_title_height},
            {"set_title_height", mt_lui_style_set_title_height},
            {"scrollbar_size", mt_lui_style_scrollbar_size},
            {"set_scrollbar_size", mt_lui_style_set_scrollbar_size},
            {"thumb_size", mt_lui_style_thumb_size},
            {"set_thumb_size", mt_lui_style_set_thumb_size},
            {"color", mt_lui_style_color},
            {"set_color", mt_lui_style_set_color},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_lui_style", reg);
    return 0;
}

// mt_lui_ref

static int mt_lui_ref_gc(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_lui_ref");
    mem_free(ref);
    return 0;
}

static int mt_lui_ref_get(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_lui_ref");

    switch (ref->kind) {
        case MUIRefKind_Boolean:
            lua_pushboolean(L, ref->boolean);
            return 1;
        case MUIRefKind_Real:
            lua_pushnumber(L, ref->real);
            return 1;
        case MUIRefKind_String:
            lua_pushstring(L, ref->string);
            return 1;
        case MUIRefKind_Nil:
        default:
            return 0;
    }
}

static int mt_lui_ref_set(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_lui_ref");
    lua_lui_set_ref(L, ref, 2);
    return 0;
}

static int open_mt_lui_ref(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_lui_ref_gc},
            {"get", mt_lui_ref_get},
            {"set", mt_lui_ref_set},
            {nullptr, nullptr},
    };
    luax_new_class(L, "mt_lui_ref", reg);
    return 0;
}

// microui api

static int mui_set_focus(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lui_set_focus(microui_ctx(), (lui_Id)id);
    return 0;
}

static int mui_get_id(lua_State *L) {
    String name = luax_check_string(L, 1);
    lui_Id id = lui_get_id(microui_ctx(), name.data, name.len);
    lua_pushinteger(L, (lua_Integer)id);
    return 1;
}

static int mui_push_id(lua_State *L) {
    String name = luax_check_string(L, 1);
    lui_push_id(microui_ctx(), name.data, name.len);
    return 0;
}

static int mui_pop_id(lua_State *L) {
    lui_pop_id(microui_ctx());
    return 0;
}

static int mui_push_clip_rect(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);
    lui_push_clip_rect(microui_ctx(), rect);
    return 0;
}

static int mui_pop_clip_rect(lua_State *L) {
    lui_pop_clip_rect(microui_ctx());
    return 0;
}

static int mui_get_clip_rect(lua_State *L) {
    lui_Rect rect = lui_get_clip_rect(microui_ctx());
    lua_lui_rect_push(L, rect);
    return 1;
}

static int mui_check_clip(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);

    i32 clip = lui_check_clip(microui_ctx(), rect);
    lua_pushinteger(L, clip);
    return 1;
}

static int mui_get_current_container(lua_State *L) {
    lui_Container *container = lui_get_current_container(microui_ctx());
    luax_ptr_userdata(L, container, "mt_lui_container");
    return 1;
}

static int mui_get_container(lua_State *L) {
    String name = luax_check_string(L, 1);
    lui_Container *container = lui_get_container(microui_ctx(), name.data);
    luax_ptr_userdata(L, container, "mt_lui_container");
    return 1;
}

static int mui_bring_to_front(lua_State *L) {
    lui_Container *container = *(lui_Container **)luaL_checkudata(L, 1, "mt_lui_container");
    lui_bring_to_front(microui_ctx(), container);
    return 0;
}

static int mui_set_clip(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);
    lui_set_clip(microui_ctx(), rect);
    return 0;
}

static int mui_draw_rect(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);
    lui_Color color = lua_lui_check_color(L, 2);
    lui_draw_rect(microui_ctx(), rect, color);
    return 0;
}

static int mui_draw_box(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);
    lui_Color color = lua_lui_check_color(L, 2);
    lui_draw_box(microui_ctx(), rect, color);
    return 0;
}

static int mui_draw_text(lua_State *L) {
    String str = luax_check_string(L, 1);
    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
    lui_Color color = lua_lui_check_color(L, 4);

    lui_Vec2 pos = {(int)x, (int)y};
    lui_draw_text(microui_ctx(), nullptr, str.data, str.len, pos, color);
    return 0;
}

static int mui_draw_icon(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lui_Rect rect = lua_lui_check_rect(L, 2);
    lui_Color color = lua_lui_check_color(L, 3);

    lui_draw_icon(microui_ctx(), id, rect, color);
    return 0;
}

static int mui_layout_row(lua_State *L) {
    lua_Number height = luaL_checknumber(L, 2);

    i32 widths[LUI_MAX_WIDTHS] = {};

    lua_Integer n = luax_len(L, 1);
    if (n > LUI_MAX_WIDTHS) {
        n = LUI_MAX_WIDTHS;
    }

    for (i32 i = 0; i < n; i++) {
        luax_geti(L, 1, i + 1);
        widths[i] = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    lui_layout_row(microui_ctx(), n, widths, height);
    return 0;
}

static int mui_layout_width(lua_State *L) {
    lua_Number width = luaL_checknumber(L, 1);
    lui_layout_width(microui_ctx(), width);
    return 0;
}

static int mui_layout_height(lua_State *L) {
    lua_Number height = luaL_checknumber(L, 1);
    lui_layout_height(microui_ctx(), height);
    return 0;
}

static int mui_layout_begin_column(lua_State *L) {
    lui_layout_begin_column(microui_ctx());
    return 0;
}

static int mui_layout_end_column(lua_State *L) {
    lui_layout_end_column(microui_ctx());
    return 0;
}

static int mui_layout_set_next(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);
    bool relative = lua_toboolean(L, 2);
    lui_layout_set_next(microui_ctx(), rect, relative);
    return 0;
}

static int mui_layout_next(lua_State *L) {
    lui_Rect rect = lui_layout_next(microui_ctx());
    lua_lui_rect_push(L, rect);
    return 1;
}

static int mui_draw_control_frame(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lui_Rect rect = lua_lui_check_rect(L, 2);
    lua_Integer colorid = luaL_checkinteger(L, 3);
    lua_Integer opt = luaL_checkinteger(L, 4);
    lui_draw_control_frame(microui_ctx(), id, rect, colorid, opt);
    return 0;
}

static int mui_draw_control_text(lua_State *L) {
    String str = luax_check_string(L, 1);
    lui_Rect rect = lua_lui_check_rect(L, 2);
    lua_Integer colorid = luaL_checkinteger(L, 3);
    lua_Integer opt = luaL_checkinteger(L, 4);
    lui_draw_control_text(microui_ctx(), str.data, rect, colorid, opt);
    return 0;
}

static int mui_mouse_over(lua_State *L) {
    lui_Rect rect = lua_lui_check_rect(L, 1);
    int res = lui_mouse_over(microui_ctx(), rect);
    lua_pushboolean(L, res);
    return 1;
}

static int mui_update_control(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lui_Rect rect = lua_lui_check_rect(L, 2);
    lua_Integer opt = luaL_checkinteger(L, 3);
    lui_update_control(microui_ctx(), id, rect, opt);
    return 0;
}

static int mui_text(lua_State *L) {
    String text = luax_check_string(L, 1);
    lui_text(microui_ctx(), text.data);
    return 0;
}

static int mui_label(lua_State *L) {
    String text = luax_check_string(L, 1);
    lui_label(microui_ctx(), text.data);
    return 0;
}

static int mui_button(lua_State *L) {
    String text = luax_check_string(L, 1);
    lua_Integer icon = luaL_optinteger(L, 2, 0);
    lua_Integer opt = luaL_optinteger(L, 3, LUI_OPT_ALIGNCENTER);
    i32 res = lui_button_ex(microui_ctx(), text.data, icon, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int mui_checkbox(lua_State *L) {
    String text = luax_check_string(L, 1);
    MUIRef *ref = lua_lui_check_ref(L, 2, MUIRefKind_Boolean);

    i32 res = lui_checkbox(microui_ctx(), text.data, &ref->boolean);
    lua_pushinteger(L, res);
    return 1;
}

static int mui_textbox_raw(lua_State *L) {
    MUIRef *ref = lua_lui_check_ref(L, 1, MUIRefKind_String);
    lua_Integer id = luaL_checkinteger(L, 2);
    lui_Rect rect = lua_lui_check_rect(L, 3);
    i32 opt = luaL_optinteger(L, 4, 0);

    i32 res = lui_textbox_raw(microui_ctx(), ref->string, array_size(ref->string), id, rect, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int mui_textbox(lua_State *L) {
    MUIRef *ref = lua_lui_check_ref(L, 1, MUIRefKind_String);
    i32 opt = luaL_optinteger(L, 2, 0);

    i32 res = lui_textbox_ex(microui_ctx(), ref->string, array_size(ref->string), opt);
    lua_pushinteger(L, res);
    return 1;
}

static int mui_slider(lua_State *L) {
    MUIRef *ref = lua_lui_check_ref(L, 1, MUIRefKind_Real);
    lui_Real low = (lui_Real)luaL_checknumber(L, 2);
    lui_Real high = (lui_Real)luaL_checknumber(L, 3);
    lui_Real step = (lui_Real)luaL_optnumber(L, 4, 0);
    String fmt = luax_opt_string(L, 5, LUI_SLIDER_FMT);
    i32 opt = luaL_optinteger(L, 6, LUI_OPT_ALIGNCENTER);

    i32 res = lui_slider_ex(microui_ctx(), &ref->real, low, high, step, fmt.data, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int mui_number(lua_State *L) {
    MUIRef *ref = lua_lui_check_ref(L, 1, MUIRefKind_Real);
    lui_Real step = (lui_Real)luaL_checknumber(L, 2);
    String fmt = luax_opt_string(L, 3, LUI_SLIDER_FMT);
    i32 opt = luaL_optinteger(L, 4, LUI_OPT_ALIGNCENTER);

    i32 res = lui_number_ex(microui_ctx(), &ref->real, step, fmt.data, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int mui_header(lua_State *L) {
    String text = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);
    i32 res = lui_header_ex(microui_ctx(), text.data, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int mui_begin_treenode(lua_State *L) {
    String label = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);

    i32 res = lui_begin_treenode_ex(microui_ctx(), label.data, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int mui_end_treenode(lua_State *L) {
    lui_end_treenode(microui_ctx());
    return 0;
}

static int mui_begin_window(lua_State *L) {
    String title = luax_check_string(L, 1);
    lui_Rect rect = lua_lui_check_rect(L, 2);
    lua_Integer opt = luaL_optinteger(L, 3, 0);

    i32 res = lui_begin_window_ex(microui_ctx(), title.data, rect, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int mui_end_window(lua_State *L) {
    lui_end_window(microui_ctx());
    return 0;
}

static int mui_open_popup(lua_State *L) {
    String name = luax_check_string(L, 1);
    lui_open_popup(microui_ctx(), name.data);
    return 0;
}

static int mui_begin_popup(lua_State *L) {
    String name = luax_check_string(L, 1);
    i32 res = lui_begin_popup(microui_ctx(), name.data);
    lua_pushboolean(L, res);
    return 1;
}

static int mui_end_popup(lua_State *L) {
    lui_end_popup(microui_ctx());
    return 0;
}

static int mui_begin_panel(lua_State *L) {
    String name = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);
    lui_begin_panel_ex(microui_ctx(), name.data, opt);
    return 0;
}

static int mui_end_panel(lua_State *L) {
    lui_end_panel(microui_ctx());
    return 0;
}

static int mui_get_hover(lua_State *L) {
    lua_pushinteger(L, microui_ctx()->hover);
    return 1;
}

static int mui_get_focus(lua_State *L) {
    lua_pushinteger(L, microui_ctx()->focus);
    return 1;
}

static int mui_get_last_id(lua_State *L) {
    lua_pushinteger(L, microui_ctx()->last_id);
    return 1;
}

static int mui_get_style(lua_State *L) {
    luax_ptr_userdata(L, microui_ctx()->style, "mt_lui_style");
    return 1;
}

static int mui_rect(lua_State *L) {
    lua_createtable(L, 0, 4);
    luax_set_int_field(L, "x", luaL_checknumber(L, 1));
    luax_set_int_field(L, "y", luaL_checknumber(L, 2));
    luax_set_int_field(L, "w", luaL_checknumber(L, 3));
    luax_set_int_field(L, "h", luaL_checknumber(L, 4));
    return 1;
}

static int mui_color(lua_State *L) {
    lua_createtable(L, 0, 4);
    luax_set_int_field(L, "r", luaL_checknumber(L, 1));
    luax_set_int_field(L, "g", luaL_checknumber(L, 2));
    luax_set_int_field(L, "b", luaL_checknumber(L, 3));
    luax_set_int_field(L, "a", luaL_checknumber(L, 4));
    return 1;
}

static int mui_ref(lua_State *L) {
    MUIRef *ref = (MUIRef *)mem_alloc(sizeof(MUIRef));
    lua_lui_set_ref(L, ref, 1);
    luax_ptr_userdata(L, ref, "mt_lui_ref");
    return 1;
}

static int open_microui(lua_State *L) {
    luaL_Reg reg[] = {
            {"set_focus", mui_set_focus},
            {"get_id", mui_get_id},
            {"push_id", mui_push_id},
            {"pop_id", mui_pop_id},
            {"push_clip_rect", mui_push_clip_rect},
            {"pop_clip_rect", mui_pop_clip_rect},
            {"get_clip_rect", mui_get_clip_rect},
            {"check_clip", mui_check_clip},
            {"get_current_container", mui_get_current_container},
            {"get_container", mui_get_container},
            {"bring_to_front", mui_bring_to_front},

            {"set_clip", mui_set_clip},
            {"draw_rect", mui_draw_rect},
            {"draw_box", mui_draw_box},
            {"draw_text", mui_draw_text},
            {"draw_icon", mui_draw_icon},

            {"layout_row", mui_layout_row},
            {"layout_width", mui_layout_width},
            {"layout_height", mui_layout_height},
            {"layout_begin_column", mui_layout_begin_column},
            {"layout_end_column", mui_layout_end_column},
            {"layout_set_next", mui_layout_set_next},
            {"layout_next", mui_layout_next},

            {"draw_control_frame", mui_draw_control_frame},
            {"draw_control_text", mui_draw_control_text},
            {"mouse_over", mui_mouse_over},
            {"update_control", mui_update_control},

            {"text", mui_text},
            {"label", mui_label},
            {"button", mui_button},
            {"checkbox", mui_checkbox},
            {"textbox_raw", mui_textbox_raw},
            {"textbox", mui_textbox},
            {"slider", mui_slider},
            {"number", mui_number},
            {"header", mui_header},
            {"begin_treenode", mui_begin_treenode},
            {"end_treenode", mui_end_treenode},
            {"begin_window", mui_begin_window},
            {"end_window", mui_end_window},
            {"open_popup", mui_open_popup},
            {"begin_popup", mui_begin_popup},
            {"end_popup", mui_end_popup},
            {"begin_panel", mui_begin_panel},
            {"end_panel", mui_end_panel},

            // access
            {"get_hover", mui_get_hover},
            {"get_focus", mui_get_focus},
            {"get_last_id", mui_get_last_id},
            {"get_style", mui_get_style},

            // utility
            {"rect", mui_rect},
            {"color", mui_color},
            {"ref", mui_ref},
            {nullptr, nullptr},
    };

    luaL_newlib(L, reg);

    // luax_set_string_field(L, "VERSION", LUI_VERSION);

    luax_set_int_field(L, "COMMANDLIST_SIZE", LUI_COMMANDLIST_SIZE);
    luax_set_int_field(L, "ROOTLIST_SIZE", LUI_ROOTLIST_SIZE);
    luax_set_int_field(L, "CONTAINERSTACK_SIZE", LUI_CONTAINERSTACK_SIZE);
    luax_set_int_field(L, "CLIPSTACK_SIZE", LUI_CLIPSTACK_SIZE);
    luax_set_int_field(L, "IDSTACK_SIZE", LUI_IDSTACK_SIZE);
    luax_set_int_field(L, "LAYOUTSTACK_SIZE", LUI_LAYOUTSTACK_SIZE);
    luax_set_int_field(L, "CONTAINERPOOL_SIZE", LUI_CONTAINERPOOL_SIZE);
    luax_set_int_field(L, "TREENODEPOOL_SIZE", LUI_TREENODEPOOL_SIZE);
    luax_set_int_field(L, "MAX_WIDTHS", LUI_MAX_WIDTHS);
    luax_set_string_field(L, "REAL_FMT", LUI_REAL_FMT);
    luax_set_string_field(L, "SLIDER_FMT", LUI_SLIDER_FMT);
    luax_set_int_field(L, "MAX_FMT", LUI_MAX_FMT);

    luax_set_int_field(L, "CLIP_PART", LUI_CLIP_PART);
    luax_set_int_field(L, "CLIP_ALL", LUI_CLIP_ALL);

    luax_set_int_field(L, "COMMAND_JUMP", LUI_COMMAND_JUMP);
    luax_set_int_field(L, "COMMAND_CLIP", LUI_COMMAND_CLIP);
    luax_set_int_field(L, "COMMAND_RECT", LUI_COMMAND_RECT);
    luax_set_int_field(L, "COMMAND_TEXT", LUI_COMMAND_TEXT);
    luax_set_int_field(L, "COMMAND_ICON", LUI_COMMAND_ICON);

    luax_set_int_field(L, "COLOR_TEXT", LUI_COLOR_TEXT);
    luax_set_int_field(L, "COLOR_BORDER", LUI_COLOR_BORDER);
    luax_set_int_field(L, "COLOR_WINDOWBG", LUI_COLOR_WINDOWBG);
    luax_set_int_field(L, "COLOR_TITLEBG", LUI_COLOR_TITLEBG);
    luax_set_int_field(L, "COLOR_TITLETEXT", LUI_COLOR_TITLETEXT);
    luax_set_int_field(L, "COLOR_PANELBG", LUI_COLOR_PANELBG);
    luax_set_int_field(L, "COLOR_BUTTON", LUI_COLOR_BUTTON);
    luax_set_int_field(L, "COLOR_BUTTONHOVER", LUI_COLOR_BUTTONHOVER);
    luax_set_int_field(L, "COLOR_BUTTONFOCUS", LUI_COLOR_BUTTONFOCUS);
    luax_set_int_field(L, "COLOR_BASE", LUI_COLOR_BASE);
    luax_set_int_field(L, "COLOR_BASEHOVER", LUI_COLOR_BASEHOVER);
    luax_set_int_field(L, "COLOR_BASEFOCUS", LUI_COLOR_BASEFOCUS);
    luax_set_int_field(L, "COLOR_SCROLLBASE", LUI_COLOR_SCROLLBASE);
    luax_set_int_field(L, "COLOR_SCROLLTHUMB", LUI_COLOR_SCROLLTHUMB);

    luax_set_int_field(L, "ICON_CLOSE", LUI_ICON_CLOSE);
    luax_set_int_field(L, "ICON_CHECK", LUI_ICON_CHECK);
    luax_set_int_field(L, "ICON_COLLAPSED", LUI_ICON_COLLAPSED);
    luax_set_int_field(L, "ICON_EXPANDED", LUI_ICON_EXPANDED);

    luax_set_int_field(L, "RES_ACTIVE", LUI_RES_ACTIVE);
    luax_set_int_field(L, "RES_SUBMIT", LUI_RES_SUBMIT);
    luax_set_int_field(L, "RES_CHANGE", LUI_RES_CHANGE);

    luax_set_int_field(L, "OPT_ALIGNCENTER", LUI_OPT_ALIGNCENTER);
    luax_set_int_field(L, "OPT_ALIGNRIGHT", LUI_OPT_ALIGNRIGHT);
    luax_set_int_field(L, "OPT_NOINTERACT", LUI_OPT_NOINTERACT);
    luax_set_int_field(L, "OPT_NOFRAME", LUI_OPT_NOFRAME);
    luax_set_int_field(L, "OPT_NORESIZE", LUI_OPT_NORESIZE);
    luax_set_int_field(L, "OPT_NOSCROLL", LUI_OPT_NOSCROLL);
    luax_set_int_field(L, "OPT_NOCLOSE", LUI_OPT_NOCLOSE);
    luax_set_int_field(L, "OPT_NOTITLE", LUI_OPT_NOTITLE);
    luax_set_int_field(L, "OPT_HOLDFOCUS", LUI_OPT_HOLDFOCUS);
    luax_set_int_field(L, "OPT_AUTOSIZE", LUI_OPT_AUTOSIZE);
    luax_set_int_field(L, "OPT_POPUP", LUI_OPT_POPUP);
    luax_set_int_field(L, "OPT_CLOSED", LUI_OPT_CLOSED);
    luax_set_int_field(L, "OPT_EXPANDED", LUI_OPT_EXPANDED);

    return 1;
}

// mt_pak

struct pak_assets_t {
    const_str name;
    size_t size;
    String data;
};

static Asset check_pak_udata(lua_State *L, i32 arg) {
    u64 *udata = (u64 *)luaL_checkudata(L, arg, "mt_pak");

    Asset asset = {};
    bool ok = asset_read(*udata, &asset);
    if (!ok) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}

static int mt_pak_gc(lua_State *L) {
    Asset asset = check_pak_udata(L, 1);
    neko_pak *pak = &asset.pak;

    pak->fini();

    NEKO_DEBUG_LOG("pak(%s) __gc %p", asset.name.cstr(), pak);

    // if (font != g_app->default_font) {
    //     font->trash();
    //     mem_free(font);
    // }
    return 0;
}

static int mt_pak_items(lua_State *L) {
    Asset asset = check_pak_udata(L, 1);
    neko_pak *pak = &asset.pak;

    u64 item_count = pak->get_item_count();

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, pak->get_item_path(i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int mt_pak_assets_load(lua_State *L) {
    Asset asset = check_pak_udata(L, 1);
    neko_pak *pak = &asset.pak;

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

    asset_write(asset);

    return 1;
}

static int mt_pak_assets_unload(lua_State *L) {
    Asset asset = check_pak_udata(L, 1);
    neko_pak *pak = &asset.pak;

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_touserdata(L, 2);

    if (assets_user_handle && assets_user_handle->data.len)
        pak->free_item(assets_user_handle->data);
    else
        NEKO_WARN("unknown assets unload %p", assets_user_handle);

    asset_write(asset);

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
    lua_pushliteral(L, LUA_LOADED_TABLE);
    lua_gettable(L, LUA_REGISTRYINDEX);

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
    sapp_request_quit();
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
    lua_pushboolean(L, sapp_is_fullscreen());
    return 1;
}

static int neko_toggle_fullscreen(lua_State *L) {
    sapp_toggle_fullscreen();
    return 0;
}

static int neko_window_width(lua_State *L) {
    float width = sapp_widthf();
    lua_pushnumber(L, width);
    return 1;
}

static int neko_window_height(lua_State *L) {
    float height = sapp_heightf();
    lua_pushnumber(L, height);
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
    sapp_show_mouse(show);
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
    lua_Number w = luaL_optnumber(L, 3, sapp_widthf());
    lua_Number h = luaL_optnumber(L, 4, sapp_heightf());

    sgl_scissor_rectf(x, y, w, h, true);
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

static int neko_set_master_volume(lua_State *L) {
    lua_Number vol = luaL_checknumber(L, 1);
    ma_engine_set_volume(&g_app->audio_engine, (float)vol);
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
    lua_pushboolean(L, vfs_file_exists(NEKO_PACKS::GAMEDATA, path));
    return 1;
}

static int neko_file_read(lua_State *L) {
    PROFILE_FUNC();

    String path = luax_check_string(L, 1);

    String contents = {};
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, path);
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

static int neko_pak_load(lua_State *L) {
    String name = luax_check_string(L, 1);
    String path = luax_check_string(L, 2);

    AssetLoadData desc = {};
    desc.kind = AssetKind_Pak;

    Asset asset = {};
    bool ok = asset_load(desc, path, &asset);
    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, asset.hash, "mt_pak");
    return 1;
}

static void neko_tolua_setfield(lua_State *L, int table, const_str f, const_str v) {
    lua_pushstring(L, f);
    lua_pushstring(L, v);
    lua_settable(L, table);
}

static void neko_tolua_add_extra(lua_State *L, const_str value) {
    int len;
    lua_getglobal(L, "_extra_parameters");
#if LUA_VERSION_NUM > 501
    len = lua_rawlen(L, -1);
#else
    len = luaL_getn(L, -1);
#endif
    lua_pushstring(L, value);
    lua_rawseti(L, -2, len + 1);
    lua_pop(L, 1);
};

void neko_tolua_boot(const_str f, const_str output) {

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushstring(L, NEKO_TOLUA_VERSION);
    lua_setglobal(L, "NEKO_TOLUA_VERSION");
    lua_pushstring(L, LUA_VERSION);
    lua_setglobal(L, "TOLUA_LUA_VERSION");

    {
        int i, t;
        lua_newtable(L);
        lua_setglobal(L, "_extra_parameters");
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "neko_tolua_flags");
        t = lua_gettop(L);

        neko_tolua_setfield(L, t, "f", f);
        neko_tolua_setfield(L, t, "o", output);

        // disable automatic exporting of destructors for classes
        // that have constructors (for compatibility with tolua5)
        // case 'D':
        //     neko_tolua_setfield(L, t, "D", "");
        //     break;
        // // add extra values to the luastate
        // case 'E':
        //     neko_tolua_add_extra(L, argv[++i]);

        lua_pop(L, 1);
    }

    int neko_tolua_boot_open(lua_State * L);
    neko_tolua_boot_open(L);
}

#if 1
#include "engine/luabind/core.hpp"
#include "engine/luabind/debug.hpp"
#include "engine/luabind/ffi.hpp"
#include "engine/luabind/prefab.hpp"
#include "engine/luabind/struct.hpp"
#endif

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
            NEKO_ASSERT(0, "unreachable");
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
            {"default_font", neko_default_font},
            {"default_sampler", neko_default_sampler},
            {"draw_filled_rect", neko_draw_filled_rect},
            {"draw_line_rect", neko_draw_line_rect},
            {"draw_line_circle", neko_draw_line_circle},
            {"draw_line", neko_draw_line},

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
            {"make_sampler", neko_make_sampler},
            {"make_thread", neko_make_thread},
            {"make_channel", neko_make_channel},
            {"image_load", neko_image_load},
            {"font_load", neko_font_load},
            {"sound_load", neko_sound_load},
            {"sprite_load", neko_sprite_load},
            {"atlas_load", neko_atlas_load},
            {"tilemap_load", neko_tilemap_load},
            {"pak_load", neko_pak_load},
            {"b2_world", neko_b2_world},
            {"ecs_create", neko_ecs_create_world},

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
        open_mt_image,
        open_mt_font,
        open_mt_sound,
        open_mt_sprite,
        open_mt_atlas_image,
        open_mt_atlas,
        open_mt_tilemap,
        open_mt_pak,
        open_mt_b2_fixture,
        open_mt_b2_body,
        open_mt_b2_world,
        open_mt_lui_container,
        open_mt_lui_style,
        open_mt_lui_ref
    };
    // clang-format on

    for (u32 i = 0; i < array_size(mt_funcs); i++) {
        mt_funcs[i](L);
    }

    luaL_requiref(L, "neko", open_neko, 1);

    open_microui(L);
    lua_setfield(L, -2, "microui");

    lua_pop(L, 1);

    register_neko_api_core_open(L);

    neko::lua::preload_module(L);   // 新的模块系统
    neko::lua::package_preload(L);  // 新的模块系统

    neko_register_common(L);
    neko_w_init();
}

#if defined(NEKO_IS_WEB) || 1

void open_luasocket(lua_State *L) {}

#else

#define LUAOPEN_EMBED_DATA(func, name, compressed_data, compressed_size)         \
    static int func(lua_State *L) {                                              \
        i32 top = lua_gettop(L);                                                 \
                                                                                 \
        String contents = stb_decompress_data(compressed_data, compressed_size); \
        neko_defer(mem_free(contents.data));                                     \
                                                                                 \
        if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {   \
            luaL_error(L, "%s", lua_tostring(L, -1));                            \
            return 0;                                                            \
        }                                                                        \
                                                                                 \
        if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {                         \
            luaL_error(L, "%s", lua_tostring(L, -1));                            \
            return 0;                                                            \
        }                                                                        \
                                                                                 \
        return lua_gettop(L) - top;                                              \
    }

LUAOPEN_EMBED_DATA(open_embed_ltn12, "ltn12.lua", ltn12_compressed_data, ltn12_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_mbox, "mbox.lua", mbox_compressed_data, mbox_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_mime, "mime.lua", mime_compressed_data, mime_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket, "socket.lua", socket_compressed_data, socket_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_ftp, "socket.ftp.lua", socket_ftp_compressed_data, socket_ftp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_headers, "socket.headers.lua", socket_headers_compressed_data, socket_headers_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_http, "socket.http.lua", socket_http_compressed_data, socket_http_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_smtp, "socket.smtp.lua", socket_smtp_compressed_data, socket_smtp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_tp, "socket.tp.lua", socket_tp_compressed_data, socket_tp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_url, "socket.url.lua", socket_url_compressed_data, socket_url_compressed_size);

static void package_preload(lua_State *L, const char *name, lua_CFunction function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
}

void open_luasocket(lua_State *L) {
    package_preload(L, "socket.core", luaopen_socket_core);
    package_preload(L, "mime.core", luaopen_mime_core);

    package_preload(L, "ltn12", open_embed_ltn12);
    package_preload(L, "mbox", open_embed_mbox);
    package_preload(L, "mime", open_embed_mime);
    package_preload(L, "socket", open_embed_socket);
    package_preload(L, "socket.ftp", open_embed_socket_ftp);
    package_preload(L, "socket.headers", open_embed_socket_headers);
    package_preload(L, "socket.http", open_embed_socket_http);
    package_preload(L, "socket.smtp", open_embed_socket_smtp);
    package_preload(L, "socket.tp", open_embed_socket_tp);
    package_preload(L, "socket.url", open_embed_socket_url);
}
#endif  // NEKO_IS_WEB
