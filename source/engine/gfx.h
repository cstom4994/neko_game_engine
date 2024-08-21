#ifndef GFX_H
#define GFX_H

#include "engine/base.h"
#include "engine/glew_glfw.h"
#include "engine/math.h"
#include "engine/prelude.h"

struct shader_pair {
    GLuint id;
    const char* name;
};

// compile link program given paths to shader files possibly NULL doesn't glUseProgram(...)
GLuint gfx_create_program(const char* name, const char* vert_path, const char* geom_path, const char* frag_path);
void gfx_free_program(GLuint program);

// get pointer offset of 'field' in struct 'type'
#define poffsetof(type, field) ((void*)(&((type*)0)->field))

// bind vertex attribute data to 'field' in struct 'type' -- gl_type
// is the GL_* type of the parameter (GL_FLOAT etc.), components is the
// number of components, param_name is a string containing the name of
// the parameter as it appears in the program
#define gfx_bind_vertex_attrib(program, gl_type, components, param_name, type, field)                    \
    do {                                                                                                 \
        GLuint a__ = glGetAttribLocation(program, param_name);                                           \
        glVertexAttribPointer(a__, components, gl_type, GL_FALSE, sizeof(type), poffsetof(type, field)); \
        glEnableVertexAttribArray(a__);                                                                  \
    } while (0)

#define gfx_bind_vertex_attrib_auto(program, gl_type, components, param_name, size, offset) \
    do {                                                                                    \
        GLuint a__ = glGetAttribLocation(program, param_name);                              \
        glVertexAttribPointer(a__, components, gl_type, GL_FALSE, size, offset);            \
        glEnableVertexAttribArray(a__);                                                     \
    } while (0)

/*=============================
// NEKO_GRAPHICS
=============================*/

#define neko_check_gl_error() gfx_print_error(__FILE__, __LINE__)
void gfx_print_error(const char* file, u32 line);

// OpenGL
#define __neko_gl_state_backup()                                                \
    GLenum last_active_texture;                                                 \
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);             \
    glActiveTexture(GL_TEXTURE0);                                               \
    GLuint last_program;                                                        \
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);                   \
    GLuint last_texture;                                                        \
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);                \
    GLuint last_array_buffer;                                                   \
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);         \
    GLint last_viewport[4];                                                     \
    glGetIntegerv(GL_VIEWPORT, last_viewport);                                  \
    GLint last_scissor_box[4];                                                  \
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);                            \
    GLenum last_blend_src_rgb;                                                  \
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);               \
    GLenum last_blend_dst_rgb;                                                  \
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);               \
    GLenum last_blend_src_alpha;                                                \
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);           \
    GLenum last_blend_dst_alpha;                                                \
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);           \
    GLenum last_blend_equation_rgb;                                             \
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);     \
    GLenum last_blend_equation_alpha;                                           \
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha); \
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);                        \
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);                \
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);              \
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);          \
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);          \
    GLboolean last_enable_mutisample = glIsEnabled(GL_MULTISAMPLE);             \
    GLboolean last_enable_framebuffer_srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB)

#define __neko_gl_state_restore()                                                                            \
    glUseProgram(last_program);                                                                              \
    glBindTexture(GL_TEXTURE_2D, last_texture);                                                              \
    glActiveTexture(last_active_texture);                                                                    \
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);                                                        \
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);                             \
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha); \
    if (last_enable_blend)                                                                                   \
        glEnable(GL_BLEND);                                                                                  \
    else                                                                                                     \
        glDisable(GL_BLEND);                                                                                 \
    if (last_enable_cull_face)                                                                               \
        glEnable(GL_CULL_FACE);                                                                              \
    else                                                                                                     \
        glDisable(GL_CULL_FACE);                                                                             \
    if (last_enable_depth_test)                                                                              \
        glEnable(GL_DEPTH_TEST);                                                                             \
    else                                                                                                     \
        glDisable(GL_DEPTH_TEST);                                                                            \
    if (last_enable_stencil_test)                                                                            \
        glEnable(GL_STENCIL_TEST);                                                                           \
    else                                                                                                     \
        glDisable(GL_STENCIL_TEST);                                                                          \
    if (last_enable_scissor_test)                                                                            \
        glEnable(GL_SCISSOR_TEST);                                                                           \
    else                                                                                                     \
        glDisable(GL_SCISSOR_TEST);                                                                          \
    if (last_enable_mutisample)                                                                              \
        glEnable(GL_MULTISAMPLE);                                                                            \
    else                                                                                                     \
        glDisable(GL_MULTISAMPLE);                                                                           \
    if (last_enable_framebuffer_srgb)                                                                        \
        glEnable(GL_FRAMEBUFFER_SRGB);                                                                       \
    else                                                                                                     \
        glDisable(GL_FRAMEBUFFER_SRGB);                                                                      \
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);    \
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3])

