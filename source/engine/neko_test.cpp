
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

// engine
#include "engine/neko.hpp"
#include "engine/neko_api.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_base.h"
#include "engine/neko_ecs.h"
#include "engine/neko_lua.h"
#include "engine/neko_lua_struct.h"
#include "engine/neko_lua_table.hpp"
#include "engine/neko_luabind.hpp"
#include "engine/neko_prelude.h"
#include "engine/neko_reflection.hpp"
#include "engine/neko_seri.h"

#pragma region test

namespace neko::lua::__unittest {

void run(lua_State* L, const char* code) {
    std::cout << "code: " << code << std::endl;
    if (luaL_dostring(L, code)) {
        std::cout << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
}

LuaRef getTesting(lua_State* L) {
    lua_getglobal(L, "testing");
    return LuaRef::fromStack(L);
}

void printString(const std::string& str) { std::cout << str << std::endl; }

int TestBinding_1(lua_State* L) {

    run(L, "function testing( ... ) print( '> ', ... ) end");

    {
        LuaRef testing(L, "testing");
        LuaRef table = LuaRef::newTable(L);
        table["testing"] = testing;

        table.push();
        lua_setglobal(L, "a");

        run(L, "print( a.testing )");
        run(L, "a.b = {}");
        run(L, "a.b.c = {}");

        std::cout << "Is table a table? " << (table.isTable() ? "true" : "false") << std::endl;
        std::cout << "Is table[\"b\"] a table? " << (table["b"].isTable() ? "true" : "false") << std::endl;

        table["b"]["c"]["hello"] = "World!";

        run(L, "print( a.b.c.hello )");

        auto b = table["b"];  // returns a LuaTableElement
        b[3] = "Index 3";

        LuaRef faster_b = b;  // Convert LuaTableElement to LuaRef for faster pushing

        for (int i = 1; i < 5; i++) {
            faster_b.append(i);
        }
        b[1] = LuaNil();
        b.append("Add more.");

        run(L, "for k,v in pairs( a.b ) do print( k,v ) end");

        table["b"] = LuaNil();

        run(L, "print( a.b )");

        testing();
        testing(1, 2, 3);
        testing("Hello", "World");

        testing("Hello", "World", 1, 2, 3, testing);

        testing("Nigel", "Alara", "Aldora", "Ayna", "Sarah", "Gavin", "Joe", "Linda", "Tom", "Sonja", "Greg", "Trish");

        // No return value
        testing.call(0, "No return value.");

        table["testing"](testing, 3, 2, 1, "Calling array element");
        table["testing"]();

        LuaRef newfuncref(L);

        newfuncref = testing;

        newfuncref("Did it copy correctly?");

        newfuncref(getTesting(L));  // Check move semantics

        newfuncref = getTesting(L);  // Check move semantics

        run(L, "text = 'This has been implicitly cast to std::string'");

        LuaRef luaStr1(L, "text");

        std::string str1 = luaStr1;

        printString(str1);

        run(L, "a.text = text");

        printString(table["text"]);
    }

    lua_pushboolean(L, 1);

    return 1;
}

// Steps to create an ECS:
// 1. Create an ECS instance (using ecs_new)
// 2. Define concrete components types (structs)
// 3. Assign each a unique, zero-based ID (using an enum is recommended)
// 4. Write system update callbacks
// 5. Define zero-based system IDs (using an enum is recommended)
// 6. Register components
// 7. Register systems
// 8. Associate components with systems (using ecs_require_component)

// Concrete component structs
typedef struct {
    float x, y;
} pos_t;

typedef struct {
    float vx, vy;
} vel_t;

typedef struct {
    int x, y, w, h;
} rect_t;

// Corresponding component IDs
ecs_id_t PosComp;
ecs_id_t VelComp;
ecs_id_t RectComp;

// System IDs
ecs_id_t System1;
ecs_id_t System2;
ecs_id_t System3;

// Register components
void register_components(ecs_t* ecs) {
    PosComp = ecs_register_component(ecs, sizeof(pos_t), NULL, NULL);
    VelComp = ecs_register_component(ecs, sizeof(vel_t), NULL, NULL);
    RectComp = ecs_register_component(ecs, sizeof(rect_t), NULL, NULL);
}

// System that prints the entity IDs of entities associated with this system
ecs_ret_t system_update(ecs_t* ecs, ecs_id_t* entities, int entity_count, ecs_dt_t dt, void* udata) {
    (void)ecs;
    (void)dt;
    (void)udata;

    for (int id = 0; id < entity_count; id++) {
        printf("%u ", entities[id]);
    }

    printf("\n");

    return 0;
}

// Register all systems and required relationships
void register_systems(ecs_t* ecs) {
    // Register systems
    System1 = ecs_register_system(ecs, system_update, NULL, NULL, NULL);
    System2 = ecs_register_system(ecs, system_update, NULL, NULL, NULL);
    System3 = ecs_register_system(ecs, system_update, NULL, NULL, NULL);

    // System1 requires PosComp compnents
    ecs_require_component(ecs, System1, PosComp);

    // System2 requires both PosComp and VelComp components
    ecs_require_component(ecs, System2, PosComp);
    ecs_require_component(ecs, System2, VelComp);

    // System3 requires the PosComp, VelComp, and RectComp components
    ecs_require_component(ecs, System3, PosComp);
    ecs_require_component(ecs, System3, VelComp);
    ecs_require_component(ecs, System3, RectComp);
}

int TestEcs(lua_State* L) {
    // Creates concrete ECS instance
    ecs_t* ecs = ecs_new(1024, NULL);

    // Register components and systems
    register_components(ecs);
    register_systems(ecs);

    // Create three entities
    ecs_id_t e1 = ecs_create(ecs);
    ecs_id_t e2 = ecs_create(ecs);
    ecs_id_t e3 = ecs_create(ecs);

    // Add components to entities
    printf("---------------------------------------------------------------\n");
    printf("Created entities: %u, %u, %u\n", e1, e2, e3);
    printf("---------------------------------------------------------------\n");

    printf("PosComp added to: %u\n", e1);
    ecs_add(ecs, e1, PosComp, NULL);

    printf("---------------------------------------------------------------\n");
    printf("PosComp added to: %u\n", e2);
    printf("VeloComp added to: %u\n", e2);

    ecs_add(ecs, e2, PosComp, NULL);
    ecs_add(ecs, e2, VelComp, NULL);

    printf("---------------------------------------------------------------\n");
    printf("PosComp added to: %u\n", e3);
    printf("VeloComp added to: %u\n", e3);
    printf("RectComp added to: %u\n", e3);

    ecs_add(ecs, e3, PosComp, NULL);
    ecs_add(ecs, e3, VelComp, NULL);
    ecs_add(ecs, e3, RectComp, NULL);

    printf("---------------------------------------------------------------\n");

    // Manually execute the systems
    printf("Executing system 1\n");
    ecs_update_system(ecs, System1, 0.0f);  // Output: e1 e2 e3

    printf("Executing system 2\n");
    ecs_update_system(ecs, System2, 0.0f);  // Output: e2 e3

    printf("Executing system 3\n");
    ecs_update_system(ecs, System3, 0.0f);  // Output: e3

    printf("---------------------------------------------------------------\n");

    ecs_free(ecs);

    lua_pushboolean(L, 1);

    return 1;
}

static int LUASTRUCT_test_vec4(lua_State* L) {
    Vector4* v4 = CHECK_STRUCT(L, 1, Vector4);
    v4->x += 10.f;
    v4->y += 10.f;
    v4->z += 10.f;
    v4->w += 10.f;
    PUSH_STRUCT(L, Vector4, *v4);
    return 1;
}

LUABIND_MODULE() {

    neko_lua_enum(L, AssetKind);
    neko_lua_enum_value(L, AssetKind, AssetKind_None);
    neko_lua_enum_value(L, AssetKind, AssetKind_LuaRef);
    neko_lua_enum_value(L, AssetKind, AssetKind_Image);
    neko_lua_enum_value(L, AssetKind, AssetKind_Sprite);
    neko_lua_enum_value(L, AssetKind, AssetKind_Tilemap);

    luaL_Reg lib[] = {
            {"LUASTRUCT_test_vec4", LUASTRUCT_test_vec4},
            {"TestAssetKind_1",
             +[](lua_State* L) {
                 AssetKind type_val;
                 neko_luabind_to(L, AssetKind, &type_val, 1);
                 neko_printf("%d", type_val);
                 lua_pushinteger(L, type_val);
                 return 1;
             }},

            {"TestAssetKind_2",
             +[](lua_State* L) {
                 AssetKind type_val = (AssetKind)lua_tointeger(L, 1);
                 neko_luabind_push(L, AssetKind, &type_val);
                 return 1;
             }},
            {"TestBinding_1", TestBinding_1},
            {"TestEcs", TestEcs},
            {NULL, NULL},
    };
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    return 1;
}
}  // namespace neko::lua::__unittest

struct PointInner {
    int xx, yy;
};

NEKO_STRUCT(PointInner, _Fs(xx, "inner_xxx"), _Fs(yy, "inner_yyy"));

struct Point {
    int x, y;
    PointInner inner;
};

NEKO_STRUCT(Point, _Fs(x, "xxx"), _Fs(y, "yyy"), _Fs(inner, "inner_shit"));

struct Rect {
    Point p1, p2;
    u32 color;
};

NEKO_STRUCT(Rect, _F(p1), _F(p2), _F(color));

template <typename T, typename Fields = std::tuple<>>
void dumpObj(T&& obj, int depth = 0, const char* fieldName = "", Fields&& fields = std::make_tuple()) {
    auto indent = [depth] {
        for (int i = 0; i < depth; ++i) {
            std::cout << "    ";
        }
    };

    if constexpr (std::is_class_v<std::decay_t<T>>) {
        indent();
        std::cout << fieldName << (*fieldName ? ": {" : "{") << std::endl;
        neko::reflection::struct_foreach(obj, [depth](auto&& fieldName, auto&& value, auto&& info) { dumpObj(value, depth + 1, fieldName, info); });
        indent();
        std::cout << "}" << (depth == 0 ? "" : ",") << std::endl;
    } else {
        indent();
        std::cout << fieldName << ": " << obj << ", " << std::get<0>(fields) << "," << std::endl;
    }
}

struct MyStruct {
    int i;
    float f;
};

template <class T>
void As(T arg) {
    std::cout << arg << std::endl;
}

static void test_struct() {

    MyStruct myStruct{1001, 2.f};
    neko::reflection::struct_apply(myStruct, [](auto&... args) { (..., As(args)); });

    Rect rect{
            {0, 0},
            {8, 9},
            12345678,
    };

    dumpObj(rect);

    std::tuple<int, double, std::string> myTuple;
    constexpr std::size_t size = neko::tuple_size_v<decltype(myTuple)>;

    std::cout << "Size of tuple: " << size << std::endl;
}

void test_se() { test_struct(); }

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        putc('\t', stdout);
    }
}

