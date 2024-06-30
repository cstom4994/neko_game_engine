local core = require("neko.struct")

local M = {}

local function parse_struct(code)
    local nest = {}
    local nest_n = 0
    code = code:gsub("(%b{})", function(v)
        nest_n = nest_n + 1
        nest[nest_n] = v
        return "{" .. nest_n .. "} "
    end)
    local names = {}
    local lines = {}
    local line_n = 0
    for line in code:gmatch "%s*(.-)%s*;" do
        line_n = line_n + 1
        line = line:gsub("%s+", " ")
        line = line:gsub(" ?%*%s*", " *")
        local prefix, array = line:match "^(.-)%s*(%b[])$"
        if array then
            array = math.tointeger(array:match "%[(%d+)%]") or 0
            line = prefix
        end
        local typestr, pointer, name = line:match "^(.-) (%**)([_%w]+)$"
        assert(typestr, line)
        local type_prefix, subtype = typestr:match "^([%w_]+)%s+(.+)"
        if type_prefix == "struct" or type_prefix == "union" then
            typestr = type_prefix
            local nesttypeid = subtype:match "^{(%d+)}$"
            if nesttypeid then
                local nestcontent = assert(nest[tonumber(nesttypeid)]):match "^{(.*)}$"
                subtype = parse_struct(nestcontent)
                subtype.type = type_prefix
            end
        end
        if pointer == "" then
            pointer = nil
        end
        local t = {
            array = array,
            type = typestr,
            subtype = subtype,
            pointer = pointer,
            name = name
        }
        assert(names[name] == nil, name)
        names[name] = true
        lines[line_n] = t
    end

    return lines
end

local function parse(what, code, types)
    for typename, content in code:gmatch(what .. "%s+([_%w]+)%s*(%b{})%s*;") do
        assert(types[typename] == nil)
        local s = parse_struct(content:match "^{%s*(.-)%s*}$")
        s.type = what
        s.name = what .. " " .. typename
        types[s.name] = s
    end
end

local buildin_types = (function(map)
    local r = {}
    for k, v in pairs(map) do
        if type(k) == "number" then
            r[v] = true
        else
            r[k] = v
        end
    end
    return r
end) {
    int = "int32_t",
    short = "int16_t",
    char = "int8_t",
    ["unsigned char"] = "uint8_t",
    ["unsigned short"] = "uint16_t",
    ["unsigned int"] = "uint32_t",
    "float",
    "double",
    "void",
    "bool",
    -- ["b"]= "bool",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    ["s8"] = "int8_t",
    ["s16"] = "int16_t",
    ["s32"] = "int32_t",
    ["s64"] = "int64_t",
    ["u8"] = "uint8_t",
    ["u16"] = "uint16_t",
    ["u32"] = "uint32_t",
    ["u64"] = "uint64_t"
}

local buildin_size = {
    int8_t = 1,
    int16_t = 2,
    int32_t = 4,
    int64_t = 8,
    uint8_t = 1,
    uint16_t = 2,
    uint32_t = 4,
    uint64_t = 8,
    float = 4,
    double = 8,
    ptr = 8,
    bool = 1
}

local buildin_id = core.typeid()

for k, v in pairs(buildin_types) do
    if v ~= true then
        buildin_size[k] = buildin_size[v]
        buildin_id[k] = buildin_id[v]
    end
end

local function check_types(types)
    for k, t in pairs(types) do
        for idx, f in ipairs(t) do
            local typename = f.type
            if typename == "struct" or typename == "union" then
                if type(f.subtype) == "string" then
                    local fullname = typename .. " " .. f.subtype
                    local subtype = types[fullname]
                    if not subtype then
                        error("Unknown " .. fullname)
                    end
                    assert(subtype.type == typename)
                    f.subtype = subtype
                end
            else
                if not buildin_types[typename] then
                    error("Unknown " .. typename)
                end
            end
            if f.array == 0 and t[idx + 1] then
                error("Array " .. f.name .. "[] must be the last field")
            end
        end
    end
