require("gen_neko_api")

G_Game = require "lua/game"

is_server = false

inspect = require "lua/libs/inspect"

function neko_proxy_before_quit()
    if is_server then
        server_quit()
    else
        client_quit()
    end
end

function neko_proxy_start(arg)
    neko.__start(arg)

    -- inspect = load_libs_from_url("https://raw.gitmirror.com/kikito/inspect.lua/master/inspect.lua", "inspect.lua")

    pixel_font = neko.font_load("assets/fonts/fusion-pixel-12px-monospaced-zh_hans.ttf")

    game_db = luadb.open("lua/data.luadb")

    if arg[1] == "test" then
        UnitTest()
        -- neko.quit()
        goto init_end
    elseif arg[1] == "-s" then
        is_server = true
        GameServer = G_Game.newServer(GetIP(), 22122)
        LocalGame = GameServer
    else
        GameClient = G_Game.newClient(22122)
        LocalGame = GameClient
    end

    local function load_world()
        local b2 = neko.b2_world {
            gx = 0,
            gy = 0,
            meter = 16
        }
        local world = World()

        local tilemap = neko.tilemap_load "assets/map.ldtk"
        tilemap:make_collision(b2, "Collision", {1})

        for k, v in ipairs(tilemap:entities()) do
            local mt
            if v.id == "Skeleton" then
                mt = _G["Npc"]
            else
                mt = _G[v.id]
            end

            if mt ~= nil then
                local obj = world:add(mt(v.x, v.y, v.id))
                if v.id == "Player" then
                    player = obj
                end
                -- print(v.id, v.x, v.y)
            else
                print("no " .. v.id .. " class exists")
            end
        end
        return world, b2, tilemap
    end

    LocalGame.world, LocalGame.b2, LocalGame.tilemap = load_world()

    if is_server then
        server_init()
    else
        client_init()
    end

    ::init_end::
end

function neko_proxy_frame(dt)
    if is_server then
        server_frame(dt)
    else
        if GameClient ~= nil then
            client_frame(dt)
        end
    end
end
