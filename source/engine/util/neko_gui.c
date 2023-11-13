
#include "engine/util/neko_gui.h"

#include "engine/util/neko_asset.h"

#ifndef NEKO_GUI_DOUBLE_CLICK_LO
#define NEKO_GUI_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NEKO_GUI_DOUBLE_CLICK_HI
#define NEKO_GUI_DOUBLE_CLICK_HI 0.2
#endif

typedef struct neko_gui_vertex_t {
    float position[2];
    float uv[2];
    neko_gui_byte col[4];
} neko_gui_vertex_t;

#ifdef NEKO_PLATFORM_WEB
#define NEKO_GUI_SHADER_VERSION "#version 300 es\n"
#else
#define NEKO_GUI_SHADER_VERSION "#version 330 core\n"
#endif

enum neko_gui_style_theme { THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK };

neko_global size_t g_nui_mem_usage = 0;

neko_private(void*) __neko_gui_malloc(size_t sz, void* user_data) { return __neko_mem_safe_alloc((sz), (char*)__FILE__, __LINE__, &g_nui_mem_usage); }

neko_private(void) __neko_gui_free(void* ptr, void* user_data) { __neko_mem_safe_free(ptr, &g_nui_mem_usage); }

size_t __neko_gui_meminuse() { return g_nui_mem_usage; }

