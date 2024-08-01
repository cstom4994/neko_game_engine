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

        local inspect = load_libs_from_url("https://raw.gitmirror.com/kikito/inspect.lua/master/inspect.lua",
            "inspect.lua")

        print(inspect(test_xml))
    end

    if ImGui.Button("test_http") then
        local http = require 'http'

        local status, data = http.request('https://www.7ed.net/ruozi/api')

        print('welcome')
        print(status)
        print(data)

        local json_data = neko.json_read(common.decode_unicode_escape(data))

        print(inspect(json_data))

        print(neko.json_write(json_data))
    end

    if ImGui.Button("test_reg") then
        local reg = Core.from_registry("_PRELOAD")
        print(inspect(reg))

        reg = Core.from_registry(LUA_RIDX_GLOBALS, "sandbox")
        print(inspect(reg))

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
                expect(inspect({va.map(tonumber, string.match(ip, "^(%d+)%.(%d+)%.(%d+)%.(%d+)$"))})).to.equal(inspect(
                    {127, 0, 0, 2})) -- Pass

                local function f(...)
                    return ...
                end
                local t = {va.concat(va(f(1, 2, 3)), va(f(4, 5, 6)))}
                expect(inspect(t)).to.equal(inspect({1, 2, 3, 4, 5, 6}))

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
                expect(inspect({v4_c.x, v4_c.y, v4_c.z, v4_c.w})).to.equal(inspect({20.0, 20.0, 20.0, 20.0}))
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
                print(inspect(test_items), type(test_handle))
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
                print(ss, inspect(tb))

                local sys1 = w:system("haha_system_1", function(ent_ct, name)
                    print("update 1", ent_ct, comp1, name)
                    local ptr_haha = w:get(ent_ct, comp1)
                    print(ptr_haha, csb.getter("haha_t", "sss")(ptr_haha))
                end, function(ent, name)
                    print("add 1", ent, name)
                end, function(ent, name)
                    print("remove 1", ent, name)
                end)

                print("haha_system_1", inspect(sys1))

                w:system_require_component(sys1, "pos_t", "vel_t", "haha_t")

                for i = 1, 10, 1 do
                    local e = w:create_ent()
                    local ptr_pos, ptr_vel, ptr_haha = w:attach(e, "pos_t", "vel_t", "haha_t")
                    print(inspect({ptr_pos, ptr_vel, ptr_haha}))
                    csb.setter("haha_t", "sss")(ptr_haha, i * 100)
                end

                -- local e_iter = w:iter(true)
                -- for e in e_iter do
                --     print("ent", e)
                -- end

                local sys1_ret = w:system_run(sys1, 0.0)

                print("sys1_ret", inspect(sys1_ret))

            end)

        end)
    end)

end
