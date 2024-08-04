//

#include "engine/neko_api_core.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko_game.h"
#include "engine/neko_math.h"
#include "engine/neko_os.h"
#include "engine/neko_prelude.h"

// sokol

// #include <sokol_gfx.h>

// #include "vendor/sokol_gp.h"

int neko_api_set_blendmode(neko_api_BlendMode blendmode) {
    // sgp_set_blend_mode((sgp_blend_mode)blendmode);
    g_app->blendmode = blendmode;
    return 0;
}

#ifndef MAX_CIRCLE_TRIS
#define MAX_CIRCLE_TRIS 1024
#endif

int neko_api_draw_point(f64 x, f64 y) {
    // sgp_draw_filled_rect(x, y, 1, 1);
    return 0;
}

int neko_api_draw_line(f64 x1, f64 y1, f64 x2, f64 y2) {
    // sgp_draw_line(x1, y1, x2, y2);
    return 0;
}

int neko_api_draw_triangle(f64 ax, f64 ay, f64 bx, f64 by, f64 cx, f64 cy) {
    // sgp_draw_filled_triangle(ax, ay, bx, by, cx, cy);
    return 0;
}

int neko_api_draw_triangle_line(f64 _ax, f64 _ay, f64 _bx, f64 _by, f64 _cx, f64 _cy) {
    f32 ax = _ax, bx = _bx, cx = _cx;
    f32 ay = _ay, by = _by, cy = _cy;
    // sgp_line lines[4] = {
    //         sgp_line{{ax, ay}, {bx, by}},
    //         sgp_line{{bx, by}, {cx, cy}},
    //         sgp_line{{cx, cy}, {ax, ay}},
    // };
    // sgp_draw_lines(lines, 4);
    return 0;
}

int neko_api_draw_rect(f64 x, f64 y, f64 w, f64 h) {
    // sgp_draw_filled_rect(x, y, w, h);
    return 0;
}

int neko_api_draw_rect_line(f64 _x, f64 _y, f64 _w, f64 _h) {
    f32 x = _x, y = _y, w = _w, h = _h;
    // sgp_line lines[4] = {
    //         sgp_line{{x, y}, {x, y + h}},
    //         sgp_line{{x, y + h}, {x + w, y + h}},
    //         sgp_line{{x + w, y + h}, {x + w, y}},
    //         sgp_line{{x + w, y}, {x, y + 1}},
    // };
    // sgp_draw_lines(lines, 4);
    return 0;
}

int neko_api_draw_circle(f64 dest_x, f64 dest_y, f64 radius) { return neko_api_draw_ellipse(dest_x, dest_y, radius, radius); }

int neko_api_draw_circle_line(f64 dest_x, f64 dest_y, f64 radius) { return neko_api_draw_ellipse_line(dest_x, dest_y, radius, radius); }

int neko_api_draw_ellipse(f64 dest_x, f64 dest_y, f64 radius_x, f64 radius_y) {
    // f32 x = (f32)dest_x;
    // f32 y = (f32)dest_y;
    // f32 r = (f32)((radius_x + radius_y) / 2.0);

    // f32 xr = (f32)radius_x / r;
    // f32 yr = (f32)radius_y / r;

    // static sgp_triangle tris[MAX_CIRCLE_TRIS];
    // int count = NEKO_MIN(NEKO_MAX(7.0f, 2.0f * r / (f32)neko_pi), MAX_CIRCLE_TRIS);
    // f32 delta_angle = 2.0 * (f32)neko_pi / (f32)count;

    // for (int i = 0; i < count; i++) {
    //     tris[i].a.x = x;
    //     tris[i].a.y = y;

    //     tris[i].b.x = x + r * xr * sinf(i * delta_angle);
    //     tris[i].b.y = y - r * yr * cosf(i * delta_angle);

    //     tris[i].c.x = x + r * xr * sinf((i + 1) * delta_angle);
    //     tris[i].c.y = y - r * yr * cosf((i + 1) * delta_angle);
    // }
    // sgp_draw_filled_triangles(tris, count);
    return 0;
}

