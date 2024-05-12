local TEX_WIDTH = 128
local TEX_HEIGHT = 128

local text = ImGui.StringBuf("12344")
local fnt_text = ImGui.StringBuf("中文渲染测试 日本語レンダリングテスト Hello World! ")

local gd = {}

neko.hooks.add("render", "main", function(v)
    ImGui.Begin("Render hooks")
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

        gd.main_fbo = neko.graphics_framebuffer_create()
        gd.main_rt = neko.graphics_texture_create(win_w, win_h, {
            type = "NEKO_GRAPHICS_TEXTURE_2D",
            format = "NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F",
            wrap_s = "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
            wrap_t = "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
            min_filter = "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST",
            mag_filter = "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST"
        })
        gd.main_rp = neko.graphics_renderpass_create(gd.main_fbo, gd.main_rt)

        gd.fbo = neko.graphics_framebuffer_create()
        gd.rt = neko.graphics_texture_create(win_w, win_h)
        gd.rp = neko.graphics_renderpass_create(gd.fbo, gd.rt)

        test_shader = neko.graphics_shader_create("compute", {
            -- VERTEX = sprite_vs,
            -- FRAGMENT = sprite_fs
            COMPUTE = comp_src
        })

        test_uniform = neko.graphics_uniform_create("u_roll", {{
            type = "NEKO_GRAPHICS_UNIFORM_FLOAT"
        }})

        test_tex = neko.graphics_texture_create(TEX_WIDTH, TEX_HEIGHT, {
            type = "NEKO_GRAPHICS_TEXTURE_2D",
            format = "NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F",
            wrap_s = "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
            wrap_t = "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
            min_filter = "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST",
            mag_filter = "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST"
        })

        test_pipeline = neko.graphics_pipeline_create("test_pipeline", {
            compute = {
                shader = test_shader
            }
        })

        test_vec4_data = __neko_cstruct_test.udata(CObject.s["neko_vec4_t"]:size("struct neko_vec4_t"))
        CObject.setter("neko_vec4_t", "x")(test_vec4_data, 0.0)
        CObject.setter("neko_vec4_t", "y")(test_vec4_data, 1.0)
        CObject.setter("neko_vec4_t", "z")(test_vec4_data, 1.0)
        CObject.setter("neko_vec4_t", "w")(test_vec4_data, 1.0)

        test_storage_buffer = neko.graphics_storage_buffer_create("u_voxels", test_vec4_data,
            CObject.s["neko_vec4_t"]:size("struct neko_vec4_t"))

        test_custom_sprite = {}
        test_custom_sprite.tex = neko.graphics_texture_create(10, 10, {
            type = "NEKO_GRAPHICS_TEXTURE_2D",
            data = neko.gen_tex(),
            format = "NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8",
            wrap_s = "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
            wrap_t = "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
            min_filter = "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST",
            mag_filter = "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST"
        })

        test_custom_sprite.tex_userdata = __neko_cstruct_test.udata(CObject.s["__lua_tex_t"]:size("struct __lua_tex_t"))
        CObject.setter("__lua_tex_t", "id")(test_custom_sprite.tex_userdata, test_custom_sprite.tex)

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

        test_custom_sprite.vbo = neko.graphics_vertex_buffer_create("vbo", test_custom_sprite.vdata,
            CObject.s["__lua_quad_vdata_t"]:size("struct __lua_quad_vdata_t"))
        test_custom_sprite.ibo = neko.graphics_index_buffer_create("ibo", test_custom_sprite.idata,
            CObject.s["__lua_quad_idata_t"]:size("struct __lua_quad_idata_t"))

        test_custom_sprite.uniform = neko.graphics_uniform_create("u_tex", {{
            type = "NEKO_GRAPHICS_UNIFORM_SAMPLER2D"
        }}, "NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT")

        test_custom_sprite.shader = neko.graphics_shader_create("quad", {
            VERTEX = custom_sprite_vs,
            FRAGMENT = custom_sprite_fs
        })

        test_custom_sprite.v_attr, test_custom_sprite.v_attr_size =
            neko.graphics_vertex_attribute_create("vertex_attribute_name", {{
                name = "a_pos",
                format = "NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2"
            }, {
                name = "a_uv",
                format = "NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2"
            }})

        test_custom_sprite.pipeline = neko.graphics_pipeline_create("test_pipeline", {
            raster = {
                shader = test_custom_sprite.shader,
                index_buffer_element_size = ffi.sizeof("uint32_t")
            },
            layout = {
                attrs = test_custom_sprite.v_attr,
                size = test_custom_sprite.v_attr_size
            }
        })

        local texture_list = {"gamedir/assets/textures/dragon_zombie.png", "gamedir/assets/textures/night_spirit.png"}
        gd.test_batch = neko.sprite_batch_create(32, texture_list, batch_vs, batch_ps)

    end,

    sub_shutdown = function()

        neko.sprite_batch_end(gd.test_batch)

        neko.graphics_renderpass_destroy(gd.rp)
        neko.graphics_texture_destroy(gd.rt)
        neko.graphics_framebuffer_destroy(gd.fbo)

        neko.graphics_renderpass_destroy(gd.main_rp)
        neko.graphics_texture_destroy(gd.main_rt)
        neko.graphics_framebuffer_destroy(gd.main_fbo)
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

        neko.graphics_pipeline_bind(test_pipeline)
        neko.graphics_apply_bindings({
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
        neko.graphics_dispatch_compute(TEX_WIDTH / 16, TEX_HEIGHT / 16, 1)

        local v1 = to_vec2(0.0, 0.0)
        local v2 = to_vec2(TEX_WIDTH, TEX_HEIGHT)

        neko.idraw_defaults()
        neko.idraw_camera2d(fbs_x, fbs_y)

        neko.idraw_texture(test_tex)
        neko.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES",
            to_color(255, 255, 255, 255))

        neko.graphics_renderpass_begin(gd.main_rp)
        neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        neko.graphics_clear(0.0, 0.0, 0.0, 0.0)
        neko.idraw_draw()
        neko.graphics_renderpass_end()

        neko.graphics_renderpass_begin(gd.main_rp)
        neko.graphics_pipeline_bind(test_custom_sprite.pipeline)
        neko.graphics_apply_bindings({
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
        neko.graphics_draw({
            start = 0,
            count = 6
        })
        neko.graphics_renderpass_end()

        -- neko.idraw_defaults()
        -- neko.idraw_camera3d(fbs_x, fbs_y)
        -- neko.idraw_translatef(0.0, 0.0, -2.0)
        -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 1.0, 0.0, 0.0)
        -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 0.0, 1.0, 0.0)
        -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 0.0, 0.0, 1.0)
        -- neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 200, 100, 50, 255, "NEKO_GRAPHICS_PRIMITIVE_LINES")

        -- neko.graphics_renderpass_begin(gd.rp)
        -- neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        -- neko.graphics_clear(0.0, 0.0, 0.0, 1.0)
        -- neko.idraw_draw()
        -- neko.graphics_renderpass_end()

        -- neko.idraw_camera3d(fbs_x, fbs_y)
        -- neko.idraw_depth_enabled(true)
        -- neko.idraw_face_cull_enabled(true)
        -- neko.idraw_translatef(0.0, 0.0, -1.0)
        -- neko.idraw_texture(gd.rt)
        -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0001, 1.0, 0.0, 0.0)
        -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0002, 0.0, 1.0, 0.0)
        -- neko.idraw_rotatev(neko_platform_elapsed_time() * 0.0003, 0.0, 0.0, 1.0)
        -- neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 255, 255, 255, 255, "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES")

        -- neko.graphics_renderpass_begin(0)
        -- neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        -- neko.idraw_draw()
        -- neko.graphics_renderpass_end()

        neko.idraw_defaults()
        neko.idraw_camera2d(fbs_x, fbs_y)
        neko.idraw_texture(gd.main_rt)
        neko.idraw_rectvd(to_vec2(0.0, 0.0), to_vec2(win_w, win_h), to_vec2(0.0, 0.0), to_vec2(1.0, 1.0),
            "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES", to_color(255, 255, 255, 255))

        neko.graphics_renderpass_begin(0)
        neko.graphics_set_viewport(0.0, 0.0, win_w, win_h)
        neko.idraw_draw()
        neko.graphics_renderpass_end()

        neko.sprite_batch_render_ortho(gd.test_batch, fbs_x, fbs_y, 0, 0)
        neko.sprite_batch_render_begin(gd.test_batch)

        local dragon_zombie = neko.sprite_batch_make_sprite(gd.test_batch, 0, 250, 200, 1, common.rad2deg(t / 100000.0),
            0)
        local night_spirit = neko.sprite_batch_make_sprite(gd.test_batch, 1, 0, 200, 1, 0, 0)
        neko.sprite_batch_push_sprite(gd.test_batch, dragon_zombie)
        neko.sprite_batch_push_sprite(gd.test_batch, night_spirit)

        -- for i = 0, 4 do
        --     local polish = neko.sprite_batch_make_sprite(gd.test_batch, 1, 200, 880, 1, common.rad2deg(t / 50000.0), 0)
        --     local translated = polish
        --     local polish_x = CObject.getter("neko_sprite_t", "x")(polish)
        --     local polish_y = CObject.getter("neko_sprite_t", "y")(polish)
        --     local polish_sx = CObject.getter("neko_sprite_t", "sx")(polish)
        --     local polish_sy = CObject.getter("neko_sprite_t", "sy")(polish)
        --     for j = 0, 6 do
        --         CObject.setter("neko_sprite_t", "x")(translated, polish_x + polish_sx * i)
        --         CObject.setter("neko_sprite_t", "y")(translated, polish_y + polish_sy * j)
        --         neko.sprite_batch_push_sprite(gd.test_batch, translated)
        --     end
        -- end

        neko.sprite_batch_render_end(gd.test_batch)

    end,
    test_update = function()

    end
}

return M
