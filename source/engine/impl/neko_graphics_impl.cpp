

/*=============================
// NEKO_GRAPHICS
=============================*/

#include "engine/neko_engine.h"
#include "engine/util/neko_fontcache.h"

#ifndef NEKO_GRAPHICS_IMPL_CUSTOM

#if (defined NEKO_PLATFORM_WIN || defined NEKO_PLATFORM_APPLE || defined NEKO_PLATFORM_LINUX)

#define NEKO_GRAPHICS_IMPL_OPENGL_CORE

#else

#define NEKO_GRAPHICS_IMPL_OPENGL_ES

#endif

#endif

#ifndef NEKO_GRAPHICS_IMPL_H
#define NEKO_GRAPHICS_IMPL_H

#define GLAD_IMPL
#include "libs/glad/glad_impl.h"

#ifndef NEKO_GRAPHICS_IMPL_CUSTOM
#define NEKO_GRAPHICS_IMPL_DEFAULT
#endif

#ifdef NEKO_GRAPHICS_IMPL_DEFAULT

/* Graphics Info Object Query */
neko_graphics_info_t* neko_graphics_info() { return &neko_subsystem(graphics)->info; }

#endif

#if (defined NEKO_GRAPHICS_IMPL_OPENGL_CORE || defined NEKO_GRAPHICS_IMPL_OPENGL_ES)

#ifdef NEKO_GRAPHICS_IMPL_OPENGL_CORE
#define CHECK_GL_CORE(...) __VA_ARGS__
#else
#define CHECK_GL_CORE(...) neko_empty_instruction(void)
#endif

// fwd
void __neko_fontcache_create();
void __neko_fontcache_shutdown();

typedef enum neko_gl_uniform_type {
    NEKO_GL_UNIFORMTYPE_FLOAT,
    NEKO_GL_UNIFORMTYPE_INT,
    NEKO_GL_UNIFORMTYPE_VEC2,
    NEKO_GL_UNIFORMTYPE_VEC3,
    NEKO_GL_UNIFORMTYPE_VEC4,
    NEKO_GL_UNIFORMTYPE_MAT4,
    NEKO_GL_UNIFORMTYPE_SAMPLER2D,
    NEKO_GL_UNIFORMTYPE_SAMPLERCUBE
} neko_gl_uniform_type;

/* Uniform (stores samplers as well as primitive uniforms) */
typedef struct neko_gl_uniform_t {
    char name[64];              // Name of uniform to find location
    neko_gl_uniform_type type;  // Type of uniform data
    u32 location;               // Location of uniform
    size_t size;                // Total data size of uniform
    u32 sid;                    // Shader id (should probably inverse this, but I don't want to force a map lookup)
    u32 count;                  // Count (used for arrays)
} neko_gl_uniform_t;

// When a user passes in a uniform layout, that handle could then pass to a WHOLE list of uniforms (if describing a struct)
typedef struct neko_gl_uniform_list_t {
    neko_dyn_array(neko_gl_uniform_t) uniforms;  // Individual uniforms in list
    size_t size;                                 // Total size of uniform data
    char name[64];                               // Base name of uniform
} neko_gl_uniform_list_t;

typedef struct neko_gl_uniform_buffer_t {
    char name[64];
    u32 location;
    size_t size;
    u32 ubo;
    u32 sid;
} neko_gl_uniform_buffer_t;

typedef struct neko_gl_storage_buffer_t {
    char name[64];
    u32 buffer;
    s32 access;
    size_t size;
    u32 block_idx;
} neko_gl_storage_buffer_t;

/* Pipeline */
typedef struct neko_gl_pipeline_t {
    neko_graphics_blend_state_desc_t blend;
    neko_graphics_depth_state_desc_t depth;
    neko_graphics_raster_state_desc_t raster;
    neko_graphics_stencil_state_desc_t stencil;
    neko_graphics_compute_state_desc_t compute;
    neko_dyn_array(neko_graphics_vertex_attribute_desc_t) layout;
} neko_gl_pipeline_t;

/* Render Pass */
typedef struct neko_gl_renderpass_t {
    neko_handle(neko_graphics_framebuffer_t) fbo;
    neko_dyn_array(neko_handle(neko_graphics_texture_t)) color;
    neko_handle(neko_graphics_texture_t) depth;
    neko_handle(neko_graphics_texture_t) stencil;
} neko_gl_renderpass_t;

/* Shader */
typedef u32 neko_gl_shader_t;

/* Gfx Buffer */
typedef u32 neko_gl_buffer_t;

/* Texture */
typedef struct neko_gl_texture_t {
    u32 id;
    neko_graphics_texture_desc_t desc;
} neko_gl_texture_t;

typedef struct neko_gl_vertex_buffer_decl_t {
    neko_gl_buffer_t vbo;
    neko_graphics_vertex_data_type data_type;
    size_t offset;
} neko_gl_vertex_buffer_decl_t;

/* Cached data between draws */
typedef struct neko_gl_data_cache_t {
    neko_gl_buffer_t vao;
    neko_gl_buffer_t ibo;
    size_t ibo_elem_sz;
    neko_dyn_array(neko_gl_vertex_buffer_decl_t) vdecls;
    neko_handle(neko_graphics_pipeline_t) pipeline;
} neko_gl_data_cache_t;

typedef struct fontcache_drawing_internal_data_t {
    GLuint vao = 0;
    neko_fontcache cache;
    GLint fontcache_shader_render_glyph;
    GLint fontcache_shader_blit_atlas;
    GLint fontcache_shader_draw_text;
    GLuint fontcache_fbo[2];
    GLuint fontcache_fbo_texture[2];
} fontcache_drawing_internal_data_t;

/* Internal Opengl Data */
typedef struct neko_gl_data_t {
    neko_slot_array(neko_gl_shader_t) shaders;
    neko_slot_array(neko_gl_texture_t) textures;
    neko_slot_array(neko_gl_buffer_t) vertex_buffers;
    neko_slot_array(neko_gl_uniform_buffer_t) uniform_buffers;
    neko_slot_array(neko_gl_storage_buffer_t) storage_buffers;
    neko_slot_array(neko_gl_buffer_t) index_buffers;
    neko_slot_array(neko_gl_buffer_t) frame_buffers;
    neko_slot_array(neko_gl_uniform_list_t) uniforms;
    neko_slot_array(neko_gl_pipeline_t) pipelines;
    neko_slot_array(neko_gl_renderpass_t) renderpasses;

    // All the required uniform data for strict aliasing.
    struct {
        neko_dyn_array(u32) ui32;
        neko_dyn_array(s32) i32;
        neko_dyn_array(float) flt;
        neko_dyn_array(neko_vec2) vec2;
        neko_dyn_array(neko_vec3) vec3;
        neko_dyn_array(neko_vec4) vec4;
        neko_dyn_array(neko_mat4) mat4;
    } uniform_data;

    // Cached data between draw calls (to minimize state changes)
    neko_gl_data_cache_t cache;

    // fontcache
    fontcache_drawing_internal_data_t* fontcache_data;

} neko_gl_data_t;

/* Internal OGL Command Buffer Op Code */
typedef enum neko_opengl_op_code_type {
    NEKO_OPENGL_OP_BEGIN_RENDER_PASS = 0x00,
    NEKO_OPENGL_OP_END_RENDER_PASS,
    NEKO_OPENGL_OP_SET_VIEWPORT,
    NEKO_OPENGL_OP_SET_VIEW_SCISSOR,
    NEKO_OPENGL_OP_CLEAR,
    NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE,
    NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE,
    NEKO_OPENGL_OP_BIND_PIPELINE,
    NEKO_OPENGL_OP_APPLY_BINDINGS,
    NEKO_OPENGL_OP_DISPATCH_COMPUTE,
    NEKO_OPENGL_OP_DRAW,
} neko_opengl_op_code_type;

void neko_gl_reset_data_cache(neko_gl_data_cache_t* cache) {
    cache->ibo = 0;
    cache->ibo_elem_sz = 0;
    cache->pipeline = neko_handle_invalid(neko_graphics_pipeline_t);
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

    CHECK_GL_CORE(neko_graphics_info_t* info = neko_graphics_info(); if (info->compute.available) {
        neko_invoke_once(neko_log_info("info->compute.available: %s", neko_bool_str(info->compute.available)););
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    })
}

/* Utilities */
s32 neko_gl_buffer_usage_to_gl_enum(neko_graphics_buffer_usage_type type) {
    s32 mode = GL_STATIC_DRAW;
    switch (type) {
        default:
        case NEKO_GRAPHICS_BUFFER_USAGE_STATIC:
            mode = GL_STATIC_DRAW;
            break;
        case NEKO_GRAPHICS_BUFFER_USAGE_STREAM:
            mode = GL_STREAM_DRAW;
            break;
        case NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC:
            mode = GL_DYNAMIC_DRAW;
            break;
    }
    return mode;
}

u32 neko_gl_access_type_to_gl_access_type(neko_graphics_access_type type) {
    CHECK_GL_CORE(u32 access = GL_WRITE_ONLY; switch (type) {
        case NEKO_GRAPHICS_ACCESS_WRITE_ONLY:
            access = GL_WRITE_ONLY;
            break;
        case NEKO_GRAPHICS_ACCESS_READ_ONLY:
            access = GL_READ_ONLY;
            break;
        case NEKO_GRAPHICS_ACCESS_READ_WRITE:
            access = GL_READ_WRITE;
            break;
        default:
            break;
    } return access;)
    return 0;
}

u32 neko_gl_texture_wrap_to_gl_texture_wrap(neko_graphics_texture_wrapping_type type) {
    u32 wrap = GL_REPEAT;
    switch (type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT:
            wrap = GL_REPEAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT:
            wrap = GL_MIRRORED_REPEAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE:
            wrap = GL_CLAMP_TO_EDGE;
            break;
            CHECK_GL_CORE(case NEKO_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_BORDER : wrap = GL_CLAMP_TO_BORDER; break;)
    };

    return wrap;
}

u32 neko_gl_texture_format_to_gl_data_type(neko_graphics_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            format = GL_UNSIGNED_INT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_UNSIGNED_INT_24_8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
    }
    return format;
}

u32 neko_gl_texture_format_to_gl_texture_format(neko_graphics_texture_format_type type) {
    u32 dt = GL_RGBA;
    switch (type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            dt = GL_RGBA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            dt = GL_ALPHA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            dt = GL_RED;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            dt = GL_RED_INTEGER;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            dt = GL_RGB;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            dt = GL_RGBA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            dt = GL_RGBA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
    }
    return dt;
}

u32 neko_gl_texture_format_to_gl_texture_internal_format(neko_graphics_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            format = GL_ALPHA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            format = GL_RED;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            format = GL_R32UI;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            format = GL_RGB8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            format = GL_RGBA8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            format = GL_RGBA16F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            format = GL_RGBA32F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            format = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            format = GL_DEPTH_COMPONENT16;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            format = GL_DEPTH_COMPONENT24;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            format = GL_DEPTH_COMPONENT32F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_DEPTH24_STENCIL8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_DEPTH32F_STENCIL8;
            break;
    }
    return format;
}

u32 neko_gl_shader_stage_to_gl_stage(neko_graphics_shader_stage_type type) {
    u32 stage = GL_VERTEX_SHADER;
    switch (type) {
        default:
        case NEKO_GRAPHICS_SHADER_STAGE_VERTEX:
            stage = GL_VERTEX_SHADER;
            break;
        case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT:
            stage = GL_FRAGMENT_SHADER;
            break;
            CHECK_GL_CORE(case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE : stage = GL_COMPUTE_SHADER; break;)
    }
    return stage;
}

u32 neko_gl_primitive_to_gl_primitive(neko_graphics_primitive_type type) {
    u32 prim = GL_TRIANGLES;
    switch (type) {
        default:
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES:
            prim = GL_TRIANGLES;
            break;
        case NEKO_GRAPHICS_PRIMITIVE_LINES:
            prim = GL_LINES;
            break;
            CHECK_GL_CORE(case NEKO_GRAPHICS_PRIMITIVE_QUADS : prim = GL_QUADS; break;)
    }
    return prim;
}

u32 neko_gl_blend_equation_to_gl_blend_eq(neko_graphics_blend_equation_type eq) {
    u32 beq = GL_FUNC_ADD;
    switch (eq) {
        default:
        case NEKO_GRAPHICS_BLEND_EQUATION_ADD:
            beq = GL_FUNC_ADD;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_SUBTRACT:
            beq = GL_FUNC_SUBTRACT;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT:
            beq = GL_FUNC_REVERSE_SUBTRACT;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_MIN:
            beq = GL_MIN;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_MAX:
            beq = GL_MAX;
            break;
    };

    return beq;
}

u32 neko_gl_blend_mode_to_gl_blend_mode(neko_graphics_blend_mode_type type, u32 def) {
    u32 mode = def;
    switch (type) {
        case NEKO_GRAPHICS_BLEND_MODE_ZERO:
            mode = GL_ZERO;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE:
            mode = GL_ONE;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_SRC_COLOR:
            mode = GL_SRC_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR:
            mode = GL_ONE_MINUS_SRC_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_DST_COLOR:
            mode = GL_DST_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR:
            mode = GL_ONE_MINUS_DST_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA:
            mode = GL_SRC_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA:
            mode = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_DST_ALPHA:
            mode = GL_DST_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA:
            mode = GL_ONE_MINUS_DST_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_CONSTANT_COLOR:
            mode = GL_CONSTANT_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR:
            mode = GL_ONE_MINUS_CONSTANT_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA:
            mode = GL_CONSTANT_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA:
            mode = GL_ONE_MINUS_CONSTANT_ALPHA;
            break;
    }
    return mode;
}

u32 neko_gl_cull_face_to_gl_cull_face(neko_graphics_face_culling_type type) {
    u32 fc = GL_BACK;
    switch (type) {
        default:
        case NEKO_GRAPHICS_FACE_CULLING_BACK:
            fc = GL_BACK;
            break;
        case NEKO_GRAPHICS_FACE_CULLING_FRONT:
            fc = GL_FRONT;
            break;
        case NEKO_GRAPHICS_FACE_CULLING_FRONT_AND_BACK:
            fc = GL_FRONT_AND_BACK;
            break;
    }
    return fc;
}

u32 neko_gl_winding_order_to_gl_winding_order(neko_graphics_winding_order_type type) {
    u32 wo = GL_CCW;
    switch (type) {
        case NEKO_GRAPHICS_WINDING_ORDER_CCW:
            wo = GL_CCW;
            break;
        case NEKO_GRAPHICS_WINDING_ORDER_CW:
            wo = GL_CW;
            break;
    }
    return wo;
}

