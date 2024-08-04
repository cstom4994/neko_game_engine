if string.find(_VERSION, "5%.0") then
    return
end

-- "loadfile"
local function pp_dofile(path)

    local loaded = false
    local getfile = function()

        if loaded then
            return
        else
            local file, err = io.open(path)
            if not file then
                error("error loading file " .. path .. ": " .. err)
            end
            local ret = file:read("*a")
            file:close()

            ret = string.gsub(ret, "%.%.%.%s*%)", "...) local arg = {n=select('#', ...), ...};")

            loaded = true
            return ret
        end
    end

    local f = load(getfile, path)
    if not f then

        error("error loading file " .. path)
    end
    return f()
end

old_dofile = dofile
dofile = pp_dofile

-- string.gsub
--[[
local ogsub = string.gsub
local function compgsub(a,b,c,d)
  if type(c) == "function" then
    local oc = c
    c = function (...) return oc(...) or '' end
  end
  return ogsub(a,b,c,d)
end
string.repl = ogsub
--]]

-- string.gsub = compgsub

-------------------------------------------------------------------
-- Real globals
-- _ALERT
-- _ERRORMESSAGE
-- _VERSION
-- _G
-- assert
-- error
-- metatable
-- next
-- print
-- require
-- tonumber
-- tostring
-- type
-- unpack

-------------------------------------------------------------------
-- collectgarbage
-- gcinfo

-- globals

-- call   -> protect(f, err)
-- loadfile
-- loadstring

-- rawget
-- rawset

-- getargs = Main.getargs ??

rawtype = type

function do_(f, err)
    if not f then
        print(err);
        return
    end
    local a, b = pcall(f)
    if not a then
        print(b);
        return nil
    else
        return b or true
    end
end

function dostring(s)
    return do_(load(s))
end
-- function dofile(s) return do_(loadfile(s)) end

-------------------------------------------------------------------
-- Table library
local tab = table
foreach = function(t, f)
    for k, v in pairs(t) do
        f(k, v)
    end
end
foreachi = function(t, f)
    for i, v in ipairs(t) do
        f(i, v)
    end
end
getn = function(t)
    return #t
end
tinsert = tab.insert
tremove = tab.remove
sort = tab.sort

-------------------------------------------------------------------
-- Debug library
local dbg = debug
getinfo = dbg.getinfo
getlocal = dbg.getlocal
setcallhook = function()
    error "`setcallhook' is deprecated"
end
setlinehook = function()
    error "`setlinehook' is deprecated"
end
setlocal = dbg.setlocal

-------------------------------------------------------------------
-- math library
local math = math
abs = math.abs
acos = function(x)
    return math.deg(math.acos(x))
end
asin = function(x)
    return math.deg(math.asin(x))
end
atan = function(x)
    return math.deg(math.atan(x))
end
atan2 = function(x, y)
    return math.deg(math.atan2(x, y))
end
ceil = math.ceil
cos = function(x)
    return math.cos(math.rad(x))
end
deg = math.deg
exp = math.exp
floor = math.floor
frexp = math.frexp
ldexp = math.ldexp
log = math.log
log10 = math.log10
max = math.max
min = math.min
mod = math.mod
PI = math.pi
-- ??? pow = math.pow  
rad = math.rad
random = math.random
randomseed = math.randomseed
sin = function(x)
    return math.sin(math.rad(x))
end
sqrt = math.sqrt
tan = function(x)
    return math.tan(math.rad(x))
end

-------------------------------------------------------------------
-- string library
local str = string
strbyte = str.byte
strchar = str.char
strfind = str.find
format = str.format
gsub = str.gsub
strlen = str.len
strlower = str.lower
strrep = str.rep
strsub = str.sub
strupper = str.upper

-------------------------------------------------------------------
-- os library
clock = os.clock
date = os.date
difftime = os.difftime
execute = os.execute -- ?
exit = os.exit
getenv = os.getenv
remove = os.remove
rename = os.rename
setlocale = os.setlocale
time = os.time
tmpname = os.tmpname

-------------------------------------------------------------------
-- compatibility only
getglobal = function(n)
    return _G[n]
end
setglobal = function(n, v)
    _G[n] = v
end

-------------------------------------------------------------------

local io, tab = io, table

-- IO library (files)
_STDIN = io.stdin
_STDERR = io.stderr
_STDOUT = io.stdout
_INPUT = io.stdin
_OUTPUT = io.stdout
seek = io.stdin.seek -- sick ;-)
tmpfile = io.tmpfile
closefile = io.close
openfile = io.open

function flush(f)
    if f then
        f:flush()
    else
        _OUTPUT:flush()
    end
end

function readfrom(name)
    if name == nil then
        local f, err, cod = io.close(_INPUT)
        _INPUT = io.stdin
        return f, err, cod
    else
        local f, err, cod = io.open(name, "r")
        _INPUT = f or _INPUT
        return f, err, cod
    end
end

function writeto(name)
    if name == nil then
        local f, err, cod = io.close(_OUTPUT)
        _OUTPUT = io.stdout
        return f, err, cod
    else
        local f, err, cod = io.open(name, "w")
        _OUTPUT = f or _OUTPUT
        return f, err, cod
    end
end

function appendto(name)
    local f, err, cod = io.open(name, "a")
    _OUTPUT = f or _OUTPUT
    return f, err, cod
end

function read(...)
    local f = _INPUT
    local arg = {...}
    if rawtype(arg[1]) == 'userdata' then
        f = tab.remove(arg, 1)
    end
    return f:read(table.unpack(arg))
end

function write(...)
    local f = _OUTPUT
    local arg = {...}
    if rawtype(arg[1]) == 'userdata' then
        f = tab.remove(arg, 1)
    end
    return f:write(table.unpack(arg))
end

-- Basic C types and their corresponding Lua types
-- All occurrences of "char*" will be replaced by "_cstring",
-- and all occurrences of "void*" will be replaced by "_userdata"
_basic = {
    ['void'] = '',
    ['char'] = 'number',
    ['int'] = 'number',
    ['short'] = 'number',
    ['long'] = 'number',
    ['unsigned'] = 'number',
    ['float'] = 'number',
    ['double'] = 'number',
    ['_cstring'] = 'string',
    ['_userdata'] = 'userdata',
    ['char*'] = 'string',
    ['void*'] = 'userdata',
    ['bool'] = 'boolean',
    ['lua_Object'] = 'value',
    ['LUA_VALUE'] = 'value', -- for compatibility with tolua 4.0
    ['lua_State*'] = 'state',
    ['_lstate'] = 'state',
    ['lua_Function'] = 'value'
}

_basic_ctype = {
    number = "lua_Number",
    string = "const char*",
    userdata = "void*",
    boolean = "bool",
    value = "int",
    state = "lua_State*"
}

-- functions the are used to do a 'raw push' of basic types
_basic_raw_push = {}

-- List of user defined types
-- Each type corresponds to a variable name that stores its tag value.
_usertype = {}

-- List of types that have to be collected
_collect = {}

-- List of types
_global_types = {
    n = 0
}
_global_types_hash = {}

-- list of classes
_global_classes = {}

-- List of enum constants
_global_enums = {}

-- List of auto renaming
_renaming = {}
function appendrenaming(s)
    local b, e, old, new = strfind(s, "%s*(.-)%s*@%s*(.-)%s*$")
    if not b then
        error("#Invalid renaming syntax; it should be of the form: pattern@pattern")
    end
    tinsert(_renaming, {
        old = old,
        new = new
    })
end

function applyrenaming(s)
    for i = 1, getn(_renaming) do
        local m, n = gsub(s, _renaming[i].old, _renaming[i].new)
        if n ~= 0 then
            return m
        end
    end
    return nil
end

-- Error handler
function neko_tolua_error(s, f)
    if _curr_code then
        print("***curr code for error is " .. tostring(_curr_code))
        print(debug.traceback())
    end
    local out = _OUTPUT
    _OUTPUT = _STDERR
    if strsub(s, 1, 1) == '#' then
        write("\n** tolua: " .. strsub(s, 2) .. ".\n\n")
        if _curr_code then
            local _, _, s = strfind(_curr_code, "^%s*(.-\n)") -- extract first line
            if s == nil then
                s = _curr_code
            end
            s = gsub(s, "_userdata", "void*") -- return with 'void*'
            s = gsub(s, "_cstring", "char*") -- return with 'char*'
            s = gsub(s, "_lstate", "lua_State*") -- return with 'lua_State*'
            write("Code being processed:\n" .. s .. "\n")
        end
    else
        if not f then
            f = "(f is nil)"
        end
        print("\n** neko tolua internal error: " .. f .. s .. ".\n\n")
        return
    end
    _OUTPUT = out
end

function warning(msg)
    local out = _OUTPUT
    _OUTPUT = _STDERR
    write("\n** neko tolua warning: " .. msg .. ".\n\n")
    _OUTPUT = out
end

-- register an user defined type: returns full type
function regtype(t)
    -- if isbasic(t) then
    --	return t
    -- end
    local ft = findtype(t)

    if not _usertype[ft] then
        return appendusertype(t)
    end
    return ft
end

-- return type name: returns full type
function typevar(type)
    if type == '' or type == 'void' then
        return type
    else
        local ft = findtype(type)
        if ft then
            return ft
        end
        _usertype[type] = type
        return type
    end
end

-- check if basic type
function isbasic(type)
    local t = gsub(type, 'const ', '')
    local m, t = applytypedef('', t)
    local b = _basic[t]
    if b then
        return b, _basic_ctype[b]
    end
    return nil
end

-- split string using a token
function split(s, t)
    local l = {
        n = 0
    }
    local f = function(s)
        l.n = l.n + 1
        l[l.n] = s
        return ""
    end
    local p = "%s*(.-)%s*" .. t .. "%s*"
    s = gsub(s, "^%s+", "")
    s = gsub(s, "%s+$", "")
    s = gsub(s, p, f)
    l.n = l.n + 1
    l[l.n] = gsub(s, "(%s%s*)$", "")
    return l
end

-- splits a string using a pattern, considering the spacial cases of C code (templates, function parameters, etc)
-- pattern can't contain the '^' (as used to identify the begining of the line)
-- also strips whitespace
function split_c_tokens(s, pat)

    s = string.gsub(s, "^%s*", "")
    s = string.gsub(s, "%s*$", "")

    local token_begin = 1
    local token_end = 1
    local ofs = 1
    local ret = {
        n = 0
    }

    function add_token(ofs)

        local t = string.sub(s, token_begin, ofs)
        t = string.gsub(t, "^%s*", "")
        t = string.gsub(t, "%s*$", "")
        ret.n = ret.n + 1
        ret[ret.n] = t
    end

    while ofs <= string.len(s) do

        local sub = string.sub(s, ofs, -1)
        local b, e = string.find(sub, "^" .. pat)
        if b then
            add_token(ofs - 1)
            ofs = ofs + e
            token_begin = ofs
        else
            local char = string.sub(s, ofs, ofs)
            if char == "(" or char == "<" then

                local block
                if char == "(" then
                    block = "^%b()"
                end
                if char == "<" then
                    block = "^%b<>"
                end

                b, e = string.find(sub, block)
                if not b then
                    -- unterminated block?
                    ofs = ofs + 1
                else
                    ofs = ofs + e
                end

            else
                ofs = ofs + 1
            end
        end

    end
    add_token(ofs)
    -- if ret.n == 0 then

    --	ret.n=1
    --	ret[1] = ""
    -- end

    return ret

end

-- concatenate strings of a table
function concat(t, f, l, jstr)
    jstr = jstr or " "
    local s = ''
    local i = f
    while i <= l do
        s = s .. t[i]
        i = i + 1
        if i <= l then
            s = s .. jstr
        end
    end
    return s
end

