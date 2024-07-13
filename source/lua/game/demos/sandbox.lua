local PHY = require "bump"
local network = require "sock"
local tweens = require "flux"

local behavior = common.behavior

local selector = behavior.func_selector
local sequence = behavior.func_sequence
local repeatNode = behavior.func_repeat

local M = {}

local game_data = {}

game_data.hot_code = {}

local tiny = require "tiny"

b2_world = neko_b2_world {
    gx = 0,
    gy = 9.81,
    meter = 80
}

-- ecs_world = ECS.fetch_world("sandbox")

-- ecs_world:register("gameobj", {
--     name = "default name",
--     sd
-- })

-- ecs_world:register("vector2", {
--     x = 0,
--     y = 0
-- })

-- ecs_world:register("vector3", {
--     x = 0,
--     y = 0,
--     z = 0
-- })

-- ecs_world:register("velocity2", {
--     x = 0,
--     y = 0
-- })

-- ecs_world:register("player", scale, {ud})
-- ecs_world:register("npc", scale, {r, ud, npc_typename})
-- ecs_world:register("tiled_map", {path, ud})

-- ecs_world:register("fallingsand", {ud})
-- ecs_world:register("gfxt", {ud})
-- ecs_world:register("particle", {ud})

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

packets = {}

-- 所有结构都有一个类型字段
local function add_struct(name, fields, map)
    local struct = string.format("typedef struct { uint8_t type; %s } %s;", fields, name)
    common.cdata:new_struct(name, struct)

    -- packet_type 结构不是真正的数据包 因此不要对其进行索引
    if map then
        map.name = name
        table.insert(packets, map)
        packets[name] = #packets
    end
end

add_struct("packet_type", "")

add_struct("player_whois", [[
        uint16_t id;
    ]], {"id"})

add_struct("player_create", [[
        uint16_t id;
        uint8_t flags;
        float position_x, position_y, position_z;
        float orientation_x, orientation_y, orientation_z;
        unsigned char name[64];
    ]], {"id", "flags", "position_x", "position_y", "position_z", "orientation_x", "orientation_y", "orientation_z",
         "name"})

add_struct("player_update", [[
        uint16_t id;
        float position_x, position_y, position_z;
        float orientation_x, orientation_y, orientation_z;
    ]], {"id", "position_x", "position_y", "position_z", "orientation_x", "orientation_y", "orientation_z"})

add_struct("gameobj_network_update", [[
        uint16_t id;
        uint16_t netid;
        float position_x, position_y, position_z;
        float orientation_x, orientation_y, orientation_z;
    ]], {"id", "netid", "position_x", "position_y", "position_z", "orientation_x", "orientation_y", "orientation_z"})

add_struct("player_action", [[
        uint16_t id;
        uint16_t action;
    ]], {"id", "action"})

local phy_world = PHY.newWorld(64)

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

            neko.idraw_rectv(v1, v2, "R_PRIMITIVE_LINES")

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

        neko.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "R_PRIMITIVE_TRIANGLES",
            to_color(0, 255, 255, 255))
    end
end

local function phy_world_tiled_load(tiled_ud)
    print("phy_world_tiled_load()")
    -- local object_groups = neko_tiled_get_objects(tiled_ud)
    -- -- print(dump_func(ttttt))
    -- for object_group_name, v in pairs(object_groups) do
    --     if object_group_name == "collisions" then
    --         for ii, vv in ipairs(v) do
    --             print(("%s %d"):format(object_group_name, ii))
    --             dump_func(vv)
    --             phy_add_block(vv[1], vv[2], vv[3], vv[4], "tiled")
    --         end
    --     end
    -- end
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

        -- TODO 热更新进度提示?需要异步

    end
end

local s_player_pos = {
    x = 0,
    y = 0
}

local fbs_x = 640 * 1.5
local fbs_y = 360 * 1.5

local player_pos

local this_player = {
    p = to_vec2(0, 0),
    pos = to_vec2(0, 0)
}

local drawFilter = tiny.requireAll('isDrawSystem')
local updateFilter = tiny.rejectAny('isDrawSystem')