NEKO_API_DECL void set_style(struct neko_gui_context* ctx, enum neko_gui_style_theme theme) {
    struct neko_gui_color table[NEKO_GUI_COLOR_COUNT];
    if (theme == THEME_WHITE) {
        table[NEKO_GUI_COLOR_TEXT] = neko_gui_rgba(70, 70, 70, 255);
        table[NEKO_GUI_COLOR_WINDOW] = neko_gui_rgba(175, 175, 175, 255);
        table[NEKO_GUI_COLOR_HEADER] = neko_gui_rgba(175, 175, 175, 255);
        table[NEKO_GUI_COLOR_BORDER] = neko_gui_rgba(0, 0, 0, 255);
        table[NEKO_GUI_COLOR_BUTTON] = neko_gui_rgba(185, 185, 185, 255);
        table[NEKO_GUI_COLOR_BUTTON_HOVER] = neko_gui_rgba(170, 170, 170, 255);
        table[NEKO_GUI_COLOR_BUTTON_ACTIVE] = neko_gui_rgba(160, 160, 160, 255);
        table[NEKO_GUI_COLOR_TOGGLE] = neko_gui_rgba(150, 150, 150, 255);
        table[NEKO_GUI_COLOR_TOGGLE_HOVER] = neko_gui_rgba(120, 120, 120, 255);
        table[NEKO_GUI_COLOR_TOGGLE_CURSOR] = neko_gui_rgba(175, 175, 175, 255);
        table[NEKO_GUI_COLOR_SELECT] = neko_gui_rgba(190, 190, 190, 255);
        table[NEKO_GUI_COLOR_SELECT_ACTIVE] = neko_gui_rgba(175, 175, 175, 255);
        table[NEKO_GUI_COLOR_SLIDER] = neko_gui_rgba(190, 190, 190, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR] = neko_gui_rgba(80, 80, 80, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER] = neko_gui_rgba(70, 70, 70, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_gui_rgba(60, 60, 60, 255);
        table[NEKO_GUI_COLOR_PROPERTY] = neko_gui_rgba(175, 175, 175, 255);
        table[NEKO_GUI_COLOR_EDIT] = neko_gui_rgba(150, 150, 150, 255);
        table[NEKO_GUI_COLOR_EDIT_CURSOR] = neko_gui_rgba(0, 0, 0, 255);
        table[NEKO_GUI_COLOR_COMBO] = neko_gui_rgba(175, 175, 175, 255);
        table[NEKO_GUI_COLOR_CHART] = neko_gui_rgba(160, 160, 160, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR] = neko_gui_rgba(45, 45, 45, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_gui_rgba(255, 0, 0, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR] = neko_gui_rgba(180, 180, 180, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR] = neko_gui_rgba(140, 140, 140, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_gui_rgba(150, 150, 150, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_gui_rgba(160, 160, 160, 255);
        table[NEKO_GUI_COLOR_TAB_HEADER] = neko_gui_rgba(180, 180, 180, 255);
        neko_gui_style_from_table(ctx, table);
    } else if (theme == THEME_RED) {
        table[NEKO_GUI_COLOR_TEXT] = neko_gui_rgba(190, 190, 190, 255);
        table[NEKO_GUI_COLOR_WINDOW] = neko_gui_rgba(30, 33, 40, 215);
        table[NEKO_GUI_COLOR_HEADER] = neko_gui_rgba(181, 45, 69, 220);
        table[NEKO_GUI_COLOR_BORDER] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_BUTTON] = neko_gui_rgba(181, 45, 69, 255);
        table[NEKO_GUI_COLOR_BUTTON_HOVER] = neko_gui_rgba(190, 50, 70, 255);
        table[NEKO_GUI_COLOR_BUTTON_ACTIVE] = neko_gui_rgba(195, 55, 75, 255);
        table[NEKO_GUI_COLOR_TOGGLE] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_TOGGLE_HOVER] = neko_gui_rgba(45, 60, 60, 255);
        table[NEKO_GUI_COLOR_TOGGLE_CURSOR] = neko_gui_rgba(181, 45, 69, 255);
        table[NEKO_GUI_COLOR_SELECT] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_SELECT_ACTIVE] = neko_gui_rgba(181, 45, 69, 255);
        table[NEKO_GUI_COLOR_SLIDER] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR] = neko_gui_rgba(181, 45, 69, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER] = neko_gui_rgba(186, 50, 74, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_gui_rgba(191, 55, 79, 255);
        table[NEKO_GUI_COLOR_PROPERTY] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_EDIT] = neko_gui_rgba(51, 55, 67, 225);
        table[NEKO_GUI_COLOR_EDIT_CURSOR] = neko_gui_rgba(190, 190, 190, 255);
        table[NEKO_GUI_COLOR_COMBO] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_CHART] = neko_gui_rgba(51, 55, 67, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR] = neko_gui_rgba(170, 40, 60, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_gui_rgba(255, 0, 0, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR] = neko_gui_rgba(30, 33, 40, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR] = neko_gui_rgba(64, 84, 95, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_gui_rgba(70, 90, 100, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_gui_rgba(75, 95, 105, 255);
        table[NEKO_GUI_COLOR_TAB_HEADER] = neko_gui_rgba(181, 45, 69, 220);
        neko_gui_style_from_table(ctx, table);
    } else if (theme == THEME_BLUE) {
        table[NEKO_GUI_COLOR_TEXT] = neko_gui_rgba(20, 20, 20, 255);
        table[NEKO_GUI_COLOR_WINDOW] = neko_gui_rgba(202, 212, 214, 215);
        table[NEKO_GUI_COLOR_HEADER] = neko_gui_rgba(137, 182, 224, 220);
        table[NEKO_GUI_COLOR_BORDER] = neko_gui_rgba(140, 159, 173, 255);
        table[NEKO_GUI_COLOR_BUTTON] = neko_gui_rgba(137, 182, 224, 255);
        table[NEKO_GUI_COLOR_BUTTON_HOVER] = neko_gui_rgba(142, 187, 229, 255);
        table[NEKO_GUI_COLOR_BUTTON_ACTIVE] = neko_gui_rgba(147, 192, 234, 255);
        table[NEKO_GUI_COLOR_TOGGLE] = neko_gui_rgba(177, 210, 210, 255);
        table[NEKO_GUI_COLOR_TOGGLE_HOVER] = neko_gui_rgba(182, 215, 215, 255);
        table[NEKO_GUI_COLOR_TOGGLE_CURSOR] = neko_gui_rgba(137, 182, 224, 255);
        table[NEKO_GUI_COLOR_SELECT] = neko_gui_rgba(177, 210, 210, 255);
        table[NEKO_GUI_COLOR_SELECT_ACTIVE] = neko_gui_rgba(137, 182, 224, 255);
        table[NEKO_GUI_COLOR_SLIDER] = neko_gui_rgba(177, 210, 210, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR] = neko_gui_rgba(137, 182, 224, 245);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER] = neko_gui_rgba(142, 188, 229, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_gui_rgba(147, 193, 234, 255);
        table[NEKO_GUI_COLOR_PROPERTY] = neko_gui_rgba(210, 210, 210, 255);
        table[NEKO_GUI_COLOR_EDIT] = neko_gui_rgba(210, 210, 210, 225);
        table[NEKO_GUI_COLOR_EDIT_CURSOR] = neko_gui_rgba(20, 20, 20, 255);
        table[NEKO_GUI_COLOR_COMBO] = neko_gui_rgba(210, 210, 210, 255);
        table[NEKO_GUI_COLOR_CHART] = neko_gui_rgba(210, 210, 210, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR] = neko_gui_rgba(137, 182, 224, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_gui_rgba(255, 0, 0, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR] = neko_gui_rgba(190, 200, 200, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR] = neko_gui_rgba(64, 84, 95, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_gui_rgba(70, 90, 100, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_gui_rgba(75, 95, 105, 255);
        table[NEKO_GUI_COLOR_TAB_HEADER] = neko_gui_rgba(156, 193, 220, 255);
        neko_gui_style_from_table(ctx, table);
    } else if (theme == THEME_DARK) {
        table[NEKO_GUI_COLOR_TEXT] = neko_gui_rgba(210, 210, 210, 255);
        table[NEKO_GUI_COLOR_WINDOW] = neko_gui_rgba(57, 67, 71, 215);
        table[NEKO_GUI_COLOR_HEADER] = neko_gui_rgba(51, 51, 56, 220);
        table[NEKO_GUI_COLOR_BORDER] = neko_gui_rgba(46, 46, 46, 255);
        table[NEKO_GUI_COLOR_BUTTON] = neko_gui_rgba(48, 83, 111, 255);
        table[NEKO_GUI_COLOR_BUTTON_HOVER] = neko_gui_rgba(58, 93, 121, 255);
        table[NEKO_GUI_COLOR_BUTTON_ACTIVE] = neko_gui_rgba(63, 98, 126, 255);
        table[NEKO_GUI_COLOR_TOGGLE] = neko_gui_rgba(50, 58, 61, 255);
        table[NEKO_GUI_COLOR_TOGGLE_HOVER] = neko_gui_rgba(45, 53, 56, 255);
        table[NEKO_GUI_COLOR_TOGGLE_CURSOR] = neko_gui_rgba(48, 83, 111, 255);
        table[NEKO_GUI_COLOR_SELECT] = neko_gui_rgba(57, 67, 61, 255);
        table[NEKO_GUI_COLOR_SELECT_ACTIVE] = neko_gui_rgba(48, 83, 111, 255);
        table[NEKO_GUI_COLOR_SLIDER] = neko_gui_rgba(50, 58, 61, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR] = neko_gui_rgba(48, 83, 111, 245);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER] = neko_gui_rgba(53, 88, 116, 255);
        table[NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE] = neko_gui_rgba(58, 93, 121, 255);
        table[NEKO_GUI_COLOR_PROPERTY] = neko_gui_rgba(50, 58, 61, 255);
        table[NEKO_GUI_COLOR_EDIT] = neko_gui_rgba(50, 58, 61, 225);
        table[NEKO_GUI_COLOR_EDIT_CURSOR] = neko_gui_rgba(210, 210, 210, 255);
        table[NEKO_GUI_COLOR_COMBO] = neko_gui_rgba(50, 58, 61, 255);
        table[NEKO_GUI_COLOR_CHART] = neko_gui_rgba(50, 58, 61, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR] = neko_gui_rgba(48, 83, 111, 255);
        table[NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT] = neko_gui_rgba(255, 0, 0, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR] = neko_gui_rgba(50, 58, 61, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR] = neko_gui_rgba(48, 83, 111, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER] = neko_gui_rgba(53, 88, 116, 255);
        table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE] = neko_gui_rgba(58, 93, 121, 255);
        table[NEKO_GUI_COLOR_TAB_HEADER] = neko_gui_rgba(48, 83, 111, 255);
        neko_gui_style_from_table(ctx, table);
    } else {
        neko_gui_style_default(ctx);
    }
}

NEKO_API_DECL void neko_gui_device_create(neko_gui_ctx_t* neko_nui) {
    static const char* neko_gui_vertsrc = NEKO_GUI_SHADER_VERSION
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

    static const char* neko_gui_fragsrc = NEKO_GUI_SHADER_VERSION
            "precision mediump float;\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main(){\n"
            "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

    // Initialize commands
    neko_gui_buffer_init_default(&neko_nui->cmds);

    // Shader source description
    neko_graphics_shader_source_desc_t sources[] = {(neko_graphics_shader_source_desc_t){.type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX, .source = neko_gui_vertsrc},
                                                    (neko_graphics_shader_source_desc_t){.type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = neko_gui_fragsrc}};

    // Create shader
    neko_nui->shader = neko_graphics_shader_create(&(neko_graphics_shader_desc_t){.sources = sources, .size = sizeof(sources), .name = "neko_gui_shader"});

    // Construct sampler buffer
    neko_nui->u_tex = neko_graphics_uniform_create(&(neko_graphics_uniform_desc_t){.name = "Texture", .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D}});

    neko_nui->u_proj = neko_graphics_uniform_create(&(neko_graphics_uniform_desc_t){.name = "ProjMtx",  // Name of uniform (used for linkage)
                                                                                    .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_MAT4}});

    // Construct vertex buffer
    neko_nui->vbo = neko_graphics_vertex_buffer_create(&(neko_graphics_index_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = NULL});

    // Create index buffer
    neko_nui->ibo = neko_graphics_index_buffer_create(&(neko_graphics_index_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = NULL});

    // Vertex attr layout
    neko_graphics_vertex_attribute_desc_t gsneko_gui_vattrs[] = {
            (neko_graphics_vertex_attribute_desc_t){.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "Position"},  // Position
            (neko_graphics_vertex_attribute_desc_t){.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "TexCoord"},  // UV
            (neko_graphics_vertex_attribute_desc_t){.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4, .name = "Color"}       // Color
    };

    // Create pipeline
    neko_nui->pip = neko_graphics_pipeline_create(
            &(neko_graphics_pipeline_desc_t){.raster = {.shader = neko_nui->shader, .index_buffer_element_size = sizeof(uint32_t)},
                                             .blend = {.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD, .src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA, .dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA},
                                             .layout = {.attrs = gsneko_gui_vattrs, .size = sizeof(gsneko_gui_vattrs)}});
}

NEKO_API_DECL struct neko_gui_context* neko_gui_init(neko_gui_ctx_t* neko_nui, uint32_t win_hndl, enum neko_gui_init_state init_state) {
    neko_nui->window_hndl = win_hndl;
    neko_gui_init_default(&neko_nui->neko_gui_ctx, 0);
    neko_nui->neko_gui_ctx.clip.copy = neko_gui_clipboard_copy;
    neko_nui->neko_gui_ctx.clip.paste = neko_gui_clipboard_paste;
    neko_nui->neko_gui_ctx.clip.userdata = neko_gui_handle_ptr(0);
    neko_nui->last_button_click = 0;
    neko_gui_device_create(neko_nui);
    neko_nui->is_double_click_down = neko_gui_false;
    neko_nui->double_click_pos = neko_gui_vec2(0, 0);

    // Tmp data buffers for upload
    neko_nui->tmp_vertex_data = neko_malloc(NEKO_GUI_MAX_VERTEX_BUFFER);
    neko_nui->tmp_index_data = neko_malloc(NEKO_GUI_MAX_INDEX_BUFFER);

    neko_assert(neko_nui->tmp_vertex_data);
    neko_assert(neko_nui->tmp_index_data);

    // Font atlas
    neko_nui->atlas = neko_malloc(sizeof(struct neko_gui_font_atlas));

    // 加载保存的 gui layout
    if (neko_platform_file_exists("gui_layout.nbt")) {
        neko_nui->gui_layout_nbt_tags = neko_nbt_readfile("gui_layout.nbt", NBT_PARSE_FLAG_USE_RAW);
    }

    return &neko_nui->neko_gui_ctx;
}

NEKO_API_DECL void neko_gui_device_upload_atlas(neko_gui_ctx_t* neko_nui, const void* image, int32_t width, int32_t height) {
    // Create font texture
    neko_nui->font_tex = neko_graphics_texture_create(&(neko_graphics_texture_desc_t){.width = width,
                                                                                      .height = height,
                                                                                      .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                                                      .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR,
                                                                                      .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR,
                                                                                      .data = (void*)image});
}

NEKO_API_DECL void neko_gui_font_stash_begin(struct neko_gui_ctx_t* neko_nui, struct neko_gui_font_atlas** atlas) {
    neko_gui_font_atlas_init_default(neko_nui->atlas);
    neko_gui_font_atlas_begin(neko_nui->atlas);
    if (atlas) {
        *atlas = neko_nui->atlas;
    }
}

NEKO_API_DECL void neko_gui_font_stash_end(struct neko_gui_ctx_t* neko_nui) {
    const void* image;
    int32_t w, h;
    image = neko_gui_font_atlas_bake(neko_nui->atlas, &w, &h, NEKO_GUI_FONT_ATLAS_RGBA32);

    // Upload texture (this is where we'll set the font texture resource handle)
    neko_gui_device_upload_atlas(neko_nui, image, w, h);

    // Create nk handle from resource handle
    neko_gui_handle hndl = neko_gui_handle_id((int32_t)neko_nui->font_tex.id);

    neko_gui_font_atlas_end(neko_nui->atlas, hndl, &neko_nui->null);

    if (neko_nui->atlas->default_font) neko_gui_style_set_font(&neko_nui->neko_gui_ctx, &neko_nui->atlas->default_font->handle);
}

NEKO_API_DECL void neko_gui_new_frame(neko_gui_ctx_t* neko_nui) {
    int i;
    int32_t x, y;
    struct neko_gui_context* ctx = &neko_nui->neko_gui_ctx;

    // Cache platform pointer
    neko_platform_t* platform = neko_subsystem(platform);

    // Get window size
    neko_platform_window_size(neko_nui->window_hndl, &neko_nui->width, &neko_nui->height);
    // Get frame buffer size
    neko_platform_framebuffer_size(neko_nui->window_hndl, &neko_nui->display_width, &neko_nui->display_height);

    // Reset wheel
    neko_nui->scroll.x = 0.f;
    neko_nui->scroll.y = 0.f;

    // Calculate fb scale
    neko_nui->fb_scale.x = (float)neko_nui->display_width / (float)neko_nui->width;
    neko_nui->fb_scale.y = (float)neko_nui->display_height / (float)neko_nui->height;

    neko_gui_input_begin(ctx);
    {
        // Poll all events that occured this frame
        neko_platform_event_t evt = neko_default_val();
        while (neko_platform_poll_events(&evt, true)) {
            switch (evt.type) {
                case NEKO_PLATFORM_EVENT_KEY: {
                    switch (evt.key.action) {
                        case NEKO_PLATFORM_KEY_PRESSED: {
                            if (neko_nui->text_len < NEKO_GUI_TEXT_MAX) neko_nui->text[neko_nui->text_len++] = evt.key.codepoint;

                        } break;

                        default:
                            break;
                    }
                } break;

                case NEKO_PLATFORM_EVENT_MOUSE: {
                    // TODO: 23/10/18 检测nui组件获得焦点
                    // if (!(ctx->input.mouse.grab || ctx->input.mouse.grabbed)) break;
                    switch (evt.mouse.action) {
                        case NEKO_PLATFORM_MOUSE_WHEEL: {
                            neko_nui->scroll.x = evt.mouse.wheel.x;
                            neko_nui->scroll.y = evt.mouse.wheel.y;
                        } break;

                        case NEKO_PLATFORM_MOUSE_MOVE: {
                            neko_gui_input_motion(ctx, (int32_t)evt.mouse.move.x, (int32_t)evt.mouse.move.y);
                        } break;

                        default:
                            break;
                    }
                } break;

                default:
                    break;
            }
        }

        for (i = 0; i < neko_nui->text_len; ++i) neko_gui_input_unicode(ctx, neko_nui->text[i]);

        // #ifdef NEKO_GUI_GLFW_GL3_MOUSE_GRABBING
        //     /* optional grabbing behavior */
        //     if (ctx->input.mouse.grab)
        //         glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        //     else if (ctx->input.mouse.ungrab)
        //         glfwSetInputMode(glfw->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // #endif

        neko_gui_input_key(ctx, NEKO_GUI_KEY_DEL, neko_platform_key_pressed(NEKO_KEYCODE_DELETE));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_ENTER, neko_platform_key_pressed(NEKO_KEYCODE_ENTER));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_TAB, neko_platform_key_pressed(NEKO_KEYCODE_TAB));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_BACKSPACE, neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_UP, neko_platform_key_pressed(NEKO_KEYCODE_UP));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_DOWN, neko_platform_key_pressed(NEKO_KEYCODE_DOWN));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_START, neko_platform_key_pressed(NEKO_KEYCODE_HOME));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_END, neko_platform_key_pressed(NEKO_KEYCODE_END));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_SCROLL_START, neko_platform_key_pressed(NEKO_KEYCODE_HOME));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_SCROLL_END, neko_platform_key_pressed(NEKO_KEYCODE_END));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_SCROLL_DOWN, neko_platform_key_pressed(NEKO_KEYCODE_PAGE_DOWN));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_SCROLL_UP, neko_platform_key_pressed(NEKO_KEYCODE_PAGE_UP));
        neko_gui_input_key(ctx, NEKO_GUI_KEY_SHIFT, neko_platform_key_pressed(NEKO_KEYCODE_LEFT_SHIFT) || neko_platform_key_pressed(NEKO_KEYCODE_RIGHT_SHIFT));

        if (neko_platform_key_down(NEKO_KEYCODE_LEFT_CONTROL) || neko_platform_key_down(NEKO_KEYCODE_RIGHT_CONTROL)) {
            neko_gui_input_key(ctx, NEKO_GUI_KEY_COPY, neko_platform_key_pressed(NEKO_KEYCODE_C));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_PASTE, neko_platform_key_pressed(NEKO_KEYCODE_V));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_CUT, neko_platform_key_pressed(NEKO_KEYCODE_X));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_UNDO, neko_platform_key_pressed(NEKO_KEYCODE_Z));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_REDO, neko_platform_key_pressed(NEKO_KEYCODE_R));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_WORD_LEFT, neko_platform_key_pressed(NEKO_KEYCODE_LEFT));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_WORD_RIGHT, neko_platform_key_pressed(NEKO_KEYCODE_RIGHT));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_LINE_START, neko_platform_key_pressed(NEKO_KEYCODE_B));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_TEXT_LINE_END, neko_platform_key_pressed(NEKO_KEYCODE_E));
        } else {
            neko_gui_input_key(ctx, NEKO_GUI_KEY_LEFT, neko_platform_key_pressed(NEKO_KEYCODE_LEFT));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_RIGHT, neko_platform_key_pressed(NEKO_KEYCODE_RIGHT));
            neko_gui_input_key(ctx, NEKO_GUI_KEY_COPY, 0);
            neko_gui_input_key(ctx, NEKO_GUI_KEY_PASTE, 0);
            neko_gui_input_key(ctx, NEKO_GUI_KEY_CUT, 0);
            neko_gui_input_key(ctx, NEKO_GUI_KEY_SHIFT, 0);
        }

        neko_platform_mouse_position(&x, &y);
        neko_gui_input_motion(ctx, (int32_t)x, (int32_t)y);

