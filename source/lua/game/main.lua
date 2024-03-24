local ECS = require("common/ecs")
local PHY = require("common/physics")
local deepcopy = require("common/nekolua/utils/deepcopy")

gui = require("neko_gui")
require("common/hotfix")
CObject = require("cobject")
imgui = require("imgui")

USE_LUA_PAK = false

if USE_LUA_PAK then
    tweens = require("source/lua/game/tweens")
    neko_web_console = require("source/lua/game/web_console")
else
    tweens = require("tweens")
    neko_web_console = require("web_console")
end

-- print(dump_func(gui))

neko_game = {
    app = {
        title = "sandbox",
        width = 1440,
        height = 880
    },
    cvar = {
        show_demo_window = false,
        show_physics_debug = false,

        -- experimental features 实验性功能
        enable_nekolua = false,
        enable_hotload = true
    }
}

game_data = {}

game_data.hot_code = {}

ecs_world = ECS.fetch_world("sandbox")

ecs_world:register("gameobj", {
    name = "default name",
    sd
})

ecs_world:register("vector2", {
    x = 0,
    y = 0
})

ecs_world:register("vector3", {
    x = 0,
    y = 0,
    z = 0
})

ecs_world:register("velocity2", {
    x = 0,
    y = 0
})

ecs_world:register("player", scale, {ud})

ecs_world:register("npc", scale, {r, ud})

ecs_world:register("tiled_map", {path, ud})

ecs_world:register("fallingsand", {ud})

ecs_world:register("gfxt", {ud})

ecs_world:register("custom_sprite", {ud})

ecs_world:register("particle", {ud})

local gd = game_data

SAFE_UD = function(tb)
    if tb ~= nil and tb.ud ~= nil then
        return tb.ud
    else
        return nil
    end
end

SAFE_SD = function(tb)
    if tb ~= nil and tb.sd ~= nil then
        return tb.sd
    else
        return nil
    end
end

phy_world = PHY.fetch_world(64)

local function phy_world_get_cellsize(world, cx, cy)
    local cellSize = phy_world.cellSize
    local l, t = phy_world:toWorld(cx, cy)
    return l, t, cellSize, cellSize
end

local function phy_debug_draw(world)
    local cellSize = phy_world.cellSize
    for cy, row in pairs(phy_world.rows) do
        for cx, cell in pairs(row) do
            local l, t, w, h = phy_world_get_cellsize(world, cx, cy)
            local intensity = (cell.itemCount * 12 + 16) / 256

            local v1 = to_vec2(l, t);
            local v2 = to_vec2(w, h);

            neko.idraw_text(l + w / 2, t + h / 2, cell.itemCount)

            neko.idraw_defaults()

            neko.idraw_rectv(v1, v2, "NEKO_GRAPHICS_PRIMITIVE_LINES")

        end
    end
end

local blocks = {}

local function phy_add_block(x, y, w, h, type)
    local block = {
        x = x,
        y = y,
        w = w,
        h = h,
        type = type
    }
    blocks[#blocks + 1] = block
    phy_world:add(block, x, y, w, h)
end

local function phy_del_block_by_type(type)
    for i, block in ipairs(blocks) do
        if block.type == type then
            blocks[i] = nil
            phy_world:remove(block)
        end
    end
end

local function draw_blocks()
    for _, block in ipairs(blocks) do

        local v1 = to_vec2(block.x, block.y)
        local v2 = to_vec2(block.w, block.h)

        neko.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES",
            to_color(0, 255, 255, 255))
    end
end

local function phy_world_tiled_load(tiled_ud)
    print("phy_world_tiled_load()")
    local object_groups = neko_tiled_get_objects(tiled_ud)
    -- print(dump_func(ttttt))
    for object_group_name, v in pairs(object_groups) do
        if object_group_name == "collisions" then
            for ii, vv in ipairs(v) do
                print(("%s %d"):format(object_group_name, ii), dump_func(vv))
                phy_add_block(vv[1], vv[2], vv[3], vv[4], "tiled")
            end
        end
    end
end

local function phy_world_tiled_unload(tiled_ud)
    print("phy_world_tiled_unload()")

    phy_del_block_by_type("tiled")

end

