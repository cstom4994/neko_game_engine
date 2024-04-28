-- local ffi = require "ffi"
local POINTER_SIZE = 4

local M = {}

local settings = {
    exposed = false,
    cPrefix = ""
}

local initialized = false
local system_types = {}
local class_types = {}
local template_param_types = {}

-----------------
local function memberString(_type, ch)
    assert(_type.resolved == true)

    local def = ""
    if _type.parent ~= nil then
        def = def .. memberString(_type.parent, ch)
    end

    for i, v in ipairs(_type.members) do
        local memberType = v.type.cType
        def = def .. ch.tab .. memberType .. " " .. v.name .. ";" .. ch.line
    end

    return def
end

local charsPretty = {
    tab = "\t",
    line = "\n"
}

local chars = {
    tab = "",
    line = ""
}

local writer = {
    write = function(_type, mode)
        local ch = chars
        if mode == "pretty" then
            ch = charsPretty
        end

        local def = "typedef struct {" .. ch.line
        def = def .. memberString(_type, ch)
        def = def .. "} " .. _type.cType .. ";"
        return def
    end
}

local debug = true

local ctypes = {}
local arrayTypes = {}
local allocators = {}

local defaultAllocator = {
    alloc = function(_type)
        return ffi.new(ctypes[_type.name])
    end
}

local function tableEmpty(t)
    for k, v in pairs(t) do
        return false
    end

    return true
end

local function callDestructors(_type, instance)
    local destroy = _type.methods["destroy"];
    if destroy ~= nil then
        destroy(instance)
    end

    if _type.parent ~= nil then
        callDestructors(_type.parent, instance)
    end
end

local function buildGetterSetterTable(t, getters, setters)
    if t.parent ~= nil then
        buildGetterSetterTable(t.parent, getters, setters)
    end

    for k, v in pairs(t.properties) do
        getters[k] = v.getter
        setters[k] = v.setter
    end

    return getters, setters
end

local function buildMethodTable(t, mt)
    if t.parent ~= nil then
        buildMethodTable(t.parent, mt)
    end

    for k, v in pairs(t.methods) do
        mt[k] = v
    end

    mt["__type"] = function(self)
        return t
    end

    mt["isTypeOf"] = function(self, v)
        return v == t
    end

    if #t.templates > 0 then
        mt["__template"] = function(self, name)
            for i, v in ipairs(t.templates) do
                if v.name == name then
                    return v.value
                end
            end
        end

        mt["__templateDefault"] = function(self, name)
            for i, v in ipairs(t.templates) do
                if v.name == name then
                    return v.default
                end
            end
        end
    end

    return mt
end

local function propertyExistError(typeName, propertyName)
    error(typeName .. "." .. propertyName .. " does not exist")
end

local function indexMethodsGetters(methods, getters)
    return function(self, k)
        local p = getters[k]
        if p ~= nil then
            return p(self)
        end
        return methods[k]
    end
end

local function newindexSetters(name, setters)
    return function(self, k, v)
        -- print("SETTER", name .. "." .. k .. " = " .. tostring(v))
        local p = setters[k]
        if p ~= nil then
            p(self, v)
        else
            propertyExistError(name, k)
        end
    end
end

local function createMetaTable(_type)
    local methods = buildMethodTable(_type, {})
    local getters, setters = buildGetterSetterTable(_type, {}, {})
    local hasGetters, hasSetters = tableEmpty(getters) == false, tableEmpty(setters) == false

    local mt = {
        __gc = function(instance)
            callDestructors(_type, instance)
        end
    }
    if hasGetters == false and hasSetters == false then
        mt.__index = methods
    elseif hasGetters == true and hasSetters == false then
        mt.__index = indexMethodsGetters(methods, getters)
    elseif hasGetters == false and hasSetters == true then
        mt.__index = methods
        mt.__newindex = newindexSetters(_type.name, setters)
    elseif hasGetters == true and hasSetters == true then
        mt.__index = indexMethodsGetters(methods, getters)
        mt.__newindex = newindexSetters(_type.name, setters)
    end

    return mt
end