u32 neko_gl_depth_func_to_gl_depth_func(neko_graphics_depth_func_type type) {
    u32 func = GL_LESS;
    switch (type) {
        default:
        case NEKO_GRAPHICS_DEPTH_FUNC_LESS:
            func = GL_LESS;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

bool neko_gl_depth_mask_to_gl_mask(neko_graphics_depth_mask_type type) {
    bool ret = true;
    switch (type) {
        default:
        case NEKO_GRAPHICS_DEPTH_MASK_ENABLED:
            ret = true;
            break;
        case NEKO_GRAPHICS_DEPTH_MASK_DISABLED:
            ret = false;
            break;
    }
    return ret;
}

u32 neko_gl_stencil_func_to_gl_stencil_func(neko_graphics_stencil_func_type type) {
    u32 func = GL_ALWAYS;
    switch (type) {
        default:
        case NEKO_GRAPHICS_STENCIL_FUNC_LESS:
            func = GL_LESS;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

u32 neko_gl_stencil_op_to_gl_stencil_op(neko_graphics_stencil_op_type type) {
    u32 op = GL_KEEP;
    switch (type) {
        default:
        case NEKO_GRAPHICS_STENCIL_OP_KEEP:
            op = GL_KEEP;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_ZERO:
            op = GL_ZERO;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_REPLACE:
            op = GL_REPLACE;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_INCR:
            op = GL_INCR;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_INCR_WRAP:
            op = GL_INCR_WRAP;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_DECR:
            op = GL_DECR;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_DECR_WRAP:
            op = GL_DECR_WRAP;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_INVERT:
            op = GL_INVERT;
            break;
    }
    return op;
}

neko_gl_uniform_type neko_gl_uniform_type_to_gl_uniform_type(neko_graphics_uniform_type gstype) {
    neko_gl_uniform_type type = NEKO_GL_UNIFORMTYPE_FLOAT;
    switch (gstype) {
        default:
        case NEKO_GRAPHICS_UNIFORM_FLOAT:
            type = NEKO_GL_UNIFORMTYPE_FLOAT;
            break;
        case NEKO_GRAPHICS_UNIFORM_INT:
            type = NEKO_GL_UNIFORMTYPE_INT;
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC2:
            type = NEKO_GL_UNIFORMTYPE_VEC2;
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC3:
            type = NEKO_GL_UNIFORMTYPE_VEC3;
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC4:
            type = NEKO_GL_UNIFORMTYPE_VEC4;
            break;
        case NEKO_GRAPHICS_UNIFORM_MAT4:
            type = NEKO_GL_UNIFORMTYPE_MAT4;
            break;
        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE:
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

size_t neko_gl_get_byte_size_of_vertex_attribute(neko_graphics_vertex_attribute_type type) {
    size_t byte_size = 0;
    switch (type) {
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4: {
            byte_size = sizeof(f32) * 4;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3: {
            byte_size = sizeof(f32) * 3;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2: {
            byte_size = sizeof(f32) * 2;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT: {
            byte_size = sizeof(f32) * 1;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4: {
            byte_size = sizeof(u32) * 4;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3: {
            byte_size = sizeof(u32) * 3;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2: {
            byte_size = sizeof(u32) * 2;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT: {
            byte_size = sizeof(u32) * 1;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4: {
            byte_size = sizeof(uint8_t) * 4;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3: {
            byte_size = sizeof(uint8_t) * 3;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2: {
            byte_size = sizeof(uint8_t) * 2;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE: {
            byte_size = sizeof(uint8_t) * 1;
        } break;
    }

    return byte_size;
}

size_t neko_gl_calculate_vertex_size_in_bytes(neko_graphics_vertex_attribute_desc_t* layout, u32 count) {
    // Iterate through all formats in delcarations and calculate total size
    size_t sz = 0;
    for (u32 i = 0; i < count; ++i) {
        neko_graphics_vertex_attribute_type type = layout[i].format;
        sz += neko_gl_get_byte_size_of_vertex_attribute(type);
    }

    return sz;
}

size_t neko_gl_get_vertex_attr_byte_offest(neko_dyn_array(neko_graphics_vertex_attribute_desc_t) layout, u32 idx) {
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

size_t neko_gl_uniform_data_size_in_bytes(neko_graphics_uniform_type type) {
    size_t sz = 0;
    switch (type) {
        case NEKO_GRAPHICS_UNIFORM_FLOAT:
            sz = sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_INT:
            sz = sizeof(s32);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC2:
            sz = 2 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC3:
            sz = 3 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC4:
            sz = 4 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_MAT4:
            sz = 16 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
            sz = sizeof(neko_handle(neko_graphics_texture_t));
            break;  // handle size
        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
            sz = sizeof(neko_handle(neko_graphics_texture_t));
            break;  // handle size
        case NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE:
            sz = sizeof(neko_handle(neko_graphics_texture_t));
            break;  // handle size
        default: {
            sz = 0;
        } break;
    }
    return sz;
}

/* Graphics Interface Creation / Initialization / Shutdown / Destruction */
NEKO_API_DECL neko_graphics_t* neko_graphics_create() {
    // Construct new graphics interface
    neko_graphics_t* gfx = neko_malloc_init(neko_graphics_t);

    // Construct internal data for opengl
    gfx->user_data = neko_malloc_init(neko_gl_data_t);

    return gfx;
}

NEKO_API_DECL void neko_graphics_destroy(neko_graphics_t* graphics) {
    // Free all resources (assuming they've been freed from the GPU already)
    if (graphics == NULL) return;

    // 销毁 fontcache
    __neko_fontcache_shutdown();

    neko_gl_data_t* ogl = (neko_gl_data_t*)graphics->user_data;

#define OGL_FREE_DATA(SA, T, FUNC)                                                                                    \
    do {                                                                                                              \
        for (neko_slot_array_iter it = 1; neko_slot_array_iter_valid(SA, it); neko_slot_array_iter_advance(SA, it)) { \
            neko_handle(T) hndl = neko_default_val();                                                                 \
            hndl.id = it;                                                                                             \
            FUNC(hndl);                                                                                               \
        }                                                                                                             \
    } while (0)

    // Free all gl data
    if (ogl->pipelines) OGL_FREE_DATA(ogl->pipelines, neko_graphics_pipeline_t, neko_graphics_pipeline_destroy);
    if (ogl->shaders) OGL_FREE_DATA(ogl->shaders, neko_graphics_shader_t, neko_graphics_shader_destroy);
    if (ogl->vertex_buffers) OGL_FREE_DATA(ogl->vertex_buffers, neko_graphics_vertex_buffer_t, neko_graphics_vertex_buffer_destroy);
    if (ogl->index_buffers) OGL_FREE_DATA(ogl->index_buffers, neko_graphics_index_buffer_t, neko_graphics_index_buffer_destroy);
    if (ogl->renderpasses) OGL_FREE_DATA(ogl->renderpasses, neko_graphics_renderpass_t, neko_graphics_renderpass_destroy);
    if (ogl->frame_buffers) OGL_FREE_DATA(ogl->frame_buffers, neko_graphics_framebuffer_t, neko_graphics_framebuffer_destroy);
    if (ogl->textures) OGL_FREE_DATA(ogl->textures, neko_graphics_texture_t, neko_graphics_texture_destroy);
    if (ogl->uniforms) OGL_FREE_DATA(ogl->uniforms, neko_graphics_uniform_t, neko_graphics_uniform_destroy);
    if (ogl->uniform_buffers) OGL_FREE_DATA(ogl->uniform_buffers, neko_graphics_uniform_buffer_t, neko_graphics_uniform_buffer_destroy);
    // if (ogl->storage_buffers)   OGL_FREE_DATA(ogl->storage_buffers, neko_graphics_storage_buffer_t, neko_graphics_storage_buffer_destroy);

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

    neko_free(graphics);
    graphics = NULL;
}

NEKO_API_DECL void neko_graphics_shutdown(neko_graphics_t* graphics) {}

neko_gl_texture_t gl_texture_update_internal(const neko_graphics_texture_desc_t* desc, u32 hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    neko_gl_texture_t tex = neko_default_val();
    if (hndl) tex = neko_slot_array_get(ogl->textures, hndl);
    u32 width = desc->width;
    u32 height = desc->height;
    void* data = NULL;

    if (!hndl) {
        glGenTextures(1, &tex.id);
    }

    GLenum target = 0x00;
    switch (desc->type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_2D: {
            target = GL_TEXTURE_2D;
        } break;
        case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
            target = GL_TEXTURE_CUBE_MAP;
        } break;
    }

    glBindTexture(target, tex.id);

    u32 cnt = NEKO_GRAPHICS_TEXTURE_DATA_MAX;
    switch (desc->type) {
        case NEKO_GRAPHICS_TEXTURE_2D:
            cnt = 1;
            break;
    }

    for (u32 i = 0; i < cnt; ++i) {
        GLenum itarget = 0x00;
        data = desc->data[i];
        switch (desc->type) {
            default:
            case NEKO_GRAPHICS_TEXTURE_2D: {
                itarget = GL_TEXTURE_2D;
            } break;
            case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
                itarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            } break;
        }

        if (tex.desc.width * tex.desc.height < width * height) {
            // Construct texture based on appropriate format
            switch (desc->format) {
                case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
                    glTexImage2D(itarget, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
                    glTexImage2D(itarget, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
                    glTexImage2D(itarget, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RG8:
                    glTexImage2D(itarget, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
                    glTexImage2D(itarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
                    glTexImage2D(itarget, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
                    glTexImage2D(itarget, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
                    glTexImage2D(itarget, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
                    glTexImage2D(itarget, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                    glTexImage2D(itarget, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                    glTexImage2D(itarget, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                    break;

                // NOTE: 因为 Apple 是一家垃圾公司 所以必须将其分开并仅提供对 4.1 功能的支持。
                // case NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
                default:
                    break;
            }
        } else {
            // Subimage
            switch (desc->format) {
                case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
                    glTexImage2D(itarget, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RG8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RG, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
                    glTexImage2D(itarget, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                    break;

                // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
                // case NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
                default:
                    break;
            }
        }
    }

    s32 mag_filter = desc->mag_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    s32 min_filter = desc->min_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        } else {
            min_filter = desc->mip_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    const u32 texture_wrap_s = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_s);
    const u32 texture_wrap_t = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_t);
    const u32 texture_wrap_r = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_r);

    if (desc->num_mips) {
        glGenerateMipmap(target);
    }

    // Need to make sure this is available before being able to use

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

/* Resource Creation */
NEKO_API_DECL neko_handle(neko_graphics_texture_t) neko_graphics_texture_create_impl(const neko_graphics_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_texture_t tex = gl_texture_update_internal(desc, 0);
    // Add texture to internal resource pool and return handle
    return (neko_handle_create(neko_graphics_texture_t, neko_slot_array_insert(ogl->textures, tex)));
}

NEKO_API_DECL neko_handle(neko_graphics_uniform_t) neko_graphics_uniform_create_impl(const neko_graphics_uniform_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Assert if data isn't named
    if (desc->name == NULL) {
        neko_log_warning("Uniform must be named for OpenGL.");
        return neko_handle_invalid(neko_graphics_uniform_t);
    }

    u32 ct = !desc->layout ? 0 : !desc->layout_size ? 1 : (u32)desc->layout_size / (u32)sizeof(neko_graphics_uniform_layout_desc_t);
    if (ct < 1) {
        neko_log_warning("Uniform layout description must not be empty for: %s.", desc->name);
        return neko_handle_invalid(neko_graphics_uniform_t);
    }

    // Construct list for uniform handles
    neko_gl_uniform_list_t ul = neko_default_val();
    memcpy(ul.name, desc->name, 64);

    // Iterate layout, construct individual handles
    for (u32 i = 0; i < ct; ++i) {
        // Uniform to fill out
        neko_gl_uniform_t u = neko_default_val();
        // Cache layout
        neko_graphics_uniform_layout_desc_t* layout = &desc->layout[i];

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

    return neko_handle_create(neko_graphics_uniform_t, neko_slot_array_insert(ogl->uniforms, ul));
}

NEKO_API_DECL neko_handle(neko_graphics_vertex_buffer_t) neko_graphics_vertex_buffer_create_impl(const neko_graphics_vertex_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_vertex_buffer_t) hndl = neko_default_val();
    neko_gl_buffer_t buffer = 0;

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc->usage == NEKO_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        neko_println("Error: Vertex buffer desc must contain data when NEKO_GRAPHICS_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(neko_graphics_vertex_buffer_t, neko_slot_array_insert(ogl->vertex_buffers, buffer));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_index_buffer_t) neko_graphics_index_buffer_create_impl(const neko_graphics_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_index_buffer_t) hndl = neko_default_val();
    neko_gl_buffer_t buffer = 0;

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc->usage == NEKO_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        neko_println("Error: Index buffer desc must contain data when NEKO_GRAPHICS_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(neko_graphics_index_buffer_t, neko_slot_array_insert(ogl->index_buffers, buffer));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_uniform_buffer_t) neko_graphics_uniform_buffer_create_impl(const neko_graphics_uniform_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_uniform_buffer_t) hndl = neko_default_val();

    // Assert if data isn't named
    if (desc->name == NULL) {
        neko_log_warning("Uniform buffer must be named for Opengl.");
    }

    neko_gl_uniform_buffer_t u = neko_default_val();
    memcpy(u.name, desc->name, 64);
    u.size = desc->size;
    u.location = UINT32_MAX;

    // Generate buffer (if needed)
    glGenBuffers(1, &u.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, u.ubo);
    glBufferData(GL_UNIFORM_BUFFER, u.size, 0, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    hndl = neko_handle_create(neko_graphics_uniform_buffer_t, neko_slot_array_insert(ogl->uniform_buffers, u));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_storage_buffer_t) neko_graphics_storage_buffer_create_impl(const neko_graphics_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_storage_buffer_t) hndl = neko_default_val();
    neko_gl_storage_buffer_t sbo = neko_default_val();

    if (desc->name == NULL) {
        neko_log_warning("Storage buffer must be named for Opengl.");
    }

    if (desc->usage == NEKO_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        neko_println("Error: Storage buffer desc must contain data when NEKO_GRAPHICS_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &sbo.buffer);

    CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo.buffer); glBufferData(GL_SHADER_STORAGE_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
                  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););

    memcpy(sbo.name, desc->name, 64);
    sbo.access = desc->access;
    sbo.size = desc->size;
    sbo.block_idx = UINT32_MAX;

    hndl = neko_handle_create(neko_graphics_storage_buffer_t, neko_slot_array_insert(ogl->storage_buffers, sbo));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_framebuffer_t) neko_graphics_framebuffer_create_impl(const neko_graphics_framebuffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_framebuffer_t) hndl = neko_default_val();
    neko_gl_buffer_t buffer = 0;
    glGenFramebuffers(1, &buffer);
    hndl = neko_handle_create(neko_graphics_framebuffer_t, neko_slot_array_insert(ogl->frame_buffers, buffer));
    return hndl;
}

#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX 0x01
#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE 0x02
#define NEKO_GL_GRAPHICS_MAX_SID 128

NEKO_API_DECL neko_handle(neko_graphics_shader_t) neko_graphics_shader_create_impl(const neko_graphics_shader_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_shader_t shader = 0;
    u32 pip = 0x00;

    u32 sid_ct = 0;
    u32 sids[NEKO_GL_GRAPHICS_MAX_SID] = neko_default_val();

    // Create shader program
    shader = glCreateProgram();

    u32 ct = (u32)desc->size / (u32)sizeof(neko_graphics_shader_source_desc_t);
    for (u32 i = 0; i < ct; ++i) {
        if (desc->sources[i].type == NEKO_GRAPHICS_SHADER_STAGE_VERTEX) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX;
        if (desc->sources[i].type == NEKO_GRAPHICS_SHADER_STAGE_COMPUTE) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE;

        // Validity Check: Desc must have vertex source if compute not selected. All other source is optional.
        if ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE) && ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX))) {
            neko_println("Error: Cannot have compute and graphics stages for shader program.");
            neko_assert(false);
        }

        u32 stage = neko_gl_shader_stage_to_gl_stage(desc->sources[i].type);
        u32 sid = glCreateShader(stage);

        if (!sid) {
            neko_println("Error: Failed to allocate memory for shader: '%s': stage: {put stage id here}", desc->name);
            neko_assert(sid);
        }

        // Set source
        glShaderSource(sid, 1, &desc->sources[i].source, NULL);

        // Compile shader
        glCompileShader(sid);

        // Check for errors
        GLint success = 0;
        glGetShaderiv(sid, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE) {
            GLint max_len = 0;
            glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &max_len);

            char* log = (char*)neko_malloc(max_len);
            memset(log, 0, max_len);

            // The max_len includes the NULL character
            glGetShaderInfoLog(sid, max_len, &max_len, log);

            // Delete shader.
            glDeleteShader(shader);

            // Provide the infolog
            neko_println("Opengl::opengl_compile_shader::shader: '%s'\nFAILED_TO_COMPILE: %s\n %s", desc->name, log, desc->sources[i].source);

            free(log);
            log = NULL;

            // neko_assert(false);
            return neko_handle_invalid(neko_graphics_shader_t);
        }

        // Attach shader to program
        glAttachShader(shader, sid);

        // Add to shader array
        sids[sid_ct++] = sid;
    }

    // Link shaders into final program
    glLinkProgram(shader);

    // Create info log for errors
    s32 is_good = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, (s32*)&is_good);
    if (is_good == GL_FALSE) {
        GLint max_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &max_len);

        char* log = (char*)neko_malloc(max_len);
        memset(log, 0, max_len);
        glGetProgramInfoLog(shader, max_len, &max_len, log);

        // Print error
        neko_log_error("Fail To Link::opengl_link_shaders::shader: '%s', \n%s", desc->name, log);

        // //We don't need the program anymore.
        glDeleteProgram(shader);

        free(log);
        log = NULL;

        // Just assert for now
        neko_assert(false);
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &is_good);
    if (!is_good) {
        neko_log_error("Failed to validate shader: '%s'", desc->name);
    }

    // Free shaders after use
    for (u32 i = 0; i < sid_ct; ++i) {
        glDeleteShader(sids[i]);
    }

    // Iterate over uniforms
    /*
    {
        char tmp_name[256] = neko_default_val();
        s32 count = 0;
        glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &count);
        neko_println("Active Uniforms: %d\n", count);

        for (u32 i = 0; i < count; i++) {
            s32 sz = 0;
            u32 type;
            glGetActiveUniform(shader, (GLuint)i, 256, NULL, &sz, &type, tmp_name);
            neko_println("Uniform #%d Type: %u Name: %s\n", i, type, tmp_name);
        }
    }
    */

    // Add to pool and return handle
    return (neko_handle_create(neko_graphics_shader_t, neko_slot_array_insert(ogl->shaders, shader)));
}

NEKO_API_DECL neko_handle(neko_graphics_renderpass_t) neko_graphics_renderpass_create_impl(const neko_graphics_renderpass_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    neko_gl_renderpass_t pass = neko_default_val();

    // Set fbo
    pass.fbo = desc->fbo;

    // Set color attachments
    u32 ct = (u32)desc->color_size / (u32)sizeof(neko_handle(neko_graphics_texture_t));
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pass.color, desc->color[i]);
    }
    // Set depth attachment
    pass.depth = desc->depth;

    // Create handle and return
    return (neko_handle_create(neko_graphics_renderpass_t, neko_slot_array_insert(ogl->renderpasses, pass)));
}

NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_graphics_pipeline_create_impl(const neko_graphics_pipeline_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    neko_gl_pipeline_t pipe = neko_default_val();

    // Add states
    pipe.blend = desc->blend;
    pipe.depth = desc->depth;
    pipe.raster = desc->raster;
    pipe.stencil = desc->stencil;
    pipe.compute = desc->compute;

    // Add layout
    u32 ct = (u32)desc->layout.size / (u32)sizeof(neko_graphics_vertex_attribute_desc_t);
    neko_dyn_array_reserve(pipe.layout, ct);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pipe.layout, desc->layout.attrs[i]);
    }

    // Create handle and return
    return (neko_handle_create(neko_graphics_pipeline_t, neko_slot_array_insert(ogl->pipelines, pipe)));
}

// Resource Destruction
NEKO_API_DECL void neko_graphics_texture_destroy_impl(neko_handle(neko_graphics_texture_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) return;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    glDeleteTextures(1, &tex->id);
    neko_slot_array_erase(ogl->textures, hndl.id);
}

NEKO_API_DECL void neko_graphics_uniform_destroy_impl(neko_handle(neko_graphics_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->uniforms, hndl.id)) return;
    neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, hndl.id);
    neko_dyn_array_free(ul->uniforms);
    neko_slot_array_erase(ogl->uniforms, hndl.id);
}

NEKO_API_DECL void neko_graphics_shader_destroy_impl(neko_handle(neko_graphics_shader_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->shaders, hndl.id)) return;
    glDeleteProgram(neko_slot_array_get(ogl->shaders, hndl.id));
    neko_slot_array_erase(ogl->shaders, hndl.id);
}

NEKO_API_DECL void neko_graphics_vertex_buffer_destroy_impl(neko_handle(neko_graphics_vertex_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->vertex_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glDeleteBuffers(1, &buffer);
    neko_slot_array_erase(ogl->vertex_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_index_buffer_destroy_impl(neko_handle(neko_graphics_index_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->index_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    glDeleteBuffers(1, &buffer);
    neko_slot_array_erase(ogl->index_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_uniform_buffer_destroy_impl(neko_handle(neko_graphics_uniform_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->uniform_buffers, hndl.id)) return;
    neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &u->ubo);

    // Delete from slot array
    neko_slot_array_erase(ogl->uniform_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_storage_buffer_destroy_impl(neko_handle(neko_graphics_storage_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) return;
    neko_gl_storage_buffer_t* sb = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &sb->buffer);

    // Delete from slot array
    neko_slot_array_erase(ogl->storage_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_framebuffer_destroy_impl(neko_handle(neko_graphics_framebuffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->frame_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->frame_buffers, hndl.id);
    glDeleteFramebuffers(1, &buffer);
    neko_slot_array_erase(ogl->frame_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_renderpass_destroy_impl(neko_handle(neko_graphics_renderpass_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->renderpasses, hndl.id)) return;
    // TODO: erase all color attachments from renderpasss
    neko_slot_array_erase(ogl->renderpasses, hndl.id);
}

NEKO_API_DECL void neko_graphics_pipeline_destroy_impl(neko_handle(neko_graphics_pipeline_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->pipelines, hndl.id)) return;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Free layout
    neko_dyn_array_free(pip->layout);

    // Erase handles from slot arrays
    neko_slot_array_erase(ogl->pipelines, hndl.id);
}

// Resource Query
NEKO_API_DECL void neko_graphics_pipeline_desc_query(neko_handle(neko_graphics_pipeline_t) hndl, neko_graphics_pipeline_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
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

NEKO_API_DECL void neko_graphics_texture_desc_query(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);

    // Read back pixels
    if (out->data && out->read.width && out->read.height) {
        u32 type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
        u32 format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
        CHECK_GL_CORE(glActiveTexture(GL_TEXTURE0); glGetTextureSubImage(tex->id, 0, out->read.x, out->read.y, 0, out->read.width, out->read.height, 1, format, type, out->read.size, out->data););
    }

    *out = tex->desc;
}

NEKO_API_DECL size_t neko_graphics_uniform_size_query(neko_handle(neko_graphics_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_uniform_list_t* u = neko_slot_array_getp(ogl->uniforms, hndl.id);
    return u->uniforms[0].size;
}

// Resource Updates (main thread only)
NEKO_API_DECL void neko_graphics_texture_update_impl(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) {
    if (!desc) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        neko_log_warning("Texture handle invalid: %zu", hndl.id);
        return;
    }
    gl_texture_update_internal(desc, hndl.id);
}

NEKO_API_DECL void neko_graphics_vertex_buffer_update_impl(neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc) {
    /*
    void __neko_graphics_update_buffer_internal(neko_command_buffer_t* cb,
        u32 id,
        neko_graphics_buffer_type type,
        neko_graphics_buffer_usage_type usage,
        size_t sz,
        size_t offset,
        neko_graphics_buffer_update_type update_type,
        void* data)
    {
        // Write command
        neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
        cb->num_commands++;

        // Write handle id
        neko_byte_buffer_write(&cb->commands, u32, id);
        // Write type
        neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_type, type);
        // Write usage
        neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_usage_type, usage);
        // Write data size
        neko_byte_buffer_write(&cb->commands, size_t, sz);
        // Write data offset
        neko_byte_buffer_write(&cb->commands, size_t, offset);
        // Write data update type
        neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_update_type, update_type);
        // Write data
        neko_byte_buffer_write_bulk(&cb->commands, data, sz);
    }
    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_VERTEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    s32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    switch (desc->update.type) {
        case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

NEKO_API_DECL void neko_graphics_index_buffer_update_impl(neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    s32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    switch (desc->update.type) {
    l:
    case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
        break;
        default:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

NEKO_API_DECL void neko_graphics_storage_buffer_update_impl(neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);
    if (!sbo) {
        neko_log_warning("Storage buffer %zu not found.", hndl.id);
        return;
    }

    CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo->buffer); switch (desc->update.type) {
        case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_SHADER_STORAGE_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    } glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););
}

NEKO_API_DECL void neko_graphics_texture_read_impl(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!desc) return;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        neko_log_warning("Texture handle invalid: %zu", hndl.id);
    }

    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    // Bind texture
    GLenum target = 0x00;
    switch (tex->desc.type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_2D: {
            target = GL_TEXTURE_2D;
        } break;
        case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
            target = GL_TEXTURE_CUBE_MAP;
        } break;
    }
    u32 gl_format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
    u32 gl_type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
    glBindTexture(target, tex->id);
    glReadPixels(desc->read.x, desc->read.y, desc->read.width, desc->read.height, gl_format, gl_type, *desc->data);
    glBindTexture(target, 0x00);
}

#define __ogl_push_command(CB, OP_CODE, ...)                                         \
    do {                                                                             \
        neko_gl_data_t* DATA = (neko_gl_data_t*)neko_subsystem(graphics)->user_data; \
        neko_byte_buffer_write(&CB->commands, u32, (u32)OP_CODE);                    \
        __VA_ARGS__                                                                  \
        CB->num_commands++;                                                          \
    } while (0)

/* Command Buffer Ops: Pipeline / Pass / Bind / Draw */
NEKO_API_DECL void neko_graphics_renderpass_begin(neko_command_buffer_t* cb, neko_handle(neko_graphics_renderpass_t) hndl) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_BEGIN_RENDER_PASS, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

NEKO_API_DECL void neko_graphics_renderpass_end(neko_command_buffer_t* cb) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_END_RENDER_PASS,
                       {
                               // Nothing...
                       });
}

NEKO_API_DECL void neko_graphics_clear(neko_command_buffer_t* cb, neko_graphics_clear_desc_t* desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_CLEAR, {
        u32 count = !desc->actions ? 0 : !desc->size ? 1 : (u32)((size_t)desc->size / (size_t)sizeof(neko_graphics_clear_action_t));
        neko_byte_buffer_write(&cb->commands, u32, count);
        for (u32 i = 0; i < count; ++i) {
            neko_byte_buffer_write(&cb->commands, neko_graphics_clear_action_t, desc->actions[i]);
        }
    });
}

NEKO_API_DECL void neko_graphics_set_viewport(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEWPORT, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

NEKO_API_DECL void neko_graphics_set_view_scissor(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEW_SCISSOR, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

NEKO_API_DECL void neko_graphics_texture_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE);
    cb->num_commands++;

    u32 num_comps = 0;
    size_t data_type_size = 0;
    size_t total_size = 0;
    switch (desc->format) {
        default:
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            num_comps = 4;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            num_comps = 3;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(float) + sizeof(uint8_t);
            break;

            // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
            // case NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
    }
    total_size = desc->width * desc->height * num_comps * data_type_size;
    neko_byte_buffer_write(&cb->commands, u32, hndl.id);
    neko_byte_buffer_write(&cb->commands, neko_graphics_texture_desc_t, *desc);
    neko_byte_buffer_write(&cb->commands, size_t, total_size);
    neko_byte_buffer_write_bulk(&cb->commands, *desc->data, total_size);
}

void __neko_graphics_update_buffer_internal(neko_command_buffer_t* cb, u32 id, neko_graphics_buffer_type type, neko_graphics_buffer_usage_type usage, size_t sz, size_t offset,
                                            neko_graphics_buffer_update_type update_type, void* data) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
    cb->num_commands++;

    // Write handle id
    neko_byte_buffer_write(&cb->commands, u32, id);
    // Write type
    neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_type, type);
    // Write usage
    neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_usage_type, usage);
    // Write data size
    neko_byte_buffer_write(&cb->commands, size_t, sz);
    // Write data offset
    neko_byte_buffer_write(&cb->commands, size_t, offset);
    // Write data update type
    neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_update_type, update_type);
    // Write data
    neko_byte_buffer_write_bulk(&cb->commands, data, sz);
}

NEKO_API_DECL void neko_graphics_vertex_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_VERTEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

NEKO_API_DECL void neko_graphics_index_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_INDEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

NEKO_API_DECL void neko_graphics_uniform_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_uniform_buffer_t) hndl, neko_graphics_uniform_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_UNIFORM, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

NEKO_API_DECL void neko_graphics_storage_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_SHADER_STORAGE, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

void neko_graphics_apply_bindings(neko_command_buffer_t* cb, neko_graphics_bind_desc_t* binds) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Increment commands
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_APPLY_BINDINGS);
    cb->num_commands++;

    // __ogl_push_command(cb, NEKO_OPENGL_OP_APPLY_BINDINGS,
    {
        // Get counts from buffers
        u32 vct = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(neko_graphics_bind_vertex_buffer_desc_t) : 1 : 0;
        u32 ict = binds->index_buffers.desc ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(neko_graphics_bind_index_buffer_desc_t) : 1 : 0;
        u32 uct = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(neko_graphics_bind_uniform_buffer_desc_t) : 1 : 0;
        u32 pct = binds->uniforms.desc ? binds->uniforms.size ? binds->uniforms.size / sizeof(neko_graphics_bind_uniform_desc_t) : 1 : 0;
        u32 ibc = binds->image_buffers.desc ? binds->image_buffers.size ? binds->image_buffers.size / sizeof(neko_graphics_bind_image_buffer_desc_t) : 1 : 0;
        u32 sbc = binds->storage_buffers.desc ? binds->storage_buffers.size ? binds->storage_buffers.size / sizeof(neko_graphics_bind_storage_buffer_desc_t) : 1 : 0;

        // Determine total count to write into command buffer
        u32 ct = vct + ict + uct + pct + ibc + sbc;
        neko_byte_buffer_write(&cb->commands, u32, ct);

        // Determine if need to clear any previous vertex buffers (if vct != 0)
        neko_byte_buffer_write(&cb->commands, bool, (vct != 0));

        // Vertex buffers
        for (u32 i = 0; i < vct; ++i) {
            neko_graphics_bind_vertex_buffer_desc_t* decl = &binds->vertex_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_VERTEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, size_t, decl->offset);
            neko_byte_buffer_write(&cb->commands, neko_graphics_vertex_data_type, decl->data_type);
        }

        // Index buffers
        for (u32 i = 0; i < ict; ++i) {
            neko_graphics_bind_index_buffer_desc_t* decl = &binds->index_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_INDEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
        }

        // Uniform buffers
        for (u32 i = 0; i < uct; ++i) {
            neko_graphics_bind_uniform_buffer_desc_t* decl = &binds->uniform_buffers.desc[i];

            u32 id = decl->buffer.id;
            size_t sz = (size_t)(neko_slot_array_getp(ogl->uniform_buffers, id))->size;
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_UNIFORM_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.offset);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.size);
        }

        // Image buffers
        for (u32 i = 0; i < ibc; ++i) {
            neko_graphics_bind_image_buffer_desc_t* decl = &binds->image_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_IMAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->tex.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, neko_graphics_access_type, decl->access);
        }

        // Uniforms
        for (u32 i = 0; i < pct; ++i) {
            neko_graphics_bind_uniform_desc_t* decl = &binds->uniforms.desc[i];

            // Get size from uniform list
            size_t sz = neko_slot_array_getp(ogl->uniforms, decl->uniform.id)->size;
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_UNIFORM);
            neko_byte_buffer_write(&cb->commands, u32, decl->uniform.id);
            neko_byte_buffer_write(&cb->commands, size_t, sz);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write_bulk(&cb->commands, decl->data, sz);
        }

        // Storage buffers
        CHECK_GL_CORE(for (u32 i = 0; i < sbc; ++i) {
            neko_graphics_bind_storage_buffer_desc_t* decl = &binds->storage_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_STORAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
        });
    };
}

void neko_graphics_pipeline_bind(neko_command_buffer_t* cb, neko_handle(neko_graphics_pipeline_t) hndl) {
    // TODO: 不确定这将来是否安全 因为管道的数据位于主线程上 并且可能会在单独的线程上被篡改
    __ogl_push_command(cb, NEKO_OPENGL_OP_BIND_PIPELINE, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

void neko_graphics_draw(neko_command_buffer_t* cb, neko_graphics_draw_desc_t* desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW, {
        neko_byte_buffer_write(&cb->commands, u32, desc->start);
        neko_byte_buffer_write(&cb->commands, u32, desc->count);
        neko_byte_buffer_write(&cb->commands, u32, desc->instances);
        neko_byte_buffer_write(&cb->commands, u32, desc->base_vertex);
        neko_byte_buffer_write(&cb->commands, u32, desc->range.start);
        neko_byte_buffer_write(&cb->commands, u32, desc->range.end);
    });
}

void neko_graphics_dispatch_compute(neko_command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DISPATCH_COMPUTE, {
        neko_byte_buffer_write(&cb->commands, u32, num_x_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_y_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_z_groups);
    });
}

/* Submission (Main Thread) */
void neko_graphics_command_buffer_submit_impl(neko_command_buffer_t* cb) {
    /*
        // Structure of command:
            - Op code
            - Data packet
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Set read position of buffer to beginning
    neko_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    neko_for_range(cb->num_commands) {
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
                        glBindFramebuffer(GL_FRAMEBUFFER, neko_slot_array_get(ogl->frame_buffers, rp->fbo.id));

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
                neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
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
                    neko_byte_buffer_readc(&cb->commands, neko_graphics_clear_action_t, action);

                    // No clear
                    if (action.flag & NEKO_GRAPHICS_CLEAR_NONE) {
                        continue;
                    }

                    u32 bit = 0x00;

                    if (action.flag & NEKO_GRAPHICS_CLEAR_COLOR || action.flag == 0x00) {
                        glClearColor(action.color[0], action.color[1], action.color[2], action.color[3]);
                        bit |= GL_COLOR_BUFFER_BIT;
                    }
                    if (action.flag & NEKO_GRAPHICS_CLEAR_DEPTH || action.flag == 0x00) {
                        bit |= GL_DEPTH_BUFFER_BIT;
                    }
                    if (action.flag & NEKO_GRAPHICS_CLEAR_STENCIL || action.flag == 0x00) {
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
                    neko_byte_buffer_readc(&cb->commands, neko_graphics_bind_type, type);
                    switch (type) {
                        case NEKO_GRAPHICS_BIND_VERTEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            neko_byte_buffer_readc(&cb->commands, size_t, offset);
                            neko_byte_buffer_readc(&cb->commands, neko_graphics_vertex_data_type, data_type);

                            if (!id || !neko_slot_array_exists(ogl->vertex_buffers, id)) {
                                neko_timed_action(1000, {
                                    neko_log_warning("Opengl:BindBindings:VertexBuffer %d does not exist.", id);
                                    continue;
                                });
                            }

                            // Grab vbo to bind
                            neko_gl_buffer_t vbo = neko_slot_array_get(ogl->vertex_buffers, id);

                            // If the data type is non-interleaved, then push size into vertex buffer decl
                            neko_gl_vertex_buffer_decl_t vbo_decl = neko_default_val();
                            vbo_decl.vbo = vbo;
                            vbo_decl.data_type = data_type;
                            vbo_decl.offset = offset;

                            // Cache vertex buffer for later use
                            neko_dyn_array_push(ogl->cache.vdecls, vbo_decl);

                        } break;

                        case NEKO_GRAPHICS_BIND_INDEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);

                            if (!neko_slot_array_exists(ogl->index_buffers, id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Opengl:BindBindings:IndexBuffer %d does not exist.", id);
                                });
                                */
                            } else {
                                neko_gl_buffer_t ibo = neko_slot_array_get(ogl->index_buffers, id);

                                // Store in cache
                                ogl->cache.ibo = id;
                            }
                        } break;

                        case NEKO_GRAPHICS_BIND_UNIFORM: {
                            // Get size from uniform list
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read data size for uniform list
                            neko_byte_buffer_readc(&cb->commands, size_t, sz);
                            // Read binding from uniform list (could make this a binding list? not sure how to handle this)
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniforms, id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Uniform:Uniform %d does not exist.", id);
                                });
                                */
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id);
                                });
                                */
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

                                        neko_timed_action(1000, { neko_log_warning("Bind Uniform:Shader %d does not exist.", sid); });

                                        // Advance by size of uniform
                                        neko_byte_buffer_advance_position(&cb->commands, sz);
                                        continue;
                                    }

                                    neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                    // Construct temp name, concat with base name + uniform field name
                                    char name[256] = neko_default_val();
                                    memcpy(name, ul->name, 64);
                                    if (u->name) {
                                        neko_snprintfc(UTMP, 256, "%s%s", ul->name, u->name);
                                        memcpy(name, UTMP, 256);
                                    }

                                    // Grab location of uniform based on name
                                    u->location = glGetUniformLocation(shader, name ? name : "__EMPTY_UNIFORM_NAME");

                                    if (u->location >= UINT32_MAX) {
                                        neko_log_warning("Bind Uniform: Uniform not found: \"%s\"", name);
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
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, float, v);
                                            neko_dyn_array_push(ogl->uniform_data.flt, v);
                                        }
                                        glUniform1fv(u->location, ct, ogl->uniform_data.flt);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_INT: {
                                        neko_assert(u->size == sizeof(s32));
                                        neko_dyn_array_clear(ogl->uniform_data.i32);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, s32, v);
                                            neko_dyn_array_push(ogl->uniform_data.i32, v);
                                        }
                                        glUniform1iv(u->location, ct, ogl->uniform_data.i32);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC2: {
                                        neko_assert(u->size == sizeof(neko_vec2));
                                        neko_dyn_array_clear(ogl->uniform_data.vec2);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec2, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec2, v);
                                        }
                                        glUniform2fv(u->location, ct, (float*)ogl->uniform_data.vec2);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC3: {
                                        neko_assert(u->size == sizeof(neko_vec3));
                                        neko_dyn_array_clear(ogl->uniform_data.vec3);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec3, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec3, v);
                                        }
                                        glUniform3fv(u->location, ct, (float*)ogl->uniform_data.vec3);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC4: {
                                        neko_assert(u->size == sizeof(neko_vec4));
                                        neko_dyn_array_clear(ogl->uniform_data.vec4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec4, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec4, v);
                                        }
                                        glUniform4fv(u->location, ct, (float*)ogl->uniform_data.vec4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_MAT4: {
                                        neko_assert(u->size == sizeof(neko_mat4));
                                        neko_dyn_array_clear(ogl->uniform_data.mat4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_mat4, v);
                                            neko_dyn_array_push(ogl->uniform_data.mat4, v);
                                        }
                                        glUniformMatrix4fv(u->location, ct, false, (float*)ogl->uniform_data.mat4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_SAMPLERCUBE:
                                    case NEKO_GL_UNIFORMTYPE_SAMPLER2D: {
                                        neko_assert(u->size == sizeof(neko_handle(neko_graphics_texture_t)));
                                        u32 ct = u->count ? u->count : 1;
                                        s32 binds[128] = neko_default_val();
                                        for (u32 i = 0; (i < ct && i < 128); ++i)  // Max of 128 texture binds. Get real.
                                        {
                                            neko_byte_buffer_read_bulkc(&cb->commands, neko_handle(neko_graphics_texture_t), v, u->size);

                                            // Get texture, also need binding, but will worry about that in a bit
                                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, v.id);

                                            // Activate texture slot
                                            glActiveTexture(GL_TEXTURE0 + binding);

                                            // Bind texture
                                            GLenum target = 0x00;
                                            switch (tex->desc.type) {
                                                default:
                                                case NEKO_GRAPHICS_TEXTURE_2D: {
                                                    target = GL_TEXTURE_2D;
                                                } break;
                                                case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
                                                    target = GL_TEXTURE_CUBE_MAP;
                                                } break;
                                            }
                                            glBindTexture(target, tex->id);

                                            binds[i] = (s32)binding++;
                                        }

                                        // Bind uniforms
                                        glUniform1iv(u->location, ct, (s32*)binds);

                                    } break;

                                    default: {
                                        // Shouldn't hit here
                                        neko_println("Assert: Bind Uniform: Invalid uniform type specified.");
                                        neko_assert(false);
                                    } break;
                                }
                            }

                        } break;

                        case NEKO_GRAPHICS_BIND_UNIFORM_BUFFER: {
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
                                neko_timed_action(1000, { neko_log_warning("Bind Uniform Buffer:Uniform %d does not exist.", id); });
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                neko_timed_action(1000, { neko_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
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
                                    neko_timed_action(1000, { neko_log_warning("Bind Uniform Buffer:Shader %d does not exist.", sid); });
                                    continue;
                                }

                                neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                // Get uniform location based on name and bound shader
                                u->location = glGetUniformBlockIndex(shader, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                // Set binding for uniform block
                                glUniformBlockBinding(shader, u->location, binding);

                                if (u->location >= UINT32_MAX) {
                                    neko_log_warning("Bind Uniform Buffer: Uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }

                                u->sid = pip->raster.shader.id;
                            }

                            glBindBufferRange(GL_UNIFORM_BUFFER, binding, u->ubo, range_offset, range_size ? range_size : u->size);

                        } break;

                        case NEKO_GRAPHICS_BIND_STORAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, sb_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Grab storage buffer from id
                            if (!sb_slot_id || !neko_slot_array_exists(ogl->storage_buffers, sb_slot_id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Storage Buffer %d does not exist.", sb_slot_id);
                                });
                                */
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                /*
                                neko_timed_action(1000, {
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
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                });
                                */
                                continue;
                            }

                            neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                            static u32 location = UINT32_MAX;

                            if ((sbo->block_idx == UINT32_MAX && sbo->block_idx != UINT32_MAX - 1)) {
                                // Get uniform location based on name and bound shader
                                CHECK_GL_CORE(sbo->block_idx = glGetProgramResourceIndex(shader, GL_SHADER_STORAGE_BLOCK, sbo->name ? sbo->name : "__EMPTY_BUFFER_NAME"); s32 params[1];
                                              GLenum props[1] = {GL_BUFFER_BINDING}; glGetProgramResourceiv(shader, GL_SHADER_STORAGE_BLOCK, sbo->block_idx, 1, props, 1, NULL, params);
                                              location = (u32)params[0];);

                                if (sbo->block_idx >= UINT32_MAX) {
                                    neko_log_warning("Bind Storage Buffer: Buffer not found: \"%s\"", sbo->name);
                                    sbo->block_idx = UINT32_MAX - 1;
                                }
                            }

                            if (sbo->block_idx < UINT32_MAX - 1) {
                                // Not sure what this actually does atm...
                                CHECK_GL_CORE(glShaderStorageBlockBinding(shader, sbo->block_idx, location););
                            }

                            // This is required
                            CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, sbo->buffer););

                        } break;

                        case NEKO_GRAPHICS_BIND_IMAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            neko_byte_buffer_readc(&cb->commands, neko_graphics_access_type, access);

                            // Grab texture from sampler id
                            if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                                neko_timed_action(1000, { neko_log_warning("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                                continue;
                            }

                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                            u32 gl_access = neko_gl_access_type_to_gl_access_type(access);
                            u32 gl_format = neko_gl_texture_format_to_gl_texture_internal_format(tex->desc.format);

                            neko_timed_action(
                                    60, neko_graphics_info_t* info = neko_graphics_info(); if (!info->compute.available) {
                                        neko_log_error("Compute shaders not available, failed to call glBindImageTexture.");
                                        continue;
                                    });

                            // Bind image texture
                            CHECK_GL_CORE(glBindImageTexture(0, tex->id, 0, GL_FALSE, 0, gl_access, gl_format);)
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

                    neko_timed_action(1000, { neko_log_warning("Pipeline %d does not exist.", pipid); });

                    continue;
                }

                // Reset cache
                neko_gl_reset_data_cache(&ogl->cache);

                // Reset state as well
                neko_gl_pipeline_state();

                /* Cache pipeline id */
                ogl->cache.pipeline = neko_handle_create(neko_graphics_pipeline_t, pipid);

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, pipid);

                /* Compute */
                // Early out if compute, since we're not doing a rasterization stage
                if (pip->compute.shader.id) {
                    /* Shader */
                    if (pip->compute.shader.id && neko_slot_array_exists(ogl->shaders, pip->compute.shader.id)) {
                        glUseProgram(neko_slot_array_get(ogl->shaders, pip->compute.shader.id));
                    } else {
                        neko_timed_action(1000, { neko_log_warning("Opengl:BindPipeline:Compute:Shader %d does not exist.", pip->compute.shader.id); });
                    }

                    continue;
                }

                /* Depth */
                if (!pip->depth.func) {
                    // If no depth function (default), then disable
                    glDisable(GL_DEPTH_TEST);
                } else {
                    glEnable(GL_DEPTH_TEST);
                    glDepthFunc(neko_gl_depth_func_to_gl_depth_func(pip->depth.func));
                }
                glDepthMask(neko_gl_depth_mask_to_gl_mask(pip->depth.mask));

                /* Stencil */
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

                /* Blend */
                if (!pip->blend.func) {
                    glDisable(GL_BLEND);
                } else {
                    glEnable(GL_BLEND);
                    glBlendEquation(neko_gl_blend_equation_to_gl_blend_eq(pip->blend.func));
                    glBlendFunc(neko_gl_blend_mode_to_gl_blend_mode(pip->blend.src, GL_ONE), neko_gl_blend_mode_to_gl_blend_mode(pip->blend.dst, GL_ZERO));
                }

                /* Raster */
                // Face culling
                if (!pip->raster.face_culling) {
                    glDisable(GL_CULL_FACE);
                } else {
                    glEnable(GL_CULL_FACE);
                    glCullFace(neko_gl_cull_face_to_gl_cull_face(pip->raster.face_culling));
                }

                // Winding order
                glFrontFace(neko_gl_winding_order_to_gl_winding_order(pip->raster.winding_order));

                /* Shader */
                if (pip->raster.shader.id && neko_slot_array_exists(ogl->shaders, pip->raster.shader.id)) {
                    glUseProgram(neko_slot_array_get(ogl->shaders, pip->raster.shader.id));
                } else {
                    neko_timed_action(1000, { neko_log_warning("Opengl:BindPipeline:Shader %d does not exist.", pip->raster.shader.id); });
                }
            } break;

            case NEKO_OPENGL_OP_DISPATCH_COMPUTE: {
                neko_byte_buffer_readc(&cb->commands, u32, num_x_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_y_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_z_groups);

                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                if (ogl->cache.pipeline.id == 0 || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                    /*
                    neko_timed_action(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // If pipeline does not have a compute state bound, then leave
                if (!pip->compute.shader.id) {
                    /*
                    neko_timed_action(1000, {
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
                    neko_timed_action(1000, { neko_println("Error:Opengl:Draw: No vertex buffer bound."); });
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
                    bool is_manual = pip->layout[i].stride | pip->layout[i].divisor | pip->layout[i].offset | vdecl.data_type == NEKO_GRAPHICS_VERTEX_DATA_NONINTERLEAVED;

                    // Bind buffer
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);

                    // Stride of vertex attribute
                    size_t stride = is_manual ? pip->layout[i].stride : neko_gl_calculate_vertex_size_in_bytes(pip->layout, neko_dyn_array_size(pip->layout));

                    // Byte offset of vertex attribute (if non-interleaved data, then grab offset from decl instead)
                    size_t offset = vdecl.data_type == NEKO_GRAPHICS_VERTEX_DATA_NONINTERLEAVED ? vdecl.offset
                                    : is_manual                                                 ? pip->layout[i].offset
                                                                                                : neko_gl_get_vertex_attr_byte_offest(pip->layout, i);

                    // If there is a vertex divisor for this layout, then we'll draw instanced
                    is_instanced |= (pip->layout[i].divisor != 0);

                    // Enable the vertex attribute pointer
                    glEnableVertexAttribArray(i);

                    switch (pip->layout[i].format) {
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:
                            glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:
                            glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:
                            glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:
                            glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:
                            glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:
                            glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:
                            glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT:
                            glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:
                            glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:
                            glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:
                            glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:
                            glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
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
                    glBindBuffer(GL_ARRAY_BUFFER, ogl->cache.vdecls[i].vbo);
                }

                // Draw based on bound primitive type in raster
                neko_byte_buffer_readc(&cb->commands, u32, start);
                neko_byte_buffer_readc(&cb->commands, u32, count);
                neko_byte_buffer_readc(&cb->commands, u32, instance_count);
                neko_byte_buffer_readc(&cb->commands, u32, base_vertex);
                neko_byte_buffer_readc(&cb->commands, u32, range_start);
                neko_byte_buffer_readc(&cb->commands, u32, range_end);

                range_end = (range_end && range_end < range_start) ? range_end : count;

                // Bind element buffer ranged
                if (ogl->cache.ibo) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, neko_slot_array_get(ogl->index_buffers, ogl->cache.ibo));
                }

                // If instance count > 1, do instanced drawing
                is_instanced |= (instance_count > 1);

                u32 prim = neko_gl_primitive_to_gl_primitive(pip->raster.primitive);
                u32 itype = neko_gl_index_buffer_size_to_gl_index_type(pip->raster.index_buffer_element_size);

                // Draw
                if (ogl->cache.ibo) {
#ifdef NEKO_GRAPHICS_IMPL_OPENGL_CORE
                    if (is_instanced)
                        glDrawElementsInstancedBaseVertex(prim, count, itype, neko_int2voidp(start), instance_count, base_vertex);
                    else
                        glDrawRangeElementsBaseVertex(prim, range_start, range_end, count, itype, neko_int2voidp(start), base_vertex);
#else
                    if (is_instanced)
                        glDrawElementsInstanced(prim, count, itype, neko_int2voidp(start), instance_count);
                    else
                        glDrawElements(prim, count, itype, neko_int2voidp(start));
#endif
                } else {
                    if (is_instanced)
                        glDrawArraysInstanced(prim, start, count, instance_count);
                    else
                        glDrawArrays(prim, start, count);
                }

            } break;

            case NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE: {
                neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                neko_byte_buffer_readc(&cb->commands, neko_graphics_texture_desc_t, desc);
                neko_byte_buffer_readc(&cb->commands, size_t, data_size);

                // Update texture with data, depending on update type (for now, just stream new data)

                // Grab texture from sampler id
                if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                    neko_timed_action(60, { neko_log_warning("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                    neko_byte_buffer_advance_position(&cb->commands, data_size);
                }

                neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                u32 int_format = neko_gl_texture_format_to_gl_texture_internal_format(desc.format);
                u32 format = neko_gl_texture_format_to_gl_texture_format(desc.format);
                u32 dt = neko_gl_texture_format_to_gl_data_type(desc.format);
                *desc.data = (cb->commands.data + cb->commands.position);
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
                neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

                // Read handle id
                neko_byte_buffer_readc(&cb->commands, u32, id);
                // Read type
                neko_byte_buffer_readc(&cb->commands, neko_graphics_buffer_type, type);
                // Read usage
                neko_byte_buffer_readc(&cb->commands, neko_graphics_buffer_usage_type, usage);
                // Read data size
                neko_byte_buffer_readc(&cb->commands, size_t, sz);
                // Read data offset
                neko_byte_buffer_readc(&cb->commands, size_t, offset);
                // Read update type
                neko_byte_buffer_readc(&cb->commands, neko_graphics_buffer_update_type, update_type);

                s32 glusage = neko_gl_buffer_usage_to_gl_enum(usage);

                switch (type) {
                    // Vertex Buffer
                    default:
                    case NEKO_GRAPHICS_BUFFER_VERTEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, id);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        switch (update_type) {
                            case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    } break;

                    case NEKO_GRAPHICS_BUFFER_INDEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, id);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                        switch (update_type) {
                            case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } break;

                    case NEKO_GRAPHICS_BUFFER_UNIFORM: {
                        // Have to
                        neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                        glBindBuffer(GL_UNIFORM_BUFFER, u->ubo);

                        switch (update_type) {
                            case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA: {
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

                    case NEKO_GRAPHICS_BUFFER_SHADER_STORAGE: {
                        neko_graphics_storage_buffer_desc_t desc = {
                                .data = cb->commands.data + cb->commands.position,
                                .size = sz,
                                .usage = usage,
                                .update =
                                        {
                                                .type = update_type,
                                                .offset = offset,
                                        },
                        };
                        neko_handle(neko_graphics_storage_buffer_t) hndl;
                        hndl.id = id;
                        neko_graphics_storage_buffer_update(hndl, &desc);

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

// fontcache

const std::string vs_source_shared = R"(
#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void ) {
    uv = vtex;
    gl_Position = vec4( vpos.xy, 0.0, 1.0 );
}
)";

const std::string fs_source_render_glyph = R"(
#version 330 core
out vec4 fragc;
void main( void ) {
    fragc = vec4( 1.0, 1.0, 1.0, 1.0 );
}
)";

const std::string fs_source_blit_atlas = R"(
#version 330 core
in vec2 uv;
out vec4 fragc;
uniform uint region;
uniform sampler2D src_texture;
float downsample( vec2 uv, vec2 texsz )
{
    float v =
        texture( src_texture, uv + vec2( 0.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 0.0f, 1.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 1.0f ) * texsz ).x * 0.25f;
    return v;
}
void main( void ) {
    const vec2 texsz = 1.0f / vec2( 2048 /* NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH */, 512 /* NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT */ );
    if ( region == 0u || region == 1u || region == 2u ) {
        float v =
            downsample( uv + vec2( -1.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2( -1.5f,  0.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f,  0.5f ) * texsz, texsz ) * 0.25f;
        fragc = vec4( 1, 1, 1, v );
    } else {
        fragc = vec4( 0, 0, 0, 1 );
    }
}
)";