local moveableSyncSystem = tiny.processingSystem()
moveableSyncSystem.isDrawSystem = false
moveableSyncSystem.filter = tiny.requireAll("gameobj", "vector2", "velocity2", "network_t")
function moveableSyncSystem:process(e, dt)

    local net = e.network_t
    local v = e.velocity2
    local v2 = e.vector2

    local _netid = 999
    -- if net.obj_typename == "player" then
    --     _netid = 1
    -- end
    _netid = net.net_id

    local Filter = function(item, other)
        -- if     other.isCoin   then return 'cross'
        -- elseif other.isWall   then return 'slide'
        -- elseif other.isExit   then return 'touch'
        -- elseif other.isSpring then return 'bounce'
        -- end
        if net.obj_typename == "pdx" then
            return "cross"
        else
            return "slide"
        end
        -- else return nil
    end

    if v.dx ~= 0 or v.dy ~= 0 then
        local cols
        v2.x, v2.y, cols, cols_len = phy_world:move(net, v2.x + v.dx, v2.y + v.dy, Filter)
        for i = 1, cols_len do
            local col = cols[i]
            -- print(("col[%d]: other = %s, type = %s, normal = %d,%d"):format(i, dump_func(col.other), col.type,
            --     col.normal.x, col.normal.y))
        end
    end

    v.dx = v.dx / 2.0
    v.dy = v.dy / 2.0

    if (gd.tick & 1) == 0 then

        local data = {
            type = packets["gameobj_network_update"],
            id = 999,
            netid = _netid,
            position_x = v2.x,
            position_y = v2.y,
            position_z = 0,
            orientation_x = 0,
            orientation_y = 0,
            orientation_z = 0
        }

        local struct = common.cdata:set_struct("gameobj_network_update", data)
        local encoded = common.cdata:encode(struct)

        M.client:send("gameobj_network_update", encoded)
    end

    b2_world:step(dt)

    -- this_player.p = net
    -- this_player.pos = v2
end

local npcSystem = tiny.processingSystem()
npcSystem.isDrawSystem = false
npcSystem.filter = tiny.requireAll("gameobj", "vector2", "velocity2", "npc")
function npcSystem:process(e, dt)
    local lineOfSightSystem = {
        canSeePlayer = function(self, entity)
            local self_pos = entity.v2 or to_vec2(0, 0)
            local d = common.distance(this_player.pos.x, this_player.pos.y, self_pos.x, self_pos.y, true)
            -- print(entity,d, this_player.pos.x, this_player.pos.y, self_pos.x, self_pos.y)
            return math.abs(d) <= 30000
        end
    }

    -- 询问其他系统该实体是否可以看到玩家
    local function canSeePlayer(entity, dt)
        if lineOfSightSystem:canSeePlayer(entity) then
            -- print(entity, "canSeePlayer")
            return behavior.success
        else
            return behavior.fail
        end
    end

    -- 等待一段时间才能成功
    local function waitRandom(low, hi)
        local elapsed = 0
        local span = math.random(low, hi)

        return function(entity, dt)
            elapsed = elapsed + dt
            if elapsed >= span then
                elapsed = 0
                span = math.random(low, hi)
                return 'success'
            end
            return behavior.running
        end
    end

    local function wanderOnPlatform(entity, dt)

        local p = entity.p
        local v = entity.v

        if (gd.tick & 15) == 0 then
            p.r = common.random(0, 100)
        end

        local r = p.r

        local player_v = 1.5 * dt

        if r >= 0 and r <= 25 then
            v.dx = v.dx - player_v
        end
        if r >= 25 and r <= 50 then
            v.dx = v.dx + player_v
        end
        if r >= 50 and r <= 75 then
            v.dy = v.dy - player_v
        end
        if r >= 75 and r <= 100 then
            v.dy = v.dy + player_v
        end

        return 'success'
    end

    -- Walk to the edge of the platform, then wait a bit.
    local function walkAround()
        return sequence({wanderOnPlatform, waitRandom(1, 3)})
    end

    -- If the player is visible, then shoot them 3 times in rapid succession.
    local function shootPlayer()
        -- print('shootPlayer            called once at beginning and that is it!')
        -- return sequence({canSeePlayer, repeatNode(3, sequence({shootBlaster, waitRandom(1, 1)}))})
        return sequence({canSeePlayer})
    end

    local function makeBrain()
        return selector({shootPlayer(), walkAround()})
    end

    local brain = makeBrain()

    local p = e.npc
    local v = e.velocity2
    local v2 = e.vector2

    local aseprite_render = SAFE_UD(p)

    if (gd.tick & 31) == 0 then
        if math.abs(v.dx) >= 0.6 or math.abs(v.dy) >= 0.6 then
            aseprite_render:update_animation("walk");
        else
            aseprite_render:update_animation("Idle");
        end
    end

    aseprite_render:update()

    local e = {
        p = p,
        v = v,
        v2 = v2
    }

    brain(e, 1)

    -- local Filter = function(item, other)
    --     -- if     other.isCoin   then return 'cross'
    --     -- elseif other.isWall   then return 'slide'
    --     -- elseif other.isExit   then return 'touch'
    --     -- elseif other.isSpring then return 'bounce'
    --     -- end

    --     if p.npc_typename == "pdx" then
    --         return "cross"
    --     else
    --         return "slide"
    --     end

    --     -- else return nil
    -- end

    -- if v.dx ~= 0 or v.dy ~= 0 then
    --     local cols
    --     v2.x, v2.y, cols, cols_len = phy_world:move(p, v2.x + v.dx, v2.y + v.dy, Filter)
    --     for i = 1, cols_len do
    --         local col = cols[i]
    --         -- print(("col[%d]: other = %s, type = %s, normal = %d,%d"):format(i, dump_func(col.other), col.type,
    --         --     col.normal.x, col.normal.y))
    --     end
    -- end

    -- v.dx = v.dx / 2.0
    -- v.dy = v.dy / 2.0

