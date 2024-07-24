#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#endif

#include "deps/luaalloc.c"

#define SOKOL_IMPL
#define SOKOL_TRACE_HOOKS
#if defined(_WIN32)
// #define SOKOL_D3D11
#define SOKOL_GLCORE
#define SOKOL_WIN32_FORCE_MAIN
#elif defined(__linux__)
#define SOKOL_GLCORE
#elif defined(__EMSCRIPTEN__)
#define SOKOL_GLES3
#endif
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_log.h>
#include <sokol_time.h>

#define SOKOL_GL_IMPL
#include <util/sokol_gl.h>

#define SOKOL_GP_IMPL
#include "deps/sokol_gp.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#define SOKOL_IMGUI_IMPL
#include <util/sokol_imgui.h>

#define SOKOL_GFX_IMGUI_IMPL
#include <util/sokol_gfx_imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "deps/cute_aseprite.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_WEBAUDIO
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif
