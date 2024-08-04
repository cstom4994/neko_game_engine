local serpent = require("libs/serpent")

-- ng.systems (shortcut ns) is a special table such that ns.sys.func evaluates
-- to C function sys_func, eg. ns.transform.rotate(...) becomes
-- transform_rotate(...)
local system_binds = {}
local systems_mt = {
    __index = function (t, k)
        local v = rawget(t, k)

        if v ~= nil then return v end

        local v = system_binds[k]
        if v ~= nil then return v end

        local names = {}
        v = setmetatable({}, {
            __index = function (_, k2)
                local name = names[k2]
                if name ~= nil then return ng[name] end
                name = k .. '_' .. k2
                names[k2] = name
                return ng[name]
            end,
        })
        system_binds[k] = v
        return v
    end,
}
ng.systems = setmetatable({}, systems_mt)
ns = ng.systems

function ng.__fire_event(event, args)
    -- store system names before firing event because systems list may change

    local sysnames = {}
    for name, sys in pairs(ns) do
        sysnames[name] = sys.receive_events == nil or sys.receive_events
    end

    for name in pairs(sysnames) do
        local system = ns[name]
        if system.enabled == nil or system.enabled then
            local func = system[event]
            if func then func(args) end
        end
    end
end

function ng.__save_all()
    local data = {}

    for name, system in pairs(ns) do
        if system.auto_saveload then
            data[name] = system
        elseif system.save_all then
            -- has special load_all() event
            data[name] = system.save_all()
        end
    end

    return serpent.dump(data, { indent = '  ', nocode = true })
end

function ng.__load_all(str)
    local f, err = loadstring(str)
    if err then error(err) end
    local data = f()

    for name, dump in pairs(data) do
        local system = rawget(ns, name)
        if system then
            -- system currently exists, must merge
            if system.auto_saveload then
                for k, v in pairs(dump) do
                    if ng.is_entity_table(system[k]) then
                        ng.entity_table_merge(system[k], v)
                    elseif --[[ system.auto_saveload_functions
                        or --]] type(v) ~= 'function' then
                            system[k] = v
                    end
                end
            elseif system.load_all then
                system.load_all(dump)
            end
        elseif dump.auto_saveload then
            -- system doesn't exist currently, just dump it in
            rawset(ns, name, dump)
        end
    end
end


-- generic add/remove, get/set for any system, property -- needs corresponding
-- C functions of the form sys_add()/sys_remove(),
-- sys_get_prop(ent)/sys_set_prop(ent, val)

unpack = table.unpack

function ng.getter(sys, prop) return ns[sys]['get_' .. prop] end
function ng.setter(sys, prop)return ns[sys]['set_' .. prop] end
function ng.get(sys, prop, ...) return ng.getter(sys, prop)(unpack({...})) end
function ng.set(sys, prop, ...) ng.setter(sys, prop)(unpack({...})) end

function ng.adder(sys) return ns[sys]['add'] end
function ng.remover(sys) return ns[sys]['remove'] end
function ng.remove(sys, ...) ng.remover(sys)(unpack({...})) end

-- multi-purpose system adder/setter, used as follows:
--
--   ent = ng.add {
--       ent = some_entity,           -- entity to modify, skip to create new
--       prefab = 'path/to/prefab',   -- initial prefab, skip for none
--       sys1 = {
--           prop1 = val1,
--           prop2 = val2,
--           ...
--       },
--       sys2 = {
--           ...
--       },
--       ...
--   }
--
-- here 'sys1' could be transform, 'prop1' could be position, for example
function ng.add(sys, ent, props)
    -- multi-add?
    if type(sys) == 'table' then
        ent = ent or sys.ent
            or (sys.prefab and ns.prefab.load(sys.prefab))
            or ng.entity_create()
        sys.ent = nil
        sys.prefab = nil
        for k, v in pairs(sys) do ng.add(k, ent, v) end
        return ent
    end

    -- all entities are already in 'entity' system
    if sys ~= 'entity' and not ns[sys].has(ent) then
        ng.adder(sys)(ent)
    end
    if (props) then
        for k, v in pairs(props) do
            ng.set(sys, k, ent, v)
        end
    end
end

ns.meta = { receive_events = false }
ns.meta.props = {}

function ng.simple_sys()
    local sys = { auto_saveload = true }
    sys.tbl = ng.entity_table()

    -- add/remove
    function sys.simple_add(ent)
        if not sys.tbl[ent] then
            local entry = { ent = ent }
            sys.tbl[ent] = entry
            if sys.create then sys.create(entry) end
        end
    end
    sys.add = sys.simple_add
    function sys.simple_remove(ent)
        local entry = sys.tbl[ent]
        if entry then
            if sys.destroy then sys.destroy(entry) end
            sys.tbl[ent] = nil
        end
    end
    sys.remove = sys.simple_remove
    function sys.simple_has(ent)
        return sys.tbl[ent] ~= nil
    end
    sys.has = sys.simple_has

    -- update
    function sys.simple_update_all()
        ng.entity_table_remove_destroyed(sys.tbl, sys.remove)
        for _, entry in pairs(sys.tbl) do
            if ns.timing.get_paused() then
                if sys.paused_update then sys.paused_update(entry) end
            else
                if sys.unpaused_update then sys.unpaused_update(entry) end
            end
            if sys.update then sys.update(entry) end
        end
    end
    sys.update_all = sys.simple_update_all

    return sys
end

function ng.wrap_string(sys, prop)
    local old = ng.getter(sys, prop)
    ng[sys .. '_get_' .. prop] = function (ent)
        return ng.string(old(ent))
    end
end
