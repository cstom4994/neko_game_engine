unsafe_require = require

-- runs on start of the engine
luadb = require("__neko.luadb")
Core = require("__neko.core")
Inspector = require("__neko.inspector")
FFI = require("ffi")
common = require "common"

-- FLECS = require "flecs"

-- ffi = unsafe_require("ffi")

__print = print
print = Core.print

dump_func = function(tbl, indent)
    if not indent then
        indent = 0
        print("|inspect: \"" .. tostring(tbl) .. "\"")
    end
    for k, v in pairs(tbl) do
        formatting = string.rep("  ", indent) .. k .. ": "
        if type(v) == "table" then
            print("|" .. formatting)
            dump_func(v, indent + 1)
        elseif type(v) == 'boolean' then
            print("|" .. formatting .. tostring(v))
        else
            print("|" .. formatting .. v)
        end
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
        content = Core.vfs_read_file(filename)
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

sprite_vs = [[
#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float use_texture;

uniform mat4 tiled_sprite_camera;

out VS_OUT {
	vec4 color;
	vec2 uv;
	float use_texture;
} vs_out;

void main() {
	vs_out.color = color;
	vs_out.uv = uv;
	vs_out.use_texture = use_texture;

	gl_Position = tiled_sprite_camera * vec4(position, 0.0, 1.0);
}
]]

sprite_fs = [[
#version 330

out vec4 color;

in VS_OUT {
    vec4 color;
    vec2 uv;
    float use_texture;
} fs_in;

uniform sampler2D batch_texture;

void main() {
    vec4 texture_color = vec4(1.0);

    if (fs_in.use_texture == 1.0) {
        texture_color = texture(batch_texture,  fs_in.uv);
    }

    color = fs_in.color * texture_color;
}
]]

batch_vs = [[
#version 330

uniform mat4 u_mvp;
in vec2 in_pos; in vec2 in_uv;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    gl_Position = u_mvp * vec4(in_pos, 0, 1);
}
]]

batch_ps = [[
#version 330

precision mediump float;
uniform sampler2D u_sprite_texture;
in vec2 v_uv; out vec4 out_col;

void main() { out_col = texture(u_sprite_texture, v_uv); }
]]

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

custom_sprite_vs = [[
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
precision mediump float;
out vec2 uv;
void main()
{
   gl_Position = vec4(a_pos, 0.0, 1.0);
   uv = a_uv;
}
]]

custom_sprite_fs = [[
#version 330 core
precision mediump float;
uniform sampler2D u_tex;
in vec2 uv;
out vec4 frag_color;
void main()
{
   frag_color = texture(u_tex, uv);
}
]]

font_vs = [[
#version 330
uniform mat4 u_mvp;
in vec2 in_pos; in vec2 in_uv;
out vec2 v_uv;
void main() {
    v_uv = in_uv;
    gl_Position = u_mvp * vec4(in_pos, 0, 1);
}
]]

font_ps = [[
#version 330
precision mediump float;
uniform sampler2D u_sprite_texture;
in vec2 v_uv; out vec4 out_col;
void main() { out_col = texture(u_sprite_texture, v_uv); }
]]

LUA_RIDX_MAINTHREAD = 1
LUA_RIDX_GLOBALS = 2

--[[

sandbox = {}
local worlds = {}

function sandbox.fetch_world(name)
    local w = worlds[name]
    if not w then
        w = neko.ecs_create()
        worlds[name] = w
    end
    return w
end

function sandbox.select(ecs_world, func, ...)
    local w
    if type(ecs_world) == "string" then
        w = worlds[ecs_world]
    else
        w = ecs_world
    end
    for c in w:match("all", ...) do
        func(c)
    end
end

]]

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
    return lhs.y < rhs.y
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

print("lua startup")