local function addClassType(_type)
    assert(_type.resolved == true, _type.name .. " is unresolved")
    assert(_type.primitiveType == "class", _type.name)

    -- 确保所有成员类型已添加到 ffi
    if _type.parent ~= nil then
        if _type.parent.primitiveType == "class" and ctypes[_type.parent.name] == nil then
            addClassType(_type.parent)
        end
    end

    for i, v in ipairs(_type.members) do
        local memType = v.type
        if v.type.primitiveType == "pointer" then
            memType = v.type.origin
        end

        if memType.primitiveType == "class" and ctypes[memType.name] == nil then
            addClassType(memType)
        end
    end

    local def = writer.write(_type, "pretty")

    print("-----------------")
    print(def)
    ffi.cdef(def);

    local mt = createMetaTable(_type)
    local ctype = ffi.metatype(_type.cType, mt)

    -- assert(_type.size == ffi.sizeof(ctype), "Differing type sizes for " .. _type.name .. ": " .. _type.size .. ", " .. ffi.sizeof(ctype))
    ctypes[_type.name] = ctype
    return ctype
end

local function addArrayType(_type)
    assert(_type.primitiveType ~= "template")
    assert(_type.resolved == true)

    local origin
    if _type.primitiveType == "class" then
        origin = _type
    elseif _type.primitiveType == "pointer" then
        origin = _type.origin
    end

    if origin ~= nil then
        if origin.primitiveType == "class" and ctypes[origin.name] == nil then
            addClassType(origin)
        end
    end

    local arrayType = ffi.typeof(_type.cType .. "[?]")
    arrayTypes[_type.name] = arrayType

    return arrayType
end

local gcTest = {}

local function setAllocator(_type, allocator)
    allocators[_type.name] = allocator
end

local function commit(_type)
    assert(_type.primitiveType == "class")
    assert(_type.resolved == true)

    local ctype = ctypes[_type.name]
    if ctype == nil then
        addClassType(_type)
    end
end

local function alloc(_type, ...)
    commit(_type)

    local obj
    local allocator = allocators[_type.name]

    if allocator ~= nil then
        obj = allocator.alloc(_type)
    else
        obj = defaultAllocator.alloc(_type)
    end

    table.insert(gcTest, obj)
    return obj
end

local function construct(obj, ...)
    local _type = obj:__type()
    if _type.methods.init ~= nil then
        _type.methods.init(obj, ...)
    end
end

local function create(_type, ...)
    local obj = alloc(_type)

    if _type.methods.init ~= nil then
        _type.methods.init(obj, ...)
    end

    return obj
end

local function createArray(_type, size)
    local arrayType = arrayTypes[_type.name]
    if arrayType == nil then
        arrayType = addArrayType(_type)
    end

    -- return ffi.C.malloc(_type.size * size)

    local ret = ffi.new(arrayType, size)
    table.insert(gcTest, ret)

    return ret
end

local function registerSystemType(_type)
    if _type.cType ~= _type.name then
        assert(_type.size == ffi.sizeof(_type.cType),
            "Size mismatch for " .. _type.name .. ": Def - " .. _type.size .. "   C - " .. ffi.sizeof(_type.cType))
    end
end

local ObjectFactory = {
    commit = commit,
    alloc = alloc,
    construct = construct,
    create = create,
    createArray = createArray,
    registerSystemType = registerSystemType,
    typeOf = typeOf
}

local tokenTypes = {
    ["<"] = {
        name = "LESS_THAN"
    },
    [">"] = {
        name = "GREATER_THAN"
    },
    ["="] = {
        name = "EQUALS"
    },
    [","] = {
        name = "COMMA"
    },
    ["("] = {
        name = "BRACKET_LEFT"
    },
    [")"] = {
        name = "BRACKET_RIGHT"
    },
    ["*"] = {
        name = "ASTERISK"
    },
    [" "] = false
}

local function tokenize(s)
    local idx = 1
    local nameIdx = 1
    local name = ""
    local tokens = {}

    while idx <= #s do
        local c = s:sub(idx, idx)
        local t = tokenTypes[c]
        if t == nil then
            if c:match("[%w_]") then
                name = name .. c
            else
                error("Invalid character '" .. c .. "' in class name")
            end
        else
            local token
            if #name > 0 then
                table.insert(tokens, {
                    name = "NAME",
                    value = name,
                    pos = nameIdx
                })
                name = ""
            end

            if t ~= false then
                table.insert(tokens, {
                    name = t.name,
                    value = t.value,
                    pos = idx
                })
            end

            nameIdx = idx
        end

        idx = idx + 1
    end

    if #name > 0 then
        table.insert(tokens, {
            name = "NAME",
            value = name,
            pos = nameIdx
        })
    end

    return tokens
