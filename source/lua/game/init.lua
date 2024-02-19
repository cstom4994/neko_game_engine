-- Copyright(c) 2022-2023, KaoruXun All rights reserved.
-- runs on start of the engine
-- used to load scripts
local dump_func = require "common/dump"

local safefunc = function()

end

love = {}

print(dump_func({
    f = print,
    ud = safefunc,
    thread = {1, 2, 3, 4}
}))

common = require "common/common"

neko_file_path = common.memoize(function(path)
    return __neko_file_path(path)
end)

function starts_with(str, start)
    return str:sub(1, #start) == start
end

function ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end

function sleep(n)
    local t0 = os.clock()
    while os.clock() - t0 <= n do
    end
end

__NEKO_CONFIG_TYPE_INT = 0
__NEKO_CONFIG_TYPE_FLOAT = 1
__NEKO_CONFIG_TYPE_STRING = 2
