imgui = require("imgui")
CObject = require("cobject")
CVar = neko.cvar
ECS = require("common/ecs")
tweens = require("common/tweens")

-- fake game proxy
local fake_game = {
    sub_init_thread = function()

    end,
    sub_init = function()

    end,
    sub_shutdown = function()

    end,
    sub_pre_update = function()

    end,
    sub_update = function(dt)

    end,
    sub_render = function()

    end,
    test_update = function()

    end
}

neko_game = {
    app = {
        title = "sandbox",
        width = 1280,
        height = 720
    },
    cvar = {
        show_demo_window = false,
        show_physics_debug = false,

        -- experimental features 实验性功能
        enable_hotload = true
    }
}

local play = fake_game
local running = true

local text = imgui.StringBuf("12344")

game_init_thread = function()
    play.sub_init_thread()
end

game_init = function()
    test_ase_witch = neko.aseprite.create(neko_file_path("gamedir/assets/textures/B_witch.ase"))
    test_ase_harvester = neko.aseprite.create(neko_file_path("gamedir/assets/textures/TheHarvester.ase"))
    test_ase_pdx = neko.aseprite.create(neko_file_path("gamedir/assets/textures/pdx.ase"))

    play.sub_init()
end

game_shutdown = function()
    play.sub_shutdown()
end

game_pre_update = function()
    if running then
        play.sub_pre_update()
    end
end

game_update = function(dt)
    if running then
        play.sub_update(dt)
        play.test_update()
    end
end

game_render = function()
    imgui.Begin("Demo")
    imgui.Text("选择测试Demo")
    if imgui.Button("Sandbox") then
        running = false
        play.sub_shutdown()
        collectgarbage()
        play = require("game_sandbox")
        play.sub_init()
        play.sub_init_thread()
    end
    if imgui.Button("Run") then
        running = true
    end
    imgui.Separator()
    if imgui.InputText("TEST", text) then
        print(tostring(text))
    end
    imgui.End()

    if running then
        play.sub_render()
    end
end

