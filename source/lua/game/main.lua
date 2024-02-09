inspect = require("inspect")
sandbox = require("sandbox")

local dump_func = require "common/dump"

local ECS = require "ecs"
local json = require "json"

c_gameobject = require("gameobject")

neko_app = {
    title = "sandbox",
    width = 1440,
    height = 880
}

neko_cvar = {
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

w:register("tiled_map", {ud})

w:register("fallingsand", {ud})

w:register("gfxt", {ud})

w:register("custom_sprite", {ud})

w:register("particle", {ud})

local gd = game_data

game_init = function()

    -- test_pack = neko_pack_construct("test_pack_handle", neko_file_path("gamedir/res.pack"))
    -- test_handle = neko_pack_assets_load(test_pack, ".\\fonts\\fusion-pixel.ttf")

    -- default_font = neko_fontcache_load(test_handle, 18.0)

    -- neko_fontcache_set_default_font(default_font)

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
            ud = neko_player_create(gd.test_witch_spr)
        }
    }

    eid2 = w:new{
        vector2 = {
            x = 20,
            y = 20
        },
        tiled_map = {
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

    eid6 = w:new{
        vector2 = {
            x = 100,
            y = 100
        },
        particle = {
            ud = neko_particle_create()
        }
    }

    local safefunc = sandbox.protect([[
        -- test_audio_src = neko_audio_load(neko_file_path("data/assets/audio/test.mp3"))
        -- test_audio_ins = neko_audio_instance(test_audio_src, 0.2)
        -- neko_audio_play(test_audio_ins)
        -- log_info("sandbox here?")
    ]])

    safefunc()
end

game_shutdown = function()
    -- neko_pack_destroy(test_pack)

    neko_sprite_end(gd.test_witch_spr)

    for v2, t in w:match("all", "vector2", "tiled_map") do
        neko_tiled_end(t.ud)
    end

    for v2, t in w:match("all", "vector2", "custom_sprite") do
        neko_custom_sprite_end(t.ud)
    end

    for v2, t in w:match("all", "vector2", "fallingsand") do
        neko_fallingsand_end(t.ud)
    end

    for v2, t in w:match("all", "vector2", "gfxt") do
        neko_gfxt_end(t.ud)
    end

    for v2, t in w:match("all", "vector2", "particle") do
        neko_particle_end(t.ud)
    end
end

__NEKO_CONFIG_TYPE_INT = 0
__NEKO_CONFIG_TYPE_FLOAT = 1
__NEKO_CONFIG_TYPE_STRING = 2

gd.tick = 0

game_update = function()

    for obj, v4, v, p in w:match("all", "gameobj", "vector2", "velocity2", "player") do

        gd.tick = gd.tick + 1

        -- if gd.tick == 256 then
        --     gd.tick = 0
        -- end

        if (gd.tick & 31) == 0 then
            if math.abs(v.dx) >= 0.5 or math.abs(v.dy) >= 0.5 then
                neko_player_update_animation(p.ud, "Run");
            else
                neko_player_update_animation(p.ud, "Idle");
            end
        end

        -- if ((tick++ & 31) == 0)
        --     if (fabs(velocity->dx) >= 0.5 || fabs(velocity->dy) >= 0.5)
        --         neko_sprite_renderer_play(sprite, "Run");
        --     else
        --         neko_sprite_renderer_play(sprite, "Idle");

        neko_player_update(p.ud)

        local player_v = 3.1

        if neko_key_pressed("NEKO_KEYCODE_T") then
            -- print(c_gameobject.CGameObject_get_id(obj.sd))
            neko_gameobject_inspect(obj.sd)
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

        v4.x = v4.x + v.dx
        v4.y = v4.y + v.dy

        v.dx = v.dx / 2.0
        v.dy = v.dy / 2.0
    end
end

game_render = function()
    for v4, v, p in w:match("all", "vector2", "velocity2", "player") do
        neko_player_render(p.ud, v4.x, v4.y)
    end

    for v2, t in w:match("all", "vector2", "tiled_map") do
        neko_tiled_update(t.ud)
    end

    for v2, t in w:match("all", "vector2", "custom_sprite") do
        neko_custom_sprite_render(t.ud)
    end

    for v2, t in w:match("all", "vector2", "fallingsand") do
        neko_fallingsand_update(t.ud)
    end

    for v2, t in w:match("all", "vector2", "gfxt") do
        neko_gfxt_update(t.ud)
    end

    for v2, t in w:match("all", "vector2", "particle") do
        neko_particle_render(t.ud, v2.x, v2.y)
    end

    neko_draw_text(50.0, 50.0, "中文渲染测试 日本語レンダリングテスト Hello World! ")

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
        neko_dolua("lua_scripts/test.lua")
        -- neko_dolua("lua_scripts/test_cstruct.lua")

        -- print(dump_func(w))
    end

    win_w, win_h = neko_window_size(neko_main_window())

    -- neko_text("NekoEngine 内部测试版本", default_font, 20, win_h - 20)

    x, y = neko_mouse_delta()
end

