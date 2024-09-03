-- 导入所有内容
-- 'nekogame.util'
-- 转义一个字符串以包含在另一个字符串中
function ng.safestr(s)
    return ("%q"):format(s):gsub("\010", "n"):gsub("\026", "\\026")
end

function ng.color_from_hex(hex)
    local function hexToRGBA(hex)
        -- 移除十六进制字符串中的 # 号（如果有）
        hex = hex:gsub("#", "")

        -- 如果没有指定Alpha值，默认为255（完全不透明）
        local r, g, b, a
        if #hex == 6 then
            r, g, b = tonumber(hex:sub(1, 2), 16), tonumber(hex:sub(3, 4), 16), tonumber(hex:sub(5, 6), 16)
            a = 255
        elseif #hex == 8 then
            r, g, b, a = tonumber(hex:sub(1, 2), 16), tonumber(hex:sub(3, 4), 16), tonumber(hex:sub(5, 6), 16),
                tonumber(hex:sub(7, 8), 16)
        else
            error("Invalid hex string")
        end

        return r, g, b, a
    end
    local r, g, b, a = hexToRGBA(hex)
    return ng.color(r / 255.0, g / 255.0, b / 255.0, a / 255.0)
end

function ng.gc_setmetatable()
    if _VERSION == "Lua 5.1" then
        local rg = assert(rawget)
        local proxy_key = rg(_G, "__GC_PROXY") or "__gc_proxy"
        local rs = assert(rawset)
        local gmt = assert(debug.getmetatable)
        local smt = assert(setmetatable)
        local np = assert(newproxy)

        return function(t, mt)
            if mt ~= nil and rg(mt, "__gc") and not rg(t, "__gc_proxy") then
                local p = np(true)
                rs(t, proxy_key, p)
                gmt(p).__gc = function()
                    rs(t, proxy_key, nil)
                    local nmt = gmt(t)
                    if not nmt then
                        return
                    end
                    local fin = rg(nmt, "__gc")
                    if fin then
                        return fin(t)
                    end
                end
            end
            return smt(t, mt)
        end
    end
    return setmetatable
end