end

local function TokenIterator(items)
    return setmetatable({
        items = items,
        idx = 1,
        current = nil
    }, {
        __index = {
            value = function(self)
                if self.idx <= #self.items then
                    return self.items[self.idx]
                end

                return {
                    name = "END"
                }
            end,

            next = function(self)
                self.idx = self.idx + 1
                self.current = self:value()
                return self:value()
            end
        }
    });
end

local function logParserError(token)
    if token.name ~= "END" then
        error("Unexpected token '" .. token.name .. "' at position " .. token.pos)
    else
        error("Unexpected end")
    end
end

local function parseTemplates(it)
    local templates = {}
    it:next()

    if it:value().name ~= "GREATER_THAN" then
        while it:value().name ~= "GREATER_THAN" do
            local template = {
                pointer = 0
            }

            if it:value().name == "NAME" then
                template.name = it:value().value
                if it:next().name ~= "COMMA" and it:value().name ~= "GREATER_THAN" then
                    if it:value().name == "NAME" then
                        template.type = template.name
                        template.name = it:value().value
                        it:next()
                    end

                    if it:value().name == "EQUALS" then
                        if it:next().name == "NAME" then
                            template.default = tonumber(it:value().value)
                            it:next()
                        else
                            logParserError(token:value())
                        end
                    end

                    while it:value().name == "ASTERISK" do
                        template.pointer = template.pointer + 1
                        it:next()
                    end

                    if it:value().name ~= "GREATER_THAN" and it:value().name ~= "COMMA" then
                        logParserError(it:value())
                    end
                end

                if it:value().name == "COMMA" then
                    it:next()

                    if it:value().name ~= "NAME" then
                        logParserError(it:value())
                    end
                end

                table.insert(templates, template)
            else
                logParserError(it:value())
            end
        end
    else
        error("No template arguments supplied")
    end

    return templates
end

local function parseRoot(it)
    local _type = {
        templates = {}
    }
    if it:value().name == "NAME" then
        _type.name = it:value().value
        if _type.name:sub(1, 1):match("%d") == nil then
            it:next()

            while it:value().name ~= "END" do
                if it:value().name == "LESS_THAN" then
                    _type.templates = parseTemplates(it)
                    it:next()
                elseif it:value().name == "ASTERISK" then
                    _type.pointer = 0
                    while it:value().name == "ASTERISK" do
                        _type.pointer = _type.pointer + 1
                        it:next()
                    end
                else
                    logParserError(it:value())
                end
            end
        else
            error("Class names can not start with a number")
        end
    else
        logParserError(it:value())
    end

    return _type
end

local function parse(s)
    local tokens = tokenize(s)
    local it = TokenIterator(tokens)
    return parseRoot(it)
end

local parser = {
    parse = parse
}

-----------------

local POINTER_SIZE = 4

local function createSystemType(name, size, cType)
    return {
        primitiveType = "system",
        name = name,
        size = size,
        cType = cType or name,
        cTypePtr = (cType or name) .. "*",
        resolved = true
    }
end

local function createTemplateType(t, mod)
    mod = mod or {}
    local template = {
        primitiveType = "template",
        name = mod.name or t.name,
        templateType = mod.templateType or t.templateType,
        value = mod.value or t.value,
        default = mod.default or t.default,
        type = mod.type or t.type
    }

    template.resolved = template.value ~= nil

    if template.templateType == nil then
        if template.type ~= nil then
            template.templateType = "value"
        else
            template.templateType = "type"
        end
    end

    return template
end

local function createClassType(data)
    data.templates = data.templates or {}

    for i, v in ipairs(data.templates) do
        data.templates[i] = createTemplateType(v)
    end

    return {
        primitiveType = "class",
        name = data.name,
        cType = data.cType,
        cTypePtr = data.cType .. "*",
        parent = data.super or object,
        templates = data.templates,
        members = {},
        methods = {},
        properties = {},
        templateDefaults = 0,
        size = 0,
        resolved = #data.templates == 0
    }
