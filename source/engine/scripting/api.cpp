

#include <filesystem>

#include "engine/editor.h"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/json.hpp"
#include "base/common/os.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/edit.h"
#include "engine/event.h"
#include "engine/physics.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/scripting.h"
#include "engine/test.h"
#include "engine/ui.h"
#include "engine/bindata.h"
#include "engine/window.h"
#include "engine/scripting/lua_util.h"
#include "engine/scripting/lua_database.h"
#include "engine/components/transform.h"
#include "engine/scripting/wrap_meta.h"

// lua
#include "lapi.h"
#include "ltable.h"

using namespace Neko::luabind;

extern int open_tools_spritepack(lua_State *L);
extern int open_math(lua_State *L);

namespace Neko {

size_t luax_dump_traceback(lua_State *L, char *buf, size_t sz, int is_show_var, int is_show_tmp_var, int top_max, int bottom_max);

namespace luabind {

void package_preload(lua_State *L) {
    luaL_Reg preloads[] = {{"__neko.spritepack", open_tools_spritepack}};
    for (auto m : preloads) {
        luax_package_preload(L, m.name, m.func);
    }
}

}  // namespace luabind

}  // namespace Neko

void package_preload_embed(lua_State *L);

void l_registerFunctions(lua_State *L, int idx, const luaL_Reg *r) {
    if (idx < 0) idx += lua_gettop(L) + 1;
    for (; r && r->name && r->func; ++r) {
        lua_pushstring(L, r->name);
        lua_pushcfunction(L, r->func);
        lua_rawset(L, idx);
    }
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

static int neko_image_load(lua_State *L) {
    String str = luax_check_string(L, 1);
    bool generate_mips = lua_toboolean(L, 2);

    AssetLoadData desc = {};
    desc.kind = AssetKind_Image;
    // desc.generate_mips = generate_mips;

    Asset asset = {};
    bool ok = asset_load(desc, str, &asset);
    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, asset.hash, "mt_image");
    return 1;
}

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

    std::string path = std::format("@code/game/script/{}", luax_check_string(L, 1).cstr());

    Asset asset = {};
    bool ok = asset_load_kind(AssetKind_LuaRef, path, &asset);
    if (!ok) {
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, assets_get<LuaRefID>(asset));
    return 1;
}

static int neko_version(lua_State *L) {
    lua_pushinteger(L, neko_buildnum());
    return 1;
}

static int neko_set_console_window(lua_State *L) {
    the<CL>().win_console = lua_toboolean(L, 1);
    return 0;
}

static int neko_quit(lua_State *L) {
    (void)L;
    // sapp_request_quit();
    return 0;
}

static int neko_fatal_error(lua_State *L) {
    String msg = luax_check_string(L, 1);
    gBase.fatal_error(msg);
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
    lua_pushnumber(L, the<CL>().GetTimeInfo().dt);
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
    lua_pushinteger(L, TimeUtil::now());
    return 1;
}

static int neko_difftime(lua_State *L) {
    lua_Integer t2 = luaL_checkinteger(L, 1);
    lua_Integer t1 = luaL_checkinteger(L, 2);

    lua_pushinteger(L, TimeUtil::difference(t2, t1));
    return 1;
}

static int neko_elapsed(lua_State *L) {
    lua_pushnumber(L, TimeUtil::to_seconds(TimeUtil::now() - the<CL>().GetTimeInfo().startup));
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

static int neko_key_down(lua_State *L) {
    String str = luax_check_string(L, 1);
    i32 key = keyboard_lookup(str);
    bool is_down = input_key_down((KeyCode)key);
    lua_pushboolean(L, is_down);
    return 1;
}

static int neko_key_release(lua_State *L) {
    String str = luax_check_string(L, 1);
    i32 key = keyboard_lookup(str);
    bool is_release = input_key_release((KeyCode)key);
    lua_pushboolean(L, is_release);
    return 1;
}

static int neko_key_press(lua_State *L) {
    String str = luax_check_string(L, 1);
    i32 key = keyboard_lookup(str);
    bool is_press = input_key_down((KeyCode)key);
    lua_pushboolean(L, is_press);
    return 1;
}

static int neko_mouse_down(lua_State *L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    if (n >= 0 && n <= 8) {
        lua_pushboolean(L, input_mouse_down((MouseCode)n));
    } else {
        lua_pushboolean(L, false);
    }

    return 1;
}

static int neko_mouse_pos(lua_State *L) {

    vec2 pos = input_get_mouse_pos_pixels();

    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

static int neko_show_mouse(lua_State *L) {
    bool show = lua_toboolean(L, 1);
    the<Window>().ShowMouse(show);
    return 0;
}

static int neko_scissor_rect(lua_State *L) {
    lua_Number x = luaL_optnumber(L, 1, 0);
    lua_Number y = luaL_optnumber(L, 2, 0);
    // lua_Number w = luaL_optnumber(L, 3, sapp_widthf());
    // lua_Number h = luaL_optnumber(L, 4, sapp_heightf());

    // sgl_scissor_rectf(x, y, w, h, true);
    return 0;
}

static int neko_set_master_volume(lua_State *L) {
    lua_Number vol = luaL_checknumber(L, 1);
    ma_engine_set_volume(the<Sound>().GetAudioEngine(), (float)vol);
    return 0;
}

static int neko_get_channel(lua_State *L) {
    String contents = luax_check_string(L, 1);

    LuaChannel *chan = lua_channel_get(contents);
    if (chan == nullptr) {
        return 0;
    }

    luax_new_userdata(L, chan, "mt_channel");
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

static int neko_file_exists(lua_State *L) {
    String path = luax_check_string(L, 1);
    lua_pushboolean(L, the<VFS>().file_exists(path));
    return 1;
}

static int neko_file_read(lua_State *L) {
    PROFILE_FUNC();

    String path = luax_check_string(L, 1);

    String contents = {};
    bool ok = the<VFS>().read_entire_file(&contents, path);
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

static int neko_make_thread(lua_State *L) {
    String contents = luax_check_string(L, 1);
    LuaThread *lt = (LuaThread *)mem_alloc(sizeof(LuaThread));
    lt->make(contents, "thread");
    luax_new_userdata(L, lt, "mt_thread");
    return 1;
}

static int neko_make_channel(lua_State *L) {
    String name = luax_check_string(L, 1);
    lua_Integer len = luaL_optinteger(L, 2, 0);
    if (len < 0) {
        len = 0;
    }

    LuaChannel *chan = lua_channel_make(name, len);
    luax_new_userdata(L, chan, "mt_channel");
    return 1;
}

#if 1

#if 0

LUA_FUNCTION(wrap_aseprite_render_create) {
    neko_aseprite* sprite_data = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)lua_newuserdata(L, sizeof(neko_aseprite_renderer));
    memset(user_handle, 0, sizeof(neko_aseprite_renderer));
    user_handle->sprite = sprite_data;
    neko_aseprite_renderer_play(user_handle, "Idle");
    luaL_setmetatable(L, "mt_aseprite_renderer");
    return 1;
}

LUA_FUNCTION(wrap_aseprite_render_gc) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    // LOG_INFO("aseprite_render __gc %p", user_handle);
    return 0;
}

LUA_FUNCTION(wrap_aseprite_render_update) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    neko_instance_t* engine = neko_instance();

    neko_aseprite_renderer_update(user_handle, engine->platform->time.delta);

    return 0;
}

LUA_FUNCTION(wrap_aseprite_render_update_animation) {
    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");
    const_str by_tag = lua_tostring(L, 2);
    neko_aseprite_renderer_play(user_handle, by_tag);
    return 0;
}

LUA_FUNCTION(wrap_aseprite_render) {
    PROFILE_FUNC();

    if (lua_gettop(L) != 4) {
        return luaL_error(L, "Function expects exactly 4 arguments");
    }

    luaL_argcheck(L, lua_gettop(L) == 4, 1, "expects exactly 4 arguments");

    neko_aseprite_renderer* user_handle = (neko_aseprite_renderer*)luaL_checkudata(L, 1, "mt_aseprite_renderer");

    auto xform = LuaGet<vec2>(L, 2);

    int direction = Neko::neko_lua_to<int>(L, 3);
    f32 scale = Neko::neko_lua_to<f32>(L, 4);

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
        idraw_rect_textured_ext(&the<CL>().idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u1, f.v0, f.u0, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);
    else
        idraw_rect_textured_ext(&the<CL>().idraw, xform.x, xform.y, xform.x + spr->width * scale, xform.y + spr->height * scale, f.u0, f.v0, f.u1, f.v1, user_handle->sprite->img.id,
                                     NEKO_COLOR_WHITE);

    return 0;
}

LUA_FUNCTION(wrap_aseprite_create) {
    const_str file_path = lua_tostring(L, 1);

    neko_aseprite* user_handle = (neko_aseprite*)lua_newuserdata(L, sizeof(neko_aseprite));
    memset(user_handle, 0, sizeof(neko_aseprite));

    neko_aseprite_load(user_handle, file_path);

    luaL_setmetatable(L, "mt_aseprite");

    return 1;
}

LUA_FUNCTION(wrap_aseprite_gc) {
    neko_aseprite* user_handle = (neko_aseprite*)luaL_checkudata(L, 1, "mt_aseprite");
    if (user_handle->frames != NULL) neko_aseprite_end(user_handle);
    // LOG_INFO("aseprite __gc %p", user_handle);
    return 0;
}

#endif

#if 0

auto wrap_tiled_get_objects(void *tiled_render_ud) {
    tiled_renderer *tiled_render = (tiled_renderer *)tiled_render_ud;
    std::map<std::string, std::vector<std::list<f32>>> data;
    for (u32 i = 0; i < neko_dyn_array_size(tiled_render->map.object_groups); i++) {
        object_group_t *group = tiled_render->map.object_groups + i;
        data.insert({group->name, {}});
        for (u32 ii = 0; ii < neko_dyn_array_size(tiled_render->map.object_groups[i].objects); ii++) {
            object_t *object = group->objects + ii;
            auto &contain = data[group->name];
            contain.push_back(std::list{(f32)(object->x * SPRITE_SCALE), (f32)(object->y * SPRITE_SCALE), (f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)});
        }
    }
    return data;
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
                                 neko_fontbatch_t* fontbatch = Neko::luabind::toudata_ptr<neko_fontbatch_t>(L, 1);
                                 neko_fontbatch_end(fontbatch);
                                 LOG_INFO("fontbatch __gc {}", fontbatch);
                                 return 0;
                             }},
                            {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

namespace Neko::luabind {
template <>
struct udata<neko_fontbatch_t> {
    static inline int nupvalue = 1;
    static inline auto metatable = fontbatch_metatable;
};
}  // namespace Neko::luabind

    int count = luaL_len(L, 2);
    std::vector<const_str> texture_list;

    for (int i = 0; i < count; i++) {
        lua_rawgeti(L, 2, i + 1);
        const_str path = lua_tostring(L, -1);
        texture_list.push_back(path);

        lua_pop(L, 1);
    }

#endif

#if 0

DEFINE_IMGUI_BEGIN(template <>, CGameObject) {
    // Neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { Neko::ImGuiWrap::Auto(value, std::string(field.name)); });
    Neko::reflection::Any v = var;
    v.foreach ([](std::string_view name, Neko::reflection::Any &value) {
        // if (value.GetType() == Neko::reflection::type_of<std::string_view>()) {
        //     std::cout << name << " = " << value.cast<std::string_view>() << std::endl;
        // } else if (value.GetType() == Neko::reflection::type_of<std::size_t>()) {
        //     std::cout << name << " = " << value.cast<std::size_t>() << std::endl;
        // }
        Neko::ImGuiWrap::Auto(value, std::string(name));
    });
}
DEFINE_IMGUI_END();

#endif

#if 0

LUA_FUNCTION(wrap_idraw_get) {
    lua_pushlightuserdata(L, &the<CL>().idraw);
    return 1;
}

LUA_FUNCTION(wrap_idraw_draw) {
    PROFILE_FUNC();
    idraw_draw(&the<CL>().idraw, &the<CL>().cb);
    return 1;
}

LUA_FUNCTION(wrap_idraw_defaults) {
    PROFILE_FUNC();
    idraw_defaults(&the<CL>().idraw);
    return 0;
}

LUA_FUNCTION(wrap_idraw_camera2d) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    idraw_camera2d(&the<CL>().idraw, w, h);
    return 0;
}