-- concatenate all parameters, following output rules
function concatparam(line, ...)
    local arg = {...}
    local i = 1
    while i <= #arg do
        if _cont and not strfind(_cont, '[%(,"]') and strfind(arg[i], "^[%a_~]") then
            line = line .. ' '
        end
        line = line .. arg[i]
        if arg[i] ~= '' then
            _cont = strsub(arg[i], -1, -1)
        end
        i = i + 1
    end
    if strfind(arg[#arg], "[%/%)%;%{%}]$") then
        _cont = nil
        line = line .. '\n'
    end
    return line
end

-- output line
function output(...)
    local arg = {...}
    local i = 1
    while i <= #arg do
        if _cont and not strfind(_cont, '[%(,"]') and strfind(arg[i], "^[%a_~]") then
            write(' ')
        end
        write(arg[i])
        if arg[i] ~= '' then
            _cont = strsub(arg[i], -1, -1)
        end
        i = i + 1
    end
    if strfind(arg[#arg], "[%/%)%;%{%}]$") then
        _cont = nil
        write('\n')
    end
end

function get_property_methods(ptype, name)

    if get_property_methods_hook and get_property_methods_hook(ptype, name) then
        return get_property_methods_hook(ptype, name)
    end

    if ptype == "default" then -- get_name, set_name
        return "get_" .. name, "set_" .. name
    end

    if ptype == "qt" then -- name, setName
        return name, "set" .. string.upper(string.sub(name, 1, 1)) .. string.sub(name, 2, -1)
    end

    if ptype == "overload" then -- name, name
        return name, name
    end

    return nil
end

-------------- the hooks

-- called right after processing the $[ichl]file directives,
-- right before processing anything else
-- takes the package object as the parameter
function preprocess_hook(p)
    -- p.code has all the input code from the pkg
end

-- called for every $ifile directive
-- takes a table with a string called 'code' inside, the filename, and any extra arguments
-- passed to $ifile. no return value
function include_file_hook(t, filename, ...)

end

-- called after processing anything that's not code (like '$renaming', comments, etc)
-- and right before parsing the actual code.
-- takes the Package object with all the code on the 'code' key. no return value
function preparse_hook(package)

end

-- called before starting output
function pre_output_hook(package)

end

-- called after writing all the output.
-- takes the Package object
function post_output_hook(package)

end

-- called from 'get_property_methods' to get the methods to retrieve a property
-- according to its type
function get_property_methods_hook(property_type, name)

end

-- called from ClassContainer:doparse with the string being parsed
-- return nil, or a substring
function parser_hook(s)

    return nil
end

-- called from classFunction:supcode, before the call to the function is output
function pre_call_hook(f)

end

-- called from classFunction:supcode, after the call to the function is output
function post_call_hook(f)

end

-- called before the register code is output
function pre_register_hook(package)

end

-- called to output an error message
function output_error_hook(...)
    return string.format(...)
end

-- custom pushers

_push_functions = {}
_is_functions = {}
_to_functions = {}

_base_push_functions = {}
_base_is_functions = {}
_base_to_functions = {}

local function search_base(t, funcs)

    local class = _global_classes[t]

    while class do
        if funcs[class.type] then
            return funcs[class.type]
        end
        class = _global_classes[class.btype]
    end
    return nil
end

function get_push_function(t)
    return _push_functions[t] or search_base(t, _base_push_functions) or "neko_tolua_pushusertype"
end

function get_to_function(t)
    return _to_functions[t] or search_base(t, _base_to_functions) or "neko_tolua_tousertype"
end

function get_is_function(t)
    return _is_functions[t] or search_base(t, _base_is_functions) or "neko_tolua_isusertype"
end

-- Feature class
-- Represents the base class of all mapped feature.
classFeature = {}
classFeature.__index = classFeature

-- write support code
function classFeature:supcode()
end

-- output tag
function classFeature:decltype()
end

-- register feature
function classFeature:register(pre)
end

-- translate verbatim
function classFeature:preamble()
end

-- check if it is a variable
function classFeature:isvariable()
    return false
end

-- check if it requires collection
function classFeature:requirecollection(t)
    return false
end

-- build names
function classFeature:buildnames()
    if self.name and self.name ~= '' then
        local n = split(self.name, '@')
        self.name = n[1]
        self.name = string.gsub(self.name, ":%d*$", "")
        if not n[2] then
            n[2] = applyrenaming(n[1])
        end
        self.lname = n[2] or gsub(n[1], "%[.-%]", "")
        self.lname = string.gsub(self.lname, ":%d*$", "")
        self.original_name = self.name
        self.lname = clean_template(self.lname)
    end
    if not self.is_parameter then
        self.name = getonlynamespace() .. self.name
    end

    local parent = classContainer.curr
    if parent then
        self.access = parent.curr_member_access
        self.global_access = self:check_public_access()
    else
    end
end

function classFeature:check_public_access()

    if type(self.global_access) == "boolean" then
        return self.global_access
    end

    if self.access and self.access ~= 0 then
        return false
    end

    local parent = classContainer.curr
    while parent do
        if parent.access and parent.access ~= 0 then
            return false
        end
        parent = parent.prox
    end
    return true
end

function clean_template(t)

    return string.gsub(t, "[<>:, %*]", "_")
end

-- check if feature is inside a container definition
-- it returns the container class name or nil.
function classFeature:incontainer(which)
    if self.parent then
        local parent = self.parent
        while parent do
            if parent.classtype == which then
                return parent.name
            end
            parent = parent.parent
        end
    end
    return nil
end

function classFeature:inclass()
    return self:incontainer('class')
end

function classFeature:inmodule()
    return self:incontainer('module')
end

function classFeature:innamespace()
    return self:incontainer('namespace')
end

-- return C binding function name based on name
-- the client specifies a prefix
function classFeature:cfuncname(n)

    if self.parent then
        n = self.parent:cfuncname(n)
    end

    local fname = self.lname
    if not fname or fname == '' then
        fname = self.name
    end
    n = string.gsub(n .. '_' .. (fname), "[<>:, %.%*&]", "_")

    return n
end

-- global
code_n = 1

-- Code class
-- Represents Lua code to be compiled and included
-- in the initialization function.
-- The following fields are stored:
--   text = text code
classCode = {
    text = ''
}
classCode.__index = classCode
setmetatable(classCode, classFeature)

-- register code
function classCode:register(pre)
    pre = pre or ''
    local s = self.text
    if not s then
        -- print(self.text)
        error("parser error in embedded code")
    end

    -- get first line
    local _, _, first_line = string.find(self.text, "^([^\n\r]*)")
    if string.find(first_line, "^%s*%-%-") then
        if string.find(first_line, "^%-%-##") then
            first_line = string.gsub(first_line, "^%-%-##", "")
            -- if neko_tolua_flags['C'] then
            --     s = string.gsub(s, "^%-%-##[^\n\r]*\n", "")
            -- end
        end
    else
        first_line = ""
    end

    -- pad to 16 bytes
    local npad = 16 - (#s % 16)
    local spad = ""
    for i = 1, npad do
        spad = spad .. "-"
    end
    s = s .. spad

    -- convert to C
    output('\n' .. pre .. '{ /* begin embedded lua code */\n')
    output(pre .. ' int top = lua_gettop(L);')
    output(pre .. ' static const unsigned char B[] = {\n   ')
    local t = {
        n = 0
    }

    local b = gsub(s, '(.)', function(c)
        local e = ''
        t.n = t.n + 1
        if t.n == 15 then
            t.n = 0
            e = '\n' .. pre .. '  '
        end
        return format('%3u,%s', strbyte(c), e)
    end)
    output(b .. strbyte(" "))
    output('\n' .. pre .. ' };\n')
    if first_line and first_line ~= "" then
        output(pre .. ' neko_tolua_dobuffer(L,(char*)B,sizeof(B),"tolua embedded: ' .. first_line .. '");')
    else
        output(pre .. ' neko_tolua_dobuffer(L,(char*)B,sizeof(B),"tolua: embedded Lua code ' .. code_n .. '");')
    end
    output(pre .. ' lua_settop(L, top);')
    output(pre .. '} /* end of embedded lua code */\n\n')
    code_n = code_n + 1
end

-- Print method
function classCode:print(ident, close)
    print(ident .. "Code{")
    print(ident .. " text = [[" .. self.text .. "]],")
    print(ident .. "}" .. close)
end

-- Internal constructor
function _Code(t)
    setmetatable(t, classCode)
    append(t)
    return t
end

-- Constructor
-- Expects a string representing the code text
function Code(l)
    return _Code {
        text = l
    }
end

-- Typedef class
-- Represents a type synonym.
-- The 'de facto' type replaces the typedef before the
-- remaining code is parsed.
-- The following fields are stored:
--   utype = typedef name
--   type = 'the facto' type
--   mod = modifiers to the 'de facto' type
classTypedef = {
    utype = '',
    mod = '',
    type = ''
}
classTypedef.__index = classTypedef

-- Print method
function classTypedef:print(ident, close)
    print(ident .. "Typedef{")
    print(ident .. " utype = '" .. self.utype .. "',")
    print(ident .. " mod = '" .. self.mod .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. "}" .. close)
end

-- Return it's not a variable
function classTypedef:isvariable()
    return false
end

-- Internal constructor
function _Typedef(t)
    setmetatable(t, classTypedef)
    t.type = resolve_template_types(t.type)
    appendtypedef(t)
    return t
end

-- Constructor
-- Expects one string representing the type definition.
function Typedef(s)
    if strfind(string.gsub(s, '%b<>', ''), '[%*&]') then
        neko_tolua_error("#invalid typedef: pointers (and references) are not supported")
    end
    local o = {
        mod = ''
    }
    if string.find(s, "[<>]") then
        _, _, o.type, o.utype = string.find(s, "^%s*([^<>]+%b<>[^%s]*)%s+(.-)$")
    else
        local t = split(gsub(s, "%s%s*", " "), " ")
        o = {
            utype = t[t.n],
            type = t[t.n - 1],
            mod = concat(t, 1, t.n - 2)
        }
    end
    return _Typedef(o)
end

-- table to store namespaced typedefs/enums in global scope
global_typedefs = {}
global_enums = {}

-- Container class
-- Represents a container of features to be bound
-- to lua.
classContainer = {
    curr = nil
}
classContainer.__index = classContainer
setmetatable(classContainer, classFeature)

-- output tags
function classContainer:decltype()
    push(self)
    local i = 1
    while self[i] do
        self[i]:decltype()
        i = i + 1
    end
    pop()
end

-- write support code
function classContainer:supcode()

    if not self:check_public_access() then
        return
    end

    push(self)
    local i = 1
    while self[i] do
        if self[i]:check_public_access() then
            self[i]:supcode()
        end
        i = i + 1
    end
    pop()
end

function classContainer:hasvar()
    local i = 1
    while self[i] do
        if self[i]:isvariable() then
            return 1
        end
        i = i + 1
    end
    return 0
end

-- Internal container constructor
function _Container(self)
    setmetatable(self, classContainer)
    self.n = 0
    self.typedefs = {
        neko_tolua_n = 0
    }
    self.usertypes = {}
    self.enums = {
        neko_tolua_n = 0
    }
    self.lnames = {}
    return self
end

-- push container
function push(t)
    t.prox = classContainer.curr
    classContainer.curr = t
end

-- pop container
function pop()
    -- print("name",classContainer.curr.name)
    -- foreach(classContainer.curr.usertypes,print)
    -- print("______________")
    classContainer.curr = classContainer.curr.prox
end

-- get current namespace
function getcurrnamespace()
    return getnamespace(classContainer.curr)
end

-- append to current container
function append(t)
    return classContainer.curr:append(t)
end

-- append typedef to current container
function appendtypedef(t)
    return classContainer.curr:appendtypedef(t)
end

-- append usertype to current container
function appendusertype(t)
    return classContainer.curr:appendusertype(t)
end

-- append enum to current container
function appendenum(t)
    return classContainer.curr:appendenum(t)
end

-- substitute typedef
function applytypedef(mod, type)
    return classContainer.curr:applytypedef(mod, type)
end

-- check if is type
function findtype(type)
    local t = classContainer.curr:findtype(type)
    return t
end

-- check if is typedef
function istypedef(type)
    return classContainer.curr:istypedef(type)
end

-- get fulltype (with namespace)
function fulltype(t)
    local curr = classContainer.curr
    while curr do
        if curr then
            if curr.typedefs and curr.typedefs[t] then
                return curr.typedefs[t]
            elseif curr.usertypes and curr.usertypes[t] then
                return curr.usertypes[t]
            end
        end
        curr = curr.prox
    end
    return t
end

-- checks if it requires collection
function classContainer:requirecollection(t)
    push(self)
    local i = 1
    local r = false
    while self[i] do
        r = self[i]:requirecollection(t) or r
        i = i + 1
    end
    pop()
    return r
end

-- get namesapce
function getnamespace(curr)
    local namespace = ''
    while curr do
        if curr and (curr.classtype == 'class' or curr.classtype == 'namespace') then
            namespace = (curr.original_name or curr.name) .. '::' .. namespace
            -- namespace = curr.name .. '::' .. namespace
        end
        curr = curr.prox
    end
    return namespace
end

-- get namespace (only namespace)
function getonlynamespace()
    local curr = classContainer.curr
    local namespace = ''
    while curr do
        if curr.classtype == 'class' then
            return namespace
        elseif curr.classtype == 'namespace' then
            namespace = curr.name .. '::' .. namespace
        end
        curr = curr.prox
    end
    return namespace
end

-- check if is enum
function isenum(type)
    return classContainer.curr:isenum(type)
end

-- append feature to container
function classContainer:append(t)
    self.n = self.n + 1
    self[self.n] = t
    t.parent = self
end

-- append typedef
function classContainer:appendtypedef(t)
    local namespace = getnamespace(classContainer.curr)
    self.typedefs.neko_tolua_n = self.typedefs.neko_tolua_n + 1
    self.typedefs[self.typedefs.neko_tolua_n] = t
    self.typedefs[t.utype] = namespace .. t.utype
    global_typedefs[namespace .. t.utype] = t
    t.ftype = findtype(t.type) or t.type
    -- print("appending typedef "..t.utype.." as "..namespace..t.utype.." with ftype "..t.ftype)
    append_global_type(namespace .. t.utype)
    if t.ftype and isenum(t.ftype) then

        global_enums[namespace .. t.utype] = true
    end
end

-- append usertype: return full type
function classContainer:appendusertype(t)
    local container
    if t == (self.original_name or self.name) then
        container = self.prox
    else
        container = self
    end
    local ft = getnamespace(container) .. t
    container.usertypes[t] = ft
    _usertype[ft] = ft
    return ft
end

-- append enum
function classContainer:appendenum(t)
    local namespace = getnamespace(classContainer.curr)
    self.enums.neko_tolua_n = self.enums.neko_tolua_n + 1
    self.enums[self.enums.neko_tolua_n] = t
    global_enums[namespace .. t.name] = t
end

-- determine lua function name overload
function classContainer:overload(lname)
    if not self.lnames[lname] then
        self.lnames[lname] = 0
    else
        self.lnames[lname] = self.lnames[lname] + 1
    end
    return format("%02d", self.lnames[lname])
end

-- applies typedef: returns the 'the facto' modifier and type
function classContainer:applytypedef(mod, type)
    if global_typedefs[type] then
        -- print("found typedef "..global_typedefs[type].type)
        local mod1, type1 = global_typedefs[type].mod, global_typedefs[type].ftype
        local mod2, type2 = applytypedef(mod .. " " .. mod1, type1)
        -- return mod2 .. ' ' .. mod1, type2
        return mod2, type2
    end
    do
        return mod, type
    end
end

-- check if it is a typedef
function classContainer:istypedef(type)
    local env = self
    while env do
        if env.typedefs then
            local i = 1
            while env.typedefs[i] do
                if env.typedefs[i].utype == type then
                    return type
                end
                i = i + 1
            end
        end
        env = env.parent
    end
    return nil
end

function find_enum_var(var)

    if tonumber(var) then
        return var
    end

    local c = classContainer.curr
    while c do
        local ns = getnamespace(c)
        for k, v in pairs(_global_enums) do
            if match_type(var, v, ns) then
                return v
            end
        end
        if c.base and c.base ~= '' then
            c = _global_classes[c:findtype(c.base)]
        else
            c = nil
        end
    end

    return var
end

-- check if is a registered type: return full type or nil
function classContainer:findtype(t)

    t = string.gsub(t, "=.*", "")
    if _basic[t] then
        return t
    end

    local _, _, em = string.find(t, "([&%*])%s*$")
    t = string.gsub(t, "%s*([&%*])%s*$", "")
    p = self
    while p and type(p) == 'table' do
        local st = getnamespace(p)

        for i = _global_types.n, 1, -1 do -- in reverse order

            if match_type(t, _global_types[i], st) then
                return _global_types[i] .. (em or "")
            end
        end
        if p.base and p.base ~= '' and p.base ~= t then
            -- print("type is "..t..", p is "..p.base.." self.type is "..self.type.." self.name is "..self.name)
            p = _global_classes[p:findtype(p.base)]
        else
            p = nil
        end
    end

    return nil
end

function append_global_type(t, class)
    _global_types.n = _global_types.n + 1
    _global_types[_global_types.n] = t
    _global_types_hash[t] = 1
    if class then
        append_class_type(t, class)
    end
end

function append_class_type(t, class)
    if _global_classes[t] then
        class.flags = _global_classes[t].flags
        class.lnames = _global_classes[t].lnames
        if _global_classes[t].base and (_global_classes[t].base ~= '') then
            class.base = _global_classes[t].base or class.base
        end
    end
    _global_classes[t] = class
    class.flags = class.flags or {}
end

function match_type(childtype, regtype, st)
    -- print("findtype "..childtype..", "..regtype..", "..st)
    local b, e = string.find(regtype, childtype, -string.len(childtype), true)
    if b then

        if e == string.len(regtype) and
            (b == 1 or
                (string.sub(regtype, b - 1, b - 1) == ':' and string.sub(regtype, 1, b - 1) == string.sub(st, 1, b - 1))) then
            return true
        end
    end

    return false
end

function findtype_on_childs(self, t)

    local tchild
    if self.classtype == 'class' or self.classtype == 'namespace' then
        for k, v in ipairs(self) do
            if v.classtype == 'class' or v.classtype == 'namespace' then
                if v.typedefs and v.typedefs[t] then
                    return v.typedefs[t]
                elseif v.usertypes and v.usertypes[t] then
                    return v.usertypes[t]
                end
                tchild = findtype_on_childs(v, t)
                if tchild then
                    return tchild
                end
            end
        end
    end
    return nil

end

function classContainer:isenum(type)
    if global_enums[type] then
        return type
    else
        return false
    end

    local basetype = gsub(type, "^.*::", "")
    local env = self
    while env do
        if env.enums then
            local i = 1
            while env.enums[i] do
                if env.enums[i].name == basetype then
                    return true
                end
                i = i + 1
            end
        end
        env = env.parent
    end
    return false
end

methodisvirtual = false -- a global

-- parse chunk
function classContainer:doparse(s)
    -- print ("parse "..s)

    -- try the parser hook
    do
        local sub = parser_hook(s)
        if sub then
            return sub
        end
    end

    -- try the null statement
    do
        local b, e, code = string.find(s, "^%s*;")
        if b then
            return strsub(s, e + 1)
        end
    end

    -- try empty verbatim line
    do
        local b, e, code = string.find(s, "^%s*$\n")
        if b then
            return strsub(s, e + 1)
        end
    end

    -- try Lua code
    do
        local b, e, code = strfind(s, "^%s*(%b\1\2)")
        if b then
            Code(strsub(code, 2, -2))
            return strsub(s, e + 1)
        end
    end

    -- try C code
    do
        local b, e, code = strfind(s, "^%s*(%b\3\4)")
        if b then
            code = '{' .. strsub(code, 2, -2) .. '\n}\n'
            Verbatim(code, 'r') -- verbatim code for 'r'egister fragment
            return strsub(s, e + 1)
        end
    end

    -- try C code for preamble section
    do
        local b, e, code = string.find(s, "^%s*(%b\5\6)")
        if b then
            code = string.sub(code, 2, -2) .. "\n"
            Verbatim(code, '')
            return string.sub(s, e + 1)
        end
    end

    -- try default_property directive
    do
        local b, e, ptype = strfind(s, "^%s*TOLUA_PROPERTY_TYPE%s*%(+%s*([^%)%s]*)%s*%)+%s*;?")
        if b then
            if not ptype or ptype == "" then
                ptype = "default"
            end
            self:set_property_type(ptype)
            return strsub(s, e + 1)
        end
    end

    -- try protected_destructor directive
    do
        local b, e = string.find(s, "^%s*TOLUA_PROTECTED_DESTRUCTOR%s*;?")
        if b then
            if self.set_protected_destructor then
                self:set_protected_destructor(true)
            end
            return strsub(s, e + 1)
        end
    end

    -- try 'extern' keyword
    do
        local b, e = string.find(s, "^%s*extern%s+")
        if b then
            -- do nothing
            return strsub(s, e + 1)
        end
    end

    -- try 'virtual' keyworkd
    do
        local b, e = string.find(s, "^%s*virtual%s+")
        if b then
            methodisvirtual = true
            return strsub(s, e + 1)
        end
    end

    -- try labels (public, private, etc)
    do
        local b, e = string.find(s, "^%s*%w*%s*:[^:]")
        if b then
            return strsub(s, e) -- preserve the [^:]
        end
    end

    -- try module
    do
        local b, e, name, body = strfind(s, "^%s*module%s%s*([_%w][_%w]*)%s*(%b{})%s*")
        if b then
            _curr_code = strsub(s, b, e)
            Module(name, body)
            return strsub(s, e + 1)
        end
    end

    -- try namesapce
    do
        local b, e, name, body = strfind(s, "^%s*namespace%s%s*([_%w][_%w]*)%s*(%b{})%s*;?")
        if b then
            _curr_code = strsub(s, b, e)
            Namespace(name, body)
            return strsub(s, e + 1)
        end
    end

    -- try define
    do
        local b, e, name = strfind(s, "^%s*#define%s%s*([^%s]*)[^\n]*\n%s*")
        if b then
            _curr_code = strsub(s, b, e)
            Define(name)
            return strsub(s, e + 1)
        end
    end

    -- try enumerates

    do
        local b, e, name, body, varname = strfind(s, "^%s*enum%s+(%S*)%s*(%b{})%s*([^%s;]*)%s*;?%s*")
        if b then
            -- error("#Sorry, declaration of enums and variables on the same statement is not supported.\nDeclare your variable separately (example: '"..name.." "..varname..";')")
            _curr_code = strsub(s, b, e)
            Enumerate(name, body, varname)
            return strsub(s, e + 1)
        end
    end

    -- do
    --  local b,e,name,body = strfind(s,"^%s*enum%s+(%S*)%s*(%b{})%s*;?%s*")
    --  if b then
    --   _curr_code = strsub(s,b,e)
    --   Enumerate(name,body)
    --  return strsub(s,e+1)
    --  end
    -- end

    do
        local b, e, body, name = strfind(s, "^%s*typedef%s+enum[^{]*(%b{})%s*([%w_][^%s]*)%s*;%s*")
        if b then
            _curr_code = strsub(s, b, e)
            Enumerate(name, body)
            return strsub(s, e + 1)
        end
    end

    -- try operator
    do
        local b, e, decl, kind, arg, const = strfind(s,
            "^%s*([_%w][_%w%s%*&:<>,]-%s+operator)%s*([^%s][^%s]*)%s*(%b())%s*(c?o?n?s?t?)%s*;%s*")
        if not b then
            -- try inline
            b, e, decl, kind, arg, const = strfind(s,
                "^%s*([_%w][_%w%s%*&:<>,]-%s+operator)%s*([^%s][^%s]*)%s*(%b())%s*(c?o?n?s?t?)[%s\n]*%b{}%s*;?%s*")
        end
        if not b then
            -- try cast operator
            b, e, decl, kind, arg, const = strfind(s, "^%s*(operator)%s+([%w_:%d<>%*%&%s]+)%s*(%b())%s*(c?o?n?s?t?)");
            if b then
                local _, ie = string.find(s, "^%s*%b{}", e + 1)
                if ie then
                    e = ie
                end
            end
        end
        if b then
            _curr_code = strsub(s, b, e)
            Operator(decl, kind, arg, const)
            return strsub(s, e + 1)
        end
    end

    -- try function
    do
        -- local b,e,decl,arg,const = strfind(s,"^%s*([~_%w][_@%w%s%*&:<>]*[_%w])%s*(%b())%s*(c?o?n?s?t?)%s*=?%s*0?%s*;%s*")
        local b, e, decl, arg, const, virt = strfind(s, "^%s*([^%(\n]+)%s*(%b())%s*(c?o?n?s?t?)%s*(=?%s*0?)%s*;%s*")
        if not b then
            -- try function with template
            b, e, decl, arg, const = strfind(s,
                "^%s*([~_%w][_@%w%s%*&:<>]*[_%w]%b<>)%s*(%b())%s*(c?o?n?s?t?)%s*=?%s*0?%s*;%s*")
        end
        if not b then
            -- try a single letter function name
            b, e, decl, arg, const = strfind(s, "^%s*([_%w])%s*(%b())%s*(c?o?n?s?t?)%s*;%s*")
        end
        if not b then
            -- try function pointer
            b, e, decl, arg, const = strfind(s, "^%s*([^%(;\n]+%b())%s*(%b())%s*;%s*")
            if b then
                decl = string.gsub(decl, "%(%s*%*([^%)]*)%s*%)", " %1 ")
            end
        end
        if b then
            if virt and string.find(virt, "[=0]") then
                if self.flags then
                    self.flags.pure_virtual = true
                end
            end
            _curr_code = strsub(s, b, e)
            Function(decl, arg, const)
            return strsub(s, e + 1)
        end
    end

    -- try inline function
    do
        local b, e, decl, arg, const = strfind(s, "^%s*([^%(\n]+)%s*(%b())%s*(c?o?n?s?t?)[^;{]*%b{}%s*;?%s*")
        -- local b,e,decl,arg,const = strfind(s,"^%s*([~_%w][_@%w%s%*&:<>]*[_%w>])%s*(%b())%s*(c?o?n?s?t?)[^;]*%b{}%s*;?%s*")
        if not b then
            -- try a single letter function name
            b, e, decl, arg, const = strfind(s, "^%s*([_%w])%s*(%b())%s*(c?o?n?s?t?).-%b{}%s*;?%s*")
        end
        if b then
            _curr_code = strsub(s, b, e)
            Function(decl, arg, const)
            return strsub(s, e + 1)
        end
    end

    -- try class
    do
        local b, e, name, base, body
        base = ''
        body = ''
        b, e, name = strfind(s, "^%s*class%s*([_%w][_%w@]*)%s*;") -- dummy class
        local dummy = false
        if not b then
            b, e, name = strfind(s, "^%s*struct%s*([_%w][_%w@]*)%s*;") -- dummy struct
            if not b then
                b, e, name, base, body = strfind(s, "^%s*class%s*([_%w][_%w@]*)%s*([^{]-)%s*(%b{})%s*")
                if not b then
                    b, e, name, base, body = strfind(s, "^%s*struct%s+([_%w][_%w@]*)%s*([^{]-)%s*(%b{})%s*")
                    if not b then
                        b, e, name, base, body = strfind(s, "^%s*union%s*([_%w][_%w@]*)%s*([^{]-)%s*(%b{})%s*")
                        if not b then
                            base = ''
                            b, e, body, name = strfind(s,
                                "^%s*typedef%s%s*struct%s%s*[_%w]*%s*(%b{})%s*([_%w][_%w@]*)%s*;")
                        end
                    end
                end
            else
                dummy = 1
            end
        else
            dummy = 1
        end
        if b then
            if base ~= '' then
                base = string.gsub(base, "^%s*:%s*", "")
                base = string.gsub(base, "%s*public%s*", "")
                base = split(base, ",")
                -- local b,e
                -- b,e,base = strfind(base,".-([_%w][_%w<>,:]*)$")
            else
                base = {}
            end
            _curr_code = strsub(s, b, e)
            Class(name, base, body)
            if not dummy then
                varb, vare, varname = string.find(s, "^%s*([_%w]+)%s*;", e + 1)
                if varb then
                    Variable(name .. " " .. varname)
                    e = vare
                end
            end
            return strsub(s, e + 1)
        end
    end

    -- try typedef
    do
        local b, e, types = strfind(s, "^%s*typedef%s%s*(.-)%s*;%s*")
        if b then
            _curr_code = strsub(s, b, e)
            Typedef(types)
            return strsub(s, e + 1)
        end
    end

    -- try variable
    do
        local b, e, decl = strfind(s, "^%s*([_%w][_@%s%w%d%*&:<>,]*[_%w%d])%s*;%s*")
        if b then
            _curr_code = strsub(s, b, e)

            local list = split_c_tokens(decl, ",")
            Variable(list[1])
            if list.n > 1 then
                local _, _, type = strfind(list[1], "(.-)%s+([^%s]*)$");

                local i = 2;
                while list[i] do
                    Variable(type .. " " .. list[i])
                    i = i + 1
                end
            end
            -- Variable(decl)
            return strsub(s, e + 1)
        end
    end

    -- try string
    do
        local b, e, decl = strfind(s, "^%s*([_%w]?[_%s%w%d]-char%s+[_@%w%d]*%s*%[%s*%S+%s*%])%s*;%s*")
        if b then
            _curr_code = strsub(s, b, e)
            Variable(decl)
            return strsub(s, e + 1)
        end
    end

    -- try array
    do
        local b, e, decl = strfind(s, "^%s*([_%w][][_@%s%w%d%*&:<>]*[]_%w%d])%s*;%s*")
        if b then
            _curr_code = strsub(s, b, e)
            Array(decl)
            return strsub(s, e + 1)
        end
    end

    -- no matching
    if gsub(s, "%s%s*", "") ~= "" then
        _curr_code = s
        error("#parse error")
    else
        return ""
    end

end

function classContainer:parse(s)

    -- self.curr_member_access = nil

    while s ~= '' do
        s = self:doparse(s)
        methodisvirtual = false
    end
end

-- property types

function get_property_type()

    return classContainer.curr:get_property_type()
end

function classContainer:set_property_type(ptype)
    ptype = string.gsub(ptype, "^%s*", "")
    ptype = string.gsub(ptype, "%s*$", "")

    self.property_type = ptype
end

function classContainer:get_property_type()
    return self.property_type or (self.parent and self.parent:get_property_type()) or "default"
end

-- Package class
-- Represents the whole package being bound.
-- The following fields are stored:
--    {i} = list of objects in the package.
classPackage = {
    classtype = 'package'
}
classPackage.__index = classPackage
setmetatable(classPackage, classContainer)

-- Print method
function classPackage:print()
    print("Package: " .. self.name)
    local i = 1
    while self[i] do
        self[i]:print("", "")
        i = i + 1
    end
end

function classPackage:preprocess()

    -- avoid preprocessing embedded Lua code
    local L = {}
    self.code = gsub(self.code, "\n%s*%$%[", "\1") -- deal with embedded lua code
    self.code = gsub(self.code, "\n%s*%$%]", "\2")
    self.code = gsub(self.code, "(%b\1\2)", function(c)
        tinsert(L, c)
        return "\n#[" .. getn(L) .. "]#"
    end)
    -- avoid preprocessing embedded C code
    local C = {}
    self.code = gsub(self.code, "\n%s*%$%<", "\3") -- deal with embedded C code
    self.code = gsub(self.code, "\n%s*%$%>", "\4")
    self.code = gsub(self.code, "(%b\3\4)", function(c)
        tinsert(C, c)
        return "\n#<" .. getn(C) .. ">#"
    end)
    -- avoid preprocessing embedded C code
    self.code = gsub(self.code, "\n%s*%$%{", "\5") -- deal with embedded C code
    self.code = gsub(self.code, "\n%s*%$%}", "\6")
    self.code = gsub(self.code, "(%b\5\6)", function(c)
        tinsert(C, c)
        return "\n#<" .. getn(C) .. ">#"
    end)

    -- self.code = gsub(self.code,"\n%s*#[^d][^\n]*\n", "\n\n") -- eliminate preprocessor directives that don't start with 'd'
    self.code = gsub(self.code, "\n[ \t]*#[ \t]*[^d%<%[]", "\n//") -- eliminate preprocessor directives that don't start with 'd'

    -- avoid preprocessing verbatim lines
    local V = {}
    self.code = gsub(self.code, "\n(%s*%$[^%[%]][^\n]*)", function(v)
        tinsert(V, v)
        return "\n#" .. getn(V) .. "#"
    end)

    -- perform global substitution

    self.code = gsub(self.code, "(//[^\n]*)", "") -- eliminate C++ comments
    self.code = gsub(self.code, "/%*", "\1")
    self.code = gsub(self.code, "%*/", "\2")
    self.code = gsub(self.code, "%b\1\2", "")
    self.code = gsub(self.code, "\1", "/%*")
    self.code = gsub(self.code, "\2", "%*/")
    self.code = gsub(self.code, "%s*@%s*", "@") -- eliminate spaces beside @
    self.code = gsub(self.code, "%s?inline(%s)", "%1") -- eliminate 'inline' keyword
    -- self.code = gsub(self.code,"%s?extern(%s)","%1") -- eliminate 'extern' keyword
    -- self.code = gsub(self.code,"%s?virtual(%s)","%1") -- eliminate 'virtual' keyword
    -- self.code = gsub(self.code,"public:","") -- eliminate 'public:' keyword
    self.code = gsub(self.code, "([^%w_])void%s*%*", "%1_userdata ") -- substitute 'void*'
    self.code = gsub(self.code, "([^%w_])void%s*%*", "%1_userdata ") -- substitute 'void*'
    self.code = gsub(self.code, "([^%w_])char%s*%*", "%1_cstring ") -- substitute 'char*'
    self.code = gsub(self.code, "([^%w_])lua_State%s*%*", "%1_lstate ") -- substitute 'lua_State*'

    -- restore embedded Lua code
    self.code = gsub(self.code, "%#%[(%d+)%]%#", function(n)
        return L[tonumber(n)]
    end)
    -- restore embedded C code
    self.code = gsub(self.code, "%#%<(%d+)%>%#", function(n)
        return C[tonumber(n)]
    end)
    -- restore verbatim lines
    self.code = gsub(self.code, "%#(%d+)%#", function(n)
        return V[tonumber(n)]
    end)

    self.code = string.gsub(self.code, "\n%s*%$([^\n]+)", function(l)
        Verbatim(l .. "\n")
        return "\n"
    end)
end

-- translate verbatim
function classPackage:preamble()
    output('//\n')
    output('// Neko tolua binding: ' .. self.name .. '\n')
    output('// Generated automatically by ' .. NEKO_TOLUA_VERSION .. ' on ' .. date() .. '.\n')
    output('//\n\n')

    output('#ifndef __cplusplus\n')
    output('#include "stdlib.h"\n')
    output('#endif\n')
    output('#include "string.h"\n\n')

    output('#include "engine/luax.h"\n\n')

    if not neko_tolua_flags.h then
        output('// Exported function\n')
        output('NEKO_API_DECL int  neko_tolua_' .. self.name .. '_open (lua_State* L);')
        output('\n')
    end

    local i = 1
    while self[i] do
        self[i]:preamble()
        i = i + 1
    end

    if self:requirecollection(_collect) then
        output('\n')
        output('// function to release collected object via destructor\n')
        output('#ifdef __cplusplus\n')
        for i, v in pairs(_collect) do
            output('\nstatic int ' .. v .. ' (lua_State* L)')
            output('{')
            output(' ' .. i .. '* self = (' .. i .. '*) neko_tolua_tousertype(L,1,0);')
            output('	Mneko_tolua_delete(self);')
            output('	return 0;')
            output('}')
        end
        output('#endif\n\n')
    end

    output('\n')
    output('// function to register type\n')
    output('static void neko_tolua_reg_types (lua_State* L)')
    output('{')

    output('// export a list of types asociates with the C++ typeid name\n')
    output("#ifndef Mneko_tolua_typeid\n#define Mneko_tolua_typeid(L,TI,T)\n#endif\n")
    foreach(_usertype, function(n, v)
        if (not _global_classes[v]) or _global_classes[v]:check_public_access() then
            output(' neko_tolua_usertype(L,"', v, '");')
            output(' Mneko_tolua_typeid(L,typeid(', v, '), "', v, '");')
        end
    end)
    output('}')
    output('\n')
end

-- register package
-- write package open function
function classPackage:register(pre)
    pre = pre or ''
    push(self)
    output(pre .. "/* Open function */")
    output(pre .. "NEKO_API_DECL int neko_tolua_" .. self.name .. "_open (lua_State* L)")
    output(pre .. "{")
    output(pre .. " neko_tolua_open(L);")
    output(pre .. " neko_tolua_reg_types(L);")
    output(pre .. " neko_tolua_module(L,NULL,", self:hasvar(), ");")
    output(pre .. " neko_tolua_beginmodule(L,NULL);")
    local i = 1
    while self[i] do
        self[i]:register(pre .. "  ")
        i = i + 1
    end
    output(pre .. " neko_tolua_endmodule(L);")
    output(pre .. " return 1;")
    output(pre .. "}")

    output("\n\n")
    output("#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501\n");
    output(pre .. "NEKO_API_DECL int luaopen_" .. self.name .. " (lua_State* L) {")
    output(pre .. " return neko_tolua_" .. self.name .. "_open(L);")
    output(pre .. "};")
    output("#endif\n\n")

    pop()
end

-- write header file
function classPackage:header()
    output('//\n')
    output('// Neko tolua binding: ' .. self.name .. '\n')
    output('// Generated automatically by ' .. NEKO_TOLUA_VERSION .. ' on ' .. date() .. '.\n')
    output('//\n\n')

    if not neko_tolua_flags.h then
        output('// Exported function\n')
        output('NEKO_API_DECL int  neko_tolua_' .. self.name .. '_open (lua_State* L);')
        output('\n')
    end
end

-- Internal constructor
function _Package(self)
    setmetatable(self, classPackage)
    return self
end

-- Parse C header file with tolua directives
-- *** Thanks to Ariel Manzur for fixing bugs in nested directives ***
function extract_code(fn, s)
    local code = '\n$#include "' .. fn .. '"\n'
    s = "\n" .. s .. "\n" -- add blank lines as sentinels
    local _, e, c, t = strfind(s, "\n([^\n]-)[Tt][Oo][Ll][Uu][Aa]_([^%s]*)[^\n]*\n")
    while e do
        t = strlower(t)
        if t == "begin" then
            _, e, c = strfind(s, "(.-)\n[^\n]*[Tt][Oo][Ll][Uu][Aa]_[Ee][Nn][Dd][^\n]*\n", e)
            if not e then
                neko_tolua_error("Unbalanced 'neko_tolua_begin' directive in header file")
            end
        end
        code = code .. c .. "\n"
        _, e, c, t = strfind(s, "\n([^\n]-)[Tt][Oo][Ll][Uu][Aa]_([^%s]*)[^\n]*\n", e)
    end
    return code
end

-- Constructor
-- Expects the package name, the file extension, and the file text.
function Package(name, fn)
    local ext = "pkg"

    -- open input file, if any
    local st, msg
    if fn then
        st, msg = readfrom(neko_tolua_flags.f)
        if not st then
            error('#' .. msg)
        end
        local _;
        _, _, ext = strfind(fn, ".*%.(.*)$")
    end
    local code
    if ext == 'pkg' then
        code = prep(st)
    else
        code = "\n" .. read('*a')
        if ext == 'h' or ext == 'hpp' then
            code = extract_code(fn, code)
        end
    end

    -- close file
    if fn then
        readfrom()
    end

    -- deal with include directive
    local nsubst
    repeat
        code, nsubst = gsub(code, '\n%s*%$(.)file%s*"(.-)"([^\n]*)\n', function(kind, fn, extra)
            local _, _, ext = strfind(fn, ".*%.(.*)$")
            local fp, msg = openfile(fn, 'r')
            if not fp then
                error('#' .. msg .. ': ' .. fn)
            end
            if kind == 'p' then
                local s = prep(fp)
                closefile(fp)
                return s
            end
            local s = read(fp, '*a')
            closefile(fp)
            if kind == 'c' or kind == 'h' then
                return extract_code(fn, s)
            elseif kind == 'l' then
                return "\n$[--##" .. fn .. "\n" .. s .. "\n$]\n"
            elseif kind == 'i' then
                local t = {
                    code = s
                }
                extra = string.gsub(extra, "^%s*,%s*", "")
                local pars = split_c_tokens(extra, ",")
                include_file_hook(t, fn, unpack(pars))
                return "\n\n" .. t.code
            else
                error('#Invalid include directive (use $cfile, $pfile, $lfile or $ifile)')
            end
        end)
    until nsubst == 0

    -- deal with renaming directive
    repeat -- I don't know why this is necesary
        code, nsubst = gsub(code, '\n%s*%$renaming%s*(.-)%s*\n', function(r)
            appendrenaming(r)
            return "\n"
        end)
    until nsubst == 0

    local t = _Package(_Container {
        name = name,
        code = code
    })
    push(t)
    preprocess_hook(t)
    t:preprocess()
    preparse_hook(t)
    t:parse(t.code)
    pop()
    return t
end

setmetatable(_extra_parameters, {
    __index = _G
})

function prep(file)

    local chunk = {'local __ret = {"\\n"}\n'}
    for line in file:lines() do
        if string.find(line, "^##") then
            table.insert(chunk, string.sub(line, 3) .. "\n")
        else
            local last = 1
            for text, expr, index in string.gmatch(line, "(.-)$(%b())()") do
                last = index
                if text ~= "" then
                    table.insert(chunk, string.format('table.insert(__ret, %q )', text))
                end
                table.insert(chunk, string.format('table.insert(__ret, %s )', expr))
            end
            table.insert(chunk, string.format('table.insert(__ret, %q)\n', string.sub(line, last) .. "\n"))
        end
    end
    table.insert(chunk, '\nreturn table.concat(__ret)\n')
    local f, e = load(table.concat(chunk), nil, nil, _extra_parameters)
    if e then
        error("#" .. e)
    end
    return f()
end

-- Module class
-- Represents module.
-- The following fields are stored:
--    {i} = list of objects in the module.
classModule = {
    classtype = 'module'
}
classModule.__index = classModule
setmetatable(classModule, classContainer)

-- register module
function classModule:register(pre)
    pre = pre or ''
    push(self)
    output(pre .. 'neko_tolua_module(L,"' .. self.name .. '",', self:hasvar(), ');')
    output(pre .. 'neko_tolua_beginmodule(L,"' .. self.name .. '");')
    local i = 1
    while self[i] do
        self[i]:register(pre .. ' ')
        i = i + 1
    end
    output(pre .. 'neko_tolua_endmodule(L);')
    pop()
end

-- Print method
function classModule:print(ident, close)
    print(ident .. "Module{")
    print(ident .. " name = '" .. self.name .. "';")
    local i = 1
    while self[i] do
        self[i]:print(ident .. " ", ",")
        i = i + 1
    end
    print(ident .. "}" .. close)
end

-- Internal constructor
function _Module(t)
    setmetatable(t, classModule)
    append(t)
    return t
end

-- Constructor
-- Expects two string representing the module name and body.
function Module(n, b)
    local t = _Module(_Container {
        name = n
    })
    push(t)
    t:parse(strsub(b, 2, strlen(b) - 1)) -- eliminate braces
    pop()
    return t
end

-- Namespace class
-- Represents a namesapce definition.
-- Stores the following fields:
--    name = class name
--    {i}  = list of members
classNamespace = {
    classtype = 'namespace',
    name = ''
}
classNamespace.__index = classNamespace
setmetatable(classNamespace, classModule)

-- Print method
function classNamespace:print(ident, close)
    print(ident .. "Namespace{")
    print(ident .. " name = '" .. self.name .. "',")
    local i = 1
    while self[i] do
        self[i]:print(ident .. " ", ",")
        i = i + 1
    end
    print(ident .. "}" .. close)
end

-- Internal constructor
function _Namespace(t)
    setmetatable(t, classNamespace)
    append(t)
    return t
end

-- Constructor
-- Expects the name and the body of the namespace.
function Namespace(n, b)
    local c = _Namespace(_Container {
        name = n
    })
    push(c)
    c:parse(strsub(b, 2, strlen(b) - 1)) -- eliminate braces
    pop()
end

-- Define class
-- Represents a numeric const definition
-- The following filds are stored:
--   name = constant name
classDefine = {
    name = ''
}
classDefine.__index = classDefine
setmetatable(classDefine, classFeature)

-- register define
function classDefine:register(pre)
    if not self:check_public_access() then
        return
    end

    pre = pre or ''
    output(pre .. 'neko_tolua_constant(L,"' .. self.lname .. '",' .. self.name .. ');')
end

-- Print method
function classDefine:print(ident, close)
    print(ident .. "Define{")
    print(ident .. " name = '" .. self.name .. "',")
    print(ident .. " lname = '" .. self.lname .. "',")
    print(ident .. "}" .. close)
end

-- Internal constructor
function _Define(t)
    setmetatable(t, classDefine)
    t:buildnames()

    if t.name == '' then
        error("#invalid define")
    end

    append(t)
    return t
end

-- Constructor
-- Expects a string representing the constant name
function Define(n)
    return _Define {
        name = n
    }
end

-- Verbatim class
-- Represents a line translated directed to the binding file.
-- The following filds are stored:
--   line = line text
classVerbatim = {
    line = '',
    cond = nil -- condition: where to generate the code (s=suport, r=register)
}
classVerbatim.__index = classVerbatim
setmetatable(classVerbatim, classFeature)

-- preamble verbatim
function classVerbatim:preamble()
    if self.cond == '' then
        write(self.line)
    end
end

-- support code
function classVerbatim:supcode()
    if strfind(self.cond, 's') then
        write(self.line)
        write('\n')
    end
end

-- register code
function classVerbatim:register(pre)
    if strfind(self.cond, 'r') then
        write(self.line)
    end
end

-- Print method
function classVerbatim:print(ident, close)
    print(ident .. "Verbatim{")
    print(ident .. " line = '" .. self.line .. "',")
    print(ident .. "}" .. close)
end

-- Internal constructor
function _Verbatim(t)
    setmetatable(t, classVerbatim)
    append(t)
    return t
end

-- Constructor
-- Expects a string representing the text line
function Verbatim(l, cond)
    if strsub(l, 1, 1) == "'" then
        l = strsub(l, 2)
    elseif strsub(l, 1, 1) == '$' then
        cond = 'sr' -- generates in both suport and register fragments
        l = strsub(l, 2)
    end
    return _Verbatim {
        line = l,
        cond = cond or ''
    }
end

-- Enumerate class
-- Represents enumeration
-- The following fields are stored:
--    {i} = list of constant names
classEnumerate = {}
classEnumerate.__index = classEnumerate
setmetatable(classEnumerate, classFeature)

-- register enumeration
function classEnumerate:register(pre)
    if not self:check_public_access() then
        return
    end
    pre = pre or ''
    local nspace = getnamespace(classContainer.curr)
    local i = 1
    while self[i] do
        if self.lnames[i] and self.lnames[i] ~= "" then

            output(pre .. 'neko_tolua_constant(L,"' .. self.lnames[i] .. '",' .. nspace .. self[i] .. ');')
        end
        i = i + 1
    end
end

-- Print method
function classEnumerate:print(ident, close)
    print(ident .. "Enumerate{")
    print(ident .. " name = " .. self.name)
    local i = 1
    while self[i] do
        print(ident .. " '" .. self[i] .. "'(" .. self.lnames[i] .. "),")
        i = i + 1
    end
    print(ident .. "}" .. close)
end

-- Internal constructor
function _Enumerate(t, varname)
    setmetatable(t, classEnumerate)
    append(t)
    appendenum(t)
    if varname and varname ~= "" then
        if t.name ~= "" then
            Variable(t.name .. " " .. varname)
        else
            local ns = getcurrnamespace()
            warning("Variable " .. ns .. varname .. " of type <anonymous enum> is declared as read-only")
            Variable("neko_tolua_readonly int " .. varname)
        end
    end
    local parent = classContainer.curr
    if parent then
        t.access = parent.curr_member_access
        t.global_access = t:check_public_access()
    end
    return t
end

-- Constructor
-- Expects a string representing the enumerate body
function Enumerate(n, b, varname)
    b = string.gsub(b, ",[%s\n]*}", "\n}") -- eliminate last ','
    local t = split(strsub(b, 2, -2), ',') -- eliminate braces
    local i = 1
    local e = {
        n = 0
    }
    while t[i] do
        local tt = split(t[i], '=') -- discard initial value
        e.n = e.n + 1
        e[e.n] = tt[1]
        i = i + 1
    end
    -- set lua names
    i = 1
    e.lnames = {}
    local ns = getcurrnamespace()
    while e[i] do
        local t = split(e[i], '@')
        e[i] = t[1]
        if not t[2] then
            t[2] = applyrenaming(t[1])
        end
        e.lnames[i] = t[2] or t[1]
        _global_enums[ns .. e[i]] = (ns .. e[i])
        i = i + 1
    end
    e.name = n
    if n ~= "" then
        Typedef("int " .. n)
    end
    return _Enumerate(e, varname)
end

-- Declaration class
-- Represents variable, function, or argument declaration.
-- Stores the following fields:
--  mod  = type modifiers
--  type = type
--  ptr  = "*" or "&", if representing a pointer or a reference
--  name = name
--  dim  = dimension, if a vector
--  def  = default value, if any (only for arguments)
--  ret  = "*" or "&", if value is to be returned (only for arguments)
classDeclaration = {
    mod = '',
    type = '',
    ptr = '',
    name = '',
    dim = '',
    ret = '',
    def = ''
}
classDeclaration.__index = classDeclaration
setmetatable(classDeclaration, classFeature)

-- Create an unique variable name
function create_varname()
    if not _varnumber then
        _varnumber = 0
    end
    _varnumber = _varnumber + 1
    return "neko_tolua_var_" .. _varnumber
end

-- Check declaration name
-- It also identifies default values
function classDeclaration:checkname()

    if strsub(self.name, 1, 1) == '[' and not findtype(self.type) then
        self.name = self.type .. self.name
        local m = split(self.mod, '%s%s*')
        self.type = m[m.n]
        self.mod = concat(m, 1, m.n - 1)
    end

    local t = split(self.name, '=')
    if t.n == 2 then
        self.name = t[1]
        self.def = find_enum_var(t[t.n])
    end

    local b, e, d = strfind(self.name, "%[(.-)%]")
    if b then
        self.name = strsub(self.name, 1, b - 1)
        self.dim = find_enum_var(d)
    end

    if self.type ~= '' and self.type ~= 'void' and self.name == '' then
        self.name = create_varname()
    elseif self.kind == 'var' then
        if self.type == '' and self.name ~= '' then
            self.type = self.type .. self.name
            self.name = create_varname()
        elseif findtype(self.name) then
            if self.type == '' then
                self.type = self.name
            else
                self.type = self.type .. ' ' .. self.name
            end
            self.name = create_varname()
        end
    end

    -- adjust type of string
    if self.type == 'char' and self.dim ~= '' then
        self.type = 'char*'
    end

    -- this code breaks variables in namespaces, what is its purpose?
    --	if self.kind and self.kind == 'var' then
    --		self.name = string.gsub(self.name, ":.*$", "") -- ???
    --	end
end

-- Check declaration type
-- Substitutes typedef's.
function classDeclaration:checktype()

    -- check if there is a pointer to basic type
    local basic = isbasic(self.type)
    if self.kind == 'func' and basic == 'number' and string.find(self.ptr, "%*") then
        self.type = '_userdata'
        self.ptr = ""
    end
    if basic and self.ptr ~= '' then
        self.ret = self.ptr
        self.ptr = nil
        if isbasic(self.type) == 'number' then
            self.return_userdata = true
        end
    end

    -- check if there is array to be returned
    if self.dim ~= '' and self.ret ~= '' then
        error('#invalid parameter: cannot return an array of values')
    end
    -- restore 'void*' and 'string*'
    if self.type == '_userdata' then
        self.type = 'void*'
    elseif self.type == '_cstring' then
        self.type = 'char*'
    elseif self.type == '_lstate' then
        self.type = 'lua_State*'
    end

    -- resolve types inside the templates
    if self.type then
        self.type = resolve_template_types(self.type)
    end

    --
    -- -- if returning value, automatically set default value
    -- if self.ret ~= '' and self.def == '' then
    --  self.def = '0'
    -- end
    --

end

function resolve_template_types(type)

    if isbasic(type) then
        return type
    end
    local b, _, m = string.find(type, "(%b<>)")
    if b then

        m = split_c_tokens(string.sub(m, 2, -2), ",")
        for i = 1, #m do
            m[i] = string.gsub(m[i], "%s*([%*&])", "%1")
            if not isbasic(m[i]) then
                if not isenum(m[i]) then
                    _, m[i] = applytypedef("", m[i])
                end
                m[i] = findtype(m[i]) or m[i]
                m[i] = resolve_template_types(m[i])
            end
        end

        local b, i
        type, b, i = break_template(type)
        -- print("concat is ",concat(m, 1, m.n))
        local template_part = "<" .. concat(m, 1, m.n, ",") .. ">"
        type = rebuild_template(type, b, template_part)
        type = string.gsub(type, ">>", "> >")
    end
    return type
end

function break_template(s)
    local b, e, timpl = string.find(s, "(%b<>)")
    if timpl then
        s = string.gsub(s, "%b<>", "")
        return s, b, timpl
    else
        return s, 0, nil
    end
end

function rebuild_template(s, b, timpl)

    if b == 0 then
        return s
    end

    return string.sub(s, 1, b - 1) .. timpl .. string.sub(s, b, -1)
end

-- Print method
function classDeclaration:print(ident, close)
    print(ident .. "Declaration{")
    print(ident .. " mod  = '" .. self.mod .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. " ptr  = '" .. self.ptr .. "',")
    print(ident .. " name = '" .. self.name .. "',")
    print(ident .. " dim  = '" .. self.dim .. "',")
    print(ident .. " def  = '" .. self.def .. "',")
    print(ident .. " ret  = '" .. self.ret .. "',")
    print(ident .. "}" .. close)
end

-- check if array of values are returned to Lua
function classDeclaration:requirecollection(t)
    if self.mod ~= 'const' and self.dim and self.dim ~= '' and not isbasic(self.type) and self.ptr == '' and
        self:check_public_access() then
        local type = gsub(self.type, "%s*const%s+", "")
        t[type] = "neko_tolua_collect_" .. clean_template(type)
        return true
    end
    return false
end

-- declare tag
function classDeclaration:decltype()

    self.type = typevar(self.type)
    if strfind(self.mod, 'const') then
        self.type = 'const ' .. self.type
        self.mod = gsub(self.mod, 'const%s*', '')
    end
end

-- output type checking
function classDeclaration:outchecktype(narg)
    local def
    local t = isbasic(self.type)
    if self.def ~= '' then
        def = 1
    else
        def = 0
    end
    if self.dim ~= '' then
        -- if t=='string' then
        --	return 'neko_tolua_isstringarray(L,'..narg..','..def..',&neko_tolua_err)'
        -- else
        return '!neko_tolua_istable(L,' .. narg .. ',0,&neko_tolua_err)'
        -- end
    elseif t then
        return '!neko_tolua_is' .. t .. '(L,' .. narg .. ',' .. def .. ',&neko_tolua_err)'
    else
        local is_func = get_is_function(self.type)
        if self.ptr == '&' or self.ptr == '' then
            return '(neko_tolua_isvaluenil(L,' .. narg .. ',&neko_tolua_err) || !' .. is_func .. '(L,' .. narg .. ',"' ..
                       self.type .. '",' .. def .. ',&neko_tolua_err))'
        else
            return '!' .. is_func .. '(L,' .. narg .. ',"' .. self.type .. '",' .. def .. ',&neko_tolua_err)'
        end
    end
end

function classDeclaration:builddeclaration(narg, cplusplus)
    local array = self.dim ~= '' and tonumber(self.dim) == nil
    local line = ""
    local ptr = ''
    local mod
    local type = self.type
    local nctype = gsub(self.type, 'const%s+', '')
    if self.dim ~= '' then
        type = gsub(self.type, 'const%s+', '') -- eliminates const modifier for arrays
    end
    if self.ptr ~= '' and not isbasic(type) then
        ptr = '*'
    end
    line = concatparam(line, " ", self.mod, type, ptr)
    if array then
        line = concatparam(line, '*')
    end
    line = concatparam(line, self.name)
    if self.dim ~= '' then
        if tonumber(self.dim) ~= nil then
            line = concatparam(line, '[', self.dim, '];')
        else
            if cplusplus then
                line = concatparam(line, ' = Mneko_tolua_new_dim(', type, ptr, ', ' .. self.dim .. ');')
            else
                line = concatparam(line, ' = (', type, ptr, '*)', 'malloc((', self.dim, ')*sizeof(', type, ptr, '));')
            end
        end
    else
        local t = isbasic(type)
        line = concatparam(line, ' = ')
        if t == 'state' then
            line = concatparam(line, 'L;')
        else
            -- print("t is "..tostring(t)..", ptr is "..tostring(self.ptr))
            if t == 'number' and string.find(self.ptr, "%*") then
                t = 'userdata'
            end
            if not t and ptr == '' then
                line = concatparam(line, '*')
            end
            line = concatparam(line, '((', self.mod, type)
            if not t then
                line = concatparam(line, '*')
            end
            line = concatparam(line, ') ')
            if isenum(nctype) then
                line = concatparam(line, '(int) ')
            end
            local def = 0
            if self.def ~= '' then
                def = self.def
                if (ptr == '' or self.ptr == '&') and not t then
                    def = "(void*)&(const " .. type .. ")" .. def
                end
            end
            if t then
                line = concatparam(line, 'neko_tolua_to' .. t, '(L,', narg, ',', def, '));')
            else
                local to_func = get_to_function(type)
                line = concatparam(line, to_func .. '(L,', narg, ',', def, '));')
            end
        end
    end
    return line
end

-- Declare variable
function classDeclaration:declare(narg)
    if self.dim ~= '' and tonumber(self.dim) == nil then
        output('#ifdef __cplusplus\n')
        output(self:builddeclaration(narg, true))
        output('#else\n')
        output(self:builddeclaration(narg, false))
        output('#endif\n')
    else
        output(self:builddeclaration(narg, false))
    end
end

-- Get parameter value
function classDeclaration:getarray(narg)
    if self.dim ~= '' then
        local type = gsub(self.type, 'const ', '')
        output('  {')
        output('#ifndef TOLUA_RELEASE\n')
        local def;
        if self.def ~= '' then
            def = 1
        else
            def = 0
        end
        local t = isbasic(type)
        if (t) then
            output('   if (!neko_tolua_is' .. t .. 'array(L,', narg, ',', self.dim, ',', def, ',&neko_tolua_err))')
        else
            output('   if (!neko_tolua_isusertypearray(L,', narg, ',"', type, '",', self.dim, ',', def, ',&neko_tolua_err))')
        end
        output('    goto neko_tolua_lerror;')
        output('   else\n')
        output('#endif\n')
        output('   {')
        output('    int i;')
        output('    for(i=0; i<' .. self.dim .. ';i++)')
        local t = isbasic(type)
        local ptr = ''
        if self.ptr ~= '' then
            ptr = '*'
        end
        output('   ', self.name .. '[i] = ')
        if not t and ptr == '' then
            output('*')
        end
        output('((', type)
        if not t then
            output('*')
        end
        output(') ')
        local def = 0
        if self.def ~= '' then
            def = self.def
        end
        if t then
            output('neko_tolua_tofield' .. t .. '(L,', narg, ',i+1,', def, '));')
        else
            output('neko_tolua_tofieldusertype(L,', narg, ',i+1,', def, '));')
        end
        output('   }')
        output('  }')
    end
end

-- Get parameter value
function classDeclaration:setarray(narg)
    if not strfind(self.type, 'const%s+') and self.dim ~= '' then
        local type = gsub(self.type, 'const ', '')
        output('  {')
        output('   int i;')
        output('   for(i=0; i<' .. self.dim .. ';i++)')
        local t, ct = isbasic(type)
        if t then
            output('    neko_tolua_pushfield' .. t .. '(L,', narg, ',i+1,(', ct, ')', self.name, '[i]);')
        else
            if self.ptr == '' then
                output('   {')
                output('#ifdef __cplusplus\n')
                output('    void* neko_tolua_obj = Mneko_tolua_new((', type, ')(', self.name, '[i]));')
                output('    neko_tolua_pushfieldusertype_and_takeownership(L,', narg, ',i+1,neko_tolua_obj,"', type, '");')
                output('#else\n')
                output('    void* neko_tolua_obj = neko_tolua_copy(L,(void*)&', self.name, '[i],sizeof(', type, '));')
                output('    neko_tolua_pushfieldusertype(L,', narg, ',i+1,neko_tolua_obj,"', type, '");')
                output('#endif\n')
                output('   }')
            else
                output('   neko_tolua_pushfieldusertype(L,', narg, ',i+1,(void*)', self.name, '[i],"', type, '");')
            end
        end
        output('  }')
    end
end

-- Free dynamically allocated array
function classDeclaration:freearray()
    if self.dim ~= '' and tonumber(self.dim) == nil then
        output('#ifdef __cplusplus\n')
        output('  Mneko_tolua_delete_dim(', self.name, ');')
        output('#else\n')
        output('  free(', self.name, ');')
        output('#endif\n')
    end
end

-- Pass parameter
function classDeclaration:passpar()
    if self.ptr == '&' and not isbasic(self.type) then
        output('*' .. self.name)
    elseif self.ret == '*' then
        output('&' .. self.name)
    else
        output(self.name)
    end
end

-- Return parameter value
function classDeclaration:retvalue()
    if self.ret ~= '' then
        local t, ct = isbasic(self.type)
        if t and t ~= '' then
            output('   neko_tolua_push' .. t .. '(L,(', ct, ')' .. self.name .. ');')
        else
            local push_func = get_push_function(self.type)
            output('   ', push_func, '(L,(void*)' .. self.name .. ',"', self.type, '");')
        end
        return 1
    end
    return 0
end

-- Internal constructor
function _Declaration(t)

    setmetatable(t, classDeclaration)
    t:buildnames()
    t:checkname()
    t:checktype()
    local ft = findtype(t.type) or t.type
    if not isenum(ft) then
        t.mod, t.type = applytypedef(t.mod, ft)
    end

    if t.kind == "var" and (string.find(t.mod, "neko_tolua_property%s") or string.find(t.mod, "neko_tolua_property$")) then
        t.mod = string.gsub(t.mod, "neko_tolua_property", "neko_tolua_property__" .. get_property_type())
    end

    return t
end

-- Constructor
-- Expects the string declaration.
-- The kind of declaration can be "var" or "func".
function Declaration(s, kind, is_parameter)

    -- eliminate spaces if default value is provided
    s = gsub(s, "%s*=%s*", "=")
    s = gsub(s, "%s*<", "<")

    local defb, tmpdef
    defb, _, tmpdef = string.find(s, "(=.*)$")
    if defb then
        s = string.gsub(s, "=.*$", "")
    else
        tmpdef = ''
    end
    if kind == "var" then
        -- check the form: void
        if s == '' or s == 'void' then
            return _Declaration {
                type = 'void',
                kind = kind,
                is_parameter = is_parameter
            }
        end
    end

    -- check the form: mod type*& name
    local t = split_c_tokens(s, '%*%s*&')
    if t.n == 2 then
        if kind == 'func' then
            error("#invalid function return type: " .. s)
        end
        -- local m = split(t[1],'%s%s*')
        local m = split_c_tokens(t[1], '%s+')
        return _Declaration {
            name = t[2] .. tmpdef,
            ptr = '*',
            ret = '&',
            -- type = rebuild_template(m[m.n], tb, timpl),
            type = m[m.n],
            mod = concat(m, 1, m.n - 1),
            is_parameter = is_parameter,
            kind = kind
        }
    end

    -- check the form: mod type** name
    t = split_c_tokens(s, '%*%s*%*')
    if t.n == 2 then
        if kind == 'func' then
            error("#invalid function return type: " .. s)
        end
        -- local m = split(t[1],'%s%s*')
        local m = split_c_tokens(t[1], '%s+')
        return _Declaration {
            name = t[2] .. tmpdef,
            ptr = '*',
            ret = '*',
            -- type = rebuild_template(m[m.n], tb, timpl),
            type = m[m.n],
            mod = concat(m, 1, m.n - 1),
            is_parameter = is_parameter,
            kind = kind
        }
    end

    -- check the form: mod type& name
    t = split_c_tokens(s, '&')
    if t.n == 2 then
        -- local m = split(t[1],'%s%s*')
        local m = split_c_tokens(t[1], '%s+')
        return _Declaration {
            name = t[2] .. tmpdef,
            ptr = '&',
            -- type = rebuild_template(m[m.n], tb, timpl),
            type = m[m.n],
            mod = concat(m, 1, m.n - 1),
            is_parameter = is_parameter,
            kind = kind
        }
    end

    -- check the form: mod type* name
    local s1 = gsub(s, "(%b%[%])", function(n)
        return gsub(n, '%*', '\1')
    end)
    t = split_c_tokens(s1, '%*')
    if t.n == 2 then
        t[2] = gsub(t[2], '\1', '%*') -- restore * in dimension expression
        -- local m = split(t[1],'%s%s*')
        local m = split_c_tokens(t[1], '%s+')
        return _Declaration {
            name = t[2] .. tmpdef,
            ptr = '*',
            type = m[m.n],
            -- type = rebuild_template(m[m.n], tb, timpl),
            mod = concat(m, 1, m.n - 1),
            is_parameter = is_parameter,
            kind = kind
        }
    end

    if kind == 'var' then
        -- check the form: mod type name
        -- t = split(s,'%s%s*')
        t = split_c_tokens(s, '%s+')
        local v
        if findtype(t[t.n]) then
            v = create_varname()
        else
            v = t[t.n];
            t.n = t.n - 1
        end
        return _Declaration {
            name = v .. tmpdef,
            -- type = rebuild_template(t[t.n], tb, timpl),
            type = t[t.n],
            mod = concat(t, 1, t.n - 1),
            is_parameter = is_parameter,
            kind = kind
        }

    else -- kind == "func"

        -- check the form: mod type name
        -- t = split(s,'%s%s*')
        t = split_c_tokens(s, '%s+')
        local v = t[t.n] -- last word is the function name
        local tp, md
        if t.n > 1 then
            tp = t[t.n - 1]
            md = concat(t, 1, t.n - 2)
        end
        -- if tp then tp = rebuild_template(tp, tb, timpl) end
        return _Declaration {
            name = v,
            type = tp,
            mod = md,
            is_parameter = is_parameter,
            kind = kind
        }
    end

end

-- Variable class
-- Represents a extern variable or a public member of a class.
-- Stores all fields present in a declaration.
classVariable = {
    _get = {}, -- mapped get functions
    _set = {} -- mapped set functions
}
classVariable.__index = classVariable
setmetatable(classVariable, classDeclaration)

-- Print method
function classVariable:print(ident, close)
    print(ident .. "Variable{")
    print(ident .. " mod  = '" .. self.mod .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. " ptr  = '" .. self.ptr .. "',")
    print(ident .. " name = '" .. self.name .. "',")
    if self.dim then
        print(ident .. " dim = '" .. self.dim .. "',")
    end
    print(ident .. " def  = '" .. self.def .. "',")
    print(ident .. " ret  = '" .. self.ret .. "',")
    print(ident .. "}" .. close)
end

-- Generates C function name
function classVariable:cfuncname(prefix)
    local parent = ""
    local unsigned = ""
    local ptr = ""

    local p = self:inmodule() or self:innamespace() or self:inclass()

    if p then
        if self.parent.classtype == 'class' then
            parent = "_" .. self.parent.type
        else
            parent = "_" .. p
        end
    end

    if strfind(self.mod, "(unsigned)") then
        unsigned = "_unsigned"
    end

    if self.ptr == "*" then
        ptr = "_ptr"
    elseif self.ptr == "&" then
        ptr = "_ref"
    end

    local name = prefix .. parent .. unsigned .. "_" .. gsub(self.lname or self.name, ".*::", "") .. ptr

    name = clean_template(name)
    return name

end

-- check if it is a variable
function classVariable:isvariable()
    return true
end

-- get variable value
function classVariable:getvalue(class, static, prop_get)

    local name
    if prop_get then

        name = prop_get .. "()"
    else
        name = self.name
    end

    if class and static then
        return self.parent.type .. '::' .. name
    elseif class then
        return 'self->' .. name
    else
        return name
    end
end

-- get variable pointer value
function classVariable:getpointervalue(class, static)
    if class and static then
        return class .. '::p'
    elseif class then
        return 'self->p'
    else
        return 'p'
    end
end

-- Write binding functions
function classVariable:supcode()

    local class = self:inclass()

    local prop_get, prop_set
    if string.find(self.mod, 'neko_tolua_property') then

        local _, _, type = string.find(self.mod, "neko_tolua_property__([^%s]*)")
        type = type or "default"
        prop_get, prop_set = get_property_methods(type, self.name)
        self.mod = string.gsub(self.mod, "neko_tolua_property[^%s]*", "")
    end

    -- get function ------------------------------------------------
    if class then
        output("/* get function:", self.name, " of class ", class, " */")
    else
        output("/* get function:", self.name, " */")
    end
    self.cgetname = self:cfuncname("neko_tolua_get")
    output("#ifndef TOLUA_DISABLE_" .. self.cgetname)
    output("\nstatic int", self.cgetname, "(lua_State* L)")
    output("{")

    -- declare self, if the case
    local _, _, static = strfind(self.mod, '^%s*(static)')
    if class and static == nil then
        output(' ', self.parent.type, '*', 'self = ')
        output('(', self.parent.type, '*) ')
        local to_func = get_to_function(self.parent.type)
        output(to_func, '(L,1,0);')
    elseif static then
        _, _, self.mod = strfind(self.mod, '^%s*static%s%s*(.*)')
    end

    -- check self value
    if class and static == nil then
        output('#ifndef TOLUA_RELEASE\n')
        output('  if (!self) neko_tolua_error(L,"' ..
                   output_error_hook("invalid \'self\' in accessing variable \'%s\'", self.name) .. '",NULL);');
        output('#endif\n')
    end

    -- return value
    if string.find(self.mod, 'neko_tolua_inherits') then
        local push_func = get_push_function(self.type)
        output('#ifdef __cplusplus\n')
        output('  ', push_func, '(L,(void*)static_cast<' .. self.type .. '*>(self), "', self.type, '");')
        output('#else\n')
        output('  ', push_func, '(L,(void*)((' .. self.type .. '*)self), "', self.type, '");')
        output('#endif\n')
    else
        local t, ct = isbasic(self.type)
        if t then
            output('  neko_tolua_push' .. t .. '(L,(', ct, ')' .. self:getvalue(class, static, prop_get) .. ');')
        else
            local push_func = get_push_function(self.type)
            t = self.type
            if self.ptr == '&' or self.ptr == '' then
                output('  ', push_func, '(L,(void*)&' .. self:getvalue(class, static, prop_get) .. ',"', t, '");')
            else
                output('  ', push_func, '(L,(void*)' .. self:getvalue(class, static, prop_get) .. ',"', t, '");')
            end
        end
    end
    output(' return 1;')
    output('}')
    output('#endif //#ifndef TOLUA_DISABLE\n')
    output('\n')

    -- set function ------------------------------------------------
    if not (strfind(self.type, 'const%s+') or string.find(self.mod, 'neko_tolua_readonly') or
        string.find(self.mod, 'neko_tolua_inherits')) then
        if class then
            output("/* set function:", self.name, " of class ", class, " */")
        else
            output("/* set function:", self.name, " */")
        end
        self.csetname = self:cfuncname("neko_tolua_set")
        output("#ifndef TOLUA_DISABLE_" .. self.csetname)
        output("\nstatic int", self.csetname, "(lua_State* L)")
        output("{")

        -- declare self, if the case
        if class and static == nil then
            output(' ', self.parent.type, '*', 'self = ')
            output('(', self.parent.type, '*) ')
            local to_func = get_to_function(self.parent.type)
            output(to_func, '(L,1,0);')
            -- check self value
        end
        -- check types
        output('#ifndef TOLUA_RELEASE\n')
        output('  neko_tolua_Error neko_tolua_err;')
        if class and static == nil then
            output('  if (!self) neko_tolua_error(L,"' ..
                       output_error_hook("invalid \'self\' in accessing variable \'%s\'", self.name) .. '",NULL);');
        elseif static then
            _, _, self.mod = strfind(self.mod, '^%s*static%s%s*(.*)')
        end

        -- check variable type
        output('  if (' .. self:outchecktype(2) .. ')')
        output('   neko_tolua_error(L,"#vinvalid type in variable assignment.",&neko_tolua_err);')
        output('#endif\n')

        -- assign value
        local def = 0
        if self.def ~= '' then
            def = self.def
        end
        if self.type == 'char*' and self.dim ~= '' then -- is string
            output(' strncpy((char*)')
            if class and static then
                output(self.parent.type .. '::' .. self.name)
            elseif class then
                output('self->' .. self.name)
            else
                output(self.name)
            end
            output(',(const char*)neko_tolua_tostring(L,2,', def, '),', self.dim, '-1);')
        else
            local ptr = ''
            if self.ptr ~= '' then
                ptr = '*'
            end
            output(' ')
            local name = prop_set or self.name
            if class and static then
                output(self.parent.type .. '::' .. name)
            elseif class then
                output('self->' .. name)
            else
                output(name)
            end
            local t = isbasic(self.type)
            if prop_set then
                output('(')
            else
                output(' = ')
            end
            if not t and ptr == '' then
                output('*')
            end
            output('((', self.mod, self.type)
            if not t then
                output('*')
            end
            output(') ')
            if t then
                if isenum(self.type) then
                    output('(int) ')
                end
                output('neko_tolua_to' .. t, '(L,2,', def, '))')
            else
                local to_func = get_to_function(self.type)
                output(to_func, '(L,2,', def, '))')
            end
            if prop_set then
                output(")")
            end
            output(";")
        end
        output(' return 0;')
        output('}')
        output('#endif //#ifndef TOLUA_DISABLE\n')
        output('\n')
    end

end

function classVariable:register(pre)

    if not self:check_public_access() then
        return
    end
    pre = pre or ''
    local parent = self:inmodule() or self:innamespace() or self:inclass()
    if not parent then
        if classVariable._warning == nil then
            warning("Mapping variable to global may degrade performance")
            classVariable._warning = 1
        end
    end
    if self.csetname then
        output(pre .. 'neko_tolua_variable(L,"' .. self.lname .. '",' .. self.cgetname .. ',' .. self.csetname .. ');')
    else
        output(pre .. 'neko_tolua_variable(L,"' .. self.lname .. '",' .. self.cgetname .. ',NULL);')
    end
end

-- Internal constructor
function _Variable(t)
    setmetatable(t, classVariable)
    append(t)
    return t
end

-- Constructor
-- Expects a string representing the variable declaration.
function Variable(s)
    return _Variable(Declaration(s, 'var'))
end

-- Array class
-- Represents a extern array variable or a public member of a class.
-- Stores all fields present in a declaration.
classArray = {}
classArray.__index = classArray
setmetatable(classArray, classDeclaration)

-- Print method
function classArray:print(ident, close)
    print(ident .. "Array{")
    print(ident .. " mod  = '" .. self.mod .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. " ptr  = '" .. self.ptr .. "',")
    print(ident .. " name = '" .. self.name .. "',")
    print(ident .. " def  = '" .. self.def .. "',")
    print(ident .. " dim  = '" .. self.dim .. "',")
    print(ident .. " ret  = '" .. self.ret .. "',")
    print(ident .. "}" .. close)
end

-- check if it is a variable
function classArray:isvariable()
    return true
end

-- get variable value
function classArray:getvalue(class, static)
    if class and static then
        return class .. '::' .. self.name .. '[neko_tolua_index]'
    elseif class then
        return 'self->' .. self.name .. '[neko_tolua_index]'
    else
        return self.name .. '[neko_tolua_index]'
    end
end

-- Write binding functions
function classArray:supcode()
    local class = self:inclass()

    -- get function ------------------------------------------------
    if class then
        output("/* get function:", self.name, " of class ", class, " */")
    else
        output("/* get function:", self.name, " */")
    end
    self.cgetname = self:cfuncname("neko_tolua_get")
    output("#ifndef TOLUA_DISABLE_" .. self.cgetname)
    output("\nstatic int", self.cgetname, "(lua_State* L)")
    output("{")
    output(" int neko_tolua_index;")

    -- declare self, if the case
    local _, _, static = strfind(self.mod, '^%s*(static)')
    if class and static == nil then
        output(' ', self.parent.type, '*', 'self;')
        output(' lua_pushstring(L,".self");')
        output(' lua_rawget(L,1);')
        output(' self = ')
        output('(', self.parent.type, '*) ')
        output('lua_touserdata(L,-1);')
    elseif static then
        _, _, self.mod = strfind(self.mod, '^%s*static%s%s*(.*)')
    end

    -- check index
    output('#ifndef TOLUA_RELEASE\n')
    output(' {')
    output('  neko_tolua_Error neko_tolua_err;')
    output('  if (!neko_tolua_isnumber(L,2,0,&neko_tolua_err))')
    output('   neko_tolua_error(L,"#vinvalid type in array indexing.",&neko_tolua_err);')
    output(' }')
    output('#endif\n')
    output(' neko_tolua_index = (int)neko_tolua_tonumber(L,2,0);\n')
    output('#ifndef TOLUA_RELEASE\n')
    if self.dim and self.dim ~= '' then
        output(' if (neko_tolua_index<0 || neko_tolua_index>=' .. self.dim .. ')')
    else
        output(' if (neko_tolua_index<0)')
    end
    output('  neko_tolua_error(L,"array indexing out of range.",NULL);')
    output('#endif\n')

    -- return value
    local t, ct = isbasic(self.type)
    local push_func = get_push_function(t)
    if t then
        output(' neko_tolua_push' .. t .. '(L,(', ct, ')' .. self:getvalue(class, static) .. ');')
    else
        t = self.type
        if self.ptr == '&' or self.ptr == '' then
            output(' ', push_func, '(L,(void*)&' .. self:getvalue(class, static) .. ',"', t, '");')
        else
            output(' ', push_func, '(L,(void*)' .. self:getvalue(class, static) .. ',"', t, '");')
        end
    end
    output(' return 1;')
    output('}')
    output('#endif //#ifndef TOLUA_DISABLE\n')
    output('\n')

    -- set function ------------------------------------------------
    if not strfind(self.type, 'const') then
        if class then
            output("/* set function:", self.name, " of class ", class, " */")
        else
            output("/* set function:", self.name, " */")
        end
        self.csetname = self:cfuncname("neko_tolua_set")
        output("#ifndef TOLUA_DISABLE_" .. self.csetname)
        output("\nstatic int", self.csetname, "(lua_State* L)")
        output("{")

        -- declare index
        output(' int neko_tolua_index;')

        -- declare self, if the case
        local _, _, static = strfind(self.mod, '^%s*(static)')
        if class and static == nil then
            output(' ', self.parent.type, '*', 'self;')
            output(' lua_pushstring(L,".self");')
            output(' lua_rawget(L,1);')
            output(' self = ')
            output('(', self.parent.type, '*) ')
            output('lua_touserdata(L,-1);')
        elseif static then
            _, _, self.mod = strfind(self.mod, '^%s*static%s%s*(.*)')
        end

        -- check index
        output('#ifndef TOLUA_RELEASE\n')
        output(' {')
        output('  neko_tolua_Error neko_tolua_err;')
        output('  if (!neko_tolua_isnumber(L,2,0,&neko_tolua_err))')
        output('   neko_tolua_error(L,"#vinvalid type in array indexing.",&neko_tolua_err);')
        output(' }')
        output('#endif\n')

        output(' neko_tolua_index = (int)neko_tolua_tonumber(L,2,0);\n')

        output('#ifndef TOLUA_RELEASE\n')
        if self.dim and self.dim ~= '' then
            output(' if (neko_tolua_index<0 || neko_tolua_index>=' .. self.dim .. ')')
        else
            output(' if (neko_tolua_index<0)')
        end
        output('  neko_tolua_error(L,"array indexing out of range.",NULL);')
        output('#endif\n')

        -- assign value
        local ptr = ''
        if self.ptr ~= '' then
            ptr = '*'
        end
        output(' ')
        if class and static then
            output(class .. '::' .. self.name .. '[neko_tolua_index]')
        elseif class then
            output('self->' .. self.name .. '[neko_tolua_index]')
        else
            output(self.name .. '[neko_tolua_index]')
        end
        local t = isbasic(self.type)
        output(' = ')
        if not t and ptr == '' then
            output('*')
        end
        output('((', self.mod, self.type)
        if not t then
            output('*')
        end
        output(') ')
        local def = 0
        if self.def ~= '' then
            def = self.def
        end
        if t then
            output('neko_tolua_to' .. t, '(L,3,', def, '));')
        else
            local to_func = get_to_function(self.type)
            output(to_func, '(L,3,', def, '));')
        end
        output(' return 0;')
        output('}')
        output('#endif //#ifndef TOLUA_DISABLE\n')
        output('\n')
    end

end

function classArray:register(pre)
    if not self:check_public_access() then
        return
    end

    pre = pre or ''
    if self.csetname then
        output(pre .. 'neko_tolua_array(L,"' .. self.lname .. '",' .. self.cgetname .. ',' .. self.csetname .. ');')
    else
        output(pre .. 'neko_tolua_array(L,"' .. self.lname .. '",' .. self.cgetname .. ',NULL);')
    end
end

-- Internal constructor
function _Array(t)
    setmetatable(t, classArray)
    append(t)
    return t
end

-- Constructor
-- Expects a string representing the variable declaration.
function Array(s)
    return _Array(Declaration(s, 'var'))
end

-- Function class
-- Represents a function or a class method.
-- The following fields are stored:
--  mod  = type modifiers
--  type = type
--  ptr  = "*" or "&", if representing a pointer or a reference
--  name = name
--  lname = lua name
--  args  = list of argument declarations
--  const = if it is a method receiving a const "this".
classFunction = {
    mod = '',
    type = '',
    ptr = '',
    name = '',
    args = {
        n = 0
    },
    const = ''
}
classFunction.__index = classFunction
setmetatable(classFunction, classFeature)

-- declare tags
function classFunction:decltype()
    self.type = typevar(self.type)
    if strfind(self.mod, 'const') then
        self.type = 'const ' .. self.type
        self.mod = gsub(self.mod, 'const', '')
    end
    local i = 1
    while self.args[i] do
        self.args[i]:decltype()
        i = i + 1
    end
end

-- Write binding function
-- Outputs C/C++ binding function.
function classFunction:supcode(local_constructor)

    local overload = strsub(self.cname, -2, -1) - 1 -- indicate overloaded func
    local nret = 0 -- number of returned values
    local class = self:inclass()
    local _, _, static = strfind(self.mod, '^%s*(static)')
    if class then

        if self.name == 'new' and self.parent.flags.pure_virtual then
            -- no constructor for classes with pure virtual methods
            return
        end

        if local_constructor then
            output("/* method: new_local of class ", class, " */")
        else
            output("/* method:", self.name, " of class ", class, " */")
        end
    else
        output("/* function:", self.name, " */")
    end

    if local_constructor then
        output("#ifndef TOLUA_DISABLE_" .. self.cname .. "_local")
        output("\nstatic int", self.cname .. "_local", "(lua_State* L)")
    else
        output("#ifndef TOLUA_DISABLE_" .. self.cname)
        output("\nstatic int", self.cname, "(lua_State* L)")
    end
    output("{")

    -- check types
    if overload < 0 then
        output('#ifndef TOLUA_RELEASE\n')
    end
    output(' neko_tolua_Error neko_tolua_err;')
    output(' if (\n')
    -- check self
    local narg
    if class then
        narg = 2
    else
        narg = 1
    end
    if class then
        local func = get_is_function(self.parent.type)
        local type = self.parent.type
        if self.name == 'new' or static ~= nil then
            func = 'neko_tolua_isusertable'
            type = self.parent.type
        end
        if self.const ~= '' then
            type = "const " .. type
        end
        output('     !' .. func .. '(L,1,"' .. type .. '",0,&neko_tolua_err) ||\n')
    end
    -- check args
    if self.args[1].type ~= 'void' then
        local i = 1
        while self.args[i] do
            local btype = isbasic(self.args[i].type)
            if btype ~= 'value' and btype ~= 'state' then
                output('     ' .. self.args[i]:outchecktype(narg) .. ' ||\n')
            end
            if btype ~= 'state' then
                narg = narg + 1
            end
            i = i + 1
        end
    end
    -- check end of list
    output('     !neko_tolua_isnoobj(L,' .. narg .. ',&neko_tolua_err)\n )')
    output('  goto neko_tolua_lerror;')

    output(' else\n')
    if overload < 0 then
        output('#endif\n')
    end
    output(' {')

    -- declare self, if the case
    local narg
    if class then
        narg = 2
    else
        narg = 1
    end
    if class and self.name ~= 'new' and static == nil then
        output(' ', self.const, self.parent.type, '*', 'self = ')
        output('(', self.const, self.parent.type, '*) ')
        local to_func = get_to_function(self.parent.type)
        output(to_func, '(L,1,0);')
    elseif static then
        _, _, self.mod = strfind(self.mod, '^%s*static%s%s*(.*)')
    end
    -- declare parameters
    if self.args[1].type ~= 'void' then
        local i = 1
        while self.args[i] do
            self.args[i]:declare(narg)
            if isbasic(self.args[i].type) ~= "state" then
                narg = narg + 1
            end
            i = i + 1
        end
    end

    -- check self
    if class and self.name ~= 'new' and static == nil then
        output('#ifndef TOLUA_RELEASE\n')
        output('  if (!self) neko_tolua_error(L,"' .. output_error_hook("invalid \'self\' in function \'%s\'", self.name) ..
                   '", NULL);');
        output('#endif\n')
    end

    -- get array element values
    if class then
        narg = 2
    else
        narg = 1
    end
    if self.args[1].type ~= 'void' then
        local i = 1
        while self.args[i] do
            self.args[i]:getarray(narg)
            narg = narg + 1
            i = i + 1
        end
    end

    pre_call_hook(self)

    local out = string.find(self.mod, "neko_tolua_outside")
    -- call function
    if class and self.name == 'delete' then
        output('  Mneko_tolua_delete(self);')
    elseif class and self.name == 'operator&[]' then
        output('  self->operator[](', self.args[1].name, ') = ', self.args[2].name, ';\n')
    else
        output('  {')
        if self.type ~= '' and self.type ~= 'void' then
            output('  ', self.mod, self.type, self.ptr, 'neko_tolua_ret = ')
            output('(', self.mod, self.type, self.ptr, ') ')
        else
            output('  ')
        end
        if class and self.name == 'new' then
            output('Mneko_tolua_new((', self.type, ')(')
        elseif class and static then
            if out then
                output(self.name, '(')
            else
                output(class .. '::' .. self.name, '(')
            end
        elseif class then
            if out then
                output(self.name, '(')
            else
                if self.cast_operator then
                    -- output('static_cast<',self.mod,self.type,self.ptr,' >(*self')
                    output('self->operator ', self.mod, self.type, '(')
                else
                    output('self->' .. self.name, '(')
                end
            end
        else
            output(self.name, '(')
        end

        if out and not static then
            output('self')
            if self.args[1] and self.args[1].name ~= '' then
                output(',')
            end
        end
        -- write parameters
        local i = 1
        while self.args[i] do
            self.args[i]:passpar()
            i = i + 1
            if self.args[i] then
                output(',')
            end
        end

        if class and self.name == 'new' then
            output('));') -- close Mneko_tolua_new(
        else
            output(');')
        end

        -- return values
        if self.type ~= '' and self.type ~= 'void' then
            nret = nret + 1
            local t, ct = isbasic(self.type)
            if t and self.name ~= "new" then
                if self.cast_operator and _basic_raw_push[t] then
                    output('   ', _basic_raw_push[t], '(L,(', ct, ')neko_tolua_ret);')
                else
                    output('   neko_tolua_push' .. t .. '(L,(', ct, ')neko_tolua_ret);')
                end
            else
                t = self.type
                new_t = string.gsub(t, "const%s+", "")
                local owned = false
                if string.find(self.mod, "neko_tolua_owned") then
                    owned = true
                end
                local push_func = get_push_function(t)
                if self.ptr == '' then
                    output('   {')
                    output('#ifdef __cplusplus\n')
                    output('    void* neko_tolua_obj = Mneko_tolua_new((', new_t, ')(neko_tolua_ret));')
                    output('    ', push_func, '(L,neko_tolua_obj,"', t, '");')
                    output('    neko_tolua_register_gc(L,lua_gettop(L));')
                    output('#else\n')
                    output('    void* neko_tolua_obj = neko_tolua_copy(L,(void*)&neko_tolua_ret,sizeof(', t, '));')
                    output('    ', push_func, '(L,neko_tolua_obj,"', t, '");')
                    output('    neko_tolua_register_gc(L,lua_gettop(L));')
                    output('#endif\n')
                    output('   }')
                elseif self.ptr == '&' then
                    output('   ', push_func, '(L,(void*)&neko_tolua_ret,"', t, '");')
                else
                    output('   ', push_func, '(L,(void*)neko_tolua_ret,"', t, '");')
                    if owned or local_constructor then
                        output('    neko_tolua_register_gc(L,lua_gettop(L));')
                    end
                end
            end
        end
        local i = 1
        while self.args[i] do
            nret = nret + self.args[i]:retvalue()
            i = i + 1
        end
        output('  }')

        -- set array element values
        if class then
            narg = 2
        else
            narg = 1
        end
        if self.args[1].type ~= 'void' then
            local i = 1
            while self.args[i] do
                self.args[i]:setarray(narg)
                narg = narg + 1
                i = i + 1
            end
        end

        -- free dynamically allocated array
        if self.args[1].type ~= 'void' then
            local i = 1
            while self.args[i] do
                self.args[i]:freearray()
                i = i + 1
            end
        end
    end

    post_call_hook(self)

    output(' }')
    output(' return ' .. nret .. ';')

    -- call overloaded function or generate error
    if overload < 0 then

        output('#ifndef TOLUA_RELEASE\n')
        output('neko_tolua_lerror:\n')
        output(' neko_tolua_error(L,"' .. output_error_hook("#ferror in function \'%s\'.", self.lname) .. '",&neko_tolua_err);')
        output(' return 0;')
        output('#endif\n')
    else
        local _local = ""
        if local_constructor then
            _local = "_local"
        end
        output('neko_tolua_lerror:\n')
        output(' return ' .. strsub(self.cname, 1, -3) .. format("%02d", overload) .. _local .. '(L);')
    end
    output('}')
    output('#endif //#ifndef TOLUA_DISABLE\n')
    output('\n')

    -- recursive call to write local constructor
    if class and self.name == 'new' and not local_constructor then

        self:supcode(1)
    end

end

-- register function
function classFunction:register(pre)

    if not self:check_public_access() then
        return
    end

    if self.name == 'new' and self.parent.flags.pure_virtual then
        -- no constructor for classes with pure virtual methods
        return
    end

    output(pre .. 'neko_tolua_function(L,"' .. self.lname .. '",' .. self.cname .. ');')
    if self.name == 'new' then
        output(pre .. 'neko_tolua_function(L,"new_local",' .. self.cname .. '_local);')
        output(pre .. 'neko_tolua_function(L,".call",' .. self.cname .. '_local);')
        -- output(' neko_tolua_set_call_event(L,'..self.cname..'_local, "'..self.parent.type..'");')
    end
end

-- Print method
function classFunction:print(ident, close)
    print(ident .. "Function{")
    print(ident .. " mod  = '" .. self.mod .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. " ptr  = '" .. self.ptr .. "',")
    print(ident .. " name = '" .. self.name .. "',")
    print(ident .. " lname = '" .. self.lname .. "',")
    print(ident .. " const = '" .. self.const .. "',")
    print(ident .. " cname = '" .. self.cname .. "',")
    print(ident .. " lname = '" .. self.lname .. "',")
    print(ident .. " args = {")
    local i = 1
    while self.args[i] do
        self.args[i]:print(ident .. "  ", ",")
        i = i + 1
    end
    print(ident .. " }")
    print(ident .. "}" .. close)
end

-- check if it returns an object by value
function classFunction:requirecollection(t)
    local r = false
    if self.type ~= '' and not isbasic(self.type) and self.ptr == '' then
        local type = gsub(self.type, "%s*const%s+", "")
        t[type] = "neko_tolua_collect_" .. clean_template(type)
        r = true
    end
    local i = 1
    while self.args[i] do
        r = self.args[i]:requirecollection(t) or r
        i = i + 1
    end
    return r
end

-- determine lua function name overload
function classFunction:overload()
    return self.parent:overload(self.lname)
end

function param_object(par) -- returns true if the parameter has an object as its default value

    if not string.find(par, '=') then
        return false
    end -- it has no default value

    local _, _, def = string.find(par, "=(.*)$")

    if string.find(par, "|") then -- a list of flags

        return true
    end

    if string.find(par, "%*") then -- it's a pointer with a default value

        if string.find(par, '=%s*new') or string.find(par, "%(") then -- it's a pointer with an instance as default parameter.. is that valid?
            return true
        end
        return false -- default value is 'NULL' or something
    end

    if string.find(par, "[%(&]") then
        return true
    end -- default value is a constructor call (most likely for a const reference)

    -- if string.find(par, "&") then

    --	if string.find(def, ":") or string.find(def, "^%s*new%s+") then

    --		-- it's a reference with default to something like Class::member, or 'new Class'
    --		return true
    --	end
    -- end

    return false -- ?
end

function strip_last_arg(all_args, last_arg) -- strips the default value from the last argument

    local _, _, s_arg = string.find(last_arg, "^([^=]+)")
    last_arg = string.gsub(last_arg, "([%%%(%)])", "%%%1");
    all_args = string.gsub(all_args, "%s*,%s*" .. last_arg .. "%s*%)%s*$", ")")
    return all_args, s_arg
end

-- Internal constructor
function _Function(t)
    setmetatable(t, classFunction)

    if t.const ~= 'const' and t.const ~= '' then
        error("#invalid 'const' specification")
    end

    append(t)
    if t:inclass() then
        -- print ('t.name is '..t.name..', parent.name is '..t.parent.name)
        if string.gsub(t.name, "%b<>", "") == string.gsub(t.parent.original_name or t.parent.name, "%b<>", "") then
            t.name = 'new'
            t.lname = 'new'
            t.parent._new = true
            t.type = t.parent.name
            t.ptr = '*'
        elseif string.gsub(t.name, "%b<>", "") == '~' ..
            string.gsub(t.parent.original_name or t.parent.name, "%b<>", "") then
            t.name = 'delete'
            t.lname = 'delete'
            t.parent._delete = true
        end
    end
    t.cname = t:cfuncname("tolua") .. t:overload(t)
    return t
end

-- Constructor
-- Expects three strings: one representing the function declaration,
-- another representing the argument list, and the third representing
-- the "const" or empty string.
function Function(d, a, c)
    -- local t = split(strsub(a,2,-2),',') -- eliminate braces
    -- local t = split_params(strsub(a,2,-2))

    if string.find(a, "%.%.%.%s*%)") then
        warning("Functions with variable arguments (`...') are not supported. Ignoring " .. d .. a .. c)
        return nil
    end

    local i = 1
    local l = {
        n = 0
    }

    a = string.gsub(a, "%s*([%(%)])%s*", "%1")
    local t, strip, last = strip_pars(strsub(a, 2, -2));
    if strip then
        -- local ns = string.sub(strsub(a,1,-2), 1, -(string.len(last)+1))
        local ns = join(t, ",", 1, last - 1)

        ns = "(" .. string.gsub(ns, "%s*,%s*$", "") .. ')'
        -- ns = strip_defaults(ns)

        local f = Function(d, ns, c)
        for i = 1, last do
            t[i] = string.gsub(t[i], "=.*$", "")
        end
    end

    while t[i] do
        l.n = l.n + 1
        l[l.n] = Declaration(t[i], 'var', true)
        i = i + 1
    end
    local f = Declaration(d, 'func')
    f.args = l
    f.const = c
    return _Function(f)
end

function join(t, sep, first, last)

    first = first or 1
    last = last or #t
    local lsep = ""
    local ret = ""
    local loop = false
    for i = first, last do

        ret = ret .. lsep .. t[i]
        lsep = sep
        loop = true
    end
    if not loop then
        return ""
    end

    return ret
end

function strip_pars(s)

    local t = split_c_tokens(s, ',')
    local strip = false
    local last

    for i = t.n, 1, -1 do

        if not strip and param_object(t[i]) then
            last = i
            strip = true
        end
        -- if strip then
        --	t[i] = string.gsub(t[i], "=.*$", "")
        -- end
    end

    return t, strip, last

end

function strip_defaults(s)

    s = string.gsub(s, "^%(", "")
    s = string.gsub(s, "%)$", "")

    local t = split_c_tokens(s, ",")
    local sep, ret = "", ""
    for i = 1, t.n do
        t[i] = string.gsub(t[i], "=.*$", "")
        ret = ret .. sep .. t[i]
        sep = ","
    end

    return "(" .. ret .. ")"
end

-- Operator class
-- Represents an operator function or a class operator method.
-- It stores the same fields as functions do plus:
--  kind = set of character representing the operator (as it appers in C++ code)
classOperator = {
    kind = ''
}
classOperator.__index = classOperator
setmetatable(classOperator, classFunction)

-- table to transform operator kind into the appropriate tag method name
_TM = {
    ['+'] = 'add',
    ['-'] = 'sub',
    ['*'] = 'mul',
    ['/'] = 'div',
    ['<'] = 'lt',
    ['<='] = 'le',
    ['=='] = 'eq',
    ['[]'] = 'geti',
    ['&[]'] = 'seti'
    -- ['->'] = 'flechita',
}

-- Print method
function classOperator:print(ident, close)
    print(ident .. "Operator{")
    print(ident .. " kind  = '" .. self.kind .. "',")
    print(ident .. " mod  = '" .. self.mod .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. " ptr  = '" .. self.ptr .. "',")
    print(ident .. " name = '" .. self.name .. "',")
    print(ident .. " const = '" .. self.const .. "',")
    print(ident .. " cname = '" .. self.cname .. "',")
    print(ident .. " lname = '" .. self.lname .. "',")
    print(ident .. " args = {")
    local i = 1
    while self.args[i] do
        self.args[i]:print(ident .. "  ", ",")
        i = i + 1
    end
    print(ident .. " }")
    print(ident .. "}" .. close)
end

function classOperator:supcode_tmp()

    if not _TM[self.kind] then
        return classFunction.supcode(self)
    end

    -- no overload, no parameters, always inclass
    output("/* method:", self.name, " of class ", self:inclass(), " */")

    output("#ifndef TOLUA_DISABLE_" .. self.cname)
    output("\nstatic int", self.cname, "(lua_State* L)")

    if overload < 0 then
        output('#ifndef TOLUA_RELEASE\n')
    end
    output(' neko_tolua_Error neko_tolua_err;')
    output(' if (\n')
    -- check self
    local is_func = get_is_function(self.parent.type)
    output('     !' .. is_func .. '(L,1,"' .. self.parent.type .. '",0,&neko_tolua_err) ||\n')
    output('     !neko_tolua_isnoobj(L,2,&neko_tolua_err)\n )')
    output('  goto neko_tolua_lerror;')

    output(' else\n')
    output('#endif\n') -- neko_tolua_release
    output(' {')

    -- declare self
    output(' ', self.const, self.parent.type, '*', 'self = ')
    output('(', self.const, self.parent.type, '*) ')
    local to_func = get_to_func(self.parent.type)
    output(to_func, '(L,1,0);')

    -- check self
    output('#ifndef TOLUA_RELEASE\n')
    output('  if (!self) neko_tolua_error(L,"' .. output_error_hook("invalid \'self\' in function \'%s\'", self.name) ..
               '",NULL);');
    output('#endif\n')

    -- cast self
    output('  ', self.mod, self.type, self.ptr, 'neko_tolua_ret = ')
    output('(', self.mod, self.type, self.ptr, ')(*self);')

    -- return value
    local t, ct = isbasic(self.type)
    if t then
        output('   neko_tolua_push' .. t .. '(L,(', ct, ')neko_tolua_ret);')
    else
        t = self.type
        local push_func = get_push_function(t)
        new_t = string.gsub(t, "const%s+", "")
        if self.ptr == '' then
            output('   {')
            output('#ifdef __cplusplus\n')
            output('    void* neko_tolua_obj = Mneko_tolua_new((', new_t, ')(neko_tolua_ret));')
            output('    ', push_func, '(L,neko_tolua_obj,"', t, '");')
            output('    neko_tolua_register_gc(L,lua_gettop(L));')
            output('#else\n')
            output('    void* neko_tolua_obj = neko_tolua_copy(L,(void*)&neko_tolua_ret,sizeof(', t, '));')
            output('    ', push_func, '(L,neko_tolua_obj,"', t, '");')
            output('    neko_tolua_register_gc(L,lua_gettop(L));')
            output('#endif\n')
            output('   }')
        elseif self.ptr == '&' then
            output('   ', push_func, '(L,(void*)&neko_tolua_ret,"', t, '");')
        else
            if local_constructor then
                output('   ', push_func, '(L,(void *)neko_tolua_ret,"', t, '");')
                output('    neko_tolua_register_gc(L,lua_gettop(L));')
            else
                output('   ', push_func, '(L,(void*)neko_tolua_ret,"', t, '");')
            end
        end
    end

    output('  }')
    output(' return 1;')

    output('#ifndef TOLUA_RELEASE\n')
    output('neko_tolua_lerror:\n')
    output(' neko_tolua_error(L,"' .. output_error_hook("#ferror in function \'%s\'.", self.lname) .. '",&neko_tolua_err);')
    output(' return 0;')
    output('#endif\n')

    output('}')
    output('#endif //#ifndef TOLUA_DISABLE\n')
    output('\n')
end

-- Internal constructor
function _Operator(t)
    setmetatable(t, classOperator)

    if t.const ~= 'const' and t.const ~= '' then
        error("#invalid 'const' specification")
    end

    append(t)
    if not t:inclass() then
        error("#operator can only be defined as class member")
    end

    -- t.name = t.name .. "_" .. (_TM[t.kind] or t.kind)
    t.cname = t:cfuncname("tolua") .. t:overload(t)
    t.name = "operator" .. t.kind -- set appropriate calling name
    return t
end

-- Constructor
function Operator(d, k, a, c)

    local op_k = string.gsub(k, "^%s*", "")
    op_k = string.gsub(k, "%s*$", "")
    -- if string.find(k, "^[%w_:%d<>%*%&]+$") then
    if d == "operator" and k ~= '' then

        d = k .. " operator"
    elseif not _TM[op_k] then
        warning("No support for operator " .. op_k .. ", ignoring")
        return nil
    end

    local ref = ''
    local t = split_c_tokens(strsub(a, 2, strlen(a) - 1), ',') -- eliminate braces
    local i = 1
    local l = {
        n = 0
    }
    while t[i] do
        l.n = l.n + 1
        l[l.n] = Declaration(t[i], 'var')
        i = i + 1
    end
    if k == '[]' then
        local _
        _, _, ref = strfind(d, '(&)')
        d = gsub(d, '&', '')
    elseif k == '&[]' then
        l.n = l.n + 1
        l[l.n] = Declaration(d, 'var')
        l[l.n].name = 'neko_tolua_value'
    end
    local f = Declaration(d, 'func')
    if k == '[]' and (l[1] == nil or isbasic(l[1].type) ~= 'number') then
        error('operator[] can only be defined for numeric index.')
    end
    f.args = l
    f.const = c
    f.kind = op_k
    f.lname = "." .. (_TM[f.kind] or f.kind)
    if not _TM[f.kind] then
        f.cast_operator = true
    end
    if f.kind == '[]' and ref == '&' and f.const ~= 'const' then
        Operator(d, '&' .. k, a, c) -- create correspoding set operator
    end
    return _Operator(f)
end

_global_templates = {}

classTemplateClass = {

    name = '',
    body = '',
    parents = {},
    args = {} -- the template arguments
}

classTemplateClass.__index = classTemplateClass

function classTemplateClass:throw(types, local_scope)

    -- if table.getn(types) ~= table.getn(self.args) then
    --	error("#invalid parameter count")
    -- end

    -- replace
    for i = 1, types.n do

        local Il = split_c_tokens(types[i], " ")
        if #Il ~= #self.args then
            error("#invalid parameter count for " .. types[i])
        end
        local bI = self.body
        local pI = {}
        for j = 1, self.args.n do
            -- Tl[j] = findtype(Tl[j]) or Tl[j]
            bI = string.gsub(bI, "([^_%w])" .. self.args[j] .. "([^_%w])", "%1" .. Il[j] .. "%2")
            if self.parents then
                for i = 1, #self.parents do
                    pI[i] = string.gsub(self.parents[i], "([^_%w]?)" .. self.args[j] .. "([^_%w]?)",
                        "%1" .. Il[j] .. "%2")
                end
            end
        end
        -- local append = "<"..string.gsub(types[i], "%s+", ",")..">"
        local append = "<" .. concat(Il, 1, #Il, ",") .. ">"
        append = string.gsub(append, "%s*,%s*", ",")
        append = string.gsub(append, ">>", "> >")
        for i = 1, #pI do
            -- pI[i] = string.gsub(pI[i], ">>", "> >")
            pI[i] = resolve_template_types(pI[i])
        end
        bI = string.gsub(bI, ">>", "> >")
        local n = self.name
        if local_scope then
            n = self.local_name
        end

        Class(n .. append, pI, bI)
    end
end

function TemplateClass(name, parents, body, parameters)

    local o = {

        parents = parents,
        body = body,
        args = parameters
    }

    local oname = string.gsub(name, "@.*$", "")
    oname = getnamespace(classContainer.curr) .. oname
    o.name = oname

    o.local_name = name

    setmetatable(o, classTemplateClass)

    if _global_templates[oname] then
        warning("Duplicate declaration of template " .. oname)
    else
        _global_templates[oname] = o
    end

    return o
end

-- Class class
-- Represents a class definition.
-- Stores the following fields:
--    name = class name
--    base = class base, if any (only single inheritance is supported)
--    {i}  = list of members
classClass = {
    classtype = 'class',
    name = '',
    base = '',
    type = '',
    btype = '',
    ctype = ''
}
classClass.__index = classClass
setmetatable(classClass, classContainer)

-- register class
function classClass:register(pre)

    if not self:check_public_access() then
        return
    end

    pre = pre or ''
    push(self)
    if _collect[self.type] then
        output(pre, '#ifdef __cplusplus\n')
        output(pre .. 'neko_tolua_cclass(L,"' .. self.lname .. '","' .. self.type .. '","' .. self.btype .. '",' ..
                   _collect[self.type] .. ');')
        output(pre, '#else\n')
        output(pre .. 'neko_tolua_cclass(L,"' .. self.lname .. '","' .. self.type .. '","' .. self.btype .. '",NULL);')
        output(pre, '#endif\n')
    else
        output(pre .. 'neko_tolua_cclass(L,"' .. self.lname .. '","' .. self.type .. '","' .. self.btype .. '",NULL);')
    end
    if self.extra_bases then
        for k, base in ipairs(self.extra_bases) do
            -- not now
            -- output(pre..' neko_tolua_addbase(L, "'..self.type..'", "'..base..'");')
        end
    end
    output(pre .. 'neko_tolua_beginmodule(L,"' .. self.lname .. '");')
    local i = 1
    while self[i] do
        self[i]:register(pre .. ' ')
        i = i + 1
    end
    output(pre .. 'neko_tolua_endmodule(L);')
    pop()
end

-- return collection requirement
function classClass:requirecollection(t)
    if self.flags.protected_destructor or (not self:check_public_access()) then
        return false
    end
    push(self)
    local r = false
    local i = 1
    while self[i] do
        r = self[i]:requirecollection(t) or r
        i = i + 1
    end
    pop()
    -- only class that exports destructor can be appropriately collected
    -- classes that export constructors need to have a collector (overrided by -D flag on command line)
    if self._delete or ((not neko_tolua_flags['D']) and self._new) then
        -- t[self.type] = "neko_tolua_collect_" .. gsub(self.type,"::","_")
        t[self.type] = "neko_tolua_collect_" .. clean_template(self.type)
        r = true
    end
    return r
end

-- output tags
function classClass:decltype()
    push(self)
    self.type = regtype(self.original_name or self.name)
    self.btype = typevar(self.base)
    self.ctype = 'const ' .. self.type
    if self.extra_bases then
        for i = 1, #self.extra_bases do
            self.extra_bases[i] = typevar(self.extra_bases[i])
        end
    end
    local i = 1
    while self[i] do
        self[i]:decltype()
        i = i + 1
    end
    pop()
end

-- Print method
function classClass:print(ident, close)
    print(ident .. "Class{")
    print(ident .. " name = '" .. self.name .. "',")
    print(ident .. " base = '" .. self.base .. "';")
    print(ident .. " lname = '" .. self.lname .. "',")
    print(ident .. " type = '" .. self.type .. "',")
    print(ident .. " btype = '" .. self.btype .. "',")
    print(ident .. " ctype = '" .. self.ctype .. "',")
    local i = 1
    while self[i] do
        self[i]:print(ident .. " ", ",")
        i = i + 1
    end
    print(ident .. "}" .. close)
end

function classClass:set_protected_destructor(p)
    self.flags.protected_destructor = self.flags.protected_destructor or p
end

-- Internal constructor
function _Class(t)
    setmetatable(t, classClass)
    t:buildnames()
    append(t)
    return t
end

-- Constructor
-- Expects the name, the base (array) and the body of the class.
function Class(n, p, b)

    if #p > 1 then
        b = string.sub(b, 1, -2)
        for i = 2, #p, 1 do
            b = b .. "\n neko_tolua_inherits " .. p[i] .. " __" .. p[i] .. "__;\n"
        end
        b = b .. "\n}"
    end

    -- check for template
    b = string.gsub(b, "^{%s*TEMPLATE_BIND", "{\nTOLUA_TEMPLATE_BIND")
    local t, _, T, I = string.find(b, '^{%s*TOLUA_TEMPLATE_BIND%s*%(+%s*\"?([^\",]*)\"?%s*,%s*([^%)]*)%s*%)+')
    if t then

        -- remove quotes
        I = string.gsub(I, "\"", "")
        T = string.gsub(T, "\"", "")
        -- get type list
        local types = split_c_tokens(I, ",")
        -- remove TEMPLATE_BIND line
        local bs = string.gsub(b, "^{%s*TOLUA_TEMPLATE_BIND[^\n]*\n", "{\n")

        local Tl = split(T, " ")
        local tc = TemplateClass(n, p, bs, Tl)

        tc:throw(types, true)
        -- for i=1,types.n do
        --	tc:throw(split_c_tokens(types[i], " "), true)
        -- end
        return
    end

    local mbase

    if p then
        mbase = table.remove(p, 1)
        if not p[1] then
            p = nil
        end
    end

    mbase = mbase and resolve_template_types(mbase)

    local c
    local oname = string.gsub(n, "@.*$", "")
    oname = getnamespace(classContainer.curr) .. oname

    if _global_classes[oname] then
        c = _global_classes[oname]
        if mbase and ((not c.base) or c.base == "") then
            c.base = mbase
        end
    else
        c = _Class(_Container {
            name = n,
            base = mbase,
            extra_bases = p
        })

        local ft = getnamespace(c.parent) .. c.original_name
        append_global_type(ft, c)
    end

    push(c)
    c:parse(strsub(b, 2, strlen(b) - 1)) -- eliminate braces
    pop()
end

-- custom

local custom_src = [[

function extract_code(fn, s)
    local code = ""
    if fn then
        code = '\n$#include "' .. fn .. '"\n'
    end
    s = "\n" .. s .. "\n" -- add blank lines as sentinels
    local _, e, c, t = strfind(s, "\n([^\n]-)SCRIPT_([%w_]*)[^\n]*\n")
    while e do
        t = strlower(t)
        if t == "bind_begin" then
            _, e, c = strfind(s, "(.-)\n[^\n]*SCRIPT_BIND_END[^\n]*\n", e)
            if not e then
                neko_tolua_error("Unbalanced 'SCRIPT_BIND_BEGIN' directive in header file")
            end
        end
        if t == "bind_class" or t == "bind_block" then
            local b
            _, e, c, b = string.find(s, "([^{]-)(%b{})", e)
            c = c .. '{\n' .. extract_code(nil, b) .. '\n};\n'
        end
        code = code .. c .. "\n"
        _, e, c, t = strfind(s, "\n([^\n]-)SCRIPT_([%w_]*)[^\n]*\n", e)
    end
    return code
end

function preprocess_hook(p)
end

function preparse_hook(p)
end

function include_file_hook(p, filename)
    do
        return
    end
    -- print("FILENAME is "..filename)
    p.code = string.gsub(p.code, "\n%s*SigC::Signal", "\n\tneko_tolua_readonly SigC::Signal")
    p.code = string.gsub(p.code, "#ifdef __cplusplus\nextern \"C\" {\n#endif", "")
    p.code = string.gsub(p.code, "#ifdef __cplusplus\n};?\n#endif", "")
    p.code = string.gsub(p.code, "DECLSPEC", "")
    p.code = string.gsub(p.code, "SDLCALL", "")
    p.code = string.gsub(p.code, "DLLINTERFACE", "")
    p.code = string.gsub(p.code, "#define[^\n]*_[hH]_?%s*\n", "\n")
    -- print("code is "..p.code)
end


]]

-- doit

function parse_extra()

    for k, v in ipairs(_extra_parameters or {}) do

        local b, e, name, value = string.find(v, "^([^=]*)=(.*)$")
        if b then
            _extra_parameters[name] = value
        else
            _extra_parameters[v] = true
        end
    end
end

function doit()
    -- define package name, if not provided
    if not neko_tolua_flags.name then
        if neko_tolua_flags.f then
            neko_tolua_flags.name = string.gsub(neko_tolua_flags.f, "%..*$", "")
            _, _, neko_tolua_flags.name = string.find(neko_tolua_flags.name, "([^/\\]*)$")
        else
            error("#no package name nor input file provided")
        end
    end

    -- parse table with extra paramters
    parse_extra()

    -- 设置包名称后执行此操作
    if neko_tolua_flags['L'] then
        dofile(neko_tolua_flags['L'])
    end

    -- add cppstring
    _basic['string'] = 'cppstring'
    _basic['std::string'] = 'cppstring'
    _basic_ctype.cppstring = 'const char*'

    -- proccess package
    local p = Package(neko_tolua_flags.name, neko_tolua_flags.f)

    if neko_tolua_flags.o then
        local st, msg = writeto(neko_tolua_flags.o)
        if not st then
            error('#' .. msg)
        end
    end

    p:decltype()
    if neko_tolua_flags.P then
        p:print()
    else
        push(p)
        pre_output_hook(p)
        pop()
        p:preamble()
        p:supcode()
        push(p)
        pre_register_hook(p)
        pop()
        p:register()
        push(p)
        post_output_hook(p)
        pop()
    end

    if neko_tolua_flags.o then
        writeto()
    end

    -- write header file
    -- if not neko_tolua_flags.P then
    --     if neko_tolua_flags.H then
    --         local st, msg = writeto(neko_tolua_flags.H)
    --         if not st then
    --             error('#' .. msg)
    --         end
    --         p:header()
    --         writeto()
    --     end
    -- end
end