const std::string vs_source_draw_text = R"(
#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void ) {
    uv = vtex;
    gl_Position = vec4( vpos.xy * 2.0f - 1.0f, 0.0, 1.0 );
}
)";
const std::string fs_source_draw_text = R"(
#version 330 core
in vec2 uv;
out vec4 fragc;
uniform sampler2D src_texture;
uniform uint downsample;
uniform vec4 colour;
void main( void ) {
    float v = texture( src_texture, uv ).x;
    if ( downsample == 1u ) {
        const vec2 texsz = 1.0f / vec2( 2048 /* NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH */, 512 /* NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT */ );
        v =
            texture( src_texture, uv + vec2(-0.5f,-0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2(-0.5f, 0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2( 0.5f,-0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2( 0.5f, 0.5f ) * texsz ).x * 0.25f;
    }
    fragc = vec4( colour.xyz, colour.a * v );
}
)";

neko_private(GLint) __neko_fontcache_compile_shader(const std::string vs, const std::string fs) {
    auto printCompileError = [&](GLuint shader) {
        char temp[4096];
        GLint compileStatus = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
        glGetShaderInfoLog(shader, 4096, NULL, temp);
        printf("%s", temp);
    };
    GLint compiled = 0;
    const char *vsrc[] = {vs.c_str()}, *fsrc[] = {fs.c_str()};
    GLint length[] = {-1};
    GLint vshader = glCreateShader(GL_VERTEX_SHADER), fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vshader, 1, vsrc, length);
    glShaderSource(fshader, 1, fsrc, length);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &compiled);
    printCompileError(vshader);
    neko_assert(compiled);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &compiled);
    printCompileError(fshader);
    neko_assert(compiled);
    GLint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glBindAttribLocation(program, 0, "vpos");
    glBindAttribLocation(program, 1, "vtexc");
    glLinkProgram(program);
    glDetachShader(program, vshader);
    glDetachShader(program, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);
    return program;
}

