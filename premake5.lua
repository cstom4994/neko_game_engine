-- premake5.lua
-- require "scripts/export-compile-commands"
-- require "scripts/ecc/ecc"
-- require "scripts/cmake/_cmake"
workspace "neko"
configurations {"Debug", "Debug_Profiler", "Release"}

buildoptions {"/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj", "/Zc:preprocessor", "/Zc:wchar_t", "/Zc:forScope",
              "/MP"}
disablewarnings {"4005", "4244", "4305", "4127", "4481", "4100", "4512", "4018", "4099"}

cppdialect "C++20"
cdialect "C17"

local arch = "x86_64"

characterset("Unicode")

location "vsbuild"

flags {"MultiProcessorCompile"}
staticruntime "off"

defines {"WIN32", "_WIN32", "_WINDOWS", "NOMINMAX", "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS",
         "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING", "WIN32_LEAN_AND_MEAN", "_SCL_SECURE_NO_WARNINGS",
         "_CRT_NONSTDC_NO_DEPRECATE"}

defines {"LUA_USE_LONGJMP", "NEKO_CFFI", "NEKO_BOX2D"}

defines {"UNICODE", "_UNICODE"}

defines {"GLFW_INCLUDE_NONE"}

includedirs {"source", "source/extern", "source/extern/luaot", "source/extern/glfw/include"}

libdirs {"source/extern/glfw/lib-vc2022"}

local function runlua(name)
    return function(config)
        assert(config.input, "Missing 'input' in config")
        assert(config.output, "Missing 'output' in config")

        local lua_cmd = {config.exe or "lua", config.script}

        if config.args then
            for _, arg in ipairs(config.args) do
                if type(arg) == "table" then
                    for _, flag in ipairs(arg) do -- 展开嵌套 table
                        table.insert(lua_cmd, flag)
                    end
                else
                    table.insert(lua_cmd, arg)
                end
            end
        end

        for i, v in ipairs(lua_cmd) do
            lua_cmd[i] = tostring(v):gsub("%$out", config.output):gsub("%$in", config.input)
        end

        local cmd = table.concat(lua_cmd, " ")
        print(name, table.unpack(lua_cmd))
        local success, exit_type, code = os.execute(cmd)
        if not success or exit_type ~= "exit" or code ~= 0 then
            error(string.format("Error running %s: %s", name, cmd))
        end
    end
end

local base64 = {}

local extract = _G.bit32 and _G.bit32.extract -- Lua 5.2/Lua 5.3 in compatibility mode
if not extract then
    if _G.bit then -- LuaJIT
        local shl, shr, band = _G.bit.lshift, _G.bit.rshift, _G.bit.band
        extract = function(v, from, width)
            return band(shr(v, from), shl(1, width) - 1)
        end
    elseif _G._VERSION == "Lua 5.1" then
        extract = function(v, from, width)
            local w = 0
            local flag = 2 ^ from
            for i = 0, width - 1 do
                local flag2 = flag + flag
                if v % flag2 >= flag then
                    w = w + 2 ^ i
                end
                flag = flag2
            end
            return w
        end
    else -- Lua 5.3+
        extract = load [[return function( v, from, width )
			return ( v >> from ) & ((1 << width) - 1)
		end]]()
    end
end

function base64.makeencoder(s62, s63, spad)
    local encoder = {}
    for b64code, char in pairs {
        [0] = 'A',
        'B',
        'C',
        'D',
        'E',
        'F',
        'G',
        'H',
        'I',
        'J',
        'K',
        'L',
        'M',
        'N',
        'O',
        'P',
        'Q',
        'R',
        'S',
        'T',
        'U',
        'V',
        'W',
        'X',
        'Y',
        'Z',
        'a',
        'b',
        'c',
        'd',
        'e',
        'f',
        'g',
        'h',
        'i',
        'j',
        'k',
        'l',
        'm',
        'n',
        'o',
        'p',
        'q',
        'r',
        's',
        't',
        'u',
        'v',
        'w',
        'x',
        'y',
        'z',
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        s62 or '+',
        s63 or '/',
        spad or '='
    } do
        encoder[b64code] = char:byte()
    end
    return encoder
end

function base64.makedecoder(s62, s63, spad)
    local decoder = {}
    for b64code, charcode in pairs(base64.makeencoder(s62, s63, spad)) do
        decoder[charcode] = b64code
    end
    return decoder
end

