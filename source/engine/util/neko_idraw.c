
#include "engine/util/neko_idraw.h"

// STB
#include "libs/stb/stb_image.h"
#include "libs/stb/stb_rect_pack.h"
#include "libs/stb/stb_truetype.h"

// Global instance of immediate draw static data
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
    neko_idraw_pipeline_state_attr_t attr = neko_default_val();
    attr.depth_enabled = false;
    attr.stencil_enabled = false;
    attr.blend_enabled = true;
    attr.face_cull_enabled = false;
    attr.prim_type = (u16)NEKO_GRAPHICS_PRIMITIVE_TRIANGLES;
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
    neko_graphics_uniform_layout_desc_t uldesc = neko_default_val();
    uldesc.type = NEKO_GRAPHICS_UNIFORM_MAT4;
    neko_graphics_uniform_desc_t udesc = neko_default_val();
    memcpy(udesc.name, NEKO_IDRAW_UNIFORM_MVP_MATRIX, sizeof(NEKO_IDRAW_UNIFORM_MVP_MATRIX));
    udesc.layout = &uldesc;
    neko_idraw()->uniform = neko_graphics_uniform_create(&udesc);

    // Create sampler buffer
    neko_graphics_uniform_layout_desc_t sldesc = neko_default_val();
    sldesc.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D;
    neko_graphics_uniform_desc_t sbdesc = neko_default_val();
    memcpy(sbdesc.name, NEKO_IDRAW_UNIFORM_TEXTURE2D, sizeof(NEKO_IDRAW_UNIFORM_TEXTURE2D));
    sbdesc.layout = &sldesc;
    neko_idraw()->sampler = neko_graphics_uniform_create(&sbdesc);

    // Create default texture (4x4 white)
    neko_color_t pixels[16] = neko_default_val();
    memset(pixels, 255, 16 * sizeof(neko_color_t));

    neko_graphics_texture_desc_t tdesc = neko_default_val();
    tdesc.width = 4;
    tdesc.height = 4;
    tdesc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    tdesc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    tdesc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    *tdesc.data = pixels;

    neko_idraw()->tex_default = neko_graphics_texture_create(&tdesc);

    // Create shader
    neko_graphics_shader_source_desc_t vsrc;
    vsrc.type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX;
    vsrc.source = neko_idraw_v_fillsrc;
    neko_graphics_shader_source_desc_t fsrc;
    fsrc.type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT;
    fsrc.source = neko_idraw_f_fillsrc;
    neko_graphics_shader_source_desc_t neko_idraw_sources[] = {vsrc, fsrc};

    neko_graphics_shader_desc_t sdesc = neko_default_val();
    sdesc.sources = neko_idraw_sources;
    sdesc.size = sizeof(neko_idraw_sources);
    memcpy(sdesc.name, "neko_immediate_default_fill_shader", sizeof("neko_immediate_default_fill_shader"));

    // Vertex attr layout
    neko_graphics_vertex_attribute_desc_t neko_idraw_vattrs[3] = neko_default_val();
    neko_idraw_vattrs[0].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3;
    memcpy(neko_idraw_vattrs[0].name, "a_position", sizeof("a_position"));
    neko_idraw_vattrs[1].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;
    memcpy(neko_idraw_vattrs[1].name, "a_uv", sizeof("a_uv"));
    neko_idraw_vattrs[2].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4;
    memcpy(neko_idraw_vattrs[2].name, "a_color", sizeof("a_color"));

    // Iterate through attribute list, then create custom pipelines requested.
    neko_handle(neko_graphics_shader_t) shader = neko_graphics_shader_create(&sdesc);

    // Pipelines
    for (u16 d = 0; d < 2; ++d)                  // Depth
        for (u16 s = 0; s < 2; ++s)              // Stencil
            for (u16 b = 0; b < 2; ++b)          // Blend
                for (u16 f = 0; f < 2; ++f)      // Face Cull
                    for (u16 p = 0; p < 2; ++p)  // Prim Type
                    {
                        neko_idraw_pipeline_state_attr_t attr = neko_default_val();

                        attr.depth_enabled = d;
                        attr.stencil_enabled = s;
                        attr.blend_enabled = b;
                        attr.face_cull_enabled = f;
                        attr.prim_type = p ? (u16)NEKO_GRAPHICS_PRIMITIVE_TRIANGLES : (u16)NEKO_GRAPHICS_PRIMITIVE_LINES;

                        // Create new pipeline based on this arrangement
                        neko_graphics_pipeline_desc_t pdesc = neko_default_val();
                        pdesc.raster.shader = shader;
                        pdesc.raster.index_buffer_element_size = sizeof(u16);
                        pdesc.raster.face_culling = attr.face_cull_enabled ? NEKO_GRAPHICS_FACE_CULLING_BACK : (neko_graphics_face_culling_type)0x00;
                        pdesc.raster.primitive = (neko_graphics_primitive_type)attr.prim_type;
                        pdesc.blend.func = attr.blend_enabled ? NEKO_GRAPHICS_BLEND_EQUATION_ADD : (neko_graphics_blend_equation_type)0x00;
                        pdesc.blend.src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA;
                        pdesc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
                        pdesc.depth.func = d ? NEKO_GRAPHICS_DEPTH_FUNC_LESS : (neko_graphics_depth_func_type)0x00;
                        pdesc.stencil.func = s ? NEKO_GRAPHICS_STENCIL_FUNC_ALWAYS : (neko_graphics_stencil_func_type)0x00;
                        pdesc.stencil.ref = s ? 1 : 0x00;
                        pdesc.stencil.comp_mask = s ? 0xFF : 0x00;
                        pdesc.stencil.write_mask = s ? 0xFF : 0x00;
                        pdesc.stencil.sfail = s ? NEKO_GRAPHICS_STENCIL_OP_KEEP : (neko_graphics_stencil_op_type)0x00;
                        pdesc.stencil.dpfail = s ? NEKO_GRAPHICS_STENCIL_OP_KEEP : (neko_graphics_stencil_op_type)0x00;
                        pdesc.stencil.dppass = s ? NEKO_GRAPHICS_STENCIL_OP_REPLACE : (neko_graphics_stencil_op_type)0x00;
                        pdesc.layout.attrs = neko_idraw_vattrs;
                        pdesc.layout.size = sizeof(neko_idraw_vattrs);

                        neko_handle(neko_graphics_pipeline_t) hndl = neko_graphics_pipeline_create(&pdesc);
                        neko_hash_table_insert(neko_idraw()->pipeline_table, attr, hndl);
                    }

    // Create default font
    neko_asset_ascii_font_t* f = &neko_idraw()->font_default;
    stbtt_fontinfo font = neko_default_val();
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

    neko_graphics_texture_desc_t desc = neko_default_val();
    desc.width = w;
    desc.height = h;
    *desc.data = flipmap;
    desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;

    // Generate atlas texture for bitmap with bitmap data
    f->texture.hndl = neko_graphics_texture_create(&desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    // Create vertex buffer
    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
    vdesc.data = NULL;
    vdesc.size = 0;
    vdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
    neko_idraw()->vbo = neko_graphics_vertex_buffer_create(&vdesc);

    neko_free(compressed_ttf_data);
    neko_free(buf_decompressed_data);
    neko_free(alpha_bitmap);
    neko_free(flipmap);
}

