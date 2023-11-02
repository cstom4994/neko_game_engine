cmake_minimum_required(VERSION 3.21)

project(neko_engine)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

file(GLOB_RECURSE neko_engine_src CONFIGURE_DEPENDS "source/engine/*.c")
file(GLOB neko_engine_lib CONFIGURE_DEPENDS "dependencies/glfw/lib/*.lib")

include_directories("source/" "dependencies/glfw/include/")

add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -DNOMINMAX -DDEBUG -D_DEBUG -DUNICODE -D_UNICODE)

if (MSVC)
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
        /EHsc
        /link
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
endif ()

if (MSVC)
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

set(neko_engine_src_all
    ${neko_engine_src}
    "source/libs/impl_build.cpp"
    )

set(neko_game_src_all
    "source/sandbox/no/nodesoup.cpp"
    "source/sandbox/game_physics_math.cpp"
    "source/sandbox/main.cpp"
    "source/sandbox/neko_sprite.c"
    "source/sandbox/test.c"
    "source/tests/test_se.cpp"
    "source/tests/test_sr.cpp"
    "source/tests/test_ut.cpp"
    )

set(neko_hotload_src
    "source/hotload/hotload.c"
    )

set(neko_script_test_src
    "source/neko_script/interpreter.cpp"
    )

add_library(neko_engine STATIC ${neko_engine_src_all})
target_link_libraries(neko_engine PRIVATE ${neko_engine_lib})

add_executable(sandbox ${neko_game_src_all})
target_link_libraries(sandbox neko_engine)
set_property(TARGET sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "./")
target_link_libraries(sandbox neko_engine)

add_library(hotload SHARED ${neko_hotload_src})
target_link_libraries(hotload neko_engine)

add_executable(script_test ${neko_script_test_src})
target_link_libraries(script_test neko_engine)
set_property(TARGET script_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "./")
target_link_libraries(script_test neko_engine)


# Copy DLLs to the build directory
# if(WIN32)
#     add_custom_command(
#         TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dependencies/lib $<TARGET_FILE_DIR:${PROJECT_NAME}>
#     )
# endif()

# Copy Assets to the build directory
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory
#                    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>)
