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

find_package(unofficial-libffi CONFIG REQUIRED)

find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)

set(BOX2D
  source/extern/box2d/collision/b2_broad_phase.cpp
  source/extern/box2d/collision/b2_chain_shape.cpp
  source/extern/box2d/collision/b2_circle_shape.cpp
  source/extern/box2d/collision/b2_collide_circle.cpp
  source/extern/box2d/collision/b2_collide_edge.cpp
  source/extern/box2d/collision/b2_collide_polygon.cpp
  source/extern/box2d/collision/b2_collision.cpp
  source/extern/box2d/collision/b2_distance.cpp
  source/extern/box2d/collision/b2_dynamic_tree.cpp
  source/extern/box2d/collision/b2_edge_shape.cpp
  source/extern/box2d/collision/b2_polygon_shape.cpp
  source/extern/box2d/collision/b2_time_of_impact.cpp
  source/extern/box2d/common/b2_block_allocator.cpp
  source/extern/box2d/common/b2_draw.cpp
  source/extern/box2d/common/b2_math.cpp
  source/extern/box2d/common/b2_settings.cpp
  source/extern/box2d/common/b2_stack_allocator.cpp
  source/extern/box2d/common/b2_timer.cpp
  source/extern/box2d/dynamics/b2_body.cpp
  source/extern/box2d/dynamics/b2_chain_circle_contact.cpp
  source/extern/box2d/dynamics/b2_chain_polygon_contact.cpp
  source/extern/box2d/dynamics/b2_circle_contact.cpp
  source/extern/box2d/dynamics/b2_contact.cpp
  source/extern/box2d/dynamics/b2_contact_manager.cpp
  source/extern/box2d/dynamics/b2_contact_solver.cpp
  source/extern/box2d/dynamics/b2_distance_joint.cpp
  source/extern/box2d/dynamics/b2_edge_circle_contact.cpp
  source/extern/box2d/dynamics/b2_edge_polygon_contact.cpp
  source/extern/box2d/dynamics/b2_fixture.cpp
  source/extern/box2d/dynamics/b2_friction_joint.cpp
  source/extern/box2d/dynamics/b2_gear_joint.cpp
  source/extern/box2d/dynamics/b2_island.cpp
  source/extern/box2d/dynamics/b2_joint.cpp
  source/extern/box2d/dynamics/b2_motor_joint.cpp
  source/extern/box2d/dynamics/b2_mouse_joint.cpp
  source/extern/box2d/dynamics/b2_polygon_circle_contact.cpp
  source/extern/box2d/dynamics/b2_polygon_contact.cpp
  source/extern/box2d/dynamics/b2_prismatic_joint.cpp
  source/extern/box2d/dynamics/b2_pulley_joint.cpp
  source/extern/box2d/dynamics/b2_revolute_joint.cpp
  source/extern/box2d/dynamics/b2_weld_joint.cpp
  source/extern/box2d/dynamics/b2_wheel_joint.cpp
  source/extern/box2d/dynamics/b2_world.cpp
  source/extern/box2d/dynamics/b2_world_callbacks.cpp
  source/extern/box2d/rope/b2_rope.cpp
)

set(LUASOCKET
  source/extern/luasocket/auxiliar.c
  source/extern/luasocket/buffer.c
  source/extern/luasocket/compat.c
  source/extern/luasocket/except.c
  source/extern/luasocket/inet.c
  source/extern/luasocket/io.c
  source/extern/luasocket/luasocket.c
  source/extern/luasocket/mime.c
  source/extern/luasocket/options.c
  source/extern/luasocket/select.c
  source/extern/luasocket/tcp.c
  source/extern/luasocket/timeout.c
  source/extern/luasocket/udp.c
)

if(EMSCRIPTEN)
  set(LUASOCKET)
  add_compile_definitions(LUA_USE_LONGJMP LUA_USE_POSIX)
  set(CFLAGS -fno-exceptions)
  set(LFLAGS -lpthread -sASYNCIFY -sALLOW_MEMORY_GROWTH -sUSE_WEBGL2)