#ifdef NEKO_GUI_MOUSE_GRABBING
        if (ctx->input.mouse.grabbed) {
            // glfwSetCursorPos(glfw->win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
            ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
            ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
        }
#endif

        // Should swap this over to polling events instead.
        neko_gui_input_button(ctx, NEKO_GUI_BUTTON_LEFT, (int)x, (int)y, neko_platform_mouse_down(NEKO_MOUSE_LBUTTON));
        neko_gui_input_button(ctx, NEKO_GUI_BUTTON_MIDDLE, (int)x, (int)y, neko_platform_mouse_down(NEKO_MOUSE_MBUTTON));
        neko_gui_input_button(ctx, NEKO_GUI_BUTTON_RIGHT, (int)x, (int)y, neko_platform_mouse_down(NEKO_MOUSE_RBUTTON));
        neko_gui_input_button(ctx, NEKO_GUI_BUTTON_DOUBLE, (int)neko_nui->double_click_pos.x, (int)neko_nui->double_click_pos.y, neko_nui->is_double_click_down);
        neko_gui_input_scroll(ctx, neko_nui->scroll);
    }
    neko_gui_input_end(ctx);

    neko_nui->text_len = 0;
}

NEKO_API_DECL void neko_gui_render(neko_gui_ctx_t* neko_nui, neko_command_buffer_t* cb, enum neko_gui_anti_aliasing AA) {
    struct neko_gui_buffer vbuf, ibuf;

    // Projection matrix
    float ortho[4][4] = {
            {2.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, -2.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (float)neko_nui->width;
    ortho[1][1] /= (float)neko_nui->height;
    neko_mat4 m = neko_mat4_elem((float*)ortho);

    // Set up data binds
    neko_graphics_bind_desc_t binds = {.vertex_buffers = {.desc = &(neko_graphics_bind_vertex_buffer_desc_t){.buffer = neko_nui->vbo}},
                                       .index_buffers = {.desc = &(neko_graphics_bind_index_buffer_desc_t){.buffer = neko_nui->ibo}},
                                       .uniforms = {.desc = &(neko_graphics_bind_uniform_desc_t){.uniform = neko_nui->u_proj, .data = &m}}};

    neko_assert(neko_nui->tmp_vertex_data);
    neko_assert(neko_nui->tmp_index_data);

    // Convert from command queue into draw list and draw to screen
    {
        const struct neko_gui_draw_command* cmd;
        void *vertices, *indices;
        const neko_gui_draw_index* offset = 0;

        vertices = neko_nui->tmp_vertex_data;
        indices = neko_nui->tmp_index_data;

        // Convert commands into draw lists
        {
            /* fill convert configuration */
            struct neko_gui_convert_config config;
            static const struct neko_gui_draw_vertex_layout_element vertex_layout[] = {{NEKO_GUI_VERTEX_POSITION, NEKO_GUI_FORMAT_FLOAT, NEKO_GUI_OFFSETOF(struct neko_gui_vertex_t, position)},
                                                                                       {NEKO_GUI_VERTEX_TEXCOORD, NEKO_GUI_FORMAT_FLOAT, NEKO_GUI_OFFSETOF(struct neko_gui_vertex_t, uv)},
                                                                                       {NEKO_GUI_VERTEX_COLOR, NEKO_GUI_FORMAT_R8G8B8A8, NEKO_GUI_OFFSETOF(struct neko_gui_vertex_t, col)},
                                                                                       {NEKO_GUI_VERTEX_LAYOUT_END}};
            NEKO_GUI_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct neko_gui_vertex_t);
            config.vertex_alignment = NEKO_GUI_ALIGNOF(struct neko_gui_vertex_t);
            config.tex_null = neko_nui->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            // Setup buffers to load vertices and indices
            neko_gui_buffer_init_fixed(&vbuf, vertices, (size_t)NEKO_GUI_MAX_VERTEX_BUFFER);
            neko_gui_buffer_init_fixed(&ibuf, indices, (size_t)NEKO_GUI_MAX_INDEX_BUFFER);
            neko_gui_convert(&neko_nui->neko_gui_ctx, &neko_nui->cmds, &vbuf, &ibuf, &config);
        }

        // Request update vertex data
        neko_graphics_vertex_buffer_request_update(cb, neko_nui->vbo,
                                                   &(neko_graphics_vertex_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = vertices, .size = NEKO_GUI_MAX_VERTEX_BUFFER});

        // Request update index data
        neko_graphics_index_buffer_request_update(cb, neko_nui->ibo,
                                                  &(neko_graphics_index_buffer_desc_t){.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM, .data = indices, .size = NEKO_GUI_MAX_INDEX_BUFFER});

        // Render pass
        neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);

        // Bind pipeline for nuklear
        neko_graphics_pipeline_bind(cb, neko_nui->pip);

        // Set viewport
        neko_graphics_set_viewport(cb, 0, 0, (uint32_t)neko_nui->display_width, (uint32_t)neko_nui->display_height);

        // Global bindings for pipeline
        neko_graphics_apply_bindings(cb, &binds);

        // Iterate and draw all commands
        neko_gui_draw_foreach(cmd, &neko_nui->neko_gui_ctx, &neko_nui->cmds) {
            if (!cmd->elem_count) continue;

            // Grab handle from command texture id
            neko_handle(neko_graphics_texture_t) tex = neko_handle_create(neko_graphics_texture_t, cmd->texture.id);

            // Bind texture
            neko_graphics_bind_desc_t sbind = {.uniforms = {.desc = &(neko_graphics_bind_uniform_desc_t){.uniform = neko_nui->u_tex, .data = &tex, .binding = 0}}};

            // Bind individual texture binding
            neko_graphics_apply_bindings(cb, &sbind);

            // Set view scissor
            neko_graphics_set_view_scissor(cb, (uint32_t)(cmd->clip_rect.x * neko_nui->fb_scale.x), (uint32_t)((neko_nui->height - (cmd->clip_rect.y + cmd->clip_rect.h)) * neko_nui->fb_scale.y),
                                           (uint32_t)(cmd->clip_rect.w * neko_nui->fb_scale.x), (uint32_t)(cmd->clip_rect.h * neko_nui->fb_scale.y));

            // Draw elements
            neko_graphics_draw(cb, &(neko_graphics_draw_desc_t){.start = (size_t)offset, .count = (uint32_t)cmd->elem_count});

            // Increment offset for commands
            offset += cmd->elem_count;
        }

        neko_graphics_renderpass_end(cb);

        // Clear gui info
        neko_gui_clear(&neko_nui->neko_gui_ctx);
        neko_gui_buffer_clear(&neko_nui->cmds);
    }
}

