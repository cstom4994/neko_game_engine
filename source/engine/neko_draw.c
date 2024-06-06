
#include "engine/neko_common.h"
#include "engine/neko_render.h"
#include "engine/neko_platform.h"

// STB
#include "deps/imgui/imstb_rectpack.h"
#include "deps/imgui/imstb_truetype.h"

/*=============================
// Camera
=============================*/

NEKO_API_DECL neko_camera_t neko_camera_default() {
    // Construct default camera parameters
    neko_camera_t cam = NEKO_DEFAULT_VAL();
    cam.transform = neko_vqs_default();
    cam.transform.position.z = 1.f;
    cam.fov = 60.f;
    cam.near_plane = 0.1f;
    cam.far_plane = 1000.f;
    cam.ortho_scale = 1.f;
    cam.proj_type = NEKO_PROJECTION_TYPE_ORTHOGRAPHIC;
    return cam;
}

NEKO_API_DECL neko_camera_t neko_camera_perspective() {
    neko_camera_t cam = neko_camera_default();
    cam.proj_type = NEKO_PROJECTION_TYPE_PERSPECTIVE;
    cam.transform.position.z = 1.f;
    return cam;
}

NEKO_API_DECL neko_vec3 neko_camera_forward(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, 0.0f, -1.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_backward(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, 0.0f, 1.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_up(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, 1.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_down(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(0.0f, -1.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_right(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(1.0f, 0.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_left(const neko_camera_t* cam) { return (neko_quat_rotate(cam->transform.rotation, neko_v3(-1.0f, 0.0f, 0.0f))); }

NEKO_API_DECL neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height) {
    // Transform world coords to screen coords to place billboarded UI elements in world
    neko_mat4 vp = neko_camera_get_view_projection(cam, view_width, view_height);
    neko_vec4 p4 = neko_v4(coords.x, coords.y, coords.z, 1.f);
    p4 = neko_mat4_mul_vec4(vp, p4);
    p4.x /= p4.w;
    p4.y /= p4.w;
    p4.z /= p4.w;

    // Bring into ndc
    p4.x = p4.x * 0.5f + 0.5f;
    p4.y = p4.y * 0.5f + 0.5f;

    // Bring into screen space
    p4.x = p4.x * (f32)view_width;
    p4.y = neko_map_range(1.f, 0.f, 0.f, 1.f, p4.y) * (f32)view_height;

    return neko_v3(p4.x, p4.y, p4.z);
}

NEKO_API_DECL neko_vec3 neko_camera_screen_to_world(const neko_camera_t* cam, neko_vec3 coords, s32 view_x, s32 view_y, s32 view_width, s32 view_height) {
    neko_vec3 wc = NEKO_DEFAULT_VAL();

    // Get inverse of view projection from camera
    neko_mat4 inverse_vp = neko_mat4_inverse(neko_camera_get_view_projection(cam, view_width, view_height));
    f32 w_x = (f32)coords.x - (f32)view_x;
    f32 w_y = (f32)coords.y - (f32)view_y;
    f32 w_z = (f32)coords.z;

    // Transform from ndc
    neko_vec4 in;
    in.x = (w_x / (f32)view_width) * 2.f - 1.f;
    in.y = 1.f - (w_y / (f32)view_height) * 2.f;
    in.z = 2.f * w_z - 1.f;
    in.w = 1.f;

    // To world coords
    neko_vec4 out = neko_mat4_mul_vec4(inverse_vp, in);
    if (out.w == 0.f) {
        // Avoid div by zero
        return wc;
    }

    out.w = fabsf(out.w) > neko_epsilon ? 1.f / out.w : 0.0001f;
    wc = neko_v3(out.x * out.w, out.y * out.w, out.z * out.w);

    return wc;
}

NEKO_API_DECL neko_mat4 neko_camera_get_view_projection(const neko_camera_t* cam, s32 view_width, s32 view_height) {
    neko_mat4 view = neko_camera_get_view(cam);
    neko_mat4 proj = neko_camera_get_proj(cam, view_width, view_height);
    return neko_mat4_mul(proj, view);
}

NEKO_API_DECL neko_mat4 neko_camera_get_view(const neko_camera_t* cam) {
    neko_vec3 up = neko_camera_up(cam);
    neko_vec3 forward = neko_camera_forward(cam);
    neko_vec3 target = neko_vec3_add(forward, cam->transform.position);
    return neko_mat4_look_at(cam->transform.position, target, up);
}

NEKO_API_DECL neko_mat4 neko_camera_get_proj(const neko_camera_t* cam, s32 view_width, s32 view_height) {
    neko_mat4 proj_mat = neko_mat4_identity();

    switch (cam->proj_type) {
        case NEKO_PROJECTION_TYPE_PERSPECTIVE: {
            proj_mat = neko_mat4_perspective(cam->fov, (f32)view_width / (f32)view_height, cam->near_plane, cam->far_plane);
        } break;
        case NEKO_PROJECTION_TYPE_ORTHOGRAPHIC: {
            f32 _ar = (f32)view_width / (f32)view_height;
            f32 distance = 0.5f * (cam->far_plane - cam->near_plane);
            const f32 ortho_scale = cam->ortho_scale;
            const f32 aspect_ratio = _ar;
            proj_mat = neko_mat4_ortho(-ortho_scale * aspect_ratio, ortho_scale * aspect_ratio, -ortho_scale, ortho_scale, -distance, distance);
        } break;
    }

    return proj_mat;
}

NEKO_API_DECL void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 pitch) {
    neko_quat x = neko_quat_angle_axis(neko_deg2rad(yaw), neko_v3(0.f, 1.f, 0.f));    // Absolute up
    neko_quat y = neko_quat_angle_axis(neko_deg2rad(pitch), neko_camera_right(cam));  // Relative right
    cam->transform.rotation = neko_quat_mul(neko_quat_mul(x, y), cam->transform.rotation);
}

/*=============================
// IDraw
=============================*/

// 立即绘制模式 静态数据的全局实例
neko_immediate_draw_static_data_t* g_neko_idraw = NULL;

#define neko_idraw() g_neko_idraw

#ifndef neko_idraw_smooth_circle_error_rate
#define neko_idraw_smooth_circle_error_rate 0.5f
#endif

const f32 neko_idraw_deg2rad = (f32)neko_pi / 180.f;

// Shaders
#if (defined NEKO_PLATFORM_WEB || defined NEKO_PLATFORM_ANDROID)
#define NEKO_IDRAW_GL_VERSION_STR "#version 300 es\n"
#else
#define NEKO_IDRAW_GL_VERSION_STR "#version 330 core\n"
#endif

#ifndef NEKO_IDRAW_UNIFORM_MVP_MATRIX
#define NEKO_IDRAW_UNIFORM_MVP_MATRIX "u_mvp"
#endif

#ifndef NEKO_IDRAW_UNIFORM_TEXTURE2D
#define NEKO_IDRAW_UNIFORM_TEXTURE2D "u_tex"
#endif

const char* neko_idraw_v_fillsrc = NEKO_IDRAW_GL_VERSION_STR
        "precision mediump float;\n"
        "layout(location = 0) in vec3 a_position;\n"
        "layout(location = 1) in vec2 a_uv;\n"
        "layout(location = 2) in vec4 a_color;\n"
        "uniform mat4 " NEKO_IDRAW_UNIFORM_MVP_MATRIX
        ";\n"
        "out vec2 uv;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  gl_Position = " NEKO_IDRAW_UNIFORM_MVP_MATRIX
        "* vec4(a_position, 1.0);\n"
        "  uv = a_uv;\n"
        "  color = a_color;\n"
        "}\n";

const char* neko_idraw_f_fillsrc = NEKO_IDRAW_GL_VERSION_STR
        "precision mediump float;\n"
        "in vec2 uv;\n"
        "in vec4 color;\n"
        "uniform sampler2D " NEKO_IDRAW_UNIFORM_TEXTURE2D
        ";\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  vec4 tex_col = texture(" NEKO_IDRAW_UNIFORM_TEXTURE2D
        ", uv);\n"
        "  if (tex_col.a < 0.5) discard;\n"
        "  frag_color = color * tex_col;\n"
        "}\n";

neko_idraw_pipeline_state_attr_t neko_idraw_pipeline_state_default() {
    neko_idraw_pipeline_state_attr_t attr = NEKO_DEFAULT_VAL();
    attr.depth_enabled = false;
    attr.stencil_enabled = false;
    attr.blend_enabled = true;
    attr.face_cull_enabled = false;
    attr.prim_type = (u16)R_PRIMITIVE_TRIANGLES;
    return attr;
}

void neko_idraw_reset(neko_immediate_draw_t* neko_idraw) {
    neko_command_buffer_clear(&neko_idraw->commands);
    neko_byte_buffer_clear(&neko_idraw->vertices);
    neko_dyn_array_clear(neko_idraw->indices);
    neko_dyn_array_clear(neko_idraw->cache.modelview);
    neko_dyn_array_clear(neko_idraw->cache.projection);
    neko_dyn_array_clear(neko_idraw->cache.pipelines);
    neko_dyn_array_clear(neko_idraw->cache.modes);
    neko_dyn_array_clear(neko_idraw->vattributes);

    neko_dyn_array_push(neko_idraw->cache.modelview, neko_mat4_identity());
    neko_dyn_array_push(neko_idraw->cache.projection, neko_mat4_identity());
    neko_dyn_array_push(neko_idraw->cache.modes, NEKO_IDRAW_MATRIX_MODELVIEW);

    neko_idraw->cache.texture = neko_idraw()->tex_default;
    neko_idraw->cache.pipeline = neko_idraw_pipeline_state_default();
    neko_idraw->cache.pipeline.prim_type = 0x00;
    neko_idraw->cache.uv = neko_v2(0.f, 0.f);
    neko_idraw->cache.color = NEKO_COLOR_WHITE;
    neko_idraw->flags = 0x00;
}

void neko_immediate_draw_static_data_init() {
    neko_idraw() = neko_malloc_init(neko_immediate_draw_static_data_t);

    // Create uniform buffer
    neko_render_uniform_layout_desc_t uldesc = NEKO_DEFAULT_VAL();
    uldesc.type = R_UNIFORM_MAT4;
    neko_render_uniform_desc_t udesc = NEKO_DEFAULT_VAL();
    memcpy(udesc.name, NEKO_IDRAW_UNIFORM_MVP_MATRIX, sizeof(NEKO_IDRAW_UNIFORM_MVP_MATRIX));
    udesc.layout = &uldesc;
    neko_idraw()->uniform = neko_render_uniform_create(udesc);

    // Create sampler buffer
    neko_render_uniform_layout_desc_t sldesc = NEKO_DEFAULT_VAL();
    sldesc.type = R_UNIFORM_SAMPLER2D;
    neko_render_uniform_desc_t sbdesc = NEKO_DEFAULT_VAL();
    memcpy(sbdesc.name, NEKO_IDRAW_UNIFORM_TEXTURE2D, sizeof(NEKO_IDRAW_UNIFORM_TEXTURE2D));
    sbdesc.layout = &sldesc;
    neko_idraw()->sampler = neko_render_uniform_create(sbdesc);

    // Create default texture (4x4 white)
    neko_color_t pixels[16] = NEKO_DEFAULT_VAL();
    memset(pixels, 255, 16 * sizeof(neko_color_t));

    neko_render_texture_desc_t tdesc = NEKO_DEFAULT_VAL();
    tdesc.width = 4;
    tdesc.height = 4;
    tdesc.format = R_TEXTURE_FORMAT_RGBA8;
    tdesc.min_filter = R_TEXTURE_FILTER_NEAREST;
    tdesc.mag_filter = R_TEXTURE_FILTER_NEAREST;
    *tdesc.data = pixels;

    neko_idraw()->tex_default = neko_render_texture_create(tdesc);

    // Create shader
    neko_render_shader_source_desc_t vsrc;
    vsrc.type = R_SHADER_STAGE_VERTEX;
    vsrc.source = neko_idraw_v_fillsrc;
    neko_render_shader_source_desc_t fsrc;
    fsrc.type = R_SHADER_STAGE_FRAGMENT;
    fsrc.source = neko_idraw_f_fillsrc;
    neko_render_shader_source_desc_t neko_idraw_sources[] = {vsrc, fsrc};

    neko_render_shader_desc_t sdesc = NEKO_DEFAULT_VAL();
    sdesc.sources = neko_idraw_sources;
    sdesc.size = sizeof(neko_idraw_sources);
    memcpy(sdesc.name, "neko_immediate_default_fill_shader", sizeof("neko_immediate_default_fill_shader"));

    // Vertex attr layout
    neko_render_vertex_attribute_desc_t neko_idraw_vattrs[3] = NEKO_DEFAULT_VAL();
    neko_idraw_vattrs[0].format = R_VERTEX_ATTRIBUTE_FLOAT3;
    memcpy(neko_idraw_vattrs[0].name, "a_position", sizeof("a_position"));
    neko_idraw_vattrs[1].format = R_VERTEX_ATTRIBUTE_FLOAT2;
    memcpy(neko_idraw_vattrs[1].name, "a_uv", sizeof("a_uv"));
    neko_idraw_vattrs[2].format = R_VERTEX_ATTRIBUTE_BYTE4;
    memcpy(neko_idraw_vattrs[2].name, "a_color", sizeof("a_color"));

    // Iterate through attribute list, then create custom pipelines requested.
    neko_handle(neko_render_shader_t) shader = neko_render_shader_create(sdesc);

    // Pipelines
    for (u16 d = 0; d < 2; ++d)                  // Depth
        for (u16 s = 0; s < 2; ++s)              // Stencil
            for (u16 b = 0; b < 2; ++b)          // Blend
                for (u16 f = 0; f < 2; ++f)      // Face Cull
                    for (u16 p = 0; p < 2; ++p)  // Prim Type
                    {
                        neko_idraw_pipeline_state_attr_t attr = NEKO_DEFAULT_VAL();

                        attr.depth_enabled = d;
                        attr.stencil_enabled = s;
                        attr.blend_enabled = b;
                        attr.face_cull_enabled = f;
                        attr.prim_type = p ? (u16)R_PRIMITIVE_TRIANGLES : (u16)R_PRIMITIVE_LINES;

                        // Create new pipeline based on this arrangement
                        neko_render_pipeline_desc_t pdesc = NEKO_DEFAULT_VAL();
                        pdesc.raster.shader = shader;
                        pdesc.raster.index_buffer_element_size = sizeof(u16);
                        pdesc.raster.face_culling = attr.face_cull_enabled ? R_FACE_CULLING_BACK : (neko_render_face_culling_type)0x00;
                        pdesc.raster.primitive = (neko_render_primitive_type)attr.prim_type;
                        pdesc.blend.func = attr.blend_enabled ? R_BLEND_EQUATION_ADD : (neko_render_blend_equation_type)0x00;
                        pdesc.blend.src = R_BLEND_MODE_SRC_ALPHA;
                        pdesc.blend.dst = R_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
                        pdesc.depth.func = d ? R_DEPTH_FUNC_LESS : (neko_render_depth_func_type)0x00;
                        pdesc.stencil.func = s ? R_STENCIL_FUNC_ALWAYS : (neko_render_stencil_func_type)0x00;
                        pdesc.stencil.ref = s ? 1 : 0x00;
                        pdesc.stencil.comp_mask = s ? 0xFF : 0x00;
                        pdesc.stencil.write_mask = s ? 0xFF : 0x00;
                        pdesc.stencil.sfail = s ? R_STENCIL_OP_KEEP : (neko_render_stencil_op_type)0x00;
                        pdesc.stencil.dpfail = s ? R_STENCIL_OP_KEEP : (neko_render_stencil_op_type)0x00;
                        pdesc.stencil.dppass = s ? R_STENCIL_OP_REPLACE : (neko_render_stencil_op_type)0x00;
                        pdesc.layout.attrs = neko_idraw_vattrs;
                        pdesc.layout.size = sizeof(neko_idraw_vattrs);

                        neko_handle(neko_render_pipeline_t) hndl = neko_render_pipeline_create(pdesc);
                        neko_hash_table_insert(neko_idraw()->pipeline_table, attr, hndl);
                    }

    // Create default font
    neko_asset_ascii_font_t* f = &neko_idraw()->font_default;
    stbtt_fontinfo font = NEKO_DEFAULT_VAL();
    const char* compressed_ttf_data_base85 = __neko_internal_GetDefaultCompressedFontDataTTFBase85();
    s32 compressed_ttf_size = (((s32)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf_data = neko_malloc((usize)compressed_ttf_size);
    __neko_internal_Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf_data);
    const u32 buf_decompressed_size = neko_decompress_length((unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char*)neko_malloc(buf_decompressed_size);
    neko_decompress(buf_decompressed_data, (unsigned char*)compressed_ttf_data, (u32)compressed_ttf_size);

    const u32 w = 512;
    const u32 h = 512;
    const u32 num_comps = 4;
    u8* alpha_bitmap = (u8*)neko_malloc(w * h);
    u8* flipmap = (u8*)neko_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);
    s32 v = stbtt_BakeFontBitmap((u8*)buf_decompressed_data, 0, 13.f, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f->glyphs);  // no guarantee this fits!

    // Flip texture
    u32 r = h - 1;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            u32 i0 = i * w + j;
            u32 i1 = i * w * num_comps + j * num_comps;
            u8 a = alpha_bitmap[i0];
            flipmap[i1 + 0] = 255;
            flipmap[i1 + 1] = 255;
            flipmap[i1 + 2] = 255;
            flipmap[i1 + 3] = a;
        }
        r--;
    }

    neko_render_texture_desc_t desc = NEKO_DEFAULT_VAL();
    desc.width = w;
    desc.height = h;
    *desc.data = flipmap;
    desc.format = R_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = R_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = R_TEXTURE_FILTER_NEAREST;

    // Generate atlas texture for bitmap with bitmap data
    f->texture.hndl = neko_render_texture_create(desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    // Create vertex buffer
    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
    vdesc.data = NULL;
    vdesc.size = 0;
    vdesc.usage = R_BUFFER_USAGE_STREAM;
    neko_idraw()->vbo = neko_render_vertex_buffer_create(vdesc);

    neko_free(compressed_ttf_data);
    neko_free(buf_decompressed_data);
    neko_free(alpha_bitmap);
    neko_free(flipmap);
}

NEKO_API_DECL void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data) { g_neko_idraw = data; }

NEKO_API_DECL neko_immediate_draw_static_data_t* neko_immediate_draw_static_data_get() { return g_neko_idraw; }

// Create / Init / Shutdown / Free
neko_immediate_draw_t neko_immediate_draw_new() {
    if (!neko_idraw()) {
        // Construct NEKO_IDRAW
        neko_immediate_draw_static_data_init();
    }

    neko_immediate_draw_t neko_idraw = NEKO_DEFAULT_VAL();
    memset(&neko_idraw, 0, sizeof(neko_idraw));

    // Set neko_idraw static data
    neko_idraw.data = g_neko_idraw;

    // Init cache
    neko_idraw.cache.color = NEKO_COLOR_WHITE;

    // Init command buffer
    neko_idraw.commands = neko_command_buffer_new();  // Not totally sure on the syntax for new vs. create

    neko_idraw.vertices = neko_byte_buffer_new();

    // Set up cache
    neko_idraw_reset(&neko_idraw);

    return neko_idraw;
}

NEKO_API_DECL void neko_immediate_draw_free(neko_immediate_draw_t* ctx) {
    neko_byte_buffer_free(&ctx->vertices);
    neko_dyn_array_free(ctx->indices);
    neko_dyn_array_free(ctx->vattributes);
    neko_command_buffer_free(&ctx->commands);
    neko_dyn_array_free(ctx->cache.pipelines);
    neko_dyn_array_free(ctx->cache.modelview);
    neko_dyn_array_free(ctx->cache.projection);
    neko_dyn_array_free(ctx->cache.modes);
}

NEKO_API_DECL neko_asset_ascii_font_t* neko_idraw_default_font() {
    if (neko_idraw()) return &neko_idraw()->font_default;
    return NULL;
}

NEKO_API_DECL neko_handle(neko_render_pipeline_t) neko_idraw_get_pipeline(neko_immediate_draw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state) {
    // Bind pipeline
    NEKO_ASSERT(neko_hash_table_key_exists(neko_idraw()->pipeline_table, state));
    return neko_hash_table_get(neko_idraw()->pipeline_table, state);
}

void neko_immediate_draw_set_pipeline(neko_immediate_draw_t* neko_idraw) {
    if (neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES) {
        return;
    }

    // Check validity
    if (neko_idraw->cache.pipeline.prim_type != (u16)R_PRIMITIVE_TRIANGLES && neko_idraw->cache.pipeline.prim_type != (u16)R_PRIMITIVE_LINES) {
        neko_idraw->cache.pipeline.prim_type = (u16)R_PRIMITIVE_TRIANGLES;
    }
    neko_idraw->cache.pipeline.depth_enabled = NEKO_CLAMP(neko_idraw->cache.pipeline.depth_enabled, 0, 1);
    neko_idraw->cache.pipeline.stencil_enabled = NEKO_CLAMP(neko_idraw->cache.pipeline.stencil_enabled, 0, 1);
    neko_idraw->cache.pipeline.face_cull_enabled = NEKO_CLAMP(neko_idraw->cache.pipeline.face_cull_enabled, 0, 1);
    neko_idraw->cache.pipeline.blend_enabled = NEKO_CLAMP(neko_idraw->cache.pipeline.blend_enabled, 0, 1);

    // Bind pipeline
    NEKO_ASSERT(neko_hash_table_key_exists(neko_idraw()->pipeline_table, neko_idraw->cache.pipeline));
    neko_render_pipeline_bind(&neko_idraw->commands, neko_hash_table_get(neko_idraw()->pipeline_table, neko_idraw->cache.pipeline));
}

// From on: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
neko_hsv_t neko_rgb_to_hsv(neko_color_t c) {
    neko_vec3 cv = (neko_vec3){(f32)c.r / 255.f, (f32)c.g / 255.f, (f32)c.b / 255.f};
    f32 fR = cv.x, fG = cv.y, fB = cv.z;

    f32 fCMax = NEKO_MAX(NEKO_MAX(fR, fG), fB);
    f32 fCMin = NEKO_MIN(NEKO_MIN(fR, fG), fB);
    f32 fDelta = fCMax - fCMin;

    neko_hsv_t hsv;

    if (fDelta > 0) {
        if (fCMax == fR) {
            hsv.h = 60 * (fmod(((fG - fB) / fDelta), 6));
        } else if (fCMax == fG) {
            hsv.h = 60 * (((fB - fR) / fDelta) + 2);
        } else if (fCMax == fB) {
            hsv.h = 60 * (((fR - fG) / fDelta) + 4);
        }

        if (fCMax > 0) {
            hsv.s = fDelta / fCMax;
        } else {
            hsv.s = 0;
        }

        hsv.v = fCMax;
    } else {
        hsv.h = 0;
        hsv.s = 0;
        hsv.v = fCMax;
    }

    if (hsv.h < 0) {
        hsv.h = 360 + hsv.h;
    }

    return hsv;
}

// Implemented from: https://stackoverflow.com/questions/27374550/how-to-compare-color-object-and-get-closest-color-in-an-color
// distance between two hues:
f32 neko_hue_dist(f32 h1, f32 h2) {
    f32 d = fabsf(h1 - h2);
    return d > 180.f ? 360.f - d : d;
}

void __neko_draw_rect_2d_impl(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color) {
    neko_vec3 tl = neko_v3(a.x, a.y, 0.f);
    neko_vec3 tr = neko_v3(b.x, a.y, 0.f);
    neko_vec3 bl = neko_v3(a.x, b.y, 0.f);
    neko_vec3 br = neko_v3(b.x, b.y, 0.f);

    neko_vec2 tl_uv = neko_v2(uv0.x, uv1.y);
    neko_vec2 tr_uv = neko_v2(uv1.x, uv1.y);
    neko_vec2 bl_uv = neko_v2(uv0.x, uv0.y);
    neko_vec2 br_uv = neko_v2(uv1.x, uv0.y);

    neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);
    {
        neko_idraw_c4ubv(neko_idraw, color);

        neko_idraw_tc2fv(neko_idraw, tl_uv);
        neko_idraw_v3fv(neko_idraw, tl);

        neko_idraw_tc2fv(neko_idraw, br_uv);
        neko_idraw_v3fv(neko_idraw, br);

        neko_idraw_tc2fv(neko_idraw, bl_uv);
        neko_idraw_v3fv(neko_idraw, bl);

        neko_idraw_tc2fv(neko_idraw, tl_uv);
        neko_idraw_v3fv(neko_idraw, tl);

        neko_idraw_tc2fv(neko_idraw, tr_uv);
        neko_idraw_v3fv(neko_idraw, tr);

        neko_idraw_tc2fv(neko_idraw, br_uv);
        neko_idraw_v3fv(neko_idraw, br);
    }
    neko_idraw_end(neko_idraw);
}

void neko_idraw_rect_textured(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, u32 tex_id, neko_color_t color) {
    neko_idraw_rect_textured_ext(neko_idraw, a.x, a.y, b.x, b.y, 0.f, 0.f, 1.f, 1.f, tex_id, color);
}

void neko_idraw_rect_textured_ext(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color) {

    neko_handle(neko_render_texture_t) tex = NEKO_DEFAULT_VAL();
    tex.id = tex_id;

    neko_idraw_texture(neko_idraw, tex);
    __neko_draw_rect_2d_impl(neko_idraw, neko_v2(x0, y0), neko_v2(x1, y1), neko_v2(u0, v0), neko_v2(u1, v1), color);
    neko_idraw_texture(neko_idraw, (neko_texture_t){0});
}

// 核心顶点函数
void neko_idraw_begin(neko_immediate_draw_t* neko_idraw, neko_render_primitive_type type) {
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES:
            type = R_PRIMITIVE_TRIANGLES;
            break;
        case R_PRIMITIVE_LINES:
            type = R_PRIMITIVE_LINES;
            break;
    }

    // 判断是否推入新的渲染管线
    if (neko_idraw->cache.pipeline.prim_type == type) {
        return;
    }

    // 否则 刷新先前的渲染管线
    neko_idraw_flush(neko_idraw);

    // 设置原始类型
    neko_idraw->cache.pipeline.prim_type = type;

    // 绑定渲染管线
    neko_immediate_draw_set_pipeline(neko_idraw);
}

void neko_idraw_end(neko_immediate_draw_t* neko_idraw) {
    // TODO
}

neko_mat4 neko_idraw_get_modelview_matrix(neko_immediate_draw_t* neko_idraw) { return neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1]; }

neko_mat4 neko_idraw_get_projection_matrix(neko_immediate_draw_t* neko_idraw) { return neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1]; }