function base64.encode(str, encoder, usecaching)
    encoder = encoder or base64.makeencoder()
    local t, k, n = {}, 1, #str
    local lastn = n % 3
    local cache = {}
    for i = 1, n - lastn, 3 do
        local a, b, c = str:byte(i, i + 2)
        local v = a * 0x10000 + b * 0x100 + c
        local s
        if usecaching then
            s = cache[v]
            if not s then
                s = string.char(encoder[extract(v, 18, 6)], encoder[extract(v, 12, 6)], encoder[extract(v, 6, 6)],
                    encoder[extract(v, 0, 6)])
                cache[v] = s
            end
        else
            s = string.char(encoder[extract(v, 18, 6)], encoder[extract(v, 12, 6)], encoder[extract(v, 6, 6)],
                encoder[extract(v, 0, 6)])
        end
        t[k] = s
        k = k + 1
    end
    if lastn == 2 then
        local a, b = str:byte(n - 1, n)
        local v = a * 0x10000 + b * 0x100
        t[k] = string.char(encoder[extract(v, 18, 6)], encoder[extract(v, 12, 6)], encoder[extract(v, 6, 6)],
            encoder[64])
    elseif lastn == 1 then
        local v = str:byte(n) * 0x10000
        t[k] = string.char(encoder[extract(v, 18, 6)], encoder[extract(v, 12, 6)], encoder[64], encoder[64])
    end
    return table.concat(t)
end

function base64.decode(b64, decoder, usecaching)
    decoder = decoder or base64.makedecoder()
    local pattern = '[^%w%+%/%=]'
    if decoder then
        local s62, s63
        for charcode, b64code in pairs(decoder) do
            if b64code == 62 then
                s62 = charcode
            elseif b64code == 63 then
                s63 = charcode
            end
        end
        pattern = ('[^%%w%%%s%%%s%%=]'):format(string.char(s62), string.char(s63))
    end
    b64 = b64:gsub(pattern, '')
    local cache = usecaching and {}
    local t, k = {}, 1
    local n = #b64
    local padding = b64:sub(-2) == '==' and 2 or b64:sub(-1) == '=' and 1 or 0
    for i = 1, padding > 0 and n - 4 or n, 4 do
        local a, b, c, d = b64:byte(i, i + 3)
        local s
        if usecaching then
            local v0 = a * 0x1000000 + b * 0x10000 + c * 0x100 + d
            s = cache[v0]
            if not s then
                local v = decoder[a] * 0x40000 + decoder[b] * 0x1000 + decoder[c] * 0x40 + decoder[d]
                s = string.char(extract(v, 16, 8), extract(v, 8, 8), extract(v, 0, 8))
                cache[v0] = s
            end
        else
            local v = decoder[a] * 0x40000 + decoder[b] * 0x1000 + decoder[c] * 0x40 + decoder[d]
            s = string.char(extract(v, 16, 8), extract(v, 8, 8), extract(v, 0, 8))
        end
        t[k] = s
        k = k + 1
    end
    if padding == 1 then
        local a, b, c = b64:byte(n - 3, n - 1)
        local v = decoder[a] * 0x40000 + decoder[b] * 0x1000 + decoder[c] * 0x40
        t[k] = string.char(extract(v, 16, 8), extract(v, 8, 8))
    elseif padding == 2 then
        local a, b = b64:byte(n - 3, n - 2)
        local v = decoder[a] * 0x40000 + decoder[b] * 0x1000
        t[k] = string.char(extract(v, 16, 8))
    end
    return table.concat(t)
end

local function format_base64(str)
    local max_length = 76 -- RFC 规定每行 76 字符
    local chunks = {}

    for i = 1, #str, max_length do
        table.insert(chunks, str:sub(i, i + max_length - 1))
    end

    local parts = {'"\\\n'}
    for i, chunk in ipairs(chunks) do
        table.insert(parts, chunk)
        if i ~= #chunks then
            table.insert(parts, '\\\n')
        end
    end
    table.insert(parts, '"')

    return table.concat(parts)
end

