local ecs = {}
local c = __neko_ecs
local worlds = {}

function ecs.fetch_world(name)
    local w = worlds[name]
    if not w then
        w = c.create_world()
        worlds[name] = w
    end
    return w
end

return ecs

