

#include "neko_text_renderer.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <memory>
#include <string>

// #define NEKO_FONTCACHE_DEBUGPRINT
#include "engine/graphics/neko_fontcache.h"
#include "engine/utility/logger.hpp"
#include "libs/glad/glad.h"

// ----------------------------------- GPU Backend ----------------------------------

#define NEKO_GL_STATE_BACKUP()                                                   \
    GLenum last_active_texture;                                                  \
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *)&last_active_texture);             \
    glActiveTexture(GL_TEXTURE0);                                                \
    GLuint last_program;                                                         \
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)&last_program);                   \
    GLuint last_texture;                                                         \
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *)&last_texture);                \
    GLuint last_array_buffer;                                                    \
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint *)&last_array_buffer);         \
    GLint last_viewport[4];                                                      \
    glGetIntegerv(GL_VIEWPORT, last_viewport);                                   \
    GLint last_scissor_box[4];                                                   \
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);                             \
    GLenum last_blend_src_rgb;                                                   \
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&last_blend_src_rgb);               \
    GLenum last_blend_dst_rgb;                                                   \
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&last_blend_dst_rgb);               \
    GLenum last_blend_src_alpha;                                                 \
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&last_blend_src_alpha);           \
    GLenum last_blend_dst_alpha;                                                 \
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&last_blend_dst_alpha);           \
    GLenum last_blend_equation_rgb;                                              \
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint *)&last_blend_equation_rgb);     \
    GLenum last_blend_equation_alpha;                                            \
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint *)&last_blend_equation_alpha); \
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);                         \
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);                 \
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);               \
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);           \
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);           \
    GLboolean last_enable_mutisample = glIsEnabled(GL_MULTISAMPLE);              \
    GLboolean last_enable_framebuffer_srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB)

#define NEKO_GL_STATE_RESTORE()                                                                              \
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

namespace neko {

static GLint fontcache_shader_render_glyph;
static GLint fontcache_shader_blit_atlas;
static GLint fontcache_shader_draw_text;
static GLuint fontcache_fbo[2];
static GLuint fontcache_fbo_texture[2];

static int mouse_scroll = 0;

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

neko_private(GLint) ME_font_compile_shader(const std::string vs, const std::string fs) {
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
    assert(compiled);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &compiled);
    printCompileError(fshader);
    assert(compiled);
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

neko_private(void) compile_vbo(GLuint &dest_vb, GLuint &dest_ib, const neko_fontcache_vertex *verts, int nverts, const uint32_t *indices, int nindices) {
    if (dest_vb == 0 || dest_ib == 0) {
        GLuint buf[2];
        glGenBuffers(2, buf);
        dest_vb = buf[0], dest_ib = buf[1];
        glBindBuffer(GL_ARRAY_BUFFER, dest_vb);
        glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(neko_fontcache_vertex), verts, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dest_ib);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, nindices * sizeof(uint32_t), indices, GL_DYNAMIC_DRAW);
    }
}