void print_xml_node(neko_xml_node_t* node, int indent) {
    print_indent(indent);
    printf("XML Node: %s\n", node->name);
    print_indent(indent);
    printf("\tText: %s\n", node->text);
    print_indent(indent);
    puts("\tAttributes:");
    for (neko_hash_table_iter it = neko_hash_table_iter_new(node->attributes); neko_hash_table_iter_valid(node->attributes, it); neko_hash_table_iter_advance(node->attributes, it)) {
        neko_xml_attribute_t attrib = neko_hash_table_iter_get(node->attributes, it);

        print_indent(indent);
        printf("\t\t%s: ", attrib.name);
        switch (attrib.type) {
            case NEKO_XML_ATTRIBUTE_NUMBER:
                printf("(number) %g\n", attrib.value.number);
                break;
            case NEKO_XML_ATTRIBUTE_BOOLEAN:
                printf("(boolean) %s\n", attrib.value.boolean ? "true" : "false");
                break;
            case NEKO_XML_ATTRIBUTE_STRING:
                printf("(string) %s\n", attrib.value.string);
                break;
            default:
                break;  // Unreachable
        }
    }

    if (neko_dyn_array_size(node->children) > 0) {
        print_indent(indent);
        printf("\t = Children = \n");
        for (uint32_t i = 0; i < neko_dyn_array_size(node->children); i++) {
            print_xml_node(node->children + i, indent + 1);
        }
    }
}

