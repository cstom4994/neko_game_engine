#include "gfx.h"

#include <stdio.h>
#include <stdlib.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/console.h"
#include "engine/game.h"

// deps
#include <stb_image.h>

static GLint gfx_compile_shader(GLuint shader, const char* filename) {
    char log[512];
    GLint status;

    String contents = {};

    bool ok = vfs_read_entire_file(&contents, filename);
    neko_defer(mem_free(contents.data));

    neko_assert(ok);

    console_printf("gfx: compiling shader '%s' ...", filename);

    glShaderSource(shader, 1, (const GLchar**)&contents.data, NULL);
    glCompileShader(shader);

    // log
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    console_printf(status ? " successful\n" : " unsuccessful\n");
    glGetShaderInfoLog(shader, 512, NULL, log);
    console_printf("%s", log);

    return status;
}

GLuint gfx_create_program(const_str name, const char* vert_path, const char* geom_path, const char* frag_path) {
    GLuint vert, geom, frag, program;

#define compile(shader, type)                                     \
    if (shader##_path) {                                          \
        shader = glCreateShader(type);                            \
        if (!gfx_compile_shader(shader, shader##_path)) return 0; \
    }

    compile(vert, GL_VERTEX_SHADER);
    compile(geom, GL_GEOMETRY_SHADER);
    compile(frag, GL_FRAGMENT_SHADER);

    program = glCreateProgram();

    if (vert_path) glAttachShader(program, vert);
    if (geom_path) glAttachShader(program, geom);
    if (frag_path) glAttachShader(program, frag);

    glLinkProgram(program);

    // GL will automatically detach and free shaders when program is deleted
    if (vert_path) glDeleteShader(vert);
    if (geom_path) glDeleteShader(geom);
    if (frag_path) glDeleteShader(frag);

    shader_pair pair = {program, name};
    neko_dyn_array_push(g_app->shader_array, pair);

    return program;
}

/*=============================
// R_IMPL
=============================*/

gfx_t* g_render;

#define RENDER() (g_render)

#if (defined NEKO_IS_WIN32 || defined NEKO_IS_APPLE || defined NEKO_IS_LINUX)
#define R_IMPL_OPENGL_CORE
#else
#define R_IMPL_OPENGL_ES
#endif

// macOS 不支持 OpenGL 4.1+
#if defined(NEKO_IS_APPLE)
#define CHECK_GL_CORE(...)
#else
#define CHECK_GL_CORE(...) __VA_ARGS__
#endif

// 图形信息对象查询
gfx_info_t* gfx_info() { return &RENDER()->info; }

const char* gl_get_error_description(GLenum error_code) {
    switch (error_code) {
        case GL_NO_ERROR:
            return "No error detected.";
        case GL_INVALID_ENUM:
            return "Enum argument out of range.";
        case GL_INVALID_VALUE:
            return "Numeric argument out of range.";
        case GL_INVALID_OPERATION:
            return "Operation illegal in current state.";
        case GL_OUT_OF_MEMORY:
            return "Not enough memory left to execute command.";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Framebuffer object is not complete.";
        default:
            return "No description available for UNKNOWN_ERROR.";
    }
}

void gfx_print_error(const char* file, u32 line) {
    GLenum code = glGetError();

    if (code != GL_NO_ERROR) {
        const_str last_slash = file;

        while (*file) {
            char c = *file;
            if (c == '\\' || c == '/') last_slash = file + 1;
            ++file;
        }

        const_str str = neko_opengl_string(code);
        const_str des = gl_get_error_description(code);
        neko_println("OpenGL Error %s (%u): %u, %s, %s", last_slash, line, code, str, des);
    }
}

void neko_gl_reset_data_cache(neko_gl_data_cache_t* cache) {
    cache->ibo.id = 0;
    cache->ibo_elem_sz = 0;
    cache->pipeline = neko_handle_invalid(gfx_pipeline_t);
    neko_dyn_array_clear(cache->vdecls);
}

void neko_gl_pipeline_state() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    CHECK_GL_CORE(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS););
    glDisable(GL_MULTISAMPLE);

    CHECK_GL_CORE(gfx_info_t* info = gfx_info(); if (info->compute.available) {
        NEKO_INVOKE_ONCE(console_log("compute shader available: %s", NEKO_BOOL_STR(info->compute.available)););
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    });
}

// Utilities
i32 neko_gl_buffer_usage_to_gl_enum(gfx_buffer_usage_type type) {
    i32 mode = GL_STATIC_DRAW;
    switch (type) {
        default:
        case R_BUFFER_USAGE_STATIC:
            mode = GL_STATIC_DRAW;
            break;
        case R_BUFFER_USAGE_STREAM:
            mode = GL_STREAM_DRAW;
            break;
        case R_BUFFER_USAGE_DYNAMIC:
            mode = GL_DYNAMIC_DRAW;
            break;
    }
    return mode;
}

u32 neko_gl_access_type_to_gl_access_type(gfx_access_type type) {
    CHECK_GL_CORE(u32 access = GL_WRITE_ONLY; switch (type) {
        case R_ACCESS_WRITE_ONLY:
            access = GL_WRITE_ONLY;
            break;
        case R_ACCESS_READ_ONLY:
            access = GL_READ_ONLY;
            break;
        case R_ACCESS_READ_WRITE:
            access = GL_READ_WRITE;
            break;
        default:
            break;
    } return access;)
    return 0;
}

u32 neko_gl_texture_wrap_to_gl_texture_wrap(gfx_texture_wrapping_type type) {
    u32 wrap = GL_REPEAT;
    switch (type) {
        default:
        case R_TEXTURE_WRAP_REPEAT:
            wrap = GL_REPEAT;
            break;
        case R_TEXTURE_WRAP_MIRRORED_REPEAT:
            wrap = GL_MIRRORED_REPEAT;
            break;
        case R_TEXTURE_WRAP_CLAMP_TO_EDGE:
            wrap = GL_CLAMP_TO_EDGE;
            break;
            CHECK_GL_CORE(case R_TEXTURE_WRAP_CLAMP_TO_BORDER : wrap = GL_CLAMP_TO_BORDER; break;)
    };

    return wrap;
}

u32 neko_gl_texture_format_to_gl_data_type(gfx_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        default:
        case R_TEXTURE_FORMAT_A8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_R8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_R32:
            format = GL_UNSIGNED_INT;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_RGBA8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_R32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_UNSIGNED_INT_24_8;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
    }
    return format;
}

u32 neko_gl_texture_format_to_gl_texture_format(gfx_texture_format_type type) {
    u32 dt = GL_RGBA;
    switch (type) {
        default:
        case R_TEXTURE_FORMAT_RGBA8:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_A8:
            dt = GL_ALPHA;
            break;
        case R_TEXTURE_FORMAT_R8:
            dt = GL_RED;
            break;
        case R_TEXTURE_FORMAT_R32:
            dt = GL_RED_INTEGER;
            break;
        case R_TEXTURE_FORMAT_R32F:
            dt = GL_RED;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            dt = GL_RGB;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
    }
    return dt;
}

u32 neko_gl_texture_format_to_gl_texture_internal_format(gfx_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        case R_TEXTURE_FORMAT_A8:
            format = GL_ALPHA;
            break;
        case R_TEXTURE_FORMAT_R8:
            format = GL_RED;
            break;
        case R_TEXTURE_FORMAT_R32:
            format = GL_R32UI;
            break;
        case R_TEXTURE_FORMAT_R32F:
            format = GL_R32F;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            format = GL_RGB8;
            break;
        case R_TEXTURE_FORMAT_RGBA8:
            format = GL_RGBA8;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            format = GL_RGBA16F;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            format = GL_RGBA32F;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            format = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            format = GL_DEPTH_COMPONENT16;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            format = GL_DEPTH_COMPONENT24;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            format = GL_DEPTH_COMPONENT32F;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_DEPTH24_STENCIL8;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_DEPTH32F_STENCIL8;
            break;
    }
    return format;
}

u32 neko_gl_shader_stage_to_gl_stage(gfx_shader_stage_type type) {
    u32 stage = GL_VERTEX_SHADER;
    switch (type) {
        default:
        case R_SHADER_STAGE_VERTEX:
            stage = GL_VERTEX_SHADER;
            break;
        case R_SHADER_STAGE_FRAGMENT:
            stage = GL_FRAGMENT_SHADER;
            break;
            CHECK_GL_CORE(case R_SHADER_STAGE_COMPUTE : stage = GL_COMPUTE_SHADER; break;)
    }
    return stage;
}

u32 neko_gl_primitive_to_gl_primitive(gfx_primitive_type type) {
    u32 prim = GL_TRIANGLES;
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES:
            prim = GL_TRIANGLES;
            break;
        case R_PRIMITIVE_LINES:
            prim = GL_LINES;
            break;
            CHECK_GL_CORE(case R_PRIMITIVE_QUADS : prim = GL_QUADS; break;)
    }
    return prim;
}

u32 neko_gl_blend_equation_to_gl_blend_eq(gfx_blend_equation_type eq) {
    u32 beq = GL_FUNC_ADD;
    switch (eq) {
        default:
        case R_BLEND_EQUATION_ADD:
            beq = GL_FUNC_ADD;
            break;
        case R_BLEND_EQUATION_SUBTRACT:
            beq = GL_FUNC_SUBTRACT;
            break;
        case R_BLEND_EQUATION_REVERSE_SUBTRACT:
            beq = GL_FUNC_REVERSE_SUBTRACT;
            break;
        case R_BLEND_EQUATION_MIN:
            beq = GL_MIN;
            break;
        case R_BLEND_EQUATION_MAX:
            beq = GL_MAX;
            break;
    };

    return beq;
}

u32 neko_gl_blend_mode_to_gl_blend_mode(gfx_blend_mode_type type, u32 def) {
    u32 mode = def;
    switch (type) {
        case R_BLEND_MODE_ZERO:
            mode = GL_ZERO;
            break;
        case R_BLEND_MODE_ONE:
            mode = GL_ONE;
            break;
        case R_BLEND_MODE_SRC_COLOR:
            mode = GL_SRC_COLOR;
            break;
        case R_BLEND_MODE_ONE_MINUS_SRC_COLOR:
            mode = GL_ONE_MINUS_SRC_COLOR;
            break;
        case R_BLEND_MODE_DST_COLOR:
            mode = GL_DST_COLOR;
            break;
        case R_BLEND_MODE_ONE_MINUS_DST_COLOR:
            mode = GL_ONE_MINUS_DST_COLOR;
            break;
        case R_BLEND_MODE_SRC_ALPHA:
            mode = GL_SRC_ALPHA;
            break;
        case R_BLEND_MODE_ONE_MINUS_SRC_ALPHA:
            mode = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case R_BLEND_MODE_DST_ALPHA:
            mode = GL_DST_ALPHA;
            break;
        case R_BLEND_MODE_ONE_MINUS_DST_ALPHA:
            mode = GL_ONE_MINUS_DST_ALPHA;
            break;
        case R_BLEND_MODE_CONSTANT_COLOR:
            mode = GL_CONSTANT_COLOR;
            break;
        case R_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR:
            mode = GL_ONE_MINUS_CONSTANT_COLOR;
            break;
        case R_BLEND_MODE_CONSTANT_ALPHA:
            mode = GL_CONSTANT_ALPHA;
            break;
        case R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA:
            mode = GL_ONE_MINUS_CONSTANT_ALPHA;
            break;
    }
    return mode;
}