NEKO_GUI_INTERN void neko_gui_clipboard_paste(neko_gui_handle usr, struct neko_gui_text_edit* edit) {
    // struct neko_gui_glfw* glfw = usr.ptr;
    // const char *text = glfwGetClipboardString(glfw->win);
    // if (text) neko_gui_textedit_paste(edit, text, neko_gui_strlen(text));
    // (void)usr;
}

NEKO_GUI_INTERN void neko_gui_clipboard_copy(neko_gui_handle usr, const char* text, int32_t len) {
    // char *str = 0;
    // if (!len) return;
    // str = (char*)malloc((size_t)len+1);
    // if (!str) return;
    // memcpy(str, text, (size_t)len);
    // str[len] = '\0';
    // struct neko_gui_glfw* glfw = usr.ptr;
    // glfwSetClipboardString(glfw->win, str);
    // free(str);
}

struct neko_gui_rect neko_gui_layout_get_bounds(neko_gui_ctx_t* neko_nui, const char* name) { return neko_gui_layout_get_bounds_ex(neko_nui, name, neko_gui_rect(400, 200, 450, 400)); }

struct neko_gui_rect neko_gui_layout_get_bounds_ex(neko_gui_ctx_t* neko_nui, const char* name, struct neko_gui_rect default_bounds) {

