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

local NEKO_CFFI = false

local base_libs = {"sokol", "imgui", "miniz", "stb", "cute_headers"}

add_requires("sokol-shdc")
add_requires("cffi-lua")

add_requires("glew")
add_requires("glfw")
add_requires("dirent")

add_requires("openrestry-luajit", {
    configs = {
        gc64 = true
    }
})

add_requires("libffi")

add_requires("lua")
add_requires("sokol")
add_requires("miniz")
add_requires("stb")
add_requires("miniaudio")
add_requires("box2d")
add_requires("enet")
add_requires("cute_headers")
add_requires("imgui v1.90.9-docking", {
    configs = {
        wchar32 = true,
        freetype = true,
        glfw = true,
        opengl3 = true
    }
})

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
        extensions = {".lua"}
    })

    add_files("source/vendor/luasocket/auxiliar.c", "source/vendor/luasocket/buffer.c",
        "source/vendor/luasocket/compat.c", "source/vendor/luasocket/except.c", "source/vendor/luasocket/inet.c",
        "source/vendor/luasocket/io.c", "source/vendor/luasocket/luasocket.c", "source/vendor/luasocket/mime.c",
        "source/vendor/luasocket/options.c", "source/vendor/luasocket/select.c", "source/vendor/luasocket/tcp.c",
        "source/vendor/luasocket/timeout.c", "source/vendor/luasocket/udp.c", "source/vendor/luasocket/*.lua")

    if is_plat("windows") then
        add_files("source/vendor/luasocket/wsocket.c")
    else
        add_files("source/vendor/luasocket/serial.c", "source/vendor/luasocket/unix.c",
            "source/vendor/luasocket/unixdgram.c", "source/vendor/luasocket/unixstream.c",
            "source/vendor/luasocket/usocket.c")
    end

    add_files("source/vendor/*.c")

    add_files("source/api/gen/**.lua", "source/api/*.lua")

    add_files("source/api/**.cpp", "source/engine/**.cpp")

    add_headerfiles("source/engine/**.h", "source/engine/**.hpp")

    add_includedirs("$(buildir)/sokol_shader")

    add_packages(base_libs)
    add_packages("miniaudio", "box2d", "enet")

    if NEKO_CFFI == true then
        add_packages("lua", "libffi")
        add_defines("NEKO_CFFI", "FFI_LITTLE_ENDIAN")

        add_files("source/vendor/cffi/*.cc")
        add_headerfiles("source/vendor/cffi/*.hh")
        add_includedirs("source/vendor/cffi")

        add_files("source/vendor/bit.c")

    else
        add_packages("openrestry-luajit")
    end

    remove_files("source/vendor/bit.c")

    add_deps("shader")

    set_basename("neko_$(mode)_$(arch)")
    set_targetdir("./")
    set_rundir("./")
end

xpack("luacode")
do
    set_formats("zip")
    add_installfiles("source/game/(**)")
    before_package(function(package)
        local outputfile = package:outputfile()
        -- if os.exists(outputfile) then
        --     os.rm(outputfile)
        -- end
    end)
    after_package(function(package)
        local outputfile = package:outputfile()
        if os.exists(outputfile) then
            os.mv(outputfile, "./gamedir/code.zip")
        end
    end)
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

package("cffi-lua")
do
    set_homepage("https://github.com/q66/cffi-lua")
    set_description("A portable C FFI for Lua 5.1+")
    set_license("MIT")

    add_urls("https://github.com/q66/cffi-lua.git")

    add_versions("v0.2.3", "b1b2d772f87581dde91e224de875164cc182c162")

    add_deps("libffi", "lua")

    on_install("windows", "macosx", "linux", function(package)
        local configs = {}
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            add_requires("libffi", "lua")

            target("cffi-lua")
                set_kind("$(kind)")
                add_files("src/*.cc")
                add_headerfiles("src/*.hh")
                add_includedirs("src")

                if is_kind("shared") then
                    add_defines("CFFI_LUA_DLL")
                end

                add_defines("FFI_LITTLE_ENDIAN")

                add_packages("libffi", "lua")
        ]])
        if package:config("shared") then
            configs.kind = "shared"
        end
        import("package.tools.xmake").install(package, configs)
    end)
end
