
#include "neko_nui.h"

#ifndef NEKO_NUI_DOUBLE_CLICK_LO
#define NEKO_NUI_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NEKO_NUI_DOUBLE_CLICK_HI
#define NEKO_NUI_DOUBLE_CLICK_HI 0.2
#endif

typedef struct neko_nui_vertex_t {
    float position[2];
    float uv[2];
    neko_nui_byte col[4];
} neko_nui_vertex_t;

#ifdef NEKO_PLATFORM_WEB
#define NEKO_NUI_SHADER_VERSION "#version 300 es\n"
#else
#define NEKO_NUI_SHADER_VERSION "#version 330 core\n"
#endif

enum neko_nui_style_theme { THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK };

NEKO_API_DECL void set_style(struct neko_nui_context* ctx, enum neko_nui_style_theme theme) {
    struct neko_nui_color table[NEKO_NUI_COLOR_COUNT];
    if (theme == THEME_WHITE) {
        table[NEKO_NUI_COLOR_TEXT] = neko_nui_rgba(70, 70, 70, 255);
        table[NEKO_NUI_COLOR_WINDOW] = neko_nui_rgba(175, 175, 175, 255);
        table[NEKO_NUI_COLOR_HEADER] = neko_nui_rgba(175, 175, 175, 255);
        table[NEKO_NUI_COLOR_BORDER] = neko_nui_rgba(0, 0, 0, 255);
        table[NEKO_NUI_COLOR_BUTTON] = neko_nui_rgba(185, 185, 185, 255);
        table[NEKO_NUI_COLOR_BUTTON_HOVER] = neko_nui_rgba(170, 170, 170, 255);
        table[NEKO_NUI_COLOR_BUTTON_ACTIVE] = neko_nui_rgba(160, 160, 160, 255);
        table[NEKO_NUI_COLOR_TOGGLE] = neko_nui_rgba(150, 150, 150, 255);
        table[NEKO_NUI_COLOR_TOGGLE_HOVER] = neko_nui_rgba(120, 120, 120, 255);
        table[NEKO_NUI_COLOR_TOGGLE_CURSOR] = neko_nui_rgba(175, 175, 175, 255);
        table[NEKO_NUI_COLOR_SELECT] = neko_nui_rgba(190, 190, 190, 255);
        table[NEKO_NUI_COLOR_SELECT_ACTIVE] = neko_nui_rgba(175, 175, 175, 255);
        table[NEKO_NUI_COLOR_SLIDER] = neko_nui_rgba(190, 190, 190, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR] = neko_nui_rgba(80, 80, 80, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_HOVER] = neko_nui_rgba(70, 70, 70, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_nui_rgba(60, 60, 60, 255);
        table[NEKO_NUI_COLOR_PROPERTY] = neko_nui_rgba(175, 175, 175, 255);
        table[NEKO_NUI_COLOR_EDIT] = neko_nui_rgba(150, 150, 150, 255);
        table[NEKO_NUI_COLOR_EDIT_CURSOR] = neko_nui_rgba(0, 0, 0, 255);
        table[NEKO_NUI_COLOR_COMBO] = neko_nui_rgba(175, 175, 175, 255);
        table[NEKO_NUI_COLOR_CHART] = neko_nui_rgba(160, 160, 160, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR] = neko_nui_rgba(45, 45, 45, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_nui_rgba(255, 0, 0, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR] = neko_nui_rgba(180, 180, 180, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR] = neko_nui_rgba(140, 140, 140, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_nui_rgba(150, 150, 150, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_nui_rgba(160, 160, 160, 255);
        table[NEKO_NUI_COLOR_TAB_HEADER] = neko_nui_rgba(180, 180, 180, 255);
        neko_nui_style_from_table(ctx, table);
    } else if (theme == THEME_RED) {
        table[NEKO_NUI_COLOR_TEXT] = neko_nui_rgba(190, 190, 190, 255);
        table[NEKO_NUI_COLOR_WINDOW] = neko_nui_rgba(30, 33, 40, 215);
        table[NEKO_NUI_COLOR_HEADER] = neko_nui_rgba(181, 45, 69, 220);
        table[NEKO_NUI_COLOR_BORDER] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_BUTTON] = neko_nui_rgba(181, 45, 69, 255);
        table[NEKO_NUI_COLOR_BUTTON_HOVER] = neko_nui_rgba(190, 50, 70, 255);
        table[NEKO_NUI_COLOR_BUTTON_ACTIVE] = neko_nui_rgba(195, 55, 75, 255);
        table[NEKO_NUI_COLOR_TOGGLE] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_TOGGLE_HOVER] = neko_nui_rgba(45, 60, 60, 255);
        table[NEKO_NUI_COLOR_TOGGLE_CURSOR] = neko_nui_rgba(181, 45, 69, 255);
        table[NEKO_NUI_COLOR_SELECT] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_SELECT_ACTIVE] = neko_nui_rgba(181, 45, 69, 255);
        table[NEKO_NUI_COLOR_SLIDER] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR] = neko_nui_rgba(181, 45, 69, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_HOVER] = neko_nui_rgba(186, 50, 74, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_nui_rgba(191, 55, 79, 255);
        table[NEKO_NUI_COLOR_PROPERTY] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_EDIT] = neko_nui_rgba(51, 55, 67, 225);
        table[NEKO_NUI_COLOR_EDIT_CURSOR] = neko_nui_rgba(190, 190, 190, 255);
        table[NEKO_NUI_COLOR_COMBO] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_CHART] = neko_nui_rgba(51, 55, 67, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR] = neko_nui_rgba(170, 40, 60, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_nui_rgba(255, 0, 0, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR] = neko_nui_rgba(30, 33, 40, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR] = neko_nui_rgba(64, 84, 95, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_nui_rgba(70, 90, 100, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_nui_rgba(75, 95, 105, 255);
        table[NEKO_NUI_COLOR_TAB_HEADER] = neko_nui_rgba(181, 45, 69, 220);
        neko_nui_style_from_table(ctx, table);
    } else if (theme == THEME_BLUE) {
        table[NEKO_NUI_COLOR_TEXT] = neko_nui_rgba(20, 20, 20, 255);
        table[NEKO_NUI_COLOR_WINDOW] = neko_nui_rgba(202, 212, 214, 215);
        table[NEKO_NUI_COLOR_HEADER] = neko_nui_rgba(137, 182, 224, 220);
        table[NEKO_NUI_COLOR_BORDER] = neko_nui_rgba(140, 159, 173, 255);
        table[NEKO_NUI_COLOR_BUTTON] = neko_nui_rgba(137, 182, 224, 255);
        table[NEKO_NUI_COLOR_BUTTON_HOVER] = neko_nui_rgba(142, 187, 229, 255);
        table[NEKO_NUI_COLOR_BUTTON_ACTIVE] = neko_nui_rgba(147, 192, 234, 255);
        table[NEKO_NUI_COLOR_TOGGLE] = neko_nui_rgba(177, 210, 210, 255);
        table[NEKO_NUI_COLOR_TOGGLE_HOVER] = neko_nui_rgba(182, 215, 215, 255);
        table[NEKO_NUI_COLOR_TOGGLE_CURSOR] = neko_nui_rgba(137, 182, 224, 255);
        table[NEKO_NUI_COLOR_SELECT] = neko_nui_rgba(177, 210, 210, 255);
        table[NEKO_NUI_COLOR_SELECT_ACTIVE] = neko_nui_rgba(137, 182, 224, 255);
        table[NEKO_NUI_COLOR_SLIDER] = neko_nui_rgba(177, 210, 210, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR] = neko_nui_rgba(137, 182, 224, 245);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_HOVER] = neko_nui_rgba(142, 188, 229, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_nui_rgba(147, 193, 234, 255);
        table[NEKO_NUI_COLOR_PROPERTY] = neko_nui_rgba(210, 210, 210, 255);
        table[NEKO_NUI_COLOR_EDIT] = neko_nui_rgba(210, 210, 210, 225);
        table[NEKO_NUI_COLOR_EDIT_CURSOR] = neko_nui_rgba(20, 20, 20, 255);
        table[NEKO_NUI_COLOR_COMBO] = neko_nui_rgba(210, 210, 210, 255);
        table[NEKO_NUI_COLOR_CHART] = neko_nui_rgba(210, 210, 210, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR] = neko_nui_rgba(137, 182, 224, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_nui_rgba(255, 0, 0, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR] = neko_nui_rgba(190, 200, 200, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR] = neko_nui_rgba(64, 84, 95, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_nui_rgba(70, 90, 100, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_nui_rgba(75, 95, 105, 255);
        table[NEKO_NUI_COLOR_TAB_HEADER] = neko_nui_rgba(156, 193, 220, 255);
        neko_nui_style_from_table(ctx, table);
    } else if (theme == THEME_DARK) {
        table[NEKO_NUI_COLOR_TEXT] = neko_nui_rgba(210, 210, 210, 255);
        table[NEKO_NUI_COLOR_WINDOW] = neko_nui_rgba(57, 67, 71, 215);
        table[NEKO_NUI_COLOR_HEADER] = neko_nui_rgba(51, 51, 56, 220);
        table[NEKO_NUI_COLOR_BORDER] = neko_nui_rgba(46, 46, 46, 255);
        table[NEKO_NUI_COLOR_BUTTON] = neko_nui_rgba(48, 83, 111, 255);
        table[NEKO_NUI_COLOR_BUTTON_HOVER] = neko_nui_rgba(58, 93, 121, 255);
        table[NEKO_NUI_COLOR_BUTTON_ACTIVE] = neko_nui_rgba(63, 98, 126, 255);
        table[NEKO_NUI_COLOR_TOGGLE] = neko_nui_rgba(50, 58, 61, 255);
        table[NEKO_NUI_COLOR_TOGGLE_HOVER] = neko_nui_rgba(45, 53, 56, 255);
        table[NEKO_NUI_COLOR_TOGGLE_CURSOR] = neko_nui_rgba(48, 83, 111, 255);
        table[NEKO_NUI_COLOR_SELECT] = neko_nui_rgba(57, 67, 61, 255);
        table[NEKO_NUI_COLOR_SELECT_ACTIVE] = neko_nui_rgba(48, 83, 111, 255);
        table[NEKO_NUI_COLOR_SLIDER] = neko_nui_rgba(50, 58, 61, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR] = neko_nui_rgba(48, 83, 111, 245);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_HOVER] = neko_nui_rgba(53, 88, 116, 255);
        table[NEKO_NUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_nui_rgba(58, 93, 121, 255);
        table[NEKO_NUI_COLOR_PROPERTY] = neko_nui_rgba(50, 58, 61, 255);
        table[NEKO_NUI_COLOR_EDIT] = neko_nui_rgba(50, 58, 61, 225);
        table[NEKO_NUI_COLOR_EDIT_CURSOR] = neko_nui_rgba(210, 210, 210, 255);
        table[NEKO_NUI_COLOR_COMBO] = neko_nui_rgba(50, 58, 61, 255);
        table[NEKO_NUI_COLOR_CHART] = neko_nui_rgba(50, 58, 61, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR] = neko_nui_rgba(48, 83, 111, 255);
        table[NEKO_NUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_nui_rgba(255, 0, 0, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR] = neko_nui_rgba(50, 58, 61, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR] = neko_nui_rgba(48, 83, 111, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_nui_rgba(53, 88, 116, 255);
        table[NEKO_NUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_nui_rgba(58, 93, 121, 255);
        table[NEKO_NUI_COLOR_TAB_HEADER] = neko_nui_rgba(48, 83, 111, 255);
        neko_nui_style_from_table(ctx, table);
    } else {
        neko_nui_style_default(ctx);
    }
}

NEKO_API_DECL void neko_nui_device_create(neko_nui_ctx_t* neko) {
    static const char* neko_nui_vertsrc = NEKO_NUI_SHADER_VERSION
            "uniform mat4 ProjMtx;\n"
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 TexCoord;\n"
            "layout (location = 2) in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main() {\n"
            "   Frag_UV = TexCoord;\n"
            "   Frag_Color = Color;\n"
            "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
            "}\n";

    static const char* neko_nui_fragsrc = NEKO_NUI_SHADER_VERSION
            "precision mediump float;\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main(){\n"
            "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

    // Initialize commands
    neko_nui_buffer_init_default(&neko->cmds);

    // Shader source description
    neko_graphics_shader_source_desc_t sources[] = {(neko_graphics_shader_source_desc_t){.type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX, .source = neko_nui_vertsrc},
                                                    (neko_graphics_shader_source_desc_t){.type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = neko_nui_fragsrc}};

    // Create shader
    neko->shader = neko_graphics_shader_create(&(neko_graphics_shader_desc_t){.sources = sources, .size = sizeof(sources), .name = "neko_nui_shader"});

    // Construct sampler buffer
    neko->u_tex = neko_graphics_uniform_create(&(neko_graphics_uniform_desc_t){.name = "Texture", .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D}});

    neko->u_proj = neko_graphics_uniform_create(&(neko_graphics_uniform_desc_t){.name = "ProjMtx",  // Name of uniform (used for linkage)
                                                                                .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_MAT4}});

    // Construct vertex buffer
    neko->vbo = neko_graphics_vertex_buffer_create(&(neko_graphics_index_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = NULL});

    // Create index buffer
    neko->ibo = neko_graphics_index_buffer_create(&(neko_graphics_index_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = NULL});

    // Vertex attr layout
    neko_graphics_vertex_attribute_desc_t gsneko_nui_vattrs[] = {
            (neko_graphics_vertex_attribute_desc_t){.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "Position"},  // Position
            (neko_graphics_vertex_attribute_desc_t){.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "TexCoord"},  // UV
            (neko_graphics_vertex_attribute_desc_t){.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4, .name = "Color"}       // Color
    };

    // Create pipeline
    neko->pip = neko_graphics_pipeline_create(
            &(neko_graphics_pipeline_desc_t){.raster = {.shader = neko->shader, .index_buffer_element_size = sizeof(uint32_t)},
                                             .blend = {.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD, .src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA, .dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA},
                                             .layout = {.attrs = gsneko_nui_vattrs, .size = sizeof(gsneko_nui_vattrs)}});
}

NEKO_API_DECL struct neko_nui_context* neko_nui_init(neko_nui_ctx_t* neko, uint32_t win_hndl, enum neko_nui_init_state init_state) {
    neko->window_hndl = win_hndl;
    neko_nui_init_default(&neko->neko_nui_ctx, 0);
    neko->neko_nui_ctx.clip.copy = neko_nui_clipboard_copy;
    neko->neko_nui_ctx.clip.paste = neko_nui_clipboard_paste;
    neko->neko_nui_ctx.clip.userdata = neko_nui_handle_ptr(0);
    neko->last_button_click = 0;
    neko_nui_device_create(neko);
    neko->is_double_click_down = neko_nui_false;
    neko->double_click_pos = neko_nui_vec2(0, 0);

    // Tmp data buffers for upload
    neko->tmp_vertex_data = neko_malloc(NEKO_NUI_MAX_VERTEX_BUFFER);
    neko->tmp_index_data = neko_malloc(NEKO_NUI_MAX_INDEX_BUFFER);

    neko_assert(neko->tmp_vertex_data);
    neko_assert(neko->tmp_index_data);

    // Font atlas
    neko->atlas = neko_malloc(sizeof(struct neko_nui_font_atlas));

    return &neko->neko_nui_ctx;
}

NEKO_API_DECL void neko_nui_device_upload_atlas(neko_nui_ctx_t* neko, const void* image, int32_t width, int32_t height) {
    // Create font texture
    neko->font_tex = neko_graphics_texture_create(&(neko_graphics_texture_desc_t){.width = width,
                                                                                  .height = height,
                                                                                  .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                                                  .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR,
                                                                                  .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR,
                                                                                  .data = (void*)image});
}

NEKO_API_DECL void neko_nui_font_stash_begin(struct neko_nui_ctx_t* neko, struct neko_nui_font_atlas** atlas) {
    neko_nui_font_atlas_init_default(neko->atlas);
    neko_nui_font_atlas_begin(neko->atlas);
    if (atlas) {
        *atlas = neko->atlas;
    }
}

NEKO_API_DECL void neko_nui_font_stash_end(struct neko_nui_ctx_t* neko) {
    const void* image;
    int32_t w, h;
    image = neko_nui_font_atlas_bake(neko->atlas, &w, &h, NEKO_NUI_FONT_ATLAS_RGBA32);

    // Upload texture (this is where we'll set the font texture resource handle)
    neko_nui_device_upload_atlas(neko, image, w, h);

    // Create nk handle from resource handle
    neko_nui_handle hndl = neko_nui_handle_id((int32_t)neko->font_tex.id);

    neko_nui_font_atlas_end(neko->atlas, hndl, &neko->null);

    if (neko->atlas->default_font) neko_nui_style_set_font(&neko->neko_nui_ctx, &neko->atlas->default_font->handle);
}

NEKO_API_DECL void neko_nui_new_frame(neko_nui_ctx_t* neko) {
    int i;
    int32_t x, y;
    struct neko_nui_context* ctx = &neko->neko_nui_ctx;

    // Cache platform pointer
    neko_platform_t* platform = neko_subsystem(platform);

    // Get window size
    neko_platform_window_size(neko->window_hndl, &neko->width, &neko->height);
    // Get frame buffer size
    neko_platform_framebuffer_size(neko->window_hndl, &neko->display_width, &neko->display_height);

    // Reset wheel
    neko->scroll.x = 0.f;
    neko->scroll.y = 0.f;

    // Calculate fb scale
    neko->fb_scale.x = (float)neko->display_width / (float)neko->width;
    neko->fb_scale.y = (float)neko->display_height / (float)neko->height;

    neko_nui_input_begin(ctx);
    {
        // Poll all events that occured this frame
        neko_platform_event_t evt = neko_default_val();
        while (neko_platform_poll_events(&evt, true)) {
            switch (evt.type) {
                case NEKO_PLATFORM_EVENT_KEY: {
                    switch (evt.key.action) {
                        case NEKO_PLATFORM_KEY_PRESSED: {
                            if (neko->text_len < NEKO_NUI_TEXT_MAX) neko->text[neko->text_len++] = evt.key.codepoint;

                        } break;

                        default:
                            break;
                    }
                } break;

                case NEKO_PLATFORM_EVENT_MOUSE: {
                    switch (evt.mouse.action) {
                        case NEKO_PLATFORM_MOUSE_WHEEL: {
                            neko->scroll.x = evt.mouse.wheel.x;
                            neko->scroll.y = evt.mouse.wheel.y;
                        } break;

                        case NEKO_PLATFORM_MOUSE_MOVE: {
                            neko_nui_input_motion(ctx, (int32_t)evt.mouse.move.x, (int32_t)evt.mouse.move.y);
                        } break;

                        default:
                            break;
                    }
                } break;

                default:
                    break;
            }
        }

        for (i = 0; i < neko->text_len; ++i) neko_nui_input_unicode(ctx, neko->text[i]);

        // #ifdef NEKO_NUI_GLFW_GL3_MOUSE_GRABBING
        //     /* optional grabbing behavior */
        //     if (ctx->input.mouse.grab)
        //         glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        //     else if (ctx->input.mouse.ungrab)
        //         glfwSetInputMode(glfw->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // #endif

        neko_nui_input_key(ctx, NEKO_NUI_KEY_DEL, neko_platform_key_pressed(NEKO_KEYCODE_DELETE));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_ENTER, neko_platform_key_pressed(NEKO_KEYCODE_ENTER));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_TAB, neko_platform_key_pressed(NEKO_KEYCODE_TAB));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_BACKSPACE, neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_UP, neko_platform_key_pressed(NEKO_KEYCODE_UP));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_DOWN, neko_platform_key_pressed(NEKO_KEYCODE_DOWN));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_START, neko_platform_key_pressed(NEKO_KEYCODE_HOME));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_END, neko_platform_key_pressed(NEKO_KEYCODE_END));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_SCROLL_START, neko_platform_key_pressed(NEKO_KEYCODE_HOME));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_SCROLL_END, neko_platform_key_pressed(NEKO_KEYCODE_END));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_SCROLL_DOWN, neko_platform_key_pressed(NEKO_KEYCODE_PAGE_DOWN));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_SCROLL_UP, neko_platform_key_pressed(NEKO_KEYCODE_PAGE_UP));
        neko_nui_input_key(ctx, NEKO_NUI_KEY_SHIFT, neko_platform_key_pressed(NEKO_KEYCODE_LEFT_SHIFT) || neko_platform_key_pressed(NEKO_KEYCODE_RIGHT_SHIFT));

        if (neko_platform_key_down(NEKO_KEYCODE_LEFT_CONTROL) || neko_platform_key_down(NEKO_KEYCODE_RIGHT_CONTROL)) {
            neko_nui_input_key(ctx, NEKO_NUI_KEY_COPY, neko_platform_key_pressed(NEKO_KEYCODE_C));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_PASTE, neko_platform_key_pressed(NEKO_KEYCODE_V));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_CUT, neko_platform_key_pressed(NEKO_KEYCODE_X));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_UNDO, neko_platform_key_pressed(NEKO_KEYCODE_Z));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_REDO, neko_platform_key_pressed(NEKO_KEYCODE_R));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_WORD_LEFT, neko_platform_key_pressed(NEKO_KEYCODE_LEFT));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_WORD_RIGHT, neko_platform_key_pressed(NEKO_KEYCODE_RIGHT));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_LINE_START, neko_platform_key_pressed(NEKO_KEYCODE_B));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_TEXT_LINE_END, neko_platform_key_pressed(NEKO_KEYCODE_E));
        } else {
            neko_nui_input_key(ctx, NEKO_NUI_KEY_LEFT, neko_platform_key_pressed(NEKO_KEYCODE_LEFT));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_RIGHT, neko_platform_key_pressed(NEKO_KEYCODE_RIGHT));
            neko_nui_input_key(ctx, NEKO_NUI_KEY_COPY, 0);
            neko_nui_input_key(ctx, NEKO_NUI_KEY_PASTE, 0);
            neko_nui_input_key(ctx, NEKO_NUI_KEY_CUT, 0);
            neko_nui_input_key(ctx, NEKO_NUI_KEY_SHIFT, 0);
        }

        neko_platform_mouse_position(&x, &y);
        neko_nui_input_motion(ctx, (int32_t)x, (int32_t)y);