end

local playerSystem = tiny.processingSystem()
playerSystem.isDrawSystem = false
playerSystem.filter = tiny.requireAll("gameobj", "vector2", "velocity2", "player")
function playerSystem:process(e, dt)

    -- if gd.tick == 256 then
    --     gd.tick = 0
    -- end

    local p = e.player
    local v = e.velocity2
    local v2 = e.vector2

    local aseprite_render = SAFE_UD(p)

    if (gd.tick & 31) == 0 then
        if math.abs(v.dx) >= 0.6 or math.abs(v.dy) >= 0.6 then
            aseprite_render:update_animation("Run");
        else
            aseprite_render:update_animation("Idle");
        end
    end

    aseprite_render:update()

    local player_vs

    if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
        player_v = 650.1 * dt
    else
        player_v = 450.1 * dt
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

    -- if v.dx ~= 0 or v.dy ~= 0 then
    --     local cols
    --     v2.x, v2.y, cols, cols_len = phy_world:move(p, v2.x + v.dx, v2.y + v.dy)
    --     for i = 1, cols_len do
    --         local col = cols[i]
    --         -- print(("col[%d]: other = %s, type = %s, normal = %d,%d"):format(i, dump_func(col.other), col.type,
    --         --     col.normal.x, col.normal.y))
    --     end
    -- end

    -- v.dx = v.dx / 2.0
    -- v.dy = v.dy / 2.0

    -- if (gd.tick & 3) == 0 then

    --     local data = {
    --         type = packets["player_update"],
    --         id = 999,
    --         position_x = v2.x,
    --         position_y = v2.y,
    --         position_z = 0,
    --         orientation_x = 0,
    --         orientation_y = 0,
    --         orientation_z = 0
    --     }

    --     local struct = common.cdata:set_struct("player_update", data)
    --     local encoded = common.cdata:encode(struct)

    --     M.client:send("player_position", encoded)
    -- end

    this_player.p = p
    this_player.pos = v2
end

local tiledRenderSystem = tiny.processingSystem()
tiledRenderSystem.isDrawSystem = true
tiledRenderSystem.filter = tiny.requireAll("vector2", "tiled", "gameobj")
function tiledRenderSystem:process(e, dt)

    local v2 = e.vector2
    local t = e.tiled
    local obj = e.gameobj

    if CObject.CGameObject_get_active(SAFE_SD(obj)) then
        neko.tiled_render(SAFE_UD(t), gd.main_rp, to_vec2(0, 0), gd.cam.x, fbs_x + gd.cam.x, gd.cam.y, fbs_y + gd.cam.y)

        -- neko.idraw_texture(gd.rt)
        -- neko.idraw_rectvd(v2, to_vec2(64 * 16 * 2, 32 * 16 * 2), to_vec2(0.0, 1.0), to_vec2(1.0, 0.0),
        --     "R_PRIMITIVE_TRIANGLES")

        neko.idraw_defaults()
        neko.idraw_text(v2.x, v2.y - 10, ("tiled_map x:%f y:%f"):format(v2.x, v2.y))
        neko.idraw_defaults()
    end
end

function tiledRenderSystem:onRemove(e)
    local t = e.tiled
    neko.tiled_end(SAFE_UD(t))
    print("tiledRenderSystem:onRemove")
