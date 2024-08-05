require("gen_neko_api")

G_Game = require "lua/game"

is_server = false

score = 0
game_tick = 0

local debug_on = false

function client_init()

    

    -- music = neko.sound_load "assets/audio/placeDungeonGlassland_Field.wav"
    -- music:set_loop(true)
    -- music:set_vol(0.25)
    -- music:start()

    local function load_assets()
        font = neko.default_font()

        sound_bow_1 = neko.sound_load "assets/audio/attackBow01.wav"
        sound_bow_2 = neko.sound_load "assets/audio/attackBow02.wav"
        sound_bow_3 = neko.sound_load "assets/audio/attackBowBig.wav"
        sound_hitbow_1 = neko.sound_load "assets/audio/hitBow01.wav"
        sound_hitbow_2 = neko.sound_load "assets/audio/hitBow02.wav"
        sound_hitbow_3 = neko.sound_load "assets/audio/hitBow03.wav"
        -- sound_hitbow_4 = neko.sound_load "assets/audio/hitBow04.wav"

        -- shader_1 = neko_api.new_shader({
        --     uniforms = {
        --         screen_size = "vec2"
        --     },
        --     vert = [[
        --         // draw rect sends x, y positions and w/h, 4 floats
        --         in vec4 coords;
        --         out vec2 image_uv;
        --         void vert_main() {
        --             gl_Position = vec4(coords.xy, 0.0, 1.0);
        --             image_uv = coords.zw;
        --         }
        --     ]],
        --     frag = [[
        --         in vec2 image_uv;
        --         out vec4 frag_color;
        --         void frag_main() {
        --             // default fragment shader looks something like the following
        --             //frag_color = texture(current_image, image_uv) * current_color;
        --             // we'll set blue to depend on the screen X of the pixel
        --             frag_color.b = gl_FragCoord.x / screen_size.x;
        --             // and red will depend on the texture X and Y of the pixel
        --             frag_color.r = image_uv.x * image_uv.y * 2.0;
        --             // and green to be the reverse of blue
        --             frag_color.g = (1.0 - frag_color.b);
        --             frag_color.a = 0.9;

        --         }
        --     ]]
        -- })

        cursor = Cursor(neko.image_load "assets/cursor.ase")
        neko.show_mouse(true)

        bow_img = neko.image_load "assets/bow.png"
        arrow_img = neko.image_load "assets/arrow.png"

        draw_fixtures = false
        neko.clear_color(48, 32, 32, 255)

        -- test ecs
        ecs = ECS()

        for i = 1, 100 do
            ecs:add{
                pos = vec2(neko.window_width() / 2, neko.window_height() / 2),
                vel = vec2(random(-100, 100), random(-100, 100)),
                scale = 0.25,
                z_index = -1,
                img = neko.image_load "assets/player_a.png"
            }
        end

        for i = 1, 10 do
            local id = ecs:add{
                pos = vec2(random(0, neko.window_width()), random(0, neko.window_height())),
                rot = {
                    angle = random(0, math.pi),
                    delta = random(-1, 1)
                },
                scale = 1,
                z_index = 0,
                img = neko.image_load "assets/player_a.png"
            }

            ecs:add{
                pos = vec2(random(0, neko.window_width()), random(0, neko.window_height())),
                follow = id,
                scale = 0.5,
                z_index = -10,
                img = neko.image_load "assets/player_a.png"
            }
        end

        -- test lui
        init_lui()

        debug_on = cvar("conf_debug_on")
    end

    load_assets()

    if not is_server then

        camera = Camera {
            x = player.x,
            y = player.y,
            scale = 5
        }
    end
end

function client_frame(dt)

    GameClient:update(dt)

    game_tick = game_tick + 1

    if neko.platform() ~= "web" and neko.key_down "esc" then
        neko.quit()
    end

    if neko.key_press "tab" then
        draw_fixtures = not draw_fixtures
    end

    if neko.key_down "l" then
        for i = 1, 2, 1 do
            local v = {
                id = "Npc",
                x = 424,
                y = 160
            }
            local mt = _G[v.id]
            if mt ~= nil then
                local obj = LocalGame.world:add(mt(v.x, v.y))
                if v.id == "Player" then
                    player = obj
                end
                -- print(v.id, v.x, v.y)
            else
                print("no " .. v.id .. " class exists")
            end
        end
    end

    if game_tick % 400 == 1 then
        random_spawn_npc()
        print("生成怪物")
    end

    -- shader_1:set("screen_size", {neko.window_width(), neko.window_height()})

    LocalGame.b2:step(dt)
    LocalGame.world:update(dt)

    cursor:update(dt)

    -- FLECS.progress(dt)

    if not is_server then

        local blend = 1 - 0.85 ^ (dt * 40)
        camera.x = lerp(camera.x, player.x, blend)
        camera.y = lerp(camera.y, player.y, blend)
        camera.scale = lerp(camera.scale, 4, dt ^ 0.8)

        camera:begin_draw()
        LocalGame.tilemap:draw()
        LocalGame.world:draw()
        cursor:draw()

        if draw_fixtures then
            LocalGame.tilemap:draw_fixtures(LocalGame.b2, "Collision")
        end
        camera:end_draw()

    end



    if LocalGame.currentGameState == LocalGame.states.CLIENT_WAIT then
        pixel_font:draw(("搜索服务器中"), 40, 210, 28)
    end

    -- test_ecs(dt)

    pixel_font:draw(("Hello, 世界23331!\nfps: %.2f (%.4f) tick: %d\nent: %d"):format(1 / dt, dt * 1000, game_tick,
        #LocalGame.world.by_id), 40, 80, 12)

    pixel_font:draw(("分数: %d\n血量: %d"):format(score, player.hp), 40, 150, 28)

    if player.hp <= 0 then
        pixel_font:draw("你死啦", neko.window_width() / 2 - 100, neko.window_height() / 2 - 40, 80)
    end

    -- test_lui()

end

function client_quit()
    if GameClient and GameClient.client then
        GameClient.client:disconnectNow()
    end

    -- test_pack:assets_unload(test_handle)
    -- test_pack:fini()
end

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

function game_proxy_before_quit()
    if is_server then
        server_quit()
    else
        client_quit()
    end
end

function game_proxy_start(arg)
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

function game_proxy_frame(dt)
    if is_server then
        server_frame(dt)
    else
        if GameClient ~= nil then
            client_frame(dt)
        end
    end
end
