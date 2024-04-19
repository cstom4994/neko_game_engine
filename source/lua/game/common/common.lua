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

return M