function ffi_reflect()
    local ffi = default_require "ffi"
    local bit = default_require "bit"
    local reflect = {}

    local CTState, init_CTState
    local miscmap, init_miscmap

    local function gc_str(gcref) -- Convert a GCref (to a GCstr) into a string
        if gcref ~= 0 then
            local ts = ffi.cast("uint32_t*", gcref)
            return ffi.string(ts + 4, ts[3])
        end
    end

    local typeinfo = rawget(ffi, "typeinfo")

    typeinfo = typeinfo or function(id)
        -- ffi.typeof is present in LuaJIT v2.1 since 8th Oct 2014 (d6ff3afc)
        -- this is an emulation layer for older versions of LuaJIT
        local ctype = (CTState or init_CTState()).tab[id]
        return {
            info = ctype.info,
            size = bit.bnot(ctype.size) ~= 0 and ctype.size,
            sib = ctype.sib ~= 0 and ctype.sib,
            name = gc_str(ctype.name)
        }
    end

    local function memptr(gcobj)
        return tonumber(tostring(gcobj):match "%x*$", 16)
    end

    init_CTState = function()
        -- Relevant minimal definitions from lj_ctype.h
        ffi.cdef [[
    typedef struct CType {
      uint32_t info;
      uint32_t size;
      uint16_t sib;
      uint16_t next;
      uint32_t name;
    } CType;
    
    typedef struct CTState {
      CType *tab;
      uint32_t top;
      uint32_t sizetab;
      void *L;
      void *g;
      void *finalizer;
      void *miscmap;
    } CTState;
  ]]

        -- Acquire a pointer to this Lua universe's CTState
        local co = coroutine.create(function(f, ...)
            return f(...)
        end)
        local uintgc = ffi.abi "gc64" and "uint64_t" or "uint32_t"
        local uintgc_ptr = ffi.typeof(uintgc .. "*")
        local G = ffi.cast(uintgc_ptr, ffi.cast(uintgc_ptr, memptr(co))[2])
        -- In global_State, `MRef ctype_state` precedes `GCRef gcroot[GCROOT_MAX]`.
        -- We first find (an entry in) gcroot by looking for a metamethod name string.
        local anchor = ffi.cast(uintgc, ffi.cast("const char*", "__index"))
        local i = 0
        while math.abs(tonumber(G[i] - anchor)) > 64 do
            i = i + 1
        end
        -- Since Aug 2013, `GCRef cur_L` has preceded `MRef ctype_state`. Try to find it.
        local ok, i2 = coroutine.resume(co, function(coptr)
            for i2 = i - 3, i - 20, -1 do
                if G[i2] == coptr then
                    return i2
                end
            end
        end, memptr(co))
        if ok and i2 then
            -- If we found it, work forwards looking for something resembling ctype_state.
            for i = i2 + 2, i - 1 do
                local Gi = G[i]
                if Gi ~= 0 and bit.band(Gi, 3) == 0 then
                    CTState = ffi.cast("CTState*", Gi)
                    if ffi.cast(uintgc_ptr, CTState.g) == G then
                        return CTState
                    end
                end
            end
        else
            -- Otherwise, work backwards looking for something resembling ctype_state.
            -- Note that since Jun 2020, this walks over the PRNGState, which is bad.
            for i = i - 1, 0, -1 do
                local Gi = G[i]
                if Gi ~= 0 and bit.band(Gi, 3) == 0 then
                    CTState = ffi.cast("CTState*", Gi)
                    if ffi.cast(uintgc_ptr, CTState.g) == G then
                        return CTState
                    end
                end
            end
        end
    end

    init_miscmap = function()
        -- Acquire the CTState's miscmap table as a Lua variable
        local t = {};
        t[0] = t
        local uptr = ffi.cast("uintptr_t", (CTState or init_CTState()).miscmap)
        if ffi.abi "gc64" then
            local tvalue = ffi.cast("uint64_t**", memptr(t))[2]
            tvalue[0] = bit.bor(bit.lshift(bit.rshift(tvalue[0], 47), 47), uptr)
        else
            local tvalue = ffi.cast("uint32_t*", memptr(t))[2]
            ffi.cast("uint32_t*", tvalue)[ffi.abi "le" and 0 or 1] = ffi.cast("uint32_t", uptr)
        end
        miscmap = t[0]
        return miscmap
    end

    -- Information for unpacking a `struct CType`.
    -- One table per CT_* constant, containing:
    -- * A name for that CT_
    -- * Roles of the cid and size fields.
    -- * Whether the sib field is meaningful.
    -- * Zero or more applicable boolean flags.
    local CTs = {
        [0] = {"int", "", "size", false, {0x08000000, "bool"}, {0x04000000, "float", "subwhat"}, {0x02000000, "const"},
               {0x01000000, "volatile"}, {0x00800000, "unsigned"}, {0x00400000, "long"}},
        {"struct", "", "size", true, {0x02000000, "const"}, {0x01000000, "volatile"}, {0x00800000, "union", "subwhat"},
         {0x00100000, "vla"}},
        {"ptr", "element_type", "size", false, {0x02000000, "const"}, {0x01000000, "volatile"},
         {0x00800000, "ref", "subwhat"}},
        {"array", "element_type", "size", false, {0x08000000, "vector"}, {0x04000000, "complex"}, {0x02000000, "const"},
         {0x01000000, "volatile"}, {0x00100000, "vla"}},
        {"void", "", "size", false, {0x02000000, "const"}, {0x01000000, "volatile"}},
        {"enum", "type", "size", true},
        {"func", "return_type", "nargs", true, {0x00800000, "vararg"}, {0x00400000, "sse_reg_params"}},
        {"typedef", -- Not seen
        "element_type", "", false},
        {"attrib", -- Only seen internally
        "type", "value", true},
        {"field", "type", "offset", true},
        {"bitfield", "", "offset", true, {0x08000000, "bool"}, {0x02000000, "const"}, {0x01000000, "volatile"},
         {0x00800000, "unsigned"}},
        {"constant", "type", "value", true, {0x02000000, "const"}},
        {"extern", -- Not seen
        "CID", "", true},
        {"kw", -- Not seen
        "TOK", "size"}
    }

    -- Set of CType::cid roles which are a CTypeID.
    local type_keys = {
        element_type = true,
        return_type = true,
        value_type = true,
        type = true
    }

    -- Create a metatable for each CT.
    local metatables = {}
    for _, CT in ipairs(CTs) do
        local what = CT[1]
        local mt = {
            __index = {}
        }
        metatables[what] = mt
    end

    -- Logic for merging an attribute CType onto the annotated CType.
    local CTAs = {
        [0] = function(a, refct)
            error("TODO: CTA_NONE")
        end,
        function(a, refct)
            error("TODO: CTA_QUAL")
        end,
        function(a, refct)
            a = 2 ^ a.value
            refct.alignment = a
            refct.attributes.align = a
        end,
        function(a, refct)
            refct.transparent = true
            refct.attributes.subtype = refct.typeid
        end,
        function(a, refct)
            refct.sym_name = a.name
        end,
        function(a, refct)
            error("TODO: CTA_BAD")
        end
    }

    -- C function calling conventions (CTCC_* constants in lj_refct.h)
    local CTCCs = {
        [0] = "cdecl",
        "thiscall",
        "fastcall",
        "stdcall"
    }

    local function refct_from_id(id) -- refct = refct_from_id(CTypeID)
        local ctype = typeinfo(id)
        local CT_code = bit.rshift(ctype.info, 28)
        local CT = CTs[CT_code]
        local what = CT[1]
        local refct = setmetatable({
            what = what,
            typeid = id,
            name = ctype.name
        }, metatables[what])

        -- Interpret (most of) the CType::info field
        for i = 5, #CT do
            if bit.band(ctype.info, CT[i][1]) ~= 0 then
                if CT[i][3] == "subwhat" then
                    refct.what = CT[i][2]
                else
                    refct[CT[i][2]] = true
                end
            end
        end
        if CT_code <= 5 then
            refct.alignment = bit.lshift(1, bit.band(bit.rshift(ctype.info, 16), 15))
        elseif what == "func" then
            refct.convention = CTCCs[bit.band(bit.rshift(ctype.info, 16), 3)]
        end

        if CT[2] ~= "" then -- Interpret the CType::cid field
            local k = CT[2]
            local cid = bit.band(ctype.info, 0xffff)
            if type_keys[k] then
                if cid == 0 then
                    cid = nil
                else
                    cid = refct_from_id(cid)
                end
            end
            refct[k] = cid
        end

        if CT[3] ~= "" then -- Interpret the CType::size field
            local k = CT[3]
            refct[k] = ctype.size or (k == "size" and "none")
        end

        if what == "attrib" then
            -- Merge leading attributes onto the type being decorated.
            local CTA = CTAs[bit.band(bit.rshift(ctype.info, 16), 0xff)]
            if refct.type then
                local ct = refct.type
                ct.attributes = {}
                CTA(refct, ct)
                ct.typeid = refct.typeid
                refct = ct
            else
                refct.CTA = CTA
            end
        elseif what == "bitfield" then
            -- Decode extra bitfield fields, and make it look like a normal field.
            refct.offset = refct.offset + bit.band(ctype.info, 127) / 8
            refct.size = bit.band(bit.rshift(ctype.info, 8), 127) / 8
            refct.type = {
                what = "int",
                bool = refct.bool,
                const = refct.const,
                volatile = refct.volatile,
                unsigned = refct.unsigned,
                size = bit.band(bit.rshift(ctype.info, 16), 127)
            }
            refct.bool, refct.const, refct.volatile, refct.unsigned = nil
        end

        if CT[4] then -- Merge sibling attributes onto this type.
            while ctype.sib do
                local entry = typeinfo(ctype.sib)
                if CTs[bit.rshift(entry.info, 28)][1] ~= "attrib" then
                    break
                end
                if bit.band(entry.info, 0xffff) ~= 0 then
                    break
                end
                local sib = refct_from_id(ctype.sib)
                sib:CTA(refct)
                ctype = entry
            end
        end

        return refct
    end

    local function sib_iter(s, refct)
        repeat
            local ctype = typeinfo(refct.typeid)
            if not ctype.sib then
                return
            end
            refct = refct_from_id(ctype.sib)
        until refct.what ~= "attrib" -- Pure attribs are skipped.
        return refct
    end

    local function siblings(refct)
        -- Follow to the end of the attrib chain, if any.
        while refct.attributes do
            refct = refct_from_id(refct.attributes.subtype or typeinfo(refct.typeid).sib)
        end

        return sib_iter, nil, refct
    end

    metatables.struct.__index.members = siblings
    metatables.func.__index.arguments = siblings
    metatables.enum.__index.values = siblings

    local function find_sibling(refct, name)
        local num = tonumber(name)
        if num then
            for sib in siblings(refct) do
                if num == 1 then
                    return sib
                end
                num = num - 1
            end
        else
            for sib in siblings(refct) do
                if sib.name == name then
                    return sib
                end
            end
        end
    end

    metatables.struct.__index.member = find_sibling
    metatables.func.__index.argument = find_sibling
    metatables.enum.__index.value = find_sibling

    function reflect.typeof(x) -- refct = reflect.typeof(ct)
        return refct_from_id(tonumber(ffi.typeof(x)))
    end

    function reflect.getmetatable(x) -- mt = reflect.getmetatable(ct)
        return (miscmap or init_miscmap())[-tonumber(ffi.typeof(x))]
    end

    local t_concat = default_require"table".concat
    local new = default_require"ffi".new
    local cache = setmetatable({}, {
        __mode = "v"
    })

    local inline_comment = "^%s*//[^\n]*\n()"
    local multi_line_comment = "^%s*/%*.-*/%s*()"
    local enumpat = "^(%s*([%w_][%a_]*)%s*(=?)%s*([x%x]*)%s*())"

    local function enum(defs)
        local cached = cache[defs]
        if cached then
            return cached
        end

        local N = 0
        local pos = 1
        local len = #defs
        local res = {}
        local coma = false

        while true do
            if pos == len + 1 then
                break
            end
            if pos > len + 1 then
                error("LARGER: " .. pos .. " " .. len)
            end

            local p = defs:match(inline_comment, pos) or defs:match(multi_line_comment, pos)

            if not p then
                if coma then
                    p = defs:match("^%s*,%s*()", pos)
                    if not p then
                        error "malformed enum: coma expected"
                    end
                    coma = false
                else
                    local chunk, name, eq, value
                    chunk, name, eq, value, p = defs:match(enumpat, pos)
                    if not p then
                        error("malformed enum definition")
                    end

                    if value ~= "" then
                        assert(value:find "^%-?%d+$" or value:find "0x%x+",
                            "badly formed number " .. value .. " in enum")
                        N = tonumber(value)
                    end

                    local i = N
                    N = N + 1

                    if eq == "" and value == "" or eq == "=" and value ~= "" then
                        res[#res + 1] = "  static const int " .. name .. " = " .. i .. ";"
                    else
                        error("badly formed enum: " .. chunk)
                    end
                    coma = true
                end
            end

            pos = p
        end

        res = new("struct{ \n" .. t_concat(res, "\n") .. "\n}")
        cache[defs] = res
        return res
    end

    local definepat = "^(#define[ \t]+([%w_][%a_]*)[ \t]+([x%x]+)[ \t]*(\n?)())"

    local function enum_define(defs)
        local cached = cache[defs]
        if cached then
            return cached
        end

        local pos = defs:match("^%s*\n()") or 1
        local len = #defs
        local res = {}

        while true do
            if pos == len + 1 then
                break
            end
            if pos > len + 1 then
                error("LARGER: " .. pos .. " " .. len)
            end

            local chunk, name, value, lf, p = defs:match(definepat, pos)
            p = p or defs:match(inline_comment, pos) or defs:match(multi_line_comment, pos)
            if chunk then
                if lf ~= "\n" and p ~= len + 1 then
                    error("end of line expected after: " .. chunk)
                end
                assert(value:find "^%-?%d+$" or value:find "0x%x+", "badly formed number " .. value .. " in enum")

                res[#res + 1] = "  static const int " .. name .. " = " .. value .. ";"
            elseif not p then
                p = defs:match("^[ \t]+()", pos)
                assert(p, "malformed #define")
            end
            pos = p
        end
        res = new("struct{ \n" .. t_concat(res, "\n") .. "\n}")
        cache[defs] = res
        return res
    end

    reflect.enum = enum
    reflect.enum_define = enum_define

    return reflect

end

-- hot_require 'nekogame.struct'
local ffi = FFI
local refct = ffi_reflect()

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

--- NativeEntity ---------------------------------------------------------------------

-- compress entity save format
_cge = function(id)
    return ng._entity_resolve_saved_id(id)
end

ng.NativeEntity = ffi.metatype('NativeEntity', {
    __eq = function(a, b)
        return type(a) == 'cdata' and type(b) == 'cdata' and ffi.istype('NativeEntity', a) and
                   ffi.istype('NativeEntity', b) and ng.native_entity_eq(a, b)
    end,
    __index = {
        __serialize = function(e)
            return string.format('_cge(%u)', e.id)
        end
    }
})

--- Vec2 -----------------------------------------------------------------------

ng.vec2 = ng.luavec2

ng.Vec2 = ffi.metatype('vec2', {
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

ng.Mat3 = ffi.metatype('mat3', {
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

-- hot_require 'nekogame.entity_table'
--
-- NativeEntity -> data map
--
-- can't use normal Lua tables because NativeEntity is cdata, which isn't
-- hashed right
--
local function bind_defaults(t, v)
    if type(v) == 'table' then
        local defaults = rawget(t, 'defaults')
        if defaults then
            setmetatable(v, {
                __index = defaults
            })
        end
    end
end

local entity_table_mt = {
    __newindex = function(t, k, v)
        local map = rawget(t, 'map')

        -- remove
        if v == nil then
            if map == nil then
                return
            end
            map[k.id] = nil
            return
        end

        -- add
        if not map then
            map = {}
            rawset(t, 'map', map)
        end
        bind_defaults(t, v)
        map[k.id] = {
            ['k'] = ng.NativeEntity(k),
            ['v'] = v
        }
    end,

    __index = function(t, k)
        local map = rawget(t, 'map')

        -- no map => empty
        if not map then
            return nil
        end

        -- find slot, return value in it
        local slot = map[k.id]
        if not slot then
            return nil
        end
        return slot.v
    end,

    __serialize_f = function(t)
        local map = rawget(t, 'map') or {}

        -- don't save filtered-out entities
        local filtered = {}
        for k, slot in pairs(map) do
            if ng.entity_get_save_filter(slot.k) then
                filtered[k] = slot
            end
        end
        return 'ng.__entity_table_load', filtered
    end,

    -- allows iteration using pairs(...)
    __pairs = function(t)
        local map = rawget(t, 'map')

        return function(_, k)
            -- no map => empty
            if not map then
                return nil, nil
            end

            -- get next in map
            local id, slot = next(map, k and k.id or nil)
            if not id then
                return nil, nil
            end -- end
            return slot.k, slot.v
        end, nil, nil
    end
}

function ng.is_entity_table(t)
    return type(t) == 'table' and getmetatable(t) == entity_table_mt
end

function ng.entity_table()
    return setmetatable({}, entity_table_mt)
end

function ng.entity_table_empty(t)
    for _ in pairs(t) do
        return false
    end
    return true
end

function ng.__entity_table_load(t)
    local e = ng.entity_table()
    for _, slot in pairs(t) do
        e[slot.k] = slot.v
    end
    return e
end

function ng.entity_table_merge(t, d)
    for _, slot in pairs(rawget(d, 'map') or {}) do
        bind_defaults(t, slot.v)
        t[slot.k] = slot.v
    end
end

function ng.entity_table_remove_destroyed(t, f)
    for e in pairs(t) do
        if ns.entity.destroyed(e) then
            f(e)
        end
    end
end

-- use to easily define properties with default values stored per-entity in a 
-- ng.entity_table
-- sys is the system, tbl is the table properties are stored in, name is the
-- name of the property and default is the default value if unset
function ng.simple_prop(sys, name, default, tbl, set, get)
    if tbl == nil then
        tbl = sys.tbl
    end

    -- update defaults
    if default ~= nil then
        local defaults = rawget(tbl, 'defaults')
        if not defaults then
            defaults = {}
            rawset(tbl, 'defaults', defaults)
            for _, v in pairs(tbl) do
                bind_defaults(tbl, v)
            end
        end
        defaults[name] = default
    end

    -- setter
    if set ~= false then
        sys['set_' .. name] = function(ent, val)
            local t = tbl[ent]
            if t then
                t[name] = val
            end
        end
    end

    -- getter
    if get ~= false then
        sys['get_' .. name] = function(ent)
            local t = tbl[ent]
            if t then
                return t[name]
            end
        end
    end
end

function ng.simple_props(sys, props, tbl)
    for name, default in pairs(props) do
        ng.simple_prop(sys, name, default, tbl)
    end
end

-- hot_require 'nekogame.system'

-- ng.systems (shortcut ns) is a special table such that ns.sys.func evaluates
-- to C function sys_func, eg. ns.transform.rotate(...) becomes
-- transform_rotate(...)
local system_binds = {}
local systems_mt = {
    __index = function(t, k)
        local v = rawget(t, k)

        if v ~= nil then
            return v
        end

        local v = system_binds[k]
        if v ~= nil then
            return v
        end

        local names = {}
        v = setmetatable({}, {
            __index = function(_, k2)
                local name = names[k2]
                if name ~= nil then
                    return ng[name]
                end
                name = k .. '_' .. k2
                names[k2] = name
                return ng[name]
            end
        })
        system_binds[k] = v
        return v
    end
}
ng.systems = setmetatable({}, systems_mt)
ns = ng.systems

function ng.__fire_event(event, args)
    -- store system names before firing event because systems list may change

    local sysnames = {}
    for name, sys in pairs(ns) do
        sysnames[name] = sys.receive_events == nil or sys.receive_events
    end

    for name in pairs(sysnames) do
        local system = ns[name]
        if system.enabled == nil or system.enabled then
            local func = system[event]
            if func then
                func(args)
                -- print("__fire_event", event, name)
                -- collectgarbage("collect")
                -- Inspector.breakpoint()
            end
        end
    end
end

local serpent = hot_require("libs/serpent")

function ng.__save_all()
    local data = {}

    for name, system in pairs(ns) do
        if system.auto_saveload then
            data[name] = system
        elseif system.save_all then
            -- has special load_all() event
            data[name] = system.save_all()
        end
    end

    return serpent.dump(data, {
        indent = '  ',
        nocode = true
    })
    -- return table.show(data)
end

function ng.__load_all(str)
    local f, err = loadstring(str)
    if err then
        error(err)
    end
    local data = f()

    for name, dump in pairs(data) do
        local system = rawget(ns, name)
        if system then
            -- system currently exists, must merge
            if system.auto_saveload then
                for k, v in pairs(dump) do
                    if ng.is_entity_table(system[k]) then
                        ng.entity_table_merge(system[k], v)
                    elseif --[[ system.auto_saveload_functions
                        or --]] type(v) ~= 'function' then
                        system[k] = v
                    end
                end
            elseif system.load_all then
                system.load_all(dump)
            end
        elseif dump.auto_saveload then
            -- system doesn't exist currently, just dump it in
            rawset(ns, name, dump)
        end
    end
end

-- generic add/remove, get/set for any system, property -- needs corresponding
-- C functions of the form sys_add()/sys_remove(),
-- sys_get_prop(ent)/sys_set_prop(ent, val)

unpack = table.unpack

function ng.getter(sys, prop)
    return ns[sys]['get_' .. prop] -- 这里转发到 ffi.C
end
function ng.setter(sys, prop)
    return ns[sys]['set_' .. prop] -- 这里转发到 ffi.C
end
function ng.get(sys, prop, ...)
    return ng.getter(sys, prop)(unpack({...}))
end
function ng.set(sys, prop, ...)
    ng.setter(sys, prop)(unpack({...}))
end
function ng.adder(sys)
    return ns[sys]['add'] -- 这里转发到 ffi.C
end
function ng.remover(sys)
    return ns[sys]['remove'] -- 这里转发到 ffi.C
end
function ng.remove(sys, ...)
    ng.remover(sys)(unpack({...}))
end

-- multi-purpose system adder/setter, used as follows:
--
--   ent = ng.add {
--       ent = some_entity,           -- entity to modify, skip to create new
--       prefab = 'path/to/prefab',   -- initial prefab, skip for none
--       sys1 = {
--           prop1 = val1,
--           prop2 = val2,
--           ...
--       },
--       sys2 = {
--           ...
--       },
--       ...
--   }
--
-- here 'sys1' could be transform, 'prop1' could be position, for example
function ng.add(sys, ent, props)
    -- multi-add?
    if type(sys) == 'table' then
        ent = ent or sys.ent or (sys.prefab and ns.prefab.load(sys.prefab)) or ng.entity_create()
        sys.ent = nil
        sys.prefab = nil
        for k, v in pairs(sys) do
            ng.add(k, ent, v)
        end
        -- print({sys, ent, props})
        return ent
    end

    -- 所有实体都已经在 'entity' 系统
    if sys ~= 'entity' then
        if ns[sys]["has"] ~= nil then
            if not ns[sys].has(ent) then
                ng.adder(sys)(ent)
            end
        else
            print("system " .. sys .. " 'has' func is nil")
        end

        -- print({"ng.adder", sys})
    end
    if (props) then
        for k, v in pairs(props) do
            ng.set(sys, k, ent, v)
        end
    end
end

ns.meta = {
    receive_events = false
}
ns.meta.props = {}

function ng.simple_sys()
    local sys = {
        auto_saveload = true
    }
    sys.tbl = ng.entity_table()

    -- add/remove
    function sys.simple_add(ent)
        if not sys.tbl[ent] then
            local entry = {
                ent = ent
            }
            sys.tbl[ent] = entry
            if sys.create then
                sys.create(entry)
            end
        end
    end
    sys.add = sys.simple_add
    function sys.simple_remove(ent)
        local entry = sys.tbl[ent]
        if entry then
            if sys.destroy then
                sys.destroy(entry)
            end
            sys.tbl[ent] = nil
        end
    end
    sys.remove = sys.simple_remove
    function sys.simple_has(ent)
        return sys.tbl[ent] ~= nil
    end
    sys.has = sys.simple_has

    -- update
    function sys.simple_update_all()
        ng.entity_table_remove_destroyed(sys.tbl, sys.remove)
        for _, entry in pairs(sys.tbl) do
            if ns.timing.get_paused() then
                if sys.paused_update then
                    sys.paused_update(entry)
                end
            else
                if sys.unpaused_update then
                    sys.unpaused_update(entry)
                end
            end
            if sys.update then
                sys.update(entry)
            end
        end
    end
    sys.update_all = sys.simple_update_all

    return sys
end

function ng.wrap_string(sys, prop)
    local old = ng.getter(sys, prop)
    ng[sys .. '_get_' .. prop] = function(ent)
        return ng.string(old(ent))
    end
end

-- hot_require 'nekogame.entity'
local old_entity_create = ng.entity_create
function ng.entity_create()
    local e = old_entity_create()
    ns.group.add(e, 'default')
    return e
end

-- hot_require 'nekogame.name'
-- can set unique string names per object, can find by name
-- name property is empty string or nil for no name
ns.name = {}

local entity_name = ng.entity_table() -- entity -> name map
local name_entity = {} -- name -> entity map

function ns.name.add(ent)
end
function ns.name.has(ent)
    return true
end
function ns.name.remove(ent)
    name = entity_name[ent]
    if name then
        name_entity[name] = nil
        entity_name[ent] = nil
    end
end

function ns.name.set_name(ent, name)
    ns.name.remove(ent) -- remove old name
    if name == '' or name == nil then
        return
    end -- no name
    if entity_name[ent] == name then
        return
    end -- already same

    -- someone else has name?
    if name_entity[name] ~= nil then
        -- already checked we don't have this name, must be another
        error("name: different entity already has name '" .. name .. "'")
    end

    name_entity[name] = ent
    entity_name[ent] = name
end
function ns.name.get_name(ent, name)
    return entity_name[ent] or ''
end

function ns.name.find(name)
    return name_entity[name] and name_entity[name] or ng.NativeEntity(ng.entity_nil)
end

function ns.name.update_all()
    ng.entity_table_remove_destroyed(entity_name, ns.name.remove)
end

function ns.name.save_all()
    return entity_name
end

local counter = 0
function ns.name.load_all(d)
    ng.entity_table_remove_destroyed(entity_name, ns.name.remove)
    for ent, rname in pairs(d) do
        local name = rname

        -- make up new name if clashes
        while name_entity[name] ~= nil do
            name, r = string.gsub(rname, '-%d+$', '-' .. counter)
            if r == 0 then
                name = string.format('%s-%d', rname, counter)
            end
            counter = counter + 1
        end

        ns.name.set_name(ent, name)
    end
end

-- hot_require 'nekogame.group'
ns.group = {}

local group_entities = {} -- group name --> entity_table
local entity_groups = ng.entity_table() -- entity --> set of group names

-- iterate over a group collection, which can be a string of
-- space-separated group names or a table with group names as keys
local function _groups(groups)
    if type(groups) == 'string' then
        return string.gmatch(groups, '%S+')
    end
    return pairs(groups)
end

function ns.group.add(ent, groups)
    -- if no groups parameter, nothing to do
    if not groups then
        return
    end

    for group in _groups(groups) do
        -- connect both ways
        if not group_entities[group] then
            group_entities[group] = ng.entity_table()
        end
        group_entities[group][ent] = true
        if not entity_groups[ent] then
            entity_groups[ent] = {}
        end
        entity_groups[ent][group] = true
    end
end

ns.group.add_groups = ns.group.add

function ns.group.remove(ent, groups)
    if type(groups) == 'nil' then
        -- no groups given, remove from all
        if entity_groups[ent] then
            groups = entity_groups[ent]
        else
            return -- no groups to remove from
        end
    end

    for group in _groups(groups) do
        -- disconnect both ways
        if group_entities[group] then
            group_entities[group][ent] = nil
        end
        if entity_groups[ent] then
            entity_groups[ent][group] = nil
        end
    end
end

function ns.group.has(ent)
    return true
end

function ns.group.set_groups(ent, groups)
    ns.group.remove(ent)
    ns.group.add(ent, groups)
end

function ns.group.get_groups(ent)
    local groups = {}
    if entity_groups[ent] then
        for group in pairs(entity_groups[ent]) do
            table.insert(groups, group)
        end
    end
    return table.concat(groups, ' ')
end

function ns.group.get_entities(groups)
    local ents = {}
    for group in _groups(groups) do
        if group_entities[group] then
            ng.entity_table_merge(ents, group_entities[group])
        end
    end
    return ents
end

function ns.group.destroy(groups)
    for group in _groups(groups) do
        if group_entities[group] then
            for ent in pairs(group_entities[group]) do
                ns.entity.destroy(ent)
            end
        end
        group_entities[group] = nil
    end
end

function ns.group.set_save_filter(groups, val)
    for group in _groups(groups) do
        if group_entities[group] then
            for ent in pairs(group_entities[group]) do
                if neko.core.ltype(ent) == "string" then
                    print(ent) -- bug
                end
                ns.entity.set_save_filter(ent, val)
            end
        end
    end
end

function ns.group.update_all()
    for ent in pairs(entity_groups) do
        if ns.entity.destroyed(ent) then
            ns.group.remove(ent)
        end
    end
end

function ns.group.save_all()
    return entity_groups
end

function ns.group.load_all(d)
    for ent, groups in pairs(d) do
        ns.group.set_groups(ent, groups)
    end
end

-- hot_require 'nekogame.input'
local keycode_to_string_tbl = {
    [ng.KC_ESCAPE] = '<escape>',
    [ng.KC_ENTER] = '<enter>',
    [ng.KC_TAB] = '<tab>',
    [ng.KC_BACKSPACE] = '<backspace>',
    [ng.KC_INSERT] = '<insert>',
    [ng.KC_DELETE] = '<delete>',
    [ng.KC_RIGHT] = '<right>',
    [ng.KC_LEFT] = '<left>',
    [ng.KC_DOWN] = '<down>',
    [ng.KC_UP] = '<up>',
    [ng.KC_PAGE_UP] = '<page_up>',
    [ng.KC_PAGE_DOWN] = '<page_down>',
    [ng.KC_HOME] = '<home>',
    [ng.KC_END] = '<end>',
    [ng.KC_CAPS_LOCK] = '<caps_lock>',
    [ng.KC_SCROLL_LOCK] = '<scroll_lock>',
    [ng.KC_NUM_LOCK] = '<num_lock>',
    [ng.KC_PRINT_SCREEN] = '<print_screen>',
    [ng.KC_PAUSE] = '<pause>',
    [ng.KC_F1] = '<f1>',
    [ng.KC_F2] = '<f2>',
    [ng.KC_F3] = '<f3>',
    [ng.KC_F4] = '<f4>',
    [ng.KC_F5] = '<f5>',
    [ng.KC_F6] = '<f6>',
    [ng.KC_F7] = '<f7>',
    [ng.KC_F8] = '<f8>',
    [ng.KC_F9] = '<f9>',
    [ng.KC_F10] = '<f10>',
    [ng.KC_F11] = '<f11>',
    [ng.KC_F12] = '<f12>',
    [ng.KC_F13] = '<f13>',
    [ng.KC_F14] = '<f14>',
    [ng.KC_F15] = '<f15>',
    [ng.KC_F16] = '<f16>',
    [ng.KC_F17] = '<f17>',
    [ng.KC_F18] = '<f18>',
    [ng.KC_F19] = '<f19>',
    [ng.KC_F20] = '<f20>',
    [ng.KC_F21] = '<f21>',
    [ng.KC_F22] = '<f22>',
    [ng.KC_F23] = '<f23>',
    [ng.KC_F24] = '<f24>',
    [ng.KC_F25] = '<f25>',
    [ng.KC_KP_0] = '<kp_0>',
    [ng.KC_KP_1] = '<kp_1>',
    [ng.KC_KP_2] = '<kp_2>',
    [ng.KC_KP_3] = '<kp_3>',
    [ng.KC_KP_4] = '<kp_4>',
    [ng.KC_KP_5] = '<kp_5>',
    [ng.KC_KP_6] = '<kp_6>',
    [ng.KC_KP_7] = '<kp_7>',
    [ng.KC_KP_8] = '<kp_8>',
    [ng.KC_KP_9] = '<kp_9>',
    [ng.KC_KP_DECIMAL] = '<kp_decimal>',
    [ng.KC_KP_DIVIDE] = '<kp_divide>',
    [ng.KC_KP_MULTIPLY] = '<kp_multiply>',
    [ng.KC_KP_SUBTRACT] = '<kp_subtract>',
    [ng.KC_KP_ADD] = '<kp_add>',
    [ng.KC_KP_ENTER] = '<kp_enter>',
    [ng.KC_KP_EQUAL] = '<kp_equal>',
    [ng.KC_LEFT_SHIFT] = '<shift>',
    [ng.KC_LEFT_CONTROL] = '<control>',
    [ng.KC_LEFT_ALT] = '<alt>',
    [ng.KC_LEFT_SUPER] = '<super>',
    [ng.KC_RIGHT_SHIFT] = '<shift>',
    [ng.KC_RIGHT_CONTROL] = '<control>',
    [ng.KC_RIGHT_ALT] = '<alt>',
    [ng.KC_RIGHT_SUPER] = '<super>',
    [ng.KC_MENU] = '<menu>'
}
function ng.input_keycode_to_string(key)
    return keycode_to_string_tbl[tonumber(key)] or string.char(ns.input.keycode_to_char(key))
end

local mousecode_to_string_tbl = {
    [ng.MC_1] = '<mouse_1>',
    [ng.MC_2] = '<mouse_2>',
    [ng.MC_3] = '<mouse_3>',
    [ng.MC_4] = '<mouse_4>',
    [ng.MC_5] = '<mouse_5>',
    [ng.MC_6] = '<mouse_6>',
    [ng.MC_7] = '<mouse_7>',
    [ng.MC_8] = '<mouse_8>'
}
function ng.input_mousecode_to_string(mouse)
    return mousecode_to_string_tbl[tonumber(mouse)]
end

-- hot_require 'nekogame.gui'
local root = ns.gui.get_root()
ns.group.set_groups(root, 'builtin')

ng.wrap_string('gui_text', 'str')

--- event ----------------------------------------------------------------------

ns.gui_event = {}

local event_handlers = ng.entity_table()
local event_defaults = {}

function ns.gui_event.add()
end

local function add_event(event, default)
    event_defaults[event] = default

    ns.gui_event['set_' .. event] = function(ent, f)
        if not event_handlers[ent] then
            event_handlers[ent] = {}
        end
        event_handlers[ent][event] = f
    end
end

add_event('focus_enter', false)
add_event('focus_exit', false)
add_event('mouse_down', ng.MC_NONE)
add_event('mouse_up', ng.MC_NONE)
add_event('key_down', ng.KC_NONE)
add_event('key_up', ng.KC_NONE)

function ns.gui_event.update_all()
    for ent in pairs(event_handlers) do
        if ns.entity.destroyed(ent) then
            event_handlers[ent] = nil
        end
    end

    for ent, handlers in pairs(event_handlers) do
        for event, f in pairs(handlers) do
            local r = ns.gui['event_' .. event](ent)
            if r ~= event_defaults[event] then
                f(ent, r)
            end
        end
    end
end

--- window ---------------------------------------------------------------------

ns.gui_window = {
    auto_saveload = true
}

ns.gui_window.tbl = ng.entity_table()

function ns.gui_window.add(ent)
    if ns.gui_window.tbl[ent] then
        return
    end
    ns.gui_window.tbl[ent] = {}
    local window = ns.gui_window.tbl[ent]

    -- add ent to gui_rect as container
    ng.add {
        ent = ent,
        gui_rect = {},
        gui = {
            color = ng.color_from_hex("#434343")
        }
    }

    -- titlebar containing text, minimize button
    window.titlebar = ng.add {
        transform = {
            parent = ent
        },
        gui_rect = {
            hfill = true
        },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color_from_hex("#3d85c6"),
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        }
    }
    window.title_buttons_area = ng.add {
        transform = {
            parent = window.titlebar
        },
        gui_rect = {},
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.0, 0.0, 0.0, 0.0),
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
    }
    window.close_text = ng.add {
        transform = {
            parent = window.title_buttons_area
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_text = {
            str = 'x'
        }
    }
    window.minmax_text = ng.add {
        transform = {
            parent = window.title_buttons_area
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_text = {
            str = '-'
        }
    }
    window.title_text_area = ng.add {
        transform = {
            parent = window.titlebar
        },
        gui_rect = {
            hfill = true
        },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.0, 0.0, 0.0, 0.0),
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
    }
    window.title_text = ng.add {
        transform = {
            parent = window.title_text_area
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_MID
        },
        gui_text = {
            str = 'new window'
        }
    }

    -- body containing contents
    window.body = ng.add {
        transform = {
            parent = ent
        },
        gui_rect = {},
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.0, 0.0, 0.0, 0.0),
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        }
    }

    window.statusbar = ng.add {
        transform = {
            parent = window.body
        },
        gui_rect = {
            -- hfill = true
        },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color_from_hex("#ff0000"),
            valign = ng.GA_TABLE,
            halign = ng.GA_MID
        },
        gui_text = {
            str = 'statusbar'
        }
    }
end

function ns.gui_window.remove(ent)
    local window = ns.gui_window.tbl[ent]
    if window then
        ns.transform.destroy_rec(ent)
    end
    ns.gui_window.tbl[ent] = nil
end

function ns.gui_window.has(ent)
    return ns.gui_window.tbl[ent] ~= nil
end

ng.simple_props(ns.gui_window, {
    minimized = false,
    closeable = true,
    highlight = false,
    resizable = true
})

function ns.gui_window.set_title(ent, str)
    local window = ns.gui_window.tbl[ent]
    if window then
        ns.gui_text.set_str(window.title_text, str)
    end
end
function ns.gui_window.get_title(ent)
    local window = ns.gui_window.tbl[ent]
    if window then
        return ns.gui_text.get_str(window.title_text)
    end
end
function ns.gui_window.get_title_buttons_area(ent)
    local window = ns.gui_window.tbl[ent]
    if window then
        return window.title_buttons_area
    end
end

function ns.gui_window.get_body(ent)
    local window = ns.gui_window.tbl[ent]
    if window then
        return window.body
    end
end

-- window that is being dragged
local drag_window
local mouse_prev = nil
local mouse_curr

function ns.gui_window.mouse_up(mouse)
    if mouse == ng.MC_LEFT then
        drag_window = nil
    end
end

function ns.gui_window.update_all()
    -- get mouse position
    local mouse_curr = ns.input.get_mouse_pos_pixels()
    if not mouse_prev then
        mouse_prev = mouse_curr
    end

    -- close button clicked?
    for ent, window in pairs(ns.gui_window.tbl) do
        if ns.gui.event_mouse_down(window.close_text) == ng.MC_LEFT and window.closeable then
            ns.entity.destroy(ent)
        end
        -- if ns.gui.event_mouse_down(window.statusbar) == ng.MC_LEFT then
        --     print("resize " .. window)
        -- end
        if ns.gui.event_mouse_down(window.body) == ng.MC_LEFT then
            print("body " .. ent.id)
        end
    end

    -- clear destroyed
    if drag_window and ns.entity.destroyed(drag_window) then
        drag_window = nil
    end
    for ent in pairs(ns.gui_window.tbl) do
        if ns.entity.destroyed(ent) then
            ns.gui_window.remove(ent)
        end
    end

    -- update all
    for ent, window in pairs(ns.gui_window.tbl) do
        -- new drag motion?
        if ns.gui.event_mouse_down(window.titlebar) == ng.MC_LEFT and ns.gui.get_halign(ent) == ng.GA_NONE and
            ns.gui.get_valign(ent) == ng.GA_NONE then
            drag_window = ent
        end

        -- highlight?
        if window.highlight then
            ns.gui.set_color(window.title_text, ng.color(1, 1, 0.2, 1))
        else
            ns.gui.set_color(window.title_text, ng.color_white)
        end

        -- closeable?
        ns.gui.set_visible(window.close_text, window.closeable)

        -- update maximize/minimize
        if ns.gui.event_mouse_down(window.minmax_text) == ng.MC_LEFT then
            window.minimized = not window.minimized
        end
        ns.gui.set_visible(window.body, not window.minimized)
        ns.gui_text.set_str(window.minmax_text, window.minimized and '+' or '-')
    end

    -- move dragged window
    if drag_window then
        ns.transform.translate(drag_window, mouse_curr - mouse_prev)
    end

    mouse_prev = mouse_curr
end

--- textbox --------------------------------------------------------------------

ns.gui_textbox = {
    auto_saveload = true
}

ns.gui_textbox.tbl = ng.entity_table()

function ns.gui_textbox.add(ent)
    if ns.gui_textbox.tbl[ent] then
        return
    end
    ns.gui_textbox.tbl[ent] = {}
    local gui_textbox = ns.gui_textbox.tbl[ent]

    -- add ent to gui_rect as container
    ng.add {
        ent = ent,
        gui_rect = {}
    }

    -- add text child
    gui_textbox.text = ng.add {
        transform = {
            parent = ent
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_MIN
        },
        gui_text = {}
    }
end

function ns.gui_textbox.remove(ent)
    local textbox = ns.gui_textbox.tbl[ent]
    if textbox then
        ns.transform.destroy_rec(ent)
    end
    ns.gui_textbox.tbl[ent] = nil
end

function ns.gui_textbox.has(ent)
    return ns.gui_textbox.tbl[ent] ~= nil
end

ng.simple_props(ns.gui_textbox, {
    click_focus = false
})

function ns.gui_textbox.get_text(ent)
    local gui_textbox = ns.gui_textbox.tbl[ent]
    if gui_textbox then
        return gui_textbox.text
    end
end

function ns.gui_textbox.update_all()
    for ent in pairs(ns.gui_textbox.tbl) do
        if ns.entity.destroyed(ent) then
            ns.gui_textbox.remove(ent)
        end
    end

    for ent, textbox in pairs(ns.gui_textbox.tbl) do
        if textbox.click_focus and ns.gui.event_mouse_up(ent) == ng.MC_LEFT then
            ns.gui.set_focus(textbox.text, true)
        end
    end
end

--- gui_checkbox ---------------------------------------------------------------

ns.gui_checkbox = {
    auto_saveload = true
}

ns.gui_checkbox.tbl = ng.entity_table()

function ns.gui_checkbox.add(ent)
    if ns.gui_checkbox.tbl[ent] then
        return
    end

    local checkbox = {}
    ns.gui_checkbox.tbl[ent] = checkbox

    checkbox.checked = false

    ns.gui_textbox.add(ent)
    checkbox.ent = ent
    checkbox.text = ns.gui_textbox.get_text(ent)
end

function ns.gui_checkbox.remove(ent)
    ns.gui_textbox.remove(ent)
    ns.gui_checkbox.tbl[ent] = nil
end

function ns.gui_checkbox.has(ent)
    return ns.gui_checkbox.tbl[ent] ~= nil
end

local function checkbox_toggle(checkbox)
    checkbox.checked = not checkbox.checked
    ns.gui.fire_event_changed(checkbox.ent)
end
function ns.gui_checkbox.toggle(ent)
    local checkbox = ns.gui_checkbox.tbl[ent]
    if checkbox then
        checkbox_toggle(checkbox)
    end
end
function ns.gui_checkbox.set_checked(ent, checked)
    local checkbox = ns.gui_checkbox.tbl[ent]
    -- do it this way to fire 'changed' event correctly
    if checkbox.checked ~= checked then
        checkbox_toggle(checkbox)
    end
end
function ns.gui_checkbox.get_checked(ent, checked)
    local checkbox = ns.gui_checkbox.tbl[ent]
    if checkbox then
        return checkbox.checked
    end
end

function ns.gui_checkbox.update_all(ent)
    for ent in pairs(ns.gui_checkbox.tbl) do
        if ns.entity.destroyed(ent) then
            ns.gui_checkbox.remove(ent)
        end
    end

    for ent, checkbox in pairs(ns.gui_checkbox.tbl) do
        if ns.gui.event_mouse_down(ent) == ng.MC_LEFT then
            checkbox_toggle(checkbox)
        end

        if checkbox.checked then
            ns.gui_text.set_str(checkbox.text, 'yes')
        else
            ns.gui_text.set_str(checkbox.text, 'no')
        end
    end
end

-- hot_require 'nekogame.edit'
local ffi = FFI

ns.edit = {
    inspect = false
}

--- expose C functions ---------------------------------------------------------

ns.edit.set_enabled = ng.edit_set_enabled
ns.edit.get_enabled = ng.edit_get_enabled

ns.edit.set_editable = ng.edit_set_editable
ns.edit.get_editable = ng.edit_get_editable

ns.edit.set_grid_size = ng.edit_set_grid_size
ns.edit.get_grid_size = ng.edit_get_grid_size

ns.edit.bboxes_update = ng.edit_bboxes_update
ns.edit.bboxes_has = ng.edit_bboxes_has
ns.edit.bboxes_get = ng.edit_bboxes_get
ns.edit.bboxes_get_num = ng.edit_bboxes_get_num
ns.edit.bboxes_get_nth_ent = ng.edit_bboxes_get_nth_ent
ns.edit.bboxes_get_nth_bbox = ng.edit_bboxes_get_nth_bbox
ns.edit.bboxes_set_selected = ng.edit_bboxes_set_selected

ns.edit.line_add = ng.edit_line_add

-- called when using ng.add { ... }, just do nothing
function ns.edit.add()
end

function ns.edit.has()
    return true
end

--- mode management-------------------------------------------------------------

ns.edit.modes = {
    ['none'] = {},
    ['all'] = {}
}
ns.edit.mode = 'none'

function ns.edit.mode_event(evt)
    -- convert key/mouse enum to number
    if type(evt) == 'cdata' then
        evt = tonumber(evt)
    end

    -- forward to all, then to current mode
    local e = ns.edit.modes.all[evt]
    if e then
        e()
    end
    local e = ns.edit.modes[ns.edit.mode][evt]
    if e then
        e()
    end
end

-- codestr is 'a', 'b', 'c', '<tab>' etc. as returned by input.*_to_string(...)
function ns.edit._mode_exec_bind(up, codestr)
    -- modifier prefixes
    local mods = {
        [ng.KC_LEFT_CONTROL] = 'C-',
        [ng.KC_RIGHT_CONTROL] = 'C-',
        [ng.KC_LEFT_ALT] = 'M-',
        [ng.KC_RIGHT_ALT] = 'M-',
        [ng.KC_LEFT_SHIFT] = 'S-',
        [ng.KC_RIGHT_SHIFT] = 'S-'
    }
    for mod, prefix in pairs(mods) do
        if ns.input.key_down(mod) then
            codestr = prefix .. codestr
        end
    end

    -- up prefix
    codestr = up and ('^' .. codestr) or codestr

    -- execute!
    ns.edit.mode_event(codestr)
end

function ns.edit.mode_key_down(key)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(false, ns.input.keycode_to_string(key))
end
function ns.edit.mode_key_up(key)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(true, ns.input.keycode_to_string(key))
end
function ns.edit.mode_mouse_down(mouse)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(false, ns.input.mousecode_to_string(mouse))
end
function ns.edit.mode_mouse_up(mouse)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(true, ns.input.mousecode_to_string(mouse))
end

function ns.edit.set_mode(mode)
    ns.edit.mode_event('exit')
    ns.edit.mode = mode
    ns.edit.mode_event('enter')
end

--- play/stop/pause ------------------------------------------------------------

ns.edit.stopped = true

-- load this when stopped
local stop_savepoint = nil
local stop_save_next_frame = false -- whether to save a stop soon
local function stop_save()
    ns.group.set_save_filter('default edit_inspector', true)
    local s = ng.store_open()
    ns.system.save_all(s)
    stop_savepoint = ffi.string(ng.store_write_str(s))
    ng.store_close(s)

    if ns.timing.get_paused() then
        ns.edit.stopped = true
    end
end
function ns.edit.stop_save()
    stop_save_next_frame = true
end

function ns.edit.stop()
    if not stop_savepoint then
        return
    end

    ns.group.destroy('default edit_inspector')
    local s = ng.store_open_str(stop_savepoint)
    ns.system.load_all(s)
    ng.store_close(s)

    ns.timing.set_paused(true)
    ns.edit.stopped = true
end

function ns.edit.play()
    ns.timing.set_paused(false)
end

function ns.edit.pause_toggle()
    ns.timing.set_paused(not ns.timing.get_paused())
end

--- undo -----------------------------------------------------------------------

ns.edit.history = {}

function ns.edit.undo_save()
    ns.group.set_save_filter('default edit_inspector', true)
    local s = ng.store_open()
    ns.system.save_all(s)

    local str = ffi.string(ng.store_write_str(s))
    table.insert(ns.edit.history, str)
    if ns.edit.stopped then
        stop_savepoint = str
    end -- update stop if stopped

    ng.store_close(s)
end

function ns.edit.undo()
    if #ns.edit.history <= 1 then
        print('nothing to undo')
        return
    end

    -- TODO: make 'edit' entity group and destroy all except that?
    ns.group.destroy('default edit_inspector')

    table.remove(ns.edit.history)
    local str = ns.edit.history[#ns.edit.history]
    local s = ng.store_open_str(str)
    ns.system.load_all(s)
    ng.store_close(s)
end

--- normal mode ----------------------------------------------------------------

ns.edit.modes.normal = {}

function ns.edit.modes.normal.enter()
    ns.edit.hide_mode_text()
end

ns.edit.mode = 'normal' -- start in normal mode

--- edit gui root --------------------------------------------------------------

ns.edit.gui_root = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true,
        vfill = true
    },
    gui = {
        visible = false,
        captures_events = false,
        color = ng.color(0, 0, 0, 0), -- invisible
        halign = ng.GA_MIN,
        valign = ng.GA_MIN,
        padding = ng.vec2_zero
    }
}

--- edit camera ----------------------------------------------------------------

local camera_default_height = 150
ns.edit.camera = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    camera = {
        viewport_height = camera_default_height,
        current = false
    }
}
ns.camera.set_edit_camera(ns.edit.camera)