test_filewatch_callback = function(change, virtual_path)
    print(change, virtual_path)

    if change == "FILEWATCH_FILE_MODIFIED" then

        for t in ecs_world:match("all", "tiled_map") do
            if virtual_path == t.path then
                phy_world_tiled_unload(SAFE_UD(t))
                neko.tiled_unload(SAFE_UD(t))
                neko.tiled_load(SAFE_UD(t), neko_file_path("gamedir" .. t.path))
                phy_world_tiled_load(SAFE_UD(t))
            end
        end

    end
end

game_init_thread = function()
    gd.assetsys = neko.assetsys_create()
    gd.filewatch = neko.filewatch_create(gd.assetsys)

    neko.filewatch_mount(gd.filewatch, neko_file_path("gamedir/maps"), "/maps");

    neko.filewatch_start(gd.filewatch, "/maps", "test_filewatch_callback")

    if not neko_game.cvar.enable_nekolua then
        print("Not enable nekolua")
    else
        nekolua = require "common/nekolua/init"
        nekolua.register()
    end

    gd.hot_code["player_update.lua"] = neko_load("player_update.lua")
    gd.hot_code["npc.lua"] = neko_load("npc.lua")

    -- neko.pack_build(neko_file_path("gamedir/sc_build.pack"),
    --     {"source/lua/game/web_console.lua", "source/lua/game/tweens.lua"})

    -- neko.pack_build(neko_file_path("gamedir/res2.pack"),
    --     {"gamedir/assets/textures/cat.aseprite", "gamedir/assets/textures/map1.ase", "gamedir/1.fnt"})

    test_pack_1, test_pack_2, test_pack_3 = neko.pack_info(neko_file_path("gamedir/res2.pack"))

    print("pack_info", test_pack_1, test_pack_2, test_pack_3)

    test_pack = neko.pack_construct("test_pack_handle", neko_file_path("gamedir/res2.pack"))
    test_handle = neko.pack_assets_load(test_pack, "gamedir/assets/textures/cat.aseprite")
    test_items = neko.pack_items(test_pack)

    print(dump_func(test_items))

end

