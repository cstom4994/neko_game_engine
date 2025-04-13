class "CPlayer"

function CPlayer:new(x, y)
    self.x, self.y = x, y

    self.sprite = neko.sprite_load "@gamedata/assets/B_witch.ase"

    self.update_thread = coroutine.create(self.co_update)

    self.facing_left = false

    self.shoot_cooldown = 0
    self.shoot_angle = 0

    self.hpbar = Hpbar(self)

    self.health = 100
    self.health_max = 100

    self.type = "player"
end

function CPlayer:on_create()
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

function CPlayer:on_death()
    self.body:destroy()
end

function CPlayer:idle(dt)
    self.sprite:play "Idle"

    while true do
        -- if player.health > 0 then
        if neko.key_down "w" or neko.key_down "s" or neko.key_down "a" or neko.key_down "d" then
            self:run(dt)
            self.sprite:play "Idle"
        end
        -- end

        self, dt = coroutine.yield()
    end
end

function CPlayer:run(dt)
    self.sprite:play "Run"

    while true do
        local vx, vy = 0, 0

        local mag = 100

        -- if player.health > 0 then
        if neko.key_down "w" then
            vy = vy + 1
        end
        if neko.key_down "s" then
            vy = vy - 1
        end
        if neko.key_down "a" then
            vx = vx - 1
        end
        if neko.key_down "d" then
            vx = vx + 1
        end
        -- end

        if vx == 0 and vy == 0 then
            return
        end

        if ng.input_key_down("KC_LEFT_SHIFT") then
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

function CPlayer:co_update(dt)
    self:idle(dt)
end

function CPlayer:update(dt)
    self.sprite:update(dt)
    self.hpbar:update(dt)

    self.x, self.y = self.body:position()

    resume(self.update_thread, self, dt)

    local cam = ns.camera.get_current_camera()
    if ng.is_nil_entity(cam) then
        print("player no camera?")
        return
    end

    local cam_pos = ns.transform.get_position(cam)

    local blend = 1 - 0.85 ^ (dt * 40)
    cam_pos.x = lerp(cam_pos.x, self.x, blend)
    cam_pos.y = lerp(cam_pos.y, self.y, blend)
    -- camera.scale = lerp(camera.scale, 4, dt ^ 0.8)

    ns.transform.set_position(cam, cam_pos)

    -- print({self.body:position()})

    -- if ns.edit.get_enabled() and not ng.is_nil_entity(pl) then
    -- ns.edit.bboxes_update(pl, ng.BBox(ng.bbox_bound(pl_pos, b)));
    -- end

    local ox = self.x
    local oy = self.y

    local mx, my = LocalGame.mouse_pos.x, LocalGame.mouse_pos.y

    local angle = direction(ox, oy, mx, my)
    self.shoot_angle = angle
    local dx, dy = heading(angle, 16)

    local function player_shoot(dt)
        -- if score <= 0 then
        --     return
        -- end

        -- 播放弓箭音效
        -- choose({sound_bow_1, sound_bow_2, sound_bow_3}):start()

        -- neko.sound_load(choose({"@gamedata/assets/sounds/attackBow01.wav", "@gamedata/assets/sounds/attackBow02.wav"})):start()

        -- neko.sound_load("@gamedata/assets/sounds/attackBow01.wav")
        neko.sound_load("@gamedata/assets/sounds/Shooting0002.ogg"):start()
        -- shoot_sound_1:start()

        LocalGame.world:add(Bullet(ox + dx, oy + dy - 5, angle))

        -- score = score - 1
    end

    self.shoot_cooldown = self.shoot_cooldown - dt
    if neko.mouse_down(0) and not ImGuiMouseHoldon() and self.shoot_cooldown <= 0 then
        self.shoot_cooldown = 0.15
        player_shoot(dt)
    end
    if neko.mouse_down(1) and not ImGuiMouseHoldon() and self.shoot_cooldown <= 0 then
        self.shoot_cooldown = 0.08
        player_shoot(dt)
    end
