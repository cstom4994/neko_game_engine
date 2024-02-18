local dump_func = require "common/dump"

local ECS = require "common/ecs"
local json = require "json"
local dbg = require("libs.debugger")

gui = require "neko_gui"
-- nuklear = require 'nuklear'

require("common/hotfix")

local hot_code = neko_load("test_hot.lua")

neko_web_console = require "web_console"

c_gameobject = require("gameobject")

neko_game = {}

neko_game.app = {
    title = "sandbox",
    width = 1440,
    height = 880
}

neko_game.cvar = {
    show_demo_window = false
}

game_data = {}

local w = ECS.fetch_world("sandbox")

w:register("gameobj", {
    name = "default name",
    sd
})

w:register("vector2", {
    x = 0,
    y = 0
})

w:register("vector3", {
    x = 0,
    y = 0,
    z = 0
})

w:register("velocity2", {
    x = 0,
    y = 0
})

w:register("player", {ud})

w:register("tiled_map", {path, ud})

w:register("fallingsand", {ud})

w:register("gfxt", {ud})

w:register("custom_sprite", {ud})

w:register("particle", {ud})

local gd = game_data

SAFE_UD = function(tb)
    if tb.ud ~= nil then
        return tb.ud
    else
        dbg()
    end
end

test_filewatch_callback = function(change, virtual_path)
    print(change, virtual_path)

    if change == "FILEWATCH_FILE_MODIFIED" then

        for t in w:match("all", "tiled_map") do
            if virtual_path == t.path then
                neko_tiled_unload(t.ud)
                neko_tiled_load(t.ud, neko_file_path("gamedir" .. t.path))
            end
        end

    end
end

game_init_thread = function()
    gd.assetsys = neko_assetsys_create()
    gd.filewatch = neko_filewatch_create(gd.assetsys)

    neko_filewatch_mount(gd.filewatch, neko_file_path("gamedir/maps"), "/maps");

    neko_filewatch_start(gd.filewatch, "/maps", "test_filewatch_callback")
end

game_init = function()

    -- test_pack = neko_pack_construct("test_pack_handle", neko_file_path("gamedir/res.pack"))
    -- test_handle = neko_pack_assets_load(test_pack, ".\\fonts\\fusion-pixel.ttf")

    -- default_font = neko_fontcache_load(test_handle, 18.0)

    -- neko_fontcache_set_default_font(default_font)

    print(hot_code)
    for k, v in pairs(hot_code) do
        print(k, v)
    end

    gd.test_witch_spr = neko_sprite_create(neko_file_path("gamedir/assets/textures/B_witch.ase"));

    eid1 = w:new{
        gameobj = {
            name = "player",
            sd = c_gameobject.new_obj(1001, true, true)
        },
        vector2 = {
            x = 20,
            y = 20
        },
        velocity2 = {
            dx = 0,
            dy = 0
        },
        player = {
            ud = neko_aseprite_create(gd.test_witch_spr)
        }
    }

    eid2 = w:new{
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
            ud = neko_tiled_create(neko_file_path("gamedir/maps/map.tmx"))
        }
    }

    -- eid3 = w:new{
    --     vector2 = {
    --         x = 20,
    --         y = 20
    --     },
    --     fallingsand = {
    --         ud = neko_fallingsand_create()
    --     }
    -- }

    -- eid4 = w:new{
    --     vector2 = {
    --         x = 20,
    --         y = 20
    --     },
    --     gfxt = {
    --         ud = neko_gfxt_create(neko_file_path("gamedir/assets/pipelines/simple.sf"),
    --             neko_file_path("gamedir/assets/meshes/Duck.gltf"), neko_file_path("gamedir/assets/textures/DuckCM.png"))
    --     }
    -- }

    -- eid5 = w:new{
    --     vector2 = {
    --         x = 20,
    --         y = 20
    --     },
    --     custom_sprite = {
    --         ud = neko_custom_sprite_create()
    --     }
    -- }

    -- eid6 = w:new{
    --     vector2 = {
    --         x = 100,
    --         y = 100
    --     },
    --     particle = {
    --         ud = neko_particle_create()
    --     }
    -- }

    -- local safefunc = sandbox.protect([[
    --     -- test_audio_src = neko_audio_load(neko_file_path("data/assets/audio/test.mp3"))
    --     -- test_audio_ins = neko_audio_instance(test_audio_src, 0.2)
    --     -- neko_audio_play(test_audio_ins)
    --     -- log_info("sandbox here?")
    -- ]])

    -- safefunc()
