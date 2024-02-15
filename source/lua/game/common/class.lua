local mt__namespace = {}

local function namespace_impl_include(self, t)
    if (type(t) == "table") then
        for k, v in pairs(t) do
            if (type(v) == "table" and type(k) == "number" and v.name ~= "name") then
                if (v.class) then
                    self[v.name] = v.class
                else
                    self[v.name] = v
                end
                _G[v.name] = nil

                v.namespace = self
            elseif (k ~= "name") then
                self[k] = v
            end
        end
    end
    return self
end

local function namespace_impl_new(this, name)
    assert(not _G[name], "")
    assert(type(name) == "string")

    local new = {}

    new.name = name

    setmetatable(new, mt__namespace)

    _G[name] = new

    return function(t)
        return namespace_impl_include(new, t)
    end
end

local namespace = {}
setmetatable(namespace, mt__namespace)
mt__namespace.__call = namespace_impl_new
mt__namespace.__index = {
    include = namespace_impl_include
}

local mt_trait = {}

local function trait_impl_new(this, name)
    assert(not _G[name], "")
    assert(type(name) == "string", "")

    local new = {}

    setmetatable(new, mt_trait)

    new.statics = {}
    new.methods = {}
    new.metas = {}
    new.name = name

    _G[name] = new

    return new
end

local function trait_impl_implements(self, name)
    local t
    if (type(name) == "string" and name ~= self.name) then
        local current = self

        while (not getmetatable(current[name]) == mt_trait) do
            current = current.namespace
            if (current) then
                t = current[name]
            else
                t = _G[name]
                break
            end
        end
    elseif (type(name) == "table" and name ~= self) then
        t = name
    end

    if (getmetatable(t) == mt_trait) then
        local statics = self.statics
        for k, v in pairs(t.statics) do
            statics[k] = v
        end

        local methods = self.methods
        for k, v in pairs(t.methods) do
            methods[k] = v
        end

        local metas = self.metas
        for k, v in pairs(t.metas) do
            metas[k] = v
        end
    end
    return self
end

local function trait_impl_static(self, t)
    if (type(t) == "table") then
        local statics = self.statics
        for k, v in pairs(t) do
            statics[k] = v
        end
    end
    return self
end

local function trait_impl_method(self, t)
    if (type(t) == "table") then
        local methods = self.methods
        for k, v in pairs(t) do
            methods[k] = v
        end
    end
    return self
end

local function trait_impl_meta(self, t)
    if (type(t) == "table") then
        local metas = self.metas
        for k, v in pairs(t) do
            metas[k] = v
        end
    end
    return self
end

local function trait_impl_is(t)
    return getmetatable(t) == mt_trait
end

local trait = {}
setmetatable(trait, mt_trait)

mt_trait.__call = trait_impl_new
mt_trait.__index = {
    implements = trait_impl_implements,
    static = trait_impl_static,
    method = trait_impl_method,
    meta = trait_impl_meta
}

trait.is = trait_impl_is

local isTrait = trait.is

local mt_class = {}

local function class_impl_new(this, name)
    assert(not _G[name], "global \"" .. name .. "\" is already declared.")
    assert(type(name) == "string", "class name \"" .. name .. "\" is not valid.")

    -- 类变量和类元表
    -- 为了访问 __call 元而分开
    local c = {}
    local cmt = {}

    setmetatable(c, cmt)

    -- 对于语法糖
    local new = {}

    new.name = name
    new.class = c
    new.metatable = cmt

    c.origin = new

    setmetatable(new, mt_class)

    -- 全局声明该类
    _G[name] = c

    -- 设置构造函数调用

    new.constructors = {}
    cmt.__call = function(this, ...)
        local instance = {}

        setmetatable(instance, cmt)

        -- 执行附加的每个构造函数
        for i = 1, #new.constructors do
            new.constructors[i](instance, ...)
        end

        return instance
    end

    cmt.__index = {}

    return new
end