NEKO_API_DECL void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data) { g_neko_idraw = data; }

// Create / Init / Shutdown / Free
neko_immediate_draw_t neko_immediate_draw_new() {
    if (!neko_idraw()) {
        // Construct NEKO_IDRAW
        neko_immediate_draw_static_data_init();
    }

    neko_immediate_draw_t neko_idraw = neko_default_val();
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

NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_idraw_get_pipeline(neko_immediate_draw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state) {
    // Bind pipeline
    neko_assert(neko_hash_table_key_exists(neko_idraw()->pipeline_table, state));
    return neko_hash_table_get(neko_idraw()->pipeline_table, state);
}

void neko_immediate_draw_set_pipeline(neko_immediate_draw_t* neko_idraw) {
    if (neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES) {
        return;
    }

    // Check validity
    if (neko_idraw->cache.pipeline.prim_type != (u16)NEKO_GRAPHICS_PRIMITIVE_TRIANGLES && neko_idraw->cache.pipeline.prim_type != (u16)NEKO_GRAPHICS_PRIMITIVE_LINES) {
        neko_idraw->cache.pipeline.prim_type = (u16)NEKO_GRAPHICS_PRIMITIVE_TRIANGLES;
    }
    neko_idraw->cache.pipeline.depth_enabled = neko_clamp(neko_idraw->cache.pipeline.depth_enabled, 0, 1);
    neko_idraw->cache.pipeline.stencil_enabled = neko_clamp(neko_idraw->cache.pipeline.stencil_enabled, 0, 1);
    neko_idraw->cache.pipeline.face_cull_enabled = neko_clamp(neko_idraw->cache.pipeline.face_cull_enabled, 0, 1);
    neko_idraw->cache.pipeline.blend_enabled = neko_clamp(neko_idraw->cache.pipeline.blend_enabled, 0, 1);

    // Bind pipeline
    neko_assert(neko_hash_table_key_exists(neko_idraw()->pipeline_table, neko_idraw->cache.pipeline));
    neko_graphics_pipeline_bind(&neko_idraw->commands, neko_hash_table_get(neko_idraw()->pipeline_table, neko_idraw->cache.pipeline));
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

    neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
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

void neko_idraw_rect_2d_textured_ext(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color) {

    neko_handle(neko_graphics_texture_t) tex = neko_default_val();
    tex.id = tex_id;

    neko_idraw_texture(neko_idraw, tex);
    __neko_draw_rect_2d_impl(neko_idraw, neko_v2(x0, y0), neko_v2(x1, y1), neko_v2(u0, v0), neko_v2(u1, v1), color);
    neko_idraw_texture(neko_idraw, (neko_texture_t){0});
}

/* Core Vertex Functions */
void neko_idraw_begin(neko_immediate_draw_t* neko_idraw, neko_graphics_primitive_type type) {
    switch (type) {
        default:
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES:
            type = NEKO_GRAPHICS_PRIMITIVE_TRIANGLES;
            break;
        case NEKO_GRAPHICS_PRIMITIVE_LINES:
            type = NEKO_GRAPHICS_PRIMITIVE_LINES;
            break;
    }

    // Push a new pipeline?
    if (neko_idraw->cache.pipeline.prim_type == type) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set primitive type
    neko_idraw->cache.pipeline.prim_type = type;

    // Bind pipeline
    neko_immediate_draw_set_pipeline(neko_idraw);
}

void neko_idraw_end(neko_immediate_draw_t* neko_idraw) {
    // Not sure what to do here...
}

neko_mat4 neko_idraw_get_modelview_matrix(neko_immediate_draw_t* neko_idraw) { return neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1]; }

neko_mat4 neko_idraw_get_projection_matrix(neko_immediate_draw_t* neko_idraw) { return neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1]; }

