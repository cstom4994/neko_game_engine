local ffi = FFI
local refct = hot_require 'reflect'

--- utilities ------------------------------------------------------------------

-- dereference a cdata from a pointer
function ng.__deref_cdata(ct, p)
    return ffi.cast(ct, p)[0]
end

-- enum --> values and enum --> string functions
local enum_values_map = {}
function ng.enum_values(typename)
    if enum_values_map[typename] then
        return enum_values_map[typename]
    end
    enum_values_map[typename] = {}
    for v in refct.typeof(typename):values() do
        enum_values_map[typename][v.name] = true
    end
    return enum_values_map[typename]
end
function ng.enum_tostring(typename, val)
    -- no nice inverse mapping exists...
    for name in pairs(ng.enum_values(typename)) do
        if ffi.new(typename, name) == val then
            return name
        end
    end
    return nil
end

-- return serialized string for cdata, func must be of form
-- void (typeof(cdata) *, Serializer *)
function ng.c_serialize(func, cdata)
    local s = ng.store_open()
    func(cdata, nil, s)
    local dump = ffi.string(ng.store_write_str(s))
    ng.store_close(s)
    return dump
end

-- return struct deserialized from string 'str', func must be of form
-- void (ctype *, Deserializer *)
function ng.c_deserialize(ctype, func, str)
    local cdata = ctype {}
    local s = ng.store_open_str(str)
    func(cdata, nil, cdata, s)
    ng.store_close(s)
    return cdata
end

-- create a save/load function given C type and C save/load functions all
-- as string names
function ng.c_save_load(ctype, c_save, c_load)
    return function(cdata)
        local cdump = ng.c_serialize(loadstring('return ' .. c_save)(), cdata)
        return 'ng.c_deserialize(' .. ctype .. ', ' .. c_load .. ', ' .. ng.safestr(cdump) .. ')'
    end
end

--- Entity ---------------------------------------------------------------------

-- compress entity save format
_cge = function(id)
    return ng._entity_resolve_saved_id(id)
end 

ng.Entity = ffi.metatype('Entity', {
    __eq = function(a, b)
        return type(a) == 'cdata' and type(b) == 'cdata' and ffi.istype('Entity', a) and ffi.istype('Entity', b) and
                   ng.entity_eq(a, b)
    end,
    __index = {
        __serialize = function(e)
            return string.format('_cge(%u)', e.id)
        end
    }
})

--- Vec2 -----------------------------------------------------------------------

ng.vec2 = ng.luavec2

ng.Vec2 = ffi.metatype('LuaVec2', {
    __add = ng.vec2_add,
    __sub = ng.vec2_sub,
    __unm = function(v)
        return ng.vec2_neg(v)
    end,
    __mul = function(a, b)
        if type(a) == 'number' then
            return ng.vec2_scalar_mul(b, a)
        elseif type(b) == 'number' then
            return ng.vec2_scalar_mul(a, b)
        else
            return ng.vec2_mul(a, b)
        end
    end,
    __div = function(a, b)
        if type(b) == 'number' then
            return ng.vec2_scalar_div(a, b)
        elseif type(a) == 'number' then
            return ng.scalar_vec2_div(a, b)
        else
            return ng.vec2_div(a, b)
        end
    end,
    __index = {
        __serialize = function(v)
            return string.format('ng.vec2(%f, %f)', v.x, v.y)
        end
    }
})

--- Mat3 -----------------------------------------------------------------------

ng.mat3 = ng.luamat3

ng.mat3_identity = ng.luamat3_identity

ng.Mat3 = ffi.metatype('LuaMat3', {
    __index = {
        __serialize = function(m)
            return string.format('ng.mat3(%f, %f, %f, %f, %f, %f, %f, %f, %f)', m.m[0][0], m.m[0][1], m.m[0][2],
                m.m[1][0], m.m[1][1], m.m[1][2], m.m[2][0], m.m[2][1], m.m[2][2])
        end
    }
})

--- BBox -----------------------------------------------------------------------

ng.BBox = ffi.metatype('BBox', {
    __index = {
        __serialize = function(b)
            return string.format('ng.bbox(ng.vec2(%f, %f), ng.vec2(%f, %f))', b.min.x, b.min.y, b.max.x, b.max.y)
        end
    }
})
