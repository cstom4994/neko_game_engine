-- 导入所有内容
-- 'nekogame.util'
-- 转义一个字符串以包含在另一个字符串中
function ng.safestr(s)
    return ("%q"):format(s):gsub("\010", "n"):gsub("\026", "\\026")
end

function ng.color_from_hex(hex)
    local function hexToRGBA(hex)
        -- 移除十六进制字符串中的 # 号（如果有）
        hex = hex:gsub("#", "")

        -- 如果没有指定Alpha值，默认为255（完全不透明）
        local r, g, b, a
        if #hex == 6 then
            r, g, b = tonumber(hex:sub(1, 2), 16), tonumber(hex:sub(3, 4), 16), tonumber(hex:sub(5, 6), 16)
            a = 255
        elseif #hex == 8 then
            r, g, b, a = tonumber(hex:sub(1, 2), 16), tonumber(hex:sub(3, 4), 16), tonumber(hex:sub(5, 6), 16),
                tonumber(hex:sub(7, 8), 16)
        else
            error("Invalid hex string")
        end

        return r, g, b, a
    end
    local r, g, b, a = hexToRGBA(hex)
    return ng.color(r / 255.0, g / 255.0, b / 255.0, a / 255.0)
end

function ng.gc_setmetatable()
    if _VERSION == "Lua 5.1" then
        local rg = assert(rawget)
        local proxy_key = rg(_G, "__GC_PROXY") or "__gc_proxy"
        local rs = assert(rawset)
        local gmt = assert(debug.getmetatable)
        local smt = assert(setmetatable)
        local np = assert(newproxy)

        return function(t, mt)
            if mt ~= nil and rg(mt, "__gc") and not rg(t, "__gc_proxy") then
                local p = np(true)
                rs(t, proxy_key, p)
                gmt(p).__gc = function()
                    rs(t, proxy_key, nil)
                    local nmt = gmt(t)
                    if not nmt then
                        return
                    end
                    local fin = rg(nmt, "__gc")
                    if fin then
                        return fin(t)
                    end
                end
            end
            return smt(t, mt)
        end
    end
    return setmetatable
end

-- hot_require 'nekogame.struct'
local ffi = FFI

--- utilities ------------------------------------------------------------------

-- dereference a cdata from a pointer
function ng.__deref_cdata(ct, p)
    return ffi.cast(ct, p)[0]
end

-- enum --> values and enum --> string functions
local enum_values_map = {}
function ng.enum_values(typename)
    if enum_values_map[typename] then
        return enum_values_map[typename]
    end
    enum_values_map[typename] = {}

    -- Lua贫瘠的反射
    -- for v in refct.typeof(typename):values() do
    --     enum_values_map[typename][v.name] = true
    -- end
    return enum_values_map[typename]
end

function ng.enum_tostring(typename, val)
    -- no nice inverse mapping exists...
    for name in pairs(ng.enum_values(typename)) do
        if ffi.new(typename, name) == val then
            return name
        end
    end
    return nil
end

-- return serialized string for cdata, func must be of form
-- void (typeof(cdata) *, Serializer *)
function ng.c_serialize(func, cdata)
    -- local s = ng.store_open()
    -- func(cdata, nil, s)
    -- local dump = ffi.string(ng.store_write_str(s))
    -- ng.store_close(s)
    return dump
end

-- return struct deserialized from string 'str', func must be of form
-- void (ctype *, Deserializer *)
function ng.c_deserialize(ctype, func, str)
    -- local cdata = ctype {}
    -- local s = ng.store_open_str(str)
    -- func(cdata, nil, cdata, s)
    -- ng.store_close(s)
    return cdata
end

-- create a save/load function given C type and C save/load functions all
-- as string names
function ng.c_save_load(ctype, c_save, c_load)
    return function(cdata)
        local cdump = ng.c_serialize(loadstring('return ' .. c_save)(), cdata)
        return 'ng.c_deserialize(' .. ctype .. ', ' .. c_load .. ', ' .. ng.safestr(cdump) .. ')'
    end
end

--- NativeEntity ---------------------------------------------------------------------

-- compress entity save format
_cge = function(id)
    return ng._entity_resolve_saved_id(id)
end