game_init = function()

    win_w, win_h = neko_window_size(neko_main_window())

    gd.fbo = neko.graphics_framebuffer_create()
    gd.rt = neko.graphics_texture_create(win_w, win_h)
    gd.rp = neko.graphics_renderpass_create(gd.fbo, gd.rt)

    gd.cam = to_vec2(0.0, 0.0)

    -- default_font = neko_fontcache_load(test_handle, 18.0)

    gd.test_witch_spr = neko.aseprite.create(neko_file_path("gamedir/assets/textures/B_witch.ase"))
    gd.test_harvester_spr = neko.aseprite.create(neko_file_path("gamedir/assets/textures/TheHarvester.ase"))
    gd.test_pdx_spr = neko.aseprite.create(neko_file_path("gamedir/assets/textures/pdx.ase"))

    gd.test_batch = neko.sprite_batch_create([[
#version 330

uniform mat4 u_mvp;
in vec2 in_pos; in vec2 in_uv;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    gl_Position = u_mvp * vec4(in_pos, 0, 1);
}
    ]], [[
#version 330

precision mediump float;
uniform sampler2D u_sprite_texture;
in vec2 v_uv; out vec4 out_col;

void main() { out_col = texture(u_sprite_texture, v_uv); }
    ]])

    eid1 = ecs_world:new{
        gameobj = {
            name = "player",
            w = 48,
            h = 48,
            sd = CObject.new_obj(1001, true, true)
        },
        vector2 = {
            x = 360,
            y = 420
        },
        velocity2 = {
            dx = 0,
            dy = 0
        },
        player = {
            scale = 3.0,
            ud = neko.aseprite_render.create(gd.test_witch_spr)
        }
    }

    local v2, v, player, player_obj = ecs_world:get(eid1, "vector2", "velocity2", "player", "gameobj")

    phy_world:add(player, v2.x, v2.y, player_obj.w * player.scale - 100, player_obj.h * player.scale - 40)

    -- phy_add_block(800 - 32, 120, 32, 600 - 32 * 2)

    eid2 = ecs_world:new{
        gameobj = {
            name = "tiled_map_1",
            sd = CObject.new_obj(1002, true, true)
        },
        vector2 = {
            x = 20,
            y = 20
        },
        velocity2 = {
            dx = 0,
            dy = 0
        },
        tiled_map = {
            path = "/maps/map.tmx",
            ud = neko.tiled_create(neko_file_path("gamedir/maps/map.tmx"), sprite_vs, sprite_fs)
        }
    }

    local tiled_v2, tiled_v, tiled, tiled_obj = ecs_world:get(eid2, "vector2", "velocity2", "tiled_map", "gameobj")

    phy_world_tiled_load(SAFE_UD(tiled))

    test_audio = neko.audio_load("C:/Users/kaoruxun/Projects/Neko/dataWak/audio/otoha.wav")

    -- eid3 = ecs_world:new{
    --     vector2 = {
    --         x = 20,
    --         y = 20
    --     },
    --     fallingsand = {
    --         ud = neko.fallingsand_create()
    --     }
    -- }

    -- eid4 = ecs_world:new{
    --     vector2 = {
    --         x = 20,
    --         y = 20
    --     },
    --     gfxt = {
    --         ud = neko.gfxt_create(neko_file_path("gamedir/assets/pipelines/simple.sf"),
    --             neko_file_path("gamedir/assets/meshes/Duck.gltf"), neko_file_path("gamedir/assets/textures/DuckCM.png"))
    --     }
    -- }

    -- eid5 = ecs_world:new{
    --     vector2 = {
    --         x = 20,
    --         y = 20
    --     },
    --     custom_sprite = {
    --         ud = neko.custom_sprite_create()
    --     }
    -- }

    -- eid6 = ecs_world:new{
    --     vector2 = {
    --         x = 100,
    --         y = 100
    --     },
    --     particle = {
    --         ud = neko.particle_create()
    --     }
    -- }

    -- local safefunc = sandbox.protect([[
    --     -- test_audio_src = neko.audio_load(neko_file_path("data/assets/audio/test.mp3"))
    --     -- test_audio_ins = neko.audio_instance(test_audio_src, 0.2)
    --     -- neko.audio_play(test_audio_ins)
    --     -- log_info("sandbox here?")
    -- ]])

    -- safefunc()

    for i = 1, 10, 1 do
        local e = ecs_world:new{
            gameobj = {
                name = "npc_pdx_" .. i,
                w = 48,
                h = 48,
                sd = CObject.new_obj(1010 + i, true, true)
            },
            vector2 = {
                x = common.random(1, 1800),
                y = common.random(1, 1800)
            },
            velocity2 = {
                dx = 0,
                dy = 0
            },
            npc = {
                r = -1,
                scale = 3.0,
                ud = neko.aseprite_render.create(gd.test_pdx_spr)
            }
        }

        local v2, v, player, player_obj = ecs_world:get(e, "vector2", "velocity2", "npc", "gameobj")

        phy_world:add(player, v2.x, v2.y, player_obj.w * player.scale - 100, player_obj.h * player.scale - 40)
    end

    for i = 1, 5, 1 do
        local e = ecs_world:new{
            gameobj = {
                name = "npc" .. i,
                w = 48,
                h = 48,
                sd = CObject.new_obj(1010 + i, true, true)
            },
            vector2 = {
                x = common.random(1, 1800),
                y = common.random(1, 1800)
            },
            velocity2 = {
                dx = 0,
                dy = 0
            },
            npc = {
                r = -1,
                scale = 4.0,
                ud = neko.aseprite_render.create(gd.test_harvester_spr)
            }
        }

        local v2, v, player, player_obj = ecs_world:get(e, "vector2", "velocity2", "npc", "gameobj")

        phy_world:add(player, v2.x, v2.y, player_obj.w * player.scale - 100, player_obj.h * player.scale - 40)
    end
end

game_shutdown = function()
    neko.pack_assets_unload(test_pack, test_handle)
    neko.pack_destroy(test_pack)

    -- neko.sprite_end(gd.test_witch_spr)

    for v2, t in ecs_world:match("all", "vector2", "tiled_map") do
        neko.tiled_end(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "custom_sprite") do
        neko.custom_sprite_end(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "fallingsand") do
        neko.fallingsand_end(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "gfxt") do
        neko.gfxt_end(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "particle") do
        neko.particle_end(SAFE_UD(t))
    end

    neko.filewatch_stop(gd.filewatch, "/maps")

    neko.filewatch_destory(gd.filewatch)
    neko.assetsys_destory(gd.assetsys)

    neko.audio_unload(test_audio)

    neko.graphics_renderpass_destroy(gd.rp)
    neko.graphics_texture_destroy(gd.rt)
    neko.graphics_framebuffer_destroy(gd.fbo)

    neko.sprite_batch_end(gd.test_batch)
