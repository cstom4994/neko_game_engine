
#ifndef GAME_IMGUI_IMPL_H
#define GAME_IMGUI_IMPL_H

#include <array>
#include <cassert>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "engine/neko.h"
#include "engine/neko_engine.h"

// game
#include "neko_api.h"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "deps/imgui/imgui.h"
#include "deps/imgui/imgui_internal.h"

// glfw
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// Main context for necessary imgui information
typedef struct neko_imgui_context_s {
    neko_command_buffer_t *cb;
    u32 win_hndl;
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
    ImGuiContext *ctx;
} neko_imgui_context_t;

typedef struct neko_imgui_vertex_t {
    float position[2];
    float uv[2];
    uint8_t col[4];
} neko_imgui_vertex_t;

// extern neko_imgui_context_t g_imgui;
// extern neko_command_buffer_t g_cb;

#ifdef NEKO_PLATFORM_WEB
#define NEKO_IMGUI_SHADER_VERSION "#version 300 es\n"
#else
#define NEKO_IMGUI_SHADER_VERSION "#version 330 core\n"
#endif

NEKO_STATIC_INLINE std::size_t g_imgui_mem_usage = 0;

NEKO_PRIVATE(void *) __neko_imgui_malloc(size_t sz, void *user_data) {
#ifdef NEKO_IMGUI_USE_GC
    return __neko_mem_safe_alloc((sz), (char *)__FILE__, __LINE__, &g_imgui_mem_usage);
#else
    return malloc(sz);
#endif
}

NEKO_PRIVATE(void) __neko_imgui_free(void *ptr, void *user_data) {
#ifdef NEKO_IMGUI_USE_GC
    __neko_mem_safe_free(ptr, &g_imgui_mem_usage);
#else
    return free(ptr);
#endif
}

NEKO_INLINE std::size_t __neko_imgui_meminuse() { return g_imgui_mem_usage; }

NEKO_INLINE static const char *neko_imgui_clipboard_getter(void *user_data) { return neko_platform_window_get_clipboard(neko_platform_main_window()); }

NEKO_INLINE static void neko_imgui_clipboard_setter(void *user_data, const char *text) { neko_platform_window_set_clipboard(neko_platform_main_window(), text); }

static void neko_imgui_opengl_init_platform_interface();

NEKO_INLINE auto neko_imgui_glfw_window() {
    struct neko_platform_t *platform = neko_instance()->ctx.platform;
    GLFWwindow *win = (GLFWwindow *)(neko_slot_array_getp(platform->windows, neko_platform_main_window()))->hndl;
    return win;
}

NEKO_INLINE void neko_imgui_style() {
    ImGuiStyle &style = ImGui::GetStyle();
    // style.WindowRounding = 5.3f;
    // style.GrabRounding = style.FrameRounding = 2.3f;
    // style.ScrollbarRounding = 5.0f;
    // style.FrameBorderSize = 1.0f;
    // style.ItemSpacing.y = 6.5f;
}

NEKO_INLINE bool neko_imgui_create_fonts_texture(neko_imgui_context_t *neko_imgui) {
    // Build texture atlas
    ImGuiIO &io = ImGui::GetIO();
    unsigned char *pixels;
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
    *tdesc.data = (void *)pixels;

    neko_imgui->font_tex = neko_graphics_texture_create(tdesc);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)neko_imgui->font_tex.id;

    return true;
}