LUA_FUNCTION(wrap_idraw_camera3d) {
    f32 w = lua_tonumber(L, 1);
    f32 h = lua_tonumber(L, 2);
    idraw_camera3d(&the<CL>().idraw, w, h);
    return 0;
}

LUA_FUNCTION(wrap_idraw_camera2d_ex) {
    f32 l = lua_tonumber(L, 1);
    f32 r = lua_tonumber(L, 2);
    f32 t = lua_tonumber(L, 3);
    f32 b = lua_tonumber(L, 4);
    idraw_camera2d_ex(&the<CL>().idraw, l, r, t, b);
    return 0;
}

LUA_FUNCTION(wrap_idraw_rotatev) {
    f32 angle = lua_tonumber(L, 1);
    f32 x = lua_tonumber(L, 2);
    f32 y = lua_tonumber(L, 3);
    f32 z = lua_tonumber(L, 4);
    idraw_rotatev(&the<CL>().idraw, angle, neko_v3(x, y, z));
    return 0;
}

LUA_FUNCTION(wrap_idraw_box) {
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
    gfx_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), gfx_primitive_type, &type_val, 11);
    idraw_box(&the<CL>().idraw, x, y, z, hx, hy, hz, r, g, b, a, type_val);
    return 0;
}

LUA_FUNCTION(wrap_idraw_translatef) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 z = lua_tonumber(L, 3);
    idraw_translatef(&the<CL>().idraw, x, y, z);
    return 0;
}

LUA_FUNCTION(wrap_idraw_rectv) {

    auto v1 = LuaGet<vec2>(L, 1);
    auto v2 = LuaGet<vec2>(L, 2);

    v2 = vec2_add(v1, v2);

    gfx_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), gfx_primitive_type, &type_val, 3);

    Color256 col = NEKO_COLOR_WHITE;

    if (lua_gettop(L) == 4) {
        col = LuaGet<Color256>(L, 4);
    }

    idraw_rectv(&the<CL>().idraw, v1, v2, col, type_val);
    return 0;
}

LUA_FUNCTION(wrap_idraw_rectvd) {

    auto v1 = LuaGet<vec2>(L, 1);
    auto v2 = LuaGet<vec2>(L, 2);

    auto uv0 = LuaGet<vec2>(L, 3);
    auto uv1 = LuaGet<vec2>(L, 4);

    gfx_primitive_type type_val;
    neko_luabind_to(ENGINE_LUA(), gfx_primitive_type, &type_val, 5);

    Color256 col = LuaGet<Color256>(L, 6);

    idraw_rectvd(&the<CL>().idraw, v1, v2, uv0, uv1, col, type_val);
    return 0;
}

LUA_FUNCTION(wrap_idraw_text) {
    PROFILE_FUNC();
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    const_str text = lua_tostring(L, 3);

    Color256 col = color256(255, 50, 50, 255);

    if (lua_gettop(L) == 4) {
        col = LuaGet<Color256>(L, 4);
    }

    idraw_text(&the<CL>().idraw, x, y, text, NULL, false, col);
    return 0;
}

// LUA_FUNCTION(wrap_idraw_camera) {
//     f32 x = lua_tonumber(L, 1);
//     f32 y = lua_tonumber(L, 2);
//     neko_camera_t camera;
//     camera = neko_camera_default();
//     vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());
//     idraw_camera(&g_app->idraw, &camera, (u32)fbs.x, (u32)fbs.y);
//     return 0;
// }

