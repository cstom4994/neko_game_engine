set_project("neko")

includes("@builtin/xpack")

add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {
    outputdir = ".vscode"
})

set_policy("check.auto_ignore_flags", true)

set_languages("c17", "c++20")

add_rules("mode.debug", "mode.release")

add_includedirs("source/")

local base_libs = {"lua", "sokol", "imgui", "miniz", "stb", "libffi"}

if is_os("windows") or is_os("macosx") or is_os("linux") then
    add_requires("lua", "sokol", "miniz", "stb", "libffi")
    add_requires("imgui v1.90.9-docking", {
        configs = {
            wchar32 = true
        }
    })
    add_requires("miniaudio", "box2d", "enet", "flecs 3.2.11")
else
    add_requires("lua", "sokol", "miniz", "stb")
    add_requires("miniaudio", "enet", "flecs 3.2.11")
end

if is_mode("debug") then
    add_defines("DEBUG", "_DEBUG")
    set_optimize("none")
    set_symbols("debug")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("faster")
    -- set_symbols("hidden")
    -- set_strip("all")
end

-- set_fpmodels("strict")
set_exceptions("cxx", "objc")

add_defines("FFI_LITTLE_ENDIAN", "UNICODE", "_UNICODE")

if is_plat("windows") then
    add_defines("WIN32", "_WIN32", "_WINDOWS", "NOMINMAX")
    add_cxflags("/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj", "/Zc:preprocessor", "/Zc:wchar_t",
        "/Zc:forScope", "/MP")
    -- add_ldflags("/MACHINE:X64", "/SUBSYSTEM:CONSOLE", "/INCREMENTAL")
    -- add_cxflags("/MT")
else
    add_cxflags("-Wtautological-compare")
    add_cxflags("-fno-strict-aliasing", "-fms-extensions", "-finline-functions", "-fPIC")
    -- add_cxflags("-Wno-implicit-int", "-Wno-error", "-Wno-multichar", "-Wno-unsequenced",
    --     "-Wno-unqualified-std-cast-call", "-Wno-implicit-const-int-float-conversion", "-Wno-unused-value",
    --     "-Wno-pointer-bool-conversion", "-Wno-unknown-attributes", "-Wno-return-stack-address", "-Wno-writable-strings",
    --     "-Wno-format", "-Wno-switch", "-Wno-incompatible-pointer-types",
    --     "-Wno-tautological-constant-out-of-range-compare", "-Wno-tautological-pointer-compare",
    --     "-Wno-shift-op-parentheses", "-Wno-visibility", "-Wno-parentheses", "-Wno-pointer-sign",
    --     "-Wno-ignored-attributes", "-Wno-c99-designator", "-Wno-null-conversion")

end

if is_plat("linux") then
    add_syslinks("GL")
end

if is_mode("release") then
    set_runtimes("MT")
else
    set_runtimes("MTd")
end

target("neko")
do
    set_kind("binary")

    add_rules("utils.bin2c", {
        extensions = {".lua", ".ttf"}
    })

    add_files("source/engine/embed/*.ttf", "source/engine/embed/*.lua")

    add_files("source/engine/**.cpp")
    add_headerfiles("source/engine/**.h", "source/engine/**.hpp")

    add_packages(base_libs)
    add_packages("miniaudio", "box2d", "enet", "flecs")

    set_targetdir("./")
    set_rundir("./")
end

xpack("luacode")
do
    set_formats("zip")
    add_installfiles("(source/lua/**)")
    before_package(function(package)
        local outputfile = package:outputfile()
        -- if os.exists(outputfile) then
        --     os.rm(outputfile)
        -- end
    end)
    after_package(function(package)
        local outputfile = package:outputfile()
        if os.exists(outputfile) then
            os.mv(outputfile, "./luacode.zip")
        end
    end)
end

xpack("gamedata")
do
    set_formats("zip")
    add_installfiles("(gamedir/**)")
    before_package(function(package)
        local outputfile = package:outputfile()
        -- if os.exists(outputfile) then
        --     os.rm(outputfile)
        -- end
    end)
    after_package(function(package)
        local outputfile = package:outputfile()
        if os.exists(outputfile) then
            os.mv(outputfile, "./gamedata.zip")
        end
    end)
end
