local M = {}

local lust = hot_require("libs/lust")
local describe, it, expect = lust.describe, lust.it, lust.expect

local function UnitTest()
    local Test = require("__neko.unittest")

    print("===============================UnitTest========================================")

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
                print(result)
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
        end)

        describe('module2', function() -- Can be nested
            it('feature_common_va', function()
                local va = common.va()

                local function f(...)
                    return ...
                end
                local t = {va.concat(va(f(1, 2, 3)), va(f(4, 5, 6)))}
                expect(table.show(t[1])).to.equal(table.show({1, 2, 3, 4, 5, 6}))

                local function bind(f, ...)
                    local args = va(...)
                    return function(...)
                        return f(table.unpack(va.concat(args, va(...))))
                    end
                end

                local debug_print = bind(print, '[debug]')
                debug_print("This is a test") -- 输出: [debug] This is a test

                expect(1).to.be.a('number') -- Pass
            end)

            -- it('feature_bind_enum', function()
            --     expect(Test.TestAssetKind_1("AssetKind_Tiledmap")).to.equal(4)
            --     expect(Test.TestAssetKind_2(2)).to.equal("AssetKind_Image")
            -- end)

            -- it('feature_bind_struct', function()
            --     local v4 = neko.vec4.new()
            --     v4.x = 10
            --     v4.y = 10
            --     v4.z = 10
            --     v4.w = 10
            --     local v4_c = Test.LUASTRUCT_test_vec4(v4)
            --     expect(table.show({v4_c.x, v4_c.y, v4_c.z, v4_c.w})).to.equal(table.show({20.0, 20.0, 20.0, 20.0}))
            -- end)

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

            -- it('feature_test_binding_1', function()
            --     expect(Test.TestBinding_1()).to.be.truthy()
            -- end)

            it('feature_nameof', function()
                print("Name of table: ", nameof({}))
                print("Name of string.sub: ", nameof(string.sub))
                print("Name of print: ", nameof(print))

                local Field_foo = 100
                print(nameof(Field_foo))
            end)

            it('feature_ltype', function()
                local coroutine_create, type = coroutine.create, neko.ltype
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

            -- it('feature_pak', function()
            --     local test_pack, test_handle, test_items

            --     local test_pack_buildnum, test_pack_item_count = neko.pak_info("fgd.pack")
            --     print("pack_info", test_pack_buildnum, test_pack_item_count)
            --     test_pack = neko.pak_load("test_pack_handle", "fgd.pack")
            --     test_handle = test_pack:assets_load("gamedir/assets/test_1.fgd")
            --     test_items = test_pack:items()
            --     print(type(test_handle))
            --     dump_func(test_items)
            -- end)

            -- it('feature_ecs', function()

            --     expect(Test.TestEcs()).to.be.truthy()

            --     local w = neko.ecs_f()

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

            it('feature_ecs_t1', function()
                local w = EcsWorld

                w:register("vector2", {
                    x = 0,
                    y = 0
                })

                w:register("vector3", {
                    x = 0,
                    y = 0,
                    z = 0
                })
                w:register("vector4", {
                    x = 0,
                    y = 0,
                    z = 0,
                    w = 0
                })

                local eid1 = w:new{
                    vector2 = {
                        x = 101,
                        y = 102
                    },
                    vector3 = {
                        x = 103,
                        y = 104,
                        z = 105
                    }
                }

                local eid2 = w:new{
                    vector3 = {
                        x = 201,
                        y = 202,
                        z = 203
                    },
                    vector4 = {
                        x = 204,
                        y = 205,
                        z = 206,
                        w = 207
                    }
                }

                local eid3 = w:new{
                    vector2 = {
                        x = 301,
                        y = 302
                    },
                    vector4 = {
                        x = 303,
                        y = 304,
                        z = 305,
                        w = 306
                    }
                }

                print("==============test get")
                local vector2, vector3, vector4 = w:get(eid1, "vector2", "vector3", "vector4")
                assert(vector2.x == 101 and vector3.x == 103 and (not vector4))
                print("===============test match")
                print("all vector4")
                -- for v4 in w:match("all", "vector4") do
                --     print(v4.x)
                -- end
                -- print("all vector4 and vector3")
                -- for v4 in w:match("all", "vector4", "vector3") do
                --     print(v4.x)
                -- end
                -- print("all dirty vector4")
                -- for v4 in w:match("dirty", "vector4") do
                --     print(v4.x)
                -- end
                -- for v4 in w:match("all", "vector4") do
                --     print("touch", v4.x)
                --     w:touch(v4)
                -- end
                -- print("all dirty vector4 and vector3")
                -- for v4 in w:match("dirty", "vector4", "vector3") do
                --     print(v4.x)
                -- end
                -- print("all dirty vector4 and vector3 and vector2")
                -- for v4 in w:match("dirty", "vector4", "vector3", "vector2") do
                --     print(v4.x)
                -- end
                -- print("add vector2 for vector4 and vector3")
                -- for v4 in w:match("dirty", "vector4", "vector3") do
                --     w:add(v4.__eid, "vector2", {
                --         x = 0,
                --         y = 0
                --     })
                --     print(v4.x)
                -- end
                -- print("remove vector2 for vector4 and vector3")
                -- for v4 in w:match("dirty", "vector4", "vector3", "vector2") do
                --     w:remove(v4.__eid, "vector2")
                --     print(v4.x)
                -- end
                -- print("dirty vector4 vector3 vector2")
                -- for v4 in w:match("dirty", "vector4", "vector3", "vector2") do
                --     print(v4.x)
                -- end

                w:del(eid2)

                w:update()

                print("=========")
                w:dump()
                print("=========")

                print("dead")
                -- for v4 in w:match("dead", "vector4") do
                --     print("dead", v4.x)
                -- end

                -- print("all vector4")
                -- for v4 in w:match("all", "vector4") do
                --     print(v4.x)
                -- end
            end)

            it("feature_cparser_1", function()

                local CDATA = neko.cdata
                local Test = {}

                local obj = CDATA.struct([[
                    struct TestStructA {
                        int8_t id8;
                        int16_t id16;
                        int32_t id32;
                        int64_t id64;
                        uint8_t idu8;
                        uint16_t idu16;
                        uint32_t idu32;
                        uint64_t idu64;
                        float x;
                        double y;
                        bool active;
                    };
                    ]])

                -- getter 和 setter 定义
                Test.get_id8 = obj:getter "struct TestStructA.id8"
                Test.set_id8 = obj:setter "struct TestStructA.id8"

                Test.get_id16 = obj:getter "struct TestStructA.id16"
                Test.set_id16 = obj:setter "struct TestStructA.id16"

                Test.get_id32 = obj:getter "struct TestStructA.id32"
                Test.set_id32 = obj:setter "struct TestStructA.id32"

                Test.get_id64 = obj:getter "struct TestStructA.id64"
                Test.set_id64 = obj:setter "struct TestStructA.id64"

                Test.get_idu8 = obj:getter "struct TestStructA.idu8"
                Test.set_idu8 = obj:setter "struct TestStructA.idu8"

                Test.get_idu16 = obj:getter "struct TestStructA.idu16"
                Test.set_idu16 = obj:setter "struct TestStructA.idu16"

                Test.get_idu32 = obj:getter "struct TestStructA.idu32"
                Test.set_idu32 = obj:setter "struct TestStructA.idu32"

                Test.get_idu64 = obj:getter "struct TestStructA.idu64"
                Test.set_idu64 = obj:setter "struct TestStructA.idu64"

                Test.get_x = obj:getter "struct TestStructA.x"
                Test.set_x = obj:setter "struct TestStructA.x"

                Test.get_y = obj:getter "struct TestStructA.y"
                Test.set_y = obj:setter "struct TestStructA.y"

                Test.get_active = obj:getter "struct TestStructA.active"
                Test.set_active = obj:setter "struct TestStructA.active"

                print(obj:size "struct TestStructA")

                local obj = CDATA.__CORE.udata(obj:size "struct TestStructA")

                -- 设置和测试
                Test.set_id8(obj, 42)
                expect(Test.get_id8(obj)).to.equal(42)

                Test.set_id16(obj, 1337)
                expect(Test.get_id16(obj)).to.equal(1337)

                Test.set_id32(obj, 123456789)
                expect(Test.get_id32(obj)).to.equal(123456789)

                Test.set_id64(obj, 123456789012345)
                expect(Test.get_id64(obj)).to.equal(123456789012345)

                Test.set_idu8(obj, 250)
                expect(Test.get_idu8(obj)).to.equal(250)

                Test.set_idu16(obj, 65535)
                expect(Test.get_idu16(obj)).to.equal(65535)

                Test.set_idu32(obj, 4294967295)
                expect(Test.get_idu32(obj)).to.equal(4294967295)

                Test.set_idu64(obj, 9007199254740991)
                expect(Test.get_idu64(obj)).to.equal(9007199254740991)

                Test.set_x(obj, 3.14159265)
                expect(math.abs(Test.get_x(obj) - 3.14159265) < 0.0001).to.be.truthy()

                Test.set_y(obj, 2.718281828459045)
                expect(math.abs(Test.get_y(obj) - 2.718281828459045) < 0.0000001).to.be.truthy()

                Test.set_active(obj, true)
                expect(Test.get_active(obj)).to.equal(true)
            end)

            it("feature_tableptr", function()

                local clone = {1, 2, 3, 4}
                local a = {
                    true,
                    false,
                    114514,
                    3.1415,
                    a = clone,
                    b = "hello",
                    c = {
                        a = 1,
                        b = clone
                    }
                }
                local p = neko.tbptr_to(a)
                neko.tbptr_create(p)
                for _, k, v in neko.tbptr_pairs(p) do
                    print(k, v)
                    if type(v) == "userdata" then
                        for _, k, v in neko.tbptr_pairs(v) do
                            print("\t", k, v)
                        end
                    end
                end
            end)
        end)
    end)

    print("===============================UnitTest========================================")
end

M.UnitTest = UnitTest

return M
