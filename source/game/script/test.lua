local M = {}

local lust = hot_require("libs/lust")
local describe, it, expect = lust.describe, lust.it, lust.expect

local function UnitTest()

    local Test = require("__neko.unittest")

    describe('my project', function()
        lust.before(function()
        end)

        describe('module_nbt', function()

            local nbt = hot_require("libs/nbt")
            local result

            it("nbt_load", function()
                local f = assert(io.open("test.nbt", "rb"))
                local str = f:read("*a")
                f:close()
                result = nbt.decode(str)
                mm(result)
            end)

            it("nbt_tag_compound_id", function()
                expect(nbt.TAG_COMPOUND).to.equal(result:getTypeID())
                expect("hello world").to.equal(result:getName())
            end)

            it("nbt_entry_name", function()
                local value = result:getValue()
                expect(value.name).to.be.truthy()
                expect(value.name:getString()).to.equal("Bananrama")
            end)

        end)

        describe('module1', function()

            it('feature_ffi_reflect_enum', function()
                local reflect = ffi_reflect()
                e = reflect.enum [[
                    A, B, /* 注释正确解析 */
                    C = 7,  // 注释正确解析
                    D,
                    E = 0xc
                ]]
                assert(e.A == 0 and e.B == 1 and e.C == 7 and e.D == 8 and e.E == 12)

                d = reflect.enum_define [[
                    #define foo 0 
                    #define bar 1
                    #define baz 5
                    #define qux 6
                ]]
                assert(d.foo == 0 and d.bar == 1 and d.baz == 5 and d.qux == 6)
            end)

            it('feature_table_gc', function()
                -- local setmetatable = hot_require("gc_metatable")
                -- local r = {}
                -- local tm = {}
                -- local count = 0
                -- tm.__gc = function(t)
                --     count = count + 1
                --     if exiting then
                --         assert(count == 3)
                --         print "looking good"
                --     end
                --     r = t -- 复活
                --     setmetatable(t, tm) -- 并告诉gc它复活了
                -- end
                -- setmetatable(r, tm)
                -- r = nil
                -- collectgarbage("collect")
                -- r = nil
                -- collectgarbage("collect")
                -- assert(count == 2)
            end)

            it('nn', function()

                local nn = hot_require("nn"):new_network({2, 3, 1})
                for _ = 1, 1000 do
                    nn:learn({0, 0}, {0})
                    nn:learn({0, 1}, {1})
                    nn:learn({1, 1}, {0})
                    nn:learn({1, 0}, {1})
                end

                -- Test
                print("Output for {1,0}: " .. nn:forward({1, 0})[1])

                -- if 0 then
                --     local nn = hot_require("nn"):new_network({2, 3, 1}, 0.1, 0.03)
                --     -- 进行 20 *10,000 次迭代 每 10,000 次打印平均误差
                --     for _ = 1, 20 do
                --         local err = 0
                --         for _ = 1, 10000 do
                --             err = err + nn:learn({0, 0}, {0})
                --             err = err + nn:learn({0, 1}, {1})
                --             err = err + nn:learn({1, 1}, {0})
                --             err = err + nn:learn({1, 0}, {1})
                --         end
                --         err = err / (20 * 1000)
                --         print(string.format("Average error: %0.5f", err))
                --     end
                --     print(string.format(
                --         "Results of XOR Test!\n{0,0} = %.1f	Expected ~(0.0)\n{0,1} = %.1f	Expected ~(1.0)\n{1,0} = %.1f	Expected ~(1.0)\n{1,1} = %.1f	Expected ~(0.0)",
                --         nn:forward({0, 0})[1], nn:forward({0, 1})[1], nn:forward({1, 0})[1], nn:forward({1, 1})[1]))
                -- end

                local my_network = hot_require("nn"):new_network({2, 3, 1})
                -- 在这里训练学习
                -- table.save(my_network.synapses, "network.txt")

                -- -- 加载网络确保结构相同
                -- local my_network = hot_require("nn"):new_network({2, 3, 1})
                -- my_network.synapses = table.load("network.txt")

            end)

            it('feature_xml_parser', function()
                Test.test_xml()
            end)

        end)

        describe('module2', function() -- Can be nested
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
                local v4 = Core.vec4.new()
                v4.x = 10
                v4.y = 10
                v4.z = 10
                v4.w = 10
                local v4_c = Test.LUASTRUCT_test_vec4(v4)
                -- expect(inspect({v4_c.x, v4_c.y, v4_c.z, v4_c.w})).to.equal(inspect({20.0, 20.0, 20.0, 20.0}))
            end)

            -- it('feature_spritepack', function()
            --     local tools = require("__neko.spritepack")
            --     local function image(filename)
            --         return {
            --             filename = filename,
            --             tools.imgcrop(filename)
            --         }
            --     end
            --     local function dump(rect)
            --         print(string.format("[%s] %dx%d+%d+%d -> (%d, %d)", rect.filename, rect[1], rect[2], rect[3],
            --             rect[4], rect.x, rect.y))
            --     end
            --     local rects = {image "gamedir/assets/arrow.png", image "gamedir/assets/bow.png",
            --     --    image "gamedir/assets/cursor.ase",
            --                    image "gamedir/assets/player_a.png" -- image "gamedir/assets/tiles.png",
            --     }
            --     tools.rectpack(2048, 2048, rects)
            --     tools.imgpack("gamedir/assets/pack_output.png", 256, 256, rects)
            -- end)

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

            -- it('feature_pak', function()
            --     local test_pack, test_handle, test_items

            --     local test_pack_buildnum, test_pack_item_count = Core.pak_info("fgd.pack")
            --     print("pack_info", test_pack_buildnum, test_pack_item_count)
            --     test_pack = neko.pak_load("test_pack_handle", "fgd.pack")
            --     test_handle = test_pack:assets_load("gamedir/assets/test_1.fgd")
            --     test_items = test_pack:items()
            --     print(type(test_handle))
            --     dump_func(test_items)
            -- end)

            -- it('feature_ecs', function()

            --     expect(Test.TestEcs()).to.be.truthy()

            --     local w = Core.ecs_f()

            --     local csb = w:csb()

            --     csb.s["haha_t"] = csb.struct([[
            --         struct haha_t {
            --             int sss;
            --             int i2;
            --         };
            --     ]])

            --     local haha_t_size = csb.size("haha_t")
            --     print("haha_t_size", haha_t_size)

            --     local comp1 = w:component("haha_t", haha_t_size)

            --     assert(w:component_id("haha_t") == comp1)

            --     local tb, ss = w:get_com()
            --     print(ss)
            --     dump_func(tb)

            --     local sys1 = w:system("haha_system_1", function(ent_ct, name)
            --         -- print("update 1", ent_ct, comp1, name)
            --         local ptr_haha = w:get(ent_ct, comp1)
            --         local v = csb.getter("haha_t", "sss")(ptr_haha)
            --         csb.setter("haha_t", "sss")(ptr_haha, v + 1)
            --         -- print(ptr_haha, csb.getter("haha_t", "sss")(ptr_haha))
            --     end, function(ent, name)
            --         print("add 1", ent, name)
            --     end, function(ent, name)
            --         print("remove 1", ent, name)
            --     end)

            --     print("haha_system_1")
            --     dump_func(sys1)

            --     w:system_require_component(sys1, "pos_t", "vel_t", "haha_t")

            --     for i = 1, 10, 1 do
            --         local e = w:create_ent()
            --         local ptr_pos, ptr_vel, ptr_haha = w:attach(e, "pos_t", "vel_t", "haha_t")
            --         dump_func({ptr_pos, ptr_vel, ptr_haha})
            --         csb.setter("haha_t", "sss")(ptr_haha, i * 100)
            --     end

            --     -- local e_iter = w:iter(true)
            --     -- for e in e_iter do
            --     --     print("ent", e)
            --     -- end

            --     local st = os.clock()

            --     for i = 1, 10000, 1 do
            --         local sys1_ret = w:system_run(sys1, 0.0)
            --     end

            --     print(os.clock() - st)
            -- end)

        end)

    end)

end

M.UnitTest = UnitTest

return M
