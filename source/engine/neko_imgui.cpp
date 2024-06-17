
#include "neko_imgui.hpp"

void neko_imgui_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    // style.WindowRounding = 5.3f;
    // style.GrabRounding = style.FrameRounding = 2.3f;
    // style.ScrollbarRounding = 5.0f;
    // style.FrameBorderSize = 1.0f;
    // style.ItemSpacing.y = 6.5f;
}

bool neko_imgui_create_fonts_texture(neko_imgui_context_t* neko_imgui) {
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int32_t width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width,
                                 &height);  // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders.
                                            // If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Create font texture
    neko_render_texture_desc_t tdesc = {};
    tdesc.width = width;
    tdesc.height = height;
    tdesc.format = R_TEXTURE_FORMAT_RGBA8;
    tdesc.min_filter = R_TEXTURE_FILTER_LINEAR;
    tdesc.mag_filter = R_TEXTURE_FILTER_LINEAR;
    *tdesc.data = (void*)pixels;

    neko_imgui->font_tex = neko_render_texture_create(tdesc);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)neko_imgui->font_tex.id;

    return true;
}

void neko_imgui_device_create(neko_imgui_context_t* neko_imgui) {
    static const char* imgui_vertsrc = NEKO_IMGUI_SHADER_VERSION
            R"(
uniform mat4 ProjMtx;
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec4 Color;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main() {
   Frag_UV = TexCoord;
   Frag_Color = Color;
   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);
}
)";

    static const char* imgui_fragsrc = NEKO_IMGUI_SHADER_VERSION
            R"(
precision mediump float;
uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;
void main(){
   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
}
)";

    // Shader source description
    neko_render_shader_source_desc_t sources[2] = {};
    sources[0].type = R_SHADER_STAGE_VERTEX;
    sources[0].source = imgui_vertsrc;
    sources[1].type = R_SHADER_STAGE_FRAGMENT;
    sources[1].source = imgui_fragsrc;

    // Shader desc
    neko_render_shader_desc_t sdesc = {};
    sdesc.sources = sources;
    sdesc.size = sizeof(sources);
    memcpy(sdesc.name, "imgui", 64);

    // Create shader
    neko_imgui->shader = neko_render_shader_create(sdesc);

    // Uniform texture
    neko_render_uniform_layout_desc_t slayout = NEKO_DEFAULT_VAL();
    slayout.type = R_UNIFORM_SAMPLER2D;
    neko_render_uniform_desc_t utexdesc = {};
    memcpy(utexdesc.name, "Texture", 64);
    utexdesc.layout = &slayout;
    neko_imgui->u_tex = neko_render_uniform_create(utexdesc);

    // Construct uniform
    neko_render_uniform_layout_desc_t ulayout = NEKO_DEFAULT_VAL();
    ulayout.type = R_UNIFORM_MAT4;
    neko_render_uniform_desc_t udesc = {};
    memcpy(udesc.name, "ProjMtx", 64);
    udesc.layout = &ulayout;

    // Construct project matrix uniform
    neko_imgui->u_proj = neko_render_uniform_create(udesc);

    // Vertex buffer description
    neko_render_vertex_buffer_desc_t vbufdesc = {};
    vbufdesc.usage = R_BUFFER_USAGE_STREAM;
    vbufdesc.data = NULL;

    // Construct vertex buffer
    neko_imgui->vbo = neko_render_vertex_buffer_create(vbufdesc);

    // Index buffer desc
    neko_render_index_buffer_desc_t ibufdesc = {};
    ibufdesc.usage = R_BUFFER_USAGE_STREAM;
    ibufdesc.data = NULL;

    // Create index buffer
    neko_imgui->ibo = neko_render_index_buffer_create(ibufdesc);

    // Vertex attr layout
    neko_render_vertex_attribute_desc_t vattrs[3] = {};
    vattrs[0].format = R_VERTEX_ATTRIBUTE_FLOAT2;  // Position
    vattrs[1].format = R_VERTEX_ATTRIBUTE_FLOAT2;  // UV
    vattrs[2].format = R_VERTEX_ATTRIBUTE_BYTE4;   // Color

    // Pipeline desc
    neko_render_pipeline_desc_t pdesc = {};
    pdesc.raster.shader = neko_imgui->shader;
    pdesc.raster.index_buffer_element_size = (sizeof(ImDrawIdx) == 2) ? sizeof(uint16_t) : sizeof(u32);
    pdesc.blend.func = R_BLEND_EQUATION_ADD;
    pdesc.blend.src = R_BLEND_MODE_SRC_ALPHA;
    pdesc.blend.dst = R_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
    pdesc.layout.attrs = vattrs;
    pdesc.layout.size = sizeof(vattrs);

    // Create pipeline
    neko_imgui->pip = neko_render_pipeline_create(pdesc);

    // Create default fonts texture
    neko_imgui_create_fonts_texture(neko_imgui);
}