neko_mat4 neko_idraw_get_mvp_matrix(neko_immediate_draw_t* neko_idraw) {
    neko_mat4* mv = &neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1];
    neko_mat4* proj = &neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1];
    return neko_mat4_mul(*proj, *mv);
}

void neko_idraw_flush(neko_immediate_draw_t* neko_idraw) {
    // 如果顶点数据为空则不刷新
    if (neko_byte_buffer_empty(&neko_idraw->vertices)) {
        return;
    }

    // 设置mvp矩阵
    neko_mat4 mv = neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1];
    neko_mat4 proj = neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1];
    neko_mat4 mvp = neko_mat4_mul(proj, mv);

    // 更新顶点缓冲区 (使用命令缓冲区)
    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
    vdesc.data = neko_idraw->vertices.data;
    vdesc.size = neko_byte_buffer_size(&neko_idraw->vertices);
    vdesc.usage = R_BUFFER_USAGE_STREAM;

    neko_render_vertex_buffer_request_update(&neko_idraw->commands, neko_idraw()->vbo, &vdesc);

    // 计算绘制数量
    size_t vsz = sizeof(neko_immediate_vert_t);
    if (neko_dyn_array_size(neko_idraw->vattributes)) {
        // 计算顶点步幅
        size_t stride = 0;
        for (u32 i = 0; i < neko_dyn_array_size(neko_idraw->vattributes); ++i) {
            neko_idraw_vattr_type type = neko_idraw->vattributes[i];
            switch (type) {
                default:
                    break;
                case NEKO_IDRAW_VATTR_POSITION:
                    stride += sizeof(neko_vec3);
                    break;
                case NEKO_IDRAW_VATTR_COLOR:
                    stride += sizeof(neko_color_t);
                    break;
                case NEKO_IDRAW_VATTR_UV:
                    stride += sizeof(neko_vec2);
                    break;
            }
        }
        vsz = stride;
    }

    u32 ct = neko_byte_buffer_size(&neko_idraw->vertices) / vsz;

    // 设置所有绑定数据
    neko_render_bind_vertex_buffer_desc_t vbuffer = NEKO_DEFAULT_VAL();
    vbuffer.buffer = neko_idraw()->vbo;

    neko_render_bind_uniform_desc_t ubinds[2] = NEKO_DEFAULT_VAL();
    ubinds[0].uniform = neko_idraw()->uniform;
    ubinds[0].data = &mvp;
    ubinds[1].uniform = neko_idraw()->sampler;
    ubinds[1].data = &neko_idraw->cache.texture;
    ubinds[1].binding = 0;

    // 所有缓冲区的绑定 vertex,uniform,sampler
    neko_render_bind_desc_t binds = NEKO_DEFAULT_VAL();
    binds.vertex_buffers.desc = &vbuffer;

    // if (neko_dyn_array_empty(neko_idraw->vattributes))
    if (~neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_UNIFORMS) {
        binds.uniforms.desc = ubinds;
        binds.uniforms.size = sizeof(ubinds);
    }

    // 绑定
    neko_render_apply_bindings(&neko_idraw->commands, &binds);

    // 提交绘制
    neko_render_draw_desc_t draw = NEKO_DEFAULT_VAL();
    draw.start = 0;
    draw.count = ct;
    neko_render_draw(&neko_idraw->commands, &draw);

    // 绘制后清理缓冲区
    neko_byte_buffer_clear(&neko_idraw->vertices);
}

// Core pipeline functions
void neko_idraw_blend_enabled(neko_immediate_draw_t* neko_idraw, bool enabled) {
    // Push a new pipeline?
    if (neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || neko_idraw->cache.pipeline.blend_enabled == enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set primitive type
    neko_idraw->cache.pipeline.blend_enabled = enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(neko_idraw);
}

// Core pipeline functions
void neko_idraw_depth_enabled(neko_immediate_draw_t* neko_idraw, bool enabled) {
    // Push a new pipeline?
    if (neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || neko_idraw->cache.pipeline.depth_enabled == (u16)enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set primitive type
    neko_idraw->cache.pipeline.depth_enabled = (u16)enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(neko_idraw);
}

void neko_idraw_stencil_enabled(neko_immediate_draw_t* neko_idraw, bool enabled) {
    // Push a new pipeline?
    if (neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || neko_idraw->cache.pipeline.stencil_enabled == (u16)enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set primitive type
    neko_idraw->cache.pipeline.stencil_enabled = (u16)enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(neko_idraw);
}

void neko_idraw_face_cull_enabled(neko_immediate_draw_t* neko_idraw, bool enabled) {
    // Push a new pipeline?
    if (neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES || neko_idraw->cache.pipeline.face_cull_enabled == (u16)enabled) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set primitive type
    neko_idraw->cache.pipeline.face_cull_enabled = (u16)enabled;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(neko_idraw);
}

NEKO_API_DECL void neko_idraw_texture(neko_immediate_draw_t* neko_idraw, neko_handle(neko_render_texture_t) texture) {
    // Push a new pipeline?
    if (neko_idraw->cache.texture.id == texture.id) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set texture
    neko_idraw->cache.texture = (texture.id && texture.id != UINT32_MAX) ? texture : neko_idraw()->tex_default;
}

NEKO_API_DECL void neko_idraw_pipeline_set(neko_immediate_draw_t* neko_idraw, neko_handle(neko_render_pipeline_t) pipeline) {
    neko_idraw_flush(neko_idraw);

    // Bind if valid
    if (pipeline.id) {
        neko_render_pipeline_bind(&neko_idraw->commands, pipeline);
        neko_idraw->flags |= NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES;
    }
    // Otherwise we set back to cache, clear vattributes, clear flag
    else {
        neko_idraw->flags &= ~NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES;
        neko_dyn_array_clear(neko_idraw->vattributes);
        neko_immediate_draw_set_pipeline(neko_idraw);
    }
}

NEKO_API_DECL void neko_idraw_vattr_list(neko_immediate_draw_t* neko_idraw, neko_idraw_vattr_type* list, size_t sz) {
    neko_idraw_flush(neko_idraw);

    neko_dyn_array_clear(neko_idraw->vattributes);
    u32 ct = sz / sizeof(neko_idraw_vattr_type);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(neko_idraw->vattributes, list[i]);
    }
}

NEKO_API_DECL void neko_idraw_vattr_list_mesh(neko_immediate_draw_t* neko_idraw, neko_asset_mesh_layout_t* layout, size_t sz) {
    neko_idraw_flush(neko_idraw);

    neko_dyn_array_clear(neko_idraw->vattributes);

#define VATTR_PUSH(TYPE)                                    \
    do {                                                    \
        neko_dyn_array_push(neko_idraw->vattributes, TYPE); \
    } while (0)

    u32 ct = sz / sizeof(neko_asset_mesh_layout_t);
    for (u32 i = 0; i < ct; ++i) {
        neko_asset_mesh_attribute_type type = layout[i].type;
        switch (type) {
            default:
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION:
                VATTR_PUSH(NEKO_IDRAW_VATTR_POSITION);
                break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD:
                VATTR_PUSH(NEKO_IDRAW_VATTR_UV);
                break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR:
                VATTR_PUSH(NEKO_IDRAW_VATTR_COLOR);
                break;
        }
    }
}

NEKO_API_DECL void neko_idraw_defaults(neko_immediate_draw_t* neko_idraw) {
    neko_idraw_flush(neko_idraw);

    // Set defaults for cache
    neko_idraw->cache.texture = neko_idraw()->tex_default;
    neko_idraw->cache.pipeline = neko_idraw_pipeline_state_default();
    neko_idraw->cache.pipeline.prim_type = 0x00;
    neko_idraw->cache.uv = neko_v2(0.f, 0.f);
    neko_idraw->cache.color = NEKO_COLOR_WHITE;
    neko_dyn_array_clear(neko_idraw->vattributes);

    // Reset flags
    neko_idraw->flags = 0x00;

    neko_immediate_draw_set_pipeline(neko_idraw);
}

NEKO_API_DECL void neko_idraw_tc2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 uv) {
    // Set cache register
    neko_idraw->cache.uv = uv;
}

void neko_idraw_tc2f(neko_immediate_draw_t* neko_idraw, f32 u, f32 v) {
    // Set cache register
    neko_idraw_tc2fv(neko_idraw, neko_v2(u, v));
}

void neko_idraw_c4ub(neko_immediate_draw_t* neko_idraw, u8 r, u8 g, u8 b, u8 a) {
    // Set cache color
    neko_idraw->cache.color = neko_color(r, g, b, a);
}

NEKO_API_DECL void neko_idraw_c4ubv(neko_immediate_draw_t* neko_idraw, neko_color_t c) { neko_idraw_c4ub(neko_idraw, c.r, c.g, c.b, c.a); }

void neko_idraw_v3fv(neko_immediate_draw_t* neko_idraw, neko_vec3 p) {
    if (neko_dyn_array_size(neko_idraw->vattributes)) {
        // Iterate through attributes and push into stream
        for (u32 i = 0; i < neko_dyn_array_size(neko_idraw->vattributes); ++i) {
            neko_idraw_vattr_type type = neko_idraw->vattributes[i];
            switch (type) {
                default: {
                } break;

                case NEKO_IDRAW_VATTR_POSITION: {
                    neko_byte_buffer_write(&neko_idraw->vertices, neko_vec3, p);
                } break;

                case NEKO_IDRAW_VATTR_COLOR: {
                    neko_byte_buffer_write(&neko_idraw->vertices, neko_color_t, neko_idraw->cache.color);
                } break;

                case NEKO_IDRAW_VATTR_UV: {
                    neko_byte_buffer_write(&neko_idraw->vertices, neko_vec2, neko_idraw->cache.uv);
                } break;
            }
        }
    } else {
        neko_immediate_vert_t v = NEKO_DEFAULT_VAL();
        v.position = p;
        v.uv = neko_idraw->cache.uv;
        v.color = neko_idraw->cache.color;
        neko_byte_buffer_write(&neko_idraw->vertices, neko_immediate_vert_t, v);
    }
}

void neko_idraw_v3f(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z) {
    // Push vert
    neko_idraw_v3fv(neko_idraw, neko_v3(x, y, z));
}

void neko_idraw_v2f(neko_immediate_draw_t* neko_idraw, f32 x, f32 y) {
    // Push vert
    neko_idraw_v3f(neko_idraw, x, y, 0.f);
}

void neko_idraw_v2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 v) {
    // Push vert
    neko_idraw_v3f(neko_idraw, v.x, v.y, 0.f);
}

void neko_idraw_push_matrix_ex(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type, bool flush) {
    // Flush
    if (flush) {
        neko_idraw_flush(neko_idraw);
    }

    // Push mode
    neko_dyn_array_push(neko_idraw->cache.modes, type);

    // Pop matrix off of stack
    switch (neko_dyn_array_back(neko_idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            neko_dyn_array_push(neko_idraw->cache.modelview, neko_dyn_array_back(neko_idraw->cache.modelview));
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            neko_dyn_array_push(neko_idraw->cache.projection, neko_dyn_array_back(neko_idraw->cache.projection));
        } break;
    }
}

void neko_idraw_push_matrix(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type) { neko_idraw_push_matrix_ex(neko_idraw, type, true); }

void neko_idraw_pop_matrix_ex(neko_immediate_draw_t* neko_idraw, bool flush) {
    // Flush
    if (flush) {
        neko_idraw_flush(neko_idraw);
    }

    // Pop matrix off of stack
    switch (neko_dyn_array_back(neko_idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            if (neko_dyn_array_size(neko_idraw->cache.modelview) > 1) {
                neko_dyn_array_pop(neko_idraw->cache.modelview);
            }
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            if (neko_dyn_array_size(neko_idraw->cache.projection) > 1) {
                neko_dyn_array_pop(neko_idraw->cache.projection);
            }
        } break;
    }

    if (neko_dyn_array_size(neko_idraw->cache.modes) > 1) {
        neko_dyn_array_pop(neko_idraw->cache.modes);
    }
}

void neko_idraw_pop_matrix(neko_immediate_draw_t* neko_idraw) { neko_idraw_pop_matrix_ex(neko_idraw, true); }

void neko_idraw_load_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m) {
    // Load matrix at current mode
    switch (neko_dyn_array_back(neko_idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            neko_mat4* mat = &neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1];
            *mat = m;
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            neko_mat4* mat = &neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1];
            *mat = m;
        } break;
    }
}

void neko_idraw_mul_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m) {
    static int i = 0;
    // Multiply current matrix at mode with m
    switch (neko_dyn_array_back(neko_idraw->cache.modes)) {
        case NEKO_IDRAW_MATRIX_MODELVIEW: {
            neko_mat4* mat = &neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1];
            *mat = neko_mat4_mul(*mat, m);
        } break;

        case NEKO_IDRAW_MATRIX_PROJECTION: {
            neko_mat4* mat = &neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1];
            *mat = neko_mat4_mul(*mat, m);
        } break;
    }
}

void neko_idraw_perspective(neko_immediate_draw_t* neko_idraw, f32 fov, f32 aspect, f32 n, f32 f) {
    // Set current matrix at mode to perspective
    neko_idraw_load_matrix(neko_idraw, neko_mat4_perspective(fov, aspect, n, f));
}

void neko_idraw_ortho(neko_immediate_draw_t* neko_idraw, f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    // Set current matrix at mode to ortho
    neko_idraw_load_matrix(neko_idraw, neko_mat4_ortho(l, r, b, t, n, f));
}

void neko_idraw_rotatef(neko_immediate_draw_t* neko_idraw, f32 angle, f32 x, f32 y, f32 z) {
    // Rotate current matrix at mode
    neko_idraw_mul_matrix(neko_idraw, neko_mat4_rotatev(angle, neko_v3(x, y, z)));
}

void neko_idraw_rotatev(neko_immediate_draw_t* neko_idraw, f32 angle, neko_vec3 v) { neko_idraw_rotatef(neko_idraw, angle, v.x, v.y, v.z); }

void neko_idraw_translatef(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z) {
    // Translate current matrix at mode
    neko_idraw_mul_matrix(neko_idraw, neko_mat4_translate(x, y, z));
}

void neko_idraw_translatev(neko_immediate_draw_t* neko_idraw, neko_vec3 v) {
    // Translate current matrix at mode
    neko_idraw_mul_matrix(neko_idraw, neko_mat4_translate(v.x, v.y, v.z));
}

void neko_idraw_scalef(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z) {
    // Scale current matrix at mode
    neko_idraw_mul_matrix(neko_idraw, neko_mat4_scale(x, y, z));
}

void neko_idraw_scalev(neko_immediate_draw_t* neko_idraw, neko_vec3 v) { neko_idraw_mul_matrix(neko_idraw, neko_mat4_scalev(v)); }

void neko_idraw_camera(neko_immediate_draw_t* neko_idraw, neko_camera_t* cam, u32 width, u32 height) {
    // 现在只需集成主窗口即可 将来需要占据视口堆栈的顶部
    neko_idraw_load_matrix(neko_idraw, neko_camera_get_view_projection(cam, width, height));
}

void neko_idraw_camera2d(neko_immediate_draw_t* neko_idraw, u32 width, u32 height) {
    // Flush previous
    neko_idraw_flush(neko_idraw);
    f32 l = 0.f, r = (f32)width, tp = 0.f, b = (f32)height;
    neko_mat4 ortho = neko_mat4_ortho(l, r, b, tp, -1.f, 1.f);
    neko_idraw_load_matrix(neko_idraw, ortho);
}

void neko_idraw_camera2d_ex(neko_immediate_draw_t* neko_idraw, f32 l, f32 r, f32 t, f32 b) {
    // Flush previous
    neko_idraw_flush(neko_idraw);
    neko_mat4 ortho = neko_mat4_ortho(l, r, b, t, -1.f, 1.f);
    neko_idraw_load_matrix(neko_idraw, ortho);
}

void neko_idraw_camera3d(neko_immediate_draw_t* neko_idraw, u32 width, u32 height) {
    // Flush previous
    neko_idraw_flush(neko_idraw);
    neko_camera_t c = neko_camera_perspective();
    neko_idraw_camera(neko_idraw, &c, width, height);
}

// Shape Drawing Utils
void neko_idraw_triangle(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type) {
    neko_idraw_trianglex(neko_idraw, x0, y0, 0.f, x1, y1, 0.f, x2, y2, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, r, g, b, a, type);
}

void neko_idraw_trianglev(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color, neko_render_primitive_type type) {
    neko_idraw_triangle(neko_idraw, a.x, a.y, b.x, b.y, c.x, c.y, color.r, color.g, color.b, color.a, type);
}

void neko_idraw_trianglex(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r, u8 g, u8 b,
                          u8 a, neko_render_primitive_type type) {
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);
            neko_idraw_c4ub(neko_idraw, r, g, b, a);
            neko_idraw_tc2f(neko_idraw, u0, v0);
            neko_idraw_v3f(neko_idraw, x0, y0, z0);
            neko_idraw_tc2f(neko_idraw, u1, v1);
            neko_idraw_v3f(neko_idraw, x1, y1, z1);
            neko_idraw_tc2f(neko_idraw, u2, v2);
            neko_idraw_v3f(neko_idraw, x2, y2, z2);
            neko_idraw_end(neko_idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            neko_idraw_line3D(neko_idraw, x0, y0, z0, x1, y1, z1, r, g, b, a);
            neko_idraw_line3D(neko_idraw, x1, y1, z1, x2, y2, z2, r, g, b, a);
            neko_idraw_line3D(neko_idraw, x2, y2, z2, x0, y0, z0, r, g, b, a);
        } break;
    }
}

void neko_idraw_trianglevx(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t color,
                           neko_render_primitive_type type) {
    neko_idraw_trianglex(neko_idraw, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, uv0.x, uv0.y, uv1.x, uv1.y, uv2.x, uv2.y, color.r, color.g, color.b, color.a, type);
}

NEKO_API_DECL void neko_idraw_trianglevxmc(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t c0, neko_color_t c1,
                                           neko_color_t c2, neko_render_primitive_type type) {
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);

            // First vert
            neko_idraw_c4ub(neko_idraw, c0.r, c0.g, c0.b, c0.a);
            neko_idraw_tc2f(neko_idraw, uv0.x, uv0.y);
            neko_idraw_v3f(neko_idraw, v0.x, v0.y, v0.z);

            // Second vert
            neko_idraw_c4ub(neko_idraw, c1.r, c1.g, c1.b, c1.a);
            neko_idraw_tc2f(neko_idraw, uv1.x, uv1.y);
            neko_idraw_v3f(neko_idraw, v1.x, v1.y, v1.z);

            // Third vert
            neko_idraw_c4ub(neko_idraw, c2.r, c2.g, c2.b, c2.a);
            neko_idraw_tc2f(neko_idraw, uv2.x, uv2.y);
            neko_idraw_v3f(neko_idraw, v2.x, v2.y, v2.z);

            neko_idraw_end(neko_idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            neko_idraw_line3Dmc(neko_idraw, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, c0.r, c0.g, c0.b, c0.a, c1.r, c1.g, c1.b, c1.a);
            neko_idraw_line3Dmc(neko_idraw, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, c1.r, c1.g, c1.b, c1.a, c2.r, c2.g, c2.b, c2.a);
            neko_idraw_line3Dmc(neko_idraw, v2.x, v2.y, v2.z, v0.x, v0.y, v0.z, c2.r, c2.g, c2.b, c2.a, c0.r, c0.g, c0.b, c0.a);
        } break;
    }
}

void neko_idraw_line3Dmc(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1) {
    neko_idraw_begin(neko_idraw, R_PRIMITIVE_LINES);

    neko_idraw_tc2f(neko_idraw, 0.f, 0.f);

    // First vert
    neko_idraw_c4ub(neko_idraw, r0, g0, b0, a0);
    neko_idraw_v3f(neko_idraw, x0, y0, z0);

    // Second vert
    neko_idraw_c4ub(neko_idraw, r1, g1, b1, a1);
    neko_idraw_v3f(neko_idraw, x1, y1, z1);
    neko_idraw_end(neko_idraw);
}

void neko_idraw_line3D(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r, u8 g, u8 b, u8 a) {
    neko_idraw_begin(neko_idraw, R_PRIMITIVE_LINES);
    neko_idraw_tc2f(neko_idraw, 0.f, 0.f);
    neko_idraw_c4ub(neko_idraw, r, g, b, a);
    neko_idraw_v3f(neko_idraw, x0, y0, z0);
    neko_idraw_v3f(neko_idraw, x1, y1, z1);
    neko_idraw_end(neko_idraw);
}

void neko_idraw_line3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 s, neko_vec3 e, neko_color_t color) {
    neko_idraw_line3D(neko_idraw, s.x, s.y, s.z, e.x, e.y, e.z, color.r, color.g, color.b, color.a);
}

void neko_idraw_line(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a) { neko_idraw_line3D(neko_idraw, x0, y0, 0.f, x1, y1, 0.f, r, g, b, a); }

void neko_idraw_linev(neko_immediate_draw_t* neko_idraw, neko_vec2 v0, neko_vec2 v1, neko_color_t c) { neko_idraw_line(neko_idraw, v0.x, v0.y, v1.x, v1.y, c.r, c.g, c.b, c.a); }

void neko_idraw_rectx(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, neko_render_primitive_type type) {
    // Shouldn't use triangles, because need to declare texture coordinates.
    switch (type) {
        case R_PRIMITIVE_LINES: {
            // First triangle
            neko_idraw_line(neko_idraw, l, b, r, b, _r, _g, _b, _a);
            neko_idraw_line(neko_idraw, r, b, r, t, _r, _g, _b, _a);
            neko_idraw_line(neko_idraw, r, t, l, t, _r, _g, _b, _a);
            neko_idraw_line(neko_idraw, l, t, l, b, _r, _g, _b, _a);
            // neko_idraw_triangle(neko_idraw, l, b, r, b, l, t, _r, _g, _b, _a, type);
            // Second triangle
            // neko_idraw_triangle(neko_idraw, r, b, r, t, l, t, _r, _g, _b, _a, type);
        } break;

        case R_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);

            neko_idraw_c4ub(neko_idraw, _r, _g, _b, _a);

            // First triangle
            neko_idraw_tc2f(neko_idraw, u0, v0);
            neko_idraw_v2f(neko_idraw, l, b);
            neko_idraw_tc2f(neko_idraw, u1, v0);
            neko_idraw_v2f(neko_idraw, r, b);
            neko_idraw_tc2f(neko_idraw, u0, v1);
            neko_idraw_v2f(neko_idraw, l, t);

            // Second triangle
            neko_idraw_tc2f(neko_idraw, u1, v0);
            neko_idraw_v2f(neko_idraw, r, b);
            neko_idraw_tc2f(neko_idraw, u1, v1);
            neko_idraw_v2f(neko_idraw, r, t);
            neko_idraw_tc2f(neko_idraw, u0, v1);
            neko_idraw_v2f(neko_idraw, l, t);

            neko_idraw_end(neko_idraw);

        } break;
    }
}

void neko_idraw_rect(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, u8 _r, u8 _g, u8 _b, u8 _a, neko_render_primitive_type type) {
    neko_idraw_rectx(neko_idraw, l, b, r, t, 0.f, 0.f, 1.f, 1.f, _r, _g, _b, _a, type);
}

void neko_idraw_rectv(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_color_t color, neko_render_primitive_type type) {
    neko_idraw_rectx(neko_idraw, bl.x, bl.y, tr.x, tr.y, 0.f, 0.f, 1.f, 1.f, color.r, color.g, color.b, color.a, type);
}

void neko_idraw_rectvx(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_render_primitive_type type) {
    neko_idraw_rectx(neko_idraw, bl.x, bl.y, tr.x, tr.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

void neko_idraw_rectvd(neko_immediate_draw_t* neko_idraw, neko_vec2 xy, neko_vec2 wh, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_render_primitive_type type) {
    neko_idraw_rectx(neko_idraw, xy.x, xy.y, xy.x + wh.x, xy.y + wh.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

NEKO_API_DECL void neko_idraw_rect3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 min, neko_vec3 max, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_render_primitive_type type) {
    const neko_vec3 vt0 = min;
    const neko_vec3 vt1 = neko_v3(max.x, min.y, min.z);
    const neko_vec3 vt2 = neko_v3(min.x, max.y, max.z);
    const neko_vec3 vt3 = max;

    switch (type) {
        case R_PRIMITIVE_LINES: {
            neko_idraw_line3Dv(neko_idraw, vt0, vt1, c);
            neko_idraw_line3Dv(neko_idraw, vt1, vt2, c);
            neko_idraw_line3Dv(neko_idraw, vt2, vt3, c);
            neko_idraw_line3Dv(neko_idraw, vt3, vt0, c);
        } break;

        case R_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);

            neko_idraw_c4ub(neko_idraw, c.r, c.g, c.b, c.a);

            const f32 u0 = uv0.x;
            const f32 u1 = uv1.x;
            const f32 v0 = uv0.y;
            const f32 v1 = uv1.y;

            // First triangle
            neko_idraw_c4ub(neko_idraw, c.r, c.g, c.b, c.a);
            neko_idraw_tc2f(neko_idraw, u0, v0);
            neko_idraw_v3fv(neko_idraw, vt0);
            neko_idraw_tc2f(neko_idraw, u0, v1);
            neko_idraw_v3fv(neko_idraw, vt2);
            neko_idraw_tc2f(neko_idraw, u1, v0);
            neko_idraw_v3fv(neko_idraw, vt1);

            // Second triangle
            neko_idraw_c4ub(neko_idraw, c.r, c.g, c.b, c.a);
            neko_idraw_tc2f(neko_idraw, u0, v1);
            neko_idraw_v3fv(neko_idraw, vt2);
            neko_idraw_tc2f(neko_idraw, u1, v1);
            neko_idraw_v3fv(neko_idraw, vt3);
            neko_idraw_tc2f(neko_idraw, u1, v0);
            neko_idraw_v3fv(neko_idraw, vt1);

            neko_idraw_end(neko_idraw);

        } break;
    }
}

NEKO_API_DECL void neko_idraw_rect3Dvd(neko_immediate_draw_t* neko_idraw, neko_vec3 xyz, neko_vec3 whd, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_render_primitive_type type) {
    neko_idraw_rect3Dv(neko_idraw, xyz, neko_vec3_add(xyz, whd), uv0, uv1, c, type);
}

void neko_idraw_circle_sector(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                              neko_render_primitive_type type) {
    if (radius <= 0.0f) {
        radius = 0.1f;
    }

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        int32_t tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        f32 th = acosf(2 * powf(1 - neko_idraw_smooth_circle_error_rate / radius, 2) - 1);
        segments = (int32_t)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) {
            segments = 4;
        }
    }

    f32 step = (f32)(end_angle - start_angle) / (f32)segments;
    f32 angle = (f32)start_angle;
    NEKO_FOR_RANGE_N(segments, i) {
        neko_vec2 _a = neko_v2(cx, cy);
        neko_vec2 _b = neko_v2(cx + sinf(neko_idraw_deg2rad * angle) * radius, cy + cosf(neko_idraw_deg2rad * angle) * radius);
        neko_vec2 _c = neko_v2(cx + sinf(neko_idraw_deg2rad * (angle + step)) * radius, cy + cosf(neko_idraw_deg2rad * (angle + step)) * radius);
        neko_idraw_trianglev(neko_idraw, _a, _b, _c, neko_color(r, g, b, a), type);
        angle += step;
    }
}

