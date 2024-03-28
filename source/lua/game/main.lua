imgui = require("imgui")
CObject = require("cobject")
CVar = neko.cvar
ECS = require("common/ecs")
tweens = require("common/tweens")

-- fake game proxy
local fake_game = {
    sub_init_thread = function()

    end,
    sub_init = function()

    end,
    sub_shutdown = function()

    end,
    sub_pre_update = function()

    end,
    sub_update = function(dt)

    end,
    sub_render = function()

    end,
    test_update = function()

    end
}

neko_game = {
    app = {
        title = "sandbox",
        width = 1280,
        height = 720
    },
    cvar = {
        show_demo_window = false,
        show_physics_debug = false,

        -- experimental features 实验性功能
        enable_hotload = true
    }
}

local play = fake_game
local running = true

local TEX_WIDTH = 512
local TEX_HEIGHT = 512

local text = imgui.StringBuf("12344")

game_init_thread = function()
    play.sub_init_thread()
end

local gd = {}

game_init = function()

    local win_w, win_h = neko_window_size(neko_main_window())

    gd.fbo = neko.graphics_framebuffer_create()
    gd.rt = neko.graphics_texture_create(win_w, win_h)
    gd.rp = neko.graphics_renderpass_create(gd.fbo, gd.rt)

    test_ase_witch = neko.aseprite.create(neko_file_path("gamedir/assets/textures/B_witch.ase"))
    test_ase_harvester = neko.aseprite.create(neko_file_path("gamedir/assets/textures/TheHarvester.ase"))
    test_ase_pdx = neko.aseprite.create(neko_file_path("gamedir/assets/textures/pdx.ase"))

    test_shader = neko.graphics_shader_create("compute", {
        -- VERTEX = sprite_vs,
        -- FRAGMENT = sprite_fs
        COMPUTE = comp_src
    })

    test_uniform = neko.graphics_uniform_create("u_roll", {{
        type = "NEKO_GRAPHICS_UNIFORM_FLOAT"
    }})

    test_tex = neko.graphics_texture_create(TEX_WIDTH, TEX_HEIGHT, "NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F",
        "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT", "NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT",
        "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST", "NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST")

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

    play.sub_init()
end

game_shutdown = function()
    play.sub_shutdown()

    neko.graphics_renderpass_destroy(gd.rp)
    neko.graphics_texture_destroy(gd.rt)
    neko.graphics_framebuffer_destroy(gd.fbo)
end

game_pre_update = function()
    if running then
        play.sub_pre_update()
    end
end

game_update = function(dt)
    if running then
        play.sub_update(dt)
        play.test_update()
    end
end

game_render = function()

    local fbs_x, fbs_y = neko_framebuffer_size()

    neko.idraw_defaults()
    neko.idraw_camera2d(fbs_x, fbs_y)

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

    neko.idraw_texture(test_tex)
    neko.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "NEKO_GRAPHICS_PRIMITIVE_TRIANGLES",
        to_color(255, 255, 255, 255))

    -- neko.graphics_test(test_uniform, test_tex, test_pipeline)

    neko.graphics_renderpass_begin(0)
    neko.graphics_set_viewport(0.0, 0.0, fbs_x, fbs_y)
    neko.idraw_draw()
    neko.graphics_renderpass_end()

    imgui.Begin("Demo")
    imgui.Text("选择测试Demo")
    if imgui.Button("Sandbox") then
        -- running = false
        play.sub_shutdown()
        collectgarbage()
        play = require("game_sandbox")
        play.sub_init()
        play.sub_init_thread()
    end
    -- if imgui.Button("Run") then
    --     running = true
    -- end
    imgui.Separator()
    if imgui.InputText("TEST", text) then
        print(tostring(text))
    end

    -- imgui.Image(test_tex, 100.0, 100.0)
    imgui.End()

    if running then
        play.sub_render()
    end

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

end