LUA_FUNCTION(wrap_idraw_depth_enabled) {
    bool enable = lua_toboolean(L, 1);
    idraw_depth_enabled(&the<CL>().idraw, enable);
    return 0;
}

LUA_FUNCTION(wrap_idraw_face_cull_enabled) {
    bool enable = lua_toboolean(L, 1);
    idraw_face_cull_enabled(&the<CL>().idraw, enable);
    return 0;
}

LUA_FUNCTION(wrap_idraw_texture) {
    gfx_texture_t rt = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(ENGINE_LUA(), gfx_texture_t, id, &rt, 1);
    rt = *CHECK_STRUCT(L, 1, gfx_texture_t);
    idraw_texture(&the<CL>().idraw, rt);
    return 0;
}

LUA_FUNCTION(wrap_render_framebuffer_create) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    fbo = gfx_framebuffer_create({});
    // neko_luabind_struct_push_member(L, neko_framebuffer_t, id, &fbo);
    PUSH_STRUCT(L, neko_framebuffer_t, fbo);
    return 1;
}

LUA_FUNCTION(wrap_render_framebuffer_fini) {
    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);
    fbo = *CHECK_STRUCT(L, 1, neko_framebuffer_t);
    gfx_framebuffer_fini(fbo);
    return 0;
}

void inspect_shader(const_str label, GLuint program) {}

LUA_FUNCTION(wrap_inspect_shaders) {
    u32 shader_id = lua_tonumber(L, 1);
    inspect_shader(std::to_string(shader_id).c_str(), shader_id);
    return 0;
}

LUA_FUNCTION(wrap_inspect_textures) {
    u32 textures_id = lua_tonumber(L, 1);
    return 0;
}

LUA_FUNCTION(wrap_render_shader_create) {

    const_str shader_name = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);              // 检查是否为table
    int n = neko_lua_get_table_pairs_count(L, 2);  //

    gfx_shader_source_desc_t *sources = (gfx_shader_source_desc_t *)mem_alloc(n * sizeof(gfx_shader_source_desc_t));

    const_str shader_type_str[] = {"VERTEX", "FRAGMENT", "COMPUTE"};
    u32 shader_type_enum[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER};

    int j = 0;

    for (int i = 0; i < 3; i++) {
        lua_pushstring(L, shader_type_str[i]);  // # -1
        lua_gettable(L, -2);                    // pop # -1
        if (!lua_isnil(L, -1)) {
            const_str src = lua_tostring(L, -1);  // # -1
            if (src != NULL) {
                sources[j++] = gfx_shader_source_desc_t{.type = shader_type_enum[i], .source = src};
            }
        }
        lua_pop(L, 1);  // # -1
    }

    neko_assert(j == n);

    gfx_shader_t shader_handle = NEKO_DEFAULT_VAL();

    gfx_shader_desc_t shader_desc = {.sources = sources, .size = n * sizeof(gfx_shader_source_desc_t), .name = "unknown"};
    strncpy(shader_desc.name, shader_name, sizeof(shader_desc.name) - 1);
    shader_desc.name[sizeof(shader_desc.name) - 1] = '\0';

    shader_handle = gfx_shader_create(shader_desc);

    mem_free(sources);

    // neko_luabind_struct_push_member(L, gfx_shader_t, id, &shader_handle);

    PUSH_STRUCT(L, gfx_shader_t, shader_handle);

    return 1;
}

LUA_FUNCTION(wrap_render_uniform_create) {

    const_str uniform_name = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    int n = lua_rawlen(L, 2);          //

    gfx_uniform_layout_desc_t *layouts = (gfx_uniform_layout_desc_t *)mem_alloc(n * sizeof(gfx_uniform_layout_desc_t));
    memset(layouts, 0, n * sizeof(gfx_uniform_layout_desc_t));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);  // 将index=i的元素压入堆栈顶部
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_pushstring(L, "type");  // # -1
        lua_gettable(L, -2);        // pop # -1
        if (!lua_isnil(L, -1)) {
            neko_luabind_to(L, gfx_uniform_type, &layouts[i - 1].type, -1);
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

    gfx_uniform_desc_t u_desc = {
            // .name = "unknown",
            .layout = layouts,
    };

    if (!!strcmp(uniform_name, "default")) {
        NEKO_STR_CPY(u_desc.name, uniform_name);
    }

    if (lua_gettop(L) == 3) {
        // neko_luabind_to(L, gfx_shader_stage_type, &u_desc.stage, 3);
        u_desc.stage = lua_tointeger(L, 3);
    }

    neko_uniform_t uniform_handle = NEKO_DEFAULT_VAL();

    // Create uniform
    uniform_handle = gfx_uniform_create(u_desc);

    mem_free(layouts);

    // neko_luabind_struct_push_member(L, neko_uniform_t, id, &uniform_handle);

    PUSH_STRUCT(L, neko_uniform_t, uniform_handle);

    return 1;
}

LUA_FUNCTION(wrap_render_pipeline_create) {
    const_str pipeline_name = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);              // 检查是否为table
    int n = neko_lua_get_table_pairs_count(L, 2);  //
    const_str pipeline_type[] = {"compute", "layout", "raster"};

    gfx_pipeline_desc_t pipeline_desc = NEKO_DEFAULT_VAL();

    for (int i = 0; i < NEKO_ARR_SIZE(pipeline_type); i++) {
        lua_pushstring(L, pipeline_type[i]);  // # -1
        lua_gettable(L, -2);                  // pop # -1
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            int n = neko_lua_get_table_pairs_count(L, -1);  //

            switch (Neko::hash(pipeline_type[i])) {
                case Neko::hash("compute"): {

                    gfx_shader_t shader_handle = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "shader");  // # -1
                    lua_gettable(L, -2);          // pop # -1
                    if (!lua_isnil(L, -1)) {
                        // neko_luabind_struct_to_member(L, gfx_shader_t, id, &shader_handle, -1);
                        shader_handle = *CHECK_STRUCT(L, -1, gfx_shader_t);
                    }
                    lua_pop(L, 1);  // # -1

                    pipeline_desc.compute.shader = shader_handle;
                } break;
                case Neko::hash("layout"): {
                    gfx_vertex_attribute_desc_t *vertex_attr = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "attrs");  // # -1
                    lua_gettable(L, -2);         // pop # -1
                    if (!lua_isnil(L, -1)) {
                        vertex_attr = (gfx_vertex_attribute_desc_t *)lua_touserdata(L, -1);
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
                case Neko::hash("raster"): {

                    gfx_shader_t shader_handle = NEKO_DEFAULT_VAL();

                    lua_pushstring(L, "shader");  // # -1
                    lua_gettable(L, -2);          // pop # -1
                    if (!lua_isnil(L, -1)) {
                        // neko_luabind_struct_to_member(L, gfx_shader_t, id, &shader_handle, -1);
                        shader_handle = *CHECK_STRUCT(L, -1, gfx_shader_t);
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
    pipeline_handle = gfx_pipeline_create(pipeline_desc);
    // neko_luabind_struct_push_member(L, neko_pipeline_t, id, &pipeline_handle);
    PUSH_STRUCT(L, neko_pipeline_t, pipeline_handle);
    return 1;
}

LUA_FUNCTION(wrap_render_vertex_buffer_create) {
    const_str vertex_buffer_name = lua_tostring(L, 1);
    void *data = lua_touserdata(L, 2);
    size_t data_buffer_size = lua_tointeger(L, 3);
    neko_vbo_t vertex_buffer_handle = NEKO_DEFAULT_VAL();
    gfx_vertex_buffer_desc_t vertex_buffer_desc = {.data = data, .size = data_buffer_size};
    vertex_buffer_handle = gfx_vertex_buffer_create(vertex_buffer_desc);
    // neko_luabind_struct_push_member(L, neko_vbo_t, id, &vertex_buffer_handle);
    PUSH_STRUCT(L, neko_vbo_t, vertex_buffer_handle);
    return 1;
}

LUA_FUNCTION(wrap_render_index_buffer_create) {
    const_str index_buffer_name = lua_tostring(L, 1);
    void *data = lua_touserdata(L, 2);
    size_t data_buffer_size = lua_tointeger(L, 3);
    neko_ibo_t index_buffer_handle = NEKO_DEFAULT_VAL();
    gfx_index_buffer_desc_t index_buffer_desc = {.data = data, .size = data_buffer_size};
    index_buffer_handle = gfx_index_buffer_create(index_buffer_desc);
    // neko_luabind_struct_push_member(L, neko_ibo_t, id, &index_buffer_handle);
    PUSH_STRUCT(L, neko_ibo_t, index_buffer_handle);
    return 1;
}

LUA_FUNCTION(wrap_render_vertex_attribute_create) {
    const_str vertex_attribute_name = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int n = neko_lua_get_table_pairs_count(L, 2);  //

    gfx_vertex_attribute_desc_t *sources = (gfx_vertex_attribute_desc_t *)mem_alloc(n * sizeof(gfx_vertex_attribute_desc_t));
    memset(sources, 0, n * sizeof(gfx_vertex_attribute_desc_t));

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
            neko_luabind_to(L, gfx_vertex_attribute_type, &sources[i].format, -1);
        }
        lua_pop(L, 1);  // # -1

        lua_pop(L, 1);  // # -1
    }

    gfx_vertex_attribute_desc_t *push_userdata = (gfx_vertex_attribute_desc_t *)lua_newuserdata(L, n * sizeof(gfx_vertex_attribute_desc_t));
    memcpy(push_userdata, sources, n * sizeof(gfx_vertex_attribute_desc_t));

    mem_free(sources);

    lua_pushinteger(L, n * sizeof(gfx_vertex_attribute_desc_t));

    return 2;
}

LUA_FUNCTION(wrap_render_pipeline_bind) {
    neko_pipeline_t pipeline_handle = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, neko_pipeline_t, id, &pipeline_handle, 1);
    pipeline_handle = *CHECK_STRUCT(L, 1, neko_pipeline_t);
    gfx_pipeline_bind(&the<CL>().cb, pipeline_handle);
    return 0;
}