neko_private(void) setup_fbo() {
    glGenFramebuffers(2, fontcache_fbo);
    glGenTextures(2, fontcache_fbo_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, fontcache_fbo[0]);
    glBindTexture(GL_TEXTURE_2D, fontcache_fbo_texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fontcache_fbo_texture[0], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fontcache_fbo[1]);
    glBindTexture(GL_TEXTURE_2D, fontcache_fbo_texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fontcache_fbo_texture[1], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

neko_private(GLuint) vao = 0;

neko_private(neko_fontcache) cache;

void text_renderer::drawcmd() {

    // ME_profiler_scope_auto("RenderGUI.Font");

    // TODO: plz!!! optimize this method
    NEKO_GL_STATE_BACKUP();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);

    neko_fontcache_optimise_drawlist(&cache);
    auto drawlist = neko_fontcache_get_drawlist(&cache);

    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    } else {
        glBindVertexArray(vao);
    }

    GLuint vbo = 0, ibo = 0;
    compile_vbo(vbo, ibo, drawlist->vertices.data(), drawlist->vertices.size(), drawlist->indices.data(), drawlist->indices.size());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(neko_fontcache_vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(neko_fontcache_vertex), (GLvoid *)(2 * sizeof(float)));

    for (auto &dcall : drawlist->dcalls) {
        if (dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH) {
            glUseProgram(fontcache_shader_render_glyph);
            glBindFramebuffer(GL_FRAMEBUFFER, fontcache_fbo[0]);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
            glViewport(0, 0, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);
            glScissor(0, 0, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);
            glDisable(GL_FRAMEBUFFER_SRGB);
        } else if (dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_ATLAS) {
            glUseProgram(fontcache_shader_blit_atlas);
            glBindFramebuffer(GL_FRAMEBUFFER, fontcache_fbo[1]);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glViewport(0, 0, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);
            glScissor(0, 0, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);
            glUniform1i(glGetUniformLocation(fontcache_shader_blit_atlas, "src_texture"), 0);
            glUniform1ui(glGetUniformLocation(fontcache_shader_blit_atlas, "region"), dcall.region);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fontcache_fbo_texture[0]);
            glDisable(GL_FRAMEBUFFER_SRGB);
        } else {
            glUseProgram(fontcache_shader_draw_text);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glViewport(0, 0, this->screen_w, this->screen_h);
            glScissor(0, 0, this->screen_w, this->screen_h);
            glUniform1i(glGetUniformLocation(fontcache_shader_draw_text, "src_texture"), 0);
            glUniform1ui(glGetUniformLocation(fontcache_shader_draw_text, "downsample"), dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? 1 : 0);
            glUniform4fv(glGetUniformLocation(fontcache_shader_draw_text, "colour"), 1, dcall.colour);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, dcall.pass == NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? fontcache_fbo_texture[0] : fontcache_fbo_texture[1]);
            glEnable(GL_FRAMEBUFFER_SRGB);
        }
        if (dcall.clear_before_draw) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (dcall.end_index - dcall.start_index == 0) continue;
        glDrawElements(GL_TRIANGLES, dcall.end_index - dcall.start_index, GL_UNSIGNED_INT, (GLvoid *)(dcall.start_index * sizeof(uint32_t)));
    }

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    // ME_CHECK_GL_ERROR();
    neko_fontcache_flush_drawlist(&cache);

    NEKO_GL_STATE_RESTORE();
}

#if 0
void test_font( ve_font_id id )
{
    std::string s = 
        /*u8"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n"
        u8"incididunt ut labore et dolore magna aliqua. Est ullamcorper eget nulla facilisi\n"
        u8"etiam dignissim diam quis enim. Convallis convallis tellus id interdum. Risus\n"
        u8"viverra adipiscing at in. Venenatis a condimentum vitae sapien pellentesque\n"
        u8"habitant morbi. Vitae et leo duis ut. Dignissim enim sit amet venenatis. Lacus\n"
        u8"viverra vitae congue eu consequat ac felis donec. Habitant morbi tristique\n"
        u8"senectus et netus. Scelerisque fermentum dui faucibus in ornare quam viverra\n"
        u8"orci sagittis. Porttitor lacus luctus accumsan tortor posuere ac. Tortor at\n"
        u8"auctor urna nunc id cursus metus. Massa id neque aliquam vestibulum morbi\n"
        u8"blandit cursus risus. A lacus vestibulum sed arcu non odio euismod lacinia at.\n"
        u8"Porttitor leo a diam sollicitudin tempor id eu nisl. Convallis aenean et tortor\n"
        u8"at risus viverra adipiscing at in. Dolor purus non enim praesent elementum\n"
        u8"facilisis. Hendrerit gravida rutrum quisque non tellus.\n"
        u8"\n"*/
        u8"Hello世界! Ça va! Això 文字列 ist řetězec by librería de software \"VEFONT\" キャッシュ! そうですね☺ 数\n"
        u8"左右中大小月日年早木林山川土空田天生花草虫犬人名女男子目耳口手足見\n"
        u8"毛頭顔首心時曜朝昼夜分週春夏秋冬今新古間方北南東西遠近前後内外場地\n"
        u8"国園谷野原里市京風雪雲池海岩星室戸家寺通門道話言答声聞語読書記紙画\n"
        u8"絵図工教晴思考知才理算作元食肉馬牛魚鳥羽鳴麦米茶色黄黒来行帰歩走止\n"
        u8"活店買売午汽弓回会組船明社切電毎合当台楽公引科歌刀番用何12345€£¥¢ ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        u8"abcdefghijklmnopqrstuvwxyz\n"
        u8"1234567890\n"
        u8"!#$%^&*()_+-=`~|\n"
        u8"Erat nam at lectus urna duis. Quam elementum pulvinar etiam non quam lacus\n"
        u8"suspendisse faucibus. Vulputate odio ut enim blandit. Arcu cursus vitae congue\n"
        u8"mauris rhoncus. Sapien eget mi proin sed libero enim. Nulla facilisi nullam\n"
        u8"vehicula ipsum. Ante metus dictum at tempor commodo ullamcorper a lacus\n"
        u8"vestibulum. Mauris in aliquam sem fringilla ut morbi tincidunt augue interdum.\n"
        u8"Et molestie ac feugiat sed. Lacus sed viverra tellus in hac habitasse platea\n"
        u8"dictumst.\n";

    neko_fontcache_flush_glyph_buffer_to_atlas( &cache );
    neko_fontcache_draw_text( &cache, id, s, 0.1f, 0.8f, 1.0f / this->screen_w,  1.0f / this->screen_h );
}