end

local function createPointerType(origin, indirection)
    local resolved = true
    local name = origin
    local cType

    if type(origin) == "table" then
        name = origin.name
        cType = origin.cType
        resolved = origin.resolved
    end

    indirection = indirection or 1
    for i = 1, indirection do
        name = name .. "*"
        if cType ~= nil then
            cType = cType .. "*"
        end
    end

    return {
        primitiveType = "pointer",
        name = name,
        cType = cType,
        cTypePtr = cType and cType .. "*" or nil,
        origin = origin,
        indirection = indirection,
        size = POINTER_SIZE,
        resolved = resolved
    }
end

local TypeFactory = {
    SystemType = createSystemType,
    ClassType = createClassType,
    PointerType = createPointerType,
    TemplateType = createTemplateType
}

-----------------

local function shallowCloneTable(t)
    local ct = {}
    for k, v in pairs(t) do
        ct[k] = v
    end

    return ct
end

local Util = {
    shallowCloneTable = shallowCloneTable
}

-----------------

local function findTemplate(_type, name)
    for i, v in ipairs(_type.templates) do
        if v.name == name then
            return v
        end
    end
end

local function getType(name)
    local t = class_types[name]
    if t ~= nil then
        return t
    end

    t = system_types[name]
    if t ~= nil then
        return t
    end
end

local function validateParent(p)
    if p == nil then
        error("Parent class undefined")
    end

    if p.resolved == false then
        error("Unable to inherit from an unresolved class type")
    end
end

local function validateTemplate(t)
end

local function validateTemplateLookup(v, name)
    assert(v ~= nil, "Template argument '" .. name .. "' could not be found in the lookup")
end

local function validateTemplateArg(arg)
    if arg == nil then
        error("Template argument undefined")
    elseif type(arg) == "string" then
        -- TODO: 确保这不是保留字
    elseif type(arg) == "number" then
        -- 在这里做一些检查
    elseif type(arg) == "table" then
        if arg.primitiveType == nil then
            error("Template argument must be a valid type")
        end
    else
        error("Template parameters of type '" .. type(arg) .. "' are not supported")
    end
end

local function updateTypeSize(t)
    t.size = 0
    for _, v in ipairs(t.members) do
        assert(v.type.resolved == nil or v.type.resolved == true)
        if v.type.pointer == 0 then
            t.size = t.size + v.type.size
        else
            t.size = t.size + POINTER_SIZE
        end
    end
end

local function generateCTypeName(name)
    return settings.cPrefix .. name:gsub("<", "_"):gsub(",", "_"):gsub(">", ""):gsub(" ", "")
end

local function cloneType(t)
    local ct = Util.shallowCloneTable(t)
    ct.members = Util.shallowCloneTable(t.members)
    ct.templates = Util.shallowCloneTable(t.templates)

    for i, v in ipairs(ct.members) do
        if v.type.primitiveType == "template" then
            ct.members[i] = Util.shallowCloneTable(v)
        end
    end

    ct.origin = t.origin or t
    ct.cType = ""
    ct.cTypePtr = ""

    return ct
end

local function bundleTemplateArgs(args, templates)
    local templateArgs = {}
    for i, arg in ipairs(args) do
        validateTemplateArg(arg)
        table.insert(templateArgs, arg)
    end

    for i = #args + 1, #templates do
        assert(templates[i].default ~= nil)
        table.insert(templateArgs, templates[i].default)
    end

    return templateArgs
end

-- 解析模板类型的成员变量
local function resolveTemplateMember(member, lookup)
    local templateParams = {}
    for i, template in ipairs(member.type.templates) do
        local lookupName = template.name
        if template.value ~= nil then
            lookupName = template.value.name
        end

        assert(lookup[lookupName] ~= nil, "Template argument '" .. lookupName .. "' could not be found in the lookup")
        table.insert(templateParams, lookup[lookupName])
    end

    local memberType = member.type
    if memberType.origin ~= nil then
        memberType = memberType.origin
    end

    return {
        name = member.name,
        type = memberType(unpack(templateParams))
    }
end

local function generateTemplateClassName(name, templates, lookup)
    name = name .. "<"
    for i, v in ipairs(templates) do
        local item = lookup[v.name]
        local tname = item
        if type(item) == "table" then
            tname = item.name
        end

        name = name .. (i > 1 and ", " or "") .. tname
    end

    return name .. ">"
