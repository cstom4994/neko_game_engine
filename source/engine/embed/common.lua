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

    local fn, msg = loadfile(neko_file_path("lua_scripts/" .. name), "tb", tb)
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

    local fn, msg = loadfile(neko_file_path("lua_scripts/" .. name), "tb", _G[key])
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

local b64chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
local b64 = {}
for i = 1, #b64chars do
    b64[i - 1] = b64chars:sub(i, i)
end

local function to_base64(d)
    local data = d
    local result = {}
    local padding = 3 - (#data % 3)
    if padding == 3 then
        padding = 0
    end
    data = data .. string.rep("\0", padding)
    for i = 1, #data, 3 do
        local n = data:byte(i) * 65536 + data:byte(i + 1) * 256 + data:byte(i + 2)
        table.insert(result, b64[(n >> 18) & 63])
        table.insert(result, b64[(n >> 12) & 63])
        table.insert(result, b64[(n >> 6) & 63])
        table.insert(result, b64[n & 63])
    end
    return table.concat(result):sub(1, #result - padding) .. string.rep("=", padding)
end

local b64dec = {}
for i = 1, #b64chars do
    b64dec[b64chars:sub(i, i)] = i - 1
end

local function from_base64(d)
    local data = d
    local result = {}
    data = data:gsub("[^" .. b64chars .. "=]", "")
    local padding = #data % 4
    if padding == 2 then
        data = data .. "=="
    elseif padding == 3 then
        data = data .. "="
    end
    for i = 1, #data, 4 do
        local n = b64dec[data:sub(i, i)] * 262144 + b64dec[data:sub(i + 1, i + 1)] * 4096 +
                      (b64dec[data:sub(i + 2, i + 2)] or 0) * 64 + (b64dec[data:sub(i + 3, i + 3)] or 0)
        table.insert(result, string.char((n >> 16) & 255))
        if data:sub(i + 2, i + 2) ~= "=" then
            table.insert(result, string.char((n >> 8) & 255))
        end
        if data:sub(i + 3, i + 3) ~= "=" then
            table.insert(result, string.char(n & 255))
        end
    end
    return table.concat(result)
end

M.to_base64 = to_base64
M.from_base64 = from_base64

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

return M