neko_imgui_context_t neko_imgui_new(neko_command_buffer_t* cb, u32 hndl, bool install_callbacks) {
    NEKO_ASSERT(cb);

    neko_imgui_context_t neko_imgui = {};
    neko_imgui.cb = cb;
    neko_imgui.ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(neko_imgui.ctx);

    ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

    neko_imgui.win_hndl = hndl;
    neko_imgui.time = 0.0;

    // Setup backend capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_neko";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = neko_platform_key_to_codepoint(NEKO_KEYCODE_TAB);
    io.KeyMap[ImGuiKey_LeftArrow] = neko_platform_key_to_codepoint(NEKO_KEYCODE_LEFT);
    io.KeyMap[ImGuiKey_RightArrow] = neko_platform_key_to_codepoint(NEKO_KEYCODE_RIGHT);
    io.KeyMap[ImGuiKey_UpArrow] = neko_platform_key_to_codepoint(NEKO_KEYCODE_UP);
    io.KeyMap[ImGuiKey_DownArrow] = neko_platform_key_to_codepoint(NEKO_KEYCODE_DOWN);
    io.KeyMap[ImGuiKey_PageUp] = neko_platform_key_to_codepoint(NEKO_KEYCODE_PAGE_UP);
    io.KeyMap[ImGuiKey_PageDown] = neko_platform_key_to_codepoint(NEKO_KEYCODE_PAGE_DOWN);
    io.KeyMap[ImGuiKey_Home] = neko_platform_key_to_codepoint(NEKO_KEYCODE_HOME);
    io.KeyMap[ImGuiKey_End] = neko_platform_key_to_codepoint(NEKO_KEYCODE_END);
    io.KeyMap[ImGuiKey_Insert] = neko_platform_key_to_codepoint(NEKO_KEYCODE_INSERT);
    io.KeyMap[ImGuiKey_Delete] = neko_platform_key_to_codepoint(NEKO_KEYCODE_DELETE);
    io.KeyMap[ImGuiKey_Backspace] = neko_platform_key_to_codepoint(NEKO_KEYCODE_BACKSPACE);
    io.KeyMap[ImGuiKey_Space] = neko_platform_key_to_codepoint(NEKO_KEYCODE_SPACE);
    io.KeyMap[ImGuiKey_Enter] = neko_platform_key_to_codepoint(NEKO_KEYCODE_ENTER);
    io.KeyMap[ImGuiKey_Escape] = neko_platform_key_to_codepoint(NEKO_KEYCODE_ESC);
    io.KeyMap[ImGuiKey_KeypadEnter] = neko_platform_key_to_codepoint(NEKO_KEYCODE_KP_ENTER);
    io.KeyMap[ImGuiKey_A] = neko_platform_key_to_codepoint(NEKO_KEYCODE_A);
    io.KeyMap[ImGuiKey_C] = neko_platform_key_to_codepoint(NEKO_KEYCODE_C);
    io.KeyMap[ImGuiKey_V] = neko_platform_key_to_codepoint(NEKO_KEYCODE_V);
    io.KeyMap[ImGuiKey_X] = neko_platform_key_to_codepoint(NEKO_KEYCODE_X);
    io.KeyMap[ImGuiKey_Y] = neko_platform_key_to_codepoint(NEKO_KEYCODE_Y);
    io.KeyMap[ImGuiKey_Z] = neko_platform_key_to_codepoint(NEKO_KEYCODE_Z);

    neko_imgui_style();

    io.FontGlobalScale = 1.0f;
    io.FontAllowUserScaling = false;

    ImFontConfig config;
    // config.OversampleH = 3;
    // config.OversampleV = 3;
    // config.PixelSnapH = 1;

    // io.FontGlobalScale = 1.5f;

    //    io.Fonts->AddFontFromFileTTF(game_assets("gamedir/assets/fonts/fusion-pixel-12px-monospaced-zh_hans.ttf").c_str(), 22.0f, &config, io.Fonts->GetGlyphRangesChineseFull());

    neko_imgui_device_create(&neko_imgui);

    io.SetClipboardTextFn = neko_imgui_clipboard_setter;
    io.GetClipboardTextFn = neko_imgui_clipboard_getter;
    io.ClipboardUserData = nullptr;

    // auto viewport = ImGui::GetMainViewport();
    // viewport->PlatformHandleRaw = (void *)neko_platform_hwnd();

    auto win = neko_imgui_glfw_window();

    // Set platform dependent data in viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void*)win;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = glfwGetWin32Window(win);
#elif defined(__APPLE__)
    // main_viewport->PlatformHandleRaw = (void*)glfwGetCocoaWindow(win);
    IM_UNUSED(main_viewport);
#else
    IM_UNUSED(main_viewport);
#endif
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        neko_imgui_opengl_init_platform_interface();
    }

    return neko_imgui;
}