ng.NativeEntity = ffi.metatype('NativeEntity', {
    __eq = function(a, b)
        return type(a) == 'cdata' and type(b) == 'cdata' and ffi.istype('NativeEntity', a) and
                   ffi.istype('NativeEntity', b) and ng.native_entity_eq(a, b)
    end,
    __index = {
        __serialize = function(e)
            return string.format('_cge(%u)', e.id)
        end
    }
})

--- Vec2 -----------------------------------------------------------------------

ng.vec2 = ng.luavec2

ng.Vec2 = ffi.metatype('vec2', {
    __add = ng.vec2_add,
    __sub = ng.vec2_sub,
    __unm = function(v)
        return ng.vec2_neg(v)
    end,
    __mul = function(a, b)
        if type(a) == 'number' then
            return ng.vec2_scalar_mul(b, a)
        elseif type(b) == 'number' then
            return ng.vec2_scalar_mul(a, b)
        else
            return ng.vec2_mul(a, b)
        end
    end,
    __div = function(a, b)
        if type(b) == 'number' then
            return ng.vec2_scalar_div(a, b)
        elseif type(a) == 'number' then
            return ng.scalar_vec2_div(a, b)
        else
            return ng.vec2_div(a, b)
        end
    end,
    __index = {
        __serialize = function(v)
            return string.format('ng.vec2(%f, %f)', v.x, v.y)
        end
    }
})

--- Mat3 -----------------------------------------------------------------------

ng.mat3 = ng.luamat3

ng.Mat3 = ffi.metatype('mat3', {
    __index = {
        __serialize = function(m)
            return string.format('ng.mat3(%f, %f, %f, %f, %f, %f, %f, %f, %f)', m.v[0], m.v[1], m.v[2], m.v[3], m.v[4],
                m.v[5], m.v[6], m.v[7], m.v[8])
        end
    }
})

--- BBox -----------------------------------------------------------------------

-- ng.BBox = ffi.metatype('BBox', {
--     __index = {
--         __serialize = function(b)
--             return string.format('ng.bbox(ng.vec2(%f, %f), ng.vec2(%f, %f))', b.min.x, b.min.y, b.max.x, b.max.y)
--         end
--     }
-- })

-- hot_require 'nekogame.entity_table'

ng.is_nil_entity = function(ent)
    return ent.id == 0
end

local entity_table_mt__regname = ng.NativeEntity(0x114514)
local defaults_table_mt__regname = ng.NativeEntity(0x114514 + 1)

--
-- NativeEntity -> data map
--
-- can't use normal Lua tables because NativeEntity is cdata, which isn't
-- hashed right
--
local function bind_defaults(t, v)
    if type(v) == 'table' then
        local defaults = rawget(t, defaults_table_mt__regname)
        if defaults then
            setmetatable(v, {
                __index = defaults
            })
        end
    end
end

local entity_table_mt = {
    __newindex = function(t, k, v)
        local map = rawget(t, entity_table_mt__regname)

        -- remove
        if v == nil then
            if map == nil then
                return
            end
            map[k.id] = nil
            return
        end

        -- add
        if not map then
            map = {}
            rawset(t, entity_table_mt__regname, map)
        end
        bind_defaults(t, v)
        map[k.id] = {
            ['k'] = ng.NativeEntity(k),
            ['v'] = v
        }
    end,

    __index = function(t, k)
        local map = rawget(t, entity_table_mt__regname)

        -- no map => empty
        if not map then
            return nil
        end

        -- find slot, return value in it
        local slot = map[k.id]
        if not slot then
            return nil
        end
        return slot.v
    end,

    __serialize_f = function(t)
        local map = rawget(t, entity_table_mt__regname) or {}

        -- don't save filtered-out entities
        local filtered = {}
        for k, slot in pairs(map) do
            if ng.entity_get_save_filter(slot.k) then
                filtered[k] = slot
            end
        end
        return 'ng.__entity_table_load', filtered
    end,

    -- allows iteration using pairs(...)
    __pairs = function(t)
        local map = rawget(t, entity_table_mt__regname)

        return function(_, k)
            -- no map => empty
            if not map then
                return nil, nil
            end

            -- get next in map
            local id, slot = next(map, k and k.id or nil)
            if not id then
                return nil, nil
            end -- end
            return slot.k, slot.v
        end, nil, nil
    end
}

function ng.is_entity_table(t)
    return type(t) == 'table' and getmetatable(t) == entity_table_mt
end

