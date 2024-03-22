cmake_minimum_required(VERSION 3.21)

project(neko_engine)

# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(MSVC)
        file(GLOB glfw_lib CONFIGURE_DEPENDS "source/deps/glfw/lib/*.lib")
        file(GLOB ffi_lib CONFIGURE_DEPENDS "source/deps/libffi/lib/*.lib")

        set(neko_engine_lib ${glfw_lib} ${ffi_lib})
        include_directories(
                "source/deps/glfw/include/"
                "source/deps/libffi/include/"
        )
else()
        find_package(OpenGL REQUIRED)
        find_package(glfw3 REQUIRED)

        set(neko_engine_lib glfw)
endif()

include_directories("source/")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG -D_DEBUG)
        add_compile_options(
                /RTC1

                /JMC
                /Gm-
        )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
endif()

if(MSVC)
        add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -DNOMINMAX -DUNICODE -D_UNICODE -DFFI_LITTLE_ENDIAN)

        add_compile_options(
                /wd4005
                /wd4267
                /wd4244
                /wd4305
                /wd4018
                /wd4800
                /wd5030
                /wd5222
                /wd4554
                /wd4002
                /wd4099
                /wd4996
                /wd4090
                /wd4101
                /wd4133
        )
        add_compile_options(
                /utf-8
                /Zc:__cplusplus
                /permissive
                /bigobj
                /Zc:preprocessor
                /Zc:wchar_t
                /Zc:forScope

                /MP
        )
        add_link_options(
                /MACHINE:X64
                /SUBSYSTEM:CONSOLE
                /INCREMENTAL
        )

        SET(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od ")
        SET(CMAKE_CXX_FLAGS_RELEASE "/Zi /O2")
        SET(CMAKE_C_FLAGS_DEBUG "/ZI /Od")
        SET(CMAKE_C_FLAGS_RELEASE "/Zi /O2")
else()
        add_compile_options(

                -fno-strict-aliasing

                -Wno-implicit-int
                -fms-extensions
                -Wno-error
                -Wno-multichar
                -Wno-unsequenced
                -Wno-unqualified-std-cast-call
                -Wno-implicit-const-int-float-conversion
                -Wno-unused-value
                -Wno-pointer-bool-conversion
                -Wno-unknown-attributes
                -Wno-return-stack-address
                -Wno-writable-strings
                -Wno-format
                -Wno-switch
                -Wno-incompatible-pointer-types
                -Wno-tautological-constant-out-of-range-compare
                -Wno-tautological-pointer-compare
                -Wno-shift-op-parentheses
                -Wno-visibility
                -Wno-parentheses
                -Wno-pointer-sign
                -Wno-ignored-attributes
                -Wno-c99-designator
                -Wno-null-conversion

                -finline-functions
                -fPIC
        )

        add_link_options(
        )
endif()

if(MSVC)
        set(CompilerFlags
                CMAKE_CXX_FLAGS
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_C_FLAGS
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
        )

        foreach(CompilerFlag ${CompilerFlags})
                string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
endif(MSVC)

set(luasocket_src
        source/deps/luasocket/auxiliar.c
        source/deps/luasocket/buffer.c
        source/deps/luasocket/compat.c
        source/deps/luasocket/except.c
        source/deps/luasocket/inet.c
        source/deps/luasocket/io.c
        source/deps/luasocket/luasocket.c
        source/deps/luasocket/mime.c
        source/deps/luasocket/options.c
        source/deps/luasocket/select.c
        source/deps/luasocket/tcp.c
        source/deps/luasocket/timeout.c
        source/deps/luasocket/udp.c
)

if(EMSCRIPTEN)
        set(luasocket_src)
        add_compile_definitions(LUA_USE_LONGJMP LUA_USE_POSIX)
        set(CFLAGS -fno-exceptions)
        set(LFLAGS -lpthread -sASYNCIFY -sALLOW_MEMORY_GROWTH -sUSE_WEBGL2)

elseif(WIN32)
        set(luasocket_src ${luasocket_src} source/deps/luasocket/wsocket.c)
        add_compile_definitions(LUA_USE_LONGJMP)

elseif(UNIX)
        set(luasocket_src
                ${luasocket_src}
                source/deps/luasocket/serial.c
                source/deps/luasocket/unix.c
                source/deps/luasocket/unixdgram.c
                source/deps/luasocket/unixstream.c
                source/deps/luasocket/usocket.c
        )
        add_compile_definitions(LUA_USE_LINUX)
        set(LFLAGS -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm)
endif()

file(GLOB_RECURSE luaffi_src CONFIGURE_DEPENDS "source/deps/luaffi/**.*")

file(GLOB_RECURSE neko_engine_src CONFIGURE_DEPENDS "source/engine/**.*")

set(neko_engine_src_all
        ${neko_engine_src}
        ${luasocket_src}
        ${luaffi_src}
        "source/deps/impl_build.cpp"
)

file(GLOB_RECURSE neko_game_src CONFIGURE_DEPENDS "source/sandbox/**.*")

set(neko_game_src_all
        ${neko_game_src}
)

set(tools_packer_src
        "source/tools/packer.c"
)

add_library(neko_engine STATIC ${neko_engine_src_all})
target_link_libraries(neko_engine PRIVATE ${neko_engine_lib})

add_executable(sandbox ${neko_game_src_all})
target_link_libraries(sandbox neko_engine)
set_property(TARGET sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "./")
target_link_libraries(sandbox neko_engine)
