set_project("neko")

includes("@builtin/xpack")

-- add_repositories("my-repo myrepo", {
--     rootdir = os.scriptdir()
-- })

add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {
    outputdir = "build"
})

set_policy("check.auto_ignore_flags", true)

set_languages("c17", "c++20")

add_rules("mode.debug", "mode.release")

add_includedirs("source/")

local NEKO_BOX2D = false

-- local NEKO_AUDIO = "miniaudio"
local NEKO_AUDIO = "none"

add_requires("glew")
add_requires("glfw")
add_requires("miniz")
add_requires("stb")
add_requires("cute_headers")

add_requires("openrestry-luajit", {
    configs = {
        gc64 = true
    }
})

if NEKO_BOX2D then
    add_requires("box2d")
end

if NEKO_AUDIO == "miniaudio" then
    add_requires("miniaudio")
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

add_defines("UNICODE", "_UNICODE")

if is_plat("windows") then
    add_defines("WIN32", "_WIN32", "_WINDOWS", "NOMINMAX")
    add_cxflags("/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj", "/Zc:preprocessor", "/Zc:wchar_t",
        "/Zc:forScope", "/MP")
    -- add_ldflags("/MACHINE:X64", "/SUBSYSTEM:CONSOLE", "/INCREMENTAL")
    -- add_cxflags("/MT")
    add_syslinks("ws2_32", "wininet")
elseif is_plat("linux") then
    add_cxflags("-Wtautological-compare")
    add_cxflags("-fno-strict-aliasing", "-fms-extensions", "-finline-functions", "-fPIC")

    add_syslinks("GL")
elseif is_plat("macosx") then
    add_cxflags("-Wtautological-compare")
    add_cxflags("-fno-strict-aliasing", "-fms-extensions", "-finline-functions", "-fPIC")
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
        extensions = {".lua"}
    })

    add_files("source/engine/**.lua")
    add_files("source/engine/**.cpp")
    add_files("source/vendor/http.c")
    add_files("source/vendor/ui.c")
    -- add_files("source/engine2/*.c")

    add_headerfiles("source/engine/**.h", "source/engine/**.hpp", "source/engine2/**.h", "source/vendor/**.h")

    add_packages("miniz", "cute_headers")
    add_packages("glfw", "stb", "glew")
    add_packages("openrestry-luajit")

    if NEKO_BOX2D then
        add_packages("box2d")
        add_defines("NEKO_BOX2D=1")
    end

    if NEKO_AUDIO == "miniaudio" then
        add_packages("miniaudio")

        add_defines("NEKO_AUDIO=1")
    end

    set_basename("neko_$(mode)_$(arch)")
    set_targetdir("./bin")
    set_rundir("./bin")
end

xpack("gamedata")
do
    set_formats("zip")
    add_installfiles("gamedir/(**)", "source/game/(**)")
    before_package(function(package)
        local outputfile = package:outputfile()
        -- if os.exists(outputfile) then
        --     os.rm(outputfile)
        -- end
    end)
    after_package(function(package)
        local outputfile = package:outputfile()
        if os.exists(outputfile) then
            os.mv(outputfile, "./bin/gamedata.zip")
        end
    end)
end

task("distribute")
do
    on_run(function()
        import("core.project.project")

        -- local exe_target = project.target("neko")
        -- local exe_path = exe_target:targetfile()

        local exe_path = "neko_release_x64.exe"

        print("neko distribute", exe_path)

        if os.exists(exe_path) then
            local output_path = "neko.exe"

            local exe_file = io.open(exe_path, "rb")
            local zip_file = io.open("./gamedata.zip", "rb")

            local output_file = io.open(output_path, "wb")
            local exe_content = exe_file:read("*all")
            output_file:write(exe_content)
            local zip_content = zip_file:read("*all")
            output_file:write(zip_content)

            exe_file:close()
            zip_file:close()
            output_file:close()

            print("Good:", output_path)
        end
    end)

    set_menu {
        usage = "xmake distribute [options]",
        description = "neko distribute!",
        options = {}
    }
end