// Graphics Pipeline

#define neko_enum_decl(NAME, ...) typedef enum NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME;

neko_enum_decl(gfx_texture_format_type, R_TEXTURE_FORMAT_RGBA8, R_TEXTURE_FORMAT_RGB8, R_TEXTURE_FORMAT_RG8, R_TEXTURE_FORMAT_R32, R_TEXTURE_FORMAT_R32F, R_TEXTURE_FORMAT_RGBA16F,
               R_TEXTURE_FORMAT_RGBA32F, R_TEXTURE_FORMAT_A8, R_TEXTURE_FORMAT_R8, R_TEXTURE_FORMAT_DEPTH8, R_TEXTURE_FORMAT_DEPTH16, R_TEXTURE_FORMAT_DEPTH24, R_TEXTURE_FORMAT_DEPTH32F,
               R_TEXTURE_FORMAT_DEPTH24_STENCIL8, R_TEXTURE_FORMAT_DEPTH32F_STENCIL8, R_TEXTURE_FORMAT_STENCIL8);

neko_enum_decl(gfx_texture_filtering_type, R_TEXTURE_FILTER_NEAREST, R_TEXTURE_FILTER_LINEAR);

neko_handle_decl(gfx_texture_t);

typedef struct gfx_shader_source_desc_t {
    u32 type;            // Shader stage type (vertex, fragment, tesselation, geometry, compute)
    const char* source;  // Source for shader
} gfx_shader_source_desc_t;

typedef struct gfx_shader_desc_t {
    gfx_shader_source_desc_t* sources;  // Array of shader source descriptions
    size_t size;                        // Size in bytes of shader source desc array
    char name[64];                      // Optional (for logging and debugging mainly)
} gfx_shader_desc_t;

// Graphics AssetTexture Desc
typedef struct gfx_texture_desc_t {
    u32 width;                              // Width of texture in texels
    u32 height;                             // Height of texture in texels
    u32 depth;                              // Depth of texture
    void* data;                             // AssetTexture data to upload (can be null)
    gfx_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    u32 wrap_s;                             // Wrapping type for s axis of texture
    u32 wrap_t;                             // Wrapping type for t axis of texture
    u32 wrap_r;                             // Wrapping type for r axis of texture
    gfx_texture_filtering_type min_filter;  // Minification filter for texture
    gfx_texture_filtering_type mag_filter;  // Magnification filter for texture
    gfx_texture_filtering_type mip_filter;  // Mip filter for texture
    vec2 offset;                            // Offset for updates
    u32 num_mips;                           // Number of mips to generate (default 0 is disable mip generation)
    struct {
        u32 x;        // X offset in texels to start read
        u32 y;        // Y offset in texels to start read
        u32 width;    // Width in texels for texture
        u32 height;   // Height in texels for texture
        size_t size;  // Size in bytes for data to be read
    } read;
    u16 flip_y;  // Whether or not y is flipped
} gfx_texture_desc_t;

typedef struct gfx_info_t {
    const_str version;
    const_str vendor;
    u32 major_version;
    u32 minor_version;
    u32 max_texture_units;
    u32 max_texture_size;
    struct {
        bool available;
        u32 max_work_group_count[3];
        u32 max_work_group_size[3];
        u32 max_work_group_invocations;
    } compute;
} gfx_info_t;

/*==========================
// Graphics Interface
==========================*/

struct neko_gl_data_t;

typedef struct gfx_t {
    neko_gl_data_t* ud;
    gfx_info_t info;  // Used for querying by user for features
} gfx_t;

extern gfx_t* g_render;

// Shader
typedef struct neko_gl_shader_t {
    u32 id;
} neko_gl_shader_t;

// AssetTexture
typedef struct neko_gl_texture_t {
    u32 id;
    gfx_texture_desc_t desc;
} neko_gl_texture_t;

// 内部 OpenGL 数据
typedef struct neko_gl_data_t {
    neko_slot_array(neko_gl_texture_t) textures;
} neko_gl_data_t;

#define neko_render() RENDER()

gfx_t* gfx_create();
void gfx_fini(gfx_t* render);
void gfx_init(gfx_t* render);

gfx_info_t* gfx_info();
neko_gl_data_t* gfx_ogl();

neko_handle(gfx_texture_t) gfx_texture_create(const gfx_texture_desc_t desc);
void gfx_texture_fini(neko_handle(gfx_texture_t) hndl);
void gfx_texture_read(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc);
void gfx_texture_request_update(command_buffer_t* cb, neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t desc);

typedef neko_handle(gfx_texture_t) gfx_texture_t;

typedef struct neko_rgb_color_t {
    float r, g, b;
} neko_rgb_color_t;