function ng.entity_table()
    return setmetatable({}, entity_table_mt)
end

function ng.entity_table_empty(t)
    for _ in pairs(t) do
        return false
    end
    return true
end

function ng.__entity_table_load(t)
    local e = ng.entity_table()
    for _, slot in pairs(t) do
        e[slot.k] = slot.v
    end
    return e
end

function ng.entity_table_merge(t, d)
    for _, slot in pairs(rawget(d, entity_table_mt__regname) or {}) do
        bind_defaults(t, slot.v)
        t[slot.k] = slot.v
    end
end

function ng.entity_table_remove_destroyed(t, f)
    for e in pairs(t) do
        if ns.entity.destroyed(e) then
            f(e)
        end
    end
end

-- use to easily define properties with default values stored per-entity in a
-- ng.entity_table
-- sys is the system, tbl is the table properties are stored in, name is the
-- name of the property and default is the default value if unset
function ng.simple_prop(sys, name, default, tbl, set, get)
    if tbl == nil then
        tbl = sys.tbl
    end

    -- update defaults
    if default ~= nil then
        local defaults = rawget(tbl, defaults_table_mt__regname)
        if not defaults then
            defaults = {}
            rawset(tbl, defaults_table_mt__regname, defaults)
            for _, v in pairs(tbl) do
                bind_defaults(tbl, v)
            end
        end
        defaults[name] = default
    end

    -- setter
    if set ~= false then
        sys['set_' .. name] = function(ent, val)
            local t = tbl[ent]
            if t then
                t[name] = val
            end
        end
    end

    -- getter
    if get ~= false then
        sys['get_' .. name] = function(ent)
            local t = tbl[ent]
            if t then
                return t[name]
            end
        end
    end
end

function ng.simple_props(sys, props, tbl)
    for name, default in pairs(props) do
        ng.simple_prop(sys, name, default, tbl)
    end
end

-- hot_require 'nekogame.system'

ng.cent = function(ent)
    local ent_c = ng.api.core.NativeEntity.new()
    ent_c.id = ent.id
    return ent_c
end

-- ng.systems (shortcut ns) is a special table such that ns.sys.func evaluates
-- to C function sys_func, eg. ns.transform.rotate(...) becomes
-- transform_rotate(...)
local system_binds = {}
local systems_mt = {
    __index = function(t, k)

        local call_api = function(name)
            -- if ng.api[name] ~= nil then
            --     return ng.api[name]
            -- else
            --     return ng[name]
            -- end
            return ng[name]
        end

        local v = rawget(t, k)

        if v ~= nil then
            return v
        end

        local v = system_binds[k]
        if v ~= nil then
            return v
        end

        local names = {}
        v = setmetatable({}, {
            __index = function(_, k2)
                local name = names[k2]
                if name ~= nil then
                    return call_api(name)
                end
                name = k .. '_' .. k2
                names[k2] = name
                return call_api(name)
            end
        })
        system_binds[k] = v
        return v
    end
}
ng.systems = setmetatable({}, systems_mt)
ns = ng.systems

function ng.__fire_event(event, args)
    -- 在触发事件之前存储系统名称 因为系统列表可能会更改

    local sysnames = {}
    for name, sys in pairs(ns) do
        sysnames[name] = sys.receive_events == nil or sys.receive_events
    end

    for name in pairs(sysnames) do
        local system = ns[name]
        if system.enabled == nil or system.enabled then
            local func = system[event]
            if func then
                func(args)
                -- print("__fire_event", event, name)
                -- collectgarbage("collect")
                -- Inspector.breakpoint()
            end
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
        print("saving system: " .. name)
    end
    -- return common.pack(data)
    return table.show(data)
end

function ng.__load_all(str)
    local f, err = loadstring(str)
    if err then
        error(err)
    end
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

--- hook ----------------------------------------------------------------------

ng["tiled_add"] = function(ent)
    return neko.tiled_add(ent.id)
end
ng["tiled_remove"] = function(ent)
    return neko.tiled_remove(ent.id)
end
ng["tiled_has"] = function(ent)
    return neko.tiled_has(ent.id)
end
ng["tiled_set_map"] = function(ent, val)
    return neko.tiled_set_map(ent.id, val)
end
ng["tiled_get_map"] = function(ent)
    return neko.tiled_get_map(ent.id)