end

gd.tick = 0

game_pre_update = function()
    gd.tick = gd.tick + 1

    if neko_game.cvar.enable_hotload then
        if (gd.tick & 255) == 0 then
            neko.filewatch_update(gd.filewatch)
            neko.filewatch_notify(gd.filewatch)

            for k, v in pairs(gd.hot_code) do
                neko_hotload(k)
            end
        end
    end

end

local max_dt = 0.0

game_update = function(dt)

    if dt > max_dt then
        max_dt = dt
        print("max_dt", max_dt)
    end

    tweens.update(dt)

    for k, v in pairs(gd.hot_code) do
        v.my_update(dt)
    end

    gd.mx, gd.my = neko_mouse_position()

    for v2, v, t in ecs_world:match("all", "vector2", "velocity2", "tiled_map") do

        local player_v

        if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
            player_v = 250.1 * dt
        else
            player_v = 220.1 * dt
        end

        if neko_was_key_down("NEKO_KEYCODE_LEFT") then
            v.dx = v.dx - player_v
        end
        if neko_was_key_down("NEKO_KEYCODE_RIGHT") then
            v.dx = v.dx + player_v
        end
        if neko_was_key_down("NEKO_KEYCODE_UP") then
            v.dy = v.dy - player_v
        end
        if neko_was_key_down("NEKO_KEYCODE_DOWN") then
            v.dy = v.dy + player_v
        end

        gd.cam.x = gd.cam.x + v.dx
        gd.cam.y = gd.cam.y + v.dy

        -- v2.x = v2.x - v.dx
        -- v2.y = v2.y - v.dy

        v.dx = v.dx / 2.0
        v.dy = v.dy / 2.0
    end

    ecs_world:update()

    neko_web_console.update()

end

local obj_select
local obj_view