NEKO_INLINE void neko_imgui_device_create(neko_imgui_context_t *neko_imgui) {
    static const char *imgui_vertsrc = NEKO_IMGUI_SHADER_VERSION
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

    static const char *imgui_fragsrc = NEKO_IMGUI_SHADER_VERSION
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
    neko_imgui->shader = neko_graphics_shader_create(sdesc);

    // Uniform texture
    neko_graphics_uniform_layout_desc_t slayout = neko_default_val();
    slayout.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D;
    neko_graphics_uniform_desc_t utexdesc = {};
    memcpy(utexdesc.name, "Texture", 64);
    utexdesc.layout = &slayout;
    neko_imgui->u_tex = neko_graphics_uniform_create(utexdesc);

    // Construct uniform
    neko_graphics_uniform_layout_desc_t ulayout = neko_default_val();
    ulayout.type = NEKO_GRAPHICS_UNIFORM_MAT4;
    neko_graphics_uniform_desc_t udesc = {};
    memcpy(udesc.name, "ProjMtx", 64);
    udesc.layout = &ulayout;

    // Construct project matrix uniform
    neko_imgui->u_proj = neko_graphics_uniform_create(udesc);

    // Vertex buffer description
    neko_graphics_vertex_buffer_desc_t vbufdesc = {};
    vbufdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
    vbufdesc.data = NULL;

    // Construct vertex buffer
    neko_imgui->vbo = neko_graphics_vertex_buffer_create(vbufdesc);

    // Index buffer desc
    neko_graphics_index_buffer_desc_t ibufdesc = {};
    ibufdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
    ibufdesc.data = NULL;

    // Create index buffer
    neko_imgui->ibo = neko_graphics_index_buffer_create(ibufdesc);

    // Vertex attr layout
    neko_graphics_vertex_attribute_desc_t vattrs[3] = {};
    vattrs[0].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;  // Position
    vattrs[1].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;  // UV
    vattrs[2].format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4;   // Color

    // Pipeline desc
    neko_graphics_pipeline_desc_t pdesc = {};
    pdesc.raster.shader = neko_imgui->shader;
    pdesc.raster.index_buffer_element_size = (sizeof(ImDrawIdx) == 2) ? sizeof(uint16_t) : sizeof(u32);
    pdesc.blend.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD;
    pdesc.blend.src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA;
    pdesc.blend.dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
    pdesc.layout.attrs = vattrs;
    pdesc.layout.size = sizeof(vattrs);

    // Create pipeline
    neko_imgui->pip = neko_graphics_pipeline_create(pdesc);

    // Create default fonts texture
    neko_imgui_create_fonts_texture(neko_imgui);
}

NEKO_INLINE neko_imgui_context_t neko_imgui_new(neko_command_buffer_t *cb, u32 hndl, bool install_callbacks) {
    neko_assert(cb);

    neko_imgui_context_t neko_imgui = {};
    neko_imgui.cb = cb;
    neko_imgui.ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(neko_imgui.ctx);

    ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

    neko_imgui.win_hndl = hndl;
    neko_imgui.time = 0.0;

    // Setup backend capabilities flags
    ImGuiIO &io = ImGui::GetIO();
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
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void *)win;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = glfwGetWin32Window(win);
#elif defined(__APPLE__)
    main_viewport->PlatformHandleRaw = (void *)glfwGetCocoaWindow(win);
#else
    IM_UNUSED(main_viewport);
#endif
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        neko_imgui_opengl_init_platform_interface();
    }

    return neko_imgui;
}

NEKO_INLINE void neko_imgui_update_mouse_and_keys(neko_imgui_context_t *ctx) {

    // auto window = (GLFWwindow *)neko_slot_array_getp(neko_subsystem(platform)->windows, neko_platform_main_window())->hndl;

    ImGuiIO &io = ImGui::GetIO();

    neko_platform_event_t evt = {};
    while (neko_platform_poll_events(&evt, false)) {
        switch (evt.type) {
            case NEKO_PLATFORM_EVENT_KEY: {
                switch (evt.key.action) {
                    case NEKO_PLATFORM_KEY_PRESSED: {
                        io.KeysDown[evt.key.codepoint] = true;
                    } break;

                    case NEKO_PLATFORM_KEY_RELEASED: {
                        io.KeysDown[evt.key.codepoint] = false;
                    } break;

                    default:
                        break;
                }

            } break;

            case NEKO_PLATFORM_EVENT_TEXT: {
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

NEKO_INLINE void neko_imgui_shutdown(neko_imgui_context_t *neko_imgui) { ImGui::DestroyContext(neko_imgui->ctx); }

NEKO_INLINE void neko_imgui_new_frame(neko_imgui_context_t *neko_imgui) {
    neko_assert(neko_imgui->ctx != nullptr);
    ImGui::SetCurrentContext(neko_imgui->ctx);
    neko_assert(ImGui::GetCurrentContext() != nullptr);

    ImGuiIO &io = ImGui::GetIO();
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

NEKO_INLINE void neko_imgui_render_window(neko_imgui_context_t *neko_imgui, ImDrawData *draw_data) {

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

    neko_mat4 m = neko_mat4_elem((float *)ortho);

    // Set up data binds
    neko_graphics_bind_vertex_buffer_desc_t vbuffers = {};
    vbuffers.buffer = neko_imgui->vbo;

    neko_graphics_bind_index_buffer_desc_t ibuffers = {};
    ibuffers.buffer = neko_imgui->ibo;

    neko_graphics_bind_uniform_desc_t ubuffers = {};
    ubuffers.uniform = neko_imgui->u_proj;
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
    neko_graphics_renderpass_begin(neko_imgui->cb, def_pass);
    {
        // Bind pipeline
        neko_graphics_pipeline_bind(neko_imgui->cb, neko_imgui->pip);

        // Set viewport
        neko_graphics_set_viewport(neko_imgui->cb, 0, 0, fb_width, fb_height);

        // Global bindings for pipeline
        neko_graphics_apply_bindings(neko_imgui->cb, &binds);

        // Render command lists
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];

            // Update vertex buffer
            neko_graphics_vertex_buffer_desc_t vdesc = {};
            vdesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
            vdesc.data = cmd_list->VtxBuffer.Data;
            vdesc.size = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            neko_graphics_vertex_buffer_request_update(neko_imgui->cb, neko_imgui->vbo, &vdesc);

            // Update index buffer
            neko_graphics_index_buffer_desc_t idesc = {};
            idesc.usage = NEKO_GRAPHICS_BUFFER_USAGE_STREAM;
            idesc.data = cmd_list->IdxBuffer.Data;
            idesc.size = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
            neko_graphics_index_buffer_request_update(neko_imgui->cb, neko_imgui->ibo, &idesc);

            // Iterate through command buffer
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
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
                        neko_graphics_set_view_scissor(neko_imgui->cb, (int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                        // Grab handle from command texture id
                        neko_handle(neko_graphics_texture_t) tex = neko_handle_create(neko_graphics_texture_t, (u32)(intptr_t)pcmd->TextureId);

                        neko_graphics_bind_uniform_desc_t sbuffer = {};
                        sbuffer.uniform = neko_imgui->u_tex;
                        sbuffer.data = &tex;
                        sbuffer.binding = 0;

                        neko_graphics_bind_desc_t sbind = {};
                        sbind.uniforms.desc = &sbuffer;
                        sbind.uniforms.size = sizeof(sbuffer);

                        // Bind individual texture bind
                        neko_graphics_apply_bindings(neko_imgui->cb, &sbind);

                        // Draw elements
                        neko_graphics_draw_desc_t draw = {};
                        draw.start = (size_t)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx));
                        draw.count = (size_t)pcmd->ElemCount;
                        neko_graphics_draw(neko_imgui->cb, &draw);
                    }
                }
            }
        }
    }
    neko_graphics_renderpass_end(neko_imgui->cb);
}

