ImGui = require("neko.imgui")
CObject = require("cobject")
CVar = neko.cvar
NODE = require("common/node")
local debugging = require("neko.debugging")

neko_ffi = require("neko.ffi")

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

local play = fake_game
local running = true

game_init_thread = function()

    -- neko.pack_build(neko_file_path("gamedir/res2.pack"),
    --     {"gamedir/assets/textures/cat.aseprite", "gamedir/assets/textures/map1.ase", "gamedir/1.fnt"})

    -- neko.pack_build(neko_file_path("gamedir/res3.pack"), neko_ls(neko_file_path("gamedir")))

    -- local test_pack = neko.pack_construct("test_pack_handle", neko_file_path("gamedir/res3.pack"))
    -- local test_items = neko.pack_items(test_pack)
    -- print(dump_func(test_items))
    -- neko.pack_destroy(test_pack)

    play.sub_init_thread()
end

game_init = function()
    luainspector = debugging.inspector_init()

    -- pixelui = neko.pixelui_create()

    play.sub_init()
end

game_shutdown = function()
    play.sub_shutdown()

    -- neko.pixelui_end(pixelui)
end

game_pre_update = function()
    if running then
        play.sub_pre_update()
    end
end

game_loop = function(dt)
    if running then
        play.sub_update(dt)
        play.test_update()
    end

    if neko_key_pressed("NEKO_KEYCODE_F4") then
        neko_dolua("lua_scripts/tests/test_cstruct.lua")
        neko_dolua("lua_scripts/tests/test_class.lua")
        neko_dolua("lua_scripts/tests/test_ds.lua")
        neko_dolua("lua_scripts/tests/test_events.lua")
        neko_dolua("lua_scripts/tests/test_common.lua")
        neko_dolua("lua_scripts/tests/nekolua_1.lua")
        neko_dolua("lua_scripts/tests/nekolua_2.lua")

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