end

local phyRenderSystem = tiny.processingSystem()
phyRenderSystem.isDrawSystem = true
phyRenderSystem.filter = tiny.requireAll("vector2", "gameobj", "phy")
function phyRenderSystem:process(e, dt)

    local v2 = e.vector2
    local obj = e.gameobj
    local phy = e.phy

    local phy_body = phy.body

    phy_body:draw_fixtures()

end

local npcRenderSystem = tiny.processingSystem()
npcRenderSystem.isDrawSystem = true
npcRenderSystem.filter = tiny.requireAll("vector2", "velocity2", "npc", "gameobj")
function npcRenderSystem:process(e, dt)

    local v = e.velocity2
    local v2 = e.vector2
    local obj = e.gameobj
    local p = e.npc

    local direction = 0
    if v.dx < 0 then
        direction = 1
    end

    local render_ase = common.deepcopy(v2)
    render_ase.x = render_ase.x - 50
    render_ase.y = render_ase.y - 20

    SAFE_UD(p):render(render_ase, direction, p.scale)

    if neko.conf.cvar.show_physics_debug then
        neko.idraw_rectv(render_ase, {
            x = obj.w * p.scale,
            y = obj.h * p.scale
        }, "R_PRIMITIVE_LINES", to_color(0, 144, 144, 255))

        neko.idraw_rectv(v2, {
            x = 10,
            y = 10
        }, "R_PRIMITIVE_LINES", to_color(0, 144, 144, 255))
    end
end

local playerRenderSystem = tiny.processingSystem()
playerRenderSystem.isDrawSystem = true
playerRenderSystem.filter = tiny.requireAll("gameobj", "vector2", "velocity2", "player")
function playerRenderSystem:process(e, dt)

    local v = e.velocity2
    local v2 = e.vector2
    local obj = e.gameobj
    local p = e.player

    local direction = 0
    if v.dx < 0 then
        direction = 1
    end

    player_pos = v2

    local render_ase = common.deepcopy(v2)
    render_ase.x = render_ase.x - 50
    render_ase.y = render_ase.y - 20

    SAFE_UD(p):render(render_ase, direction, p.scale)

    if neko.conf.cvar.show_physics_debug then
        neko.idraw_rectv(render_ase, {
            x = obj.w * p.scale,
            y = obj.h * p.scale
        }, "R_PRIMITIVE_LINES", to_color(255, 144, 144, 255))

        neko.idraw_rectv(v2, {
            x = 10,
            y = 10
        }, "R_PRIMITIVE_LINES", to_color(255, 144, 144, 255))
    end
end

local test_pack, test_handle, test_items

M.sub_init_thread = function()
    -- gd.assetsys = neko.filesystem_create()
    -- gd.filewatch = neko.filewatch_create(gd.assetsys)

    -- neko.filewatch_mount(gd.filewatch, neko_file_path("gamedir/maps"), "/maps");
    -- neko.filewatch_start(gd.filewatch, "/maps", "test_filewatch_callback")

    -- gd.hot_code["player_update.lua"] = neko_load("player_update.lua")
    -- gd.hot_code["npc.lua"] = neko_load("npc.lua")

    -- pack.build(neko_file_path("gamedir/sc_build.pack"),
    --     {"source/lua/game/web_console.lua", "source/lua/game/tweens.lua"})

    local test_pack_1, test_pack_2, test_pack_3 = pack.info(neko_file_path("gamedir/res2.pack"))

    print("pack_info", test_pack_1, test_pack_2, test_pack_3)

    test_pack = pack.construct("test_pack_handle", neko_file_path("gamedir/res2.pack"))
    test_handle = pack.assets_load(test_pack, "gamedir/assets/textures/cat.aseprite")
    test_items = pack.items(test_pack)

    dump_func(test_items)

    -- server_load()

    M.client = network.newClient("localhost", 22122)
    M.server = network.newServer("localhost", 22122)

    M.server:on("connect", function(data, peer)
        local msg = "Hello from server!"
        peer:send("hello", msg)
    end)

    M.server:on("gameobj_network_update", function(data)

        local header = common.cdata:decode("packet_type", data)
        local map = packets[header.type]

        if not map then
            error(string.format("Invalid packet type (%s) received!", header.type))
            return
        end

        local decoded = common.cdata:decode(map.name, data)

        if decoded.netid == 1 then
            s_player_pos = {
                x = decoded.position_x,
                y = decoded.position_y
            }
        end

    end)

    -- M.server:on("player_position", function(data)

    --     local header = common.cdata:decode("packet_type", data)
    --     local map = packets[header.type]

    --     if not map then
    --         error(string.format("Invalid packet type (%s) received!", header.type))
    --         return
    --     end

    --     local decoded = common.cdata:decode(map.name, data)

    --     s_player_pos = {
    --         x = decoded.position_x,
    --         y = decoded.position_y
    --     }
    -- end)

    M.client:on("connect", function(data)
        print("Client connected to the server.")
    end)

    M.client:on("disconnect", function(data)
        print("Client disconnected from the server.")
    end)

    M.client:on("hello", function(msg)
        print("The server replied: " .. msg)
    end)

    M.client:connect()

