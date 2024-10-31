-- premake5.lua

require "export-compile-commands"

workspace "neko"
configurations {"Debug", "Release"}

buildoptions {"/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj", "/Zc:preprocessor", "/Zc:wchar_t", "/Zc:forScope",
              "/MP"}
disablewarnings {"4005", "4244", "4305", "4127", "4481", "4100", "4512", "4018"}

cppdialect "C++20"
cdialect "C17"

local FMOD_LIB_DIR = "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows"
--local arch = "x86"
local arch = "x86_64"

characterset("MBCS")

location "vsbuild"

flags {"MultiProcessorCompile"}
staticruntime "off"

defines {"WIN32", "_WIN32", "_WINDOWS", "NOMINMAX", "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS",
         "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING", "WIN32_LEAN_AND_MEAN", "_SCL_SECURE_NO_WARNINGS",
         "_CRT_NONSTDC_NO_DEPRECATE"}

defines {"FFI_LITTLE_ENDIAN", "NEKO_CFFI", "NEKO_BOX2D", "NEKO_AUDIO=2"}

-- defines {"UNICODE", "_UNICODE"}

defines {"GLFW_INCLUDE_NONE"}

includedirs {"source/deps/SDL2-2.30.8/include"}
libdirs {"source/deps/libffi/lib", FMOD_LIB_DIR .. "/api/core/lib/x64", FMOD_LIB_DIR .. "/api/studio/lib/x64"}
libdirs {"source/deps/SDL2-2.30.8/lib/x64"}

filter "configurations:Debug"
do
    defines {"_DEBUG", "DEBUG", "_CONSOLE"}
    symbols "On"
    architecture (arch)
end

filter "configurations:Release"
do
    defines {"_NDEBUG", "NDEBUG"}
    optimize "On"
    architecture (arch)
end

project "engine"
do
    kind "ConsoleApp"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    includedirs {"source", "source/vendor", "source/deps/imgui", "source/deps/box2d", "source/deps/libffi/include"}

    includedirs {FMOD_LIB_DIR .. "/api/core/inc", FMOD_LIB_DIR .. "/api/studio/inc"}

    files {"source/vendor/cffi/**.cc"}

    files {"source/vendor/luaalloc.c", "source/vendor/bit.c", "source/vendor/http.c", "source/vendor/miniz.c",
           "source/vendor/glad/glad.c", "source/vendor/ui.cpp"}
    files {"source/engine/**.cpp", "source/engine/**.hpp", "source/engine/**.h", "source/engine/**.lua"}
    files {"source/game/**.cpp", "source/game/**.hpp", "source/game/**.h", "source/game/**.lua"}
    files {"source/editor/**.cpp", "source/editor/**.hpp", "source/editor/**.h", "source/editor/**.lua"}

    files {"source/deps/box2d/**.cpp"}

    links {"fmod_vc", "fmodstudio_vc", "ffi", "ws2_32", "wininet"}

    files {"premake5.lua"}

    -- warnings "off"
end

project "client"
do
    kind "SharedLib"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    defines {"CLIENT_EXPORTS", "CLIENT", "_64BUILD"}

    includedirs {"source/engine2/common", "source/engine2/shared"}

    includedirs {"source/engine2/clientdll/../gameui", "source/engine2/clientdll", "source/engine2/shared/renderer"}

    files {"source/engine2/clientdll/**.cpp", "source/engine2/clientdll/**.h"}

    files {"source/engine2/shared/**.cpp", "source/engine2/shared/**.h", "source/engine2/shared/**.hpp",
           "source/engine2/common/**.cpp", "source/engine2/common/**.h", "source/engine2/common/**.hpp"}

    defines {"GAMEDLL_EXPORTS", "GAMEDLL", "_64BUILD"}

    includedirs {"source/engine2/common", "source/engine2/shared"}

    includedirs {"source/engine2/gamedll", "source/engine2/common", "source/engine2/shared"}

    files {"source/engine2/gamedll/**.cpp", "source/engine2/gamedll/**.h", "source/engine2/gamedll/**.hpp"}

    files {"source/engine2/shared/**.cpp", "source/engine2/shared/**.h", "source/engine2/shared/**.hpp",
           "source/engine2/common/**.cpp", "source/engine2/common/**.h", "source/engine2/common/**.hpp"}

    -- links {"glfw3", "ffi"}

    links {"SDL2"}

    links {"opengl32"}

    files {"premake5.lua"}

    -- warnings "off"

end

project "engine2"
do
    kind "ConsoleApp"
    language "C++"
    targetdir "./bin"
    debugdir "./bin"

    defines {"_64BUILD"}

    defines {"NETWORKING", "DONT_USE_VAO"}

    includedirs {"source/engine2/common", "source/engine2/shared"}

    includedirs {"source/engine2/engine", "source/engine2/shared/renderer", "source/engine2/engine/renderer",
                 "source/engine2/common", "libs", "source/engine2/engine/ui", "source/engine2/engine/server",
                 "source/engine2/engine/client", "source/engine2/shared", "source/engine2/engine/windows"}

    files {"source/engine2/engine/**.cpp", "source/engine2/engine/**.h", "source/engine2/engine/**.hpp"}

    files {"source/engine2/shared/**.cpp", "source/engine2/shared/**.h", "source/engine2/shared/**.hpp",
           "source/engine2/common/**.cpp", "source/engine2/common/**.h", "source/engine2/common/**.hpp"}

    -- links {"client"}

    links {"SDL2"}

    links {"ws2_32", "winmm", "user32"}

    links {"opengl32"}

    files {"premake5.lua"}

    -- warnings "off"

end