game_render = function()
    ImGui.Begin("Demo")
    ImGui.Text("选择测试Demo")

    if ImGui.Button("Sandbox") then
        play.sub_shutdown()
        collectgarbage()
        play = require("demos/sandbox")
        play.sub_init()
        play.sub_init_thread()
    end

    if ImGui.Button("fluid") then
        play.sub_shutdown()
        collectgarbage()
        play = require("demos/fluid")
        play.sub_init()
        play.sub_init_thread()
    end

    if ImGui.Button("test_draw") then
        play.sub_shutdown()
        collectgarbage()
        play = require("demos/test_draw")
        play.sub_init()
        play.sub_init_thread()
    end

    if ImGui.Button("test_ecs") then

        for i = 1, 10, 1 do
            local w = neko.ecs_f()
            local e = w:create_ent()
            w:attach(e, "position_t", "velocity_t")
            print(e)
            local tb, ss = w:get_com()
            print(ss)
            dump_func(tb)

            local hash = neko_hash_str("bounds_t")
            if tb["comp_map"][hash] then
                print(tb["comp_map"][hash].id)
            else
                print("component not found")
            end
        end

        -- for i = 1, 1024 * 256, 1 do
        --     local w = neko.ecs_f()
        --     local e = w:create_ent()
        --     w:attach(e, "position_t", "velocity_t")
        --     -- print(e)
        -- end

        collectgarbage()

        -- for i, k in pairs(tb) do
        --     print(i, k)
        -- end
        -- local jsondata, err = neko.json_write(tb)
        -- print(jsondata)
    end

    if ImGui.Button("test_prefab") then
        local file_content = read_file("source/lua/game/test_prefab.neko")
        if file_content then
            local map_node = NODE.load(file_content)
            print(NODE.check(map_node))
            print(NODE.node_type(map_node))
            dump_func(map_node)
        else
            print("无法打开文件或文件不存在")
        end

        local encoded = neko.base64_encode(file_content)
        print("Encoded:", encoded)

        local decoded = neko.base64_decode(encoded)
        print("Decoded:", decoded)
    end

    if ImGui.Button("test_profiler") then

        local function fact(n)
            if n <= 1 then
                return 1
            else
                return fact(n - 1) * n
            end
        end

        local function foo(n)
            local s = 0
            for i = 1, n do
                s = s + fact(i)
            end
            return s
        end

        neko.profiler_start(10000, 10)

        foo(2000)

        local info, n, t = neko.profiler_info()

        neko.profiler_stop()

        for filename, line_t in pairs(info) do
            for line, count in pairs(line_t) do
                print(filename, line, count)
            end
        end

        print("total=", n, "t=", t)

    end

    if ImGui.Button("test_cffi") then

        local holy_shit_dll
        do
            holy_shit_dll = cffi.loadlib('hello.dll', {
                hello_shit = cffi.makecif {
                    ret = cffi.sint32,
                    cffi.sint32,
                    cffi.sint32
                }
            })
        end

        do
            print(holy_shit_dll.hello_shit(100, 444))
        end

    end

    if ImGui.Button("test_debugging") then
        debugging.breakpoint()
    end

    if ImGui.Button("test_luastate") then
        local s = require("neko.lowlua")

        local lua_getregistry = s.lua_getregistry or function()
            return s.lua_pushvalue(LUA_REGISTRYINDEX);
        end
        local lua_getglobal = s.lua_getglobal or function(s)
            assert(isstring(s));
            return s.lua_getfield(LUA_GLOBALSINDEX, s);
        end
        local lua_setglobal = s.lua_setglobal or function(s)
            assert(isstring(s));
            return s.lua_setfield(LUA_GLOBALSINDEX, s);
        end
        local lua_pop = s.lua_pop or function(n)
            assert(isnumber(n));
            return s.lua_settop(-(n) - 1);
        end
        local lua_newtable = s.lua_newtable or function()
            return s.lua_createtable(0, 0);
        end
        local lua_register = s.lua_register or function(f, s)
            assert(isfunction(f) and isstring(s));
            s.lua_pushcclosure(f, 0);
            s.lua_setglobal(s);
            return nil;
        end
        local lua_strlen = s.lua_strlen or function(i)
            assert(isnumber(i));
            return s.lua_objlen(i);
        end

        s.lua_settop(0)
        s.lua_getglobal("print")
        s.lua_pushstring("I am a string to be printed!")
        s.lua_call(1, 0)
    end

    if ImGui.Button("test_luastruct") then
        local v3 = neko.VEC3.new()
        v3.x = 10
        v3.y = 10
        v3.z = 10
        local v3_c = neko.LUASTRUCT_test_vec3(v3)
        dump_func({v3_c.x, v3_c.y, v3_c.z})
    end

    if ImGui.Button("luadb") then
        local db = luadb.open("data.lua")
        print("a=", db.a)
        print("b=", db.b)
        print("#c=", #db.c)
        for k, v in pairs(db.c) do
            print("pairs c:", k, v)
        end
        for k, v in ipairs(db.c) do
            print("ipairs c:", k, v)
        end
        for k, v in pairs(db) do
            print("pairs", k, v)
        end
    end

    if ImGui.Button("sound") then
        neko_sound_load("gamedir/assets/audio/Run_It_Might_Be_Somebody.wav"):start()
    end
    
    if ImGui.Button("pack") then
        local test_pack = pack.construct("test_pack_handle", neko_file_path("gamedir/sc.pack"))
        local test_items = pack.items(test_pack)
        dump_func(test_items)
        pack.destroy(test_pack)
    end

    -- ImGui.Checkbox(neko_game.cvar.shader_inspect)

    ImGui.Separator()
    -- if ImGui.InputText("TEST", text) then
    --     print(tostring(text))
    -- end

    -- ImGui.Image(test_tex, 100.0, 100.0)
    -- ImGui.Image(test_custom_sprite.tex, 100.0, 100.0)
    ImGui.End()

    if neko_game.cvar.shader_inspect then
        ImGui.Begin("Shader")
        local iter = neko.render_shader_iterator()
        for shader in iter do
            neko.render_shader_inspector(shader)
        end
        ImGui.End()
    end

    -- __neko_luainspector_draw(__neko_luainspector_get())

    debugging.inspector_draw(debugging.inspector_get())

    if running then
        play.sub_render()
    end

    neko.events.run()

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