void test_xml(const std::string& file) {

    neko_xml_document_t* doc = neko_xml_parse_file(file.c_str());
    if (!doc) {
        printf("XML Parse Error: %s\n", neko_xml_get_error());
    } else {
        for (uint32_t i = 0; i < neko_dyn_array_size(doc->nodes); i++) {
            neko_xml_node_t* node = doc->nodes + i;
            print_xml_node(node, 0);
        }
        neko_xml_free(doc);
    }
}

typedef struct custom_key_t {
    uint32_t uval;
    float fval;
} custom_key_t;

neko_dyn_array(uint32_t) arr = NULL;
neko_hash_table(float, uint32_t) ht = NULL;
neko_hash_table(custom_key_t, uint32_t) htc = NULL;
neko_slot_array(double) sa = NULL;
neko_slot_map(uint64_t, uint32_t) sm = NULL;
neko_byte_buffer_t bb = {0};

#define ITER_CT 5

// Keys for slot map
const char* smkeys[ITER_CT] = {"John", "Dick", "Harry", "Donald", "Wayne"};

void test_containers() {

    NEKO_INVOKE_ONCE([] {
        bb = neko_byte_buffer_new();

        neko_byte_buffer_write(&bb, uint32_t, ITER_CT);

        for (uint32_t i = 0; i < ITER_CT; ++i) {
            neko_dyn_array_push(arr, i);

            neko_hash_table_insert(ht, (float)i, i);

            custom_key_t k = {.uval = i, .fval = (float)i * 2.f};
            neko_hash_table_insert(htc, k, i * 2);

            neko_slot_array_insert(sa, (double)i * 3.f);

            neko_slot_map_insert(sm, neko_hash_str64(smkeys[i]), i);

            neko_byte_buffer_write(&bb, uint32_t, i);
        }

        neko_byte_buffer_seek_to_beg(&bb);
    }(););

    neko_printf("neko_dyn_array: [");
    for (uint32_t i = 0; i < neko_dyn_array_size(arr); ++i) {
        neko_printf("%zu, ", arr[i]);
    }
    neko_println("]");

    neko_println("neko_hash_table: [");
    for (neko_hash_table_iter it = neko_hash_table_iter_new(ht); neko_hash_table_iter_valid(ht, it); neko_hash_table_iter_advance(ht, it)) {
        float k = neko_hash_table_iter_getk(ht, it);
        uint32_t v = neko_hash_table_iter_get(ht, it);
        neko_println("  {k: %.2f, v: %zu},", k, v);
    }
    neko_println("]");

    neko_println("neko_hash_table: [");
    for (neko_hash_table_iter it = neko_hash_table_iter_new(htc); neko_hash_table_iter_valid(htc, it); neko_hash_table_iter_advance(htc, it)) {
        custom_key_t* kp = neko_hash_table_iter_getkp(htc, it);
        uint32_t v = neko_hash_table_iter_get(htc, it);
        neko_println("  {k: {%zu, %.2f}, v: %zu},", kp->uval, kp->fval, v);
    }
    neko_println("]");

    neko_println("neko_slot_array: [");
    for (neko_slot_array_iter it = neko_slot_array_iter_new(sa); neko_slot_array_iter_valid(sa, it); neko_slot_array_iter_advance(sa, it)) {
        double v = neko_slot_array_iter_get(sa, it);
        neko_println("  id: %zu, v: %.2f", it, v);
    }
    neko_println("]");

    neko_println("neko_slot_map (manual): [");
    for (uint32_t i = 0; i < ITER_CT; ++i) {
        uint32_t v = neko_slot_map_get(sm, neko_hash_str64(smkeys[i]));
        neko_println("k: %s, h: %lu, v: %zu", smkeys[i], neko_hash_str64(smkeys[i]), v);
    }
    neko_println("]");

    neko_println("neko_slot_map (iterator): [");
    for (neko_slot_map_iter it = neko_slot_map_iter_new(sm); neko_slot_map_iter_valid(sm, it); neko_slot_map_iter_advance(sm, it)) {
        uint64_t k = neko_slot_map_iter_getk(sm, it);
        uint32_t v = neko_slot_map_iter_get(sm, it);
        neko_println("k: %lu, v: %zu", k, v);
    }
    neko_println("]");

    neko_println("neko_byte_buffer_t: [");

    neko_byte_buffer_readc(&bb, uint32_t, ct);

    for (uint32_t i = 0; i < ct; ++i) {
        neko_byte_buffer_readc(&bb, uint32_t, v);
        neko_println("v: %zu", v);
    }
    neko_println("]");

    neko_byte_buffer_seek_to_beg(&bb);

    neko_byte_buffer_free(&bb);

    neko_dyn_array_free(arr);

    neko_slot_array_free(sa);

    neko_slot_map_free(sm);

    neko_hash_table_free(ht);

    neko_hash_table_free(htc);
}

