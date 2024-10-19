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
        [0] = { "int", "", "size", false, { 0x08000000, "bool" }, { 0x04000000, "float", "subwhat" }, { 0x02000000, "const" },
            { 0x01000000, "volatile" }, { 0x00800000, "unsigned" }, { 0x00400000, "long" } },
        { "struct", "", "size", true, { 0x02000000, "const" }, { 0x01000000, "volatile" }, { 0x00800000, "union", "subwhat" },
            { 0x00100000, "vla" } },
        { "ptr", "element_type", "size", false, { 0x02000000, "const" }, { 0x01000000, "volatile" },
            { 0x00800000, "ref",  "subwhat" } },
        { "array", "element_type", "size", false, { 0x08000000, "vector" }, { 0x04000000, "complex" }, { 0x02000000, "const" },
            { 0x01000000, "volatile" }, { 0x00100000, "vla" } },
        { "void", "",            "size",  false, { 0x02000000, "const" },  { 0x01000000, "volatile" } },
        { "enum", "type",        "size",  true },
        { "func", "return_type", "nargs", true,  { 0x00800000, "vararg" }, { 0x00400000, "sse_reg_params" } },
        { "typedef", -- Not seen
            "element_type", "", false },
        { "attrib",  -- Only seen internally
            "type", "value", true },
        { "field",    "type", "offset", true },
        { "bitfield", "", "offset", true, { 0x08000000, "bool" }, { 0x02000000, "const" }, { 0x01000000, "volatile" },
            { 0x00800000, "unsigned" } },
        { "constant", "type", "value",  true, { 0x02000000, "const" } },
        { "extern", -- Not seen
            "CID", "", true },
        { "kw",     -- Not seen
            "TOK", "size" }
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

    local t_concat = default_require "table".concat
    local new = default_require "ffi".new
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

ng.is_nil_entity = function(ent)
    return ent.id == 0
end

local entity_table_mt__regname = ng.NativeEntity(0x114514)
local defaults_table_mt__regname = ng.NativeEntity(0x114514 + 1)

--
-- NativeEntity -> data map
--
-- can't use normal Lua tables because NativeEntity is cdata, which isn't
-- hashed right
--
local function bind_defaults(t, v)
    if type(v) == 'table' then
        local defaults = rawget(t, defaults_table_mt__regname)
        if defaults then
            setmetatable(v, {
                __index = defaults
            })
        end
    end
end

local entity_table_mt = {
    __newindex = function(t, k, v)
        local map = rawget(t, entity_table_mt__regname)

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
            rawset(t, entity_table_mt__regname, map)
        end
        bind_defaults(t, v)
        map[k.id] = {
            ['k'] = ng.NativeEntity(k),
            ['v'] = v
        }
    end,

    __index = function(t, k)
        local map = rawget(t, entity_table_mt__regname)

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
        local map = rawget(t, entity_table_mt__regname) or {}

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
        local map = rawget(t, entity_table_mt__regname)

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
    for _, slot in pairs(rawget(d, entity_table_mt__regname) or {}) do
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
        local defaults = rawget(tbl, defaults_table_mt__regname)
        if not defaults then
            defaults = {}
            rawset(tbl, defaults_table_mt__regname, defaults)
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

ng.cent = function(ent)
    local ent_c = ng.api.core.NativeEntity.new()
    ent_c.id = ent.id
    return ent_c
end

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
    -- 在触发事件之前存储系统名称 因为系统列表可能会更改

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

function ng.__save_all()
    local data = {}

    for name, system in pairs(ns) do
        if system.auto_saveload then
            data[name] = system
        elseif system.save_all then
            -- has special load_all() event
            data[name] = system.save_all()
        end
        print("saving system: " .. name)
    end
    -- return common.pack(data)
    return table.show(data)
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
    return ng.getter(sys, prop)(unpack({ ... }))
end

function ng.set(sys, prop, ...)
    ng.setter(sys, prop)(unpack({ ... }))
end

function ng.adder(sys)
    return ns[sys]['add'] -- 这里转发到 ffi.C
end

function ng.remover(sys)
    return ns[sys]['remove'] -- 这里转发到 ffi.C
end

function ng.remove(sys, ...)
    ng.remover(sys)(unpack({ ... }))
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
local name_entity = {}                -- name -> entity map

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

local group_entities = {}               -- group name --> entity_table
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

print("nekogame.lua loaded")