void neko_imgui_update_mouse_and_keys(neko_imgui_context_t* ctx) {

    // auto window = (GLFWwindow *)neko_slot_array_getp(neko_subsystem(platform)->windows, neko_platform_main_window())->hndl;

    ImGuiIO& io = ImGui::GetIO();

    neko_platform_event_t evt = {};
    while (neko_platform_poll_events(&evt, false)) {
        switch (evt.type) {
            case NEKO_PF_EVENT_KEY: {
                switch (evt.key.action) {
                    case NEKO_PF_KEY_PRESSED: {
                        io.KeysDown[evt.key.codepoint] = true;
                    } break;

                    case NEKO_PF_KEY_RELEASED: {
                        io.KeysDown[evt.key.codepoint] = false;
                    } break;

                    default:
                        break;
                }

            } break;

            case NEKO_PF_EVENT_TEXT: {
                u32 cp = evt.text.codepoint;
                if (cp <= IM_UNICODE_CODEPOINT_MAX) {
                    io.AddInputCharacter(cp);
                }
            } break;

            default:
                break;
        }
    }

    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_BACKSPACE)] = neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE);
    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_TAB)] = neko_platform_key_pressed(NEKO_KEYCODE_TAB);
    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_ENTER)] = neko_platform_key_pressed(NEKO_KEYCODE_ENTER);
    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_LEFT)] = neko_platform_key_pressed(NEKO_KEYCODE_LEFT);
    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_RIGHT)] = neko_platform_key_pressed(NEKO_KEYCODE_RIGHT);
    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_UP)] = neko_platform_key_pressed(NEKO_KEYCODE_UP);
    io.KeysDown[neko_platform_key_to_codepoint(NEKO_KEYCODE_DOWN)] = neko_platform_key_pressed(NEKO_KEYCODE_DOWN);

    // Modifiers
    io.KeyCtrl = neko_platform_key_down(NEKO_KEYCODE_LEFT_CONTROL) || neko_platform_key_down(NEKO_KEYCODE_RIGHT_CONTROL);
    io.KeyShift = neko_platform_key_down(NEKO_KEYCODE_LEFT_SHIFT) || neko_platform_key_down(NEKO_KEYCODE_RIGHT_SHIFT);
    io.KeyAlt = neko_platform_key_down(NEKO_KEYCODE_LEFT_ALT) || neko_platform_key_down(NEKO_KEYCODE_RIGHT_ALT);
    io.KeySuper = false;

    // Update buttons
    io.MouseDown[0] = neko_platform_mouse_down(NEKO_MOUSE_LBUTTON);
    io.MouseDown[1] = neko_platform_mouse_down(NEKO_MOUSE_RBUTTON);
    io.MouseDown[2] = neko_platform_mouse_down(NEKO_MOUSE_MBUTTON);

    // Mouse position
    int32_t mpx = 0, mpy = 0;
    neko_platform_mouse_position(&mpx, &mpy);
    io.MousePos = ImVec2((float)mpx, (float)mpy);

    // Mouse wheel
    neko_platform_mouse_wheel(&io.MouseWheelH, &io.MouseWheel);
}

