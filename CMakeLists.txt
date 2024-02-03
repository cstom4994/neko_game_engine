cmake_minimum_required(VERSION 3.21)

project(neko_engine)

# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
        file(GLOB neko_engine_lib CONFIGURE_DEPENDS "dependencies/glfw/lib/*.lib")
        include_directories(
                "dependencies/glfw/include/"
                "dependencies/netcore/"
        )
else()
        find_package(OpenGL REQUIRED)
        find_package(glfw3 REQUIRED)

        set(neko_engine_lib glfw)
endif()

include_directories("source/")

add_definitions(-DDEBUG -D_DEBUG)

if(MSVC)
        add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -DNOMINMAX -DUNICODE -D_UNICODE)

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
                /RTC1

                /JMC
                /MP
                /Gm-
                /Od
        )
        add_link_options(
                /MACHINE:X64
                /SUBSYSTEM:CONSOLE
                /NODEFAULTLIB:msvcrt.lib
                /NODEFAULTLIB:LIBCMT
                /INCREMENTAL
        )

        SET(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od ")
        SET(CMAKE_CXX_FLAGS_RELEASE "/Zi /O2")
        SET(CMAKE_C_FLAGS_DEBUG "/ZI /Od")
        SET(CMAKE_C_FLAGS_RELEASE "/Zi /O2")
else()
        add_compile_options(

                # -finput-charset=UTF-8            # Equivalent to /utf-8
                # -pedantic-errors                 # Equivalent to /permissive (strict standards)
                # -flto                            # Enable link-time optimization, similar to /bigobj
                # -g                               # Generate debug information, similar to /ZI
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

file(GLOB_RECURSE neko_engine_src CONFIGURE_DEPENDS "source/engine/**.*")

file(GLOB_RECURSE luaecs_src CONFIGURE_DEPENDS "source/libs/luaecs/**.*")

set(neko_engine_src_all
        ${neko_engine_src}
        ${luaecs_src}
        "source/libs/impl_build.cpp"
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

# add_library(hotload SHARED ${neko_hotload_src})
# target_link_libraries(hotload neko_engine)

# add_executable(tools_packer ${tools_packer_src})
# set_property(TARGET tools_packer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "./")
# target_link_libraries(tools_packer neko_engine)

# Copy DLLs to the build directory
# if(WIN32)
# add_custom_command(
# TARGET ${PROJECT_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dependencies/lib $<TARGET_FILE_DIR:${PROJECT_NAME}>
# )
# endif()

# Copy Assets to the build directory
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy_directory
# ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# add_subdirectory(source/engine/managed)