neko_private(void) __neko_fontcache_compile_vbo(GLuint& dest_vb, GLuint& dest_ib, const neko_fontcache_vertex* verts, int nverts, const u32* indices, int nindices) {
    if (dest_vb == 0 || dest_ib == 0) {
        GLuint buf[2];
        glGenBuffers(2, buf);
        dest_vb = buf[0], dest_ib = buf[1];
        glBindBuffer(GL_ARRAY_BUFFER, dest_vb);
        glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(neko_fontcache_vertex), verts, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dest_ib);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, nindices * sizeof(u32), indices, GL_DYNAMIC_DRAW);
    }
}

neko_private(void) __neko_fontcache_setup_fbo() {

    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    glGenFramebuffers(2, fc_data->fontcache_fbo);
    glGenTextures(2, fc_data->fontcache_fbo_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, fc_data->fontcache_fbo[0]);
    glBindTexture(GL_TEXTURE_2D, fc_data->fontcache_fbo_texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fc_data->fontcache_fbo_texture[0], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fc_data->fontcache_fbo[1]);
    glBindTexture(GL_TEXTURE_2D, fc_data->fontcache_fbo_texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fc_data->fontcache_fbo_texture[1], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void __neko_fontcache_draw() {

    // ME_profiler_scope_auto("RenderGUI.Font");

    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    neko_platform_t* platform = neko_instance()->ctx.platform;
    neko_vec2 ws = neko_platform_window_sizev(neko_platform_main_window());

    // TODO: plz!!! optimize this method
    __neko_gl_state_backup();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);

    __neko_fontcache_optimise_drawlist(&fc_data->cache);
    auto drawlist = __neko_fontcache_get_drawlist(&fc_data->cache);

    if (fc_data->vao == 0) {
        glGenVertexArrays(1, &fc_data->vao);
        glBindVertexArray(fc_data->vao);
    } else {
        glBindVertexArray(fc_data->vao);
    }

    GLuint vbo = 0, ibo = 0;
    __neko_fontcache_compile_vbo(vbo, ibo, drawlist->vertices.data(), drawlist->vertices.size(), drawlist->indices.data(), drawlist->indices.size());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(neko_fontcache_vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(neko_fontcache_vertex), (GLvoid*)(2 * sizeof(float)));

    for (auto& dcall : drawlist->dcalls) {
        if (dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH) {
            glUseProgram(fc_data->fontcache_shader_render_glyph);
            glBindFramebuffer(GL_FRAMEBUFFER, fc_data->fontcache_fbo[0]);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
            glViewport(0, 0, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);
            glScissor(0, 0, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);
            glDisable(GL_FRAMEBUFFER_SRGB);
        } else if (dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_ATLAS) {
            glUseProgram(fc_data->fontcache_shader_blit_atlas);
            glBindFramebuffer(GL_FRAMEBUFFER, fc_data->fontcache_fbo[1]);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glViewport(0, 0, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);
            glScissor(0, 0, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);
            glUniform1i(glGetUniformLocation(fc_data->fontcache_shader_blit_atlas, "src_texture"), 0);
            glUniform1ui(glGetUniformLocation(fc_data->fontcache_shader_blit_atlas, "region"), dcall.region);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fc_data->fontcache_fbo_texture[0]);
            glDisable(GL_FRAMEBUFFER_SRGB);
        } else {
            glUseProgram(fc_data->fontcache_shader_draw_text);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glViewport(0, 0, ws.x, ws.y);
            glScissor(0, 0, ws.x, ws.y);
            glUniform1i(glGetUniformLocation(fc_data->fontcache_shader_draw_text, "src_texture"), 0);
            glUniform1ui(glGetUniformLocation(fc_data->fontcache_shader_draw_text, "downsample"), dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? 1 : 0);
            glUniform4fv(glGetUniformLocation(fc_data->fontcache_shader_draw_text, "colour"), 1, dcall.colour);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? fc_data->fontcache_fbo_texture[0] : fc_data->fontcache_fbo_texture[1]);
            glEnable(GL_FRAMEBUFFER_SRGB);
        }
        if (dcall.clear_before_draw) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (dcall.end_index - dcall.start_index == 0) continue;
        glDrawElements(GL_TRIANGLES, dcall.end_index - dcall.start_index, GL_UNSIGNED_INT, (GLvoid*)(dcall.start_index * sizeof(u32)));
    }

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    // ME_CHECK_GL_ERROR();
    __neko_fontcache_flush_drawlist(&fc_data->cache);

    __neko_gl_state_restore();
}