-- drag

local camera_drag_mouse_prev
function ns.edit.camera_drag_start()
    camera_drag_mouse_prev = ns.input.get_mouse_pos_unit()
    ns.edit_camera_drag.enabled = true
end
function ns.edit.camera_drag_end()
    ns.edit_camera_drag.enabled = false
end

ns.edit_camera_drag = {
    enabled = false
}
function ns.edit_camera_drag.update_all()
    if not ns.edit.get_enabled() then
        ns.edit_camera_drag.enabled = false
        return
    end

    local m = ns.input.get_mouse_pos_unit()

    -- find screen mouse motion in world coordinates, move opposite way
    local campos = ns.transform.local_to_world(ns.edit.camera, ng.vec2_zero)
    local mp = ns.camera.unit_to_world(camera_drag_mouse_prev) - campos
    local mc = ns.camera.unit_to_world(m) - campos
    ns.transform.translate(ns.edit.camera, mp - mc)
    camera_drag_mouse_prev = m
end

-- zoom
local camera_zoom_factor = 0
function ns.edit.camera_zoom(f)
    camera_zoom_factor = camera_zoom_factor + f
    local h = math.pow(0.8, camera_zoom_factor) * camera_default_height
    ns.camera.set_viewport_height(ns.edit.camera, h)