#pragma endregion test

#define NEKO_PROP

#if !defined(NEKO_PROP)

#include <string_view>

//
// Type names
//

#ifndef __FUNCSIG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

template <typename T>
constexpr std::string_view getTypeName() {
    constexpr auto prefixLength = 36, suffixLength = 1;
    const_str data = __FUNCSIG__;
    auto end = data;
    while (*end) {
        ++end;
    }
    return {data + prefixLength, size_t(end - data - prefixLength - suffixLength)};
}

//
// Component types list
//

template <int N>
struct neko_prop_component_type_counter : neko_prop_component_type_counter<N - 1> {
    static constexpr auto num = N;
};
template <>
struct neko_prop_component_type_counter<0> {
    static constexpr auto num = 0;
};
neko_prop_component_type_counter<0> numComponentTypes(neko_prop_component_type_counter<0>);

template <int I>
struct neko_prop_component_typelist;
template <>
struct neko_prop_component_typelist<0> {
    static void each(auto&& f) {}
};

inline constexpr auto maxNumComponentTypes = 32;

template <typename T>
inline constexpr auto isComponentType = false;

#define ComponentTypeListAdd(T)                                                                                                                       \
    template <>                                                                                                                                       \
    inline constexpr auto isComponentType<T> = true;                                                                                                  \
    constexpr auto ComponentTypeList_##T##_Size = decltype(numComponentTypes(neko_prop_component_type_counter<maxNumComponentTypes>()))::num + 1;     \
    static_assert(ComponentTypeList_##T##_Size < maxNumComponentTypes);                                                                               \
    neko_prop_component_type_counter<ComponentTypeList_##T##_Size> numComponentTypes(neko_prop_component_type_counter<ComponentTypeList_##T##_Size>); \
    template <>                                                                                                                                       \
    struct neko_prop_component_typelist<ComponentTypeList_##T##_Size> {                                                                               \
        static void each(auto&& f) {                                                                                                                  \
            neko_prop_component_typelist<ComponentTypeList_##T##_Size - 1>::each(f);                                                                  \
            f.template operator()<T>();                                                                                                               \
        }                                                                                                                                             \
    }