neko_font_index __neko_fontcache_load(const void* data, size_t data_size, f32 font_size) {
    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    neko_platform_t* platform = neko_instance()->ctx.platform;
    neko_vec2 ws = neko_platform_window_sizev(neko_platform_main_window());

    __neko_fontcache_init(&fc_data->cache);
    __neko_fontcache_configure_snap(&fc_data->cache, ws.x, ws.y);

    // 返回字体索引
    return __neko_fontcache_load(&fc_data->cache, data, data_size, font_size);
}

// 将绘制字加入待绘制列表
// pos 不是屏幕坐标也不是NDC
// pos 以窗口左下角为原点 窗口空间为第一象限
void __neko_fontcache_push(const char* text, const neko_font_index font, const neko_vec2 pos) {

    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    neko_platform_t* platform = neko_instance()->ctx.platform;
    neko_vec2 ws = neko_platform_window_sizev(neko_platform_main_window());

    __neko_fontcache_configure_snap(&fc_data->cache, ws.x, ws.y);
    __neko_fontcache_draw_text(&fc_data->cache, font, text, pos.x, pos.y, 1.0f / ws.x, 1.0f / ws.y);
}

// 将屏幕窗口坐标转换为 fontcache 绘制坐标
neko_vec2 __neko_fontcache_calc_pos(f32 x, f32 y) {
    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    neko_platform_t* platform = neko_instance()->ctx.platform;
    neko_vec2 ws = neko_platform_window_sizev(neko_platform_main_window());
    f32 tmpx = x / ws.x;
    f32 tmpy = y / ws.y;
    return neko_vec2{tmpx, 1.0f - tmpy};
}

void __neko_fontcache_push_x_y(const char* text, const neko_font_index font, const f32 x, const f32 y) { __neko_fontcache_push(text, font, __neko_fontcache_calc_pos(x, y)); }

void __neko_fontcache_create() {

    // 这里 fontcache_drawing_internal_data_t 为非聚合类
    // 使用 new 与 delete 管理内存
    ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data = new fontcache_drawing_internal_data_t;

    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    fc_data->fontcache_shader_render_glyph = __neko_fontcache_compile_shader(vs_source_shared, fs_source_render_glyph);
    fc_data->fontcache_shader_blit_atlas = __neko_fontcache_compile_shader(vs_source_shared, fs_source_blit_atlas);
    fc_data->fontcache_shader_draw_text = __neko_fontcache_compile_shader(vs_source_draw_text, fs_source_draw_text);

    __neko_fontcache_setup_fbo();
}

void __neko_fontcache_shutdown() {
    fontcache_drawing_internal_data_t* fc_data = ((neko_gl_data_t*)neko_subsystem(graphics)->user_data)->fontcache_data;

    __neko_fontcache_destroy(&fc_data->cache);

    delete fc_data;
}

NEKO_API_DECL void neko_graphics_init(neko_graphics_t* graphics) {
    // Push back 0 handles into slot arrays (for 0 init validation)
    neko_gl_data_t* ogl = (neko_gl_data_t*)graphics->user_data;

    neko_slot_array_insert(ogl->shaders, 0);
    neko_slot_array_insert(ogl->vertex_buffers, 0);
    neko_slot_array_insert(ogl->index_buffers, 0);
    neko_slot_array_insert(ogl->frame_buffers, 0);

    neko_gl_uniform_list_t ul = neko_default_val();
    neko_gl_uniform_buffer_t ub = neko_default_val();
    neko_gl_pipeline_t pip = neko_default_val();
    neko_gl_renderpass_t rp = neko_default_val();
    neko_gl_texture_t tex = neko_default_val();
    neko_gl_storage_buffer_t sb = neko_default_val();

    neko_slot_array_insert(ogl->uniforms, ul);
    neko_slot_array_insert(ogl->pipelines, pip);
    neko_slot_array_insert(ogl->renderpasses, rp);
    neko_slot_array_insert(ogl->uniform_buffers, ub);
    neko_slot_array_insert(ogl->textures, tex);
    neko_slot_array_insert(ogl->storage_buffers, sb);

    // Construct vao then bind
    glGenVertexArrays(1, &ogl->cache.vao);
    glBindVertexArray(ogl->cache.vao);

    // Reset data cache for rendering ops
    neko_gl_reset_data_cache(&ogl->cache);

    // Init info object
    neko_graphics_info_t* info = &neko_subsystem(graphics)->info;

    // Major/Minor version
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);

    // Max texture units
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&info->max_texture_units);

    // Compute shader info
    CHECK_GL_CORE(info->compute.available = info->major_version >= 4 && info->minor_version >= 3; if (info->compute.available) {
        // Work group counts
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (s32*)&info->compute.max_work_group_count[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (s32*)&info->compute.max_work_group_count[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (s32*)&info->compute.max_work_group_count[2]);
        // Work group sizes
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (s32*)&info->compute.max_work_group_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (s32*)&info->compute.max_work_group_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (s32*)&info->compute.max_work_group_size[2]);
        // Work group invocations
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (s32*)&info->compute.max_work_group_invocations);
    });

    // 初始化 fontcache
    __neko_fontcache_create();

    const GLubyte* glslv = glGetString(GL_SHADING_LANGUAGE_VERSION);
    neko_log_info("GLSL Version: %s", glslv);

    // Set up all function pointers for graphics context

    // Create
    graphics->api.texture_create = neko_graphics_texture_create_impl;
    graphics->api.uniform_create = neko_graphics_uniform_create_impl;
    graphics->api.shader_create = neko_graphics_shader_create_impl;
    graphics->api.vertex_buffer_create = neko_graphics_vertex_buffer_create_impl;
    graphics->api.index_buffer_create = neko_graphics_index_buffer_create_impl;
    graphics->api.uniform_buffer_create = neko_graphics_uniform_buffer_create_impl;
    graphics->api.storage_buffer_create = neko_graphics_storage_buffer_create_impl;
    graphics->api.framebuffer_create = neko_graphics_framebuffer_create_impl;
    graphics->api.renderpass_create = neko_graphics_renderpass_create_impl;
    graphics->api.pipeline_create = neko_graphics_pipeline_create_impl;

    // Destroy
    graphics->api.texture_destroy = neko_graphics_texture_destroy_impl;
    graphics->api.uniform_destroy = neko_graphics_uniform_destroy_impl;
    graphics->api.shader_destroy = neko_graphics_shader_destroy_impl;
    graphics->api.vertex_buffer_destroy = neko_graphics_vertex_buffer_destroy_impl;
    graphics->api.index_buffer_destroy = neko_graphics_index_buffer_destroy_impl;
    graphics->api.uniform_buffer_destroy = neko_graphics_uniform_buffer_destroy_impl;
    graphics->api.storage_buffer_destroy = neko_graphics_storage_buffer_destroy_impl;
    graphics->api.framebuffer_destroy = neko_graphics_framebuffer_destroy_impl;
    graphics->api.renderpass_destroy = neko_graphics_renderpass_destroy_impl;
    graphics->api.pipeline_destroy = neko_graphics_pipeline_destroy_impl;

    // Resource Updates (main thread only)
    graphics->api.vertex_buffer_update = neko_graphics_vertex_buffer_update_impl;
    graphics->api.index_buffer_update = neko_graphics_index_buffer_update_impl;
    graphics->api.storage_buffer_update = neko_graphics_storage_buffer_update_impl;
    graphics->api.texture_update = neko_graphics_texture_update_impl;
    graphics->api.texture_read = neko_graphics_texture_read_impl;

    // Submission (Main Thread)
    graphics->api.command_buffer_submit = neko_graphics_command_buffer_submit_impl;

    /*============================================================
    // Fontcache
    ============================================================*/
    graphics->api.fontcache_create = &__neko_fontcache_create;
    graphics->api.fontcache_destroy = &__neko_fontcache_shutdown;
    graphics->api.fontcache_draw = &__neko_fontcache_draw;
    graphics->api.fontcache_load = &__neko_fontcache_load;
    graphics->api.fontcache_push = &__neko_fontcache_push;
    graphics->api.fontcache_push_x_y = &__neko_fontcache_push_x_y;
}