end
function ns.edit.camera_zoom_in()
    ns.edit.camera_zoom(1)
end
function ns.edit.camera_zoom_out()
    ns.edit.camera_zoom(-1)
end

--- other files ----------------------------------------------------------------

-- core edit stuff
-- hot_require 'nekogame.edit_select'
ns.edit.select = ng.entity_table()

-- get some selected entity, or nil if none selected
function ns.edit.select_get_first()
    for ent in pairs(ns.edit.select) do
        return ent
    end
    return nil
end

function ns.edit.select_toggle(ent)
    if ns.edit.select[ent] then
        ns.edit.select[ent] = nil
    else
        ns.edit.select[ent] = true
    end
end

function ns.edit.select_clear()
    ns.edit.select = ng.entity_table()
end

--- click select ---------------------------------------------------------------

local function _get_entities_under_mouse()
    local ents = {}

    local m = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        local bbox = ns.edit.bboxes_get_nth_bbox(i)

        -- transform m to local space
        local t = ng.mat3_inverse(ns.transform.get_world_matrix(ent))
        if ng.bbox_contains(bbox, ng.mat3_transform(t, m)) then
            table.insert(ents, ng.NativeEntity(ent))
        end
    end

    -- sort by distance to mouse
    local distcomp = function(e1, e2)
        local p1 = ns.transform.get_world_position(e1)
        local p2 = ns.transform.get_world_position(e2)
        return ng.vec2_dist(p1, m) < ng.vec2_dist(p2, m)
    end
    table.sort(ents, distcomp)

    return ents
end

function ns.edit.select_click_single()
    -- 判断鼠标下有东西
    local ents = _get_entities_under_mouse()

    if #ents == 0 then
        ns.edit.select = ng.entity_table()
        ns.edit.undo_save()
        return
    end

    -- if something's already selected, select the next thing
    ents[#ents + 1] = ents[1] -- duplicate first at end to wrap-around
    local sel = 0
    for i = 1, #ents - 1 do
        sel = i
        if ns.edit.select[ents[i]] then
            break
        end
    end
    ns.edit.select = ng.entity_table()
    ns.edit.select[ents[sel + 1]] = true

    ns.edit.undo_save()
end

function ns.edit.select_click_multi()
    -- anything under mouse?
    local ents = _get_entities_under_mouse()
    if #ents == 0 then
        ns.edit.undo_save()
        return
    end

    -- if something isn't selected, select it
    for i = 1, #ents do
        if not ns.edit.select[ents[i]] then
            ns.edit.select[ents[i]] = true
            ns.edit.undo_save()
            return
        end
    end

    -- otherwise deselect the first
    ns.edit.select[ents[1]] = nil

    ns.edit.undo_save()
end

--- boxsel ---------------------------------------------------------------------

local boxsel_has_begun, boxsel_init_mouse_pos

local function _boxsel_select()
end

function ns.edit.boxsel_start()
    ns.edit.set_mode('boxsel')
end
function ns.edit.boxsel_begin()
    boxsel_has_begun = true
    boxsel_init_mouse_pos = ns.input.get_mouse_pos_pixels()
end
function ns.edit.boxsel_end()
    ns.edit.select = ng.entity_table()
    ns.edit.boxsel_end_add()
end
function ns.edit.boxsel_end_add()
    if boxsel_has_begun then
        b = ns.camera.pixels_to_world(boxsel_init_mouse_pos)
        e = ns.camera.pixels_to_world(ns.input.get_mouse_pos_pixels())
        bb = ng.BBox(ng.bbox_bound(b, e))

        for i = 0, ns.edit.bboxes_get_num() - 1 do
            local ent = ng.NativeEntity(ns.edit.bboxes_get_nth_ent(i))
            if ng.bbox_contains(bb, ns.transform.get_world_position(ent)) then
                ns.edit.select[ent] = true
            end
        end
    end

    ns.edit.set_mode('normal')
    ns.edit.undo_save()
end

ns.edit.boxsel_box = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    transform = {
        position = ng.vec2(-20, -20)
    },
    gui_rect = {
        size = ng.vec2(10, 10),
        hfit = false,
        vfit = false
    },
    gui = {
        captures_events = false,
        color = ng.color(0.8, 0.5, 0.1, 0.3)
    }
}

ns.edit.modes.boxsel = {}

function ns.edit.modes.boxsel.enter()
    ns.edit.set_mode_text('boxsel')

    boxsel_has_begun = false
end

function ns.edit.modes.boxsel.exit()
    ns.transform.set_position(ns.edit.boxsel_box, ng.vec2(-20, -20))
    ns.gui_rect.set_size(ns.edit.boxsel_box, ng.vec2(10, 10))
end

function ns.edit.modes.boxsel.update_all()
    if not boxsel_has_begun then
        return
    end

    m = ns.input.get_mouse_pos_pixels()
    b = ng.BBox(ng.bbox_bound(m, boxsel_init_mouse_pos))
    ns.transform.set_position(ns.edit.boxsel_box, ng.vec2(b.min.x, b.max.y))
    ns.gui_rect.set_size(ns.edit.boxsel_box, b.max - b.min)
end

-- hot_require 'nekogame.edit_command'
--- command mode ---------------------------------------------------------------
ns.edit.modes.command = {}

local command_end_callback, command_completion_func, command_completions
local command_completions_index, command_always_complete

local function command_update_completions_text()
    ns.gui_text.set_str(ns.edit.command_completions_text, table.concat(command_completions, ' | '))
end
local function command_update_completions()
    local s = ns.gui_text.get_str(ns.edit.command_text)
    command_completions = command_completion_func(s)
    command_update_completions_text()
end

-- whether sub is a subsequence of seq
local function subseq(seq, sub)
    local j = 1
    local lsub = #sub + 1
    if lsub == 1 then
        return true
    end
    for i = 1, #seq do
        if string.byte(seq, i) == string.byte(sub, j) then
            j = j + 1
            if j == lsub then
                return true
            end
        end
    end
    return false
end

-- returns a completion function that uses substring search
-- case insensitive
function ns.edit.command_completion_substr(t)
    return function(s)
        local comps = {}
        local s = string.lower(s)
        for k in pairs(t) do
            if subseq(string.lower(k), s) then
                table.insert(comps, k)
            end
        end
        return comps
    end
end

-- 使用文件系统搜索的补全函数 不区分大小写
function ns.edit.command_completion_fs(s)
    local comps = {}
    -- local s = string.lower(s)
    -- local dir_path = string.match(s, '(.*/)') or './'
    -- local suffix = string.match(s, '.*/(.*)') or s
    -- local dir = ns.fs.dir_open(dir_path)
    -- if dir == nil then
    --     return {}
    -- end
    -- while true do
    --     local f = ns.fs.dir_next_file(dir)
    --     if f == nil then
    --         break
    --     end
    --     f = ng.string(f)
    --     if f ~= '.' and f ~= '..' and subseq(string.lower(dir_path .. f), s) then
    --         table.insert(comps, dir_path .. f)
    --     end
    -- end
    -- ns.fs.dir_close(dir)
    return comps
end

local function run_string(s)
    local r, e = loadstring(s)
    if r then
        r()
    else
        error(e)
    end
end