elseif(WIN32)
  set(LUASOCKET ${LUASOCKET} source/extern/luasocket/wsocket.c)
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
    source/extern/luasocket/serial.c
    source/extern/luasocket/unix.c
    source/extern/luasocket/unixdgram.c
    source/extern/luasocket/unixstream.c
    source/extern/luasocket/usocket.c
  )
  add_compile_definitions(LUA_USE_LINUX)
  set(LFLAGS -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm)
endif()

# set(IMGUI_SRC
#   source/extern/imgui/imgui.cpp
#   source/extern/imgui/imgui_demo.cpp
#   source/extern/imgui/imgui_draw.cpp
#   source/extern/imgui/imgui_tables.cpp
#   source/extern/imgui/imgui_widgets.cpp
#   source/extern/imgui/backends/imgui_impl_glfw.cpp
#   source/extern/imgui/backends/imgui_impl_opengl3.cpp
# )

add_compile_definitions(WIN32_LEAN_AND_MEAN WIN32 _WIN32 _WINDOWS NOMINMAX)
add_compile_definitions(NEKO_CFFI NEKO_BOX2D NEKO_AUDIO=2)
add_compile_definitions(FFI_LITTLE_ENDIAN GLFW_INCLUDE_NONE)
add_compile_definitions(UNICODE _UNICODE)

file(GLOB BASE_SRC CONFIGURE_DEPENDS source/base/*.cpp source/base/*.hpp source/base/common/*.cpp source/base/common/*.hpp source/base/scripting/*.cpp source/base/scripting/*.hpp)
file(GLOB ENGINE_SRC CONFIGURE_DEPENDS source/engine/*.cpp source/engine/*.h source/engine/*.hpp source/engine/components/*.cpp source/engine/components/*.hpp source/engine/ecs/*.cpp source/engine/ecs/*.hpp source/engine/renderer/*.cpp source/engine/renderer/*.h source/engine/scripting/*.cpp source/engine/scripting/*.hpp)
file(GLOB EDITOR_SRC CONFIGURE_DEPENDS source/editor/*.cpp source/editor/*.h source/editor/*.hpp)
file(GLOB GAME_SRC CONFIGURE_DEPENDS source/game/*.cpp source/game/*.h source/game/*.hpp)
file(GLOB GEN_SRC CONFIGURE_DEPENDS source/gen/*.cpp)
file(GLOB LIB_SRC CONFIGURE_DEPENDS
    #source/extern/lua/onelua.c
    source/vendor/luaalloc.c
    source/vendor/http.c
    source/vendor/miniz.c
    source/vendor/ui.cpp
    source/vendor/glad/glad.c)

set(MAIN_NAME neko_engine)

add_executable(${MAIN_NAME} ${BASE_SRC} ${ENGINE_SRC} ${EDITOR_SRC} ${GAME_SRC} ${GEN_SRC} ${LIB_SRC} ${BOX2D} ${LUASOCKET})

target_include_directories(${MAIN_NAME} PRIVATE source source/vendor source/extern/wamr/include source/extern/box2d)
target_include_directories(${MAIN_NAME} PRIVATE "${FMOD_LIB_DIR}/api/core/inc" "${FMOD_LIB_DIR}/api/studio/inc")

target_include_directories(${MAIN_NAME} PRIVATE ${LUA_INCLUDE_DIR} ${DETOURS_INCLUDE_DIRS})

target_compile_options(${MAIN_NAME} PRIVATE ${CFLAGS})

target_link_directories(${MAIN_NAME} PRIVATE "source/extern/wamr/lib" "${FMOD_LIB_DIR}/api/core/lib/x64" "${FMOD_LIB_DIR}/api/studio/lib/x64")

target_link_libraries(${MAIN_NAME} PRIVATE unofficial::libffi::libffi glfw imgui::imgui ${LUA_LIBRARIES} ${DETOURS_LIBRARY})

target_link_libraries(${MAIN_NAME} PRIVATE fmod_vc fmodstudio_vc ws2_32 wininet vmlib iwasm)


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

