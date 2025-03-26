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

--- CEntity ---------------------------------------------------------------------

-- compress entity save format
_cge = function(id)
    return ng._entity_resolve_saved_id(id)
end

-- ng.CEntity = ffi.metatype('CEntity', {
--     __eq = function(a, b)
--         return type(a) == 'cdata' and type(b) == 'cdata' and ffi.istype('CEntity', a) and
--                    ffi.istype('CEntity', b) and ng.native_entity_eq(a, b)
--     end,
--     __index = {
--         __serialize = function(e)
--             return string.format('_cge(%u)', e.id)
--         end
--     }
-- })

ng.CEntity = function(id)
    local ent = neko.CEntity.new()
    ent.id = id
    return ent
end

neko.CEntity.metatype({
    __eq = function(a, b)
        return a.id == b.id
    end
})

--- Vec2 -----------------------------------------------------------------------

ng.Vec2 = function(v1, v2)
    local v = neko.vec2_alloc(v1, v2)
    return v
end

ng.vec2_zero = neko.vec2.new()
ng.vec2_zero.x = 0
ng.vec2_zero.y = 0

neko.vec2.metatype({
    __add = ng.vec2_add,
    __sub = ng.vec2_sub,
    __unm = function(v)
        return ng.vec2_neg(v)
    end,
    __mul = function(a, b)
        if type(a) == 'number' then
            return ng.vec2_float_mul(b, a)
        elseif type(b) == 'number' then
            return ng.vec2_float_mul(a, b)
        else
            return ng.vec2_mul(a, b)
        end
    end,
    __div = function(a, b)
        if type(b) == 'number' then
            return ng.vec2_float_div(a, b)
        elseif type(a) == 'number' then
            return ng.float_vec2_div(a, b)
        else
            return ng.vec2_div(a, b)
        end
    end
})

-- ng.Vec2 = ffi.metatype('vec2', {
--     __add = ng.vec2_add,
--     __sub = ng.vec2_sub,
--     __unm = function(v)
--         return ng.vec2_neg(v)
--     end,
--     __mul = function(a, b)
--         if type(a) == 'number' then
--             return ng.vec2_float_mul(b, a)
--         elseif type(b) == 'number' then
--             return ng.vec2_float_mul(a, b)
--         else
--             return ng.vec2_mul(a, b)
--         end
--     end,
--     __div = function(a, b)
--         if type(b) == 'number' then
--             return ng.vec2_float_div(a, b)
--         elseif type(a) == 'number' then
--             return ng.float_vec2_div(a, b)
--         else
--             return ng.vec2_div(a, b)
--         end
--     end,
--     __index = {
--         __serialize = function(v)
--             return string.format('ng.vec2(%f, %f)', v.x, v.y)
--         end
--     }
-- })

--- Mat3 -----------------------------------------------------------------------

ng.mat3 = ng.luamat3

-- ng.Mat3 = ffi.metatype('mat3', {
--     __index = {
--         __serialize = function(m)
--             return string.format('ng.mat3(%f, %f, %f, %f, %f, %f, %f, %f, %f)', m.v[0], m.v[1], m.v[2], m.v[3], m.v[4],
--                 m.v[5], m.v[6], m.v[7], m.v[8])
--         end
--     }
-- })

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

local entity_table_mt__regname = ng.CEntity(0x114514)
local defaults_table_mt__regname = ng.CEntity(0x114514 + 1)

