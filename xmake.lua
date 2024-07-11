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

local base_libs = {"glfw", "libffi", "lua", "imgui", "miniz", "stb"}

if is_os("windows") or is_os("macosx") or is_os("linux") then
    add_requires("glfw", "libffi", "lua", "miniz", "stb")
    add_requires("imgui v1.90.8-docking", {
        configs = {
            wchar32 = true
        }
    })
    add_requires("miniaudio", "box2d", "enet", "flecs")
else
    -- add_requires(base_libs)
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

target("neko_engine")
do
    set_kind("static")

    add_rules("utils.bin2c", {
        extensions = {".lua"}
    })

    add_files("source/engine/embed/*.lua")
    add_files("source/engine/**.cpp")
    add_headerfiles("source/engine/**.h", "source/engine/**.hpp")

    add_packages(base_libs)
    add_packages("miniaudio", "box2d", "enet", "flecs")
end

local function neko_module_name(m)
    return "neko_module_" .. m
end

local function neko_build_register(module, package)
    local buildname = neko_module_name(module)
    target(buildname)
    do
        set_kind("shared")
        add_defines("NEKO_MODULE_BUILD")
        add_rules("utils.bin2c", {
            extensions = {".lua"}
        })
        add_files("source/modules/" .. module .. "/**.c", "source/modules/" .. module .. "/**.cpp")
        add_files("source/modules/" .. module .. "/*.lua")
        -- add_deps("neko_engine")
        add_packages(base_libs, package)

        set_targetdir("./")
    end
end

-- neko_build_register("sound", {"miniaudio"})
-- neko_build_register("physics", {"box2d"})
-- neko_build_register("flecs", {"flecs"})

target("sandbox")
do
    set_kind("binary")

    add_files("source/sandbox/**.cpp")
    add_headerfiles("source/sandbox/**.h", "source/sandbox/**.hpp")

    add_deps("neko_engine")

    add_packages(base_libs)
    add_packages("miniaudio", "box2d", "enet", "flecs")

    set_targetdir("./")
    set_rundir("./")
end

-- target(neko_module_name("test"))
-- do
--     set_kind("shared")
-- end

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
