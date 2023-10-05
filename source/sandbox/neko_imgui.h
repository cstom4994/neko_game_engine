#ifndef NEKO_IMGUI_IMPL_H
#define NEKO_IMGUI_IMPL_H

#include "engine/neko_engine.h"
#include "neko_imgui.h"

// ImGui
#include "libs/imgui/imgui.h"

// Main context for necessary imgui information
typedef struct neko_imgui_context_s {
    uint32_t win_hndl;
    double time;
    bool mouse_just_pressed[ImGuiMouseButton_COUNT];
    bool mouse_cursors[ImGuiMouseCursor_COUNT];
    neko_handle(neko_graphics_pipeline_t) pip;
    neko_handle(neko_graphics_vertex_buffer_t) vbo;
    neko_handle(neko_graphics_index_buffer_t) ibo;
    neko_handle(neko_graphics_shader_t) shader;
    neko_handle(neko_graphics_texture_t) font_tex;
    neko_handle(neko_graphics_uniform_t) u_tex;
    neko_handle(neko_graphics_uniform_t) u_proj;
    ImGuiContext* ctx;
} neko_imgui_context_t;

typedef struct neko_imgui_vertex_t {
    float position[2];
    float uv[2];
    uint8_t col[4];
} neko_imgui_vertex_t;

#ifdef NEKO_PLATFORM_WEB
#define NEKO_IMGUI_SHADER_VERSION "#version 300 es\n"
#else
#define NEKO_IMGUI_SHADER_VERSION "#version 330 core\n"
#endif

neko_imgui_context_t neko_imgui_new(uint32_t hndl, bool install_callbacks);
void neko_imgui_device_create(neko_imgui_context_t* gs);
bool neko_imgui_create_fonts_texture(neko_imgui_context_t* gs);
void neko_imgui_render(neko_imgui_context_t* gs, neko_command_buffer_t* cb);

void neko_imgui_new_frame(neko_imgui_context_t* gs);

/*===============================
//      Implementation
================================*/

#ifdef NEKO_IMGUI_IMPL

void neko_imgui_device_create(neko_imgui_context_t* gs) {
    static const char* imgui_vertsrc = NEKO_IMGUI_SHADER_VERSION
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

    static const char* imgui_fragsrc = NEKO_IMGUI_SHADER_VERSION
            "precision mediump float;\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main(){\n"
            "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

    // Shader source description
    neko_graphics_shader_source_desc_t sources[2] = {};
    sources[0].type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX;
    sources[0].source = imgui_vertsrc;
    sources[1].type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT;
    sources[1].source = imgui_fragsrc;

    // Shader desc
    neko_graphics_shader_desc_t sdesc = {};
    sdesc.sources = sources;
    sdesc.size = sizeof(sources);
    memcpy(sdesc.name, "imgui", 64);

    // Create shader
    gs->shader = neko_graphics_shader_create(&sdesc);

    // Uniform texture
    neko_graphics_uniform_layout_desc_t slayout = neko_default_val();
    slayout.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D;
    neko_graphics_uniform_desc_t utexdesc = {};
    memcpy(utexdesc.name, "Texture", 64);
    utexdesc.layout = &slayout;
    gs->u_tex = neko_graphics_uniform_create(&utexdesc);

    // Construct uniform
    neko_graphics_uniform_layout_desc_t ulayout = neko_default_val();
    ulayout.type = NEKO_GRAPHICS_UNIFORM_MAT4;
    neko_graphics_uniform_desc_t udesc = {};
    memcpy(udesc.name, "ProjMtx", 64);
    udesc.layout = &ulayout;

    // Construct project matrix uniform
    gs->u_proj = neko_graphics_uniform_create(&udesc);

    // Vertex buffer description
    neko_graphics_vertex_buffer_desc_t vbufdesc = {};
    vbufdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
    vbufdesc.data = NULL;

    // Construct vertex buffer
    gs->vbo = neko_graphics_vertex_buffer_create(&vbufdesc);

    // Index buffer desc
    neko_graphics_index_buffer_desc_t ibufdesc = {};
    ibufdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
    ibufdesc.data = NULL;

    // Create index buffer
    gs->ibo = neko_graphics_index_buffer_create(&ibufdesc);

    // Vertex attr layout
    neko_graphics_vertex_attribute_desc_t vattrs[3] = {};
    vattrs[0].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;  // Position
    vattrs[1].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;  // UV
    vattrs[2].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4;   // Color

    // Pipeline desc
    neko_graphics_pipeline_desc_t pdesc = {};
    pdesc.raster.shader = gs->shader;
    pdesc.raster.index_buffer_element_size = (sizeof(ImDrawIdx) == 2) ? sizeof(uint16_t) : sizeof(uint32_t);
    pdesc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD;
    pdesc.blend.src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA;
    pdesc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
    pdesc.layout.attrs = vattrs;
    pdesc.layout.size = sizeof(vattrs);

    // Create pipeline
    gs->pip = neko_graphics_pipeline_create(&pdesc);

    // Create default fonts texture
    neko_imgui_create_fonts_texture(gs);
}