int neko_api_draw_ellipse_line(f64 dest_x, f64 dest_y, f64 radius_x, f64 radius_y) {
    // f32 x = (f32)dest_x;
    // f32 y = (f32)dest_y;
    // f32 r = (f32)((radius_x + radius_y) / 2.0);

    // f32 xr = (f32)radius_x / r;
    // f32 yr = (f32)radius_y / r;

    // static sgp_line lines[MAX_CIRCLE_TRIS];
    // int count = NEKO_MIN(NEKO_MAX(7.0f, 2.0f * r / (f32)neko_pi), MAX_CIRCLE_TRIS);
    // f32 delta_angle = 2.0 * (f32)neko_pi / (f32)count;

    // for (int i = 0; i < count; i++) {
    //     lines[i].a.x = x + r * xr * sinf(i * delta_angle);
    //     lines[i].a.y = y - r * yr * cosf(i * delta_angle);

    //     lines[i].b.x = x + r * xr * sinf((i + 1) * delta_angle);
    //     lines[i].b.y = y - r * yr * cosf((i + 1) * delta_angle);
    // }
    // sgp_draw_lines(lines, count);
    return 0;
}

int neko_api_core_shader_init(void) { return 0; }

int neko_api_core_shader_cleanup(void) { return 0; }

// ShaderBuilder

int neko_api_shaderbuilder_new(neko_api_ShaderBuilder *val) {
    ShaderBuilderItem *sbi = (ShaderBuilderItem *)mem_alloc(sizeof(ShaderBuilderItem));
    sbi->uniform_definitions = NULL;
    sbi->num_uniform_definitions = 0;
    sbi->vert_code = NULL;
    sbi->frag_code = NULL;

    // val = (neko_api_ShaderBuilder *)&sbi;
    *val = sbi;
    return 0;
}

int neko_api_shaderbuilder_cleanup(neko_api_ShaderBuilder shaderbuilder) {
    ShaderBuilderItem *sbi = (ShaderBuilderItem *)shaderbuilder;
    if (sbi) {
        mem_free(sbi);
    }

    return 0;
}

static inline int get_uniform_count(neko_api_UniformType t) {
    int ret = 0;
    switch (t) {
        case NEKO_UNIFORMTYPE_FLOAT:
        case NEKO_UNIFORMTYPE_INT: {
            ret = 1;
        } break;
        case NEKO_UNIFORMTYPE_VEC2:
        case NEKO_UNIFORMTYPE_IVEC2: {
            ret = 2;
        } break;
        case NEKO_UNIFORMTYPE_VEC3:
        case NEKO_UNIFORMTYPE_IVEC3: {
            ret = 3;
        } break;
        case NEKO_UNIFORMTYPE_VEC4:
        case NEKO_UNIFORMTYPE_IVEC4: {
            ret = 4;
        } break;
        case NEKO_UNIFORMTYPE_MAT4: {
            ret = 16;
        } break;
        case NEKO_UNIFORMTYPE_SAMPLER2D: {
            ret = 0;
        } break;
    }
    return ret;
}

int neko_api_shaderbuilder_uniform(neko_api_ShaderBuilder shaderbuilder, const char *uniform_name, int utype) {
    neko_api_UniformType uniform_type = (neko_api_UniformType)utype;
    ShaderBuilderItem *sbi = (ShaderBuilderItem *)shaderbuilder;
    if (!sbi) {
        fprintf(stderr, "ShaderBuilder not found.");
        return 1;
    }
    int idx = sbi->num_uniform_definitions;
    int cnt = get_uniform_count(uniform_type);
    sbi->num_uniform_definitions++;
    sbi->uniform_definitions = (ShaderUniformDefinition *)mem_realloc(sbi->uniform_definitions, sbi->num_uniform_definitions * sizeof(ShaderBuilderItem));
    sbi->uniform_definitions[idx].name = uniform_name;
    sbi->uniform_definitions[idx].type = uniform_type;
    sbi->uniform_definitions[idx].float_count = cnt;
    sbi->uniform_definitions[idx].location = 0;

    return 0;
}