#endif  // NEKO_GRAPHICS_IMPL_OPENGL
#endif

#ifndef NEKO_FONTCACHE_IMPL
#define NEKO_FONTCACHE_IMPL

#include "libs/external/utf8.h"

void __neko_fontcache_init(neko_fontcache* cache) {
    neko_assert(cache);

    // Reserve global context data.
    cache->entry.reserve(8);
    cache->temp_path.reserve(256);
    cache->temp_codepoint_seen.reserve(512);
    cache->drawlist.vertices.reserve(4096);
    cache->drawlist.indices.reserve(8192);
    cache->drawlist.dcalls.reserve(512);

    // Reserve data atlas LRU regions.
    cache->atlas.next_atlas_idx_A = 0;
    cache->atlas.next_atlas_idx_B = 0;
    cache->atlas.next_atlas_idx_C = 0;
    cache->atlas.next_atlas_idx_D = 0;
    __neko_fontcache_LRU_init(cache->atlas.stateA, NEKO_FONTCACHE_ATLAS_REGION_A_CAPACITY);
    __neko_fontcache_LRU_init(cache->atlas.stateB, NEKO_FONTCACHE_ATLAS_REGION_B_CAPACITY);
    __neko_fontcache_LRU_init(cache->atlas.stateC, NEKO_FONTCACHE_ATLAS_REGION_C_CAPACITY);
    __neko_fontcache_LRU_init(cache->atlas.stateD, NEKO_FONTCACHE_ATLAS_REGION_D_CAPACITY);

    // Reserve data for shape cache. This is pretty big!
    __neko_fontcache_LRU_init(cache->shape_cache.state, NEKO_FONTCACHE_SHAPECACHE_SIZE);
    cache->shape_cache.storage.resize(NEKO_FONTCACHE_SHAPECACHE_SIZE);
    for (int i = 0; i < NEKO_FONTCACHE_SHAPECACHE_SIZE; i++) {
        cache->shape_cache.storage[i].glyphs.reserve(NEKO_FONTCACHE_SHAPECACHE_RESERNEKO_LENGTH);
        cache->shape_cache.storage[i].pos.reserve(NEKO_FONTCACHE_SHAPECACHE_RESERNEKO_LENGTH);
    }

    // We can actually go over NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH batches due to smart packing!
    cache->atlas.glyph_update_batch_drawlist.dcalls.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2);
    cache->atlas.glyph_update_batch_drawlist.vertices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 4);
    cache->atlas.glyph_update_batch_drawlist.indices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 6);
    cache->atlas.glyph_update_batch_clear_drawlist.dcalls.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2);
    cache->atlas.glyph_update_batch_clear_drawlist.vertices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 4);
    cache->atlas.glyph_update_batch_clear_drawlist.indices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 6);
}

void __neko_fontcache_destroy(neko_fontcache* cache) {
    neko_assert(cache);
    for (neko_fontcache_entry& et : cache->entry) {
        __neko_fontcache_unload(cache, et.font_id);
    }
}

ve_font_id __neko_fontcache_load(neko_fontcache* cache, const void* data, size_t data_size, float size_px) {
    neko_assert(cache);
    if (!data) return -1;

    // Allocate cache entry.
    int id = -1;
    for (int i = 0; i < cache->entry.size(); i++) {
        if (!cache->entry[i].used) {
            id = i;
            break;
        }
    }
    if (id == -1) {
        cache->entry.push_back(neko_fontcache_entry());
        id = cache->entry.size() - 1;
    }
    neko_assert(id >= 0 && id < cache->entry.size());

    // Load hb_font from memory.
    auto& et = cache->entry[id];
    int success = stbtt_InitFont(&et.info, (const unsigned char*)data, 0);
    if (!success) {
        return -1;
    }
    et.font_id = id;
    et.size = size_px;
    et.size_scale = size_px < 0.0f ? stbtt_ScaleForPixelHeight(&et.info, -size_px) : stbtt_ScaleForMappingEmToPixels(&et.info, size_px);
    et.used = true;

    return id;
}

ve_font_id __neko_fontcache_loadfile(neko_fontcache* cache, const char* filename, std::vector<uint8_t>& buffer, float size_px) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer.resize(sz);

    if (fread(buffer.data(), 1, sz, fp) != sz) {
        buffer.clear();
        fclose(fp);
        return -1;
    }
    fclose(fp);

    ve_font_id ret = __neko_fontcache_load(cache, buffer.data(), buffer.size(), size_px);
    return ret;
}

void __neko_fontcache_unload(neko_fontcache* cache, ve_font_id font) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());

    auto& et = cache->entry[font];
    et.used = false;
}

void __neko_fontcache_configure_snap(neko_fontcache* cache, u32 snap_width, u32 snap_height) {
    neko_assert(cache);
    cache->snap_width = snap_width;
    cache->snap_height = snap_height;
}

neko_fontcache_drawlist* __neko_fontcache_get_drawlist(neko_fontcache* cache) {
    neko_assert(cache);
    return &cache->drawlist;
}

void neko_fontcache_clear_drawlist(neko_fontcache_drawlist& drawlist) {
    drawlist.dcalls.clear();
    drawlist.indices.clear();
    drawlist.vertices.clear();
}

void neko_fontcache_merge_drawlist(neko_fontcache_drawlist& dest, const neko_fontcache_drawlist& src) {
    int voffset = dest.vertices.size();
    for (int i = 0; i < src.vertices.size(); i++) {
        dest.vertices.push_back(src.vertices[i]);
    }
    int ioffset = dest.indices.size();
    for (int i = 0; i < src.indices.size(); i++) {
        dest.indices.push_back(src.indices[i] + voffset);
    }
    for (int i = 0; i < src.dcalls.size(); i++) {
        neko_fontcache_draw dcall = src.dcalls[i];
        dcall.start_index += ioffset;
        dcall.end_index += ioffset;
        dest.dcalls.push_back(dcall);
    }
}

void __neko_fontcache_flush_drawlist(neko_fontcache* cache) {
    neko_assert(cache);
    neko_fontcache_clear_drawlist(cache->drawlist);
}

inline neko_vec2 neko_fontcache_eval_bezier(neko_vec2 p0, neko_vec2 p1, neko_vec2 p2, float t) {
    float t2 = t * t, c0 = (1.0f - t) * (1.0f - t), c1 = 2.0f * (1.0f - t) * t, c2 = t2;
    return neko_fontcache_make_vec2(c0 * p0.x + c1 * p1.x + c2 * p2.x, c0 * p0.y + c1 * p1.y + c2 * p2.y);
}

inline neko_vec2 neko_fontcache_eval_bezier(neko_vec2 p0, neko_vec2 p1, neko_vec2 p2, neko_vec2 p3, float t) {
    float t2 = t * t, t3 = t2 * t;
    float c0 = (1.0f - t) * (1.0f - t) * (1.0f - t), c1 = 3.0f * (1.0f - t) * (1.0f - t) * t, c2 = 3.0f * (1.0f - t) * t2, c3 = t3;
    return neko_fontcache_make_vec2(c0 * p0.x + c1 * p1.x + c2 * p2.x + c3 * p3.x, c0 * p0.y + c1 * p1.y + c2 * p2.y + c3 * p3.y);
}

// WARNING: doesn't actually append drawcall; caller is responsible for actually appending the drawcall.
void neko_fontcache_draw_filled_path(neko_fontcache_drawlist& drawlist, neko_vec2 outside, std::vector<neko_vec2>& path, float scaleX = 1.0f, float scaleY = 1.0f, float translateX = 0.0f,
                                     float translateY = 0.0f) {
#ifdef NEKO_FONTCACHE_DEBUGPRINT_VERBOSE
    printf("outline_path: \n");
    for (int i = 0; i < path.size(); i++) {
        printf("    %.2f %.2f\n", path[i].x * scaleX + translateX, path[i].y * scaleY + +translateY);
    }
#endif  // NEKO_FONTCACHE_DEBUGPRINT_VERBOSE

    int voffset = drawlist.vertices.size();
    for (int i = 0; i < path.size(); i++) {
        neko_fontcache_vertex vertex;
        vertex.x = path[i].x * scaleX + translateX;
        vertex.y = path[i].y * scaleY + +translateY;
        vertex.u = 0.0f;
        vertex.v = 0.0f;
        drawlist.vertices.push_back(vertex);
    }
    int voutside = drawlist.vertices.size();
    {
        neko_fontcache_vertex vertex;
        vertex.x = outside.x * scaleX + translateX;
        vertex.y = outside.y * scaleY + +translateY;
        vertex.u = 0.0f;
        vertex.v = 0.0f;
        drawlist.vertices.push_back(vertex);
    }
    for (int i = 1; i < path.size(); i++) {
        drawlist.indices.push_back(voutside);
        drawlist.indices.push_back(voffset + i - 1);
        drawlist.indices.push_back(voffset + i);
    }
}

// WARNING: doesn't actually append drawcall; caller is responsible for actually appending the drawcall.
void neko_fontcache_blit_quad(neko_fontcache_drawlist& drawlist, float x0 = 0.0f, float y0 = 0.0f, float x1 = 1.0f, float y1 = 1.0f, float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f,
                              float v1 = 1.0f) {
    int voffset = drawlist.vertices.size();

    neko_fontcache_vertex vertex;
    vertex.x = x0;
    vertex.y = y0;
    vertex.u = u0;
    vertex.v = v0;
    drawlist.vertices.push_back(vertex);
    vertex.x = x0;
    vertex.y = y1;
    vertex.u = u0;
    vertex.v = v1;
    drawlist.vertices.push_back(vertex);
    vertex.x = x1;
    vertex.y = y0;
    vertex.u = u1;
    vertex.v = v0;
    drawlist.vertices.push_back(vertex);
    vertex.x = x1;
    vertex.y = y1;
    vertex.u = u1;
    vertex.v = v1;
    drawlist.vertices.push_back(vertex);

    static u32 quad_indices[] = {0, 1, 2, 2, 1, 3};
    for (int i = 0; i < 6; i++) {
        drawlist.indices.push_back(voffset + quad_indices[i]);
    }
}

bool __neko_fontcache_cache_glyph(neko_fontcache* cache, ve_font_id font, ve_glyph glyph_index, float scaleX, float scaleY, float translateX, float translateY) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());
    neko_fontcache_entry& entry = cache->entry[font];

    if (!glyph_index) {
        // Glyph not in current hb_font.
        return false;
    }

    // Retrieve the shape definition from STB TrueType.
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return true;
    stbtt_vertex* shape = nullptr;
    int nverts = stbtt_GetGlyphShape(&entry.info, glyph_index, &shape);
    if (!nverts || !shape) {
        return false;
    }

#ifdef NEKO_FONTCACHE_DEBUGPRINT_VERBOSE
    printf("shape: \n");
    for (int i = 0; i < nverts; i++) {
        if (shape[i].type == STBTT_vmove) {
            printf("    move_to %d %d\n", shape[i].x, shape[i].y);
        } else if (shape[i].type == STBTT_vline) {
            printf("    line_to %d %d\n", shape[i].x, shape[i].y);
        } else if (shape[i].type == STBTT_vcurve) {
            printf("    curve_to %d %d through %d %d\n", shape[i].x, shape[i].y, shape[i].cx, shape[i].cy);
        } else if (shape[i].type == STBTT_vcubic) {
            printf("    cubic_to %d %d through %d %d and %d %d\n", shape[i].x, shape[i].y, shape[i].cx, shape[i].cy, shape[i].cx1, shape[i].cy1);
        }
    }
#endif  // NEKO_FONTCACHE_DEBUGPRINT_VERBOSE

    // We need a random point that is outside our shape. We simply pick something diagonally across from top-left bound corner.
    // Note that this outside point is scaled alongside the glyph in neko_fontcache_draw_filled_path, so we don't need to handle that here.
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    neko_assert(success);
    neko_vec2 outside = neko_fontcache_make_vec2(bounds_x0 - 21, bounds_y0 - 33);

    // Figure out scaling so it fits within our box.
    neko_fontcache_draw draw;
    draw.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH;
    draw.start_index = cache->drawlist.indices.size();

    // Draw the path using simplified version of https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac.
    // Instead of involving fragment shader code we simply make use of modern GPU ability to crunch triangles and brute force curve definitions.
    //
    std::vector<neko_vec2>& path = cache->temp_path;
    path.clear();
    for (int i = 0; i < nverts; i++) {
        stbtt_vertex& edge = shape[i];
        switch (edge.type) {
            case STBTT_vmove:
                if (path.size() > 0) {
                    neko_fontcache_draw_filled_path(cache->drawlist, outside, path, scaleX, scaleY, translateX, translateY);
                }
                path.clear();
                // Fallthrough.
            case STBTT_vline:
                path.push_back(neko_fontcache_make_vec2(shape[i].x, shape[i].y));
                break;
            case STBTT_vcurve: {
                neko_assert(path.size() > 0);
                neko_vec2 p0 = path[path.size() - 1];
                neko_vec2 p1 = neko_fontcache_make_vec2(shape[i].cx, shape[i].cy);
                neko_vec2 p2 = neko_fontcache_make_vec2(shape[i].x, shape[i].y);

                float step = 1.0f / NEKO_FONTCACHE_CURNEKO_QUALITY, t = step;
                for (int i = 0; i < NEKO_FONTCACHE_CURNEKO_QUALITY; i++) {
                    path.push_back(neko_fontcache_eval_bezier(p0, p1, p2, t));
                    t += step;
                }
                break;
            }
            case STBTT_vcubic: {
                neko_assert(path.size() > 0);
                neko_vec2 p0 = path[path.size() - 1];
                neko_vec2 p1 = neko_fontcache_make_vec2(shape[i].cx, shape[i].cy);
                neko_vec2 p2 = neko_fontcache_make_vec2(shape[i].cx1, shape[i].cy1);
                neko_vec2 p3 = neko_fontcache_make_vec2(shape[i].x, shape[i].y);

                float step = 1.0f / NEKO_FONTCACHE_CURNEKO_QUALITY, t = step;
                for (int i = 0; i < NEKO_FONTCACHE_CURNEKO_QUALITY; i++) {
                    path.push_back(neko_fontcache_eval_bezier(p0, p1, p2, p3, t));
                    t += step;
                }
                break;
            }
            default:
                neko_assert(!"Unknown shape edge type.");
        }
    }
    if (path.size() > 0) {
        neko_fontcache_draw_filled_path(cache->drawlist, outside, path, scaleX, scaleY, translateX, translateY);
    }

    // Append the draw call.
    draw.end_index = cache->drawlist.indices.size();
    if (draw.end_index > draw.start_index) {
        cache->drawlist.dcalls.push_back(draw);
    }

    stbtt_FreeShape(&entry.info, shape);
    return true;
}

