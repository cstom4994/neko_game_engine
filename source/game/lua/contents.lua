class "Arrow"
class "Camera"
class "Coin"
class "Cursor"
class "Hpbar"
class "Target"

function Arrow:new(x, y, angle)
    self.x = x
    self.y = y
    self.angle = angle
    self.lifetime = 0
end

function Arrow:on_create()
    local vx, vy = heading(self.angle, 500)

    self.body = LocalGame.b2:make_dynamic_body{
        x = self.x,
        y = self.y,
        vx = vx,
        vy = vy,
        angle = self.angle,
        fixed_rotation = true
    }

    self.body:make_box_fixture{
        w = 8,
        h = 2,
        udata = self.id,
        begin_contact = Arrow.begin_contact
    }
end

function Arrow:on_death()
    self.body:destroy()
end

function Arrow:update(dt)
    self.lifetime = self.lifetime + dt
    if self.lifetime > 10 then
        LocalGame.world:kill(self)
    end

    self.x, self.y = self.body:position()
end

function Arrow:draw()
    local img = arrow_img
    local ox = img:width() / 2
    local oy = img:height() / 2
    img:draw(self.x, self.y, self.angle + math.pi / 2, 1, 1, ox, oy)

    if draw_fixtures then
        self.body:draw_fixtures()
    end
end

function Arrow.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local is_player = b:udata() == player.id

    if not is_player then
        LocalGame.world:kill(self)
        -- 播放弓箭音效
        choose({sound_hitbow_1, sound_hitbow_2, sound_hitbow_3}):start()
    end
end

function Camera:new(def)
    self.x = def.x
    self.y = def.y
    self.scale = def.scale
end

function Camera:to_world_space(x, y)
    x = x - neko.window_width() / 2
    y = y - neko.window_height() / 2

    x = x / camera.scale
    y = y / camera.scale

    x = x + camera.x
    y = y + camera.y

    return x, y
end

function Camera:begin_draw()
    neko.push_matrix()
    neko.translate(neko.window_width() / 2, neko.window_height() / 2)
    neko.scale(self.scale, self.scale)
    neko.translate(-self.x, -self.y)
end

function Camera:end_draw()
    neko.pop_matrix()
end

function Coin:new(x, y)
    self.x = x
    self.y = y
    self.z = -1
    self.vx, self.vy = heading(random(0, math.pi * 2), 20)
    self.vz = random(-120, -330)
    self.spring = Spring()
    self.update_thread = coroutine.create(self.co_update)

    self.sprite = neko.sprite_load "assets/coin.ase"
    self.sprite:set_frame(math.random(0, self.sprite:total_frames() - 1))
end

function Coin:co_update(dt)
    while true do
        self.vz = self.vz + dt * 800

        self.z = self.z + self.vz * dt
        self.x = self.x + self.vx * dt
        self.y = self.y + self.vy * dt

        if self.z > 0 then
            break
        end

        self, dt = coroutine.yield()
    end

    self.z = 0
    self.spring:pull(0.6)
    dt = sleep(0.5)

    repeat
        local dist = distance(self.x, self.y, player.x, player.y)
        self, dt = coroutine.yield()
    until dist < 128

    repeat
        local blend = 1 - 0.5 ^ (dt * 40)
        self.x = lerp(self.x, player.x, blend)
        self.y = lerp(self.y, player.y, blend)

        local dist = distance(self.x, self.y, player.x, player.y)
        self, dt = coroutine.yield()
    until dist < 8

    score = score + 1

    LocalGame.world:kill(self)
end

function Coin:update(dt)
    self.sprite:update(dt)
    self.spring:update(dt)
    resume(self.update_thread, self, dt)
end

function Coin:draw()
    local sx = 1 + self.spring.x
    local sy = 1 - self.spring.x
    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y + self.z, 0, sx, sy, ox, oy)
end

function Cursor:new(img)
    self.img = img
    self.angle = 0
end

function Cursor:update(dt)
    self.angle = self.angle + 2 * dt
end

function Cursor:draw()
    local x, y = neko.mouse_pos()
    if x == 0 and y == 0 then
        return
    end

    x, y = camera:to_world_space(x, y)

    local ox = self.img:width() / 2
    local oy = self.img:height() / 2
    self.img:draw(x, y, self.angle, 1, 1, ox, oy)
end

function Hpbar:new(father)
    self.father = father
    self.sprite = neko.sprite_load "assets/textures/hpbar.ase"
    self.angle = 0
    self.hp = 10
end

function Hpbar:on_create()
    self.sprite:play "10"
end

function Hpbar:update(dt)
    if self.father.hp ~= nil then
        self.hp = math.floor((self.father.hp / self.father.hp_max) * 10)
        self.sprite:play(tostring(self.hp))
    end
end

function Hpbar:draw(x, y)
    if self.hp < 10 then
        local x = x or self.father.x
        local y = y or self.father.y - 20

        local ox = self.sprite:width() / 2
        local oy = self.sprite:height() / 2
        self.sprite:draw(x, y, self.angle, 0.6, 0.6, ox, oy)
    end
end

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
