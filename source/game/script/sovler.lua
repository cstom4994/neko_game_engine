class "solver"

-- 将中文变量名映射为安全名
local function make_safe_var(index)
    return "__GEN_VALUE_" .. index
end

-- 解析表达式并建立依赖图 同时生成变量映射
function solver:expr(expr_list)
    local expr = {
        rules = {},
        deps = {},
        name2alias = {},
        alias2name = {}
    }

    local var_index = 0

    local function get_alias(var)
        if not expr.name2alias[var] then
            var_index = var_index + 1
            local alias = make_safe_var(var_index)
            expr.name2alias[var] = alias
            expr.alias2name[alias] = var
        end
        return expr.name2alias[var]
    end

    -- 检查是否为函数名或保留字
    local reserved = {
        ["math"] = true,
        ["sin"] = true,
        ["cos"] = true,
        ["tan"] = true,
        ["abs"] = true,
        ["sqrt"] = true,
        ["max"] = true,
        ["min"] = true,
        ["log"] = true,
        ["exp"] = true,
        ["pi"] = true,
        ["mod"] = true,
        ["floor"] = true,
        ["ceil"] = true,
        ["and"] = true,
        ["or"] = true
    }

    for _, line in ipairs(expr_list) do
        local var, formula = line:match("^%s*(.-)%s*=%s*(.+)%s*$")
        if var and formula then
            -- print("处理变量: " .. var .. " 表达式: " .. formula)
            local alias = get_alias(var)
            local converted = formula:gsub("([%w_\128-\255]+)", function(word)
                -- print("处理表达式: " .. word)
                if not tonumber(word) and not _G[word] and not reserved[word] and not word:match("^math%..+") then
                    return get_alias(word)
                end
                return word
            end)
            -- print("编译后的表达式: " .. converted)
            expr.rules[alias] = converted
            expr.deps[alias] = {}
            for dep in converted:gmatch("__GEN_VALUE_%d+") do
                if dep ~= alias then
                    -- print("依赖: " .. dep .. " -> " .. alias)
                end
                table.insert(expr.deps[alias], dep) -- 自引用也算依赖
            end
        else
            error("无效的表达式: " .. line)
        end
    end

    -- 环检测和拓扑排序
    local visit_state = {} -- nil未访问 visiting访问中 visited已访问

    local function dfs(node)
        if visit_state[node] == "visiting" then
            local var = expr.alias2name[node] or node
            error("Detected circular dependency involving variable: " .. var)
        elseif visit_state[node] == "visited" then
            return
        end

        visit_state[node] = "visiting"
        for _, dep in ipairs(expr.deps[node] or {}) do
            dfs(dep)
        end
        visit_state[node] = "visited"
    end

    for alias in pairs(expr.rules) do
        dfs(alias)
    end

    return expr
end

-- 创建属性对象
function solver:create(expr)
    local data = {}
    local cache = {}
    local dirty = {}

    local function eval(alias)
        if cache[alias] ~= nil and not dirty[alias] then
            return cache[alias]
        end

        if data[alias] ~= nil then
            cache[alias] = data[alias]
            dirty[alias] = false
            return cache[alias]
        end

        local rule = expr.rules[alias]
        if rule then
            local env = setmetatable({
                math = math,
                sin = math.sin,
                cos = math.cos,
                tan = math.tan,
                abs = math.abs,
                sqrt = math.sqrt,
                max = math.max,
                min = math.min,
                log = math.log,
                exp = math.exp,
                pi = math.pi,
                mod = math.fmod,
                floor = math.floor,
                ceil = math.ceil
            }, {
                __index = function(_, key)
                    return eval(key)
                end
            })
            -- print("Eval: " .. alias .. " = " .. rule)
            local f, err = load("return " .. rule, "solver_expr", "t", env)
            if not f then
                error("Failed to compile: " .. err)
            end
            local ok, result = pcall(f)
            if not ok then
                error("Eval failed for " .. alias .. ": " .. result)
            end
            cache[alias] = result
            dirty[alias] = false
            return result
        end

        return nil
    end

    local solver_table = {}

    setmetatable(solver_table, {
        __index = function(_, key)
            local alias = expr.name2alias[key] or key
            return eval(alias)
        end,
        __newindex = function(_, key, value)
            -- print("设置值: " .. key .. " = " .. value)
            local alias = expr.name2alias[key]
            if not alias then
                alias = make_safe_var(10000 + #expr.name2alias)
                expr.name2alias[key] = alias
                expr.alias2name[alias] = key
            end
            data[alias] = value
            cache[alias] = value
            dirty[alias] = false
            local function mark_dirty(k)
                for v, deps in pairs(expr.deps) do
                    for _, d in ipairs(deps) do
                        if d == k then
                            dirty[v] = true
                            mark_dirty(v)
                        end
                    end
                end
            end
            mark_dirty(alias)
        end,
        __pairs = function()
            local seen = {}
            local keys = {}
            for alias in pairs(expr.rules) do
                local name = expr.alias2name[alias]
                seen[name] = true
                table.insert(keys, name)
            end
            for name in pairs(expr.name2alias) do
                if not seen[name] then
                    table.insert(keys, name)
                end
            end
            local i = 0
            return function()
                i = i + 1
                local k = keys[i]
                if k then
                    return k, solver_table[k]
                end
            end
        end
    })

    return solver_table
end