static ve_atlas_region neko_fontcache_decide_codepoint_region(neko_fontcache* cache, neko_fontcache_entry& entry, int glyph_index, neko_fontcache_LRU*& state, u32*& next_idx, float& oversample_x,
                                                              float& oversample_y) {
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return '\0';

    // Get hb_font text metrics. These are unscaled!
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    int bounds_width = bounds_x1 - bounds_x0, bounds_height = bounds_y1 - bounds_y0;
    neko_assert(success);

    // Decide which atlas to target. This logic should work well for reasonable on-screen text sizes of around 24px.
    // For 4k+ displays, caching hb_font at a lower pt and drawing it upscaled at a higher pt is recommended.
    //
    ve_atlas_region region;
    float bwidth_scaled = bounds_width * entry.size_scale + 2.0f * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING, bheight_scaled = bounds_height * entry.size_scale + 2.0f * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH && bheight_scaled <= NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT) {
        // Region A for small glyphs. These are good for things such as punctuation.
        region = 'A';
        state = &cache->atlas.stateA;
        next_idx = &cache->atlas.next_atlas_idx_A;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH && bheight_scaled > NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT) {
        // Region B for tall glyphs. These are good for things such as european alphabets.
        region = 'B';
        state = &cache->atlas.stateB;
        next_idx = &cache->atlas.next_atlas_idx_B;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH && bheight_scaled <= NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT) {
        // Region C for big glyphs. These are good for things such as asian typography.
        region = 'C';
        state = &cache->atlas.stateC;
        next_idx = &cache->atlas.next_atlas_idx_C;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH && bheight_scaled <= NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT) {
        // Region D for huge glyphs. These are good for things such as titles and 4k.
        region = 'D';
        state = &cache->atlas.stateD;
        next_idx = &cache->atlas.next_atlas_idx_D;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH && bheight_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT) {
        // Region 'E' for massive glyphs. These are rendered uncached and un-oversampled.
        region = 'E';
        state = nullptr;
        next_idx = nullptr;
        if (bwidth_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH / 2 && bheight_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT / 2) {
            oversample_x = 2.0f;
            oversample_y = 2.0f;
        } else {
            oversample_x = 1.0f;
            oversample_y = 1.0f;
        }
        return region;
    } else {
        return '\0';
    }

    neko_assert(state);
    neko_assert(next_idx);

    return region;
}

static void neko_fontcache_flush_glyph_buffer_to_atlas(neko_fontcache* cache) {
    // Flush drawcalls to draw list.
    neko_fontcache_merge_drawlist(cache->drawlist, cache->atlas.glyph_update_batch_clear_drawlist);
    neko_fontcache_merge_drawlist(cache->drawlist, cache->atlas.glyph_update_batch_drawlist);
    neko_fontcache_clear_drawlist(cache->atlas.glyph_update_batch_clear_drawlist);
    neko_fontcache_clear_drawlist(cache->atlas.glyph_update_batch_drawlist);

    // Clear glyph_update_FBO.
    if (cache->atlas.glyph_update_batch_x != 0) {
        neko_fontcache_draw dcall;
        dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH;
        dcall.start_index = 0;
        dcall.end_index = 0;
        dcall.clear_before_draw = true;
        cache->drawlist.dcalls.push_back(dcall);
        cache->atlas.glyph_update_batch_x = 0;
    }
}

static void neko_fontcache_screenspace_xform(float& x, float& y, float& scalex, float& scaley, float width, float height) {
    scalex /= width;
    scaley /= height;
    scalex *= 2.0f;
    scaley *= 2.0f;
    x *= (2.0f / width);
    y *= (2.0f / height);
    x -= 1.0f;
    y -= 1.0f;
}

static void neko_fontcache_texspace_xform(float& x, float& y, float& scalex, float& scaley, float width, float height) {
    x /= width;
    y /= height;
    scalex /= width;
    scaley /= height;
}

static void neko_fontcache_atlas_bbox(ve_atlas_region region, int local_idx, float& x, float& y, float& width, float& height) {
    if (region == 'A') {
        width = NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_A_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_A_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_A_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_A_YOFFSET;
    } else if (region == 'B') {
        width = NEKO_FONTCACHE_ATLAS_REGION_B_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_B_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_B_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_B_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_B_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_B_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_B_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_B_YOFFSET;
    } else if (region == 'C') {
        width = NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_C_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_C_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_C_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_C_YOFFSET;
    } else {
        neko_assert(region == 'D');
        width = NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_D_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_D_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_D_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_D_YOFFSET;
    }
}

void neko_fontcache_cache_glyph_to_atlas(neko_fontcache* cache, ve_font_id font, ve_glyph glyph_index) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());
    neko_fontcache_entry& entry = cache->entry[font];

    if (!glyph_index) {
        // Glyph not in current hb_font.
        return;
    }
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return;

    // Get hb_font text metrics. These are unscaled!
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    int bounds_width = bounds_x1 - bounds_x0, bounds_height = bounds_y1 - bounds_y0;
    neko_assert(success);

    // Decide which atlas to target.
    neko_fontcache_LRU* state = nullptr;
    u32* next_idx = nullptr;
    float oversample_x = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X, oversample_y = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y;
    ve_atlas_region region = neko_fontcache_decide_codepoint_region(cache, entry, glyph_index, state, next_idx, oversample_x, oversample_y);

    // E region is special case and not cached to atlas.
    if (region == '\0' || region == 'E') return;

    // Grab an atlas LRU cache slot.
    uint64_t lru_code = glyph_index + ((0x100000000ULL * font) & 0xFFFFFFFF00000000ULL);
    int atlas_index = __neko_fontcache_LRU_get(*state, lru_code);
    if (atlas_index == -1) {
        if (*next_idx < state->capacity) {
            uint64_t evicted = __neko_fontcache_LRU_put(*state, lru_code, *next_idx);
            atlas_index = *next_idx;
            (*next_idx)++;
            neko_assert(evicted == lru_code);
        } else {
            uint64_t next_evict_codepoint = __neko_fontcache_LRU_get_next_evicted(*state);
            neko_assert(next_evict_codepoint != 0xFFFFFFFFFFFFFFFFULL);
            atlas_index = __neko_fontcache_LRU_peek(*state, next_evict_codepoint);
            neko_assert(atlas_index != -1);
            uint64_t evicted = __neko_fontcache_LRU_put(*state, lru_code, atlas_index);
            neko_assert(evicted == next_evict_codepoint);
        }
        neko_assert(__neko_fontcache_LRU_get(*state, lru_code) != -1);
    }

#ifdef NEKO_FONTCACHE_DEBUGPRINT
    static int debug_total_cached = 0;
    printf("glyph 0x%x( %c ) caching to atlas region %c at idx %d. %d total glyphs cached.\n", unicode, (char)unicode, (char)region, atlas_index, debug_total_cached++);
#endif  // NEKO_FONTCACHE_DEBUGPRINT

    // Draw oversized glyph to update FBO.
    float glyph_draw_scale_x = entry.size_scale * oversample_x;
    float glyph_draw_scale_y = entry.size_scale * oversample_y;
    float glyph_draw_translate_x = -bounds_x0 * glyph_draw_scale_x + NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    float glyph_draw_translate_y = -bounds_y0 * glyph_draw_scale_y + NEKO_FONTCACHE_GLYPHDRAW_PADDING;

    glyph_draw_translate_x = (int)(glyph_draw_translate_x + 0.9999999f);
    glyph_draw_translate_y = (int)(glyph_draw_translate_y + 0.9999999f);

    // Allocate a glyph_update_FBO region.
    int gdwidth_scaled_px = (int)(bounds_width * glyph_draw_scale_x + 1.0f) + 2 * oversample_x * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    if (cache->atlas.glyph_update_batch_x + gdwidth_scaled_px >= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH) {
        neko_fontcache_flush_glyph_buffer_to_atlas(cache);
    }

    // Calculate the src and destination regions.

    float destx, desty, destw, desth, srcx, srcy, srcw, srch;
    neko_fontcache_atlas_bbox(region, atlas_index, destx, desty, destw, desth);
    float dest_glyph_x = destx + NEKO_FONTCACHE_ATLAS_GLYPH_PADDING, dest_glyph_y = desty + NEKO_FONTCACHE_ATLAS_GLYPH_PADDING, dest_glyph_w = bounds_width * entry.size_scale,
          dest_glyph_h = bounds_height * entry.size_scale;
    dest_glyph_x -= NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    dest_glyph_y -= NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    dest_glyph_w += 2 * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    dest_glyph_h += 2 * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    neko_fontcache_screenspace_xform(dest_glyph_x, dest_glyph_y, dest_glyph_w, dest_glyph_h, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);
    neko_fontcache_screenspace_xform(destx, desty, destw, desth, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);

    srcx = cache->atlas.glyph_update_batch_x;
    srcy = 0.0;
    srcw = bounds_width * glyph_draw_scale_x;
    srch = bounds_height * glyph_draw_scale_y;
    srcw += 2 * oversample_x * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    srch += 2 * oversample_y * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    neko_fontcache_texspace_xform(srcx, srcy, srcw, srch, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // Advance glyph_update_batch_x and calcuate final glyph drawing transform.
    glyph_draw_translate_x += cache->atlas.glyph_update_batch_x;
    cache->atlas.glyph_update_batch_x += gdwidth_scaled_px;
    neko_fontcache_screenspace_xform(glyph_draw_translate_x, glyph_draw_translate_y, glyph_draw_scale_x, glyph_draw_scale_y, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH,
                                     NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // Queue up clear on target region on atlas.
    neko_fontcache_draw dcall;
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_ATLAS;
    dcall.region = (u32)(-1);
    dcall.start_index = cache->atlas.glyph_update_batch_clear_drawlist.indices.size();
    neko_fontcache_blit_quad(cache->atlas.glyph_update_batch_clear_drawlist, destx, desty, destx + destw, desty + desth, 1.0f, 1.0f, 1.0f, 1.0f);
    dcall.end_index = cache->atlas.glyph_update_batch_clear_drawlist.indices.size();
    cache->atlas.glyph_update_batch_clear_drawlist.dcalls.push_back(dcall);

    // Queue up a blit from glyph_update_FBO to the atlas.
    dcall.region = (u32)(0);
    dcall.start_index = cache->atlas.glyph_update_batch_drawlist.indices.size();
    neko_fontcache_blit_quad(cache->atlas.glyph_update_batch_drawlist, dest_glyph_x, dest_glyph_y, destx + dest_glyph_w, desty + dest_glyph_h, srcx, srcy, srcx + srcw, srcy + srch);
    dcall.end_index = cache->atlas.glyph_update_batch_drawlist.indices.size();
    cache->atlas.glyph_update_batch_drawlist.dcalls.push_back(dcall);

    // the<engine>().eng()-> glyph to glyph_update_FBO.
    __neko_fontcache_cache_glyph(cache, font, glyph_index, glyph_draw_scale_x, glyph_draw_scale_y, glyph_draw_translate_x, glyph_draw_translate_y);
}

void neko_fontcache_shape_text_uncached(neko_fontcache* cache, ve_font_id font, neko_fontcache_shaped_text& output, const std::string& text_utf8) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());

    bool use_full_text_shape = cache->text_shape_advanced;
    neko_fontcache_entry& entry = cache->entry[font];
    output.glyphs.clear();
    output.pos.clear();

    int ascent = 0, descent = 0, line_gap = 0;
    stbtt_GetFontVMetrics(&entry.info, &ascent, &descent, &line_gap);

    cache->text_shape_advanced = false;

    // We use our own fallback dumbass text shaping.
    // WARNING: PLEASE USE HARFBUZZ. GOOD TEXT SHAPING IS IMPORTANT FOR INTERNATIONALISATION.

    utf8_int32_t codepoint, prev_codepoint = 0;
    size_t u32_length = utf8len(text_utf8.data());
    output.glyphs.reserve(u32_length);
    output.pos.reserve(u32_length);

    float pos = 0.0f, vpos = 0.0f;
    int advance = 0, to_left_side_glyph = 0;

    // Loop through text and shape.
    for (void* v = utf8codepoint(text_utf8.data(), &codepoint); codepoint; v = utf8codepoint(v, &codepoint)) {
        if (prev_codepoint) {
            int kern = stbtt_GetCodepointKernAdvance(&entry.info, prev_codepoint, codepoint);
            pos += kern * entry.size_scale;
        }
        if (codepoint == '\n') {
            pos = 0.0f;
            vpos -= (ascent - descent + line_gap) * entry.size_scale;
            vpos = (int)(vpos + 0.5f);
            prev_codepoint = 0;
            continue;
        }
        if (std::abs(entry.size) <= NEKO_FONTCACHE_ADVANCE_SNAP_SMALLFONT_SIZE) {
            // Expand advance to closest pixel for hb_font small sizes.
            pos = std::ceilf(pos);
        }

        output.glyphs.push_back(stbtt_FindGlyphIndex(&entry.info, codepoint));
        stbtt_GetCodepointHMetrics(&entry.info, codepoint, &advance, &to_left_side_glyph);
        output.pos.push_back(neko_fontcache_make_vec2(int(pos + 0.5), vpos));

        float adv = advance * entry.size_scale;

        pos += adv;
        prev_codepoint = codepoint;
    }

    output.end_cursor_pos.x = pos;
    output.end_cursor_pos.y = vpos;
}