end

function CPlayer:draw()
    local sx = self.facing_left and -1 or 1

    -- if self.shoot_cooldown > 0 then
    local cos = math.cos(self.shoot_angle)
    if -1e-16 < cos and cos < 1e-16 then
        -- pass
    elseif cos < 0 then
        sx = -1
    elseif cos > 0 then
        sx = 1
    end
    -- end

    local ox = sx * self.sprite:width() / 2
    local oy = -self.sprite:height() / 2
    self.sprite:draw(self.x, self.y, 0, sx * 1, -1, ox, oy)

    -- if self.shoot_cooldown > 0 then
    local weapon_img = weapon_img_1
    local ox = 1
    local oy = (-weapon_img:height() / 2) + 8

    local weapen_angle = self.shoot_angle
    if sx < 0 then
        weapen_angle = weapen_angle + math.pi
    end
    weapon_img:draw(self.x, self.y, weapen_angle, sx * 1, -1, sx * 8, oy)
    -- end

    self.hpbar:draw(self.x + 3, self.y + 15)

    if draw_fixtures then
        self.body:draw_fixtures()
    end
end

class "CEnemy"

function CEnemy:new(x, y, name, brain)
    self.x, self.y = x, y
    self.sprite = neko.sprite_load "@gamedata/assets/enemy.ase"
    self.facing_left = false
    self.hit_cooldown = 0
    self.health = 100
    self.health_max = 100
    self.spring = Spring()
    self.update_thread = coroutine.create(self.co_update)
    self.hpbar = Hpbar(self)
    self.type = "enemy"

    self.name = name or "chort"

    self.CEntity = ns.entity.create(name)

    EcsWorld:add(self.CEntity.id, "test_component_1", {
        x = 114514,
        y = 233
    })

    if brain then
        self.brain = EnemyAI(self.x, self.y, self)
    else
        self.brain = nil
    end

    self.draw_info_on_world = {}

end

function CEnemy:on_create()
    self.body = LocalGame.b2:make_dynamic_body{
        x = self.x,
        y = self.y,
        linear_damping = 10,
        fixed_rotation = true
    }

    self.body:make_circle_fixture{
        y = -9,
        radius = 6,
        udata = self.id,
        begin_contact = CEnemy.begin_contact
    }
    self.body:make_circle_fixture{
        y = -6,
        radius = 6,
        udata = self.id,
        begin_contact = CEnemy.begin_contact
    }
end

function CEnemy:on_death()
    self.body:destroy()

    for i = 1, 8 do
        local x = self.x + random(-16, 16)
        local y = self.y + random(-16, 16)
        LocalGame.world:add(Coin(x, y))
    end

    neko.EntityDestroy(self.CEntity)
end

function CEnemy:hit(other, damage)
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

function CEnemy:co_update(dt)
    self.sprite:play(self.name .. "_idle")
    while true do
        local dist = distance(self.x, self.y, player.x, player.y)
        if dist < 128 or self.hit_cooldown > 0 then
            break
        end

        self, dt = coroutine.yield()
    end

    self.sprite:play(self.name .. "_run")
    while true do

        if self.brain ~= nil then

            self.brain:update_player_position(player.x, player.y)
            local reward = self.brain:update()

            -- self.body:set_position(self.brain.x, self.brain.y)

            local dx = self.brain.x - self.x
            local dy = self.brain.y - self.y
            dx, dy = normalize(dx, dy)
            self.facing_left = dx < 0

            local mag = 50
            self.body:set_velocity(dx * mag, dy * mag)

        else

            if self.hit_cooldown < 0 then
                local dx = player.x - self.x
                local dy = player.y - self.y
                dx, dy = normalize(dx, dy)
                self.facing_left = dx < 0

                local mag = 40
                self.body:set_velocity(dx * mag, dy * mag)
            end

        end

        self, dt = coroutine.yield()
    end
end

function CEnemy:update(dt)
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