int neko_api_shaderbuilder_vertex(neko_api_ShaderBuilder shaderbuilder, const char *vertex_code) {
    ShaderBuilderItem *sbi = (ShaderBuilderItem *)shaderbuilder;
    if (!sbi) {
        fprintf(stderr, "ShaderBuilder not found.");
        return 1;
    }
    sbi->vert_code = vertex_code;

    return 0;
}

int neko_api_shaderbuilder_fragment(neko_api_ShaderBuilder shaderbuilder, const char *fragment_code) {
    ShaderBuilderItem *sbi = (ShaderBuilderItem *)shaderbuilder;
    if (!sbi) {
        fprintf(stderr, "ShaderBuilder not found.");
        return 1;
    }
    sbi->frag_code = fragment_code;

    return 0;
}

// Shader

#if !defined(__EMSCRIPTEN__)
static const char *shader_header = "\n\n#version 330\n// ---\n\n";
// static const char *shader_header = "#version 330 core\n// ---\n";
#else
static const char *shader_header = "#version 300 es\nprecision highp float;\n// ---\n";
// static const char *shader_header = "#version 300 es\nprecision mediump float;\n// ---\n";
// static const char *shader_header = "\n\n#version 300 es\n// ---\n\n";
#endif

static const char *vert_footer = "\n// ---\nvoid main() { vert_main(); }\n";
static const char *frag_footer = "\n// ---\nvoid main() { frag_main(); }\n";
static const char *frag_header_1 = "uniform sampler2D current_image;\n";
static const char *frag_header_2 = "uniform vec4 current_color;\n";
static const char *frag_sep = "// ---\n\n";

static inline char *get_full_uniforms_code(ShaderBuilderItem *sbi) {
    size_t num_uniforms = sbi->num_uniform_definitions;
    char *ret = (char *)mem_alloc(256 * (num_uniforms + 1));  // TODO: mem_free at neko_api_shaderbuilder_build
    sprintf(ret, "// uniform declarations...\n");
    for (int i = 0; i < num_uniforms; i++) {
        const char *type = NULL;
        neko_api_UniformType utype = sbi->uniform_definitions[i].type;
        switch (utype) {
            case NEKO_UNIFORMTYPE_FLOAT: {
                type = "float";
            } break;
            case NEKO_UNIFORMTYPE_VEC2: {
                type = "vec2";
            } break;
            case NEKO_UNIFORMTYPE_VEC3: {
                type = "vec3";
            } break;
            case NEKO_UNIFORMTYPE_VEC4: {
                type = "vec4";
            } break;
            case NEKO_UNIFORMTYPE_INT: {
                type = "int";
            } break;
            case NEKO_UNIFORMTYPE_IVEC2: {
                type = "ivec3";
            } break;
            case NEKO_UNIFORMTYPE_IVEC3: {
                type = "ivec3";
            } break;
            case NEKO_UNIFORMTYPE_IVEC4: {
                type = "ivec4";
            } break;
            case NEKO_UNIFORMTYPE_MAT4: {
                type = "mat4";
            } break;
            case NEKO_UNIFORMTYPE_SAMPLER2D: {
                type = "sampler2D";
            } break;
            default: {
                printf("Unknown uniform type: %d", utype);
                exit(1);
            } break;
        }
        const char *name = sbi->uniform_definitions[i].name;
        sprintf(ret + strlen(ret), "uniform %s %s;\n", type, name);
    }
    return ret;
}

static inline char *get_full_vertex_code(ShaderBuilderItem *sbi, const char *uniforms_text) {
    char *str = (char *)sbi->vert_code;
    size_t orig_len = strlen(str);
    size_t new_len = orig_len + 1000;
    char *ret = (char *)mem_alloc(new_len);  // TODO: mem_free at neko_api_shaderbuilder_build
    memset(ret, 0, new_len);
    sprintf(ret, "%s%s%s%s%s%s", shader_header, uniforms_text, frag_sep, str, frag_sep, vert_footer);
    return ret;
}