static void neko_imgui_internal_render(ImGuiViewport *viewport, void *) {
    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear)) {
        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    //    neko_imgui_render_window(viewport->DrawData);
}

static void neko_imgui_opengl_init_platform_interface() {
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_RenderWindow = neko_imgui_internal_render;
}

// static void ImGui_ImplOpenGL3_ShutdownPlatformInterface() { ImGui::DestroyPlatformWindows(); }

NEKO_INLINE void neko_imgui_render(neko_imgui_context_t *neko_imgui) {

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::SetCurrentContext(neko_imgui->ctx);

    ImGui::Render();

    neko_imgui_render_window(neko_imgui, ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

NEKO_INLINE void neko_imgui_draw_text(std::string text, neko_color_t col, int x, int y, bool outline, neko_color_t outline_col) {

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImDrawList *draw_list = ImGui::GetBackgroundDrawList(viewport);

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

#if 0


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
        if (!(iter->flags & NEKO_GUI_WINDOW_HIDDEN) && !(iter->flags & NEKO_GUI_WINDOW_NO_SAVE)) {
            // neko_println("%f,%f,%f,%f %s", iter->bounds.x, iter->bounds.y, iter->bounds.w, iter->bounds.h, iter->name_string);

            neko_nbt_tag_t* win_tag_level = neko_nbt_tag_compound_get(tag_level, iter->name_string);
            if (win_tag_level == NULL) {
                win_tag_level = neko_nbt_new_tag_compound();
                neko_nbt_set_tag_name(win_tag_level, iter->name_string, strlen(iter->name_string));
                neko_nbt_tag_compound_append(tag_level, win_tag_level);
            }

#define overwrite_nbt(_tag, _name, _value)                                 \
    neko_nbt_tag_t *tag_##_name = neko_nbt_tag_compound_get(_tag, #_name); \
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

#endif

#define neko_imgui_tree_max_elementsize sizeof(std::string)
#define neko_imgui_tree_max_tuple 3

NEKO_STATIC_INLINE ImVec4 vec4_to_imvec4(const neko_vec4 &v4) { return {v4.x, v4.y, v4.z, v4.w}; }
NEKO_STATIC_INLINE ImColor vec4_to_imcolor(const neko_vec4 &v4) { return {v4.x * 255.0f, v4.y * 255.0f, v4.z * 255.0f, v4.w * 255.0f}; }

NEKO_INLINE neko_color_t imvec_to_rgba(ImVec4 iv) {
    u8 newr = iv.x * 255;
    u8 newg = iv.y * 255;
    u8 newb = iv.z * 255;
    u8 newa = iv.w * 255;
    return neko_color_t{newr, newg, newb, newa};
}

NEKO_INLINE ImVec4 rgba_to_imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

namespace neko::cpp {
// same as std::as_const in c++17
template <class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept {
    return t;
}
}  // namespace neko::cpp

namespace neko::imgui {

// 这就是这个库实现的功能 只是类 neko::imgui::Auto_t<T> 的包装
template <typename T>
void Auto(T &anything, const std::string &name = std::string());

namespace detail {

template <typename T>
bool AutoExpand(const std::string &name, T &value);
template <typename Container>
bool AutoContainerTreeNode(const std::string &name, Container &cont);
template <typename Container>
bool AutoContainerValues(const std::string &name,
                         Container &cont);  // Container must have .size(), .begin() and .end() methods and ::value_type.
template <typename Container>
bool AutoMapContainerValues(const std::string &name,
                            Container &map);  // Same as above but that iterates over pairs
template <typename Container>
void AutoContainerPushFrontButton(Container &cont);
template <typename Container>
void AutoContainerPushBackButton(Container &cont);
template <typename Container>
void AutoContainerPopFrontButton(Container &cont);
template <typename Container>
void AutoContainerPopBackButton(Container &cont);
template <typename Key, typename var>
void AutoMapKeyValue(Key &key, var &value);

template <std::size_t I, typename... Args>
void AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> * = 0);
template <std::size_t I, typename... Args>
inline void AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 == I> * = 0) {}  // End of recursion.
template <std::size_t I, typename... Args>
void AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> * = 0);
template <std::size_t I, typename... Args>
inline void AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 == I> * = 0) {}  // End of recursion.
template <typename... Args>
void AutoTuple(const std::string &name, std::tuple<Args...> &tpl);
template <typename... Args>
void AutoTuple(const std::string &name, const std::tuple<Args...> &tpl);

template <typename T, std::size_t N>
using c_array_t = T[N];  // so arrays are regular types and can be used in macro

}  // namespace detail

template <typename T>
struct Auto_t {
    static void Auto(T &anything, const std::string &name) {
        // auto tuple = neko::cpp::pfr::structure_tie(anything);
        // neko::imgui::detail::AutoTuple("Struct " + name, tuple);
        static_assert("Auto not support struct!");
    }
};
}  // namespace neko::imgui