end

local function setTemplateValue(template, value)
    -- 这个函数可能可以整理一下
    local resolved = true
    if type(value) == "string" then
        value = TypeFactory.TemplateType({
            name = value
        })
        template = TypeFactory.TemplateType(template, {
            value = value
        })
        resolved = false
    elseif type(value) == "table" then
        assert(value.primitiveType ~= "pointer")
        if value.primitiveType == "template" then
            template = TypeFactory.TemplateType(template, {
                value = value
            })
            resolved = false
        else
            -- 检查此代码路径是否被调用
            template = TypeFactory.TemplateType(template, {
                value = value
            })
        end
    else
        -- 检查此代码路径是否被调用
        template = TypeFactory.TemplateType(template, {
            value = value
        })
    end

    return template, resolved
end

local function resolveMembers(_type, lookup)
    for i, v in ipairs(_type.members) do
        if v.type.primitiveType == "template" then
            local temp = lookup[v.type.name]
            validateTemplateLookup(temp, v.type.name)
            _type.members[i] = {
                name = v.name,
                type = temp
            }
        elseif v.type.primitiveType == "pointer" and v.type.resolved == false then
            local temp = lookup[v.type.origin.name]
            validateTemplateLookup(temp, v.type.origin.name)
            local pt = TypeFactory.PointerType(temp, v.type.indirection)
            _type.members[i] = {
                name = v.name,
                type = pt
            }
        elseif v.type.primitiveType == "class" and v.type.resolved == false then
            _type.members[i] = resolveTemplateMember(v, lookup)
        end
    end
end