#define Comp(T)              \
    T;                       \
    ComponentTypeListAdd(T); \
    struct T

#define UseComponentTypes()                                                                                                                                           \
    static void forEachComponentType(auto&& f) {                                                                                                                      \
        neko_prop_component_typelist<decltype(numComponentTypes(neko_prop_component_type_counter<maxNumComponentTypes>()))::num>::each(std::forward<decltype(f)>(f)); \
    }

//
// Props
//

constexpr u32 props_hash(std::string_view str) {
    constexpr u32 offset = 2166136261;
    constexpr u32 prime = 16777619;
    auto result = offset;
    for (auto c : str) {
        result = (result ^ c) * prime;
    }
    return result;
}

struct neko_prop_attribs {
    std::string_view name;
    u32 nameHash = props_hash(name);

    bool exampleFlag = false;
};

inline constexpr auto maxNumProps = 24;

template <int N>
struct neko_prop_counter : neko_prop_counter<N - 1> {
    static constexpr auto num = N;
};
template <>
struct neko_prop_counter<0> {
    static constexpr auto num = 0;
};
[[maybe_unused]] static inline neko_prop_counter<0> numProps(neko_prop_counter<0>);

template <int N>
struct neko_prop_index {
    static constexpr auto index = N;
};

template <typename T, int N>
struct neko_prop_tag_wrapper {
    struct tag {
        static inline neko_prop_attribs attribs = T::getPropAttribs(neko_prop_index<N>{});
    };
};
template <typename T, int N>
struct neko_prop_tag_wrapper<const T, N> {
    using tag = typename neko_prop_tag_wrapper<T, N>::tag;
};
template <typename T, int N>
using neko_prop_tag = typename neko_prop_tag_wrapper<T, N>::tag;

