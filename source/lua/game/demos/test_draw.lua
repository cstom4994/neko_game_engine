local TEX_WIDTH = 128
local TEX_HEIGHT = 128

local text = ImGui.StringBuf("12344")
local fnt_text = ImGui.StringBuf("中文渲染测试 日本語レンダリングテスト Hello World! ")

local gd = {}

neko.hooks.add("render", "main", function(v)
    ImGui.Begin("render hooks")
    if ImGui.InputText("TEST", fnt_text) then
    end
    ImGui.Text("%s", v)
    ImGui.End()

    neko.draw_text(50.0, 50.0, tostring(fnt_text), 3.0)
end)

-- hooks.add("inputPressed","test",function(v)
--     ImGui.Begin("inputPressed hooks")
--     ImGui.Text("%s", v)
--     ImGui.End()
-- end)

local M = {

    sub_init_thread = function()

    end,
    sub_init = function()
        local win_w, win_h = neko_window_size(neko_main_window())

        gd.main_fbo = neko.render_framebuffer_create()
        gd.main_rt = neko.render_texture_create(win_w, win_h, {
            type = "R_TEXTURE_2D",
            format = "R_TEXTURE_FORMAT_RGBA32F",
            wrap_s = "R_TEXTURE_WRAP_REPEAT",
            wrap_t = "R_TEXTURE_WRAP_REPEAT",
            min_filter = "R_TEXTURE_FILTER_NEAREST",
            mag_filter = "R_TEXTURE_FILTER_NEAREST"
        })
        gd.main_rp = neko.render_renderpass_create(gd.main_fbo, gd.main_rt)

        gd.fbo = neko.render_framebuffer_create()
        gd.rt = neko.render_texture_create(win_w, win_h)
        gd.rp = neko.render_renderpass_create(gd.fbo, gd.rt)

        test_shader = neko.render_shader_create("compute", {
            -- VERTEX = sprite_vs,
            -- FRAGMENT = sprite_fs
            COMPUTE = comp_src
        })

        test_uniform = neko.render_uniform_create("u_roll", {{
            type = "R_UNIFORM_FLOAT"
        }})

        test_tex = neko.render_texture_create(TEX_WIDTH, TEX_HEIGHT, {
            type = "R_TEXTURE_2D",
            format = "R_TEXTURE_FORMAT_RGBA32F",
            wrap_s = "R_TEXTURE_WRAP_REPEAT",
            wrap_t = "R_TEXTURE_WRAP_REPEAT",
            min_filter = "R_TEXTURE_FILTER_NEAREST",
            mag_filter = "R_TEXTURE_FILTER_NEAREST"
        })

        test_pipeline = neko.render_pipeline_create("test_pipeline", {
            compute = {
                shader = test_shader
            }
        })

        test_vec4_data = __neko_cstruct_test.udata(CObject.s["neko_vec4_t"]:size("struct neko_vec4_t"))
        CObject.setter("neko_vec4_t", "x")(test_vec4_data, 0.0)
        CObject.setter("neko_vec4_t", "y")(test_vec4_data, 1.0)
        CObject.setter("neko_vec4_t", "z")(test_vec4_data, 1.0)
        CObject.setter("neko_vec4_t", "w")(test_vec4_data, 1.0)

        test_storage_buffer = neko.render_storage_buffer_create("u_voxels", test_vec4_data,
            CObject.s["neko_vec4_t"]:size("struct neko_vec4_t"))

        test_custom_sprite = {}
        test_custom_sprite.tex = neko.render_texture_create(10, 10, {
            type = "R_TEXTURE_2D",
            data = neko.gen_tex(),
            format = "R_TEXTURE_FORMAT_RGBA8",
            wrap_s = "R_TEXTURE_WRAP_REPEAT",
            wrap_t = "R_TEXTURE_WRAP_REPEAT",
            min_filter = "R_TEXTURE_FILTER_NEAREST",
            mag_filter = "R_TEXTURE_FILTER_NEAREST"
        })

        test_custom_sprite.tex_userdata = __neko_cstruct_test.udata(CObject.s["__lua_tex_t"]:size("struct __lua_tex_t"))
        CObject.setter("__lua_tex_t", "id")(test_custom_sprite.tex_userdata, test_custom_sprite.tex)

        print(CObject.generate_array_type("__lua_quad_vdata_t", "float", 16))

        local v = {-0.5, -0.5, 0.0, 0.0, 0.5, -0.5, 1.0, 0.0, -0.5, 0.5, 0.0, 1.0, 0.5, 0.5, 1.0, 1.0}
        test_custom_sprite.vdata = __neko_cstruct_test.udata(
            CObject.s["__lua_quad_vdata_t"]:size("struct __lua_quad_vdata_t"))

        for i, v in ipairs(v) do
            CObject.setter("__lua_quad_vdata_t", "v" .. i)(test_custom_sprite.vdata, v)
        end

        v = {0, 3, 2, 0, 1, 3}
        test_custom_sprite.idata = __neko_cstruct_test.udata(
            CObject.s["__lua_quad_idata_t"]:size("struct __lua_quad_idata_t"))
        for i, v in ipairs(v) do
            CObject.setter("__lua_quad_idata_t", "v" .. i)(test_custom_sprite.idata, v)
        end

        test_custom_sprite.vbo = neko.render_vertex_buffer_create("vbo", test_custom_sprite.vdata,
            CObject.s["__lua_quad_vdata_t"]:size("struct __lua_quad_vdata_t"))
        test_custom_sprite.ibo = neko.render_index_buffer_create("ibo", test_custom_sprite.idata,
            CObject.s["__lua_quad_idata_t"]:size("struct __lua_quad_idata_t"))

        test_custom_sprite.uniform = neko.render_uniform_create("u_tex", {{
            type = "R_UNIFORM_SAMPLER2D"
        }}, "R_SHADER_STAGE_FRAGMENT")

        test_custom_sprite.shader = neko.render_shader_create("quad", {
            VERTEX = custom_sprite_vs,
            FRAGMENT = custom_sprite_fs
        })

        test_custom_sprite.v_attr, test_custom_sprite.v_attr_size =
            neko.render_vertex_attribute_create("vertex_attribute_name", {{
                name = "a_pos",
                format = "R_VERTEX_ATTRIBUTE_FLOAT2"
            }, {
                name = "a_uv",
                format = "R_VERTEX_ATTRIBUTE_FLOAT2"
            }})

        test_custom_sprite.pipeline = neko.render_pipeline_create("test_pipeline", {
            raster = {
                shader = test_custom_sprite.shader,
                index_buffer_element_size = ffi.sizeof("uint32_t")
            },
            layout = {
                attrs = test_custom_sprite.v_attr,
                size = test_custom_sprite.v_attr_size
            }
        })
    end,

    sub_shutdown = function()
        neko.render_renderpass_destroy(gd.rp)
        neko.render_texture_destroy(gd.rt)
        neko.render_framebuffer_destroy(gd.fbo)

        neko.render_renderpass_destroy(gd.main_rp)
        neko.render_texture_destroy(gd.main_rt)
        neko.render_framebuffer_destroy(gd.main_fbo)
    end,

    sub_pre_update = function()

    end,
    sub_update = function(dt)

    end,
    sub_render = function()
        local t = neko_platform_elapsed_time()

        local fbs_x = 640
        local fbs_y = 360

        local win_w, win_h = neko_window_size(neko_main_window())

        local roll = neko_platform_elapsed_time() * 0.001

        neko.render_pipeline_bind(test_pipeline)
        neko.render_apply_bindings({
            uniforms = {{
                uniform = test_uniform,
                data = roll
            }},
            image_buffers = {{
                tex = test_tex,
                binding = 0
            }},
            storage_buffers = {{
                buffer = test_storage_buffer,
                binding = 1
            }}
        })
        neko.render_dispatch_compute(TEX_WIDTH / 16, TEX_HEIGHT / 16, 1)

        local v1 = to_vec2(0.0, 0.0)
        local v2 = to_vec2(TEX_WIDTH, TEX_HEIGHT)

        neko.idraw_defaults()
        neko.idraw_camera2d(fbs_x, fbs_y)

        neko.idraw_texture(test_tex)
        neko.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "R_PRIMITIVE_TRIANGLES",
            to_color(255, 255, 255, 255))

        neko.render_renderpass_begin(gd.main_rp)
        neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        neko.render_clear(0.0, 0.0, 0.0, 0.0)
        neko.idraw_draw()
        neko.render_renderpass_end()

        neko.render_renderpass_begin(gd.main_rp)
        neko.render_pipeline_bind(test_custom_sprite.pipeline)
        neko.render_apply_bindings({
            uniforms = {{
                uniform = test_custom_sprite.uniform,
                data = test_custom_sprite.tex_userdata,
                binding = 0
            }},
            vertex_buffers = {{
                buffer = test_custom_sprite.vbo
            }},
            index_buffers = {{
                buffer = test_custom_sprite.ibo
            }}
        })
        neko.render_draw({
            start = 0,
            count = 6
        })
        neko.render_renderpass_end()

        neko.idraw_defaults()
        neko.idraw_camera3d(fbs_x, fbs_y)
        neko.idraw_translatef(0.0, 0.0, -2.0)
        neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 1.0, 0.0, 0.0)
        neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 0.0, 1.0, 0.0)
        neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 0.0, 0.0, 1.0)
        neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 200, 100, 50, 255, "R_PRIMITIVE_LINES")

        neko.render_renderpass_begin(gd.rp)
        neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        neko.render_clear(0.0, 0.0, 0.0, 1.0)
        neko.idraw_draw()
        neko.render_renderpass_end()

        neko.idraw_camera3d(fbs_x, fbs_y)
        neko.idraw_depth_enabled(true)
        neko.idraw_face_cull_enabled(true)
        neko.idraw_translatef(0.0, 0.0, -1.0)
        neko.idraw_texture(gd.rt)
        neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 1.0, 0.0, 0.0)
        neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0002, 0.0, 1.0, 0.0)
        neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0003, 0.0, 0.0, 1.0)
        neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 255, 255, 255, 255, "R_PRIMITIVE_TRIANGLES")

        neko.render_renderpass_begin(0)
        neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        neko.idraw_draw()
        neko.render_renderpass_end()

        neko.idraw_defaults()
        neko.idraw_camera2d(fbs_x, fbs_y)
        neko.idraw_texture(gd.main_rt)
        neko.idraw_rectvd(to_vec2(0.0, 0.0), to_vec2(win_w, win_h), to_vec2(0.0, 0.0), to_vec2(1.0, 1.0),
            "R_PRIMITIVE_TRIANGLES", to_color(255, 255, 255, 255))

        neko.render_renderpass_begin(0)
        neko.render_set_viewport(0.0, 0.0, win_w, win_h)
        neko.idraw_draw()
        neko.render_renderpass_end()
    end,
    test_update = function()

    end
}

return M
