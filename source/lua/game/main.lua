
CObject = require("cstruct")
NODE = require("prefabs")
local debugging = require("neko.debugging")

neko_ffi = require("neko.ffi")

ecs = require "ecs"

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

local play = fake_game
local running = true

function neko.__define_default_callbacks()

    neko.conf = {
        app = {
            title = "sandbox",
            width = 1280,
            height = 720
        },
        cvar = {
            show_editor = false,
            show_demo_window = false,
            show_pack_editor = false,
            show_profiler_window = false,
            show_test_window = false,
            show_gui = false,
            shader_inspect = true,
            hello_ai_shit = false,
            vsync = false,
            is_hotfix = false,

            -- experimental features 实验性功能
            enable_hotload = true
        }
    }

    neko.game_init_thread = function()

        -- neko.pack_build(neko_file_path("gamedir/res2.pack"),
        --     {"gamedir/assets/textures/cat.aseprite", "gamedir/assets/textures/map1.ase", "gamedir/1.fnt"})

        -- neko.pack_build(neko_file_path("gamedir/res3.pack"), neko_ls(neko_file_path("gamedir")))

        -- local test_pack = neko.pack_construct("test_pack_handle", neko_file_path("gamedir/res3.pack"))
        -- local test_items = neko.pack_items(test_pack)
        -- print(dump_func(test_items))
        -- neko.pack_destroy(test_pack)

        -- play.sub_init_thread()
    end

    neko.game_init = function()
        luainspector = debugging.inspector_init()

        -- pixelui = neko.pixelui_create()

        play.sub_init()

    end

    neko.game_fini = function()
        play.sub_shutdown()

        -- neko.pixelui_end(pixelui)
    end

    neko.game_pre_update = function()
        if running then
            play.sub_pre_update()
        end
    end

    neko.game_loop = function(dt)
        if running then
            play.sub_update(dt)
            play.test_update()
        end

        ecs.progress(dt)

        if neko_key_pressed("NEKO_KEYCODE_F4") then
            require("test")

            -- neko.audio_play(test_audio)

            -- print(dump_func(w))

            -- neko_callback_save("callback1", function(ha)
            --     print("This is callback 1 " .. neko_hash(ha))
            -- end)
            -- neko_callback_save("callback2", function(ha, haa)
            --     print("This is callback 2 " .. ha .. haa)
            -- end)
            -- neko_callback_call("callback1", "haha1")
            -- neko_callback_call("callback2", "haha2", "haha3")
        end
    end

    neko.game_render = function()


        if neko.conf.cvar.shader_inspect then
            ImGui.Begin("Shader")
            local iter_shader = neko.inspect_shaders_iter()
            for shader in iter_shader do
                neko.inspect_shaders(shader)
            end
            local iter_texture = neko.inspect_textures_iter()
            for tex in iter_texture do
                -- neko.inspect_shaders(shader)
                ImGui.Image(tex, 100.0, 100.0)
            end
            ImGui.End()
        end

        -- __neko_luainspector_draw(__neko_luainspector_get())

        debugging.inspector_draw(debugging.inspector_get())

        if running then
            play.sub_render()
        end

        -- local fbs_x, fbs_y = neko_framebuffer_size()

        -- neko.idraw_defaults()
        -- neko.idraw_camera2d(fbs_x, fbs_y)

        -- neko.pixelui_update(pixelui)
        -- -- neko_idraw_rect_textured_ext(idraw, 0, 0, fbs.x, fbs.y, 0, 1, 1, 0, pui->tex_ui.id, NEKO_COLOR_WHITE);

        -- neko.idraw_texture(neko.pixelui_tex(pixelui))
        -- neko.idraw_rectvd(to_vec2(0.0, 0.0), to_vec2(fbs_x, fbs_y), to_vec2(0.0, 0.0), to_vec2(1.0, 1.0),
        --     "R_PRIMITIVE_TRIANGLES", to_color(255, 255, 255, 255))

        -- neko.render_renderpass_begin(0)
        -- neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
        -- neko.idraw_draw()
        -- neko.render_renderpass_end()
    end

end