typedef u32 neko_color_t;

struct neko_resource_t;

neko_color_t neko_color_from_rgb_color(neko_rgb_color_t rgb);
neko_rgb_color_t neko_rgb_color_from_color(neko_color_t color);

void neko_bind_shader(u32 shader);
void neko_shader_set_int(u32 shader, const char* name, i32 v);
void neko_shader_set_uint(u32 shader, const char* name, u32 v);
void neko_shader_set_float(u32 shader, const char* name, float v);
void neko_shader_set_color(u32 shader, const char* name, neko_color_t color);
void neko_shader_set_rgb_color(u32 shader, const char* name, neko_rgb_color_t color);
void neko_shader_set_v2i(u32 shader, const char* name, neko_v2i_t v);
void neko_shader_set_v2u(u32 shader, const char* name, neko_v2u_t v);
void neko_shader_set_v2f(u32 shader, const char* name, neko_v2f_t v);
void neko_shader_set_v3i(u32 shader, const char* name, neko_v3i_t v);
void neko_shader_set_v3u(u32 shader, const char* name, neko_v3u_t v);
void neko_shader_set_v3f(u32 shader, const char* name, neko_v3f_t v);
void neko_shader_set_v4i(u32 shader, const char* name, neko_v4i v);
void neko_shader_set_v4u(u32 shader, const char* name, neko_v4u v);
void neko_shader_set_v4f(u32 shader, const char* name, neko_v4f_t v);
void neko_shader_set_m4f(u32 shader, const char* name, neko_m4f_t v);

typedef enum neko_vertex_buffer_flags_t {
    NEKO_VERTEXBUFFER_STATIC_DRAW = 1 << 0,
    NEKO_VERTEXBUFFER_DYNAMIC_DRAW = 1 << 1,
    NEKO_VERTEXBUFFER_DRAW_LINES = 1 << 2,
    NEKO_VERTEXBUFFER_DRAW_LINE_STRIP = 1 << 3,
    NEKO_VERTEXBUFFER_DRAW_TRIANGLES = 1 << 4,
} neko_vertex_buffer_flags_t;

typedef struct neko_vertex_buffer_t {
    u32 va_id;
    u32 vb_id;
    u32 ib_id;
    u32 index_count;
    neko_vertex_buffer_flags_t flags;
} neko_vertex_buffer_t;

neko_vertex_buffer_t* neko_new_vertex_buffer(neko_vertex_buffer_flags_t flags);
void neko_free_vertex_buffer(neko_vertex_buffer_t* buffer);
void neko_bind_vertex_buffer_for_draw(neko_vertex_buffer_t* buffer);
void neko_bind_vertex_buffer_for_edit(neko_vertex_buffer_t* buffer);
void neko_push_vertices(neko_vertex_buffer_t* buffer, float* vertices, u32 count);
void neko_push_indices(neko_vertex_buffer_t* buffer, u32* indices, u32 count);
void neko_update_vertices(neko_vertex_buffer_t* buffer, float* vertices, u32 offset, u32 count);
void neko_update_indices(neko_vertex_buffer_t* buffer, u32* indices, u32 offset, u32 count);
void neko_configure_vertex_buffer(neko_vertex_buffer_t* buffer, u32 index, u32 component_count, u32 stride, u32 offset);
void neko_draw_vertex_buffer(neko_vertex_buffer_t* buffer);
void neko_draw_vertex_buffer_custom_count(neko_vertex_buffer_t* buffer, u32 count);

typedef enum neko_texture_flags_t {
    NEKO_TEXTURE_ALIASED = 1 << 0,
    NEKO_TEXTURE_ANTIALIASED = 1 << 1,
} neko_texture_flags_t;

typedef struct neko_texture_t {
    u32 id;
    i32 width, height, component_count;

    neko_texture_flags_t flags;
} neko_texture_t;

neko_texture_t* neko_new_texture(neko_resource_t* resource, neko_texture_flags_t flags);
neko_texture_t* neko_new_texture_from_memory(void* data, u32 size, neko_texture_flags_t flags);
neko_texture_t* neko_new_texture_from_memory_uncompressed(unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags);
void neko_init_texture(neko_texture_t* texture, neko_resource_t* resource, neko_texture_flags_t flags);
void neko_init_texture_from_memory(neko_texture_t* texture, void* data, u32 size, neko_texture_flags_t flags);
void neko_init_texture_from_memory_uncompressed(neko_texture_t* texture, unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags);
void neko_deinit_texture(neko_texture_t* texture);

void neko_free_texture(neko_texture_t* texture);
void neko_bind_texture(neko_texture_t* texture, u32 slot);

typedef struct neko_rect_t {
    float x, y, w, h;
} neko_rect_t;

#endif