end

local win_w, win_h
-- local test_audio

local world = tiny.world(npcSystem, playerSystem, moveableSyncSystem, tiledRenderSystem, npcRenderSystem,
    playerRenderSystem, phyRenderSystem)

M.sub_init = function()

    test_ase_witch = neko.aseprite.create(neko_file_path("gamedir/assets/textures/B_witch.ase"))
    test_ase_harvester = neko.aseprite.create(neko_file_path("gamedir/assets/textures/TheHarvester.ase"))
    test_ase_pdx = neko.aseprite.create(neko_file_path("gamedir/assets/textures/pdx.ase"))

    win_w, win_h = neko_window_size(neko_main_window())

    gd.main_fbo = neko.render_framebuffer_create()
    gd.main_rt = neko.render_texture_create(fbs_x, fbs_y, {
        type = "R_TEXTURE_2D",
        format = "R_TEXTURE_FORMAT_RGBA32F",
        wrap_s = "R_TEXTURE_WRAP_REPEAT",
        wrap_t = "R_TEXTURE_WRAP_REPEAT",
        min_filter = "R_TEXTURE_FILTER_NEAREST",
        mag_filter = "R_TEXTURE_FILTER_NEAREST"
    })
    gd.main_rp = neko.render_renderpass_create(gd.main_fbo, gd.main_rt)

    local texture_list = {"gamedir/assets/textures/dragon_zombie.png", "gamedir/assets/textures/night_spirit.png"}
    gd.test_batch = neko.sprite_batch_create(32, texture_list, batch_vs, batch_ps)

    gd.cam = to_vec2(0.0, 0.0)

    local eid1 = tiny.addEntity(world, {
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
            ud = neko.aseprite_render.create(test_ase_witch)
        },
        network_t = {
            net_id = 1,
            obj_typename = "player"
        },
        phy = {
            body = b2_world:make_dynamic_body{
                x = 360,
                y = 420,
                linear_damping = 25,
                fixed_rotation = true
            },
            test = 1
        }
    })

    local v2, v, player, player_obj, netid, player_phy = eid1.vector2, eid1.velocity2, eid1.player, eid1.gameobj,
        eid1.network_t, eid1.phy
    phy_world:add(netid, v2.x, v2.y, player_obj.w * player.scale - 100, player_obj.h * player.scale - 40)

    player_phy.body:make_circle_fixture{
        y = -9,
        radius = 10
    }
    player_phy.body:make_circle_fixture{
        y = -6,
        radius = 10
    }

    -- phy_add_block(800 - 32, 120, 32, 600 - 32 * 2)

    local eid2 = tiny.addEntity(world, {
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
        tiled = {
            path = "/maps/map.tmx",
            ud = neko.tiled_create(neko_file_path("gamedir/maps/map.tmx"), sprite_vs, sprite_fs)
        }
    })

    -- local tiled_v2, tiled_v, tiled, tiled_obj = ecs_world:get(eid2, "vector2", "velocity2", "tiled_map", "gameobj")
    phy_world_tiled_load(SAFE_UD(eid2.tiled))

    -- test_audio = neko.audio_load("C:/Users/kaoruxun/Projects/Neko/dataWak/audio/otoha.wav")

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
        local w_x = common.random(1, 1000)
        local w_y = common.random(1, 1000)

        local e = tiny.addEntity(world, {
            gameobj = {
                name = "npc_pdx_" .. i,
                w = 48,
                h = 48,
                sd = CObject.new_obj(1010 + i, true, true)
            },
            vector2 = {
                x = w_x,
                y = w_y
            },
            velocity2 = {
                dx = 0,
                dy = 0
            },
            npc = {
                r = -1,
                scale = 3.0,
                ud = neko.aseprite_render.create(test_ase_pdx),
                npc_typename = "pdx"
            },
            network_t = {
                net_id = 2,
                obj_typename = "pdx"
            },
            phy = {
                body = b2_world:make_dynamic_body{
                    x = w_x,
                    y = w_y,
                    linear_damping = 25,
                    fixed_rotation = true
                },
                test = 1
            }
        })

        local v2, v, player, player_obj, netid, player_phy = e.vector2, e.velocity2, e.npc, e.gameobj, e.network_t,
            e.phy
        phy_world:add(netid, v2.x, v2.y, player_obj.w * player.scale - 100, player_obj.h * player.scale - 40)

        player_phy.body:make_circle_fixture{
            y = -9,
            radius = 10
        }
        player_phy.body:make_circle_fixture{
            y = -6,
            radius = 10
        }
    end

    for i = 1, 5, 1 do

        local w_x = common.random(1, 1000)
        local w_y = common.random(1, 1000)

        local e = tiny.addEntity(world, {
            gameobj = {
                name = "npc" .. i,
                w = 48,
                h = 48,
                sd = CObject.new_obj(1010 + i, true, true)
            },
            vector2 = {
                x = w_x,
                y = w_y
            },
            velocity2 = {
                dx = 0,
                dy = 0
            },
            npc = {
                r = -1,
                scale = 4.0,
                ud = neko.aseprite_render.create(test_ase_harvester),
                npc_typename = "harvester"
            },
            network_t = {
                net_id = 3,
                obj_typename = "harvester"
            },
            phy = {
                body = b2_world:make_dynamic_body{
                    x = w_x,
                    y = w_y,
                    linear_damping = 25,
                    fixed_rotation = true
                },
                test = 1
            }
        })

        local v2, v, player, player_obj, netid, player_phy = e.vector2, e.velocity2, e.npc, e.gameobj, e.network_t,
            e.phy
        phy_world:add(netid, v2.x, v2.y, player_obj.w * player.scale - 100, player_obj.h * player.scale - 40)

        player_phy.body:make_circle_fixture{
            y = -9,
            radius = 10
        }
        player_phy.body:make_circle_fixture{
            y = -6,
            radius = 10
        }
    end
