#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#endif

#include "neko_os.h"

#define SOKOL_TIME_IMPL
#include "vendor/sokol_time.h"

#define STBI_MALLOC(sz) mem_alloc(sz)
#define STBI_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBI_FREE(p) mem_free(p)

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBIR_MALLOC(size, user_data) ((void)(user_data), mem_alloc(size))
#define STBIR_FREE(ptr, user_data) ((void)(user_data), mem_free(ptr))

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#define STBIW_MALLOC(sz) mem_alloc(sz)
#define STBIW_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBIW_FREE(p) mem_free(p)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>

#define STBTT_malloc(x, u) ((void)(u), mem_alloc(x))
#define STBTT_free(x, u) ((void)(u), mem_free(x))

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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
