
#ifndef NEKO_SHADER_H
#define NEKO_SHADER_H

#include "base/common/string.hpp"

#include "extern/glad/glad.h"

#include "engine/graphics.h"

namespace Neko {

typedef struct AssetShader {
    GLuint id;
    const char* name;
    bool panic_mode;
} AssetShader;

bool neko_load_shader(AssetShader* shader, String path);
void neko_unload_shader(AssetShader* shader);

NEKO_API() void neko_bind_shader(u32 shader);
NEKO_API() void neko_shader_set_int(u32 shader, const char* name, i32 v);
NEKO_API() void neko_shader_set_uint(u32 shader, const char* name, u32 v);
NEKO_API() void neko_shader_set_float(u32 shader, const char* name, float v);
NEKO_API() void neko_shader_set_color(u32 shader, const char* name, neko_color_t color);
NEKO_API() void neko_shader_set_rgb_color(u32 shader, const char* name, neko_rgb_color_t color);
NEKO_API() void neko_shader_set_v2f(u32 shader, const char* name, vec2 v);
NEKO_API() void neko_shader_set_v3f(u32 shader, const char* name, vec3 v);
NEKO_API() void neko_shader_set_v4f(u32 shader, const char* name, vec4 v);
NEKO_API() void neko_shader_set_m4f(u32 shader, const char* name, mat4 v);

}  // namespace Neko

#endif