end

M.sub_shutdown = function()
    pack.assets_unload(test_pack, test_handle)
    pack.destroy(test_pack)

    -- neko.sprite_end(gd.test_witch_spr)

    -- for v2, t in ecs_world:match("all", "vector2", "tiled_map") do
    --     neko.tiled_end(SAFE_UD(t))
    -- end

    -- for v2, t in ecs_world:match("all", "vector2", "fallingsand") do
    --     neko.fallingsand_end(SAFE_UD(t))
    -- end

    -- for v2, t in ecs_world:match("all", "vector2", "gfxt") do
    --     neko.gfxt_end(SAFE_UD(t))
    -- end

    -- for v2, t in ecs_world:match("all", "vector2", "particle") do
    --     neko.particle_end(SAFE_UD(t))
    -- end

    tiny.clearEntities(world)

    tiny.update(world) -- 最后更新

    -- neko.filewatch_stop(gd.filewatch, "/maps")

    -- neko.filewatch_fini(gd.filewatch)
    -- neko.filesystem_fini(gd.assetsys)

    -- neko.audio_unload(test_audio)

    M.client:disconnect()

    M.server:destroy()

    neko.sprite_batch_end(gd.test_batch)

    neko.render_renderpass_fini(gd.main_rp)
    neko.render_texture_fini(gd.main_rt)
    neko.render_framebuffer_fini(gd.main_fbo)
end

gd.tick = 0

M.sub_pre_update = function()
    gd.tick = gd.tick + 1

    if neko.conf.cvar.enable_hotload then
        if (gd.tick & 255) == 0 then
            -- neko.filewatch_update(gd.filewatch)
            -- neko.filewatch_notify(gd.filewatch)

            for k, v in pairs(gd.hot_code) do
                neko_hotload(k)
            end
        end
    end

    -- if (gd.tick & 255) == 0 then
    --     coroutine.resume(thread1)
    --     coroutine.resume(thread2)
    -- end

end

local max_dt = 0.0

gd.mx, gd.my = 0.0, 0.0

