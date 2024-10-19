cmake_minimum_required(VERSION 3.12...3.27)
project(Neko LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_TOOLCHAIN_FILE "C:/Users/kaoruxun/scoop/apps/vcpkg/current/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(FMOD_LIB_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows" CACHE STRING "fmod here")

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Lua REQUIRED)

set(BOX2D
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

set(LUASOCKET
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
  set(LUASOCKET)
  add_compile_definitions(LUA_USE_LONGJMP LUA_USE_POSIX)
  set(CFLAGS -fno-exceptions)
  set(LFLAGS -lpthread -sASYNCIFY -sALLOW_MEMORY_GROWTH -sUSE_WEBGL2)

elseif(WIN32)
  set(LUASOCKET ${LUASOCKET} source/deps/luasocket/wsocket.c)
  add_compile_definitions(LUA_USE_LONGJMP)

  set(CFLAGS ${CFLAGS}
      /utf-8
      /Zc:__cplusplus
      /permissive
      /bigobj
      /Zc:preprocessor
      /Zc:wchar_t
      /Zc:forScope
      /MP
  )

elseif(UNIX)
  set(LUASOCKET
    ${LUASOCKET}
    source/deps/luasocket/serial.c
    source/deps/luasocket/unix.c
    source/deps/luasocket/unixdgram.c
    source/deps/luasocket/unixstream.c
    source/deps/luasocket/usocket.c
  )
  add_compile_definitions(LUA_USE_LINUX)
  set(LFLAGS -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm)
endif()

# set(IMGUI_SRC
#   source/deps/imgui/imgui.cpp
#   source/deps/imgui/imgui_demo.cpp
#   source/deps/imgui/imgui_draw.cpp
#   source/deps/imgui/imgui_tables.cpp
#   source/deps/imgui/imgui_widgets.cpp
#   source/deps/imgui/backends/imgui_impl_glfw.cpp
#   source/deps/imgui/backends/imgui_impl_opengl3.cpp
# )

add_compile_definitions(WIN32 _WIN32 _WINDOWS NOMINMAX)
add_compile_definitions(NEKO_CFFI NEKO_BOX2D NEKO_AUDIO=2)
add_compile_definitions(FFI_LITTLE_ENDIAN)
add_compile_definitions(UNICODE _UNICODE)

file(GLOB ENGINE_SRC CONFIGURE_DEPENDS source/engine/*.cpp source/engine/*.h source/engine/*.hpp source/engine/base/*.cpp source/engine/base/*.hpp source/engine/components/*.cpp source/engine/components/*.hpp source/engine/ecs/*.cpp source/engine/ecs/*.hpp source/engine/scripting/*.cpp source/engine/scripting/*.hpp)
file(GLOB EDITOR_SRC CONFIGURE_DEPENDS source/editor/*.cpp source/editor/*.h source/editor/*.hpp)
file(GLOB GAME_SRC CONFIGURE_DEPENDS source/game/*.cpp source/game/*.h source/game/*.hpp)
file(GLOB LIB_SRC CONFIGURE_DEPENDS
    #source/deps/lua/onelua.c
    source/vendor/cffi/*.cc 
    source/vendor/cffi/*.hh
    source/vendor/luaalloc.c
    source/vendor/bit.c
    source/vendor/http.c
    source/vendor/miniz.c
    source/vendor/ui.cpp
    source/vendor/glad/glad.c)

set(MAIN_NAME neko_engine)

add_executable(${MAIN_NAME} ${ENGINE_SRC} ${EDITOR_SRC} ${GAME_SRC} ${LIB_SRC} ${BOX2D} ${LUASOCKET})

target_include_directories(${MAIN_NAME} PRIVATE source source/vendor source/deps/imgui source/deps/box2d source/deps/libffi/include)
target_include_directories(${MAIN_NAME} PRIVATE "${FMOD_LIB_DIR}/api/core/inc" "${FMOD_LIB_DIR}/api/studio/inc")

target_include_directories(${MAIN_NAME} PRIVATE ${LUA_INCLUDE_DIR})

target_compile_options(${MAIN_NAME} PRIVATE ${CFLAGS})

target_link_directories(${MAIN_NAME} PRIVATE source/deps/libffi/lib "${FMOD_LIB_DIR}/api/core/lib/x64" "${FMOD_LIB_DIR}/api/studio/lib/x64")

target_link_libraries(${MAIN_NAME} PRIVATE glfw imgui::imgui ${LUA_LIBRARIES})
target_link_libraries(${MAIN_NAME} PRIVATE fmod_vc fmodstudio_vc ffi ws2_32 wininet)


target_link_libraries(${MAIN_NAME} PRIVATE ${LFLAGS})

set_target_properties(${MAIN_NAME} PROPERTIES
  OUTPUT_NAME_DEBUG "neko_debug_x64"
  OUTPUT_NAME_RELEASE "neko_release_x64"
)

set_target_properties(${MAIN_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)
set_target_properties(${MAIN_NAME} PROPERTIES
  PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
  PDB_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