void neko_imgui_shutdown(neko_imgui_context_t* neko_imgui) { ImGui::DestroyContext(neko_imgui->ctx); }

void neko_imgui_new_frame(neko_imgui_context_t* neko_imgui) {
    NEKO_ASSERT(neko_imgui->ctx != nullptr);
    ImGui::SetCurrentContext(neko_imgui->ctx);
    NEKO_ASSERT(ImGui::GetCurrentContext() != nullptr);

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! Missing call to renderer _NewFrame() function?");

    // Setup display size (every frame to accommodate for window resizing)
    u32 w, h;
    u32 display_w, display_h;

    // Get platform window size and framebuffer size from window handle
    neko_platform_window_size(neko_imgui->win_hndl, &w, &h);
    neko_platform_framebuffer_size(neko_imgui->win_hndl, &display_w, &display_h);

    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0) io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    io.DeltaTime = neko_subsystem(platform)->time.delta;

    neko_imgui_update_mouse_and_keys(neko_imgui);

    ImGui::NewFrame();
}

void neko_imgui_render_window(neko_imgui_context_t* neko_imgui, ImDrawData* draw_data) {

    // ImDrawData *draw_data = ImGui::GetDrawData();

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    // Setup viewport, orthographic projection matrix
    float l = draw_data->DisplayPos.x;
    float r = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float t = draw_data->DisplayPos.y;
    float b = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

    const float ortho[4][4] = {
            {2.0f / (r - l), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (t - b), 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {(r + l) / (l - r), (t + b) / (b - t), 0.0f, 1.0f},
    };

    neko_mat4 m = neko_mat4_elem((float*)ortho);

    // Set up data binds
    neko_render_bind_vertex_buffer_desc_t vbuffers = {};
    vbuffers.buffer = neko_imgui->vbo;

    neko_render_bind_index_buffer_desc_t ibuffers = {};
    ibuffers.buffer = neko_imgui->ibo;

    neko_render_bind_uniform_desc_t ubuffers = {};
    ubuffers.uniform = neko_imgui->u_proj;
    ubuffers.data = &m;

    // Set up data binds
    neko_render_bind_desc_t binds = {};
    binds.vertex_buffers.desc = &vbuffers;
    binds.vertex_buffers.size = sizeof(vbuffers);
    binds.index_buffers.desc = &ibuffers;
    binds.index_buffers.size = sizeof(ibuffers);
    binds.uniforms.desc = &ubuffers;
    binds.uniforms.size = sizeof(ubuffers);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;          // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale;  // (1,1) unless using retina display which are often (2,2)

    // Default action pass
    neko_handle(neko_render_renderpass_t) def_pass = {};
    def_pass.id = 0;

    // Render pass action for clearing screen (could handle this if you wanted to render gui into a separate frame buffer)
    neko_render_renderpass_begin(neko_imgui->cb, def_pass);
    {
        // Bind pipeline
        neko_render_pipeline_bind(neko_imgui->cb, neko_imgui->pip);

        // Set viewport
        neko_render_set_viewport(neko_imgui->cb, 0, 0, fb_width, fb_height);

        // Global bindings for pipeline
        neko_render_apply_bindings(neko_imgui->cb, &binds);

        // Render command lists
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            // Update vertex buffer
            neko_render_vertex_buffer_desc_t vdesc = {};
            vdesc.usage = R_BUFFER_USAGE_STREAM;
            vdesc.data = cmd_list->VtxBuffer.Data;
            vdesc.size = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            neko_render_vertex_buffer_request_update(neko_imgui->cb, neko_imgui->vbo, &vdesc);

            // Update index buffer
            neko_render_index_buffer_desc_t idesc = {};
            idesc.usage = R_BUFFER_USAGE_STREAM;
            idesc.data = cmd_list->IdxBuffer.Data;
            idesc.size = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
            neko_render_index_buffer_request_update(neko_imgui->cb, neko_imgui->ibo, &idesc);

            // Iterate through command buffer
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL) {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                        // Could add something here...not sure what though
                        // ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                    } else
                        pcmd->UserCallback(cmd_list, pcmd);
                } else {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec4 clip_rect;
                    clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                    clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                    clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                    clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                    // Render
                    if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
                        // Set view scissor
                        neko_render_set_view_scissor(neko_imgui->cb, (int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                        // Grab handle from command texture id
                        neko_handle(neko_render_texture_t) tex = neko_handle_create(neko_render_texture_t, (u32)(intptr_t)pcmd->TextureId);

                        neko_render_bind_uniform_desc_t sbuffer = {};
                        sbuffer.uniform = neko_imgui->u_tex;
                        sbuffer.data = &tex;
                        sbuffer.binding = 0;

                        neko_render_bind_desc_t sbind = {};
                        sbind.uniforms.desc = &sbuffer;
                        sbind.uniforms.size = sizeof(sbuffer);

                        // Bind individual texture bind
                        neko_render_apply_bindings(neko_imgui->cb, &sbind);

                        // Draw elements
                        neko_render_draw_desc_t draw = {};
                        draw.start = (size_t)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx));
                        draw.count = (size_t)pcmd->ElemCount;
                        neko_render_draw(neko_imgui->cb, &draw);
                    }
                }
            }
        }
    }
    neko_render_renderpass_end(neko_imgui->cb);
}

