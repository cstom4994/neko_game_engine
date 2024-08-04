local old_entity_create = ng.entity_create
function ng.entity_create()
    local e = old_entity_create()
    ns.group.add(e, 'default')
    return e
end
