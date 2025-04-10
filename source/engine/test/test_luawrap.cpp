
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "base/scripting/lua_wrapper.hpp"

constexpr static inline const_str table_show_src = R"lua(
function _table_show(t, name, indent)
    local cart -- 一个容器
    local autoref -- 供自我参考
    local function isemptytable(t)
        return next(t) == nil
    end
    local function basicSerialize(o)
        local so = tostring(o)
        if type(o) == "function" then
            local info = debug.getinfo(o, "S")
            -- info.name is nil because o is not a calling level
            if info.what == "C" then
                return string.format("%q", so .. ", C function")
            else
                -- the information is defined through lines
                return string.format("%q", so .. ", defined in (" .. info.linedefined .. "-" .. info.lastlinedefined ..
                    ")" .. info.source)
            end
        elseif type(o) == "number" or type(o) == "boolean" then
            return so
        else
            return string.format("%q", so)
        end
    end

    local function addtocart(value, name, indent, saved, field)
        indent = indent or ""
        saved = saved or {}
        field = field or name

        cart = cart .. indent .. field

        if type(value) ~= "table" then
            cart = cart .. " = " .. basicSerialize(value) .. ";\n"
        else
            if saved[value] then
                cart = cart .. " = {}; -- " .. saved[value] .. " (self reference)\n"
                autoref = autoref .. name .. " = " .. saved[value] .. ";\n"
            else
                saved[value] = name
                -- if tablecount(value) == 0 then
                if isemptytable(value) then
                    cart = cart .. " = {};\n"
                else
                    cart = cart .. " = {\n"
                    for k, v in pairs(value) do
                        k = basicSerialize(k)
                        local fname = string.format("%s[%s]", name, k)
                        field = string.format("[%s]", k)
                        -- three spaces between levels
                        addtocart(v, fname, indent .. "   ", saved, field)
                    end
                    cart = cart .. indent .. "};\n"
                end
            end
        end
    end

    name = name or "__unnamed__"
    if type(t) ~= "table" then
        return name .. " = " .. basicSerialize(t)
    end
    cart, autoref = "", ""
    addtocart(t, name, indent)
    return cart .. autoref
end
function table_show(...)
    local tb = {...}
    print(_table_show(tb))
end
)lua";

using namespace Neko;
using namespace Neko::luabind;

LuaRef getTesting(lua_State *L) {
    lua_getglobal(L, "testing");
    return LuaRef::FromStack(L);
}

