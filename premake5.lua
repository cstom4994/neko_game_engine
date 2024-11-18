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

-- characterset("MBCS")
characterset("Unicode")

location "vsbuild"

flags {"MultiProcessorCompile"}
staticruntime "off"

defines {"WIN32", "_WIN32", "_WINDOWS", "NOMINMAX", "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS",
         "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING", "WIN32_LEAN_AND_MEAN", "_SCL_SECURE_NO_WARNINGS",
         "_CRT_NONSTDC_NO_DEPRECATE"}

defines {"FFI_LITTLE_ENDIAN", "NEKO_CFFI", "NEKO_BOX2D", "NEKO_AUDIO=2"}

defines {"UNICODE", "_UNICODE"}

defines {"GLFW_INCLUDE_NONE"}

-- includedirs {"source/deps/SDL2-2.30.8/include"}
-- libdirs {"source/deps/SDL2-2.30.8/lib/x64"}

includedirs {"source", "source/vendor",  "source/deps/wamr/include"}

includedirs {FMOD_LIB_DIR .. "/api/core/inc", FMOD_LIB_DIR .. "/api/studio/inc"}

libdirs { "source/deps/wamr/lib", "source/deps/libffi/lib", FMOD_LIB_DIR .. "/api/core/lib/x64",
         FMOD_LIB_DIR .. "/api/studio/lib/x64"}

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

            print("Embedded Lua file: " .. luaFilePath .. " as u8 array in " .. cppFilePath)
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

project "base"
do
    kind "StaticLib"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source", "source/vendor", "source/deps/imgui", "source/deps/box2d", "source/deps/libffi/include"}

    files {"source/base/**.cpp", "source/base/**.hpp", "source/base/**.h", "source/base/**.lua"}

    files {"source/deps/imgui/**.cpp", "source/deps/imgui/**.h"}

    files {"source/vendor/cffi/**.cc"}

    files {"source/vendor/luaalloc.c", "source/vendor/bit.c", "source/vendor/http.c", "source/vendor/miniz.c",
           "source/vendor/glad/glad.c", "source/vendor/ui.cpp", "source/vendor/vtf/*.*"}

    files {"premake5.lua"}
end

project "engine"
do
    kind "ConsoleApp"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source/deps/imgui", "source/deps/box2d", "source/deps/libffi/include"}

    files {"source/engine/**.cpp", "source/engine/**.hpp", "source/engine/**.h", "source/engine/**.lua"}
    -- files {"source/game/**.cpp", "source/game/**.hpp", "source/game/**.h", "source/game/**.lua"}
    files {"source/editor/**.cpp","source/editor/**.c", "source/editor/**.hpp", "source/editor/**.h", "source/editor/**.lua"}

    files {"source/deps/box2d/**.cpp"}

    files {"source/gen/*.cpp"}

    
    links {"vmlib", "iwasm"}

    links {"fmod_vc","fmodstudio_vc", "ffi", "ws2_32", "wininet"}

    links {"base"}

    files {"premake5.lua"}

    -- warnings "off"
end

group "gs"
do
    project "engine2"
    do
        kind "StaticLib"
        language "C++"
        targetdir "./bin"
        debugdir "./bin"

        defines {"NETWORKING", "DONT_USE_VAO"}

        includedirs {"source", "source/engine2"}

        files {"source/game/**.cpp", "source/game/**.hpp", "source/game/**.h", "source/game/**.lua"}

        -- links {"glfw3", "ffi"}

        links {"base"}

        links {"vmlib", "iwasm"}


        links {"base"}

        -- links {"SDL2"}

        links {"ws2_32", "winmm", "user32"}

        links {"opengl32"}

        files {"premake5.lua"}

        -- warnings "off"

    end
end