function CEnemy:draw()
    local sx = 1 - self.spring.x
    local sy = 1 + self.spring.x

    sx = self.facing_left and -sx or sx

    local ox = sx * self.sprite:width() / 2
    local oy = -self.sprite:height() / 2
    self.sprite:draw(self.x, self.y, 0, sx * 1, -1, ox, oy)

    self.hpbar:draw(self.x + 3, self.y - 15)

    if draw_fixtures then
        self.body:draw_fixtures()
    end

    local detailed = ns.edit.get_enabled() or true

    if self.brain ~= nil then
        local info = self.brain:info(1)

        local ImGui = neko.imgui
        local window<close> = ImGuiWindow("EnemyAI")
        if window then
            if ImGui.Checkbox("绘制到世界", self.draw_info_on_world) then
            end
            ImGui.Text(info)
        end

        if self.draw_info_on_world[1] then
            default_font:draw(self.brain:info(0), self.x + 8, -(self.y + 14), 16, 1, 0.25)
        end
    end
end

function CEnemy.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local other = LocalGame.world:query_id(b:udata())

    if other == nil then
        return
    end

    local mt = getmetatable(other)

    if mt == CPlayer then
        -- LocalGame.world:kill(self)
        hit_player(choose {4, 5, 6})
    elseif mt == Bullet then
        self:hit(other, 50)
    end
end

class "Cursor"

function Cursor:new(img)
    self.img = img
    self.angle = 0
end

function Cursor:update(dt)
    self.angle = self.angle + 2 * dt
end

function Cursor:draw()
    -- local x, y = neko.mouse_pos()
    local x, y = LocalGame.mouse_pos.x, LocalGame.mouse_pos.y
    if x == 0 and y == 0 then
        return
    end

    -- x, y = camera:to_world_space(x, y)

    local ox = self.img:width() / 2
    local oy = self.img:height() / 2
    self.img:draw(x, y, self.angle, 1, 1, ox, oy)
end

class "Coin"

function Coin:new(x, y)
    self.x = x
    self.y = y
    self.z = -1
    self.vx, self.vy = heading(random(0, math.pi * 2), 20)
    self.vz = random(-120, -330)
    self.spring = Spring()
    self.update_thread = coroutine.create(self.co_update)

    self.sprite = neko.sprite_load "@gamedata/assets/coin.ase"
    self.sprite:set_frame(math.random(0, self.sprite:total_frames() - 1))
end

function Coin:co_update(dt)
    while true do
        self.vz = self.vz + dt * 800

        self.z = self.z - self.vz * dt
        self.x = self.x + self.vx * dt
        self.y = self.y + self.vy * dt

        if self.z < 0 then
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

    LocalGame.score = LocalGame.score + 1

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

class "Bullet"

function Bullet:new(x, y, angle)
    self.x = x
    self.y = y
    self.angle = angle
    self.lifetime = 0
end

function Bullet:on_create()
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
        begin_contact = Bullet.begin_contact
    }
end

function Bullet:on_death()
    self.body:destroy()
end

function Bullet:update(dt)
    self.lifetime = self.lifetime + dt
    if self.lifetime > 10 then
        LocalGame.world:kill(self)
    end

    self.x, self.y = self.body:position()
end

function Bullet:draw()
    local img = bullet_img
    local ox = img:width() / 2
    local oy = img:height() / 2

    img:play("Tag2")

    img:draw(self.x, self.y, self.angle + math.pi, -1, 1, -ox, oy)
    if draw_fixtures then
        self.body:draw_fixtures()
    end
end

function Bullet.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local is_player = b:udata() == player.id

    if not is_player then
        LocalGame.world:kill(self)
        -- 播放弓箭音效
        -- choose({sound_hitbow_1, sound_hitbow_2, sound_hitbow_3}):start()
    end
end

class "Hpbar"

function Hpbar:new(father)
    self.father = father
    self.sprite = neko.sprite_load "@gamedata/assets/hpbar.ase"
    self.angle = 0
    self.health = 10
