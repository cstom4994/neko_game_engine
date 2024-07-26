class "Npc"

local function Npc_load_prefab(self)
    local prefab_tb
    -- local file_content = read_file("data/" .. self.name .. ".neko")
    local file_content = game_db.all[self.name]
    if file_content then
        prefab_tb = Prefabs.load(file_content)
        assert(Prefabs.check(prefab_tb))
        assert(Prefabs.node_type(prefab_tb) == "npc")
    else
        print("无法打开文件或文件不存在", self.name)
    end

    local sprite_res = prefab_tb[2]["sprite"]

    self.sprite = neko.sprite_load(sprite_res)

    self.facing_left = prefab_tb[2]["facing_left"]
    self.was_hit = prefab_tb[2]["was_hit"]
    self.hp = prefab_tb[2]["hp"]

    if prefab_tb[2]["spring"] then
        self.spring = Spring()
    end

    self.sprite:play "skel_run"
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

function Npc:hit(other)
    if self.was_hit then
        return
    end
    self.was_hit = true

    self.hp = self.hp - 1
    if self.hp == 0 then
        LocalGame.world:kill(self)
    end

    self.spring:pull(0.3)
end

function Npc:update(dt)
    self.x, self.y = self.body:position()
    self.sprite:update(dt)
    self.spring:update(dt)

    local vx, vy = self.body:velocity()
    if vx ~= 0 then
        self.facing_left = vx < 0
    end

    if self.was_hit then
        self.was_hit = false

        local mag = -70
        vx, vy = normalize(vx, vy)
        self.body:set_velocity(vx * mag, vy * mag)
    end
end

function Npc:draw()
    local sx = 1 - self.spring.x
    local sy = 1 + self.spring.x

    sx = self.facing_left and -sx or sx

    local ox = self.sprite:width() / 2
    local oy = self.sprite:height()
    self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)

    if draw_fixtures then
        self.body:draw_fixtures()
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
            self:hit(other)
        end
    end
end
