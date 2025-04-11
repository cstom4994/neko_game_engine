class "WorkingMan"

function WorkingMan:new(x, y)
    self.x, self.y = x, y
    self.sprite = neko.sprite_load "@gamedata/assets/workingman.ase"
    self.facing_left = false
    self.hit_cooldown = 0
    self.health = 100
    self.health_max = 100
    self.spring = Spring()
    self.update_thread = coroutine.create(self.co_update)
    self.hpbar = Hpbar(self)
    self.type = "enemy"
end

function WorkingMan:on_create()
    self.body = LocalGame.b2:make_dynamic_body{
        x = self.x,
        y = self.y,
        linear_damping = 40,
        fixed_rotation = true
    }

    self.body:make_circle_fixture{
        y = -9,
        radius = 6,
        udata = self.id,
        begin_contact = WorkingMan.begin_contact
    }
    self.body:make_circle_fixture{
        y = -6,
        radius = 6,
        udata = self.id,
        begin_contact = WorkingMan.begin_contact
    }
end

function WorkingMan:on_death()
    self.body:destroy()

    for i = 1, 8 do
        local x = self.x + random(-16, 16)
        local y = self.y + random(-16, 16)
        LocalGame.world:add(Coin(x, y))
    end
end

function WorkingMan:hit(other, damage)
    damage = damage or 1
    if self.hit_cooldown > 0 then
        return
    end

    self.health = self.health - damage
    if self.health <= 0 then
        LocalGame.world:kill(self)
    end

    self.hit_cooldown = 0.2

    if getmetatable(other) == Bullet then
        self.body:set_velocity(heading(other.angle, 200))
        self.spring:pull(0.3)
    end
end

function WorkingMan:co_update(dt)
    -- self.sprite:play "chort_idle"
    -- while true do
    --     local dist = distance(self.x, self.y, player.x, player.y)
    --     if dist < 128 or self.hit_cooldown > 0 then
    --         break
    --     end

    --     self, dt = coroutine.yield()
    -- end

    -- self.sprite:play "chort_run"
    while true do
        -- if self.hit_cooldown < 0 then
        --     local dx = player.x - self.x
        --     local dy = player.y - self.y
        --     dx, dy = normalize(dx, dy)
        --     self.facing_left = dx < 0

        --     local mag = 40
        --     self.body:set_velocity(dx * mag, dy * mag)
        -- end

        local dist = distance(self.x, self.y, LocalGame.mouse_pos.x, LocalGame.mouse_pos.y)

        if dist < 20 then
            self.sprite:effect("100")
        else
            self.sprite:effect(0)
        end

        self, dt = coroutine.yield()
    end
end

function WorkingMan:update(dt)
    self.hit_cooldown = self.hit_cooldown - dt
    self.x, self.y = self.body:position()
    self.sprite:update(dt)
    self.spring:update(dt)
    self.hpbar:update(dt)
    resume(self.update_thread, self, dt)

    -- local vx, vy = self.body:velocity()
    -- if vx ~= 0 then
    --     self.facing_left = vx < 0
    -- end

    -- if self.was_hit then
    --     self.was_hit = false

    --     local mag = -0.01
    --     local vxn, vyn = normalize(vx, vy)
    --     self.body:set_velocity(vxn * mag, vyn * mag)
    -- end
end

function WorkingMan:draw()
    local sx = 1 - self.spring.x
    local sy = 1 + self.spring.x

    sx = self.facing_left and -sx or sx

    local ox = self.sprite:width() - 8
    local oy = -self.sprite:height()
    self.sprite:draw(self.x, self.y, 0, sx * 1.5, -1.5, ox, oy)

    self.hpbar:draw(self.x + 3, self.y - 15)

    if draw_fixtures then
        self.body:draw_fixtures()
    end
end

function WorkingMan.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local other = LocalGame.world:query_id(b:udata())

    if other == nil then
        return
    end

    local mt = getmetatable(other)

    if mt == Player then
        LocalGame.world:kill(self)
        -- hit_player(20)
    elseif mt == Bullet then
        -- self:hit(other, 50)
    end
end