end
ng["tiled_map_edit"] = function(ent, layer, x, y, t)
    return neko.tiled_map_edit(ent.id, layer, x, y, t)
end

ng["entity_create"] = function()
    return ng.NativeEntity(neko.entity_create())
end
ng["entity_destroy"] = function(ent)
    return neko.entity_destroy(ent.id)
end
ng["entity_destroy_all"] = function()
    return neko.entity_destroy_all()
end
ng["entity_destroyed"] = function(ent)
    return neko.entity_destroyed(ent.id)
end
ng["native_entity_eq"] = function(a, b)
    return neko.native_entity_eq(a.id, b.id)
end
ng["entity_set_save_filter"] = function(ent, filter)
    return neko.entity_set_save_filter(ent.id, filter)
end
ng["entity_get_save_filter"] = function(ent)
    return neko.entity_get_save_filter(ent.id)
end
ng["entity_clear_save_filters"] = function()
    return neko.entity_clear_save_filters()
end

ng["console_set_visible"] = function(val)
    return neko.console_set_visible(val)
end
ng["console_get_visible"] = function()
    return neko.console_get_visible()
end
ng["console_puts"] = function(str)
    return neko.console_puts(str)
end

ng["timing_set_scale"] = function(v)
    return neko.timing_set_scale(v)
end
ng["timing_get_scale"] = function()
    return neko.timing_get_scale()
end
ng["timing_get_elapsed"] = function()
    return neko.timing_get_elapsed()
end
ng["timing_set_paused"] = function(v)
    return neko.timing_set_paused(v)
end
ng["timing_get_paused"] = function()
    return neko.timing_get_paused()
end

ng["window_clipboard"] = function()
    return neko.window_clipboard()
end
ng["window_prompt"] = function(msg, title)
    return neko.window_prompt(msg, title)
end
ng["window_setclipboard"] = function(v)
    return neko.window_setclipboard(v)
end
ng["window_focus"] = function()
    return neko.window_focus()
end
ng["window_has_focus"] = function()
    return neko.window_has_focus()
end
ng["window_scale"] = function()
    return neko.window_scale()
end

ng["transform_add"] = function(v)
    return neko.transform_add(v.id)
end
ng["transform_remove"] = function(v)
    return neko.transform_remove(v.id)
end
ng["transform_has"] = function(v)
    return neko.transform_has(v.id)
end
ng["transform_set_parent"] = function(a, b)
    return neko.transform_set_parent(a.id, b.id)
end
ng["transform_get_parent"] = function(v)
    return ng.NativeEntity(neko.transform_get_parent(v.id))
end
ng["transform_get_num_children"] = function(v)
    return neko.transform_get_num_children(v.id)
end
ng["transform_get_children"] = function(v)
    return ng.NativeEntity(neko.transform_get_children(v.id))
end
ng["transform_detach_all"] = function(v)
    return neko.transform_detach_all(v.id)
end
ng["transform_destroy_rec"] = function(v)
    return neko.transform_destroy_rec(v.id)
end

ng["input_key_down"] = function(v)
    return neko.input_key_down(v)
end
ng["input_key_release"] = function(v)
    return neko.input_key_release(v)
end
ng["input_get_mouse_pos_pixels_fix"] = function()
    return neko.input_get_mouse_pos_pixels_fix()
end
ng["input_get_mouse_pos_pixels"] = function()
    return neko.input_get_mouse_pos_pixels()
end
ng["input_get_mouse_pos_unit"] = function()
    return neko.input_get_mouse_pos_unit()
end
ng["input_mouse_down"] = function(v)
    return neko.input_mouse_down(v)
end

ng["camera_world_to_pixels"] = function(v)
    return neko.camera_world_to_pixels(v)
end
ng["camera_world_to_unit"] = function(v)
    return neko.camera_world_to_unit(v)
end
ng["camera_pixels_to_world"] = function(v)
    return neko.camera_pixels_to_world(v)
end
ng["camera_unit_to_world"] = function(v)
    return neko.camera_unit_to_world(v)
end

ng["inspector_set_visible"] = function(v)
    return neko.inspector_set_visible(v)
end
ng["inspector_get_visible"] = function()
    return neko.inspector_get_visible()
end

ng.MC_NONE = 32
ng.KC_NONE = -1

--- call ----------------------------------------------------------------------

local _call_raw = function(sys, func_name)
    return ns[sys][func_name]
