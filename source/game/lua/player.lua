class "Player"

function Player:new(x, y)
    self.x, self.y = x, y

    self.sprite = neko.sprite_load "assets/B_witch.ase"

    self.update_thread = coroutine.create(self.co_update)

    self.facing_left = false

    self.shoot_cooldown = 0
    self.shoot_angle = 0

    self.hpbar = Hpbar(self)

    self.hp = 100
    self.hp_max = 100
end

function Player:on_create()
    self.body = LocalGame.b2:make_dynamic_body{
        x = self.x,
        y = self.y,
        linear_damping = 25,
        fixed_rotation = true
    }

    self.body:make_circle_fixture{
        y = -14,
        radius = 8,
        udata = self.id
    }
    self.body:make_circle_fixture{
        y = -10,
        radius = 8,
        udata = self.id
    }
end

function Player:on_death()
    self.body:destroy()
end

function Player:idle(dt)
    self.sprite:play "Idle"

    while true do
        if player.hp > 0 then
            if neko.key_down "w" or neko.key_down "s" or neko.key_down "a" or neko.key_down "d" then
                self:run(dt)
                self.sprite:play "Idle"
            end
        end

        self, dt = coroutine.yield()
    end
end

function Player:run(dt)
    self.sprite:play "Run"

    while true do
        local vx, vy = 0, 0

        local mag = 100

        if player.hp > 0 then
            if neko.key_down "w" then
                vy = vy - 1
            end
            if neko.key_down "s" then
                vy = vy + 1
            end
            if neko.key_down "a" then
                vx = vx - 1
            end
            if neko.key_down "d" then
                vx = vx + 1
            end
        end

        if vx == 0 and vy == 0 then
            return
        end

        if neko.key_down "lshift" then
            mag = 250
        end

        vx, vy = normalize(vx, vy)

        if vx ~= 0 then
            self.facing_left = vx < 0
        end

        self.body:set_velocity(vx * mag, vy * mag)

        self, dt = coroutine.yield()
    end
end

function Player:co_update(dt)
    self:idle(dt)
end

function Player:update(dt)
    self.sprite:update(dt)
    self.hpbar:update(dt)
    self.x, self.y = self.body:position()
    resume(self.update_thread, self, dt)

    function player_shoot(dt)
        if score <= 0 then
            return
        end

        -- 播放弓箭音效
        choose({sound_bow_1, sound_bow_2, sound_bow_3}):start()

        local ox = self.x
        local oy = self.y - 18

        local mx, my = camera:to_world_space(neko.mouse_pos())

        local angle = direction(ox, oy, mx, my)
        self.shoot_angle = angle
        local dx, dy = heading(angle, 16)

        LocalGame.world:add(Arrow(ox + dx, oy + dy, angle))

        score = score - 1
    end

    self.shoot_cooldown = self.shoot_cooldown - dt
    if neko.mouse_down(0) and self.shoot_cooldown <= 0 then
        self.shoot_cooldown = 0.4
        player_shoot(dt)
    end
    if neko.mouse_down(1) and self.shoot_cooldown <= 0 then
        self.shoot_cooldown = 0.1
        player_shoot(dt)
    end
end

function Player:draw()
    local sx = self.facing_left and -1 or 1

    if self.shoot_cooldown > 0 then
        local cos = math.cos(self.shoot_angle)
        if -1e-16 < cos and cos < 1e-16 then
            -- pass
        elseif cos < 0 then
            sx = -1
        elseif cos > 0 then
            sx = 1
        end
    end

    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y, 0, sx * 0.9, 1 * 0.9, ox, oy)

    if self.shoot_cooldown > 0 then
        local bow = bow_img
        local ox = -bow:width() - 4
        local oy = bow:height() / 2
        bow:draw(self.x, self.y - 18, self.shoot_angle, 1, 1, ox, oy)
    end

    self.hpbar:draw(self.x, self.y - 40)

    if draw_fixtures then
        self.body:draw_fixtures()
    end
end