template <typename T>
inline void neko::imgui::Auto(T &anything, const std::string &name) {
    neko::imgui::Auto_t<T>::Auto(anything, name);
}

template <typename T>
bool neko::imgui::detail::AutoExpand(const std::string &name, T &value) {
    if (sizeof(T) <= neko_imgui_tree_max_elementsize) {
        ImGui::PushID(name.c_str());
        ImGui::Bullet();
        neko::imgui::Auto_t<T>::Auto(value, name);
        ImGui::PopID();
        return true;
    } else if (ImGui::TreeNode(name.c_str())) {
        neko::imgui::Auto_t<T>::Auto(value, name);
        ImGui::TreePop();
        return true;
    } else
        return false;
}

template <typename Container>
bool neko::imgui::detail::AutoContainerTreeNode(const std::string &name, Container &cont) {
    // std::size_t size = neko::imgui::detail::AutoContainerSize(cont);
    std::size_t size = cont.size();
    if (ImGui::CollapsingHeader(name.c_str())) {
        size_t elemsize = sizeof(decltype(*std::begin(cont)));
        ImGui::Text("大小: %d, 非动态元素大小: %d bytes", (int)size, (int)elemsize);
        return true;
    } else {
        float label_width = ImGui::CalcTextSize(name.c_str()).x + ImGui::GetTreeNodeToLabelSpacing() + 5;
        std::string sizetext = "(大小 = " + std::to_string(size) + ')';
        float sizet_width = ImGui::CalcTextSize(sizetext.c_str()).x;
        float avail_width = ImGui::GetContentRegionAvail().x;
        if (avail_width > label_width + sizet_width) {
            ImGui::SameLine(avail_width - sizet_width);
            ImGui::TextUnformatted(sizetext.c_str());
        }
        return false;
    }
}
template <typename Container>
bool neko::imgui::detail::AutoContainerValues(const std::string &name, Container &cont) {
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        ImGui::PushID(name.c_str());
        std::size_t i = 0;
        for (auto &elem : cont) {
            std::string itemname = "[" + std::to_string(i) + ']';
            neko::imgui::detail::AutoExpand(itemname, elem);
            ++i;
        }
        ImGui::PopID();
        ImGui::Unindent();
        return true;
    } else
        return false;
}
template <typename Container>
bool neko::imgui::detail::AutoMapContainerValues(const std::string &name, Container &cont) {
    if (neko::imgui::detail::AutoContainerTreeNode(name, cont)) {
        ImGui::Indent();
        std::size_t i = 0;
        for (auto &elem : cont) {
            ImGui::PushID(i);
            AutoMapKeyValue(elem.first, elem.second);
            ImGui::PopID();
            ++i;
        }
        ImGui::Unindent();
        return true;
    } else
        return false;
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushFrontButton(Container &cont) {
    if (ImGui::SmallButton("Push Front")) cont.emplace_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPushBackButton(Container &cont) {
    if (ImGui::SmallButton("Push Back ")) cont.emplace_back();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopFrontButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Front ")) cont.pop_front();
}
template <typename Container>
void neko::imgui::detail::AutoContainerPopBackButton(Container &cont) {
    if (!cont.empty() && ImGui::SmallButton("Pop Back  ")) cont.pop_back();
}
template <typename Key, typename var>
void neko::imgui::detail::AutoMapKeyValue(Key &key, var &value) {
    bool b_k = sizeof(Key) <= neko_imgui_tree_max_elementsize;
    bool b_v = sizeof(var) <= neko_imgui_tree_max_elementsize;
    if (b_k) {
        ImGui::TextUnformatted("[");
        ImGui::SameLine();
        neko::imgui::Auto_t<Key>::Auto(key, "");
        ImGui::SameLine();
        ImGui::TextUnformatted("]");
        if (b_v) ImGui::SameLine();
        neko::imgui::Auto_t<var>::Auto(value, "Value");
    } else {
        neko::imgui::Auto_t<Key>::Auto(key, "Key");
        neko::imgui::Auto_t<var>::Auto(value, "Value");
    }
}

template <std::size_t I, typename... Args>
void neko::imgui::detail::AutoTupleRecurse(std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko::imgui::detail::AutoTupleRecurse<I - 1, Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + (std::is_const_v<type> ? "const " : "") + typeid(type).name();
    neko::imgui::detail::AutoExpand(str, std::get<I - 1>(tpl));
}
template <std::size_t I, typename... Args>
void neko::imgui::detail::AutoTupleRecurse(const std::tuple<Args...> &tpl, std::enable_if_t<0 != I> *) {
    neko::imgui::detail::AutoTupleRecurse<I - 1, const Args...>(tpl);  // first draw smaller indeces
    using type = decltype(std::get<I - 1>(tpl));
    std::string str = '<' + std::to_string(I) + ">: " + "const " + typeid(type).name();
    neko::imgui::detail::AutoExpand(str, neko::cpp::as_const(std::get<I - 1>(tpl)));
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string &name, std::tuple<Args...> &tpl) {
    constexpr std::size_t tuple_size = sizeof(decltype(tpl));
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " (" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}
template <typename... Args>
void neko::imgui::detail::AutoTuple(const std::string &name,
                                    const std::tuple<Args...> &tpl)  // same but const
{
    constexpr std::size_t tuple_size = sizeof(std::tuple<Args...>);
    constexpr std::size_t tuple_numelems = sizeof...(Args);
    if (tuple_size <= neko_imgui_tree_max_elementsize && tuple_numelems <= neko_imgui_tree_max_tuple) {
        ImGui::TextUnformatted((name + " !(" + std::to_string(tuple_size) + " bytes)").c_str());
        ImGui::PushID(name.c_str());
        ImGui::Indent();
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::Unindent();
        ImGui::PopID();
    } else if (ImGui::TreeNode((name + " (" + std::to_string(tuple_size) + " bytes)").c_str())) {
        neko::imgui::detail::AutoTupleRecurse<tuple_numelems, Args...>(tpl);
        ImGui::TreePop();
    }
}

// 在此版本中将 templatespec 和 typespec 括在括号中
#define neko_imgui_def_begin_p(templatespec, typespec)                   \
    namespace neko::imgui {                                              \
    neko_va_unpack templatespec struct Auto_t<neko_va_unpack typespec> { \
        static void Auto(neko_va_unpack typespec &var, const std::string &name) {

// 如果宏参数内部没有逗号 请使用不带括号的此版本
#define neko_imgui_def_begin(templatespec, typespec) neko_imgui_def_begin_p((templatespec), (typespec))

#define neko_imgui_def_end() \
    }                        \
    }                        \
    ;                        \
    }

#define neko_imgui_def_inline_p(template_spec, type_spec, code) neko_imgui_def_begin_p(template_spec, type_spec) code neko_imgui_def_end()

#define neko_imgui_def_inline(template_spec, type_spec, code) neko_imgui_def_inline_p((template_spec), (type_spec), code)

#define NEKO_GUI_NULLPTR_COLOR ImVec4(1.0, 0.5, 0.5, 1.0)

neko_imgui_def_begin(template <>, const_str) if (name.empty()) ImGui::TextUnformatted(var);
else ImGui::Text("%s=%s", name.c_str(), var);
neko_imgui_def_end();

neko_imgui_def_begin_p((template <std::size_t N>), (const detail::c_array_t<char, N>)) if (name.empty()) ImGui::TextUnformatted(var, var + N - 1);
else ImGui::Text("%s=%s", name.c_str(), var);
neko_imgui_def_end();

neko_imgui_def_inline(template <>, char *, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, char *const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_inline(template <>, const_str const, const_str tmp = var; neko::imgui::Auto_t<const_str>::Auto(tmp, name););

neko_imgui_def_begin(template <>, std::string) const std::size_t lines = var.find('\n');
if (var.find('\n') != std::string::npos)
    ImGui::InputTextMultiline(name.c_str(), const_cast<char *>(var.c_str()), 256);
else
    ImGui::InputText(name.c_str(), const_cast<char *>(var.c_str()), 256);
neko_imgui_def_end();
neko_imgui_def_begin(template <>, const std::string) if (name.empty()) ImGui::TextUnformatted(var.c_str(), var.c_str() + var.length());
else ImGui::Text("%s=%s", name.c_str(), var.c_str());
neko_imgui_def_end();

neko_imgui_def_inline(template <>, float, ImGui::DragFloat(name.c_str(), &var););
neko_imgui_def_inline(template <>, int, ImGui::InputInt(name.c_str(), &var););
neko_imgui_def_inline(template <>, unsigned int, ImGui::InputInt(name.c_str(), (int *)&var););
neko_imgui_def_inline(template <>, bool, ImGui::Checkbox(name.c_str(), &var););
neko_imgui_def_inline(template <>, ImVec2, ImGui::DragFloat2(name.c_str(), &var.x););
neko_imgui_def_inline(template <>, ImVec4, ImGui::DragFloat4(name.c_str(), &var.x););
neko_imgui_def_inline(template <>, const float, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const int, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const unsigned, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const bool, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name););
neko_imgui_def_inline(template <>, const ImVec2, ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y););
neko_imgui_def_inline(template <>, const ImVec4, ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var.x, var.y, var.z, var.w););

