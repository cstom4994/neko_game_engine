-- can set unique string names per object, can find by name
-- name property is empty string or nil for no name
ns.name = {}

local entity_name = ng.entity_table() -- entity -> name map
local name_entity = {} -- name -> entity map

function ns.name.add(ent)
end
function ns.name.has(ent)
    return true
end
function ns.name.remove(ent)
    name = entity_name[ent]
    if name then
        name_entity[name] = nil
        entity_name[ent] = nil
    end
end

function ns.name.set_name(ent, name)
    ns.name.remove(ent) -- remove old name
    if name == '' or name == nil then
        return
    end -- no name
    if entity_name[ent] == name then
        return
    end -- already same

    -- someone else has name?
    if name_entity[name] ~= nil then
        -- already checked we don't have this name, must be another
        error("name: different entity already has name '" .. name .. "'")
    end

    name_entity[name] = ent
    entity_name[ent] = name
end
function ns.name.get_name(ent, name)
    return entity_name[ent] or ''
end

function ns.name.find(name)
    return name_entity[name] and name_entity[name] or ng.Entity(ng.entity_nil)
end

function ns.name.update_all()
    ng.entity_table_remove_destroyed(entity_name, ns.name.remove)
end

function ns.name.save_all()
    return entity_name
end

local counter = 0
function ns.name.load_all(d)
    ng.entity_table_remove_destroyed(entity_name, ns.name.remove)
    for ent, rname in pairs(d) do
        local name = rname

        -- make up new name if clashes
        while name_entity[name] ~= nil do
            name, r = string.gsub(rname, '-%d+$', '-' .. counter)
            if r == 0 then
                name = string.format('%s-%d', rname, counter)
            end
            counter = counter + 1
        end

        ns.name.set_name(ent, name)
    end
end