    if (neko_nui->gui_layout_nbt_tags == NULL || strncmp(neko_nui->gui_layout_nbt_tags->name, "gui_layout", neko_nui->gui_layout_nbt_tags->name_size)) goto default_layout;

    neko_assert(neko_nui->gui_layout_nbt_tags->type == NBT_TYPE_COMPOUND);

    for (size_t i = 0; i < neko_nui->gui_layout_nbt_tags->tag_compound.size; i++) {
        neko_nbt_tag_t* win_tag_level = neko_nui->gui_layout_nbt_tags->tag_compound.value[i];
        if (!strcmp(win_tag_level->name, name)) {
            neko_assert(win_tag_level->type == NBT_TYPE_COMPOUND);
            // for (size_t i = 0; i < win_tag_level->tag_compound.size; i++) {
            //     neko_nbt_tag_t* windows_prop = win_tag_level->tag_compound.value[i];
            // }
            f32 bx = neko_nbt_tag_compound_get(win_tag_level, "bx")->tag_float.value;
            f32 by = neko_nbt_tag_compound_get(win_tag_level, "by")->tag_float.value;
            f32 bw = neko_nbt_tag_compound_get(win_tag_level, "bw")->tag_float.value;
            f32 bh = neko_nbt_tag_compound_get(win_tag_level, "bh")->tag_float.value;

            return neko_gui_rect(bx, by, bw, bh);
        }
    }

default_layout:
    return default_bounds;  // 默认
}

