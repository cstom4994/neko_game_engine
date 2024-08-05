-- 声明全局静态变量 不被热更新
function _G.MEMVAR(key, def)
    if not __NEKO_MEMVARS then
        __NEKO_MEMVARS = {}
    end
    if not __NEKO_MEMVARS[key] then
        __NEKO_MEMVARS[key] = def or {}
    end
    return __NEKO_MEMVARS[key]
end

local function neko_hotfix_get_file_key(name)
    return "__NEKO_HOTCODE_:" .. name
end

-- 加载一个热更新代码
function _G.neko_load(name)
    local key = neko_hotfix_get_file_key(name)
    if _G[key] then
        return _G[key]
    end

    local tb = {}
    setmetatable(tb, {
        __index = _G
    })
    _G[key] = tb

    local fn, msg = loadfile(neko.file_path("neko_cache/lua_scripts/" .. name), "tb", tb)
    if not fn then
        print(string.format("load [%s] filed. %s", name, msg))
        return _G[key]
    end

    local t = fn()
    if type(t) == "table" then
        for k, v in pairs(t) do
            if type(_G[key][k]) ~= "nil" then
                print(string.format("load [%s] ignore var [%s]. duplicate keys", name, k))
            end
            _G[key][k] = v
        end
    end

    return _G[key]
end

-- 热更新
function _G.neko_hotload(name)
    local key = neko_hotfix_get_file_key(name)
    if not _G[key] then
        return
    end

    local fn, msg = loadfile(neko.file_path("neko_cache/lua_scripts/" .. name), "tb", _G[key])
    if not fn then
        print(string.format("hotload [%s] filed. %s", name, msg))
        return
    end

    local t = fn()
    if type(t) == "table" then
        for k, v in pairs(t) do
            if type(_G[key][k]) ~= "nil" then
                -- print(string.format("load [%s] ignore var [%s]. duplicate keys", name, k))
            end
            _G[key][k] = v
        end
    end
    -- print(string.format("hotload [%s] ok", name))
end

local M = {}

local pairs, ipairs = pairs, ipairs
local type, assert, unpack = type, assert, unpack or table.unpack
local tostring, tonumber = tostring, tonumber
local math_floor = math.floor
local math_ceil = math.ceil
local math_atan2 = math.atan2 or math.atan
local math_sqrt = math.sqrt
local math_abs = math.abs

M.neko_pi = 3.14159265358979323846264

local is_callable = function(x)
    if type(x) == "function" then
        return true
    end
    local mt = getmetatable(x)
    return mt and mt.__call ~= nil
end

function M.distance(x1, y1, x2, y2, squared)
    local dx = x1 - x2
    local dy = y1 - y2
    local s = dx * dx + dy * dy
    return squared and s or math_sqrt(s)
end

function M.random(a, b)
    if not a then
        a, b = 0, 1
    end
    if not b then
        b = 0
    end
    return a + math.random() * (b - a)
end