static inline char *get_full_fragment_code(ShaderBuilderItem *sbi, const char *uniforms_text) {
    char *str = (char *)sbi->frag_code;
    size_t orig_len = strlen(str);
    size_t new_len = orig_len + 1000;
    char *ret = (char *)mem_alloc(new_len);  // TODO: mem_free at neko_api_shaderbuilder_build
    memset(ret, 0, new_len);
    sprintf(ret, "%s%s%s%s%s%s%s%s", shader_header, frag_header_1, frag_header_2, frag_sep, uniforms_text, frag_sep, str, frag_footer);
    return ret;
}

int neko_api_shaderbuilder_build(neko_api_ShaderBuilder shaderbuilder, neko_api_Shader *shader) {
    ShaderBuilderItem *sbi = (ShaderBuilderItem *)shaderbuilder;
    if (!sbi) {
        fprintf(stderr, "ShaderBuilder not found.");
        return 1;
    }

    const char *uniforms_code = get_full_uniforms_code(sbi);
    if (!uniforms_code) {
        fprintf(stderr, "Uniforms code was not generated.");
        return 2;
    }
    const char *full_code_vertex = get_full_vertex_code(sbi, uniforms_code);
    if (!full_code_vertex) {
        fprintf(stderr, "Vertex code was not generated.");
        return 3;
    }
    const char *full_code_fragment = get_full_fragment_code(sbi, uniforms_code);
    if (!full_code_vertex) {
        fprintf(stderr, "Fragment code was not generated.");
        return 4;
    }

    ShaderItem *shd = (ShaderItem *)mem_alloc(sizeof(ShaderItem));

    // shd->uniform_floats = (float *)mem_alloc(SGP_UNIFORM_CONTENT_SLOTS * sizeof(float));  // TODO: mem_free in neko_api_cleanup_shader
    // memset(shd->uniform_floats, 0, SGP_UNIFORM_CONTENT_SLOTS * sizeof(float));
    // shd->images = (neko_api_Image *)mem_alloc(SGP_TEXTURE_SLOTS * sizeof(neko_api_Image));  // TODO: mem_free in neko_api_cleanup_shader
    // memset(shd->images, 0, SGP_TEXTURE_SLOTS * sizeof(neko_api_Image));

    // shd->num_uniform_definitions = sbi->num_uniform_definitions;
    // shd->uniform_definitions = (ShaderUniformDefinition *)mem_alloc(shd->num_uniform_definitions * sizeof(ShaderBuilderItem));
    // memcpy(shd->uniform_definitions, sbi->uniform_definitions, shd->num_uniform_definitions * sizeof(ShaderBuilderItem));  // TODO: mem_free in neko_api_cleanup_shader

    // // sg shader description

    // // init
    // sg_shader_desc shader_desc = {0};
    // shader_desc.label = "lyteshaderprogram";
    // shd->num_uniform_floats = 0;
    // shd->num_images = 0;

    // // shader code
    // shader_desc.vs.source = full_code_vertex;
    // shader_desc.fs.source = full_code_fragment;
    // shader_desc.vs.entry = "main";
    // shader_desc.fs.entry = "main";
    // // MAGIC uniform: vec4 current_color 和 sampler2D current_image
    // shader_desc.fs.uniform_blocks->uniforms[0].name = "current_color";
    // shader_desc.fs.uniform_blocks->uniforms[0].type = (sg_uniform_type)NEKO_UNIFORMTYPE_VEC4;
    // shader_desc.fs.uniform_blocks->uniforms[0].array_count = 1;
    // shader_desc.vs.uniform_blocks->uniforms[0].name = "current_color";
    // shader_desc.vs.uniform_blocks->uniforms[0].type = (sg_uniform_type)NEKO_UNIFORMTYPE_VEC4;
    // shader_desc.vs.uniform_blocks->uniforms[0].array_count = 1;

    // shd->num_uniform_floats += 4;  // r,g,b,a for current_color

    // // 如果用户调用 draw_image 这是对应的MAGIC图像名称
    // shader_desc.fs.image_sampler_pairs[0].glsl_name = "current_image";
    // shader_desc.fs.image_sampler_pairs[0].used = true;
    // shader_desc.fs.image_sampler_pairs[0].image_slot = 0;
    // shader_desc.fs.image_sampler_pairs[0].sampler_slot = 0;
    // shader_desc.fs.samplers[0].used = true;
    // shader_desc.fs.images[0].used = true;
    // shader_desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
    // shader_desc.fs.images[0].sample_type = SG_IMAGESAMPLETYPE_FLOAT;

    // shd->num_images += 1;  // current_image

    // size_t num_uniforms = sbi->num_uniform_definitions;

    // int flt_idx = 1;
    // int img_idx = 1;

    // for (int i = 0; i < num_uniforms; i++) {
    //     ShaderUniformDefinition *sud = &shd->uniform_definitions[i];
    //     if (sud->type == NEKO_UNIFORMTYPE_SAMPLER2D) {
    //         // image
    //         shader_desc.fs.image_sampler_pairs[img_idx].glsl_name = sud->name;
    //         shader_desc.fs.image_sampler_pairs[img_idx].used = true;
    //         shader_desc.fs.image_sampler_pairs[img_idx].image_slot = img_idx;
    //         shader_desc.fs.image_sampler_pairs[img_idx].sampler_slot = img_idx;
    //         shader_desc.fs.samplers[img_idx].used = true;
    //         shader_desc.fs.images[img_idx].used = true;
    //         shader_desc.fs.images[img_idx].image_type = SG_IMAGETYPE_2D;
    //         shader_desc.fs.images[img_idx].sample_type = SG_IMAGESAMPLETYPE_FLOAT;
    //         sud->location = img_idx;
    //         img_idx++;
    //         shd->num_images += 1;
    //     } else {
    //         shader_desc.fs.uniform_blocks[0].uniforms[flt_idx].name = sud->name;
    //         shader_desc.fs.uniform_blocks[0].uniforms[flt_idx].type = (sg_uniform_type)sud->type;
    //         shader_desc.fs.uniform_blocks[0].uniforms[flt_idx].array_count = 1;
    //         shader_desc.vs.uniform_blocks[0].uniforms[flt_idx].name = sud->name;
    //         shader_desc.vs.uniform_blocks[0].uniforms[flt_idx].type = (sg_uniform_type)sud->type;
    //         shader_desc.vs.uniform_blocks[0].uniforms[flt_idx].array_count = 1;
    //         flt_idx++;
    //         sud->location = shd->num_uniform_floats;
    //         shd->num_uniform_floats += sud->float_count;
    //     }
    // }

    // shader_desc.fs.uniform_blocks[0].size = shd->num_uniform_floats * 4;

    // // sg_shader
    // sg_shader sgshd = sg_make_shader(&shader_desc);
    // // sgp pipeline
    // sgp_pipeline_desc pip_desc = {0};
    // pip_desc.blend_mode = (sgp_blend_mode)g_app->blendmode;  // TODO: neko
    // pip_desc.shader = sgshd;
    // // pip_desc.primitive_type = SG_PRIMITIVETYPE_ ;
    // sg_pipeline pip = sgp_make_pipeline(&pip_desc);
    // sg_resource_state state = sg_query_pipeline_state(pip);
    // if (state != SG_RESOURCESTATE_VALID) {
    //     fprintf(stderr, "Failed to make custom pipeline: %d\n ", state);
    //     return 5;
    // }

    // // 将 sgp 管道与 Shaderitem 关联
    // shd->pip_id = pip.id;
    // shd->shd_id = sgshd.id;

    *shader = shd;

    // 这些在 get_full_* 函数中分配
    mem_free((void *)uniforms_code);
    mem_free((void *)full_code_vertex);
    mem_free((void *)full_code_fragment);

    return 0;
}

