local nekonode = require "neko_nekonode"

local M = {}

M.node = nekonode

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

local function load_node(src)
    local this_map_node = M.node.parse(src)
    return this_map_node
    -- print(this_map_node[1]["engine_version"])
end

local function check(node)
    if node[1]["this_is_neko_node_file"] ~= nil then
        return true
    end
    return false
end

local function node_type(node)
    if node[1]["file"] ~= nil then
        local file_tb = node[1]["file"]
        return file_tb["type"]
    end
    assert(false, "not a neko node")
    return nil
end

M.load = load_node
M.check = check
M.node_type = node_type

local test_code = [[

]]

return M
