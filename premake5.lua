-- premake5.lua
require "scripts/export-compile-commands"
require "scripts/ecc/ecc"
-- require "scripts/cmake/_cmake"

workspace "neko"
configurations {"Debug", "Release"}

buildoptions {"/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj", "/Zc:preprocessor", "/Zc:wchar_t", "/Zc:forScope",
              "/MP"}
disablewarnings {"4005", "4244", "4305", "4127", "4481", "4100", "4512", "4018", "4099"}

cppdialect "C++20"
cdialect "C17"

local FMOD_LIB_DIR = "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows"
-- local arch = "x86"
local arch = "x86_64"

characterset("Unicode")

location "vsbuild"

flags {"MultiProcessorCompile"}
staticruntime "off"

defines {"WIN32", "_WIN32", "_WINDOWS", "NOMINMAX", "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS",
         "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING", "WIN32_LEAN_AND_MEAN", "_SCL_SECURE_NO_WARNINGS",
         "_CRT_NONSTDC_NO_DEPRECATE"}

defines {"LUA_USE_LONGJMP", "NEKO_CFFI", "NEKO_BOX2D", "NEKO_AUDIO=2"}

defines {"UNICODE", "_UNICODE"}

defines {"GLFW_INCLUDE_NONE"}

includedirs {"source", "source/extern", "source/extern/luaot"}

includedirs {FMOD_LIB_DIR .. "/api/core/inc", FMOD_LIB_DIR .. "/api/studio/inc", "source/extern/glfw/include"}

libdirs {"source/extern/libffi/lib", FMOD_LIB_DIR .. "/api/core/lib/x64", FMOD_LIB_DIR .. "/api/studio/lib/x64",
         "source/extern/glfw/lib-vc2022"}

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
end

filter "configurations:Release"
do
    defines {"_NDEBUG", "NDEBUG"}
    optimize "On"
    architecture(arch)

    defines {"_WIN64"}
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
    kind "ConsoleApp"
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

    links {"fmod_vc", "fmodstudio_vc", "ws2_32", "wininet", "glfw3"}

    links {"base"}

    files {"premake5.lua"}

    -- warnings "off"
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

    -- warnings "off"

end
