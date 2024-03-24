local M = {}

local static_data = MEMVAR("test.static_data")
local static_data2 = MEMVAR("test.static_data2", {
    A = 3
})
-- print(static_data2.A)

local test_nn = 3

local gd = game_data

function g_foo()
    -- print("g_foo")
end

function M.my_update(dt)
    for obj, v2, v, p in ecs_world:match("all", "gameobj", "vector2", "velocity2", "player") do

        -- if gd.tick == 256 then
        --     gd.tick = 0
        -- end

        local aseprite_render = SAFE_UD(p)

        if (gd.tick & 31) == 0 then
            if math.abs(v.dx) >= 0.6 or math.abs(v.dy) >= 0.6 then
                aseprite_render:update_animation("Run");
            else
                aseprite_render:update_animation("Idle");
            end
        end

        aseprite_render:update()

        local player_v

        if neko_was_key_down("NEKO_KEYCODE_LEFT_SHIFT") then
            player_v = 550.1 * dt
        else
            player_v = 220.1 * dt
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

        if v.dx ~= 0 or v.dy ~= 0 then
            local cols
            v2.x, v2.y, cols, cols_len = phy_world:move(p, v2.x + v.dx, v2.y + v.dy)
            for i = 1, cols_len do
                local col = cols[i]
                -- print(("col[%d]: other = %s, type = %s, normal = %d,%d"):format(i, dump_func(col.other), col.type,
                --     col.normal.x, col.normal.y))
            end
        end

        v.dx = v.dx / 2.0
        v.dy = v.dy / 2.0
    end
end

return M