newaction {
    trigger = 'encode_b64',
    description = 'Embed base64 files',
    execute = function()
        local entries = {}
        -- local luaFiles = os.matchfiles("source/game/script/*.lua")
        -- local luaFiles = os.matchfiles("./*.md")
        local luaFiles = os.matchfiles("./gamedir/**")

        local out = io.open("out.lua", "w")
        out:write("local F = function(x) return x end\n")
        out:write("local M = {\n")

        for _, luaFilePath in ipairs(luaFiles) do

            local luaFile, err = io.open(luaFilePath, "rb")
            local content = luaFile:read("*a")

            local encoded = base64.encode(content)
            local formatted = format_base64(encoded)
            table.insert(entries, ('  ["%s"] = %s'):format("luabp/" .. luaFilePath, formatted))

            luaFile:close()

            print("embed_b64", luaFilePath)
        end

        out:write(table.concat(entries, ",\n"))
        out:write("\n}\nreturn M\n")
        out:close()
    end
}

newaction {
    trigger = 'decode_b64',
    description = 'decode base64 files',
    execute = function()
        local function save_files(files, output_dir)
            for filename, content in pairs(files) do
                local path = output_dir .. '/' .. filename
                os.mkdir(output_dir)
                local file = io.open(path, "wb")
                if file then
                    file:write(base64.decode(content))
                    file:close()
                    print("Saved:", path)
                else
                    error("Failed to write: " .. path)
                end
            end
        end

        local entries = {}
        local luaFiles = dofile("out.lua")

        save_files(luaFiles, "output_directory")
    end
}

newaction {
    trigger = 'embed',
    description = 'Embed lua',
    execute = function()
        local outputDir = "source/gen"
        local luaFiles = os.matchfiles("source/engine/*.lua")
        for _, luaFilePath in ipairs(luaFiles) do
            local file = luaFilePath:match("([^/]+)%.lua$")
            local cppFilePath = outputDir .. "/" .. file .. "_embedded.cpp"

            local luaFile = io.open(luaFilePath, "r")
            local content = luaFile:read("*a")
            luaFile:close()

            local cppFile = io.open(cppFilePath, "w")
            cppFile:write("#include <cstdint>\n\n")
            local varName = file .. "_lua"
            cppFile:write("static const uint8_t " .. varName .. "[] = {")

            for i = 1, #content do
                local byte = string.byte(content, i)
                cppFile:write(string.format("0x%02X", byte))
                if i < #content then
                    cppFile:write(", ")
                end
                if i % 15 == 1 then
                    cppFile:write("\n")
                end
            end
            cppFile:write(", 0x00\n};\n\n")
            cppFile:write("const uint8_t* get_" .. varName .. "() { return " .. varName .. "; }")
            cppFile:close()

            print("embed", luaFilePath, cppFilePath)
        end
    end
}

newaction {
    trigger = 'luaot',
    description = 'Lua AOT',
    execute = function()
        local outputDir = "source/gen"
        local luaFiles = os.matchfiles("source/engine/*.lua")
        for _, luaFilePath in ipairs(luaFiles) do
            local file = luaFilePath:match("([^/]+)%.lua$")
            local cppFilePath = outputDir .. "/" .. file .. "_embedded.cpp"
            local luaot_exe = os.getcwd() .. "/bin/luaot"

            if os.isfile(luaot_exe .. ".exe") then
                runlua("luaot") {
                    exe = luaot_exe,
                    args = {"$in", "-o", "$out"},
                    input = luaFilePath,
                    output = outputDir .. "/" .. file .. "_luaot.c"
                }
            else
                print("no luaot.exe")
            end

        end
    end
}

filter "configurations:Debug"
do
    defines {"_DEBUG", "DEBUG", "_CONSOLE"}
    symbols "On"
    architecture(arch)

    defines {"_WIN64"}

    targetsuffix("_debug")
end

filter "configurations:Debug_Profiler"
do
    defines {"_DEBUG", "DEBUG", "_CONSOLE", "USE_PROFILER"}
    symbols "On"
    architecture(arch)

    defines {"_WIN64"}

    targetsuffix("_debug_profiler")
end

filter "configurations:Release"
do
    defines {"_NDEBUG", "NDEBUG"}
    optimize "On"
    architecture(arch)

    defines {"_WIN64"}

    targetsuffix("_release")
end

filter "configurations:*"
do
    local function vcpkg(prj)
        -- premake.w('<VcpkgTriplet Condition="\'$(Platform)\'==\'x64\'">x64-windows-static</VcpkgTriplet>')
        premake.w('<VcpkgEnabled>false</VcpkgEnabled>')
    end

    require('vstudio')
    local vs = premake.vstudio.vc2010
    premake.override(premake.vstudio.vc2010.elements, "globals", function(base, prj)
        local calls = base(prj)
        table.insertafter(calls, vs.globals, vcpkg)
        return calls
    end)
