-- local test_pack, test_handle, test_items
score = 0

local debug_on = false

function server_init()

    luainspector = Inspector.inspector_init()

    game_db = luadb.open("lua/data.luadb")

    local function load_assets()
        neko.show_mouse(true)

        draw_fixtures = false
        neko.clear_color(48, 32, 32, 255)

        debug_on = cvar("conf_debug_on")
    end

    load_assets()

end

function server_frame(dt)

    if neko.platform() ~= "web" and neko.key_down "esc" then
        neko.quit()
    end

    if neko.key_press "tab" then
        draw_fixtures = not draw_fixtures
    end

    if GameServer.game_tick % 2000 == 1 then
        random_spawn_npc()
        print("生成怪物")
    end

    -- shader_1:set("screen_size", {neko.window_width(), neko.window_height()})

    LocalGame.b2:step(dt)
    LocalGame.world:update(dt)

    -- FLECS.progress(dt)

    if debug_on then
        Inspector.inspector_draw(Inspector.inspector_get())

        draw_imgui(dt)
    end

    pixel_font:draw(("这是服务端 %s"):format(GameServer.enetServer:getAddress() .. ":" ..
                                                      GameServer.enetServer:getPort()), 40, 150, 28)

    -- for id, obj in pairs(LocalGame.world.by_id) do
    --     print(inspect({id, obj}))
    -- end

    GameServer:update(dt)

end

local quit = true
function server_quit()
    if quit then
        GameServer.currentGameState = GameServer.states.SERVER_DISCONNECT
        GameServer:sendGameState()
        quit = not quit
    else
        print("Thanks for playing. Please play again soon!")
        GameServer:destroy()
        return quit
    end
    return true
end
