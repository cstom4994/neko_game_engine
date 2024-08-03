lust = require("lua/libs/lust")
local describe, it, expect = lust.describe, lust.it, lust.expect

cvar = function(name)
    local w = neko_w_f()
    local tb, ss = w:get_com()
    local hash = neko_hash_str(name)
    if tb["cvar_map"][hash] then
        return tb["cvar_map"][hash].data
    else
        return nil
    end
end

function init_lui()
    mu = neko.microui

    log_input = mu.ref ""

    logbuf = mu.ref ""
    logbuf_updated = false

    bg = {
        r = mu.ref(90),
        g = mu.ref(95),
        b = mu.ref(100)
    }
    checks = map({true, false, true}, mu.ref)

    local function make_color(tab)
        return {
            r = mu.ref(tab[1]),
            g = mu.ref(tab[2]),
            b = mu.ref(tab[3]),
            a = mu.ref(tab[4])
        }
    end

    colors = {{"text:", mu.COLOR_TEXT, make_color {230, 230, 230, 255}},
              {"border:", mu.COLOR_BORDER, make_color {25, 25, 25, 255}},
              {"windowbg:", mu.COLOR_WINDOWBG, make_color {50, 50, 50, 255}},
              {"titlebg:", mu.COLOR_TITLEBG, make_color {25, 25, 25, 255}},
              {"titletext:", mu.COLOR_TITLETEXT, make_color {240, 240, 240, 255}},
              {"panelbg:", mu.COLOR_PANELBG, make_color {0, 0, 0, 0}},
              {"button:", mu.COLOR_BUTTON, make_color {75, 75, 75, 255}},
              {"buttonhover:", mu.COLOR_BUTTONHOVER, make_color {95, 95, 95, 255}},
              {"buttonfocus:", mu.COLOR_BUTTONFOCUS, make_color {115, 115, 115, 255}},
              {"base:", mu.COLOR_BASE, make_color {30, 30, 30, 255}},
              {"basehover:", mu.COLOR_BASEHOVER, make_color {35, 35, 35, 255}},
              {"basefocus:", mu.COLOR_BASEFOCUS, make_color {40, 40, 40, 255}},
              {"scrollbase:", mu.COLOR_SCROLLBASE, make_color {43, 43, 43, 255}},
              {"scrollthumb:", mu.COLOR_SCROLLTHUMB, make_color {30, 30, 30, 255}}}
end

function write_log(str)
    local log = logbuf:get()
    if #log ~= 0 then
        log = log .. "\n"
    end
    log = log .. str
    logbuf:set(log)
    logbuf_updated = true
end

function test_lui()
    test_window()
    log_window()
    style_window()
end

