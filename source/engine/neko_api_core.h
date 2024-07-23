
#ifndef NEKO_API_CORE_H
#define NEKO_API_CORE_H

#include <cstddef>
#include <cstdint>

#include "engine/neko_prelude.h"

typedef enum neko_api_UniformType {
    // values map to SOKOL GP
    NEKO_UNIFORMTYPE__INVALID = 0,    // order: 0, str: '_invalid'
    NEKO_UNIFORMTYPE_FLOAT = 1,       // order: 1, str: 'float'
    NEKO_UNIFORMTYPE_VEC2 = 2,        // order: 2, str: 'vec2'
    NEKO_UNIFORMTYPE_VEC3 = 3,        // order: 3, str: 'vec3'
    NEKO_UNIFORMTYPE_VEC4 = 4,        // order: 4, str: 'vec4'
    NEKO_UNIFORMTYPE_INT = 5,         // order: 5, str: 'int'
    NEKO_UNIFORMTYPE_IVEC2 = 6,       // order: 6, str: 'ivec2'
    NEKO_UNIFORMTYPE_IVEC3 = 7,       // order: 7, str: 'ivec3'
    NEKO_UNIFORMTYPE_IVEC4 = 8,       // order: 8, str: 'ivec4'
    NEKO_UNIFORMTYPE_MAT4 = 9,        // order: 9, str: 'mat4'
    NEKO_UNIFORMTYPE_SAMPLER2D = 10,  // order: 10, str: 'sampler2D'
    NEKO_UNIFORMTYPE_COUNT = 11,
    NEKO_UNIFORMTYPE_FORCE_UINT32 = 0x7FFFFFFF,  // inspired by sokol_gfx enums
} neko_api_UniformType;

typedef enum neko_api_BlendMode {
    // values map to SOKOL GP
    NEKO_BLENDMODE_NONE = 0,   // order: 0, str: 'none'
    NEKO_BLENDMODE_BLEND = 1,  // order: 1, str: 'blend'
    NEKO_BLENDMODE_ADD = 2,    // order: 2, str: 'add'
    NEKO_BLENDMODE_MOD = 3,    // order: 3, str: 'mod'
    NEKO_BLENDMODE_MUL = 4,    // order: 4, str: 'mul'
    NEKO_BLENDMODE_COUNT = 5,
    NEKO_BLENDMODE_FORCE_UINT32 = 0x7FFFFFFF,  // inspired by sokol_gfx enums
} neko_api_BlendMode;

typedef enum neko_api_FilterMode {
    // values map to SOKOL GP
    NEKO_FILTERMODE__INVALID = 1,  // order: 0, str: '_invalid'
    NEKO_FILTERMODE_NEAREST = 2,   // order: 1, str: 'nearest'
    NEKO_FILTERMODE_LINEAR = 3,    // order: 2, str: 'linear'
    NEKO_FILTERMODE_COUNT = 3,
    NEKO_FILTERMODE_FORCE_UINT32 = 0x7FFFFFFF,  // inspired by sokol_gfx enums
} neko_api_FilterMode;

typedef void *neko_api_Image;
typedef void *neko_api_ImageBatch;

typedef void *neko_api_ShaderBuilder;  // ShaderBuilderItem
typedef void *neko_api_Shader;         // ShaderItem

typedef struct neko_api_FloatVec4 {
    float data[4];
    size_t count;
} neko_api_FloatVec4;
typedef struct neko_api_ShaderUniformValue {
    int which;
    union {
        float float_val;
        neko_api_FloatVec4 vec_val;
        neko_api_Image sampler2D_val;
    } options;
} neko_api_ShaderUniformValue;

#ifndef INIT_NUM_SHADERBUILDERITEMS
#define INIT_NUM_SHADERBUILDERITEMS 10
#endif

#ifndef INIT_NUM_SHADERITEMS
#define INIT_NUM_SHADERITEMS 10
#endif

typedef struct ShaderUniformDefinition {
    const char *name;
    neko_api_UniformType type;
    int float_count;
    int location;
} ShaderUniformDefinition;

typedef struct ShaderBuilderItem {
    ShaderUniformDefinition *uniform_definitions;
    size_t num_uniform_definitions;

    const char *vert_code;
    const char *frag_code;

} ShaderBuilderItem;