LUA_FUNCTION(wrap_render_apply_bindings) {

    gfx_bind_desc_t binds = NEKO_DEFAULT_VAL();

    const_str pipeline_type[] = {"uniforms", "image_buffers", "vertex_buffers", "index_buffers"};

    gfx_bind_uniform_desc_t *u_desc = nullptr;
    gfx_bind_image_buffer_desc_t *ib_desc = nullptr;
    // gfx_bind_storage_buffer_desc_t *sb_desc = nullptr;
    gfx_bind_vertex_buffer_desc_t *vbo_desc = nullptr;
    gfx_bind_index_buffer_desc_t *ibo_desc = nullptr;

    luaL_checktype(L, -1, LUA_TTABLE);

    f32 data;  // TODO

    for (int i = 0; i < NEKO_ARR_SIZE(pipeline_type); i++) {
        lua_pushstring(L, pipeline_type[i]);  // # -1
        lua_gettable(L, -2);                  // pop # -1
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            int n = neko_lua_get_table_pairs_count(L, -1);  //

            switch (Neko::hash(pipeline_type[i])) {
                case Neko::hash("uniforms"): {

                    u_desc = (gfx_bind_uniform_desc_t *)mem_alloc(n * sizeof(gfx_bind_uniform_desc_t));
                    memset(u_desc, 0, n * sizeof(gfx_bind_uniform_desc_t));

                    binds.uniforms.desc = u_desc;
                    binds.uniforms.size = n == 1 ? 0 : n * sizeof(gfx_bind_uniform_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "uniform");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (!lua_isnil(L, -1)) {
                            // neko_luabind_struct_to_member(L, neko_uniform_t, id, &u_desc[i - 1].uniform, -1);
                            u_desc[i - 1].uniform = *CHECK_STRUCT(L, -1, neko_uniform_t);
                        } else
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
                            u_desc[i - 1].binding = Neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case Neko::hash("image_buffers"): {
                    ib_desc = (gfx_bind_image_buffer_desc_t *)mem_alloc(n * sizeof(gfx_bind_image_buffer_desc_t));
                    memset(ib_desc, 0, n * sizeof(gfx_bind_image_buffer_desc_t));

                    binds.image_buffers.desc = ib_desc;
                    binds.image_buffers.size = n == 1 ? 0 : n * sizeof(gfx_bind_image_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "tex");  // # -1
                        lua_gettable(L, -2);       // pop # -1
                        if (!lua_isnil(L, -1)) {
                            // neko_luabind_struct_to_member(L, gfx_texture_t, id, &ib_desc[i - 1].tex, -1);
                            ib_desc[i - 1].tex = *CHECK_STRUCT(L, -1, gfx_texture_t);
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        ib_desc[i - 1].access = GL_WRITE_ONLY;

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            ib_desc[i - 1].binding = Neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
#if 0
                case Neko::hash("storage_buffers"): {
                    sb_desc = (gfx_bind_storage_buffer_desc_t *)mem_alloc(n * sizeof(gfx_bind_storage_buffer_desc_t));
                    memset(sb_desc, 0, n * sizeof(gfx_bind_storage_buffer_desc_t));

                    binds.storage_buffers.desc = sb_desc;
                    binds.storage_buffers.size = n == 1 ? 0 : n * sizeof(gfx_bind_storage_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            // neko_luabind_struct_to_member(L, neko_storage_buffer_t, id, &sb_desc[i - 1].buffer, -1);
                            sb_desc[i - 1].buffer = *CHECK_STRUCT(L, -1, neko_storage_buffer_t);
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pushstring(L, "binding");  // # -1
                        lua_gettable(L, -2);           // pop # -1
                        if (lua_isinteger(L, -1)) {
                            sb_desc[i - 1].binding = Neko::neko_lua_to<u32>(L, -1);
                        }
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
#endif
                case Neko::hash("vertex_buffers"): {
                    vbo_desc = (gfx_bind_vertex_buffer_desc_t *)mem_alloc(n * sizeof(gfx_bind_vertex_buffer_desc_t));
                    memset(vbo_desc, 0, n * sizeof(gfx_bind_vertex_buffer_desc_t));

                    binds.vertex_buffers.desc = vbo_desc;
                    binds.vertex_buffers.size = n == 1 ? 0 : n * sizeof(gfx_bind_vertex_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            // neko_luabind_struct_to_member(L, neko_vbo_t, id, &vbo_desc[i - 1].buffer, -1);
                            vbo_desc[i - 1].buffer = *CHECK_STRUCT(L, -1, neko_vbo_t);
                        } else
                            neko_assert(false);
                        lua_pop(L, 1);  // # -1

                        lua_pop(L, 1);  // # -1
                    }
                } break;
                case Neko::hash("index_buffers"): {
                    ibo_desc = (gfx_bind_index_buffer_desc_t *)mem_alloc(n * sizeof(gfx_bind_index_buffer_desc_t));
                    memset(ibo_desc, 0, n * sizeof(gfx_bind_index_buffer_desc_t));

                    binds.index_buffers.desc = ibo_desc;
                    binds.index_buffers.size = n == 1 ? 0 : n * sizeof(gfx_bind_index_buffer_desc_t);

                    for (int i = 1; i <= n; i++) {
                        lua_rawgeti(L, -1, i);  // 将index=i的元素压入堆栈顶部
                        luaL_checktype(L, -1, LUA_TTABLE);

                        lua_pushstring(L, "buffer");  // # -1
                        lua_gettable(L, -2);          // pop # -1
                        if (!lua_isnil(L, -1)) {
                            // neko_luabind_struct_to_member(L, neko_ibo_t, id, &ibo_desc[i - 1].buffer, -1);
                            ibo_desc[i - 1].buffer = *CHECK_STRUCT(L, -1, neko_ibo_t);
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

    gfx_apply_bindings(&the<CL>().cb, &binds);

    if (u_desc) mem_free(u_desc);
    if (ib_desc) mem_free(ib_desc);
    if (vbo_desc) mem_free(vbo_desc);
    if (ibo_desc) mem_free(ibo_desc);

    return 0;
}

static int test_tex(lua_State *L) {

#define ROW_COL_CT 10

    Color256 c0 = NEKO_COLOR_WHITE;
    Color256 c1 = color256(20, 50, 150, 255);
    // Color256 pixels[ROW_COL_CT * ROW_COL_CT] = NEKO_DEFAULT_VAL();

    Color256 *pixels = (Color256 *)lua_newuserdata(L, sizeof(Color256) * ROW_COL_CT * ROW_COL_CT);

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

LUA_FUNCTION(wrap_render_dispatch_compute) {
    f32 x_groups = lua_tonumber(L, 1);
    f32 y_groups = lua_tonumber(L, 2);
    f32 z_groups = lua_tonumber(L, 3);
    gfx_dispatch_compute(&the<CL>().cb, x_groups, y_groups, z_groups);
    return 0;
}

LUA_FUNCTION(wrap_render_texture_create) {
    u32 w = lua_tointeger(L, 1);
    u32 h = lua_tointeger(L, 2);
    gfx_texture_desc_t texture_desc = {
            .width = w,                             // 纹理的宽度
            .height = h,                            // 纹理的高度
            .format = R_TEXTURE_FORMAT_RGBA8,       // 纹理数据的格式
            .wrap_s = GL_REPEAT,                    // 纹理 s 轴的包裹类型
            .wrap_t = GL_REPEAT,                    // 纹理 t 轴的包裹类型
            .min_filter = R_TEXTURE_FILTER_LINEAR,  // 纹理缩小过滤器
            .mag_filter = R_TEXTURE_FILTER_LINEAR   // 纹理放大滤镜
    };
    if (lua_gettop(L) == 3) {
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);

            // lua_pushstring(L, "type");  // # -1
            // lua_gettable(L, -2);        // pop # -1
            // if (!lua_isnil(L, -1)) {
            //     neko_luabind_to(L, gfx_texture_type, &texture_desc.type, -1);
            // }
            // lua_pop(L, 1);  // # -1

            lua_pushstring(L, "format");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, gfx_texture_format_type, &texture_desc.format, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_s");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                // neko_luabind_to(L, gfx_texture_wrapping_type, &texture_desc.wrap_s, -1);
                texture_desc.wrap_s = lua_tointeger(L, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "wrap_t");  // # -1
            lua_gettable(L, -2);          // pop # -1
            if (!lua_isnil(L, -1)) {
                // neko_luabind_to(L, gfx_texture_wrapping_type, &texture_desc.wrap_t, -1);
                texture_desc.wrap_t = lua_tointeger(L, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "min_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, gfx_texture_filtering_type, &texture_desc.min_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "mag_filter");  // # -1
            lua_gettable(L, -2);              // pop # -1
            if (!lua_isnil(L, -1)) {
                neko_luabind_to(L, gfx_texture_filtering_type, &texture_desc.mag_filter, -1);
            }
            lua_pop(L, 1);  // # -1

            lua_pushstring(L, "data");  // # -1
            lua_gettable(L, -2);        // pop # -1
            if (!lua_isnil(L, -1)) {
                void *data = lua_touserdata(L, -1);
                texture_desc.data = data;
            }
            lua_pop(L, 1);  // # -1
        }
    }
    gfx_texture_t texture = NEKO_DEFAULT_VAL();
    texture = gfx_texture_create(texture_desc);
    // neko_luabind_struct_push_member(L, gfx_texture_t, id, &texture);
    PUSH_STRUCT(L, gfx_texture_t, texture);
    return 1;
}

LUA_FUNCTION(wrap_render_texture_fini) {
    gfx_texture_t rt = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, gfx_texture_t, id, &rt, 1);
    rt = *CHECK_STRUCT(L, 1, gfx_texture_t);
    gfx_texture_fini(rt);
    return 0;
}

LUA_FUNCTION(wrap_render_renderpass_create) {

    neko_framebuffer_t fbo = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, neko_framebuffer_t, id, &fbo, 1);
    fbo = *CHECK_STRUCT(L, 1, neko_framebuffer_t);

    gfx_texture_t rt = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, gfx_texture_t, id, &rt, 2);
    rt = *CHECK_STRUCT(L, 2, gfx_texture_t);

    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    rp = gfx_renderpass_create(gfx_renderpass_desc_t{
            .fbo = fbo,               // Frame buffer to bind for render pass
            .color = &rt,             // Color buffer array to bind to frame buffer
            .color_size = sizeof(rt)  // Size of color attachment array in bytes
    });
    // neko_luabind_struct_push_member(L, neko_renderpass_t, id, &rp);
    PUSH_STRUCT(L, neko_renderpass_t, rp);
    return 1;
}

LUA_FUNCTION(wrap_render_renderpass_fini) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    rp = *CHECK_STRUCT(L, 1, neko_renderpass_t);
    gfx_renderpass_fini(rp);
    return 0;
}

LUA_FUNCTION(wrap_render_renderpass_default) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    PUSH_STRUCT(L, neko_renderpass_t, rp);
    return 1;
}

LUA_FUNCTION(wrap_render_renderpass_begin) {
    neko_renderpass_t rp = NEKO_DEFAULT_VAL();
    // neko_luabind_struct_to_member(L, neko_renderpass_t, id, &rp, 1);
    rp = *CHECK_STRUCT(L, 1, neko_renderpass_t);
    gfx_renderpass_begin(&the<CL>().cb, rp);
    return 0;
}

LUA_FUNCTION(wrap_render_renderpass_end) {
    gfx_renderpass_end(&the<CL>().cb);
    return 0;
}

LUA_FUNCTION(wrap_render_draw) {
    gfx_draw_desc_t draw_desc = NEKO_DEFAULT_VAL();

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

    gfx_draw(&the<CL>().cb, draw_desc);
    return 0;
}

LUA_FUNCTION(wrap_render_set_viewport) {
    f32 x = lua_tonumber(L, 1);
    f32 y = lua_tonumber(L, 2);
    f32 w = lua_tonumber(L, 3);
    f32 h = lua_tonumber(L, 4);
    gfx_set_viewport(&the<CL>().cb, x, y, w, h);
    return 0;
}

LUA_FUNCTION(wrap_render_clear) {
    f32 r = lua_tonumber(L, 1);
    f32 g = lua_tonumber(L, 2);
    f32 b = lua_tonumber(L, 3);
    f32 a = lua_tonumber(L, 4);
    gfx_clear_action_t clear = {.color = {r, g, b, a}};
    gfx_clear(&the<CL>().cb, clear);
    return 0;
}

LUA_FUNCTION(wrap_render_display_size) {
    // vec2 v1 = neko_game()->DisplaySize;
    // lua2struct::pack_struct<vec2, 2>(L, v1);
    lua_pushnumber(L, the<CL>().width);
    lua_pushnumber(L, the<CL>().height);
    return 2;
}

#endif

static int open_enum(lua_State *L) { return 0; }

#define NEKO_LUA_INSPECT_ITER(NAME)                                                                          \
    LUA_FUNCTION(wrap_inspect_##NAME##_next) {                                                               \
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
    LUA_FUNCTION(wrap_inspect_##NAME##_iterator) {                                                           \
        neko_gl_data_t *ogl = gfx_ogl();                                                                     \
        neko_slot_array_iter *it = (neko_slot_array_iter *)lua_newuserdata(L, sizeof(neko_slot_array_iter)); \
        *it = neko_slot_array_iter_new(ogl->NAME);                                                           \
        lua_pushlightuserdata(L, ogl);                                                                       \
        lua_pushvalue(L, -2);                                                                                \
        lua_pushcclosure(L, wrap_inspect_##NAME##_next, 2);                                                  \
        return 1;                                                                                            \
    }

#define NEKO_LUA_INSPECT_REG(NAME) {"inspect_" #NAME "_iter", wrap_inspect_##NAME##_iterator}

// NEKO_LUA_INSPECT_ITER(textures)
// NEKO_LUA_INSPECT_ITER(vertex_buffers)
// NEKO_LUA_INSPECT_ITER(index_buffers)
// NEKO_LUA_INSPECT_ITER(frame_buffers)
// NEKO_LUA_INSPECT_ITER(uniform_buffers)
// NEKO_LUA_INSPECT_ITER(storage_buffers)
// NEKO_LUA_INSPECT_ITER(uniforms)
// NEKO_LUA_INSPECT_ITER(pipelines)
// NEKO_LUA_INSPECT_ITER(renderpasses)

LUA_FUNCTION(wrap_print) {
    int n = lua_gettop(L);
    int i;
    std::string str;
    lua_getglobal(L, "tostring");  // 获取全局函数 tostring
    for (i = 1; i <= n; i++) {
        const char *s;
        lua_pushvalue(L, -1);  // 将 tostring 函数推入堆栈
        lua_pushvalue(L, i);   // 将第 i 个参数推入堆栈
        lua_call(L, 1, 1);     // 调用 tostring
        s = lua_tostring(L, -1);
        if (s == NULL) return luaL_error(L, "'tostring' must return a string to 'print'");
        if (i > 1) str.append("\t");
        str.append(std::string(s, strlen(s)));
        lua_pop(L, 1);
    }

#ifdef DEBUG
    lua_Debug ar;
    if (lua_getstack(L, 2, &ar)) {  // 获取调用栈信息
        lua_getinfo(L, "Sl", &ar);  // 获取文件名(S)和行号(l)
        Logger::getInstance()->log("", ar.short_src, ar.currentline, Logger::Level::INFO, "{}", str);
    } else {
        Logger::getInstance()->log("", "lua", n, Logger::Level::INFO, "{}", str);
    }
#else
    Logger::getInstance()->log("", "lua", n, Logger::Level::INFO, "{}", str);
#endif

    return 0;
}

LUA_FUNCTION(wrap_bindata_build) {

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

    bool ok = BinDataBuild(path, n, item_paths, true);

    mem_free(item_paths);

    if (!ok) {
        const_str error_message = "wrap_bindata_build failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    return 0;
}

LUA_FUNCTION(wrap_bindata_info) {
    const_str path = lua_tostring(L, 1);
    i32 buildnum;
    u64 item_count;
    bool ok = BinDataInfo(path, &buildnum, &item_count);
    if (ok) {
        lua_pushinteger(L, buildnum);
        lua_pushinteger(L, item_count);
        return 2;
    } else
        return 0;
}

LUA_FUNCTION(wrap_vfs_read_file) {
    const_str path = lua_tostring(L, 1);

    String str;
    bool ok = the<VFS>().read_entire_file(&str, path);

    if (ok) {
        const_str data = str.data;
        lua_pushlstring(L, str.data, str.len);
        neko_defer(mem_free(str.data));
        return 1;
    } else {
        const_str error_message = "todo";
        lua_pushstring(L, error_message);
        return lua_error(L);
    }
}

LUA_FUNCTION(wrap_vfs_load_luabp) {
    String name = luax_check_string(L, 1);

    LuaRef tb = LuaRef::FromStack(L, 2);

    bool ok = the<VFS>().mount_type<LuaBpFileSystem>(name, "", L, tb);

    if (!ok) {
        return luaL_error(L, "Failed to mount LuaBpFileSystem");
    }

    return 0;
}

// 返回包含路径和 isDirectory 对的表
int __neko_ls(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        LOG_INFO("invalid lua argument");
        return 0;
    }
    auto string = lua_tostring(L, 1);
    if (!std::filesystem::is_directory(string)) {
        LOG_INFO("{0} is not directory", string);
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

#if 0

int register_mt_aseprite_renderer(lua_State* L) {
    luaL_Reg reg[] = {
            {"__gc", wrap_aseprite_render_gc},
            {"create", wrap_aseprite_render_create},
            {"update", wrap_aseprite_render_update},
            {"render", wrap_aseprite_render},
            {"update_animation", wrap_aseprite_render_update_animation},
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
            {"__gc", wrap_aseprite_gc},
            {"create", wrap_aseprite_create},
            {NULL, NULL},
    };
    luaL_newmetatable(L, "mt_aseprite");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);            // # -1 复制一份 为了让 neko 主表设定
    lua_setfield(L, -2, "__index");  // # -2
    return 1;
}

#endif

LUA_FUNCTION(wrap_ecs_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_ECS_CORE);
    return 1;
}

LUA_FUNCTION(wrap_w_f) {
    lua_getfield(L, LUA_REGISTRYINDEX, W_LUA_REGISTRY_CONST::W_CORE);
    return 1;
}

static int __neko_w_lua_get_com(lua_State *L) {
    CL *w = (CL *)luaL_checkudata(L, W_LUA_REGISTRY_CONST::W_CORE_IDX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);
    lua_getiuservalue(L, 1, W_LUA_UPVALUES::NEKO_W_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, W_LUA_UPVALUES::NEKO_W_UPVAL_N);
    return 2;
}

template <>
struct std::formatter<CL *> : std::formatter<void *> {
    auto format(CL *ptr, std::format_context &ctx) const { return std::formatter<void *>::format(static_cast<void *>(ptr), ctx); }
};

static int __neko_w_lua_gc(lua_State *L) {
    CL *w = (CL *)luaL_checkudata(L, W_LUA_REGISTRY_CONST::W_CORE_IDX, W_LUA_REGISTRY_CONST::ENG_UDATA_NAME);
    // ecs_fini_i(w);
    LOG_INFO("Game __gc {}", w);
    return 0;
}

void neko_w_init() {

    lua_State *L = ENGINE_LUA();

    CL *ins = (CL *)lua_newuserdatauv(L, sizeof(CL), NEKO_W_UPVAL_N);  // # -1
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

    lua_register(L, "neko_w_f", wrap_w_f);

    // neko_w_lua_variant<i64> version("neko_engine_version", neko_buildnum());

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

static int l_bbox_bound(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    BBox v = bbox_bound(*v1, *v2);
    LuaPush<BBox>(L, v);
    return 1;
}

static int l_bbox_merge(lua_State *L) {
    BBox *v1 = LuaGet<BBox>(L, 1);
    BBox *v2 = LuaGet<BBox>(L, 2);
    BBox v = bbox_merge(*v1, *v2);
    LuaPush<BBox>(L, v);
    return 1;
}

static int l_bbox_contains(lua_State *L) {
    BBox *v1 = LuaGet<BBox>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    bool v = bbox_contains(*v1, *v2);
    lua_pushboolean(L, v);
    return 1;
}

static int l_bbox_contains2(lua_State *L) {
    BBox *v1 = LuaGet<BBox>(L, 1);
    mat3 *v2 = LuaGet<mat3>(L, 2);
    vec2 *v3 = LuaGet<vec2>(L, 3);

    vec2 pos = mat3_transform(mat3_inverse(*v2), *v3);

    bool v = bbox_contains(*v1, pos);
    lua_pushboolean(L, v);
    return 1;
}

static int l_bbox_transform(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    BBox *v2 = LuaGet<BBox>(L, 2);
    BBox v = bbox_transform(*v1, *v2);
    LuaPush<BBox>(L, v);
    return 1;
}

static int l_transform_get_world_matrix(lua_State *L) {
    CEntity *ent = LuaGet<CEntity>(L, 1);
    mat3 v = the<Transform>().transform_get_world_matrix(*ent);
    LuaPush<mat3>(L, v);
    return 1;
}

// 将table转换为lightuserdata
int tbptr_to(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    const void *tb = lua_topointer(L, 1);
    lua_pushlightuserdata(L, (void *)tb);
    return 1;
}

// 通过lightuserdata指针创建table
int tbptr_create(lua_State *L) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);   // 确保参数是 lightuserdata
    Table *tb = (Table *)lua_touserdata(L, 1);  // 取出指针并转换为 Table*
    int narr = tb->alimit;                      // 获取数组部分的大小
    int nrec = 1 << tb->lsizenode;              // 计算哈希部分的大小 (2^lsizenode)
    lua_createtable(L, narr, nrec);             // 创建一个新的 Lua 表
    return 1;
}

// pairs推入迭代器
int tbptr_pairs(lua_State *L) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);  // 确保参数是 lightuserdata

    // fastable迭代器
    auto iter = [](lua_State *L) -> int {
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);  // 确保参数是 lightuserdata
        Table *t = (Table *)lua_touserdata(L, 1);  // 取出指针并转换为 Table*
        int index = luaL_checkinteger(L, 2);       // 获取索引

        if (index >= 0) {  // 遍历数组部分
            if (t->alimit > 0) {
                index = LuaTableArrayNext(t, index);
                if (index != 0) {
                    lua_pushinteger(L, index);
                    lua_pushinteger(L, index);
                    PushTValue(L, &t->array[index - 1]);
                    return 3;
                }
            }
        }
        index = LuaTableHashNext(t, index);  // 遍历哈希部分
        if (index != 0) {
            Node *n = gnode(t, -index - 1);
            lua_pushinteger(L, index);
            TValue tv{n->u.key_val, n->u.key_tt};
            PushTValue(L, &tv);
            PushTValue(L, gval(n));
            return 3;
        }
        return 0;
    };

    lua_pushcfunction(L, iter);  // 压入迭代器函数
    lua_pushvalue(L, 1);         // 压入表指针
    lua_pushinteger(L, 0);       // 索引
    return 3;                    // iter | table | 0
}

extern int open_cdata(lua_State *L);

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
            "cdata",     // 10
    };
    size_t i;
    const size_t n = NEKO_ARR_SIZE(typenames);
    for (i = 0; i < n; i++) {
        lua_pushstring(L, typenames[i]);
    }
    lua_pushcclosure(L, ltype, n);
}

#endif

int wrap_timing_set_scale(lua_State *L) {
    f32 v = lua_tonumber(L, 1);
    the<CL>().timing_set_scale(v);
    return 0;
}
int wrap_timing_get_scale(lua_State *L) {
    f32 v = the<CL>().timing_get_scale();
    lua_pushnumber(L, v);
    return 1;
}
int wrap_timing_get_elapsed(lua_State *L) {
    f32 v = the<CL>().timing_get_elapsed();
    lua_pushnumber(L, v);
    return 1;
}
int wrap_timing_set_paused(lua_State *L) {
    bool v = lua_toboolean(L, 1);
    the<CL>().timing_set_paused(v);
    return 0;
}
int wrap_timing_get_paused(lua_State *L) {
    bool v = the<CL>().timing_get_paused();
    lua_pushboolean(L, v);
    return 1;
}

int wrap_luamat3(lua_State *L) {
    f32 v1 = LuaGet<f32>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    f32 v3 = LuaGet<f32>(L, 3);
    f32 v4 = LuaGet<f32>(L, 4);
    f32 v5 = LuaGet<f32>(L, 5);
    f32 v6 = LuaGet<f32>(L, 6);
    f32 v7 = LuaGet<f32>(L, 7);
    f32 v8 = LuaGet<f32>(L, 8);
    f32 v9 = LuaGet<f32>(L, 9);
    mat3 ret = luamat3(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    LuaPush<mat3>(L, ret);
    return 1;
}
int wrap_mat3_identity(lua_State *L) {
    mat3 ret = mat3_identity();
    LuaPush<mat3>(L, ret);
    return 1;
}
int wrap_mat3_mul(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    mat3 *v2 = LuaGet<mat3>(L, 2);
    mat3 ret = mat3_mul(*v1, *v2);
    LuaPush<mat3>(L, ret);
    return 1;
}
int wrap_mat3_scaling_rotation_translation(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    vec2 *v3 = LuaGet<vec2>(L, 3);
    mat3 ret = mat3_scaling_rotation_translation(*v1, v2, *v3);
    LuaPush<mat3>(L, ret);
    return 1;
}
int wrap_mat3_get_translation(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    vec2 ret = mat3_get_translation(*v1);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_mat3_get_rotation(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    f32 ret = mat3_get_rotation(*v1);
    LuaPush(L, ret);
    return 1;
}
int wrap_mat3_get_scale(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    vec2 ret = mat3_get_scale(*v1);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_mat3_inverse(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    mat3 ret = mat3_inverse(*v1);
    LuaPush<mat3>(L, ret);
    return 1;
}
int wrap_mat3_transform(lua_State *L) {
    mat3 *v1 = LuaGet<mat3>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    vec2 ret = mat3_transform(*v1, *v2);
    LuaPush<vec2>(L, ret);
    return 1;
}

int wrap_vec2_alloc(lua_State *L) {
    f32 v1 = LuaGet<f32>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    vec2 ret = luavec2(v1, v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_add(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    vec2 ret = vec2_add(*v1, *v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_sub(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    vec2 ret = vec2_sub(*v1, *v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_mul(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    vec2 ret = vec2_mul(*v1, *v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_div(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    vec2 ret = vec2_div(*v1, *v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_float_mul(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    vec2 ret = vec2_float_mul(*v1, v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_float_div(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    vec2 ret = vec2_float_div(*v1, v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_float_vec2_div(lua_State *L) {
    f32 v1 = LuaGet<f32>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    vec2 ret = float_vec2_div(v1, *v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_neg(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 ret = vec2_neg(*v1);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_len(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    f32 ret = vec2_len(*v1);
    LuaPush(L, ret);
    return 1;
}
int wrap_vec2_normalize(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 ret = vec2_normalize(*v1);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_dot(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    f32 ret = vec2_dot(*v1, *v2);
    LuaPush(L, ret);
    return 1;
}
int wrap_vec2_dist(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    vec2 *v2 = LuaGet<vec2>(L, 2);
    f32 ret = vec2_dist(*v1, *v2);
    LuaPush(L, ret);
    return 1;
}
int wrap_vec2_rot(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    vec2 ret = vec2_rot(*v1, v2);
    LuaPush<vec2>(L, ret);
    return 1;
}
int wrap_vec2_atan2(lua_State *L) {
    vec2 *v1 = LuaGet<vec2>(L, 1);
    f32 ret = vec2_atan2(*v1);
    LuaPush(L, ret);
    return 1;
}

int wrap_color(lua_State *L) {
    f32 v1 = LuaGet<f32>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    f32 v3 = LuaGet<f32>(L, 3);
    f32 v4 = LuaGet<f32>(L, 4);
    Color ret = color(v1, v2, v3, v4);
    LuaPush<Color>(L, ret);
    return 1;
}
int wrap_color_opaque(lua_State *L) {
    f32 v1 = LuaGet<f32>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    f32 v3 = LuaGet<f32>(L, 3);
    Color ret = color_opaque(v1, v2, v3, 1);
    LuaPush<Color>(L, ret);
    return 1;
}

int wrap_pixels_to_unit(lua_State *L) {
    f32 v1 = LuaGet<f32>(L, 1);
    f32 v2 = LuaGet<f32>(L, 2);
    vec2 ret = the<CL>().pixels_to_unit({v1, v2});
    LuaPush<vec2>(L, ret);
    return 1;
}

// DEFINE_LUAOPEN_EXTERN(LuaDatabase)
DEFINE_LUAOPEN_EXTERN(unittest)

static int open_neko(lua_State *L) {

    lua_pushfstring(L, "Neko %d", neko_buildnum());
    lua_setglobal(L, "_NEKO_VERSION");

    luaL_Reg reg[] = {
            // internal
            {"__registry_load", neko_registry_load},
            {"__registry_lua_script", neko_require_lua_script},
            {"__cdata", open_cdata},

            {"ecs_f", wrap_ecs_f},

            {"callback_save", __neko_bind_callback_save},
            {"callback_call", __neko_bind_callback_call},

            {"vfs_read_file", wrap_vfs_read_file},
            {"vfs_load_luabp", wrap_vfs_load_luabp},

            {"tbptr_create", tbptr_create},
            {"tbptr_pairs", tbptr_pairs},
            {"tbptr_to", tbptr_to},

            {"print", wrap_print},

            // bindata
            {"bindata_build", wrap_bindata_build},
            {"bindata_info", wrap_bindata_info},

            // reg
            {"from_registry", from_registry},

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
            {"mouse_pos", neko_mouse_pos},
            {"show_mouse", neko_show_mouse},

            {"bbox_bound", l_bbox_bound},
            {"bbox_merge", l_bbox_merge},
            {"bbox_contains", l_bbox_contains},
            {"bbox_contains2", l_bbox_contains2},
            {"bbox_transform", l_bbox_transform},
            {"transform_get_world_matrix", l_transform_get_world_matrix},

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
            {"file_exists", neko_file_exists},
            {"file_read", neko_file_read},
            {"file_write", neko_file_write},

            // construct types
            {"make_thread", neko_make_thread},
            {"make_channel", neko_make_channel},
            //{"image_load", neko_image_load},
            {"font_load", FontWrap::neko_font_load},
            {"sound_load", SoundWrap::neko_sound_load},
            {"sprite_load", SpriteWrap::neko_sprite_load},
            // {"atlas_load", neko_atlas_load},
            // {"tilemap_load", neko_tilemap_load},
            {"bindata_load", mt_bindata::neko_bindata_load},
#ifdef NEKO_BOX2D
            {"b2_world", neko_b2_world},
#endif

            {"timing_set_scale", wrap_timing_set_scale},
            {"timing_get_scale", wrap_timing_get_scale},
            {"timing_get_elapsed", wrap_timing_get_elapsed},
            {"timing_set_paused", wrap_timing_set_paused},
            {"timing_get_paused", wrap_timing_get_paused},

            {"pixels_to_unit", wrap_pixels_to_unit},

            {"luamat3", wrap_luamat3},
            {"mat3_identity", wrap_mat3_identity},
            {"mat3_mul", wrap_mat3_mul},
            {"mat3_scaling_rotation_translation", wrap_mat3_scaling_rotation_translation},
            {"mat3_get_translation", wrap_mat3_get_translation},
            {"mat3_get_rotation", wrap_mat3_get_rotation},
            {"mat3_get_scale", wrap_mat3_get_scale},
            {"mat3_inverse", wrap_mat3_inverse},
            {"mat3_transform", wrap_mat3_transform},

            {"vec2_alloc", wrap_vec2_alloc},
            {"vec2_add", wrap_vec2_add},
            {"vec2_sub", wrap_vec2_sub},
            {"vec2_mul", wrap_vec2_mul},
            {"vec2_div", wrap_vec2_div},
            {"vec2_float_mul", wrap_vec2_float_mul},
            {"vec2_float_div", wrap_vec2_float_div},
            {"float_vec2_div", wrap_float_vec2_div},
            {"vec2_neg", wrap_vec2_neg},
            {"vec2_len", wrap_vec2_len},
            {"vec2_normalize", wrap_vec2_normalize},
            {"vec2_dot", wrap_vec2_dot},
            {"vec2_dist", wrap_vec2_dist},
            {"vec2_rot", wrap_vec2_rot},
            {"vec2_atan2", wrap_vec2_atan2},

            {"color", wrap_color},
            {"color_opaque", wrap_color_opaque},

            {nullptr, nullptr},
    };

    luaL_newlib(L, reg);

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_State *LD = L;
        if (lua_isthread(L, 1)) {
            LD = lua_tothread(L, 1);
            lua_remove(L, 1);
        }
        int top_max = luaL_optinteger(L, 1, 32);
        int bottom_max = luaL_optinteger(L, 2, 16);
        bool is_show_var = lua_isnoneornil(L, 3) ? true : lua_toboolean(L, 3);
        bool is_show_tmp_var = lua_isnoneornil(L, 4) ? true : lua_toboolean(L, 4);
        size_t buff_sz = luaL_optinteger(L, 5, 32 * 1024);

        char *buff = (char *)mem_alloc(buff_sz);
        if (buff == NULL) return 0;
        size_t len = luax_dump_traceback(LD, buff, buff_sz, is_show_var, is_show_tmp_var, top_max, bottom_max);
        lua_pushlstring(L, buff, len);
        mem_free(buff);
        return 1;
    });
    lua_setfield(L, -2, "traceback");

    return 1;
}

void createStructTables(lua_State *L) {

    LuaEnum<KeyCode, -1, 350>(L);
    LuaEnum<MouseCode>(L);
    LuaEnum<TextureFlags>(L);
    LuaEnum<AssetKind>(L);

    LuaStruct<vec2>(L);
    LuaStruct<vec4>(L);
    LuaStruct<mat3>(L);
    LuaStruct<AssetTexture>(L);
    LuaStruct<Color>(L);
    LuaStruct<CEntity>(L);
    LuaStruct<BBox>(L);

    // LuaStruct<Asset>(L);
}

void open_neko_api(lua_State *L) {
    PROFILE_FUNC();

    luaL_checkversion(L);

    lua_CFunction funcs[] = {
            open_mt_thread,
            open_mt_channel,

            SoundWrap::open_mt_sound,
            FontWrap::open_mt_font,
            SpriteWrap::open_mt_sprite,
            mt_bindata::open_mt_bindata,

    // open_mt_image,
    // open_mt_atlas_image,
    // open_mt_atlas,
    // open_mt_tilemap,

#ifdef NEKO_BOX2D
            open_mt_b2_fixture,
            open_mt_b2_body,
            open_mt_b2_world,
#endif
            open_mt_ui_container,
            open_mt_ui_style,
            open_mt_ui_ref,

            open_enum,
    };

    for (u32 i = 0; i < NEKO_ARR_SIZE(funcs); i++) {
        funcs[i](L);
    }

    luaL_requiref(L, "neko", open_neko, 1);

#if 0
    luaL_Reg inspector_reg[] = {{"inspect_shaders", wrap_inspect_shaders},

                                NEKO_LUA_INSPECT_REG(shaders),
                                NEKO_LUA_INSPECT_REG(textures),
                                NEKO_LUA_INSPECT_REG(vertex_buffers),
                                NEKO_LUA_INSPECT_REG(index_buffers),
                                NEKO_LUA_INSPECT_REG(frame_buffers),
                                // NEKO_LUA_INSPECT_REG(uniform_buffers),
                                // NEKO_LUA_INSPECT_REG(storage_buffers),
                                // NEKO_LUA_INSPECT_REG(uniforms),
                                // NEKO_LUA_INSPECT_REG(pipelines),
                                // NEKO_LUA_INSPECT_REG(renderpasses),
                                {NULL, NULL}};
    luaL_setfuncs(L, inspector_reg, 0);
#endif

    typeclosure(L);
    lua_setfield(L, -2, "ltype");

    createStructTables(L);

    open_ui(L);
    lua_setfield(L, -2, "ui");

    open_db(L);
    lua_setfield(L, -2, "db");

    open_math(L);
    lua_setfield(L, -2, "math");

    lua_pop(L, 1);

    preload_module(L);
    Neko::luabind::package_preload(L);
    package_preload_embed(L);

    openlib_Event(L);

    lua_register(L, "__neko_ls", __neko_ls);

    neko_w_init();
}