#define neko_prop(type, name_, ...) neko_prop_named(#name_, type, name_, __VA_ARGS__)
#define neko_prop_named(nameStr, type, name_, ...)                                                                                                                                             \
    using name_##_Index = neko_prop_index<decltype(numProps(neko_prop_counter<maxNumProps>()))::num>;                                                                                          \
    static inline neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1> numProps(neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1>); \
    static std::type_identity<PROP_PARENS_1(PROP_PARENS_3 type)> propType(name_##_Index);                                                                                                      \
    static constexpr neko_prop_attribs getPropAttribs(name_##_Index) { return {.name = #name_, __VA_ARGS__}; };                                                                                \
    std::type_identity_t<PROP_PARENS_1(PROP_PARENS_3 type)> name_

#define PROP_PARENS_1(...) PROP_PARENS_2(__VA_ARGS__)
#define PROP_PARENS_2(...) NO##__VA_ARGS__
#define PROP_PARENS_3(...) PROP_PARENS_3 __VA_ARGS__
#define NOPROP_PARENS_3

template <auto memPtr>
struct neko_prop_containing_type {};
template <typename C, typename R, R C::*memPtr>
struct neko_prop_containing_type<memPtr> {
    using Type = C;
};
#define neko_prop_tag(field) neko_prop_tag<neko_prop_containing_type<&field>::Type, field##_Index::index>

struct __neko_prop_any {
    template <typename T>
    operator T() const;  // NOLINT(google-explicit-constructor)
};

template <typename Aggregate, typename Base = std::index_sequence<>, typename = void>
struct __neko_prop_count_fields : Base {};
template <typename Aggregate, int... Indices>
struct __neko_prop_count_fields<Aggregate, std::index_sequence<Indices...>,
                                std::void_t<decltype(Aggregate{{(static_cast<void>(Indices), std::declval<__neko_prop_any>())}..., {std::declval<__neko_prop_any>()}})>>
    : __neko_prop_count_fields<Aggregate, std::index_sequence<Indices..., sizeof...(Indices)>> {};
template <typename T>
constexpr int countFields() {
    return __neko_prop_count_fields<std::remove_cvref_t<T>>().size();
}

template <typename T>
concept neko_props = std::is_aggregate_v<T>;

template <neko_props T, typename F>
inline void forEachProp(T& val, F&& func) {
    if constexpr (requires { forEachField(const_cast<std::remove_cvref_t<T>&>(val), func); }) {
        forEachField(const_cast<std::remove_cvref_t<T>&>(val), func);
    } else if constexpr (requires { T::propType(neko_prop_index<0>{}); }) {
        constexpr auto n = countFields<T>();
        const auto call = [&]<typename Index>(Index index, auto& val) {
            if constexpr (requires { T::propType(index); }) {
                static_assert(std::is_same_v<typename decltype(T::propType(index))::type, std::remove_cvref_t<decltype(val)>>);
                func(neko_prop_tag<T, Index::index>{}, val);
            }
        };
#define C(i) call(neko_prop_index<i>{}, f##i)
        if constexpr (n == 1) {
            auto& [f0] = val;
            (C(0));
        } else if constexpr (n == 2) {
            auto& [f0, f1] = val;
            (C(0), C(1));
        } else if constexpr (n == 3) {
            auto& [f0, f1, f2] = val;
            (C(0), C(1), C(2));
        } else if constexpr (n == 4) {
            auto& [f0, f1, f2, f3] = val;
            (C(0), C(1), C(2), C(3));
        } else if constexpr (n == 5) {
            auto& [f0, f1, f2, f3, f4] = val;
            (C(0), C(1), C(2), C(3), C(4));
        } else if constexpr (n == 6) {
            auto& [f0, f1, f2, f3, f4, f5] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5));
        } else if constexpr (n == 7) {
            auto& [f0, f1, f2, f3, f4, f5, f6] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6));
        } else if constexpr (n == 8) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7));
        } else if constexpr (n == 9) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8));
        } else if constexpr (n == 10) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9));
        } else if constexpr (n == 11) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10));
        } else if constexpr (n == 12) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11));
        } else if constexpr (n == 13) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12));
        } else if constexpr (n == 14) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13));
        } else if constexpr (n == 15) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14));
        } else if constexpr (n == 16) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15));
        } else if constexpr (n == 17) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16));
        } else if constexpr (n == 18) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17));
        } else if constexpr (n == 19) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18));
        } else if constexpr (n == 20) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19));
        } else if constexpr (n == 21) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20));
        } else if constexpr (n == 22) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21));
        } else if constexpr (n == 23) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21), C(22));
        } else if constexpr (n == 24) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21), C(22), C(23));
        }
#undef C
    }
}

#endif

#if 0

void print(const float &val) { std::printf("%f", val); }
void print(const std::string &val) { std::printf("%s", val.c_str()); }

struct Comp(Position) {
    // Register reflectable fields ('props') using the `Prop` macro. This can be used in any aggregate
    // `struct`, doesn't have to be a `Comp`.
    neko_prop(float, x) = 0;
    neko_prop(float, y) = 0;
};

struct Comp(Name) {
    neko_prop(std::string, first) = "First";

    // Props can have additional attributes, see the `neko_prop_attribs` type. You can customize and add
    // your own attributes there.
    neko_prop(std::string, last, .exampleFlag = true) = "Last";

    int internal = 42;  // This is a non-prop field that doesn't show up in reflection. IMPORTANT NOTE:
    // All such non-prop fields must be at the end of the struct, with all props at
    // the front. Mixing props and non-props in the order is not allowed.
};

// After all compnonent types are defined, this macro must be called to be able to use
// `forEachComponentType` to iterate all component types
UseComponentTypes();

#endif