function ns.edit.command_start(prompt, callback, completion_func, always_complete, initial)
    ns.edit.set_mode('command')

    -- default is eval script
    prompt = prompt or 'lua: '
    command_end_callback = callback or run_string
    command_completion_func = completion_func or function()
        return {}
    end
    command_always_complete = always_complete and true or false

    initial = initial or ''
    ns.gui_text.set_str(ns.edit.command_text, initial or '')
    ns.gui_textedit.set_cursor(ns.edit.command_text, #initial)

    ns.gui_text.set_str(ns.edit.command_text_colon, prompt)
    command_update_completions()
end
function ns.edit.command_end()
    if command_always_complete then
        if #command_completions == 0 then
            return
        end -- no completions
        ns.edit.command_complete()
    end

    ns.edit.set_mode('normal')

    local s = ns.gui_text.get_str(ns.edit.command_text)
    if command_end_callback then
        command_end_callback(s)
    else
        print('no command callback for \'' .. s .. '\'')
    end
end
function ns.edit.command_cancel()
    ns.edit.set_mode('normal')
end

-- actually pick a completion
function ns.edit.command_complete()
    if #command_completions > 0 then
        local comp = command_completions[1]
        ns.gui_text.set_str(ns.edit.command_text, comp)
        ns.gui_textedit.set_cursor(ns.edit.command_text, #comp)
        command_update_completions()
    end
end

function ns.edit.modes.command.enter()
    ns.edit.set_mode_text('command')

    ns.gui.set_visible(ns.edit.command_bar, true)

    ns.gui_text.set_str(ns.edit.command_completions_text, '')
end
function ns.edit.modes.command.exit()
    ns.gui.set_visible(ns.edit.command_bar, false)

    ns.gui.set_focus(ns.edit.command_text, false)
    command_completions = {}
end

function ns.edit.modes.command.update_all()
    -- done?
    if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_ENTER then
        ns.edit.command_end()
        return
    elseif ns.gui.event_focus_exit(ns.edit.command_text) then
        ns.edit.command_cancel()
        return
    end

    if ns.gui.event_changed(ns.edit.command_text) then
        command_update_completions()
    end
    if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_TAB then
        ns.edit.command_complete()
    end

    -- next/prev completion
    if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_DOWN then
        table.insert(command_completions, table.remove(command_completions, 1))
        command_update_completions_text()
    elseif ns.gui.event_key_down(ns.edit.command_text) == ng.KC_UP then
        table.insert(command_completions, 1, table.remove(command_completions))
        command_update_completions_text()
    end

    ns.gui.set_focus(ns.edit.command_text, true)
end

--- built-in prompts -----------------------------------------------------------

-- asks for grid size -- first x then y
local function command_gridx(x)
    local function gridy(y)
        ns.edit.set_grid_size(ng.vec2(tonumber(x) or 0, tonumber(y) or 0))
    end
    ns.edit.command_start('grid y: ', gridy)
end
function ns.edit.command_grid()
    ns.edit.command_start('grid x: ', command_gridx)
end

-- inspects system on selected entities, or creates entity if none selected
function ns.edit.command_inspect()
    local add = ng.entity_table_empty(ns.edit.select)

    local function system(s)
        if add then
            local e = ng.entity_create()
            ns.edit_inspector.add(e, s)
            ns.edit.select[e] = true
        elseif not ng.entity_table_empty(ns.edit.select) then
            for ent in pairs(ns.edit.select) do
                ns.edit_inspector.add(ent, s)
            end
        end
        ns.edit.undo_save()
    end

    -- complete to systems that have properties listed
    local syss = ns.edit_inspector.get_systems()
    local comp = ns.edit.command_completion_substr(syss)

    ns.edit.command_start(add and 'new entity: ' or 'edit system: ', system, comp, true)
end

local last_save = nekogame_usr_path .. 'levels/'
function ns.edit.command_save()
    local function save(f)
        ns.console.printf("edit: saving group 'default' to file '" .. f .. "' ... ")
        ns.group.set_save_filter('default', true)
        local s = ng.store_open()
        ns.system.save_all(s)
        ng.store_write_file(s, f)
        ng.store_close(s)
        print("done")

        ns.edit.stop_save()

        last_save = f
    end

    ns.edit.command_start('save to file: ', save, ns.edit.command_completion_fs, false, last_save)
end

local last_load = nekogame_usr_path .. 'levels/'
function ns.edit.command_load()
    local function load(f)
        ns.group.destroy('default')

        ns.console.printf("edit: loading from file '" .. f .. "' ... ")
        local s = ng.store_open_file(f)
        ns.system.load_all(s)
        ng.store_close(s)
        print("done")

        ns.edit.stop_save()
        ns.timing.set_paused(true)
        ns.edit.stopped = true

        last_load = f
    end

    ns.edit.command_start('load from file: ', load, ns.edit.command_completion_fs, false, last_load)
end

function ns.edit.set_default_file(s)
    last_save = s
    last_load = s
end

local last_save_prefab = nekogame_usr_path .. 'prefabs/'
function ns.edit.command_save_prefab()
    if ng.entity_table_empty(ns.edit.select) then
        return
    end

    local function save(f)
        for ent in pairs(ns.edit.select) do
            if ns.transform.has(ent) then
                ns.transform.set_save_filter_rec(ent, true)
            else
                ns.entity.set_save_filter(ent, true)
            end
        end

        ns.prefab.save(f, ns.edit.select_get_first())

        last_save_prefab = f
    end

    ns.edit.command_start('save prefab: ', save, ns.edit.command_completion_fs, false, last_save_prefab)
end

local last_load_prefab = nekogame_usr_path .. 'prefabs/'
function ns.edit.command_load_prefab()
    local function load(f)
        ns.edit.select_clear()
        local ent = ns.prefab.load(f)
        ns.edit.select[ent] = true
        if ns.transform.has(ent) then
            -- move to center of view
            local w = ns.transform.local_to_world(ns.edit.camera, ng.vec2_zero)
            w.x = math.floor(w.x + 0.5)
            w.y = math.floor(w.y + 0.5)
            ns.transform.set_position(ent, w)
        end
        ns.edit.undo_save()

        last_load_prefab = f
    end

    ns.edit.command_start('load prefab: ', load, ns.edit.command_completion_fs, true, last_load_prefab)
end

function ns.edit.set_default_prefab_file(s)
    last_save_prefab = s
    last_load_prefab = s
end

-- hot_require 'nekogame.edit_bottom_gui'
--- layout ---------------------------------------------------------------------
ns.edit.bottom_rect = ng.add {
    transform = {
        parent = ns.edit.gui_root
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        color = ng.color(0, 0, 0, 0), -- invisible
        captures_events = false,
        halign = ng.GA_MIN,
        valign = ng.GA_MIN,
        padding = ng.vec2_zero
    }
}

ns.edit.status_bar = ng.add {
    transform = {
        parent = ns.edit.bottom_rect
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        color = ng.color(0.1, 0.1, 0.1, 1.0),
        halign = ng.GA_MIN,
        valign = ng.GA_TABLE,
        padding = ng.vec2_zero
    }
}
ng.add {
    transform = {
        parent = ns.edit.status_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_text = {
        str = ''
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_MIN,
        valign = ng.GA_MAX,
        padding = ng.vec2(2, 2)
    }
}

ns.edit.bottom_bar = ng.add {
    transform = {
        parent = ns.edit.bottom_rect
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        color = ng.color(0.9, 0.9, 0.9, 1.0),
        halign = ng.GA_MIN,
        valign = ng.GA_TABLE,
        padding = ng.vec2_zero
    }
}
ng.add {
    transform = {
        parent = ns.edit.bottom_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_text = {
        str = ''
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_MIN,
        valign = ng.GA_MAX,
        padding = ng.vec2(2, 2)
    }
}

--- text -----------------------------------------------------------------------

local function create_status_textbox(gap, label)
    local textbox = ng.add {
        transform = {
            parent = ns.edit.status_bar
        },
        group = {
            groups = 'builtin'
        },
        edit = {
            editable = false
        },
        gui = {
            color = ng.color(0.6, 0.1, 0.1, 1),
            halign = ng.GA_TABLE,
            valign = ng.GA_MAX,
            padding = ng.vec2(gap, 0)
        },
        gui_rect = {}
    }
    local text = ng.add {
        transform = {
            parent = textbox
        },
        group = {
            groups = 'builtin'
        },
        edit = {
            editable = false
        },
        gui_text = {
            str = label or ''
        },
        gui = {
            color = ng.color_white,
            halign = ng.GA_MIN,
            valign = ng.GA_MAX,
            padding = ng.vec2(4, 2)
        }
    }
    return textbox, text
end

ns.edit.edit_textbox, ns.edit.edit_text = create_status_textbox(0, 'edit')
ns.edit.grid_textbox, ns.edit.grid_text = create_status_textbox(7, '')
ns.edit.select_textbox, ns.edit.select_text = create_status_textbox(7, '')
ns.edit.mode_textbox, ns.edit.mode_text = create_status_textbox(7, '')

ns.edit.play_textbox, ns.edit.play_text = create_status_textbox(0, '\xcb')
ns.gui.set_halign(ns.edit.play_textbox, ng.GA_MAX)

function ns.edit.set_mode_text(s)
    ns.gui.set_visible(ns.edit.mode_textbox, true)
    ns.gui_text.set_str(ns.edit.mode_text, s)
end
function ns.edit.hide_mode_text()
    ns.gui.set_visible(ns.edit.mode_textbox, false)
end
ns.edit.hide_mode_text()

--- command text ---------------------------------------------------------------

ns.edit.command_bar = ng.add {
    transform = {
        parent = ns.edit.bottom_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        visible = false,
        color = ng.color_clear,
        halign = ng.GA_MIN,
        valign = ng.GA_TABLE,
        padding = ng.vec2_zero
    }
}
ns.edit.command_text_colon = ng.add {
    transform = {
        parent = ns.edit.command_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_TABLE,
        valign = ng.GA_MAX,
        padding = ng.vec2(2, 2)
    },
    gui_text = {
        str = ':'
    }
}
ns.edit.command_text = ng.add {
    transform = {
        parent = ns.edit.command_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_TABLE,
        valign = ng.GA_MAX,
        padding = ng.vec2(0, 2)
    },
    gui_text = {
        str = ''
    },
    gui_textedit = {}
}
ns.edit.command_completions_text = ng.add {
    transform = {
        parent = ns.edit.command_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui = {
        color = ng.color(0.5, 0.30, 0.05, 1.0),
        halign = ng.GA_TABLE,
        valign = ng.GA_MAX,
        padding = ng.vec2(0, 2)
    },
    gui_text = {
        str = 'completion1 | completion2'
    },
    gui_textedit = {}
}

-- hot_require 'nekogame.edit_inspector'

--- edit_field -----------------------------------------------------------------

local field_types = {}

-- edit_field makes it easy to provide editors for simple properties of various
-- types (Vec2, Scalar etc.)

-- 'args' may contain:
--      field_type: the field type (boolean, string, Scalar, enum, Vec2,
--                                  Color, NativeEntity)
--      ctype: the name of the underlying C type, if any
--      parent: the parent GUI entity
--      label: descriptive label (optional)
--      halign, valign: alignment for container (optional)
function ng.edit_field_create(args)
    assert(args.field_type, 'field type expected')
    local field = field_types[args.field_type].create(args)
    field.type = args.field_type
    return field
end

-- updates display to val
function ng.edit_field_update(field, val)
    local f = field_types[field.type].update
    if f then
        f(field, val)
    end
end

-- updates display to val, calls setter with new value if edited
function ng.edit_field_post_update(field, ...)
    local f = field_types[field.type].post_update
    if f then
        f(field, unpack({...}))
    end
end

-- returns textbox, text
local function field_create_textbox(args)
    local textbox = ng.add {
        transform = {
            parent = args.field.container
        },
        gui = {
            color = ng.color(0.2, 0.2, 0.4, 1),
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_textbox = {}
    }

    local text = ns.gui_textbox.get_text(textbox)
    if args.editable == nil or args.editable or args.numerical then
        ns.gui_textedit.add(text)
        ns.gui_textbox.set_click_focus(textbox, true)
    end
    if args.numerical then
        ns.gui_textedit.set_numerical(text, true)
    end

    return textbox, text
end

local function field_create_common(args)
    local field = {}

    field.container = ng.add {
        transform = {
            parent = args.parent
        },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color_clear,
            valign = args.valign or ng.GA_TABLE,
            halign = args.halign or ng.GA_MIN
        },
        gui_rect = {
            hfill = true
        }
    }

    if args.label then
        field.name = args.label
        field.label = ng.add {
            transform = {
                parent = field.container
            },
            gui = {
                color = ng.color_white,
                valign = ng.GA_MID,
                halign = ng.GA_TABLE
            },
            gui_text = {
                str = args.label
            }
        }
    end

    return field
end

field_types['boolean'] = {
    create = function(args)
        local field = field_create_common(args)
        field.checkbox = ng.add {
            transform = {
                parent = field.container
            },
            gui = {
                color = ng.color(0.2, 0.2, 0.4, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_checkbox = {}
        }
        return field
    end,

    post_update = function(field, val, setter)
        if ns.gui.event_changed(field.checkbox) then
            setter(ns.gui_checkbox.get_checked(field.checkbox))
            ns.edit.undo_save()
        else
            ns.gui_checkbox.set_checked(field.checkbox, val)
        end
    end
}

field_types['string'] = {
    create = function(args)
        local field = field_create_common(args)
        field.textbox, field.textedit = field_create_textbox {
            field = field
        }
        return field
    end,

    post_update = function(field, val, setter)
        if ns.gui.event_focus_exit(field.textedit) then
            ns.edit.undo_save()
        end
        if ns.gui.event_changed(field.textedit) then
            setter(ns.gui_text.get_str(field.textedit))
        elseif not ns.gui.get_focus(field.textedit) then
            ns.gui_text.set_str(field.textedit, val)
        end
    end
}

field_types['Scalar'] = {
    create = function(args)
        local field = field_create_common(args)
        field.textbox, field.textedit = field_create_textbox {
            field = field,
            numerical = true
        }
        return field
    end,

    post_update = function(field, val, setter)
        if ns.gui.event_focus_exit(field.textedit) then
            ns.edit.undo_save()
        end
        if ns.gui.event_changed(field.textedit) then
            setter(ns.gui_textedit.get_num(field.textedit))
        elseif not ns.gui.get_focus(field.textedit) then
            ns.gui_text.set_str(field.textedit, string.format('%.4f', val))
        end
    end
}

-- if it's a C enum field the C enum values are automatically used, else a
-- set of values must be provided as an extra parameter to
-- ng.edit_field_post_update(...)
field_types['enum'] = {
    create = function(args)
        local field = field_create_common(args)
        field.enumtype = args.ctype
        field.textbox, field.text = field_create_textbox {
            field = field,
            editable = false
        }
        return field
    end,

    post_update = function(field, val, setter, vals)
        if ns.gui.event_mouse_down(field.textbox) == ng.MC_LEFT then
            local function setter_wrap(s)
                setter(s)
                ns.edit.undo_save()
            end
            vals = field.enumtype and ng.enum_values(field.enumtype) or vals
            local comp = ns.edit.command_completion_substr(vals)
            local prompt = 'set ' .. (field.name or '') .. ': '
            ns.edit.command_start(prompt, setter_wrap, comp, true)
        end
        val = field.enumtype and ng.enum_tostring(field.enumtype, val) or val
        ns.gui_text.set_str(field.text, val)
    end
}

field_types['Vec2'] = {
    create = function(args)
        local field = field_create_common(args)
        field.x_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.y_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.x_field, val.x, function(x)
            setter(ng.vec2(x, val.y))
        end)
        ng.edit_field_post_update(field.y_field, val.y, function(y)
            setter(ng.vec2(val.x, y))
        end)
    end
}

field_types['Color'] = {
    create = function(args)
        local field = field_create_common(args)
        field.r_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.g_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.b_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.a_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.r_field, val.r, function(r)
            setter(ng.color(r, val.g, val.b, val.a))
        end)
        ng.edit_field_post_update(field.g_field, val.g, function(g)
            setter(ng.color(val.r, g, val.b, val.a))
        end)
        ng.edit_field_post_update(field.b_field, val.b, function(b)
            setter(ng.color(val.r, val.g, b, val.a))
        end)
        ng.edit_field_post_update(field.a_field, val.a, function(a)
            setter(ng.color(val.r, val.g, val.b, a))
        end)
    end
}

field_types['NativeEntity'] = {
    create = function(args)
        local field = field_create_common(args)
        field.enumtype = args.ctype
        field.textbox, field.text = field_create_textbox {
            field = field,
            editable = false
        }

        -- 'set' button
        field.pick = ng.add {
            transform = {
                parent = field.container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(field.pick), 'set')

        return field
    end,

    post_update = function(field, val, setter)
        -- pick new value?
        if ns.gui.event_mouse_down(field.pick) == ng.MC_LEFT then
            val = ns.edit.select_get_first() or ng.entity_nil
            setter(val)
            ns.edit.undo_save()
        end

        -- display, select on click
        ns.gui_text.set_str(field.text, val == ng.entity_nil and '(nil)' or string.format('[%d]', val.id))
        if ns.gui.event_mouse_down(field.textbox) == ng.MC_LEFT and val ~= ng.entity_nil then
            ns.edit.select_clear()
            ns.edit.select[val] = true
        end
    end
}

field_types['BBox'] = {
    create = function(args)
        local field = field_create_common(args)
        field.min_field = ng.edit_field_create {
            field_type = 'Vec2',
            ctype = 'Vec2',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.max_field = ng.edit_field_create {
            field_type = 'Vec2',
            ctype = 'Vec2',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.min_field, val.min, function(m)
            setter(ng.bbox(m, val.max))
        end)
        ng.edit_field_post_update(field.max_field, val.max, function(m)
            setter(ng.bbox(val.min, m))
        end)
    end
}

--- inspector ------------------------------------------------------------------

ns.edit_inspector = {
    inspect = false
}

ns.edit_inspector.custom = {} -- custom inspectors -- eg. for physics

local inspectors = ng.entity_table() -- NativeEntity (sys --> inspector) map

ns.edit_inspector.gui_root = ng.add {
    transform = {
        parent = ns.edit.gui_root
    },
    group = {
        groups = 'builtin edit_inspector'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true,
        vfill = true
    },
    gui = {
        captures_events = false,
        color = ng.color(0, 0, 0, 0), -- invisible
        halign = ng.GA_MIN,
        valign = ng.GA_MAX,
        padding = ng.vec2_zero
    }
}

-- forward event to custom event handler
local function custom_event(inspector, evt)
    local custom = ns.edit_inspector.custom[inspector.sys]
    if custom then
        local f = ns.edit_inspector.custom[inspector.sys][evt]
        if f then
            f(inspector)
        end
    end
end

-- returns field type, C type
local function property_type(inspector, name)
    local r = ng.get(inspector.sys, name, inspector.ent)

    local field_type = type(r)
    local ctype = nil
    if field_type == 'cdata' then
        local refctt = refct.typeof(r)
        ctype = refctt.name
        if refctt.what == 'enum' then
            field_type = 'enum'
        else
            field_type = ctype
        end
    end
    if field_type == 'number' then
        field_type = 'Scalar'
    end

    return field_type, ctype
end

-- add field for property
local function add_property(inspector, name)
    if inspector.props[name] then
        return
    end -- already exists

    local field_type, ctype = property_type(inspector, name)
    if not field_type or not field_types[field_type] then
        return
    end

    inspector.props[name] = {
        name = name,
        field = ng.edit_field_create {
            field_type = field_type,
            ctype = ctype,
            parent = inspector.window_body,
            label = name
        }
    }
end

-- add all properties for an inspector, either through ns.meta.props or
-- through automatic discovery
local function add_properties(inspector)
    if ns.meta.props[inspector.sys] then
        for _, p in ipairs(ns.meta.props[inspector.sys]) do
            add_property(inspector, p.name)
        end
    elseif rawget(ns, inspector.sys) then
        for f in pairs(ns[inspector.sys]) do
            if string.sub(f, 1, 4) == 'set_' then
                local prop = string.sub(f, 5, string.len(f))
                if ns[inspector.sys]['get_' .. prop] then
                    add_property(inspector, prop)
                end
            end
        end
    end
end

-- return inspector object
local function make_inspector(ent, sys)
    local inspector = {}

    inspector.ent = ng.NativeEntity(ent)
    inspector.sys = sys

    -- put near entity initially
    local pos = ng.vec2(16, -16)
    if ns.transform.has(ent) then
        pos = ns.transform.local_to_world(ent, ng.vec2_zero)
        pos = ns.transform.world_to_local(ns.edit_inspector.gui_root, pos) + ng.vec2(22, -10)
    end

    -- window
    inspector.window = ng.add {
        transform = {
            parent = ns.edit_inspector.gui_root,
            position = pos
        },
        gui_window = {},
        gui = {}
    }
    inspector.last_pos = pos
    inspector.window_body = ns.gui_window.get_body(inspector.window)
    inspector.docked = false

    -- dock toggle button
    inspector.dock_text = ng.add {
        transform = {
            parent = ns.gui_window.get_title_buttons_area(inspector.window)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_text = {
            str = '<'
        }
    }

    -- 'remove' button
    inspector.remove_text = ng.add {
        transform = {
            parent = ns.gui_window.get_title_buttons_area(inspector.window)
        },
        gui = {
            color = ng.color_red,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_text = {
            str = 'r'
        }
    }

    -- property fields
    inspector.props = {}
    add_properties(inspector)

    custom_event(inspector, 'add')
    return inspector
end

-- add a sys inspector for NativeEntity ent
function ns.edit_inspector.add(ent, sys)
    local adder = ns[sys].add
    if not adder then
        error("system '" .. sys .. "' has no 'add(...)' function")
    end

    if not inspectors[ent] then
        inspectors[ent] = {}
    end

    if inspectors[ent][sys] then
        return
    end
    if not ns[sys].has(ent) then
        adder(ent)
    end
    inspectors[ent][sys] = make_inspector(ent, sys)
end

-- remove sys inspector for NativeEntity ent -- sys is optional, removes all
-- inspectors on ent if not specified
function ns.edit_inspector.remove(ent, sys)
    if not inspectors[ent] then
        return
    end

    if not sys then
        for sys in pairs(inspectors[ent]) do
            ns.edit_inspector.remove(ent, sys)
        end
        inspectors[ent] = nil
        return
    end

    custom_event(inspectors[ent][sys], 'remove')
    ns.gui_window.remove(inspectors[ent][sys].window)
    inspectors[ent][sys] = nil
end

-- return set of all valid inspector systems
function ns.edit_inspector.get_systems()
    local sys = {}
    -- system must either have property metadata or an 'add(...)' function
    for k in pairs(ns.meta.props) do
        sys[k] = true
    end
    for k in pairs(ns) do
        if ns[k].inspect ~= false and ns[k].add then
            sys[k] = true
        end
    end
    return sys
end

local function remove_destroyed()
    -- if closed a window, save an undo point
    local some_closed = false

    for ent, insps in pairs(inspectors) do
        if ns.entity.destroyed(ent) then
            ns.edit_inspector.remove(ent)
        else
            for _, inspector in pairs(insps) do
                if ns.gui.event_mouse_down(inspector.remove_text) == ng.MC_LEFT then
                    ns[inspector.sys].remove(inspector.ent)
                    ns.edit_inspector.remove(inspector.ent, inspector.sys)
                    some_closed = true
                elseif ns.entity.destroyed(inspector.window) or not ns[inspector.sys].has(ent) then
                    ns.edit_inspector.remove(inspector.ent, inspector.sys)
                    some_closed = true
                end
            end
        end
    end

    if some_closed then
        ns.edit.undo_save()
    end
end

-- make entities uneditable/unsaveable etc. recursively
local function update_group_editable_rec(ent)
    ns.edit.set_editable(ent, false)
    ns.group.set_groups(ent, 'builtin edit_inspector')

    if ns.transform.has(ent) then
        local children = ns.transform.get_children(ent)
        for i = 0, ns.transform.get_num_children(ent) - 1 do
            update_group_editable_rec(children[i])
        end
    end
end

local function update_inspector(inspector)
    ns.transform.set_parent(inspector.window, ns.edit_inspector.gui_root)

    ns.gui_window.set_highlight(inspector.window, ns.edit.select[inspector.ent])
    local title = inspector.sys
    ns.gui_window.set_title(inspector.window, title)

    -- docking
    if ns.gui.event_mouse_down(inspector.dock_text) == ng.MC_LEFT then
        inspector.docked = not inspector.docked
        if not inspector.docked then
            ns.transform.set_position(inspector.window, inspector.last_pos)
        end
    end
    if inspector.docked then
        ns.gui.set_halign(inspector.window, ng.GA_MAX)
        ns.gui.set_valign(inspector.window, ng.GA_TABLE)
        ns.gui_text.set_str(inspector.dock_text, '<')
    else
        inspector.last_pos = ns.transform.get_position(inspector.window)
        ns.gui.set_halign(inspector.window, ng.GA_NONE)
        ns.gui.set_valign(inspector.window, ng.GA_NONE)
        ns.gui_text.set_str(inspector.dock_text, '>')
    end

    add_properties(inspector) -- capture newly added properties

    update_group_editable_rec(inspector.window)

    for _, prop in pairs(inspector.props) do
        ng.edit_field_update(prop.field, ng.get(inspector.sys, prop.name, inspector.ent))
    end
    custom_event(inspector, 'update')
end

function ns.edit_inspector.update_all()
    ns.transform.set_parent(ns.edit_inspector.gui_root, ns.edit.gui_root)
    remove_destroyed()
    if not ns.edit.get_enabled() then
        return
    end

    for _, insps in pairs(inspectors) do
        for _, inspector in pairs(insps) do
            update_inspector(inspector)
        end
    end
end

local function post_update_inspector(inspector)
    -- draw line from inspector to target entity
    if ns.transform.has(inspector.ent) then
        local a = ns.transform.local_to_world(inspector.window, ng.vec2(0, -16))
        local b = ns.transform.local_to_world(inspector.ent, ng.vec2_zero)
        ns.edit.line_add(a, b, 0, ng.color(1, 0, 1, 0.6))
    end

    for _, prop in pairs(inspector.props) do
        ng.edit_field_post_update(prop.field, ng.get(inspector.sys, prop.name, inspector.ent), function(val)
            ng.set(inspector.sys, prop.name, inspector.ent, val)
        end)
    end
    custom_event(inspector, 'post_update')
end

function ns.edit_inspector.post_update_all()

    -- print("ns.edit_inspector.post_update_all")

    remove_destroyed()
    if not ns.edit.get_enabled() then
        return
    end

    for _, insps in pairs(inspectors) do
        for _, inspector in pairs(insps) do
            post_update_inspector(inspector)
        end
    end
end

function ns.edit_inspector.save_all()
    local data = {}

    if ns.entity.get_save_filter(ns.edit_inspector.gui_root) then
        data.gui_root = ns.edit_inspector.gui_root
    end

    data.tbl = ng.entity_table()
    for _, insps in pairs(inspectors) do
        for _, inspector in pairs(insps) do
            if ns.entity.get_save_filter(inspector.ent) then
                data.tbl[inspector.window] = inspector
            end
        end
    end

    return data
end
function ns.edit_inspector.load_all(data)
    if data.gui_root then
        ns.edit_inspector.gui_root = data.gui_root
    end

    for win, inspector in pairs(data.tbl) do
        if not inspectors[inspector.ent] then
            inspectors[inspector.ent] = {}
        end
        inspectors[inspector.ent][inspector.sys] = inspector
    end
end

--- C system properties --------------------------------------------------------

ns.meta.props['transform'] = {{
    name = 'parent'
}, {
    name = 'position'
}, {
    name = 'rotation'
}, {
    name = 'scale'
}}

ns.meta.props['camera'] = {{
    name = 'current'
}, {
    name = 'viewport_height'
}}

ns.meta.props['sprite'] = {{
    name = 'size'
}, {
    name = 'texcell'
}, {
    name = 'texsize'
}, {
    name = 'depth'
}}

ns.meta.props['physics'] = {{
    name = 'type'
}, {
    name = 'mass'
}, {
    name = 'freeze_rotation'
}, {
    name = 'velocity'
}, {
    name = 'force'
}, {
    name = 'angular_velocity'
}, {
    name = 'torque'
}, {
    name = 'velocity_limit'
}, {
    name = 'angular_velocity_limit'
}}

ns.meta.props['gui'] = {{
    name = 'color'
}, {
    name = 'visible'
}, {
    name = 'focusable'
}, {
    name = 'captures_events'
}, {
    name = 'halign'
}, {
    name = 'valign'
}, {
    name = 'padding'
}}
ns.meta.props['gui_rect'] = {{
    name = 'size'
}, {
    name = 'hfit'
}, {
    name = 'vfit'
}, {
    name = 'hfill'
}, {
    name = 'vfill'
}}
ns.meta.props['gui_text'] = {{
    name = 'str'
}}
ns.meta.props['gui_textedit'] = {{
    name = 'cursor'
}, {
    name = 'numerical'
}}

ns.meta.props['sound'] = {{
    name = 'path'
}, {
    name = 'playing'
}, {
    name = 'seek'
}, {
    name = 'finish_destroy'
}, {
    name = 'loop'
}, {
    name = 'gain'
}}

-- system-specific
-- hot_require 'nekogame.edit_entity'
local ffi = FFI

function ns.edit.destroy_rec()
    for ent in pairs(ns.edit.select) do
        ns.transform.destroy_rec(ent)
    end

    ns.edit.undo_save()
end

function ns.edit.destroy()
    for ent in pairs(ns.edit.select) do
        ns.entity.destroy(ent)
    end

    ns.edit.undo_save()
end

function ns.edit.duplicate()
    -- save just current selection to a string
    for ent in pairs(ns.edit.select) do
        if ns.transform.has(ent) then
            ns.transform.set_save_filter_rec(ent, true)
        else
            ns.entity.set_save_filter(ent, true)
        end
    end
    local s = ng.store_open()
    ns.system.save_all(s)
    local str = ffi.string(ng.store_write_str(s))
    ng.store_close(s)

    -- clear selection
    ns.edit.select_clear()

    -- load from the string -- they were selected on save and so will be
    -- selected when loaded
    local d = ng.store_open_str(str)
    ns.system.load_all(d)
    ng.store_close(d)

    ns.edit.undo_save()
end

-- hot_require 'nekogame.edit_transform'
--- grab -----------------------------------------------------------------------
local grab_old_pos, grab_mouse_start
local grab_disp -- 'extra' displacement on top of mouse motion
local grab_snap -- whether snapping to grid

function ns.edit.grab_start()
    ns.edit.set_mode('grab')
end
function ns.edit.grab_end()
    ns.edit.set_mode('normal')
    ns.edit.undo_save()
end
function ns.edit.grab_cancel()
    for ent in pairs(ns.edit.select) do
        ns.transform.set_position(ent, grab_old_pos[ent])
    end
    ns.edit.set_mode('normal')
end

function ns.edit.grab_snap_on()
    grab_snap = true
end
function ns.edit.grab_snap_off()
    grab_snap = false
end

-- move all selected grid size times mult in a direction
function ns.edit.grab_move_left(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.x = grab_disp.x - (mult or 1) * (g.x > 0 and g.x or 1)
end
function ns.edit.grab_move_right(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.x = grab_disp.x + (mult or 1) * (g.x > 0 and g.x or 1)
end
function ns.edit.grab_move_up(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.y = grab_disp.y + (mult or 1) * (g.y > 0 and g.y or 1)
end
function ns.edit.grab_move_down(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.y = grab_disp.y - (mult or 1) * (g.y > 0 and g.y or 1)
end

ns.edit.modes.grab = {}

function ns.edit.modes.grab.enter()
    grab_mouse_start = ns.input.get_mouse_pos_unit()
    grab_disp = ng.Vec2(ng.vec2_zero)
    grab_snap = false

    -- store old positions
    grab_old_pos = ng.entity_table()
    for ent in pairs(ns.edit.select) do
        grab_old_pos[ent] = ns.transform.get_position(ent)
    end
end

function ns.edit.modes.grab.update_all()
    local ms = ns.camera.unit_to_world(grab_mouse_start)
    local mc = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())

    -- snap mc to grid if needed
    if grab_snap then
        local g = ns.edit.get_grid_size()
        if g.x > 0 then
            mc.x = g.x * math.floor(0.5 + (mc.x - ms.x) / g.x) + ms.x
        end
        if g.y > 0 then
            mc.y = g.y * math.floor(0.5 + (mc.y - ms.y) / g.y) + ms.y
        end
    end

    -- move selected objects
    for ent in pairs(ns.edit.select) do
        -- move only if no ancestor is being moved (avoid double-move)
        local anc = ns.transform.get_parent(ent)
        while anc ~= ng.entity_nil and not ns.edit.select[anc] do
            anc = ns.transform.get_parent(anc)
        end
        if anc == ng.entity_nil then
            -- find translation in parent space
            local parent = ns.transform.get_parent(ent)
            local m = ng.mat3_inverse(ns.transform.get_world_matrix(parent))
            local d = ng.mat3_transform(m, mc) - ng.mat3_transform(m, ms)
            d = d + ng.mat3_transform(m, grab_disp) - ng.mat3_transform(m, ng.vec2_zero)
            ns.transform.set_position(ent, grab_old_pos[ent] + d)
        end
    end

    -- update
    local snap_text = grab_snap and 'snap ' or ''
    local d = mc - ms + grab_disp
    local mode_text = string.format('grab %s%.4f, %.4f', snap_text, d.x, d.y)
    ns.edit.set_mode_text(mode_text)
end

--- rotate ---------------------------------------------------------------------

local rotate_old_posrot, rotate_mouse_start, rotate_pivot

function ns.edit.rotate_start()
    ns.edit.set_mode('rotate')
end
function ns.edit.rotate_end()
    ns.edit.set_mode('normal')
    ns.edit.undo_save()
end
function ns.edit.rotate_cancel()
    for ent in pairs(ns.edit.select) do
        ns.transform.set_position(ent, rotate_old_posrot[ent].pos)
        ns.transform.set_rotation(ent, rotate_old_posrot[ent].rot)
    end
    ns.edit.set_mode('normal')
end

ns.edit.modes.rotate = {}

function ns.edit.modes.rotate.enter()
    ns.edit.set_mode_text('rotate')

    rotate_mouse_start = ns.input.get_mouse_pos_unit()

    -- store old positions, rotations
    rotate_old_posrot = ng.entity_table()
    for ent in pairs(ns.edit.select) do
        rotate_old_posrot[ent] = {
            pos = ns.transform.get_position(ent),
            rot = ns.transform.get_rotation(ent)
        }
    end

    -- compute pivot (currently just the median)
    local n = 0
    rotate_pivot = ng.vec2_zero
    for ent in pairs(ns.edit.select) do
        rotate_pivot = rotate_pivot + ns.transform.get_world_position(ent)
        n = n + 1
    end
    rotate_pivot = rotate_pivot / n
end

function ns.edit.modes.rotate.update_all()
    local ms = ns.camera.unit_to_world(rotate_mouse_start)
    local mc = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    local ang = ng.vec2_atan2(mc - rotate_pivot) - ng.vec2_atan2(ms - rotate_pivot)

    for ent in pairs(ns.edit.select) do
        -- set new rotation
        ns.transform.set_rotation(ent, rotate_old_posrot[ent].rot + ang)

        -- compute new position
        local parent = ns.transform.get_parent(ent)
        local m = ns.transform.get_world_matrix(parent)
        local wpos = ng.mat3_transform(m, rotate_old_posrot[ent].pos)
        local d = wpos - rotate_pivot
        d = ng.vec2_rot(d, ang)
        wpos = rotate_pivot + d

        -- set new position
        local im = ng.mat3_inverse(m)
        ns.transform.set_position(ent, ng.mat3_transform(im, wpos))
    end
end

-- hot_require 'nekogame.edit_physics'
--- custom inspector -----------------------------------------------------------
local function add_circle(ent, bbox)
    local r = 0.5 * math.min(bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y)
    local mid = 0.5 * (bbox.max + bbox.min)

    local function add_r(rs)
        local function add_midx(midxs)
            local function add_midy(midys)
                ns.physics.shape_add_circle(ent, tonumber(rs) or 0, ng.vec2(tonumber(midxs) or 0, tonumber(midys) or 0))
            end
            ns.edit.command_start('offset y: ', add_midy, nil, false, tostring(mid.y))
        end
        ns.edit.command_start('offset x: ', add_midx, nil, false, tostring(mid.x))
    end

    ns.edit.command_start('radius: ', add_r, nil, false, tostring(r))
end

local function add_box(ent, bbox)
    local mindim = math.min(bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y)

    local function add(s)
        -- reduce bbox size to account for radius, then add
        local r = math.min(tonumber(s) or 0, 0.5 * mindim)
        local bbox2 = ng.bbox(ng.vec2(bbox.min.x + r, bbox.min.y + r), ng.vec2(bbox.max.x - r, bbox.max.y - r))
        ns.physics.shape_add_box(ent, bbox2, r)
    end

    ns.edit.command_start('rounding radius: ', add)
end

local function add_poly(ent)
    local function add(s)
        ns.edit.phypoly_start(ent, tonumber(s) or 0)
    end
    ns.edit.command_start('rounding radius: ', add)
end

ns.edit_inspector.custom['physics'] = {
    add = function(inspector)
        -- add buttons
        inspector.add_container = ng.add {
            transform = {
                parent = inspector.window_body
            },
            gui = {
                padding = ng.vec2_zero,
                color = ng.color_clear,
                valign = ng.GA_TABLE,
                halign = ng.GA_MIN
            },
            gui_rect = {
                hfill = true
            }
        }

        inspector.add_box = ng.add {
            transform = {
                parent = inspector.add_container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_box), 'add box')

        inspector.add_poly = ng.add {
            transform = {
                parent = inspector.add_container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_poly), 'add poly')

        inspector.add_circle = ng.add {
            transform = {
                parent = inspector.add_container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_circle), 'add circle')

        inspector.shapes = {}
    end,

    post_update = function(inspector)
        ns.physics.debug_draw(inspector.ent)

        -- 'add box' button
        if ns.gui.event_mouse_down(inspector.add_box) == ng.MC_LEFT then
            local bbox
            if ns.edit.bboxes_has(inspector.ent) then
                bbox = ns.edit.bboxes_get(inspector.ent)
            else
                bbox = ng.bbox(ng.vec2(-1, -1), ng.vec2(1, 1))
            end
            add_box(inspector.ent, bbox)
            ns.edit.undo_save()
        end

        -- 'add poly' button
        if ns.gui.event_mouse_down(inspector.add_poly) == ng.MC_LEFT then
            add_poly(inspector.ent)
        end

        -- 'add circle' button
        if ns.gui.event_mouse_down(inspector.add_circle) == ng.MC_LEFT then
            local bbox
            if ns.edit.bboxes_has(inspector.ent) then
                bbox = ns.edit.bboxes_get(inspector.ent)
            else
                bbox = ng.bbox(ng.vec2(-1, -1), ng.vec2(1, 1))
            end
            add_circle(inspector.ent, bbox)
            ns.edit.undo_save()
        end

        local nshapes = ns.physics.get_num_shapes(inspector.ent)

        while nshapes > #inspector.shapes do
            local shape = {}
            table.insert(inspector.shapes, shape)
            shape.window = ng.add {
                transform = {
                    parent = inspector.window_body
                },
                gui_window = {},
                gui_rect = {
                    hfill = true
                },
                gui = {
                    valign = ng.GA_TABLE,
                    halign = ng.GA_MIN
                }
            }
            shape.window_body = ns.gui_window.get_body(shape.window)

            shape.poly_container = ng.add {
                transform = {
                    parent = shape.window_body
                },
                gui = {
                    padding = ng.vec2_zero,
                    color = ng.color_clear,
                    valign = ng.GA_TABLE,
                    halign = ng.GA_MIN
                },
                gui_rect = {
                    hfill = true
                }
            }

            shape.poly_info = ng.add {
                transform = {
                    parent = shape.poly_container
                },
                gui = {
                    color = ng.color_white,
                    valign = ng.GA_MID,
                    halign = ng.GA_TABLE
                },
                gui_text = {}
            }
        end

        while nshapes < #inspector.shapes do
            local shape = table.remove(inspector.shapes)
            ns.gui_window.remove(shape.window)
        end

        for i = #inspector.shapes, 1, -1 do -- backwards for safe remove
            local shape = inspector.shapes[i]
            if ns.entity.destroyed(shape.window) then
                ns.physics.shape_remove(inspector.ent, i - 1)
                table.remove(inspector.shapes, i)
            else
                local t = ns.physics.shape_get_type(inspector.ent, i - 1)
                ns.gui_window.set_title(shape.window, ng.enum_tostring('PhysicsShape', t))
                if t == ng.PS_POLYGON then
                    local n = ns.physics.poly_get_num_verts(inspector.ent, i - 1)
                    ns.gui_text.set_str(shape.poly_info, n .. ' vertices')
                end
            end
        end
    end
}

--- phypoly mode (draw polygon shape) ------------------------------------------

local phypoly_ent, phypoly_verts, phypoly_radius

ns.edit.modes.phypoly = {}

local function phypoly_update_verts()
    if #phypoly_verts >= 4 then
        phypoly_verts = ns.physics.convex_hull(phypoly_verts)
    end
end

function ns.edit.phypoly_start(ent, radius)
    ns.edit.set_mode('phypoly')
    phypoly_ent = ent
    phypoly_verts = {}
    phypoly_radius = radius or 0
end
function ns.edit.phypoly_end()
    if #phypoly_verts < 3 then
        return
    end

    ns.edit.set_mode('normal')
    ns.physics.shape_add_poly(phypoly_ent, phypoly_verts, phypoly_radius)
    phypoly_ent = nil
    phypoly_verts = nil
    ns.edit.undo_save()
end
function ns.edit.phypoly_cancel()
    ns.edit.set_mode('normal')
    phypoly_ent = nil
    phypoly_verts = nil
end

function ns.edit.phypoly_add_vertex()
    local m = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    -- TODO: remove scaling issue
    local t = ng.mat3_inverse(ns.transform.get_world_matrix(phypoly_ent))
    table.insert(phypoly_verts, ng.Vec2(ng.mat3_transform(t, m)))
    phypoly_update_verts()
end

function ns.edit.modes.phypoly.enter()
    ns.edit.set_mode_text('phypoly')
end

function ns.edit.modes.phypoly.update_all()
    if not ns.physics.has(phypoly_ent) then
        ns.edit.phypoly_cancel()
    end

    if #phypoly_verts > 1 then
        for i = 1, #phypoly_verts do
            local j = i < #phypoly_verts and i + 1 or 1
            local a = ns.transform.local_to_world(phypoly_ent, phypoly_verts[i])
            local b = ns.transform.local_to_world(phypoly_ent, phypoly_verts[j])
            ns.edit.line_add(a, b, 5, ng.color(1.0, 0.0, 0.0, 1.0))
        end
    end
end

-- default binds
-- hot_require 'nekogame.edit_binds'
-- normal mode
ns.edit.modes.normal['S-;'] = ns.edit.command_start
ns.edit.modes.normal['u'] = ns.edit.undo

ns.edit.modes.normal['s'] = ns.edit.command_save
ns.edit.modes.normal['l'] = ns.edit.command_load
ns.edit.modes.normal['\''] = ns.edit.command_save_prefab
ns.edit.modes.normal['.'] = ns.edit.command_load_prefab

ns.edit.modes.normal['p'] = ns.edit.pause_toggle
ns.edit.modes.normal['S-p'] = ns.edit.stop

ns.edit.modes.normal['a'] = ns.edit.select_clear
ns.edit.modes.normal['<mouse_1>'] = ns.edit.select_click_single
ns.edit.modes.normal['C-<mouse_1>'] = ns.edit.select_click_multi

ns.edit.modes.normal['x'] = ns.edit.destroy
ns.edit.modes.normal['S-x'] = ns.edit.destroy_rec
ns.edit.modes.normal['S-d'] = ns.edit.duplicate

ns.edit.modes.normal['S-<mouse_1>'] = ns.edit.camera_drag_start
ns.edit.modes.normal['^S-<mouse_1>'] = ns.edit.camera_drag_end
ns.edit.modes.normal['<mouse_3>'] = ns.edit.camera_drag_start
ns.edit.modes.normal['^<mouse_3>'] = ns.edit.camera_drag_end
ns.edit.modes.normal['-'] = ns.edit.camera_zoom_out
ns.edit.modes.normal['='] = ns.edit.camera_zoom_in

ns.edit.modes.normal['g'] = ns.edit.grab_start
ns.edit.modes.normal['r'] = ns.edit.rotate_start
ns.edit.modes.normal['b'] = ns.edit.boxsel_start

ns.edit.modes.normal[','] = ns.edit.command_inspect
ns.edit.modes.normal['S-g'] = ns.edit.command_grid

-- grab mode
ns.edit.modes.grab['<enter>'] = ns.edit.grab_end
ns.edit.modes.grab['<escape>'] = ns.edit.grab_cancel
ns.edit.modes.grab['<mouse_1>'] = ns.edit.grab_end
ns.edit.modes.grab['<mouse_2>'] = ns.edit.grab_cancel
ns.edit.modes.grab['g'] = ns.edit.grab_snap_on
ns.edit.modes.grab['<left>'] = ns.edit.grab_move_left
ns.edit.modes.grab['<right>'] = ns.edit.grab_move_right
ns.edit.modes.grab['<up>'] = ns.edit.grab_move_up
ns.edit.modes.grab['<down>'] = ns.edit.grab_move_down
ns.edit.modes.grab['S-<left>'] = function()
    ns.edit.grab_move_left(5)
end
ns.edit.modes.grab['S-<right>'] = function()
    ns.edit.grab_move_right(5)
end
ns.edit.modes.grab['S-<up>'] = function()
    ns.edit.grab_move_up(5)
end
ns.edit.modes.grab['S-<down>'] = function()
    ns.edit.grab_move_down(5)
end

-- rotate mode
ns.edit.modes.rotate['<enter>'] = ns.edit.rotate_end
ns.edit.modes.rotate['<escape>'] = ns.edit.rotate_cancel
ns.edit.modes.rotate['<mouse_1>'] = ns.edit.rotate_end
ns.edit.modes.rotate['<mouse_2>'] = ns.edit.rotate_cancel

-- boxsel mode
ns.edit.modes.boxsel['<mouse_1>'] = ns.edit.boxsel_begin
ns.edit.modes.boxsel['C-<mouse_1>'] = ns.edit.boxsel_begin
ns.edit.modes.boxsel['^<mouse_1>'] = ns.edit.boxsel_end
ns.edit.modes.boxsel['^C-<mouse_1>'] = ns.edit.boxsel_end_add

-- phypoly mode
ns.edit.modes.phypoly['<enter>'] = ns.edit.phypoly_end
ns.edit.modes.phypoly['<escape>'] = ns.edit.phypoly_cancel
ns.edit.modes.phypoly['<mouse_1>'] = ns.edit.phypoly_add_vertex

--- main events ----------------------------------------------------------------

function ns.edit.key_up(key)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_key_up(key)
end
function ns.edit.key_down(key)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_key_down(key)
end
function ns.edit.mouse_down(mouse)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_mouse_down(mouse)
end
function ns.edit.mouse_up(mouse)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_mouse_up(mouse)
end
function ns.edit.scroll(scroll)
    ns.edit.camera_zoom((scroll.y > 0 and 0.9 or -0.9) + 0.1 * scroll.y)
end

function ns.edit.update_all()
    for ent in pairs(ns.edit.select) do
        if ns.entity.destroyed(ent) then
            ns.edit.select[ent] = nil
        end
    end

    if ns.gui.event_mouse_down(ns.edit.play_text) == ng.MC_LEFT then
        if ns.edit.stopped then
            ns.edit.play()
        else
            ns.edit.stop()
        end
    end
    if not ns.timing.get_paused() then
        ns.edit.stopped = false
    end

    -- if not enabled skip -- also handle gui visibility
    if not ns.edit.get_enabled() then
        ns.gui.set_visible(ns.edit.gui_root, false)
        return
    end
    ns.gui.set_visible(ns.edit.gui_root, true)

    -- forward to mode
    ns.edit.mode_event('update_all')

    -- update grid text
    local g = ns.edit.get_grid_size()
    if g.x <= 0 and g.y <= 0 then
        ns.gui.set_visible(ns.edit.grid_textbox, false)
    else
        ns.gui.set_visible(ns.edit.grid_textbox, true)
        local s
        if g.x == g.y then
            s = string.format('grid %.4g', g.x)
        else
            s = string.format('grid %.4g %.4g', g.x, g.y)
        end
        ns.gui_text.set_str(ns.edit.grid_text, s)
    end

    -- update select text
    local nselect = 0
    for _ in pairs(ns.edit.select) do
        nselect = nselect + 1
    end
    if nselect > 0 then
        ns.gui.set_visible(ns.edit.select_textbox, true)
        ns.gui_text.set_str(ns.edit.select_text, 'select ' .. nselect)
    else
        ns.gui.set_visible(ns.edit.select_textbox, false)
        ns.gui_text.set_str(ns.edit.select_text, '')
    end

    -- update play/stop text
    if ns.edit.stopped then
        ns.gui_text.set_str(ns.edit.play_text, '\x10')
    else
        ns.gui_text.set_str(ns.edit.play_text, '\xcb')
    end
end

function ns.edit.post_update_all()

    -- print("ns.edit.post_update_all")

    ns.edit.mode_event('post_update_all')

    -- update bbox highlight
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        local bbox = ns.edit.bboxes_get_nth_bbox(i)
        ns.edit.bboxes_set_selected(ent, ns.edit.select[ent] ~= nil)
    end

    -- save stop?
    if stop_save_next_frame then
        stop_save()
        stop_save_next_frame = false
    end
end

function ns.edit.save_all()
    return {
        sel = ns.edit.select
    }
end
function ns.edit.load_all(d)
    ng.entity_table_merge(ns.edit.select, d.sel)
end

-- hot_require 'nekogame.animation'
ns.animation = {
    auto_saveload = true
}

ns.animation.tbl = ng.entity_table()

function ns.animation.add(ent)
    if ns.animation.tbl[ent] then
        return
    end

    ns.animation.tbl[ent] = {
        ent = ent,
        anims = {},
        curr_anim = nil, -- name of animation
        frame = 1, -- index of frame
        t = 1 -- time left in this frame
    }
end
function ns.animation.remove(ent, anim)
    local entry = ns.animation.tbl[ent]
    if entry then
        if anim then
            if entry.curr_anim == anim then
                entry.curr_anim = nil
            end
            entry.anims[anim] = nil
        else
            ns.animation.tbl[ent] = nil
        end
    end
end
function ns.animation.has(ent)
    return ns.animation.tbl[ent] ~= nil
end

-- utility for contiguous strips of frames
function ns.animation.set_strips(ent, tbl)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    for anim, strip in pairs(tbl) do
        entry.anims[anim] = {
            n = strip.n,
            strip = strip
        }
    end
end

-- manual specification of every frame and its duration
function ns.animation.set_frames(ent, tbl)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    for anim, frames in pairs(anim) do
        entry.anims[anim] = {
            n = #frames,
            frames = frames
        }
    end
end

local function _enter_frame(entry, frame, anim)
    anim = anim or entry.anims[entry.curr_anim]
    entry.frame = frame
    if anim.strip then
        entry.t = anim.strip.t > 0 and anim.strip.t or 1
        local v = ng.Vec2(anim.strip.base)
        v.x = v.x + (frame - 1) * ns.sprite.get_texsize(entry.ent).x
        ns.sprite.set_texcell(entry.ent, v)
    elseif anim.frames then
        local frm = anim.frames[frame]
        entry.t = frm.t > 0 and frm.t or 1
        if frm.texcell then
            ns.sprite.set_texcell(entry.ent, frm.texcell)
        end
        if frm.texsize then
            ns.sprite.set_texsize(entry.ent, frm.texsize)
        end
    end
end

function ns.animation.get_curr_anim(ent, anim)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    return entry.curr_anim
end

function ns.animation.switch(ent, anim)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    assert(entry.anims[anim], "must have an animation with name '" .. anim .. "'")
    if entry.curr_anim ~= anim then
        entry.curr_anim = anim
        _enter_frame(entry, 1)
    end
end

function ns.animation.start(ent, anim)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    assert(entry.anims[anim], "must have an animation with name '" .. anim .. "'")
    entry.curr_anim = anim
    _enter_frame(entry, 1)
end

function ns.animation.update_all()
    ng.entity_table_remove_destroyed(ns.animation.tbl, ns.animation.remove)

    if ns.timing.get_paused() then
        return
    end

    for ent, entry in pairs(ns.animation.tbl) do
        if entry.curr_anim then
            local dt = ns.timing.instance.dt
            local anim = entry.anims[entry.curr_anim]

            -- next frame?
            while entry.t <= dt do
                if anim.after and entry.frame >= anim.n then
                    assert(entry.anims[anim.after], "must have an animation " .. "with name '" .. anim.after .. "'")
                    entry.curr_anim = anim.after
                    entry.frame = 1
                    anim = entry.anims[anim.after]
                end
                _enter_frame(entry, entry.frame >= anim.n and 1 or entry.frame + 1, anim)
                dt = dt - entry.t
            end

            entry.t = entry.t - dt
        end
    end
end

ns.edit_inspector.custom['animation'] = {
    add = function(inspector)
        -- current animation
        inspector.curr_anim = ng.edit_field_create {
            field_type = 'enum',
            parent = inspector.window_body,
            label = 'curr_anim'
        }

        -- animation list
        inspector.anim_views_container = ng.add {
            transform = {
                parent = inspector.window_body
            },
            gui = {
                padding = ng.vec2_zero,
                color = ng.color_clear,
                valign = ng.GA_TABLE,
                halign = ng.GA_MIN
            },
            gui_rect = {
                hfill = true
            }
        }
        inspector.anim_views = {}

        -- 'add strip' button
        inspector.add_strip = ng.add {
            transform = {
                parent = inspector.window_body
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_TABLE,
                halign = ng.GA_MID
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_strip), 'add strip')
    end,

    post_update = function(inspector)
        local ent = inspector.ent
        local entry = ns.animation.tbl[ent]
        local anims = entry.anims

        -- current animation
        ng.edit_field_post_update(inspector.curr_anim, entry.curr_anim or '(none)', function(v)
            ns.animation.switch(ent, v)
        end, anims)

        -- add strip?
        if ns.gui.event_mouse_down(inspector.add_strip) == ng.MC_LEFT then
            local function new_strip(s)
                local strips = {
                    [s] = {
                        n = 1,
                        t = 1,
                        base = ng.vec2(0, 0)
                    }
                }
                ns.animation.set_strips(ent, strips)
            end
            ns.edit.command_start('new strip name: ', new_strip)
        end

        -- add missing views
        for name, anim in pairs(anims) do
            if not inspector.anim_views[name] then
                local view = {}
                inspector.anim_views[name] = view

                -- view window
                view.window = ng.add {
                    transform = {
                        parent = inspector.anim_views_container
                    },
                    gui_window = {
                        title = name,
                        minimized = true
                    },
                    gui_rect = {
                        hfill = true
                    },
                    gui = {
                        valign = ng.GA_TABLE,
                        halign = ng.GA_MIN
                    }
                }
                view.window_body = ns.gui_window.get_body(view.window)

                -- 'duplicate' button
                view.dup_text = ng.add {
                    transform = {
                        parent = ns.gui_window.get_title_buttons_area(view.window)
                    },
                    gui = {
                        color = ng.color_white,
                        valign = ng.GA_MAX,
                        halign = ng.GA_TABLE
                    },
                    gui_text = {
                        str = 'd'
                    }
                }

                -- fields
                view.n = ng.edit_field_create {
                    field_type = 'Scalar',
                    parent = view.window_body,
                    label = 'n'
                }
                view.t = ng.edit_field_create {
                    field_type = 'Scalar',
                    parent = view.window_body,
                    label = 't'
                }
                view.base = ng.edit_field_create {
                    field_type = 'Vec2',
                    parent = view.window_body,
                    label = 'base'
                }
                view.after = ng.edit_field_create {
                    field_type = 'enum',
                    parent = view.window_body,
                    label = 'after'
                }
            end
        end

        -- remove extra views
        for name, view in pairs(inspector.anim_views) do
            if not anims[name] then
                ns.gui_window.remove(view.window)
                inspector.anim_views[name] = nil
            end
        end

        -- update views
        for name, view in pairs(inspector.anim_views) do
            if ns.entity.destroyed(view.window) then
                ns.animation.remove(ent, name)
            else
                local anim = anims[name]

                -- duplicate?
                if ns.gui.event_mouse_down(view.dup_text) == ng.MC_LEFT then
                    local function new_strip(s)
                        local strips = {
                            [s] = {
                                n = anim.n,
                                t = anim.strip.t,
                                base = ng.Vec2(anim.strip.base),
                                after = anim.after
                            }
                        }
                        ns.animation.set_strips(ent, strips)
                    end
                    ns.edit.command_start('duplicate strip name: ', new_strip)
                end

                -- update fields
                ng.edit_field_post_update(view.n, anim.n, function(v)
                    anim.n = v
                end)
                ng.edit_field_post_update(view.t, anim.strip.t, function(v)
                    anim.strip.t = v
                    entry.t = v > 0 and v or 1
                end)
                ng.edit_field_post_update(view.base, anim.strip.base, function(v)
                    anim.strip.base = v
                end)
                ng.edit_field_post_update(view.after, anim.after or '(none)', function(s)
                    anim.after = s
                end, anims)
            end
        end
    end
}

-- hot_require 'nekogame.bump'
local bump = hot_require 'libs/bump'

local world = bump.newWorld()

ns.bump = ng.simple_sys()

ng.simple_prop(ns.bump, 'bbox', ng.bbox(ng.vec2(-0.5, -0.5), ng.vec2(0.5, 0.5)))

local function _update_rect(obj, add)
    if world:hasItem(obj.ent.id) and obj.last_dirty == ns.transform.get_dirty_count(obj.ent) then
        return
    end

    local lt = obj.bbox.min + ns.transform.get_position(obj.ent)
    local wh = obj.bbox.max - obj.bbox.min
    if world:hasItem(obj.ent.id) then
        world:move(obj.ent.id, lt.x, lt.y, wh.x, wh.y)
    else
        world:add(obj.ent.id, lt.x, lt.y, wh.x, wh.y)
    end

    obj.last_dirty = ns.transform.get_dirty_count(obj.ent)
end

function ns.bump.create(obj)
    ns.transform.add(obj.ent)
    _update_rect(obj, true)
end
function ns.bump.destroy(obj)
    if world:hasItem(obj.ent.id) then
        world:remove(obj.ent.id)
    end
end

function ns.bump.set_position(ent, pos)
    local obj = ns.bump.tbl[ent]
    assert(obj, 'entity must be in bump system')
    ns.transform.set_position(ent, pos)
    _update_rect(obj)
end

local function _filter_wrap(filter)
    return filter and function(id)
        return filter(ng.NativeEntity {
            id = id
        })
    end
end

function ns.bump.sweep(ent, p, filter)
    local obj = ns.bump.tbl[ent]
    assert(obj, 'entity must be in bump system')
    _update_rect(obj)

    p = obj.bbox.min + ns.transform.get_position(ent) + (p or ng.vec2_zero)

    local wfilter = _filter_wrap(filter)
    local cols, len = world:check(obj.ent.id, p.x, p.y, wfilter)
    local min = obj.bbox.min
    local ecols = {}
    for i = 1, len do
        local col = cols[i]
        local tl, tt, nx, ny, sl, st = col:getSlide()
        table.insert(ecols, {
            other = ng.NativeEntity {
                id = col.other
            },
            touch = ng.vec2(tl, tt) - min,
            normal = ng.vec2(nx, ny),
            slide = ng.vec2(sl, st) - min
        })
    end
    return ecols
end

function ns.bump.slide(ent, p, filter)
    local obj = ns.bump.tbl[ent]
    assert(obj, 'entity must be in bump system')
    _update_rect(obj)

    p = ns.transform.get_position(ent) + p

    local min = obj.bbox.min
    local wfilter = _filter_wrap(filter)

    local function rslide(ax, ay, bx, by, depth)
        if depth > 3 then
            return ax, ay, {}
        end

        world:move(obj.ent.id, ax, ay)
        local cols, len = world:check(obj.ent.id, bx, by, wfilter)
        if len == 0 then
            return bx, by, {}
        end

        -- find best next collision recursively
        local m = -1
        local mcols, mcol, mtx, mty, mnx, mny, msx, msy
        for i = 1, len do
            world:move(obj.ent.id, ax, ay) -- TODO: avoid re-moving
            local tx, ty, nx, ny, sx, sy = cols[i]:getSlide()
            local px, py, pcols = rslide(tx, ty, sx, sy, depth + 1)
            local dx, dy = px - ax, py - ay
            local d = dx * dx + dy * dy
            if d > m then
                m = d
                bx, by = px, py
                mcols, mcol = pcols, cols[i]
                mtx, mty, mnx, mny, msx, msy = tx, ty, nx, ny, sx, sy
            end
        end

        -- add next collision and return
        table.insert(mcols, {
            other = ng.NativeEntity {
                id = mcol.other
            },
            touch = ng.vec2(mtx, mty) - min,
            normal = ng.vec2(mnx, mny),
            slide = ng.vec2(msx, msy) - min
        })
        return bx, by, mcols
    end

    local a = ns.transform.get_position(obj.ent) + min
    local bx, by, cols = rslide(a.x, a.y, p.x + min.x, p.y + min.y, 0)
    ns.transform.set_position(ent, ng.vec2(bx - min.x, by - min.y))
    _update_rect(obj)
    return cols
end

function ns.bump.update(obj)
    _update_rect(obj)
    ns.edit.bboxes_update(obj.ent, obj.bbox)
end

-- hot_require 'nekogame.physics'
-- local ffi = require 'ffi'

-- ng.Collision = ffi.metatype('Collision', {})

-- local old_physics_shape_add_box = ng.physics_shape_add_box
-- function ng.physics_shape_add_box(ent, b, r)
--     r = r or 0
--     return old_physics_shape_add_box(ent, b, r)
-- end

-- local old_physics_shape_add_poly = ng.physics_shape_add_poly
-- function ng.physics_shape_add_poly(ent, verts, r)
--     r = r or 0
--     local n = #verts
--     return old_physics_shape_add_poly(ent, n, ffi.new('Vec2[?]', n, verts), r)
-- end

-- local old_physics_convex_hull = ng.physics_convex_hull
-- function ng.physics_convex_hull(verts)
--     local n = #verts
--     local c_arr = ffi.new('Vec2[?]', n, verts)
--     n = old_physics_convex_hull(n, c_arr)

--     local lua_arr = {}
--     for i = 0, n - 1 do
--         table.insert(lua_arr, ng.Vec2(c_arr[i]))
--     end
--     return lua_arr
-- end

-- local old_physics_get_collisions = ng.physics_get_collisions
-- function ng.physics_get_collisions(ent)
--     local n = ng.physics_get_num_collisions(ent)
--     local c_arr = old_physics_get_collisions(ent)
--     local lua_arr = {}
--     for i = 0, n - 1 do
--         table.insert(lua_arr, ng.Collision(c_arr[i]))
--     end
--     return lua_arr
-- end

-- hot_require 'nekogame.sound'
-- ng.wrap_string('sound', 'path')

-- hot_require 'nekogame.console'
-- hot_require 'nekogame.fps'
-- hot_require 'nekogame.inspector'

print("nekogame.lua loaded")