template <typename T>
void neko_fontcache_ELFhash64(uint64_t& hash, const T* ptr, size_t count = 1) {
    uint64_t x = 0;
    auto bytes = reinterpret_cast<const uint8_t*>(ptr);

    for (int i = 0; i < sizeof(T) * count; i++) {
        hash = (hash << 4) + bytes[i];
        if ((x = hash & 0xF000000000000000ULL) != 0) {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }
}

static neko_fontcache_shaped_text& neko_fontcache_shape_text_cached(neko_fontcache* cache, ve_font_id font, const std::string& text_utf8) {
    uint64_t hash = 0x9f8e00d51d263c24ULL;
    neko_fontcache_ELFhash64(hash, (const uint8_t*)text_utf8.data(), text_utf8.size());
    neko_fontcache_ELFhash64(hash, &font);

    neko_fontcache_LRU& state = cache->shape_cache.state;
    int shape_cache_idx = __neko_fontcache_LRU_get(state, hash);
    if (shape_cache_idx == -1) {
        if (cache->shape_cache.next_cache_idx < state.capacity) {
            shape_cache_idx = cache->shape_cache.next_cache_idx++;
            __neko_fontcache_LRU_put(state, hash, shape_cache_idx);
        } else {
            uint64_t next_evict_idx = __neko_fontcache_LRU_get_next_evicted(state);
            neko_assert(next_evict_idx != 0xFFFFFFFFFFFFFFFFULL);
            shape_cache_idx = __neko_fontcache_LRU_peek(state, next_evict_idx);
            neko_assert(shape_cache_idx != -1);
            __neko_fontcache_LRU_put(state, hash, shape_cache_idx);
        }
        neko_fontcache_shape_text_uncached(cache, font, cache->shape_cache.storage[shape_cache_idx], text_utf8);
    }

    return cache->shape_cache.storage[shape_cache_idx];
}

static void neko_fontcache_directly_draw_massive_glyph(neko_fontcache* cache, neko_fontcache_entry& entry, ve_glyph glyph, int bounds_x0, int bounds_y0, int bounds_width, int bounds_height,
                                                       float oversample_x = 1.0f, float oversample_y = 1.0f, float posx = 0.0f, float posy = 0.0f, float scalex = 1.0f, float scaley = 1.0f) {
    // Flush out whatever was in the glyph buffer beforehand to atlas.
    neko_fontcache_flush_glyph_buffer_to_atlas(cache);

    // Draw un-antialiased glyph to update FBO.
    float glyph_draw_scale_x = entry.size_scale * oversample_x;
    float glyph_draw_scale_y = entry.size_scale * oversample_y;
    float glyph_draw_translate_x = -bounds_x0 * glyph_draw_scale_x + NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    float glyph_draw_translate_y = -bounds_y0 * glyph_draw_scale_y + NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    neko_fontcache_screenspace_xform(glyph_draw_translate_x, glyph_draw_translate_y, glyph_draw_scale_x, glyph_draw_scale_y, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH,
                                     NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // the<engine>().eng()-> glyph to glyph_update_FBO.
    __neko_fontcache_cache_glyph(cache, entry.font_id, glyph, glyph_draw_scale_x, glyph_draw_scale_y, glyph_draw_translate_x, glyph_draw_translate_y);

    // Figure out the source rect.
    float glyph_x = 0.0f, glyph_y = 0.0f, glyph_w = bounds_width * entry.size_scale * oversample_x, glyph_h = bounds_height * entry.size_scale * oversample_y;
    float glyph_dest_w = bounds_width * entry.size_scale, glyph_dest_h = bounds_height * entry.size_scale;
    glyph_w += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_h += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_dest_w += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_dest_h += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;

    // Figure out the destination rect.
    float bounds_x0_scaled = int(bounds_x0 * entry.size_scale - 0.5f);
    float bounds_y0_scaled = int(bounds_y0 * entry.size_scale - 0.5f);
    float dest_x = posx + scalex * bounds_x0_scaled;
    float dest_y = posy + scaley * bounds_y0_scaled;
    float dest_w = scalex * glyph_dest_w, dest_h = scaley * glyph_dest_h;
    dest_x -= scalex * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    dest_y -= scaley * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    neko_fontcache_texspace_xform(glyph_x, glyph_y, glyph_w, glyph_h, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // Add the glyph drawcall.
    neko_fontcache_draw dcall;
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED;
    dcall.colour[0] = cache->colour[0];
    dcall.colour[1] = cache->colour[1];
    dcall.colour[2] = cache->colour[2];
    dcall.colour[3] = cache->colour[3];
    dcall.start_index = cache->drawlist.indices.size();
    neko_fontcache_blit_quad(cache->drawlist, dest_x, dest_y, dest_x + dest_w, dest_y + dest_h, glyph_x, glyph_y, glyph_x + glyph_w, glyph_y + glyph_h);
    dcall.end_index = cache->drawlist.indices.size();
    cache->drawlist.dcalls.push_back(dcall);

    // Clear glyph_update_FBO.
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH;
    dcall.start_index = 0;
    dcall.end_index = 0;
    dcall.clear_before_draw = true;
    cache->drawlist.dcalls.push_back(dcall);
}

static bool neko_fontcache_empty(neko_fontcache* cache, neko_fontcache_entry& entry, ve_glyph glyph_index) {
    if (!glyph_index) {
        // Glyph not in current hb_font.
        return true;
    }
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return true;
    return false;
}

// This function only draws codepoints that have been drawn. Returns false without drawing anything if uncached.
bool neko_fontcache_draw_cached_glyph(neko_fontcache* cache, neko_fontcache_entry& entry, ve_glyph glyph_index, float posx = 0.0f, float posy = 0.0f, float scalex = 1.0f, float scaley = 1.0f) {
    if (!glyph_index) {
        // Glyph not in current hb_font.
        return true;
    }
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return true;

    // Get hb_font text metrics. These are unscaled!
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    int bounds_width = bounds_x1 - bounds_x0, bounds_height = bounds_y1 - bounds_y0;
    neko_assert(success);

    // Decide which atlas to target.
    neko_fontcache_LRU* state = nullptr;
    u32* next_idx = nullptr;
    float oversample_x = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X, oversample_y = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y;
    ve_atlas_region region = neko_fontcache_decide_codepoint_region(cache, entry, glyph_index, state, next_idx, oversample_x, oversample_y);

    // E region is special case and not cached to atlas.
    if (region == 'E') {
        neko_fontcache_directly_draw_massive_glyph(cache, entry, glyph_index, bounds_x0, bounds_y0, bounds_width, bounds_height, oversample_x, oversample_y, posx, posy, scalex, scaley);
        return true;
    }

    // Is this codepoint cached?
    uint64_t lru_code = glyph_index + ((0x100000000ULL * entry.font_id) & 0xFFFFFFFF00000000ULL);
    int atlas_index = __neko_fontcache_LRU_get(*state, lru_code);
    if (atlas_index == -1) {
        return false;
    }

    // Figure out the source bounding box in atlas texture.
    float atlas_x, atlas_y, atlas_w, atlas_h;
    neko_fontcache_atlas_bbox(region, atlas_index, atlas_x, atlas_y, atlas_w, atlas_h);
    float glyph_x = atlas_x, glyph_y = atlas_y, glyph_w = bounds_width * entry.size_scale, glyph_h = bounds_height * entry.size_scale;
    glyph_w += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_h += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;

    // Figure out the destination rect.
    float bounds_x0_scaled = int(bounds_x0 * entry.size_scale - 0.5f);
    float bounds_y0_scaled = int(bounds_y0 * entry.size_scale - 0.5f);
    float dest_x = posx + scalex * bounds_x0_scaled;
    float dest_y = posy + scaley * bounds_y0_scaled;
    float dest_w = scalex * glyph_w, dest_h = scaley * glyph_h;
    dest_x -= scalex * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    dest_y -= scaley * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    neko_fontcache_texspace_xform(glyph_x, glyph_y, glyph_w, glyph_h, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);

    // Add the glyph drawcall.
    neko_fontcache_draw dcall;
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET;
    dcall.colour[0] = cache->colour[0];
    dcall.colour[1] = cache->colour[1];
    dcall.colour[2] = cache->colour[2];
    dcall.colour[3] = cache->colour[3];
    dcall.start_index = cache->drawlist.indices.size();
    neko_fontcache_blit_quad(cache->drawlist, dest_x, dest_y, dest_x + dest_w, dest_y + dest_h, glyph_x, glyph_y, glyph_x + glyph_w, glyph_y + glyph_h);
    dcall.end_index = cache->drawlist.indices.size();
    cache->drawlist.dcalls.push_back(dcall);

    return true;
}

static void neko_fontcache_reset_batch_codepoint_state(neko_fontcache* cache) {
    cache->temp_codepoint_seen.clear();
    cache->temp_codepoint_seen.reserve(256);
}

static bool neko_fontcache_can_batch_glyph(neko_fontcache* cache, ve_font_id font, neko_fontcache_entry& entry, ve_glyph glyph_index) {
    neko_assert(cache);
    neko_assert(entry.font_id == font);

    // Decide which atlas to target.
    neko_assert(glyph_index != -1);
    neko_fontcache_LRU* state = nullptr;
    u32* next_idx = nullptr;
    float oversample_x = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X, oversample_y = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y;
    ve_atlas_region region = neko_fontcache_decide_codepoint_region(cache, entry, glyph_index, state, next_idx, oversample_x, oversample_y);

    // E region can't batch.
    if (region == 'E' || region == '\0') return false;
    if (cache->temp_codepoint_seen.size() > 1024) return false;

    // Is this glyph cached?
    uint64_t lru_code = glyph_index + ((0x100000000ULL * entry.font_id) & 0xFFFFFFFF00000000ULL);
    int atlas_index = __neko_fontcache_LRU_get(*state, lru_code);
    if (atlas_index == -1) {
        if (*next_idx >= state->capacity) {
            // We will evict LRU. We must predict which LRU will get evicted, and if it's something we've seen then we need to take slowpath and flush batch.
            uint64_t next_evict_codepoint = __neko_fontcache_LRU_get_next_evicted(*state);
            if (cache->temp_codepoint_seen[next_evict_codepoint]) {
                return false;
            }
        }
        neko_fontcache_cache_glyph_to_atlas(cache, font, glyph_index);
    }

    neko_assert(__neko_fontcache_LRU_get(*state, lru_code) != -1);
    cache->temp_codepoint_seen[lru_code] = true;

    return true;
}

static void neko_fontcache_draw_text_batch(neko_fontcache* cache, neko_fontcache_entry& entry, neko_fontcache_shaped_text& shaped, int batch_start_idx, int batch_end_idx, float posx, float posy,
                                           float scalex, float scaley) {
    neko_fontcache_flush_glyph_buffer_to_atlas(cache);
    for (int j = batch_start_idx; j < batch_end_idx; j++) {
        ve_glyph glyph_index = shaped.glyphs[j];
        float glyph_translate_x = posx + shaped.pos[j].x * scalex, glyph_translate_y = posy + shaped.pos[j].y * scaley;
        bool glyph_cached = neko_fontcache_draw_cached_glyph(cache, entry, glyph_index, glyph_translate_x, glyph_translate_y, scalex, scaley);
        neko_assert(glyph_cached);
    }
}

bool __neko_fontcache_draw_text(neko_fontcache* cache, ve_font_id font, const std::string& text_utf8, float posx, float posy, float scalex, float scaley) {
    neko_fontcache_shaped_text& shaped = neko_fontcache_shape_text_cached(cache, font, text_utf8);

    if (cache->snap_width) posx = ((int)(posx * cache->snap_width + 0.5f)) / (float)cache->snap_width;
    if (cache->snap_height) posy = ((int)(posy * cache->snap_height + 0.5f)) / (float)cache->snap_height;

    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());
    neko_fontcache_entry& entry = cache->entry[font];

    int batch_start_idx = 0;
    for (int i = 0; i < shaped.glyphs.size(); i++) {
        ve_glyph glyph_index = shaped.glyphs[i];
        if (neko_fontcache_empty(cache, entry, glyph_index)) continue;

        if (neko_fontcache_can_batch_glyph(cache, font, entry, glyph_index)) {
            continue;
        }

        neko_fontcache_draw_text_batch(cache, entry, shaped, batch_start_idx, i, posx, posy, scalex, scaley);
        neko_fontcache_reset_batch_codepoint_state(cache);

        neko_fontcache_cache_glyph_to_atlas(cache, font, glyph_index);
        uint64_t lru_code = glyph_index + ((0x100000000ULL * font) & 0xFFFFFFFF00000000ULL);
        cache->temp_codepoint_seen[lru_code] = true;

        batch_start_idx = i;
    }

    neko_fontcache_draw_text_batch(cache, entry, shaped, batch_start_idx, shaped.glyphs.size(), posx, posy, scalex, scaley);
    neko_fontcache_reset_batch_codepoint_state(cache);
    cache->cursor_pos.x = posx + shaped.end_cursor_pos.x * scalex;
    cache->cursor_pos.y = posy + shaped.end_cursor_pos.x * scaley;

    return true;
}

neko_vec2 __neko_fontcache_get_cursor_pos(neko_fontcache* cache) { return cache->cursor_pos; }

void __neko_fontcache_optimise_drawlist(neko_fontcache* cache) {
    neko_assert(cache);

    int write_idx = 0;
    for (int i = 1; i < cache->drawlist.dcalls.size(); i++) {

        neko_assert(write_idx <= i);
        neko_fontcache_draw& draw0 = cache->drawlist.dcalls[write_idx];
        neko_fontcache_draw& draw1 = cache->drawlist.dcalls[i];

        bool merge = true;
        if (draw0.pass != draw1.pass) merge = false;
        if (draw0.end_index != draw1.start_index) merge = false;
        if (draw0.region != draw1.region) merge = false;
        if (draw1.clear_before_draw) merge = false;
        if (draw0.colour[0] != draw1.colour[0] || draw0.colour[1] != draw1.colour[1] || draw0.colour[2] != draw1.colour[2] || draw0.colour[3] != draw1.colour[3]) merge = false;

        if (merge) {
            draw0.end_index = draw1.end_index;
            draw1.start_index = draw1.end_index = 0;
        } else {
            neko_fontcache_draw& draw2 = cache->drawlist.dcalls[++write_idx];
            if (write_idx != i) draw2 = draw1;
        }
    }
    cache->drawlist.dcalls.resize(write_idx + 1);
}

void __neko_fontcache_set_colour(neko_fontcache* cache, float c[4]) {
    cache->colour[0] = c[0];
    cache->colour[1] = c[1];
    cache->colour[2] = c[2];
    cache->colour[3] = c[3];
}

// ------------------------------------------------------ Generic Data Structure Implementations ------------------------------------------

void __neko_fontcache_poollist_init(neko_fontcache_poollist& plist, int capacity) {
    plist.pool.resize(capacity);
    plist.freelist.resize(capacity);
    plist.capacity = capacity;
    for (int i = 0; i < capacity; i++) plist.freelist[i] = i;
}

void __neko_fontcache_poollist_push_front(neko_fontcache_poollist& plist, neko_fontcache_poollist_value v) {
    if (plist.size >= plist.capacity) return;
    neko_assert(plist.freelist.size() > 0);
    neko_assert(plist.freelist.size() == plist.capacity - plist.size);

    neko_fontcache_poollist_itr idx = plist.freelist.back();
    plist.freelist.pop_back();
    plist.pool[idx].prev = -1;
    plist.pool[idx].next = plist.front;
    plist.pool[idx].value = v;

    if (plist.front != -1) plist.pool[plist.front].prev = idx;
    if (plist.back == -1) plist.back = idx;
    plist.front = idx;
    plist.size++;
}

void __neko_fontcache_poollist_erase(neko_fontcache_poollist& plist, neko_fontcache_poollist_itr it) {
    if (plist.size <= 0) return;
    neko_assert(it >= 0 && it < plist.capacity);
    neko_assert(plist.freelist.size() == plist.capacity - plist.size);

    if (plist.pool[it].prev != -1) plist.pool[plist.pool[it].prev].next = plist.pool[it].next;
    if (plist.pool[it].next != -1) plist.pool[plist.pool[it].next].prev = plist.pool[it].prev;

    if (plist.front == it) plist.front = plist.pool[it].next;
    if (plist.back == it) plist.back = plist.pool[it].prev;

    plist.pool[it].prev = -1;
    plist.pool[it].next = -1;
    plist.pool[it].value = 0;
    plist.freelist.push_back(it);

    if (--plist.size == 0) plist.back = plist.front = -1;
}

neko_fontcache_poollist_value __neko_fontcache_poollist_peek_back(neko_fontcache_poollist& plist) {
    neko_assert(plist.back != -1);
    return plist.pool[plist.back].value;
}

neko_fontcache_poollist_value __neko_fontcache_poollist_pop_back(neko_fontcache_poollist& plist) {
    if (plist.size <= 0) return 0;
    neko_assert(plist.back != -1);
    neko_fontcache_poollist_value v = plist.pool[plist.back].value;
    __neko_fontcache_poollist_erase(plist, plist.back);
    return v;
}

void __neko_fontcache_LRU_init(neko_fontcache_LRU& LRU, int capacity) {
    // Thanks to dfsbfs from leetcode! This code is eavily based on that with simplifications made on top.
    // ref: https://leetcode.com/problems/lru-cache/discuss/968703/c%2B%2B
    //      https://leetcode.com/submissions/detail/436667816/
    LRU.capacity = capacity;
    LRU.cache.reserve(capacity);
    __neko_fontcache_poollist_init(LRU.key_queue, capacity);
}

void __neko_fontcache_LRU_refresh(neko_fontcache_LRU& LRU, uint64_t key) {
    auto it = LRU.cache.find(key);
    __neko_fontcache_poollist_erase(LRU.key_queue, it->second.ptr);
    __neko_fontcache_poollist_push_front(LRU.key_queue, key);
    it->second.ptr = LRU.key_queue.front;
}

int __neko_fontcache_LRU_get(neko_fontcache_LRU& LRU, uint64_t key) {
    auto it = LRU.cache.find(key);
    if (it == LRU.cache.end()) {
        return -1;
    }
    __neko_fontcache_LRU_refresh(LRU, key);
    return it->second.value;
}

int __neko_fontcache_LRU_peek(neko_fontcache_LRU& LRU, uint64_t key) {
    auto it = LRU.cache.find(key);
    if (it == LRU.cache.end()) {
        return -1;
    }
    return it->second.value;
}

uint64_t __neko_fontcache_LRU_put(neko_fontcache_LRU& LRU, uint64_t key, int val) {
    auto it = LRU.cache.find(key);
    if (it != LRU.cache.end()) {
        __neko_fontcache_LRU_refresh(LRU, key);
        it->second.value = val;
        return key;
    }

    uint64_t evict = key;
    if (LRU.key_queue.size >= LRU.capacity) {
        evict = __neko_fontcache_poollist_pop_back(LRU.key_queue);
        LRU.cache.erase(evict);
    }

    __neko_fontcache_poollist_push_front(LRU.key_queue, key);
    LRU.cache[key].value = val;
    LRU.cache[key].ptr = LRU.key_queue.front;
    return evict;
}

uint64_t __neko_fontcache_LRU_get_next_evicted(neko_fontcache_LRU& LRU) {
    if (LRU.key_queue.size >= LRU.capacity) {
        uint64_t evict = __neko_fontcache_poollist_peek_back(LRU.key_queue);
        ;
        return evict;
    }
    return 0xFFFFFFFFFFFFFFFFULL;
}

#endif