void test_font2( ve_font_id id )
{
    neko_fontcache_flush_glyph_buffer_to_atlas( &cache );
    std::string s = u8"Hello世界! Ça va! Això 文字列 ist řetězec by librería de software \"VEFONT\" キャッシュ! そうですね☺。";
    for( int i= 0; i < 2; i++ ) {
        neko_fontcache_draw_text( &cache, id, s, 0.12f, 0.05f + i * 0.033f, 1.0f / this->screen_w,  1.0f / this->screen_h );
    }
}
#endif

font_index text_renderer::load(const void *data, size_t data_size, f32 font_size) {
    neko_fontcache_init(&cache);
    neko_fontcache_configure_snap(&cache, this->screen_w, this->screen_h);
    // neko_private(std::vector<u8>) buffer;

    // 返回字体索引
    return neko_fontcache_load(&cache, data, data_size, font_size);
}

// 将绘制字加入待绘制列表
// pos 不是屏幕坐标也不是NDC
// pos 以窗口左下角为原点 窗口空间为第一象限
void text_renderer::push(const std::string &text, const font_index font, const neko_vec2 pos) {

    // ME_profiler_scope_auto("RenderGUI.Font.Post");

    neko_fontcache_configure_snap(&cache, this->screen_w, this->screen_h);

    neko_fontcache_draw_text(&cache, font, text, pos.x, pos.y, 1.0f / this->screen_w, 1.0f / this->screen_h);

    // // neko_fontcache_configure_snap(&cache, this->screen_w, this->screen_h);
    // static float current_scroll = 0.1f;

    // if (current_scroll < 1.5f) {
    //     std::string intro =
    //             U8_STR("Ça va! Everything here is rendered using VE Font Cache, a single header-only library designed for game engines.\n"
    //                    "It aims to:\n"
    //                    "           •    Be fast and simple to integrate.\n"
    //                    "           •    Take advantage of modern GPU power.\n"
    //                    "           •    Be backend agnostic and easy to port to any API such as Vulkan, DirectX, OpenGL.\n"
    //                    "           •    Load TTF & OTF file formats directly.\n"
    //                    "           •    Use only runtime cache with no offline calculation.\n"
    //                    "           •    Render glyphs at reasonable quality at a wide range of hb_font sizes.\n"
    //                    "           •    Support a good amount of internationalisation. そうですね!\n"
    //                    "           •    Support cached text shaping with HarfBuzz with simple Latin-style fallback.\n"
    //                    "           •    Load and unload fonts at any time.\n");

    //     // neko_fontcache_draw_text(&cache, logo_font, U8_STR("草"), 0.4f, current_scroll + 0.0f, 1.0f / this->screen_w, 1.0f / this->screen_h);
    //     // neko_fontcache_draw_text(&cache, title_font, U8_STR("基于 FontCache 的字体渲染"), 0.2f, current_scroll - 0.1f, 1.0f / this->screen_w, 1.0f / this->screen_h);
    //     // neko_fontcache_draw_text(&cache, print_font, intro, 0.2f, current_scroll - 0.14f, 1.0f / this->screen_w, 1.0f / this->screen_h);
    // }

#if 0

    float section_start = 0.42f;
    float section_end = 2.32f;
    if (current_scroll > section_start && current_scroll < section_end) {
        std::string how_it_works =
                u8"Glyphs are GPU rasterised with 16x supersampling. This method is a simplification of \"Easy Scalable Text the<engine>().eng()->ing on the GPU\",\n"
                u8"by Evan Wallace, making use of XOR blending. Bézier curves are handled via brute force triangle tessellation; even 6 triangles per\n"
                u8"curve only generates < 300 triangles, which is nothing for modern GPUs! This avoids complex frag shader for reasonable quality.\n"
                u8"\n"
                u8"Texture atlas caching uses naïve grid placement; this wastes a lot of space but ensures interchangeable cache slots allowing for\n"
                u8"straight up LRU ( Least Recently Used ) caching scheme to be employed.\n"
                u8"The hb_font atlas is a single 4k x 2k R8 texture divided into 4 regions:";
        std::string caching_strategy =
                u8"                         2k\n"
                u8"                         --------------------\n"
                u8"                         |         |        |\n"
                u8"                         |    A    |        |\n"
                u8"                         |         |        | 2\n"
                u8"                         |---------|    C   | k  \n"
                u8"                         |         |        |\n"
                u8"                      1k |    B    |        |\n"
                u8"                         |         |        |\n"
                u8"                         --------------------\n"
                u8"                         |                  |\n"
                u8"                         |                  |\n"
                u8"                         |                  | 2\n"
                u8"                         |        D         | k  \n"
                u8"                         |                  |\n"
                u8"                         |                  |\n"
                u8"                         |                  |\n"
                u8"                         --------------------\n"
                u8"                    \n"
                u8"                         Region A = 32x32 caches, 1024 glyphs\n"
                u8"                         Region B = 32x64 caches, 512 glyphs\n"
                u8"                         Region C = 64x64 caches, 512 glyphs\n"
                u8"                         Region D = 128x128 caches, 256 glyphs\n";
        std::string how_it_works2 =
                u8"Region A is designed for small glyphs, Region B is for tall glyphs, Region C is for large glyphs, and Region D for huge glyphs.\n"
                u8"Glyphs are first rendered to an intermediate 2k x 512px R8 texture. This allows for minimum 4 Region D glyphs supersampled at\n"
                u8"4 x 4 = 16x supersampling, and 8 Region C glyphs similarly. A simple 16-tap box downsample shader is then used to blit from this\n"
                u8"intermediate texture to the final atlas location.\n";
        neko_fontcache_draw_text(&cache, title_font, u8"How it works", 0.2f, current_scroll - (section_start + 0.06f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, how_it_works, 0.2f, current_scroll - (section_start + 0.1f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, mono_font, caching_strategy, 0.28f, current_scroll - (section_start + 0.32f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, how_it_works2, 0.2f, current_scroll - (section_start + 0.82f), 1.0f / this->screen_w, 1.0f / this->screen_h);
    }

    section_start = 1.2f;
    section_end = 3.2f;
    if (current_scroll > section_start && current_scroll < section_end) {
        std::string font_family_test =
                u8"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n"
                u8"incididunt ut labore et dolore magna aliqua. Est ullamcorper eget nulla facilisi\n"
                u8"etiam dignissim diam quis enim. Convallis convallis tellus id interdum.";
        neko_fontcache_draw_text(&cache, title_font, u8"Showcase", 0.2f, current_scroll - (section_start + 0.2f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"This is a showcase demonstrating different hb_font categories and languages.", 0.2f, current_scroll - (section_start + 0.24f),
                               1.0f / this->screen_w, 1.0f / this->screen_h);

        neko_fontcache_draw_text(&cache, print_font, u8"Sans serif", 0.2f, current_scroll - (section_start + 0.28f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_sans_font, font_family_test, 0.3f, current_scroll - (section_start + 0.28f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Serif", 0.2f, current_scroll - (section_start + 0.36f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_serif_font, font_family_test, 0.3f, current_scroll - (section_start + 0.36f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Script", 0.2f, current_scroll - (section_start + 0.44f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_script_font, font_family_test, 0.3f, current_scroll - (section_start + 0.44f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Monospace", 0.2f, current_scroll - (section_start + 0.52f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_mono_font, font_family_test, 0.3f, current_scroll - (section_start + 0.52f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Small", 0.2f, current_scroll - (section_start + 0.60f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, small_font, font_family_test, 0.3f, current_scroll - (section_start + 0.60f), 1.0f / this->screen_w, 1.0f / this->screen_h);

        neko_fontcache_draw_text(&cache, print_font, u8"Greek", 0.2f, current_scroll - (section_start + 0.72f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_sans_font, u8"Ήταν απλώς θέμα χρόνου.", 0.3f, current_scroll - (section_start + 0.72f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Vietnamnese", 0.2f, current_scroll - (section_start + 0.76f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_sans_font, u8"Bầu trời trong xanh thăm thẳm, không một gợn mây.", 0.3f, current_scroll - (section_start + 0.76f), 1.0f / this->screen_w,
                               1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Thai", 0.2f, current_scroll - (section_start + 0.80f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_thai_font, u8"การเดินทางขากลับคงจะเหงา", 0.3f, current_scroll - (section_start + 0.80f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Chinese", 0.2f, current_scroll - (section_start + 0.84f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_chinese_font, u8"床前明月光 疑是地上霜 举头望明月 低头思故乡", 0.3f, current_scroll - (section_start + 0.84f), 1.0f / this->screen_w,
                               1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Japanese", 0.2f, current_scroll - (section_start + 0.88f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_japanese_font, u8"ぎょしょうとナレズシの研究 モンスーン・アジアの食事文化", 0.3f, current_scroll - (section_start + 0.88f), 1.0f / this->screen_w,
                               1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Korean", 0.2f, current_scroll - (section_start + 0.92f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_korean_font, u8"그들의 장비와 기구는 모두 살아 있다.", 0.3f, current_scroll - (section_start + 0.92f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Arabic", 0.2f, current_scroll - (section_start + 0.96f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_arabic_font, u8"حب السماء لا تمطر غير الأحلام. This one needs HarfBuzz to work!", 0.3f, current_scroll - (section_start + 0.96f), 1.0f / this->screen_w,
                               1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, print_font, u8"Hebrew", 0.2f, current_scroll - (section_start + 1.0f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        neko_fontcache_draw_text(&cache, demo_hebrew_font, u8"אז הגיע הלילה של כוכב השביט הראשון. This one needs HarfBuzz to work!", 0.3f, current_scroll - (section_start + 1.0f),
                               1.0f / this->screen_w, 1.0f / this->screen_h);
    }

    section_start = 2.1f;
    section_end = section_start + 2.23f;
    if (current_scroll > section_start && current_scroll < section_end) {
        const int GRID_W = 80, GRID_H = 50, NUM_RAINDROPS = GRID_W / 3;

        static bool init_grid = false;
        static int grid[GRID_W * GRID_H];
        static float grid_age[GRID_W * GRID_H];
        static int raindropsX[NUM_RAINDROPS];
        static int raindropsY[NUM_RAINDROPS];
        static float code_colour[4];
        static std::array<std::string, 72> codes = {u8" ",  u8"0", u8"1", u8"2", u8"3", u8"4", u8"5", u8"6", u8"7", u8"8", u8"9", u8"Z", u8"T", u8"H", u8"E", u8"｜", u8"¦",  u8"日",
                                                    u8"ﾊ",  u8"ﾐ", u8"ﾋ", u8"ｰ", u8"ｳ", u8"ｼ", u8"ﾅ", u8"ﾓ", u8"ﾆ", u8"ｻ", u8"ﾜ", u8"ﾂ", u8"ｵ", u8"ﾘ", u8"ｱ", u8"ﾎ",  u8"ﾃ",  u8"ﾏ",
                                                    u8"ｹ",  u8"ﾒ", u8"ｴ", u8"ｶ", u8"ｷ", u8"ﾑ", u8"ﾕ", u8"ﾗ", u8"ｾ", u8"ﾈ", u8"ｽ", u8"ﾂ", u8"ﾀ", u8"ﾇ", u8"ﾍ", u8":",  u8"・", u8".",
                                                    u8"\"", u8"=", u8"*", u8"+", u8"-", u8"<", u8">", u8"ç", u8"ﾘ", u8"ｸ", u8"ｺ", u8"ﾁ", u8"ﾔ", u8"ﾙ", u8"ﾝ", u8"C",  u8"O",  u8"D"};

        if (!init_grid) {
            for (int i = 0; i < NUM_RAINDROPS; i++) raindropsY[i] = GRID_H;
            init_grid = true;
        }

        static float fixed_timestep_passed = 0.0f;
        fixed_timestep_passed += dT;
        while (fixed_timestep_passed > (1.0f / 20.0f)) {
            // Step grid.
            for (int i = 0; i < GRID_W * GRID_H; i++) {
                grid_age[i] += dT;
            }

            // Step raindrops.
            for (int i = 0; i < NUM_RAINDROPS; i++) {
                raindropsY[i]++;
                if (raindropsY[i] < 0) continue;
                if (raindropsY[i] >= GRID_H) {
                    raindropsY[i] = -5 - (rand() % 40);
                    raindropsX[i] = rand() % GRID_W;
                    continue;
                }
                grid[raindropsY[i] * GRID_W + raindropsX[i]] = rand() % codes.size();
                grid_age[raindropsY[i] * GRID_W + raindropsX[i]] = 0.0f;
            }
            fixed_timestep_passed -= (1.0f / 20.0f);
        }

        // Draw grid.
        neko_fontcache_draw_text(&cache, title_font, u8"Raincode demo", 0.2f, current_scroll - (section_start + 0.2f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                float posx = 0.2f + x * 0.007f, posy = current_scroll - (section_start + 0.24f + y * 0.018f);
                float age = grid_age[y * GRID_W + x];
                code_colour[0] = 1.0f;
                code_colour[1] = 1.0f;
                code_colour[2] = 1.0f;
                code_colour[3] = 1.0f;
                if (age > 0.0f) {
                    code_colour[0] = 0.2f;
                    code_colour[1] = 0.3f;
                    code_colour[2] = 0.4f;
                    code_colour[3] = 1.0f - age;
                    if (code_colour[3] < 0.0f) continue;
                }
                neko_fontcache_set_colour(&cache, code_colour);
                neko_fontcache_draw_text(&cache, demo_raincode_font, codes[grid[y * GRID_W + x]], posx, posy, 1.0f / this->screen_w, 1.0f / this->screen_h);
            }
        }

        code_colour[0] = code_colour[1] = code_colour[2] = code_colour[3] = 1.0f;
        neko_fontcache_set_colour(&cache, code_colour);
    }

    section_start = 3.3f;
    section_end = 5.1;
    if (current_scroll > section_start && current_scroll < section_end) {
        const int GRID_W = 30, GRID_H = 15, GRID2_W = 8, GRID2_H = 2, GRID3_W = 16, GRID3_H = 4;
        static int grid[GRID_W * GRID_H];
        static int grid2[GRID2_W * GRID2_H];
        static int grid3[GRID3_W * GRID3_H];

        static int rotate_current = 0;
        static float fixed_timestep_passed = 0.0f;
        fixed_timestep_passed += dT;
        while (fixed_timestep_passed > (1.0f / 20.0f)) {
            rotate_current = (rotate_current + 1) % 4;
            int rotate_idx = 0;
            for (auto& g : grid) {
                // CJK Unified Ideographs
                if ((rotate_idx++ % 4) != rotate_current) continue;
                g = 0x4E00 + rand() % (0x9FFF - 0x4E00);
            }
            for (auto& g : grid2) {
                g = 0x4E00 + rand() % (0x9FFF - 0x4E00);
            }
            for (auto& g : grid3) {
                g = rand() % 128;
            }
            fixed_timestep_passed -= (1.0f / 20.0f);
        }

        auto codepoint_to_utf8 = [](char* c, int chr) {
            if (0 == chr) {
                return;
            } else if (0 == ((int32_t)0xffffff80 & chr)) {
                c[0] = (char)chr;
            } else if (0 == ((int32_t)0xfffff800 & chr)) {
                c[0] = 0xc0 | (char)(chr >> 6);
                c[1] = 0x80 | (char)(chr & 0x3f);
            } else if (0 == ((int32_t)0xffff0000 & chr)) {
                c[0] = 0xe0 | (char)(chr >> 12);
                c[1] = 0x80 | (char)((chr >> 6) & 0x3f);
                c[2] = 0x80 | (char)(chr & 0x3f);
            } else {
                c[0] = 0xf0 | (char)(chr >> 18);
                c[1] = 0x80 | (char)((chr >> 12) & 0x3f);
                c[2] = 0x80 | (char)((chr >> 6) & 0x3f);
                c[3] = 0x80 | (char)(chr & 0x3f);
            }
        };

        // Draw grid.
        neko_fontcache_draw_text(&cache, title_font, u8"Cache pressure test", 0.2f, current_scroll - (section_start + 0.2f), 1.0f / this->screen_w, 1.0f / this->screen_h);
        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                float posx = 0.2f + x * 0.02f, posy = current_scroll - (section_start + 0.24f + y * 0.025f);
                char c[5] = {'\0', '\0', '\0', '\0', '\0'};
                codepoint_to_utf8(c, grid[y * GRID_W + x]);
                neko_fontcache_draw_text(&cache, demo_chinese_font, c, posx, posy, 1.0f / this->screen_w, 1.0f / this->screen_h);
            }
        }
        for (int y = 0; y < GRID2_H; y++) {
            for (int x = 0; x < GRID2_W; x++) {
                float posx = 0.2f + x * 0.03f, posy = current_scroll - (section_start + 0.66f + y * 0.052f);
                char c[5] = {'\0', '\0', '\0', '\0', '\0'};
                codepoint_to_utf8(c, grid2[y * GRID2_W + x]);
                neko_fontcache_draw_text(&cache, demo_grid2_font, c, posx, posy, 1.0f / this->screen_w, 1.0f / this->screen_h);
            }
        }
        for (int y = 0; y < GRID3_H; y++) {
            for (int x = 0; x < GRID3_W; x++) {
                float posx = 0.45f + x * 0.02f, posy = current_scroll - (section_start + 0.64f + y * 0.034f);
                char c[5] = {'\0', '\0', '\0', '\0', '\0'};
                codepoint_to_utf8(c, grid3[y * GRID3_W + x]);
                neko_fontcache_draw_text(&cache, demo_grid3_font, c, posx, posy, 1.0f / this->screen_w, 1.0f / this->screen_h);
            }
        }
    }

    // Smooth scrolling!
    // printf("%f\n", current_scroll);
    static float mouse_down_pos = -1.0f, mouse_down_scroll = -1.0f, mouse_prev_pos, scroll_velocity = 0.0f;
    if (window->mouseButton[(int)TinyWindow::mouseButton_t::left] == TinyWindow::buttonState_t::down) {
        if (mouse_down_pos < 0.0f) {
            mouse_down_pos = mouse_prev_pos = window->mousePosition.y;
            mouse_down_scroll = current_scroll;
        }
        demo_autoscroll = false;
        current_scroll = mouse_down_scroll + (mouse_down_pos - window->mousePosition.y) / this->screen_h;

        float new_scroll_velocity = (mouse_prev_pos - window->mousePosition.y) / this->screen_h;
        scroll_velocity = scroll_velocity * 0.2f + new_scroll_velocity * 0.8f;
        mouse_prev_pos = window->mousePosition.y;
    } else {
        scroll_velocity += mouse_scroll * 0.05f;
        mouse_down_pos = -1.0f;
        float substep_dT = dT / 4.0f;
        for (int i = 0; i < 4; i++) {
            scroll_velocity *= exp(-3.5f * substep_dT);
            current_scroll += scroll_velocity * substep_dT * 18.0f;
        }
        if (demo_autoscroll) current_scroll += 0.01f * dT;
        mouse_scroll = 0;
    }

#endif
}

void text_renderer::push(const std::string &text, const font_index font, const f32 x, const f32 y) { push(text, font, calc_pos(x, y)); }

void text_renderer::resize(neko_vec2 size) {
    screen_w = size.x;
    screen_h = size.y;
}

// 将屏幕窗口坐标转换为 fontcache 绘制坐标
neko_vec2 text_renderer::calc_pos(f32 x, f32 y) const {
    f32 tmpx = x / screen_w;
    f32 tmpy = y / screen_h;
    return neko_vec2{tmpx, 1.0f - tmpy};
}

void test_plist() {
    neko_fontcache_poollist plist;
    neko_fontcache_poollist_init(plist, 8);

    for (int repeat = 0; repeat < 128; repeat++) {
        neko_fontcache_poollist_push_front(plist, 31337);
        assert(plist.size == 1);
        neko_fontcache_poollist_push_front(plist, 31338);
        assert(plist.size == 2);
        neko_fontcache_poollist_push_front(plist, 31339);
        assert(plist.size == 3);
        auto v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 31337);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 31338);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 31339);
        assert(plist.size == 0);

        neko_fontcache_poollist_push_front(plist, 1337);
        assert(plist.size == 1);
        neko_fontcache_poollist_push_front(plist, 1338);
        assert(plist.size == 2);
        neko_fontcache_poollist_push_front(plist, 1339);
        assert(plist.size == 3);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 1337);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 1338);
        neko_fontcache_poollist_push_front(plist, 1339);
        assert(plist.size == 2);
        neko_fontcache_poollist_push_front(plist, 1339);
        assert(plist.size == 3);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 1339);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 1339);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 1339);
        assert(plist.size == 0);

        neko_fontcache_poollist_push_front(plist, 10);
        neko_fontcache_poollist_push_front(plist, 11);
        neko_fontcache_poollist_push_front(plist, 12);
        neko_fontcache_poollist_push_front(plist, 13);
        auto itr = plist.front;

        neko_fontcache_poollist_push_front(plist, 14);
        neko_fontcache_poollist_push_front(plist, 15);
        neko_fontcache_poollist_push_front(plist, 16);
        neko_fontcache_poollist_push_front(plist, 17);
        assert(plist.size == 8);

        neko_fontcache_poollist_erase(plist, itr);
        assert(plist.size == 7);
        neko_fontcache_poollist_erase(plist, plist.front);
        assert(plist.size == 6);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 10);
        assert(plist.size == 5);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 11);
        assert(plist.size == 4);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 12);
        assert(plist.size == 3);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 14);
        assert(plist.size == 2);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 15);
        assert(plist.size == 1);
        v = neko_fontcache_poollist_pop_back(plist);
        assert(v == 16);
        assert(plist.size == 0);
    }
}