end

function Hpbar:on_create()
    self.sprite:play "10"
end

function Hpbar:update(dt)
    if self.father.health ~= nil then
        self.health = math.floor((self.father.health / self.father.health_max) * 10)
        self.sprite:play(tostring(self.health))
    end
end

function Hpbar:draw(x, y)
    if self.health < 10 then
        local x = x or self.father.x
        local y = y or self.father.y - 20

        local ox = self.sprite:width() / 2
        local oy = self.sprite:height() / 2
        self.sprite:draw(x, y, self.angle, 0.6, 0.6, ox, oy)
    end
end

class "Target"

function Target:new(x, y, name, sprite)
    self.x, self.y = x, y
    self.sprite = sprite or neko.sprite_load "@gamedata/assets/target.ase"
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
    local oy = self.sprite:height() / 2

    if draw_fixtures then
        self.body:draw_fixtures()
    end

    local highlight = false

    -- local enemy_mt = _G["Npc"]
    -- local enemy_tb = LocalGame.world.by_mt[enemy_mt]
    local enemy_tb = LocalGame.world.by_id
    if enemy_tb ~= nil then
        for k, v in pairs(enemy_tb) do
            if v.type ~= nil then
                local local_pos = vec2(self.x, self.y)
                local enemy_pos = vec2(v.x, v.y)
                local enemy_mt = getmetatable(v)
                if local_pos:distance(enemy_pos) <= 100 and v.type == "enemy" then
                    highlight = true
                    local a = ng.Vec2(self.x, self.y)
                    local b = ng.Vec2(v.x, v.y)
                    neko.draw_line(a, b, 2.0, ng.color(1, 1, 1, 1))
                    if enemy_mt == CEnemy then
                        v:hit(self, 30)
                    else
                        v:hit(self)
                    end
                end
                if local_pos:distance(enemy_pos) <= 100 and v.type == "player" then
                    highlight = true
                    local a = ng.Vec2(self.x, self.y)
                    local b = ng.Vec2(v.x, v.y)
                    neko.draw_line(a, b, 2.0, ng.color(0.75, 0, 0, 1))
                    if enemy_mt == CPlayer then
                        if v.health < 200 then
                            v.health = v.health + 1
                        end
                    else
                        -- v:hit(self)
                    end
                end
            end
        end
    end

    if highlight then
        self.sprite:effect("1100")
    else
        self.sprite:effect(0)
    end

    self.sprite:draw(self.x, self.y + 65, self.angle, 1, -1, ox, oy)

end

function Target.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local other = LocalGame.world:query_id(b:udata())

    if other == nil then
        return
    end

    local mt = getmetatable(other)

    if mt == CPlayer then
        -- hit_player(20)
    elseif mt == Bullet then
        self:hit(other)
    end
end

class "Robot"

function Robot:new(x, y)
    self.x = x
    self.y = y
    self.z = -1

    self.sprite = neko.sprite_load "@gamedata/assets/robot.ase"

    self.angle = 0 -- 当前角度
    self.radius = 30 -- 旋转半径
    self.targetSpeed = math.pi * 4 -- 目标角速度
    self.speed = 0 -- 当前角速度
    self.damping = 4.0 -- 阻尼系数

    self.som = SecondOrderMovement(x, y)

end

function Robot:update(dt)
    self.sprite:update(dt)

    local diff = self.targetSpeed - self.speed
    -- self.speed = self.speed + diff * dt * self.damping
    self.speed = self.speed + diff * dt * self.damping - self.speed * 0.1

    -- 更新角度
    self.angle = self.angle + self.speed * dt

    local ox = player.x + math.cos(self.angle) * self.radius
    local oy = player.y + 10 + math.sin(self.angle) * self.radius

    self.som:update(dt, ox, oy)

    self.x = self.som.x
    self.y = self.som.y
end

function Robot:draw()
    local sx = 1
    local sy = -1
    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)
end