game_render = function()

    fbs_x, fbs_y = neko_framebuffer_size()

    local t = neko_platform_elapsed_time()

    neko.idraw_defaults()
    -- neko.idraw_camera2d(fbs_x, fbs_y)
    neko.idraw_camera2d_ex(gd.cam.x, fbs_x + gd.cam.x, gd.cam.y, fbs_y + gd.cam.y)

    -- neko.idraw_defaults()
    neko.idraw_rectv(to_vec2(0, 0), to_vec2(50, 50), "NEKO_GRAPHICS_PRIMITIVE_LINES")

    for v2, t, obj in ecs_world:match("all", "vector2", "tiled_map", "gameobj") do
        if CObject.CGameObject_get_active(SAFE_SD(obj)) then
            neko.tiled_render(SAFE_UD(t), 0, to_vec2(0, 0), gd.cam.x, fbs_x + gd.cam.x, gd.cam.y, fbs_y + gd.cam.y)

            -- neko.idraw_texture(gd.rt)
            -- neko.idraw_rectvd(v2, to_vec2(64 * 16 * 2, 32 * 16 * 2), to_vec2(0.0, 1.0), to_vec2(1.0, 0.0),
            --     "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES")

            neko.idraw_defaults()
            neko.idraw_text(v2.x, v2.y - 10, ("tiled_map x:%f y:%f"):format(v2.x, v2.y))
            neko.idraw_defaults()
        end
    end

    for v2, v, p, obj in ecs_world:match("all", "vector2", "velocity2", "npc", "gameobj") do
        local direction = 0
        if v.dx < 0 then
            direction = 1
        end

        player_pos = v2

        local render_ase = deepcopy(v2)
        render_ase.x = render_ase.x - 50
        render_ase.y = render_ase.y - 20

        SAFE_UD(p):render(render_ase, direction, p.scale)

        if neko_game.cvar.show_physics_debug then
            neko.idraw_rectv(render_ase, {
                x = obj.w * p.scale,
                y = obj.h * p.scale
            }, "NEKO_GRAPHICS_PRIMITIVE_LINES", to_color(0, 144, 144, 255))

            neko.idraw_rectv(v2, {
                x = 10,
                y = 10
            }, "NEKO_GRAPHICS_PRIMITIVE_LINES", to_color(0, 144, 144, 255))
        end
    end

    local player_pos

    for v2, v, p, obj in ecs_world:match("all", "vector2", "velocity2", "player", "gameobj") do
        local direction = 0
        if v.dx < 0 then
            direction = 1
        end

        player_pos = v2

        local render_ase = deepcopy(v2)
        render_ase.x = render_ase.x - 50
        render_ase.y = render_ase.y - 20

        SAFE_UD(p):render(render_ase, direction, p.scale)

        if neko_game.cvar.show_physics_debug then
            neko.idraw_rectv(render_ase, {
                x = obj.w * p.scale,
                y = obj.h * p.scale
            }, "NEKO_GRAPHICS_PRIMITIVE_LINES", to_color(255, 144, 144, 255))

            neko.idraw_rectv(v2, {
                x = 10,
                y = 10
            }, "NEKO_GRAPHICS_PRIMITIVE_LINES", to_color(255, 144, 144, 255))
        end
    end

    local cc_x = player_pos.x - win_w / 2 + 24 * 3
    local cc_y = player_pos.y - win_h / 2 + 24 * 3
    tweens.to(gd.cam, 1.5, {
        x = cc_x,
        y = cc_y
    }):ease("cubicout")

    neko.idraw_rectv(to_vec2(gd.mx + gd.cam.x, gd.my + gd.cam.y), {
        x = 10,
        y = 10
    }, "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES", to_color(255, 0, 0, 255))

    for v2, t in ecs_world:match("all", "vector2", "custom_sprite") do
        neko.custom_sprite_render(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "fallingsand") do
        neko.fallingsand_update(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "gfxt") do
        neko.gfxt_update(SAFE_UD(t))
    end

    for v2, t in ecs_world:match("all", "vector2", "particle") do
        neko.particle_render(SAFE_UD(t), v2)
    end

    imgui.Begin("Hello")
    obj_select = obj_select or '' -- eid从0开始
    imgui.PushID("TestCombo")
    if imgui.BeginCombo("Type", obj_select) then
        for obj in ecs_world:match("all", "gameobj") do
            if imgui.SelectableEx(obj.name, obj_select == obj.name) then
                obj_select = obj.name
                obj_view = obj
            end
        end
        imgui.EndCombo()
    end
    imgui.PopID()
    if obj_view ~= nil then
        local v2, v, player_obj = ecs_world:get(obj_view.__eid, "vector2", "velocity2", "gameobj")
        neko.gameobject_inspect(SAFE_SD(obj_view))
        imgui.Text("%d %s", obj_view.__eid, obj_view.name)
        imgui.Text("坐标: %f %f", v2.x, v2.y)
        imgui.Text("速度: %f %f (%f)", v.dx, v.dy, math.abs(math.sqrt(v.dx ^ 2 + v.dy ^ 2)))
        -- ECS.select(ecs_world, function(c)
        --     local s = {c.x}
        --     imgui.DragFloat("啊？", s)
        --     c.x = s[1]
        -- end, "vector2")
        -- imgui.Text("%d", n)
        -- for i = 1, n, 1 do
        --     imgui.DragFloat("啊？", {list[i].x})
        -- end
    end
    imgui.End()

    -- neko.draw_text(50.0, 50.0, "中文渲染测试 日本語レンダリングテスト Hello World! ", 3.0)

    if neko_game.cvar.show_physics_debug then
        phy_debug_draw()
        draw_blocks()
    end

    neko.graphics_renderpass_begin(0)
    neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
    neko.idraw_draw()
    neko.graphics_renderpass_end()

    -- neko.idraw_defaults()
    -- neko.idraw_camera3d(fbs_x, fbs_y)
    -- neko.idraw_translatef(0.0, 0.0, -2.0)
    -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 1.0, 0.0, 0.0)
    -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 0.0, 1.0, 0.0)
    -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 0.0, 0.0, 1.0)
    -- neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 200, 100, 50, 255, "NEKO_GRAPHICS_PRIMITIVE_LINES")

    -- neko.graphics_renderpass_begin(gd.rp)
    -- neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
    -- neko.graphics_clear(0.0, 0.0, 0.0, 1.0)
    -- neko.idraw_draw()
    -- neko.graphics_renderpass_end()

    -- neko.idraw_camera3d(fbs_x, fbs_y)
    -- neko.idraw_depth_enabled(true)
    -- neko.idraw_face_cull_enabled(true)
    -- neko.idraw_translatef(0.0, 0.0, -1.0)
    -- neko.idraw_texture(gd.rt)
    -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 1.0, 0.0, 0.0)
    -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0002, 0.0, 1.0, 0.0)
    -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0003, 0.0, 0.0, 1.0)
    -- neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 255, 255, 255, 255, "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES")

    -- neko.graphics_renderpass_begin(0)
    -- neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
    -- neko.idraw_draw()
    -- neko.graphics_renderpass_end()

    neko.sprite_batch_render_ortho(gd.test_batch, fbs_x, fbs_y, 0, 0)
    neko.sprite_batch_render_begin(gd.test_batch)

    local dragon_zombie = neko.sprite_batch_make_sprite(gd.test_batch, 0, 650, 500, 1, common.rad2deg(t / 100000.0), 0)
    local night_spirit = neko.sprite_batch_make_sprite(gd.test_batch, 1, 50, 250, 1, 0, 0)
    neko.sprite_batch_push_sprite(gd.test_batch, dragon_zombie)
    neko.sprite_batch_push_sprite(gd.test_batch, night_spirit)

    for i = 0, 4 do
        local polish = neko.sprite_batch_make_sprite(gd.test_batch, 1, 200, 880, 1, common.rad2deg(t / 50000.0), 0)
        local translated = polish
        local polish_x = CObject.getter("neko_sprite_t", "x")(polish)
        local polish_y = CObject.getter("neko_sprite_t", "y")(polish)
        local polish_sx = CObject.getter("neko_sprite_t", "sx")(polish)
        local polish_sy = CObject.getter("neko_sprite_t", "sy")(polish)
        for j = 0, 6 do
            CObject.setter("neko_sprite_t", "x")(translated, polish_x + polish_sx * i)
            CObject.setter("neko_sprite_t", "y")(translated, polish_y + polish_sy * j)
            neko.sprite_batch_push_sprite(gd.test_batch, translated)
        end
    end

    neko.sprite_batch_render_end(gd.test_batch)

