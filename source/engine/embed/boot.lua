local boot = {}
local worlds = {}

function boot.fetch_world(name)
    local w = worlds[name]
    if not w then
        w = neko.boot_f()
        worlds[name] = w
    end
    return w
end

function boot.select(ecs_world, func, ...)
    local w
    if type(ecs_world) == "string" then
        w = worlds[ecs_world]
    else
        w = ecs_world
    end
    for c in w:match("all", ...) do
        func(c)
    end
end

return boot

