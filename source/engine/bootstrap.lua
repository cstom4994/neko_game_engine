unsafe_require = require

-- runs on start of the engine

ng = setmetatable({}, {
    __index = neko
})

ng.api = neko

-- 命令行参数
ng.args = nekogame_args

-- ffi = unsafe_require("ffi")

if table.unpack == nil then
    table.unpack = unpack
end

------------------------------------------------------------------

common = {}

local pairs, ipairs = pairs, ipairs
local type, assert, unpack = type, assert, unpack or table.unpack
local tostring, tonumber = tostring, tonumber
local math_floor = math.floor
local math_ceil = math.ceil
local math_atan2 = math.atan2 or math.atan
local math_sqrt = math.sqrt
local math_abs = math.abs

common.neko_pi = 3.14159265358979323846264

local is_callable = function(x)
    if type(x) == "function" then
        return true
    end
    local mt = getmetatable(x)
    return mt and mt.__call ~= nil
end

function common.distance(x1, y1, x2, y2, squared)
    local dx = x1 - x2
    local dy = y1 - y2
    local s = dx * dx + dy * dy
    return squared and s or math_sqrt(s)
end

function common.random(a, b)
    if not a then
        a, b = 0, 1
    end
    if not b then
        b = 0
    end
    return a + math.random() * (b - a)
end

