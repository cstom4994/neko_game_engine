class "Target"

function Target:new(x, y, name, img)
    self.x, self.y = x, y
    self.img = img or neko.sprite_load "assets/textures/target.ase"
    self.angle = 0
    -- self.z_index = -1
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
end

function Target:on_death()
    self.body:destroy()

end

function Target:update(dt)
    self.x, self.y = self.body:position()
    -- self.angle = self.angle + 2 * dt
end

function Target:hit(what)
end

function Target:draw()
    -- local x, y = neko.mouse_pos()
    -- if x == 0 and y == 0 then
    --     return
    -- end

    -- x, y = camera:to_world_space(x, y)

    local ox = self.img:width() / 2
    local oy = self.img:height()
    self.img:draw(self.x, self.y, self.angle, 1, 1, ox, oy)

    if draw_fixtures then
        self.body:draw_fixtures()
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
