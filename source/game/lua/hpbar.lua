class "Hpbar"

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