function test_window()
    -- do window
    if mu.begin_window("Demo Window", mu.rect(40, 40, 300, 450)) then
        local win = mu.get_current_container()
        local rect = win:rect()
        win:set_rect{
            x = rect.x,
            y = rect.y,
            w = math.max(rect.w, 240),
            h = math.max(rect.h, 300)
        }

        -- window info
        if mu.header "Window Info" then
            local win = mu.get_current_container()
            local rect = win:rect()
            mu.layout_row({54, -1}, 0)
            mu.label "Position:"
            mu.label(("%d, %d"):format(rect.x, rect.y))
            mu.label "Size:"
            mu.label(("%d, %d"):format(rect.w, rect.h))
        end

        -- labels + buttons
        if mu.header("Test Buttons", mu.OPT_EXPANDED) then
            mu.layout_row({86, -110, -1}, 0)
            mu.label "Test buttons 1:"
            if mu.button "Button 1" then
                write_log "Pressed button 1"
            end
            if mu.button "Button 2" then
                write_log "Pressed button 2"
            end
            mu.label "Test buttons 2:"
            if mu.button "Button 3" then
                write_log "Pressed button 3"
            end
            if mu.button "Popup" then
                mu.open_popup "Test Popup"
            end
            if mu.begin_popup "Test Popup" then
                mu.button "Hello"
                mu.button "World"
                mu.end_popup()
            end
        end

        -- tree
        if mu.header("Tree and Text", mu.OPT_EXPANDED) then
            mu.layout_row({140, -1}, 0)
            mu.layout_begin_column()
            if mu.begin_treenode "Test 1" then
                if mu.begin_treenode "Test 1a" then
                    mu.label "Hello"
                    mu.label "World"
                    mu.end_treenode()
                end
                if mu.begin_treenode "Test 1b" then
                    if mu.button "Button 1" then
                        write_log "Pressed button 1"
                    end
                    if mu.button "Button 2" then
                        write_log "Pressed button 2"
                    end
                    mu.end_treenode()
                end
                mu.end_treenode()
            end
            if mu.begin_treenode "Test 2" then
                mu.layout_row({54, 54}, 0)
                if mu.button "Button 3" then
                    write_log "Pressed button 3"
                end
                if mu.button "Button 4" then
                    write_log "Pressed button 4"
                end
                if mu.button "Button 5" then
                    write_log "Pressed button 5"
                end
                if mu.button "Button 6" then
                    write_log "Pressed button 6"
                end
                mu.end_treenode()
            end
            if mu.begin_treenode "Test 3" then
                mu.checkbox("Checkbox 1", checks[1])
                mu.checkbox("Checkbox 2", checks[2])
                mu.checkbox("Checkbox 3", checks[3])
                mu.end_treenode()
            end
            mu.layout_end_column()

            mu.layout_begin_column()
            mu.layout_row({-1}, 0)
            mu.text(
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus ipsum, eu varius magna felis a nulla.")
            mu.layout_end_column()
        end

        -- background color sliders
        if mu.header("Background Color", mu.OPT_EXPANDED) then
            mu.layout_row({-78, -1}, 74)
            mu.layout_begin_column()
            mu.layout_row({46, -1}, 0)
            mu.label "Red:";
            mu.slider(bg.r, 0, 255)
            mu.label "Green:";
            mu.slider(bg.g, 0, 255)
            mu.label "Blue:";
            mu.slider(bg.b, 0, 255)
            mu.layout_end_column()

            local r = mu.layout_next()
            local col = {
                r = math.floor(bg.r:get()),
                g = math.floor(bg.g:get()),
                b = math.floor(bg.b:get()),
                a = 255
            }
            neko.clear_color(col.r, col.g, col.b, col.a)
            mu.draw_rect(r, col)
            local str = ("#%02X%02X%02X"):format(col.r, col.g, col.b)
            mu.draw_control_text(str, r, mu.COLOR_TEXT, mu.OPT_ALIGNCENTER)
        end

        mu.end_window()
    end
end

function log_window()
    if mu.begin_window("Log Window", mu.rect(350, 40, 300, 200)) then
        -- output text panel
        mu.layout_row({-1}, -25)
        mu.begin_panel "Log Output"
        local panel = mu.get_current_container()
        mu.layout_row({-1}, -1)
        mu.text(logbuf:get())
        mu.end_panel()
        if logbuf_updated then
            local sx, sy = panel:scroll()
            local x, y = panel:content_size()
            panel:set_scroll(sx, y)
            logbuf_updated = false
        end

        -- input textbox + buttons
        local submitted = false
        mu.layout_row({-120, -60, -1}, 0)
        if (mu.textbox(log_input) and mu.RES_SUBMIT) ~= 0 then
            mu.set_focus(mu.get_last_id())
            submitted = true
        end
        if mu.button "Submit" then
            submitted = true
        end
        if mu.button "Clear" then
            logbuf:set ""
        end
        if submitted then
            write_log(log_input:get())
            log_input:set ""
        end

        mu.end_window()
    end
end

function uint8_slider(label, palette, key)
    mu.push_id(label .. key)
    mu.slider(palette[key], 0, 255, 0, "%.0f")
    mu.pop_id()
end

function style_window()
    if mu.begin_window("Style Editor", mu.rect(350, 250, 300, 240)) then
        local style = mu.get_style()

        local sw = mu.get_current_container():body().w * 0.14
        mu.layout_row({80, sw, sw, sw, sw, -1}, 0)
        for _, color in ipairs(colors) do
            local label, idx, palette = color[1], color[2], color[3]
            mu.label(label)
            uint8_slider(label, palette, "r")
            uint8_slider(label, palette, "g")
            uint8_slider(label, palette, "b")
            uint8_slider(label, palette, "a")
            local c = {
                r = palette.r:get(),
                g = palette.g:get(),
                b = palette.b:get(),
                a = palette.a:get()
            }
            mu.draw_rect(mu.layout_next(), c)
            style:set_color(idx, c)
        end
        mu.end_window()
    end
end


draw_imgui = function(dt)

    ImGui.Begin("Demo")
    ImGui.Text("选择测试Demo")

    if ImGui.Button("test_w") then

        local w = neko_w_f()

        local tb, ss = w:get_com()
        print(ss)
        dump_func(tb)

        local hash = neko_hash_str("settings_window_frame_rate")
        if tb["cvar_map"][hash] then
            print(tb["cvar_map"][hash].data)
        else
            print("cvar not found")
        end

    end

    if ImGui.Button("test_load_libs") then
        test_load_libs()
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

        Core.profiler_start(10000, 10)

        foo(2000)

        local info, n, t = Core.profiler_info()

        Core.profiler_stop()

        for filename, line_t in pairs(info) do
            for line, count in pairs(line_t) do
                print(filename, line, count)
            end
        end

        print("total=", n, "t=", t)

    end

    if ImGui.Button("test_debugging") then
        Inspector.breakpoint()
    end

    if ImGui.Button("sound") then

        local music = neko.sound_load(neko.file_path("Run_It_Might_Be_Somebody.wav"))
        music:set_loop(true)
        music:set_vol(0.25)
        music:start()
    end

    if ImGui.Button("tolua_gen") then
        neko_tolua_gen("source/tests/test.pkg", "source/tests/test_gen.cpp")
    end

    if ImGui.Button("build_pack") then
        Core.pak_build(("fgd.pack"), {"gamedir/assets/test_1.fgd", "gamedir/assets/primext.fgd"})
    end

    if ImGui.Button("test_xml") then
        local xml_code = game_db.all["test_xml"]
        local xml = common.xml_parser()
        local test_xml = xml:ParseXmlText(xml_code)

        -- local inspect = load_libs_from_url("https://raw.gitmirror.com/kikito/inspect.lua/master/inspect.lua",
        --     "inspect.lua")

        dump_func(test_xml)
    end

    if ImGui.Button("test_http") then
        local http = require 'http'

        local status, data = http.request('https://www.7ed.net/ruozi/api')

        print('welcome')
        print(status)
        print(data)

        local json_data = neko.json_read(common.decode_unicode_escape(data))

        dump_func(json_data)

        print(neko.json_write(json_data))
    end

    if ImGui.Button("test_reg") then
        local reg = Core.from_registry("_PRELOAD")
        dump_func(reg)

        reg = Core.from_registry(LUA_RIDX_GLOBALS, "sandbox")
        dump_func(reg)

        local va = common.va()

        local function bind(f, ...)
            local args = va(...)
            return function(...)
                return f(va.concat(args, va(...)))
            end
        end
        debug_print = bind(print, '[debug]')
        debug_print('hello')
    end

    -- ImGui.Checkbox(neko.conf.cvar.shader_inspect)

    ImGui.Separator()

    if ImGui.Button("thread") then
        local c1 = neko.make_channel 'c1'
        local c2 = neko.make_channel 'c2'

        local t1 = neko.make_thread [[
          local c1 = neko.get_channel 'c1'
          neko.thread_sleep(1)
          c1:send 'one'
        ]]

        local t2 = neko.make_thread [[
          local c2 = neko.get_channel 'c2'
          neko.thread_sleep(2)
          c2:send 'two'
        ]]

        for i = 1, 2 do
            local msg, ch = neko.select(c1, c2)
            print(msg, ch)
        end
    end

    -- if ImGui.InputText("TEST", text) then
    --     print(tostring(text))
    -- end

    -- ImGui.Image(test_tex, 100.0, 100.0)
    -- ImGui.Image(test_custom_sprite.tex, 100.0, 100.0)
    ImGui.End()

end

function test_ecs(dt)

    ecs:update()

    for id, e in ecs:select{"pos", "vel"} do
        e.pos = e.pos + e.vel * vec2(dt, dt)

        if neko.key_down "p" then
            e.vel = nil
        end

        if neko.key_down "k" then
            ecs:kill(id)
        end
    end

    for id, e in ecs:select{"pos", "follow"} do
        local other = ecs:get(e.follow)
        if other ~= nil then
            e.pos = e.pos:lerp(other.pos, dt)
        end
    end

    for id, e in ecs:select{"pos", "rot"} do
        e.rot.angle = e.rot.angle + e.rot.delta * dt
    end

    for id, e in ecs:query{
        select = {"pos", "img", "z_index"},
        where = function(e)
            return e.pos.x < neko.window_width() / 2
        end,
        order_by = function(lhs, rhs)
            return lhs.entity.z_index < rhs.entity.z_index
        end
    } do
        local ox = e.img:width() * 0.5
        local oy = e.img:height() * 0.5
        local angle = e.rot and e.rot.angle or 0
        local scale = e.scale or 1

        e.img:draw(e.pos.x, e.pos.y, angle, scale * 3, scale * 3, ox, oy)

        if e.vel == nil and neko.key_down "v" then
            e.vel = vec2(4, 1)
        end
    end

    neko.default_font():draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))