end

local function calc_offset(types)
    local solve

    local function calc_align(t)
        local align = 0
        for _, f in ipairs(t) do
            if f.pointer then
                f.size = buildin_size.ptr
                f.align = f.size
            elseif f.subtype then
                local subtype = solve(f.subtype)
                f.size = subtype.size
                f.align = subtype.align
                if subtype.align > align then
                    align = subtype.align
                end
            else
                f.size = assert(buildin_size[f.type])
                f.align = f.size
                if f.align > align then
                    align = f.align
                end
            end
            if f.array then
                f.size = f.size * f.array
            end
        end
        return align
    end

    local function solve_struct(t)
        t.align = calc_align(t)
        local size = 0
        for _, f in ipairs(t) do
            if size % f.align ~= 0 then
                size = (size // f.align + 1) * f.align
            end
            f.offset = size
            size = size + f.size
        end
        if size % t.align ~= 0 then
            size = (size // t.align + 1) * t.align
        end
        t.size = size
    end
    local function solve_union(t)
        t.align = align(t)
        local size = 0
        for _, f in ipairs(t) do
            f.offset = 0
            if f.size > size then
                size = f.size
            end
        end
        t.size = size
    end
    do -- 解决局部函数
        local unsolved = {}
        local solved = {}
        function solve(t)
            local fullname = t.name
            if fullname then
                if solved[fullname] then
                    return solved[fullname]
                end
                assert(not unsolved[fullname])
                unsolved[fullname] = true
            end

            if t.type == "struct" then
                solve_struct(t)
            else
                solve_union(t)
            end

            if fullname then
                solved[fullname] = t
                unsolved[fullname] = nil
            end
            return t
        end
    end

    for k, t in pairs(types) do
        solve(t)
    end

    local function solve_pointer_size(t)
        for _, f in ipairs(t) do
            if f.pointer then
                assert(f.pointer == "*")
                if f.subtype then
                    f.pointer_size = f.subtype.size
                else
                    f.pointer_size = buildin_size[f.type]
                end
            end
        end
    end

    for k, t in pairs(types) do
        solve_pointer_size(t)
    end
end

local function keys_lookup(t)
    local keys = {}
    for _, f in ipairs(t) do
        keys[f.name] = f
    end
    t.keys = keys
    return keys
end

local function find_key(t, key)
    local keys = t.keys or keys_lookup(t)
    return assert(keys[key], key)
end

local function gen_check(types, k)
    local keys = {}
    local n = 0
    for name in k:gmatch "[^.]*" do
        n = n + 1
        keys[n] = name
    end
    local t = types[keys[1]]
    if t == nil then
        error(keys[1] .. " undefined")
    end
    local offset = {}
    local last_offset = 0
    local offset_n = 1
    local i = 2
    local typename
    while i <= n do
        local name = keys[i]
        local array_name, array_index = name:match "(.+)%[(%d+)]$"
        name = array_name or name

        local f = find_key(t, name)
        offset[offset_n] = last_offset + f.offset
        if f.pointer then
            assert(f.pointer == "*") -- todo: support "**"
            offset_n = offset_n + 1
            last_offset = 0
            typename = "ptr"
        elseif f.subtype then
            last_offset = last_offset + f.offset
            t = f.subtype
            assert(i ~= n)
        else
            assert(i == n)
            typename = f.type
        end

        if array_index then
            local index = tonumber(array_index)
            if f.pointer then
                offset[offset_n] = index * f.pointer_size
                offset_n = offset_n + 1
            else
                last_offset = last_offset + index * f.size
                offset[offset_n] = last_offset
            end
        end

        i = i + 1
    end
    local getter, setter = core.cstruct(buildin_id[typename], table.unpack(offset))
    return {getter, setter}
end

local function check(types)
    local function cache_check(self, k)
        local v = gen_check(types, k)
        self[k] = v
        return v
    end
    return setmetatable({}, {
        __index = cache_check
    })
end

local methods = {};
methods.__index = methods

function methods:dump()
    for _, s in pairs(self._types) do
        print(s.name, "size", s.size, "align", s.align)
        for _, f in ipairs(s) do
            local array = ""
            if f.array then
                array = "[" .. f.array .. "]"
            end
            local typename = f.type
            if f.subtype then
                typename = f.subtype.name or ("nest " .. f.subtype.type)
            end
            print(string.format("\t%3d : %s %s%s%s", f.offset, typename, (f.pointer or ""), f.name, array))
        end
    end
end

function methods:size(name)
    local t = assert(self._types[name])
    return t.size
end

function methods:getter(name)
    return self._check[name][1]
end

function methods:setter(name)
    return self._check[name][2]
end

function M.struct(code)
    local types = {}
    parse("struct", code, types)
    parse("union", code, types)
    check_types(types)
    calc_offset(types)

    local obj = {
        _types = types,
        _check = check(types)
    }

    return setmetatable(obj, methods)
end

M.s = {}

M.s["CGameObject"] = M.struct([[
    struct CGameObject {
        int id;
        bool active;
        bool visible;
        bool selected;
    };
    ]])

-- M.sz = M.s:size "struct CGameObject"

M.CGameObject_get_id = M.s["CGameObject"]:getter "struct CGameObject.id"
M.CGameObject_get_active = M.s["CGameObject"]:getter "struct CGameObject.active"
M.CGameObject_get_visible = M.s["CGameObject"]:getter "struct CGameObject.visible"
M.CGameObject_get_selected = M.s["CGameObject"]:getter "struct CGameObject.selected"

M.CGameObject_set_id = M.s["CGameObject"]:setter "struct CGameObject.id"
M.CGameObject_set_active = M.s["CGameObject"]:setter "struct CGameObject.active"
M.CGameObject_set_visible = M.s["CGameObject"]:setter "struct CGameObject.visible"
M.CGameObject_set_selected = M.s["CGameObject"]:setter "struct CGameObject.selected"

-- M.test_obj = luastruct_test.udata(M.sz)

M.new_obj = function(id, active, visible)
    local obj = luastruct_test.udata(M.s["CGameObject"]:size "struct CGameObject")
    M.CGameObject_set_id(obj, id)
    M.CGameObject_set_active(obj, active)
    M.CGameObject_set_visible(obj, visible)
    M.CGameObject_set_selected(obj, false)
    return obj
end

-- M.CGameObject_set_visible(M.test_obj, false)
-- print(M.CGameObject_get_visible(M.test_obj))
-- M.s:dump()

M.s["neko_sprite_t"] = M.struct([[
    struct neko_sprite_t {
        uint64_t image_id;
        int depth;
        float x;
        float y;
        float sx;
        float sy;
        float c;
        float s;
    };
    ]])

M.s["neko_vec4_t"] = M.struct([[
    struct neko_vec4_t {
        float x;
        float y;
        float z;
        float w;
    };
    ]])

M.s["__lua_quad_idata_t"] = M.struct([[
    struct __lua_quad_idata_t {
        uint32_t v1;
        uint32_t v2;
        uint32_t v3;
        uint32_t v4;
        uint32_t v5;
        uint32_t v6;
    };
    ]])

M.s["__lua_tex_t"] = M.struct([[
    struct __lua_tex_t {
        uint32_t id;
    };
    ]])

M.generate_array_type = function(name, base_type, n)
    local code = "struct " .. name .. "{\n"
    for i = 1, n do
        code = code .. base_type .. " v" .. i .. ";\n"
    end
    code = code .. "};"
    M.s[name] = M.struct(code)
    return M.s[name]
end

M.getter = function(struct_name, field)
    return M.s[struct_name]:getter("struct " .. struct_name .. "." .. field)
end

M.setter = function(struct_name, field)
    return M.s[struct_name]:setter("struct " .. struct_name .. "." .. field)
end

return M