end

function ng.getter(sys, prop)
    return _call_raw(sys, 'get_' .. prop) -- 这里转发到 ffi.C.* / neko.*
end

function ng.setter(sys, prop)
    return _call_raw(sys, 'set_' .. prop) -- 这里转发到 ffi.C.* / neko.*
end

function ng.adder(sys)
    return _call_raw(sys, 'add') -- 这里转发到 ffi.C.* / neko.*
end

function ng.remover(sys)
    return _call_raw(sys, 'remove') -- 这里转发到 ffi.C.* / neko.*
end

function ng.get(sys, prop, ...)
    return ng.getter(sys, prop)(unpack({...}))
end

function ng.set(sys, prop, ...)
    ng.setter(sys, prop)(unpack({...}))
end

function ng.remove(sys, ...)
    ng.remover(sys)(unpack({...}))
end

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
        ent = ent or sys.ent or (sys.prefab and ns.prefab.load(sys.prefab)) or ng.entity_create()
        sys.ent = nil
        sys.prefab = nil
        for k, v in pairs(sys) do
            ng.add(k, ent, v)
        end
        -- print({sys, ent, props})
        return ent
    end

    -- 所有实体都已经在 'entity' 系统
    if sys ~= 'entity' then
        if ns[sys]["has"] ~= nil then
            if not ns[sys].has(ent) then
                ng.adder(sys)(ent)
            end
        else
            print("system " .. sys .. " 'has' func is nil")
        end

        -- print({"ng.adder", sys})
    end
    if (props) then
        for k, v in pairs(props) do
            ng.set(sys, k, ent, v)
        end
    end
end

ns.meta = {
    receive_events = false
}
ns.meta.props = {}

function ng.simple_sys()
    local sys = {
        auto_saveload = true
    }
    sys.tbl = ng.entity_table()

    -- add/remove
    function sys.simple_add(ent)
        if not sys.tbl[ent] then
            local entry = {
                ent = ent
            }
            sys.tbl[ent] = entry
            if sys.create then
                sys.create(entry)
            end
        end
    end

    sys.add = sys.simple_add
    function sys.simple_remove(ent)
        local entry = sys.tbl[ent]
        if entry then
            if sys.destroy then
                sys.destroy(entry)
            end
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
                if sys.paused_update then
                    sys.paused_update(entry)
                end
            else
                if sys.unpaused_update then
                    sys.unpaused_update(entry)
                end
            end
            if sys.update then
                sys.update(entry)
            end
        end
    end

    sys.update_all = sys.simple_update_all

    return sys
end

function ng.wrap_string(sys, prop)
    local old = ng.getter(sys, prop)
    ng[sys .. '_get_' .. prop] = function(ent)
        return ng.string(old(ent))
    end
end

-- hot_require 'nekogame.entity'
local old_entity_create = ng.entity_create
function ng.entity_create()
    local e = old_entity_create()
    ns.group.add(e, 'default')
    return e
end

-- hot_require 'nekogame.name'
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
    return name_entity[name] and name_entity[name] or ng.NativeEntity(ng.entity_nil)
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

-- hot_require 'nekogame.group'
ns.group = {}

local group_entities = {} -- group name --> entity_table
local entity_groups = ng.entity_table() -- entity --> set of group names

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
    if not groups then
        return
    end

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
        if ns.entity.destroyed(ent) then
            ns.group.remove(ent)
        end
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

