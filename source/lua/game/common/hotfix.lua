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
    return "FILE:" .. name
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
                print(string.format("load [%s] ignore var [%s]. duplicate keys", name, k))
            end
            _G[key][k] = v
        end
    end
    print(string.format("hotload [%s] ok", name))
end