neko_mat4 neko_idraw_get_mvp_matrix(neko_immediate_draw_t* neko_idraw) {
    neko_mat4* mv = &neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1];
    neko_mat4* proj = &neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1];
    return neko_mat4_mul(*proj, *mv);
}

void neko_idraw_flush(neko_immediate_draw_t* neko_idraw) {
    // Don't flush if verts empty
    if (neko_byte_buffer_empty(&neko_idraw->vertices)) {
        return;
    }

    // Set up mvp matrix
    neko_mat4 mv = neko_idraw->cache.modelview[neko_dyn_array_size(neko_idraw->cache.modelview) - 1];
    neko_mat4 proj = neko_idraw->cache.projection[neko_dyn_array_size(neko_idraw->cache.projection) - 1];
    neko_mat4 mvp = neko_mat4_mul(proj, mv);

    // Update vertex buffer (command buffer version)
    neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
    vdesc.data = neko_idraw->vertices.data;
    vdesc.size = neko_byte_buffer_size(&neko_idraw->vertices);
    vdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;

    neko_graphics_vertex_buffer_request_update(&neko_idraw->commands, neko_idraw()->vbo, &vdesc);

    // Calculate draw count
    size_t vsz = sizeof(neko_immediate_vert_t);
    if (neko_dyn_array_size(neko_idraw->vattributes)) {
        // Calculate vertex stride
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

    // Set up all binding data
    neko_graphics_bind_vertex_buffer_desc_t vbuffer = neko_default_val();
    vbuffer.buffer = neko_idraw()->vbo;

    neko_graphics_bind_uniform_desc_t ubinds[2] = neko_default_val();
    ubinds[0].uniform = neko_idraw()->uniform;
    ubinds[0].data = &mvp;
    ubinds[1].uniform = neko_idraw()->sampler;
    ubinds[1].data = &neko_idraw->cache.texture;
    ubinds[1].binding = 0;

    // Bindings for all buffers: vertex, uniform, sampler
    neko_graphics_bind_desc_t binds = neko_default_val();
    binds.vertex_buffers.desc = &vbuffer;

    // if (neko_dyn_array_empty(neko_idraw->vattributes))
    if (~neko_idraw->flags & NEKO_IDRAW_FLAG_NO_BIND_UNIFORMS) {
        binds.uniforms.desc = ubinds;
        binds.uniforms.size = sizeof(ubinds);
    }

    // Bind bindings
    neko_graphics_apply_bindings(&neko_idraw->commands, &binds);

    // Submit draw
    neko_graphics_draw_desc_t draw = neko_default_val();
    draw.start = 0;
    draw.count = ct;
    neko_graphics_draw(&neko_idraw->commands, &draw);

    // Clear data
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

NEKO_API_DECL void neko_idraw_texture(neko_immediate_draw_t* neko_idraw, neko_handle(neko_graphics_texture_t) texture) {
    // Push a new pipeline?
    if (neko_idraw->cache.texture.id == texture.id) {
        return;
    }

    // Otherwise, we need to flush previous content
    neko_idraw_flush(neko_idraw);

    // Set texture
    neko_idraw->cache.texture = (texture.id && texture.id != UINT32_MAX) ? texture : neko_idraw()->tex_default;
}

NEKO_API_DECL void neko_idraw_pipeline_set(neko_immediate_draw_t* neko_idraw, neko_handle(neko_graphics_pipeline_t) pipeline) {
    neko_idraw_flush(neko_idraw);

    // Bind if valid
    if (pipeline.id) {
        neko_graphics_pipeline_bind(&neko_idraw->commands, pipeline);
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
        neko_immediate_vert_t v = neko_default_val();
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
    // Just grab main window for now. Will need to grab top of viewport stack in future
    neko_idraw_load_matrix(neko_idraw, neko_camera_get_view_projection(cam, width, height));
}

void neko_idraw_camera2D(neko_immediate_draw_t* neko_idraw, u32 width, u32 height) {
    // Flush previous
    neko_idraw_flush(neko_idraw);
    f32 l = 0.f, r = (f32)width, tp = 0.f, b = (f32)height;
    neko_mat4 ortho = neko_mat4_ortho(l, r, b, tp, -1.f, 1.f);
    neko_idraw_load_matrix(neko_idraw, ortho);
}

void neko_idraw_camera3D(neko_immediate_draw_t* neko_idraw, u32 width, u32 height) {
    // Flush previous
    neko_idraw_flush(neko_idraw);
    neko_camera_t c = neko_camera_perspective();
    neko_idraw_camera(neko_idraw, &c, width, height);
}

// Shape Drawing Utils
void neko_idraw_triangle(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type) {
    neko_idraw_trianglex(neko_idraw, x0, y0, 0.f, x1, y1, 0.f, x2, y2, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, r, g, b, a, type);
}

void neko_idraw_trianglev(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color, neko_graphics_primitive_type type) {
    neko_idraw_triangle(neko_idraw, a.x, a.y, b.x, b.y, c.x, c.y, color.r, color.g, color.b, color.a, type);
}

void neko_idraw_trianglex(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r, u8 g, u8 b,
                          u8 a, neko_graphics_primitive_type type) {
    switch (type) {
        default:
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
            neko_idraw_c4ub(neko_idraw, r, g, b, a);
            neko_idraw_tc2f(neko_idraw, u0, v0);
            neko_idraw_v3f(neko_idraw, x0, y0, z0);
            neko_idraw_tc2f(neko_idraw, u1, v1);
            neko_idraw_v3f(neko_idraw, x1, y1, z1);
            neko_idraw_tc2f(neko_idraw, u2, v2);
            neko_idraw_v3f(neko_idraw, x2, y2, z2);
            neko_idraw_end(neko_idraw);
        } break;

        case NEKO_GRAPHICS_PRIMITIVE_LINES: {
            neko_idraw_line3D(neko_idraw, x0, y0, z0, x1, y1, z1, r, g, b, a);
            neko_idraw_line3D(neko_idraw, x1, y1, z1, x2, y2, z2, r, g, b, a);
            neko_idraw_line3D(neko_idraw, x2, y2, z2, x0, y0, z0, r, g, b, a);
        } break;
    }
}

void neko_idraw_trianglevx(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t color,
                           neko_graphics_primitive_type type) {
    neko_idraw_trianglex(neko_idraw, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, uv0.x, uv0.y, uv1.x, uv1.y, uv2.x, uv2.y, color.r, color.g, color.b, color.a, type);
}

NEKO_API_DECL void neko_idraw_trianglevxmc(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t c0, neko_color_t c1,
                                           neko_color_t c2, neko_graphics_primitive_type type) {
    switch (type) {
        default:
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

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

        case NEKO_GRAPHICS_PRIMITIVE_LINES: {
            neko_idraw_line3Dmc(neko_idraw, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, c0.r, c0.g, c0.b, c0.a, c1.r, c1.g, c1.b, c1.a);
            neko_idraw_line3Dmc(neko_idraw, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, c1.r, c1.g, c1.b, c1.a, c2.r, c2.g, c2.b, c2.a);
            neko_idraw_line3Dmc(neko_idraw, v2.x, v2.y, v2.z, v0.x, v0.y, v0.z, c2.r, c2.g, c2.b, c2.a, c0.r, c0.g, c0.b, c0.a);
        } break;
    }
}

void neko_idraw_line3Dmc(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1) {
    neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_LINES);

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
    neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_LINES);
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

void neko_idraw_rectx(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, neko_graphics_primitive_type type) {
    // Shouldn't use triangles, because need to declare texture coordinates.
    switch (type) {
        case NEKO_GRAPHICS_PRIMITIVE_LINES: {
            // First triangle
            neko_idraw_line(neko_idraw, l, b, r, b, _r, _g, _b, _a);
            neko_idraw_line(neko_idraw, r, b, r, t, _r, _g, _b, _a);
            neko_idraw_line(neko_idraw, r, t, l, t, _r, _g, _b, _a);
            neko_idraw_line(neko_idraw, l, t, l, b, _r, _g, _b, _a);
            // neko_idraw_triangle(neko_idraw, l, b, r, b, l, t, _r, _g, _b, _a, type);
            // Second triangle
            // neko_idraw_triangle(neko_idraw, r, b, r, t, l, t, _r, _g, _b, _a, type);
        } break;

        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

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

void neko_idraw_rect(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, u8 _r, u8 _g, u8 _b, u8 _a, neko_graphics_primitive_type type) {
    neko_idraw_rectx(neko_idraw, l, b, r, t, 0.f, 0.f, 1.f, 1.f, _r, _g, _b, _a, type);
}

void neko_idraw_rectv(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_color_t color, neko_graphics_primitive_type type) {
    neko_idraw_rectx(neko_idraw, bl.x, bl.y, tr.x, tr.y, 0.f, 0.f, 1.f, 1.f, color.r, color.g, color.b, color.a, type);
}

void neko_idraw_rectvx(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type) {
    neko_idraw_rectx(neko_idraw, bl.x, bl.y, tr.x, tr.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

void neko_idraw_rectvd(neko_immediate_draw_t* neko_idraw, neko_vec2 xy, neko_vec2 wh, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type) {
    neko_idraw_rectx(neko_idraw, xy.x, xy.y, xy.x + wh.x, xy.y + wh.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

NEKO_API_DECL void neko_idraw_rect3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 min, neko_vec3 max, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_graphics_primitive_type type) {
    const neko_vec3 vt0 = min;
    const neko_vec3 vt1 = neko_v3(max.x, min.y, min.z);
    const neko_vec3 vt2 = neko_v3(min.x, max.y, max.z);
    const neko_vec3 vt3 = max;

    switch (type) {
        case NEKO_GRAPHICS_PRIMITIVE_LINES: {
            neko_idraw_line3Dv(neko_idraw, vt0, vt1, c);
            neko_idraw_line3Dv(neko_idraw, vt1, vt2, c);
            neko_idraw_line3Dv(neko_idraw, vt2, vt3, c);
            neko_idraw_line3Dv(neko_idraw, vt3, vt0, c);
        } break;

        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

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

NEKO_API_DECL void neko_idraw_rect3Dvd(neko_immediate_draw_t* neko_idraw, neko_vec3 xyz, neko_vec3 whd, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_graphics_primitive_type type) {
    neko_idraw_rect3Dv(neko_idraw, xyz, neko_vec3_add(xyz, whd), uv0, uv1, c, type);
}

void neko_idraw_circle_sector(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                              neko_graphics_primitive_type type) {
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
    neko_for_range_i(segments) {
        neko_vec2 _a = neko_v2(cx, cy);
        neko_vec2 _b = neko_v2(cx + sinf(neko_idraw_deg2rad * angle) * radius, cy + cosf(neko_idraw_deg2rad * angle) * radius);
        neko_vec2 _c = neko_v2(cx + sinf(neko_idraw_deg2rad * (angle + step)) * radius, cy + cosf(neko_idraw_deg2rad * (angle + step)) * radius);
        neko_idraw_trianglev(neko_idraw, _a, _b, _c, neko_color(r, g, b, a), type);
        angle += step;
    }
}

void neko_idraw_circle_sectorvx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, neko_color_t color,
                                neko_graphics_primitive_type type) {
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
    neko_for_range_i(segments) {
        neko_vec3 _a = neko_v3(cx, cy, cz);
        neko_vec3 _b = neko_v3(cx + sinf(neko_idraw_deg2rad * angle) * radius, cy + cosf(neko_idraw_deg2rad * angle) * radius, cz);
        neko_vec3 _c = neko_v3(cx + sinf(neko_idraw_deg2rad * (angle + step)) * radius, cy + cosf(neko_idraw_deg2rad * (angle + step)) * radius, cz);
        neko_idraw_trianglevx(neko_idraw, _a, _b, _c, neko_v2s(0.f), neko_v2s(0.5f), neko_v2s(1.f), color, type);
        angle += step;
    }
}

void neko_idraw_circle(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type) {
    neko_idraw_circle_sector(neko_idraw, cx, cy, radius, 0, 360, segments, r, g, b, a, type);
}

void neko_idraw_circlevx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t segments, neko_color_t color, neko_graphics_primitive_type type) {
    neko_idraw_circle_sectorvx(neko_idraw, c, radius, 0, 360, segments, color, type);
}

NEKO_API_DECL void neko_idraw_arc(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                                  neko_graphics_primitive_type type) {
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

void neko_idraw_box(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type) {
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
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
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

        case NEKO_GRAPHICS_PRIMITIVE_LINES: {
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

void neko_idraw_sphere(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type) {
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
        V.p.x = cx + (XZ)*cosf((SECANGLE));                \
        V.p.z = cz + (XZ)*sinf((SECANGLE));                \
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
    neko_vec2 current = neko_default_val();
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
                                       neko_graphics_primitive_type type) {
    if (sides < 3) sides = 3;

    int32_t numVertex = sides * 8;
    const f32 hh = height * 0.5f;

    switch (type) {
        default:
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
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

        case NEKO_GRAPHICS_PRIMITIVE_LINES: {
            neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_LINES);
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

NEKO_API_DECL void neko_idraw_cone(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type) {
    neko_idraw_cylinder(neko_idraw, x, y, z, 0.f, radius, height, sides, r, g, b, a, type);
}

void neko_idraw_text(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, const char* text, const neko_asset_ascii_font_t* fp, bool32_t flip_vertical, u8 r, u8 g, u8 b, u8 a) {
    // If no font, set to default
    if (!fp) {
        fp = &neko_idraw()->font_default;
    }

    neko_idraw_texture(neko_idraw, fp->texture.hndl);

    neko_mat4 rot = neko_mat4_rotatev(neko_deg2rad(-180.f), NEKO_XAXIS);

    // Get total dimensions of text
    neko_vec2 td = neko_asset_ascii_font_text_dimensions(fp, text, -1);
    f32 th = neko_asset_ascii_font_max_height(fp);

    // Move text to accomdate height
    // y += td.y;
    y += th;

    // Needs to be fixed in here. Not elsewhere.
    neko_idraw_begin(neko_idraw, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    {
        neko_idraw_c4ub(neko_idraw, r, g, b, a);
        while (text[0] != '\0') {
            char c = text[0];
            if (c >= 32 && c <= 127) {
                stbtt_aligned_quad q = neko_default_val();
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
    neko_graphics_set_view_scissor(&neko_idraw->commands, x, y, w, h);
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
    neko_graphics_clear_action_t action = neko_default_val();
    action.color[0] = (f32)c.r / 255.f;
    action.color[1] = (f32)c.g / 255.f;
    action.color[2] = (f32)c.b / 255.f;
    action.color[3] = (f32)c.a / 255.f;
    neko_graphics_clear_desc_t clear = neko_default_val();
    clear.actions = &action;
    neko_renderpass_t pass = neko_default_val();
    neko_graphics_renderpass_begin(cb, pass);
    neko_graphics_set_viewport(cb, (u32)viewport.x, (u32)viewport.y, (u32)viewport.z, (u32)viewport.w);
    neko_graphics_clear(cb, &clear);
    neko_idraw_draw(neko_idraw, cb);
    neko_graphics_renderpass_end(cb);
}

NEKO_API_DECL void neko_idraw_renderpass_submit_ex(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_graphics_clear_action_t* action) {
    neko_graphics_clear_desc_t clear = neko_default_val();
    clear.actions = action;
    neko_renderpass_t pass = neko_default_val();
    neko_graphics_renderpass_begin(cb, pass);
    neko_graphics_set_viewport(cb, (u32)viewport.x, (u32)viewport.y, (u32)viewport.z, (u32)viewport.w);
    neko_graphics_clear(cb, &clear);
    neko_idraw_draw(neko_idraw, cb);
    neko_graphics_renderpass_end(cb);
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
