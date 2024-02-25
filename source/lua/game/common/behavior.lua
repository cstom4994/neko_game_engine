local M = {}

M.fail = "fail"
M.success = "success"
M.running = "running"

M.func_invert = function(child)
    return function(entity, dt)
        local result = child(entity, dt)
        if result == M.success then
            return M.fail
        elseif result == M.fail then
            return M.success
        else
            return result
        end
    end
end

M.func_parallel = function(children)
    return function(...)
        local last_result = M.success
        for i = 1, #children do
            local child = children[i]
            local result = child(...)
            if result ~= M.success then
                last_result = result
            end
        end
        return last_result
    end
end

M.func_repeat = function(times, child)
    local count = 1
    return function(...)
        local result = child(...)
        if result == M.fail then
            count = 1
            return M.fail
        elseif result == M.success then
            count = count + 1
        end
        if count > times then
            count = 1
            return M.success
        else
            return M.running
        end
    end
end

M.func_selector = function(children)
    local index = 1
    return function(...)
        while index <= #children do
            local current = children[index]
            local result = current(...)
            if result == M.success then
                index = 1
                return M.success
            end
            if result == M.running then
                return M.running
            end
            index = index + 1
        end
        index = 1
        return M.fail
    end
end

M.func_sequence = function(children)
    local index = 1
    return function(...)
        while index <= #children do
            local current = children[index]
            local result = current(...)
            if result == M.fail then
                index = 1
                return M.fail
            end
            if result == M.running then
                return M.running
            end
            index = index + 1
        end
        index = 1
        return M.success
    end
end

return M