function common.random_table(t)
    return t[math.random(#t)]
end

function common.once(fn, ...)
    local f = common.fn(fn, ...)
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

function common.memoize(fn)
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

function common.fn(fn, ...)
    assert(is_callable(fn), "expected a function as the first argument")
    local args = {...}
    return function(...)
        local a = common.concat(args, {...})
        return fn(unpack(a))
    end
end

function common.call(fn, ...)
    if fn then
        return fn(...)
    end
end

function common.dostring(str)
    return assert((loadstring or load)(str))()
end

local lambda_cache = {}

function common.lambda(str)
    if not lambda_cache[str] then
        local args, body = str:match([[^([%w,_ ]-)%->(.-)$]])
        assert(args and body, "bad string lambda")
        local s = "return function(" .. args .. ")\nreturn " .. body .. "\nend"
        lambda_cache[str] = common.dostring(s)
    end
    return lambda_cache[str]
end

function common.uuid()
    local fn = function(x)
        local r = math.random(16) - 1
        r = (x == "x") and (r + 1) or (r % 4) + 9
        return ("0123456789abcdef"):sub(r, r)
    end
    return (("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"):gsub("[xy]", fn))
end

function common.rad2deg(__R)
    return (__R * 180.0) / common.neko_pi
end

function common.str2bytes(str)
    local bytes = {}
    local length = string.len(str)
    for i = 1, length do
        local s = string.sub(str, i, i)
        local b = string.byte(s)
        table.insert(bytes, b)
    end
    return bytes
end

function common.bytes2str(bytes)
    return string.char(table.unpack(bytes))
end

local cdata = {}

-- function cdata:new_struct(name, struct)
--     ffi.cdef(struct)

--     self.structs = self.structs or {}
--     self.structs[name] = ffi.typeof(name)

--     self.pointers = self.pointers or {}
--     self.pointers[name] = ffi.typeof(name .. "*")
-- end

-- function cdata:set_struct(name, data)
--     return ffi.new(self.structs[name], data)
-- end

-- function cdata:encode(data)
--     return ffi.string(ffi.cast("const char*", data), ffi.sizeof(data))
-- end

-- function cdata:decode(name, data)
--     return ffi.cast(self.pointers[name], data)[0]
-- end

common.cdata = cdata

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

common.deepcopy = deepcopy

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

common.behavior = behavior

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

common.utils = utils

-- va
common.va = function()
    local va = {}

    -- 将两个表连接起来，返回一个新的表
    function va.concat(t1, t2)
        local result = {}
        for i = 1, #t1 do
            result[#result + 1] = t1[i]
        end
        for i = 1, #t2 do
            result[#result + 1] = t2[i]
        end
        return result
    end

    setmetatable(va, {
        __call = function(_, ...) -- 将变长参数捕获为一个表
            return {...}
        end
    })

    return va
end

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

common.decode_unicode_escape = decode_unicode_escape

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

common.prefabs = prefabs

------------------------------------------------------------------

__print = print

function table.show(t, name, indent)
    local cart -- 一个容器
    local autoref -- 供自我参考
    local function isemptytable(t)
        return next(t) == nil
    end
    local function basicSerialize(o)
        local so = tostring(o)
        if type(o) == "function" then
            local info = debug.getinfo(o, "S")
            -- info.name is nil because o is not a calling level
            if info.what == "C" then
                return string.format("%q", so .. ", C function")
            else
                -- the information is defined through lines
                return string.format("%q", so .. ", defined in (" .. info.linedefined .. "-" .. info.lastlinedefined ..
                    ")" .. info.source)
            end
        elseif type(o) == "number" or type(o) == "boolean" then
            return so
        else
            return string.format("%q", so)
        end
    end

    local function addtocart(value, name, indent, saved, field)
        indent = indent or ""
        saved = saved or {}
        field = field or name

        cart = cart .. indent .. field

        if type(value) ~= "table" then
            cart = cart .. " = " .. basicSerialize(value) .. ";\n"
        else
            if saved[value] then
                cart = cart .. " = {}; -- " .. saved[value] .. " (self reference)\n"
                autoref = autoref .. name .. " = " .. saved[value] .. ";\n"
            else
                saved[value] = name
                -- if tablecount(value) == 0 then
                if isemptytable(value) then
                    cart = cart .. " = {};\n"
                else
                    cart = cart .. " = {\n"
                    for k, v in pairs(value) do
                        k = basicSerialize(k)
                        local fname = string.format("%s[%s]", name, k)
                        field = string.format("[%s]", k)
                        -- three spaces between levels
                        addtocart(v, fname, indent .. "   ", saved, field)
                    end
                    cart = cart .. indent .. "};\n"
                end
            end
        end
    end

    name = name or "__unnamed__"
    if type(t) ~= "table" then
        return name .. " = " .. basicSerialize(t)
    end
    cart, autoref = "", ""
    addtocart(t, name, indent)
    return cart .. autoref
end

print = function(...)
    local print_func = neko.print
    local tb = {...}
    local n = select("#", ...)
    if n == 1 and type(tb[1]) == "table" then
        print_func(table.show(tb))
    else
        print_func(...)
    end
end

default_require = require

-- require
function hot_require(name)
    local loaded = package.loaded[name]
    if loaded ~= nil then
        return loaded
    end
    local preload = package.preload[name]
    if preload ~= nil then
        return neko.__registry_load(name, preload(name))
    end
    local path = name:gsub("%.", "/")
    if path:sub(-4) ~= ".lua" then
        path = path .. ".lua"
    end
    local ret_tb = neko.__registry_lua_script(path)
    if ret_tb ~= nil then
        return table.unpack(ret_tb)
    end
end

-- default callbacks

function neko.__start(arg)
    if arg[#arg] == "-mobdebug" then
        unsafe_require"mobdebug".start()
    end
    if os.getenv "LOCAL_LUA_DEBUGGER_VSCODE" == "1" then
        unsafe_require"lldebugger".start()
        print("LOCAL_LUA_DEBUGGER_VSCODE=1")
    end
end

function neko.__define_default_callbacks()
    function neko.arg(arg)
        local usage = false
        local version = false
        local console = false

        for _, a in ipairs(arg) do
            if a == "--help" or a == "-h" then
                usage = true
            elseif a == "--version" or a == "-v" then
                version = true
            elseif a == "--console" then
                console = true
            end
        end

        if usage then
            local str = ([[usage:
    %s [command...]
  commands:
    --help, -h                  show this usage
    --version, -v               show neko version
    --console                   windows only. use console output
  ]]):format(neko.program_path())

            print(str)
            os.exit()
        end

        if version then
            print(neko.version())
            os.exit()
        end

        neko.set_console_window(console)
    end

    function neko.conf()
    end

    function neko.start()
    end

    function neko.before_quit()
    end

    function neko.frame(dt)
        if neko.key_down "esc" then
            neko.quit()
        end

        if default_font == nil then
            default_font = neko.default_font()
        end

        local text = "- no game! -"
        local text_size = 36
        local x = (neko.window_width() - default_font:width(text, text_size)) / 2
        local y = (neko.window_height() - text_size) * 0.45

        default_font:draw(text, x, y, text_size)
    end
end

neko.file_path = common.memoize(function(path)
    return neko.program_dir() .. path
end)

function starts_with(str, start)
    return str:sub(1, #start) == start
end

function ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end

function neko_sleep(n)
    local t0 = os.clock()
    while os.clock() - t0 <= n do
    end
end

function to_vec2(_x, _y)
    return {
        x = _x,
        y = _y
    }
end

function to_color(r, g, b, a)
    return {
        r = r,
        g = g,
        b = b,
        a = a
    }
end

function easeOutCubic(x)
    return 1 - math.pow(1 - x, 3)
end

function read_file(filename)
    local file = io.open(filename, "r")
    local content
    if not file then
        content = neko.vfs_read_file(filename)
        if content ~= nil then
            return content
        end
        return nil
    else
        content = file:read("*all")
        file:close()
        return content
    end
end

function table_merge(t1, t2)
    for k, v in pairs(t2) do
        if type(v) == "table" then
            if type(t1[k] or false) == "table" then
                table_merge(t1[k] or {}, t2[k] or {})
            else
                t1[k] = v
            end
        else
            t1[k] = v
        end
    end
    return t1
end

function neko_ls(path)
    local result = {}
    local tb = __neko_ls(path)
    for _, tb1 in pairs(tb) do
        if tb1["isDirectory"] == true then
            result = table_merge(result, neko_ls(tb1["path"]))
        else
            table.insert(result, tb1["path"])
        end
    end
    return result
end

__NEKO_CONFIG_TYPE_INT = 0
__NEKO_CONFIG_TYPE_FLOAT = 1
__NEKO_CONFIG_TYPE_STRING = 2

comp_src = [[
#version 430 core
uniform float u_roll;
layout(rgba32f, binding = 0) uniform image2D destTex;
layout (std430, binding = 1) readonly buffer u_voxels {
    vec4 data;
};
layout (local_size_x = 16, local_size_y = 16) in;
void main() {
ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8 ) / 8.0);
float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + u_roll) * 0.5;
vec4 rc = vec4(1.0 - globalCoef * localCoef, globalCoef * localCoef, 0.0, 1.0);
vec4 color = mix(rc, data, 0.5f);
imageStore(destTex, storePos, color);
}
]]

LUA_RIDX_MAINTHREAD = 1
LUA_RIDX_GLOBALS = 2

-- object oriented

Object = {}
Object.__index = Object

function Object:__call(...)
    local obj = setmetatable({}, self)
    if obj.new ~= nil then
        obj:new(...)
    end
    return obj
end

function Object:is(T)
    local mt = getmetatable(self)
    while mt ~= nil do
        if mt == T then
            return true
        end
        mt = getmetatable(mt)
    end
    return false
end

function class(name, parent)
    parent = parent or Object

    local cls = {}

    for k, v in pairs(parent) do
        if k:sub(1, 2) == '__' then
            cls[k] = v
        end
    end

    cls.super = parent

    function cls:__index(key)
        return rawget(_G, name)[key]
    end

    rawset(_G, name, setmetatable(cls, parent))
end

function newproxy(new_meta)
    local proxy = {}

    if (new_meta == true) then
        local mt = {}
        setmetatable(proxy, mt)
    elseif (new_meta == false) then
        -- new_meta must have a metatable.
        local mt = getmetatable(new_meta)
        setmetatable(proxy, mt)
    end

    return proxy
end

function classnew_ctor(ctor_method_name)
    ctor_method_name = ctor_method_name or "__new"
    assert(type(ctor_method_name == "string"))

    local classnew = function(tbl, ...)
        tbl.__index = tbl.__index or tbl
        local inst = {}
        -- dtor (with the name __gc)
        -- this works with types defined in lua tables in 5.1 via "newproxy(true)" hack
        if tbl["__gc"] then
            local proxy_for_gc = newproxy(true)
            getmetatable(proxy_for_gc).__gc = function()
                local ok, err = pcall(tbl.__gc, inst)
                if not ok then
                    print(err)
                end
            end
            inst[proxy_for_gc] = true
        end
        inst = setmetatable(inst, tbl)
        -- ctor (with the given name. __new if nothing given)
        if tbl[ctor_method_name] then
            local ok, err = pcall(tbl[ctor_method_name], inst, ...)
            if not ok then
                print(err)
                error(err)
            end
        end
        return inst
    end

    return classnew
end

classnew = classnew_ctor("__new")

-- 2d vector

class "vec2"

function vec2:new(x, y)
    self.x = x
    self.y = y
end

function vec2:normalize()
    local x = self.x
    local y = self.y

    local len = math.sqrt(x * x + y * y)
    if len == 0 then
        return vec2(0, 0)
    end

    return vec2(x / len, y / len)
end

function vec2:distance(rhs)
    local dx = rhs.x - self.x
    local dy = rhs.y - self.y
    return math.sqrt(dx * dx + dy * dy)
end

function vec2:direction(rhs)
    local dx = rhs.x - self.x
    local dy = rhs.y - self.y
    return math.atan(dy, dx)
end

function vec2:lerp(rhs, t)
    local x = self.x
    local y = self.y
    return vec2(x + (rhs.x - x) * t, y + (rhs.y - y) * t)
end

function vec2:dot(rhs)
    return self.x * rhs.x + self.y * rhs.y
end

function vec2:unpack()
    return self.x, self.y
end

function vec2.__add(lhs, rhs)
    return vec2(lhs.x + rhs.x, lhs.y + rhs.y)
end

function vec2.__sub(lhs, rhs)
    return vec2(lhs.x - rhs.x, lhs.y - rhs.y)
end

function vec2.__mul(lhs, rhs)
    return vec2(lhs.x * rhs.x, lhs.y * rhs.y)
end

function vec2.__div(lhs, rhs)
    return vec2(lhs.x / rhs.x, lhs.y / rhs.y)
end

function vec2:__len()
    local x = self.x
    local y = self.y
    return math.sqrt(x * x + y * y)
end

function vec2.__eq(lhs, rhs)
    return lhs.x == rhs.x and lhs.y == rhs.y
end

function vec2:__tostring()
    return "vec2(" .. self.x .. ", " .. self.y .. ")"
end

-- group of entities in a world

class "World"

function World:new()
    self.next_id = 1
    self.by_id = {}
    self.by_mt = {}
    self.to_create = {}
    self.to_kill = {}
end

function World:destroy_all()
    for id, obj in pairs(self.by_id) do
        if obj.on_death then
            obj:on_death()
        end
    end
end

function World:add(obj)
    obj.id = self.next_id

    if obj.z_index == nil then
        obj.z_index = 0
    end

    self.to_create[self.next_id] = obj
    self.next_id = self.next_id + 1
    return obj
end

function World:kill(id)
    local obj
    if type(id) == "table" then
        obj = id
    else
        obj = self.by_id[id]
    end

    if obj ~= nil then
        self.to_kill[obj.id] = obj
    end
end

function World:update(dt)
    for id, obj in pairs(self.to_create) do
        local mt = getmetatable(obj)
        if self.by_mt[mt] == nil then
            self.by_mt[mt] = {}
        end

        self.by_mt[mt][id] = obj
        self.by_id[id] = obj
        self.to_create[id] = nil

        if obj.on_create then
            obj:on_create()
        end
    end

    for id, obj in pairs(self.by_id) do
        obj:update(dt)
    end

    for id, obj in pairs(self.to_kill) do
        if obj.on_death then
            obj:on_death()
        end

        local mt = getmetatable(obj)
        self.by_mt[mt][id] = nil
        self.by_id[id] = nil
        self.to_kill[id] = nil
    end
end

local function _sort_z_index(lhs, rhs)
    return lhs.z_index < rhs.z_index
end

local function _sort_y(lhs, rhs)
    return lhs.y > rhs.y
end

function World:draw()
    local sorted = {}
    for id, obj in pairs(self.by_id) do
        sorted[#sorted + 1] = obj
    end

    -- table.sort(sorted, _sort_z_index)
    table.sort(sorted, _sort_y)

    for k, obj in ipairs(sorted) do
        obj:draw()
    end
end

function World:query_id(id)
    return self.by_id[id]
end

function World:query_mt(mt)
    if self.by_mt[mt] == nil then
        self.by_mt[mt] = {}
    end

    return self.by_mt[mt]
end

-- entity component system

class "ECS"

function ECS:new()
    self.next_id = 1
    self.entities = {}
    self.to_create = {}
    self.to_kill = {}
end

function ECS:update()
    for id, entity in pairs(self.to_create) do
        self.entities[id] = entity
        self.to_create[id] = nil
    end

    for id, entity in pairs(self.to_kill) do
        self.entities[id] = nil
        self.to_kill[id] = nil
    end
end

function ECS:add(entity)
    local id = self.next_id
    self.to_create[id] = entity
    self.next_id = id + 1
    return id, entity
end

function ECS:kill(id)
    self.to_kill[id] = true
end

function ECS:get(id)
    return self.entities[id]
end

function ECS:query(t)
    local rows = {}

    for id, entity in pairs(self.entities) do
        if self.to_kill[id] ~= nil then
            goto continue
        end

        local missing = false
        for i, key in pairs(t.select) do
            if entity[key] == nil then
                missing = true
                break
            end
        end

        if missing then
            goto continue
        end

        if t.where ~= nil then
            if not t.where(entity) then
                goto continue
            end
        end

        rows[id] = entity

        ::continue::
    end

    if t.order_by == nil then
        return pairs(rows)
    end

    local tied = {}
    for id, entity in pairs(rows) do
        tied[#tied + 1] = {
            id = id,
            entity = entity
        }
    end
    table.sort(tied, t.order_by)

    local i = 0
    return function()
        i = i + 1
        local el = tied[i]
        if el ~= nil then
            return el.id, el.entity
        end
    end
end

function ECS:select(columns)
    return self:query{
        select = columns
    }
end

-- bouncing spring

class "Spring"

function Spring:new(k, d)
    self.stiffness = k or 400
    self.damping = d or 28
    self.x = 0
    self.v = 0
end

function Spring:update(dt)
    local a = -self.stiffness * self.x - self.damping * self.v
    self.v = self.v + a * dt
    self.x = self.x + self.v * dt
end

function Spring:pull(f)
    self.x = self.x + f
end

-- intervals/timeouts

local timer = {}

timer.next_id = 0
timer.intervals = {}
timer.timeouts = {}

function neko.__timer_update(dt)
    for id, t in pairs(timer.intervals) do
        t.elapsed = t.elapsed + dt
        if t.elapsed >= t.seconds then
            t.elapsed = t.elapsed - t.seconds
            t.callback()
        end
    end

    for id, t in pairs(timer.timeouts) do
        t.elapsed = t.elapsed + dt
        if t.elapsed >= t.seconds then
            t.elapsed = t.elapsed - t.seconds
            t.callback()
            timer.timeouts[id] = nil
        end
    end
end

function interval(sec, action)
    local id = timer.next_id

    timer.intervals[id] = {
        callback = action,
        seconds = sec,
        elapsed = 0
    }
    timer.next_id = timer.next_id + 1

    return id
end

function timeout(sec, action)
    local id = timer.next_id

    timer.timeouts[id] = {
        callback = action,
        seconds = sec,
        elapsed = 0
    }
    timer.next_id = timer.next_id + 1

    return id
end

function stop_interval(id)
    if timer.intervals[id] ~= nil then
        timer.intervals[id] = nil
    end
end

function stop_timeout(id)
    if timer.timeouts[id] ~= nil then
        timer.timeouts[id] = nil
    end
end

-- utility functions

local function _stringify(value, visited, indent)
    local T = type(value)
    if T == "table" then
        local mt = getmetatable(value)
        if mt ~= nil and rawget(mt, "__tostring") then
            return tostring(value)
        elseif visited[value] then
            return tostring(value)
        end

        visited[value] = true
        local white = string.rep(" ", indent + 2)
        local s = "{\n"
        for k, v in pairs(value) do
            s = s .. white .. tostring(k) .. " = " .. _stringify(v, visited, indent + 2) .. ",\n"
        end

        return s .. string.rep(" ", indent) .. "}"
    elseif T == "string" then
        return "'" .. value .. "'"
    else
        return tostring(value)
    end
end

function stringify(value)
    return _stringify(value, {}, 0)
end

function clamp(n, min, max)
    if n < min then
        return min
    end
    if n > max then
        return max
    end
    return n
end

function sign(x)
    if x < 0 then
        return -1
    end
    if x > 0 then
        return 1
    end
    return x
end

function lerp(src, dst, t)
    return src + (dst - src) * t
end

function direction(x0, y0, x1, y1)
    local dx = x1 - x0
    local dy = y1 - y0
    return math.atan(dy, dx)
end

function heading(angle, mag)
    local x = math.cos(angle) * mag
    local y = math.sin(angle) * mag
    return x, y
end

function delta_angle(src, dst)
    local tau = math.pi * 2
    return (dst - src + math.pi) % tau - math.pi
end

function distance(x0, y0, x1, y1)
    local dx = x1 - x0
    local dy = y1 - y0
    return math.sqrt(dx * dx + dy * dy)
end

function normalize(x, y)
    local len = math.sqrt(x * x + y * y)
    if len == 0 then
        return 0, 0
    end

    return x / len, y / len
end

function dot(x0, y0, x1, y1)
    return x0 * x1 + y0 * y1
end

function random(min, max)
    return math.random() * (max - min) + min
end

function aabb_overlap(ax0, ay0, ax1, ay1, bx0, by0, bx1, by1)
    if bx1 == nil then
        bx1 = bx0
    end
    if by1 == nil then
        by1 = by0
    end

    return ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1
end

function rect_overlap(ax, ay, aw, ah, bx, by, bw, bh)
    if bw == nil then
        bw = 0
    end
    if bh == nil then
        bh = 0
    end

    return ax < (bx + bw) and bx < (ax + aw) and ay < (by + bh) and by < (ay + ah)
end

function clone(t)
    local tab = {}
    for k, v in pairs(t) do
        tab[k] = v
    end
    return tab
end

function push(arr, x)
    arr[#arr + 1] = x
end

function map(arr, fn)
    local t = {}
    for k, v in pairs(arr) do
        t[k] = fn(v)
    end
    return t
end

function filter(arr, fn)
    local t = {}
    for k, v in ipairs(arr) do
        if fn(v) then
            t[#t + 1] = v
        end
    end
    return t
end

function zip(lhs, rhs)
    local len = math.min(#lhs, #rhs)
    local t = {}
    for i = 1, len do
        t[i] = {lhs[i], rhs[i]}
    end
    return t
end

function choose(arr)
    return arr[math.random(#arr)]
end

function find(arr, x)
    for k, v in pairs(arr) do
        if v == x then
            return k
        end
    end
end

function sortpairs(t)
    local keys = {}
    for k in pairs(t) do
        keys[#keys + 1] = k
    end
    table.sort(keys)

    local i = 0
    return function()
        i = i + 1
        local k = keys[i]
        return k, t[k]
    end
end

function resume(co, ...)
    local ok, err = coroutine.resume(co, ...)
    if not ok then
        error(err, 2)
    end
end

function sleep(secs)
    while secs > 0 do
        secs = secs - neko.dt()
        coroutine.yield()
    end

    return neko.dt()
end

if math.pow == nil then
    math.pow = function(a, b)
        return a ^ b
    end
end

function intdiv(a, b)
    return math.floor(a / b)
end

function common.unpack(str)
    local func_str = "return " .. str
    local func = loadstring(func_str)
    return func()
end

local function serialize(obj)
    local lua = ""
    local t = type(obj)
    if t == "number" then
        lua = lua .. obj
    elseif t == "boolean" then
        lua = lua .. tostring(obj)
    elseif t == "string" then
        lua = lua .. string.format("%q", obj)
    elseif t == "table" then
        lua = lua .. "{"
        for k, v in pairs(obj) do
            lua = lua .. "[" .. serialize(k) .. "]=" .. serialize(v) .. ","
        end
        local metatable = getmetatable(obj)
        if metatable ~= nil and type(metatable.__index) == "table" then
            for k, v in pairs(metatable.__index) do
                lua = lua .. "[" .. serialize(k) .. "]=" .. serialize(v) .. ","
            end
        end
        lua = lua .. "}"
    elseif t == "nil" then
        return "nil"
    elseif t == "userdata" then
        return "userdata"
    elseif t == "function" then
        return "function"
    elseif t == "thread" then
        return "thread"
    else
        error("can not serialize a " .. t .. " type.")
    end
    return lua
end

function ImGuiWindow(name, flags)
    flags = flags or nil
    if neko.imgui.Begin(name, nil, flags) then
        return setmetatable({}, {
            __close = function()
                neko.imgui.End()
            end
        })
    end
end

common.pack = serialize

local cdata_init = function()
    local M = {}
    M.__CORE = neko.__cdata()
    local function parse_struct(code)
        local nest = {}
        local nest_n = 0
        code = code:gsub("(%b{})", function(v)
            nest_n = nest_n + 1
            nest[nest_n] = v
            return "{" .. nest_n .. "} "
        end)
        local names = {}
        local lines = {}
        local line_n = 0
        for line in code:gmatch "%s*(.-)%s*;" do
            line_n = line_n + 1
            line = line:gsub("%s+", " ")
            line = line:gsub(" ?%*%s*", " *")
            local prefix, array = line:match "^(.-)%s*(%b[])$"
            if array then
                array = math.tointeger(array:match "%[(%d+)%]") or 0
                line = prefix
            end
            local typestr, pointer, name = line:match "^(.-) (%**)([_%w]+)$"
            assert(typestr, line)
            local type_prefix, subtype = typestr:match "^([%w_]+)%s+(.+)"
            if type_prefix == "struct" or type_prefix == "union" then
                typestr = type_prefix
                local nesttypeid = subtype:match "^{(%d+)}$"
                if nesttypeid then
                    local nestcontent = assert(nest[tonumber(nesttypeid)]):match "^{(.*)}$"
                    subtype = parse_struct(nestcontent)
                    subtype.type = type_prefix
                end
            end
            if pointer == "" then
                pointer = nil
            end
            local t = {
                array = array,
                type = typestr,
                subtype = subtype,
                pointer = pointer,
                name = name
            }
            assert(names[name] == nil, name)
            names[name] = true
            lines[line_n] = t
        end

        return lines
    end

    local function parse(what, code, types)
        for typename, content in code:gmatch(what .. "%s+([_%w]+)%s*(%b{})%s*;") do
            assert(types[typename] == nil)
            local s = parse_struct(content:match "^{%s*(.-)%s*}$")
            s.type = what
            s.name = what .. " " .. typename
            types[s.name] = s
        end
    end

    local buildin_types = (function(map)
        local r = {}
        for k, v in pairs(map) do
            if type(k) == "number" then
                r[v] = true
            else
                r[k] = v
            end
        end
        return r
    end) {
        int = "int32_t",
        short = "int16_t",
        char = "int8_t",
        ["unsigned char"] = "uint8_t",
        ["unsigned short"] = "uint16_t",
        ["unsigned int"] = "uint32_t",
        "float",
        "double",
        "void",
        "bool",
        "int8_t",
        "int16_t",
        "int32_t",
        "int64_t",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        ["i8"] = "int8_t",
        ["i16"] = "int16_t",
        ["i32"] = "int32_t",
        ["i64"] = "int64_t",
        ["u8"] = "uint8_t",
        ["u16"] = "uint16_t",
        ["u32"] = "uint32_t",
        ["u64"] = "uint64_t"
    }

    local buildin_size = {
        int8_t = 1,
        int16_t = 2,
        int32_t = 4,
        int64_t = 8,
        uint8_t = 1,
        uint16_t = 2,
        uint32_t = 4,
        uint64_t = 8,
        float = 4,
        double = 8,
        ptr = 8,
        bool = 1
    }

    local buildin_id = M.__CORE.ctype()

    for k, v in pairs(buildin_types) do
        if v ~= true then -- 如果k不是原始类型(即为别名)
            buildin_size[k] = buildin_size[v] -- 复制k的原类型大小
            buildin_id[k] = buildin_id[v] -- 复制k的原类型id
        end
    end

    local function check_types(types)
        for k, t in pairs(types) do
            for idx, f in ipairs(t) do
                local typename = f.type
                if typename == "struct" or typename == "union" then
                    if type(f.subtype) == "string" then
                        local fullname = typename .. " " .. f.subtype
                        local subtype = types[fullname]
                        if not subtype then
                            error("Unknown " .. fullname)
                        end
                        assert(subtype.type == typename)
                        f.subtype = subtype
                    end
                else
                    if not buildin_types[typename] then
                        error("Unknown " .. typename)
                    end
                end
                if f.array == 0 and t[idx + 1] then
                    error("Array " .. f.name .. "[] must be the last field")
                end
            end
        end
    end

    local function calc_offset(types)
        local solve

        local function calc_align(t)
            local align = 0
            for _, f in ipairs(t) do
                if f.pointer then
                    f.size = buildin_size.ptr
                    f.align = f.size
                elseif f.subtype then
                    local subtype = solve(f.subtype)
                    f.size = subtype.size
                    f.align = subtype.align
                    if subtype.align > align then
                        align = subtype.align
                    end
                else
                    f.size = assert(buildin_size[f.type])
                    f.align = f.size
                    if f.align > align then
                        align = f.align
                    end
                end
                if f.array then
                    f.size = f.size * f.array
                end
            end
            return align
        end

        local function solve_struct(t)
            t.align = calc_align(t)
            local size = 0
            for _, f in ipairs(t) do
                if size % f.align ~= 0 then
                    size = (size // f.align + 1) * f.align
                end
                f.offset = size
                size = size + f.size
            end
            if size % t.align ~= 0 then
                size = (size // t.align + 1) * t.align
            end
            t.size = size
        end
        local function solve_union(t)
            t.align = align(t)
            local size = 0 -- 取最大的成员大小
            for _, f in ipairs(t) do
                f.offset = 0
                if f.size > size then
                    size = f.size
                end
            end
            t.size = size
        end
        do -- 解决局部函数
            local unsolved = {}
            local solved = {}
            function solve(t)
                local fullname = t.name
                if fullname then
                    if solved[fullname] then
                        return solved[fullname]
                    end
                    assert(not unsolved[fullname])
                    unsolved[fullname] = true
                end

                if t.type == "struct" then
                    solve_struct(t)
                else
                    solve_union(t)
                end

                if fullname then
                    solved[fullname] = t
                    unsolved[fullname] = nil
                end
                return t
            end
        end

        for k, t in pairs(types) do
            solve(t)
        end

        local function solve_pointer_size(t)
            for _, f in ipairs(t) do
                if f.pointer then
                    assert(f.pointer == "*")
                    if f.subtype then
                        f.pointer_size = f.subtype.size
                    else
                        f.pointer_size = buildin_size[f.type]
                    end
                end
            end
        end

        for k, t in pairs(types) do
            solve_pointer_size(t)
        end
    end

    local function keys_lookup(t)
        local keys = {}
        for _, f in ipairs(t) do
            keys[f.name] = f
        end
        t.keys = keys
        return keys
    end

    local function find_key(t, key)
        local keys = t.keys or keys_lookup(t)
        return assert(keys[key], key)
    end

    local function gen_check(types, k)
        local keys = {}
        local n = 0
        for name in k:gmatch "[^.]*" do
            n = n + 1
            keys[n] = name
        end
        local t = types[keys[1]]
        if t == nil then
            error(keys[1] .. " undefined")
        end
        local offset = {}
        local last_offset = 0
        local offset_n = 1
        local i = 2
        local typename
        while i <= n do
            local name = keys[i]
            local array_name, array_index = name:match "(.+)%[(%d+)]$"
            name = array_name or name

            local f = find_key(t, name)
            offset[offset_n] = last_offset + f.offset
            if f.pointer then
                assert(f.pointer == "*") -- todo: support "**"
                offset_n = offset_n + 1
                last_offset = 0
                typename = "ptr"
            elseif f.subtype then
                last_offset = last_offset + f.offset
                t = f.subtype
                assert(i ~= n)
            else
                assert(i == n)
                typename = f.type
            end

            if array_index then
                local index = tonumber(array_index)
                if f.pointer then
                    offset[offset_n] = index * f.pointer_size
                    offset_n = offset_n + 1
                else
                    last_offset = last_offset + index * f.size
                    offset[offset_n] = last_offset
                end
            end

            i = i + 1
        end
        local type_id = buildin_id[typename]
        if type_id ~= nil and type_id.typeid >= 0 then
            local getter, setter = M.__CORE.parser(type_id.typeid, table.unpack(offset))
            return {getter, setter}
        else
            error("unknown cdata type: " .. typename)
            return {
                getter = function(...)
                    error("unimplemented")
                end,
                setter = function(...)
                    error("unimplemented")
                end
            }
        end

    end

    local function check(types)
        local function cache_check(self, k)
            local v = gen_check(types, k)
            self[k] = v
            return v
        end
        return setmetatable({}, {
            __index = cache_check
        })
    end

    local methods = {};
    methods.__index = methods

    function methods:dump()
        for _, s in pairs(self._types) do
            print(s.name, "size", s.size, "align", s.align)
            for _, f in ipairs(s) do
                local array = ""
                if f.array then
                    array = "[" .. f.array .. "]"
                end
                local typename = f.type
                if f.subtype then
                    typename = f.subtype.name or ("nest " .. f.subtype.type)
                end
                print(string.format("\t%3d : %s %s%s%s", f.offset, typename, (f.pointer or ""), f.name, array))
            end
        end
    end

    function methods:size(name)
        local t = assert(self._types[name])
        return t.size
    end

    function methods:getter(name)
        return self._check[name][1]
    end

    function methods:setter(name)
        return self._check[name][2]
    end

    function M.struct(code)
        local types = {}
        parse("struct", code, types)
        parse("union", code, types)
        check_types(types)
        calc_offset(types)

        local obj = {
            _types = types,
            _check = check(types)
        }

        return setmetatable(obj, methods)
    end

    return M
end

neko.cdata = cdata_init()

print("lua startup")