neko_imgui_context_t neko_imgui_new(uint32_t hndl, bool install_callbacks) {
    neko_imgui_context_t gs = {};

    gs.ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(gs.ctx);

    gs.win_hndl = hndl;
    gs.time = 0.0;

    // Setup backend capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    // io.BackendFlags |= ImGuiBackendFlaneko_HasMouseCursors;  // We can honor GetMouseCursor() values (optional)
    // io.BackendFlags |= ImGuiBackendFlaneko_HasSetMousePos;   // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_neko";

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
    io.KeyMap[ImGuiKey_KeyPadEnter] = neko_platform_key_to_codepoint(NEKO_KEYCODE_KP_ENTER);
    io.KeyMap[ImGuiKey_A] = neko_platform_key_to_codepoint(NEKO_KEYCODE_A);
    io.KeyMap[ImGuiKey_C] = neko_platform_key_to_codepoint(NEKO_KEYCODE_C);
    io.KeyMap[ImGuiKey_V] = neko_platform_key_to_codepoint(NEKO_KEYCODE_V);
    io.KeyMap[ImGuiKey_X] = neko_platform_key_to_codepoint(NEKO_KEYCODE_X);
    io.KeyMap[ImGuiKey_Y] = neko_platform_key_to_codepoint(NEKO_KEYCODE_Y);
    io.KeyMap[ImGuiKey_Z] = neko_platform_key_to_codepoint(NEKO_KEYCODE_Z);

    // Rendering
    // io.BackendFlags |= ImGuiBackendFlaneko_RendererHasVtxOffset;

    neko_imgui_device_create(&gs);

    //     io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    //     io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    //     io.ClipboardUserData = g_Window;
    // #if defined(_WIN32)
    //     io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
    // #endif

    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return NULL and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
    //     GLFWerrorfun prev_error_callback = glfwSetErrorCallback(NULL);
    //     g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    // #if GLFW_HAS_NEW_CURSORS
    //     g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
    // #else
    //     g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    //     g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    // #endif
    //     glfwSetErrorCallback(prev_error_callback);

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    // g_PrevUserCallbackMousebutton = NULL;
    // g_PrevUserCallbackScroll = NULL;
    // g_PrevUserCallbackKey = NULL;
    // g_PrevUserCallbackChar = NULL;
    // if (install_callbacks)
    // {
    //     g_InstalledCallbacks = true;
    //     g_PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    //     g_PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    //     g_PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    //     g_PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    // }

    // g_ClientApi = client_api;

    return gs;
}

bool neko_imgui_create_fonts_texture(neko_imgui_context_t* gs) {
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int32_t width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width,
                                 &height);  // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders.
                                            // If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Create font texture
    neko_graphics_texture_desc_t tdesc = {};
    tdesc.width = width;
    tdesc.height = height;
    tdesc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    tdesc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
    tdesc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
    *tdesc.data = (void*)pixels;

    gs->font_tex = neko_graphics_texture_create(&tdesc);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)gs->font_tex.id;

    return true;
}