M.sub_update = function(dt)

    M.server:update()
    M.client:update()

    if dt > max_dt then
        max_dt = dt
        print("max_dt", max_dt)
    end

    tweens.update(dt)

    for k, v in pairs(gd.hot_code) do
        if v.my_update ~= nil then
            v.my_update(dt)
        end
    end

    -- for obj, v2, v, p in ecs_world:match("all", "gameobj", "vector2", "velocity2", "player") do
    -- end

    -- for obj, v2, v, p in ecs_world:match("all", "gameobj", "vector2", "velocity2", "npc") do
    -- end

    gd.mx, gd.my = neko_mouse_position()

    -- for v2, v, t in ecs_world:match("all", "vector2", "velocity2", "tiled_map") do

    --     local player_v

    --     if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
    --         player_v = 250.1 * dt
    --     else
    --         player_v = 220.1 * dt
    --     end

    --     if neko_was_key_down("NEKO_KEYCODE_LEFT") then
    --         v.dx = v.dx - player_v
    --     end
    --     if neko_was_key_down("NEKO_KEYCODE_RIGHT") then
    --         v.dx = v.dx + player_v
    --     end
    --     if neko_was_key_down("NEKO_KEYCODE_UP") then
    --         v.dy = v.dy - player_v
    --     end
    --     if neko_was_key_down("NEKO_KEYCODE_DOWN") then
    --         v.dy = v.dy + player_v
    --     end

    --     gd.cam.x = gd.cam.x + v.dx
    --     gd.cam.y = gd.cam.y + v.dy

    --     -- v2.x = v2.x - v.dx
    --     -- v2.y = v2.y - v.dy

    --     v.dx = v.dx / 2.0
    --     v.dy = v.dy / 2.0
    -- end

    -- ecs_world:update()

    world:update(dt * 0.0001, updateFilter)

end

local obj_select
local obj_view

