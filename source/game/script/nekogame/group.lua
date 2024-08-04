ns.group = {}

local group_entities = {}                 -- group name --> entity_table
local entity_groups = ng.entity_table()   -- entity --> set of group names

-- iterate over a group collection, which can be a string of
-- space-separated group names or a table with group names as keys
local function _groups(groups)
    if type(groups) == 'string' then
        return string.gmatch(groups, '%S+')
    end
    return pairs(groups)
end

function ns.group.add(ent, groups)
    -- if no groups parameter, nothing to do
    if not groups then return end

    for group in _groups(groups) do
        -- connect both ways
        if not group_entities[group] then
            group_entities[group] = ng.entity_table()
        end
        group_entities[group][ent] = true
        if not entity_groups[ent] then
            entity_groups[ent] = {}
        end
        entity_groups[ent][group] = true
    end
end

ns.group.add_groups = ns.group.add

function ns.group.remove(ent, groups)
    if type(groups) == 'nil' then
        -- no groups given, remove from all
        if entity_groups[ent] then
            groups = entity_groups[ent]
        else
            return -- no groups to remove from
        end
    end

    for group in _groups(groups) do
        -- disconnect both ways
        if group_entities[group] then
            group_entities[group][ent] = nil
        end
        if entity_groups[ent] then
            entity_groups[ent][group] = nil
        end
    end
end

function ns.group.has(ent)
    return true
end

function ns.group.set_groups(ent, groups)
    ns.group.remove(ent)
    ns.group.add(ent, groups)
end

function ns.group.get_groups(ent)
    local groups = {}
    if entity_groups[ent] then
        for group in pairs(entity_groups[ent]) do
            table.insert(groups, group)
        end
    end
    return table.concat(groups, ' ')
end

function ns.group.get_entities(groups)
    local ents = {}
    for group in _groups(groups) do
        if group_entities[group] then
            ng.entity_table_merge(ents, group_entities[group])
        end
    end
    return ents
end

function ns.group.destroy(groups)
    for group in _groups(groups) do
        if group_entities[group] then
            for ent in pairs(group_entities[group]) do
                ns.entity.destroy(ent)
            end
        end
        group_entities[group] = nil
    end
end

function ns.group.set_save_filter(groups, val)
    for group in _groups(groups) do
        if group_entities[group] then
            for ent in pairs(group_entities[group]) do
                ns.entity.set_save_filter(ent, val)
            end
        end
    end
end

function ns.group.update_all()
    for ent in pairs(entity_groups) do
        if ns.entity.destroyed(ent) then ns.group.remove(ent) end
    end
end

function ns.group.save_all()
    return entity_groups
end

function ns.group.load_all(d)
    for ent, groups in pairs(d) do
        ns.group.set_groups(ent, groups)
    end
end