void neko_imgui_update_mouse_and_keys(neko_imgui_context_t* ctx) {
    ImGuiIO& io = ImGui::GetIO();

    // Need to add an input character...check to see if it's an alpha/numeric string

    // Have to poll events from platform layer to do this
    neko_platform_event_t evt = {};
    while (neko_platform_poll_events(&evt, false)) {
        switch (evt.type) {
            case NEKO_PLATFORM_EVENT_KEY: {
                switch (evt.key.action) {
                    case NEKO_PLATFORM_KEY_PRESSED: {
                        // Not sure if this is correct at all.
                        uint32_t cp = evt.key.codepoint;
                        if (cp <= IM_UNICODE_CODEPOINT_MAX) {
                            io.AddInputCharacter(cp);
                        }

                        // io.KeysDown[io.KeyMap[evt.key.codepoint]] = true;

                    } break;

                    case NEKO_PLATFORM_KEY_RELEASED: {
                        io.KeysDown[evt.key.codepoint] = false;
                    } break;

                    default:
                        break;
                }
            } break;

            default:
                break;
        }
    }

    // if (neko_platform_key_pressed(NEKO_KEYCODE_TAB)) neko_println("tab");
    // if (neko_platform_key_pressed(NEKO_KEYCODE_ENTER)) neko_println("enter");
    // if (neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE)) neko_println("bspace");
    // if (neko_platform_key_pressed(NEKO_KEYCODE_LEFT)) neko_println("left");
    // if (neko_platform_key_pressed(NEKO_KEYCODE_RIGHT)) neko_println("right");
    // if (neko_platform_key_pressed(NEKO_KEYCODE_UP)) neko_println("up");
    // if (neko_platform_key_pressed(NEKO_KEYCODE_DOWN)) neko_println("down");

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

void neko_imgui_new_frame(neko_imgui_context_t* gs) {
    neko_assert(gs->ctx != nullptr);
    ImGui::SetCurrentContext(gs->ctx);
    neko_assert(ImGui::GetCurrentContext() != nullptr);

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    uint32_t w, h;
    uint32_t display_w, display_h;

    // Get platform window size and framebuffer size from window handle
    neko_platform_window_size(gs->win_hndl, &w, &h);
    neko_platform_framebuffer_size(gs->win_hndl, &display_w, &display_h);

    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0) io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    // double current_time = (double)neko_platform_elapsed_time();
    io.DeltaTime = neko_subsystem(platform)->time.delta;
    // io.DeltaTime = gs->time > 0.0 ? (float)(current_time - gs->time) : (float)(1.0f / 60.0f);
    // gs->time = current_time;

    neko_imgui_update_mouse_and_keys(gs);

    ImGui::NewFrame();
}

void neko_imgui_render(neko_imgui_context_t* gs, neko_command_buffer_t* cb) {
    // Set current context
    ImGui::SetCurrentContext(gs->ctx);

    // Do da drawing
    ImGui::Render();

    // Cache draw data
    ImDrawData* draw_data = ImGui::GetDrawData();

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

    // neko_mat4 m = neko_mat4_identity();
    neko_mat4 m = neko_mat4_elem((float*)ortho);

    // Set up data binds
    neko_graphics_bind_vertex_buffer_desc_t vbuffers = {};
    vbuffers.buffer = gs->vbo;

    neko_graphics_bind_index_buffer_desc_t ibuffers = {};
    ibuffers.buffer = gs->ibo;

    neko_graphics_bind_uniform_desc_t ubuffers = {};
    ubuffers.uniform = gs->u_proj;
    ubuffers.data = &m;

    // Set up data binds
    neko_graphics_bind_desc_t binds = {};
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
    neko_handle(neko_graphics_renderpass_t) def_pass = {};
    def_pass.id = 0;

    // Render pass action for clearing screen (could handle this if you wanted to render gui into a separate frame buffer)
    neko_graphics_renderpass_begin(cb, def_pass);
    {
        // Bind pipeline
        neko_graphics_pipeline_bind(cb, gs->pip);

        // Set viewport
        neko_graphics_set_viewport(cb, 0, 0, fb_width, fb_height);

        // Global bindings for pipeline
        neko_graphics_apply_bindings(cb, &binds);

        // Render command lists
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            // Update vertex buffer
            neko_graphics_vertex_buffer_desc_t vdesc = {};
            vdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
            vdesc.data = cmd_list->VtxBuffer.Data;
            vdesc.size = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            neko_graphics_vertex_buffer_request_update(cb, gs->vbo, &vdesc);

            // Update index buffer
            neko_graphics_index_buffer_desc_t idesc = {};
            idesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
            idesc.data = cmd_list->IdxBuffer.Data;
            idesc.size = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
            neko_graphics_index_buffer_request_update(cb, gs->ibo, &idesc);

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
                        neko_graphics_set_view_scissor(cb, (int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                        // Grab handle from command texture id
                        neko_handle(neko_graphics_texture_t) tex = neko_handle_create(neko_graphics_texture_t, (uint32_t)(intptr_t)pcmd->TextureId);

                        neko_graphics_bind_uniform_desc_t sbuffer = {};
                        sbuffer.uniform = gs->u_tex;
                        sbuffer.data = &tex;
                        sbuffer.binding = 0;

                        neko_graphics_bind_desc_t sbind = {};
                        sbind.uniforms.desc = &sbuffer;
                        sbind.uniforms.size = sizeof(sbuffer);

                        // Bind individual texture bind
                        neko_graphics_apply_bindings(cb, &sbind);

                        // Draw elements
                        neko_graphics_draw_desc_t draw = {};
                        draw.start = (size_t)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx));
                        draw.count = (size_t)pcmd->ElemCount;
                        neko_graphics_draw(cb, &draw);
                    }
                }
            }
        }
    }
    neko_graphics_renderpass_end(cb);
}

#endif  // NEKO_IMGUI_IMPL

#endif
