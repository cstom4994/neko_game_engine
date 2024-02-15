-- Copyright(c) 2022-2023, KaoruXun All rights reserved.
-- runs on start of the engine
-- used to load scripts
sandbox = require("sandbox")

local dump_func = require "common/dump"

local safefunc = sandbox.protect([[
]])

safefunc()

print(dump_func({
    f = print,
    ud = safefunc,
    thread = {1, 2, 3, 4}
}))

function starts_with(str, start)
    return str:sub(1, #start) == start
end

function ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end
