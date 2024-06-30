local add = function(a, b)
    return a + b
end
print(common.call(add, 1, 2))

local f = common.memoize(function(a, b, c)
    return tostring(a) .. tostring(b) .. tostring(c)
end)
print(f("hello", nil, 15))
print(f("hello", nil, 15))

print(common.uuid())

local lambda = common.lambda

print(lambda "x,y -> 2*x+y"(10, 5))
print(lambda "a, b -> a / b"(1, 2))
print(lambda "v -> '你好->' .. v"("Neko"))
print(lambda "_1,_->_1.._"("te", "st"))

local cstruct = require "cstruct"

local code = [[

struct node {
	float x;
	float y;
};

struct list {
	struct node node[2];
	struct list * next;
};

]]

local s = cstruct.struct(code)

local sz = s:size "struct list"

local get_x = s:getter "struct list.node[0].x"
local get_y = s:getter "struct list.node[0].y"
local set_x = s:setter "struct list.node[0].x"
local set_y = s:setter "struct list.node[0].y"
local get_next = s:getter "struct list.next"
local set_next = s:setter "struct list.next"

local head = luastruct_test.udata(sz)

set_x(head, 0)
set_y(head, 0)
set_next(head, luastruct_test.NULL)

local last = head

for i = 1, 5 do
    local obj = luastruct_test.udata(sz)
    set_next(last, obj)
    set_x(obj, 1.1)
    set_y(obj, 2.2)
    set_next(obj, luastruct_test.NULL)
    last = obj
end

print(get_x(head))
print(get_y(head))

print(s:getter("struct list.next.next.next.node[0].x")(head))
print(s:getter("struct list.next.next.next.node[0].y")(head))

s:dump()

local function C(str)
    local t = NODE.load(str)
    return function(tbl)
        local ok, err = pcall(NODE.compare_table, t, tbl)
        if not ok then
            print("Error in :")
            print(str)
            for k, v in pairs(t) do
                print(k, v, type(v))
            end
            error(err)
        end
    end
end

local function F(str)
    local ok = pcall(NODE.load, str)
    assert(not ok)
end

C [[
---
{}
---
  --- *e001
--- &e001
light:true
]] {{{}}, {{
    light = true
}}, {
    light = true
}}

C [[
--- &1
x : 1
--- *1
--- *2
--- &2
y : &3 { 1, 2, 3}
z : *1
---
*1 *2 *3
]] {{
    x = 1
}, {
    x = 1
}, {
    y = {1, 2, 3},
    z = {
        x = 1
    }
}, {
    y = {1, 2, 3},
    z = {
        x = 1
    }
}, {{
    x = 1
}, {
    y = {1, 2, 3},
    z = {
        x = 1
    }
}, {1, 2, 3}}}

C [[
---
x : 1
y : 2
---
---
b : 2
---
hello
world
--- { 1,2,3 }
---
	---
	x : 1
	---
	y : 2
]] {{
    x = 1,
    y = 2
}, {}, {
    b = 2
}, {"hello", "world"}, {1, 2, 3}, {{
    x = 1
}, {
    y = 2
}}}