void neko_idraw_circle_sectorvx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, neko_color_t color,
                                neko_render_primitive_type type) {
    if (radius <= 0.0f) {
        radius = 0.1f;
    }

    // Cache elements of center vector
    f32 cx = c.x, cy = c.y, cz = c.z;

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        int32_t tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        f32 th = acosf(2 * powf(1 - neko_idraw_smooth_circle_error_rate / radius, 2) - 1);
        segments = (int32_t)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) {
            segments = 4;
        }
    }

    f32 step = (f32)(end_angle - start_angle) / (f32)segments;
    f32 angle = (f32)start_angle;
    NEKO_FOR_RANGE_N(segments, i) {
        neko_vec3 _a = neko_v3(cx, cy, cz);
        neko_vec3 _b = neko_v3(cx + sinf(neko_idraw_deg2rad * angle) * radius, cy + cosf(neko_idraw_deg2rad * angle) * radius, cz);
        neko_vec3 _c = neko_v3(cx + sinf(neko_idraw_deg2rad * (angle + step)) * radius, cy + cosf(neko_idraw_deg2rad * (angle + step)) * radius, cz);
        neko_idraw_trianglevx(neko_idraw, _a, _b, _c, neko_v2s(0.f), neko_v2s(0.5f), neko_v2s(1.f), color, type);
        angle += step;
    }
}

void neko_idraw_circle(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type) {
    neko_idraw_circle_sector(neko_idraw, cx, cy, radius, 0, 360, segments, r, g, b, a, type);
}

void neko_idraw_circlevx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t segments, neko_color_t color, neko_render_primitive_type type) {
    neko_idraw_circle_sectorvx(neko_idraw, c, radius, 0, 360, segments, color, type);
}

NEKO_API_DECL void neko_idraw_arc(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                                  neko_render_primitive_type type) {
    if (start_angle == end_angle) return;

    if (start_angle > end_angle) {
        f32 tmp = end_angle;
        end_angle = start_angle;
        start_angle = tmp;
    }

    if (radius_outer < radius_inner) {
        f32 tmp = radius_outer;
        radius_outer = radius_inner;
        radius_inner = tmp;
    }

    int32_t min_segments = (int32_t)((end_angle - start_angle) / 90.f);

    if (segments < min_segments) {
        f32 th = acosf(2 * powf(1 - neko_idraw_smooth_circle_error_rate / radius_outer, 2) - 1);
        segments = (int32_t)((end_angle - start_angle) * ceilf(2 * neko_pi / th) / 360);
        if (segments <= 0) segments = min_segments;
    }

    // Not a ring
    if (radius_inner <= 0.0f) {
        // BALLS
        neko_idraw_circle_sector(neko_idraw, cx, cy, radius_outer, (s32)start_angle, (s32)end_angle, segments, r, g, b, a, type);
        return;
    }

    f32 step = (end_angle - start_angle) / (f32)segments;
    f32 angle = start_angle;

    for (int i = 0; i < segments; i++) {
        f32 ar = neko_deg2rad(angle);
        f32 ars = neko_deg2rad((angle + step));

        neko_idraw_trianglev(neko_idraw, neko_v2(cx + sinf(ar) * radius_inner, cy + cosf(ar) * radius_inner), neko_v2(cx + sinf(ars) * radius_inner, cy + cosf(ars) * radius_inner),
                             neko_v2(cx + sinf(ar) * radius_outer, cy + cosf(ar) * radius_outer), neko_color(r, g, b, a), type);

        neko_idraw_trianglev(neko_idraw, neko_v2(cx + sinf(ars) * radius_inner, cy + cosf(ars) * radius_inner), neko_v2(cx + sinf(ars) * radius_outer, cy + cosf(ars) * radius_outer),
                             neko_v2(cx + sinf(ar) * radius_outer, cy + cosf(ar) * radius_outer), neko_color(r, g, b, a), type);

        angle += step;
    }
}

void neko_idraw_box(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type) {
    f32 width = hx;
    f32 height = hy;
    f32 length = hz;

    neko_vec3 v0 = neko_v3(x - width, y - height, z + length);
    neko_vec3 v1 = neko_v3(x + width, y - height, z + length);
    neko_vec3 v2 = neko_v3(x - width, y + height, z + length);
    neko_vec3 v3 = neko_v3(x + width, y + height, z + length);
    neko_vec3 v4 = neko_v3(x - width, y - height, z - length);
    neko_vec3 v5 = neko_v3(x - width, y + height, z - length);
    neko_vec3 v6 = neko_v3(x + width, y - height, z - length);
    neko_vec3 v7 = neko_v3(x + width, y + height, z - length);

    neko_color_t color = neko_color(r, g, b, a);

    switch (type) {
        case R_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);
            {
                neko_vec2 uv0 = neko_v2(0.f, 0.f);
                neko_vec2 uv1 = neko_v2(1.f, 0.f);
                neko_vec2 uv2 = neko_v2(0.f, 1.f);
                neko_vec2 uv3 = neko_v2(1.f, 1.f);

                // Front Face
                neko_idraw_trianglevx(neko_idraw, v0, v1, v2, uv0, uv1, uv2, color, type);
                neko_idraw_trianglevx(neko_idraw, v3, v2, v1, uv3, uv2, uv1, color, type);

                // Back face
                neko_idraw_trianglevx(neko_idraw, v6, v5, v7, uv0, uv3, uv2, color, type);
                neko_idraw_trianglevx(neko_idraw, v6, v4, v5, uv0, uv1, uv3, color, type);

                // Top face
                neko_idraw_trianglevx(neko_idraw, v7, v2, v3, uv0, uv3, uv2, color, type);
                neko_idraw_trianglevx(neko_idraw, v7, v5, v2, uv0, uv1, uv3, color, type);

                // Bottom face
                neko_idraw_trianglevx(neko_idraw, v4, v1, v0, uv0, uv3, uv2, color, type);
                neko_idraw_trianglevx(neko_idraw, v4, v6, v1, uv0, uv1, uv3, color, type);

                // Right face
                neko_idraw_trianglevx(neko_idraw, v1, v7, v3, uv0, uv3, uv2, color, type);
                neko_idraw_trianglevx(neko_idraw, v1, v6, v7, uv0, uv1, uv3, color, type);

                // Left face
                neko_idraw_trianglevx(neko_idraw, v4, v2, v5, uv0, uv3, uv2, color, type);
                neko_idraw_trianglevx(neko_idraw, v4, v0, v2, uv0, uv1, uv3, color, type);
            }
            neko_idraw_end(neko_idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            neko_color_t color = neko_color(r, g, b, a);
            neko_idraw_tc2f(neko_idraw, 0.f, 0.f);

            // Front face
            neko_idraw_line3Dv(neko_idraw, v0, v1, color);
            neko_idraw_line3Dv(neko_idraw, v1, v3, color);
            neko_idraw_line3Dv(neko_idraw, v3, v2, color);
            neko_idraw_line3Dv(neko_idraw, v2, v0, color);

            // Back face
            neko_idraw_line3Dv(neko_idraw, v4, v6, color);
            neko_idraw_line3Dv(neko_idraw, v6, v7, color);
            neko_idraw_line3Dv(neko_idraw, v7, v5, color);
            neko_idraw_line3Dv(neko_idraw, v5, v4, color);

            // Right face
            neko_idraw_line3Dv(neko_idraw, v1, v6, color);
            neko_idraw_line3Dv(neko_idraw, v6, v7, color);
            neko_idraw_line3Dv(neko_idraw, v7, v3, color);
            neko_idraw_line3Dv(neko_idraw, v3, v1, color);

            // Left face
            neko_idraw_line3Dv(neko_idraw, v4, v6, color);
            neko_idraw_line3Dv(neko_idraw, v6, v1, color);
            neko_idraw_line3Dv(neko_idraw, v1, v0, color);
            neko_idraw_line3Dv(neko_idraw, v0, v4, color);

            // Bottom face
            neko_idraw_line3Dv(neko_idraw, v5, v7, color);
            neko_idraw_line3Dv(neko_idraw, v7, v3, color);
            neko_idraw_line3Dv(neko_idraw, v3, v2, color);
            neko_idraw_line3Dv(neko_idraw, v2, v5, color);

            // Top face
            neko_idraw_line3Dv(neko_idraw, v0, v4, color);
            neko_idraw_line3Dv(neko_idraw, v4, v5, color);
            neko_idraw_line3Dv(neko_idraw, v5, v2, color);
            neko_idraw_line3Dv(neko_idraw, v2, v0, color);

        } break;
    }
}

void neko_idraw_sphere(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type) {
    // Modified from: http://www.songho.ca/opengl/gl_sphere.html
    const u32 stacks = 16;
    const u32 sectors = 32;
    f32 sector_step = 2.f * (f32)neko_pi / (f32)sectors;
    f32 stack_step = (f32)neko_pi / (f32)stacks;
    struct {
        neko_vec3 p;
        neko_vec2 uv;
    } v0, v1, v2, v3;
    neko_color_t color = neko_color(r, g, b, a);

// TODO: Need to get these verts to be positioned correctly (translate then rotate all verts to correct for odd 90 degree rotation)
#define make_vert(V, I, J, XZ, SECANGLE)                   \
    do {                                                   \
        /* vertex position (x, y, z) */                    \
        V.p.x = cx + (XZ) * cosf((SECANGLE));              \
        V.p.z = cz + (XZ) * sinf((SECANGLE));              \
        /* vertex tex coord (s, t) range between [0, 1] */ \
        V.uv.x = (f32)(J) / sectors;                       \
        V.uv.y = (f32)(I) / stacks;                        \
    } while (0)

#define push_vert(V)                               \
    do {                                           \
        neko_idraw_tc2f(neko_idraw, V.s, V.t);     \
        neko_idraw_v3f(neko_idraw, V.x, V.y, V.z); \
    } while (0)

    for (u32 i = 0; i < stacks; ++i) {
        f32 sa0 = neko_pi / 2.f - i * stack_step;
        f32 sa1 = neko_pi / 2.f - (i + 1) * stack_step;
        f32 xz0 = radius * cosf(sa0);
        f32 xz1 = radius * cosf(sa1);
        f32 y0 = cy + radius * sinf(sa0);  // r * sin(u)
        f32 y1 = cy + radius * sinf(sa1);  // r * sin(u)

        v0.p.y = y0;
        v1.p.y = y0;
        v2.p.y = y1;
        v3.p.y = y1;

        for (u32 j = 0; j < sectors; ++j) {
            f32 sca0 = j * sector_step;  // starting from 0 to 2pi
            f32 sca1 = (j + 1) * sector_step;

            // Make verts
            make_vert(v0, i, j, xz0, sca0);
            make_vert(v1, i, j + 1, xz0, sca1);
            make_vert(v2, i + 1, j, xz1, sca0);
            make_vert(v3, i + 1, j + 1, xz1, sca1);

            // First triangle
            neko_idraw_trianglevx(neko_idraw, v0.p, v3.p, v2.p, v0.uv, v3.uv, v2.uv, color, type);
            // Second triangle
            neko_idraw_trianglevx(neko_idraw, v0.p, v1.p, v3.p, v0.uv, v1.uv, v3.uv, color, type);
        }
    }
}

// Modified from Raylib's implementation
void neko_idraw_bezier(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a) {
    neko_vec2 start = neko_v2(x0, y0);
    neko_vec2 end = neko_v2(x1, y1);
    neko_vec2 previous = start;
    neko_vec2 current = NEKO_DEFAULT_VAL();
    neko_color_t color = neko_color(r, g, b, a);
    const u32 bezier_line_divisions = 24;

    for (int i = 1; i <= bezier_line_divisions; i++) {
        current.y = neko_ease_cubic_in_out((f32)i, start.y, end.y - start.y, (f32)bezier_line_divisions);
        current.x = previous.x + (end.x - start.x) / (f32)bezier_line_divisions;
        neko_idraw_linev(neko_idraw, previous, current, color);
        previous = current;
    }
}

NEKO_API_DECL void neko_idraw_cylinder(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 r_top, f32 r_bottom, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a,
                                       neko_render_primitive_type type) {
    if (sides < 3) sides = 3;

    int32_t numVertex = sides * 8;
    const f32 hh = height * 0.5f;

    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);
            {
                neko_idraw_c4ub(neko_idraw, r, g, b, a);

                if (sides < 3) sides = 3;

                numVertex = sides * 6;

                if (r_top > 0) {
                    // Draw Body -------------------------------------------------------------------------------------
                    for (int i = 0; i < 360; i += 360 / sides) {
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * i) * r_bottom);  // Bottom Left
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh,
                                       z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);                                                                                // Bottom Right
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top);  // Top Right

                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * i) * r_top);                                        // Top Left
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * i) * r_bottom);                                  // Bottom Left
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top);  // Top Right
                    }

                    // Draw Cap --------------------------------------------------------------------------------------
                    for (int i = 0; i < 360; i += 360 / sides) {
                        neko_idraw_v3f(neko_idraw, x + 0, y + hh, z + 0);
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * i) * r_top);
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top);
                    }
                } else {
                    // Draw Cone -------------------------------------------------------------------------------------
                    for (int i = 0; i < 360; i += 360 / sides) {
                        neko_idraw_v3f(neko_idraw, x + 0, y + hh, z + 0);
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * i) * r_bottom);
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);
                    }
                }

                // Draw Base -----------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360 / sides) {
                    neko_idraw_v3f(neko_idraw, x + 0, y - hh, z + 0);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * i) * r_bottom);
                }
            }
            neko_idraw_end(neko_idraw);
        } break;

        case R_PRIMITIVE_LINES: {
            neko_idraw_begin(neko_idraw, R_PRIMITIVE_LINES);
            {
                neko_idraw_c4ub(neko_idraw, r, g, b, a);

                for (int32_t i = 0; i < 360; i += 360 / sides) {
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_bottom, y - hh, cosf(neko_idraw_deg2rad * i) * r_bottom);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);

                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top);

                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * i) * r_top);

                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * i) * r_top);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * i) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * i) * r_bottom);
                }

                // Draw Top/Bottom circles
                for (int i = 0; i < 360; i += 360 / sides) {
                    neko_idraw_v3f(neko_idraw, x, y - hh, z);
                    neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom, y - hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_bottom);

                    if (r_top) {
                        neko_idraw_v3f(neko_idraw, x + 0, y + hh, z + 0);
                        neko_idraw_v3f(neko_idraw, x + sinf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top, y + hh, z + cosf(neko_idraw_deg2rad * (i + 360.0f / sides)) * r_top);
                    }
                }
            }
            neko_idraw_end(neko_idraw);

        } break;
    }
}

NEKO_API_DECL void neko_idraw_cone(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type) {
    neko_idraw_cylinder(neko_idraw, x, y, z, 0.f, radius, height, sides, r, g, b, a, type);
}

NEKO_API_DECL void neko_idraw_text(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, const char* text, const neko_asset_ascii_font_t* fp, b32 flip_vertical, neko_color_t col) {
    // 如果没有指定字体 则使用默认字体
    if (!fp) {
        fp = &neko_idraw()->font_default;
    }

    neko_idraw_texture(neko_idraw, fp->texture.hndl);

    neko_mat4 rot = neko_mat4_rotatev(neko_deg2rad(-180.f), NEKO_XAXIS);

    // Get total dimensions of text
    neko_vec2 td = neko_asset_ascii_font_text_dimensions(fp, text, -1);
    f32 th = neko_asset_ascii_font_max_height(fp);

    // Move text to accomdate height
    y += NEKO_MAX(td.y, th);

    // Needs to be fixed in here. Not elsewhere.
    neko_idraw_begin(neko_idraw, R_PRIMITIVE_TRIANGLES);
    {
        neko_idraw_c4ub(neko_idraw, col.r, col.g, col.b, col.a);
        while (text[0] != '\0') {
            char c = text[0];
            if (c >= 32 && c <= 127) {
                stbtt_aligned_quad q = NEKO_DEFAULT_VAL();
                stbtt_GetBakedQuad((stbtt_bakedchar*)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);

                neko_vec3 v0 = neko_v3(q.x0, q.y0, 0.f);  // TL
                neko_vec3 v1 = neko_v3(q.x1, q.y0, 0.f);  // TR
                neko_vec3 v2 = neko_v3(q.x0, q.y1, 0.f);  // BL
                neko_vec3 v3 = neko_v3(q.x1, q.y1, 0.f);  // BR

                if (flip_vertical) {
                    v0 = neko_mat4_mul_vec3(rot, v0);
                    v1 = neko_mat4_mul_vec3(rot, v1);
                    v2 = neko_mat4_mul_vec3(rot, v2);
                    v3 = neko_mat4_mul_vec3(rot, v3);
                }

                neko_vec2 uv0 = neko_v2(q.s0, q.t0);  // TL
                neko_vec2 uv1 = neko_v2(q.s1, q.t0);  // TR
                neko_vec2 uv2 = neko_v2(q.s0, q.t1);  // BL
                neko_vec2 uv3 = neko_v2(q.s1, q.t1);  // BR

                neko_idraw_tc2fv(neko_idraw, uv0);
                neko_idraw_v3fv(neko_idraw, v0);

                neko_idraw_tc2fv(neko_idraw, uv3);
                neko_idraw_v3fv(neko_idraw, v3);

                neko_idraw_tc2fv(neko_idraw, uv2);
                neko_idraw_v3fv(neko_idraw, v2);

                neko_idraw_tc2fv(neko_idraw, uv0);
                neko_idraw_v3fv(neko_idraw, v0);

                neko_idraw_tc2fv(neko_idraw, uv1);
                neko_idraw_v3fv(neko_idraw, v1);

                neko_idraw_tc2fv(neko_idraw, uv3);
                neko_idraw_v3fv(neko_idraw, v3);
            }
            text++;
        }
    }
    neko_idraw_end(neko_idraw);
}

// View/Scissor commands
NEKO_API_DECL void neko_idraw_set_view_scissor(neko_immediate_draw_t* neko_idraw, u32 x, u32 y, u32 w, u32 h) {
    // Flush previous
    neko_idraw_flush(neko_idraw);

    // Set graphics viewport scissor
    neko_render_set_view_scissor(&neko_idraw->commands, x, y, w, h);
}

// Final Submit / Merge
NEKO_API_DECL void neko_idraw_draw(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb) {
    // Final flush (if necessary)(this might be a part of neko_idraw_end() instead)
    neko_idraw_flush(neko_idraw);

    // Merge neko_idraw commands to end of cb
    neko_byte_buffer_write_bulk(&cb->commands, neko_idraw->commands.commands.data, neko_idraw->commands.commands.position);

    // Increase number of commands of merged buffer
    cb->num_commands += neko_idraw->commands.num_commands;

    // Reset cache
    neko_idraw_reset(neko_idraw);
}

NEKO_API_DECL void neko_idraw_renderpass_submit(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_color_t c) {
    neko_render_clear_action_t action = NEKO_DEFAULT_VAL();
    action.color[0] = (f32)c.r / 255.f;
    action.color[1] = (f32)c.g / 255.f;
    action.color[2] = (f32)c.b / 255.f;
    action.color[3] = (f32)c.a / 255.f;
    neko_renderpass_t pass = NEKO_DEFAULT_VAL();
    neko_render_renderpass_begin(cb, pass);
    neko_render_set_viewport(cb, (u32)viewport.x, (u32)viewport.y, (u32)viewport.z, (u32)viewport.w);
    neko_render_clear(cb, action);
    neko_idraw_draw(neko_idraw, cb);
    neko_render_renderpass_end(cb);
}

NEKO_API_DECL void neko_idraw_renderpass_submit_ex(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_render_clear_action_t action) {
    neko_renderpass_t pass = NEKO_DEFAULT_VAL();
    neko_render_renderpass_begin(cb, pass);
    neko_render_set_viewport(cb, (u32)viewport.x, (u32)viewport.y, (u32)viewport.z, (u32)viewport.w);
    neko_render_clear(cb, action);
    neko_idraw_draw(neko_idraw, cb);
    neko_render_renderpass_end(cb);
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------

// Modified from stb lib for embedding without collisions
NEKO_API_DECL unsigned int neko_decompress_length(const unsigned char* input) { return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11]; }

static unsigned char* neko__barrier;
static unsigned char* neko__barrier2;
static unsigned char* neko__barrier3;
static unsigned char* neko__barrier4;

static unsigned char* neko__dout;
static void neko__match(const unsigned char* data, unsigned int length) {
    // INVERSE of memmove... write each byte before copying the next...
    assert(neko__dout + length <= neko__barrier);
    if (neko__dout + length > neko__barrier) {
        neko__dout += length;
        return;
    }
    if (data < neko__barrier4) {
        neko__dout = neko__barrier + 1;
        return;
    }
    while (length--) *neko__dout++ = *data++;
}

static void neko__lit(const unsigned char* data, unsigned int length) {
    assert(neko__dout + length <= neko__barrier);
    if (neko__dout + length > neko__barrier) {
        neko__dout += length;
        return;
    }
    if (data < neko__barrier2) {
        neko__dout = neko__barrier + 1;
        return;
    }
    memcpy(neko__dout, data, length);
    neko__dout += length;
}

#define neko__in2(x) ((i[x] << 8) + i[(x) + 1])
#define neko__in3(x) ((i[x] << 16) + neko__in2((x) + 1))
#define neko__in4(x) ((i[x] << 24) + neko__in3((x) + 1))