u32 neko_gl_cull_face_to_gl_cull_face(gfx_face_culling_type type) {
    u32 fc = GL_BACK;
    switch (type) {
        default:
        case R_FACE_CULLING_BACK:
            fc = GL_BACK;
            break;
        case R_FACE_CULLING_FRONT:
            fc = GL_FRONT;
            break;
        case R_FACE_CULLING_FRONT_AND_BACK:
            fc = GL_FRONT_AND_BACK;
            break;
    }
    return fc;
}

u32 neko_gl_winding_order_to_gl_winding_order(gfx_winding_order_type type) {
    u32 wo = GL_CCW;
    switch (type) {
        case R_WINDING_ORDER_CCW:
            wo = GL_CCW;
            break;
        case R_WINDING_ORDER_CW:
            wo = GL_CW;
            break;
    }
    return wo;
}

u32 neko_gl_depth_func_to_gl_depth_func(gfx_depth_func_type type) {
    u32 func = GL_LESS;
    switch (type) {
        default:
        case R_DEPTH_FUNC_LESS:
            func = GL_LESS;
            break;
        case R_DEPTH_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case R_DEPTH_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case R_DEPTH_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case R_DEPTH_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case R_DEPTH_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case R_DEPTH_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case R_DEPTH_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

bool neko_gl_depth_mask_to_gl_mask(gfx_depth_mask_type type) {
    bool ret = true;
    switch (type) {
        default:
        case R_DEPTH_MASK_ENABLED:
            ret = true;
            break;
        case R_DEPTH_MASK_DISABLED:
            ret = false;
            break;
    }
    return ret;
}

u32 neko_gl_stencil_func_to_gl_stencil_func(gfx_stencil_func_type type) {
    u32 func = GL_ALWAYS;
    switch (type) {
        default:
        case R_STENCIL_FUNC_LESS:
            func = GL_LESS;
            break;
        case R_STENCIL_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case R_STENCIL_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case R_STENCIL_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case R_STENCIL_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case R_STENCIL_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case R_STENCIL_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case R_STENCIL_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

u32 neko_gl_stencil_op_to_gl_stencil_op(gfx_stencil_op_type type) {
    u32 op = GL_KEEP;
    switch (type) {
        default:
        case R_STENCIL_OP_KEEP:
            op = GL_KEEP;
            break;
        case R_STENCIL_OP_ZERO:
            op = GL_ZERO;
            break;
        case R_STENCIL_OP_REPLACE:
            op = GL_REPLACE;
            break;
        case R_STENCIL_OP_INCR:
            op = GL_INCR;
            break;
        case R_STENCIL_OP_INCR_WRAP:
            op = GL_INCR_WRAP;
            break;
        case R_STENCIL_OP_DECR:
            op = GL_DECR;
            break;
        case R_STENCIL_OP_DECR_WRAP:
            op = GL_DECR_WRAP;
            break;
        case R_STENCIL_OP_INVERT:
            op = GL_INVERT;
            break;
    }
    return op;
}

neko_gl_uniform_type neko_gl_uniform_type_to_gl_uniform_type(gfx_uniform_type gstype) {
    neko_gl_uniform_type type = NEKO_GL_UNIFORMTYPE_FLOAT;
    switch (gstype) {
        default:
        case R_UNIFORM_FLOAT:
            type = NEKO_GL_UNIFORMTYPE_FLOAT;
            break;
        case R_UNIFORM_INT:
            type = NEKO_GL_UNIFORMTYPE_INT;
            break;
        case R_UNIFORM_VEC2:
            type = NEKO_GL_UNIFORMTYPE_VEC2;
            break;
        case R_UNIFORM_VEC3:
            type = NEKO_GL_UNIFORMTYPE_VEC3;
            break;
        case R_UNIFORM_VEC4:
            type = NEKO_GL_UNIFORMTYPE_VEC4;
            break;
        case R_UNIFORM_MAT4:
            type = NEKO_GL_UNIFORMTYPE_MAT4;
            break;
        case R_UNIFORM_USAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case R_UNIFORM_SAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case R_UNIFORM_SAMPLERCUBE:
            type = NEKO_GL_UNIFORMTYPE_SAMPLERCUBE;
            break;
    }
    return type;
}

u32 neko_gl_index_buffer_size_to_gl_index_type(size_t sz) {
    u32 type = GL_UNSIGNED_INT;
    switch (sz) {
        default:
        case 4:
            type = GL_UNSIGNED_INT;
            break;
        case 2:
            type = GL_UNSIGNED_SHORT;
            break;
        case 1:
            type = GL_UNSIGNED_BYTE;
            break;
    }
    return type;
}

size_t neko_gl_get_byte_size_of_vertex_attribute(gfx_vertex_attribute_type type) {
    size_t byte_size = 0;
    switch (type) {
        case R_VERTEX_ATTRIBUTE_FLOAT4: {
            byte_size = sizeof(f32) * 4;
        } break;
        case R_VERTEX_ATTRIBUTE_FLOAT3: {
            byte_size = sizeof(f32) * 3;
        } break;
        case R_VERTEX_ATTRIBUTE_FLOAT2: {
            byte_size = sizeof(f32) * 2;
        } break;
        case R_VERTEX_ATTRIBUTE_FLOAT: {
            byte_size = sizeof(f32) * 1;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT4: {
            byte_size = sizeof(u32) * 4;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT3: {
            byte_size = sizeof(u32) * 3;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT2: {
            byte_size = sizeof(u32) * 2;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT: {
            byte_size = sizeof(u32) * 1;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE4: {
            byte_size = sizeof(uint8_t) * 4;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE3: {
            byte_size = sizeof(uint8_t) * 3;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE2: {
            byte_size = sizeof(uint8_t) * 2;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE: {
            byte_size = sizeof(uint8_t) * 1;
        } break;
    }

    return byte_size;
}

size_t neko_gl_calculate_vertex_size_in_bytes(gfx_vertex_attribute_desc_t* layout, u32 count) {
    // Iterate through all formats in delcarations and calculate total size
    size_t sz = 0;
    for (u32 i = 0; i < count; ++i) {
        gfx_vertex_attribute_type type = layout[i].format;
        sz += neko_gl_get_byte_size_of_vertex_attribute(type);
    }

    return sz;
}

size_t neko_gl_get_vertex_attr_byte_offest(neko_dyn_array(gfx_vertex_attribute_desc_t) layout, u32 idx) {
    // Recursively calculate offset
    size_t total_offset = 0;

    // Base case
    if (idx == 0) {
        return total_offset;
    }

    // Calculate total offset up to this point
    for (u32 i = 0; i < idx; ++i) {
        total_offset += neko_gl_get_byte_size_of_vertex_attribute(layout[i].format);
    }

    return total_offset;
}

size_t neko_gl_uniform_data_size_in_bytes(gfx_uniform_type type) {
    size_t sz = 0;
    switch (type) {
        case R_UNIFORM_FLOAT:
            sz = sizeof(float);
            break;
        case R_UNIFORM_INT:
            sz = sizeof(i32);
            break;
        case R_UNIFORM_VEC2:
            sz = 2 * sizeof(float);
            break;
        case R_UNIFORM_VEC3:
            sz = 3 * sizeof(float);
            break;
        case R_UNIFORM_VEC4:
            sz = 4 * sizeof(float);
            break;
        case R_UNIFORM_MAT4:
            sz = 16 * sizeof(float);
            break;
        case R_UNIFORM_SAMPLER2D:
            sz = sizeof(neko_handle(gfx_texture_t));
            break;  // handle size
        case R_UNIFORM_USAMPLER2D:
            sz = sizeof(neko_handle(gfx_texture_t));
            break;  // handle size
        case R_UNIFORM_SAMPLERCUBE:
            sz = sizeof(neko_handle(gfx_texture_t));
            break;  // handle size
        default: {
            sz = 0;
        } break;
    }
    return sz;
}

gfx_t* gfx_create() {
    // 构建新的图形界面
    gfx_t* gfx = neko_malloc_init(gfx_t);
    // 为OpenGL构建内部数据
    gfx->ud = neko_malloc_init(neko_gl_data_t);
    return gfx;
}

void gfx_fini(gfx_t* render) {
    // 释放所有资源 (假设它们已从GPU中释放)
    if (render == NULL) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)render->ud;

#define OGL_FREE_DATA(SA, T, FUNC)                                                                                    \
    do {                                                                                                              \
        for (neko_slot_array_iter it = 1; neko_slot_array_iter_valid(SA, it); neko_slot_array_iter_advance(SA, it)) { \
            neko_handle(T) hndl = NEKO_DEFAULT_VAL();                                                                 \
            hndl.id = it;                                                                                             \
            FUNC(hndl);                                                                                               \
        }                                                                                                             \
    } while (0)

    // Free all gl data
    if (ogl->pipelines) OGL_FREE_DATA(ogl->pipelines, gfx_pipeline_t, gfx_pipeline_fini);
    if (ogl->shaders) OGL_FREE_DATA(ogl->shaders, gfx_shader_t, gfx_shader_fini);
    if (ogl->vertex_buffers) OGL_FREE_DATA(ogl->vertex_buffers, gfx_vertex_buffer_t, gfx_vertex_buffer_fini);
    if (ogl->index_buffers) OGL_FREE_DATA(ogl->index_buffers, gfx_index_buffer_t, gfx_index_buffer_fini);
    if (ogl->renderpasses) OGL_FREE_DATA(ogl->renderpasses, gfx_renderpass_t, gfx_renderpass_fini);
    if (ogl->frame_buffers) OGL_FREE_DATA(ogl->frame_buffers, gfx_framebuffer_t, gfx_framebuffer_fini);
    if (ogl->textures) OGL_FREE_DATA(ogl->textures, gfx_texture_t, gfx_texture_fini);
    if (ogl->uniforms) OGL_FREE_DATA(ogl->uniforms, gfx_uniform_t, gfx_uniform_fini);
    if (ogl->uniform_buffers) OGL_FREE_DATA(ogl->uniform_buffers, gfx_uniform_buffer_t, gfx_uniform_buffer_fini);
    // if (ogl->storage_buffers)   OGL_FREE_DATA(ogl->storage_buffers, gfx_storage_buffer_t, gfx_storage_buffer_fini);

    neko_slot_array_free(ogl->shaders);
    neko_slot_array_free(ogl->vertex_buffers);
    neko_slot_array_free(ogl->index_buffers);
    neko_slot_array_free(ogl->frame_buffers);
    neko_slot_array_free(ogl->uniforms);
    neko_slot_array_free(ogl->textures);
    neko_slot_array_free(ogl->pipelines);
    neko_slot_array_free(ogl->renderpasses);
    neko_slot_array_free(ogl->uniform_buffers);
    neko_slot_array_free(ogl->storage_buffers);

    // Free uniform data array
    neko_dyn_array_free(ogl->uniform_data.mat4);
    neko_dyn_array_free(ogl->uniform_data.vec4);
    neko_dyn_array_free(ogl->uniform_data.vec3);
    neko_dyn_array_free(ogl->uniform_data.vec2);
    neko_dyn_array_free(ogl->uniform_data.flt);
    neko_dyn_array_free(ogl->uniform_data.i32);
    neko_dyn_array_free(ogl->uniform_data.ui32);

    // Free data cache
    neko_dyn_array_free(ogl->cache.vdecls);

    mem_free(ogl);

    mem_free(render);
    render = NULL;
}

neko_gl_texture_t gl_texture_update_internal(const gfx_texture_desc_t* desc, u32 hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    neko_gl_texture_t tex = NEKO_DEFAULT_VAL();
    if (hndl) tex = neko_slot_array_get(ogl->textures, hndl);
    u32 width = desc->width;
    u32 height = desc->height;
    void* data = desc->data;

    if (!hndl) {
        glGenTextures(1, &tex.id);
    }

    GLenum target = GL_TEXTURE_2D;

    // glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, tex.id);

    if (tex.desc.width * tex.desc.height < width * height) {
        // Construct texture based on appropriate format
        switch (desc->format) {
            case R_TEXTURE_FORMAT_A8:
                glTexImage2D(target, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R8:
                glTexImage2D(target, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32:
                glTexImage2D(target, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                break;
            case R_TEXTURE_FORMAT_RG8:
                glTexImage2D(target, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGB8:
                glTexImage2D(target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGBA8:
                glTexImage2D(target, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32F:
                glTexImage2D(target, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA16F:
                glTexImage2D(target, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA32F:
                glTexImage2D(target, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH8:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH16:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                glTexImage2D(target, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                glTexImage2D(target, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                break;

            // NOTE: 因为 Apple 是一家垃圾公司 所以必须将其分开并仅提供对 4.1 功能的支持。
            // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
            default:
                break;
        }
    } else {
        // Subimage
        switch (desc->format) {
            case R_TEXTURE_FORMAT_A8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32:
                glTexImage2D(target, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                break;
            case R_TEXTURE_FORMAT_RG8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RG, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGB8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGBA8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32F:
                glTexImage2D(target, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA16F:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA32F:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH16:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                break;

            // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
            // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
            default:
                break;
        }
    }

    i32 mag_filter = desc->mag_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    i32 min_filter = desc->min_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == R_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        } else {
            min_filter = desc->mip_filter == R_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    const u32 texture_wrap_s = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_s);
    const u32 texture_wrap_t = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_t);
    const u32 texture_wrap_r = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_r);

    if (desc->num_mips) {
        glGenerateMipmap(target);
    }

    // 在使用之前需要确保它可用
    CHECK_GL_CORE(float aniso = 0.0f; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso); glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso););

    glTexParameteri(target, GL_TEXTURE_WRAP_S, texture_wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, texture_wrap_t);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, texture_wrap_r);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);

    // Unbind buffers
    glBindTexture(target, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Set description
    tex.desc = *desc;

    // Add texture to internal resource pool and return handle
    return tex;
}

// Resource Creation
neko_handle(gfx_texture_t) gfx_texture_create_impl(const gfx_texture_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_texture_t tex = gl_texture_update_internal(&desc, 0);
    // Add texture to internal resource pool and return handle
    return (neko_handle_create(gfx_texture_t, neko_slot_array_insert(ogl->textures, tex)));
}

neko_handle(gfx_uniform_t) gfx_uniform_create_impl(const gfx_uniform_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // Assert if data isn't named
    if (desc.name == NULL) {
        console_log("Uniform must be named for OpenGL.");
        return neko_handle_invalid(gfx_uniform_t);
    }

    u32 ct = !desc.layout ? 0 : !desc.layout_size ? 1 : (u32)desc.layout_size / (u32)sizeof(gfx_uniform_layout_desc_t);
    if (ct < 1) {
        console_log("Uniform layout description must not be empty for: %s.", desc.name);
        return neko_handle_invalid(gfx_uniform_t);
    }

    // Construct list for uniform handles
    neko_gl_uniform_list_t ul = NEKO_DEFAULT_VAL();
    memcpy(ul.name, desc.name, 64);

    // Iterate layout, construct individual handles
    for (u32 i = 0; i < ct; ++i) {
        // Uniform to fill out
        neko_gl_uniform_t u = NEKO_DEFAULT_VAL();
        // Cache layout
        gfx_uniform_layout_desc_t* layout = &desc.layout[i];

        memcpy(u.name, layout->fname, 64);
        u.type = neko_gl_uniform_type_to_gl_uniform_type(layout->type);
        u.size = neko_gl_uniform_data_size_in_bytes(layout->type);
        u.count = layout->count ? layout->count : 1;
        u.location = UINT32_MAX;

        // Add to size of ul
        ul.size += u.size * u.count;

        // Push uniform into list
        neko_dyn_array_push(ul.uniforms, u);
    }

    return neko_handle_create(gfx_uniform_t, neko_slot_array_insert(ogl->uniforms, ul));
}

neko_handle(gfx_vertex_buffer_t) gfx_vertex_buffer_create_impl(const gfx_vertex_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_handle(gfx_vertex_buffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_buffer_t buffer = {0};

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc.usage == R_BUFFER_USAGE_STATIC && !desc.data) {
        neko_println("Error: Vertex buffer desc must contain data when R_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &buffer.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    glBufferData(GL_ARRAY_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(gfx_vertex_buffer_t, neko_slot_array_insert(ogl->vertex_buffers, buffer));

    return hndl;
}

neko_handle(gfx_index_buffer_t) gfx_index_buffer_create_impl(const gfx_index_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_handle(gfx_index_buffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_buffer_t buffer = {0};

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc.usage == R_BUFFER_USAGE_STATIC && !desc.data) {
        neko_println("Error: Index buffer desc must contain data when R_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &buffer.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(gfx_index_buffer_t, neko_slot_array_insert(ogl->index_buffers, buffer));

    return hndl;
}

neko_handle(gfx_uniform_buffer_t) gfx_uniform_buffer_create_impl(const gfx_uniform_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_handle(gfx_uniform_buffer_t) hndl = NEKO_DEFAULT_VAL();

    // Assert if data isn't named
    if (desc.name == NULL) {
        console_log("Uniform buffer must be named for Opengl.");
    }

    neko_gl_uniform_buffer_t u = NEKO_DEFAULT_VAL();
    memcpy(u.name, desc.name, 64);
    u.size = desc.size;
    u.location = UINT32_MAX;

    // Generate buffer (if needed)
    glGenBuffers(1, &u.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, u.ubo);
    glBufferData(GL_UNIFORM_BUFFER, u.size, 0, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    hndl = neko_handle_create(gfx_uniform_buffer_t, neko_slot_array_insert(ogl->uniform_buffers, u));

    return hndl;
}

neko_handle(gfx_storage_buffer_t) gfx_storage_buffer_create_impl(const gfx_storage_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_handle(gfx_storage_buffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_storage_buffer_t sbo = NEKO_DEFAULT_VAL();

    if (desc.name == NULL) {
        console_log("Storage buffer must be named for Opengl.");
    }

    if (desc.usage == R_BUFFER_USAGE_STATIC && !desc.data) {
        neko_println("Error: Storage buffer desc must contain data when R_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &sbo.buffer);

    // CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo.buffer); glBufferData(GL_SHADER_STORAGE_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    //               glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););

    // CHECK_GL_CORE(
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo.buffer);

    // 检查 desc 标志以映射缓冲区
    GLbitfield flags = 0x00;
    if (desc.flags & R_BUFFER_FLAG_MAP_PERSISTENT) {
        flags |= GL_MAP_PERSISTENT_BIT;
    }
    if (desc.flags & R_BUFFER_FLAG_MAP_COHERENT) {
        flags |= GL_MAP_PERSISTENT_BIT;
        flags |= GL_MAP_COHERENT_BIT;
    }
    if (desc.access & R_ACCESS_READ_ONLY) {
        flags |= GL_MAP_READ_BIT;
    } else if (desc.access & R_ACCESS_WRITE_ONLY) {
        flags |= GL_MAP_WRITE_BIT;
    } else if (desc.access & R_ACCESS_READ_WRITE) {
        flags |= (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    }

    GLbitfield store_flags = flags;
    if (desc.usage == R_BUFFER_USAGE_DYNAMIC) {
        store_flags |= GL_DYNAMIC_STORAGE_BIT;
    }

    // 现在只进行读/写访问
    // flags |= GL_MAP_READ_BIT; flags |= GL_MAP_WRITE_BIT;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, desc.size, desc.data, store_flags);
    // glBufferData(GL_SHADER_STORAGE_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // 存储映射
    if (flags & GL_MAP_PERSISTENT_BIT) {
        sbo.map = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, desc.size, flags);
    }

    GLenum err = glGetError();
    while ((err = glGetError()) != GL_NO_ERROR) {
        neko_println("GL ERROR: 0x%x: %s", err, glGetString(err));
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // );

    memcpy(sbo.name, desc.name, 64);
    sbo.access = desc.access;
    sbo.size = desc.size;
    sbo.block_idx = UINT32_MAX;

    hndl = neko_handle_create(gfx_storage_buffer_t, neko_slot_array_insert(ogl->storage_buffers, sbo));

    return hndl;
}

neko_handle(gfx_framebuffer_t) gfx_framebuffer_create_impl(const gfx_framebuffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_handle(gfx_framebuffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_buffer_t buffer = {0};
    glGenFramebuffers(1, &buffer.id);
    hndl = neko_handle_create(gfx_framebuffer_t, neko_slot_array_insert(ogl->frame_buffers, buffer));
    return hndl;
}

#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX 0x01
#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE 0x02
#define NEKO_GL_GRAPHICS_MAX_SID 128

neko_handle(gfx_shader_t) gfx_shader_create_impl(const gfx_shader_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_shader_t shader = {0};
    u32 pip = 0x00;

    u32 sid_ct = 0;
    u32 sids[NEKO_GL_GRAPHICS_MAX_SID] = NEKO_DEFAULT_VAL();

    // Create shader program
    shader.id = glCreateProgram();

    u32 ct = (u32)desc.size / (u32)sizeof(gfx_shader_source_desc_t);
    for (u32 i = 0; i < ct; ++i) {
        if (desc.sources[i].type == R_SHADER_STAGE_VERTEX) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX;
        if (desc.sources[i].type == R_SHADER_STAGE_COMPUTE) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE;

        // Validity Check: Desc must have vertex source if compute not selected. All other source is optional.
        if ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE) && ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX))) {
            neko_println("Error: Cannot have compute and render stages for shader program.");
            neko_assert(false);
        }

        u32 stage = neko_gl_shader_stage_to_gl_stage(desc.sources[i].type);
        u32 sid = glCreateShader(stage);

        if (!sid) {
            neko_println("Error: Failed to allocate memory for shader: '%s': stage: {put stage id here}", desc.name);
            neko_assert(sid);
        }

        // Set source
        glShaderSource(sid, 1, &desc.sources[i].source, NULL);

        // Compile shader
        glCompileShader(sid);

        // Check for errors
        GLint success = 0;
        glGetShaderiv(sid, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE) {
            GLint max_len = 0;
            glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &max_len);

            char* log = (char*)mem_alloc(max_len);
            memset(log, 0, max_len);

            // The max_len includes the NULL character
            glGetShaderInfoLog(sid, max_len, &max_len, log);

            // Delete shader.
            glDeleteShader(shader.id);

            // Provide the infolog
            neko_println("Opengl::opengl_compile_shader::shader: '%s'\nFAILED_TO_COMPILE: %s\n %s", desc.name, log, desc.sources[i].source);

            mem_free(log);
            log = NULL;

            // neko_assert(false);
            return neko_handle_invalid(gfx_shader_t);
        }

        // Attach shader to program
        glAttachShader(shader.id, sid);

        // Add to shader array
        sids[sid_ct++] = sid;
    }

    // Link shaders into final program
    glLinkProgram(shader.id);

    // Create info log for errors
    i32 is_good = 0;
    glGetProgramiv(shader.id, GL_LINK_STATUS, (i32*)&is_good);
    if (is_good == GL_FALSE) {
        GLint max_len = 0;
        glGetProgramiv(shader.id, GL_INFO_LOG_LENGTH, &max_len);

        char* log = (char*)mem_alloc(max_len);
        memset(log, 0, max_len);
        glGetProgramInfoLog(shader.id, max_len, &max_len, log);

        // Print error
        neko_panic("Fail To Link::opengl_link_shaders::shader: '%s', \n%s", desc.name, log);

        // //We don't need the program anymore.
        glDeleteProgram(shader.id);

        mem_free(log);
        log = NULL;

        // Just assert for now
        neko_assert(false);
    }

    glValidateProgram(shader.id);
    glGetProgramiv(shader.id, GL_VALIDATE_STATUS, &is_good);
    if (!is_good) {
        neko_panic("Failed to validate shader: '%s'", desc.name);
    }

    // Free shaders after use
    for (u32 i = 0; i < sid_ct; ++i) {
        glDeleteShader(sids[i]);
    }

    // Iterate over uniforms
    /*
    {
        char tmp_name[256] = NEKO_DEFAULT_VAL();
        i32 count = 0;
        glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &count);
        neko_println("Active Uniforms: %d\n", count);

        for (u32 i = 0; i < count; i++) {
            i32 sz = 0;
            u32 type;
            glGetActiveUniform(shader, (GLuint)i, 256, NULL, &sz, &type, tmp_name);
            neko_println("Uniform #%d Type: %u the_name: %s\n", i, type, tmp_name);
        }
    }
    */

    // Add to pool and return handle
    return (neko_handle_create(gfx_shader_t, neko_slot_array_insert(ogl->shaders, shader)));
}

neko_handle(gfx_renderpass_t) gfx_renderpass_create_impl(const gfx_renderpass_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    neko_gl_renderpass_t pass = NEKO_DEFAULT_VAL();

    // Set fbo
    pass.fbo = desc.fbo;

    // Set color attachments
    u32 ct = (u32)desc.color_size / (u32)sizeof(neko_handle(gfx_texture_t));
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pass.color, desc.color[i]);
    }
    // Set depth attachment
    pass.depth = desc.depth;

    // Create handle and return
    return (neko_handle_create(gfx_renderpass_t, neko_slot_array_insert(ogl->renderpasses, pass)));
}

neko_handle(gfx_pipeline_t) gfx_pipeline_create_impl(const gfx_pipeline_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    neko_gl_pipeline_t pipe = NEKO_DEFAULT_VAL();

    // Add states
    pipe.blend = desc.blend;
    pipe.depth = desc.depth;
    pipe.raster = desc.raster;
    pipe.stencil = desc.stencil;
    pipe.compute = desc.compute;

    // Add layout
    u32 ct = (u32)desc.layout.size / (u32)sizeof(gfx_vertex_attribute_desc_t);
    neko_dyn_array_reserve(pipe.layout, ct);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pipe.layout, desc.layout.attrs[i]);
    }

    // Create handle and return
    return (neko_handle_create(gfx_pipeline_t, neko_slot_array_insert(ogl->pipelines, pipe)));
}

// Resource Destruction
void gfx_texture_fini_impl(neko_handle(gfx_texture_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) return;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    glDeleteTextures(1, &tex->id);
    neko_slot_array_erase(ogl->textures, hndl.id);
}

void gfx_uniform_fini_impl(neko_handle(gfx_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->uniforms, hndl.id)) return;
    neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, hndl.id);
    neko_dyn_array_free(ul->uniforms);
    neko_slot_array_erase(ogl->uniforms, hndl.id);
}

void gfx_shader_fini_impl(neko_handle(gfx_shader_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->shaders, hndl.id)) return;
    glDeleteProgram(neko_slot_array_get(ogl->shaders, hndl.id).id);
    neko_slot_array_erase(ogl->shaders, hndl.id);
}

void gfx_vertex_buffer_fini_impl(neko_handle(gfx_vertex_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->vertex_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glDeleteBuffers(1, &buffer.id);
    neko_slot_array_erase(ogl->vertex_buffers, hndl.id);
}

void gfx_index_buffer_fini_impl(neko_handle(gfx_index_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->index_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    glDeleteBuffers(1, &buffer.id);
    neko_slot_array_erase(ogl->index_buffers, hndl.id);
}

void gfx_uniform_buffer_fini_impl(neko_handle(gfx_uniform_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->uniform_buffers, hndl.id)) return;
    neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &u->ubo);

    // Delete from slot array
    neko_slot_array_erase(ogl->uniform_buffers, hndl.id);
}

void gfx_storage_buffer_fini_impl(neko_handle(gfx_storage_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) return;
    neko_gl_storage_buffer_t* sb = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &sb->buffer);

    // Delete from slot array
    neko_slot_array_erase(ogl->storage_buffers, hndl.id);
}

void gfx_framebuffer_fini_impl(neko_handle(gfx_framebuffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->frame_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->frame_buffers, hndl.id);
    glDeleteFramebuffers(1, &buffer.id);
    neko_slot_array_erase(ogl->frame_buffers, hndl.id);
}

void gfx_renderpass_fini_impl(neko_handle(gfx_renderpass_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->renderpasses, hndl.id)) return;
    // TODO: erase all color attachments from renderpasss
    neko_slot_array_erase(ogl->renderpasses, hndl.id);
}

void gfx_pipeline_fini_impl(neko_handle(gfx_pipeline_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->pipelines, hndl.id)) return;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Free layout
    neko_dyn_array_free(pip->layout);

    // Erase handles from slot arrays
    neko_slot_array_erase(ogl->pipelines, hndl.id);
}

// Resource Query
void gfx_pipeline_desc_query(neko_handle(gfx_pipeline_t) hndl, gfx_pipeline_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Add states
    out->blend = pip->blend;
    out->depth = pip->depth;
    out->raster = pip->raster;
    out->stencil = pip->stencil;
    out->compute = pip->compute;

    // Add layout
    u32 ct = neko_dyn_array_size(pip->layout);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(out->layout.attrs, pip->layout[i]);
    }
}

void gfx_texture_desc_query(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);

    // Read back pixels
    if (out->data && out->read.width && out->read.height) {
        u32 type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
        u32 format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
        CHECK_GL_CORE(glActiveTexture(GL_TEXTURE0); glGetTextureSubImage(tex->id, 0, out->read.x, out->read.y, 0, out->read.width, out->read.height, 1, format, type, out->read.size, out->data););
    }

    *out = tex->desc;
}

size_t gfx_uniform_size_query(neko_handle(gfx_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_uniform_list_t* u = neko_slot_array_getp(ogl->uniforms, hndl.id);
    return u->uniforms[0].size;
}

// Resource Updates (main thread only)
void gfx_texture_update_impl(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) {
    if (!desc) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        console_log("Texture handle invalid: %zu", hndl.id);
        return;
    }
    gl_texture_update_internal(desc, hndl.id);
}

void gfx_vertex_buffer_update_impl(neko_handle(gfx_vertex_buffer_t) hndl, gfx_vertex_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    i32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    switch (desc->update.type) {
        case R_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx_index_buffer_update_impl(neko_handle(gfx_index_buffer_t) hndl, gfx_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    i32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
    switch (desc->update.type) {
        case R_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void gfx_storage_buffer_update_impl(neko_handle(gfx_storage_buffer_t) hndl, gfx_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);
    if (!sbo) {
        console_log("Storage buffer %zu not found.", hndl.id);
        return;
    }

    // CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo->buffer); switch (desc->update.type) {
    //     case R_BUFFER_UPDATE_SUBDATA:
    //         glBufferSubData(GL_SHADER_STORAGE_BUFFER, desc->update.offset, desc->size, desc->data);
    //         break;
    //     default:
    //         glBufferData(GL_SHADER_STORAGE_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    // } glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    //               glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););
}

void gfx_texture_read_impl(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!desc) return;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        console_log("Texture handle invalid: %zu", hndl.id);
    }

    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    // Bind texture
    GLenum target = GL_TEXTURE_2D;
    u32 gl_format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
    u32 gl_type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
    glBindTexture(target, tex->id);
    glReadPixels(desc->read.x, desc->read.y, desc->read.width, desc->read.height, gl_format, gl_type, desc->data);
    glBindTexture(target, 0x00);
}

void* gfx_storage_buffer_map_get_impl(neko_handle(gfx_storage_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) {
        console_log("Storage buffer handle invalid: %zu", hndl.id);
        return NULL;
    }
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);
    return sbo->map;
}

void gfx_storage_buffer_unlock_impl(neko_handle(gfx_storage_buffer_t) hndl) {
    // 解锁并返回映射指针
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) {
        console_log("Storage buffer handle invalid: %zu", hndl.id);
        return;
    }
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // 已经解锁
    if (sbo->sync) {
        while (1) {
            GLenum wait = glClientWaitSync(sbo->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
                break;
            }
        }
    }
}

void* gfx_storage_buffer_lock_impl(neko_handle(gfx_storage_buffer_t) hndl) {
    // 锁
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) {
        console_log("Storage buffer handle invalid: %zu", hndl.id);
        return NULL;
    }
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // 已经锁定
    if (sbo->sync) {
        glDeleteSync(sbo->sync);
    }
    sbo->sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    if (sbo->sync) {
        while (1) {
            GLenum wait = glClientWaitSync(sbo->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
                break;
            }
        }
    }
    return sbo->map;
}

#define __ogl_push_command(CB, OP_CODE, ...)                      \
    do {                                                          \
        neko_gl_data_t* DATA = (neko_gl_data_t*)RENDER()->ud;     \
        neko_byte_buffer_write(&CB->commands, u32, (u32)OP_CODE); \
        __VA_ARGS__                                               \
        CB->num_commands++;                                       \
    } while (0)

// Command Buffer Ops: Pipeline / Pass / Bind / Draw
void gfx_renderpass_begin(command_buffer_t* cb, neko_handle(gfx_renderpass_t) hndl) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_BEGIN_RENDER_PASS, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

void gfx_renderpass_end(command_buffer_t* cb) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_END_RENDER_PASS,
                       {
                               // 没事...
                       });
}

void gfx_clear(command_buffer_t* cb, gfx_clear_action_t action) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_CLEAR, {
        u32 count = 1;
        neko_byte_buffer_write(&cb->commands, u32, count);
        neko_byte_buffer_write(&cb->commands, gfx_clear_action_t, action);
    });
}

void gfx_clear_ex(command_buffer_t* cb, gfx_clear_desc_t* desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_CLEAR, {
        u32 count = !desc->actions ? 0 : !desc->size ? 1 : (u32)((size_t)desc->size / (size_t)sizeof(gfx_clear_action_t));
        neko_byte_buffer_write(&cb->commands, u32, count);
        for (u32 i = 0; i < count; ++i) {
            neko_byte_buffer_write(&cb->commands, gfx_clear_action_t, desc->actions[i]);
        }
    });
}

void gfx_set_viewport(command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEWPORT, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

void gfx_set_view_scissor(command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEW_SCISSOR, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

void gfx_texture_request_update(command_buffer_t* cb, neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t desc) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE);
    cb->num_commands++;

    u32 num_comps = 0;
    size_t data_type_size = 0;
    size_t total_size = 0;
    switch (desc.format) {
        default:
        case R_TEXTURE_FORMAT_RGBA8:
            num_comps = 4;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_RGB8:
            num_comps = 3;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_A8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_R8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_R32:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(float) + sizeof(uint8_t);
            break;

            // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
            // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
    }
    total_size = desc.width * desc.height * num_comps * data_type_size;
    neko_byte_buffer_write(&cb->commands, u32, hndl.id);
    neko_byte_buffer_write(&cb->commands, gfx_texture_desc_t, desc);
    neko_byte_buffer_write(&cb->commands, size_t, total_size);
    neko_byte_buffer_write_bulk(&cb->commands, desc.data, total_size);
}

void __gfx_update_buffer_internal(command_buffer_t* cb, u32 id, gfx_buffer_type type, gfx_buffer_usage_type usage, size_t sz, size_t offset, gfx_buffer_update_type update_type, void* data) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
    cb->num_commands++;

    // Write handle id
    neko_byte_buffer_write(&cb->commands, u32, id);
    // Write type
    neko_byte_buffer_write(&cb->commands, gfx_buffer_type, type);
    // Write usage
    neko_byte_buffer_write(&cb->commands, gfx_buffer_usage_type, usage);
    // Write data size
    neko_byte_buffer_write(&cb->commands, size_t, sz);
    // Write data offset
    neko_byte_buffer_write(&cb->commands, size_t, offset);
    // Write data update type
    neko_byte_buffer_write(&cb->commands, gfx_buffer_update_type, update_type);
    // Write data
    neko_byte_buffer_write_bulk(&cb->commands, data, sz);
}

void gfx_vertex_buffer_request_update(command_buffer_t* cb, neko_handle(gfx_vertex_buffer_t) hndl, gfx_vertex_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // 如果hndl无效则返回
    if (!hndl.id) return;

    __gfx_update_buffer_internal(cb, hndl.id, R_BUFFER_VERTEX, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

void gfx_index_buffer_request_update(command_buffer_t* cb, neko_handle(gfx_index_buffer_t) hndl, gfx_index_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // 如果hndl无效则返回
    if (!hndl.id) return;

    __gfx_update_buffer_internal(cb, hndl.id, R_BUFFER_INDEX, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

void gfx_uniform_buffer_request_update(command_buffer_t* cb, neko_handle(gfx_uniform_buffer_t) hndl, gfx_uniform_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // 如果hndl无效则返回
    if (!hndl.id) return;

    __gfx_update_buffer_internal(cb, hndl.id, R_BUFFER_UNIFORM, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

void gfx_storage_buffer_request_update(command_buffer_t* cb, neko_handle(gfx_storage_buffer_t) hndl, gfx_storage_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // 如果hndl无效则返回
    if (!hndl.id) return;

    __gfx_update_buffer_internal(cb, hndl.id, R_BUFFER_SHADER_STORAGE, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

void gfx_apply_bindings(command_buffer_t* cb, gfx_bind_desc_t* binds) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // Increment commands
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_APPLY_BINDINGS);
    cb->num_commands++;

    // __ogl_push_command(cb, NEKO_OPENGL_OP_APPLY_BINDINGS,
    {
        // Get counts from buffers
        u32 vct = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(gfx_bind_vertex_buffer_desc_t) : 1 : 0;
        u32 ict = binds->index_buffers.desc ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(gfx_bind_index_buffer_desc_t) : 1 : 0;
        u32 uct = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(gfx_bind_uniform_buffer_desc_t) : 1 : 0;
        u32 pct = binds->uniforms.desc ? binds->uniforms.size ? binds->uniforms.size / sizeof(gfx_bind_uniform_desc_t) : 1 : 0;
        u32 ibc = binds->image_buffers.desc ? binds->image_buffers.size ? binds->image_buffers.size / sizeof(gfx_bind_image_buffer_desc_t) : 1 : 0;
        u32 sbc = binds->storage_buffers.desc ? binds->storage_buffers.size ? binds->storage_buffers.size / sizeof(gfx_bind_storage_buffer_desc_t) : 1 : 0;

        // Determine total count to write into command buffer
        u32 ct = vct + ict + uct + pct + ibc + sbc;
        neko_byte_buffer_write(&cb->commands, u32, ct);

        // Determine if need to clear any previous vertex buffers (if vct != 0)
        neko_byte_buffer_write(&cb->commands, bool, (vct != 0));

        // Vertex buffers
        for (u32 i = 0; i < vct; ++i) {
            gfx_bind_vertex_buffer_desc_t* decl = &binds->vertex_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, gfx_bind_type, R_BIND_VERTEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, size_t, decl->offset);
            neko_byte_buffer_write(&cb->commands, gfx_vertex_data_type, decl->data_type);
        }

        // Index buffers
        for (u32 i = 0; i < ict; ++i) {
            gfx_bind_index_buffer_desc_t* decl = &binds->index_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, gfx_bind_type, R_BIND_INDEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
        }

        // Uniform buffers
        for (u32 i = 0; i < uct; ++i) {
            gfx_bind_uniform_buffer_desc_t* decl = &binds->uniform_buffers.desc[i];

            u32 id = decl->buffer.id;
            size_t sz = (size_t)(neko_slot_array_getp(ogl->uniform_buffers, id))->size;
            neko_byte_buffer_write(&cb->commands, gfx_bind_type, R_BIND_UNIFORM_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.offset);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.size);
        }

        // Image buffers
        for (u32 i = 0; i < ibc; ++i) {
            gfx_bind_image_buffer_desc_t* decl = &binds->image_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, gfx_bind_type, R_BIND_IMAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->tex.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, gfx_access_type, decl->access);
        }

        // Uniforms
        for (u32 i = 0; i < pct; ++i) {
            gfx_bind_uniform_desc_t* decl = &binds->uniforms.desc[i];

            // Get size from uniform list
            size_t sz = neko_slot_array_getp(ogl->uniforms, decl->uniform.id)->size;
            neko_byte_buffer_write(&cb->commands, gfx_bind_type, R_BIND_UNIFORM);
            neko_byte_buffer_write(&cb->commands, u32, decl->uniform.id);
            neko_byte_buffer_write(&cb->commands, size_t, sz);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write_bulk(&cb->commands, decl->data, sz);
        }

        // Storage buffers
        CHECK_GL_CORE(for (u32 i = 0; i < sbc; ++i) {
            gfx_bind_storage_buffer_desc_t* decl = &binds->storage_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, gfx_bind_type, R_BIND_STORAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
        });
    };
}

void gfx_pipeline_bind(command_buffer_t* cb, neko_handle(gfx_pipeline_t) hndl) {
    // TODO: 不确定这将来是否安全 因为管道的数据位于主线程上 并且可能会在单独的线程上被篡改
    __ogl_push_command(cb, NEKO_OPENGL_OP_BIND_PIPELINE, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

void gfx_draw(command_buffer_t* cb, gfx_draw_desc_t desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW, {
        neko_byte_buffer_write(&cb->commands, u32, desc.start);
        neko_byte_buffer_write(&cb->commands, u32, desc.count);
        neko_byte_buffer_write(&cb->commands, u32, desc.instances);
        neko_byte_buffer_write(&cb->commands, u32, desc.base_vertex);
        neko_byte_buffer_write(&cb->commands, u32, desc.range.start);
        neko_byte_buffer_write(&cb->commands, u32, desc.range.end);
    });
}

void gfx_draw_func(command_buffer_t* cb, R_DRAW_FUNC draw_func) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW_FUNC, { neko_byte_buffer_write(&cb->commands, R_DRAW_FUNC, draw_func); });
}

void gfx_dispatch_compute(command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DISPATCH_COMPUTE, {
        neko_byte_buffer_write(&cb->commands, u32, num_x_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_y_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_z_groups);
    });
}

// 提交绘制命令 (Main Thread)
void gfx_cmd_submit(command_buffer_t* cb) {
    /*
        // Structure of command:
            - Op code
            - Data packet
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    // Set read position of buffer to beginning
    neko_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    NEKO_FOR_RANGE(cb->num_commands) {
        // Read in op code of command
        neko_byte_buffer_readc(&cb->commands, neko_opengl_op_code_type, op_code);

        switch (op_code) {
            case NEKO_OPENGL_OP_BEGIN_RENDER_PASS: {
                // Bind render pass stuff
                neko_byte_buffer_readc(&cb->commands, u32, rpid);

                // If render pass exists, then we'll bind frame buffer and attachments
                if (rpid && neko_slot_array_exists(ogl->renderpasses, rpid)) {
                    neko_gl_renderpass_t* rp = neko_slot_array_getp(ogl->renderpasses, rpid);

                    // Bind frame buffer since it actually exists
                    if (rp->fbo.id && neko_slot_array_exists(ogl->frame_buffers, rp->fbo.id)) {
                        // Bind frame buffer
                        glBindFramebuffer(GL_FRAMEBUFFER, neko_slot_array_get(ogl->frame_buffers, rp->fbo.id).id);

                        // Bind color attachments
                        for (u32 r = 0; r < neko_dyn_array_size(rp->color); ++r) {
                            u32 cid = rp->color[r].id;
                            if (cid && neko_slot_array_exists(ogl->textures, cid)) {
                                neko_gl_texture_t* rt = neko_slot_array_getp(ogl->textures, cid);

                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + r, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }

                        // Bind depth attachment
                        {
                            u32 depth_id = rp->depth.id;
                            if (depth_id && neko_slot_array_exists(ogl->textures, depth_id)) {
                                neko_gl_texture_t* rt = neko_slot_array_getp(ogl->textures, depth_id);
                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }
                    }
                }
            } break;

            case NEKO_OPENGL_OP_END_RENDER_PASS: {
                neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
                neko_gl_reset_data_cache(&ogl->cache);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0););
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
            } break;

            case NEKO_OPENGL_OP_CLEAR: {
                // Actions
                neko_byte_buffer_readc(&cb->commands, u32, action_count);
                for (u32 j = 0; j < action_count; ++j) {
                    neko_byte_buffer_readc(&cb->commands, gfx_clear_action_t, action);

                    // No clear
                    if (action.flag & R_CLEAR_NONE) {
                        continue;
                    }

                    u32 bit = 0x00;

                    if (action.flag & R_CLEAR_COLOR || action.flag == 0x00) {
                        glClearColor(action.color[0], action.color[1], action.color[2], action.color[3]);
                        bit |= GL_COLOR_BUFFER_BIT;
                    }
                    if (action.flag & R_CLEAR_DEPTH || action.flag == 0x00) {
                        bit |= GL_DEPTH_BUFFER_BIT;
                    }
                    if (action.flag & R_CLEAR_STENCIL || action.flag == 0x00) {
                        bit |= GL_STENCIL_BUFFER_BIT;
                        glStencilMask(~0);
                    }

                    glClear(bit);
                }
            } break;

            case NEKO_OPENGL_OP_SET_VIEWPORT: {
                neko_byte_buffer_readc(&cb->commands, u32, x);
                neko_byte_buffer_readc(&cb->commands, u32, y);
                neko_byte_buffer_readc(&cb->commands, u32, w);
                neko_byte_buffer_readc(&cb->commands, u32, h);

                glViewport(x, y, w, h);
            } break;

            case NEKO_OPENGL_OP_SET_VIEW_SCISSOR: {
                neko_byte_buffer_readc(&cb->commands, u32, x);
                neko_byte_buffer_readc(&cb->commands, u32, y);
                neko_byte_buffer_readc(&cb->commands, u32, w);
                neko_byte_buffer_readc(&cb->commands, u32, h);

                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
            } break;

            case NEKO_OPENGL_OP_APPLY_BINDINGS: {
                neko_byte_buffer_readc(&cb->commands, u32, ct);

                // Determine if need to clear any previous vertex buffers here
                neko_byte_buffer_readc(&cb->commands, bool, clear_vertex_buffers);

                // Clear previous vertex decls if necessary
                if (clear_vertex_buffers) {
                    neko_dyn_array_clear(ogl->cache.vdecls);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                for (u32 i = 0; i < ct; ++i) {
                    neko_byte_buffer_readc(&cb->commands, gfx_bind_type, type);
                    switch (type) {
                        case R_BIND_VERTEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            neko_byte_buffer_readc(&cb->commands, size_t, offset);
                            neko_byte_buffer_readc(&cb->commands, gfx_vertex_data_type, data_type);

                            if (!id || !neko_slot_array_exists(ogl->vertex_buffers, id)) {
                                NEKO_TIMED_ACTION(1000, {
                                    console_log("Opengl:BindBindings:VertexBuffer %d does not exist.", id);
                                    continue;
                                });
                            }

                            // Grab vbo to bind
                            neko_gl_buffer_t vbo = neko_slot_array_get(ogl->vertex_buffers, id);

                            // If the data type is non-interleaved, then push size into vertex buffer decl
                            neko_gl_vertex_buffer_decl_t vbo_decl = NEKO_DEFAULT_VAL();
                            vbo_decl.vbo = vbo;
                            vbo_decl.data_type = data_type;
                            vbo_decl.offset = offset;

                            // Cache vertex buffer for later use
                            neko_dyn_array_push(ogl->cache.vdecls, vbo_decl);

                        } break;

                        case R_BIND_INDEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);

                            if (!neko_slot_array_exists(ogl->index_buffers, id)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Opengl:BindBindings:IndexBuffer %d does not exist.", id);
                                });
                                */
                            } else {
                                neko_gl_buffer_t ibo = neko_slot_array_get(ogl->index_buffers, id);

                                // Store in cache
                                ogl->cache.ibo.id = id;
                            }
                        } break;

                        case R_BIND_UNIFORM: {
                            // Get size from uniform list
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read data size for uniform list
                            neko_byte_buffer_readc(&cb->commands, size_t, sz);
                            // Read binding from uniform list (could make this a binding list? not sure how to handle this)
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniforms, id)) {
                                NEKO_TIMED_ACTION(1000, { console_log("Bind Uniform:Uniform %d does not exist.", id); });
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                NEKO_TIMED_ACTION(1000, { console_log("Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. If UINT32_T max, then must construct and place location
                            for (u32 ui = 0; ui < neko_dyn_array_size(ul->uniforms); ++ui) {
                                neko_gl_uniform_t* u = &ul->uniforms[ui];

                                // Searching for location if not bound or sid doesn't match previous use
                                if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) {
                                    if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {

                                        NEKO_TIMED_ACTION(1000, { console_log("Bind Uniform:Shader %d does not exist.", sid); });

                                        // Advance by size of uniform
                                        neko_byte_buffer_advance_position(&cb->commands, sz);
                                        continue;
                                    }

                                    neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                    // Construct temp name, concat with base name + uniform field name
                                    char name[256] = NEKO_DEFAULT_VAL();
                                    memcpy(name, ul->name, 64);
                                    if (u->name) {
                                        neko_snprintfc(UTMP, 256, "%s%s", ul->name, u->name);
                                        memcpy(name, UTMP, 256);
                                    }

                                    // Grab location of uniform based on name
                                    u->location = glGetUniformLocation(shader.id, name ? name : "__EMPTY_UNIFORM_NAME");

                                    if (u->location >= UINT32_MAX) {
                                        console_log("Bind Uniform: Uniform not found: \"%s\"", name);
                                        u->location = UINT32_MAX - 1;
                                    }

                                    u->sid = pip->raster.shader.id;
                                }

                                // Switch on uniform type to upload data
                                switch (u->type) {
                                    case NEKO_GL_UNIFORMTYPE_FLOAT: {
                                        // Need to read bulk data for array.
                                        neko_assert(u->size == sizeof(float));
                                        neko_dyn_array_clear(ogl->uniform_data.flt);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, float, v);
                                            neko_dyn_array_push(ogl->uniform_data.flt, v);
                                        }
                                        glUniform1fv(u->location, ct, ogl->uniform_data.flt);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_INT: {
                                        neko_assert(u->size == sizeof(i32));
                                        neko_dyn_array_clear(ogl->uniform_data.i32);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, i32, v);
                                            neko_dyn_array_push(ogl->uniform_data.i32, v);
                                        }
                                        glUniform1iv(u->location, ct, ogl->uniform_data.i32);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC2: {
                                        neko_assert(u->size == sizeof(vec2));
                                        neko_dyn_array_clear(ogl->uniform_data.vec2);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, vec2, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec2, v);
                                        }
                                        glUniform2fv(u->location, ct, (float*)ogl->uniform_data.vec2);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC3: {
                                        neko_assert(u->size == sizeof(vec3));
                                        neko_dyn_array_clear(ogl->uniform_data.vec3);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, vec3, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec3, v);
                                        }
                                        glUniform3fv(u->location, ct, (float*)ogl->uniform_data.vec3);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC4: {
                                        neko_assert(u->size == sizeof(vec4));
                                        neko_dyn_array_clear(ogl->uniform_data.vec4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, vec4, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec4, v);
                                        }
                                        glUniform4fv(u->location, ct, (float*)ogl->uniform_data.vec4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_MAT4: {
                                        neko_assert(u->size == sizeof(mat4));
                                        neko_dyn_array_clear(ogl->uniform_data.mat4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, mat4, v);
                                            neko_dyn_array_push(ogl->uniform_data.mat4, v);
                                        }
                                        glUniformMatrix4fv(u->location, ct, false, (float*)ogl->uniform_data.mat4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_SAMPLERCUBE:
                                    case NEKO_GL_UNIFORMTYPE_SAMPLER2D: {
                                        neko_assert(u->size == sizeof(neko_handle(gfx_texture_t)));
                                        u32 ct = u->count ? u->count : 1;
                                        i32 binds[128] = NEKO_DEFAULT_VAL();
                                        for (u32 i = 0; (i < ct && i < 128); ++i)  // Max of 128 texture binds. Get real.
                                        {
                                            neko_byte_buffer_read_bulkc(&cb->commands, neko_handle(gfx_texture_t), v, u->size);

                                            // Get texture, also need binding, but will worry about that in a bit
                                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, v.id);

                                            // Activate texture slot
                                            glActiveTexture(GL_TEXTURE0 + binding);

                                            // Bind texture
                                            GLenum target = GL_TEXTURE_2D;
                                            glBindTexture(target, tex->id);

                                            binds[i] = (i32)binding++;
                                        }

                                        // Bind uniforms
                                        glUniform1iv(u->location, ct, (i32*)binds);

                                    } break;

                                    default: {
                                        // Shouldn't hit here
                                        neko_println("Assert: Bind Uniform: Invalid uniform type specified.");
                                        neko_assert(false);
                                    } break;
                                }
                            }

                        } break;

                        case R_BIND_UNIFORM_BUFFER: {
                            // Read slot id of uniform buffer
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read binding
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            // Read range offset
                            neko_byte_buffer_readc(&cb->commands, size_t, range_offset);
                            // Read range size
                            neko_byte_buffer_readc(&cb->commands, size_t, range_size);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniform_buffers, id)) {
                                NEKO_TIMED_ACTION(1000, { console_log("Bind Uniform Buffer:Uniform %d does not exist.", id); });
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                NEKO_TIMED_ACTION(1000, { neko_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location.
                            // If UINT32_T max, then must construct and place location, or if shader id doesn't match previously used shader with this uniform
                            // TODO: To avoid constant lookups in this case, allow for shaders to hold uniform handles references instead.
                            if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) {
                                if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {
                                    NEKO_TIMED_ACTION(1000, { console_log("Bind Uniform Buffer:Shader %d does not exist.", sid); });
                                    continue;
                                }

                                neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                // Get uniform location based on name and bound shader
                                u->location = glGetUniformBlockIndex(shader.id, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                // Set binding for uniform block
                                glUniformBlockBinding(shader.id, u->location, binding);

                                if (u->location >= UINT32_MAX) {
                                    console_log("Bind Uniform Buffer: Uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }

                                u->sid = pip->raster.shader.id;
                            }

                            glBindBufferRange(GL_UNIFORM_BUFFER, binding, u->ubo, range_offset, range_size ? range_size : u->size);

                        } break;

                        case R_BIND_STORAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, sb_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Grab storage buffer from id
                            if (!sb_slot_id || !neko_slot_array_exists(ogl->storage_buffers, sb_slot_id)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Storage Buffer %d does not exist.", sb_slot_id);
                                });
                                */
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Pipeline %d does not exist or is not bound.", ogl->cache.pipeline.id);
                                });
                                */
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, sb_slot_id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                });
                                */
                                continue;
                            }

                            neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                            if ((sbo->block_idx == UINT32_MAX && sbo->block_idx != UINT32_MAX - 1)) {
                                // Get uniform location based on name and bound shader
                                // CHECK_GL_CORE({
                                //     sbo->block_idx = glGetProgramResourceIndex(shader, GL_SHADER_STORAGE_BLOCK, sbo->name ? sbo->name : "__EMPTY_BUFFER_NAME");
                                //     i32 params[1];
                                //     GLenum props[1] = {GL_BUFFER_BINDING};
                                //     glGetProgramResourceiv(shader, GL_SHADER_STORAGE_BLOCK, sbo->block_idx, 1, props, 1, NULL, params);
                                //     sbo->location = (u32)params[0];
                                //     console_log("Bind Storage Buffer: Binding \"%s\" to location %u, block index: %u, binding: %u", sbo->name, sbo->location, sbo->block_idx, binding);
                                // });

                                if (sbo->block_idx >= UINT32_MAX) {
                                    console_log("Bind Storage Buffer: Buffer not found: \"%s\"", sbo->name);
                                    sbo->block_idx = UINT32_MAX - 1;
                                }
                            }

                            if (sbo->block_idx < UINT32_MAX - 1) {
                                // Not sure what this actually does atm...
                                // CHECK_GL_CORE(glShaderStorageBlockBinding(shader, sbo->block_idx, sbo->location););
                            }

                            // This is required
                            CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, sbo->buffer););

                        } break;

                        case R_BIND_IMAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            neko_byte_buffer_readc(&cb->commands, gfx_access_type, access);

                            // Grab texture from sampler id
                            if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                                NEKO_TIMED_ACTION(1000, { console_log("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                                continue;
                            }

                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                            u32 gl_access = neko_gl_access_type_to_gl_access_type(access);
                            u32 gl_format = neko_gl_texture_format_to_gl_texture_internal_format(tex->desc.format);

                            NEKO_TIMED_ACTION(
                                    60, gfx_info_t* info = gfx_info(); if (info->major_version < 4) {
                                        neko_panic("%s", "OpenGL4 not available, failed to call glBindImageTexture.");
                                        continue;
                                    });

                            // Bind image texture
                            CHECK_GL_CORE(glBindImageTexture(binding, tex->id, 0, GL_FALSE, 0, gl_access, gl_format);)
                        } break;

                        default:
                            neko_assert(false);
                            break;
                    }
                }

            } break;

            case NEKO_OPENGL_OP_BIND_PIPELINE: {
                // Bind pipeline stuff
                neko_byte_buffer_readc(&cb->commands, u32, pipid);

                // Make sure pipeline exists
                if (!pipid || !neko_slot_array_exists(ogl->pipelines, pipid)) {

                    NEKO_TIMED_ACTION(1000, { console_log("Pipeline %d does not exist.", pipid); });

                    continue;
                }

                // Reset cache
                neko_gl_reset_data_cache(&ogl->cache);

                // Reset state as well
                neko_gl_pipeline_state();

                // Cache pipeline id
                ogl->cache.pipeline = neko_handle_create(gfx_pipeline_t, pipid);

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, pipid);

                // Compute
                // Early out if compute, since we're not doing a rasterization stage
                if (pip->compute.shader.id) {
                    // Shader
                    if (pip->compute.shader.id && neko_slot_array_exists(ogl->shaders, pip->compute.shader.id)) {
                        glUseProgram(neko_slot_array_get(ogl->shaders, pip->compute.shader.id).id);
                    } else {
                        NEKO_TIMED_ACTION(1000, { console_log("Opengl:BindPipeline:Compute:Shader %d does not exist.", pip->compute.shader.id); });
                    }

                    continue;
                }

                // Depth
                if (!pip->depth.func) {
                    // If no depth function (default), then disable
                    glDisable(GL_DEPTH_TEST);
                } else {
                    glEnable(GL_DEPTH_TEST);
                    glDepthFunc(neko_gl_depth_func_to_gl_depth_func(pip->depth.func));
                }
                glDepthMask(neko_gl_depth_mask_to_gl_mask(pip->depth.mask));

                // Stencil
                if (!pip->stencil.func) {
                    // If no stencil function (default), then disable
                    glDisable(GL_STENCIL_TEST);
                } else {
                    glEnable(GL_STENCIL_TEST);
                    u32 func = neko_gl_stencil_func_to_gl_stencil_func(pip->stencil.func);
                    u32 sfail = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.sfail);
                    u32 dpfail = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.dpfail);
                    u32 dppass = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.dppass);
                    glStencilFunc(func, pip->stencil.ref, pip->stencil.comp_mask);
                    glStencilMask(pip->stencil.write_mask);
                    glStencilOp(sfail, dpfail, dppass);
                }

                // Blend
                if (!pip->blend.func) {
                    glDisable(GL_BLEND);
                } else {
                    glEnable(GL_BLEND);
                    glBlendEquation(neko_gl_blend_equation_to_gl_blend_eq(pip->blend.func));
                    glBlendFunc(neko_gl_blend_mode_to_gl_blend_mode(pip->blend.src, GL_ONE), neko_gl_blend_mode_to_gl_blend_mode(pip->blend.dst, GL_ZERO));
                }

                // Raster
                // Face culling
                if (!pip->raster.face_culling) {
                    glDisable(GL_CULL_FACE);
                } else {
                    glEnable(GL_CULL_FACE);
                    glCullFace(neko_gl_cull_face_to_gl_cull_face(pip->raster.face_culling));
                }

                // Winding order
                glFrontFace(neko_gl_winding_order_to_gl_winding_order(pip->raster.winding_order));

                // Shader
                if (pip->raster.shader.id && neko_slot_array_exists(ogl->shaders, pip->raster.shader.id)) {
                    glUseProgram(neko_slot_array_get(ogl->shaders, pip->raster.shader.id).id);
                } else {
                    NEKO_TIMED_ACTION(1000, { console_log("Opengl:BindPipeline:Shader %d does not exist.", pip->raster.shader.id); });
                }
            } break;

            case NEKO_OPENGL_OP_DISPATCH_COMPUTE: {
                neko_byte_buffer_readc(&cb->commands, u32, num_x_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_y_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_z_groups);

                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                if (ogl->cache.pipeline.id == 0 || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                    /*
                    NEKO_TIMED_ACTION(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // If pipeline does not have a compute state bound, then leave
                if (!pip->compute.shader.id) {
                    /*
                    NEKO_TIMED_ACTION(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                // Dispatch shader
                CHECK_GL_CORE(
                        // Memory barrier (TODO: make this specifically set in the pipeline state)
                        glDispatchCompute(num_x_groups, num_y_groups, num_z_groups); glMemoryBarrier(GL_ALL_BARRIER_BITS);)
            } break;

            case NEKO_OPENGL_OP_DRAW: {
                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // Must have a vertex buffer bound to draw
                if (neko_dyn_array_empty(ogl->cache.vdecls)) {
                    NEKO_TIMED_ACTION(1000, { neko_println("Error:Opengl:Draw: No vertex buffer bound."); });
                    // neko_assert(false);
                }

                // Keep track whether or not the data is to be instanced
                bool is_instanced = false;

                for (u32 i = 0; i < neko_dyn_array_size(pip->layout); ++i) {
                    // Vertex buffer to bind
                    u32 vbo_idx = i;  // pip->layout[i].buffer_idx;
                    neko_gl_vertex_buffer_decl_t vdecl = vbo_idx < neko_dyn_array_size(ogl->cache.vdecls) ? ogl->cache.vdecls[vbo_idx] : ogl->cache.vdecls[0];
                    neko_gl_buffer_t vbo = vdecl.vbo;

                    // Manual override. If you manually set divisor/stride/offset, then will not automatically calculate any of those.
                    bool is_manual = pip->layout[i].stride | pip->layout[i].divisor | pip->layout[i].offset | vdecl.data_type == R_VERTEX_DATA_NONINTERLEAVED;

                    // Bind buffer
                    glBindBuffer(GL_ARRAY_BUFFER, vbo.id);

                    // Stride of vertex attribute
                    size_t stride = is_manual ? pip->layout[i].stride : neko_gl_calculate_vertex_size_in_bytes(pip->layout, neko_dyn_array_size(pip->layout));

                    // Byte offset of vertex attribute (if non-interleaved data, then grab offset from decl instead)
                    size_t offset = vdecl.data_type == R_VERTEX_DATA_NONINTERLEAVED ? vdecl.offset : is_manual ? pip->layout[i].offset : neko_gl_get_vertex_attr_byte_offest(pip->layout, i);

                    // If there is a vertex divisor for this layout, then we'll draw instanced
                    is_instanced |= (pip->layout[i].divisor != 0);

                    // Enable the vertex attribute pointer
                    glEnableVertexAttribArray(i);

                    switch (pip->layout[i].format) {
                        case R_VERTEX_ATTRIBUTE_FLOAT4:
                            glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_FLOAT3:
                            glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_FLOAT2:
                            glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_FLOAT:
                            glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT4:
                            glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT3:
                            glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT2:
                            glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT:
                            glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE:
                            glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE2:
                            glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE3:
                            glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE4:
                            glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;

                        // Shouldn't get here
                        default: {
                            neko_assert(false);
                        } break;
                    }
                    // Set up divisor (for instancing)
                    glVertexAttribDivisor(i, pip->layout[i].divisor);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                // Bind all vertex buffers after setting up data and pointers
                for (u32 i = 0; i < neko_dyn_array_size(ogl->cache.vdecls); ++i) {
                    glBindBuffer(GL_ARRAY_BUFFER, ogl->cache.vdecls[i].vbo.id);
                }

                // Draw based on bound primitive type in raster
                neko_byte_buffer_readc(&cb->commands, u32, start);
                neko_byte_buffer_readc(&cb->commands, u32, count);
                neko_byte_buffer_readc(&cb->commands, u32, instance_count);
                neko_byte_buffer_readc(&cb->commands, u32, base_vertex);
                neko_byte_buffer_readc(&cb->commands, u32, range_start);
                neko_byte_buffer_readc(&cb->commands, u32, range_end);

                range_end = (range_end && range_end > range_start) ? range_end : start + count;

                // Bind element buffer ranged
                if (ogl->cache.ibo.id) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, neko_slot_array_get(ogl->index_buffers, ogl->cache.ibo.id).id);
                }

                // If instance count > 1, do instanced drawing
                is_instanced |= (instance_count > 1);

                u32 prim = neko_gl_primitive_to_gl_primitive(pip->raster.primitive);
                u32 itype = neko_gl_index_buffer_size_to_gl_index_type(pip->raster.index_buffer_element_size);

                // Draw
                if (ogl->cache.ibo.id) {
#ifdef R_IMPL_OPENGL_CORE
                    if (is_instanced)
                        glDrawElementsInstancedBaseVertex(prim, count, itype, NEKO_INT2VOIDP(start), instance_count, base_vertex);
                    else
                        glDrawRangeElementsBaseVertex(prim, range_start, range_end, count, itype, NEKO_INT2VOIDP(start), base_vertex);
#else
                    if (is_instanced)
                        glDrawElementsInstanced(prim, count, itype, NEKO_INT2VOIDP(start), instance_count);
                    else
                        glDrawElements(prim, count, itype, NEKO_INT2VOIDP(start));
#endif
                } else {
                    if (is_instanced)
                        glDrawArraysInstanced(prim, start, count, instance_count);
                    else
                        glDrawArrays(prim, start, count);
                }

            } break;

            case NEKO_OPENGL_OP_DRAW_FUNC: {
                neko_byte_buffer_readc(&cb->commands, R_DRAW_FUNC, draw_func);
                draw_func(cb);
            } break;

            case NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE: {
                neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                neko_byte_buffer_readc(&cb->commands, gfx_texture_desc_t, desc);
                neko_byte_buffer_readc(&cb->commands, size_t, data_size);

                // Update texture with data, depending on update type (for now, just stream new data)

                // Grab texture from sampler id
                if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                    NEKO_TIMED_ACTION(60, { console_log("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                    neko_byte_buffer_advance_position(&cb->commands, data_size);
                }

                neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                u32 int_format = neko_gl_texture_format_to_gl_texture_internal_format(desc.format);
                u32 format = neko_gl_texture_format_to_gl_texture_format(desc.format);
                u32 dt = neko_gl_texture_format_to_gl_data_type(desc.format);
                desc.data = (cb->commands.data + cb->commands.position);
                *tex = gl_texture_update_internal(&desc, tex_slot_id);

                // Bind texture
                // glBindTexture(GL_TEXTURE_2D, tex->id);
                // // Update texture data
                // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (cb->commands.data, cb->commands.position));
                // // glTexImage2D(GL_TEXTURE_2D, 0, int_format, desc.width, desc.height, 0, format, dt, (cb->commands.data, cb->commands.position));
                // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.width, desc.height, format, dt, (cb->commands.data + cb->commands.position));
                // glBindTexture(GL_TEXTURE_2D, 0);
                neko_byte_buffer_advance_position(&cb->commands, data_size);
            } break;

            case NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE: {
                neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

                // Read handle id
                neko_byte_buffer_readc(&cb->commands, u32, id);
                // Read type
                neko_byte_buffer_readc(&cb->commands, gfx_buffer_type, type);
                // Read usage
                neko_byte_buffer_readc(&cb->commands, gfx_buffer_usage_type, usage);
                // Read data size
                neko_byte_buffer_readc(&cb->commands, size_t, sz);
                // Read data offset
                neko_byte_buffer_readc(&cb->commands, size_t, offset);
                // Read update type
                neko_byte_buffer_readc(&cb->commands, gfx_buffer_update_type, update_type);

                i32 glusage = neko_gl_buffer_usage_to_gl_enum(usage);

                switch (type) {
                    // Vertex Buffer
                    default:
                    case R_BUFFER_VERTEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, id);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
                        switch (update_type) {
                            case R_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    } break;

                    case R_BUFFER_INDEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, id);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
                        switch (update_type) {
                            case R_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } break;

                    case R_BUFFER_UNIFORM: {
                        // Have to
                        neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                        glBindBuffer(GL_UNIFORM_BUFFER, u->ubo);

                        switch (update_type) {
                            case R_BUFFER_UPDATE_SUBDATA: {
                                glBufferSubData(GL_UNIFORM_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                            } break;
                            default: {
                                // Reset uniform size
                                u->size = sz;
                                // Recreate buffer
                                glBufferData(GL_UNIFORM_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                            } break;
                        }

                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    } break;

                    case R_BUFFER_SHADER_STORAGE: {
                        gfx_storage_buffer_desc_t desc = {
                                .data = cb->commands.data + cb->commands.position,
                                .size = sz,
                                .usage = usage,
                                .update =
                                        {
                                                .type = update_type,
                                                .offset = offset,
                                        },
                        };
                        neko_handle(gfx_storage_buffer_t) hndl;
                        hndl.id = id;
                        gfx_storage_buffer_update(hndl, &desc);

                    } break;
                }

                // Advance past data
                neko_byte_buffer_advance_position(&cb->commands, sz);

            } break;

            default: {
                // Op code not supported yet!
                neko_println("Op code not supported yet: %zu", (u32)op_code);
                neko_assert(false);
            }
        }
    }

    // Clear byte buffer of commands
    neko_byte_buffer_clear(&cb->commands);

    // Set num commands to 0
    cb->num_commands = 0;
}

neko_gl_data_t* gfx_ogl() {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    return ogl;
}

void gfx_init(gfx_t* render) {
    // Push back 0 handles into slot arrays (for 0 init validation)
    neko_gl_data_t* ogl = (neko_gl_data_t*)render->ud;

    neko_slot_array_insert(ogl->shaders, neko_gl_shader_t{0});
    neko_slot_array_insert(ogl->vertex_buffers, neko_gl_buffer_t{0});
    neko_slot_array_insert(ogl->index_buffers, neko_gl_buffer_t{0});
    neko_slot_array_insert(ogl->frame_buffers, neko_gl_buffer_t{0});

    neko_gl_uniform_list_t ul = NEKO_DEFAULT_VAL();
    neko_gl_uniform_buffer_t ub = NEKO_DEFAULT_VAL();
    neko_gl_pipeline_t pip = NEKO_DEFAULT_VAL();
    neko_gl_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_gl_texture_t tex = NEKO_DEFAULT_VAL();
    neko_gl_storage_buffer_t sb = NEKO_DEFAULT_VAL();

    neko_slot_array_insert(ogl->uniforms, ul);
    neko_slot_array_insert(ogl->pipelines, pip);
    neko_slot_array_insert(ogl->renderpasses, rp);
    neko_slot_array_insert(ogl->uniform_buffers, ub);
    neko_slot_array_insert(ogl->textures, tex);
    neko_slot_array_insert(ogl->storage_buffers, sb);

    // Construct vao then bind
    glGenVertexArrays(1, &ogl->cache.vao.id);
    glBindVertexArray(ogl->cache.vao.id);

    // Reset data cache for rendering ops
    neko_gl_reset_data_cache(&ogl->cache);

    // Init info object
    gfx_info_t* info = &RENDER()->info;

    // OpenGL 主机信息
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);
    info->version = (const_str)glGetString(GL_VERSION);
    info->vendor = (const_str)glGetString(GL_RENDERER);

    // Max texture units
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&info->max_texture_units);

    // Compute shader info
    info->compute.available = info->major_version >= 4 && info->minor_version >= 3;
    if (info->compute.available) {
        CHECK_GL_CORE({
            // Work group counts
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (i32*)&info->compute.max_work_group_count[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (i32*)&info->compute.max_work_group_count[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (i32*)&info->compute.max_work_group_count[2]);
            // Work group sizes
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (i32*)&info->compute.max_work_group_size[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (i32*)&info->compute.max_work_group_size[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (i32*)&info->compute.max_work_group_size[2]);
            // Work group invocations
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (i32*)&info->compute.max_work_group_invocations);
        });
    } else {
        // console_log("opengl compute shaders not available");
    }

#if defined(_DEBUG) && 0
    int numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    neko_printf("Supported extensions:");
    for (int i = 0; i < numExtensions; i++) {
        const GLubyte* extension = glGetStringi(GL_EXTENSIONS, i);
        neko_printf("%s;", extension);
    }
    neko_printf("\n");
#endif
}

// 资源构造函数
neko_handle(gfx_texture_t) gfx_texture_create(const gfx_texture_desc_t desc) { return gfx_texture_create_impl(desc); }

neko_handle(gfx_uniform_t) gfx_uniform_create(const gfx_uniform_desc_t desc) { return gfx_uniform_create_impl(desc); }

neko_handle(gfx_shader_t) gfx_shader_create(const gfx_shader_desc_t desc) { return gfx_shader_create_impl(desc); }

neko_handle(gfx_vertex_buffer_t) gfx_vertex_buffer_create(const gfx_vertex_buffer_desc_t desc) { return gfx_vertex_buffer_create_impl(desc); }

neko_handle(gfx_index_buffer_t) gfx_index_buffer_create(const gfx_index_buffer_desc_t desc) { return gfx_index_buffer_create_impl(desc); }

neko_handle(gfx_uniform_buffer_t) gfx_uniform_buffer_create(const gfx_uniform_buffer_desc_t desc) { return gfx_uniform_buffer_create_impl(desc); }

neko_handle(gfx_storage_buffer_t) gfx_storage_buffer_create(const gfx_storage_buffer_desc_t desc) { return gfx_storage_buffer_create_impl(desc); }

neko_handle(gfx_framebuffer_t) gfx_framebuffer_create(const gfx_framebuffer_desc_t desc) { return gfx_framebuffer_create_impl(desc); }

neko_handle(gfx_renderpass_t) gfx_renderpass_create(const gfx_renderpass_desc_t desc) { return gfx_renderpass_create_impl(desc); }

neko_handle(gfx_pipeline_t) gfx_pipeline_create(const gfx_pipeline_desc_t desc) { return gfx_pipeline_create_impl(desc); }

// Destroy
void gfx_texture_fini(neko_handle(gfx_texture_t) hndl) { gfx_texture_fini_impl(hndl); }

void gfx_uniform_fini(neko_handle(gfx_uniform_t) hndl) { gfx_uniform_fini_impl(hndl); }

void gfx_shader_fini(neko_handle(gfx_shader_t) hndl) { gfx_shader_fini_impl(hndl); }

void gfx_vertex_buffer_fini(neko_handle(gfx_vertex_buffer_t) hndl) { gfx_vertex_buffer_fini_impl(hndl); }

void gfx_index_buffer_fini(neko_handle(gfx_index_buffer_t) hndl) { gfx_index_buffer_fini_impl(hndl); }

void gfx_uniform_buffer_fini(neko_handle(gfx_uniform_buffer_t) hndl) { gfx_uniform_buffer_fini_impl(hndl); }

void gfx_storage_buffer_fini(neko_handle(gfx_storage_buffer_t) hndl) { gfx_storage_buffer_fini_impl(hndl); }

void gfx_framebuffer_fini(neko_handle(gfx_framebuffer_t) hndl) { gfx_framebuffer_fini_impl(hndl); }

void gfx_renderpass_fini(neko_handle(gfx_renderpass_t) hndl) { gfx_renderpass_fini_impl(hndl); }

void gfx_pipeline_fini(neko_handle(gfx_pipeline_t) hndl) { gfx_pipeline_fini_impl(hndl); }

// 资源更新 (仅限主线程)
void gfx_vertex_buffer_update(neko_handle(gfx_vertex_buffer_t) hndl, gfx_vertex_buffer_desc_t* desc) { gfx_vertex_buffer_update_impl(hndl, desc); }

void gfx_index_buffer_update(neko_handle(gfx_index_buffer_t) hndl, gfx_index_buffer_desc_t* desc) { gfx_index_buffer_update_impl(hndl, desc); }

void gfx_storage_buffer_update(neko_handle(gfx_storage_buffer_t) hndl, gfx_storage_buffer_desc_t* desc) { gfx_storage_buffer_update_impl(hndl, desc); }

void gfx_texture_update(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) { gfx_texture_update_impl(hndl, desc); }

void gfx_texture_read(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) { gfx_texture_read_impl(hndl, desc); }

void* gfx_storage_buffer_map_get(neko_handle(gfx_storage_buffer_t) hndl) { return gfx_storage_buffer_map_get_impl(hndl); }

void gfx_storage_buffer_unlock(neko_handle(gfx_storage_buffer_t) hndl) { return gfx_storage_buffer_unlock_impl(hndl); }

void* gfx_storage_buffer_lock(neko_handle(gfx_storage_buffer_t) hndl) { return gfx_storage_buffer_lock_impl(hndl); }