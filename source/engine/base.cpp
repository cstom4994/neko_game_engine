#include "engine/base.hpp"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <new>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "engine/bootstrap.h"
#include "engine/graphics.h"
#include "base/scripting/lua_wrapper.hpp"
#include "base/scripting/scripting.h"
#include "extern/luaalloc.h"

static void _error(const char *s) { script_error(s); }

void errorf(const char *fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char *s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char *)mem_alloc(n + 1);
    vsprintf(s, fmt, ap1);
    va_end(ap1);
    _error(s);
    mem_free(s);
}

Color color_black = {0.0, 0.0, 0.0, 1.0};
Color color_white = {1.0, 1.0, 1.0, 1.0};
Color color_gray = {0.5, 0.5, 0.5, 1.0};
Color color_red = {1.0, 0.0, 0.0, 1.0};
Color color_green = {0.0, 1.0, 0.0, 1.0};
Color color_blue = {0.0, 0.0, 1.0, 1.0};
Color color_clear = {0.0, 0.0, 0.0, 0.0};

#undef color_opaque
Color color_opaque(f32 r, f32 g, f32 b) { return color(r, g, b, 1); }

#undef color
Color color(f32 r, f32 g, f32 b, f32 a) { return Color{r, g, b, a}; }

/*================================================================================
// Deps
================================================================================*/

#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#endif

#define STBI_MALLOC(sz) mem_alloc(sz)
#define STBI_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBI_FREE(p) mem_free(p)

#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"

#define STBIR_MALLOC(size, user_data) ((void)(user_data), mem_alloc(size))
#define STBIR_FREE(ptr, user_data) ((void)(user_data), mem_free(ptr))

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "extern/stb_image_resize2.h"

#define STBIW_MALLOC(sz) mem_alloc(sz)
#define STBIW_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBIW_FREE(p) mem_free(p)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "extern/stb_rect_pack.h"

#define STBTT_malloc(x, u) ((void)(u), mem_alloc(x))
#define STBTT_free(x, u) ((void)(u), mem_free(x))

#define STB_TRUETYPE_IMPLEMENTATION
#include "extern/stb_truetype.h"

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
#define MA_MALLOC(sz) mem_alloc((sz))
#define MA_REALLOC(p, sz) mem_realloc((p), (sz))
#define MA_FREE(p) mem_free((p))
#include "extern/miniaudio.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