function M.random_table(t)
    return t[math.random(#t)]
end

function M.once(fn, ...)
    local f = M.fn(fn, ...)
    local done = false
    return function(...)
        if done then
            return
        end
        done = true
        return f(...)
    end
end

local memoize_fnkey = {}
local memoize_nil = {}

function M.memoize(fn)
    local cache = {}
    return function(...)
        local c = cache
        for i = 1, select("#", ...) do
            local a = select(i, ...) or memoize_nil
            c[a] = c[a] or {}
            c = c[a]
        end
        c[memoize_fnkey] = c[memoize_fnkey] or {fn(...)}
        return unpack(c[memoize_fnkey])
    end
end

function M.fn(fn, ...)
    assert(is_callable(fn), "expected a function as the first argument")
    local args = {...}
    return function(...)
        local a = M.concat(args, {...})
        return fn(unpack(a))
    end
end

function M.call(fn, ...)
    if fn then
        return fn(...)
    end
end

function M.dostring(str)
    return assert((loadstring or load)(str))()
end

local lambda_cache = {}

function M.lambda(str)
    if not lambda_cache[str] then
        local args, body = str:match([[^([%w,_ ]-)%->(.-)$]])
        assert(args and body, "bad string lambda")
        local s = "return function(" .. args .. ")\nreturn " .. body .. "\nend"
        lambda_cache[str] = M.dostring(s)
    end
    return lambda_cache[str]
end

function M.uuid()
    local fn = function(x)
        local r = math.random(16) - 1
        r = (x == "x") and (r + 1) or (r % 4) + 9
        return ("0123456789abcdef"):sub(r, r)
    end
    return (("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"):gsub("[xy]", fn))
end

function M.rad2deg(__R)
    return (__R * 180.0) / M.neko_pi
end

function M.str2bytes(str)
    local bytes = {}
    local length = string.len(str)
    for i = 1, length do
        local s = string.sub(str, i, i)
        local b = string.byte(s)
        table.insert(bytes, b)
    end
    return bytes
end

function M.bytes2str(bytes)
    return string.char(table.unpack(bytes))
end

local cdata = {}

function cdata:new_struct(name, struct)
    ffi.cdef(struct)

    self.structs = self.structs or {}
    self.structs[name] = ffi.typeof(name)

    self.pointers = self.pointers or {}
    self.pointers[name] = ffi.typeof(name .. "*")
end

function cdata:set_struct(name, data)
    return ffi.new(self.structs[name], data)
end

function cdata:encode(data)
    return ffi.string(ffi.cast("const char*", data), ffi.sizeof(data))
end

function cdata:decode(name, data)
    return ffi.cast(self.pointers[name], data)[0]
end

M.cdata = cdata

--[[
	This function comes directly from a stackoverflow answer by islet8.
	https://stackoverflow.com/a/16077650
--]]
local deepcopy
function deepcopy(o, seen)
    seen = seen or {}
    if o == nil then
        return nil
    end
    if seen[o] then
        return seen[o]
    end

    local no = {}
    seen[o] = no
    setmetatable(no, deepcopy(getmetatable(o), seen))

    for k, v in next, o, nil do
        k = (type(k) == 'table') and deepcopy(k, seen) or k
        v = (type(v) == 'table') and deepcopy(v, seen) or v
        no[k] = v
    end
    return no
end

M.deepcopy = deepcopy

local behavior = {}

behavior.fail = "fail"
behavior.success = "success"
behavior.running = "running"

-- 反转结果
behavior.func_invert = function(child)
    return function(entity, dt)
        local result = child(entity, dt)
        if result == behavior.success then
            return behavior.fail
        elseif result == behavior.fail then
            return behavior.success
        else
            return result
        end
    end
end

-- 平行执行
behavior.func_parallel = function(children)
    return function(...)
        local last_result = behavior.success
        for i = 1, #children do
            local child = children[i]
            local result = child(...)
            if result ~= behavior.success then
                last_result = result
            end
        end
        return last_result
    end
end

-- 重复执行
behavior.func_repeat = function(times, child)
    local count = 1
    return function(...)
        local result = child(...)
        if result == behavior.fail then
            count = 1
            return behavior.fail
        elseif result == behavior.success then
            count = count + 1
        end
        if count > times then
            count = 1
            return behavior.success
        else
            return behavior.running
        end
    end
end

-- 选择器
behavior.func_selector = function(children)
    local index = 1
    return function(...)
        while index <= #children do
            local current = children[index]
            local result = current(...)
            if result == behavior.success then
                index = 1
                return behavior.success
            end
            if result == behavior.running then
                return behavior.running
            end
            index = index + 1
        end
        index = 1
        return behavior.fail
    end
end

-- 线性执行
behavior.func_sequence = function(children)
    local index = 1
    return function(...)
        while index <= #children do
            local current = children[index]
            local result = current(...)
            if result == behavior.fail then
                index = 1
                return behavior.fail
            end
            if result == behavior.running then
                return behavior.running
            end
            index = index + 1
        end
        index = 1
        return behavior.success
    end
end

M.behavior = behavior

local utils = {}

function utils.computeIndexFromPoint(x, y, n)
    return x + y * n
end

function utils.fillArray(t, val)
    for k in ipairs(t) do
        t[k] = val
    end
end

function utils.map(x, min, max, nMin, nMax)
    return (x - min) * (nMax - nMin) / (max - min) + nMin
end

function utils.bind(x, min, max)
    return math.min(math.max(x, min), max)
end

M.utils = utils

local supported_type = {
    type_map = true,
    type_enum = true,
    type_array = true,
    type_undefined = true,
    type_struct = true,
    type_internal = true,
    type_object = true
}

local internal_type = {
    number = true,
    string = true,
    boolean = true
}

local function type_tostring(self)
    return self._typename
end

-------- 内置类型 (number/string/boolean)

local internal_mt = {
    __metatable = "type_internal",
    __tostring = type_tostring
};
internal_mt.__index = internal_mt

local function gen_type(v)
    return setmetatable({
        _default = v,
        _typename = type(v)
    }, internal_mt)
end

function internal_mt:__call(v)
    if v ~= nil then
        if type(v) ~= self._typename then
            error("type mismatch " .. tostring(v) .. " is not " .. self._typename)
        end
        return v
    else
        return self._default
    end
end

function internal_mt:verify(v)
    if type(v) == self._typename then
        return true
    else
        return false, "type mismatch " .. tostring(v) .. " is not " .. self._typename
    end
end

-------- 枚举类型

local enum_mt = {
    __metatable = "type_enum",
    __tostring = type_tostring
};
enum_mt.__index = enum_mt

function enum_mt:__call(v)
    if v == nil then
        return self._default
    else
        assert(self[v] == true, "Invalid enum value")
        return v
    end
end

function enum_mt:verify(v)
    if self[v] == true then
        return true
    else
        return false, "Invalid enum value " .. tostring(v)
    end
end

local function new_enum(enums)
    assert(type(enums) == "table", "Invalid enum")
    local enum_obj = {
        _default = enums[1],
        _typename = "enum"
    }
    for idx, v in ipairs(enums) do
        assert(type(v) == "string" and enum_obj[v] == nil, "Invalid enum value")
        enum_obj[v] = true
    end
    return setmetatable(enum_obj, enum_mt)
end

----------- 对象类型

local object_mt = {
    __metatable = "type_object",
    __tostring = type_tostring
};
object_mt.__index = object_mt

function object_mt:__call(obj)
    assert(self:verify(obj))
    return obj
end

function object_mt:verify(v)
    if v == nil then
        return true
    end
    local t = type(v)
    if t == "table" or t == "userdata" then
        return true
    end

    return false, "Not an object " .. tostring(v)
end

---------- 数组类型

local array_mt = {
    __metatable = "type_array",
    __tostring = type_tostring
};
array_mt.__index = array_mt

function array_mt:__call(init)
    if init == nil then
        return {}
    else
        local array = {}
        local t = self._array
        for idx, v in ipairs(init) do
            assert(t:verify(v))
            array[idx] = t(v)
        end
        return array
    end
end

function array_mt:verify(obj)
    if type(obj) ~= "table" then
        return false, "Not an table"
    else
        local t = self._array
        local max = 0
        for idx, v in ipairs(obj) do
            local ok, err = t:verify(v)
            if not ok then
                return ok, err
            end
            max = idx
        end
        for k in pairs(obj) do
            if type(k) ~= "number" then
                return false, "Invalid key " .. tostring(k)
            end
            local nk = math.tointeger(k)
            if nk == nil or nk <= 0 or nk > max then
                return false, "Invalid key " .. tostring(nk)
            end
        end
        return true
    end
end

local function new_array(t)
    assert(supported_type[getmetatable(t)], "Need a type for array")
    return setmetatable({
        _typename = "array of " .. tostring(t),
        _array = t
    }, array_mt)
end

-------------- 映射类型

local map_mt = {
    __metatable = "type_map",
    __tostring = type_tostring
};
map_mt.__index = map_mt

function map_mt:__call(init)
    if init == nil then
        return {}
    else
        local map = {}
        local keyt = self._key
        local valuet = self._value
        for k, v in pairs(init) do
            assert(keyt:verify(k))
            assert(valuet:verify(v))
            map[keyt(k)] = valuet(v)
        end
        return map
    end
end

function map_mt:verify(obj)
    if type(obj) ~= "table" then
        return false, "Not an table"
    else
        local keyt = self._key
        local valuet = self._value
        for k, v in pairs(obj) do
            local ok, err = keyt:verify(k)
            if not ok then
                return false, string.format("Invalid key %s : %s", k, err)
            end
            local ok, err = valuet:verify(v)
            if not ok then
                return false, string.format("Invalid value %s : %s", k, err)
            end
        end
        return true
    end
end

local function new_map(key, value)
    assert(supported_type[getmetatable(key)], "Need a type for key")
    assert(supported_type[getmetatable(value)], "Need a type for value")
    return setmetatable({
        _typename = string.format("map of %s:%s", key, value),
        _key = key,
        _value = value
    }, map_mt)
end

---------------------

local types = {
    enum = new_enum,
    array = new_array,
    map = new_map,
    object = setmetatable({
        _typename = "object"
    }, object_mt)
}

for _, v in ipairs {0, false, ""} do
    types[type(v)] = gen_type(v)
end

local struct_mt = {
    __metatable = "type_struct",
    __tostring = type_tostring
};
struct_mt.__index = struct_mt

function struct_mt:__call(init)
    local obj = {}
    local meta = self._types
    local default = self._defaults
    if init then
        for k, type_obj in pairs(meta) do
            local v = init[k]
            if v == nil then
                v = default[k]
            end
            obj[k] = type_obj(v)
        end
        for k, v in pairs(init) do
            if not meta[k] then
                error(tostring(k) .. " is not a valid key")
            end
        end
    else
        for k, type_obj in pairs(meta) do
            local v = default[k]
            obj[k] = type_obj(v)
        end
    end
    return obj
end

function struct_mt:verify(obj)
    local t = self._types
    if type(obj) ~= "table" then
        return false, "not a table"
    end
    for k, v in pairs(obj) do
        local meta = t[k]
        if not meta then
            return false, "Invalid key : " .. tostring(k)
        end
    end
    for k, meta in pairs(t) do
        local v = obj[k]
        local ok, err = meta:verify(v)
        if not ok then
            return false, string.format("Type mismatch : %s should be %s (%s)", k, meta, err)
        end
    end
    return true
end

local function create_type(proto, t)
    t._defaults = {}
    t._types = {}
    for k, v in pairs(proto) do
        t._defaults[k] = v
        local vt = type(v)
        if internal_type[vt] then
            t._types[k] = types[vt]
        elseif vt == "table" and supported_type[getmetatable(v)] then
            t._types[k] = v
            t._defaults[k] = nil
        else
            error("Unsupport type " .. tostring(k) .. ":" .. tostring(v))
        end
    end
    return setmetatable(t, struct_mt)
end

function types.struct(proto)
    assert(type(proto) == "table", "Invalid type proto")
    return create_type(proto, {
        _typename = "anonymous struct"
    })
end

local function define_type(_, typename, proto)
    local t = rawget(types, typename)
    if t == nil then
        t = {}
    elseif getmetatable(t) == "type_undefined" then
        debug.setmetatable(t, nil)
    else
        error("Redefined type " .. tostring(typename))
    end
    assert(type(proto) == "table", "Invalid type proto")
    local pt = getmetatable(proto)
    if pt == nil then
        proto = create_type(proto, t)
    elseif not supported_type[pt] then
        error("Invalid proto meta " .. pt)
    end
    proto._typename = typename
    types[typename] = proto
end

local function undefined_error(self)
    error(self._typename .. " is undefined")
end

local undefined_type_mt = {
    __call = undefined_error,
    verify = undefined_error,
    __metatable = "type_undefined",
    __tostring = function(self)
        return "undefined " .. self._typename
    end
}

undefined_type_mt.__index = undefined_type_mt

local function create_undefined_type(_, typename)
    local type_obj = setmetatable({
        _typename = typename
    }, undefined_type_mt)
    types[typename] = type_obj
    return type_obj
end

setmetatable(types, {
    __index = create_undefined_type
})

M.td_create = function()
    return setmetatable({}, {
        __index = types,
        __newindex = define_type
    })
end

-- xml parser

local function newParser()

    XmlParser = {};

    function XmlParser:ToXmlString(value)
        value = string.gsub(value, "&", "&amp;"); -- '&' -> "&amp;"
        value = string.gsub(value, "<", "&lt;"); -- '<' -> "&lt;"
        value = string.gsub(value, ">", "&gt;"); -- '>' -> "&gt;"
        value = string.gsub(value, "\"", "&quot;"); -- '"' -> "&quot;"
        value = string.gsub(value, "([^%w%&%;%p%\t% ])", function(c)
            return string.format("&#x%X;", string.byte(c))
        end);
        return value;
    end

    function XmlParser:FromXmlString(value)
        value = string.gsub(value, "&#x([%x]+)%;", function(h)
            return string.char(tonumber(h, 16))
        end);
        value = string.gsub(value, "&#([0-9]+)%;", function(h)
            return string.char(tonumber(h, 10))
        end);
        value = string.gsub(value, "&quot;", "\"");
        value = string.gsub(value, "&apos;", "'");
        value = string.gsub(value, "&gt;", ">");
        value = string.gsub(value, "&lt;", "<");
        value = string.gsub(value, "&amp;", "&");
        return value;
    end

    function XmlParser:ParseArgs(node, s)
        string.gsub(s, "(%w+)=([\"'])(.-)%2", function(w, _, a)
            node:addProperty(w, self:FromXmlString(a))
        end)
    end

    function XmlParser:ParseXmlText(xmlText)

        local newNode = M.xml_node

        local stack = {}
        local top = newNode()
        table.insert(stack, top)
        local ni, c, label, xarg, empty
        local i, j = 1, 1
        while true do
            ni, j, c, label, xarg, empty = string.find(xmlText, "<(%/?)([%w_:]+)(.-)(%/?)>", i)
            if not ni then
                break
            end
            local text = string.sub(xmlText, i, ni - 1);
            if not string.find(text, "^%s*$") then
                local lVal = (top:value() or "") .. self:FromXmlString(text)
                stack[#stack]:setValue(lVal)
            end
            if empty == "/" then -- empty element tag
                local lNode = newNode(label)
                self:ParseArgs(lNode, xarg)
                top:addChild(lNode)
            elseif c == "" then -- start tag
                local lNode = newNode(label)
                self:ParseArgs(lNode, xarg)
                table.insert(stack, lNode)
                top = lNode
            else -- end tag
                local toclose = table.remove(stack) -- remove top

                top = stack[#stack]
                if #stack < 1 then
                    error("XmlParser: nothing to close with " .. label)
                end
                if toclose:name() ~= label then
                    error("XmlParser: trying to close " .. toclose.name .. " with " .. label)
                end
                top:addChild(toclose)
            end
            i = j + 1
        end
        local text = string.sub(xmlText, i);
        if #stack > 1 then
            error("XmlParser: unclosed " .. stack[#stack]:name())
        end
        return top
    end

    function XmlParser:loadFile(xmlFilename, base)
        if not base then
            base = system.ResourceDirectory
        end

        local path = system.pathForFile(xmlFilename, base)
        local hFile, err = io.open(path, "r");

        if hFile and not err then
            local xmlText = hFile:read("*a"); -- read file content
            io.close(hFile);
            return self:ParseXmlText(xmlText), nil;
        else
            print(err)
            return nil
        end
    end

    return XmlParser
end

local function newNode(name)
    local node = {}
    node.___value = nil
    node.___name = name
    node.___children = {}
    node.___props = {}

    function node:value()
        return self.___value
    end
    function node:setValue(val)
        self.___value = val
    end
    function node:name()
        return self.___name
    end
    function node:setName(name)
        self.___name = name
    end
    function node:children()
        return self.___children
    end
    function node:numChildren()
        return #self.___children
    end
    function node:addChild(child)
        if self[child:name()] ~= nil then
            if type(self[child:name()].name) == "function" then
                local tempTable = {}
                table.insert(tempTable, self[child:name()])
                self[child:name()] = tempTable
            end
            table.insert(self[child:name()], child)
        else
            self[child:name()] = child
        end
        table.insert(self.___children, child)
    end

    function node:properties()
        return self.___props
    end
    function node:numProperties()
        return #self.___props
    end
    function node:addProperty(name, value)
        local lName = "@" .. name
        if self[lName] ~= nil then
            if type(self[lName]) == "string" then
                local tempTable = {}
                table.insert(tempTable, self[lName])
                self[lName] = tempTable
            end
            table.insert(self[lName], value)
        else
            self[lName] = value
        end
        table.insert(self.___props, {
            name = name,
            value = self[name]
        })
    end

    return node
end

M.xml_parser = newParser
M.xml_node = newNode

-- va

local function va()
    local math = require "math"
    local table = require "table"

    local error, assert, select = error, assert, select
    local max, unpack = math.max, table.unpack or unpack
    local setmetatable = setmetatable

    local tinsert2 = function(t, n, i, v)
        -- lua 5.2 rise error if index out of range
        -- assert(type(t) =='table')
        -- assert(type(n) =='number')
        -- assert(type(i) =='number')
        if i > n then
            t[i] = v
            return i
        end

        for j = n, i, -1 do
            t[j + 1] = t[j]
        end
        t[i] = v

        return n + 1
    end

    local tremove2 = function(t, n, i)
        -- lua 5.2 rise error if index out of range
        -- assert(type(t) =='table')
        -- assert(type(n) =='number')
        -- assert(type(i) =='number')
        if i > n then
            for j = n + 1, i do
                t[j] = nil
            end
            return n
        end

        for j = i, n do
            t[j] = t[j + 1]
        end

        return n - 1
    end

    local function idx(i, n, d)
        if i == nil then
            if not d then
                return error("number expected, got nil", 2)
            end
            return d
        end
        if i < 0 then
            i = n + i + 1
        end
        if i <= 0 then
            return error("index out of bounds", 2)
        end
        return i
    end

    local function pack(...)
        local n = select("#", ...)
        local v = {...}
        return function(...)
            if (...) == "#" then
                return n
            else
                local argc = select("#", ...)
                if argc == 0 then
                    return unpack(v, 1, n)
                else
                    local i, j = ...
                    if i == nil then
                        if j == nil then
                            j = 0
                        end
                        i = j + 1
                        if i > 0 and i <= n then
                            return i, v[i]
                        end
                    else
                        i = idx(i, n, 1)
                        j = idx(j, n, i)
                        return unpack(v, i, j)
                    end
                end
            end
        end
    end

    local function range(i, j, ...)
        local n = select("#", ...)
        i, j = idx(i, n), idx(j, n)
        if i > j then
            return
        end
        return unpack({...}, i, j)
    end

    local function remove(i, ...)
        local n = select("#", ...)
        local t = {...}
        i = idx(i, n)
        assert(i > 0, "index out of bounds")
        if i <= n then
            n = tremove2(t, n, i)
        end
        return unpack(t, 1, n)
    end

    local function insert(v, i, ...)
        local n = select("#", ...)
        local t = {...}
        i = idx(i, n)
        assert(i > 0, "index out of bounds")
        n = tinsert2(t, n, i, v)
        return unpack(t, 1, n)
    end

    local function replace(v, i, ...)
        local n = select("#", ...)
        local t = {...}
        i = idx(i, n)
        assert(i > 0, "index out of bounds")
        t[i] = v
        n = max(n, i)
        return unpack(t, 1, n)
    end

    local function append(...)
        local n = select("#", ...)
        if n <= 1 then
            return ...
        end
        local t = {select(2, ...)}
        t[n] = (...)
        return unpack(t, 1, n)
    end

    local function map(...)
        local n = select("#", ...)
        assert(n > 0)
        local f = ...
        local t = {}
        for i = 2, n do
            t[i - 1] = f((select(i, ...)))
        end
        return unpack(t, 1, n - 1)
    end

    local function packinto(n, t, ...)
        local c = select("#", ...)
        for i = 1, c do
            t[n + i] = select(i, ...)
        end
        return n + c
    end

    local function concat(...)
        local n = 0
        local t = {}
        for i = 1, select("#", ...) do
            local f = select(i, ...)
            n = packinto(n, t, f())
        end
        return unpack(t, 1, n)
    end

    local function count(...)
        return select("#", ...)
    end

    local function at(i, ...)
        local n = select("#", ...)
        i = idx(i, n)
        if i > n then
            return
        end
        return (select(i, ...))
    end

    return setmetatable({
        pack = pack,
        range = range,
        insert = insert,
        remove = remove,
        replace = replace,
        append = append,
        map = map,
        concat = concat,
        count = count,
        at = at
    }, {
        __call = function(_, ...)
            return pack(...)
        end
    })
end

M.va = va

-- 将 Unicode 代码点转换为字符的辅助函数
local function utf8_char(code)
    if code < 0x80 then
        return string.char(code)
    elseif code < 0x800 then
        return string.char(0xC0 + math.floor(code / 0x40), 0x80 + (code % 0x40))
    elseif code < 0x10000 then
        return string.char(0xE0 + math.floor(code / 0x1000), 0x80 + (math.floor(code / 0x40) % 0x40),
            0x80 + (code % 0x40))
    elseif code < 0x200000 then
        return string.char(0xF0 + math.floor(code / 0x40000), 0x80 + (math.floor(code / 0x1000) % 0x40),
            0x80 + (math.floor(code / 0x40) % 0x40), 0x80 + (code % 0x40))
    end
end

-- 使用 \uXXXX Unicode 序列解码字符串的函数
local function decode_unicode_escape(s)
    return s:gsub("\\u(%x%x%x%x)", function(code)
        return utf8_char(tonumber(code, 16))
    end)
end

M.decode_unicode_escape = decode_unicode_escape

local prefabs = {}

local function keys(a)
    local key = {}
    for k in pairs(a) do
        key[#key + 1] = k
    end
    return key
end

local function compare_table(a, b)
    if type(a) ~= "table" then
        assert(a == b)
    else
        assert(type(b) == "table", "Not a table")
        local k = keys(a)
        assert(#k == #keys(b))
        for k, v in pairs(a) do
            local v2 = b[k]
            compare_table(v, v2)
        end
    end
end

local function load_prefabs(src)
    local pf = src
    return pf
    -- print(this_map_node[1]["engine_version"])
end

local function check(node)
    if node[1]["this_is_neko_prefab_file"] ~= nil then
        return node[1]["engine_version"]
    end
    return 0
end

local function node_type(node)
    if node[1]["file"] ~= nil then
        local file_tb = node[1]["file"]
        return file_tb["type"]
    end
    assert(false, "not a neko prefab")
    return nil
end

prefabs.load = load_prefabs
prefabs.check = check
prefabs.node_type = node_type
prefabs.compare_table = compare_table

M.prefabs = prefabs

return M

