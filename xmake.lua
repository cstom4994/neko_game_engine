set_project("neko")

add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("check.auto_ignore_flags", true)

set_languages("c17", "c++20")

add_rules("mode.debug", "mode.release")

add_includedirs("source/")

add_includedirs("source/deps/glad/include/")

if is_os("windows") then
    add_requires("glfw", "libffi")
else
    add_requires("glfw", "libffi")
end

if is_mode("debug") then
	add_defines("DEBUG", "_DEBUG")
	set_optimize("none")
	set_symbols("debug")
elseif is_mode("release") then
	add_defines("NDEBUG")
	set_optimize("faster")
end

-- set_fpmodels("strict")
set_exceptions("cxx", "objc")

if is_plat("windows") then
    add_defines("WIN32", "_WIN32", "_WINDOWS", "NOMINMAX", "UNICODE", "_UNICODE", "FFI_LITTLE_ENDIAN")
    add_cxflags("/utf-8", "/Zc:__cplusplus", "/permissive", "/bigobj", "/Zc:preprocessor", "/Zc:wchar_t",
        "/Zc:forScope", "/MP")
    -- add_ldflags("/MACHINE:X64", "/SUBSYSTEM:CONSOLE", "/INCREMENTAL")
    -- add_cxflags("/MT")
else
    add_cxflags("-fno-strict-aliasing", "-Wno-implicit-int", "-fms-extensions", "-Wno-error", "-Wno-multichar",
        "-Wno-unsequenced", "-Wno-unqualified-std-cast-call", "-Wno-implicit-const-int-float-conversion",
        "-Wno-unused-value", "-Wno-pointer-bool-conversion", "-Wno-unknown-attributes", "-Wno-return-stack-address",
        "-Wno-writable-strings", "-Wno-format", "-Wno-switch", "-Wno-incompatible-pointer-types",
        "-Wno-tautological-constant-out-of-range-compare", "-Wno-tautological-pointer-compare",
        "-Wno-shift-op-parentheses", "-Wno-visibility", "-Wno-parentheses", "-Wno-pointer-sign",
        "-Wno-ignored-attributes", "-Wno-c99-designator", "-Wno-null-conversion", "-finline-functions", "-fPIC")
end

if is_mode("release") then
    set_runtimes("MT")
else
    set_runtimes("MTd")
end

target("neko_engine")
do
    set_kind("static")

    add_files("source/engine/**.c")
    add_files("source/engine/**.cpp")
    add_files("source/deps/impl_build.cpp")

    add_packages("glfw", "libffi")
end

target("sandbox")
do
    set_kind("binary")

    add_files("source/sandbox/**.c")
    add_files("source/sandbox/**.cpp")

    add_deps("neko_engine")

    add_packages("glfw", "libffi")

    set_targetdir("./")
    set_rundir("./")
end