end

function random_spawn_npc(n)
    n = n or 10
    for i = 1, n do
        local v = {
            id = choose({"Npc", "Chort"}),
            x = random(0, 360),
            y = random(0, 360)
        }
        local mt = _G[v.id]
        if mt ~= nil then
            local obj = LocalGame.world:add(mt(v.x, v.y))
            if v.id == "Player" then
                player = obj
            end
            -- print(v.id, v.x, v.y)
        else
            print("no " .. v.id .. " class exists")
        end
    end
end

function hit_player(demage)
    player.hp = player.hp - demage
end

-- Test

function UnitTest()

    local Test = require("__neko.unittest")

    describe('my project', function()
        lust.before(function()
        end)

        describe('module1', function() -- Can be nested
            it('feature_common_va', function()

                local va = common.va()
                local ip = "127.0.0.2"
                -- expect(inspect({va.map(tonumber, string.match(ip, "^(%d+)%.(%d+)%.(%d+)%.(%d+)$"))})).to.equal(inspect(
                --     {127, 0, 0, 2})) -- Pass

                local function f(...)
                    return ...
                end
                local t = {va.concat(va(f(1, 2, 3)), va(f(4, 5, 6)))}
                -- expect(inspect(t)).to.equal(inspect({1, 2, 3, 4, 5, 6}))

                expect(1).to.be.a('number') -- Pass
            end)

            it('feature_td', function()
                local td_create = common.td_create

                local td = td_create()

                td.ee = td.enum {"XXX", "YYY"}

                td.foo = {
                    x = td.number,
                    y = 1,
                    z = td.enum {"a", "b", "c"},
                    obj = td.object, -- a ref to lua object (table/userdata)
                    a = td.array(td.ee),
                    m = td.map(td.string, td.number),
                    s = td.struct { -- anonymous struct
                        alpha = false,
                        beta = true
                    }
                }

                local foo = td.foo {
                    a = {"XXX", "XXX"},
                    m = {
                        x = 1,
                        y = 2
                    },
                    z = "c",
                    obj = td.foo,
                    s = {
                        alpha = true
                    }
                }

                expect(foo.x).to.equal(0) -- default number is 0
                expect(foo.y).to.equal(1) -- default 1
                expect(foo.z).to.equal("c")
                expect(foo.a[1]).to.equal("XXX")
                expect(foo.m.x).to.equal(1)
                expect(foo.m.y).to.equal(2)
                expect(foo.obj).to.equal(td.foo) -- a type
                expect(foo.s.alpha).to.equal(true)
                expect(foo.s.beta).to.equal(true)

                -- foo.z = "d" -- invalid enum
                -- print(td.foo:verify(foo))
                -- foo.z = nil
                -- print(td.foo:verify(foo))
                -- foo.z = "a"
                -- print(td.foo:verify(foo))
                -- foo.a[1] = nil
                -- print(td.foo:verify(foo))
            end)

            it('feature_bind_enum', function()
                expect(Test.TestAssetKind_1("AssetKind_Tilemap")).to.equal(4)
                expect(Test.TestAssetKind_2(2)).to.equal("AssetKind_Image")
            end)

            it('feature_bind_struct', function()
                local v4 = Core.Vector4.new()
                v4.x = 10
                v4.y = 10
                v4.z = 10
                v4.w = 10
                local v4_c = Test.LUASTRUCT_test_vec4(v4)
                -- expect(inspect({v4_c.x, v4_c.y, v4_c.z, v4_c.w})).to.equal(inspect({20.0, 20.0, 20.0, 20.0}))
            end)

            it('feature_spritepack', function()
                local tools = require("__neko.spritepack")
                local function image(filename)
                    return {
                        filename = filename,
                        tools.imgcrop(filename)
                    }
                end
                local function dump(rect)
                    print(string.format("[%s] %dx%d+%d+%d -> (%d, %d)", rect.filename, rect[1], rect[2], rect[3],
                        rect[4], rect.x, rect.y))
                end
                local rects = {image "gamedir/assets/arrow.png", image "gamedir/assets/bow.png",
                --    image "gamedir/assets/cursor.ase",
                               image "gamedir/assets/player_a.png" -- image "gamedir/assets/tiles.png",
                }
                tools.rectpack(2048, 2048, rects)
                tools.imgpack("gamedir/assets/pack_output.png", 256, 256, rects)
            end)

            -- it('feature_c_struct_bridge', function()
            --     local csb = common.CStructBridge()
            --     local test_custom_sprite = {}
            --     expect(csb.generate_array_type("__lua_quad_vdata_t", "float", 16)).to.be.a('table')
            --     local v = {-0.5, -0.5, 0.0, 0.0, 0.5, -0.5, 1.0, 0.0, -0.5, 0.5, 0.0, 1.0, 0.5, 0.5, 1.0, 1.0}
            --     test_custom_sprite.vdata = luastruct_test.udata(
            --         csb.s["__lua_quad_vdata_t"]:size("struct __lua_quad_vdata_t"))
            --     for i, v in ipairs(v) do
            --         csb.setter("__lua_quad_vdata_t", "v" .. i)(test_custom_sprite.vdata, v)
            --     end
            -- end)

            it('feature_lua_filesys', function()
                local filesys = require("__neko.filesys")
                for name, err in filesys.scandir(".") do
                    -- print(name, err)
                    expect(err).to.equal(nil)
                end
                -- print("exists", filesys.exists("xmake.lua"))
                -- print("getsize", filesys.getsize("xmake.lua"))
                -- print("getmtime", filesys.getmtime("xmake.lua"))
                -- print(os.date("%c", math.floor(filesys.getmtime("xmake.lua"))))
                -- print("getatime", filesys.getatime("xmake.lua"))
                -- print("getctime", filesys.getctime("xmake.lua"))
            end)

            it('feature_test_binding_1', function()
                expect(Test.TestBinding_1()).to.be.truthy()
            end)

            it('feature_nameof', function()
                print("Name of table: ", Core.nameof({}))
                print("Name of string.sub: ", Core.nameof(string.sub))
                print("Name of print: ", Core.nameof(print))

                local Field_foo = 100
                print(Core.nameof(Field_foo))
            end)

            it('feature_ltype', function()
                local coroutine_create, type = coroutine.create, Core.ltype
                local nil_ = nil
                local boolean_ = true
                local number_ = 123
                local string_ = "abc"
                local table_ = {}
                local function_ = function()
                end
                local thread_ = coroutine_create(function()
                end)
                assert(type(nil_) == "nil")
                assert(type(boolean_) == "boolean")
                assert(type(string_) == "string")
                assert(type(table_) == "table")
                assert(type(function_) == "function")
                assert(type(thread_) == "thread")
            end)

            it('feature_luaref', function()
                local ref = Core.ref_init()
                assert(Core.ref_ref(ref) == 2)
                local lst = {1, 2, 3, 4, 5, 1.2345, {}, "ok"}
                local r = {}
                for i, v in ipairs(lst) do
                    r[i] = Core.ref_ref(ref, v)
                end
                for i, v in ipairs(lst) do
                    assert(v == Core.ref_get(ref, r[i]))
                    print(v, Core.ref_get(ref, r[i]))
                end
                r = {}
                for i = 1, 10 do
                    r[i] = Core.ref_ref(ref, i)
                end
                for i = 1, 10 do
                    Core.ref_set(ref, r[i], i * 2)
                end
                for i = 1, 10 do
                    assert(i * 2 == Core.ref_get(ref, r[i]))
                    print(i * 2, Core.ref_get(ref, r[i]))
                end
                Core.ref_close(ref)
            end)

            it('feature_pak', function()
                local test_pack, test_handle, test_items

                local test_pack_buildnum, test_pack_item_count = Core.pak_info("fgd.pack")
                print("pack_info", test_pack_buildnum, test_pack_item_count)
                test_pack = neko.pak_load("test_pack_handle", "fgd.pack")
                test_handle = test_pack:assets_load("gamedir/assets/test_1.fgd")
                test_items = test_pack:items()
                print(type(test_handle))
                dump_func(test_items)
            end)

            it('feature_ecs', function()

                expect(Test.TestEcs()).to.be.truthy()

                local w = Core.ecs_f()

                local csb = w:csb()

                csb.s["haha_t"] = csb.struct([[
                    struct haha_t {
                        int sss;
                        int i2;
                    };
                ]])

                local haha_t_size = csb.size("haha_t")
                print("haha_t_size", haha_t_size)

                local comp1 = w:component("haha_t", haha_t_size)

                assert(w:component_id("haha_t") == comp1)

                local tb, ss = w:get_com()
                print(ss)
                dump_func(tb)

                local sys1 = w:system("haha_system_1", function(ent_ct, name)
                    -- print("update 1", ent_ct, comp1, name)
                    local ptr_haha = w:get(ent_ct, comp1)
                    local v = csb.getter("haha_t", "sss")(ptr_haha)
                    csb.setter("haha_t", "sss")(ptr_haha, v + 1)
                    -- print(ptr_haha, csb.getter("haha_t", "sss")(ptr_haha))
                end, function(ent, name)
                    print("add 1", ent, name)
                end, function(ent, name)
                    print("remove 1", ent, name)
                end)

                print("haha_system_1")
                dump_func(sys1)

                w:system_require_component(sys1, "pos_t", "vel_t", "haha_t")

                for i = 1, 10, 1 do
                    local e = w:create_ent()
                    local ptr_pos, ptr_vel, ptr_haha = w:attach(e, "pos_t", "vel_t", "haha_t")
                    dump_func({ptr_pos, ptr_vel, ptr_haha})
                    csb.setter("haha_t", "sss")(ptr_haha, i * 100)
                end

                -- local e_iter = w:iter(true)
                -- for e in e_iter do
                --     print("ent", e)
                -- end

                local st = os.clock()

                for i = 1, 10000, 1 do
                    local sys1_ret = w:system_run(sys1, 0.0)
                end

                print(os.clock() - st)
            end)

        end)
    end)

end