-- hot_require 'nekogame.input'
local keycode_to_string_tbl = {
    KC_ESCAPE = '<escape>',
    KC_ENTER = '<enter>',
    KC_TAB = '<tab>',
    KC_BACKSPACE = '<backspace>',
    KC_INSERT = '<insert>',
    KC_DELETE = '<delete>',
    KC_RIGHT = '<right>',
    KC_LEFT = '<left>',
    KC_DOWN = '<down>',
    KC_UP = '<up>',
    KC_PAGE_UP = '<page_up>',
    KC_PAGE_DOWN = '<page_down>',
    KC_HOME = '<home>',
    KC_END = '<end>',
    KC_CAPS_LOCK = '<caps_lock>',
    KC_SCROLL_LOCK = '<scroll_lock>',
    KC_NUM_LOCK = '<num_lock>',
    KC_PRINT_SCREEN = '<print_screen>',
    KC_PAUSE = '<pause>',
    KC_F1 = '<f1>',
    KC_F2 = '<f2>',
    KC_F3 = '<f3>',
    KC_F4 = '<f4>',
    KC_F5 = '<f5>',
    KC_F6 = '<f6>',
    KC_F7 = '<f7>',
    KC_F8 = '<f8>',
    KC_F9 = '<f9>',
    KC_F10 = '<f10>',
    KC_F11 = '<f11>',
    KC_F12 = '<f12>',
    KC_F13 = '<f13>',
    KC_F14 = '<f14>',
    KC_F15 = '<f15>',
    KC_F16 = '<f16>',
    KC_F17 = '<f17>',
    KC_F18 = '<f18>',
    KC_F19 = '<f19>',
    KC_F20 = '<f20>',
    KC_F21 = '<f21>',
    KC_F22 = '<f22>',
    KC_F23 = '<f23>',
    KC_F24 = '<f24>',
    KC_F25 = '<f25>',
    KC_KP_0 = '<kp_0>',
    KC_KP_1 = '<kp_1>',
    KC_KP_2 = '<kp_2>',
    KC_KP_3 = '<kp_3>',
    KC_KP_4 = '<kp_4>',
    KC_KP_5 = '<kp_5>',
    KC_KP_6 = '<kp_6>',
    KC_KP_7 = '<kp_7>',
    KC_KP_8 = '<kp_8>',
    KC_KP_9 = '<kp_9>',
    KC_KP_DECIMAL = '<kp_decimal>',
    KC_KP_DIVIDE = '<kp_divide>',
    KC_KP_MULTIPLY = '<kp_multiply>',
    KC_KP_SUBTRACT = '<kp_subtract>',
    KC_KP_ADD = '<kp_add>',
    KC_KP_ENTER = '<kp_enter>',
    KC_KP_EQUAL = '<kp_equal>',
    KC_LEFT_SHIFT = '<shift>',
    KC_LEFT_CONTROL = '<control>',
    KC_LEFT_ALT = '<alt>',
    KC_LEFT_SUPER = '<super>',
    KC_RIGHT_SHIFT = '<shift>',
    KC_RIGHT_CONTROL = '<control>',
    KC_RIGHT_ALT = '<alt>',
    KC_RIGHT_SUPER = '<super>',
    KC_MENU = '<menu>'
}
function ng.input_keycode_to_string(key)
    return keycode_to_string_tbl[neko.input_keycode_str(key)] or neko.input_keycode_str(key)
end

local mousecode_to_string_tbl = {
    [0] = '<mouse_1>', -- MC_LEFT
    [1] = '<mouse_2>', -- MC_RIGHT
    [2] = '<mouse_3>', -- MC_MIDDLE
    [3] = '<mouse_4>',
    [4] = '<mouse_5>',
    [5] = '<mouse_6>',
    [6] = '<mouse_7>',
    [7] = '<mouse_8>'
}
function ng.input_mousecode_to_string(mouse)
    return mousecode_to_string_tbl[tonumber(mouse)]
end

-- hot_require 'nekogame.gui'
-- local root = ns.gui.get_root()
-- ns.group.set_groups(root, 'builtin')

-- ng.wrap_string('gui_text', 'str')

--- event ----------------------------------------------------------------------

ns.gui_event = {}

local event_handlers = ng.entity_table()
local event_defaults = {}

function ns.gui_event.add()
end

local function add_event(event, default)
    event_defaults[event] = default

    ns.gui_event['set_' .. event] = function(ent, f)
        if not event_handlers[ent] then
            event_handlers[ent] = {}
        end
        event_handlers[ent][event] = f
    end
end

add_event('focus_enter', false)
add_event('focus_exit', false)
add_event('mouse_down', ng.MC_NONE)
add_event('mouse_up', ng.MC_NONE)
add_event('key_down', ng.KC_NONE)
add_event('key_up', ng.KC_NONE)

function ns.gui_event.update_all()
    for ent in pairs(event_handlers) do
        if ns.entity.destroyed(ent) then
            event_handlers[ent] = nil
        end
    end

    for ent, handlers in pairs(event_handlers) do
        for event, f in pairs(handlers) do
            local r = ns.gui['event_' .. event](ent)
            if r ~= event_defaults[event] then
                f(ent, r)
            end
        end
    end
end

print("nekogame.lua loaded")
