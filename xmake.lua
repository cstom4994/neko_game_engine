set_project("neko")

includes("@builtin/xpack")

-- includes("xmake_box2d.lua")

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
    add_requires("lua", "sokol", "sokol-shdc", "miniz", "stb", "libffi")
    add_requires("imgui v1.90.9-docking", {
        configs = {
            wchar32 = true,
            backend = "none",
            freetype = true
        }
    })
    add_requires("miniaudio", "box2d", "enet", "flecs 3.2.11")
else
    -- add_requires("lua", "sokol", "miniz", "stb")
    -- add_requires("miniaudio", "enet", "flecs 3.2.11")
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

rule("sg_shader")
do
    set_extensions(".glsl")
    on_buildcmd_file(function(target, batchcmds, sourcefile, opt)
        import("lib.detect.find_tool")
        local sokolshdc = find_tool("sokol-shdc", {
            check = "--help"
        })
        local targetfile = path.relative(sourcefile, "$(projectdir)")
        if targetfile:startswith("..") then
            targetfile = targetfile:sub(4)
        end
        if targetfile:startswith("src") then
            targetfile = targetfile:sub(5)
        end
        batchcmds:mkdir(path.join("$(buildir)/sokol_shader", path.directory(targetfile)))
        local targetfile = vformat(path.join("$(buildir)/sokol_shader", targetfile .. ".h"))
        batchcmds:vrunv(sokolshdc.program,
            {"--ifdef", "-l", "hlsl5:glsl410:glsl300es:metal_macos:metal_ios:metal_sim", "--input", sourcefile,
             "--output", targetfile})
        batchcmds:show_progress(opt.progress, "${color.build.object}glsl %s", sourcefile)
        batchcmds:add_depfiles(sourcefile)
    end)
end

target("shader")
do
    set_kind("object")
    add_packages("sokol-shdc")
    add_rules("sg_shader")
    add_files("source/**.glsl")
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

    add_includedirs("$(buildir)/sokol_shader")

    add_packages(base_libs)
    add_packages("miniaudio", "box2d", "enet", "flecs")

    add_deps("shader")

    set_basename("neko_$(mode)_$(arch)")
    set_targetdir("./")
    set_rundir("./")
end

xpack("gamedata")
do
    set_formats("zip")
    add_installfiles("gamedir/(**)")
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

package("sokol-shdc")
do
    set_kind("binary")
    set_license("MIT")
    set_homepage("https://github.com/floooh/sokol-tools")
    local version = "2024.07.01"
    local url_prefix = "https://github.com/floooh/sokol-tools-bin/raw/$(version)/bin/"
    local url_suffix = "win32/sokol-shdc.exe"
    if is_host("macosx") then
        if os.arch() == "arm64" then
            url_suffix = "osx_arm64/sokol-shdc"
            add_versions(version, "5b662dd49eeee2d0a1116d8be47609024207695a07c0cf796a36e1d2eba1a51f")
        else
            url_suffix = "osx/sokol-shdc"
            add_versions(version, "31c88f494ef0f0c9287d73087d00db19df6150c8c7bb70df2ab62051f1bd817f")
        end
    elseif is_host("linux") then
        url_suffix = "linux/sokol-shdc"
        add_versions(version, "1bd32769dfc4f41121816f475d7a37a4fbd539435ba972a7791b31653872c828")
    else
        add_versions(version, "4944cf9ac838d06aac6df89fa64eee05034a3275929a757ac88e294ffd5ea58c")
    end
    set_urls(url_prefix .. url_suffix, {
        version = function(version)
            local versions = {
                ["2024.07.01"] = "0d91b038780614a867f2c8eecd7d935d76bcaae3"
            }
            return versions[tostring(version)]
        end
    })
    on_install(function(package)
        local bin = package:installdir("bin")
        if is_host("windows") then
            os.cp("../sokol-shdc.exe", bin)
        else
            os.run("chmod 755 ../sokol-shdc")
            os.cp("../sokol-shdc", bin)
        end
    end)
    on_test(function(package)
        os.vrun("sokol-shdc --help")
    end)
end