static void neko_imgui_internal_render(ImGuiViewport* viewport, void*) {
    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear)) {
        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    //    neko_imgui_render_window(viewport->DrawData);
}

static void neko_imgui_opengl_init_platform_interface() {
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_RenderWindow = neko_imgui_internal_render;
}

// static void ImGui_ImplOpenGL3_ShutdownPlatformInterface() { ImGui::DestroyPlatformWindows(); }

void neko_imgui_render(neko_imgui_context_t* neko_imgui) {

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::SetCurrentContext(neko_imgui->ctx);

    ImGui::Render();

    neko_imgui_render_window(neko_imgui, ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void neko_imgui_draw_text(std::string text, neko_color_t col, int x, int y, bool outline, neko_color_t outline_col) {

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList(viewport);

    if (outline) {

        auto outline_col_im = ImColor(outline_col.r, outline_col.g, outline_col.b, col.a);

        draw_list->AddText(ImVec2(x + 0, y - 1), outline_col_im, text.c_str());  // up
        draw_list->AddText(ImVec2(x + 0, y + 1), outline_col_im, text.c_str());  // down
        draw_list->AddText(ImVec2(x + 1, y + 0), outline_col_im, text.c_str());  // right
        draw_list->AddText(ImVec2(x - 1, y + 0), outline_col_im, text.c_str());  // left

        draw_list->AddText(ImVec2(x + 1, y + 1), outline_col_im, text.c_str());  // down-right
        draw_list->AddText(ImVec2(x - 1, y + 1), outline_col_im, text.c_str());  // down-left

        draw_list->AddText(ImVec2(x + 1, y - 1), outline_col_im, text.c_str());  // up-right
        draw_list->AddText(ImVec2(x - 1, y - 1), outline_col_im, text.c_str());  // up-left
    }

    draw_list->AddText(ImVec2(x, y), ImColor(col.r, col.g, col.b, col.a), text.c_str());  // base
}
