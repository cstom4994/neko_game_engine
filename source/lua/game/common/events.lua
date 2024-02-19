local events_pfx = '__neko_event_bind_'
local events_pfx_len = #events_pfx
local M = {
    default_max_listeners = 10
}

setmetatable(M, {
    __call = function(_, ...)
        return M:new(...)
    end
})

local function __neko_lua_event_remove_entry(tb, v)
    local x, len = 0, #tb
    for i = 1, len do
        local trusy, idx = false, (i - x)
        if (type(v) == 'function') then
            trusy = v(tb[idx])
        else
            trusy = tb[idx] == v
        end

        if (tb[idx] ~= nil and trusy) then
            tb[idx] = nil
            table.remove(tb, idx)
            x = x + 1
        end
    end
    return tb
end

function M:new(obj)
    obj = obj or {}
    self.__index = self
    setmetatable(obj, self)
    obj._on = {}

    return obj
end

function M:ev_table(ev)
    if (type(self._on[ev]) ~= 'table') then
        self._on[ev] = {}
    end
    return self._on[ev]
end

function M:get_ev_table(ev)
    return self._on[ev]
end

function M:add_listener(ev, listener)
    local pfx_ev = events_pfx .. tostring(ev)
    local evtbl = self:ev_table(pfx_ev)
    local maxLsnNum = self.current_max_listeners or self.default_max_listeners
    local lsnNum = self:listener_count(ev)
    table.insert(evtbl, listener)

    if (lsnNum > maxLsnNum) then
        print('WARN: Number of ' .. string.sub(pfx_ev, events_pfx_len + 1) .. " event listeners: " .. tostring(lsnNum))
    end
    return self
end

function M:emit(ev, ...)
    local pfx_ev = events_pfx .. tostring(ev)
    local evtbl = self:get_ev_table(pfx_ev)
    if (evtbl ~= nil) then
        for _, lsn in ipairs(evtbl) do
            local status, err = pcall(lsn, ...)
            if not (status) then
                print(string.sub(_, events_pfx_len + 1) .. " emit error: " .. tostring(err))
            end
        end
    end

    -- 一次性 Listener
    pfx_ev = pfx_ev .. ':once'
    evtbl = self:get_ev_table(pfx_ev)

    if (evtbl ~= nil) then
        for _, lsn in ipairs(evtbl) do
            local status, err = pcall(lsn, ...)
            if not (status) then
                print(string.sub(_, events_pfx_len + 1) .. " emit error: " .. tostring(err))
            end
        end

        __neko_lua_event_remove_entry(evtbl, function(v)
            return v ~= nil
        end)
        self._on[pfx_ev] = nil
    end
    return self
end

function M:get_max_listeners()
    return self.current_max_listeners or self.default_max_listeners
end

function M:listener_count(ev)
    local totalNum = 0
    local pfx_ev = events_pfx .. tostring(ev)
    local evtbl = self:get_ev_table(pfx_ev)

    if (evtbl ~= nil) then
        totalNum = totalNum + #evtbl
    end

    pfx_ev = pfx_ev .. ':once'
    evtbl = self:get_ev_table(pfx_ev)

    if (evtbl ~= nil) then
        totalNum = totalNum + #evtbl
    end

    return totalNum
end

function M:listeners(ev)
    local pfx_ev = events_pfx .. tostring(ev)
    local evtbl = self:get_ev_table(pfx_ev)
    local clone = {}

    if (evtbl ~= nil) then
        for i, lsn in ipairs(evtbl) do
            table.insert(clone, lsn)
        end
    end

    pfx_ev = pfx_ev .. ':once'
    evtbl = self:get_ev_table(pfx_ev)

    if (evtbl ~= nil) then
        for i, lsn in ipairs(evtbl) do
            table.insert(clone, lsn)
        end
    end

    return clone
end

M.on = M.add_listener

function M:once(ev, listener)
    local pfx_ev = events_pfx .. tostring(ev) .. ':once'
    local evtbl = self:ev_table(pfx_ev)
    local maxLsnNum = self.current_max_listeners or self.default_max_listeners
    local lsnNum = self:listener_count(ev)
    if (lsnNum > maxLsnNum) then
        print('WARN: Number of ' .. ev .. " event listeners: " .. tostring(lsnNum))
    end

    table.insert(evtbl, listener)
    return self
end

function M:remove_all(ev)
    if ev ~= nil then
        local pfx_ev = events_pfx .. tostring(ev)
        local evtbl = self:ev_table(pfx_ev)
        __neko_lua_event_remove_entry(evtbl, function(v)
            return v ~= nil
        end)

        pfx_ev = pfx_ev .. ':once'
        evtbl = self:ev_table(pfx_ev)
        __neko_lua_event_remove_entry(evtbl, function(v)
            return v ~= nil
        end)
        self._on[pfx_ev] = nil
    else
        for _pfx_ev, _t in pairs(self._on) do
            self:remove_all(string.sub(_pfx_ev, events_pfx_len + 1))
        end
    end

    for _pfx_ev, _t in pairs(self._on) do
        if (#_t == 0) then
            self._on[_pfx_ev] = nil
        end
    end

    return self
end

function M:remove(ev, listener)
    local pfx_ev = events_pfx .. tostring(ev)
    local evtbl = self:ev_table(pfx_ev)
    local lsnCount = 0
    assert(listener ~= nil, "listener is nil")
    -- 普通 Listener
    __neko_lua_event_remove_entry(evtbl, listener)

    if (#evtbl == 0) then
        self._on[pfx_ev] = nil
    end

    -- 激发一次性 Listener
    pfx_ev = pfx_ev .. ':once'
    evtbl = self:ev_table(pfx_ev)
    __neko_lua_event_remove_entry(evtbl, listener)

    if (#evtbl == 0) then
        self._on[pfx_ev] = nil
    end
    return self
end

function M:set_max_listeners(n)
    self.current_max_listeners = n
    return self
end

return M