--
-- CEntity -> data map
--
-- can't use normal Lua tables because CEntity is cdata, which isn't
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
    -- 当向表中添加新键值对时调用
    __newindex = function(t, k, v)
        -- 获取注册表中的实体映射
        local map = rawget(t, entity_table_mt__regname)

        -- 如果值为 nil 则从映射中移除该键
        if v == nil then
            if map == nil then
                return
            end
            map[k.id] = nil
            return
        end

        -- 如果映射不存在 则创建一个新的映射
        if not map then
            map = {}
            rawset(t, entity_table_mt__regname, map)
        end
        -- 绑定默认值
        bind_defaults(t, v)
        -- 将键值对添加到映射中
        map[k.id] = {
            ["k"] = k,
            ["v"] = v
        }
    end,

    -- 当从表中获取值时调用
    __index = function(t, k)
        -- 获取注册表中的实体映射
        local map = rawget(t, entity_table_mt__regname)

        -- 如果映射不存在 则返回 nil
        if not map then
            return nil
        end

        -- 查找映射中的槽位 并返回其中的值
        local slot = map[k.id]
        if not slot then
            return nil
        end
        return slot.v
    end,

    -- 序列化函数 用于将表转换为可保存的格式
    __serialize_f = function(t)
        -- 获取注册表中的实体映射 如果不存在则返回空表
        local map = rawget(t, entity_table_mt__regname) or {}

        -- 不保存被过滤掉的实体
        local filtered = {}
        for k, slot in pairs(map) do
            if ng.entity_get_save_filter(slot.k) then
                filtered[k] = slot
            end
        end
        return 'ng.__entity_table_load', filtered
    end,

    -- 允许使用 pairs 函数进行迭代
    __pairs = function(t)
        -- 获取注册表中的实体映射
        local map = rawget(t, entity_table_mt__regname)

        return function(_, k)
            -- 如果映射不存在 则返回空
            if not map then
                return nil, nil
            end

            -- 获取映射中的下一个键值对
            local id, slot = next(map, k and k.id or nil)
            if not id then
                return nil, nil
            end
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
    local ent_c = neko.CEntity.new()
    ent_c.id = ent.id
    return ent_c
end

-- ng.systems 表
local SystemBindTable = {} -- 用于存储系统绑定的表
local systems_mt = {
    __index = function(t, systemName) -- 当访问表中不存在的键时调用
        -- 定义一个函数 用于调用 API
        local call_api = function(name)
            return ng[name] or function()
                error("API not found: " .. name)
            end
        end

        -- 尝试从表 t 中获取键 systemName 的值
        local v = rawget(t, systemName)
        if v ~= nil then
            return v
        end

        -- 尝试从 SystemBindTable 中获取键 systemName 的值
        local v = SystemBindTable[systemName]
        if v ~= nil then
            return v
        end

        -- 如果前面都没有找到 则创建一个新的表并设置元表
        local FuncTable = {} -- 这个表用于存储函数名 会被v表作为闭包捕获 并持久存在
        local v = setmetatable({}, {
            __index = function(_, systemFuncName) -- 当访问表中不存在的键时调用
                local funcName = FuncTable[systemFuncName]
                if funcName ~= nil then
                    return call_api(funcName)
                end
                funcName = systemName .. '_' .. systemFuncName -- 构造新的函数名 并存储在 FuncTable 表中
                FuncTable[systemFuncName] = funcName
                return call_api(funcName)
            end
        })
        -- 将新创建的表存储在 SystemBindTable 中 并返回
        SystemBindTable[systemName] = v
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

ng.MC_NONE = 32
ng.KC_NONE = -1

--- call ----------------------------------------------------------------------

local _call_raw = function(sys, func_name)
    return ns[sys][func_name]
end

function ng.getter(sys, prop)
    return _call_raw(sys, 'get_' .. prop) -- 这里转发到 neko.*
end

function ng.setter(sys, prop)
    return _call_raw(sys, 'set_' .. prop) -- 这里转发到 neko.*
end

function ng.adder(sys)
    return _call_raw(sys, 'add') -- 这里转发到 neko.*
end

function ng.remover(sys)
    return _call_raw(sys, 'remove') -- 这里转发到 neko.*
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

    sys.OnUpdate = sys.simple_update_all

    return sys
end

function ng.wrap_string(sys, prop)
    local old = ng.getter(sys, prop)
    ng[sys .. '_get_' .. prop] = function(ent)
        return ng.string(old(ent))
    end
end

--- 创建实体
---@param name string 实体的名字
---@return userdata 返回创建的实体CEntity
function ng.entity_create(name)
    name = name or "New Entity " .. math.random(1, 256)
    local e = neko.EntityCreate(name)
    return e
end

--- 销毁实体
---@param ent userdata 需要销毁的实体CEntity
function ng.entity_destroy(ent)
    neko.EntityDestroy(ent)
end

--- 判断实体是否被销毁
---@param ent userdata 实体CEntity
---@return boolean 返回是否被销毁
function ng.entity_destroyed(ent)
    return neko.EntityDestroyed(ent)
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
    return name_entity[name] and name_entity[name] or ng.CEntity(ng.entity_nil)
end

function ns.name.OnUpdate()
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
add_event('OnMouseDown', ng.MC_NONE)
add_event('OnMouseUp', ng.MC_NONE)
add_event('OnKeyDown', ng.KC_NONE)
add_event('OnKeyUp', ng.KC_NONE)

function ns.gui_event.OnUpdate()
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
