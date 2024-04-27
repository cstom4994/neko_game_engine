ImGui = require("imgui")
CObject = require("cobject")
CVar = neko.cvar
ECS = require("common/ecs")

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

game_init_thread = function()
    play.sub_init_thread()
end

game_init = function()

    luainspector = __neko_luainspector_init()

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

    if neko_key_pressed("NEKO_KEYCODE_F4") then
        -- neko_dolua("lua_scripts/test_datalist.lua")
        -- neko_dolua("lua_scripts/tests/test_cstruct.lua")
        -- neko_dolua("lua_scripts/tests/test_class.lua")
        neko_dolua("lua_scripts/tests/test_ds.lua")
        -- neko_dolua("lua_scripts/tests/test_events.lua")
        -- neko_dolua("lua_scripts/tests/test_common.lua")
        -- neko_dolua("lua_scripts/tests/test_behavior.lua")
        -- neko_dolua("lua_scripts/tests/cffi/t.lua")
        -- neko_dolua("lua_scripts/tests/nekolua_1.lua")

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

end

game_render = function()

    ImGui.Begin("Demo")
    ImGui.Text("选择测试Demo")

    if ImGui.Button("Sandbox") then
        play.sub_shutdown()
        collectgarbage()
        play = require("demos/sandbox")
        play.sub_init()
        play.sub_init_thread()
    end

    if ImGui.Button("test_draw") then
        play.sub_shutdown()
        collectgarbage()
        play = require("demos/test_draw")
        play.sub_init()
        play.sub_init_thread()
    end

    if ImGui.Button("TTT") then
        local file_content = read_file("D:/Projects/Neko/Dev/source/lua/game/test_prefab.neko")
        if file_content then
            local dl = require "neko_lua_datalist.core"
            local this_map_node = dl.parse(file_content)
            print(dump_func(this_map_node))

            print(this_map_node[1]["engine_version"])
        else
            print("无法打开文件或文件不存在")
        end

        -- local encoded = neko.base64_encode(file_content)
        -- print("Encoded:", encoded)

        -- local decoded = neko.base64_decode(encoded)
        -- print("Decoded:", decoded)

    end

    ImGui.Separator()
    -- if ImGui.InputText("TEST", text) then
    --     print(tostring(text))
    -- end

    -- ImGui.Image(test_tex, 100.0, 100.0)
    -- ImGui.Image(test_custom_sprite.tex, 100.0, 100.0)
    ImGui.End()

    __neko_luainspector_draw(__neko_luainspector_get())

    if running then
        play.sub_render()
    end

    neko.hooks.run()

end

