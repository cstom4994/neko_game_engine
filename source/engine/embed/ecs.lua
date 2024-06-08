local ecs = {}
local worlds = {}

function ecs.fetch_world(name)
    local w = worlds[name]
    if not w then
        w = neko.ecs_lua_create()
        worlds[name] = w
    end
    return w
end

-- function ecs.select(ecs_world, func, ...)
--     local w
--     if type(ecs_world) == "string" then
--         w = worlds[ecs_world]
--     else
--         w = ecs_world
--     end
--     for c in w:match("all", ...) do
--         func(c)
--     end
-- end

return ecs