static unsigned char* neko_decompress_token(unsigned char* i) {
    if (*i >= 0x20) {  // use fewer if's for cases that expand small
        if (*i >= 0x80)
            neko__match(neko__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)
            neko__match(neko__dout - (neko__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
        else /* *i >= 0x20 */
            neko__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else {  // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)
            neko__match(neko__dout - (neko__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
        else if (*i >= 0x10)
            neko__match(neko__dout - (neko__in3(0) - 0x100000 + 1), neko__in2(3) + 1), i += 5;
        else if (*i >= 0x08)
            neko__lit(i + 2, neko__in2(0) - 0x0800 + 1), i += 2 + (neko__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)
            neko__lit(i + 3, neko__in2(1) + 1), i += 3 + (neko__in2(1) + 1);
        else if (*i == 0x06)
            neko__match(neko__dout - (neko__in3(1) + 1), i[4] + 1), i += 5;
        else if (*i == 0x04)
            neko__match(neko__dout - (neko__in3(1) + 1), neko__in2(4) + 1), i += 6;
    }
    return i;
}

unsigned int neko_adler32(unsigned int adler32, unsigned char* buffer, unsigned int buflen) {
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i = 0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i) s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

NEKO_API_DECL unsigned int neko_decompress(unsigned char* output, unsigned char* i, unsigned int length) {
    u32 olen;
    if (neko__in4(0) != 0x57bC0000) return 0;
    if (neko__in4(4) != 0) return 0;  // error! stream is > 4GB
    olen = neko_decompress_length(i);
    neko__barrier2 = i;
    neko__barrier3 = i + length;
    neko__barrier = output + olen;
    neko__barrier4 = output;
    i += 16;

    neko__dout = output;
    while (1) {
        unsigned char* old_i = i;
        i = neko_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                assert(neko__dout == output + olen);
                if (neko__dout != output + olen) return 0;
                if (neko_adler32(1, output, olen) != (u32)neko__in4(2)) return 0;
                return olen;
            } else {
                assert(0); /* NOTREACHED */
                return 0;
            }
        }
        assert(neko__dout <= output + olen);
        if (neko__dout > output + olen) return 0;
    }
}

NEKO_API_DECL unsigned int __neko_internal_Decode85Byte(char c) { return c >= '\\' ? c - 36 : c - 35; }
NEKO_API_DECL void __neko_internal_Decode85(const unsigned char* src, unsigned char* dst) {
    while (*src) {
        unsigned int tmp =
                __neko_internal_Decode85Byte(src[0]) +
                85 * (__neko_internal_Decode85Byte(src[1]) + 85 * (__neko_internal_Decode85Byte(src[2]) + 85 * (__neko_internal_Decode85Byte(src[3]) + 85 * __neko_internal_Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF);
        dst[1] = ((tmp >> 8) & 0xFF);
        dst[2] = ((tmp >> 16) & 0xFF);
        dst[3] = ((tmp >> 24) & 0xFF);  // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

static const char __neko_proggy_clean_ttf_compressed_data_base85[11980 + 1] =
        "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
        "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
        "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
        "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
        "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
        "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
        "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
        "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
        "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
        "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
        "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
        "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
        "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
        "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
        "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
        "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
        "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
        "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
        "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
        "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
        "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
        "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
        "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
        "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
        "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
        "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
        "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
        "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
        "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
        "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
        ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
        "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
        "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
        "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
        "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
        "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
        "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
        ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
        "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
        "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
        "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
        "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
        "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
        "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
        "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
        "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
        ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
        "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
        "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
        ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
        "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
        "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
        "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
        ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
        "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
        "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
        "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
        "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
        "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
        "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
        "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
        "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
        "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
        "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
        "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
        "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
        "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
        "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
        ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
        "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
        "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
        "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
        "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
        "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
        "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
        "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
        "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
        ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
        "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
        "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
        "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
        "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
        "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
        "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
        "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
        "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

NEKO_API_DECL const char* __neko_internal_GetDefaultCompressedFontDataTTFBase85() { return __neko_proggy_clean_ttf_compressed_data_base85; }

NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_create(const neko_draw_pipeline_desc_t* desc) {
    neko_draw_pipeline_t pip = NEKO_DEFAULT_VAL();

    if (!desc) {
        NEKO_ASSERT(false);
        return pip;
    }

    pip.hndl = neko_render_pipeline_create(desc->pip_desc);
    pip.ublock = neko_draw_uniform_block_create(&desc->ublock_desc);
    pip.desc = desc->pip_desc;
    pip.desc.layout.attrs = neko_malloc(desc->pip_desc.layout.size);
    memcpy(pip.desc.layout.attrs, desc->pip_desc.layout.attrs, desc->pip_desc.layout.size);
    return pip;
}

NEKO_API_DECL neko_draw_uniform_block_t neko_draw_uniform_block_create(const neko_draw_uniform_block_desc_t* desc) {
    neko_draw_uniform_block_t block = NEKO_DEFAULT_VAL();

    if (!desc) return block;

    // Iterate through layout, construct uniforms, place them into hash table
    uint32_t offset = 0;
    uint32_t image2D_offset = 0;
    uint32_t ct = desc->size / sizeof(neko_draw_uniform_desc_t);
    for (uint32_t i = 0; i < ct; ++i) {
        neko_draw_uniform_desc_t* ud = &desc->layout[i];

        neko_draw_uniform_t u = NEKO_DEFAULT_VAL();
        neko_render_uniform_desc_t u_desc = NEKO_DEFAULT_VAL();
        neko_render_uniform_layout_desc_t u_layout = NEKO_DEFAULT_VAL();
        u_layout.type = ud->type;
        memcpy(u_desc.name, ud->name, 64);
        u_desc.layout = &u_layout;
        u.binding = ud->binding;
        u.type = ud->type;

        // Determine offset/hndl
        switch (ud->type) {
            case R_UNIFORM_IMAGE2D_RGBA32F: {
                u.offset = image2D_offset;
            } break;

            default: {
                u.hndl = neko_render_uniform_create(u_desc);
                u.offset = offset;
            } break;
        }

        // Add to data offset based on type
        switch (ud->type) {
            default:
            case R_UNIFORM_FLOAT:
                offset += sizeof(float);
                break;
            case R_UNIFORM_INT:
                offset += sizeof(int32_t);
                break;
            case R_UNIFORM_VEC2:
                offset += sizeof(neko_vec2);
                break;
            case R_UNIFORM_VEC3:
                offset += sizeof(neko_vec3);
                break;
            case R_UNIFORM_VEC4:
                offset += sizeof(neko_vec4);
                break;
            case R_UNIFORM_MAT4:
                offset += sizeof(neko_mat4);
                break;
            case R_UNIFORM_SAMPLER2D:
                offset += sizeof(neko_handle(neko_render_texture_t));
                break;
            case R_UNIFORM_USAMPLER2D:
                offset += sizeof(neko_handle(neko_render_texture_t));
                break;
            case R_UNIFORM_IMAGE2D_RGBA32F: {
                image2D_offset += sizeof(neko_handle(neko_render_texture_t));
            } break;
        }

        // Add uniform to block with name as key
        uint64_t key = neko_hash_str64(ud->name);
        neko_dyn_array_push(block.uniforms, u);
        neko_hash_table_insert(block.lookup, key, neko_dyn_array_size(block.uniforms) - 1);
    }
    block.size = offset;

    return block;
}

NEKO_API_DECL neko_draw_texture_t neko_draw_texture_create(neko_render_texture_desc_t desc) { return neko_render_texture_create(desc); }

NEKO_API_DECL neko_draw_material_t neko_draw_material_create(neko_draw_material_desc_t* desc) {
    neko_draw_material_t mat = NEKO_DEFAULT_VAL();

    if (!desc) {
        NEKO_ASSERT(false);
        return mat;
    }

    // Set desc information to defaults if not provided.
    if (!desc->pip_func.func) desc->pip_func.func = neko_draw_raw_data_default_impl;
    neko_draw_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&desc->pip_func, neko_draw_pipeline_t);
    NEKO_ASSERT(pip);

    mat.desc = *desc;
    mat.uniform_data = neko_byte_buffer_new();
    mat.image_buffer_data = neko_byte_buffer_new();

    neko_byte_buffer_resize(&mat.uniform_data, pip->ublock.size);
    neko_byte_buffer_memset(&mat.uniform_data, 0);
    return mat;
}

NEKO_API_DECL neko_draw_mesh_t neko_draw_mesh_create(const neko_draw_mesh_desc_t* desc) {
    neko_draw_mesh_t mesh = NEKO_DEFAULT_VAL();

    if (!desc) {
        return mesh;
    }

    const uint32_t mesh_count = desc->size / sizeof(neko_draw_mesh_raw_data_t);

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i) {
        neko_draw_mesh_raw_data_t* m = &desc->meshes[i];

        for (uint32_t p = 0; p < neko_dyn_array_size(m->primitives); ++p) {
            // Get raw vertex data
            neko_draw_mesh_vertex_data_t* vdata = &m->primitives[p];

            // Construct primitive
            neko_draw_mesh_primitive_t prim = NEKO_DEFAULT_VAL();
            prim.count = vdata->count;

            // Positions
            if (vdata->positions.data) {
                neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                vdesc.data = vdata->positions.data;
                vdesc.size = vdata->positions.size;
                prim.stream.positions = neko_render_vertex_buffer_create(vdesc);
                if (!desc->keep_data) {
                    neko_free(vdata->positions.data);
                }
            }

            // Normals
            if (vdata->normals.data) {
                neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                vdesc.data = vdata->normals.data;
                vdesc.size = vdata->normals.size;
                prim.stream.normals = neko_render_vertex_buffer_create(vdesc);
                if (!desc->keep_data) {
                    neko_free(vdata->normals.data);
                }
            }

            // Tangents
            if (vdata->tangents.data) {
                neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                vdesc.data = vdata->tangents.data;
                vdesc.size = vdata->tangents.size;
                prim.stream.tangents = neko_render_vertex_buffer_create(vdesc);
                if (!desc->keep_data) {
                    neko_free(vdata->tangents.data);
                }
            }

            // Texcoords
            for (uint32_t j = 0; j < NEKO_GFXT_TEX_COORD_MAX; ++j) {
                if (vdata->tex_coords[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->tex_coords[j].data;
                    vdesc.size = vdata->tex_coords[j].size;
                    prim.stream.tex_coords[j] = neko_render_vertex_buffer_create(vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->tex_coords[j].data);
                    }
                }
            }

            // Colors
            for (uint32_t j = 0; j < NEKO_GFXT_COLOR_MAX; ++j) {
                if (vdata->colors[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->colors[j].data;
                    vdesc.size = vdata->colors[j].size;
                    prim.stream.colors[j] = neko_render_vertex_buffer_create(vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->colors[j].data);
                    }
                }
            }

            // Joints
            for (uint32_t j = 0; j < NEKO_GFXT_JOINT_MAX; ++j) {
                if (vdata->joints[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->joints[j].data;
                    vdesc.size = vdata->joints[j].size;
                    prim.stream.joints[j] = neko_render_vertex_buffer_create(vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->joints[j].data);
                    }
                }
            }

            // Weights
            for (uint32_t j = 0; j < NEKO_GFXT_WEIGHT_MAX; ++j) {
                if (vdata->weights[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->weights[j].data;
                    vdesc.size = vdata->weights[j].size;
                    prim.stream.weights[j] = neko_render_vertex_buffer_create(vdesc);
                    if (!desc->keep_data) {
                        neko_free(vdata->weights[j].data);
                    }
                }
            }

            // Index buffer decl
            neko_render_index_buffer_desc_t idesc = NEKO_DEFAULT_VAL();
            idesc.data = vdata->indices.data;
            idesc.size = vdata->indices.size;

            // Construct index buffer for primitive
            prim.indices = neko_render_index_buffer_create(idesc);

            if (!desc->keep_data) {
                neko_free(vdata->indices.data);
            }

            // Add primitive to mesh
            neko_dyn_array_push(mesh.primitives, prim);
        }

        if (!desc->keep_data) {
            neko_dyn_array_free(m->primitives);
        }
    }

    if (!desc->keep_data) {
        neko_free(desc->meshes);
    }

    return mesh;
}

NEKO_API_DECL void neko_draw_mesh_update_or_create(neko_draw_mesh_t* mesh, const neko_draw_mesh_desc_t* desc) {
    if (!desc || !mesh) {
        return;
    }

    /*
    // Need to create mesh if not already done
    if (neko_dyn_array_empty(mesh->primitives)) {
        *mesh = neko_draw_mesh_create(desc);
        return;
    }
    */

    const uint32_t mesh_count = desc->size / sizeof(neko_draw_mesh_raw_data_t);

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i) {
        neko_draw_mesh_raw_data_t* m = &desc->meshes[i];

        for (uint32_t p = 0; p < neko_dyn_array_size(m->primitives); ++p) {
            // Get raw vertex data
            neko_draw_mesh_vertex_data_t* vdata = &m->primitives[p];

            // Construct or retrieve mesh primitive
            neko_draw_mesh_primitive_t* prim = NULL;
            if (neko_dyn_array_empty(mesh->primitives) || neko_dyn_array_size(mesh->primitives) < p) {
                neko_draw_mesh_primitive_t dprim = NEKO_DEFAULT_VAL();
                neko_dyn_array_push(mesh->primitives, dprim);
            }
            prim = &mesh->primitives[p];

            // Set prim count
            prim->count = vdata->count;

            // Positions
            if (vdata->positions.data) {
                neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                vdesc.data = vdata->positions.data;
                vdesc.size = vdata->positions.size;

                // Update
                if (prim->stream.positions.id) {
                    neko_render_vertex_buffer_update(prim->stream.positions, &vdesc);
                }
                // Create
                else {
                    prim->stream.positions = neko_render_vertex_buffer_create(vdesc);
                }
                if (!desc->keep_data) {
                    neko_free(vdata->positions.data);
                }
            }

            // Normals
            if (vdata->normals.data) {
                neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                vdesc.data = vdata->normals.data;
                vdesc.size = vdata->normals.size;

                // Update
                if (prim->stream.normals.id) {
                    neko_render_vertex_buffer_update(prim->stream.normals, &vdesc);
                } else {
                    prim->stream.normals = neko_render_vertex_buffer_create(vdesc);
                }
                if (!desc->keep_data) {
                    neko_free(vdata->normals.data);
                }
            }

            // Tangents
            if (vdata->tangents.data) {
                neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                vdesc.data = vdata->tangents.data;
                vdesc.size = vdata->tangents.size;

                if (prim->stream.tangents.id) {
                    neko_render_vertex_buffer_update(prim->stream.tangents, &vdesc);
                } else {
                    prim->stream.tangents = neko_render_vertex_buffer_create(vdesc);
                }
                if (!desc->keep_data) {
                    neko_free(vdata->tangents.data);
                }
            }

            // Texcoords
            for (uint32_t j = 0; j < NEKO_GFXT_TEX_COORD_MAX; ++j) {
                if (vdata->tex_coords[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->tex_coords[j].data;
                    vdesc.size = vdata->tex_coords[j].size;

                    if (prim->stream.tex_coords[j].id) {
                        neko_render_vertex_buffer_update(prim->stream.tex_coords[j], &vdesc);
                    } else {
                        prim->stream.tex_coords[j] = neko_render_vertex_buffer_create(vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->tex_coords[j].data);
                    }
                }
            }

            // Colors
            for (uint32_t j = 0; j < NEKO_GFXT_COLOR_MAX; ++j) {
                if (vdata->colors[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->colors[j].data;
                    vdesc.size = vdata->colors[j].size;

                    if (prim->stream.colors[j].id) {
                        neko_render_vertex_buffer_update(prim->stream.colors[j], &vdesc);
                    } else {
                        prim->stream.colors[j] = neko_render_vertex_buffer_create(vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->colors[j].data);
                    }
                }
            }

            // Joints
            for (uint32_t j = 0; j < NEKO_GFXT_JOINT_MAX; ++j) {
                if (vdata->joints[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->joints[j].data;
                    vdesc.size = vdata->joints[j].size;

                    if (prim->stream.joints[j].id) {
                        neko_render_vertex_buffer_update(prim->stream.joints[j], &vdesc);
                    } else {
                        prim->stream.joints[j] = neko_render_vertex_buffer_create(vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->joints[j].data);
                    }
                }
            }

            // Weights
            for (uint32_t j = 0; j < NEKO_GFXT_WEIGHT_MAX; ++j) {
                if (vdata->weights[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->weights[j].data;
                    vdesc.size = vdata->weights[j].size;

                    if (prim->stream.weights[j].id) {
                        neko_render_vertex_buffer_update(prim->stream.weights[j], &vdesc);
                    } else {
                        prim->stream.weights[j] = neko_render_vertex_buffer_create(vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->weights[j].data);
                    }
                }
            }

            // Custom uint
            for (uint32_t j = 0; j < NEKO_GFXT_CUSTOM_UINT_MAX; ++j) {
                if (vdata->custom_uint[j].data) {
                    neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
                    vdesc.data = vdata->custom_uint[j].data;
                    vdesc.size = vdata->custom_uint[j].size;

                    if (prim->stream.custom_uint[j].id) {
                        neko_render_vertex_buffer_update(prim->stream.custom_uint[j], &vdesc);
                    } else {
                        prim->stream.custom_uint[j] = neko_render_vertex_buffer_create(vdesc);
                    }
                    if (!desc->keep_data) {
                        neko_free(vdata->custom_uint[j].data);
                    }
                }
            }

            // Index buffer decl
            neko_render_index_buffer_desc_t idesc = NEKO_DEFAULT_VAL();
            idesc.data = vdata->indices.data;
            idesc.size = vdata->indices.size;

            // Construct index buffer for primitive
            if (prim->indices.id) {
                neko_render_index_buffer_update(prim->indices, &idesc);
            } else {
                prim->indices = neko_render_index_buffer_create(idesc);
            }

            if (!desc->keep_data) {
                neko_free(vdata->indices.data);
            }
        }

        if (!desc->keep_data) {
            neko_dyn_array_free(m->primitives);
        }
    }

    if (!desc->keep_data) {
        neko_free(desc->meshes);
    }
}

NEKO_API_DECL neko_draw_renderable_t neko_draw_renderable_create(const neko_draw_renderable_desc_t* desc) {
    neko_draw_renderable_t rend = NEKO_DEFAULT_VAL();

    if (!desc) {
        return rend;
    }

    rend.model_matrix = neko_mat4_identity();
    rend.desc = *desc;

    return rend;
}

//=== Destruction ===//
NEKO_API_DECL void neko_draw_texture_destroy(neko_draw_texture_t* texture) { neko_render_texture_destroy(*texture); }

NEKO_API_DECL void neko_draw_material_destroy(neko_draw_material_t* material) {
    // Destroy all material data
    neko_byte_buffer_free(&material->uniform_data);
    neko_byte_buffer_free(&material->image_buffer_data);
}

NEKO_API_DECL void neko_draw_mesh_destroy(neko_draw_mesh_t* mesh) {
    // Iterate through all primitives, destroy all vertex and index buffers
    for (uint32_t p = 0; p < neko_dyn_array_size(mesh->primitives); ++p) {
        neko_draw_mesh_primitive_t* prim = &mesh->primitives[p];

        // Free index buffer
        if (prim->indices.id) neko_render_index_buffer_destroy(prim->indices);

        // Free vertex stream
        if (prim->stream.positions.id) neko_render_vertex_buffer_destroy(prim->stream.positions);
        if (prim->stream.normals.id) neko_render_vertex_buffer_destroy(prim->stream.normals);
        if (prim->stream.tangents.id) neko_render_vertex_buffer_destroy(prim->stream.tangents);

        for (uint32_t i = 0; i < NEKO_GFXT_COLOR_MAX; ++i) {
            if (prim->stream.colors[i].id) neko_render_vertex_buffer_destroy(prim->stream.colors[i]);
        }

        for (uint32_t i = 0; i < NEKO_GFXT_TEX_COORD_MAX; ++i) {
            if (prim->stream.tex_coords[i].id) neko_render_vertex_buffer_destroy(prim->stream.tex_coords[i]);
        }

        for (uint32_t i = 0; i < NEKO_GFXT_JOINT_MAX; ++i) {
            if (prim->stream.joints[i].id) neko_render_vertex_buffer_destroy(prim->stream.joints[i]);
        }

        for (uint32_t i = 0; i < NEKO_GFXT_WEIGHT_MAX; ++i) {
            if (prim->stream.weights[i].id) neko_render_vertex_buffer_destroy(prim->stream.weights[i]);
        }
    }
}

NEKO_API_DECL void neko_draw_uniform_block_destroy(neko_draw_uniform_block_t* ub) {
    for (uint32_t i = 0; i < neko_dyn_array_size(ub->uniforms); ++i) {
        neko_draw_uniform_t* u = &ub->uniforms[i];
        neko_render_uniform_destroy(u->hndl);
    }

    neko_dyn_array_free(ub->uniforms);
    neko_hash_table_free(ub->lookup);
}

NEKO_API_DECL void neko_draw_pipeline_destroy(neko_draw_pipeline_t* pipeline) {
    // Destroy uniform block for pipeline
    neko_draw_uniform_block_destroy(&pipeline->ublock);

    // Free shaders (if responsible for them)
    neko_render_shader_destroy(pipeline->desc.raster.shader);

    // Destroy pipeline
    if (pipeline->desc.layout.attrs) neko_free(pipeline->desc.layout.attrs);
    if (pipeline->mesh_layout) neko_dyn_array_free(pipeline->mesh_layout);
    neko_render_pipeline_destroy(pipeline->hndl);
}

//=== Copy API ===//

NEKO_API_DECL neko_draw_material_t neko_draw_material_deep_copy(neko_draw_material_t* src) {
    neko_draw_material_t mat = neko_draw_material_create(&src->desc);
    neko_byte_buffer_copy_contents(&mat.uniform_data, &src->uniform_data);
    return mat;
}

//=== Pipeline API ===//
NEKO_API_DECL neko_draw_uniform_t* neko_draw_pipeline_get_uniform(neko_draw_pipeline_t* pip, const char* name) {
    uint64_t key = neko_hash_str64(name);
    if (!neko_hash_table_exists(pip->ublock.lookup, key)) {
        return NULL;
    }
    // Based on name, need to get uniform
    uint32_t uidx = neko_hash_table_get(pip->ublock.lookup, key);
    return &pip->ublock.uniforms[uidx];
}

//=== Material API ===//

NEKO_API_DECL
void neko_draw_material_set_uniform(neko_draw_material_t* mat, const char* name, const void* data) {
    if (!mat || !name || !data) return;

    neko_draw_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_draw_pipeline_t);
    NEKO_ASSERT(pip);

    // Get key for name lookup
    uint64_t key = neko_hash_str64(name);
    if (!neko_hash_table_exists(pip->ublock.lookup, key)) {
        NEKO_TIMED_ACTION(60, { neko_log_warning("Unable to find uniform: %s", name); });
        return;
    }

    // Based on name, need to get uniform
    uint32_t uidx = neko_hash_table_get(pip->ublock.lookup, key);
    neko_draw_uniform_t* u = &pip->ublock.uniforms[uidx];

    // Seek to beginning of data
    neko_byte_buffer_seek_to_beg(&mat->uniform_data);
    neko_byte_buffer_seek_to_beg(&mat->image_buffer_data);

    // Advance by offset
    switch (u->type) {
        case R_UNIFORM_IMAGE2D_RGBA32F:
            neko_byte_buffer_advance_position(&mat->image_buffer_data, u->offset);
            break;
        default:
            neko_byte_buffer_advance_position(&mat->uniform_data, u->offset);
            break;
    }

    switch (u->type) {
        case R_UNIFORM_FLOAT:
            neko_byte_buffer_write(&mat->uniform_data, float, *(float*)data);
            break;
        case R_UNIFORM_INT:
            neko_byte_buffer_write(&mat->uniform_data, int32_t, *(int32_t*)data);
            break;
        case R_UNIFORM_VEC2:
            neko_byte_buffer_write(&mat->uniform_data, neko_vec2, *(neko_vec2*)data);
            break;
        case R_UNIFORM_VEC3:
            neko_byte_buffer_write(&mat->uniform_data, neko_vec3, *(neko_vec3*)data);
            break;
        case R_UNIFORM_VEC4:
            neko_byte_buffer_write(&mat->uniform_data, neko_vec4, *(neko_vec4*)data);
            break;
        case R_UNIFORM_MAT4:
            neko_byte_buffer_write(&mat->uniform_data, neko_mat4, *(neko_mat4*)data);
            break;

        case R_UNIFORM_SAMPLERCUBE:
        case R_UNIFORM_SAMPLER2D:
        case R_UNIFORM_USAMPLER2D: {
            neko_byte_buffer_write(&mat->uniform_data, neko_handle(neko_render_texture_t), *(neko_handle(neko_render_texture_t)*)data);
        } break;

        case R_UNIFORM_IMAGE2D_RGBA32F: {
            neko_byte_buffer_write(&mat->image_buffer_data, neko_handle(neko_render_texture_t), *(neko_handle(neko_render_texture_t)*)data);
        } break;
    }
}

NEKO_API_DECL neko_draw_pipeline_t* neko_draw_material_get_pipeline(neko_draw_material_t* mat) {
    neko_draw_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_draw_pipeline_t);
    return pip;
}

NEKO_API_DECL
void neko_draw_material_bind(neko_command_buffer_t* cb, neko_draw_material_t* mat) {
    neko_draw_material_bind_pipeline(cb, mat);
    neko_draw_material_bind_uniforms(cb, mat);
}

NEKO_API_DECL
void neko_draw_material_bind_pipeline(neko_command_buffer_t* cb, neko_draw_material_t* mat) {
    // Binds the pipeline
    neko_draw_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_draw_pipeline_t);
    NEKO_ASSERT(pip);
    neko_render_pipeline_bind(cb, pip->hndl);
}

NEKO_API_DECL
void neko_draw_material_bind_uniforms(neko_command_buffer_t* cb, neko_draw_material_t* mat) {
    if (!mat) return;

    neko_draw_pipeline_t* pip = NEKO_GFXT_RAW_DATA(&mat->desc.pip_func, neko_draw_pipeline_t);
    NEKO_ASSERT(pip);

    // Grab uniform layout from pipeline
    for (uint32_t i = 0; i < neko_dyn_array_size(pip->ublock.uniforms); ++i) {
        neko_draw_uniform_t* u = &pip->ublock.uniforms[i];
        neko_render_bind_desc_t bind = NEKO_DEFAULT_VAL();

        // Need to buffer these up so it's a single call...
        switch (u->type) {
            case R_UNIFORM_IMAGE2D_RGBA32F: {
                neko_render_bind_image_buffer_desc_t ibuffer[1];
                ibuffer[0].tex = *(neko_handle(neko_render_texture_t)*)(mat->image_buffer_data.data + u->offset);
                ibuffer[0].binding = u->binding;
                ibuffer[0].access = R_ACCESS_WRITE_ONLY;
                bind.image_buffers.desc = ibuffer;
                bind.image_buffers.size = sizeof(ibuffer);
                neko_render_apply_bindings(cb, &bind);
            } break;

            default: {
                neko_render_bind_uniform_desc_t uniforms[1];
                uniforms[0].uniform = u->hndl;
                uniforms[0].data = (mat->uniform_data.data + u->offset);
                uniforms[0].binding = u->binding;
                bind.uniforms.desc = uniforms;
                bind.uniforms.size = sizeof(uniforms);
                neko_render_apply_bindings(cb, &bind);
            } break;
        }
    }
}

// Mesh API
NEKO_API_DECL void neko_draw_mesh_draw(neko_command_buffer_t* cb, neko_draw_mesh_t* mp) {
    /*
    // For each primitive in mesh
    for (uint32_t i = 0; i < neko_dyn_array_size(mp->primitives); ++i)
    {
        neko_draw_mesh_primitive_t* prim = &mp->primitives[i];

        // Bindings for all buffers: vertex, index, uniform, sampler
        neko_render_bind_desc_t binds = NEKO_DEFAULT_VAL();
        neko_render_bind_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
        neko_render_bind_index_buffer_desc_t idesc = NEKO_DEFAULT_VAL();
        vdesc.buffer = prim->vbo;
        idesc.buffer = prim->indices;
        binds.vertex_buffers.desc = &vdesc;
        binds.index_buffers.desc = &idesc;
        neko_render_draw_desc_t ddesc = NEKO_DEFAULT_VAL();
        ddesc.start = 0;
        ddesc.count = prim->count;
        neko_render_apply_bindings(cb, &binds);
        neko_render_draw(cb, &ddesc);
    }
    */
}

NEKO_API_DECL void neko_draw_mesh_primitive_draw_layout(neko_command_buffer_t* cb, neko_draw_mesh_primitive_t* prim, neko_draw_mesh_layout_t* layout, size_t layout_size, u32 instance_count) {
    if (!layout || !layout_size || !prim || !cb) {
        return;
    }

    neko_render_bind_vertex_buffer_desc_t vbos[8] = {0};  // Make this a define
    uint32_t l = 0;
    const uint32_t ct = layout_size / sizeof(neko_draw_mesh_layout_t);
    for (uint32_t a = 0; a < ct; ++a) {
        vbos[l].data_type = R_VERTEX_DATA_NONINTERLEAVED;
        switch (layout[a].type) {
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                if (!prim->stream.positions.id) continue;
                vbos[l].buffer = prim->stream.positions;
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                if (!prim->stream.normals.id) continue;
                vbos[l].buffer = prim->stream.normals;
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                if (!prim->stream.tangents.id) continue;
                vbos[l].buffer = prim->stream.tangents;
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT: {
                if (!prim->stream.joints[0].id) continue;
                vbos[l].buffer = prim->stream.joints[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT: {
                if (!prim->stream.weights[0].id) continue;
                vbos[l].buffer = prim->stream.weights[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                if (!prim->stream.tex_coords[0].id) continue;
                vbos[l].buffer = prim->stream.tex_coords[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                if (!prim->stream.colors[0].id) continue;
                vbos[l].buffer = prim->stream.colors[0];
            } break;
            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT: {
                if (!prim->stream.custom_uint[0].id) continue;
                vbos[l].buffer = prim->stream.custom_uint[0];
            } break;
        }
        ++l;
    }

    neko_render_bind_index_buffer_desc_t ibos = NEKO_DEFAULT_VAL();
    ibos.buffer = prim->indices;

    // Bindings for all buffers: vertex, index, uniform, sampler
    neko_render_bind_desc_t binds = NEKO_DEFAULT_VAL();

    // .vertex_buffers = {.desc = vbos, .size = sizeof(vbos)},
    binds.vertex_buffers.desc = vbos;
    binds.vertex_buffers.size = l * sizeof(neko_render_bind_vertex_buffer_desc_t);
    binds.index_buffers.desc = &ibos;

    neko_render_draw_desc_t ddesc = NEKO_DEFAULT_VAL();
    ddesc.start = 0;
    ddesc.count = prim->count;
    ddesc.instances = instance_count;

    neko_render_apply_bindings(cb, &binds);
    neko_render_draw(cb, &ddesc);
}

NEKO_API_DECL void neko_draw_mesh_draw_layout(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_mesh_layout_t* layout, size_t layout_size) {
    if (!layout || !mesh || !cb) {
        return;
    }

    uint32_t ct = layout_size / sizeof(neko_draw_mesh_layout_t);

    // For each primitive in mesh
    for (uint32_t i = 0; i < neko_dyn_array_size(mesh->primitives); ++i) {
        neko_draw_mesh_primitive_t* prim = &mesh->primitives[i];
        neko_draw_mesh_primitive_draw_layout(cb, prim, layout, layout_size, 1);
    }
}

NEKO_API_DECL void neko_draw_mesh_draw_materials(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_material_t** mats, size_t mats_size) {
    // Iterate through primitives, draw each primitive with assigned mat
    if (!mats || !mats_size || !cb || !mesh) {
        return;
    }

    const uint32_t ct = mats_size / sizeof(neko_draw_material_t*);
    neko_draw_material_t* mat = NULL;

    // For each primitive in mesh
    for (uint32_t i = 0; i < neko_dyn_array_size(mesh->primitives); ++i) {
        neko_draw_mesh_primitive_t* prim = &mesh->primitives[i];

        // Get corresponding material, if available
        uint32_t mat_idx = i < ct ? i : ct - 1;
        mat = mats[mat_idx] ? mats[mat_idx] : mat;

        // Can't draw without a valid material present
        if (!mat) continue;

        // Bind material pipeline and uniforms
        neko_draw_material_bind(cb, mat);

        // Get pipeline
        neko_draw_pipeline_t* pip = neko_draw_material_get_pipeline(mat);

        neko_draw_mesh_primitive_draw_layout(cb, prim, pip->mesh_layout, neko_dyn_array_size(pip->mesh_layout) * sizeof(neko_draw_mesh_layout_t), 1);
    }
}

NEKO_API_DECL void neko_draw_mesh_draw_material(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_material_t* mat) {
    if (!mat || !mesh || !cb) {
        return;
    }

    neko_draw_pipeline_t* pip = neko_draw_material_get_pipeline(mat);
    neko_draw_mesh_draw_layout(cb, mesh, pip->mesh_layout, neko_dyn_array_size(pip->mesh_layout) * sizeof(neko_draw_mesh_layout_t));
}

NEKO_API_DECL void neko_draw_mesh_draw_pipeline(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_pipeline_t* pip) {
    if (!pip || !mesh || !cb) {
        return;
    }

    neko_draw_mesh_draw_layout(cb, mesh, pip->mesh_layout, neko_dyn_array_size(pip->mesh_layout) * sizeof(neko_draw_mesh_layout_t));
}

// Util API
NEKO_API_DECL
void* neko_draw_raw_data_default_impl(NEKO_GFXT_HNDL hndl, void* user_data) { return hndl; }

NEKO_API_DECL void neko_draw_mesh_import_options_free(neko_draw_mesh_import_options_t* opt) {
    if (opt->layout) {
        neko_dyn_array_free(opt->layout);
    }
}

NEKO_API_DECL
neko_draw_mesh_t neko_draw_mesh_load_from_file(const char* path, neko_draw_mesh_import_options_t* options) {
    neko_draw_mesh_t mesh = NEKO_DEFAULT_VAL();

    if (!neko_platform_file_exists(path)) {
        neko_log_trace("[gfxt] Warning:GFXT:MeshLoadFromFile:File does not exist: %s", path);
        return mesh;
    }

    // Mesh data to fill out
    uint32_t mesh_count = 0;
    neko_draw_mesh_raw_data_t* meshes = NULL;

    // Get file extension from path
    neko_transient_buffer(file_ext, 32);
    neko_platform_file_extension(file_ext, 32, path);

    if (neko_string_compare_equal(file_ext, "gltf")) {  // GLTF
        // neko_draw_load_gltf_data_from_file(path, options, &meshes, &mesh_count);
        NEKO_ASSERT(false);
    } else if (neko_string_compare_equal(file_ext, "glb")) {  // GLB
        NEKO_ASSERT(false);
    } else {
        neko_log_trace("[gfxt] Warning:GFXT:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return mesh;
    }

    neko_draw_mesh_desc_t mdesc = NEKO_DEFAULT_VAL();
    mdesc.meshes = meshes;
    mdesc.size = mesh_count * sizeof(neko_draw_mesh_raw_data_t);

    mesh = neko_draw_mesh_create(&mdesc);
    mesh.desc = mdesc;

    return mesh;
}

NEKO_API_DECL
neko_draw_mesh_t neko_draw_mesh_unit_quad_generate(neko_draw_mesh_import_options_t* options) {
    neko_draw_mesh_t mesh = NEKO_DEFAULT_VAL();

    neko_vec3 v_pos[] = {
            neko_v3(-1.0f, -1.0f, 0.f),  // Top Left
            neko_v3(+1.0f, -1.0f, 0.f),  // Top Right
            neko_v3(-1.0f, +1.0f, 0.f),  // Bottom Left
            neko_v3(+1.0f, +1.0f, 0.f)   // Bottom Right
    };

    // Vertex data for quad
    neko_vec2 v_uvs[] = {
            neko_v2(0.0f, 0.0f),  // Top Left
            neko_v2(1.0f, 0.0f),  // Top Right
            neko_v2(0.0f, 1.0f),  // Bottom Left
            neko_v2(1.0f, 1.0f)   // Bottom Right
    };

    neko_vec3 v_norm[] = {neko_v3(0.f, 0.f, 1.f), neko_v3(0.f, 0.f, 1.f), neko_v3(0.f, 0.f, 1.f), neko_v3(0.f, 0.f, 1.f)};

    neko_vec3 v_tan[] = {neko_v3(1.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f), neko_v3(1.f, 0.f, 0.f)};

    neko_color_t v_color[] = {NEKO_COLOR_WHITE, NEKO_COLOR_WHITE, NEKO_COLOR_WHITE, NEKO_COLOR_WHITE};

    // Index data for quad
    uint16_t i_data[] = {
            0, 3, 2,  // First Triangle
            0, 1, 3   // Second Triangle
    };

    // Mesh data
    neko_draw_mesh_raw_data_t mesh_data = NEKO_DEFAULT_VAL();

    // Primitive to upload
    neko_draw_mesh_vertex_data_t vert_data = NEKO_DEFAULT_VAL();
    vert_data.positions.data = v_pos;
    vert_data.positions.size = sizeof(v_pos);
    vert_data.normals.data = v_norm;
    vert_data.normals.size = sizeof(v_norm);
    vert_data.tangents.data = v_tan;
    vert_data.tangents.size = sizeof(v_tan);
    vert_data.colors[0].data = v_color;
    vert_data.colors[0].size = sizeof(v_color);
    vert_data.tex_coords[0].data = v_uvs;
    vert_data.tex_coords[0].size = sizeof(v_uvs);
    vert_data.indices.data = i_data;
    vert_data.indices.size = sizeof(i_data);
    vert_data.count = 6;

    // Push into primitives
    neko_dyn_array_push(mesh_data.primitives, vert_data);

    // If no decl, then just use default layout
    /*
    neko_draw_mesh_import_options_t* moptions = options ? options : &def_options;
    uint32_t ct = moptions->size / sizeof(neko_asset_mesh_layout_t);
    */

    neko_draw_mesh_desc_t mdesc = NEKO_DEFAULT_VAL();
    mdesc.meshes = &mesh_data;
    mdesc.size = 1 * sizeof(neko_draw_mesh_raw_data_t);
    mdesc.keep_data = true;

    mesh = neko_draw_mesh_create(&mdesc);
    mesh.desc = mdesc;

    // Free data
    neko_dyn_array_free(mesh_data.primitives);

    return mesh;
}

neko_handle(neko_render_texture_t) neko_draw_texture_generate_default() {
// Generate procedural texture data (checkered texture)
#define NEKO_GFXT_ROW_COL_CT 5
    neko_color_t c0 = NEKO_COLOR_WHITE;
    neko_color_t c1 = neko_color(20, 50, 150, 255);
    neko_color_t pixels[NEKO_GFXT_ROW_COL_CT * NEKO_GFXT_ROW_COL_CT] = NEKO_DEFAULT_VAL();
    for (uint32_t r = 0; r < NEKO_GFXT_ROW_COL_CT; ++r) {
        for (uint32_t c = 0; c < NEKO_GFXT_ROW_COL_CT; ++c) {
            const bool re = (r % 2) == 0;
            const bool ce = (c % 2) == 0;
            uint32_t idx = r * NEKO_GFXT_ROW_COL_CT + c;
            pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
        }
    }

    neko_render_texture_desc_t desc = NEKO_DEFAULT_VAL();
    desc.width = NEKO_GFXT_ROW_COL_CT;
    desc.height = NEKO_GFXT_ROW_COL_CT;
    desc.format = R_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = R_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = R_TEXTURE_FILTER_NEAREST;
    desc.wrap_s = R_TEXTURE_WRAP_REPEAT;
    desc.wrap_t = R_TEXTURE_WRAP_REPEAT;
    *desc.data = pixels;

    // Create dynamic texture
    return neko_render_texture_create(desc);
}

//=== Resource Loading ===//

typedef struct tmp_buffer_t {
    char txt[1024];
} tmp_buffer_t;

typedef struct neko_shader_io_data_t {
    char type[64];
    char name[64];
} neko_shader_io_data_t;

typedef struct neko_pipeline_parse_data_t {
    neko_dyn_array(neko_shader_io_data_t) io_list[3];
    neko_dyn_array(neko_draw_mesh_layout_t) mesh_layout;
    neko_dyn_array(neko_render_vertex_attribute_type) vertex_layout;
    char* code[3];
    char dir[256];
} neko_ppd_t;

#define neko_parse_warning(TXT, ...)     \
    do {                                 \
        neko_printf("WARNING::");        \
        neko_printf(TXT, ##__VA_ARGS__); \
        neko_log_trace("[gfxt] ");       \
    } while (0)

#define neko_parse_error(TXT, ASSERT, ...) \
    do {                                   \
        neko_printf("ERROR::");            \
        neko_printf(TXT, ##__VA_ARGS__);   \
        neko_log_trace("[gfxt] ");         \
        if (ASSERT) NEKO_ASSERT(false);    \
    } while (0)

#define neko_parse_block(NAME, ...)                                                                               \
    do {                                                                                                          \
        neko_log_trace("[gfxt] neko_pipeline_load_from_file::parsing::%s", #NAME);                                \
        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {                                           \
            neko_log_trace("[gfxt] error::neko_pipeline_load_from_file::error parsing raster from .sf resource"); \
            NEKO_ASSERT(false);                                                                                   \
        }                                                                                                         \
                                                                                                                  \
        uint32_t bc = 1;                                                                                          \
        while (neko_lexer_can_lex(lex) && bc) {                                                                   \
            neko_token_t token = neko_lexer_next_token(lex);                                                      \
            switch (token.type) {                                                                                 \
                case NEKO_TOKEN_LBRACE: {                                                                         \
                    bc++;                                                                                         \
                } break;                                                                                          \
                case NEKO_TOKEN_RBRACE: {                                                                         \
                    bc--;                                                                                         \
                } break;                                                                                          \
                                                                                                                  \
                case NEKO_TOKEN_IDENTIFIER: {                                                                     \
                    __VA_ARGS__                                                                                   \
                }                                                                                                 \
            }                                                                                                     \
        }                                                                                                         \
    } while (0)

const char* neko_get_vertex_attribute_string(neko_render_vertex_attribute_type type) {
    switch (type) {
        case R_VERTEX_ATTRIBUTE_FLOAT:
            return "float";
            break;
        case R_VERTEX_ATTRIBUTE_FLOAT2:
            return "vec2";
            break;
        case R_VERTEX_ATTRIBUTE_FLOAT3:
            return "vec3";
            break;
        case R_VERTEX_ATTRIBUTE_FLOAT4:
            return "vec4";
            break;
        case R_VERTEX_ATTRIBUTE_UINT:
            return "int";
            break;
        case R_VERTEX_ATTRIBUTE_UINT2:
            return "vec2";
            break;
        case R_VERTEX_ATTRIBUTE_UINT3:
            return "vec3";
            break;
        case R_VERTEX_ATTRIBUTE_UINT4:
            return "vec4";
            break;
        case R_VERTEX_ATTRIBUTE_BYTE:
            return "float";
            break;
        case R_VERTEX_ATTRIBUTE_BYTE2:
            return "vec2";
            break;
        case R_VERTEX_ATTRIBUTE_BYTE3:
            return "vec3";
            break;
        case R_VERTEX_ATTRIBUTE_BYTE4:
            return "vec4";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}

neko_render_vertex_attribute_type neko_get_vertex_attribute_from_token(const neko_token_t* t) {
    if (neko_token_compare_text(t, "float"))
        return R_VERTEX_ATTRIBUTE_FLOAT;
    else if (neko_token_compare_text(t, "float2"))
        return R_VERTEX_ATTRIBUTE_FLOAT2;
    else if (neko_token_compare_text(t, "float3"))
        return R_VERTEX_ATTRIBUTE_FLOAT3;
    else if (neko_token_compare_text(t, "float4"))
        return R_VERTEX_ATTRIBUTE_FLOAT4;
    else if (neko_token_compare_text(t, "uint4"))
        return R_VERTEX_ATTRIBUTE_UINT4;
    else if (neko_token_compare_text(t, "uint3"))
        return R_VERTEX_ATTRIBUTE_UINT3;
    else if (neko_token_compare_text(t, "uint2"))
        return R_VERTEX_ATTRIBUTE_UINT2;
    else if (neko_token_compare_text(t, "uint"))
        return R_VERTEX_ATTRIBUTE_UINT;
    else if (neko_token_compare_text(t, "byte4"))
        return R_VERTEX_ATTRIBUTE_BYTE4;
    else if (neko_token_compare_text(t, "byte3"))
        return R_VERTEX_ATTRIBUTE_BYTE3;
    else if (neko_token_compare_text(t, "byte2"))
        return R_VERTEX_ATTRIBUTE_BYTE2;
    else if (neko_token_compare_text(t, "byte"))
        return R_VERTEX_ATTRIBUTE_BYTE;
    return (neko_render_vertex_attribute_type)0x00;
}

neko_render_uniform_type neko_uniform_type_from_token(const neko_token_t* t) {
    if (neko_token_compare_text(t, "float"))
        return R_UNIFORM_FLOAT;
    else if (neko_token_compare_text(t, "int"))
        return R_UNIFORM_INT;
    else if (neko_token_compare_text(t, "vec2"))
        return R_UNIFORM_VEC2;
    else if (neko_token_compare_text(t, "vec3"))
        return R_UNIFORM_VEC3;
    else if (neko_token_compare_text(t, "vec4"))
        return R_UNIFORM_VEC4;
    else if (neko_token_compare_text(t, "mat4"))
        return R_UNIFORM_MAT4;
    else if (neko_token_compare_text(t, "sampler2D"))
        return R_UNIFORM_SAMPLER2D;
    else if (neko_token_compare_text(t, "usampler2D"))
        return R_UNIFORM_USAMPLER2D;
    else if (neko_token_compare_text(t, "samplerCube"))
        return R_UNIFORM_SAMPLERCUBE;
    else if (neko_token_compare_text(t, "img2D_rgba32f"))
        return R_UNIFORM_IMAGE2D_RGBA32F;
    return (neko_render_uniform_type)0x00;
}

const char* neko_uniform_string_from_type(neko_render_uniform_type type) {
    switch (type) {
        case R_UNIFORM_FLOAT:
            return "float";
            break;
        case R_UNIFORM_INT:
            return "int";
            break;
        case R_UNIFORM_VEC2:
            return "vec2";
            break;
        case R_UNIFORM_VEC3:
            return "vec3";
            break;
        case R_UNIFORM_VEC4:
            return "vec4";
            break;
        case R_UNIFORM_MAT4:
            return "mat4";
            break;
        case R_UNIFORM_SAMPLER2D:
            return "sampler2D";
            break;
        case R_UNIFORM_USAMPLER2D:
            return "usampler2D";
            break;
        case R_UNIFORM_SAMPLERCUBE:
            return "samplerCube";
            break;
        case R_UNIFORM_IMAGE2D_RGBA32F:
            return "image2D";
            break;
        default:
            return "UNKNOWN";
            break;
    }
    return (char*)0x00;
}

// Make this an extern function that can be bubbled up to the app
bool neko_parse_uniform_special_keyword(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_render_shader_stage_type stage, neko_draw_uniform_desc_t* uniform) {
    neko_token_t token = lex->current_token;

    // Determine if uniform is one of special key defines
    if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX")) {
        uniform->type = R_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX, sizeof(NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX")) {
        uniform->type = R_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX, sizeof(NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_MODEL_MATRIX")) {
        uniform->type = R_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_MODEL_MATRIX, sizeof(NEKO_GFXT_UNIFORM_MODEL_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX")) {
        uniform->type = R_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX, sizeof(NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_PROJECTION_MATRIX")) {
        uniform->type = R_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_PROJECTION_MATRIX, sizeof(NEKO_GFXT_UNIFORM_PROJECTION_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_VIEW_MATRIX")) {
        uniform->type = R_UNIFORM_MAT4;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_VIEW_MATRIX, sizeof(NEKO_GFXT_UNIFORM_VIEW_MATRIX));
        return true;
    } else if (neko_token_compare_text(&token, "NEKO_GFXT_UNIFORM_TIME")) {
        uniform->type = R_UNIFORM_FLOAT;
        memcpy(uniform->name, NEKO_GFXT_UNIFORM_TIME, sizeof(NEKO_GFXT_UNIFORM_TIME));
        return true;
    }

    return false;
}

bool neko_parse_uniforms(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_render_shader_stage_type stage) {
    uint32_t image_binding = 0;

    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Unable to parsing uniforms from .sf resource");
        return false;
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                neko_draw_uniform_desc_t uniform = {0};
                uniform.stage = stage;

                bool special = neko_parse_uniform_special_keyword(lex, desc, ppd, stage, &uniform);

                // Determine if uniform is one of special key defines
                if (!special) {
                    uniform.type = neko_uniform_type_from_token(&token);
                    switch (uniform.type) {
                        default:
                            break;

                        case R_UNIFORM_SAMPLER2D:
                        case R_UNIFORM_USAMPLER2D:
                        case R_UNIFORM_IMAGE2D_RGBA32F: {
                            uniform.binding = image_binding++;
                        } break;
                    }

                    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                        neko_log_warning("Unidentified token (Expected identifier)");
                        neko_token_debug_print(&lex->current_token);
                        return false;
                    }
                    token = lex->current_token;

                    memcpy(uniform.name, token.text, token.len);
                }

                // Add uniform to ublock descriptor
                neko_dyn_array_push(desc->ublock_desc.layout, uniform);

            } break;
        }
    }
    return true;
}

bool neko_parse_io(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_render_shader_stage_type type) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Expected opening left brace. Unable to parse io from .sf resource");
        return false;
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;
            case NEKO_TOKEN_IDENTIFIER: {
                neko_shader_io_data_t io = {0};
                memcpy(io.type, token.text, token.len);

                switch (type) {
                    case R_SHADER_STAGE_VERTEX: {
                        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                            neko_log_warning("IO expected identifier name after type, shader stage vertex.");
                            neko_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        neko_dyn_array_push(ppd->io_list[0], io);
                    } break;

                    case R_SHADER_STAGE_FRAGMENT: {
                        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                            neko_log_warning("IO expected identifier name after type, shader stage fragment.");
                            neko_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        neko_dyn_array_push(ppd->io_list[1], io);
                    } break;

                    case R_SHADER_STAGE_COMPUTE: {
                        if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                            neko_log_warning("IO expected number after type, shader stage compute.");
                            neko_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        neko_dyn_array_push(ppd->io_list[2], io);
                    } break;
                }
            } break;
        }
    }
    return true;
}

bool neko_parse_code(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_render_shader_stage_type stage) {
    if (!neko_lexer_require_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Expected opening left brace");
        return false;
    }

    // Something is broken up here...
    uint32_t bc = 1;
    neko_token_t cur = neko_lexer_peek(lex);
    neko_token_t token = lex->current_token;
    while (neko_lexer_can_lex(lex) && bc) {
        token = lex->next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;
        }
    }

    // Allocate size for code
    const size_t sz = (size_t)(token.text - cur.text);
    char* code = (char*)neko_malloc(sz);
    memset(code, 0, sz);
    memcpy(code, cur.text, sz - 1);

    // List of include directories to gather
    u32 iidx = 0;
    char includes[NEKO_GFXT_INCLUDE_DIR_MAX][256] = {0};

    // Need to parse through code and replace keywords with appropriate mappings
    neko_lexer_t clex = neko_lexer_c_ctor(code);
    while (clex.can_lex(&clex)) {
        neko_token_t tkn = clex.next_token(&clex);
        switch (tkn.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_MODEL_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_MODEL_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_VIEW_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_VIEW_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_PROJECTION_MATRIX")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_PROJECTION_MATRIX, (char)32);
                } else if (neko_token_compare_text(&tkn, "NEKO_GFXT_UNIFORM_TIME")) {
                    neko_util_string_replace(tkn.text, tkn.len, NEKO_GFXT_UNIFORM_TIME, (char)32);
                }
            } break;

            case NEKO_TOKEN_HASH: {
                // Parse include
                tkn = clex.next_token(&clex);
                switch (tkn.type) {
                    case NEKO_TOKEN_IDENTIFIER: {
                        if (neko_token_compare_text(&tkn, "include") && iidx < NEKO_GFXT_INCLUDE_DIR_MAX) {
                            // Length of include string
                            size_t ilen = 8;

                            // Grab next token, expect string
                            tkn = clex.next_token(&clex);
                            if (tkn.type == NEKO_TOKEN_STRING) {
                                memcpy(includes[iidx], tkn.text + 1, tkn.len - 2);
                                neko_util_string_replace(tkn.text - ilen, tkn.len + ilen, " ", (char)32);
                                iidx++;
                            }
                        }
                    }
                }
            } break;
        }
    }

    for (uint32_t i = 0; i < NEKO_GFXT_INCLUDE_DIR_MAX; ++i) {
        if (!includes[i][0]) continue;

        // Need to collect other uniforms from these includes (parse code)
        neko_snprintfc(FINAL_PATH, 256, "%s/%s", ppd->dir, includes[i]);
        // gs_println("INC_DIR: %s", FINAL_PATH);

        // Load include using final path and relative path from include
        size_t len = 0;
        char* inc_src = neko_platform_read_file_contents(FINAL_PATH, "rb", &len);
        NEKO_ASSERT(inc_src);

        // Realloc previous code to greater size, shift contents around
        char* cat = neko_util_string_concat(inc_src, code);
        neko_safe_free(code);
        code = cat;
    }

    switch (stage) {
        case R_SHADER_STAGE_VERTEX:
            ppd->code[0] = code;
            break;
        case R_SHADER_STAGE_FRAGMENT:
            ppd->code[1] = code;
            break;
        case R_SHADER_STAGE_COMPUTE:
            ppd->code[2] = code;
            break;
    }

    return true;
}

neko_draw_mesh_attribute_type neko_mesh_attribute_type_from_token(const neko_token_t* token) {
    if (neko_token_compare_text(token, "POSITION"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION;
    else if (neko_token_compare_text(token, "NORMAL"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL;
    else if (neko_token_compare_text(token, "COLOR"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR;
    else if (neko_token_compare_text(token, "TANGENT"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT;
    else if (neko_token_compare_text(token, "TEXCOORD0"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD1"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD2"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD3"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD4"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD5"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD6"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD7"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD8"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD9"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD10"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD11"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "TEXCOORD12"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (neko_token_compare_text(token, "UINT"))
        return NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT;

    // Default
    return (neko_draw_mesh_attribute_type)0x00;
}

bool neko_parse_vertex_mesh_attributes(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        NEKO_ASSERT(false);
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        // neko_token_debug_print(&token);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                // Get attribute name
                if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                    NEKO_ASSERT(false);
                }

                neko_token_t token_name = lex->current_token;
                // neko_token_debug_print(&token_name);

#define PUSH_ATTR(MESH_ATTR, VERT_ATTR)                                  \
    do {                                                                 \
        neko_draw_mesh_layout_t layout = NEKO_DEFAULT_VAL();             \
        layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_##MESH_ATTR;        \
        neko_dyn_array_push(ppd->mesh_layout, layout);                   \
        neko_render_vertex_attribute_desc_t attr = NEKO_DEFAULT_VAL(); \
        memcpy(attr.name, token_name.text, token_name.len);              \
        attr.format = R_VERTEX_ATTRIBUTE_##VERT_ATTR;        \
        neko_dyn_array_push(desc->pip_desc.layout.attrs, attr);          \
        /*neko_log_trace("[gfxt] %s: %s", #MESH_ATTR, #VERT_ATTR);*/     \
    } while (0)

                if (neko_token_compare_text(&token, "POSITION"))
                    PUSH_ATTR(POSITION, FLOAT3);
                else if (neko_token_compare_text(&token, "NORMAL"))
                    PUSH_ATTR(NORMAL, FLOAT3);
                else if (neko_token_compare_text(&token, "COLOR"))
                    PUSH_ATTR(COLOR, BYTE4);
                else if (neko_token_compare_text(&token, "TANGENT"))
                    PUSH_ATTR(TANGENT, FLOAT3);
                else if (neko_token_compare_text(&token, "TEXCOORD"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD0"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD1"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD2"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD3"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD4"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD5"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD6"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD8"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD9"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD10"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD11"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "TEXCOORD12"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "FLOAT"))
                    PUSH_ATTR(WEIGHT, FLOAT4);
                else if (neko_token_compare_text(&token, "FLOAT2"))
                    PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (neko_token_compare_text(&token, "FLOAT3"))
                    PUSH_ATTR(POSITION, FLOAT3);
                else if (neko_token_compare_text(&token, "UINT"))
                    PUSH_ATTR(UINT, UINT);
                // else if (neko_token_compare_text(&token, "FLOAT4"))     PUSH_ATTR(TANGENT, FLOAT4);
                else {
                    neko_log_warning("Unidentified vertex attribute: %.*s: %.*s", token.len, token.text, token_name.len, token_name.text);
                    return false;
                }
            }
        }
    }
    return true;
}

bool neko_parse_vertex_attributes(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd) { return neko_parse_vertex_mesh_attributes(lex, desc, ppd); }

bool neko_parse_shader_stage(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd, neko_render_shader_stage_type stage) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_trace("[gfxt] error::neko_pipeline_load_from_file::error parsing raster from .sf resource");
        NEKO_ASSERT(false);
    }

    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                if (stage == R_SHADER_STAGE_VERTEX && neko_token_compare_text(&token, "attributes")) {
                    neko_log_trace("[gfxt] parsing attributes...");
                    if (!neko_parse_vertex_attributes(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse vertex attributes.");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "uniforms")) {
                    neko_log_trace("[gfxt] parsing uniforms...");
                    if (!neko_parse_uniforms(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'uniforms' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "out")) {
                    neko_log_trace("[gfxt] parsing out...");
                    if (!neko_parse_io(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'out' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "in")) {
                    neko_log_trace("[gfxt] parsing in...");
                    if (!neko_parse_io(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'in' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "code")) {
                    neko_log_trace("[gfxt] parsing code...");
                    if (!neko_parse_code(lex, desc, ppd, stage)) {
                        neko_log_warning("Unable to parse 'code' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }
            } break;
        }
    }
    return true;
}

bool neko_parse_compute_shader_stage(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::COMPUTE_SHADER_STAGE, {
        if (neko_token_compare_text(&token, "uniforms")) {
            if (!neko_parse_uniforms(lex, desc, ppd, R_SHADER_STAGE_COMPUTE)) {
                neko_log_warning("Unable to parse 'uniforms' for compute shader");
                return false;
            }
        }

        else if (neko_token_compare_text(&token, "in")) {
            if (!neko_parse_io(lex, desc, ppd, R_SHADER_STAGE_COMPUTE)) {
                neko_log_warning("Unable to parse 'in' for compute shader");
                return false;
            }
        }

        else if (neko_token_compare_text(&token, "code")) {
            if (!neko_parse_code(lex, desc, ppd, R_SHADER_STAGE_COMPUTE)) {
                neko_log_warning("Unable to parse 'code' for compute shader");
                return false;
            }
        }
    });
    return true;
}

bool neko_parse_shader(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_LBRACE)) {
        neko_log_warning("Unable to parse shader from .sf resource. Expected opening left brace.");
        return false;
    }

    // Braces
    uint32_t bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        neko_token_t token = lex->next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE: {
                bc++;
            } break;
            case NEKO_TOKEN_RBRACE: {
                bc--;
            } break;

            case NEKO_TOKEN_IDENTIFIER: {
                // Vertex shader
                if (neko_token_compare_text(&token, "vertex")) {
                    neko_log_trace("[gfxt] parsing vertex shader");
                    if (!neko_parse_shader_stage(lex, desc, ppd, R_SHADER_STAGE_VERTEX)) {
                        neko_log_warning("Unable to parse shader stage: Vertex");
                        return false;
                    }
                }

                // Fragment shader
                else if (neko_token_compare_text(&token, "fragment")) {
                    neko_log_trace("[gfxt] parsing fragment shader");
                    if (!neko_parse_shader_stage(lex, desc, ppd, R_SHADER_STAGE_FRAGMENT)) {
                        neko_log_warning("Unable to parse shader stage: Fragment");
                        return false;
                    }
                }

                // Compute shader
                else if (neko_token_compare_text(&token, "compute")) {
                    neko_log_trace("[gfxt] parsing compute shader");
                    if (!neko_parse_shader_stage(lex, desc, ppd, R_SHADER_STAGE_COMPUTE)) {
                        neko_log_warning("Unable to parse shader stage: Compute");
                        return false;
                    }
                }

            } break;
        }
    }
    return true;
}

bool neko_parse_depth(neko_lexer_t* lex, neko_draw_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::DEPTH, {
        // Depth function
        if (neko_token_compare_text(&token, "func")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                token = lex->current_token;
                neko_log_warning("Depth func type not found after function decl: %.*s", token.len, token.text);
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "LESS"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_LESS;
            else if (neko_token_compare_text(&token, "EQUAL"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_EQUAL;
            else if (neko_token_compare_text(&token, "LEQUAL"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_LEQUAL;
            else if (neko_token_compare_text(&token, "GREATER"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_GREATER;
            else if (neko_token_compare_text(&token, "NOTEQUAL"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_NOTEQUAL;
            else if (neko_token_compare_text(&token, "GEQUAL"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_GEQUAL;
            else if (neko_token_compare_text(&token, "ALWAYS"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_ALWAYS;
            else if (neko_token_compare_text(&token, "NEVER"))
                pdesc->pip_desc.depth.func = R_DEPTH_FUNC_NEVER;
            else {
                token = lex->current_token;
                neko_log_warning("Func type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
        if (neko_token_compare_text(&token, "mask")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                token = lex->current_token;
                neko_log_warning("Depth mask type not found after function decl: %.*s", token.len, token.text);
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ENABLED"))
                pdesc->pip_desc.depth.mask = R_DEPTH_MASK_ENABLED;
            else if (neko_token_compare_text(&token, "TRUE"))
                pdesc->pip_desc.depth.mask = R_DEPTH_MASK_ENABLED;
            else if (neko_token_compare_text(&token, "true"))
                pdesc->pip_desc.depth.mask = R_DEPTH_MASK_ENABLED;
            else if (neko_token_compare_text(&token, "DISABLED"))
                pdesc->pip_desc.depth.mask = R_DEPTH_MASK_DISABLED;
            else if (neko_token_compare_text(&token, "FALSE"))
                pdesc->pip_desc.depth.mask = R_DEPTH_MASK_DISABLED;
            else if (neko_token_compare_text(&token, "false"))
                pdesc->pip_desc.depth.mask = R_DEPTH_MASK_DISABLED;
            else {
                token = lex->current_token;
                neko_log_warning("Mask type %.*s not valid.", token.len, token.text);
                return false;
            }
            neko_log_trace("[gfxt] MASK: %zu", (uint32_t)pdesc->pip_desc.depth.mask);
        }
    });
    return true;
}

bool neko_parse_blend(neko_lexer_t* lex, neko_draw_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::BLEND, {
        // Blend function
        if (neko_token_compare_text(&token, "func")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Blend func type not found after function decl.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ADD"))
                pdesc->pip_desc.blend.func = R_BLEND_EQUATION_ADD;
            else if (neko_token_compare_text(&token, "SUBTRACT"))
                pdesc->pip_desc.blend.func = R_BLEND_EQUATION_SUBTRACT;
            else if (neko_token_compare_text(&token, "REVERSE_SUBTRACT"))
                pdesc->pip_desc.blend.func = R_BLEND_EQUATION_REVERSE_SUBTRACT;
            else if (neko_token_compare_text(&token, "MIN"))
                pdesc->pip_desc.blend.func = R_BLEND_EQUATION_MIN;
            else if (neko_token_compare_text(&token, "MAX"))
                pdesc->pip_desc.blend.func = R_BLEND_EQUATION_MAX;
            else {
                neko_log_warning("Blend func type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Source blend
        else if (neko_token_compare_text(&token, "src")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Blend src type not found after decl.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ZERO"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ZERO;
            else if (neko_token_compare_text(&token, "ONE"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE;
            else if (neko_token_compare_text(&token, "SRC_COLOR"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_SRC_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_COLOR"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE_MINUS_SRC_COLOR;
            else if (neko_token_compare_text(&token, "DST_COLOR"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_DST_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_COLOR"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE_MINUS_DST_COLOR;
            else if (neko_token_compare_text(&token, "SRC_ALPHA"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_ALPHA"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "DST_ALPHA"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_DST_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_ALPHA"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE_MINUS_DST_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_COLOR"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_CONSTANT_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_COLOR"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.src = R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else {
                neko_log_warning("Blend src type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Dest blend
        else if (neko_token_compare_text(&token, "dst")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Blend dst type not found after decl.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "ZERO"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ZERO;
            else if (neko_token_compare_text(&token, "ONE"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE;
            else if (neko_token_compare_text(&token, "SRC_COLOR"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_SRC_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_COLOR"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE_MINUS_SRC_COLOR;
            else if (neko_token_compare_text(&token, "DST_COLOR"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_DST_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_COLOR"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE_MINUS_DST_COLOR;
            else if (neko_token_compare_text(&token, "SRC_ALPHA"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_SRC_ALPHA"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
            else if (neko_token_compare_text(&token, "DST_ALPHA"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_DST_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_DST_ALPHA"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE_MINUS_DST_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_COLOR"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_CONSTANT_COLOR;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_COLOR"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_CONSTANT_ALPHA;
            else if (neko_token_compare_text(&token, "ONE_MINUS_CONSTANT_ALPHA"))
                pdesc->pip_desc.blend.dst = R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA;
            else {
                neko_log_warning("Blend dst type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
    });

    return true;
}

bool neko_parse_stencil(neko_lexer_t* lex, neko_draw_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::STENCIL, {
        // Function
        if (neko_token_compare_text(&token, "func")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil func type not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "LESS"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_LESS;
                else if (neko_token_compare_text(&token, "EQUAL"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_EQUAL;
                else if (neko_token_compare_text(&token, "LEQUAL"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_LEQUAL;
                else if (neko_token_compare_text(&token, "GREATER"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_GREATER;
                else if (neko_token_compare_text(&token, "NOTEQUAL"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_NOTEQUAL;
                else if (neko_token_compare_text(&token, "GEQUAL"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_GEQUAL;
                else if (neko_token_compare_text(&token, "ALWAYS"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_ALWAYS;
                else if (neko_token_compare_text(&token, "NEVER"))
                    pdesc->pip_desc.stencil.func = R_STENCIL_FUNC_NEVER;
                else {
                    neko_log_warning("Stencil func type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }

        }

        // Reference value
        else if (neko_token_compare_text(&token, "ref")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                neko_log_warning("Stencil reference value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;
                neko_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.ref = atoi(TMP);
            }
        }

        // Component mask
        else if (neko_token_compare_text(&token, "comp_mask")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                neko_log_warning("Stencil component mask value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;
                neko_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.comp_mask = atoi(TMP);
            }
        }

        // Write mask
        else if (neko_token_compare_text(&token, "write_mask")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_NUMBER)) {
                neko_log_warning("Stencil write mask value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;
                neko_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.write_mask = atoi(TMP);
            }
        }

        // Stencil test failure
        else if (neko_token_compare_text(&token, "sfail")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil sfail value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "KEEP"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_KEEP;
                else if (neko_token_compare_text(&token, "ZERO"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_ZERO;
                else if (neko_token_compare_text(&token, "REPLACE"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_REPLACE;
                else if (neko_token_compare_text(&token, "INCR"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_INCR;
                else if (neko_token_compare_text(&token, "INCR_WRAP"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_INCR_WRAP;
                else if (neko_token_compare_text(&token, "DECR"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_DECR;
                else if (neko_token_compare_text(&token, "DECR_WRAP"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_DECR_WRAP;
                else if (neko_token_compare_text(&token, "INVERT"))
                    pdesc->pip_desc.stencil.sfail = R_STENCIL_OP_INVERT;
                else {
                    neko_log_warning("Stencil sfail type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }
        }

        // Stencil test pass, Depth fail
        else if (neko_token_compare_text(&token, "dpfail")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil dpfail value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "KEEP"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_KEEP;
                else if (neko_token_compare_text(&token, "ZERO"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_ZERO;
                else if (neko_token_compare_text(&token, "REPLACE"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_REPLACE;
                else if (neko_token_compare_text(&token, "INCR"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_INCR;
                else if (neko_token_compare_text(&token, "INCR_WRAP"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_INCR_WRAP;
                else if (neko_token_compare_text(&token, "DECR"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_DECR;
                else if (neko_token_compare_text(&token, "DECR_WRAP"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_DECR_WRAP;
                else if (neko_token_compare_text(&token, "INVERT"))
                    pdesc->pip_desc.stencil.dpfail = R_STENCIL_OP_INVERT;
                else {
                    neko_log_warning("Stencil dpfail type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }
        }

        // Stencil test pass, Depth pass
        else if (neko_token_compare_text(&token, "dppass")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Stencil dppass value not found after decl.");
                return false;
            }

            else {
                token = lex->current_token;

                if (neko_token_compare_text(&token, "KEEP"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_KEEP;
                else if (neko_token_compare_text(&token, "ZERO"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_ZERO;
                else if (neko_token_compare_text(&token, "REPLACE"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_REPLACE;
                else if (neko_token_compare_text(&token, "INCR"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_INCR;
                else if (neko_token_compare_text(&token, "INCR_WRAP"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_INCR_WRAP;
                else if (neko_token_compare_text(&token, "DECR"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_DECR;
                else if (neko_token_compare_text(&token, "DECR_WRAP"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_DECR_WRAP;
                else if (neko_token_compare_text(&token, "INVERT"))
                    pdesc->pip_desc.stencil.dppass = R_STENCIL_OP_INVERT;
                else {
                    neko_log_warning("Stencil dppass type %.*s not valid.", token.len, token.text);
                    return false;
                }
            }
        }
    });
    return true;
}

bool neko_parse_raster(neko_lexer_t* lex, neko_draw_pipeline_desc_t* pdesc, neko_ppd_t* ppd) {
    neko_parse_block(PIPELINE::RASTER, {
        // Index Buffer Element Size
        if (neko_token_compare_text(&token, "index_buffer_element_size")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster index buffer element size not found.", token.len, token.text);
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "UINT32") || neko_token_compare_text(&token, "uint32_t") || neko_token_compare_text(&token, "u32")) {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint32_t);
            }

            else if (neko_token_compare_text(&token, "UINT16") || neko_token_compare_text(&token, "uint16_t") || neko_token_compare_text(&token, "u16")) {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint16_t);
            }

            // Default
            else {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint32_t);
            }
        }

        // Face culling
        if (neko_token_compare_text(&token, "face_culling")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster face culling type not found.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "FRONT"))
                pdesc->pip_desc.raster.face_culling = R_FACE_CULLING_FRONT;
            else if (neko_token_compare_text(&token, "BACK"))
                pdesc->pip_desc.raster.face_culling = R_FACE_CULLING_BACK;
            else if (neko_token_compare_text(&token, "FRONT_AND_BACK"))
                pdesc->pip_desc.raster.face_culling = R_FACE_CULLING_FRONT_AND_BACK;
            else {
                neko_log_warning("Raster face culling type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Winding order
        if (neko_token_compare_text(&token, "winding_order")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster winding order type not found.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "CW"))
                pdesc->pip_desc.raster.winding_order = R_WINDING_ORDER_CW;
            else if (neko_token_compare_text(&token, "CCW"))
                pdesc->pip_desc.raster.winding_order = R_WINDING_ORDER_CCW;
            else {
                neko_log_warning("Raster winding order type %.*s not valid.", token.len, token.text);
                return false;
            }
        }

        // Primtive
        if (neko_token_compare_text(&token, "primitive")) {
            if (!neko_lexer_find_next_token_type(lex, NEKO_TOKEN_IDENTIFIER)) {
                neko_log_warning("Raster primitive type not found.");
                return false;
            }

            token = lex->current_token;

            if (neko_token_compare_text(&token, "LINES"))
                pdesc->pip_desc.raster.primitive = R_PRIMITIVE_LINES;
            else if (neko_token_compare_text(&token, "TRIANGLES"))
                pdesc->pip_desc.raster.primitive = R_PRIMITIVE_TRIANGLES;
            else if (neko_token_compare_text(&token, "QUADS"))
                pdesc->pip_desc.raster.primitive = R_PRIMITIVE_QUADS;
            else {
                neko_log_warning("Raster primitive type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
    });
    return true;
}

bool neko_parse_pipeline(neko_lexer_t* lex, neko_draw_pipeline_desc_t* desc, neko_ppd_t* ppd) {
    // Get next identifier
    while (lex->can_lex(lex)) {
        neko_token_t token = lex->next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "shader")) {
                    neko_log_trace("[gfxt] parsing shader");
                    if (!neko_parse_shader(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse shader descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "raster")) {
                    if (!neko_parse_raster(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse raster descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "depth")) {
                    if (!neko_parse_depth(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse depth descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "stencil")) {
                    if (!neko_parse_stencil(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse stencil descriptor");
                        return false;
                    }
                }

                else if (neko_token_compare_text(&token, "blend")) {
                    if (!neko_parse_blend(lex, desc, ppd)) {
                        neko_log_warning("Unable to parse blend descriptor");
                        return false;
                    }
                }

            } break;
        }
    }
    return true;
}

char* neko_pipeline_generate_shader_code(neko_draw_pipeline_desc_t* pdesc, neko_ppd_t* ppd, neko_render_shader_stage_type stage) {
    neko_log_trace("[gfxt] GENERATING CODE...");

    // Get major/minor version of shader
    neko_render_info_t* ginfo = neko_render_info();
    neko_snprintfc(MAJMINSTR, 128, "#version %u%u0\n", ginfo->major_version, ginfo->minor_version);

// Shaders
#ifdef NEKO_PLATFORM_WEB
#define _NEKO_VERSION_STR "#version 300 es\n"
#else
#define _NEKO_VERSION_STR MAJMINSTR  // should be "#version 430\n" or newer
#endif

    // Source code
    char* src = NULL;
    uint32_t sidx = 0;

    // Set sidx
    switch (stage) {
        case R_SHADER_STAGE_VERTEX:
            sidx = 0;
            break;
        case R_SHADER_STAGE_FRAGMENT:
            sidx = 1;
            break;
        case R_SHADER_STAGE_COMPUTE:
            sidx = 2;
            break;
    }

    // Early out for now...
    if (!ppd->code[sidx]) {
        return src;
    }

    // Shader header
    neko_snprintfc(shader_header, 512, "%s precision mediump float;\n", stage == R_SHADER_STAGE_COMPUTE ? "#version 430\n" : _NEKO_VERSION_STR);

    // Generate shader code
    if (ppd->code[sidx]) {
        const size_t header_sz = (size_t)neko_string_length(shader_header);
        size_t total_sz = neko_string_length(ppd->code[sidx]) + header_sz + 2048;
        src = (char*)neko_malloc(total_sz);
        memset(src, 0, total_sz);
        strncat(src, shader_header, header_sz);

        // Attributes
        if (stage == R_SHADER_STAGE_VERTEX) {
            for (uint32_t i = 0; i < neko_dyn_array_size(pdesc->pip_desc.layout.attrs); ++i) {
                const char* aname = pdesc->pip_desc.layout.attrs[i].name;
                const char* atype = neko_get_vertex_attribute_string(pdesc->pip_desc.layout.attrs[i].format);

                neko_snprintfc(ATTR, 64, "layout(location = %zu) in %s %s;\n", i, atype, aname);
                const size_t sz = neko_string_length(ATTR);
                strncat(src, ATTR, sz);
            }
        }

        // Compute shader image buffer binding
        uint32_t img_binding = 0;

        // Uniforms
        for (uint32_t i = 0; i < neko_dyn_array_size(pdesc->ublock_desc.layout); ++i) {
            neko_draw_uniform_desc_t* udesc = &pdesc->ublock_desc.layout[i];

            if (udesc->stage != stage) continue;

            switch (stage) {
                case R_SHADER_STAGE_COMPUTE: {
                    // Need to go from uniform type to string
                    const char* utype = neko_uniform_string_from_type(udesc->type);
                    const char* uname = udesc->name;

                    switch (udesc->type) {
                        default: {
                            neko_snprintfc(TMP, 64, "uniform %s %s;\n", utype, uname);
                            const size_t sz = neko_string_length(TMP);
                            strncat(src, TMP, sz);
                        } break;

                        case R_UNIFORM_IMAGE2D_RGBA32F: {
                            neko_snprintfc(TMP, 64, "layout (rgba32f, binding = %zu) uniform image2D %s;\n", img_binding++, uname);
                            const size_t sz = neko_string_length(TMP);
                            strncat(src, TMP, sz);
                        } break;
                    }
                } break;

                default: {
                    // Need to go from uniform type to string
                    const char* utype = neko_uniform_string_from_type(udesc->type);
                    const char* uname = udesc->name;
                    neko_snprintfc(TMP, 64, "uniform %s %s;\n", utype, uname);
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                } break;
            }
        }

        // Out
        switch (stage) {
            case R_SHADER_STAGE_FRAGMENT:
            case R_SHADER_STAGE_VERTEX: {
                for (uint32_t i = 0; i < neko_dyn_array_size(ppd->io_list[sidx]); ++i) {
                    neko_shader_io_data_t* out = &ppd->io_list[sidx][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    neko_transient_buffer(TMP, 64);
                    if (stage == R_SHADER_STAGE_FRAGMENT) {
                        neko_snprintf(TMP, 64, "layout(location = %zu) out %s %s;\n", i, otype, oname);
                    } else {
                        neko_snprintf(TMP, 64, "out %s %s;\n", otype, oname);
                    }
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                }
            } break;

            default:
                break;
        }

        // In
        switch (stage) {
            case R_SHADER_STAGE_FRAGMENT: {
                for (uint32_t i = 0; i < neko_dyn_array_size(ppd->io_list[0]); ++i) {
                    neko_shader_io_data_t* out = &ppd->io_list[0][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    neko_snprintfc(TMP, 64, "in %s %s;\n", otype, oname);
                    const size_t sz = neko_string_length(TMP);
                    strncat(src, TMP, sz);
                }
            } break;

                /*
                case R_SHADER_STAGE_COMPUTE: {
                    neko_snprintfc(TMP, 64, "layout(");
                    strncat(src, "layout(", 7);

                    for (uint32_t i = 0; i < neko_dyn_array_size(ppd->io_list[2]); ++i) {
                        neko_shader_io_data_t* out = &ppd->io_list[2][i];
                        const char* otype = out->type;
                        const char* oname = out->name;
                        neko_snprintfc(TMP, 64, "%s = %s%s", otype, oname, i == neko_dyn_array_size(ppd->io_list[2]) - 1 ? "" : ", ");
                        const size_t sz = neko_string_length(TMP);
                        strncat(src, TMP, sz);
                    }

                    strncat(src, ") in;\n", 7);
                } break;
                */

            default:
                break;
        }

        // Code
        {
            const size_t sz = neko_string_length(ppd->code[sidx]);
            strncat(src, ppd->code[sidx], sz);
        }
    }

    return src;
}

NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_load_from_file(const char* path) {
    // Load file, generate lexer off of file data, parse contents for pipeline information
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(path, "rb", &len);
    NEKO_ASSERT(file_data);
    neko_log_trace("Parsing pipeline: %s", path);
    neko_draw_pipeline_t pip = neko_draw_pipeline_load_from_memory_ext(file_data, len, path);
    neko_safe_free(file_data);
    return pip;
}

NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_load_from_memory(const char* file_data, size_t sz) { return neko_draw_pipeline_load_from_memory_ext(file_data, sz, "."); }

NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_load_from_memory_ext(const char* file_data, size_t sz, const char* file_path) {
    // Cast to pip
    neko_draw_pipeline_t pip = NEKO_DEFAULT_VAL();

    neko_ppd_t ppd = NEKO_DEFAULT_VAL();
    neko_draw_pipeline_desc_t pdesc = NEKO_DEFAULT_VAL();
    pdesc.pip_desc.raster.index_buffer_element_size = sizeof(uint32_t);

    // Determine original file directory from path
    if (file_path) {
        neko_lexer_t lex = neko_lexer_c_ctor(file_path);
        neko_token_t tparen = {0};
        while (lex.can_lex(&lex)) {
            neko_token_t token = lex.next_token(&lex);

            // Look for last paren + identifier combo
            switch (token.type) {
                case NEKO_TOKEN_FSLASH:
                case NEKO_TOKEN_BSLASH: {
                    tparen = token;
                } break;
            }
        }
        // Now save dir
        neko_log_trace("[gfxt] HERE: %zu", tparen.text - file_path);
        memcpy(ppd.dir, file_path, tparen.text - file_path);
        neko_log_trace("[gfxt] PPD_DIR: %s", ppd.dir);
    }

    neko_lexer_t lex = neko_lexer_c_ctor(file_data);
    while (lex.can_lex(&lex)) {
        neko_token_t token = lex.next_token(&lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "pipeline")) {
                    if (!neko_parse_pipeline(&lex, &pdesc, &ppd)) {
                        neko_log_warning("Unable to parse pipeline");
                        return pip;
                    }
                }
            } break;
        }
    }

    // Generate vertex shader code
    char* v_src = neko_pipeline_generate_shader_code(&pdesc, &ppd, R_SHADER_STAGE_VERTEX);
    // neko_log_trace("[gfxt] %s", v_src);

    // Generate fragment shader code
    char* f_src = neko_pipeline_generate_shader_code(&pdesc, &ppd, R_SHADER_STAGE_FRAGMENT);
    // neko_log_trace("[gfxt] %s", f_src);

    // Generate compute shader code (need to check for this first)
    char* c_src = neko_pipeline_generate_shader_code(&pdesc, &ppd, R_SHADER_STAGE_COMPUTE);
    // neko_log_trace("[gfxt] %s", c_src);

    // Construct compute shader
    if (c_src) {
        neko_render_shader_desc_t sdesc = NEKO_DEFAULT_VAL();
        neko_render_shader_source_desc_t source_desc[1] = NEKO_DEFAULT_VAL();
        source_desc[0].type = R_SHADER_STAGE_COMPUTE;
        source_desc[0].source = c_src;
        sdesc.sources = source_desc;
        sdesc.size = 1 * sizeof(neko_render_shader_source_desc_t);

        pdesc.pip_desc.compute.shader = neko_render_shader_create(sdesc);
    }
    // Construct raster shader
    else {
        neko_render_shader_desc_t sdesc = NEKO_DEFAULT_VAL();
        neko_render_shader_source_desc_t source_desc[2] = NEKO_DEFAULT_VAL();
        source_desc[0].type = R_SHADER_STAGE_VERTEX;
        source_desc[0].source = v_src;
        source_desc[1].type = R_SHADER_STAGE_FRAGMENT;
        source_desc[1].source = f_src;
        sdesc.sources = source_desc;
        sdesc.size = 2 * sizeof(neko_render_shader_source_desc_t);

        pdesc.pip_desc.raster.shader = neko_render_shader_create(sdesc);
    }

    // Set up layout
    pdesc.pip_desc.layout.size = neko_dyn_array_size(pdesc.pip_desc.layout.attrs) * sizeof(neko_render_vertex_attribute_desc_t);

    // Set up ublock
    pdesc.ublock_desc.size = neko_dyn_array_size(pdesc.ublock_desc.layout) * sizeof(neko_draw_uniform_desc_t);

    // Create pipeline
    pip = neko_draw_pipeline_create(&pdesc);

    // Create mesh layout
    if (ppd.mesh_layout) {
        for (uint32_t i = 0; i < neko_dyn_array_size(ppd.mesh_layout); ++i) {
            neko_dyn_array_push(pip.mesh_layout, ppd.mesh_layout[i]);
        }
    }

    // Free all malloc'd data
    if (v_src) neko_free(v_src);
    if (f_src) neko_free(f_src);
    if (c_src) neko_free(c_src);
    neko_dyn_array_free(pdesc.ublock_desc.layout);
    neko_dyn_array_free(pdesc.pip_desc.layout.attrs);
    neko_dyn_array_free(ppd.mesh_layout);
    neko_dyn_array_free(ppd.vertex_layout);

    for (uint32_t i = 0; i < 3; ++i) {
        if (ppd.code[i]) neko_free(ppd.code[i]);
        neko_dyn_array_free(ppd.io_list[i]);
    }

    return pip;
}

NEKO_API_DECL neko_draw_texture_t neko_draw_texture_load_from_file(const char* path, neko_render_texture_desc_t* desc, bool flip, bool keep_data) {
    neko_asset_texture_t tex = NEKO_DEFAULT_VAL();
    neko_asset_texture_load_from_file(path, &tex, desc, flip, keep_data);
    if (desc) {
        *desc = tex.desc;
    }
    return tex.hndl;
}

NEKO_API_DECL neko_draw_texture_t neko_draw_texture_load_from_memory(const char* data, size_t sz, neko_render_texture_desc_t* desc, bool flip, bool keep_data) {
    neko_asset_texture_t tex = NEKO_DEFAULT_VAL();
    neko_asset_texture_load_from_memory(data, sz, &tex, desc, flip, keep_data);
    if (desc) {
        *desc = tex.desc;
    }
    return tex.hndl;
}

#include <stdbool.h>

bool sprite_batch_internal_use_scratch_buffer(neko_spritebatch_t* sb) { return sb->sprites_sorter_callback == 0; }

int neko_spritebatch_init(neko_spritebatch_t* sb, neko_spritebatch_config_t* config, void* udata) {
    // read config params
    if (!config | !sb) return 1;
    sb->pixel_stride = config->pixel_stride;
    sb->atlas_width_in_pixels = config->atlas_width_in_pixels;
    sb->atlas_height_in_pixels = config->atlas_height_in_pixels;
    sb->atlas_use_border_pixels = config->atlas_use_border_pixels;
    sb->ticks_to_decay_texture = config->ticks_to_decay_texture;
    sb->lonely_buffer_count_till_flush = config->lonely_buffer_count_till_flush;
    sb->lonely_buffer_count_till_decay = sb->lonely_buffer_count_till_flush / 2;
    if (sb->lonely_buffer_count_till_decay <= 0) sb->lonely_buffer_count_till_decay = 1;
    sb->ratio_to_decay_atlas = config->ratio_to_decay_atlas;
    sb->ratio_to_merge_atlases = config->ratio_to_merge_atlases;
    sb->batch_callback = config->batch_callback;
    sb->get_pixels_callback = config->get_pixels_callback;
    sb->generate_texture_callback = config->generate_texture_callback;
    sb->delete_texture_callback = config->delete_texture_callback;
    sb->sprites_sorter_callback = config->sprites_sorter_callback;
    sb->udata = udata;

    if (sb->atlas_width_in_pixels < 1 || sb->atlas_height_in_pixels < 1) return 1;
    if (sb->ticks_to_decay_texture < 1) return 1;
    if (sb->ratio_to_decay_atlas < 0 || sb->ratio_to_decay_atlas > 1.0f) return 1;
    if (sb->ratio_to_merge_atlases < 0 || sb->ratio_to_merge_atlases > 0.5f) return 1;
    if (!sb->batch_callback) return 1;
    if (!sb->get_pixels_callback) return 1;
    if (!sb->generate_texture_callback) return 1;
    if (!sb->delete_texture_callback) return 1;

    // initialize input buffer
    sb->input_count = 0;
    sb->input_capacity = 1024;
    sb->input_buffer = (neko_spritebatch_internal_sprite_t*)malloc(sizeof(neko_spritebatch_internal_sprite_t) * sb->input_capacity);
    if (!sb->input_buffer) return 1;

    // initialize sprite buffer
    sb->sprite_count = 0;
    sb->sprite_capacity = 1024;
    sb->sprites = (neko_spritebatch_sprite_t*)malloc(sizeof(neko_spritebatch_sprite_t) * sb->sprite_capacity);

    sb->sprites_scratch = 0;
    if (sprite_batch_internal_use_scratch_buffer(sb)) {
        sb->sprites_scratch = (neko_spritebatch_sprite_t*)malloc(sizeof(neko_spritebatch_sprite_t) * sb->sprite_capacity);
    }
    if (!sb->sprites) return 1;

    if (sprite_batch_internal_use_scratch_buffer(sb)) {
        if (!sb->sprites_scratch) return 1;
    }

    // initialize key buffer (for marking hash table entries for deletion)
    sb->key_buffer_count = 0;
    sb->key_buffer_capacity = 1024;
    sb->key_buffer = (u64*)malloc(sizeof(u64) * sb->key_buffer_capacity);

    // initialize pixel buffer for grabbing pixel data from the user as needed
    sb->pixel_buffer_size = 1024;
    sb->pixel_buffer = malloc(sb->pixel_buffer_size * sb->pixel_stride);

    // setup tables
    hashtable_init(&sb->sprites_to_lonely_textures, sizeof(neko_spritebatch_internal_lonely_texture_t), 1024, NULL);
    hashtable_init(&sb->sprites_to_premade_textures, sizeof(neko_spritebatch_internal_premade_atlas), 16, NULL);
    hashtable_init(&sb->sprites_to_atlases, sizeof(neko_spritebatch_internal_atlas_t*), 16, NULL);

    sb->atlases = 0;

    return 0;
}

void neko_spritebatch_term(neko_spritebatch_t* sb) {
    free(sb->input_buffer);
    free(sb->sprites);
    if (sb->sprites_scratch) {
        free(sb->sprites_scratch);
    }
    free(sb->key_buffer);
    free(sb->pixel_buffer);
    hashtable_term(&sb->sprites_to_lonely_textures);
    hashtable_term(&sb->sprites_to_premade_textures);
    hashtable_term(&sb->sprites_to_atlases);

    if (sb->atlases) {
        neko_spritebatch_internal_atlas_t* atlas = sb->atlases;
        neko_spritebatch_internal_atlas_t* sentinel = sb->atlases;
        do {
            hashtable_term(&atlas->sprites_to_textures);
            neko_spritebatch_internal_atlas_t* next = atlas->next;
            free(atlas);
            atlas = next;
        } while (atlas != sentinel);
    }

    memset(sb, 0, sizeof(neko_spritebatch_t));
}

void neko_spritebatch_reset_function_ptrs(neko_spritebatch_t* sb, submit_batch_fn* batch_callback, get_pixels_fn* get_pixels_callback, generate_texture_handle_fn* generate_texture_callback,
                                          destroy_texture_handle_fn* delete_texture_callback, sprites_sorter_fn* sprites_sorter_callback) {
    sb->batch_callback = batch_callback;
    sb->get_pixels_callback = get_pixels_callback;
    sb->generate_texture_callback = generate_texture_callback;
    sb->delete_texture_callback = delete_texture_callback;
    sb->sprites_sorter_callback = sprites_sorter_callback;
}

void neko_spritebatch_set_default_config(neko_spritebatch_config_t* config) {
    config->pixel_stride = sizeof(char) * 4;
    config->atlas_width_in_pixels = 1024;
    config->atlas_height_in_pixels = 1024;
    config->atlas_use_border_pixels = 0;
    config->ticks_to_decay_texture = 60 * 30;
    config->lonely_buffer_count_till_flush = 64;
    config->ratio_to_decay_atlas = 0.5f;
    config->ratio_to_merge_atlases = 0.25f;
    config->batch_callback = 0;
    config->generate_texture_callback = 0;
    config->delete_texture_callback = 0;
    config->sprites_sorter_callback = 0;
}

#define SPRITEBATCH_CHECK_BUFFER_GROW(ctx, count, capacity, data, type) \
    do {                                                                \
        if (ctx->count == ctx->capacity) {                              \
            int new_capacity = ctx->capacity * 2;                       \
            void* new_data = malloc(sizeof(type) * new_capacity);       \
            if (!new_data) return 0;                                    \
            memcpy(new_data, ctx->data, sizeof(type) * ctx->count);     \
            free(ctx->data);                                            \
            ctx->data = (type*)new_data;                                \
            ctx->capacity = new_capacity;                               \
        }                                                               \
    } while (0)

int neko_spritebatch_internal_fill_internal_sprite(neko_spritebatch_t* sb, neko_spritebatch_sprite_t sprite, neko_spritebatch_internal_sprite_t* out) {
    NEKO_ASSERT(sprite.w <= sb->atlas_width_in_pixels);
    NEKO_ASSERT(sprite.h <= sb->atlas_height_in_pixels);
    SPRITEBATCH_CHECK_BUFFER_GROW(sb, input_count, input_capacity, input_buffer, neko_spritebatch_internal_sprite_t);

    out->image_id = sprite.image_id;
    out->sort_bits = sprite.sort_bits;
    out->w = sprite.w;
    out->h = sprite.h;
    out->x = sprite.x;
    out->y = sprite.y;
    out->sx = sprite.sx + (sb->atlas_use_border_pixels ? (sprite.sx / (float)sprite.w) * 2.0f : 0);
    out->sy = sprite.sy + (sb->atlas_use_border_pixels ? (sprite.sy / (float)sprite.h) * 2.0f : 0);
    out->c = sprite.c;
    out->s = sprite.s;

    out->premade_minx = sprite.minx;
    out->premade_miny = sprite.miny;
    out->premade_maxx = sprite.maxx;
    out->premade_maxy = sprite.maxy;

#ifdef SPRITEBATCH_SPRITE_USERDATA
    out->udata = sprite.udata;
#endif

    return 1;
}

void neko_spritebatch_internal_append_sprite(neko_spritebatch_t* sb, neko_spritebatch_internal_sprite_t sprite) { sb->input_buffer[sb->input_count++] = sprite; }

int neko_spritebatch_push(neko_spritebatch_t* sb, neko_spritebatch_sprite_t sprite) {
    neko_spritebatch_internal_sprite_t sprite_out;

    neko_spritebatch_internal_fill_internal_sprite(sb, sprite, &sprite_out);

    sb->input_buffer[sb->input_count++] = sprite_out;
    return 1;
}

void neko_spritebatch_register_premade_atlas(neko_spritebatch_t* sb, u64 image_id, int w, int h) {
    neko_spritebatch_internal_premade_atlas info;
    info.w = w;
    info.h = h;
    info.image_id = image_id;
    info.texture_id = ~0ULL;
    info.mark_for_cleanup = false;

    hashtable_insert(&sb->sprites_to_premade_textures, image_id, &info);
}

void neko_spritebatch_cleanup_premade_atlas(neko_spritebatch_t* sb, u64 image_id) {
    neko_spritebatch_internal_premade_atlas* tex = (neko_spritebatch_internal_premade_atlas*)hashtable_find(&sb->sprites_to_premade_textures, image_id);
    if (tex) tex->mark_for_cleanup = true;
}

int neko_spritebatch_internal_lonely_sprite(neko_spritebatch_t* sb, u64 image_id, int w, int h, neko_spritebatch_sprite_t* sprite_out, int skip_missing_textures);
neko_spritebatch_internal_premade_atlas* neko_spritebatch_internal_premade_sprite(neko_spritebatch_t* sb, u64 image_id, neko_spritebatch_sprite_t* sprite_out, int skip_missing_textures);

void neko_spritebatch_prefetch(neko_spritebatch_t* sb, u64 image_id, int w, int h) {
    neko_spritebatch_internal_premade_atlas* premade_atlas = neko_spritebatch_internal_premade_sprite(sb, image_id, NULL, 0);
    if (!premade_atlas) {
        void* atlas_ptr = hashtable_find(&sb->sprites_to_atlases, image_id);
        if (!atlas_ptr) neko_spritebatch_internal_lonely_sprite(sb, image_id, w, h, NULL, 0);
    }
}

neko_spritebatch_sprite_t neko_spritebatch_fetch(neko_spritebatch_t* sb, u64 image_id, int w, int h) {
    neko_spritebatch_sprite_t s;
    memset(&s, 0, sizeof(s));

    s.w = w;
    s.h = h;
    s.c = 1;
    s.s = 0;

    neko_spritebatch_internal_premade_atlas* tex = (neko_spritebatch_internal_premade_atlas*)hashtable_find(&sb->sprites_to_premade_textures, image_id);
    if (!tex) {
        void* atlas_ptr = hashtable_find(&sb->sprites_to_atlases, image_id);
        if (atlas_ptr) {
            neko_spritebatch_internal_atlas_t* atlas = *(neko_spritebatch_internal_atlas_t**)atlas_ptr;
            s.texture_id = atlas->texture_id;

            neko_spritebatch_internal_texture_t* tex = (neko_spritebatch_internal_texture_t*)hashtable_find(&atlas->sprites_to_textures, image_id);
            if (tex) {
                s.maxx = tex->maxx;
                s.maxy = tex->maxy;
                s.minx = tex->minx;
                s.miny = tex->miny;
            }
        } else {
            neko_spritebatch_internal_lonely_sprite(sb, image_id, w, h, &s, 0);
        }
    } else {
        neko_spritebatch_internal_premade_sprite(sb, image_id, &s, 0);
    }
    return s;
}

static int neko_spritebatch_internal_sprite_less_than_or_equal(neko_spritebatch_sprite_t* a, neko_spritebatch_sprite_t* b) {
    if (a->sort_bits < b->sort_bits) return 1;
    if (a->sort_bits == b->sort_bits && a->texture_id <= b->texture_id) return 1;
    return 0;
}

void neko_spritebatch_internal_merge_sort_iteration(neko_spritebatch_sprite_t* a, int lo, int split, int hi, neko_spritebatch_sprite_t* b) {
    int i = lo, j = split;
    for (int k = lo; k < hi; k++) {
        if (i < split && (j >= hi || neko_spritebatch_internal_sprite_less_than_or_equal(a + i, a + j))) {
            b[k] = a[i];
            i = i + 1;
        } else {
            b[k] = a[j];
            j = j + 1;
        }
    }
}

void neko_spritebatch_internal_merge_sort_recurse(neko_spritebatch_sprite_t* b, int lo, int hi, neko_spritebatch_sprite_t* a) {
    if (hi - lo <= 1) return;
    int split = (lo + hi) / 2;
    neko_spritebatch_internal_merge_sort_recurse(a, lo, split, b);
    neko_spritebatch_internal_merge_sort_recurse(a, split, hi, b);
    neko_spritebatch_internal_merge_sort_iteration(b, lo, split, hi, a);
}

void neko_spritebatch_internal_merge_sort(neko_spritebatch_sprite_t* a, neko_spritebatch_sprite_t* b, int n) {
    memcpy(b, a, sizeof(neko_spritebatch_sprite_t) * n);
    neko_spritebatch_internal_merge_sort_recurse(b, 0, n, a);
}

void neko_spritebatch_internal_sort_sprites(neko_spritebatch_t* sb) {
    if (sb->sprites_sorter_callback)
        sb->sprites_sorter_callback(sb->sprites, sb->sprite_count);
    else
        neko_spritebatch_internal_merge_sort(sb->sprites, sb->sprites_scratch, sb->sprite_count);
}

static inline void neko_spritebatch_internal_get_pixels(neko_spritebatch_t* sb, u64 image_id, int w, int h) {
    int size = sb->atlas_use_border_pixels ? sb->pixel_stride * (w + 2) * (h + 2) : sb->pixel_stride * w * h;
    if (size > sb->pixel_buffer_size) {
        free(sb->pixel_buffer);
        sb->pixel_buffer_size = size;
        sb->pixel_buffer = malloc(sb->pixel_buffer_size);
        if (!sb->pixel_buffer) return;
    }

    memset(sb->pixel_buffer, 0, size);
    int size_from_user = sb->pixel_stride * w * h;
    sb->get_pixels_callback(image_id, sb->pixel_buffer, size_from_user, sb->udata);

    if (sb->atlas_use_border_pixels) {
        // Expand image from top-left corner, offset by (1, 1).
        int w0 = w;
        int h0 = h;
        w += 2;
        h += 2;
        char* buffer = (char*)sb->pixel_buffer;
        int dst_row_stride = w * sb->pixel_stride;
        int src_row_stride = w0 * sb->pixel_stride;
        int src_row_offset = sb->pixel_stride;
        for (int i = 0; i < h - 2; ++i) {
            char* src_row = buffer + (h0 - i - 1) * src_row_stride;
            char* dst_row = buffer + (h - i - 2) * dst_row_stride + src_row_offset;
            memmove(dst_row, src_row, src_row_stride);
        }

        // Clear the border pixels.
        int pixel_stride = sb->pixel_stride;
        memset(buffer, 0, dst_row_stride);
        for (int i = 1; i < h - 1; ++i) {
            memset(buffer + i * dst_row_stride, 0, pixel_stride);
            memset(buffer + i * dst_row_stride + src_row_stride + src_row_offset, 0, pixel_stride);
        }
        memset(buffer + (h - 1) * dst_row_stride, 0, dst_row_stride);
    }
}

static inline u64 neko_spritebatch_internal_generate_texture_handle(neko_spritebatch_t* sb, u64 image_id, int w, int h) {
    neko_spritebatch_internal_get_pixels(sb, image_id, w, h);
    if (sb->atlas_use_border_pixels) {
        w += 2;
        h += 2;
    }
    return sb->generate_texture_callback(sb->pixel_buffer, w, h, sb->udata);
}

neko_spritebatch_internal_lonely_texture_t* neko_spritebatch_internal_lonelybuffer_push(neko_spritebatch_t* sb, u64 image_id, int w, int h, int make_tex) {
    neko_spritebatch_internal_lonely_texture_t texture;
    texture.timestamp = 0;
    texture.w = w;
    texture.h = h;
    texture.image_id = image_id;
    texture.texture_id = make_tex ? neko_spritebatch_internal_generate_texture_handle(sb, image_id, w, h) : ~0;
    return (neko_spritebatch_internal_lonely_texture_t*)hashtable_insert(&sb->sprites_to_lonely_textures, image_id, &texture);
}

neko_spritebatch_internal_premade_atlas* neko_spritebatch_internal_premadebuffer_push(neko_spritebatch_t* sb, u64 image_id, int w, int h, int make_tex) {
    neko_spritebatch_internal_premade_atlas texture;
    texture.w = w;
    texture.h = h;
    texture.image_id = image_id;
    texture.texture_id = make_tex ? neko_spritebatch_internal_generate_texture_handle(sb, image_id, w, h) : ~0;
    return (neko_spritebatch_internal_premade_atlas*)hashtable_insert(&sb->sprites_to_premade_textures, image_id, &texture);
}

int neko_spritebatch_internal_lonely_sprite(neko_spritebatch_t* sb, u64 image_id, int w, int h, neko_spritebatch_sprite_t* sprite_out, int skip_missing_textures) {
    neko_spritebatch_internal_lonely_texture_t* tex = (neko_spritebatch_internal_lonely_texture_t*)hashtable_find(&sb->sprites_to_lonely_textures, image_id);

    if (skip_missing_textures) {
        if (!tex) neko_spritebatch_internal_lonelybuffer_push(sb, image_id, w, h, 0);
        return 1;
    }

    else {
        if (!tex)
            tex = neko_spritebatch_internal_lonelybuffer_push(sb, image_id, w, h, 1);
        else if (tex->texture_id == ~0)
            tex->texture_id = neko_spritebatch_internal_generate_texture_handle(sb, image_id, w, h);
        tex->timestamp = 0;

        if (sprite_out) {
            sprite_out->texture_id = tex->texture_id;
            sprite_out->minx = sprite_out->miny = 0;
            sprite_out->maxx = sprite_out->maxy = 1.0f;

            if (SPRITEBATCH_LONELY_FLIP_Y_AXIS_FOR_UV) {
                float tmp = sprite_out->miny;
                sprite_out->miny = sprite_out->maxy;
                sprite_out->maxy = tmp;
            }
        }

        return 0;
    }
}

neko_spritebatch_internal_premade_atlas* neko_spritebatch_internal_premade_sprite(neko_spritebatch_t* sb, u64 image_id, neko_spritebatch_sprite_t* sprite_out, int skip_missing_textures) {
    neko_spritebatch_internal_premade_atlas* tex = (neko_spritebatch_internal_premade_atlas*)hashtable_find(&sb->sprites_to_premade_textures, image_id);

    if (!skip_missing_textures) {
        if (!tex) return tex;

        if (tex->texture_id == ~0) {
            int w = tex->w;
            int h = tex->h;
            tex->texture_id = neko_spritebatch_internal_generate_texture_handle(sb, image_id, w, h);
        }

        if (sprite_out) {
            sprite_out->texture_id = tex->texture_id;
        }
    }

    return tex;
}

int neko_spritebatch_internal_push_sprite(neko_spritebatch_t* sb, neko_spritebatch_internal_sprite_t* s, int skip_missing_textures) {
    int skipped_tex = 0;
    neko_spritebatch_sprite_t sprite;
    sprite.image_id = s->image_id;
    sprite.sort_bits = s->sort_bits;
    sprite.x = s->x;
    sprite.y = s->y;
    sprite.w = s->w;
    sprite.h = s->h;
    sprite.sx = s->sx;
    sprite.sy = s->sy;
    sprite.c = s->c;
    sprite.s = s->s;

    sprite.minx = s->premade_minx;
    sprite.miny = s->premade_miny;
    sprite.maxx = s->premade_maxx;
    sprite.maxy = s->premade_maxy;
#ifdef SPRITEBATCH_SPRITE_USERDATA
    sprite.udata = s->udata;
#endif

    neko_spritebatch_internal_premade_atlas* premade_atlas = neko_spritebatch_internal_premade_sprite(sb, s->image_id, &sprite, skip_missing_textures);

    if (!premade_atlas) {
        void* atlas_ptr = hashtable_find(&sb->sprites_to_atlases, s->image_id);
        if (atlas_ptr) {
            neko_spritebatch_internal_atlas_t* atlas = *(neko_spritebatch_internal_atlas_t**)atlas_ptr;
            sprite.texture_id = atlas->texture_id;

            neko_spritebatch_internal_texture_t* tex = (neko_spritebatch_internal_texture_t*)hashtable_find(&atlas->sprites_to_textures, s->image_id);
            NEKO_ASSERT(tex);
            tex->timestamp = 0;
            sprite.w = tex->w;
            sprite.h = tex->h;
            sprite.minx = tex->minx;
            sprite.miny = tex->miny;
            sprite.maxx = tex->maxx;
            sprite.maxy = tex->maxy;
        } else
            skipped_tex = neko_spritebatch_internal_lonely_sprite(sb, s->image_id, s->w, s->h, &sprite, skip_missing_textures);
    }

    if (!skipped_tex) {
        if (sb->sprite_count >= sb->sprite_capacity) {
            int new_capacity = sb->sprite_capacity * 2;
            void* new_data = malloc(sizeof(neko_spritebatch_sprite_t) * new_capacity);
            if (!new_data) return 0;
            memcpy(new_data, sb->sprites, sizeof(neko_spritebatch_sprite_t) * sb->sprite_count);
            free(sb->sprites);
            sb->sprites = (neko_spritebatch_sprite_t*)new_data;
            sb->sprite_capacity = new_capacity;

            if (sb->sprites_scratch) {
                free(sb->sprites_scratch);
            }

            if (sprite_batch_internal_use_scratch_buffer(sb)) {
                sb->sprites_scratch = (neko_spritebatch_sprite_t*)malloc(sizeof(neko_spritebatch_sprite_t) * new_capacity);
            }
        }
        sb->sprites[sb->sprite_count++] = sprite;
    }
    return skipped_tex;
}

void neko_spritebatch_internal_process_input(neko_spritebatch_t* sb, int skip_missing_textures) {
    int skipped_index = 0;
    for (int i = 0; i < sb->input_count; ++i) {
        neko_spritebatch_internal_sprite_t* s = sb->input_buffer + i;
        int skipped = neko_spritebatch_internal_push_sprite(sb, s, skip_missing_textures);
        if (skip_missing_textures && skipped) sb->input_buffer[skipped_index++] = *s;
    }

    sb->input_count = skipped_index;
}

void neko_spritebatch_tick(neko_spritebatch_t* sb) {
    neko_spritebatch_internal_atlas_t* atlas = sb->atlases;
    if (atlas) {
        neko_spritebatch_internal_atlas_t* sentinel = atlas;
        do {
            int texture_count = hashtable_count(&atlas->sprites_to_textures);
            neko_spritebatch_internal_texture_t* textures = (neko_spritebatch_internal_texture_t*)hashtable_items(&atlas->sprites_to_textures);
            for (int i = 0; i < texture_count; ++i) textures[i].timestamp += 1;
            atlas = atlas->next;
        } while (atlas != sentinel);
    }

    int texture_count = hashtable_count(&sb->sprites_to_lonely_textures);
    neko_spritebatch_internal_lonely_texture_t* lonely_textures = (neko_spritebatch_internal_lonely_texture_t*)hashtable_items(&sb->sprites_to_lonely_textures);
    for (int i = 0; i < texture_count; ++i) lonely_textures[i].timestamp += 1;
}

int neko_spritebatch_flush(neko_spritebatch_t* sb) {
    // process input buffer, make any necessary lonely textures
    // convert user sprites to internal format
    // lookup uv coordinates
    neko_spritebatch_internal_process_input(sb, 0);

    // patchup any missing lonely textures that may have come from atlases decaying and whatnot
    int texture_count = hashtable_count(&sb->sprites_to_lonely_textures);
    neko_spritebatch_internal_lonely_texture_t* lonely_textures = (neko_spritebatch_internal_lonely_texture_t*)hashtable_items(&sb->sprites_to_lonely_textures);
    for (int i = 0; i < texture_count; ++i) {
        neko_spritebatch_internal_lonely_texture_t* lonely = lonely_textures + i;
        if (lonely->texture_id == ~0) lonely->texture_id = neko_spritebatch_internal_generate_texture_handle(sb, lonely->image_id, lonely->w, lonely->h);
    }

    // sort internal sprite buffer and submit batches
    neko_spritebatch_internal_sort_sprites(sb);

    int min = 0;
    int max = 0;
    int done = !sb->sprite_count;
    int count = 0;
    while (!done) {
        u64 id = sb->sprites[min].texture_id;
        u64 image_id = sb->sprites[min].image_id;

        while (1) {
            if (max == sb->sprite_count) {
                done = 1;
                break;
            }

            if (id != sb->sprites[max].texture_id) break;

            ++max;
        }

        int batch_count = max - min;
        if (batch_count) {
            int w, h;
            neko_spritebatch_internal_premade_atlas* premade_atlas = (neko_spritebatch_internal_premade_atlas*)hashtable_find(&sb->sprites_to_premade_textures, image_id);
            if (!premade_atlas) {
                void* atlas_ptr = hashtable_find(&sb->sprites_to_atlases, image_id);

                if (atlas_ptr) {
                    w = sb->atlas_width_in_pixels;
                    h = sb->atlas_height_in_pixels;
                }

                else {
                    neko_spritebatch_internal_lonely_texture_t* tex = (neko_spritebatch_internal_lonely_texture_t*)hashtable_find(&sb->sprites_to_lonely_textures, image_id);
                    NEKO_ASSERT(tex);
                    w = tex->w;
                    h = tex->h;
                    if (sb->atlas_use_border_pixels) {
                        w += 2;
                        h += 2;
                    }
                }
            } else {
                w = premade_atlas->w;
                h = premade_atlas->h;
            }

            sb->batch_callback(sb->sprites + min, batch_count, w, h, sb->udata);
            ++count;
        }
        min = max;
    }

    sb->sprite_count = 0;
    if (count > 1) {
        neko_log_trace("[batch] Flushed %d batches.", count);
    }

    return count;
}

typedef struct {
    int x;
    int y;
} neko_spritebatch_v2_t;

typedef struct {
    int img_index;
    neko_spritebatch_v2_t size;
    neko_spritebatch_v2_t min;
    neko_spritebatch_v2_t max;
    int fit;
} neko_spritebatch_internal_integer_image_t;

static neko_spritebatch_v2_t neko_spritebatch_v2(int x, int y) {
    neko_spritebatch_v2_t v;
    v.x = x;
    v.y = y;
    return v;
}

static neko_spritebatch_v2_t neko_spritebatch_sub(neko_spritebatch_v2_t a, neko_spritebatch_v2_t b) {
    neko_spritebatch_v2_t v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    return v;
}

static neko_spritebatch_v2_t neko_spritebatch_add(neko_spritebatch_v2_t a, neko_spritebatch_v2_t b) {
    neko_spritebatch_v2_t v;
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    return v;
}

typedef struct {
    neko_spritebatch_v2_t size;
    neko_spritebatch_v2_t min;
    neko_spritebatch_v2_t max;
} neko_spritebatch_internal_atlas_node_t;

static neko_spritebatch_internal_atlas_node_t* neko_spritebatch_best_fit(int sp, int w, int h, neko_spritebatch_internal_atlas_node_t* nodes) {
    int best_volume = INT_MAX;
    neko_spritebatch_internal_atlas_node_t* best_node = 0;
    int img_volume = w * h;

    for (int i = 0; i < sp; ++i) {
        neko_spritebatch_internal_atlas_node_t* node = nodes + i;
        int can_contain = node->size.x >= w && node->size.y >= h;
        if (can_contain) {
            int node_volume = node->size.x * node->size.y;
            if (node_volume == img_volume) return node;
            if (node_volume < best_volume) {
                best_volume = node_volume;
                best_node = node;
            }
        }
    }

    return best_node;
}

static int neko_spritebatch_internal_image_less_than_or_equal(neko_spritebatch_internal_integer_image_t* a, neko_spritebatch_internal_integer_image_t* b) {
    int perimeterA = 2 * (a->size.x + a->size.y);
    int perimeterB = 2 * (b->size.x + b->size.y);
    return perimeterB <= perimeterA;
}

void neko_spritebatch_internal_image_merge_sort_iteration(neko_spritebatch_internal_integer_image_t* a, int lo, int split, int hi, neko_spritebatch_internal_integer_image_t* b) {
    int i = lo, j = split;
    for (int k = lo; k < hi; k++) {
        if (i < split && (j >= hi || neko_spritebatch_internal_image_less_than_or_equal(a + i, a + j))) {
            b[k] = a[i];
            i = i + 1;
        } else {
            b[k] = a[j];
            j = j + 1;
        }
    }
}

void neko_spritebatch_internal_image_merge_sort_recurse(neko_spritebatch_internal_integer_image_t* b, int lo, int hi, neko_spritebatch_internal_integer_image_t* a) {
    if (hi - lo <= 1) return;
    int split = (lo + hi) / 2;
    neko_spritebatch_internal_image_merge_sort_recurse(a, lo, split, b);
    neko_spritebatch_internal_image_merge_sort_recurse(a, split, hi, b);
    neko_spritebatch_internal_image_merge_sort_iteration(b, lo, split, hi, a);
}

void neko_spritebatch_internal_image_merge_sort(neko_spritebatch_internal_integer_image_t* a, neko_spritebatch_internal_integer_image_t* b, int n) {
    memcpy(b, a, sizeof(neko_spritebatch_internal_integer_image_t) * n);
    neko_spritebatch_internal_image_merge_sort_recurse(b, 0, n, a);
}

typedef struct {
    int img_index;     // index into the `imgs` array
    int w, h;          // pixel w/h of original image
    float minx, miny;  // u coordinate
    float maxx, maxy;  // v coordinate
    int fit;           // non-zero if image fit and was placed into the atlas
} neko_spritebatch_internal_atlas_image_t;

void neko_spritebatch_make_atlas(neko_spritebatch_t* sb, neko_spritebatch_internal_atlas_t* atlas_out, const neko_spritebatch_internal_lonely_texture_t* imgs, int img_count) {
    float w0, h0, div, wTol, hTol;
    int atlas_image_size, atlas_stride, sp;
    void* atlas_pixels = 0;
    int atlas_node_capacity = img_count * 2;
    neko_spritebatch_internal_integer_image_t* images = 0;
    neko_spritebatch_internal_integer_image_t* images_scratch = 0;
    neko_spritebatch_internal_atlas_node_t* nodes = 0;
    int pixel_stride = sb->pixel_stride;
    int atlas_width = sb->atlas_width_in_pixels;
    int atlas_height = sb->atlas_height_in_pixels;
    float volume_used = 0;

    images = (neko_spritebatch_internal_integer_image_t*)malloc(sizeof(neko_spritebatch_internal_integer_image_t) * img_count);
    images_scratch = (neko_spritebatch_internal_integer_image_t*)malloc(sizeof(neko_spritebatch_internal_integer_image_t) * img_count);
    nodes = (neko_spritebatch_internal_atlas_node_t*)malloc(sizeof(neko_spritebatch_internal_atlas_node_t) * atlas_node_capacity);
    NEKO_ASSERT(images && "out of mem");
    NEKO_ASSERT(nodes && "out of mem");

    for (int i = 0; i < img_count; ++i) {
        const neko_spritebatch_internal_lonely_texture_t* img = imgs + i;
        neko_spritebatch_internal_integer_image_t* image = images + i;
        image->fit = 0;
        image->size = sb->atlas_use_border_pixels ? neko_spritebatch_v2(img->w + 2, img->h + 2) : neko_spritebatch_v2(img->w, img->h);
        image->img_index = i;
    }

    // Sort PNGs from largest to smallest
    neko_spritebatch_internal_image_merge_sort(images, images_scratch, img_count);

    // stack pointer, the stack is the nodes array which we will
    // allocate nodes from as necessary.
    sp = 1;

    nodes[0].min = neko_spritebatch_v2(0, 0);
    nodes[0].max = neko_spritebatch_v2(atlas_width, atlas_height);
    nodes[0].size = neko_spritebatch_v2(atlas_width, atlas_height);

    // Nodes represent empty space in the atlas. Placing a texture into the
    // atlas involves splitting a node into two smaller pieces (or, if a
    // perfect fit is found, deleting the node).
    for (int i = 0; i < img_count; ++i) {
        neko_spritebatch_internal_integer_image_t* image = images + i;
        int width = image->size.x;
        int height = image->size.y;
        neko_spritebatch_internal_atlas_node_t* best_fit = neko_spritebatch_best_fit(sp, width, height, nodes);

        if (!best_fit) {
            image->fit = 0;
            continue;
        }

        image->min = best_fit->min;
        image->max = neko_spritebatch_add(image->min, image->size);

        if (best_fit->size.x == width && best_fit->size.y == height) {
            neko_spritebatch_internal_atlas_node_t* last_node = nodes + --sp;
            *best_fit = *last_node;
            image->fit = 1;

            continue;
        }

        image->fit = 1;

        if (sp == atlas_node_capacity) {
            int new_capacity = atlas_node_capacity * 2;
            neko_spritebatch_internal_atlas_node_t* new_nodes = (neko_spritebatch_internal_atlas_node_t*)malloc(sizeof(neko_spritebatch_internal_atlas_node_t) * new_capacity);
            NEKO_ASSERT(new_nodes && "out of mem");
            memcpy(new_nodes, nodes, sizeof(neko_spritebatch_internal_atlas_node_t) * sp);
            free(nodes);
            nodes = new_nodes;
            atlas_node_capacity = new_capacity;
        }

        neko_spritebatch_internal_atlas_node_t* new_node = nodes + sp++;
        new_node->min = best_fit->min;

        // Split bestFit along x or y, whichever minimizes
        // fragmentation of empty space
        neko_spritebatch_v2_t d = neko_spritebatch_sub(best_fit->size, neko_spritebatch_v2(width, height));
        if (d.x < d.y) {
            new_node->size.x = d.x;
            new_node->size.y = height;
            new_node->min.x += width;

            best_fit->size.y = d.y;
            best_fit->min.y += height;
        }

        else {
            new_node->size.x = width;
            new_node->size.y = d.y;
            new_node->min.y += height;

            best_fit->size.x = d.x;
            best_fit->min.x += width;
        }

        new_node->max = neko_spritebatch_add(new_node->min, new_node->size);
    }

    // Write the final atlas image, use SPRITEBATCH_ATLAS_EMPTY_COLOR as base color
    atlas_stride = atlas_width * pixel_stride;
    atlas_image_size = atlas_width * atlas_height * pixel_stride;
    atlas_pixels = malloc(atlas_image_size);
    NEKO_ASSERT(atlas_image_size && "out of mem");
    memset(atlas_pixels, SPRITEBATCH_ATLAS_EMPTY_COLOR, atlas_image_size);

    for (int i = 0; i < img_count; ++i) {
        neko_spritebatch_internal_integer_image_t* image = images + i;

        if (image->fit) {
            const neko_spritebatch_internal_lonely_texture_t* img = imgs + image->img_index;
            neko_spritebatch_internal_get_pixels(sb, img->image_id, img->w, img->h);
            char* pixels = (char*)sb->pixel_buffer;

            neko_spritebatch_v2_t min = image->min;
            neko_spritebatch_v2_t max = image->max;
            int atlas_offset = min.x * pixel_stride;
            int tex_stride = image->size.x * pixel_stride;

            for (int row = min.y, y = 0; row < max.y; ++row, ++y) {
                void* row_ptr = (char*)atlas_pixels + (row * atlas_stride + atlas_offset);
                memcpy(row_ptr, pixels + y * tex_stride, tex_stride);
            }
        }
    }

    hashtable_init(&atlas_out->sprites_to_textures, sizeof(neko_spritebatch_internal_texture_t), img_count, NULL);
    atlas_out->texture_id = sb->generate_texture_callback(atlas_pixels, atlas_width, atlas_height, sb->udata);

    // squeeze UVs inward by 128th of a pixel
    // this prevents atlas bleeding. tune as necessary for good results.
    w0 = 1.0f / (float)(atlas_width);
    h0 = 1.0f / (float)(atlas_height);
    div = 1.0f / 128.0f;
    wTol = w0 * div;
    hTol = h0 * div;

    for (int i = 0; i < img_count; ++i) {
        neko_spritebatch_internal_integer_image_t* img = images + i;

        if (img->fit) {
            neko_spritebatch_v2_t min = img->min;
            neko_spritebatch_v2_t max = img->max;
            volume_used += img->size.x * img->size.y;

            float min_x = (float)min.x * w0 + wTol;
            float min_y = (float)min.y * h0 + hTol;
            float max_x = (float)max.x * w0 - wTol;
            float max_y = (float)max.y * h0 - hTol;

            // flip image on y axis
            if (SPRITEBATCH_ATLAS_FLIP_Y_AXIS_FOR_UV) {
                float tmp = min_y;
                min_y = max_y;
                max_y = tmp;
            }

            neko_spritebatch_internal_texture_t texture;
            texture.w = img->size.x;
            texture.h = img->size.y;
            texture.timestamp = 0;
            texture.minx = min_x;
            texture.miny = min_y;
            texture.maxx = max_x;
            texture.maxy = max_y;
            NEKO_ASSERT(!(img->size.x < 0));
            NEKO_ASSERT(!(img->size.y < 0));
            NEKO_ASSERT(!(min_x < 0));
            NEKO_ASSERT(!(max_x < 0));
            NEKO_ASSERT(!(min_y < 0));
            NEKO_ASSERT(!(max_y < 0));
            texture.image_id = imgs[img->img_index].image_id;
            hashtable_insert(&atlas_out->sprites_to_textures, texture.image_id, &texture);
        }
    }

    // Need to adjust atlas_width and atlas_height in config params, as none of the images for this
    // atlas actually fit inside of the atlas! Either adjust the config, or stop sending giant images
    // to the sprite batcher.
    NEKO_ASSERT(volume_used > 0);

    atlas_out->volume_ratio = volume_used / (atlas_width * atlas_height);

sb_err:
    // no specific error handling needed here (yet)

    free(atlas_pixels);
    free(nodes);
    free(images);
    free(images_scratch);
    return;
}

static int neko_spritebatch_internal_lonely_pred(neko_spritebatch_internal_lonely_texture_t* a, neko_spritebatch_internal_lonely_texture_t* b) { return a->timestamp < b->timestamp; }

static void neko_spritebatch_internal_qsort_lonely(hashtable_t* lonely_table, neko_spritebatch_internal_lonely_texture_t* items, int count) {
    if (count <= 1) return;

    neko_spritebatch_internal_lonely_texture_t pivot = items[count - 1];
    int low = 0;
    for (int i = 0; i < count - 1; ++i) {
        if (neko_spritebatch_internal_lonely_pred(items + i, &pivot)) {
            hashtable_swap(lonely_table, i, low);
            low++;
        }
    }

    hashtable_swap(lonely_table, low, count - 1);
    neko_spritebatch_internal_qsort_lonely(lonely_table, items, low);
    neko_spritebatch_internal_qsort_lonely(lonely_table, items + low + 1, count - 1 - low);
}

int neko_spritebatch_internal_buffer_key(neko_spritebatch_t* sb, u64 key) {
    SPRITEBATCH_CHECK_BUFFER_GROW(sb, key_buffer_count, key_buffer_capacity, key_buffer, u64);
    sb->key_buffer[sb->key_buffer_count++] = key;
    return 0;
}

void neko_spritebatch_internal_remove_table_entries(neko_spritebatch_t* sb, hashtable_t* table) {
    for (int i = 0; i < sb->key_buffer_count; ++i) hashtable_remove(table, sb->key_buffer[i]);
    sb->key_buffer_count = 0;
}

void neko_spritebatch_internal_flush_atlas(neko_spritebatch_t* sb, neko_spritebatch_internal_atlas_t* atlas, neko_spritebatch_internal_atlas_t** sentinel, neko_spritebatch_internal_atlas_t** next) {
    int ticks_to_decay_texture = sb->ticks_to_decay_texture;
    int texture_count = hashtable_count(&atlas->sprites_to_textures);
    neko_spritebatch_internal_texture_t* textures = (neko_spritebatch_internal_texture_t*)hashtable_items(&atlas->sprites_to_textures);

    for (int i = 0; i < texture_count; ++i) {
        neko_spritebatch_internal_texture_t* atlas_texture = textures + i;
        if (atlas_texture->timestamp < ticks_to_decay_texture) {
            int w = atlas_texture->w;
            int h = atlas_texture->h;
            if (sb->atlas_use_border_pixels) {
                w -= 2;
                h -= 2;
            }
            neko_spritebatch_internal_lonely_texture_t* lonely_texture = neko_spritebatch_internal_lonelybuffer_push(sb, atlas_texture->image_id, w, h, 0);
            lonely_texture->timestamp = atlas_texture->timestamp;
        }
        hashtable_remove(&sb->sprites_to_atlases, atlas_texture->image_id);
    }

    if (sb->atlases == atlas) {
        if (atlas->next == atlas)
            sb->atlases = 0;
        else
            sb->atlases = atlas->prev;
    }

    // handle loop end conditions if sentinel was removed from the chain
    if (sentinel && next) {
        if (*sentinel == atlas) {
            neko_log_trace("[batch] sentinel was also atlas: %p", *sentinel);
            if ((*next)->next != *sentinel) {
                neko_log_trace("[batch] *next = (*next)->next : %p = (*next)->%p", *next, (*next)->next);
                *next = (*next)->next;
            }

            neko_log_trace("[batch] *sentinel = *next : %p =  %p", *sentinel, *next);
            *sentinel = *next;
        }
    }

    atlas->next->prev = atlas->prev;
    atlas->prev->next = atlas->next;
    hashtable_term(&atlas->sprites_to_textures);
    sb->delete_texture_callback(atlas->texture_id, sb->udata);
    free(atlas);
}

void neko_spritebatch_internal_log_chain(neko_spritebatch_internal_atlas_t* atlas) {
    if (atlas) {
        // neko_spritebatch_internal_atlas_t* sentinel = atlas;
        // neko_log_trace("[batch] sentinel: %p", sentinel);
        // do {
        //     neko_spritebatch_internal_atlas_t* next = atlas->next;
        //     neko_log_trace("[batch] atlas %p", atlas);
        //     atlas = next;
        // } while (atlas != sentinel);
    }
}

int neko_spritebatch_defrag(neko_spritebatch_t* sb) {
    // remove decayed atlases and flush them to the lonely buffer
    // only flush textures that are not decayed
    int ticks_to_decay_texture = sb->ticks_to_decay_texture;
    float ratio_to_decay_atlas = sb->ratio_to_decay_atlas;
    neko_spritebatch_internal_atlas_t* atlas = sb->atlases;
    if (atlas) {
        neko_spritebatch_internal_log_chain(atlas);
        neko_spritebatch_internal_atlas_t* sentinel = atlas;
        do {
            neko_spritebatch_internal_atlas_t* next = atlas->next;
            int texture_count = hashtable_count(&atlas->sprites_to_textures);
            neko_spritebatch_internal_texture_t* textures = (neko_spritebatch_internal_texture_t*)hashtable_items(&atlas->sprites_to_textures);
            int decayed_texture_count = 0;
            for (int i = 0; i < texture_count; ++i)
                if (textures[i].timestamp >= ticks_to_decay_texture) decayed_texture_count++;

            float ratio;
            if (!decayed_texture_count)
                ratio = 0;
            else
                ratio = (float)texture_count / (float)decayed_texture_count;
            if (ratio > ratio_to_decay_atlas) {
                neko_log_trace("[batch] flushed atlas %p", atlas);
                neko_spritebatch_internal_flush_atlas(sb, atlas, &sentinel, &next);
            }
            atlas = next;
        } while (atlas != sentinel);
    }

    // merge mostly empty atlases
    float ratio_to_merge_atlases = sb->ratio_to_merge_atlases;
    atlas = sb->atlases;
    if (atlas) {
        int sp = 0;
        neko_spritebatch_internal_atlas_t* merge_stack[2];

        neko_spritebatch_internal_atlas_t* sentinel = atlas;
        do {
            neko_spritebatch_internal_atlas_t* next = atlas->next;

            NEKO_ASSERT(sp >= 0 && sp <= 2);
            if (sp == 2) {
                neko_log_trace("[batch] merged 2 atlases");
                neko_spritebatch_internal_flush_atlas(sb, merge_stack[0], &sentinel, &next);
                neko_spritebatch_internal_flush_atlas(sb, merge_stack[1], &sentinel, &next);
                sp = 0;
            }

            float ratio = atlas->volume_ratio;
            if (ratio < ratio_to_merge_atlases) merge_stack[sp++] = atlas;

            atlas = next;
        } while (atlas != sentinel);

        if (sp == 2) {
            neko_log_trace("[batch] merged 2 atlases (out of loop)");
            neko_spritebatch_internal_flush_atlas(sb, merge_stack[0], 0, 0);
            neko_spritebatch_internal_flush_atlas(sb, merge_stack[1], 0, 0);
        }
    }

    // remove decayed textures from the lonely buffer
    int lonely_buffer_count_till_decay = sb->lonely_buffer_count_till_decay;
    int lonely_count = hashtable_count(&sb->sprites_to_lonely_textures);
    neko_spritebatch_internal_lonely_texture_t* lonely_textures = (neko_spritebatch_internal_lonely_texture_t*)hashtable_items(&sb->sprites_to_lonely_textures);
    if (lonely_count >= lonely_buffer_count_till_decay) {
        neko_spritebatch_internal_qsort_lonely(&sb->sprites_to_lonely_textures, lonely_textures, lonely_count);
        int index = 0;
        while (1) {
            if (index == lonely_count) break;
            if (lonely_textures[index].timestamp >= ticks_to_decay_texture) break;
            ++index;
        }
        for (int i = index; i < lonely_count; ++i) {
            u64 texture_id = lonely_textures[i].texture_id;
            if (texture_id != ~0) sb->delete_texture_callback(texture_id, sb->udata);
            neko_spritebatch_internal_buffer_key(sb, lonely_textures[i].image_id);
            neko_log_trace("[batch] lonely texture decayed");
        }
        neko_spritebatch_internal_remove_table_entries(sb, &sb->sprites_to_lonely_textures);
        lonely_count -= lonely_count - index;
        NEKO_ASSERT(lonely_count == hashtable_count(&sb->sprites_to_lonely_textures));
    }

    // process input, but don't make textures just yet
    neko_spritebatch_internal_process_input(sb, 1);
    lonely_count = hashtable_count(&sb->sprites_to_lonely_textures);

    // while greater than lonely_buffer_count_till_flush elements in lonely buffer
    // grab lonely_buffer_count_till_flush of them and make an atlas
    int lonely_buffer_count_till_flush = sb->lonely_buffer_count_till_flush;
    int stuck = 0;
    while (lonely_count > lonely_buffer_count_till_flush && !stuck) {
        atlas = (neko_spritebatch_internal_atlas_t*)malloc(sizeof(neko_spritebatch_internal_atlas_t));
        if (sb->atlases) {
            atlas->prev = sb->atlases;
            atlas->next = sb->atlases->next;
            sb->atlases->next->prev = atlas;
            sb->atlases->next = atlas;
        }

        else {
            atlas->next = atlas;
            atlas->prev = atlas;
            sb->atlases = atlas;
        }

        neko_spritebatch_make_atlas(sb, atlas, lonely_textures, lonely_count);
        neko_log_trace("[batch] making atlas");

        int tex_count_in_atlas = hashtable_count(&atlas->sprites_to_textures);
        if (tex_count_in_atlas != lonely_count) {
            int hit_count = 0;
            for (int i = 0; i < lonely_count; ++i) {
                u64 key = lonely_textures[i].image_id;
                if (hashtable_find(&atlas->sprites_to_textures, key)) {
                    neko_spritebatch_internal_buffer_key(sb, key);
                    u64 texture_id = lonely_textures[i].texture_id;
                    if (texture_id != ~0) sb->delete_texture_callback(texture_id, sb->udata);
                    hashtable_insert(&sb->sprites_to_atlases, key, &atlas);
                    neko_log_trace("[batch] removing lonely texture for atlas%s", texture_id != ~0 ? "" : " (tex was ~0)");
                } else {
                    hit_count++;
                    NEKO_ASSERT(lonely_textures[i].w <= sb->atlas_width_in_pixels);
                    NEKO_ASSERT(lonely_textures[i].h <= sb->atlas_height_in_pixels);
                }
            }
            neko_spritebatch_internal_remove_table_entries(sb, &sb->sprites_to_lonely_textures);

            lonely_count = hashtable_count(&sb->sprites_to_lonely_textures);

            if (!hit_count) {
                // TODO
                // handle case where none fit in atlas
                stuck = 1;
                NEKO_ASSERT(0);
            }
        }

        else {
            for (int i = 0; i < lonely_count; ++i) {
                u64 key = lonely_textures[i].image_id;
                u64 texture_id = lonely_textures[i].texture_id;
                if (texture_id != ~0) sb->delete_texture_callback(texture_id, sb->udata);
                hashtable_insert(&sb->sprites_to_atlases, key, &atlas);
                neko_log_trace("[batch] (fast path) removing lonely texture for atlas%s", texture_id != ~0 ? "" : " (tex was ~0)");
            }
            hashtable_clear(&sb->sprites_to_lonely_textures);
            lonely_count = 0;
            break;
        }
    }

    int total_premade_count = hashtable_count(&sb->sprites_to_premade_textures);
    const neko_spritebatch_internal_premade_atlas* premade_atlases = (const neko_spritebatch_internal_premade_atlas*)hashtable_items(&sb->sprites_to_premade_textures);
    for (int i = 0; i < total_premade_count; ++i) {
        if (premade_atlases[i].mark_for_cleanup) {
            const u64 texture_id = premade_atlases[i].texture_id;
            if (texture_id != ~0) sb->delete_texture_callback(texture_id, sb->udata);
            neko_spritebatch_internal_buffer_key(sb, premade_atlases[i].image_id);
            neko_log_trace("[batch] premade atlas texture cleanedup");
        }
    }
    neko_spritebatch_internal_remove_table_entries(sb, &sb->sprites_to_premade_textures);

    return 1;
}