void neko_gui_layout_save(neko_gui_ctx_t* neko_nui) {

    // 释放先前加载的 gui layout
    // if (neko_nui->gui_layout_nbt_tags) neko_nbt_free_tag(neko_nui->gui_layout_nbt_tags);

    struct neko_gui_window* iter;
    if (!&neko_nui->neko_gui_ctx) return;
    iter = neko_nui->neko_gui_ctx.begin;

    neko_nbt_tag_t* tag_level;
    if (!neko_nui->gui_layout_nbt_tags) {
        tag_level = neko_nbt_new_tag_compound();
        neko_nbt_set_tag_name(tag_level, "gui_layout", strlen("gui_layout"));
    } else {
        tag_level = neko_nui->gui_layout_nbt_tags;
    }

    while (iter) {
        if (!(iter->flags & NEKO_GUI_WINDOW_HIDDEN)) {
            // neko_println("%f,%f,%f,%f %s", iter->bounds.x, iter->bounds.y, iter->bounds.w, iter->bounds.h, iter->name_string);

            neko_nbt_tag_t* win_tag_level = neko_nbt_tag_compound_get(tag_level, iter->name_string);
            if (win_tag_level == NULL) {
                win_tag_level = neko_nbt_new_tag_compound();
                neko_nbt_set_tag_name(win_tag_level, iter->name_string, strlen(iter->name_string));
                neko_nbt_tag_compound_append(tag_level, win_tag_level);
            }

#define overwrite_nbt(_tag, _name, _value)                                 \
    neko_nbt_tag_t* tag_##_name = neko_nbt_tag_compound_get(_tag, #_name); \
    if (tag_##_name == NULL) {                                             \
        tag_##_name = neko_nbt_new_tag_float(_value);                      \
        neko_nbt_set_tag_name(tag_##_name, #_name, strlen(#_name));        \
        neko_nbt_tag_compound_append(_tag, tag_##_name);                   \
    } else {                                                               \
        tag_##_name->tag_float.value = _value;                             \
    }

            overwrite_nbt(win_tag_level, bx, iter->bounds.x);
            overwrite_nbt(win_tag_level, by, iter->bounds.y);
            overwrite_nbt(win_tag_level, bw, iter->bounds.w);
            overwrite_nbt(win_tag_level, bh, iter->bounds.h);
        }
        iter = iter->next;
    }

    // neko_nbt_print_tree(tag_level, 2);

    neko_nbt_writefile("gui_layout.nbt", tag_level, NBT_WRITE_FLAG_USE_RAW);

    neko_nbt_free_tag(tag_level);
}