#ifdef NEKO_NUI_MOUSE_GRABBING
        if (ctx->input.mouse.grabbed) {
            // glfwSetCursorPos(glfw->win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
            ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
            ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
        }
#endif

        // Should swap this over to polling events instead.
        neko_nui_input_button(ctx, NEKO_NUI_BUTTON_LEFT, (int)x, (int)y, neko_platform_mouse_down(NEKO_MOUSE_LBUTTON));
        neko_nui_input_button(ctx, NEKO_NUI_BUTTON_MIDDLE, (int)x, (int)y, neko_platform_mouse_down(NEKO_MOUSE_MBUTTON));
        neko_nui_input_button(ctx, NEKO_NUI_BUTTON_RIGHT, (int)x, (int)y, neko_platform_mouse_down(NEKO_MOUSE_RBUTTON));
        neko_nui_input_button(ctx, NEKO_NUI_BUTTON_DOUBLE, (int)neko->double_click_pos.x, (int)neko->double_click_pos.y, neko->is_double_click_down);
        neko_nui_input_scroll(ctx, neko->scroll);
    }
    neko_nui_input_end(ctx);

    neko->text_len = 0;
}

NEKO_API_DECL void neko_nui_render(neko_nui_ctx_t* neko, neko_command_buffer_t* cb, enum neko_nui_anti_aliasing AA) {
    struct neko_nui_buffer vbuf, ibuf;

    // Projection matrix
    float ortho[4][4] = {
            {2.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, -2.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (float)neko->width;
    ortho[1][1] /= (float)neko->height;
    neko_mat4 m = neko_mat4_elem((float*)ortho);

    // Set up data binds
    neko_graphics_bind_desc_t binds = {.vertex_buffers = {.desc = &(neko_graphics_bind_vertex_buffer_desc_t){.buffer = neko->vbo}},
                                       .index_buffers = {.desc = &(neko_graphics_bind_index_buffer_desc_t){.buffer = neko->ibo}},
                                       .uniforms = {.desc = &(neko_graphics_bind_uniform_desc_t){.uniform = neko->u_proj, .data = &m}}};

    neko_assert(neko->tmp_vertex_data);
    neko_assert(neko->tmp_index_data);

    // Convert from command queue into draw list and draw to screen
    {
        const struct neko_nui_draw_command* cmd;
        void *vertices, *indices;
        const neko_nui_draw_index* offset = 0;

        vertices = neko->tmp_vertex_data;
        indices = neko->tmp_index_data;

        // Convert commands into draw lists
        {
            /* fill convert configuration */
            struct neko_nui_convert_config config;
            static const struct neko_nui_draw_vertex_layout_element vertex_layout[] = {{NEKO_NUI_VERTEX_POSITION, NEKO_NUI_FORMAT_FLOAT, NEKO_NUI_OFFSETOF(struct neko_nui_vertex_t, position)},
                                                                                       {NEKO_NUI_VERTEX_TEXCOORD, NEKO_NUI_FORMAT_FLOAT, NEKO_NUI_OFFSETOF(struct neko_nui_vertex_t, uv)},
                                                                                       {NEKO_NUI_VERTEX_COLOR, NEKO_NUI_FORMAT_R8G8B8A8, NEKO_NUI_OFFSETOF(struct neko_nui_vertex_t, col)},
                                                                                       {NEKO_NUI_VERTEX_LAYOUT_END}};
            NEKO_NUI_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct neko_nui_vertex_t);
            config.vertex_alignment = NEKO_NUI_ALIGNOF(struct neko_nui_vertex_t);
            config.tex_null = neko->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            // Setup buffers to load vertices and indices
            neko_nui_buffer_init_fixed(&vbuf, vertices, (size_t)NEKO_NUI_MAX_VERTEX_BUFFER);
            neko_nui_buffer_init_fixed(&ibuf, indices, (size_t)NEKO_NUI_MAX_INDEX_BUFFER);
            neko_nui_convert(&neko->neko_nui_ctx, &neko->cmds, &vbuf, &ibuf, &config);
        }

        // Request update vertex data
        neko_graphics_vertex_buffer_request_update(cb, neko->vbo,
                                                   &(neko_graphics_vertex_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = vertices, .size = NEKO_NUI_MAX_VERTEX_BUFFER});

        // Request update index data
        neko_graphics_index_buffer_request_update(cb, neko->ibo, &(neko_graphics_index_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = indices, .size = NEKO_NUI_MAX_INDEX_BUFFER});

        // Render pass
        neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);

        // Bind pipeline for nuklear
        neko_graphics_pipeline_bind(cb, neko->pip);

        // Set viewport
        neko_graphics_set_viewport(cb, 0, 0, (uint32_t)neko->display_width, (uint32_t)neko->display_height);

        // Global bindings for pipeline
        neko_graphics_apply_bindings(cb, &binds);

        // Iterate and draw all commands
        neko_nui_draw_foreach(cmd, &neko->neko_nui_ctx, &neko->cmds) {
            if (!cmd->elem_count) continue;

            // Grab handle from command texture id
            neko_handle(neko_graphics_texture_t) tex = neko_handle_create(neko_graphics_texture_t, cmd->texture.id);

            // Bind texture
            neko_graphics_bind_desc_t sbind = {.uniforms = {.desc = &(neko_graphics_bind_uniform_desc_t){.uniform = neko->u_tex, .data = &tex, .binding = 0}}};

            // Bind individual texture binding
            neko_graphics_apply_bindings(cb, &sbind);

            // Set view scissor
            neko_graphics_set_view_scissor(cb, (uint32_t)(cmd->clip_rect.x * neko->fb_scale.x), (uint32_t)((neko->height - (cmd->clip_rect.y + cmd->clip_rect.h)) * neko->fb_scale.y),
                                           (uint32_t)(cmd->clip_rect.w * neko->fb_scale.x), (uint32_t)(cmd->clip_rect.h * neko->fb_scale.y));

            // Draw elements
            neko_graphics_draw(cb, &(neko_graphics_draw_desc_t){.start = (size_t)offset, .count = (uint32_t)cmd->elem_count});

            // Increment offset for commands
            offset += cmd->elem_count;
        }

        neko_graphics_renderpass_end(cb);

        // Clear nuklear info
        neko_nui_clear(&neko->neko_nui_ctx);
        neko_nui_buffer_clear(&neko->cmds);
    }
}

NEKO_NUI_INTERN void neko_nui_clipboard_paste(neko_nui_handle usr, struct neko_nui_text_edit* edit) {
    // struct neko_nui_glfw* glfw = usr.ptr;
    // const char *text = glfwGetClipboardString(glfw->win);
    // if (text) neko_nui_textedit_paste(edit, text, neko_nui_strlen(text));
    // (void)usr;
}

NEKO_NUI_INTERN void neko_nui_clipboard_copy(neko_nui_handle usr, const char* text, int32_t len) {
    // char *str = 0;
    // if (!len) return;
    // str = (char*)malloc((size_t)len+1);
    // if (!str) return;
    // memcpy(str, text, (size_t)len);
    // str[len] = '\0';
    // struct neko_nui_glfw* glfw = usr.ptr;
    // glfwSetClipboardString(glfw->win, str);
    // free(str);
}