int TestBinding_1(lua_State *L) {

    auto run = [](lua_State *L, const char *code) {
        if (luaL_dostring(L, code)) {
            std::cout << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    };

    run(L, "function testing( ... ) print( '> ', ... ) end");

    {
        LuaRef test_func(L, "testing");
        LuaRef table = LuaRef::NewTable(L);
        table["testing"] = test_func;

        table.Push();
        lua_setglobal(L, "a");

        run(L, "print( a.testing )");
        run(L, "a.b = {}");
        run(L, "a.b.c = {}");

        std::cout << "table是表吗? " << (table.IsTable() ? "true" : "false") << std::endl;
        std::cout << "table[\"b\"]是表吗? " << (table["b"].IsTable() ? "true" : "false") << std::endl;

        table["b"]["c"]["hello"] = "World!";
        run(L, "print( a.b.c.hello )");
        auto b = table["b"];  // 返回一个 LuaTableElement
        b[3] = "Index 3";

        LuaRef faster_b = b;  // 将 LuaTableElement 转换为 LuaRef 以加快速度
        for (int i = 1; i < 5; i++) {
            faster_b.Append(i);
        }
        b[1] = LuaNil();  // GC
        b.Append("Add more.");
        run(L, "for k,v in pairs( a.b ) do print( k,v ) end");
        table["b"] = LuaNil();
        run(L, "print( a.b )");

        test_func();
        test_func(1, 2, 3);
        test_func("Hello", "World");
        test_func("你好世界", "World", 1, 2, 3, test_func);

        // 无返回值
        test_func.Call(0, "无返回值");
        table["testing"](test_func, 3, 2, 1, "调用数组元素");
        table["testing"]();
        LuaRef newfuncref(L);
        newfuncref = test_func;
        newfuncref("复制正确");
        newfuncref(getTesting(L));   // 检查移动语义
        newfuncref = getTesting(L);  // 检查移动语义
        run(L, "text = '这已被隐式转换为 std::string'");
        LuaRef luaStr1(L, "text");
        std::string str1 = luaStr1;
        std::cout << str1 << std::endl;
        run(L, "a.text = text");
        std::cout << table["text"].Cast<std::string>() << std::endl;
    }

    return 0;
}

enum TestEnum : int {
    TestEnum_A,
    TestEnum_B,
    TestEnum_C,
};

struct TestStruct {
    float x, y, z, w;
    int x1, x2;

    void print() { std::cout << x << ' ' << y << ' ' << z << ' ' << w << ' ' << x1 << ' ' << x2 << std::endl; }
};

struct TestStruct2 {
    int x1, x2;
    bool x3;

    void print() { std::cout << x1 << ' ' << x2 << ' ' << x3 << std::endl; }
};

struct TestStruct3 {
    TestStruct s1;
    TestStruct2 s2;

    void print() {
        s1.print();
        s2.print();
    }
};

struct TestStruct4 {
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16;
    int x17, x18;

    TestEnum e1;
};

struct TestStruct_NoReg {
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16;
    int x17, x18;

    void print() {
        auto [NEKO_PP_PARAMS(x, 18)] = *this;
        std::tuple tu = {NEKO_PP_PARAMS(x, 18)};
        [&]<size_t... I>(std::index_sequence<I...>) {
            std::cout << "TestStruct_NoReg[";
            (..., (std::cout << std::get<I>(tu)));
            std::cout << "]\n";
        }(std::make_index_sequence<18>{});
    }
};

struct TestStruct_RawArr {
    int arr[9];
};

struct TestStruct_WithOp {
    int x1, x2;

    bool operator==(const TestStruct_WithOp &any) { return x1 == any.x1 && x2 == any.x2; }
};

static int LuaStruct_test_1(lua_State *L) {

    auto v = LuaGet<TestStruct>(L, 1);
    v->x += 10.f;
    v->y += 10.f;
    v->z += 10.f;
    v->w += 10.f;
    LuaPush<TestStruct>(L, *v);
    return 1;
}

static int LuaStruct_test_2(lua_State *L) {

    auto v = LuaGet<TestStruct2>(L, 1);
    v->x1 += 114.f;
    v->x2 += 514.f;
    v->x3 = true;
    LuaPush<TestStruct2>(L, *v);
    return 1;
}

static int LuaStruct_test_3(lua_State *L) {

    auto v = LuaGet<TestStruct3>(L, 1);

    v->s2.x1 = 666.f;
    v->s2.x2 = 233.f;

    LuaPush<TestStruct3>(L, *v);
    return 1;
}

static int LuaStruct_test_4(lua_State *L) {

    auto v = LuaGet<TestStruct4>(L, 1);

    v->x1 += 1001;
    v->x2 += 2002;
    v->x17 += 1777;
    v->x18 += 1888;

    v->e1 = TestEnum_C;

    LuaPush<TestStruct4>(L, *v);
    return 1;
}

static int LuaStruct_test_5(lua_State *L) {

    TestStruct v{};
    v.x = 10086;

    LuaPush<TestStruct>(L, v);
    return 1;
}

static int LuaStruct_test_rawarr(lua_State *L) {

    auto v = LuaGet<TestStruct_RawArr>(L, 1);

    v->arr[0] = 114514;
    v->arr[5] = 666;

    LuaPush<TestStruct_RawArr>(L, *v);
    return 1;
}

struct TestStruct_FAST {
    float a;
    double b;
    int x;
    int y;
    int z;
};

static lua_Number getfield(lua_State *L, int index, const char *key) {
    lua_getfield(L, index, key);
    lua_Number v = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return v;
}

template <typename T, std::size_t... Is>
static void TestReflection_field_name(T &v, std::index_sequence<Is...>) {
    (..., [&] {
        constexpr auto field_name = reflection::field_name<T, Is>;
        using field_type = typename reflection::field_type<T, Is>;
        auto &af = reflection::field_access<Is>(v);
        std::cout << "field_name " << field_name.data() << '\n';
    }());
}

int Test_LuaWrap() {

    // std::cout << neko::reflection::field_count<TestStruct_RawArr> << std::endl;
    // std::cout << memberCount<TestStruct3>() << std::endl;
    // std::cout << neko::reflection::counting_fields::num_aggregate_unique_fields_v<TestStruct3> << std::endl;

    constexpr std::size_t field_count = reflection::field_count<TestStruct_FAST>;
    TestReflection_field_name(TestStruct_FAST{}, std::make_index_sequence<field_count>{});

    LuaVM vm;

    vm.Create(true);

    lua_State *L = vm;

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = lua_tostring(L, -1);
                printf("nekolua_panic error: %s", msg);
                return 0;
            });

    LuaEnum<TestEnum>(L);

    lua_newtable(L);
    LuaStruct<TestStruct>(L, "TestStruct");
    LuaStruct<TestStruct2>(L, "TestStruct2");
    LuaStruct<TestStruct3>(L, "TestStruct3");
    LuaStruct<TestStruct4>(L, "TestStruct4");
    LuaStruct<TestStruct_RawArr>(L, "TestStruct_RawArr");
    LuaStruct<TestStruct_WithOp>(L, "TestStruct_WithOp");
    lua_setglobal(L, "LuaStruct");

    luaL_Reg lib[] = {{"LuaStruct_test_1", Wrap<LuaStruct_test_1>},
                      {"LuaStruct_test_2", Wrap<LuaStruct_test_2>},
                      {"LuaStruct_test_3", Wrap<LuaStruct_test_3>},
                      {"LuaStruct_test_4", Wrap<LuaStruct_test_4>},
                      {"LuaStruct_test_5", Wrap<LuaStruct_test_5>},
                      {"LuaStruct_test_rawarr", Wrap<LuaStruct_test_rawarr>},
                      {"TestAssetKind_1",
                       +[](lua_State *L) {
                           auto type_val = LuaGet<TestEnum>(L, 1);
                           lua_pushinteger(L, type_val);
                           return 1;
                       }},

                      {"TestAssetKind_2",
                       +[](lua_State *L) {
                           TestEnum type_val = (TestEnum)lua_tointeger(L, 1);
                           LuaPush<TestEnum>(L, type_val);
                           return 1;
                       }},
                      {"TestAssetKind_3", +[](lua_State *L) {
                           auto type_val = LuaGet<TestEnum>(L, 1);
                           std::cout << "TestAssetKind_3 " << type_val << std::endl;
                           return 0;
                       }}};

    for (auto f : lib) {
        lua_register(L, f.name, f.func);
    }

    vm.RunString(table_show_src);

    {
        vm.RunString(R"lua(
        function hello()
            print("hello?")
        end
        )lua");
        vm("hello");
    }

    {
        LuaPush(
                L, +[](lua_State *L) -> int {
                    std::cout << "any lambda?" << std::endl;
                    return 0;
                });
        lua_setglobal(L, "TestLambda_1");
        vm.RunString(R"lua(
        TestLambda_1()
        )lua");
    }

    {
        int *ptr = new int(114514);
        LuaPush(L, ptr);
        int *ptr_ = LuaGet<int *>(L, -1);
        std::cout << "LuaPush/Get int* : " << *ptr << std::endl;
        lua_pop(L, 1);
        delete ptr_;
    }

    //    vm.RunString(R"lua(
    // TestStruct_FAST_1 = {
    //    a = 114514,
    //    b = 3.1415926
    //}
    //        )lua");
    //
    //    TestStruct_FAST testStruct_fast;
    //    struct_foreach_luatable(L, "TestStruct_FAST_1", testStruct_fast);
    //    std::cout << testStruct_fast.a << " " << testStruct_fast.b << '\n';

    //{
    //    vm.RunString(R"lua(
    //    function test_invoke(a, b)
    //        print("test_invoke", a, b)
    //        return 3.14
    //    end
    //    )lua");
    //    f32 ret = InvokeLua<f32>(L, "test_invoke", "a_str", 114514);
    //    std::cout << ret << '\n';
    //}

    {
        TestStruct_NoReg s1 = {1, 2, 3, 4, 5, 6, 7, 8, 2, 2, 3, 4, 5, 6, 7, 8};
        LuaPushRaw<TestStruct_NoReg>(L, s1);
        auto TestVec4_ = LuaGetRaw<TestStruct_NoReg>(L, -1);
        TestVec4_.print();
        lua_pop(L, 1);
    }

    std::cout << Neko::reflection::GetTypeName<TestStruct4>() << '\n';

    std::cout << LuaEnumHas<TestEnum>(L, "TestEnum_D") << '\n';
    std::cout << LuaEnumHas<TestEnum>(L, "TestEnum_C") << '\n';
    std::cout << LuaEnumHas<TestEnum>(L, 3) << '\n';
    std::cout << LuaEnumHas<TestEnum>(L, 2) << '\n';

    std::cout << "======= Test LuaSide =======" << std::endl;

    vm.RunString(R"(
        print("Hello NekoLua!")
        print(TestAssetKind_1("TestEnum_A"))
        print(TestAssetKind_2(2))
        TestAssetKind_3("TestEnum_B")

        test_struct = LuaStruct.TestStruct.new()
        table_show(test_struct.x,test_struct.y,test_struct.z,test_struct.w)
        test_struct = LuaStruct_test_1(test_struct)
        table_show(test_struct.x,test_struct.y,test_struct.z,test_struct.w)

        test_struct2 = LuaStruct.TestStruct2.new()
        table_show(test_struct2.x1,test_struct2.x2,test_struct2.x3)
        test_struct = LuaStruct_test_2(test_struct2)
        table_show(test_struct2.x1,test_struct2.x2,test_struct2.x3)

        test_struct3 = LuaStruct.TestStruct3.new()
        table_show(test_struct3.s1,test_struct3.s2)

        test_struct3_s2 = test_struct3.s2
        table_show(test_struct3_s2.x1,test_struct3_s2.x2)

        test_struct = LuaStruct_test_3(test_struct3)
        table_show(test_struct3_s2.x1,test_struct3_s2.x2)

        test_struct4 = LuaStruct.TestStruct4.new()
        table_show(test_struct4.x1,test_struct4.x2,test_struct4.x3,test_struct4.x4,test_struct4.x17,test_struct4.x18,test_struct4.e1)
        test_struct4 = LuaStruct_test_4(test_struct4)
        table_show(test_struct4.x1,test_struct4.x2,test_struct4.x3,test_struct4.x4,test_struct4.x17,test_struct4.x18,test_struct4.e1)

        test_struct5 = LuaStruct_test_5()
        table_show(test_struct5.x,test_struct5.y,test_struct5.z,test_struct5.w)

        test_struct_rawarr = LuaStruct.TestStruct_RawArr.new()
        table_show(test_struct_rawarr.arr)
        test_struct_rawarr = LuaStruct_test_rawarr(test_struct_rawarr)
        table_show(test_struct_rawarr.arr)

        LuaStruct.TestStruct_WithOp.metatype({
            __eq = function(a, b)
                return a.x1 == b.x1 and a.x2 == b.x2
            end,
            __index = function(m)
            end,
            __newindex = function(m)
            end,
            __gc = function(m)
            end
        })

        test_struct_withop1 = LuaStruct.TestStruct_WithOp.new()
        test_struct_withop2 = LuaStruct.TestStruct_WithOp.new()
        print(test_struct_withop1==test_struct_withop2)
        test_struct_withop1.x1 = 2
        print(test_struct_withop1==test_struct_withop2)

        print(nameof(LuaStruct.TestStruct))
    )");

    // TestBinding_1(L);

    // luaL_dostring(L, "t = {x = 10, y = 'hello', z = {a = 1, b = 2}}");
    // lua_getglobal(L, "t");

    std::cout << "======= Test Typeinfo =======" << std::endl;

    auto PrintTypeinfo = [](LuaTypeinfo ts) {
        auto [a, b] = ts;
        std::cout << ts.name << ' ' << ts.size << '\n';
    };

    LuaTypeid f1 = TypeFind(L, "TestEnum");
    PrintTypeinfo(GetLuaTypeinfo(L, f1));
    PrintTypeinfo(GetLuaTypeinfo(L, "TestEnum"));

    // TestStruct3 TestStruct3 = {114514.f, 2.f, 3.f, 4.f, 199, 233};
    // LuaPush(L, TestStruct3);

    std::cout << "======= Test FastStruct =======" << std::endl;

    {
        LuaTableAccess<TestStruct_FAST>::Init(L);
        auto &meta = LuaTableAccess<TestStruct_FAST>::GetMeta();
        for (auto &field : meta.field_offsets) {
            std::cout << "field_offsets " << field.first << " " << field.second << '\n';
        }
        lua_newtable(L);
        Table *ttt = LuaTableAccess<TestStruct_FAST>::GetTable(L, -1);

        LuaTableAccess<TestStruct_FAST>::Fields<lua_Number>(ttt, "a") = 114514;
        LuaTableAccess<TestStruct_FAST>::Fields<lua_Number>(ttt, "b") = 3.1415926;

        printf("a=%f,%f\n", LuaTableAccess<TestStruct_FAST>::Fields<lua_Number>(ttt, "a"), getfield(L, -1, "a"));
        printf("b=%f,%f\n", LuaTableAccess<TestStruct_FAST>::Fields<lua_Number>(ttt, "b"), getfield(L, -1, "b"));

        lua_pop(L, 1);
    }

    std::cout << "======= END =======" << std::endl;

    vm.Fini(L);

    return 0;
}