end

game_shutdown = function()
    -- neko_pack_destroy(test_pack)

    neko_sprite_end(gd.test_witch_spr)

    for v2, t in w:match("all", "vector2", "tiled_map") do
        neko_tiled_end(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "custom_sprite") do
        neko_custom_sprite_end(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "fallingsand") do
        neko_fallingsand_end(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "gfxt") do
        neko_gfxt_end(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "particle") do
        neko_particle_end(SAFE_UD(t))
    end

    neko_filewatch_stop(gd.filewatch, "/maps")

    neko_filewatch_destory(gd.filewatch)
    neko_assetsys_destory(gd.assetsys)
end

__NEKO_CONFIG_TYPE_INT = 0
__NEKO_CONFIG_TYPE_FLOAT = 1
__NEKO_CONFIG_TYPE_STRING = 2

gd.tick = 0

game_pre_update = function()
    gd.tick = gd.tick + 1

    if (gd.tick & 255) == 0 then
        neko_filewatch_update(gd.filewatch)
        neko_filewatch_notify(gd.filewatch)

        neko_hotload("test_hot.lua")
        -- hot_code.foo()
        hot_code.g_foo()
    end
end

game_update = function()

    for obj, v2, v, p in w:match("all", "gameobj", "vector2", "velocity2", "player") do

        -- if gd.tick == 256 then
        --     gd.tick = 0
        -- end

        if (gd.tick & 31) == 0 then
            if math.abs(v.dx) >= 0.6 or math.abs(v.dy) >= 0.6 then
                neko_aseprite_update_animation(p.ud, "Run");
            else
                neko_aseprite_update_animation(p.ud, "Idle");
            end
        end

        neko_aseprite_update(SAFE_UD(p))

        local player_v = 3.1

        if neko_key_pressed("NEKO_KEYCODE_T") then
            print(c_gameobject.CGameObject_get_id(obj.sd))
        end

        if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
            player_v = 5.1
        else
            player_v = 3.1
        end

        if neko_was_key_down("NEKO_KEYCODE_A") then
            v.dx = v.dx - player_v
        end
        if neko_was_key_down("NEKO_KEYCODE_D") then
            v.dx = v.dx + player_v
        end
        if neko_was_key_down("NEKO_KEYCODE_W") then
            v.dy = v.dy - player_v
        end
        if neko_was_key_down("NEKO_KEYCODE_S") then
            v.dy = v.dy + player_v
        end

        v2.x = v2.x + v.dx
        v2.y = v2.y + v.dy

        v.dx = v.dx / 2.0
        v.dy = v.dy / 2.0
    end

    for v2, v, t in w:match("all", "vector2", "velocity2", "tiled_map") do

        local player_v = 2.1

        if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
            player_v = 5.1
        else
            player_v = 2.1
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
    for v2, v, p in w:match("all", "vector2", "velocity2", "player") do
        neko_aseprite_render(SAFE_UD(p), v2)
    end

    for v2, t in w:match("all", "vector2", "tiled_map") do
        neko_tiled_render(SAFE_UD(t), v2)
    end

    for v2, t in w:match("all", "vector2", "custom_sprite") do
        neko_custom_sprite_render(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "fallingsand") do
        neko_fallingsand_update(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "gfxt") do
        neko_gfxt_update(SAFE_UD(t))
    end

    for v2, t in w:match("all", "vector2", "particle") do
        neko_particle_render(SAFE_UD(t), v2)
    end

    for obj in w:match("all", "gameobj") do
        neko_gameobject_inspect(obj.sd)
    end

    neko_draw_text(50.0, 50.0, "中文渲染测试 日本語レンダリングテスト Hello World! ", 3.0)

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

    if neko_key_pressed("NEKO_KEYCODE_F3") then
        if neko_cvar("test_lua_newnew_f") ~= nil then
            neko_cvar_set("test_lua_newnew_f", 999.9)
        end

        print(neko_rand())
    end

    if neko_key_pressed("NEKO_KEYCODE_F2") then
        -- neko_dolua("lua_scripts/test_map.lua")
        -- neko_dolua("lua_scripts/test_datalist.lua")
        -- neko_dolua("lua_scripts/test.lua")
        -- neko_dolua("lua_scripts/test_cstruct.lua")
        -- neko_dolua("lua_scripts/test_class.lua")
        -- neko_dolua("lua_scripts/test_ds.lua")

        -- dbg()

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