typedef struct ShaderItem {

    // 定义 包括位置
    ShaderUniformDefinition *uniform_definitions;
    size_t num_uniform_definitions;

    // 实际浮点数
    float *uniform_floats;
    size_t num_uniform_floats;

    // 实际图像
    neko_api_Image *images;
    size_t num_images;

    uint32_t pip_id;
    uint32_t shd_id;
} ShaderItem;

int neko_api_draw_point(f64 x, f64 y);
int neko_api_draw_line(f64 x1, f64 y1, f64 x2, f64 y2);
int neko_api_draw_triangle(f64 ax, f64 ay, f64 bx, f64 by, f64 cx, f64 cy);
int neko_api_draw_triangle_line(f64 ax, f64 ay, f64 bx, f64 by, f64 cx, f64 cy);
int neko_api_draw_rect(f64 dest_x, f64 dest_y, f64 rect_width, f64 rect_height);
int neko_api_draw_rect_line(f64 dest_x, f64 dest_y, f64 rect_width, f64 rect_height);
int neko_api_draw_circle(f64 dest_x, f64 dest_y, f64 radius);
int neko_api_draw_circle_line(f64 dest_x, f64 dest_y, f64 radius);
int neko_api_draw_ellipse(f64 dest_x, f64 dest_y, f64 radius_x, f64 radius_y);
int neko_api_draw_ellipse_line(f64 dest_x, f64 dest_y, f64 radius_x, f64 radius_y);

// -------------------------
// core_shader
// -------------------------

int neko_api_core_shader_init(void);
int neko_api_core_shader_cleanup(void);
int neko_api_core_shader_set_color(void);

// ShaderBuilder
int neko_api_shaderbuilder_new(neko_api_ShaderBuilder *val);
int neko_api_shaderbuilder_cleanup(neko_api_ShaderBuilder shaderbuilder);

int neko_api_shaderbuilder_uniform(neko_api_ShaderBuilder shaderbuilder, const char *uniform_name, int utype);
int neko_api_shaderbuilder_vertex(neko_api_ShaderBuilder shaderbuilder, const char *vertex_code);
int neko_api_shaderbuilder_fragment(neko_api_ShaderBuilder shaderbuilder, const char *fragment_code);

// Shader
int neko_api_shaderbuilder_build(neko_api_ShaderBuilder shaderbuilder, neko_api_Shader *shader);
int neko_api_shader_cleanup(neko_api_Shader shader);

int neko_api_shader_set(neko_api_Shader shader);
int neko_api_reset_shader(void);
int neko_api_shader_set_uniform(neko_api_Shader shader, const char *uniform_name, neko_api_ShaderUniformValue uniform_value);
int neko_api_shader_reset_uniform(neko_api_Shader shader, const char *uniform_name);

inline int neko_api_shader_set_uniform_float(void *shader, const char *uniform_name, float uniform_value) {
    (void)shader;
    (void)uniform_name;
    (void)uniform_value;
    int err = 0;
    neko_api_ShaderUniformValue u = {0};
    u.options.float_val = uniform_value;
    u.which = 0;
    neko_api_shader_set_uniform(shader, uniform_name, u);
    return err;
}
inline int neko_api_shader_set_uniform_floatvec4(void *shader, const char *uniform_name, float *uniform_value, size_t uniform_value_count) {
    (void)shader;
    (void)uniform_name;
    (void)uniform_value;
    (void)uniform_value_count;
    int err = 0;
    neko_api_ShaderUniformValue u = {0};
    memcpy(u.options.vec_val.data, uniform_value, 4 * sizeof(float));
    u.options.vec_val.count = uniform_value_count;
    u.which = 1;
    neko_api_shader_set_uniform(shader, uniform_name, u);
    return err;
}
inline int neko_api_shader_set_uniform_image(void *shader, const char *uniform_name, void *uniform_value) {
    (void)shader;
    (void)uniform_name;
    (void)uniform_value;
    int err = 0;
    neko_api_ShaderUniformValue u = {0};
    u.options.sampler2D_val = uniform_value;
    u.which = 2;
    neko_api_shader_set_uniform(shader, uniform_name, u);
    return err;
}

#endif