#define INTERNAL_NUM(_c, _imn)                                                                             \
    neko_imgui_def_inline(template <>, _c, ImGui::InputScalar(name.c_str(), ImGuiDataType_##_imn, &var);); \
    neko_imgui_def_inline(template <>, const _c, neko::imgui::Auto_t<const std::string>::Auto(std::to_string(var), name);)

INTERNAL_NUM(u8, U8);
INTERNAL_NUM(u16, U16);
INTERNAL_NUM(u64, U64);
INTERNAL_NUM(s8, S8);
INTERNAL_NUM(s16, S16);
INTERNAL_NUM(s64, S64);

neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 1>), ImGui::DragFloat(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 1>), ImGui::Text("%s%f", (name.empty() ? "" : name + "=").c_str(), var[0]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 2>), ImGui::DragFloat2(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 2>), ImGui::Text("%s(%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 3>), ImGui::DragFloat3(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 3>), ImGui::Text("%s(%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<float, 4>), ImGui::DragFloat4(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<float, 4>), ImGui::Text("%s(%f,%f,%f,%f)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 1>), ImGui::InputInt(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 1>), ImGui::Text("%s%d", (name.empty() ? "" : name + "=").c_str(), var[0]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 2>), ImGui::InputInt2(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 2>), ImGui::Text("%s(%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 3>), ImGui::InputInt3(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 3>), ImGui::Text("%s(%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2]););
neko_imgui_def_inline_p((template <>), (detail::c_array_t<int, 4>), ImGui::InputInt4(name.c_str(), &var[0]););
neko_imgui_def_inline_p((template <>), (const detail::c_array_t<int, 4>), ; ImGui::Text("%s(%d,%d,%d,%d)", (name.empty() ? "" : name + "=").c_str(), var[0], var[1], var[2], var[3]););

neko_imgui_def_begin(template <typename T>, T *) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, T *const) if (var != nullptr) neko::imgui::detail::AutoExpand<T>("Pointer " + name, *var);
else ImGui::TextColored(NEKO_GUI_NULLPTR_COLOR, "%s=NULL", name.c_str());
neko_imgui_def_end();

neko_imgui_def_inline_p((template <typename T, std::size_t N>), (std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const std::array<T, N>), neko::imgui::detail::AutoContainerValues("array " + name, var););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(std::array<T, N> *)(&var)););
neko_imgui_def_inline_p((template <typename T, std::size_t N>), (const detail::c_array_t<T, N>), neko::imgui::detail::AutoContainerValues("Array " + name, *(const std::array<T, N> *)(&var)););

neko_imgui_def_begin_p((template <typename T1, typename T2>),
                       (std::pair<T1, T2>)) if ((std::is_fundamental_v<T1> || std::is_same_v<std::string, T1>) && (std::is_fundamental_v<T2> || std::is_same_v<std::string, T2>)) {
    float width = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(width * 0.4 - 10);  // a bit less than half
    neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    ImGui::SameLine();
    neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
    ImGui::PopItemWidth();
}
else {
    neko::imgui::detail::AutoExpand<T1>(name + ".first", var.first);
    neko::imgui::detail::AutoExpand<T2>(name + ".second", var.second);
}

neko_imgui_def_end();

neko_imgui_def_begin_p((template <typename T1, typename T2>), (const std::pair<T1, T2>)) neko::imgui::detail::AutoExpand<const T1>(name + ".first", var.first);
if (std::is_fundamental_v<T1> && std::is_fundamental_v<T2>) ImGui::SameLine();
neko::imgui::detail::AutoExpand<const T2>(name + ".second", var.second);
neko_imgui_def_end();

neko_imgui_def_inline(template <typename... Args>, std::tuple<Args...>, neko::imgui::detail::AutoTuple("Tuple " + name, var););
neko_imgui_def_inline(template <typename... Args>, const std::tuple<Args...>, neko::imgui::detail::AutoTuple("Tuple " + name, var););

neko_imgui_def_begin(template <typename T>, std::vector<T>) if (neko::imgui::detail::AutoContainerValues<std::vector<T>>("Vector " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <>, std::vector<bool>) if (neko::imgui::detail::AutoContainerTreeNode<std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        bool b = var[i];
        ImGui::Bullet();
        neko::imgui::Auto_t<bool>::Auto(b, '[' + std::to_string(i) + ']');
        var[i] = b;
    }
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, const std::vector<T>) neko::imgui::detail::AutoContainerValues<const std::vector<T>>("Vector " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <>, const std::vector<bool>) if (neko::imgui::detail::AutoContainerTreeNode<const std::vector<bool>>("Vector " + name, var)) {
    ImGui::Indent();
    for (int i = 0; i < var.size(); ++i) {
        ImGui::Bullet();
        neko::imgui::Auto_t<const bool>::Auto(var[i], '[' + std::to_string(i) + ']');
    }
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::list<T>) if (neko::imgui::detail::AutoContainerValues<std::list<T>>("List " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, const std::list<T>) neko::imgui::detail::AutoContainerValues<const std::list<T>>("List " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::deque<T>) if (neko::imgui::detail::AutoContainerValues<std::deque<T>>("Deque " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    ImGui::SameLine();
    neko::imgui::detail::AutoContainerPushBackButton(var);
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopBackButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, const std::deque<T>) neko::imgui::detail::AutoContainerValues<const std::deque<T>>("Deque " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::forward_list<T>) if (neko::imgui::detail::AutoContainerValues<std::forward_list<T>>("Forward list " + name, var)) {
    ImGui::PushID(name.c_str());
    ImGui::Indent();
    neko::imgui::detail::AutoContainerPushFrontButton(var);
    if (!var.empty()) ImGui::SameLine();
    neko::imgui::detail::AutoContainerPopFrontButton(var);
    ImGui::PopID();
    ImGui::Unindent();
}
neko_imgui_def_end();
neko_imgui_def_begin(template <typename T>, const std::forward_list<T>) neko::imgui::detail::AutoContainerValues<const std::forward_list<T>>("Forward list " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::set<T>) neko::imgui::detail::AutoContainerValues<std::set<T>>("Set " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin(template <typename T>, const std::set<T>) neko::imgui::detail::AutoContainerValues<const std::set<T>>("Set " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin(template <typename T>, std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<std::unordered_set<T>>("Unordered set " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin(template <typename T>, const std::unordered_set<T>) neko::imgui::detail::AutoContainerValues<const std::unordered_set<T>>("Unordered set " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin_p((template <typename K, typename V>), (std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::map<K, V>>("Map " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin_p((template <typename K, typename V>), (const std::map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::map<K, V>>("Map " + name, var);
neko_imgui_def_end();

neko_imgui_def_begin_p((template <typename K, typename V>), (std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<std::unordered_map<K, V>>("Unordered map " + name, var);
// todo insert
neko_imgui_def_end();
neko_imgui_def_begin_p((template <typename K, typename V>), (const std::unordered_map<K, V>)) neko::imgui::detail::AutoMapContainerValues<const std::unordered_map<K, V>>("Unordered map " + name, var);
neko_imgui_def_end();

neko_imgui_def_inline(template <>, std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););
neko_imgui_def_inline(template <>, const std::add_pointer_t<void()>, if (ImGui::Button(name.c_str())) var(););

neko_imgui_def_begin(template <>, neko_vec2_t) {
    //    neko::static_refl::neko_type_info<CGameObject>::ForEachVarOf(var, [&](const auto& field, auto&& value) { neko::imgui::Auto(value, std::string(field.name)); });
    ImGui::Text("%f %f", var.x, var.y);
}
neko_imgui_def_end();

namespace neko::imgui {

NEKO_INLINE ImVec4 neko_rgba2imvec(int r, int g, int b, int a = 255) {
    float newr = r / 255.f;
    float newg = g / 255.f;
    float newb = b / 255.f;
    float newa = a / 255.f;
    return ImVec4(newr, newg, newb, newa);
}

NEKO_INLINE void neko_imgui_help_marker(const_str desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

#if 1
NEKO_INLINE void neko_imgui_file_browser(std::string &path) {

    ImGui::Text("Current Path: %s", path.c_str());
    ImGui::Separator();

    if (ImGui::Button("Parent Directory")) {
        std::filesystem::path current_path(path);
        if (!current_path.empty()) {
            current_path = current_path.parent_path();
            path = current_path.string();
        }
    }

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        const auto &entry_path = entry.path();
        const auto &filename = entry_path.filename().string();
        if (entry.is_directory()) {
            if (ImGui::Selectable((filename + "/").c_str())) path = entry_path.string();

        } else {
            if (ImGui::Selectable(filename.c_str())) path = entry_path.string();
        }
    }
}
#endif

NEKO_INLINE bool toggle(const char *label, bool *v) {
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    f32 height = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;

    f32 width = height * 2.f;
    f32 radius = height * 0.50f;

    const ImRect total_bb(pos, pos + ImVec2(width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id)) return false;

    f32 last_active_id_timer = g.LastActiveIdTimer;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
        g.LastActiveIdTimer = 0.f;
    }

    if (g.LastActiveIdTimer == 0.f && g.LastActiveId == id && !pressed) g.LastActiveIdTimer = last_active_id_timer;

    f32 t = *v ? 1.0f : 0.0f;

    if (g.LastActiveId == id) {
        f32 t_anim = ImSaturate(g.LastActiveIdTimer / 0.16f);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    const ImRect frame_bb(pos, pos + ImVec2(width, height));

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, col_bg, true, height * 0.5f);
    ImGui::RenderNavHighlight(total_bb, id);

    ImVec2 label_pos = ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y);
    ImGui::RenderText(label_pos, label);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius), radius - 1.5f, ImGui::GetColorU32(ImGuiCol_CheckMark), 36);

    return pressed;
}

NEKO_INLINE bool button_scrollable_ex(const char *label, const ImVec2 &size_arg, ImGuiButtonFlags flags) {
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool is_pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

    const f32 offset = size.x >= label_size.x + style.FramePadding.x * 2.0f ? size.x + style.FramePadding.x
                                                                            : static_cast<int>(g.Time * 60.f) % static_cast<int>(label_size.x + size.x + style.FramePadding.x * 2.f + 4);
    const ImRect text = ImRect(ImVec2(bb.Min.x + size.x - offset + style.FramePadding.x * 2.f, bb.Min.y + style.FramePadding.y), bb.Max - style.FramePadding);

    ImGui::RenderTextClipped(text.Min, text.Max, label, NULL, &label_size, size.x >= label_size.x + style.FramePadding.x * 2.0f ? g.Style.ButtonTextAlign : ImVec2(0, 0), &bb);
    return is_pressed;
}

NEKO_INLINE bool button_scrollable(const char *label, const ImVec2 &size_arg) { return button_scrollable_ex(label, size_arg, ImGuiButtonFlags_None); }

struct InputTextCallback_UserData {
    std::string *Str;
    ImGuiInputTextCallback ChainCallback;
    void *ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData *data) {
    InputTextCallback_UserData *user_data = (InputTextCallback_UserData *)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string *str = user_data->Str;
        neko_assert(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char *)str->c_str();
    } else if (user_data->ChainCallback) {
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

NEKO_INLINE bool InputText(const char *label, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputText(label, (char *)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

NEKO_INLINE bool InputTextMultiline(const char *label, std::string *str, const ImVec2 &size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                                    void *user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputTextMultiline(label, (char *)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

NEKO_INLINE bool InputTextWithHint(const char *label, const char *hint, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return ImGui::InputTextWithHint(label, hint, (char *)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

template <typename T, typename... Args>
NEKO_INLINE void TextFmt(T &&fmt, const Args &...args) {
    std::string str = std::format(std::forward<T>(fmt), args...);
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}

}  // namespace neko::imgui

#endif