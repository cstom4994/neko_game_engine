class "Target"

function Target:new(x, y, name, sprite)
    self.x, self.y = x, y
    self.sprite = sprite or neko.sprite_load "assets/textures/target.ase"
    self.angle = 0
    -- self.z_index = -1

    self.update_thread = coroutine.create(self.co_update)
end

function Target:on_create()
    self.body = LocalGame.b2:make_dynamic_body{
        x = self.x,
        y = self.y,
        linear_damping = 30,
        fixed_rotation = true
    }

    self.body:make_circle_fixture{
        y = -10,
        radius = 7,
        udata = self.id,
        begin_contact = Target.begin_contact
    }
    self.body:make_circle_fixture{
        y = -16,
        radius = 7,
        udata = self.id,
        begin_contact = Target.begin_contact
    }

    self.sprite:play "Base"
end

function Target:co_update(dt)
    while true do
        -- local dist = distance(self.x, self.y, player.x, player.y)
        -- if dist < 128 or self.hit_cooldown > 0 then
        --     break
        -- end

        self, dt = coroutine.yield()
    end
end

function Target:on_death()
    self.body:destroy()

end

function Target:update(dt)
    self.x, self.y = self.body:position()
    -- self.angle = self.angle + 2 * dt

    resume(self.update_thread, self, dt)
end

function Target:hit(what)
end

function Target:draw()
    -- local x, y = neko.mouse_pos()
    -- if x == 0 and y == 0 then
    --     return
    -- end

    -- x, y = camera:to_world_space(x, y)

    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y, self.angle, 1, 1, ox, oy)

    if draw_fixtures then
        self.body:draw_fixtures()
    end

    -- local enemy_mt = _G["Npc"]
    -- local enemy_tb = LocalGame.world.by_mt[enemy_mt]
    local enemy_tb = LocalGame.world.by_id
    if enemy_tb ~= nil then
        for k, v in pairs(enemy_tb) do
            if v.type ~= nil and v.type == "enemy" then
                local local_pos = vec2(self.x, self.y)
                local enemy_pos = vec2(v.x, v.y)
                local enemy_mt = getmetatable(v)
                if local_pos:distance(enemy_pos) <= 100 then
                    neko.draw_line(self.x, self.y - 40, v.x, v.y)
                    if enemy_mt == Chort then
                        v:hit(self, 30)
                    else
                        v:hit(self)
                    end
                end
            end
        end
    end

end

function Target.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local other = LocalGame.world:query_id(b:udata())

    if other == nil then
        return
    end

    local mt = getmetatable(other)

    if mt == Player then
        -- world:kill(self)
        -- hit_player(20)

    elseif mt == Arrow then
        self:hit(other)
    end
end