local function class_impl_constructor(self, fn)
    assert(type(fn) == "function", "\"fn\" is not a function.")

    -- 追加函数
    self.constructors[#self.constructors + 1] = fn
    return self
end

local function class_impl_extends(self, name)

    -- 首先检查继承是否从未发生过
    if (not self.inherited) then
        -- 上课
        local t
        if (type(name) == "string" and name ~= self.name) then
            local current = self.namespace
            if (current) then
                t = current[name]
                while (t == nil) do
                    current = current.namespace
                    if (current) then
                        t = current[name]
                    else
                        t = _G[name]
                        break
                    end
                end
            else
                t = _G[name]
            end
        elseif (type(name) == "table" and name ~= self) then
            t = name
        end

        -- 复制静态方法
        local c = self.class

        for k, v in pairs(t) do
            if (k ~= "origin" and k ~= "namespace") then
                c[k] = v
            end
        end

        -- 复制对象方法
        local po = t.origin

        local cmt = self.metatable
        local pmt = po.metatable

        local nmt = {}

        local index = {}

        for k, v in pairs(pmt.__index) do
            index[k] = v

        end

        for k, v in pairs(cmt.__index) do
            index[k] = v
        end

        nmt.__index = index

        for k, v in pairs(pmt) do
            if (k ~= "__index" and k ~= "__call") then
                nmt[k] = v
            end
        end

        for k, v in pairs(cmt) do
            if (k ~= "__index" and k ~= "__call") then
                nmt[k] = v
            end
        end

        -- 合并构造函数
        local constructors = {}
        local count = 0
        for k, v in pairs(po.constructors) do
            count = count + 1
            constructors[count] = v
        end
        for k, v in pairs(self.constructors) do
            count = count + 1
            constructors[count] = v
        end

        self.constructors = constructors

        -- setup __call 

        nmt.__call = function(this, ...)
            local instance = {}

            setmetatable(instance, nmt)

            -- 执行附加的每个构造函数
            for i = 1, #constructors do
                constructors[i](instance, ...)
            end

            return instance
        end

        setmetatable(c, nmt)

        self.metatable = nmt

        c.super = t

        nmt.__index.super = function(self)
            local cast = c.super()

            for k, v in pairs(self) do
                cast[k] = v
            end

            return cast
        end

        self.inherited = true
    end

    return self
end

local function class_impl_static(self, t)
    assert(type(t) == "table", "\"t\" is not a table.")
    local c = self.class

    -- 将所有表元素复制到类中
    for k, v in pairs(t) do
        c[k] = v
    end

    return self
end

local function class_impl_method(self, t)
    assert(type(t) == "table", "\"t\" is not a table.")
    local cmt = self.metatable

    local index = cmt.__index or {}

    -- 将所有元素复制到 __index
    for k, v in pairs(t) do
        index[k] = v
    end

    cmt.__index = index

    return self
end

local function class_impl_meta(self, t)
    assert(type(t) == "table", "\"t\" is not a table.")
    local cmt = self.metatable

    for k, v in pairs(t) do
        if (k ~= "__index" and k ~= "__newindex") then
            cmt[k] = v
        end
    end

    return self
end

local function class_impl_implements(self, name)
    local t
    if (type(name) == "string") then
        local current = self
        while (not isTrait(t)) do
            current = current.namespace
            if (current) then
                t = current[name]
            else
                t = _G[name]
                break
            end
        end
    elseif (type(name) == "table" and isTrait(name)) then
        t = name
    end

    if (t) then
        local c = self.class
        for k, v in pairs(t.statics) do
            c[k] = v
        end

        local cmt = self.metatable
        local index = cmt.__index or {}

        for k, v in pairs(t.methods) do
            index[k] = v
        end
        cmt.__index = index

        for k, v in pairs(t.metas) do
            if (k ~= "__index" and k ~= "__newindex") then
                cmt[k] = v
            end
        end
    end
    return self
end

local class = {}
setmetatable(class, mt_class)

mt_class.__call = class_impl_new
mt_class.__index = {
    constructor = class_impl_constructor,
    extends = class_impl_extends,
    static = class_impl_static,
    method = class_impl_method,
    meta = class_impl_meta,
    implements = class_impl_implements
}

local M = {}
M.class = class
M.trait = trait
M.namespace = namespace

return M
