local ECS = require "common/ecs"
local PHY = require "common/physics"

gui = require "neko_gui"

require("common/hotfix")

neko_web_console = require "web_console"

c_gameobject = require("gameobject")

neko_game = {}

neko_game.app = {
    title = "sandbox",
    width = 1440,
    height = 880
}

neko_game.cvar = {
    show_demo_window = false,
    show_physics_debug = false,

    -- experimental features 实验性功能
    enable_nekolua = false,
    enable_hotload = true
}

game_data = {}

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

ecs_world:register("tiled_map", {path, ud})

ecs_world:register("fallingsand", {ud})

ecs_world:register("gfxt", {ud})

ecs_world:register("custom_sprite", {ud})

ecs_world:register("particle", {ud})

local gd = game_data

SAFE_UD = function(tb)
    if tb.ud ~= nil then
        return tb.ud
    else
        dbg()
    end
end

SAFE_SD = function(tb)
    if tb.sd ~= nil then
        return tb.sd
    else
        dbg()
    end
end

test_filewatch_callback = function(change, virtual_path)
    print(change, virtual_path)

    if change == "FILEWATCH_FILE_MODIFIED" then

        for t in ecs_world:match("all", "tiled_map") do
            if virtual_path == t.path then
                neko.tiled_unload(SAFE_UD(t))
                neko.tiled_load(SAFE_UD(t), neko_file_path("gamedir" .. t.path))
            end
        end

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

local function phy_add_block(x, y, w, h)
    local block = {
        x = x,
        y = y,
        w = w,
        h = h
    }
    blocks[#blocks + 1] = block
    phy_world:add(block, x, y, w, h)
end

local function draw_blocks()
    for _, block in ipairs(blocks) do

        local v1 = to_vec2(block.x, block.y)
        local v2 = to_vec2(block.w, block.h)

        neko.idraw_rectvd(v1, v2, "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES")
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

    gd.hot_code = neko_load("player_update.lua")

    -- print(gd.hot_code)
    -- for k, v in pairs(gd.hot_code) do
    --     print(k, v)
    -- end

end

game_init = function()

    gd.fbo = neko.graphics_framebuffer_create()
    gd.rt = neko.graphics_texture_create()
    gd.rp = neko.graphics_renderpass_create(gd.fbo, gd.rt)

    -- test_pack = neko_pack_construct("test_pack_handle", neko_file_path("gamedir/res.pack"))
    -- test_handle = neko_pack_assets_load(test_pack, ".\\fonts\\fusion-pixel.ttf")

    -- default_font = neko_fontcache_load(test_handle, 18.0)

    -- neko_fontcache_set_default_font(default_font)

    gd.test_witch_spr = neko.sprite_create(neko_file_path("gamedir/assets/textures/B_witch.ase"));

    eid1 = ecs_world:new{
        gameobj = {
            name = "player",
            w = 48,
            h = 48,
            sd = c_gameobject.new_obj(1001, true, true)
        },
        vector2 = {
            x = 120,
            y = 120
        },
        velocity2 = {
            dx = 0,
            dy = 0
        },
        player = {
            scale = 3.0,
            ud = neko.aseprite.create(gd.test_witch_spr)
        }
    }

    local v2, v, player, player_obj = ecs_world:get(eid1, "vector2", "velocity2", "player", "gameobj")

    phy_world:add(player, v2.x, v2.y, player_obj.w * player.scale, player_obj.h * player.scale)

    phy_add_block(800 - 32, 120, 32, 600 - 32 * 2)

    eid2 = ecs_world:new{
        gameobj = {
            name = "tiled_map_1",
            sd = c_gameobject.new_obj(1002, true, true)
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

    print("=======================")
    local ttttt = neko_tiled_get_objects(SAFE_UD(tiled))
    print(dump_func(ttttt))

    for object_group_name, v in pairs(ttttt) do
        if object_group_name == "collisions" then
            for ii, vv in ipairs(v) do
                print(("%s %d"):format(object_group_name, ii), dump_func(vv))
                phy_add_block(vv[1], vv[2], vv[3], vv[4])
            end
        end
    end

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
end

game_shutdown = function()
    -- neko.pack_destroy(test_pack)

    neko.sprite_end(gd.test_witch_spr)

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
end

gd.tick = 0

game_pre_update = function()
    gd.tick = gd.tick + 1

    if neko_game.cvar.enable_hotload then
        if (gd.tick & 255) == 0 then
            neko.filewatch_update(gd.filewatch)
            neko.filewatch_notify(gd.filewatch)

            neko_hotload("player_update.lua")
        end
    end

end

local max_dt = 0.0

game_update = function(dt)

    if dt > max_dt then
        max_dt = dt
        print("max_dt", max_dt)
    end

    gd.hot_code.my_update(dt)

    for v2, v, t in ecs_world:match("all", "vector2", "velocity2", "tiled_map") do

        local player_v

        if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
            player_v = 50.1 * dt
        else
            player_v = 20.1 * dt
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

        v2.x = v2.x + v.dx
        v2.y = v2.y + v.dy

        v.dx = v.dx / 2.0
        v.dy = v.dy / 2.0
    end

    neko_web_console.update()

end

game_render = function()

    fbs_x, fbs_y = neko_framebuffer_size()

    neko.idraw_defaults()
    neko.idraw_camera2d(fbs_x, fbs_y)

    for v2, v, p, obj in ecs_world:match("all", "vector2", "velocity2", "player", "gameobj") do
        local direction = 0
        if v.dx < 0 then
            direction = 1
        end

        SAFE_UD(p):render(v2, direction, p.scale)

        neko.idraw_rectv(v2, {
            x = obj.w * p.scale,
            y = obj.h * p.scale
        }, "NEKO_GRAPHICS_PRIMITIVE_LINES")
    end

    for v2, t in ecs_world:match("all", "vector2", "tiled_map") do
        neko.tiled_render(SAFE_UD(t), v2)

        neko.idraw_defaults()
        neko.idraw_text(v2.x, v2.y - 10, ("tiled_map x:%f y:%f"):format(v2.x, v2.y))
        neko.idraw_defaults()
    end

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

    for obj in ecs_world:match("all", "gameobj") do
        neko.gameobject_inspect(SAFE_SD(obj))
    end

    neko.draw_text(50.0, 50.0, "中文渲染测试 日本語レンダリングテスト Hello World! ", 3.0)

    if neko_game.cvar.show_physics_debug then
        phy_debug_draw()
        draw_blocks()
    end

    -- local sliderFloat = {0.1, 0.5}
    -- local clearColor = {0.2, 0.2, 0.2}
    -- local comboSelection = 1
    -- local floatValue = 0

    -- imgui.Text("Hello, world!");
    -- clearColor[1], clearColor[2], clearColor[3] = imgui.ColorEdit3("Clear color", clearColor[1], clearColor[2],
    --     clearColor[3]);

    -- floatValue = imgui.SliderFloat("SliderFloat", floatValue, 0.0, 1.0);
    -- sliderFloat[1], sliderFloat[2] = imgui.SliderFloat2("SliderFloat2", sliderFloat[1], sliderFloat[2], 0.0, 1.0);

    -- comboSelection = imgui.Combo("Combo", comboSelection, "combo1, haha", 4);

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
        neko_dolua("lua_scripts/tests/test_loader.lua")

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

    win_w, win_h = neko_window_size(neko_main_window())

    -- neko_text("NekoEngine 内部测试版本", default_font, 20, win_h - 20)

    x, y = neko_mouse_delta()
end