int neko_api_shader_cleanup(neko_api_Shader shader) {
    ShaderItem *shd = (ShaderItem *)shader;
    if (shd) {
        mem_free(shd->uniform_definitions);
        mem_free(shd->uniform_floats);
        mem_free(shd->images);
        // sg_destroy_pipeline(sg_pipeline{.id = shd->pip_id});
        // sg_destroy_shader(sg_shader{.id = shd->shd_id});
        mem_free(shd);
    }
    return 0;
}

int neko_api_shader_set(neko_api_Shader shader) {
    ShaderItem *shd = (ShaderItem *)shader;
    if (!shd) {
        fprintf(stderr, "Shader not found.");
        return 1;
    }
    // sgp_set_pipeline(sg_pipeline{.id = shd->pip_id});
    memcpy(shd->uniform_floats, g_app->current_color, 16);
    // sgp_set_uniform(shd->uniform_floats, shd->num_uniform_floats * 4);
    neko_api_set_blendmode(g_app->blendmode);
    g_app->shader = shd;

    return 0;
}

int neko_api_reset_shader(void) {
    g_app->shader = NULL;
    // sgp_reset_pipeline();

    return 0;
}

int neko_api_core_shader_set_color() {
    if (g_app->shader) {
        ShaderItem *shd = g_app->shader;
        memcpy(shd->uniform_floats, g_app->current_color, 16);
        // sgp_set_uniform(shd->uniform_floats, shd->num_uniform_floats * 4);
    }
    return 0;
}

