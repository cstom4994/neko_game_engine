class "Camera"

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
