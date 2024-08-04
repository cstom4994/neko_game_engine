lust = require("libs/lust")
local describe, it, expect = lust.describe, lust.it, lust.expect

function UnitTest()

    -- local Test = require("__neko.unittest")

    describe('my project', function()
        lust.before(function()
        end)

        describe('module1', function()

            it('feature_ffi_reflect_enum', function()
                local reflect = require("reflect")
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
                local setmetatable = require("gc_metatable")
                r = {}
                tm = {}
                count = 0
                tm.__gc = function(t)
                    count = count + 1
                    if exiting then
                        assert(count == 3)
                        print "looking good"
                    end
                    r = t -- resurrect
                    setmetatable(t, tm) -- and tell gc it resurrected
                end
                setmetatable(r, tm)
                r = nil
                collectgarbage("collect")
                r = nil
                collectgarbage("collect")
                assert(count == 2)
                exiting = 1
            end)

            it('feature_ffi_luastate', function()

            end)

        end)
    end)

end