local function resolveTemplateArgs(t, ...)
    if t.resolved == false then
        local args = {...}
        if #args >= (#t.templates - t.templateDefaults) and #args <= #t.templates then
            local templateArgs = bundleTemplateArgs(args, t.templates)
            local ct = cloneType(t)
            ct.resolved = true

            local lookup = {}
            for i, arg in ipairs(templateArgs) do
                local template, resolved = setTemplateValue(ct.templates[i], arg)
                ct.templates[i] = template
                lookup[template.name] = template.value

                if resolved == false then
                    ct.resolved = false
                end
            end

            resolveMembers(ct, lookup)

            if ct.resolved == true then
                ct.name = generateTemplateClassName(ct.name, ct.templates, lookup)
                ct.cType = generateCTypeName(ct.name)
                ct.cTypePtr = ct.cType .. "*"
                updateTypeSize(ct)
            else
                ct.cType = nil
            end

            return ct
        else
            error("Wrong number of template arguments supplied to class '" .. t.name .. "' (" .. #t.templates ..
                      " expected, " .. #args .. " supplied)")
        end
    else
        error("Type '" .. t.name .. "' does not support template arguments")
    end
end

local function applyPointerMetatable(_type)
    setmetatable(_type, {
        __index = {
            new = function(self, ...)
                if self.origin.resolved == true then
                    return ObjectFactory.create(self.origin, ...)
                end

                error("Unable to instantiate class type '" .. self.name .. "' as it is unresolved");
            end,
            newArray = function(self, size)
                return ObjectFactory.createArray(_type, size)
            end,
            ptr = function(self, level)
                level = (level or 1) + _type.indirection
                local pointerType = TypeFactory.PointerType(_type.origin, level)
                applyPointerMetatable(pointerType)
                return pointerType
            end
        }
    })
end

local function applySystemMetatable(_type)
    setmetatable(_type, {
        __call = function(t, default)
            local ct = Util.shallowCloneTable(t)
            ct.default = default
            return ct
        end,
        __index = {
            newArray = function(t, size)
                return ObjectFactory.createArray(_type, size)
            end,
            ptr = function(self, level)
                local pointerType = TypeFactory.PointerType(_type, level)
                applyPointerMetatable(pointerType)
                return pointerType
            end
        }
    })
end

local function applyClassMetatable(_type)
    setmetatable(_type, {
        __call = function(t, ...)
            local ct = resolveTemplateArgs(t, ...)
            applyClassMetatable(ct)
            return ct
        end,
        __index = {
            commit = function(self)
                if self.resolved == true then
                    return ObjectFactory.commit(self)
                end

                error("Unable to commit class type '" .. self.name .. "' as it is unresolved");
            end,
            alloc = function(self)
                if self.resolved == true then
                    return ObjectFactory.alloc(self)
                end

                error("Unable to instantiate class type '" .. self.name .. "' as it is unresolved");
            end,
            new = function(self, ...)
                if self.resolved == true then
                    return ObjectFactory.create(self, ...)
                end

                error("Unable to instantiate class type '" .. self.name .. "' as it is unresolved");
            end,
            newArray = function(self, size)
                if self.resolved == true then
                    return ObjectFactory.createArray(self, size)
                end

                error("Unable to instantiate array of class type '" .. self.name .. "' as it is unresolved");
            end,
            findMember = function(self, name)
                for i, v in ipairs(self.members) do
                    if v.name == name then
                        return v
                    end
                end
            end,
            findTemplate = function(self, name)
                for i, v in ipairs(self.templates) do
                    if v.name == name then
                        return v
                    end
                end
            end,
            ptr = function(self, level)
                local pointerType = TypeFactory.PointerType(_type, level)
                applyPointerMetatable(pointerType)
                return pointerType
            end
        },
        __newindex = function(t, k, v)
            _type.methods[k] = v

            if #k > 4 then
                local s = k:sub(1, 4)
                local n = k:sub(5)
                local prop = _type.properties[n]

                if s == "get_" then
                    prop = prop or {}
                    prop.getter = v
                elseif s == "set_" then
                    prop = prop or {}
                    prop.setter = v
                end

                _type.properties[n] = prop
            end
        end
    })
end

local function addSystemType(name, size, cType)
    local _type = TypeFactory.SystemType(name, size, cType)
    applySystemMetatable(_type)

    system_types[name] = _type
    M[name] = _type

    if settings.exposed == true then
        _G[name] = _type
    end

    ObjectFactory.registerSystemType(_type)
end

local initialize = function(_settings)
    if _settings == nil then
        _settings = settings
    end

    if _settings.exposed == true then
        _G["class"] = class
        settings.exposed = true
    end

    if _settings.cPrefix ~= nil then
        settings.cPrefix = _settings.cPrefix
    end

    addSystemType("char", 1)
    addSystemType("bool", 4)
    addSystemType("float", 4)
    addSystemType("double", 8)
    addSystemType("int8", 1, "int8_t")
    addSystemType("uint8", 1, "uint8_t")
    addSystemType("int16", 2, "int16_t")
    addSystemType("uint16", 2, "uint16_t")
    addSystemType("int32", 4, "int32_t")
    addSystemType("uint32", 4, "uint32_t")
    addSystemType("int64", 8, "int64_t")
    addSystemType("uint64", 8, "uint64_t")
    addSystemType("intptr", 8, "intptr_t")
    addSystemType("uintptr", 8, "uintptr_t")
    addSystemType("f32", 4, "float")
    addSystemType("f64", 8, "double")

    initialized = true
end

local function class(name, super)
    if initialized == false then
        initialize()
    end

    local parsed = parser.parse(name)

    if system_types[parsed.name] ~= nil then
        error("The class name '" .. parsed.name .. "' is invalid as it is a reserved system type name")
    end

    if class_types[parsed.name] ~= nil then
        error("The class name '" .. parsed.name .. "' is invalid as it is already in use")
    end

    if super ~= nil then
        if type(super) == "string" then
            local parsedSuper = parser.parse(super)
            assert(system_types[parsedSuper.name] == nil)
            super = class_types[parsedSuper.name]

            if #super.templates > 0 then
            end
        end

        validateParent(super)
    end

    local _type = TypeFactory.ClassType({
        name = parsed.name,
        cType = generateCTypeName(parsed.name),
        super = super or M["object"],
        templates = parsed.templates
    })

    applyClassMetatable(_type)

    class_types[parsed.name] = _type
    M[parsed.name] = _type
    if settings.exposed == true then
        _G[parsed.name] = _type
    end

    local modifier
    modifier = setmetatable({}, {
        __call = function(t, members)
            -- TODO: 验证

            for k, v in pairs(members) do
                if type(v) == "string" then
                    local parsed = parser.parse(v)
                    local t = getType(parsed.name)
                    if t == nil then
                        t = findTemplate(_type, parsed.name)
                        if t == nil then
                            error("The type or template argument '" .. parsed.name .. "' does not exist")
                        end
                    end

                    if parsed.pointer ~= nil then
                        t = TypeFactory.PointerType(t, parsed.pointer)
                    end

                    table.insert(_type.members, {
                        name = k,
                        type = t
                    })
                elseif type(v) == "table" then
                    table.insert(_type.members, {
                        name = k,
                        type = v
                    })
                end
            end

            if _type.resolved == true then
                updateTypeSize(_type)
            else
                _type.cType = nil
            end

            return _type
        end,
        __index = {
            inherits = function(parent)
                validateParent(parent)
                _type.parent = parent
                return modifier
            end,

            templates = function(...)
                for i, v in ipairs({...}) do
                    validateTemplate(v, _type.templateDefaults > 0)

                    local template
                    if type(v) == "string" then
                        template = TypeFactory.TemplateType({
                            name = v
                        })
                    else
                        template = TypeFactory.TemplateType(v)
                        if template.default ~= nil then
                            _type.templateDefaults = _type.templateDefaults + 1
                        end
                    end

                    table.insert(_type.templates, template)
                end

                _type.resolved = false
                return modifier
            end
        }
    })

    return modifier
end

local function p(_type, level)
    local t = _type
    if type(_type) == "string" then
        local parsed = parser.parse(_type)
        t = getType(parsed.name)
        if t == nil then
            t = parsed.name
        end

        if parsed.pointer ~= nil then
            level = (level or 1) + parsed.pointer
        end
    end

    local pointerType = TypeFactory.PointerType(t, level)
    applyPointerMetatable(pointerType)

    return pointerType
end

local function typeOf(_type)
    if type(_type) == "string" then
        local t = system_types[_type]
        if t ~= nil then
            return t
        end

        t = class_types[_type]
        if t ~= nil then
            return t
        end
    elseif type(_type) == "table" then
        if _type.primitiveType ~= nil then
            return _type
        end
    elseif type(_type) == "cdata" then
        if _type.__type ~= nil then
            return _type.__type()
        end
    end
end

local function typeDef(from, to)

end

local function parseClass(name, fields)
end

local function setAllocator(_type, allocator)
    _type = typeOf(_type)
    ObjectFactory.setAllocator(_type, allocator)
end

M.setup = initialize
M.class = class
M.typeOf = typeOf
-- M.templateOf = templateOf
M.p = p
M.typeDef = typeDef
M.parseClass = parseClass
M.setAllocator = setAllocator
M.parser = parser

-----------------
-- 基础类型

local s = M

s.class "Array<T>" {
    items = "T*",
    length = s.int32
}

function s.Array:init(size)
    self.length = size or 0
    if self.length > 0 then
        self:resize(self.length)
    end
end

function s.Array:set(idx, value)
    assert(idx >= 0 and idx < self.length)
    self.items[idx] = value
end

function s.Array:get(idx)
    assert(idx >= 0 and idx < self.length)
    return self.items[idx]
end

function s.Array:resize(length)
    local old = self.items
    local _type = self:__template("T")
    self.items = _type:newArray(length)

    if self.length > 0 then
        ffi.copy(self.items, old, math.min(self.length, length) * _type.size)
    end

    self.length = length
end

local INITIAL_CAPACITY = 10
local CAPACITY_MULTIPLIER = 1.5

s.class "List<T>" {
    items = s.Array("T"),
    count = s.int32
}

function s.List:init(capacity)
    self.count = 0
    if capacity ~= nil and capacity > 0 then
        self:resize(capacity)
    end
end

function s.List:add(item)
    if self.count == self.items.length then
        local capacity = math.ceil(self.items.length * CAPACITY_MULTIPLIER)
        self.items:resize(capacity > 0 and capacity or INITIAL_CAPACITY)
    end

    self.items:set(self.count, item)
    self.count = self.count + 1
end

function s.List:get(idx)
    assert(idx < self.count)
    return self.items:get(idx)
end

function s.List:set(idx, v)
    assert(idx < self.count)
    self.items:set(idx, v)
end

function s.List:resize(capacity)
    self.items:resize(capacity)
    self.capacity = capacity
end

return M