int neko_api_shader_set_uniform(neko_api_Shader shader, const char *uniform_name, neko_api_ShaderUniformValue uniform_value) {
    ShaderItem *shd = (ShaderItem *)shader;
    if (!shd) {
        fprintf(stderr, "Shader not found.");
        return 1;
    }
    ShaderUniformDefinition *sud = NULL;
    for (int i = 0; i < shd->num_uniform_definitions; i++) {
        if (shd->uniform_definitions[i].name == uniform_name) {
            sud = &shd->uniform_definitions[i];
            break;
        }
    }
    if (!sud) {
        fprintf(stderr, "Unknown shader uniform: %s\n", uniform_name);
        return 2;
    }
    switch (uniform_value.which) {
        case 0: {  // float/int
            shd->uniform_floats[sud->location] = uniform_value.options.float_val;
            memcpy(shd->uniform_floats, g_app->current_color, 16);
            // sgp_set_uniform(shd->uniform_floats, shd->num_uniform_floats * 4);
        } break;
        case 1: {  // vecX/ivecX. X = 2 or 3 or 4
            if (uniform_value.options.vec_val.count > sud->float_count) {
                fprintf(stderr, "warning: too many uniform values sent\n");
            } else if (uniform_value.options.vec_val.count > sud->float_count) {
                fprintf(stderr, "warning: not enough uniform values sent\n");
            }
            int count = NEKO_MIN(NEKO_MAX(uniform_value.options.vec_val.count, sud->float_count), 4);
            for (int i = 0; i < count; i++) {
                shd->uniform_floats[sud->location + i] = uniform_value.options.vec_val.data[i];
            }
            memcpy(shd->uniform_floats, g_app->current_color, 16);
            // sgp_set_uniform(shd->uniform_floats, shd->num_uniform_floats * 4);
        } break;
        case 2: {  // sampler2D
            shd->images[sud->location] = uniform_value.options.sampler2D_val;
            uint32_t img_id = *(uint32_t *)uniform_value.options.sampler2D_val;  // **MAGIC_1** NOTE: uint32_t 句柄是 ImageItem 结构中的第一项
            // sg_image sgimg = sg_image{.id = img_id};
            // sgp_set_image(sud->location, sgimg);
        } break;
        default: {
            fprintf(stderr, "Unknown shader uniform value, shouldn't happen.\n");
            return -1;
        } break;
    }

    return 0;
}

int neko_api_shader_reset_uniform(neko_api_Shader shader, const char *uniform_name) {
    ShaderItem *shd = (ShaderItem *)shader;
    if (!shd) {
        fprintf(stderr, "Shader not found.");
        return 1;
    }
    ShaderUniformDefinition *sud = NULL;
    for (int i = 0; i < shd->num_uniform_definitions; i++) {
        if (shd->uniform_definitions[i].name == uniform_name) {
            sud = &shd->uniform_definitions[i];
            break;
        }
    }
    if (!sud) {
        fprintf(stderr, "Unknown shader uniform: %s\n", uniform_name);
        return 2;
    }

    return 0;
}