M.sub_render = function()

    -- local fbs_x, fbs_y = neko_framebuffer_size()
    win_w, win_h = neko_window_size(neko_main_window())

    local t = neko_pf_elapsed_time()

    neko.render_renderpass_begin(gd.main_rp)
    neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
    neko.render_clear(0.0, 0.0, 0.0, 0.0)
    neko.render_renderpass_end()

    neko.idraw_defaults()
    -- neko.idraw_camera2d(fbs_x, fbs_y)
    neko.idraw_camera2d_ex(gd.cam.x, fbs_x + gd.cam.x, gd.cam.y, fbs_y + gd.cam.y)

    -- neko.idraw_defaults()
    neko.idraw_rectv(to_vec2(0, 0), to_vec2(50, 50), "R_PRIMITIVE_LINES")

    -- for v2, t, obj in ecs_world:match("all", "vector2", "tiled_map", "gameobj") do
    -- end

    -- for v2, v, p, obj in ecs_world:match("all", "vector2", "velocity2", "npc", "gameobj") do
    -- end

    -- for v2, v, p, obj in ecs_world:match("all", "vector2", "velocity2", "player", "gameobj") do
    -- end

    world:update(0.01, drawFilter)

    neko.idraw_rectv({
        x = s_player_pos.x - 50,
        y = s_player_pos.y - 20
    }, {
        x = 48 * 3.0,
        y = 48 * 3.0
    }, "R_PRIMITIVE_LINES", to_color(255, 255, 144, 255))

    local cc_x = player_pos.x - fbs_x / 2 + 24 * 3
    local cc_y = player_pos.y - fbs_y / 2 + 24 * 3
    tweens.to(gd.cam, 0.9, {
        x = cc_x,
        y = cc_y
    }):ease("cubicout")

    -- neko.idraw_rectv(to_vec2(gd.mx + gd.cam.x, gd.my + gd.cam.y), {
    --     x = 10,
    --     y = 10
    -- }, "R_PRIMITIVE_TRIANGLES", to_color(255, 0, 0, 255))

    -- for v2, t in ecs_world:match("all", "vector2", "fallingsand") do
    --     neko.fallingsand_update(SAFE_UD(t))
    -- end

    -- for v2, t in ecs_world:match("all", "vector2", "gfxt") do
    --     neko.gfxt_update(SAFE_UD(t))
    -- end

    -- for v2, t in ecs_world:match("all", "vector2", "particle") do
    --     neko.particle_render(SAFE_UD(t), v2)
    -- end

    -- ImGui.Begin("Hello")
    -- obj_select = obj_select or '' -- eid从0开始
    -- ImGui.PushID("TestCombo")
    -- if ImGui.BeginCombo("对象", obj_select) then
    --     for obj in ecs_world:match("all", "gameobj") do
    --         if ImGui.SelectableEx(obj.name, obj_select == obj.name) then
    --             obj_select = obj.name
    --             obj_view = obj
    --         end
    --     end
    --     ImGui.EndCombo()
    -- end
    -- ImGui.PopID()
    -- if obj_view ~= nil then
    --     local v2, v, player_obj = ecs_world:get(obj_view.__eid, "vector2", "velocity2", "gameobj")
    --     neko.gameobject_inspect(SAFE_SD(obj_view))
    --     ImGui.Text("EID %d\n命名 %s", obj_view.__eid, obj_view.name)
    --     ImGui.Text("坐标: %f %f", v2.x, v2.y)
    --     ImGui.Text("速度: %f %f (%f)", v.dx, v.dy, math.abs(math.sqrt(v.dx ^ 2 + v.dy ^ 2)))
    --     -- ECS.select(ecs_world, function(c)
    --     --     local s = {c.x}
    --     --     ImGui.DragFloat("啊？", s)
    --     --     c.x = s[1]
    --     -- end, "vector2")
    --     -- ImGui.Text("%d", n)
    --     -- for i = 1, n, 1 do
    --     --     ImGui.DragFloat("啊？", {list[i].x})
    --     -- end
    -- end
    -- ImGui.End()

    -- neko.draw_text(50.0, 50.0, "中文渲染测试 日本語レンダリングテスト Hello World! ", 3.0)

    if neko.conf.cvar.show_physics_debug then
        phy_debug_draw()
        draw_blocks()
    end

    neko.render_renderpass_begin(gd.main_rp)
    neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
    neko.idraw_draw()
    neko.render_renderpass_end()

    neko.idraw_defaults()
    neko.idraw_camera2d(win_w, win_h)
    neko.idraw_texture(gd.main_rt)
    neko.idraw_rectvd(to_vec2(0.0, 0.0), to_vec2(win_w, win_h), to_vec2(0.0, 1.0), to_vec2(1.0, 0.0),
        "R_PRIMITIVE_TRIANGLES", to_color(255, 255, 255, 255))

    -- neko.idraw_text(100, 100, "Hello 你好 world!")

    neko.render_renderpass_begin(0)
    neko.render_set_viewport(0.0, 0.0, win_w, win_h)
    neko.idraw_draw()
    neko.render_renderpass_end()

    neko.sprite_batch_render_ortho(gd.test_batch, fbs_x, fbs_y, 0, 0)
    neko.sprite_batch_render_begin(gd.test_batch)

    local dragon_zombie = neko.sprite_batch_make_sprite(gd.test_batch, 0, 250, 200, 1, common.rad2deg(t / 100000.0), 0)
    local night_spirit = neko.sprite_batch_make_sprite(gd.test_batch, 1, 0, 200, 1, 0, 0)
    neko.sprite_batch_push_sprite(gd.test_batch, dragon_zombie)
    neko.sprite_batch_push_sprite(gd.test_batch, night_spirit)

    -- for i = 0, 4 do
    --     local polish = neko.sprite_batch_make_sprite(gd.test_batch, 1, 200, 880, 1, common.rad2deg(t / 50000.0), 0)
    --     local translated = polish
    --     local polish_x = CObject.getter("neko_sprite_t", "x")(polish)
    --     local polish_y = CObject.getter("neko_sprite_t", "y")(polish)
    --     local polish_sx = CObject.getter("neko_sprite_t", "sx")(polish)
    --     local polish_sy = CObject.getter("neko_sprite_t", "sy")(polish)
    --     for j = 0, 6 do
    --         CObject.setter("neko_sprite_t", "x")(translated, polish_x + polish_sx * i)
    --         CObject.setter("neko_sprite_t", "y")(translated, polish_y + polish_sy * j)
    --         neko.sprite_batch_push_sprite(gd.test_batch, translated)
    --     end
    -- end

    neko.sprite_batch_render_end(gd.test_batch)

end

M.test_update = function()

    if neko_key_pressed("NEKO_KEYCODE_T") then
        -- collectgarbage("collect")
        -- __neko_print_registry_list()
        -- ecs_world:dump()
    end

    if neko_key_pressed("NEKO_KEYCODE_F2") then
        neko.conf.cvar.show_physics_debug = not neko.conf.cvar.show_physics_debug
    end

    -- neko_text("NekoEngine 内部测试版本", default_font, 20, win_h - 20)

    x, y = neko_mouse_delta()
end

return M

