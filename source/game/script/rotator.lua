--
-- another silly test system
--

ns.rotator = { auto_saveload = true }

ns.rotator.tbl = ng.entity_table()

-- properties

ng.simple_props(ns.rotator, {
    speed = 2 * math.pi
})


-- add/remove

function ns.rotator.add(ent, speed)
    if ns.rotator.tbl[ent] then return end
    ns.rotator.tbl[ent] = { speed = speed or math.pi / 4 }
end
function ns.rotator.remove(ent)
    ns.rotator.tbl[ent] = nil
end
function ns.rotator.has(ent)
    return ns.rotator.tbl[ent] ~= nil
end


-- update

function ns.rotator.update_all()
    ng.entity_table_remove_destroyed(ns.rotator.tbl, ns.rotator.remove)

    for ent, rotator in pairs(ns.rotator.tbl) do
        ns.transform.rotate(ent, rotator.speed * ns.timing.dt)
    end
end