void text_renderer::__init() {

    fontcache_shader_render_glyph = ME_font_compile_shader(vs_source_shared, fs_source_render_glyph);
    fontcache_shader_blit_atlas = ME_font_compile_shader(vs_source_shared, fs_source_blit_atlas);
    fontcache_shader_draw_text = ME_font_compile_shader(vs_source_draw_text, fs_source_draw_text);

    setup_fbo();

#ifndef NEKO_FONTCACHE_DEBUGPRINT
    // const int numGlyphs = 1024;
    //{
    //     Timer timer;
    //     timer.start();
    //     for (int i = 0; i < numGlyphs; i++) {
    //         // neko_fontcache_cache_glyph(&cache, id, 0x3091);
    //         // neko_fontcache_cache_glyph(&cache, id, 'o');
    //     }
    //     neko_fontcache_flush_drawlist(&cache);
    //     timer.stop();
    //     METADOT_BUG(std::format("[UI] fontcache_cache_glyph() benchmark: total {0:.6f} ms for {1} glyphs, per-glyph {2} ms", timer.get(), numGlyphs, timer.get() / numGlyphs).c_str());
    // }
#endif  // NEKO_FONTCACHE_DEBUGPRINT
}

void text_renderer::__end() { neko_fontcache_shutdown(&cache); }

}  // namespace neko