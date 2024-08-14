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
                local reflect = hot_require("reflect")
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

            it('feature_parser', function()
                local parser = hot_require("parser")

                parser([[
                    const vv = 3;
                    -- vv = 4;
    
                    let test = 3;
                    test++;
                    test--;
                    test += 5;
                    test /= 4;
                    test *= 3;
                    test ^= 2;
                    test -= 1;
    
                    print(test)
    
                    let test = () => {
                        print("hello, world!");
                    }
                    test();
    
                    let test = (a, b) => a + b;
                    print(test(1, 2));
    
                    -- try {
                    --     nonExistingFunction();
                    -- } catch(e) {
                    --     print("can't call this function!\nerror: " .. e);
                    -- }
                ]])
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
    end)

end

M.UnitTest = UnitTest

return M