end

test_update = function()
    if neko_key_pressed("NEKO_KEYCODE_F4") then

        print(neko_cvar("test_cvar"))
        print(neko_cvar("test_cvar_str"))

        if neko_cvar("test_lua_newnew_f") == nil then
            neko_cvar_new("test_lua_newnew_f", __NEKO_CONFIG_TYPE_FLOAT, 5.44)
        end

        print(neko_cvar("test_lua_newnew_f"))
    end

    if neko_key_pressed("NEKO_KEYCODE_T") then
        -- collectgarbage("collect")
        -- __neko_print_registry_list()
        ecs_world:dump()
    end

    if neko_key_pressed("NEKO_KEYCODE_F2") then
        neko_game.cvar.show_physics_debug = not neko_game.cvar.show_physics_debug
    end

    if neko_key_pressed("NEKO_KEYCODE_F3") then
        if neko_cvar("test_lua_newnew_f") ~= nil then
            neko_cvar_set("test_lua_newnew_f", 999.9)
        end

        print(neko_rand())
    end

    if neko_key_pressed("NEKO_KEYCODE_F4") then
        -- neko_dolua("lua_scripts/test_map.lua")
        -- neko_dolua("lua_scripts/test_datalist.lua")
        -- neko_dolua("lua_scripts/test.lua")
        -- neko_dolua("lua_scripts/test_cstruct.lua")
        -- neko_dolua("lua_scripts/test_class.lua")
        -- neko_dolua("lua_scripts/test_ds.lua")
        -- neko_dolua("lua_scripts/test_events.lua")
        -- neko_dolua("lua_scripts/test_nekolua.lua")
        -- neko_dolua("lua_scripts/test_common.lua")
        -- neko_dolua("lua_scripts/test_behavior.lua")
        -- neko_dolua("lua_scripts/tests/test_loader.lua")
        -- neko_dolua("lua_scripts/tests/cffi/t.lua")
        neko_dolua("lua_scripts/stronger/example.lua")

        -- neko.audio_play(test_audio)

        -- print(dump_func(w))

        -- neko_callback_save("callback1", function(ha)
        --     print("This is callback 1 " .. neko_hash(ha))
        -- end)
        -- neko_callback_save("callback2", function(ha, haa)
        --     print("This is callback 2 " .. ha .. haa)
        -- end)
        -- neko_callback_call("callback1", "haha1")
        -- neko_callback_call("callback2", "haha2", "haha3")
    end

    -- neko_text("NekoEngine 内部测试版本", default_font, 20, win_h - 20)

    x, y = neko_mouse_delta()
end