end

project "base"
do
    kind "StaticLib"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source", "source/extern/luaot", "source/extern/imgui", "source/extern/box2d",
                 "source/extern/libffi/include"}

    files {"source/base/**.cpp", "source/base/**.hpp", "source/base/**.h", "source/base/**.lua"}

    files {"source/extern/imgui/**.cpp", "source/extern/imgui/**.h"}

    files {"source/extern/luaalloc.c", "source/extern/http.c", "source/extern/miniz.c", "source/extern/glad/glad.c",
           "source/extern/ui.cpp", "source/extern/vtf/*.*"}

    local luasocketsrc = {"source/extern/luasocket/auxiliar.c", "source/extern/luasocket/buffer.c",
                          "source/extern/luasocket/compat.c", "source/extern/luasocket/except.c",
                          "source/extern/luasocket/inet.c", "source/extern/luasocket/io.c",
                          "source/extern/luasocket/luasocket.c", "source/extern/luasocket/mime.c",
                          "source/extern/luasocket/options.c", "source/extern/luasocket/select.c",
                          "source/extern/luasocket/tcp.c", "source/extern/luasocket/timeout.c",
                          "source/extern/luasocket/udp.c"}

    table.insert(luasocketsrc, "source/extern/luasocket/wsocket.c")

    files {"source/extern/luasocket/**.h", luasocketsrc}

    files {"source/extern/luaot/**.h"}

    local luaotsrc = {"lapi.c", "lauxlib.c", "lbaselib.c", "lcode.c", "lcorolib.c", "lctype.c", "ldblib.c", "ldebug.c",
                      "ldo.c", "ldump.c", "lfunc.c", "lgc.c", "linit.c", "liolib.c", "llex.c", "lmathlib.c", "lmem.c",
                      "loadlib.c", "lobject.c", "lopcodes.c", "loslib.c", "lparser.c", "lstate.c", "lstring.c",
                      "lstrlib.c", "ltable.c", "ltablib.c", "ltm.c", "lundump.c", "lutf8lib.c", "lvm.c", "lzio.c"}

    for i, v in ipairs(luaotsrc) do
        luaotsrc[i] = "source/extern/luaot/" .. luaotsrc[i]
    end

    files {luaotsrc}

    files {"premake5.lua"}
end

project "engine"
do
    kind "StaticLib"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source/extern/luaot", "source/extern/imgui", "source/extern/box2d", "source/extern/libffi/include"}

    files {"source/engine/**.cpp", "source/engine/**.hpp", "source/engine/**.h", "source/engine/**.lua"}
    -- files {"source/game/**.cpp", "source/game/**.hpp", "source/game/**.h", "source/game/**.lua"}
    files {"source/editor/**.cpp", "source/editor/**.c", "source/editor/**.hpp", "source/editor/**.h",
           "source/editor/**.lua"}

    files {"source/extern/box2d/**.cpp"}

    files {"source/gen/*_embedded.cpp"}
    -- files {"source/gen/*_luaot.c"}

    files {"premake5.lua"}

    -- warnings "off"
end

project "sandbox"
do
    kind "ConsoleApp"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source", "source/extern/luaot"}

    defines {"LUAOT_USE_GOTOS"}

    files {"source/game/*.cpp"}

    files {"premake5.lua"}

    links {"base", "engine"}

    links {"ws2_32", "wininet", "glfw3"}
end

project "luaot"
do
    kind "ConsoleApp"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source", "source/extern/luaot"}

    defines {"LUAOT_USE_GOTOS"}

    files {"source/extern/luaot/luaot.c"}

    files {"premake5.lua"}

    links {"base"}
end

group "Tests"
do
    local function gen_test_proj(name, src)
        project(name)
        do
            kind "ConsoleApp"
            language "C++"
            targetdir "./bin"
            debugdir "./bin"
            includedirs {"source", "source/extern/luaot"}
            defines {"LUAOT_USE_GOTOS"}
            files {src}
            files {"premake5.lua"}
            links {"base", "engine"}
            links {"ws2_32", "wininet", "glfw3"}
        end
    end

    gen_test_proj("test_shader", "source/test/test_shader.cpp")
    gen_test_proj("test_luawrap", "source/test/test_luawrap.cpp")
end
