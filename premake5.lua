--
-- NekoEngine
--
workspace "neko_engine"
configurations {"Debug", "Release"}

buildoptions {"/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj"}
disablewarnings {"4005", "4267", "4244", "4305", "4018", "4800", "5030", "5222", "4554", "4002", "4099"}

cppdialect "C++20"
cdialect "C17"

-- characterset("MBCS")

location "vsbuild"

flags {"MultiProcessorCompile"}

staticruntime "on"

win32_libs = {"DbgHelp", "winmm", "opengl32", "kernel32", "user32", "gdi32", "iphlpapi", "Shlwapi", "wsock32", "ws2_32",
              "shell32", "advapi32", "imm32", "bcrypt", "Avrt", "dwmapi", "Version", "Usp10", "userenv", "psapi",
              "setupapi", "ole32", "oleaut32"}

defines {"WIN32", "_WIN32", "_WINDOWS", "NOMINMAX", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_DEPRECATE",
         "_CRT_SECURE_NO_WARNINGS", "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS", "WIN32_LEAN_AND_MEAN",
         "_SCL_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_CRT_SILENCE_NONCONFORMING_TGMATH_H"}

platforms {"Win64"}

filter "configurations:Debug"
defines {"_DEBUG", "DEBUG"} -- "_CRTDBG_MAP_ALLOC"
symbols "On"

filter "configurations:Release"
defines {"_NDEBUG", "NDEBUG"}
optimize "On"
flags {"NoRuntimeChecks"}

-- if is_arch("arm.*") then
--        add_defines("__METADOT_ARCH_ARM")
-- elseif is_arch("x86_64", "i386") then
--        add_defines("__METADOT_ARCH_X86")
-- end

-- filter "platforms:Win32"
-- system "Windows"
-- architecture "x86"

filter "platforms:Win64"
system "Windows"
architecture "x86_64"
libdirs {"dependencies/libffi/lib", "dependencies/fmod/lib", "dependencies/glfw/lib"}

objdir "vsbuild/obj/%{cfg.platform}_%{cfg.buildcfg}"

includedirs {"source"}

includedirs {"dependencies/libffi/include", "dependencies/fmod/include", "dependencies/glfw/include"}

----------------------------------------------------------------------------
-- projects
----------------------------------------------------------------------------

project "external"
do
    kind "StaticLib"
    language "C++"

    files {"source/libs/**.cpp", "source/libs/**.c"}
    files {"source/libs/**.h", "source/libs/**.hpp"}

    files {"dependencies/libffi/include/**.**", "dependencies/fmod/include/**.**"}

    vpaths {
        ["libs/*"] = {"source/libs"},
        ["deps/glfw/*"] = {"dependencies/glfw/include"},
        ["deps/libffi/*"] = {"dependencies/libffi/include"},
        ["deps/fmod/*"] = {"dependencies/fmod/include"}
    }
end

project "engine_core"
do
    kind "StaticLib"
    language "C++"

    files {"source/engine/**.cpp", "source/engine/**.c",  "source/engine/**.h", "source/engine/**.hpp", "source/engine/**.inl"}

    vpaths {
        ["*"] = {"source/engine_core"}
    }
end

-- just for dev
project "_old_engine"
do
    kind "None"
    files {"../engine/**"}
    -- removefiles {"**.cpp", "**.h", "**.c"}
end

project "sandbox"
do
    kind "ConsoleApp"
    language "C++"
    targetdir "output"
    debugdir "output/../"

    files {"source/sandbox/**.cpp", "source/sandbox/**.h"}
    files {"data/scripts/**.lua", "data/shaders/**.**"}
    files {"premake5.lua"}

    vpaths {
        ["engine/*"] = {"source/engine"},
        ["scripts/*"] = {"data/scripts"},
        ["shaders/*"] = {"data/shaders"},
        ["*"] = {"premake5.lua"}
    }

    links {"external", "engine_core", "glfw3", "ffi", win32_libs}
end
