local ecs = require "ecs"
local m = {}

function m.system(it)
    for p, e in ecs.each(it) do
        -- print("entity: " .. e)
        p.x = p.x + 1
        p.y = p.y + 1
    end
end

ecs.module("name", function()
    m.Position = ecs.struct("Position", "{float x; float y;}")
    ecs.system(m.system, "Move", ecs.OnUpdate, "Position")
end)

return m
