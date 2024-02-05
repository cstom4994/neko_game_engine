-- Copyright(c) 2022-2023, KaoruXun All rights reserved.
-- runs on start of the engine
-- used to load scripts
tl = require("tl")
inspect = require("inspect")
sandbox = require("sandbox")

assert(inspect({1, 2, 3, 4}) == "{ 1, 2, 3, 4 }")
assert(inspect(1) == "1")
assert(inspect("Hello") == '"Hello"')

local safefunc = sandbox.protect([[
]])

safefunc()

function starts_with(str, start)
    return str:sub(1, #start) == start
end

function ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end
