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
                "source/deps/box2d/"
                "source/deps/glad/include/"
                "source/deps/glfw/include/"
                "source/deps/libffi/include/"
                "source/deps/enet/include/"
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


set(box2d_src
        source/deps/box2d/collision/b2_broad_phase.cpp
        source/deps/box2d/collision/b2_chain_shape.cpp
        source/deps/box2d/collision/b2_circle_shape.cpp
        source/deps/box2d/collision/b2_collide_circle.cpp
        source/deps/box2d/collision/b2_collide_edge.cpp
        source/deps/box2d/collision/b2_collide_polygon.cpp
        source/deps/box2d/collision/b2_collision.cpp
        source/deps/box2d/collision/b2_distance.cpp
        source/deps/box2d/collision/b2_dynamic_tree.cpp
        source/deps/box2d/collision/b2_edge_shape.cpp
        source/deps/box2d/collision/b2_polygon_shape.cpp
        source/deps/box2d/collision/b2_time_of_impact.cpp
        source/deps/box2d/common/b2_block_allocator.cpp
        source/deps/box2d/common/b2_draw.cpp
        source/deps/box2d/common/b2_math.cpp
        source/deps/box2d/common/b2_settings.cpp
        source/deps/box2d/common/b2_stack_allocator.cpp
        source/deps/box2d/common/b2_timer.cpp
        source/deps/box2d/dynamics/b2_body.cpp
        source/deps/box2d/dynamics/b2_chain_circle_contact.cpp
        source/deps/box2d/dynamics/b2_chain_polygon_contact.cpp
        source/deps/box2d/dynamics/b2_circle_contact.cpp
        source/deps/box2d/dynamics/b2_contact.cpp
        source/deps/box2d/dynamics/b2_contact_manager.cpp
        source/deps/box2d/dynamics/b2_contact_solver.cpp
        source/deps/box2d/dynamics/b2_distance_joint.cpp
        source/deps/box2d/dynamics/b2_edge_circle_contact.cpp
        source/deps/box2d/dynamics/b2_edge_polygon_contact.cpp
        source/deps/box2d/dynamics/b2_fixture.cpp
        source/deps/box2d/dynamics/b2_friction_joint.cpp
        source/deps/box2d/dynamics/b2_gear_joint.cpp
        source/deps/box2d/dynamics/b2_island.cpp
        source/deps/box2d/dynamics/b2_joint.cpp
        source/deps/box2d/dynamics/b2_motor_joint.cpp
        source/deps/box2d/dynamics/b2_mouse_joint.cpp
        source/deps/box2d/dynamics/b2_polygon_circle_contact.cpp
        source/deps/box2d/dynamics/b2_polygon_contact.cpp
        source/deps/box2d/dynamics/b2_prismatic_joint.cpp
        source/deps/box2d/dynamics/b2_pulley_joint.cpp
        source/deps/box2d/dynamics/b2_revolute_joint.cpp
        source/deps/box2d/dynamics/b2_weld_joint.cpp
        source/deps/box2d/dynamics/b2_wheel_joint.cpp
        source/deps/box2d/dynamics/b2_world.cpp
        source/deps/box2d/dynamics/b2_world_callbacks.cpp
        source/deps/box2d/rope/b2_rope.cpp
)


file(GLOB_RECURSE neko_engine_src CONFIGURE_DEPENDS "source/engine/**.*")

set(neko_engine_src_all
        ${neko_engine_src}
        ${box2d_src}
        "source/deps/impl_build.cpp"
)

file(GLOB_RECURSE neko_game_src CONFIGURE_DEPENDS "source/sandbox/**.*")

set(neko_game_src_all
        ${neko_game_src}
)

add_library(neko_engine STATIC ${neko_engine_src_all})
target_link_libraries(neko_engine PRIVATE ${neko_engine_lib})

add_executable(sandbox ${neko_game_src_all})
target_link_libraries(sandbox neko_engine)
set_property(TARGET sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "./")
target_link_libraries(sandbox neko_engine)
