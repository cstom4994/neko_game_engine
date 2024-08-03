class "Npc"
class "Chort"

local function Npc_load_prefab(self)
    local prefab_tb
    -- local file_content = read_file("data/" .. self.name .. ".neko")
    local file_content = game_db.all[self.name]
    if file_content then
        prefab_tb = common.prefabs.load(file_content)
        assert(common.prefabs.check(prefab_tb))
        assert(common.prefabs.node_type(prefab_tb) == "npc")
    else
        print("无法打开文件或文件不存在", self.name)
    end

    local sprite_res = prefab_tb[2]["sprite"]

    self.sprite = neko.sprite_load(sprite_res)

    self.facing_left = prefab_tb[2]["facing_left"]
    self.was_hit = prefab_tb[2]["was_hit"]
    self.hp = prefab_tb[2]["hp"]
    self.hp_max = prefab_tb[2]["hp_max"]

    if prefab_tb[2]["spring"] then
        self.spring = Spring()
    end

    self.sprite:play "skel_run"

    self.hpbar = Hpbar(self)

    self.type = "enemy"
end

function Npc:new(x, y, name)
    self.x, self.y = x, y

    self.name = name or "Skeleton"

    Npc_load_prefab(self)
end

function Npc:on_create()
    local vx, vy = heading(random(0, math.pi * 2), 70)

    self.body = LocalGame.b2:make_dynamic_body{
        x = self.x,
        y = self.y,
        vx = vx,
        vy = vy,
        fixed_rotation = true
    }

    self.body:make_circle_fixture{
        y = -9,
        radius = 5,
        udata = self.id,
        restitution = 1,
        begin_contact = Npc.begin_contact
    }
    self.body:make_circle_fixture{
        y = -6,
        radius = 5,
        udata = self.id,
        restitution = 1,
        begin_contact = Npc.begin_contact
    }
end

function Npc:on_death()
    self.body:destroy()

    for i = 1, 5 do
        local x = self.x + random(-16, 16)
        local y = self.y + random(-16, 16)
        LocalGame.world:add(Coin(x, y))
    end
end

function Npc:hit(other, damage)
    damage = damage or 1
    if self.was_hit then
        return
    end
    self.was_hit = true

    self.hp = self.hp - damage
    if self.hp <= 0 then
        LocalGame.world:kill(self)
    end

    -- self.spring:pull(0.3)
end

function Npc:update(dt)
    self.x, self.y = self.body:position()
    self.sprite:update(dt)
    self.spring:update(dt)
    self.hpbar:update(dt)

    local vx, vy = self.body:velocity()
    if vx ~= 0 then
        self.facing_left = vx < 0
    end

    if self.was_hit then
        self.was_hit = false

        local mag = -0.01
        local vxn, vyn = normalize(vx, vy)
        self.body:set_velocity(vxn * mag, vyn * mag)
    end
end

function Npc:draw()
    local sx = 1 - self.spring.x
    local sy = 1 + self.spring.x

    sx = self.facing_left and -sx or sx

    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)

    self.hpbar:draw()

    if draw_fixtures then
        self.body:draw_fixtures()
        local vx, vy = self.body:velocity()
        pixel_font:draw(("x:%.2f, y:%.2f\nvx:%.2f, vy:%.2f"):format(self.x, self.y, vx, vy), self.x, self.y - 20, 12)
    end
end

function Npc.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local other = LocalGame.world:query_id(b:udata())

    if other ~= nil then
        local mt = getmetatable(other)

        if mt == Player then
            -- world:kill(self)
            hit_player(10)

        elseif mt == Arrow then
            self:hit(other, 50)
        end
    end
end

function Chort:new(x, y)
    self.x, self.y = x, y
    self.sprite = neko.sprite_load "assets/enemy.ase"
    self.facing_left = false
    self.hit_cooldown = 0
    self.hp = 100
    self.hp_max = 100
    self.spring = Spring()
    self.update_thread = coroutine.create(self.co_update)
    self.hpbar = Hpbar(self)
    self.type = "enemy"
end

function Chort:on_create()
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
        begin_contact = Chort.begin_contact
    }
    self.body:make_circle_fixture{
        y = -6,
        radius = 6,
        udata = self.id,
        begin_contact = Chort.begin_contact
    }
end

function Chort:on_death()
    self.body:destroy()

    for i = 1, 8 do
        local x = self.x + random(-16, 16)
        local y = self.y + random(-16, 16)
        LocalGame.world:add(Coin(x, y))
    end
end

function Chort:hit(other, damage)
    damage = damage or 1
    if self.hit_cooldown > 0 then
        return
    end

    self.hp = self.hp - damage
    if self.hp <= 0 then
        LocalGame.world:kill(self)
    end

    self.hit_cooldown = 0.2

    if getmetatable(other) == Arrow then
        self.body:set_velocity(heading(other.angle, 200))
        self.spring:pull(0.3)
    end

end

function Chort:co_update(dt)
    self.sprite:play "chort_idle"
    while true do
        local dist = distance(self.x, self.y, player.x, player.y)
        if dist < 128 or self.hit_cooldown > 0 then
            break
        end

        self, dt = coroutine.yield()
    end

    self.sprite:play "chort_run"
    while true do
        if self.hit_cooldown < 0 then
            local dx = player.x - self.x
            local dy = player.y - self.y
            dx, dy = normalize(dx, dy)
            self.facing_left = dx < 0

            local mag = 40
            self.body:set_velocity(dx * mag, dy * mag)
        end

        self, dt = coroutine.yield()
    end
end

function Chort:update(dt)
    self.hit_cooldown = self.hit_cooldown - dt
    self.x, self.y = self.body:position()
    self.sprite:update(dt)
    self.spring:update(dt)
    self.hpbar:update(dt)
    resume(self.update_thread, self, dt)
end

function Chort:draw()
    local sx = 1 - self.spring.x
    local sy = 1 + self.spring.x

    sx = self.facing_left and -sx or sx

    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)

    self.hpbar:draw()

    if draw_fixtures then
        self.body:draw_fixtures()
    end
end

function Chort.begin_contact(a, b)
    local self = LocalGame.world:query_id(a:udata())
    local other = LocalGame.world:query_id(b:udata())

    if other == nil then
        return
    end

    local mt = getmetatable(other)

    if mt == Player then
        LocalGame.world:kill(self)
        hit_player(20)

    elseif mt == Arrow then
        self:hit(other, 50)
    end
end
